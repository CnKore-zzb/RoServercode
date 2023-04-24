#pragma once

#include <set>
#include <string>
#include "xDefine.h"
#include "xDefine.h"
#include "xLuaTable.h"
#include "xTools.h"
#include "xSceneEntry.h"

using std::string;

class SceneNpc;

//NPC AIִ
enum AI_RET_ENUM
{
  AI_RET_COND_FAIL,
  AI_RET_COND_FAIL_CONT,
  AI_RET_RUN_FAIL,
  AI_RET_RUN_SUCC,
  AI_RET_RUN_SUCC_CONT,
  AI_RET_RUN_OK_BREAK,
};

// AICondition - base
class AICondition
{
  public:
    AICondition(const xLuaData& params) : m_oParams(params) { reset(); }
    virtual ~AICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();

    DWORD getWeight();
    void setName(const string& name) { m_strName = name; }
    const string& getName() const { return m_strName; }
    bool isNoRandExe() { return m_bNoRandExe; }

    void addCount() { m_dwCount ++; }
    bool checkOdds();
  protected:
    xLuaData m_oParams;
    string m_strName;

    DWORD m_dwNextTime = 0;
    DWORD m_dwCount = 0;

    bool m_bNoRandExe = false;
};

// AICondition - timer
class TimerAICondition : public AICondition
{
  public:
    TimerAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~TimerAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();
  private:
    QWORD timeFromBirth = 0;
};

// AICondition - birth
class BirthAICondition : public AICondition
{
  public:
    BirthAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~BirthAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();
};

// AICondition - hpless
class HPLessAICondition: public AICondition
{
  public:
    HPLessAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~HPLessAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();
  private:
    DWORD hpLess = 0;
    DWORD hpPercent = 0;
    DWORD hpMinPercent = 0;
};

// AICondition - chantlock  �ܵ�ʩ������
class ChantLockAICondition : public AICondition
{
  public:
    ChantLockAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~ChantLockAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - onbuff  �Լ�����ĳ��״̬ / buffЧ��
class OnBuffAICondition : public AICondition
{
  public:
    OnBuffAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~OnBuffAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - changehp
class HPChangeAICondition: public AICondition
{
  public:
    HPChangeAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~HPChangeAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();
  private:
    float m_fhpLessPercent = 0;
};

// AICondition - beattack
class BeAttackAICondition:public AICondition
{
  public:
    BeAttackAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~BeAttackAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();
};

class DeathAICondition:public AICondition
{
  public:
    DeathAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~DeathAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();
};

// AICondtion - kill user
class KillUserAICondition : public AICondition
{
  public:
    KillUserAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~KillUserAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - normal ״̬
class NormalAICondition : public AICondition
{
  public:
    NormalAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~NormalAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - attack ״̬
class AttackAICondition : public AICondition
{
  public:
    AttackAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~AttackAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
    virtual void reset();
private:
  DWORD hpLess = 0;
  DWORD hpPercent = 0;
  DWORD timeFromAttack = 0;
};

// AICondition - disp hide
class DispHideAICondition : public AICondition
{
  public:
    DispHideAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~DispHideAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - see dead
class SeeDeadAICondition : public AICondition
{
  public:
    SeeDeadAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~SeeDeadAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - buff layers
class BuffLayerAICondition : public AICondition
{
  public:
    BuffLayerAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~BuffLayerAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - camera shot
class CameraShotAICondition : public AICondition
{
  public:
    CameraShotAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~CameraShotAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - camera lock
class CameraLockAICondition : public AICondition
{
  public:
    CameraLockAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~CameraLockAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - weaponpet user die
class EmployerDieAICondition : public AICondition
{
  public:
    EmployerDieAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~EmployerDieAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - weaponpet user die event
class EmployerReliveAICondition : public AICondition
{
  public:
    EmployerReliveAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~EmployerReliveAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - weaponpet user die status
class EmployerDieStatusAICondition : public AICondition
{
  public:
    EmployerDieStatusAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~EmployerDieStatusAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
  private:
    bool m_bTimeAlreadyOk = false;
};

// AICondition - weaponpet user action event
class EmployerActionAICondition : public AICondition
{
  public:
    EmployerActionAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~EmployerActionAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - npc function
class NpcFunctionAICondition : public AICondition
{
  public:
    NpcFunctionAICondition(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~NpcFunctionAICondition() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondtion : check servants all die
class ServantDieAICondtion : public AICondition
{
  public:
    ServantDieAICondtion(const xLuaData& params) : AICondition(params) { reset(); }
    virtual ~ServantDieAICondtion() {}

    virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - visit  被点击访问了
class VisitAICondition : public AICondition
{
public:
  VisitAICondition(const xLuaData& params) : AICondition(params) { reset(); }
  virtual ~VisitAICondition() {}

  virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
  virtual void reset();
};

// AICondition - kill 击杀目标时
class KillMonsterAIConditon : public AICondition
{
public:
  KillMonsterAIConditon(const xLuaData& params) : AICondition(params) { reset(); }
  virtual ~KillMonsterAIConditon() {}

  virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

// AICondition - alert 范围内出现玩家/怪物时
class AlertAIConditon : public AICondition
{
public:
  AlertAIConditon(const xLuaData& params) : AICondition(params) { reset(); }
  virtual ~AlertAIConditon() {}

  virtual AI_RET_ENUM checkCondition(SceneNpc* pNpc);
};

//--------------------------------------------------------------AITarget
// AITarget - base
class xSceneEntry;
// typedef std::unordered_set<xSceneEntry *> xSceneEntrySet;
class AITarget
{
  public:
    AITarget(const xLuaData& params) { m_oParams = params;  }
    virtual ~AITarget() {}

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet) = 0;

    void setName(const string& name) { m_strName = name; }
    const string& getName() const { return m_strName; }
    DWORD getStep() const { return m_oParams.getTableInt("ai_step_index"); }
  protected:
    xLuaData m_oParams;
    string m_strName;
    //DWORD m_dwPercent = 0;
};

// AITarget - self
class SelfAITarget : public AITarget
{
  public:
    SelfAITarget(const xLuaData& params) : AITarget(params) {}
    virtual ~SelfAITarget() {}

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
};

// AITarget - range
class RangeAITarget : public AITarget
{
  public:
    RangeAITarget(const xLuaData& params);
    virtual ~RangeAITarget() {}

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
  private:
    DWORD m_dwRadius = 0;
    DWORD m_dwMinRadius = 0;
    DWORD m_dwCountMax = 0;
    DWORD m_fHpper = 0;
    TSetDWORD m_setNpcIds;
    TSetDWORD m_setStates;

    bool m_bSelectMinHSp = false;
    DWORD m_dwPriorExpression = 0;
    bool m_bSelectMinHp = false;
    bool m_bNeedCanLock = false;
    bool m_bMap = false;
    TVecDWORD m_vecPriorProfession;
};

// AITarget - attacker
class AttackMeAITarget : public AITarget
{
  public:
    AttackMeAITarget(const xLuaData& params);
    virtual ~AttackMeAITarget() {}
    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
};

// AITarget - lockme
class LockMeAITarget : public AITarget
{
  public:
    LockMeAITarget(const xLuaData& params);
    virtual ~LockMeAITarget() {};

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
};

// AITarget -  user killed
class UserKilledAITarget : public AITarget
{
  public:
    UserKilledAITarget(const xLuaData& params);
    virtual ~UserKilledAITarget() {};

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
};

// AITarget -  sprint
class SprintAITarget : public AITarget
{
  public:
    SprintAITarget(const xLuaData& params);
    virtual ~SprintAITarget() {};

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
};

// AITarget - current locking
class LockingAITarget : public AITarget
{
  public:
    LockingAITarget(const xLuaData& params);
    virtual ~LockingAITarget() {};

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
};

// AITarget - dead
class DeadAITarget : public AITarget
{
  public:
    DeadAITarget(const xLuaData& params);
    virtual ~DeadAITarget() {}

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
  private:
    DWORD m_dwRange = 0;
    std::set<DWORD> m_setNpcIds;
};

// AITarget - hand user
class HandUserAITarget : public AITarget
{
  public:
    HandUserAITarget(const xLuaData& params);
    virtual ~HandUserAITarget() {}

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);
};

// AITarget - user expression or action
class ActionUserAITarget : public AITarget
{
  public:
    ActionUserAITarget(const xLuaData& params);
    virtual ~ActionUserAITarget() {}

    virtual bool getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet);

  private:
    TSetDWORD m_setExpressions;
    TSetDWORD m_setActions;
};

// AITarget - weaponpet 's teamer
class WeaponPetUserAITarget : public AITarget
{
  public:
    WeaponPetUserAITarget(const xLuaData& params);
    virtual ~WeaponPetUserAITarget() {}

