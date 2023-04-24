#include "TradeLog.h"
#include "SessionUser.h"
#include "RewardConfig.h"
#include "SessionServer.h"
#include "GuidManager.h"
#include "SceneTrade.pb.h"
#include "PlatLogManager.h"
#include "MsgManager.h"
#include "LuaManager.h"
#include "NpcConfig.h"
#include "RedisManager.h"
#include "SessionThread.h"

#ifdef _OLD_TRADE
//
//// TradeLog
//TradeLog::TradeLog(SessionUser* pUser) : m_pUser(pUser)
//{
//}
//
//TradeLog::~TradeLog()
//{
//}
//
//void TradeLog::onUserOnline()
//{
//  loadGiveToMeFromDb();
//  loadGiveToOtherFromDb();
//}
//
//void TradeLog::onUserOffline()
//{
//  m_listLogItemInfo.clear();
//}
//
////one sec tick
//void TradeLog::timer(DWORD curTime)
//{
//  processGiveToOther(curTime);
//
//  retryAcceptGive();
//}
//
//void TradeLog::updateTradeLog(const Cmd::UpdateTradeLogCmd& rev)
//{
//  std::string tableName = getTableName(rev.type());
//
//  if (tableName == DB_TABLE_BUYED_LIST)
//  {
//    addBuyedLog(rev.id());
//  }
//  else if (tableName == DB_TABLE_SELLED_LIST)
//  {
//    addSelledLog(rev.id());
//  }
//  sendCanTakeCountToClient();
//}
//
//void TradeLog::addBuyedLog(QWORD id)
//{
//  //购买记录
//  xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//  if (!field)
//    return;//
//  char where[128];
//  bzero(where, sizeof(where));
//
//  snprintf(where, sizeof(where), "id=%llu ", id);
//  xRecordSet set;
//  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//  if (ret == QWORD_MAX)
//  {
//    XERR << "[交易-数据库] 数据库错误 charid：" << m_pUser->id << id << "table: " << DB_TABLE_BUYED_LIST << XEND;
//    return;
//  }
//  for (QWORD i = 0; i < ret; ++i)
//  {
//    LogItemInfo logInfo;
//    logInfo.set_id(set[i].get<QWORD>("id"));
//    logInfo.set_status(ETakeStatus(set[i].get<DWORD>("status")));
//    EOperType type = EOperType(set[i].get<DWORD>("logtype"));
//    logInfo.set_logtype(type);
//    logInfo.set_itemid(set[i].get<DWORD>("itemid"));
//    logInfo.set_refine_lv(set[i].get<DWORD>("refine_lv"));
//    logInfo.set_damage(set[i].get<DWORD>("damage"));
//    logInfo.set_tradetime(set[i].get<DWORD>("tradetime"));
//    logInfo.set_count(set[i].get<DWORD>("count"));
//    DWORD price = set[i].get<DWORD>("price");
//
//    logInfo.set_costmoney(price * logInfo.count());   //购买花费
//    logInfo.set_failcount(set[i].get<DWORD>("failcount"));
//    logInfo.set_totalcount(set[i].get<DWORD>("totalcount"));
//    logInfo.set_retmoney(price * logInfo.failcount());
//    logInfo.set_endtime(set[i].get<DWORD>("endtime"));
//
//    std::string sellerInfo((const char *)set[i].getBin("sellerinfo"), set[i].getBinSize("sellerinfo"));
//    if (!sellerInfo.empty())
//      logInfo.mutable_name_list()->ParseFromString(sellerInfo);
//
//    {
//      std::string strItemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
//      if (!strItemData.empty())
//        logInfo.mutable_itemdata()->ParseFromString(strItemData);
//    }
//    //已经安装交易时间从大到小排序了
//    m_listLogItemInfo.push_front(logInfo);
//    if (canTake(logInfo))
//      addCanTakeCount();
//
//    XLOG << "[交易-购买记录-新增]" << m_pUser->id << id;
//    sendNewLogToClient(logInfo);
//  }
//}
//
//void TradeLog::addSelledLog(QWORD id)
//{
//  //出售记录
//  xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_SELLED_LIST);
//  if (!field)
//    return;//
//  char where[128];
//  bzero(where, sizeof(where));
//
//  snprintf(where, sizeof(where), "id=%llu", id);
//  xRecordSet set;
//  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//  if (ret == QWORD_MAX)
//  {
//    XERR << "[交易-数据库] 数据库错误 charid：" << m_pUser->id << id << "table: " << DB_TABLE_SELLED_LIST << XEND;
//    return;
//  }
//
//  for (QWORD i = 0; i < ret; ++i)
//  {
//    LogItemInfo logInfo;
//    logInfo.set_id(set[i].get<QWORD>("id"));
//    logInfo.set_status(ETakeStatus(set[i].get<DWORD>("status")));
//    EOperType type = EOperType(set[i].get<DWORD>("logtype"));
//    logInfo.set_logtype(type);
//    logInfo.set_itemid(set[i].get<DWORD>("itemid"));
//    logInfo.set_refine_lv(set[i].get<DWORD>("refine_lv"));
//    logInfo.set_damage(set[i].get<DWORD>("damage"));
//    logInfo.set_tradetime(set[i].get<DWORD>("tradetime"));
//    logInfo.set_count(set[i].get<DWORD>("count"));
//    DWORD price = set[i].get<DWORD>("price");
//
//    logInfo.set_tax(set[i].get<DWORD>("tax"));   //扣费
//    DWORD getMoney = price * logInfo.count() - logInfo.tax();
//    logInfo.set_getmoney(getMoney);
//
//    std::string buyerInfo((const char *)set[i].getBin("buyerinfo"), set[i].getBinSize("buyerinfo"));
//    if (!buyerInfo.empty())
//      logInfo.mutable_name_list()->ParseFromString(buyerInfo);
//    {
//      std::string strItemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
//      if (!strItemData.empty())
//        logInfo.mutable_itemdata()->ParseFromString(strItemData);
//    }
//
//    //已经安装交易时间从大到小排序了
//    m_listLogItemInfo.push_front(logInfo);
//    if (canTake(logInfo))
//      addCanTakeCount();
//    XLOG << "[交易-出售记录-新增]" << m_pUser->id << id;
//    sendNewLogToClient(logInfo);
//  }
//}
//
//void TradeLog::dataFilter(LogItemInfo& info)
//{
//  info.mutable_itemdata()->Clear();
//
//  for (int i = 0; i < info.name_list().name_infos_size(); ++i)
//  {
//    info.mutable_name_info()->CopyFrom(info.name_list().name_infos(i)); //展示一个玩家
//    break;
//  }
//
//  if (info.name_list().name_infos_size() > 1)
//    info.set_is_many_people(true);
//  else
//    info.set_is_many_people(false);
//
//  info.mutable_name_list()->Clear();
//}
//
//void TradeLog::sendNewLogToClient(LogItemInfo& info)
//{
//  dataFilter(info);
//  AddNewLog cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.mutable_log()->CopyFrom(info);
//
//  DWORD page_count = m_listLogItemInfo.size() / MiscConfig::getMe().getTradeCFG().dwPageNumber;
//  if (m_listLogItemInfo.size() % MiscConfig::getMe().getTradeCFG().dwPageNumber != 0)
//    page_count++;
//
//  cmd.set_total_page_count(page_count);
//  PROTOBUF(cmd, send, len);
//
//  m_pUser->sendCmdToMe(send, len);
//}
//
//void TradeLog::sendAllTradeLog(Cmd::MyTradeLogRecordTradeCmd& cmd)
//{
//  if (m_pUser == nullptr)
//    return;
//
//  checkPublicityEnd();
//
//  DWORD page_count = m_listLogItemInfo.size() / MiscConfig::getMe().getTradeCFG().dwPageNumber;
//  if (m_listLogItemInfo.size() % MiscConfig::getMe().getTradeCFG().dwPageNumber != 0)
//    page_count++;
//
//  cmd.set_total_page_count(page_count);
//
//  //索引超过最大页显示当前页
//  if (cmd.index() >= page_count)
//  {
//    if (page_count == 0)
//      cmd.set_index(0);
//    else
//      cmd.set_index(page_count - 1);
//  }
//
//  DWORD i = 0;
//  DWORD start = cmd.index() * MiscConfig::getMe().getTradeCFG().dwPageNumber;
//  DWORD end = (cmd.index() + 1) * MiscConfig::getMe().getTradeCFG().dwPageNumber;
//
//  do
//  {
//    //超过索引 不处理了
//    if (start >= m_listLogItemInfo.size())
//    {
//      break;
//    }
//
//    //[0, 1)
//    for (auto& v : m_listLogItemInfo)
//    {
//      if (i >= start && i < end)
//      {
//        LogItemInfo* pLogItemInfo = cmd.add_log_list();
//        if (pLogItemInfo != nullptr)
//        {
//          LogItemInfo logItemInfo = v;
//          dataFilter(logItemInfo);
//          pLogItemInfo->CopyFrom(logItemInfo);
//          XDBG << "wld" << i << "time" << logItemInfo.tradetime() << XEND;
//        }
//      }
//      i++;
//      if (i >= end)
//        break;
//    }
//
//  } while (0);
//
//  PROTOBUF(cmd, send, len);
//  m_pUser->sendCmdToMe(send, len);
//
//  XDBG << "[交易-我的交易记录] 请求返回：" << cmd.ShortDebugString() << XEND;
//  XINF << "[交易-我的交易记录] 请求返回：" << m_pUser->id << "index" << cmd.index() << "total count" << cmd.total_page_count() << XEND;
//  sendCanTakeCountToClient();
//}
//
////获取我的购买出售记录
//bool TradeLog::fetchMyTradeLogList(Cmd::MyTradeLogRecordTradeCmd& rev)
//{
//  QWORD charId = m_pUser->id;
//
//  //点击就请求
//  m_listLogItemInfo.clear();
//
//  if (!m_listLogItemInfo.empty())
//  {
//    sendAllTradeLog(rev);
//    return true;
//  }
//
//  DWORD curSec = now();
//  DWORD delTime = curSec - 30 * DAY_T;   //30天前未领取的清除掉
//
//  m_canTakeCount = 0;
//
//  //购买记录
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//    if (!field)
//      return false;//
//    char where[128];
//    bzero(where, sizeof(where));
//
//    snprintf(where, sizeof(where), "buyerid=%llu ", charId);
//    xRecordSet set;
//    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//    if (ret == QWORD_MAX)
//    {
//      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_BUYED_LIST << XEND;
//      return false;
//    }
//    for (QWORD i = 0; i < ret; ++i)
//    {
//      LogItemInfo logInfo;
//      packBuyedItemInfo(set[i], logInfo);
//
//      //30天未领取
//      if (logInfo.tradetime() <= delTime)
//      {
//        continue;
//      }
//      //公示期购买结束了
//      if (logInfo.logtype() == EOperType_PublicityBuying  && logInfo.endtime() <= curSec)
//      {
//        continue;
//      }
//
//      if (canTake(logInfo))
//        addCanTakeCount();
//
//      m_listLogItemInfo.push_back(logInfo);
//    }
//  }
//
//  //出售记录
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_SELLED_LIST);
//    if (!field)
//      return false;//
//    char where[128];
//    bzero(where, sizeof(where));
//
//    snprintf(where, sizeof(where), "sellerid=%llu", charId);
//    xRecordSet set;
//    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//    if (ret == QWORD_MAX)
//    {
//      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_SELLED_LIST << XEND;
//      return false;
//    }
//
//    for (QWORD i = 0; i < ret; ++i)
//    {
//      if (set[i].get<DWORD>("tradetime") <= delTime)
//      {
//        continue;
//      }
//
//      LogItemInfo logInfo;
//      logInfo.set_id(set[i].get<QWORD>("id"));
//      logInfo.set_status(ETakeStatus(set[i].get<DWORD>("status")));
//      EOperType type = EOperType(set[i].get<DWORD>("logtype"));
//      logInfo.set_logtype(type);
//      logInfo.set_itemid(set[i].get<DWORD>("itemid"));
//      logInfo.set_refine_lv(set[i].get<DWORD>("refine_lv"));
//      logInfo.set_damage(set[i].get<DWORD>("damage"));
//      logInfo.set_tradetime(set[i].get<DWORD>("tradetime"));
//      logInfo.set_count(set[i].get<DWORD>("count"));
//      DWORD price = set[i].get<DWORD>("price");
//      logInfo.set_price(price);
//      std::string buyerInfo((const char *)set[i].getBin("buyerinfo"), set[i].getBinSize("buyerinfo"));
//      if (!buyerInfo.empty())
//        logInfo.mutable_name_list()->ParseFromString(buyerInfo);
//
//      {
//        std::string strItemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
//        if (!strItemData.empty())
//          logInfo.mutable_itemdata()->ParseFromString(strItemData);
//      }
//      
//      if (type == EOperType_NormalSell || type == EOperType_PublicitySellSuccess)
//      {
//        logInfo.set_tax(set[i].get<DWORD>("tax"));   //扣费
//        DWORD getMoney = price * logInfo.count() - logInfo.tax();
//        logInfo.set_getmoney(getMoney);
//      }
//      else if (type == EOperType_AutoOffTheShelf)
//      {
//        logInfo.set_ret_cost(set[i].get<DWORD>("tax"));   //自动下架返还上架费
//      }
//
//      if (logInfo.status() == ETakeStatus_CanTakeGive)
//        m_canTakeCount++;
//      m_listLogItemInfo.push_back(logInfo);
//    }
//  }
//
//  //order by
//  m_listLogItemInfo.sort(CmpByValue1());
//
//  sendAllTradeLog(rev);
//  return true;
//}
//
//bool TradeLog::canTake(const Cmd::LogItemInfo& logInfo)
//{
//  if (logInfo.logtype() == EOperType_PublicityBuying)
//  {
//    XERR << "[交易-领取中] 公示期购买中的物品 不可领取" << m_pUser->id << logInfo.id() << logInfo.logtype() << XEND;
//    return false;
//  }
//
//  if (logInfo.status() == ETakeStatus_CanTakeGive)
//  {
//    return true;
//  }
//  XERR << "[交易-领取中] 物品处在正在不可领取的状态" << m_pUser->id << logInfo.id() << logInfo.logtype() << logInfo.status() << XEND;
//  return false;
//}
//
//bool TradeLog::canGive(const Cmd::LogItemInfo& logInfo)
//{
//  QWORD qwTotalPrice = logInfo.price()* logInfo.count();
//  if (qwTotalPrice < MiscConfig::getMe().getTradeCFG().dwSendMoneyLimit)
//  {
//    XINF << "[交易-赠送] 没到达赠送价值 " << logInfo.id() << logInfo.logtype() << qwTotalPrice << "limit" << MiscConfig::getMe().getTradeCFG().dwSendMoneyLimit << XEND;
//    return false;
//  }
//  if (logInfo.status() != ETakeStatus_CanTakeGive)
//  {
//    XERR << "[交易-赠送] 不可赠送的状态" << logInfo.id() << logInfo.logtype() << qwTotalPrice << "status" << logInfo.status() << XEND;
//    return false;
//  }
//  return true;
//}
//
//bool TradeLog::takeLog(Cmd::TakeLogCmd& rev)
//{
//  bool ret = true;
//  Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(rev.log().id(), rev.log().logtype());
//  do
//  {
//    if (pLogItemInfo == nullptr)
//    {
//      ret = false;
//      XERR << "[交易-领取中] 找不到 LogItemInfo charid" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << rev.ShortDebugString() << XEND;
//      break;
//    }
//
//    if (canTake(*pLogItemInfo) == false)
//    {
//      ret = false;
//      XERR << "[交易-领取中] 物品不可领取" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << pLogItemInfo->ShortDebugString() << XEND;
//      break;
//    }
//
//    //modify db
//    std::string tableName = getTableName(rev.log().logtype());
//    xField *field = xFieldsM::getMe().getField(REGION_DB, tableName);
//    if (!field)
//    {
//      ret = false;//
//      XERR << "[交易-领取中] 数据库错误" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << pLogItemInfo->ShortDebugString() << XEND;
//      break;
//    }
//    char where[128];
//    bzero(where, sizeof(where));
//
//    snprintf(where, sizeof(where), "id=%lu and status=%u", rev.log().id(), ETakeStatus_CanTakeGive);
//
//    xRecord record(field);
//    record.put("status", ETakeStatus_Taking);
//    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
//    if (ret != 1)
//    {
//      ret = false;//
//      XERR << "[交易-领取中] 数据库错误，修改订单状态出错" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << pLogItemInfo->ShortDebugString() << XEND;
//      break;
//    }
//    pLogItemInfo->set_status(ETakeStatus_Taking);
//
//    GetTradeLogSessionCmd cmd;
//    cmd.set_id(rev.log().id());
//    cmd.set_logtype(rev.log().logtype());
//
//    //出售 卖家给钱
//    if (rev.log().logtype() == EOperType_NormalSell || rev.log().logtype() == EOperType_PublicitySellSuccess)
//    {
//      ItemInfo* pItemInfo = cmd.mutable_item();
//      if (pItemInfo)
//      {
//        DWORD getMoney = pLogItemInfo->getmoney();
//        pItemInfo->set_id(100);
//        pItemInfo->set_count(getMoney);
//        pItemInfo->set_source(ESOURCE_TRADE);   
//      }
//      //出售记录 分享使用
//      cmd.set_sell_item_id(pLogItemInfo->itemid());
//      cmd.set_sell_price(pLogItemInfo->price());
//      cmd.set_sell_count(pLogItemInfo->count());
//      cmd.set_refine_lv(pLogItemInfo->refine_lv());
//    }
//
//    //公示期购买失败，买家返回钱
//    if (rev.log().logtype() == EOperType_PublicityBuyFail)
//    {
//      ItemInfo* pItemInfo = cmd.mutable_item();
//      if (pItemInfo)
//      {
//        pItemInfo->set_id(100);
//        pItemInfo->set_count(pLogItemInfo->retmoney());
//        pItemInfo->set_source(ESOURCE_TRADE_PUBLICITY_FAILRET);
//      }
//    }
//
//    //购买 买家加装备
//    if (rev.log().logtype() == EOperType_NoramlBuy || rev.log().logtype() == EOperType_PublicityBuySuccess)
//    {
//      if (pLogItemInfo->itemdata().has_base())
//      {
//        if (rev.log().logtype() == EOperType_NoramlBuy)
//          pLogItemInfo->mutable_itemdata()->mutable_base()->set_source(ESOURCE_TRADE);
//        else
//          pLogItemInfo->mutable_itemdata()->mutable_base()->set_source(ESOURCE_TRADE_PUBLICITY);
//
//        cmd.mutable_itemdata()->CopyFrom(pLogItemInfo->itemdata());
//      }
//      else
//      {
//        ItemInfo* pItemInfo = cmd.mutable_item();
//        if (pItemInfo)
//        {
//          pItemInfo->set_id(pLogItemInfo->itemid());
//          pItemInfo->set_count(pLogItemInfo->count());
//          pItemInfo->set_source(ESOURCE_TRADE);
//        }
//      }
//    }
//
//    //自动下架，返还上架费和道具
//    if (rev.log().logtype() == EOperType_AutoOffTheShelf)
//    {
//      if (pLogItemInfo->itemdata().has_base())
//      {
//        pLogItemInfo->mutable_itemdata()->mutable_base()->set_source(ESOURCE_TRADE);
//        cmd.mutable_itemdata()->CopyFrom(pLogItemInfo->itemdata());
//      }
//      else
//      {
//        ItemInfo* pItemInfo = cmd.mutable_item();
//        if (pItemInfo)
//        {
//          pItemInfo->set_id(pLogItemInfo->itemid());
//          pItemInfo->set_count(pLogItemInfo->count());
//          pItemInfo->set_source(ESOURCE_TRADE);
//        }
//      }
//      cmd.set_ret_cost(pLogItemInfo->ret_cost());
//    }
//
//    cmd.set_charid(m_pUser->id);
//    PROTOBUF(cmd, send, len);
//    m_pUser->sendCmdToScene(send, len);
//
//    //platlog
//    {
//      ETRADE_TYPE tradeType = ETRADETYPE_TAKE;
//
//      switch (rev.log().logtype())
//      {
//      case EOperType_NormalSell:
//        tradeType = ETRADETYPE_TAKE_SELL_MONEY;
//        break;
//      case EOperType_PublicitySellSuccess:
//        tradeType = ETRADETYPE_TAKE_PUBLICITY_SELL_MONEY;
//        break;
//      case EOperType_PublicityBuyFail:
//        tradeType = ETRADETYPE_TAKE_RETURN_MONEY;
//        break;
//      case EOperType_NoramlBuy:
//        tradeType = ETRADETYPE_TAKE_BUY_ITEM;
//        break;
//      case EOperType_PublicityBuySuccess:
//        tradeType = ETRADETYPE_TAKE_PUBLICITY_BUY_ITEM;
//        break;
//      case EOperType_AutoOffTheShelf:
//        tradeType = ETRADETYPE_AUTO_OFFSHELF;
//        break;
//      default:
//        break;
//      }
//
//      string jsonStr = pb2json(pLogItemInfo->itemdata());
//      PlatLogManager::getMe().TradeLog(thisServer,
//        thisServer->getPlatformID(),
//        m_pUser->id,
//        tradeType,
//        pLogItemInfo->itemid(),
//        pLogItemInfo->count(),
//        pLogItemInfo->price(),
//        pLogItemInfo->tax(),
//        pLogItemInfo->getmoney(),
//        jsonStr);
//    }
//
//
//  } while (0);
//
//  if (pLogItemInfo)
//    XLOG << "[交易-领取中] charid" << m_pUser->id << "ret" << (ret ? "成功" : "失败") << rev.log().id() << rev.log().logtype() << "msg" << pLogItemInfo->ShortDebugString() << XEND;
//  else
//    XLOG << "[交易-领取中] 出错，找不到 LogItemInfo charid" << m_pUser->id << "ret" << ret << rev.log().id() << rev.log().logtype() << XEND;
//  return ret;
//}
//
//bool TradeLog::takeSceneRes(const GetTradeLogSessionCmd& cmd)
//{
//  bool ret = false;
//
//  do
//  {
//    ETakeStatus newStatus = ETakeStatus_Took;
//    if (cmd.success())
//    {
//      newStatus = ETakeStatus_Took;
//      ret = true;
//      //dec
//      decCanTakeCount();
//    }
//    else
//    {
//      newStatus = ETakeStatus_CanTakeGive;
//      ret = false;
//    }
//
//    //modify db
//    std::string tableName = getTableName(cmd.logtype());
//    xField *field = xFieldsM::getMe().getField(REGION_DB, tableName);
//    if (!field)
//    {
//      XERR << "[交易-领取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
//        << "还原状态失败。数据库找不到" << "newstatus" << newStatus << cmd.ShortDebugString() << XEND;
//      ret = false;
//      break;
//    }
//    char where[128];
//    bzero(where, sizeof(where));
//
//    snprintf(where, sizeof(where), "id=%lu and status=%u", cmd.id(), ETakeStatus_Taking);
//
//    xRecord record(field);
//    record.put("status", newStatus);
//    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
//    if (ret != 1)
//    {
//      XERR << "[交易-领取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
//        << "还原状态失败。数据库插入失败" << "newstatus" << newStatus << cmd.ShortDebugString() << XEND;
//      ret = false;
//      break;
//    }
//
//    Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(cmd.id(), cmd.logtype());
//    if (pLogItemInfo)
//    {
//      pLogItemInfo->set_status(newStatus);
//    }
//
//    XLOG << "[交易-领取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
//      << "成功还原数据库状态" << "newstatus" << newStatus << cmd.ShortDebugString() << XEND;
//  } while (0);
//
//  TakeLogCmd sendCmd;
//  sendCmd.mutable_log()->set_id(cmd.id());
//  sendCmd.mutable_log()->set_logtype(cmd.logtype());
//  sendCmd.set_success(ret);
//  PROTOBUF(sendCmd, send, len);
//  m_pUser->sendCmdToMe(send, len);
//
//  sendCanTakeCountToClient();
//
//  return true;
//}
//
//void TradeLog::fetchNameInfo(Cmd::FetchNameInfoCmd& cmd)
//{
//  do
//  {
//    LogItemInfo* pItemInfo = getLogItemInfo(cmd.id(), cmd.type());
//    if (pItemInfo == nullptr)
//      break;
//
//    const NameInfoList& rNameList = pItemInfo->name_list();
//
//    DWORD allSize = rNameList.name_infos_size();
//
//    DWORD page_count = allSize / MiscConfig::getMe().getTradeCFG().dwPageNumber;
//    if (allSize % MiscConfig::getMe().getTradeCFG().dwPageNumber != 0)
//      page_count++;
//
//    cmd.set_total_page_count(page_count);
//
//    DWORD start = cmd.index() * MiscConfig::getMe().getTradeCFG().dwPageNumber;
//    DWORD end = (cmd.index() + 1) * MiscConfig::getMe().getTradeCFG().dwPageNumber;
//
//    //超过索引 不处理了
//    if (start >= allSize)
//      return;
//
//    //[0, 1)
//    for (DWORD i = 0; i < allSize; ++i)
//    {
//      if (i >= end)
//        break;
//      if (i >= start && i < end)
//      {
//        cmd.mutable_name_list()->add_name_infos()->CopyFrom(rNameList.name_infos(i));
//      }
//    }
//  } while (0);
//
//  PROTOBUF(cmd, send, len);
//  m_pUser->sendCmdToMe(send, len);
//  XLOG << "[交易-查询名字]" << m_pUser->id << cmd.ShortDebugString() << XEND;
//}
//
//bool TradeLog::give(Cmd::GiveTradeCmd& rev)
//{
//  if (!m_pUser)
//    return false;
//
//  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_SEND) == true)
//  {
//    XERR << "[交易-赠送] 赠送功能未开启" << m_pUser->id << XEND;
//    return true;
//  }
//  if (!m_pUser->m_bSafeDevice)
//  {
//    XERR << "[交易-赠送] 不是安全设备 charid" << m_pUser->id<<"safedevice"<<m_pUser->m_bSafeDevice << XEND;
//    return false;
//  }
//
//  if (lockGive() == false)
//    return false;
//
//  std::string tableName = getTableName(rev.logtype());
//  if (tableName != DB_TABLE_BUYED_LIST)
//  {
//    XERR << "[交易-赠送] 非法的tablename" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
//    unlockGive();
//    return false;
//  }
//
//  Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(rev.id(), rev.logtype());
//  if (pLogItemInfo == nullptr)
//  {
//    unlockGive();
//    XERR << "[交易-赠送] 找不到 LogItemInfo charid" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
//    return false;
//  }
//
//  if (canGive(*pLogItemInfo) == false)
//  {
//    unlockGive();
//    XERR << "[交易-赠送] 物品不可赠送" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
//    return false;
//  }
//
//  DWORD startTime = BaseConfig::getMe().getTradeGiveStartTime();
//  if (pLogItemInfo->tradetime() <= startTime)
//  {
//    MsgManager::sendMsg(m_pUser->id, 25008, MsgParams(BaseConfig::getMe().getStrTradeGiveTime()));
//    XERR << "[交易-赠送] 交易记录超过赠送开始时间" << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << "tradetime" << pLogItemInfo->tradetime() << "赠送开始时间" << startTime << XEND;
//    unlockGive();
//    return false;
//  }
//
//  DWORD limitTime = now() - MiscConfig::getMe().getTradeCFG().dwCantSendTime;
//  if (pLogItemInfo->tradetime() <= limitTime)
//  {
//    unlockGive();
//    MsgManager::sendMsg(m_pUser->id, 25004);
//    XERR << "[交易-赠送] 交易记录超过29天不可赠送" << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << "tradetime" << pLogItemInfo->tradetime() << "limittime" << limitTime << XEND;
//    return false;
//  }
//
//  // check only one
//  if (isGiving())
//  {
//    unlockGive();
//    //已有物品正在赠送，无法赠送msg
//    MsgManager::sendMsg(m_pUser->id, 25005);
//    XERR << "[交易-赠送] 其他物品正在赠送" << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
//    return false;
//  }
//
//  //check friend
//  //if (!m_pUser->getGCharData()->checkRelation(rev.friendid(), ESOCIALRELATION_FRIEND))
//  if (!m_pUser->getSocial().checkRelation(rev.friendid(), ESOCIALRELATION_FRIEND))
//  {
//    unlockGive();
//    XERR << "[交易-赠送] 失败，对方不是我的好友" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
//    return false;
//  }
//
//  GCharReader gCharReader(thisServer->getRegionID(), rev.friendid());
//  if (!gCharReader.get())
//  {
//    unlockGive();
//    XERR << "[交易-赠送] 失败，获取好友数据失败" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
//    return false;
//  }
//
//  //is online
//  if (gCharReader.getOnlineTime() < gCharReader.getOfflineTime())
//  {
//    unlockGive();
//    XERR << "[交易-赠送] 失败，好友不在线" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
//    return false;
//  }
//
//  DWORD fee = calcGiveFee(pLogItemInfo->quota(), rev.background());
//  Cmd::GiveCheckMoneySceneTradeCmd cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.set_friendid(rev.friendid());
//  cmd.set_type(rev.logtype());
//  cmd.set_id(rev.id());
//  cmd.set_quota(pLogItemInfo->quota());
//  cmd.set_fee(fee);
//  cmd.set_anonymous(rev.anonymous());
//  cmd.set_content(rev.content());
//  cmd.set_background(rev.background());
//
//  PROTOBUF(cmd, send, len);
//  if (m_pUser->sendCmdToScene(send, len) == false)
//  {
//    unlockGive();
//    XERR << "[交易-赠送] 发送赠送消息给场景失败" << m_pUser->id << "type" << cmd.type() << "赠送给" << cmd.friendid() << "id" << cmd.id() << "quota" << cmd.quota() << "fee" << cmd.fee() << XEND;
//    return false;
//  }
//
//  XLOG << "[交易-赠送] 发送赠送消息给场景成功" << m_pUser->id << "type" << cmd.type() << "赠送给" << cmd.friendid() << "id" << cmd.id() << "quota" << cmd.quota() << "fee" << cmd.fee() << XEND;
//  return true;
//}
//
//bool TradeLog::giveSceneRes(Cmd::GiveCheckMoneySceneTradeCmd& rev)
//{
//  unlockGive();
//
//  if (rev.ret() == false)
//  {
//    XERR << "[交易所-赠送-从场景扣除赠送返回] 场景检查不通过" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//    return false;
//  }
//
//  //success
//  Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(rev.id(), rev.type());
//  if (pLogItemInfo == nullptr)
//  {
//    XERR << "[交易-赠送-从场景扣除赠送返回] 找不到 LogItemInfo charid" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//    return false;
//  }
//
//  GCharReader gCharReader(thisServer->getRegionID(), rev.friendid());
//  if (!gCharReader.get())
//  {
//    XERR << "[交易-赠送-从场景扣除赠送返回] 找不到好友gchar数据" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//    return false;
//  }
//
//  ////is online
//  //if (gCharReader.getOnlineTime() < gCharReader.getOfflineTime())
//  //{
//  //  XERR << "[交易-赠送-从场景扣除赠送返回] 好友不在线" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//  //  return false;
//  //}
//
//  DWORD dwExpireTime = now() + MiscConfig::getMe().getTradeCFG().dwFollowTime;
//  pLogItemInfo->set_status(ETakeStatus_Giving);
//  pLogItemInfo->set_expiretime(dwExpireTime);
//
//  //modify db 
//  xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//  if (!field)
//  {
//    XERR << "[交易-赠送-从场景扣除赠送返回] 数据库错误" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//    return false;
//  }
//  char where[128];
//  bzero(where, sizeof(where));
//  snprintf(where, sizeof(where), "id=%lu and status=%u", rev.id(), ETakeStatus_CanTakeGive);
//  xRecord record(field);
//  record.put("status", ETakeStatus_Giving);
//  record.put("buyername", m_pUser->name);
//  record.put("receiveid", rev.friendid());
//  record.putString("receivername", gCharReader.getName());
//  record.put("receiverzoneid", gCharReader.getZoneID());
//  record.put("expiretime", dwExpireTime);
//  record.put("anonymous", rev.anonymous());
//  record.put("background", rev.background());
//  record.putString("content", rev.content());
//
//  QWORD r = thisServer->getTradeConnPool().exeUpdate(record, where);
//  if (r != 1)
//  {
//    XERR << "[交易-赠送-从场景扣除赠送返回] DB_TABLE_BUYED_LIST 修改数据失败" << m_pUser->accid << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//    return false;
//  }
//
//  //
//  MsgManager::sendMsg(m_pUser->id, 25007);
//
//  GiveItemInfo giveItemInfo;
//  packGiveToOtherItemInfo(*pLogItemInfo, giveItemInfo);
//  m_giveToOther[giveItemInfo.id()] = giveItemInfo;
//
//  //send cmd to friend
//  ReceiveGiveSceneTradeCmd cmd;
//  cmd.set_id(rev.id());
//  cmd.set_charid(rev.friendid());
//  PROTOBUF(cmd, send, len);
//  if (sendCmdToWorldUser(rev.friendid(), send, len) == false)
//  {
//    XERR << "[交易-赠送-从场景扣除赠送返回] 通知好友消息发送失败" << m_pUser->accid << m_pUser->id << "赠送" << rev.id() << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//    return false;
//  }
//  XLOG << "[交易-赠送-从场景扣除赠送返回] 通知好友消息发送成功" << m_pUser->accid << m_pUser->id << "赠送" << rev.id() << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
//
//  ntfClientToUpdateLog();
//  return true;
//}
//
//bool TradeLog::receiveGive(QWORD id)
//{
//  xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//  if (!field)
//    return false;//
//  char where[128];
//  bzero(where, sizeof(where));
//
//  snprintf(where, sizeof(where), "id=%llu and receiveid=%llu ", id, m_pUser->id);
//  xRecordSet set;
//  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//  if (ret != 1)
//  {
//    XERR << "[交易-赠送接收者] DB_TABLE_BUYED_LIST数据库错误 charid：" << m_pUser->id << "table: " << DB_TABLE_BUYED_LIST << XEND;
//    return false;
//  }
//
//  Cmd::GiveItemInfo giveInfo;
//  packGiveItemInfo(set[0], giveInfo);
//  m_giveToMe[giveInfo.id()] = giveInfo;
//
//  //send to scene
//  AddGiveSceneTradeCmd cmd;
//  cmd.set_charid(m_pUser->id);
//  GiveItemInfo* pInfo = cmd.mutable_iteminfo();
//  if (pInfo == nullptr)
//  {
//    XERR << "[交易-赠送接收者] protobuf协议添加失败" << m_pUser->id << "id" << giveInfo.id() << "from" << giveInfo.senderid() << XEND;
//    return false;
//  }
//  pInfo->set_id(giveInfo.id());
//  pInfo->set_expiretime(giveInfo.expiretime());
//  PROTOBUF(cmd, send, len);
//  if (m_pUser->sendCmdToScene(send, len) == false)
//  {
//    XERR << "[交易-赠送接收者] 发送通知到场景失败" << m_pUser->id << "id" << giveInfo.id() << "from" << giveInfo.senderid() << XEND;
//  }
//
//  //您好！你有一份赠礼送达，请在姜饼人处领取礼物。
//  DWORD npcId = 4364;
//  const SNpcCFG* pNpcCfg = NpcConfig::getMe().getNpcCFG(npcId);
//  if (pNpcCfg)
//  {
//    MsgParams params;
//    params.addString(pNpcCfg->strName);
//    MsgManager::sendMsg(m_pUser->id, 25100, params);
//  }
//
//  XLOG << "[交易-赠送接收者] 发送通知到场景成功" << m_pUser->id << "id" << giveInfo.id() << "from" << giveInfo.senderid() << "expitetime" << giveInfo.expiretime() << XEND;
//  return true;
//}
//
//bool TradeLog::reqGiveItemInfo(Cmd::ReqGiveItemInfoCmd& rev)
//{
//  QWORD id = rev.id();
//  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(rev.id());
//  if (pGiveItemInfo == nullptr)
//  {
//    XERR << "[交易-赠送接收者] 请求赠送信息，找不到赠送id" << m_pUser->id << id << "logid" << rev.id() << XEND;
//    return false;
//  }
//
//  if (pGiveItemInfo->status() != ETakeStatus_Giving)
//  {
//    XERR << "[交易-赠送接收者] 请求赠送信息，状态不对" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//    return false;
//  }
//
//  rev.mutable_iteminfo()->CopyFrom(*pGiveItemInfo);
//
//  PROTOBUF(rev, send, len);
//  if (m_pUser->sendCmdToMe(send, len) == false)
//  {
//    XERR << "[交易-赠送接收者] 请求赠送信息，发送消息给玩家失败" << m_pUser->id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//    return false;
//  }
//
//  XLOG << "[交易-赠送接收者] 请求赠送信息，发送消息给玩家场景" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << rev.ShortDebugString() << XEND;
//  return true;
//}
//
//bool TradeLog::acceptGive(AcceptTradeCmd& rev)
//{
//  QWORD id = rev.id();
//
//  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(id);
//  if (pGiveItemInfo == nullptr)
//  {
//    XERR << "[交易-赠送接收者] 接受赠送失败，找不到赠送id" << m_pUser->id << id << "logid" << rev.id() << XEND;
//    return false;
//  }
//
//  if (pGiveItemInfo->status() != ETakeStatus_Giving)
//  {
//    XERR << "[交易-赠送接收者] 接受赠送失败，状态不对" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//    return false;
//  }
//
//  QWORD logid = pGiveItemInfo->id();
//
//  //modify buyed db
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//    if (!field)
//      return false;//
//    char where[128];
//    bzero(where, sizeof(where));
//    snprintf(where, sizeof(where), "id=%llu and status=%u", logid, ETakeStatus_Giving);
//    xRecord record(field);
//    record.put("status", ETakeStatus_Give_Accepting);  //赠送领取中
//    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
//    if (ret != 1)
//    {
//      XERR << "[交易-赠送接收者] 接受赠送失败，数据库修改失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//      return false;
//    }
//    pGiveItemInfo->set_status(ETakeStatus_Give_Accepting);
//  }
//
//  //add item to scene
//  AddGiveItemSceneTradeCmd cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.set_id(id);
//  cmd.set_itemid(pGiveItemInfo->itemid());
//  cmd.set_count(pGiveItemInfo->count());
//  cmd.set_background(pGiveItemInfo->background());
//  cmd.mutable_itemdata()->CopyFrom(pGiveItemInfo->itemdata());
//  PROTOBUF(cmd, send, len);
//  if (m_pUser->sendCmdToScene(send, len) == false)
//  {
//    XERR << "[交易-赠送接收者] 接受赠送成功，发送消息给场景失败" << m_pUser->id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//    return false;
//  }
//
//  XLOG << "[交易-赠送接收者] 接受赠送成功，发送消息给场景" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//  return true;
//}
//
//bool TradeLog::acceptGiveSceneRes(const Cmd::AddGiveItemSceneTradeCmd& rev)
//{
//  if (rev.ret() == false)
//  {
//    XERR << "[交易-赠送接收者] 接受赠送场景返回，场景加道具失败等待后面重试" << m_pUser->id << rev.id() << "ret" << rev.ret() << rev.ShortDebugString() << XEND;
//    return false;
//  }
//
//  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(rev.id());
//  if (pGiveItemInfo == nullptr)
//  {
//    XERR << "[交易-赠送接收者] 接受赠送场景返回，找不到赠送id" << m_pUser->id << rev.id() << XEND;
//    return false;
//  }
//
//  //modify give db
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//    if (!field)
//      return false;//
//    char where[128];
//    bzero(where, sizeof(where));
//    snprintf(where, sizeof(where), "id=%lu and status=%u", rev.id(), ETakeStatus_Give_Accepting);
//    xRecord record(field);
//    record.put("status", ETakeStatus_Give_Accepted_1);  //赠送已经领取额度未扣
//    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
//    if (ret != 1)
//    {
//      XERR << "[交易-赠送接收者] 接受赠送场景返回，修改数据失败" << m_pUser->id << rev.id() << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//      return false;
//    }
//    pGiveItemInfo->set_status(ETakeStatus_Give_Accepted_1);
//  }
//
//  NtfGiveStatusSceneTradeCmd ntfCmd;
//  ntfCmd.set_charid(pGiveItemInfo->senderid());
//  ntfCmd.set_id(pGiveItemInfo->id());
//  ntfCmd.set_status(EGiveStatus_Accept);
//  ntfCmd.set_name(m_pUser->name);
//  PROTOBUF(ntfCmd, ntfSend, ntfLen);
//  if (sendCmdToWorldUser(pGiveItemInfo->senderid(), ntfSend, ntfLen) == false)
//    XERR << "[交易-赠送接收者] 接受赠送场景返回，转发消息给赠送者失败" << m_pUser->id << "赠送者" << pGiveItemInfo->senderid() << rev.id() << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//  else
//    XLOG << "[交易-赠送接收者] 接受赠送场景返回，转发消息给赠送者成功" << m_pUser->id << "赠送者" << pGiveItemInfo->senderid() << rev.id() << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//
//  //一定要放在打印后面
//  m_giveToMe.erase(rev.id());
//  return true;
//}
//
//bool TradeLog::refuseGive(RefuseTradeCmd& rev)
//{
//  QWORD id = rev.id();
//  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(id);
//  if (pGiveItemInfo == nullptr)
//  {
//    XERR << "[交易-赠送接收者] 拒绝赠送，找不到赠送id" << m_pUser->id << id << XEND;
//    return false;
//  }
//
//  if (pGiveItemInfo->status() != ETakeStatus_Giving)
//  {
//    XERR << "[交易-赠送接收者] 拒绝赠送，状态不对" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//    return false;
//  }
//
//  QWORD logid = pGiveItemInfo->id();
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//    if (!field)
//      return false;//
//    char where[128];
//    bzero(where, sizeof(where));
//    snprintf(where, sizeof(where), "id=%llu and status=%u", logid, ETakeStatus_Giving);
//    xRecord record(field);
//    record.put("status", ETakeStatus_CanTakeGive);  //领取成功
//    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
//    if (ret != 1)
//    {
//      XERR << "[交易-赠送接收者] 拒绝赠送，修改数据失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//      return false;
//    }
//  }
//
//  //send msg to scene to del npc
//  DelGiveSceneTradeCmd cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.set_id(id);
//  PROTOBUF(cmd, send, len);
//  if (m_pUser->sendCmdToScene(send, len) == false)
//  {
//    XERR << "[交易-赠送接收者] 拒绝赠送成功,发送通知到场景失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//  }
//
//  //send msg to sender
//  NtfGiveStatusSceneTradeCmd ntfCmd;
//  ntfCmd.set_charid(pGiveItemInfo->senderid());
//  ntfCmd.set_id(pGiveItemInfo->id());
//  ntfCmd.set_status(EGiveStatus_Refuse);
//  ntfCmd.set_name(m_pUser->name);
//  PROTOBUF(ntfCmd, ntfSend, ntfLen);
//  if (sendCmdToWorldUser(pGiveItemInfo->senderid(), ntfSend, ntfLen) == false)
//    XERR << "[交易-赠送接收者] 拒绝赠送成功，转发消息给赠送者失败" << m_pUser->id << "赠送者" << pGiveItemInfo->senderid() << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//  else
//    XLOG << "[交易-赠送接收者] 拒绝赠送成功，转发消息给赠送者成功" << m_pUser->id << "赠送者" << pGiveItemInfo->senderid() << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//
//  //礼物被拒绝时提示msg（非常遗憾，您的好友[63cd4e]%s[-]拒绝了你的礼物！礼物将返回至交易所交易记录。）
//  MsgParams param;
//  param.addString(m_pUser->name);
//  MsgManager::sendMsgMust(pGiveItemInfo->senderid(), 25001, param);   //离线也能收到
//
//                                                                      //一定要放在打印后面
//  m_giveToMe.erase(rev.id());
//  return true;
//}
//
//bool TradeLog::ntfGiveRes(const Cmd::NtfGiveStatusSceneTradeCmd& rev)
//{
//  XLOG << "[交易-赠送者] 收到赠送反馈" << m_pUser->id << "id" << rev.id() << "friendname" << rev.name() << XEND;
//
//  if (rev.status() == EGiveStatus_Accept)
//  {
//    return processAccept(rev.id());
//  }
//  else
//  {
//    return processRefuse(rev.id());
//  }
//  return true;
//}
//
//void TradeLog::syncToScene(Cmd::SyncGiveItemSceneTradeCmd& cmd)
//{
//  if (m_giveToMe.empty())
//    return;
//
//  cmd.set_charid(m_pUser->id);
//  for (auto it = m_giveToMe.begin(); it != m_giveToMe.end(); ++it)
//  {
//    GiveItemInfo* pInfo = cmd.add_iteminfo();
//    if (pInfo == nullptr)
//      continue;
//    pInfo->set_id(it->second.id());
//    pInfo->set_expiretime(it->second.expiretime());
//  }
//  PROTOBUF(cmd, send, len);
//
//  if (m_pUser->sendCmdToScene(send, len) == false)
//  {
//    XERR << "[交易-赠送接收者] 列表发送到场景失败" << m_pUser->id << m_pUser->name << "size" << cmd.iteminfo_size() << XEND;
//    return;
//  }
//  XLOG << "[交易-赠送接收者] 列表发送到场景成功" << m_pUser->id << m_pUser->name << "size" << cmd.iteminfo_size() << XEND;
//}
//
///*
//enum EOperType
//{
//EOperType_NormalSell = 1;               //普通出售
//EOperType_NoramlBuy = 2;                //普通购买
//EOperType_Publicity = 3;                //公示期
//EOperType_PublicitySellSuccess = 4;     //公示期出售成功
//EOperType_PublicitySellFail = 5;        //公示期出售失败
//EOperType_PublicityBuySuccess = 6;      //公示期购买成功
//EOperType_PublicityBuyFail = 7;         //公示期购买失败
//EOperType_PublicityBuying = 8;          //公示期正在购买
//};
//*/
//std::string TradeLog::getTableName(EOperType logType)
//{
//  if (logType == EOperType_NormalSell ||
//    logType == EOperType_PublicitySellSuccess ||
//    logType == EOperType_PublicitySellFail ||
//    logType == EOperType_AutoOffTheShelf
//    )
//  {
//    return DB_TABLE_SELLED_LIST;
//  }
//  else
//  {
//    return DB_TABLE_BUYED_LIST;
//  }
//}
//
//void TradeLog::addCanTakeCount()
//{
//  m_canTakeCount++;
//}
//
//void TradeLog::decCanTakeCount()
//{
//  if (m_canTakeCount > 0)
//    m_canTakeCount--;
//}
//
//void TradeLog::sendCanTakeCountToClient()
//{
//  NtfCanTakeCountTradeCmd cmd;
//  cmd.set_count(m_canTakeCount);
//  PROTOBUF(cmd, send, len);
//  m_pUser->sendCmdToMe(send, len);
//}
//
//void TradeLog::checkPublicityEnd()
//{
//  DWORD curSec = now();
//  for (auto it = m_listLogItemInfo.begin(); it != m_listLogItemInfo.end();)
//  {
//    if (it->logtype() == EOperType_PublicityBuying && it->endtime() <= curSec)
//    {
//      it = m_listLogItemInfo.erase(it);
//      continue;
//    }
//    ++it;
//  }
//}
//
//bool TradeLog::sendCmdToWorldUser(QWORD charid, const void *buf, WORD len)
//{
//  Cmd::GlobalForwardCmdSocialCmd2 message;
//  message.set_charid(charid);
//  message.set_data(buf, len);
//  message.set_len(len);
//  PROTOBUF(message, send, len2);
//
//  if (thisServer->sendCmd(ClientType::global_server, send, len2) == false)
//  {
//    return false;
//  }
//  return true;
//}
//
////ETakeStatus_CanTakeGive = 0;                //可领取可赠送
////ETakeStatus_Took = 1;                       //已领取         
////ETakeStatus_Taking = 2;                     //正在领取
////ETakeStatus_Giving = 3;                     //正在赠送
////ETakeStatus_Give_Accepting = 4;             //赠送正在领取中
////ETakeStatus_Give_Accepted_1 = 5;            //赠送已经领取额度未扣
////ETakeStatus_Give_Accepted_2 = 6;            //赠送已经领取额度已扣  
//void TradeLog::loadGiveToMeFromDb()
//{
//  xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//  if (!field)
//    return;//
//  char where[128];
//  bzero(where, sizeof(where));
//
//  snprintf(where, sizeof(where), "receiveid=%llu", m_pUser->id);
//  xRecordSet set;
//  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//  if (ret == QWORD_MAX)
//  {
//    XERR << "[交易-数据库-赠送] DB_TABLE_BUYED_LIST数据库错误 charid：" << m_pUser->id << "table: " << DB_TABLE_BUYED_LIST << XEND;
//    return;
//  }
//
//  for (QWORD i = 0; i < ret; ++i)
//  {
//    ETakeStatus status = ETakeStatus(set[i].get<DWORD>("status"));
//    if (status == ETakeStatus_Giving || status == ETakeStatus_Give_Accepting)
//    {//别人赠给我的
//      Cmd::GiveItemInfo giveInfo;
//      packGiveItemInfo(set[i], giveInfo);
//      m_giveToMe[giveInfo.id()] = giveInfo;
//      if (status == ETakeStatus_Give_Accepting)
//      {
//        m_retryList.push_back(giveInfo);
//      }
//    }
//  }
//  XLOG << "[交易-赠送] 加载别人赠送给我的列表" << m_pUser->id << "成功加载了别人赠送给我的列表" << m_giveToMe.size() << "条" << "赠送重试列表" << m_retryList.size() << "条" << XEND;
//}
//
//void TradeLog::loadGiveToOtherFromDb()
//{
//  xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//  if (!field)
//    return;//
//  char where[128];
//  bzero(where, sizeof(where));
//
//  snprintf(where, sizeof(where), "buyerid=%llu", m_pUser->id);
//  xRecordSet set;
//  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//  if (ret == QWORD_MAX)
//  {
//    XERR << "[交易-数据库-赠送] DB_TABLE_BUYED_LIST数据库错误 charid：" << m_pUser->id << "table: " << DB_TABLE_BUYED_LIST << XEND;
//    return;
//  }
//
//  for (QWORD i = 0; i < ret; ++i)
//  {
//    ETakeStatus status = ETakeStatus(set[i].get<DWORD>("status"));
//    if (status == ETakeStatus_Giving || status == ETakeStatus_Give_Accepting || status == ETakeStatus_Give_Accepted_1)
//    {//我送给别人的
//      Cmd::GiveItemInfo giveInfo;
//      packGiveToOtherItemInfo(set[i], giveInfo);
//      m_giveToOther[giveInfo.id()] = giveInfo;
//    }
//  }
//
//  XLOG << "[交易-赠送] 加载赠送给别人的列表" << m_pUser->id << "成功加载了" << m_giveToOther.size() << "条" << XEND;
//}
//
//DWORD TradeLog::calcGiveFee(QWORD qwTotal, DWORD bg)
//{
//  DWORD dwTotal = (DWORD)qwTotal;
//  DWORD fee = LuaManager::getMe().call<DWORD>("calcTradeGiveFee", dwTotal, bg);
//  XDBG << "[交易-赠送者] 赠送费用" << m_pUser->id << "totalprice" << qwTotal << "bgid" << bg << "fee" << fee << XEND;
//  return fee;
//}
//
//bool TradeLog::isGiving()
//{
//  for (auto& v : m_giveToOther)
//  {
//    if (isGivingExpire(v.second.id(), v.second.status(), v.second.expiretime()) == false)
//      return true;
//  }
//
//  return false;
//}
//
//bool TradeLog::isGivingExpire(QWORD id, ETakeStatus status, DWORD expireTime)
//{
//  if (status != ETakeStatus_Giving)
//    return true;
//
//  DWORD curSec = now();
//  if (status == ETakeStatus_Giving && expireTime >= curSec)
//  {
//    //赠送没过期
//    XDBG << "[交易-赠送] 尚未过期 id" << id << "status" << status << "expiretime" << expireTime << XEND;
//    return false;
//  }
//  return true;
//}
//
//Cmd::GiveItemInfo* TradeLog::getGiveItemInfo(QWORD id)
//{
//  auto it = m_giveToMe.find(id);
//  if (it == m_giveToMe.end())
//    return nullptr;
//
//  return &(it->second);
//}
//
//Cmd::GiveItemInfo* TradeLog::getGive2OtherItemInfo(QWORD id)
//{
//  auto it = m_giveToOther.find(id);
//  if (it == m_giveToOther.end())
//    return nullptr;
//
//  return &(it->second);
//}
//
//void TradeLog::processGiveToOther(DWORD curSec)
//{
//  TSetQWORD setAccept;
//  TSetQWORD setRefuse;
//  for (auto it = m_giveToOther.begin(); it != m_giveToOther.end(); ++it)
//  {
//    if (it->second.status() == ETakeStatus_Give_Accepted_1)
//    {
//      setAccept.insert(it->second.id());
//      continue;
//    }
//
//    //超时过期
//    if (isGivingExpire(it->second.id(), it->second.status(), it->second.expiretime()))
//    {
//      setRefuse.insert(it->second.id());
//      continue;
//    }
//  }
//
//  for (auto&v : setAccept)
//    processAccept(v);
//
//  for (auto &v : setRefuse)
//    processRefuse(v, true);
//}
//
//bool TradeLog::processAccept(QWORD id)
//{
//  //accept
//  GiveItemInfo* pGiveItemInfo = getGive2OtherItemInfo(id);
//  if (pGiveItemInfo == nullptr)
//  {
//    XERR << "[交易-赠送者] 处理接收赠送,找不到赠送信息" << m_pUser->id << "id" << id << XEND;
//    return false;
//  }
//
//  //modify give db
//  xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//  if (!field)
//    return false;//
//  char where[128];
//  bzero(where, sizeof(where));
//  snprintf(where, sizeof(where), "id=%llu and status=%u", id, ETakeStatus_Give_Accepted_1);
//  xRecord record(field);
//  record.put("status", ETakeStatus_Give_Accepted_2);  //赠送已经领取额度已扣
//  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
//  if (ret == QWORD_MAX)
//  {
//    XERR << "[交易-赠送者] 处理接收赠送，修改数据失败" << m_pUser->id << "id" << id << "status" << pGiveItemInfo->status() << "itemid" << pGiveItemInfo->itemid() << "count" << pGiveItemInfo->count() << XEND;
//    return false;
//  }
//
//  //reduce quota
//  ReduceQuotaSceneTradeCmd cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.set_id(id);
//  cmd.set_quota(pGiveItemInfo->quota());
//  cmd.set_receivername(pGiveItemInfo->receivername());
//  PROTOBUF(cmd, send, len);
//  if (m_pUser->sendCmdToScene(send, len) == false)
//  {
//    XERR << "[交易-赠送者] 处理接收赠送,发送扣除额度消息到场景失败" << m_pUser->id << "id" << id << XEND;
//    return false;
//  }
//
//  pGiveItemInfo->set_status(ETakeStatus_Give_Accepted_2);
//  XLOG << "[交易-赠送者] 处理接收赠送，修改数据状态改为 赠送已经领取额度已扣" << m_pUser->id << "id" << id << "status" << pGiveItemInfo->status() << "itemid" << pGiveItemInfo->itemid() << "count" << pGiveItemInfo->count() << XEND;
//
//  ntfClientToUpdateLog();
//
//  //warning erase
//  m_giveToOther.erase(id);
//
//  return true;
//}
//
//bool TradeLog::processRefuse(QWORD id, bool isExpire/*=false*/)
//{
//  GiveItemInfo* pGiveItemInfo = getGive2OtherItemInfo(id);
//  if (pGiveItemInfo == nullptr)
//  {
//    XERR << "[交易-赠送者] 处理拒绝赠送,找不到赠送信息" << m_pUser->id << "id" << id << XEND;
//    return false;
//  }
//  if (pGiveItemInfo->status() != ETakeStatus_Giving)
//  {
//    XERR << "[交易-赠送者] 处理拒绝赠送,处在不可拒绝的状态" << m_pUser->id << "id" << id << "status" << pGiveItemInfo->status() << "isexpire" << isExpire << XEND;
//    return false;
//  }
//
//  if (isExpire)
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//    if (!field)
//      return false;//
//    char where[128];
//    bzero(where, sizeof(where));
//    snprintf(where, sizeof(where), "id=%llu and status=%u", id, ETakeStatus_Giving);
//    xRecord record(field);
//    record.put("status", ETakeStatus_CanTakeGive);
//    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
//    if (ret != 1)  //前一个状态必须是ETakeStatus_Giving
//    {
//      XERR << "[交易-赠送者] 处理拒绝赠送，修改数据失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
//      return false;
//    }
//    //因长时间未领取导致礼物没有成功送达msg
//    MsgManager::sendMsg(m_pUser->id, 25002);
//  }
//
//  pGiveItemInfo->set_status(ETakeStatus_CanTakeGive);
//  XLOG << "[交易-赠送者] 处理拒绝赠送，修改数据状态改为 可领取可赠送" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << "是否超时" << (isExpire ? "是" : "否") << XEND;
//
//  ntfClientToUpdateLog();
//  //warning erase
//  m_giveToOther.erase(id);
//  return true;
//}
//
//void TradeLog::packBuyedItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo)
//{
//  logInfo.set_id(r.get<QWORD>("id"));
//  logInfo.set_status(ETakeStatus(r.get<DWORD>("status")));
//  EOperType type = EOperType(r.get<DWORD>("logtype"));
//  logInfo.set_logtype(type);
//  logInfo.set_itemid(r.get<DWORD>("itemid"));
//  logInfo.set_refine_lv(r.get<DWORD>("refine_lv"));
//  logInfo.set_damage(r.get<DWORD>("damage"));
//  logInfo.set_tradetime(r.get<DWORD>("tradetime"));
//  logInfo.set_count(r.get<DWORD>("count"));
//  DWORD price = r.get<DWORD>("price");
//  logInfo.set_price(price);
//
//  logInfo.set_costmoney(price * logInfo.count());   //购买花费
//  logInfo.set_failcount(r.get<DWORD>("failcount"));
//  logInfo.set_totalcount(r.get<DWORD>("totalcount"));
//  logInfo.set_retmoney(price * logInfo.failcount());
//  logInfo.set_endtime(r.get<DWORD>("endtime"));
//
//  std::string sellerInfo((const char *)r.getBin("sellerinfo"), r.getBinSize("sellerinfo"));
//  if (!sellerInfo.empty())
//    logInfo.mutable_name_list()->ParseFromString(sellerInfo);
//
//  {
//    std::string strItemData((const char *)r.getBin("itemdata"), r.getBinSize("itemdata"));
//    if (!strItemData.empty())
//      logInfo.mutable_itemdata()->ParseFromString(strItemData);
//  }
//
//  logInfo.set_receiverid(r.get<QWORD>("receiveid"));
//  logInfo.set_receivername(r.getString("receivername"));
//  logInfo.set_receiverzoneid(r.get<DWORD>("receiverzoneid"));
//  logInfo.set_expiretime(r.get<DWORD>("expiretime"));
//
//  logInfo.set_quota(price * logInfo.count());
//  logInfo.set_background(r.get<DWORD>("background"));
//}
//
//void TradeLog::packGiveItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo)
//{
//  giveInfo.set_id(r.get<QWORD>("id"));
//  giveInfo.set_status(ETakeStatus(r.get<DWORD>("status")));
//  giveInfo.set_senderid(r.get<QWORD>("buyerid"));
//  giveInfo.set_sendername(r.getString("buyername"));
//  giveInfo.set_anonymous(r.get<DWORD>("anonymous"));
//  giveInfo.set_content(r.getString("content"));
//  giveInfo.set_expiretime(r.get<DWORD>("expiretime"));
//  giveInfo.set_itemid(r.get<DWORD>("itemid"));
//  giveInfo.set_count(r.get<DWORD>("count"));
//  giveInfo.set_background(r.get<DWORD>("background"));
//
//  {
//    std::string strItemData((const char *)r.getBin("itemdata"), r.getBinSize("itemdata"));
//    if (!strItemData.empty())
//      giveInfo.mutable_itemdata()->ParseFromString(strItemData);
//  }
//}
//
//void TradeLog::packGiveToOtherItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo)
//{
//  giveInfo.set_id(r.get<QWORD>("id"));
//  giveInfo.set_status(ETakeStatus(r.get<DWORD>("status")));
//  giveInfo.set_expiretime(r.get<DWORD>("expiretime"));
//  giveInfo.set_itemid(r.get<DWORD>("itemid"));
//  giveInfo.set_count(r.get<DWORD>("count"));
//  DWORD price = r.get<DWORD>("price");
//  giveInfo.set_quota(price * r.get<DWORD>("count"));
//  giveInfo.set_receivername(r.getString("receivername"));
//  giveInfo.set_background(r.get<DWORD>("background"));
//}
//
//void TradeLog::packGiveToOtherItemInfo(const Cmd::LogItemInfo& logInfo, Cmd::GiveItemInfo& giveInfo)
//{
//  giveInfo.set_id(logInfo.id());
//  giveInfo.set_status(logInfo.status());
//  giveInfo.set_expiretime(logInfo.expiretime());
//  giveInfo.set_itemid(logInfo.itemid());
//  giveInfo.set_count(logInfo.count());
//  giveInfo.set_quota(logInfo.quota());
//  giveInfo.set_receivername(logInfo.receivername());
//  giveInfo.set_background(logInfo.background());
//}
//
//inline bool TradeLog::lockGive()
//{
//  DWORD curSec = now();
//  if (curSec <= m_dwGiveLockTime)
//  {
//    XERR << "[交易-赠送] 赠送被锁住" << m_pUser->id << "curSec" << curSec << m_dwGiveLockTime << XEND;
//    return false;
//  }
//
//  m_dwGiveLockTime = curSec + 2 * 60;
//  return true;
//}
//
//inline void TradeLog::unlockGive()
//{
//  m_dwGiveLockTime = 0;
//}
//
//void TradeLog::retryAcceptGive()
//{
//  if (m_retryList.empty())
//    return;
//
//  for (auto& v : m_retryList)
//  {
//    //add item to scene
//    AddGiveItemSceneTradeCmd cmd;
//    cmd.set_charid(m_pUser->id);
//    cmd.set_id(v.id());
//    cmd.set_itemid(v.itemid());
//    cmd.set_count(v.count());
//    cmd.mutable_itemdata()->CopyFrom(v.itemdata());
//    cmd.set_background(v.background());
//    PROTOBUF(cmd, send, len);
//    if (m_pUser->sendCmdToScene(send, len) == false)
//    {
//      XERR << "[交易-赠送接收者] 重试加道具，接受赠送成功，发送消息给场景失败" << m_pUser->id << v.id() << v.status() << v.itemid() << v.count() << XEND;
//
//    }
//    XLOG << "[交易-赠送接收者]  重试加道具，接受赠送成功，发送消息给场景" << m_pUser->id << v.id() << v.status() << v.itemid() << v.count() << XEND;
//  }
//  m_retryList.clear();
//}
//
//void TradeLog::ntfClientToUpdateLog()
//{
//  ListNtfRecordTrade cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.set_type(ELIST_NTF_MY_LOG);
//  PROTOBUF(cmd, send, len);
//  m_pUser->sendCmdToMe(send, len);
//}

