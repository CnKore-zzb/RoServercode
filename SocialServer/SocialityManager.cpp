#include "SocialityManager.h"
#include "SocialServer.h"
#include "RecordCmd.pb.h"
#include "PlatLogManager.h"
#include "SocialCmd.pb.h"
#include "MiscConfig.h"
#include "UserCmd.h"
#include "TeamCmd.pb.h"
#include "UserConfig.h"
#include "CommonConfig.h"
#include "GSocial.h"
#include "RedisManager.h"
#include "MailManager.h"

// social manger
SocialityManager::SocialityManager()
{

}

SocialityManager::~SocialityManager()
{

}

void SocialityManager::final()
{
  DWORD curTime = xTime::getCurSec();
  for (auto &m : m_mapSocialData)
    m.second.save(curTime + SOCIAL_SAVE_TICK, m_bRecallOpen);
}

void SocialityManager::delChar(QWORD qwCharID)
{
  auto self = m_mapSocialData.find(qwCharID);
  if (self == m_mapSocialData.end())
  {
    loadSocialData(qwCharID);
    self = m_mapSocialData.find(qwCharID);
    if (self == m_mapSocialData.end())
      return;
  }

  const TMapSociality& mapSocial = self->second.getSocialityList();
  for (auto &m : mapSocial)
  {
    auto target = m_mapSocialData.find(m.second.getGUID());
    if (target != m_mapSocialData.end())
    {
      target->second.delSociality(qwCharID);
    }
    else
    {
      RemoveSocialitySocialCmd cmd;
      cmd.mutable_user()->set_charid(qwCharID);
      cmd.set_destid(m.second.getGUID());
      cmd.set_to_global(true);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToSession(send, len);
    }
  }

  m_mapSocialData.erase(self);

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "social");
  if (pField == nullptr)
  {
    XERR << "[社交管理-玩家移除] charid :" << qwCharID << "获取social数据表失败" << XEND;
    return;
  }

  stringstream sstr;
  sstr << "id = " << qwCharID;
  QWORD retcode = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
  if (retcode == QWORD_MAX)
  {
    XERR << "[社交管理-玩家移除] charid :" << qwCharID << "从数据库删除主键失败" << XEND;
    return;
  }

  sstr.str("");
  sstr << "destid = " << qwCharID;
  retcode = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
  if (retcode == QWORD_MAX)
  {
    XERR << "[社交管理-玩家移除] charid :" << qwCharID << "从数据库删除副键失败" << XEND;
    return;
  }

  XLOG << "[社交管理-玩家移除] charid :" << qwCharID << "玩家 成功从数据库删除" << XEND;
}

void SocialityManager::onUserOnline(const SocialUser& rUser)
{
  auto m = m_mapSocialData.find(rUser.charid());
  if (m == m_mapSocialData.end())
  {
    loadSocialData(rUser.charid());
    m = m_mapSocialData.find(rUser.charid());
    if (m == m_mapSocialData.end())
      return;
  }

  m->second.getGCharData().getByTutor();
  m->second.setZoneID(rUser.zoneid());
  m->second.setName(rUser.name());
  m->second.setOfflineTime(0);
  m->second.syncSocialList();
  m->second.refreshBlackStatus();

  updateSocialData(rUser.charid(), ESOCIALDATA_OFFLINETIME, 0, "");
  updateSocialData(rUser.charid(), ESOCIALDATA_ZONEID, rUser.zoneid(), "");

  m->second.querySocialNtfList();
  m->second.queryRecallList();

  XLOG << "[社交管理-上线]" << rUser.ShortDebugString() << "上线" << XEND;
}

void SocialityManager::onUserOffline(const SocialUser& rUser)
{
  removeLoad(rUser.charid());
  removeRequest(rUser.charid());

  auto m = m_mapSocialData.find(rUser.charid());
  if (m == m_mapSocialData.end())
    return;

  DWORD curTime = now();
  m->second.setOfflineTime(curTime);
  m->second.clearUpdate();
  m->second.save(curTime + SOCIAL_SAVE_TICK * 10, m_bRecallOpen);

  updateSocialData(rUser.charid(), ESOCIALDATA_OFFLINETIME, curTime, "");
  m_mapSocialData.erase(m);

  XLOG << "[社交管理-下线]" << rUser.ShortDebugString() << "下线" << XEND;
}

void SocialityManager::updateUserInfo(const UserInfoSyncSocialCmd& cmd)
{
  const SocialUser& rUser = cmd.info().user();
  GCharReader* pGChar = nullptr;
  UserSociality* pSocial = getUserSociality(rUser.charid());
  if (pSocial != nullptr)
    pGChar = &pSocial->getGCharData();

  for (int i = 0; i < cmd.info().datas_size(); ++i)
  {
    const UserData& rData = cmd.info().datas(i);
    switch (rData.type())
    {
      case EUSERDATATYPE_SEX:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_GENDER, rData.value(), "");
        if (pGChar != nullptr) pGChar->setGender(static_cast<EGender>(rData.value()));
        break;
      case EUSERDATATYPE_ROLELEVEL:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_LEVEL, rData.value(), "");
        if (pGChar != nullptr) pGChar->setBaseLevel(rData.value());
        break;
      case EUSERDATATYPE_PORTRAIT:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_PORTRAIT, rData.value(), "");
        if (pGChar != nullptr) pGChar->setPortrait(rData.value());
        break;
      case EUSERDATATYPE_FRAME:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_FRAME, rData.value(), "");
        if (pGChar != nullptr) pGChar->setFrame(rData.value());
        break;
      case EUSERDATATYPE_PROFESSION:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_PROFESSION, rData.value(), "");
        if (pGChar != nullptr) pGChar->setProfession(static_cast<EProfession>(rData.value()));
        break;
      case EUSERDATATYPE_BLINK:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_BLINK, rData.value(), "");
        if (pGChar != nullptr) pGChar->setBlink(rData.value());
        break;
      /*case EUSERDATATYPE_MAPID:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_MAP, rData.value(), "");
        break;*/
      case EUSERDATATYPE_MANUAL_LV:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_ADVENTURELV, rData.value(), "");
        if (pGChar != nullptr) pGChar->setManualLv(rData.value());
        break;
      /*case EUSERDATATYPE_MANUAL_EXP:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_ADVENTUREEXP, rData.value(), "");
        break;*/
      case EUSERDATATYPE_CUR_TITLE:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_APPELLATION, rData.value(), "");
        if (pGChar != nullptr) pGChar->setTitleID(rData.value());
        break;
      case EUSERDATATYPE_BODY:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_BODY, rData.value(), "");
        if (pGChar != nullptr) pGChar->setBody(rData.value());
        break;
      case EUSERDATATYPE_HAIR:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_HAIR, rData.value(), "");
        if (pGChar != nullptr) pGChar->setHair(rData.value());
        break;
      case EUSERDATATYPE_HAIRCOLOR:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_HAIRCOLOR, rData.value(), "");
        if (pGChar != nullptr) pGChar->setHairColor(rData.value());
        break;
      case EUSERDATATYPE_NAME:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_NAME, rData.value(), rData.data());
        if (pGChar != nullptr) pGChar->setName(rData.data());
        break;
      case EUSERDATATYPE_HEAD:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_HEAD, rData.value(), "");
        if (pGChar != nullptr) pGChar->setHead(rData.value());
        break;
      case EUSERDATATYPE_FACE:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_FACE, rData.value(), "");
        if (pGChar != nullptr) pGChar->setFace(rData.value());
        break;
      case EUSERDATATYPE_MOUTH:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_MOUTH, rData.value(), "");
        if (pGChar != nullptr) pGChar->setMouth(rData.value());
        break;
      case EUSERDATATYPE_TUTOR_PROFIC:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_TUTOR_PROFIC, rData.value(), "");
        if (pGChar != nullptr) pGChar->setProfic(rData.value());
        break;
      case EUSERDATATYPE_TUTOR_ENABLE:
        if (pGChar != nullptr) pGChar->setTutor(rData.value());
        break;
      case EUSERDATATYPE_EYE:
        updateSocialData(cmd.info().user().charid(), ESOCIALDATA_EYE, rData.value(), "");
        break;
      /*case EUSERDATATYPE_QUERYTYPE:
        {
          UserSociality* pUserSocial = getUserSociality(cmd.info().user().charid());
          if (pUserSocial != nullptr)
          {
            pUserSocial->setQueryType(static_cast<EQueryType>(rData.value()));
            XLOG << "[社交管理-设置querytype]" << cmd.info().user().charid() << "type :" << pUserSocial->getQueryType() << XEND;
          }
        }
        break;*/
      default:
        XDBG << "[社交-数据同步]" << rUser.ShortDebugString() << "收到数据更新" << rData.ShortDebugString() << "未处理" << XEND;
        break;
    }
    XDBG << "[社交-数据同步]" << rUser.ShortDebugString() << "收到数据更新" << rData.ShortDebugString() << XEND;
  }
}

