#pragma once

#include "xLuaTable.h"
#include "xPos.h"
#include "FuBenCmd.pb.h"

using std::string;
using std::vector;
using std::pair;
using namespace Cmd;

class DScene;
class SceneUser;

enum FuBenResult
{
  FUBEN_RESULT_NULL = 0,
  FUBEN_RESULT_SUCCESS = 1,
  FUBEN_RESULT_FAIL = 2,
};

struct FuBenPhaseVar
{
  // table表的id
  DWORD m_dwID = 0;
  // 追踪
  string m_track;
  // npc计数
  DWORD m_dwKillNpcNum = 0;
  TSetDWORD m_oKillNpcIDSet;
  DWORD m_dwGroupID = 0;
  DWORD m_dwKillUniqueID = 0;
  // 计时器
  DWORD m_dwDelayTimer = 0;

  // 研究所是否开启
  bool m_blStart = false;

  FuBenPhaseVar() {}
};

// 倒计时
enum ETimerDownState
{
  ETIMERDOWNSTATE_STOP,
  ETIMERDOWNSTATE_RUN,
  ETIMERDWONSTATE_PASS
};

struct STimerDownState
{
  STimerDownState()
  {
    eTimerState = ETIMERDOWNSTATE_STOP;
    dwTimerSec = 10;         // default
    dwRefreshTimeSec = 0;
  }
  void reset()
  {
    eTimerState = ETIMERDOWNSTATE_STOP;
    dwTimerSec = 10;         // default
    dwRefreshTimeSec = 0;
  }

  ETimerDownState eTimerState;
  DWORD dwTimerSec;
  DWORD dwRefreshTimeSec;
};

struct SFuBenCarrierInfo
{
  DWORD dwCarrierID = 0;
  QWORD qwMasterID = 0;

  SFuBenCarrierInfo() {}
};

// fuben phase
class FuBenPhase
{
  public:
    FuBenPhase(const xLuaData &d) : m_data(d) {}
    virtual ~FuBenPhase() {}

    virtual bool exec(DScene *s) = 0;
    virtual bool isNotify() { return false; }
    virtual bool init();
    virtual bool isSyncClient() { return false; }

    bool isType(const string& t) const { return strcmp(t.c_str(), m_type.c_str()) == 0; }
    const char* getTypeString() const { return (const char *)(m_type.c_str()); }
    DWORD getStarID() const { return m_dwPhaseStarID; }
    void setID(DWORD dwID) { m_dwID = dwID; }
    DWORD getID() const { return m_dwID; }
    void syncDelThisStep(SceneUser* user);

    void initPConfig(RaidPConfig* pConfig);
  protected:
    bool checkClient(DScene* s);
  protected:
    xLuaData m_data;

    DWORD m_dwID = 0;   // table表的id
    DWORD m_dwPhaseStarID = 0;
    string m_type;
};

// story phase
class StoryFuBenPhase : public FuBenPhase
{
  public:
    StoryFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~StoryFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    //DWORD m_dwStoryID = 0;
    //DWORD m_dwTrigRange = 0;
    xPos m_oTrigPos;
    xPos m_oTrigEndPos;
};

// rush phase
class RushFuBenPhase : public FuBenPhase
{
  public:
    RushFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~RushFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    //DWORD m_dwRushID = 0;
    //DWORD m_dwTrigRange = 0;
    xPos m_oTrigPos;
};

// track phase
class TrackFuBenPhase : public FuBenPhase
{
  public:
    TrackFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~TrackFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    string m_track;
};

// killall phase
class KillAllFuBenPhase : public FuBenPhase
{
  public:
    KillAllFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~KillAllFuBenPhase() {}

    virtual bool isNotify() { return true; }
    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    TSetDWORD m_oNpcIDSet;
    TVecDWORD m_vecGroupID;
    TSetDWORD m_setRandGroupIDs;
};

// reward phase
class RewardFuBenPhase : public FuBenPhase
{
  public:
    RewardFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~RewardFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();

  private:
    DWORD m_dwRewardID;
    bool m_blLeave;
};

// summon phase
class SummonFuBenPhase : public FuBenPhase
{
  public:
    SummonFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~SummonFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwGroupID = 0;
    DWORD m_dwPurify = 0;
};

// randsummon phase
class RandSummonFuBenPhase : public FuBenPhase
{
  public:
    RandSummonFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~RandSummonFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwGroupID = 0;
};

// uniqrandsummon phase
class UniqRandSummonFuBenPhase : public FuBenPhase
{
  public:
    UniqRandSummonFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~UniqRandSummonFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwGroupID = 0;
};

// win phase
class WinFuBenPhase : public FuBenPhase
{
  public:
    WinFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~WinFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwStageID = 0;
    DWORD m_dwStageSubID = 0;
    DWORD m_dwEffect = 0;
};

// lose phase
class LoseFuBenPhase : public FuBenPhase
{
  public:
    LoseFuBenPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~LoseFuBenPhase() {}

    virtual bool exec(DScene* pScene);
    virtual bool init();
};

