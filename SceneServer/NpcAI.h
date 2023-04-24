#pragma once

#include "xSceneEntry.h"
#include "xStateMachine.h"
#include "NpcState.h"
#include "xPos.h"
#include "NpcConfig.h"
#include "xNoncopyable.h"

using std::set;

struct SNpcSkillItem;
class SceneNpc;
class SceneUser;
class NpcAI : private xNoncopyable
{
  public:
    NpcAI(SceneNpc *npc);
    ~NpcAI();

    void timer();
    void clear();

    bool normal();
    bool attack();
    bool pickup();
    bool wait();
    bool gocamera();
    bool naughty();
    bool smile();
    bool seedead();
    bool sleep();
    bool goback();
    bool runaway();

    void changeState(ENpcState type);

    bool switchPickTarget();
    bool switchAttackTarget(bool switchFlag = false);
    bool switchSleepTraget();
    bool searchAttackTarget(xSceneEntrySet &set, BYTE range, BYTE mode=0);
    xSceneEntryDynamic* getAttackTarget(xSceneEntrySet &set, BYTE mode, bool switchFlag);
    void removeAllyInEntryList(xSceneEntrySet &targetSet);
    void moveTo(xPos p);
    void moveToPos(float x, float y, float z);
    BYTE getSwitchTargetMode();
    xPos getSearchPos();
    DWORD getSearchRange();
    BYTE getSkillRange();
    void beAttack(xSceneEntryDynamic* attacker);

    void setCurLockID(QWORD id);
    QWORD getCurLockID();

    void setCurItemID(QWORD id);
    QWORD getCurItemID() const { return m_qwCurItemID; }

    void setLastAttacker(QWORD id) { m_qwLastAttacker = id; }
    QWORD getLastAttacker() const { return m_qwLastAttacker; }

    //void putAttacker(QWORD id) { m_attacklist.insert(id); }
    void putAttacker(xSceneEntryDynamic* pEntry);
    TSetQWORD& getAttackList() { return m_attacklist; }
    void setTauntUser(QWORD id) { m_qwTauntUser = id; if (id) setCurLockID(id); }
    QWORD getTauntUser() { return m_qwTauntUser; }
    QWORD getMasterTauntUser();

    DWORD getTerritory();
    bool isMoveable();
    bool isConfigCanMove();
    bool isAttackBack();
    bool isOutRangeBack();
    bool isPickup();
    bool isTeamAttack();
    bool isSwitchAttack();
    bool isSwitchBeAttack();
    bool isBeAttack1Max();
    bool isBeChantAttack();

    bool isGocamera();
    bool isSmileCamera();
    bool isAlertFakeDead();
    bool isExpelDead();
    bool isGhost();
    bool isDemon();
    bool isNaughty();
    bool isNotSkillSelect();
    bool isReckless();
    bool isFly();
    bool isAngerHand();
    bool isAttackGhost();
    bool isSleep();
    bool isStatusAttack();
    bool isDHide();
    bool isNightWork();
    bool isEndure();
    bool isNoTransformAtt();
    bool isImmuneTaunt();
    bool isFarSearch();

    void autoMove();
    bool checkRePathFinding(xPos pos);
    BYTE getReturnDis();
    void checkAutoBack();
    DWORD getPursueDis();
    bool checkPursueNeedBack();

    ENpcState getFormerState() const { return m_dwFormerState; }
    ENpcState getStateType() const { return m_dwStateType; }
    DWORD getAttackStartTime() const { return m_dwAttackStartTime; }
    void setStateType(ENpcState type) { m_dwFormerState = m_dwStateType; m_dwStateType = type; }

    void setWaitExitTime(DWORD exittime) { m_dwWaitExitTime = exittime; }
    DWORD getNextGocameraTime() const { return m_dwNextGocameraTime; }

    bool isInChangeAngle() const { return getFormerState() == ENPCSTATE_SMILE && getStateType() == ENPCSTATE_WAIT; }
    DWORD getAngle() const { return m_dwAngle; }

    void initAddBuff();
  public:
    bool isNight(time_t time);
  public:
    void onBeChantLock(QWORD attackerid);
    void onFocusByCamera(QWORD userid);
    void onSeeDead(QWORD userid);
    void onBreakUserSkill(QWORD userid) { m_breakUserChant = true; }
    void onSeeStatus(SceneUser* pUser, DWORD status);
  private:
    void checkNaughty(QWORD attackerid);
    void checkEndure(QWORD attackerid);
    // 技能
  private:
    bool processUseSkill();
    bool checkSkillCondition(const SNpcSkill& rItem);
    DWORD getAttackSkillID();
    void resetCurSkill();
    bool stateChange();
    void addCD();
  private:
    DWORD m_dwCurSkillID = 0;
    DWORD m_dwCurIndex = 0;

    DWORD m_dwNormalStartTime = 0;
    DWORD m_dwAttackStartTime = 0;

    ENpcState m_eStateChange = ENPCSTATE_MIN;
    ENpcState m_eSkillState = ENPCSTATE_MIN;

    bool fearRun();
    bool alertDead();
    bool expelDead();

    bool checkReaction(xSceneEntryDynamic* pEntry, EReactType eType);
    bool canNormalLock(xSceneEntryDynamic* pEntry);

