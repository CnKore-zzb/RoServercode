/**
 * @file MiscConfig.h
 * @brief misc config
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version V1
 * @date 2015-05-13
 */

#pragma once

#include "xSingleton.h"
#include "xPos.h"
#include "GuildCmd.pb.h"
#include "SessionTeam.pb.h"
#include "xLuaTable.h"
#include "SceneUser2.pb.h"
#include "SysMsg.pb.h"
#include "ProtoCommon.pb.h"
#include "SceneQuest.pb.h"
#include "ItemConfig.h"
#include "AstrolabeCmd.pb.h"
#include "SceneFood.pb.h"
#include "BaseConfig.h"
#include "FuBenCmd.pb.h"
#include "Var.pb.h"
#include "SkillConfig.h"
#include "MatchCCmd.pb.h"

using namespace Cmd;
using std::vector;
using std::pair;
using std::string;
using std::map;
using std::set;

// relive config
struct SReliveCFG
{
  DWORD dwReliveHpPercent = 0;

  TVecItemInfo vecConsume;
  TVecDWORD vecBuff;

  SReliveCFG() {}
};

// scene item config
struct SDropRange
{
  DWORD dwMin = 0;
  DWORD dwMax = 0;

  float fRange = 0.0f;
};
typedef vector<SDropRange> TVecDropRange;
struct SSceneItemCFG
{
  QWORD qwPickupInterval = 0;

  DWORD dwDisappearTime = 0;
  DWORD dwOwnerTime = 0;
  DWORD dwDropInterval = 0;
  DWORD dwDropRadius = 0;
  DWORD dwPickEffectRange = 0;
  float fTeamValidRange = 0;

  TVecDropRange vecRange;

  SSceneItemCFG() {}

  float getRange(DWORD dwCount) const;
};

// scene purify config
struct SPurifyCFG
{
  DWORD dwItemDisTime = 0;
  DWORD dwItemDropInterval = 0;
  DWORD dwPurifyRange = 0;
  DWORD dwMaxPurify = 0;
  DWORD dwGainInterval = 0;
};

// new role
struct SNewRoleCFG
{
  TVecDWORD vecMalePortrait;
  TVecDWORD vecFemalePortrait;

  TVecDWORD vecFrame;
  TVecDWORD vecHair;
  TVecDWORD vecEye;
  TVecDWORD vecAction;
  TVecDWORD vecExpression;

  TVecItemInfo vecItems;
  TVecDWORD vecBuffs;

  TVecDWORD vecFirstShortcut;
  TVecDWORD vecActiveMap;
  TVecDWORD vecMapArea;

  TVecDWORD vecManualCard;

  DWORD dwCollectSkill = 0;
  DWORD dwTransSkill = 0;
  //DWORD dwRepairSkill = 0;
  DWORD dwFlashSkill = 0;
  DWORD dwPurify = 0;

  DWORD dwMaxShortCut = 0;

  pair<DWORD, DWORD> pairDefaultHair;   // first : male  second : female
  pair<DWORD, DWORD> pairDefaultEye;    // first : male  second : female

  TVecDWORD vecManualItems;
  TVecDWORD vecManualNpcs;

  EQueryType eDefaultQueryType = EQUERYTYPE_MIN;
  map<DWORD, TSetDWORD> mapClassInitBuff;

  const TSetDWORD& getClassInitBuff(DWORD professionid) const;
  DWORD getDefaultHair(EGender eGender) const { return eGender == EGENDER_MALE ? pairDefaultHair.first : pairDefaultHair.second; }
  DWORD getDefaultEye(EGender eGender) const { return eGender == EGENDER_MALE ? pairDefaultEye.first : pairDefaultEye.second; }
  SNewRoleCFG() {}
};

// team
struct STeamCFG
{
  DWORD dwMaxMember = 0;
  DWORD dwMaxInvite = 0;
  DWORD dwInviteTime = 0;
  DWORD dwApplyTime = 0;
  DWORD dwOverTime = 0;
  DWORD dwQuickEnterTime = 0;

  DWORD dwDefaultType = 0;
  DWORD dwDefaultMinLv = 0;
  DWORD dwDefaultMaxLv = 0;
  EAutoType eDefaultAuto = EAUTOTYPE_CLOSE;

  string sDefaultName;
  DWORD dwPickupMode = 1;

  STeamCFG() {}
};

// equip decompose
/*struct SEquipDecomposeCFG
{
  EQualityType eQuality = EQUALITYTYPE_MIN;
  EItemType eType = EITEMTYPE_MIN;

  TVecItemInfo vecProduct;
};
typedef vector<SEquipDecomposeCFG> TVecEquipDecomposeCFG;*/

// package
enum EPackFunc
{
  EPACKFUNC_MIN = 0,
  EPACKFUNC_PRODUCE = 1,
  EPACKFUNC_UPGRADE = 2,
  EPACKFUNC_EQUIPEXCHANGE = 3,
  EPACKFUNC_EXCHANGE = 4,
  EPACKFUNC_REFINE = 5,
  EPACKFUNC_REPAIR = 6,
  EPACKFUNC_ENCHANT = 7,
  EPACKFUNC_GUILDDONATE = 8,
  EPACKFUNC_RESTORE = 9,
  EPACKFUNC_PETWORK = 10,
  EPACKFUNC_SHOP = 11,
  EPACKFUNC_QUEST_RANDITEM = 12,
  EPACKFUNC_GUILDBUILDING = 13,
  EPACKFUNC_EXCHANGESHOP = 14, //兑换商店 追赶系统
  EPACKFUNC_EQUIPCOMPOSE = 15,
};
typedef vector<EItemType> TVecItemType;
struct SPackageCFG
{
  DWORD dwMaxStoreCount = 0;
  TVecItemType vecSortType;

  TSetDWORD setPackFuncDefault;
  map<EPackFunc, TSetDWORD> mapPackFunc;

  DWORD getIndex(EItemType type) const;
  const TSetDWORD& getPackFunc(EPackFunc eFunc) const;
};

// card
//typedef vector<EItemType> TVecItemType;
struct SCardPositionCFG
{
  DWORD dwPosition = 0;

  TVecItemType vecItemType;

  SCardPositionCFG() {}
};
typedef vector<SCardPositionCFG> TVecCardPosition;
struct SCardCFG
{
  TVecCardPosition vecCardPosition;

  SCardCFG() {}
};

// quest
struct SDailyPerDay
{
  DWORD dwSubmitCount = 0;
  DWORD dwAcceptCount = 0;
};
struct SQuickBoardItem
{
  DWORD dwItemID = 0;
  pair<DWORD, DWORD> lvrange;
};
struct SQuestMiscCFG
{
  DWORD arrMaxWanted[EWANTEDTYPE_MAX];
  DWORD dwResetProtectTime = 0;

  DWORD dwDailyIncrease = 0;
  DWORD dwMaxDailyCount = 0;
  DWORD dwMaxMapCount = 0;
  DWORD dwTeamFinishBoardQuestCD = 0;

  map<DWORD, SDailyPerDay> mapDailyPerDay;
  map<DWORD, string> mapPrefixion;

  TVecDWORD vecWantedRefresh;
  TVecFloat vecWantedParams;
  vector<SQuickBoardItem> vecBoardQuickFinishItems;

  set<EQuestType> setManualMainType;
  set<EQuestType> setManualStoryType;

  SQuestMiscCFG() {}

  float getWantedParams(DWORD count) const;
  DWORD getMaxWantedCount(EWantedType eType) const;
  const SDailyPerDay* getDailyCount(DWORD dwMapID) const;

  bool isManualMain(EQuestType eType) const { return setManualMainType.find(eType) != setManualMainType.end(); }
  bool isManualStory(EQuestType eType) const { return setManualStoryType.find(eType) != setManualStoryType.end(); }

  const string& getPrefixion(DWORD dwIndex) const;
};

// quest table
struct SQuestTableCFG
{
  TVecString vecTables;

  SQuestTableCFG() {}
};

// Timer table
struct STimerTableCFG
{
  TVecString vecTables;

  STimerTableCFG() {}
};

// infinite tower config
struct SEndlessBox
{
  pair<DWORD, DWORD> pairFloor;

  xPos oPos;

  SEndlessBox() {}
};
struct SEndlessTowerCFG
{
  DWORD dwDefaultFloor = 0;
  //DWORD dwUpdateTimeSecDiff = 0;
  DWORD dwMaxSkillLv = 0;
  DWORD dwLimitUserLv = 0;

  float fRewardBoxRange = 0.0f;
  DWORD dwRewardBoxGetAction = 0;
  DWORD dwRewardBoxUngetAction = 0;
  DWORD dwBossFloorEffect = 0;
  string strRewardBoxEffect;
  xLuaData oRewardBoxDefine;

  DWORD dwDeadBossSummonTime = 0; // 亡者boss次数
  DWORD dwDeadBossUID = 0; // 亡者boss uniqueid
  DWORD dwDeadBossNum = 0; // 每次召唤数量
  DWORD dwDeadBossLayer = 0; // 召唤层数

  pair<float, float> pairMiniScale;
  vector<SEndlessBox> vecBosPos;

  DWORD dwRecordLayer = 0; // 无限塔记录层数
  SEndlessTowerCFG()
  {
    dwDefaultFloor = 20;          // 20 层
    //dwUpdateTimeSecDiff = 604800; // 7  天
  }

  const xPos& getPos(DWORD dwFloor) const;
};

// seal
struct SealMiscCFG
{
  DWORD dwSkillID = 0;
  DWORD dwSealNpcID = 0;
  DWORD dwSealRefresh = 0;
  DWORD dwChangePosTime = 0;
  DWORD dwMaxDaySealNum = 0;
  DWORD dwRewardDispTime = 0;
  DWORD dwSpeed = 0;
  DWORD dwSealNextTime = 0;
  DWORD dwSealRange = 0;
  DWORD dwRewardRange = 0;
  DWORD dwDropDelay = 0;
  DWORD dwPreActionTime = 0;
  DWORD dwCountDownTime = 0;
  string strGMShakeScreen;
  DWORD dwQuickFinishBuff = 0;
  DWORD dwWaitTime = 0;

  map<DWORD, DWORD> mapMapLv;
  vector<pair<DWORD, DWORD>> vecQuickFinishItems;

  SealMiscCFG() {}
  DWORD getSealLv(DWORD dwMapID) const;
};

// refineRate
typedef vector<pair<DWORD, DWORD>> TVecItem2Rate;

// refine
struct SRefineActionCFG
{
  DWORD dwNpcAction = 0;
  DWORD dwGuildNpcAction = 0;
  TVecDWORD vecSuccessEmoji;
  TVecDWORD vecFailEmoji;
  string strSuccessEffect;
  string strFailEffect;
  DWORD dwDelayMSec = 0;

  DWORD dwBeginChangeRateLv = 0;
  float fLastSuccessDecRate = 0.0f;
  float fLastFailAddRate = 0.0f;
  float fRepairPerRate = 0.0f;

  DWORD dwRepairMaxLv = 0;
};

// strength
struct SStrengthActionCFG
{
  DWORD dwGuildNpcAction = 0;
};

// system
enum ENameType
{
  ENAMETYPE_MIN = 0,
  ENAMETYPE_USER = 1,
  ENAMETYPE_GUILD = 2,
  ENAMETYPE_GUILD_JOB = 3,
  ENAMETYPE_GUILD_RECRUIT = 4,
  ENAMETYPE_GUILD_BOARD = 5,
  ENAMETYPE_TEAM = 6,
  ENAMETYPE_ROOM = 7,
  ENAMETYPE_PET = 8,
  ENAMETYPE_RECORD = 9,
  ENAMETYPE_BOOTH = 10,
  ENAMETYPE_MAX,
};

enum EGameTimeType
{
  EGAMETIMETYPE_MIN = 0,
  EGAMETIMETYPE_DAWN = 1,
  EGAMETIMETYPE_DAYTIME = 2,
  EGAMETIMETYPE_DUSK = 3,
  EGAMETIMETYPE_NIGHT = 4,
  EGAMETIMETYPE_MAX = 5,
};

struct SSystemCFG
{
  DWORD dwTimeSpeed = 0;
  DWORD dwGameDaySec = 0;

  DWORD dwMaxBaseLv = 0;
  DWORD dwMaxAttrPoint = 0;

  DWORD dwMusicBoxNpc = 0;
  DWORD dwMusicStatusPlay = 0;
  DWORD dwMusicStatusStop = 0;
  DWORD dwMusicRange = 0;

  DWORD dwNameMinSize = 0;
  DWORD dwNameMaxSize = 0;

  DWORD dwGuildNameMinSize = 0;
  DWORD dwGuildNameMaxSize = 0;

  DWORD dwGuildBoardMaxSize = 0;
  DWORD dwGuildRecruitMaxSize = 0;

  DWORD dwGuildJobMinSize = 0;
  DWORD dwGuildJobMaxSize = 0;

  DWORD dwBattleInterval = 0;

  TVecString vecForbidSymbol;
  TVecString vecForbidWord;
  TVecString vecForbidName;

  DWORD dwItemShowTime = 0;
  DWORD dwHandRange = 0;
  DWORD dwGoMapBuff = 0;

  DWORD dwTeamNameMinSize = 0;
  DWORD dwTeamNameMaxSize = 0;

  DWORD dwRoomNameMinSize = 0;
  DWORD dwRoomNameMaxSize = 0;

  DWORD dwPetNameMinSize = 0;
  DWORD dwPetNameMaxSize = 0;

  DWORD dwRecordNameMinSize = 0;
  DWORD dwRecordNameMaxSize = 0;

  DWORD dwBoothNameMinSize = 0;
  DWORD dwBoothNameMaxSize = 0;

  TVecDWORD vecBarrageMap;

  DWORD dwMaxMusicBattleTime = 0;
  DWORD dwMusicCheckTime = 0;
  DWORD dwMusicContinueTime = 0;
  DWORD dwMusicReturnTime = 0;
  DWORD dwMaxBaseBattleTime = 0;
  DWORD dwMusicBuff = 0;

  DWORD dwNewCharMapID = 0;
  DWORD dwMainCityMapID = 0;

  DWORD dwZoneBossLimitBuff = 0;

  EProfession eNewCharPro = EPROFESSION_MIN;

  DWORD dwSkyPassTime = 0;
  DWORD dwOpenStoreCost = 0;

  //DWORD dwMailOverTime = 0;
  DWORD dwSysMailOverTime = 0;

  bool bValidPosCheck = false;
  float fValidPosRadius = 0.0f;
  DWORD dwMaxExtraAddBattleTime = 0;
  DWORD dwMaxWeaponPetNum = 0;
  DWORD dwDebtMailID = 0;

  DWORD dwChatWorldReqLv = 0;
  DWORD dwSellLimitWarning = 0;
  DWORD dwNpcFadeoutTime = 0;

  DWORD dwKillMonsterPeriod = 0;
  DWORD dwShopRandomCnt = 0;
  DWORD dwStatKillNum = 0;// 杀怪数统计的最小数量

  DWORD dwOpenTowerLayer = 0;
  bool bMergeLineValid = false;

  float fMaxSeatDis = 10.0f;
  map<DWORD, DWORD> m_mapMapID2MaxScopeNum;
  map<DWORD, DWORD> m_mapMapID2MaxScopeFriendNum;

  SSystemCFG() {}

  bool isValidBarrageMap(DWORD dwMapID) const { return find(vecBarrageMap.begin(), vecBarrageMap.end(), dwMapID) != vecBarrageMap.end(); }

  ESysMsgID checkNameValid(const string& name, ENameType eType) const;
  bool isNight(DWORD curTime) const;
  DWORD getSkyTypeIndex(DWORD curTime) const;
  EGameTimeType getTimeType(DWORD curTime) const;
  DWORD getMapScopeNumByMap(DWORD mapid) const
  {
    auto it = m_mapMapID2MaxScopeNum.find(mapid);
    return it != m_mapMapID2MaxScopeNum.end() ? it->second: 0;
  };
  DWORD getMapScopeFriendNumByMap(DWORD mapid) const
  {
    auto it = m_mapMapID2MaxScopeFriendNum.find(mapid);
    return it != m_mapMapID2MaxScopeFriendNum.end() ? it->second: (DWORD)-1;
  };
};

// be lock influence flee/def
struct SLockEffectCFG
{
  DWORD dwRefreshInterval = 0;
  DWORD dwValidRange = 0;

  DWORD dwMinFleeLockNum = 0;
  DWORD dwMaxFleeLockNum = 0;
  DWORD dwMinDefLockNum = 0;
  DWORD dwMaxDefLockNum = 0;
  DWORD dwAutoHitTime = 0 ;
  DWORD dwChainAtkLmtTime = 0;

  float fFleeOneDecPer = 0.0f;
  float fDefOneDecPer = 0.0f;

  float getFleeDecPer(DWORD num) const
  {
    num = num > dwMaxFleeLockNum ? dwMaxFleeLockNum : num;
    return num > dwMinFleeLockNum ? ((num - dwMinFleeLockNum) * fFleeOneDecPer) : 0;
  }
  float getDefDecPer(DWORD num) const
  {
    num = num > dwMaxDefLockNum ? dwMaxDefLockNum : num;
    return num > dwMinDefLockNum ? ((num - dwMinDefLockNum) * fDefOneDecPer) : 0;
  }
  bool haveEffect(DWORD num) const { return num > dwMinFleeLockNum || num > dwMinDefLockNum; }
};

struct SEffectPath
{
  string strLeaveSceneEffect;
  string strLeaveSceneSound;
  string strEnterSceneEffect;
  string strEnterSceneSound;
  string strTeleportEffect;
  xLuaData oImmuneEffect;
  xLuaData oResistEffect;
};