// leave phase
class LeaveFuBenPhase : public FuBenPhase
{
  public:
    LeaveFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~LeaveFuBenPhase(){}

    virtual bool exec(DScene *s);
    virtual bool init();
};

// kill phase
class KillFuBenPhase : public FuBenPhase
{
  public:
    KillFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~KillFuBenPhase() {}
    virtual bool isNotify() { return true; }

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    TSetDWORD m_oNpcIDSet;
    DWORD m_dwGroupID = 0;
    DWORD m_dwNum = 0;
};

// clearnpc phase
class ClearNpcFuBenPhase : public FuBenPhase
{
  public:
    ClearNpcFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~ClearNpcFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    TSetDWORD m_oNpcIDSet;
    DWORD m_dwGroupID = 0;

    enum EClearMethod
    {
      ECLEARMETHOD_LEAVE = 0,
      ECLEARMETHOD_DEAD = 1,
    };
    EClearMethod m_eMethod = ECLEARMETHOD_LEAVE;
    bool m_bEffect = false;
    xLuaData m_oEffect;
};

// timer phase
class TimerFuBenPhase : public FuBenPhase
{
  public:
    TimerFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~TimerFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwDelayTime = 0;
    DWORD m_dwTipFlag = 0;
    DWORD m_dwNotifyID = 0;
    DWORD m_dwQuestID = 0;
    bool m_bJustNeedOne = false;
};

// close phase
class CloseFuBenPhase : public FuBenPhase
{
  public:
    CloseFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~CloseFuBenPhase(){}

    virtual bool exec(DScene *s);
    virtual bool init();
};

// visit phase
class VisitFuBenPhase : public FuBenPhase
{
  public:
    VisitFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~VisitFuBenPhase(){}

    virtual bool isNotify() { return true; }
    virtual bool exec(DScene *s);
    virtual bool isSyncClient() { return true; }
};

// star phase
class StarFuBenPhase : public FuBenPhase
{
  public:
    StarFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~StarFuBenPhase(){}

    virtual bool isNotify() { return true; }
    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwStarID = 0;
};

// gm phase
class GMCmdFuBenPhase : public FuBenPhase
{
  public:
    GMCmdFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~GMCmdFuBenPhase(){}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    xLuaData m_oGmCmd;
};

// multi gm phase
class MultiGMFuBenPhase : public FuBenPhase
{
  public:
    MultiGMFuBenPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~MultiGMFuBenPhase() {}

    virtual bool exec(DScene* s);
    virtual bool init();
  private:
    xLuaData m_oGmCmd;
};

// npc gm
class NpcGMFuBenPhase : public FuBenPhase
{
  public:
    NpcGMFuBenPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~NpcGMFuBenPhase() {}

    virtual bool exec(DScene* s);
    virtual bool init();

  private:
    xLuaData m_oGmCmd;
};

// move phase
class MoveFuBenPhase : public FuBenPhase
{
  public:
    MoveFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~MoveFuBenPhase(){}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    xPos m_oPos;
    DWORD m_dwDistance = 0;
};

// gear phase
class GearFuBenPhase : public FuBenPhase
{
  public:
    GearFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~GearFuBenPhase(){}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwGearID = 0;
    DWORD m_dwStateID = 0;
};

// wait phase
class WaitStarFuBenPhase : public FuBenPhase
{
  public:
    WaitStarFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~WaitStarFuBenPhase(){}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwStarID = 0;
};

// append phase
class AppendFuBenPhase : public FuBenPhase
{
  public:
    AppendFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~AppendFuBenPhase(){}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwMapRaid = 0;
};

// enter next layer phase
class EnterLayerPhase : public FuBenPhase
{
  public:
    EnterLayerPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~EnterLayerPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
};

// Tower reward phase
class TowerRewardPhase : public FuBenPhase
{
  public:
    TowerRewardPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~TowerRewardPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();

    static void rollAllReward(SceneUser* pUser, TVecItemInfo& vecItemInfo, const vector<pair<DWORD, float>> vecRewardIDs, DWORD layer);
};

// tower timer phase
class TowerTimerPhase : public FuBenPhase
{
  public:
    TowerTimerPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~TowerTimerPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();

  private:
    xPos m_oExitPos;
    DWORD m_dwDistance = 0;
};

// tower summon phase
class TowerSummonPhase : public FuBenPhase
{
  public:
    TowerSummonPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~TowerSummonPhase() {};

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwGroupID = 0;
};

// tower killAll phase
class TowerKillAllPhase : public FuBenPhase
{
  public:
    TowerKillAllPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~TowerKillAllPhase() {};

    virtual bool exec(DScene *s);
    virtual bool init();
};

// player num phase
class PlayerNumPhase : public FuBenPhase
{
  public:
    PlayerNumPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~PlayerNumPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwNum = 0;
};

// carrier phase
class CarrierFuBenPhase : public FuBenPhase
{
  public:
    CarrierFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~CarrierFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    string m_sType;
    string m_sGMCmd;
};

// timer down phase
class TimerDownPhase : public FuBenPhase
{
  public:
    TimerDownPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~TimerDownPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    DWORD m_dwTimeSec = 0;
};

