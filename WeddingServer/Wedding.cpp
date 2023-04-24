#include "Wedding.h"
#include "WeddingServer.h"
#include "GCharManager.h"
#include "WeddingUserMgr.h"
#include "WeddingConfig.h"
#include "MailManager.h"
#include "MiscConfig.h"

Wedding::Wedding() : m_oManual(this)
{
}

Wedding::Wedding(QWORD qwId, DWORD dwZoneId, DWORD dwDate, DWORD dwConfigId, QWORD qwCharId1, QWORD qwCharId2) : m_oManual(this)
{
  m_qwId = qwId;
  m_dwZoneId = dwZoneId;
  m_dwDate = dwDate;
  m_dwConfigId = dwConfigId;
  m_qwCharId1 = qwCharId1;
  m_qwCharId2 = qwCharId2;
  m_eStatus = EWeddingStatus_Reserve;
}

Wedding::~Wedding()
{
}

void Wedding::fromRecord(const xRecord& record)
{
  m_qwId = record.get<QWORD>("id");
  m_dwZoneId = record.get<DWORD>("zoneid");
  m_dwDate = record.get<DWORD>("date");
  m_dwStartTime = record.get<DWORD>("starttime");
  m_dwEndTime = record.get<DWORD>("endtime");
  m_dwConfigId = record.get<DWORD>("configid");
  m_qwCharId1 = record.get<QWORD>("charid1");
  m_qwCharId2 = record.get<QWORD>("charid2");
  
  DWORD s = record.get<DWORD>("status");
  if (EWeddingStatus_IsValid(s))
    m_eStatus = static_cast<EWeddingStatus>(s);

  string data;
  data.assign((const char*)record.getBin("manual"), record.getBinSize("manual"));
  m_oManual.fromString(data);
}

void Wedding::toRecord(xRecord& record, bool bId/*=true*/)
{
  if (bId)
    record.put("id", m_qwId);
  record.put("zoneid", m_dwZoneId);
  record.put("date", m_dwDate);
  record.put("starttime", m_dwStartTime);
  record.put("endtime", m_dwEndTime);
  record.put("configid", m_dwConfigId);
  record.put("charid1", m_qwCharId1);
  record.put("charid2", m_qwCharId2);
  record.put("status", m_eStatus);

  string data;
  if (m_oManual.toString(data))
    record.putBin("manual", (unsigned char*)data.c_str(), data.size());
}

void Wedding::multableWeddingInfo(WeddingInfo* pInfo)
{
  if (!pInfo)
    return;
  if (m_eStatus == EWeddingStatus_None)   //删除的不发给玩家了
  {
    pInfo->Clear();
    return;
  }
  pInfo->set_id(m_qwId);
  pInfo->set_status(m_eStatus);
  pInfo->set_zoneid(m_dwZoneId);
  pInfo->set_date(m_dwDate);
  pInfo->set_configid(m_dwConfigId);
  pInfo->set_charid1(m_qwCharId1);
  pInfo->set_charid2(m_qwCharId2);
  pInfo->set_starttime(m_dwStartTime);
  pInfo->set_endtime(m_dwEndTime);
  m_oManual.toData(pInfo->mutable_manual(), false);
}

void Wedding::mutableBirefWeddingInfo(BirefWeddingInfo* pInfo)
{
  if (!pInfo)
    return;
  pInfo->set_id(m_qwId);
  pInfo->set_status(m_eStatus);
  pInfo->set_zoneid(m_dwZoneId);
  pInfo->set_starttime(m_dwStartTime);
  pInfo->set_endtime(m_dwEndTime);
  pInfo->set_can_single_divorce(m_bCanSingleDivorce);
  fillCharData();
  pInfo->mutable_char1()->CopyFrom(m_oChar1);
  pInfo->mutable_char2()->CopyFrom(m_oChar2);
}

