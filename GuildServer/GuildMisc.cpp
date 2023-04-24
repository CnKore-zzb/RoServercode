#include "GuildMisc.h"
#include "Guild.h"
#include "LuaManager.h"
#include "GuildServer.h"
#include "MiscConfig.h"
#include "GGuild.h"

GuildMisc::GuildMisc(Guild *pGuild) : m_pGuild(pGuild), m_oBuilding(pGuild), m_oWelfare(pGuild), m_oChallenge(pGuild), m_oQuest(pGuild), m_oArtifact(pGuild), m_oGvg(pGuild), m_oTreasure(pGuild), m_oShop(pGuild)
{
  for (int i = EGUILDJOB_MIN + 1; i < EGUILDJOB_MAX; ++i)
    m_arrJob[i].set_job(static_cast<EGuildJob>(i));
  for (int i = EVARTYPE_MIN + 1; i < EVARTYPE_MAX; ++i)
    m_arrVarValue[i] = 0;

  m_mapVarGuild[EVARTYPE_GUILD_MAXASSET] = EGUILDDATA_ASSET_DAY;
  m_mapVarGuild[EVARTYPE_GUILD_TREASURE_COUNT] = EGUILDDATA_TREASURE_GUILD_COUNT;
  m_mapVarGuild[EVARTYPE_BCOIN_TREASURE_COUNT] = EGUILDDATA_TREASURE_BCOIN_COUNT;
}

GuildMisc::~GuildMisc()
{
}

