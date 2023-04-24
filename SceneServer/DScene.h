/**
 * @file DScene.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-06-27
 */

#pragma once

#include "Scene.h"
#include "Laboratory.h"
#include "MatchCCmd.pb.h"
#include "MatchSCmd.pb.h"
#include "WeddingSCmd.pb.h"
#include "MiscConfig.h"
#include "FuBenCmd.pb.h"

using std::pair;
using namespace std;
class SceneTeam;

// quest scene
class RaidScene : public DScene
{
  public:
    RaidScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~RaidScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool init();
};

// laboratory scene
class LaboratoryScene : public DScene
{
  public:
    LaboratoryScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~LaboratoryScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    //virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer);

    virtual void onClose() {}

    bool summon(DWORD roundid);
    void addPoint(DWORD point, DWORD surviveTime);
    void finish();
  private:
    void summon(DWORD groupid, DWORD npcid, DWORD point);
    void clear() { m_dwRoundNum = 0; m_mapPointList.clear(); }
  public:
    // 第几轮
    DWORD m_dwRoundNum = 0;
    // 玩家本次得分
    std::map<QWORD, DWORD> m_mapPointList;
    // 等级
    DWORD m_dwLevel = 0;
    // 玩家结算积分
    std::map<QWORD, DWORD> m_mapCalcPointList;
    // 玩家奖励倍数
    std::map<QWORD, DWORD> m_mapRewardTimes;
};

// tower scene
class TowerScene : public DScene
{
  public:
    TowerScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD dwLayer, DWORD dwNoMonsterLayer);
    virtual ~TowerScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool isTowerScene() { return true; }
    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);
    virtual void entryAction(QWORD curMSec);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    //virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer) {}

    virtual void onClose();

    virtual void getRandomDeadBoss(std::list<pair<DWORD,DWORD>>& bosslist);
    virtual bool checkSummonDeadBoss();

    void setChangeRaid(bool b) { m_bChangeRaid = b; }

    void setLayer(DWORD dwLayer) { m_dwLayer = dwLayer; }
    DWORD getLayer() const { return m_dwLayer; }
    DWORD getNoMonsterLayer() const { return m_dwNoMonsterLayer; }
    bool getBornPos(xPos& pos);

    const vector<pair<DWORD, float>>& getAllRewardIDs() const { return m_vecAllRewardIDs; }
    vector<pair<DWORD, float>>& getAllRewardIDs() { return m_vecAllRewardIDs; }
    void clearAllRewardIDs() { m_vecAllRewardIDs.clear(); }

    void summonUserRewardBox(SceneUser* pUser, bool bEnter);
    static void addMonsterReward(DWORD npcid, vector<pair<DWORD, float>>& m_vecAllRewardIDs, DWORD layer, DWORD mapid, bool superAiNpc);

    void setChangeScene(bool ischange) { m_bChangeScene = ischange; }
    bool isChangeScene() { return m_bChangeScene; }
  private:
    bool m_bChangeRaid = false;
    vector<pair<DWORD, float>> m_vecAllRewardIDs;
    DWORD m_dwLayer = 0;
    xTimer m_oTimer;
    DWORD m_dwNoMonsterLayer = 0;
    bool m_bChangeScene = false;
};

// ferriswheel scene
class FerrisWheelScene : public DScene
{
  public:
    FerrisWheelScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~FerrisWheelScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer) {}
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer) {}

    virtual void onClose() {}
  private:
    QWORD m_qwCarrierMasterID = 0;
};

// dojo scene
class DojoScene : public DScene
{
  public:
    DojoScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD dwDojoId);
    virtual ~DojoScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer);

    virtual void onClose() {}

    DWORD getDojoId() { return m_dwDojoId; }

private:
  DWORD m_dwDojoId;
};

