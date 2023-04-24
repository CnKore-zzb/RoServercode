/**
 * @file RewardConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-11
 */

#pragma once

#include "xSingleton.h"
#include "ProtoCommon.pb.h"
#include "SceneItem.pb.h"
#include "NpcConfig.h"
#include "ActivityConfig.h"
#include "Var.pb.h"
#include "RecordCmd.pb.h"

using std::pair;
using std::vector;
using std::map;
using std::string;
using std::set;
using namespace Cmd;

const DWORD BASE_RATE = 100000000;
const DWORD REWARD_SAFETY_DEFAULT_EXPIRE = 30;

typedef map<DWORD, RewardSafetyItem> TMapRewardSafetyItem;

class SRewardItem;
class SRewardCFG;

class RewardEntry
{
  public:
    RewardEntry() {}
    RewardEntry(EProfession eProfession, DWORD dwBaseLv, DWORD dwJobLv) { m_eProfession = eProfession; m_dwBaseLv = dwBaseLv; m_dwJobLv = dwJobLv; }

    void set_pro(EProfession eProfesion) { m_eProfession = eProfesion; }
    EProfession pro() const { return m_eProfession; }

    void set_baselv(DWORD lv) { m_dwBaseLv = lv; }
    DWORD baselv() const { return m_dwBaseLv; }

    void set_joblv(DWORD lv) { m_dwJobLv = lv; }
    DWORD joblv() const { return m_dwJobLv; }

    void set_charid(QWORD charid) { m_qwCharID = charid; }

    void set_safetymap(TMapRewardSafetyItem* safety) { m_pmapSafetyItem = safety; }
    RewardSafetyItem* safetyitem(const SRewardCFG* pCFG);
    const SRewardItem* safety(const SRewardCFG* pCFG, bool roll);

  private:
    EProfession m_eProfession = EPROFESSION_MIN;
    DWORD m_dwBaseLv = 0;
    DWORD m_dwJobLv = 0;
    QWORD m_qwCharID = 0;
    TMapRewardSafetyItem* m_pmapSafetyItem;
};

// config data
enum ERewardType
{
  EREWARDTYPE_MIN = 0,
  EREWARDTYPE_ALL = 1,
  EREWARDTYPE_ONE = 2,
  EREWARDTYPE_REWARD = 3,
  EREWARDTYPE_REWARD_ALL = 4,
  EREWARDTYPE_ONE_DYNAMIC = 5,
  EREWARDTYPE_MAX,
};

struct SSingleItem
{
  DWORD dwStack = 0;
  ItemInfo oItem;

  SSingleItem() {}
};
typedef vector<SSingleItem> TVecSingleItem;
struct SRewardItem
{
  DWORD dwRate = 0;
  DWORD dwCount = 0;

  vector<EProfession> vecProfession;
  pair<DWORD, DWORD> pairBaseLvReq;
  pair<DWORD, DWORD> pairJobLvReq;
  TVecSingleItem vecItems;

  bool bIsSafety = false; // 是否是保底奖励

  SRewardItem() {}

  bool isValid(const RewardEntry& rEntry) const;
};
typedef vector<SRewardItem> TVecRewardItem;

// reward保底机制
struct SRewardSafetyCFG
{
  DWORD dwX = 0; // 触发保底roll次数下限
  DWORD dwY = 0; // 触发保底roll次数上限
  DWORD dwLimit = 0; // 保底生效次数上限
  DWORD dwExpire = 0; // 保底记录过期时长
  DWORD dwVersion = 0; // 保底版本号,用于保底配置修改时,当玩家记录的版本与配置的版本不一致时,直接清空玩家配置

  bool isSafety() const { return dwX > 0 && dwY >= dwX; } // 是否有保底机制
  bool isSafetyExpire(DWORD expire) const { return now() >= expire; } // 保底记录过期
  DWORD getNextSafetyCount() const { return randBetween(dwX, dwY); } // roll多少次后保底
  DWORD getExpireTime() const { return now() + dwExpire; }
};