// sociality
struct SSocialityMiscCFG
{
  DWORD dwMaxApplyCount = 0;
  DWORD dwMaxFriendCount = 0;
  DWORD dwMaxFindCount = 0;
  DWORD dwMaxNearTeamCount = 0;
  DWORD dwMaxChatWords = 0;
  DWORD dwBlackOverTime = 0;
  DWORD dwMaxBlackCount = 0;
  DWORD dwMaxChatCount = 0;

  SSocialityMiscCFG() {}
};

// produce
struct SProduceCFG
{
  DWORD dwPreAction = 0;
  DWORD dwPreActionTime = 0;

  SProduceCFG() {}
};

// laboratory
struct SLaboratoryCFG
{
  DWORD dwGarden = 0;
  DWORD dwRob = 0;

  float fGarden = 0.0f;
  float fRob = 0.0f;

  SLaboratoryCFG() {}
};

// chatroom
struct SChatRoomMiscCFG
{
  DWORD dwEnterProtectTime = 0;
  TVecDWORD vecBuffIDs;

  SChatRoomMiscCFG() {}
};

// guild
struct SGuildMiscCFG
{
  DWORD dwCreateBaseLv = 0;
  DWORD dwDismissTime = 0;
  DWORD dwChairOffline = 0;
  DWORD dwMaxApplyCount = 0;
  //DWORD dwFreezePercent = 0;
  DWORD dwTerritory = 0;
  DWORD dwPrayItem = 0;
  DWORD dwPrayRob = 0;
  DWORD dwPrayAction = 0;
  DWORD dwMaxEventCount = 0;
  DWORD dwEventOverTime = 0;
  DWORD dwQuestClearTime = 0;
  DWORD dwQuestDefaultCount = 0;
  DWORD dwQuestProtectTime = 0;
  DWORD dwMaintenanceProtectTime = 0;
  DWORD dwMaxPhotoPerMember = 0;
  DWORD dwPhotoRefreshTime = 0;
  DWORD dwPhotoMemberActiveDay = 0;
  DWORD dwMaxFramePhotoCount = 0;
  DWORD dwTreasureBroadcastNpc = 0;

  DWORD dwAssetItemID = 0;
  DWORD dwAssetGoldID = 0;

  DWORD dwRenameCoolDown = 24*3600;
  DWORD dwRenameItemId = 5527;

  DWORD dwCityGiveupCD = 0;

  DWORD dwIconCount = 32;
  DWORD dwEnterPunishTime = 0;

  DWORD dwRealtimeVoiceLimit = 0;
  DWORD dwBuildingCheckLv = 0;

  TVecItemInfo vecCreateItems;
  string strPrayEffect;
  string strDefaultPortrait;
  string strOfflineChChariManMsg;

  pair<DWORD, DWORD> pairIconNtfLv;
  map<EAuth, string> mapAuthName;

  TSetDWORD setFrameIDs;

  map<DWORD, DWORD> mapTreasureBCoin;
  map<DWORD, DWORD> mapTreasureAsset;

  SGuildMiscCFG() {}

  bool isValidFrame(DWORD dwID) const { return setFrameIDs.find(dwID) != setFrameIDs.end(); }
  const string& getAuthName(EAuth eAuth) const;
  DWORD getAssetPrice(DWORD dwTime) const;
  DWORD getBCoinPrice(DWORD dwTime) const;
};

// trade
struct SGiveBackground
{
  DWORD dwBuffid = 0;   //buffid
};
struct STradeCFG
{
  DWORD dwExpireTime = 0;       //挂单过期时间
  float fRate = 0;             //税率 
  DWORD dwMaxPendingCount = 0;
  DWORD dwAdjustPendingInterval = 0;
  DWORD dwMaxLogCount = 0;
  DWORD dwHotTime = 0;    //热门时间，秒
  float fSellCost = 0.0f;
  DWORD dwMaxBoothfee = 0;  //最大上架费用
  
  DWORD dwCycleTBegin = 0;        //分钟
  DWORD dwCycleTEnd = 0;
  DWORD dwCycleKT = 0;
  float fMaxPriceUp = 0;
  float fNoDealDropRatio = 0;
  DWORD dwUpRate = 10;
  DWORD dwDownRate = 100;
  
  float fGoodRate = 1.3f;         //点赞相性
  DWORD dwMobPrice = 0;           //莫拉比价格
  float fInflation = 0.0f;        //通货膨胀系数
  std::vector<DWORD> vecLvRate;   //附魔等级系数
  std::vector<float> vecTypeRate; //附魔等级系数
  DWORD dwLogTime = 0;
  DWORD dwPageNumber = 0;
  
  DWORD dwCantSendTime = 0;       //超过该时间无法赠送
  DWORD dwSendMoneyLimit = 0;     //赠送最低价值
  
  DWORD dwMaleDialog = 0;         
  DWORD dwFemaleDialog = 0;
  DWORD dwDialogInterval = 0;
  DWORD dwSignDialog = 0;
  DWORD dwRefuseDialog = 0;
  DWORD dwFollowTime = 0;
  std::map<DWORD/*backgroundid*/, SGiveBackground> m_giveBack;
  DWORD dwWeekRefineRate = 0;
  DWORD dwMonthRefineRate = 0;
  DWORD dwSendButtonTime = 0;    //赠送按钮持续时间

  float getTypeRate(EEnchantType type) const
  {
    if (type == EENCHANTTYPE_PRIMARY)
    {
      if (vecTypeRate.size() >= 1)
        return vecTypeRate.at(0);
      return 1;
    }
    else if (type == EENCHANTTYPE_MEDIUM)
    {
      if (vecTypeRate.size() >= 2)
        return vecTypeRate.at(1);
      return 1;
    }
    else if (type == EENCHANTTYPE_SENIOR)
    {
      if (vecTypeRate.size() >= 3)
        return vecTypeRate.at(2);
      return 1;
    }
    return 1;
  }
 
  const SGiveBackground* getGiveBackGround(DWORD bgid) const 
  {
    auto it = m_giveBack.find(bgid);
    if (it == m_giveBack.end())
      return nullptr;
    return &(it->second);
  }  

  STradeCFG() {}
};

// 摆摊配置
struct SBoothCFG
{
  DWORD dwBasePendingCount = 0;   // 基础上架数
  DWORD dwQuotaExchangeRate = 0;  // zeny和信用额度比值
  DWORD dwNameLengthMax = 0;      // 摊位名字最大长度
  DWORD dwBuffId = 0;             // 不可移动buff
  DWORD dwQuotaCostMax = 0;       // 单件扣除额度上限
  DWORD dwPendingCountMax = 0;    // 上架数上限
  DWORD dwScoreExchangeRate = 0;  // 税费和积分比值
  DWORD dwMaxSizeOneScene = 0;    // 单个场景中其他线同步最大摊位数量
  DWORD dwMaxSizeNine = 0;        // 九屏其他线同步最大摊位数量
  DWORD dwUpdateCD = 0;           // 更新操作cd
  DWORD dwSkillId = 0;            // 摆摊技能id

  DWORD getMaxSizeNine() const { return dwMaxSizeNine ? dwMaxSizeNine : 50; }
  DWORD getMaxSizeOneScene() const { return dwMaxSizeOneScene ? dwMaxSizeOneScene : 100; }

  TVecDWORD vecMaps;              // 可摆摊地图id
  std::map<DWORD, DWORD> mapSign2Score; // 招牌对应积分

  bool checkMap(DWORD id) const { return vecMaps.end() != std::find(vecMaps.begin(), vecMaps.end(), id); }
  DWORD getSign(DWORD score) const
  {
    DWORD sign = 0;
    for(auto& m : mapSign2Score)
    {
      if(score >= m.second)
        sign = m.first;
    }
    return sign;
  }

  DWORD getScoreByRate(QWORD score) const
  {
    if(!dwScoreExchangeRate)
      return 0;
    return score/dwScoreExchangeRate;
  }

};

//斗技场配置
struct SPvpCFG
{
  string strName;
  DWORD dwTeamLimit = 0;
  DWORD dwPeopleLImit = 0;
  std::map<DWORD/*raidid*/, DWORD/*count*/> mapRaid;
  DWORD raidId = 0;
  DWORD dwDuration = 0;
  DWORD dwMaxScore = 0;
};

// 斗技场通用配置
struct SPvpCommonCFG
{
  // pvp coin
  DWORD dwKillCoin = 0; // 击杀获得斗币
  DWORD dwHelpKillCoin = 0; // 助攻获得斗币
  DWORD dwDesertWinCoin = 0; // 沙漠之狼胜利奖励斗币
  DWORD dwGlamWinCoin = 0; // 华丽金属胜利奖励斗币
  DWORD dwDayMaxCoin = 0; // 每日获得最大斗币数
  DWORD dwWeekMaxCoin = 0; // 每周获得最大斗币数

  // hp
  DWORD dwHpRate = 0; // 血量倍率
  DWORD dwDesertWinScore; // 沙漠之狼一方离开 另一方积分胜利判断
  DWORD dwExtraRewardCoinNum = 0;
  DWORD dwHealRate = 0;
  pair<DWORD, DWORD> paExtraItem2Num;
};

// trap npc
struct STrapNpcCFG
{
  TVecDWORD vecTrapNpcIDs;
  STrapNpcCFG() {}

  bool isTrapNpc(DWORD dwID) const;
};

// dojo
struct SGuildDojoCFG
{
  DWORD dwCountDownTick = 0;
  map<DWORD, DWORD> mapBaseLvReq;

  DWORD getBaseLvReq(DWORD dwGroupID) const;
};

// expression
struct SExpressionCFG
{
  DWORD dwBlinkNeedSkill = 0;

  SExpressionCFG() {}
};

// attr
struct SAttrMiscCFG
{
  vector<EAttrType> vecShowAttrs;
};

// delay
struct SDelayMiscCFG
{
  DWORD dwWantedQuest = 0;
  DWORD dwExchangeCard = 0;
  DWORD dwLottery = 0;
  DWORD dwGuildTreasure = 0;
};

// monster
struct SMonsterMiscCFG
{
  DWORD dwMonsterDisappearTime = 0;
  DWORD dwMiniDisappearTime = 0;
  DWORD dwMvpDisappearTime = 0;
  DWORD dwNpcDisappearTime = 0;

  DWORD getDisappearTime(DWORD dwNpcType) const;
};

struct SHandShowData
{
  DWORD dwOdds = 0;
  TVecDWORD vecValue;
};
// hand npc
struct SHandNpcCFG
{
  TVecDWORD vecBody;
  TVecDWORD vecHead;
  TVecDWORD vecHair;
  TVecDWORD vecHairColor;
  TVecDWORD vecEye;

  SHandShowData stBirthEmoji;
  SHandShowData stBirthDialog;
  SHandShowData stDispEmoji;
  SHandShowData stDispDialog;

  SHandShowData stNormalEmoji;
  SHandShowData stNormalDialog;
  SHandShowData stAttackEmoji;
  SHandShowData stAttackDialog;

  DWORD dwEmojiInterval = 0;
  DWORD dwDialogInterval = 0;
  DWORD dwContinueTime = 0;

  DWORD getRandomOne(const SHandShowData& stData) const;
  void clear(){
    vecBody.clear();
    vecHead.clear();
    vecHair.clear();
    vecHairColor.clear();
    vecEye.clear();
  }
};

//itemimage
struct SItemImageCFG
{
  DWORD dwNpcId;
  DWORD dwRange;
  DWORD dwLoveNpcId;
};

// treasure
struct STreasureMiscCFG
{
  DWORD dwMaxGoldTree = 0;
  DWORD dwGoldTreeRefreshTime = 0;
  DWORD dwMaxMagicTree = 0;
  DWORD dwMagicTreeRefreshTime = 0;
  DWORD dwMaxHighTree = 0;
  DWORD dwHighTreeRefreshTime = 0;

  DWORD dwShakeActionNpc = 0;
  DWORD dwShakeActionMonster = 0;
  DWORD dwDisTime = 0;
  DWORD dwKnownBuffID = 0;
};


struct SCameraMonster
{
  DWORD dwOdds = 1;
  DWORD dwDistance = 0;

  DWORD dwID = 0;
  DWORD dwDayMaxCnt = 0;
  xLuaData oParams;
};

struct SCameraCFG
{
  vector<SCameraMonster> vecMonster;
  DWORD dwInterval = 0;
};

// zone
struct SZoneItem
{
  pair<DWORD, DWORD> status;
  TVecItemInfo vecCost;

  EZoneStatus eStatus = EZONESTATUS_MIN;
};
typedef vector<SZoneItem> TVecZoneItem;
struct SZoneMiscCFG
{
  DWORD dwGuildZoneTime = 0;
  DWORD dwZoneStateRate = 0;
  DWORD dwActionQuest = 0;
  DWORD dwMaxRecent = 0;
  DWORD dwJumpBaseLv = 0;

  TVecZoneItem vecItems;
  TVecItemInfo vecGuildZoneCost;

  EZoneStatus getZoneStatus(DWORD dwRate) const;
  EZoneState getZoneState(DWORD dwRate) const;
  const TVecItemInfo& getJumpCost(EZoneStatus eStatus) const;
};

//问答活动配置
struct SQACFG
{
  DWORD rewardCount = 0;
  std::map<DWORD/*level*/, DWORD/*rewardid*/> mapReward;
  DWORD getRewardByLevel(DWORD level) const 
  {
    for (auto it = mapReward.begin(); it != mapReward.end(); ++it)
    {
      if (level <= it->first)
        return it->second;
    }
    return 0;
  }
};

enum EHelpType
{
  EHELPTYPE_MIN = 0,
  EHELPTYPE_SEAL = 1,
  EHELPTYPE_DOJO = 2,
  //EHELPTYPE_TOWER = 3,
  EHELPTYPE_LABORATORY = 4,
  EHELPTYPE_WANTEDQUEST = 5,
  EHELPTYPE_QUESTREWARD = 6,
  EHELPTYPE_TOWER_MINI = 7,
  EHELPTYPE_TOWER_MVP = 8,
  EHELPTYPE_GUILDRAID = 9,
  EHELPTYPE_PVECARD = 10,
  EHELPTYPE_MAX = 11,
};

//友情之证配置
struct SFriendShipReward
{
  DWORD dwItemId = 0;
  DWORD dwMaxLimitCount = 0;
  DWORD dwSealRewardCount = 0;
  DWORD dwDojoRewardCount = 0;
  //DWORD dwTowerRewardCount = 0;
  DWORD dwTowerMiniRewardCount = 0;
  DWORD dwTowerMvpRewardCount = 0;
  DWORD dwLaboratoryRewardCount = 0;
  DWORD dwWantedQuestRewardCount = 0;
  DWORD dwQuestRewardCount = 0;
  DWORD dwGuildRaidCount = 0;
  DWORD dwPveCardCount = 0;

  DWORD getCountByType(EHelpType eType) const
  {
    switch(eType)
    {
      case EHELPTYPE_SEAL:
        return dwSealRewardCount;
        break;
      case EHELPTYPE_DOJO:
        return dwDojoRewardCount;
        break;
      case EHELPTYPE_TOWER_MINI:
        return dwTowerMiniRewardCount;
        break;
      case EHELPTYPE_TOWER_MVP:
        return dwTowerMvpRewardCount;
        break;
      case EHELPTYPE_LABORATORY:
        return dwLaboratoryRewardCount;
        break;
      case EHELPTYPE_WANTEDQUEST:
        return dwWantedQuestRewardCount;
        break;
      case EHELPTYPE_QUESTREWARD:
        return dwQuestRewardCount;
        break;
      case EHELPTYPE_GUILDRAID:
          return dwGuildRaidCount;
        break;
      case EHELPTYPE_PVECARD:
        return dwPveCardCount;
      default:
        return 0;
    }
    return 0;
  }
};

struct SFriendShipCFG
{
  const SFriendShipReward* getReward(EVarType type) const
  {
    std::string key;
    switch (type)
    {
    case EVARTYPE_FRIENDSHIP_FRIEND:
      key = "Friend";
      break;
    case EVARTYPE_FRIENDSHIP_GUILD:
      key = "Guild";
      break;
    default:
      break;
    }

    auto it = mapReward.find(key);
    if (it == mapReward.end())
      return nullptr;
    return &(it->second);
  }
  std::map<std::string, SFriendShipReward> mapReward;
};

// item
struct SSkillSlot
{
  DWORD dwSkillID = 0;
  DWORD dwAttrValue = 0;
  DWORD dwSlot = 0;

  string attr;
};
typedef vector<SSkillSlot> TVecSkillSlot;
enum EPickupMode
{
  EPICKUPMODE_CLIENT = 0,
  EPICKUPMODE_SERVER = 1
};
struct SItemMiscCFG
{
  DWORD dwMaxSellRefineLv = 0;

  DWORD dwMainMaxSlot = 0;
  DWORD dwTempMainMaxSlot = 0;
  DWORD dwTempMainOvertime = 0;
  DWORD dwTempPackOverTimeCount = 0;
  DWORD dwPersonalStoreMaxSlot = 0;
  DWORD dwStoreMaxSlot = 0;
  DWORD dwBarrowMaxSlot = 0;
  DWORD dwQuestMaxSlot = 0;
  DWORD dwFoodMaxSlot = 0;
  DWORD dwPetMaxSlot = 0;
  DWORD dwStoreBaseLvReq = 0;
  DWORD dwStoreTakeBaseLvReq = 0;

