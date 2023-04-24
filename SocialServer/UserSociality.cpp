#include "UserSociality.h"
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

UserSociality::UserSociality() : m_oGCharData(thisServer->getRegionID(), 0)
{
  m_dwTickTime = xTime::getCurSec();
}

UserSociality::~UserSociality()
{

}

bool UserSociality::loadRecall()
{
  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "social_recall");
  if (field == nullptr)
  {
    XERR << "[社交-回归加载]" << getGUID() << "加载失败,获取数据库" << REGION_DB << "social_recall 失败" << XEND;
    return false;
  }

  stringstream sstr;
  sstr << "id = " << getGUID() << " or destid = " << getGUID();

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, sstr.str().c_str());
  if (ret == QWORD_MAX)
  {
    XERR << "[社交-回归加载]" << getGUID() << "查询召唤列表失败 ret :" << ret << XEND;
    return false;
  }
  for (DWORD d = 0; d < set.size(); ++d)
  {
    QWORD id = set[d].get<QWORD>("id");
    QWORD destid = set[d].get<QWORD>("destid");

    if (id == getGUID())
      m_setRecallIDs.insert(destid);
    else if (destid == getGUID())
      m_setBeRecallIDs.insert(id);
  }

  XLOG << "[社交-回归加载]" << getGUID() << "加载成功,加载召回列表" << m_setRecallIDs << "被召回列表" << m_setBeRecallIDs << XEND;

  checkRecallList();
  return true;
}

void UserSociality::queryRecallList()
{
  QueryRecallListSocialCmd cmd;
  for (auto &s : m_setBeRecallIDs)
  {
    Recall* pItem = cmd.add_items();
    pItem->set_charid(s);
  }
  if (cmd.items_size() <= 0)
    return;

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(getZoneID(), getGUID(), send, len);
  XDBG << "[社交-回归同步]" << getZoneID() << getGUID() << "同步被召回列表" << cmd.ShortDebugString() << XEND;
}

