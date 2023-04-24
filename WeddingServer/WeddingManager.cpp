#include "WeddingManager.h"
#include "WeddingServer.h"
#include "MiscConfig.h"
#include "WeddingConfig.h"
#include "WeddingUserMgr.h"
#include "Wedding.h"

/************************************************************************/
/*DayReserve                                                                      */
/************************************************************************/

DayReserve::DayReserve(ReserveMgr* pMgr):m_pReserveMgr(pMgr)
{
}

void DayReserve::init()
{
  TMapId2WeddingCFG& rMap = WeddingConfig::getMe().getAllWeddingCFG();

  for (auto& m : rMap)
  {
    WeddingOneDayInfo info;
    info.set_configid(m.second.dwId);
    info.set_starttime(WeddingConfig::getUnixTime(m_dwDate, m.second.m_dwStartHour));
    info.set_endtime(WeddingConfig::getUnixTime(m_dwDate, m.second.m_dwEndHour));
    info.set_price(m.second.m_dwPrice);
    m_mapOneDayInfo[info.configid()] = info;
  } 
}

bool DayReserve::addWedding(Wedding* pWedding)
{
  WeddingOneDayInfo* pInfo = getOneDayInfo(pWedding->getConfigId());
  if (!pInfo)
    return false;
  pInfo->set_id(pWedding->getId()); 
  m_dwReservedCnt++;
  refreshStatus();
  return true;
}

void DayReserve::delWedding(Wedding* pWedding)
{
  WeddingOneDayInfo* pInfo = getOneDayInfo(pWedding->getConfigId());
  if (!pInfo)
    return;
  pInfo->set_id(0);
  if (m_dwReservedCnt > 0)
    m_dwReservedCnt--;
  refreshStatus();
}

EDateStatus DayReserve::getStatus()
{
  return m_eStatus;
}

void DayReserve::sendWeddingOneDayList(QWORD qwCharId, DWORD dwZoneId, Cmd::ReqWeddingOneDayListCCmd& cmd)
{
  for (auto&m : m_mapOneDayInfo)
  {
    WeddingOneDayInfo* pInfo = cmd.add_info();
    if (!pInfo)
      continue;
    *pInfo = m.second;
  }
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(qwCharId, dwZoneId, send, len);
  XDBG <<"[婚礼-某天排期] "<<qwCharId <<dwZoneId <<"date"<<m_dwDate <<"msg"<<cmd.ShortDebugString() << XEND;
}

bool DayReserve::checkCanReserve(DWORD dwConfigId)
{  
  auto it = m_mapOneDayInfo.find(dwConfigId);
  if (it == m_mapOneDayInfo.end())
    return false;
  if (it->second.id())
  {
    return false;
  }

  if (isLock(dwConfigId))
    return false;
  return true;
}

bool DayReserve::isLock(DWORD dwConfigId)
{
  auto it = m_mapLock.find(dwConfigId);
  if (it == m_mapLock.end())
    return false;
  if (now() < it->second)
    return false;

  return true;
}

bool DayReserve::lock(DWORD dwConfigId)
{
  DWORD curSec = now();
  auto it = m_mapLock.find(dwConfigId);
  if (it != m_mapLock.end())
  {
    if (curSec < it->second)
      return false;
  }
  m_mapLock[dwConfigId] = curSec + MiscConfig::getMe().getWeddingMiscCFG().dwEngageInviteOverTime + 20;
  return true;
}

bool DayReserve::unlock(DWORD dwConfigId)
{
  m_mapLock.erase(dwConfigId);
  return true;
}

void DayReserve::refreshStatus()
{
  EDateStatus newStatus = EDateStatus_None;
  if (m_dwReservedCnt >= m_mapOneDayInfo.size())
  {
    newStatus = EDateStatus_Full;
  }
  else if (m_dwReservedCnt >= (m_mapOneDayInfo.size() / 2))
  {
    newStatus = EdateStatus_Hot;
  }
  if (newStatus == m_eStatus)
    return;

  m_eStatus = newStatus;
  m_pReserveMgr->setStatusRefresh();
}

WeddingOneDayInfo* DayReserve::getOneDayInfo(DWORD dwConfigId)
{
  auto it = m_mapOneDayInfo.find(dwConfigId);
  if (it == m_mapOneDayInfo.end())
    return nullptr;
  return &(it->second);
}