  DWORD dwKapulaMapItem = 0;
  DWORD dwKapulaStoreItem = 0;

  DWORD dwUpgradeRefineLvDec = 0;

  DWORD dwMaxMaterialRefineLv = 0;

  map<EPackType, TVecSkillSlot> mapPackSkillSlot;
  TSetDWORD setPackSkill;

  EPickupMode ePickupMode = EPICKUPMODE_CLIENT;

  DWORD dwRestoreStrengthCost = 0;
  map<EQualityType, DWORD> mapRestoreCardCost;
  map<EEnchantType, DWORD> mapRestoreEnchantCost;
  map<DWORD, DWORD> mapRestoreUpgradeCost;

  map<EEquipType, set<EEquipPos>> mapEquipType2Pos;
  DWORD dwWeponcatItem = 0;

  TSetDWORD setExtraHint;

  map<DWORD, DWORD> mapDecomposePrice;

  map<EEnchantType, map<EItemType, TVecItemInfo>> mapSpecEnchantCost; // 抚摸特殊消耗, 若有配置, 优先此配置, 不使用enchantequip表itemcost
  TSetDWORD setMachineMount;
  TSetDWORD setNoColorMachineMount;

  DWORD getPackMaxSlot(EPackType eType) const;
  const TVecSkillSlot& getPackSkillSlot(EPackType eType) const;
  bool isPackSkill(DWORD dwSkillID) const { return setPackSkill.find(dwSkillID) != setPackSkill.end(); }
  bool isExtraHint(DWORD dwItemID) const { return setExtraHint.find(dwItemID) != setExtraHint.end(); }
  bool isMachineMount(DWORD mountid) const { return setMachineMount.find(mountid) != setMachineMount.end(); }
  bool isNoColorMachineMount(DWORD mountid) const { return setNoColorMachineMount.find(mountid) != setNoColorMachineMount.end(); }

  DWORD getCardRestoreCost(EQualityType eQuality) const;
  DWORD getEnchantRestoreCost(EEnchantType eType) const;
  DWORD getUpgradeRestoreCost(DWORD dwLv) const;
  DWORD getDecomposePrice(DWORD dwID) const;

  EEquipPos getValidEquipPos(EEquipPos ePos, EEquipType eType) const;

  bool isSpecEnchantItem(EEnchantType eEnchantType, EItemType eType) const;
  const TVecItemInfo& getSpecEnchantItem(EEnchantType eEnchantType, EItemType eType) const;
};


struct SDepositTypeCFG
{
  EDepositCardType type ;
  DWORD duration = 0;
  std::string name;
  TVecDWORD vecFuns;
  DWORD expandpackage = 0;
};

enum EMvpScoreType
{
  EMVPSCORETYPE_MIN = 0,
  EMVPSCORETYPE_DAMAGE = 1,
  EMVPSCORETYPE_BELOCK = 2,
  EMVPSCORETYPE_HEAL = 3,
  EMVPSCORETYPE_RELIVE = 4,
  EMVPSCORETYPE_DEADHIT = 5,
  EMVPSCORETYPE_FIRSTDAM = 6,
  EMVPSCORETYPE_BREAKSKILL = 7,
  EMVPSCORETYPE_TOPDAMAGE = 8,
  EMVPSCORETYPE_MAX = 9,
};

struct SMvpScoreCFG
{
  DWORD dwDeadNoticeTime = 0;
  DWORD dwValidTime = 0;

  DWORD dwFirstHitScore = 0;
  DWORD dwDeadHitTime = 0;

  TVecDWORD vecDamScore;
  TVecDWORD vecBeLockScore;
  TVecDWORD vecHealScore;
  TVecDWORD vecRebirthScore;
  TVecDWORD vecDeadHitScore;

  map<EMvpScoreType, pair<string, DWORD>> mapType2NameRank;
  vector<pair<DWORD, DWORD>> vecDamDecScore;
  vector<float> vecRewardRatio;
  float fMinifirstRatio = 0;
  float fMiniDamMaxRatio = 0;

  const string& getNameByType(EMvpScoreType eType) const{
    auto it = mapType2NameRank.find(eType);
    if (it != mapType2NameRank.end())
      return it->second.first;
    static const string emptystr;
    return emptystr;
  }
  DWORD getOrderByType(EMvpScoreType eType) const {
    auto it = mapType2NameRank.find(eType);
    if (it != mapType2NameRank.end())
      return it->second.second;
    return (DWORD)EMVPSCORETYPE_MAX;
  }
  DWORD getDamDecScore(float per) const {
    for (auto &v : vecDamDecScore)
    {
      if ((float)(v.first) > per)
        return v.second;
    }
    return 0;
  }
  void clear()
  {
    vecDamScore.clear();
    vecBeLockScore.clear();
    vecHealScore.clear();
    vecRebirthScore.clear();
    vecDeadHitScore.clear();
    mapType2NameRank.clear();
    vecDamDecScore.clear();
  }
};

typedef pair<EMvpScoreType, pair<DWORD, DWORD>> TPairType2RankScore;
struct SMvpKillInfo
{
  QWORD charid = 0;
  string name;

  DWORD m_dwHealHp = 0;
  DWORD m_dwReliveOtherTimes = 0;
  QWORD m_dwHitDamage = 0;
  QWORD m_qwBeLockTime = 0;
  QWORD m_qwHitTime = 0;
  QWORD m_qwFirstHitTime = 0;

  DWORD m_dwLastMarkTime = 0;

  DWORD m_dwScore = 0;

  vector<TPairType2RankScore> vecRankInfo;
};

struct SOperRewardMiscCfg
{
  DWORD expireTime = 0; 
};

struct SCreditCFG
{
  DWORD dwDefaultValue = 0;

  DWORD dwMaxLimitValue = 0;

  DWORD dwDecLimitValue = 0;
  DWORD dwDayDecValue = 0;

  DWORD dwMonsterValue = 0;
  DWORD dwMaxMstValue = 0;
  DWORD dwMiniValue = 0;
  DWORD dwMvpValue = 0;

  DWORD dwWQuestValue = 0;
  DWORD dwWQuestTimes= 0;
  DWORD dwSealValue = 0;
  DWORD dwSealTimes = 0;

  DWORD dwChargeRatio = 0;
  DWORD dwBuyRatio = 0;

  DWORD dwAddInterval = 0;
  DWORD dwIntervalValue = 0;

  DWORD dwFirstBadValue = 0;
  DWORD dwSecondBadValue = 0;
  DWORD dwBadInterval = 0;

  DWORD dwRepeatValue = 0;
  DWORD dwRepeatInterval = 0;

  DWORD dwTimeForbid = 0;
  int iValueForbid = 0;
  int iValueLimitPersonal = 0;
  bool bPunish = false;
  int iBlack = 0;

  DWORD dwChatWorldReqLv = 0;
  DWORD dwChatSaveCharCount = 0;
  DWORD dwChatSaveMaxCount = 0;
  DWORD dwChatCalcMaxCount = 0;

  TSetDWORD setChannels;
  TSetString setBadStrs;
  bool hasBadStrs(const string& words) const
  {
    for (auto &s : setBadStrs)
    {
      if (words.find(s) != std::string::npos)
        return true;
    }
    return false;
  }
};

struct SMoneyCatCFG
{
  DWORD dwMaxMoney = 0;

  std::map<DWORD, pair<DWORD, DWORD>> mapStep2Money;

  DWORD getRandMoney(DWORD step) const
  {
    auto m = mapStep2Money.find(step);
    if (m == mapStep2Money.end())
      return 0;

    DWORD rand = randBetween(m->second.first, m->second.second);
    return rand;
  }
};

struct SMapTransCFG
{
  float fHandRate = 0.0f;

};

struct SAuguryTypeCFG
{
  DWORD dwNpcId = 0;
  string strTableName;
};

struct SAuguryCFG
{
  DWORD dwItemId = 0;   //奖励物品和消耗物品
  DWORD dwMaxRewardCountPerDay = 0; //每天牵手最大奖励个数
  
  DWORD dwStartTime = 0;
  DWORD dwEndTime = 0;        // "2017-03-03 00:00:00"-> 
  DWORD dwHandRewardTime = 0;
  DWORD dwHandTipSysId = 0;
  DWORD dwHandTipTime = 0;      //提示间隔
  DWORD dwRewardTipSysId = 0;
  float fRange = 0.0;           //npc 范围

  bool  inTime(DWORD curSec) const;  
  std::map<DWORD, SAuguryTypeCFG>m_typeConfig;
  DWORD dwMaxRewardCount = 1;   //每天占卜奖励次数
  DWORD dwExtraItemId = 0;
};

struct SAuthorizeCFG
{
  DWORD dwResetTime = 0;      //重置密码时间
  DWORD dwInputTimes = 0;     //错误输入次数
  DWORD dwInterval = 0;       //间隔时间
  DWORD dwRefineLv = 0;       //精炼等级
};


enum EItemCheckDelType
{
  EITEMCHECKDEL_MIN = 0,
  EITEMCHECKDEL_LEVEL = 1,
};

struct SItemCheckDel
{
  EItemCheckDelType eType = EITEMCHECKDEL_MIN;
  DWORD param1 = 0;
};

struct SItemCheckDelCFG
{
  std::map<DWORD, SItemCheckDel> mapItem2DelInfo;
  bool hasItem(DWORD id) const { return mapItem2DelInfo.find(id) != mapItem2DelInfo.end(); }
  const SItemCheckDel* getItemInfo(DWORD id) const
  {
    auto it = mapItem2DelInfo.find(id);
    return it != mapItem2DelInfo.end() ? &it->second : nullptr;
  }
};

struct SSkillTypeCFG
{
  TSetDWORD setBuff;
};

struct SSkillMiscCFG
{
  pair<DWORD, DWORD> pairSkillAction;
  map<ESkillType, SSkillTypeCFG> mapSkillType2CFG;
  map<DWORD, TSetDWORD> mapTypeBranch2SkillID;

  DWORD getActionBySkill(DWORD dwID) const;
  const SSkillTypeCFG* getSkillTypeCFG(ESkillType type) const
  {
    auto it = mapSkillType2CFG.find(type);
    if (it == mapSkillType2CFG.end())
      return nullptr;
    return &it->second;
  }
  const TSetDWORD& getEnsembleSkill(DWORD typebranch) const
  {
    static TSetDWORD empty;
    auto it = mapTypeBranch2SkillID.find(typebranch);
    if (it == mapTypeBranch2SkillID.end())
      return empty;
    return it->second;
  }
};

// pet
struct SPetMiscCFG
{
  TSetDWORD setArcherPartnerID;
  TSetDWORD setMerchantPartnerID;
  TSetDWORD setCancelEffectsID;

  map<DWORD, DWORD> mapBarrowEquipNpc;

  DWORD dwUserPetTimeTick = 0;

  DWORD dwUserPetHapplyGift = 1;
  DWORD dwUserPetHapplyFriend = 1;
  DWORD dwUserPetExciteGift = 1;
  DWORD dwUserPetExciteFriend = 1;
  DWORD dwUserPetHappinessGift = 1;
  DWORD dwUserPetHappinessFriend = 1;

  DWORD dwUserPetTouchTime = 0;
  DWORD dwUserPetTouchFriend = 0;
  DWORD dwUserPetTouchPerDay = 0;
  DWORD dwUserPetTouchTick = 0;
  DWORD dwUserPetTouchEffectID = 0;

  DWORD dwUserPetFeedTime = 0;
  DWORD dwUserPetFeedFriend = 0;
  DWORD dwUserPetFeedPerDay = 0;
  DWORD dwUserPetFeedTick = 0;
  DWORD dwUserPetFeedEffectID = 0;

  DWORD dwUserPetGiftTime = 0;
  DWORD dwUserPetGiftPerDay = 0;
  DWORD dwUserPetGiftFriend = 0;
  DWORD dwUserPetGiftMaxValue = 0;
  DWORD dwUserPetGiftReqValue = 0;
  DWORD dwUserPetGiftMonthCard = 0;
  DWORD dwUserPetGiftMonthCardItem = 0;
  DWORD dwHandReqFriendLv = 0;

  DWORD dwTransformEggBuff = 0;
  DWORD dwMaxCatchCartoonTime = 0;
  DWORD dwMaxCatchTime = 0;
  DWORD dwMaxCatchDistance = 0;
  DWORD dwOfflineKeepCatchTime = 0;
  DWORD dwNoOperationNoticeTime = 0;
  DWORD dwCatchSkill = 0;
  DWORD dwCatchSkillPlayTime = 0;
  DWORD dwTransformTime = 0;
  DWORD dwPetAdventureFriendReq = 0;
  DWORD dwPetAdventureMaxCount = 0;
  DWORD dwEatFoodSkill = 0;
  DWORD dwOverUserLv = 0;
  DWORD dwBirthRange = 0;
  DWORD dwCatchLineID = 0;
  DWORD dwPetReliveTime = 0;
  DWORD dwHatchFadeInTime = 0;
  DWORD dwHatchFadeOutTime = 0;

  DWORD dwWorkManualFriendLv = 0;
  DWORD dwWorkManualBaseLv = 0;
  DWORD dwWorkMaxContinueDay = 0;
  DWORD dwWorkMaxExchange = 0;
  DWORD dwWorkMaxWorkCount = 0;
  map<DWORD, DWORD> mapSkillWork;

  TSetDWORD setDefaultAdventureArea;
  map<DWORD, map<DWORD, DWORD>> mapID2SkillReset;
  map<DWORD, DWORD> mapOldEquip2NewUseItem;

  bool isValidPartner(EProfession eProfession, DWORD dwNpcID) const;
  DWORD getNpcBarrow(DWORD dwEquipID) const;

  DWORD randSkill(DWORD dwItemID) const;
  bool canRandSkill(DWORD dwItemID) const { return mapID2SkillReset.find(dwItemID) != mapID2SkillReset.end(); }
  bool cancelEffects(DWORD NpcID) const { return setCancelEffectsID.find(NpcID) != setCancelEffectsID.end(); }
};

// 雇佣猫
struct SWeaponPetMiscCFG
{
  DWORD dwHelpTeamerDis = 0;
  DWORD dwOffOwnerToBattleTime = 0;
  DWORD dwOffTeamerToBattleTime = 0;
};

// arena
struct SArenaMiscCFG
{
  DWORD dwBeKilledScore = 0;
  DWORD dwKillScore = 0;
  DWORD dwHelpScore = 0;
  DWORD dwComboScore = 0;
  DWORD dwBraverScore = 0;
  DWORD dwSaviorScore = 0;
  DWORD dwHelpTime = 0;
  DWORD dwComboTime = 0;
  DWORD dwBraverHP = 0;
  DWORD dwSaviorHP = 0;
  DWORD dwOriginScore = 0;

  DWORD dwMetalNpcID = 0;
};

// guild random raid
struct SGuildRaid
{
  DWORD dwNpcID = 0;
  DWORD dwGuildLevel = 0; // 解锁需要公会等级
  vector<pair<DWORD, DWORD>> vecUnlockItem; // 解锁道具
  vector<pair<DWORD, DWORD>> vecOpenItem; // 开启道具
  TSetDWORD setLevel; // 可选副本等级
  bool bSpecial = false; // 是否是特殊门
  DWORD dwUserLevel = 0; // 开启需要玩家等级

  SGuildRaid() {}
};

struct SGuildRaidCFG
{
  DWORD dwBossMapNum = 0;
  pair<DWORD, DWORD> pairMapDepth;
  pair<DWORD, DWORD> pairMapNumRange;
  map<DWORD, SGuildRaid> mapNpcID2GuildRaid;
  map<DWORD, pair<DWORD, DWORD>> mapGroup2NpcAndLv; //动态生成, 公会副本组对应的NpcID和等级
  DWORD unsteady_time = 0;
  float fRewardRatio = 0;

  DWORD dwDeadBossSummonTime = 0; // 亡者boss次数
  DWORD dwDeadBossUID = 0; // 亡者boss uniqueid
  DWORD dwDeadBossNum = 0; // 每次召唤数量
  DWORD dwDeadBossRaidLv = 0; // 召唤所需副本等级

  void clear() { mapNpcID2GuildRaid.clear(); mapGroup2NpcAndLv.clear(); }

  DWORD getMapGroupSize() const { return mapGroup2NpcAndLv.size(); }
  DWORD getGroupByNpc(DWORD npcid, DWORD lv = 0) const
  {
    for (auto &m : mapGroup2NpcAndLv)
    {
      if (npcid != m.second.first)
        continue;
      if (lv != 0 && lv != m.second.second)
        continue;
      return m.first;
    }
    return 0;
  }
  DWORD getLvByGroup(DWORD groupindex) const
  {
    if (groupindex > ONE_THOUSAND)
      groupindex /= ONE_THOUSAND;
    auto it = mapGroup2NpcAndLv.find(groupindex);
    return it != mapGroup2NpcAndLv.end() ? it->second.second : 0;
  }
  DWORD getNpcIDByGroup(DWORD groupindex) const
  {
    if (groupindex > ONE_THOUSAND)
      groupindex /= ONE_THOUSAND;
    auto it = mapGroup2NpcAndLv.find(groupindex);
    return it != mapGroup2NpcAndLv.end() ? it->second.first : 0;
  }
};

