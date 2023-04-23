#include "AuctionManager.h" //拍卖管理器头文件
#include "AuctionServer.h"//拍卖服务头文件 
#include "CommonConfig.h"//通用配置头文件
#include "AuctionServer.h"//拍卖服务头文件
#include "MiscConfig.h"//杂项配置头文件
#include "TradePriceMgr.h"//交易价格管理头文件
#include "ItemConfig.h" //道具配置头文件
#include "LuaManager.h"//LUA管理器头文件
#include "RedisManager.h"//Redis管理器头文件 
#include "SceneTip.pb.h"//场景提示protobuf头文件  
#include "SceneTrade.pb.h"//场景交易protobuf头文件

/************************************************************************/
/* SAuctionEvent    拍卖活动                                                                 */
/************************************************************************/
void SAuctionEvent::toRecord(xRecord& s) 
{ 
s.put("batchid", m_qwBatchId); 
//放入拍卖批次ID
s.put("event", m_eEvent);  
//放入拍卖事件 
s.put("itemid", m_dwItemId);
 //放入物品ID
s.put("time", m_dwTime);  
//放入时间
s.put("price", m_qwPrice); 
//放入价格 
s.putString("name", m_strName); 
//放入名称 
s.put("zoneid", m_dwZoneId); 
//放入区域ID
s.put("max_price", m_qwMaxPrice);  
//放入最高价格
s.put("player_id", m_qwCharId);  
//放入玩家ID 
s.put("signup_id", m_qwSignupId); 
//放入报名ID
}

/************************************************************************/
/*OfferPriceRecord                                                                      */
/************************************************************************/
void OfferPriceRecord::toRecord(xRecord& s, bool id)
{
  if (id) 
    s.put("id", m_qwId); 
     //如果id为真,则将id放入xRecord中
  s.put("batchid", m_qwBatchId); 
  //将拍卖批次ID放入xRecord中
  s.put("itemid", m_dwItemId);
  //将物品ID放入xRecord中
  s.put("charid", m_qwCharId);
 //将角色ID放入xRecord中 
  s.putString("name", m_strName); 
  //将名称放入xRecord中
  s.put("zoneid", m_dwZoneId);
  //将区域ID放入xRecord中
  s.put("price", m_qwPrice);
  //将价格放入xRecord中 
  s.put("type", m_eType);
  //将类型放入xRecord中 
  s.put("take_status", m_eTakestatus);
  //将提取状态放入xRecord中
  s.put("verify_time", m_dwVerifyTime); 
  //将验证时间放入xRecord中
  s.put("time", m_dwTime);
  //将时间放入xRecord中
  s.put("signup_id", m_qwSignupId);
  //将报名ID放入xRecord中 
  std::string strItemData;
  m_oItemData.SerializeToString(&strItemData);
  s.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size()); 
   //将物品数据序列化并放入xRecord中
} 

void OfferPriceRecord::fromRecord(const xRecord& s)
{
  m_qwId = s.get<QWORD>("id");
  m_qwBatchId = s.get<QWORD>("batchid");
  m_dwItemId = s.get<DWORD>("itemid");
  m_qwCharId = s.get<QWORD>("charid");
  m_strName = s.getString("name");
  m_dwZoneId = s.get<DWORD>("zoneid");
  m_qwPrice = s.get<DWORD>("price");
  m_eType = static_cast<Cmd::ERecordType>(s.get<DWORD>("type"));
  m_eTakestatus = static_cast<Cmd::EAuctionTakeStatus>(s.get<DWORD>("take_status"));
  m_dwVerifyTime = s.get<DWORD>("verify_time");
  m_dwTime = s.get<DWORD>("time");
  m_qwSignupId = s.get<QWORD>("signup_id");

  std::string strItemData;
  strItemData.assign((const char*)s.getBin("itemdata"), s.getBinSize("itemdata"));
  m_oItemData.ParseFromString(strItemData);
}

/************************************************************************/
/*SignupItem                                                                      */
/************************************************************************/
void SignupItem::toRecord(xRecord& s, bool id)
{
  if (id)
    s.put("id", m_qwId);

  s.put("batchid", m_qwBatchId);
  s.put("itemid", m_dwItemId);
  s.put("charid", m_qwSellerId);
  s.putString("name", m_strSellerName);
  s.put("zoneid", m_dwZoneId);
  s.put("type", m_eType);
  s.put("take_status", m_eTakestatus);
  s.put("offerprice_id", m_qwOfferPriceId);
  s.put("verify_time", m_dwVerifyTime);
  s.put("time", now());
  s.put("base_price", m_qwBasePrice);
  s.put("zeny_price", m_qwZenyPrice);
  s.put("fm_point", m_dwFmPoint);
  s.put("fm_buff", m_dwFmBuff);

  std::string strItemData;
  m_oItemData.SerializeToString(&strItemData);
  s.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());
}

void SignupItem::fromRecord(const xRecord& s)
{
  m_qwId = s.get<QWORD>("id");
  m_qwBatchId = s.get<QWORD>("batchid");
  m_dwItemId = s.get<DWORD>("itemid");
  m_qwSellerId = s.get<QWORD>("charid");
  m_strSellerName = s.getString("name");
  m_dwZoneId = s.get<DWORD>("zoneid");
  m_eType = static_cast<Cmd::ERecordType>(s.get<DWORD>("type"));
  m_eTakestatus = static_cast<Cmd::EAuctionTakeStatus>(s.get<DWORD>("take_status"));
  m_qwOfferPriceId = s.get<QWORD>("offerprice_id");
  m_dwVerifyTime = s.get<DWORD>("verify_time");
  m_qwBasePrice = s.get<QWORD>("base_price");
  m_qwZenyPrice = s.get<QWORD>("zeny_price");
  m_dwFmPoint = s.get<DWORD>("fm_point");
  m_dwFmBuff = s.get<DWORD>("fm_buff");

  std::string strItemData;
  strItemData.assign((const char*)s.getBin("itemdata"), s.getBinSize("itemdata"));
  m_oItemData.ParseFromString(strItemData);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool AuctionUser::hasSignuped(DWORD itemId, QWORD batchId)
{
  auto it = m_mapSignupItem.find(itemId);
  if (it == m_mapSignupItem.end())
    return false;
  return true;
}

/*通知个人上架信息*/
void AuctionUser::ntfMySignupInfo(QWORD qwCurBatchId)
{
  if (m_bLoadSignUpFrom == false)
  {
    //load from db
    loadSignupInfoFromDb(qwCurBatchId);
    m_bLoadSignUpFrom = true;
  }
  
  NtfMySignUpInfoCCmd cmd;

  for (auto&m :m_mapSignupItem)
  {
    cmd.add_signuped(m.first);
  }
  
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(m_qwCharId, m_dwZoneId, send, len);
}

/*上架锁物品*/
bool AuctionUser::lockSignup(DWORD itemId)
{
  DWORD curSec = now();
  auto it = m_mapSignupLock.find(itemId);
  if (it != m_mapSignupLock.end())
  {
    if (curSec < it->second)
      return false;
    it->second = curSec + 500;
    return true;
  }

  m_mapSignupLock[itemId] = curSec + 500;
  
  return true;
}
/*上架解锁物品*/
bool AuctionUser::unlockSignup(DWORD itemId)
{
  m_mapSignupLock.erase(itemId);
  return true;
}

void AuctionUser::addSignupItem(const SignupItem& item)
{
  auto it = m_mapSignupItem.find(item.m_dwItemId);
  if (it != m_mapSignupItem.end())
  {
    return;
  }  
  m_mapSignupItem.insert(std::make_pair(item.m_dwItemId, item));
}

void AuctionUser::loadSignupInfoFromDb(QWORD qwCurBatchId)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
  if (!field)
  {
    return;
  }
  char where[64];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "batchid=%llu and charid=%llu ", qwCurBatchId, m_qwCharId);

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    return;
  }
  
  for (QWORD i = 0; i < ret; ++i)
  {
    SignupItem& rSignupItem = m_mapSignupItem[set[i].get<DWORD>("itemid")];
    rSignupItem.fromRecord(set[i]);
  }
}

void AuctionUser::newBranch()
{
  m_mapSignupItem.clear();
  m_mapSignupLock.clear();  
}

bool AuctionUser::lockOfferPrice(QWORD batchId, QWORD signupId)
{
  DWORD curSec = now();

  auto it = m_mapOfferPriceLock.find(batchId);
  if (it != m_mapOfferPriceLock.end())
  {
    auto subMap = it->second;
    auto it2 = subMap.find(signupId);
    if (it2 != subMap.end())
    {
      if (curSec < it2->second)
        return false;
      it2->second = curSec + 500;
      return true;
    }
  }
  m_mapOfferPriceLock[batchId][signupId] = curSec + 500;
  return true;
}

bool AuctionUser::unlockOfferPrice(QWORD batchId, QWORD signupId)
{
  auto it = m_mapOfferPriceLock.find(batchId);
  if (it != m_mapOfferPriceLock.end())
  {
    it->second.erase(signupId);
  }
  return true;
}

void AuctionUser::newItemAuction(DWORD itemId)
{

}

QWORD AuctionUser::getReduceMoney(QWORD batchId, DWORD itemId, QWORD signupId, QWORD totalMoney)
{
  loadOfferPriceFromDb(batchId, itemId, signupId);

  if (isLastOfferPrice(batchId, itemId, signupId))
  {
    if (totalMoney <= m_stCurAuctionOfferPrice.m_qwPrice)
      return 0;
    else
      return totalMoney - m_stCurAuctionOfferPrice.m_qwPrice;
  }
  return totalMoney;
}

//判断是否是上次的出价
bool AuctionUser::isLastOfferPrice(QWORD batchId, DWORD itemId, QWORD signupId)
{
  if (m_stCurAuctionOfferPrice.m_qwBatchId == 0)
    return false;  
  if (m_stCurAuctionOfferPrice.m_qwBatchId == batchId && m_stCurAuctionOfferPrice.m_dwItemId == itemId && m_stCurAuctionOfferPrice.m_qwSignupId == signupId)
    return true;
  return false;
}

bool AuctionUser::addOfferPriceRecord(OfferPriceRecord& r)
{
  if (isLastOfferPrice(r.m_qwBatchId, r.m_dwItemId, r.m_qwSignupId))
  { //update
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
    if (!field)
    {
      XERR << "[拍卖行-更新出价] 找不到数据表， charid" << m_qwCharId << "zoneid" << m_dwZoneId << "场次" << r.m_qwBatchId << "itemid" << r.m_dwItemId << "价格" << r.m_qwPrice << XEND;
      return false;
    }
    xRecord record(field);
    record.put("price", r.m_qwPrice);
    record.put("time", r.m_dwTime);
    record.put("type", r.m_eType);
    record.put("take_status", r.m_eTakestatus);
    
    char where[64] = { 0 };
    snprintf(where, sizeof(where), "id=%llu and take_status != %u", m_stCurAuctionOfferPrice.m_qwId, EAuctionTakeStatus_Took);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[拍卖行-更新出价] 数据库操作失败， charid" << m_qwCharId << "zoneid" << m_dwZoneId <<"ret" << ret << "场次" << r.m_qwBatchId << "itemid" << r.m_dwItemId << "价格" << r.m_qwPrice << "数据库id" << m_stCurAuctionOfferPrice.m_qwId << XEND;
      return false;
    }
    m_stCurAuctionOfferPrice.m_qwPrice = r.m_qwPrice;
    m_stCurAuctionOfferPrice.m_dwTime = r.m_dwTime;
    m_stCurAuctionOfferPrice.m_eType = r.m_eType;
    m_stCurAuctionOfferPrice.m_eTakestatus = r.m_eTakestatus;
    m_stCurAuctionOfferPrice.m_dwZoneId = r.m_dwZoneId;
    
    r = m_stCurAuctionOfferPrice;
    XLOG << "[拍卖行-更新出价] 成功， charid" << m_qwCharId << "zoneid" << m_dwZoneId << "场次" << r.m_qwBatchId << "itemid" << r.m_dwItemId << "价格" << r.m_qwPrice <<"数据库id" <<m_stCurAuctionOfferPrice.m_qwId << XEND;
    return true;
  }
  else
  {
    //insert new
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
    if (!field)
    {
      XERR << "[拍卖行-插入出价] 找不到数据表， charid" << m_qwCharId << "zoneid" << m_dwZoneId << "场次" << r.m_qwBatchId << "itemid" << r.m_dwItemId << "价格" << r.m_qwPrice << XEND;
      return false;
    }
    xRecord record(field);
    r.toRecord(record, false);
    QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
    if (ret == QWORD_MAX)
    {
      XERR << "[拍卖行-插入出价] 插入数据库失败， charid" << m_qwCharId << "zoneid" << m_dwZoneId << "场次" << r.m_qwBatchId << "itemid" << r.m_dwItemId << "价格" << r.m_qwPrice << XEND;
      return false;
    }
    r.m_qwId = ret;
    
    if (AuctionManager::getMe().isNowAuctionItem(r.m_qwBatchId, r.m_dwItemId, r.m_qwSignupId))
      m_stCurAuctionOfferPrice = r;

    XLOG << "[拍卖行-插入出价] 成功， charid" << m_qwCharId << "zoneid" << m_dwZoneId << "场次" << r.m_qwBatchId << "itemid" << r.m_dwItemId << "价格" << r.m_qwPrice << "数据库id" << m_stCurAuctionOfferPrice.m_qwId << XEND;
    return true;
  }
  return false;
}

void AuctionUser::ntfMyOfferPriceInfo(QWORD qwCurBatchId, DWORD itemId, QWORD signupId)
{
  if (!isLastOfferPrice(qwCurBatchId, itemId, signupId))
  {
    m_stCurAuctionOfferPrice.clear();
    loadOfferPriceFromDb(qwCurBatchId, itemId, signupId);
  }

  NtfMyOfferPriceCCmd cmd;
  cmd.set_batchid(qwCurBatchId);
  cmd.set_itemid(itemId);
  cmd.set_signup_id(signupId);
  cmd.set_my_price(m_stCurAuctionOfferPrice.m_qwPrice);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(m_qwCharId, m_dwZoneId, send, len);
  XLOG << "[拍卖行-推送我的出价] 场次" << qwCurBatchId <<"itemid"<<itemId <<"charid"<<m_qwCharId <<"zoneid"<< m_dwZoneId <<"我的出价" <<m_stCurAuctionOfferPrice.m_qwPrice<< XEND;
}

void AuctionUser::clearOfferPrice(QWORD batchId, DWORD itemId, QWORD signupId)
{
  if (!isLastOfferPrice(batchId, itemId, signupId))
    return;
  m_stCurAuctionOfferPrice.clear();

  ntfMyOfferPriceInfo(batchId, itemId, signupId);
}

bool AuctionUser::loadOfferPriceFromDb(QWORD qwCurBatchId, DWORD itemId, QWORD signupId)
{
  if (m_bLoadOfferPriceFromDb == true)
    return false;
  
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
  if (!field)
  {
    return false;
  }
  char where[256];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "batchid=%llu and charid=%llu and itemid =%u and signup_id =%llu and type=%u and take_status=%u ", qwCurBatchId, m_qwCharId, itemId, signupId, ERecordType_OverTakePrice, EAuctionTakeStatus_CanTake);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    return false;
  }
  
  m_bLoadOfferPriceFromDb = true;

  if (ret == 1)
  {
    m_stCurAuctionOfferPrice.fromRecord(set[0]);
  }
  else
    return false;
  return true;
}

void AuctionUser::sendOverTakeCmd()
{
  NtfOverTakePriceCCmd cmd;
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(m_qwCharId,m_dwZoneId, send, len);
  XDBG <<"[拍卖行-通知] 通知玩家出价被超过了，charid"<<m_qwCharId <<"zoneid"<<m_dwZoneId << XEND;
}

bool AuctionUser::addOrderId(QWORD orderId)
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_AUCTION_ORDERIDSET, m_qwCharId);
  
  return RedisManager::getMe().addSet(key, orderId);
}

bool AuctionUser::delOrderId(QWORD orderId)
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_AUCTION_ORDERIDSET, m_qwCharId);
  
  if (RedisManager::getMe().setExist(key, orderId) == false)
    return false;
  
  OfferPriceDelOrderSCmd cmd;
  cmd.set_orderid(orderId);
  cmd.set_charid(m_qwCharId);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSceneServer(m_qwCharId, m_dwZoneId, send, len);

  return RedisManager::getMe().delSet(key, orderId);
}

