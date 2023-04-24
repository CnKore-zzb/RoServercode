#include "GlobalShopMgr.h"
#include "ShopConfig.h"
#include "MatchServer.h"
#include "xDBConnPool.h"
#include "MatchManager.h"

GlobalShopMgr::GlobalShopMgr()
{
}
GlobalShopMgr::~GlobalShopMgr()
{
}

void GlobalShopMgr::timeTick(DWORD curTime)
{
  if (curTime > m_dwNextSaveTime && m_bSaveDb)
  {
    saveDb();
    m_dwNextSaveTime = curTime + 1 * MIN_T;
  }
}

void GlobalShopMgr::loadDb()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_GLOBAL_SHOP);
  if (!field)
    return;
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, NULL, NULL);

  if (QWORD_MAX == ret)
  {
    XERR << "[全服商店-加载数据库] 失败，ret" << QWORD_MAX << XEND;
    return;
  }
 
  for (DWORD i = 0; i < ret; ++i)
  {
    DWORD id = set[i].get<DWORD>("id");
    DWORD count = set[i].get<DWORD>("count");
    SoldCnt*pItem = getMutableSoldCnt(id);
    if (!pItem)
      continue;
    pItem->dwSoldCnt = count;
  }

  XLOG << "[全服商店-加载数据库] 成功，size" << m_mapSoldCnt.size() << XEND;
  return;
}

void GlobalShopMgr::saveDb()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_GLOBAL_SHOP);
  if (!field)
    return;
  
  for (auto&m : m_mapSoldCnt)
  {
    xRecord record(field);
    record.put("id", m.first);
    record.put("count", m.second.dwSoldCnt);

    QWORD ret = thisServer->getDBConnPool().exeInsert(record, true, true);
    if (ret == QWORD_MAX)
    {
      //插入失败
      XERR << "[全服商店-插入数据库] id" << m.first << "count" << m.second.dwSoldCnt << " 插入数据库失败" << XEND;
    }
  }

  m_bSaveDb = false;

}

void GlobalShopMgr::querySoldCnt(Cmd::QuerySoldCntMatchSCmd& rev)
{
  //for (auto& m : m_mapSoldCnt)
  //{
  //  const ShopItem*pCFG = ShopConfig::getMe().getShopItem(m.first);
  //  if (!pCFG)
  //    continue;
  //  ShopSoldItem*pItem = m_cacheCmd.add_items();
  //  if (!pItem)
  //    continue;
  //  pItem->set_id(m.first);
  //  DWORD cnt = m.second.getCount();
  // if (cnt > pCFG->producenum())
  //    cnt = pCFG->producenum();
  //  pItem->set_count(cnt);
  //  pItem->set_shopid(pCFG->shopid());
  //  pItem->set_type(pCFG->shoptype());
  //}
  //
  if (!m_bInit)
    updateCache();
  if (m_bInit)
  {
    PROTOBUF(m_cacheCmd, send, len);
    thisServer->sendCmdToClient(rev.charid(), rev.zoneid(), send, len);
    XLOG << "[全服商店-出售个数] 返回给客户端" <<rev.charid() <<rev.zoneid() <<"msg"<< m_cacheCmd.ShortDebugString() << XEND;
  }
  else
  {
    XLOG << "[全服商店-出售个数] 返回给客户端" <<rev.charid() <<rev.zoneid() <<"没有出售个数" << XEND;
  }
}

void GlobalShopMgr::checkSoldCnt(Cmd::CheckCanBuyMatchSCmd& cmd)
{
  bool bSuccess = false;
  do 
  {
    const ShopItem*pCFG = ShopConfig::getMe().getShopItem(cmd.id());
    if (!pCFG)
      break;
    if (pCFG->producenum() == 0)
      break;
    
    SoldCnt*pItem = getMutableSoldCnt(cmd.id());
    if (!pItem)
      break;
    if (pItem->getCount() + cmd.count() > pCFG->producenum())
    {
      MatchManager::getMe().sendSysMsg(cmd.charid(), cmd.zoneid(), 3626); //库存不足
      break;
    }

    pItem->lockCnt(cmd.count());
    bSuccess = true;
  } while (0);
 
  cmd.set_success(bSuccess);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSceneServer(cmd.charid(), cmd.zoneid(), send, len);

  XLOG << "[全服商店-检测购买个数] 返回给客户端" << cmd.charid() << cmd.zoneid() <<"id"<<cmd.id() <<"count"<<cmd.count() <<"success"<<bSuccess << "msg" << cmd.ShortDebugString() << XEND;
}

void GlobalShopMgr::addSoldCnt(Cmd::AddBuyCntMatchSCmd& cmd)
{ 
  const ShopItem*pCFG = ShopConfig::getMe().getShopItem(cmd.id());
  if (!pCFG)
    return;
  if (pCFG->producenum() == 0)
    return;

  SoldCnt*pItem = getMutableSoldCnt(cmd.id());
  if (!pItem)
    return;
  pItem->unlockCnt(cmd.count());
  if (pItem->getCount() + cmd.count() > pCFG->producenum())
  {
    pItem->setCount(pCFG->producenum());   
  }
  else
  {
    pItem->setCount(pItem->getCount() + cmd.count());
  }
  m_bSaveDb = true;

  updateCache();
  
  //更新给客户端
  PROTOBUF(m_cacheCmd, send, len);
  for (auto&s : m_mapOpenUser)
  {
    thisServer->sendCmdToClient(s.first, s.second, send, len);
  }
  XLOG << "[全服商店-更新购买个数] " << cmd.charid() << cmd.zoneid() << "id" << cmd.id() << "count" << cmd.count() << "打开面板玩家数量" << m_mapOpenUser.size() << XEND;
}

SoldCnt* GlobalShopMgr::getMutableSoldCnt(DWORD id)
{
  auto it = m_mapSoldCnt.find(id);
  if (it != m_mapSoldCnt.end())
  {
    return &(it->second);
  }
  SoldCnt& rCnt = m_mapSoldCnt[id];
  return &rCnt;
}

void GlobalShopMgr::onOpenPanel(QWORD qwCharId, DWORD dwZoneid)
{
  m_mapOpenUser[qwCharId] = dwZoneid;
  XDBG << "[全服商店-打开] " << qwCharId << dwZoneid << m_mapOpenUser.size() << XEND;
}

void GlobalShopMgr::onClosePanel(QWORD qwCharId)
{
  m_mapOpenUser.erase(qwCharId);
  XDBG << "[全服商店-关闭] " << qwCharId << XEND;
}

void GlobalShopMgr::updateCache()
{
  m_cacheCmd.Clear();
  m_bInit = false;
  for (auto& m : m_mapSoldCnt)
  {
    const ShopItem*pCFG = ShopConfig::getMe().getShopItem(m.first);
    if (!pCFG)
      continue;
    ShopSoldItem*pItem = m_cacheCmd.add_items();
    if (!pItem)
      continue;
    pItem->set_id(m.first);
    DWORD cnt = m.second.getCount();
    if (cnt > pCFG->producenum())
      cnt = pCFG->producenum();
    pItem->set_count(cnt);
    pItem->set_shopid(pCFG->shopid());
    pItem->set_type(pCFG->shoptype());
    m_bInit =true;
  }
}