struct SRewardCFG
{
  DWORD id = 0;
  DWORD maxRand = 0;

  ERewardType eType = EREWARDTYPE_MIN;

  bool bType2RatioEnable = false;

  TSetDWORD setValidMap;
  TVecRewardItem vecItems;
  SRewardSafetyCFG oSafety;

  SRewardCFG() {}
};
typedef map<DWORD, SRewardCFG> TMapRewardCFG;

struct SExtraNpcRwd
{
  DWORD uniqueID = 0;
  DWORD exclude = 0;

  set<ENpcZoneType> setZoneTypes;
  set<ENpcType> setNpcTypes;
  set<ERaceType> setRaceTypes;
  set<ENatureType> setNatureTypes;

  TSetDWORD setExtraIDs;
};
typedef map<DWORD, SExtraNpcRwd> TMapExtraRwds;

enum EExtraRewardType
{
  EEXTRAREWARD_MIN = 0,
  EEXTRAREWARD_WANTEDQUEST = 1, // 看板
  EEXTRAREWARD_DAILYMONSTER = 2, // 抗魔
  EEXTRAREWARD_SEAL = 3, // 封印
  EEXTRAREWARD_LABORATORY = 4, // 研究所
  EEXTRAREWARD_ENDLESS = 5, // 无限塔
  EEXTRAREWARD_GUILD_QUEST = 6, //工会任务
  EEXTRAREWARD_GUILD_DONATE = 7,  //工会捐赠
  EEXTRAREWARD_PVECARD = 8, // 卡牌副本
  EEXTRAREWARD_GVG = 9, // gvg
  EEXTRAREWARD_SUPERGVG = 10, // 决战
};

struct SExtraRewardData
{
  EExtraRewardType eType = EEXTRAREWARD_MIN;
  DWORD dwNeedTimes = 0;
  DWORD dwDayRewardTimes = 0;
  TSetDWORD setRewards;
  DWORD dwAccLimit = 0;
  map<DWORD,TSetDWORD> mapParam2Rewards; // ex: 决战, 名词->奖励
};

enum EDoubleRewardType
{
  EDOUBLEREWARD_MIN = 0,
  EDOUBLEREWARD_WANTEDQUEST = 1, // 看板
  EDOUBLEREWARD_DAILYMONSTER = 2, // 抗魔
  EDOUBLEREWARD_SEAL = 3, // 封印
  EDOUBLEREWARD_LABORATORY = 4, // 研究所
  EDOUBLEREWARD_ENDLESS = 5, // 无限塔
  EDOUBLEREWARD_PVECARD = 6, // 卡牌副本
};

struct SDoubleRewardData
{
  EDoubleRewardType eType = EDOUBLEREWARD_MIN;
  DWORD dwNeedTimes = 0;
  DWORD dwDayRewardTimes = 0;
  DWORD dwTimes = 0;
  DWORD dwAccLimit = 0;
};

class OtherFactor
{
public:
  void setEnable(bool bEnable) {m_bEnable = bEnable;}
  bool getEnable()const{return m_bEnable;}
private:
  bool m_bEnable = false;
public:
  //卡片Buff相关 
  bool hasCardBuff() const {return m_bHasCardBuff;}
  void setCardBuffRatio(float ratio) 
  {
    m_bEnable = true;
    m_bHasCardBuff = true;
    m_fCardRatio = ratio;
  } 
  void clearCardBuff()
  {
    m_bHasCardBuff = false;
    m_fCardRatio = 0.0f;
  }
  float getCardBuffRatio() const
  {
    return m_bHasCardBuff ? m_fCardRatio : 0.0f;
  }

  //物品种类Buff相关 
  void setKindRatio(float ratio)
  {
    m_bEnable = true;
    m_fKindRatio = ratio;
  }
  float getKindRatio() const 
  {
    return m_fKindRatio;
  }
private:
  bool m_bHasCardBuff = false;
  float m_fCardRatio  = 0.0f;
  float m_fKindRatio = 0.0f;
};