// guild scene
#include "GuildConfig.h"
#include "GuildRaidConfig.h"
#include "GGuild.h"
struct SGuildShowPhoto
{
  vector<GuildPhoto> vecPhotos;
  DWORD dwIndex = 0;
};
class GuildScene : public DScene
{
  public:
    GuildScene(DWORD sID, const char* name, const SceneBase* pBase, const SRaidCFG* pRaidCFG, const GuildInfo& oInfo);
    virtual ~GuildScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool init();
    virtual void entryAction(QWORD curMSec);

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer) {}
    //virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer) {}

    virtual void onClose();

    void setGuildInfo(const GuildInfo& rInfo);
    GGuild& getGuild() { return m_oGuild; }
    void refreshNpc(bool bNtf = true);
    void syncGuildRaidGate();

    void queryPhotoFromGuild(const QueryPhotoListGuildSCmd& cmd);
    void updatePhoto(const PhotoUpdateGuildSCmd& cmd);

    bool frameAction(SceneUser* pUser, const FrameActionPhotoCmd& cmd);
    bool queryFrame(SceneUser* pUser, DWORD dwFrameID);
    void sendSelfPhoto(SceneUser* pUser);

    QWORD getGuildID() { return m_oGuild.id(); }

    void refreshBuildingGate();
    void syncBuildingDataToUser(SceneUser* user = nullptr, const set<EGuildBuilding>& types = set<EGuildBuilding>());
    void updateGuildInfo(const QueryGuildInfoGuildSCmd& cmd);
    void refreshArtifactNpc();

    void queryTreasureFromGuild(const QueryTreasureGuildSCmd& cmd);
    void syncCurTreasureStatus(SceneUser* pTarget);
    void treasureAction(SceneUser* pUser, TreasureActionGuildCmd& cmd);

    void notifyServantEvent();
  private:
    void treasure_action_gvg_frame_on(SceneUser* pUser, TreasureActionGuildCmd& cmd);
    void treasure_action_guild_frame_on(SceneUser* pUser, TreasureActionGuildCmd& cmd);
    void treasure_action_frame_off(SceneUser* pUser, TreasureActionGuildCmd& cmd);
    void treasure_action_change(SceneUser* pUser, TreasureActionGuildCmd& cmd);
    void treasure_action_open_pre(SceneUser* pUser, TreasureActionGuildCmd& cmd);
    void treasure_action_open(SceneUser* pUser, TreasureActionGuildCmd& cmd);

    void refreshPhotoWall(DWORD curSec);
    void refreshDelayCreateNpc(DWORD curSec);
    void sendPhotoWall(SceneUser* pUser);

    bool photoAction(SceneUser* pUser, EFrameAction eAction, DWORD dwFrameID, const GuildPhoto& rPhoto);
    bool removeFirstPhoto(DWORD dwFrameID);
    void refreshBuildingNpc();
    void updateBuildingDataRefreshTime();
  private:
    GGuild m_oGuild;

    DWORD m_dwPhotoRefreshTick = 0;
    bool m_bQueryPhoto = false;
    bool m_bPhotoInit = false;
    DWORD m_dwBuildingDataRefreshTime = 0;

    map<DWORD, TVecGuildPhoto> m_mapFramePhoto;
    map<QWORD, TVecString> m_mapAccPhoto;
    map<string, DWORD> m_mapPhotoFrame;
    map<DWORD, DWORD> m_mapFrameIndexp;

    map<DWORD, QWORD> m_mapExistNpc;
    TListGuildQuestCFG m_listQuest;
    bool m_bGuildInfoInit = false;
    map<DWORD, DWORD> m_mapDelayCreatedNpc; // 延迟创建npc, key:npc unique id, value:创建时间

    bool m_bGuildTreasureInit = false;
    TMapTreasure m_mapTreasure;
    QWORD m_qwTreasureOperID = 0;
    DWORD m_dwTreasureIndex = 0;
    DWORD m_dwBCoinCount = 0;
    DWORD m_dwAssetCount = 0;
    EGuildTreasureType m_eTreasureType = EGUILDTREASURETYPE_MIN;
    ETreasureAction m_eLastTreasureAction = ETREASUREACTION_MIN;
    TVecGuildTreasureCFG m_vecCurTreasureCFG;
};

// item image scene
class ItemImageScene : public DScene
{
public:
  ItemImageScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, xPos pos, DWORD npcid);
  virtual ~ItemImageScene() {}
  virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
  virtual bool init();
  SceneNpc* getNpc() { return m_pNpc; };
  virtual void entryAction(QWORD curMSec);
private:
  void createNpc();

private:
  xPos m_centerPos;
  SceneNpc* m_pNpc = nullptr;
  bool m_bNtfed = false;
  DWORD m_dwNpcId = 0;
};

struct SPvpUserShow
{
  QWORD qwID = 0;
  DWORD dwBeKillCount = 0;
  DWORD dwKillCount = 0;
  DWORD dwHelpCount = 0;
  DWORD dwBraveCount = 0;
  DWORD dwComboCount = 0;
  DWORD dwSaviorCount = 0;
  DWORD dwLastKillTime = 0;
  DWORD dwTotalCombo = 0;
  DWORD dwHealHp = 0;
  DWORD dwDamage = 0;
};


enum ECalcState
{
  ECALCSTATE_FIGHTING = 1,
  ECALCSTATE_WIN = 2,
  ECALCSTATE_WIN_END = 3,
  ECALCSTATE_LEAVE = 4,
  ECALCSTATE_LEAVE_END = 5,
  ECALCSTATE_TIMEOUT = 6,
  ECALCSTATE_TIMEOUT_END = 7,
};

enum EPvpRewardType
{
  EPVPREWARD_KILL = 1,
  EPVPREWARD_HELPKILL = 2,
  EPVPREWARD_DESERTWIN = 3,
  EPVPREWARD_GLAMWIN = 4,
};

class MatchScene : public DScene
{
  public:
    MatchScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~MatchScene();

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool isMatchScene() { return true; }
    virtual bool init();
    virtual void getBornPos(SceneUser* pUser, xPos& pos) {}
    virtual void onReceiveRoomInfo(const SyncRoomSceneMatchSCmd& cmd) = 0;
  protected:
    void ntfMatchCloseRoom();
    void ntfMatchOpenRoom();
  protected:
    std::map<DWORD, xPos> m_mapPos;
};

class PvpBaseScene : public MatchScene
{
  public:
    PvpBaseScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~PvpBaseScene();

    virtual bool init();

    virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath);
    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);
    virtual bool isPVPScene() { return true;};
    virtual bool isDPvpScene() { return true; };
    virtual void addDamageUser(QWORD attackID, QWORD suffererID, DWORD damage);
    virtual void addHealUser(QWORD id, QWORD healID, DWORD hp);
    virtual void checkCombo(QWORD id, DWORD curSec);
    virtual void entryAction(QWORD curMSec);
    virtual bool isHideUser();
    virtual bool isUserCanFireScene() { return true; }
    virtual void onReceiveRoomInfo(const SyncRoomSceneMatchSCmd& cmd) {}

    void comboNotify(QWORD id, DWORD num);
  protected:
    void broadcastFinalScore();
    void addPvpKickUser(QWORD userid) { m_setKickPvpList.insert(userid); }
    void backOriginMap(SceneUser *user);
    void getPvpCoin(SceneUser* pUser, EPvpRewardType eType);
  protected:
    map<QWORD, SPvpUserShow> m_mapUserShow;
    map<QWORD, map<QWORD,DWORD>> m_mapDamage;
    map<QWORD, map<QWORD,DWORD>> m_mapHeal;
    bool m_isOver = false;
    set<QWORD> m_setOnceIn;
    TSetQWORD m_setKickPvpList;
};

// pvp glam metal scene
class GlamMetalScene : public PvpBaseScene
{
  public:
    GlamMetalScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~GlamMetalScene();

    virtual bool init();
    virtual void entryAction(QWORD curMSec);
    virtual void getBornPos(SceneUser* pUser, xPos& pos);
    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    void addNpcDamage(QWORD userid, DWORD damage);
  private:
    void notify(SceneUser* user);
    void notifyAllUser();
    void setEnd();
    void sendWinResult();
    void sendTimeOverResult();

  private:
    DWORD m_dwStartTime = 0;
    DWORD m_dwEndTime = 0;
    DWORD m_dwMetalNpcHpPer = 0;
    QWORD m_qwGlamMetalGuid = 0;
    DWORD m_dwCheckRefreshTime = 0;
    std::map<DWORD/*index*/, xPos> m_mapTeamPos;
    bool m_bIsNpcDie = false;
    DWORD m_dwSendWinResultTime = 0;
    QWORD m_killerId = 0;
    DWORD m_dwSendTimerOverTime = 0;
    ECalcState m_calcState = ECALCSTATE_FIGHTING;
};

// monkey scene
class MonkeyScene : public PvpBaseScene
{
  public:
    MonkeyScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~MonkeyScene() {}

    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);
    virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath);
    virtual void getBornPos(SceneUser* pUser, xPos& pos);

    void calculateScore(QWORD id);
    void notify(QWORD id);
    void notifyAllUser();

  private:
    void syncSceneUserCount();

  private:
    map<QWORD,DWORD> m_mapScore;
};


// pvp沙漠之狼
class DesertWolfScene : public PvpBaseScene
{
public:
  DesertWolfScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
  virtual ~DesertWolfScene();

  virtual bool init();
  virtual void entryAction(QWORD curMSec);
  virtual void userEnter(SceneUser *user);
  virtual void userLeave(SceneUser *user);
  virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath);
  virtual void getBornPos(SceneUser* pUser, xPos& pos);
private:
  void notify(SceneUser* user);
  void notifyAllUser();
  DWORD getMyTeamScore(SceneUser* pUser);
  DWORD getEnemyTeamScore(SceneUser* pUser);
  void addTeamScore(SceneUser* pUser, DWORD score);
  void setEnd(QWORD winTeam=0);
  void sendWinResult();
private:
  DWORD m_dwStartTime = 0;
  DWORD m_dwEndTime = 0;
  DWORD m_dwMaxScore = 0;
  DWORD m_dwCheckRefreshTime = 0;
  std::map<QWORD/*index*/, DWORD/*score*/> m_mapTeamScore;
  std::map<QWORD/*teamid*/, xPos> m_mapTeamPos;
  std::map<QWORD/*teamid*/, string> m_mapTeamName;
  QWORD m_qwWinTeamId = 0;
  DWORD m_dwShouResultTime = 0;
  ECalcState m_calcState = ECALCSTATE_FIGHTING;
};

// pvp波利乱斗

struct ScoreInfo
{
  QWORD qwCharId = 0;
  string strName;
  DWORD dwScore = 0;
  bool bCanPickUp = true;
  
  void init(QWORD charId, const std::string&name, DWORD defaultScore);
  void resetRank()
  {
    dwRank = 9999;              //没有名次
    dwMaxScore = 0;
    qwScoreStartTime = 0;
    mapKeepDuration.clear();
  }
  void setNewSocre(DWORD newScore);
  void refreshKeepTime(QWORD msec);

  DWORD dwRank = 9999;               //当前拍名
  DWORD dwMaxScore = 0;               //最大苹果数
  QWORD qwScoreStartTime = 0;         //最大苹果数开始时间，毫秒
  QWORD qwNextPickTime = 0;           // 每次拾取后增加CD
  QWORD qwDropAppleTime = 0;          //掉落苹果时间
  std::map<DWORD/*score*/, QWORD/*duraction*/> mapKeepDuration;    //积分保持时间，毫秒
  SceneUser* pUser = nullptr;
};

struct RankFun {   //积分从大到小排序，积分一样，找最大积分
  bool operator()(ScoreInfo* lhs, ScoreInfo* rhs) {
    if (lhs->dwScore > rhs->dwScore)
      return true;
    else if (lhs->dwScore == rhs->dwScore)
    {
      if (lhs->dwMaxScore > rhs->dwMaxScore)
        return true;
      else if (lhs->dwMaxScore == rhs->dwMaxScore)
      {
        if (lhs->mapKeepDuration[lhs->dwMaxScore] > rhs->mapKeepDuration[rhs->dwMaxScore])
          return true;
      }
    }
    return false;
  }
};

