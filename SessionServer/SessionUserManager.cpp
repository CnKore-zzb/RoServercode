#include "SessionUserManager.h"
#include "SessionUser.h"
//#include "Define.h"
#include "RegCmd.h"
#include "SessionScene.h"
#include "ServerTask.h"
#include "SessionServer.h"
#include "RecordCmd.pb.h"
#include "PlatLogManager.h"
#include "RedisManager.h"
#include "UserActive.h"
#include "CommonConfig.h"
#include "GlobalManager.h"

/*bool SessionUserCallBack::exec(xEntry *e)
{
  if (0 == len || nullptr == data)
    return false;
  SessionUser* pUser = dynamic_cast<SessionUser*>(e);
  if (nullptr == pUser)
    return false;

  if (0 == pUser->getUserWeather() && dwMapID == pUser->getMapID() && dwMapID > 0)
  {
    pUser->sendCmdToMe(data, len);
  }

  return true;
}*/


SessionUserManager::SessionUserManager() : m_oOneMinTimer(60)
{
}

SessionUserManager::~SessionUserManager()
{
  struct CallBack : public xEntryCallBack
  {
    virtual bool exec(xEntry *e)
    {
      SessionUserManager::getMe().delUser((SessionUser*)e);
      return true;
    }
  };
  CallBack callback;
  forEach(callback);
}

bool SessionUserManager::addUser(SessionUser* user)
{
  bool blRet = addEntry(user);
  if (blRet)
  {
    UserActive::getMe().update();
    ONLINEUSER_ALL_MAP_T::iterator it = m_oOnlineUserList.find(user->m_platformId);
    if (it == m_oOnlineUserList.end())
    {
      ONLINEUSER_ZONE_MAP_T zoneMap;
      std::set<QWORD> tmpSet = { user->getTempID() };
      zoneMap.insert(std::make_pair(user->getZoneID(), tmpSet));
      m_oOnlineUserList.insert(std::make_pair(user->m_platformId, zoneMap));
    }
    else
    {
      ONLINEUSER_ZONE_MAP_T& zoneMap = it->second;
      ONLINEUSER_ZONE_MAP_T::iterator subIt = zoneMap.find(user->getZoneID());
      if (subIt == zoneMap.end())
      {
        std::set<QWORD> tmpSet = { user->getTempID() };
        zoneMap.insert(std::make_pair(user->getZoneID(), tmpSet));
      }
      else
      {
        subIt->second.insert(user->getTempID());
      }
    }
  }
  return blRet;
}

void SessionUserManager::delUser(SessionUser* user)
{
  {
    ONLINEUSER_ALL_MAP_T::iterator it = m_oOnlineUserList.find(user->m_platformId);
    if (it == m_oOnlineUserList.end())
    {
      XERR << "" << XEND;
    }
    else
    {
      ONLINEUSER_ZONE_MAP_T& zoneMap = it->second;
      ONLINEUSER_ZONE_MAP_T::iterator subIt = zoneMap.find(user->getZoneID());
      if (subIt == zoneMap.end())
      {
        XERR << "" << XEND;
      }
      else
      {
        subIt->second.erase(user->getTempID());
      }
    }
  }
  removeEntry(user);
  SAFE_DELETE(user);

  UserActive::getMe().update();
}

SessionUser* SessionUserManager::getUserByAccID(ACCID accid)
{
  return (SessionUser *)getEntryByTempID(accid);
}

SessionUser* SessionUserManager::getUserByID(QWORD uid)
{
  return (SessionUser*)getEntryByID(uid);
}

void SessionUserManager::onUserQuit(SessionUser* user)
{
  if (!user) return;
  user->userOffline();
  XLOG << "[注销]" << user->accid << user->id << user->name << "从管理器删除" << XEND;
  delUser(user);
}

void SessionUserManager::loginOutGate(QWORD accid, ServerTask *gt)
{
  LoginOutGateRegCmd notify;
  notify.accid = accid;
  if (gt)
  {
    gt->sendCmd(&notify, sizeof(notify));
    XLOG << "[注销]" << accid << "通知网关退出" << gt->getName() << XEND;
  }
  else
  {
    thisServer->sendCmdToServer(&notify, sizeof(notify), "GateServer");
    XLOG << "[注销]" << accid << "广播网关退出" << XEND;
  }

}

void SessionUserManager::removeUserOnScene(SessionScene *pScene)
{
  if (!pScene) return;

  auto it = xEntryID::ets_.begin(),end=xEntryID::ets_.end();
  auto temp = it;
  for ( ; it!=end; )
  {
    temp = it++;
    SessionUser *user = dynamic_cast<SessionUser *>(temp->second);
    if (user && user->getScene() && pScene==user->getScene())
    {
      SessionUserManager::getMe().onUserQuit(user);
    }
  }
}

void SessionUserManager::timer(DWORD curTime)
{
  for (auto it = xEntryID::ets_.begin(); it != xEntryID::ets_.end(); ++it)
  {
    SessionUser* pUser = dynamic_cast<SessionUser*>(it->second);
    if (pUser != nullptr)
      pUser->timer(curTime);
  }

  if (m_oOneMinTimer.timeUp(curTime))
  {
    onOneMinTimeUp(curTime);
  }
  processSysMail(curTime);
}

