
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
#include "SocialCmd.pb.h"
#include "AuctionCCmd.pb.h"
#include "AuctionSCmd.pb.h"

//#define  D_ROBOTS_TEST 1

#define DB_TABLE_AUCTION_CONFIG       "auction_config"
#define DB_TABLE_AUCTION_ITEM         "auction_item"
#define DB_TABLE_AUCTION_ITEM_SIGNUP  "auction_item_signup"
#define DB_TABLE_AUCTION_OFFERPRICE   "auction_offerprice"
#define DB_TABLE_AUCTION_EVENT        "auction_event"
#define DB_TABLE_AUCTION_CAN_SIGNUP   "auction_can_signup"

//sysid
#define AUCTION_SYSID_NO_SIGNUPTIME                   9505    //不是报名阶段
#define AUCTION_SYSID_INVALID_ITEM                    9506    //该物品不可上架报名
#define AUCTION_SYSID_DUPLICATE_ITEM                  9507    //相同物品不可重复上架

#define AUCTION_SYSID_INVALID_OFFERPEICE              9508    //不再竞拍期间不可出价
#define AUCTION_SYSID_OFFERPEICE_LESSTHEN_MAXPRICE    9509    //出价低于当前最高价
#define AUCTION_SYSID_OFFERPEICE_LOCKED               9510    //出价被锁住
#define AUCTION_SYSID_NO_LAST_LOG                     9504    //没有上次拍卖记录
#define AUCTION_SYSID_SIGNUP_SUCCESS                  9511    //报名上架成功
#define AUCTION_SYSID_AUCTION_END                     9512    //拍卖结束后，拍卖界面中的人提示

#define AUCTION_SYSID_AUCTION_DIALOG_PUBLICITY        57837   //公示期拍卖商品确认dialog
#define AUCTION_SYSID_AUCTION_DIALOG_START            57625   //拍卖开始dialog
#define AUCTION_SYSID_AUCTION_DIALOG_SUCCESS          57626   //拍卖成功dialog
#define AUCTION_SYSID_AUCTION_DIALOG_FAIL             57627   //拍卖失败dialog

#define RESULT_TIME_1     30
#define RESULT_TIME_2     10

#define  USER_CACHE_MAX_CNT   1000
#define  USER_CACHE_TIME      12 * 3600
#define  AUCTION_MAX_PRICE    666666   // 拍卖最高价默认值

#define ITER_PROTECT  {if (m_curAuctionOrderIter == m_vecAuctionItem.end()) return;}

//enum ERecordType
//{
//  ERecordType_BuySuccess = 1;       //竞拍获得道具
//ERecordType_SellSucess = 2;       //竞拍出售成功
//ERecordType_OverTakePrice = 3;    //出价被超过
//ERecordType_SignUpFail = 4;       //报名失败的拍品
//ERecordType_SellFail = 5;         //流拍
//ERecordType_SignUp = 6;           //拍品报名  
//ERecordType_SignUpSuccess = 7;    //拍品报名成功

//服务器使用  
//ERecordType_MaxOfferPrice = 8;       //出价   
//}

struct SAuctionEvent
{
  QWORD m_qwBatchId = 0;
  Cmd::AuctionEvent m_eEvent;
  DWORD m_dwItemId = 0;
  DWORD m_dwTime = 0;
  QWORD m_qwPrice = 0;
  string m_strName;
  DWORD m_dwZoneId = 0;
  QWORD m_qwMaxPrice = 0;
  QWORD m_qwCharId = 0;
  QWORD m_qwSignupId = 0;
  void toRecord(xRecord& s);
};

typedef std::list<SAuctionEvent> TListAuctionEvent;

struct OfferPriceRecord
{  
  QWORD m_qwId = 0;
  QWORD m_qwBatchId = 0;
  DWORD m_dwItemId = 0;
  QWORD m_qwCharId = 0;
  DWORD m_dwZoneId = 0;
  QWORD m_qwPrice = 0;
  QWORD m_qwSignupId = 0;
  Cmd::ERecordType m_eType = ERecordType_MaxOfferPrice;
  Cmd::EAuctionTakeStatus m_eTakestatus = EAuctionTakeStatus_None;
  DWORD m_dwVerifyTime = 0;
  DWORD m_dwTime = 0;
  string m_strName;
  Cmd::ItemData m_oItemData;

  void toRecord(xRecord& s, bool id);
  void fromRecord(const xRecord& s);
  void clear()
  {
    m_qwId = 0;
    m_qwBatchId = 0;
    m_dwItemId = 0;
    m_qwCharId = 0;
    m_dwZoneId = 0;
    m_qwPrice = 0;
    m_dwVerifyTime = 0;
    m_dwTime = 0;
    m_qwSignupId = 0;
  }
};