void AuctionUser::sendRedTip()
{
  if (!m_bOnline)
    return;

  GameTipCmd  tipCmd;
  RedTip* pRedTip = tipCmd.add_redtip();
  if (pRedTip)
  {
    pRedTip->set_redsys(EREDSYS_AUCTION_RECORD);
  }
  tipCmd.set_opt(ETIPOPT_UPDATE);
  PROTOBUF(tipCmd, send, len);
  thisServer->sendCmdToClient(m_qwCharId, m_dwZoneId, send, len);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void OrderItem::toRecord(xRecord& s)
{
  s.put("batchid", m_qwBatchId);
  s.put("itemid", m_dwItemId);
  s.put("base_price", m_qwBasePrice);
  s.put("zeny_price", m_qwZenyPrice);
  s.put("choose", m_bChoose);
  s.put("orderid", m_dwOrder);
  s.put("signup_id", m_qwSignupId);
  s.put("offerprice_id", m_qwOfferPriceId);
  s.put("status", m_eStatus);
  s.put("trade_price", m_qwTradePrice);
  s.put("time", m_dwTime);
  s.put("auction", m_dwAuction);
  s.put("istemp", m_dwIsTemp);
  s.put("isread", m_dwIsRead);
}

void OrderItem::fromRecord(const xRecord& s)
{
  m_qwBatchId = s.get<QWORD>("batchid");
  m_dwItemId = s.get<DWORD>("itemid");
  m_qwBasePrice = s.get<QWORD>("base_price");
  m_qwZenyPrice = s.get<QWORD>("zeny_price");
  m_bChoose = s.get<DWORD>("choose");
  m_dwOrder = s.get<DWORD>("orderid");
  m_qwSignupId = s.get<DWORD>("signup_id");
  m_qwOfferPriceId = s.get<QWORD>("offerprice_id");
  m_eStatus = static_cast<Cmd::EAuctionResult>(s.get<DWORD>("status"));
  m_qwTradePrice = s.get<QWORD>("trade_price");
  m_dwTime = s.get<DWORD>("time");
  m_dwAuction = s.get<DWORD>("auction");
  m_dwIsTemp = s.get<DWORD>("istemp");
  m_dwIsRead = s.get<DWORD>("isread");
}

void OrderItem::fillSignupItem()
{
  if (m_qwSignupId == 0)
  {
    return;
  }

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
  if (!field)
  {
    return;
  }
  char where[32];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%llu ", m_qwSignupId);

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    return;
  }
  if (ret != 1)
  {
    return;
  }  
  m_stSignupItem.fromRecord(set[0]);
}

void OrderItem::fillOfferPrice()
{
  if (m_qwOfferPriceId == 0)
  {
    return;
  }

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
  if (!field)
  {
    return;
  }
  char where[32];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%llu ", m_qwOfferPriceId);

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    return;
  }
  if (ret != 1)
  {
    return;
  }
  m_stMaxPrice.fromRecord(set[0]);
}

void OrderItem::toAuctionItemInfo(AuctionItemInfo* pInfo)
{
  if (!pInfo)
    return;
  pInfo->set_itemid(m_dwItemId);
  pInfo->set_price(m_qwBasePrice);
  pInfo->set_seller(m_stSignupItem.m_strSellerName);
  pInfo->set_sellerid(m_stSignupItem.m_qwSellerId);
  pInfo->set_result(m_eStatus);
  if (m_eStatus == EAuctionResult_Sucess)
    pInfo->set_trade_price(m_stMaxPrice.m_qwPrice);
  pInfo->set_auction_time(m_dwAuctionTime);
  QWORD curPrice = m_qwBasePrice;
  if (m_stMaxPrice.m_qwPrice)
    curPrice = m_stMaxPrice.m_qwPrice;
  pInfo->set_cur_price(curPrice);
  pInfo->set_mask_price(m_dwMaskPrice);

  pInfo->set_signup_id(m_qwSignupId);
  pInfo->mutable_itemdata()->CopyFrom(m_stSignupItem.m_oItemData);
}

// 获取拍卖最高价
QWORD getAuctionMaxPrice()
{
  QWORD qwMaxPrice = MiscConfig::getMe().getAuctionMiscCFG().qwMaxPrice;
  if(0 == qwMaxPrice)
  {
    qwMaxPrice = AUCTION_MAX_PRICE;
    XERR << "[拍卖行-竞拍出价] 获取配置最高价失败，采用默认最高价：" << qwMaxPrice << XEND;
  }

  return qwMaxPrice;
}

/************************************************************************/
/*AuctionManager                                                                      */
/************************************************************************/
AuctionManager::AuctionManager() :m_oneSecTimer(1), m_fiveSecTimer(5), m_FiveMinTimer(5 * 60), m_ThirtyMinTimer(30 * 60), m_oneHourTimer(3600)
{
  m_oMessageStat.setFlag("拍卖行");
}

AuctionManager::~AuctionManager()
{
}

void AuctionManager::init()
{ 
  m_qwCurBatchId = 0;
  m_eAuctionState = EAuctionState_Close;
  loadDb();
  m_curAuctionOrderIter = m_vecAuctionItem.end();
}

bool AuctionManager::loadDb()
{
  //加载场次信息
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_CONFIG);
    if (field)
    {
      XINF << "[拍卖行-初始化加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
      xRecordSet set;

      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, NULL, NULL);
      if (ret == QWORD_MAX)
      {
        XERR << "[拍卖行-初始化加载数据库] 数据库错误" << XEND;
        return false;
      }
      for (DWORD i = 0; i < ret; ++i)
      {
        QWORD batchId = set[i].get<QWORD>("id");
        if (batchId < m_qwCurBatchId)
          continue; 
        m_qwCurBatchId = batchId;
        m_eAuctionState = static_cast<Cmd::EAuctionState>(set[i].get<DWORD>("state"));
        m_dwAuctionBeginTime = set[i].get<DWORD>("begin_time");
        m_dwSignupEndTime = set[i].get<DWORD>("verify_time");
        m_dwAuctionEndTime = set[i].get<DWORD>("end_time");    
        m_dwBatchCreateTime = set[i].get<DWORD>("create_time");
      }
    }
    else
    {
      XERR << "[拍卖行-初始化加载数据库] 数据库错误，找不到表" << XEND;
      return false;
    }
  }
  
  setAuctionBeginTime(m_dwAuctionBeginTime);
  
  DWORD curSec = now();
  
  if (m_eAuctionState == EAuctionState_SignUp || m_eAuctionState == EAuctionState_SignUpVerify || m_eAuctionState == EAuctionState_AuctionPublicity)
  {
    if (curSec >= m_dwAuctionBeginTime)
    { 
      //延迟到下一个拍卖日
      m_dwAuctionBeginTime = calcAuctionBeginTime(curSec);
      setAuctionBeginTime(m_dwAuctionBeginTime);
      saveAuctionConfig2Db();
      XLOG << "[拍卖行-初始化] 场次" << m_qwCurBatchId <<"状态" << m_eAuctionState << "延到下一个拍卖日" << m_dwAuctionBeginTime  << XEND;
    }
  }  
  else if (m_eAuctionState == EAuctionState_AuctionEnd)
  {
    //根据拍卖结束时间计算报名开始时间
    calcSignupTime();
  }
  
  m_bFirstInit = true;

  XLOG << "[拍卖行-初始化] 加载批次配置成功" << "当前批次" << m_qwCurBatchId << "状态" << m_eAuctionState << "拍卖开始时间" << m_dwAuctionBeginTime << "拍卖结束时间" << m_dwAuctionEndTime <<"报名开始时间" << m_dwSignupBeginTime << "报名截止时间" << m_dwSignupEndTime << XEND;
  return true;
}

bool AuctionManager::loadConfig()
{
  std::map<DWORD, DWORD> mapCanSignup;
  loadCanSignup(mapCanSignup);
  
  std::set<const SExchangeItemCFG*> setAdd;
  auto& exchangeItemMap = ItemConfig::getMe().getExchangeItemCFGMap();
  for (auto it = exchangeItemMap.begin(); it != exchangeItemMap.end(); ++it)
  {
    const SExchangeItemCFG& rCfg = it->second;
    auto i = mapCanSignup.find(rCfg.dwId);
    if (i == mapCanSignup.end())
    {
      setAdd.insert(&rCfg);
    }
  }
  //add
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_CAN_SIGNUP);
  if (!field)
  {
    return false;
  }

  for (auto&s : setAdd)
  {
    if (!s)
    {
      continue;
    }
    xRecord record(field);
    record.put("itemid", s->dwId);
    record.put("auction", s->eAuctionSignupType);
    QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
    if (ret == QWORD_MAX)
    {
      //插入失败
      continue;
    }
  }
  
  TSetDWORD setDel;
  for (auto it = mapCanSignup.begin(); it != mapCanSignup.end(); ++it)
  {
    const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(it->first);
    if (pCfg == nullptr)
    {
      setDel.insert(it->first);
    }
  }
  
  for (auto&s : setDel)
  {
    if (!s)
    {
      continue;
    }

    char where[64] = { 0 };
    snprintf(where, sizeof(where), "itemid=%d", s);
    QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
    if (ret == QWORD_MAX)
    {
      continue;
    }
  }
  return true;
}

bool AuctionManager::doUserCmd(QWORD charId,const std::string& name, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  MessageStatHelper msgStat(cmd->cmd, cmd->param);
  if (!MsgGuard::getMe().lock(cmd->param, charId))
    return true;
  
  switch (cmd->param)
  {
  case AUCTIONCPARAM_REQ_ACUTION_INFO:
  {   
    onOpenPanel(charId, name, zoneId);
    ntfAuctionInfo(charId, zoneId);
    return true;
  }
  case AUCTIONCPARAM_OPEN_AUCTION_PANEL:
  {
    PARSE_CMD_PROTOBUF(Cmd::OpenAuctionPanelCCmd, rev);
    if (rev.open())
    {
      onOpenPanel(charId, name, zoneId);
    }
    else
    {
      onClosePanel(charId, zoneId);
    }
    return true;
  }
  case AUCTIONCPARAM_SIGNUP_ITEM:
  {
    PARSE_CMD_PROTOBUF(Cmd::SignUpItemCCmd, rev);
    
    signup(charId, zoneId, rev);
    return true;
  }
  case AUCTIONCPARAM_OFFER_PRICE:
  {
    PARSE_CMD_PROTOBUF(Cmd::OfferPriceCCmd, rev);

    offerPrice(charId, zoneId, rev);
    return true;
  }
  case AUCTIONCPARAM_REQ_LAST_AUCTION_INFO:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReqLastAuctionInfoCCmd, rev);
    if (m_qwCurBatchId <= 1)
    {
      thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_NO_LAST_LOG);
      return true;
    }

    getLastAuctionInfo();

    if (m_cacheLastAuction.has_batchid())
    {
      PROTOBUF(m_cacheLastAuction, send, len);
      thisServer->sendCmdToClient(charId, zoneId, send, len);
    }
    return true;
  }
  case AUCTIONCPARAM_TAKE_AUCTION_RECORD:
  {
    PARSE_CMD_PROTOBUF(Cmd::TakeAuctionRecordCCmd, rev);
    AuctionUser* pUser = getAuctionUser(charId);
    if (takeRecord(pUser, rev) == false)
    {
      rev.set_ret(false);
      PROTOBUF(rev, send, len);
      thisServer->sendCmdToClient(charId, zoneId, send, len);
    }   
    return true;
  }
  case AUCTIONCPARAM_REQ_AUCTION_FLOWINGWATER:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReqAuctionFlowingWaterCCmd, rev);
    
    reqFlowingWater(charId, zoneId, rev);
    
    if (rev.flowingwater_size() == 0)
      return true;

    PROTOBUF(rev, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);
    XDBG << "[拍卖行-请求流水] charid" <<charId << "zoneid" <<zoneId <<"msg"<<rev.ShortDebugString() << XEND;

    return true;
  }
  }
  return false;
}

bool AuctionManager::doServerCmd(QWORD charId,const std::string& name, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  MessageStatHelper msgStat(cmd->cmd, cmd->param);
  switch (cmd->param)
  {
  case AUCTIONSPARAM_SIGNUP_ITEM:
  {
    PARSE_CMD_PROTOBUF(Cmd::SignUpItemSCmd, rev);
    signupSceneRes(charId, zoneId, rev);

    //给客户端返回成功失败
    SignUpItemCCmd cmd;
    cmd.set_ret(rev.ret());
    cmd.mutable_iteminfo()->CopyFrom(rev.iteminfo());
    PROTOBUF(cmd, send2, len2);
    thisServer->sendCmdToClient(charId, zoneId, send2, len2);
    return true;
    break;
  }
  case AUCTIONSPARAM_OFFER_PRICE:
  {
    PARSE_CMD_PROTOBUF(Cmd::OfferPriceSCmd, rev);

    offerPriceSceneRes(charId, zoneId, rev);
    return true;
  }
  case AUCTIONSPARAM_TAKE_RECORD:
  {
    PARSE_CMD_PROTOBUF(Cmd::TakeRecordSCmd, rev);
    
    AuctionUser* pUser = getAuctionUser(charId);
    takeRecordSceneRes(pUser, rev);
    
    TakeAuctionRecordCCmd cmd;
    cmd.set_id(rev.id());
    cmd.set_type(rev.type());
    cmd.set_ret(rev.ret());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);
    return true;    
  }
  }
  return false;
}

void AuctionManager::timeTick(DWORD curTime)
{
  if (m_oneSecTimer.timeUp(curTime))
  {
    checkState(curTime); 
    if (m_fiveSecTimer.timeUp(curTime))
    {
      insertAuciontEvent2Db();

      if (m_FiveMinTimer.timeUp(curTime))
      {
        m_oMessageStat.print();
        refreshTempRecord();
        if (m_ThirtyMinTimer.timeUp(curTime))
        {
          verifyAuction(curTime);
          if (m_oneHourTimer.timeUp(curTime))
          {
            clearUserCache(curTime);
            delteDb(curTime);
          }
        }       
      }
    }
  } 
}

void AuctionManager::initMaskPrice(DWORD curSec)
{
  m_mapMaskPrice.clear();
  for(auto& v : MiscConfig::getMe().getAuctionMiscCFG().m_mapMaskPrice)
  {
    if(v.second > 0)
      m_mapMaskPrice[v.first] = curSec + v.second;
  }
}

void AuctionManager::onUserOnline(const SocialUser& rUser)
{
  FUN_TRACE();
  AuctionUser& rAuctionUser = m_mapAuctionUser[rUser.charid()];
  rAuctionUser.m_qwCharId = rUser.charid();
  rAuctionUser.m_dwZoneId = rUser.zoneid();
  rAuctionUser.m_strName = rUser.name();
  rAuctionUser.m_bOnline = true;
  rAuctionUser.m_dwActiveTime = now();

  if (m_qwCurBatchId)
  {
    NtfAuctionStateCCmd cmd;
    getAuctionStateCmd(cmd);
    PROTOBUF(cmd, send, len);
    
    bool ret = thisServer->sendCmdToClient(rUser.charid(), rUser.zoneid(), send, len);
    XDBG << "[拍卖行-玩家上线] 推送拍卖状态给玩家 charid" <<rUser.charid() << rUser.zoneid() <<"ret"<< ret <<"msg"<<cmd.ShortDebugString() << XEND;
  }
  
  auto it = m_setOfflineOverTake.find(rUser.charid());
  if (it != m_setOfflineOverTake.end())
  {
    rAuctionUser.sendOverTakeCmd();
    m_setOfflineOverTake.erase(rUser.charid());
  }
}

void AuctionManager::onUserOffline(const SocialUser& rUser)
{
  FUN_TRACE();
  AuctionUser* pUser = getAuctionUser(rUser.charid());
  if (pUser)
    pUser->m_bOnline = false;

  onClosePanel(rUser.charid(), rUser.zoneid());
}

void AuctionManager::calcSignupTime()
{
  if (m_dwAuctionEndTime == 0)
    return;

  DWORD secDayStart = xTime::getDayStart(m_dwAuctionEndTime);
  if (m_dwAuctionEndTime - secDayStart < 5 * 3600)
  {
    //当日5点开始报名
    m_dwSignupBeginTime = secDayStart + 5 * 3600;
  }
  else
  {
    //次日的凌晨5点开始报名
    m_dwSignupBeginTime = secDayStart + 24 * 3600 + 5 * 3600;
  }
  
  ////TODO for test
  //m_dwSignupBeginTime = m_dwAuctionEndTime + 3 * 60;
}

