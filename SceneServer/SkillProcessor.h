/**
 * @file SkillProcessor.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-08-07
 */

#pragma once

#include <map>
#include <list>
#include <vector>
#include "SkillItem.h"
#include "xPos.h"
#include <queue>

using std::map;
using std::vector;
using std::set;
using std::queue;
using namespace Cmd;

#define TIME_SKILL_OFFSET_MSEC    2000
#define TIME_SKILL_RAND_ERR_PERSENT   50/100
#define CHANT_ERROR 100

class SceneUser;
class SceneNpc;
class SkillRunner
{
  friend class SkillProcessor;
  public:
    SkillRunner();
    ~SkillRunner();

    xSceneEntryDynamic* getEntry() const { return m_pEntry; }
    void setEntry(xSceneEntryDynamic* pEntry) { m_pEntry = pEntry; }

    const BaseSkill* getCFG() const { return m_pSkillCFG; }
    void setCFG(const BaseSkill* pCFG) { m_pSkillCFG = pCFG; }

    const xPos& getTargetPos() const { return m_oTargetPos; }
    void setTargetPos(const xPos& pos) { m_oTargetPos = pos; }

    void setState(ESkillState eState) { m_eState = eState; }
    ESkillState getState() const { return m_eState; }

    QWORD getStartTime() const { return m_qwStartTime; }
    void setStartTime(QWORD time) { m_qwStartTime = time; }

    QWORD getChantEndTime() const { return m_qwChantEndTime; }
    void setChantEndTime(QWORD time) { m_qwChantEndTime = time; }

    SkillBroadcastUserCmd& getCmd() { return m_oCmd; }
    void setCmd(const SkillBroadcastUserCmd& oCmd) { m_oCmd.CopyFrom(oCmd); }

    void setNextTime(QWORD time) { m_qwNextTime = time; }
    QWORD getNextTime() const { return m_qwNextTime; }

    void setCount(DWORD count) { m_dwCount = count; }
    DWORD getCount() const { return m_dwCount; }

    void setParam1(QWORD param1) { m_qwParam1 = param1; }
    QWORD getParam1() const { return m_qwParam1; }

    void setTrap(bool bTrap) { m_bTrap = bTrap; }
    bool getTrap() const { return m_bTrap; }

    void setBreak(bool bBreak) { m_bBreak = bBreak; }
    bool getBreak() const { return m_bBreak; }

    void setBreaker(QWORD attackerid) { m_dwBreaker = attackerid; }
    QWORD getBreaker() const { return m_dwBreaker; }

    void addBreakHp(DWORD dwDamage) { m_dwBreakHp += dwDamage; }
    DWORD getBreakHp() const { return m_dwBreakHp; }

    void setTrapTrigger() { m_bTrapTrigger = true; }
    bool isTrapTriggered() const { return m_bTrapTrigger; }

    void setArrowID(DWORD arrowid) { m_dwArrowID = arrowid; }
    DWORD getArrowID() const { return m_dwArrowID; }

    void setTransInfo(ETransportType eType, DWORD mapid = 0, const xPos* pTransPos = nullptr) { m_eTransType = eType; m_dwTransMapID = mapid; m_pTransPos = pTransPos; }
    DWORD getTransMap() const { return m_dwTransMapID; }
    ETransportType getTransType() const { return m_eTransType; }
    const xPos* getTransPos() const { return m_pTransPos; }

    QWORD getNextChantTime() const { return m_qwChantNextTime; }
    void setNextChantTime(QWORD time) { m_qwChantNextTime = time; }

    set<QWORD>& getTargets() { return m_setTargets; }
    set<QWORD>& getOldTargets() { return m_setOldTargets; }
    set<QWORD>& getBuffTargets() { return m_setBuffTargets; }

    void setNoTarget() { m_bNoTarget = true; }
    bool noTarget() { return m_bNoTarget; }

    void setValidBreak() { m_bValidBreak = true; }
    bool isValidBreak() const { return m_bValidBreak; }

    void setBuffLayer(DWORD buffid, DWORD layer) { m_mapBuff2Layers[buffid] = layer; }
    bool haveBuffLayer() { return m_mapBuff2Layers.empty() == false; }
    const map<DWORD, DWORD>& getBuffLayers() { return m_mapBuff2Layers; }

    void setLockTargetID(QWORD id) { m_qwLockTargetID = id; }
    QWORD getLockTargetID() { return m_qwLockTargetID; }

    void setDelStatus() { m_bDelStatus = true; }
    bool needDel() { return m_bDelStatus; }

    void timer(QWORD curTime);
    void reset();
    
    void setIsFirst(bool value) {m_bFirst = value;}
    bool isFirst() const {return m_bFirst;}