void SessionUserManager::onOneMinTimeUp(QWORD curSec)
{
  DWORD t = now();
  DWORD min = xTime::getMin(t);
  if (min % SEND_USERCOUNT_INTERVAL == 0)
  {
    logOnlineUser();
  }
}

void SessionUserManager::addSysMailCache(QWORD qwMailID, QWORD qwCharID)
{
  m_mapSysMailOnlineCache[qwMailID].insert(qwCharID);
  m_dwSysOnlineCacheTick = xTime::getCurSec() + randBetween(CommonConfig::m_dwMailSendMinTick, CommonConfig::m_dwMailSendMaxTick);
  XDBG << "[玩家管理-添加] mail :" << qwMailID << "charid :" << qwCharID << XEND;
}

void SessionUserManager::removeCharIDFromSysCache(QWORD qwCharID)
{
  for (auto &m : m_mapSysMailOnlineCache)
  {
    auto s = m.second.find(qwCharID);
    if (s != m.second.end())
      m.second.erase(s);
  }
}

/*打印在线玩家的数目*/
void SessionUserManager::logOnlineUser()
{
  time_t minTime = xTime::getMinuteStart(now());//务必是分钟的整数时间

  if (minTime == m_dwLastLogTime)
    return;
  DWORD sendTime = minTime;
  while (sendTime > m_dwLastLogTime)
  {
    for (auto it = m_oOnlineUserList.begin(); it != m_oOnlineUserList.end(); ++it)
    {
      auto subMap = it->second;
      for (auto subIt = subMap.begin(); subIt != subMap.end(); ++subIt)
      {
        DWORD count = subIt->second.size();
        PlatLogManager::getMe().onlineCountLog(thisServer, it->first, subIt->first, sendTime, count, 0, 0, count);
        XLOG << "[在线人数打印]，平台id:" << it->first << ", 区id:" << subIt->first << ", 在线人数:" << count << sendTime <<minTime <<m_dwLastLogTime << XEND;
      }
    }
    if (m_dwLastLogTime == 0)
      break;
    if (sendTime > SEND_USERCOUNT_INTERVAL * MIN_T)
      sendTime -= (SEND_USERCOUNT_INTERVAL * MIN_T);
    else
      break;
  }
  m_dwLastLogTime = minTime;
}

void SessionUserManager::processSysMail(DWORD curTime)
{
  if (m_mapSysMailOnlineCache.empty() == true || curTime < m_dwSysOnlineCacheTick)
    return;

  m_dwSysOnlineCacheTick = curTime + randBetween(CommonConfig::m_dwMailSendMinTick, CommonConfig::m_dwMailSendMaxTick);

  for (auto m = m_mapSysMailOnlineCache.begin(); m != m_mapSysMailOnlineCache.end();)
  {
    const MailData* pData = MailManager::getMe().getSysMail(m->first);
    if (pData == nullptr || m->second.empty() == true)
    {
      XDBG << "[玩家管理-邮件发送] 邮件" << m->first << "发送完毕" << XEND;
      m = m_mapSysMailOnlineCache.erase(m);
      continue;
    }

    for (DWORD d = 0; d < CommonConfig::m_dwMailSendMaxCount; ++d)
    {
      if (m->second.empty() == true)
        break;

      SessionUser* pUser = getUserByID(*m->second.begin());
      if (pUser == nullptr)
      {
        XERR << "[玩家管理-邮件发送] 进行了邮件" << m->first << "发送给" << *m->second.begin() << "失败,未找到玩家" << XEND;
        continue;
      }
      pUser->getMail().checkSystemMail();
      m->second.erase(m->second.begin());
    }

    XDBG << "[玩家管理-邮件发送] 进行了邮件" << m->first << "发送, 还剩余" << m->second.size() << "待发送,下一次是" << m_dwSysOnlineCacheTick << "后发送" << XEND;
    break;
  }
}

void SessionUserManager::onSceneClose(SessionScene *scene)
{
  return;
  if (!scene) return;

  for (auto it : xEntryID::ets_)
  {
    SessionUser* pUser = dynamic_cast<SessionUser *>(it.second);
    if (pUser != nullptr)
    {
      if (pUser->getScene() == scene)
      {
        pUser->setScene(nullptr);
      }
    }
  }
}

void SessionUserManager::registRegion(ClientType type)
{
  if (type != ClientType::guild_server && type != ClientType::global_server && type != ClientType::match_server && type != ClientType::wedding_server) return;

  for (auto it : xEntryID::ets_)
  {
    SessionUser* pUser = dynamic_cast<SessionUser *>(it.second);
    if (pUser != nullptr)
    {
      Cmd::OnlineStatusSocialCmd message;
      pUser->toData(message.mutable_user());
      message.set_online(1);
      PROTOBUF(message, send, len);
      thisServer->sendCmd(type, send, len);
    }
  }
}

