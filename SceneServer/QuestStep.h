/**
 * @file QuestStep.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-06-01
 */

#pragma once

#include <sstream>
#include "SceneQuest.pb.h"
#include "SceneManual.pb.h"
#include "SessionSociality.pb.h"
#include "Var.pb.h"
#include "xEntry.h"
#include "xDefine.h"
#include "xPos.h"
#include "xLuaTable.h"
#include "SceneDefine.h"
#include "NpcConfig.h"
#include "WeaponPetConfig.h"

using namespace Cmd;
using std::vector;
using std::string;
using std::pair;
using std::ostringstream;

struct SQuestCFG;
class SceneUser;
class BasePackage;
class BaseStep : public xEntry
{
  friend class TestQuest;
  public:
    BaseStep(DWORD questid);
    virtual ~BaseStep();

    DWORD getQuestID() const { return m_dwQuestID; }
    DWORD getMapID() const { return m_dwMapID; }
    DWORD getSubGroup() const { return m_dwSubGroup; }
    DWORD getFinishJump() const { return m_dwFinishJump; }
    DWORD getFailJump() const { return m_dwFailJump; }
    DWORD getDetailID() const { return m_dwDetailID; }
    DWORD getResetJump() const { return m_dwQuestResetJump; }

    const string& getTraceInfo() const { return m_strTraceInfo; }

    virtual EQuestStep getStepType() const = 0;

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
    virtual bool stepUpdate(SceneUser* pUser);

    bool isSyncTeamWanted() const { return m_bSyncTeamWanted; }
    bool isTeamCanFinish() const { return m_bTeamCanFinish; }
    bool canAbandon() const { return m_bAbandon; }
    bool isDayNightLimit() const { return m_bDayNightLimit; }
    bool canDayNightReset() const { return m_bDayNightReset; }

    std::string m_strContent;
    std::string m_strName;
  protected:
    DWORD m_dwQuestID = 0;
    DWORD m_dwMapID = 0;
    DWORD m_dwSubGroup = 0;

    DWORD m_dwFinishJump = 0;
    DWORD m_dwFailJump = 0;

    DWORD m_dwDetailID = 0;
    DWORD m_dwQuestResetJump = 0;

    bool m_bReset = false;
    bool m_bAbandon = false;
    bool m_bPos = false;

    string m_strTraceInfo;

    xPos m_oDestPos;
    TVecDWORD m_vecClientSelects;
    bool m_bSyncTeamWanted = false;
    bool m_bTeamCanFinish = false;
    bool m_bDayNightLimit = false;
    bool m_bDayNightReset = false;
};

// quest step - visit
class VisitStep : public BaseStep
{
  public:
    VisitStep(DWORD questid);
    virtual ~VisitStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_VISIT; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    TSetDWORD m_setNpcIDs;
};

// quest step - kill
class KillStep : public BaseStep
{
  public:
    KillStep(DWORD questid);
    virtual ~KillStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_KILL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwMonsterID = 0;
    DWORD m_dwGroupID = 0;
    DWORD m_dwNum = 0;
};

enum ERewardObject
{
  EREWARD_OBJECT_ME = 1 << 0,
  EREWARD_OBJECT_VISITER = 1 << 1,
  EREWARD_OBJECT_TEAM = 1 << 2,
  EREWARD_OBJECT_CHAT = 1 << 3,
};

// quest step - reward
class RewardStep : public BaseStep
{
  public:
    RewardStep(DWORD questid);
    virtual ~RewardStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_REWARD; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwRewardID = 0;
    DWORD m_dwBuff = 0;
    DWORD m_dwSource = 0;
    DWORD m_dwObjectType = EREWARD_OBJECT_ME;
    DWORD m_dwMailID = 0;
    DWORD m_dwShow = 0;
    xLuaData m_oGMParams;
};

// quest step - collect
class CollectStep : public BaseStep
{
  public:
    CollectStep(DWORD questid);
    virtual ~CollectStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_COLLECT; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwNpcID = 0;
    DWORD m_dwGroupID = 0;
    DWORD m_dwNum = 0;
    DWORD m_dwReward = 0;
};