void AuctionManager::checkState(DWORD curSec)
{
  switch (m_eAuctionState)
  {
  case EAuctionState_Close:
  {
    enterNewBranch(curSec);
    break;
  }
  case EAuctionState_SignUp:
  {
    if (m_bFirstInit == true)
    {
      loadOrderInfoFromDb(m_qwCurBatchId, m_mapCanSignupItem);
      m_bFirstInit = false;
    }

    if (curSec > m_dwSignupEndTime)
    {
      enterVerify(curSec);
    }
    break;
  }
  case EAuctionState_SignUpVerify:
  {
    if (m_bFirstInit == true)
    {
      loadOrderInfoFromDb(m_qwCurBatchId, m_mapCanSignupItem);
      m_bFirstInit = false;
    }

    if (curSec > m_dwPublicityBeginTime)
    {
      serverChoose();
      enterPublicity(curSec);
    }
    break;
  }
  case EAuctionState_AuctionPublicity:
  {
    if(m_bFirstInit == true)
    {
      enterPublicity(curSec);
      m_bFirstInit = false;
    }

    if(0 != m_dwAuctionNtfTime && curSec > m_dwAuctionNtfTime)
    {
      m_dwAuctionNtfTime = 0;

      TVecString vec;
      vec.push_back(std::to_string(MiscConfig::getMe().getAuctionMiscCFG().dwStartDialogTime/60));

      ntfAuctionDialog(EDialogType_AuctionStart, AUCTION_SYSID_AUCTION_DIALOG_START, vec);
    }

    if(curSec > m_dwAuctionBeginTime)
    {
      enterAuction(curSec);
    }
    break;
  }
  case EAuctionState_Auction:
  {
    if (m_bFirstInit)
    {
      enterAuction(curSec);
      m_bFirstInit = false;
    }

    auctionTick(curSec);
    break;
  }
  case EAuctionState_AuctionEnd:
  {
    if (m_bFirstInit)
    {
      loadAuctionOrderInfoFromDb(m_qwCurBatchId, m_vecAuctionItem);
      m_curAuctionOrderIter = m_vecAuctionItem.end();
      m_bFirstInit = false;
    }

    if (curSec >= m_dwSignupBeginTime)
    {
      enterNewBranch(curSec);
    }
    break;
  }
  default:
    break;
  }
}

bool AuctionManager::enterNewBranch(DWORD curSec)
{
  m_dwCurAuctionIndex = 0;
  m_mapMaskPrice.clear();
  QWORD newBatchId = m_qwCurBatchId + 1;
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_CONFIG);
  if (!field)
  {
    XERR << "[拍卖行-新加场次] 场次id:" << newBatchId << "数据库错误" << XEND;
    return false;
  }
  DWORD dwAuctionBeginTime = calcAuctionBeginTime(curSec);
  m_dwBatchCreateTime = dwAuctionBeginTime;
  setAuctionBeginTime(dwAuctionBeginTime);
  xRecord record(field);
  record.put("id", newBatchId);
  record.put("state", EAuctionState_SignUp);
  record.put("begin_time", dwAuctionBeginTime);
  record.put("verify_time", m_dwSignupEndTime);
  record.put("create_time", m_dwBatchCreateTime);

  QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    //插入失败
    XERR << "[拍卖行-新加场次] 场次id:" << newBatchId << "数据库插入失败" << XEND; 
    return false;
  }
  
  std::map<DWORD/*itemid*/, QWORD/*price*/> mapPrice;  
  std::map<DWORD, DWORD> mapCanSignup;
  loadCanSignup(mapCanSignup);
  for (auto it = mapCanSignup.begin(); it != mapCanSignup.end(); ++it)
  {    
    if (it->second)
    {
      mapPrice[it->first] = 0;
    }
  }
  XLOG << "[拍卖行-新加场次]  场次id:" << newBatchId << "可报名数" << mapPrice.size() << XEND;
  
  if (TradePriceMgr::getMe().getPrice(mapPrice) == false)
  {
    XERR << "[拍卖行-新加场次] 场次id:" << newBatchId << "从redis获取交易所服价格失败" << XEND;
    return false;
  }

  {
    //insert to db auction_item
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
    if (!field)
    {
      return false;
    }
    xRecordSet recordSet;
    m_mapCanSignupItem.clear();

    for (auto &m : mapPrice)
    {
      //交易所价格为0
      if (m.second == 0)
      {
        XERR << "[拍卖行-新加场次] 场次id:" << newBatchId << "从redis获取交易所服价格为0，itemid" <<m.first << XEND;
        continue;
      }

      OrderItem& orderItem = m_mapCanSignupItem[m.first];
      orderItem.m_qwBatchId = newBatchId;
      orderItem.m_dwItemId = m.first;
      orderItem.m_qwZenyPrice = m.second*MiscConfig::getMe().getAuctionMiscCFG().dwTradePriceDiscount/100;
      orderItem.m_qwBasePrice = AuctionManager::zeny2bcat(orderItem.m_qwZenyPrice);
      orderItem.m_dwOrder = 0;
      orderItem.m_eStatus = EAuctionResult_None;    
      orderItem.m_dwTime = curSec;
      orderItem.m_dwAuction = mapCanSignup[m.first];
      xRecord record(field);
      orderItem.toRecord(record);
      recordSet.push(record);
    }

    QWORD retcode = thisServer->getTradeConnPool().exeInsertSet(recordSet);
    if (retcode == QWORD_MAX)
    {
      m_mapCanSignupItem.clear();
      return false;
    }
  }

  m_qwCurBatchId = newBatchId;  
  setAuctionSate(EAuctionState_SignUp);

  //通知给所有打开报名界面的玩家
  NtfSignUpInfoCCmd cmd;
  for (auto& m: m_mapCanSignupItem)
  {
    SignUpItemInfo* p = cmd.add_iteminfos();
    if (!p)
      continue;
    p->set_itemid(m.first);
    p->set_price(m.second.m_qwZenyPrice);
    p->set_auction(m.second.m_dwAuction);
  }
  PROTOBUF(cmd, send, len);
  sendCmd2OpenUser(send, len);    
  
  //clear 
  for (auto&m : m_mapAuctionUser)
  {
    m.second.newBranch();
  }

  m_cacheLastAuction.Clear();
  m_bFirstInit = false;

  XLOG << "[拍卖行-阶段变化] 进入报名上架阶段，场次" << m_qwCurBatchId << "拍卖开始时间" << m_dwAuctionBeginTime << XEND;
  return true;
}

/*人审核*/
bool AuctionManager::enterVerify(DWORD curSec)
{
  XLOG << "[拍卖行-阶段变化] 报名截止，进入人工筛选阶段，场次" << m_qwCurBatchId << "拍卖开始时间" << m_dwAuctionBeginTime << XEND;
  
  TSetDWORD setSignupItem;
  do 
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
    if (!field)
      break;
    char where[32];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "batchid=%llu", m_qwCurBatchId);

    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
      break;
    
    for (DWORD i = 0; i < ret; ++i)
    {
      setSignupItem.insert(set[i].get<DWORD>("id"));
    }
  } while (0);
  
  //检查上架拍品数量
  if (setSignupItem.size() < MiscConfig::getMe().getAuctionMiscCFG().dwMinAuctionItemCount)
  {
    DWORD dwAuctionBeginTime = calcAuctionBeginTime(curSec);
    setAuctionBeginTime(dwAuctionBeginTime);
    setAuctionSate(EAuctionState_SignUp);
    saveAuctionConfig2Db();
    XLOG << "[拍卖行-拍卖阶段] 上架的拍品太少，延期到下次拍卖 场次" << m_qwCurBatchId << "拍品数量" << setSignupItem.size() << "拍卖时间" << m_dwAuctionBeginTime << XEND;
    return true;
  }
  
  setAuctionSate(EAuctionState_SignUpVerify);
  saveAuctionConfig2Db();
  return true;
}

/*公示期*/
bool AuctionManager::enterPublicity(DWORD curSec)
{
  XLOG << "[拍卖行-阶段变化] 进入公示阶段，场次" << m_qwCurBatchId << "拍卖开始时间" << m_dwAuctionBeginTime << XEND;

  loadAuctionOrderInfoFromDb(m_qwCurBatchId, m_vecAuctionItem);
  /*
  loadOrderInfoFromDb(m_qwCurBatchId,m_mapCanSignupItem);
  m_vecAuctionItem.clear();
  for (auto it = m_mapCanSignupItem.begin(); it != m_mapCanSignupItem.end(); ++it)
  {
    //无人选中或者报名
    if (!it->second.m_qwSignupId)
      continue;
    it->second.fillSignupItem();
    it->second.fillOfferPrice();
    m_vecAuctionItem.push_back(it->second);  
  } 
  */
  m_curAuctionOrderIter = m_vecAuctionItem.end();

  if (m_vecAuctionItem.empty() || m_vecAuctionItem.size() < MiscConfig::getMe().getAuctionMiscCFG().dwMinAuctionItemCount)
  {
    DWORD dwAuctionBeginTime = calcAuctionBeginTime(curSec);
    setAuctionBeginTime(dwAuctionBeginTime);
    setAuctionSate(EAuctionState_SignUp);
    saveAuctionConfig2Db();
    XLOG <<"[拍卖行-拍卖阶段] 上架的拍品太少，延期到下次拍卖 场次" << m_qwCurBatchId << "拍品数量" << m_vecAuctionItem.size() << "拍卖时间" << m_dwAuctionBeginTime << XEND;
    return true;
  }
  
  std::sort(m_vecAuctionItem.begin(), m_vecAuctionItem.end(), CmpBySort());
  setAuctionSate(EAuctionState_AuctionPublicity);
  sendAuctionInfoToAllUser();
  saveAuctionConfig2Db();

  TVecString vec;
  ntfAuctionDialog(EDialogType_AuctionPubicity, AUCTION_SYSID_AUCTION_DIALOG_PUBLICITY, vec);
  return true;
}

/*竞拍*/
bool AuctionManager::enterAuction(DWORD curSec)
{
  XLOG << "[拍卖行-阶段变化] 进入拍卖阶段，场次" << m_qwCurBatchId << "拍卖开始时间" << m_dwAuctionBeginTime << XEND; 

  loadAuctionOrderInfoFromDb(m_qwCurBatchId, m_vecAuctionItem);
  /*
  loadOrderInfoFromDb(m_qwCurBatchId,m_mapCanSignupItem);
  
  m_vecAuctionItem.clear();

  for (auto it = m_mapCanSignupItem.begin(); it != m_mapCanSignupItem.end(); ++it)
  {
    //无人选中或者报名
    if (!it->second.m_qwSignupId)
      continue;
    it->second.fillSignupItem();
    it->second.fillOfferPrice();
    m_vecAuctionItem.push_back(it->second);  
  } 
  */
  m_curAuctionOrderIter = m_vecAuctionItem.end();

  std::sort(m_vecAuctionItem.begin(), m_vecAuctionItem.end(), CmpBySort());
  
  //找到上次正在竞拍的物品
  m_curAuctionOrderIter = m_vecAuctionItem.begin();
  for (TOrderItemIter it = m_vecAuctionItem.begin(); it != m_vecAuctionItem.end(); ++it)
  {
    //找到上次拍卖的物品
    if (it->m_eStatus == EAuctionResult_None || it->m_eStatus == EAuctionResult_AtAuction)
    {
      m_curAuctionOrderIter = it;
      break;
    }
  }

  m_dwCurOrderStartTime = 0;
  setAuctionSate(EAuctionState_Auction);
  sendAuctionInfoToAllUser();
  saveAuctionConfig2Db();
  
  if (m_curAuctionOrderIter->m_eStatus == EAuctionResult_AtAuction)
  {
    if (m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId)
    {
      enterAuctionSuccess(curSec, true);
    }
    else
    {  //上次无人出价直接流拍
      enterAuctionFail(curSec, true);
    }
    m_curAuctionOrderIter++;
    m_dwCurOrderStartTime = 0;
  }

  return true;
}
/*进入竞拍结束*/
bool AuctionManager::enterAuctionEnd(DWORD curSec)
{
  m_setOfflineOverTake.clear();
  m_curListFlowingWaterInfo.clear();
  bNext = false;

  //拍卖结束
  m_curAuctionOrderIter = m_vecAuctionItem.end();
  m_dwAuctionEndTime = curSec;
  //计算下次报名时间
  calcSignupTime();
  setAuctionSate(EAuctionState_AuctionEnd);
  saveAuctionConfig2Db();
  
  //
  {
    SysMsg cmd;
    thisServer->mutableMsg(cmd, AUCTION_SYSID_AUCTION_END);
    PROTOBUF(cmd, send, len);
    sendCmd2OpenUser(send, len);
  }

  XLOG << "[拍卖行-阶段变化] 进入拍卖结束阶段，场次" << m_qwCurBatchId << "拍卖开始时间" << m_dwAuctionBeginTime << "下场报名开始时间" << m_dwSignupBeginTime << XEND;
  return true;
}

void AuctionManager::checkMaskPrice(DWORD curSec)
{
  if(m_curAuctionOrderIter == m_vecAuctionItem.end())
    return;

  for(auto &v : m_mapMaskPrice)
  {
    if(curSec < v.second)
      return;

    DWORD sign = 1 << (v.first - 1);
    if(!m_curAuctionOrderIter->hasMaskPrice(sign))
    {
      m_curAuctionOrderIter->setMaskPrice(sign);

      NtfMaskPriceCCmd cmd;
      cmd.set_batchid(m_qwCurBatchId);
      cmd.set_itemid(m_curAuctionOrderIter->m_dwItemId);
      cmd.set_signup_id(m_curAuctionOrderIter->m_qwSignupId);
      cmd.set_mask_price(m_curAuctionOrderIter->m_dwMaskPrice);

      PROTOBUF(cmd, send, len);
      sendWorldCmd(send, len);
      XLOG << "[拍卖行-出价限制] 场次" << m_qwCurBatchId << "物品" << m_curAuctionOrderIter->m_dwItemId << "signupid" << m_curAuctionOrderIter->m_qwSignupId << "档位" << v.first<< XEND;
    }
  }
}

void AuctionManager::refreshTempRecord()
{
  if(EAuctionState_SignUp != m_eAuctionState)
    return;

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
  if (!field)
  {
    XERR << "[拍卖行-刷新临时上架记录] 失败，场次" << m_qwCurBatchId << XEND;
    return;
  }

  char where[64];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "batchid=%llu and istemp=1 and isread=0", m_qwCurBatchId);

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[拍卖行-刷新临时上架记录] 失败，场次" << m_qwCurBatchId << XEND;
    return;
  }

  auto updatedb = [&](OrderItem& orderItem)
  {
    char where[64];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "batchid=%llu and itemid=%u", m_qwCurBatchId, orderItem.m_dwItemId);

    xRecord record(field);
    record.put("base_price", orderItem.m_qwBasePrice);
    record.put("zeny_price", orderItem.m_qwZenyPrice);
    record.put("isread", 1);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[拍卖行-刷新临时上架记录] 更新数据库失败，场次:" << m_qwCurBatchId << "itemid:" << orderItem.m_dwItemId << "ret:" << ret << XEND;
      return;
    }
  };

  for (DWORD i = 0; i < ret; ++i)
  {
    // 临时数据处理
    // 判断是否已有起拍价
    // 有：继续
    // 没有：取交易所价格*折扣
    // 添加至容器中
    // 更新已读信息
    DWORD itemId = set[i].get<DWORD>("itemid");
    const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(itemId);
    if(!pItemCfg)
      continue;

    OrderItem& rItem = m_mapCanSignupItem[itemId];
    rItem.fromRecord(set[i]);

    if(!rItem.m_qwBasePrice)
    {
      rItem.m_qwZenyPrice = TradePriceMgr::getMe().getPrice(itemId)*MiscConfig::getMe().getAuctionMiscCFG().dwTradePriceDiscount/100;
      rItem.m_qwBasePrice = AuctionManager::zeny2bcat(rItem.m_qwZenyPrice);
    }

    updatedb(rItem);
    XLOG << "[拍卖行-刷新临时上架记录]读取记录成功，场次:" << m_qwCurBatchId << "itemid:" << itemId << XEND;
  }
}

