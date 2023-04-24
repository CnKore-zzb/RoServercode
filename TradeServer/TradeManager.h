
#pragma once

#include "xSingleton.h"
#include <list>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "SceneItem.pb.h"
#include "RecordTrade.pb.h"
#include "SceneTrade.pb.h"
#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "ItemConfig.h"
#include "TableManager.h"
#include "MsgManager.h"
#include "MessageStat.h"
#include "xTime.h"

#define DB_TABLE_SECURITY  "trade_security"

#define MAX_LIMIT_NAME_COUNT 100

#define MSG_INTERVAL1        1000      //毫秒
#define MSG_INTERVAL2        1000      //毫秒

#define LIST_FIND(a, b)   ( std::find_if(a.begin(), a.end(), [b](PendingInfo& a) { return a.orderId == b; }) )


#define SYS_MSG_CFG_ERROR       10000               //策划表不一致
#define SYS_MSG_DB_ERROR        10001               //数据库操作失败
#define SYS_MSG_SYS_ERROR       10002               //系统异常

#define SYS_MSG_SELL_INVALID_COUNT    10100         //出售-出售的个数不正确
#define SYS_MSG_SELL_INVALID_PRICE    10101         //出售-出售的价格不正确
#define SYS_MSG_SELL_INVALID_ITEMID   10102         //出售-不可交易的物品
#define SYS_MSG_SELL_INVALID_PARAMS   10103         //出售-无法出售，请求参数有误
#define SYS_MSG_SELL_IS_FULL          10104         //出售-无法出售，出售挂单已达上限
#define SYS_MSG_SELL_CANNOT_SELL1     10105         //出售-无法出售，插卡物品无法出售
#define SYS_MSG_SELL_CANNOT_SELL2     10107         //出售-无法出售，背包找不到该物品
#define SYS_MSG_SELL_CANNOT_SELL3     10108         //出售-无法出售，物品数据同步异常
#define SYS_MSG_SELL_CANNOT_SELL4     10106         //出售-强化的装备不可出售，前往分解
#define SYS_MSG_SELL_MONEY_NOT_ENOUGH 10109         //出售-无法出售，金钱不足

#define SYS_MSG_BUY_INVALID_COUNT       10150       //购买-购买的个数不正确
#define SYS_MSG_BUY_CANNOT_FIND_PENDING 10151       //购买-找不到相应的挂单，挂单已售空
#define SYS_MSG_BUY_PENDING_WAS_LOCKED  10152       //购买-无法购买，挂单被锁定
#define SYS_MSG_BUY_INVALID_PARAMS      10153       //购买-无法购买，请求参数有误
#define SYS_MSG_BUY_MONEY_NOT_ENOUGH    10154       //购买-无法购买，金钱不足

#define SYS_MSG_CANCEL_ALREADY_SELLED   10200       //撤单-无法撤单，已经出售
#define SYS_MSG_CANCEL_WAS_LOCKED       10201       //撤单-无法撤单，该挂单已被锁定     

#define SYS_MGS_SELL_SUCCESS            10250       //成功出售：恭喜！您成功地售出了[63cd4e]%s[-]个[63cd4e]%s[-]，共获得了[63cd4e]%s[-]金币。

#define SYS_MSG_BUY_INVALID_PRICE       10156       //购买-价格非法

typedef std::pair<DWORD, DWORD> DWORDPAIR;

struct BriefListCache {
    BriefPendingListRecordTradeCmd cmd;
      DWORD expireTime;
};

struct CmpByValue1 {   //从大到小排序
  bool operator()(const DWORDPAIR& lhs, const DWORDPAIR& rhs) {
    return lhs.second > rhs.second;
  }
};

struct CmpByValue2 {   //从小到大排序
  bool operator()(const DWORDPAIR& lhs, const DWORDPAIR& rhs) {
    return lhs.second < rhs.second;
  }
};

struct HotInfo {
  DWORD dwType = 0;
  DWORD dwItemid = 0;
  DWORD dwCount = 0;
  vector<EProfession> vecJob;
  bool isJob(DWORD dwJob);
};

