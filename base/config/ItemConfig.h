/**
 * @file ItemConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-11
 */

#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"
#include "SceneItem.pb.h"
#include "SceneManual.pb.h"
#include "UserEvent.pb.h"
#include "TableManager.h"

using namespace Cmd;
using std::set;
using std::map;
using std::pair;
using std::vector;
using std::string;
using std::make_pair;

#define DB_TABLE_TRADE_INFO     "trade_item_info"
#define DB_TABLE_PENDING_LIST   "trade_pending_list"
#define DB_TABLE_PRICE_ADJUST   "trade_price_adjust_log"
#define DB_TABLE_PUBLICITY      "trade_publicity"
#define DB_TABLE_PUBLICITY_BUY  "trade_publicity_buy"

#define DB_TABLE_BOOTH_SELLED_LIST "booth_record_sold"
#define DB_TABLE_BOOTH_BUYED_LIST "booth_record_bought"


#define LOTTERY_PER_MAX 1000000

#ifdef _OLD_TRADE

#define DB_TABLE_SELLED_LIST    "trade_saled_list"
#define DB_TABLE_BUYED_LIST     "trade_buyed_list"

#else 

#define DB_TABLE_SELLED_LIST    "trade_record_sold"
#define DB_TABLE_BUYED_LIST     "trade_record_bought"
#define DB_TABLE_GIFT           "trade_gift"
#define DB_TABLE_GOODS          "trade_goods"

#endif // _OLD_TRADE

const DWORD ITEM_ZENY = 100;
const DWORD ITEM_GOLD = 105;
const DWORD ITEM_GARDEN = 110;
const DWORD ITEM_CONTRIBUTE = 140;
const DWORD ITEM_ASSET = 146;
const DWORD ITEM_FRIENDSHIP = 147;
const DWORD ITEM_BCOIN = 151;
const DWORD ITEM_HONOR = 156;
const DWORD ITEM_DEAD = 169;
const DWORD ITEM_APOLOGY = 6000;
const DWORD ITEM_BASEEXP = 300;
const DWORD ITEM_MEDAL = 5261;
const DWORD ITEM_QUEST_MANUAL = 5400;
const DWORD ITEM_PRAY_ATKCARD = 5530;
const DWORD ITEM_PRAY_DEFCARD = 5531;
const DWORD ITEM_PRAY_ELEMCARD = 5532;
const DWORD ITEM_PET_WORK = 5542;
const DWORD ITEM_JOBEXP = 400;
const DWORD ITEM_QUEST_PACK = 5045;
const DWORD ITEM_FOOD_PACK = 5047;
const DWORD ITEM_PET_PACK = 5640;
const DWORD ITEM_WEDDING_CERT = 5802;
const DWORD ITEM_ENCHANT_TRANS = 12617;
const DWORD ITEM_FOODBOOK_LV_1 = 550002;
const DWORD ITEM_FOODBOOK_LV_2 = 550003;
const DWORD ITEM_FOODBOOK_LV_3 = 550004;
const DWORD ITEM_FOODBOOK_LV_4 = 550005;
const DWORD ITEM_FOODBOOK_LV_5 = 550006;
const DWORD ITEM_FOODBOOK_LV_6 = 550007;
const DWORD ITEM_FOODBOOK_LV_7 = 550008;
const DWORD ITEM_FOODBOOK_LV_8 = 550009;
const DWORD ITEM_FOODBOOK_LV_9 = 550010;
const DWORD ITEM_FOODBOOK_LV_10 = 550011;

const DWORD PACK_QUEST_DEFAULT_MAXSLOT = 600;
const DWORD PACK_FOOD_DEFAULT_MAXSLOT = 600;
const DWORD QUICK_SELL_REFINE_LESS = 5;
const DWORD PACK_PET_DEFAULT_MAXSLOT = 600;

// exchange item config
enum EPriceType
{
  PRICETYPE_SELF = 1,
  PRICETYPE_SUM = 2,
};

enum EMinPriceType
{
  MINPRICETYPE_SELF = 1,              //直接取price
  MINPRICETYPE_CARD_COMPOSE = 2,      //图纸合成   
  MINPRICETYPE_EQUIP_COMPOSE = 3,     //装备制作
  MINPRICETYPE_EQUIP_UPGRADE = 4,     //装备升级
};

enum EForbid
{
  EFORBID_ENCHANT = 1,
  EFORBID_STRENGTH = 1 << 1,
  EFORBID_REFINE = 1 << 2,
  EFORBID_UNLOCKPET = 1 << 3, // 存入冒险手册不解锁宠物avatar
};

enum ENoStore
{
  ENOSTORE_STORE = 1,
  ENOSTORE_PSTORE_BARROW = 2,
};

struct SMinPrice
{
  EMinPriceType type;
  DWORD dwPrice = 0;
  DWORD dwComposeId = 0;
  DWORD dwEquipUpgrade = 0;
  //std::vector < std::pair<DWORD/*itemid*/, DWORD/*count*/>> vecPriceSum;
  TVecItemInfo vecMaterial;
  DWORD dwRob = 0;
  DWORD dwEquipId = 0;
  float ratio = 0;
};

enum EAuctionSignupType
{
  EAUCTIONSIGNUPTYPE_NONE = 0,    // 不能上架
  EAUCTIONSIGNUPTYPE_ITEM = 1,    // 物品上架
  EAUCTIONSIGNUPTYPE_EQUIP = 2,   // 装备上架
};

