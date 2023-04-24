/**
 * @file BossStep.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-08-16
 */

#pragma once

#include <memory>
#include "xLuaTable.h"
#include "BossCmd.pb.h"
#include "SceneDefine.h"

class BossAct;
class SceneUser;

// BossStep - base
class BossBaseStep
{
  public:
    BossBaseStep() {}
    virtual ~BossBaseStep() {}

    virtual Cmd::EBossStep getType() const = 0;
    void toParams(ConfigParam* pConfig) { if (pConfig) pConfig->CopyFrom(m_oParams); }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    ConfigParam m_oParams;
};
typedef std::shared_ptr<BossBaseStep> TPtrBossStep;

// BossStep - visit
class BossVisitStep : public BossBaseStep
{
  public:
    BossVisitStep() : BossBaseStep() {}
    virtual ~BossVisitStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_VISIT; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);

    DWORD getNpcID() const { return m_dwNpcID; }
    DWORD getMonsterID() const { return m_dwMonsterID; }
    bool hasSelect() const { return m_bHasSelect; }
  private:
    DWORD m_dwNpcID = 0;
    DWORD m_dwMonsterID = 0;
    DWORD m_dwMonsterNum = 0;
    bool m_bHasSelect = false;
};

// BossStep - summon
class BossSummonStep : public BossBaseStep
{
  public:
    BossSummonStep() : BossBaseStep() {}
    virtual ~BossSummonStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_SUMMON; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    NpcDefine m_oDefine;
    DWORD m_dwNum = 0;

    enum ERandPos
    {
      ERANDPOS_FULLMAP = 0,
      ERANDPOS_RADIUS = 1,
      ERANDPOS_RANDRADIUS = 2,
      ERANDPOS_NPCPOS = 3,
    };
    ERandPos m_ePos = ERANDPOS_FULLMAP;
    float m_fRadius = 0.0f;
    DWORD m_dwNeedNpcPos = 0;
};

// BossStep - clear
class BossClearStep : public BossBaseStep
{
  public:
    BossClearStep() : BossBaseStep() {}
    virtual ~BossClearStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_CLEAR; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    DWORD m_dwMonsterID = 0;
};

// BossStep - boss
class BossBossStep : public BossBaseStep
{
  public:
    BossBossStep() : BossBaseStep() {}
    virtual ~BossBossStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_BOSS; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    DWORD m_dwNeedNpcPos = 0;
};

// BossStep - limit
class BossLimitStep : public BossBaseStep
{
  public:
    BossLimitStep() : BossBaseStep() {}
    virtual ~BossLimitStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_LIMIT; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    DWORD m_dwTime = 0;
};

// BossStep - dialog
class BossDialogStep : public BossBaseStep
{
  public:
    BossDialogStep() : BossBaseStep() {}
    virtual ~BossDialogStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_DIALOG; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
};

// BossStep - status
class BossStatusStep : public BossBaseStep
{
  public:
    BossStatusStep() : BossBaseStep() {}
    virtual ~BossStatusStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_STATUS; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    ECreatureStatus m_eStatus = ECREATURESTATUS_MIN;
    DWORD m_dwNum = 0;
};

// BossStep - wait
class BossWaitStep : public BossBaseStep
{
  public:
    BossWaitStep() : BossBaseStep() {}
    virtual ~BossWaitStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_WAIT; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    DWORD m_dwTime = 0;
};

// BossStep - kill
class BossKillStep : public BossBaseStep
{
  public:
    BossKillStep() : BossBaseStep() {}
    virtual ~BossKillStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_KILL; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);

    bool isMonster(DWORD dwID) const { return m_setMonsterIDs.find(dwID) != m_setMonsterIDs.end(); }
  private:
    TSetDWORD m_setMonsterIDs;
    DWORD m_dwNum = 0;
};

// BossStep - world
class BossWorldStep : public BossBaseStep
{
  public:
    BossWorldStep() : BossBaseStep() {}
    virtual ~BossWorldStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_WORLD; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);

    DWORD getTime() const { return m_dwTime; }
  private:
    DWORD m_dwTime = 0;
};

// BossStep - show
class BossShowStep : public BossBaseStep
{
  public:
    BossShowStep() : BossBaseStep() {}
    virtual ~BossShowStep() {}

    virtual Cmd::EBossStep getType() const { return Cmd::EBOSSSTEP_SHOW; }

    virtual bool init(xLuaData data);
    virtual bool doStep(BossAct* pAct, SceneUser* pUser);
  private:
    enum EShowType
    {
      ESHOWTYPE_WORLD = 0,
    };
    EShowType m_eType = ESHOWTYPE_WORLD;
};