//typedef std::vector<NameInfo> TVecSellerInfo;

struct PendingInfo
{
  QWORD orderId = 0;
  DWORD itemId = 0;
  DWORD price = 0;
  DWORD count = 0;
  QWORD sellerId = 0;
  std::string name;      //卖家姓名
  QWORD pendingTime = 0; //挂单时间
  DWORD refineLv = 0;

  Cmd::ItemData itemData;
  bool isOverlap;

  DWORD endTime = 0;      //公示期结束时间
  DWORD publicityId = 0;

  DWORD sellCount = 0;

  NameInfoList buyerList;
  NameInfoList sellerList;

  PendingInfo() :
    count(0),
    sellerId(0)    
  {
    isOverlap = true;
    m_isLocked = false;
  }

  void marshal(std::string& out) const ;
  void unmarshal(std::string& in);
  
  bool lock()
  {
    if (m_isLocked)
      return false;
    m_isLocked = true;
    return true;
  }

  bool unlock()
  {
    if (!m_isLocked)
      return false;
    m_isLocked = false;
    return true;
  }
  
  DWORD refinelv() const
  {
    return itemData.equip().refinelv();
  }

  bool damage() const 
  {
    return itemData.equip().damage();
  }

  DWORD upgradeLv() const
  {
    return itemData.equip().lv();
  }

  std::string toString()
  {
    std::stringstream ss;
    ss << "orderId:" << orderId
      << " ,itemId:" << itemId
      << " ,price:" << price
      << " ,count:" << count
      << " ,sellerId:" << sellerId
      << " ,refineLv:" << refineLv
      << " ,pendingTime:" << pendingTime
      << " ,isOverlap:" << isOverlap;
    return ss.str();
  }
  
  bool addSellCount(DWORD addCount)
  {
    if ((sellCount + addCount) > count)
      return false;
    sellCount += addCount;
    if (sellCount >= count)
      return false;
    return true;
  }
  void pack2Proto(Cmd::TradeItemBaseInfo* pOut) const;
private:
  bool m_isLocked;
};

typedef std::list<PendingInfo> PENDING_INFO_LIST_T;
typedef std::vector<PendingInfo>  PENDINGINFO_VEC_T;

struct LogCount
{
  DWORD time = 0;
  DWORD count = 0;
};

class HotInfoMgr
{
public:
  void addItem(DWORD itemId, DWORD count, DWORD sec);
  void refresh(DWORD curSec);
  std::vector<DWORD> getRank(DWORD dwType, DWORD job);
  std::vector<DWORD> getPublicityRank();

  DWORD getCount(DWORD itemId);
  //
  void addPublicity(DWORD itemId, QWORD buyerId);
  void decPublicity(DWORD itemId, QWORD buyerId);

private:
  std::map<DWORD/*dwtype*/, std::map<DWORD/*time*/, std::vector<HotInfo>>> m_mapHotInfos;
  std::map<DWORD/*itemid*/, std::vector<LogCount>> m_mapCount;          //最近三天的历史成交量
  std::map<DWORD/*itemid*/, DWORD/*count*/ > m_allCount;
  std::map<DWORD/*itemid*/, std::set<QWORD/*buyer id*/>> m_mapPublicity;                    //公示期抢购人, 默认给一个buyerid =0
};

class SecurityCmd;

class StoreBaseMgr
{  
public:
  StoreBaseMgr(DWORD itemId);
  virtual ~StoreBaseMgr();
  bool hasOrder(QWORD orderId);
  virtual bool lockOrder(QWORD orderId) = 0;
  virtual bool unlockOrder(QWORD orderId) = 0;
  void addPlayerPendingInfo(const PendingInfo & info);
  virtual void checkOffTheShelf() = 0;
  DWORD getTotalCount() { return  getPlayerCount(); }
  DWORD getPlayerCount();
  PendingInfo* getPendingInfo(QWORD orderId);
  //系统补仓
  bool resell(QWORD charId, DWORD zoneId, QWORD orderId, DWORD newPrice);
  bool addPending(QWORD charId, std::string name, const Cmd::TradeItemBaseInfo& itemBaseInfo);
  bool reducePending(QWORD orderId, DWORD reduceCount, bool isAutoErase, bool& needErase, PendingInfo* pPendingInfo);
  bool reducePendingAutoErase(QWORD orderId, DWORD reduceCount, PendingInfo* pPendingInfo);

