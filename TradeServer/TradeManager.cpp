#include "TradeManager.h"
#include "xLuaTable.h"
#include "TradeServer.h"
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
#include "LuaManager.h"
#include "GCharManager.h"
#include "TradeUserMgr.h"
#include "RedisManager.h"

// static bool cmp2(PendingInfo a, PendingInfo b)   //时间小的放前面
// {
//   return a.pendingTime < b.pendingTime;
// }

//static bool min2(PendingInfo a, PendingInfo b)   //个数小的放前面
//{
//  return a.count < b.count;
//}

bool HotInfo::isJob(DWORD dwJob)
{
  if (vecJob.empty())
    return true;
  auto it = std::find_if(vecJob.begin(), vecJob.end(), [dwJob](EProfession a) { return  a == dwJob; });
  if (it == vecJob.end())
    return false;
  return true;
}

DWORD HotInfoMgr::getCount(DWORD itemId)
{
  auto it = m_allCount.find(itemId);
  if (it == m_allCount.end())
    return 0;
  return it->second;
}

void HotInfoMgr::addItem(DWORD itemId, DWORD count, DWORD sec)
{
  const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(itemId);
  if (pItemCfg == nullptr)
  {
    return;
  }

  const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(itemId);
  if (!pCfg)
  {
    return;
  }
  
  //m_mapCount[itemId] += count;
  m_allCount[itemId] += count;
  auto& vec = m_mapCount[itemId];
  LogCount logCount;
  logCount.time = sec;
  logCount.count = count;
  vec.push_back(logCount);

  DWORD dwType =  pCfg->dwBigCategory;
  DWORD curSec = now();

  auto it = m_mapHotInfos.find(dwType);
  if (it == m_mapHotInfos.end())
  {
    HotInfo hotInfo;
    hotInfo.dwType = dwType;
    hotInfo.dwItemid = itemId;
    hotInfo.dwCount = count;
    hotInfo.vecJob =  pItemCfg->vecEquipPro; 
    std::map<DWORD, std::vector<HotInfo>>subMap;
    auto &l = subMap[curSec];
    l.push_back(hotInfo);
    m_mapHotInfos.insert(std::make_pair(dwType, subMap));
    return;
  }

  auto& subMap = it->second;
  auto subIt = subMap.find(curSec);
  if (subIt == subMap.end())
  {
    std::vector<HotInfo> tmp;
    subIt = subMap.insert(std::make_pair(curSec, tmp)).first;
  }
  HotInfo tInfo;
  tInfo.dwType = dwType;
  tInfo.dwItemid = itemId;
  tInfo.dwCount = count;
  tInfo.vecJob = pItemCfg->vecEquipPro;
  auto& l = subIt->second;
  l.push_back(tInfo);
  return;
}

void HotInfoMgr::refresh(DWORD curSec)
{
  DWORD cutTime = curSec - MiscConfig::getMe().getTradeCFG().dwHotTime;

  for (auto it = m_mapHotInfos.begin(); it != m_mapHotInfos.end(); it++)
  {
    auto& subMap = it->second;
    for (auto subIt = subMap.begin(); subIt != subMap.end();)
    {
      if (subIt->first <= cutTime)
      {
        subIt = subMap.erase(subIt);
        XDBG << "[交易-热门删除] " << "time:" << subIt->first << XEND;
        continue;
      }
      else
        break;
    }
  }

  //log time
  {
    cutTime = curSec - MiscConfig::getMe().getTradeCFG().dwLogTime;
    for (auto it = m_mapCount.begin(); it != m_mapCount.end(); ++it)
    {
      for (auto subIt = it->second.begin(); subIt != it->second.end();)
      {
        if (subIt->time <= cutTime)
        {
          auto &count = m_allCount[it->first];
          if (count > subIt->count)
            count -= subIt->count;
          else
            count = 0;
          subIt = it->second.erase(subIt);
          continue;
        }       
        ++subIt;
      }
    }    
  }
}

std::vector<DWORD> HotInfoMgr::getRank(DWORD dwType, DWORD job)
{
  std::vector<DWORD> res;
  res.reserve(50);

  std::map<DWORD, DWORD> resMap;
  
  auto it = m_mapHotInfos.find(dwType);
  if (it == m_mapHotInfos.end())
    return res;
  
  auto& subMap = it->second;  
  for (auto subIt = subMap.begin(); subIt != subMap.end(); ++subIt)
  {
    for (auto liIt = subIt->second.begin(); liIt != subIt->second.end(); ++liIt)
    {
      if (liIt->isJob(job))
      {
        resMap[liIt->dwItemid] += liIt->dwCount;
      }
    }
  }
  if (!resMap.empty())
  {
    std::vector<DWORDPAIR> hotVec(resMap.begin(), resMap.end());
    std::sort(hotVec.begin(), hotVec.end(), CmpByValue1());
    for (auto it= hotVec.begin(); it!= hotVec.end(); ++it) {
      XDBG << "[交易-热门排序] showtype:" << dwType << "job:" << job << "itemid:" << it->first << "count:" << it->second << XEND;
      res.push_back(it->first);
    }
  }
  return res;
}

std::vector<DWORD> HotInfoMgr::getPublicityRank()
{
  std::vector<DWORD> res;
  res.reserve(50);

  DWORD typeCount = 2;

  //公示期物品
  DWORD dwTotalCount2 = 0;
  std::map<DWORD, DWORD> wantCount;
  auto func1 = [&](const xEntry* pEntry)
  {
    const STradeItemTypeData* pBase = static_cast<const STradeItemTypeData*>(pEntry);
    if (!pBase)
      return;
    if (pBase->getHotSale() == 0)
      return;
    wantCount[pBase->id] = typeCount;
    dwTotalCount2 += typeCount;
  };
  Table<STradeItemTypeData>* pTradeItemList = TableManager::getMe().getTradeItemTypeCFGListNoConst();
  if (pTradeItemList != nullptr)
    pTradeItemList->foreachNoConst(func1);

  std::vector<DWORDPAIR> hotVec;
  std::map<DWORD, DWORD> insertedMap;
  std::vector<DWORDPAIR> extraVec;
  std::vector<DWORDPAIR> retVec;

  for (auto &v : m_mapPublicity)
  {
    auto p = std::make_pair(v.first, v.second.size());
    hotVec.push_back(p);
  }

  std::sort(hotVec.begin(), hotVec.end(), CmpByValue1());
  for (auto it = hotVec.begin(); it != hotVec.end(); ++it)
  {
    const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(it->first);
    if (!pCfg)
    {
      continue;;
    }
    DWORD dwType = pCfg->dwBigCategory;

    auto &count = insertedMap[dwType];
    if (count >= typeCount)
    {
      extraVec.push_back(*it);
      continue;
    }
    count++;

    XDBG << "[交易-公示期-热门排序] showtype:" << dwType << "itemid:" << it->first << "buyer count:" << it->second << XEND;
    retVec.push_back(*it);
  }
  
  for (auto &v : extraVec)
  {
    retVec.push_back(v);
  }

  std::sort(retVec.begin(), retVec.end(), CmpByValue1());

  for (auto &v : retVec)
  {
    res.push_back(v.first);
  }

  return res;
}

void HotInfoMgr::addPublicity(DWORD itemId, QWORD buyerId)
{
  auto &map = m_mapPublicity[itemId];
  map.insert(buyerId);
}

void HotInfoMgr::decPublicity(DWORD itemId, QWORD buyerId)
{
  auto it = m_mapPublicity.find(itemId);
  if (it != m_mapPublicity.end())
  {
    if (buyerId != 0)
      it->second.erase(buyerId);
    if (it->second.size() <= 1)   //为0
    {
      //erase
      m_mapPublicity.erase(it);
    }
  }  
}

void PendingInfo::pack2Proto(Cmd::TradeItemBaseInfo* pOut) const
{
  if (pOut == nullptr)
    return;
  pOut->set_order_id(orderId);
  pOut->set_itemid(itemId);
  pOut->set_count(count);
  pOut->set_price(price);
  if (itemData.has_base())      //堆叠物品为空
  {
    pOut->mutable_item_data()->CopyFrom(itemData);
    pOut->mutable_item_data()->mutable_base()->set_count(count);      //以外面的count 为准
  }
}

void PendingInfo::marshal(std::string& out) const
{
  out.clear();
  if (itemData.has_base())
    itemData.SerializeToString(&out);
}
void PendingInfo::unmarshal(std::string& in)
{
  if (in.empty())
  {
    itemData.Clear();
    return;
  }  
  itemData.ParseFromString(in);
}

/************************************************************************/
/*      StoreBaseMgr                                                                */
/************************************************************************/

StoreBaseMgr::StoreBaseMgr(DWORD itemId):
  m_dwItemId(itemId)
{
  m_vecBriefBuyInfo.reserve(m_dwMaxShowCount);
}

StoreBaseMgr::~StoreBaseMgr() 
{
  m_playerStore.clear();
}

bool StoreBaseMgr::hasOrder(QWORD orderId)
{
  auto it = LIST_FIND(m_playerStore, orderId);
  if (it == m_playerStore.end())
  {
    return false;
  }
  return true;
}

void StoreBaseMgr::addPlayerPendingInfo(const PendingInfo & info)
{
  m_playerStore.push_back(info);
  addPendingCount(info.refineLv, info.damage(), info.upgradeLv(), info.count);
}

DWORD StoreBaseMgr::getPlayerCount()
{
  DWORD total = 0;
  for (auto it = m_playerStore.begin(); it != m_playerStore.end(); ++it)   //可以改
  {
    total += it->count;
  }
  //  XINF("[交易-内存仓数目] 玩家仓：%u 系统仓：%u 锁仓数：%u", total, m_sysCount, m_lockCount);
  return total;
}

TVecQWORD StoreBaseMgr::getRandomPending(DWORD count)
{
  if (count > m_playerStore.size())
    count = m_playerStore.size();
  
  TVecQWORD vecOrderId;
  vecOrderId.reserve(m_playerStore.size());
  for (auto it = m_playerStore.begin(); it != m_playerStore.end(); ++it)
  {
    vecOrderId.push_back(it->orderId);
  }
  
  std::random_shuffle(vecOrderId.begin(), vecOrderId.begin() + count);
  vecOrderId.resize(count);
  return vecOrderId;
}

bool StoreBaseMgr::addPending(QWORD charId, std::string name, const Cmd::TradeItemBaseInfo& itemBaseInfo)
{
  if (itemBaseInfo.count() == 0)
  {
    XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price:" << itemBaseInfo.price() << "charid:" << charId << "count=0" << XEND;
    return false;
  }
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price:" << itemBaseInfo.price() << "charid:" << charId << "数据库错误" << XEND;
    return false;
  }
  const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(m_dwItemId);
  if (pItemCfg == nullptr)
  {
    XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price:" << itemBaseInfo.price() << "charid:" << charId << "策划表找不到物品" << XEND;
    return false;
  }

  xRecord record(field);
  record.put("itemid", m_dwItemId);
  record.put("price", itemBaseInfo.price());
  record.put("count", itemBaseInfo.count());
  record.put("sellerid", charId);
  record.put("name", name);
  record.put("pendingtime", now());
  DWORD refineLv = 0;
  if (itemBaseInfo.has_item_data() && !isOverlap())
  {
    std::string strItemData;
    refineLv = itemBaseInfo.item_data().equip().refinelv();
    record.put("refine_lv", refineLv);
    itemBaseInfo.item_data().SerializeToString(&strItemData);
    record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());
  }
  record.put("is_overlap", isOverlap());

  QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    //插入失败
    XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price"<< itemBaseInfo.price()<< "charid:" << charId << " 插入数据库失败" << field->m_strDatabase << field->m_strTable << XEND;
    return false;
  }

  PendingInfo pendingInfo;
  pendingInfo.orderId = ret;
  pendingInfo.itemId = m_dwItemId;
  pendingInfo.name = name;
  pendingInfo.count = itemBaseInfo.count();
  pendingInfo.isOverlap = m_overLap;
  pendingInfo.price = itemBaseInfo.price();
  pendingInfo.sellerId = charId;
  pendingInfo.refineLv = refineLv;
  //pendingInfo.refineLv = itemBaseInfo.refine_lv();
  //pendingInfo.refineData = strRefineData;
  if (itemBaseInfo.has_item_data() && !isOverlap())
  {
    pendingInfo.itemData = itemBaseInfo.item_data();
  }
  else {
    pendingInfo.itemData.Clear();
  }
  pendingInfo.pendingTime = record.get<QWORD>("pendingtime");
  addPlayerPendingInfo(pendingInfo);
  XINF << "[交易-增加挂单] itemid:" << m_dwItemId <<  "charid:" << charId << "新增挂单成功，" << pendingInfo.toString().c_str() << XEND;
  return true;
}

bool StoreBaseMgr::resell(QWORD charid, DWORD zoneId, QWORD orderId, DWORD newPrice)
{
  if (orderId == 0)
    return false;

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    MsgParams params;
    TradeManager::getMe().sendSysMsg(charid, zoneId, SYS_MSG_DB_ERROR, EMESSAGETYPE_FRAME, params);
    return false;
  }
  PendingInfo pendingInfo;
  if (TradeManager::getPendInfoFromDb(orderId, pendingInfo) == false)
  {
    return false;
  }
  pendingInfo.pendingTime = now();
  pendingInfo.price = newPrice;
  xRecord record(field);
  record.put("pendingtime", pendingInfo.pendingTime);
  record.put("price", pendingInfo.price);

  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%llu", orderId);

  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {//
    XERR << "[交易-重新上架] charid:" << charid << "失败， 数据库操作失败，orderid：" << orderId << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
    return false;
  }

  addPlayerPendingInfo(pendingInfo);
  XINF << "[交易-重新上架] charid:" << charid << "重新上架成功，" << pendingInfo.toString() << XEND;
  return true;
}

bool StoreBaseMgr::reducePending(QWORD orderId, DWORD reduceCount, bool isAutoErase, bool& needErase, PendingInfo* pPendingInfo)
{
  needErase = false;
  if (orderId == 0)
  {
    XERR << "[交易-修改订单] 失败， order id 为0" << XEND;
    return false;
  }
  PENDING_INFO_LIST_T::iterator iter;
  iter = LIST_FIND(m_playerStore, orderId);
  if (iter == m_playerStore.end())
  {
    XERR << "[交易-修改订单] 失败， 内存玩加仓中找不到该订单，orderid：" << orderId << "reduceCount：" << reduceCount << "iserase：" << isAutoErase << XEND;
    return false;
  }

  if (iter->count < reduceCount)
  {
    XERR << "[交易-修改订单] 失败， 购买个数大于仓里的数目，orderid：" << orderId << iter->orderId << "reduceCount：" << reduceCount << "仓数：" << iter->count <<"iserase：" << isAutoErase << "itemid" <<iter->itemId <<"sellerid"<<iter->sellerId << XEND;
    return false;
  }

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    XERR << "[交易-修改订单] 失败， 获取不到数据库， table：" << DB_TABLE_PENDING_LIST << "orderid：" << orderId << "reduceCount：" << reduceCount << XEND;
    return false;
  }
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%llu", orderId);

  DWORD newCount = iter->count - reduceCount;
  if (newCount == 0)
  {
    //直接删除掉
    QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-修改订单] 失败， 数据库操作失败，orderid：" << orderId << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
      return false;
    }
  }
  else
  {
    xRecord record(field);
    record.put("count", newCount);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX)
    {//
      XERR << "[交易-修改订单] 失败， 数据库操作失败，orderid：" << orderId << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
      return false;
    }
  }
  
  if (iter->sellerId)
  {
    TradeManager::getMe().ListNtf(iter->sellerId, ELIST_NTF_MY_PENDING);
  }

  if (pPendingInfo)
  {
    *pPendingInfo = (*iter);
    pPendingInfo->count = reduceCount;
  }
  //update 
  iter->count = newCount;
  iter->sellCount = 0;  // must clear
  decPendingCount(iter->refineLv, iter->damage(), iter->upgradeLv(), reduceCount);
  if (iter->count == 0)
  {
    needErase = true;
    if (isAutoErase)
    {
      m_playerStore.erase(iter);
    }
  }
  return true;
}

bool StoreBaseMgr::reducePendingAutoErase(QWORD orderId, DWORD reduceCount, PendingInfo* pPendingInfo)
{
  bool needErase;
  return reducePending(orderId, reduceCount, true, needErase, pPendingInfo);
}

DWORD StoreBaseMgr::getPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv)
{
  DWORD key = getKey(refineLv, isDamage, upgradeLv);
  auto it = m_pendingCount.find(key);
  if (it != m_pendingCount.end())
    return it->second;
  return 0;
}

PendingInfo* StoreBaseMgr::getPendingInfo(QWORD orderId)
{
  auto it = LIST_FIND(m_playerStore, orderId);
  if (it == m_playerStore.end())
  {
    return nullptr;
  }
  return &(*it);
}

DWORD  StoreBaseMgr::getKey(DWORD refineLv, bool bDamage, DWORD upgradeLv)
{
  DWORD key = refineLv;
  if (bDamage)
  {
    key += 100;
  }
  else
  {
    key += 200;
  }
  key = key * 10 + upgradeLv;
  return key;
}

void StoreBaseMgr::addPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv, DWORD count)
{
  DWORD key = getKey(refineLv, isDamage, upgradeLv);
  m_pendingCount[key] += count;
}

void StoreBaseMgr::decPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv, DWORD count)
{
  DWORD key = getKey(refineLv, isDamage, upgradeLv);
  if (m_pendingCount[key] >= count)
    m_pendingCount[key] -= count;
  else
    m_pendingCount[key] = 0;
}

void StoreBaseMgr::addBriefBuyInfo(const string& buyerName)
{
  Cmd::BriefBuyInfo info;
  info.set_name(buyerName);
  info.set_time(now());
  
  if (m_vecBriefBuyInfo.size() >= m_dwMaxShowCount)
  {
    m_vecBriefBuyInfo.erase(m_vecBriefBuyInfo.begin());
  }
  m_vecBriefBuyInfo.push_back(info);
}

bool StoreBaseMgr::tradeSecurityCancel(SecurityCmd& cmd)
{
  if (m_playerStore.empty())
    return true;

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    XERR << "[交易-安全指令-过期物品] 失败， 获取不到数据库， table：" << DB_TABLE_PENDING_LIST << "itemid"<<m_dwItemId << XEND;
    return false;
  }
  char where[128];
  DWORD newTime = now() - MiscConfig::getMe().getTradeCFG().dwExpireTime - 120;
  xRecord record(field);
  record.put("pendingtime", newTime);
  QWORD affectRows = 0;

  for (auto it = m_playerStore.begin(); it != m_playerStore.end();)
  {
    if (cmd.needProcess(it->sellerId, it->itemId, it->refinelv()) == false)
    {
      ++it;
      continue;
    }
    PendingInfo& info = *it;

    if (!lockOrder(info.orderId))
    {
      ++it;
      continue;
    }

    unlockOrder(info.orderId);

    //update db
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id=%llu", info.orderId);
    affectRows = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (affectRows == QWORD_MAX)
      XERR << "[交易-安全指令-过期物品] 失败，update执行错误 itemid" << m_dwItemId << "orderid" << info.orderId << XEND;
    else
      XLOG << "[交易-安全指令-过期物品] 成功，update执行成功 itemid" << m_dwItemId << "orderid" << info.orderId <<"sellerid"<<info.sellerId<<"refinelv"<<info.refinelv()<<"count"<<info.count<<"新的过期时间"<<newTime << XEND;
    
    decPendingCount(info.refineLv, info.damage(), info.upgradeLv(), info.count);
    it = m_playerStore.erase(it);
  }

  return true;
}

/************************************************************************/
/* OverlapStoreMgr                                                                     */
/************************************************************************/

OverlapStoreMgr::OverlapStoreMgr(DWORD itemId):StoreBaseMgr(itemId)
{
  m_overLap = true;
  m_lockCount = 0;
}
OverlapStoreMgr::~OverlapStoreMgr()
{

}

DWORD OverlapStoreMgr::getLockCount() { return m_lockCount; }

void OverlapStoreMgr::addLockCount(DWORD count)
{
  m_lockCount += count;
}

void OverlapStoreMgr::reduceLockCount(DWORD count)
{
  m_lockCount -= count;
}

bool OverlapStoreMgr::lockOrder(QWORD orderId)
{
  PendingInfo* pInfo = getPendingInfo(orderId);
  if (pInfo == nullptr)
    return false;
  return lock(pInfo->count);
}

bool OverlapStoreMgr::unlockOrder(QWORD orderId)
{
  PendingInfo* pInfo = getPendingInfo(orderId);
  if (pInfo == nullptr)
    return false;
  return unlock(pInfo->count);
}

void OverlapStoreMgr::reducePlayerPendingInfo(QWORD orderId, DWORD newCount)
{
  auto it = std::find_if(m_playerStore.begin(), m_playerStore.end(), [orderId](PendingInfo a) { return a.orderId == orderId; });
  if (it == m_playerStore.end())
    return;

  if (0 != newCount)
  {
    it->count = newCount;
  }
  else
  {//删了这单
    m_playerStore.erase(it);
  }
}

bool OverlapStoreMgr::lock(DWORD count)
{
  if (getTotalCount() >= getLockCount() + count)
  {
    addLockCount(count);
    return true;
  }
  return false;
}

bool OverlapStoreMgr::unlock(DWORD count)
{
  if (getLockCount() >= count)
  {
    reduceLockCount(count);
    return true;
  }
  return false;
}

void OverlapStoreMgr::checkOffTheShelf()
{
  time_t nowTime = now();
  //m_playerStore.sort(cmp2);   //排序

  for (auto it = m_playerStore.begin(); it != m_playerStore.end();)
  {
    PendingInfo& info = *it;
    time_t diff = nowTime - info.pendingTime;
    if (diff >= MiscConfig::getMe().getTradeCFG().dwExpireTime)
    {      
      if (lock(info.count))
      {
        unlock(info.count);
        //下架
        TradeManager::getMe().ListNtf(info.sellerId, ELIST_NTF_MY_PENDING);
        XINF << "[交易-检查下架] 堆叠物品，挂单下架： " << info.toString().c_str() << XEND;
        decPendingCount(info.refineLv, info.damage(), 0, info.count);
        it = m_playerStore.erase(it);
        continue;
      }
      else
      {
        //有人在购买，不下架。让他买
        XINF << "[交易-检查下架] 堆叠物品，挂单下架失败，有人在购买： " << info.toString() << XEND;
      }
    }
    ++it;
  }
}

/*出仓*/
bool OverlapStoreMgr::outStore(QWORD buyerId, const string& name, DWORD zoneId, DWORD buyPrice, DWORD count)
{
  if (getTotalCount() < count)
  {
    XERR << "[交易-购买] 出仓失败， 价格的物品个数不够，不可能发生。buyerid"<<buyerId<<"zoneid"<<zoneId<<"buyerprice"<<buyPrice<< "itemid:" << m_dwItemId  << "totalcount:" << getTotalCount() << "buycount:" << count <<"retmoney"<<count*buyPrice << XEND;
    return false;
  }
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    MsgParams params;
    TradeManager::getMe().sendSysMsg(buyerId, zoneId, SYS_MSG_DB_ERROR, EMESSAGETYPE_FRAME, params);
    return false;
  }
  
  //玩家出仓
  PendingInfo pendingInfo = *(m_playerStore.begin());
  Cmd::NameInfoList nameList;
  DWORD trueCount = playerOutStore(buyerId, name, zoneId, count, pendingInfo, buyPrice, nameList);
  if (trueCount == count)
  {
    //合并 一次给装备 
    pendingInfo.count = count;
    TradeManager::getMe().addItem(EOperType_NoramlBuy, buyerId, zoneId, pendingInfo);
    TradeManager::getMe().insertBuyedDbLog(EOperType_NoramlBuy, buyerId, pendingInfo, nameList, 0, 0, 0);
  }
  else
  {
    pendingInfo.count = count;
    TradeManager::getMe().addItem(EOperType_NoramlBuy, buyerId, zoneId, pendingInfo);
    TradeManager::getMe().insertBuyedDbLog(EOperType_NoramlBuy, buyerId, pendingInfo, nameList, 0, 0, 0);
    XERR << "[交易-购买] 出仓个数异常，不可能发生。buyerid" << buyerId << "zoneid" << zoneId << "buyerprice" << buyPrice << "itemid:" << m_dwItemId << "totalcount:" << getTotalCount() << "buycount:" << count << "真正出仓个数" << trueCount << "retmoney" << count*buyPrice << XEND;
  }
  
  addBriefBuyInfo(name);

  return true;
}

