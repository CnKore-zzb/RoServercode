#include "Guild.h"
#include "TableManager.h"
#include "xDBFields.h"
#include "xFunctionTime.h"
#include "GuildManager.h"
#include "PlatLogManager.h"
//#include "SocialityManager.h"
#include "GuildServer.h"
#include "MiscConfig.h"
#include "ItemConfig.h"
#include "GCharManager.h"
#include "TeamCmd.pb.h"
#include "GuildSCmd.pb.h"
//#include "ChatManager_SO.h"
#include "RedisManager.h"
#include "GGuild.h"
#include "GuildIconManager.h"
#include "QuestConfig.h"
#include "MsgManager.h"

// guild
Guild::Guild(QWORD guid, DWORD dwZoneID, const string& name) : m_qwGuildID(guid), m_dwZoneID(dwZoneID), m_strName(name), m_oPack(this), m_oEvent(this), m_oMisc(this), m_oPhotoMgr(this)
{
  ((xEntryC *)this)->set_id(guid);
  ((xEntryC *)this)->set_name(name.c_str());
  m_dwLevel = 1;
  m_dwVarTick = xTime::getDayStart(now()) + DAY_T + randBetween(GUILD_RECORD_TICK_TIME / 2, GUILD_RECORD_TICK_TIME);
}

Guild::~Guild()
{
  for(auto &v : m_vecMembers)
  {
    SAFE_DELETE(v);
  }
}

bool Guild::toMemberString(string& str)
{
  BlobGuildMember oBlob;

  for (auto v : m_vecMembers)
    v->toData(oBlob.add_members());

  return oBlob.SerializeToString(&str);
}

bool Guild::toApplyString(string& str)
{
  BlobGuildApply oBlob;

  for (auto v : m_vecApplys)
    v->toData(oBlob.add_applys());

  return oBlob.SerializeToString(&str);
}

void Guild::toSummaryData(GuildSummaryData* pData)
{
  if (pData == nullptr)
    return;

  pData->set_guid(m_qwGuildID);
  pData->set_level(m_dwLevel);
  pData->set_zoneid(getClientZoneID(m_dwZoneID));
  pData->set_curmember(static_cast<DWORD>(m_vecMembers.size()));
  pData->set_cityid(getMisc().getCityID());
  if (m_pGuildCFG != nullptr)
    pData->set_maxmember(m_pGuildCFG->dwMaxMember);

  GMember* pChairman = getChairman();
  if (pChairman != nullptr)
  {
    pData->set_chairmangender(pChairman->getGender());
    pData->set_chairmanname(pChairman->getName());
  }

  pData->set_guildname(getName());
  pData->set_recruitinfo(getRecruit());
  pData->set_portrait(getPortrait());
}

void Guild::toData(GuildData* pData, bool bChairman /*= true*/, GMember* pMember /*= nullptr*/)
{
  if (pData == nullptr)
    return;

  toSummaryData(pData->mutable_summary());

  pData->set_questresettime(m_dwQuestTime);
  //pData->set_constructionlevel(m_dwConLevel);
  pData->set_dismisstime(m_dwDismissTime);
  pData->set_createtime(m_dwCreateTime);
  pData->set_zonetime(m_dwZoneTime);
  pData->set_nextzone(getClientZoneID(m_dwNextZone));
  pData->set_name(m_strName);
  pData->set_boardinfo(m_strBoard);
  pData->set_recruitinfo(m_strRecruit);
  pData->set_donatetime1(getMisc().getDonateTime1());
  pData->set_donatetime2(getMisc().getDonateTime2());
  pData->set_assettoday(getMisc().getVarValue(EVARTYPE_GUILD_MAXASSET));
  pData->set_citygiveuptime(getMisc().getCityGiveupTime());
  pData->set_openfunction(getMisc().getOpenFunction());
  pData->set_asset(getAsset());
  pData->set_gvg_treasure_count(getMisc().getTreasure().hasTreasure() == true ? 1 : 0);
  pData->set_guild_treasure_count(getMisc().getVarValue(EVARTYPE_GUILD_TREASURE_COUNT));
  pData->set_bcoin_treasure_count(getMisc().getVarValue(EVARTYPE_BCOIN_TREASURE_COUNT));
  pData->set_insupergvg(getMisc().getGvg().inSuperGvg());
  pData->set_supergvg_lv(getMisc().getGvg().getSuperGvgLv());

  pData->clear_members();
  for (auto &v : m_vecMembers)
    v->toData(pData->add_members(), true);

  pData->clear_jobs();
  for (int i = EGUILDJOB_MIN + 1; i < EGUILDJOB_MAX; ++i)
  {
    if (i == EGUILDJOB_APPLY || i == EGUILDJOB_INVITE)
      continue;
    const GuildJob& rJob = getMisc().getJob(static_cast<EGuildJob>(i));
    pData->add_jobs()->CopyFrom(rJob);
    XDBG << "[公会-职位同步]" << getGUID() << getName() << "职位" << rJob.ShortDebugString() << XEND;
  }

  if (bChairman)
  {
    for (auto &v : m_vecApplys)
      v->toData(pData->add_applys());
  }

  pData->clear_challenges();
  getMisc().getChallenge().toGuildData(pData, pMember);
}

void Guild::toData(GuildInfo* pInfo, const GuildJob& rJob /*= GuildJob()*/)
{
  if (pInfo == nullptr)
    return;

  pInfo->set_id(getGUID());
  pInfo->set_zoneid(getZoneID());
  pInfo->set_lv(getLevel());
  pInfo->set_auth(rJob.auth());
  pInfo->set_name(getName());
  pInfo->set_portrait(getPortrait());
  pInfo->set_jobname(rJob.name());

  for (auto &v : m_vecMembers)
    v->toData(pInfo->add_members());

  const TListGuildQuestCFG& listQuest = getMisc().getQuestList();
  for (auto &l : listQuest)
    l.toData(pInfo->add_quests());

  getMisc().getBuilding().toData(pInfo->mutable_building());
  pInfo->set_openfunction(m_oMisc.getOpenFunction());

  getMisc().getArtifact().toGuildInfo(pInfo);
  toData(pInfo->mutable_artifacequest());

  pInfo->mutable_gvg()->set_insupergvg(getMisc().getGvg().inSuperGvg());
}

void Guild::toData(GuildArtifactQuest* pInfo)
{
  if (pInfo == nullptr)
    return;

  pInfo->clear_submitids();
  const TSetDWORD& setIDs = getMisc().getQuest().getSubmitList();
  for (auto &s : setIDs)
    pInfo->add_submitids(s);

  pInfo->clear_datas();
  const TSetDWORD& setPieces = QuestConfig::getMe().getArtifactPieceList();
  for (auto &s : setPieces)
  {
    DWORD dwCount = getPack().getItemCount(s);
    if (dwCount > 0)
    {
      ItemData* pData = pInfo->add_datas();
      pData->mutable_base()->set_id(s);
      pData->mutable_base()->set_count(dwCount);
    }
  }
}

bool Guild::toRecord(xRecord& record)
{
  record.put("id", m_qwGuildID);
  record.put("zoneid", m_dwZoneID);
  record.put("lv", m_dwLevel);
  record.put("questtime", m_dwQuestTime);
  record.put("conlv", m_dwAsset);
  record.put("dismisstime", m_dwDismissTime);
  record.put("createtime", m_dwCreateTime);

  record.putString("name", m_strName);
  record.putString("board", m_strBoard);
  record.putString("recruit", m_strRecruit);
  record.putString("portrait", m_strPortrait);

  string data;
  if (toMemberString(data) == false)
    return false;
  record.putBin("member", (unsigned char*)data.c_str(), data.size());

  data.clear();
  if (toApplyString(data) == false)
    return false;
  record.putBin("apply", (unsigned char*)data.c_str(), data.size());

  data.clear();
  if (toBlobMiscString(data) == false)
    return false;
  record.putBin("misc", (unsigned char*)data.c_str(), data.size());

  return true;
}

void Guild::reload(ConfigType type)
{
  if (type == ConfigType::guild)
  {
    setGuildCFG(GuildConfig::getMe().getGuildCFG(getLevel()));
  }
}