class PollyScene :public PvpBaseScene
{
public:
  PollyScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
  virtual ~PollyScene();

  virtual bool init();
  virtual void entryAction(QWORD curMSec);
  virtual void userEnter(SceneUser *user);
  virtual void userLeave(SceneUser *user);
  virtual void getBornPos(SceneUser* pUser, xPos& pos);
  virtual bool isPollyScene() { return true; }
  virtual bool isHideUser() { return true; }

public:
  DWORD getScore(SceneUser* user);
  QWORD getNextPickAppleTime(SceneUser* user);
  void setScore(SceneUser* user, DWORD newScore);
  void markDropApple(SceneUser* user);
  void notify(QWORD id);
  void notifyAllUser();
  bool canPickUp(QWORD charid);
  bool canPickUpApple(QWORD charid);
  virtual void onLeaveScene(SceneUser* user);// 在userleave之前调用, 避免先通知客户端离开地图

private:
  ScoreInfo* getScoreInfo(QWORD charId);
  bool checkEnd(DWORD curSec);
  void processResult();
  void sendResult(SceneUser* pUser, ScoreInfo& rInfo, PvpResultCCmd& cmd);
  bool rank();
  void sendRank2Client();

private:
  DWORD m_dwStartTime = 0;
  DWORD m_dwEndTime = 0;
  DWORD m_dwMsgTime = 0;
  ECalcState m_calcState = ECALCSTATE_FIGHTING;
  
  bool m_bScoreChanged = false;
  DWORD m_dwMaxScore = 0;   //当前最高的积分
  std::map<QWORD/*charid*/,ScoreInfo> m_mapScoreInfo;
  std::vector<ScoreInfo> m_vecTopRank;    //前三名
  DWORD m_dwNextRankTime = 0;
  bool m_bNeedRank = false;
  DWORD m_dwProtectTime = 0;
  TSetQWORD m_setEntered;     //曾经进来过的人
};

// guild raid scene
class GuildRaidScene : public DScene
{
  public:
    GuildRaidScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, QWORD guildid, QWORD teamid, DWORD dwGuildRaidIndex);
    virtual ~GuildRaidScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_GUILD_RAID; }
    virtual bool init();
    virtual void entryAction(QWORD curMSec);
    virtual void userLeave(SceneUser *user);
    virtual void userEnter(SceneUser *user);

    virtual void getRandomDeadBoss(std::list<pair<DWORD,DWORD>>& bosslist);
    virtual bool checkSummonDeadBoss();
    DWORD getMapIndex() { return m_dwGuildRaidIndex; }
    void checkExit(const ExitPoint* pPoint);
    bool goNextMap(const ExitPoint* pPoint);
    DWORD getGuildID() const { return m_qwGuildID; }
  private:
    void checkExit();
    void closeGuildRaid();
  private:
    QWORD m_qwGuildID = 0;
    QWORD m_qwTeamID = 0;
    DWORD m_dwGuildRaidIndex = 0;
    DWORD m_dwCurEpID = 0;
    DWORD m_dwExitCheckTime = 0;

    bool m_bGoOtherGRaid = false;

  public:
    // 记录副本召唤出的boss id 用于后续副本奖励
    void setSummonBossID(DWORD bossid) { m_dwTempCurBossID = bossid; }
    DWORD getSummonBossID() { return m_dwTempCurBossID; }
    void summonRewardBox(SceneUser* pUser);
  private:
    DWORD m_dwTempCurBossID = 0;
};

// 约会圣地
class DateLandScene : public DScene
{
  public:
    DateLandScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~DateLandScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer) {}
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer) {}

    virtual void onClose() {}
};

enum EGuildFireState
{
  EGUILDFIRE_MIN = 0,
  EGUILDFIRE_PEACE = 1,
  EGUILDFIRE_FIRE = 2,
  EGUILDFIRE_CALM = 3,
  EUIGLDFIRE_WAITEND = 4, // 特殊胜利后, 等待公会战时间结束
};

class GuildCity;
class GuildFireScene : public DScene
{
  struct SGvgSafeArea
  {
    xPos pos1;
    xPos pos2;

    float fWidth = 0.0f;
    /*
     *
     * ___________________________
     *|                           |
     *|                           |
     *|p1                      p2 | w
     *|                           |
     *|___________________________|
     *
    */
  };

  public:
    GuildFireScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD dwMapIndex, bool bFireOpen);
    virtual ~GuildFireScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_GUILD_FIRE; }
    virtual bool isGvg() { return true; }
    virtual bool init();
    virtual void entryAction(QWORD curMSec);
    virtual void userLeave(SceneUser *user);
    virtual void userEnter(SceneUser *user);
    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    virtual void onClose();
    virtual bool isHideUser() { return true; }
    virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath);
    virtual bool isUserCanFireScene() { return true; }
    virtual void onNpcBeAttack(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp);
    virtual void onNpcBeHeal(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp);

  public:
    bool checkMonsterCity();
    EGuildFireState getFireState();
    DWORD getCityID() { return m_dwMapIndex; }
    QWORD getDefenseGuildID();
    void onSummonMetalNpc(QWORD npcid);
  private:
    DWORD m_dwMapIndex = 0;
    GuildCity* m_pCity = nullptr;

    std::vector<SGvgSafeArea> m_vecSafeArea;
    xTimer m_oOneSecTimer;
};