/*玩家出仓*/
DWORD OverlapStoreMgr::playerOutStore(QWORD buyerId, const string& name,  DWORD zoneId, DWORD wantCount, PendingInfo& pendingInfo, DWORD buyPrice, Cmd::NameInfoList& nameList)
{
  DWORD trueCount = 0;

  DWORD needCount = wantCount;
 
  std::vector<PENDING_INFO_LIST_T::iterator> sellers;
  sellers.reserve(m_playerStore.size());
  for (PENDING_INFO_LIST_T::iterator it = m_playerStore.begin(); it != m_playerStore.end(); ++it)
  {
    it->sellCount = 0;
    sellers.push_back(it);
  }

  //随机排序一下
  //random_shuffle(sellers.begin(), sellers.end());

  while (needCount)
  {
    for (auto it = sellers.begin(); it != sellers.end();)
    {
      if (needCount == 0)
        break;
      bool nextSeller = false;
      for (DWORD i = 0; i < 30; ++i)
      {
        if (needCount == 0)
          break;
        needCount--;
        if ((*it)->addSellCount(1) == false)
        {
        /*  XLOG << "[交易-玩家出仓-明细] buyerid" << buyerId << "itemid" << m_dwItemId <<"orderid" <<(*it)->orderId <<"sellCount"<<(*it)->sellCount<<"allcount" <<(*it)->count << "needcount" << needCount << "wantcount" << wantCount << XEND;*/
          it = sellers.erase(it);
          nextSeller = true;
          break;
        }
      }  

      if (nextSeller)
        continue;
      ++it;
    }
  }

  //
  xRecordSet recordSet;
  
  for (auto it = m_playerStore.begin(); it != m_playerStore.end();)
  {
    bool needErase;
    if (it->sellCount == 0)
    {
      ++it;
      continue;
    }
    DWORD reduceCount = it->sellCount;
    DWORD sellerZoneid = TradeManager::getMe().getMyZoneId(it->sellerId);

    if (reducePending(it->orderId, reduceCount, false, needErase, &pendingInfo))
    {
      {
        pendingInfo.price = buyPrice;
        TradeManager::getMe().addMoney(EOperType_NormalSell, pendingInfo.sellerId, 0, pendingInfo, buyerId);
        
        NameInfoList buyerInfoList;
        NameInfo* pNameInfo = buyerInfoList.add_name_infos();
        if (pNameInfo)
        {
          pNameInfo->set_name(name);
          pNameInfo->set_count(pendingInfo.count);
          pNameInfo->set_zoneid(zoneId);
        }

        TradeManager::getMe().insertSelledDbLog2(EOperType_NormalSell, buyerId, pendingInfo, buyerInfoList, recordSet);
        trueCount += reduceCount;
      }


      NameInfo* pNameInfo = nameList.add_name_infos();
      if (pNameInfo)
      {
        pNameInfo->set_name(pendingInfo.name);
        pNameInfo->set_count(reduceCount);
        pNameInfo->set_zoneid(sellerZoneid);
      }
      
      XINF << "[交易-购买] 玩家仓出仓 buyerid"<<buyerId << "itemid"<<pendingInfo.itemId <<"price" <<pendingInfo.price  << "seller:" << pendingInfo.sellerId <<"sellerzoneid"<<sellerZoneid << "出仓个数：" << pendingInfo.count << "总共要出仓：" << wantCount << XEND;
      
      if (needErase)
      {
        it = m_playerStore.erase(it);
        continue;
      }
    }
    else
    {
      XERR << "[交易-购买] 玩家仓出仓失败 buyerid" << buyerId << "itemid" << it->itemId <<"order id"<<it->orderId << "price" << it->price<< "seller:" << it->sellerId << "sellerzoneid" << sellerZoneid << "出仓个数：" << reduceCount <<it->sellCount <<"仓内剩余个数" <<it->count << "总共要出仓：" << wantCount << XEND;
    }
    ++it;
  }

  TradeManager::getMe().insertDbLogSet(recordSet);

  XINF << "[交易-购买] 玩家仓出仓，buyerid:" << buyerId <<"itemid"<<m_dwItemId <<"price"<<pendingInfo.price<< "想要出仓：" << wantCount << "真实出仓：" << trueCount << XEND;
  return trueCount;
}

bool OverlapStoreMgr::adjustSelfPrice()
{
  if (m_playerStore.empty())
    return false;

  auto it = m_playerStore.begin();
  DWORD newPrice = TradeManager::getMe().getServerPrice(it->itemData, it->itemId);
  if (newPrice == it->price)
    return false;
  DWORD oldPrice = it->price;
  for (auto &v : m_playerStore)
  {
    v.price = newPrice;
  }
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (field)
  {
    xRecord record(field);
    record.put("price", newPrice);
    char where[256] = { 0 };
    DWORD LIMIT_COUNT = 5000;
    snprintf(where, sizeof(where), "itemid=%u and pendingtime>%u and price <>%u limit %u", it->itemId, now() - MiscConfig::getMe().getTradeCFG().dwExpireTime, newPrice, LIMIT_COUNT);
    QWORD affectRows = 0;
    do
    {
      affectRows = thisServer->getTradeConnPool().exeUpdate(record, where);
      if (affectRows == QWORD_MAX)
      {
        XERR << "[交易-上架价格调整] 批量修改失败，itemid" << it->itemId << "old price" << oldPrice << " NEW price" << newPrice << XEND;
        break;
      }
      if (affectRows < LIMIT_COUNT)
      {
        XINF << "[交易-上架价格调整] 批量修改成功，itemid" << it->itemId << "old price" << oldPrice << " NEW price" << newPrice << XEND;
        break;
      }
    } while (affectRows > 0);
  }
  else
  {
    XERR << "[交易-上架价格调整]  失败，找不到数据库，itemid" << it->itemId << "old price" << oldPrice << " NEW price" << newPrice << XEND;
  }
  return true;
}

bool OverlapStoreMgr::checkBuyPrice(DWORD price)
{
  if(m_playerStore.empty())
    return false;

  auto it = m_playerStore.begin();
  return price == it->price;
}

/************************************************************************/
/* NonOverlapStoreMgr                                                                     */
/************************************************************************/

NonOverlapStoreMgr::NonOverlapStoreMgr(DWORD itemId) :StoreBaseMgr(itemId)
{
  m_overLap = false;
}

NonOverlapStoreMgr::~NonOverlapStoreMgr()
{
}

bool NonOverlapStoreMgr::lockOrder(QWORD orderId)
{
  PendingInfo* pInfo = getPendingInfo(orderId);
  if (pInfo == nullptr)
    return false;
  return pInfo->lock();
}
bool NonOverlapStoreMgr::unlockOrder(QWORD orderId)
{
  PendingInfo* pInfo = getPendingInfo(orderId);
  if (pInfo == nullptr)
    return false;
  return pInfo->unlock();
}
void NonOverlapStoreMgr::checkOffTheShelf()
{
  time_t nowTime = now();

  //玩家仓
  for (auto it = m_playerStore.begin(); it != m_playerStore.end();)
  {
    PendingInfo& info = *it;
    time_t diff = nowTime - info.pendingTime;
    if (diff >= MiscConfig::getMe().getTradeCFG().dwExpireTime)
    {      
      if (it->lock())
      {
        it->unlock();
        //下架
        TradeManager::getMe().ListNtf(info.sellerId, ELIST_NTF_MY_PENDING);
        XINF << "[交易-检查下架] 非堆叠物品，挂单下架： " << info.toString() << XEND;

        decPendingCount(info.refineLv, info.damage(), info.upgradeLv(), info.count);
        it = m_playerStore.erase(it);
        continue;
      }
      else
      {
        //有人在购买，不下架。让他买
        XINF << "[交易-检查下架] 非堆叠物品，挂单下架失败，该挂单被锁定： " << info.toString() << XEND;
      }
    }
    ++it;
  }
}

//出仓
bool NonOverlapStoreMgr::outStore(QWORD orderId, QWORD buyerId,const string& name,  DWORD zoneId, DWORD buyPrice, DWORD outCount/* = 1*/)
{
  auto it = LIST_FIND(m_playerStore, orderId);
  if (it != m_playerStore.end())
  {
    PendingInfo pendingInfo;
    if (reducePendingAutoErase(orderId, 1, &pendingInfo) == false)
    {
      XERR << "[交易-购买-出仓] 非堆叠，出仓失败" << XEND;
      return false;
    }
    pendingInfo.price = buyPrice;     //此处可能服务器调整价格了，以购买的价格为准。
    NameInfoList infoList;
    NameInfo* pNameInfo = infoList.add_name_infos();
    if (pNameInfo) //卖家信息
    {
      pNameInfo->set_name(name);
      pNameInfo->set_count(1);      
      pNameInfo->set_zoneid(zoneId);
    }
    TradeManager::getMe().addMoney(EOperType_NormalSell, pendingInfo.sellerId, 0, pendingInfo, buyerId);
    TradeManager::getMe().insertSelledDbLog(EOperType_NormalSell, buyerId, pendingInfo, infoList);

    {
      infoList.Clear();
      NameInfo* pNameInfo = infoList.add_name_infos();
      if (pNameInfo)
      {
        pNameInfo->set_name(pendingInfo.name);
        pNameInfo->set_count(pendingInfo.count);
        DWORD sellerZoneId = TradeManager::getMe().getMyZoneId(pendingInfo.sellerId);
        pNameInfo->set_zoneid(sellerZoneId);
      }
      TradeManager::getMe().addItem(EOperType_NoramlBuy, buyerId, zoneId, pendingInfo);
      TradeManager::getMe().insertBuyedDbLog(EOperType_NoramlBuy, buyerId, pendingInfo, infoList, 0, 0, 0);
    }
    addBriefBuyInfo(name);

    return true;
  }
  return false;
}

bool NonOverlapStoreMgr::adjustSelfPrice()
{
  DWORD newPrice = 0;
  DWORD oldPrice = 0;
  for (auto &v : m_playerStore)
  {
    newPrice = TradeManager::getMe().getServerPrice(v.itemData, v.itemId);
    if (newPrice == v.price)
      continue;
    oldPrice = v.price;
    v.price = newPrice;
    TradeManager::getMe().savePendingPrice(v.orderId, v.itemId, oldPrice, newPrice);
  }

  return true;
}

bool NonOverlapStoreMgr::checkBuyPrice(QWORD orderId, DWORD price)
{
  auto it = LIST_FIND(m_playerStore, orderId);
  if (it != m_playerStore.end())
  {
    return price == it->price;
  }
  return false;
}

/************************************************************************/
/*        TradeItemInfo                                                              */
/************************************************************************/

TradeItemInfo::TradeItemInfo(DWORD dwId, bool isOverlap):
  m_isOverlap(isOverlap),
  m_dwItemId(dwId),
  m_lastCalcPriceTime(0)
{
}

TradeItemInfo::~TradeItemInfo()
{
  if (m_pStoreMgr)
  {
    delete m_pStoreMgr;
  }
}

void TradeItemInfo::init(DWORD dwServerPrice, DWORD dwLastCalcPriceTime, DWORD T)
{
  m_dwServerPrice = dwServerPrice;
  m_lastCalcPriceTime = dwLastCalcPriceTime;
  m_dwT = T;
}

void TradeItemInfo::loadPlayerPendingInfo(PendingInfo& pendingInfo)
{
  DWORD price = TradeManager::getMe().getServerPrice(pendingInfo.itemData, pendingInfo.itemId);
  if (price != pendingInfo.price)
  {
    TradeManager::getMe().savePendingPrice(pendingInfo.orderId, pendingInfo.itemId, pendingInfo.price, price);
    pendingInfo.price = price;
  }

  if (m_pStoreMgr == nullptr)
  {
    if (m_isOverlap)
    {
      m_pStoreMgr = NEW OverlapStoreMgr(m_dwItemId);
      if (m_pStoreMgr == nullptr)
      {
        return;
      }
    }
    else
    {
      m_pStoreMgr = NEW NonOverlapStoreMgr(m_dwItemId);
      if (m_pStoreMgr == nullptr)
      {
        return;
      }      
    }   
  }  
  m_pStoreMgr->addPlayerPendingInfo(pendingInfo);
}

void TradeItemInfo::saveToDb()
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_TRADE_INFO);
  if (field)
  {
    xRecord record(field);
    record.put("itemid", m_dwItemId);
    record.put("refineLv", 0);
    //record.put("last_calc_price_time", m_lastCalcPriceTime);
    QWORD ret = thisServer->getTradeConnPool().exeInsert(record, false, true);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-数据库] 保存数据库失败 database:" << field->m_strDatabase << "table:" << field->m_strTable << "code:" << ret
        << "itemid:" << m_dwItemId  << "last_calc_price_time:" << m_lastCalcPriceTime << XEND;
    }
    else
    {
      //XINF("[交易-数据库] 保存数据库成功 database:%s  table:%s itemid:%u, last_server_price:%u last_calc_price_time:%lu last_sys_pending_time:%lu", field->m_strDatabase.c_str(), field->m_strTable.c_str(),
      //  m_dwItemId, m_dwServerPrice, m_lastCalcPriceTime, m_lastSysPendingTime);
    }
  }
}

void TradeItemInfo::saveServerPrice(DWORD curSec, DWORD refineLv, DWORD T, DWORD K, DWORD D, float QK, DWORD P0, DWORD oldP, DWORD newP)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_TRADE_INFO);
  if (field)
  {
    xRecord record(field);
    record.put("itemid", m_dwItemId);
    record.put("last_server_price", newP);
    record.put("last_calc_price_time", m_lastCalcPriceTime);
    record.put("t", m_dwT);
    QWORD ret = thisServer->getTradeConnPool().exeInsert(record, false, true);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-服务器价格调整] 保存数据库失败 database:" << field->m_strDatabase << "table:" << field->m_strTable << "code:" << ret
        << "itemid:" << m_dwItemId << "last_server_price:" << newP << "last_calc_price_time:" << m_lastCalcPriceTime<< XEND;
    }
    else
    {
      XINF << "[交易-服务器价格调整] 保存数据库成功 database:" << field->m_strDatabase << "table:" << field->m_strTable << "code:" << ret
        << "itemid:" << m_dwItemId << "last_server_price:" << newP << "last_calc_price_time:" << m_lastCalcPriceTime <<"t"<<m_dwT << XEND;
    }
  }
    
  //
  field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PRICE_ADJUST);
  if (field)
  {
    xRecord record(field);
    record.put("itemid", m_dwItemId);
    record.put("refinelv", refineLv);
    record.put("last_time", curSec);
    record.put("t", T);
    record.put("d", D);
    record.put("k", K);
    record.put("qk", QK);
    record.put("p0", P0);
    record.put("oldprice", oldP);
    record.put("newprice", newP);
    QWORD ret = thisServer->getTradeConnPool().exeInsert(record, false, true);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-数据库] 保存数据库失败 database:" << field->m_strDatabase << "table:" << field->m_strTable << "code:" << ret
        << "itemid:" << m_dwItemId << "last_server_price:" << newP << "last_calc_price_time:" << m_lastCalcPriceTime << XEND;
    }
    else
    {
      //XINF("[交易-数据库] 保存数据库成功 database:%s  table:%s itemid:%u, last_server_price:%u last_calc_price_time:%lu last_sys_pending_time:%lu", field->m_strDatabase.c_str(), field->m_strTable.c_str(),
      //  m_dwItemId, m_dwServerPrice, m_lastCalcPriceTime, m_lastSysPendingTime);
    }
  }
}

/*挂单*/
bool TradeItemInfo::addNewPending(QWORD charId, std::string name, DWORD zoneId, const Cmd::ReduceItemRecordTrade& rev)
{
  const Cmd::TradeItemBaseInfo& itemBaseInfo = rev.item_info();
    
  if (m_pStoreMgr == nullptr)
  {
    if (m_isOverlap)
    {
      m_pStoreMgr = NEW OverlapStoreMgr(m_dwItemId);
      if (m_pStoreMgr == nullptr)
      {
        XERR << "[交易-增加挂单] 分配内存错误 charid：" << charId << XEND;
        TradeManager::getMe().sendSysMsg(charId, zoneId, SYS_MSG_SYS_ERROR, EMESSAGETYPE_FRAME, MsgParams());
        return false;
      }
    }
    else
    {
      m_pStoreMgr = NEW NonOverlapStoreMgr(m_dwItemId);
      if (m_pStoreMgr == nullptr)
      {
        XERR << "[交易-增加挂单] 分配内存错误 charid：" << charId << XEND;
        TradeManager::getMe().sendSysMsg(charId, zoneId, SYS_MSG_SYS_ERROR, EMESSAGETYPE_FRAME, MsgParams());
        return false;
      }
    }
  }

  if (!m_pStoreMgr)
    return false;
  
  if (rev.has_is_resell() && rev.is_resell())
  {//重新上架
    return m_pStoreMgr->resell(rev.charid(), zoneId, rev.orderid(), rev.item_info().price());
  }
  else
  {
    return m_pStoreMgr->addPending(charId, name, itemBaseInfo);
  }
}

void TradeItemInfo::checkExpirePending()
{ 
  if (!m_pStoreMgr) return;
  if (m_nextExpireTime == 0 || m_nextExpireTime > now())  
    m_pStoreMgr->checkOffTheShelf();
  else
  {
    m_nextExpireTime = now() + randBetween(60, 300);
  }
}

bool TradeItemInfo::adjustServerPrice(DWORD curSec)
{
  if (m_lastCalcPriceTime && curSec < (m_lastCalcPriceTime + m_dwT))
    return false;
  const SExchangeItemCFG *pBase = ItemConfig::getMe().getExchangeItemCFG(m_dwItemId);
  if (pBase == nullptr)
  {
    XERR << "[交易-服务器价格] [策划表错误] 读取不到 itemid:" << m_dwItemId << XEND;
    return false;
  }

  if (pBase->dwPriceType == PRICETYPE_SUM)
  {
    return false;
  }

  bool priceChanged = false;

  if (m_dwT == 0)
    m_dwT = randBetween(MiscConfig::getMe().getTradeCFG().dwCycleTBegin, MiscConfig::getMe().getTradeCFG().dwCycleTEnd);
  DWORD T = m_dwT;


  DWORD KT = MiscConfig::getMe().getTradeCFG().dwCycleKT;
  DWORD dwUp = MiscConfig::getMe().getTradeCFG().dwUpRate;
  DWORD dwDown = MiscConfig::getMe().getTradeCFG().dwDownRate;
  
  DWORD price0 = pBase->dwPrice;
  DWORD K = 0;      //目前挂单的商品数
  DWORD D = 0;      // 本期成交量
  DWORD oldPrice = m_dwServerPrice;
  float QK = 0.0f;

  float R = 0;        //幅  
  bool isUp = false;
  DWORD dwMinPrice = 0;
  if (m_dwServerPrice == 0)
  {
    m_dwServerPrice = price0;
  }
  else
  {
    do 
    {
      getDbPendingCount(0, false, 0, K);
      if (getDbSaledCount(curSec, T, 0, D) == false)
      {
        priceChanged = false;
        break;
      }
      if (D == 0 && K == 0)
      {
        priceChanged = false;
        break;
      }

      if (D == 0 && K > 0)
      {
        isUp = false;
        R = MiscConfig::getMe().getTradeCFG().fNoDealDropRatio;
      }
      else
      {
        float a = (float)(T) / (float)(KT);
        QK = D / a;
        if (K < QK)
        {
          isUp = true;
          R = K / QK;
          R = (1 - R) / dwUp;
          R = std::min(0.1f, R);
        }
        else
        {
          isUp = false;
          R = K / QK;
          R = (R - 1) / dwDown;
          R = std::min(0.1f, R);
        }
      }

      if (isUp)
      {
        m_dwServerPrice = std::ceil(m_dwServerPrice * (1 + R));
        //2016-12-5 去掉最高价限制
        //m_dwServerPrice = std::min(m_dwServerPrice, (DWORD)(price0 * MiscConfig::getMe().getTradeCFG().fMaxPriceRatio));
      }
      else
      {
        m_dwServerPrice = std::ceil(m_dwServerPrice * (1 - R));
        dwMinPrice = TradeManager::getMe().getMinServerPrice(pBase);
        m_dwServerPrice = std::max(m_dwServerPrice, dwMinPrice);
      }
    } while (0);    
  }
  if (oldPrice != m_dwServerPrice)
    priceChanged = true;

  XINF << "[交易-服务器价格] 服务器价格调整 物品id：" << m_dwItemId << "精炼等级：" << 0 << "周期T：" << T << "库存K" << K << "成交量D：" << D
    << "KT" << KT << "QK" << QK << "R" << R << "P0" << price0 << "old price" << oldPrice << " NEW Price" << m_dwServerPrice<<"价格是否改变"<<priceChanged<<"计算的最低服务价格"<< dwMinPrice << XEND;

  m_dwT = randBetween(MiscConfig::getMe().getTradeCFG().dwCycleTBegin, MiscConfig::getMe().getTradeCFG().dwCycleTEnd);
  m_lastCalcPriceTime = curSec;
  saveServerPrice(curSec, 0, T, K, D, QK, price0, oldPrice, m_dwServerPrice);
  return priceChanged;
}

bool TradeItemInfo::isOverlap()
{
  return m_isOverlap;  
}

bool TradeItemInfo::getDbPendingCount(DWORD refineLv, bool isDamage, DWORD upgradeLv, DWORD& count)
{
  count = 0;
  if (!m_pStoreMgr)
    return false;
  count = m_pStoreMgr->getPendingCount(refineLv, isDamage, upgradeLv);
  return true;
}

bool TradeItemInfo::getDbSaledCount(DWORD curSec, DWORD T, DWORD refineLv, DWORD& count)
{
  xField field(REGION_DB, DB_TABLE_SELLED_LIST);
  field.setValid("COALESCE(sum(count), 0) as trade_count");
  field.m_list["trade_count"] = MYSQL_TYPE_NEWDECIMAL;
  count = 0;
  char where[128];
  bzero(where, sizeof(where));

  DWORD beginTime = curSec - T;   //T 周期内
  snprintf(where, sizeof(where), "itemid=%u AND tradetime>=%u AND refine_lv=%u ", m_dwItemId, beginTime, refineLv);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(&field, set, where, NULL);

  if (QWORD_MAX == ret)
  {
    XERR << "[交易] 查询平均成交价格失败。itemid:" << m_dwItemId << XEND;
    return false;
  }
  else
  {
    count = set[0].get<DWORD>("trade_count");
    XINF << "[交易] 周期内成交量，itemid:" << m_dwItemId <<"curSec"<<curSec <<"beginTime"<<beginTime <<  "refineLv:" << refineLv<<"周期"<<T << "秒内，成交量：" << count << XEND;
  }
  return true;
}

//是否有物品在售
bool TradeItemInfo::hasPending()
{
  if (!m_pStoreMgr)
    return false;
  return m_pStoreMgr->hasPending();
}

bool TradeItemInfo::adjustSelfPrice()
{
  if (!m_pStoreMgr) return false;
  //checkExpirePending();

  return m_pStoreMgr->adjustSelfPrice();

 /* std::vector<StoreBaseMgr*> vecStorMgr;
  for (auto it = m_storeMgrMap.begin(); it != m_storeMgrMap.end(); )
  {
    if (it->second)
    {
      if (it->second->adjustSelfPrice())
      {
        vecStorMgr.push_back(it->second);
        it = m_storeMgrMap.erase(it);
        continue;
      }
    }
    ++it;
  }
  
  for (auto &v : vecStorMgr)
  {
    if (v)
    {      
      m_storeMgrMap[v->getPrice()] = v;
    }
  }*/
}

/************************************************************************/
/*PublicityInfo                                                                      */
/************************************************************************/

bool PublicityInfo::addPending(QWORD charId, std::string name, const Cmd::TradeItemBaseInfo& itemBaseInfo)
{
  if (itemBaseInfo.count() == 0)
  {
    XERR << "[交易-增加挂单] publicityid:" << id << " itemid:" << itemBaseInfo.itemid() << "price:" << itemBaseInfo.price() << "charid:" << charId << "count=0" << XEND;
    return false;
  }

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    XERR << "[交易-增加挂单]  publicityid:" << id << "itemid:" << itemBaseInfo.itemid()  << "price:" << itemBaseInfo.price() << "charid:" << charId << "数据库错误" << XEND;
    return false;
  }

  xRecord record(field);
  PendingInfo pendingInfo;
  record.put("itemid", itemBaseInfo.itemid());
  record.put("price", itemBaseInfo.price());
  record.put("count", itemBaseInfo.count());
  record.put("sellerid", charId);
  record.put("name", name);
  record.put("pendingtime", now());
  DWORD refineLv = 0;

  pendingInfo.itemId = itemBaseInfo.itemid();
  pendingInfo.name = name;
  pendingInfo.count = itemBaseInfo.count();
  pendingInfo.price = itemBaseInfo.price();
  pendingInfo.sellerId = charId;

  if (itemBaseInfo.has_item_data() && !isOverlap)
  {
    std::string strItemData;
    refineLv = itemBaseInfo.item_data().equip().refinelv();
    record.put("refine_lv", refineLv);
    itemBaseInfo.item_data().SerializeToString(&strItemData);
    record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());  
    pendingInfo.itemData = itemBaseInfo.item_data();
  }
  record.put("is_overlap", isOverlap);
  record.put("publicity_id", id);
  record.put("endtime", endTime);

  QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    //插入失败
    XERR << "[交易-公示期-增加挂单] publicityid:" << id << " itemid:" << itemBaseInfo.itemid()  << "price" << itemBaseInfo.price() << "charid:" << charId << " 插入数据库失败" << field->m_strDatabase << field->m_strTable << XEND;
    return false;
  }
  pendingInfo.orderId = ret;

  vecPending.push_back(pendingInfo);

  TradeManager::getMe().m_hotInfoMgr.addPublicity(pendingInfo.itemId, 0);

  XINF << "[交易-公示期-增加挂单] publicityid:"<< id <<" itemid:" << itemBaseInfo.itemid()  << "charid:" << charId << "新增挂单成功，" << pendingInfo.toString().c_str() << XEND;
  return true;
}