struct SExchangeItemCFG
{
  DWORD dwId = 0;
  std::string strName;
  DWORD dwMoneyType = 0;
  EPriceType dwPriceType;
  DWORD dwPrice = 0;
  std::vector < std::pair<DWORD/*itemid*/, DWORD/*count*/>> vecPriceSum;
  DWORD dwDealNum = 0;
  float fInRatio = 0;
  float fOutRatio = 0;
  DWORD dwBoothfee = 0;
  bool isOverlap = false;
  DWORD dwCategory = 0;
  DWORD dwBigCategory = 0;
  DWORD dwShowOrdcer = 0;
  //std::string strJob;
  TVecDWORD vecJobs;
  DWORD dwFrozenTime;
  DWORD dwFashionType;
  SMinPrice minPrice;
  bool bTrade;          //可否交易
  DWORD dwShowTime;     //秒
  DWORD dwExchangeNum;  //公示期达到的数量

  std::map<DWORD/*lv*/, TVecItemInfo> mapUpgradePriceSum; //装备升级后的价格材料 lv 1 2 3 4
  DWORD dwMaxUpgradeLv = 0;   //4
  DWORD dwRefineCycle = 0;    //精炼调价周期 0：不调整  1： 周周期 2：月周期
  DWORD dwTradeTime = 0;      //可以上交易所的时间
  EAuctionSignupType eAuctionSignupType = EAUCTIONSIGNUPTYPE_NONE;  // 拍卖行上架类型

  bool isJob(DWORD job) 
  {
    if (vecJobs.empty())
      return true;
    auto it = std::find_if(vecJobs.begin(), vecJobs.end(), [job](DWORD a) { return  a == job; });
    if (it == vecJobs.end())
      return false;
    return true;
  }
  bool canTrade()const {
    return now() >= dwTradeTime ? true : false;
  }
};
typedef map<DWORD, SExchangeItemCFG> TMapExchangeItemCFG;

struct SEquipUpgradeCFG
{
  DWORD dwLv = 0;
  DWORD dwBuffID = 0;
  DWORD dwNpcID = 0;
  DWORD dwEvoReq = 0;

  ItemInfo oProduct;
  TVecItemInfo vecMaterial;
};
typedef map<DWORD, SEquipUpgradeCFG> TMapEquipUpgradeCFG;

struct SEquipDecomposeCFG : public SBaseCFG
{
  DWORD dwID = 0;
  DWORD dwCost = 0;
  TVecItemInfo vecMaterial;
};
typedef map<DWORD, SEquipDecomposeCFG> TMapEquipDecomposeCFG;
typedef map<DWORD, TSetDWORD> TMapDecomposeEquipCFG;

struct SEquipSuitRefineAttrCFG
{
  DWORD dwLevel = 0;
  float fBase = 0;
  float fMax = 0;
};
typedef map<DWORD, SEquipSuitRefineAttrCFG> TMapEquipSuitRefineAttrCFG;

enum EItemGetLimitType
{
  EITEMGETLIMITTYPE_NONE = 0,
  EITEMGETLIMITTYPE_DAY = 1,
  EITEMGETLIMITTYPE_WEEK = 7,
};

enum ECardFunc
{
  ECARDFUNC_NOMATERIAL = 4,
  ECARDFUNC_NODECOMPOSE = 8,    // 不可分解
};

struct LevelData
{
  std::pair<DWORD/*fromlv*/, DWORD/*tolv*/> prLvRange;    //[fromlv, tolv]
  TVecAttrSvrs vecEquipBase;
};

typedef std::vector<LevelData> TVecLevelData;


enum EUseLimitType
{
  EUSELIMITTYPE_GVG = 1,
  EUSELIMITTYPE_ONLYGUILD = 2, // 只能在公会领地使用
  EUSELIMITTYPE_PVPGVG = 4, // PVP/GVG地图使用
  EUSELIMITTYPE_NOTPVECARD = 8, // pve地图不可以使用
  EUSELIMITTYPE_NOTMVP = 16,  // 不能在MVP副本使用
  EUSELIMITTYPE_TEAMMATES = 32, // 必须有队友同在PVP或者GVG
  EUSELIMITTYPE_NOTEAMPWS = 64, // 不能在组队竞技中使用
};

// item config

enum EFeature
{
  EFeature_Give = 2,      //能不能赠送
  EFeature_Sell = 4,
  EFeature_Use = 8,
};

enum EItemDelType
{
  EITEMDELTYPE_MIN = 0,
  EITEMDELTYPE_TIME = 1,        // 获取道具n秒后删除
  EITEMDELTYPE_DATE = 2,        // 指定日期删除
  EITEMDELTYPE_MAX = 3,
};

struct SItemCFG
{
  friend class ItemConfig;
  DWORD dwTypeID = 0;
  DWORD dwMaxNum = 0;
  DWORD dwSellPrice = 0;
  DWORD dwComposeID = 0;
  DWORD dwRefineComposeID = 0;
  DWORD dwLevel = 0;
  DWORD dwCD = 0;
  DWORD dwPVPCD =0;
  DWORD dwCDGroup = 0;
  DWORD dwCardSlot = 0;
  DWORD dwCardPosition = 0;
  DWORD dwCardType = 0;
  DWORD dwItemSort = 0;
  DWORD dwInHandLmt = 0;
  DWORD dwHandInLmt = 0;
  DWORD dwUsedSys = 0;
  DWORD dwTargeUsedSys = 0;
  DWORD dwFailSys = 0;
  DWORD dwTransformLmt = 0;
  DWORD dwDaliyLimit = 0;
  DWORD dwWeekLimit = 0;
  DWORD dwMaxRefineLv = 0;
  DWORD dwUseLimit = 0;
  DWORD dwUseStartTime = 0;
  DWORD dwUseEndTime = 0;