// quest step - summon
class SummonStep : public BaseStep
{
  public:
    SummonStep(DWORD questid);
    virtual ~SummonStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_SUMMON; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    NpcDefine m_oDefine;
    DWORD m_dwNum = 0;
    DWORD m_dwUserRange = 0;
};

// quest step - guard
class GuardStep : public BaseStep
{
  public:
    GuardStep(DWORD questid);
    virtual ~GuardStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_GUARD; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - gmcmd
class GMCmdStep : public BaseStep
{
  public:
    GMCmdStep(DWORD questid);
    virtual ~GMCmdStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_GMCMD; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

    xLuaData& getData() { return m_oGMCmd; }
  private:
    xLuaData m_oGMCmd;
};

// quest step - testfail
class TestFailStep : public BaseStep
{
  public:
    TestFailStep(DWORD questid);
    virtual ~TestFailStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_TESTFAIL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - use
class UseStep : public BaseStep
{
  public:
    UseStep(DWORD questid);
    virtual ~UseStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_USE; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwDistance = 0;
};

// quest step - gather
class GatherStep : public BaseStep
{
  public:
    GatherStep(DWORD questid);
    virtual ~GatherStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_GATHER; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

    void collectDropItem(SceneUser* pUser, DWORD monsterid, TVecItemInfo& vecItemInfo);
    DWORD getRewardID(DWORD monsterid);
    bool isPrivateGather() const { return m_bPrivateGather; }
  private:
    DWORD m_dwMonsterID = 0;
    DWORD m_dwGroupID = 0;
    DWORD m_dwRewardID = 0;
    DWORD m_dwNum = 0;
    bool m_bPrivateGather = false;

    TVecItemInfo m_vecItemInfo;
};

// quest step - delete
class DeleteStep : public BaseStep
{
  public:
    DeleteStep(DWORD questid);
    virtual ~DeleteStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_DELETE; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    TVecItemInfo m_vecDeleteItems;
};

// quest step - raid
class RaidStep : public BaseStep
{
  public:
    RaidStep(DWORD questid);
    virtual ~RaidStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_RAID; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

    DWORD getRaidID() const { return m_dwRaidID; }
    DWORD getRange() const { return m_dwRange; }
    const xPos& getCenterPos() const { return m_oCenterPos; }
  private:
    DWORD m_dwRaidID = 0;
    DWORD m_dwRange = 0;
    xPos m_oCenterPos;
};

// quest step - camera
class CameraStep : public BaseStep
{
  public:
    CameraStep(DWORD questid);
    virtual ~CameraStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CAMERA; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - level
class LevelStep : public BaseStep
{
  public:
    LevelStep(DWORD questid);
    virtual ~LevelStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_LEVEL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwBaseLevel = 0;
    DWORD m_dwJobLevel = 0;
};

// quest step - wait
class WaitStep : public BaseStep
{
  public:
    WaitStep(DWORD questid);
    virtual ~WaitStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_WAIT; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwTime = 0;
};

// quest step - move
class MoveStep : public BaseStep
{
  public:
    MoveStep(DWORD questid);
    virtual ~MoveStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_MOVE; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwDistance = 0;
};

// quest step - dialog
class DialogStep : public BaseStep
{
  public:
    DialogStep(DWORD questid);
    virtual ~DialogStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_DIALOG; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

    bool isDynamic() const { return m_bDynamic; }
    void toDynamicConfig(QuestStep* pStep, QWORD dialogid);
  private:
    bool m_bDynamic = false;
};

// quest step - prequest
class PreQuestStep : public BaseStep
{
  public:
    PreQuestStep(DWORD questid);
    virtual ~PreQuestStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_PREQUEST; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwPreQuest = 0;
};

// quest step - clearnpc
class ClearNpcStep : public BaseStep
{
  public:
    ClearNpcStep(DWORD questid);
    virtual ~ClearNpcStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CLEARNPC; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwNpcID = 0;

