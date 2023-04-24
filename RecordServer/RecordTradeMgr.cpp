#include "RecordTradeMgr.h"
#include "xLuaTable.h"
#include "RecordServer.h"
#include "TableManager.h"
#include "ItemConfig.h"
#include "xCommand.h"
#include "SceneUser2.pb.h"
#include "SceneTrade.pb.h"
#include "MiscConfig.h"
#include "GuidManager.h"
#include <vector>
#include "StatisticsDefine.h"
#include "MailManager.h"
#include "PlatLogManager.h"
extern "C"
{
#include "md5/md5.h"
}

/************************************************************************/
/*      RecordTradeMgr                                                                */
/************************************************************************/
RecordTradeMgr::RecordTradeMgr()
{
}

RecordTradeMgr::~RecordTradeMgr()
{

}

void RecordTradeMgr::init()
{  
}

bool RecordTradeMgr::doTradeUserCmd(QWORD charId,  const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;//::Record;
  xCommand* cmd = (xCommand*)buf;
  MessageStatHelper msgStat(cmd->cmd, cmd->param);
  switch (cmd->param)
  {
    case BRIEF_PENDING_LIST_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::BriefPendingListRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
          return false;

        fetchAllBriefPendingList(charId, rev);

        return true;
      }
      break;
    case DETAIL_PENDING_LIST_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::DetailPendingListRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
          return false;

        fetchDetailPendingList(charId, rev);

        return true;
      }
      break;
    case ITEM_SELL_INFO_RECORDTRADE:
      {
        return true;
        PARSE_CMD_PROTOBUF(Cmd::ItemSellInfoRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
        {
          return false;
        }

        fetchItemSellInfoList(charId, rev);
        return true;
      }
      break;
    case MY_PENDING_LIST_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::MyPendingListRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
        {
          return false;
        }

        fetchMyPendingList(charId, rev);

        return true;
      }
      break;
    //case MY_TRADE_LOG_LIST_RECORDTRADE:
    //  {
    //    PARSE_CMD_PROTOBUF(Cmd::MyTradeLogRecordTradeCmd, rev);
    //    //TODO just for test
    //    /* if (checkCharId(charId, rev.charid()) == false)
    //       return false;*/
    //    if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
    //    {
    //      return false;
    //    }
    //    fetchMyTradeLogList(charId, rev);

    //    return true;
    //  }
    //  break;
    default:
      break;
  }
  return false;
}

bool RecordTradeMgr::checkCharId(QWORD charId1, QWORD charId2)
{
  if (charId1 == charId2)
    return true;
  XERR << "[交易]" << "玩家charid 不一致"<<charId1<<charId2 << XEND;
  return false;
}

bool RecordTradeMgr::sendCmdToMe(QWORD charId, const BYTE* buf, WORD len)
{
  Cmd::SessionToMeRecordTrade cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToSession(send2, len2);
}