bool PublicityInfo::addBuyer(QWORD charId, DWORD count)
{  
  if (count == 0)
  {
    // XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price:" << itemBaseInfo.price() << "charid:" << charId << "count=0" << XEND;
    return false;
  }

  auto it = mapBuyer.find(charId);
  if (it == mapBuyer.end())
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY_BUY);
    if (!field)
    {
      // XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price:" << itemBaseInfo.price() << "charid:" << charId << "数据库错误" << XEND;
      return false;
    }

    xRecord record(field);
    record.put("publicity_id", id);
    record.put("buyerid", charId);
    record.put("count", count);

    QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
    if (ret == QWORD_MAX)
    {
      //插入失败
      // XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price" << itemBaseInfo.price() << "charid:" << charId << " 插入数据库失败" << field->m_strDatabase << field->m_strTable << XEND;
      return false;
    }

    BuyerInfo buyerInfo;
    buyerInfo.id = ret;
    buyerInfo.buyerId = charId;
    buyerInfo.count = count;
    mapBuyer.insert(std::make_pair(charId, buyerInfo));
    
    {
      //update buyer count
      xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY);
      if (!field)
      {
        return false;
      }
      char where[32] = { 0 };
      snprintf(where, sizeof(where), "id = %u", id);
      xRecord record(field);
      record.put("buy_people", mapBuyer.size());
      ret = thisServer->getTradeConnPool().exeUpdate(record, where);
      if (ret == QWORD_MAX)
      {
        return false;
      }
      TradeManager::getMe().m_hotInfoMgr.addPublicity(itemId, charId);
    }
  }
  else
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY_BUY);
    if (!field)
    {
      // XERR << "[交易-增加挂单] itemid:" << m_dwItemId << "price:" << itemBaseInfo.price() << "charid:" << charId << "数据库错误" << XEND;
      return false;
    }

    xRecord record(field);
    record.put("count", it->second.count + count);

    char where[32] = { 0 };
    snprintf(where, sizeof(where), "id=%llu", it->second.id);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX)
    {
      //TODO error
      return false;
    }
    it->second.count += count;
  } 

  // 您在公示期活动中抢购了[63cd4e]%s[-]个[63cd4e]%s[-]，共预先扣除[63cd4e]%s[-]Zeny。
  
  MsgParams params;
  params.addNumber(count);
  params.addString(itemName);
  params.addNumber(count * price);
  DWORD zoneId = TradeManager::getMe().getMyZoneId(charId);
  TradeManager::getMe().sendSysMsg(charId, zoneId, 10402, EMESSAGETYPE_FRAME, params);

  XINF << "[交易-公示期-购买] publicity_id" << id << "charid:" << charId << "新增购买成功，" << "count" << count << XEND;

  {
    auto it = vecPending.begin();
    if (it != vecPending.end())
    {
      PendingInfo pendingInfo = *it;
      pendingInfo.count = count;
      pendingInfo.publicityId = id;
      NameInfoList nameList;
      TradeManager::getMe().insertBuyedDbLog(EOperType_PublicityBuying, charId, pendingInfo, nameList, 0, endTime, 0);
    }   
  }

  return true;
}

void PublicityInfo::calcBuyer(DWORD needCount)
{
  if (mapBuyer.size() == 0)
    return;

  std::vector<BUYERINFO_MAP_T::iterator> buyers;
  buyers.reserve(mapBuyer.size());
  for (BUYERINFO_MAP_T::iterator it = mapBuyer.begin(); it != mapBuyer.end();++it)
  {
    buyers.push_back(it);
  }
  //随机排序一下
  random_shuffle(buyers.begin(), buyers.end());

  while (needCount)
  {
    if (buyers.size() <= needCount)
    {
      for (auto it = buyers.begin(); it != buyers.end(); )
      {
        needCount--;
        if ((*it)->second.addCount(1) == false)
        {
          it = buyers.erase(it);
          continue;
        }
        ++it;
      }
      continue;
    }

    for (auto it = buyers.begin(); it != buyers.end(); ++it)
    {
      if (needCount == 0)
        break;
      (*it)->second.addCount(1);
      needCount--;
    }    
  }
}

void PublicityInfo::calcSeller(DWORD needCount)
{
  if (vecPending.size() == 0)
    return;

  std::vector<PENDINGINFO_VEC_T::iterator> sellers;
  sellers.reserve(vecPending.size());
  for (PENDINGINFO_VEC_T::iterator it = vecPending.begin(); it != vecPending.end(); ++it)
  {    
    sellers.push_back(it);
  }

  //随机排序一下
  random_shuffle(sellers.begin(), sellers.end());

  while (needCount)
  {
    if (sellers.size() <= needCount)
    {
      for (auto it = sellers.begin(); it != sellers.end();)
      {
        needCount--;
        if ((*it)->addSellCount(1) == false)
        {
          it = sellers.erase(it);
          continue;
        }
        ++it;
      }
      continue;
    }

    for (auto it = sellers.begin(); it != sellers.end(); ++it)
    {
      if (needCount == 0)
        break;
      (*it)->addSellCount(1);
      needCount--;
    }
  }
}
//
bool PublicityInfo::outStore()
{
  Benchmarck bench("性能测试", 0, "公示期出仓");

  DWORD itemCount = getCount();
  DWORD buyCount = getAllBuyCount();
  if (itemCount == 0 || vecPending.empty())
  {
    XERR << "[交易-公示期] 异常， 出售数量为0 id" << id << uniqueid << XEND;
    return false;
  }
  
  auto fullSeller = [&]()
  {
    for (auto &v : vecPending)
    {
      v.addSellCount(v.count);
    } 
  };
  auto fullBuyer = [&]()
  {
    for (auto &v : mapBuyer)
    {
      v.second.addCount(v.second.count);
    }
  };

  if (buyCount > 0)
  {
    if (itemCount < buyCount)
    {
      calcBuyer(itemCount);
      fullSeller();
    }
    else if (itemCount == buyCount)
    {
      fullBuyer();
      fullSeller();
    }
    else
    {
      fullBuyer();
      calcSeller(buyCount);
    }
  }
  
  //del db pending 
  if (TradeManager::getMe().dbDelPending(id) == false)
    return false;
  bench.Output("删除挂单");

  //del db publicity_buy
  if (TradeManager::getMe().dbDelPublicityBuy(id) == false)
    return false;
  bench.Output("删除公示期买单");
  //del db publicity 
  if (TradeManager::getMe().dbDelPublicity(id, EOperType_Publicity) == false)
    return false;
  bench.Output("清空公示期");
  //
  std::vector<std::pair<QWORD/*charId*/, DWORD/*buycount*/>> buyer;
  buyer.reserve(mapBuyer.size());
  for (auto &v : mapBuyer)
  {
    if (v.second.trueCount)
      buyer.push_back(std::make_pair(v.first, v.second.trueCount));
    TradeManager::getMe().m_hotInfoMgr.decPublicity(itemId, v.first);
  }
  TradeManager::getMe().m_hotInfoMgr.decPublicity(itemId, 0);
  
  const SExchangeItemCFG* pExchangeCfg = ItemConfig::getMe().getExchangeItemCFG(itemId);
  if (pExchangeCfg == nullptr)
  {
    return false;
  }

  //买家 add item
  
  NameInfoList sellerNameList;
  
  xRecordSet successBuySet;
  for (auto &v : vecPending)
  {
    DWORD sellCount = v.sellCount;
    while (sellCount)
    {
      auto it = buyer.begin();
      if (it == buyer.end())
        break;
      DWORD buyCount = it->second;
      if (sellCount < buyCount)
      {
        // add item sellCount
        PendingInfo info = v;
        info.count = sellCount;

        //buyer info
        NameInfo* pNameInfo = v.buyerList.add_name_infos();
        if (pNameInfo)
        {
          pNameInfo->set_name(TradeManager::getMe().getMyName(it->first));
          DWORD buyerZoneId = TradeManager::getMe().getMyZoneId(it->first);
          pNameInfo->set_zoneid(buyerZoneId);
          pNameInfo->set_count(sellCount);
        }

        //seller info
        {
          NameInfo* pNameInfo = sellerNameList.add_name_infos();
          if (pNameInfo)
          {
            pNameInfo->set_name(v.name);
            DWORD sellerZoneId = TradeManager::getMe().getMyZoneId(v.sellerId);
            pNameInfo->set_zoneid(sellerZoneId);
            pNameInfo->set_count(sellCount);
          }
        }

        //if (pExchangeCfg->isOverlap == false)
        {
          TradeManager::getMe().addItem(EOperType_PublicityBuySuccess, it->first, 0, info);
          BuyerInfo* buyerInfo = getBuyerInfo(it->first);
          if (buyerInfo)
          {
            TradeManager::getMe().insertBuyedDbLog2(EOperType_PublicityBuySuccess, it->first, info, sellerNameList, 0, 0, buyerInfo->count, successBuySet);
            sellerNameList.Clear();
          }
        }
 
        it->second -= sellCount;
        sellCount = 0;
        break;
      }
      // add item buyCount
      PendingInfo info = v;
      info.count = buyCount;

      //seller info
      {
        NameInfo* pNameInfo = sellerNameList.add_name_infos();
        if (pNameInfo)
        {
          pNameInfo->set_name(v.name);
          DWORD sellerZoneId = TradeManager::getMe().getMyZoneId(v.sellerId);
          pNameInfo->set_zoneid(sellerZoneId);
          pNameInfo->set_count(info.count);
        }
      }

      //if (pExchangeCfg->isOverlap == false)
      {
        TradeManager::getMe().addItem(EOperType_PublicityBuySuccess, it->first, 0, info);
        BuyerInfo* buyerInfo = getBuyerInfo(it->first);
        if (buyerInfo)
        {
          TradeManager::getMe().insertBuyedDbLog2(EOperType_PublicityBuySuccess, it->first, info, sellerNameList, 0, 0, buyerInfo->count, successBuySet);
          sellerNameList.Clear();
        }
      }
      sellCount -= buyCount;

      //buerinfo
      NameInfo* pNameInfo = v.buyerList.add_name_infos();
      if (pNameInfo)
      {
        pNameInfo->set_name(TradeManager::getMe().getMyName(it->first));
        DWORD buyerZoneId = TradeManager::getMe().getMyZoneId(it->first);
        pNameInfo->set_zoneid(buyerZoneId);
        pNameInfo->set_count(buyCount);
      }

      buyer.erase(it);
    }
  }
  TradeManager::getMe().insertDbLogSet(successBuySet);
   
  bench.Output("给装备");
  xRecordSet sellSet;

  Cmd::NameInfoList tempList;
  for (auto & v : vecPending)
  {
    //money
    PendingInfo info = v;
    info.count = v.sellCount;
    if (info.count > 0)
    {
      TradeManager::getMe().addMoney(EOperType_PublicitySellSuccess, info.sellerId, 0, info);       //公示期出售成功 获得金钱 。。。
           
      TradeManager::getMe().insertSelledDbLog2(EOperType_PublicitySellSuccess, 0, info, info.buyerList, sellSet);             //公示期出售成功日志
    }    
    DWORD left = v.count - v.sellCount;
    //重新上架
    if (left)
    {    
      info.count = left;
      info.sellCount = 0;
      DWORD zoneId = TradeManager::getMe().getMyZoneId(info.sellerId);
      Cmd::ReduceItemRecordTrade rev;
      rev.set_charid(info.sellerId);
      TradeItemBaseInfo* pItemInfo = rev.mutable_item_info();
      if (pItemInfo)
      {
        info.pack2Proto(pItemInfo);        
        TradeItemInfo* pTradeItemInfo = TradeManager::getMe().getTradeItemInfo(info.itemId);
        if (pTradeItemInfo)
        {
          // NEW price
          DWORD serverPrice = TradeManager::getMe().getServerPrice(info.itemData, info.itemId);
          pItemInfo->set_price(serverPrice);
          TradeManager::getMe().trueSell(info.sellerId, info.name, zoneId, rev);
          XINF << "[交易-公示期-结束-重新上架] orderid" << info.orderId << "charid" << info.sellerId << "itemid" << info.itemId << "count" << info.count << "price" << pItemInfo->price() << "old price" << info.price << XEND;
        }
        else
        {
          TradeManager::getMe().insertSelledDbLog(EOperType_AutoOffTheShelf, 0, info, tempList);
          TradeManager::getMe().dbDelPending(info.orderId, info.sellerId, EOperType_AutoOffTheShelf);
          XINF << "[交易-公示期-结束-不可交易物品] 自动下架 orderid" << info.orderId << "charid" << info.sellerId << "itemid" << info.itemId <<"count"<<info.count<< "price" << pItemInfo->price() << "old price" << info.price << XEND;
        }
      }
    }
  }
  TradeManager::getMe().insertDbLogSet(sellSet);

  bench.Output("给钱，装备重新上架");

  PendingInfo sampleInfo = *(vecPending.begin());

  xRecordSet failBuySet;

  NameInfoList infoList;
  for (auto &v : mapBuyer)
  {
    DWORD zoneId = TradeManager::getMe().getMyZoneId(v.first);
    DWORD failCount = v.second.count - v.second.trueCount;
    DWORD consume = v.second.trueCount * price; //购买花费
    DWORD retMoney = failCount * price;              //返回金额

    sampleInfo.count = failCount;  //抢购失败个数
    //里面判断了0
    TradeManager::getMe().addMoney(EOperType_PublicityBuyFail, v.first, 0, sampleInfo, v.first ,v.second.trueCount, consume);
    if (retMoney)
      TradeManager::getMe().insertBuyedDbLog2(EOperType_PublicityBuyFail, v.first, sampleInfo, infoList, failCount, 0, v.second.count, failBuySet);

    if (consume == 0)
    {
      //抢购失败
      MsgParams params;
      params.addString(itemName);
      params.addNumber(retMoney);
      TradeManager::getMe().sendSysMsg(v.first, zoneId, 10405, EMESSAGETYPE_FRAME, params);
    }
    else
    {
      //您在公示期抢购中成功购买[63cd4e]%s[-]个[63cd4e]%s[-]，共扣除[63cd4e]%s[-]Zeny，返还[63cd4e]%s[-]Zeny。
      MsgParams params;
      params.addNumber(v.second.trueCount);
      params.addString(itemName);
      params.addNumber(consume);
      params.addNumber(retMoney);
      TradeManager::getMe().sendSysMsg(v.first, zoneId, 10401, EMESSAGETYPE_FRAME, params);
    }   
  }
  TradeManager::getMe().insertDbLogSet(failBuySet);
  bench.Output("买家 退钱");
  
  //clear
  price = 0;
  endTime = 0;
  vecPending.clear();
  mapBuyer.clear();
  return true;
}

bool PublicityInfo::updateEndTime(DWORD maintainBeginTime, DWORD maintainEndTime, DWORD curTime)
{
  if (endTime == 0)
  {
    XLOG << "[交易所-公示期延迟] 公示期时间结束,不再延时 id" << id << "curtime" << curTime << "endtime" << endTime << XEND;
    return true;
  }
  if (startTime > maintainBeginTime && startTime < maintainEndTime)
  {
    XLOG << "[交易所-公示期延迟] 维护期间进入公示期的物品，不再延时 id" << id << "curtime" << curTime <<"startTime"<< startTime<< "endtime" << endTime <<"维护开始时间"<<maintainBeginTime <<"维护结束时间"<<maintainEndTime << XEND;
    return true;
  }

  DWORD fromTime = m_dwLastStopTime;
  if (m_dwLastStopTime < maintainBeginTime)
    fromTime = maintainBeginTime;

  if (fromTime >= curTime)
    return true;

  DWORD diffTime = curTime - fromTime;
  m_dwLastStopTime = curTime;

  endTime = endTime + diffTime;

  //update db
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY);
    if (!field)
    {
      return false;
    }
    xRecord record(field);
    record.put("endtime", endTime);
    record.put("laststoptime", m_dwLastStopTime);
    char where[32] = { 0 };
    snprintf(where, sizeof(where), "id=%u", id);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX)
    {
      return false;
    }
  }
  
  //modify pending list
  QWORD ret1 = 0;
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
    if (!field)
    {
      return false;
    }
    xRecord record(field);
    record.put("endtime", endTime);
    char where[64] = { 0 };
    snprintf(where, sizeof(where), "publicity_id=%u", id);
    ret1 = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret1 == QWORD_MAX)
    {
      return false;
    }
  }
  
  //modify log
  QWORD ret2 = 0;
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
    if (!field)
    {
      return false;
    }
    xRecord record(field);
    record.put("endtime", endTime);
    char where[128] = { 0 };
    snprintf(where, sizeof(where), "publicity_id=%u and endtime >%u", id, maintainBeginTime);
    ret2 = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret2 == QWORD_MAX)
    {
      return false;
    }
  }

  XINF << "[交易所-公示期-延迟公示期时间] 成功，publiciy_id" << id << uniqueid <<"curtime"<<curTime<< "newendtime" << endTime <<"laststoptime"<<m_dwLastStopTime <<"延后"<< diffTime<<"fromtime"<<fromTime <<"maintainbegiontime"<< maintainBeginTime<<"maintainendtime"<< maintainEndTime <<"影响挂单列表条数"<< ret1<<"交易记录条数"<< ret2 << XEND;
  return true;
}

void PublicityInfo::securityCancelSell(SecurityCmd& security)
{  
  if (endTime == 0)
    return;
  
  if (vecPending.empty())
    return;

  //modify db
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    XERR << "[交易-安全指令-过期物品] 失败， 获取不到数据库， table：" << DB_TABLE_PENDING_LIST << XEND;
    return;
  }
  char where[128];
  DWORD newTime = now() - MiscConfig::getMe().getTradeCFG().dwExpireTime - 120;
  xRecord record(field);
  record.put("pendingtime", newTime);
  record.put("publicity_id", 0);
  record.put("endtime", 0);
  QWORD affectRows = 0;
  
  for (auto it = vecPending.begin(); it != vecPending.end();)
  {
    PendingInfo& pendingInfo = *it;
    if (!security.needProcess(pendingInfo.sellerId, pendingInfo.itemId, pendingInfo.refinelv()))
    {
      ++it;
      continue;
    }
       
    //update db
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id=%llu", pendingInfo.orderId);
    affectRows = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (affectRows == QWORD_MAX)
      XERR << "[交易-安全指令-公示期过期物品] 失败，update执行错误 itemid" << pendingInfo.itemId << "orderid" << pendingInfo.orderId << XEND;
    else
      XLOG << "[交易-安全指令-公示期过期物品] 成功，update执行成功 itemid" << pendingInfo.itemId << "orderid" << pendingInfo.orderId << "sellerid" << pendingInfo.sellerId << "refinelv" << pendingInfo.refinelv() << "count" << pendingInfo.count << "新的过期时间" << newTime << XEND;

    it = vecPending.erase(it); 
  }
}

void PublicityInfo::securityCancelBuy(SecurityCmd& security)
{
  if (endTime == 0)
    return;

  if (mapBuyer.empty())
    return;

  PendingInfo* pInfo = getAPendingInfo();
  if (pInfo == nullptr)
    return;

  //modify db
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY_BUY);
  if (!field)
  {
    XERR << "[交易-安全指令-公示期强制抢购失败] 失败， 获取不到数据库， table：" << DB_TABLE_PUBLICITY_BUY << XEND;
    return;
  }
  char where[128];

  for (auto it = mapBuyer.begin(); it != mapBuyer.end();)
  {
    BuyerInfo& buyerInfo = it->second;
    if (!security.needProcess(buyerInfo.buyerId, pInfo->itemId, pInfo->refinelv()))
    {
      ++it;
      continue;
    }

    //del db
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id=%llu", buyerInfo.id);
    QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
    if (ret == QWORD_MAX)
      XERR << "[交易-安全指令-公示期强制抢购失败] 失败，删除购买执行错误 itemid" << pInfo->itemId << "orderid" << pInfo->orderId << "buyerid" << buyerInfo.buyerId << "公示期id" << id << XEND;
    else
      XLOG << "[交易-安全指令-公示期强制抢购失败] 成功，删除购买执行成功 itemid" << pInfo->itemId << "orderid" << pInfo->orderId << "buyerid" << buyerInfo.buyerId << "公示期id" << id << "refinelv" << pInfo->refinelv() << "count" << buyerInfo.count << "price" << pInfo->price <<XEND;


    //删除公示期购买记录
    {
      xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
      if (!field)
        return;
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "buyerid=%llu and logtype=8 and publicity_id=%u ", buyerInfo.buyerId, id);
      QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
      if (ret == QWORD_MAX)
        XERR << "[交易-安全指令-公示期删除购买计时记录] 失败，删除购买执行错误 itemid" << pInfo->itemId << "orderid" << pInfo->orderId << "buyerid" << buyerInfo.buyerId << "公示期id"<<id << XEND;
      else
        XLOG << "[交易-安全指令-公示期删除购买计时记录] 成功，删除购买执行成功 itemid" << pInfo->itemId << "orderid" << pInfo->orderId << "buyerid" << buyerInfo.buyerId <<"公示期id" << id << "refinelv" << pInfo->refinelv() << "count" << buyerInfo.count << "price" << pInfo->price << XEND;
    }

    //插入失败交易记录    
    pInfo->count = buyerInfo.count;
    DWORD retMoney = pInfo->price * pInfo->count; 
    if (retMoney)
    {
      NameInfoList infoList;
      DWORD failCount = pInfo->count;
      DWORD totalCount = failCount;
      TradeManager::getMe().insertBuyedDbLog(EOperType_PublicityBuyFail, buyerInfo.buyerId, *pInfo, infoList, failCount, 0, totalCount);
      XLOG << "[交易-安全指令-公示期强制抢购失败] 成功，插入抢购失败数据库成功 itemid" << pInfo->itemId << "orderid" << pInfo->orderId << "buyerid" << buyerInfo.buyerId << "公示期id" << id << "refinelv" << pInfo->refinelv() << "count" << buyerInfo.count<<"price"<<pInfo->price << XEND;
    }
    it = mapBuyer.erase(it);
  }
}

/************************************************************************/
/*PublicityMgr                                                                      */
/************************************************************************/

void PublicityMgr::loadFromDb()
{
  //加载公示期
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY);
    if (field)
    {
      XINF << "[交易-公示期-加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set ,NULL, NULL);
      for (DWORD i = 0; i < ret; ++i)
      {
        const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(set[i].get<DWORD>("itemid"));
        if (pCfg == nullptr)
          continue;        

        PublicityInfo publicityInfo;
        publicityInfo.id = set[i].get<DWORD>("id");
        publicityInfo.uniqueid = set[i].getString("uniqueid");
        publicityInfo.itemId = set[i].get<DWORD>("itemid");
        publicityInfo.startTime = set[i].get<DWORD>("starttime");
        publicityInfo.endTime = set[i].get<DWORD>("endtime");
        publicityInfo.price = set[i].get<DWORD>("price");
        publicityInfo.m_dwLastStopTime = set[i].get<DWORD>("laststoptime");
        publicityInfo.itemName = pCfg->strName;
        publicityInfo.isOverlap = pCfg->isOverlap;
        
        auto it = m_mapPublicity.find(publicityInfo.uniqueid);
        if (it == m_mapPublicity.end())
        {
          m_mapPublicity.insert(std::make_pair(publicityInfo.uniqueid, publicityInfo));
        }
        else
        {
          XERR << "[交易-公示期-加载数据库] 重复的key database:" << field->m_strDatabase << "table:" << field->m_strTable << "id"<<publicityInfo.id<<"key"<<publicityInfo.uniqueid << XEND;
          continue;
        }

       /* XDBG << "[交易-加载数据库] 加载数据 orderid:" << set[i].get<QWORD>("id") << "itemid:" << set[i].get<QWORD>("itemid")
          << "price:" << set[i].get<DWORD>("price") << "count:" << set[i].get<DWORD>("count") << "selerid:" << set[i].get<QWORD>("sellerid") << "pendingtime:" << set[i].get<QWORD>("pendingtime") << XEND;
  */
      }
      XINF << "[交易-公示期-加载数据库] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
    }
    else
    {
      XERR << "[交易-公示期-加载数据库] 数据库错误" << XEND;
    }
  }

  //加载公示期挂单
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
    if (field)
    {
      XINF << "[交易-公示期-加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "publicity_id <> 0");

      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
      for (DWORD i = 0; i < ret; ++i)
      {
        PublicityInfo* pInfo = getPublicityInfo(set[i].get<DWORD>("publicity_id"));
        if (!pInfo)
        {
          XERR << "[交易-加载数据库] 数据库里的该物品已经不能交易 database:" << field->m_strDatabase << "table:" << field->m_strTable << "itemid:" << set[i].get<DWORD>("itemid") << XEND;
          continue;
        }        

        PendingInfo info;
        info.orderId = set[i].get<QWORD>("id");
        info.itemId = set[i].get<DWORD>("itemid");
        info.price = set[i].get<DWORD>("price");
        info.sellerId = set[i].get<QWORD>("sellerid");
        info.name = set[i].getString("name");
        info.count = set[i].get<DWORD>("count");
        std::string itemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
        if (!itemData.empty())
          info.itemData.ParseFromString(itemData);
        pInfo->vecPending.push_back(info);	
        m_setInPulibicy.insert(info.itemId);
        XDBG << "[交易-加载数据库] 加载数据 orderid:" << set[i].get<QWORD>("id") << "itemid:" << set[i].get<QWORD>("itemid")
          << "price:" << set[i].get<DWORD>("price") << "count:" << set[i].get<DWORD>("count") << "selerid:" << set[i].get<QWORD>("sellerid") << "pendingtime:" << set[i].get<QWORD>("pendingtime") << XEND;
      }
      XINF << "[交易-加载数据库] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
    }
    else
    {
      XERR << "[交易-加载数据库] 数据库错误" << XEND;
    }
  }

  //加载公式购买信息
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY_BUY);
    if (field)
    {
      XINF << "[交易-公示期-加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, NULL, NULL);
      for (DWORD i = 0; i < ret; ++i)
      {
        PublicityInfo* pInfo = getPublicityInfo(set[i].get<DWORD>("publicity_id"));
        if (!pInfo)
        {
          XERR << "[交易-加载数据库] 数据库里的该物品已经不能交易 database:" << field->m_strDatabase << "table:" << field->m_strTable << "itemid:" << set[i].get<DWORD>("itemid") << XEND;
          continue;
        }
        
        BuyerInfo info;
        info.id = set[i].get<QWORD>("id");
        info.buyerId = set[i].get<QWORD>("buyerid");
        info.count = set[i].get<DWORD>("count");
        pInfo->mapBuyer[info.buyerId] = info;

        TradeManager::getMe().m_hotInfoMgr.addPublicity(pInfo->itemId, info.buyerId);

      /*  XDBG << "[交易-加载数据库] 加载数据 orderid:" << set[i].get<QWORD>("id") << "itemid:" << set[i].get<QWORD>("itemid")
          << "price:" << set[i].get<DWORD>("price") << "count:" << set[i].get<DWORD>("count") << "selerid:" << set[i].get<QWORD>("sellerid") << "pendingtime:" << set[i].get<QWORD>("pendingtime") << XEND;*/
      }
      XINF << "[交易-加载数据库] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
    }
    else
    {
      XERR << "[交易-加载数据库] 数据库错误" << XEND;
    }
  }
}