  TVecQWORD getRandomPending(DWORD count);
  
  //是否有物品在售
  bool hasPending() { return  !m_playerStore.empty(); }
  
  DWORD  getKey(DWORD refineLv, bool bDamage, DWORD upgradeLv);
  void addPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv, DWORD count);
  void decPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv, DWORD count);

  //物品挂单count 总和
  DWORD getPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv) ;
  void addBriefBuyInfo(const string& buyerName);
  std::vector<Cmd::BriefBuyInfo>& getBriefBuyInfo() { return m_vecBriefBuyInfo; }
  //禁止出售，下架
  bool tradeSecurityCancel(SecurityCmd& cmd);

  virtual void printDebug() = 0;

  virtual bool adjustSelfPrice() = 0;

  bool isOverlap()
  {
    return m_overLap;
  }
protected:
  PENDING_INFO_LIST_T m_playerStore; 
  std::map<DWORD/*key*/, DWORD/*count*/> m_pendingCount;
  std::vector<Cmd::BriefBuyInfo> m_vecBriefBuyInfo;       //简单的购买人信息
  DWORD m_dwMaxShowCount = 6;
  DWORD m_dwItemId;
  bool m_overLap = false;
};

class OverlapStoreMgr :public StoreBaseMgr
{
public:
  OverlapStoreMgr(DWORD itemId);
  ~OverlapStoreMgr();
  virtual void checkOffTheShelf();
  DWORD getLockCount();
  virtual bool lockOrder(QWORD orderId);
  virtual bool unlockOrder(QWORD orderId);
  bool lock(DWORD count);
  bool unlock(DWORD count);
  /*出仓*/
  bool outStore(QWORD buyerId, const string& name, DWORD zoneId, DWORD buyPrice, DWORD outCount);
  bool checkBuyPrice(DWORD price);
  virtual DWORD getPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv) { return m_pendingSum; }
private:
  void reducePlayerPendingInfo(QWORD orderId, DWORD newCount);
  void addLockCount(DWORD count);
  void reduceLockCount(DWORD count);

  DWORD playerOutStore(QWORD buyerId, const string& name, DWORD zoneId, DWORD count, PendingInfo& pendingInfo, DWORD buyPrice, Cmd::NameInfoList& nameList);
  virtual bool adjustSelfPrice();
  void printDebug()
  {
    /* XDBG << "[交易-内存打印], itemid:" << m_dwItemId << "price:" << m_price << "总个数：" << getTotalCount() << "系统仓个数：" << getSysCount() << "玩家仓个数：" << getPlayerCount() << "锁仓数：" << getLockCount() << XEND;
     for (auto it = m_playerStore.begin(); it != m_playerStore.end(); ++it)
       XDBG << "[交易-内存打印] 堆叠，玩家仓详细" << it->toString() << XEND;*/
  }
private:
  DWORD m_pendingSum = 0; //堆叠物品挂单里的物品总和
  DWORD m_lockCount = 0;
};

class NonOverlapStoreMgr :public StoreBaseMgr
{
public:
  NonOverlapStoreMgr(DWORD itemId);
  ~NonOverlapStoreMgr();
  virtual void checkOffTheShelf();
  virtual bool lockOrder(QWORD orderId);
  virtual bool unlockOrder(QWORD orderId);
  /*出仓*/
  bool outStore(QWORD orderId, QWORD buyerId, const string& name, DWORD zoneId, DWORD buyPrice, DWORD outCount = 1);
  virtual bool adjustSelfPrice();
  void printDebug()
  {
  }
  bool checkBuyPrice(QWORD orderId, DWORD price);
};

typedef std::map<DWORD/*prcie*/, StoreBaseMgr*/**/, std::less<DWORD>> STOR_MGR_MAP_T;