void RecordTradeMgr::fetchAllBriefPendingList(QWORD charid, Cmd::BriefPendingListRecordTradeCmd& rev)
{
  stringstream ss;
  ss << "category" << rev.category() << "job" << rev.job() << "fashion" << rev.fashion();
  std::hash<string> strHash;
  QWORD key = strHash(ss.str());
  
  //fetch from cache
  auto it = m_mapBriefListCache.find(key);
  if (it != m_mapBriefListCache.end())
  {
    if (it->second.expireTime > now())
    {
      auto & cmd = it->second.cmd;
      cmd.set_charid(rev.charid());
      PROTOBUF(cmd, send, len);
      sendCmdToMe(charid, (BYTE*)send, len);
      return;
    }
  }

  do
  {
    TVecDWORD itemList = ItemConfig::getMe().getExchangeItemIds(rev.category(), rev.job(), rev.fashion());   
    if (itemList.empty()) {
      XERR << "[交易-挂单总览] fetchAllBriefPendingList，筛选不到相应的物品id,charid:" << rev.charid() <<"category:"<<rev.category()<<"job:"<<rev.job() << XEND;
      break;
    }

    xField field(REGION_DB, DB_TABLE_PENDING_LIST);
    field.setValid("distinct(itemid) as itemid, publicity_id");
    field.m_list["itemid"] = MYSQL_TYPE_NEWDECIMAL;
    field.m_list["publicity_id"] = MYSQL_TYPE_NEWDECIMAL;
    char where[10240];
    bzero(where, sizeof(where));

    time_t currTime = now();
    time_t shelfTime = currTime - MiscConfig::getMe().getTradeCFG().dwExpireTime;

    stringstream ss;
    bool isFirst = true;
    for (auto &v: itemList)
    {
      if (!isFirst)
      {
        ss << ",";
      }
      ss << v;
      isFirst = false;
    }

    snprintf(where, sizeof(where), "itemid IN (%s)  AND ((publicity_id=0 and pendingtime >= %lu) OR (publicity_id >= 1)) order by itemid, publicity_id desc",
      ss.str().c_str(),
      shelfTime);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(&field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-挂单总览] fetchAllBriefPendingList， 查询数据库出错, charid:" << rev.charid() << "ret:" << ret << XEND;
      break ;
    }

    TSetDWORD itemSet;

    for (DWORD i = 0; i < ret; ++i)
    {
      DWORD dwItemId = set[i].get<DWORD>("itemid");
      DWORD publicity_id = set[i].get<DWORD>("publicity_id");

      const SExchangeItemCFG*p = ItemConfig::getMe().getExchangeItemCFG(dwItemId);
      if (p == nullptr)
      {
        XINF << "[交易-挂单总览] 过滤掉不可出售物品 itemid:" << dwItemId << XEND;
        continue;
      }

      if (itemSet.find(dwItemId) == itemSet.end())
      {
        if (publicity_id)
        {
          rev.add_pub_lists(dwItemId);
        }
        else
          rev.add_lists(dwItemId);
      }
      itemSet.insert(dwItemId);
    }

  } while (0);

  //insert cache
  {
    DWORD  CACHE_TIME = 60;
    BriefListCache cache;
    cache.expireTime = now() + CACHE_TIME;
    cache.cmd = rev;
    m_mapBriefListCache[key] = cache;
  }

  //出错也要给客户端返回
  PROTOBUF(rev, send, len);
  sendCmdToMe(charid,  (BYTE*)send, len);
  XDBG << "[交易-挂单总览] charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
}