PublicityInfo* PublicityMgr::getPublicityInfo(const string& key) 
{
  auto it = m_mapPublicity.find(key);
  if (it == m_mapPublicity.end())
    return nullptr;
  return &(it->second);
}

PublicityInfo* PublicityMgr::getPublicityInfo(DWORD id)
{
  for (auto it = m_mapPublicity.begin(); it != m_mapPublicity.end(); ++it)
  {
    if (it->second.id == id)
    {
      return &(it->second);
    }
  }
  return nullptr;
}

bool PublicityMgr::reqSell(QWORD charId, DWORD zoneId, Cmd::SellItemRecordTradeCmd* rev)
{
  if (!rev)
    return false;

  std::string key = TradeManager::getMe().generateKey(rev->item_info().item_data());
  PublicityInfo* pInfo = getPublicityInfo(key);
  DWORD serverPrice = 0;
  if (pInfo)
  {
    if (pInfo->isPublicity())
    {
      serverPrice = pInfo->price;
    }
    else
    {
      serverPrice = TradeManager::getMe().getServerPrice(rev->item_info().item_data());
      pInfo->price = serverPrice;
    }
  }
  else
  {
    serverPrice = TradeManager::getMe().getServerPrice(rev->item_info().item_data());
  }
  
  if (rev->item_info().price() != serverPrice)
  {

    XERR << "[交易-公示期-请求出售] 出售的价格非法, charid:" << charId << "itemid:" << rev->item_info().itemid()
      << "sell price:" << rev->item_info().price() << "server pirce:" << serverPrice << XEND;
    MsgParams params;
    TradeManager::getMe().sendSysMsg(charId, zoneId, SYS_MSG_SELL_INVALID_PRICE, EMESSAGETYPE_FRAME, params);
    return false;
  }
  //
  rev->mutable_item_info()->set_key(key);
  return true;
}

bool PublicityMgr::trueSell(QWORD charId, std::string name, DWORD zoneId, Cmd::ReduceItemRecordTrade& rev)
{
  //resell
  if (rev.is_resell())
  {
    //
    if (TradeManager::getMe().dbDelPending(rev.orderid(), charId, EOperType_Publicity) == false)
      return false;
  }

  const std::string& key = rev.item_info().key();
  PublicityInfo* pInfo = getPublicityInfo(key);

  if (pInfo)
  {
    if (!pInfo->isPublicity())
    {
      const SExchangeItemCFG* pCFG = ItemConfig::getMe().getExchangeItemCFG(pInfo->itemId);
      if (!pCFG)
        return false;

      pInfo->price = rev.item_info().price();
      pInfo->startTime = now();
      pInfo->endTime = pInfo->startTime + pCFG->dwShowTime;
      if (restart(*pInfo) == false)
      {
        //TODO xerror
        return false;
      }
    }
  }
  else
  {
    PublicityInfo info;
    info.itemId = rev.item_info().item_data().base().id();
    const SExchangeItemCFG* pCFG = ItemConfig::getMe().getExchangeItemCFG(info.itemId);
    if (!pCFG)
      return false;
    info.startTime = now();
    info.endTime = info.startTime + pCFG->dwShowTime;
    info.price = rev.item_info().price();
    info.uniqueid = key;
    if (addnew(info) == false)
    {
      //TODO xerror
      return false;
    }
    pInfo = getPublicityInfo(key);
    if (!pInfo)
      return false;
  }
  
  //add pending
  return pInfo->addPending(charId, name, rev.item_info());
}

bool PublicityMgr::reqBuy(QWORD charId, DWORD zoneId, Cmd::BuyItemRecordTradeCmd& rev)
{
  DWORD publicityId = rev.item_info().publicity_id();
  PublicityInfo* pInfo = getPublicityInfo(publicityId);
  if (!pInfo || !pInfo->isPublicity())
  {
    MsgParams params;
    TradeManager::getMe().sendSysMsg(charId, zoneId, 10404, EMESSAGETYPE_FRAME, params);
    XERR << "[交易所-公示期] 找不到公示期物品 " << "publicity_id" << publicityId << "charid:" << charId <<rev.item_info().itemid() << rev.item_info().count() << rev.item_info().price() << XEND;
    return false;
  }

  if (pInfo->price != rev.item_info().price())
  {
    MsgParams params;
    TradeManager::getMe().sendSysMsg(charId, zoneId, SYS_MSG_SELL_INVALID_PRICE, EMESSAGETYPE_FRAME, params);
    XERR << "[交易-公示期-购买], 价格不一致" << "charid" << charId << "publicity id" << publicityId << "itemid" << pInfo->itemId << "my price" << rev.item_info().price() << "server price" <<pInfo->price << XEND;
    return false;
  }
  DWORD haveBuy = pInfo->getMyBuyCount(charId);
  if (pInfo->getCount() < haveBuy + rev.item_info().count())
  {
     MsgParams params;
    TradeManager::getMe().sendSysMsg(charId, zoneId, 10403, EMESSAGETYPE_FRAME, params);
    XERR <<"[交易-公示期-购买], 超过购买上限"<<"charid"<<charId <<"publicity id"<<publicityId<<"itemid"<<pInfo->itemId <<" want buy count"<< rev.item_info().count() <<"have buy count"<< haveBuy <<"库存"<<pInfo->getCount()<< "price"<<rev.item_info().price()  << XEND;
    return false;
  }  
  return true;
}

bool PublicityMgr::trueBuy(QWORD charId, DWORD zoneId, Cmd::ReduceMoneyRecordTradeCmd& rev)
{
  if (rev.ret() != ETRADE_RET_CODE_SUCCESS)
  {
    return false;
  }

  DWORD publicityId = rev.item_info().publicity_id();
  PublicityInfo* pInfo = getPublicityInfo(publicityId);
  if (!pInfo || !pInfo->isPublicity())
  {
    XERR << "[交易所-公示期] 物品已经不在公示期, 退款"<<"publicity_id"<<publicityId <<"charid:"<<charId<< "return money"<<rev.total_money() <<rev.item_info().itemid() <<rev.item_info().count()<<rev.item_info().price() << XEND;
    PendingInfo info;
    info.itemId = rev.item_info().itemid();
    info.price = rev.item_info().price();
    info.count = rev.item_info().count();
    DWORD retMoney = info.price*info.count;
    TradeManager::getMe().addMoney(EOperType_PublicityBuyFail, charId, zoneId, info);
    info.publicityId = publicityId;
    if (retMoney)
    {
      NameInfoList infoList;
      DWORD failCount = info.count;
      DWORD totalCount = failCount;
      TradeManager::getMe().insertBuyedDbLog(EOperType_PublicityBuyFail, charId, info, infoList, failCount, 0, totalCount);
    }

    //您在公示期抢购中成功购买[63cd4e]%s[-]个[63cd4e]%s[-]，共扣除[63cd4e]%s[-]Zeny，返还[63cd4e]%s[-]Zeny。
    const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(info.itemId);
    if (pCfg == nullptr)
      return false;
    MsgParams params;
    params.addNumber(0);
    params.addString(pCfg->strName);
    params.addNumber(0);
    params.addNumber(retMoney);
    TradeManager::getMe().sendSysMsg(charId, zoneId, 10401, EMESSAGETYPE_FRAME, params);

    return false;
  }

  //公示期购买扣押
  ETRADE_TYPE tradeType = ETRADETYPE_PUBLICITY_SEIZURE;
  string jsonStr = pInfo->uniqueid;
  PlatLogManager::getMe().TradeLog(thisServer,
    thisServer->getPlatformID(),
    rev.charid(),
    tradeType,
    rev.item_info().itemid(),
    rev.item_info().count(),
    rev.item_info().price(),
    0,
    rev.item_info().count() * rev.item_info().price(),
    jsonStr);
  
  return pInfo->addBuyer(charId, rev.item_info().count());
}

bool PublicityMgr::addnew(PublicityInfo& info)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY);
  if (!field)
  {
    XERR << "[交易所-公示期-新建] 失败，publiciy_id" << info.id << info.uniqueid << "endtime" << info.endTime << XEND;
    return false;
  }
  xRecord record(field);
  record.putString("uniqueid", info.uniqueid);
  record.put("itemid", info.itemId);
  record.put("starttime", info.startTime);
  record.put("endtime", info.endTime);
  record.put("price", info.price);
  record.put("buy_people", 0);

  QWORD id = thisServer->getTradeConnPool().exeInsert(record, true);
  if (id == QWORD_MAX)
  {
    XERR << "[交易所-公示期-新建] 失败，publiciy_id" << info.id << info.uniqueid << "endtime" << info.endTime << XEND;
    return false;
  }
  info.id = (DWORD) id;

  const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(info.itemId);
  if (pCfg == nullptr)
    return false;
  info.itemName = pCfg->strName;
  info.isOverlap = pCfg->isOverlap;
  notify(info);
  m_mapPublicity[info.uniqueid] = info;
  XINF << "[交易所-公示期-新建] 成功，publiciy_id" << info.id << info.uniqueid << "endtime" << info.endTime << XEND;
  
  m_setInPulibicy.insert(info.itemId);
  return true;
}

bool PublicityMgr::restart(PublicityInfo& info)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY);
  if (!field)
  {
    return false;
  }
  xRecord record(field);
  record.put("starttime", info.startTime);
  record.put("endtime", info.endTime);
  record.put("price", info.price);
  record.put("buy_people", 0);
  char where[32] = { 0 };
  snprintf(where ,sizeof(where), "id=%u", info.id);
  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    return false;
  }
  notify(info);
  XINF << "[交易所-公示期-重启] 成功，publiciy_id" << info.id << info.uniqueid <<"endtime"<<info.endTime << XEND;
  m_setInPulibicy.insert(info.itemId);
  return true;
}

bool PublicityMgr::stop(PublicityInfo& info)
{
  m_setInPulibicy.erase(info.itemId);
  return info.outStore();
}

void PublicityMgr::notify(const PublicityInfo& info)
{
  return;

  for (auto &v : m_mapPublicity)
  {
    if (v.first == info.uniqueid)
      continue;
    
    if (info.itemId == v.second.itemId && v.second.isPublicity())
    {
      return;
    }
  }

  //  10400 [63cd4e] % s[-]进入了公示期，冒险者们快来抢购！
  MsgParams params;
  params.addString(info.itemName);
  TradeManager::getMe().sendWorldMsg(10400, EMESSAGETYPE_FRAME, params);
}

void PublicityMgr::timeTick(DWORD curSec)
{
  //5 min check
  checkMaintain(curSec); //一定要在stop

  for (auto it = m_mapPublicity.begin(); it != m_mapPublicity.end(); ++it)
  {
    if (it->second.endTime && curSec >= it->second.endTime)
    {
      stop(it->second);
    }
  }
}

void PublicityMgr::checkMaintain(DWORD curSec)
{
  if (m_dwNextCheckMaintain && curSec < m_dwNextCheckMaintain)
    return;
    
  DWORD beginTime = 0;
  DWORD endTime = 0; 
  getMaintaintime(beginTime, endTime);
  XLOG << "[交易所-公示期延迟-获取停服维护时间] curtime" << curSec << "begintime" << beginTime << "endtime" << endTime << XEND;
  m_dwNextCheckMaintain = curSec + 5 * 60;
  if (curSec < beginTime || curSec > endTime)
  {
    return;
  }

  for (auto it = m_mapPublicity.begin(); it != m_mapPublicity.end(); ++it)
  {
    it->second.updateEndTime(beginTime, endTime, curSec);
  }
}

void PublicityMgr::getMaintaintime(DWORD& beginTime, DWORD& endTime)
{
  xLuaData oData;
  std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_REGION_INFO, "data");
  if (RedisManager::getMe().getHashAll(key, oData))
  {
    std::string msStr = oData.getTableString("maintainstart");
    if (msStr != "")
    {
      parseTime(msStr.c_str(), beginTime);
    }
    std::string meStr = oData.getTableString("maintainend");
    if (meStr != "")
    {
      parseTime(meStr.c_str(), endTime);
    }      
    return ;
  }

  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "region");  // 先查redis
  if (field)
  {
    field->setValid("maintainstart,maintainend");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "regionid=%u", thisServer->getRegionID());

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX != ret && ret)
    {
      std::string msStr = set[0].getString("maintainstart");
      if (msStr != "")
      {
        parseTime(msStr.c_str(), beginTime);
      }
      std::string meStr = set[0].getString("maintainend");
      if (meStr != "")
      {
        parseTime(meStr.c_str(), endTime);
      }
    }
  }
}

bool PublicityMgr::isPublicity(DWORD itemId)
{
  auto it = m_setInPulibicy.find(itemId);
  return it != m_setInPulibicy.end();
}

void PublicityMgr::securityCancelSell(SecurityCmd& security)
{
  for (auto it = m_mapPublicity.begin(); it != m_mapPublicity.end(); ++it)
  {
    if (!it->second.isPublicity())
      continue;

    it->second.securityCancelSell(security);
  }
  return ;
}

void PublicityMgr::securityCancelBuy(SecurityCmd& security)
{
  for (auto it = m_mapPublicity.begin(); it != m_mapPublicity.end(); ++it)
  {
    if (!it->second.isPublicity())
      continue;

    it->second.securityCancelBuy(security);
  }
}

/************************************************************************/
/*SecurityCmd                                                                      */
/************************************************************************/

void SecurityCmd::init(const Cmd::SecurityCmdSceneTradeCmd& rev)
{
  m_type = rev.type();
  m_qwCharid = rev.charid();
  m_dwItemId = rev.itemid();
  m_refineLv = rev.refinelv();
  m_dwTime = now();
  m_strKey = TradeManager::getMe().getSecuiryKey(rev);
}

bool SecurityCmd::fromDb(const xRecord& rRecord)
{
  m_qwId = rRecord.get<QWORD>("id");
  m_qwCharid = rRecord.get<QWORD>("charid");
  m_type = static_cast<ESecurityType>(rRecord.get<DWORD>("type"));
  m_dwItemId = rRecord.get<DWORD>("itemid");
  m_refineLv = rRecord.get<int>("refinelv");
  m_dwTime = rRecord.get<DWORD>("time");
  m_strKey = rRecord.getString("uniquekey");

  XLOG << "[交易-安全指令] 从数据库加载 id" << m_qwId << "key" << m_strKey << "charid" << m_qwCharid << "type" << m_type << "itemid" << m_dwItemId << "refinelv" << m_refineLv << XEND;
  return true;
}

bool SecurityCmd::toDb()
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_SECURITY);
  if (!field)
  {
    return false;
  }
  xRecord record(field);
  record.put("charid", m_qwCharid);
  record.put("type", m_type);
  record.put("itemid", m_dwItemId);
  record.put("refinelv", m_refineLv); 
  record.put("time", m_dwTime);
  record.putString("uniquekey", m_strKey);
  QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    return false;
  }
  
  m_qwId = ret;
  
  XLOG << "[交易-安全指令] 插入数据库成功 id"<< m_qwId << "key" << m_strKey <<"charid"<<m_qwCharid<<"type"<<m_type<<"itemid"<<m_dwItemId<<"refinelv"<<m_refineLv<<XEND;
  return true;
}

bool SecurityCmd::delDb()
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_SECURITY);
  if (!field)
  {
    return false;
  }  
  char deleWhere[64];
  bzero(deleWhere, sizeof(deleWhere));
  snprintf(deleWhere, sizeof(deleWhere), "id=%llu", m_qwId);
  QWORD ret = thisServer->getTradeConnPool().exeDelete(field, deleWhere);
  if (ret == QWORD_MAX)
    return false;
  
  XLOG << "[交易-安全指令] 删除数据库成功 id" << m_qwId << "key" << m_strKey << "charid" << m_qwCharid << "type" << m_type << "itemid" << m_dwItemId << "refinelv" << m_refineLv << XEND;
  return true;
}

bool SecurityCmd::needProcess(QWORD charId, DWORD itemId, DWORD refineLv)
{
  if (m_qwCharid && m_qwCharid != charId)
    return false;  
  if (m_dwItemId != itemId)
    return false;
  int rLv = refineLv;
  if (m_refineLv != -1 && m_refineLv != rLv)
    return false;
  return true;
}

/************************************************************************/
/*      TradeManager                                                                */
/************************************************************************/
TradeManager::TradeManager(): m_oOneSecTimer(1), m_oFivSecTimer(5), m_oOneMinTimer(60), m_oOneHourTimer(60 * 60), m_oTenMinTimer(10*60), m_oOneDayTimer(5, 0)
{
}

TradeManager::~TradeManager()
{
  for (auto it = m_itemInfoMap.begin(); it != m_itemInfoMap.end();)
  {
    if (it->second)
    {
      SAFE_DELETE(it->second);
      it = m_itemInfoMap.erase(it);
    }
    else {
      ++it;
    }
  }
  m_itemInfoMap.clear();
}

void TradeManager::init()
{ 
#ifndef _OLD_TRADE
  return;   //使用新的交易所 立即退出
#endif // _OLD_TRADE

  loadConfig();
  load();
}

bool TradeManager::loadConfig()
{
  auto& exchangeItemMap = ItemConfig::getMe().getExchangeItemCFGMap();
  for (auto it = exchangeItemMap.begin(); it != exchangeItemMap.end(); ++it)
  {
    SExchangeItemCFG exchangeItemCfg = it->second;
    XINF << "[交易-加载策划表]" << exchangeItemCfg.dwId << "bTrade:"<<exchangeItemCfg.bTrade << XEND;
    if (!exchangeItemCfg.bTrade) //不可交易的不加载进内存了
      continue;
    
    const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(exchangeItemCfg.dwId);
    if (pItemCfg == nullptr)
    {
      XERR << "[交易-加载策划表], 策划表找不到物品" << exchangeItemCfg.dwId << XEND;
      continue;
    }
    TradeItemInfo* pTradeItemInfo = NEW TradeItemInfo(exchangeItemCfg.dwId, exchangeItemCfg.isOverlap);
    if (pTradeItemInfo == nullptr)
      continue;
    m_itemInfoMap[exchangeItemCfg.dwId] = pTradeItemInfo;
    XINF << "[交易-加载策划表], 加载策划表成功" << exchangeItemCfg.dwId << XEND;
  }

  return true;
}

bool TradeManager::load()
{

  //加载时间等信息
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_TRADE_INFO);
    if (field)
    {
      XINF << "[交易-加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
      xRecordSet set;

      //char where[20] = "refinelv = 0";
      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, NULL, NULL);
      for (DWORD i = 0; i < ret; ++i)
      {
        auto it = m_itemInfoMap.find(set[i].get<DWORD>("itemid"));
        if (it == m_itemInfoMap.end())
        {
          XERR << "[交易-加载数据库] 数据库里的该物品已经不能交易 database:" << field->m_strDatabase << "table:" << field->m_strTable << "itemid:" << set[i].get<DWORD>("itemid") << XEND;
          continue;
        }
        TradeItemInfo* pTradeItemInfo = it->second;
        if (pTradeItemInfo == nullptr)
        {
          XERR << "[交易-加载数据库] 数据库里的该物品已经不能交易 database:" << field->m_strDatabase << "table:" << field->m_strTable << "itemid:" << set[i].get<DWORD>("itemid") << XEND;
          continue;
        }      

        pTradeItemInfo->init(set[i].get<DWORD>("last_server_price"), 
          set[i].get<DWORD>("last_calc_price_time"),
          set[i].get<DWORD>("t"));
        XDBG << "[交易-加载数据库] 加载数据 itemid:" << set[i].get<DWORD>("itemid") 
          << "t:" << set[i].get<DWORD>("t")
          << "last_server_price:"<< set[i].get<DWORD>("last_server_price")
          << "last_calc_price_time:" << set[i].get<DWORD>("last_calc_price_time") << XEND;
      }
      XINF << "[交易-加载数据库] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
    }
    else
    {
      XERR << "[交易-加载数据库] 数据库错误" << XEND;
    }
  }

  //
  m_oPublicityMgr.loadFromDb();

  //加载玩家挂单信息
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
    if (field)
    {
      XINF << "[交易-加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;

      DWORD beginTime = now() - MiscConfig::getMe().getTradeCFG().dwExpireTime;   //48小时内的
      char where[128];      
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "publicity_id = 0");

      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);

      Cmd::NameInfoList tempList;
      for (DWORD i = 0; i < ret; ++i)
      {
        PendingInfo info;
        info.orderId = set[i].get<QWORD>("id");
        info.itemId = set[i].get<DWORD>("itemid");
        info.price = set[i].get<DWORD>("price");
        info.sellerId = set[i].get<QWORD>("sellerid");
        info.name = set[i].getString("name");
        info.count = set[i].get<DWORD>("count");
        info.pendingTime = set[i].get<QWORD>("pendingtime");
        info.isOverlap = set[i].get<DWORD>("is_overlap");

        info.refineLv = set[i].get<DWORD>("refine_lv");
        
	      std::string itemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
        if (!itemData.empty())
		      info.itemData.ParseFromString(itemData);

        auto it = m_itemInfoMap.find(set[i].get<DWORD>("itemid"));
        TradeItemInfo*pTradeItemInfo = nullptr;
        if (it != m_itemInfoMap.end())
          pTradeItemInfo = it->second;
        if (pTradeItemInfo == nullptr)
        {
          insertSelledDbLog(EOperType_AutoOffTheShelf, 0, info, tempList);
          dbDelPending(info.orderId, info.sellerId, EOperType_AutoOffTheShelf);
          
          XINF << "[交易-加载数据库] 数据库里的该物品已经不能交易,自动下架 "<< "itemid:" << info.itemId <<"orderid"<<info.orderId <<"sellerId"<<info.sellerId <<
            "count"<<info.count << XEND;
          continue;
        }
        if (info.pendingTime <= beginTime)
          continue;

        pTradeItemInfo->loadPlayerPendingInfo(info);

        XDBG << "[交易-加载数据库] 加载数据 orderid:" << set[i].get<QWORD>("id") << "itemid:" << set[i].get<QWORD>("itemid")
          << "price:" << set[i].get<DWORD>("price") << "count:" << set[i].get<DWORD>("count") << "selerid:" << set[i].get<QWORD>("sellerid") << "pendingtime:" << set[i].get<QWORD>("pendingtime") << XEND;
      }
      XINF << "[交易-加载数据库] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
    }
    else
    {
      XERR << "[交易-加载数据库] 数据库错误" << XEND;
    }
  }

  //加载购买记录
  {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
    if (field)
    {
      XINF << "[交易-加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;

      //time_t beginTime = now() - MiscConfig::getMe().getTradeCFG().dwHotTime;   //48小时内的
      //char where[128];
      //bzero(where, sizeof(where));
      //snprintf(where, sizeof(where), "tradetime >= %lu ", beginTime);

      xRecordSet set;
      QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, NULL, NULL);
      for (DWORD i = 0; i < ret; ++i)
      {
        DWORD dwItemid = set[i].get<DWORD>("itemid");
        DWORD dwCount = set[i].get<DWORD>("count");
        EOperType type = EOperType(set[i].get<DWORD>("logtype"));
        DWORD tradeTime = set[i].get<DWORD>("tradetime");
        DWORD offset = now() - MiscConfig::getMe().getTradeCFG().dwLogTime;
        if (type != EOperType_PublicityBuying && tradeTime >= offset)
          m_hotInfoMgr.addItem(dwItemid, dwCount, tradeTime);
      }
      XINF << "[交易-加载数据库] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
    }
    else
    {
      XERR << "[交易-加载数据库] 数据库错误" << XEND;
    }
  }
  return true;
}

void TradeManager::checkDbItem()
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    return;
  }

  XINF << "[交易-检查数据库物品] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;

  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, NULL, NULL);
  for (DWORD i = 0; i < ret; ++i)
  {
    const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(set[i].get<DWORD>("itemid"));
    if (!pCfg)
    {
      XERR << "[交易-检查数据库物品] 数据库里的该物品已经不能交易 database:" << field->m_strDatabase << "table:" << field->m_strTable << "itemid:" << set[i].get<DWORD>("itemid") << XEND;

      PendingInfo info;
      info.orderId = set[i].get<QWORD>("id");
      info.itemId = set[i].get<DWORD>("itemid");
      info.price = set[i].get<DWORD>("price");
      info.sellerId = set[i].get<QWORD>("sellerid");
      info.name = set[i].getString("name");
      info.count = set[i].get<DWORD>("count");
      info.pendingTime = set[i].get<QWORD>("pendingtime");
      info.isOverlap = set[i].get<DWORD>("is_overlap");
      info.refineLv = set[i].get<DWORD>("refine_lv");
      std::string itemData((const char *)set[i].getBin("itemdata"), set[i].getBinSize("itemdata"));
      if (!itemData.empty())
        info.itemData.ParseFromString(itemData);

      addItem(EOperType_NormalSell, info.sellerId, 0, info, true);
      dbDelPending(info.orderId, info.sellerId, EOperType_NoramlBuy);
    }
  }
  XINF << "[交易-检查数据库物品] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
}

