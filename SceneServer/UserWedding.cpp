#include "UserWedding.h"
#include "SceneUser.h"
#include "MiscConfig.h"
#include "TableManager.h"
#include "WeddingConfig.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "SceneManager.h"
#include "ShopConfig.h"
#include "Menu.h"

UserWedding::UserWedding(SceneUser* pUser) :m_pUser(pUser)
{
}

UserWedding::~UserWedding()
{
}

void UserWedding::updateWeddingInfo(const Cmd::SyncWeddingInfoSCmd& rev)
{
  if (rev.has_weddinginfo())
  {
    m_oWeddingInfo = rev.weddinginfo();
  }
  else
  {
    m_oWeddingInfo.Clear();
  }

  SceneFighter* pFighter = m_pUser->getFighter(EPROFESSION_NOVICE);
  if (pFighter != nullptr)
    pFighter->getSkill().resetWeddingSkill(isMarried());
  m_pUser->getQuest().resetWeddingQuest(isMarried());
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_WEDDING);
  m_pUser->setDataMark(EUSERDATATYPE_MARITAL);
  m_pUser->refreshDataAtonce();
  m_pUser->checkWeddingBuff();

  if (m_oWeddingInfo.has_id() == false)
  {
    checkDivorceDelItem();
  }
  m_pUser->getEvent().onWeddingUpdate();

  XLOG <<"[婚礼-收到婚礼服同步数据] charid" <<m_pUser->id <<m_pUser->name <<"是否是清空" << !m_oWeddingInfo.has_id() <<rev.ShortDebugString() << XEND;
}

void UserWedding::weddingEvent(const Cmd::WeddingEventMsgCCmd& rev)
{
  XLOG <<"[婚礼-事件-更新] "<<m_pUser->id <<m_pUser->name <<"事件"<<rev.event() <<"msg"<<rev.ShortDebugString() << XEND;

  switch (rev.event())
  {
  case EWeddingEvent_Reserve:
  {
    onReserved(rev);
  }
  break;
  case EWeddingEvent_GiveupReserve:
  case EWeddingEvent_CancelReserveTimeOut:
  case EWeddingEvent_CancelReserveSys:
  {
    onGiveupReserve(rev);
  }
  break;
  case EWeddingEvent_Marry:
  {
    onMarried(rev);
  }
  break;
  case EWeddingEvent_DivorceDelChar:
  case EWeddingEvent_DivorceForce:
  case EWeddingEvent_DivorceTogether:
  case EWeddingEvent_DivorceSingle:
  {
    onDivorce(rev);
  }
  break;
  case EWeddingEvent_DelInvitation:
  {
    delInvitation(MiscConfig::getMe().getWeddingMiscCFG().dwInvitationItemID,rev.id());
  }
  default:
    break;
  }
}

//获取婚姻状态
EMARITAL UserWedding::getMaritalState()
{
  EMARITAL eMarital = EMARITAL_SINGLE;
  if (!m_oWeddingInfo.has_status())
    m_oWeddingInfo.set_status(EWeddingStatus_None);

  switch (m_oWeddingInfo.status())
  {
  case EWeddingStatus_Reserve:
    eMarital = EMARITAL_RESERVED;
    break;
  case EWeddingStatus_Married:
    eMarital = EMARITAL_MARRIED;
    break;
  case EWeddingStatus_None:
    eMarital = EMARITAL_SINGLE;
    break;
  default:
    break;
  }

  //离婚惩罚
  if (eMarital == EMARITAL_SINGLE)
  {
    DWORD t = m_pUser->getBuff().getEndTimeByID(MiscConfig::getMe().getWeddingMiscCFG().dwDivorceBuffid);
    if (t > 0)
      eMarital = EMARITAL_DIVORCE_PUNISH;
  }
  return eMarital;
}