//获取物品每个价格的详细挂单列表
void RecordTradeMgr::fetchDetailPendingList(QWORD charId, Cmd::DetailPendingListRecordTradeCmd& rev)
{
  const int maxPageCount = 8;   
  DWORD itemId = rev.search_cond().item_id();
  const SExchangeItemCFG*pCfg = ItemConfig::getMe().getExchangeItemCFG(itemId);
  if (pCfg == nullptr)
  {
    XERR << "[交易] [错误的查询id] charid:" << rev.charid() << "item id:" << itemId << XEND;
    return;
  }

  xField field(REGION_DB, DB_TABLE_PENDING_LIST);
  field.m_list["id"] = MYSQL_TYPE_LONG;
  field.m_list["itemid"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["total_count"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["price"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["refine_lv"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["is_overlap"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["itemdata"] = MYSQL_TYPE_BLOB;
  field.m_list["publicity_id"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["endtime"] = MYSQL_TYPE_NEWDECIMAL;

  char orderBy[512];
  bzero(orderBy, sizeof(orderBy));

  switch (rev.search_cond().rank_type())
  {
  case RANKTYPE_ITEM_PRICE_INC:
    snprintf(orderBy, sizeof(orderBy), " order by price,publicity_id desc , rand()");
    break;
  case RANKTYPE_ITEM_PRICE_DES:
    snprintf(orderBy, sizeof(orderBy), " order by price desc,publicity_id desc, rand() ");
    break;
  case RANKTYPE_REFINE_LV_INC:
    snprintf(orderBy, sizeof(orderBy), " order by refine_lv,publicity_id desc, rand() ");
    break;
  case RANKTYPE_REFINE_LV_DES:
    snprintf(orderBy, sizeof(orderBy), " order by refine_lv desc,publicity_id desc, rand() ");
    break;
  default:
    snprintf(orderBy, sizeof(orderBy), " order by publicity_id desc , rand()");
    break;
  }

  time_t currTime = now();
  time_t shelfTime = currTime/60*60 - MiscConfig::getMe().getTradeCFG().dwExpireTime;

  char table[64];
  bzero(table, sizeof(table));
  snprintf(table, sizeof(table), "%s.%s", field.m_strDatabase.c_str(), field.m_strTable.c_str());
  char totalCountSql[1024];
  bzero(totalCountSql, sizeof(totalCountSql));

  if (pCfg->isOverlap)
  {
    //total page count
    char where[512];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), " where itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) AND (`is_overlap` = 1) OR (publicity_id >= 1))  group by `price`",
      itemId,
      shelfTime);

    snprintf(totalCountSql, sizeof(totalCountSql), 
      "select `id`,`itemid`,`price`,COALESCE(sum(count),0) AS `total_count`,`refine_lv`,`is_overlap`, publicity_id, endtime from %s %s",
      table,
    where);
  }
  else
  {
    snprintf(totalCountSql, sizeof(totalCountSql),
      " select `id`,`itemid`,`price`,`count` AS `total_count`,`refine_lv`,`is_overlap` from %s where itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) AND (`is_overlap` = 0) OR (publicity_id >= 1))",
      table,
      itemId,
      shelfTime);
  }


  char sql_hash[1024];
  bzero(sql_hash, sizeof(sql_hash));

  upyun_md5(totalCountSql, sizeof(totalCountSql), sql_hash);
  XDBG << "[交易所]" << totalCountSql << "hash:" << sql_hash;
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TRADE_COUNT, sql_hash);

  DWORD totalCount = 0;
  if (!RedisManager::getMe().getData(key, totalCount))
  {
    std::string sql(totalCountSql);
    xRecordSet tmpSet;
    QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, tmpSet, sql);
    if ((QWORD)-1 == ret)
    {
      XERR << "[交易-详细挂单] fetchDetailPendingList， 查询数据库出错， charid:" << rev.charid() << "ret:" << ret << XEND;
      return;
    }
    //calc total page count
    totalCount = ret;
    RedisManager::getMe().setData(key, totalCount, 60);
  }

  DWORD totalPageCount = totalCount / maxPageCount;
  if (totalCount % maxPageCount)
  {
    totalPageCount += 1;
  }
  rev.set_total_page_count(totalPageCount);

  if (rev.search_cond().page_index() >= rev.total_page_count() && rev.total_page_count())
  {
    rev.mutable_search_cond()->set_page_index(rev.total_page_count() - 1);
  }
  char sqlStr[1024];
  bzero(sqlStr, sizeof(sqlStr));

  DWORD offSet = rev.search_cond().page_index() * maxPageCount;
  if (pCfg->isOverlap)
  {
    //total page count
    char where[512];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), " where itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) and (`is_overlap` = 1) OR (publicity_id >= 1))  group by `price`",
      itemId,
      shelfTime);

    snprintf(totalCountSql, sizeof(totalCountSql),
      "select `id`,`itemid`,`price`,COALESCE(sum(count),0) AS `total_count`,`refine_lv`,`is_overlap`, publicity_id, endtime from %s %s ",
      table,
    where);

    snprintf(sqlStr, sizeof(sqlStr), "select `id`,`itemid`,`price`,COALESCE(sum(count), 0) AS `total_count`,`refine_lv`,`is_overlap`, publicity_id, endtime from %s %s limit %u, %u",
      table,
    where,      
      offSet, maxPageCount);
  }
  else
  {
    snprintf(sqlStr, sizeof(sqlStr),
      " select `id`,`itemid`,`price`,`count` AS `total_count`,`refine_lv`,`is_overlap`, `itemdata`, publicity_id, endtime  from %s where (`is_overlap` = 0) AND itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) OR (publicity_id >= 1)) "
      " %s limit %u, %u",
      table,
      itemId,
      shelfTime,
      orderBy,
      offSet, maxPageCount);
  }

  bzero(sql_hash, sizeof(sql_hash));

  upyun_md5(sqlStr, sizeof(sqlStr), sql_hash);
  XDBG << "[交易所]" << sqlStr << "hash:" << sql_hash;

  string key_sql = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TRADE_DATA, sql_hash);
  if (!RedisManager::getMe().getProtoData(key_sql, &rev) == true)
  {
    std::string sql2(sqlStr);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, set, sql2);
    if ((QWORD)-1 == ret)
    {
      XERR << "[交易-详细挂单] fetchDetailPendingList， 查询数据库出错， charid:" << rev.charid() << "ret:" << ret << XEND;
      return;
    }

    for (DWORD i = 0; i < ret; i++)
    {
      TradeItemBaseInfo* pItem = rev.add_lists();
      pItem->set_order_id(set[i].get<QWORD>("id"));
      pItem->set_itemid(set[i].get<DWORD>("itemid"));
      pItem->set_price(set[i].get<DWORD>("price"));
      pItem->set_count(set[i].get<DWORD>("total_count"));
      pItem->set_refine_lv(set[i].get<DWORD>("refine_lv"));
      pItem->set_overlap(set[i].get<DWORD>("is_overlap"));
      pItem->set_publicity_id(set[i].get<DWORD>("publicity_id"));
      pItem->set_end_time(set[i].get<DWORD>("endtime"));
      std::string itemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
      if (!itemData.empty())
      {
        pItem->mutable_item_data()->ParseFromString(itemData);
        if (ItemConfig::getMe().isGoodEnchant(pItem->item_data()) == false)
        {
          //给客户端显示，清空附魔属性
          pItem->mutable_item_data()->mutable_enchant()->Clear();
        }      
      }
    }

    RedisManager::getMe().setProtoData(key_sql, &rev, 60);
  }

  PROTOBUF(rev, send, len);
  sendCmdToMe(charId, (BYTE*)send, len);

  XDBG << "[交易-详细挂单] charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
}

