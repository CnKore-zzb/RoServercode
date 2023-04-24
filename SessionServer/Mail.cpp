#include "Mail.h"
#include "SessionUser.h"
#include "RewardConfig.h"
#include "SessionServer.h"
#include "GuidManager.h"
#include "SceneTrade.pb.h"
#include "PlatLogManager.h"
#include "MsgManager.h"
#include "MailTemplateManager.h"
#include "SessionThread.h"

// mail
Mail::Mail(SessionUser* pUser) : m_pUser(pUser)
{

}

Mail::~Mail()
{

}

void Mail::onUserOnline()
{
  SessionThreadData *pData = SessionThread::getMe().create(m_pUser->id, SessionThreadAction_Mail_Load);
  if (pData)
  {
    SessionThread::getMe().add(pData);
  }
}

void Mail::onUserOffline()
{

}

bool Mail::addMail(const MailData& rData, bool addDb/*=true*/)
{
  if (addDb == false)
  {
    if (isMailValid(rData.id()) == false)
    {
      XERR << "[邮件-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << rData.ShortDebugString() << "失败,邮件已失效" << XEND;
      return false;
    }
  }

  //直接处理
  if (rData.type() == EMAILTYPE_WEDDINGMSG)
  {
    return processWeddingMsg(rData);
  }
  if (rData.type() == EMAILTYPE_USEREVENT)
  {
    return processEventMail(rData);
  }

  MailData oCopy;
  oCopy.CopyFrom(rData);
  oCopy.set_receiveid(m_pUser->id);
  if (addDb)
  {
    if (MailManager::getMe().insertNormalMailToDB(&oCopy) == false)
    {
      XERR << "[邮件-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << rData.ShortDebugString() << "失败,添加邮件失败" << XEND;
      return false;
    }
  }

  if (oCopy.sysid() != 0)
  {
    auto s = m_setSysMailIDs.find(oCopy.sysid());
    if (s != m_setSysMailIDs.end())
    {
      XERR << "[邮件-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << rData.ShortDebugString() << "失败,已领取过" << XEND;
      return false;
    }
    m_setSysMailIDs.insert(oCopy.sysid());
  }

  XLOG << "[邮件-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << rData.ShortDebugString() << "成功" << XEND;

  // insert to list and inform client
  m_mapMail[oCopy.id()].CopyFrom(oCopy);  
  if (oCopy.type() == EMAILTYPE_LOTTERY_GIVE)
  {
    addLotteryMail(oCopy.id(), oCopy.time(), true);
  }
  else
    m_setUpdateIDs.insert(oCopy.id());

  // plat log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_AddMail;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->m_platformId,
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    0,  /*charge */
    eType, 0, 1);

  stringstream ss;

  const BlobAttach& rAttachs = oCopy.attach();
  
  for (int i = 0; i < rAttachs.attachs_size(); i++)
  {
    const MailAttach rMailAttach = rAttachs.attachs(i);
    if (rMailAttach.items_size())
    {
      const ItemInfo rItemInfo = rMailAttach.items(0);
      ss << rItemInfo.id() << "," << rItemInfo.count() << ";";
    }
    else if (rMailAttach.itemdatas_size())
    {
      const ItemData rItemData = rMailAttach.itemdatas(0);
      ss << rItemData.base().id() << "," << rItemData.base().count()<< ";";
    }
  }
  std::string strItem = ss.str();
  PlatLogManager::getMe().MailLog(thisServer,
    m_pUser->m_platformId,
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    oCopy.id(),
    oCopy.sysid(),
    oCopy.type(),
    oCopy.title(),
    strItem);

  return true;
}