  DWORD dwSubID = 0;
  DWORD dwUpgID = 0;
  DWORD dwVID = 0;
  DWORD dwDecomposeID = 0;
  DWORD dwDecomposeNum = 0;
  DWORD dwForbid = 0;
  DWORD dwCardWeight = 0;
  DWORD dwCardFunc = 0;
  DWORD dwGroupID = 0;

  DWORD dwHairID = 0;

  DWORD dwNoStoreage = 0;

  bool bNoSale = false;

  string strNameZh;
  SDWORD swAdventureValue = 0;

  DWORD dwPostId = 0;
  ETitleType eTitleType = ETITLE_TYPE_MIN;
  bool bFirst = false;

  EItemType eItemType = EITEMTYPE_MIN;
  EEquipType eEquipType = EEQUIPTYPE_MIN;
  EQualityType eQualityType = EQUALITYTYPE_MIN;
  //EManualLockMethod eLockType = EMANUALLOCKMETHOD_MIN;
  DWORD eLockType = 0;
  TVecDWORD vecLockType;

  DWORD dwCardComposeStartTime = 0;
  DWORD dwCardComposeEndTime = 0;
  DWORD dwCardLotteryStartTime = 0;
  DWORD dwCardLotteryEndTime = 0;

  ETragetType eTargetType = ETARGETTYPE_MY;
  DWORD dwRange = 0;
  vector<EProfession> vecEquipPro;
  EGender eSexEquip = EGENDER_MIN;

  TVecAttrSvrs vecEquipBase;
  TVecAttrSvrs vecEquipStrength;
  TVecAttrSvrs vecEquipRefine;
  TVecAttrSvrs vecCardBase;
  TVecAttrSvrs vecTitleBase;
  TVecAttrSvrs vecPVPEquipBase;
  TVecLevelData vecLevelData;

  TVecDWORD vecBuffIDs;
  TVecDWORD vecAdvBuffIDs;
  TSetDWORD setAdvRewardIDs;
  TVecDWORD vecStoreBuffIDs;
  TVecDWORD vecPVPBuffIDs;
  TSetDWORD setFashionBuffIDs;

  TVecItemInfo vecManualItems;

  std::vector<pair<DWORD, TVecDWORD>> vecRefine2Buff;

  TMapEquipUpgradeCFG mapUpgradeCFG;

  TVecDWORD vecSuitRefineAttrIDs;
  TMapEquipSuitRefineAttrCFG mapSuitRefineAttrs;

  EItemGetLimitType eGetLimitType = EITEMGETLIMITTYPE_NONE;
  std::vector<ESource> vecLimitSource;
  map<DWORD, pair<DWORD, DWORD>> mapLevel2GetLimit;
  bool bUniformSource = false;

  vector<EProfession> vecProLimit;

  xLuaData oGMData;
  xLuaData oPickGMData;
  bool bLottery = false;    //扭蛋装备
private:
  DWORD dwUseMultiple = 0;  //一次可使用多个
public:
  DWORD dwBody = 0;

  EItemDelType eDelType = EITEMDELTYPE_MIN;
  DWORD dwDelTime = 0;
  DWORD dwDelDate = 0;

  DWORD dwFeature = 0;

  //检查这个道具是否适合这个职业
  bool isRightJob(DWORD dwJob) const
  {
    if (vecEquipPro.empty())   
      return true;
    auto it = std::find_if(vecEquipPro.begin(), vecEquipPro.end(), [dwJob](EProfession a) { return  a == dwJob; });
    if (it == vecEquipPro.end())
      return false;
    return true;
  }

  //检查这个职业能否使用这个道具
  bool isRightJobForUse(DWORD dwJob) const
  {
    if (vecProLimit.empty())
      return true;
    auto it = std::find_if(vecProLimit.begin(), vecProLimit.end(), [dwJob](EProfession a) { return  a == dwJob; });
    if (it == vecProLimit.end())
      return false;
    return true;
  }

  bool isRepairId(DWORD vid) const 
  {
    if (vid == 0)
      return false;
    if ((dwVID / 10000) != (vid / 10000))
      return false;
    return  ((dwVID % 1000) == (vid % 1000));
  }

  bool isForbid(EForbid eType) const
  {
    return (dwForbid & eType) != 0;
  }
  bool isNoStore(ENoStore eType) const { return (dwNoStoreage & eType) != 0; }

  bool isLotteryEquip() const { return bLottery; }
  
  bool hasCardFunc(ECardFunc eFunc) const { return (dwCardFunc & eFunc) != 0; }

  const SEquipUpgradeCFG* getUpgradeCFG(DWORD dwLevel) const;
  void calcLockType();

  DWORD getItemGetLimit(DWORD lv, ESource source, bool bMonthCard = false) const;
  const LevelData* getLevelData(DWORD level)  const
  {
    for (auto it = vecLevelData.begin(); it != vecLevelData.end(); ++it)
      if (level >= it->prLvRange.first && level <= it->prLvRange.second)
        return &(*it);
    return nullptr;
  }
  DWORD getUseMultiple() const;
  bool isTimeValid() const;
  bool isCardCompoaseTimeValid() const;

  bool hasFeatrue(EFeature f) const
  {
    return dwFeature & f;
  }

  SItemCFG() {}
};
typedef map<DWORD, SItemCFG> TMapItemCFG;

// master config
enum EMasterType
{
  EMASTERTYPE_MIN = 0,
  EMASTERTYPE_STRENGTH,
  EMASTERTYPE_REFINE,
  EMASTERTYPE_MAX
};
struct SMasterCFG
{
  DWORD dwLv = 0;