    bool attackNeedGoBack(xSceneEntryDynamic* pLockTar);
  public:
    // 需要同步给客户端显示
    DWORD m_dwSearchRange = 0;

  public:
    void onUserLeaveNine(QWORD id) { m_setImmuneList.erase(id); m_setJealousList.erase(id); }

    void addImmuneID(QWORD id) { m_setImmuneList.insert(id); }
    bool hasImmuneID(QWORD id) { return m_setImmuneList.find(id) != m_setImmuneList.end(); }

    bool isInNightWork() { return m_bNightWork; }
    void setNightWork(bool flag) { m_bNightWork = flag; }

    DWORD getLastAttackTime() { return m_dwLastAttackTime; }
    DWORD getLastBeAttackTime() { return m_dwLastBeAttackTime; }
    DWORD getLastLockTime() { return m_qwLockTimeTick; }

    bool canAI2Attack(xSceneEntryDynamic* target);
  private:
    DWORD m_dwMaxAttackCount = 0;
    DWORD m_dwLastAttackTime = 0;

    QWORD m_qwCurLockID = 0;
    QWORD m_qwLockTimeTick = 0;
    QWORD m_qwCurItemID = 0;

    xStateMachine<SceneNpc>* fsm = nullptr;
    xTimer _three_sec;
    xTimer _one_sec;

    set<QWORD> m_attacklist;
    QWORD m_qwLastAttacker = 0;
    float m_flTargetDis = 0.0f;

    SceneNpc* m_pNpc = nullptr;

    DWORD m_dwNextSetPosTime = 0;
    xPos fearRunPos;

    bool m_blAttackState = false;

    ENpcState m_dwStateType = ENPCSTATE_NORMAL;
    ENpcState m_dwFormerState = ENPCSTATE_NORMAL;

    bool m_breakUserChant = false;

    DWORD m_dwWaitExitTime = 0;

    DWORD m_dwNextGocameraTime = 0;
    DWORD m_dwAngle = 0;
    DWORD m_dwNextSmileTime = 0;

    DWORD m_dwNextAlertTime = 0;
    DWORD m_dwNextExpelTime = 0;

    DWORD m_dwNextSearchTime = 0;
    set<QWORD> m_setImmuneList;

    DWORD m_dwLastBeAttackTime = 0;
    set<QWORD> m_setJealousList;

    DWORD m_dwNextSleepActionTime = 0;

    // 夜行
    bool m_bNightWork = false;
    // 开霸体cd
    DWORD m_dwCanEndureTime = 0;

    // 挑衅我的玩家
    QWORD m_qwTauntUser = 0;

  // 优先攻击职业、性别
  public:
    void priorAttackProfession(DWORD id) { m_setPriorProfession.insert(id); }
    void priorAttackGender(EGender eGender) { m_ePriorGender = eGender; }
    void clearPriorAttack() { m_setPriorProfession.clear(); m_ePriorGender = EGENDER_MIN; }
    void ignoreAttackProfession(DWORD id) { m_setIgnoreProfession.insert(id); }
    void clearIgnoreAttack() { m_setIgnoreProfession.clear(); }
  private:
    void verifyAttackList(xSceneEntrySet& set);
    TSetDWORD m_setPriorProfession;
    EGender m_ePriorGender = EGENDER_MIN;
    TSetDWORD m_setIgnoreProfession;
  // 优先攻击职业、性别

  // 记录最近锁定我的玩家和技能, 攻击我的玩家和技能
  public:
    const pair<QWORD, DWORD>& getLockMeUserSkill() { return m_paLockMeUserSkill; }
    const pair<QWORD, DWORD>& getAttMeUserSkill() { return m_paAttMeUserSkill; }
    void addLockMeUserSkill(QWORD userid, DWORD skillid) { m_paLockMeUserSkill.first = userid; m_paLockMeUserSkill.second = skillid; }
    void addAttMeUserSkill(QWORD userid, DWORD skillid) { m_paAttMeUserSkill.first = userid; m_paAttMeUserSkill.second = skillid; }
  private:
    pair<QWORD, DWORD> m_paLockMeUserSkill;
    pair<QWORD, DWORD> m_paAttMeUserSkill;
  // 记录最近锁定我的玩家和技能, 攻击我的玩家和技能
  // 记录我最近杀死的玩家
  public:
    void setMyKillUser(QWORD id) { m_qwKillUserID = id; }
    QWORD getMyKillUser() { return m_qwKillUserID; }
  private:
    QWORD m_qwKillUserID = 0;
  // 切换目标
  public:
    void changeTarget() { setCurLockID(0); m_bInChangeTarget = true; }
  private:
    QWORD m_qwLastLockID = 0;
    bool m_bInChangeTarget = false;
  // 逃跑
  public:
    void setRunAwayStopTime(DWORD time) { m_dwStopRunAwayTime = time; }
  private:
    DWORD m_dwStopRunAwayTime = 0;

  // 优先攻击某个玩家
  public:
    void setPriAttackUser(QWORD userid) { m_qwPriAttackUserID = userid; }
  private:
    QWORD m_qwPriAttackUserID = 0;

  public:
    void setPosSync() { m_bMarkPosNeedSync = true; }
  private:
    bool m_bMarkPosNeedSync = false;
  public:
    void onNpcDie();
};