bool Mail::getAttach(QWORD id, bool bAuto/*=false*/, bool fromWedding/* = false*/, bool weddingValid/* = false*/)
{
  // check mail exist and status
  auto m = m_mapMail.find(id);
  if (m == m_mapMail.end())
  {
    XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "附件失败,未发现该邮件" << XEND;
    return false;
  }
  if (m->second.status() == EMAILSTATUS_INVALID)
  {
    XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "附件失败,该邮件在领取中" << XEND;
    return false;
  }

  // 婚礼邀请函邮件须向wedding服查询婚礼有效性, 确认有效才发放道具
  bool bSendAttach = true;
  DWORD dwMsgID = 0;
  if (fromWedding == false && m->second.type() == EMAILTYPE_WEDDINGINVITATION)
  {
    DWORD itemid = MiscConfig::getMe().getWeddingMiscCFG().dwInvitationItemID, cur = now();
    QWORD qwWedddingID = 0;
    bool bWeddingNotEnd = false;
    for (int i = 0; i < m->second.attach().attachs_size(); ++i)
    {
      const MailAttach& rAttach = m->second.attach().attachs(i);
      for (int j = 0; j < rAttach.itemdatas_size(); ++j)
        if (rAttach.itemdatas(j).base().id() == itemid)
        {
          bWeddingNotEnd = rAttach.itemdatas(j).wedding().endtime() > cur;
          qwWedddingID = rAttach.itemdatas(j).wedding().id();
          break;
        }
      if (qwWedddingID)
        break;
    }
    if (bWeddingNotEnd)
    {
      CheckWeddingReserverSCmd cmd;
      cmd.set_weddingid(qwWedddingID);
      cmd.set_mailid(id);
      cmd.set_charid(m_pUser->id);
      PROTOBUF(cmd, send, len);
      ForwardS2WeddingSCmd fcmd;
      fcmd.set_charid(m_pUser->id);
      fcmd.set_zoneid(thisServer->getZoneID());
      fcmd.set_name(m_pUser->name);
      fcmd.set_data(send, len);
      fcmd.set_len(len);
      PROTOBUF(fcmd, send2, len2);
      thisServer->sendCmd(ClientType::wedding_server, send2, len2);
      XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "婚礼邀请函邮件,成功发送wedding服查询婚礼有效性" << XEND;
      return true;
    }
    else
    {
      bSendAttach = false;
      dwMsgID = 9645;
    }
  }
  if (fromWedding && !weddingValid)
  {
    bSendAttach = false;
    dwMsgID = 9645;
  }

  // set mail status first to prevent get reward twice
  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "mail");
  if (field == nullptr)
  {
    XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "附件失败,未找到 mail 数据库表" << XEND;
    return false;
  }
  xRecord record(field);
  record.put("status", EMAILSTATUS_INVALID);

  stringstream sstr;
  sstr << "id = " << id;
  QWORD ret = thisServer->getDBConnPool().exeUpdate(record, sstr.str().c_str());
  if (ret == QWORD_MAX)
  {
    XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "附件失败,更新数据库失败 ret :" << ret << XEND;
    return false;
  }

  // check read mail
  if (m->second.status() == EMAILSTATUS_READ)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_MAIL_ACCOUNT_ATTACH);
    m_mapMail.erase(m);
    m_setUpdateIDs.insert(id);
    XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "附件失败,同账号其他角色已领取过该邮件" << XEND;
    return true;
  }
  m->second.set_status(EMAILSTATUS_INVALID);

  // set mail status read if account mail
  if (m->second.receiveaccid() != 0)
  {
    record.put("status", EMAILSTATUS_INVALID);
    char where[256] = { 0 };
    snprintf(where, sizeof(where), "receiveaccid = %llu and groupid=\"%s\" and receiveid <> %llu ", m_pUser->accid, m->second.groupid().c_str(), m_pUser->id);
    ret = thisServer->getDBConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX)
    {
      XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "附件失败,更新同账号数据库失败 ret :" << ret << XEND;
      return false;
    }
  }

  // check system mail overtime
  DWORD dwNow = xTime::getCurSec();
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  if (m->second.sysid() != 0 && dwNow > m->second.time() && dwNow - m->second.time() > rCFG.dwSysMailOverTime)
  {
    m_mapMail.erase(m);
    m_setUpdateIDs.insert(id);
    XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "附件失败,邮件已过期" << XEND;
    return true;
  }

  TVecItemInfo vecItems;
  TVecItemData vecItamDatas;
  for (int i = 0; i < m->second.attach().attachs_size(); ++i)
  {
    const MailAttach& rAttach = m->second.attach().attachs(i);
    if (rAttach.type() == EMAILATTACHTYPE_REWARD)
    {
      TVecItemInfo vecReward;
      if (RewardManager::roll(m->second.attach().attachs(i).id(), m_pUser, vecReward, ESOURCE_REWARD) == true)
        combinItemInfo(vecItems, vecReward);
    }
    else if (rAttach.type() == EMAILATTACHTYPE_ITEM)
    {
      for (int j = 0; j < rAttach.items_size(); ++j)
        vecItems.push_back(rAttach.items(j));
    }

    for (int j = 0; j < rAttach.itemdatas_size(); ++j)
    {
      vecItamDatas.push_back(rAttach.itemdatas(j));
    }
  }

  GetMailAttachSessionCmd cmd;
  cmd.set_mailid(id);
  cmd.set_charid(m_pUser->id);
  cmd.set_groupid(m->second.groupid());
  if (m->second.type() == EMAILTYPE_LOTTERY_GIVE)
  {
    if (bAuto)
      cmd.set_opt(EGetMailOpt_LotteryGive_Auto);
    else
      cmd.set_opt(EGetMailOpt_LotteryGive);
  }

  // 不发送附件到包裹
  if (bSendAttach == false)
  {
    processGetAttach(cmd);
    if (dwMsgID)
      MsgManager::sendMsg(m_pUser->id, dwMsgID);
    XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "成功,无附件" << XEND;
    return true;
  }

  for (auto &v : vecItems)
    cmd.add_items()->CopyFrom(v);
  for (auto &v : vecItamDatas)
    cmd.add_itemdatas()->CopyFrom(v);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToScene(send, len);

  XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 mailid :" << id << "成功并发送到场景服务器处理" <<"是否自动领取"<<bAuto << XEND;
  return true;
}