const string& Wedding::getName(QWORD qwCharId)
{
  fillCharData();
  if (m_oChar1.charid() == qwCharId)
    return m_oChar1.name();
  else
    return m_oChar2.name();
}

void Wedding::fillCharData()
{
  auto f = [&](QWORD qwCharid, CharData& rCharData, DWORD& offlinTime)
  {
    GCharReader gChar(thisServer->getRegionID(), qwCharid);
    if (gChar.getBySocial())
    {
      rCharData.set_charid(qwCharid);
      rCharData.set_name(gChar.getName());
      rCharData.set_profession(gChar.getProfession());
      rCharData.set_gender(gChar.getGender());
      rCharData.set_portrait(gChar.getPortrait());
      rCharData.set_hair(gChar.getHair());
      rCharData.set_haircolor(gChar.getHairColor());
      rCharData.set_body(gChar.getBody());
      rCharData.set_head(gChar.getHead());
      rCharData.set_face(gChar.getFace());
      rCharData.set_mouth(gChar.getMouth());
      rCharData.set_eye(gChar.getEye());
      rCharData.set_level(gChar.getBaseLevel());
      rCharData.set_guildname(gChar.getGuildName());

      if (offlinTime == 0)
        offlinTime = gChar.getOfflineTime();
    }
  };
  
  if (!m_oChar1.charid())
  {
    f(m_qwCharId1, m_oChar1, m_dwOfflineTime1);
  }
  if (!m_oChar2.charid())
  {
    f(m_qwCharId2, m_oChar2, m_dwOfflineTime2);
  }
}

void Wedding::timeTick(DWORD curSec)
{
  if (curSec / 60 <= m_dwStartTime / 60)
  {
    auto getnextmsgtime = [&]() -> DWORD
    {
      const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
      for (auto s = rCFG.setWeddingMsgTime.rbegin(); s != rCFG.setWeddingMsgTime.rend(); ++s)
      {
        if (m_dwMsgTime == m_dwStartTime / 60 - *s)
          continue;

        if (curSec / 60 + *s < m_dwStartTime / 60)
          return m_dwStartTime / 60 - *s;
      }
      return 0;
    };

    if (m_dwMsgTime == 0)
      m_dwMsgTime = getnextmsgtime();

    if (m_dwMsgTime != 0 && curSec / 60 == m_dwMsgTime)
    {
      thisServer->sendWorldMsg(m_dwZoneId, MiscConfig::getMe().getWeddingMiscCFG().dwWeddingMsgID, MsgParams(m_oChar1.name(), m_oChar2.name()));
      m_dwMsgTime = getnextmsgtime();
    }
  }
}

bool Wedding::checkCanStart(DWORD curSec)
{
  if (curSec < m_dwEndTime && curSec >= m_dwStartTime)
  {
    if (m_eSyncState != ESyncState_StartWedding)
    {
      return true;
    }
  }
  return false;
}

bool Wedding::checkCanStop(DWORD curSec)
{
  if (m_eSyncState != ESyncState_StartWedding)
    return false;

  if (curSec > m_dwEndTime)
    return true;

  return false;
}

//婚期过了，却没有结婚成功
bool Wedding::checkTimeOut(DWORD curSec)
{
  if (m_eStatus != EWeddingStatus_Reserve)
    return false;

  if (curSec > m_dwEndTime)
  {
    return true;
  }
  return false;
}

void Wedding::processStartCurWedding()
{
  m_eSyncState = ESyncState_StartWedding;
  sendStartWeding();

  // 通知玩家婚礼开始
  WeddingUser* pUser1 = WeddingUserMgr::getMe().getWeddingUser(m_qwCharId1);
  if (pUser1)
    thisServer->sendMsg(pUser1->m_qwCharId, pUser1->m_dwZoneId, 9640);
  WeddingUser* pUser2 = WeddingUserMgr::getMe().getWeddingUser(m_qwCharId2);
  if (pUser2)
    thisServer->sendMsg(pUser2->m_qwCharId, pUser2->m_dwZoneId, 9640);

  m_oManual.onWeddingStart();
}