void UserSociality::querySocialNtfList()
{
  QueryDataNtfSocialCmd cmd;
  for (auto &m : m_mapSocial)
  {
    SocialData* pData = cmd.add_relations();
    pData->set_guid(m.first);
    pData->set_relation(m.second.getRelation());
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(getZoneID(), getGUID(), send, len);
}

bool UserSociality::syncSocialList()
{
  //const TMapSociality& mapSocial = getSocialityList();

  SyncSocialListSocialCmd cmd;
  cmd.set_charid(m_oGCharData.getCharID());
  for (auto &m : m_mapSocial)
  {
    if (m.second.getRelation() == 0 || m.second.getRelation() == ESOCIALRELATION_MIN)
      continue;

    m.second.toData(cmd.add_items());
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  XDBG << "[社交-列表同步] charid :" << m_oGCharData.getCharID() << "同步了" << cmd.items_size() << "个关系" << XEND;
  return true;
}

bool UserSociality::querySocialList()
{
  TSetQWORD setValidIDs;
  QuerySocialData cmd;
  for (auto &m : m_mapSocial)
  {
    xTime frameDebug;
    SocialData oData;
    if (m.second.toData(&oData) == false)
    {
      setValidIDs.insert(m.first);
      m_setSaveIDs.insert(m.first);
      m_setUpdateIDs.insert(m.first);
      XERR << "[社交-列表请求] charid :" << m_oGCharData.getCharID() << "加载社交关系" << m.first << "失败,该社交关系被丢弃" << XEND;
    }
    else
    {
      oData.set_canrecall(canBeRecall(m.second));
      cmd.add_datas()->CopyFrom(oData);
    }
    XDBG << "[社交-列表请求] charid :" << m_oGCharData.getCharID() << "加载" << m.first << "耗时" << frameDebug.uElapse() << XEND;
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(0, m_oGCharData.getCharID(), send, len);

  for (auto &s : setValidIDs)
    m_mapSocial.erase(s);

  update();
  XDBG << "[社交-列表请求] charid :" << m_oGCharData.getCharID() << "请求了列表,获得了" << cmd.datas_size() << "个好友" << setValidIDs.size() << "个失效好友" << XEND;
  return true;
}

bool UserSociality::reqUpdate()
{
  for (auto &m : m_mapSocial)
  {
    SocialDataUpdate cmd;
    cmd.set_guid(m.first);
    m.second.fetchChangeData(cmd);
    if (cmd.items_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToMe(0, m_oGCharData.getCharID(), send, len);

#ifdef _DEBUG
      for (int i = 0; i < cmd.items_size(); ++i)
        XDBG << "[社交管理-更新]" << getGUID() << "好友" << m.first << m.second.getName() << "更新" << cmd.items(i).ShortDebugString() << XEND;
#endif
    }
  }

  XDBG << "[社交管理-更新] 请求更新好友数据" << getGUID() << "请求更新好友数据" << XEND;
  return true;
}

void UserSociality::refreshBlackStatus()
{
  const SSocialityMiscCFG& rCFG = MiscConfig::getMe().getSocialCFG();
  DWORD dwNow = xTime::getCurSec();
  TSetQWORD setRemoveIDs;
  for (auto &m : m_mapSocial)
  {
    if (m.second.checkRelation(ESOCIALRELATION_BLACK) == false)
      continue;
    DWORD dwBlackTime = m.second.getRelationTime(ESOCIALRELATION_BLACK);
    if (dwBlackTime > dwNow)
      break;
    if (m.second.checkRelation(ESOCIALRELATION_BLACK) == true && dwNow - dwBlackTime >= rCFG.dwBlackOverTime)
      setRemoveIDs.insert(m.first);
  }
  for (auto &s : setRemoveIDs)
  {
    if (removeRelation(s, ESOCIALRELATION_BLACK) == true)
      XLOG << "[社交-黑名单刷新]" << getGUID() << "黑名单列表中" << s << "超过" << rCFG.dwBlackOverTime / DAY_T << "天,自动删除" << XEND;
  }
}

void UserSociality::checkRecallList()
{
  const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();

  // check recall
  DWORD dwRecallRelationCount = 0;
  for (auto &m : m_mapSocial)
  {
    if (m.second.checkRelation(ESOCIALRELATION_RECALL) == true)
      ++dwRecallRelationCount;
  }
  if (dwRecallRelationCount > rCFG.dwMaxRecallCount)
  {
    for (auto &s : m_setRecallIDs)
      m_setRecallUpdateIDs.insert(s);
    m_setRecallIDs.clear();
    if (m_setRecallUpdateIDs.empty() == false)
      XLOG << "[玩家-回归检查]" << getGUID() << "召回关系超过上限" << rCFG.dwMaxRecallCount << "个,剩余召回列表" << m_setRecallUpdateIDs << "被清空" << XEND;
  }
  else
  {
    for (auto &s : m_setRecallIDs)
    {
      Sociality* pSociality = getSociality(s);
      if (pSociality == nullptr)
      {
        XERR << "[玩家-回归检查]" << getGUID() << "召回" << s << "不在社交列表内,被移除" << XEND;
        m_setRecallUpdateIDs.insert(s);
        continue;
      }
      GCharReader& rData = pSociality->getGCharData();
      if (rData.getRelationCount(ESOCIALRELATION_BERECALL) >= 1)
      {
        XERR << "[玩家-回归检查]" << getGUID() << "召回" << s << "已与其他玩家签订契约,被移除" << XEND;
        m_setRecallUpdateIDs.insert(s);
        continue;
      }
    }
    for (auto &s : m_setRecallUpdateIDs)
      m_setRecallIDs.erase(s);
  }

  // check berecall
  for (auto &s : m_setBeRecallIDs)
  {
    GCharReader oData(thisServer->getRegionID(), s);
    if (oData.getBySocial() == EERROR_NOT_EXIST)
    {
      XERR << "[玩家-回归检查]" << getGUID() << "自己被" << s << "召回,获取redis数据失败,被移除" << XEND;
      m_setBeRecallUpdateIDs.insert(s);
      continue;
    }
    if (oData.getRelationCount(ESOCIALRELATION_BERECALL) >= 1)
    {
      XERR << "[玩家-回归检查]" << getGUID() << "自己被" << s << "召回,已与其他玩家签订契约,被移除" << XEND;
      m_setBeRecallUpdateIDs.insert(s);
      continue;
    }
    if (oData.getRecallCount() >= rCFG.dwMaxRecallCount)
    {
      XERR << "[玩家-回归检查]" << getGUID() << "自己被" << s << "召回,对方超过契约上限,被移除" << XEND;
      m_setBeRecallUpdateIDs.insert(s);
      continue;
    }
  }
  for (auto &s : m_setBeRecallUpdateIDs)
    m_setBeRecallIDs.erase(s);

  if (m_setRecallUpdateIDs.empty() == false || m_setBeRecallUpdateIDs.empty() == false)
    updateRecall();
}

void UserSociality::updateRecall()
{
  if (m_setRecallUpdateIDs.empty() == true && m_setBeRecallUpdateIDs.empty() == true)
    return;

  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "social_recall");
  if (field == nullptr)
  {
    XERR << "[社交-回归更新]" << getGUID() << "更新失败,获取数据库" << REGION_DB << "social_recall 失败" << XEND;
    return;
  }

  if (m_setRecallUpdateIDs.empty() == false)
  {
    TVecQWORD vecUpdateIDs;
    TVecQWORD vecDeleteIDs;
    for (auto &s : m_setRecallUpdateIDs)
    {
      if (m_setRecallIDs.find(s) != m_setRecallIDs.end())
        vecUpdateIDs.push_back(s);
      else
        vecDeleteIDs.push_back(s);
    }

    if (vecUpdateIDs.empty() == false)
    {
      xRecordSet recordSet;
      for (auto &v : vecUpdateIDs)
      {
        xRecord record(field);
        record.put("id", getGUID());
        record.put("destid", v);
        recordSet.push(record);
      }
      QWORD ret = thisServer->getDBConnPool().exeInsertSet(recordSet);
      if (ret == QWORD_MAX)
        XERR << "[社交-回归更新]" << getGUID() << "更新召回列表失败,ret :" << ret << "列表包含" << vecUpdateIDs << XEND;
      else
        XLOG << "[社交-回归更新]" << getGUID() << "更新召回列表成功,列表包含" << vecUpdateIDs << XEND;
    }
    if (vecDeleteIDs.empty() == false)
    {
      stringstream sstr;
      for (size_t i = 0; i < vecDeleteIDs.size(); ++i)
      {
        if (i != vecDeleteIDs.size() - 1)
          sstr << "(id = " << getGUID() << " and destid = " << vecDeleteIDs[i] << ") or ";
        else
          sstr << "(id = " << getGUID() << " and destid = " << vecDeleteIDs[i] << ")";
      }

      QWORD ret = thisServer->getDBConnPool().exeDelete(field, sstr.str().c_str());
      if (ret == QWORD_MAX)
        XERR << "[社交-回归更新]" << getGUID() << "删除召回列表失败,ret :" << ret << "列表包含" << vecDeleteIDs << XEND;
      else
        XLOG << "[社交-回归更新]" << getGUID() << "删除召回列表成功,列表包含" << vecDeleteIDs << XEND;
    }
  }

  if (m_setBeRecallUpdateIDs.empty() == false)
  {
    TVecQWORD vecUpdateIDs;
    TVecQWORD vecDeleteIDs;
    for (auto &s : m_setBeRecallUpdateIDs)
    {
      if (m_setBeRecallIDs.find(s) != m_setBeRecallIDs.end())
        vecUpdateIDs.push_back(s);
      else
        vecDeleteIDs.push_back(s);
    }

    if (vecUpdateIDs.empty() == false)
    {
      xRecordSet recordSet;
      for (auto &v : vecUpdateIDs)
      {
        xRecord record(field);
        record.put("id", v);
        record.put("destid", getGUID());
        recordSet.push(record);
      }
      QWORD ret = thisServer->getDBConnPool().exeInsertSet(recordSet);
      if (ret == QWORD_MAX)
        XERR << "[社交-回归更新]" << getGUID() << "更新被召回列表失败,ret :" << ret << "列表包含" << vecUpdateIDs << XEND;
      else
        XLOG << "[社交-回归更新]" << getGUID() << "更新被召回列表成功,列表包含" << vecUpdateIDs << XEND;
    }
    if (vecDeleteIDs.empty() == false)
    {
      stringstream sstr;
      for (size_t i = 0; i < vecDeleteIDs.size(); ++i)
      {
        if (i != vecDeleteIDs.size() - 1)
          sstr << "(id = " << vecDeleteIDs[i] << " and destid = " << getGUID() << ") or ";
        else
          sstr << "(id = " << vecDeleteIDs[i] << " and destid = " << getGUID() << ")";
      }

      QWORD ret = thisServer->getDBConnPool().exeDelete(field, sstr.str().c_str());
      if (ret == QWORD_MAX)
        XERR << "[社交-回归更新]" << getGUID() << "删除被召回列表失败,ret :" << ret << "列表包含" << vecDeleteIDs << XEND;
      else
        XLOG << "[社交-回归更新]" << getGUID() << "删除被召回列表成功,列表包含" << vecDeleteIDs << XEND;
    }
  }

  m_setRecallUpdateIDs.clear();
  m_setBeRecallUpdateIDs.clear();
}

bool UserSociality::addRelation(QWORD qwGUID, ESocialRelation eRelation, bool bCheck /*= true*/)
{
  if (eRelation <= ESOCIALRELATION_MIN || eRelation >= ESOCIALRELATION_MAX || ESocialRelation_IsValid(eRelation) == false)
  {
    XERR << "[社交管理(个人)-添加关系]" << m_oGCharData.getCharID() << "添加" << qwGUID << eRelation << "失败,relation :" << eRelation << "不合法" << XEND;
    return false;
  }

  bool bInit = false;
  auto m = m_mapSocial.find(qwGUID);
  if (m == m_mapSocial.end())
  {
    Sociality oSocial;
    oSocial.setGUID(qwGUID);
    if (createSocialData(qwGUID, oSocial) == false)
    {
      thisServer->sendMsg(0, m_oGCharData.getCharID(), ESYSTEMMSG_ID_USER_NOEXIST);
      XERR << "[社交管理(个人)-添加关系]" << m_oGCharData.getCharID() << "添加" << qwGUID << eRelation << "失败,创建社交数据失败" << XEND;
      return false;
    }

    m_mapSocial.insert(std::make_pair(qwGUID, oSocial));
    bInit = true;

    m = m_mapSocial.find(qwGUID);
    if (m == m_mapSocial.end())
    {
      XERR << "[社交管理(个人)-添加关系]" << m_oGCharData.getCharID() << "添加" << qwGUID << eRelation << "失败,添加后列表里未发现" << XEND;
      return false;
    }
  }

  Sociality& rSocial = m->second;
  if (!bInit && createSocialData(qwGUID, rSocial) == false)
  {
    thisServer->sendMsg(0, m_oGCharData.getCharID(), ESYSTEMMSG_ID_USER_NOEXIST);
    XERR << "[社交管理(个人)-添加关系]" << m_oGCharData.getCharID() << "添加" << qwGUID << eRelation << "失败,创建社交数据失败" << XEND;
    return false;
  }
  if (rSocial.checkRelation(eRelation) == true)
  {
    XERR << "[社交管理(个人)-添加关系]" << m_oGCharData.getCharID() << "添加" << qwGUID << eRelation << "失败,已存在该关系" << XEND;
    return false;
  }
  rSocial.addRelation(eRelation);

  if (eRelation == ESOCIALRELATION_FRIEND)
  {
    rSocial.removeRelation(ESOCIALRELATION_APPLY);
    rSocial.removeRelation(ESOCIALRELATION_BLACK);
    rSocial.removeRelation(ESOCIALRELATION_BLACK_FOREVER);
  }
  else if (eRelation == ESOCIALRELATION_APPLY)
  {
    const SSocialityMiscCFG& rCFG = MiscConfig::getMe().getSocialCFG();
    if (getSocialityCount(eRelation) > rCFG.dwMaxApplyCount)
      removeRelation(ESOCIALRELATION_APPLY, ESRMETHOD_TIME_MIN);
  }
  else if (eRelation == ESOCIALRELATION_TEAM)
  {
    const SSocialityMiscCFG& rCFG = MiscConfig::getMe().getSocialCFG();
    if (getSocialityCount(eRelation) > rCFG.dwMaxNearTeamCount)
      removeRelation(ESOCIALRELATION_TEAM, ESRMETHOD_TIME_MIN);
  }
  else if (eRelation == ESOCIALRELATION_BLACK)
  {
    rSocial.removeRelation(ESOCIALRELATION_FRIEND);
    rSocial.removeRelation(ESOCIALRELATION_BLACK_FOREVER);
  }
  else if (eRelation == ESOCIALRELATION_BLACK_FOREVER)
  {
    rSocial.removeRelation(ESOCIALRELATION_FRIEND);
    rSocial.removeRelation(ESOCIALRELATION_BLACK);
  }
  else if (eRelation == ESOCIALRELATION_CHAT)
  {
    const SSocialityMiscCFG& rCFG = MiscConfig::getMe().getSocialCFG();
    if (getSocialityCount(eRelation) > rCFG.dwMaxChatCount)
    removeRelation(ESOCIALRELATION_CHAT, ESRMETHOD_TIME_MIN);
  }
  else if (eRelation == ESOCIALRELATION_TUTOR)
  {
    rSocial.removeRelation(ESOCIALRELATION_TUTOR_APPLY);
  }
  else if (eRelation == ESOCIALRELATION_STUDENT)
  {
    removeRelation(qwGUID, ESOCIALRELATION_STUDENT_APPLY);
  }
  else if (eRelation == ESOCIALRELATION_TUTOR_APPLY || eRelation == ESOCIALRELATION_STUDENT_APPLY)
  {
    if (!m_bTutorMatch)
      thisServer->sendMsg(rSocial.getZoneID(), rSocial.getGUID(), ESYSTEMMSG_ID_TUTOR_TAPPLY_SUCCESS);
  }
  else if (eRelation == ESOCIALRELATION_RECALL)
  {
    if (isBeRecall(qwGUID) == true)
      rSocial.addRelation(ESOCIALRELATION_BERECALL);

    for (auto &s : m_setBeRecallIDs)
      m_setBeRecallUpdateIDs.insert(s);
    m_setBeRecallIDs.clear();
  }

  /*if (rSocial.getRelation() != 0 && rSocial.getRelation() != ESOCIALRELATION_MIN)
    m_mapFocus.erase(qwGUID);*/

  if (bInit)
  {
    m_setUpdateIDs.insert(qwGUID);
    update();
  }
  else
  {
    rSocial.setMark(ESOCIALDATA_CREATETIME);
    if (m_bFrameOpen)
      reqUpdate();
  }

  m_setSaveIDs.insert(qwGUID);
  updateRedisAndSync();
  XLOG << "[社交管理(个人)-添加关系]" << m_oGCharData.getCharID() << "添加" << qwGUID << eRelation << "成功" << XEND;
  return true;
}

bool UserSociality::removeRelation(QWORD qwGUID, ESocialRelation eRelation, bool bCheck /*= false*/)
{
  /*
  if(getAuthorize() == false)
  {
    XERR << "[社交管理(个人)-移除关系]" << m_oGCharData.getCharID() << "移除" << qwGUID << eRelation << "失败, 密码验证失败" << XEND;
    return false;
  }
  */

  if (eRelation <= ESOCIALRELATION_MIN || eRelation >= ESOCIALRELATION_MAX || ESocialRelation_IsValid(eRelation) == false)
  {
    XERR << "[社交管理(个人)-移除关系]" << m_oGCharData.getCharID() << "移除" << qwGUID << eRelation << "失败,relation :" << eRelation << "不合法" << XEND;
    return false;
  }

  auto m = m_mapSocial.find(qwGUID);
  if (m == m_mapSocial.end())
  {
    XERR << "[社交管理(个人)-移除关系]" << m_oGCharData.getCharID() << "移除" << qwGUID << eRelation << "失败,未发现该社交数据" << XEND;
    return false;
  }

  Sociality& rSocial = m->second;
  if (rSocial.checkRelation(eRelation) == false)
  {
    XERR << "[社交管理(个人)-移除关系]" << m_oGCharData.getCharID() << "移除" << qwGUID << eRelation << "失败,未存在该关系" << XEND;
    return false;
  }
  rSocial.removeRelation(eRelation);

  if (bCheck && (eRelation == ESOCIALRELATION_TUTOR || eRelation == ESOCIALRELATION_STUDENT))
  {
    const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();
    DWORD dwOfflineTime = rSocial.getOfflineTime();
    DWORD dwNow = xTime::getCurSec();
    if (dwOfflineTime == 0 || (dwNow >= dwOfflineTime && dwNow - dwOfflineTime <= rCFG.dwProtectTime))
    {
      const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_PUNISH, getGUID());
      RedisManager::getMe().setData<DWORD>(key, 1, MiscConfig::getMe().getTutorCFG().dwForbidTime);
      XLOG << "[社交管理(个人)-移除关系]" << m_oGCharData.getCharID() << "移除" << qwGUID << eRelation << "成功,进入惩罚期" << XEND;
    }
    thisServer->sendMsg(getZoneID(), getGUID(), ESYSTEMMSG_ID_TUTOR_RELATION_OVER, MsgParams(rSocial.getName()));

    if (eRelation == ESOCIALRELATION_TUTOR)
    {
      const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_REWARD_PUNISH, qwGUID);
      DWORD dwLeft = DAY_T - (dwNow - xTime::getDayStart(dwNow));
      RedisManager::getMe().setData<DWORD>(key, dwNow, dwLeft);
      XLOG << "[社交管理(个人)-移除关系]" << m_oGCharData.getCharID() << "移除" << qwGUID << eRelation << "成功" << qwGUID << "导师进入奖励惩罚,剩余时间" << dwLeft << "秒" << XEND;
    }
  }

  if (eRelation == ESOCIALRELATION_TUTOR_APPLY || eRelation == ESOCIALRELATION_STUDENT_APPLY)
  {
    DWORD dwCount = 0;
    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_REFUSE_PROTECT, qwGUID, getGUID());
    RedisManager::getMe().getData<DWORD>(key, dwCount);
    if (dwCount < 3)
      RedisManager::getMe().setData<DWORD>(key, ++dwCount, MiscConfig::getMe().getTutorCFG().dwApplyRefuseProtect);
  }

  DWORD dwRelation = rSocial.getRelation();
  if (dwRelation == 0 || dwRelation == ESOCIALRELATION_MIN)
  {
    /*bool bFocusAdd = false;

    UserSociality* pTargetSocial = SocialityManager::getMe().getUserSociality(qwGUID);
    if (pTargetSocial != nullptr)
    {
      Sociality* pSociality = pTargetSocial->getSociality(m_oGCharData.getCharID(), false);
      if (pSociality != nullptr && pSociality->getRelation() != 0 && pSociality->getRelation() != ESOCIALRELATION_MIN)
        bFocusAdd = true;
      else
        pTargetSocial->removeFocus(m_oGCharData.getCharID());
    }
    else
    {
      GCharReader pGChar(thisServer->getRegionID(), qwGUID);
      if (pGChar.get())
      {
        const TMapSocial& mapSocial = pGChar.getSocial();
        auto item = mapSocial.find(m_oGCharData.getCharID());
        if (item != mapSocial.end() && item->second != 0 && item->second != ESOCIALRELATION_MIN)
          bFocusAdd = true;
        else
        {
          RemoveFocusSocialCmd cmd;
          cmd.mutable_user()->set_zoneid(thisServer->getZoneID());
          cmd.mutable_user()->set_charid(qwGUID);
          cmd.set_destid(m_oGCharData.getCharID());
          cmd.set_to_global(true);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
    }

    if (bFocusAdd)
      addFocus(qwGUID);
    else
      removeFocus(qwGUID);*/

    m_mapSocial.erase(m);
    m_setUpdateIDs.insert(qwGUID);
    update();
  }
  else
  {
    if (m_bFrameOpen)
      reqUpdate();
  }

  m_setSaveIDs.insert(qwGUID);
  updateRedisAndSync();
  XLOG << "[社交管理(个人)-移除关系]" << m_oGCharData.getCharID() << "移除" << qwGUID << eRelation << "成功" << XEND;
  return true;
}