void Guild::dismiss()
{
  ExitGuildGuildCmd cmd;
  PROTOBUF(cmd, send, len);

  for (auto &v : m_vecMembers)
  {
    if (v->isOnline() == false)
    {
      //ChatManager_SO::getMe().addOfflineMsg(v->getCharID(), ESYSTEMMSG_ID_GUILD_DISMISSDONE, m_strName);
      continue;
    }

    // msg inform
    thisServer->sendMsg(v->getZoneID(), v->getCharID(), ESYSTEMMSG_ID_GUILD_DISMISS, MsgParams(m_strName));

    // exit guild
    v->sendCmdToMe(send, len);
  }

  // 删除留声机数据
  GuildMusicDeleteGuildSCmd delcmd;
  delcmd.set_guildid(m_qwGuildID);
  PROTOBUF(delcmd, delsend, dellen);
  thisServer->sendCmdToZone(m_dwZoneID, delsend, dellen);
  XLOG << "[公会管理-删除点唱机数据] guildid:" << m_qwGuildID << "成功发送至RecordServer处理" << XEND;

  // clear city info
  if (getMisc().getCityID() != 0)
  {
    XLOG << "[公会-解散]" << getGUID() << getName() << "放弃城池" << getMisc().getCityID() << XEND;
    getMisc().setCityGiveupTime(xTime::getCurSec());
    getMisc().updateGiveupCity(getMisc().getCityGiveupTime() * 2);
  }
}

void Guild::broadcastMsg(DWORD dwMsgID, MsgParams oParams /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, DWORD dwDelay /*= 0*/)
{
  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  cmd.set_delay(dwDelay);
  oParams.toData(cmd);

  PROTOBUF(cmd, send, len);
  for (auto &v : m_vecMembers)
    v->sendCmdToMe(send, len);
}

// 在公会频道聊天窗口显示npc的发言
void Guild::broadcastNpcMsg(DWORD dwNpcID, DWORD dwMsgID, MsgParams oParams /*= MsgParams()*/)
{
  NpcChatNtf cmd;
  cmd.set_channel(ECHAT_CHANNEL_GUILD);
  cmd.set_npcid(dwNpcID);
  cmd.set_msgid(dwMsgID);
  oParams.toNpcChatNtf(cmd);

  PROTOBUF(cmd, send, len);
  for (auto &v : m_vecMembers)
    v->sendCmdToMe(send, len);
}

void Guild::broadcastCmd(const void* buf, WORD len, bool bLeader /*= false*/)
{
  if (buf == nullptr || len == 0)
    return;

  for (auto &v : m_vecMembers)
  {
    if (bLeader && v->getJob() != EGUILDJOB_CHAIRMAN && v->getJob() != EGUILDJOB_VICE_CHAIRMAN)
      continue;
    v->sendCmdToMe(buf, len);
  }
}

DWORD Guild::getActiveMemberCount() const
{
  DWORD dwCount = 0;
  DWORD dwDayStart = xTime::getDayStart(xTime::getCurSec());
  for (auto &v : m_vecMembers)
  {
    if (GGuild::isActive(dwDayStart, v->getOnlineTime(), v->getOfflineTime()) == true)
      ++dwCount;
  }
  return dwCount;
}

DWORD Guild::getMaxViceCount() const
{
  if (m_pGuildCFG != nullptr)
    return m_pGuildCFG->dwViceCount;

  return 0;
}

DWORD Guild::getViceCount() const
{
  DWORD dwCount = 0;
  for (auto &v : m_vecMembers)
  {
    if (v->getJob() == EGUILDJOB_VICE_CHAIRMAN)
      ++dwCount;
  }

  return dwCount;
}

bool Guild::hasOnlineMember() const
{
  for (auto &v : m_vecMembers)
  {
    if (v->isOnline() == true)
      return true;
  }

  return false;
}

bool Guild::addMember(const SocialUser& rUser, EGuildJob eJob, bool bBack)
{
  GMember* pMember = getMember(rUser.charid());
  if (pMember != nullptr)
  {
    XERR << "[公会-添加成员]" << m_qwGuildID << m_strName << "添加成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "失败,成员已存在" << XEND;
    return false;
  }

  DWORD dwMaxMember = m_pGuildCFG != nullptr ? m_pGuildCFG->dwMaxMember : 0;
  if (getMemberCount() >= dwMaxMember)
  {
    XERR << "[公会-添加成员]" << m_qwGuildID << m_strName << "添加成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "失败,超过最大上限 :" << dwMaxMember << XEND;
    return false;
  }

  pMember = NEW GMember(this, rUser, eJob);
  if (pMember == nullptr)
  {
    XERR << "[公会-添加成员]" << m_qwGuildID << m_strName << "添加成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "失败,创建成员失败" << XEND;
    return false;
  }

  GMember* pApply = getApply(rUser.charid());
  GMember* pInvite = getInvite(rUser.charid());
  if (pApply != nullptr)
    *pMember = *pApply;
  else if (pInvite != nullptr)
    *pMember = *pInvite;
  else
  {
    XERR << "[公会-添加成员]" << m_qwGuildID << m_strName << "添加成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "失败,非法添加" << XEND;
    SAFE_DELETE(pMember);
    return false;
  }

  removeApply(rUser.charid());
  removeInvite(rUser.charid());

  m_setMemberUpdate.insert(rUser.charid());
  m_vecMembers.push_back(pMember);

  if (getMember(rUser.charid()) == nullptr)
  {
    XERR << "[公会-添加成员]" << m_qwGuildID << m_strName << "添加成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "失败" << XEND;
    SAFE_DELETE(pMember);
    return false;
  }

  GCharWriter pGChar(thisServer->getRegionID(), rUser.charid());
  pGChar.setGuildID(getGUID());
  pGChar.setGuildName(getName());
  pGChar.setGuildPortrait(getPortrait());
  pGChar.save();

  pMember->setJob(eJob);
  if (pMember->isOnline() == true)
  {
    EnterGuildGuildCmd cmd;
    toData(cmd.mutable_data(), pMember->getJob() == EGUILDJOB_CHAIRMAN, pMember);
    PROTOBUF(cmd, send, len);
    pMember->sendCmdToMe(send, len);

    syncInfoToScene(*pMember);
    thisServer->sendMsg(pMember->getZoneID(), pMember->getCharID(), ESYSTEMMSG_ID_GUILD_SELFENTER, MsgParams(getName()));

    getPhoto().queryAutoPhoto(pMember->getAccid(), pMember->getZoneID(), EPHOTOACTION_LOAD_FROM_SCENE);

    getMisc().getWelfare().notifyMember(pMember);
    getMisc().getArtifact().notifyAllData(pMember);
  }

  if (eJob != EGUILDJOB_CHAIRMAN)
    broadcastMsg(ESYSTEMMSG_ID_GUILD_ENTER,  MsgParams(pMember->getName()));

  getEvent().addEvent(EGUILDEVENT_NEW_MEMBER, TVecString{pMember->getName()});

  // 同步实时语音房间号
  pMember->sendRealtimeVoiceID();

  updateMember();
  XLOG << "[公会-添加成员]" << m_qwGuildID << m_strName << "添加成员" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "成功" << XEND;
  return true;
}

