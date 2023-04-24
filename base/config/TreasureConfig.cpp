#include "TreasureConfig.h"
#include "xLuaTable.h"

// config data
const STreasureNpcProcess* STreasureNpc::getProcess(DWORD dwIndex) const
{
  if (dwIndex >= vecProcess.size())
    return nullptr;

  return &vecProcess[dwIndex];
}

DWORD STreasureNpc::randPosIndex(const TSetDWORD& setExclude) const
{
  DWORD dwCount = 0;
  while (++dwCount < 30)
  {
    DWORD dwRand = randBetween(0, vecPos.size() - 1);
    if (setExclude.find(dwRand) == setExclude.end())
      return dwRand;
  }

  return DWORD_MAX;
}

const STreasureNpc* STreasureCFG::getTree(ETreeType eType) const
{
  auto m = mapNpcType.find(eType);
  if (m != mapNpcType.end())
    return &m->second;
  return nullptr;
}

// config
TreasureConfig::TreasureConfig()
{

}

TreasureConfig::~TreasureConfig()
{

}

bool TreasureConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_MapHuntTreasure.txt"))
  {
    XERR << "[寻宝配置-加载] 加载配置 Table_MapHuntTreasure.txt 失败" << XEND;
    return false;
  }

  m_mapTreasureCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_MapHuntTreasure", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwMapID = m->second.getTableInt("MapId");
    auto item = m_mapTreasureCFG.find(dwMapID);
    if (item == m_mapTreasureCFG.end())
    {
      STreasureCFG& rCFG = m_mapTreasureCFG[dwMapID];
      rCFG.dwMapID = dwMapID;
      item = m_mapTreasureCFG.find(dwMapID);
    }
    if (item == m_mapTreasureCFG.end())
    {
      bCorrect = false;
      XERR << "[寻宝配置-加载] mapid :" << dwMapID << "未在列表中发现" << XEND;
      continue;
    }

    xLuaData& type = m->second.getMutableData("Type");
    DWORD dwType = type.getTableInt("1");
    DWORD dwNpcID = type.getTableInt("2");

    if (dwType <= ETREETYPE_MIN || dwType >= ETREETYPE_MAX)
    {
      bCorrect = false;
      XERR << "[寻宝配置-加载] mapid :" << dwMapID << "Type :" << dwType << "不合法" << XEND;
      continue;
    }

    ETreeType eTreeType = static_cast<ETreeType>(dwType);
    auto npc = item->second.mapNpcType.find(eTreeType);
    if (npc != item->second.mapNpcType.end())
    {
      bCorrect = false;
      XERR << "[寻宝配置-加载] mapid :" << dwMapID << "Type :" << dwType << "重复了" << XEND;
      continue;
    }

    STreasureNpc& stNpc = item->second.mapNpcType[eTreeType];
    stNpc.dwNpcID = dwNpcID;
    stNpc.eType = eTreeType;

    xLuaData& pos = m->second.getMutableData("Pos");
    auto posf = [&](const string& key, xLuaData& data)
    {
      xPos oPos;
      oPos.x = data.getTableFloat("1");
      oPos.y = data.getTableFloat("2");
      oPos.z = data.getTableFloat("3");
      stNpc.vecPos.push_back(oPos);
    };
    pos.foreach(posf);

    auto process_monster = [&](xLuaData& data)
    {
      DWORD dwNpcID = data.getTableInt("id");
      if (dwNpcID == 0)
        return;

      STreasureNpcProcess stProcess;
      stProcess.dwNpcID = dwNpcID;
      stProcess.dwCount = data.getTableInt("num");
      stProcess.eType = ETREESTATUS_MONSTER;
      stProcess.oNpcData = data;
      stNpc.vecProcess.push_back(stProcess);
    };
    /*auto process_reward = [&](DWORD dwRewardID)
    {
      if (dwRewardID == 0)
        return;

      STreasureNpcProcess stProcess;
      stProcess.dwRewardID = dwRewardID;
      stProcess.eType = ETREESTATUS_REWARD;
      stNpc.vecProcess.push_back(stProcess);
    };*/

    process_monster(m->second.getMutableData("GuardMonster1"));
    //process_reward(m->second.getTableInt("Reward1"));
    process_monster(m->second.getMutableData("GuardMonster2"));
    //process_reward(m->second.getTableInt("Reward2"));
    process_monster(m->second.getMutableData("GuardMonster3"));
    //process_reward(m->second.getTableInt("Reward3"));
    process_monster(m->second.getMutableData("GuardMonster4"));
  }

  if (bCorrect)
    XLOG << "[寻宝配置-加载] 成功加载 Table_MapHuntTreasure.txt 配置" << XEND;
  return true;
}

const STreasureCFG* TreasureConfig::getTreasureCFG(DWORD dwMapID) const
{
  auto m = m_mapTreasureCFG.find(dwMapID);
  if (m != m_mapTreasureCFG.end())
    return &m->second;

  return nullptr;
}