bool Mail::processGetAttach(const GetMailAttachSessionCmd& cmd)
{
  // process mail list
  auto m = m_mapMail.find(cmd.mailid());
  QWORD qwReceiveaccID = 0;
  if (m != m_mapMail.end())
  {
    qwReceiveaccID = m->second.receiveaccid();
    if (cmd.msgid() != 0)
    {
      m->second.set_status(EMAILSTATUS_MIN);
      XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "处理获取 mailid :" << cmd.mailid() << "msgid :" << cmd.msgid() << "附件成功,还原了邮件可领取状态" << XEND;
    }
    else
    {
      QWORD eid = xTime::getCurUSec();
      EVENT_TYPE eType = EventType_Mail;
      PlatLogManager::getMe().eventLog(thisServer,
        m_pUser->m_platformId,
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eid,
        0,  //charge 
        eType, 0, 1);

      MailData& oData = m->second;

      stringstream ss;
      const BlobAttach& rAttachs = oData.attach();
      for (int i = 0; i < rAttachs.attachs_size(); i++)
      {
        const MailAttach rMailAttach = rAttachs.attachs(i);
        if (rMailAttach.items_size())
        {
          const ItemInfo rItemInfo = rMailAttach.items(0);
          ss << rItemInfo.id() << "," << rItemInfo.count() << ";";
        }
        else if (rMailAttach.itemdatas_size())
        {
          const ItemData rItemData = rMailAttach.itemdatas(0);
          ss << rItemData.base().id() << "," << rItemData.base().count() << ";";
        }
      }

      std::string strItem = ss.str();
      PlatLogManager::getMe().MailLog(thisServer,
        m_pUser->m_platformId,
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eType,
        eid,
        oData.id(),
        oData.sysid(),
        oData.type(),
        oData.title(),
        strItem);

      if (cmd.opt() == EGetMailOpt_LotteryGive || cmd.opt() == EGetMailOpt_LotteryGive_Auto)
      {
        //您的好友xx已经成功签收了你的礼物！
        MsgManager::sendMsgMust(oData.senderid(), 25309, MsgParams{m_pUser->name});
        delLotteryMail(cmd.mailid());

        ItemData itemdata;
        for (int i = 0; i < rAttachs.attachs_size(); i++)
        {
          const MailAttach rMailAttach = rAttachs.attachs(i);
          if (rMailAttach.items_size())
          {
            ItemInfo* p = itemdata.mutable_base();
            if (p)
              p->CopyFrom(rMailAttach.items(0));
            break;
          }
        }
        PlatLogManager::getMe().TradeGiveLog(thisServer,
                                             oData.senderid(),
                                             EGiveEvent_Accept,
                                             itemdata.base().id(),
                                             0,
                                             pb2json(itemdata),
                                             m_pUser->id,
                                             oData.sender(),
                                             m_pUser->name,
                                             oData.time(),
                                             ELogGiveType_Lottery,
                                             itemdata.base().count()
          );
      }
      else
        m_setUpdateIDs.insert(cmd.mailid());  
      XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "处理获取 mailid :" << cmd.mailid() << "msgid :" << cmd.msgid() << "附件成功,从列表中删除了该邮件" << XEND;
      m_mapMail.erase(m);

    }
  }

  // process mail db
  if (cmd.msgid() != 0)
  {
    xField* field = thisServer->getDBConnPool().getField(REGION_DB, "mail");
    if (field == nullptr)
    {
      XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "处理获取 mailid :" << cmd.mailid() << "msgid :" << cmd.msgid() << "附件失败,未找到 mail 数据库表" << XEND;
      return false;
    }
    xRecord record(field);
    record.put("status", EMAILSTATUS_MIN);

    stringstream sstr;
    sstr << "id = " << cmd.mailid();
    QWORD ret = thisServer->getDBConnPool().exeUpdate(record, sstr.str().c_str());
    if (ret == QWORD_MAX)
    {
      XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "处理获取 mailid :" << cmd.mailid() << "msgid :" << cmd.msgid() << "附件失败,更新数据库失败 ret :" << ret << XEND;
      return false;
    }
    // set mail status read if account mail
    if (qwReceiveaccID != 0)
    {
      record.put("status", EMAILSTATUS_MIN);

      char where[512] = { 0 };
      snprintf(where, sizeof(where), "receiveaccid = %llu and groupid=\"%s\" and receiveid <> %llu ", m_pUser->accid, cmd.groupid().c_str(), m_pUser->id);

      ret = thisServer->getDBConnPool().exeUpdate(record, where);
      if (ret == QWORD_MAX)
      {
        XERR << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "处理获取 mailid :" << cmd.mailid() << "附件失败,更新同账号数据库失败 ret :" << ret << XEND;
        return false;
      }
    }
    XLOG << "[邮件-获取附件]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "处理获取 mailid :" << cmd.mailid() << "msgid :" << cmd.msgid() << "附件成功,成功更新数据库mail状态" << XEND;
  }

  return true;
}

