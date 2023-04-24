/**
 * @file ActionItem.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-09-26
 */

#pragma once

#include "xDefine.h"
#include "ActionConfig.h"
#include "MsgManager.h"

class SceneUser;

enum EActionStatus
{
  EACTIONSTATUS_MIN = 0,
  EACTIONSTATUS_RUN = 1,
  EACTIONSTATUS_OVER = 2,
  EACTIONSTATUS_MAX,
};

// base action
class BaseAction
{
  public:
    BaseAction(SceneUser* pUser);
    virtual ~BaseAction();

    virtual DWORD id() const { return m_dwID; }
    virtual EActionType getType() const { return EACTIONTYPE_MIN; }

    virtual bool init(const SActionCFG* pCFG);
    virtual EActionStatus doAction(DWORD curSec);

    virtual void setTime(DWORD dwTime) { m_dwTime = dwTime; }
  protected:
    SceneUser* m_pUser = nullptr;

    DWORD m_dwID = 0;
    DWORD m_dwStartTime = 0;
    DWORD m_dwTime = 0;

    bool m_bSet = false;
};

// effect action
class EffectAction : public BaseAction
{
  public:
    EffectAction(SceneUser* pUser);
    virtual ~EffectAction();

    virtual EActionType getType() const { return EACTIONTYPE_EFFECT; }

    virtual bool init(const SActionCFG* pCFG);
    virtual EActionStatus doAction(DWORD curSec);
  private:
    DWORD m_dwEffectID = 0;
};

// exp action
class ExpAction : public BaseAction
{
  public:
    ExpAction(SceneUser* pUser);
    virtual ~ExpAction();

    virtual EActionType getType() const { return EACTIONTYPE_EXP; }

    virtual bool init(const SActionCFG* pCFG);
    virtual EActionStatus doAction(DWORD curSec);
  private:
    enum EExpMethod
    {
      EEXPMETHOD_MIN = 0,
      EEXPMETHOD_ADD_BASE = 1,
      EEXPMETHOD_ADD_JOB = 2,
      EEXPMETHOD_MAX,
    };

    EExpMethod m_eMethod = EEXPMETHOD_MIN;
    string m_strScript;
};

// sys msg action
class SysMsgAction : public BaseAction
{
  public:
    SysMsgAction(SceneUser* pUser);
    virtual ~SysMsgAction();

    virtual EActionType getType() const { return EACTIONTYPE_SYSMSG; }

    virtual bool init(const SActionCFG* pCFG);
    virtual EActionStatus doAction(DWORD curSec);

    MsgParams& getParams() { return m_oParam; }
  private:
    DWORD m_dwMsgID = 0;
    MsgParams m_oParam;
};

// buff action
class BuffAction : public BaseAction
{
  public:
    BuffAction(SceneUser* pUser);
    virtual ~BuffAction();

    virtual EActionType getType() const { return EACTIONTYPE_BUFF; }

    virtual bool init(const SActionCFG* pCFG);
    virtual EActionStatus doAction(DWORD curSec);
  private:
    enum EBuffMethod
    {
      EBUFFMETHOD_MIN = 0,
      EBUFFMETHOD_ADD = 1,
      EBUFFMETHOD_MAX,
    };

    EBuffMethod m_eMethod = EBUFFMETHOD_MIN;
    DWORD m_dwBuffID = 0;
};

// equip attr action
class EquipAttrAction : public BaseAction
{
  public:
    EquipAttrAction(SceneUser* pUser);
    virtual ~EquipAttrAction();

    virtual EActionType getType() const { return EACTIONTYPE_EQUIPATTR; }

    virtual bool init(const SActionCFG* pCFG);
    virtual EActionStatus doAction(DWORD curSec);
  private:
    string m_strGUID;
};