struct SignupItem
{
  QWORD m_qwId = 0;
  QWORD m_qwBatchId = 0;
  DWORD m_dwItemId = 0;
  QWORD m_qwSellerId = 0;
  string m_strSellerName;
  DWORD m_dwZoneId = 0;
  Cmd::ERecordType m_eType = ERecordType_SignUp;
  Cmd::EAuctionTakeStatus m_eTakestatus = EAuctionTakeStatus_None;
  QWORD m_qwOfferPriceId = 0;
  DWORD m_dwVerifyTime = 0;

  QWORD m_qwBasePrice = 0;// 起拍价(bcat)
  QWORD m_qwZenyPrice = 0;// 起拍价(zeny)
  DWORD m_dwFmPoint = 0;  // 附魔总点数
  DWORD m_dwFmBuff = 0;   // 附魔buff
  Cmd::ItemData m_oItemData;

  void toRecord(xRecord& s, bool id);
  void fromRecord(const xRecord& s);
};

/*订单*/
//enum EAuctionResult
//{
//  EAuctionResult_None = 0;
//EAuctionResult_Fail = 1;        //竞拍失败，流拍
//EAuctionResult_Sucess = 2;      //竞拍成功
//EAuctionResult_AtAuction = 3;   //拍卖中
//}
struct OrderItem
{
  void toRecord(xRecord& s);
  void fromRecord(const xRecord& s);
  void fillSignupItem();
  void fillOfferPrice();
  void toAuctionItemInfo(AuctionItemInfo* pInfo);

  void setMaskPrice(DWORD sign) { m_dwMaskPrice |= sign; }
  bool hasMaskPrice(DWORD sign) { return m_dwMaskPrice & sign; }
  
  QWORD m_qwBatchId = 0;
  DWORD m_dwItemId = 0;
  QWORD m_qwBasePrice = 0;
  QWORD m_qwZenyPrice = 0;
  bool m_bChoose = false;
  DWORD m_dwOrder = 0;
  QWORD m_qwSignupId = 0;
  QWORD m_qwOfferPriceId = 0;
  Cmd::EAuctionResult m_eStatus = EAuctionResult_None;
  QWORD m_qwTradePrice = 0;     //成交价格
  DWORD m_dwTime = 0;
  DWORD m_dwMaskPrice = 0;      //禁用档位
  DWORD m_dwAuction = 0;        //上架类型0:禁止上架 1:物品上架 2:装备上架
  DWORD m_dwIsRead = 0;         //是否已读取
  DWORD m_dwIsTemp = 0;         //是否临时上架

  //
  SignupItem m_stSignupItem;        //上架的玩家记录
  DWORD m_dwAuctionTime = 0;        //拍卖时间 
  OfferPriceRecord m_stMaxPrice;    //当前拍卖的最高价
  //拍卖事件
  Cmd::AuctionEvent m_eEvent = Cmd::AuctionEvent_None;  
};

struct CmpBySort {   //从小到大排序
  bool operator()(const OrderItem& lhs, const OrderItem& rhs) {
    if (lhs.m_dwOrder < rhs.m_dwOrder)
      return true;
    else if (lhs.m_dwOrder == rhs.m_dwOrder)
    {
      if (lhs.m_dwItemId < rhs.m_dwItemId)
        return true;
      else
        return false;
    }
    else
      return false;
  }
};

struct AuctionUser
{
  /*是否上架了某个物品*/
  bool hasSignuped(DWORD itemId, QWORD batchId);
  /*通知个人上架信息*/
  void ntfMySignupInfo(QWORD qwCurBatchId);

  /*上架锁物品*/
  bool lockSignup(DWORD itemId);
  /*上架解锁物品*/
  bool unlockSignup(DWORD itemId);
  void addSignupItem(const SignupItem& item);
  void loadSignupInfoFromDb(QWORD qwCurBatchId);
  void newBranch();

  //=======出价
  void ntfMyOfferPriceInfo(QWORD qwCurBatchId, DWORD itemId, QWORD signupId);
  bool loadOfferPriceFromDb(QWORD qwCurBatchId, DWORD itemId, QWORD signupId);
  bool lockOfferPrice(QWORD batchId, QWORD signupId);
  bool unlockOfferPrice(QWORD batchId, QWORD signupId);
  QWORD getReduceMoney(QWORD batchId, DWORD itemId, QWORD signupId, QWORD totalMoney);
  bool addOfferPriceRecord(OfferPriceRecord& record);
  OfferPriceRecord* getLastOfferPriceRecord() { return nullptr; }
  /*清空我的出价*/
  void clearOfferPrice(QWORD batchId, DWORD itemId, QWORD signupId);

