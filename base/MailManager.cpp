#include "MailManager.h"
#include "xServer.h"
#include "ZoneServer.h"
#include "RegionServer.h"
#include "TableManager.h"
#include "GuidManager.h"
#include "SceneItem.pb.h"
#include "config/MiscConfig.h"
#include "config/DepositConfig.h"
#include "GuildCmd.pb.h"
#include "config/SysmsgConfig.h"
#include "CommonConfig.h"

extern xServer* thisServer;

MailManager::MailManager()
{

}

MailManager::~MailManager()
{

}

bool MailManager::addSysMail(QWORD qwMailID, const MailData& rData)
{
  auto m = m_mapSystemMail.find(qwMailID);
  if (m != m_mapSystemMail.end())
    return false;

  m_mapSystemMail[qwMailID] = rData;
  return true;
}

const MailData* MailManager::getSysMail(QWORD qwMailID) const
{
  auto m = m_mapSystemMail.find(qwMailID);
  if (m != m_mapSystemMail.end())
    return &m->second;
  return nullptr;
}

bool MailManager::loadAllSystemMail()
{
  if (strncmp(thisServer->getServerName(), "SessionServer", 13) != 0)
  {
    XERR << "[系统邮件-加载] 加载失败" << thisServer->getServerName() << "进行了加载,邮件只能由SessionServer进行加载" << XEND;
    return false;
  }

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "sysmail");
  if (pField == nullptr)
  {
    XERR << "[系统邮件-加载] 失败,未找到sysmail数据库表" << XEND;
    return false;
  }

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  DWORD curTime = xTime::getCurSec();
  stringstream sstr;
  sstr << "status != " << EMAILSTATUS_INVALID << " and time + " << rCFG.dwSysMailOverTime << " >= " << curTime;

  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
  if (QWORD_MAX == retNum)
  {
    XERR << "[系统邮件-加载] 失败,查询失败 ret :" << retNum << XEND;
    return false;
  }

  for (QWORD q = 0; q < retNum; ++q)
  {
    QWORD qwSysID = set[q].get<QWORD>("sysid");

    auto m = m_mapSystemMail.find(qwSysID);
    if (m != m_mapSystemMail.end())
    {
      XERR << "[系统邮件-加载] sysid :" << qwSysID << "重复了,忽略此邮件" << XEND;
      continue;
    }

    MailData oData;

    oData.set_sysid(qwSysID);
    oData.set_senderid(set[q].get<QWORD>("senderid"));
    oData.set_receiveid(set[q].get<QWORD>("receiveid"));
    oData.set_receiveaccid(set[q].get<QWORD>("receiveaccid"));
    oData.set_type(static_cast<EMailType>(set[q].get<DWORD>("mailtype")));
    oData.set_status(static_cast<EMailStatus>(set[q].get<DWORD>("status")));
    oData.set_time(set[q].get<DWORD>("time"));
    oData.set_title(set[q].getString("title"));
    oData.set_sender(set[q].getString("sender"));
    oData.set_msg(set[q].getString("msg"));
    oData.set_starttime(set[q].get<DWORD>("starttime"));
    oData.set_endtime(set[q].get<DWORD>("endtime"));

    string attach;
    attach.assign((const char *)set[q].getBin("attach"), set[q].getBinSize("attach"));
    if (oData.mutable_attach()->ParseFromString(attach) == false)
    {
      XERR << "[系统邮件-加载] sysid :" << qwSysID << "附件反序列化失败" << XEND;
      continue;
    }

    m_mapSystemMail[qwSysID].CopyFrom(oData);
  }

  XLOG << "[系统邮件-加载] 加载了" << m_mapSystemMail.size() << "封系统邮件" << XEND;

  return true;
}

bool MailManager::sendMail(QWORD qwTargetID, DWORD dwMailID, MsgParams params /*= MsgParams()*/)
{
  const MailBase* pMailBase = TableManager::getMe().getMailCFG(dwMailID);
  if (pMailBase == nullptr)
    return false;

  TVecItemInfo vecItemInfo;
  xLuaData item = pMailBase->getData("item");
  auto itemf = [&](const string& str, xLuaData& data)
  {
    ItemInfo oItem;
    oItem.set_id(data.getTableInt("id"));
    oItem.set_count(data.getTableInt("num"));
    oItem.set_source(ESOURCE_MAIL);
    vecItemInfo.push_back(oItem);
  };
  item.foreach(itemf);

  return sendMail(qwTargetID, 0, "", pMailBase->getTableString("name"), pMailBase->getTableString("Desc"), EMAILTYPE_NORMAL, dwMailID, vecItemInfo, TVecItemData{}, EMAILATTACHTYPE_ITEM, false, params);
}