enum EWeddingState
{
  EWEDDINGSTATE_WAITSTART, //等待邀请开始
  EWEDDINGSTATE_START, //邀请成功,等待婚礼双方移动到指定位置
  EWEDDINGSTATE_QUESTION_0, //双方到达, 进入答题模式, 前言
  EWEDDINGSTATE_QUESTION_1, //双方到达, 进入答题模式, 第一阶段
  EWEDDINGSTATE_QUESTION_2, //双方到达, 进入答题模式, 第二阶段
  EWEDDINGSTATE_FINISH, //完成婚礼
};

class WeddingScene : public DScene
{
  public:
    WeddingScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, QWORD qwWeddingID);
    virtual ~WeddingScene() {};

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_WEDDING_SCENE; }
    virtual bool init();

    virtual void userEnter(SceneUser* user);
    virtual void userLeave(SceneUser* user);

    const WeddingInfo& getWeddingInfo() const { return m_oWeddingInfo; }
    // 婚礼流程
  public:
    void onInviteOk(QWORD masterid, QWORD followerid);
    void onUserInPos(SceneUser* user);
    void onReceiveAnswer(SceneUser* user, DWORD questionid, DWORD answer);
    void onUserQuitQuestion(SceneUser* user);
    void onAddService(const TSetDWORD& ids);
    void updateManual(const WeddingManualInfo& info);

  private:
    bool isWeddingUser(SceneUser* user);
    void sendCmdToWeddingUser(const void* cmd, DWORD len);
    void processQuestion_0();
    void processQuestion_1();
    void processQuestion_2();
    void onFinishWedding();
    bool isInAnswer() { return m_eWeddingState >= EWEDDINGSTATE_QUESTION_0 && m_eWeddingState <= EWEDDINGSTATE_QUESTION_2; }

    void resetWedding();
    void resetQuestion();

    void sendChatQuestion(DWORD questionid);
    void sendResult(bool ret);
    void doServiceEffect(bool marry = false);
  private:
    QWORD m_qwWeddingID = 0;
    WeddingInfo m_oWeddingInfo;

    EWeddingState m_eWeddingState = EWEDDINGSTATE_WAITSTART;
    QWORD m_qwTempOnPosUser = 0;
    QWORD m_qwMasterID = 0;
    QWORD m_qwFollowerID = 0;

    DWORD m_dwCurQuestionIndex = 0;
    QWORD m_qwCurAnswerUserID = 0;
    QWORD m_qwTempCeremonyNpcID = 0;
};

// 离婚过山车 scene
class DivorceRollerCoasterScene : public DScene
{
public:
  DivorceRollerCoasterScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
  virtual ~DivorceRollerCoasterScene() {}

  virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
  virtual bool init();

  virtual void userEnter(SceneUser *user);
  virtual void userLeave(SceneUser *user);

  virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer) {}
  virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer) {}

  virtual void onClose() {}
private:
  QWORD m_qwCarrierMasterID = 0;
};

enum EPveCardState
{
  EPVECARDSTATE_MIN = 0, // 未选牌
  EPVECARDSTATE_SELECTOK = 1, //已选牌
  EPVECARDSTATE_PLAYCARD = 2, // 敌方npc打牌
  EPVECARDSTATE_FRIENDPLAYCARD = 3, //友方npc打牌
  EPVECARDSTATE_FINISHCARD = 4, // 打牌结束
  EPVECARDSTATE_OVER = 5, // 已获取奖励等待副本关闭
  EPVECARDSTATE_DEADBOSS = 6, // 等待召唤boss
  EPVECARDSTATE_BOSSOVER = 7, // 等待击杀boss
};

class PveCardScene : public DScene
{
  public:
    PveCardScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD configid);
    virtual ~PveCardScene() {}
    virtual bool init();

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_PVECARD; }

    virtual void entryAction(QWORD curMSec);
    virtual void userEnter(SceneUser *user);
    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer);

    virtual void getRandomDeadBoss(std::list<pair<DWORD,DWORD>>& bosslist);
    virtual bool checkSummonDeadBoss();
  public:
    void queryAllCardInfo(SceneUser* user);
    bool selectCard(SceneUser* user, DWORD index);
    bool beginPlayCard(SceneUser* user);
    const xPos& getCurCardPos() { return m_oTempCurCardPos; }
    DWORD getDifficulty() const;
  private:
    void playCard();
    void friendPlayCard();
    bool doCardEffect(DWORD cardid);
    void summonCardNpc(bool isFriend); // 招怪前(卡牌效果生效前), 召唤表现NPC
    void showCardNpc(); // 招怪时, 播放卡牌动作, 删除卡牌npc
    void sendDialogToUser(DWORD dialogid);
    void checkFinishReward();
    bool isEnemyCardOver() { return m_dwCardIndex == m_vecAllCards.size(); }
    void summonRewardBox(SceneUser* pUser, bool bNeedReward);
  private:
    DWORD m_dwPveRaidConfgID = 0;
    xTimer m_oOneSecTimer;
    TVecDWORD m_vecAllCards;
    DWORD m_dwCardIndex = 0;

    EPveCardState m_eState = EPVECARDSTATE_MIN;
    DWORD m_dwTimeTick = 0;

    QWORD m_qwEnemyNpcID = 0;
    QWORD m_qwFriendNpcID = 0;
    QWORD m_qwSandglassNpcID = 0; // 沙漏npc

    DWORD m_dwSafeRange = 0;
    xPos m_oTempCurCardPos;

    bool m_bSummonedCardNpc = false; // 记录
    TSetQWORD m_setSummonedCardNpc; // 召唤的表现卡牌npc guid

    DWORD m_dwShrinkIndex = 0; // 毒圈收缩次数
    QWORD m_qwPosionEffectID = 0; // 毒圈特效ID
};

