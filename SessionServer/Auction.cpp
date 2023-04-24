#include "TradeLog.h"
#include "SessionUser.h"
#include "RewardConfig.h"
#include "SessionServer.h"
#include "GuidManager.h"
#include "SceneTrade.pb.h"
#include "PlatLogManager.h"
#include "MsgManager.h"
#include "LuaManager.h"
#include "SceneTip.pb.h"

// Auction
Auction::Auction(SessionUser* pUser) : m_pUser(pUser)
{
}

Auction::~Auction()
{
}

void Auction::sendAllRecord(Cmd::ReqAuctionRecordCCmd& cmd)
{
  if (m_pUser == nullptr)
    return;
  
  DWORD maxPageCnt = MiscConfig::getMe().getAuctionMiscCFG().dwRecordPageCnt;

  DWORD page_count = m_listLogItemInfo.size() / maxPageCnt;
  if (m_listLogItemInfo.size() % maxPageCnt != 0)
    page_count++;

  cmd.set_total_page_cnt(page_count);
  
  //索引超过最大页显示当前页
  if (cmd.index() >= page_count)
  {
    if (page_count == 0)
      cmd.set_index(0);
    else
      cmd.set_index(page_count - 1);
  }

  DWORD i = 0;
  DWORD start = cmd.index() * maxPageCnt;
  DWORD end = (cmd.index() + 1) * maxPageCnt;
  do 
  {
    //超过索引 不处理了
    if (start >= m_listLogItemInfo.size())
      break;
    //[0, 1)
    for (auto& v : m_listLogItemInfo)
    {
      if (i >= start && i < end)
      {
        AuctionRecord* pLogItemInfo = cmd.add_records();
        if (pLogItemInfo != nullptr)
        {
          pLogItemInfo->CopyFrom(v);
        }
      }
      i++;
      if (i >= end)
        break;
    }
  } while (0);
  
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XDBG << "[拍卖行-我的交易记录] 请求返回：" << cmd.ShortDebugString() << XEND;
  XINF << "[拍卖行-我的交易记录] 请求返回：" << m_pUser->id <<"index"<<cmd.index() <<"total count"<<cmd.total_page_cnt() << XEND;
  sendCanTakeCountToClient();
}

//获取我的拍卖记录
bool Auction::reqAuctionRecord(Cmd::ReqAuctionRecordCCmd& rev)
{
  DWORD curSec = now();
  if (m_dwNextLogTime && m_dwNextLogTime > curSec)
    return true;

  m_dwNextLogTime = curSec + 2;     //访问间隔2秒

  QWORD charId = m_pUser->id;
  
  //点击就请求
  m_listLogItemInfo.clear();

  if (!m_listLogItemInfo.empty())
  {
    sendAllRecord(rev);
    return true;
  }
  
  DWORD delTime = curSec - MiscConfig::getMe().getAuctionMiscCFG().dwReceiveTime;   //30天前的清除掉

  m_canTakeCount = 0;

  //报名记录
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
    if (!field)
      return false;//

    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "charid=%llu ", charId);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[拍卖行-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_AUCTION_ITEM_SIGNUP << XEND;
      return false;
    }
    for (QWORD i = 0; i < ret; ++i)
    {
      if (set[i].get<DWORD>("time") < delTime)
        continue;

      AuctionRecord recordInfo;
      packSignupRecord(set[i], recordInfo);
      
      if (canTake(recordInfo))
        m_canTakeCount++;
      m_listLogItemInfo.push_back(recordInfo);
    }
  }

  //竞拍购买记录
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
    if (!field)
      return false;//
    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "charid=%llu", charId);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[拍卖行-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_AUCTION_OFFERPRICE << XEND;
      return false;
    }

    for (QWORD i = 0; i < ret; ++i)
    {
      if (set[i].get<DWORD>("time") < delTime)
        continue;

      AuctionRecord recordInfo;
      packOfferPriceRecord(set[i], recordInfo);

      if (canTake(recordInfo))
        m_canTakeCount++;
      m_listLogItemInfo.push_back(recordInfo);
    }
  } 
  
  //order by time
  m_listLogItemInfo.sort(CmpByTime());

  sendAllRecord(rev);
  return true;
}

void Auction::onUserOnline()
{
  //只做红点提示用
  auto func = [&](const string& tableName)
  {
    xField field(REGION_DB, tableName.c_str());
    field.m_list["id"] = MYSQL_TYPE_LONG;

    char table[64];
    bzero(table, sizeof(table));
    snprintf(table, sizeof(table), "%s.%s", field.m_strDatabase.c_str(), field.m_strTable.c_str());
    char sqlStr[1024];
    bzero(sqlStr, sizeof(sqlStr));

    snprintf(sqlStr, sizeof(sqlStr), "select `id` from %s where charid=%llu and take_status = %u",
      table, m_pUser->id, EAuctionTakeStatus_CanTake);

    std::string sql2(sqlStr);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, set, sql2);
    if (ret == QWORD_MAX)
    {
      return false;
    }
    
    if (ret > 0)
    {
      sendRedTip();
      return true;
    }
    return false;
  };
  
  //上架记录
  if (func(DB_TABLE_AUCTION_ITEM_SIGNUP))
    return;
  //竞拍购买记录
  func(DB_TABLE_AUCTION_OFFERPRICE);

}

bool Auction::canTake(const Cmd::AuctionRecord& logInfo)
{
  if (logInfo.take_status() == EAuctionTakeStatus_CanTake) 
    return true;
  return false;
}