bool MailManager::sendMail(QWORD qwTargetID, QWORD qwSenderID, const string& strSender, const string& strTitle1, const string& strMsg1,
    EMailType eType /*= EMAILTYPE_NORMAL*/, DWORD dwMailID /*= 0*/, const TVecItemInfo& vecItemInfo /*= TVecItemInfo{}*/, const TVecItemData& vecItemDatas /*= TVecItemData{}*/,
    EMailAttachType attachType /*=EMAILATTACHTYPE_ITEM*/ , bool bAccMail /*= false*/,
    MsgParams params/*=MsgParams()*/, DWORD startTime/*=0*/, DWORD endTime/*=0*/, DWORD chargeMoney/* = 0*/, bool bSplit /*= true*/, QWORD qwQuota/*=0*/)
{
  if (eType <= EMAILTYPE_MIN || eType >= EMAILTYPE_MAX || EMailType_IsValid(eType) == false)
    return false;

  SendMail cmd;

  string strTitle = strTitle1;
  string strMsg = strMsg1;
  if (dwMailID)
  {
    const MailBase* pMailBase = TableManager::getMe().getMailCFG(dwMailID);
    if (pMailBase)
    {
      strTitle = pMailBase->getTableString("name");
      strMsg = MsgParams::fmtString(pMailBase->getTableString("Desc"), params);
    }
  }

  cmd.mutable_data()->set_id(0);
  cmd.mutable_data()->set_sysid(0);
  cmd.mutable_data()->set_senderid(qwSenderID);
  cmd.mutable_data()->set_receiveid(qwTargetID);
  cmd.mutable_data()->set_receiveaccid(bAccMail);
  cmd.mutable_data()->set_mailid(dwMailID);
  cmd.mutable_data()->set_type(eType);

  cmd.mutable_data()->set_title(strTitle);
  cmd.mutable_data()->set_sender(strSender);
  cmd.mutable_data()->set_msg(strMsg);
  cmd.mutable_data()->set_time(xTime::getCurSec());
  cmd.mutable_data()->set_starttime(startTime);
  cmd.mutable_data()->set_endtime(endTime);

  if (vecItemInfo.empty() == true && vecItemDatas.empty() == true)
  {
    addMailMsg(cmd, params);
    return sendMail(cmd);
  }
  if (!bSplit)
  {
    bool bSet = false;
    cmd.mutable_data()->clear_attach();
    for (auto &v : vecItemInfo)
    {
      MailAttach* pAttach = cmd.mutable_data()->mutable_attach()->add_attachs();
      if (pAttach != nullptr)
      {
        pAttach->set_type(attachType);
        ItemInfo* pInfo = pAttach->add_items();
        if (pInfo != nullptr)
        {
          pInfo->CopyFrom(v);
          if (chargeMoney && !bSet)
          {
            pInfo->set_chargemoney(chargeMoney);
            pInfo->set_quota(qwQuota);
            bSet = true;
          }
        }
      }
    }
    for (auto &v : vecItemDatas)
    {
      MailAttach* pAttach = cmd.mutable_data()->mutable_attach()->add_attachs();
      if (pAttach != nullptr)
      {
        pAttach->set_type(attachType);
        ItemData* pItemData = pAttach->add_itemdatas();
        if (pItemData != nullptr)
          pItemData->CopyFrom(v);
      }
    }

    addMailMsg(cmd, params);
    return sendMail(cmd);
  }

  bool bResult = true;
  bool bSet = false;
  for (auto &v : vecItemInfo)
  {
    cmd.mutable_data()->clear_attach();
    MailAttach* pAttach = cmd.mutable_data()->mutable_attach()->add_attachs();
    if (pAttach != nullptr)
    {
      pAttach->set_type(attachType);
      ItemInfo* pInfo = pAttach->add_items();
      if (pInfo != nullptr)
      {
        pInfo->CopyFrom(v);
        if (chargeMoney && !bSet)
        {
          pInfo->set_chargemoney(chargeMoney);
          pInfo->set_quota(qwQuota);
          bSet = true;
        }
      }
    }

    addMailMsg(cmd, params);
    if (sendMail(cmd) == false)
      bResult = false;
  }

  for (auto &v : vecItemDatas)
  {
    cmd.mutable_data()->clear_attach();
    MailAttach* pAttach = cmd.mutable_data()->mutable_attach()->add_attachs();
    if (pAttach != nullptr)
    {
      pAttach->set_type(attachType);
      ItemData* pItemData = pAttach->add_itemdatas();
      if (pItemData != nullptr)
        pItemData->CopyFrom(v);
    }

    addMailMsg(cmd, params);
    if (sendMail(cmd) == false)
      bResult = false;
  }

  return bResult;
}