    void setEnsemblePartner(QWORD charid) { m_qwEnsemblePartner = charid; }
    QWORD getEnsemblePartner() { return m_qwEnsemblePartner; }
  private:
    void run(QWORD curTime);
    void end();
  private:
    xSceneEntryDynamic* m_pEntry = nullptr;
    const BaseSkill* m_pSkillCFG = nullptr;

    SkillBroadcastUserCmd m_oCmd;
    set<QWORD> m_setBuffTargets;

    xPos m_oTargetPos;

    ESkillState m_eState = ESKILLSTATE_MIN;

    QWORD m_qwStartTime = 0;
    QWORD m_qwNextTime = 0;
    QWORD m_qwChantEndTime = 0;

    DWORD m_dwCount = 0;
    DWORD m_dwBreakHp = 0;

    QWORD m_qwParam1 = 0;

    bool m_bTrap = false;
    bool m_bBreak = false;
    bool m_bValidBreak = false;
    QWORD m_dwBreaker = 0;

    // 陷阱引爆
    bool m_bTrapTrigger = false;

    DWORD m_dwArrowID = 0;

    set<QWORD> m_setTargets;
    set<QWORD> m_setOldTargets;

    ETransportType m_eTransType = ETRANSPORTTYPE_MIN;
    DWORD m_dwTransMapID = 0;
    const xPos* m_pTransPos = nullptr;

    QWORD m_qwChantNextTime = 0;
    bool m_bNoTarget = false; // 需要服务端选择技能目标

    map<DWORD, DWORD> m_mapBuff2Layers; // 消耗的buff以及层数, 记录后buff删除, 用于后续计算

    QWORD m_qwLockTargetID = 0;

    bool m_bDelStatus = false;

    bool m_bFirst = true; //是否是第一次执行

    QWORD m_qwEnsemblePartner = 0;
};
typedef std::list<SkillRunner> TListSkillRunner;

enum ESkillQueueType
{
  ESKILLQUEUETYPE_MIN = 0,
  ESKILLQUEUETYPE_NORMALSKILL = 1,
  ESKILLQUEUETYPE_CHANT = 2,
  ESKILLQUEUETYPE_CD = 3,
  ESKILLQUEUETYPE_WAITSTATE = 4,
  ESKILLQUEUETYPE_SKILLDIS = 5,
};

struct SSkillQueueAtom
{
  ESkillQueueType eType = ESKILLQUEUETYPE_MIN;
  QWORD qwTimeOut = 0;
  SkillBroadcastUserCmd oSkillCmd;
  QWORD qwTimeIn = 0;
};
typedef queue<SSkillQueueAtom> TQueueSkillCmd;

typedef pair<QWORD, SkillBroadcastUserCmd> TPairTime2SkillCmd;
typedef vector<TPairTime2SkillCmd> TVecSkillQueue;

enum EBreakSkillType
{
  EBREAKSKILLTYPE_HP = 1,
  EBREAKSKILLTYPE_BUFF = 2,
  EBREAKSKILLTYPE_SYSTEM = 3,
  EBREAKSKILLTYPE_SKILL = 4,
  EBREAKSKILLTYPE_DEAD = 5,
};

enum ECheckDisResult
{
  ECHECKDISRESULT_MIN = 0,
  ECHECKDISRESULT_FAIL = 1,
  ECHECKDISRESULT_DISFAIL = 2,
  ECHECKDISRESULT_SUCCESS = 3,
};

struct SNpcSkillQuene
{
  DWORD dwSkillID = 0;
  QWORD qwTargetID = 0;
  bool bSpecPos = false;
  xPos oSkillPos;
};
typedef queue<SNpcSkillQuene> TQueueNpcSkill;

struct SSkillMoveCheckData
{
  bool bWaitCheck = false;
  DWORD dwSkillID = 0;
  float fMoveDistance = 0;
  xPos oOriPos;
  pair<DWORD, DWORD> oPairAttMoveCheck; // 总次数,瞬移次数
  bool checkScenePos() const { return oPairAttMoveCheck.second * 3 <= oPairAttMoveCheck.first || oPairAttMoveCheck.first <= 5; }
  void clear()
  {
    bWaitCheck = false;
    oPairAttMoveCheck.first = oPairAttMoveCheck.second = 0;
  }
};

class SkillProcessor
{
  public:
    SkillProcessor(xSceneEntryDynamic* pEntry);
    ~SkillProcessor();

    const SkillRunner& getRunner() const { return m_oRunner; }
    const SkillBroadcastUserCmd& getCurCmd() { return m_oRunner.getCmd(); }

    bool setActiveSkill(const SkillBroadcastUserCmd& oCmd, bool bFromQueue = false);
    bool useBuffSkill(xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy, DWORD skillid, bool isActive, bool free = false, bool ignoreCD = false);
    bool breakSkill(EBreakSkillType eType, QWORD qwBreakerID, DWORD param = 0, bool bFromClient = false);
    bool breakSkill(QWORD qwBreakerID, bool bFromClient = false) { return breakSkill(EBREAKSKILLTYPE_SYSTEM, qwBreakerID, 0, bFromClient); }
    void timer(QWORD curTime);
    void clear();
    void setClearState() { m_bClear = true; }