void GuildMisc::init()
{
  switch (m_dwInitStatus)
  {
    case GUILD_BLOB_INIT_NULL:
      {
        XDBG << "[公会-misc]" << m_pGuild->getName() << "初始化失败，没有数据" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_COPY_DATA:
      {
        fromMiscString(m_oBlobMisc);
        m_dwInitStatus = GUILD_BLOB_INIT_OK;
        XDBG << "[公会-misc]" << m_pGuild->getName() << "初始化成功" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_OK:
      break;
    default:
      break;
  }
}

void GuildMisc::setBlobMiscString(const char* str, DWORD len)
{
  if (GUILD_BLOB_INIT_OK == m_dwInitStatus)
  {
    XERR << "[公会-misc]" << "初始化异常" << XEND;
    return;
  }
  m_dwInitStatus = GUILD_BLOB_INIT_COPY_DATA;
  m_oBlobMisc.assign(str, len);
  XDBG << "[公会-misc]" << m_pGuild->getName() << "设置blob string" << XEND;
}

bool GuildMisc::toMiscString(string& str)
{
  if (GUILD_BLOB_INIT_COPY_DATA == m_dwInitStatus)
  {
    str.assign(m_oBlobMisc.c_str(), m_oBlobMisc.size());
    return true;
  }

  if (GUILD_BLOB_INIT_OK != m_dwInitStatus) return true;

  BlobGuildMisc oBlob;

  // var
  string var;
  BlobVar oVar;
  m_oVar.save(&oVar);
  oVar.SerializeToString(&var);
  oBlob.set_var(var.c_str(), var.size());

  // jobname
  for (int i = EGUILDJOB_MIN + 1; i < EGUILDJOB_MAX; ++i)
  {
    GuildJob* pJob = oBlob.add_job();
    if (pJob == nullptr)
      continue;

    m_arrJob[i].set_job(static_cast<EGuildJob>(i));
    pJob->CopyFrom(m_arrJob[i]);
  }
  oBlob.set_auth_version(m_dwAuthVersion);

  // donate
  oBlob.set_donatetime1(m_dwDonateTime1);
  oBlob.set_donatetime2(m_dwDonateTime2);
  oBlob.set_donatetime3(m_dwDonateTime3);
  oBlob.set_donatetime4(m_dwDonateTime4);

  // quest
  for (auto &s : m_listGuildQuest)
  {
    GuildQuest* pQuest = oBlob.add_quest();
    pQuest->set_questid(s.dwQuestID);
    pQuest->set_time(s.dwTime);
  }
  oBlob.set_nextquesttime(m_dwNextQuestTime);
  oBlob.set_renametime(m_dwRenameTime);

  for (auto &m: m_mapDojoMsg)
  {
    DojoMsgBlob* pDojoMsg = oBlob.add_dojomsg();
    if (pDojoMsg)
    {
      pDojoMsg->set_dojoid(m.first);

      for (auto&v : m.second)
      {
        DojoMsg* pMsg = pDojoMsg->add_msgs();
        if (pMsg)
        {
          pMsg->CopyFrom(v);
        }
      }
    }
  }

  // city
  oBlob.set_city_giveup_time(m_dwGiveupTime);

  // building
  m_oBuilding.toData(oBlob.mutable_building());

  // open function
  oBlob.set_openfunction(m_qwOpenFunction);

  // welfare
  m_oWelfare.toData(oBlob.mutable_welfare());

  // challenge
  m_oChallenge.toData(oBlob.mutable_challenge());

  // quest
  m_oQuest.toData(oBlob.mutable_quests());

  // artifact
  m_oArtifact.toData(oBlob.mutable_artifact());

  //gvg
  m_oGvg.toData(oBlob.mutable_gvg());

  // treasure
  m_oTreasure.toData(oBlob.mutable_treasures());

  return oBlob.SerializeToString(&str);
}

bool GuildMisc::fromMiscString(const string& str)
{
  BlobGuildMisc oBlob;
  if (oBlob.ParseFromString(str) == false)
    return false;

  // var
  BlobVar oVar;
  oVar.ParseFromString(oBlob.var());
  m_oVar.load(oVar);
  for (int i = 0; i < oVar.datas_size(); ++i)
  {
    const Var& rVar = oVar.datas(i);
    setVarTemp(rVar.type(), m_oVar.getVarValue(rVar.type()));
  }

  // jobname
  for (int i = 0; i < oBlob.job_size(); ++i)
  {
    const GuildJob& rJob = oBlob.job(i);
    m_arrJob[rJob.job()].CopyFrom(rJob);

    const SGuildJobCFG* pJob = GuildConfig::getMe().getGuildJobCFG(rJob.job());
    if (pJob != nullptr && m_arrJob[rJob.job()].auth() == 0)
      m_arrJob[rJob.job()].set_auth(pJob->dwDefaultAuth);
    if (pJob != nullptr && m_arrJob[rJob.job()].editauth() == 0)
      m_arrJob[rJob.job()].set_editauth(pJob->dwDefaultEditAuth);
    if (pJob != nullptr && m_arrJob[rJob.job()].name().empty() == true)
      m_arrJob[rJob.job()].set_name(pJob->name);
  }
  m_dwAuthVersion = oBlob.auth_version();

  // donate
  m_dwDonateTime1 = oBlob.donatetime1();
  m_dwDonateTime2 = oBlob.donatetime2();
  m_dwDonateTime3 = oBlob.donatetime3();
  m_dwDonateTime4 = oBlob.donatetime4();

  // quest
  m_listGuildQuest.clear();
  for (int i = 0; i < oBlob.quest_size(); ++i)
  {
    const GuildQuest& rQuest = oBlob.quest(i);
    const SGuildQuestCFG* pCFG = GuildConfig::getMe().getGuildQuestCFG(rQuest.questid());
    if (pCFG == nullptr)
    {
      XERR << "[公会-其他加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载任务" << rQuest.questid() << "失败, 未在Table_GuildQuest.txt表中找到" << XEND;
      continue;
    }
    SGuildQuestCFG cfg = *pCFG;
    cfg.dwTime = rQuest.time();
    m_listGuildQuest.push_back(cfg);
#ifdef _LX_DEBUG
    XLOG << "[公会-其他加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载任务" << rQuest.questid() << cfg.dwTime << XEND;
#endif
  }
  m_dwNextQuestTime = oBlob.nextquesttime();
  m_dwRenameTime = oBlob.renametime();

  if (m_listGuildQuest.empty())
  {
    if (m_dwNextQuestTime)
    {
      m_dwNextQuestTime = 0;
      refreshQuest(now());
    }
  }

  for (int i = 0; i < oBlob.dojomsg_size(); ++i)
  {
    const DojoMsgBlob& rDojoMsg = oBlob.dojomsg(i);
    std::list<DojoMsg>& rList = m_mapDojoMsg[rDojoMsg.dojoid()];
    for (int j = 0; j < rDojoMsg.msgs_size(); ++j)
    {
      rList.push_back(rDojoMsg.msgs(j));
    }
  }
  // city
  m_dwGiveupTime = oBlob.city_giveup_time();

  // building
  m_oBuilding.fromData(oBlob.building());

  // open function
  m_qwOpenFunction = oBlob.openfunction();

  // welfare
  m_oWelfare.fromData(oBlob.welfare());

  // challenge
  m_oChallenge.fromData(oBlob.challenge());

  // quest
  m_oQuest.fromData(oBlob.quests());

  // artifact
  m_oArtifact.fromData(oBlob.artifact());

  // gvg
  m_oGvg.fromData(oBlob.gvg());

  // treasure
  m_oTreasure.fromData(oBlob.treasures());

  auth_version();
  return true;
}

bool GuildMisc::modifyAuth(bool bAdd, EModify eModify, EGuildJob eJob, EAuth eAuth)
{
  if (eJob >= EGUILDJOB_MAX)
    return false;
  if (eAuth <= EAUTH_MIN || eAuth >= EAUTH_MAX || EAuth_IsValid(eAuth) == false)
    return false;

  DWORD dwValue = 1 << (eAuth - 1);
  if (bAdd)
  {
    if (eModify == EMODIFY_AUTH)
    {
      if ((m_arrJob[eJob].auth() & dwValue) != 0)
        return false;
      m_arrJob[eJob].set_auth(m_arrJob[eJob].auth() | dwValue);
    }
    else if (eModify == EMODIFY_EDITAUTH)
    {
      if ((m_arrJob[eJob].editauth() & dwValue) != 0)
        return false;
      m_arrJob[eJob].set_editauth(m_arrJob[eJob].editauth() | dwValue);
    }
    else
    {
      return false;
    }
  }
  else
  {
    if (eModify == EMODIFY_AUTH)
    {
      if ((m_arrJob[eJob].auth() & dwValue) == 0)
        return false;
      m_arrJob[eJob].set_auth(m_arrJob[eJob].auth() & (~dwValue));
    }
    else if (eModify == EMODIFY_EDITAUTH)
    {
      if ((m_arrJob[eJob].editauth() & dwValue) == 0)
        return false;
      m_arrJob[eJob].set_editauth(m_arrJob[eJob].editauth() & (~dwValue));
    }
    else
    {
      return false;
    }
  }

  updateJob(eJob);
  return true;
}

bool GuildMisc::hasAuth(EGuildJob eJob, EAuth eAuth)
{
  return GGuild::hasAuth(m_arrJob[eJob].auth(), eAuth);
}

bool GuildMisc::hasEditAuth(EGuildJob eJob, EAuth eAuth)
{
  return GGuild::hasAuth(m_arrJob[eJob].editauth(), eAuth);
}

void GuildMisc::updateJob(EGuildJob eJob)
{
  if (eJob >= EGUILDJOB_MAX || m_pGuild == nullptr)
    return;

  JobUpdateGuildCmd cmd;
  cmd.mutable_job()->CopyFrom(m_arrJob[eJob]);

  PROTOBUF(cmd, send, len);
  m_pGuild->broadcastCmd(send, len);

  JobUpdateGuildSCmd scmd;
  scmd.mutable_job()->CopyFrom(m_arrJob[eJob]);

  const TVecGuildMember& vecMember = m_pGuild->getMemberList();
  for (auto &v : vecMember)
  {
    if (v->isOnline() == true)
    {
      scmd.set_charid(v->getCharID());
      PROTOBUF(scmd, ssend, slen);
      thisServer->sendCmdToZone(m_pGuild->getZoneID(), ssend, slen);
    }
  }

  XDBG << "[公会-职位同步]" << m_pGuild->getGUID() << m_pGuild->getName() << "同步职位" << m_arrJob[eJob].ShortDebugString() << XEND;
}

const GuildJob& GuildMisc::getJob(EGuildJob eJob) const
{
  static const GuildJob d;
  if (eJob <= EGUILDJOB_MIN || eJob >= EGUILDJOB_MAX)
    return d;
  return m_arrJob[eJob];
}

void GuildMisc::auth_version()
{
  for (DWORD d = m_dwAuthVersion; d < AUTH_VERSION; ++d)
  {
    if (d == 0)
    {
      DWORD dwAuth = 1 << (EAUTH_GUILD_RENAME - 1);

      GuildJob& rChair = m_arrJob[EGUILDJOB_CHAIRMAN];
      rChair.set_auth(rChair.auth() | dwAuth);

      XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "执行version : 0 成功" << XEND;
    }
    else if (d == 1)
    {
      DWORD dwAuth = 1 << (EAUTH_GIVEUP_CITY - 1);

      GuildJob& rChair = m_arrJob[EGUILDJOB_CHAIRMAN];
      rChair.set_auth(rChair.auth() | dwAuth);

      GuildJob& rVice = m_arrJob[EGUILDJOB_VICE_CHAIRMAN];
      rVice.set_auth(rVice.auth() | dwAuth);

      XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "执行version : 1 成功" << XEND;
    }
    else if (d == 2)
    {
      DWORD dwAuth = (1 << (EAUTH_OPEN_BUILDING - 1)) | (1 << (EAUTH_BUILD - 1));

      GuildJob& rChair = m_arrJob[EGUILDJOB_CHAIRMAN];
      rChair.set_auth(rChair.auth() | dwAuth);

      GuildJob& rVice = m_arrJob[EGUILDJOB_VICE_CHAIRMAN];
      rVice.set_auth(rVice.auth() | dwAuth);

      XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "执行version : 2 成功" << XEND;
    }
    else if (d == 3)
    {
      DWORD dwAuth = (1 << (EAUTH_ARTIFACT_QUEST - 1)) | (1 << (EAUTH_ARTIFACT_PRODUCE - 1)) | (1 << (EAUTH_ARTIFACT_OPT - 1));

      GuildJob& rChair = m_arrJob[EGUILDJOB_CHAIRMAN];
      rChair.set_auth(rChair.auth() | dwAuth);

      GuildJob& rVice = m_arrJob[EGUILDJOB_VICE_CHAIRMAN];
      rVice.set_auth(rVice.auth() | dwAuth);

      XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "执行version : 3 成功" << XEND;
    }
    else if (d == 4)
    {
      DWORD dwAuth = (1 << (EAUTH_TREASURE_OPT - 1));

      GuildJob& rChair = m_arrJob[EGUILDJOB_CHAIRMAN];
      rChair.set_auth(rChair.auth() | dwAuth);

      GuildJob& rVice = m_arrJob[EGUILDJOB_VICE_CHAIRMAN];
      rVice.set_auth(rVice.auth() | dwAuth);

      XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "执行version : 4 成功" << XEND;
    }
    else if (d == 5)
    {
      DWORD dwAuth = (1 << (EAUTH_GUILD_SHOP - 1));

      GuildJob& rChair = m_arrJob[EGUILDJOB_CHAIRMAN];
      rChair.set_auth(rChair.auth() | dwAuth);

      GuildJob& rVice = m_arrJob[EGUILDJOB_VICE_CHAIRMAN];
      rVice.set_auth(rVice.auth() | dwAuth);

      XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "执行version : 5 成功" << XEND;
    }
    else if (d == 6)
    {
      DWORD dwAuth = (1 << (EAUTH_VOICE - 1));

      GuildJob& rChair = m_arrJob[EGUILDJOB_CHAIRMAN];
      rChair.set_auth(rChair.auth() | dwAuth);

      GuildJob& rVice = m_arrJob[EGUILDJOB_VICE_CHAIRMAN];
      rVice.set_auth(rVice.auth() | dwAuth);

      XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "执行version : 6 成功" << XEND;
    }
  }
  m_dwAuthVersion = AUTH_VERSION;
}

DWORD GuildMisc::getVarValue(EVarType eType)
{
  DWORD dwValue = m_oVar.getVarValue(eType);
  bool bChange = getVarTemp(eType) != dwValue;

  if (bChange)
  {
    setVarTemp(eType, dwValue);
    auto m = m_mapVarGuild.find(eType);
    if (m != m_mapVarGuild.end())
      m_pGuild->setMark(m->second);
  }

  return dwValue;
}

void GuildMisc::setVarValue(EVarType eType, DWORD dwValue)
{
  m_oVar.setVarValue(eType, dwValue);
  setVarTemp(eType, dwValue);
  auto m = m_mapVarGuild.find(eType);
  if (m != m_mapVarGuild.end())
    m_pGuild->setMark(m->second);
}

DWORD GuildMisc::getVarTemp(EVarType eType)
{
  if (eType >= EVARTYPE_MAX)
    return 0;
  return m_arrVarValue[eType];
}

void GuildMisc::setVarTemp(EVarType eType, DWORD dwValue)
{
  if (eType <= EVARTYPE_MIN || eType >= EVARTYPE_MAX)
    return;
  m_arrVarValue[eType] = dwValue;
}

void GuildMisc::refreshDonateTime(DWORD curSec)
{
  if (m_pGuild == nullptr || m_pGuild->getGuildCFG() == nullptr)
    return;

  if (curSec > m_dwDonateTime1)
  {
    m_dwDonateTime1 = curSec + m_pGuild->getGuildCFG()->dwDonateRefreshInterval1;
    m_pGuild->setMark(EGUILDDATA_MISC);
    m_pGuild->setMark(EGUILDDATA_DONATETIME1);
    m_pGuild->updateData();
    XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "刷新" << EDONATETYPE_NORMAL << "下一次刷新时间为 :" << m_dwDonateTime1 << XEND;
  }

  if (curSec > m_dwDonateTime2)
  {
    m_dwDonateTime2 = curSec + m_pGuild->getGuildCFG()->dwDonateRefreshInterval2;
    m_pGuild->setMark(EGUILDDATA_MISC);
    m_pGuild->setMark(EGUILDDATA_DONATETIME2);
    m_pGuild->updateData();
    XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "刷新" << EDONATETYPE_HIGHER << "下一次刷新时间为 :" << m_dwDonateTime2 << XEND;
  }

  if (curSec > m_dwDonateTime3)
  {
    m_dwDonateTime3 = curSec + m_pGuild->getGuildCFG()->dwDonateRefreshInterval3;
    m_pGuild->setMark(EGUILDDATA_MISC);
    m_pGuild->setMark(EGUILDDATA_DONATETIME3);
    m_pGuild->updateData();
    XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "刷新" << EDONATETYPE_HIGHER << "下一次刷新时间为 :" << m_dwDonateTime3 << XEND;
  }

  if (curSec > m_dwDonateTime4)
  {
    m_dwDonateTime4 = curSec + m_pGuild->getGuildCFG()->dwDonateRefreshInterval4;
    m_pGuild->setMark(EGUILDDATA_MISC);
    m_pGuild->setMark(EGUILDDATA_DONATETIME4);
    m_pGuild->updateData();
    XLOG << "[公会-其他]" << m_pGuild->getGUID() << m_pGuild->getName() << "刷新" << EDONATETYPE_HIGHER << "下一次刷新时间为 :" << m_dwDonateTime4 << XEND;
  }
}

void GuildMisc::refreshQuest(DWORD curSec)
{
  if (m_pGuild == nullptr || m_pGuild->getGuildCFG() == nullptr)
    return;
  if (curSec < m_dwQuestTick)
    return;

  m_dwQuestTick = curSec + GUILD_RECORD_TICK_TIME;

  bool bNtf = false;
  GuildQuestUpdateGuildSCmd scmd;

  // overtime
  for (auto s = m_listGuildQuest.begin(); s != m_listGuildQuest.end();)
  {
    if (curSec <= s->dwTime)
    {
      ++s;
      continue;
    }
    XLOG << "[公会-任务刷新]" << m_pGuild->getGUID() << m_pGuild->getName() << "在" << curSec << "任务 id :" << s->dwQuestID << "time :" << s->dwTime << "超时被移除" << XEND;
    scmd.add_dels(s->dwQuestID);
    s = m_listGuildQuest.erase(s);
    bNtf = true;
  }

  // refresh new
  if (m_dwNextQuestTime <= curSec)
  {
    const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
    DWORD dwCount = m_dwNextQuestTime == 0 ? rCFG.dwQuestDefaultCount : 1;
    DWORD dwActiveMember = m_pGuild->getActiveMemberCount();

    // refresh new
    for (DWORD d = 0; d < dwCount; ++d)
    {
      const SGuildQuestCFG* pCFG = GuildConfig::getMe().randomGuildQuest(m_pGuild->getLevel(), dwActiveMember, m_listGuildQuest);
      if (pCFG == nullptr)
      {
        XERR << "[公会-任务刷新]" << m_pGuild->getGUID() << m_pGuild->getName() << "有" << dwActiveMember << "活跃成员,在" << curSec << "刷新任务失败" << XEND;
        continue;
      }
      SGuildQuestCFG stCFG = *pCFG;
      stCFG.dwTime = curSec + rCFG.dwQuestClearTime;
      m_listGuildQuest.push_back(stCFG);
      XLOG << "[公会-任务刷新]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "有" << dwActiveMember << "在" << curSec << "刷新出新任务 :" << stCFG.dwQuestID << "有效时间 :" << stCFG.dwTime << XEND;
      stCFG.toData(scmd.add_updates());
      bNtf = true;
    }

    // calc next time
    if (m_dwNextQuestTime == 0)
      m_dwNextQuestTime = curSec + LuaManager::getMe().call<DWORD>("calcGuildQuestNextTime", m_pGuild->getGuildCFG()->dwQuestTime);
    else
      m_dwNextQuestTime += LuaManager::getMe().call<DWORD>("calcGuildQuestNextTime", m_pGuild->getGuildCFG()->dwQuestTime);
    XLOG << "[公会-任务刷新]" << m_pGuild->getGUID() << m_pGuild->getName() << "下一次刷新时间为" << m_dwNextQuestTime << "当前包含任务";
    for (auto &l : m_listGuildQuest)
      XLOG << l.dwQuestID << l.dwTime;
    XLOG << XEND;
  }

  // inform scene for npc refresh
  if (bNtf)
  {
    RefreshGuildTerritoryGuildSCmd cmd;
    m_pGuild->toData(cmd.mutable_info());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len);

    if (scmd.updates_size() > 0 || scmd.dels_size() > 0)
    {
      const TVecGuildMember& vecMembers = m_pGuild->getMemberList();
      for (auto &v : vecMembers)
      {
        scmd.set_charid(v->getCharID());
        PROTOBUF(scmd, ssend, slen);
        v->sendCmdToZone(ssend, slen);
      }
    }
  }
}

void GuildMisc::getPublicInfo(const SocialUser& rUser, DojoPublicInfoCmd& rev)
{
  auto it = m_mapDojoMsg.find(rev.dojoid());
  if (it != m_mapDojoMsg.end())
  {
    auto& subList = it->second;
    for (auto li = subList.begin(); li != subList.end(); ++li)
    {
      rev.mutable_msgblob()->add_msgs()->CopyFrom(*li);
    }
  }
  PROTOBUF(rev, send, len);
  
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
}

/*增加留言*/
void GuildMisc::addMsg(const SocialUser& rUser, DojoAddMsg& rev)
{
  std::list<DojoMsg>& rList = m_mapDojoMsg[rev.dojoid()];
  rList.push_back(rev.dojomsg());
  while (rList.size() > 50)
  {
    rList.pop_front();
  }
  //sync client

  DojoPublicInfoCmd cmd;
  cmd.set_dojoid(rev.dojoid());
  getPublicInfo(rUser, cmd);
}

void GuildMisc::setCityID(DWORD dwCityID)
{
  if (m_pGuild == nullptr)
    return;

  m_dwCityID = dwCityID;
  m_pGuild->setMark(EGUILDDATA_CITYID);

  m_pGuild->updateData();

  TVecGuildMember& vecMember = m_pGuild->getAllMemberList();
  for (auto &v : vecMember)
  {
    if (v->isOnline() == true && hasAuth(v->getJob(), EAUTH_GIVEUP_CITY) == true)
    {
      if (v->getFrameStatus() == false)
      {
        v->setFrameStatus(true);
        v->setFrameStatus(false);
      }
    }
  }

  XLOG << "[公会杂项-城池]" << m_pGuild->getGUID() << m_pGuild->getName() << "设置城池" << dwCityID << XEND;
}

void GuildMisc::setCityGiveupTime(DWORD dwTime)
{
  if (m_pGuild == nullptr || m_dwGiveupTime == dwTime)
    return;

  m_dwGiveupTime = dwTime;
  m_pGuild->setMark(EGUILDDATA_CITY_GIVEUP_CD);

  m_pGuild->updateData();
  XLOG << "[公会杂项-城池]" << m_pGuild->getGUID() << m_pGuild->getName() << "设置城池放弃时间" << m_dwGiveupTime << XEND;
}

void GuildMisc::updateGiveupCity(DWORD curTime)
{
  if (getCityGiveupTime() == 0 || curTime < getCityGiveupTime())
    return;

  GuildCityActionGuildSCmd cmd;
  cmd.set_action(EGUILDCITYACTION_TO_RECORD_SAVE);
  cmd.set_zoneid(m_pGuild->getZoneID());
  cmd.set_status(EGUILDCITYSTATUS_GIVEUP);

  GuildCityInfo* pInfo = cmd.add_infos();
  pInfo->set_flag(getCityID());
  pInfo->set_id(0);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len);

  setCityID(0);
  setCityGiveupTime(0);

  m_pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_GIVEUPCITY_DONE);
  XLOG << "[公会-城池放弃]" << m_pGuild->getGUID() << m_pGuild->getName() << "在" << curTime << "成功放弃据点" << XEND;
}

void GuildMisc::openFunction(EGuildFunction eFunction)
{
  m_qwOpenFunction |= (QWORD(1) << (DWORD(eFunction) - 1));
  m_pGuild->setMark(EGUILDDATA_OPEN_FUNCTION);
  m_pGuild->setMark(EGUILDDATA_MISC);
  m_pGuild->updateData();

  RefreshGuildTerritoryGuildSCmd cmd;
  m_pGuild->toData(cmd.mutable_info());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len);

  XLOG << "[公会-开启功能]" << m_pGuild->getGUID() << m_pGuild->getName() << "开启功能:" << eFunction << XEND;
}