void RecordTradeMgr::fetchItemSellInfoList(QWORD charId,  Cmd::ItemSellInfoRecordTradeCmd& rev)
{
  const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(rev.itemid());
  if (pCfg == nullptr)
    return;
  
  if (rev.has_publicity_id() && rev.publicity_id())
  { 
    //sell count
    {
      xField field(REGION_DB, DB_TABLE_PENDING_LIST);
      field.m_list["total_count"] = MYSQL_TYPE_NEWDECIMAL;

      char table[64];
      bzero(table, sizeof(table));
      snprintf(table, sizeof(table), "%s.%s", field.m_strDatabase.c_str(), field.m_strTable.c_str());
      char sqlStr[512] = { 0 };
      snprintf(sqlStr, sizeof(sqlStr),
        " select COALESCE(sum(count),0) as total_count from %s where itemid=%u AND publicity_id=%u", table, rev.itemid(), rev.publicity_id());

      std::string sql2(sqlStr);
      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, set, sql2);
      if (ret != 1)
      {
        XERR << "[交易-在售物品信息-公示期] 获取交易所出售数量， 查询数据库出错， charid:" << charId << "ret:" << ret << XEND;
        return;
      }
      DWORD sellCount = set[0].get<DWORD>("total_count");

      rev.set_statetype(St_InPublicity);
      rev.set_count(sellCount);
    }

    //buy people
    {
      xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY);
      if (!field)
        return;
      field->setValid("buy_people");

      char where[32] = {0};
      snprintf(where, sizeof(where),"id=%u AND itemid=%u", rev.publicity_id(), rev.itemid());
      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where);
      if (ret != 1)
      {
        XERR << "[交易-在售物品信息-公示期] 获取购买人数， 查询数据库出错， charid:" << charId << "ret:" << ret << XEND;
        return;
      }
      DWORD buyCount = set[0].get<DWORD>("buy_people");
      rev.set_buyer_count(buyCount);
    }
  }
  else
  {
    if (pCfg->isOverlap)
    {
      //sell count
      xField field(REGION_DB, DB_TABLE_PENDING_LIST);
      // field.setValid("COALESCE(sum(count),0) as total_count");
      field.m_list["total_count"] = MYSQL_TYPE_NEWDECIMAL;

      char table[64];
      bzero(table, sizeof(table));
      snprintf(table, sizeof(table), "%s.%s", field.m_strDatabase.c_str(), field.m_strTable.c_str());

      time_t currTime = now();
      time_t shelfTime = currTime/60*60 - MiscConfig::getMe().getTradeCFG().dwExpireTime;

      char sqlStr[512] = { 0 };
      snprintf(sqlStr, sizeof(sqlStr),
        " select COALESCE(sum(count),0) as total_count from %s where itemid=%u AND (publicity_id=0 AND pendingtime >%lu)", table,
        rev.itemid(), shelfTime);

      std::string sql2(sqlStr);
      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, set, sql2);
      if (ret != 1)
      {
        XERR << "[交易-在售物品信息-非公示期] 获取交易所出售数量， 查询数据库出错， charid:" << charId << "ret:" << ret << XEND;
        return;
      }
      rev.set_statetype(St_OverlapNormal);
      rev.set_count(set[0].get<DWORD>("total_count"));
    }
    else
      rev.set_statetype(St_NonoverlapNormal);

    //item be buyed log
    {
      xField field(REGION_DB, DB_TABLE_BUYED_LIST);
      field.m_list["buyername"] = MYSQL_TYPE_VAR_STRING;
      field.m_list["tradetime"] = MYSQL_TYPE_NEWDECIMAL;
      char table[64];
      bzero(table, sizeof(table));
      snprintf(table, sizeof(table), "%s.%s", field.m_strDatabase.c_str(), field.m_strTable.c_str());

      char sqlStr[512] = { 0 };
      snprintf(sqlStr, sizeof(sqlStr),
        " select buyerid, buyername, tradetime from %s where itemid=%u AND logtype=%u AND buyername<>\"\" group by buyerid order by tradetime desc limit 6", table,
        rev.itemid(), EOperType_NoramlBuy);

      std::string sql2(sqlStr);
      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, set, sql2);
      if (ret == QWORD_MAX)
      {
        XERR << "[交易-在售物品信息-非公示期] 获取交易出售记录， 查询数据库出错， charid:" << charId << "ret:" << ret << XEND;
        return;
      }

      for (DWORD i = 0; i < ret; ++i)
      {
        BriefBuyInfo* pInfo = rev.add_buy_info();
        if (pInfo)
        {
          pInfo->set_name(set[i].getString("buyername"));
          pInfo->set_time(set[i].get<DWORD>("tradetime"));
        }
      }
    }
  }

  PROTOBUF(rev, send, len);
  sendCmdToMe(charId, (BYTE*)send, len); 
  XDBG << "[交易-在售物品信息] charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
}