//extrareward
enum ETaskExtraRewardType
{
  ETASKEXTRAREWARDTYPE_MIN = 0,
  ETASKEXTRAREWARDTYPE_WANTED = 1,        //委托任务
  ETASKEXTRAREWARDTYPE_GUILD_TASK= 2,     //公会任务
  ETASKEXTRAREWARDTYPE_SEAL = 3,          //裂隙
  ETASKEXTRAREWARDTYPE_RESIST = 4,        //抗击魔潮
  ETASKEXTRAREWARDTYPE_INSTITUTE = 5,     //研究所
  ETASKEXTRAREWARDTYPE_ENDLESS = 6,       //无限塔
  ETASKEXTRAREWARDTYPE_MAX,
};

struct SReward
{
  DWORD dwFinishTimes = 0;
  DWORD dwItemID = 0;
  DWORD dwItemNum = 0;
};

struct SExtraRewardCFG
{
  ETaskExtraRewardType etype;
  SReward normalReward;
  DWORD dwLevel = 0;
  SReward addReward;
};
// buff status
struct SBuffStatusData
{
  DWORD dwStatus = 0;
  DWORD dwPeriod = 0;
  DWORD dwMaxTime = 0;
};

struct SBuffStatusCFG
{
  map<DWORD, SBuffStatusData> mapBuffStatusData;
};

// 星盘
struct SAstrolabeInfo
{
  /* DWORD dwInitId = 0;  // 激活星盘后默认激活的星位 */
};

struct SAstrolabeMiscCFG
{
  map<DWORD, SAstrolabeInfo> mapAstrolabeInfo;
  TVecItemInfo vecResetCost;
  float fResetRate = 1;
  map<DWORD, DWORD> mapResetCostLimit;
  map<EAstrolabeType, pair<DWORD, DWORD>> mapInitStar; // 初始星位
  map<DWORD, DWORD> mapDefaultTypeBranch; // 1转职业默认对应的系别分支
  DWORD dwMenuId = 0;

  bool isInitStar(EAstrolabeType type, DWORD id, DWORD starid) const
  {
    auto it = mapInitStar.find(type);
    if (it == mapInitStar.end())
      return false;
    return it->second.first == id && it->second.second == starid;
  }
  DWORD getResetCostLimitById(DWORD id) const
  {
    auto it = mapResetCostLimit.find(id);
    if (it == mapResetCostLimit.end())
      return DWORD_MAX;
    return it->second;
  }

  DWORD getDefaultTypeBranch(DWORD type) const
  {
    auto it = mapDefaultTypeBranch.find(type);
    if (it == mapDefaultTypeBranch.end())
      return 0;
    return it->second;
  }
};

enum ECelebrationLevel
{
  ECELEBRATIONLEVEL_MIN = 0,
  ECELEBRATIONLEVEL_ONE = 1,
  ECELEBRATIONLEVEL_TWO = 2,
  ECELEBRATIONLEVEL_THREE = 3,
  ECELEBRATIONLEVEL_MAX = 4,
};

struct SCelebrationCFG
{
  DWORD dwID = 0;   //活动ID
  DWORD dwAuguryReward = 0;   //占卜额外奖励
  DWORD dwGuildTaskTimes = 0;     //公会任务奖励倍数
  DWORD dwGuildDonateTimes = 0;   //公会捐赠奖励倍数
  DWORD dwGuildPhaseTimes = 0;    //公会副本奖励倍数
};

struct SCelebrationMCardCFG
{
  DWORD dwCostCard = 0;
  DWORD dwRewardID = 0;
  DWORD dwCount = 0;
};

struct SMonthCardActivityCFG
{
  DWORD dwID = 0;   //活动ID

  std::map<ECelebrationLevel,SCelebrationMCardCFG> m_mapCelebrationMCardCfg;
};

struct SFunctionSwitchCFG
{
  DWORD dwFreeFreyja = 0;
  DWORD dwFreePackage = 0;
  DWORD dwFreeFreyjaTeam = 0;
};

// achieve
struct SAchieveMiscCFG
{
  DWORD dwHandTimeLimit = 0;
  DWORD dwMvpTimeLimit = 0;
  DWORD dwDeadTimeLimit = 0;
  DWORD dwItemUseTimeLimit = 0;
  
  TSetDWORD setCollectExclude;

  bool isExclude(DWORD dwID) const { return setCollectExclude.find(dwID) != setCollectExclude.end(); }
};

// chatchannel
struct SChatChannelCFG
{
  DWORD dwRound = 0;
  DWORD dwTeam = 0;
  DWORD dwGuild = 0;
  DWORD dwFriend = 0;
  DWORD dwWorld = 0;
  DWORD dwMap = 0;
  DWORD dwSys = 0;
  DWORD dwRoom = 0;
  DWORD dwBarrage = 0;
  DWORD dwChat = 0;
};

struct SUserPhotoCFG
{
  DWORD dwDefaultSize = 0;
  vector<pair<DWORD, DWORD>> vecSkillIncreaseSize;

  bool isPhotoSkill(DWORD id) const
  {
    auto v = find_if(vecSkillIncreaseSize.begin(), vecSkillIncreaseSize.end(), [id](const pair<DWORD, DWORD>& v) -> bool {
        return v.first == id;
      });
    return v != vecSkillIncreaseSize.end();
  }
  DWORD getSizeBySkillID(DWORD id) const
  {
    auto v = find_if(vecSkillIncreaseSize.begin(), vecSkillIncreaseSize.end(), [id](const pair<DWORD, DWORD>& v) -> bool {
        return v.first == id;
      });
    if (v == vecSkillIncreaseSize.end())
      return 0;
    return v->second;
  }
};

struct SFoodMiscCFG
{
  //server
  DWORD dwSkillId = 0;
  DWORD dwCookerKnife = 0;
  DWORD dwCookerHat = 0;
  DWORD dwDarkFood = 0;
  float fSatietyRate = 1.0;   //饱腹度消耗速率   饱腹度/秒
  set<EItemType> setPackTypes;

  //misc
  DWORD dwCookMeterialExpPerLv = 0;
  DWORD dwMaxCookMeterialLv = 0;
  DWORD dwCookFoodExpPerLv = 0;
  DWORD dwMaxCookFoodLv = 0;
  DWORD dwTaserFoodExpPerLv = 0;
  DWORD dwMaxTasterFoodLv = 0;
  DWORD dwDefaultSatiey = 0;     //0 级的饱腹度
  DWORD dwDefaultLimitFood = 0;    //0 级的享用料理上限

  TVecDWORD vecEatSkill;          //吃料理的技能id
  DWORD dwMaxLastCooked = 0;      //记录的最近烹饪的料理数
  DWORD dwFoodNpcDuration = 0;    //料理npc在场景上存在的时长
  DWORD dwMaxPutFoodCount = 0;       //同时最大摆放料理个数
  DWORD dwMaxPutMatCount = 0;       //同时最大摆放野餐垫个数
  DWORD dwChristmasCake = 0;      //圣诞蛋糕

  DWORD getExpPerLv(Cmd::EFoodDataType type) const 
  {
    switch (type)
    {
    case EFOODDATATYPE_MATERIAL:
      return dwCookMeterialExpPerLv;
    case EFOODDATATYPE_FOODCOOK:
      return dwCookFoodExpPerLv;
    case EFOODDATATYPE_FOODTASTE:
      return dwTaserFoodExpPerLv;
    default:
      break;
    }
    return 0;
  }

  DWORD getMaxLv(Cmd::EFoodDataType type) const
  {
    switch (type)
    {
    case EFOODDATATYPE_MATERIAL:
      return dwMaxCookMeterialLv;
    case EFOODDATATYPE_FOODCOOK:
      return dwMaxCookFoodLv;
    case EFOODDATATYPE_FOODTASTE:
      return dwMaxTasterFoodLv;
    default:
      break;
    }
    return 0;
  }

  bool isEatSkill(DWORD dwSkillId) const
  {
    auto it = std::find(vecEatSkill.begin(), vecEatSkill.end(), dwSkillId);
    return it != vecEatSkill.end();
  }

  bool isDarkFood(DWORD itemId) const { return dwDarkFood == itemId; }
  bool isFoodItem(EItemType eType) const { return setPackTypes.find(eType) != setPackTypes.end(); }
};

struct STradeBlack
{
  bool isIn(QWORD accid) const{ return m_setAccid.find(accid) != m_setAccid.end(); }
  std::set<QWORD> m_setAccid;
};

struct SLotteryRepair
{
  DWORD dwItemID = 0;

  map<EQualityType, DWORD> mapQualityCount;

  DWORD getCount(EQualityType eQuality) const
  {
    auto m = mapQualityCount.find(eQuality);
    if (m != mapQualityCount.end())
      return m->second;
    return 0;
  }
};

struct SLotteryLinkage
{
  DWORD dwType = 0;    // 扭蛋类型
  DWORD dwYear = 0;    // 年
  DWORD dwMonth = 0;   // 月
  DWORD dwEndTime = 0; // 截止时间
};

typedef std::vector<SLotteryLinkage> TVecLotteryLinkage;

struct SLotteryMiscCFG
{
  //gameconfig
  std::map<DWORD/*type*/, std::pair<DWORD/*ticket*/, DWORD/*count*/>> m_mapTicket;
  DWORD dwSendPrice = 0;      //扭蛋盒子单价
  DWORD dwDiscountPrice = 0;  //扭蛋盒子折后单价
  DWORD dwExpireTime = 0;     //自动领取时间
  DWORD dwLoveLeterItemId = 0;//祝福卡道具id
  std::map<DWORD, SLotteryRepair> mapRepairItem;
  ItemInfo oEnchantTrans;

  //servergame
  DWORD dwPrice = 0;
  DWORD dwDiscount = 0;   //折扣 30%
  std::map<DWORD/*itemtype*/, DWORD/*batchid*/> m_mapBatch;
  std::map<DWORD/*itemtype*/, DWORD/*weight*/> m_mapCoinWeight;
  DWORD m_dwTotalCoinWeight = 0;
  std::map<DWORD/*itemtype*/, DWORD/*weight*/> m_mapTicketWeight;
  DWORD m_dwTotalTicketWeight = 0;
  DWORD m_dwDayBuyLotteryGiveCnt = 0;   //每日购买扭蛋盒数
  DWORD m_dwBasePriceRateMin = 0;       //保底价参数最小值
  DWORD m_dwBasePriceRateMax = 0;       //保底价参数最大值
  DWORD m_dwSaveRateMax = 0;            //概率小于等于该值时保存
  std::map<DWORD/*扭蛋类型*/, DWORD> m_mapDayMaxCnt;    //每天每个扭蛋机金币每个角色最大扭蛋次数

  TVecLotteryLinkage m_vecLinkage; // 联动活动

  bool checkLinkage(DWORD type, DWORD year, DWORD month) const;
  
  // 获取保底价
  DWORD getBasePrice(DWORD dwPrice) const
  {
    return dwPrice*randBetween(m_dwBasePriceRateMin, m_dwBasePriceRateMax)/100;
  }
  DWORD getTicketCount(DWORD type, DWORD itemId) const {
    auto it = m_mapTicket.find(type);
    if (it == m_mapTicket.end())
     return 0;
    else
      return it->second.first == itemId ? it->second.second : 0;
  }
  DWORD getTicketId() const {
    if (m_mapTicket.empty()) return 0; else return m_mapTicket.begin()->first;
  }
  DWORD getCurBatch(DWORD itemType) const {
    auto it = m_mapBatch.find(itemType);
    if (it == m_mapBatch.end()) return 0;
    else return it->second;
  }
  //获取概率, *10000
  DWORD getRate(DWORD itemType) const {
    const auto & it = m_mapCoinWeight.find(itemType);
    if (it == m_mapCoinWeight.end()) return 0;
    float  a = it->second / (float)m_dwTotalCoinWeight;
    return a * 10000;
  }
  DWORD randomItemType(bool isTicket) const;

  void fillLotteryMagicRate(Cmd::LotteryRateQueryCmd& cmd) const
  {
    for(auto& m : m_mapCoinWeight)
    {
      Cmd::LotteryRateInfo* pInfo = cmd.add_infos();
      if(!pInfo)
      {
        XERR << "[扭蛋-大扭蛋概率] add infos failed! type:" << m.first << "weight:" << m.second << XEND;
        return;
      }

      pInfo->set_type(m.first);
      pInfo->set_rate(m.second*LOTTERY_PER_MAX/m_dwTotalCoinWeight);
    }
  }

  DWORD getMaxLotteryCnt(DWORD t) const {
    auto it =  m_mapDayMaxCnt.find(t);
    if (it == m_mapDayMaxCnt.end()) return 0;
    else
      return it->second;
  }

  const SLotteryRepair* getRepairCFG(DWORD dwItemID) const {
    auto m = mapRepairItem.find(dwItemID);
    if (m != mapRepairItem.end())
      return &m->second;
    return nullptr;
  }
};

struct SAuctionMiscCFG
{
  //gameconfig

  DWORD dwRecordPageCnt = 10;                         //拍卖日志每页最大数
  DWORD dwFlowingWaterMaxCount = 10;                  //拍卖文本最大数
  float fRate = 0.0;                                  //拍卖税率
  DWORD dwNpcId = 0;
  QWORD qwMaxPrice = 0;                               //拍卖最高价
  //servergame
  DWORD dwVerifySignupDuration = 0;                   //拍卖报名审核时长
  DWORD dwVerifyTakeDuration = 0;                      //拍卖成功领取审核时长
  std::map<DWORD/*wday*/, struct tm> m_mapBeginTime;  //拍卖开放日,按周几从小大排序
  DWORD dwMaxAuctionItemCount = 0;                    //最大拍卖物品个数
  DWORD dwMinAuctionItemCount = 0;                    //最低拍卖物品个数
  DWORD dwDurationPerOrder = 0;                       //每个拍品无人出价时竞拍时间  
  DWORD dwNextOrderDuration = 0;                      //倒计时多少秒进入下一个拍品  
  DWORD dwReceiveTime = 0;                            //未领取日志清除时间
  DWORD dwTradePriceDiscount = 0;                     //交易所价格折扣
  DWORD dwStartDialogTime = 0;                        //拍卖开始前多少秒推送dialog
  DWORD dwPublicityDuration = 0;                      //拍卖公示期时间
  DWORD dwEnchantAttrCount = 0;                       //附魔条数
  DWORD dwEnchantAttrValuableCount = 0;               //附魔good条数
  DWORD dwEnchantBuffExtraCount = 0;                  //附魔buff条数
  std::map<DWORD, DWORD> m_mapMaskPrice;              //拍卖禁止出价档位时间
};

struct SShopInfo
{
  DWORD dwCount = 0;
  bool bDuplicate = false;
};

struct SWeekShopRand
{
  DWORD dwGroupId = 0;
  DWORD dwWeight = 0;
  TVecDWORD vecShopId;
};

typedef map<DWORD/*groupid*/, SWeekShopRand> TMapGroupId2Rand;

struct SShopCFG
{
  map<DWORD, map<DWORD, SShopInfo>> mapInfos;
  TMapGroupId2Rand mapWeekShopRand;
  TSetDWORD setWeekShopRand;        //随机的
};

struct SGameShopCFG
{
  map<DWORD, DWORD> mapLimitItem;

  DWORD getLimitNum(DWORD id) const
  {
    auto it = mapLimitItem.find(id);
    if(it != mapLimitItem.end())
      return it->second;

    return 0;
  }
};

struct STutorMiscCFG
{
  DWORD dwStudentBaseLvReq = 0;
  DWORD dwTutorBaseLvReq = 0;

  DWORD dwMaxTutorApply = 0;
  DWORD dwMaxStudent = 0;

  DWORD dwProtectTime = 0;
  DWORD dwForbidTime = 0;

  DWORD dwStudentGraduationTime = 0;
  DWORD dwApplyRefuseProtect = 0;
  DWORD dwApplyOverTime = 0;

  DWORD dwMaxProfrociency = 0;
  DWORD dwStudentGraduationReward = 0;
  DWORD dwTeacherGraduationReward = 0;
  DWORD dwTutorMenuID = 0;
  DWORD dwTeacherTaskMailID = 0;
  DWORD dwTeacherGrowMailID = 0;
  DWORD dwTeacherGraduationMailID = 0;

  float fTutorExtraBattleTimePer = 0;
  DWORD dwTutorExtraMaxBattleTime = 0;

  bool bGrowRewardPatchSwitch = false; // 成长奖励补丁开关(仅用于海外)
};

struct SPlayerRenameCFG
{
  DWORD dwRenameItemId = 5526;
  DWORD dwRenameCoolDown = 86400;
};

enum EBranchType
{
  EBRANCH_TYPE_TRUNK = 1,
  EBRANCH_TYPE_STUDIO = 2,
  EBRANCH_TYPE_TF = 4,
  EBRANCH_TYPE_RELEASE = 8,
  EBRANCH_TYPE_DEBUG = EBRANCH_TYPE_TRUNK | EBRANCH_TYPE_STUDIO,
};

struct SBranchForbidCFG
{
  map<string, map<DWORD, DWORD>> mapForbids;

  bool isForbid(const string& tablename, DWORD id) const
  {
    auto itt = mapForbids.find(tablename);
    if (itt == mapForbids.end())
      return false;
    auto it = itt->second.find(id);
    if (it == itt->second.end())
      return false;
    if ((BaseConfig::getMe().getBranch() == BRANCH_DEBUG && (it->second & EBRANCH_TYPE_DEBUG) != 0) ||
        (BaseConfig::getMe().getBranch() == BRANCH_TF && (it->second & EBRANCH_TYPE_TF) != 0) ||
        (BaseConfig::getMe().getBranch() == BRANCH_PUBLISH && (it->second & EBRANCH_TYPE_RELEASE) != 0))
    {
      XLOG << "[配置屏蔽] 分支:" << BaseConfig::getMe().getBranch() << "表:" << tablename << "id:" << id << "被屏蔽" << XEND;
      return true;
    }
    return false;
  }
};