void Wedding::sendStartWeding()
{
  StartWeddingSCmd cmd;
  WeddingInfo* pInfo = cmd.mutable_weddinginfo();
  if (pInfo)
  {
    multableWeddingInfo(pInfo);
  }
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmdToZone(m_dwZoneId, send, len);
  XLOG << "[婚礼-开始] 同步到session" << m_dwZoneId << m_qwId << "ret" << ret << XEND;
}

void Wedding::processStopCurWedding()
{
  m_eSyncState = ESyncState_StopWedding;
  StopWeddingSCmd cmd;
  cmd.set_id(m_qwId);
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmdToZone(m_dwZoneId, send, len);
  XLOG << "[婚礼-结束] 同步到session" << m_dwZoneId << m_qwId << "ret" << ret << XEND;
}

//处理超时婚礼,bSys=true 表示是系统维护导致的超时
bool Wedding::processTimeOut(bool bSys)
{
  //del db
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_WEDDING);
  if (!field)
  {
    return false;
  }
  char where[64];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "id=%llu and status=%d", m_qwId, EWeddingStatus_Reserve);
  QWORD ret = thisServer->getDBConnPool().exeDelete(field, where);
  if (ret != 1)
  {
    XERR << "[婚礼-删除过期订婚] 失败，操作数据库失败，id" <<m_qwId <<m_qwCharId1 <<m_qwCharId2 <<"ret"<< ret << XEND;
    return false;
  }
  m_eStatus = EWeddingStatus_None;


  auto mailF = [&](QWORD charId, DWORD mailId)
  {
    MsgParams param;
    if (mailId == 12101)
    {
      //12101 错过婚期（玩家）', Desc = '非常抱歉，您已错过%s线%s月%s日%s:00~%s:00与%s的婚礼排期，神圣之时已经结束，下次不要再错过了哦~！
      param.addString(thisServer->getZoneName(m_dwZoneId));
      param.addNumber(xTime::getMonth(m_dwStartTime));
      param.addNumber(xTime::getDay(m_dwStartTime));
      param.addNumber(xTime::getHour(m_dwStartTime));
      param.addNumber(xTime::getHour(m_dwEndTime));
      string name = getName(getOtherCharId(charId));
      param.addString(name);
    }
    else if (mailId == 12102)
    {
      //非常抱歉，因为系统原因，您已错过%s线%s月%s日%s:00~%s:00与%s的婚礼排期。系统已补发您一张【B格猫的婚庆礼券】，您可以凭此券在幸福小姐处免费预约未来21日内一次最高档次的婚礼哦喵~
      param.addString(thisServer->getZoneName(m_dwZoneId));
      param.addNumber(xTime::getMonth(m_dwStartTime));
      param.addNumber(xTime::getDay(m_dwStartTime));
      param.addNumber(xTime::getHour(m_dwStartTime));
      param.addNumber(xTime::getHour(m_dwEndTime));
      string name = getName(getOtherCharId(charId));
      param.addString(name);
    }
    else if (mailId == 12106)
    {
      //非常抱歉，因为系统原因，您已错过%s线%s月%s日%s:00~%s:00与%s的婚礼排期。系统已补发%s一张【B格猫的婚庆礼券】，你们可以凭此券在幸福小姐处免费预约未来21日内一次最高档次的婚礼哦！喵~
      param.addString(thisServer->getZoneName(m_dwZoneId));
      param.addNumber(xTime::getMonth(m_dwStartTime));
      param.addNumber(xTime::getDay(m_dwStartTime));
      param.addNumber(xTime::getHour(m_dwStartTime));
      param.addNumber(xTime::getHour(m_dwEndTime));
      string name = getName(getOtherCharId(charId));
      param.addString(name);
      param.addString(name);
    }
    MailManager::getMe().sendMail(charId, mailId, param);
  };
  
  //发送邮件
  if (bSys)   //系统维护导致的超时
  {
    //错过婚期（系统）
    mailF(m_qwCharId1, 12102);  //在邮件里补偿了
    mailF(m_qwCharId2, 12106);
  }
  else
  {
    mailF(m_qwCharId1, 12101);
    mailF(m_qwCharId2, 12101);
  }
  
  //发送消息给双方
  bool ret1 = WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId1);
  bool ret2 = WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId2); 
  
  WeddingEventMsgCCmd eventMsg;
  if (bSys)
    eventMsg.set_event(EWeddingEvent_CancelReserveSys);
  else
    eventMsg.set_event(EWeddingEvent_CancelReserveTimeOut);
  eventMsg.set_id(m_qwId);
  eventMsg.set_charid1(m_qwCharId1);
  eventMsg.set_charid2(m_qwCharId2);
  sendWeddingMsg2Scene(m_qwCharId1, eventMsg);
  sendWeddingMsg2Scene(m_qwCharId2, eventMsg);

  //删除婚礼邀请函
  m_oManual.delInvitation();
  
  XLOG << "[婚礼-删除过期婚礼] 成功 id" << m_qwId << m_qwCharId1 << m_qwCharId2 <<"是否是维护超时" << bSys <<"通知情况" <<ret1 <<ret2 << XEND;
  return true;
}

