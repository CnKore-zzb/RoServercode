#include "GuildManager.h"
#include "GuildServer.h"
#include "MapConfig.h"
#include "LuaManager.h"
#include "PlatLogManager.h"
#include "GuidManager.h"
#include "StatisticsDefine.h"
#include "UserCmd.h"
#include "MiscConfig.h"
#include "CommonConfig.h"
#include "GuildSCmd.pb.h"
#include "TeamCmd.pb.h"
#include "Dojo.pb.h"
#include "GGuild.h"
#include "GuildIconManager.h"
#include "GMCommandMgr.h"

// guild manager
GuildManager::GuildManager()// : m_oDayTimer(0, 5)
{

}

GuildManager::~GuildManager()
{
}

bool GuildManager::addGuild(Guild *p)
{
  if (!p) return false;

  bool bSuccess = addEntry(p);
  if (bSuccess)
    m_mapName2Guild[p->getName()] = p;
  return bSuccess;
}

void GuildManager::delGuild(Guild *p)
{
  if (!p) return;

  m_mapName2Guild.erase(p->getName());
  removeEntry(p);
  SAFE_DELETE(p);
}

bool GuildManager::init()
{
  bool bResult = true;
  if (loadAllGuildData() == false)
    bResult = false;
  if (loadOfflineData() == false)
    bResult = false;

  return bResult;
}

void GuildManager::final()
{
  processOfflineGM();

  DWORD dwNow = xTime::getCurSec();
  for (auto &m : xEntryID::ets_)
  {
    Guild *p = (Guild *)m.second;
    p->m_recordbitset.set();
    p->updateRecord(dwNow + GUILD_RECORD_TICK_TIME * 2);

    SAFE_DELETE(p);
  }
  updateOffline(dwNow + GUILD_RECORD_TICK_TIME * 2, true);
  XLOG << "[公会管理-final]" << XEND;
}

bool GuildManager::loadAllGuildData()
{
  // load guild data
  xTime guildTimer;
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild");
  if (pField == nullptr)
  {
    XERR << "[公会管理-加载] 无法获取数据库guild" << XEND;
    return false;
  }

  xRecordSet set;
  char szWhere[64] = {0};
  snprintf(szWhere, 64, "status = 0");
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, szWhere);
  if (QWORD_MAX == retNum)
  {
    XERR << "[公会管理-加载] 查询guild表失败" << retNum << XEND;
    return false;
  }

  DWORD dwNow = xTime::getCurSec();
  for (QWORD q = 0; q < retNum; ++q)
  {
    QWORD guid = set[q].get<QWORD>("id");
    const string& name = set[q].getString("name");

    Guild* pGuild = NEW Guild(guid, set[q].get<DWORD>("zoneid"), name);
    if (pGuild == nullptr)
    {
      XERR << "[公会管理-加载] 公会" << guid << name << "创建失败" << XEND;
      continue;
    }
    if (!addGuild(pGuild))
    {
      XERR << "[公会管理-加载]" << guid << name << "添加到管理器失败" << XEND;
      SAFE_DELETE(pGuild);
      continue;
    }

    // guild info
    pGuild->m_dwLevel = set[q].get<DWORD>("lv");
    pGuild->m_dwQuestTime = set[q].get<DWORD>("questtime");
    pGuild->m_dwAsset = set[q].get<DWORD>("conlv");
    pGuild->m_dwDismissTime = set[q].get<DWORD>("dismisstime");
    pGuild->m_dwCreateTime = set[q].get<DWORD>("createtime");
    pGuild->m_dwZoneTime = set[q].get<DWORD>("zonetime");
    pGuild->m_dwNextZone = set[q].get<DWORD>("nextzone");

    pGuild->m_strName = set[q].getString("name");
    pGuild->m_strBoard = set[q].getString("board");
    pGuild->m_strRecruit = set[q].getString("recruit");
    pGuild->m_strPortrait = set[q].getString("portrait");

    pGuild->m_pGuildCFG = GuildConfig::getMe().getGuildCFG(pGuild->m_dwLevel);

    // name
    m_setGuildName.insert(pGuild->getName());

    // member
    string member;
    member.assign((const char *)set[q].getBin("member"), set[q].getBinSize("member"));
    BlobGuildMember oBlob;
    XLOG << "[公会管理-加载] 公会" << guid << name << "公会成员数据" << member.size() << XEND;
    if (oBlob.ParseFromString(member) == false)
    {
      XERR << "[公会管理-加载] 公会" << guid << name << "公会成员序列化失败" << XEND;
      continue;
    }
    SocialUser oUser;
    for (int i = 0; i < oBlob.members_size(); ++i)
    {
      const GuildMember& rMember = oBlob.members(i);
      if (isMember(rMember.charid()) == true)
      {
        XERR << "[公会管理-加载]" << guid << name << "添加成员 charid :" << rMember.charid() << "失败,已是其他公会成员" << XEND;
        continue;
      }

      GMember* pMember = NEW GMember(pGuild, oUser, rMember.job());
      if (pMember == nullptr)
      {
        XERR << "[公会管理-加载]" << guid << name << "创建成员 :" << oBlob.members(i).ShortDebugString() << "失败" << XEND;
        continue;
      }
      pMember->fromData(oBlob.members(i));
      pGuild->m_vecMembers.push_back(pMember);

      if (pMember->isOnline() == true)
        pMember->resetOfflineTime(dwNow);
      m_mapUserID2Guild[pMember->getCharID()] = pGuild;

      if (pGuild->getGUID() != pMember->getGuildID())
        addAdjustCharID(pMember->getCharID());
    }

    // apply
    string apply;
    apply.assign((const char *)set[q].getBin("apply"), set[q].getBinSize("apply"));
    BlobGuildApply oApply;
    if (oBlob.ParseFromString(apply) == false)
    {
      XERR << "[公会管理-加载]" << guid << name << "公会申请序列化失败" << XEND;
      return false;
    }
    DWORD dwMaxApply = MiscConfig::getMe().getGuildCFG().dwMaxApplyCount;
    for (int i = 0; i < oApply.applys_size(); ++i)
    {
      const GuildApply& rApply = oApply.applys(i);
      if (pGuild->m_vecApplys.size() >= dwMaxApply)
      {
        XERR << "[公会管理-加载]" << guid << name << "添加申请" << rApply.ShortDebugString() << "失败,超过上限,被忽略" << XEND;
        continue;
      }
      if (rApply.charid() == 0)
      {
        XERR << "[公会管理-加载]" << guid << name << "添加申请" << rApply.ShortDebugString() << "失败,charid = 0" << XEND;
        continue;
      }
      if (isMember(rApply.charid()) == true)
      {
        XERR << "[公会管理-加载]" << guid << name << "添加申请" << rApply.ShortDebugString() << "失败,已是其他公会成员" << XEND;
        continue;
      }

      GMember* pApply = NEW GMember(pGuild, oUser, EGUILDJOB_APPLY);
      if (pApply == nullptr)
      {
        XERR << "[公会-申请加载]" << guid << name << "添加申请" << rApply.ShortDebugString() << "失败" << XEND;
        continue;
      }
      pApply->fromData(rApply);
      pGuild->m_vecApplys.push_back(pApply);

      if (pApply->isOnline() == true)
        pApply->resetOfflineTime(dwNow);
      addApplyGuild(pApply->getCharID(), pGuild);
    }

    // other data
    pGuild->setBlobMiscString((const char *)set[q].getBin("misc"), set[q].getBinSize("misc"));
    pGuild->setBlobPackString((const char *)set[q].getBin("pack"), set[q].getBinSize("pack"));
    pGuild->setBlobEventString((const char *)set[q].getBin("event"), set[q].getBinSize("event"));
    pGuild->setBlobPhotoString((const char *)set[q].getBin("photo"), set[q].getBinSize("photo"));
  }

  XLOG << "[公会管理-加载] 成功加载" << this->size() << "个公会,总共耗时" << guildTimer.uElapse() << "微秒" << XEND;

  fixGuildPrayRestore();
  processOfflineGM();
  patch();
  //patch_1();
  return true;
}

bool GuildManager::loadOfflineData()
{
  // load guild offline
  xTime offlineTimer;
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guildoffline");
  if (pField == nullptr)
  {
    XERR << "[公会管理-加载离线] 获得数据库guildoffline" << XEND;
    return false;
  }

  xRecordSet setOffline;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, setOffline, nullptr);
  if (QWORD_MAX == retNum)
  {
    XERR << "[公会管理-加载] 查询guildoffline表失败" << retNum << XEND;
    return false;
  }

  for (QWORD q = 0; q < retNum; ++q)
  {
    QWORD qwCharID = setOffline[q].get<QWORD>("charid");

    auto m = m_mapUserGuildOffline.find(qwCharID);
    if (m != m_mapUserGuildOffline.end())
    {
      XERR << "[公会管理-加载] 成功加载 charid :" << qwCharID << "重复了" << XEND;
      continue;
    }

    SGuildOffline& stOffline = m_mapUserGuildOffline[qwCharID];
    stOffline.qwCharID = qwCharID;
    stOffline.qwGUILDID = setOffline[q].get<QWORD>("guildid");
    stOffline.bFreeze = setOffline[q].get<bool>("freeze");
    stOffline.dwContribute = setOffline[q].get<DWORD>("contribute");
    stOffline.dwTotalContribute = setOffline[q].get<DWORD>("totalcontribute");
    stOffline.dwGiftPoint = setOffline[q].get<DWORD>("giftpoint");
    stOffline.strPray.assign((const char *)setOffline[q].getBin("pray"), setOffline[q].getBinSize("pray"));
    //stOffline.strDonate.assign((const char *)setOffline[q].getBin("donate"), setOffline[q].getBinSize("donate"));
    stOffline.strVar.assign((const char *)setOffline[q].getBin("var"), setOffline[q].getBinSize("var"));
    stOffline.strBuilding.assign((const char *)setOffline[q].getBin("building"), setOffline[q].getBinSize("building"));
    stOffline.dwExitTime = setOffline[q].get<DWORD>("exittime");;
  }

  XLOG << "[公会管理-加载] 成功加载" << m_mapUserGuildOffline.size() << "个公会离线 目标 :" << retNum << "总共耗时" << offlineTimer.uElapse() << XEND;
  return true;
}

void GuildManager::reload(ConfigType type)
{
  for (auto &m : xEntryID::ets_)
  {
    Guild *p = (Guild *)m.second;
    p->reload(type);
  }
}

void GuildManager::delChar(QWORD qwCharID)
{
  auto apply = m_mapUserApplyGuild.find(qwCharID);
  if (apply != m_mapUserApplyGuild.end())
  {
    for (auto &s : apply->second)
      s->removeApply(qwCharID);
    m_mapUserApplyGuild.erase(apply);
    XLOG << "[公会管理-删号] charid :" << qwCharID << "删除申请成功" << XEND;
  }

  auto guild = m_mapUserID2Guild.find(qwCharID);
  if (guild != m_mapUserID2Guild.end())
  {
    SocialUser oUser;
    oUser.set_charid(qwCharID);
    if (exitGuild(oUser) == true)
      XLOG << "[公会管理-删号] charid :" << qwCharID << "退出公会成功" << XEND;
  }

  auto offline = m_mapUserGuildOffline.find(qwCharID);
  if (offline != m_mapUserGuildOffline.end())
  {
    m_setGOfflineUpdate.insert(qwCharID);
    m_mapUserGuildOffline.erase(offline);
    XLOG << "[公会管理-删号] charid :" << qwCharID << "删除离线数据成功" << XEND;
  }
}

void GuildManager::onUserOnline(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return;

  syncPrayToScene(rUser, GUILDOPTCONTYPE_LOGIN);
  syncPunishTimeToClient(rUser);

  auto apply = m_mapUserApplyGuild.find(rUser.charid());
  if (apply != m_mapUserApplyGuild.end())
  {
    for (auto &s : apply->second)
    {
      GMember* pApply = s->getApply(rUser.charid());
      if (pApply != nullptr && pApply->isOnline() == false)
        pApply->setOfflineTime(0);
    }
  }

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    GuildInfoSyncGuildSCmd sync;
    sync.set_charid(rUser.charid());
    PROTOBUF(sync, send, len);
    thisServer->sendCmdToZone(rUser.zoneid(), send, len);
    return;
  }

  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
  {
    GuildInfoSyncGuildSCmd sync;
    sync.set_charid(rUser.charid());
    PROTOBUF(sync, send, len);
    thisServer->sendCmdToZone(rUser.zoneid(), send, len);
    return;
  }

  EnterGuildGuildCmd cmd;
  pGuild->toData(cmd.mutable_data(), pMember->getJob() == EGUILDJOB_CHAIRMAN || pMember->getJob() == EGUILDJOB_VICE_CHAIRMAN, pMember);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

  DWORD dwNow = xTime::getCurSec();
  pMember->setOnlineTime(dwNow);
  pMember->setOfflineTime(0);
  pMember->setZoneID(rUser.zoneid());

  pGuild->syncInfoToScene(*pMember);
  pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_MEMBER_ONLINE, MsgParams(pGuild->getJobName(pMember->getJob()), pMember->getName()));

  if (pGuild->getDismissTime() != 0)
  {
    if (pGuild->getDismissTime() > dwNow)
    {
      DWORD dwDismissTime = pGuild->getDismissTime() - dwNow;
      thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_GUILD_DISMISS_ONLINE, MsgParams(dwDismissTime / 3600));
    }
  }

  pMember->redTipMessage(EREDSYS_GUILD_APPLY, pMember->hasRedTip());

  GMember* pChair = pGuild->getChairman();
  if (pChair == nullptr)
    pGuild->exchangeChairman(0, 0, EEXCHANGEMETHOD_FIND);

  if(EGUILDJOB_CHAIRMAN == pMember->getJob())
    GuildIconManager::getMe().onUserOnline(pGuild->getGUID(), pMember);

  pGuild->getMisc().getChallenge().notifyAllData(pMember, true);
  pGuild->getMisc().getWelfare().notifyMember(pMember);
  pGuild->getMisc().getArtifact().notifyAllData(pMember);

  //pMember->syncRedTipToScene(EREDSYS_GUILD_CHALLENGE_ADD);
  pMember->syncRedTipToScene(EREDSYS_GUILD_CHALLENGE_REWARD);

  if (m_bArtifactFixPack)
    pGuild->getMisc().getArtifact().fixPack();
  // 同步实时语音房间号
  pMember->sendRealtimeVoiceID();

  XLOG << "[公会管理-成员上线] 公会" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.ShortDebugString() << "上线了" << XEND;
}

void GuildManager::onUserOffline(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return;

  DWORD dwNow = xTime::getCurSec();
  auto apply = m_mapUserApplyGuild.find(rUser.charid());
  if (apply != m_mapUserApplyGuild.end())
  {
    for (auto &s : apply->second)
    {
      GMember* pApply = s->getApply(rUser.charid());
      if (pApply != nullptr && pApply->isOnline() == true)
        pApply->setOfflineTime(dwNow);
    }
  }

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return;
  pMember->setOfflineTime(dwNow);
  pMember->setFrameStatus(false, true);

  XLOG << "[公会管理-成员下线] 公会" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.ShortDebugString() << "下线了" << XEND;
}

void GuildManager::updateUserInfo(const UserInfoSyncSocialCmd& cmd)
{
  Guild* pGuild = getGuildByUserID(cmd.info().user().charid());
  if (pGuild != nullptr)
  {
    GMember* pMember = pGuild->getMember(cmd.info().user().charid());
    if (pMember != nullptr)
      pMember->updateData(cmd.info());
  }

  auto m = m_mapUserApplyGuild.find(cmd.info().user().charid());
  if (m != m_mapUserApplyGuild.end())
  {
    for (auto &s : m->second)
    {
      GMember* pApply = s->getApply(cmd.info().user().charid());
      if (pApply != nullptr)
        pApply->updateData(cmd.info());
    }
  }
}