bool SocialityManager::frameStatus(QWORD qwGUID, bool bFrame)
{
  auto m = m_mapSocialData.find(qwGUID);
  if (m == m_mapSocialData.end())
    return false;

  m->second.setFrame(bFrame);
  return true;
}

bool SocialityManager::addRelation(QWORD qwGUID, QWORD qwDestGUID, ESocialRelation eRelation, bool bCheck /*= true*/)
{
  if (qwGUID == qwDestGUID)
  {
    XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 不能添加自己" << XEND;
    return false;
  }

  switch (eRelation)
  {
    case ESOCIALRELATION_MIN:
    case ESOCIALRELATION_MAX:
    case ESOCIALRELATION_FRIEND:
    case ESOCIALRELATION_MERRY:
    case ESOCIALRELATION_CHAT:
    case ESOCIALRELATION_TEAM:
    case ESOCIALRELATION_APPLY:
    case ESOCIALRELATION_BLACK:
    case ESOCIALRELATION_BLACK_FOREVER:
    case ESOCIALRELATION_RECALL:
    case ESOCIALRELATION_BERECALL:
      break;
    case ESOCIALRELATION_TUTOR:
    case ESOCIALRELATION_TUTOR_APPLY:
    case ESOCIALRELATION_STUDENT:
    case ESOCIALRELATION_STUDENT_APPLY:
    case ESOCIALRELATION_STUDENT_RECENT:
      return addTutor(qwGUID, qwDestGUID, eRelation, bCheck);
    case ESOCIALRELATION_TUTOR_PUNISH:
    case ESOCIALRELATION_TUTOR_CLASSMATE:
      XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败,无法手动添加" << XEND;
      return false;
  }

  UserSociality* pUserSocial = getUserSociality(qwGUID);
  if (pUserSocial == nullptr)
  {
    if (isLoad(qwGUID) == true)
    {
      XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败,正在加载数据中" << XEND;
      return false;
    }

    AddRelationSocialCmd cmd;
    cmd.mutable_user()->set_charid(qwGUID);
    cmd.set_destid(qwDestGUID);
    cmd.mutable_user()->set_zoneid(thisServer->getZoneID());
    cmd.set_relation(eRelation);
    cmd.set_to_global(true);
    cmd.set_check(bCheck);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
    XLOG << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "未发现对方,发送至对方线处理" << XEND;
    return true;
  }

  // check valid
  const SSocialityMiscCFG& rCFG = MiscConfig::getMe().getSocialCFG();
  if (eRelation == ESOCIALRELATION_FRIEND)
  {
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_FRIEND) >= rCFG.dwMaxFriendCount)
    {
      XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 超过好友最大上限" << rCFG.dwMaxFriendCount << XEND;
      return false;
    }
  }
  else if (eRelation == ESOCIALRELATION_APPLY)
  {
    Sociality* pSocial = pUserSocial->getSociality(qwDestGUID);
    if (pSocial != nullptr)
    {
      if (pSocial->checkRelation(ESOCIALRELATION_FRIEND) == true)
      {
        XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 已是好友 无法申请" << XEND;
        return false;
      }
      if (pSocial->checkRelation(ESOCIALRELATION_BLACK) == true || pSocial->checkRelation(ESOCIALRELATION_BLACK_FOREVER) == true)
      {
        XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 已是黑名单 无法申请" << XEND;
        return false;
      }
    }
  }
  else if (eRelation == ESOCIALRELATION_BLACK || eRelation == ESOCIALRELATION_BLACK_FOREVER)
  {
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_BLACK) >= rCFG.dwMaxBlackCount)
    {
      XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 超过好友最大上限" << rCFG.dwMaxFriendCount << XEND;
      return false;
    }
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_BLACK_FOREVER) >= rCFG.dwMaxBlackCount)
    {
      XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 超过好友最大上限" << rCFG.dwMaxFriendCount << XEND;
      return false;
    }
  }
  else if (eRelation == ESOCIALRELATION_RECALL)
  {
    if (bCheck)
    {
      if (pUserSocial->isBeRecall(qwDestGUID) == false)
      {
        AddRelationResultSocialCmd cmd;
        cmd.set_charid(qwDestGUID);
        cmd.set_relation(eRelation);
        cmd.set_success(false);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToMe(pUserSocial->getZoneID(), pUserSocial->getGUID(), send, len);
        XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 未在召回列表内" << XEND;
        return false;
      }

      GCharReader oTarget(thisServer->getRegionID(), qwDestGUID);
      UserSociality* pTargetSocial = getUserSociality(qwDestGUID);
      if (pTargetSocial != nullptr)
        oTarget = pTargetSocial->getGCharData();
      else
      {
        if (oTarget.getBySocial() != EERROR_SUCCESS)
        {
          XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 未获取到对方信息" << XEND;
          return false;
        }
      }

      const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
      if (oTarget.getRecallCount() >= rCFG.dwMaxRecallCount)
      {
        AddRelationResultSocialCmd cmd;
        cmd.set_charid(qwDestGUID);
        cmd.set_relation(eRelation);
        cmd.set_success(false);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToMe(pUserSocial->getZoneID(), pUserSocial->getGUID(), send, len);
        thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), 3618);
        XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 对方契约关系" << oTarget.getRecallCount() << "超过上限" << rCFG.dwMaxRecallCount << XEND;
        return false;
      }
    }
  }

  // add relation
  if (pUserSocial->addRelation(qwDestGUID, eRelation, bCheck) == false)
  {
    XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "失败, 添加关系失败" << XEND;
    return false;
  }

  // do after adding relation
  if (eRelation == ESOCIALRELATION_FRIEND)
  {
    addRelation(qwDestGUID, qwGUID, ESOCIALRELATION_APPLY);
  }
  else if (eRelation == ESOCIALRELATION_APPLY)
  {
    SyncRedTip(qwDestGUID, qwGUID, EREDSYS_SOCIAL_FRIEND_APPLY, true);
  }
  if (eRelation == ESOCIALRELATION_RECALL)
  {
    AddRelationResultSocialCmd cmd;
    cmd.set_charid(qwDestGUID);
    cmd.set_relation(eRelation);
    cmd.set_success(true);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(pUserSocial->getZoneID(), pUserSocial->getGUID(), send, len);

    do
    {
      const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
      const MailBase* pBase = TableManager::getMe().getMailCFG(rCFG.dwContractMailID);
      if (pBase == nullptr)
      {
        XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "成功,发送契约邮件" << rCFG.dwRecallMailID << "失败,未在 Table_Mail.txt 表中找到" << XEND;
        break;
      }

      if (MailManager::getMe().sendMail(qwGUID, rCFG.dwContractMailID) == false)
      {
        XERR << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "成功,发送契约邮件" << rCFG.dwRecallMailID << "失败" << XEND;
        break;
      }
      XLOG << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "成功,发送契约邮件" << rCFG.dwRecallMailID << "成功" << XEND;
    }while(0);

    addRelation(qwDestGUID, qwGUID, ESOCIALRELATION_RECALL, false);
  }

  XDBG << "[社交管理-添加关系]" << qwGUID << "和" << qwDestGUID << "添加关系 :" << eRelation << "成功" << XEND;

  if (eRelation == ESOCIALRELATION_FRIEND)
  {
    //log
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_AddFriend;
    PlatLogManager::getMe().eventLog(thisServer,
        thisServer->getPlatformID(),
        thisServer->getRegionID(),
        0,
        qwGUID,
        eid,
        0,  // charge
        eType, 0, 1);

    PlatLogManager::getMe().SocialLog(thisServer,
        thisServer->getPlatformID(),
        thisServer->getRegionID(),
        0,
        qwGUID,
        eType,
        eid,
        ESocial_AddFriend,
        0,
        qwDestGUID,
        0,
        0);
  }

  return true;
}

