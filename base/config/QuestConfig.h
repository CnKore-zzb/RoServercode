/**
 * @file QuestConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-10-10
 */

#pragma once

#include "xSingleton.h"
#include "SceneQuest.pb.h"
#include "SceneManual.pb.h"
#include "SessionCmd.pb.h"
#include "xLuaTable.h"
#include "CommonConfig.h"

using namespace Cmd;
using std::vector;
using std::map;
using std::string;
using std::pair;

const DWORD QUEST_ID_PARAM = 10000;
const DWORD QUEST_CHRISTMAS = 393080001;
const DWORD QUEST_GRADUATION = 391020001;
const DWORD QUEST_WEDDING_HONEYMOON = 396190001;
const DWORD QUEST_WEDDING_WELCOME = 396050002;
const DWORD QUEST_PEAK_EFFECT_ID = 601580006;
const DWORD QUEST_DREAMMIRROR_1 = 99640007;
const DWORD QUEST_DREAMMIRROR_2 = 99660005;
const DWORD QUEST_DREAMMIRROR_3 = 99670005;

// config data
enum EQuestCond
{
  EQUESTCOND_MIN = 0,
  EQUESTCOND_MANUAL = 1,
  EQUESTCOND_MAX = 2,
};
enum EQuestReset
{
  EQUESTRESET_MIN = 0,
  EQUESTRESET_CHANGEMAP = 1,
  EQUESTRESET_RELOGIN = 2,
  EQUESTRESET_MAX
};
enum EQuestRefresh
{
  EQUESTREFRESH_MIN = 0,
  EQUESTREFRESH_DAY = 1,
  EQUESTREFRESH_WEEK = 2,
  EQUESTREFRESH_PERIOD = 3,
  EQUESTREFRESH_MAX,
};
struct SQuestStepCFG
{
  xLuaData data;
  QuestPConfig oConfig;
};
struct SGuildReq
{
  EGuildBuilding eType = EGUILDBUILDING_MIN;
  DWORD dwAuth = 0;
  TVecItemData vecDatas;
};
struct SManualReq
{
  EManualType eType = EMANUALTYPE_MIN;
  EManualStatus eStatus = EMANUALSTATUS_MIN;

  TSetDWORD setIDs;
};
struct SQuestCFG
{
  DWORD id = 0;

  DWORD lv = 0;
  DWORD joblv = 0;
  DWORD manuallv = 0;
  DWORD mapid = 0;
  DWORD rewardGroupID = 0;
  DWORD gender = 0;
  DWORD toBranch = 0;
  DWORD dwPrefixion = 0;

  DWORD starttime = 0;
  DWORD endtime = 0;
  TSetDWORD setValidWeekday;
  struct tm startsend;
  struct tm endsend;

  DWORD version = 0;

  EQuestType eType = EQUESTTYPE_MIN;

  TVecDWORD vecBranch;
  vector<EProfession> vecProfession;
  EProfession eDestProfession = EPROFESSION_MIN;

  TVecDWORD vecPreQuest;
  TVecDWORD vecMustPreQuest;
  DWORD cookerLv = 0;
  DWORD tasterLv = 0;
  vector<SQuestStepCFG> vecStepData;
  SGuildReq stGuildReq;
  SManualReq stManualReq;

  string name;
  string questname;
  string manualVersion;

  EQuestRefresh eRefreshType = EQUESTREFRESH_MIN;
  DWORD refreshTimes = 0;

  SQuestCFG() {}
  virtual ~SQuestCFG() {}

  bool hasProfession(EProfession eProfession) const;
  bool checkProfession(EProfession eProfession) const;
  bool checkBranch(DWORD dwBranch) const;
  bool checkGender(DWORD dwGender) const;
  void initPConfig(QuestPConfig& rConfig, const xLuaData& data) const;

  bool toStepConfig(QuestStep* pStep, DWORD step, DWORD language = ELANGUAGE_CHINESE_SIMPLIFIED) const;
  bool toPreview(QuestStep* pStep, DWORD step, DWORD language = ELANGUAGE_CHINESE_SIMPLIFIED) const;
  bool isInTime(DWORD curTime) const;
  bool isInSendTime(DWORD curTime) const;

  bool toData(QuestData* pData, DWORD language = ELANGUAGE_CHINESE_SIMPLIFIED) const;
};
typedef vector<SQuestCFG> TVecQuestCFG;
typedef map<EQuestType, TVecQuestCFG> TMapTypeQuest;
typedef map<DWORD, QuestPreview> TMapQuestPreview;

static const TVecQuestCFG VEC_QUEST_EMPTY;
static const TMapQuestPreview MAP_QUEST_EMPTY;