bool Guild::removeMember(QWORD qwCharID)
{
  auto v = find_if(m_vecMembers.begin(), m_vecMembers.end(), [qwCharID](const GMember* p) -> bool{
    return p == nullptr ? false : p->getCharID() == qwCharID;
  });
  if (v == m_vecMembers.end())
  {
    XERR << "[公会-移除成员] charid :" << qwCharID << "失败,未找到该成员" << XEND;
    return false;
  }

  GMember* pMember = *v;
  if (pMember->getJob() == EGUILDJOB_CHAIRMAN)
  {
    if (getMemberCount() > 1 && exchangeChairman(0, 0, EEXCHANGEMETHOD_FIND) == false)
      XERR << "[公会-移除成员]" << pMember->getCharID() << pMember->getName() << pMember->getJob() << "退出公会 移交会长失败" << XEND;
  }

  pMember->setRedTip(false);

  GCharWriter pGChar(thisServer->getRegionID(), qwCharID);
  pGChar.setGuildID(0);
  pGChar.setGuildName("");
  pGChar.setGuildPortrait("");
  pGChar.save();

  if (pMember->isOnline() == true)
  {
    ExitGuildGuildCmd cmd;
    PROTOBUF(cmd, send, len);
    pMember->sendCmdToMe(send, len);

    GuildInfoSyncGuildSCmd cmd1;
    cmd1.set_charid(pMember->getCharID());
    PROTOBUF(cmd1, send1, len1);
    thisServer->sendCmdToZone(pMember->getZoneID(), send1, len1);
  }

  //SocialityManager::getMe().updateSocialData(qwCharID, ESOCIALDATA_GUILDNAME, 0, "");
  //SocialityManager::getMe().updateSocialData(qwCharID, ESOCIALDATA_GUILDPORTRAIT, 0, "");

  getMisc().getArtifact().onRemoveMember(pMember);
  getMisc().getQuest().questSyncToZone(pMember->getCharID());

  m_setMemberUpdate.insert(qwCharID);
  m_vecMembers.erase(v);

  XLOG << "[公会-移除成员]" << pMember->getCharID() << pMember->getName() << pMember->getJob() << "成功" << XEND;
  SAFE_DELETE(pMember);

  getPhoto().refreshPhoto();
  updateMember();
  return true;
}

GMember* Guild::getMember(QWORD qwCharID, QWORD qwAccID /*= 0*/)
{
  auto v = find_if(m_vecMembers.begin(), m_vecMembers.end(), [qwCharID](const GMember* p) -> bool{
    return p == nullptr ? false : p->getCharID() == qwCharID;
  });
  if (v != m_vecMembers.end())
    return *v;

  if (qwAccID != 0)
  {
    v = find_if(m_vecMembers.begin(), m_vecMembers.end(), [qwAccID](const GMember* p) -> bool{
      return p == nullptr ? false : p->getAccid() == qwAccID;
    });
    if (v != m_vecMembers.end())
      return *v;
  }
  return nullptr;
}

GMember* Guild::getChairman()
{
  auto v = find_if(m_vecMembers.begin(), m_vecMembers.end(), [](const GMember* p) -> bool{
    return p == nullptr ? false : p->getJob() == EGUILDJOB_CHAIRMAN;
  });
  if (v != m_vecMembers.end())
    return *v;
  return nullptr;
}

bool Guild::addApply(const UserInfo& rUser)
{
  const SocialUser& user = rUser.user();
  GMember* pApply = getApply(rUser.user().charid());
  if (pApply != nullptr)
  {
    XERR << "[公会-添加申请]" << getGUID() << getName() << "添加申请" << user.accid() << user.charid() << user.profession() << user.name() << "失败,已存在" << XEND;
    return false;
  }

  DWORD dwMaxApply = MiscConfig::getMe().getGuildCFG().dwMaxApplyCount;
  if (getApplyCount() >= dwMaxApply)
  {
    if (removeApply(0, EAPPLYMETHOD_TIME) == false)
    {
      XERR << "[公会-添加申请]" << getGUID() << getName() << "添加申请" << user.accid() << user.charid() << user.profession() << user.name() << "失败,超上限 :" << dwMaxApply << XEND;
      return false;
    }
  }

  pApply = NEW GMember(this, rUser.user(), EGUILDJOB_APPLY);
  if (pApply == nullptr)
  {
    XERR << "[公会-添加申请]" << getGUID() << getName() << "添加申请" << user.accid() << user.charid() << user.profession() << user.name() << "失败,创建失败"<< XEND;
    return false;
  }
  pApply->updateData(rUser, true);

  m_setApplyUpdate.insert(pApply->getCharID());
  m_vecApplys.push_back(pApply);
  updateApply();
  processRedTip(true);

  return true;
}

bool Guild::removeApply(QWORD qwCharID, EApplyMethod eMethod /*= EAPPLYMETHOD_MIN*/)
{
  if (eMethod == EAPPLYMETHOD_MIN)
  {
    auto v = find_if(m_vecApplys.begin(), m_vecApplys.end(), [qwCharID](const GMember* p) -> bool {
      return p == nullptr ? false : p->getCharID() == qwCharID;
    });
    if (v == m_vecApplys.end())
    {
      XERR << "[公会-移除申请]" << getGUID() << getName() << "移除申请 charid:" << qwCharID << "method :" << eMethod << "失败,未有该成员"<< XEND;
      return false;
    }

    m_setApplyUpdate.insert((*v)->getCharID());
    GMember* pApply = *v;
    m_vecApplys.erase(v);
    XLOG << "[公会-移除申请]" << getGUID() << getName() << "移除申请 charid:" << qwCharID << "method :" << eMethod << "成功"<< XEND;
    SAFE_DELETE(pApply);
    updateApply();
    processRedTip(false);
    return true;
  }
  else if (eMethod == EAPPLYMETHOD_TIME)
  {
    if (m_vecApplys.empty() == true)
      return true;
    sort(m_vecApplys.begin(), m_vecApplys.end(), [](const GMember* p1, const GMember* p2) -> bool{
      if (p1 == nullptr || p2 == nullptr)
        return false;
      return p1->getEnterTime() < p2->getEnterTime();
    });
    GMember* pApply = *m_vecApplys.begin();
    if (pApply == nullptr)
      return false;
    m_setApplyUpdate.insert(pApply->getCharID());
    m_vecApplys.erase(m_vecApplys.begin());
    XLOG << "[公会-移除申请]" << getGUID() << getName() << "移除申请 charid:" << pApply->getCharID() << "method :" << eMethod << "成功"<< XEND;
    SAFE_DELETE(pApply);
    updateApply();
    processRedTip(false);
    return true;
  }

  return false;
}

bool Guild::sendApplyList(QWORD qwCharID)
{
  GMember* pMember = getMember(qwCharID);
  if (pMember == nullptr || pMember->isOnline() == false)
    return false;
  if (pMember->getJob() != EGUILDJOB_CHAIRMAN && pMember->getJob() != EGUILDJOB_VICE_CHAIRMAN)
    return false;

  GuildApplyUpdateGuildCmd cmd;
  for (auto &v : m_vecApplys)
    v->toData(cmd.add_updates());
  if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    pMember->sendCmdToMe(send, len);
  }

  return true;
}

GMember* Guild::getApply(QWORD qwCharID)
{
  auto v = find_if(m_vecApplys.begin(), m_vecApplys.end(), [qwCharID](const GMember* p) -> bool{
    return p == nullptr ? false : p->getCharID() == qwCharID;
  });
  if (v != m_vecApplys.end())
    return *v;
  return nullptr;
}

bool Guild::addInvite(QWORD qwCharID)
{
  auto v = find_if(m_vecInvites.begin(), m_vecInvites.end(), [qwCharID](const GMember* p) -> bool{
    return p == nullptr ? false : p->getCharID() == qwCharID;
  });
  if (v != m_vecInvites.end())
    return false;

  SocialUser oUser;
  oUser.set_charid(qwCharID);
  GMember* pInvite = NEW GMember(this, oUser, EGUILDJOB_INVITE);
  if (pInvite == nullptr)
    return false;
  m_vecInvites.push_back(pInvite);
  return true;
}

bool Guild::removeInvite(QWORD qwCharID)
{
  auto v = find_if(m_vecInvites.begin(), m_vecInvites.end(), [qwCharID](const GMember* p) -> bool{
    return p == nullptr ? false : p->getCharID() == qwCharID;
  });
  if (v == m_vecInvites.end())
    return false;

  m_vecInvites.erase(v);
  return true;
}

GMember* Guild::getInvite(QWORD qwCharID)
{
  auto v = find_if(m_vecInvites.begin(), m_vecInvites.end(), [qwCharID](const GMember* p) -> bool{
    return p == nullptr ? false : p->getCharID() == qwCharID;
  });
  if (v != m_vecInvites.end())
    return *v;

  return nullptr;
}