bool MailManager::sendMail(QWORD qwTargetID, DWORD dwMailID, const TVecItemInfo& vecItemInfo, MsgParams params /*= MsgParams()*/, bool bAccMail /*= false*/, bool bSplit /*= true*/)
{
  const MailBase* pMailBase = TableManager::getMe().getMailCFG(dwMailID);
  if (pMailBase == nullptr)
    return false;

  return sendMail(qwTargetID, 0, "", pMailBase->getTableString("name"), pMailBase->getTableString("Desc"), EMAILTYPE_NORMAL, dwMailID, vecItemInfo, TVecItemData{},
      EMAILATTACHTYPE_ITEM, bAccMail, params, 0, 0, 0, bSplit);
}

bool MailManager::sendMail(QWORD qwTargetID, DWORD dwMailID, const TVecItemData& vecItemData, MsgParams params /*= MsgParams()*/, bool bAccMail /*= false*/, bool bSplit /*= true*/, EMailType eType /*= EMAILTYPE_NORMAL*/)
{
  const MailBase* pMailBase = TableManager::getMe().getMailCFG(dwMailID);
  if (pMailBase == nullptr)
    return false;

  return sendMail(qwTargetID, 0, "", pMailBase->getTableString("name"), pMailBase->getTableString("Desc"), eType, dwMailID, TVecItemInfo{}, vecItemData,
      EMAILATTACHTYPE_ITEM, bAccMail, params, 0, 0, 0, bSplit);
}

void MailManager::checkInvalidMail(DWORD curTime)
{
  if (strncmp(thisServer->getServerName(), "StatServer", 10) != 0)
  {
    XERR << "[邮件-检查] 检查失败" << thisServer->getServerName() << "进行了检查,邮件只能由StatServer进行检查" << XEND;
    return;
  }

  if (curTime < m_dwInvalidCheckTick)
    return;
  m_dwInvalidCheckTick = curTime + MAIL_INVALID_CHECK_TICK;

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  if (rCFG.dwSysMailOverTime == 0)
  {
    XERR << "[邮件-检查] 检查失败,未正确加载GameConfig配置, 系统邮件过期时间 :" << rCFG.dwSysMailOverTime << XEND;
    return;
  }

  xField* pSysField = thisServer->getDBConnPool().getField(REGION_DB, "sysmail");
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "mail");
  if (pSysField == nullptr || pField == nullptr)
  {
    XERR << "[邮件-检查] 检查失败,未找到 sysmail 和 mail 数据库表" << XEND;
    return;
  }
  pSysField->setValid("sysid, time");
  pField->setValid("id, sysid, time");

  stringstream sstr;
  sstr << "time + " << rCFG.dwSysMailOverTime << " < " << curTime << " and status != " << EMAILSTATUS_INVALID;

  // system mail
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pSysField, set, sstr.str().c_str());
  if (ret == QWORD_MAX)
  {
    XERR << "[邮件-检查] 检查系统邮件失败 ret :" << ret << XEND;
    return;
  }
  for (DWORD d = 0; d < set.size(); ++d)
  {
    QWORD qwSysID = set[d].get<DWORD>("sysid");
    DWORD dwTime = set[d].get<DWORD>("time");

    stringstream sstr;
    sstr << "sysid = " << qwSysID << " and status = " << EMAILSTATUS_INVALID << " limit " << MAIL_DELETE_LIMIT_COUNT;

    // delete from mail
    ret = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
    if (ret == QWORD_MAX)
    {
      XERR << "[邮件-检查] 从mail表中删除玩家系统邮件" << qwSysID << dwTime << "失败 ret :" << ret << XEND;
      return;
    }
    if (ret != 0)
      XLOG << "[邮件-检查] 从mail表中删除玩家系统邮件" << qwSysID << dwTime << "成功,共删除" << ret << "封" << XEND;

    // set sysmail status : invalid
    if (ret == 0)
    {
      xRecord record(pSysField);
      record.put("status", EMAILSTATUS_INVALID);
      sstr.str("");
      sstr << "sysid = " << qwSysID;
      ret = thisServer->getDBConnPool().exeUpdate(record, sstr.str().c_str());
      if (QWORD_MAX == ret)
      {
        XERR << "[系统邮件-检查] 设置系统邮件" << qwSysID << dwTime << "为过期邮件失败 ret :" << ret << XEND;
        return;
      }
      XLOG << "[系统邮件-检查] 设置系统邮件" << qwSysID << dwTime << "成功" << XEND;
    }
  }

  // mail
  sstr.str("");
  sstr << "sysid = 0 and time + " << rCFG.dwSysMailOverTime << " < " << curTime << " and status = " << EMAILSTATUS_INVALID << " limit " << MAIL_DELETE_LIMIT_COUNT;
  ret = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
  if (ret == QWORD_MAX)
  {
    XERR << "[邮件-检查] 从mail表中删除邮件失败ret :" << ret << XEND;
    return;
  }
  if (ret != 0)
    XLOG << "[邮件-检查] 从mail表中删除了" << ret << "封过期邮件" << XEND;
}