  //新的物品拍卖
  void newItemAuction(DWORD itemId);
  bool isLastOfferPrice(QWORD batchId, DWORD itemId, QWORD signupId);
  void sendOverTakeCmd();
  bool addOrderId(QWORD orderId);
  bool delOrderId(QWORD orderId);
  void sendRedTip();

  bool m_bLoadSignUpFrom = false;         //是否从数据加载了我的上架信息
  bool m_bLoadOfferPriceFromDb = false;   //是否从数据加载了我的出价信息

  QWORD m_qwCharId;
  DWORD m_dwZoneId;
  string m_strName;
  bool m_bOnline = false;
  std::map<DWORD/*itemid*/, SignupItem> m_mapSignupItem;
  bool m_bOpenPanel;
  std::map<DWORD/*itemid*/, DWORD/*expire time*/> m_mapSignupLock;

  DWORD m_dwCurItemId = 0;        //当前拍卖物品
  QWORD m_qwSaveMoney = 0;        //已经出价的价格
  QWORD m_qwLastOfferPriceId = 0; //没领取的id
  std::map<DWORD/*batchid*/, std::map<QWORD/*signupId*/, DWORD/*expire time*/>> m_mapOfferPriceLock;
  OfferPriceRecord m_stCurAuctionOfferPrice;    //当前正在竞拍出价的记录
  DWORD m_dwActiveTime = 0;       //最后活跃时间，根据这个来清除内存
};


typedef std::map<QWORD/*signup_id*/, SignupItem> TMapSignupItem;
typedef std::vector<OrderItem> TVecOrderItem;
typedef TVecOrderItem::iterator TOrderItemIter;

typedef std::unordered_map<QWORD/*charid*/, AuctionUser> TMapId2User;
typedef std::unordered_map<QWORD/*charid*/, DWORD/*zoneid*/ > TMapId2ZoneId;
typedef std::map<DWORD/*itemid*/, OrderItem> TMapCanSignUpItem;

class AuctionManager:public xSingleton<AuctionManager>
{
friend class MessageStatHelper;
public:
  AuctionManager();
  ~AuctionManager();

  void init();
  bool loadDb();
  bool loadConfig();

  bool doUserCmd(QWORD charId, const std::string& name,DWORD zoneId, const BYTE* buf, WORD len);
  bool doServerCmd(QWORD charId, const std::string& name, DWORD zoneId, const BYTE* buf, WORD len);
  
  void timeTick(DWORD curSec);
  void onUserOnline(const SocialUser& rUser);
  void onUserOffline(const SocialUser& rUser);  
  //是否正在竞拍
  bool isNowAuction();
  //该物品是否正在竞拍
  bool isNowAuctionItem(QWORD batchId, DWORD itemId, QWORD signupId);
  /*结束全场拍卖*/
  void modifyAuctionTime(DWORD time);
  /*结束全场拍卖*/
  void stopAuction();
  
  static QWORD zeny2bcat(QWORD t);
  static QWORD bcat2zeny(QWORD t);

public:
  bool updateSignupPrice(QWORD batchId, QWORD signupId, QWORD price);

private:
  // 初始化档位
  void initMaskPrice(DWORD curSec);
  void calcSignupTime();
  void checkState(DWORD curSec);
  /*新场次*/
  bool enterNewBranch(DWORD curSec);
  /*人审核*/
  bool enterVerify(DWORD curSec);
  /*进入公示期*/
  bool enterPublicity(DWORD curSec);
  /*进入竞拍开始*/
  bool enterAuction(DWORD curSec);
  /*进入竞拍结束*/
  bool enterAuctionEnd(DWORD curSec);
  /*竞拍*/
  bool auctionTick(DWORD curSec);
  /*拍卖成功*/
  void enterAuctionStart(DWORD curSec);
  /*拍卖成功*/
  void enterAuctionSuccess(DWORD curSec, bool serverRestart = false);
  /*拍卖失败*/
  void enterAuctionFail(DWORD curSec, bool serverRestart = false);