  EMasterType eType = EMASTERTYPE_MIN;

  TVecAttrSvrs vecAttrs;

  SMasterCFG() {}
};
typedef vector<SMasterCFG> TVecMasterCFG;

// suit config
typedef pair<DWORD, TVecDWORD> TPairCountBuff;
typedef vector<TPairCountBuff> TVecSuitBuff;

struct SEquipRefineBuff {
  DWORD dwLevel = 0;
  TVecDWORD vecBuffs;
};
typedef map<DWORD, SEquipRefineBuff> TMapEquipRefineBuff;

struct SSuitCFG
{
  DWORD id = 0;
  TVecDWORD vecEquipIDs;

  TVecSuitBuff vecBuffs;

  DWORD dwRefineLevel = 0;
  TVecDWORD vecRefineBuffs;
  TMapEquipRefineBuff mapEquipRefineBuffs;

  SSuitCFG() {}

  void getBuffs(DWORD count, TVecDWORD& vecIDs) const;
  void getRefineBuffs(TVecDWORD& buffs) const;
  void getRefineEquipBuffs(DWORD id, TVecDWORD& buffs) const;
  void getRefineAllBuffs(TVecDWORD& buffs) const;
  DWORD getEquipRefineLv(DWORD equipid) const;
};
typedef map<DWORD, SSuitCFG> TMapSuitCFG;
typedef map<DWORD, TSetDWORD> TMapEquipSuit;

// refine config
struct SRefineRateCFG
{
  EQualityType eQuaType;

  TVecDWORD vecCompose;
  DWORD dwSuccessRate = 0;
  DWORD dwFailStayRate = 0;
  DWORD dwFailNoDamageRate = 0;

  DWORD dwReturnZeny = 0;
  TVecDWORD vecSafeCompose;
};

struct SRefineCFG
{
  EItemType eItemType = EITEMTYPE_MIN;
  DWORD dwRefineLv = 0;

  SRefineRateCFG sComposeRate;

  SRefineCFG() {}
};

typedef vector<SRefineCFG> TVecRefineCFG;

struct SRefineTradeCFG
{
  EItemType eItemType = EITEMTYPE_MIN;
  DWORD dwRefineLv = 0;

  DWORD dwEquipRate = 0;
  DWORD dwEquipRateNew = 0;
  DWORD dwItemID = 0;
  DWORD dwItemRate = 0;
  DWORD dwItemRateNew = 0;
  DWORD dwLvRate = 0;
  DWORD dwLvRateNew = 0;
  float fRiskRate = 0.0;
  SRefineTradeCFG() {}
};
typedef vector<SRefineTradeCFG> TVecRefineTradeCFG;

// fashion config
struct SFashionCFG
{
  DWORD id = 0;
  DWORD num = 0;
  TVecAttrSvrs uAttrs;
};
typedef vector<SFashionCFG> TVecFashionCFG;

// card config
typedef pair<DWORD, DWORD> TPairMonsterCard;
typedef vector<TPairMonsterCard> TVecMonsterCard;
struct SQualityCard
{
  DWORD dwMaxWeight = 0;
  vector<SItemCFG> vecCardCFG;
};
typedef map<EQualityType, SQualityCard> TMapQualityCard;

// item no source config
typedef set<DWORD> TSetItemNoSource;
typedef set<EItemType> TSetItemType;

// item error
struct SItemErrorCFG
{
  EItemType eType = EITEMTYPE_MIN;

  DWORD dwLvErrMsg = 0;
  string name;
  DWORD dwUseNumber = 0;
  SItemErrorCFG() {}
};
typedef map<EItemType, SItemErrorCFG> TMapItemErrorCFG;

// enchant config
enum EEnchantExtraCon
{
  EENCHANTEXTRACON_MIN = 0,
  EENCHANTEXTRACON_REFINELV = 1,
  EENCHANTEXTRACON_MAX = 2,
};
struct SEnchantAttrItem
{
  float fMin = 0.0f;
  float fMax = 0.0f;
  DWORD dwWeight = 0;         //累加的值
  DWORD dwRawWeight = 0;      //策划表原始值

  SEnchantAttrItem() {}
};
typedef vector<SEnchantAttrItem> TVecEnchantAttrItem;
struct SEnchantAttr
{
  DWORD dwConfigID = 0;

  EAttrType eType = EATTRTYPE_MIN;
  vector<EAttrType> vecPairType;

  DWORD dwMaxWeight = 0;

  vector<pair<DWORD, DWORD>> vecExtraBuff;
  DWORD dwExtraMaxWeigth = 0;

  float fMin = 0.0f;
  float fMax = 0.0f;
  float fExpressionOfMaxUp = 0.0f;
  TVecDWORD vecExtraCondition;
  TVecEnchantAttrItem vecItems;
  std::set<EItemType> setNoBuffid;
  std::set<EItemType> setValidItemType;

  SEnchantAttr() {}

  float random() const;
  DWORD randomExtraBuff() const;
  float getRandomRate() const;
  bool canGetExtraBuff(EItemType eType) const;
  bool checkBuffId(DWORD buffId) const;
};
typedef vector<SEnchantAttr> TVecEnchantAttr;

struct SEnchantEquipItem
{
  pair<EAttrType, DWORD> p;
  SEnchantEquipItem() {}
};
typedef vector<SEnchantEquipItem> TVecEnchantEquipItem;
struct SEnchantEquip
{
  EItemType eItemType = EITEMTYPE_MIN;
  TVecEnchantEquipItem vecItems;
  DWORD dwMaxWeight = 0;
  SEnchantEquip() {}