bool MailManager::sendMail(SendMail& cmd)
{
  //先插数据库再通知，场景服不能操作数据库，移到sessionserver
  ServerType serverName = thisServer->getServerType();
  if (serverName != SERVER_TYPE_SCENE && serverName != SERVER_TYPE_GUILD && serverName != SERVER_TYPE_SOCIAL && serverName != SERVER_TYPE_MATCH)
  {
    if (cmd.data().type() == EMAILTYPE_SYSTEM)
    {
      if (insertSysMailToDB(cmd.mutable_data()) == false)
        return false;
    }
    else
    {
      if (insertNormalMailToDB(cmd.mutable_data()) == false)
        return false;
    }
  }

  PROTOBUF(cmd, send, len);

  SendMailSocialCmd cmd2;
  cmd2.set_zoneid(thisServer->getZoneID());
  cmd2.set_data(send, len);
  cmd2.set_len(len);
  PROTOBUF(cmd2, send2, len2);
    
  //SocialServer
  /*if (serverName == "SocialServer")
  {
    RegionServer* pServer = dynamic_cast<RegionServer*>(thisServer);
    if (!pServer)
    {
      XERR << "[邮件-发送] 非法的服务器类型转换" << serverName << XEND;
      return false;
    }
    return pServer->doSocialCmd((BYTE*)send2, len2);
  }*/

  //SceneServer
  if (serverName == SERVER_TYPE_SCENE || serverName == SERVER_TYPE_SOCIAL)
  {
    ZoneServer* pServer = dynamic_cast<ZoneServer*>(thisServer);
    if (!pServer)
    {
      XERR << "[邮件-发送] 非法的服务器类型转换" << serverName << XEND;
      return false;
    }
    SendMailFromScene sceneCmd;
    sceneCmd.mutable_data()->CopyFrom(cmd.data());
    PROTOBUF(sceneCmd, send3, len3);
    if (pServer->sendCmdToSession(send3, len3) == false)
    {
      XERR << "[邮件-发送] 从场景发送到session 失败"<< cmd.ShortDebugString() << XEND;
    }
    else
    {
      XINF << "[邮件-发送] 从场景发送到session 成功" << cmd.ShortDebugString() << XEND;
    }
  }

  //SessionServer
  if (serverName == SERVER_TYPE_SESSION)
  {
    ZoneServer* pServer = dynamic_cast<ZoneServer*>(thisServer);
    if (!pServer)
    {
      XERR << "[邮件-发送] 非法的服务器类型转换" << serverName << XEND;
      return false;
    }
    SendMailSocialCmd cmd2;
    cmd2.set_zoneid(thisServer->getZoneID());
    cmd2.set_data(send, len);
    cmd2.set_len(len);
    PROTOBUF(cmd2, send2, len2);

    if (pServer->sendCmd(ClientType::global_server, send2, len2) == false)
    {
      XERR << "[邮件-发送] 发送通知社交服失败" << "mailid" << cmd.data().id() << "sys mailid" << cmd.data().sysid() << XEND;
    }
  }
  //GuildServer, MatchServer
  if (serverName == SERVER_TYPE_GUILD || serverName == SERVER_TYPE_MATCH)
  {
    RegionServer* pServer = dynamic_cast<RegionServer*>(thisServer);
    if (!pServer)
    {
      XERR << "[邮件-发送] 公共服邮件, 非法的服务器类型转换" << serverName << XEND;
      return false;
    }
    SendMailGuildSCmd cmd3;
    cmd3.mutable_data()->CopyFrom(cmd.data());
    PROTOBUF(cmd3, send3, len3);
    pServer->sendCmdToOneZone(send3, len3);
    XLOG << "[邮件发送], 公共服发送邮件到session成功, maild:" << cmd.data().id() << "sys mailid" << cmd.data().sysid() << XEND;
  }
  
  //婚礼服
  if (serverName == SERVER_TYPE_WEDDING)
  {
    RegionServer* pServer = dynamic_cast<RegionServer*>(thisServer);
    if (!pServer)
    {
      return false;
    }
    return pServer->sendMail(cmd);
  }
  return true;
}