struct SMvpBattleTeamData
{
  QWORD qwTeamID = 0;
  string strTeamName;

  DWORD dwKillUserNum = 0;
  TVecDWORD vecKillMvpIDs;
  TVecDWORD vecKillMiniIDs;
  TVecDWORD vecKillDeadBoss;

  void toData(MvpBattleTeamData* pData);
};

class MvpBattleScene : public MatchScene
{
  public:
    MvpBattleScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~MvpBattleScene() {}
    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_MVPBATTLE; }
    virtual bool init();
    virtual void entryAction(QWORD curMSec);

    virtual bool isPVPScene() { return true;};
    virtual bool isDPvpScene() { return true; };
    virtual bool isUserCanFireScene() { return true; }
    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);
    virtual void getBornPos(SceneUser* pUser, xPos& pos);

    virtual void onNpcDieReward(SceneNpc* npc, SceneUser* user);
    virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath);

    virtual void onReceiveRoomInfo(const Cmd::SyncRoomSceneMatchSCmd& cmd);
  public:
    void getRelivePos(SceneUser* pUser, xPos& pos);
    void onUserGetTeamInfo(SceneUser* user);
  private:
    void reportEndInfo();
    void summonDeadBoss();
  private:
    xTimer m_oOneSecTimer;
    DWORD m_dwTeamNum = 0; //匹配成功时的队伍人数
    QWORD m_qwRoomID = 0; // 房间id
    DWORD m_dwUserNum = 0;
    DWORD tempReliveIndex = 0;

    TVecDWORD m_vecLiveBossAndMini;
    TVecDWORD m_vecDieBossAndMini;
    TVecDWORD m_vecDeadBossIDs;
    TVecDWORD m_vecDieDeadBossIDs;
    std::map<DWORD/*color*/, xPos> m_mapTeamBornPos;
    std::map<QWORD/*teamid*/, xPos> m_mapTeamRelivePos;
    std::map<QWORD, SMvpBattleTeamData> m_mapTeamMvpData;/*队伍击杀信息*/
    TSetQWORD m_setClearCDTeams; /*记录已清除报名cd的队伍*/
    std::list<DWORD> m_listDeadBossSummonTime;
};

struct SSuperGvgGuildData
{
  QWORD qwGuildID = 0;

  DWORD dwColor = 0;
  QWORD qwMetalNpcID = 0; // 华丽金属guid
  DWORD dwCrystalNum = 0; // 水晶数量
  DWORD dwChipNum = 0; // 碎片数量
  QWORD qwGetScoreTime = 0; // 最近积分增加的时间,us
  DWORD dwRank = 0; // 排名
  DWORD dwBpID = 0; // bp点
  DWORD dwNpcShowActionID = 0; // 公会对应de占领圈动作id
  bool bMetalLive = true;
  DWORD dwExpelUserTime = 0;

  DWORD dwFireCount = 0;
  DWORD dwFireScore = 0;
  string strGuildName;
  string strGuildIcon;
  string strColorName;
  map<QWORD,DWORD> mapGuild2AttackTime;

  void toData(GvgCrystalInfo* pInfo);
  void toData(GvgGuildInfo* pInfo);
};

struct SGvgTowerData
{
  EGvgTowerType eType = EGVGTOWERTYPE_CORE;
  EGvgTowerState eState = EGVGTOWERSTATE_INITFREE;

  QWORD qwCurOwnerGuildID = 0; // 当前占领的公会
  QWORD qwLastGuildID; // 上一秒中最多人数公会
  QWORD qwShowNpcID = 0; // 光圈npc guid

  DWORD dwValue = 0; // 剩余的点数
  map<QWORD, DWORD> mapGuild2Value;

  float fRange = 0;
  xPos pos;
  std::set<SceneUser*> setSyncUsers; // 同步的玩家

  void toData(GvgTowerData* pData);
};

struct SSugvgUserData
{
  QWORD qwUserID = 0;
  QWORD qwGuildID = 0;
  DWORD dwProfession = 0;
  DWORD dwPartInTime = 0; // 参战时间
  string strUserName;

  DWORD dwKillUserNum = 0; // 击杀玩家数
  DWORD dwDieNum = 0; // 死亡数
  DWORD dwChipNum = 0; // 获取碎片数
  DWORD dwTowerTime = 0; // 占塔时间
  DWORD dwHealHp = 0; // 治疗量
  DWORD dwReliveNum = 0; // 复活其他玩家数
  DWORD dwMetalDamage = 0; // 对华丽金属造成伤害量

  void toData(SuperGvgUserData* pData);
};

class SuperGvgScene : public MatchScene
{
  enum ESGvgState
  {
    ESGVGSTATE_MIN = 0,
    ESGVGSTATE_PREPARE = 1,
    ESGVGSTATE_FIRE = 2,
    ESGVGSTATE_END = 3,
  };