bool SocialityManager::removeRelation(QWORD qwGUID, QWORD qwDestGUID, ESocialRelation eRelation, bool bCheck /*= false*/)
{
  if (qwGUID == qwDestGUID)
  {
    XERR << "[社交管理-移除关系] " << qwGUID << " 和 " << qwDestGUID << " 移除关系 : " << eRelation << " 失败";
    return false;
  }

  UserSociality* pUserSocial = getUserSociality(qwGUID);
  if (pUserSocial == nullptr)
  {
    if (isLoad(qwGUID) == true)
    {
      XERR << "[社交管理-移除关系]" << qwGUID << "和" << qwDestGUID << "移除关系 :" << eRelation << "失败,正在加载数据中" << XEND;
      return false;
    }

    GCharReader oReader(thisServer->getRegionID(), qwGUID);
    oReader.getByTutor();
    if (oReader.checkRelation(qwDestGUID, eRelation) == false)
    {
      XERR << "[社交管理-移除关系]" << qwGUID << "和" << qwDestGUID << "移除关系 :" << eRelation << "失败,无该关系" << XEND;
      return false;
    }

    RemoveRelationSocialCmd cmd;
    cmd.mutable_user()->set_charid(qwGUID);
    cmd.mutable_user()->set_zoneid(thisServer->getZoneID());
    cmd.set_destid(qwDestGUID);
    cmd.set_relation(eRelation);
    cmd.set_to_global(true);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
    XLOG << "[社交管理-移除关系]" << qwGUID << "和" << qwDestGUID << "移除关系 :" << eRelation << "未发现对方,发送至对方线处理" << XEND;
    return true;
  }

  if (eRelation == ESOCIALRELATION_APPLY && qwDestGUID == 0)
  {
    if (pUserSocial->removeRelation(eRelation, ESRMETHOD_ALL) == false)
    {
      XERR << "[社交管理-移除关系] " << qwGUID << " 和 " << qwDestGUID << " 移除所有申请关系 : " << eRelation << " 失败" << XEND;
      return false;
    }

    return true;
  }

  if (pUserSocial->removeRelation(qwDestGUID, eRelation, bCheck) == false)
  {
    XERR << "[社交管理-移除关系] " << qwGUID << " 和 " << qwDestGUID << " 移除关系 : " << eRelation << " 失败, 移除关系失败" << XEND;
    return false;
  }

  if (eRelation == ESOCIALRELATION_STUDENT)
  {
    TSetQWORD setStudentIDs;
    pUserSocial->collectID(setStudentIDs, ESOCIALRELATION_STUDENT);
    for (auto &s : setStudentIDs)
    {
      if (s != qwDestGUID)
      {
        removeRelation(s, qwDestGUID, ESOCIALRELATION_TUTOR_CLASSMATE);
        removeRelation(qwDestGUID, s, ESOCIALRELATION_TUTOR_CLASSMATE);
      }
    }
  }

  if (eRelation == ESOCIALRELATION_FRIEND)
  {
    //log 
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_DelFriend;
    PlatLogManager::getMe().eventLog(thisServer,
      thisServer->getPlatformID(),
      thisServer->getRegionID(),
      0,
      qwGUID,
      eid,
      0,  // charge
      eType, 0, 1);

    PlatLogManager::getMe().SocialLog(thisServer,
      thisServer->getPlatformID(),
      thisServer->getRegionID(),
      0,
      qwGUID,
      eType,
      eid,
      ESocial_DelFriend,
      0,
      qwDestGUID,
      0,
      0);
  }

  XDBG << "[社交管理-移除关系]" << qwGUID << "和" << qwDestGUID << "移除关系 :" << eRelation << "成功" << XEND;
  return true;
}

bool SocialityManager::checkRelation(QWORD qwCharID, QWORD qwDestCharID, ESocialRelation eRelation)
{
  if (qwCharID == qwDestCharID)
    return false;

  auto m = m_mapSocialData.find(qwCharID);
  if (m == m_mapSocialData.end())
    return false;
  Sociality* pSociality = m->second.getSociality(qwDestCharID);
  if (pSociality == nullptr)
    return false;

  return pSociality->checkRelation(eRelation);
}

bool SocialityManager::updateSocialData(QWORD qwGUID, ESocialData eType, DWORD dwValue, const string& strName)
{
  if (eType <= ESOCIALDATA_MIN || eType >= ESOCIALDATA_MAX)
    return false;

  TSetQWORD setUpdateIDs;
  auto m = m_mapSocialData.find(qwGUID);
  if (m != m_mapSocialData.end())
    m->second.collectID(setUpdateIDs);

  return updateSocialData(qwGUID, setUpdateIDs, eType, dwValue, strName);
}