    enum EClearMethod
    {
      ECLEARMETHOD_LEAVE = 0,
      ECLEARMETHOD_DEAD = 1,
    };
    EClearMethod m_eMethod = ECLEARMETHOD_LEAVE;
    bool m_bEffect = false;
    xLuaData m_oEffect;
};

// quest step - mounton
class MountStep : public BaseStep
{
  public:
    MountStep(DWORD questid);
    virtual ~MountStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_MOUNTRIDE; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwID = 0;
    DWORD m_dwTime = 0;

    ERideType m_eType = ERIDETYPE_MIN;
};

// quest step - selfie
class SelfieStep : public BaseStep
{
  public:
    SelfieStep(DWORD questid);
    virtual ~SelfieStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_SELFIE; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwDistance = 0;
};

enum ESexCondition
{
  ESEXCOND_IGNOER = 0,
  ESEXCOND_SAME = 1,
  ESEXCOND_DIFFERENT = 2
};

// quest step - checkteam
class CheckTeamStep : public BaseStep
{
  public:
    CheckTeamStep(DWORD questid);
    virtual ~CheckTeamStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKTEAM; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwMemberNum = 0;
    DWORD m_eSex = 0;       // 0- 忽略 1-必须相同 2-必须不同
    DWORD m_dwType = 0;     // 0-当前地图队伍人数  1-队伍人数
};

// quest step - remove money
class RemoveMoneyStep : public BaseStep
{
  public:
    RemoveMoneyStep(DWORD questid);
    virtual ~RemoveMoneyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_REMOVEMONEY; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwROB = 0;

    EMoneyType m_eType = EMONEYTYPE_MIN;
    DWORD m_dwCount = 0;
};

// quest step - class
class ClassStep : public BaseStep
{
  public:
    ClassStep(DWORD questid);
    virtual ~ClassStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CLASS; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    EProfession m_eProfession = EPROFESSION_MIN;
    bool m_bExact = false;
};

// quest step - org class
class OrgClassStep : public BaseStep
{
  public:
    OrgClassStep(DWORD questid);
    virtual ~OrgClassStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_ORGCLASS; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    EProfession m_eProfession = EPROFESSION_MIN;
    bool m_bExact = false;
};

// quest step - evo
class EvoStep : public BaseStep
{
  public:
    EvoStep(DWORD questid);
    virtual ~EvoStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_EVO; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwEvo = 0;
    bool m_bExact = false;
};

// quest step - check quest
class CheckQuestStep : public BaseStep
{
  public:
    CheckQuestStep(DWORD questid);
    virtual ~CheckQuestStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKQUEST; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwQuestID = 0;

    bool m_bStart = false;
    bool m_bFinish = false;
};

// quest step - check group
class CheckGroupStep : public BaseStep
{
  public:
    CheckGroupStep(DWORD questid);
    virtual ~CheckGroupStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKGROUP; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum ECheckMethod
    {
      ECHECKMETHOD_NORMAL = 0,
      ECHECKMETHOD_DAILYRAND = 1,
    };

    DWORD m_dwQuestID = 0;
    DWORD m_dwNum = 0;
    DWORD m_dwMapID = 0;

    bool m_bStart = false;
    bool m_bFinish = false;

    ECheckMethod m_eMethod = ECHECKMETHOD_NORMAL;
};

// quest step - check item
class CheckItemStep : public BaseStep
{
  public:
    CheckItemStep(DWORD questid);
    virtual ~CheckItemStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKITEM; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    bool collectPackItem(SceneUser* pUser, EPackType eType, TVecItemInfo& vecResult);
  private:
    TVecItemInfo m_vecItemInfo;

    bool m_bEquip = false;
    bool m_bStoreage = false;
    bool m_bPStoreage = false;
    bool m_bCheckAll = false;
};

// quest step - remove item
class RemoveItemStep : public BaseStep
{
  public:
    RemoveItemStep(DWORD questid);
    virtual ~RemoveItemStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_REMOVEITEM; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum ERemoveMethod
    {
      EREMOVEMETHOD_REMOVECOUNT = 0,
      EREMOVEMETHOD_REMOVEALL,
      EREMOVEMETHOD_MAX,
    };