#else
// TradeLog
TradeLog::TradeLog(SessionUser* pUser) : m_pUser(pUser)
{

}

TradeLog::~TradeLog()
{

}

void TradeLog::onUserOnline()
{
  {
    SessionThreadData *pData = SessionThread::getMe().create(m_pUser->id, SessionThreadAction_Trade_LoadGiveToMe);
    if (pData)
    {
      SessionThread::getMe().add(pData);
    }
  }
  {
    SessionThreadData *pData = SessionThread::getMe().create(m_pUser->id, SessionThreadAction_Trade_LoadGiveToOther);
    if (pData)
    {
      SessionThread::getMe().add(pData);
    }
  }
}

void TradeLog::onUserOffline()
{
  m_listLogItemInfo.clear();
}

//one sec tick
void TradeLog::timer(DWORD curTime)
{
  processGiveToOther(curTime);

  retryAcceptGive();
  
  if (m_dwQuckTakeTime && curTime >= m_dwQuckTakeTime)
    sendQuickResToClient();
}

// 这条协议之前被废，因需要从新启用
// 只适用于摆摊普通购买成功
void TradeLog::updateTradeLog(const Cmd::UpdateTradeLogCmd& rev)
{
  std::string tableName = getTableName(rev.type(), rev.trade_type());
  
  if (tableName == DB_TABLE_BUYED_LIST)
  {
    addBuyedLog(rev.id());
  }
  else if (tableName == DB_TABLE_SELLED_LIST)
  {
    addSelledLog(rev.id());
  }
  else if(tableName == DB_TABLE_BOOTH_BUYED_LIST)
  {
    addBoothBuyedLog(rev.id());
  }
  else if(tableName == DB_TABLE_BOOTH_SELLED_LIST)
  {
    //todo
  }
  sendCanTakeCountToClient(rev.trade_type());
}