  bool random(DWORD dwNum, vector<EAttrType>& vecAttrs) const;
  const SEnchantEquipItem* getEnchantEquipItem(EAttrType type) const;
};
typedef vector<SEnchantEquip> TVecEnchantEquip;

struct SEnchantCFG
{
  EEnchantType eType = EENCHANTTYPE_MIN;
  DWORD dwMaxNum = 0;

  ItemInfo oNeedItem;
  DWORD dwRob = 0;

  TVecEnchantAttr vecAttrs;
  TVecEnchantEquip vecEquips;

  std::map<DWORD/*buffid*/,std::vector<EItemType>> mapNoGoodEnchant;      //不算极品附魔的装备

  SEnchantCFG() {}
  const SEnchantAttr* getEnchantAttr(EAttrType eType, EItemType eItem) const;
  const SEnchantAttr* getEnchantAttr(DWORD dwConfigID) const;
  const SEnchantEquip* getEnchantEquip(EItemType eType) const;
  bool random(EItemType eItemType, EnchantData& rData) const;
  //特殊处理的附魔
  bool specialRandom(EItemType eItemType, EnchantData& rData) const;
  bool contain(const vector<EAttrType>& vecAttrs, const vector<EAttrType>& vecPairs, EAttrType eExclude) const;
  //判断某个有buffid的装备是否算是极品附魔
  bool isGoodEnchant(DWORD buffId, EItemType eType) const;
  bool gmCheckAndSet(EItemType eItemType, EnchantData& rData, DWORD buffId) const;

  // 判断某个属性是否为极品属性
  bool isGoodEnchant(const Cmd::EnchantAttr& attr, EItemType eType, DWORD& dwPointOut) const;
};
typedef map<DWORD, SEnchantCFG> TMapEnchantCFG;

struct SEnchantPriceCFG
{
  std::map<EItemType, float> mapValue;
};
typedef map<DWORD, SEnchantPriceCFG> TMapEnchantPriceCFG;

// cd config
typedef map<DWORD, TSetDWORD> TMapCDGroupCFG;

// card rate config
struct SCardRateCFG : public SBaseCFG
{
  vector<EQualityType> vecQuality;

  DWORD dwWriteRate = 0;
  DWORD dwGreenRate = 0;
  DWORD dwBlueRate = 0;
  DWORD dwPurpleRate = 0;

  DWORD dwZeny = 0;

  TVecItemInfo vecReward;
};
typedef vector<SCardRateCFG> TVecCardRateCFG;

struct SLotteryItemCFG
{
  friend class ItemConfig;
  DWORD dwId = 0;
  DWORD eType = 0;
  DWORD dwCount = 0;
  DWORD dwYear = 0;
  DWORD dwMonth = 0;
  DWORD dwWeight = 0;
  DWORD dwWeightOffSet = 0;
  DWORD dwHideWeight = 0;
  DWORD dwHideWeightOffSet = 0;
  DWORD dwRecoveryItemId = 0;     //回收获得物品
  DWORD dwRecoveryCount = 0;      //回收获得个数
  DWORD dwRate = 0;       // weight/totalweight * 10000;
  string strRarity;
  DWORD dwBatch = 0;
  DWORD dwItemType = 0;
  DWORD dwFemaleItemId = 0;
  DWORD dwBasePrice = 0; // 稀有物品基础保底价格
  DWORD dwCardType = 0;
  static DWORD getKey(DWORD year, DWORD month, DWORD itemType)
  {
    return year * 1000 + month *10 + itemType;
  }

  bool isCurBatch() const;
  DWORD getRate() const;
  //往期类别 = 100 + 当期类别
  DWORD convert2Old()const {
    return 100 + dwItemType;
  }
  DWORD getItemId() const
  {
    return dwItemId;
  }
  DWORD getItemIdByGender(EGender gender) const
  {
    if (dwFemaleItemId == 0) return dwItemId;
    return (gender == EGENDER_MALE ? dwItemId : dwFemaleItemId);
  }
  bool operator ==(const SLotteryItemCFG& cfg)
  {
    return dwId == cfg.dwId;
  }

private:
  DWORD dwItemId = 0;
};

typedef std::map<DWORD/*id*/, SLotteryItemCFG> TMapLotteryItemCFG;
class SLotteryCFG
{
  public:
    DWORD eType = 0;
    DWORD dwYear = 0;
    DWORD dwMonth = 0;
    DWORD dwTotalWeight = 0;
    DWORD dwTotalHideWeight = 0;
    TMapLotteryItemCFG mapItemCFG;

    bool addItemCFG(const SLotteryItemCFG& rItemCFG);
    bool random(DWORD& outItemId, DWORD &count, EGender gender, DWORD &rate) const;
    bool randomWithHideWeight(DWORD& outItemId, DWORD &count, EGender gender, DWORD &rate) const;
    DWORD needMoney() const;
    DWORD getDisCount() const;
    bool isCurMonth() const;
    const SLotteryItemCFG* getItemCFG(const string& strRarity) const
    {
      for (auto m = mapItemCFG.begin(); m != mapItemCFG.end(); ++m)
      {
        if (m->second.strRarity == strRarity)
          return &m->second;
      }
      return nullptr;
    }
};

struct SLotteryGiveCFG
{
  DWORD dwYear = 0;
  DWORD dwMonth = 0;
  DWORD dwLotteryBoxItemId = 0;
  DWORD dwLotteryItemId = 0;
};

typedef std::map<DWORD/*year*1000+month*10+itemtype*/, SLotteryCFG> TMapLotteryCFG;
typedef std::map<DWORD/*type*/, TMapLotteryCFG> TMapType2LotteryCFG;