class TradeItemInfo
{
public:
  TradeItemInfo(DWORD dwId, bool isOverlap);

  ~TradeItemInfo();
  
  void init(DWORD dwServerPrice, DWORD dwLastCalcPriceTime, DWORD T);
  void loadPlayerPendingInfo(PendingInfo&pendingInfo);
  void saveToDb();
  void saveServerPrice(DWORD curSec, DWORD refineLv, DWORD T, DWORD K, DWORD D, float QK, DWORD P0, DWORD oldP, DWORD newP);

  bool addNewPending(QWORD charId, std::string name, DWORD zoneId, const Cmd::ReduceItemRecordTrade& );

  /*检查过期 挂单*/
  void checkExpirePending();
  
  bool isOverlap();

  //是否有物品在售
  bool hasPending();

  void printDebug()
  {
    // for (auto it = m_storeMgrMap.begin(); it != m_storeMgrMap.end(); ++it)
    // {
    //   if (it->second)
    //    it->second->printDebug();
    // }
  }
  void printDebug(DWORD price)
  {
    // auto it = m_storeMgrMap.find(price);
    // if (it == m_storeMgrMap.end())
    // {
    //   XDBG << "[交易-内存打印] 内存中没有 物品id:" << m_dwItemId << "价格:" << it->first << XEND;
    //   return;
    // }
    // if (it->second)
    //   it->second->printDebug();
  }
  StoreBaseMgr* getStoreBaseMgr()
  {
    return m_pStoreMgr;
  }

  //当前数据库里的库存量，过期的不统计, 优化不从数据库查询
  bool getDbPendingCount(DWORD refineLv,bool isDamage, DWORD upgradeLv, DWORD &count);

  //当前数据库里T秒内的成交量，过期的不统计
  bool getDbSaledCount(DWORD curSec, DWORD T, DWORD refineLv, DWORD &count);
  /*更新服务器价格*/
  bool adjustServerPrice(DWORD curSec);  
  bool adjustSelfPrice();
  DWORD getServerPrice() { return m_dwServerPrice; }
  void setServerPrice(DWORD price) { m_dwServerPrice = price; }

private:
  bool m_isOverlap;
  DWORD m_dwItemId = 0;
  DWORD m_dwServerPrice = 0 ;          //服务器价格,只有价格依赖与自身的装备才有
  time_t m_lastCalcPriceTime = 0 ;
  DWORD m_dwT = 0;
  StoreBaseMgr* m_pStoreMgr = nullptr;
  DWORD m_nextExpireTime = 0;
};

typedef std::map<DWORD/*item id*/, TradeItemInfo* > TRADE_ITEM_INFO_MAP_T;

//============公示期

struct BuyerInfo
{
  QWORD id = 0;
  QWORD buyerId = 0;
  DWORD count = 0;
  DWORD trueCount = 0;
  DWORD getLeft() { return count - trueCount; }
  bool addCount(DWORD addCount)
  {
    if ((trueCount + addCount) > count)
      return false;
    trueCount += addCount;
    if (trueCount >= count)
      return false;
    return true;
  }
};

typedef std::map<QWORD /*charid*/, BuyerInfo> BUYERINFO_MAP_T;

struct PublicityInfo
{
  DWORD id;
  string uniqueid;
  DWORD itemId;
  DWORD startTime;
  DWORD endTime;
  DWORD price;
  DWORD m_dwLastStopTime = 0;
  //cfg
  string itemName;      
  bool isOverlap = true;

  PENDINGINFO_VEC_T vecPending;
  BUYERINFO_MAP_T mapBuyer;

  bool isPublicity() { return endTime != 0; }
  DWORD getCount() 
  {
    DWORD count = 0;
    for (auto& v : vecPending)
    {
      count += v.count;
    }
    return count;
  }
  DWORD getBuyerCount() { return mapBuyer.size(); }
  DWORD getMyBuyCount(QWORD charId)
  {
    auto it = mapBuyer.find(charId);
    if (it == mapBuyer.end())
      return 0;
    return it->second.count;
  }
  DWORD getAllBuyCount()
  {
    DWORD count = 0;
    for (auto &v : mapBuyer)
    {
      count += v.second.count;
    }
    return count;
  }
  bool addPending(QWORD charId, std::string name, const Cmd::TradeItemBaseInfo& itemBaseInfo); 
  bool addBuyer(QWORD charId, DWORD count);
  void calcBuyer(DWORD needCount);
  void calcSeller(DWORD needCount);

