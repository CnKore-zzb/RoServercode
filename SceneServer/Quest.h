/**
 * @file Quest.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-05-15
 */

#pragma once

#include <memory>
#include "xSingleton.h"
#include "RecordCmd.pb.h"
#include "SessionCmd.pb.h"
#include "QuestStep.h"
#include "QuestConfig.h"

using std::vector;
using std::string;
using std::map;
using std::set;
using std::shared_ptr;
using namespace Cmd;

enum EQuestMethod
{
  EQUESTMETHOD_DEL_ACCEPT = 1,
  EQUESTMETHOD_ADD_SUBMIT = 2,
};

// quest config data
typedef shared_ptr<BaseStep> TPtrBaseStep;
typedef vector<TPtrBaseStep> TVecQuestStep;
struct SQuestCFGEx : public SQuestCFG
{
  TVecQuestStep vecSteps;

  SQuestCFGEx() {}
  virtual ~SQuestCFGEx() {}

  const TPtrBaseStep getStep(DWORD step) const;
  DWORD getStepBySubGroup(DWORD subGroup) const;
  const string& getName() const { return vecSteps.empty() == false ? vecSteps[0]->m_strName : STRING_EMPTY; }

  //bool toData(QuestData* pData, DWORD language = ELANGUAGE_CHINESE_SIMPLIFIED) const;
};
typedef map<DWORD, SQuestCFGEx> TMapQuestCFGEx;

// quest manager
class QuestManager : public xSingleton<QuestManager>
{
  friend class TestQuest;
  public:
    QuestManager();
    virtual ~QuestManager();

    bool init();

    void setDailyExtra(DWORD dwValue);
    DWORD getDailyExtra() const { return m_dwDailyExtra; }

    const SQuestCFGEx* getQuestCFG(DWORD id) const;

    bool createQuestData(QuestData* pData, const SQuestCFGEx* pCFG, SceneUser* pUser, const SWantedItem* pWanted = nullptr);
    void collectCanAcceptQuest(SceneUser* pUser, QuestList& rList);
    void collectWantedQuest(SceneUser* pUser, DWORD dwWantedID, QuestList& rList);

    bool isQuestComplete(DWORD id, SceneUser* pUser);
    bool isQuestComplete(const string& version, DWORD id, SceneUser* pUser);

    void timer(DWORD curTime);
    const TMapQuestCFGEx& getList() const { return m_mapQuestCFG; }
    const TMapQuestCFGEx& getCycleList() const { return m_mapCycleQuestCFG; }
    const TMapQuestCFGEx& getOutDateCycleList() const { return m_mapOutDateCycleQuestCFG; }
  private:
    TPtrBaseStep createStep(DWORD id, const string& type, xLuaData& data);

    bool clear();
  private:
    TMapQuestCFGEx m_mapQuestCFG;
    TMapQuestCFGEx m_mapCycleQuestCFG;
    TMapQuestCFGEx m_mapOutDateCycleQuestCFG;

    DWORD m_dwWantedTick = 0;
    DWORD m_dwDailyExtra = 10;
};

// quest item
class QuestItem
{
  friend class Quest;
  public:
    QuestItem(const QuestData& data, const SQuestCFGEx* pCFG);
    ~QuestItem();

    void toData(QuestData* pData);
    void toClientData(QuestData* pData, DWORD language = 1);
    void toSubmitData(QuestData* pData);

    DWORD getID() const { return m_oData.id(); }
    DWORD getStep() const { return m_oData.step(); }
    DWORD getTime() const { return m_oData.time(); }
    DWORD getAcceptLv() const { return m_oData.acceptlv(); }
    DWORD getGroupID() const { return m_oData.id() / QUEST_ID_PARAM; }
    DWORD getVersion() const { return m_oData.version(); }

    EQuestStep getStepType();
    TPtrBaseStep getStepCFG() const;
    QuestStep* getStepData();
    const SQuestCFGEx* getQuestCFG() const { return m_pCFG; }

    void setComplete(bool complete) { m_oData.set_complete(complete); }
    bool isComplete() const { return m_oData.complete(); }

    void setTrace(bool trace) { m_oData.set_trace(trace); }
    bool getTrace() const { return m_oData.trace(); }

    bool isMain() const { return m_pCFG == nullptr ? false : m_pCFG->eType == EQUESTTYPE_MAIN; }
    bool isNoStep() const { return m_pCFG == nullptr ? true : m_oData.step() >= m_pCFG->vecSteps.size(); }