bool SocialityManager::updateSocialData(QWORD qwGUID, const TSetQWORD& setUpdateIDs, ESocialData eType, DWORD dwValue, const string& strName)
{
  auto update = [](Sociality& rData, ESocialData eType, DWORD dwValue, const string& strName) -> bool
  {
    bool bUpdate = false;
    switch (eType)
    {
      case ESOCIALDATA_MIN:
        break;
      case ESOCIALDATA_LEVEL:
        bUpdate = rData.getLevel() != dwValue;
        if (bUpdate)
          rData.setLevel(dwValue);
        break;
      case ESOCIALDATA_OFFLINETIME:
        bUpdate = rData.getOfflineTime() != dwValue;
        if (bUpdate)
          rData.setOfflineTime(dwValue);
        break;
      case ESOCIALDATA_RELATION:
        bUpdate = rData.getRelation() != dwValue;
        if (bUpdate)
          rData.setRelation(dwValue);
        break;
      case ESOCIALDATA_PROFESSION:
        bUpdate = rData.getProfession() != dwValue;
        if (bUpdate)
          rData.setProfession(static_cast<EProfession>(dwValue));
        break;
      case ESOCIALDATA_PORTRAIT:
        bUpdate = rData.getPortrait() != dwValue;
        if (bUpdate)
          rData.setPortrait(dwValue);
        break;
      case ESOCIALDATA_FRAME:
        bUpdate = rData.getFrame() != dwValue;
        if (bUpdate)
          rData.setFrame(dwValue);
        break;
      case ESOCIALDATA_HAIR:
        bUpdate = rData.getHair() != dwValue;
        if (bUpdate)
          rData.setHair(dwValue);
        break;
      case ESOCIALDATA_HAIRCOLOR:
        bUpdate = rData.getHairColor() != dwValue;
        if (bUpdate)
          rData.setHairColor(dwValue);
        break;
      case ESOCIALDATA_BODY:
        bUpdate = rData.getBody() != dwValue;
        if (bUpdate)
          rData.setBody(dwValue);
        break;
      case ESOCIALDATA_ADVENTURELV:
        bUpdate = rData.getAdventureLv() != dwValue;
        if (bUpdate)
          rData.setAdventureLv(dwValue);
        break;
      /*case ESOCIALDATA_ADVENTUREEXP:
        bUpdate = rData.getAdventureExp() != dwValue;
        if (bUpdate)
          rData.setAdventureExp(dwValue);
        break;*/
      case ESOCIALDATA_APPELLATION:
        bUpdate = rData.getAppellation() != dwValue;
        if (bUpdate)
          rData.setAppellation(dwValue);
        break;
      case ESOCIALDATA_GENDER:
        bUpdate = rData.getGender() != dwValue;
        if (bUpdate)
          rData.setGender(static_cast<EGender>(dwValue));
        break;
      case ESOCIALDATA_GUILDNAME:
        bUpdate = rData.getGuildName() != strName;
        if (bUpdate)
          rData.setGuildName(strName);
        break;
      case ESOCIALDATA_GUILDPORTRAIT:
        bUpdate = rData.getGuildPortrait() != strName;
        if (bUpdate)
          rData.setGuildPortrait(strName);
        break;
      /*case ESOCIALDATA_MAP:
        bUpdate = rData.getMapID() != dwValue;
        if (bUpdate)
          rData.setMapID(dwValue);
        break;*/
      case ESOCIALDATA_ZONEID:
        bUpdate = rData.getZoneID() != dwValue;
        if (bUpdate)
          rData.setZoneID(dwValue);
        break;
      case ESOCIALDATA_BLINK:
        bUpdate = rData.getBlink() != dwValue;
        if (bUpdate)
          rData.setBlink(dwValue);
        break;
      case ESOCIALDATA_NAME:
        bUpdate = rData.getName() != strName;
        if (bUpdate)
          rData.setName(strName);
        break;
      case ESOCIALDATA_CREATETIME:
        bUpdate = true;
        break;
      case ESOCIALDATA_HEAD:
        bUpdate = rData.getHead() != dwValue;
        if (bUpdate)
          rData.setHead(dwValue);
        break;
      case ESOCIALDATA_FACE:
        bUpdate = rData.getFace() != dwValue;
        if (bUpdate)
          rData.setFace(dwValue);
        break;
      case ESOCIALDATA_MOUTH:
        bUpdate = rData.getMouth() != dwValue;
        if (bUpdate)
          rData.setMouth(dwValue);
        break;
      case ESOCIALDATA_TUTOR_PROFIC:
        bUpdate = rData.getProfic() != dwValue;
        if (bUpdate)
          rData.setProfic(dwValue);
        break;
      case ESOCIALDATA_EYE:
        bUpdate = rData.getEye() != dwValue;
        if (bUpdate)
          rData.setEye(dwValue);
        break;
      case ESOCIALDATA_CANRECALL:
        bUpdate = true;
        rData.setMark(eType);
        break;
      case ESOCIALDATA_MAX:
        break;
      default:
        break;
    }

    return bUpdate;
  };

  for (auto s = setUpdateIDs.begin(); s != setUpdateIDs.end(); ++s)
  {
    auto mf = m_mapSocialData.find(*s);
    if (mf == m_mapSocialData.end())
    {
      if (isLoad(*s) == false)
      {
        SocialDataUpdateSocialCmd cmd;
        cmd.set_charid(*s);
        cmd.set_targetid(qwGUID);
        cmd.set_to_global(true);
        SocialDataItem* pItem = cmd.mutable_update()->add_items();
        pItem->set_type(eType);
        pItem->set_value(dwValue);
        pItem->set_data(strName);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
        XDBG << "[社交管理-数据更新]" << *s << "收到" << qwGUID << "更新数据 type :" << eType << "value :" << dwValue << "name :" << strName << "未找到,发送至SessionServer尝试"<< XEND;
      }
      continue;
    }

    Sociality* pSocial = mf->second.getSociality(qwGUID);
    if (pSocial == nullptr || pSocial->getRelation() == ESOCIALRELATION_MIN)
      continue;
    if (update(*pSocial, eType, dwValue, strName) == false)
      continue;

    /*if (eType == ESOCIALDATA_RELATION)
      mf->second.addUpdateID(qwGUID);*/

    if (mf->second.getFrame() == true)
      mf->second.reqUpdate();

    XDBG << "[社交管理-数据更新]" << *s << "收到好友" << qwGUID << "更新数据 type :" << eType << "value :" << dwValue << "name :" << strName << XEND;
  }

  return true;
}