bool MailManager::insertNormalMailToDB(MailData* pData)
{
  if (pData == nullptr)
  {
    XERR << "[邮件-添加个人离线邮件] 添加失败 邮件内容为空" << XEND;
    return false;
  }

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "mail");
  if (pField == nullptr)
  {
    XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败,未找到 mail 数据库表" << XEND;
    return false;
  }

  if (pData->receiveid() == 0)
  {
    XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败,邮件没有接受人 receiveid:" << pData->receiveid() << "信息" << XEND;
    return false;
  }

  //if (pData->id() != 0)
  //{
  //  stringstream sstr;
  //  sstr << "id = " << pData->id();
  //  xRecordSet set;
  //  QWORD ret = thisServer->getDBConnPool(pField).exeSelect(pField, set, sstr.str().c_str());
  //  if (ret == QWORD_MAX)
  //  {
  //    XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败,id已存在,但是查询mail表失败" << XEND;
  //    return false;
  //  }
  //  if (ret != 0 || set.empty() == false)
  //  {
  //    XLOG << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "成功,mail表中已存在该邮件" << XEND;
  //    return true;
  //  }
  //}

  // collect target ids
  TSetQWORD setIDs;
  if (pData->receiveaccid() != 0)
  {
    xField* pCharField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (pCharField == nullptr)
    {
      XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败,账号邮件未找到 charbase 数据库表" << XEND;
      return false;
    }
    pCharField->setValid("accid, charid");

    stringstream sstr;
    sstr << "charid = " << pData->receiveid();
    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(pCharField, set, sstr.str().c_str());
    if (ret == QWORD_MAX || ret == 0 || set.empty() == true)
    {
      XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败,查询" << pData->receiveid() << "账号角色失败 ret :" << ret << XEND;
      return false;
    }

    sstr.str("");
    sstr << "accid = " << set[0].get<QWORD>("accid");
    set.clear();
    ret = thisServer->getDBConnPool().exeSelect(pCharField, set, sstr.str().c_str());
    if (ret == QWORD_MAX)
    {
      XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败,查询" << pData->receiveid() << "账号角色失败 ret :" << ret << XEND;
      return false;
    }
    for (size_t i = 0; i < set.size(); ++i)
      setIDs.insert(set[i].get<QWORD>("charid"));

    pData->set_receiveaccid(set[0].get<QWORD>("accid"));
  }
  else
  {
    setIDs.insert(pData->receiveid());
  }
    if (setIDs.empty() == true)
  {
    XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败,未查询到邮件发送目标" << XEND;
    return false;
  }

  // set create time
  //pData->set_time(xTime::getCurSec());

  // create mail data
  xRecord record(pField);
  record.put("mailtype", pData->type());
  record.put("senderid", pData->senderid());
  record.put("receiveaccid", pData->receiveaccid());
  record.put("sysid", pData->sysid());
  record.put("status", pData->status());
  record.put("time", pData->time());
  record.put("title", pData->title());
  record.put("sender", pData->sender());
  record.put("msg", pData->msg());
  string attach;
 
  if (pData->type() == EMAILTYPE_WEDDINGMSG)
  {
    if (pData->weddingmsg().SerializeToString(&attach) == false)
    {
      XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败-序列化weddingmsg失败" << XEND;
      return false;
    }
  }
  else if (pData->type() == EMAILTYPE_USEREVENT)
  {
    if (pData->eventmsg().SerializeToString(&attach) == false)
    {
      XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败-序列化eventmsg失败" << XEND;
      return false;
    }
  }
  else
  {
    if (pData->attach().SerializeToString(&attach) == false)
    {
      XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败-序列化attach失败" << XEND;
      return false;
    }
  }
  record.putBin("attach", (unsigned char *)attach.c_str(), attach.size());

  // insert mail by ids

  string groupid = GuidManager::getMe().newGuidStr(thisServer->getZoneID(), 0);
  pData->set_groupid(groupid);
  for (auto &s : setIDs)
  {
    record.put("receiveid", s);
    record.putString("groupid", groupid);
    QWORD retcode = thisServer->getDBConnPool().exeInsert(record, true);
    if (retcode == QWORD_MAX)
    {
      XERR << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "失败-数据库插入失败" << XEND;
      return false;
    }
    if (s == pData->receiveid())
    {
      pData->set_id(retcode);      
    }
  }

  XLOG << "[邮件-添加个人离线邮件] 添加" << pData->ShortDebugString() << "成功" << XEND;
  return true;
}

