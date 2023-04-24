#pragma once
#include "xDefine.h"
#include <list>
#include "SessionCmd.pb.h"
#include "SceneTrade.pb.h"
#include "xDBFields.h"
#include "ItemConfig.h"
#include "RecordTrade.pb.h"

#ifdef _OLD_TRADE
//
//enum EGiveStatus
//{
//  EGiveStatus_No = 0,           //未领取
//  EGiveStatus_Sending = 1,      //发送到scene中
//  EGiveStatus_Toke = 2,         //领取成功
//  EGiveStatus_Refuse = 3,         //拒绝
//};

//struct CmpByValue1 {   //从大到小
//  bool operator()(const Cmd::LogItemInfo& first, const Cmd::LogItemInfo& second)
//  {
//    if (first.tradetime() > second.tradetime())
//      return true;
//    return false;
//  }
//};
//
//class SessionUser;
//class TradeLog
//{
//public:
//  TradeLog(SessionUser* pUser);
//  ~TradeLog();
//
//  void onUserOnline();
//  void onUserOffline();
//
//  void loadGiveToMeFromDb();
//  void loadGiveToOtherFromDb();
//
//  void timer(DWORD curTime);
//  void sendAllTradeLog(Cmd::MyTradeLogRecordTradeCmd& cmd);
//  void updateTradeLog(const Cmd::UpdateTradeLogCmd& rev);
//
//  void addBuyedLog(QWORD id);
//  void addSelledLog(QWORD id);
//
//  void dataFilter(Cmd::LogItemInfo& info);
//  void sendNewLogToClient(Cmd::LogItemInfo& info);
//
//  bool fetchMyTradeLogList(Cmd::MyTradeLogRecordTradeCmd& rev);
//  bool takeLog(Cmd::TakeLogCmd& rev);
//  bool takeSceneRes(const Cmd::GetTradeLogSessionCmd& cmd);
//  void fetchNameInfo(Cmd::FetchNameInfoCmd& cmd);
//  void checkPublicityEnd();
//  bool sendCmdToWorldUser(QWORD charid, const void *buf, WORD len);
//
//  //赠送
//  bool give(Cmd::GiveTradeCmd& rev);
//  bool receiveGive(QWORD id);
//  bool giveSceneRes(Cmd::GiveCheckMoneySceneTradeCmd& rev);
//  bool acceptGive(AcceptTradeCmd& rev);
//  bool acceptGiveSceneRes(const Cmd::AddGiveItemSceneTradeCmd& rev);
//  bool refuseGive(RefuseTradeCmd& rev);
//  bool ntfGiveRes(const Cmd::NtfGiveStatusSceneTradeCmd& rev);
//  bool reqGiveItemInfo(Cmd::ReqGiveItemInfoCmd& rev);
//
//  void syncToScene(Cmd::SyncGiveItemSceneTradeCmd& cmd);
//
//  Cmd::LogItemInfo* getLogItemInfo(QWORD id, Cmd::EOperType logtype)
//  {
//    for (auto it = m_listLogItemInfo.begin(); it != m_listLogItemInfo.end(); ++it)
//    {
//      if (it->id() == id && it->logtype() == logtype)
//      {
//        return &(*it);
//      }
//    }
//    return nullptr;
//  }
//private:
//  std::string getTableName(Cmd::EOperType logType);
//  bool canTake(const Cmd::LogItemInfo& logInfo);
//  bool canGive(const Cmd::LogItemInfo& logInfo);
//  DWORD calcGiveFee(QWORD qwTotal, DWORD bg);
//
//  void addCanTakeCount();
//  void decCanTakeCount();
//  void sendCanTakeCountToClient();
//
//  void packBuyedItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo);
//  void packGiveItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo);
//  void packGiveToOtherItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo);
//  void packGiveToOtherItemInfo(const Cmd::LogItemInfo& logInfo, Cmd::GiveItemInfo& giveInfo);
//
//  bool isGiving();
//  //是否赠送过期
//  bool isGivingExpire(QWORD id, ETakeStatus status, DWORD expireTime);
//  void processGiveToOther(DWORD curSec);
//  inline bool lockGive();
//  inline void unlockGive();
//  bool processAccept(QWORD id);
//  bool processRefuse(QWORD id, bool isExpire = false);
//
//  Cmd::GiveItemInfo* getGiveItemInfo(QWORD id);
//  Cmd::GiveItemInfo* getGive2OtherItemInfo(QWORD id);
//  void retryAcceptGive();
//  void ntfClientToUpdateLog();
//private:
//  SessionUser* m_pUser = nullptr;
//  std::list<Cmd::LogItemInfo> m_listLogItemInfo;
//  DWORD m_canTakeCount = 0;     //
//  std::map<QWORD/*id*/, Cmd::GiveItemInfo> m_giveToMe;    //give to me
//  std::map<QWORD/*id*/, Cmd::GiveItemInfo> m_giveToOther;  //give to other
//
//  std::list<Cmd::GiveItemInfo> m_retryList;  //give to me retry
//  DWORD m_dwGiveLockTime = 0;
//};