//获取订婚对象的charid
QWORD UserWedding::getReserveParnter()
{
  if (m_oWeddingInfo.status() != EWeddingStatus_Reserve)
    return 0;
  return m_oWeddingInfo.charid1() == m_pUser->id ? m_oWeddingInfo.charid2() : m_oWeddingInfo.charid1();
}
//获取结婚对象的charid
QWORD UserWedding::getWeddingParnter()
{
  if (m_oWeddingInfo.status() != EWeddingStatus_Married)
    return 0;
  return m_oWeddingInfo.charid1() == m_pUser->id ? m_oWeddingInfo.charid2() : m_oWeddingInfo.charid1();
}

bool UserWedding::checkCanReserve(QWORD qwOtherId)
{
  //check buff
  //离婚惩罚buff
  DWORD t = m_pUser->getBuff().getEndTimeByID(MiscConfig::getMe().getWeddingMiscCFG().dwDivorceBuffid);
  if (t > 0)
  {
    DWORD leftDay = (t / 1000 / DAY_T) + 1;
    MsgParams params;
    params.addNumber(leftDay);   //剩余天数
    MsgManager::sendMsg(m_pUser->id, 9602, params);
    XERR << "[婚礼-订婚] 失败,自己处在离婚惩罚阶段" << m_pUser->name << m_pUser->name << "剩余时间，毫秒" << t << "天数" << leftDay << XEND;
    return false;
  }

  if (getMaritalState() != EMARITAL_SINGLE)
  {
    XERR << "[婚礼-订婚] 失败,自己不是单身" << m_pUser->name << m_pUser->name << "状态" << getMaritalState() << XEND;
    return false;
  }

  if (!m_pUser->m_oHands.has())
    return false;
  if (m_pUser->m_oHands.getOtherID() != qwOtherId)
    return false;

  SceneUser* pOtherUser = SceneUserManager::getMe().getUserByID(qwOtherId);
  if (!pOtherUser)
    return false;
  if (!pOtherUser->getMenu().isOpen(EMENUID_WEDDING_RESERVE))
  {
    MsgManager::sendMsg(m_pUser->id, 9641);
    XERR << "[婚礼-订婚] 失败,对方尚未开启功能" << m_pUser->name << m_pUser->name <<pOtherUser->id <<pOtherUser->name << XEND;
    return false;
  }

  if (pOtherUser->getUserWedding().getMaritalState() != EMARITAL_SINGLE)
  {
    //【sysmsg 9601】“等等，NTR是不被允许的哦！”；
    MsgManager::sendMsg(m_pUser->id, 9601);
    return false;
  }

  //离婚惩罚buff
  t = pOtherUser->getBuff().getEndTimeByID(MiscConfig::getMe().getWeddingMiscCFG().dwDivorceBuffid);
  if (t > 0)
  {
    DWORD leftDay = (t / 1000 / DAY_T) + 1;
    MsgParams params;
    params.addNumber(leftDay);   //剩余天数
    MsgManager::sendMsg(m_pUser->id, 9617, params);
    XERR << "[婚礼-订婚] 失败,对方处在离婚惩罚阶段" << m_pUser->name << m_pUser->name << "剩余时间，毫秒" << t <<"天数"<<leftDay << XEND;
    return false;
  }

  // check gender
  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
  if (rCFG.bGenderCheck && m_pUser->getUserSceneData().getGender() == pOtherUser->getUserSceneData().getGender())
  {
    MsgManager::sendMsg(m_pUser->id, 9652);
    XERR << "[婚礼-订婚] 失败,性别检查状态开启,双方性别相同" << m_pUser->name << m_pUser->name << XEND;
    return false;
  }

  m_dwReserveTime = pOtherUser->getUserWedding().m_dwReserveTime = xTime::getCurSec();
  return true;
}

bool UserWedding::hasTicket()
{
  BasePackage* pPkg = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (!pPkg)
    return false;
  bool ret = pPkg->checkItemCount(MiscConfig::getMe().getWeddingMiscCFG().dwTicketItemId);

  return ret;
}

