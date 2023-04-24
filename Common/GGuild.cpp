#include "GGuild.h"
#include "xLog.h"
#include "xTools.h"

GGuild::GGuild()
{
}

GGuild::~GGuild()
{

}

bool GGuild::add_gdata(GuildDataUpdate* pData, EGuildData eType, QWORD qwValue, const string& strData /*= ""*/)
{
  if (pData == nullptr)
    return false;
  pData->set_type(eType);
  pData->set_value(qwValue);
  pData->set_data(strData);
  return true;
}

bool GGuild::add_mdata(GuildMemberDataUpdate* pData, EGuildMemberData eType, QWORD qwValue, const string& strData /*= ""*/)
{
  if (pData == nullptr)
    return false;
  pData->set_type(eType);
  pData->set_value(qwValue);
  pData->set_data(strData);
  return true;
}

bool GGuild::isSame(const GuildPhoto& r1, const GuildPhoto& r2)
{
  return r1.accid_svr() == r2.accid_svr() && r1.charid() == r2.charid() && r1.source() == r2.source() && r1.sourceid() == r2.sourceid();
}

bool GGuild::hasAuth(DWORD dwAuth, EAuth eAuth)
{
  if (eAuth <= EAUTH_MIN || eAuth >= EAUTH_MAX)
    return false;
  DWORD dwValue = 1 << (eAuth - 1);
  return (dwValue & dwAuth) != 0;
}

string GGuild::getPhotoGUID(const GuildPhoto& r)
{
  stringstream sstr;
  sstr << ":" << r.accid_svr() << ":" << r.charid() << ":" << r.source() << ":" << r.sourceid();
  return sstr.str();
}

bool GGuild::toData(GuildInfo* pInfo)
{
  if (pInfo == nullptr || !m_bInfoInit)
    return false;

  pInfo->CopyFrom(m_oInfo);

  pInfo->clear_members();
  for (auto &m : m_mapMember)
    pInfo->add_members()->CopyFrom(m.second);

  pInfo->clear_quests();
  for (auto &m : m_mapQuest)
    pInfo->add_quests()->CopyFrom(m.second);

  pInfo->mutable_building()->clear_buildings();
  for (auto &m : m_mapBuilding)
    pInfo->mutable_building()->add_buildings()->CopyFrom(m.second);

  pInfo->clear_artifactitems();
  for (auto &m : m_mapArtifact)
    pInfo->add_artifactitems()->CopyFrom(m.second);

  pInfo->mutable_artifacequest()->Clear();
  for (auto &s : m_setGQuest)
    pInfo->mutable_artifacequest()->add_submitids(s);
  for (auto &m : m_mapPack)
    pInfo->mutable_artifacequest()->add_datas()->CopyFrom(m.second);
  return true;
}

bool GGuild::toData(GuildUserInfo* pInfo)
{
  if (pInfo == nullptr || !m_bUserInfoInit)
    return false;
  pInfo->CopyFrom(m_oUserInfo);
  return true;
}

bool GGuild::toData(GuildArtifactQuest* pInfo)
{
  if (pInfo == nullptr || !m_bInfoInit)
    return false;

  pInfo->clear_submitids();
  for (auto &s : m_setGQuest)
    pInfo->add_submitids(s);

  pInfo->clear_datas();
  for (auto &m : m_mapPack)
    pInfo->add_datas()->CopyFrom(m.second);
  return true;
}

const GuildSMember* GGuild::getChairman() const
{
  for (auto &m : m_mapMember)
  {
    if (m.second.job() == EGUILDJOB_CHAIRMAN)
      return &m.second;
  }
  return nullptr;
}

const GuildSMember* GGuild::getMember(QWORD id, QWORD accid /*= 0*/) const
{
  auto m = m_mapMember.find(id);
  if (m != m_mapMember.end())
    return &m->second;
  if (accid != 0)
  {
    for (auto &acc : m_mapMember)
    {
      if (acc.second.accid() == accid)
        return &acc.second;
    }
  }
  return nullptr;
}

const GuildSMember* GGuild::randMember(const TSetQWORD& setExclude /*= TSetQWORD{}*/) const
{
  DWORD dwDayStart = xTime::getDayStart(xTime::getCurSec());
  TSetQWORD setIDs;
  for (auto &m : m_mapMember)
  {
    auto s = find(setExclude.begin(), setExclude.end(), m.first);
    if (s != setExclude.end())
      continue;
    if (GGuild::isActive(dwDayStart, m.second.onlinetime(), m.second.offlinetime()) == false)
      continue;
    if (m_qwAccID == m.second.accid() && m_qwCharID != m.second.charid())
      continue;
    setIDs.insert(m.first);
  }
  if (setIDs.empty() == true)
    return nullptr;

  QWORD* p = randomStlContainer(setIDs);
  if (p == nullptr)
    return nullptr;

  auto m = m_mapMember.find(*p);
  if (m == m_mapMember.end())
    return nullptr;

  return &m->second;
}

const GuildQuest* GGuild::getQuest(DWORD dwQuestID) const
{
  auto m = m_mapQuest.find(dwQuestID);
  if (m != m_mapQuest.end())
    return &m->second;
  return nullptr;
}