    virtual bool getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet);
  private:
    DWORD m_dwRange = 0;
};

// AITarget - master of being
class BeingMasterAITarget : public AITarget
{
  public:
    BeingMasterAITarget(const xLuaData& params);
    virtual ~BeingMasterAITarget() {}

    virtual bool getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet);
  private:
    TSetDWORD m_setBuff;
    bool m_bLockTarget = false;
    bool m_bMasterWhenLock = false;
};

// AITarget - using skill
class UsingSkillAITarget : public AITarget
{
  public:
    UsingSkillAITarget(const xLuaData& params);
    virtual ~UsingSkillAITarget() {}

    virtual bool getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet);
  private:
    TSetDWORD m_setSkillIDs;
    DWORD m_dwRange;
    bool m_bFirstChant = false;
};

// AITarget - visitor
class VisitorAITarget : public AITarget
{
  public:
    VisitorAITarget(const xLuaData& params);
    virtual ~VisitorAITarget() {}

    virtual bool getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet);
};

// AITarget : if satisfy dis, target include self and master, if not, target is empty
class NearMasterAITarget : public AITarget
{
  public:
    NearMasterAITarget(const xLuaData& params);
    virtual ~NearMasterAITarget() {}

    virtual bool getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet);

  private:
    float m_fNearRange = 0;
};

// AITarget : get targets between self and master
class BetweenMasterAITarget : public AITarget
{
  public:
    BetweenMasterAITarget(const xLuaData& params);
    virtual ~BetweenMasterAITarget() {}

    virtual bool getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet);

  private:
    float m_fWidth = 0;
};

// AITarget : 携带BUFF者
class BufferAITarget : public AITarget
{
  public:
    BufferAITarget(const xLuaData& params);
    virtual ~BufferAITarget() {}

    virtual bool getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet);

  private:
    DWORD m_dwBuffid = 0;
    DWORD m_dwRange = 0;
    DWORD m_dwNum = 0;
    string m_sType;
};