void GuildManager::syncPrayToScene(const SocialUser& rUser, GuildOptConType eType, bool bLevelup /*= false*/)
{
  if (rUser.charid() == 0)
    return;

  GuildUserInfoSyncGuildCmd cmd;
  cmd.set_charid(rUser.charid());
  cmd.set_levelup(bLevelup);
  cmd.set_optcontype(eType);
  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild != nullptr)
  {
    GMember* pMember = pGuild->getMember(rUser.charid());
    if (pMember != nullptr)
      pMember->toScenePrayData(cmd);
  }
  else
  {
    auto m = m_mapUserGuildOffline.find(rUser.charid());
    if (m != m_mapUserGuildOffline.end())
    {
      BlobGuildPray oBlob;
      if (oBlob.ParseFromString(m->second.strPray) == true)
      {
        for (int i = 0; i < oBlob.prays_size(); ++i)
        {
          GuildMemberPray* pPray = cmd.mutable_info()->add_prays();
          if (pPray != nullptr)
            pPray->CopyFrom(oBlob.prays(i));
        }
      }
      cmd.mutable_info()->set_contribute(m->second.dwContribute);
      cmd.mutable_info()->set_giftpoint(m->second.dwGiftPoint);
    }
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(rUser.zoneid(), send, len);
}

void GuildManager::syncPunishTimeToClient(const SocialUser& rUser)
{
  const SGuildOffline* pOffline = getOfflineData(rUser.charid());
  if (pOffline == nullptr)
    return;
  EnterPunishTimeNtfGuildCmd cmd;
  cmd.set_exittime(pOffline->dwExitTime);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
  XDBG << "[公会管理-退会惩罚]" << rUser.ShortDebugString() << "同步惩罚时间" << cmd.ShortDebugString() << XEND;
}

void GuildManager::timer(DWORD curTime)
{
  //bool bDayReset = m_oDayTimer.timeUp(curTime);

  std::list<Guild *> listDeleteGuild;

  xTime frameTime;
  for (auto &it : xEntryID::ets_)
  {
    Guild* pGuild = (Guild *)(it.second);
    if (pGuild == nullptr)
    {
      continue;
    }

    if (pGuild->getMemberCount() == 0 || (pGuild->getDismissTime() != 0 && pGuild->getDismissTime() < curTime))
    {
      listDeleteGuild.push_back(pGuild);
      continue;
    }

    pGuild->timer(curTime);

    /*if (bDayReset)
    {
      pGuild->maintenance(curTime);
      pGuild->updateVar();
    }*/

    if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwGuildFrameOvertime)
    {
      XLOG << "[公会管理-帧] 帧处理时间 :" << frameTime.uElapse() << "超过 :" << CommonConfig::m_dwGuildFrameOvertime << "跳出循环" << XEND;
      break;
    }
  }

  frameTime.elapseStart();
  if (!listDeleteGuild.empty())
  {
    xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "guild");
    if (pField)
    {
      for (auto it : listDeleteGuild)
      {
        Guild* pGuild = (Guild *)(it);

        xRecord record(pField);
        record.put("status", 1);
        char szWhere[64] = {0};
        snprintf(szWhere, sizeof(szWhere), "id=%llu", pGuild->getGUID());
        QWORD ret = thisServer->getDBConnPool().exeUpdate(record, szWhere);
        if (ret == QWORD_MAX)
        {
          XERR << "[公会管理-合法检查] 公会" << pGuild->getGUID() << pGuild->getName() << "更新状态 1 失败" << XEND;
          continue;
        }

        pGuild->dismiss();
        removeApplyGuild(pGuild);

        m_setGuildName.erase(pGuild->getName());

        TSetQWORD setIDs;
        for (auto s = pGuild->m_vecMembers.begin(); s != pGuild->m_vecMembers.end(); ++s)
          setIDs.insert((*s)->getCharID());
        for (auto &s : setIDs)
        {
          if (removeMember(pGuild, s, false) == false)
            XERR << "[公会管理-合法检查] 公会" << pGuild->getGUID() << pGuild->getName() << "移除成员 " << s << "失败" << XEND;
        }

        //platlog
        EVENT_TYPE eType = EventType_GuildDismiss;
        ESOCIAL_TYPE eSocialType = ESocial_GuildDismiss;
        QWORD eid = xTime::getCurUSec();
        PlatLogManager::getMe().eventLog(thisServer,
            0,
            0,
            0,
            0,
            eid,
            0,  // charge
            eType, 0, 1);
        GMember* pChairman = pGuild->getChairman();
        QWORD chairManId = 0;
        if (pChairman)
          chairManId = pChairman->getCharID();

        PlatLogManager::getMe().SocialLog(thisServer,
            0,
            0,
            0,
            0,
            eType,
            eid,
            eSocialType,
            pGuild->getGUID(),
            chairManId,
            0,
            0);

        XLOG << "[公会管理-合法检查] 公会" << pGuild->getGUID() << pGuild->getName() << "成员人数 :" << pGuild->getMemberCount() <<
          "解散时间 :" << pGuild->getDismissTime() << "当前时间 :" << curTime << "从列表中删除" << XEND;

        // 删除公会icon信息
        GuildIconManager::getMe().clearGuild(pGuild->getGUID());

        // 放最后  会删除pGuild指针
        delGuild(pGuild);

        if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwGuildFrameOvertime)
        {
          XLOG << "[公会管理-合法检查] 处理时间 :" << frameTime.uElapse() << "超过 :" << CommonConfig::m_dwGuildFrameOvertime << "跳出循环" << XEND;
          break;
        }
      }
    }
    else
    {
      XERR << "[公会管理-删除]" << "获取数据库" << REGION_DB << "失败" << XEND;
    }
    listDeleteGuild.clear();
  }

  // update guild offline
  updateOffline(curTime);
  // adjust member info
  adjustMemberRedis();
}

ESysMsgID GuildManager::checkName(const string& name)
{
  if (name.empty() == true)
    return ESYSTEMMSG_ID_GUILD_NAMEEMPTY;
  auto s = m_setGuildName.find(name);
  if (s != m_setGuildName.end())
    return ESYSTEMMSG_ID_GUILD_NAMEDUPLICATE;
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  return rCFG.checkNameValid(name, ENAMETYPE_GUILD);
}

ESysMsgID GuildManager::addName(const string& name)
{
  ESysMsgID eMsgID = checkName(name);
  if (eMsgID == ESYSTEMMSG_ID_MIN)
    m_setGuildName.insert(name);
  return eMsgID;
}

bool GuildManager::removeName(const string& name)
{
  auto s = m_setGuildName.find(name);
  if (s == m_setGuildName.end())
    return false;

  m_setGuildName.erase(name);
  return true;
}

Guild* GuildManager::getGuildByID(QWORD qwGuildID)
{
  return (Guild *)getEntryByID(qwGuildID);
}

Guild* GuildManager::getGuildByUserID(QWORD qwCharID)
{
  auto m = m_mapUserID2Guild.find(qwCharID);
  if (m != m_mapUserID2Guild.end())
    return m->second;

  return nullptr;
}

Guild* GuildManager::getGuildByGuildName(const string& strName)
{
  auto m = m_mapName2Guild.find(strName);
  if (m != m_mapName2Guild.end())
    return m->second;
  return nullptr;
}

TSetGuild& GuildManager::getApplyGuild(QWORD qwCharID)
{
  static TSetGuild e;

  auto m = m_mapUserApplyGuild.find(qwCharID);
  if (m == m_mapUserApplyGuild.end())
    return e;

  return m->second;
}

bool GuildManager::moveGuildZone(DWORD dwFromZone, DWORD dwToZone)
{
  if (dwFromZone == 0 || dwToZone == 0 || dwFromZone == dwToZone)
    return false;

  for (auto &m : xEntryID::ets_)
  {
    Guild *p = (Guild *)m.second;
    if (p->getZoneID() != dwFromZone)
      continue;
    if (p->getZoneID() == dwToZone)
      continue;
    p->setZoneID(dwToZone);
    XLOG << "[公会管理-切换公会线]" << p->getGUID() << p->getName() << "从线" << dwFromZone << "切换到了" << dwToZone << XEND;
  }

  return true;
}

bool GuildManager::doGuildCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case GUILDPARAM_GUILDLIST:
      {
        PARSE_CMD_PROTOBUF(QueryGuildListGuildCmd, rev);
        queryGuildList(rUser, rev.keyword(), rev.page());
      }
      break;
    case GUILDPARAM_CREATEGUILD:
      {
        if (isMember(rUser.charid()) == true)
          break;

        const SGuildCFG* pBase = GuildConfig::getMe().getGuildCFG(1);
        if (pBase == nullptr)
          break;

        PARSE_CMD_PROTOBUF(CreateGuildGuildCmd, rev);
        DWORD dwMsgID = GuildManager::getMe().addName(rev.name());
        if (dwMsgID != 0)
        {
          thisServer->sendMsg(rUser.zoneid(), rUser.charid(), dwMsgID);
          break;
        }

        CreateGuildSocialCmd cmd;
        cmd.mutable_user()->mutable_user()->CopyFrom(rUser);
        cmd.set_name(rev.name());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToScene(rUser.zoneid(), rUser.charid(), send, len);
      }
      break;
    case GUILDPARAM_PROCESSAPPLY:
      {
        PARSE_CMD_PROTOBUF(ProcessApplyGuildCmd, rev);
        processApplyGuild(rUser, rev.charid(), rev.action());
      }
      break;
    case GUILDPARAM_INVITEMEMBER:
      {
        PARSE_CMD_PROTOBUF(InviteMemberGuildCmd, rev);
        inviteMember(rUser, rev.charid());
      }
      break;
    case GUILDPARAM_SETOPTION:
      {
        PARSE_CMD_PROTOBUF(SetGuildOptionGuildCmd, rev);
        setOption(rUser, rev);
      }
      break;
    case GUILDPARAM_KICKMEMBER:
      {
        PARSE_CMD_PROTOBUF(KickMemberGuildCmd, rev);
        kickMember(rUser, rev.charid());
      }
      break;
    case GUILDPARAM_CHANGEJOB:
      {
        PARSE_CMD_PROTOBUF(ChangeJobGuildCmd, rev);
        changeJob(rUser, rev.charid(), rev.job());
      }
      break;
    case GUILDPARAM_EXITGUILD:
      {
        exitGuild(rUser);
      }
      break;
    case GUILDPARAM_EXCHANGECHAIR:
      {
        PARSE_CMD_PROTOBUF(ExchangeChairGuildCmd, rev);
        exchangeChair(rUser, rev.newcharid());
      }
      break;
    case GUILDPARAM_DISMISSGUILD:
      {
        PARSE_CMD_PROTOBUF(DismissGuildCmd, rev);
        dismissGuild(rUser, rev.set());
      }
      break;
    case GUILDPARAM_LEVELUPGUILD:
      {
        levelupGuild(rUser);
      }
      break;
    case GUILDPARAM_DONATE:
      {
        PARSE_CMD_PROTOBUF(DonateGuildCmd, rev);
        donate(rUser, rev.configid(), rev.time());
      }
      break;
    case GUILDPARAM_DONATELIST:
      {
        PARSE_CMD_PROTOBUF(DonateListGuildCmd, rev);
        donateList(rUser);
      }
      break;
    case GUILDPARAM_DONATEFRAMESTATUS:
      {
        PARSE_CMD_PROTOBUF(DonateFrameGuildCmd, rev);
        setDonateFrame(rUser, rev.open());
      }
      break;
    case GUILDPARAM_ENTERGUILDTERRITORY:
      {
        PARSE_CMD_PROTOBUF(EnterTerritoryGuildCmd, rev);
        enterTerritory(rUser, rev.handid());
      }
      break;
    case GUILDPARAM_PRAY:
      {
        PARSE_CMD_PROTOBUF(PrayGuildCmd, rev);
        pray(rUser, rev.npcid(), rev.pray());
      }
      break;
    case GUILDPARAM_LEVELUPEFFECT:
      levelupEffect(rUser);
      break;
    case GUILDPARAM_QUERYPACK:
      {
        PARSE_CMD_PROTOBUF(QueryPackGuildCmd, rev);
        Guild* pGuild = getGuildByUserID(rUser.charid());
        if (pGuild == nullptr)
        {
          XERR << "[公会管理-包裹请求]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "请求包裹失败, 未在公会内" << XEND;
          break;
        }
        GMember* pMember = pGuild->getMember(rUser.charid());
        if (pMember == nullptr)
        {
          XERR << "[公会管理-包裹请求]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "请求包裹失败, 不是公会成员" << XEND;
          break;
        }
        QueryPackGuildCmd packcmd;
        pGuild->getPack().toData(packcmd);
        PROTOBUF(packcmd, packsend, packlen);
        pMember->sendCmdToMe(packsend, packlen);
        XLOG << "[公会管理-包裹请求]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "请求包裹数据成功过,获得" << packcmd.items_size() << "个物品" << XEND;
      }
      break;
    case GUILDPARAM_EXCHANGEZONE:
      {
        PARSE_CMD_PROTOBUF(ExchangeZoneGuildCmd, rev);
        exchangeZone(rUser, rev.zoneid(), rev.set());
      }
      break;
    case GUILDPARAM_EXCHANGEZONE_ANSWER:
      {
        PARSE_CMD_PROTOBUF(ExchangeZoneAnswerGuildCmd, rev);
        memberExchangeZoneAnswer(rUser, rev.agree());
      }
      break;
    case GUILDPARAM_QUERY_EVENT_LIST:
      {
        PARSE_CMD_PROTOBUF(QueryEventListGuildCmd, rev);
        queryEventList(rUser);
      }
      break;
    case GUILDPARAM_APPLYCONFIG:
      {
        PARSE_CMD_PROTOBUF(ApplyRewardConGuildCmd, rev);
        applyGuildReward(rUser, rev.configid());
      }
      break;
    case GUILDPARAM_FRAME_STATUS:
      {
        PARSE_CMD_PROTOBUF(FrameStatusGuildCmd, rev);
        frameStatus(rUser, rev.open());
      }
      break;
    case GUILDPARAM_MODIFY_AUTH:
      {
        PARSE_CMD_PROTOBUF(ModifyAuthGuildCmd, rev);
        modifyAuth(rUser, rev);
      }
      break;
    case GUILDPARAM_RENAME_QUERY:
      {
        PARSE_CMD_PROTOBUF(RenameQueryGuildCmd, rev);
        checkRename(rUser, rev.name());
      }
      break;
    case GUILDPARAM_CITY_ACTION:
      {
        PARSE_CMD_PROTOBUF(CityActionGuildCmd, rev);
        cityAction(rUser, rev.action());
      }
      break;
    case GUILDPARAM_GUILD_ICON_ADD:
      {
        PARSE_CMD_PROTOBUF(GuildIconAddGuildCmd, rev);
        Guild* pGuild = getGuildByUserID(rUser.charid());
        if(!pGuild)
        {
          XERR << "[公会管理-添加、删除自定义图标]" << rUser.accid() << rUser.charid() << rev.ShortDebugString() << "请求失败，未在公会" << XEND;
          return false;
        }
        GMember* pMember = pGuild->getMember(rUser.charid());
        if(!pMember)
        {
          XERR << "[公会管理-添加、删除自定义图标]" << rUser.accid() << rUser.charid() << rev.ShortDebugString() << "请求失败，不是成员" << XEND;
          return false;
        }

        if(EGUILDJOB_CHAIRMAN != pMember->getJob())
        {
          XERR << "[公会管理-添加、删除自定义图标]" << rUser.ShortDebugString() << pMember->getJob() << "请求失败，权限不足" << XEND;
          return false;
        }

        GuildIconSyncGuildCmd cmd;
        if(rev.isdelete())
        {
          TVecDWORD vec;
          numTok(pGuild->getPortrait(), "_", vec);
          if(2 == vec.size() && rev.index() == vec[0])
          {
            XERR << "[公会管理-删除自定义图标]" << "请求失败，图标正在被使用" << pGuild->getGUID() << rev.index() << pGuild->getPortrait() << XEND;
            return false;
          }

          GuildIconManager::getMe().remove(pGuild->getGUID(), rev.index());
          cmd.add_dels(rev.index());
        }
        else
        {
          GuildIconManager::getMe().add(pGuild->getGUID(), rUser.charid(), rev.index(), rev.type());
          IconInfo* pInfo = cmd.add_infos();
          pInfo->set_index(rev.index());
          pInfo->set_time(rev.createtime());
          pInfo->set_type(rev.type());
        }

        PROTOBUF(cmd, send, len);
        pMember->sendCmdToMe(send, len);
      }
      break;
    case GUILDPARAM_GUILD_ICON_UPLOAD:
      {
        PARSE_CMD_PROTOBUF(GuildIconUploadGuildCmd, rev);
        Guild* pGuild = getGuildByUserID(rUser.charid());
        if(!pGuild)
        {
          XERR << "[公会管理-上传自定义图标]" << rUser.accid() << rUser.charid() << rUser.profession() << "请求失败，未在公会中" << XEND;
          return false;
        }
        GMember* pMember = pGuild->getMember(rUser.charid());
        if(!pGuild)
        {
          XERR << "[公会管理-上传自定义图标]" << rUser.accid() << rUser.charid() << rUser.profession() << "请求失败，不是成员" << XEND;
          return false;
        }

        // 检测职位
        if(EGUILDJOB_CHAIRMAN != pMember->getJob())
        {
          XERR << "[公会管理-上传自定义图标]" << rUser.ShortDebugString() << pMember->getJob() << "请求失败，权限不足" << XEND;
          return false;
        }
        // 检测index
        if(GuildIconManager::getMe().hasIndex(pGuild->getGUID(), rev.index()))
        {
          XERR << "[公会管理-上传自定义图标]" << rUser.ShortDebugString() << pMember->getJob() << "请求失败，index已存在" << XEND;
          return false;
        }

        std::stringstream stream;
        stream.str("");
        if(thisServer->isOuter())
          stream << "/game/icon/";
        else
          stream << "/debug/icon/";
        stream << thisServer->getRegionID() << "/guild/" << pGuild->getGUID() << "/" << rev.index();

        std::string policy,signature;
        CommonConfig::upyun_form_str(stream.str(), policy, signature, rev.type());

        GuildIconUploadGuildCmd cmd;
        cmd.set_policy(policy);
        cmd.set_signature(signature);
        cmd.set_index(rev.index());
        cmd.set_type(rev.type());

        PROTOBUF(cmd, send, len);
        pMember->sendCmdToMe(send, len);
      }
      break;
    case GUILDPARAM_OPEN_FUNCTION:
      {
        PARSE_CMD_PROTOBUF(OpenFunctionGuildCmd, rev);
        openFunction(rUser, rev.func());
      }
      break;
    case GUILDPARAM_BUILD:
      {
        PARSE_CMD_PROTOBUF(BuildGuildCmd, rev);
        build(rUser, rev.building());
      }
      break;
    case GUILDPARAM_SUBMIT_MATERIAL:
      {
        PARSE_CMD_PROTOBUF(SubmitMaterialGuildCmd, rev);
        submitMaterial(rUser, rev);
      }
      break;
    case GUILDPARAM_BUILDING_SUBMIT_COUNT:
      {
        PARSE_CMD_PROTOBUF(BuildingSubmitCountGuildCmd, rev);
        querySubmitCount(rUser, rev.type());
      }
      break;
    case GUILDPARAM_GET_WELFARE:
      {
        getWelfare(rUser);
      }
      break;
    case GUILDPARAM_BUILDING_LVUP_EFF:
      {
        PARSE_CMD_PROTOBUF(BuildingLvupEffGuildCmd, rev);
        buildingLevelUpEffect(rUser, rev);
      }
      break;
    case GUILDPARAM_ARTIFACT_PRODUCE:
      {
        PARSE_CMD_PROTOBUF(ArtifactProduceGuildCmd, rev);
        produceArtifact(rUser, rev);
      }
      break;
    case GUILDPARAM_ARTIFACT_OPT:
      {
        PARSE_CMD_PROTOBUF(ArtifactOptGuildCmd, rev);
        optArtifact(rUser, rev);
      }
      break;
    case GUILDPARAM_QUERY_GQUEST:
      queryGQuest(rUser);
      break;
    case GUILDPARAM_QUERY_BUILDING_RANK:
      {
        PARSE_CMD_PROTOBUF(QueryBuildingRankGuildCmd, rev);
        queryBuildingSubmitRank(rUser, rev.type());
      }
      break;
    case GUILDPARAM_TREASURE_QUERYRESULT:
      {
        PARSE_CMD_PROTOBUF(QueryTreasureResultGuildCmd, rev);
        queryTreasureResult(rUser, rev);
      }
      break;
    case GUILDPARAM_OPEN_REALTIME_VOICE:
      {
        PARSE_CMD_PROTOBUF(OpenRealtimeVoiceGuildCmd, rev);
        openRealtimeVoice(rUser, rev.charid(), rev.open());
      }
      break;
    default:
      return false;
  }

  return true;
}