bool MailManager::insertSysMailToDB(MailData* pData)
{
  if (pData == nullptr)
  {
    XERR << "[邮件-添加系统离线邮件] 添加失败,邮件为空" << XEND;
    return false;
  }
  if (pData->type() != EMAILTYPE_SYSTEM)
  {
    XERR << "[邮件-添加系统离线邮件] 添加" << pData->ShortDebugString() << "失败,邮件类型 type :" << pData->type() << "不是系统邮件" << XEND;
    return false;
  }
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "sysmail");
  if (pField == nullptr)
  {
    XERR << "[邮件-添加系统离线邮件] 添加" << pData->ShortDebugString() << "失败,未发现 sysmail 数据库表" << XEND;
    return false;
  }

  // set create time
  pData->set_time(xTime::getCurSec());

  // create mail data
  xRecord record(pField);
  record.put("mailtype", pData->type());
  record.put("senderid", pData->senderid());
  record.put("receiveid", pData->receiveid());
  record.put("receiveaccid", pData->receiveaccid());
  record.put("status", pData->status());
  record.put("time", pData->time());
  record.put("title", pData->title());
  record.put("sender", pData->sender());
  record.put("msg", pData->msg());
  record.put("starttime", pData->starttime());
  record.put("endtime", pData->endtime());

  string attach;
  if (pData->attach().SerializeToString(&attach) == false)
  {
    XERR << "[邮件-添加系统离线邮件] 添加" << pData->ShortDebugString() << "失败-序列化attach失败" << XEND;
    return false;
  }
  record.putBin("attach", (unsigned char *)attach.c_str(), attach.size());

  // insert mail by ids
  QWORD retcode = thisServer->getDBConnPool().exeInsert(record, true);
  if (retcode == QWORD_MAX)
  {
    XERR << "[邮件-添加系统离线邮件] 添加" << pData->ShortDebugString() << "失败-数据库插入失败" << XEND;
    return false;
  }
  pData->set_sysid(retcode);

  if (MailManager::getMe().addSysMail(pData->sysid(), *pData) == false)
  {
    XERR << "[邮件-添加系统离线邮件] 添加" << pData->ShortDebugString() << "失败-无法添加到邮件管理器" << XEND;
    return false;
  }

  XLOG << "[邮件-添加系统离线邮件] 添加" << pData->ShortDebugString() << "成功" << XEND;
  return true;
}

