#pragma once
#include "xDefine.h"
#include <list>
#include "SessionCmd.pb.h"
#include "xDBFields.h"
#include "ItemConfig.h"
#include "AuctionCCmd.pb.h"

#define DB_TABLE_AUCTION_CONFIG       "auction_config"
#define DB_TABLE_AUCTION_ITEM         "auction_item"
#define DB_TABLE_AUCTION_ITEM_SIGNUP  "auction_item_signup"
#define DB_TABLE_AUCTION_OFFERPRICE   "auction_offerprice"
#define DB_TABLE_AUCTION_EVENT        "auction_event"

struct CmpByTime {   //从大到小
  bool operator()(const Cmd::AuctionRecord& first, const Cmd::AuctionRecord& second)
  {
    if (first.time() > second.time())
      return true;
    return false;
  }
};

class SessionUser;
class Auction
{
  public:
    Auction(SessionUser* pUser);
    ~Auction();

    void sendAllRecord(Cmd::ReqAuctionRecordCCmd& cmd);
    bool reqAuctionRecord(Cmd::ReqAuctionRecordCCmd& rev);
    void onUserOnline();
    void reqMyTradedPrice(Cmd::ReqMyTradedPriceCCmd& rev);
  private:
    /*报名上架记录*/
    void packSignupRecord(const xRecord& r, Cmd::AuctionRecord& info);
    /*竞拍记录*/
    void packOfferPriceRecord(const xRecord& r, Cmd::AuctionRecord& info);
    bool canTake(const Cmd::AuctionRecord& logInfo);
    void sendCanTakeCountToClient();
    void sendRedTip();
  private:
    SessionUser* m_pUser = nullptr;
    std::list<Cmd::AuctionRecord> m_listLogItemInfo;
    DWORD m_canTakeCount = 0;     //

    DWORD m_dwNextLogTime = 0;    //交易记录保护时间
    std::map <QWORD/*batchid*/, std::map<DWORD/*itemid*/, QWORD/*price*/>> m_mapTradePrice; //历史出价
};

class AuctionMgr:public xSingleton<AuctionMgr>
{
public:
  AuctionMgr();
  ~AuctionMgr();

  void openPanel(SessionUser *pUser);
  void closePanel(SessionUser* pUser);
  void sendCmd2OpenUser(const void* data, WORD len);

private:
  std::set<SessionUser*> m_setOpenUser;
};