void TradeLog::addBoothBuyedLog(QWORD id)
{
  //摆摊购买记录
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BOOTH_BUYED_LIST);
  if (!field)
    return ;
  char where[128];
  bzero(where, sizeof(where));

  snprintf(where, sizeof(where), "id=%llu ", id);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-数据库] 数据库错误 charid：" << m_pUser->id << id <<"table: " << DB_TABLE_BOOTH_BUYED_LIST << XEND;
    return ;
  }
  for (QWORD i = 0; i < ret; ++i)
  {
    LogItemInfo logInfo;
    logInfo.set_id(set[i].get<QWORD>("id"));
    logInfo.set_status(ETakeStatus(set[i].get<DWORD>("take_status")));
    EOperType type = EOperType(set[i].get<DWORD>("status"));
    logInfo.set_logtype(type);
    logInfo.set_itemid(set[i].get<DWORD>("item_id"));
    logInfo.set_refine_lv(set[i].get<DWORD>("refine_lv"));
    logInfo.set_damage(set[i].get<DWORD>("damage"));
    logInfo.set_tradetime(set[i].get<DWORD>("tradetime"));
    logInfo.set_count(set[i].get<DWORD>("count"));
    DWORD price = set[i].get<DWORD>("price");

    logInfo.set_costmoney(price * logInfo.count());   //购买花费
    logInfo.set_failcount(set[i].get<DWORD>("failcount"));
    logInfo.set_totalcount(set[i].get<DWORD>("totalcount"));
    logInfo.set_retmoney(price * logInfo.failcount());
    logInfo.set_endtime(set[i].get<DWORD>("endtime"));
    logInfo.set_is_public(set[i].get<DWORD>("is_public"));
    logInfo.set_trade_type(Cmd::ETRADETYPE_BOOTH);

    {
      std::string strItemData((const char *)set[i].getBin("item_data"), set[i].getBinSize("item_data"));
      if (!strItemData.empty())
        logInfo.mutable_itemdata()->ParseFromString(strItemData);
    }
    //已经安装交易时间从大到小排序了
    m_listLogItemInfo.push_front(logInfo);
    if (canTake(logInfo))
      addCanTakeCount();

    XLOG << "[交易-购买记录-新增]" << m_pUser->id << id;
    sendNewLogToClient(logInfo);
  }
}
void TradeLog::addBuyedLog(QWORD id)
{
  //购买记录
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
  if (!field)
    return ;//
  char where[128];
  bzero(where, sizeof(where));

  snprintf(where, sizeof(where), "id=%llu ", id);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-数据库] 数据库错误 charid：" << m_pUser->id << id <<"table: " << DB_TABLE_BUYED_LIST << XEND;
    return ;
  }
  for (QWORD i = 0; i < ret; ++i)
  {
    LogItemInfo logInfo;
    logInfo.set_id(set[i].get<QWORD>("id"));
    logInfo.set_status(ETakeStatus(set[i].get<DWORD>("take_status")));
    EOperType type = EOperType(set[i].get<DWORD>("status"));
    logInfo.set_logtype(type);
    logInfo.set_itemid(set[i].get<DWORD>("item_id"));
    logInfo.set_refine_lv(set[i].get<DWORD>("refine_lv"));
    logInfo.set_damage(set[i].get<DWORD>("damage"));
    logInfo.set_tradetime(set[i].get<DWORD>("tradetime"));
    logInfo.set_count(set[i].get<DWORD>("count"));
    DWORD price = set[i].get<DWORD>("price");

    logInfo.set_costmoney(price * logInfo.count());   //购买花费
    logInfo.set_failcount(set[i].get<DWORD>("failcount"));
    logInfo.set_totalcount(set[i].get<DWORD>("totalcount"));
    logInfo.set_retmoney(price * logInfo.failcount());
    logInfo.set_endtime(set[i].get<DWORD>("endtime"));

    std::string sellerInfo((const char *)set[i].getBin("sellerinfo"), set[i].getBinSize("sellerinfo"));
    if (!sellerInfo.empty())
      logInfo.mutable_name_list()->ParseFromString(sellerInfo);

    {
      std::string strItemData((const char *)set[i].getBin("item_data"), set[i].getBinSize("item_data"));
      if (!strItemData.empty())
        logInfo.mutable_itemdata()->ParseFromString(strItemData);
    }
    //已经安装交易时间从大到小排序了
    m_listLogItemInfo.push_front(logInfo);
    if (canTake(logInfo))
      addCanTakeCount();

    XLOG << "[交易-购买记录-新增]" << m_pUser->id << id;
    sendNewLogToClient(logInfo);
  }
}