//处理订婚
bool Wedding::processReserve(DWORD dwZoneId, DWORD dwDate, DWORD dwConfigId, QWORD qwCharId1, QWORD qwCharId2, bool bTicket)
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_WEDDING);
  if (!field)
  {
    return false;
  }
  const SWeddingCFG* pCfg = WeddingConfig::getMe().getWeddingCFG(dwConfigId);
  if (!pCfg)
    return false;

  m_dwZoneId = dwZoneId;
  m_dwDate = dwDate;
  m_dwConfigId = dwConfigId;
  m_dwStartTime = WeddingConfig::getUnixTime(dwDate, pCfg->m_dwStartHour);
  m_dwEndTime = WeddingConfig::getUnixTime(dwDate, pCfg->m_dwEndHour);
  m_qwCharId1 = qwCharId1;
  m_qwCharId2 = qwCharId2;
  m_eSyncState = ESyncState_None;
  m_eStatus = EWeddingStatus_Reserve;

  xRecord record(field);
  toRecord(record, false);
  QWORD ret = thisServer->getDBConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    //插入失败
    XERR << "[婚礼-订婚]失败，数据库插入失败" << m_qwCharId1 << "婚礼id" << m_qwId << "对方id" << m_qwCharId2 << "zoneid" << m_dwZoneId << m_dwDate << m_dwConfigId << XEND;
    return false;
  }
  m_qwId = ret;

  WeddingManager::getMe().onAddWedding(this);

  // 初始化手册
  m_oManual.init(bTicket);

  //同步结婚数据变化了给玩家
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId1);
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId2);

  //发送事件消息给双方
  WeddingEventMsgCCmd eventMsg;
  eventMsg.set_event(EWeddingEvent_Reserve);
  eventMsg.set_id(m_qwId);
  eventMsg.set_charid1(m_qwCharId1);
  eventMsg.set_charid2(m_qwCharId2);
  sendWeddingMsg2Scene(m_qwCharId1, eventMsg);
  sendWeddingMsg2Scene(m_qwCharId2, eventMsg);

  XLOG << "[婚礼-订婚] 成功,操作者" << m_qwCharId1 << "婚礼id" << m_qwId << "对方id" << m_qwCharId2 <<"zoneid"<<m_dwZoneId <<m_dwDate <<m_dwConfigId << XEND;
  return true;
}
//处理放弃订婚
bool Wedding::processGiveupReserve(WeddingUser*pUser)
{
  if (!pUser)
    return false;

  QWORD qwCharId = pUser->m_qwCharId;
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_WEDDING);
  if (!field)
    return false;

  char where[64] = { 0 };
  snprintf(where, sizeof(where), "id=%llu", m_qwId);

  QWORD ret = thisServer->getDBConnPool().exeDelete(field, where);
  if (ret == QWORD_MAX)
  {
    //插入失败
    XERR << "[婚礼-放弃订婚] 数据库操作失败" <<m_qwId << m_dwZoneId << m_dwDate << m_dwConfigId << m_qwCharId1 << m_qwCharId2 << XEND;
    return false;
  }
  WeddingManager::getMe().onDelWedding(this);
  //warning: 一定要放onDelWedding后面onDelWedding会用到m_eStatus
  m_eStatus = EWeddingStatus_None;

  //发送邮件给另一个人
  QWORD qwOtherCharId = getOtherCharId(qwCharId);
  //邮件id12100，您的伴侣%s已经解除了你们的订婚关系，您的婚礼排期预订失败(??_?)
  MsgParams param;
  param.addString(pUser->m_strName);
  MailManager::getMe().sendMail(qwOtherCharId, 12100, param);
  
  //同步结婚数据变化了给玩家
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId1);
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId2);

  //发送事件消息给双方
  WeddingEventMsgCCmd eventMsg;
  eventMsg.set_event(EWeddingEvent_GiveupReserve);
  eventMsg.set_id(m_qwId);
  eventMsg.set_charid1(m_qwCharId1);
  eventMsg.set_charid2(m_qwCharId2);
  sendWeddingMsg2Scene(m_qwCharId1, eventMsg);
  sendWeddingMsg2Scene(m_qwCharId2, eventMsg);

  //删除婚礼邀请函
  m_oManual.delInvitation();

  XLOG << "[婚礼-放弃婚礼] 成功,操作者" <<qwCharId <<"婚礼id"<<m_qwId <<"对方id"<< qwOtherCharId << XEND;

  //msg 9627
  thisServer->sendMsg(pUser->m_qwCharId, pUser->m_dwZoneId, 9627);
  return true;
}
//处理结婚成功
bool Wedding::processMarry(const Cmd::MarrySCmd& cmd)
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_WEDDING);
  if (!field)
    return false;
  
  char where[64] = { 0 };
  snprintf(where, sizeof(where), "id=%llu and status=%d", m_qwId, EWeddingStatus_Reserve);

  xRecord record(field);
  record.put("status", EWeddingStatus_Married);

  QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
  if (ret != 1)
  {
    //插入失败
    XERR << "[婚礼-结婚] 失败，数据库修改失败" << "婚礼id" << m_qwId << m_qwCharId1 << m_qwCharId2 << "zoneid" << m_dwZoneId << m_dwDate << m_dwConfigId << XEND;
    return false;
  }

  m_eStatus = EWeddingStatus_Married;
  
  fillCharData();
  {
    GCharWriter gChar(thisServer->getRegionID(), m_qwCharId1);
    gChar.setWeddingPartner(m_oChar2.name());
    gChar.save();
  }

  {
    GCharWriter gChar(thisServer->getRegionID(), m_qwCharId2);
    gChar.setWeddingPartner(m_oChar1.name());
    gChar.save();
  }

  //同步结婚数据变化了给玩家
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId1);
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId2);

  //发送事件消息给双方
  WeddingEventMsgCCmd eventMsg;
  eventMsg.set_event(EWeddingEvent_Marry);
  eventMsg.set_id(m_qwId);
  eventMsg.set_charid1(m_qwCharId1);
  eventMsg.set_charid2(m_qwCharId2);
  sendWeddingMsg2Scene(m_qwCharId1, eventMsg);
  sendWeddingMsg2Scene(m_qwCharId2, eventMsg);

  //同步到场景
  MarrySuccessSCmd mcmd;
  mcmd.set_weddingid(m_qwId);
  PROTOBUF(mcmd, msend, mlen);
  thisServer->sendCmdToZone(m_dwZoneId, msend, mlen);

  //删除婚礼邀请函
  m_oManual.delInvitation();

  // 保存结婚证/戒指
  TVecItemData items;
  for (int i = 0; i < cmd.items_size(); ++i)
    items.push_back(cmd.items(i));
  m_oManual.saveWeddingItem(items);

  XLOG << "[婚礼-结婚] 成功" << "婚礼id" << m_qwId << m_qwCharId1 << m_qwCharId2 << "zoneid" << m_dwZoneId << m_dwDate << m_dwConfigId << XEND;
  return true;
}
//处理离婚成功
bool Wedding::processDivorce(QWORD qwCharId, Cmd::EGiveUpType type)
{
  if (m_eStatus != EWeddingStatus_Married)
    return false;

  //check
  if (!isMine(qwCharId))
    return false;
  
  if (type == EGiveUpType_Single) //单方面的离婚
  {
    if (m_bCanSingleDivorce == false)
    {
      XERR <<"[婚礼-离婚] 失败，不可以单方面离婚 操作者"<<qwCharId <<"婚礼id"<<m_qwId << XEND;
      return false;
    }
  }
  
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_WEDDING);
  if (!field)
    return false;

  char where[64] = { 0 };
  snprintf(where, sizeof(where), "id=%llu", m_qwId);

  QWORD ret = thisServer->getDBConnPool().exeDelete(field, where);
  if (ret == QWORD_MAX)
  {
    //插入失败
    XERR << "[婚礼-放弃订婚] 数据库操作失败" << m_qwId << m_dwZoneId << m_dwDate << m_dwConfigId << m_qwCharId1 << m_qwCharId2 << XEND;
    return false;
  }
  WeddingManager::getMe().onDelWedding(this);
  //warning: 一定要放onDelWedding后面onDelWedding会用到m_eStatus
  m_eStatus = EWeddingStatus_None;

  {
    GCharWriter gChar(thisServer->getRegionID(), m_qwCharId1);
    gChar.setWeddingPartner("");
    gChar.save();
  }

  {
    GCharWriter gChar(thisServer->getRegionID(), m_qwCharId2);
    gChar.setWeddingPartner("");
    gChar.save();
  }

  //同步结婚数据变化了给玩家
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId1);
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_qwCharId2);

  //发送事件消息给双方
  WeddingEventMsgCCmd eventMsg;
  EWeddingEvent eventType = EWeddingEvent_DivorceForce;
  switch (type)
  {
  case EGiveUpType_DelChar:
    eventType = EWeddingEvent_DivorceDelChar;
    break;
  case EGiveUpType_Force:
    eventType = EWeddingEvent_DivorceForce;
    break;
  case EGiveUpType_Together:
    eventType = EWeddingEvent_DivorceTogether;
    break;
  case EGiveUpType_Single:
    eventType = EWeddingEvent_DivorceSingle;
    break;
  default:
    break;
  }
  eventMsg.set_event(eventType);
  eventMsg.set_id(m_qwId);
  eventMsg.set_charid1(m_qwCharId1);
  eventMsg.set_charid2(m_qwCharId2);
  eventMsg.set_opt_charid(qwCharId);
  sendWeddingMsg2Scene(m_qwCharId1, eventMsg);
  sendWeddingMsg2Scene(m_qwCharId2, eventMsg);

  if (type == EGiveUpType_DelChar)
  {
    //如结婚玩家有一方删除角色后，婚姻关系立刻解除。另一方玩家收到系统邮件【12105】，结婚技能删除。
    QWORD otherId = getOtherCharId(qwCharId);
    MailManager::getMe().sendMail(otherId, 12105, MsgParams(getName(qwCharId)));
  }
  else if (type == EGiveUpType_Single)
  {
    //2.另一方玩家收到系统邮件【12103】，通知结婚状态已解除。
    QWORD otherId = getOtherCharId(qwCharId);
    MailManager::getMe().sendMail(otherId, 12103, MsgParams(getName(qwCharId)));
  }

  XLOG << "[婚礼-离婚] 成功" << m_qwId << m_dwZoneId << m_dwDate << m_dwConfigId << m_qwCharId1 << m_qwCharId2 <<"操作者"<<qwCharId <<"离婚类型"<<type << XEND;
  return true;
}