  BuyerInfo* getBuyerInfo(QWORD buyerId) 
  {
    auto it = mapBuyer.find(buyerId);
    if (it == mapBuyer.end())
      return nullptr;
    return &(it->second);
  }

  PendingInfo* getAPendingInfo()
  {
    if (vecPending.empty())
      return nullptr;
    
    return &(vecPending[0]);
  }

  //
  bool outStore();
  bool updateEndTime(DWORD beginTime, DWORD endTime, DWORD curTime);
  //禁止出售
  void securityCancelSell(SecurityCmd& security);
  //禁止购买，撤销买单放入抢购失败记录
  void securityCancelBuy(SecurityCmd& security);
};

typedef std::map<string, PublicityInfo> PUBLICITYINFO_MAP_T;

class PublicityMgr
{
public:
  PublicityMgr() {}
  ~PublicityMgr() {}

  void loadFromDb();
 
  bool reqBuy(QWORD charId, DWORD zoneId, Cmd::BuyItemRecordTradeCmd& rev);
  bool trueBuy(QWORD charId, DWORD zoneId, Cmd::ReduceMoneyRecordTradeCmd& rev);

  //请求出售
  bool reqSell(QWORD charId, DWORD zoneId, Cmd::SellItemRecordTradeCmd* rev);
  bool trueSell(QWORD charId, std::string name, DWORD zoneId, Cmd::ReduceItemRecordTrade& rev);
  PublicityInfo* getPublicityInfo(const string& key);
  PublicityInfo* getPublicityInfo(DWORD id);

  bool addnew(PublicityInfo& info);
  bool restart(PublicityInfo& info);
  bool stop(PublicityInfo& info);
  void notify(const PublicityInfo& info);
  void timeTick(DWORD curSec);
  void checkMaintain(DWORD curSec);
  void getMaintaintime(DWORD& beginTime, DWORD& endTime);

  //某个物品是否处于公示期
  bool isPublicity(DWORD itemId);

  //禁止出售
  void securityCancelSell(SecurityCmd& security);
  //禁止购买，撤销买单放入抢购失败记录
  void securityCancelBuy(SecurityCmd& security);
private:
  PUBLICITYINFO_MAP_T m_mapPublicity;
  TSetDWORD m_setInPulibicy;          //公示期的物品id
  DWORD m_dwNextCheckMaintain = 0;
};

class SecurityCmd
{
public:
  void init(const Cmd::SecurityCmdSceneTradeCmd& rev);
  bool fromDb(const xRecord& rRecord);
  bool toDb();
  bool delDb();
  bool needProcess(QWORD charId, DWORD itemId, DWORD refineLv);
public:
  QWORD m_qwId = 0;
  string m_strKey;
  QWORD m_qwCharid = 0;
  DWORD m_dwItemId = 0;
  Cmd::ESecurityType m_type;
  int  m_refineLv = 0;
  DWORD m_dwTime = 0;
};

class TradeManager : public xSingleton<TradeManager>
{
  public:
    TradeManager();
    ~TradeManager();
    void init();
    bool loadConfig();
    bool load();

    bool doTradeServerCmd(QWORD charId, std::string name, DWORD zoneId, const BYTE* buf, WORD len);
    bool doTradeUserCmd(QWORD charId, DWORD zoneId, const BYTE* buf, WORD len);
    inline bool checkCharId(QWORD charId1, QWORD charId2);
    //send
    bool sendCmdToMe(QWORD charId, DWORD zoneId, const BYTE* buf, WORD len);
    void sendSysMsg(QWORD charId, DWORD zoneId, DWORD msgid, EMessageType eType, const MsgParams& params, EMessageActOpt opt = EMESSAGEACT_ADD);
    void sendWorldMsg(DWORD msgid, EMessageType eType, const MsgParams& params, EMessageActOpt opt = EMESSAGEACT_ADD);