bool UserWedding::reqReserve(Cmd::ReserveWeddingDateCCmd& rev)
{
  if (!m_pUser->getMenu().isOpen(EMENUID_WEDDING_RESERVE))
  {
    XERR << "[婚礼-订婚] 失败，功能尚未开启 " << m_pUser->id << m_pUser->name << "date" << rev.date() << "condigid" << rev.configid() << "对方" << rev.charid2() <<"menuid"<< EMENUID_WEDDING_RESERVE << XEND;
    return false;
  }

  if (!checkCanReserve(rev.charid2()))
  {
    XERR <<"[婚礼-订婚] 失败，不符合条件 "<<m_pUser->id <<m_pUser->name <<"date"<<rev.date() <<"condigid"<<rev.configid()<<"对方"<<rev.charid2() << XEND;
    return false;
  }

  //check money
  const SWeddingCFG* pCfg =WeddingConfig::getMe().getWeddingCFG(rev.configid());
  if (!pCfg)
  {
    XERR << "[婚礼-订婚] 失败，找不到配置 " << m_pUser->id << m_pUser->name << "date" << rev.date() << "condigid" << rev.configid() << "对方" << rev.charid2() << XEND;
    return false;
  }
 
  if (hasTicket())
  {
    rev.set_use_ticket(true);
  }
  else
  {
    if (!m_pUser->checkMoney(EMONEYTYPE_SILVER, pCfg->m_dwPrice))
    {
      XERR << "[婚礼-订婚] 失败，zeny不足 " << m_pUser->id << m_pUser->name << "date" << rev.date() << "condigid" << rev.configid() << "对方" << rev.charid2() << XEND;
    }
    rev.set_use_ticket(false);
  }
    
  //NtfReserveWeddingDateCCmd cmd;
  //cmd.set_date(rev.date());
  //cmd.set_configid(rev.date());
  //cmd.set_charid1(m_pUser->id);
  //cmd.set_name(m_pUser->name);
  //cmd.set_starttime(0);   //todo 配置里
  //cmd.set_endtime(0);     //todo
  //cmd.set_time(now());
  //cmd.set_sign(getSign(m_pUser->id, cmd.date(), cmd.configid(), cmd.time()));
  //PROTOBUF(cmd, send, len);
  //pOtherUser->sendCmdToMe(send, len);  
  //XLOG <<"[婚礼-预定] 发送邀请给牵手方"<<m_pUser->id <<m_pUser->name<<"date" << cmd.date() <<"configid"<< cmd.configid() <<cmd.starttime()  <<"对方id" << rev.charid2() << XEND;
  //return true;

  PROTOBUF(rev, send, len);
  bool ret  = thisServer->sendUserCmdToWeddingServer(m_pUser->id, m_pUser->name, send, len);
  XLOG << "[婚礼-订婚] 发送到婚礼服处理" << m_pUser->id << m_pUser->name << "date" << rev.date() << "condigid" << rev.configid() << "对方" << rev.charid2() << ret << XEND;
  return true;
}

void UserWedding::reserveInviteeReply(const Cmd::ReplyReserveWeddingDateCCmd& rev)
{
  string sign = WeddingConfig::getSign(rev.charid1(), rev.date(), rev.configid(), rev.time(),rev.use_ticket(),rev.zoneid());
  if (sign != rev.sign())
  {
    XERR <<"[婚礼-订婚-被邀请人回复] 校验不合法 邀请人"<<rev.charid1()<<"被邀请人" <<m_pUser->id <<"date"<<rev.date()<<rev.configid() <<rev.time() << rev.zoneid() << XEND;
    return;
  }  
  DWORD duration = MiscConfig::getMe().getWeddingMiscCFG().dwEngageInviteOverTime;
  if (now() > rev.time() + duration)
  {
    XERR << "[婚礼-订婚-被邀请人回复] 回复超时了 邀请人" << rev.charid1() << "被邀请人" << m_pUser->id << "date" << rev.date() << rev.configid() << rev.time() << rev.zoneid() << XEND;
    return;
  }
  
  SceneUser* pInviterUser = SceneUserManager::getMe().getUserByID(rev.charid1()); 
  if (!pInviterUser)
  {    
    sendReseveResultCmd2Wedding(rev.date(), rev.configid(), rev.charid1(), m_pUser->id, false, rev.zoneid());
    XERR << "[婚礼-订婚-被邀请人回复] 邀请人下线了 邀请人" << rev.charid1() << "被邀请人" << m_pUser->id << "date" << rev.date() << rev.configid() << rev.time() << rev.zoneid() << XEND;
    return;
  }

  if (rev.reply() == EReply_Refuse)
  {
    //【sysmsg 9607】“xxx拒绝了该婚期~”
    MsgParams msgParam;
    msgParam.addString(m_pUser->name);
    MsgManager::sendMsg(pInviterUser->id, 9607, msgParam);
    sendReseveResultCmd2Wedding(rev.date(), rev.configid(), rev.charid1(), m_pUser->id, false, rev.zoneid());
    XLOG << "[婚礼-订婚-被邀请人回复] 拒绝了邀请 邀请人" << rev.charid1() << "被邀请人" << m_pUser->id << "date" << rev.date() << rev.configid() << rev.time() << rev.zoneid() << XEND;
    return;
  }

  pInviterUser->getUserWedding().reserveInviterReply(rev, m_pUser->id);
}