// quest version
struct SQuestVersion
{
  string version;

  TVecQuestCFG vecMainCFG;
  TVecQuestCFG vecStoryCFG;
  map<DWORD, map<DWORD, TVecQuestCFG>> mapBranchCFG;

  TMapQuestPreview mapMainCFG;
  TMapQuestPreview mapStoryCFG;

  void findBranchPreQuest(DWORD questid, DWORD id, TVecQuestCFG& vecCFG);
};
typedef map<string, SQuestVersion> TMapQuestVersion;
struct SQuestPreQuest
{
  DWORD questid = 0;
  TMapTypeQuest mapTypeQuest;

  const TVecQuestCFG& getQuestList(EQuestType eType) const
  {
    auto m = mapTypeQuest.find(eType);
    return m != mapTypeQuest.end() ? m->second : VEC_QUEST_EMPTY;
  }
};
typedef map<DWORD, SQuestPreQuest> TMapPreQuest;

// detail data
typedef pair<DWORD, TVecDWORD> TPairQuestDetail;
typedef vector<TPairQuestDetail> TVecQuestDetailCFG;

// wanted quest
struct SWantedItem
{
  DWORD dwQuestID = 0;
  DWORD dwBaseExp = 0;
  DWORD dwJobExp = 0;
  DWORD dwGold = 0;
  DWORD dwRob = 0;
  DWORD dwReward = 0;
  bool bTeamSync = false;
  string strName;
  bool bActivity = false;

  EWantedType eType = EWANTEDTYPE_TOTAL;

  TVecItemInfo vecRewards;

  SWantedItem() {}
};
typedef vector<SWantedItem> TVecWantedItem;
struct SWantedQuest
{
  DWORD dwWantedID = 0;

  TVecWantedItem arrCurItem[EWANTEDTYPE_MAX];
  TVecWantedItem arrAllItem[EWANTEDTYPE_MAX];

  SWantedQuest() {}

  //const SWantedItem* getWantedItem(DWORD questid) const;
};
typedef vector<SWantedQuest> TVecWantedQuest;
struct SWantedQuestCFG
{
  pair<DWORD, DWORD> lvRange;

  TVecWantedQuest vecQuest;

  SWantedQuestCFG() {}

  const SWantedQuest* getWantedQuest(DWORD dwWantedID) const;
  const SWantedItem* getWantedItem(DWORD questid, bool bIncludeAll = false) const;
};
typedef vector<SWantedQuestCFG> TVecWantedQuestCFG;

struct SMapRandQuest
{
  DWORD dwMapID = 0;

  TVecQuestCFG vecCurCFG;
  TVecQuestCFG vecAllCFG;
};
typedef map<DWORD, SMapRandQuest> TMapMapRandQuestCFG;

// daily exp poll
struct SDailyGift
{
  float fRate = 0.0f;
  ItemInfo oItem;

  SDailyGift() {}
};
typedef vector<SDailyGift> TVecDailyGift;
struct SDailyExpPool
{
  DWORD dwLevel = 0;
  DWORD dwExp = 0;
  DWORD dwSingleCost = 0;
  DWORD dwSubmitBaseExp = 0;
  DWORD dwSubmitJobExp = 0;
  DWORD dwWantedBaseExp = 0;
  DWORD dwWantedJobExp = 0;
  DWORD dwWantedRob = 0;

  TVecDailyGift vecProcessGift;

  SDailyExpPool() {}
};
typedef map<DWORD, SDailyExpPool> TMapDailyExpPool;

// quest puzzle
struct SQuestPuzzleCFG
{
  DWORD dwIndex = 0;
  TVecQuestCFG vecQuestCFG;
  TSetDWORD setRewards;
};
typedef map<DWORD, SQuestPuzzleCFG> TMapQuestPuzzleCFG;
struct SVersionPuzzleCFG
{
  string version;

  TMapQuestPuzzleCFG mapActivePuzzleCFG;
  TMapQuestPuzzleCFG mapCollectPuzzleCFG;

  const SQuestPuzzleCFG* getActivePuzzleCFG(DWORD index) const;
  const TMapQuestPuzzleCFG& getCollectPuzzleList() const { return mapCollectPuzzleCFG; }
};
typedef map<string, SVersionPuzzleCFG> TMapVersionPuzzleCFG;

struct SQuestMainStoryCFG
{
  DWORD dwID = 0;
  TSetDWORD setQuestIDs;
};
typedef map<DWORD, SQuestMainStoryCFG> TMapQuestMainStoryCFG;

// config
class QuestConfig : public xSingleton<QuestConfig>
{
  friend class xSingleton<QuestConfig>;
  private:
    QuestConfig();
  public:
    virtual ~QuestConfig();

