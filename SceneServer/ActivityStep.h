#pragma once

#include "ActivityConfig.h"
#include "xPos.h"
#include "SceneDefine.h"

using std::pair;
using std::map;

class ActivityBase;

//执行结果
enum EACTIVITY_RET
{
  EACTIVITY_RET_ERROR = 1,      
  EACTIVITY_RET_CONTINUE = 2,
  EACTIVITY_RET_OK = 3,
};

class ActivityNodeBase;
//stage
class ActivityStageBase
{
public:
  ActivityStageBase(const xLuaData& params) : m_oParams(params)
  { reset(); }
  virtual ~ActivityStageBase();

  EACTIVITY_RET run(DWORD curSec, ActivityBase* pActivity);
  void reset();
  void addNote(EACTNODE_TYPE nodeType, ActivityNodeBase*pNode);
  void setName(const string& name) { m_strName = name; }
  const string& getName() const { return m_strName; }
  virtual bool isLoopStage() const { return false; }

public:
  bool isCondtionStage() const { return m_oParams.has("master_stage"); }
  DWORD getMasterStage() const { return m_oParams.getTableInt("master_stage"); }
  DWORD getIndex() { return m_oParams.getTableInt("stage_index"); }
  void enable() { m_bEnable = true; }
  bool isEnabled() const { return m_bEnable; }
  bool isOver() const { return m_lastSeqPos == m_seqNode.end(); }
  bool sonStageOver(ActivityBase* pActivity);

  void setResetStatus() { m_bReset = true; }
  bool needReset() { return m_bReset; }
  void resetData();

public:
  DWORD m_duration = 0;
  DWORD m_expireTime = 0;
protected:
  xLuaData m_oParams;
  string m_strName;
  bool m_bEnable = false;
  bool m_bReset = false;

  std::vector<ActivityNodeBase*> m_seqNode;
  std::vector<ActivityNodeBase*> m_parNode;
  std::vector<ActivityNodeBase*>::iterator m_lastSeqPos;
};

class ActivityStageLoop : public ActivityStageBase
{
  public:
    ActivityStageLoop(const xLuaData& params) : ActivityStageBase(params) {}
  public:
    virtual bool isLoopStage() const { return true; }
};

//node
class ActivityNodeBase
{
  public:
    ActivityNodeBase(const xLuaData& params) : m_oParams(params)
    { reset(); }
    virtual ~ActivityNodeBase() {}

    EACTIVITY_RET run(DWORD curSec, ActivityBase* pActivity);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity) { return EACTIVITY_RET_OK; }
    void reset();
    void setName(const string& name) { m_strName = name; }
    const string& getName() const { return m_strName; }
    void resetData() { m_count = 0; m_dwEndTime = 0; m_dwNextTime = 0; }
    void enable(ActivityBase* pActivity, DWORD stage) {};
    bool sonStageOver(ActivityBase* pActivity);
  protected:
    bool bSelectNode() { return m_dwOkStage || m_dwFailStage; }
  protected:
    xLuaData m_oParams;
    string m_strName;

    DWORD m_dwInterval = 0;
    int m_limitCount = 0;

    DWORD m_dwNextTime = 0;
    int m_count = 0;

    DWORD m_dwEndTime = 0;
    DWORD m_dwOkStage = 0;
    DWORD m_dwFailStage = 0;
};


//name:notify
class ActivityNodeNotify:public ActivityNodeBase
{
public:
  ActivityNodeNotify(const xLuaData& params) : ActivityNodeBase(params)
  { reset(); }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();

private:
  DWORD m_dwMsgId;
  DWORD m_dwEffectId;
  std::string m_ntfRange;
};

//name:wait
class ActivityNodeWait :public ActivityNodeBase
{
public:
  ActivityNodeWait(const xLuaData& params) : ActivityNodeBase(params)
  {
  }
};


class ActivityBaseSummon :public ActivityNodeBase
{
public:

  ActivityBaseSummon(const xLuaData& params) : ActivityNodeBase(params)
  {
    reset();
  }