//主邀请人收到回复
void UserWedding::reserveInviterReply(const Cmd::ReplyReserveWeddingDateCCmd& rev, QWORD qwCharid2)
{
  bool ret = false;
  DWORD ticket = 0;
  DWORD money = 0;
  do 
  {
    if (!checkCanReserve(qwCharid2))
    {
      XERR << "[婚礼-订婚-被邀请人回复] 不符合预定条件" << rev.charid1() << "被邀请人" << qwCharid2 << "date" << rev.date() << rev.configid() << rev.time() << XEND;
      ret = false;
      break;
    }

    //check money
    const SWeddingCFG* pCfg = WeddingConfig::getMe().getWeddingCFG(rev.configid());
    if (!pCfg)
    {
      XERR << "[婚礼-订婚-被邀请人回复] 找不到配置" << rev.charid1() << "被邀请人" << qwCharid2 << "date" << rev.date() << rev.configid() << rev.time() << XEND;
      ret = false;
      break;
    }

    if (rev.use_ticket())
    {
      if (!hasTicket())
      {
        XERR << "[婚礼-订婚-被邀请人回复] 找不到券" << rev.charid1() << "被邀请人" << qwCharid2 << "date" << rev.date() << rev.configid() << rev.time() << XEND;
        ret = false;
        break;
      }

      BasePackage* pPkg = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
      if (!pPkg)
        return;
      ticket = MiscConfig::getMe().getWeddingMiscCFG().dwTicketItemId;
      pPkg->reduceItem(ticket, ESOURCE_RESERVE_WEDDING);
      ret = true;
    }
    else
    {
      if (!m_pUser->subMoney(EMONEYTYPE_SILVER, pCfg->m_dwPrice, ESOURCE_RESERVE_WEDDING))
      {
        XERR << "[婚礼-订婚-被邀请人回复] 扣除zeny失败" << rev.charid1() << "被邀请人" << qwCharid2 << "date" << rev.date() << rev.configid() << rev.time() << XEND;
        ret = false;
        break;
      }
      ret = true;
      money = pCfg->m_dwPrice;
    }
  } while (0);
  
  sendReseveResultCmd2Wedding(rev.date(), rev.configid(), rev.charid1(), qwCharid2, ret, rev.zoneid(), ticket,money);
  if (ret)
  {
    XLOG << "[婚礼-订婚-被邀请人回复] 成功，发送到婚礼服处理" << rev.charid1() << "被邀请人" << qwCharid2 << "date" << rev.date() << rev.configid() << rev.time() << "ticket" << rev.use_ticket() << ticket << "zeny" << money << rev.zoneid() << XEND;
  }   
}