bool Guild::exchangeChairman(QWORD qwOldChair, QWORD qwNewChair, EExchangeMethod eMethod /*= EEXCHANGEMETHOD_MIN*/)
{
  if (eMethod == EEXCHANGEMETHOD_MIN)
  {
    if (qwOldChair == qwNewChair)
      return false;
    GMember* pChair = getMember(qwOldChair);
    GMember* pNewChair = getMember(qwNewChair);
    if (pChair == nullptr || pNewChair == nullptr)
      return false;
    if (pChair->getJob() != EGUILDJOB_CHAIRMAN || pNewChair->getJob() == EGUILDJOB_CHAIRMAN)
      return false;
    if (getMisc().getVarValue(EVARTYPE_GUILD_EXCHANGECHAIR) != 0)
    {
      thisServer->sendMsg(pChair->getZoneID(), pChair->getCharID(), ESYSTEMMSG_ID_GUILD_EXCHANGECHAIR_ONCE);
      return false;
    }

    bool newHasAuth = getMisc().hasAuth(pNewChair->getJob(), EAUTH_AGREE);

    getMisc().setVarValue(EVARTYPE_GUILD_EXCHANGECHAIR, 1);
    setMark(EGUILDDATA_MISC);

    pChair->setJob(pNewChair->getJob());
    pNewChair->setJob(EGUILDJOB_CHAIRMAN);

    if(newHasAuth == false)
    {
      pChair->setRedTip(false);
      pNewChair->setRedTip(true);
    }

    syncInfoToScene(*pChair);
    syncInfoToScene(*pNewChair);

    if (pNewChair)
    {
      broadcastMsg(ESYSTEMMSG_ID_GUILD_EXCHANGE, MsgParams(pNewChair->getName()));
      sendApplyList(pNewChair->getCharID());
    }

    XLOG << "[公会-会长交接] 公会" << getGUID() << getName() << "会长" << pChair->getCharID() << pChair->getName() << "转让会长给" << pNewChair->getCharID() << pNewChair->getName() << XEND;

    //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Change_GuildLeader;

    // 同步自定义icon
    GuildIconManager::getMe().syncData(m_qwGuildID, pNewChair);

    if (getMisc().getCityID() != 0)
    {
      CityDataUpdateGuildSCmd ccmd;
      ccmd.set_cityid(getMisc().getCityID());
      ccmd.set_membercount(m_vecMembers.size());
      ccmd.set_leadername(pNewChair->getName());

      PROTOBUF(ccmd, csend, clen);
      thisServer->sendCmdToZone(getZoneID(), csend, clen);
    }

    if (pNewChair)
    {
      PlatLogManager::getMe().eventLog(thisServer,
        0,
        pNewChair->getZoneID(),
        pNewChair->getAccid(),
        pNewChair->getCharID(),
        eid,
        0,  // charge
        eType, 0, 1);

      PlatLogManager::getMe().changeLog(thisServer,
        0,
        pNewChair->getZoneID(),
        pNewChair->getAccid(),
        pNewChair->getCharID(),
        eType,
        eid,
        EChange_GuildLeader,
        qwOldChair,
        qwNewChair,
        getGUID());
    }
  }
  else if (eMethod == EEXCHANGEMETHOD_FIND)
  {
    TVecGuildMember vecTemp;
    for (auto &v : m_vecMembers)
    {
      if (v->isOnline() == false)
        continue;
      if (v->getJob() != EGUILDJOB_CHAIRMAN)
        vecTemp.push_back(v);
    }
    if (vecTemp.empty() == true)
      return false;

    GMember* pNewChair = nullptr;
    DWORD dwMaxCon = 0;
    for (auto &v : vecTemp)
    {
      if (v->getContribution() >= dwMaxCon)
        pNewChair = v;
    }
    if (pNewChair == nullptr)
    {
      XERR << "[公会-会长交接] 公会" << getGUID() << getName() << "交接会长失败,未能获得下一任会长" << XEND;
      return false;
    }

    GMember* pChairman = getChairman();
    if (pChairman != nullptr)
    {
      pChairman->setJob(EGUILDJOB_MEMBER1);
      syncInfoToScene(*pChairman);
    }

    bool newHasAuth = getMisc().hasAuth(pNewChair->getJob(), EAUTH_AGREE);

    pNewChair->setJob(EGUILDJOB_CHAIRMAN);
    sendApplyList(pNewChair->getCharID());
    syncInfoToScene(*pNewChair);
    pChairman = pNewChair;

    if(newHasAuth == false)
    {
      pNewChair->setRedTip(true);
    }
    pChairman->setRedTip(false);

    if (getMisc().getCityID() != 0)
    {
      CityDataUpdateGuildSCmd ccmd;
      ccmd.set_cityid(getMisc().getCityID());
      ccmd.set_membercount(m_vecMembers.size());
      ccmd.set_leadername(pNewChair->getName());

      PROTOBUF(ccmd, csend, clen);
      thisServer->sendCmdToZone(getZoneID(), csend, clen);
    }

    XLOG << "[公会-会长交接] 公会" << getGUID() << getName() << pChairman->getCharID() << pChairman->getName() << "成为会长" << XEND;
  }
  else
  {
    return false;
  }

  return true;
}

bool Guild::levelup(QWORD qwCharID, DWORD addlevel)
{
  if (m_pGuildCFG == nullptr)
    m_pGuildCFG = GuildConfig::getMe().getGuildCFG(getLevel());
  if (m_pGuildCFG == nullptr)
    return false;
  const SGuildCFG* pNextGuildCFG = GuildConfig::getMe().getGuildCFG(getLevel() + addlevel);
  if (pNextGuildCFG == nullptr)
    return false;

  setLevel(getLevel() + addlevel);
  m_pGuildCFG = pNextGuildCFG;
  broadcastMsg(ESYSTEMMSG_ID_GUILD_LEVELUP, MsgParams(getLevel() - addlevel, getLevel()));

  for (auto &v : m_vecMembers)
    v->setLevelupEffect(true);
  GMember* pMem = getMember(qwCharID);
  if (!pMem)
    pMem = getChairman();
  if(!pMem)
    return false;

  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  if (getLevel() >= rCFG.pairIconNtfLv.first && getLevel() <= rCFG.pairIconNtfLv.second)
  {
    GMember* pChairman = getChairman();
    if (pChairman != nullptr)
      thisServer->sendMsg(pChairman->getZoneID(), pChairman->getCharID(), ESYSTEMMSG_ID_GUILD_NEW_PORTRAIT_NTF);
  }

  RefreshGuildTerritoryGuildSCmd cmd;
  toData(cmd.mutable_info());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(getZoneID(), send, len);

  //log
  {
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Change_GuildLv;
    PlatLogManager::getMe().eventLog(thisServer,
      0,
      pMem->getZoneID(),
      pMem->getAccid(),
      pMem->getCharID(),
      eid,
      0,  // charge
      eType, 0, 1);

    PlatLogManager::getMe().changeLog(thisServer,
      0,
      pMem->getZoneID(),
      pMem->getAccid(),
      pMem->getCharID(),
      eType,
      eid,
      EChange_GuildLv,
      getLevel() - addlevel,
      getLevel(),
      getGUID());
  }
  return true;
}