bool GuildManager::doUserDojoCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
  case EDOJOPARAM_DOJO_PUBLIC_INFO:
  {
    PARSE_CMD_PROTOBUF(DojoPublicInfoCmd, rev);
    Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
    if (!pGuild)
    {
      return false;
    }
    pGuild->getMisc().getPublicInfo(rUser, rev);    
  }
  break;
  case EDOJOPARAM_ADD_MSG:
  {
    PARSE_CMD_PROTOBUF(DojoAddMsg, rev);
    Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
    if (!pGuild)
    {
      return false;
    } 
    pGuild->getMisc().addMsg(rUser, rev);
    return true;
  }
  break;
  default:
    return false;
  }

  return true;
}

bool GuildManager::queryGuildList(const SocialUser& rUser, const string& keyword, DWORD dwPage)
{
  if (rUser.charid() == 0 || rUser.accid() == 0)
    return false;

  if (dwPage > 0)
    --dwPage;

  string keywordex;
  keywordex.resize(keyword.size());
  transform(keyword.begin(), keyword.end(), keywordex.begin(), ::tolower);

  set<Guild*> setTemp;
  for (auto &m : xEntryID::ets_)
  {
    Guild *p = (Guild *)m.second;
    if (keyword.empty() == false)
    {
      string nameex = p->getName();
      nameex.resize(p->getName().size());
      transform(p->getName().begin(), p->getName().end(), nameex.begin(), ::tolower);
      if (nameex.find(keywordex) == string::npos)
        continue;
    }
    else
    {
      if (p->getZoneID() != rUser.zoneid())
        continue;
    }

    setTemp.insert(p);
  }

  DWORD dwCount = 0;
  DWORD dwAdd = 0;
  DWORD dwStart = dwPage * EGUILDGLOBAL_LISTCOUNT_PERPAGE;
  QueryGuildListGuildCmd cmd;
  for (auto s : setTemp)
  {
    if (dwCount++ < dwStart)
      continue;

    if (dwAdd++ >= EGUILDGLOBAL_LISTCOUNT_PERPAGE)
      break;

    s->toSummaryData(cmd.add_list());
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
  //pUser->sendCmdToMe(send, len);

  XLOG << "[公会管理-获取列表]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
    << "查询公会列表, 页数 :" << dwPage << "获取了" <<  (DWORD)cmd.list_size() << "公会" << XEND;
  return true;
}

bool GuildManager::createGuild(const UserInfo& rUser, const string& name)
{
  const SocialUser& user = rUser.user();
  if (user.charid() == 0 || isMember(user.charid()) == true)
    return false;

  // create guild
  Guild* pGuild = NEW Guild(0, user.zoneid(), name);
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "创建公会" << name << "失败" << XEND;
    return false;
  }

  // init data
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  pGuild->m_dwLevel = 1;
  pGuild->m_dwCreateTime = xTime::getCurSec();
  pGuild->m_strPortrait = rCFG.strDefaultPortrait;

  const TMapGuildJobCFG& mapJobCFG = GuildConfig::getMe().getGuildJobList();
  for (auto &m : mapJobCFG)
  {
    pGuild->setJobName(m.first, m.second.name);
    pGuild->getMisc().setAuth(m.first, m.second.dwDefaultAuth);
    pGuild->getMisc().setEditAuth(m.first, m.second.dwDefaultEditAuth);
  }
  const SGuildCFG* pBase = GuildConfig::getMe().getGuildCFG(pGuild->m_dwLevel);
  if (pBase == nullptr)
  {
    XERR << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "创建公会" << name << "失败, 无法获取1级公会配置" << XEND;
    return false;
  }
  pGuild->setGuildCFG(pBase);

  GMember* pLeader = NEW GMember(pGuild, rUser.user(), EGUILDJOB_CHAIRMAN);
  if (pLeader == nullptr)
  {
    XERR << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "创建公会" << name << "失败,创建会长失败" << XEND;
    return false;
  }
  pLeader->updateData(rUser, true);
  pGuild->m_vecMembers.push_back(pLeader);

  pLeader = pGuild->getMember(user.charid());
  if (pLeader == nullptr)
  {
    SAFE_DELETE(pGuild);
    SAFE_DELETE(pLeader);
    XERR << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "创建公会" << name << "添加会长失败" << XEND;
    return false;
  }

  obtainOfflineData(pGuild, pLeader);

  // insert to db
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild");
  if (pField == nullptr)
  {
    XERR << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "获取数据库" << REGION_DB << "失败" << XEND;
    removeMember(pGuild, user.charid(), false);
    SAFE_DELETE(pGuild);
    // pLeader由removeMember进行内存管理
    //SAFE_DELETE(pLeader);
    return false;
  }

  xRecord record(pField);
  pGuild->toRecord(record);

  QWORD ret = thisServer->getDBConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "插入数据库失败 ret :" << ret << XEND;
    removeMember(pGuild, user.charid(), false);
    SAFE_DELETE(pGuild);
    // pLeader由removeMember进行内存管理
    //SAFE_DELETE(pLeader);
    return false;
  }
  pGuild->setGUID(ret);

  if (!addGuild(pGuild))
  {
    XERR << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "添加到管理器失败" << XEND;
    removeMember(pGuild, user.charid(), false);
    SAFE_DELETE(pGuild);
    // pLeader由removeMember进行内存管理
    //SAFE_DELETE(pLeader);
    return false;
  }

  // clear update mark
  pGuild->m_bitset.reset();
  pGuild->m_recordbitset.reset();

  std::string str;
  pGuild->setBlobMiscString(str.c_str(), str.size());
  pGuild->setBlobPackString(str.c_str(), str.size());
  pGuild->setBlobEventString(str.c_str(), str.size());

  // refresh quest
  pGuild->getMisc().refreshQuest(xTime::getCurSec());

  // add event
  pGuild->getEvent().addEvent(EGUILDEVENT_CREATEGUILD, TVecString{user.name(), name});

  // inform client
  EnterGuildGuildCmd cmd;
  pGuild->toData(cmd.mutable_data(), pLeader->getJob() == EGUILDJOB_CHAIRMAN, pLeader);
  PROTOBUF(cmd, send, len);
  pLeader->sendCmdToMe(send, len);

  // inform scene
  pGuild->syncInfoToScene(*pLeader, true);

  // update to list
  m_mapUserID2Guild[user.charid()] = pGuild;
  m_setGuildName.insert(pGuild->getName());

  // send world msg
  thisServer->sendWorldMsg(user.zoneid(), ESYSTEMMSG_ID_GUILD_CREATE, MsgParams(pLeader->getName(), pGuild->getName()));
  // send self msg
  thisServer->sendMsg(user.zoneid(), user.charid(), ESYSTEMMSG_ID_GUILD_CREATEOK);

  // update gchar info
  GCharWriter pGChar(thisServer->getRegionID(), rUser.user().charid());
  pGChar.setGuildID(pGuild->getGUID());
  pGChar.setGuildName(pGuild->getName());
  pGChar.setGuildPortrait(pGuild->getPortrait());
  pGChar.save();

  // 实时语音房间号
  pLeader->sendRealtimeVoiceID();

  XLOG << "[公会管理-创建公会]" << user.accid() << user.charid() << user.profession() << user.name() << "创建公会" << pGuild->getGUID() << name << "成功" << XEND;

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_GuildCreate;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rUser.user().zoneid(),
    rUser.user().accid(),
    rUser.user().charid(),
    eid,
    0,  // charge
    eType, 0, 1);

  PlatLogManager::getMe().SocialLog(thisServer,
    0,
    rUser.user().zoneid(),
    rUser.user().accid(),
    rUser.user().charid(),
    eType,
    eid,
    ESocial_GuildCreate,
    pGuild->getGUID(),
    0,
    0,
    0);

  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_CREATE_GUILD_COUNT, 0, 0, user.baselv(), (DWORD)1);
  return true;
}

bool GuildManager::applyGuild(const UserInfo& rUser, QWORD qwGuildID)
{
  if (rUser.user().charid() == 0 || isMember(rUser.user().charid()) == true)
    return false;

  Guild* pGuild = getGuildByID(qwGuildID);
  if (pGuild == nullptr)
  {
    thisServer->sendMsg(rUser.user().zoneid(), rUser.user().charid(), ESYSTEMMSG_ID_GUILD_NOTEXIST);
    return false;
  }

  const SGuildOffline* pOffline = getOfflineData(rUser.user().charid());
  if (pOffline != nullptr)
  {
    const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
    if (pOffline->dwExitTime + rCFG.dwEnterPunishTime > now())
    {
      XERR << "[公会管理-添加申请]" << rUser.user().ShortDebugString() << "申请加入公会" << pGuild->getGUID() << pGuild->getName() << "失败,还在惩罚期" << XEND;
      return false;
    }
  }

  if (pGuild->addApply(rUser) == false)
    return false;

  addApplyGuild(rUser.user().charid(), pGuild);
  thisServer->sendMsg(rUser.user().zoneid(), rUser.user().charid(), ESYSTEMMSG_ID_GUILD_APPLY, MsgParams(pGuild->getName()));
  XLOG << "[公会管理-添加申请]" << rUser.user().accid() << rUser.user().charid() << rUser.user().profession() << rUser.user().name()
    << "成功申请了" << pGuild->getGUID() << pGuild->getName() << "公会" << XEND;

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_GuildApply;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rUser.user().zoneid(),
    rUser.user().accid(),
    rUser.user().charid(),
    eid,
    0,  //charge
    eType, 0, 1);
  PlatLogManager::getMe().SocialLog(thisServer,
    0,
    rUser.user().zoneid(),
    rUser.user().accid(),
    rUser.user().charid(),
    eType,
    eid,
    ESocial_GuildApply,
    pGuild->getGUID(),
    0,
    pGuild->getLevel(),
    0);

  return true;
}

bool GuildManager::processApplyGuild(const SocialUser& rUser, QWORD qwCharID, EGuildAction eAction)
{
  if (eAction <= EGUILDACTION_MIN || eAction >= EGUILDACTION_MAX)
    return false;
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;

  GMember* pChairMember = pGuild->getMember(rUser.charid());
  if (pChairMember == nullptr || pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_AGREE) == false)
    return false;

  GMember* pApply = pGuild->getApply(qwCharID);
  if (pApply == nullptr)
    return false;

  QWORD charid = pApply->getCharID();
  string name = pApply->getName();

  if (isMember(qwCharID) == true)
  {
    removeApplyGuild(qwCharID, pGuild, true);
    thisServer->sendMsg(pChairMember->getZoneID(), pChairMember->getCharID(), ESYSTEMMSG_ID_GUILD_MEMBER);
    XLOG << "[公会管理-处理申请] " << charid << ", " << name << " 已加入其他公会" << XEND;
    return false;
  }

  string str;
  if (eAction == EGUILDACTION_AGREE)
  {
    // check max count
    const SGuildCFG* pCFG = pGuild->getGuildCFG();
    if (pCFG == nullptr || pGuild->getMemberCount() >= pCFG->dwMaxMember)
    {
      thisServer->sendMsg(pChairMember->getZoneID(), pChairMember->getCharID(), ESYSTEMMSG_ID_GUILD_MAXMEMBER);
      return false;
    }

    SocialUser oApply;
    oApply.set_charid(qwCharID);
    oApply.set_zoneid(pApply->getZoneID());
    oApply.set_baselv(pApply->getBaseLevel());
    if (addMember(pGuild, oApply, EGUILDJOB_MEMBER1) == false)
      return false;
    str = "同意";
  }
  else if (eAction == EGUILDACTION_DISAGREE)
  {
    str = "拒绝";
    removeApplyGuild(qwCharID, pGuild, false);
  }
  else
  {
    XERR << "[公会管理-处理申请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pChairMember->getJob() << "执行了不合法的action :" << eAction << XEND;
    return false;
  }

  XLOG << "[公会管理-处理申请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pChairMember->getJob() << str << charid << name << "的申请请求" << XEND;
  return true;
}