void TradeLog::addSelledLog(QWORD id)
{
  //出售记录
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_SELLED_LIST);
  if (!field)
    return ;//
  char where[128];
  bzero(where, sizeof(where));

  snprintf(where, sizeof(where), "id=%llu", id);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-数据库] 数据库错误 charid：" << m_pUser->id <<id << "table: " << DB_TABLE_SELLED_LIST << XEND;
    return ;
  }

  for (QWORD i = 0; i < ret; ++i)
  {
    LogItemInfo logInfo;
    logInfo.set_id(set[i].get<QWORD>("id"));
    logInfo.set_status(ETakeStatus(set[i].get<DWORD>("take_status")));
    EOperType type = EOperType(set[i].get<DWORD>("status"));
    logInfo.set_logtype(type);
    logInfo.set_itemid(set[i].get<DWORD>("item_id"));
    logInfo.set_refine_lv(set[i].get<DWORD>("refine_lv"));
    logInfo.set_damage(set[i].get<DWORD>("damage"));
    logInfo.set_tradetime(set[i].get<DWORD>("tradetime"));
    logInfo.set_count(set[i].get<DWORD>("count"));
    DWORD price = set[i].get<DWORD>("price");

    logInfo.set_tax(set[i].get<DWORD>("tax"));   //扣费
    DWORD getMoney = price * logInfo.count() - logInfo.tax();
    logInfo.set_getmoney(getMoney);

    std::string buyerInfo((const char *)set[i].getBin("buyerinfo"), set[i].getBinSize("buyerinfo"));
    if (!buyerInfo.empty())
      logInfo.mutable_name_list()->ParseFromString(buyerInfo);
    {
      std::string strItemData((const char *)set[i].getBin("item_data"), set[i].getBinSize("item_data"));
      if (!strItemData.empty())
        logInfo.mutable_itemdata()->ParseFromString(strItemData);
    }

    //已经安装交易时间从大到小排序了
    m_listLogItemInfo.push_front(logInfo);
    if (canTake(logInfo))
      addCanTakeCount();
    XLOG << "[交易-出售记录-新增]" << m_pUser->id << id;
    sendNewLogToClient(logInfo);
  }
}

void TradeLog::dataFilter(LogItemInfo& info)
{
  info.mutable_itemdata()->Clear();
  info.mutable_name_list()->Clear(); 
}

void TradeLog::sendNewLogToClient(LogItemInfo& info)
{
  dataFilter(info);
  AddNewLog cmd;
  cmd.set_charid(m_pUser->id);
  cmd.mutable_log()->CopyFrom(info);
  
  DWORD page_count = m_listLogItemInfo.size() / MiscConfig::getMe().getTradeCFG().dwPageNumber;
  if (m_listLogItemInfo.size() % MiscConfig::getMe().getTradeCFG().dwPageNumber != 0)
    page_count++;
  
  cmd.set_total_page_count(page_count);
  PROTOBUF(cmd, send, len);
  
  m_pUser->sendCmdToMe(send, len);
}

void TradeLog::getLogListByType(const Cmd::ETradeType eType, std::list<Cmd::LogItemInfo>& list)
{
  if(Cmd::ETRADETYPE_TRADE == eType)
  {
    for(auto& v : m_listLogItemInfo)
    {
      list.push_back(v);
    }
  }
  else if(Cmd::ETRADETYPE_BOOTH == eType)
  {
    for(auto& v : m_listLogItemInfo)
    {
      if(eType == v.trade_type())
        list.push_back(v);
    }
  }
}

void TradeLog::sendAllTradeLog(Cmd::MyTradeLogRecordTradeCmd& cmd)
{
  if (m_pUser == nullptr)
    return;

  checkPublicityEnd();

  // 根据类型选择记录
  std::list<Cmd::LogItemInfo> list;
  getLogListByType(cmd.trade_type(), list);

  DWORD page_count = list.size() / MiscConfig::getMe().getTradeCFG().dwPageNumber;
  if (list.size() % MiscConfig::getMe().getTradeCFG().dwPageNumber != 0)
    page_count++;

  cmd.set_total_page_count(page_count);
  
  //索引超过最大页显示当前页
  if (cmd.index() >= page_count)
  {
    if (page_count == 0)
      cmd.set_index(0);
    else
      cmd.set_index(page_count - 1);
  }

  DWORD i = 0;
  DWORD start = cmd.index() * MiscConfig::getMe().getTradeCFG().dwPageNumber;
  DWORD end = (cmd.index() + 1) * MiscConfig::getMe().getTradeCFG().dwPageNumber;

  do 
  {
    //超过索引 不处理了
    if (start >= list.size())
      break;

    //[0, 1)
    for (auto& v : list)
    {
      if (i >= start && i < end)
      {
        LogItemInfo* pLogItemInfo = cmd.add_log_list();
        if (pLogItemInfo != nullptr)
        {
          if (isBuy(v.logtype()))
            fillGiftInfo(v);
          LogItemInfo logItemInfo = v;
          dataFilter(logItemInfo);
          pLogItemInfo->CopyFrom(logItemInfo);
        }
      }
      i++;
      if (i >= end)
        break;
    }
  } while (0);
  
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XDBG << "[交易-我的交易记录] 请求返回：" << cmd.ShortDebugString() << XEND;
  XINF << "[交易-我的交易记录] 请求返回：" << m_pUser->id <<"index"<<cmd.index() <<"total count"<<cmd.total_page_count() << XEND;
  sendCanTakeCountToClient(cmd.trade_type());
}