bool UserSociality::removeRelation(ESocialRelation eRelation, ESRMethod eMethod)
{
  if (eMethod == ESRMETHOD_TIME_MIN)
  {
    QWORD qwCharID = 0;
    DWORD dwMin = xTime::getCurSec();
    for (auto &m : m_mapSocial)
    {
      if (m.second.checkRelation(eRelation) == false)
        continue;
      DWORD dwTime = m.second.getRelationTime(eRelation);
      if (dwTime > dwMin)
        continue;
      dwMin = m.second.getRelationTime(eRelation);
      qwCharID = m.first;
    }

    if (qwCharID != 0)
      removeRelation(qwCharID, eRelation);
  }
  else if (eMethod == ESRMETHOD_ALL)
  {
    TSetQWORD setGUID;
    for (auto &m : m_mapSocial)
      setGUID.insert(m.first);
    for (auto s = setGUID.begin(); s != setGUID.end(); ++s)
      removeRelation(*s, eRelation);
  }
  else if (eMethod == ESRMETHOD_CONTRACT_OVERTIME)
  {
    DWORD dwNow = xTime::getCurSec();
    const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
    TSetQWORD setGUID;
    for (auto &m : m_mapSocial)
    {
      if (m.second.checkRelation(eRelation) == false)
        continue;
      if (m.second.getRelationTime(eRelation) + rCFG.dwContractTime > dwNow)
        continue;
      setGUID.insert(m.first);
    }
    for (auto s = setGUID.begin(); s != setGUID.end(); ++s)
      removeRelation(*s, eRelation);
  }

  return true;
}