/************************************************************************/
/*ReserveMgr                                                                      */
/************************************************************************/
void ReserveMgr::refresh()
{
  DWORD curSec = now();
  //DWORD newStartDate = xTime::getDayStart(curSec, MiscConfig::getMe().getWeddingMiscCFG().dwEngageRefresh) + DAY_T;
  DWORD newStartDate = xTime::getDayStart(curSec, MiscConfig::getMe().getWeddingMiscCFG().dwEngageRefresh);
  if (newStartDate <= m_dwStartDate)
  {
    checkStatusRefresh();
    return;
  }
  DWORD diffDay = 0;
  if (m_dwStartDate == 0)
    diffDay = MiscConfig::getMe().getWeddingMiscCFG().dwMaxTicketReserveDay;
  else
  {
    diffDay = (newStartDate - m_dwStartDate) / DAY_T;
  }

  //del old
  if (m_dwStartDate)
  {
    for (DWORD i = 0; i < diffDay; ++i)
    {
      DWORD t = m_dwStartDate + i*DAY_T;
      m_mapDayReserve.erase(t);     
    }
  }

  //add new
  DWORD addStartDay = 0;
  if (m_dwEndDate == 0)
  {
    addStartDay = newStartDate;
  }
  else
  {
    addStartDay = m_dwEndDate + DAY_T;
  }

  for (DWORD i = 0; i < diffDay; ++i)
  {
    DWORD t = addStartDay + i*DAY_T;    
    DayReserve day(this);
    day.m_dwDate = t;
    day.init();
    m_mapDayReserve.insert(std::make_pair(t, day));
  }
  m_dwStartDate = newStartDate;
  m_dwEndDate = m_dwStartDate + MiscConfig::getMe().getWeddingMiscCFG().dwMaxTicketReserveDay* DAY_T;
  m_dwNoTicketEndDate = m_dwStartDate + MiscConfig::getMe().getWeddingMiscCFG().dwMaxReserveDay* DAY_T;
  
  XLOG << "[婚礼-婚期刷新] zoneid" <<m_dwZoneId <<"startdate"<<m_dwStartDate <<"enddate"<<m_dwEndDate <<"noticketenddate"<<m_dwNoTicketEndDate << MiscConfig::getMe().getWeddingMiscCFG().dwMaxTicketReserveDay << XEND;
  setStatusRefresh();
  checkStatusRefresh();
}

void ReserveMgr::checkStatusRefresh()
{
  if (m_bStatusRefresh == false)
    return;

  m_oDateListCmdCache.Clear();
  m_oDateListTicketCmdCache.Clear();
  m_oDateListTicketCmdCache.set_use_ticket(true);

  auto sF = [&](ReqWeddingDateListCCmd& rCmd, DWORD date, EDateStatus s)
  {
    WeddingDateStatus*pStatus = rCmd.add_date_list();
    if (pStatus)
    {
      pStatus->set_date(date);
      pStatus->set_status(s);
      pStatus->set_count(WeddingManager::getMe().getDateWeddingCount(date));
    }
  };

  for (auto&m : m_mapDayReserve)
  {
    EDateStatus s = m.second.getStatus();
    if (m.first < m_dwNoTicketEndDate)
    {
      sF(m_oDateListCmdCache, m.first, s);
    }
    sF(m_oDateListTicketCmdCache, m.first, s);
  }

  m_bStatusRefresh = false;
}

bool ReserveMgr::addWedding(Wedding* pWedding)
{
  if (!pWedding)
    return false;

  if (m_dwZoneId != pWedding->getZoneId())
    return false;
  DayReserve* pDayReserve = getDayReserve(pWedding->getDate());
  if (!pDayReserve)
    return false;
  pDayReserve->addWedding(pWedding);

  return true;
}

void ReserveMgr::delWedding(Wedding* pWedding)
{
  if (!pWedding)
    return ;
  if (m_dwZoneId != pWedding->getZoneId())
    return ;
  DayReserve* pDayReserve = getDayReserve(pWedding->getDate());
  if (!pDayReserve)
    return ;
  pDayReserve->delWedding(pWedding);
  return ;
}

DayReserve* ReserveMgr::getDayReserve(DWORD date)
{
  auto it = m_mapDayReserve.find(date);
  if (it == m_mapDayReserve.end())
    return nullptr;
  return &(it->second);
}