typedef map<DWORD, vector<pair<DWORD, DWORD>>> TMapBaseVID2Equips; //basevid -> [(vid1, equip1), (vid2, equip2)...]

enum SHeadCondType
{
  SHEADCONDTYPE_MIN = 0,
  SHEADCONDTYPE_ACTION = 1,
  SEFFECTCONDTYPE_MAX,
};
struct SHeadEffectCond
{
  SHeadCondType condType = SHEADCONDTYPE_MIN;

  TVecDWORD vecParams;
};

struct SHeadEffect
{
  DWORD dwId = 0;     //itemid
  SHeadEffectCond oCond;
  xLuaData oGMData;

  bool checkCond(SHeadCondType condType,const TVecQWORD& vecParam = TVecQWORD{}) const;
};

typedef std::map<DWORD/*itemid*/, SHeadEffect> TMapId2HeadEffect;

// pack slot
struct SPackSlotLvCFG : public SBaseCFG
{
  DWORD dwBaseLv = 0;

  DWORD dwMainSlot = 0;
  DWORD dwPStoreSlot = 0;
};
typedef map<DWORD, SPackSlotLvCFG> TMapPackSlotLvCFG;
typedef vector<pair<DWORD, DWORD>> TVecGenderReward;

// 保底池
// 根据策划要求type、year、month组合对应保底池
// key=type*1000000+year*100+month
class LotteryPool
{
  public:
    void add(SLotteryItemCFG& cfg);
    bool get(DWORD& dwOutItem, DWORD& dwOutCount, DWORD& dwOutRate, EGender gender, DWORD dwCost, TVecDWORD& vecItemDels);

    static DWORD getKey(DWORD dwTypeId, DWORD dwYear, DWORD dwMonth)
    {
      return dwTypeId*1000000 + dwYear*100 + dwMonth;
    }

  private:
    std::vector<SLotteryItemCFG> m_vecLotteryItemCFG;
};
typedef map<DWORD, LotteryPool> TMapTypeLotteryPool; //<key, LotteryPool>

typedef set<EEquipPos> TSetEquipPos;
// equip gender
typedef map<DWORD, DWORD> TMapEquipGender;
typedef map<EQualityType, TSetDWORD> TMapPetWear;

// equip compose
struct SEquipComposeCFG : public SBaseCFG
{
  DWORD dwID = 0;
  DWORD dwZenyCost = 0;

  TVecItemData vecMaterialEquip;
  TVecItemInfo vecMaterialItem;
};
typedef map<DWORD, SEquipComposeCFG> TMapEquipComposeCFG;

// config
class ItemConfig : public xSingleton<ItemConfig>
{
  friend class xSingleton<ItemConfig>;
  private:
    ItemConfig();
  public:
    virtual ~ItemConfig();

    bool loadConfig();
    bool checkConfig();
    bool canEnterManual(const SItemCFG* pCFG) const;

    template<class T> void foreach(T func) { for_each(m_mapPetWearIDs.begin(), m_mapPetWearIDs.end(), [func](const TMapPetWear::value_type& r) {func(r.first, r.second);}); }

    //const TSetItemNoSource& getItemNoSourceList() const { return m_setItemNoSource; }
    const SItemCFG* getItemCFG(DWORD dwTypeID) const;
    const SItemCFG* getHairCFG(DWORD dwHairID) const;
    const SMasterCFG* getMasterCFG(EMasterType eType, DWORD lv) const;
    const SSuitCFG* getSuitCFG(DWORD id) const;
    const SRefineCFG* getRefineCFG(EItemType eType, EQualityType eQuality, DWORD lv) const;
    const SRefineTradeCFG* getRefineTradeCFG(EItemType eType, DWORD lv) const;
    const SItemErrorCFG* getItemErrorCFG(EItemType eType) const;
    const SEnchantCFG* getEnchantCFG(EEnchantType eType) const;
    const TSetDWORD& getCDGroup(DWORD dwGroup) const;
    const SExchangeItemCFG* getExchangeItemCFG(DWORD id) const;
    const SEquipDecomposeCFG* getEquipDecomposeCFG(DWORD dwID) const;
    const SItemCFG* randCardByQuality(EQualityType eType) const;
    const SCardRateCFG* getCardRateCFG(const vector<EQualityType>& vecQuality) const;
    const SQualityCard* getCardQualityCFG(EQualityType eType) const;
    const TVecGenderReward& getGenderRewardList() const { return m_vecGenderReward; }
    ////获取交易所物品初始服务器价格
    //DWORD getExchangePrice(DWORD id, DWORD refineLv) const;
    const TMapExchangeItemCFG& getExchangeItemCFGMap() const { return m_mapExchangeItemCFG; }
    TVecDWORD getExchangeItemIds(DWORD category, DWORD job, DWORD fashionType);

    const TSetDWORD& getSuitIDs(DWORD equipTypeID) const;
    DWORD getCardIDByMonsterID(DWORD monsterid) const;
    float getEnchantPriceRate(DWORD attr, DWORD itemId) const;
    DWORD getPackSlotLvCFG(DWORD dwLv, EPackType eType) const;