  DWORD calcNeedNpcCount(ActivityBase* pActivity);
  void reset();

protected:
  TVecDWORD m_vecIds;
  DWORD m_dwCount = 0;
  DWORD m_dwPosNpc = 0;
  DWORD m_dwPosRange = 0;
  DWORD m_dwPercent = 0;
  DWORD m_dwMax = 0;
  DWORD m_dwMin = 0;
  DWORD m_dwAllMax = 0;
  DWORD m_dwAllMin = 0;
  DWORD m_dwAllPercent = 0;
  NpcDefine m_npcDef;
  std::map<DWORD/*id*/, xPos> m_pos;
};

//name:summon
class ActivityNodeSummon :public ActivityBaseSummon
{
public:
  ActivityNodeSummon(const xLuaData& params) : ActivityBaseSummon(params)
  { reset(); }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();

private:
  //DWORD m_dwCount;
  //DWORD m_dwPosNpc = 0;
  //DWORD m_dwPosRange = 0;
  //DWORD m_dwPercent = 0;
  //std::map<DWORD/*id*/, xPos> m_pos;
  //NpcDefine m_npcDef;
  //DWORD m_dwWeight = 0;
  //DWORD m_dwAllMax = 0;
};

////name:randomsummon
//class ActivityNodeRandomSummon :public ActivityBaseSummon
//{
//public:
//  ActivityNodeRandomSummon(const xLuaData& params) : ActivityBaseSummon(params)
//  {
//    reset();
//  }
//  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
//  void reset();
//
//private:
//  TVecDWORD m_vecIds;
//  DWORD m_dwCount = 0;
//  DWORD m_dwPosNpc = 0;
//  DWORD m_dwPosRange = 0;
//  DWORD m_dwPercent = 0;
//  DWORD m_dwMax = 0;
//  DWORD m_dwMin = 0;
//  DWORD m_dwAllMax = 0;
//  NpcDefine m_npcDef;
//};