void Auction::reqMyTradedPrice(Cmd::ReqMyTradedPriceCCmd& rev)
{
  //竞拍购买记录
  QWORD qwMaxPrice = 0;
  bool bFind = false;
  auto it1 = m_mapTradePrice.find(rev.batchid());
  if (it1 != m_mapTradePrice.end())
  {
    auto it2 = it1->second.find(rev.itemid());
    if (it2 != it1->second.end())
    {
      qwMaxPrice = it2->second;
      bFind = true;
    }
  }

  if (!bFind)
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
    if (!field)
      return ;//
    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "batchid=%lu and itemid=%u and signup_id=%lu and charid=%llu", rev.batchid(), rev.itemid(), rev.signup_id(), m_pUser->id);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[拍卖行-数据库] 数据库错误 charid：" << m_pUser->id << "table: " << DB_TABLE_AUCTION_OFFERPRICE << XEND;
      return ;
    }

    QWORD qwPrice = 0;
    for (QWORD i = 0; i < ret; ++i)
    {
      qwPrice = set[i].get<QWORD>("price");
      if (qwPrice > qwMaxPrice)
        qwMaxPrice = qwPrice;
    }
    m_mapTradePrice[rev.batchid()][rev.itemid()] = qwMaxPrice;
  }

  rev.set_my_price(qwMaxPrice);
  PROTOBUF(rev, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[拍卖行-请求历史竞拍出价] charid" << m_pUser->id << "batchid" << rev.batchid() << "itemid" << rev.itemid() << "price" << qwMaxPrice << XEND;
}

/*报名上架记录*/
void Auction::packSignupRecord(const xRecord& r, Cmd::AuctionRecord& info)
{
  info.set_type(ERecordType(r.get<DWORD>("type")));
  info.set_id(r.get<QWORD>("id"));
  info.set_batchid(r.get<QWORD>("batchid"));
  info.set_itemid(r.get<DWORD>("itemid"));
  info.set_take_status(EAuctionTakeStatus(r.get<DWORD>("take_status")));
  info.set_time(r.get<DWORD>("time"));
  info.set_signup_id(r.get<QWORD>("signup_id"));
  std::string strItemData;
  strItemData.assign((const char*)r.getBin("itemdata"), r.getBinSize("itemdata"));
  ItemData* pData = info.mutable_itemdata();
  if(!pData)
  {
    XERR << "[拍卖行-上架记录] get ItemData failed! signup_id：" << info.signup_id() << XEND;
    return;
  }
  pData->ParseFromString(strItemData);
  if (info.type() == ERecordType_SellSucess || 
    info.type() == ERecordType_SellSucessPass ||
    info.type() == ERecordType_SellSucessNoPass)
  {
    info.set_buyer(r.getString("buyer_name"));
    info.set_zoneid(r.get<DWORD>("buyer_zoneid"));
    info.set_get_money(r.get<QWORD>("earn"));     //税后出售获得的钱
    info.set_tax(r.get<QWORD>("tax"));            //扣税
  }
}
/*竞拍记录*/
void Auction::packOfferPriceRecord(const xRecord& r, Cmd::AuctionRecord& info)
{
  info.set_type(ERecordType(r.get<DWORD>("type")));
  info.set_id(r.get<QWORD>("id"));
  info.set_batchid(r.get<QWORD>("batchid"));
  info.set_itemid(r.get<DWORD>("itemid"));
  info.set_take_status(EAuctionTakeStatus(r.get<DWORD>("take_status")));
  info.set_price(r.get<QWORD>("price"));
  info.set_time(r.get<DWORD>("time"));
  info.set_signup_id(r.get<QWORD>("signup_id"));
  if (info.type() == ERecordType_BuySuccess || 
    info.type() == ERecordType_BuySuccessPass || 
    info.type() == ERecordType_BuySuccessNoPass)
  {
    info.set_seller(r.getString("seller_name"));
    info.set_zoneid(r.get<DWORD>("seller_zoneid"));
    info.set_cost_money(r.get<QWORD>("price"));     //税后出售获得的钱
  }
}

void Auction::sendCanTakeCountToClient()
{
  if (m_canTakeCount)
    sendRedTip();
  NtfCanTakeCntCCmd cmd;
  cmd.set_count(m_canTakeCount);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[拍卖行-可领取数] charid" << m_pUser->id << "cnt" << m_canTakeCount << XEND;
}

void Auction::sendRedTip()
{
  GameTipCmd  tipCmd;
  RedTip* pRedTip = tipCmd.add_redtip();
  if (pRedTip)
  {
    pRedTip->set_redsys(EREDSYS_AUCTION_RECORD);
  }
  tipCmd.set_opt(ETIPOPT_UPDATE);
  PROTOBUF(tipCmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}


AuctionMgr::AuctionMgr()
{
}

AuctionMgr::~AuctionMgr()
{
}

void AuctionMgr::openPanel(SessionUser *pUser)
{
  if (!pUser) return;
  m_setOpenUser.insert(pUser);
}

void AuctionMgr::closePanel(SessionUser* pUser)
{
  if (!pUser) return;
  m_setOpenUser.erase(pUser);
}

void AuctionMgr::sendCmd2OpenUser(const void* data, WORD len)
{
  for (auto& s : m_setOpenUser)
  {
    if (!s)
      continue;
    s->sendCmdToMe(data, len);
  }
}