#else

enum ETradeBuyStatus
{
  ETradeBuy_STATUS_PAY_SUCCESS              = 0,  //扣款成功;
  ETradeBuy_STATUS_PUBLICITY_PAY_SUCCESS    = 2,  //公示物品预先支付成功; 
  ETradeBuy_STATUS_PUBLICITY_SUCCESS        = 3,  //公示抢购成功，等待领取物品; 
  ETradeBuy_STATUS_PUBLICITY_CANCEL         = 4,  //公示订单没有抢购到，无效的订单，等待玩家退款; 
};

enum ETradeSellStatus
{
  ETradeSell_STATUS_PAY_SUCCESS             = 0,
  ETradeSell_STATUS_AUTO_SHELF              = 9,
};

struct CmpByValue1 {   //从大到小
  bool operator()(const Cmd::LogItemInfo& first, const Cmd::LogItemInfo& second)
  {
    if (first.tradetime() > second.tradetime())
      return true;
    return false;
  }
};

class SessionUser;
class TradeLog
{
  public:
    TradeLog(SessionUser* pUser);
    ~TradeLog();

    void onUserOnline();
    void onUserOffline();

    void loadGiveToMeFromDb(xRecordSet &set);
    void loadGiveToOtherFromDb(xRecordSet &set);

    void timer(DWORD curTime);
    void getLogListByType(const Cmd::ETradeType eType, std::list<Cmd::LogItemInfo>& list);
    void sendAllTradeLog(Cmd::MyTradeLogRecordTradeCmd& cmd);
    void updateTradeLog(const Cmd::UpdateTradeLogCmd& rev);

    void addBuyedLog(QWORD id);
    void addSelledLog(QWORD id);
    void addBoothBuyedLog(QWORD id);

    void dataFilter(Cmd::LogItemInfo& info);
    void sendNewLogToClient(Cmd::LogItemInfo& info);

    bool fetchMyTradeLogList(Cmd::MyTradeLogRecordTradeCmd& rev);
    bool takeLog(Cmd::TakeLogCmd& rev);
    bool takeSceneRes(const Cmd::GetTradeLogSessionCmd& cmd);
    void fetchNameInfo(Cmd::FetchNameInfoCmd& cmd);
    void checkPublicityEnd();
    bool sendCmdToWorldUser(QWORD charid, const void *buf, WORD len);

    //赠送
    bool give(Cmd::GiveTradeCmd& rev);
    bool receiveGive(QWORD id);
    bool giveTradeRes(Cmd::GiveCheckMoneySceneTradeCmd& rev);
    bool giveSceneRes(Cmd::GiveCheckMoneySceneTradeCmd& rev);
    bool acceptGive(AcceptTradeCmd& rev);
    bool acceptGiveSceneRes(const Cmd::AddGiveItemSceneTradeCmd& rev);
    bool refuseGive(RefuseTradeCmd& rev);
    bool ntfGiveRes(const Cmd::NtfGiveStatusSceneTradeCmd& rev);
    bool reqGiveItemInfo(Cmd::ReqGiveItemInfoCmd& rev);