/*bool UserSociality::addFocus(QWORD qwGUID)
{
  auto m = m_mapFocus.find(qwGUID);
  if (m != m_mapFocus.end())
    return false;
  Sociality& rSociality = m_mapFocus[qwGUID];
  rSociality.setGUID(qwGUID);
  m_setSaveIDs.insert(qwGUID);
  XLOG << "[社交管理(个人)-关注]" << m_qwGUID << "添加" << qwGUID << "为关注关系" << XEND;
  return true;
}

bool UserSociality::removeFocus(QWORD qwGUID)
{
  auto m = m_mapFocus.find(qwGUID);
  if (m == m_mapFocus.end())
    return false;
  m_mapFocus.erase(m);
  m_setSaveIDs.insert(qwGUID);
  XLOG << "[社交管理(个人)-关注]" << m_qwGUID << "与" << qwGUID << "解除关注关系" << XEND;
  return true;
}*/

Sociality* UserSociality::getSociality(QWORD qwGUID)//, bool bFoucsInclude /*= true*/)
{
  auto m = m_mapSocial.find(qwGUID);
  if (m != m_mapSocial.end())
    return &m->second;

  /*if (bFoucsInclude)
  {
    m = m_mapFocus.find(qwGUID);
    if (m != m_mapFocus.end())
      return &m->second;
  }*/

  return nullptr;
}