void ReserveMgr::sendWeddingDateList(QWORD qwCharid, DWORD dwZoneid, bool bTicket)
{
  if (bTicket)
  {
    PROTOBUF(m_oDateListTicketCmdCache, send, len);
    thisServer->sendCmdToClient(qwCharid, dwZoneid, send, len);
    XDBG <<"[婚礼-查看婚期] 有券 charid" <<qwCharid <<dwZoneid <<"msg"<< m_oDateListTicketCmdCache.ShortDebugString() << XEND;
  }
  else
  {
    PROTOBUF(m_oDateListCmdCache, send, len);
    thisServer->sendCmdToClient(qwCharid, dwZoneid, send, len);
    XDBG << "[婚礼-查看婚期] 无券 charid" << qwCharid << dwZoneid << "msg" << m_oDateListCmdCache.ShortDebugString() << XEND;
  }
}

bool ReserveMgr::checkTicket(DWORD date, bool bUseTicket)
{
  if (bUseTicket)
  {
    if (date >= m_dwEndDate)
      return false;
  }
  else
  {
    if (date >= m_dwNoTicketEndDate)
      return false;
  }
  return true;
}

/************************************************************************/
/*WeddingManager                                                                      */
/************************************************************************/
WeddingManager::WeddingManager() :m_oneSecTimer(1), m_fiveSecTimer(5), m_OneMinTimer(60), m_ThirtyMinTimer(30 * 60), m_oneHourTimer(3600), m_FiveMinTimer(5 * 60)
{
  m_oMessageStat.setFlag("婚礼服");
}

WeddingManager::~WeddingManager()
{
  for (auto it = m_mapAllWedding.begin(); it != m_mapAllWedding.end(); ++it)
  {
    if (it->second)
      SAFE_DELETE(it->second);
  }
  m_mapAllWedding.clear();
  XDBG << "WeddingManager 析构" << XEND;
}

void WeddingManager::init()
{  
  loadDb();
}

void WeddingManager::final()
{
  updateRecord(true);
}

bool WeddingManager::loadDb()
{  
  DWORD curSec = now();

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_WEDDING);
  if (!field)
  {
    XERR << "[婚礼服-初始化加载数据库] 数据库错误，找不到表" << XEND;
    return false;
  }

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, NULL, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[婚礼服-初始化加载数据库] 数据库错误" << XEND;
    return false;
  }

  for (DWORD i = 0; i < ret; ++i)
  {
    Wedding* pWedding = new Wedding();
    if (pWedding == nullptr)
      continue;
    pWedding->fromRecord(set[i]);
    if (pWedding->checkTimeOut(curSec))
    {
      pWedding->processTimeOut(true);       //停服期间过期了
      SAFE_DELETE(pWedding);
      continue;
    }
    onAddWedding(pWedding);
  }

  XLOG << "[婚礼服-初始化加载数据库] 成功加载条数" << ret << XEND;
  return true;
}

void WeddingManager::timeTick(DWORD curSec)
{
  if (m_OneMinTimer.timeUp(curSec))
    checkWedingTimeUp(curSec);
  if (m_FiveMinTimer.timeUp(curSec))
    updateRecord(false);
}

void WeddingManager::checkWedingTimeUp(DWORD curSec)
{
  //check stop
  std::set<Wedding*> setTimeOutWedding;
  for (auto it = m_mapCurWedding.begin(); it != m_mapCurWedding.end();)
  {
    Wedding* pWedding = WeddingManager::getMe().getWeddingById(*it);
    if (pWedding == nullptr)
    {
      it = m_mapCurWedding.erase(it);
      continue;
    }
    auto oldIt = it;
    ++it;
    
    if (pWedding->checkCanStop(curSec))
    {
      pWedding->processStopCurWedding();
      //婚礼期间没有结婚成功
      if (pWedding->checkTimeOut(curSec))
        setTimeOutWedding.insert(pWedding);
      m_mapCurWedding.erase(oldIt);
    }
  }
  
  //处理结婚超时的订婚
  {
    for (auto it = setTimeOutWedding.begin(); it != setTimeOutWedding.end();)
    {
      Wedding* pWedding = *it;
      onDelWedding(pWedding);
      pWedding->processTimeOut(false);
      SAFE_DELETE(pWedding);
      it = setTimeOutWedding.erase(it);
    }
  }

  //check pre start
  for (auto it = m_mapPreWedding.begin(); it != m_mapPreWedding.end();)
  {
    Wedding* pWedding = it->second;
    if (pWedding == nullptr)
    {
      it = m_mapPreWedding.erase(it);
      continue;
    }
    auto oldIt = it;
    ++it;

    pWedding->timeTick(curSec);
    
    if (pWedding->checkCanStart(curSec))
    {
      pWedding->processStartCurWedding();
      m_mapCurWedding.insert(pWedding->getId());
      m_mapPreWedding.erase(oldIt);    
    }    
  }   
}