    DWORD getLastUseTime(DWORD skillid) const;
    void addSkillUseInfo(DWORD skillid, QWORD time);

    void showSkill(DWORD skillid);
    bool checkActionTime(QWORD curm) { return curm >= m_qwNextActionTime; }
    ECheckDisResult checkSkillDistance(const SkillBroadcastUserCmd& oCmd);

    void triggerAllTrap();
    void triggerTrap(QWORD id);
    void triggerTrapByType(const TSetDWORD& setNpcid);

    void runSkillAtonce(xSceneEntryDynamic* pTarget, DWORD skillid);
    void useTransportSkill(SceneUser* user, ETransportType eType, DWORD mapid = 0, const xPos* pTransPos = nullptr);

    void addBeSkillTime(DWORD skillGroup, QWORD nexttime);
    QWORD getNextBeSkillTime(DWORD skillGroup);

    void clearActionTime() { m_qwNextActionTime = 0; }
    void addServerTrigSkill(DWORD skillid) { m_setServerTrigSkills.insert(skillid); }

    const pair<DWORD, DWORD>& getNormalQueueCnt() const { return m_oNormalQueueCnt; }
    const pair<DWORD, DWORD>& getNoNormalQueueCnt() const { return m_oNoNormalQueueCnt; }
    void clearQueueCnt() { m_oNormalQueueCnt.first = 0; m_oNormalQueueCnt.second = 0; m_oNoNormalQueueCnt.first = 0; m_oNoNormalQueueCnt.second = 0; }
    void clearTransInfo() { m_oRunner.setTransInfo(ETRANSPORTTYPE_MIN, 0); }

    bool isTrapHoldOn() { return m_bTrapHoldOn; }
    void setTrapHoldOn(bool hold) { m_bTrapHoldOn = hold; }

    void getTrapNpcs(DWORD familySkillID, TSetQWORD& setnpc) const;
    void getAllTrapNpcs(std::set<SceneNpc*>& npcset) const;
    void onSkillNpcDie(QWORD npcguid);

    void addQueueNpcSkill(DWORD skillid, QWORD targetid, xPos pos, bool specpos);

    void onTriggerNpc(QWORD npcguid, SceneUser* user);

    void setValidBreak() { m_oRunner.setValidBreak(); }
    void setBuffLayer(DWORD id, DWORD layer) { m_oRunner.setBuffLayer(id, layer); }

    void delTrapSkill(QWORD trapnpcid);

    void addSkillMove(DWORD skillid, float distance, const xPos& curPos);
    void checkSkillMove(DWORD skillid, const ScenePos& pos);
    void endSkill(DWORD skillid);
  private:
    void runMainRunner(QWORD curTime, SkillRunner& runner);
    void runSubRunner(QWORD curTime);
    void addCD(const BaseSkill* pSkillCFG);
    void addCDBeforeChant(const BaseSkill* pSkillCFG);
    void addCDBeforeRun(const BaseSkill* pSkillCFG);

    void addActionTime(DWORD time);
    void limitSkillNum(const BaseSkill* pSkillCFG);
    void sendBreakSkill(DWORD skillid);
    bool isTrapThisType(DWORD tempid, const TSetDWORD& setNpcid); // 判断陷阱是否属于npcid的这一类
    xSceneEntryDynamic* getOneTarget(const SkillBroadcastUserCmd& oCmd);
  private:
    SkillRunner m_oRunner;
    SkillRunner m_oHelpRunner;

    TListSkillRunner m_listRunner;
    map<DWORD, QWORD> m_mapSkill2Time;
    QWORD m_qwNextActionTime = 0;
    bool m_bClear = false;

    TVecSkillQueue m_vecSkillQuene;
    QWORD m_qwInvalidAttackID = 0;
    DWORD m_dwInvalidTimes = 0;
    map<DWORD, QWORD> m_mapBeSkill2Time;

    TQueueSkillCmd m_oQueueCmd;

    bool m_bQueueStillQueue = false;
    //DWORD m_dwSkillDisErrCnt = 0;
    TSetDWORD m_setServerTrigSkills;
    //DWORD m_bLastSkillIsNormal = false;

    pair<DWORD, DWORD> m_oNormalQueueCnt;
    pair<DWORD, DWORD> m_oNoNormalQueueCnt;
    map<DWORD, QWORD> m_mapBuffSkillCD;

    bool m_bTrapHoldOn = false;
    TListSkillRunner m_listExtraRunner;

    TQueueNpcSkill m_oQueueNpcSkill;

    SSkillMoveCheckData m_stMoveCheckData;
};