DWORD UserSociality::getSocialityCount(ESocialRelation eRelation) const
{
  DWORD dwCount = 0;
  for (auto &m : m_mapSocial)
  {
    if (m.second.checkRelation(eRelation) == true)
      ++dwCount;
  }

  return dwCount;
}

void UserSociality::collectID(TSetQWORD& setID, ESocialRelation eRelation /*= ESOCIALRELATION_MIN*/)
{
  setID.clear();
  for (auto m = m_mapSocial.begin(); m != m_mapSocial.end(); ++m)
  {
    if (m->second.getRelation() == ESOCIALRELATION_CHAT)
      continue;
    if (eRelation != ESOCIALRELATION_MIN && m->second.checkRelation(eRelation) == false)
      continue;
    if (m->second.getOfflineTime() != 0)
      continue;
    setID.insert(m->first);
  }
}

void UserSociality::clearUpdate()
{
  setFrame(false);
  for (auto &m : m_mapSocial)
    m.second.m_bitmark.reset();
}

void UserSociality::update()
{
  if (m_setUpdateIDs.empty() == true)
    return;

  SocialUpdate cmd;
  for (auto &s : m_setUpdateIDs)
  {
    Sociality* pSociality = getSociality(s);//, false);
    if (pSociality != nullptr)
    {
      SocialData* pData = cmd.add_updates();
      pSociality->toData(pData);
      pData->set_canrecall(canBeRecall(*pSociality));
    }
    else
    {
      cmd.add_dels(s);
    }
  }
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(0, m_oGCharData.getCharID(), send, len);

  m_setUpdateIDs.clear();
}