void GGuild::updateInfo(const GuildInfo& rInfo)
{
  m_oInfo.CopyFrom(rInfo);

  m_mapMember.clear();
  for (int i = 0; i < m_oInfo.members_size(); ++i)
  {
    const GuildSMember& rMember = m_oInfo.members(i);
    m_mapMember[rMember.charid()] = rMember;
  }

  m_mapQuest.clear();
  for (int i = 0; i < m_oInfo.quests_size(); ++i)
  {
    const GuildQuest& rQuest = m_oInfo.quests(i);
    m_mapQuest[rQuest.questid()] = rQuest;
  }

  m_mapBuilding.clear();
  for (int i = 0; i < m_oInfo.building().buildings_size(); ++i)
  {
    const GuildBuilding& rBuilding = m_oInfo.building().buildings(i);
    m_mapBuilding[rBuilding.type()] = rBuilding;
  }

  m_mapArtifact.clear();
  for (int i = 0; i < m_oInfo.artifactitems_size(); ++i)
  {
    const GuildArtifactItem& rArtifactItem = m_oInfo.artifactitems(i);
    m_mapArtifact[rArtifactItem.guid()] = rArtifactItem;
  }

  m_setGQuest.clear();
  for (int i = 0; i < m_oInfo.artifacequest().submitids_size(); ++i)
    m_setGQuest.insert(m_oInfo.artifacequest().submitids(i));

  m_mapPack.clear();
  for (int i = 0; i < m_oInfo.artifacequest().datas_size(); ++i)
  {
    const ItemData& rData = m_oInfo.artifacequest().datas(i);
    m_mapPack[rData.base().id()].CopyFrom(rData);
  }
}

void GGuild::updateGuild(const GuildInfoSyncGuildSCmd& cmd)
{
  updateInfo(cmd.info());
  m_bInfoInit = true;
  XDBG << "[GGuild-公会]" << cmd.charid() << "同步数据" << cmd.ShortDebugString() << XEND;
}

void GGuild::updateGuildData(const GuildDataUpdateGuildSCmd& cmd)
{
  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const GuildDataUpdate& rData = cmd.updates(i);
    switch (rData.type())
    {
      case EGUILDDATA_LEVEL:
        m_oInfo.set_lv(rData.value());
        break;
      case EGUILDDATA_PORTRAIT:
        m_oInfo.set_portrait(rData.data());
        break;
      case EGUILDDATA_OPEN_FUNCTION:
        m_oInfo.set_openfunction(rData.value());
        break;
      case EGUILDDATA_ZONEID:
        m_oInfo.set_zoneid(rData.value());
        break;
      case EGUILDDATA_SUPERGVG:
        m_oInfo.mutable_gvg()->set_insupergvg(rData.value() != 0);
        break;
      default:
        XERR << "[GGuild-数据更新]" << cmd.charid() << "公会" << m_oInfo.id() << m_oInfo.name() << "更新数据" << rData.ShortDebugString() << "未处理" << XEND;
        break;
    }
  }
  XLOG << "[GGuild-数据更新]" << cmd.charid() << "公会" << m_oInfo.id() << m_oInfo.name() << "更新数据" << cmd.ShortDebugString() << XEND;
}

void GGuild::updateMember(const GuildMemberUpdateGuildSCmd& cmd)
{
  // update
  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const GuildSMember& rMember = cmd.updates(i);
    m_mapMember[rMember.charid()].CopyFrom(rMember);
  }

  // dels
  for (int i = 0; i < cmd.dels_size(); ++i)
  {
    auto m = m_mapMember.find(cmd.dels(i));
    if (m != m_mapMember.end())
      m_mapMember.erase(m);
  }

  XDBG << "[GGuild-成员更新]" << cmd.charid() << "公会" << m_oInfo.id() << m_oInfo.name() << "更新成员" << cmd.ShortDebugString() << XEND;
}

void GGuild::updateMemberData(const GuildMemberDataUpdateGuildSCmd& cmd)
{
  auto m = m_mapMember.find(cmd.destid());
  if (m == m_mapMember.end())
    return;

  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const GuildMemberDataUpdate& rUpdate = cmd.updates(i);
    switch (rUpdate.type())
    {
      case EGUILDMEMBERDATA_ONLINETIME:
        m->second.set_onlinetime(rUpdate.value());
        break;
      case EGUILDMEMBERDATA_OFFLINETIME:
        m->second.set_offlinetime(rUpdate.value());
        break;
      case EGUILDMEMBERDATA_JOB:
        m->second.set_job(static_cast<EGuildJob>(rUpdate.value()));
        break;
      case EGUILDMEMBERDATA_BUILDINGEFFECT:
        m->second.set_buildingeffect(rUpdate.value());
        break;
      default:
        XERR << "[GGuild-成员数据]" << cmd.charid() << "公会" << m_oInfo.id() << m_oInfo.name() << "更新成员" << m->first << "数据" << rUpdate.ShortDebugString() << "未被处理" << XEND;
        break;
    }
  }

  XDBG << "[GGuild-成员数据]" << cmd.charid() << "公会" << m_oInfo.id() << m_oInfo.name() << "更新成员数据" << cmd.ShortDebugString() << XEND;
}