    bool addStep();
    bool setStep(DWORD step);

    void collectWantedReward(TVecItemInfo& vecReward);

    void addsparams(const string& str) { m_oData.add_names(str); }
    void addqparams(QWORD qwValue) { m_oData.add_params(qwValue); }

    void clearqparams() { m_oData.clear_params(); }

    void collectSParams(TVecString& vecSParams);
    void collectQParams(TVecQWORD& vecQParams);

    void setQParams(DWORD index, QWORD qwValue);
    QWORD getQParams(DWORD index);
  private:
    QuestData m_oData;
    const SQuestCFGEx* m_pCFG = nullptr;
};

// quest detail
class QuestDetailData
{
  public:
    QuestDetailData(const QuestDetail& data);
    virtual ~QuestDetailData();

    void toData(QuestDetail* pData);

    void setID(DWORD id) { m_oData.set_id(id); }
    DWORD getID() const { return m_oData.id(); }

    void setTime(DWORD time) { m_oData.set_time(time); }
    DWORD getTime() const { return m_oData.time(); }

    void setMap(DWORD map) { m_oData.set_map(map); }
    DWORD getMap() const { return m_oData.map(); }

    void setComplete(bool complete) { m_oData.set_complete(complete); }
    bool getComplete() const { return m_oData.complete(); }

    void setTrace(bool trace) { m_oData.set_trace(trace); }
    bool getTrace() const { return m_oData.trace(); }

    bool addDetailID(DWORD id);
  private:
    QuestDetail m_oData;
};

// quest
class SceneUser;
class SceneNpc;
typedef std::shared_ptr<QuestItem> TPtrQuestItem;
typedef map<DWORD, TPtrQuestItem> TMapQuest;
typedef vector<QuestDetailData> TVecQuestDetail;
typedef map<DWORD, TSetDWORD> TMapMapQuest;
typedef set<SceneUser*> TSetTmpUser;
typedef set<SceneNpc*> TSetTmpNpc;
typedef map<DWORD, ActivityQuestItem> TMapActivityQuest;

struct SQuestPuzzle
{
  string version;

  TSetDWORD setOpen;
  TSetDWORD setUnlock;
};
typedef map<string, SQuestPuzzle> TMapVersionPuzzle;

class Quest
{
  friend class Achieve;
  friend class BaseStep;
  friend class TestFailStep;
  friend class RandomJumpStep;
  friend class RandItemStep;
  friend class GMCommandRuler;
  public:
    Quest(SceneUser* pUser);
    ~Quest();

    bool load(const BlobQuest& oAccQuest, const BlobQuest& oCharQuest);
    bool save(BlobQuest* pAccBlob, BlobQuest* pCharBlob);
    bool loadAccQuest(const string& data);
    bool saveAccQuest(string& rData);
    bool reload();
    bool loadActivityQuest(const BlobActivityQuest& data);
    bool saveActivityQuest(BlobActivityQuest* data);

    //void collectQuestItem(DWORD monsterid, TVecItemInfo& vecItemInfo);
    void collectQuestReward(DWORD monsterid, TSetDWORD& setReward);
    void collectQuestReward(DWORD monsterid, TSetDWORD& setReward, TSetDWORD& setSelfReward);

    TPtrQuestItem getQuest(DWORD id);
    bool hasAcceptQuest(DWORD groupID);

    DWORD getSubmitQuestConut() const;
    DWORD getDailyCount();
    DWORD getDailyTCount();
    void setDailyTCount(DWORD count);
    DWORD getDailyExp();
    DWORD getDailyExtra() const { return (m_pExpPool == nullptr || m_dwDailyCurExp == 0) ? 0 : QuestManager::getMe().getDailyExtra(); }
    const SDailyExpPool* getExpPool() const { return m_pExpPool; }
    void reduceDailyExp(DWORD dwExp);
    bool initDaily();

    bool canAcceptQuest(DWORD id);
    bool questAction(EQuestAction eAction, DWORD id, bool bNoCheck = false);
    bool runStep(DWORD id, DWORD subGroup = 0, QWORD param1 = 0, QWORD param2 = 0, const string& sparam1 = "");
    bool hasMainQuest();
    bool hasWantedQuest();
    bool hasDailyQuest();
    bool hasDailyMapQuest(DWORD dwMapID);
    bool isAccept(DWORD id);
    bool isSubmit(DWORD id);
    bool setTrace(DWORD id, bool trace);
    bool setGroupTrace(DWORD id, bool trace);
    bool isQuestComplete(DWORD id) const;
    bool isForbid(DWORD id) const;