void UserSociality::save(DWORD curTime, bool bRecallOpen)
{
  if (bRecallOpen)
    updateRecall();

  if (curTime < m_dwTickTime || m_setSaveIDs.empty() == true)
    return;
  m_dwTickTime = curTime + SOCIAL_SAVE_TICK;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "social");
  if (pField == nullptr)
  {
    XERR << "[社交管理(个人)-保存]" << m_oGCharData.getCharID() << "保存失败, 获取 social 数据库表失败" << XEND;
    return;
  }

  for (auto &s : m_setSaveIDs)
  {
    if (m_oGCharData.getCharID() == s)
      continue;

    Sociality* pSociality = getSociality(s);
    if (pSociality == nullptr)
    {
      stringstream sstr;
      sstr << "id = " << m_oGCharData.getCharID() << " and destid = " << s;
      QWORD ret = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
      if (ret == QWORD_MAX)
        XERR << "[社交管理(个人)-保存]" << m_oGCharData.getCharID() << "删除社交" << s << "失败 ret :" << ret << XEND;
    }
    else
    {
      xRecord record(pField);
      if (pSociality->toData(record, m_oGCharData.getCharID()) == false)
        XERR << "[社交管理(个人)-保存]" << m_oGCharData.getCharID() << "保存社交" << s << "失败, 序列化失败" << XEND;
      else
      {
        QWORD ret = thisServer->getDBConnPool().exeInsert(record, true, true);
        if (ret == QWORD_MAX)
          XERR << "[社交管理(个人)-保存]" << m_oGCharData.getCharID() << "保存社交" << s << "失败, 插入数据库失败, ret :" << ret << XEND;
      }
    }
  }

  m_setSaveIDs.clear();
}