void UserWedding::sendReseveResultCmd2Wedding(DWORD dwDate, DWORD dwCondigId, QWORD qwCharid1, QWORD qwCharid2, bool bSuccess,DWORD dwZoneId, DWORD ticket/*=0*/, DWORD money/*=0*/)
{
  ReserveWeddingResultSCmd cmd;
  cmd.set_date(dwDate);
  cmd.set_configid(dwCondigId);
  cmd.set_charid1(qwCharid1);
  cmd.set_charid2(qwCharid2);
  cmd.set_success(bSuccess);
  cmd.set_ticket(ticket);
  cmd.set_money(money);
  cmd.set_zoneid(dwZoneId);
  PROTOBUF(cmd, send, len);
  thisServer->sendSCmdToWeddingServer(m_pUser->id,m_pUser->name, send, len);
}

//订婚成功
void UserWedding::onReserved(const Cmd::WeddingEventMsgCCmd& rev)
{
  //发送接收手册
  ItemData item;
  item.mutable_wedding()->set_id(rev.id());
  item.mutable_wedding()->set_zoneid(m_oWeddingInfo.zoneid());
  item.mutable_wedding()->set_myname(m_pUser->name);
  item.mutable_wedding()->set_partnername(m_pUser->id == m_oWeddingInfo.charid1() ? m_oWeddingInfo.manual().name2() : m_oWeddingInfo.manual().name1());
  item.mutable_wedding()->set_starttime(m_oWeddingInfo.starttime());
  item.mutable_wedding()->set_endtime(m_oWeddingInfo.endtime());
  item.mutable_base()->set_id(MiscConfig::getMe().getWeddingMiscCFG().dwWeddingManualId);
  item.mutable_base()->set_count(1);
  m_pUser->getPackage().addItem(item, EPACKMETHOD_NOCHECK);
  m_pUser->getAchieve().onWedding(EACHIEVECOND_WEDDING_RESERVE);

  XLOG << "[婚礼-事件-订婚成功] " << m_pUser->id << m_pUser->name <<"结婚手册id"  << MiscConfig::getMe().getWeddingMiscCFG().dwWeddingManualId << XEND;
}
//放弃订婚成功
void UserWedding::onGiveupReserve(const Cmd::WeddingEventMsgCCmd& rev)
{
  //删除结婚手册
  delManualItem(MiscConfig::getMe().getWeddingMiscCFG().dwWeddingManualId);

  XLOG << "[婚礼-事件-放弃订婚] " << m_pUser->id << m_pUser->name << XEND;
}
//结婚成功
void UserWedding::onMarried(const Cmd::WeddingEventMsgCCmd& rev)
{  
  XLOG << "[婚礼-事件-结婚] " << m_pUser->id << m_pUser->name << XEND;
  
  //刷新离婚过山车状态
  m_pUser->getUserSceneData().setDivorceRollerCoaster(false);

  //删除结婚手册
  delManualItem(MiscConfig::getMe().getWeddingMiscCFG().dwWeddingManualId);

  //刷新buff
  m_pUser->checkWeddingBuff();

  // checkmenu
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_WEDDING);
}
//离婚成功
void UserWedding::onDivorce(const Cmd::WeddingEventMsgCCmd& rev)
{
  if (rev.event() == EWeddingEvent_DivorceForce)
  {
    if (m_pUser->id == rev.opt_charid())
    {
      m_pUser->getBuff().add(MiscConfig::getMe().getWeddingMiscCFG().dwDivorceBuffid, m_pUser);
      XLOG << "[婚礼-强制离婚] 添加惩罚buff" << m_pUser->id << m_pUser->name << "buffid" << MiscConfig::getMe().getWeddingMiscCFG().dwDivorceBuffid << XEND;
    }
  }

  m_pUser->getAchieve().onWedding(EACHIEVECOND_WEDDING_DIVORCE, TVecQWORD{rev.event() == EWeddingEvent_DivorceForce ? 1UL : 0UL});

  //TODO 删除结婚技能

  //刷新离婚过山车状态
  m_pUser->getUserSceneData().setDivorceRollerCoaster(false);

  //刷新buff
  m_pUser->checkWeddingBuff();

  XLOG << "[婚礼-事件-离婚] " << m_pUser->id << m_pUser->name <<"离婚类型" <<rev.event() << XEND;
}