bool GuildManager::inviteMember(const SocialUser& rUser, QWORD qwCharID)
{
  if (rUser.charid() == 0 || rUser.charid() == qwCharID)
    return false;
  if (isMember(qwCharID) == true)
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_GUILD_MEMBER);
    return false;
  }

  const SGuildOffline* pOffline = getOfflineData(qwCharID);
  if (pOffline != nullptr)
  {
    const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
    if (pOffline->dwExitTime + rCFG.dwEnterPunishTime > now())
    {
      thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 4045);
      return false;
    }
  }

  GCharReader pGChar(thisServer->getRegionID(), qwCharID);
  if (!pGChar.get())
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  const SGuildCFG* pCFG = pGuild->getGuildCFG();
  if (pCFG == nullptr || pGuild->getMemberCount() >= pCFG->dwMaxMember)
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_GUILD_MAXMEMBER);
    return false;
  }

  GMember* pChairMember = pGuild->getMember(rUser.charid());
  if (pChairMember == nullptr)
  {
    XERR << "[公会管理-邀请]" << rUser.ShortDebugString() << "邀请" << qwCharID << "失败,不是公会成员" << XEND;
    return false;
  }
  if (pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_INVITE) == false)
  {
    XERR << "[公会管理-邀请]" << rUser.ShortDebugString() << "邀请" << qwCharID << "失败,没有权限" << XEND;
    return false;
  }

  GMember* pInvite = pGuild->getInvite(qwCharID);
  if (pInvite != nullptr)
    return false;
  if (pGuild->addInvite(qwCharID) == false)
    return false;

  InviteMemberGuildCmd cmd;
  cmd.set_charid(pChairMember->getCharID());
  cmd.set_guildid(pGuild->getGUID());
  cmd.set_guildname(pGuild->getName());
  cmd.set_invitename(pChairMember->getName());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(pGChar.getZoneID(), qwCharID, send, len);

  XLOG << "[公会管理-邀请加入]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << " job :" << pChairMember->getJob()
    << "邀请" << qwCharID << pGChar.getName() << "加入" << pGuild->getGUID() << pGuild->getName() << "公会" << XEND;
  return true;
}

bool GuildManager::processInviteMember(const UserInfo& rUser, QWORD qwGuildID, EGuildAction eAction)
{
  const SocialUser& user = rUser.user();
  if (eAction <= EGUILDACTION_MIN || eAction >= EGUILDACTION_MAX)
    return false;
  if (rUser.user().charid() == 0)
    return false;

  Guild* pGuild = getGuildByID(qwGuildID);
  if (pGuild == nullptr)
    return false;

  if (isMember(user.charid()) == true)
  {
    XLOG << "[公会管理-处理邀请]" << user.accid() << user.charid() << user.profession() << user.name() << "已加入其他公会" << XEND;
    return false;
  }

  GMember* pInvite = pGuild->getInvite(user.charid());
  if (pInvite == nullptr)
  {
    XLOG << "[公会管理-处理邀请] 没有邀请过" << user.accid() << user.charid() << user.profession() << user.name() << "该玩家" << XEND;
    return false;
  }

  string str;
  if (eAction == EGUILDACTION_AGREE)
  {
    pInvite->setZoneID(rUser.user().zoneid());
    pInvite->updateData(rUser, true);
    if (addMember(pGuild, rUser.user(), EGUILDJOB_MEMBER1) == false)
      return false;
    str = "同意";
  }
  else if (eAction == EGUILDACTION_DISAGREE)
  {
    str = "拒绝";
    pGuild->removeInvite(user.charid());
  }
  else
  {
    return false;
  }

  XLOG << "[公会管理-处理邀请]" << user.accid() << user.charid() << user.profession() << user.name() << str << "加入" << pGuild->getGUID() << pGuild->getName() << XEND;
  return true;
}

bool GuildManager::setOption(const SocialUser& rUser, const SetGuildOptionGuildCmd& cmd)
{
  if (rUser.charid() == 0)
  {
    XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置属性" << cmd.ShortDebugString() << "失败,user不合法" << XEND;
    return false;
  }

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置属性" << cmd.ShortDebugString() << "失败,不是公会成员" << XEND;
    return false;
  }

  GMember* pChairMember = pGuild->getMember(rUser.charid());
  if (pChairMember == nullptr)
  {
    XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << "属性" << cmd.ShortDebugString() << "失败,不是公会成员" << XEND;
    return false;
  }

  bool bUpdate = false;
  if (cmd.portrait() != "")
  {
    /*决战期间不可修改*/
    if (pGuild->getMisc().getGvg().inSuperGvg())
    {
      thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 25518);
    }
    else
    {
      if (cmd.portrait() == "null")
        pGuild->setPortrait("");
      else if(GuildIconManager::getMe().checkPortrait(pGuild->getGUID(), cmd.portrait()))
        pGuild->setPortrait(cmd.portrait());
      bUpdate = true;
    }
  }

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  if (cmd.board() != "")
  {
    if (pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_EDIT_BOARD) == false)
    {
      XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << "属性" << "失败,没有权限" << XEND;
      return false;
    }
    if (cmd.board() == "null")
    {
      pGuild->setBoard(rUser.zoneid(), rUser.charid(), "");
    }
    else
    {
      if (rCFG.checkNameValid(cmd.board(), ENAMETYPE_GUILD_BOARD) != ESYSTEMMSG_ID_MIN)
      {
        XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << "属性" << "失败,公告含有屏蔽字" << XEND;
        return false;
      }
      pGuild->setBoard(rUser.zoneid(), rUser.charid(), cmd.board());
    }
    pGuild->getEvent().addEvent(EGUILDEVENT_EDIT_BOARD, TVecString{pChairMember->getName()});
    bUpdate = true;
  }

  if (cmd.recruit() != "")
  {
    if (pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_EDIT_RECRUIT) == false)
    {
      XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << "属性" << "失败,没有权限" << XEND;
      return false;
    }

    if (cmd.recruit() == "null")
    {
      pGuild->setRecruit("");
    }
    else
    {
      if (rCFG.checkNameValid(cmd.recruit(), ENAMETYPE_GUILD_RECRUIT) != ESYSTEMMSG_ID_MIN)
      {
        XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << "属性" << "失败,招募信息含有屏蔽字" << XEND;
        return false;
      }
      pGuild->setRecruit(cmd.recruit());
    }
    pGuild->getEvent().addEvent(EGUILDEVENT_EDIT_RECRUIT, TVecString{pChairMember->getName()});
    bUpdate = true;
  }

  for (int i = 0; i < cmd.jobs_size(); ++i)
  {
    if (pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_CHANGE_JOBNAME) == false)
    {
      XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << "属性" << "失败,没有权限" << XEND;
      return false;
    }

    const GuildJob& rJob = cmd.jobs(i);
    if (rCFG.checkNameValid(rJob.name(), ENAMETYPE_GUILD_JOB) != ESYSTEMMSG_ID_MIN)
    {
      XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << rJob.job() << "职位名失败,职位称号含有屏蔽字" << XEND;
      continue;
    }
    if (rJob.job() >= EGUILDJOB_MAX || rJob.job() == EGUILDJOB_APPLY || rJob.job() == EGUILDJOB_INVITE)
    {
      XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << rJob.job() << "职位名失败,职位枚举不合法" << XEND;
      continue;
    }
    const SGuildJobCFG* pCFG = GuildConfig::getMe().getGuildJobCFG(cmd.jobs(i).job());
    if (pCFG == nullptr || pGuild->getLevel() < pCFG->dwReqLv)
    {
      XERR << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << rJob.job() << "职位名失败,公会等级不足" << XEND;
      continue;
    }
    pGuild->setJobName(cmd.jobs(i).job(), cmd.jobs(i).name());
    pGuild->getEvent().addEvent(EGUILDEVENT_JOBNAME_CHANGE, TVecString{pChairMember->getName()});
    bUpdate = true;
  }

  if (bUpdate)
    XLOG << "[公会管理-属性设置]" << rUser.ShortDebugString() << "设置" << pGuild->getGUID() << pGuild->getName() << "属性" << cmd.ShortDebugString() << "成功" << XEND;
  return bUpdate;
}

bool GuildManager::kickMember(const SocialUser& rUser, QWORD qwCharID)
{
  if (rUser.charid() == 0)
    return false;
  if (rUser.charid() == qwCharID)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pChairMember = pGuild->getMember(rUser.charid());
  GMember* pMember = pGuild->getMember(qwCharID);
  if (pChairMember == nullptr || pMember == nullptr)
    return false;
  if (pMember->getJob() == EGUILDJOB_CHAIRMAN)
    return false;
  if (pMember->getJob() == EGUILDJOB_VICE_CHAIRMAN && pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_KICK_VICE) == false)
    return false;
  if (pMember->getJob() >= EGUILDJOB_MEMBER1 && pMember->getJob() <= EGUILDJOB_MEMBER3 && pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_KICK_MEMBER) == false)
    return false;

  QWORD charid = pMember->getCharID();
  DWORD zoneid = pMember->getZoneID();
  bool online = pMember->isOnline();
  string name = pMember->getName();
  if (removeMember(pGuild, qwCharID, false) == false)
    return false;
  pChairMember = pGuild->getMember(rUser.charid());
  if (pChairMember == nullptr)
    return false;

  MsgParams params;
  params.addString(name);
  params.addString(pGuild->getJobName(pChairMember->getJob()));
  params.addString(pChairMember->getName());
  pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_KICK, params);

  pGuild->getEvent().addEvent(EGUILDEVENT_KICK_MEMBER, TVecString{pChairMember->getName(), name});

  if (online)
    thisServer->sendMsg(zoneid, charid, ESYSTEMMSG_ID_GUILD_KICKSELF, MsgParams(pGuild->getJobName(pChairMember->getJob()), pChairMember->getName()));

  XLOG << "[公会管理-踢人]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pChairMember->getJob()
    << "把" << charid << name << "逐出了" << pGuild->getGUID() << pGuild->getName() << "公会" << XEND;

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_GuildKick;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eid,
    0,  //charge
    eType, 0, 1);
  PlatLogManager::getMe().SocialLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eType,
    eid,
    ESocial_GuildKick,
    pGuild->getGUID(),
    qwCharID,
    0,
    0);

  return true;
}

bool GuildManager::changeJob(const SocialUser& rUser, QWORD qwCharID, EGuildJob eJob)
{
  if (rUser.charid() == 0)
    return false;
  if (rUser.charid() == qwCharID)
    return false;
  if (eJob <= EGUILDJOB_MIN || eJob >= EGUILDJOB_MAX || eJob == EGUILDJOB_CHAIRMAN)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;

  GMember* pChairMember = pGuild->getMember(rUser.charid());
  GMember* pMember = pGuild->getMember(qwCharID);
  if (pChairMember == nullptr || pMember == nullptr)
    return false;
  if (eJob == EGUILDJOB_VICE_CHAIRMAN && pMember->getJob() != EGUILDJOB_VICE_CHAIRMAN)
  {
    DWORD dwViceCount = pGuild->getViceCount();
    DWORD dwMaxVice = pGuild->getMaxViceCount();
    if (dwViceCount + 1 > dwMaxVice)
    {
      //ChatManager::sendSysMessage2User(pChairman, ESYSTEMMSG_ID_GUILD_MAXVICE, EMESSAGETYPE_FRAME, SysMsgParams());
      return false;
    }
  }
  if (pGuild->getMisc().hasAuth(pChairMember->getJob(), EAUTH_CHANGE_JOB) == false)
    return false;
  if (pMember->getJob() <= pChairMember->getJob())
    return false;

  EGuildJob eOld = pMember->getJob();
  pMember->setJob(eJob);
  if (eJob == EGUILDJOB_VICE_CHAIRMAN)
    pGuild->sendApplyList(qwCharID);

  bool oldHasAuth = pGuild->getMisc().hasAuth(eOld, EAUTH_AGREE);
  bool newHasAuth = pGuild->getMisc().hasAuth(eJob, EAUTH_AGREE);
  if(oldHasAuth != newHasAuth)
    pMember->setRedTip(newHasAuth);

  MsgParams params;
  params.addString(pMember->getName());
  params.addString(pGuild->getJobName(pChairMember->getJob()));
  params.addString(pChairMember->getName());
  params.addString(pGuild->getJobName(pMember->getJob()));
  pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_CHANGJOB, params);

  pGuild->syncInfoToScene(*pMember);
  if (eJob == EGUILDJOB_VICE_CHAIRMAN)
    pGuild->getEvent().addEvent(EGUILDEVENT_VICE_SET, TVecString{pChairMember->getName(), pMember->getName()});

  XLOG << "[公会管理-职位变更]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pChairMember->getJob()
    << "把" << pMember->getCharID() << pMember->getName() << "职位从" << eOld << "变更为" << eJob << XEND;
  return true;
}

bool GuildManager::exitGuild(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  EGuildJob eOld = pMember->getJob();
  string name = pMember->getName();
  if (removeMember(pGuild, rUser.charid(), true) == false)
    return false;

  pGuild->getEvent().addEvent(EGUILDEVENT_EXIT_MEMBER, TVecString{name});
  pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_LEAVE, MsgParams(name));
  thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_GUILD_SELFLEAVE);

  XLOG << "[公会管理-退出]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << eOld << "退出了" << pGuild->getGUID() << pGuild->getName() << "公会" << XEND;

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_GuildLeave;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eid,
    0,  //charge
    eType, 0, 1);
  PlatLogManager::getMe().SocialLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eType,
    eid,
    ESocial_GuildLeave,
    pGuild->getGUID(),
    0,
    0,
    0);
  return true;
}

bool GuildManager::exchangeChair(const SocialUser& rUser, QWORD qwNewChair)
{
  if (rUser.charid() == 0 || rUser.charid() == qwNewChair)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;

  GMember* pChair = pGuild->getMember(rUser.charid());
  GMember* pNewChair = pGuild->getMember(qwNewChair);
  if (pChair == nullptr || pNewChair == nullptr)
    return false;
  if (pGuild->getMisc().hasAuth(pChair->getJob(), EAUTH_LEADER_GIVE) == false)
    return false;

  if (pGuild->exchangeChairman(pChair->getCharID(), qwNewChair) == false)
    return false;

  pGuild->getEvent().addEvent(EGUILDEVENT_LEADER_CHANGE, TVecString{pChair->getName(), pNewChair->getName()});
  pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_EXCHANGE, MsgParams(pNewChair->getName()));
  XLOG << "[公会管理-会长交接]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pChair->getJob()
    << "会长交接给了" << pNewChair->getCharID() << pNewChair->getName() << XEND;
  return true;
}