//获取我的挂单列表
void RecordTradeMgr::fetchMyPendingList(QWORD charId,  Cmd::MyPendingListRecordTradeCmd& rev)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
    return;
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "sellerid=%lu order by pendingtime desc", rev.charid());
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-数据库] 我的挂单列表 数据库错误 charid：" << rev.charid() << "table: " << DB_TABLE_PENDING_LIST << XEND;
    return;
  }

  DWORD curTime = now();

  for (QWORD i = 0; i < ret; ++i)
  {
    TradeItemBaseInfo* pItem = rev.add_lists();
    pItem->set_order_id(set[i].get<QWORD>("id"));
    pItem->set_itemid(set[i].get<DWORD>("itemid"));
    pItem->set_price(set[i].get<DWORD>("price"));
    pItem->set_count(set[i].get<DWORD>("count"));  
    pItem->set_overlap(set[i].get<DWORD>("is_overlap"));
    pItem->set_refine_lv(set[i].get<DWORD>("refine_lv"));
    pItem->set_publicity_id(set[i].get<DWORD>("publicity_id"));
    pItem->set_end_time(set[i].get<DWORD>("endtime"));

    std::string itemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
    if (!itemData.empty())
      pItem->mutable_item_data()->ParseFromString(itemData);
    if (curTime >= set[i].get<QWORD>("pendingtime") && (pItem->publicity_id() == 0))
    {
      time_t diffTime = curTime - set[i].get<QWORD>("pendingtime");
      if (diffTime > MiscConfig::getMe().getTradeCFG().dwExpireTime)
      {
        pItem->set_is_expired(true);
      }
      else
      {
        pItem->set_is_expired(false);
      }
    }
  }

  XDBG << "[交易-我的挂单列表] 请求返回：" << rev.ShortDebugString() << XEND;
  PROTOBUF(rev, send, len);
  sendCmdToMe(charId, (BYTE*)send, len);
}