    // 2016-04-19 申林移除Table_EquipFashion.txt表
    //void getFashionAttr(DWORD num, TVecAttrSvrs& vecAttrs);
    bool isFashion(EItemType eType) const;
    bool isEquip(EItemType eType) const;
    bool isCard(EItemType eType) const;
    bool isUseItem(EItemType eType) const;
    bool isShowItem(EItemType eType) const;
    bool isUseNoConsumeItem(EItemType eType) const;
    bool isPackCheck(EItemType eType) const;
    bool isQuest(EItemType eType) const;
    bool isGuild(EItemType eType) const;
    bool isItemSkill(DWORD skillid) const { return m_setItemSkills.find(skillid) != m_setItemSkills.end(); } 
    bool isPetWearItem(DWORD dwID) const;
    //附魔是否是赞
    bool isGoodEnchant(const ItemData& rData);
    bool isRealAdd(ESource eSource) const;
    bool isArtifact(EItemType eType) const;
    bool isSameItemType(EItemType l, EItemType r) const;
    // 获取保底池物品
    bool getItemFromLotteryPool(DWORD& dwOutItem, DWORD& dwOutCount, DWORD& dwOutRate, EGender gender, DWORD key, DWORD dwCost, TVecDWORD& vecItemDels);
    // 判断是否为稀有物品
    bool isRareItem(DWORD dwItemId) { return m_mapItem2BasePrice.end() != m_mapItem2BasePrice.find(dwItemId); }
    bool isArtifactPos(EEquipPos pos) const;
    bool isAccPack(EPackType eType) const { return eType == EPACKTYPE_STORE || eType == EPACKTYPE_FOOD || eType == EPACKTYPE_PET; }

    //获取低洞装备id
    DWORD getLowVidItemId(DWORD vid);

    const TSetDWORD& getDecomposeEquipIDs(DWORD dwDecomposeID) const;

    DWORD getTitlePreID(DWORD titleid) const;

    const SLotteryCFG* getLotteryCFG(DWORD type, DWORD year, DWORD month, DWORD itemType) const
    {
      const TMapLotteryCFG *pMapCfg = getAllLotteryCFG(type);
      if (!pMapCfg)
        return nullptr;
      if (type != ELotteryType_Head)
      {
        year = 0; month = 0;
      }

      auto it = pMapCfg->find(SLotteryItemCFG::getKey(year, month, itemType));
      if (it == pMapCfg->end())
        return nullptr;
      return &(it->second);
    }
    const SLotteryItemCFG* getLotteryItemCFG(DWORD type, DWORD itemId)const 
    {
      DWORD key = itemId * 100 + type;

      auto it = m_mapLotteryItemCFG.find(key);
      if (it == m_mapLotteryItemCFG.end())
        return nullptr;
      return &(it->second);
    }
    // 获取全部上架期间奖品
    const TMapLotteryCFG* getAllLotteryCFG(DWORD type) const { auto it = m_mapLotteryCFG.find(type); if (it == m_mapLotteryCFG.end()) return nullptr; else return &(it->second); }
    bool isLotteryHead(DWORD dwItemID) const { return m_setLotteryHeadIDs.find(dwItemID) != m_setLotteryHeadIDs.end(); }
    const TSetDWORD& getAllLotteryHead() const { return m_setLotteryHeadIDs; }
    void getSuitUpEquips(DWORD baseid, TSetDWORD& upEquips) const;
    void getItemByManualType(TSetDWORD& itemids, EManualType eType) const;

    const SHeadEffect* getHeadEffectCFG(DWORD itemId) const
    {
      auto it = m_mapHeadEffect.find(itemId);
      if (it == m_mapHeadEffect.end())
        return nullptr;
      return &(it->second);
    }
    TSetDWORD getFashionIDSet() const { return setFashionID; }
    bool loadLotteryConfig();
    bool loadLotteryGiveConfig();

    const SLotteryGiveCFG* getLotteryGiveCFGByTime(DWORD dwYear, DWORD dwMonth) const
    {
      DWORD k = dwYear * 100 + dwMonth;
      auto it = m_mapTime2LotteryGiveCfg.find(k);
      if (it == m_mapTime2LotteryGiveCfg.end())
        return nullptr;
      return &(it->second);
    }
    const SLotteryGiveCFG* getLotteryGiveCFGByBoxId(DWORD dwBoxId) const
    {
      auto it = m_mapBox2LotteryGiveCfg.find(dwBoxId);
      return it == m_mapBox2LotteryGiveCfg.end() ? nullptr : &(it->second);
    }
    const SLotteryGiveCFG* getLotteryGiveCFGByLotteryId(DWORD dwLotteryId) const
    {
      auto it = m_mapLottery2LotteryGiveCfg.find(dwLotteryId);
      return it == m_mapLottery2LotteryGiveCfg.end() ? nullptr : &(it->second);
    }
    const SLotteryGiveCFG*  getLotteryBoxYearMonth(TVecItemData& rVec) const;
    EEquipPos getArtifactEquipPos(EEquipPos pos) const; // 获取神器装备位对应的普通装备位, 反之亦然
    const TSetEquipPos& getArtifactPos() const;

    DWORD getGenderEquip(EGender eGender, DWORD dwEquip) const;

    const SEquipComposeCFG* getEquipComposeCFG(DWORD dwID) const;
  public:
    void fillLotteryCardRate(Cmd::LotteryRateQueryCmd& cmd) const;
    void timer(DWORD curSec);
  private:
    bool loadItemConfig();
    bool loadStuffConfig();
    bool loadEquipConfig();
    bool loadEquipLotteryConfig();
    bool loadCardConfig();
    bool loadMountConfig();
    bool loadMasterConfig();
    bool loadSuitConfig();
    bool loadRefineConfig();
    //bool loadFashionConfig();
    bool loadItemOriginConfig();
    bool loadItemType();
    bool loadEnchantConfig();
    bool loadExchangeConfig();
    bool loadEnchantPriceConfig();
    bool loadHairStyle();
    bool loadTitleConfig();
    bool loadAppellationConfig();
    bool loadEquipUpgradeConfig();
    bool loadEquipDecomposeConfig();
    bool loadCardRateConfig();
    bool loadHeadEffectConfig();
    bool loadPackSlotLvConfig();
    bool loadPickEffectConfig();
    bool loadEquipGenderConfig();
    bool loadEquipComposeConfig();

