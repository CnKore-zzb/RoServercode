#include "DeathTransferMapConfig.h"
#include "ItemConfig.h"
#include "xTools.h"

DeathTransferConfig::DeathTransferConfig()
{
}

DeathTransferConfig::~DeathTransferConfig()
{
}

bool DeathTransferConfig::loadConfig()
{
  bool bCorrect = true;
  if (loadDeathTransferConfig() == false)
    bCorrect = false;

  return bCorrect;
}

bool DeathTransferConfig::checkConfig()
{
  return true;
}

bool DeathTransferConfig::loadDeathTransferConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_DeathTransferMap.txt"))
  {
    XERR << "[表格],加载表格,Table_DeathTransferMap.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_DeathTransferMap", table);
  m_mapDeathTransferCfg.clear();
  //m_mapQuest2Transfer.clear();

  //尼夫海姆平原1或2
  DWORD deathMapNumber = 0;
  DWORD lastMapId = 0;
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD mapid = m->second.getTableInt("MapID");
    if(mapid != lastMapId)
    {
      deathMapNumber++;
      lastMapId = mapid;
    }

    xLuaData& data = m->second.getMutableData("Transfer");

    SDeathTransferConfig stTransferCFG;
    stTransferCFG.mapid = mapid;
    stTransferCFG.deathMapNumber = deathMapNumber;
    stTransferCFG.npcid = data.getTableInt("NpcID");
    stTransferCFG.npcType = data.getTableInt("NpcType") == 0?ETransferType::Parent:ETransferType::Child;
    xLuaData& pos = data.getMutableData("Position");
    stTransferCFG.pos.x = pos.getTableFloat("1");
    stTransferCFG.pos.y = pos.getTableFloat("2");
    stTransferCFG.pos.z = pos.getTableFloat("3");

    m_mapDeathTransferCfg[stTransferCFG.npcid] = stTransferCFG;
      
      // 读取传送门激活对应任务
      //DWORD questid = data.getTableInt("QuestID");
      //m_mapQuest2Transfer[questid] = stTransferCFG.npcid;
      // 地图对应传送门
    if(deathMapNumber == 1){
      m_vecMap1Transferid.push_back(stTransferCFG.npcid);
    }
    else if(deathMapNumber == 2){
      m_vecMap2Transferid.push_back(stTransferCFG.npcid);
    }
    else{
      XERR << "[表格],加载表格,Table_DeathTransferMap.txt,失败,读取到超过2张地图" << XEND;
    }
    
  }

  if (bCorrect)
    XLOG << "[Table_DeathTransferMap], 成功加载表格Table_DeathTransferMap.txt" << XEND;
  return bCorrect;
}

/*
DWORD DeathTransferConfig::getTransferIdByQuestId(DWORD dwQuestid){
  auto it = m_mapQuest2Transfer.find(dwQuestid);
  if(it != m_mapQuest2Transfer.end()){
    return it->second;
  }
  else{
    return 0;
  }
}
*/

SDeathTransferConfig* DeathTransferConfig::getDeathTransferCfg(DWORD dwNpcid)
{
  auto it = m_mapDeathTransferCfg.find(dwNpcid);
  if (it == m_mapDeathTransferCfg.end())
    return nullptr;
  return &(it->second);
}

bool DeathTransferConfig::isMapAllActivated(DWORD dwDeathMapNumber, const std::set<DWORD>& setActivatedNpcid)
{
  // 判断是否vector内元素set都有
  auto isAllInFunc = [&](std::vector<DWORD> vec){
    for(auto it = vec.begin(); it != vec.end(); it++){
      if(setActivatedNpcid.find(*it) == setActivatedNpcid.end()){
        return false;
      }
    }
    return true;
  };
  if(dwDeathMapNumber == 1){
    return isAllInFunc(m_vecMap1Transferid);
  }
  else if(dwDeathMapNumber == 2){
    return isAllInFunc(m_vecMap2Transferid);
  }
  return false;
}