bool UserWedding::hasPlan(EWeddingPlanType type)
{
  if (m_oWeddingInfo.has_manual() == false)
    return false;
  const Cmd::WeddingManualInfo& manual = m_oWeddingInfo.manual();
  for (int i = 0; i < manual.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* cfg = WeddingConfig::getMe().getWeddingServiceCFG(manual.serviceids(i));
    if (cfg && cfg->hasPlan(type))
      return true;
  }
  return false;
}

bool UserWedding::enterRollterCoaster()
{
  QWORD qwOtherId = getWeddingParnter();
  if (qwOtherId == 0)
    return false;
  
  if (!m_pUser->m_oHands.has())
    return false;
  if (m_pUser->m_oHands.getOtherID() != qwOtherId)
    return false;
  
  if (m_pUser->getSceneTeamCnt() != 2)
    return false;
  SceneUser* pOtherUser = SceneUserManager::getMe().getUserByID(qwOtherId);
  if (!pOtherUser)
    return false;
 
  //m_pUser->gomap(MiscConfig::getMe().getWeddingMiscCFG().dwRollerCoasterMapId, GoMapType::Carrier);
  CreateDMapParams params;
  params.qwCharID = m_pUser->id;
  params.dwRaidID = MiscConfig::getMe().getWeddingMiscCFG().dwRollerCoasterMapId;
  params.vecMembers.push_back(qwOtherId);
  SceneManager::getMe().createDScene(params);

  XLOG <<"[婚礼-过山车] 请求进入过山车" <<m_pUser->id <<m_pUser->name <<"副本id" << MiscConfig::getMe().getWeddingMiscCFG().dwRollerCoasterMapId <<"对方charid"<<qwOtherId << XEND;
  return true;
}

bool UserWedding::divorceRollerCoasterInvite(Cmd::DivorceRollerCoasterInviteCCmd& rev)
{
  if (getMaritalState() != EMARITAL_MARRIED)
    return false;
  if (rev.invitee() != getWeddingParnter())
    return false;

  SceneUser* pOtherUser = SceneUserManager::getMe().getUserByID(rev.invitee());
  if (!pOtherUser)
  {
    //对方不在线
    MsgManager::sendMsg(m_pUser->id, 9624);
    return false;
  }
  
  if (m_pUser->getSceneTeamCnt() != 2)
  {
    return false;
  }

  if (!m_pUser->isMyTeamMember(pOtherUser->id))
    return false;
    
  if (m_pUser->getUserSceneData().getDivorceRollerCoaster())
  {
    if (!pOtherUser->getUserSceneData().getDivorceRollerCoaster())
    {
      pOtherUser->getUserSceneData().setDivorceRollerCoaster(true);
    }
    XERR <<"[婚礼-协议离婚-过山车邀请] 自己已经坐过过山车"<<m_pUser->id <<m_pUser->name  << XEND;
    return false;
  }
  else if (pOtherUser->getUserSceneData().getDivorceRollerCoaster())
  {
    m_pUser->getUserSceneData().setDivorceRollerCoaster(true);
    XERR << "[婚礼-协议离婚-过山车邀请] 对方已经坐过过山车" << m_pUser->id << m_pUser->name << XEND;
    return false;
  }
  
  ////check npc
  //SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  //if (!pNpc)
  //  return false;
  //if (pNpc->getNpcID() != MiscConfig::getMe().getWeddingMiscCFG().dwDivorceNpc)
  //  return false;
  //
  ////check distance
  //float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  //if (fDist > MiscConfig::getMe().getWeddingMiscCFG().dwDivorceNpcDistance)
  //{
  //  MsgManager::sendMsg(m_pUser->id, 9623);
  //  XERR << "[婚礼-协议离婚-过山车邀请] 自己距离npc过远" << m_pUser->id << m_pUser->name <<"对方"<<pOtherUser->id <<pOtherUser->name << XEND;
  //  return false;
  //}

  //fDist = ::getXZDistance(pOtherUser->getPos(), pNpc->getPos());
  //if (fDist > MiscConfig::getMe().getWeddingMiscCFG().dwDivorceNpcDistance)
  //{
  //  MsgManager::sendMsg(m_pUser->id, 9623);
  //  XERR << "[婚礼-协议离婚-过山车邀请] 对方距离npc过远" << m_pUser->id << m_pUser->name << "对方" << pOtherUser->id << pOtherUser->name << XEND;
  //  return false;
  //}
  
  DWORD curSec = now();
  if (!isLockInvite(curSec) || !pOtherUser->getUserWedding().isLockInvite(curSec))
  {
    XERR << "[婚礼-协议离婚-过山车邀请] 邀请太过频繁" << m_pUser->id << m_pUser->name << "对方" << pOtherUser->id << pOtherUser->name << XEND;
    return false;
  }
  
  rev.set_inviter(m_pUser->id);
  rev.set_inviter_name(m_pUser->name);
  PROTOBUF(rev, send, len);  
  pOtherUser->sendCmdToMe(send, len);
  lockInvite(curSec);
  pOtherUser->getUserWedding().lockInvite(curSec);
  
  XLOG << "[婚礼-协议离婚-过山车邀请] 成功发给对方" << m_pUser->id << m_pUser->name << "对方" << pOtherUser->id << pOtherUser->name << XEND;

  return true;
}