void Guild::maintenance(DWORD curTime)
{
  if (m_pGuildCFG == nullptr)
    return;

  DWORD dwValue = m_pGuildCFG->dwMaintenance;
  if (getAsset() >= dwValue)
  {
    subAsset(dwValue, ESOURCE_GUILD_MAINTENANCE);
    XLOG << "[公会-维护]" << getGUID() << getLevel() << getName() << "在时间 :" << curTime << "进行了维护, 消耗了" << dwValue << "资金" << XEND;
    return;
  }

  if (m_dwLevel <= MiscConfig::getMe().getGuildCFG().dwMaintenanceProtectTime)
    return;

  DWORD dwAssetNum = getAsset();
  subAsset(getAsset(), ESOURCE_GUILD_MAINTENANCE);

  if (dwValue >= dwAssetNum)
  {
    DWORD rest = dwValue - dwAssetNum;
    if (rest > m_pGuildCFG->dwLevelupFund)
    {
      XERR << "[公会-维护]" << getGUID() << getLevel() << getName() << "需要资金" << rest << "剩余资金:" << m_pGuildCFG->dwLevelupFund << "降级后资金仍然不足,维护失败" << XEND;
      return;
    }
    DWORD ret = m_pGuildCFG->dwLevelupFund - rest;
    addAsset(ret, true, ESOURCE_GUILD_MAINTENANCE);

    setLevel(m_dwLevel - 1);
    m_pGuildCFG = GuildConfig::getMe().getGuildCFG(getLevel());

    broadcastMsg(ESYSTEMMSG_ID_GUILD_LEVELDOWN, MsgParams(getLevel() + 1, getLevel()));
    XLOG << "[公会-维护] " << getGUID() << getLevel() << getName() << "在时间 :" << curTime << "进行了维护, 消耗了" << dwValue << "降了 1 级 获得了" << ret << "资金" << XEND;
  }
}

void Guild::syncInfoToScene(const GMember& rMember, bool bCreate /*= false*/)
{
  GuildInfoSyncGuildSCmd cmd;
  cmd.set_charid(rMember.getCharID());
  toData(cmd.mutable_info(), getMisc().getJob(rMember.getJob()));
  cmd.mutable_info()->set_create(bCreate);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(rMember.getZoneID(), send, len);
}

void Guild::broadcastGuildInfoToScene()
{
  for (auto &v : m_vecMembers)
    syncInfoToScene(*v);
}

bool Guild::addAsset(DWORD dwCount, bool bNoCheck/* = false*/, ESource eSource/* = ESOURCE_MAX*/)
{
  if (m_pGuildCFG == nullptr)
  {
    XERR << "[公会资金-添加]" << getGUID() << getName() << "添加资金" << dwCount << "失败,公会未包含正确的配置" << XEND;
    return false;
  }

  DWORD dwDayAsset = getMisc().getVarValue(EVARTYPE_GUILD_MAXASSET);
  if (bNoCheck == false && m_pGuildCFG->dwAssetMaxPerDay && dwDayAsset >= m_pGuildCFG->dwAssetMaxPerDay)
  {
    XERR << "[公会资金-添加]" << getGUID() << getName() << "添加资金" << dwCount << "失败,超过上限" << m_pGuildCFG->dwAssetMaxPerDay << XEND;
    return false;
  }

  DWORD dwRealCount = 0, dwRealDayCount = 0;
  if (bNoCheck == false && m_pGuildCFG->dwAssetMaxPerDay)
  {
    DWORD dwLeft = m_pGuildCFG->dwAssetMaxPerDay - dwDayAsset;
    bool bOver = dwCount <= dwLeft;
    if (bOver)
    {
      dwRealCount = dwCount;
      dwRealDayCount = dwDayAsset + dwCount;
    }
    else
    {
      dwRealCount = dwLeft;
      dwRealDayCount = m_pGuildCFG->dwAssetMaxPerDay;
    }
  }
  else
  {
    dwRealCount = dwCount;
    dwRealDayCount = dwDayAsset + dwCount;
  }

  m_dwAsset += dwRealCount;
  if (bNoCheck == false)
    getMisc().setVarValue(EVARTYPE_GUILD_MAXASSET, dwRealDayCount);

  m_bitset.set(EGUILDDATA_ASSET);
  updateData();
  if (eSource != ESOURCE_MAX)
    getPack().itemLog(ITEM_ASSET, SQWORD(dwRealCount), m_dwAsset, eSource);
  XLOG << "[公会资金-添加]" << getGUID() << getName() << "添加资金" << dwCount << "成功,实际添加" << dwRealCount << "上限" << m_pGuildCFG->dwAssetMaxPerDay << XEND;
  return true;
}

bool Guild::subAsset(DWORD dwCount, ESource eSource)
{
  if (m_dwAsset < dwCount)
    return false;

  m_dwAsset -= dwCount;
  m_bitset.set(EGUILDDATA_ASSET);
  updateData();
  getPack().itemLog(ITEM_ASSET, -SQWORD(dwCount), m_dwAsset, eSource);
  XLOG << "[公会资金-删除]" << getGUID() << getName() << "删除资金" << dwCount << "成功,剩余" << m_dwAsset << XEND;
  return true;
}

void Guild::setDismissTime(DWORD dwDismiss)
{
  m_dwDismissTime = dwDismiss;
  setMark(EGUILDDATA_DISMISSTIME);
  updateData();
  GMember* pChairman = getChairman();
  if (pChairman != nullptr)
    getEvent().addEvent(dwDismiss != 0 ? EGUILDEVENT_SET_DISMISSGUILD : EGUILDEVENT_CANCEL_DISMISSGUILD, TVecString{pChairman->getName()});
  DWORD dwNow = xTime::getCurSec();
  if (m_dwDismissTime > dwNow)
  {
    DWORD dwDismissTime = m_dwDismissTime - dwNow;
    broadcastMsg(ESYSTEMMSG_ID_GUILD_DISMISSTIME, MsgParams(dwDismissTime / 3600, (dwDismissTime - dwDismissTime / 3600 * 3600) / 60));
  }
}

DWORD Guild::getDismissLastTime()
{
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  DWORD dwCount = getMemberCount();
  if (dwCount < 10)
    return rCFG.dwDismissTime;

  DWORD dwLastOnlineCount = 0;
  DWORD dwNow = xTime::getCurSec();
  for (auto &v : m_vecMembers)
  {
    if (v->isOnline() == true)
      continue;
    if (dwNow - v->getOfflineTime() >= 86400)
      continue;

    ++dwLastOnlineCount;
  }

  return (5 * dwLastOnlineCount) * 60 * 60 + rCFG.dwDismissTime;
}

void Guild::clearNextZone()
{
  setNextZone(0);
  setZoneTime(0);

  for (auto &v : m_vecMembers)
    v->setExchangeZone(false);
}

void Guild::setBoard(QWORD qwCharID, DWORD dwZoneID, const string& board)
{
  std::string oldBoard = m_strBoard;

  m_strBoard = board;
  setMark(EGUILDDATA_BOARDINFO);
  updateData();

  GMember* pMem = getMember(qwCharID);
  if (pMem)
  {
    //plat log
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Change_GuildGlobal;
    PlatLogManager::getMe().eventLog(thisServer,
      0,
      pMem->getZoneID(),
      pMem->getAccid(),
      pMem->getCharID(),
      eid,
      0,  //charge
      eType, 0, 1);

    PlatLogManager::getMe().changeLog(thisServer,
      0,
      pMem->getZoneID(),
      pMem->getAccid(),
      pMem->getCharID(),
      eType,
      eid,
      EChange_GuildGlobal,
      oldBoard,
      m_strBoard,
      getGUID());
  }
}

void Guild::timer(DWORD curTime)
{
  checkChairman(curTime);
  updateRecord(curTime);
  updateExchangeZone(curTime);
  updateVar(curTime);

  getMisc().refreshQuest(curTime);
  getMisc().updateGiveupCity(curTime);
  getMisc().getBuilding().timer(curTime);
  getMisc().getChallenge().timer(curTime);
  getMisc().getArtifact().timer(curTime);
  getMisc().getGvg().timer(curTime);
}

void Guild::setJobName(EGuildJob eJob, const string& name)
{
  if (eJob <= EGUILDJOB_MIN || eJob >= EGUILDJOB_MAX)
    return;

  getMisc().setJobName(eJob, name);
  setMark(EGUILDDATA_MISC);
}

const string& Guild::getJobName(EGuildJob eJob)
{
  return getMisc().getJobName(eJob);
}