void UserSociality::updateRedisAndSync()
{
  if (m_setSaveIDs.empty() == true)
    return;

  SocialListUpdateSocialCmd cmd;
  cmd.set_charid(m_oGCharData.getCharID());
  for (auto &s : m_setSaveIDs)
  {
    Sociality* pSociality = getSociality(s);//, false);
    if (pSociality == nullptr)
      cmd.add_dels(s);
    else
      pSociality->toData(cmd.add_updates());
  }

  if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);

    GCharWriter pGCharWriter(thisServer->getRegionID(), m_oGCharData.getCharID());
    m_oGCharData.clearSocial();
    for (auto &it : m_mapSocial)
    {
      pGCharWriter.updateRelation(it.first, it.second.getRelation());
      m_oGCharData.updateRelation(it.first, it.second.getRelation());
    }

    pGCharWriter.setSocial();
    pGCharWriter.save();
#ifdef _DEBUG
    pGCharWriter.debug_log();
#endif
  }
}

bool UserSociality::canBeRecall(Sociality& rSocial)
{
  if (rSocial.canBeRecall() == false)
    return false;
  if (rSocial.getGCharData().checkRelation(getGUID(), ESOCIALRELATION_FRIEND) == false)
    return false;
  if (isRecall(rSocial.getGUID()) == true)
    return false;
  return true;
}