// change monster character phase
class ChangeMonsterCharacPhase : public FuBenPhase
{
  public:
    ChangeMonsterCharacPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~ChangeMonsterCharacPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
};

// laboratory monster phase
class LaboratoryMonsterPhase : public FuBenPhase
{
  public:
    LaboratoryMonsterPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~LaboratoryMonsterPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  public:
    DWORD m_dwRoundID = 0;
};

// monster count phase
class MonsterCountPhase : public FuBenPhase
{
  public:
    MonsterCountPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~MonsterCountPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  public:
    static void notify(DScene *scene, bool isClose=false);
};

// beattack phase
class BeAttackPhase : public FuBenPhase
{
  public:
    BeAttackPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~BeAttackPhase() {}

    virtual bool exec(DScene* s);
    virtual bool init();
  private:
    DWORD m_dwBeAttackCount = 0;
};

// Dojo reward phase
class DojoRewardPhase : public FuBenPhase
{
  public:
    DojoRewardPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~DojoRewardPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    void getActivityEventExtraReward(SceneUser* user, DWORD dojoid, TVecItemInfo& rewards);
};

// Dojo summon phase --DojoSummon
class DojoSummonPhase : public FuBenPhase
{
public:
  DojoSummonPhase(const xLuaData &d) : FuBenPhase(d) {}
  virtual ~DojoSummonPhase() {};

  virtual bool exec(DScene *s);
  virtual bool init();
private:
  DWORD m_dwGroupID = 0;
};

// dialog
class DialogFuBenPhase : public FuBenPhase
{
  public:
    DialogFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~DialogFuBenPhase(){}

    virtual bool isNotify() { return true; }
    virtual bool exec(DScene *s);
    virtual bool isSyncClient() { return true; }
};

// collect phase
class CollectFuBenPhase : public FuBenPhase
{
  public:
    CollectFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~CollectFuBenPhase() {}

    virtual bool isNotify() { return true; }
    virtual bool exec(DScene *s);
    virtual bool init();
    virtual bool isSyncClient() { return true; }
  private:
    TSetDWORD m_setGroupIDs;
};

// client plot
class ClientPlotFuBenPhase : public FuBenPhase
{
  public:
    ClientPlotFuBenPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~ClientPlotFuBenPhase(){}

    virtual bool isNotify() { return true; }
    virtual bool exec(DScene *s);
    virtual bool isSyncClient() { return true; }
};

// use phase
class UseFuBenPhase : public FuBenPhase
{
  public:
    UseFuBenPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~UseFuBenPhase() {}

    virtual bool isNotify() { return true; }
    virtual bool exec(DScene *s);
    virtual bool init();
    virtual bool isSyncClient() { return true; }
};

// seal reward
class SealRewardFuBenPhase : public FuBenPhase
{
  public:
    SealRewardFuBenPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~SealRewardFuBenPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
  private:
    TSetDWORD m_setRewardIDs;
    bool m_bTaskExtraReward = false;
};

// guild raid summon monster
class GRaidSummonFuBenPhase : public FuBenPhase
{
  public:
    GRaidSummonFuBenPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~GRaidSummonFuBenPhase() {}

    virtual bool exec(DScene* s);
    virtual bool init();
  private:
    TSetDWORD m_setGroupIDs;
    TSetDWORD m_setUIDs;
    bool m_bKeepGuildSame = false;
    //std::pair<TSetDWORD, TSetDWORD> m_pairGroup2Uids;
};

// guild raid reward
class GRaidRewardFuBenPhase : public FuBenPhase
{
  public:
    GRaidRewardFuBenPhase(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~GRaidRewardFuBenPhase() {}

    virtual bool exec(DScene* s);
    virtual bool init();
  public:
    DWORD m_dwRewardMsg = 0;
};

// guild fire check is monster ciry and in fire
class GuildFireCheckMonster : public FuBenPhase
{
  public:
    GuildFireCheckMonster(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~GuildFireCheckMonster() {}

    virtual bool exec(DScene* s);
};

// guild fire summon metalnpc
class GuildFireSummonMetal : public FuBenPhase
{
  public:
    GuildFireSummonMetal(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~GuildFireSummonMetal() {}

    virtual bool exec(DScene* s);
    virtual bool init();
    //virtual bool exec(DScene* s);
  private:
    DWORD m_dwFireGroupID = 0;
    DWORD m_dwPeaceGroupID = 0;
};

// Altman reward phase
class AltmanRewardPhase : public FuBenPhase
{
  public:
    AltmanRewardPhase(const xLuaData &d) : FuBenPhase(d) {}
    virtual ~AltmanRewardPhase() {}

    virtual bool exec(DScene *s);
    virtual bool init();
};

class CheckSummonDeadBossNpc : public FuBenPhase
{
  public:
    CheckSummonDeadBossNpc(const xLuaData& d) : FuBenPhase(d) {}
    virtual ~CheckSummonDeadBossNpc() {}

    virtual bool exec(DScene* s);
    virtual bool init();
  private:
    DWORD m_dwNpcUniqueID = 0;
};