    ERemoveMethod m_eMethod = EREMOVEMETHOD_REMOVECOUNT;
    TVecItemInfo m_vecItemInfo;

    bool m_bAll = false;
    bool m_bTemp = false;
};

// quest step - random jump
class RandomJumpStep : public BaseStep
{
  public:
    RandomJumpStep(DWORD questid);
    virtual ~RandomJumpStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_RANDOMJUMP; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    vector<pair<DWORD, DWORD>> m_vecJumpPoint;

    DWORD m_dwMaxRand = 0;
};

// quest step - check level
class CheckLevelStep : public BaseStep
{
  public:
    CheckLevelStep(DWORD questid);
    virtual ~CheckLevelStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKLEVEL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwBaseLv = 0;
    DWORD m_dwJobLv = 0;
};

// quest step - check gear
class CheckGearStep : public BaseStep
{
  public:
    CheckGearStep(DWORD questid);
    virtual ~CheckGearStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKGEAR; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwGearID = 0;
    DWORD m_dwStateID = 0;
};

// quest step - purify
class PurifyStep : public BaseStep
{
  public:
    PurifyStep(DWORD questid);
    virtual ~PurifyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_PURIFY; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - action
class ActionStep : public BaseStep
{
  public:
    ActionStep(DWORD questid);
    virtual ~ActionStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_ACTION; } 

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    QWORD m_qwID = 0;

    DWORD m_dwType = 0;
    DWORD m_dwObjects = 0;
    DWORD m_dwNpcID = 0;
    DWORD m_dwRelateion= 0;
    DWORD m_dwRange = 0;
    xPos m_oCenterPos;
    TVecDWORD m_vecMapID;

    set<ENpcZoneType> m_setZoneTypes;
    set<ENpcType> m_setNpcTypes;
    set<ERaceType> m_setRaceTypes;
    set<ENatureType> m_setNatureTypes;
};

// quest step - skill
class SkillStep : public BaseStep
{
  public:
    SkillStep(DWORD questid);
    virtual ~SkillStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_SKILL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum ESkillStatus
    {
      ESKILLSTATUS_MIN = 0,
      ESKILLSTATUS_CHANT = 1,
      ESKILLSTATUS_RUN = 2,
      ESKILLSTATUS_STOP = 3,
      ESKILLSTATUS_MAX
    };
  private:
    DWORD m_dwSkillID = 0;
    ESkillStatus m_eStatus = ESKILLSTATUS_MIN;

    TVecDWORD m_vecNpcIDs;
    xPos m_oDestPos;
};

// quest step - interlocution
class InterStep : public BaseStep
{
  public:
    InterStep(DWORD questid);
    virtual ~InterStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_INTERLOCUTION; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwInterID = 0;
};

// quest step - guid
class GuideStep : public BaseStep
{
  public:
    GuideStep(DWORD questid);
    virtual ~GuideStep();

    virtual EQuestStep getStepType() const { return m_eGuideStep; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    EQuestStep m_eGuideStep = EQUESTSTEP_MIN;
};

// quest step - guid check
class GuideCheckStep : public BaseStep
{
  public:
    GuideCheckStep(DWORD questid);
    virtual ~GuideCheckStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_GUIDE_CHECK; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwAttrPoint = 0;
    DWORD m_dwSkillPoint = 0;
    DWORD m_dwRob = 0;
    DWORD m_dwSkillID = 0;

    typedef vector<pair<DWORD, DWORD>> TVecSkillLevel;
    TVecSkillLevel m_vecSkillLv;
};

// quest step - empty
class EmptyStep : public BaseStep
{
  public:
    EmptyStep(DWORD questid);
    virtual ~EmptyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_EMPTY; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - check strength
class CheckEquipLvStep : public BaseStep
{
  public:
    CheckEquipLvStep(DWORD questid);
    virtual ~CheckEquipLvStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKEQUIPLV; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwTypeID = 0;
    pair<DWORD, DWORD> m_pairLv;

