/**
 * @file ActionConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-09-26
 */

#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"
#include "TableManager.h"

using std::map;

const DWORD TUTOR_ADVENTURE_ACTION_ID = 9999;
const DWORD EQUIP_ATTR_ACTION_ID = 20000;

enum EActionType
{
  EACTIONTYPE_MIN = 0,
  EACTIONTYPE_EFFECT = 1,
  EACTIONTYPE_EXP = 2,
  EACTIONTYPE_SYSMSG = 3,
  EACTIONTYPE_BUFF = 4,
  EACTIONTYPE_EQUIPATTR = 5,
  EACTIONTYPE_MAX,
};

struct SActionCFG
{
  DWORD dwID = 0;
  EActionType type = EACTIONTYPE_MIN;
  xLuaData oData;
};

typedef map<DWORD, SActionCFG> TMapActionCFG;

class ActionConfig : public xSingleton<ActionConfig>
{
  friend class xSingleton<ActionConfig>;
  private:
    ActionConfig();
  public:
    virtual ~ActionConfig();

    bool loadConfig();
    bool checkConfig() { return true; }

    const SActionCFG* getActionCFG(DWORD dwID) const;
  private:
    EActionType getActionType(const string& str) const;
  private:
    TMapActionCFG m_mapActionCFG;
};