void Guild::updateMember()
{
  if (m_setMemberUpdate.empty() == true)
    return;

  setMark(EGUILDDATA_MEMBER);

  GuildMemberUpdateGuildCmd cmd;
  GuildMemberUpdateGuildSCmd scmd;

  fetchMember(m_setMemberUpdate, cmd, scmd);

  if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    for (auto &v : m_vecMembers)
    {
      if (v->isOnline() == false)
        continue;
      if (v->getFrameStatus() == true)
      {
        v->sendCmdToMe(send, len);
#ifdef _DEBUG
        XDBG << "[公会-成员更新]" << v->getAccid() << v->getCharID() << v->getProfession() << v->getName() << "收到" << m_qwGuildID << m_strName << "成员更新" << cmd.ShortDebugString() << XEND;
#endif
        continue;
      }
      for (auto &s : m_setMemberUpdate)
        v->addMemberUpdate(s);
    }

#ifdef _DEBUG
    XDBG << "[公会-成员更新]" << m_qwGuildID << m_strName << "同步客户端" << cmd.ShortDebugString() << XEND;
#endif
  }

  if (scmd.updates_size() > 0 || scmd.dels_size() > 0)
  {
    for (auto &v : m_vecMembers)
    {
      if (v->isOnline() == false)
        continue;

      scmd.set_charid(v->getCharID());
      PROTOBUF(scmd, ssend, slen);
      thisServer->sendCmdToZone(v->getZoneID(), ssend, slen);
    }
#ifdef _DEBUG
    XDBG << "[公会-成员更新]" << m_qwGuildID << m_strName << "同步场景" << scmd.ShortDebugString() << XEND;
#endif
  }

  if (getMisc().getCityID() != 0)
  {
    CityDataUpdateGuildSCmd ccmd;
    ccmd.set_cityid(getMisc().getCityID());
    ccmd.set_membercount(m_vecMembers.size());

    PROTOBUF(ccmd, csend, clen);
    thisServer->sendCmdToZone(getZoneID(), csend, clen);
#ifdef _DEBUG
    XDBG << "[公会-城池成员更新]" << m_qwGuildID << m_strName << "同步场景" << ccmd.ShortDebugString() << XEND;
#endif
  }

  m_setMemberUpdate.clear();
}

void Guild::fetchMember(const TSetQWORD& setIDs, GuildMemberUpdateGuildCmd& cmd, GuildMemberUpdateGuildSCmd& scmd)
{
  for (auto &s : setIDs)
  {
    GMember* pMember = getMember(s);
    if (pMember != nullptr)
    {
      pMember->toData(cmd.add_updates(), true);
      pMember->toData(scmd.add_updates());
    }
    else
    {
      cmd.add_dels(s);
      scmd.add_dels(s);
    }
  }
}

void Guild::updateApply()
{
  if (m_setApplyUpdate.empty() == true)
    return;

  setMark(EGUILDDATA_APPLY);

  GuildApplyUpdateGuildCmd cmd;
  for (auto &s : m_setApplyUpdate)
  {
    GMember* pApply = getApply(s);
    if (pApply != nullptr)
      pApply->toData(cmd.add_updates());
    else
      cmd.add_dels(s);
  }
  if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    broadcastCmd(send, len, true);
  }

  m_setApplyUpdate.clear();
}

void Guild::updateData(bool bNoCache /*= false*/)
{
  if (m_bitset.any() == false)
    return;

  GuildDataUpdateGuildCmd cmd;
  GuildDataUpdateGuildSCmd scmd;
  CityDataUpdateGuildSCmd ccmd;

  fetchData(m_bitset, cmd, scmd, ccmd);

  set<EGuildData> setDatas;
  for (int i = 0; i < EGUILDDATA_MAX; ++i)
  {
    if (m_bitset.test(i) == true)
      setDatas.insert(static_cast<EGuildData>(i));
  }
  m_bitset.reset();

  if (cmd.updates_size() > 0)
  {
    PROTOBUF(cmd, send, len);

    for (auto &m : m_vecMembers)
    {
      if (m->isOnline() == false)
        continue;
      if (bNoCache || m->getFrameStatus() == true)
      {
        m->sendCmdToMe(send, len);
#ifdef _DEBUG
        XDBG << "[公会-数据更新]" << m->getAccid() << m->getCharID() << m->getProfession() << m->getName() << "收到" << getGUID() << getName() << "数据 :" << cmd.ShortDebugString() << XEND;
#endif
        continue;
      }
      for (auto &s : setDatas)
        m->setGuildMark(s);
    }

#ifdef _DEBUG
    XDBG << "[公会-数据更新]" << getGUID() << getName() << "同步客户端数据 :" << cmd.ShortDebugString() << XEND;
#endif
  }

  if (scmd.updates_size() > 0)
  {
    for (auto &m : m_vecMembers)
    {
      if (m->isOnline() == true)
      {
        scmd.set_charid(m->getCharID());
        PROTOBUF(scmd, ssend, slen);
        thisServer->sendCmdToZone(m->getZoneID(), ssend, slen);
      }
    }
#ifdef _DEBUG
    XDBG << "[公会-数据更新]" << getGUID() << getName() << "同步客户端数据 :" << scmd.ShortDebugString() << XEND;
#endif
  }

  if (getMisc().getCityID() != 0 && ccmd.updates_size() > 0)
  {
    PROTOBUF(ccmd, csend, clen);
    thisServer->sendCmdToZone(getZoneID(), csend, clen);
#ifdef _DEBUG
    XDBG << "[公会-数据更新]" << getGUID() << getName() << "同步城池数据到SceneServer :" << ccmd.ShortDebugString() << XEND;
#endif
  }
}

void Guild::fetchData(const bitset<EGUILDDATA_MAX>& bit, GuildDataUpdateGuildCmd& cmd, GuildDataUpdateGuildSCmd& scmd, CityDataUpdateGuildSCmd& ccmd)
{
  ccmd.set_cityid(getMisc().getCityID());
  ccmd.set_membercount(m_vecMembers.size());

  for (int i = 0; i < EGUILDDATA_MAX; ++i)
  {
    if (bit.test(i) == false)
      continue;

    EGuildData type = static_cast<EGuildData>(i);
    switch (type)
    {
      case EGUILDDATA_MIN:
        break;
      case EGUILDDATA_ID:
        break;
      case EGUILDDATA_NAME:
        GGuild::add_gdata(cmd.add_updates(), type, 0, getName());
        GGuild::add_gdata(scmd.add_updates(), type, 0, getName());
        GGuild::add_gdata(ccmd.add_updates(), type, 0, getName());
        break;
      case EGUILDDATA_LEVEL:
        GGuild::add_gdata(cmd.add_updates(), type, getLevel());
        GGuild::add_gdata(scmd.add_updates(), type, getLevel());
        GGuild::add_gdata(ccmd.add_updates(), type, getLevel());
        break;
      case EGUILDDATA_BOARDINFO:
        GGuild::add_gdata(cmd.add_updates(), type, 0, getBoard());
        break;
      case EGUILDDATA_RECRUITINFO:
        GGuild::add_gdata(cmd.add_updates(), type, 0, getRecruit());
        break;
      case EGUILDDATA_PORTRAIT:
        GGuild::add_gdata(cmd.add_updates(), type, 0, getPortrait());
        GGuild::add_gdata(scmd.add_updates(), type, 0, getPortrait());
        GGuild::add_gdata(ccmd.add_updates(), type, 0, getPortrait());
        break;
      case EGUILDDATA_QUEST_RESETTIME:
        GGuild::add_gdata(cmd.add_updates(), type, getQuestTime());
        break;
      case EGUILDDATA_ASSET:
        GGuild::add_gdata(cmd.add_updates(), type, getAsset());
        break;
      case EGUILDDATA_DISMISSTIME:
        GGuild::add_gdata(cmd.add_updates(), type, getDismissTime());
        break;
      case EGUILDDATA_ZONETIME:
        GGuild::add_gdata(cmd.add_updates(), type, getZoneTime());
        break;
      case EGUILDDATA_NEXTZONE:
        GGuild::add_gdata(cmd.add_updates(), type, getClientZoneID(getNextZone()));
        break;
      case EGUILDDATA_DONATETIME1:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getDonateTime1());
        break;
      case EGUILDDATA_DONATETIME2:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getDonateTime2());
        break;
      case EGUILDDATA_DONATETIME3:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getDonateTime3());
        break;
      case EGUILDDATA_DONATETIME4:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getDonateTime4());
        break;
      case EGUILDDATA_ASSET_DAY:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getVarValue(EVARTYPE_GUILD_MAXASSET));
        break;
      case EGUILDDATA_MEMBER:
      case EGUILDDATA_APPLY:
      case EGUILDDATA_MISC:
      case EGUILDDATA_PACK:
        break;
      case EGUILDDATA_ZONEID:
        GGuild::add_gdata(cmd.add_updates(), type, getClientZoneID(getZoneID()));
        GGuild::add_gdata(scmd.add_updates(), type, getZoneID());
        break;
      case EGUILDDATA_EVENT:
      case EGUILDDATA_PHOTO:
        break;
      case EGUILDDATA_CITYID:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getCityID());
        break;
      case EGUILDDATA_CITY_GIVEUP_CD:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getCityGiveupTime());
        break;
      case EGUILDDATA_OPEN_FUNCTION:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getOpenFunction());
        GGuild::add_gdata(scmd.add_updates(), type, getMisc().getOpenFunction());
        break;
      case EGUILDDATA_TREASURE_GVG_COUNT:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getTreasure().hasTreasure() == true ? 1 : 0);
        break;
      case EGUILDDATA_TREASURE_GUILD_COUNT:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getVarValue(EVARTYPE_GUILD_TREASURE_COUNT));
        break;
      case EGUILDDATA_TREASURE_BCOIN_COUNT:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getVarValue(EVARTYPE_BCOIN_TREASURE_COUNT));
        break;
      case EGUILDDATA_SUPERGVG:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getGvg().inSuperGvg());
        GGuild::add_gdata(scmd.add_updates(), type, getMisc().getGvg().inSuperGvg());
        break;
      case EGUILDDATA_SUPERGVG_LV:
        GGuild::add_gdata(cmd.add_updates(), type, getMisc().getGvg().getSuperGvgLv());
        break;
      case EGUILDDATA_MAX:
        break;
    }
  }
}