bool RecordTradeMgr::canMerge(Cmd::EOperType type, DWORD itemId, DWORD price, DWORD time)
{
  // if (m_lastIter == m_vecItemInfo.rend())
  //   return false;
  // if (m_lastIter->logtype() != type)
  //   return false;
  // if (m_lastIter->itemid() != itemId)
  //   return false;
  // if (m_lastIter->price() != price)
  //   return false;
  //int diff = m_lastIter->time() - time;     //已经按照时间从大到小排序
  //if (diff > 20)
  //  return false;

  return true;
}

////获取我的购买出售记录
//void RecordTradeMgr::fetchMyTradeLogList(QWORD charId,  Cmd::MyTradeLogRecordTradeCmd& rev)
//{
//  //购买记录
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_BUYED_LIST);
//    if (!field)
//      return;//
//    char where[128];
//    bzero(where, sizeof(where));
//
//    snprintf(where, sizeof(where), "buyerid=%llu order by tradetime desc limit %u", charId, MiscConfig::getMe().getTradeCFG().dwMaxLogCount);
//    xRecordSet set;
//    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//    if (ret == QWORD_MAX)
//    {
//      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_BUYED_LIST << XEND;
//      return;
//    }
//    Cmd::EOperType type;
//    DWORD itemId;
//    DWORD price;
//    DWORD time;
//    DWORD count;
//    m_vecItemInfo.clear();
//    m_lastIter = m_vecItemInfo.rbegin();
//    for (QWORD i = 0; i < ret; ++i)
//    {
//      LogItemInfo logInfo;
//      type = EOperType(set[i].get<DWORD>("logtype"));
//      itemId = set[i].get<DWORD>("itemid");
//      price = set[i].get<DWORD>("price");
//      time = set[i].get<DWORD>("tradetime");            
//      count = set[i].get<DWORD>("count");
//
//      if (canMerge(type, itemId, price, time))
//      {
//        m_lastIter->set_count(m_lastIter->count() + count);
//        m_lastIter->set_time(time);
//      }
//      else
//      {
//        logInfo.set_logtype(type);
//        logInfo.set_itemid(itemId);
//        logInfo.set_count(count);
//        logInfo.set_price(price);
//        logInfo.set_time(time);
//        std::string itemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
//        if (!itemData.empty())
//          logInfo.mutable_item_data()->ParseFromString(itemData);
//
//        m_vecItemInfo.push_back(logInfo);
//        m_lastIter = m_vecItemInfo.rbegin();
//      }
//    }
//
//    for (auto &v : m_vecItemInfo)
//    {
//      LogItemInfo* pInfo = rev.add_buy_lists();
//      pInfo->CopyFrom(v);
//    }
//    m_vecItemInfo.clear();
//    m_lastIter = m_vecItemInfo.rbegin();
//  }
//
//  //出售记录
//  {
//    xField *field = xFieldsM::getMe().getField(REGION_DB, DB_TABLE_SELLED_LIST);
//    if (!field)
//      return;//
//    char where[128];
//    bzero(where, sizeof(where));
//
//    snprintf(where, sizeof(where), "sellerid=%llu order by tradetime desc limit %u", charId, MiscConfig::getMe().getTradeCFG().dwMaxLogCount);
//    xRecordSet set;
//    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
//    if (ret == QWORD_MAX)
//    {
//      XERR << "[交易-数据库] 数据库错误 charid：" << charId << "table: " << DB_TABLE_SELLED_LIST << XEND;
//      return;
//    }
//
//    LogItemInfo logInfo;
//    Cmd::EOperType type;
//    DWORD itemId;
//    DWORD price;
//    DWORD time;
//    DWORD count;
//    QWORD buyerId;
//    m_vecItemInfo.clear();
//    m_lastIter = m_vecItemInfo.rbegin();
//    TSetQWORD setPeople;
//    for (QWORD i = 0; i < ret; ++i)
//    {
//      type = EOperType(set[i].get<DWORD>("logtype"));
//      itemId = set[i].get<DWORD>("itemid");
//      price = set[i].get<DWORD>("price");
//      time = set[i].get<DWORD>("tradetime");
//      count = set[i].get<DWORD>("count");
//      buyerId = set[i].get<QWORD>("buyerid");
//
//      if (canMerge(type, itemId, price, time))
//      {
//        if (setPeople.find(buyerId) == setPeople.end())
//        {
//          m_lastIter->set_people_count(m_lastIter->people_count() + 1);
//          setPeople.insert(buyerId);
//        }
//        m_lastIter->set_count(m_lastIter->count() + count);
//        m_lastIter->set_time(time);
//      }
//      else
//      {
//        setPeople.clear();
//        setPeople.insert(buyerId);
//        logInfo.set_logtype(type);
//        logInfo.set_itemid(itemId);
//        logInfo.set_count(count);
//        logInfo.set_price(price);
//        logInfo.set_time(time);
//        logInfo.set_people_count(1);
//        std::string itemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
//        if (!itemData.empty())
//          logInfo.mutable_item_data()->ParseFromString(itemData);
//
//        m_vecItemInfo.push_back(logInfo);
//        m_lastIter = m_vecItemInfo.rbegin();
//      }
//    }
//
//    for (auto &v : m_vecItemInfo)
//    {
//      LogItemInfo* pInfo = rev.add_sell_lists();
//      pInfo->CopyFrom(v);
//    }
//    m_vecItemInfo.clear();
//    m_lastIter = m_vecItemInfo.rbegin();
//  }
//  XINF << "[交易-我的交易记录] 请求返回：" << rev.ShortDebugString() << XEND;
//
//  PROTOBUF(rev, send, len);
//  sendCmdToMe(charId, (BYTE*)send, len);
//}