  public:
    SuperGvgScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~SuperGvgScene();
    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_SUPERGVG; }
    virtual bool isGvg() { return true; }
    virtual bool isSuperGvg() { return true; }
    virtual bool isUserCanFireScene() { return true; }
    virtual bool canUseGoToPos() { return m_eSGvgState != ESGVGSTATE_PREPARE; }

    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser* user);

    virtual void entryAction(QWORD curMSec);
    virtual void onReceiveRoomInfo(const Cmd::SyncRoomSceneMatchSCmd& cmd);
    virtual void getBornPos(SceneUser* pUser, xPos& pos);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    virtual void onNpcBeAttack(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp);

    virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath);
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer);
    virtual void addHealUser(QWORD id, QWORD healID, DWORD hp);
    virtual void onReliveUser(SceneUser* user, SceneUser* reliver); //reliver复活user
  public:
    void queryTowerInfo(SceneUser* user, EGvgTowerType type, bool open);
    void queryUserData(SceneUser* user);
    void addPartinTime(SceneUser* user, DWORD time);
    void openFireAtonce(); // 测试使用
    bool needExpelDieUser(SceneUser* user);
    bool isInPrepare() const { return m_eSGvgState == ESGVGSTATE_PREPARE; }
  private:
    void syncAllDataToUser(SceneUser* user);
    void updateTower(DWORD cur);
    void onTakeTower(const SGvgTowerCFG& tower, QWORD guildid);
    void onLoseTower(const SGvgTowerCFG& tower, QWORD guildid, bool empty);
    void updateRank(bool bForce = false);
    bool checkWin();
    void onTimeUp();
    void onAddCrystal(const SSuperGvgGuildData& data);
    void onGetChip(SceneUser* user);
    void onDamageMeatal(SceneUser* user, DWORD hp);
    void reward();

    // 空气墙
    void clearBarrier();

    // 晶化boss
    void summonBoss(bool south);
    void addTimeBossMsg(DWORD time, DWORD msgid);
    void checkBossMsg(DWORD cur);

    // 场景buff等
    void doSceneEffect();
  private:
    xTimer m_oOneSecTimer;
    ESGvgState m_eSGvgState = ESGVGSTATE_MIN;
    bool m_bOpenRank = false; // 标记是否开启排名显示, 占塔、合成水晶、丢掉金属时开启
    DWORD m_dwOpenFireTime = 0;
    DWORD m_dwStopFireTime = 0;
    DWORD m_dwNorthBossTimeTick = 0; // 北部召唤boss
    DWORD m_dwSouthBossTimeTick = 0; // 南部召唤boss
    DWORD m_dwSceneEffectTimeTick = 0;
    DWORD m_dwTowerOpenTime = 0; // 塔开启时间戳
    QWORD m_qwRoomID = 0;
    DWORD m_dwRoomLevel = 0; // 房间段位
    map<QWORD, SSuperGvgGuildData> m_mapGuild2Data;
    map<EGvgTowerType, SGvgTowerData> m_mapTowerData;

    TSetDWORD m_setUpdateGuildCrystals;
    std::set<SceneUser*> m_setSyncUsers; // 打开界面需实时同步的玩家
    TSetQWORD m_setSouthBossIDs; // 南方召唤的bossid
    TSetQWORD m_setNorthBossIDs; // 北方召唤的bossid

    map<DWORD, TSetDWORD> m_mapTimeTick2BossMsg; // boss 出现的 msg提示
    map<QWORD, SSugvgUserData> m_mapSuGvgUserData; // 玩家个人数据
    map<QWORD, DWORD> m_mapUser2ColorIndex; // 记录玩家颜色
};

class AltmanScene : public DScene
{
  public:
    AltmanScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~AltmanScene() {}
    virtual bool init();

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_QUEST; }
    virtual bool isAltmanScene() { return true; }

    virtual void entryAction(QWORD curMSec);
    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);
    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer);
    virtual void onClose();

    void reward();
    void notify(SceneUser *user);
    virtual void onLeaveScene(SceneUser *user);
  private:
    void summonUserRewardBox(SceneUser* pUser);
  private:
    DWORD m_dwTimeTick = 0;
    bool m_bRewarded = false;
    DWORD m_dwDeadMonster = 0;
    map<QWORD, DWORD> m_mapUserKill;
};

struct SPwsUserData
{
  QWORD qwUserID = 0;
  string strName;
  UserPortraitData oPortrait;
  EProfession eProfession = EPROFESSION_MIN;;

  DWORD dwKillUserNum = 0; // 击杀玩家数
  DWORD dwDieNum = 0; // 死亡次数
  DWORD dwHealHp = 0; // 治疗
  DWORD dwKillScore = 0; // 击杀得分
  float fBallScore = 0; // 持球得分
  DWORD dwBuffScore = 0; // 拾取buff得分

  QWORD qwScoreTime = 0; // 最后一次得分时间, 队伍内排名, 同分时使用
  QWORD qwKillScoreTime = 0; // 最后一次击杀得分时间

  EMagicBallType eHoldBall = EMAGICBALL_MIN;
  DWORD dwBallBeginValueTime = 0; // 持球后过一段时间, 开始得分

  DWORD dwOriginScore = 0; // 参战前积分
  bool bJoined = false; // 是否进过副本
  bool bLeave = false; // 离开了队伍