  bool ntfAuctionDialog(Cmd::EDialogType type, DWORD msgId, TVecString& vec);
  bool ntfAuctionState();
  bool sendWorldCmd(void* buf, WORD len);
  void onOpenPanel(QWORD charId, const string& name, DWORD zoneId);
  void onClosePanel(QWORD charId, DWORD zoneId);
  void ntfAuctionInfo(QWORD charId, DWORD zoneId);
  OrderItem* getOrderItem(DWORD itemId) { auto it = m_mapCanSignupItem.find(itemId); if (it == m_mapCanSignupItem.end()) return nullptr; return &(it->second); }
  OrderItem* getOrderItemBySignupId(QWORD batchId, QWORD signupId)
  {
    for(auto& v : m_vecAuctionItem)
    {
      if(v.m_qwBatchId == batchId && v.m_qwSignupId == signupId)
        return &v;
    }

    return nullptr;
  }
  AuctionUser* getAuctionUser(QWORD charId) { auto it = m_mapAuctionUser.find(charId); if (it == m_mapAuctionUser.end()) return nullptr; return &(it->second); }
  
  //===cmd
  void signup(QWORD charId, DWORD zoneId, Cmd::SignUpItemCCmd& rev);
  void signupSceneRes(QWORD charId, DWORD zoneId, Cmd::SignUpItemSCmd& rev);
  void offerPrice(QWORD charId, DWORD zoneId, Cmd::OfferPriceCCmd& rev);
  void offerPriceSceneRes(QWORD charId, DWORD zoneId, Cmd::OfferPriceSCmd& rev);
  /*请求拍流水*/
  void reqFlowingWater(QWORD charId, DWORD zoneId, Cmd::ReqAuctionFlowingWaterCCmd& rev);
  bool takeRecord(AuctionUser*pUser, const::TakeAuctionRecordCCmd& rev);
  bool takeRecordSceneRes(AuctionUser*pUser, const::TakeRecordSCmd& rev);
  //===cmd

  //===db
  void loadCanSignup(std::map<DWORD/*itemid*/, DWORD>& mapCanSignup);
  void loadOrderInfoFromDb(QWORD batchId, TMapCanSignUpItem& mapCanSignupItem);
  //加载拍卖的物品信息
  void loadAuctionOrderInfoFromDb(QWORD batchId, TVecOrderItem& vecOrderItem);
  void loadChooseOrderInfoFromDb(QWORD batchId, TVecOrderItem& vecOrderItem);
  bool saveAuctionConfig2Db();
  bool removeOldMaxPrice(OfferPriceRecord& oldMaxPrice);
  bool loadRecordFromDb(Cmd::ERecordType type, QWORD id, QWORD charId, Cmd::AuctionRecord& info);
  void packSignupRecord(const xRecord& r, Cmd::AuctionRecord& info);
  void packOfferPriceRecord(const xRecord& r, Cmd::AuctionRecord& info);
  void verifyAuction(DWORD curSec);
  /*流拍写数据库*/
  void sellFail2Db(QWORD id);
  /*拍卖成功写数据库*/
  void sellSuccess2Db(OfferPriceRecord& buyer, SignupItem& seller);
  void insertAuciontEvent2Db();
  void delteDb(DWORD curSec);

  //===db
  
  void serverChoose();
  void updateChooseOrder(QWORD batchId, OrderItem& orderItem);
  void updateChooseSignup(QWORD id, QWORD batchId, ERecordType type);
  
  void sendSignupInfoToAllUser();
  void sendAuctionInfoToAllUser();
  void sendCmd2OpenUser(void* data, WORD len);
  void updateFlowingWater(Cmd::AuctionEvent eEvent, DWORD itemId, QWORD signupId, QWORD price = 0,const string& name= string(), QWORD playerId = 0, DWORD zoneId = 0, bool maxPrice = false);
  bool isRightOrder() { return m_curAuctionOrderIter != m_vecAuctionItem.end(); }
  bool canOfferPrice();  
  bool updateMaxPrice(AuctionUser* pUser, OfferPriceRecord& record);
  void nextAuctionOrder(DWORD curSec);
  void setAuctionSate(Cmd::EAuctionState state);
  void setAuctionBeginTime(DWORD t);

  //设置正在拍卖的物品状态
  void setOrderAuctionState(EAuctionResult status);
  void getLastAuctionInfo();
  //计算拍卖开始时间
  DWORD calcAuctionBeginTime(DWORD curSec);