bool AuctionManager::auctionTick(DWORD curSec)
{
  if (m_curAuctionOrderIter == m_vecAuctionItem.end())
  {
    enterAuctionEnd(curSec);
    return true;
  }  

  if(AuctionEvent_None != m_curAuctionOrderIter->m_eEvent)
    checkMaskPrice(curSec);

  switch (m_curAuctionOrderIter->m_eEvent)
  {
  case AuctionEvent_None:
  {
    if (curSec > m_dwCurOrderStartTime)
    {
      //开始拍卖
      enterAuctionStart(curSec);
    }  
    break;
  }
  case AuctionEvent_Start:
  {
    if (curSec >= m_dwCurOrderEndTime)
    {
      enterAuctionFail(curSec);
    }
    break;
  }
  case AuctionEvent_OfferPrice:
  {
    if (curSec >= m_dwResultTime)
    {
      m_curAuctionOrderIter->m_eEvent = AuctionEvent_Result1;
      updateFlowingWater(m_curAuctionOrderIter->m_eEvent, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, m_curAuctionOrderIter->m_qwBasePrice, m_curAuctionOrderIter->m_stMaxPrice.m_strName, m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId, m_curAuctionOrderIter->m_stMaxPrice.m_dwZoneId);
      m_dwResultTime = curSec + RESULT_TIME_2;   //还有30秒
    }
    break;
  }
  case AuctionEvent_Result1:
  {
    if (curSec >= m_dwResultTime)
    {
      m_curAuctionOrderIter->m_eEvent = AuctionEvent_Result2;
      updateFlowingWater(m_curAuctionOrderIter->m_eEvent, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, m_curAuctionOrderIter->m_qwBasePrice, m_curAuctionOrderIter->m_stMaxPrice.m_strName, m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId, m_curAuctionOrderIter->m_stMaxPrice.m_dwZoneId);
      m_dwResultTime = curSec + RESULT_TIME_2;   //还有20秒
    }
    break;
  }
  case AuctionEvent_Result2:
  {
    if (curSec >= m_dwResultTime)
    {
      m_curAuctionOrderIter->m_eEvent = AuctionEvent_Result3;
      updateFlowingWater(m_curAuctionOrderIter->m_eEvent, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, m_curAuctionOrderIter->m_qwBasePrice, m_curAuctionOrderIter->m_stMaxPrice.m_strName, m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId, m_curAuctionOrderIter->m_stMaxPrice.m_dwZoneId);
      m_dwResultTime = curSec + RESULT_TIME_2;   //还有10秒
    }
    break;
  }
  case AuctionEvent_Result3:
  {
    if (curSec >= m_dwResultTime)
    {
      enterAuctionSuccess(curSec);
    }
    break;
  }
  case AuctionEvent_ResultSuccess:
  {
    nextAuctionOrder(curSec);
    break;
  }
  case AuctionEvent_ResultFail:
  {
    nextAuctionOrder(curSec);
    break;
  }
  default:
    break;
  }
  
  return true;
}

void AuctionManager::enterAuctionStart(DWORD curSec)
{
  ITER_PROTECT;
  if (bNext)
  {
    m_curAuctionOrderIter++;
    bNext = false;
    if (m_curAuctionOrderIter == m_vecAuctionItem.end())
    {
      enterAuctionEnd(curSec);
      return;
    }
  }

  m_dwCurAuctionIndex++;
  initMaskPrice(curSec);
  m_curAuctionOrderIter->m_eEvent = AuctionEvent_Start;
  m_curListFlowingWaterInfo.clear();
  m_dwCurOrderEndTime = curSec + MiscConfig::getMe().getAuctionMiscCFG().dwDurationPerOrder;

  updateFlowingWater(m_curAuctionOrderIter->m_eEvent, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, m_curAuctionOrderIter->m_qwBasePrice);
  setOrderAuctionState(EAuctionResult_AtAuction);
  
  NtfCurAuctionInfoCCmd cmd;
  cmd.set_itemid(m_curAuctionOrderIter->m_dwItemId);
  PROTOBUF(cmd, send, len);
  sendWorldCmd(send, len);

  //清除流水缓存
  string key = getFlowingWaterKey(m_qwCurBatchId, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, 0);
  m_mapCacheFlowing.erase(key);
  
  XLOG << "[拍卖行-竞拍阶段] 物品开始拍卖，场次" << m_qwCurBatchId <<"物品id" << m_curAuctionOrderIter->m_dwItemId << "基础价格" << m_curAuctionOrderIter->m_qwBasePrice << "上架者" << m_curAuctionOrderIter->m_stSignupItem.m_qwSellerId << m_curAuctionOrderIter->m_stSignupItem.m_strSellerName << "拍卖开始时间" << m_dwAuctionBeginTime << XEND;
}

void AuctionManager::enterAuctionSuccess(DWORD curSec, bool serverRestart/*=false*/)
{
  ITER_PROTECT;
  m_curAuctionOrderIter->m_eEvent = AuctionEvent_ResultSuccess;
  updateFlowingWater(m_curAuctionOrderIter->m_eEvent, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, m_curAuctionOrderIter->m_qwBasePrice, m_curAuctionOrderIter->m_stMaxPrice.m_strName, m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId, m_curAuctionOrderIter->m_stMaxPrice.m_dwZoneId);
  
  sellSuccess2Db(m_curAuctionOrderIter->m_stMaxPrice, m_curAuctionOrderIter->m_stSignupItem);

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(m_curAuctionOrderIter->m_dwItemId);
  if(!pCFG)
    return;

  TVecString vec;
  vec.push_back(std::to_string(m_dwCurAuctionIndex));
  vec.push_back(pCFG->strNameZh);
  vec.push_back(LuaManager::getMe().call<char*>("ZoneNumToString", getClientZoneID(m_curAuctionOrderIter->m_stMaxPrice.m_dwZoneId)));
  vec.push_back(m_curAuctionOrderIter->m_stMaxPrice.m_strName);
  vec.push_back(std::to_string(m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice));

  ntfAuctionDialog(EDialogType_AuctionSuccess, AUCTION_SYSID_AUCTION_DIALOG_SUCCESS, vec);

  XLOG << "[拍卖行-竞拍阶段] 物品竞拍成功"<< "服务器重启" << serverRestart<<" 场次" << m_qwCurBatchId << "物品id" << m_curAuctionOrderIter->m_dwItemId << "基础价格" << m_curAuctionOrderIter->m_qwBasePrice << "上架者" << m_curAuctionOrderIter->m_stSignupItem.m_qwSellerId << m_curAuctionOrderIter->m_stSignupItem.m_strSellerName << "拍卖开始时间" << m_dwAuctionBeginTime
    <<"竞拍获得者"<< m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId << m_curAuctionOrderIter->m_stMaxPrice.m_strName <<XEND;
}

void AuctionManager::enterAuctionFail(DWORD curSec, bool serverRestart/*=false*/)
{
  ITER_PROTECT;
  //无人竞拍，流拍
  m_curAuctionOrderIter->m_eEvent = AuctionEvent_ResultFail;
  updateFlowingWater(m_curAuctionOrderIter->m_eEvent, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, m_curAuctionOrderIter->m_qwBasePrice);
  //update seller
  sellFail2Db(m_curAuctionOrderIter->m_stSignupItem.m_qwId);

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(m_curAuctionOrderIter->m_dwItemId);
  if(!pCFG)
    return;

  TVecString vec;
  vec.push_back(std::to_string(m_dwCurAuctionIndex));
  vec.push_back(pCFG->strNameZh);

  ntfAuctionDialog(EDialogType_AuctionFail, AUCTION_SYSID_AUCTION_DIALOG_FAIL, vec);

  XLOG << "[拍卖行-竞拍阶段] 物品竞拍失败"<< "服务器重启" << serverRestart << "场次" << m_qwCurBatchId << "物品id" << m_curAuctionOrderIter->m_dwItemId << "基础价格" << m_curAuctionOrderIter->m_qwBasePrice << "上架者" << m_curAuctionOrderIter->m_stSignupItem.m_qwSellerId << m_curAuctionOrderIter->m_stSignupItem.m_strSellerName << "拍卖开始时间" << m_dwAuctionBeginTime << XEND;
}

void AuctionManager::nextAuctionOrder(DWORD curSec)
{
  m_setOfflineOverTake.clear();

  ITER_PROTECT;
  m_dwLastItemId = m_curAuctionOrderIter->m_dwItemId;  
  m_qwLastSignupId = m_curAuctionOrderIter->m_qwSignupId;
  auto nextIt = m_curAuctionOrderIter + 1;
  if (nextIt == m_vecAuctionItem.end())
  {
    enterAuctionEnd(curSec);
    return;
  }
  
  m_curAuctionOrderIter->m_eEvent = AuctionEvent_None;
  m_dwResultTime = 0;
  m_dwCurOrderStartTime = curSec + MiscConfig::getMe().getAuctionMiscCFG().dwNextOrderDuration;
  m_dwCurOrderEndTime = 0;
  m_dwCurItemId = nextIt->m_dwItemId;
  m_qwCurSignupId = nextIt->m_qwSignupId;
  m_dwCurItemPrice = nextIt->m_qwBasePrice;
  bNext = true;
  
  packNextAuctionInfo(m_cmdNextAuctionInfo);
  PROTOBUF(m_cmdNextAuctionInfo, send, len);
  sendCmd2OpenUser(send, len);  

  XLOG << "[拍卖行-下一个拍品] 场次" <<m_qwCurBatchId <<"当前拍品" << m_curAuctionOrderIter->m_dwItemId <<"下一个拍品" << nextIt->m_dwItemId << "开拍时间" << m_dwCurOrderStartTime << XEND;
}

bool AuctionManager::ntfAuctionDialog(Cmd::EDialogType type, DWORD msgId, TVecString &vec)
{
  AuctionDialogCCmd cmd;
  cmd.set_type(type);
  cmd.set_msg_id(msgId);
  for(auto &v : vec)
  {
    cmd.add_params(v);
  }

  PROTOBUF(cmd, send, len);
  XLOG << "[拍卖行-推送dialog给所有玩家] 场次"<< m_qwCurBatchId <<"type"<< type <<"msg"<< msgId << XEND;
  return sendWorldCmd(send, len);
}

bool AuctionManager::ntfAuctionState()
{
  NtfAuctionStateCCmd cmd;
  getAuctionStateCmd(cmd);
  PROTOBUF(cmd, send, len);
  XLOG << "[拍卖行-推送拍卖状态给所有玩家] 场次"<<cmd.batchid() <<"state"<<cmd.state() <<"begintim"<<cmd.auctiontime() <<"delay"<<cmd.delay() << XEND;
  return sendWorldCmd(send, len);
}

bool AuctionManager::sendWorldCmd(void* buf, WORD len)
{
  WorldCmdSCmd cmd;
  cmd.set_len(len);
  cmd.set_data(buf, len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToAllZone(send2, len2);
}

void AuctionManager::onOpenPanel(QWORD charId,const string& name, DWORD zoneId)
{  
  AuctionUser& rUser = m_mapAuctionUser[charId];
  rUser.m_qwCharId = charId;
  rUser.m_dwZoneId = zoneId;
  rUser.m_strName = name;
  rUser.m_dwActiveTime = now();
  
  m_mapOpenUser[charId] = zoneId;
  XDBG << "[拍卖行-界面] 打开界面，场次" << m_qwCurBatchId << "charid"<< charId <<"zoneid" << zoneId <<XEND;
  if (m_mapOpenUser.size() % 100 == 0)
  {
    XLOG << "[拍卖行-界面] 打开界面，场次" << m_qwCurBatchId << "charid" << charId << "zoneid" << zoneId <<"人数"<<m_mapOpenUser.size() << XEND;
  }
}

void AuctionManager::onClosePanel(QWORD charId, DWORD zoneId)
{
  m_mapOpenUser.erase(charId);
  XDBG << "[拍卖行-界面] 关闭界面，场次" << m_qwCurBatchId << "charid" << charId << "zoneid" << zoneId << XEND;
}

void AuctionManager::ntfAuctionInfo(QWORD charId, DWORD zoneId)
{
  AuctionUser* pUser = getAuctionUser(charId);
  if (!pUser)
    return;

  if (m_eAuctionState == EAuctionState_SignUp || m_eAuctionState == EAuctionState_SignUpVerify)
  {
    if (m_mapCanSignupItem.empty())
      return;
    
    NtfSignUpInfoCCmd cmd;
    for (auto& m: m_mapCanSignupItem)
    {
      SignUpItemInfo* p = cmd.add_iteminfos();
      if (!p)
        continue;
      p->set_itemid(m.first);
      p->set_price(m.second.m_qwZenyPrice);
      p->set_auction(m.second.m_dwAuction);
    }    
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);
      
    pUser->ntfMySignupInfo(m_qwCurBatchId);
    XDBG << "[拍卖行-推送上架信息给玩家] 场次" <<m_qwCurBatchId << "charid" << charId << "zoneid"<<zoneId << "msg" << cmd.ShortDebugString() << XEND;
  }
  else if (m_eAuctionState == EAuctionState_AuctionPublicity || m_eAuctionState == EAuctionState_Auction || m_eAuctionState == EAuctionState_AuctionEnd)
  {
    NtfAuctionInfoCCmd cmd;    
    cmd.set_batchid(m_qwCurBatchId);
    for (TOrderItemIter it = m_vecAuctionItem.begin(); it != m_vecAuctionItem.end(); ++it)
    {
      it->toAuctionItemInfo(cmd.add_iteminfos());
    }

    PROTOBUF(cmd, send, len);    
    thisServer->sendCmdToClient(charId, zoneId, send, len);
    
    if (isNowAuction())
    {
      if (m_curAuctionOrderIter != m_vecAuctionItem.end())
        pUser->ntfMyOfferPriceInfo(m_curAuctionOrderIter->m_qwBatchId, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId);

      if (m_dwCurOrderStartTime > now())
      {
        packNextAuctionInfo(m_cmdNextAuctionInfo);
        PROTOBUF(m_cmdNextAuctionInfo, send, len);
        thisServer->sendCmdToClient(charId, zoneId, send, len);
      }
    }

    XDBG << "[拍卖行-推送竞拍信息给玩家] 场次" << m_qwCurBatchId << "charid" << charId << "zoneid" << zoneId << "msg" << cmd.ShortDebugString() << XEND;
  }
}

void AuctionManager::signup(QWORD charId, DWORD zoneId, Cmd::SignUpItemCCmd& rev)
{
  if (m_eAuctionState != EAuctionState_SignUp)
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_NO_SIGNUPTIME);
    XERR <<"[拍卖行-报名上架] 不是报名期间，charid" << charId <<"zoneid"<<zoneId <<"itemid"<<rev.iteminfo().itemid() << XEND;
    return;
  }
  
  DWORD itemId = rev.iteminfo().itemid();
  OrderItem* pItem = getOrderItem(itemId);
  if (!pItem)
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_INVALID_ITEM);
    XERR << "[拍卖行-报名上架] 该物品不可上架报名，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << XEND;
    return;
  }  

  AuctionUser* pUser = getAuctionUser(charId);
  if (!pUser)
  {
    XERR << "[拍卖行-报名上架] 找不到玩家，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << XEND;
    return;
  }
  
  if (pUser->hasSignuped(itemId, m_qwCurBatchId))
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_INVALID_ITEM);
    XERR << "[拍卖行-报名上架] 该物品不可重复报名上架，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << XEND;
    return;
  }
  
  if (!pUser->lockSignup(itemId))
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_DUPLICATE_ITEM);
    XERR << "[拍卖行-报名上架] 该物品已经在上架路中，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << XEND;
    return;
  }
  pUser->m_dwActiveTime = now();
    
  SignUpItemSCmd scmd;
  scmd.set_batchid(m_qwCurBatchId);
  scmd.set_charid(charId);
  scmd.set_guid(rev.guid());
  scmd.mutable_iteminfo()->CopyFrom(rev.iteminfo());
  scmd.mutable_iteminfo()->set_auction(pItem->m_dwAuction);
  QWORD orderId = genOrderId();
  if (orderId == 0)
  {
    XERR << "[拍卖行-报名上架] 订单号生成失败，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << XEND;
    pUser->unlockSignup(itemId);
    return;
  }
  scmd.set_orderid(orderId);
  PROTOBUF(scmd, send, len);
  bool ret = thisServer->forwardCmdToSceneServer(pUser->m_qwCharId, pUser->m_dwZoneId, send, len);
  if (ret)
  {
    pUser->addOrderId(orderId);
  }
  XLOG << "[拍卖行-报名上架] 通知场景扣除物品，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << "ret" << ret << "订单号" << orderId<< XEND;
}