//one sec
void Mail::timer(DWORD curTime)
{
  giveTimer(curTime);

  if (m_setUpdateIDs.empty() == true)
    return;

  processMailLanguage();

  MailUpdate cmd;
  for (auto s = m_setUpdateIDs.begin(); s != m_setUpdateIDs.end(); ++s)
  {
    auto m = m_mapMail.find(*s);
    if (m != m_mapMail.end())
    {
      MailData* pData = cmd.add_updates();
      if (pData != nullptr)
      {
        pData->CopyFrom(m->second);
        MailManager::getMe().translateMailMsg(pData, m_pUser->getLanguage());
      }
    }
    else
    {
      cmd.add_dels(*s);
    }
  }

  if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);

    for (int i = 0; i < cmd.updates_size(); ++i)
      XLOG << "[邮件同步-更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid :" << cmd.updates(i).id() << XEND;
    for (int i = 0; i < cmd.dels_size(); ++i)
      XLOG << "[邮件同步-删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid :" << cmd.dels(i) << XEND;
  }

  m_setUpdateIDs.clear();
  
}

void Mail::giveTimer(DWORD curTime)
{
  if (curTime < m_dwNextTickTime)
  {
    return;
  }
  m_dwNextTickTime = curTime + 30;

  if (m_mapLotterGive.empty())
    return;
  for (auto& m:m_mapLotterGive)
  {
    if (curTime >= m.second)
    {
      getAttach(m.first, true);
    }
  }
}