bool UserSociality::addSociality(const Sociality& rSocial)
{
  if (rSocial.getGUID() == 0)
  {
    XERR << "[社交-添加]" << m_oGCharData.getCharID() << "添加" << rSocial.getGUID() << "失败" << XEND;
    return false;
  }
  if (rSocial.getRelation() == 0 || rSocial.getRelation() == ESOCIALRELATION_MIN)
  {
    XERR << "[社交-添加]" << m_oGCharData.getCharID() << "添加" << rSocial.getGUID() << "失败,关注列表已取消" << XEND;
    return false;
  }

  TMapSociality* pMapSocial = /*rSocial.getRelation() == 0 ? &m_mapFocus :*/ &m_mapSocial;
  if (pMapSocial == nullptr)
  {
    XERR << "[社交-添加]" << m_oGCharData.getCharID() << "添加" << rSocial.getGUID() << "失败,没发现列表" << XEND;
    return false;
  }

  auto m = pMapSocial->find(rSocial.getGUID());
  if (m != pMapSocial->end())
  {
    XERR << "[社交-添加]" << m_oGCharData.getCharID() << "添加关注列表" << rSocial.getGUID() << "已存在" << XEND;
    return false;
  }
  (*pMapSocial)[rSocial.getGUID()] = rSocial;
  return true;
}

bool UserSociality::delSociality(QWORD qwCharID)
{
  auto m = m_mapSocial.find(qwCharID);
  if (m != m_mapSocial.end())
  {
    m_setUpdateIDs.insert(m->first);
    m_mapSocial.erase(m);
  }

  /*m = m_mapFocus.find(qwCharID);
  if (m != m_mapFocus.end())
    m_mapFocus.erase(m);*/

  return true;
}

bool UserSociality::createSocialData(QWORD qwGUID, Sociality& rData)
{
  GCharReader &pGChar = rData.m_oGCharData;
  if (pGChar.getBySocial() != EERROR_SUCCESS)
  {
    XERR << "[UserSociality::createSocialData] charid :" << qwGUID << XEND;
    return false;
  }

  pGChar.setOfflineTime(pGChar.getOnlineTime() > pGChar.getOfflineTime() ? 0 : pGChar.getOfflineTime());
  return true;
}