bool TradeManager::doTradeServerCmd(QWORD charId, std::string name, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;//::Record;
  xCommand* cmd = (xCommand*)buf;
  MessageStatHelper msgStat(cmd->cmd, cmd->param);
  switch (cmd->param)
  {
  case REDUCE_MONEY_RECORDTRADE:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReduceMoneyRecordTradeCmd, rev);
    XINF << "[交易-购买] scene扣钱返回， " << rev.ShortDebugString() << XEND;
    //rev.Utf8DebugString().c_str()

    ETRADE_RET_CODE ret = ETRADE_RET_CODE_FAIL;
    if (trueBuy(charId, name, zoneId, rev))
      ret = ETRADE_RET_CODE_SUCCESS;
    else
      ret = ETRADE_RET_CODE_FAIL;
    Cmd::BuyItemRecordTradeCmd sendCmd;
    sendCmd.mutable_item_info()->CopyFrom(rev.item_info());
    sendCmd.set_ret(ret);
    PROTOBUF(sendCmd, send, len);
    sendCmdToMe(charId, zoneId, (BYTE*)send, len);
    return true;
  }

  case REDUCE_ITEM_RECORDTRADE:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReduceItemRecordTrade, rev);

    XINF << "[交易-出售] scene返回 " << rev.ShortDebugString() << XEND;

    if (rev.is_resell())
    {
      resellUnlock(rev.orderid());
    }

    if (rev.ret() == ETRADE_RET_CODE_SUCCESS)
    {//扣除物品成功
      if (trueSell(charId, name, zoneId, rev))
      {
        Cmd::SellItemRecordTradeCmd sendCmd;
        sendCmd.mutable_item_info()->CopyFrom(rev.item_info());
        sendCmd.set_ret(ETRADE_RET_CODE_SUCCESS);
        PROTOBUF(sendCmd, send, len);
        sendCmdToMe(charId, zoneId, (BYTE*)send, len);
        
        if (rev.is_resell())
        { //商人返回部分上架费
          DWORD percent = getReturnRate(rev.charid());
          if (percent)      //总价的千分比
          {
            QWORD retMoney = rev.boothfee() * percent / 1000;
            if (retMoney)
            {
              //返还部分费用
              AddItemRecordTradeCmd sendCmd;
              TradeItemBaseInfo* pBaseInfo = sendCmd.mutable_item_info();
              if (pBaseInfo)
              {
                pBaseInfo->set_itemid(100);
                pBaseInfo->set_count(retMoney);
                sendCmd.set_charid(rev.charid());
                sendCmd.set_addtype(EADDITEMTYP_RETURN);
                PROTOBUF(sendCmd, send, len);
                if (thisServer->sendCmdToZone(zoneId, send, len) == false)
                {
                  MailManager::getMe().addOfflineTradeItem(sendCmd);
                }
                XLOG << "[交易所-重新上架返还费用] charid" << rev.charid() << zoneId << "boothfee" << rev.boothfee() << "percent" << percent << "retmoney" << retMoney << XEND;
              }
            }
          }
        }

        //上架
        ETRADE_TYPE tradeType= ETRADETYPE_SELL;

        if (rev.item_info().has_key() && !rev.item_info().key().empty())
        {
          if (rev.is_resell())
            tradeType = ETRADETYPE_RESELL_AUTO;
          else
            tradeType = ETRADETYPE_PUBLICITY_SELL;
        }
        else
        {
          if (rev.is_resell())
            tradeType = ETRADETYPE_RESELL;
          else
            tradeType = ETRADETYPE_SELL;
        }

        string jsonStr = pb2json(rev.item_info().item_data());
        PlatLogManager::getMe().TradeLog(thisServer,
          thisServer->getPlatformID(),
          rev.charid(),
          tradeType,
          rev.item_info().itemid(),
          rev.item_info().count(),
          rev.item_info().price(),
          rev.boothfee(),
          0,
          jsonStr);
      }
    }
    return true;
  }
  case EXTRA_PERMISSION_RECORDTRADE:
  {
    PARSE_CMD_PROTOBUF(Cmd::ExtraPermissionSceneTradeCmd, rev);
    
    TradeUser* pUser = TradeUserMgr::getMe().getTradeUser(rev.charid());
    if (pUser == nullptr)
    {
      return false;
    }
    pUser->setPermission(rev.permission(), rev.value());
    XLOG << "[交易所-设置额外权限成功] charid" << rev.charid() << "per" << rev.permission() << "value" << rev.value() << XEND;
    return true;
  }
  default:
    break;
  }
  return false;
}

bool TradeManager::doTradeUserCmd(QWORD charId, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;//::Record;
  xCommand* cmd = (xCommand*)buf;
  MessageStatHelper msgStat(cmd->cmd, cmd->param);
  switch (cmd->param)
  {
    case DETAIL_PENDING_LIST_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::DetailPendingListRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
          return false;

        fetchDetailPendingList(charId, zoneId, rev);

        return true;
      }
      break;
    case BRIEF_PENDING_LIST_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::BriefPendingListRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
          return false;

        fetchAllBriefPendingList(charId, zoneId, rev);

        return true;
      }
      break;
    case HOT_ITEMID_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::HotItemidRecordTrade, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
        {
          return false;
        }

        fetchHotPendingList(charId, zoneId, rev);
        return true;
      }
      break;
    case Cmd::REQ_SERVER_PRICE_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::ReqServerPriceRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
      /*  if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
        {
          return false;
        }*/
        DWORD itemId = rev.itemdata().base().id();

        TradeItemInfo* pTradeItemInfo = getTradeItemInfo(itemId);
        if (pTradeItemInfo == nullptr)
        {
          //异常
          XERR << "[交易-请求服务器价格] 失败，获取不到TradeItemInfo charid:" << charId << "itemid：" << itemId << XEND;
          MsgParams params;
          if (!thisServer->isOuter())
            sendSysMsg(charId, zoneId, SYS_MSG_CFG_ERROR, EMESSAGETYPE_FRAME, params);
          return true;
        }
        else
        {             
          getItemSellInfo(rev);
          if (rev.price() == 0)
          {
            MsgParams params;
            sendSysMsg(charId, zoneId, SYS_MSG_CFG_ERROR, EMESSAGETYPE_FRAME, params);
            return true;
          }

          XLOG << "[交易-请求服务器价格]" << rev.charid() << zoneId << itemId 
            <<"issell" << rev.issell() 
            <<"price:"<< rev.price()
            <<"交易所状态"<< rev.statetype()
            <<"交易所数量"<<rev.count()
            <<"购买人数"<< rev.buyer_count() 
            <<"公示期结束时间"<<rev.end_time()
            <<"itemdata:" << rev.itemdata().ShortDebugString() << XEND;
        }
        PROTOBUF(rev, send, len);
        sendCmdToMe(charId, zoneId, (BYTE*)send, len);

        return true;
      }
      break;
    case BUY_ITEM_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::BuyItemRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
        {
          return false;
        }

        XINF << "[交易-请求购买] charid:" << charId << "orderid:" << rev.item_info().order_id() << "itemid:" << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;

        bool ret = reqBuy(charId, zoneId, rev);
        if (ret)
        {
          //通知scene扣钱
          const SExchangeItemCFG *pBase = ItemConfig::getMe().getExchangeItemCFG(rev.item_info().itemid());
          if (pBase == nullptr)
          {
            XERR << "[交易-请求购买] 策划表错误，charid:" << charId << "itemid:" << rev.item_info().itemid() << XEND;
            return false;
          }
          Cmd::ReduceMoneyRecordTradeCmd sendCmd;
          sendCmd.mutable_item_info()->CopyFrom(rev.item_info());
          sendCmd.set_money_type(pBase->dwMoneyType);
          DWORD totalMoney = rev.item_info().count() * rev.item_info().price();
          sendCmd.set_total_money(totalMoney);
          sendCmd.set_charid(charId);
          PROTOBUF(sendCmd, send, len);
          thisServer->sendCmdToZone(zoneId, send, len);
          XINF << "[交易-请求购买] Record验证成功，通知scene 扣钱，charid:" << charId << "orderid:" << rev.item_info().order_id()
            << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
        }
        else
        {
          rev.set_ret(ETRADE_RET_CODE_FAIL);
          PROTOBUF(rev, send, len);
          sendCmdToMe(charId, zoneId, (BYTE*)send, len);
        }

        return true;
      }
      break;
    case SELL_ITEM_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::SellItemRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        XINF << "[交易-请求出售] charid:" << rev.charid() << "itemid:" << rev.item_info().itemid()
          << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << "refine_lv:" << rev.item_info().refine_lv() << XEND;

        if (!TradeMsgGuard::getMe().lock(rev.param(), rev.charid()))
        {
          return false;
        }
        bool ret = reqSell(charId, zoneId, &rev);
        if (ret)
        {
          //通知scene 扣除装备   --scene need check the refine lv
          Cmd::ReduceItemRecordTrade sendCmd;
          sendCmd.mutable_item_info()->CopyFrom(rev.item_info());
          sendCmd.set_charid(rev.charid());
          //     sendCmd.set_boothfee(pBase->getBoothfee() * sendCmd.item_info().count());  //上架费单价 银币
          //       
          sendCmd.set_boothfee(calcBoothfee(sendCmd.item_info().price(),sendCmd.item_info().count()));
          PROTOBUF(sendCmd, send, len);
          thisServer->sendCmdToZone(zoneId, send, len);
          XINF << "[交易-请求出售] 出售，通知scene 扣除装备 charid:" << sendCmd.charid() << "上架费:"
            << sendCmd.boothfee() << "itemid:" << sendCmd.item_info().itemid() << "count:" << sendCmd.item_info().count() << "price:" << sendCmd.item_info().price() << XEND;
        }
        return true;
      }
      break;
    case RESELL_PENDING_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::ResellPendingRecordTrade, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), rev.charid()))
        {
          return false;
        }
        
        //lock
        if (resellLock(rev.order_id()) == false)
        {
          return true;
        }
        XINF << "[交易-重新上架] 收到消息: charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
        if (resell(charId, zoneId, rev) == false)
        {
          resellUnlock(rev.order_id());
        }

        return true;
      }
      break;
    case CANCEL_PENDING_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::CancelItemRecordTrade, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), rev.charid()))
        {
          return false;
        }
        if (resellLock(rev.order_id()) == false)
        {
          return false;
        }

        XINF << "[交易-取消订单] 收到消息: charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
        bool ret = cancelPending(charId, zoneId, rev);
        if (ret)
        {
          rev.set_ret(ETRADE_RET_CODE_SUCCESS);
          PROTOBUF(rev, send, len);
          sendCmdToMe(charId, zoneId, (BYTE*)send, len);
        }
        resellUnlock(rev.order_id());
        return true;
      }
      break;
    case PANEL_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::PanelRecordTrade, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
        {
          return false;
        }
        XINF << "[交易-面板操作] 收到消息: charid:" << charId << "msg:" << rev.ShortDebugString() << XEND;
        if (rev.oper() == EPANEL_OPEN)
        {
          openPanel(charId, zoneId);
        }
        else if (rev.oper() == EPANEL_CLOSE)
        {
          closePanel(charId);
        }
        return true;
      }
      break;
    case ITEM_SELL_INFO_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::ItemSellInfoRecordTradeCmd, rev);
        if (checkCharId(charId, rev.charid()) == false)
          return false;
        if (!TradeMsgGuard::getMe().lock(rev.param(), charId))
        {
          return false;
        }

        fetchItemSellInfoList(charId, zoneId, rev);
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

void TradeManager::fetchAllBriefPendingList(QWORD charid, DWORD zoneId, Cmd::BriefPendingListRecordTradeCmd& rev)
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
      sendCmdToMe(charid, zoneId, (BYTE*)send, len);
      XDBG << "[交易-挂单总览] 缓存 charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
      return;
    }
  }

  do
  {
    auto it = m_itemJobCache.find(key);
    if (it == m_itemJobCache.end())
    {
      TVecDWORD tempItemList = ItemConfig::getMe().getExchangeItemIds(rev.category(), rev.job(), rev.fashion());
      it = m_itemJobCache.insert(make_pair(key, tempItemList)).first;
    }  
    TVecDWORD& itemList = it->second;   
    if (itemList.empty()) 
    {
      XERR << "[交易-挂单总览] fetchAllBriefPendingList，筛选不到相应的物品id,charid:" << rev.charid() <<"category:"<<rev.category()<<"job:"<<rev.job() << XEND;
      break;
    }
    
    for (auto &v : itemList)
    {
      //publicity
      if (m_oPublicityMgr.isPublicity(v))
      {
        rev.add_pub_lists(v);
        continue;
      }

      //normal 
      TradeItemInfo *pNormalTrade = getTradeItemInfo(v);
      if (pNormalTrade && pNormalTrade->hasPending())
      {
          rev.add_lists(v);
      }
    }
  } while (0);

  //insert cache
  {
    DWORD  CACHE_TIME = 50;
    BriefListCache cache;
    cache.expireTime = now() + CACHE_TIME;
    cache.cmd = rev;
    m_mapBriefListCache[key] = cache;
  }

  //出错也要给客户端返回
  PROTOBUF(rev, send, len);
  sendCmdToMe(charid,  zoneId, (BYTE*)send, len);
  XDBG << "[交易-挂单总览] charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
}

//获取物品每个价格的详细挂单列表
void TradeManager::fetchDetailPendingList(QWORD charId, DWORD zoneId, Cmd::DetailPendingListRecordTradeCmd& rev)
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
    snprintf(orderBy, sizeof(orderBy), " order by publicity_id desc , price");
    break;
  case RANKTYPE_ITEM_PRICE_DES:
    snprintf(orderBy, sizeof(orderBy), " order by publicity_id desc , price desc");
    break;
  case RANKTYPE_REFINE_LV_INC:
    snprintf(orderBy, sizeof(orderBy), " order by publicity_id desc , refine_lv");
    break;
  case RANKTYPE_REFINE_LV_DES:
    snprintf(orderBy, sizeof(orderBy), " order by publicity_id desc , refine_lv desc");
    break;
  default:
    snprintf(orderBy, sizeof(orderBy), " order by publicity_id desc , temid");
    break;
  }

  time_t currTime = now();
  time_t shelfTime = currTime - MiscConfig::getMe().getTradeCFG().dwExpireTime;

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
    snprintf(where, sizeof(where), " where (`is_overlap` = 1) AND itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) OR (publicity_id <> 0))  group by `price`",
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
      " select `id`,`itemid`,`price`,`count` AS `total_count`,`refine_lv`,`is_overlap` from %s where (`is_overlap` = 0) AND itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) OR (publicity_id <> 0))",
      table,
      itemId,
      shelfTime);
  }
  std::string sql(totalCountSql);
  xRecordSet tmpSet;
  QWORD ret = thisServer->getTradeConnPool().exeRawSelect(&field, tmpSet, sql);
  if ((QWORD)-1 == ret)
  {
    XERR << "[交易-详细挂单] fetchDetailPendingList， 查询数据库出错， charid:" << rev.charid() << "ret:" << ret << XEND;
    return;
  }
  //calc total page count
  DWORD totalCount = ret;
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
    snprintf(where, sizeof(where), " where (`is_overlap` = 1) AND itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) OR (publicity_id <> 0))  group by `price`",
      itemId,
      shelfTime);

    snprintf(totalCountSql, sizeof(totalCountSql),
      "select `id`,`itemid`,`price`,COALESCE(sum(count),0) AS `total_count`,`refine_lv`,`is_overlap`, publicity_id, endtime from %s %s ",
      table,
    where);

    snprintf(sqlStr, sizeof(sqlStr), "select `id`,`itemid`,`price`,COALESCE(sum(count), 0) AS `total_count`,`refine_lv`,`is_overlap`, publicity_id, endtime from %s %s %s limit %u, %u",
      table,
    where,
      orderBy,
      offSet, maxPageCount);
  }
  else
  {
    snprintf(sqlStr, sizeof(sqlStr),
      " select `id`,`itemid`,`price`,`count` AS `total_count`,`refine_lv`,`is_overlap`, `itemdata`, publicity_id, endtime  from %s where (`is_overlap` = 0) AND itemid=%u AND ((publicity_id=0 and pendingtime >=%lu) OR (publicity_id <> 0)) "
      " %s limit %u, %u",
      table,
      itemId,
      shelfTime,
      orderBy,
      offSet, maxPageCount);
  }

  std::string sql2(sqlStr);
  xRecordSet set;
  ret = thisServer->getTradeConnPool().exeRawSelect(&field, set, sql2);
  if ((QWORD)-1 == ret)
  {
    XERR << "[交易-详细挂单] fetchDetailPendingList， 查询数据库出错， charid:" << rev.charid() << "ret:" << ret << XEND;
    return;
  }
  
  //std::vector<std::vector<DWORD>> vecRank;
  //
  //auto randomRank = [&](const char* byStr) {
  //  DWORD dwLast = 0;
  //  for (DWORD i = 0; i < ret; ++i)
  //  {
  //    DWORD dwCur = set[i].get<DWORD>(byStr);
  //    if (vecRank.empty() || dwLast != dwCur)
  //    {
  //      vecRank.push_back(std::vector<DWORD>());
  //    }
  //    std::vector<DWORD>& vec = vecRank[vecRank.size() - 1];
  //    vec.push_back(i);
  //    dwLast = dwCur;
  //  }
  //  for (auto it = vecRank.begin(); it != vecRank.end(); ++it)
  //  {
  //    std::random_shuffle(it->begin(), it->end());
  //  }
  //};

  //switch (rev.search_cond().rank_type())
  //{
  //case RANKTYPE_ITEM_PRICE_INC:   
  //case RANKTYPE_ITEM_PRICE_DES:
  //  randomRank("price");
  //  break;
  //case RANKTYPE_REFINE_LV_INC:    
  //case RANKTYPE_REFINE_LV_DES:
  //  randomRank("refine_lv");
  //  break;
  //default:
  //  randomRank("price");
  //  break;
  //}

  //for (auto it = vecRank.begin(); it != vecRank.end(); ++it)
  //{
  //  for (auto subIt = it->begin(); subIt != it->end(); ++subIt)
  //  {
  //    DWORD index = *subIt;
  //    
  //    const SExchangeItemCFG*p = ItemConfig::getMe().getExchangeItemCFG(set[index].get<DWORD>("itemid"));
  //    if (p == nullptr)
  //    {
  //      XINF << "[交易-详细挂单] [过滤掉不可出售物品] itemid:" << set[index].get<DWORD>("itemid") << XEND;
  //      continue;
  //    }
  //    TradeItemBaseInfo* pItem = rev.add_lists();
  //    pItem->set_order_id(set[index].get<QWORD>("id"));
  //    pItem->set_itemid(set[index].get<DWORD>("itemid"));
  //    pItem->set_price(set[index].get<DWORD>("price"));
  //    pItem->set_count(set[index].get<DWORD>("total_count"));
  //    pItem->set_refine_lv(set[index].get<DWORD>("refine_lv"));
  //    pItem->set_overlap(set[index].get<DWORD>("is_overlap"));
  //    std::string itemData((const char *)set[index].getBin("itemdata"), set[index].getBinSize("itemdata"));
  //    if (!itemData.empty())
  //      pItem->mutable_item_data()->ParseFromString(itemData);
  //  }
  //}

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

  PROTOBUF(rev, send, len);
  sendCmdToMe(charId, zoneId, (BYTE*)send, len);

  XDBG << "[交易-详细挂单] charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
}

bool TradeManager::checkCharId(QWORD charId1, QWORD charId2)
{
  if (charId1 == charId2)
    return true;
  XERR << "[交易]" << "玩家charid 不一致"<<charId1<<charId2 << XEND;
  return false;
}

bool TradeManager::sendCmdToMe(QWORD charId, DWORD zoneId, const BYTE* buf, WORD len)
{
  Cmd::SessionToMeRecordTrade cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);
  return thisServer->sendCmdToZone(zoneId, send, len2);
}

void TradeManager::sendSysMsg(QWORD charId, DWORD zoneId, DWORD msgid, EMessageType eType, const MsgParams& params, EMessageActOpt opt)
{
  if (charId == 0)
    return;

  SysMsg cmd;
  cmd.set_id(msgid);
  cmd.set_type(eType);
  cmd.set_act(opt);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  sendCmdToMe(charId, zoneId, (BYTE*)send, len);
}

void TradeManager::sendWorldMsg(DWORD msgid, EMessageType eType, const MsgParams& params, EMessageActOpt opt/* = EMESSAGEACT_ADD */)
{
  SysMsg cmd;
  cmd.set_id(msgid);
  cmd.set_type(eType);
  cmd.set_act(opt);
  params.toData(cmd);
  PROTOBUF(cmd, send, len);

  Cmd::WorldMsgCmd cmd2;
  cmd2.set_data(send, len);
  cmd2.set_len(len);
  PROTOBUF(cmd2, send2, len2);
  thisServer->sendCmdToAllZone(send2, len2);
}

//购买
bool TradeManager::reqBuy(QWORD charId, DWORD zoneId, Cmd::BuyItemRecordTradeCmd& rev)
{
  if (rev.item_info().count() == 0)
  {
    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_BUY_INVALID_COUNT, EMESSAGETYPE_FRAME, params);
    return false;
  }
  
  if (rev.item_info().has_publicity_id() && rev.item_info().publicity_id())
  {    
    return m_oPublicityMgr.reqBuy(charId, zoneId, rev);
  }

  StoreBaseMgr* pStoreBaseMgr = getStoreBaseMgr(rev.item_info().itemid());
  if (pStoreBaseMgr == nullptr)
  {
    XERR << "[交易-请求购买] 内存中找不到该订单，charid:" << rev.charid() << "orderid:"
      << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
    return false;
  }  
   
  if (pStoreBaseMgr->isOverlap())
  {
    OverlapStoreMgr* pOverlapMgr = dynamic_cast<OverlapStoreMgr*>(pStoreBaseMgr);
    if (pOverlapMgr == nullptr)
    {
      XERR << "[交易-请求购买] 内存中找不到该订单，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
      return false;
    }  
    if (!pOverlapMgr->checkBuyPrice(rev.item_info().price()))
    {
      XERR << "[交易-请求购买] 价格不合法，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_INVALID_PRICE, EMESSAGETYPE_FRAME, params);
      return false;
    }

    if (!pOverlapMgr->lock(rev.item_info().count()))
    {
      XERR << "[交易-请求购买] 该订单个数不够，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_PENDING_WAS_LOCKED, EMESSAGETYPE_FRAME, params);
      return false;
    }
  }
  else
  {
    if (!rev.item_info().has_order_id() || rev.item_info().order_id() == 0)
    {
      XERR << "[交易-请求购买] 参数不合法， 没有orderid，charid:" << rev.charid() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_INVALID_PARAMS, EMESSAGETYPE_FRAME, params);
      return false;     
    }

    if (rev.item_info().count() != 1)
    {
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_INVALID_COUNT, EMESSAGETYPE_FRAME, params);
      return false;
    }

    NonOverlapStoreMgr* pNoOverlapMgr = dynamic_cast<NonOverlapStoreMgr*>(pStoreBaseMgr);
    if (pNoOverlapMgr == nullptr)
    {
      XERR << "[交易-请求购买] 内存中找不到该订单，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
      //return Cmd::ETRADE_RET_CODE_INVALID_PARAMS;
      return false;
    }

    if (!pNoOverlapMgr->checkBuyPrice(rev.item_info().order_id(), rev.item_info().price()))
    {
      XERR << "[交易-请求购买] 价格不合法，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_INVALID_PRICE, EMESSAGETYPE_FRAME, params);
      return false;
    }

    if (!pNoOverlapMgr->lockOrder(rev.item_info().order_id()))
    {
      XERR << "[交易-请求购买] 订单已经锁定，或者不存在该订单，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_PENDING_WAS_LOCKED, EMESSAGETYPE_FRAME, params);
      return false;
    }
  }
  return true;
}

bool TradeManager::trueBuy(QWORD charId, const string& name, DWORD zoneId, Cmd::ReduceMoneyRecordTradeCmd& rev)
{
  if (rev.item_info().has_publicity_id() && rev.item_info().publicity_id())
  {
    return m_oPublicityMgr.trueBuy(charId, zoneId, rev);
  }

  StoreBaseMgr* pStoreBaseMgr = getStoreBaseMgr(rev.item_info().itemid());
  if (pStoreBaseMgr == nullptr)
  {
    XERR << "[交易-请求购买] 内存中找不到该订单，charid:" << rev.charid() << "orderid:"
      << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
    return false;
  }

  if (pStoreBaseMgr->isOverlap())
  {
    OverlapStoreMgr* pOverlapMgr = dynamic_cast<OverlapStoreMgr*>(pStoreBaseMgr);
    if (pOverlapMgr == nullptr)
    {
      XERR << "[交易-请求购买] 内存中找不到该订单，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
      return false;
    }

    if (!pOverlapMgr->unlock(rev.item_info().count()))
    {
      XERR << "[交易-请求购买] 解锁异常，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
      return false;
    }

    if (rev.ret() == ETRADE_RET_CODE_SUCCESS)
    {
      return pOverlapMgr->outStore(rev.charid(), name, zoneId, rev.item_info().price(), rev.item_info().count());
    }
    else {}
  }
  else
  {
    if (!rev.item_info().has_order_id() || rev.item_info().order_id() == 0)
    {
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_INVALID_PARAMS, EMESSAGETYPE_FRAME, params);
      return false;
    }

    if (rev.item_info().count() != 1)
    {
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_INVALID_COUNT, EMESSAGETYPE_FRAME, params);
      return false;
    }

    NonOverlapStoreMgr* pNoOverlapMgr = dynamic_cast<NonOverlapStoreMgr*>(pStoreBaseMgr);
    if (pNoOverlapMgr == nullptr)
    {
      XERR << "[交易-请求购买] 内存中找不到该订单，charid:" << rev.charid() << "orderid:" << rev.item_info().order_id()
        << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
      return false;
    }
    if (!pNoOverlapMgr->unlockOrder(rev.item_info().order_id()))
    {
      XERR << "[交易-请求购买] 解锁异常，charid:" << rev.charid() << "orderid:"
        << rev.item_info().order_id() << "itemid:" << rev.item_info().itemid() << "count:" << rev.item_info().count() << "price:" << rev.item_info().price() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_BUY_CANNOT_FIND_PENDING, EMESSAGETYPE_FRAME, params);
      return false;
    }
    if (rev.ret() == ETRADE_RET_CODE_SUCCESS)
    {
      return pNoOverlapMgr->outStore(rev.item_info().order_id(), rev.charid(), name, zoneId, rev.item_info().price(), rev.item_info().count());
    }
    else {}
  }

  return false;
}