bool Wedding::canGiveupReserve(QWORD qwCharId)
{
  if (m_eStatus != EWeddingStatus_Reserve)
    return false;
  //结婚期间不能取消订婚
  if (isWeddingTime())
    return false;
  if (m_qwCharId1 == qwCharId)
    return true;
  if (m_qwCharId2 == qwCharId)
    return true;

  return false;
}

void Wedding::sendWeddingMsg2Scene(QWORD qwCharId, Cmd::WeddingEventMsgCCmd& msg)
{
  msg.set_charid(qwCharId);  
  MailManager::getMe().sendWeddingEventMsgMail(qwCharId, msg);
  WeddingUser* pUser = WeddingUserMgr::getMe().getWeddingUser(qwCharId);
  if (pUser)
    pUser->ntfBriefWeddingInfo2Client();
}

bool Wedding::canMarry(QWORD charId1, QWORD charId2, DWORD zoneId)
{
  if (m_eStatus != EWeddingStatus_Reserve)
  {
    XERR << "[婚礼-结婚] 失败，不是订婚状态，id" << m_qwId << "status" << m_eStatus << "chari1" << m_qwCharId1 << "charid2" << m_qwCharId2 << "zoneid" << m_dwZoneId << charId1 << charId2 << zoneId << XEND;
    return false;
  }
  
  if (m_qwCharId1 != charId1)
  {
    XERR << "[婚礼-结婚] 失败，charid1不一致，id" << m_qwId << "status" << m_eStatus << "chari1" << m_qwCharId1 << "charid2" << m_qwCharId2 << "zoneid" << m_dwZoneId << charId1 << charId2 << zoneId << XEND;
    return false;
  }

  if (m_qwCharId2 != charId2)
  {
    XERR << "[婚礼-结婚] 失败，charid2不一致，id" << m_qwId << "status" << m_eStatus << "chari1" << m_qwCharId1 << "charid2" << m_qwCharId2 << "zoneid" << m_dwZoneId << charId1 << charId2 << zoneId << XEND;
    return false;
  }

  if (m_dwZoneId != zoneId)
  {
    XERR << "[婚礼-结婚] 失败，不在订婚的线不一致，id" << m_qwId << "status" << m_eStatus << "chari1" << m_qwCharId1 << "charid2" << m_qwCharId2 << "zoneid" << m_dwZoneId << charId1 << charId2 << zoneId << XEND;
    return false;
  }

  if (!isWeddingTime())
    return false;

  return true;
}