void Guild::updateRecord(DWORD curTime)
{
  if (curTime < m_dwRecordTick)
    return;
  m_dwRecordTick = curTime + randBetween(GUILD_RECORD_TICK_TIME / 2, GUILD_RECORD_TICK_TIME);

  if (m_recordbitset.any() == false)
    return;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild");
  if (pField == nullptr)
  {
    XERR << "[公会-保存]" << getGUID() << getName() << "未取到guild数据库表" << XEND;
    return;
  }

  xSQLAction *pAct = xSQLThread::create(pField);
  if (!pAct) 
  {
    XERR << "[公会-保存]" << getGUID() << getName() << "创建xSQLAction失败" << XEND;
    return;
  }
  xRecord &record = pAct->m_oRecord;
  // xRecord record(pField);


  string data;
  for (int i = 0; i < EGUILDDATA_MAX; ++i)
  {
    if (m_recordbitset.test(i) == false)
      continue;

    EGuildData type = static_cast<EGuildData>(i);
    switch (type)
    {
      case EGUILDDATA_MIN:
        break;
      case EGUILDDATA_ID:
        break;
      case EGUILDDATA_NAME:
        record.putString("name", getName());
        break;
      case EGUILDDATA_LEVEL:
        record.put("lv", getLevel());
        break;
      case EGUILDDATA_BOARDINFO:
        record.put("board", getBoard());
        break;
      case EGUILDDATA_RECRUITINFO:
        record.put("recruit", getRecruit());
        break;
      case EGUILDDATA_PORTRAIT:
        record.put("portrait", getPortrait());
        break;
      case EGUILDDATA_QUEST_RESETTIME:
        record.put("questtime", getQuestTime());
        break;
      case EGUILDDATA_ASSET:
        record.put("conlv", getAsset());
        break;
      case EGUILDDATA_DISMISSTIME:
        record.put("dismisstime", getDismissTime());
        break;
      case EGUILDDATA_ZONETIME:
        record.put("zonetime", getZoneTime());
        break;
      case EGUILDDATA_NEXTZONE:
        record.put("nextzone", getNextZone());
        break;
      case EGUILDDATA_DONATETIME1:
      case EGUILDDATA_DONATETIME2:
      case EGUILDDATA_DONATETIME3:
      case EGUILDDATA_DONATETIME4:
      case EGUILDDATA_ASSET_DAY:
      case EGUILDDATA_TREASURE_GVG_COUNT:
      case EGUILDDATA_TREASURE_GUILD_COUNT:
      case EGUILDDATA_TREASURE_BCOIN_COUNT:
        break;
      case EGUILDDATA_MEMBER:
        if (toMemberString(data) == true)
          record.putBin("member", (unsigned char*)data.c_str(), data.size());
        break;
      case EGUILDDATA_APPLY:
        if (toApplyString(data) == true)
          record.putBin("apply", (unsigned char*)data.c_str(), data.size());
        break;
      case EGUILDDATA_MISC:
        if (toBlobMiscString(data) == true)
          record.putBin("misc", (unsigned char*)data.c_str(), data.size());
        break;
      case EGUILDDATA_PACK:
        if (toBlobPackString(data) == true)
          record.putBin("pack", (unsigned char*)data.c_str(), data.size());
        break;
      case EGUILDDATA_ZONEID:
        record.put("zoneid", getZoneID());
        break;
      case EGUILDDATA_EVENT:
        if (toBlobEventString(data) == true)
          record.putBin("event", (unsigned char*)data.c_str(), data.size());
        break;
      case EGUILDDATA_PHOTO:
        if (toBlobPhotoString(data) == true)
          record.putBin("photo", (unsigned char*)data.c_str(), data.size());
        break;
      case EGUILDDATA_CITYID:
      case EGUILDDATA_CITY_GIVEUP_CD:
      case EGUILDDATA_OPEN_FUNCTION:
      case EGUILDDATA_SUPERGVG:
      case EGUILDDATA_SUPERGVG_LV:
      case EGUILDDATA_MAX:
        break;
    }
  }

  // update to database
  if (record.m_rs.empty() == false)
  {
    char szWhere[64] = {0};
    snprintf(szWhere, sizeof(szWhere), "id=%llu", m_qwGuildID);
    pAct->m_strWhere = szWhere;
    pAct->m_eType = xSQLType_Update;
    thisServer->m_oSQLThread.add(pAct);

    /*
    QWORD ret = thisServer->getDBConnPool().exeUpdate(record, szWhere);
    if (ret == QWORD_MAX)
    {
      XERR << "[公会-保存]" << m_qwGuildID << m_strName << "公会更新数据失败" << XEND;
      return;
    }
    */
    XLOG << "[公会-保存]" << m_qwGuildID << m_strName << "更新数据成功" << XEND;
  }

  m_recordbitset.reset();
}

void Guild::checkChairman(DWORD curTime)
{
  if (curTime < m_dwChairTick)
    return;
  m_dwChairTick = curTime + randBetween(DAY_T / 2, DAY_T);

  GMember* pChairman = getChairman();
  if (pChairman == nullptr || pChairman->isOnline() == true)
    return;

  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  if (curTime - pChairman->getOfflineTime() < rCFG.dwChairOffline)
    return;

  QWORD qwCharID = pChairman->getCharID();
  DWORD dwOfflineTime = pChairman->getOfflineTime();
  string strName = pChairman->getName();

  if (exchangeChairman(0, 0, EEXCHANGEMETHOD_FIND) == false)
  {
    XERR << "[公会-会长检查]" << getGUID() << getName() << "会长" << qwCharID << strName << "已经超过" << (curTime - dwOfflineTime) / DAY_T << "天没上线, 移交会长失败" << XEND;
    return;
  }
  // 修改公告栏提示
  setBoard(0, 0, rCFG.strOfflineChChariManMsg);

  XLOG << "[公会-会长检查]" << getGUID() << getName() << "会长" << qwCharID << strName << "已经超过" << (curTime - dwOfflineTime) / DAY_T << "天没上线, 会长被移交" << XEND;
}