//出售
bool TradeManager::reqSell(QWORD charId, DWORD zoneId, Cmd::SellItemRecordTradeCmd* rev, bool checkFull /*=true*/)
{
  if (rev == nullptr)
  {
    MsgParams params;
    sendSysMsg(charId, zoneId,  SYS_MSG_CFG_ERROR, EMESSAGETYPE_FRAME, params);
    return false;
  }
  if (rev->item_info().count() == 0)
  {
    XERR << "[交易-请求出售] 物品出售个数有错, charid:" << rev->charid() << "itemid:" << rev->item_info().itemid() << "count:" << rev->item_info().count() << XEND;
    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_SELL_INVALID_COUNT, EMESSAGETYPE_FRAME, params);
    return false;
  }

  if (rev->item_info().item_data().equip().lv() > 0)
  {
    XERR << "[交易-请求出售] 装备等级的不可出售, charid:" << rev->charid() << "itemid:" << rev->item_info().itemid() << "count:" << rev->item_info().count() << "upgradelv" << rev->item_info().item_data().equip().lv() << XEND;
    return false;
  }
  
  if (checkPublicity(rev->item_info().item_data()))
  {
    if (m_oPublicityMgr.reqSell(charId, zoneId, rev) == false)
      return false;
  }
  else
  {
    TradeItemInfo* pTradeItemInfo = getTradeItemInfo(rev->item_info().itemid());
    if (pTradeItemInfo == nullptr)
    {
      XERR << "[交易-请求出售] 找不到TradeItemInfo ,charid:" << rev->charid() << "itemid:" << rev->item_info().itemid() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_SELL_INVALID_PARAMS, EMESSAGETYPE_FRAME, params);
      return false;
    }

    if (!pTradeItemInfo->isOverlap())
    {
      //不可堆叠的物品一次只能卖一个
      if (rev->item_info().count() != 1)
      {
        XERR << "[交易-请求出售] 不可堆叠物品出售个数有错, charid:" << rev->charid() << "itemid:" << rev->item_info().itemid() << "count:" << rev->item_info().count() << XEND;
        MsgParams params;
        sendSysMsg(charId, zoneId, SYS_MSG_SELL_INVALID_COUNT, EMESSAGETYPE_FRAME, params);
        return false;
      }
    }

    DWORD serverPrice = 0;
    if (!TradeManager::getMe().checkIsRightPrice(rev->item_info(), serverPrice))
    {
      XERR << "[交易-请求出售] 出售的价格非法, charid:" << rev->charid() << "itemid:" << rev->item_info().itemid()
        << "sell price:" << rev->item_info().price() << "server pirce:" << serverPrice << XEND;

      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_SELL_INVALID_PRICE, EMESSAGETYPE_FRAME, params);
      return false;
    }
  }

  if (checkFull) {  //重新上架 不去检查
    //验证 挂了几单，最多只能挂8单 
    {
      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "sellerid=%lu", rev->charid());
      QWORD ret = thisServer->getTradeConnPool().getNum(REGION_DB, DB_TABLE_PENDING_LIST, where);
      DWORD maxPendingCount = getMaxPendingLimit(charId);
      if (ret >= maxPendingCount)
      {
        XLOG << "[交易-请求出售] charid:" << rev->charid() << "挂单已满" << XEND;
        MsgParams params;
        sendSysMsg(charId, zoneId, SYS_MSG_SELL_IS_FULL, EMESSAGETYPE_FRAME, params);
        return false;
      }
    }
  }
  return true;
}

bool TradeManager::trueSell(QWORD charId,std::string name, DWORD zoneId, Cmd::ReduceItemRecordTrade& rev)
{
  if (rev.item_info().has_key() && !rev.item_info().key().empty())
  {
    return m_oPublicityMgr.trueSell(charId, name, zoneId, rev);
  }
  else
  {
    TradeItemInfo* pTradeItemInfo = getTradeItemInfo(rev.item_info().itemid());
    if (pTradeItemInfo)
    {
      return pTradeItemInfo->addNewPending(charId, name, zoneId, rev);
    }
    else
    {
      XERR << "[交易-出售] 异常，找不到pTradeItemInfo， charid：" << rev.charid() << "itemid：" << rev.item_info().itemid() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_CFG_ERROR, EMESSAGETYPE_FRAME, params);
      return false;
    }
  }
  return false;
}

/*取消挂单，取回物品*/
bool TradeManager::cancelPending(QWORD charId, DWORD zoneId, Cmd::CancelItemRecordTrade& rev)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_DB_ERROR, EMESSAGETYPE_FRAME, params);
    XERR << "[交易-取消订单] charid:" << rev.charid() << "找不到数据库" << XEND;
    return false;
  }

  PendingInfo pendingInfo;
  if (TradeManager::getPendInfoFromDb(rev.order_id(), pendingInfo) == false)
  {
    //订单已被卖掉
    XLOG << "[交易-取消订单] charid:" << rev.charid() << "找不到挂单 orderid:" << rev.order_id() << XEND;

    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_CANCEL_ALREADY_SELLED, EMESSAGETYPE_FRAME, params);
    return false;
  }

  if (pendingInfo.sellerId != charId)
  {
    XERR << "[交易-取消订单] charid:" << rev.charid() << "非法下架" << rev.order_id() <<"true sellerid" <<pendingInfo.sellerId << XEND;
    return false;
  }
  
  if (pendingInfo.endTime != 0)
  {
    XERR << "[交易-取消订单] 异常，公示期物品不可下架， charid：" << rev.charid() << "itemid：" << rev.item_info().itemid()<<"orderid"<<rev.order_id() << "endtime"<<pendingInfo.endTime << XEND;
    return false;
  }

  bool isExpire = false;
  StoreBaseMgr* pStoreInfo = getStoreBaseMgr(pendingInfo.itemId);

  if (pStoreInfo && pStoreInfo->hasOrder(rev.order_id()))
  {//挂单没过期
    if (!pStoreInfo->lockOrder(rev.order_id()))
    {
      //订单被锁定 无法撤单
      XLOG << "[交易-取消挂单] charid：" << rev.charid() << "orderid：" << rev.order_id() << "挂单被锁定，无法撤单" << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_CANCEL_WAS_LOCKED, EMESSAGETYPE_FRAME, params);
      return false;
    }
    pStoreInfo->unlockOrder(rev.order_id());

    if (!pStoreInfo->reducePendingAutoErase(rev.order_id(), pendingInfo.count, nullptr))
    {
      XERR << "[交易-取消挂单] charid:" << rev.charid() << "操作失败， orderinfo：" << pendingInfo.toString() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_DB_ERROR, EMESSAGETYPE_FRAME, params);
      return false;
    }else {
      XINF << "[交易-取消挂单] charid：" << rev.charid() << "撤单成功， orderinfo：" << pendingInfo.toString() << XEND;
    }
  }
  else
  {//过期挂单
    isExpire = true;
  }

  if (isExpire)
  {//过期，删除挂单
    char deleWhere[128];
    bzero(deleWhere, sizeof(deleWhere));
    snprintf(deleWhere, sizeof(deleWhere), "id=%lu", rev.order_id());
    QWORD ret = thisServer->getTradeConnPool().exeDelete(field, deleWhere);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-取消挂单] 从数据库删除挂单失败，charid:" << rev.charid() << "orderid:" << rev.order_id() << XEND;
      MsgParams params;
      sendSysMsg(charId, zoneId, SYS_MSG_DB_ERROR, EMESSAGETYPE_FRAME, params);
      return false;
    }
  }

  //返回装备      
  AddItemRecordTradeCmd sendCmd;
  TradeItemBaseInfo* pBaseInfo = sendCmd.mutable_item_info();
  pendingInfo.pack2Proto(pBaseInfo);
  sendCmd.set_charid(rev.charid());
  sendCmd.set_addtype(EADDITEMTYP_RETURN);
  PROTOBUF(sendCmd, send, len);
  if (thisServer->sendCmdToZone(zoneId, send, len) == false)
  {
    MailManager::getMe().addOfflineTradeItem(sendCmd);
  }

  // //撤单
  string jsonStr = pb2json(pBaseInfo->item_data());
  PlatLogManager::getMe().TradeLog(thisServer,
    thisServer->getPlatformID(),
    rev.charid(),
    ETRADETYPE_CANCEL,
    pendingInfo.itemId,
    pendingInfo.count,
    pendingInfo.price,
    0,
    0,
    jsonStr);
  XINF << "[交易] 撤单成功，" << charId << zoneId << "orderid:" << rev.order_id() << XEND;

  DWORD percent = getReturnRate(rev.charid());
  if (percent)      //总价的千分比
  {
    QWORD boothfee = calcBoothfee(pendingInfo.price, pendingInfo.count);
    QWORD retMoney = boothfee * percent / 1000;
    if (retMoney)
    {
      //返还部分费用
      AddItemRecordTradeCmd sendCmd;
      TradeItemBaseInfo* pBaseInfo = sendCmd.mutable_item_info();
      if (pBaseInfo)
      {
        pBaseInfo->set_itemid(100);
        pBaseInfo->set_count(retMoney);
        sendCmd.set_charid(rev.charid());
        sendCmd.set_addtype(EADDITEMTYP_RETURN);
        PROTOBUF(sendCmd, send, len);
        if (thisServer->sendCmdToZone(zoneId, send, len) == false)
        {
          MailManager::getMe().addOfflineTradeItem(sendCmd);
        }
        XLOG << "[交易所-下架返还费用] charid" << rev.charid() << zoneId << "orderid:" << rev.order_id() << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price
          << "percent" << percent << "retmoney" << retMoney << XEND;
      }
    }
  }

  return true;
}

//del char
bool TradeManager::cancelPendingDelChar(QWORD charId, QWORD orderId)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
    return false;

  PendingInfo pendingInfo;
  if (TradeManager::getPendInfoFromDb(orderId, pendingInfo) == false)
  {
    //订单已被卖掉
    XLOG << "[交易-角色删除] charid:" << charId << "找不到挂单 orderid:" << orderId << XEND;
    return false;
  }

  bool isExpire = false;
  StoreBaseMgr* pStoreInfo = getStoreBaseMgr(pendingInfo.itemId);
  if (pStoreInfo && pStoreInfo->hasOrder(orderId))
  {//挂单没过期
    if (!pStoreInfo->lockOrder(orderId))
    {
      //订单被锁定 无法撤单
      XLOG << "[交易-取消挂单-角色删除] charid：" << charId << "orderid：" << orderId << "挂单被锁定，无法撤单" << XEND;
      return false;
    }
    pStoreInfo->unlockOrder(orderId);
    if (!pStoreInfo->reducePendingAutoErase(orderId, pendingInfo.count, nullptr))
    {
      XERR << "[交易-取消挂单-角色删除] charid:" << charId << "操作失败， orderinfo：" << pendingInfo.toString() << XEND;
      return false;
    }
    else 
    {
      XINF << "[交易-取消挂单-角色删除] charid：" << charId << "撤单成功， orderinfo：" << pendingInfo.toString() << XEND;
    }
  }
  else
  {//过期挂单
    isExpire = true;
  }

  if (isExpire)
  {//过期，删除挂单
    char deleWhere[128];
    bzero(deleWhere, sizeof(deleWhere));
    snprintf(deleWhere, sizeof(deleWhere), "id=%llu", orderId);
    QWORD ret = thisServer->getTradeConnPool().exeDelete(field, deleWhere);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-取消挂单-角色删除] 从数据库删除挂单失败，charid:" << charId << "orderid:" << orderId << XEND;  
      return false;
    }
  }
  XINF << "[交易-角色删除] 撤单成功，" << charId << "orderid:" << orderId << XEND;
  return true;
}

bool TradeManager::resell(QWORD charId, DWORD zoneId, Cmd::ResellPendingRecordTrade& rev)
{
  TradeItemInfo* pTradeItemInfo = getTradeItemInfo(rev.item_info().itemid());
  if (!pTradeItemInfo)
  {
    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_CFG_ERROR, EMESSAGETYPE_FRAME, params);
    return false;
  }
  //pTradeItemInfo->checkExpirePending();

  PendingInfo pendingInfo;
  if (TradeManager::getPendInfoFromDb(rev.order_id(), pendingInfo, true) == false)
  {
    MsgParams params;
    sendSysMsg(charId, zoneId, SYS_MSG_SELL_INVALID_PARAMS, EMESSAGETYPE_FRAME, params);
    XERR << "[交易-重新上架] 找不到该挂单， charid：" << rev.charid() << "orderid：" << rev.order_id() << XEND;
    return false;
  }

  if (pendingInfo.sellerId != charId)
  {
    XERR << "[交易-重新上架] 非法， charid" << charId << "orderid" << pendingInfo.orderId << "true sellerid" << pendingInfo.sellerId << XEND;
    return false;
  }


  DWORD dwNewPrice = rev.item_info().price();

  StoreBaseMgr* pStoreBaseMgr = pTradeItemInfo->getStoreBaseMgr();
  if (pStoreBaseMgr != nullptr)
  {
    if (pStoreBaseMgr->hasOrder(rev.order_id()))
    {//尚未过期
      XERR << "[交易-重新上架] 尚未过期 charid：" << rev.charid() << "orderid：" << rev.order_id() << XEND;
      return false;
    }
  }

  Cmd::SellItemRecordTradeCmd cmd;
  cmd.set_charid(rev.charid());
  cmd.mutable_item_info()->set_itemid(pendingInfo.itemId);
  cmd.mutable_item_info()->set_count(pendingInfo.count);
  cmd.mutable_item_info()->set_price(dwNewPrice);
  cmd.mutable_item_info()->set_refine_lv(pendingInfo.refineLv);
  cmd.mutable_item_info()->mutable_item_data()->CopyFrom(pendingInfo.itemData);

  if (!reqSell(charId, zoneId, &cmd, false))
  {
    XERR << "[交易-重新上架] 出售失败， charid：" << rev.charid() << "orderid：" << rev.order_id() << XEND;
    return false;
  }

  //通知scene 扣除装备  
  Cmd::ReduceItemRecordTrade sendCmd;
  sendCmd.mutable_item_info()->CopyFrom(cmd.item_info());
  sendCmd.set_orderid(rev.order_id());
  sendCmd.set_is_resell(true);
  sendCmd.set_charid(rev.charid());
  sendCmd.set_boothfee(calcBoothfee(sendCmd.item_info().price() , sendCmd.item_info().count()));
  PROTOBUF(sendCmd, send, len);
  thisServer->sendCmdToZone(zoneId, send, len);
  XINF << "[交易-重新上架] 出售，通知scene 扣除上架费 charid:" << sendCmd.charid() << "上架费:" << sendCmd.boothfee() << "itemid:" << "count:" << sendCmd.item_info().count() << "price:" << sendCmd.item_info().price() << XEND;

  //// //上架
  //string jsonStr = pb2json(sendCmd.item_info().item_data());
  //PlatLogManager::getMe().TradeLog(thisServer,
  //  thisServer->getPlatformID(),
  //  sendCmd.charid(),
  //  ETRADETYPE_RESELL,
  //  sendCmd.item_info().itemid(),
  //  sendCmd.item_info().count(),
  //  sendCmd.item_info().price(),
  //  sendCmd.boothfee(),
  //  0,
  //  jsonStr);

  return true;
}

void TradeManager::openPanel(QWORD charId, DWORD zoneId)
{
  m_panelMap[charId] = zoneId;
}

void TradeManager::closePanel(QWORD charId)
{
  m_panelMap.erase(charId);
}

void TradeManager::ListNtf(QWORD charId, EListNtfType type)
{
  auto it = m_panelMap.find(charId);
  if (it == m_panelMap.end())
    return;

  ListNtfRecordTrade cmd;
  cmd.set_charid(charId);
  cmd.set_type(type);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(charId, it->second, (BYTE*)send, len);
}

void TradeManager::fetchHotPendingList(QWORD charId, DWORD zoneId, Cmd::HotItemidRecordTrade& rev)
{ 
  if (charId != rev.charid())
  {
    return;
  }

  //公示期
  std::vector<DWORD> publicityVec = m_hotInfoMgr.getPublicityRank();

  //非公示期物品

  std::set<DWORD> addSet;
  DWORD dwTotalWant = 0;  

  auto func = [&](const xEntry* pEntry)
  {
    const STradeItemTypeData* pBase = static_cast<const STradeItemTypeData*>(pEntry);
    if (!pBase)
      return;
    if (pBase->getHotSale() == 0)
      return;
    getHotId(rev.job(), pBase->id, pBase->getHotSale(), addSet);
    dwTotalWant += pBase->getHotSale();
  };
  Table<STradeItemTypeData>* pTradeItemList = TableManager::getMe().getTradeItemTypeCFGListNoConst();
  if (pTradeItemList != nullptr)
    pTradeItemList->foreachNoConst(func);

  //个数不够，补充
  if (addSet.size() < dwTotalWant)
  {
    DWORD diffCount = dwTotalWant - addSet.size();
    DWORD addCount = 0;
    for (auto it = m_itemInfoMap.begin(); it != m_itemInfoMap.end(); ++it)   //这里没有做随机排序
    {
      if (addCount >= diffCount)
        break;

      const SItemCFG *pCfg = ItemConfig::getMe().getItemCFG(it->first);
      if (!pCfg)
        continue;
      if (!pCfg->isRightJob(rev.job())) 
        continue;
      if (addSet.find(it->first) != addSet.end())
        continue;
      if (!it->second)
        continue;
      //it->second->checkExpirePending();
      if (it->second->hasPending())
      {
        addCount++;
        addSet.insert(it->first);
        XDBG << "[交易-补充添加] temid:" << it->first << XEND;
      }
    }
  }
    
  //排序
  std::vector<DWORDPAIR> rankVec;
  rankVec.reserve(addSet.size());
  for (auto it = addSet.begin(); it != addSet.end(); ++it)
  {
    const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(*it);
    if (!pCfg)
      continue;;
    DWORD dwOrder = pCfg->dwShowOrdcer;
    rankVec.push_back(std::make_pair(*it, dwOrder));
  }
  std::sort(rankVec.begin(), rankVec.end(), CmpByValue2());
  
  std::set<DWORD> publicitySet;
  for (auto&v : publicityVec)
  {
    XDBG << "[交易-公示期-热门展示排序] itemid:" << v << XEND;
    rev.add_pub_lists(v);
    publicitySet.insert(v);
  }

  for (auto it = rankVec.begin(); it != rankVec.end(); ++it) 
  {
    if (publicitySet.find(it->first) == publicitySet.end())
    {
      XDBG << "[交易-热门展示排序] itemid:" << it->first << "order:" << it->second << XEND;
      rev.add_lists(it->first);
    }
  }
  
  PROTOBUF(rev, send, len);
  sendCmdToMe(charId, zoneId, (BYTE*)send, len);
  XDBG << "[交易-热门返回] charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
}

void TradeManager::getHotId(DWORD dwJob, DWORD dwShowType, DWORD dwCount, std::set<DWORD>& resultSet)
{
  //热门排序
  std::vector<DWORD> oldHotVec= TradeManager::getMe().m_hotInfoMgr.getRank(dwShowType, dwJob);

  DWORD dwAddCount = 0;
  for (auto it = oldHotVec.begin(); it != oldHotVec.end(); ++it)
  {
    if (dwAddCount >= dwCount)
      break;
    TradeItemInfo* pItemInfo = getTradeItemInfo(*it);
    if (pItemInfo)
    {
      //pItemInfo->checkExpirePending();
      if (pItemInfo->hasPending())
      {
        resultSet.insert(*it);
        XDBG << "[交易-热门添加] type:" << dwShowType << "job:" << dwJob << "itemid:" << *it << XEND;
        dwAddCount++;
      }
    }
  }
}

bool TradeManager::getPendInfoFromDb(QWORD orderId, PendingInfo& out, bool fillItemData /*= false*/)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    XINF << "[交易] 找不到挂单 orderid:" << orderId << XEND;
    return false;
  }
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%llu", orderId);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XINF << "[交易] 找不到挂单 orderid:" << orderId << XEND;
    return false;
  }
  if (ret == 0)
  {
    //订单已被卖掉 
    XINF << "[交易] 找不到挂单 orderid:" << orderId << XEND;
    return false;
  }

  out.orderId = set[0].get<QWORD>("id");
  out.itemId = set[0].get<DWORD>("itemid");
  out.price = set[0].get<DWORD>("price");
  out.sellerId = set[0].get<QWORD>("sellerid");
  out.name = set[0].getString("name");
  out.count = set[0].get<DWORD>("count");
  out.pendingTime = set[0].get<QWORD>("pendingtime");
  out.isOverlap = set[0].get<DWORD>("is_overlap");
  std::string itemData((const char *)set[0].getBin("itemdata"), set[0].getBinSize("itemdata"));
  out.refineLv = set[0].get<DWORD>("refine_lv");
  out.publicityId = set[0].get<DWORD>("publicity_id");
  out.endTime = set[0].get<DWORD>("endtime");
  out.itemData.ParseFromString(itemData);
  if (fillItemData)
  {
    out.itemData.mutable_base()->set_id(out.itemId);
  }
  return true;
}

void TradeManager::timeTick(DWORD curSec)
{
  if (m_oOneSecTimer.timeUp(curSec))
  {
    m_oFrameTimer.elapseStart();
    m_oPublicityMgr.timeTick(curSec);

    QWORD e = m_oFrameTimer.uElapse();
    if (e > 30 * 1000)
      XLOG << "[帧耗时]" << "publicity" << e << " 微秒" << XEND;

    if (m_oFivSecTimer.timeUp(curSec))
    {
      m_oFrameTimer.elapseStart();
      adjustPrice(curSec);
      e = m_oFrameTimer.uElapse();
      if (e > 30 * 1000)
        XLOG << "[帧耗时]" << "adjust price" << e << " 微秒" << XEND;
      if (m_oOneMinTimer.timeUp(curSec))
      {
        if (m_oTenMinTimer.timeUp(curSec))
        {
          m_oMessageStat.print();

          if (m_oOneHourTimer.timeUp(curSec))
          {
            m_hotInfoMgr.refresh(curSec);
          }
        }

        for (auto &v : m_itemInfoMap)
        {
          v.second->checkExpirePending();
        }
      }
    }

    if (m_oOneDayTimer.timeUp(curSec))
    {
      onOneDayTimeUp(curSec);
    }
  }
}

//每天5点刷新
void  TradeManager::onOneDayTimeUp(DWORD curSec)
{
  MiscConfig::getMe().loadConfig();

  for (auto &m : m_itemInfoMap)
  {
    if (m.second && !m.second->isOverlap())
    {
      m.second->adjustSelfPrice();
    }
  }
  XLOG << "[交易-重加载GameConfig] weekrate" << MiscConfig::getMe().getTradeCFG().dwWeekRefineRate << "monthrate"<< MiscConfig::getMe().getTradeCFG().dwMonthRefineRate << XEND;
}

void TradeManager::savePendingPrice(QWORD orderid, DWORD itemId, DWORD oldPrice, DWORD newPrice)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (field)
  {
    xRecord record(field);
    record.put("price", newPrice);
    char where[32] = { 0 };
    snprintf(where, sizeof(where), "id=%llu", orderid);
    QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX)
    {
      XERR << "[交易-上架价格调整] 修改失败， orderid" << orderid << "itemid" << itemId << "old price" << oldPrice << " NEW price" << newPrice << XEND;
    }
    else
    {
      XINF << "[交易-上架价格调整] 成功， orderid" << orderid << "itemid" << itemId << "old price" << oldPrice << " NEW price" << newPrice << XEND;
    }
    return;
  }
  XERR << "[交易-上架价格调整]  失败，找不到数据库， orderid" << orderid << "itemid" << itemId << "old price" << oldPrice << " NEW price" << newPrice << XEND;
}

void TradeManager::getItemSellInfo(Cmd::ReqServerPriceRecordTradeCmd& rev)
{
  string key = generateKey(rev.itemdata());
  if (rev.issell())
  {
    bool getPrice = false;
    if (checkPublicity(rev.itemdata()))
    {
      PublicityInfo* pInfo = m_oPublicityMgr.getPublicityInfo(key);
      if (pInfo && pInfo->isPublicity())
      {
        rev.set_statetype(Cmd::St_InPublicity);
        rev.set_count(pInfo->getCount());
        rev.set_buyer_count(pInfo->getBuyerCount());
        rev.set_end_time(pInfo->endTime);
        rev.set_price(pInfo->price);
        getPrice = true;
      }
      else
      {
        rev.set_statetype(Cmd::St_WillPublicity);
      }
    }
    else
    {
      TradeItemInfo* pInfo = getTradeItemInfo(rev.itemdata().base().id());
      if (!pInfo)
        return;
      if (pInfo->isOverlap())
      {
        DWORD count = 0;
        pInfo->getDbPendingCount(0, false, 0, count);
        rev.set_statetype(Cmd::St_OverlapNormal);
        rev.set_count(count);
      }
      else
      {
        rev.set_statetype(Cmd::St_NonoverlapNormal);
      }
    }

    if (!getPrice)
    {      
      DWORD serverPrice = getServerPrice(rev.itemdata(), 0);
      rev.set_price(serverPrice);
      updatePriceCache(rev.itemdata(), serverPrice);
      return;
    }
  }
  else
  { //不是出售
    PublicityInfo* pInfo = m_oPublicityMgr.getPublicityInfo(key);
    if (pInfo && pInfo->isPublicity())
    {
      rev.set_price(pInfo->price);
      return;
    }
    
    DWORD price;
    if (getPriceFromCache(rev.itemdata(), price) == false)
    {
      price = getServerPrice(rev.itemdata(), 0);
      updatePriceCache(rev.itemdata(), price);
    }
    rev.set_price(price);
    return;
  }
}