    bool loadConfig();
    bool checkConfig();

    void setWantedTime(DWORD dwTime) { m_dwWantedTime = dwTime; }
    DWORD getWantedTime() const { return m_dwWantedTime; }

    void setRefreshIndex(DWORD dwIndex) { m_dwRefreshIndex = dwIndex; }
    DWORD getRefreshIndex() const { return m_dwRefreshIndex; }

    void setWantedNextTime(DWORD dwNextTime) { m_dwNextTime = dwNextTime; }
    DWORD getWantedNextTime() const { return m_dwNextTime; }

    const SQuestCFG* getQuestCFG(DWORD id) const;
    const SWantedQuestCFG* getWantedQuestCFG(DWORD lv) const;
    const SDailyExpPool* getExpPool(DWORD lv) const;
    const SWantedItem* getWantedItemCFG(DWORD questid, bool bActivity = false) const;
    const SQuestVersion* getVersionCFG(const string& version) const;
    const SQuestPreQuest* getQuestCFGByPre(DWORD questid) const;
    const SVersionPuzzleCFG* getQuestPuzzleCFG(const string& version) const;

    const TVecDWORD& getQuestDetail(DWORD id) const;
    const TVecQuestCFG& getQuestCFGList() const { return m_vecQuestCFG; }
    const TVecWantedQuestCFG& getWantedQuestList() const { return m_vecWantedQuestCFG; }
    const TMapMapRandQuestCFG& getMapRandQuestList() const { return m_mapMapRandCFG; }
    const TSetDWORD& getArtifactPieceList() const { return m_setArtifactPiece; }
    const TMapVersionPuzzleCFG& getVersionPuzzleList() const { return m_mapQuestPuzzleCFG; }
    const TMapQuestMainStoryCFG& getMainStoryCFGList() const { return m_mapMainStoryCFG; }

    const TVecQuestCFG* getTypeQuestCFGList(EQuestType eType) const;
    const TVecQuestCFG* getCondQuestCFGList(EQuestCond eCond) const;

    void randomWantedQuest(bool isIgnoreTime = false);
    bool randomMapRandQuest();
    void setWantedActive(bool bActive, DWORD dwCount);
    bool getActiveWanted() const { return m_bWantedActiveEnable; }
    DWORD getActiveMaxWantedCount() const { return m_dwWantedActiveCount; }

    bool addReplace(DWORD index, const TSetDWORD& setRwds);
    bool delReplace(DWORD index);

    EQuestType getQuestType(const string& type) const;

    void collectQuestGroupReward(DWORD dwGroupID, TSetDWORD& setRewardIDs);

    bool isAccQuest(EQuestType eType) const;
    bool isShareQuest(EQuestType eType) const;
    bool isArtifactPiece(DWORD dwID) const { return m_setArtifactPiece.find(dwID) != m_setArtifactPiece.end(); }

    bool isRealWantedQuest(DWORD id);
    const SWantedQuestCFG* getActivityWantedQuestCFG(DWORD lv) const;
    void debugVersion();
  private:
    bool loadQuestConfig();
    bool loadQuestTable(const string& str);
    bool loadWantedQuestConfig();
    bool loadDialogConfig();
    bool loadDailyExpPool();
    bool loadPuzzleConfig();
    bool loadMainStoryConfig();
  private:
    TVecQuestCFG m_vecQuestCFG;
    TVecQuestDetailCFG m_vecQuestDetailCFG;
    TVecWantedQuestCFG m_vecWantedQuestCFG;
    TVecWantedQuestCFG m_vecActivityWantedQuestCFG;
    TMapMapRandQuestCFG m_mapMapRandCFG;

    TMapDailyExpPool m_mapDailyExpPool;

    DWORD m_dwWantedTime = 0;
    DWORD m_dwNextTime = 0;
    DWORD m_dwRefreshIndex = 0;
    DWORD m_dwNextMapRandTime = 0;

    bool m_bWantedActiveEnable = false;
    DWORD m_dwWantedActiveCount = 0;

    map<DWORD, TSetDWORD> m_mapIndex2RepRewards;
    map<DWORD, TSetDWORD> m_mapGroupRewards;

    TSetDWORD m_setArtifactPiece;
    TMapTypeQuest m_mapTypeQuestCFG;
    map<EQuestCond, TVecQuestCFG> m_mapCondQuestCFG;

    TMapQuestVersion m_mapQuestVersion;
    TMapPreQuest m_mapPreQuest;
    TMapVersionPuzzleCFG m_mapQuestPuzzleCFG;
    TMapQuestMainStoryCFG m_mapMainStoryCFG;
};