void Guild::updateVar(DWORD curTime)
{
  if (curTime < m_dwVarTick)
    return;

  m_dwVarTick = xTime::getDayStart(curTime) + DAY_T + randBetween(GUILD_RECORD_TICK_TIME / 2, GUILD_RECORD_TICK_TIME);
  maintenance(curTime);
  for (auto &v : m_vecMembers)
  {
    v->getWeekContribution();
    v->getWeekAsset();
    v->getWeekBCoin();
  }
}

void Guild::refreshDonate(DWORD& dwTime, DWORD curTime, EDonateType eType, QWORD qwUserId/*=0*/)
{
  if (dwTime > curTime || m_pGuildCFG == nullptr)
    return;

  //DWORD dwInterval = eType == EDONATETYPE_NORMAL ? m_pGuildCFG->dwDonateRefreshInterval1 : m_pGuildCFG->dwDonateRefreshInterval2;
  //EGuildData eData = eType == EDONATETYPE_NORMAL ? EGUILDDATA_DONATETIME1 : EGUILDDATA_DONATETIME2;
  DWORD dwInterval = 0;
  EGuildData eData = EGUILDDATA_MIN;
  switch (eType)
  {
    case EDONATETYPE_NORMAL:
      {
        dwInterval = m_pGuildCFG->dwDonateRefreshInterval1;
        eData = EGUILDDATA_DONATETIME1;
      }
      break;
    case EDONATETYPE_HIGHER:
      {
        dwInterval = m_pGuildCFG->dwDonateRefreshInterval2;
        eData = EGUILDDATA_DONATETIME2;
      }
      break;
    case EDONATETYPE_SILVER:
      {
        dwInterval = m_pGuildCFG->dwDonateRefreshInterval3;
        eData = EGUILDDATA_DONATETIME3;
      }
      break;
    case EDONATETYPE_BOSS:
      {
        dwInterval = m_pGuildCFG->dwDonateRefreshInterval4;
        eData = EGUILDDATA_DONATETIME4;
      }
      break;
    default:
      return;
  }
  dwTime = curTime + dwInterval;
  setMark(EGUILDDATA_MISC);
  setMark(eData);
  updateData();

  for (auto &v : m_vecMembers)
  {
    if (qwUserId && qwUserId != v->getCharID())
      continue;
    v->resetWeek();

    const SGuildDonateCFG* pCFG = GuildConfig::getMe().randGuildDonateCFG(v->getBaseLevel(), eType);
    if (pCFG == nullptr)
    {
      XERR << "[公会-捐赠刷新]" << "公会" << m_qwGuildID << getLevel() << m_strName << "成员" << v->getCharID() << v->getProfession() << v->getBaseLevel()
        << "刷新Type为" << eType << "物品失败" << XEND;
      return;
    }
    const SGuildDonateItem* pItem = pCFG->randDonateItem();
    if (pItem == nullptr)
    {
      XERR << "[公会-捐赠刷新]" << "公会" << m_qwGuildID << getLevel() << m_strName << "成员" << v->getCharID() << v->getProfession() << v->getBaseLevel()
        << "刷新Type为" << eType << "随机物品失败" << XEND;
      return;
    }
    v->addDonateItem(pCFG->dwID, curTime + eType + dwInterval, pItem->dwItemID, pItem->dwCount);
    XLOG << "[公会-捐赠刷新]" << "公会" << m_qwGuildID << getLevel() << m_strName << "成员" << v->getCharID() << v->getProfession() << v->getBaseLevel()
      << "刷新Type为" << eType <<"表id :" << pCFG->dwID<<"物品id"<<pItem->dwItemID<<"个数："<<pItem->dwCount << XEND;
  }
}

void Guild::updateExchangeZone(DWORD curTime)
{
  if (m_dwZoneTime == 0 || curTime < m_dwZoneTime)
    return;

  ExchangeZoneNtfGuildCmd cmd;
  cmd.set_nextzoneid(getClientZoneID(getNextZone()));
  cmd.set_curzoneid(getClientZoneID(getZoneID()));
  PROTOBUF(cmd, send, len);

  if (getMisc().getCityID() != 0)
  {
    XLOG << "[公会-搬家]" << getGUID() << getName() << "搬家至" << m_dwNextZone << "放弃城池" << getMisc().getCityID() << XEND;
    getMisc().setCityGiveupTime(xTime::getCurSec());
    getMisc().updateGiveupCity(getMisc().getCityGiveupTime() * 2);
  }

  setZoneID(m_dwNextZone);

  for (auto &v : m_vecMembers)
  {
    if (v->getZoneID() == m_dwNextZone)
      continue;

    v->setExchangeZone(true);
    v->sendCmdToMe(send, len);
  }

  setNextZone(0);
  setZoneTime(0);

  //broadcastMsg(ESYSTEMMSG_ID_GUILD_EXCHANGEZONE_DONE, MsgParams(getClientZoneID(getZoneID())));
}

void Guild::processRedTip(bool addMember)
{
  if(addMember == true)
  {
    for (auto &v : m_vecMembers)
    {
      if(getMisc().hasAuth(v->getJob(), EAUTH_AGREE))
        v->setRedTip(true);
    }
    XLOG << "[公会-红点] 添加" << getGUID() << getName() << getLevel() << addMember << XEND;
  }
  else if(m_vecApplys.empty() == true)
  {
    for (auto &v : m_vecMembers)
    {
      if(getMisc().hasAuth(v->getJob(), EAUTH_AGREE))
        v->setRedTip(false);
    }
    XLOG << "[公会-红点] 删除" << getGUID() << getName() << getLevel() << addMember << XEND;
  }
}

void Guild::rename(const string& str)
{
  m_strName = str;
  setRenameTime(xTime::getCurSec());
  setMark(EGUILDDATA_NAME);
  updateData();
}

const string& Guild::getRealtimeVoiceID()
{
  if (m_strRealtimeVoiceID.empty())
  {
    m_strRealtimeVoiceID = RealtimeVoiceManager::getMe().getID();
    if (m_strRealtimeVoiceID.empty())
    {
      stringstream ss;
      ss << getGUID();
      m_strRealtimeVoiceID = ss.str();
    }
  }
  return m_strRealtimeVoiceID;
}

DWORD Guild::getOpenedRealtimeVoiceCount()
{
  DWORD count = 0;
  for (auto& v : m_vecMembers)
    if (v->isRealtimeVoiceOpen())
      ++count;
  return count;
}

bool Guild::canAddRealtimeVoice()
{
  return getOpenedRealtimeVoiceCount() < MiscConfig::getMe().getGuildCFG().dwRealtimeVoiceLimit;
}

bool Guild::openRealtimeVoice(QWORD operatorid, QWORD memberid, bool open)
{
  GMember* pOperator = getMember(operatorid);
  if (pOperator == nullptr)
  {
    XERR << "[公会-开启实时语音]" << getGUID() << getName() << "成员:" << operatorid << "成员找不到" << XEND;
    return false;
  }
  if (pOperator->getJob() != EGUILDJOB_CHAIRMAN && pOperator->getJob() != EGUILDJOB_VICE_CHAIRMAN)
  {
    XERR << "[公会-开启实时语音]" << getGUID() << getName() << "成员:" << operatorid << "不是会长/副会长" << XEND;
    return false;
  }
  if (open && canAddRealtimeVoice() == false)
  {
    XERR << "[公会-开启实时语音]" << getGUID() << getName() << "成员:" << operatorid << "不可添加更多有语音权限的成员" << XEND;
    return false;
  }
  GMember* pMember = getMember(memberid);
  if (pMember == nullptr)
  {
    XERR << "[公会-开启实时语音]" << getGUID() << getName() << "操作对象:" << memberid << "找不到" << XEND;
    return false;
  }
  if (pMember->openRealtimeVoice(open) == false)
  {
    XERR << "[公会-开启实时语音]" << getGUID() << getName() << "操作者:" << operatorid << "操作对象:" << memberid << "open:" << open << "设置失败" << XEND;
    return false;
  }
  XLOG << "[公会-开启实时语音]" << getGUID() << getName() << "操作者:" << operatorid << "操作对象:" << memberid << "open:" << open << "设置成功" << XEND;
  return true;
}