bool SocialityManager::findUser(QWORD id, const string& keyword)
{
  QWORD qwGUID = id;
  if (keyword.empty() == true)
  {
    XERR << "[社交管理-查找]" << qwGUID << "查找 keyword :" << keyword << "失败,没有关键字" << XEND;
    return false;
  }

  bool bIsID = true;
  for (auto s = keyword.begin(); s != keyword.end(); ++s)
  {
    if (isdigit(*s) == false)
    {
      bIsID = false;
      break;
    }
  }

  DWORD dwCount = 0;
  auto create = [&](SocialData* pData, const GCharReader* pInfo)
  {
    if (pData == nullptr || pInfo == nullptr)
      return;

    pData->set_guid(pInfo->getCharID());
    pData->set_accid(pInfo->getAccID());

    pData->set_level(pInfo->getBaseLevel());
    pData->set_portrait(pInfo->getPortrait());
    pData->set_frame(pInfo->getFrame());
    pData->set_hair(pInfo->getHair());
    pData->set_haircolor(pInfo->getHairColor());
    pData->set_body(pInfo->getBody());
    pData->set_head(pInfo->getHead());
    pData->set_face(pInfo->getFace());
    pData->set_mouth(pInfo->getMouth());
    pData->set_eye(pInfo->getEye());

    pData->set_offlinetime(pInfo->getOfflineTime());
    pData->set_profession(static_cast<EProfession>(pInfo->getProfession()));
    pData->set_gender(static_cast<EGender>(pInfo->getGender()));

    pData->set_blink(pInfo->getBlink());
    pData->set_zoneid(getClientZoneID(pInfo->getZoneID()));

    pData->set_name(pInfo->getName());

    pData->set_guildname(pInfo->getGuildName());
    pData->set_guildportrait(pInfo->getGuildPortrait());

    // init body for old account
    if (pData->body() == 0)
    {
      const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(pData->profession());
      if (pCFG != nullptr)
      {
        if (pData->gender() == EGENDER_MALE)
          pData->set_body(pCFG->maleBody);
        else if (pData->gender() == EGENDER_FEMALE)
          pData->set_body(pCFG->femaleBody);
      }
    }
    // init haircolor for old account
    pData->set_haircolor(pData->haircolor() == 0 ? 3 : pData->haircolor());
    // init hairstyle for old account
    if (pData->hair() == 0)
    {
      if (pData->gender() == EGENDER_MALE)
        pData->set_hair(998);
      else if (pData->gender() == EGENDER_FEMALE)
        pData->set_hair(999);
    }

    ++dwCount;
  };

  FindUser cmd;
  if (bIsID)
  {
    do
    {
      QWORD qwCharID = atoll(keyword.c_str());
      GCharReader pGChar(thisServer->getRegionID(), qwCharID);
      if (pGChar.getBySocial() != EERROR_SUCCESS)
      {
        XLOG << "[社交管理-查找]" << qwGUID << "查找 keyword :" << keyword << "为指定id查找, 未查找到数据" << XEND;
        break;
      }

      create(cmd.add_datas(), &pGChar);
    } while (0);
  }
  if (cmd.datas_size() <= 0)
  {
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (field == nullptr)
    {
      XERR << "[社交管理-查找]" << qwGUID << "查找 keyword :" << keyword << "失败,未找到 charbase 数据表" << XEND;
      return false;
    }
    field->setValid("charid, onlinetime");

    char where[64] = { 0 };
    const SSocialityMiscCFG& rCFG = MiscConfig::getMe().getSocialCFG();

    //转义
    string keyword2 = keyword;
    replaceAll(keyword2, "\\", "");
    replaceAll(keyword2, "_", "");
    replaceAll(keyword2, "%", "");

    snprintf(where, sizeof(where), "name like \"%s%%\" limit %u", keyword2.c_str(), rCFG.dwMaxFindCount);
    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX != ret)
    {
      for (QWORD q = 0; q < ret; ++q)
      {
        if (set[q].get<QWORD>("charid") == id)
          continue;
        DWORD dwOnlineTime = set[q].get<DWORD>("onlinetime");
        if (dwOnlineTime == 0)
          continue;

        QWORD qwCharID = set[q].get<QWORD>("charid");
        GCharReader pGChar(thisServer->getRegionID(), qwCharID);
        if (pGChar.getBySocial() != EERROR_SUCCESS)
        {
          XERR << "[社交管理-查找]" << qwGUID << "查找 keyword :" << keyword << "模糊查找,未获取 charid :" << qwCharID << "数据" << XEND;
          continue;
        }
        create(cmd.add_datas(), &pGChar);
      }
    }
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(0, id, send, len);

  XDBG << "[社交管理-好友查询]" << qwGUID << "使用 keyword :" << keyword << "查询到了" << cmd.datas_size() << "个结果" << XEND;
  return true;
}

bool SocialityManager::recallFriend(DWORD zoneid, QWORD charid, QWORD targetid)
{
  if (getRecallActivityOpen() == false)
  {
    thisServer->sendDebugMsg(zoneid, charid, "测试log:回归活动未开启");
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,活动未开启" << XEND;
    return false;
  }

  UserSociality* pSocial = getUserSociality(charid);
  if (pSocial == nullptr)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,未找到自己社交对象" << XEND;
    return false;
  }

  if (pSocial->isRecall(targetid) == true)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,对方已在召回列表中" << XEND;
    return false;
  }

  Sociality* pSociality = pSocial->getSociality(targetid);
  if (pSociality == nullptr)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,自己和对方无任何社交关系" << XEND;
    return false;
  }
  if (pSociality->checkRelation(ESOCIALRELATION_FRIEND) == false)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,自己和对方不是好友关系" << XEND;
    return false;
  }

  const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
  if (pSocial->getGCharData().getRecallCount() >= rCFG.dwMaxRecallCount)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,自己签订契约已达上限" << rCFG.dwMaxRecallCount << XEND;
    return false;
  }

  GCharReader oTarget(thisServer->getRegionID(), targetid);
  UserSociality* pUserSocial = getUserSociality(targetid);
  if (pUserSocial != nullptr)
    oTarget = pUserSocial->getGCharData();
  else
  {
    if (oTarget.getBySocial() != EERROR_SUCCESS)
    {
      XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,未查询到对方redis数据" << XEND;
      return false;
    }
  }

  if (oTarget.getRelationCount(ESOCIALRELATION_BERECALL) >= 1)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,对方已与其他玩家签订契约" << XEND;
    return false;
  }
  if (oTarget.checkRelation(charid, ESOCIALRELATION_FRIEND) == false)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,对方与自己不是好友关系" << XEND;
    return false;
  }
  if (xTime::getCurSec() - oTarget.getOfflineTime() < rCFG.dwNeedOffline)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,对方离线时间未超过" << rCFG.dwNeedOffline << "秒" << XEND;
    return false;
  }
  if (oTarget.getBaseLevel() < rCFG.dwNeedBaseLv)
  {
    XERR << "[社交管理-好友召回]" << charid << "召回" << targetid << "失败,对方等级未达到" << rCFG.dwNeedBaseLv << "级" << XEND;
    return false;
  }

  pSocial->addRecall(targetid);

  SocialDataUpdate cmd;
  cmd.set_guid(targetid);

  SocialDataItem* pItem = cmd.add_items();
  pItem->set_type(ESOCIALDATA_CANRECALL);
  pItem->set_value(0);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(0, charid, send, len);
  XLOG << "[社交管理-好友召回]" << charid << "召回" << targetid << "成功" << XEND;
  return true;
}

void SocialityManager::setRecallActivityOpen(bool bOpen)
{
  m_bRecallOpen = bOpen;
  XLOG << "[社交-回归活动]" << (bOpen ? "开启" : "关闭") << XEND;
}

UserSociality* SocialityManager::getUserSociality(QWORD qwCharID)
{
  if (qwCharID == 0)
    return nullptr;
  auto m = m_mapSocialData.find(qwCharID);
  if (m != m_mapSocialData.end())
    return &m->second;
  /*loadSocialData(qwCharID);
  m = m_mapSocialData.find(qwCharID);
  if (m != m_mapSocialData.end())
    return &m->second;*/
  return nullptr;
}