bool GuildManager::dismissGuild(const SocialUser& rUser, bool bSet)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  if (bSet && pGuild->getDismissTime() != 0)
    return false;

  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr || pGuild->getMisc().hasAuth(pMember->getJob(), EAUTH_DISMISS_GUILD) == false)
    return false;

  EVENT_TYPE eType = EventType_GuildSetDismiss;
  ESOCIAL_TYPE eSocialType = ESocial_GuildSetDismiss;
  if (bSet)
  {
    pGuild->setDismissTime(xTime::getCurSec() + pGuild->getDismissLastTime());
    eType = EventType_GuildSetDismiss;
    eSocialType = ESocial_GuildSetDismiss;
  }
  else
  {
    pGuild->setDismissTime(0);
    eType = EventType_GuildCancelDismiss;
    eSocialType = ESocial_GuildCancelDismiss;
  }

  //log
  QWORD eid = xTime::getCurUSec();
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eid,
    0,  // charge
    eType, 0, 1);
  PlatLogManager::getMe().SocialLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eType,
    eid,
    eSocialType,
    pGuild->getGUID(),
    rUser.charid(),
    0,
    0);

  XLOG << "[公会管理-设置解散时间] " << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pMember->getJob()
    << (bSet ? "设置" : "取消") << "解散时间 :" << pGuild->getDismissTime();
  return true;
}

bool GuildManager::levelupGuild(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr || pGuild->getMisc().hasAuth(pMember->getJob(), EAUTH_LEVELUP_GUILD) == false)
    return false;
  const SGuildCFG* pNextCFG = GuildConfig::getMe().getGuildCFG(pGuild->getLevel() + 1);
  if (pNextCFG == nullptr)
    return false;
  if (pGuild->getAsset() < pNextCFG->dwLevelupFund)
    return false;
  if (pGuild->getAsset() < pNextCFG->dwNeedFund)
    return false;
  if (pGuild->getPack().reduceItem(pNextCFG->vecLevelupItem, ESOURCE_GUILD_LEVEL_UP) != ESYSTEMMSG_ID_MIN)
    return false;

  if (pGuild->levelup(rUser.charid()) == false)
  {
    XERR << "[公会管理-升级]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "成功升级 :" << pNextCFG->dwLevel << "异常,可返还消耗道具" << XEND;
    return false;
  }

  XLOG << "[公会管理-升级]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pMember->getJob()
    << "升级公会" << pNextCFG->dwLevel - 1 << "->" << pGuild->getLevel() << "消耗了" << pNextCFG->dwLevelupFund << XEND;
  return true;
}

bool GuildManager::exchangeZone(const SocialUser& rUser, DWORD dwZoneID, bool bSet)
{
  if (rUser.charid() == 0)
  {
    XERR << "[公会管理-切线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << (bSet ? "确认" : "取消") << "公会切线失败,charid为0" << XEND;
    return false;
  }

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-切线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << (bSet ? "确认" : "取消") << "公会切线失败,没有公会" << XEND;
    return false;
  }
  GMember* pChairMember = pGuild->getChairman();
  if (pChairMember == nullptr || pChairMember->getCharID() != rUser.charid())
  {
    XERR << "[公会管理-切线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
      << (bSet ? "确认" : "取消") << "公会" << pGuild->getGUID() << pGuild->getName() << "切线到" << dwZoneID << "失败,不是会长" << XEND;
    return false;
  }
  if (pGuild->getDismissTime() != 0)
  {
    XERR << "[公会管理-切线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
      << (bSet ? "确认" : "取消") << "公会" << pGuild->getGUID() << pGuild->getName() << "切线到" << dwZoneID << "失败,公会解散中,不允许切线" << XEND;
    return false;
  }

  // check zone state
  DWORD dwDestZone = getServerZoneID(pGuild->getZoneID(), dwZoneID);
  /*QueryZoneStatusRetSocialCmd cmd;
  if (SocialityManager::getMe().queryZoneStatus(cmd) == false)
  {
    XERR << "[公会管理-切线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << (bSet ? "确认" : "取消") << "公会切线失败,切线状态查询失败" << XEND;
    return false;
  }
  EZoneState eState = EZONESTATE_MIN;
  for (int i = 0; i < cmd.infos_size(); ++i)
  {
    if (dwDestZone == cmd.infos(i).zoneid())
    {
      eState = cmd.infos(i).state();
      break;
    }
  }
  if (eState != EZONESTATE_NOFULL)
  {
    XERR << "[公会管理-切线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << (bSet ? "确认" : "取消") << "公会切线失败,切线状态 :" << eState << "无法切线" << XEND;
    return false;
  }*/

  const SZoneMiscCFG& rCFG = MiscConfig::getMe().getZoneCFG();
  if (bSet)
  {
    TSetDWORD setIDs;
    thisServer->collectZoneIDs(setIDs);
    auto s = setIDs.find(dwDestZone);
    if (s == setIDs.end())
      return false;

    if (pGuild->getPack().reduceItem(rCFG.vecGuildZoneCost, ESOURCE_GUILD_CHANGE_ZONE) != ESYSTEMMSG_ID_MIN)
      return false;

    pGuild->setNextZone(dwDestZone);

    DWORD cur = now();
    DWORD changetime = cur + rCFG.dwGuildZoneTime;

    const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();
    DWORD gvgbegintime = firecfg.getNextStartTime(cur);
    if (changetime > gvgbegintime && changetime < gvgbegintime + firecfg.dwLastTime)
    {
      XLOG << "[公会管理-切线], 切线时间在公会战期间, 延时执行, 公会:" << pGuild->getGUID() <<  pGuild->getName() << "原时间:" << changetime << "延时至:" << gvgbegintime + firecfg.dwLastTime << XEND;
      changetime = gvgbegintime + firecfg.dwLastTime;
    }

    pGuild->setZoneTime(xTime::getCurSec() + rCFG.dwGuildZoneTime);
    string str = LuaManager::getMe().call<char*>("ZoneNumToString", getClientZoneID(pGuild->getNextZone()));
    pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_EXCHANGEZONE_DO, MsgParams(str));
  }
  else
  {
    pGuild->clearNextZone();
    pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_UNDO);
  }

  XLOG << "[公会管理-切线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
    << (bSet ? "确认" : "取消") << "公会" << pGuild->getGUID() << pGuild->getName() << "切线,从" << pGuild->getZoneID() << "切换到" << dwZoneID << "成功, CD :" << rCFG.dwGuildZoneTime << XEND;
  return true;
}

bool GuildManager::memberExchangeZoneAnswer(const SocialUser& rUser, bool bAgree)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;
  if (pMember->getExchangeZone() == false)
    return false;

  if (bAgree)
  {
    GuildExchangeZoneSocialCmd cmd;
    cmd.mutable_user()->CopyFrom(rUser);
    cmd.set_zoneid(pGuild->getNextZone());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToScene(pMember->getZoneID(), pMember->getCharID(), send, len);
  }

  pMember->setExchangeZone(false);
  XLOG << "[公会管理-切线应答]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << (bAgree ? "同意" : "拒绝") << "切线到" << pGuild->getNextZone() << XEND;
  return true;
}

bool GuildManager::queryEventList(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;

  QueryEventListGuildCmd cmd;
  pGuild->getEvent().collectEventList(cmd);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

  XLOG << "[公会管理-事件请求]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "请求了" << cmd.events_size() << "个事件" << XEND;
  return true;
}

bool GuildManager::donate(const SocialUser& rUser, DWORD dwConfigID, DWORD dwTime)
{
  if (rUser.charid() == 0)
  {
    XERR << "[公会管理-贡献捐献]" << rUser.ShortDebugString() << "进行捐献失败,charid=0" << XEND;
    return false;
  }

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-贡献捐献]" << rUser.ShortDebugString() << "进行捐献失败,不是公会成员" << XEND;
    return false;
  }

  const SGuildDonateCFG* pCFG = GuildConfig::getMe().getGuildDonateCFG(dwConfigID);
  if (pCFG == nullptr)
  {
    XERR << "[公会管理-贡献捐献]" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.ShortDebugString() << "进行捐献失败,id :" << dwConfigID << "未在配置表中找到" << XEND;
    return false;
  }

  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
  {
    XERR << "[公会管理-贡献捐献]" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.ShortDebugString() << "进行捐献失败,不是此公会成员" << XEND;
    return false;
  }
  DWORD dwProtectTime = MiscConfig::getMe().getGuildCFG().dwQuestProtectTime;
  if (pMember->getEnterTime() + dwProtectTime > xTime::getCurSec())
  {
#ifdef _DEBUG
    thisServer->sendMsg(pMember->getZoneID(), pMember->getCharID(), 10, MsgParams("这不是一个bug,重熙提的24小时后才能捐赠,但是没给sysid!!!"));
#endif
    XERR << "[公会管理-贡献捐献]" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.ShortDebugString() << "进行捐献失败,进入公会后需要" << dwProtectTime << "后才能捐赠" << XEND;
    return false;
  }

  //pMember->refreshDonate();
  DonateItem* pItem = pMember->getDonateItem(dwConfigID, dwTime);
  if (pMember == nullptr || pItem  == nullptr || pItem->count() != 0)
  {
    XERR << "[公会管理-贡献捐献]" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.ShortDebugString() << "进行捐献失败,不是列表中的物品" << XEND;
    return false;
  }

  GuildDonateSocialCmd cmd;
  cmd.mutable_user()->CopyFrom(rUser);
  cmd.mutable_item()->CopyFrom(*pItem);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToScene(rUser.zoneid(), rUser.charid(), send, len);

  XLOG << "[公会管理-贡献捐献]" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.ShortDebugString() << "进行捐献失败,成功发送至SceneServer处理" << XEND;
  return true;
}

bool GuildManager::donateList(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;

  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  DonateListGuildCmd cmd;
  pMember->collectDonateList(cmd);
  if (cmd.items_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
  }

  XLOG << "[公会管理-捐赠]" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
    << "请求捐赠列表,获得" << cmd.items_size() << "个物品" << XEND;
  return true;
}

bool GuildManager::setDonateFrame(const SocialUser& rUser, bool bDonate)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  pMember->setDonateFrame(bDonate);
  XLOG << "[公会管理-捐赠]" << pGuild->getGUID() << pGuild->getName() << "成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
    << "设置界面状态" << pMember->getDonateFrame() << XEND;
  return true;
}

bool GuildManager::enterTerritory(const SocialUser& rUser, QWORD qwHandID)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  if (qwHandID != 0)
  {
    Guild* pHandGuild = getGuildByUserID(qwHandID);
    if (pGuild != pHandGuild)
    {
      thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_HAND_GUILD_ERROR);
      return false;
    }
  }

  EnterGuildTerritoryGuildSCmd cmd;

  cmd.set_charid(rUser.charid());
  cmd.mutable_info()->set_id(pGuild->getGUID());
  cmd.mutable_info()->set_lv(pGuild->getLevel());

  pGuild->getMisc().refreshQuest(xTime::getCurSec());
  const TListGuildQuestCFG& listQuest = pGuild->getMisc().getQuestList();
  for (auto &l : listQuest)
    l.toData(cmd.mutable_info()->add_quests());

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToScene(pMember->getZoneID(), pMember->getCharID(), send, len);
  XLOG << "[公会管理-领地]" << rUser.ShortDebugString() << "申请进入公会" << pGuild->getGUID() << pGuild->getName() << "领地" << XEND;
  return true;
}

bool GuildManager::pray(const SocialUser& rUser, QWORD qwNpcID, DWORD dwPrayID)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-祈祷] " << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "进行了 pray :" << dwPrayID << "祈祷失败, 没有公会" << XEND;
    return false;
  }
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
  {
    XERR << "[公会管理-祈祷] " << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "进行了 pray :" << dwPrayID << "祈祷失败, 不是公会成员" << XEND;
    return false;
  }

  DWORD dwPrayLv = pMember->getPrayLv(dwPrayID) + 1 ;
  const SGuildPrayCFG* pPrayCFG = GuildConfig::getMe().getGuildPrayCFG(dwPrayID);
  if (pPrayCFG == nullptr)
  {
    XERR << "[公会管理-祈祷] " << rUser.ShortDebugString() << "进行了 pray :" << dwPrayID << "祈祷, praylv :" << dwPrayLv << "未在 Table_Guild_Faith.txt 表中找到" << XEND;
    return false;
  }

  const SGuildCFG* pCFG = pGuild->getGuildCFG();
  if (pCFG == nullptr)
  {
    XERR << "[公会管理-祈祷] " << rUser.ShortDebugString() << "进行了 pray :" << dwPrayID << "祈祷, praylv :" << dwPrayLv << "失败,公会配置非法" << XEND;
    return false;
  }
  if (pCFG == nullptr || (pPrayCFG->eType == EPRAYTYPE_GODDESS && dwPrayLv > pCFG->dwMaxPrayLv))
  {
    XERR << "[公会管理-祈祷] " << rUser.ShortDebugString() << "进行了 pray :" << dwPrayID << "祈祷, praylv :" << dwPrayLv << "超过公会最大等级" << pCFG->dwMaxPrayLv << XEND;
    return false;
  }

  DWORD dwCon = 0;
  DWORD dwMon = 0;
  if (pPrayCFG->eType != EPRAYTYPE_GODDESS)
  {
    const SGuildPrayItemCFG* pItemCFG = pPrayCFG->getItem(dwPrayLv);
    if (pItemCFG == nullptr)
    {
      XERR << "[公会管理-祈祷] " << rUser.ShortDebugString() << "进行了 pray :" << dwPrayID << "祈祷, praylv :" << dwPrayLv << "未在 Table_GuildPray.txt 表中找到" << XEND;
      return false;
    }
  }
  else
  {
    // calc count
    dwCon = LuaManager::getMe().call<DWORD>("calcGuildPrayCon", dwPrayID, dwPrayLv - 1);
    dwMon = LuaManager::getMe().call<DWORD>("calcGuildPrayMon", dwPrayID, dwPrayLv - 1);

    if (pMember->getContribution() < dwCon)
    {
      XERR << "[公会管理-祈祷] " << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pMember->getJob()
        << "进行了 pray :" << dwPrayID << "祈祷, praylv :" << dwPrayLv << "失败,贡献不足" << XEND;
      return false;
    }
  }

  GuildPrayGuildSCmd cmd;
  cmd.mutable_user()->CopyFrom(rUser);
  cmd.set_npcid(qwNpcID);
  cmd.set_prayid(dwPrayID);
  cmd.set_praylv(dwPrayLv);
  cmd.set_needcon(dwCon);
  cmd.set_needmon(dwMon);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToScene(rUser.zoneid(), rUser.charid(), send, len);

  XLOG << "[公会管理-祈祷] " << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pMember->getJob()
    << "进行了 pray :" << dwPrayID << "祈祷, 发送给场景服务器处理, 总贡献 :" << pMember->getContribution() << XEND;
  return true;
}

bool GuildManager::levelupEffect(const SocialUser& rUser)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr || pMember->getLevelupEffect() == false)
    return false;

  pMember->setLevelupEffect(false);
  return true;
}

bool GuildManager::queryPackage(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;

  QueryPackGuildCmd cmd;
  pGuild->getPack().toData(cmd);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
  XLOG << "[公会管理-仓库查询]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "查询了公会仓库,获得了" << cmd.items_size() << "个物品" << XEND;
  return true;
}