    bool m_bEquip = false;
    bool m_bStoreage = false;
};

// quest step - check money
class CheckMoneyStep : public BaseStep
{
  public:
    CheckMoneyStep(DWORD questid);
    virtual ~CheckMoneyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKMONEY; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    EMoneyType m_eType = EMONEYTYPE_MIN;
    DWORD m_dwCount = 0;
};

// quest step - option
class OptionStep : public BaseStep
{
  public:
    OptionStep(DWORD questid);
    virtual ~OptionStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_OPTION; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwLayer = 0;
};

// quest step - check option
class CheckOptionStep : public BaseStep
{
  public:
    CheckOptionStep(DWORD questid);
    virtual ~CheckOptionStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKOPTION; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwSaveMap = 0;
    DWORD m_dwActiveMap = 0;
    DWORD m_dwPartnerID = 0;

    DWORD m_dwManualMonsterID = 0;
    EManualStatus m_eMonsterStatus = EMANUALSTATUS_MIN;
    DWORD m_dwMonsterUnlock = 0;

    EVarType m_eVarType = EVARTYPE_MIN;
    DWORD m_dwVarValue = 0;

    bool m_bGenderCheck = false;

    enum EHandStatus
    {
      EHANDSTATUS_MIN = 0,
      EHANDSTATUS_HAND = 1,
      EHANDSTATUS_NOHAND = 2,
      EHANDSTATUS_MAX
    };
    EHandStatus m_eHandStatus = EHANDSTATUS_MIN;

    bool m_bHasMonthCard = false;

    set<EMARITAL> m_setMarital;
};

// quest step - hint
class HintStep : public BaseStep
{
  public:
    HintStep(DWORD questid);
    virtual ~HintStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_HINT; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwMsgID = 0;
    DWORD m_dwMapID = 0;

    string m_strParam;
};

// quest step - seal
class SealStep : public BaseStep
{
  public:
    SealStep(DWORD questid);
    virtual ~SealStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_SEAL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwSealID = 0;
};

// quest step - str
class EquipLvStep : public BaseStep
{
  public:
    EquipLvStep(DWORD questid);
    virtual ~EquipLvStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_EQUIPLV; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwTypeID = 0;
    pair<DWORD, DWORD> m_pairLv;

    bool m_bEquip = false;
    bool m_bStoreage = false;
};

// quest step - video
class VideoStep : public BaseStep
{
  public:
    VideoStep(DWORD questid);
    virtual ~VideoStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_VIDEO; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - illustration
class IllustrationStep : public BaseStep
{
  public:
    IllustrationStep(DWORD questid);
    virtual ~IllustrationStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_ILLUSTRATION; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - quest npc play
class NpcPlayStep : public BaseStep
{
  public:
    NpcPlayStep(DWORD questid);
    virtual ~NpcPlayStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_NPCPLAY; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    xLuaData m_oGMCmd;
    DWORD m_dwQuestNpcID = 0;
};

// quest step - item
class ItemStep : public BaseStep
{
  public:
    ItemStep(DWORD questid);
    virtual ~ItemStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_ITEM; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    bool collectItem(TVecItemInfo& vecItem, BasePackage* pPack);
  private:
    TVecItemInfo m_vecItemInfo;

    bool m_bEquip = false;
    bool m_bTemp = false;
};

// quest step - daily
class DailyStep : public BaseStep
{
  public:
    DailyStep(DWORD questid);
    virtual ~DailyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_DAILY; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - check manual status
class CheckManualStep : public BaseStep
{
  public:
    CheckManualStep(DWORD questid);
    virtual ~CheckManualStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECK_MANUAL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

  private:
    EManualStatus eStatus = EMANUALSTATUS_MIN;
    EManualType eType = EMANUALTYPE_MIN;
    ENpcType eMonsterType = ENPCTYPE_MIN;
    DWORD dwNeedNum = 0;
};

// quest step - check manual status
class ManualStep : public BaseStep
{
  public:
    ManualStep(DWORD questid);
    virtual ~ManualStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_MANUAL; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