bool WeddingManager::doUserCmd(QWORD charId, const std::string& name, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  MessageStatHelper msgStat(cmd->cmd, cmd->param);
  if (!MsgGuard::getMe().lock(cmd->param, charId))
    return true;

  switch (cmd->param)
  {
  case WEDDINGCPARAM_REQ_WEDDINGDATE_LIST:  //查看所有排期
  {
    PARSE_CMD_PROTOBUF(ReqWeddingDateListCCmd, rev);
    ReserveMgr* pReserveMgr = getMutableReserveMgr(zoneId);
    if (pReserveMgr)
    {
      pReserveMgr->sendWeddingDateList(charId, zoneId, rev.use_ticket());
    }
    return true;
  }
  break;
  case WEDDINGCPARAM_REQ_WEDDING_ONEDAY_LIST:   //查看某一天的预定
  {   
    PARSE_CMD_PROTOBUF(ReqWeddingOneDayListCCmd, rev);
    ReserveMgr* pReserveMgr = getMutableReserveMgr(zoneId);
    if (!pReserveMgr)
    {
      return false;
    }
    DayReserve*pDay = pReserveMgr->getDayReserve(rev.date());
    if (!pDay)
    {
      return false;
    }    
    pDay->sendWeddingOneDayList(charId, zoneId, rev);
    return true;
  }
  break;
  case WEDDINGCPARAM_REQ_WEDDING_INFO:            //查看某个预定id的详情
  {
    PARSE_CMD_PROTOBUF(ReqWeddingInfoCCmd, rev);
    Wedding* pWedding = getWeddingById(rev.id());
    if (!pWedding)
      return false;
    pWedding->mutableBirefWeddingInfo(rev.mutable_info());
    
    PROTOBUF(rev, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);
    XDBG << "[婚礼-查看某个预定详情] " << charId << zoneId << rev.id() << "msg" << rev.ShortDebugString() << XEND;
    return true;
  }
  break;
  case WEDDINGCPARAM_RESERVE_WEDDINGDATE:   //请求订婚，客户端->场景服->婚礼服->客户端
  {
    PARSE_CMD_PROTOBUF(ReserveWeddingDateCCmd, rev);
    WeddingUser* pUser = WeddingUserMgr::getMe().getWeddingUser(charId);
    if (!pUser)
      return true;
    if (!pUser->lockReserve())
    {
      return true;
    }
    if (reserveWedding(pUser, rev) == false)
    {
      pUser->unlockReserve();
    }
    return true;
  }
  break;
  case WEDDINGCPARAM_GIVEUP_RESERVE:   //放弃订婚,客户端->session->wedding
  {
    PARSE_CMD_PROTOBUF(GiveUpReserveCCmd, rev);
    WeddingUser* pUser = WeddingUserMgr::getMe().getWeddingUser(charId);
    if (!pUser)
      return true;
    giveUpReserve(pUser, rev);
    return true;
  }
  break;
  case WEDDINGCPARAM_UPDATE_MANUAL:
  {
    Wedding* w = WeddingManager::getMe().getWeddingByCharId(charId);
    if (!w)
      return false;
    w->getManual().handleUpdateWeddingManualCCmd(charId);
    return true;
  }
  break;
  case WEDDINGCPARAM_BUY_PACKAGE:
  {
    Wedding* w = WeddingManager::getMe().getWeddingByCharId(charId);
    if (!w)
      return false;
    PARSE_CMD_PROTOBUF(BuyWeddingPackageCCmd, rev);
    return w->getManual().buyPackage(charId, rev);
  }
  break;
  case WEDDINGCPARAM_BUY_RING:
  {
    Wedding* w = WeddingManager::getMe().getWeddingByCharId(charId);
    if (!w)
      return false;
    PARSE_CMD_PROTOBUF(BuyWeddingRingCCmd, rev);
    return w->getManual().buyRing(charId, rev);
  }
  break;
  case WEDDINGCPARAM_INVITE:
  {
    Wedding* w = WeddingManager::getMe().getWeddingByCharId(charId);
    if (!w)
      return false;
    PARSE_CMD_PROTOBUF(WeddingInviteCCmd, rev);
    TSetQWORD ids;
    for (int i = 0; i < rev.charids_size(); ++i)
      ids.insert(rev.charids(i));
    return w->getManual().invite(charId, ids);
  }
  break;
  case WEDDINGCPARAM_UPLOAD_WEDDING_PHOTO:
  {
    Wedding* w = WeddingManager::getMe().getWeddingByCharId(charId);
    if (!w)
      return false;
    PARSE_CMD_PROTOBUF(UploadWeddingPhotoCCmd, rev);
    return w->getManual().uploadPhoto(charId, rev.index(), rev.time());
  }
  break;
  case WEDDINGCPARAM_REQ_DIVORCE:
  {
    PARSE_CMD_PROTOBUF(ReqDivorceCCmd, rev);
    return divorce(charId, rev);
  }
  break;
  }
  return true;
}