bool SocialityManager::loadSocialData(QWORD qwGUID)
{
  auto m = m_mapSocialData.find(qwGUID);
  if (m != m_mapSocialData.end())
  {
    XERR << "[社交管理-加载]" << qwGUID << "加载失败,数据已加载" << XEND;
    return true;
  }

  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "social");
  if (field == nullptr)
  {
    XERR << "[社交管理-加载]" << qwGUID << "加载失败,获取数据库" << REGION_DB << "social 失败" << XEND;
    return false;
  }
  field->setValid("id, destid, relation, relationtime");

  stringstream sstr;
  sstr << "id = " << qwGUID;

  xRecordSet set;
  QWORD retcode = thisServer->getDBConnPool().exeSelect(field, set, sstr.str().c_str());
  if (retcode == QWORD_MAX)
  {
    XERR << "[社交管理-加载]" << qwGUID << "加载失败,数据库查询失败 ret :" << retcode << XEND;
    return false;
  }

  UserSociality& rSocial = m_mapSocialData[qwGUID];
  rSocial.getGCharData().getBySocial();
  rSocial.setGUID(qwGUID);

  for (QWORD q = 0; q < retcode; ++q)
  {
    QWORD id = set[q].get<QWORD>("id");
    QWORD destid = set[q].get<QWORD>("destid");
    if (id == destid)
    {
      XERR << "[社交管理-加载]" << qwGUID << "加载社交成员" << destid << "失败,是自己,忽略" << XEND;
      rSocial.addSaveID(destid);
      continue;
    }

    Sociality oSocial;
    oSocial.setGUID(destid);
    if (oSocial.fromData(set[q]) == false)
    {
      XERR << "[社交管理-加载]" << qwGUID << "加载社交成员" << destid << "失败,序列化失败" << XEND;
      rSocial.addSaveID(destid);
      continue;
    }

    if (rSocial.addSociality(oSocial) == false)
    {
      XERR << "[社交管理-加载]" << qwGUID << "加载社交成员" << destid << "失败,添加失败" << XEND;
      rSocial.addSaveID(destid);
    }
  }

  const SSocialityMiscCFG& rCFG = MiscConfig::getMe().getSocialCFG();
  if (rSocial.getSocialityCount(ESOCIALRELATION_TEAM) > rCFG.dwMaxNearTeamCount)
    rSocial.removeRelation(ESOCIALRELATION_TEAM, ESRMETHOD_TIME_MIN);
  if (rSocial.getSocialityCount(ESOCIALRELATION_CHAT) > rCFG.dwMaxChatCount)
    rSocial.removeRelation(ESOCIALRELATION_CHAT, ESRMETHOD_TIME_MIN);
  rSocial.removeRelation(ESOCIALRELATION_RECALL, ESRMETHOD_CONTRACT_OVERTIME);  
  rSocial.removeRelation(ESOCIALRELATION_BERECALL, ESRMETHOD_CONTRACT_OVERTIME);

  XDBG << "[社交管理-加载]" << qwGUID << "成功成功,加载" << rSocial.getSocialityList().size() << "社交关系" << XEND;

  rSocial.loadRecall();
  return true;
}

void SocialityManager::timer(DWORD dwCurTime)
{
  xTime frameTime;
  for (auto &m : m_mapSocialData)
  {
    if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwSocialLoadTime)
    {
      XLOG << "[社交管理-帧] 保存耗时" << frameTime.uElapse() << "超过" << CommonConfig::m_dwSocialLoadTime << "跳出循环" << XEND;
      break;
    }
    m.second.save(dwCurTime, m_bRecallOpen);
  }
}

void SocialityManager::loadtimer(DWORD curTime)
{
  processLoad();
  processReq();
}