  void calcTax(QWORD price, QWORD& earn, QWORD& tax);  
  string getFlowingWaterKey(QWORD batchId, DWORD itemId, QWORD signupId, DWORD index);
  bool canTake(const Cmd::AuctionRecord& logInfo);
  string getTableName(Cmd::ERecordType type)
  {
    string tableName = DB_TABLE_AUCTION_ITEM_SIGNUP;
    if (type == ERecordType_SignUpFail || 
	type == ERecordType_SellFail ||
       	type == ERecordType_SellSucess ||
	type == ERecordType_SellSucessPass ||
	type == ERecordType_SellSucessNoPass)
      tableName = DB_TABLE_AUCTION_ITEM_SIGNUP;
    else
      tableName = DB_TABLE_AUCTION_OFFERPRICE;
    return tableName;
  }
  void getAuctionStateCmd(NtfAuctionStateCCmd& cmd);
  bool isOpenPanel(QWORD charId);
  void sendOverTakePriceMsg(QWORD charId);
  void packNextAuctionInfo(NtfNextAuctionInfoCCmd& cmd);
  //生成一个唯一的订单号
  QWORD genOrderId();
  void clearUserCache(DWORD curSec);
  OrderItem* getAuctionItem(QWORD batchId, DWORD itemId, QWORD signupId);
  void sendRedTip(QWORD charId);

  void checkMaskPrice(DWORD curSec);
  // 刷新临时上架物品
  void refreshTempRecord();

private:
  xTimer m_oneSecTimer;
  xTimer m_fiveSecTimer;
  xTimer m_FiveMinTimer;
  xTimer m_ThirtyMinTimer;
  xTimer m_oneHourTimer;

  QWORD m_qwCurBatchId = 0;             //当前竞拍场次
  Cmd::EAuctionState m_eAuctionState = EAuctionState_Close;
  DWORD m_dwAuctionBeginTime = 0;       //竞拍开始时间
  DWORD m_dwBatchCreateTime = 0;        //当前批次创建的时间，不会受延期改变
  DWORD m_dwAuctionEndTime = 0;         //竞拍结束时间

  DWORD m_dwSignupBeginTime = 0;        //报名开始时间 
  DWORD m_dwSignupEndTime = 0;          //报名截止时间

  DWORD m_dwAuctionNtfTime = 0;         //竞拍开始通告时间
  DWORD m_dwPublicityBeginTime = 0;     //公示期开始时间

  DWORD m_dwCurAuctionIndex = 0;        //第几件拍品

  //tmp
  bool m_bFirstInit = false;

  TMapId2User m_mapAuctionUser;
  TMapId2ZoneId m_mapOpenUser;          //开界面的玩家
  TMapCanSignUpItem m_mapCanSignupItem;    //可报名上架物品礼包
  TVecOrderItem m_vecAuctionItem;                         //当前拍卖物品列表
  TOrderItemIter m_curAuctionOrderIter;                   //正在竞拍的物品   

  UpdateAuctionFlowingWaterCCmd m_cacheCmdUpdateFlowingWater; 
  std::list<FlowingWaterInfo> m_curListFlowingWaterInfo;    //当前拍卖的流水,从头插入
  
  DWORD m_dwCurOrderStartTime = 0;      //当前拍卖开始时间
  DWORD m_dwCurOrderEndTime = 0;        //当前拍卖结束时间
  DWORD m_dwResultTime = 0;

  std::map<DWORD, DWORD> m_mapMaskPrice; //屏蔽档位价格对应的时间戳

  QWORD m_qwLastBatchId = 0;
  TVecOrderItem m_vecLastAuctionItem;       //上次拍卖的物品信息
  NtfAuctionInfoCCmd m_cacheLastAuction;    //上次拍卖的信息推送缓存
  
  std::unordered_map<string/*key*/, ReqAuctionFlowingWaterCCmd> m_mapCacheFlowing;    //缓存的历史拍卖流水
  std::set<QWORD/*charid*/> m_setOfflineOverTake;       //出价被超过了离线消息，玩家上线后通知。

  NtfNextAuctionInfoCCmd m_cmdNextAuctionInfo;
  DWORD m_dwLastItemId = 0;
  DWORD m_dwCurItemId = 0;
  DWORD m_dwCurItemPrice = 0;
  QWORD m_qwLastSignupId = 0;
  QWORD m_qwCurSignupId = 0;
  bool bNext = false;
  MessageStat m_oMessageStat;
  TListAuctionEvent m_listAuctionEvent;
  DWORD m_dwNextDelTime = 0;
};

class MessageStatHelper
{
public:
  MessageStatHelper(DWORD cmd, DWORD param);
  ~MessageStatHelper();
};

class MsgGuard : public xSingleton<MsgGuard>
{
public:
  bool lock(DWORD type, QWORD charId);

  bool unlock(DWORD type, QWORD charId);
private:
  std::unordered_map<DWORD/*key*/, std::unordered_map<QWORD/*charId*/, QWORD/*time*/>> m_mapGuard;
};