void AuctionManager::signupSceneRes(QWORD charId, DWORD zoneId, Cmd::SignUpItemSCmd& rev)
{
  AuctionUser* pUser = getAuctionUser(charId);
  if (!pUser)
  {
    XERR << "[拍卖行-报名上架-场景返回]，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << "ret"<<rev.ret() << "订单号" << rev.orderid() << XEND;
    return;
  }
  DWORD itemId = rev.iteminfo().itemid();
  pUser->unlockSignup(itemId);

  if (pUser->delOrderId(rev.orderid()) == false)
  {
    XERR << "[拍卖行-报名上架-场景返回] 检查订单号错误，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() <<"订单号"<< rev.orderid() << XEND;
    return;
  }

  if (rev.ret() == false)
  {
    XLOG << "[拍卖行-报名上架-场景返回] 场景扣除物品失败，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << XEND;
    return;
  }
  
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
  if (!field)
  {
    XERR << "[拍卖行-报名上架-场景返回]，插入数据库失败，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << "ret" << rev.ret() << "订单号" << rev.orderid() << XEND;
    return;
  }
    
  SignupItem signupItem;
  signupItem.m_dwItemId = itemId;
  signupItem.m_qwBatchId = rev.batchid();
  signupItem.m_qwSellerId = pUser->m_qwCharId;
  signupItem.m_dwZoneId = pUser->m_dwZoneId;
  signupItem.m_strSellerName = pUser->m_strName;
  signupItem.m_dwFmPoint = rev.fm_point();
  signupItem.m_dwFmBuff = rev.fm_buff();
  signupItem.m_oItemData.CopyFrom(rev.itemdata());
  
  //是否是以往批次
  bool oldBatch = false;
  
  if (rev.batchid() == m_qwCurBatchId && m_eAuctionState == EAuctionState_SignUp)
  {
    signupItem.m_eType = ERecordType_SignUp;
    signupItem.m_eTakestatus = EAuctionTakeStatus_None;
  }
  else
  {
    oldBatch = true;
    signupItem.m_eType = ERecordType_SignUpFail;
    signupItem.m_eTakestatus = EAuctionTakeStatus_CanTake;
  }    
  xRecord record(field);
  signupItem.toRecord(record, false);
  
  QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[拍卖行-报名上架-场景返回]，插入数据库失败，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << "ret" << rev.ret() << XEND;
    return ;
  }
  signupItem.m_qwId = ret;
  if (oldBatch == false)
    pUser->addSignupItem(signupItem);

  if(rev.has_itemdata())
  {
    // 获取交易所价格
    TradePriceQueryTradeCmd cmd;
    cmd.set_batchid(rev.batchid());
    cmd.set_signup_id(ret);
    cmd.mutable_itemdata()->CopyFrom(rev.itemdata());

    PROTOBUF(cmd, send, len);
    thisServer->forwardCmdToTradeServer(pUser->m_dwZoneId, send, len);
  }

  thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_SIGNUP_SUCCESS);
  XLOG << "[拍卖行-报名上架-场景返回]，插入数据库成功，charid" << charId << "zoneid" << zoneId << "itemid" << rev.iteminfo().itemid() << "数据库id" << ret << "是否是以往批次" << oldBatch <<"订单号" <<rev.orderid() << XEND;
}

void AuctionManager::offerPrice(QWORD charId, DWORD zoneId, Cmd::OfferPriceCCmd& rev)
{
  ITER_PROTECT;
  AuctionUser* pUser = getAuctionUser(charId);
  if (!pUser)
  {
    return;
  }

  if(m_curAuctionOrderIter->hasMaskPrice(1 << (rev.level() - 1)))
    return;

  if (canOfferPrice() == false)
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_INVALID_OFFERPEICE);
    XERR << "[拍卖行-竞拍出价] 不再拍卖期间" <<"charid" << pUser->m_qwCharId <<"zoneid"<<pUser->m_dwZoneId << "场次" <<m_qwCurBatchId
      <<"竞拍物品" <<rev.itemid() << "之前最高价"<< rev.max_price() << "我的出价"<<rev.add_price() <<  XEND;
    return;
  }
  if (m_curAuctionOrderIter->m_qwSignupId != rev.signup_id())
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_INVALID_OFFERPEICE);
    XERR << "[拍卖行-竞拍出价] 拍卖物品尚未开始拍卖" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << rev.itemid() << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() <<"当前在拍卖的物品" << m_curAuctionOrderIter->m_dwItemId <<"我要拍卖的物品" <<rev.itemid() << XEND;
    return;
  }

  DWORD itemId = rev.itemid();
  QWORD signupId = rev.signup_id();

  if (m_curAuctionOrderIter->m_stSignupItem.m_qwSellerId == charId)
  {
    XERR << "[拍卖行-竞拍出价] 不可拍卖自己上架的物品" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << rev.itemid() << XEND;
    return;
  }
  
  if (rev.add_price() == 0)
  {
    XERR << "[拍卖行-竞拍出价] 非法的出价" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() <<XEND;
    return;
  }

#ifndef D_ROBOTS_TEST
  QWORD maxPrice = m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice;
  if (maxPrice == 0)
    maxPrice = m_curAuctionOrderIter->m_qwBasePrice;
  if (rev.max_price() != maxPrice)
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_OFFERPEICE_LESSTHEN_MAXPRICE);
    XERR << "[拍卖行-竞拍出价] 最高价不一致，请刷新" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << itemId << "客户端最高价" << rev.max_price() << "服务器最高价" << maxPrice << XEND;
    return;
  }
  
  stringstream ss;
  ss << maxPrice;

  QWORD addPrice = LuaManager::getMe().call<QWORD>("calcAuctionPrice", ss.str().c_str(), rev.level());
  if (addPrice != rev.add_price())
  {
    XERR << "[拍卖行-竞拍出价] 出价计算不一致" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() << "level" << rev.level() << "服务器加价" << addPrice << XEND;
    return;
  }
#endif // !D_ROBOTS_TEST  
  
  QWORD totalPrice = rev.max_price() + rev.add_price();
  QWORD dwMaxPrice = getAuctionMaxPrice();
  if(totalPrice > dwMaxPrice)
  {
    totalPrice = dwMaxPrice;
    XLOG << "[拍卖行-竞拍出价] 出价大于最高价，设为最高价" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() << totalPrice << "当前最高价" << m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice << XEND;
  }
  
  if (totalPrice <= m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice)
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_OFFERPEICE_LESSTHEN_MAXPRICE);
    XERR << "[拍卖行-竞拍出价] 出价不高于当前最高价" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() << totalPrice << "当前最高价" << m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice << XEND;
    return;
  }  
  
  QWORD reduceMoney = pUser->getReduceMoney(m_qwCurBatchId, itemId, signupId, totalPrice);
  if (reduceMoney == 0)
  {
    XERR << "[拍卖行-竞拍出价] 非法的出价" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << totalPrice << "我的出价" << rev.add_price() << XEND;
    return;
  }

  if (pUser->lockOfferPrice(m_qwCurBatchId, signupId) == false)
  {
    thisServer->sendMsg(charId, zoneId, AUCTION_SYSID_OFFERPEICE_LOCKED);
    XERR << "[拍卖行-竞拍出价] 出价被锁住" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() << "当前在拍卖的物品" << m_curAuctionOrderIter->m_dwItemId <<  XEND;
    return;
  }    
  QWORD orderId = genOrderId();
  if (orderId == 0)
  {
    pUser->unlockOfferPrice(m_qwCurBatchId, signupId);
    XERR << "[拍卖行-竞拍出价] 订单号生成失败" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
      << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() << totalPrice << "当前最高价" << m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice << XEND;
    return;
  }   
  pUser->m_dwActiveTime = now();

  OfferPriceSCmd cmd;
  cmd.set_batchid(m_qwCurBatchId);
  cmd.set_itemid(rev.itemid());
  cmd.set_signupid(rev.signup_id());
  cmd.set_reduce_money(reduceMoney);
  cmd.set_orderid(orderId);  
  cmd.set_total_price(totalPrice);
  cmd.set_charid(charId);
  PROTOBUF(cmd, send, len); 
  bool ret = thisServer->forwardCmdToSceneServer(charId, zoneId, send, len);
  if (ret)
  {
    pUser->addOrderId(orderId);
  }
  XLOG << "[拍卖-竞拍出价] 发送到场景扣钱，场次" << "charid" << pUser->m_qwCharId << "zoneid" << pUser->m_dwZoneId << "场次" << m_qwCurBatchId
    << "竞拍物品" << itemId << "之前最高价" << rev.max_price() << "我的出价" << rev.add_price() << "当前在拍卖的物品" << m_curAuctionOrderIter->m_dwItemId << "需要扣钱" << reduceMoney <<"发送状态" << ret << "订单号" << orderId << XEND;
  return;
}

void AuctionManager::offerPriceSceneRes(QWORD charId, DWORD zoneId, Cmd::OfferPriceSCmd& rev)
{
  ITER_PROTECT
  FUN_TRACE();
  AuctionUser* pUser = getAuctionUser(charId);
  if (!pUser)
  {
    return;
  }
  
  DWORD itemId = rev.itemid();
  QWORD batchId = rev.batchid();
  QWORD signupId = rev.signupid();

  pUser->unlockOfferPrice(batchId, signupId);
  
  if (pUser->delOrderId(rev.orderid()) == false)
  {
    XLOG << "[拍卖-报价] 检查订单号错误，场次" << m_qwCurBatchId << "物品" << rev.itemid() << "玩家" << charId << "报价" << rev.total_price() << "最高价" << m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice << "实际扣除" << rev.reduce_money() <<"orderid"<<rev.orderid() << XEND;
    return;
  }

  if (rev.ret() == false)
  {
    XLOG << "[拍卖-报价] 场景扣钱失败返回，场次" << m_qwCurBatchId << "物品" << rev.itemid() << "玩家" << charId << "报价" << rev.total_price() << "最高价" << m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice << "实际扣除" << rev.reduce_money() << XEND;
    return;
  }
    
  //insert to db
  OfferPriceRecord record;
  record.m_qwBatchId = batchId;
  record.m_dwItemId = itemId;
  record.m_qwPrice = rev.total_price();
  record.m_qwCharId = charId;
  record.m_dwZoneId = zoneId;  
  record.m_strName = pUser->m_strName;
  record.m_dwVerifyTime = 0;
  record.m_dwTime = now();
  record.m_qwSignupId = signupId;
  DWORD skip = 0;

  OrderItem* orderItem = getOrderItemBySignupId(batchId, signupId);
  if(orderItem)
    record.m_oItemData.CopyFrom(orderItem->m_stSignupItem.m_oItemData);

  if (!isNowAuctionItem(batchId, itemId, signupId))
  { //拍卖已经结束或者进行到下一个物品的拍卖
    OrderItem* pAuctionItem = getAuctionItem(batchId, itemId, signupId);
    if (pAuctionItem == nullptr || pAuctionItem->m_stMaxPrice.m_qwCharId == charId)  //已经是自己拍下来的就跳过
    {
      skip = 1;
    }
    else
    {
      record.m_eType = ERecordType_OverTakePrice;
      record.m_eTakestatus = EAuctionTakeStatus_CanTake;
    }
  }
  else
  {
    if (!canOfferPrice())   //场景出价返回卡住了，拍卖已经出结果了
    { //过了出价期间
      //warning 如果上次最高价是自己跳过不更新
      if (m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId == charId)  //已经是自己拍下来的就跳过
      {
        skip = 2;
      }
      else
      {
        record.m_eType = ERecordType_OverTakePrice;
        record.m_eTakestatus = EAuctionTakeStatus_CanTake;
      }
    }
    else
    {
      if (record.m_qwPrice <= m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice)
      { //出价已经不是最高价了
        record.m_eType = ERecordType_OverTakePrice;
        record.m_eTakestatus = EAuctionTakeStatus_CanTake;
      }
      else
      {
        record.m_eType = ERecordType_MaxOfferPrice;
        record.m_eTakestatus = EAuctionTakeStatus_None;
      }
    }
  }
  OfferPriceRecord oldMaxPrice;

  if (m_curAuctionOrderIter != m_vecAuctionItem.end())
    oldMaxPrice = m_curAuctionOrderIter->m_stMaxPrice;

  if (!skip)
  {
    pUser->addOfferPriceRecord(record);

    pUser->ntfMyOfferPriceInfo(m_qwCurBatchId, itemId, signupId);

    updateMaxPrice(pUser, record);

    // 最高价直接成功
    if(rev.total_price() >= getAuctionMaxPrice())
    {
      enterAuctionSuccess(now());
    }
  }
    
  XLOG << "[拍卖-报价] 场景扣钱成功返回，场次" << m_qwCurBatchId << "物品" << rev.itemid()<<"skip" << skip << "玩家" << charId << "报价" << rev.total_price() << "实际扣除" << rev.reduce_money()<<"当前最高价" << m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice <<"数据库id"<< m_curAuctionOrderIter->m_stMaxPrice.m_qwId <<"出价人" << m_curAuctionOrderIter->m_stMaxPrice.m_qwCharId<<"上次最高价"<< oldMaxPrice.m_qwPrice <<"数据库id"<<oldMaxPrice.m_qwId<<"出价人"<<oldMaxPrice.m_qwCharId<< XEND;
  if (record.m_eTakestatus == EAuctionTakeStatus_CanTake)
    pUser->sendRedTip();
  return;
}

/*请求拍流水*/
void AuctionManager::reqFlowingWater(QWORD charId, DWORD zoneId, Cmd::ReqAuctionFlowingWaterCCmd& rev)
{
  DWORD pageCnt = MiscConfig::getMe().getAuctionMiscCFG().dwFlowingWaterMaxCount;
  if (isNowAuctionItem(rev.batchid(), rev.itemid(), rev.signup_id()))
  {
    //请求的时当前正在竞拍的流水
    if (m_curListFlowingWaterInfo.empty())
      return;

    DWORD beginIndex = rev.page_index()* pageCnt;
    DWORD endIndex = (rev.page_index() + 1) * pageCnt;
    DWORD size = m_curListFlowingWaterInfo.size();
    if (beginIndex >= size)
      return;
    
    //[benginIndex, endIndex)
    DWORD i = 0;
    for (auto it = m_curListFlowingWaterInfo.begin(); it != m_curListFlowingWaterInfo.end(); ++it)
    {
      if (i >= beginIndex && i < endIndex)
      {
        FlowingWaterInfo* p = rev.add_flowingwater();
        if (p)
        {
          p->CopyFrom(*it);
        }
      }
      i++;
    }
  }
  else
  {
    //请求的历史流水
    string key = getFlowingWaterKey(rev.batchid(), rev.itemid(), rev.signup_id(), rev.page_index());
    auto it = m_mapCacheFlowing.find(key);
    if (it != m_mapCacheFlowing.end())
    {
      rev = it->second;
      return;
    }

    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_EVENT);
    if (!field)
    {
      return;
    }

    char table[64];
    bzero(table, sizeof(table));
    snprintf(table, sizeof(table), "%s.%s", field->m_strDatabase.c_str(), field->m_strTable.c_str());
    char sqlStr[1024];
    bzero(sqlStr, sizeof(sqlStr));
    DWORD offSet = rev.page_index() * pageCnt;

    snprintf(sqlStr, sizeof(sqlStr), "select `id`,`batchid`,`itemid`,`event`,`price` ,`name`,`zoneid`, `max_price`, `time`, `player_id` from %s where batchid=%lu and itemid=%u and signup_id=%lu order by time desc limit %u, %u",
      table, rev.batchid(), rev.itemid(), rev.signup_id(), offSet, pageCnt);

    std::string sql2(sqlStr);
    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeRawSelect(field, set, sql2);
    if (ret == QWORD_MAX)
    {
      return;
    }
    
    for (DWORD i = 0; i < ret; i++)
    {
      FlowingWaterInfo* pInfo = rev.add_flowingwater();
      pInfo->set_time(set[i].get<DWORD>("time"));
      pInfo->set_event(Cmd::AuctionEvent(set[i].get<DWORD>("event")));
      pInfo->set_price(set[i].get<DWORD>("price"));
      pInfo->set_player_name(set[i].getString("name"));
      pInfo->set_zoneid(set[i].get<DWORD>("zoneid"));
      pInfo->set_max_price(set[i].get<DWORD>("max_price"));
      pInfo->set_player_id(set[i].get<QWORD>("player_id"));
    }    
    m_mapCacheFlowing[key] = rev;
  }
}