    void fetchAllBriefPendingList(QWORD charid, DWORD zoneId, Cmd::BriefPendingListRecordTradeCmd& rev);
    std::map<QWORD, BriefListCache> m_mapBriefListCache;
    void fetchDetailPendingList(QWORD charId, DWORD zoneId, Cmd::DetailPendingListRecordTradeCmd& rev);
    //获取热门列表
    void fetchHotPendingList(QWORD charId, DWORD zoneId, Cmd::HotItemidRecordTrade& rev);
    void getHotId(DWORD dwJob, DWORD dwShowType,DWORD dwCount, std::set<DWORD>& resultSet);
    //请求购买
    bool reqBuy(QWORD charId, DWORD zoneId, Cmd::BuyItemRecordTradeCmd& rev);
    bool trueBuy(QWORD charId, const string& name, DWORD zoneId, Cmd::ReduceMoneyRecordTradeCmd& rev);
    //请求出售
    bool reqSell(QWORD charId, DWORD zoneId, Cmd::SellItemRecordTradeCmd* rev, bool checkFull = true);
    bool trueSell(QWORD charId,std::string name,  DWORD zoneId, Cmd::ReduceItemRecordTrade& rev);
    //取消挂单
    bool cancelPending(QWORD charId, DWORD zoneId, Cmd::CancelItemRecordTrade& rev);
    //del char
    bool cancelPendingDelChar(QWORD charId, QWORD orderId);
    //重新上架
    bool resell(QWORD charId, DWORD zoneId, Cmd::ResellPendingRecordTrade& rev);
    //是否需要公示
    bool checkPublicity(const Cmd::ItemData& itemData);

    void getItemSellInfo(Cmd::ReqServerPriceRecordTradeCmd& rev);

    void fetchItemSellInfoList(QWORD charId, DWORD zoneId, Cmd::ItemSellInfoRecordTradeCmd& rev);

    //del char 
    bool delChar(QWORD charId);
    void adjustPrice(DWORD curSec);
    void openPanel(QWORD charId, DWORD zoneId);
    void closePanel(QWORD charId); 
    void ListNtf(QWORD charId, EListNtfType);
    void timeTick(DWORD curSec);
    void savePendingPrice(QWORD orderid, DWORD itemId, DWORD oldPrice, DWORD newPrice);

    TradeItemInfo* getTradeItemInfo(DWORD itemId)
    {
      auto it = m_itemInfoMap.find(itemId);
      if (it == m_itemInfoMap.end())
        return nullptr;      
      return it->second;
    }
    StoreBaseMgr* getStoreBaseMgr(DWORD itemId)
    {
      TradeItemInfo* p = getTradeItemInfo(itemId);
      if (p == nullptr)
        return nullptr;
      return p->getStoreBaseMgr();
    }

public:
  static bool getPendInfoFromDb(QWORD orderId, PendingInfo& out, bool fillItemData = false);
  
  void addItem(EOperType type, QWORD buyerId, DWORD zoneId, const PendingInfo& pendingInfo, bool bOffline = false);
  void addMoney(EOperType type, QWORD getMoneyPlayer, DWORD zoneId, const PendingInfo& pendingInfo, QWORD buyerId = 0, DWORD successCount =0, DWORD consumeMoney=0);
  bool insertSelledDbLog(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& buyerList);
  bool insertSelledDbLog2(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& nameList, xRecordSet& recordSet);
  bool insertDbLogSet(xRecordSet& recordSet);