void Mail::loadAllMail(xRecordSet &set)
{
  TVecQWORD vecInvalidIDs;
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  DWORD dwNow = xTime::getCurSec();

  DWORD retNum = set.size();
  for (QWORD q = 0; q < retNum; ++q)
  {
    DWORD dwType = set[q].get<DWORD>("mailtype");
    QWORD qwSysID = set[q].get<QWORD>("sysid");
    if (dwType == EMAILTYPE_SYSTEM)
      m_setSysMailIDs.insert(qwSysID);

    DWORD dwStatus = set[q].get<DWORD>("status");
    QWORD qwID = set[q].get<QWORD>("id");
    if (qwID > m_qwLastMailID)
      m_qwLastMailID = qwID;
    if (dwStatus == EMAILSTATUS_INVALID)
      continue;

    DWORD dwTime = set[q].get<DWORD>("time");
    if (dwType == EMAILTYPE_WEDDINGMSG)
    {//不算超时的
    }
    else if (dwType == EMAILTYPE_USEREVENT)
    {
    }
    else if (dwType == EMAILTYPE_NORMAL_NOTIME)
    {
    }
    else
    {
      if (dwNow > dwTime && dwNow - dwTime >= rCFG.dwSysMailOverTime)
      {
        vecInvalidIDs.push_back(qwID);
        continue;
      }
    }

    if (dwType == EMAILTYPE_TRADE)
    {
      auto m = m_mapTradeMail.find(qwID);
      if (m != m_mapTradeMail.end())
        continue;
    }
    else {
      auto m = m_mapMail.find(qwID);
      if (m != m_mapMail.end())
        continue;
    }

    MailData oData;

    oData.set_id(qwID);
    oData.set_sysid(set[q].get<QWORD>("sysid"));
    oData.set_senderid(set[q].get<QWORD>("senderid"));
    oData.set_receiveid(set[q].get<QWORD>("receiveid"));
    oData.set_receiveaccid(set[q].get<QWORD>("receiveaccid"));
    oData.set_type(static_cast<EMailType>(dwType));
    oData.set_status(static_cast<EMailStatus>(set[q].get<DWORD>("status")));
    oData.set_time(set[q].get<DWORD>("time"));
    oData.set_title(set[q].getString("title"));
    oData.set_sender(set[q].getString("sender"));
    oData.set_msg(set[q].getString("msg"));
    oData.set_groupid(set[q].getString("groupid"));

    string attach;
    attach.assign((const char *)set[q].getBin("attach"), set[q].getBinSize("attach"));
    if (oData.type() == EMAILTYPE_WEDDINGMSG)
    {
      if (oData.mutable_weddingmsg()->ParseFromString(attach) == false)
      {
        XERR << "[邮件-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid :" << qwID << "附件反序列化失败" << XEND;
        continue;
      }
    }
    else if (oData.type() == EMAILTYPE_USEREVENT)
    {
      if (oData.mutable_eventmsg()->ParseFromString(attach) == false)
      {
        XERR << "[邮件-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid :" << qwID << "附件反序列化失败" << XEND;
        continue;
      }
    }
    else
    {
      if (oData.mutable_attach()->ParseFromString(attach) == false)
      {
        XERR << "[邮件-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid :" << qwID << "附件反序列化失败" << XEND;
        continue;
      }
    }

    if (dwType == EMAILTYPE_TRADE) {
      m_mapTradeMail[qwID].CopyFrom(oData);
    }
    else if (dwType == EMAILTYPE_WEDDINGMSG)
    {
      m_mapWeddingEventMail[qwID].CopyFrom(oData);
    }
    else if (dwType == EMAILTYPE_USEREVENT)
    {
      m_mapUserEventMail[qwID].CopyFrom(oData);
    }
    else {
      m_mapMail[qwID].CopyFrom(oData);
    }
    if (dwType == EMAILTYPE_LOTTERY_GIVE)
    {
      addLotteryMail(qwID, oData.time(), false);
    }
  }

  if (vecInvalidIDs.empty() == false)
  {
    stringstream sstr;
    for (size_t i = 0; i < vecInvalidIDs.size(); ++i)
    {
      if (i == vecInvalidIDs.size() - 1)
        sstr << " id = " << vecInvalidIDs[i];
      else
        sstr << " id = " << vecInvalidIDs[i] << " or ";
    }
    xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "mail");
    if (pField == nullptr)
    {
      XERR << "[邮件-加载]" << m_pUser->id << "加载失败,del未找到mail数据库表" << XEND;
      return;
    }
    xRecord del(pField);
    del.put("status", EMAILSTATUS_INVALID);
    retNum = thisServer->getDBConnPool().exeUpdate(del, sstr.str().c_str());
    if (retNum == QWORD_MAX)
      XERR << "[邮件-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "删除过期邮件失败" << XEND;
    else
      XLOG << "[邮件-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置 id :" << sstr.str() << "过期邮件" << XEND;
  }
}