//获取我的购买出售记录
bool TradeLog::fetchMyTradeLogList(Cmd::MyTradeLogRecordTradeCmd& rev)
{
  DWORD curSec = now();
  if (m_dwNextLogTime && m_dwNextLogTime > curSec)
    return true;

  m_listLogItemInfo.clear();
  m_dwNextLogTime = curSec + 2;     //访问间隔2秒
  QWORD charId = m_pUser->id;
  DWORD delTime = curSec - 30 * DAY_T;   //30天前未领取的清除掉
  m_canTakeCount = 0;
  m_canTakeBoothCount = 0;
  bool bLog = true;
  bool bLogCheck = true;
  QWORD canTakeMoney = 0;
  DWORD limitTradeTime = curSec - 20 * HOUR_T;
  QWORD limitMoney = 1000 *10000;

  //交易所购买记录
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
    if (!field)
      return false;//

    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "char_id=%llu ", charId);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_BUYED_LIST << XEND;
      return false;
    }
    for (QWORD i = 0; i < ret; ++i)
    {
      LogItemInfo logInfo;
      packBuyedItemInfo(set[i], logInfo);    

      //30天未领取
      if (logInfo.tradetime() <= delTime)
      {
        continue;
      }
      //公示期购买结束了
      if (logInfo.logtype() == EOperType_PublicityBuying  && logInfo.endtime() <= curSec)
      {
        continue;
      }

      if (canTake(logInfo))
      {
        if (bLogCheck)
        {
          if (logInfo.tradetime() >= limitTradeTime)
          {
            bLog = false;
            bLogCheck = false;
          }
        }
        canTakeMoney += getTakeMoney(logInfo);
        addCanTakeCount();
      }
      m_listLogItemInfo.push_back(logInfo);
    }
  }

  //交易所出售记录
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_SELLED_LIST);
    if (!field)
      return false;//
    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "char_id=%llu", charId);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_SELLED_LIST << XEND;
      return false;
    }

    for (QWORD i = 0; i < ret; ++i)
    {
      if (set[i].get<DWORD>("time") <= delTime)
      {
        continue;
      }

      LogItemInfo logInfo;
      packSellItemInfo(set[i], logInfo);
      if (logInfo.status() == ETakeStatus_CanTakeGive)
      {     
        addCanTakeCount();
        
        if (bLogCheck)
        {
          if (logInfo.tradetime() >= limitTradeTime)
          {
            bLog = false;
            bLogCheck = false;
          }
        }
        canTakeMoney += getTakeMoney(logInfo);      
      }
      m_listLogItemInfo.push_back(logInfo);
    }
  } 

  //摆摊购买记录
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BOOTH_BUYED_LIST);
    if (!field)
      return false;

    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "char_id=%llu ", charId);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_BOOTH_BUYED_LIST << XEND;
      return false;
    }

    for (QWORD i = 0; i < ret; ++i)
    {
      LogItemInfo logInfo;
      packBoothBuyItemInfo(set[i], logInfo);

      if(EOperType_PayPending == logInfo.logtype() ||
          EOperType_PayFail == logInfo.logtype() ||
          EOperType_PayTimeout == logInfo.logtype())
        continue;

      //30天未领取
      if (logInfo.tradetime() <= delTime)
        continue;
      //公示期购买结束了
      if (logInfo.logtype() == EOperType_PublicityBuying  && logInfo.endtime() <= curSec)
        continue;

      if (canTake(logInfo))
      {
        if (bLogCheck)
        {
          if (logInfo.tradetime() >= limitTradeTime)
          {
            bLog = false;
            bLogCheck = false;
          }
        }
        canTakeMoney += getTakeMoney(logInfo);
        addCanTakeCount();
      }
      m_listLogItemInfo.push_back(logInfo);
    }
  }

  //摆摊出售记录
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BOOTH_SELLED_LIST);
    if (!field)
      return false;
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "char_id=%llu", charId);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_BOOTH_SELLED_LIST << XEND;
      return false;
    }

    for (QWORD i = 0; i < ret; ++i)
    {
      if (set[i].get<DWORD>("time") <= delTime)
        continue;

      LogItemInfo logInfo;
      packBoothSellItemInfo(set[i], logInfo);
      if (logInfo.status() == ETakeStatus_CanTakeGive)
      {
        addCanTakeCount();
        addCanTakeBoothCount();
        if (bLogCheck)
        {
          if (logInfo.tradetime() >= limitTradeTime)
          {
            bLog = false;
            bLogCheck = false;
          }
        }
        canTakeMoney += getTakeMoney(logInfo);
      }
      m_listLogItemInfo.push_back(logInfo);
    }
  }

  //order by
  m_listLogItemInfo.sort(CmpByValue1());

  sendAllTradeLog(rev);
  
  if (bLog)
  {
    if (canTakeMoney >= limitMoney)
    {
      string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TADE_UNTAKELOG, m_pUser->id);
      DWORD loged = 0;
      RedisManager::getMe().getData(key, loged);
      if (!loged)
      {
        PlatLogManager::getMe().TradeUntakeLog(thisServer,
          thisServer->getPlatformID(),
          m_pUser->id,
          m_pUser->name,
          canTakeMoney,
          m_pUser->getGuild().name());
        
        DWORD expireTime = xTime::getDayStart(curSec) + DAY_T - curSec; //跨天到期
        RedisManager::getMe().setData(key, 1, expireTime);
      }
    }
  }
  return true;
}

bool TradeLog::canTake(const Cmd::LogItemInfo& logInfo, bool log/* = false*/)
{
  if(ETakeStatus_CanTakeGive != logInfo.status())
    return false;

  if(Cmd::EOperType_NoramlBuy == logInfo.logtype() ||
      Cmd::EOperType_NormalSell == logInfo.logtype() ||
      Cmd::EOperType_PublicityBuySuccess == logInfo.logtype() ||
      Cmd::EOperType_PublicitySellSuccess == logInfo.logtype() ||
      Cmd::EOperType_PublicityBuyFail == logInfo.logtype()
      )
  {
    return true;
  }

  XERR << "[交易-领取中] 状态不对 不可领取" << m_pUser->id << logInfo.id() << logInfo.logtype() <<XEND;
  return false;
}

bool TradeLog::canGive(const Cmd::LogItemInfo& logInfo)
{
  if (logInfo.cangive() == false)
    return false;
  
  DWORD curSec = now();
  if (curSec > logInfo.tradetime())
  {
    if (curSec - logInfo.tradetime() > MiscConfig::getMe().getTradeCFG().dwSendButtonTime)
      return false;
  }

  QWORD qwTotalPrice = logInfo.price()* logInfo.count();
  if (qwTotalPrice < MiscConfig::getMe().getTradeCFG().dwSendMoneyLimit)
  {
    XINF << "[交易-赠送] 没到达赠送价值 "<<logInfo.id()<<logInfo.logtype()<<  qwTotalPrice << "limit" << MiscConfig::getMe().getTradeCFG().dwSendMoneyLimit << XEND;
    return false;
  }
  if (logInfo.status() != ETakeStatus_CanTakeGive)
  {
    XERR <<"[交易-赠送] 不可赠送的状态"<< logInfo.id() << logInfo.logtype() << qwTotalPrice << "take_status" << logInfo.status() << XEND;
    return false;
  }
  return true;
}

bool TradeLog::takeLog(Cmd::TakeLogCmd& rev)
{ 
  bool ret = true;
  Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(rev.log().id(), rev.log().logtype(), rev.log().trade_type());
  do 
  {
    if (pLogItemInfo == nullptr)
    {
      ret = false;
      XERR << "[交易-领取中] 找不到 LogItemInfo charid" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << rev.ShortDebugString() << XEND;
      break;
    }
    
    if (canTake(*pLogItemInfo, true) == false)
    {
      ret = false;
      XERR << "[交易-领取中] 物品不可领取" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << pLogItemInfo->ShortDebugString() << XEND;
      break;
    }
    
    //modify db
    std::string tableName = getTableName(rev.log().logtype(), rev.log().trade_type());
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, tableName);
    if (!field)
    {
      ret = false;//
      XERR << "[交易-领取中] 数据库错误" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << pLogItemInfo->ShortDebugString() << XEND;
      break;
    }
    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "id=%lu and take_status=%u", rev.log().id(), ETakeStatus_CanTakeGive);

    xRecord record(field);
    record.put("take_status", ETakeStatus_Taking);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      ret = false;//
      XERR << "[交易-领取中] 数据库错误，修改订单状态出错" << m_pUser->id << rev.log().id() << rev.log().logtype() << "msg" << pLogItemInfo->ShortDebugString() << XEND;
      break;
    } 
    pLogItemInfo->set_status(ETakeStatus_Taking);
    
    GetTradeLogSessionCmd cmd;
    cmd.set_id(rev.log().id());
    cmd.set_logtype(rev.log().logtype());   
    cmd.set_trade_type(rev.log().trade_type());
   
    //出售 卖家给钱
    if (rev.log().logtype() == EOperType_NormalSell || rev.log().logtype() == EOperType_PublicitySellSuccess)
    {
      ItemInfo* pItemInfo = cmd.mutable_item();
      if (pItemInfo)
      {
        DWORD getMoney = pLogItemInfo->getmoney();
        pItemInfo->set_id(100);
        pItemInfo->set_count(getMoney);
        pItemInfo->set_source(ESOURCE_TRADE);
      }
      //出售记录 分享使用
      cmd.set_sell_item_id(pLogItemInfo->itemid());
      cmd.set_sell_price(pLogItemInfo->price());
      cmd.set_sell_count(pLogItemInfo->count());
      cmd.set_refine_lv(pLogItemInfo->refine_lv());
      cmd.set_tax(pLogItemInfo->tax());
    }
    
    //公示期购买失败，买家返回钱
    if (rev.log().logtype() == EOperType_PublicityBuyFail)
    {
      ItemInfo* pItemInfo = cmd.mutable_item();
      if (pItemInfo)
      {
        pItemInfo->set_id(100);
        pItemInfo->set_count(pLogItemInfo->retmoney());
        pItemInfo->set_source(ESOURCE_TRADE_PUBLICITY_FAILRET);
      }
    }

    // 摆摊公示购买解锁额度
    if(Cmd::ETRADETYPE_BOOTH == rev.log().trade_type())
    {
      if(Cmd::EOperType_PublicityBuyFail == rev.log().logtype() || Cmd::EOperType_PublicityBuySuccess == rev.log().logtype())
        cmd.set_quota(pLogItemInfo->quota_cost());
    }

    //购买 买家加装备
    if (rev.log().logtype() == EOperType_NoramlBuy || rev.log().logtype() == EOperType_PublicityBuySuccess)
    {
      if (pLogItemInfo->itemdata().has_base())
      {
        if (rev.log().logtype() == EOperType_NoramlBuy)
          pLogItemInfo->mutable_itemdata()->mutable_base()->set_source(ESOURCE_TRADE);
        else
          pLogItemInfo->mutable_itemdata()->mutable_base()->set_source(ESOURCE_TRADE_PUBLICITY);

        cmd.mutable_itemdata()->CopyFrom(pLogItemInfo->itemdata());
      }
      else
      {
        ItemInfo* pItemInfo = cmd.mutable_item();
        if (pItemInfo)
        {
          pItemInfo->set_id(pLogItemInfo->itemid());
          pItemInfo->set_count(pLogItemInfo->count());
          pItemInfo->set_source(ESOURCE_TRADE);
        }
      }     
    }

    //自动下架，返还上架费和道具
    if (rev.log().logtype() == EOperType_AutoOffTheShelf)
    {
      if (pLogItemInfo->itemdata().has_base())
      {
        pLogItemInfo->mutable_itemdata()->mutable_base()->set_source(ESOURCE_TRADE);
        cmd.mutable_itemdata()->CopyFrom(pLogItemInfo->itemdata());
      }
      else
      {
        ItemInfo* pItemInfo = cmd.mutable_item();
        if (pItemInfo)
        {
          pItemInfo->set_id(pLogItemInfo->itemid());
          pItemInfo->set_count(pLogItemInfo->count());
          pItemInfo->set_source(ESOURCE_TRADE);
        }
      }
      cmd.set_ret_cost(pLogItemInfo->ret_cost());
    }

    cmd.set_charid(m_pUser->id);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToScene(send, len);

    //platlog
    {
      ETRADE_TYPE tradeType = ETRADETYPE_NONE;
      if(Cmd::ETRADETYPE_BOOTH == pLogItemInfo->trade_type())
      {
        tradeType = ETRADETYPE_BOOTH_TAKE;
        switch (rev.log().logtype())
        {
          case EOperType_NormalSell:
            tradeType = ETRADETYPE_BOOTH_TAKE_SELL_MONEY;
            break;
          case EOperType_PublicitySellSuccess:
            tradeType = ETRADETYPE_BOOTH_TAKE_PUBLICITY_SELL_MONEY;
            break;
          case EOperType_PublicityBuyFail:
            tradeType = ETRADETYPE_BOOTH_TAKE_RETURN_MONEY;
            break;
          case EOperType_NoramlBuy:
            tradeType = ETRADETYPE_BOOTH_TAKE_BUY_ITEM;
            break;
          case EOperType_PublicityBuySuccess:
            tradeType = ETRADETYPE_BOOTH_TAKE_PUBLICITY_BUY_ITEM;
            break;
          default:
            break;
        }
      }
      else if(Cmd::ETRADETYPE_TRADE == pLogItemInfo->trade_type())
      {
        tradeType = ETRADETYPE_TAKE;
        switch (rev.log().logtype())
        {
          case EOperType_NormalSell:
            tradeType = ETRADETYPE_TAKE_SELL_MONEY;
            break;
          case EOperType_PublicitySellSuccess:
            tradeType = ETRADETYPE_TAKE_PUBLICITY_SELL_MONEY;
            break;
          case EOperType_PublicityBuyFail:
            tradeType = ETRADETYPE_TAKE_RETURN_MONEY;
            break;
          case EOperType_NoramlBuy:
            tradeType = ETRADETYPE_TAKE_BUY_ITEM;
            break;
          case EOperType_PublicityBuySuccess:
            tradeType = ETRADETYPE_TAKE_PUBLICITY_BUY_ITEM;
            break;
          default:
            break;
        }
      }

      string jsonStr = pb2json(pLogItemInfo->itemdata());
      PlatLogManager::getMe().TradeLog(thisServer,
        thisServer->getPlatformID(),
        m_pUser->id,
        tradeType,
        pLogItemInfo->itemid(),
        pLogItemInfo->count(),
        pLogItemInfo->price(),
        pLogItemInfo->tax(),
        pLogItemInfo->getmoney(),
        jsonStr);
    }

  } while (0);

  if (pLogItemInfo)
    XLOG << "[交易-领取中] charid"<<m_pUser->id<<"ret"<<(ret?"成功":"失败") << rev.log().id() << rev.log().logtype() << "msg"<<pLogItemInfo->ShortDebugString() << XEND;
  else
    XLOG << "[交易-领取中] 出错，找不到 LogItemInfo charid" << m_pUser->id << "ret" << ret <<rev.log().id() <<rev.log().logtype() <<XEND;
  return ret;
}

bool TradeLog::takeSceneRes(const GetTradeLogSessionCmd& cmd)
{
  bool ret = false;

  do
  {
    ETakeStatus newStatus = ETakeStatus_Took;
    if (cmd.success())
    {
      newStatus = ETakeStatus_Took;
      ret = true;
      //dec
      decCanTakeCount();
      if(Cmd::ETRADETYPE_BOOTH == cmd.trade_type() || !isBuy(cmd.logtype()))
        decCanTakeBoothCount();
    }
    else
    {
      newStatus = ETakeStatus_CanTakeGive;
      ret = false;
    }

    //modify db
    std::string tableName = getTableName(cmd.logtype(), cmd.trade_type());
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, tableName);
    if (!field)
    {
      XERR << "[交易-领取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "还原状态失败。数据库找不到" << "newstatus" << newStatus << cmd.ShortDebugString() << XEND;
      ret = false;
      break;
    }
    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "id=%lu and take_status=%u", cmd.id(), ETakeStatus_Taking);

    xRecord record(field);
    record.put("take_status", newStatus);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[交易-领取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "还原状态失败。数据库插入失败" << "newstatus" << newStatus << cmd.ShortDebugString() << XEND;
      ret = false;
      break;
    }

    Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(cmd.id(), cmd.logtype(), cmd.trade_type());
    if (pLogItemInfo)
    {
      pLogItemInfo->set_status(newStatus);
    }

    XLOG << "[交易-领取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "成功还原数据库状态" << "newstatus" << newStatus << cmd.ShortDebugString() << XEND;
  } while (0);

  TakeLogCmd sendCmd;
  sendCmd.mutable_log()->set_id(cmd.id());
  sendCmd.mutable_log()->set_logtype(cmd.logtype());
  sendCmd.mutable_log()->set_trade_type(cmd.trade_type());
  sendCmd.set_success(ret);
  PROTOBUF(sendCmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  if(Cmd::ETRADETYPE_BOOTH == cmd.trade_type())
    sendCanTakeCountToClient(Cmd::ETRADETYPE_BOOTH);
  sendCanTakeCountToClient(Cmd::ETRADETYPE_TRADE);
  quickTakeSceneRes(cmd);

  return true;
}

void TradeLog::fetchNameInfo(Cmd::FetchNameInfoCmd& cmd)
{
  do
  {
    LogItemInfo* pItemInfo = getLogItemInfo(cmd.id(), cmd.type());
    if (pItemInfo == nullptr)
      break;

    const NameInfoList& rNameList = pItemInfo->name_list();

    DWORD allSize = rNameList.name_infos_size();

    DWORD page_count = allSize / MiscConfig::getMe().getTradeCFG().dwPageNumber;
    if (allSize % MiscConfig::getMe().getTradeCFG().dwPageNumber != 0)
      page_count++;

    cmd.set_total_page_count(page_count);

    DWORD start = cmd.index() * MiscConfig::getMe().getTradeCFG().dwPageNumber;
    DWORD end = (cmd.index() + 1) * MiscConfig::getMe().getTradeCFG().dwPageNumber;

    //超过索引 不处理了
    if (start >= allSize)
      return;

    //[0, 1)
    for (DWORD i = 0; i < allSize; ++i)
    {
      if (i >= end)
        break;
      if (i >= start && i < end)
      {
        cmd.mutable_name_list()->add_name_infos()->CopyFrom(rNameList.name_infos(i));
      }
    }
  } while (0);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[交易-查询名字]" << m_pUser->id << cmd.ShortDebugString() << XEND;
}