void GGuild::updateQuest(const GuildQuestUpdateGuildSCmd& cmd)
{
  for (int i = 0; i < cmd.dels_size(); ++i)
  {
    auto m = m_mapQuest.find(cmd.dels(i));
    if (m != m_mapQuest.end())
      m_mapQuest.erase(m);
  }

  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const GuildQuest& rQuest = cmd.updates(i);
    m_mapQuest[rQuest.questid()].CopyFrom(rQuest);
  }

  XDBG << "[GGuild-公会任务]" << m_oInfo.id() << m_oInfo.name() << "更新任务数据";
  for (auto &m : m_mapQuest)
    XDBG << m.second.ShortDebugString();
  XDBG << XEND;
}

void GGuild::updateUserInfo(const GuildUserInfoSyncGuildCmd& cmd)
{
  m_oUserInfo.CopyFrom(cmd.info());
  m_bUserInfoInit = true;
  XLOG << "[GGuild-个人数据]" << cmd.charid() << "收到公会" << m_oInfo.id() << m_oInfo.name() << "个人数据" << cmd.ShortDebugString() << XEND;
}

void GGuild::updateJob(const JobUpdateGuildSCmd& cmd)
{
  for (auto &m : m_mapMember)
  {
    if (m.second.job() == cmd.job().job() && m.first == m_qwCharID)
    {
      m.second.set_auth(cmd.job().auth());
      m_oInfo.set_jobname(cmd.job().name());
      m_oInfo.set_auth(cmd.job().auth());
      XDBG << "[GGuild-公会职位]" << m_oInfo.id() << m_oInfo.name() << "charid :" << m_qwCharID << "更新职位信息" << cmd.job().ShortDebugString() << XEND;
      break;
    }
  }
}

void GGuild::updateBuilding(const BuildingUpdateGuildSCmd& cmd)
{
  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const GuildBuilding& rBuilding = cmd.updates(i);
    m_mapBuilding[rBuilding.type()].CopyFrom(rBuilding);
  }

  XDBG << "[GGuild-公会建筑]" << m_oInfo.id() << m_oInfo.name() << "更新建筑数据";
  for (auto &m : m_mapBuilding)
    XDBG << m.second.ShortDebugString();
  XDBG << XEND;
}

DWORD GGuild::getBuildingLevel(EGuildBuilding type) const
{
  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return 0;
  return it->second.level();
}

DWORD GGuild::getBuildingNum(DWORD lv) const
{
  DWORD count = 0;
  for(int i = EGUILDBUILDING_VENDING_MACHINE; i < EGUILDBUILDING_MAX; ++i)
  {
    if(getBuildingLevel(static_cast<EGuildBuilding>(i)) >= lv)
      count++;
  }

  return count;
}

const GuildBuilding* GGuild::getBuilding(EGuildBuilding type) const
{
  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return nullptr;
  return &it->second;
}

void GGuild::updateArtifact(const ArtifactUpdateGuildSCmd& cmd)
{
  for (int i = 0; i < cmd.itemupdates_size(); ++i)
  {
    if (m_mapArtifact.find(cmd.itemupdates(i).guid()) == m_mapArtifact.end())
      m_setNewArtifact.insert(cmd.itemupdates(i).guid());
    m_mapArtifact[cmd.itemupdates(i).guid()] = cmd.itemupdates(i);
  }
  for (int i = 0; i < cmd.itemdels_size(); ++i)
  {
    auto it = m_mapArtifact.find(cmd.itemdels(i));
    if (it != m_mapArtifact.end())
      m_mapArtifact.erase(it);
  }
}

void GGuild::updateGQuest(const GuildArtifactQuestGuildSCmd& cmd)
{
  m_setGQuest.clear();
  for (int i = 0; i < cmd.quest().submitids_size(); ++i)
    m_setGQuest.insert(cmd.quest().submitids(i));

  m_mapPack.clear();
  for (int i = 0; i < cmd.quest().datas_size(); ++i)
    m_mapPack[cmd.quest().datas(i).base().id()].CopyFrom(cmd.quest().datas(i));
  XDBG << "[GGuild-公会] 任务更新" << cmd.ShortDebugString() << XEND;
}

const GuildArtifactItem* GGuild::getArtifact(const string& guid) const
{
  auto it = m_mapArtifact.find(guid);
  if (it == m_mapArtifact.end())
    return nullptr;
  return &it->second;
}

const ItemData* GGuild::getArtifactPiece(DWORD dwID) const
{
  auto m = m_mapPack.find(dwID);
  if (m != m_mapPack.end())
    return &m->second;
  return nullptr;
}

bool GGuild::isUserOwnArtifact(QWORD charid, const string& guid) const
{
  if (charid == 0)
    return false;
  const GuildArtifactItem* artifact = getArtifact(guid);
  if (artifact == nullptr)
    return false;
  return artifact->ownerid() == charid;
}