void Mail::sendAllMail()
{
  if (m_pUser == nullptr)
    return;

  QueryAllMail cmd;
  for (auto m = m_mapMail.begin(); m != m_mapMail.end(); ++m)
  {
    //不发给前端
    if (m->second.type() == EMAILTYPE_LOTTERY_GIVE)
      continue;

    //不发给客户端
    if (m->second.type() == EMAILTYPE_WEDDINGMSG || m->second.type() == EMAILTYPE_USEREVENT)
    {
      continue;
    }

    MailData* pData = cmd.add_datas();
    if (pData != nullptr)
    {
      pData->CopyFrom(m->second);
      MailManager::getMe().translateMailMsg(pData, m_pUser->getLanguage());
    }

    if (cmd.ByteSize() > TRANS_BUFSIZE)
    {
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      cmd.Clear();
    }
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Mail::processMailLanguage()
{
  // 多语言处理
  map<QWORD, DWORD> mapMailID2TemplateID;
  for (auto& v : m_mapMail)
  {
    if (v.second.type() != EMAILTYPE_NORMAL && v.second.type() != EMAILTYPE_SYSTEM)
      continue;
    DWORD templateid = MailTemplateManager::getMe().getTemplateID(v.second.msg());
    if (templateid)
    {
      if (MailTemplateManager::getMe().getMsg(templateid, m_pUser->getLanguage(), v.second.mutable_title(), v.second.mutable_msg()) == false)
        mapMailID2TemplateID[v.second.id()] = templateid;
    }
  }

  if (mapMailID2TemplateID.empty() == false)
  {
    TSetDWORD templateids;
    for (auto& v : mapMailID2TemplateID)
      templateids.insert(v.second);
    if (MailTemplateManager::getMe().load(templateids))
    {
      for (auto& v : mapMailID2TemplateID)
      {
        MailData* data = getMailData(v.first);
        if (data)
        {
          if (MailTemplateManager::getMe().getMsg(v.second, m_pUser->getLanguage(), data->mutable_title(), data->mutable_msg()) == false)
          {
            XERR << "[邮件-多语言处理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "邮件模板获取失败,msg:" << data->msg() << XEND;
          }
        }
      }
    }
  }
}

bool Mail::checkSystemMail()
{
  DWORD dwNow = xTime::getCurSec();
  const TMapMailData& mapList = MailManager::getMe().getSystemMailList();
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  for (auto m = mapList.begin(); m != mapList.end(); ++m)
  {
    if (dwNow > m->second.time() && dwNow - m->second.time() > rCFG.dwSysMailOverTime)
      continue;

    QWORD qwSysID = m->second.sysid();
    auto s = m_setSysMailIDs.find(qwSysID);
    if (s != m_setSysMailIDs.end())
      continue;
    if (m->second.receiveid() != 0 && m->second.receiveid() != m_pUser->id)
      continue;

    DWORD endTime = m->second.endtime();
    if (endTime == 0)
    {
      endTime = m->second.time();
    }   

    if (m_pUser->getCreateTime() > endTime || endTime == 0)
      continue;
    
    if (m->second.starttime() && m_pUser->getCreateTime() < m->second.starttime())
      continue;

    addMail(m->second);
  }

  return true;
}

bool Mail::processTradeMail()
{
  for (auto it = m_mapTradeMail.begin(); it != m_mapTradeMail.end(); ++it) 
  {
    QWORD id = it->first;
    MailData mailData = it->second;

    if (mailData.attach().attachs_size() == 1)
    {
      if (mailData.attach().attachs(0).type() == EMAILATTACHTYPE_TRADE_SELL)
      {
        if (mailData.attach().attachs(0).items_size() > 0)
        {
          Cmd::ItemInfo oItemInfo = mailData.attach().attachs(0).items(0);
          AddMoneyRecordTradeCmd cmd;
          cmd.set_charid(mailData.receiveid());
          cmd.set_money_type(oItemInfo.id());
          cmd.set_total_money(oItemInfo.count());
          PROTOBUF(cmd, send, len);
          XLOG << "[交易-离线邮件] 玩家上线，发送获得钱币，charid:" << mailData.receiveid() << ", moneytype:" << oItemInfo.id() << ", count:" << oItemInfo.count() << XEND;
          if (m_pUser && m_pUser->sendCmdToScene(send, len) == false)
            return false;
        }
      }
      else
      {
        AddItemRecordTradeCmd sendCmd;
        TradeItemBaseInfo* pBaseInfo = sendCmd.mutable_item_info();
        if (mailData.attach().attachs(0).items_size() > 0)
        {
          Cmd::ItemInfo oItemInfo = mailData.attach().attachs(0).items(0);
          pBaseInfo->set_itemid(oItemInfo.id());
          pBaseInfo->set_count(oItemInfo.count());
        }

        if (mailData.attach().attachs(0).itemdatas_size() > 0)
        {
          Cmd::ItemData oItemData = mailData.attach().attachs(0).itemdatas(0);
          pBaseInfo->set_itemid(oItemData.base().id());
          pBaseInfo->set_count(oItemData.base().count());
          Cmd::ItemData *pData = pBaseInfo->mutable_item_data();
          if (pData)
          {
            pData->CopyFrom(oItemData);
          }
        }

        sendCmd.set_charid(mailData.receiveid());
        if (mailData.attach().attachs(0).type() == EMAILATTACHTYPE_TRADE_BUY)
        {
          sendCmd.set_addtype(EADDITEMTYP_BUY);
        }
        else
        {
          sendCmd.set_addtype(EADDITEMTYP_RETURN);
        }
        PROTOBUF(sendCmd, send, len);
        thisServer->doTradeCmd((BYTE*)send, len);
      }
    }
    else
    {
      //异常
      XERR << "[邮件-交易所]userid = " << m_pUser->id << " name = " << m_pUser->name << " get mail = " << id << " attach size:" << (DWORD)mailData.attach().attachs_size() << XEND;
    }

    xField* field = thisServer->getDBConnPool().getField(REGION_DB, "mail");
    if (field == nullptr)
      return false;

    xRecord record(field);
    record.put("status", EMAILSTATUS_INVALID);

    char szWhere[32] = { 0 };
    snprintf(szWhere, 32, "id = %llu", id);

    if (thisServer->getDBConnPool().exeUpdate(record, szWhere) == QWORD_MAX)
      return false;
    XLOG << "[邮件-交易所] userid = " << m_pUser->id << " name = " << m_pUser->name << " get mail = " << id << " 处理成功，不代表玩家真正收到。" << XEND;
  }
  return true;
}

void Mail::processLotterGiveMail()
{
  if (m_mapLotterGive.empty())
    return;
  
}

void Mail::updateMail()
{
  checkSystemMail();
  processMailLanguage();
  sendAllMail();
  processTradeMail();
  processLotterGiveMail();
  processEventMail();
}

void Mail::syncGingerToScene()
{
  if (m_mapLotterGive.empty())
    return;

  syncGingerToScene(m_mapLotterGive);
}

void Mail::syncGingerToScene(TMapId2ExpireTime& rMap)
{
  Cmd::SyncGiveItemSceneTradeCmd cmd;
  cmd.set_type(EGiveType_Lottery);
  cmd.set_charid(m_pUser->id);
  for (auto it = m_mapLotterGive.begin(); it != m_mapLotterGive.end(); ++it)
  {
    MailData* pMail = getMailData(it->first);
    if (pMail == nullptr)
      continue;

    GiveItemInfo* pInfo = cmd.add_iteminfo();
    if (pInfo == nullptr)
      continue;
    pInfo->set_id(it->first);
    pInfo->set_expiretime(it->second);
  }
  PROTOBUF(cmd, send, len);

  if (m_pUser->sendCmdToScene(send, len) == false)
  {
    XERR << "[扭蛋-赠送接收者] 列表发送到场景失败" << m_pUser->id << m_pUser->name << "size" << cmd.iteminfo_size() << XEND;
    return;
  }
  XLOG << "[扭蛋-赠送接收者] 列表发送到场景成功" << m_pUser->id << m_pUser->name << "size" << cmd.iteminfo_size() << XEND;
}

void Mail::addLotteryMail(QWORD id, DWORD createTime, bool bSync2Scene)
{
  DWORD dwExpireTime = createTime + MiscConfig::getMe().getLotteryCFG().dwExpireTime;
  m_mapLotterGive[id] = dwExpireTime;
  
  if (bSync2Scene)
  {
    TMapId2ExpireTime tmpMap = { {id, dwExpireTime } };
    syncGingerToScene(tmpMap);
  }
}

void Mail::delLotteryMail(QWORD id)
{
  m_mapLotterGive.erase(id);
}

MailData* Mail::getMailData(QWORD id)
{
  auto it = m_mapMail.find(id);
  if (it == m_mapMail.end())
    return nullptr;
  return &(it->second);
}
//一定要放到sendWeddingInfo2Scene后面，不然场景数据状态不一致
bool Mail::processWeddingMsg()
{
  if (m_mapWeddingEventMail.empty())
    return true;

  DWORD count = 0;
  for (auto it = m_mapWeddingEventMail.begin(); it != m_mapWeddingEventMail.end();)
  {
    if (processWeddingMsg(it->second))
    {
      it = m_mapWeddingEventMail.erase(it);
      count++;
      continue;
    }
    ++it;
  }

  XLOG <<"[邮件-婚礼事件] 成功处理"<<m_pUser->accid <<m_pUser->id <<m_pUser->name <<count <<"封" << XEND;
  return true;
}

bool Mail::processWeddingMsg(const MailData& rData)
{ 
  PROTOBUF(rData.weddingmsg(), send, len);
  //发送消息到场景
  if (m_pUser->sendCmdToSceneUser(send, len) == false)
  {
    //发送到场景失败
    return false;
  }
  XLOG << "[邮件-婚礼事件-发送场景] 成功" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name <<"mailid"<<rData.id() << XEND;

  //warning:如果发送场景后，场景玩家不在线，会存在丢失

  //删除邮件记录
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "mail");
  xRecord del(pField);
  del.put("status", EMAILSTATUS_INVALID);
  stringstream sstr;
  sstr << "id = " << rData.id();
  QWORD retNum = thisServer->getDBConnPool().exeUpdate(del, sstr.str().c_str());
  if (retNum == QWORD_MAX)
  {
    XERR << "[邮件-婚礼事件-删除] 失败" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid" << rData.id()<< XEND;
    return false;
  }
  else
  {
    XLOG << "[邮件-婚礼事件-删除] 成功" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid" << rData.id() << XEND;
    return true;
  }
  return false;
}

bool Mail::processEventMail(const MailData& rData)
{
  PROTOBUF(rData.eventmsg(), send, len);
  //发送消息到场景
  if (m_pUser->sendCmdToSceneUser(send, len) == false)
  {
    //发送到场景失败
    return false;
  }
  XLOG << "[邮件-事件-发送场景] 成功" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name <<"mailid"<<rData.id() << XEND;

  //删除邮件记录
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "mail");
  xRecord del(pField);
  del.put("status", EMAILSTATUS_INVALID);
  stringstream sstr;
  sstr << "id = " << rData.id();
  QWORD retNum = thisServer->getDBConnPool().exeUpdate(del, sstr.str().c_str());
  if (retNum == QWORD_MAX)
  {
    XERR << "[邮件-事件-删除] 失败" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid" << rData.id()<< XEND;
    return false;
  }
  else
  {
    XLOG << "[邮件-事件-删除] 成功" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mailid" << rData.id() << XEND;
    return true;
  }
  return false;
}

bool Mail::processEventMail()
{
  if (m_mapUserEventMail.empty())
    return true;

  DWORD count = 0;
  for (auto it = m_mapUserEventMail.begin(); it != m_mapUserEventMail.end();)
  {
    if (processEventMail(it->second))
    {
      it = m_mapUserEventMail.erase(it);
      count++;
      continue;
    }
    ++it;
  }

  XLOG <<"[邮件-事件] 成功处理"<<m_pUser->accid <<m_pUser->id <<m_pUser->name <<count <<"封" << XEND;
  return true;
}

bool Mail::isMailValid(QWORD mailid)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "mail");
  if (pField == nullptr)
  {
    XERR << "[邮件-检查存在]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加载失败,未找到mail数据库表" << XEND;
    return false;
  }
  pField->setValid("`status`");

  char szWhere[32] = { 0 };
  snprintf(szWhere, 32, "id = %llu", mailid);

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, szWhere);
  if (ret == QWORD_MAX)
  {
    XERR << "[邮件-检查存在]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询失败" << XEND;
    return false;
  }
  return ret <= 0 ? false : set[0].get<DWORD>("status") != EMAILSTATUS_INVALID;
}
