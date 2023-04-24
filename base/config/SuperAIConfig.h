#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"

using std::map;
using std::string;
using std::vector;

class AITarget;
class AIAction;
class AICondition;
class BirthAICondition;
class TimerAICondition;
class HPLessAICondition;
class SelfAITarget;
class RangeAITarget;
class TalkAIAction;
class SummonNpcAIAction;
class GMCmdAIAction;

// ai config data
struct SSuperConditionCFG
{
  string content;

  xLuaData oParams;

  SSuperConditionCFG() {}
};
struct SSuperTargetCFG
{
  string content;

  xLuaData oParams;

  SSuperTargetCFG() {}
};
typedef vector<SSuperTargetCFG> TVecSuperTargetCFG;
struct SSuperActionCFG
{
  string content;

  xLuaData oParams;

  SSuperActionCFG() {}
};
typedef vector<SSuperActionCFG> TVecSuperActionCFG;
struct SSuperAIItem
{
  DWORD id = 0;

  SSuperConditionCFG stCondition;

  TVecSuperTargetCFG vecTargets;
  TVecSuperActionCFG vecActions;

  SSuperAIItem() {}
};
typedef vector<SSuperAIItem> TVecSSuperAIItem;

struct SSuperAICFG
{
  DWORD dwID = 0;

  TVecSSuperAIItem vecItems;

  SSuperAICFG() {}
};

// aiconfig
class SuperAIConfig : public xSingleton<SuperAIConfig>
{
  public:
    SuperAIConfig();
    virtual ~SuperAIConfig();

    bool loadConfig();

    const SSuperAICFG* getSuperAICFG(DWORD dwID) const;
  private:
    void regAIItem();
  private:
    map<DWORD, SSuperAICFG> m_mapSuperAICFG;
};