bool TradeLog::give(Cmd::GiveTradeCmd& rev)
{
  if (!m_pUser)
    return false;

  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_SEND) == true)
  {
    XERR<<"[交易-赠送] 赠送功能未开启" << m_pUser->id << XEND;
    return true;
  }

  if (thisServer->isOuter())
  {
    if (!m_pUser->m_bSafeDevice)
    {
      XERR << "[交易-赠送] 不是安全设备 charid" << m_pUser->id<<"safedevice"<<m_pUser->m_bSafeDevice << XEND;
      return false;
    }
  }

  if(m_pUser->getAuthorize().getConfirmed() == false)
  {
    XERR << "[交易-赠送] 安全密码验证失败 " << m_pUser->id<<"name: "<<m_pUser->name<< XEND;
    return false;
  }

  if (lockGive() == false)
    return false;
  
  std::string tableName = getTableName(rev.logtype());
  if (tableName != DB_TABLE_BUYED_LIST)
  {
    XERR << "[交易-赠送] 非法的tablename" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
    unlockGive();
    return false;
  }

  Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(rev.id(), rev.logtype());
  if (pLogItemInfo == nullptr)
  {
    unlockGive();
    XERR << "[交易-赠送] 找不到 LogItemInfo charid" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
    return false;
  }

  if (canGive(*pLogItemInfo) == false)
  {
    unlockGive();
    XERR << "[交易-赠送] 物品不可赠送" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
    return false;
  }  

  DWORD startTime = BaseConfig::getMe().getTradeGiveStartTime();
  if (pLogItemInfo->tradetime() <= startTime)
  {
    MsgManager::sendMsg(m_pUser->id, 25008, MsgParams(BaseConfig::getMe().getStrTradeGiveTime()));
    XERR << "[交易-赠送] 交易记录超过赠送开始时间" << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << "tradetime" << pLogItemInfo->tradetime() << "赠送开始时间" << startTime << XEND;
    unlockGive();
    return false;
  }

  DWORD limitTime = now() - MiscConfig::getMe().getTradeCFG().dwCantSendTime;
  if (pLogItemInfo->tradetime() <= limitTime)
  {
    unlockGive();
    MsgManager::sendMsg(m_pUser->id, 25004);
    XERR << "[交易-赠送] 交易记录超过29天不可赠送" << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid()<<"tradetime"<<pLogItemInfo->tradetime() <<"limittime"<<limitTime << XEND;
    return false;
  }
  
  // check only one
  if (isGiving())
  {
    unlockGive();
    //已有物品正在赠送，无法赠送msg
    MsgManager::sendMsg(m_pUser->id, 25005);
    XERR << "[交易-赠送] 其他物品正在赠送" << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
    return false;
  }
  
  //check friend
//  if (!m_pUser->getGCharData()->checkRelation(rev.friendid(), ESOCIALRELATION_FRIEND))
  if (!m_pUser->getSocial().checkRelation(rev.friendid(), ESOCIALRELATION_FRIEND))
  {
    unlockGive();
    XERR << "[交易-赠送] 失败，对方不是我的好友" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype()<<"friendid"<<rev.friendid() << XEND;
    return false;
  }

  DWORD friendTime = m_pUser->getSocial().getRelationTime(rev.friendid(), ESOCIALRELATION_FRIEND);
  if(friendTime >= pLogItemInfo->tradetime())
  {
    unlockGive();
    XERR << "[交易-赠送] 失败，与对方建立好友的时间晚于交易时间" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << "friendTime" << friendTime << "tradeTime" << pLogItemInfo->tradetime() << XEND;
    return false;
  }

  GCharReader gCharReader(thisServer->getRegionID(), rev.friendid());
  if (!gCharReader.get())
  {    
    unlockGive();
    XERR << "[交易-赠送] 失败，获取好友数据失败" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
    return false;
  }
  
  //is online
  if (gCharReader.getOnlineTime() < gCharReader.getOfflineTime())
  {
    unlockGive();
    XERR << "[交易-赠送] 失败，好友不在线" << m_pUser->id << "logid" << rev.id() << "type" << rev.logtype() << "friendid" << rev.friendid() << XEND;
    return false;
  }

  DWORD fee = calcGiveFee(pLogItemInfo->quota(), rev.background());
  Cmd::GiveCheckMoneySceneTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_friendid(rev.friendid());
  cmd.set_type(rev.logtype());
  cmd.set_id(rev.id());
  cmd.set_quota(pLogItemInfo->quota());   //从交易所服赋值
  cmd.set_fee(fee);
  cmd.set_anonymous(rev.anonymous());
  cmd.set_content(rev.content());
  cmd.set_background(rev.background());
  cmd.set_fromtrade(false);     //不发到交易所了

  //ItemData* pItemData = cmd.mutable_itemdata();
  //if (!pItemData)
  //{
  //  unlockGive();
  //  return false;
  //}
  //if (pLogItemInfo->itemdata().has_base())
  //{
  //  pItemData->CopyFrom(pLogItemInfo->itemdata());
  //}
  //else
  //{
  //  pItemData->mutable_base()->set_id(pLogItemInfo->itemid());
  //  pItemData->mutable_base()->set_count(pLogItemInfo->count());
  //}
  
  PROTOBUF(cmd, send, len);
  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    unlockGive();
    XERR << "[交易-赠送] 发送赠送消息给场景服失败" << m_pUser->id << "type" << cmd.type() << "赠送给" <<cmd.friendid() << "id" << cmd.id() << "quota" << cmd.quota() << "fee" << cmd.fee() << XEND;
    return false;
  }
  
  XLOG << "[交易-赠送] 发送赠送消息给场景服成功" << m_pUser->id << "type" << cmd.type() << "赠送给" << cmd.friendid() << "id" << cmd.id() << "quota" << cmd.quota() << "fee" << cmd.fee() << XEND;
  return true;
}

bool TradeLog::giveTradeRes(Cmd::GiveCheckMoneySceneTradeCmd& rev)
{
  //Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(rev.id(), rev.type());
  //if (pLogItemInfo == nullptr)
  //{
  //  XERR << "[交易-赠送-从交易所服获取额度返回] 找不到 LogItemInfo charid" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
  //  return false;
  //}  
  //pLogItemInfo->set_quota(rev.quota());
  //DWORD fee = calcGiveFee(rev.quota(), rev.background());
  //rev.set_fee(fee);
  //rev.set_fromtrade(false);  
  //PROTOBUF(rev, send, len);
  //bool ret = m_pUser->sendCmdToScene(send, len);
  //XLOG << "[交易-赠送者] 发送赠送消息给场景服" << m_pUser->id << "type" << rev.type() << "赠送给" << rev.friendid() << "id" << rev.id() << "quota" << rev.quota() << "fee" << rev.fee() << "ret" << ret << "deail" << rev.ShortDebugString() << XEND;
  return true;
}

bool TradeLog::giveSceneRes(Cmd::GiveCheckMoneySceneTradeCmd& rev)
{
  unlockGive();

  if (rev.ret() == false)
  {
    XERR << "[交易所-赠送-从场景扣除赠送返回] 场景检查不通过" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
    return false;
  }

  //success
  Cmd::LogItemInfo* pLogItemInfo = getLogItemInfo(rev.id(), rev.type());
  if (pLogItemInfo == nullptr)
  {
    XERR << "[交易-赠送-从场景扣除赠送返回] 找不到 LogItemInfo charid" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
    return false;
  }

  GCharReader gCharReader(thisServer->getRegionID(), rev.friendid());
  if (!gCharReader.get())
  {
    XERR << "[交易-赠送-从场景扣除赠送返回] 找不到好友gchar数据" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
    return false;
  }

  DWORD dwExpireTime = now() + MiscConfig::getMe().getTradeCFG().dwFollowTime;
  pLogItemInfo->set_status(ETakeStatus_Giving);
  pLogItemInfo->set_expiretime(dwExpireTime);

  //modify db 
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
  if (!field)
  {
    XERR << "[交易-赠送-从场景扣除赠送返回] 数据库错误" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
    return false;
  }
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%lu and take_status=%u", rev.id(), ETakeStatus_CanTakeGive);
  xRecord record(field);
  record.put("take_status", ETakeStatus_Giving);
  record.put("player_name", m_pUser->name);       //数据库存的不准，此处在更新下
  record.put("player_zone_id", thisServer->getZoneID());

  QWORD r = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (r != 1)
  {
    XERR << "[交易-赠送-从场景扣除赠送返回] DB_TABLE_BUYED_LIST 修改数据失败" << m_pUser->accid << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
    return false;
  }

  pLogItemInfo->set_receivername(gCharReader.getName());
  pLogItemInfo->set_receiverzoneid(gCharReader.getZoneID());

  //insert into 
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_GIFT);
    if (!field)
    {
      XERR << "[交易-赠送-从场景扣除赠送返回] 数据库错误" << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
      return false;
    }
    xRecord record(field);
    record.put("record_id", pLogItemInfo->id());
    record.put("receiver_id", rev.friendid());
    record.putString("receiver_name", pLogItemInfo->receivername());
    record.put("receiver_zone_id", pLogItemInfo->receiverzoneid());
    record.put("expire_time", dwExpireTime);
    record.put("anonymous", rev.anonymous());
    record.put("background", rev.background());
    record.putString("content", rev.content());
    QWORD r = thisServer->getTradeConnPool().exeInsert(record, true, true);
    if (r == QWORD_MAX)
    {
      XERR << "[交易-赠送-从场景扣除赠送返回] DB_TABLE_BUYED_LIST 插入trade_gift数据库失败" << m_pUser->accid << m_pUser->id << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
      return false;
    }    
  }
  
  //
  MsgManager::sendMsg(m_pUser->id, 25007);

  //
  string jsonStr = pb2json(pLogItemInfo->itemdata());
  PlatLogManager::getMe().TradeGiveLog(thisServer,
                                       m_pUser->id,
                                       EGiveEvent_Give,
                                       pLogItemInfo->itemid(),
                                       rev.quota(),
                                       jsonStr,
                                       rev.friendid(),
                                       m_pUser->name,
                                       pLogItemInfo->receivername(),
                                       now(),
                                       ELogGiveType_Trade,
                                       pLogItemInfo->count()
    );

  GiveItemInfo giveItemInfo;
  packGiveToOtherItemInfo(*pLogItemInfo, giveItemInfo);
  m_giveToOther[giveItemInfo.id()] = giveItemInfo;

  //send cmd to friend
  ReceiveGiveSceneTradeCmd cmd;
  cmd.set_id(rev.id());
  cmd.set_charid(rev.friendid());
  PROTOBUF(cmd, send, len);
  if (sendCmdToWorldUser(rev.friendid(), send, len) == false)
  {
    XERR << "[交易-赠送-从场景扣除赠送返回] 通知好友消息发送失败" << m_pUser->accid << m_pUser->id << "赠送" << rev.id() << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;
    return false;
  }
  XLOG << "[交易-赠送-从场景扣除赠送返回] 通知好友消息发送成功" << m_pUser->accid << m_pUser->id << "赠送" << rev.id() << "friendid" << rev.friendid() << "type" << rev.type() << "id" << rev.id() << XEND;

  ntfClientToUpdateLog();
  return true;
}

bool TradeLog::receiveGive(QWORD id)
{
  xField field(REGION_DB, DB_TABLE_BUYED_LIST);
  field.m_list["id"] = MYSQL_TYPE_LONG;
  field.m_list["char_id"] = MYSQL_TYPE_LONG;
  field.m_list["take_status"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["player_name"] = MYSQL_TYPE_STRING;
  field.m_list["item_id"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["count"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["price"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["item_data"] = MYSQL_TYPE_BLOB;
  field.m_list["expire_time"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["anonymous"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["background"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["content"] = MYSQL_TYPE_STRING;

  char sql[512];
  bzero(sql, sizeof(sql));
  snprintf(sql, sizeof(sql), "select t1.id as id, t1.char_id as char_id, t1.take_status as take_status,t1.player_name as player_name, t1.item_id as item_id, t1.count as count, t1.price as price, t1.item_data as item_data, COALESCE(t2.expire_time, 0) as expire_time,COALESCE(t2.anonymous, 0) as anonymous, COALESCE(t2.background, 0) as background,COALESCE(t2.content, \"\") as content from %s as t1 left join %s as t2 on t1.id=t2.record_id where t1.id =%llu", DB_TABLE_BUYED_LIST, DB_TABLE_GIFT, id);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, set, sql);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-赠送接收者] DB_TABLE_BUYED_LIST数据库错误 charid：" << m_pUser->id << "table: " << DB_TABLE_BUYED_LIST << XEND;
    return false;
  }  
  Cmd::GiveItemInfo giveInfo;
  packGiveItemInfo(set[0], giveInfo);    
  
  m_giveToMe[giveInfo.id()] = giveInfo;
  
  //send to scene
  AddGiveSceneTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  GiveItemInfo* pInfo = cmd.mutable_iteminfo();
  if (pInfo == nullptr)
  {
    XERR << "[交易-赠送接收者] protobuf协议添加失败" << m_pUser->id << "id" << giveInfo.id() << "from" << giveInfo.senderid() << XEND;
    return false;
  }
  pInfo->set_id(giveInfo.id());
  pInfo->set_expiretime(giveInfo.expiretime());
  PROTOBUF(cmd, send, len);
  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    XERR << "[交易-赠送接收者] 发送通知到场景失败" << m_pUser->id << "id" << giveInfo.id() << "from" << giveInfo.senderid() << XEND;
  }

  //您好！你有一份赠礼送达，请在姜饼人处领取礼物。
  DWORD npcId = 4364;
  const SNpcCFG* pNpcCfg = NpcConfig::getMe().getNpcCFG(npcId);
  if (pNpcCfg)
  {
    MsgParams params;
    params.addString(pNpcCfg->strName);
    MsgManager::sendMsg(m_pUser->id, 25100, params);
  }

  XLOG << "[交易-赠送接收者] 发送通知到场景成功" << m_pUser->id << "id" << giveInfo.id() << "from" << giveInfo.senderid()<<"expitetime"<<giveInfo.expiretime() << XEND;
  return true;
}

bool TradeLog::reqGiveItemInfo(Cmd::ReqGiveItemInfoCmd& rev)
{
  QWORD id = rev.id();
  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(rev.id());
  if (pGiveItemInfo == nullptr)
  {
    XERR << "[交易-赠送接收者] 请求赠送信息，找不到赠送id" << m_pUser->id << id <<"logid"<< rev.id() << XEND;
    return false;
  }

  if (pGiveItemInfo->status() != ETakeStatus_Giving)
  {
    XERR << "[交易-赠送接收者] 请求赠送信息，状态不对" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
    return false;
  }

  rev.mutable_iteminfo()->CopyFrom(*pGiveItemInfo);
 
  PROTOBUF(rev, send, len);
  if (m_pUser->sendCmdToMe(send, len) == false)
  {
    XERR << "[交易-赠送接收者] 请求赠送信息，发送消息给玩家失败" << m_pUser->id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
    return false;
  }

  XLOG << "[交易-赠送接收者] 请求赠送信息，发送消息给玩家场景" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << rev.ShortDebugString() <<XEND;
  return true;
}

bool TradeLog::acceptGive(AcceptTradeCmd& rev)
{
  QWORD id = rev.id();

  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(id);
  if (pGiveItemInfo == nullptr)
  {
    XERR << "[交易-赠送接收者] 接受赠送失败，找不到赠送id"<<m_pUser->id << id << "logid" << rev.id() << XEND;
    return false;
  }
  
  if (pGiveItemInfo->status() != ETakeStatus_Giving)
  {
    XERR << "[交易-赠送接收者] 接受赠送失败，状态不对" <<m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
    return false;
  }
    
  QWORD logid = pGiveItemInfo->id();

  //modify buyed db
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
    if (!field)
      return false;//
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id=%llu and take_status=%u", logid, ETakeStatus_Giving);
    xRecord record(field);
    record.put("take_status", ETakeStatus_Give_Accepting);  //赠送领取中
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[交易-赠送接收者] 接受赠送失败，数据库修改失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
      return false;
    }
    pGiveItemInfo->set_status(ETakeStatus_Give_Accepting);
  }
  
  //add item to scene
  AddGiveItemSceneTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_id(id);
  cmd.set_itemid(pGiveItemInfo->itemid());
  cmd.set_count(pGiveItemInfo->count());
  cmd.set_background(pGiveItemInfo->background());
  cmd.mutable_itemdata()->CopyFrom(pGiveItemInfo->itemdata());
  PROTOBUF(cmd, send, len);
  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    XERR << "[交易-赠送接收者] 接受赠送成功，发送消息给场景失败" << m_pUser->id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
    return false;
  }
  
  XLOG << "[交易-赠送接收者] 接受赠送成功，发送消息给场景" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
  return true;
}

bool TradeLog::acceptGiveSceneRes(const Cmd::AddGiveItemSceneTradeCmd& rev)
{
  if (rev.ret() == false)
  {
    XERR << "[交易-赠送接收者] 接受赠送场景返回，场景加道具失败等待后面重试" << m_pUser->id << rev.id() <<"ret" <<rev.ret()<<rev.ShortDebugString() << XEND;
    return false;
  }

  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(rev.id());
  if (pGiveItemInfo == nullptr)
  {
    XERR << "[交易-赠送接收者] 接受赠送场景返回，找不到赠送id" << m_pUser->id << rev.id() << XEND;
    return false;
  }

  //modify give db
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
    if (!field)
      return false;//
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id=%lu and take_status=%u", rev.id(), ETakeStatus_Give_Accepting);
    xRecord record(field);
    record.put("take_status", ETakeStatus_Give_Accepted_1);  //赠送已经领取额度未扣
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[交易-赠送接收者] 接受赠送场景返回，修改数据失败" << m_pUser->id << rev.id() << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
      return false;
    }
    pGiveItemInfo->set_status(ETakeStatus_Give_Accepted_1);
  }

  string jsonStr = pb2json(pGiveItemInfo->itemdata());
  PlatLogManager::getMe().TradeGiveLog(thisServer,
                                       pGiveItemInfo->senderid(),
                                       EGiveEvent_Accept,
                                       pGiveItemInfo->itemid(),
                                       pGiveItemInfo->quota(),
                                       jsonStr,
                                       m_pUser->id,
                                       pGiveItemInfo->sendername(),
                                       m_pUser->name,
                                       pGiveItemInfo->expiretime() - MiscConfig::getMe().getTradeCFG().dwFollowTime,
                                       ELogGiveType_Trade,
                                       pGiveItemInfo->count()
    );

  NtfGiveStatusSceneTradeCmd ntfCmd;
  ntfCmd.set_charid(pGiveItemInfo->senderid());
  ntfCmd.set_id(pGiveItemInfo->id());
  ntfCmd.set_status(EGiveStatus_Accept);
  ntfCmd.set_name(m_pUser->name);
  PROTOBUF(ntfCmd, ntfSend, ntfLen);
  if (sendCmdToWorldUser(pGiveItemInfo->senderid(), ntfSend, ntfLen) == false)
    XERR << "[交易-赠送接收者] 接受赠送场景返回，转发消息给赠送者失败" << m_pUser->id << "赠送者" << pGiveItemInfo->senderid() << rev.id() << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
  else
    XLOG << "[交易-赠送接收者] 接受赠送场景返回，转发消息给赠送者成功" << m_pUser->id << "赠送者" << pGiveItemInfo->senderid() << rev.id() << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
   
  //一定要放在打印后面
  m_giveToMe.erase(rev.id());
  return true;
}

