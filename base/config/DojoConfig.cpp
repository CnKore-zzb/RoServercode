#include "DojoConfig.h"
#include "xLuaTable.h"
#include "RewardConfig.h"
// config
DojoConfig::DojoConfig()
{
}

DojoConfig::~DojoConfig()
{
}

bool DojoConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild_Dojo.txt"))
  {
    XERR << "[DojoConfig],加载配置Table_Guild_Dojo.txt失败" << XEND;
    return false;
  }

  std::set<DWORD> head;
  std::set<DWORD> tail;

  m_mapID2Item.clear();
  m_mapRaid2Group.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Guild_Dojo", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    auto o = m_mapID2Item.find(m->first);
    if (o != m_mapID2Item.end())
    {
      XERR << "[DojoConfig] id = " << m->first << " duplicated!" << XEND;
      bCorrect = false;
      continue;
    }
    SDojoItemCfg item;
    item.dwDojoId = m->first;

    item.dwFirstReward = m->second.getTableInt("FirstReward");
    item.dwGroupId = m->second.getTableInt("DojoGroupId");
    item.dwRapid = m->second.getTableInt("FirstReward");
    item.dwHelpReward = m->second.getTableInt("HelpReward");
    item.dwRapid = m->second.getTableInt("Raid");
    item.dwLevel = m->second.getTableInt("Level");

    m_mapID2Item.insert(std::make_pair(item.dwDojoId, item));
    m_mapRaid2Group[item.dwRapid] = item.dwGroupId;
    m_mapGroupDojo[item.dwGroupId].insert(m->first);
  }

  if (bCorrect)
    XLOG << "[DojoConfig] 成功加载Table_Guild_Dojo.txt" << XEND;
  return bCorrect;
}

bool DojoConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto it = m_mapID2Item.begin(); it != m_mapID2Item.end(); ++it)
  {
    // check reward
    if (RewardConfig::getMe().getRewardCFG(it->second.dwFirstReward) == nullptr)
    {
      XERR << "[DojoConfig-配置检查] id : " << it->first << " dwFirstReward : " << it->second.dwFirstReward << " 未在 Table_Reward.txt 表中找到" << XEND;
      bCorrect = false;
    }
    // check reward
    if (RewardConfig::getMe().getRewardCFG(it->second.dwHelpReward) == nullptr)
    {
      XERR << "[DojoConfig-配置检查] id : " << it->first << " dwHelpReward : " << it->second.dwHelpReward << " 未在 Table_Reward.txt 表中找到" << XEND;
      bCorrect = false;
    }
  }
  return bCorrect;
}

const SDojoItemCfg* DojoConfig::getDojoItemCfg(DWORD dwID) const
{
  auto it = m_mapID2Item.find(dwID);
  if (it != m_mapID2Item.end())
  {
    return &(it->second);
  }
  return nullptr;
}

bool DojoConfig::isDojoMap(DWORD dwMapId)
{
  auto it = find_if(m_mapID2Item.begin(), m_mapID2Item.end(), [dwMapId](const std::pair<DWORD, SDojoItemCfg> &p)->bool { return p.second.dwRapid == dwMapId; });
  if (it == m_mapID2Item.end())
    return false;
  return true;
}

bool DojoConfig::hasGroup(DWORD dwGroupID) const
{
  for (auto &m : m_mapID2Item)
  {
    if (m.second.dwGroupId == dwGroupID)
      return true;
  }

  return false;
}

DWORD DojoConfig::getGroupIDByRaid(DWORD dwRaid) const
{
  auto m = m_mapRaid2Group.find(dwRaid);
  if (m != m_mapRaid2Group.end())
    return m->second;
  return 0;
}

void DojoConfig::collectDojoID(DWORD dwGroupID, TSetDWORD& setIDs) const
{
  auto m = m_mapGroupDojo.find(dwGroupID);
  if (m == m_mapGroupDojo.end())
    setIDs.clear();
  else
    setIDs = m->second;
}