bool GuildManager::addMember(Guild* pGuild, const SocialUser& rUser, EGuildJob eJob)
{
  if (pGuild == nullptr)
    return false;

  const SGuildOffline* pOffline = getOfflineData(rUser.charid());
  if (pOffline != nullptr)
  {
    const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
    if (pOffline->dwExitTime + rCFG.dwEnterPunishTime > now())
    {
      XERR << "[公会管理-添加成员]" << rUser.ShortDebugString() << "加入公会" << pGuild->getGUID() << pGuild->getName() << "失败,该玩家在惩罚期" << XEND;
      return false;
    }
  }

  bool bBack = false;
  auto m = m_mapUserGuildOffline.find(rUser.charid());
  if (m != m_mapUserGuildOffline.end())
    bBack = pGuild->getGUID() == m->second.qwGUILDID;

  if (pGuild->addMember(rUser, eJob, bBack) == false)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  m_mapUserID2Guild[rUser.charid()] = pGuild;
  removeApplyGuild(rUser.charid(), pGuild, true);

  obtainOfflineData(pGuild, pMember);

  pGuild->getMisc().getBuilding().onAddMember(pMember);
  pGuild->getMisc().getChallenge().onAddMember(pMember);

  //SocialityManager::getMe().updateSocialData(rUser.charid(), ESOCIALDATA_GUILDNAME, 0, pGuild->getName());
  //SocialityManager::getMe().updateSocialData(rUser.charid(), ESOCIALDATA_GUILDPORTRAIT, 0, pGuild->getPortrait());
  XLOG << "[公会管理-添加成员]" << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "加入了公会" << pGuild->getGUID() << pGuild->getName() << XEND;

  {
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_GuildJoin;
    PlatLogManager::getMe().eventLog(thisServer,
      0,
      rUser.zoneid(),
      rUser.accid(),
      rUser.charid(),
      eid,
      0,  // charge
      eType, 0, 1);

    PlatLogManager::getMe().SocialLog(thisServer,
      0,
      rUser.zoneid(),
      rUser.accid(),
      rUser.charid(),
      eType,
      eid,
      ESocial_GuildJoin,
      pGuild->getGUID(),
      0,
      pGuild->getMemberCount(),
      0);

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_JOIN_GUILD_COUNT, 0, 0, rUser.baselv(), (DWORD)1);
  }

  return true;
}

bool GuildManager::removeMember(Guild* pGuild, QWORD qwCharID, bool bFreeze)
{
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(qwCharID);
  if (pMember == nullptr)
    return false;

  DWORD zoneid = pMember->getZoneID();
  QWORD charid = pMember->getCharID();
  EProfession eProfession = pMember->getProfession();
  string name = pMember->getName();
  DWORD dwContribute = pMember->getContribution();
  DWORD dwTotalContribute = pMember->getTotalContribution();
  DWORD dwGiftPoint = pMember->getGiftPoint();
  string pray;
  string donate;
  string var, building, welfare;
  pMember->toPrayData(pray);
  //pMember->toDonateData(donate);
  pMember->clearVarWhenRemovedFromGuild();
  pMember->toVarData(var);
  pMember->clearBuildingDataWhenExitGuild();
  pMember->toBuildingData(building);

  /*auto needsave = [](GMember* pMember) -> bool {
    if (pMember == nullptr)
      return false;
    if (pMember->getContribution() != 0)
      return true;
    if (pMember->getTotalContribution() != 0)
      return true;
    if (pMember->hasPray() == true)
      return true;

    DonateListGuildCmd cmd;
    pMember->collectDonateList(cmd);
    return cmd.items_size() != 0;
  };*/
  bool bSave = true;//needsave(pMember);

  m_mapUserID2Guild.erase(qwCharID);
  if (pGuild->removeMember(qwCharID) == false)
    return false;

  if (bSave)
  {
    SGuildOffline stOffline;
    stOffline.qwCharID = qwCharID;
    stOffline.qwGUILDID = pGuild->getGUID();
    stOffline.bFreeze = bFreeze;
    stOffline.dwContribute = dwContribute;
    stOffline.dwTotalContribute = dwTotalContribute;
    stOffline.dwGiftPoint = dwGiftPoint;
    stOffline.strPray = pray;
    stOffline.strDonate = donate;
    stOffline.strVar = var;
    stOffline.strBuilding = building;
    stOffline.dwExitTime = now();
    m_mapUserGuildOffline[qwCharID] = stOffline;
    m_setGOfflineUpdate.insert(qwCharID);

    SocialUser oUser;
    oUser.set_zoneid(zoneid);
    oUser.set_charid(charid);
    syncPunishTimeToClient(oUser);
  }

  //SocialityManager::getMe().updateSocialData(qwCharID, ESOCIALDATA_GUILDNAME, 0, "");
  //SocialityManager::getMe().updateSocialData(qwCharID, ESOCIALDATA_GUILDPORTRAIT, 0, "");
  XLOG << "[公会管理-移除成员]" << charid << eProfession << name << "退出了 公会" << pGuild->getGUID() << pGuild->getName() << XEND;
  return true;
}

bool GuildManager::addApplyGuild(QWORD qwCharID, Guild* pGuild)
{
  if (pGuild == nullptr)
    return false;

  m_mapUserApplyGuild[qwCharID].insert(pGuild);
  XDBG << "[公会管理-添加申请管理] charid : " << qwCharID << " 添加公会申请 " << pGuild->getGUID() << ", " << pGuild->getName() << " 总申请 " << static_cast<DWORD>(m_mapUserApplyGuild.size()) << " 个" << XEND;
  return true;
}

bool GuildManager::removeApplyGuild(QWORD qwCharID, Guild* pGuild, bool addMember)
{
  if (pGuild == nullptr)
    return false;

  auto m = m_mapUserApplyGuild.find(qwCharID);
  if (m == m_mapUserApplyGuild.end())
    return false;

  pGuild->removeApply(qwCharID);
  m->second.erase(pGuild);
  if (m->second.empty() == true || addMember == true)
  {
    for(auto &s : m->second)
    {
      if(s != nullptr)
        s->removeApply(qwCharID);
    }
    m_mapUserApplyGuild.erase(m);
  }

  XDBG << "[公会管理-移除申请管理] charid : " << qwCharID << " 移除公会申请 " << pGuild->getGUID() << ", " << pGuild->getName() << " 总申请 " << static_cast<DWORD>(m_mapUserApplyGuild.size()) << " 个" << XEND;
  return true;
}

bool GuildManager::removeApplyGuild(Guild* pGuild)
{
  if (pGuild == nullptr)
    return false;

  for (auto m = m_mapUserApplyGuild.begin(); m != m_mapUserApplyGuild.end();)
  {
    m->second.erase(pGuild);
    if (m->second.empty() == true)
      m = m_mapUserApplyGuild.erase(m);
    else
      ++m;
  }

  return true;
}

bool GuildManager::subContribute(const SocialUser& rUser, DWORD dwCon)
{
  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild != nullptr)
  {
    GMember* pMember = pGuild->getMember(rUser.charid());
    if (pMember == nullptr)
      return false;
    if (pMember->getContribution() < dwCon)
      return false;
    pMember->setContribution(pMember->getContribution() - dwCon);
  }
  else
  {
    auto m = m_mapUserGuildOffline.find(rUser.charid());
    if (m == m_mapUserGuildOffline.end())
      return false;
    if (m->second.dwContribute < dwCon)
    {
      XERR << "[退会玩家扣除贡献]" << rUser.accid() << rUser.charid() << rUser.zoneid() << "贡献" << m->second.dwContribute << "扣除" << dwCon << "贡献不足" << XEND;
      return false;
    }
    DWORD old = m->second.dwContribute;
    m->second.dwContribute -= dwCon;
    m_setGOfflineUpdate.insert(m->first);
    XLOG << "[退会玩家扣除贡献]" << rUser.accid() << rUser.charid() << rUser.zoneid() << "扣除前" << old << "扣除" << dwCon << "扣除后" << m->second.dwContribute << XEND;
  }

  syncPrayToScene(rUser, GUILDOPTCONTYPE_SUB, false);
  return true;
}

bool GuildManager::addContribute(const SocialUser& rUser, DWORD dwCon, DWORD dwSource)
{
  DWORD dwTotalCon = 0;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild != nullptr)
  {
    GMember* pMember = pGuild->getMember(rUser.charid());
    if (pMember == nullptr)
    {
      XERR << "[退会玩家增加贡献]" << rUser.accid() << rUser.charid() << rUser.zoneid() << "增加" << dwCon << "source" << dwSource << "GMember未找到" << XEND;
      return false;
    }

    dwTotalCon = pMember->getContribution() + dwCon;
    pMember->setContribution(dwTotalCon, false, static_cast<ESource>(dwSource) != ESOURCE_ASTROLABE_RESET);
  }
  else
  {
    auto m = m_mapUserGuildOffline.find(rUser.charid());
    if (m == m_mapUserGuildOffline.end()) {
      SGuildOffline stOffline;
      stOffline.qwCharID = rUser.charid();
      stOffline.qwGUILDID = 0;
      stOffline.bFreeze = false;
      stOffline.dwContribute = 0;
      stOffline.dwTotalContribute = 0;
      stOffline.dwGiftPoint = 0;
      stOffline.dwExitTime = 0;
      m_mapUserGuildOffline[rUser.charid()] = stOffline;
      m_setGOfflineUpdate.insert(rUser.charid());
      XLOG << "[公会管理-添加贡献-添加退会数据]" << rUser.accid() << rUser.charid() << rUser.zoneid() << XEND;

      m = m_mapUserGuildOffline.find(rUser.charid());
      if (m == m_mapUserGuildOffline.end())
        return false;
    }
    DWORD old = m->second.dwContribute;
    m->second.dwContribute += dwCon;
    dwTotalCon = m->second.dwContribute;
    m_setGOfflineUpdate.insert(m->first);
    syncPrayToScene(rUser, GUILDOPTCONTYPE_ADD, false);
    XLOG << "[退会玩家增加贡献]" << rUser.accid() << rUser.charid() << rUser.zoneid() << "增加前" << old <<  "增加" << dwCon << "增加后" << dwTotalCon << XEND;
  }

  QWORD eid = xTime::getCurUSec();
  PlatLogManager::getMe().incomeMoneyLog(thisServer,
                                         0,
                                         rUser.zoneid(),
                                         rUser.accid(),
                                         rUser.charid(),
                                         eid,
                                         0,  //charge
                                         EMONEYTYPE_CONTRIBUTE, dwCon, dwTotalCon,
                                         dwSource);

  return true;
}

bool GuildManager::frameStatus(const SocialUser& rUser, bool bOpen)
{
  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  pMember->setFrameStatus(bOpen);
  return true;
}

bool GuildManager::obtainOfflineData(Guild* pGuild, GMember* pMember)
{
  if (pGuild == nullptr || pMember == nullptr)
    return false;

  auto m = m_mapUserGuildOffline.find(pMember->getCharID());
  if (m == m_mapUserGuildOffline.end())
    return false;

  /*const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  bool bDec = pGuild->getGUID() == m->second.qwGUILDID;
  if (bDec) //back to old guild
  {
    pMember->setContribution(m->second.dwContribute, false, false);
    pMember->setTotalContribution(m->second.dwTotalContribute);
    //pMember->fromDonateData(m->second.strDonate);
  }
  else
  {
    pMember->setContribution(static_cast<DWORD>(m->second.dwContribute * rCFG.dwFreezePercent / 10000.0f), false, false);
    pMember->setTotalContribution(0);
  }*/
  pMember->setContribution(m->second.dwContribute, false, false);
  pMember->setTotalContribution(m->second.dwTotalContribute);
  pMember->setGiftPoint(m->second.dwGiftPoint);
  pMember->setLastExitTime(m->second.dwExitTime);
  pMember->fromPrayData(m->second.strPray);
  pMember->fromVarData(m->second.strVar);
  pMember->fromBuildingData(m->second.strBuilding);

  m_setGOfflineUpdate.insert(pMember->getCharID());
  m_mapUserGuildOffline.erase(m);
  XLOG << "[公会管理-贡献携带]" << pMember->getCharID() << pMember->getProfession() << pMember->getName()
    << "加入了 公会" << pGuild->getGUID() << pGuild->getName() << "返回了" << pMember->getContribution() << "贡献" << XEND;

  return true;
}

bool GuildManager::updateOffline(DWORD curTime, bool bFinal /*= false*/)
{
  if (curTime < m_dwOfflineRecordTick)
    return true;
  m_dwOfflineRecordTick = curTime + randBetween(GUILD_RECORD_TICK_TIME / 2, GUILD_RECORD_TICK_TIME);

  if (m_setGOfflineUpdate.empty() == true)
    return true;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guildoffline");
  if (pField == nullptr)
  {
    XERR << "[公会管理-离线保存] 未找到 guildoffline 数据库表" << XEND;
    return false;
  }

  xTime frameTime;
  for (auto s = m_setGOfflineUpdate.begin(); s != m_setGOfflineUpdate.end();)
  {
    QWORD id = *s;
    char szWhere[64] = {0};
    snprintf(szWhere, 64, "charid=%llu", id);

    auto m = m_mapUserGuildOffline.find(id);
    if (m == m_mapUserGuildOffline.end())
    {
      QWORD ret = thisServer->getDBConnPool().exeDelete(pField, szWhere);
      if (ret == QWORD_MAX)
        XERR << "[公会管理-删除离线] 玩家 charid :" << id << "离线数据删除失败" << XEND;
      else
        XLOG << "[公会管理-删除离线] 玩家 charid :" << id << "离线数据删除成功" << XEND;
    }
    else
    {
      xRecord record(pField);
      record.put("zoneid", thisServer->getZoneID());
      record.put("charid", m->second.qwCharID);
      record.put("guildid", m->second.qwGUILDID);
      record.put("freeze", m->second.bFreeze);
      record.put("contribute", m->second.dwContribute);
      record.put("totalcontribute", m->second.dwTotalContribute);
      record.put("giftpoint", m->second.dwGiftPoint);
      record.put("exittime", m->second.dwExitTime);
      record.putBin("pray", (unsigned char *)(m->second.strPray.c_str()), m->second.strPray.size());
      record.putBin("donate", (unsigned char *)(m->second.strDonate.c_str()), m->second.strDonate.size());
      record.putBin("var", (unsigned char *)(m->second.strVar.c_str()), m->second.strVar.size());
      record.putBin("building", (unsigned char *)(m->second.strBuilding.c_str()), m->second.strBuilding.size());
      QWORD ret = thisServer->getDBConnPool().exeReplace(record);
      if (ret == QWORD_MAX)
        XERR << "[公会管理-添加离线] 玩家 charid :" << id << "freeze :" << m->second.bFreeze << "contribute :" << m->second.dwContribute << "添加离线数据失败" << XEND;
      else
        XLOG << "[公会管理-添加离线] 玩家 charid :" << id << "freeze :" << m->second.bFreeze << "contribute :" << m->second.dwContribute << "添加离线数据成功" << XEND;
    }

    if (!bFinal)
    {
      s = m_setGOfflineUpdate.erase(s);
      if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwGuildFrameOvertime)
      {
        XLOG << "[公会管理-离线数据] 帧处理时间 :" << frameTime.uElapse() << "超过 :" << CommonConfig::m_dwGuildFrameOvertime << "跳出循环" << XEND;
        break;
      }
    }
    else
    {
      ++s;
    }
  }

  if (bFinal)
    m_setGOfflineUpdate.clear();
  return true;
}

