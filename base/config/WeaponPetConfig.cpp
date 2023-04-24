#include "WeaponPetConfig.h"
#include "xLuaTable.h"
#include "NpcConfig.h"

WeaponPetConfig::WeaponPetConfig()
{

}

WeaponPetConfig::~WeaponPetConfig()
{

}

bool WeaponPetConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_MercenaryCat.txt"))
  {
    XERR << "[Table_MercenaryCat],加载配置Table_MercenaryCat.txt失败" << XEND;
      return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_MercenaryCat", table);
  m_mapID2CFG.clear();
  for (auto &m : table)
  {
    if (m_mapID2CFG.find(m.first) != m_mapID2CFG.end())
    {
      bCorrect = false;
      XERR << "[Table_MercenaryCat], 加载, ID重复" << m.first << XEND;
      continue;
    }
    SWeaponPetCFG& stCFG = m_mapID2CFG[m.first];

    stCFG.dwConfigID = m.first;
    stCFG.dwMonsterID = m.second.getTableInt("MonsterID");
    const SNpcCFG* pNpcCFG = NpcConfig::getMe().getNpcCFG(stCFG.dwMonsterID);
    if (pNpcCFG == nullptr || pNpcCFG->eNpcType!= ENPCTYPE_WEAPONPET)
    {
      bCorrect = false;
      XERR << "[Table_MercenaryCat], 怪物配置错误, ID:" << m.first << "MonsterID:" << stCFG.dwMonsterID << XEND;
      continue;
    }

    stCFG.dwReliveTime = m.second.getTableInt("Rest");
    stCFG.dwFollowDis = m.second.getTableInt("Follow");
    DWORD daymoney = m.second.getTableInt("Price1Day");
    if (daymoney)
      stCFG.mapType2Money[EEMPLOYTYPE_DAY] = daymoney;
    DWORD weekmoney = m.second.getTableInt("Price7Day");
    if (weekmoney)
      stCFG.mapType2Money[EEMPLOYTYPE_WEEK] = weekmoney;

    stCFG.dwReleatedNpcID = m.second.getTableInt("NPCID");
    stCFG.dwDiscount = m.second.getTableInt("Discount");
    stCFG.name = m.second.getTableString("CatName");
  }
  if (bCorrect)
    XLOG << "[Table_MercenaryCat], 加载配置成功" << XEND;
  return bCorrect;
}

const SWeaponPetCFG* WeaponPetConfig::getCFG(DWORD id) const
{
  auto it = m_mapID2CFG.find(id);
  if (it != m_mapID2CFG.end())
    return &it->second;
  return nullptr;
}

