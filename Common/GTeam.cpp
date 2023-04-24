#include "GTeam.h"
#include "xLog.h"

GTeam::GTeam()
{

}

GTeam::~GTeam()
{

}

bool GTeam::add_mdata(MemberData* pData, EMemberData eType, QWORD qwValue, const string& strData /*= ""*/)
{
  if (pData == nullptr)
    return false;
  pData->set_type(eType);
  pData->set_value(qwValue);
  pData->set_data(strData);
  return true;
}

bool GTeam::toData(TeamInfo* pInfo)
{
  if (pInfo == nullptr || !m_bInfoInit)
    return false;

  pInfo->set_teamid(getTeamID());
  pInfo->set_leaderid(getLeaderID());
  pInfo->set_pickupmode(getPickupMode());
  pInfo->set_name(getName());
  for (auto &m : getTeamMemberList(true))
    pInfo->add_member()->CopyFrom(m.second);
  return true;
}

void GTeam::clear()
{
  m_qwTeamID = 0;
  m_qwLeaderID = 0;
  m_dwPickupMode = 0;
  m_mapMember.clear();
  m_mapMemberNoCat.clear();
}

void GTeam::updateTeam(const TeamDataSyncTeamCmd& cmd)
{
  clear();
  const TeamInfo& rInfo = cmd.info();

  m_qwTeamID = rInfo.teamid();
  m_qwLeaderID = rInfo.leaderid();
  m_qwTrueLeaderID = rInfo.leaderid();
  m_dwPickupMode = rInfo.pickupmode();
  if (!rInfo.name().empty())
    m_strName = rInfo.name();

  for (int i = 0; i < rInfo.member_size(); ++i)
  {
    const TeamMemberInfo& rMember = rInfo.member(i);
    m_mapMember[rMember.charid()].CopyFrom(rMember);
  }

  m_bInfoInit = true;
  updateNoCatMembers();
  XDBG << "[GTeam-队伍同步]" << m_qwCharID<<"队伍名字"<<m_strName << "同步数据" << cmd.ShortDebugString() << XEND;
}

void GTeam::updateTeamData(const TeamDataUpdateTeamCmd& cmd)
{
  for (int i = 0; i < cmd.datas_size(); ++i)
  {
    const TeamSummaryItem& rItem = cmd.datas(i);

    switch (rItem.type())
    {
      case ETEAMDATA_PICKUP_MODE:
        m_dwPickupMode = rItem.value();
        break;
      default:
        XERR << "[GTeam-队伍数据更新]" << m_qwCharID << "更新队伍数据 type :" << rItem.type() << "value :" << rItem.value() << "未处理" << XEND;
        break;
    }
  }

  XDBG << "[GTeam-队伍数据更新]" << m_qwCharID << "更新队伍数据" << cmd.ShortDebugString() << XEND;
}

void GTeam::updateMember(const TeamMemberUpdateTeamCmd& cmd)
{
  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const TeamMemberInfo& rMember = cmd.updates(i);
    m_mapMember[rMember.charid()].CopyFrom(rMember);
  }
  for (int i = 0; i < cmd.dels_size(); ++i)
  {
    auto m = m_mapMember.find(cmd.dels(i));
    if (m != m_mapMember.end())
      m_mapMember.erase(m);
  }

  updateNoCatMembers();
  XDBG << "[GTeam-队伍成员更新]" << m_qwCharID << "更新队伍成员" << cmd.ShortDebugString() << XEND;
}

void GTeam::updateMemberData(const MemberDataUpdateTeamCmd& cmd)
{
  auto m = m_mapMember.find(cmd.updatecharid());
  if (m == m_mapMember.end())
    return;

  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const MemberData& rData = cmd.updates(i);
    switch (rData.type())
    {
      case EMEMBERDATA_MAPID:
        m->second.set_mapid(rData.value());
        break;
      case EMEMBERDATA_RAIDID:
        m->second.set_raidid(rData.value());
        break;
      case EMEMBERDATA_JOB:
        if (rData.value() == ETEAMJOB_LEADER || rData.value() == ETEAMJOB_TEMPLEADER)
          m_qwLeaderID = cmd.updatecharid();
        if (rData.value() == ETEAMJOB_LEADER)
          m_qwTrueLeaderID = cmd.updatecharid();
        else if (rData.value() == ETEAMJOB_TEMPLEADER)
          m_qwTempLeaderID = cmd.updatecharid();
        else
        {
          if (m_qwTrueLeaderID == cmd.updatecharid())
            m_qwTrueLeaderID = 0;
          if (m_qwTempLeaderID == cmd.updatecharid())
            m_qwTempLeaderID = 0;
        }
        break;
      case EMEMBERDATA_GUILDRAIDINDEX:
        m->second.set_guildraidindex(rData.value());
        break;
      case EMEMBERDATA_BASELEVEL:
        m->second.set_level(rData.value());
        break;
      default:
        XERR << "[GTeam-队伍成员数据更新]" << m_qwCharID << "更新队伍成员数据 type :" << rData.type() << "value :" << rData.value() << "未处理" << XEND;
        break;
    }
  }

  updateNoCatMembers();
  XDBG << "[GTeam-队伍成员数据更新]" << m_qwCharID <<"m_qwTrueLeaderID"<< m_qwTrueLeaderID <<"m_qwTempLeaderID"<< m_qwTempLeaderID << "更新队伍成员数据" << cmd.ShortDebugString() << XEND;
}

const TeamMemberInfo* GTeam::getTeamMember(QWORD qwCharID) const
{
  auto m = m_mapMember.find(qwCharID);
  if (m != m_mapMember.end())
    return &m->second;
  return nullptr;
}

void GTeam::updateNoCatMembers()
{
  m_mapMemberNoCat.clear();
  for (auto &m : m_mapMember)
  {
    if (m.second.catid() != 0)
      continue;
    m_mapMemberNoCat[m.first].CopyFrom(m.second);
  }
}

const string& GTeam::getLeaderName() const
{
  for (auto &m : m_mapMemberNoCat)
  {
    if (m.first == m_qwLeaderID)
      return m.second.name();
  }
  static const string emptystr;
  return emptystr;
}