void SocialityManager::SyncRedTip(QWORD dwid, QWORD charid, ERedSys redsys, bool opt)
{
  SyncRedTipSocialCmd cmd;
  cmd.set_dwid(dwid);
  cmd.set_charid(charid);
  cmd.set_red(redsys);
  cmd.set_add(opt);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

bool SocialityManager::addTutor(QWORD qwGUID, QWORD qwDestGUID, ESocialRelation eRelation, bool bConfim /*= true*/)
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_TUTOR) == true)
    return false;
  if (qwGUID == qwDestGUID)
  {
    XERR << "[社交管理-添加关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败, 不能添加自己" << XEND;
    return false;
  }

  if (eRelation == ESOCIALRELATION_TUTOR_APPLY || eRelation == ESOCIALRELATION_STUDENT_APPLY)
  {
    GCharReader oUser(thisServer->getRegionID(), qwGUID);
    UserSociality* pUserSocial = getUserSociality(qwGUID);
    if (pUserSocial != nullptr)
      oUser = pUserSocial->getGCharData();
    else
      oUser.getByTutor();
    if (oUser.getOfflineTime() > oUser.getOnlineTime())
    {
      thisServer->sendMsg(oUser.getZoneID(), oUser.getCharID(), 3247);
      return false;
    }

    DWORD dwTemp = 0;
    RedisManager::getMe().getData<DWORD>(RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_REFUSE_PROTECT, qwGUID, qwDestGUID), dwTemp);
    if (dwTemp >= 3)
    {
      thisServer->sendMsg(oUser.getZoneID(), oUser.getCharID(), 3211);
      return false;
    }

    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_PUNISH, oUser.getCharID());
    DWORD dwPunish = 0;
    RedisManager::getMe().getData<DWORD>(key, dwPunish);
    if (dwPunish != 0)
    {
      if (eRelation == ESOCIALRELATION_TUTOR_APPLY)
        thisServer->sendMsg(oUser.getZoneID(), oUser.getCharID(), ESYSTEMMSG_ID_TUTOR_PUNISH_STUDENT_S);
      else
        thisServer->sendMsg(oUser.getZoneID(), oUser.getCharID(), ESYSTEMMSG_ID_TUTOR_PUNISH_TUTOR_S);
      return false;
    }

    GCharReader oTarget(thisServer->getRegionID(), qwDestGUID);
    UserSociality* pTarget = getUserSociality(qwDestGUID);
    if (pTarget != nullptr)
      oTarget = pTarget->getGCharData();
    else
      oTarget.getByTutor();
    if (oTarget.getOfflineTime() > oTarget.getOnlineTime())
    {
      thisServer->sendMsg(oUser.getZoneID(), oUser.getCharID(), 3247);
      return false;
    }

    QWORD qwTemp = qwDestGUID;
    qwDestGUID = qwGUID;
    qwGUID = qwTemp;
  }

  // get user
  UserSociality* pUserSocial = getUserSociality(qwGUID);
  if (pUserSocial == nullptr)
  {
    if (isLoad(qwGUID) == true)
    {
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,正在加载数据中" << XEND;
      return false;
    }

    AddRelationSocialCmd cmd;
    cmd.mutable_user()->set_charid(qwGUID);
    cmd.set_destid(qwDestGUID);
    cmd.mutable_user()->set_zoneid(thisServer->getZoneID());
    cmd.set_relation(eRelation);
    cmd.set_to_global(true);
    cmd.set_check(bConfim);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
    XLOG << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系,未发现对方,发送至对方线处理" << XEND;
    return true;
  }

  // get target gchar
  GCharReader oTarget(thisServer->getRegionID(), qwDestGUID);
  UserSociality* pTargetSocial = getUserSociality(qwDestGUID);
  if (pTargetSocial == nullptr)
    oTarget.getByTutor();
  else
    oTarget = pTargetSocial->getGCharData();

  const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();

  // check can add
  if (eRelation == ESOCIALRELATION_TUTOR_APPLY && bConfim)
  {
    // check student enable
    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_PUNISH, pUserSocial->getGUID());
    DWORD dwPunish = 0;
    RedisManager::getMe().getData<DWORD>(key, dwPunish);
    if (dwPunish != 0)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_PUNISH_STUDENT_T);
      return false;
    }

    if (pUserSocial->getBaseLevel() < rCFG.dwStudentBaseLvReq)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_NOOPEND, MsgParams(rCFG.dwTutorBaseLvReq));
      return false;
    }
    if (pUserSocial->getBaseLevel() >= rCFG.dwTutorBaseLvReq)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_STUDENT_LV_UP, MsgParams(rCFG.dwTutorBaseLvReq));
      return false;
    }
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_TUTOR) > 0)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_HAS_TUTOR);
      return false;
    }
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_TUTOR_APPLY) >= rCFG.dwMaxTutorApply)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), 3248);
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方导师申请列表超上限" << rCFG.dwMaxTutorApply << "个" << XEND;
      return false;
    }
    Sociality* pSocial = pUserSocial->getSociality(qwDestGUID);
    if (pSocial != nullptr &&
        (pSocial->checkRelation(ESOCIALRELATION_BLACK) == true || pSocial->checkRelation(ESOCIALRELATION_BLACK_FOREVER) == true || pSocial->checkRelation(eRelation) == true))
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_TAPPLY_SUCCESS);
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方在自己黑名单里或已存在该关系" << XEND;
      return false;
    }

    // check tutor enable
    if (oTarget.getBaseLevel() < rCFG.dwTutorBaseLvReq)
    {
      thisServer->sendDebugMsg(oTarget.getZoneID(), oTarget.getCharID(), "测试log:自己未达到导师资格等级");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方没有达到导师资格等级" << rCFG.dwTutorBaseLvReq << XEND;
      return false;
    }
    if (oTarget.getTutor() == false)
    {
      thisServer->sendDebugMsg(oTarget.getZoneID(), oTarget.getCharID(), "测试log:自己未完成导师资格任务");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方没有完成导师资格任务" << XEND;
      return false;
    }
    if (oTarget.getRelationCount(ESOCIALRELATION_STUDENT) >= rCFG.dwMaxStudent)
    {
      thisServer->sendDebugMsg(oTarget.getZoneID(), oTarget.getCharID(), "测试log:自己学生列表超上限");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,自己学生列表超上限" << rCFG.dwMaxStudent << XEND;
      return false;
    }
    if (oTarget.checkRelation(qwGUID, ESOCIALRELATION_BLACK) == true || oTarget.checkRelation(qwGUID, ESOCIALRELATION_BLACK_FOREVER) == true)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_TAPPLY_SUCCESS);
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,自己在对方黑名单里" << XEND;
      return false;
    }
  }
  else if (eRelation == ESOCIALRELATION_TUTOR && bConfim)
  {
    // check punish time
    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_PUNISH, pUserSocial->getGUID());
    DWORD dwPunish = 0;
    RedisManager::getMe().getData<DWORD>(key, dwPunish);
    if (dwPunish != 0)
    {
      thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), ESYSTEMMSG_ID_TUTOR_PUNISH_TUTOR_S);
      return false;
    }

    // check black
    Sociality* pTarget = pUserSocial->getSociality(qwDestGUID);
    if (pTarget != nullptr && (pTarget->checkRelation(ESOCIALRELATION_BLACK) == true || pTarget->checkRelation(ESOCIALRELATION_BLACK_FOREVER) == true))
    {
      thisServer->sendDebugMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), "测试log:与对方未黑名单关系");
      return false;
    }

    // check tutor enable
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_TUTOR) > 0)
    {
      thisServer->sendDebugMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), "测试log:自己已拥有导师");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方已拥有导师" << XEND;
      return false;
    }
    Sociality* pSocial = pUserSocial->getSociality(qwDestGUID);
    if (pSocial == nullptr || pSocial->checkRelation(ESOCIALRELATION_TUTOR_APPLY) == false)
    {
      thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), 10, MsgParams("测试log:未收到导师申请直接添加"));
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方未收到导师申请直接添加" << XEND;
      return false;
    }
    const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();
    if (pUserSocial->getBaseLevel() >= rCFG.dwTutorBaseLvReq)
    {
      thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), ESYSTEMMSG_ID_TUTOR_STUDENT_LV_UP, MsgParams(rCFG.dwTutorBaseLvReq));
      return false;
    }

    // check student enable
    if (oTarget.getRelationCount(ESOCIALRELATION_STUDENT) >= rCFG.dwMaxStudent)
    {
      thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), ESYSTEMMSG_ID_TUTOR_MAX_STUDENT);
      return false;
    }
  }
  else if (eRelation == ESOCIALRELATION_STUDENT_APPLY && bConfim)
  {
    const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();

    // check tutor enable
    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_PUNISH, pUserSocial->getGUID());
    DWORD dwPunish = 0;
    RedisManager::getMe().getData<DWORD>(key, dwPunish);
    if (dwPunish != 0)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_PUNISH_TUTOR_T);
      return false;
    }
    if (pUserSocial->getBaseLevel() < rCFG.dwTutorBaseLvReq || pUserSocial->getGCharData().getTutor() == false)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_TUTOR_UNENABLE);
      return false;
    }
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_STUDENT) >= rCFG.dwMaxStudent)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_MAX_STUDENT);
      return false;
    }
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_STUDENT_APPLY) >= rCFG.dwMaxStudent)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), 3249);
      return false;
    }
    Sociality* pTarget = pUserSocial->getSociality(qwDestGUID);
    if (pTarget != nullptr &&
        (pTarget->checkRelation(ESOCIALRELATION_BLACK) == true || pTarget->checkRelation(ESOCIALRELATION_BLACK_FOREVER) == true || pTarget->checkRelation(eRelation) == true))
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_TAPPLY_SUCCESS);
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方在自己黑名单里或已存在该关系" << XEND;
      return false;
    }

    // check student enable
    if (oTarget.getBaseLevel() < rCFG.dwStudentBaseLvReq)
    {
      thisServer->sendDebugMsg(oTarget.getZoneID(), oTarget.getCharID(), "测试log:自己未达到学生资格等级");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方未达到学生资格等级" << rCFG.dwStudentBaseLvReq << XEND;
      return false;
    }
    if (oTarget.getBaseLevel() >= rCFG.dwTutorBaseLvReq)
    {
      thisServer->sendDebugMsg(oTarget.getZoneID(), oTarget.getCharID(), "测试log:自己超过导师资格等级");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方未达到学生资格等级" << rCFG.dwStudentBaseLvReq << XEND;
      return false;
    }
    if (oTarget.getRelationCount(ESOCIALRELATION_TUTOR) > 0)
    {
      thisServer->sendDebugMsg(oTarget.getZoneID(), oTarget.getCharID(), "测试log:自己已拥有导师");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方已拥有导师" << XEND;
      return false;
    }
    if (oTarget.getRelationCount(ESOCIALRELATION_TUTOR_APPLY) > rCFG.dwMaxTutorApply)
    {
      thisServer->sendDebugMsg(oTarget.getZoneID(), oTarget.getCharID(), "测试log:自己导师申请列表已达上限");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方导师申请列表达到上限" << XEND;
      return false;
    }
    if (oTarget.checkRelation(qwGUID, ESOCIALRELATION_BLACK) == true || oTarget.checkRelation(qwGUID, ESOCIALRELATION_BLACK_FOREVER) == true)
    {
      thisServer->sendMsg(oTarget.getZoneID(), oTarget.getCharID(), ESYSTEMMSG_ID_TUTOR_TAPPLY_SUCCESS);
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,自己在对方黑名单里" << XEND;
      return false;
    }
  }
  else if (eRelation == ESOCIALRELATION_STUDENT && bConfim)
  {
    // check punish time
    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_PUNISH, pUserSocial->getGUID());
    DWORD dwPunish = 0;
    RedisManager::getMe().getData<DWORD>(key, dwPunish);
    if (dwPunish != 0)
    {
      thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), ESYSTEMMSG_ID_TUTOR_PUNISH_STUDENT_T);
      return false;
    }

    // check black
    Sociality* pTarget = pUserSocial->getSociality(qwDestGUID);
    if (pTarget != nullptr && (pTarget->checkRelation(ESOCIALRELATION_BLACK) == true || pTarget->checkRelation(ESOCIALRELATION_BLACK_FOREVER) == true))
    {
      thisServer->sendDebugMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), "测试log:与对方未黑名单关系");
      return false;
    }

    // check student enable
    const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();
    if (pUserSocial->getSocialityCount(ESOCIALRELATION_STUDENT) >= rCFG.dwMaxStudent)
    {
      thisServer->sendDebugMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), "测试log:自己学生列表已达上限");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,自己学生列表达到上限" << rCFG.dwMaxStudent << XEND;
      return false;
    }
    Sociality* pSocial = pUserSocial->getSociality(qwDestGUID);
    if (pSocial == nullptr || pSocial->checkRelation(ESOCIALRELATION_STUDENT_APPLY) == false)
    {
      thisServer->sendDebugMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), "测试log:未收到学生申请直接添加");
      XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败,对方未收到学生申请直接添加" << XEND;
      return false;
    }

    // check tutor enable
    if (oTarget.getRelationCount(ESOCIALRELATION_TUTOR) > 0)
    {
      thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), ESYSTEMMSG_ID_TUTOR_HAS_TUTOR);
      return false;
    }
    if (oTarget.getBaseLevel() >= rCFG.dwTutorBaseLvReq)
    {
      thisServer->sendMsg(pUserSocial->getZoneID(), pUserSocial->getGUID(), ESYSTEMMSG_ID_TUTOR_STUDENT_LV_UP, MsgParams(rCFG.dwTutorBaseLvReq));
      return false;
    }
  }

  // add relation
  if (pUserSocial->addRelation(qwDestGUID, eRelation) == false)
  {
    XERR << "[社交管理-导师关系]" << qwGUID << "添加" << qwDestGUID << eRelation << "关系失败, 添加关系失败" << XEND;
    return false;
  }

  // do after adding relation
  if (eRelation == ESOCIALRELATION_TUTOR)
  {
    if (bConfim)
    {
      addTutor(qwDestGUID, qwGUID, ESOCIALRELATION_STUDENT, false);
      SyncRedTip(qwDestGUID, qwGUID, EREDSYS_TUTOR_APPLY, false);
    }

    const TMapSocial& mapList = oTarget.getSocial();
    for (auto &m : mapList)
    {
      if (oTarget.checkRelation(m.first, ESOCIALRELATION_STUDENT) == true)
      {
        addTutor(m.first, qwGUID, ESOCIALRELATION_TUTOR_CLASSMATE);
        addTutor(qwGUID, m.first, ESOCIALRELATION_TUTOR_CLASSMATE);
      }
    }
  }
  else if (eRelation == ESOCIALRELATION_STUDENT)
  {
    if (bConfim)
    {
      addTutor(qwDestGUID, qwGUID, ESOCIALRELATION_TUTOR, false);
      SyncRedTip(qwGUID, qwDestGUID, EREDSYS_TUTOR_APPLY, false);
    }
  }
  else if (eRelation == ESOCIALRELATION_TUTOR_APPLY || eRelation == ESOCIALRELATION_STUDENT_APPLY)
  {
    SyncRedTip(qwDestGUID, qwGUID, EREDSYS_TUTOR_APPLY, true);
  }

  return true;
}