//////////////////////////////////////////////////////////////////////////
bool TradeMsgGuard::lock(DWORD type, QWORD charId)
{
  DWORD key = type;
  QWORD dwMsgInverval = 10;
  auto it = m_mapGuard.find(key);
  QWORD curTime = xTime::getCurMSec();
  if (it == m_mapGuard.end())
  {
    std::unordered_map<QWORD, QWORD> subMap({{charId, curTime}});
    m_mapGuard.insert(std::make_pair(key, subMap));
  }
  else
  {
    auto subIt = it->second.find(charId);
    if (subIt == it->second.end())
    {
      it->second.insert(std::make_pair(charId, curTime));
    }
    else
    {
      if (key == SELL_ITEM_RECORDTRADE || key == RESELL_PENDING_RECORDTRADE)
      {
        dwMsgInverval = MSG_INTERVAL2;
      }
      else
      {
        dwMsgInverval = MSG_INTERVAL1;
      }

      if (curTime > subIt->second + dwMsgInverval)
      {
        subIt->second = curTime;
        return true;
      }

      XLOG << "[交易-协议检测] 请求过于频繁，param:" << key << "charid:" << charId << "lasttime:" << subIt->second << XEND;
      return false;
    }
  }
  return true;
}

bool TradeMsgGuard::unlock(DWORD type, QWORD charId)
{
  DWORD key = type;

  auto it = m_mapGuard.find(key);
  if (it == m_mapGuard.end())
  {
    return false;
  }

  auto subIt = it->second.find(charId);
  if (subIt == it->second.end())
  {
    return false;
  }

  it->second.erase(subIt);
  return true;
}

MessageStatHelper::MessageStatHelper(DWORD cmd, DWORD param)
{
  RecordTradeMgr::getMe().m_oMessageStat.start(cmd, param);
}

MessageStatHelper::~MessageStatHelper()
{
  RecordTradeMgr::getMe().m_oMessageStat.end();
}