bool WeddingManager::doServerCmd(QWORD charId, const std::string& name, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  MessageStatHelper msgStat(cmd->cmd, cmd->param);
  switch (cmd->param)
  {
  case WEDDINGSPARAM_RESERVE_WEDDING_RESULT:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReserveWeddingResultSCmd, rev);
    reserveWeddingSceneRes(rev);
    return true;
  }
  break; 
  case WEDDINGSPARAM_BUY_SERVICE:
  {
    PARSE_CMD_PROTOBUF(Cmd::BuyServiceWeddingSCmd, rev);
    Wedding* w = WeddingManager::getMe().getWeddingById(rev.weddingid());
    if (!w)
    {
      XERR << "[婚礼-购买服务]" << "玩家:" << rev.charid() << "婚礼:" << rev.weddingid() << "服务:" << rev.serviceid() << "cmd:" << rev.ShortDebugString() << "SceneServer处理失败" << XEND;
      return false;
    }
    w->getManual().handleBuyServiceWeddingSCmd(rev);
    return true;
  }
  break;
  case WEDDINGSPARAM_MARRY:
  {
    PARSE_CMD_PROTOBUF(Cmd::MarrySCmd, rev);
    marry(rev, zoneId);
    return true;
  }
  break;
  case WEDDINGSPARAM_CHECK_WEDDING_RESERVE:
  {
    PARSE_CMD_PROTOBUF(Cmd::CheckWeddingReserverSCmd, rev);
    Wedding* w = getWeddingById(rev.weddingid());
    if (w == nullptr)
      rev.set_result(false);
    else
      rev.set_result(w->getStatus() == EWeddingStatus_Reserve);
    PROTOBUF(rev, send, len1);
    thisServer->sendCmdToZone(zoneId, send, len1);
    return true;
  }
  break;
  case WEDDINGSPARAM_USER_RENAME:
  {
    PARSE_CMD_PROTOBUF(UserRenameWedSCmd, rev);
    Wedding* w = getWeddingById(rev.weddingid());
    if (w == nullptr)
    {
      XERR << "[婚礼-更名] charid :" << rev.charid() << "更名更新失败, weddingid :" << rev.weddingid() << "未找到该婚礼对象" << XEND;
      break;
    }
    w->rename(charId, name);
  }
  break;
  }
  return true;
}

void WeddingManager::onAddWedding(Wedding* pWedding)
{
  if (!pWedding)
    return;
  m_mapAllWedding.insert(std::make_pair(pWedding->getId(), pWedding));
  m_mapChar2Wedding.insert(std::make_pair(pWedding->getCharId1(), pWedding->getId()));
  m_mapChar2Wedding.insert(std::make_pair(pWedding->getCharId2(), pWedding->getId()));

  addDateWedding(pWedding);

  if (pWedding->getStatus() == EWeddingStatus_Reserve)
  {
    m_mapPreWedding.insert(std::make_pair(pWedding->getId(), pWedding));

    ReserveMgr* pMgr = getMutableReserveMgr(pWedding->getZoneId());
    if (pMgr)
    {
      pMgr->addWedding(pWedding);
    }  
  }
}

void WeddingManager::onDelWedding(Wedding* pWedding)
{
  if (!pWedding)
    return;
  m_mapAllWedding.erase(pWedding->getId());
  m_mapChar2Wedding.erase(pWedding->getCharId1());
  m_mapChar2Wedding.erase(pWedding->getCharId2());
  m_mapPreWedding.erase(pWedding->getId());

  removeDateWedding(pWedding);

  if (pWedding->getStatus() == EWeddingStatus_Reserve)
  {
    ReserveMgr* pMgr = getMutableReserveMgr(pWedding->getZoneId());
    if (pMgr)
    {
      pMgr->setStatusRefresh();
      pMgr->delWedding(pWedding);
    }
  }
}