  private:
    EManualStatus eStatus = EMANUALSTATUS_MIN;
    EManualType eType = EMANUALTYPE_MIN;

    ENpcType eMonsterType = ENPCTYPE_MIN;
    DWORD dwNeedNum = 0;

    DWORD m_dwID = 0;
};

// quest step - play music
class PlayMusicStep : public BaseStep
{
public:
  PlayMusicStep(DWORD questid);
  virtual ~PlayMusicStep();

  virtual EQuestStep getStepType() const { return EQUESTSTEP_PLAY_MUSIC; }

  virtual bool init(xLuaData& data);
  virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
  virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

private:
  DWORD m_dwMusicID = 0;
  DWORD m_dwNum = 0;
};

// reward help
class RewardHelpStep : public BaseStep
{
  public:
    RewardHelpStep(DWORD questid);
    virtual ~RewardHelpStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_REWRADHELP; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");

    void sendHelpReward(SceneUser* pUser);
  private:
    DWORD dwFriendRewardCnt = 0;
    DWORD dwGuildRewardCnt = 0;
};

// money
class MoneyStep : public BaseStep
{
  public:
    MoneyStep(DWORD questid);
    virtual ~MoneyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_MONEY; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum EMoneyMethod
    {
      EMONEYMETHOD_MIN = 0,
      EMONEYMETHOD_CHECK_WAIT = 1,
      EMONEYMETHOD_CHECK_NOW = 2,
      EMONEYMETHOD_SUB = 3,
      EMONEYMETHOD_MAX = 4,
    };
    EMoneyMethod m_eMethod = EMONEYMETHOD_MIN;
    EMoneyType m_eType = EMONEYTYPE_MIN;
    DWORD m_dwNum = 0;
};

// cat
class CatStep : public BaseStep
{
  public:
    CatStep(DWORD questid);
    virtual ~CatStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_MONEY; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum ECatMethod
    {
      ECATMETHOD_MIN = 0,
      ECATMETHOD_CHECK_MONEY = 1,
      ECATMETHOD_CHECK_HIRE = 2,
      ECATMETHOD_CHECK_MAX = 3,
      ECATMETHOD_HIRE = 4,
      ECATMETHOD_MAX = 5,
    };
    ECatMethod m_eMethod = ECATMETHOD_MIN;
    EEmployType m_eType = EEMPLOYTYPE_MIN;
    DWORD m_dwCatID = 0;
};

// activity
class ActivityStep : public BaseStep
{
  public:
    ActivityStep(DWORD questid);
    virtual ~ActivityStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_ACTIVITY; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum EActMethod
    {
      EACTMETHOD_MANUAL_FULL = 1,
      EACTMETHOD_MANUAL_NUM = 2,
    };
    EActMethod m_eMethod = EACTMETHOD_MANUAL_NUM;

    TSetDWORD m_setManualIDs;
    DWORD m_dwNum = 0;
    EManualType m_eType = EMANUALTYPE_MIN;
    EManualStatus m_eStatus = EMANUALSTATUS_MIN;

    bool m_bPhoto = false;
};

// photo
class PhotoStep : public BaseStep
{
  public:
    PhotoStep(DWORD questid);
    virtual ~PhotoStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_PHOTO; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwCreatureID = 0;
    DWORD m_dwNum = 0;
    DWORD m_dwActionID = 0;
    DWORD m_dwRange = 0;

    ECreatureStatus m_eStatus = ECREATURESTATUS_MIN;

    bool m_bFocus = false;
    bool m_bGuild = false;
    bool m_bSelf = false;
    bool m_bGroup = false;
};

// item use
class ItemUseStep : public BaseStep
{
  public:
    ItemUseStep(DWORD questid);
    virtual ~ItemUseStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_ITEMUSE; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    ItemInfo m_oItem;

    DWORD m_dwNum = 0;
    TSetDWORD m_setAvailableMap;
    EItemType eItemType = EITEMTYPE_MIN;

    bool m_bFocus = false;
    bool m_bGuild = false;
};

// hand
class HandStep : public BaseStep
{
  public:
    HandStep(DWORD questid);
    virtual ~HandStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_HAND; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwTime = 0;