bool MailManager::addOfflineTradeItem(Cmd::AddItemRecordTradeCmd& rev)
{
  //只有下架才需要存储
  if (rev.addtype() != EADDITEMTYP_RETURN)
    return false;
  TVecItemInfo vecItems;
  TVecItemData vecItemDatas;

  if (rev.item_info().has_item_data())
  {
    Cmd::ItemData oItemData;
    oItemData.CopyFrom(rev.item_info().item_data());
    vecItemDatas.push_back(oItemData);
  }
  else 
  {
    Cmd::ItemInfo oItemInfo;
    oItemInfo.set_id(rev.item_info().itemid());
    oItemInfo.set_count(rev.item_info().count());
    oItemInfo.set_source(ESOURCE_TRADE);
    vecItems.push_back(oItemInfo);
  }

  if (rev.addtype() == EADDITEMTYP_BUY)
  {
    return MailManager::getMe().sendMail(rev.charid(), 0, SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_SYSTEM), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_EXCHANGE), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_MAIL), EMAILTYPE_TRADE, 0, vecItems, vecItemDatas, EMAILATTACHTYPE_TRADE_BUY);
  }
  else
  {
    return MailManager::getMe().sendMail(rev.charid(), 0, SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_SYSTEM), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_EXCHANGE), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_MAIL), EMAILTYPE_TRADE, 0, vecItems, vecItemDatas, EMAILATTACHTYPE_TRADE_RETURN);
  }
  return false;
}

bool MailManager::addOfflineTradeMoney(Cmd::AddMoneyRecordTradeCmd& rev)
{
  return false;
 /* if (rev.total_money() == 0)
    return true;

  TVecItemInfo vecItems;
  TVecItemData vecItemDatas;
  Cmd::ItemInfo oItemInfo;
  oItemInfo.set_id(rev.money_type());
  oItemInfo.set_count(rev.total_money());
  oItemInfo.set_source(ESOURCE_TRADE);
  vecItems.push_back(oItemInfo);
  
  return MailManager::getMe().sendMail(rev.charid(), 0, "系统", "交易所", "获得钱币", EMAILTYPE_TRADE, 0, vecItems, vecItemDatas, EMAILATTACHTYPE_TRADE_SELL);*/
}

bool MailManager::sendChargeMail(const Cmd::ChargeSessionCmd& rev, QWORD qwQuota, bool bVirgin) {
  const SDeposit* pCFG = DepositConfig::getMe().getSDeposit(rev.dataid());
  if (pCFG == nullptr)
  {
    return false;
  }

  MsgParams params;
  params.addFloat(rev.charge());
  DWORD dwMailID = pCFG->mailId;
  if (dwMailID == 10001 || dwMailID == 10009 )
  {
    params.addNumber(pCFG->count);
    if(bVirgin)
      params.addNumber(pCFG->virginCount);
    else
      params.addNumber(pCFG->count2);
  }

  TVecItemInfo vecItems;
  TVecItemData vecItemDatas;
  Cmd::ItemInfo oItemInfo;
  oItemInfo.set_id(rev.itemid());
  oItemInfo.set_count(rev.count());
  oItemInfo.set_source(ESOURCE_CHARGE);
  vecItems.push_back(oItemInfo);

  std::string serverName(thisServer->getServerName());

  if (MailManager::getMe().sendMail(rev.charid(), 0, SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_SYSTEM), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_RECHARGE), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_GET_BY_RECHARGE), EMAILTYPE_NORMAL, dwMailID, vecItems, vecItemDatas,EMAILATTACHTYPE_ITEM, false, params, 0, 0, rev.charge(), true, qwQuota) == false)
  {
    XERR << "[充值-离线],充值存到离线邮件失败 order id" << rev.orderid() << "charge" << rev.charge() << "charid" << rev.charid() << "itemid" << rev.itemid() << "totalcount" << rev.count() <<"quota"<< qwQuota << XEND;
    return false;
  }
  XINF << "[充值-离线],充值存到离线邮件 order id" << rev.orderid() << "charge" << rev.charge() << "元，charid" << rev.charid() << "itemid" << rev.itemid() << "totalcount" << rev.count() << "quota" << qwQuota << XEND;
  
  return true;
}