typedef map<EExtraRewardType, SExtraRewardData> TMapExtraReward;
typedef map<EDoubleRewardType, SDoubleRewardData> TMapDoubleReward;

// config
class RewardConfig : public xSingleton<RewardConfig>
{
  friend class xSingleton<RewardConfig>;
  private:
    RewardConfig();
  public:
    virtual ~RewardConfig();

    bool loadConfig();
    bool checkConfig();

    const SRewardCFG* getRewardCFG(DWORD dwID) const;
    bool roll(DWORD id, const RewardEntry& rEntry, TVecItemInfo& vecItemInfo, ESource source, float fDropRatio = 1.0f, DWORD dwMapID = 0);
    /**
     * 暂时仅用于宠物冒险,逻辑与roll基本一致
     */
    bool roll_adventure(DWORD id, const RewardEntry& rEntry, TVecItemInfo& vecItemInfo, ESource source, const OtherFactor& oFactor, float fDropRatio = 1.0f, DWORD dwMapID = 0);

    bool addReplace(DWORD dwOldID, DWORD dwNewID);
    bool delReplace(DWORD dwID);

    bool addExtraRwd(const SExtraNpcRwd& stExtraRwd);
    bool delExtraRwd(const SExtraNpcRwd& stExtraRwd);

    bool addExtraRwd(DWORD dwNpcID, const TSetDWORD& setRwds);
    bool delExtraRwd(DWORD dwNpcID, const TSetDWORD& setRwds);

    bool getNpcExtraRwd(const SNpcCFG* pCFG, TSetDWORD& setRwds, bool superAiNpc) const;

    bool addExtraRwd(EExtraRewardType eType, const SExtraRewardData& rwdData);
    bool delExtraRwd(EExtraRewardType eType);
    bool getExtraByType(EExtraRewardType eType, DWORD donwcount, DWORD nowtimes, TSetDWORD& setRwds, DWORD param = 0);
    bool hasExtraReward(EExtraRewardType eType) const { return m_mapExtraReward.find(eType) != m_mapExtraReward.end(); }
    bool getExtraRwdAccLimit(EExtraRewardType eType);
    EVarType getVarByExtra(EExtraRewardType eType);
    EAccVarType getAccVarByExtra(EExtraRewardType eType);

    bool addDoubleRwd(EDoubleRewardType eType, const SDoubleRewardData& rwdData);
    bool delDoubleRwd(EDoubleRewardType eType);
    bool hasDoubleReward(EDoubleRewardType eType) const { return m_mapDoubleReward.find(eType) != m_mapDoubleReward.end(); }
    DWORD getDoubleRewardTimesByType(EDoubleRewardType eType, DWORD donwcount, DWORD nowtimes);
    bool getDoubleRwdAccLimit(EDoubleRewardType eType);
    EVarType getVarByDouble(EDoubleRewardType eType);
    EAccVarType getAccVarByDouble(EDoubleRewardType eType);
  private:
    bool loadRewardConfig();

    /**
     * 暂时仅用于宠物冒险,逻辑与roll_all基本一致
     */
    bool roll_all_adventure(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio, const OtherFactor& oFactor);
    bool roll_all(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio);
    bool roll_one(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio);
    bool roll_reward(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio);
    bool roll_reward_all(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio);
    bool roll_one_dynamic(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio);
    /**
     * 检查奖励是否有卡片Buff影响
     */  
    bool checkCardBuffConfition(const SRewardItem& rItem, const OtherFactor& factor);
  private:
    TMapRewardCFG m_mapRewardCFG;
    map<DWORD, DWORD> m_mapReplacePairs;
    map<DWORD, TSetDWORD> m_mapNpcID2ExtraRwds;
    TMapExtraRwds m_mapExtraRwds;
    TMapExtraReward m_mapExtraReward;
    TMapDoubleReward m_mapDoubleReward;
};