struct SGvgRewardData
{
  DWORD dwNeedTimes = 0;
  DWORD dwItemID = 0;
  DWORD dwItemCnt = 0;
};
typedef map<DWORD, SGvgRewardData> TMapGvgTimes2RewardData;

struct SGvgTowerCFG
{
  EGvgTowerType eType;
  DWORD dwAllValue = 0;
  DWORD dwShowNpcID = 0;
  DWORD dwCrystalNum = 0;
  DWORD dwBaseValue = 0;

  vector<pair<DWORD, float>> vecGuildNum2Spd;
  vector<pair<DWORD, float>> vecUserNum2Spd;

  float fRange = 0;
  xPos pos;

  string strTowerName;
  DWORD getSpeed(DWORD guildnum, DWORD usernum) const;
};

struct SGvgCampCFG
{
  DWORD dwColor = 0;
  DWORD dwMetalUniqID = 0; // 水晶npc uniqueid
  DWORD dwGearUniqID = 0; // 空气墙npc uniqueid
  DWORD dwBpID = 0; // bp点id
  DWORD dwNpcShowActionID = 0; // 光墙动作id
  string strColorName;
};

struct SSuGvgRewardCFG
{
  DWORD dwLevel = 0;
  float fMinScore = 0;
  float fMaxScore = 0;
  string strLvName;
  map<DWORD, pair<float, float>> mapRank2RewardPer; // rank->(guild reward per, user reward per)
};

struct SSuperGvgCFG
{
  DWORD dwSuperGvgBeginTime = 0; // 公会战开战后, 决战开始还需要的时间
  DWORD dwNoticeTime = 0; // 决战开始前一段时间, 提示
  DWORD dwMetalGodTime = 0; // 华丽金属无敌时长
  DWORD dwMetalGodBuff = 0; // 华丽金属无敌Buff
  DWORD dwRaidID = 0;
  DWORD dwPartinRewardTime = 0; // 最小参战时间
  DWORD dwUserRewardMail = 0; // 玩家参战奖励邮件
  DWORD dwFirstBossTime = 0; // 开战后第一次出boss时间
  DWORD dwMinBossInterval = 0; // boss出现间隔, 小
  DWORD dwMaxBossInterval = 0; // boss出现间隔,大
  DWORD dwBuffEffectInterval = 0; // 场景效果间隔
  DWORD dwWinMetalNum = 0; // 胜利所需水晶数量
  DWORD dwComposeChipNum = 0; // 合成水晶需要的碎片数
  DWORD dwNpcDefaultActionID = 0; // 占领圈初始id
  DWORD dwMetalDieExpelTime = 0; // 华丽金属死亡, 驱逐时长
  DWORD dwExpelSkill = 0; //驱逐技能
  DWORD dwMatchToEnterTime = 0; // 从报名到可以进入的时间
  DWORD dwMatchToFireTime = 0; // 从报名到可以开始战斗的时间
  DWORD dwTowerOpenTime = 0; // 塔开启时间
  DWORD dwItemDispTime = 0; // 道具消失时间
  DWORD dwOneSecJoinUserNum = 0; // 1s最多进入的玩家数量
  DWORD dwExtraMailID = 0; // 额外奖励邮件id
  DWORD dwExtraMsgID = 0; // 额外奖励msg提示

  TSetDWORD setSuperGvgDays; // 记录周几需要开启决战
  TSetDWORD setSouthBossUids;
  TSetDWORD setNorthBossUids;
  TSetDWORD setBossIDs;
  TSetDWORD setNorthSceneEffectIDs; // 场景事件, 对应 PveCardEffect表
  TSetDWORD setSouthSceneEffectIDs; // 场景事件, 对应 PveCardEffect表

  TVecDWORD vecRankScore;
  TVecItemInfo vecBaseGuildReward;
  TVecItemInfo vecBaseUserReward;
  TVecItemInfo vecUserStableReward;
  vector<SSuGvgRewardCFG> vecRewardPerCFG;
  vector<pair<xPos,float>> vecPreLimitArea; // 开战前, 限制移动区域

  const SSuGvgRewardCFG* getRewardCFGByLv(DWORD lv) const
  {
    for (auto &v : vecRewardPerCFG)
    {
      if (v.dwLevel == lv)
        return &v;
    }
    return nullptr;
  }

  float getRewardPer(DWORD rank, DWORD level, bool userOrGuild) const;
  DWORD getLevelByScore(float aveScore) const;
  DWORD getScoreByRank(DWORD rank) const
  {
    rank -= 1;
    if (rank >= vecRankScore.size())
      return 0;
    return vecRankScore[rank];
  }

  map<EGvgTowerType, SGvgTowerCFG> mapTowerCFG;
  const SGvgTowerCFG* getTowerCFG(EGvgTowerType eType) const
  {
    auto it = mapTowerCFG.find(eType);
    return it != mapTowerCFG.end() ? &(it->second) : nullptr;
  }
  map<DWORD, SGvgCampCFG> mapCampCFG; // 营地信息
  const SGvgCampCFG* getCampCFG(DWORD color) const
  {
    auto it = mapCampCFG.find(color);
    return it != mapCampCFG.end() ? &(it->second) : nullptr;
  }

  bool isOutLimitArea(const xPos& p) const
  {
    for (auto &v : vecPreLimitArea)
    {
      if (getXZDistance(p, v.first) <= v.second)
        return false;
    }
    return true;
  }
};

struct SGuildFireCFG
{
  TSetDWORD setWeekStartTime;// 距离每周一0点时间
  DWORD dwLastTime = 0;
  DWORD dwDangerTime = 0;
  DWORD dwDangerSuccessTime = 0;
  DWORD getNextStartTime(DWORD cur) const;

  DWORD dwPartInBuffID = 0; // 用于发送参与奖励的Buff
  DWORD dwPartInMailID = 0;
  DWORD dwSpecMailID = 0;
  DWORD dwWinGuildMailID = 0;
  DWORD dwExtraMailID = 0; // 额外奖励邮件id
  DWORD dwExtraMsgID = 0; // 额外奖励msg提示
  DWORD dwExpelSkill = 0;

  TSetDWORD setMsgMap;

  vector<pair<DWORD, pair<DWORD, DWORD>>> time2ReliveCD;
  DWORD dwResetCDTime = 0;
  DWORD dwMaxCD = 0;
  DWORD dwHpRate = 0;
  DWORD dwAttExpelTime = 0;
  DWORD dwDefExpelTime = 0;

  TMapGvgTimes2RewardData mapPartinTimeReward; //参与时间奖励
  TMapGvgTimes2RewardData mapKillMonsterReward; //杀怪奖励
  TMapGvgTimes2RewardData mapReliveUserReward; //复活玩家奖励
  TMapGvgTimes2RewardData mapExpelReward; //驱逐敌方奖励
  TMapGvgTimes2RewardData mapDamMetalReward; //伤害华丽金属奖励
  TMapGvgTimes2RewardData mapKillMetalReward; //击破华丽金属奖励
  TMapGvgTimes2RewardData mapGetHonorReward; //每获得x(500)荣誉, 奖励宝箱

  DWORD dwKillUserItem = 0;//每击杀一个玩家获得奖励道具Id
  DWORD dwKillUserItemCnt = 0;//每击杀一个玩家活动奖励道具数量
  DWORD dwPartInTime = 0; //最少有效参战时间,秒
  DWORD dwMaxHonorCount = 0; // 最多荣誉获取值

  DWORD dwSafeBuffID = 0; // 安全区buffid
  DWORD dwKillUserTeamGetNum = 0; // 每击杀一个玩家, 队友获得的奖励数量

  DWORD dwMinShowNum = 0; // 人数达到此值时区分显示
  float fCriPer = 0; // 区分激烈争夺的百分比
  float fNormalPer = 0; // 区分平缓的百分比

  SSuperGvgCFG stSuperGvgCFG;

  DWORD getReliveCD(DWORD time) const
  {
    for (auto &v : time2ReliveCD)
    {
      if (v.second.first <= time && v.second.second >= time)
        return v.first;
    }
    return 0;
  }

  const TMapGvgTimes2RewardData& getRewardInfo(EGvgDataType eType) const;
  bool hasSuperGvg(DWORD starttime) const; /*判断某次开启后是否有supergvg*/
};

struct SMapBranchForbidCFG
{
  map<int, DWORD> mapNextMapID2Forbid;
  // trunk 1, studio 2, tf 4, release 8
  bool isForbid(DWORD nextmapid) const { return mapNextMapID2Forbid.find(nextmapid) != mapNextMapID2Forbid.end(); }
  bool isForbid(int nextmapid, DWORD branchid) const
  {
    auto it = mapNextMapID2Forbid.find(nextmapid);
    if (it == mapNextMapID2Forbid.end())
      return false;
    return it->second & branchid;
  }
};

struct SQuotaCFG
{
  DWORD dwItemId = 0;           //积分道具id
  DWORD dwDefaultExpireDay = 0;
  DWORD dwLogCountPerPage = 0;
  DWORD dwMaxSaveLogCount = 0;
  DWORD dwDetailCountPerPage = 0;
  DWORD dwDetailExpireDay = 0;
  DWORD dwDetailExpireDayDel = 0;
};

struct SSafeRefineCost
{
  DWORD lv = 0;
  DWORD normal_cost = 0;
  DWORD discount_cost = 0;
};

enum ESystemForbidType
{
  ESYSTEM_FORBID_MIN = 0,
  ESYSTEM_FORBID_LIMIT = 1,
  ESYSTEM_FORBID_TUTOR = 2,
  ESYSTEM_FORBID_GVG = 3,
  ESYSTEM_FORBID_PEAK = 4,
  ESYSTEM_FORBID_PRAY = 5,
  ESYSTEM_FORBID_AUCTION = 6,
  ESYSTEM_FORBID_SEND = 7,
  ESYSTEM_FORBID_PVE = 8,
  ESYSTEM_FORBID_MULTI_CAREER = 9,
  ESYSTEM_FORBID_DEAD = 10,
  ESYSTEM_FORBID_BOOTH = 11,
  ESYSTEM_FORBID_EXCHANGESHOP = 12,
  ESYSTEM_FORBID_MAX,
};

struct SAutoSkillGroupCFG
{
  vector<TVecDWORD> vecAutoSkillGroup;

  void getHigherSkill(DWORD skillid, TSetDWORD& skills) const;
};

struct SPoliFireCFG
{
  float fDropRange = 0;
  DWORD dwItemDispTime = 0;
  DWORD dwDefaultSkillID = 0;
  DWORD dwMaxSkillPos = 0;
  DWORD dwAppleItemID = 0;
  DWORD dwGhostPoliBuff = 0;
  DWORD dwRecoverAppleNum = 0;
  DWORD dwShowBuff = 0;
  map<DWORD, DWORD> mapItem2Skill;
  DWORD getItemBySkill(DWORD skillid) const
  {
    for (auto &m : mapItem2Skill)
    {
      if (m.second == skillid)
        return m.first;
    }
    return 0;
  }
  DWORD dwDefualtScore = 0;           //初始苹果个数
  DWORD dwGodBuffid = 0;              //无敌buffid
  DWORD dwGodDuration = 0;            //无敌持续时间
  DWORD dwAppleLimitCount = 0;        //每局苹果上限个数
  DWORD dwSocreItemId = 0;            //积分的道具id
  DWORD dwActivityMap = 0;            //报名地图
  DWORD dwTransportNpcID = 0;         //传送NpcID
  DWORD dwGreedyBuff = 0;             //贪婪BuffID
  DWORD dwPickAppleCDMs = 0;          //捡苹果cd,毫秒
  DWORD dwPreCloseMsgTime = 0;        //结束前提示时间
  DWORD dwDropAppleCDMs = 0;          //掉落苹果后cd

  std::map<DWORD/*lv*/, DWORD/*rewardid*/> mapLv2ReardId;
  std::set<DWORD> setMaskBuffId;

  //gameconfig
  std::map<DWORD/*index*/, DWORD/*buffid*/> mapTransBuffId; //变身卷轴buffid
  DWORD getBuffId(DWORD index) const ;
  DWORD getRewardId(DWORD dwLevel) const;
 
  const TSetDWORD& getMaskBuffId() const {
    return setMaskBuffId;
  }
};

struct SBeingMiscCFG
{
  DWORD dwBirthRange = 0;
  DWORD dwOffOwnerToBattleTime = 0;
  DWORD dwAutoSkillMax = 0;
  map<DWORD, vector<DWORD>> mapID2SkillOrder;
};

// manual misc
struct SManualMiscCFG
{
  DWORD dwHeadReturnMailID = 0;
  DWORD dwCardReturnMailID = 0;
  DWORD dwHeadUnlockReturnMailID = 0;
  DWORD dwCardUnlockReturnMailID = 0;
  DWORD dwLevelReturnMailID = 0;
  DWORD dwQualityReturnMailID = 0;
  DWORD dwSkillReturnMailID = 0;
  DWORD dwUnsolvedPhotoOvertime = 0;

  map<DWORD, DWORD> mapLevelReturn;
  map<EQualityType, map<DWORD, ItemInfo>> mapQualityReturn;
  map<EQualityType, string> mapQualityName;

  DWORD getLevelReturn(DWORD dwLv) const
  {
    for (auto m = mapLevelReturn.rbegin(); m != mapLevelReturn.rend(); ++m)
    {
      if (dwLv >= m->first)
        return m->second;
    }
    return 0;
  }

  const ItemInfo* getQualityReturn(EQualityType eQuality, DWORD dwNum) const
  {
    auto m = mapQualityReturn.find(eQuality);
    if (m == mapQualityReturn.end())
      return nullptr;
    for (auto item = m->second.rbegin(); item != m->second.rend(); ++item)
    {
      if (dwNum >= item->first)
        return &item->second;
    }
    return nullptr;
  }
  const string& getQualityName(EQualityType eQuality) const
  {
    auto m = mapQualityName.find(eQuality);
    if (m != mapQualityName.end())
      return m->second;
    return STRING_EMPTY;
  }
};

struct SLoveLetterCFG
{
  DWORD dwConstellation = 0;  //星座絮语
  DWORD dwChristmas = 0;      //圣诞情书
  DWORD dwChristmasReward = 0;//圣诞奖励
  DWORD dwSpring = 0;         //春节情书
  DWORD dwSpringReward = 0;   //春节奖励
};

struct SPetAdventureEffCFG
{
  TVecDWORD vecRefineScore;
  map<EAttrType, pair<DWORD, DWORD>> mapType2MaxScoreAndBaseAttr; // key : attr, value :<p1,p2>, myattr/baseattr * maxscore
  TVecDWORD vecSuperEnchantScore;

  map<DWORD, DWORD> mapItemID2Score; // 称号道具id -> 评分
  map<EQualityType, DWORD> mapHeadWearQua2Score;
  map<EQualityType, DWORD> mapCardQua2Score;

  DWORD dwRefineMax = 0;
  DWORD dwEnchantMax = 0;
  DWORD dwStarMax = 0;
  DWORD dwTitleMax = 0;
  DWORD dwHeadWearMax = 0;
  DWORD dwCardMax = 0;
  DWORD dwEnchantCalcPer = 0;
};

//功能屏蔽
struct SFuncForbidCFG
{
#define FUNC_FORBID_HIGHREINE "highrefine"        //极限精炼

  std::set<string> m_setForbid;
  bool isForbid(const std::string& name) const {
    if (name.empty() || name == "") return false;
    bool ret = m_setForbid.find(name) != m_setForbid.end();
    if (ret)
      XLOG << "功能已被屏蔽" << name << XEND;
    return ret;
  }
};

struct SGuildBuildingMiscCFG
{
  DWORD dwMaxSubmitCount = 0;
  pair<DWORD, DWORD> pairOpenCost;
  DWORD dwOpenGuildLevel = 0;
  DWORD dwGateNpcID = 0;
  DWORD dwBuildingLvUpNpcID = 0; // 建筑升级在聊天窗口说话的npcid
  DWORD dwBuildingLvUpMsgID = 0;
  DWORD dwBuildingBuildMsgID = 0;
  DWORD dwBuildingLvUpFinishMsgID = 0;
  DWORD dwBuildingBuildFinishMsgID = 0;
};

struct SGuildWelfareMiscCFG
{
  DWORD dwOverdueTime = 0;
};

struct SGuildChallengeMiscCFG
{
  DWORD dwExtraReward = 0;
};

struct SJoyLimitCFG
{
  DWORD dwGuess = 0;
  DWORD dwMischief = 0;
  DWORD dwQuestion = 0;
  DWORD dwFood = 0;
  DWORD dwYoyo = 0;
  DWORD dwATF = 0;
  DWORD dwAugury = 0;

  std::map<DWORD, DWORD> mapReward;
  std::set<DWORD> setFoodID;
  DWORD dwFoodAdd = 0;
  DWORD dwAuguryAdd = 0;
  DWORD dwSeatAdd = 0;

  DWORD dwMusicID = 0;
  DWORD dwMusicNpcID = 0;
  DWORD dwShopType = 0;
  DWORD dwShopID = 0;
};

struct SBossMiscCFG
{
  DWORD dwRefreshBaseTimes = 0;
  DWORD dwDeadBossOpenNtf = 0;
  DWORD dwDeadSetRate = 0;
  TSetDWORD setRefreshTimes;
};

struct SArtifactBuildingMiscCFG
{
  DWORD dwProduceNpcID = 0;
  DWORD dwProduceMsgID = 0;
};