bool MailManager::sendWeddingEventMsgMail(QWORD qwTargetID, const Cmd::WeddingEventMsgCCmd& cmd)
{
  ServerType svrType = thisServer->getServerType();
  //只有weddingserver 才可以发送
  if (svrType != SERVER_TYPE_WEDDING)
  {
    return false;
  }  
  SendMail mailCmd;
  mailCmd.mutable_data()->set_id(0);
  mailCmd.mutable_data()->set_receiveid(qwTargetID);
  mailCmd.mutable_data()->set_type(EMAILTYPE_WEDDINGMSG);
  mailCmd.mutable_data()->set_title(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_WEDDING_MSG));
  mailCmd.mutable_data()->set_sender(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_WEDDING_GM));
  mailCmd.mutable_data()->set_time(xTime::getCurSec());
  mailCmd.mutable_data()->mutable_weddingmsg()->CopyFrom(cmd);
  //先插入数据库
  insertNormalMailToDB(mailCmd.mutable_data());
  PROTOBUF(mailCmd, send, len);
  
  //再转发给相应的sessionserver
  RegionServer* pServer = dynamic_cast<RegionServer*>(thisServer);
  if (!pServer)
  {
    return false;
  }
  return pServer->sendMail(mailCmd);
}

bool MailManager::addEventMail(QWORD qwTargetID, const UserEventMailCmd& cmd)
{
  ServerType svrType = thisServer->getServerType();
  if (svrType == SERVER_TYPE_SCENE)
    return false;

  SendMail mailCmd;
  mailCmd.mutable_data()->set_id(0);
  mailCmd.mutable_data()->set_receiveid(qwTargetID);
  mailCmd.mutable_data()->set_type(EMAILTYPE_USEREVENT);
  mailCmd.mutable_data()->set_title(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_EVENT_MAIL));
  mailCmd.mutable_data()->set_sender(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_SYSTEM));
  mailCmd.mutable_data()->set_time(xTime::getCurSec());
  mailCmd.mutable_data()->mutable_eventmsg()->CopyFrom(cmd);
  //插入数据库
  insertNormalMailToDB(mailCmd.mutable_data());

  XLOG << "[邮件-事件邮件], 添加成功, 玩家id:" << qwTargetID << "事件类型:" << cmd.etype() << XEND;
  return true;
}

void MailManager::addMailMsg(Cmd::SendMail &cmd, MsgParams params)
{
  MailAttach* pAttach = cmd.mutable_data()->mutable_attach()->add_attachs();
  if(pAttach == nullptr)
    return;

  MailMsgParam* pMsg = pAttach->mutable_msgdatas();
  if(pMsg == nullptr)
    return;

  pAttach->set_type(EAMILATTACHTYPE_MSG_PARAMS);
  pMsg->set_mailid(cmd.mutable_data()->mailid());
  const vector<MsgParam> msgParams = params.getMsgParams();
  for (auto &v: msgParams)
  {
    pMsg->add_params(v.param());
  }
}

void MailManager::translateMailMsg(MailData* pData, DWORD dwLanguage)
{
  if(pData == nullptr || dwLanguage == ELANGUAGE_CHINESE_SIMPLIFIED || dwLanguage == ELANGUAGE_Chinese)
    return;

  for(int i = 0; i < pData->attach().attachs_size(); ++i)
  {
    const MailAttach rMailAttach = pData->attach().attachs(i);
    if(rMailAttach.type() != EAMILATTACHTYPE_MSG_PARAMS)
      continue;

    DWORD dwMailID = rMailAttach.msgdatas().mailid();
    const MailBase* pMailBase = TableManager::getMe().getMailCFG(dwMailID);
    if (pMailBase == nullptr)
      return;

    pData->set_title(CommonConfig::getMe().getTranslateString(pMailBase->getTableString("name"), dwLanguage));
    MsgParams params;
    for(int j = 0; j < rMailAttach.msgdatas().params_size(); ++j)
      params.addString(CommonConfig::getMe().getTranslateString(rMailAttach.msgdatas().params(j), dwLanguage));

    string strMsg = CommonConfig::getMe().getTranslateString(pMailBase->getTableString("Desc"), dwLanguage);
    pData->set_msg(MsgParams::fmtString(strMsg, params));

    break;
  }
}