void TradeManager::fetchItemSellInfoList(QWORD charId, DWORD zoneId, Cmd::ItemSellInfoRecordTradeCmd& rev)
{
  const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(rev.itemid());
  if (pCfg == nullptr)
    return;

  //公示期产品
  if (rev.has_publicity_id() && rev.publicity_id())
  {
    //sell count
    PublicityInfo* pPublicityInfo = m_oPublicityMgr.getPublicityInfo(rev.publicity_id());
    if (!pPublicityInfo)
      return;
    rev.set_statetype(St_InPublicity);
    rev.set_count(pPublicityInfo->getCount());
    //buy people
    rev.set_buyer_count(pPublicityInfo->getBuyerCount());
  }
  else
  {
    StoreBaseMgr* pStoreBaseMgr = getStoreBaseMgr(rev.itemid());
    if (!pStoreBaseMgr)
      return;
    if (pCfg->isOverlap)
    {
      rev.set_statetype(St_OverlapNormal);
      rev.set_count(pStoreBaseMgr->getPendingCount(0, false, 0));
    }
    else
      rev.set_statetype(St_NonoverlapNormal);

    std::vector<Cmd::BriefBuyInfo>& vecInfo = pStoreBaseMgr->getBriefBuyInfo();    

    for (auto it = vecInfo.rbegin(); it != vecInfo.rend(); ++it)
    {
      BriefBuyInfo* pInfo = rev.add_buy_info();
      if (pInfo)
      {
        pInfo->CopyFrom(*it);
      }
    }
  }

  PROTOBUF(rev, send, len);
  sendCmdToMe(charId, zoneId, (BYTE*)send, len);
  XDBG << "[交易-在售物品信息] charid:" << rev.charid() << "msg:" << rev.ShortDebugString() << XEND;
}

DWORD TradeManager::calcBoothfee(DWORD price, DWORD count)
{
  float fboothfee = price * MiscConfig::getMe().getTradeCFG().fSellCost;
  DWORD boothfee =  std::ceil(fboothfee);
  if (boothfee > MiscConfig::getMe().getTradeCFG().dwMaxBoothfee)
    boothfee = MiscConfig::getMe().getTradeCFG().dwMaxBoothfee;
  boothfee = boothfee * count;
  return boothfee;
}

bool TradeManager::delChar(QWORD charId)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
    return false;
  field->setValid("id");
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "sellerid=%llu", charId);
  xRecordSet set;
  QWORD ret = thisServer->getTradeConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-数据库] 我的挂单列表 数据库错误 charid：" << charId << "table: " << DB_TABLE_PENDING_LIST << XEND;
    return false;
  }
  for (QWORD i = 0; i < ret; ++i)
  {
    QWORD orderId = set[i].get<QWORD>("id");    
    cancelPendingDelChar(charId, orderId);
  }
  return true;
}

void TradeManager::adjustPrice(DWORD curSec)
{
  bool bPriceChange = false;
  std::set<DWORD/*itemid*/> setItemid;

  std::set<DWORD> setItem1;

  for (auto &v : m_itemInfoMap)
  {
    if (v.second)
    {
      bPriceChange = v.second->adjustServerPrice(curSec);
      if (bPriceChange)
        setItem1.insert(v.first);
    }
  }
  
  while (true)
  {
    std::set<DWORD> temp = setItem1;
    setItem1.clear();
    if (temp.empty())
      break;

    for (auto& v : temp)
    {
      auto setIt = m_mapPrice.find(v);
      if (setIt != m_mapPrice.end())
      {
        for (auto &subV : setIt->second)
        {
          XINF << "[交易-价格调整] 因" << v << "调整" << subV << XEND;
          setItemid.insert(subV);
          if (subV != v)
            setItem1.insert(subV);
        }
      }
    }
  }  
    
  for (auto &itemId : setItemid)
  {
    TradeItemInfo* pItemInfo = getTradeItemInfo(itemId);
    if (!pItemInfo)
      continue;

    pItemInfo->adjustSelfPrice();
  }
}

DWORD TradeManager::getServerPrice(const Cmd::ItemData& itemData, DWORD itemId /*=0*/)
{
  if (!itemData.has_base())
  {
    return getServerPrice(itemId);
  }

  itemId = itemData.base().id();
  DWORD refineLv = itemData.equip().refinelv();
  QWORD dwServerPrice = 0;

  do 
  {
    XDBG << "[交易-价格关联===============] map" << itemId << "refinelv" << refineLv << XEND;
    auto &s = m_mapPrice[itemId];
    s.insert(itemId);
    XDBG << "[交易-价格关联] insert" << itemId << itemId << XEND;
    if (refineLv > 0)
    {
      dwServerPrice = calcRefinePrice(itemId, refineLv, itemData.equip().damage());
      ////精炼装备服务器价格算法
      //const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(itemId);
      //if (pItemCFG == nullptr)
      //{
      //  XERR << "[交易-服务器价格] 失败，获取不到SItemCFG 策划表 itemid：" << itemId << XEND;
      //  break;
      //}

      //if (pItemCFG->eEquipType <= EEQUIPTYPE_MIN || pItemCFG->eEquipType >= EEQUIPTYPE_MAX)
      //{
      //  XERR << "[交易-服务器价格] 失败，装备类型错误 itemid：" << itemId << "equiptype：" << pItemCFG->eEquipType << XEND;
      //  break;
      //}

      //const SRefineTradeCFG* pRefineTradeCFG = ItemConfig::getMe().getRefineTradeCFG(pItemCFG->eItemType, refineLv);
      //if (pRefineTradeCFG == nullptr)
      //{
      //  XERR << "[交易-服务器价格] 失败，获取不到pRefineTradeCFG 策划表 itemtype:" << pItemCFG->eItemType << "refinelv：" << refineLv << XEND;
      //  break;
      //}

      //DWORD otherItemid = pRefineTradeCFG->dwItemID;
      ////add price map
      //auto &s = m_mapPrice[otherItemid];
      //s.insert(itemId);
      //XDBG << "[交易-价格关联] insert refine" << otherItemid << itemId << XEND;

      ////获取低洞装备
      //DWORD lowItemId = 0;
      //{
      //  lowItemId = ItemConfig::getMe().getLowVidItemId(pItemCFG->dwVID);
      //  if (lowItemId == 0)
      //    lowItemId = itemId;
      //  else
      //  {
      //    auto &s = m_mapPrice[lowItemId];
      //    s.insert(itemId);
      //  }
      //}

      //QWORD  selfPrice = getServerPrice(itemId);
      //QWORD lowPrice = getServerPrice(lowItemId);
      //dwServerPrice = lowPrice * pRefineTradeCFG->dwEquipRate;
      //dwServerPrice += getServerPrice(otherItemid)*pRefineTradeCFG->dwItemRate;
      //dwServerPrice += pRefineTradeCFG->dwLvRate;
      //dwServerPrice = selfPrice + (dwServerPrice)* pRefineTradeCFG->fRiskRate;

      ////破损装备减去一次自身价格
      //if (itemData.equip().damage())
      //{
      //  dwServerPrice -= lowPrice;
      //}
    }
    else
    {
      dwServerPrice = getServerPrice(itemId);
    }

    //enchant
    if (itemData.has_enchant())
    {
      dwServerPrice += calcEnchantPrice(itemData);
    }
    
    //upgrade lv
    if (itemData.equip().lv())
    {
      dwServerPrice += calcUpgradePrice(itemId, itemData.equip().lv(), false);
    }

  } while (0);

  QWORD maxDWORD = 4000000000;
  if (dwServerPrice > maxDWORD)
    dwServerPrice = maxDWORD;

  return dwServerPrice;
}

DWORD TradeManager::getServerPrice(DWORD itemId)
{
  XDBG << "[交易-价格关联===============] map" << itemId << XEND;
  auto &s = m_mapPrice[itemId];
  s.insert(itemId);
  XDBG << "[交易-价格关联] insert" << itemId << itemId << XEND;

  QWORD retPrice = 0;
  do 
  {
    //refinelv == 0
    const SExchangeItemCFG *pBase = ItemConfig::getMe().getExchangeItemCFG(itemId);
    if (pBase == nullptr)
    {
      XERR << "[交易-服务器价格] 获取不到相应id 的配置表， itemid:" << itemId << XEND;
      break;
    }

    TradeItemInfo* pMyself = getTradeItemInfo(itemId);
    if (pMyself == nullptr)
    {
      //交易所不能交易的，但是服务器价格会用到的物品。
      if (pBase->dwPriceType == PRICETYPE_SELF)
      {
        retPrice = pBase->dwPrice;
        //check min price
        QWORD minPrice = getMinServerPrice(pBase);
        retPrice = std::max(retPrice, minPrice);
        break;
      }
    }
    else
    {
      if (pBase->dwPriceType == PRICETYPE_SELF)
      {
        retPrice = (QWORD)pMyself->getServerPrice();
        //第一次从数据库加载时 对最小价格进行验证
        if (retPrice == 0)
          retPrice = pBase->dwPrice;

        //check min price
        QWORD minPrice = getMinServerPrice(pBase);
        retPrice = std::max(retPrice, minPrice);
        pMyself->setServerPrice(retPrice);
        break;
      }
    }

    if (pBase->dwPriceType == PRICETYPE_SUM)
    {
      for (auto&v : pBase->vecPriceSum)
      {
        retPrice += (QWORD)(getServerPrice(v.first)) * v.second;
        //add price map
        auto &s = m_mapPrice[v.first];
        s.insert(itemId);
        XDBG << "[交易-价格] insert sum" << v.first << itemId << XEND;
      }
      break;
    }
  } while (0);

  QWORD maxDWORD = 4000000000;
  if (retPrice > maxDWORD)
    retPrice = maxDWORD;

  return retPrice;
}

DWORD TradeManager::getMinServerPrice(const SExchangeItemCFG* pCfg)
{
  if (!pCfg)
    return 0;

  DWORD price = 0;
  if (pCfg->minPrice.type == MINPRICETYPE_SELF)
    return pCfg->minPrice.dwPrice;
  else if (pCfg->minPrice.type == MINPRICETYPE_EQUIP_UPGRADE)
  {
    price += getServerPrice(pCfg->minPrice.dwEquipUpgrade);
    price = price +(calcUpgradePrice(pCfg->minPrice.dwEquipUpgrade, 0, true) * pCfg->minPrice.ratio);
    auto &s = m_mapPrice[pCfg->minPrice.dwEquipUpgrade];
    s.insert(pCfg->dwId);
    return price;
  }
  else
  {
    DWORD price = pCfg->minPrice.dwRob;
    for (auto &v : pCfg->minPrice.vecMaterial)
    {
      auto &s = m_mapPrice[v.id()];
      s.insert(pCfg->dwId);
      price += getServerPrice(v.id()) * v.count();
    }
    price = price * pCfg->minPrice.ratio;       
    return price;
  }
  return 0;
}

bool TradeManager::checkIsRightPrice(const Cmd::TradeItemBaseInfo& rItemBaseInfo, DWORD& serverPrice)
{
  serverPrice = getServerPrice(rItemBaseInfo.item_data(), rItemBaseInfo.itemid());
  return serverPrice == rItemBaseInfo.price();
}

bool TradeManager::checkPublicity(const Cmd::ItemData& itemData) 
{
  DWORD itemId = itemData.base().id();
  std::string key = generateKey(itemData);
  PublicityInfo* pInfo = m_oPublicityMgr.getPublicityInfo(key);
  
  //正在公示中的
  if (pInfo && pInfo->isPublicity())
      return true;

  //附魔赞的 不用检查交易量和库存
  if (ItemConfig::getMe().isGoodEnchant(itemData))
    return true;

  //成交量小于配置的不公示
  const SExchangeItemCFG* pCFG = ItemConfig::getMe().getExchangeItemCFG(itemId);
  if (!pCFG)
    return false;

  DWORD count = m_hotInfoMgr.getCount(itemId);
  if (count < pCFG->dwExchangeNum)
  {
    return false;
  }

  //check 库存
  TradeItemInfo* pTradeInfo = getTradeItemInfo(itemId);
  if (!pTradeInfo)
    return false;
  count = 0;
  pTradeInfo->getDbPendingCount(itemData.equip().refinelv(), itemData.equip().damage(), itemData.equip().lv(), count);
  if (count > 0)
    return false;
  return true;
}

void TradeManager::addItem(EOperType type, QWORD buyerId, DWORD zoneId, const PendingInfo& pendingInfo, bool bOffline /*=false*/)
{
  if (zoneId == 0)
  {
    zoneId = TradeManager::getMe().getMyZoneId(buyerId);
  }

  AddItemRecordTradeCmd sendCmd;
  TradeItemBaseInfo* pBaseInfo = sendCmd.mutable_item_info();
  pendingInfo.pack2Proto(pBaseInfo);
  sendCmd.set_charid(buyerId);
  sendCmd.set_addtype(EADDITEMTYP_BUY);
  PROTOBUF(sendCmd, send, len);
  if (thisServer->sendCmdToZone(zoneId, send, len) == false)
  {
  }
  else 
    XINF << "[交易][交易完成，玩家获得装备] type:"<<type<< "buyerid:" << buyerId << "itemid:" << pendingInfo.itemId << "count:" << pendingInfo.count << "price:" << pendingInfo.price << XEND;
  
  //msg 10159
  //您成功购买了[63cd4e]%s[-]个[63cd4e]%s[-]，共扣除[63cd4e]%s[-]Zeny，请在[63cd4e]交易记录[-]中领取商品。
  if (type == EOperType_NoramlBuy)
  {
    DWORD totalMoney = pendingInfo.count * pendingInfo.price;
    auto *pCfg = ItemConfig::getMe().getExchangeItemCFG(pendingInfo.itemId);
    if (pCfg)
    {
      MsgParams params;
      params.addNumber(pendingInfo.count);
      params.addString(pCfg->strName);
      params.addNumber(totalMoney);
      sendSysMsg(buyerId, zoneId, 10159, EMESSAGETYPE_FRAME, params);
    }
  }

  //msg  购买了谁的某个物品
  if (!pendingInfo.name.empty())
  {
    auto *pCfg = ItemConfig::getMe().getExchangeItemCFG(pendingInfo.itemId);
    if (pCfg && !pCfg->isOverlap)
    {
      sendSysMsg(buyerId, zoneId, 10157, EMESSAGETYPE_FRAME, MsgParams(pendingInfo.name, pCfg->strName));
    }
  }

  //购买
  ETRADE_TYPE tradeType = ETRADETYPE_TRUEBUY;
  if (type != EOperType_NoramlBuy)
  {
    tradeType = ETRADETYPE_PUBLICITY_BUY;
  }
  string jsonStr = pb2json(pBaseInfo->item_data());
  PlatLogManager::getMe().TradeLog(thisServer,
    thisServer->getPlatformID(),
    buyerId,
    tradeType,
    pendingInfo.itemId,
    pendingInfo.count,
    pendingInfo.price,
    0,
    pendingInfo.count* pendingInfo.price,
    jsonStr,
    pendingInfo.sellerId);
}

//只是发消息
void TradeManager::addMoney(EOperType type, QWORD getMoneyPlayer, DWORD sellerZoneId, const PendingInfo& pendingInfo, QWORD buyerId, DWORD successCount /*=0*/, DWORD consumeMoney/*=0*/)
{
  if (sellerZoneId == 0)
  {
    sellerZoneId = TradeManager::getMe().getMyZoneId(getMoneyPlayer);
  }
  const SExchangeItemCFG *pBase = ItemConfig::getMe().getExchangeItemCFG(pendingInfo.itemId);
  if (pBase == nullptr)
  {
    XERR << "[交易-购买] 策划表错误 读取不到 itemid:" << pendingInfo.itemId << XEND;
    return;
  }

  DWORD sellCount = pendingInfo.count;
  //if (sellCount == 0)
  //  return;

  AddMoneyRecordTradeCmd sendCmd;
  sendCmd.set_charid(getMoneyPlayer);
  sendCmd.set_money_type(pBase->dwMoneyType);
  DWORD gainMoney = sellCount * pendingInfo.price;

  ETRADE_TYPE tradeType = ETRADETYPE_TRUESELL;
  sendCmd.set_count(sellCount);

  if (type == EOperType_PublicitySellSuccess  || type == EOperType_NormalSell)
  {
    if (sellCount == 0)
      return;
    //税率   
    DWORD tax = calcTax(gainMoney, pendingInfo.price);
    
    MsgParams params;
    params.addNumber(sellCount);
    params.addString(pBase->strName);
    params.addNumber(tax);
    params.addNumber(gainMoney);
    TradeManager::getMe().sendSysMsg(getMoneyPlayer, sellerZoneId, SYS_MGS_SELL_SUCCESS, EMESSAGETYPE_FRAME, params);
    tradeType = ETRADETYPE_TRUESELL;
  }

  if (type == EOperType_PublicityBuyFail)
  {
    sendCmd.set_money2(consumeMoney);
    sendCmd.set_count(successCount);  //用来提示用的
    tradeType = ETRADETYPE_PUBLICITY_RETURN;
  }

  //log
  //出售
  string jsonStr = pb2json(pendingInfo.itemData);
  PlatLogManager::getMe().TradeLog(thisServer,
    thisServer->getPlatformID(),
    getMoneyPlayer,
    tradeType,
    pendingInfo.itemId,
    sellCount,
    pendingInfo.price,
    0,
    gainMoney,
    jsonStr,
    buyerId);

  sendCmd.set_type(type);
  sendCmd.set_total_money(gainMoney);
  sendCmd.set_itemid(pendingInfo.itemId);
  sendCmd.set_price(pendingInfo.price);
  PROTOBUF(sendCmd, send, len);
  if (thisServer->sendCmdToZone(sellerZoneId, send, len) == false)
  {
  /*  MailManager::getMe().addOfflineTradeMoney(sendCmd);
     XINF << "[交易-获得钱,卖家消息发送失败，存到离线数据] type:" << type <<"getMoneyPlayer:" << getMoneyPlayer << "sellerid:" << pendingInfo.sellerId
       << "itemid:" << pendingInfo.itemId << "count:" << sellCount << "price:" << pendingInfo.price << "收入：" << sellCount* pendingInfo.price <<
       "税后所得:" << gainMoney <<
       "consumeMoney" << consumeMoney <<
       "successCount" << successCount << XEND;*/
  }

  XINF << "[交易-获得钱] type:"<<type <<"getMoneyPlayer:" << getMoneyPlayer << "sellerid:" << pendingInfo.sellerId
    << "itemid:" << pendingInfo.itemId << "count:" << sellCount << "price:" << pendingInfo.price <<
    "收入：" << sellCount* pendingInfo.price <<
    "税后所得:" << gainMoney <<
    "consumeMoney"<<consumeMoney <<
    "successCount"<<successCount <<XEND;
}

bool TradeManager::insertSelledDbLog(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& nameList)
{  
  DWORD gainMoney = pendingInfo.count * pendingInfo.price;
  DWORD tax = 0;
  if (type == EOperType_AutoOffTheShelf)
  {
    //自动下架返还上架费
    tax = calcBoothfee(pendingInfo.price, pendingInfo.count);
  }
  else
  {
    tax = calcTax(gainMoney, pendingInfo.price);
  }

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_SELLED_LIST);
  if (!field)
  {
    XERR << "[交易-出售记录] 插入数据库出错,找不到数据表 type" << type << "sellerid" << pendingInfo.sellerId << "buyerid" << buyerId <<"itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "tax" << tax << XEND;
    return false;
  }
  DWORD tradeTime = now();
  xRecord record(field);
  record.put("status", ETakeStatus_CanTakeGive);
  record.put("logtype", type);
  record.put("itemid", pendingInfo.itemId);
  record.put("price", pendingInfo.price);
  record.put("count", pendingInfo.count);
  record.put("sellerid", pendingInfo.sellerId);
  record.put("buyerid", buyerId);
  record.put("pendingtime", pendingInfo.pendingTime);
  record.put("tradetime", tradeTime);
  record.put("refine_lv", pendingInfo.refinelv());
  record.put("tax", tax);
  bool damage = pendingInfo.damage();
  record.put("damage", damage);
  {
    string strItemData;
    pendingInfo.marshal(strItemData);
    if (!strItemData.empty())
      record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());
  }
  {
    string strNameList;
    nameList.SerializeToString(&strNameList);
    if (!strNameList.empty())
      record.putBin("buyerinfo", (unsigned char*)strNameList.c_str(), strNameList.size());
  }

  QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-出售记录] 插入数据库出错 type" <<type<<"sellerid"<<pendingInfo.sellerId<<"itemid"<<pendingInfo.itemId<<"count"<<pendingInfo.count<<"price"<<pendingInfo.price<<"tax"<<tax<< "namelist" << nameList.ShortDebugString() << XEND;
    return false;
  }

  TradeManager::getMe().ListNtf(pendingInfo.sellerId, ELIST_NTF_MY_LOG);
  {
    TradeUser* pUser = TradeUserMgr::getMe().getTradeUser(pendingInfo.sellerId);
    if (pUser)
    {
      UpdateTradeLogCmd  updateCmd;
      updateCmd.set_charid(pendingInfo.sellerId);
      updateCmd.set_type(type);
      updateCmd.set_id(ret);
      PROTOBUF(updateCmd, send, len);
      thisServer->sendCmdToZone(pUser->zoneid(), send, len);
    }
  }

  //XINF << "[交易-出售记录] 插入数据库成功 type" << type << "sellerid" << pendingInfo.sellerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "tax" << tax <<  "数据库id" <<ret << "namelist" << nameList.ShortDebugString() << XEND;
  XINF << "[交易-出售记录] 插入数据库成功 type" << type << "sellerid" << pendingInfo.sellerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "tax" << tax << "数据库id" << ret << XEND;
  return true;
  return true;
}

bool TradeManager::insertSelledDbLog2(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& nameList, xRecordSet& recordSet)
{

  DWORD gainMoney = pendingInfo.count * pendingInfo.price;
  DWORD tax = calcTax(gainMoney, pendingInfo.price);

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_SELLED_LIST);
  if (!field)
  {
    XERR << "[交易-出售记录] 插入数据库出错,找不到数据表 type" << type << "sellerid" << pendingInfo.sellerId << "buyerid" << buyerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "tax" << tax << XEND;
    return false;
  }
  DWORD tradeTime = now();
  xRecord record(field);
  record.put("status", ETakeStatus_CanTakeGive);
  record.put("logtype", type);
  record.put("itemid", pendingInfo.itemId);
  record.put("price", pendingInfo.price);
  record.put("count", pendingInfo.count);
  record.put("sellerid", pendingInfo.sellerId);
  record.put("buyerid", buyerId);
  record.put("pendingtime", pendingInfo.pendingTime);
  record.put("tradetime", tradeTime);
  record.put("refine_lv", pendingInfo.refinelv());
  record.put("tax", tax);
  bool damage = pendingInfo.damage();
  record.put("damage", damage);
  {
    string strItemData;
    pendingInfo.marshal(strItemData);
    if (!strItemData.empty())
      record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());
  }
  {
    string strNameList;
    nameList.SerializeToString(&strNameList);
    if (!strNameList.empty())
      record.putBin("buyerinfo", (unsigned char*)strNameList.c_str(), strNameList.size());
  }

  recordSet.push(record);
  
  XINF << "[交易-出售记录] 插入数据库成功 type" << type << "sellerid" << pendingInfo.sellerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "tax" << tax << "数据库id" << 0 << XEND;

  static DWORD MAX_SET_COUNT = 500;
  if (recordSet.size() > MAX_SET_COUNT)
  {
    //insert
    TradeManager::getMe().insertDbLogSet(recordSet);
  }

  TradeManager::getMe().ListNtf(pendingInfo.sellerId, ELIST_NTF_MY_LOG);

  return true;
}

bool TradeManager::insertDbLogSet(xRecordSet& recordSet)
{
  if (recordSet.empty())
    return true;

  QWORD retcode = thisServer->getTradeConnPool().exeInsertSet(recordSet);
  if (retcode == QWORD_MAX)
  {
    XERR << "[交易-出售购买记录插入] set"<< recordSet[0].m_field->m_strTable << "size:" << recordSet.size() << XEND;
    recordSet.clear();
    return false;
  }
  else
  {
    XLOG << "[交易-出售购买记录插入] set" << recordSet[0].m_field->m_strTable << "size:" << recordSet.size() << XEND;
    recordSet.clear();
  }

  return true;
}

bool  TradeManager::insertBuyedDbLog(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& nameList,DWORD failCount, DWORD endTime, DWORD totalBuyCount)
{

  {
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MIN_TRADE_PRICE, pendingInfo.itemId, 0, 0, pendingInfo.price);
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_AVG_TRADE_PRICE, pendingInfo.itemId, 0, 0, pendingInfo.price);
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MAX_TRADE_PRICE, pendingInfo.itemId, 0, 0, pendingInfo.price);
  }

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
  if (!field)
  {
    XERR << "[交易-购买记录] 插入数据库失败 type" << type << "buyerid" << buyerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << XEND;
    return false;
  }
  xRecord record(field);

  DWORD curSec = now();

  record.put("status", ETakeStatus_CanTakeGive);
  record.put("publicity_id", pendingInfo.publicityId);
  record.put("logtype", type);
  record.put("itemid", pendingInfo.itemId);
  record.put("price", pendingInfo.price);
  record.put("count", pendingInfo.count);
  record.put("buyerid", buyerId);
  record.put("tradetime", curSec);
  // if (!name.empty())
  // {
  //   record.putString("buyername", name);
  // }
  record.put("refine_lv", pendingInfo.refinelv());
  record.put("damage", pendingInfo.damage());
  record.put("failcount", failCount);
  record.put("totalcount", totalBuyCount);
  record.put("endtime", endTime);

  std::string strItemData;
  pendingInfo.marshal(strItemData);
  if (!strItemData.empty())
    record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());
  
  {
    string strNameList;
    nameList.SerializeToString(&strNameList);
    if (!strNameList.empty())
      record.putBin("sellerinfo", (unsigned char*)strNameList.c_str(), strNameList.size());
  }

  TradeManager::getMe().m_hotInfoMgr.addItem(pendingInfo.itemId, pendingInfo.count, curSec);
  QWORD ret = thisServer->getTradeConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-购买记录] 插入数据库失败 type" << type << "buyerid" << buyerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << nameList.ShortDebugString() << XEND;
    return false;
  }

  TradeManager::getMe().ListNtf(buyerId, ELIST_NTF_MY_LOG);

  //{
  //  TradeUser* pUser = TradeUserMgr::getMe().getTradeUser(buyerId);
  //  if (pUser)
  //  {
  //    UpdateTradeLogCmd  updateCmd;
  //    updateCmd.set_charid(buyerId);
  //    updateCmd.set_type(type);
  //    updateCmd.set_id(ret);
  //    PROTOBUF(updateCmd, send, len);
  //    thisServer->sendCmdToZone(pUser->zoneid(), send, len);
  //  }
  //}
  //XINF << "[交易-购买记录] 插入数据库成功 type" << type << "buyerid" << buyerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "数据库id" << ret << nameList.ShortDebugString()  << XEND;
  XINF << "[交易-购买记录] 插入数据库成功 type" << type << "buyerid" << buyerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "数据库id" << ret << XEND;
  return true;
}