struct SArtifactMiscCFG
{
  DWORD dwRetrieveCD = 0;
  map<EGuildBuilding, SArtifactBuildingMiscCFG> mapBuilding2CFG;

  const SArtifactBuildingMiscCFG* getBuildingCFG(EGuildBuilding type) const
  {
    auto it = mapBuilding2CFG.find(type);
    if (it == mapBuilding2CFG.end())
      return nullptr;
    return &it->second;
  }
};

struct SDepositMiscCFG
{
  bool getDiscountPair(DWORD id, DWORD& fromId, DWORD& toId) const
  {
    for (auto&v : m_vecDiscountVersionCard)
    {
      if (v.first == id)
      {
        fromId = v.first;
        toId = v.second;
        return true;
      }
      if (v.second == id)
      {
        fromId = v.first;
        toId = v.second;
        return true;
      }
    }    
    return false;
  }
  std::vector<std::pair<DWORD/*from id*/, DWORD/*to id*/>> m_vecDiscountVersionCard;   //版本折扣
};

// 周年庆-好友回归
struct SRecallCFG
{
  DWORD dwNeedOffline = 0;
  DWORD dwNeedBaseLv = 0;
  DWORD dwRecallMailID = 0;
  DWORD dwRecallBuffID = 0;

  DWORD dwMaxRecallCount = 0;
  DWORD dwFirstShareReward = 0;
  DWORD dwContractMailID = 0;
  DWORD dwContractTime = 0;

  DWORD dwRewardBuffID = 0;
  vector<pair<DWORD, DWORD>> vecRewardBuff2Layer;
};

struct SPeakItem
{
  DWORD dwItem;
  DWORD dwNum;
  DWORD dwAddLv;
};

struct SPeakLevelCFG
{
  DWORD m_dwSkillBreakPoint = 0;
  std::vector<SPeakItem> m_vecPeakItem;
  map<DWORD, DWORD> m_mapEvo2PreSkillPoint; // 每一个转职的技能升级需要前面的职业消耗多少技能点, 比如:3转技能需要1/2/2进阶总共消耗150点技能点, 才能升级

  DWORD getNeedPreSkillPoint(EProfession pro) const
  {
    return getNeedPreSkillPointByEvo(pro == EPROFESSION_NOVICE ? 0 : DWORD(pro) - DWORD(pro) / 10 * 10);
  }
  DWORD getNeedPreSkillPointByEvo(DWORD evo) const
  {
    auto it = m_mapEvo2PreSkillPoint.find(evo);
    if (it == m_mapEvo2PreSkillPoint.end())
      return 0;
    return it->second;
  }
};

struct SWeddingMiscCFG
{
  //gameconfig
  DWORD dwEngageRefresh = 0;          //婚期刷新时间，距离当天凌晨读秒
  DWORD dwInviteMaxCount = 0;         //邀请函发送人数上限
  DWORD dwEngageInviteOverTime = 0;   //邀请超时时间，秒
  DWORD dwMaxFramePhotoCount = 0;     // 结婚相框最大照片数
  DWORD dwPhotoRefreshTime = 0;       // 结婚相框刷新间隔
  DWORD dwCourtshipInviteOverTime = 0;//求婚戒指邀请框超时时间
  DWORD dwDivorceNpcDistance = 0;        //协议离婚距离npc的距离
  DWORD dwDivorceOverTime = 0;        //协议离婚过山车超时时间
  DWORD dwTicketItemId = 0;           //b格猫婚期礼券

  //servergame
  DWORD dwMaxReserveDay = 0;          //能提前预定的最大天数
  DWORD dwMaxTicketReserveDay = 0;    //使用券预定的天数
  DWORD dwDivorceBuffid = 0;          //离婚惩罚buffid
  DWORD dwWeddingManualId = 0;       //结婚手册道具id
  DWORD dwInvitationMailID = 0;       //邀请函邮件id
  DWORD dwInvitationItemID = 0;       //邀请函道具id
  DWORD dwDelManualMailID = 0;        //删除手册邮件id
  DWORD dwDelInvitationMailID = 0;    //删除邀请函邮件id
  DWORD dwDefaultRingID = 0;          //默认戒指id
  DWORD dwTopPackageID = 0;           //最高级套餐id
  DWORD dwRingShopType = 0;           //戒指商店type
  DWORD dwRingShopID = 0;             //戒指商店id

  DWORD dwShowQuestID = 0;            //与主教对话的展示任务
  TVecDWORD vecPreQuestion;           //答题前, 前言问题ID
  TVecDWORD vecFirstStageQuestion;    //第一阶段问题ID
  TVecDWORD vecSecondStageQuestion;   //第二阶段问题ID
  TSetDWORD setWeddingMsgTime;        //婚礼开始前公告时间(分钟)
  DWORD dwWeddingMsgID = 0;           //婚礼公告ID
  DWORD dwDressLetterId = 0;          //婚纱赠送的情书id
  DWORD dwWeddingNpcID = 0;           //大主教NpcID
  DWORD dwRingItemID = 0;             //婚礼戒指ID
  DWORD dwWeddingCertificate = 0;     //结婚证道具ID
  DWORD dwRollerCoasterMapId = 0;     //过山车副本id
  DWORD dwDivorceNpc = 0;             //协议离婚npcid
  DWORD dwDivorceRollerCoasterMapId = 0;     //协议离婚过山车副本id
  DWORD dwForceDivorceQuestId = 0;    //强制离婚任务id
  DWORD dwDivorceCarrierId = 0;              //离婚过山车载具id
  DWORD dwWeddingRaidID = 0;          //婚礼副本ID
  DWORD dwWeddingTeamBuff = 0;        //结婚双方组队buff
  DWORD dwWeddingCarrierID = 0;       //婚礼载具ID
  DWORD dwWeddingCarrierLine = 0;     //婚礼载具line

  TSetDWORD setMarrySkills;           //结婚后获得的技能

  bool bGenderCheck = false;          // 结婚性别检查
};

struct SKFCActivityCFG
{
  DWORD dwStartTime = 0;
  DWORD dwEndTime = 0;
  DWORD dwRewardID = 0;

  DWORD getReward() const
  {
    DWORD cur = now();
    return (cur >= dwStartTime && cur < dwEndTime) ? dwRewardID : 0;
  }
};

struct SProfessionMiscCFG
{
  DWORD dwGoldCost = 0; // 购买不同系职业金币消耗
  DWORD dwZenyCost = 0; // 购买相同系职业zeny消耗
  DWORD dwSwitchCost = 0; // 切换职业zeny消耗

  DWORD dwLoadCD = 0; // 切换分支冷却时间
};

struct SCardMiscCFG
{
  DWORD dwDecomposePriceID = 0;
  DWORD dwDecomposePriceCount = 0;
  DWORD dwDecomposeItemID = 0;
  DWORD dwExchangeCardMaxDraw = 0;
  map<DWORD, DWORD> mapDecomposeRate;
  map<DWORD, DWORD> mapDecomposeBase;

  DWORD getDecomposeItemCount(DWORD type) const
  {
    if (mapDecomposeRate.empty())
      return 0;
    auto it = mapDecomposeBase.find(type);
    if (it == mapDecomposeBase.end())
      return 0;
    DWORD r = randBetween(1, mapDecomposeRate.rbegin()->second), d = 0;
    for (auto v : mapDecomposeRate)
      if (r <= v.first)
      {
        d = v.second;
        break;
      }
    return it->second * d / 100;
  }
};

// pve 卡牌副本
struct SPveMiscCFG
{
  DWORD dwLimitLv = 0;
  DWORD dwWeekResetTime = 0; // 每周开始重置时间(距离本周一0点时间差, 下同)
  DWORD dwWeekResetEndTime = 0; //每周重置完成时间
  DWORD dwRaidCardSuitNum = 0; // 每个副本卡牌套数
  DWORD dwWeekRewardNum = 0; // 每周可获取的奖励次数

  DWORD dwBossCardNum = 0; // boss卡的数量
  DWORD dwAllMonsterCardNum = 0; //小怪卡的总数(包括普通小怪和小怪组合中的小怪)
  DWORD dwMaxSameMonsterNum = 0; // 小怪卡最多重复的数量
  DWORD dwAllItemAndEnvNum = 0; // 环境卡与道具卡的数量
  DWORD dwMinEnvCardNum = 0; // 环境卡最少的数量
  DWORD dwMinItemCardNum = 0; // 道具卡最少的数量
  DWORD dwMaxSameEnvOrItemNum = 0; //环境卡和道具卡最多重复出现的数量

  TSetDWORD setBossGroups;  // boss组的位置

  // 打牌相关
  DWORD dwPlayCardInterval = 0; // 敌方npc打牌间隔
  DWORD dwFriendCardDelay = 0; // 友方npc在敌方npc打牌结束后, 延迟出牌时间
  DWORD dwFinishCardCloseTime = 0; // 打牌结束后, 倒计时关闭时间
  DWORD dwPrepareCardTime = 0; // 点击开始后到第一次打牌准备时间
  DWORD dwFriendNpcID = 0; // 友方NpcID
  DWORD dwEnemyNpcID = 0; // 敌方NpcID
  DWORD dwSandGlassNpcID = 0; // 沙漏NpcID
  xPos oEnemyNpcDestPos; // 开始打牌后, 敌方Npc传送到的位置
  xLuaData oPreGotoEffect; // 敌方npc传送前特效
  xLuaData oEndGotoEffect; // 敌方npc传送后特效
  xPos oSafeCentralPos; // 毒圈中心位置
  TSetDWORD setSafeRange; // 毒圈收缩范围
  TSetDWORD setPosionBuff; // 毒圈buff
  DWORD dwSafeShrinkInterval = 0; // 毒圈收缩间隔, s
  std::vector<xPos> vecCardPos; // 副本3个打牌点
  DWORD dwEnemyCardDir = 0; // 敌方npc打牌, 牌(npc)朝向
  DWORD dwMaxValidDis = 0; // 离中心超出该距离强制瞬移到圈内

  DWORD dwNormalCardNpcID = 0; // 卡牌表现NpcID, 非boss
  DWORD dwBossCardNpcID = 0; // 卡牌表现NpcID, boss
  DWORD dwCardNpcShowTime = 0; // 卡牌出现到招怪时间
  DWORD dwCardNpcStayTime = 0; // 卡牌npc, 从招怪到删除时间
  xPos oFriendCardPos; // 友方npc打牌点
  DWORD dwSitActionID = 0; // npc坐下动作id
  DWORD dwEnemyNpcDir = 0; // 敌方npc传送后的方向
  DWORD dwItemDispTime = 0; // pve掉落道具删除时间

  DWORD dwDeadBossSummonTime = 0; // 亡者boss次数
  DWORD dwDeadBossUID = 0; // 亡者boss uniqueid
  DWORD dwDeadBossNum = 0; // 每次召唤数量
  DWORD dwDeadBossNpcUID = 0; // 交互npc
  DWORD dwDeadBossMinDifficulty = 0; // 召唤所需最低难度
  DWORD dwWaitDeadNpcTime = 0; // 等待召唤npc的时间

  TVecString vecEffectPath; // 毒圈收缩特效
  TVecDWORD vecSandGlassAction; // 开始收缩后沙漏动作

  bool canEnter(DWORD time) const
  {
    DWORD weekstart = xTime::getWeekStart(time);
    DWORD delta = time - weekstart;
    return !(delta <= dwWeekResetEndTime && delta >= dwWeekResetTime);
  }
};

struct SBuffMiscCFG
{
  TSetDWORD setNineSyncBuffs; // 需9屏同步的buff
};

struct SMvpTeamNumMatchCFG
{
  DWORD dwMinTeamNum = 0;  // 所需最小队伍数

  DWORD dwMinMatchTime = 0; // 最短匹配时间
  DWORD dwMaxMatchTime = 0; // 最长匹配时间
  DWORD dwMvpNum = 0; // mvp数量
  DWORD dwMiniNum = 0; // mini数量
  DWORD dwRaidID = 0; // 副本id
  DWORD dwMatchPunishTime = 0; // 下次匹配所需时间
};

// mvp 竞争战
struct SMvpBattleCFG
{
  DWORD dwMaxBossRewardCnt = 0;
  DWORD dwMaxMiniRewardCnt = 0;
  DWORD dwLimitUserLv = 0;
  DWORD dwLimitTeamUserNum = 0;
  DWORD dwEndDialogID = 0;
  DWORD dwEndWaitTime = 0;
  DWORD dwMaxMatchTime = 0;/*最长匹配时间,超过退出匹配队列*/

  DWORD dwResetCDTime = 0;
  DWORD dwMaxCD = 0;
  DWORD dwExpelTime = 0;
  DWORD dwDeadBossNum = 0; // 副本中召唤亡者boss数量
  DWORD dwDeadBossKillTime = 0; // 每周最多击杀亡者boss次数
  pair<DWORD, DWORD> paSummonDeadBossTime; // 召唤亡者boss时间段
  TSetDWORD setDeadBossUniqIDs; // 亡者boss召唤uniqueid

  vector<SMvpTeamNumMatchCFG> vecTeamNum2MatchInfo;
  vector<pair<DWORD, pair<DWORD, DWORD>>> time2ReliveCD;

  bool checkMatchOk(DWORD time, DWORD teamnum) const;
  const SMvpTeamNumMatchCFG* getMatchRoomInfo(DWORD teamnum) const;
  bool getBossAndMiniNum(DWORD teamnum, pair<DWORD, DWORD>& bossAndMini) const;
  DWORD getReliveCD(DWORD time) const
  {
    for (auto &v : time2ReliveCD)
    {
      if (v.second.first <= time && v.second.second >= time)
        return v.first;
    }
    return 0;
  }
};

struct SServantCFG
{
  DWORD dwMaxFavorability = 0;
  float fRange = 0.0f;
  map<DWORD, string> m_mapDescription;
  map<DWORD, DWORD> m_mapVar;
  vector<pair<DWORD, DWORD>> m_vecReward;

  map<DWORD, map<DWORD, DWORD>> m_mapGrowthReward;
  DWORD dwStayTime = 0;
  DWORD dwStayNum = 0;
  DWORD dwStayFavorability = 0;
  DWORD dwDisappearTime = 0;
  DWORD dwAddFavo = 0;
  DWORD dwCemeteryRaid = 0;
  DWORD dwSpaceRaid = 0;
  DWORD dwTerroristRaid = 0;

  bool isExist(DWORD npcid) const
  {
    auto m = m_mapDescription.find(npcid);
    return m != m_mapDescription.end();
  }

  const string& getServantName(DWORD npcid) const
  {
    auto m = m_mapDescription.find(npcid);
    if(m != m_mapDescription.end())
      return m->second;

    return STRING_EMPTY;
  }

  DWORD getReward(DWORD dwFavorability) const
  {
    for(DWORD i = 0; i < m_vecReward.size(); ++i)
    {
      if(m_vecReward[i].first == dwFavorability)
        return m_vecReward[i].second;
    }

    return 0;
  }

  DWORD getFavorabilityVar(DWORD dwFavorability) const
  {
    auto m = m_mapVar.find(dwFavorability);
    if(m != m_mapVar.end())
      return m->second;

    return 0;
  }

  vector<pair<DWORD, DWORD>> getFavorabilityReward() const { return m_vecReward; }

  DWORD getGrowthRewardNum(DWORD groupid) const
  {
    DWORD num = 0;
    auto it = m_mapGrowthReward.find(groupid);
    if(it != m_mapGrowthReward.end())
      num = it->second.size();

    return num;
  }

  DWORD getGrowthRewardID(DWORD groupid, DWORD value) const
  {
    DWORD reward = 0;
    auto it = m_mapGrowthReward.find(groupid);
    if(it != m_mapGrowthReward.end())
    {
      auto iter = it->second.find(value);
      if(iter != it->second.end())
        reward = iter->second;
    }

    return reward;
  }

  DWORD getCanGrowthRewardNum(DWORD groupid, DWORD value) const
  {
    DWORD num = 0;
    auto it = m_mapGrowthReward.find(groupid);
    if(it != m_mapGrowthReward.end())
    {
      for(auto &m : it->second)
      {
        if(value >= m.first)
          ++num;
      }
    }

    return num;
  }
};

//存档位
struct SRecordCFG
{
  DWORD dwDefaultSlotNum = 0; //默认开启格子数
  DWORD dwMonthCardSlotNum = 0; //月卡开启数量
  DWORD dwTotalBuyNum = 0; //总共可购买次数
  DWORD dwLoadCD = 0;  //加载存档cd 单位秒
  DWORD dwReocrdNameMaxLen = 0;  //存档名的最大长度
  DWORD dwRestoreBookItemID = 0; //伊米尔的记事簿 物品ID
  DWORD dwOpenNoviceSkillID = 0; //时空断裂 冒险技能ID
  DWORD dwMenuID = 0; // 时空断裂技能解锁的功能ID
  map<DWORD /*buy times*/, pair<DWORD /* itemid */, DWORD /* itemnum */>> mapBuyTimes2CostIDAndNum; //购买格子 价格信息
};

struct SVarMiscCFG
{
  DWORD dwOffset = 0;
  map<EVarType, DWORD> mapVar2Offset;
  map<EAccVarType, DWORD> mapAccVar2Offset;

  DWORD getOffset(EVarType var) const
  {
    auto it = mapVar2Offset.find(var);
    if (it != mapVar2Offset.end())
      return it->second;
    return dwOffset;
  }
  DWORD getOffset(EAccVarType var) const
  {
    auto it = mapAccVar2Offset.find(var);
    if (it != mapAccVar2Offset.end())
      return it->second;
    return dwOffset;
  }
};