bool AuctionManager::takeRecord(AuctionUser*pUser, const::TakeAuctionRecordCCmd& rev)
{
  FUN_TRACE();
  if (pUser == nullptr)
    return false;
  XDBG <<"[拍卖行-领取物品] 请求领取，charid"<< pUser->m_qwCharId <<pUser->m_strName << pUser->m_dwZoneId <<"type"<<rev.type() <<"id"<<rev.id() << XEND;
  Cmd::AuctionRecord recordInfo;
  if (loadRecordFromDb(rev.type(), rev.id(), pUser->m_qwCharId, recordInfo) == false)
  {
    XERR <<"[拍卖行-领取] 找不到记录，charid" <<pUser->m_qwCharId <<pUser->m_strName << "type" <<rev.type() <<"id"<<rev.id() <<XEND;
    return false;
  }
  DWORD delTime = now() - MiscConfig::getMe().getAuctionMiscCFG().dwReceiveTime;
  if (recordInfo.time() < delTime)
  {
    XERR << "[拍卖行-领取] 30天前的不可领取，charid" << pUser->m_qwCharId << pUser->m_strName << "type" << rev.type() << "id" << rev.id() <<"deltime"<< delTime << XEND;
    return false;
  }
  
  if (!canTake(recordInfo))
  {
    XERR << "[拍卖行-领取] 记录处于不可领取状态,charid" << pUser->m_qwCharId << "type" << recordInfo.type() << "id" << recordInfo.id() << "领取状态" << recordInfo.take_status() << XEND;
    return false;
  }

  ItemInfo itemInfo;
  QWORD bcat = 0;
  QWORD zeny = 0;
  switch (recordInfo.type())
  {
  case ERecordType_SignUpFail:    //上架失败
  case ERecordType_SellFail:      //流拍， 领取道具
  {
    itemInfo.set_id(recordInfo.itemid());
    itemInfo.set_count(1);
    itemInfo.set_source(ESOURCE_AUCTION);
    break;
  }
  case ERecordType_OverTakePrice:  //出价被超过了,领取b格猫金币
  {
    bcat = recordInfo.price();
    break;
  }
  case ERecordType_SellSucessPass://出售成功审核通过,领取zeny
  {
    zeny = recordInfo.get_money();
    break;
  }
  case ERecordType_SellSucessNoPass:      //出售成功审核不通过，领取物品
  {
    itemInfo.set_id(recordInfo.itemid());
    itemInfo.set_count(1);
    itemInfo.set_source(ESOURCE_AUCTION);
    break;
  }
  case ERecordType_BuySuccessPass:      //购买成功审核通过，领物品
  {
    itemInfo.set_id(recordInfo.itemid());
    itemInfo.set_count(1);
    itemInfo.set_source(ESOURCE_AUCTION);
    break;
  }
  case ERecordType_BuySuccessNoPass:      //购买成功审核不通过，退钱,退还b格猫金币
  {
    bcat = recordInfo.cost_money();
    break;
  }
  default:
    XERR << "[拍卖行-领取] 记录处于不可领取,charid" << pUser->m_qwCharId << "type" << recordInfo.type() << "id" << recordInfo.id() << "领取状态" << recordInfo.take_status() << XEND;
    return false;
  }
  pUser->m_dwActiveTime = now();
   
  TakeRecordSCmd scmd;
  if (recordInfo.type() == ERecordType_OverTakePrice)
  {
    if (!pUser->lockOfferPrice(recordInfo.batchid(), recordInfo.signup_id()))
    {
      XERR << "[拍卖行-领取] 记录被锁住不可领取,charid" << pUser->m_qwCharId << "type" << recordInfo.type() << "id" << recordInfo.id() << "领取状态" << recordInfo.take_status() << XEND;
      return false;
    }
    scmd.set_batchid(recordInfo.batchid());
    scmd.set_itemid(recordInfo.itemid());
    scmd.set_signup_id(recordInfo.signup_id());
  }

  scmd.set_type(rev.type());
  scmd.set_id(rev.id());
  scmd.set_charid(pUser->m_qwCharId);
  scmd.set_zeny(zeny);
  scmd.set_bcat(bcat);
  if (itemInfo.id())
  {
    ItemInfo* pItemInfo = scmd.mutable_item();
    if (pItemInfo)
    {
      pItemInfo->CopyFrom(itemInfo);
    }
  }

  if(recordInfo.has_itemdata())
  {
    ItemData* pData = scmd.mutable_itemdata();
    if(!pData)
    {
      XERR << "[拍卖行-领取] get itemdata failed, charid" << pUser->m_qwCharId << "type" << recordInfo.type() << "id" << recordInfo.id() << "领取状态" << recordInfo.take_status() << XEND;
      return false;
    }
    pData->CopyFrom(recordInfo.itemdata());
  }
  
  PROTOBUF(scmd, send, len);
  bool ret = thisServer->forwardCmdToSceneServer(pUser->m_qwCharId, pUser->m_dwZoneId, send, len);
  XLOG << "[拍卖行-领取] 领取检验通过，发送到场景加到玩家身上,charid" << pUser->m_qwCharId << "type" << recordInfo.type() << "id" << recordInfo.id() << "领取状态" << recordInfo.take_status() <<"zeny" <<zeny <<"bcat"<<bcat <<"itemid"<<scmd.item().id() << "ret"<< ret << XEND;
  return true;
}

bool AuctionManager::takeRecordSceneRes(AuctionUser*pUser, const::TakeRecordSCmd& rev)
{
  FUN_TRACE();
  if (!pUser)
    return false;
  if (!rev.ret())
  {
    XERR <<"[拍卖行-领取场景返回] 场景处理失败， charid" << pUser->m_qwCharId << pUser->m_dwZoneId << "type" << rev.type() << "id" << rev.id() <<"zeny"<<rev.zeny()<<"itemid"<<rev.item().id() << XEND;
    return false;
  }

  XLOG << "[拍卖行-领取场景返回] 场景处理成功， charid" << pUser->m_qwCharId << pUser->m_dwZoneId << "type" << rev.type() << "id" << rev.id() << "zeny" << rev.zeny() << "itemid" << rev.item().id() << XEND;
  
  if (rev.batchid() && rev.itemid())
    pUser->unlockOfferPrice(rev.batchid(), rev.signup_id());
  
  //update db
  {
    string tableName = getTableName(rev.type());

    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, tableName);
    if (!field)
    {
      return false;
    }
    char where[64] = { 0 };
    snprintf(where, sizeof(where), "id=%lu", rev.id());
    xRecord record(field);
    record.put("take_status", EAuctionTakeStatus_Took);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[拍卖行-领取场景返回] 场景处理成功,修改领取状态为不可领取，失败， charid" << pUser->m_qwCharId << pUser->m_dwZoneId << "type" << rev.type() << "id" << rev.id() << "zeny" << rev.zeny() << "itemid" << rev.item().id() << XEND;
      return false;
    }
  }

  if (isNowAuctionItem(rev.batchid(), rev.itemid(), rev.signup_id()) && rev.type() == ERecordType_OverTakePrice)
  {
    pUser->clearOfferPrice(rev.batchid(), rev.itemid(), rev.signup_id());
  }

  return true;
}

bool AuctionManager::canOfferPrice()
{
  if(m_eAuctionState != EAuctionState_Auction)
    return false;

  if (!isRightOrder())
    return false;

  if (m_curAuctionOrderIter->m_eEvent == AuctionEvent_None || 
    m_curAuctionOrderIter->m_eEvent == AuctionEvent_ResultSuccess ||
    m_curAuctionOrderIter->m_eEvent == AuctionEvent_ResultFail)
    return false;

  return true;
}

void AuctionManager::loadCanSignup(std::map<DWORD/*itemid*/, DWORD>& mapCanSignup)
{
  mapCanSignup.clear();

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_CAN_SIGNUP);
  if (!field)
  {
    XERR << "[拍卖行-数据加载可报名物品] 失败，场次" << XEND;
    return;
  }

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, NULL, NULL);
  if (ret == QWORD_MAX)
  {
    return;
  }

  for (DWORD i = 0; i < ret; ++i)
  {
    mapCanSignup[set[i].get<DWORD>("itemid")] = set[i].get<DWORD>("auction");
  }

  XLOG << "[拍卖行-数据加载可报名物品] 成功" << "加载条数" << mapCanSignup.size() << XEND;
}

void AuctionManager::loadOrderInfoFromDb(QWORD batchId, TMapCanSignUpItem& mapCanSignupItem)
{
  mapCanSignupItem.clear();
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
  if (!field)
  {
    XERR << "[拍卖行-数据加载上架订单] 失败，场次" << batchId << XEND;
    return;
  }

  char where[32];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "batchid=%llu ", m_qwCurBatchId);

  char order[64];
  bzero(order, sizeof(order));
  snprintf(order, sizeof(order), "order by orderid");

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, order);
  if (ret == QWORD_MAX)
  {
    XERR << "[拍卖行-数据加载上架订单] 失败，场次" << batchId << XEND;
    return;
  }

  for (DWORD i = 0; i < ret; ++i)
  {
    OrderItem& rItem = mapCanSignupItem[set[i].get<DWORD>("itemid")];
    rItem.fromRecord(set[i]);
  }
  
  XLOG << "[拍卖行-数据加载上架订单] 成功，场次" <<batchId <<"加载条数" << ret << XEND;
}

//加载拍卖的物品信息
void AuctionManager::loadAuctionOrderInfoFromDb(QWORD batchId, TVecOrderItem& vecOrderItem)
{
  vecOrderItem.reserve(10);
  vecOrderItem.clear();

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
  if (!field)
  {
    return;
  }

  char where[32];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "batchid=%llu and signup_id>0", batchId);
  char order[64];
  bzero(order, sizeof(order));
  snprintf(order, sizeof(order), "order by orderid");

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, order);
  if (ret == QWORD_MAX)
  {
    return;
  }  

  for (DWORD i = 0; i < ret; ++i)
  {
    OrderItem orderItem;
    orderItem.fromRecord(set[i]);
    orderItem.fillSignupItem();
    orderItem.fillOfferPrice();
    vecOrderItem.push_back(orderItem);
  }
}

void AuctionManager::loadChooseOrderInfoFromDb(QWORD batchId, TVecOrderItem& vecOrderItem)
{
  vecOrderItem.reserve(10);
  vecOrderItem.clear();

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
  if (!field)
  {
    return;
  }

  char where[32];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "batchid=%llu and choose>0", batchId);
  char order[64];
  bzero(order, sizeof(order));
  snprintf(order, sizeof(order), "order by orderid");

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, order);
  if (ret == QWORD_MAX)
  {
    return;
  }  

  for (DWORD i = 0; i < ret; ++i)
  {
    OrderItem orderItem;
    orderItem.fromRecord(set[i]);
    orderItem.fillSignupItem();
    orderItem.fillOfferPrice();
    vecOrderItem.push_back(orderItem);
  }
}

bool AuctionManager::loadRecordFromDb(Cmd::ERecordType type, QWORD id, QWORD charId, Cmd::AuctionRecord& info)
{

  string tableName = getTableName(type);
  
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, tableName);
  if (!field)
    return false;//
  char where[128];
  bzero(where, sizeof(where));

  snprintf(where, sizeof(where), "id=%llu and charid=%llu  ", id, charId);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[拍卖行-领取] 查询数据错误 charid：" << charId << "table: " << tableName << XEND;
    return false ;
  }
  
  if (ret == 1)
  {
    if (tableName == DB_TABLE_AUCTION_ITEM_SIGNUP)
      packSignupRecord(set[0], info);
    else
      packOfferPriceRecord(set[0], info);
  }
  else
    return false;
  return true;
}

/*报名上架记录*/
void AuctionManager::packSignupRecord(const xRecord& r, Cmd::AuctionRecord& info)
{
  info.set_type(ERecordType(r.get<DWORD>("type")));
  info.set_id(r.get<QWORD>("id"));
  info.set_batchid(r.get<QWORD>("batchid"));
  info.set_itemid(r.get<DWORD>("itemid"));
  info.set_take_status(EAuctionTakeStatus(r.get<DWORD>("take_status")));
  info.set_time(r.get<DWORD>("time"));
  info.set_signup_id(r.get<QWORD>("signup_id"));
  if (r.getBinSize("itemdata") > 0)
  {
    std::string strItemData;
    strItemData.assign((const char*)r.getBin("itemdata"), r.getBinSize("itemdata"));
    ItemData* pData = info.mutable_itemdata();
    if(!pData)
    {
      XERR << "[拍卖行-上架记录] get ItemData failed! signup_id：" << info.signup_id() << XEND;
      return;
    }
    pData->ParseFromString(strItemData);
  }
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
void AuctionManager::packOfferPriceRecord(const xRecord& r, Cmd::AuctionRecord& info)
{
  info.set_type(ERecordType(r.get<DWORD>("type")));
  info.set_id(r.get<QWORD>("id"));
  info.set_batchid(r.get<QWORD>("batchid"));
  info.set_itemid(r.get<DWORD>("itemid"));
  info.set_take_status(EAuctionTakeStatus(r.get<DWORD>("take_status")));
  info.set_price(r.get<QWORD>("price"));
  info.set_time(r.get<DWORD>("time"));
  info.set_signup_id(r.get<QWORD>("signup_id"));
  if (r.getBinSize("itemdata") > 0)
  {
    std::string strItemData;
    strItemData.assign((const char*)r.getBin("itemdata"), r.getBinSize("itemdata"));
    ItemData* pData = info.mutable_itemdata();
    if(!pData)
    {
      XERR << "[拍卖行-竞拍记录] get ItemData failed! signup_id：" << info.signup_id() << XEND;
      return;
    }
    pData->ParseFromString(strItemData);
  }
  if (info.type() == ERecordType_BuySuccess ||
    info.type() == ERecordType_BuySuccessPass||
    info.type() == ERecordType_BuySuccessNoPass)
  {
    info.set_seller(r.getString("seller_name"));
    info.set_zoneid(r.get<DWORD>("seller_zoneid"));
    info.set_cost_money(r.get<QWORD>("price"));     //税后出售获得的钱
  }
}

/*warning:涉及到数据状态修改，一次拍卖只可调用一次*/
void AuctionManager::serverChoose()
{
  TVecOrderItem vecChooseItem;
  DWORD maxSellCount = MiscConfig::getMe().getAuctionMiscCFG().dwMaxAuctionItemCount;
  vecChooseItem.reserve(maxSellCount);
  loadChooseOrderInfoFromDb(m_qwCurBatchId, vecChooseItem);

  // 只选上架物品，未选物品拍卖人，需要随机一个拍卖人
  bool needChooseSignup = false;
  for(auto& v : vecChooseItem)
  {
    if(!v.m_qwSignupId)
    {
      needChooseSignup = true;
      break;
    }
  }

  auto haschoose = [&](QWORD signupId) ->bool{
    for(auto& v : vecChooseItem)
    {
      if(v.m_qwSignupId == signupId)
        return true;
    }
    return false;
  };

  auto hasitem = [&](DWORD itemId) ->bool{
    for(auto& v : vecChooseItem)
    {
      if(v.m_dwItemId == itemId)
        return true;
    }
    return false;
  };

  //上架商品
  TMapSignupItem mapSignupItem;
  std::map<DWORD, std::vector<QWORD>> mapItem2SignupId;
  std::vector<std::pair<DWORD, QWORD>> vecItem2SignupId;
  TSetQWORD redUser;
  if (vecChooseItem.size() < maxSellCount || needChooseSignup)
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
    if (!field)
    {
      return;
    }

    char where[32];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "batchid=%llu ", m_qwCurBatchId);

    xRecordSet set;
    QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      return;
    }
    for (DWORD i = 0; i < ret; ++i)
    {
      SignupItem signupItem;
      signupItem.fromRecord(set[i]);
      mapSignupItem[signupItem.m_qwId] = signupItem;
      redUser.insert(signupItem.m_qwSellerId);
    }

    for(auto& m : mapSignupItem)
    {
      DWORD itemId = m.second.m_dwItemId;
      if(haschoose(m.first))
        continue;
      auto iter = m_mapCanSignupItem.find(itemId);
      if(m_mapCanSignupItem.end() == iter)
        continue;

      if(1 == iter->second.m_dwAuction)
        mapItem2SignupId[itemId].push_back(m.first);
      else
        vecItem2SignupId.push_back(std::make_pair(itemId, m.first));
    }

    DWORD leftCnt = maxSellCount - vecChooseItem.size();
    // 先物品
    for(auto& m : mapItem2SignupId)
    {
      if(!leftCnt)
        break;
      if(hasitem(m.first))
        continue;
      auto iter = m_mapCanSignupItem.find(m.first);
      if(m_mapCanSignupItem.end() == iter)
        continue;
      std::random_shuffle(m.second.begin(), m.second.end());
      iter->second.m_qwSignupId = m.second[0];
      vecChooseItem.push_back(iter->second);
      iter->second.m_bChoose = true;
      leftCnt--;
    }

    // 后装备
    std::random_shuffle(vecItem2SignupId.begin(), vecItem2SignupId.end());
    for(auto& v : vecItem2SignupId)
    {
      if(!leftCnt)
        break;
      auto iter = m_mapCanSignupItem.find(v.first);
      if(m_mapCanSignupItem.end() == iter)
        continue;
      iter->second.m_qwSignupId = v.second;
      iter->second.m_bChoose = false;
      // 获取起拍价
      auto it = mapSignupItem.find(v.second);
      if(mapSignupItem.end() != it)
      {
        iter->second.m_qwBasePrice = it->second.m_qwBasePrice;
        iter->second.m_qwZenyPrice = it->second.m_qwZenyPrice;
      }
      vecChooseItem.push_back(iter->second);
      iter->second.m_bChoose = true;
      leftCnt--;
    }
  }
  
  //先把所有订单标记为上架失败
  updateChooseSignup(0, m_qwCurBatchId, ERecordType_SignUpFail);
  for (auto& v : vecChooseItem)
  {
    if (!v.m_bChoose)
    {
      v.m_bChoose = true;
      XLOG << "[拍卖行-服务器选中上架商品卖家] 场次" << m_qwCurBatchId << "上架物品id" << v.m_dwItemId << "卖家上架id" << v.m_qwSignupId << XEND;
      updateChooseOrder(m_qwCurBatchId, v);
    }
    else
    {
      XLOG << "[拍卖行-人工选中上架商品] 场次" << m_qwCurBatchId << "上架物品id" << v.m_dwItemId << XEND;
    }

    if(!v.m_qwSignupId)
    {
      auto iter = mapItem2SignupId.find(v.m_dwItemId);
      if(mapItem2SignupId.end() == iter)
      {
        //log
        break;
      }
      std::random_shuffle(iter->second.begin(), iter->second.end());
      v.m_qwSignupId = iter->second[0];
      updateChooseOrder(m_qwCurBatchId, v);
      XLOG << "[拍卖行-服务器选中上架商品卖家] 场次" << m_qwCurBatchId << "上架物品id" << v.m_dwItemId << "卖家上架id" << v.m_qwSignupId << XEND;
    }
    updateChooseSignup(v.m_qwSignupId, m_qwCurBatchId, ERecordType_SignUpSuccess);
    redUser.erase(v.m_stSignupItem.m_qwSellerId);
  }

  for (auto&s : redUser)
  {
    sendRedTip(s);
  }
}