  bool insertBuyedDbLog(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& nameList, DWORD failCount/*公示期抢购失败*/, DWORD endTime /*
公示期结束时间*/, DWORD totalBuyCount/*公示期总购买个数*/);
  bool insertBuyedDbLog2(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& nameList, DWORD failCount, DWORD endTime, DWORD totalBuyCount, xRecordSet& recordSet);
  //db 
  bool dbDelPending(QWORD orderId, QWORD charId, EOperType type);
  bool dbDelPending(DWORD publicityId);
  bool dbDelPublicityBuy(QWORD orderId, QWORD charId, EOperType type);
  bool dbDelPublicityBuy(DWORD publicityId);
  bool dbDelPublicity(QWORD orderId, EOperType type);
  string generateKey(const Cmd::ItemData& itemData);
  bool getPriceFromCache(const Cmd::ItemData& itemData, DWORD& outPrice);
  void updatePriceCache(const Cmd::ItemData& itemData, DWORD inPrice);
  
private:
  inline DWORD calcBoothfee(DWORD price, DWORD count);  
  void  onOneDayTimeUp(DWORD curSec);
  bool resellLock(QWORD orderId);
  bool resellUnlock(QWORD orderId);
  DWORD calcTax(DWORD& gainMoney, DWORD price);
public:
  HotInfoMgr m_hotInfoMgr;

  //server price mgr
public:
  DWORD getServerPrice(DWORD itemId);
  DWORD getServerPrice(const Cmd::ItemData& itemData, DWORD itemId =0);
  DWORD getMinServerPrice(const SExchangeItemCFG* pCfg);
  bool checkIsRightPrice(const Cmd::TradeItemBaseInfo& rItemBaseInfo, DWORD& serverPrice);
  QWORD calcRefinePrice(DWORD itemId, DWORD refineLv, bool damage);
  DWORD calcEnchantPrice(const ItemData& rItemData);
  DWORD calcUpgradePrice(DWORD itemId, DWORD lv, bool maxLv);
  DWORD getMyZoneId(QWORD charId);
  string getMyName(QWORD charId);
  DWORD getMaxPendingLimit(QWORD charId);
  DWORD getReturnRate(QWORD charId);
  void checkDbItem();

public:
  void addOneSecurity(const Cmd::SecurityCmdSceneTradeCmd& rev);
  void delOneSecurity(const Cmd::SecurityCmdSceneTradeCmd& rev);
  bool checkSellSecurity(QWORD charId, Cmd::SellItemRecordTradeCmd* rev);
  bool checkBuySecurity(QWORD charId, DWORD itemId);
  bool checkBuySecurity(QWORD charId, PendingInfo& rInfo);
  string getSecuiryKey(const Cmd::SecurityCmdSceneTradeCmd& rev);

private:
  std::map<DWORD/*itemid*/, std::set<DWORD>> m_mapPrice;
private:
  TRADE_ITEM_INFO_MAP_T m_itemInfoMap;
  std::map<DWORD/*order*/,const STradeItemTypeData*> m_sortMap;
  std::map<QWORD/*charid*/, DWORD/*zoneid*/> m_panelMap;
  
  xTimer m_oOneSecTimer;
  xTimer m_oFivSecTimer;
  xTimer m_oOneMinTimer;
  xTimer m_oOneHourTimer;
  xTimer m_oTenMinTimer;
  xDayTimer m_oOneDayTimer;

  PublicityMgr m_oPublicityMgr;
  //zoneid cache
  std::map<QWORD/*charId*/, std::pair<DWORD/*zoneid*/, DWORD/*cache time*/>> m_zoneCache;
  //server price cache
  std::map<string/*unique*/, DWORD/*price*/> m_priceCache;
  
  std::map<QWORD/*uniqueid*/, TVecDWORD> m_itemJobCache;
  std::map<string/*key*/, SecurityCmd> m_mapSecurityCmd;

public:
  MessageStat m_oMessageStat;
  xTime m_oFrameTimer;
  std::map<QWORD/*order id*/, DWORD/*expire time*/> m_resellLock;
};

class TradeMsgGuard : public xSingleton<TradeMsgGuard>
{
public:
  bool lock(DWORD type, QWORD charId);
  bool unlock(DWORD type, QWORD charId);
private:  
  std::unordered_map<DWORD/*key*/, std::unordered_map<QWORD/*charId*/, QWORD/*time*/>> m_mapGuard;
};

class MessageStatHelper
{
public:
  MessageStatHelper(DWORD cmd, DWORD param);
  ~MessageStatHelper(); 
};
