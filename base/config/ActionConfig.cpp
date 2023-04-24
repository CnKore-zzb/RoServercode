#include "ActionConfig.h"

ActionConfig::ActionConfig()
{

}

ActionConfig::~ActionConfig()
{

}

bool ActionConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Action.txt"))
  {
    XERR << "[表格],加载表格,Table_Action.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Action", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    xLuaData& actions = m->second.getMutableData("actions");

    EActionType type = getActionType(actions.getTableString("type"));
    if (type == EACTIONTYPE_MIN)
    {
      bCorrect = false;
      XERR << "[Table_Action] id :" << m->first << "type :" << actions.getTableString("type") << "不合法" << XEND;
      continue;
    }

    SActionCFG& rCFG = m_mapActionCFG[m->first];
    rCFG.dwID = m->first;
    rCFG.type = type;
    rCFG.oData = actions.getMutableData("params");
  }

  if (bCorrect)
    XLOG << "[Table_Action], 成功加载表格Table_Action.txt" << XEND;
  return bCorrect;
}

const SActionCFG* ActionConfig::getActionCFG(DWORD dwID) const
{
  auto m = m_mapActionCFG.find(dwID);
  if (m != m_mapActionCFG.end())
    return &m->second;
  return nullptr;
}

EActionType ActionConfig::getActionType(const string& str) const
{
  if (str == "playeffect")
    return EACTIONTYPE_EFFECT;
  else if (str == "exp")
    return EACTIONTYPE_EXP;
  else if (str == "sysmsg")
    return EACTIONTYPE_SYSMSG;
  else if (str == "buff")
    return EACTIONTYPE_BUFF;

  return EACTIONTYPE_MIN;
}