    bool checkVarReward(DWORD id) const;
    bool addVarReward(DWORD id);
    void resetVarReward();

    void sendWantedQuestList(DWORD wantid);
    void sendGuildQuestList();
    //void sendQuestDetailList();
    void sendCurQuestList(DWORD dwMapID = 0, bool bClear = false);
    void acceptNewQuest();
    void acceptTypeQuest(EQuestType eType);
    void acceptCondQuest(EQuestCond eCond);
    void removeInvalidQuest();
    void resetQuest(EQuestReset eReset);
    bool finishQuest(DWORD id);
    bool finishBigQuest(DWORD id, bool bRefresh /*= true*/);
    bool finishBoardQuest(DWORD id, bool skipLastStep);
    bool canFinishBoard(DWORD id, bool skipLastStep);

    void timer(DWORD curTime);

    void onMonsterKill(DWORD monsterID);
    void onItemAdd(const ItemInfo& rItem);
    void onItemAdd(const string& guid);
    void onItemUse(QWORD qwTargetID, DWORD dwItemID);
    void onPassRaid(DWORD raidid, bool bSuccess);
    void onRaidCmd(DWORD questid);
    void onLevelup(DWORD baseLv, DWORD jobLv);
    void onTimer(DWORD curTime);
    void onTenMinTimeUp(DWORD curTime);
    void onChristmasTimer(DWORD curTime);
    void onLogin();
    void onAction(EUserActionType eType, QWORD id);
    void onInterlocution(DWORD dwInterID, bool bCorrect);
    void onSeal(DWORD dwSealID);
    void onDaily();
    void onLeaveScene();
    void onManualUnlock();
    void onPlayMusic(DWORD dwMusicID);
    void onMoneyChange();
    void onTowerPass();
    void onPhoto(const TSetQWORD& setGUIDs);
    void onHand(QWORD qwTargetID, DWORD dwTime);
    void onMusic(DWORD dwTime);
    void onCarrier(const TSetTmpUser& setUser, DWORD dwCarrierID);
    void onPetAdd();
    void onCookFood(DWORD dwFoodId);
    void onEnterScene();
    void onBuff();
    void onQuestSubmit();
    void onBeingLvUp();
    void onExitGuild();
    void onChat(const string& key);
    void onTransfer(DWORD dwFromid,DWORD dwToid);
    void onChatSystem(DWORD channel, QWORD destid, const string& key);

    void patch_2016_07_22();
    void patch_2017_09_11();

    bool acceptGuildQuest(DWORD id);
    bool acceptQuest(DWORD id, bool bNoCheck = false);
    bool abandonGroup(DWORD id, bool bSubmitInclue = false, bool bNtf = true);
    bool abandonQuest(DWORD id);
    bool canQuickFinishBoard(DWORD id, ItemInfo& oItem);
    bool quickFinishBoard(DWORD id);
    bool removeQuest(EQuestList eList, EQuestType eType);
    bool queryOtherData(EOtherData eType);
    bool queryWorldQuest();

    bool checkStep(DWORD id, DWORD step);

    DWORD getRecentSubmit() const { return m_dwRecentSubmitID; }
    DWORD getDailyCount() const { return m_dwDailyCount; }
    DWORD getDailyRandCount(DWORD dwMapID, bool bSubmit = false);

    bool getWantedQuest(std::pair<DWORD, DWORD>& id2step, bool bSyncTeam = false) const;
    DWORD getQuestStep(DWORD questid) const;
    bool setQuestStep(DWORD id, DWORD step);

    // temp
    DWORD getPatchStep() const { return m_dwPatchStep; }
    const TSetTmpUser getTmpUser() const { return m_setTmpUser; }
    const TSetTmpNpc& getTmpNpc() const { return m_setTmpNpc; }

    void getWantedQuestLeaderTeammate(DWORD questid, set<SceneUser*> &teammate);

    bool addForbidQuest(DWORD dwQuestID);
    bool removeForbidQuest(DWORD dwQuestID);

    bool addChoiceQuest(DWORD dwQuestID);
    bool removeChoiceQuest(DWORD dwQuestID);