//name:attacksummon
class ActivityNodeAttackSummon :public ActivityBaseSummon
{
public:
  ActivityNodeAttackSummon(const xLuaData& params) : ActivityBaseSummon(params)
  {
    reset();
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();
};

//name:checknpcnum
class ActivityNodeCheckNpcNum :public ActivityNodeBase
{
public:
  ActivityNodeCheckNpcNum(const xLuaData& params) : ActivityNodeBase(params)
  {
    reset();
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();

private:
  TVecDWORD m_vecIds;
  DWORD m_dwMinCount;
};

//name:questsummon
class ActivityNodeQuestSummon :public ActivityNodeBase
{
public:
  ActivityNodeQuestSummon(const xLuaData& params) : ActivityNodeBase(params)
  {
    reset();
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();

private:
  DWORD m_dwNpcId = 0;
  DWORD m_dwWeight = 0; //随机权重 100
  DWORD m_dwDuration = 0;
};

//name:progress     //活动状态同步
class ActivityNodeProgress :public ActivityNodeBase
{
public:
  ActivityNodeProgress(const xLuaData& params) : ActivityNodeBase(params)
  {
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
};

//name:checkend   //检查是否结束
enum ECHECKTYPE
{
  ECHECKTYPE_NONE = 0,
  ECHECKTYPE_NPCDIE = 1,
  ECHECKTYPE_MONEYCAT = 2,
};
class ActivityNodeCheckEnd :public ActivityNodeBase
{
public:
  ActivityNodeCheckEnd(const xLuaData& params) : ActivityNodeBase(params)
  {
    reset();
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();
private:
  ECHECKTYPE m_type = ECHECKTYPE_NONE;
  DWORD m_dwParam1 = 0;
  DWORD m_dwProgess = 0;
  DWORD m_dwMsgId = 0;
  std::string m_ntfRange;
};

//name:move   //npc 移动
class ActivityNodeMove :public ActivityNodeBase
{
public:
  ActivityNodeMove(const xLuaData& params) : ActivityNodeBase(params)
  {
    reset();
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();
private:
  DWORD m_dwNpcId = 0;
  std::map<DWORD/*mapid*/, std::vector<xPos>> m_pos;
  DWORD m_index = 0;
};

//name:turnmonster   
class ActivityTurnMonster :public ActivityNodeBase
{
public:
  ActivityTurnMonster(const xLuaData& params) : ActivityNodeBase(params)
  {
    reset();
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  void reset();

private:
  NpcDefine m_npcDef;
  DWORD m_dwOldNpc = 0;
};

//name:kill   //kill npc
class ActivityNodeKill :public ActivityNodeBase
{
public:
  ActivityNodeKill(const xLuaData& params) : ActivityNodeBase(params)
  {
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
};

//name:scenegm
class ActivityNodeSceneGm :public ActivityNodeBase
{
public:
  ActivityNodeSceneGm(const xLuaData& params) : ActivityNodeBase(params)
  {
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
};

//name:npcgm
class ActivityNodeNpcGm :public ActivityNodeBase
{
public:
  ActivityNodeNpcGm(const xLuaData& params) : ActivityNodeBase(params)
  {
  }
  virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
};

// mon
class ActivityNodeMoneyHit : public ActivityNodeBase
{
  public:
    ActivityNodeMoneyHit(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    DWORD m_dwNpcID = 0;
    DWORD m_dwStep = 0;

    DWORD m_dwLayerBuff = 0;
    DWORD m_dwLayerLimit = 0;

    DWORD m_dwSkillID = 0;
    DWORD m_dwDizzyBuff = 0;

    DWORD m_dwRange = 0;
    TSetDWORD m_setExpressions;

    DWORD m_dwMsgID = 0;
};

class ActivityNodeNormalSummon : public ActivityNodeBase
{
  public:
    ActivityNodeNormalSummon(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  protected:
    std::map<DWORD, std::pair<NpcDefine, DWORD>> m_mapMapID2Npc;
};

class ActivityNodeNotifyMoneyHit : public ActivityNodeBase
{
  public:
    ActivityNodeNotifyMoneyHit(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    DWORD m_dwMsgID = 0;
};

class ActivityCheckNpcDie : public ActivityNodeBase
{
  public:
    ActivityCheckNpcDie(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    TSetDWORD m_setNpcIDs;
};

class ActivityCheckNpcNum : public ActivityNodeBase
{
  public:
    ActivityCheckNpcNum(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    DWORD m_dwNpcID = 0;
    DWORD m_dwNum = 0;
};

class ActivityCheckStage : public ActivityNodeBase
{
  public:
    ActivityCheckStage(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    TSetDWORD m_setStages;
};

class ActivityHaveBuff: public ActivityNodeBase
{
  public:
    ActivityHaveBuff(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    TSetDWORD m_setNpcIDs;
    DWORD m_dwBuffID = 0;
};

class ActivityMarkRecordNpc : public ActivityNodeBase
{
  public:
    ActivityMarkRecordNpc(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    TSetDWORD m_setNpcIDs;
};

class ActivityKillCnt: public ActivityNodeBase
{
  public:
    ActivityKillCnt(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    DWORD m_dwNpcID = 0;
    DWORD m_dwCnt = 0;
};

class ActivityPlayDialog : public ActivityNodeBase
{
  public:
    ActivityPlayDialog(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
};

class ActivitySupplySummon : public ActivityNodeNormalSummon
{
  public:
    ActivitySupplySummon(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    map<DWORD, DWORD> m_mapMap2LimitCnt;
};

class ActivityNodeTimeSummon : public ActivityNodeNormalSummon
{
  public:
    ActivityNodeTimeSummon(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    map<DWORD, DWORD> m_mapMap2WeekLimit;
    DWORD m_dwWeekLoopCnt = 0;
};

class ActivitySpecialEffect: public ActivityNodeBase
{
  public:
    ActivitySpecialEffect(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    DWORD m_dwDramaID = 0;
    DWORD m_dwOpen = 0;
};

class ActivityCheckNpcHp : public ActivityNodeBase
{
  public:
    ActivityCheckNpcHp(const xLuaData& params);
    virtual EACTIVITY_RET v_run(DWORD curSec, ActivityBase* pActivity);
  private:
    DWORD m_dwNpcID = 0;
};