bool  TradeManager::insertBuyedDbLog2(EOperType type, QWORD buyerId, const PendingInfo& pendingInfo, Cmd::NameInfoList& nameList, DWORD failCount, DWORD endTime, DWORD totalBuyCount, xRecordSet& recordSet)
{
  {
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MIN_TRADE_PRICE, pendingInfo.itemId, 0, 0, pendingInfo.price);
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_AVG_TRADE_PRICE, pendingInfo.itemId, 0, 0, pendingInfo.price);
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MAX_TRADE_PRICE, pendingInfo.itemId, 0, 0, pendingInfo.price);
  }

  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
  if (!field)
  {
    XERR << "[交易-购买记录] 插入数据库失败 type" << type << "buyerid" << buyerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << XEND;
    return false;
  }
  xRecord record(field);

  DWORD curSec = now();

  record.put("publicity_id", pendingInfo.publicityId);
  record.put("status", ETakeStatus_CanTakeGive);
  record.put("logtype", type);
  record.put("itemid", pendingInfo.itemId);
  record.put("price", pendingInfo.price);
  record.put("count", pendingInfo.count);
  record.put("buyerid", buyerId);
  record.put("tradetime", curSec);
  record.put("refine_lv", pendingInfo.refinelv());
  record.put("damage", pendingInfo.damage());
  record.put("failcount", failCount);
  record.put("totalcount", totalBuyCount);
  record.put("endtime", endTime);

  std::string strItemData;
  pendingInfo.marshal(strItemData);
  if (!strItemData.empty())
    record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());

  {
    string strNameList;
    nameList.SerializeToString(&strNameList);
    if (!strNameList.empty())
      record.putBin("sellerinfo", (unsigned char*)strNameList.c_str(), strNameList.size());
  }

  TradeManager::getMe().m_hotInfoMgr.addItem(pendingInfo.itemId, pendingInfo.count, curSec);

  recordSet.push(record);
  
  static DWORD MAX_SET_COUNT = 500;
  if (recordSet.size() > MAX_SET_COUNT)
  {
    //insert
    TradeManager::getMe().insertDbLogSet(recordSet);
  }

  TradeManager::getMe().ListNtf(buyerId, ELIST_NTF_MY_LOG);

  XINF << "[交易-购买记录] 插入数据库成功 type" << type << "buyerid" << buyerId << "itemid" << pendingInfo.itemId << "count" << pendingInfo.count << "price" << pendingInfo.price << "数据库id" << 0 << XEND;
  return true;
}


bool TradeManager::resellLock(QWORD orderId)
{
  
  DWORD curSec = now();
  DWORD expireTime = curSec + 30*60;
  auto it = m_resellLock.find(orderId);
  if (it == m_resellLock.end())
  {
    m_resellLock[orderId] = expireTime;
    return true;
  }
  
  if (curSec > it->second)
  {
    m_resellLock[orderId] = expireTime;
    return true;
  }
  
  XLOG << "[交易-重新上架] 订单已锁定 orderid" << orderId << XEND;
  return false;
}

bool TradeManager::resellUnlock(QWORD orderId)
{
  m_resellLock.erase(orderId);
  
  return true;
}

bool TradeManager::dbDelPending(QWORD orderId, QWORD charId, EOperType type)
{
   xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
   if (!field)
   {
     XERR << "[交易-删除订单] 失败， 获取不到数据库， table：" << DB_TABLE_PENDING_LIST << "orderid：" << orderId << "charId：" << charId <<XEND;
     return false;
   }
   char where[128];
   bzero(where, sizeof(where));
   snprintf(where, sizeof(where), "id=%llu", orderId);

   //直接删除掉
   QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
   if (ret == QWORD_MAX)
   {
     XERR << "[交易-删除订单] 失败， 数据库操作失败，orderid：" << orderId << "charId：" << charId << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
     return false;
   }  
   XINF << "[交易-删除订单] 成功,"<<"orderid：" << orderId << "charId：" << charId << XEND;
  return true;
}

bool TradeManager::dbDelPending(DWORD publicityId)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PENDING_LIST);
  if (!field)
  {
    XERR << "[交易-删除订单] 失败， 获取不到数据库， table：" << DB_TABLE_PENDING_LIST << "publicityId：" << publicityId << XEND;
    return false;
  }
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "publicity_id=%u", publicityId);

  //直接删除掉
  QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-删除订单] 失败， 数据库操作失败，publicityId：" << publicityId <<"database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
    return false;
  }
  XINF << "[交易-删除订单] 成功," << "publicityId：" << publicityId << XEND;
  return true;
}

bool TradeManager::dbDelPublicityBuy(QWORD orderId, QWORD charId, EOperType type)
{
   xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY_BUY);
   if (!field)
   {
     XERR << "[交易-删除公示期-买单] 失败， 获取不到数据库， table：" << DB_TABLE_PUBLICITY_BUY << "orderid：" << orderId << "charId：" << charId << XEND;
     return false;
   }
   char where[128];
   bzero(where, sizeof(where));
   snprintf(where, sizeof(where), "id=%llu", orderId);

   //直接删除掉
   QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
   if (ret == QWORD_MAX)
   {
     XERR << "[交易-删除公示期-买单] 失败， 数据库操作失败，orderid：" << orderId << "charId：" << charId << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
     return false;
   }
   XINF << "[交易-删除公示期-买单] 成功," << "orderid：" << orderId << "charId：" << charId << XEND;
  return true;
}

bool TradeManager::dbDelPublicityBuy(DWORD publicityId)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY_BUY);
  if (!field)
  {
    XERR << "[交易-删除公示期-买单] 失败， 获取不到数据库， table：" << DB_TABLE_PUBLICITY_BUY << "publicityId：" << publicityId << XEND;
    return false;
  }
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "publicity_id=%u", publicityId);

  //直接删除掉
  QWORD ret = thisServer->getTradeConnPool().exeDelete(field, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-删除公示期-买单] 失败， 数据库操作失败，publicityId：" << publicityId << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
    return false;
  }
  XINF << "[交易-删除公示期-买单] 成功," << "publicityId：" << publicityId << XEND;
  return true;
}

bool TradeManager::dbDelPublicity(QWORD orderId, EOperType type)
{
  xField *field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_PUBLICITY);
  if (!field)
  {
    XERR << "[交易-清除公示期] 失败， 获取不到数据库， table：" << DB_TABLE_PUBLICITY << "orderid：" << orderId << "type:" << type << XEND;
    return false;
  }

  xRecord record(field);
  record.put("endtime", 0);
  record.put("price", 0);
  record.put("buy_people", 0);
  char where[32] = { 0 };
  snprintf(where, sizeof(where), "id=%llu", orderId);
  QWORD ret = thisServer->getTradeConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[交易-清除公示期] 失败， 数据库操作失败，orderid：" << orderId << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
    return false;
  }
  XINF << "[交易-清除公示期] 成功," << "orderid：" << orderId << "type:" << type << XEND;
  return true;
}

QWORD TradeManager::calcRefinePrice(DWORD itemId, DWORD refineLv, bool damage)
{
  //精炼装备服务器价格算法
  const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(itemId);
  if (pItemCFG == nullptr)
  {
    XERR << "[交易-服务器价格] 失败，获取不到SItemCFG 策划表 itemid：" << itemId << XEND;
    return 0;
  }

  if (pItemCFG->eEquipType <= EEQUIPTYPE_MIN || pItemCFG->eEquipType >= EEQUIPTYPE_MAX)
  {
    XERR << "[交易-服务器价格] 失败，装备类型错误 itemid：" << itemId << "equiptype：" << pItemCFG->eEquipType << XEND;
    return 0;
  }
  
  const SExchangeItemCFG* pExchangeCfg = ItemConfig::getMe().getExchangeItemCFG(itemId);
  if (pExchangeCfg == nullptr)
    return 0;
  
  const SRefineTradeCFG* pRefineTradeCFG = ItemConfig::getMe().getRefineTradeCFG(pItemCFG->eItemType, refineLv);
  if (pRefineTradeCFG == nullptr)
  {
    XERR << "[交易-服务器价格] 失败，获取不到pRefineTradeCFG 策划表 itemtype:" << pItemCFG->eItemType << "refinelv：" << refineLv << XEND;
    return 0;
  }

  DWORD otherItemid = pRefineTradeCFG->dwItemID;
  //add price map
  auto &s = m_mapPrice[otherItemid];
  s.insert(itemId);
  XDBG << "[交易-价格关联] insert refine" << otherItemid << itemId << XEND;

  //获取低洞装备
  DWORD lowItemId = 0;
  {
    lowItemId = ItemConfig::getMe().getLowVidItemId(pItemCFG->dwVID);
    if (lowItemId == 0)
      lowItemId = itemId;
    else
    {
      auto &s = m_mapPrice[lowItemId];
      s.insert(itemId);
    }
  }

  QWORD  selfPrice = getServerPrice(itemId);
  QWORD lowPrice = getServerPrice(lowItemId);
  QWORD otherPrice = getServerPrice(otherItemid);
  QWORD newPrice = 0;

  do 
  {
    QWORD qwPriceA = 0;
    qwPriceA = lowPrice * pRefineTradeCFG->dwEquipRate;
    qwPriceA += otherPrice * pRefineTradeCFG->dwItemRate;
    qwPriceA += pRefineTradeCFG->dwLvRate;
    newPrice = qwPriceA;

    if (pExchangeCfg->dwRefineCycle == 0)
      break;

    float rate = 0;
    if (pExchangeCfg->dwRefineCycle == 1)
    {
      if (MiscConfig::getMe().getTradeCFG().dwWeekRefineRate == 0)
        break;
      rate = MiscConfig::getMe().getTradeCFG().dwWeekRefineRate / 1000.0; //千分比
    }
    else if (pExchangeCfg->dwRefineCycle == 2)
    {
      if (MiscConfig::getMe().getTradeCFG().dwMonthRefineRate == 0)
        break;
      rate = MiscConfig::getMe().getTradeCFG().dwMonthRefineRate / 1000.0;
    }
    else
      break;
    if (rate > 1)
      rate = 0;

    QWORD qwPriceB = lowPrice * pRefineTradeCFG->dwEquipRateNew;
    qwPriceB += otherPrice * pRefineTradeCFG->dwItemRateNew;
    qwPriceB += pRefineTradeCFG->dwLvRateNew;

    if (qwPriceB < qwPriceA)
      newPrice = qwPriceA - (qwPriceA - qwPriceB) * rate;
    else
      newPrice = qwPriceA + (qwPriceB - qwPriceA) * rate;
    XLOG << "[交易-精炼价格计算] itemid" << itemId << "refinelv" << refineLv <<"newprice"<< newPrice << "pricea" << qwPriceA << "priceb" << qwPriceB << "rate" << rate <<"refinecycle"<< pExchangeCfg->dwRefineCycle << XEND;
  } while (0); 

  newPrice = selfPrice + (newPrice)* pRefineTradeCFG->fRiskRate;
  if (damage)
  {
    newPrice -= lowPrice;
  }
  
  return newPrice;
}

DWORD TradeManager::calcEnchantPrice(const ItemData& rItemData)
{
  if (ItemConfig::getMe().isGoodEnchant(rItemData) == false)
    return 0;

  const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(rItemData.base().id());
  if (pItemCfg == nullptr)
    return 0;

  const SEnchantCFG* pEnchantCFG = ItemConfig::getMe().getEnchantCFG(EENCHANTTYPE_SENIOR);
  if (!pEnchantCFG)
    return 0;
  
  const SEnchantEquip* pEnchantEquip = pEnchantCFG->getEnchantEquip(pItemCfg->eItemType);
  if (pEnchantEquip == nullptr)
    return 0;
  
  const EnchantData& rEnchantData = rItemData.enchant();

  float fAll = pEnchantEquip->dwMaxWeight;
  float fRate = 1.0f;
  float fRet = 1.0f;  //平均附魔次数
  for (int i = 0; i < rEnchantData.attrs_size(); ++i)
  {
    EAttrType type = rEnchantData.attrs(i).type();
    float  value = rEnchantData.attrs(i).value();

    const SEnchantEquipItem* pCfg = pEnchantEquip->getEnchantEquipItem(type);
    if (pCfg == nullptr)
      continue;

    const SEnchantAttr* pAttrCfg = pEnchantCFG->getEnchantAttr(type, pItemCfg->eItemType);
    if (pAttrCfg == nullptr)
      continue;
    float f1 = pAttrCfg->getRandomRate();
    XDBG << "[交易-附魔] 点赞概率" << "附魔类型：" << EENCHANTTYPE_SENIOR << "attr" << type << "value" << value << "结果" << f1 << XEND;
    
    float baseRate = pCfg->p.second;
    if (baseRate == 0)
      baseRate = 100;

    float f2 = 0;
    if (fAll > 0)
    {
      f2 = baseRate / fAll;
    }
    if (fAll > baseRate)
      fAll -= baseRate;

    float f3 = (1 - f1* f2);

    XDBG << "[交易-附魔] 附魔属性概率" << "附魔类型："<< EENCHANTTYPE_SENIOR << "attr" << type << "value" << value <<"weight" <<pCfg->p.second <<"Left Weight"<<fAll <<"结果 f2" << f2 <<"f3" <<f3 << XEND;
    fRet *= f3;
    if (pCfg->p.second)
      fRate *= ItemConfig::getMe().getEnchantPriceRate(type, rItemData.base().id());
  }
  fRet = 1 - fRet;
  if (fRet > 0)
    fRet =  std::floor (1 / fRet);
  else
    fRet = 1;
  
  XDBG << "[交易-附魔] 属性随机概率" << "附魔类型：" << rItemData.enchant().type() << "fRet" << fRet <<"相性积"<<fRate << XEND;

  //特殊属性的平均附魔次数
  
  const STradeCFG& rTradeCfg = MiscConfig::getMe().getTradeCFG();
  
  DWORD extraCount = 1;
  for (int i = 0; i < rItemData.enchant().extras_size(); ++i)
  {
    const EnchantExtra& rExtra = rItemData.enchant().extras(i);
    const SEnchantCFG* pEnchantCFG = ItemConfig::getMe().getEnchantCFG(rItemData.enchant().type());
    if (!pEnchantCFG)
      break;
    const SEnchantAttr* pAttrCfg = pEnchantCFG->getEnchantAttr(rExtra.configid());
    if (!pAttrCfg)
    {
      XERR << "[交易-服务器价格-附魔] 异常，找不到 SEnchantAttr 配置。 enchant type " << rItemData.enchant().type() << "configid" << rExtra.configid() << XEND;
      break;
    }
    DWORD level = 0;  //start from 0. 0 1 2 3
    for (auto &v : pAttrCfg->vecExtraBuff)
    {
      if (v.first == rExtra.buffid())
        break;
      level++;
    }
    if (level < rTradeCfg.vecLvRate.size())
      extraCount *= rTradeCfg.vecLvRate.at(level);
  }

  DWORD mobPrice = rTradeCfg.dwMobPrice;
  float fInflation = rTradeCfg.fInflation;
  float typeRate = rTradeCfg.getTypeRate(rItemData.enchant().type());
  DWORD ret = fRet * extraCount * mobPrice * fInflation * fRate * typeRate;
  return ret;
}

DWORD TradeManager::calcUpgradePrice(DWORD itemId, DWORD lv, bool maxLv)
{  
  DWORD retPrice = 0;
  const SExchangeItemCFG* pExchangeCfg = ItemConfig::getMe().getExchangeItemCFG(itemId);
 
  if (pExchangeCfg == nullptr)
  {
    return 0;
  }
	
  if (maxLv)
    lv = pExchangeCfg->dwMaxUpgradeLv;

  auto it = pExchangeCfg->mapUpgradePriceSum.find(lv);
  if (it == pExchangeCfg->mapUpgradePriceSum.end())
  {
    return 0;
  }
  DWORD otherItemId = 0;
  DWORD price = 0;
  for (auto &v : it->second)
  {
    otherItemId = v.id();
    price = getServerPrice(otherItemId) * v.count();
    retPrice += price;

    auto &s = m_mapPrice[otherItemId];
    s.insert(itemId);
    XDBG << "[交易所-服务器价格] itemid" << itemId << "upgradelv" << lv << "otheritemid" << otherItemId << "price" << price << "count" << v.count() << "retprice" << retPrice << XEND;
  }  
  return retPrice;
}

bool TradeManager::getPriceFromCache(const Cmd::ItemData& itemData, DWORD& outPrice)
{
  outPrice = 0;
  string key = generateKey(itemData);
  auto it = m_priceCache.find(key);
  if (it == m_priceCache.end())
    return false;
  outPrice = it->second;
  XLOG << "[价格-缓存] 获取价格 key" << key << "price:" << outPrice << XEND;
  return true;
}

void TradeManager::updatePriceCache(const Cmd::ItemData& itemData, DWORD inPrice)
{
  if (inPrice == 0)return;
  string key = generateKey(itemData);
  auto it = m_priceCache.find(key);
  if (it == m_priceCache.end())
  {
    m_priceCache.insert(std::make_pair(key, inPrice));
  }
  else
  {
    if (it->second != inPrice)    
    {
      it->second = inPrice;
    }
  }
  XDBG << "[价格-缓存] 插入价格 key" << key << "price:" << inPrice << XEND;  
  return;
}

DWORD TradeManager::getMyZoneId(QWORD charId)
{
  TradeUser* pTradeUser = TradeUserMgr::getMe().getTradeUser(charId);
  if (pTradeUser)
  {
    return pTradeUser->zoneid();
  }
  else
  {
    GCharReader pDestChar(thisServer->getRegionID(), charId);
    if (pDestChar.get())
    {
      return pDestChar.getZoneID();
    }
  }
  return 100010002;
}

string TradeManager::getMyName(QWORD charId)
{
  TradeUser* pTradeUser = TradeUserMgr::getMe().getTradeUser(charId);
  if (pTradeUser)
  {
    return pTradeUser->name();
  }
  else
  {
    GCharReader pDestChar(thisServer->getRegionID(), charId);
    if (pDestChar.get())
    {
      return  pDestChar.getName();
    }
  }
  return "";
}

DWORD TradeManager::getMaxPendingLimit(QWORD charId)
{
  TradeUser* pUser = TradeUserMgr::getMe().getTradeUser(charId);
  DWORD maxPendingCount = 0;
  if (pUser)
  {
    maxPendingCount = pUser->getPerValue(EPERMISSION_MAX_PENDING_LIMIT);
    XLOG << "[交易-请求出售] charid" << charId << "maxPendingCount" << maxPendingCount << XEND;
  }
  else
  {
    GCharReader pDestChar(thisServer->getRegionID(), charId);
    if (pDestChar.get())
    {
      maxPendingCount = pDestChar.getPendingLimit();
    }
  }

  if (maxPendingCount == 0)
  {
    maxPendingCount = MiscConfig::getMe().getTradeCFG().dwMaxPendingCount;
  }
  return maxPendingCount;
}

DWORD TradeManager::getReturnRate(QWORD charId)
{
  TradeUser* pUser = TradeUserMgr::getMe().getTradeUser(charId);
  DWORD rate = 0;
  if (pUser)
  {
    rate = pUser->getPerValue(EPERMISSION_RETURN_PERCENT);
  }
  else
  {
    GCharReader pDestChar(thisServer->getRegionID(), charId);
    if (pDestChar.get())
    {
      rate = pDestChar.getReturnRate();
    }
  }
  return rate;
}

string TradeManager::generateKey(const Cmd::ItemData& itemData)
{
  stringstream ss;
  ss << "itemid:" << itemData.base().id() << "refinelv:" << itemData.equip().refinelv() << "damage:" << itemData.equip().damage();
  //附魔

  ss << "enchant:";

  if (ItemConfig::getMe().isGoodEnchant(itemData))
  {
    std::map<EAttrType, float> enchantMap; //排序，附魔的属性确认去过重
    for (int i = 0; i != itemData.enchant().attrs_size(); ++i)
    {
      const EnchantAttr attr = itemData.enchant().attrs(i);
      enchantMap[attr.type()] = attr.value();
    }
    for (auto &v : enchantMap)
    {
      ss << v.first << "," << v.second;
    }
    
    std::set<DWORD> setBuffid;
    for (int i = 0; i < itemData.enchant().extras_size(); ++i)
    {
      const EnchantExtra& rExtra = itemData.enchant().extras(i);
      setBuffid.insert(rExtra.buffid());
    }
    for (auto & v : setBuffid)
    {
      ss <<"b"<< v << ",";
    }
  }
  //注意:不为0才进入，和老数据兼容。
  if (itemData.equip().lv())
    ss << "lv:" << itemData.equip().lv() << XEND;
  return ss.str();
}

DWORD TradeManager::calcTax(DWORD& gainMoney, DWORD price)
{
  DWORD taxRate = LuaManager::getMe().call<DWORD>("calcTradeTax", price);
  DWORD tax = gainMoney *((float)taxRate / 100.0);
  if (gainMoney < tax)
    gainMoney = 0;
  else
    gainMoney -= tax; //扣税
  return tax;
}

void TradeManager::addOneSecurity(const Cmd::SecurityCmdSceneTradeCmd& rev)
{
  string key = getSecuiryKey(rev);
  auto it = m_mapSecurityCmd.find(key);
  if (it != m_mapSecurityCmd.end())
  {
    XERR << "[安全指令-添加] 失败，重复的key" << key << XEND;
    return;
  }
    
  SecurityCmd security;   
  security.init(rev);
  if (!security.toDb())
  {
    return;
  }

  //禁止购买
  if (security.m_type == ESECURITYTYPE_ALL || security.m_type == ESECURITYTYPE_BUY)
  {
    m_oPublicityMgr.securityCancelBuy(security);
  }
  
  //禁止出售
  if (security.m_type == ESECURITYTYPE_ALL || security.m_type == ESECURITYTYPE_SELL)
  {
    //处理非公示期
    StoreBaseMgr* pStoreBaseMgr = getStoreBaseMgr(security.m_dwItemId);
    if (pStoreBaseMgr)
    {
      pStoreBaseMgr->tradeSecurityCancel(security);
    }
    //处理公示期    
    m_oPublicityMgr.securityCancelSell(security);
  }

  m_mapSecurityCmd.insert(std::make_pair(key, security));
  XLOG << "[交易-安全指令-添加] 成功， key" << key << XEND;

}

void TradeManager::delOneSecurity(const Cmd::SecurityCmdSceneTradeCmd& rev)
{
  QWORD id = rev.key();
  string key;
  if (0 == id)
    key = getSecuiryKey(rev);
  else
  {
    for (auto& m : m_mapSecurityCmd)
    {
      if (m.second.m_qwId == id)
      {
        key = m.second.m_strKey;
        break;
      }
    }
  }

  if (key.empty())
  {
    XERR << "[交易-安全指令-删除] 失败， key 为空" << XEND;
    return;
  }

  auto it = m_mapSecurityCmd.find(key);
  if (it == m_mapSecurityCmd.end())
    return ;

  //del db
  if (it->second.delDb())
  {
    XLOG << "[交易-安全指令-删除] 成功， key" << key << XEND;
    m_mapSecurityCmd.erase(it);
  }
  else
    XLOG << "[交易-安全指令-删除] 失败， key" << key << XEND;
}

bool TradeManager::checkSellSecurity(QWORD charId, Cmd::SellItemRecordTradeCmd* rev)
{
  if (!rev)
    return false;

  for (auto &m : m_mapSecurityCmd)
  {
    if (m.second.m_type == ESECURITYTYPE_ALL || m.second.m_type == ESECURITYTYPE_SELL)
    {
      if (m.second.needProcess(charId, rev->item_info().itemid(),  rev->item_info().item_data().equip().refinelv()))
        return false;
    }    
  }
  return true;
}

bool TradeManager::checkBuySecurity(QWORD charId, DWORD itemId)
{
  PendingInfo info;
  info.itemId = itemId;
  return checkBuySecurity(charId, info);
}

bool TradeManager::checkBuySecurity(QWORD charId, PendingInfo& rInfo)
{
  for (auto &m : m_mapSecurityCmd)
  {
    if (m.second.m_type == ESECURITYTYPE_ALL || m.second.m_type == ESECURITYTYPE_BUY)
    {
      if (m.second.needProcess(charId, rInfo.itemId, rInfo.refinelv()))
        return false;
    }
  }
  return true;
}

string TradeManager::getSecuiryKey(const Cmd::SecurityCmdSceneTradeCmd& rev)
{
  std::stringstream ss;
  ss << rev.charid() << ";" << rev.itemid() << ";" << rev.refinelv() << ";" << rev.type() << ";";
 
  XDBG << "[交易所-安全指令] 获得key" << ss.str() << XEND;
  return ss.str();  
}

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
  TradeManager::getMe().m_oMessageStat.start(cmd, param);
}

MessageStatHelper::~MessageStatHelper()
{
  TradeManager::getMe().m_oMessageStat.end();
}