Wedding* WeddingManager::getWeddingByCharId(QWORD qwCharId)
{
  auto it = m_mapChar2Wedding.find(qwCharId);
  if (it == m_mapChar2Wedding.end())
    return nullptr;
  return getWeddingById(it->second);
}

Wedding* WeddingManager::getWeddingById(QWORD qwId)
{
  auto it = m_mapAllWedding.find(qwId);
  if (it == m_mapAllWedding.end())
    return nullptr;
  return it->second;
}

ReserveMgr* WeddingManager::getMutableReserveMgr(DWORD dwZoneId)
{
  auto it = m_mapReserveMgr.find(dwZoneId);
  if (it == m_mapReserveMgr.end())
  {
    ReserveMgr mgr;
    mgr.m_dwZoneId = dwZoneId;
    it = m_mapReserveMgr.insert(std::make_pair(dwZoneId, mgr)).first;
  }
  it->second.refresh();
  return &(it->second);
}

bool WeddingManager::reserveWedding(WeddingUser*pUser, Cmd::ReserveWeddingDateCCmd& rev)
{
  if (!pUser)
    return false;

  // 只能预定明天的
  DWORD curSec = now();
  DWORD dwValidDay = xTime::getDay(now() + DAY_T);
  DWORD dwDateDay = xTime::getDay(rev.date());
  if (dwDateDay < dwValidDay)
  {
    XERR << "[婚礼-请求订婚] 失败，只能预定明天的，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }

  //每天日期刷新前两分钟不给订婚，防止订婚场景返回后找到日期，导致扣钱成功订婚失败。
  DWORD a = xTime::getDayStart(curSec) + MiscConfig::getMe().getWeddingMiscCFG().dwEngageRefresh;
  if (curSec >= (a - 2 * 60) && curSec < a)
  {
    XERR << "[婚礼-请求订婚] 失败，婚礼排期刷新前两分钟不给订婚，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }

  Wedding*pWedding = getWeddingByCharId(pUser->m_qwCharId);
  if (pWedding)
  {
    XERR << "[婚礼-请求订婚] 失败，邀请人重复结婚订婚，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId <<rev.date() <<rev.configid() <<"useticket"<<rev.use_ticket() << XEND;
    return false;
  }

  pWedding = getWeddingByCharId(rev.charid2());
  if (pWedding)
  {
    XERR << "[婚礼-请求订婚] 失败，被邀请人重复结婚订婚，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() <<pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }

  const SWeddingCFG* pCfg = WeddingConfig::getMe().getWeddingCFG(rev.configid());
  if (!pCfg)
  {
    XERR << "[婚礼-请求订婚] 失败，找不到婚期配置，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }

  ReserveMgr* pReserveMgr = getMutableReserveMgr(pUser->m_dwZoneId);
  if (!pReserveMgr)
  {
    XERR << "[婚礼-请求订婚] 失败，找不到订婚管理器，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }

  if (!pReserveMgr->checkTicket(rev.date(), rev.use_ticket()))
  {
    XERR << "[婚礼-请求订婚] 失败，超过时间，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }

  DayReserve* pDayReserve = pReserveMgr->getDayReserve(rev.date());
  if (!pDayReserve)
  {
    XERR << "[婚礼-请求订婚] 失败，找不到订婚天数，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }
    
  if (!pDayReserve->checkCanReserve(rev.configid()))
  {
    //【sysmsg 9605】“有玩家正在预订该日期”
    thisServer->sendMsg(pUser->m_qwCharId, pUser->m_dwZoneId, 9605);
    XERR << "[婚礼-请求订婚] 失败，有玩家正在预订该日期，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }
  WeddingUser* pOtherUser = WeddingUserMgr::getMe().getWeddingUser(rev.charid2());
  if (!pOtherUser)
  {
    XERR << "[婚礼-请求订婚] 失败，对方不在线，charid1" << pUser->m_qwCharId << "charid2" << rev.charid2() << pUser->m_dwZoneId << rev.date() << rev.configid() << "useticket" << rev.use_ticket() << XEND;
    return false;
  }
  //发送邀请给对方
  NtfReserveWeddingDateCCmd cmd;
  cmd.set_date(rev.date());
  cmd.set_configid(rev.configid());
  cmd.set_charid1(pUser->m_qwCharId);
  cmd.set_name(pUser->m_strName);
  cmd.set_starttime(WeddingConfig::getUnixTime(rev.date(), pCfg->m_dwStartHour));
  cmd.set_endtime(WeddingConfig::getUnixTime(rev.date(), pCfg->m_dwEndHour));
  cmd.set_time(now());
  cmd.set_use_ticket(rev.use_ticket());
  cmd.set_zoneid(pUser->m_dwZoneId);
  cmd.set_sign(WeddingConfig::getSign(cmd.charid1(), cmd.date(), cmd.configid(), cmd.time(), cmd.use_ticket(), cmd.zoneid()));
  PROTOBUF(cmd, send, len);
  if (thisServer->sendCmdToClient(pOtherUser->m_qwCharId, pOtherUser->m_dwZoneId, send, len) == false)
    return false;

  pDayReserve->lock(rev.configid());
  
  //9606等待伴侣确认排期
  thisServer->sendMsg(pUser->m_qwCharId, pUser->m_dwZoneId, 9606);
  XLOG <<"[婚礼-请求订婚] 发送邀请给牵手方"<<pUser->m_qwCharId <<pUser->m_strName<<"zoneid"<<cmd.zoneid() << "date" << cmd.date() <<"configid"<< cmd.configid() <<cmd.starttime()  <<"对方id" << rev.charid2() << "useticket" << rev.use_ticket() << XEND;
  return true;
}

//warning: rev.success == true 场景里玩家的钱已经扣除了
bool WeddingManager::reserveWeddingSceneRes(Cmd::ReserveWeddingResultSCmd& rev)
{
  WeddingUser* pUser1 = WeddingUserMgr::getMe().getWeddingUser(rev.charid1());
  if (pUser1)
  {
    pUser1->unlockReserve();
  }
  
  ReserveMgr* pReserMgr = getMutableReserveMgr(rev.zoneid());
  if (!pReserMgr)
  {
    if (rev.success())
    {
      XERR <<"[婚礼-订婚] 失败，找不到婚期管理器" <<rev.zoneid() <<rev.date()<<rev.configid()<<rev.charid1()<<rev.charid2() <<rev.money()<<rev.ticket() << XEND;
    }
    return false;    
  }
  DayReserve* pDayReserve = pReserMgr->getDayReserve(rev.date());
  if (!pDayReserve)
  {
    if (rev.success())
    {
      //把钱打印出来 
      XERR << "[婚礼-订婚] 失败，找不到婚期日期管理器" << rev.zoneid() << rev.date() << rev.configid() << rev.charid1() << rev.charid2() << rev.money() << rev.ticket() << XEND;
    }
    return false;
  }  
  pDayReserve->unlock(rev.configid());
  
  if (!rev.success())
  {
    XLOG << "[婚礼-订婚] 失败，邀请人拒绝了或者场景处理失败" << rev.zoneid() << rev.date() << rev.configid() << rev.charid1() << rev.charid2() << rev.money() << rev.ticket() << XEND;
    return false;
  }
  
  //不可能发生的情况
  if (!pDayReserve->checkCanReserve(rev.configid()))
  {
    XERR << "[婚礼-订婚] 失败，排期被预定了，异常" << rev.zoneid() << rev.date() << rev.configid() << rev.charid1() << rev.charid2() << rev.money() << rev.ticket() << XEND;
    return false;
  }

  Wedding* pWedding = new Wedding();
  if (!pWedding)
    return false;
 
  if (!pWedding->processReserve(rev.zoneid(), rev.date(), rev.configid(), rev.charid1(), rev.charid2(), rev.ticket()))
  {
    XERR << "[婚礼-订婚] 失败" << rev.zoneid() << rev.date() << rev.configid() << rev.charid1() << rev.charid2() << rev.money() << rev.ticket() << XEND;
    SAFE_DELETE(pWedding);
    return false;
  }
  //【sysmsg 9608】“您的伴侣已经同意该婚期，付款成功~！“
  if (pUser1)
  {
    thisServer->sendMsg(pUser1->m_qwCharId, pUser1->m_dwZoneId, 9608);
  }  

  pReserMgr->setStatusRefresh();
  XLOG << "[婚礼-订婚] 成功，婚礼id" <<rev.zoneid() << rev.date() << rev.configid() << rev.charid1() << rev.charid2() << rev.money() << rev.ticket() << XEND;
  return true;
}

bool WeddingManager::giveUpReserve(WeddingUser*pUser, Cmd::GiveUpReserveCCmd& rev)
{
  if (!pUser)
    return false;
  Wedding* pWedding = getWeddingById(rev.id());
  if (!pWedding)
  {
    XERR << "[婚礼-放弃订婚] 找不到婚礼数据,charid"<<pUser->m_qwCharId <<pUser->m_strName <<"婚礼id" << rev.id() << XEND;
    return false;
  }
  if (!pWedding->canGiveupReserve(pUser->m_qwCharId))
  {
    XERR << "[婚礼-放弃订婚] 订婚不可被放弃,charid" << pUser->m_qwCharId << pUser->m_strName << "婚礼id" << rev.id() << XEND;
    return false;
  }
  
  if (pWedding->processGiveupReserve(pUser) == false)
    return false;

  //删除
  SAFE_DELETE(pWedding);
  return true;
}

bool WeddingManager::marry(Cmd::MarrySCmd& rev, DWORD dwZoneId)
{
  Wedding* w = WeddingManager::getMe().getWeddingById(rev.weddingid());
  if (!w)
  {
    XERR << "[婚礼-结婚] 失败，找不到订婚数据" << "玩家:" << rev.charid1() << "婚礼:" << rev.weddingid() << "对方:" << rev.charid2() << XEND;
    return false;
  }
  if (!w->canMarry(rev.charid1(), rev.charid2(), dwZoneId))
    return false;
  return w->processMarry(rev);
}

bool WeddingManager::divorce(QWORD charId, Cmd::ReqDivorceCCmd& rev)
{
  Wedding* pW = WeddingManager::getMe().getWeddingById(rev.id());
  if (!pW)
    return false;

  if (!pW->processDivorce(charId, rev.type()))
    return false;

  SAFE_DELETE(pW);
  return true;
}

void WeddingManager::onDelChar(QWORD charId)
{
  XDBG << "[婚礼-玩家删角了] charid" << charId << XEND;

  Wedding* pW = WeddingManager::getMe().getWeddingByCharId(charId);
  if (!pW)
    return;
  
  //离婚
  if (!pW->processDivorce(charId, EGiveUpType_DelChar))
    return;

  SAFE_DELETE(pW);
}

void WeddingManager::updateRecord(bool force)
{
  xTime optTime;
  for (auto& v : m_mapAllWedding)
  {
    if (!force && optTime.uElapse() / ONE_THOUSAND > ONE_THOUSAND)
    {
      XLOG << "[婚礼管理-数据入库]" << optTime.uElapse() << "超过" << ONE_THOUSAND << "跳出循环" << XEND;
      break;
    }
    v.second->updateRecord();
  }
}

void WeddingManager::onSessionRestart(DWORD dwZoneId)
{
  for (auto&s : m_mapCurWedding)
  {
    Wedding* pWedding = getWeddingById(s);
    if (!pWedding)
      continue;
    if (pWedding->getZoneId() != dwZoneId)
      continue;

    XLOG << "[婚礼-session重启] 重新推送开始的婚礼到session" << dwZoneId << XEND;
    pWedding->sendStartWeding();
  }
}

DWORD WeddingManager::getDateWeddingCount(DWORD dwDate) const
{
  DWORD dwTime = xTime::getDayStart(dwDate);
  auto m = m_mapDateWedding.find(dwTime);
  if (m != m_mapDateWedding.end())
    return m->second.size();
  return 0;
}

bool WeddingManager::addDateWedding(Wedding* pWedding)
{
  if (pWedding == nullptr)
    return false;

  DWORD dwTime = xTime::getDayStart(pWedding->getStartTime());
  m_mapDateWedding[dwTime].insert(pWedding);

  for (auto &m : m_mapDateWedding)
    XDBG << "[婚礼-日期列表] 新增日期" << m.first << "包含" << m.second.size() << "订婚场次" << XEND;
  return true;
}

bool WeddingManager::removeDateWedding(Wedding* pWedding)
{
  if (pWedding == nullptr)
    return false;

  DWORD dwTime = xTime::getDayStart(pWedding->getStartTime());
  m_mapDateWedding[dwTime].erase(pWedding);

  for (auto &m : m_mapDateWedding)
    XDBG << "[婚礼-日期列表] 删除日期" << m.first << "包含" << m.second.size() << "订婚场次" << XEND;
  return true;
}

MessageStatHelper::MessageStatHelper(DWORD cmd, DWORD param)
{
  WeddingManager::getMe().m_oMessageStat.start(cmd, param);
}

MessageStatHelper::~MessageStatHelper()
{
  WeddingManager::getMe().m_oMessageStat.end();
}

bool MsgGuard::lock(DWORD type, QWORD charId)
{
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

      XLOG << "[婚礼-协议检测] 请求过于频繁，param:" << key << "charid:" << charId << "lasttime:" << subIt->second << XEND;
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