bool TradeLog::refuseGive(RefuseTradeCmd& rev)
{
  QWORD id = rev.id();
  GiveItemInfo* pGiveItemInfo = getGiveItemInfo(id);
  if (pGiveItemInfo == nullptr)
  {
    XERR << "[交易-赠送接收者] 拒绝赠送，找不到赠送id" << m_pUser->id << id << XEND;
    return false;
  }

  if (pGiveItemInfo->status() != ETakeStatus_Giving)
  {
    XERR << "[交易-赠送接收者] 拒绝赠送，状态不对" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
    return false;
  }

  QWORD logid = pGiveItemInfo->id();
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
    if (!field)
      return false;//
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id=%llu and take_status=%u", logid, ETakeStatus_Giving);
    xRecord record(field);
    record.put("take_status", ETakeStatus_CanTakeGive);  //领取成功
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[交易-赠送接收者] 拒绝赠送，修改数据失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
      return false;
    }    
  }

  //send msg to scene to del npc
  DelGiveSceneTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_id(id);
  PROTOBUF(cmd, send, len);
  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    XERR << "[交易-赠送接收者] 拒绝赠送成功,发送通知到场景失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
  }
 
  //send msg to sender
  NtfGiveStatusSceneTradeCmd ntfCmd;
  ntfCmd.set_charid(pGiveItemInfo->senderid());
  ntfCmd.set_id(pGiveItemInfo->id());
  ntfCmd.set_status(EGiveStatus_Refuse);
  ntfCmd.set_name(m_pUser->name);
  PROTOBUF(ntfCmd, ntfSend, ntfLen);
  if (sendCmdToWorldUser(pGiveItemInfo->senderid(), ntfSend, ntfLen) == false)
    XERR << "[交易-赠送接收者] 拒绝赠送成功，转发消息给赠送者失败" << m_pUser->id << "赠送者" << pGiveItemInfo->senderid() << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
  else
    XLOG << "[交易-赠送接收者] 拒绝赠送成功，转发消息给赠送者成功" << m_pUser->id<<"赠送者"<<pGiveItemInfo->senderid() << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;

  //礼物被拒绝时提示msg（非常遗憾，您的好友[63cd4e]%s[-]拒绝了你的礼物！礼物将返回至交易所交易记录。）
  MsgParams param;
  param.addString(m_pUser->name);
  MsgManager::sendMsgMust(pGiveItemInfo->senderid(), 25001, param);   //离线也能收到

  string jsonStr = pb2json(pGiveItemInfo->itemdata());
  PlatLogManager::getMe().TradeGiveLog(thisServer,
                                       pGiveItemInfo->senderid(),
                                       EGiveEvent_Refuse,
                                       pGiveItemInfo->itemid(),
                                       pGiveItemInfo->quota(),
                                       jsonStr,
                                       m_pUser->id,
                                       pGiveItemInfo->sendername(),
                                       m_pUser->name,
                                       pGiveItemInfo->expiretime() - MiscConfig::getMe().getTradeCFG().dwFollowTime,
                                       ELogGiveType_Trade,
                                       pGiveItemInfo->count()
    );
                                                                      
  //一定要放在打印后面
  m_giveToMe.erase(rev.id());
  return true; 
}

bool TradeLog::ntfGiveRes(const Cmd::NtfGiveStatusSceneTradeCmd& rev)
{
  XLOG << "[交易-赠送者] 收到赠送反馈" << m_pUser->id << "id" << rev.id() << "friendname" << rev.name() << XEND;

  if (rev.status() == EGiveStatus_Accept)
  {
    return processAccept(rev.id());
  }
  else
  {    
    return processRefuse(rev.id());
  }
  return true;
}

void TradeLog::syncToScene(Cmd::SyncGiveItemSceneTradeCmd& cmd)
{
  if (!m_blTradeInit)
  {
    XLOG << "[交易-赠送接收者] 未初始化，延时发送" << m_pUser->id << m_pUser->name << XEND;
    m_blNeedSyncTradeToScene = true;
    return;
  }
  if (m_giveToMe.empty())
    return;
  cmd.set_type(EGiveType_Trade);
  cmd.set_charid(m_pUser->id);
  for (auto it = m_giveToMe.begin(); it != m_giveToMe.end(); ++it)
  {
    GiveItemInfo* pInfo = cmd.add_iteminfo();
    if (pInfo == nullptr)
      continue;
    pInfo->set_id(it->second.id());
    pInfo->set_expiretime(it->second.expiretime());
  }
  PROTOBUF(cmd, send, len);

  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    XERR << "[交易-赠送接收者] 列表发送到场景失败" << m_pUser->id << m_pUser->name << "size" << cmd.iteminfo_size() << XEND;
    return;
  }
  XLOG << "[交易-赠送接收者] 列表发送到场景成功" << m_pUser->id << m_pUser->name << "size" << cmd.iteminfo_size() << XEND;
}

/*
enum EOperType
{
EOperType_NormalSell = 1;               //普通出售
EOperType_NoramlBuy = 2;                //普通购买
EOperType_Publicity = 3;                //公示期
EOperType_PublicitySellSuccess = 4;     //公示期出售成功
EOperType_PublicitySellFail = 5;        //公示期出售失败
EOperType_PublicityBuySuccess = 6;      //公示期购买成功
EOperType_PublicityBuyFail = 7;         //公示期购买失败
EOperType_PublicityBuying = 8;          //公示期正在购买
};
*/
std::string TradeLog::getTableName(EOperType logType, ETradeType tradeType)
{
  if(ETRADETYPE_TRADE == tradeType)
  {
    if (isBuy(logType))
      return DB_TABLE_BUYED_LIST;
    else
      return DB_TABLE_SELLED_LIST; 
  }
  else if(ETRADETYPE_BOOTH == tradeType)
  {
    if (isBuy(logType))
      return DB_TABLE_BOOTH_BUYED_LIST;
    else
      return DB_TABLE_BOOTH_SELLED_LIST; 
  }

  return "";
}

void TradeLog::addCanTakeCount()
{
  m_canTakeCount++;
}

void TradeLog::decCanTakeCount()
{
  if (m_canTakeCount > 0)
    m_canTakeCount--;
}

void TradeLog::addCanTakeBoothCount()
{
  m_canTakeBoothCount++;
}

void TradeLog::decCanTakeBoothCount()
{
  if(m_canTakeBoothCount > 0)
  {
    m_canTakeBoothCount--;
  }
}

void TradeLog::sendCanTakeCountToClient(const Cmd::ETradeType tradeType)
{
  NtfCanTakeCountTradeCmd cmd;
  cmd.set_trade_type(tradeType);

  if(Cmd::ETRADETYPE_TRADE == tradeType)
    cmd.set_count(m_canTakeCount);
  else if(Cmd::ETRADETYPE_BOOTH == tradeType)
    cmd.set_count(m_canTakeBoothCount);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void TradeLog::checkPublicityEnd()
{
  DWORD curSec = now();
  for (auto it = m_listLogItemInfo.begin(); it != m_listLogItemInfo.end();)
  {
    if (it->logtype() == EOperType_PublicityBuying && it->endtime() <= curSec)
    {
      it = m_listLogItemInfo.erase(it);
      continue;
    }
    ++it;
  }
}

bool TradeLog::sendCmdToWorldUser(QWORD charid, const void *buf, WORD len)
{     
  Cmd::GlobalForwardCmdSocialCmd2 message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);

  if (thisServer->sendCmd(ClientType::global_server, send, len2) == false)
  {
    return false;
  }
  return true;
}

//ETakeStatus_CanTakeGive = 0;                //可领取可赠送
//ETakeStatus_Took = 1;                       //已领取         
//ETakeStatus_Taking = 2;                     //正在领取
//ETakeStatus_Giving = 3;                     //正在赠送
//ETakeStatus_Give_Accepting = 4;             //赠送正在领取中
//ETakeStatus_Give_Accepted_1 = 5;            //赠送已经领取额度未扣
//ETakeStatus_Give_Accepted_2 = 6;            //赠送已经领取额度已扣  
void TradeLog::loadGiveToMeFromDb(xRecordSet &set)
{
  DWORD num = set.size();
  for (QWORD i = 0; i < num; ++i)
  {
    ETakeStatus status = ETakeStatus(set[i].get<DWORD>("take_status"));
    if (status == ETakeStatus_Giving || status == ETakeStatus_Give_Accepting)
    {//别人赠给我的
      Cmd::GiveItemInfo giveInfo;
      packGiveItemInfo(set[i], giveInfo);
      m_giveToMe[giveInfo.id()] = giveInfo;
      if (status == ETakeStatus_Give_Accepting)
      {
        m_retryList.push_back(giveInfo);
      }
    }
  }
  m_blTradeInit = true;
  if (m_blNeedSyncTradeToScene)
  {
    Cmd::SyncGiveItemSceneTradeCmd cmd;
    syncToScene(cmd);
    m_pUser->getMail().syncGingerToScene();

    m_blNeedSyncTradeToScene = false;
  }
  XLOG << "[交易-赠送] 加载别人赠送给我的列表" << m_pUser->id << "成功加载了别人赠送给我的列表" << m_giveToMe.size() << "条" <<"赠送重试列表"<<m_retryList.size()<<"条" << XEND;
}

void TradeLog::loadGiveToOtherFromDb(xRecordSet &set)
{
  DWORD num = set.size();
  for (QWORD i = 0; i < num; ++i)
  {
    ETakeStatus status = ETakeStatus(set[i].get<DWORD>("take_status"));
    if (status == ETakeStatus_Giving || status == ETakeStatus_Give_Accepting || status == ETakeStatus_Give_Accepted_1)
    {//我送给别人的
      Cmd::GiveItemInfo giveInfo;
      packGiveToOtherItemInfo(set[i], giveInfo);
      m_giveToOther[giveInfo.id()] = giveInfo;
    }
  }

  XLOG << "[交易-赠送] 加载赠送给别人的列表" << m_pUser->id << "成功加载了" << m_giveToOther.size() << "条" << XEND;
}

DWORD TradeLog::calcGiveFee(QWORD qwTotal, DWORD bg)
{
  DWORD dwTotal = (DWORD)qwTotal;
  DWORD fee = LuaManager::getMe().call<DWORD>("calcTradeGiveFee", dwTotal, bg);
  XDBG << "[交易-赠送者] 赠送费用" << m_pUser->id << "totalprice" << qwTotal << "bgid" << bg <<"fee"<<fee << XEND;
  return fee;
}

bool TradeLog::isGiving()
{
  for (auto& v : m_giveToOther)
  {    
    if (isGivingExpire(v.second.id(), v.second.status(), v.second.expiretime()) == false)
      return true;
  }

  return false;
}

bool TradeLog::isGivingExpire(QWORD id, ETakeStatus status, DWORD expireTime)
{
  if (status != ETakeStatus_Giving)
    return true;

  DWORD curSec = now();
  if (status == ETakeStatus_Giving && expireTime >= curSec)
  {
    //赠送没过期
    XDBG << "[交易-赠送] 尚未过期 id" << id << "take_status" <<status << "expiretime" << expireTime << XEND;
    return false;
  }
  return true;
}

Cmd::GiveItemInfo* TradeLog::getGiveItemInfo(QWORD id)
{
  auto it = m_giveToMe.find(id);
  if (it == m_giveToMe.end())
    return nullptr;

  return &(it->second);
}

Cmd::GiveItemInfo* TradeLog::getGive2OtherItemInfo(QWORD id)
{
  auto it = m_giveToOther.find(id);
  if (it == m_giveToOther.end())
    return nullptr;

  return &(it->second);
}

void TradeLog::processGiveToOther(DWORD curSec)
{
  TSetQWORD setAccept;
  TSetQWORD setRefuse;
  for (auto it = m_giveToOther.begin(); it != m_giveToOther.end(); ++it)
  {
    if (it->second.status() == ETakeStatus_Give_Accepted_1)
    {
      setAccept.insert(it->second.id());
      continue;
    }    
    
    //超时过期
    if (isGivingExpire(it->second.id(), it->second.status(), it->second.expiretime()))
    {
      setRefuse.insert(it->second.id());
      continue;
    }
  }

  for (auto&v : setAccept)
    processAccept(v);

  for (auto &v : setRefuse)
    processRefuse(v, true);
}

bool TradeLog::processAccept(QWORD id)
{ 
  //accept
  GiveItemInfo* pGiveItemInfo = getGive2OtherItemInfo(id);
  if (pGiveItemInfo == nullptr)
  {
    XERR << "[交易-赠送者] 处理接收赠送,找不到赠送信息" << m_pUser->id << "id" << id << XEND;
    return false;
  }

  //modify give db
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
  if (!field)
    return false;//
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%llu and take_status=%u", id, ETakeStatus_Give_Accepted_1);
  xRecord record(field);
  record.put("take_status", ETakeStatus_Give_Accepted_2);  //赠送已经领取额度已扣
  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-赠送者] 处理接收赠送，修改数据失败" << m_pUser->id << "id" << id << "take_status" << pGiveItemInfo->status() << "itemid" << pGiveItemInfo->itemid() << "count" << pGiveItemInfo->count() << XEND;
    return false;
  } 

  //reduce quota
  ReduceQuotaSceneTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_id(id);
  cmd.set_quota(pGiveItemInfo->quota());
  cmd.set_receivername(pGiveItemInfo->receivername());
  PROTOBUF(cmd, send, len);
  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    XERR << "[交易-赠送者] 处理接收赠送,发送扣除额度消息到场景失败" << m_pUser->id << "id" << id << XEND;
    return false;
  }

  pGiveItemInfo->set_status(ETakeStatus_Give_Accepted_2);
  XLOG << "[交易-赠送者] 处理接收赠送，修改数据状态改为 赠送已经领取额度已扣" << m_pUser->id << "id" << id << "take_status" << pGiveItemInfo->status() << "itemid" << pGiveItemInfo->itemid() << "count" << pGiveItemInfo->count() << XEND;
  
  ntfClientToUpdateLog();

  //warning erase
  m_giveToOther.erase(id);
  
  return true;
}

bool TradeLog::processRefuse(QWORD id, bool isExpire/*=false*/)
{ 
  GiveItemInfo* pGiveItemInfo = getGive2OtherItemInfo(id);
  if (pGiveItemInfo == nullptr)
  {
    XERR << "[交易-赠送者] 处理拒绝赠送,找不到赠送信息" << m_pUser->id << "id" << id << XEND;
    return false;
  }
  if (pGiveItemInfo->status() != ETakeStatus_Giving)
  {
    XERR << "[交易-赠送者] 处理拒绝赠送,处在不可拒绝的状态" << m_pUser->id << "id" << id<<"take_status"<< pGiveItemInfo->status() <<"isexpire"<<isExpire << XEND;
    return false;
  }

  if (isExpire)  
  {  
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
    if (!field)
      return false;//
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id=%llu and take_status=%u", id, ETakeStatus_Giving);
    xRecord record(field);
    record.put("take_status", ETakeStatus_CanTakeGive);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)  //前一个状态必须是ETakeStatus_Giving
    {
      XERR << "[交易-赠送者] 处理拒绝赠送，修改数据失败" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << XEND;
      return false;
    } 
    //因长时间未领取导致礼物没有成功送达msg
    MsgManager::sendMsg(m_pUser->id, 25002);
  }

  // unlock quota
  UnlockQuotaSceneTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_id(id);
  cmd.set_quota(pGiveItemInfo->quota());
  cmd.set_receivername(pGiveItemInfo->receivername());
  PROTOBUF(cmd, send, len);
  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    XERR << "[交易-赠送者] 处理接收赠送,发送扣除额度消息到场景失败" << m_pUser->id << "id" << id << XEND;
    return false;
  }

  pGiveItemInfo->set_status(ETakeStatus_CanTakeGive);
  XLOG << "[交易-赠送者] 处理拒绝赠送，修改数据状态改为 可领取可赠送" << m_pUser->id << id << pGiveItemInfo->status() << pGiveItemInfo->itemid() << pGiveItemInfo->count() << "是否超时" << (isExpire ? "是" : "否") << XEND;

  ntfClientToUpdateLog();  
  //warning erase
  m_giveToOther.erase(id);
  return true;
}

void TradeLog::packGiveItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo)
{
  giveInfo.set_id(r.get<QWORD>("id"));
  giveInfo.set_status(ETakeStatus(r.get<DWORD>("take_status")));
  giveInfo.set_senderid(r.get<QWORD>("char_id"));
  giveInfo.set_sendername(r.getString("player_name"));
  giveInfo.set_anonymous(r.get<DWORD>("anonymous"));
  giveInfo.set_content(r.getString("content"));
  giveInfo.set_expiretime(r.get<DWORD>("expire_time"));
  giveInfo.set_itemid(r.get<DWORD>("item_id"));
  giveInfo.set_count(r.get<DWORD>("count"));
  giveInfo.set_background(r.get<DWORD>("background"));
  DWORD price = r.get<DWORD>("price");
  giveInfo.set_quota(price * r.get<DWORD>("count"));

  {
    std::string strItemData((const char *)r.getBin("item_data"), r.getBinSize("item_data"));
    if (!strItemData.empty())
      giveInfo.mutable_itemdata()->ParseFromString(strItemData);
  }
}

void TradeLog::packGiveToOtherItemInfo(const xRecord& r, Cmd::GiveItemInfo& giveInfo)
{
  giveInfo.set_id(r.get<QWORD>("id"));
  giveInfo.set_status(ETakeStatus(r.get<DWORD>("take_status")));
  giveInfo.set_expiretime(r.get<DWORD>("expire_time"));
  giveInfo.set_itemid(r.get<DWORD>("item_id"));
  giveInfo.set_count(r.get<DWORD>("count"));
  DWORD price = r.get<DWORD>("price");
  giveInfo.set_quota(price * r.get<DWORD>("count"));
  giveInfo.set_background(r.get<DWORD>("background"));
  giveInfo.set_receivername(r.getString("receiver_name"));
}