// 恢复祈祷等级
#include "xLuaTable.h"
#include <sstream>
#include <fstream>
#include <iostream>
using std::stringstream;
using std::ofstream;
using std::fstream;
void GuildManager::fixGuildPrayRestore()
{
  // 屏蔽祈祷恢复 2017-06-06
  return;
  typedef map<QWORD, map<DWORD, DWORD>> TMapPrayRestore;

  fstream file;
  file.open("/data/rogame/log/guildprayitem.txt", std::ios::in);
  if (file)
  {
    XLOG << "[祈祷等级恢复] 已处理" << XEND;
    file.close();
    return;
  }

  XLOG << "[祈祷等级恢复] 开始处理" << XEND;

  // 读取拉取数据
  if (!xLuaTable::getMe().open("role_before.txt"))
  {
    XERR << "[祈祷等级恢复] 加载文件role_before.txt失败" << XEND;
    return;
  }
  if (!xLuaTable::getMe().open("role_after.txt"))
  {
    XERR << "[祈祷等级恢复] 加载文件role_after.txt失败" << XEND;
    return;
  }
  xLuaTableData tableBefore, tableAfter;
  xLuaTable::getMe().getLuaTable("Fix_GuildPrayRestoreBefore", tableBefore);
  xLuaTable::getMe().getLuaTable("Fix_GuildPrayRestoreAfter", tableAfter);

  // format:
  // { charid=4295074834, pray={} }
  // { charid=4295098682, pray={[1]=30, [4]=30, [3]=30, [5]=30} }
  auto loadfunc = [&](xLuaTableData& table, TMapPrayRestore& prays)
    {
      for (auto m = table.begin(); m != table.end(); ++m)
      {
        QWORD charid = m->second.getTableQWORD("charid");
        prays[charid];

        xLuaData& praydata = m->second.getMutableData("pray");
        auto f = [&](const string& key, xLuaData& data)
        {
          prays[charid][atoi(key.c_str())] = data.getInt();
        };
        praydata.foreach(f);
      }
    };
  TMapPrayRestore prayBefore, prayAfter;
  loadfunc(tableBefore, prayBefore);
  loadfunc(tableAfter, prayAfter);

  // 计算返还道具
  auto prayfunc = [&](const TVecMemberPray& prays, const map<DWORD, DWORD>& data, DWORD& con, DWORD& mon)
    {
      con = mon = 0;
      for (auto& pray : prays)
      {
        DWORD maxlv = pray.lv(), minlv = maxlv;
        auto it = data.find(pray.pray());
        if (it == data.end())
          minlv = 0;
        else
          minlv = it->second;
        for (DWORD lv = minlv; lv < maxlv; ++lv)
        {
          con += LuaManager::getMe().call<DWORD>("calcGuildPrayCon", pray.pray(), lv);
          mon += LuaManager::getMe().call<DWORD>("calcGuildPrayMon", pray.pray(), lv);
        }
      }
    };

  stringstream retStr;
  for (auto& it : prayBefore)
  {
    QWORD charid = it.first;

    // 必须前后都有数据才处理
    if (prayAfter.find(charid) == prayAfter.end())
      continue;

    map<DWORD, DWORD>& dataBefore = it.second;
    map<DWORD, DWORD>& dataAfter = prayAfter[charid];

    // 降级了才处理
    if (dataBefore.size() == dataAfter.size())
    {
      bool same = true;
      for (auto& v : dataBefore)
        if (dataAfter.find(v.first) == dataAfter.end() || v.second > dataAfter[v.first])
          same = false;
      if (same) continue;
    }

    Guild* guild = getGuildByUserID(charid);
    DWORD con = 0, mon = 0;

    if (guild != nullptr) // 未退会
    {
      GMember* member = guild->getMember(charid);
      if (member == nullptr)
        continue;

      prayfunc(member->m_vecPrays, dataAfter, con, mon);

      // 还原祈祷数据
      XLOG << "[祈祷等级恢复]" << charid << "公会:" << guild->id << "恢复前数据:";
      for (auto& v : member->m_vecPrays)
        XLOG << "id:" << v.pray() << "lv:" << v.lv();
      XLOG << "恢复后数据:";
      member->m_vecPrays.clear();
      for (auto& v : dataBefore)
      {
        GuildMemberPray p;
        p.set_pray(v.first);
        p.set_lv(v.second);
        member->m_vecPrays.push_back(p);
        XLOG << "id:" << p.pray() << "lv:" << p.lv();
      }
      XLOG << XEND;
    }
    else // 已退会
    {
      auto itOffline = m_mapUserGuildOffline.find(charid);
      if (itOffline == m_mapUserGuildOffline.end())
      {
        SGuildOffline stOffline;
        stOffline.qwCharID = charid;
        stOffline.qwGUILDID = 0;
        stOffline.bFreeze = true;
        stOffline.dwContribute = 0;
        stOffline.dwTotalContribute = 0;
        stOffline.dwGiftPoint = 0;
        stOffline.dwExitTime = 0;
        stOffline.strDonate = "";
        m_mapUserGuildOffline[charid] = stOffline;
        itOffline = m_mapUserGuildOffline.find(charid);
        if (itOffline == m_mapUserGuildOffline.end())
          continue;
      }

      BlobGuildPray blob;
      if (blob.ParseFromString(itOffline->second.strPray) == false)
        continue;

      TVecMemberPray prays;
      prays.clear();
      for (int i = 0; i < blob.prays_size(); ++i)
      {
        GuildMemberPray pray;
        pray.set_pray(blob.prays(i).pray());
        pray.set_lv(blob.prays(i).lv());
        prays.push_back(pray);
      }

      prayfunc(prays, dataAfter, con, mon);

      // 还原祈祷数据
      XLOG << "[祈祷等级恢复]" << charid << "无公会,恢复前数据:";
      for (auto& v : prays)
        XLOG << "id:" << v.pray() << "lv:" << v.lv();
      XLOG << "恢复后数据:";
      BlobGuildPray newPrays;
      for (auto& v : dataBefore)
      {
        GuildMemberPray* p = newPrays.add_prays();
        if (p == nullptr)
          continue;
        p->set_pray(v.first);
        p->set_lv(v.second);
        XLOG << "id:" << p->pray() << "lv:" << p->lv();
      }
      XLOG << XEND;
      string str;
      newPrays.SerializeToString(&str);
      itOffline->second.strPray = str;
      m_setGOfflineUpdate.insert(charid);
    }

    // 返还道具
    if (con != 0 || mon != 0)
    {
      // format: 4297164036 con 400 zeny 312190
      retStr << charid << " con " << con << " zeny " << mon << "\n";
      XLOG << "[祈祷等级恢复] 玩家 charid:" << charid << "返还道具: 贡献:" << con << "zeny:" << mon << XEND;
    }
  }

  ofstream outFile;
  outFile.open("/data/rogame/log/guildprayitem.txt");
  if (!outFile.is_open())
  {
    XERR << "[祈祷等级恢复] 打开文件/data/rogame/log/guildprayitem.txt失败" << XEND;
  }
  else
  {
    outFile << retStr.str();
    outFile.close();
  }

  XLOG << "[祈祷等级恢复] 处理完毕" << XEND;
}

void GuildManager::adjustMemberRedis()
{
  if (m_setAdjustMemberIDs.empty() == true)
    return;

  xTime frameTime;
  for (auto s = m_setAdjustMemberIDs.begin(); s != m_setAdjustMemberIDs.end();)
  {
    stringstream sstr;
    Guild* pGuild = getGuildByUserID(*s);
    if (pGuild == nullptr)
    {
      GCharWriter pGChar(thisServer->getRegionID(), 0);
      pGChar.setGuildID(0);
      pGChar.setGuildName("");
      pGChar.setGuildPortrait("");
      pGChar.save();
      sstr << "id : 0 name : \"\" portrait : \"\"";
    }
    else
    {
      GCharWriter pGChar(thisServer->getRegionID(), *s);
      pGChar.setGuildID(pGuild->getGUID());
      pGChar.setGuildName(pGuild->getName());
      pGChar.setGuildPortrait(pGuild->getPortrait());
      pGChar.save();
      sstr << "id : " << pGuild->getGUID() << " name : " << pGuild->getName() << " portrait : " << pGuild->getPortrait();
    }

    s = m_setAdjustMemberIDs.erase(s);
    XLOG << "[公会管理-修正]" << *s << "修正公会信息为" << sstr.str() << XEND;

    if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwGuildFrameOvertime)
    {
      XLOG << "[公会管理-修正] 耗时:" << frameTime.uElapse() << "超过 :" << CommonConfig::m_dwGuildFrameOvertime << "跳出循环" << XEND;
      break;
    }
  }
}

const SGuildOffline* GuildManager::getOfflineData(QWORD charid) const
{
  auto m = m_mapUserGuildOffline.find(charid);
  if (m != m_mapUserGuildOffline.end())
    return &m->second;
  return nullptr;
}

void GuildManager::patch()
{
  for (auto &it : xEntryID::ets_)
  {
    Guild* pGuild = (Guild *)(it.second);
    if(pGuild == nullptr)
      continue;
    auto& gvg = pGuild->getMisc().getGvg();
    gvg.checkVersion();
  }
}

void GuildManager::patch_1()
{
  for (auto &it : xEntryID::ets_)
  {
    Guild* pGuild = (Guild *)(it.second);
    if(pGuild == nullptr)
      continue;

    TVecGuildMember& vecMember = pGuild->getAllMemberList();
    for (auto &v : vecMember)
    {
      v->cleatDonateItem();
      v->resetDonateTime(0);
    }
  }

  XLOG << "[公会捐赠时间刷新] : 强制刷新" << XEND;
}

/*void GuildManager::patch_2()
{
  for (auto &it : xEntryID::ets_)
  {
    Guild* pGuild = (Guild *)(it.second);
    if(pGuild == nullptr)
      continue;

    const TVecGuildMember& memlist = pGuild->getMemberList();
    for (auto &m : memlist)
      m->patch_checkLvOverflow();
  }
  XLOG << "[公会祈祷等级溢出补丁], 刷新成功" << XEND;
}*/

bool GuildManager::applyGuildReward(const SocialUser& rUser, DWORD id)
{
  if (rUser.charid() == 0)
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;

  const SGuildDonateCFG* pCFG = GuildConfig::getMe().getGuildDonateCFG(id);
  if(pCFG == nullptr)
    return false;

  ApplyRewardConGuildCmd cmd;
  cmd.set_configid(id);
  for (auto &v : pCFG->vecUserReward)
  {
    GuildReward reward;
    reward.set_id(v.id());
    reward.set_num(v.count());
    cmd.add_con()->CopyFrom(reward);
  }
  for (auto &v : pCFG->vecGuildReward)
  {
    GuildReward reward;
    reward.set_id(v.id());
    reward.set_num(v.count());
    cmd.add_asset()->CopyFrom(reward);
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

  XLOG << "[公会管理-奖励请求]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "请求了" << id << "个事件" << XEND;
  return true;
}

bool GuildManager::modifyAuth(const SocialUser& rUser, const ModifyAuthGuildCmd & cmd)
{
  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-权限设置]" << rUser.ShortDebugString() << "执行操作" << cmd.ShortDebugString() << "失败,不是公会成员" << XEND;
    return false;
  }

  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
  {
    XERR << "[公会管理-权限设置]" << rUser.ShortDebugString() << "执行操作" << cmd.ShortDebugString() << "失败,不是公会成员" << XEND;
    return false;
  }
  if (pMember->getJob() > cmd.job())
  {
    XERR << "[公会管理-权限设置]" << rUser.ShortDebugString() << "执行操作" << cmd.ShortDebugString() << "失败,不能编辑比自己大的职位" << XEND;
    return false;
  }
  if (cmd.modify() == EMODIFY_AUTH)
  {
    if (pGuild->getMisc().hasEditAuth(pMember->getJob(), cmd.auth()) == false)
    {
      XERR << "[公会管理-权限设置]" << rUser.ShortDebugString() << "执行操作" << cmd.ShortDebugString() << "失败,没有编辑" << cmd.auth() << "权限" << XEND;
      return false;
    }
  }
  else if (cmd.modify() == EMODIFY_EDITAUTH)
  {
    const SGuildJobCFG* pJobCFG = GuildConfig::getMe().getGuildJobCFG(cmd.job());
    if (pJobCFG == nullptr || GGuild::hasAuth(pJobCFG->dwDefaultEditAuth, cmd.auth()) == false)
    {
      XERR << "[公会管理-权限设置]" << rUser.ShortDebugString() << "执行操作" << cmd.ShortDebugString() << "失败,没有编辑" << cmd.auth() << "权限" << XEND;
      return false;
    }
  }

  if (pGuild->getMisc().modifyAuth(cmd.add(), cmd.modify(), cmd.job(), cmd.auth()) == false)
  {
    XERR << "[公会管理-权限设置]" << rUser.ShortDebugString() << "执行操作" << cmd.ShortDebugString() << "失败,编辑失败" << XEND;
    return false;
  }

  if (cmd.add())
  {
    if (cmd.modify() == EMODIFY_AUTH)
      pGuild->getEvent().addEvent(EGUILDEVENT_AUTH_ADD, TVecString{pMember->getName(), pGuild->getMisc().getJobName(cmd.job()), MiscConfig::getMe().getGuildCFG().getAuthName(cmd.auth())});
    else if (cmd.modify() == EMODIFY_EDITAUTH)
      pGuild->getEvent().addEvent(EGUILDEVENT_EDITAUTH_ADD, TVecString{pMember->getName(), pGuild->getMisc().getJobName(cmd.job()), MiscConfig::getMe().getGuildCFG().getAuthName(cmd.auth())});
  }
  else
  {
    if (cmd.modify() == EMODIFY_AUTH)
      pGuild->getEvent().addEvent(EGUILDEVENT_AUTH_REMOVE, TVecString{pMember->getName(), pGuild->getMisc().getJobName(cmd.job()), MiscConfig::getMe().getGuildCFG().getAuthName(cmd.auth())});
    else if (cmd.modify() == EMODIFY_EDITAUTH)
      pGuild->getEvent().addEvent(EGUILDEVENT_EDITAUTH_REMOVE, TVecString{pMember->getName(), pGuild->getMisc().getJobName(cmd.job()), MiscConfig::getMe().getGuildCFG().getAuthName(cmd.auth())});
  }

  if (cmd.auth() == EAUTH_UPLOAD_PHOTO)
    pGuild->getPhoto().refreshPhoto();

  XLOG << "[公会管理-权限设置]" << rUser.ShortDebugString() << "执行操作" << cmd.ShortDebugString() << "成功" << XEND;
  return true;
}

bool GuildManager::checkRename(const SocialUser& rUser, const string& name)
{
  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (!pGuild)
  {
    XERR << "[公会-改名] 失败，无法获取公会信息" << rUser.ShortDebugString() << XEND;
    return false;
  }

  GMember* pChairMember = pGuild->getMember(rUser.charid());
  if(!pChairMember)
  {
    XERR << "[公会-改名] 失败，无法获取成员信息" << rUser.ShortDebugString() << XEND;
    return false;
  }

  if(EGUILDJOB_CHAIRMAN != pChairMember->getJob())
  {
    XERR << "[公会-改名] 失败，权限不足（客户端检测的，有作弊嫌疑）" << rUser.ShortDebugString() << pChairMember->getJob() << XEND;
    return false;
  }

  ERenameErrCode code = ERENAME_SUCCESS;
  do
  {
    const SGuildMiscCFG& rCfg = MiscConfig::getMe().getGuildCFG();
    if(pGuild->getRenameTime() + rCfg.dwRenameCoolDown > xTime::getCurSec())
    {
      code = ERENAME_CD;
      break;
    }

    ESysMsgID eId = addName(name);
    if(ESYSTEMMSG_ID_GUILD_NAMEDUPLICATE == eId)
    {
      code = ERENAME_CONFLICT;
      break;
    }
    else if(ESYSTEMMSG_ID_MIN != eId)
    {
      XERR << "[公会-改名] 失败，名字不合法（客户端检测的，有作弊嫌疑）" << rUser.ShortDebugString() << name << XEND;
      return false;
    }
    else if (pGuild->getMisc().getGvg().inSuperGvg())/*决战期间不能修改*/
    {
      thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 25518);
      code = ERENAME_CD;
      break;
    }
  }while(0);

  if(ERENAME_SUCCESS != code)
  {
    RenameQueryGuildCmd cmd;
    cmd.set_name(name);
    cmd.set_code(code);

    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
    return false;
  }

  RenameNTFGuildCmd cmd;
  cmd.mutable_user()->CopyFrom(rUser);
  cmd.set_newname(name);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToScene(rUser.zoneid(), rUser.charid(), send, len);
  return true;
}

