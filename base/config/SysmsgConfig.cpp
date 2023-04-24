#include "SysmsgConfig.h"

SysmsgConfig::SysmsgConfig()
{
}

SysmsgConfig::~SysmsgConfig()
{
}

bool SysmsgConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Sysmsg.txt"))
  {
    XERR << "[Table_Sysmsg],加载配置Table_Sysmsg.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Sysmsg", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    m_mapSysmsgCFG[m->first] = m->second.getTableString("Text");
  }

  if (bCorrect)
    XLOG << "[Table_Sysmsg] 成功加载Table_Sysmsg.txt" << XEND;
  return true;
}

bool SysmsgConfig::checkConfig()
{
  return true;
}

const string& SysmsgConfig::getSysmsgCFG(DWORD ID) const
{
  auto m = m_mapSysmsgCFG.find(ID);
  if (m != m_mapSysmsgCFG.end())
    return m->second;
  return STRING_EMPTY;
}