    //EEquipType getEquipType(EItemType eItemType) const;
    EItemType getItemType(const string& str) const;
    DWORD getBaseVID(DWORD vid) const { return vid - vid % 10000 + vid % 1000; } // ex:1701*001
    void getHigherEquipByVID(DWORD vid, TSetDWORD& equipids) const;
    bool initLottery();

  private:
    TMapItemCFG m_mapItemCFG;
    map<DWORD, DWORD> m_mapHairID2ItemID;
    TVecMasterCFG m_vecMasterCFG;
    TMapSuitCFG m_mapSuitCFG;
    TMapEquipSuit m_mapEquipID2SuitID;
    TVecRefineCFG m_vecRefineCFG;
    TVecMonsterCard m_vecMonster2Card;
    TMapQualityCard m_mapQualityCard;
    //TVecFashionCFG m_vecFashionCFG;
    //TSetItemNoSource m_setItemNoSource;
    TMapItemErrorCFG m_mapType2Error;
    TSetItemType m_setItemShow;
    TMapCDGroupCFG m_mapCDGroupCFG;
    TVecRefineTradeCFG m_vecRefineTradeCFG;

    TMapExchangeItemCFG m_mapExchangeItemCFG;
    SEnchantCFG m_arrEnchantCFG[EENCHANTTYPE_MAX];
    TSetDWORD m_setItemSkills;
    TMapEnchantPriceCFG m_mapEnchantPriceCFG;
    TMapEquipDecomposeCFG m_mapDecomposeCFG;
    TMapDecomposeEquipCFG m_mapDecomposeEquipCFG;
    TVecCardRateCFG m_vecCardRateCFG;

    map<DWORD, DWORD> mapTitle2PreID;
    TMapType2LotteryCFG m_mapLotteryCFG;  // 上架期间奖品
    TMapLotteryItemCFG m_mapLotteryItemCFG;  //itemid*100 + type
    TSetDWORD m_setLotteryHeadIDs;
    TMapId2HeadEffect m_mapHeadEffect;
    TSetDWORD setFashionID;
    TVecGenderReward m_vecGenderReward;

    // 套装相关
    TMapBaseVID2Equips m_mapBaseVID2Equips; // vid -> 装备列表
    map<DWORD, DWORD> m_mapBaseEquip2Equip; // 基础装备 -> 升级装备

    TMapPackSlotLvCFG m_mapLv2PackSlot;
    std::map<DWORD/*year*100+monty*/, SLotteryGiveCFG> m_mapTime2LotteryGiveCfg;
    std::map<DWORD/*boxid*/, SLotteryGiveCFG> m_mapBox2LotteryGiveCfg;
    std::map<DWORD/*lotteryid*/, SLotteryGiveCFG> m_mapLottery2LotteryGiveCfg;

    TMapTypeLotteryPool m_mapType2LotteryPool;
    std::map<DWORD, DWORD> m_mapItem2BasePrice; // 稀有物品查找表

    std::map<DWORD, DWORD> m_mapCardType2Rate; // 扭蛋卡牌类型对应的概率列表（客户端显示用）

    TMapEquipGender m_mapMaleEquipGender;
    TMapEquipGender m_mapFemaleEquipGender;

    DWORD m_dwCardCFGLoadTick = DWORD_MAX;

    TMapEquipComposeCFG m_mapEquipComposeCFG;
    TMapPetWear m_mapPetWearIDs;
};

inline void combinItemInfo(TVecItemInfo& vecDestItem, const TVecItemInfo& vecNeedItem)
{
  for (auto v = vecNeedItem.begin(); v != vecNeedItem.end(); ++v)
  {
    DWORD id = v->id();
    /*
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
    if (pCFG == nullptr)
      continue;
    if (pCFG->dwMaxNum == 1)
    {
      vecDestItem.push_back(*v);
      continue;
    }
    */

    auto o = find_if(vecDestItem.begin(), vecDestItem.end(), [id](const Cmd::ItemInfo& oItem) -> bool{
      return oItem.id() == id;
    });

    if (o == vecDestItem.end())
      vecDestItem.push_back(*v);
    else
      o->set_count(o->count() + v->count());
  }
}

inline void combinItemInfo(TVecItemInfo& vecDestItem, const ItemInfo& rNeedItem)
{
  combinItemInfo(vecDestItem, TVecItemInfo{rNeedItem});
}

inline void combineItemData(TVecItemData& vecDest, const TVecItemData& vecNeed)
{
  for (auto v = vecNeed.begin(); v != vecNeed.end(); ++v)
  {
    DWORD id = v->base().id();

    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
    if (pCFG == nullptr)
      continue;
    if (pCFG->dwMaxNum == 1)
    {
      vecDest.push_back(*v);
      continue;
    }

    auto o = find_if(vecDest.begin(), vecDest.end(), [id](const Cmd::ItemData& rData) -> bool{
      return rData.base().id() == id;
    });

    if (o == vecDest.end())
      vecDest.push_back(*v);
    else
      o->mutable_base()->set_count(o->base().count() + v->base().count());
  }
}

inline void combineItemData(TVecItemData& vecDest, const ItemData& rNeed)
{
  combineItemData(vecDest, TVecItemData{rNeed});
}