    void syncToScene(Cmd::SyncGiveItemSceneTradeCmd& cmd);

    Cmd::LogItemInfo* getLogItemInfo(QWORD id, Cmd::EOperType logtype, const Cmd::ETradeType tradeType = Cmd::ETRADETYPE_TRADE)
    {
      for (auto it = m_listLogItemInfo.begin(); it != m_listLogItemInfo.end(); ++it)
      {
        if (it->id() == id && it->logtype() ==logtype && it->trade_type() == tradeType)
        {
          return &(*it);
        }
      }
      return nullptr;
    }
    
    EOperType status2LogType(ETradeBuyStatus eStatus);

    DWORD calcGiveFee(QWORD qwTotal, DWORD bg);
    void quickTakeLog(const Cmd::ETradeType tradeType = Cmd::ETRADETYPE_TRADE);
    void quickTakeSceneRes(const Cmd::GetTradeLogSessionCmd& rev);
    void sendQuickResToClient();

    // 请求扭蛋赠送，依赖交易的状态
    void reqLotteryGive(const LotteryGiveInfo& info);
    bool isGiving();
  private:
    std::string getTableName(Cmd::EOperType logType, const Cmd::ETradeType tradeType = Cmd::ETRADETYPE_TRADE);
    bool canTake(const Cmd::LogItemInfo& logInfo, bool log = false);
    bool canGive(const Cmd::LogItemInfo& logInfo);

    void addCanTakeCount();
    void decCanTakeCount();
    void addCanTakeBoothCount();
    void decCanTakeBoothCount();
    void sendCanTakeCountToClient(const Cmd::ETradeType tradeType = Cmd::ETRADETYPE_TRADE);   

    //是否赠送过期
    bool isGivingExpire(QWORD id, ETakeStatus status, DWORD expireTime);
    void processGiveToOther(DWORD curSec);
    inline bool lockGive();
    inline void unlockGive();
    bool processAccept(QWORD id);
    bool processRefuse(QWORD id, bool isExpire = false);

    Cmd::GiveItemInfo* getGiveItemInfo(QWORD id);
    Cmd::GiveItemInfo* getGive2OtherItemInfo(QWORD id);
    void retryAcceptGive();
    void ntfClientToUpdateLog();
    //填充赠送信息
    void fillGiftInfo(Cmd::LogItemInfo& logInfo);
    void packBoothSellItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo);
    void packBoothBuyItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo);
    void packSellItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo);
    void packBuyedItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo);
    void packGiveItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo);
    void packGiveToOtherItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo);
    void packGiveToOtherItemInfo(const Cmd::LogItemInfo& logInfo, Cmd::GiveItemInfo& giveInfo);
    bool isBuy(EOperType logType);
    bool forwardCmdToTrade(const void* data, unsigned short len);
    QWORD getTakeMoney(const Cmd::LogItemInfo& logInfo);
  private:
    SessionUser* m_pUser = nullptr;
    std::list<Cmd::LogItemInfo> m_listLogItemInfo;
    DWORD m_canTakeCount = 0;     //交易所可领取数量
    DWORD m_canTakeBoothCount = 0;     //摆摊可领取数量
    std::map<QWORD/*id*/, Cmd::GiveItemInfo> m_giveToMe;    //give to me
    std::map<QWORD/*id*/, Cmd::GiveItemInfo> m_giveToOther;  //give to other

    std::list<Cmd::GiveItemInfo> m_retryList;  //give to me retry
    DWORD m_dwGiveLockTime = 0;
    DWORD m_dwNextLogTime = 0;    //交易记录保护时间
    
    //快速领取
    DWORD m_dwQuckTakeTime = 0;
    QWORD m_qwTotalZeny = 0;
    QWORD m_qwTotalZenyFail = 0;
    DWORD m_dwItemCount = 0;
    DWORD m_dwItemCountFail = 0;
    std::list<std::pair<QWORD, Cmd::EOperType>> m_listQuickTake;

    bool m_blTradeInit = false;
    bool m_blNeedSyncTradeToScene = false;
};
#endif // !_OLD_TRADE
