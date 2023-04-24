#include "DeadConfig.h"

DeadConfig::DeadConfig()
{
}

DeadConfig::~DeadConfig()
{
}

bool DeadConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_DeadLevel.txt"))
  {
    XERR << "[Table_DeadLevel],加载配置Table_DeadLevel.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapLvCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_DeadLevel", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SDeadLvCFG& rCFG = m_mapLvCFG[m->first];
    rCFG.dwLv = m->first;
    rCFG.dwExp = m->second.getTableInt("exp");
    rCFG.blInit = true;
  }

  if (bCorrect)
    XLOG << "[Table_DeadLevel] 成功加载Table_DeadLevel.txt" << XEND;
  return true;
}

bool DeadConfig::checkConfig()
{
  return true;
}

const SDeadLvCFG* DeadConfig::getDeadCFG(DWORD lv) const
{
  auto m = m_mapLvCFG.find(lv);
  if (m != m_mapLvCFG.end())
    return &m->second;
  return nullptr;
}