bool UserWedding::divorceRollerCoasterReply(Cmd::DivorceRollerCoasterReplyCCmd& rev)
{
  if (getMaritalState() != EMARITAL_MARRIED)
    return false;
  if (rev.inviter() != getWeddingParnter())
    return false;
  unlockInvite();  
  SceneUser* pInviterUser = SceneUserManager::getMe().getUserByID(rev.inviter());
  if (!pInviterUser)
  {
    //对方不在线
    XERR << "[婚礼-协议离婚-过山车回复] 邀请人不在线" << "被邀请人" << m_pUser->id << m_pUser->name << "对方" << rev.inviter() << XEND;
    return false;
  }
  pInviterUser->getUserWedding().unlockInvite();

  if (m_pUser->getSceneTeamCnt() != 2)
  {
    XERR << "[婚礼-协议离婚-过山车回复] 队伍不是两个人" << "被邀请人" << m_pUser->id << m_pUser->name << "对方" << rev.inviter() << XEND;
    return false;
  }

  if (!m_pUser->isMyTeamMember(pInviterUser->id))
    return false;
  return pInviterUser->getUserWedding().processDivorceRollerCoasterReply(m_pUser, rev.reply());
}

bool UserWedding::processDivorceRollerCoasterReply(SceneUser* pInviter, EReply reply)
{
  if (!pInviter)
    return false;
  
  XLOG <<"[婚礼-协议离婚-过山车回复] 被邀请人回复"<<m_pUser->id <<m_pUser->name <<"被邀请人"<<pInviter->id <<pInviter->name <<"reply"<<reply << XEND;

  if (reply == EReply_Refuse)
  {
    MsgManager::sendMsg(m_pUser->id, 9626);
    return false;
  }

  CreateDMapParams params;
  params.qwCharID = m_pUser->id;
  params.dwRaidID = MiscConfig::getMe().getWeddingMiscCFG().dwDivorceRollerCoasterMapId;
  params.vecMembers.push_back(pInviter->id);

  return SceneManager::getMe().createDScene(params);
}

bool UserWedding::isLockInvite(DWORD curSec)
{
  if (curSec > m_dwDivorceRollterCoasterReplyTime)
    return true;
  return false;
}

void UserWedding::lockInvite(DWORD curSec)
{
  m_dwDivorceRollterCoasterReplyTime = curSec + MiscConfig::getMe().getWeddingMiscCFG().dwDivorceOverTime;
}

void UserWedding::unlockInvite()
{
  m_dwDivorceRollterCoasterReplyTime = 0;
}


bool UserWedding::reqDivorce(Cmd::ReqDivorceCCmd& rev)
{
  rev.set_id(m_oWeddingInfo.id());
  if (rev.type() == EGiveUpType_Together)
    return reqTogetherDivorce(rev);
  else if (rev.type() == EGiveUpType_Single)
  {
    //单方面离婚，直接发到婚礼服处理
    PROTOBUF(rev, send, len);
    return thisServer->sendUserCmdToWeddingServer(m_pUser->id, m_pUser->name, send, len);
  }
  else if (rev.type() == EGiveUpType_Force)
  {//任务那块处理了

  }  
  return false;
}

