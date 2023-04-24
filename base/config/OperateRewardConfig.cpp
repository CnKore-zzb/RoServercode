#include "OperateRewardConfig.h"
#include "xLuaTable.h"

OperateRewardConfig::OperateRewardConfig()
{
}

OperateRewardConfig::~OperateRewardConfig()
{
}

bool OperateRewardConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_OperateReward.txt"))
  {
    XERR << "[表格],加载表格,Table_OperateReward.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_OperateReward", table);
  m_mapOperateRewardCfg.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapOperateRewardCfg.find(id);
    if (it == m_mapOperateRewardCfg.end())
    {
      m_mapOperateRewardCfg[id] = SOperateRewardCFG();
      it = m_mapOperateRewardCfg.find(id);
    }
    if (it == m_mapOperateRewardCfg.end())
    {
      XERR << "[Table_OperateReward] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }
    SOperateRewardCFG& rCfg = it->second;
    rCfg.dwId = id;
    rCfg.type = m->second.getTableInt("type");
    rCfg.uid = m->second.getTableString("uid");
    rCfg.dwRewardId = m->second.getTableInt("reward");
    rCfg.dwZeny = m->second.getTableInt("zeny");
    rCfg.dwMonthcardCount = m->second.getTableInt("monthcard");
  }
  if (bCorrect)
    XLOG << "[Table_OperateReward], 成功加载表格Table_OperateReward.txt" << XEND;
  return bCorrect;
}

SOperateRewardCFG* OperateRewardConfig::getOperateRewardCFG(string uid, DWORD type) 
{
  for (auto &v : m_mapOperateRewardCfg)
  {
    if (v.second.uid == uid && v.second.type == type)
      return &(v.second);
  }
  return nullptr;
}