void SocialityManager::processLoad()
{
  xTime frameTime;
  TSetQWORD setRemoveIDs;
  for (auto &m : m_mapLoadList)
  {
    onUserOnline(m.second);
    setRemoveIDs.insert(m.first);

    if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwSocialLoadTime)
    {
      DWORD dwLeft = m_mapLoadList.size() > setRemoveIDs.size() ? m_mapLoadList.size() - setRemoveIDs.size() : 0;
      XLOG << "[社交管理] 加载耗时" << frameTime.uElapse() << "已超过" << CommonConfig::m_dwSocialLoadTime << "毫秒, 还剩余" << dwLeft << "个等待加载" << XEND;
      break;
    }
  }
  for (auto &s : setRemoveIDs)
    removeLoad(s);
}

void SocialityManager::processReq()
{
  xTime frameTime;
  TSetQWORD setRemoveIDs;
  for (auto &m : m_mapReqList)
  {
    UserSociality* pUserSocial = getUserSociality(m.first);
    if (pUserSocial != nullptr)
      pUserSocial->querySocialList();
    setRemoveIDs.insert(m.first);

    if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwSocialLoadTime)
    {
      DWORD dwLeft = m_mapLoadList.size() > setRemoveIDs.size() ? m_mapLoadList.size() - setRemoveIDs.size() : 0;
      XLOG << "[社交管理] 加载耗时" << frameTime.uElapse() << "已超过" << CommonConfig::m_dwSocialLoadTime << "毫秒, 还剩余" << dwLeft << "个等待加载" << XEND;
      break;
    }
  }
  for (auto &s : setRemoveIDs)
    removeRequest(s);
}

bool SocialityManager::doUserCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0 || rUser.charid() == 0 || rUser.zoneid() == 0)
    return false;

  switch (cmd->param)
  {
    case SOCIALITYPARAM_QUERYDATA:
      {
        PARSE_CMD_PROTOBUF(QuerySocialData, rev);
        UserSociality* pUserSocial = getUserSociality(rUser.charid());
        if (pUserSocial == nullptr)
        {
          XERR << "[社交管理-消息]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "请求好友数据列表失败,数据不存在" << XEND;
          break;
        }
        addRequest(rUser);
      }
      break;
    case SOCIALITYPARAM_FINDUSER:
      {
        PARSE_CMD_PROTOBUF(FindUser, rev);
        findUser(rUser.charid(), rev.keyword());
      }
      break;
    case SOCIALITYPARAM_FRAMESTATUS:
      {
        PARSE_CMD_PROTOBUF(FrameStatusSocialCmd, rev);
        frameStatus(rUser.charid(), rev.open());
      }
      break;
    case SOCIALITYPARAM_ADDRELATION:
      {
        PARSE_CMD_PROTOBUF(AddRelation, rev);
        for (int i = 0; i < rev.charid_size(); ++i)
          addRelation(rUser.charid(), rev.charid(i), rev.relation());
      }
      break;
    case SOCIALITYPARAM_REMOVERELATION:
      {
        PARSE_CMD_PROTOBUF(RemoveRelation, rev);
        removeRelation(rUser.charid(), rev.charid(), rev.relation());

        if (rev.relation() == ESOCIALRELATION_TUTOR)
          removeRelation(rev.charid(), rUser.charid(), ESOCIALRELATION_STUDENT, false);
        else if (rev.relation() == ESOCIALRELATION_STUDENT)
          removeRelation(rev.charid(), rUser.charid(), ESOCIALRELATION_TUTOR, false);
      }
      break;
    case SOCIALITYPARAM_RECALL_FRIEND:
      {
        PARSE_CMD_PROTOBUF(RecallFriendSocialCmd, rev);
        recallFriend(rUser.zoneid(), rUser.charid(), rev.charid());
      }
      break;
    default:
      return false;
  }

  return true;
}