bool UserWedding::reqTogetherDivorce(Cmd::ReqDivorceCCmd& rev)
{
  if (getMaritalState() != EMARITAL_MARRIED)
    return false;

  SceneUser* pOtherUser = SceneUserManager::getMe().getUserByID(getWeddingParnter());
  if (!pOtherUser)
  {
    MsgManager::sendMsg(m_pUser->id, 9624);
    XERR << "[婚礼-协议离婚-请求] 对方不在线" << m_pUser->id << m_pUser->name << "对方" << getWeddingParnter() << XEND;
    return false;
  }  

  if (m_pUser->getUserSceneData().getDivorceRollerCoaster())
  {
    if (!pOtherUser->getUserSceneData().getDivorceRollerCoaster())
    {
      pOtherUser->getUserSceneData().setDivorceRollerCoaster(true);
    }   
  }
  else if (pOtherUser->getUserSceneData().getDivorceRollerCoaster())
  {
    m_pUser->getUserSceneData().setDivorceRollerCoaster(true);
  }
  
  if (!m_pUser->getUserSceneData().getDivorceRollerCoaster())
  {
    XERR << "[婚礼-协议离婚-请求] 没做过过山车" << m_pUser->id << m_pUser->name << "对方" << pOtherUser->id << pOtherUser->name << XEND;
    return false;
  }  
  setTogetherDivorce();
  
  //等待对方确认
  if (!pOtherUser->getUserWedding().getTogetherDivorce())
  {
    return true;
  }

  PROTOBUF(rev, send, len);
  thisServer->sendUserCmdToWeddingServer(m_pUser->id, m_pUser->name, send, len);
  XLOG << "[婚礼-协议离婚] 发送到婚礼服" << m_pUser->id << m_pUser->name << "id" << rev.id() << XEND;
  return true;
}

void UserWedding::delManualItem(DWORD itemId)
{
  static std::vector<EPackType> sVecPackType = { EPACKTYPE_MAIN, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_TEMP_MAIN };

  for (auto&v : sVecPackType)
  {
    m_pUser->getPackage().itemRemove(itemId, v, ESOURCE_WEDDING);
  }
}

void UserWedding::delInvitation(DWORD itemId, QWORD qwWeddingId)
{
  static std::vector<EPackType> sVecPackType = { EPACKTYPE_MAIN, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_TEMP_MAIN };  
  BasePackage* pPkg = nullptr;

  for (auto&v : sVecPackType)
  {
    pPkg = m_pUser->getPackage().getPackage(v);
    if (!pPkg)
      continue;
    const TSetItemBase rItemSet = pPkg->getItemBaseList(itemId);
    
    for (auto&s : rItemSet)
    {
      ItemWedding* pItem = dynamic_cast<ItemWedding*> (s);
      if (!pItem)
        continue;
      if (pItem->getWeddingData().id() != qwWeddingId)
        continue;
      XLOG << "[婚礼-删除婚礼邀请函]" << m_pUser->id << m_pUser->name << "道具id" <<itemId << pItem->getGUID() << "个数" << pItem->getCount() << "婚礼id" << qwWeddingId << XEND;
      pPkg->reduceItem(pItem->getGUID(), ESOURCE_WEDDING, pItem->getCount());
    }
  }
}

void UserWedding::checkDivorceDelItem()
{
  static std::vector<EPackType> sVecPackType = { EPACKTYPE_MAIN, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_TEMP_MAIN, EPACKTYPE_BARROW, EPACKTYPE_EQUIP, EPACKTYPE_FASHIONEQUIP };
  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();

  for (auto&v : sVecPackType)
  {
    m_pUser->getPackage().itemRemove(rCFG.dwWeddingCertificate, v, ESOURCE_WEDDING);
  }
}