    bool processQuest(EQuestMethod eMethod, DWORD dwQuest);

    bool doStepWithoutCheck(DWORD id);
    bool deleteQuest(DWORD dwQuestID);
    bool checkOutDateQuest();

    void addActivityFinishTimes(DWORD id);
    DWORD getActivityFinishTimes(DWORD id);

    void setQuickFinishBoardID(DWORD questid) { m_dwQuickFinishBoardID = questid; }
    void setTeamFinishBoardQuestTime(DWORD time) { if (time > m_dwTeamFinishBoardQuestTime) m_dwTeamFinishBoardQuestTime = time; }
    bool isTeamFinishBoardQuestCD();

    void queryManualData(const string& version);
    void openPuzzle(const string& version, DWORD dwID);

    void addQuestToTimer(DWORD id) { m_mapTimerQuestUpdate[id] = true; }
    void delQuestFromTimer(DWORD id) { m_mapTimerQuestUpdate[id] = false; }
  private:
    DWORD m_dwPatchStep = 0;
  private:
    bool submitQuest(DWORD id);
    bool addQuestStep(DWORD id);
    bool stepUpdate(DWORD id);
    bool canUpdate(DWORD id);
    bool canStep(DWORD id);
    bool canSubmit(EQuestType eType) const;
    bool canDetail(EQuestType eType) const;

    bool addQuestDetail(const TPtrBaseStep pStep);
    bool removeQuestDetail(DWORD id);
    bool updateBigQuestComplete(DWORD id);

    void update();
    void submitupdate();
    void completeupdate();
    void detailupdate();

    void addDailyCount();
    void clearDaily();

    void resetWantedQuest();
    void resetDailyQuest();
    void resetDailyDayQuest();
    void resetDailyMapQuest();
    void resetDailyResetQuest();
    void resetAccDailyQuest();
    void resetActivityQuest();
  public:
    void resetQuestTime(DWORD curTime);
    void resetCycleQuest();
    void resetGuildQuest();
    void resetArtifactQuest();
    void resetWeddingQuest(bool bWedding);

    void refreshCollectPuzzles();
  public:
    void setRefresh(bool refresh) { m_bRefresh = refresh; }
    bool getRefresh() { return m_bRefresh; }
    void resetDeadQuest();
  private:
    void resetDailyMapRandQuest();
    void removeDailyMapRandQuest(DWORD dwMapID);

    void inviteFinishBoard(DWORD id, bool isquickfinish = false);
    void checkQuestVersion(bool bLoad = false);
  private:
    TMapQuest m_mapAcceptQuest;
    TMapQuest m_mapCompleteQuest;
    TMapQuest m_mapSubmitQuest;

    TMapMapQuest m_mapMapQuest;
    TMapMapQuest m_mapMapRandQuest;

    TVecQuestDetail m_vecDetails;
    TMapVersionPuzzle m_mapPuzzle;

    SceneUser* m_pUser = nullptr;

    TSetDWORD m_setUpdateIDs;
    TSetDWORD m_setSubmitUpdateIDs;
    TSetDWORD m_setCompleteUpdateIDs;
    TSetDWORD m_setDetailIDs;

    DWORD m_dwDataTick = 0;

    DWORD m_dwDailyCount = 0;
    DWORD m_dwDailyTCount = 0;
    DWORD m_dwDailyCurExp = 0;
    DWORD m_dwDailyLevel = 0;
    TVecDWORD m_vecDailyGeted;
    const SDailyExpPool* m_pExpPool = nullptr;
    DWORD m_dwLastCalcDailyCountTime = 0;

    DWORD m_dwRecentSubmitID = 0;

    TSetDWORD m_setVarReward;
    TSetDWORD m_setForbidQuest;
    TSetDWORD m_setProcessAccQuest;
    TSetDWORD m_setDeadIDs;

    TSetTmpUser m_setTmpUser;
    TSetTmpNpc m_setTmpNpc;

    TMapActivityQuest m_mapActivityQuest;

    DWORD m_dwTeamFinishBoardQuestTime = 0;
    DWORD m_dwQuickFinishBoardID = 0; // 对应看板任务快速完成时不检查npc位置

    bool m_bRefresh = false; // 标识下一帧是否需要刷新

    TSetDWORD m_setTimerQuestIDs;
    map<DWORD, bool> m_mapTimerQuestUpdate;
};