//可否单方面离婚
bool Wedding::canSingleDevore()
{
  if (m_eStatus != EWeddingStatus_Married)
    return false;
  DWORD curSec = now();

  auto checkf =[&](QWORD qwCharId)
  {
    DWORD offlineTime = getOfflineTime(qwCharId);
    if (curSec <= offlineTime)
      return false;
    DWORD  diffDay = (curSec - offlineTime) / DAY_T;
    if (diffDay >= 14)
      return true;
    return false;
  };  
  if (checkf(m_qwCharId1))
    return true;
  if (checkf(m_qwCharId2))
    return true;
  return false;
}

void Wedding::onUserOnline(WeddingUser* pUser)
{
  if (!pUser)
    return;
  setOfflineTime(pUser->m_qwCharId);
  m_bCanSingleDivorce = canSingleDevore();
}

void Wedding::onUserOffline(QWORD qwCharId)
{ 
  m_oManual.onUserOffline(qwCharId);
  
  setOfflineTime(qwCharId);
}

void Wedding::setOfflineTime(QWORD qwCharId)
{
  if (qwCharId == m_qwCharId1)
    m_dwOfflineTime1 = now();
  else
    m_dwOfflineTime2 = now();
}

DWORD Wedding::getOfflineTime(QWORD qwCharId)
{
  if (qwCharId == m_qwCharId1)
  {
    if (m_dwOfflineTime1 == 0)
    {
      fillCharData();
    }
    return m_dwOfflineTime1;
  }
  else
  {
    if (m_dwOfflineTime2 == 0)
    {
      fillCharData();
    }
    return m_dwOfflineTime2;
  }
  return 0;
}