void AuctionManager::updateChooseOrder(QWORD batchId, OrderItem& orderItem)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
  if (!field)
  {
    return;
  }
  char where[64] = { 0 };
  snprintf(where, sizeof(where), "batchid=%llu and itemid=%u", batchId, orderItem.m_dwItemId);

  auto updatedb = [&]()
  {
    xRecord record(field);
    record.put("choose", true);
    record.put("signup_id", orderItem.m_qwSignupId);
    thisServer->getTradeConnPool().exeUpdate(record, where);
  };

  auto insertdb = [&]()
  {
    xRecord record(field);
    orderItem.toRecord(record);
    thisServer->getTradeConnPool().exeInsert(record);
  };

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if(QWORD_MAX == ret)
    return;

  if(0 == ret)
    return;
  else if(1 == ret)
  {
    QWORD qwSignupId = set[0].get<QWORD>("signup_id");
    if(0 == qwSignupId)
      updatedb();
    else
      insertdb();
  }
  else if(1 < ret)
  {
    insertdb();
  }
}

void AuctionManager::updateChooseSignup(QWORD id, QWORD batchId, ERecordType type)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
  if (!field)
  {
    return;
  }

  char where[32] = { 0 };
  if (id)
    snprintf(where, sizeof(where), "id=%llu", id);
  else //设置该场次所有的
    snprintf(where, sizeof(where), "batchid=%llu", batchId);

  xRecord record(field);
  record.put("type", type);
  if (type == ERecordType_SignUpSuccess)
    record.put("take_status", EAuctionTakeStatus_None);
  else if (type == ERecordType_SignUpFail)
    record.put("take_status", EAuctionTakeStatus_CanTake);

  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    return;
  }
}

void AuctionManager::sendAuctionInfoToAllUser()
{
  NtfAuctionInfoCCmd cmd;
  cmd.set_batchid(m_qwCurBatchId);

  for (TOrderItemIter it = m_vecAuctionItem.begin(); it != m_vecAuctionItem.end(); ++it)
  {
    it->toAuctionItemInfo(cmd.add_iteminfos());
  }

  PROTOBUF(cmd, send, len);
  sendCmd2OpenUser(send, len);
  XLOG <<"[拍卖行-拍卖中] 推送正在拍卖物品信息给所有打开界面的玩家 场次" << m_qwCurBatchId << "msg" << cmd.ShortDebugString() << XEND;
}

void AuctionManager::sendCmd2OpenUser(void* data, WORD len)
{
  if (m_mapOpenUser.empty())
    return;

  BroadcastMsgBySessionAuctionSCmd cmd;
  cmd.set_len(len);
  cmd.set_data(data, len);
  PROTOBUF(cmd, send2, len2);
  thisServer->sendCmdToAllZone(send2, len2);
}

void AuctionManager::updateFlowingWater(Cmd::AuctionEvent eEvent, DWORD itemId, QWORD signupId, QWORD price, const string& name, QWORD playerId, DWORD zoneId, bool maxPrice)
{
  ITER_PROTECT;

  m_cacheCmdUpdateFlowingWater.set_batchid(m_qwCurBatchId);
  m_cacheCmdUpdateFlowingWater.set_itemid(itemId);
  m_cacheCmdUpdateFlowingWater.set_signup_id(signupId);
  FlowingWaterInfo*pFlowingWater = m_cacheCmdUpdateFlowingWater.mutable_flowingwater();
  if (!pFlowingWater)
    return;
  pFlowingWater->set_event(eEvent);
  pFlowingWater->set_time(now());

  switch (eEvent)
  {
  case AuctionEvent_Start:
  {     
    pFlowingWater->set_price(price);      //[时间]  [道具名]拍卖正式开始，拍卖底价zenyXXXXXX								  
    break;
  }
  case AuctionEvent_OfferPrice:           //[时间]  出价  [线] [玩家名] 出价zenyXXXXXXX								
  {
    pFlowingWater->set_player_name(name);
    pFlowingWater->set_zoneid(zoneId);
    pFlowingWater->set_price(price);
    pFlowingWater->set_max_price(maxPrice);   //是否是最高价
    pFlowingWater->set_player_id(playerId);
    break;
  }
  case AuctionEvent_Result1:              //[时间] 30秒  若无竞价，拍品将由[线][玩家名]获得 								
  { 
    pFlowingWater->set_player_name(name);
    pFlowingWater->set_zoneid(zoneId);
    pFlowingWater->set_player_id(playerId);
    break;
  }
  case AuctionEvent_Result2:            //[时间]  20秒  拍品即将落锤，还有没有竞价者？ 								
  {
    pFlowingWater->set_player_name(name);
    pFlowingWater->set_zoneid(zoneId);
    pFlowingWater->set_player_id(playerId);
    break;
  }
  case AuctionEvent_Result3:          //[时间] 10秒  最后十秒，拍品即将归 [线][玩家名] 								
  {
    pFlowingWater->set_player_name(name);
    pFlowingWater->set_zoneid(zoneId);
    pFlowingWater->set_player_id(playerId);
    break;
  }
  case AuctionEvent_ResultSuccess:  //[时间] 成交  恭喜[线] [玩家名] 成功拍得[道具名]，下一场拍卖将在60秒后开始，敬请等待。 								
  {
    pFlowingWater->set_player_name(name);
    pFlowingWater->set_zoneid(zoneId);
    pFlowingWater->set_player_id(playerId);
    break;
  }
  case AuctionEvent_ResultFail:     //[时间]  流拍  拍卖时间内无人出价，[道具名]流拍。								
  {
    break;
  }
  default:
    break;
  }

  //insert into db  
  SAuctionEvent e;
  e.m_qwBatchId = m_qwCurBatchId;
  e.m_eEvent = eEvent;
  e.m_dwItemId = itemId;
  e.m_dwTime = now();
  e.m_qwPrice = price;
  e.m_strName = name;
  e.m_dwZoneId = zoneId;
  e.m_qwMaxPrice = maxPrice;
  e.m_qwCharId = playerId;
  e.m_qwSignupId = signupId;
  m_listAuctionEvent.push_back(e);

  //最近的插最前面
  m_curListFlowingWaterInfo.push_front(*pFlowingWater);
  
  PROTOBUF(m_cacheCmdUpdateFlowingWater, send, len);
  sendCmd2OpenUser(send, len);
}

bool AuctionManager::updateSignupPrice(QWORD batchId, QWORD signupId, QWORD price)
{
  if(m_qwCurBatchId != batchId)
    return false;

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
  if(!field)
    return false;

  QWORD qwZenyPrice = price*MiscConfig::getMe().getAuctionMiscCFG().dwTradePriceDiscount/100;
  QWORD qwBasePrice = AuctionManager::zeny2bcat(qwZenyPrice);

  xRecord record(field);
  record.put("base_price", qwBasePrice);
  record.put("zeny_price", qwZenyPrice);
  char where[128] = {0};
  snprintf(where, sizeof(where), "id=%llu and batchid=%llu", signupId, batchId);
  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (1 != ret)
  {
    XERR << "[拍卖-更新起拍价] 失败 batchid:" << batchId  <<"signup_id:" << signupId <<"price:"<< price <<"ret:" <<ret<< XEND;
    return false;
  }

  XLOG << "[拍卖-更新起拍价] 成功 batchid:" << batchId  <<"signup_id:" << signupId <<"price:"<< price <<"ret:" <<ret<< XEND;
  return true;
}

bool AuctionManager::updateMaxPrice(AuctionUser* pUser,OfferPriceRecord& rRecord)
{
  if (m_curAuctionOrderIter == m_vecAuctionItem.end()) 
    return false;
  if (!pUser)
    return false;
  //不在竞拍中
  if (!isNowAuctionItem(rRecord.m_qwBatchId, rRecord.m_dwItemId, rRecord.m_qwSignupId))
    return false;

  bool isMaxPrice = false;
  do 
  {
    if (rRecord.m_eType == ERecordType_OverTakePrice)
    {
      isMaxPrice = false;
      break;
    }
    //update db    
    OfferPriceRecord lastMaxPrice = m_curAuctionOrderIter->m_stMaxPrice;
    m_curAuctionOrderIter->m_stMaxPrice = rRecord;
    
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
    if (!field)
    {
      return false;
    }
    xRecord record(field);
    record.put("offerprice_id", m_curAuctionOrderIter->m_stMaxPrice.m_qwId);
    record.put("trade_price", m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice);
    char where[64] = { 0 };
    snprintf(where, sizeof(where), "batchid=%llu and signup_id=%llu", rRecord.m_qwBatchId, rRecord.m_qwSignupId);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      //插入失败
      XERR << "[拍卖-更新最高出价] 失败 batchid" << rRecord.m_qwBatchId  <<"itemid" << rRecord.m_dwItemId <<"offerprice_id"<< m_curAuctionOrderIter->m_stMaxPrice.m_qwId <<"ret" <<ret<< XEND;
      return false;
    }
    isMaxPrice = true;
    m_curAuctionOrderIter->m_eEvent = AuctionEvent_OfferPrice;
    m_dwResultTime = now() + RESULT_TIME_1;
    m_dwCurOrderEndTime = now() + MiscConfig::getMe().getAuctionMiscCFG().dwDurationPerOrder;
    
    //上次最高价不是自己的
    if (lastMaxPrice.m_qwId != rRecord.m_qwId)
      removeOldMaxPrice(lastMaxPrice);

  } while (0);   

  updateFlowingWater(AuctionEvent_OfferPrice, m_curAuctionOrderIter->m_dwItemId, m_curAuctionOrderIter->m_qwSignupId, rRecord.m_qwPrice, pUser->m_strName, pUser->m_qwCharId, rRecord.m_dwZoneId, isMaxPrice);
 
  //更新最高价给打开界面的玩家
  UpdateAuctionInfoCCmd cmd;
  cmd.set_batchid(m_qwCurBatchId);
  m_curAuctionOrderIter->toAuctionItemInfo(cmd.mutable_iteminfo());
  PROTOBUF(cmd, send, len);
  sendCmd2OpenUser(send, len);

  return true;
}

void AuctionManager::setAuctionSate(Cmd::EAuctionState state)
{
  //if (m_eAuctionState == state)
  //  return;  
  m_eAuctionState = state;
  ntfAuctionState();
}

void AuctionManager::setAuctionBeginTime(DWORD t)
{
  if (t == 0)
    return;
  m_dwAuctionBeginTime = t;
  m_dwAuctionNtfTime = m_dwAuctionBeginTime - MiscConfig::getMe().getAuctionMiscCFG().dwStartDialogTime;
  m_dwPublicityBeginTime = m_dwAuctionBeginTime - MiscConfig::getMe().getAuctionMiscCFG().dwPublicityDuration;
  m_dwSignupEndTime = m_dwPublicityBeginTime - MiscConfig::getMe().getAuctionMiscCFG().dwVerifySignupDuration;
}

void AuctionManager::setOrderAuctionState(EAuctionResult status)
{
  ITER_PROTECT;

  if (m_curAuctionOrderIter->m_eStatus == status)
    return;
 
  m_curAuctionOrderIter->m_eStatus = status;
  
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
    if (!field)
    {
      return;
    }
    char where[64] = { 0 };
    snprintf(where, sizeof(where), "batchid=%llu and signup_id=%llu", m_curAuctionOrderIter->m_qwBatchId, m_curAuctionOrderIter->m_qwSignupId);
    xRecord record(field);
    record.put("status", m_curAuctionOrderIter->m_eStatus);
    record.put("time", now());
    if (m_curAuctionOrderIter->m_eStatus == EAuctionResult_Sucess)
    {
      record.put("offerprice_id", m_curAuctionOrderIter->m_stMaxPrice.m_qwId);
      record.put("trade_price", m_curAuctionOrderIter->m_stMaxPrice.m_qwPrice);
    }

    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      return;
    }
  }

  UpdateAuctionInfoCCmd cmd;
  cmd.set_batchid(m_qwCurBatchId);
  m_curAuctionOrderIter->toAuctionItemInfo(cmd.mutable_iteminfo());
  PROTOBUF(cmd, send, len);
  sendCmd2OpenUser(send, len);
  XLOG << "[拍卖行-竞拍阶段] 竞拍状态修改,场次" <<m_qwCurBatchId << "signupid" <<m_curAuctionOrderIter->m_qwSignupId << "竞拍状态修改为" << status << XEND;
}

void AuctionManager::getLastAuctionInfo()
{
  if (m_qwCurBatchId <= 1)
    return ;
  if (m_cacheLastAuction.has_batchid())
    return;
  m_qwLastBatchId = m_qwCurBatchId - 1;
  
  loadAuctionOrderInfoFromDb(m_qwLastBatchId, m_vecLastAuctionItem);
  
  if (m_vecLastAuctionItem.empty())
    return;
  
  m_cacheLastAuction.set_batchid(m_qwLastBatchId);
  for (auto&v : m_vecLastAuctionItem)
  {
    v.toAuctionItemInfo(m_cacheLastAuction.add_iteminfos());
  }
}

//计算拍卖开始时间
DWORD AuctionManager::calcAuctionBeginTime(DWORD curSec)
{
  const std::map<DWORD/**/, struct tm>& rMapTm = MiscConfig::getMe().getAuctionMiscCFG().m_mapBeginTime;
  
  if (rMapTm.empty())
  {
    XERR << "[拍卖行-计算本次拍卖开始时间] 当前时间" << curSec << "没有配置时间" <<XEND;
    //TODO test
    curSec = curSec + 4 * 60;
    return curSec;
  }

  DWORD wday = xTime::getWeek(curSec);  //[0,6]
  if (wday == 0)    //周日
    wday = 7;

  DWORD wStartTime = xTime::getWeekStart(curSec);
  bool find = false;
  
  DWORD wTime = 0;
  while (find == false)
  {
    for (auto&m : rMapTm)
    {
      if (wday < m.first)   //当天的也算到下个周期
      {
        wStartTime = wTime + wStartTime + (m.second.tm_wday - 1) * DAY_T + m.second.tm_hour * HOUR_T + m.second.tm_min * MIN_T;
        find = true;
        break;
      }
    }
    if (find)
      break;
    wday = 0;     //把时间设成下周，继续找
    wTime = WEEK_T;
  }
  
  XLOG << "[拍卖行-计算本次拍卖开始时间] 当前时间" <<curSec << "当前周几" << wday << "拍卖开始时间" << wStartTime << XEND;
  return wStartTime;
}

bool AuctionManager::saveAuctionConfig2Db()
{
  //insert or update db
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_CONFIG);
  if (!field)
  {
    return false;
  }
  xRecord record(field);
  record.put("state", m_eAuctionState);
  if (m_dwAuctionBeginTime)
    record.put("begin_time", m_dwAuctionBeginTime);
  if (m_dwSignupEndTime)
    record.put("verify_time", m_dwSignupEndTime);
  if (m_dwAuctionEndTime)
    record.put("end_time", m_dwAuctionEndTime);

  char where[64] = { 0 };
  snprintf(where, sizeof(where), "id=%llu", m_qwCurBatchId);
  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    return false;
  }
  
  XLOG << "[拍卖行-数据库保存] DB_TABLE_AUCTION_CONFIG 场次" <<m_qwCurBatchId <<"状态"<<m_eAuctionState << "开始时间" << m_dwAuctionBeginTime << "结束时间" << m_dwAuctionEndTime <<XEND;
  return true;
}

bool AuctionManager::isNowAuction()
{
  if (m_eAuctionState != EAuctionState_Auction && m_eAuctionState != EAuctionState_AuctionEnd)
    return false;
  if (m_curAuctionOrderIter == m_vecAuctionItem.end())
    return false;
    
  return true;
}