  map<DWORD, QWORD> mapBuff2EndTime; // 仅在副本中添加的buff
  void toData(TeamPwsRaidUserInfo* pData);
};

typedef std::pair<EMagicBallType, EMagicBallType> TPairBallMagic;
class TeamPwsScene;
struct SPwsTeamData
{
  QWORD qwTeamID = 0;
  ETeamPwsColor eColor = ETEAMPWS_RED;
  string strTeamColorName;

  DWORD dwScore = 0; // 队伍积分
  DWORD dwBallScoreNextTime = 0;
  DWORD dwBallScoreRate = 0; // 当前持球数对应的每次加分
  DWORD dwBallMagicCD = 0;

  QWORD qwScoreTime = 0; // 最后一次得分时间, 队伍间排名, 同分时使用

  TPairBallMagic oPairMagic; // 记录当前队伍的魔法组合
  TPairBallMagic oPairBuffMagic; // 记录当前队伍携带的buff对应的魔法组合

  map<QWORD, SPwsUserData> mapUserDatas;
  TeamPwsScene* pScene = nullptr;

  SPwsUserData* getUserData(QWORD charid){
    auto it = mapUserDatas.find(charid);
    return it != mapUserDatas.end() ? &(it->second) : nullptr;
  }

  bool bUpdate = false; // 数据发生变化, 需要同步客户端
  bool bRelax = false; // 是否是休闲模式
  bool inMagic(EMagicBallType b1, EMagicBallType b2);
  bool hasMagic() const { return oPairMagic.first != EMAGICBALL_MIN; }
  bool hasMagicBuff() const { return oPairBuffMagic.first != EMAGICBALL_MIN; }
  void setUpdate() { bUpdate = true; }
  void addScore(DWORD add);

  void updateBallScore(DWORD cur);
  std::set<EMagicBallType> getAllBalls() const;
  void toData(TeamPwsInfoSyncData* pData); // 追踪栏常驻
  void toData(TeamPwsRaidTeamInfo* pTeamInfo); // 详细数据
  void createData(const TeamPwsRoomData& matchdata, bool relax);

  SPwsTeamData(TeamPwsScene* scene);
};

#define LOG_SCENE_INFO "副本:" << name << id << m_qwRoomID
class TeamPwsScene : public MatchScene
{
    enum ETeamPwsState
    {
      ETEAMPWS_MIN,
      ETEAMPWS_PREPARE,
      ETEAMPWS_FIRE,
      ETEAMPWS_END,
    };
  public:
    TeamPwsScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~TeamPwsScene();
    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_TEAMPWS; }
    virtual bool isUserCanFireScene() { return true; }
    virtual bool isPVPScene() { return true;};
    virtual bool isDPvpScene() { return true; };
    virtual bool isHideUser() { return true; }

    virtual bool init();

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser* user);

    virtual void entryAction(QWORD curMSec);
    virtual void onReceiveRoomInfo(const Cmd::SyncRoomSceneMatchSCmd& cmd);
    virtual void getBornPos(SceneUser* pUser, xPos& pos);

    //virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    //virtual void onNpcBeAttack(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp);

    virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath);
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer);
    virtual void addHealUser(QWORD id, QWORD healID, DWORD hp);
    //virtual void onReliveUser(SceneUser* user, SceneUser* reliver); //reliver复活user
    virtual void onLeaveScene(SceneUser* user);// 在userleave之前调用, 避免先通知客户端离开地图

  public:
    void onUserCollectBall(SceneUser* user, SceneNpc* npc);
    void onUserCollectBuff(SceneUser* user, SceneNpc* npc);
    void selectMagic(SceneUser* user, DWORD magicid);
    void queryDetailInfo(SceneUser* user);
    void onUserLeaveTeam(SceneUser* user);
    bool isInPrepare() { return m_eState == ETEAMPWS_PREPARE; }

  private:
    void onUserIn(SceneUser* user);
    void summonBall(EMagicBallType eType);
    void summonAllBall();
    void summonBuffNpc();
    void onTeamGetBall(SPwsTeamData* pTeam);
    void onTeamLoseBall(SPwsTeamData* pTeam, bool swap = false);
    void teamAddMagic(SPwsTeamData* pTeam, EMagicBallType ball1, EMagicBallType ball2);
    void teamDelMagic(SPwsTeamData* pTeam);
    void switchTeamMagicBuff(SPwsTeamData* pTeam, TPairBallMagic magic, bool add);
    void dropUserBall(SceneUser* user);
    void updateTeamInfoToCient();

    SPwsTeamData* getTeamData(QWORD teamid);
    SPwsTeamData* getTeamData(SceneUser* user);
    SPwsUserData* getUserData(SceneUser* user);
    void checkTeamMagicCD(DWORD cur);
    bool checkWin(bool bTimeOut = false);
    void onEnd();
  private:
    xTimer m_oOneSecTimer;
    DWORD m_qwRoomID = 0;
    ETeamPwsState m_eState = ETEAMPWS_MIN;
    DWORD m_dwTimeTick = 0;
    DWORD m_dwSummonBuffTick = 0;
    DWORD m_dwEndTime = 0;
    SPwsTeamData m_stRedTeam;
    SPwsTeamData m_stBlueTeam;
    map<EMagicBallType, pair<QWORD, xPos>> m_mapBallData; // 魔法球(guid,坐标点)
    bool m_bRelax = false;
};