struct SCapraActivityCFG
{
  DWORD mapId = 0;
  TVecDWORD m_vecAddQuests;               // 添加任务
  TVecDWORD m_vecDelQuests;               // 删除任务
  TVecDWORD m_vecDelBuffs;                // 删除buff
};

struct SAltmanCFG
{
  DWORD dwDefaultSkillID = 0;
  DWORD dwMaxSkillPos = 0;
  DWORD dwRewardID = 0;
  DWORD dwClearNpcTime = 0;
  DWORD dwFashionBuff = 0;
  TSetDWORD setEnterBuffs;
  TSetDWORD setManualHeads;
  TSetDWORD setFashionEquips;
  vector<pair<DWORD, DWORD>> vecHeadReward;
  vector<pair<DWORD, DWORD>> vecKillReward;
  map<DWORD, DWORD> mapItem2Skill;
  map<DWORD, string> mapKill2Title;

  xLuaData oRewardBoxDefine;

  DWORD randomTransBuff() const
  {
    DWORD* pBuff = randomStlContainer(setEnterBuffs);
    if(pBuff == nullptr)
      return 0;
    return *pBuff;
  }

  DWORD getHeadReward(DWORD num) const
  {
    for(auto it = vecHeadReward.rbegin(); it != vecHeadReward.rend(); ++it)
    {
      if(num >= it->first)
        return it->second;
    }

    return 0;
  }
  DWORD getKillReward(DWORD num) const
  {
    for(auto it = vecKillReward.rbegin(); it != vecKillReward.rend(); ++it)
    {
      if(num >= it->first)
        return it->second;
    }

    return 0;
  }
  bool isAltmanFashion(DWORD dwid) const
  {
    return setFashionEquips.find(dwid) != setFashionEquips.end();
  }

  const string& getTitleByKill(DWORD kill) const
  {
    DWORD dwTemp = 0;
    for (auto &s : mapKill2Title)
    {
      if (kill >= s.first && s.first > dwTemp)
        dwTemp = s.first;
    }

    auto it = mapKill2Title.find(dwTemp);
    if (it != mapKill2Title.end())
      return it->second;

    return STRING_EMPTY;
  }
};

// activity
struct SActMiscCFG
{
  map<DWORD, DWORD> mapPetWorkEndTime;

  DWORD getPetWorkEndTime(DWORD dwID) const
  {
    auto m = mapPetWorkEndTime.find(dwID);
    if (m != mapPetWorkEndTime.end())
      return m->second;
    return 0;
  }
};

typedef map<DWORD, TSetDWORD> TMapDressEquip;
struct SDressStageCFG
{
  DWORD m_dwStaticMap = 0;
  DWORD m_dwShowTime = 0;
  DWORD m_dwMateRange = 0;
  DWORD m_dwStageRange = 0;
  DWORD m_dwUnattackedBuff = 0;
  DWORD m_dwRideWolfSkill = 0;
  xPos m_pQuitPos;
  xPos m_pMusicPos;
  TSetDWORD m_setStageIDs;
  map<DWORD, DWORD> m_mapCost;
  map<DWORD, xPos> m_mapEnterPos;
  map<DWORD, pair<xPos,xPos>> m_mapDoubleEnterPos;
  map<DWORD, TMapDressEquip> m_mapVaildEquip;
  map<DWORD, map<DWORD, DWORD>> m_mapDefaultEquip;

  bool canEquip(DWORD stageid, DWORD equiptype, DWORD equipid) const
  {
    auto it = m_mapVaildEquip.find(stageid);
    if(it == m_mapVaildEquip.end())
      return false;
    auto iter = it->second.find(equiptype);
    if(iter == it->second.end())
      return false;
    auto iterEquip = iter->second.find(equipid);
    if(iterEquip == iter->second.end())
      return false;

    return true;
  }

  const xPos getEnterPos(DWORD id) const
  {
    xPos pos;
    auto it = m_mapEnterPos.find(id);
    if(it != m_mapEnterPos.end())
      pos = it->second;

    return pos;
  }
};

struct SExchangeShopCFG
{
  DWORD dwMinDelayTime = 0;
  DWORD dwMaxDelayTime = 0;
  DWORD dwBaseWorldLevelExpBuff = 0;
  DWORD dwJobWorldLevelExpBuff = 0;

  set<EProfession> setOpenPros;

  bool isProOpen(EProfession eProfession) const { return setOpenPros.find(eProfession) == setOpenPros.end(); }
};

// dead
struct SDeadMiscCFG
{
  DWORD dwMaxCoin = 0;
  DWORD dwQuestNum = 0;
  DWORD dwMaxLv = 0;
};

struct SPwsMagicBallData
{
  EMagicBallType eBallType = EMAGICBALL_MIN;
  DWORD dwSummonNpcID = 0;
  DWORD dwBuffID = 0;
  string strName;
};

struct SPwsMagicCombine
{
  DWORD dwConfigID = 0;
  EMagicBallType eBall1 = EMAGICBALL_MIN;
  EMagicBallType eBall2 = EMAGICBALL_MIN;
  TSetDWORD setBuffIDs;
  string strName;
};

struct SPwsPickBuffEffect
{
  map<DWORD, DWORD> mapSelfBuff2Time;
  map<DWORD, DWORD> mapTeamBuff2Time;
  string strName;
};

struct SPwsSeasonReward
{
  DWORD dwNeedRank = 0; // 最低排名
  DWORD dwMailID = 0; // 奖励邮件
};

struct SPwsColorInfo
{
  string strName;
};

struct SPwsRankCFG
{
  ETeamPwsRank eRank = ETEAMPWSRANK_NONE;
  DWORD dwMinScore = 0;
  DWORD dwMaxScore = 0;

  TVecItemInfo vecWinItems;
  TVecItemInfo vecLoseItems;
};

// 组队排位赛
struct STeamPwsCFG
{
  // 匹配相关
  DWORD dwCacheMatchNum = 0;
  DWORD dwMaxTeamMatchTime = 0;
  DWORD dwMaxFireMatchTime = 0;
  DWORD dwMaxAPoolMatchTime = 0;
  DWORD dwMaxBPoolMatchTime = 0;
  DWORD dwMaxPrepareTime = 0; // 准备超时时间
  DWORD dwWeekMaxCount = 0; // 每周最多参加次数
  DWORD dwRequireLv = 0; // 参战所需等级
  DWORD dwMaxRecordCnt = 0; // 最多纪录的排名数量
  DWORD dwLeaveTeamPunishCD = 0; // 离队惩罚cd
  int intLeaveTeamPunishScore = 0; // 离队惩罚分数
  DWORD dwSeasonBattleTimes = 0; // 每个赛季活动次数
  DWORD dwRewardTime = 0; // 最后一次比赛开赛后, 多长时间奖励
  vector<pair<DWORD,DWORD>> vecMatchScoreGroup; // 匹配分组, vec[pa(minscore,maxscore)]
  map<ETeamPwsRank, SPwsRankCFG> mapRankInfo; // 段位分数&奖励信息
  vector<SPwsSeasonReward> vecSeasonReward; // 赛季奖励
  TSetDWORD setRaidIDs; // 副本

  // 副本相关
  DWORD dwLastTime = 0; // 副本持续时间
  DWORD dwCollectSkill = 0; // 采集元素球技能
  DWORD dwColleckBuffSkill = 0; // 采集buff技能
  DWORD dwBallInterval = 0; // 分数增加间隔s
  DWORD dwBallDelayTime = 0; // 添加后延迟生效时间s
  DWORD dwWinScore = 0; // 获胜所需积分
  DWORD dwPrepareTime = 0; // 准备时间
  DWORD dwMagicCD = 0; // 魔法球组合更换CD
  DWORD dwKillScore = 0; // 击杀玩家得分
  DWORD dwSummonBallTime = 0; // 开场后多久召唤法球
  DWORD dwWarnScore = 0; // 提示分数
  DWORD dwRewardMailID = 0; // 副本奖励邮件

    //场地buff
  DWORD dwPickBuffScore = 0; // 拾取buff得分
  DWORD dwBuffNpcBeginTime = 0; // 开始刷新buff的时间
  DWORD dwBuffNpcInterval = 0; // 刷新间隔
  DWORD dwBuffNpcUniqueID = 0; // buff召唤npc位置
  DWORD dwBuffNpcClearTime = 0; // buff npc存活时间
  map<DWORD, SPwsPickBuffEffect> mapNpc2Buffs; // 场地buff效果

  TSetDWORD setBallBuffs; // 持球添加的buff
  map<EMagicBallType, SPwsMagicBallData> mapBallData; // 元素球配置信息
  TVecDWORD vecBallUniqueID; // 元素法球uniqueid
  map<DWORD, DWORD> mapBallNum2Score; // 持球数->间隔增分数
  map<DWORD, SPwsMagicCombine> mapBallCombines; // 魔法球组合
  map<DWORD, SPwsColorInfo> mapColorInfo; // 参战方信息
  vector<pair<xPos,float>> vecPreLimitArea; // 开战前, 限制移动区域
  TVecDWORD vecAllScoreAdd; // 总表现分, 排名得分
  TVecDWORD vecKillScoreAdd; // 击杀数量, 排名得分
  TVecDWORD vecHealSocreAdd; // 治疗量, 排名得分

  bool isOutLimitArea(const xPos& p) const
  {
    for (auto &v : vecPreLimitArea)
    {
      if (getXZDistance(p, v.first) <= v.second)
        return false;
    }
    return true;
  }

  const SPwsMagicBallData* getBallData(EMagicBallType e) const
  {
    auto it = mapBallData.find(e);
    return it != mapBallData.end() ? &(it->second) : nullptr;
  }

  DWORD getGroupByScore(DWORD score) const
  {
    DWORD group = 0;
    for (auto &v : vecMatchScoreGroup)
    {
      if (score >= v.first && score < v.second)
        return group;
      group ++;
    }
    return 0;
  }

  ETeamPwsRank getERankByScore(DWORD score) const
  {
    for (auto &m : mapRankInfo)
    {
      if (score > m.second.dwMinScore && score <= m.second.dwMaxScore)
        return m.first;
    }
    return ETEAMPWSRANK_NONE;
  }
  const SPwsRankCFG* getRankInfoByScore(DWORD score) const
  {
    if (score == 0)
    {
      auto it = mapRankInfo.find(ETEAMPWSRANK_NORMAL);
      return it != mapRankInfo.end() ? &(it->second) : nullptr;
    }
    auto e = getERankByScore(score);
    auto it = mapRankInfo.find(e);
    return it != mapRankInfo.end() ? &(it->second) : nullptr;
  }

  DWORD getScoreByBallNum(DWORD num) const
  {
    auto it = mapBallNum2Score.find(num);
    return it != mapBallNum2Score.end() ? it->second : 0;
  }
  const SPwsMagicCombine* getMagicByBall(EMagicBallType b1, EMagicBallType b2) const
  {
    DWORD configid = (b1 < b2 ? (DWORD)b1 * 10 + (DWORD)b2 : (DWORD)b2 * 10 + (DWORD)b1);
    auto it = mapBallCombines.find(configid);
    if (it == mapBallCombines.end())
      return nullptr;
    return &(it->second);
  }
  const SPwsMagicCombine* getMagicByID(DWORD id) const
  {
    auto it = mapBallCombines.find(id);
    if (it == mapBallCombines.end())
      return nullptr;
    return &(it->second);
  }
  DWORD getMagicIDByBall(EMagicBallType b1, EMagicBallType b2) const
  {
    auto p = getMagicByBall(b1, b2);
    return p ? p->dwConfigID : 0;
  }
  const SPwsColorInfo* getColorInfo(DWORD color) const
  {
    auto it = mapColorInfo.find(color);
    return it != mapColorInfo.end() ? &(it->second) : nullptr;
  }
};

// misc config
class MiscConfig : public xSingleton<MiscConfig>
{
  public:
    MiscConfig();
    virtual ~MiscConfig();

    bool loadConfig();
    bool checkConfig();

    // user rename
    const SPlayerRenameCFG getPlayerRenameCFG() const { return m_stPlayerRenameCFG; }

    // user relive
    const SReliveCFG& getReliveCFG() const { return m_stReliveCFG; }

    // scene item
    const SSceneItemCFG& getSceneItemCFG() const { return m_stSceneItemCFG; }

    // purify
    const SPurifyCFG& getSPurifyCFG() const { return m_stSPurifyCFG; }

    // new role
    const SNewRoleCFG& getNewRoleCFG() const { return m_stNewRoleCFG; }

    // team
    const STeamCFG& getTeamCFG() const { return m_stTeamCFG; }

    // strength
    //const SStrengthCFG& getStrengthCFG() const { return m_stStrengthCFG; }

    // NpcAI
    //const SNpcAICFG& getNpcAICFG() const { return m_stNpcAICFG; }

    // equip decompose
    //const SEquipDecomposeCFG* getEquipDecomposeCFG(EQualityType eQuality, EItemType eType) const;

    // package
    const SPackageCFG& getPackageCFG() const {return m_stPackageCFG;}

    // card
    bool canEquip(DWORD position, EItemType eType);

    // quest
    const SQuestMiscCFG& getQuestCFG() const { return m_stQuestCFG; }
    const SQuestTableCFG& getQuestTableCFG() const { return m_stQuestTableCFG; }

    // infinite tower
    const SEndlessTowerCFG& getEndlessTowerCFG() const { return m_stEndlessTowerCFG;};

    // seal
    const SealMiscCFG& getSealCFG() const { return m_stSealCFG; }

    DWORD getRefineRate(DWORD itemid);

    // system
    const SSystemCFG& getSystemCFG() const { return m_stSystemCFG; }

    // be lock
    const SLockEffectCFG& getBeLockCFG() const { return m_stBeLockCFG; }

    // effect path
    const SEffectPath& getEffectPath() const { return m_stEffectPath; }

    // social
    const SSocialityMiscCFG& getSocialCFG() const { return m_stSocialityCFG; }

    // produce
    const SProduceCFG& getProduceCFG() const { return m_stProduceCFG; }

    // laboratory
    const SLaboratoryCFG& getLaboratoryCFG() const { return m_stLaboratoryCFG; }

    // chatroom
    const SChatRoomMiscCFG& getChatRoomCFG() const { return m_stChatRoomCFG; }

    // guild
    const SGuildMiscCFG& getGuildCFG() const { return m_stGuildCFG; }

    // trade
    const STradeCFG& getTradeCFG() const { return m_stTradeCFG; }

    // booth
    const SBoothCFG& getBoothCFG() const { return m_stBoothCFG; }

    //pvpconfig
    const SPvpCFG* getPvpCfg(DWORD type)
    {
      auto it = m_mapPvpCFG.find(type);
      if (it == m_mapPvpCFG.end()) return nullptr;
      return &(it->second);
    }

    // trap npc
    const STrapNpcCFG& getTrapNpcCFG() const { return m_stTrapNpcCFG; }

    // guild dojo
    const SGuildDojoCFG& getGuildDojoCFG() const { return m_stGuildDojoCFG; }

    // refine
    const SRefineActionCFG& getRefineActionCFG() const { return m_stRefineCFG; }
    // equip
    const SStrengthActionCFG& getStrengthActionCFG() const { return m_stStrengthCFG; }

    // expression
    const SExpressionCFG& getExpressionCFG() const { return m_stExpressionCFG; }

    // attr
    const SAttrMiscCFG& getAttrCFG() const { return m_stAttrCFG; }

    // delay
    const SDelayMiscCFG& getDelayCFG() const { return m_stDelayCFG; }

    // monster
    const SMonsterMiscCFG& getMonsterCFG() const { return m_stMonsterCFG; }

    // hand npc
    const SHandNpcCFG& getHandNpcCFG() const { return m_stHandNpcCFG; }

    // hand npc
    const SItemImageCFG& getItemImageCFG() const { return m_stItemImageCFG; }

    // treasure
    const STreasureMiscCFG& getTreasureCFG() const { return m_stTreasureCFG; }

    // camera
    const SCameraCFG& getCameraCFG() const { return m_stCameraCFG; }

    // zone
    const SZoneMiscCFG& getZoneCFG() const { return m_stZoneCFG; }

    //Q&A
    const SQACFG& getQACFG() const { return m_stQACFG; }

    // item
    const SItemMiscCFG& getItemCFG() const { return m_stItemCFG; }

    const SFriendShipCFG& getFriendShipCFG() const {return m_stFriendShipCFG;}

    const SDepositTypeCFG* getDepositTypeCFG(EDepositCardType type)
    {
      auto it = m_mapDepositTypeCFG.find(type);
      if (it == m_mapDepositTypeCFG.end()) return nullptr;
      return &(it->second);
    }

    const SExtraRewardCFG* getExtraRewardCFG(ETaskExtraRewardType type)
    {
      auto it = m_mapExtraRewardCFG.find(type);
      if (it == m_mapExtraRewardCFG.end()) return nullptr;
      return &(it->second);
    }

    const DWORD getSafeRefineCount(DWORD refinelv, bool discount)
    {
      auto it = m_mapSafeRefineCFG.find(refinelv);
      if (it == m_mapSafeRefineCFG.end()) return DWORD_MAX;
      if(discount == false)
        return it->second.normal_cost;

      return it->second.discount_cost;
    }
    const DWORD getSafeRefineLotteryCount(DWORD refinelv)
    {
      auto it = m_mapSafeRefineLotteryCFG.find(refinelv);
      if (it == m_mapSafeRefineLotteryCFG.end()) return DWORD_MAX;
      return it->second;
    }

