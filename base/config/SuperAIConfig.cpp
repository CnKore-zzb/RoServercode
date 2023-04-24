#include "SuperAIConfig.h"
#include "xLuaTable.h"

// const char LOG_NAME[] = "SuperAIConfig";

// aiconfig
SuperAIConfig::SuperAIConfig()
{

}

SuperAIConfig::~SuperAIConfig()
{

}

bool SuperAIConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_SuperAI.txt"))
  {
    XERR << "[表格],加载表格,Table_SuperAI.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_SuperAI", table);
  m_mapSuperAICFG.clear();
  DWORD tmpStepIndex = 0;
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("ID");
    auto o = m_mapSuperAICFG.find(id);
    if (o == m_mapSuperAICFG.end())
    {
      m_mapSuperAICFG[id] = SSuperAICFG();
      o = m_mapSuperAICFG.find(id);
    }
    if (o == m_mapSuperAICFG.end())
    {
      XERR << "[SuperAIConfig] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }

    xLuaData params = m->second.getMutableData("Params");
    //params.setData("Odds", m->second.getTableInt("Odds"));

    string type = m->second.getTableString("ItemType");
    if (type == "condition")
    {
      SSuperAIItem stCFG;
      tmpStepIndex = 0;

      stCFG.id = id;
      stCFG.stCondition.content = m->second.getTableString("Content");
      stCFG.stCondition.oParams = params;
      stCFG.stCondition.oParams.setData("weight", m->second.getTableInt("weight"));

      xLuaData& fre = m->second.getMutableData("Frequency");
      stCFG.stCondition.oParams.setData("interval", fre.getTableInt("interval"));
      stCFG.stCondition.oParams.setData("count", fre.getTableInt("count"));
      stCFG.stCondition.oParams.setData("odds", fre.getTableInt("odds"));

      o->second.vecItems.push_back(stCFG);
    }
    else if (type == "target")
    {
      SSuperAIItem& rCFG = *o->second.vecItems.rbegin();
      tmpStepIndex ++;

      SSuperTargetCFG stTarget;
      stTarget.content = m->second.getTableString("Content");
      stTarget.oParams = params;
      stTarget.oParams.setData("ai_step_index", tmpStepIndex);
      rCFG.vecTargets.push_back(stTarget);
    }
    else if (type == "action")
    {
      SSuperAIItem& rCFG = *o->second.vecItems.rbegin();

      SSuperActionCFG stAction;
      stAction.content = m->second.getTableString("Content");
      stAction.oParams = params;
      stAction.oParams.setData("odds", m->second.getMutableData("Frequency").getTableInt("odds"));
      stAction.oParams.setData("ai_step_index", tmpStepIndex);
      rCFG.vecActions.push_back(stAction);
    }
    else
    {
      XERR << "[SuperAIConfig] id = " << id << " ItemType = " << type.c_str() << " not valid" << XEND;
      bCorrect = false;
      continue;
    }
  }

  // calc weight rate
  for (auto &m : m_mapSuperAICFG)
  {
    DWORD dwMaxRate = 0;
    for (auto &v : m.second.vecItems)
    {
      dwMaxRate += v.stCondition.oParams.getTableInt("rate");
      v.stCondition.oParams.setData("maxrate", dwMaxRate);
    }
  }

  if (bCorrect)
    XLOG << "[SuperAIConfig], 成功加载表格Table_SuperAI.txt" << XEND;
  return bCorrect;
}

const SSuperAICFG* SuperAIConfig::getSuperAICFG(DWORD dwID) const
{
  auto m = m_mapSuperAICFG.find(dwID);
  if (m != m_mapSuperAICFG.end())
    return &m->second;

  return nullptr;
}