    bool m_bFocus = false;
    bool m_bGuild = false;
};

// music
class MusicStep : public BaseStep
{
  public:
    MusicStep(DWORD questid);
    virtual ~MusicStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_MUSIC; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwTime = 0;
    DWORD m_dwNum = 0;

    bool m_bFocus = false;
    bool m_bGuild = false;
    bool m_bTeam = false;
};

// rand item
class RandItemStep : public BaseStep
{
  public:
    RandItemStep(DWORD questid);
    virtual ~RandItemStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_RANDITEM; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum ERandItemMethod
    {
      ERANDITEMMETHOD_RAND = 0,
      ERANDITEMMETHOD_CHECK = 1,
      ERANDITEMMETHOD_REMOVE = 2,
      ERANDITEMMETHOD_MAX
    };
    TVecItemInfo m_vecItemInfo;
    DWORD m_dwNum = 0;
    ERandItemMethod m_eMethod = ERANDITEMMETHOD_RAND;
};

// carrier
class CarrierStep : public BaseStep
{
  public:
    CarrierStep(DWORD questid);
    virtual ~CarrierStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CARRIER; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwCarrierID = 0;
    DWORD m_dwNum = 0;

    bool m_bFocus = false;
    bool m_bGuild = false;
    bool m_bTeam = false;
};

// battle
class BattleStep : public BaseStep
{
  public:
    BattleStep(DWORD questid);
    virtual ~BattleStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_BATTLE; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser);
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwMonsterID = 0;
    DWORD m_dwNum = 0;

    bool m_bFocus = false;
    bool m_bGuild = false;
};

// pet
class PetStep : public BaseStep
{
  public:
    PetStep(DWORD questid);
    virtual ~PetStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_PET; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum EPetMethod
    {
      EPETMETHOD_MIN = 0,
      EPETMETHOD_FRIENDLV_WAIT = 1,
      EPETMETHOD_FRIENDLV_NOW = 2,
      EPETMETHOD_MAX,
    };

    EPetMethod m_eMethod = EPETMETHOD_MIN;

    DWORD m_dwBaseLv = 0;
    DWORD m_dwFriendLv = 0;
    DWORD m_dwNum = 0;
};

// cookfood
class CookFoodStep : public BaseStep
{
public:
  CookFoodStep(DWORD questid);
  virtual ~CookFoodStep();

  virtual EQuestStep getStepType() const { return EQUESTSTEP_COOKFOOD; }
  virtual bool init(xLuaData& data);
  virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
  virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
private:
  DWORD m_dwFoodId = 0;
  DWORD m_dwNum = 0;
};

// scene
class SceneStep : public BaseStep
{
  public:
    SceneStep(DWORD questid);
    virtual ~SceneStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_SCENE; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwMapID = 0;
};

// cook
class CookStep : public BaseStep
{
  public:
    CookStep(DWORD questid);
    virtual ~CookStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_COOK; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum ECookMethod
    {
      ECOOKMETHOD_MIN = 0,
      ECOOKMETHOD_LV = 1,
      ECOOKMETHOD_MAX = 2,
    };

    ECookMethod m_eMethod = ECOOKMETHOD_MIN;
    map<DWORD, DWORD> m_mapLv2Step;
};

// buff
class BuffStep : public BaseStep
{
  public:
    BuffStep(DWORD questid);
    virtual ~BuffStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_BUFF; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum EBuffMethod
    {
      EBUFFMETHOD_MIN = 0,
      EBUFFMETHOD_CHECK = 1,
      EBUFFMETHOD_WAIT = 2,
      EBUFFMETHOD_MAX,
    };

    EBuffMethod m_eMethod = EBUFFMETHOD_MIN;
    DWORD m_dwBuffID = 0;
};

// social
class TutorStep : public BaseStep
{
  public:
    TutorStep(DWORD questid);
    virtual ~TutorStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_TUTOR; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum ETutorMethod
    {
      ETUTORMETHOD_MIN = 0,
      ETUTORMETHOD_WAIT = 1,
      ETUTORMETHOD_CHECK = 2,
      ETUTORMETHOD_GRADUATION = 3,
      ETUTORMETHOD_MAX,
    };