void TradeLog::packGiveToOtherItemInfo(const Cmd::LogItemInfo& logInfo, Cmd::GiveItemInfo& giveInfo)
{
  giveInfo.set_id(logInfo.id());
  giveInfo.set_status(logInfo.status());
  giveInfo.set_expiretime(logInfo.expiretime());
  giveInfo.set_itemid(logInfo.itemid());
  giveInfo.set_count(logInfo.count());
  giveInfo.set_quota(logInfo.quota());
  giveInfo.set_receivername(logInfo.receivername());
  giveInfo.set_background(logInfo.background());
}

inline bool TradeLog::lockGive()
{
  DWORD curSec = now();
  if (curSec <= m_dwGiveLockTime)
  {
    XERR << "[交易-赠送] 赠送被锁住" << m_pUser->id << "curSec" << curSec << m_dwGiveLockTime << XEND;
    return false;
  }

  m_dwGiveLockTime = curSec + 2 * 60;
  return true;
}

inline void TradeLog::unlockGive()
{
  m_dwGiveLockTime = 0;
}

void TradeLog::retryAcceptGive()
{
  if (m_retryList.empty())
    return;
  
  for (auto& v : m_retryList)
  {
    //add item to scene
    AddGiveItemSceneTradeCmd cmd;
    cmd.set_charid(m_pUser->id);
    cmd.set_id(v.id());
    cmd.set_itemid(v.itemid());
    cmd.set_count(v.count());
    cmd.mutable_itemdata()->CopyFrom(v.itemdata());
    cmd.set_background(v.background());
    PROTOBUF(cmd, send, len);
    if (m_pUser->sendCmdToScene(send, len) == false)
    {
      XERR << "[交易-赠送接收者] 重试加道具，接受赠送成功，发送消息给场景失败" << m_pUser->id <<v.id() << v.status() << v.itemid() << v.count() << XEND;
      
    }
    XLOG << "[交易-赠送接收者]  重试加道具，接受赠送成功，发送消息给场景" << m_pUser->id << v.id() << v.status() << v.itemid() << v.count() << XEND;
  }  
  m_retryList.clear();
}

void TradeLog::ntfClientToUpdateLog()
{
  ListNtfRecordTrade cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_type(ELIST_NTF_MY_LOG);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

EOperType TradeLog::status2LogType(ETradeBuyStatus eStatus)
{
  switch (eStatus)
  {
  case ETradeBuy_STATUS_PAY_SUCCESS:
    return EOperType_NoramlBuy;
  case ETradeBuy_STATUS_PUBLICITY_PAY_SUCCESS:
    return EOperType_PublicityBuying;
  case ETradeBuy_STATUS_PUBLICITY_SUCCESS:
    return EOperType_PublicityBuySuccess;
  case ETradeBuy_STATUS_PUBLICITY_CANCEL:
    return EOperType_PublicityBuyFail;
  default:
    break;
  }
  return EOperType_NoramlBuy;
}

// 摊位信息卖家处理
void TradeLog::packBoothSellItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo)
{
  logInfo.set_id(r.get<QWORD>("id"));
  ETradeSellStatus status = ETradeSellStatus(r.get<DWORD>("status"));  
  EOperType type = EOperType_NormalSell;
  if (status == ETradeSell_STATUS_AUTO_SHELF)
    type = EOperType_AutoOffTheShelf;

  logInfo.set_logtype(type);
  logInfo.set_status(ETakeStatus(r.get<DWORD>("take_status")));
  logInfo.set_itemid(r.get<DWORD>("item_id"));
  logInfo.set_refine_lv(r.get<DWORD>("refine_lv")); 
  logInfo.set_damage(r.get<DWORD>("is_damage"));
  logInfo.set_tradetime(r.get<DWORD>("time"));
  logInfo.set_count(r.get<DWORD>("count"));
  logInfo.set_quota_cost(r.get<DWORD>("quota"));
  DWORD price = r.get<DWORD>("price");
  logInfo.set_price(price);
  logInfo.set_is_public(r.get<DWORD>("is_publicity"));
  logInfo.set_trade_type(Cmd::ETRADETYPE_BOOTH);

  if (type == EOperType_NormalSell)
  {
    logInfo.set_tax(r.get<DWORD>("tax"));   //扣费
    DWORD getMoney = price * logInfo.count() - logInfo.tax();
    logInfo.set_getmoney(getMoney);

    std::string buyersInfo((const char *)r.getBin("buyers_info"), r.getBinSize("buyers_info"));
    if (!buyersInfo.empty())
    {
      logInfo.mutable_name_list()->ParseFromString(buyersInfo);
      if(logInfo.name_list().name_infos_size())
        logInfo.mutable_name_info()->CopyFrom(logInfo.name_list().name_infos(0));
    }
  }
  else
  {
    logInfo.set_ret_cost(r.get<DWORD>("tax"));   //自动下架返还上架费
    std::string strItemData((const char *)r.getBin("item_data"), r.getBinSize("item_data"));
    if (!strItemData.empty())
    {
      if (logInfo.mutable_itemdata())
      {
        logInfo.mutable_itemdata()->ParseFromString(strItemData);
      }
    }
  }
}

// 摊位信息买家处理
void TradeLog::packBoothBuyItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo)
{
  logInfo.set_id(r.get<QWORD>("id"));
  logInfo.set_logtype(EOperType(r.get<DWORD>("status")));
  logInfo.set_status(ETakeStatus(r.get<DWORD>("take_status")));
  logInfo.set_itemid(r.get<DWORD>("item_id"));
  logInfo.set_tradetime(r.get<DWORD>("time"));
  logInfo.set_count(r.get<DWORD>("count"));
  DWORD price = r.get<DWORD>("price");
  logInfo.set_price(price);

  logInfo.set_refine_lv(r.get<DWORD>("refine_lv"));
  logInfo.set_damage(r.get<DWORD>("is_damage"));

  logInfo.set_costmoney(price * logInfo.count());               //购买花费
  logInfo.set_totalcount(r.get<DWORD>("total_count"));
  logInfo.set_failcount(logInfo.totalcount() - logInfo.count());    //总的 - 成功的=失败的
  logInfo.set_retmoney(price * logInfo.failcount());
  logInfo.set_endtime(r.get<DWORD>("end_time"));
  logInfo.set_is_public(r.get<DWORD>("is_publicity"));
  logInfo.set_quota_cost(r.get<QWORD>("spend_quota")*logInfo.count());
  logInfo.set_trade_type(Cmd::ETRADETYPE_BOOTH);
  logInfo.set_cangive(false);

  if(EOperType_PublicityBuying == logInfo.logtype())
  {
    logInfo.set_count(r.get<DWORD>("total_count"));
    logInfo.set_costmoney(price * logInfo.count());
    logInfo.set_quota_cost(r.get<QWORD>("spend_quota")*logInfo.count());
  }
  else if(EOperType_PublicityBuyFail == logInfo.logtype())
    logInfo.set_quota_cost(r.get<QWORD>("spend_quota")*logInfo.failcount());
  else if(EOperType_PublicityBuySuccess == logInfo.logtype())
    logInfo.set_quota_cost(r.get<QWORD>("spend_quota")*logInfo.count());

  {
    Cmd::NameInfo nameInfo;
    nameInfo.set_name(r.getString("seller_name"));
    nameInfo.set_count(r.get<DWORD>("count"));
    nameInfo.set_zoneid(r.get<DWORD>("seller_zone_id"));
    logInfo.mutable_name_info()->CopyFrom(nameInfo);
  }

  {
    std::string strItemData((const char *)r.getBin("item_data"), r.getBinSize("item_data"));
    if (!strItemData.empty())
    {
      if (logInfo.mutable_itemdata())
      {
        logInfo.mutable_itemdata()->ParseFromString(strItemData);
      }
    }
  }
  logInfo.set_quota(logInfo.price() * logInfo.count());
}

void TradeLog::packSellItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo)
{
  logInfo.set_id(r.get<QWORD>("id"));
  ETradeSellStatus status = ETradeSellStatus(r.get<DWORD>("status"));  
  EOperType type = EOperType_NormalSell;
  if (status == ETradeSell_STATUS_AUTO_SHELF)
    type = EOperType_AutoOffTheShelf;

  logInfo.set_logtype(type);
  logInfo.set_status(ETakeStatus(r.get<DWORD>("take_status")));
  logInfo.set_itemid(r.get<DWORD>("item_id"));
  logInfo.set_refine_lv(r.get<DWORD>("refine_lv")); 
  logInfo.set_damage(r.get<DWORD>("is_damage"));
  logInfo.set_tradetime(r.get<DWORD>("time"));
  logInfo.set_count(r.get<DWORD>("count"));
  DWORD price = r.get<DWORD>("price");
  logInfo.set_price(price);

  if (type == EOperType_NormalSell)
  {
    logInfo.set_tax(r.get<DWORD>("tax"));   //扣费
    DWORD getMoney = price * logInfo.count() - logInfo.tax();
    logInfo.set_getmoney(getMoney);

    logInfo.set_is_many_people(r.get<DWORD>("is_many_people"));

    std::string buyerInfo((const char *)r.getBin("buyer_info"), r.getBinSize("buyer_info"));
    if (!buyerInfo.empty())
      logInfo.mutable_name_info()->ParseFromString(buyerInfo);

    if (logInfo.is_many_people())
    {
      std::string buyersInfo((const char *)r.getBin("buyers_info"), r.getBinSize("buyers_info"));
      if (!buyersInfo.empty())
        logInfo.mutable_name_list()->ParseFromString(buyersInfo);
    }
  }
  else
  {
    logInfo.set_ret_cost(r.get<DWORD>("tax"));   //自动下架返还上架费    
    std::string strItemData((const char *)r.getBin("item_data"), r.getBinSize("item_data"));
    if (!strItemData.empty())
    {
      if (logInfo.mutable_itemdata())
      {
        logInfo.mutable_itemdata()->ParseFromString(strItemData);
      }
    }
  }
}

void TradeLog::packBuyedItemInfo(const xRecord& r, Cmd::LogItemInfo& logInfo)
{
  logInfo.set_id(r.get<QWORD>("id"));
  ETradeBuyStatus status = ETradeBuyStatus(r.get<DWORD>("status"));

  EOperType type = status2LogType(status);
  logInfo.set_logtype(type);
  logInfo.set_status(ETakeStatus(r.get<DWORD>("take_status")));
  logInfo.set_itemid(r.get<DWORD>("item_id"));
  logInfo.set_tradetime(r.get<DWORD>("time"));
  logInfo.set_count(r.get<DWORD>("count"));
  DWORD price = r.get<DWORD>("price");
  logInfo.set_price(price);

  logInfo.set_refine_lv(r.get<DWORD>("refine_lv"));
  logInfo.set_damage(r.get<DWORD>("is_damage"));

  logInfo.set_costmoney(price * logInfo.count());               //购买花费
  logInfo.set_totalcount(r.get<DWORD>("total_count"));
  logInfo.set_failcount(logInfo.totalcount() - logInfo.count());    //总的 - 成功的=失败的
  logInfo.set_retmoney(price * logInfo.failcount());
  logInfo.set_endtime(r.get<DWORD>("end_time"));
  logInfo.set_is_many_people(r.get<DWORD>("is_many_people"));
  logInfo.set_cangive(r.get<DWORD>("can_give"));

  if (type == EOperType_PublicityBuying)      
  {
    logInfo.set_count(r.get<DWORD>("total_count"));     
    logInfo.set_costmoney(price * logInfo.count());
  }

  std::string sellerInfo((const char *)r.getBin("seller_info"), r.getBinSize("seller_info"));
  if (!sellerInfo.empty())
    logInfo.mutable_name_info()->ParseFromString(sellerInfo);

  if (logInfo.is_many_people())
  {
    std::string sellersInfo((const char *)r.getBin("sellers_info"), r.getBinSize("sellers_info"));
    if (!sellersInfo.empty())
      logInfo.mutable_name_list()->ParseFromString(sellersInfo);
  }

  {
    std::string strItemData((const char *)r.getBin("item_data"), r.getBinSize("item_data"));
    if (!strItemData.empty())
    {
      if (logInfo.mutable_itemdata())
      {
        logInfo.mutable_itemdata()->ParseFromString(strItemData);
      }
    }
  }
  logInfo.set_quota(logInfo.price() * logInfo.count());

}

bool TradeLog::isBuy(EOperType logType)
{
  if (logType == EOperType_NormalSell ||
    logType == EOperType_PublicitySellSuccess ||
    logType == EOperType_PublicitySellFail ||
    logType == EOperType_AutoOffTheShelf
    )
  {
    return false;
  }
  else
  {
    return true;
  }
  return true;
}

void TradeLog::fillGiftInfo(Cmd::LogItemInfo& logInfo)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_GIFT);
  if (!field)
    return;//
  char where[128];
  bzero(where, sizeof(where));

  snprintf(where, sizeof(where), "record_id=%lu ", logInfo.id());
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-数据库] 数据库错误 charid：" << m_pUser->id << logInfo.id() << "table: " << DB_TABLE_GIFT << XEND;
    return;
  }

  if (ret != 1)
  {
    return;
  }
  const xRecord &r = set[0];
  
  logInfo.set_receiverid(r.get<QWORD>("receiver_id"));
  logInfo.set_receivername(r.getString("receiver_name"));
  logInfo.set_receiverzoneid(r.get<DWORD>("receiver_zone_id"));
  logInfo.set_expiretime(r.get<DWORD>("expire_time"));
  logInfo.set_background(r.get<DWORD>("background"));
}


bool TradeLog::forwardCmdToTrade(const void* data, unsigned short len)
{
  SessionForwardScenecmdTrade cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_name(m_pUser->name);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_data(data, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send1, len1);
  return thisServer->sendCmd(ClientType::trade_server, send1, len1);
}

void TradeLog::quickTakeLog(const Cmd::ETradeType tradeType)
{
  if (m_dwQuckTakeTime)
    return;

  bool ret = false;
  do 
  {
    if (m_listLogItemInfo.empty())
      break;

    for (auto it = m_listLogItemInfo.begin(); it != m_listLogItemInfo.end();)
    {
      auto temp = it++;
      if(Cmd::ETRADETYPE_BOOTH == tradeType)
      {
        if(temp->trade_type() != tradeType || isBuy(temp->logtype()))
          continue;
      }

      if (!canTake(*temp, false))
        continue;    
      TakeLogCmd cmd;
      cmd.mutable_log()->CopyFrom(*temp);
      takeLog(cmd);
      std::pair<QWORD, Cmd::EOperType> pr;
      pr.first = temp->id();
      pr.second = temp->logtype();
      m_listQuickTake.push_back(pr);
      ret = true;
    }
  } while (0);
  
  if (ret)
  {
    m_dwQuckTakeTime = now() + 10;
  }
  else
  {
    sendQuickResToClient();
  }
   
  XLOG << "[交易所-一键领取] charid" << m_pUser->id << m_pUser->name << "领取条数" << m_listQuickTake.size() << "ret" << ret << XEND;
}

void TradeLog::quickTakeSceneRes(const GetTradeLogSessionCmd& rev)
{
  if (m_dwQuckTakeTime == 0)
    return;
  
  for (auto it = m_listQuickTake.begin(); it != m_listQuickTake.end(); ++it)
  {
    if (it->first == rev.id() && it->second == rev.logtype())
    {
      m_listQuickTake.erase(it);
      break;
    }
  }
  
  auto f = [&](QWORD &zeny, DWORD &item)
  {
    zeny += rev.ret_cost();
    if (rev.has_item())
    {
      if (rev.item().id() == 100)
        zeny += rev.item().count();
      else
        item += rev.item().count();
    }
    if (rev.has_itemdata())
      item += rev.itemdata().base().count();
  };

  if (rev.success())
    f(m_qwTotalZeny, m_dwItemCount);
  else
    f(m_qwTotalZenyFail, m_dwItemCountFail);
  
  XLOG << "[交易所-一键领取] 场景返回，charid" << m_pUser->id << m_pUser->name << "时间" << m_dwQuckTakeTime <<"itemid"<<rev.item().id()<<"itemcount"<<rev.item().count()<<"itemdata"<<rev.itemdata().base().id() <<"itemdatacount"<<rev.itemdata().base().count() <<"ret_cost"<< rev.ret_cost() << "领取zeny" << m_qwTotalZeny << "领取物品个数" << m_dwItemCount << "领取zeny失败" << m_qwTotalZenyFail << "领取物品失败个数" << m_dwItemCountFail << XEND;
  if (m_listQuickTake.empty())
    sendQuickResToClient();
}

void TradeLog::sendQuickResToClient()
{
  QucikTakeLogTradeCmd cmd;
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  
  if ((m_qwTotalZeny || m_dwItemCount) && !(m_qwTotalZenyFail || m_dwItemCountFail))
  {
    //msg
    MsgParams params;
    params.addNumber(m_qwTotalZeny);
    params.addNumber(m_dwItemCount);
    MsgManager::sendMsg(m_pUser->id, 3130, params);
  }
  else if (m_qwTotalZeny || m_dwItemCount || m_qwTotalZenyFail || m_dwItemCountFail)
  {
    //msg
    MsgParams params;
    params.addNumber(m_qwTotalZeny);
    params.addNumber(m_dwItemCount);
    params.addNumber(m_qwTotalZenyFail);
    params.addNumber(m_dwItemCountFail);
    MsgManager::sendMsg(m_pUser->id, 3131, params);
  }

  XLOG << "[交易所-一键领取] 给客户端返回，charid" << m_pUser->id << m_pUser->name << "时间" << m_dwQuckTakeTime << "领取zeny" << m_qwTotalZeny << "领取物品个数" << m_dwItemCount <<"领取zeny失败" <<m_qwTotalZenyFail <<"领取物品失败个数"<<m_dwItemCountFail << XEND;

  m_dwQuckTakeTime = 0;
  m_listQuickTake.clear();
  m_qwTotalZeny = 0;
  m_dwItemCount = 0;
  m_qwTotalZenyFail = 0;
  m_dwItemCountFail = 0;
}

QWORD TradeLog::getTakeMoney(const Cmd::LogItemInfo& logInfo)
{
  //出售 卖家给钱
  if (logInfo.logtype() == EOperType_NormalSell || logInfo.logtype() == EOperType_PublicitySellSuccess)
  {
    return logInfo.getmoney();
  }

  //公示期购买失败，买家返回钱
  if (logInfo.logtype() == EOperType_PublicityBuyFail)
  {
    return logInfo.retmoney();
  }

  //自动下架，返还上架费和道具
  if (logInfo.logtype() == EOperType_AutoOffTheShelf)
  {
    return logInfo.ret_cost();
  }
  return 0;
}

void TradeLog::reqLotteryGive(const LotteryGiveInfo& info)
{
  if(!m_pUser)
    return;

  if(isGiving())
  {
    MsgManager::sendMsg(m_pUser->id, 25313);
    return;
  }

  // 发送至场景
  ReqLotteryGiveSessionCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.mutable_iteminfo()->CopyFrom(info);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToScene(send, len);
}

#endif // DEBUG