bool Wedding::isWeddingTime(DWORD curSec /*= 0*/)
{
  if (curSec == 0)
    curSec = now();
  if (curSec >= m_dwStartTime && curSec < m_dwEndTime)
    return true;
  return false;
}

bool Wedding::rename(QWORD charid, const string& name)
{
  if (m_qwCharId1 == charid)
  {
    m_oChar1.set_name(name);
    m_oManual.rename1(name);

    GCharWriter gChar(thisServer->getRegionID(), m_qwCharId2);
    gChar.setWeddingPartner(name);
    gChar.save();
  }
  else if (m_qwCharId2 == charid)
  {
    m_oChar2.set_name(name);
    m_oManual.rename2(name);

    GCharWriter gChar(thisServer->getRegionID(), m_qwCharId1);
    gChar.setWeddingPartner(name);
    gChar.save();
  }
  else
  {
    XERR << "[婚礼-更名] charid :" << charid << "更名为" << name << "失败,charid 不属于婚姻双方id" << m_qwCharId1 << m_qwCharId2 << XEND;
    return false;
  }

  WeddingUser* pWeddingUser = WeddingUserMgr::getMe().getWeddingUser(m_qwCharId1);
  if (pWeddingUser != nullptr)
  {
    WeddingUserMgr::getMe().syncWeddingInfo2Scene(pWeddingUser->m_qwCharId, pWeddingUser->m_dwZoneId);
    pWeddingUser->ntfBriefWeddingInfo2Client();
  }
  pWeddingUser = WeddingUserMgr::getMe().getWeddingUser(m_qwCharId2);
  if (pWeddingUser != nullptr)
  {
    WeddingUserMgr::getMe().syncWeddingInfo2Scene(pWeddingUser->m_qwCharId, pWeddingUser->m_dwZoneId);
    pWeddingUser->ntfBriefWeddingInfo2Client();
  }

  XLOG << "[婚礼-更名] charid :" << charid << "更名为" << name << "成功" << XEND;
  return true;
}

void Wedding::updateRecord()
{
  if (m_bitsetRecord.none())
    return;

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_WEDDING);
  if (!field)
    return;

  xRecord record(field);

  for (int i = 0; i < EWEDDINGDATA_MAX; ++i)
  {
    if (!m_bitsetRecord.test(i))
      continue;
    EWeddingData type = static_cast<EWeddingData>(i);
    switch (type)
    {
    case EWEDDINGDATA_MANUAL:
    {
      string data;
      if (m_oManual.toString(data))
        record.putBin("manual", (unsigned char*)(data.c_str()), data.size());
      break;
    }
    case EWEDDINGDATA_MIN:
    default:
      break;
    }
  }

  if (record.m_rs.empty())
    return;

  char where[64] = { 0 };
  snprintf(where, sizeof(where), "id=%llu", m_qwId);
  QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[婚礼-数据入库]" << "婚礼id" << m_qwId << m_qwCharId1 << m_qwCharId2 << "zoneid" << m_dwZoneId << m_dwDate << m_dwConfigId << "入库失败" << XEND;
    return;
  }

  m_bitsetRecord.reset();
}