bool AuctionManager::isNowAuctionItem(QWORD batchId, DWORD itemId, QWORD signupId)
{
  if (!isNowAuction())
    return false;
  //TODO status 要不要考虑
  
  if (m_curAuctionOrderIter->m_qwBatchId == batchId && m_curAuctionOrderIter->m_dwItemId == itemId && m_curAuctionOrderIter->m_qwSignupId == signupId)
    return true;

  return false;
}

bool AuctionManager::removeOldMaxPrice(OfferPriceRecord& oldMaxPrice)
{
  if (oldMaxPrice.m_qwId == 0)
    return false;

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
  if (!field)
  {
    XERR << "[拍卖行-更新出价] 移除最高价，找不到数据表， charid" << oldMaxPrice.m_qwCharId << "zoneid" << oldMaxPrice.m_dwZoneId << "场次" << oldMaxPrice.m_qwBatchId << "itemid" << oldMaxPrice.m_dwItemId << "价格" << oldMaxPrice.m_qwPrice << XEND;
    return false;
  }
  xRecord record(field);
  record.put("type", ERecordType_OverTakePrice);
  record.put("take_status", EAuctionTakeStatus_CanTake);
  char where[64] = { 0 };
  snprintf(where, sizeof(where), "id=%llu and type=%u", oldMaxPrice.m_qwId, ERecordType_MaxOfferPrice);
  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret != 1)
  {
    XERR << "[拍卖行-更新出价] 移除最高价，数据库操作失败， charid" << oldMaxPrice.m_qwCharId << "zoneid" << oldMaxPrice.m_dwZoneId << "场次" << oldMaxPrice.m_qwBatchId << "itemid" << oldMaxPrice.m_dwItemId << "价格" << oldMaxPrice.m_qwPrice << "数据库id" << oldMaxPrice.m_qwId <<"ret" << ret << XEND;
    return false;
  }
  sendOverTakePriceMsg(oldMaxPrice.m_qwCharId);
  sendRedTip(oldMaxPrice.m_qwCharId);

  XLOG << "[拍卖行-更新出价] 移除最高价，成功， charid" << oldMaxPrice.m_qwCharId << "zoneid" << oldMaxPrice.m_dwZoneId << "场次" << oldMaxPrice.m_qwBatchId << "itemid" << oldMaxPrice.m_dwItemId << "价格" << oldMaxPrice.m_qwPrice << "数据库id" << oldMaxPrice.m_qwId << XEND;
  return true;
}

void AuctionManager::calcTax(QWORD price, QWORD& earn, QWORD& tax)
{ 
  tax = price * MiscConfig::getMe().getAuctionMiscCFG().fRate;
  if (tax > price)
    tax = price;
  earn = price - tax;
}

string AuctionManager::getFlowingWaterKey(QWORD batchId, DWORD itemId, QWORD signupId, DWORD index)
{
  stringstream ss;
  ss << batchId << "i" << itemId << "i" << signupId << "i" << index;
  return ss.str();
}

bool AuctionManager::canTake(const Cmd::AuctionRecord& logInfo)
{
  if (logInfo.take_status() == EAuctionTakeStatus_CanTake)
    return true;
  return false;
}

void AuctionManager::getAuctionStateCmd(NtfAuctionStateCCmd& cmd)
{
  cmd.set_batchid(m_qwCurBatchId);
  cmd.set_state(m_eAuctionState);
  cmd.set_auctiontime(m_dwAuctionBeginTime);
  cmd.set_delay(m_dwAuctionBeginTime > m_dwBatchCreateTime);
}

bool AuctionManager::isOpenPanel(QWORD charId)
{
  return m_mapOpenUser.find(charId) != m_mapOpenUser.end();
}

void AuctionManager::sendOverTakePriceMsg(QWORD charId)
{
  //打开面板的玩家不通知
  if (isOpenPanel(charId))
    return;

  //check online
  AuctionUser* pUser = getAuctionUser(charId);
  if (pUser && pUser->m_bOnline)
  {
    pUser->sendOverTakeCmd();
    return;
  }   

  m_setOfflineOverTake.insert(charId);
}

void AuctionManager::verifyAuction(DWORD curSec)
{
  auto updateDb = [&](const string& tableName, ERecordType type, ERecordType newType)
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, tableName);
    if (!field)
      return ;//
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "type=%u and take_status=%u and verify_time<%u", type, EAuctionTakeStatus_None, curSec);
    xRecord record(field);
    record.put("type", newType);
    record.put("take_status", EAuctionTakeStatus_CanTake);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX)
    {
      XERR << "[拍卖行-审核] 更新数据库失败" << XEND;
      return ;
    }
    XLOG << "[拍卖行-审核] 审核通过" << tableName << "条数" << ret << XEND;
    return ;
  }; 
  
  updateDb(DB_TABLE_AUCTION_ITEM_SIGNUP, ERecordType_SellSucess, ERecordType_SellSucessPass);
  updateDb(DB_TABLE_AUCTION_OFFERPRICE, ERecordType_BuySuccess, ERecordType_BuySuccessPass); 
}

void AuctionManager::packNextAuctionInfo(NtfNextAuctionInfoCCmd& cmd)
{
  cmd.Clear();
  cmd.set_batchid(m_qwCurBatchId);
  cmd.set_itemid(m_dwCurItemId);
  cmd.set_signup_id(m_qwCurSignupId);
  cmd.set_last_itemid(m_dwLastItemId);
  cmd.set_last_signup_id(m_qwLastSignupId);
  cmd.set_base_price(m_dwCurItemPrice);
  cmd.set_start_time(m_dwCurOrderStartTime);
}

QWORD AuctionManager::genOrderId()
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_AUCTION_ORDERID, "orderid");
  QWORD ret = 0;
  RedisManager::getMe().incr(key, ret);
  
  XLOG << "[拍卖行-订单号] 生成订单号" << ret << XEND;
  return ret;
}

void AuctionManager::modifyAuctionTime(DWORD time)
{
  if (m_eAuctionState == EAuctionState_Auction)
  {
    XERR << "[拍卖行-GM修改拍卖开始时间] 拍卖期间不可修改" << XEND;
    return;
  }

  if (time <= now())
  {
    XERR << "[拍卖行-GM修改拍卖开始时间] 拍卖时间非法"<< time << XEND;
    return;
  }

  if (m_eAuctionState == EAuctionState_AuctionEnd)
  {
    m_dwSignupBeginTime = 0;
  }  
  setAuctionBeginTime(time);  
  saveAuctionConfig2Db();  
  ntfAuctionState();

  XLOG << "[拍卖行-GM修改拍卖开始时间] 场次" << m_qwCurBatchId<<"state"<<m_eAuctionState <<"time" << 0 <<"报名截止时间"<<m_dwSignupEndTime <<"开始拍卖时间"<< m_dwAuctionBeginTime << XEND;
}

void AuctionManager::stopAuction()
{
  if (!isNowAuction())
  {
    XERR << "[拍卖行-GM结束拍卖] 不在拍卖期间" << XEND;
    return;
  }  
  
  for (; m_curAuctionOrderIter != m_vecAuctionItem.end(); ++m_curAuctionOrderIter)
  {
    switch (m_curAuctionOrderIter->m_eEvent)
    {
    case AuctionEvent_None:
    {
      //流拍
      sellFail2Db(m_curAuctionOrderIter->m_stSignupItem.m_qwId);
      break;
    }
    case AuctionEvent_Start:
    {
      //流拍
      sellFail2Db(m_curAuctionOrderIter->m_stSignupItem.m_qwId);
      break;
    }
    case AuctionEvent_OfferPrice:
    {
      //成功
      sellSuccess2Db(m_curAuctionOrderIter->m_stMaxPrice, m_curAuctionOrderIter->m_stSignupItem);
      break;
    }
    case AuctionEvent_Result1:
    {
      //成功
      sellSuccess2Db(m_curAuctionOrderIter->m_stMaxPrice, m_curAuctionOrderIter->m_stSignupItem);
      break;
    }
    case AuctionEvent_Result2:
    {
      //成功
      sellSuccess2Db(m_curAuctionOrderIter->m_stMaxPrice, m_curAuctionOrderIter->m_stSignupItem);
      break;
    }
    case AuctionEvent_Result3:
    {
      //成功
      sellSuccess2Db(m_curAuctionOrderIter->m_stMaxPrice, m_curAuctionOrderIter->m_stSignupItem);
      break;
    }
    case AuctionEvent_ResultSuccess:
    {     
      break;
    }
    case AuctionEvent_ResultFail:
    {      
      break;
    }
    default:
      break;
    }
  }

  enterAuctionEnd(now());
  XLOG << "[拍卖行-GM结束拍卖] 结束拍卖成功 场次"<<m_qwCurBatchId << XEND;
}

/*流拍写数据库*/
void AuctionManager::sellFail2Db(QWORD id)
{
  setOrderAuctionState(EAuctionResult_Fail);

  DWORD curSec = now();
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
  if (!field)
  {
    return;
  }
  char where[64] = { 0 };
  snprintf(where, sizeof(where), "id=%llu and take_status=%u", id, EAuctionTakeStatus_None);
  xRecord record(field);
  record.put("type", ERecordType_SellFail);
  record.put("take_status", EAuctionTakeStatus_CanTake);
  record.put("time", curSec);

  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret != 1)
  {
    XERR << "[拍卖-设置流拍] 写数据库出错上架id" << id <<"ret" <<ret << XEND;
    return;
  }
}
/*拍卖成功写数据库*/
void AuctionManager::sellSuccess2Db(OfferPriceRecord& buyer, SignupItem& seller)
{
  setOrderAuctionState(EAuctionResult_Sucess);

  DWORD curSec = now();
  DWORD verifyEndTime = curSec + MiscConfig::getMe().getAuctionMiscCFG().dwVerifyTakeDuration;
  
  //update seller
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
    if (!field)
    {
      return;
    }
    char where[64] = { 0 };
    snprintf(where, sizeof(where), "id=%llu and take_status=%u", seller.m_qwId, EAuctionTakeStatus_None);
    xRecord record(field);
    record.put("type", ERecordType_SellSucess);
    record.put("take_status", EAuctionTakeStatus_None);
    record.put("buyer_name", buyer.m_strName);
    record.put("buyer_zoneid", buyer.m_dwZoneId);
    record.put("verify_time", verifyEndTime);
    QWORD tax = 0;   //税
    QWORD earn = 0;
    QWORD zeny = AuctionManager::bcat2zeny(buyer.m_qwPrice);
    calcTax(zeny, earn, tax);
    record.put("earn", earn);
    record.put("tax", tax);
    record.put("time", curSec);

    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[拍卖-拍卖成功] 选定出售者数据库出错,DB_TABLE_AUCTION_ITEM_SIGNUP,上架者数据库id" << seller.m_qwId << "ret" << ret << XEND;
    }
  }

  //update buyer
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
    if (!field)
    {
      return;
    }
    char where[64] = { 0 };
    snprintf(where, sizeof(where), "id=%llu and take_status=%u", buyer.m_qwId, EAuctionTakeStatus_None);
    xRecord record(field);
    record.put("type", ERecordType_BuySuccess);
    record.put("take_status", EAuctionTakeStatus_None);
    record.put("seller_name", seller.m_strSellerName);
    record.put("seller_zoneid", seller.m_dwZoneId);
    record.put("verify_time", verifyEndTime);
    record.put("time", curSec);

    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret != 1)
    {
      XERR << "[拍卖-拍卖成功] 选的购买者出错,DB_TABLE_AUCTION_OFFERPRICE,购买者数据库id" << buyer.m_qwId << "ret" << ret << XEND;
    }
  }
}

void AuctionManager::insertAuciontEvent2Db()
{
  if (m_listAuctionEvent.empty())
    return;  
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_EVENT);
  if (!field)
  {
    return;
  }

  xRecordSet recordSet;  

  for (auto it = m_listAuctionEvent.begin(); it != m_listAuctionEvent.end(); ++it)
  {
    xRecord record(field);
    it->toRecord(record);
    recordSet.push(record);
  }

  QWORD retcode = thisServer->getTradeConnPool().exeInsertSet(recordSet);
  if (retcode == QWORD_MAX)
  {
    return;
  }
  XDBG << "[拍卖行-流水] 插入拍卖流水，条数" << m_listAuctionEvent.size() << XEND;
  m_listAuctionEvent.clear();
}

void AuctionManager::clearUserCache(DWORD curSec)
{
  if (m_mapAuctionUser.size() < USER_CACHE_MAX_CNT)
    return;
  if (m_eAuctionState == EAuctionState_Auction)
    return;
  DWORD oldCount = m_mapAuctionUser.size();

  for (auto it = m_mapAuctionUser.begin(); it != m_mapAuctionUser.end();)
  {
    if (it->second.m_dwActiveTime + USER_CACHE_TIME < curSec)
    {
      it = m_mapAuctionUser.erase(it);
      continue;
    }
    ++it;
  }
  XLOG << "[拍卖行-玩家缓存清空] 清空前个数" << oldCount  << "清空后个数" << m_mapAuctionUser.size() << XEND;
}

OrderItem* AuctionManager::getAuctionItem(QWORD batchId, DWORD itemId, QWORD signupId)
{
  if (m_vecAuctionItem.empty())
    return nullptr;
  for (auto it = m_vecAuctionItem.begin(); it != m_vecAuctionItem.end(); ++it)
  {
    if (it->m_qwBatchId == batchId && it->m_dwItemId == itemId && it->m_qwSignupId == signupId)
      return &(*it);
  }
  return nullptr;
}

QWORD AuctionManager::zeny2bcat(QWORD t)
{
  QWORD a = t % 10000;
  QWORD b = t / 10000;
  if (a)
    b += 1;
  return b;
}

QWORD AuctionManager::bcat2zeny(QWORD t)
{
  return t * 10000;
}

void AuctionManager::sendRedTip(QWORD charId)
{
  AuctionUser* pUser = getAuctionUser(charId);
  if (!pUser)
    return ;
  pUser->sendRedTip();
}

void AuctionManager::delteDb(DWORD curSec)
{
  if (m_eAuctionState != EAuctionState_SignUp)
    return;
  
  if (curSec < m_dwNextDelTime)
    return;
  
  m_dwNextDelTime = curSec + DAY_T;

  //clear auction_event
  static DWORD sDelEventCount = 4;
  {
    if (m_qwCurBatchId > sDelEventCount)
    {
      xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_EVENT);
      if (field)
      {
        char where[128];
        bzero(where, sizeof(where));
        snprintf(where, sizeof(where), "id<%llu", m_qwCurBatchId - sDelEventCount);
        thisServer->getTradeConnPool().exeDelete(field, where);
      }
    }
  }
  //clear auction_item
  static DWORD sDelItemCount = 30;
  {
    if (m_qwCurBatchId > sDelItemCount)
    {
      xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM);
      if (field)
      {
        char where[128];
        bzero(where, sizeof(where));
        snprintf(where, sizeof(where), "id<%llu", m_qwCurBatchId - sDelItemCount);
        thisServer->getTradeConnPool().exeDelete(field, where);
      }
    }
  }

  //clear auction_item_signup
  DWORD delTime = curSec - MiscConfig::getMe().getAuctionMiscCFG().dwReceiveTime;
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_ITEM_SIGNUP);
    if (field)
    {
      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "time<%u", delTime);
      thisServer->getTradeConnPool().exeDelete(field, where);
    }
  }

  //clear auction_offerprice
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_AUCTION_OFFERPRICE);
    if (field)
    {
      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "time<%u", delTime);
      thisServer->getTradeConnPool().exeDelete(field, where);
    }
  }
}

MessageStatHelper::MessageStatHelper(DWORD cmd, DWORD param)
{
  AuctionManager::getMe().m_oMessageStat.start(cmd, param);
}

MessageStatHelper::~MessageStatHelper()
{
  AuctionManager::getMe().m_oMessageStat.end();
}

bool MsgGuard::lock(DWORD type, QWORD charId)
{
  //屏蔽协议后客户端会存在bug
  if (type == Cmd::AUCTIONCPARAM_OPEN_AUCTION_PANEL)
    return true;

  DWORD key = type;
  QWORD dwMsgInverval = 500;   //1秒
  auto it = m_mapGuard.find(key);
  QWORD curTime = xTime::getCurMSec();
  if (it == m_mapGuard.end())
  {
    std::unordered_map<QWORD, QWORD> subMap({ { charId, curTime } });
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
      if (curTime > subIt->second + dwMsgInverval)
      {
        subIt->second = curTime;
        return true;
      }

      XLOG << "[拍卖行-协议检测] 请求过于频繁，param:" << key << "charid:" << charId << "lasttime:" << subIt->second << XEND;
      return false;
    }
  }
  return true;
}

bool MsgGuard::unlock(DWORD type, QWORD charId)
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