    ETutorMethod m_eMethod = ETUTORMETHOD_MIN;
    bool m_bTutor = false;
};

// christmas
class ChristmasStep : public BaseStep
{
  public:
    ChristmasStep(DWORD questid);
    virtual ~ChristmasStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHRISTMAS; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    TVecDWORD vecStartTime;
    DWORD m_dwPreQuestID = 0;
};

// christmas
class ChristmasRunStep : public BaseStep
{
  public:
    ChristmasRunStep(DWORD questid);
    virtual ~ChristmasRunStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHRISTMAS_RUN; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    std::map<DWORD, DWORD> m_mapReward;
};

class BeingStep : public BaseStep
{
  public:
    BeingStep(DWORD questid);
    virtual ~BeingStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_BEING; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwBeingID = 0;
    DWORD m_dwBeingLv = 0;
    bool m_bFailStay = false;
};

//check joy
class CheckJoyStep : public BaseStep
{
  public:
    CheckJoyStep(DWORD questid);
    virtual ~CheckJoyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECK_JOY; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    EJoyActivityType m_eType;
};

//add joy
class AddJoyStep : public BaseStep
{
  public:
    AddJoyStep(DWORD questid);
    virtual ~AddJoyStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_ADD_JOY; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    EJoyActivityType m_eType;
    DWORD m_dwAddJoy = 0;
};

// rand dialog
class RandDialogStep : public BaseStep
{
  public:
    RandDialogStep(DWORD questid);
    virtual ~RandDialogStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_RAND_DIALOG; }
    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    std::map<DWORD, TVecDWORD> m_mapDialog;
};

// quest step - cg
class CgStep : public BaseStep
{
  public:
    CgStep(DWORD questid);
    virtual ~CgStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CG; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - check servant
class CheckServantStep : public BaseStep
{
  public:
    CheckServantStep(DWORD questid);
    virtual ~CheckServantStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHECKSERVANT; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwServantID = 0;
};

// quest step - client_plot
class ClientPlotStep : public BaseStep
{
  public:
    ClientPlotStep(DWORD questid);
    virtual ~ClientPlotStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CLIENTPLOT; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
};

// quest step - chat
class ChatStep : public BaseStep
{
  public:
    ChatStep(DWORD questid);
    virtual ~ChatStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHAT; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    TSetString m_setKey;
};

// quest step - transfer
class TransferStep : public BaseStep
{
  public:
    TransferStep(DWORD questid);
    virtual ~TransferStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_TRANSFER; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    DWORD m_dwFromTransferid = 0;
    DWORD m_dwToTransferid = 0;
};

// quest step - redialog
class ReDialogStep : public BaseStep
{
  public:
    ReDialogStep(DWORD questid);
    virtual ~ReDialogStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_REDIALOG; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    enum EReDialogMethod
    {
      EREDIALOGMETHOD_RAND = 1,
      EREDIALOGMETHOD_CLEAR = 2,
    };
    EReDialogMethod m_eMethod = EREDIALOGMETHOD_RAND;
    bool m_bExclude = false;
    DWORD m_dwTimes = 0;
    TSetDWORD m_setDialogIDs;
};

// quest step - chatsystem
class ChatSystemStep : public BaseStep
{
  public:
    ChatSystemStep(DWORD questid);
    virtual ~ChatSystemStep();

    virtual EQuestStep getStepType() const { return EQUESTSTEP_CHAT_SYSTEM; }

    virtual bool init(xLuaData& data);
    virtual void reset(SceneUser* pUser) { BaseStep::reset(pUser); }
    virtual bool doStep(SceneUser* pUser, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
  private:
    TVecString m_vecKey;
    DWORD m_dwChannel = 0;
    DWORD m_dwRelation = 0;
    DWORD m_dwMultiKeyWord = 0;
    DWORD m_dwOrder = 0;
};