bool GuildManager::rename(const SocialUser& rUser, const string& name)
{
  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (!pGuild)
  {
    // 可能出现的情况，改名过程中公会被解散或自己被提出公会，概率极小
    XERR << "[公会-改名] 失败，无法获取公会信息(严重：导致改名道具消耗，改名失败)" << rUser.ShortDebugString() << XEND;
    return false;
  }

  string oldname = pGuild->getName();
  removeName(oldname);
  pGuild->rename(name);

  m_mapName2Guild.erase(oldname);
  m_mapName2Guild[pGuild->getName()] = pGuild;

  RenameQueryGuildCmd cmd;
  cmd.set_name(name);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

  // add event
  pGuild->getEvent().addEvent(EGUILDEVENT_RENAME, TVecString{rUser.name(), name});
  // inform scene
  pGuild->broadcastGuildInfoToScene();
  // broadcast
  thisServer->sendWorldMsg(rUser.zoneid(), ESYSTEMMSG_ID_GUILD_RENAME, MsgParams(rUser.name(), oldname, name));

  // update gchar info
  GCharWriter pGChar(thisServer->getRegionID(), rUser.charid());
  pGChar.setGuildName(pGuild->getName());
  pGChar.save();

  // update member gchar
  for (auto &v : pGuild->getAllMemberList())
    if (v && v->getCharID() != rUser.charid())
      addAdjustCharID(v->getCharID());

  XLOG << "[公会-改名]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << pGuild->getGUID() << "修改公会名" << oldname << "为" << name << "成功" << XEND;

  PlatLogManager::getMe().changeFlagLog(thisServer,
      0,
      rUser.zoneid(),
      rUser.charid(),
      EChangeFlag_Guild_Name, oldname, name);

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_GuildRename;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eid,
    0,  // charge
    eType, 0, 1);

  PlatLogManager::getMe().SocialLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eType,
    eid,
    ESocial_GuildRename,
    pGuild->getGUID(),
    0,
    0,
    0);

  //StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_CREATE_GUILD_COUNT, 0, 0, user.baselv(), (DWORD)1);
  return true;
}

bool GuildManager::cityAction(const SocialUser& rUser, ECityAction eAction)
{
  if (eAction <= ECITYACTION_MIN || eAction >= ECITYACTION_MAX || ECityAction_IsValid(eAction) == false)
  {
    XERR << "[公会管理-城池放弃]" << rUser.ShortDebugString() << "执行" << eAction << "城池失败, 未知action" << XEND;
    return false;
  }
  string action;
  if (eAction == ECITYACTION_GIVEUP)
    action = "放弃";
  else if (eAction == ECITYACTION_CANCEL_GIVEUP)
    action = "取消放弃";
  else
    return false;

  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-城池放弃]" << rUser.ShortDebugString() << action << "城池失败, 未发现公会" << XEND;
    return false;
  }

  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
  {
    XERR << "[公会管理-城池放弃]" << rUser.ShortDebugString() << action << pGuild->getGUID() << pGuild->getName() << "城池失败, 不是公会成员" << XEND;
    return false;
  }
  GuildMisc& rMisc = pGuild->getMisc();
  if (rMisc.hasAuth(pMember->getJob(), EAUTH_GIVEUP_CITY) == false)
  {
    XERR << "[公会管理-城池放弃]" << rUser.ShortDebugString() << action << pGuild->getGUID() << pGuild->getName() << "城池失败, 没有权限" << XEND;
    return false;
  }

  if (rMisc.getCityID() == 0)
  {
    XERR << "[公会管理-城池放弃]" << rUser.ShortDebugString() << action << pGuild->getGUID() << pGuild->getName() << "城池失败, 没有占领城池" << XEND;
    return false;
  }

  if (eAction == ECITYACTION_GIVEUP)
  {
    if (rMisc.getCityGiveupTime() != 0)
    {
      XERR << "[公会管理-城池放弃]" << rUser.ShortDebugString() << action << pGuild->getGUID() << pGuild->getName() << "城池失败, 领地已处于放弃状态" << XEND;
      return false;
    }

    rMisc.setCityGiveupTime(xTime::getCurSec() + MiscConfig::getMe().getGuildCFG().dwCityGiveupCD);
    pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_GIVEUPCITY, MsgParams(rMisc.getJob(pMember->getJob()).name(), pMember->getName()));
  }
  else if (eAction == ECITYACTION_CANCEL_GIVEUP)
  {
    if (rMisc.getCityGiveupTime() == 0)
    {
      XERR << "[公会管理-城池放弃]" << rUser.ShortDebugString() << action << pGuild->getGUID() << pGuild->getName() << "城池失败, 领地未处于放弃状态" << XEND;
      return false;
    }
    rMisc.setCityGiveupTime(0);
  }
  else
  {
    return false;
  }

  pMember->setFrameStatus(true);
  pMember->setFrameStatus(false);

  XLOG << "[公会管理-城池放弃]" << rUser.ShortDebugString() << action << pGuild->getGUID() << pGuild->getName() << "城池成功, 领地放弃时间" << rMisc.getCityGiveupTime() << XEND;
  return true;
}

bool GuildManager::openFunction(const SocialUser& rUser, EGuildFunction func)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return false;
  if (func <= EGUILDFUNCTION_MIN || func >= EGUILDFUNCTION_MAX)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  switch (func)
  {
  case EGUILDFUNCTION_BUILDING:
  {
    if (pGuild->getMisc().hasAuth(pMember->getJob(), EAUTH_OPEN_BUILDING) == false)
    {
      XERR << "[公会管理-开启功能]" << pGuild->getGUID() << pGuild->getName() << "玩家:" << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "功能:" << func
           << "权限不够" << XEND;
      return false;
    }
    if (pGuild->getLevel() < MiscConfig::getMe().getGuildBuildingCFG().dwOpenGuildLevel)
    {
      XERR << "[公会管理-开启功能]" << pGuild->getGUID() << pGuild->getName() << "玩家:" << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "功能:" << func
           << "公会等级:" << pGuild->getLevel() << "需要等级:" << MiscConfig::getMe().getGuildBuildingCFG().dwOpenGuildLevel << "等级不够" << XEND;
      return false;
    }

    DWORD itemid = MiscConfig::getMe().getGuildBuildingCFG().pairOpenCost.first;
    DWORD itemcount = MiscConfig::getMe().getGuildBuildingCFG().pairOpenCost.second;
    if (itemid)
    {
      if (pGuild->getPack().reduceItem(itemid, itemcount, ESOURCE_GUILD_OPEN_BUILDING) != ESYSTEMMSG_ID_MIN)
      {
        XERR << "[公会管理-开启功能]" << pGuild->getGUID() << pGuild->getName() << "玩家:" << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "功能:" << func
             << "道具:" << itemid << itemcount << "不够" << XEND;
        const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(itemid);
        if (cfg)
          thisServer->sendMsg(pMember->getZoneID(), pMember->getCharID(), 3711, MsgParams{cfg->strNameZh});
        return false;
      }
    }

    pGuild->getMisc().getBuilding().openBuildingFunction();

    // 建筑开启后同时开启挑战功能
    pGuild->getMisc().getChallenge().open();
    break;
  }
  default:
    return false;
  }

  XLOG << "[公会管理-开启功能]" << pGuild->getGUID() << pGuild->getName() << "玩家:" << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "功能:" << func << "开启成功" << XEND;
  return true;
}

bool GuildManager::build(const SocialUser& rUser, EGuildBuilding building)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return false;
  if (building <= EGUILDBUILDING_MIN || building >= EGUILDBUILDING_MAX)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  if (pGuild->getMisc().isFunctionOpen(EGUILDFUNCTION_BUILDING) == false)
    return false;

  if (pGuild->getMisc().hasAuth(pMember->getJob(), EAUTH_BUILD) == false)
    return false;

  if (pGuild->getMisc().getBuilding().build(pMember, building))
    XLOG << "[公会管理-建造]" << pGuild->getGUID() << pGuild->getName() << "建筑:" << building << "开始建造" << XEND;

  return true;
}

bool GuildManager::submitMaterial(const SocialUser& rUser, const SubmitMaterialGuildCmd& cmd)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  if (cmd.materialid() == 0)
    return false;

  if (pGuild->getMisc().isFunctionOpen(EGUILDFUNCTION_BUILDING) == false)
    return false;

  DWORD totalcount = 1;
  TMapMaterial material;
  // for (int i = 0; i < cmd.materials_size(); ++i)
  // {
  //   auto it = material.find(cmd.materials(i).id());
  //   if (it == material.end())
  //     material[cmd.materials(i).id()] = cmd.materials(i).count();
  //   else
  //     material[cmd.materials(i).id()] += cmd.materials(i).count();
  // }
  material[cmd.materialid()] = totalcount;

  if (pMember->canSubmitMaterial(cmd.building(), totalcount) == false)
    return false;
  if (pGuild->getMisc().getBuilding().isSubmitLock())
    return false;

  if (pGuild->getMisc().getBuilding().canSubmitMaterial(cmd.building(), material) == false)
    return false;

  SubmitMaterialGuildSCmd scmd;
  scmd.set_charid(rUser.charid());
  scmd.set_building(cmd.building());
  scmd.set_submitcount(pMember->getSubmitCount(cmd.building()));
  scmd.set_counter(pGuild->getMisc().getBuilding().lockSubmit());
  scmd.set_curlevel(pGuild->getMisc().getBuilding().getBuildingLevel(cmd.building()));
  for (auto& v : material)
  {
    BuildingMaterial* p = scmd.add_materials();
    if (p)
    {
      p->set_id(v.first);
      p->set_count(v.second);
    }
  }
  PROTOBUF(scmd, send, len);
  thisServer->sendCmdToScene(rUser.zoneid(), rUser.charid(), send, len);

  XLOG << "[公会管理-提交建筑材料]" << pGuild->getGUID() << pGuild->getName() << rUser.ShortDebugString() << "材料:" << cmd.ShortDebugString() << "发送到scene处理" << XEND;
  return true;
}

void GuildManager::querySubmitCount(const SocialUser& rUser, EGuildBuilding type)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return;

  pMember->sendSubmitCountToMe(type);
}

void GuildManager::getWelfare(const SocialUser& rUser)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return;

  if (pGuild->getMisc().getWelfare().getWelfare(pMember) == false)
    return;

  WelfareNtfGuildCmd cmd;
  cmd.set_welfare(false);
  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);

  XLOG << "[公会管理-领取福利]" << pGuild->getGUID() << pGuild->getName() << rUser.ShortDebugString() << "成功" << XEND;
}

void GuildManager::buildingLevelUpEffect(const SocialUser& rUser, const BuildingLvupEffGuildCmd& cmd)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return;

  set<EGuildBuilding> types;
  for (int i = 0; i < cmd.effects_size(); ++i)
    types.insert(cmd.effects(i).type());
  pMember->buildingLevelUpEffect(types, false);
}

void GuildManager::produceArtifact(const SocialUser& rUser, const ArtifactProduceGuildCmd& cmd)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return;

  if (pGuild->getMisc().getArtifact().produce(pMember, cmd.id()) == false)
    return;

  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);
  XLOG << "[公会管理-打折神器]" << pGuild->getGUID() << pGuild->getName() << rUser.ShortDebugString() << "itemid:" << cmd.id() << "打造成功" << XEND;
}

void GuildManager::optArtifact(const SocialUser& rUser, const ArtifactOptGuildCmd& cmd)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
    return;
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
    return;

  if (cmd.guid_size() <= 0)
    return;

  set<string> guids;
  for (int i = 0; i < cmd.guid_size(); ++i)
    guids.insert(cmd.guid(i));

  if (pGuild->getMisc().getArtifact().operate(pMember, cmd.opt(), guids, cmd.charid()) == false)
    return;

  XLOG << "[公会管理-操作神器]" << pGuild->getGUID() << pGuild->getName() << rUser.ShortDebugString() << "cmd:" << cmd.ShortDebugString() << "操作成功" << XEND;
}

bool GuildManager::queryGQuest(const SocialUser& rUser)
{
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-任务查询]" << rUser.ShortDebugString() << "查询公会任务失败,未找到公会" << XEND;
    return false;
  }
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
  {
    XERR << "[公会管理-任务查询]" << rUser.ShortDebugString() << "查询公会" << pGuild->getGUID() << pGuild->getName() << "任务失败,不是成员" << XEND;
    return false;
  }
  if (pGuild->getMisc().hasAuth(pMember->getJob(), EAUTH_ARTIFACT_QUEST) == false)
  {
    XERR << "[公会管理-任务查询]" << rUser.ShortDebugString() << "查询公会" << pGuild->getGUID() << pGuild->getName() << "任务失败,没有权限" << XEND;
    return false;
  }

  QueryGQuestGuildCmd cmd;
  pGuild->getMisc().getQuest().toData(cmd);
  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);
  XLOG << "[公会管理-任务查询]" << rUser.ShortDebugString() << "查询公会" << pGuild->getGUID() << pGuild->getName() << "任务成功,内容" << cmd.ShortDebugString() << XEND;
  return true;
}

bool GuildManager::processOfflineGM()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_gm");
  if (pField == nullptr)
  {
    XERR << "[公会GM-离线执行] 执行失败, 未找到 guild_gm 数据库表" << XEND;
    return false;
  }

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, nullptr, nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[公会GM-离线执行] 执行失败, 查询失败 ret :" << ret << XEND;
    return false;
  }

  for (DWORD d = 0; d < set.size(); ++d)
  {
    QWORD qwGuildID = set[d].get<QWORD>("guildid");
    QWORD qwCharID = set[d].get<QWORD>("charid");

    string data;
    data.assign((const char *)set[d].getBin("data"), set[d].getBinSize("data"));

    GMCommandGuildSCmd cmd;
    if (cmd.ParseFromString(data) == false)
    {
      XERR << "[公会GM-离线执行] 执行" << qwGuildID << qwCharID << "失败, 反序列化失败" << XEND;
      continue;
    }

    bool bResult = GMCommandMgr::getMe().exec(cmd);
    XLOG << "[公会GM-离线执行] 执行" << cmd.ShortDebugString() << "成功, 结果" << (bResult ? "成功" : "失败") << XEND;
  }

  ret = thisServer->getDBConnPool().exeDelete(pField, nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[公会GM-离线执行] 执行完毕,清空数据库表失败 ret :" << ret << XEND;
    return false;
  }

  XLOG << "[公会GM-离线执行] 执行完毕,清空数据库表成功 ret :" << ret << XEND;
  return true;
}

bool GuildManager::queryTreasureResult(const SocialUser& rUser, const QueryTreasureResultGuildCmd& cmd)
{
  Guild* pGuild = getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 4044);
    return false;
  }
  pGuild->getMisc().getTreasure().queryTreasureResult(rUser.charid(), cmd.eventguid());
  return true;
}

QWORD GuildManager::getCityOwner(DWORD dwZoneID, DWORD dwCityID)
{
  auto it = m_mapCityGuild.find(dwZoneID);
  if (it == m_mapCityGuild.end())
    return 0;
  auto m = it->second.find(dwCityID);
  return m != it->second.end() ? m->second : 0;
}

void GuildManager::setCityOwner(DWORD dwZoneID, DWORD dwCityID, QWORD qwGuildID)
{
  auto it = m_mapCityGuild.find(dwZoneID);
  if (it == m_mapCityGuild.end())
  {
    m_mapCityGuild[dwZoneID];
    it = m_mapCityGuild.find(dwZoneID);
    if (it == m_mapCityGuild.end())
      return;
  }

  it->second[dwCityID] = qwGuildID;
}

bool GuildManager::queryBuildingSubmitRank(const SocialUser& rUser, EGuildBuilding type)
{
  if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX)
    return false;
  if (rUser.charid() == 0 || isMember(rUser.charid()) == false)
    return false;

  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-查询建筑排行榜]" << rUser.ShortDebugString() << "查询公会任务失败,未找到公会" << XEND;
    return false;
  }
  GMember* pMember = pGuild->getMember(rUser.charid());
  if (pMember == nullptr)
  {
    XERR << "[公会管理-查询建筑排行榜]" << rUser.ShortDebugString() << "查询公会" << pGuild->getGUID() << pGuild->getName() << "任务失败,不是成员" << XEND;
    return false;
  }
  pGuild->getMisc().getBuilding().querySubmitRank(type, pMember);
  return true;
}

bool GuildManager::openRealtimeVoice(const SocialUser& rUser, QWORD memberid, bool open)
{
  Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
  if (pGuild == nullptr)
  {
    XERR << "[公会管理-实时语音封禁]" << rUser.ShortDebugString() << "未找到公会" << XEND;
    return false;
  }
  return pGuild->openRealtimeVoice(rUser.charid(), memberid, open);
}