void SessionUserManager::onRegistServer(const string& svrname)
{
  static const TSetString setValidSever = TSetString{"TeamServer", "SocialServer"};
  if (setValidSever.find(svrname) == setValidSever.end())
    return;

  for (auto it : xEntryID::ets_)
  {
    SessionUser* pUser = dynamic_cast<SessionUser *>(it.second);
    if (pUser != nullptr)
    {
      Cmd::OnlineStatusSocialCmd message;
      pUser->toData(message.mutable_user());
      message.set_online(1);
      PROTOBUF(message, send, len);
      thisServer->sendCmdToServer(send, len, svrname.c_str());
    }
  }
}

void SessionUserManager::addActivityProgress(DWORD activityid, DWORD progress, DWORD starttime, DWORD endtime)
{
  auto it = m_mapActivityProgress.find(activityid);
  if(it != m_mapActivityProgress.end())
  {
    if(it->second.progress() == progress)
      return;
    else
    {
      it->second.set_progress(progress);
      it->second.add_starttime(starttime);
      it->second.add_endtime(endtime);
    }
  }
  else
  {
    ActivityProgress message;
    message.set_actid(activityid);
    message.set_progress(progress);
    message.add_starttime(starttime);
    message.add_endtime(endtime);
    m_mapActivityProgress.insert(std::make_pair(activityid, message));
  }

  for (auto it : xEntryID::ets_)
  {
    SessionUser* pUser = dynamic_cast<SessionUser *>(it.second);
    if (pUser != nullptr)
    {
      if(progress != 0)
      {
        Cmd::ActProgressNtfCmd message;
        message.set_id(activityid);
        message.set_progress(static_cast<EActProgress>(progress));
        message.set_starttime(starttime);
        message.set_endtime(endtime);
        PROTOBUF(message, send, len);
        pUser->sendCmdToMe(send, len);
      }
      else
      {
        StartActCmd cmd;
        cmd.set_id(activityid);
        cmd.set_starttime(starttime);
        cmd.set_endtime(endtime);
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
      }
    }
  }
  XLOG << "[活动进度] : 通知" << "activityid: " << activityid << "progress:" << progress << XEND;
}

void SessionUserManager::delActivityProgress(DWORD activityid, DWORD progress)
{
  auto it = m_mapActivityProgress.find(activityid);
  if(it != m_mapActivityProgress.end() && progress == 0)
  {
    m_mapActivityProgress.erase(it);
    StopActCmd cmd;
    cmd.set_id(activityid);
    PROTOBUF(cmd, send, len);
    MsgManager::sendWorldCmd(send, len);
    XLOG << "[活动进度] : 删除" << "activityid: " << activityid << "progress:" << progress << XEND;
  }
}

void SessionUserManager::sendActivityProgress(SessionUser* pUser)
{
  if(pUser == nullptr)
    return;

  for(auto s : m_mapActivityProgress)
  {
    StartActCmd cmd;
    cmd.set_id(s.second.actid());
    cmd.set_starttime(s.second.starttime(0));
    cmd.set_endtime(s.second.endtime(0));
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
    if(s.second.progress() != 0)
    {
      Cmd::ActProgressNtfCmd message;
      message.set_id(s.second.actid());
      message.set_progress(static_cast<EActProgress>(s.second.progress()));
      DWORD size = s.second.starttime_size();
      message.set_starttime(s.second.starttime(size-1));
      message.set_endtime(s.second.endtime(size-1));
      PROTOBUF(message, send, len);
      pUser->sendCmdToMe(send, len);
    }
  }
}

void SessionUserManager::updateGlobalBoss(const DeadBossInfo& rInfo)
{
  const DeadBossInfo& rBossInfo = GlobalManager::getMe().getGlobalBoss();
  if (rBossInfo.charid() != 0)
  {
    XDBG << "[世界boss-变更] 尝试开启世界boss功能被忽略,已有信息" << rBossInfo.ShortDebugString() << XEND;
    return;
  }

  DeadBossOpenBossSCmd scmd;
  scmd.mutable_info()->CopyFrom(rInfo);
  PROTOBUF(scmd, ssend, slen);
  bool bSuccess = thisServer->sendCmd(ClientType::global_server, ssend, slen);
  XLOG << "[世界boss-变更] 尝试开启世界boss功能,信息" << rBossInfo.ShortDebugString() << "发送至GlobalServer" << (bSuccess ? "成功" : "失败") << XEND;
}

void SessionUserManager::syncGlobalBossScene(ServerTask *task /*= nullptr*/)
{
  const DeadBossInfo& rBossInfo = GlobalManager::getMe().getGlobalBoss();
  if (rBossInfo.charid() == 0)
    return;

  DeadBossOpenSyncBossSCmd scmd;
  scmd.mutable_info()->CopyFrom(rBossInfo);
  PROTOBUF(scmd, ssend, slen);
  bool bSuccess = task != nullptr ? task->sendCmd(ssend, slen) : thisServer->sendCmdToAllScene(ssend, slen);
  XLOG << "[世界boss-同步] 同步数据" << scmd.ShortDebugString() << "到" << (task == nullptr ? "全部场景服" : "单个场景服") << (bSuccess ? "成功" : "失败") << XEND;
}