    const SOperRewardMiscCfg& getOperateRewardMiscCFG()const
    {
      return m_operRewardMiscCfg;
    }

    // mvp score cfg
    const SMvpScoreCFG& getMvpScoreCFG() const { return m_stMvpScoreCFG; }
    void formatMvpInfo(EMvpScoreType eType, TVecString& param, const SMvpKillInfo& info, bool bShowHighest) const;

    // credit
    const SCreditCFG& getCreditCFG() const { return m_stCreditCFG; }

    // money cat
    const SMoneyCatCFG& getMoneyCatCFG() const { return m_stMoneyCatCFG; }

    const SMapTransCFG& getMapTransCFG() const { return m_stMapTransCFG; }

    const SAuguryCFG& getAuguryCfg() const { return m_stAugury; }

    const SAuthorizeCFG& getAuthorize() const { return m_stAuthorize; }

    // checc item del
    const SItemCheckDelCFG& getItemCheckDelCFG() const { return m_stItemCheckDelCFG; }

    // skill
    const SSkillMiscCFG& getSkillCFG() const { return m_stSkillCFG; }

    // pet
    const SPetMiscCFG& getPetCFG() const { return m_stPetCFG; }

    // arena
    const SArenaMiscCFG& getArenaCFG() const { return m_stArenaCFG; }

    // buff status
    const SBuffStatusCFG& getBuffStatusCFG() const { return m_stBuffStatusCFG; }
    bool loadServerGameConfig();

    // guild raid
    const SGuildRaidCFG& getGuildRaidCFG() const { return m_stGuildRaidCFG; }

    // weaponpet
    const SWeaponPetMiscCFG& getWeaponPetCFG() const { return m_stWeaponPetCFG; }

    // 星盘
    const SAstrolabeMiscCFG& getAstrolabeCFG() const { return m_stAstrolabeCFG; }
    const SAstrolabeInfo* getAstrolabeInfo(DWORD id) const
    {
      auto it = m_stAstrolabeCFG.mapAstrolabeInfo.find(id);
      if (it == m_stAstrolabeCFG.mapAstrolabeInfo.end())
        return nullptr;
      return &it->second;
    }

    const DWORD getItemGetLimitMsg(EItemGetLimitType type) const
    {
      auto it = m_mapGetLimitMsg.find(type);
      if (it == m_mapGetLimitMsg.end())
        return 0;
      return it->second;
    }

    // pvp 奖励
    const SPvpCommonCFG& getPvpCommonCFG() const { return m_stPvpCommonCFG; }

    //半周年庆典
    const SMonthCardActivityCFG& getMonthCardActivityCFG() const { return m_stMonthCardActivityCfg; }
    SCelebrationMCardCFG* getCelebrationMCardCFGbyID(ECelebrationLevel type);
    const SFunctionSwitchCFG& getFunctionSwitchCFG() const { return m_stFunctionSwitchCfg; }

    // 成就
    const SAchieveMiscCFG& getAchieveCFG() const { return m_stAchieveCFG; }
    // 聊天频道
    const SChatChannelCFG& getChatChannelCFG() const { return m_stChatChannelCFG; }

    // 个人相册
    const SUserPhotoCFG& getPhotoCFG() const { return m_stUserPhotoCFG; }

    // 料理
    const SFoodMiscCFG& getFoodCfg() const { return m_stFood; }

    // Timer表
    const STimerTableCFG& getTimerTableCFG() const { return m_stTimerTableCFG; }

    const STradeBlack& getTradeBlock() const { return m_stTradeBlack; }

    //扭蛋
    const SLotteryMiscCFG& getLotteryCFG() const { return m_stLotteryCFG; }

    const SAuctionMiscCFG& getAuctionMiscCFG() const { return m_stAuctionMiscCFG; }

    // 商店
    const SShopInfo* getShopInfo(DWORD shoptype, DWORD shopid) const
    {
      auto it = m_stShopCFG.mapInfos.find(shoptype);
      if (it == m_stShopCFG.mapInfos.end())
        return nullptr;
      auto infoit = it->second.find(shopid);
      if (infoit == it->second.end())
        return nullptr;
      return &infoit->second;
    }
    const SShopCFG& getShopCFG() const { return m_stShopCFG; }
    const SGameShopCFG& getGameShopCFG() const { return m_stGameShopCFG; }

    // 导师
    const STutorMiscCFG& getTutorCFG() const { return m_stTutorCFG; }

    // 配置屏蔽检查
    bool isForbid(const string& tablename, DWORD id) const { return m_stBranchForbidCFG.isForbid(tablename, id); }

    const SGuildFireCFG& getGuildFireCFG() const { return m_stGuildFireCFG; }
    const SSuperGvgCFG& getSuperGvgCFG() const { return m_stGuildFireCFG.stSuperGvgCFG; }
    const SMapBranchForbidCFG& getBranchForbid() const { return m_stMapBranchForbidCFG; }

    const SQuotaCFG& getQuotaCFG() const { return m_stQuotaCFG; }

    const bool isSystemForbid(ESystemForbidType eType);
    const SAutoSkillGroupCFG& getAutoSkillGrp() const { return m_stAutoSkillGrpCFG; }

    const SPoliFireCFG& getPoliFireCFG() const { return m_stPoliFireCFG; }

    // 炼金术师生命体
    const SBeingMiscCFG& getBeingCFG() const { return m_stBeingCFG; }

    // 冒险手册
    const SManualMiscCFG& getManualCFG() const { return m_stManualCFG; }

    const SLoveLetterCFG& getLetterCFG() const { return m_stLetterCFG; }

    const SPeakLevelCFG& getPeakLevelCFG() const { return m_stPeakLevelCFG; }
    const DWORD getSkillBreakPoint() const { return m_stPeakLevelCFG.m_dwSkillBreakPoint; }
    const DWORD getAddLvByItem(DWORD item, DWORD num) const;

    const SPetAdventureEffCFG& getPetAdvEffCFG() const { return m_stPetAdvEffCFG; }

    const SFuncForbidCFG& getFuncForbidCFG() const { return m_stFuncForbid; }

    const SGuildBuildingMiscCFG& getGuildBuildingCFG() { return m_stGuildBuildingCFG; }
    const SGuildWelfareMiscCFG& getGuildWelfareCFG() { return m_stGuildWelfareCFG; }
    const SGuildChallengeMiscCFG& getGuildChallengeCFG() { return m_stGuildChallengeCFG; }
    const SJoyLimitCFG& getJoyLimitCFG() const { return m_stJoyCFG; }
    const DWORD getJoyLimitByType(EJoyActivityType eType);
    const DWORD getJoyReward(DWORD oldjoy, DWORD newjoy);
    const DWORD getFoodValue(DWORD foodid);
    const SBossMiscCFG& getBossCFG() const { return m_stBossCFG; }

    const SArtifactMiscCFG& getArtifactCFG() { return m_stArtifactCFG; }

    const SDepositMiscCFG& getSDepositMiscCFG() const { return m_stSDepositCFG; }

    // 周年庆-好友回归
    const SRecallCFG& getRecallCFG() const { return m_stRecallCFG; }

    const SWeddingMiscCFG& getWeddingMiscCFG() const { return m_stWeddingMiscCFG; }
    const SKFCActivityCFG& getKFCActivityCFG() const { return m_stKFCActivityCFG; }
    const SCardMiscCFG& getCardMiscCFG() const { return m_stCardMiscCFG; }
    const SProfessionMiscCFG& getProfessionMiscCFG() const { return m_stProfessionMiscCFG; }

    // pve
    const SPveMiscCFG& getMiscPveCFG() const { return m_stPveCFG; }

    const SBuffMiscCFG& getBuffMiscCFG() const { return m_stBuffMiscCFG; }
    const SMvpBattleCFG& getMvpBattleCFG() const { return m_stMvpBattleCFG; }

    const SServantCFG& getServantCFG() { return m_stServantCFG; }
    const SRecordCFG& getMiscRecordCFG() const { return m_stRecordCFG; }
    const SVarMiscCFG& getVarMiscCFG() const { return m_stVarMiscCFG; }
    const SCapraActivityCFG& getCapraActivityCFG() const { return m_stCapraActivityCFG; }

    const SAltmanCFG& getAltmanCFG() const { return m_stAltmanCFG; }

    DWORD getRaidByType(ERaidType eType) const;

    const SActMiscCFG& getActCFG() const { return m_stActCFG; }

    const SDressStageCFG& getDressStageCFG() const { return m_stDressStageCFG; }
    const SExchangeShopCFG& getExchangeShopCFG() const { return m_stExchangeShopCFG; }
    const SDeadMiscCFG& getDeadCFG() const { return m_stDeadCFG; }

    const STeamPwsCFG& getTeamPwsCFG(bool relax = false ) const { return relax ? m_stRelaxTeamPwsCFG : m_stTeamPwsCFG; }
  private:
    bool loadObscenceLanguageConfig();
    bool loadUnionConfig();
  private:
    // user rename
    SPlayerRenameCFG m_stPlayerRenameCFG;

    // user relive
    SReliveCFG m_stReliveCFG;

    // scene item
    SSceneItemCFG m_stSceneItemCFG;

    // purify
    SPurifyCFG m_stSPurifyCFG;

    // new role
    SNewRoleCFG m_stNewRoleCFG;

    // team
    STeamCFG m_stTeamCFG;

    // strength
    //SStrengthCFG m_stStrengthCFG;

    // NpcAI
    //SNpcAICFG m_stNpcAICFG;

    // equip decompose
    //TVecEquipDecomposeCFG m_vecEquipDecomposeCFG;

    // package
    SPackageCFG m_stPackageCFG;

    // card
    SCardCFG m_stCardCFG;

    // quest
    SQuestMiscCFG m_stQuestCFG;
    SQuestTableCFG m_stQuestTableCFG;

    // infinite tower
    SEndlessTowerCFG m_stEndlessTowerCFG;

    // seal
    SealMiscCFG m_stSealCFG;

    // refine rate
    TVecItem2Rate m_vecItem2Rate;

    // system
    SSystemCFG m_stSystemCFG;

    // be lock config
    SLockEffectCFG m_stBeLockCFG;

    // effect path info
    SEffectPath m_stEffectPath;

    // sociality
    SSocialityMiscCFG m_stSocialityCFG;

    // produce
    SProduceCFG m_stProduceCFG;

    // garden
    SLaboratoryCFG m_stLaboratoryCFG;

    // chatroom
    SChatRoomMiscCFG m_stChatRoomCFG;

    // guild
    SGuildMiscCFG m_stGuildCFG;

    // trade
    STradeCFG m_stTradeCFG;

    // booth
    SBoothCFG m_stBoothCFG;

    //pvpconfig
    std::map<DWORD, SPvpCFG> m_mapPvpCFG;

    // trap npc
    STrapNpcCFG m_stTrapNpcCFG;

    // guild dojo
    SGuildDojoCFG m_stGuildDojoCFG;

    // refine
    SRefineActionCFG m_stRefineCFG;

    // strength
    SStrengthActionCFG m_stStrengthCFG;

    // expression
    SExpressionCFG m_stExpressionCFG;

    // attr
    SAttrMiscCFG m_stAttrCFG;

    // delay
    SDelayMiscCFG m_stDelayCFG;

    // monster
    SMonsterMiscCFG m_stMonsterCFG;

    // hand npc
    SHandNpcCFG m_stHandNpcCFG;

    SItemImageCFG m_stItemImageCFG;

    // treasure
    STreasureMiscCFG m_stTreasureCFG;

    // camera
    SCameraCFG m_stCameraCFG;

    // zone
    SZoneMiscCFG m_stZoneCFG;

    //Q&A
    SQACFG m_stQACFG;

    // item
    SItemMiscCFG m_stItemCFG;

    // friendship reward
    SFriendShipCFG m_stFriendShipCFG;

    // SDepositTypeCFG
    std::map<EDepositCardType, SDepositTypeCFG> m_mapDepositTypeCFG;

    // mvp score cfg
    SMvpScoreCFG m_stMvpScoreCFG;

    SOperRewardMiscCfg m_operRewardMiscCfg;

    // credit cfg
    SCreditCFG m_stCreditCFG;

    // money cat cfg
    SMoneyCatCFG m_stMoneyCatCFG;
    SMapTransCFG m_stMapTransCFG;
    // item check del cfg
    SItemCheckDelCFG m_stItemCheckDelCFG;
    SAuguryCFG  m_stAugury;

    SAuthorizeCFG m_stAuthorize;
    // skill cfg
    SSkillMiscCFG m_stSkillCFG;
    // pet
    SPetMiscCFG m_stPetCFG;
    // weapon pet
    SWeaponPetMiscCFG m_stWeaponPetCFG;
    // arena
    SArenaMiscCFG m_stArenaCFG;
    // guild raid cfg
    SGuildRaidCFG m_stGuildRaidCFG;
    // extrareward
    std::map<ETaskExtraRewardType, SExtraRewardCFG> m_mapExtraRewardCFG;
    //SafeRefineEquipCost
    std::map<DWORD, SSafeRefineCost> m_mapSafeRefineCFG;
    //SafeRefineEquipCostLottery
    std::map<DWORD, DWORD> m_mapSafeRefineLotteryCFG;

    // buff status
    SBuffStatusCFG m_stBuffStatusCFG;
    // 星盘
    SAstrolabeMiscCFG m_stAstrolabeCFG;
    // 道具获取数量限制msg
    map<EItemGetLimitType, DWORD> m_mapGetLimitMsg;
    // pvp 奖励
    SPvpCommonCFG m_stPvpCommonCFG;
    //半周年庆典
    SMonthCardActivityCFG m_stMonthCardActivityCfg;
    SFunctionSwitchCFG m_stFunctionSwitchCfg;
    // 成就
    SAchieveMiscCFG m_stAchieveCFG;
    // 聊天频道
    SChatChannelCFG m_stChatChannelCFG;
    // 个人相册
    SUserPhotoCFG m_stUserPhotoCFG;

    // 料理
    SFoodMiscCFG m_stFood;
    //Timer表
    STimerTableCFG m_stTimerTableCFG;

    STradeBlack m_stTradeBlack;
    //扭蛋机
    SLotteryMiscCFG m_stLotteryCFG;
    SAuctionMiscCFG m_stAuctionMiscCFG;

    // 商店
    SShopCFG m_stShopCFG;
    SGameShopCFG m_stGameShopCFG;
    // 导师
    STutorMiscCFG m_stTutorCFG;
    // 配置屏蔽列表
    SBranchForbidCFG m_stBranchForbidCFG;
    SGuildFireCFG m_stGuildFireCFG;

    // 分支屏蔽
    SMapBranchForbidCFG m_stMapBranchForbidCFG;

    SQuotaCFG m_stQuotaCFG;
    // 系统屏蔽
    std::map<ESystemForbidType, DWORD> m_mapSystemsForbidCFG;

    // 自动技能组
    SAutoSkillGroupCFG m_stAutoSkillGrpCFG;

    // 波利乱斗配置
    SPoliFireCFG m_stPoliFireCFG;

    // 炼金术师生命体
    SBeingMiscCFG m_stBeingCFG;
    // 冒险手册
    SManualMiscCFG m_stManualCFG;
    //情书配置
    SLoveLetterCFG m_stLetterCFG;
    // 巅峰等级
    SPeakLevelCFG m_stPeakLevelCFG;

    // 宠物冒险战斗效率相关配置
    SPetAdventureEffCFG m_stPetAdvEffCFG;

    //功能屏蔽
    SFuncForbidCFG m_stFuncForbid;
    // 公会建筑
    SGuildBuildingMiscCFG m_stGuildBuildingCFG;
    // 公会福利
    SGuildWelfareMiscCFG m_stGuildWelfareCFG;
    // 公会挑战
    SGuildChallengeMiscCFG m_stGuildChallengeCFG;
    //欢乐值配置
    SJoyLimitCFG m_stJoyCFG;
    // boss
    SBossMiscCFG m_stBossCFG;
    // 神器
    SArtifactMiscCFG m_stArtifactCFG;

    SDepositMiscCFG m_stSDepositCFG;
    // 周年庆-好友回归
    SRecallCFG m_stRecallCFG;
    // 婚礼
    SWeddingMiscCFG m_stWeddingMiscCFG;
    // kfc活动
    SKFCActivityCFG m_stKFCActivityCFG;
    // 卡片
    SCardMiscCFG m_stCardMiscCFG;
    // pve 卡牌副本
    SPveMiscCFG m_stPveCFG;
    // buff
    SBuffMiscCFG m_stBuffMiscCFG;
    // mvp 竞争战
    SMvpBattleCFG m_stMvpBattleCFG;
    //仆人
    SServantCFG m_stServantCFG;
    // 多职业
    SProfessionMiscCFG m_stProfessionMiscCFG;
    //存档位
    SRecordCFG m_stRecordCFG;
    // var
    SVarMiscCFG m_stVarMiscCFG;
    // 卡普拉
    SCapraActivityCFG m_stCapraActivityCFG;
    // 副本类型
    std::map<ERaidType, DWORD> mapType2RaidID;
    SAltmanCFG m_stAltmanCFG;
    // 活动
    SActMiscCFG m_stActCFG;
    // 换装舞台
    SDressStageCFG m_stDressStageCFG;
    // 追赶系统 兑换商店
    SExchangeShopCFG m_stExchangeShopCFG;
    // 亡者气息
    SDeadMiscCFG m_stDeadCFG;
    // 组队排位赛
    STeamPwsCFG m_stTeamPwsCFG;
    // 组队排位赛休闲
    STeamPwsCFG m_stRelaxTeamPwsCFG;
};

