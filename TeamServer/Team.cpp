#include "Team.h"
#include "xCommand.h"
#include "MiscConfig.h"
#include "xTime.h"
#include "RedisManager.h"
#include "RecordCmd.pb.h"
#include "DojoConfig.h"
#include "PlatLogManager.h"
#include "MsgManager.h"
#include "TeamServer.h"
#include "xlib/xSha1.h"
#include "TableManager.h"
#include "QuestConfig.h"
#include "GTeam.h"
#include "TowerConfig.h"
#include "MatchSCmd.pb.h"
#include "MatchCCmd.pb.h"
#include "TeamDataThread.h"
#include "PveCard.pb.h"
#include "PveCardConfig.h"

// team
Team::Team(QWORD guid, DWORD zoneid) : m_qwGUID(guid), m_dwZoneID(zoneid)
{
  set_id(guid);
}

Team::~Team()
{

}

bool Team::add(TMember *p)
{
  if (!p) return false;

  auto it = m_mapMembers.find(p->getGUID());
  if (it != m_mapMembers.end())
    return false;

  m_mapMembers[p->getGUID()] = p;
  return true;
}

void Team::del(TMember *p)
{
  if (!p) return;

  m_mapMembers.erase(p->getGUID());
  SAFE_DELETE(p);
}

bool Team::fromMemberData(const string& str)
{
  BlobTeamMember oBlob;
  if (oBlob.ParseFromString(str) == false)
    return false;

  for (int i = 0; i < oBlob.members_size(); ++i)
  {
    TMember *p = NEW TMember(this, oBlob.members(i));
    if (!p) continue;

    if (!add(p))
    {
      SAFE_DELETE(p);
    }
  }

  return true;
}

bool Team::toMemberData(string& str)
{
  BlobTeamMember oBlob;

  for (auto v = m_mapMembers.begin(); v != m_mapMembers.end(); ++v)
  {
    if (!v->second) continue;

    TeamMember* pMember = oBlob.add_members();
    if (pMember == nullptr)
    {
      XERR << "[队伍-成员保存] teamid :" << m_qwGUID << "user :" << v->second->getName() << "create protobuf error!" << XEND;
      continue;
    }

    v->second->toData(pMember);
  }

  if (oBlob.SerializeToString(&str) == false)
    return false;
  return true;
}

bool Team::fromData(const xRecord& record)
{
  m_qwGUID = record.get<QWORD>("id");

  m_strName = record.getString("name");
  m_dwMinLv = record.get<DWORD>("minlv");
  m_dwMaxLv = record.get<DWORD>("maxlv");

  m_dwOverTime = record.get<DWORD>("timer");
  m_eAutoType = static_cast<EAutoType>(record.get<DWORD>("autoaccept"));
  m_dwPickUpMode = record.get<DWORD>("pickupmode");

  string member;
  member.assign((const char *)record.getBin("member"), record.getBinSize("member"));
  if (fromMemberData(member) == false)
  {
    XERR << "[队伍-加载]" << getGUID() << getName() << "成员序列化失败" << XEND;
    return false;
  }
  string memberquest;
  memberquest.assign((const char*)record.getBin("wantedquest"), record.getBinSize("wantedquest"));
  if (fromWantedQuestData(memberquest) == false)
  {
    XERR << "[队伍-看板任务信息加载]" << getGUID() << getName() << "读取数据库失败" << XEND;
    return false;
  }

  clearUpdate();

  return true;
}

bool Team::toData(xRecord& record)
{
  if (getTeamType() == nullptr)
    return false;

  record.put("zoneid", getZoneID());
  record.put("id", getGUID());
  record.put("name", getName());
  record.put("teamtype", getTeamType()->id);
  record.put("minlv", getMinReqLv());
  record.put("maxlv", getMaxReqLv());
  record.put("timer", getOverTime());
  record.put("autoaccept", getAuto());
  record.put("pickupmode", getPickupMode());

  string str;
  if (toMemberData(str) == false)
    return false;
  record.putBin("member", (unsigned char*)str.c_str(), str.size());

  string queststr;
  if (toWantedQuestData(queststr) == false)
    return false;
  record.putBin("wantedquest", (unsigned char*)queststr.c_str(), queststr.size());

  return true;
}

void Team::toSummary(TeamData* pSummary)
{
  if (pSummary == nullptr)
    return;

  pSummary->set_guid(m_qwGUID);
  pSummary->set_name(m_strName);

  TeamSummaryItem* pData = pSummary->add_items();
  if (pData != nullptr)
  {
    pData->set_type(ETEAMDATA_TYPE);
    pData->set_value(m_pTeamCFG->id);
  }

  pData = pSummary->add_items();
  if (pData != nullptr)
  {
    pData->set_type(ETEAMDATA_MINLV);
    pData->set_value(m_dwMinLv);
  }

  pData = pSummary->add_items();
  if (pData != nullptr)
  {
    pData->set_type(ETEAMDATA_MAXLV);
    pData->set_value(m_dwMaxLv);
  }

  pData = pSummary->add_items();
  if (pData != nullptr)
  {
    pData->set_type(ETEAMDATA_AUTOACCEPT);
    pData->set_value(static_cast<DWORD>(m_eAutoType));
  }

  pData = pSummary->add_items();
  if (pData != nullptr)
  {
    pData->set_type(ETEAMDATA_MEMBERCOUNT);
    pData->set_value(getMemberCount(false));
  }
  pData = pSummary->add_items();
  if (pData != nullptr)
  {
    pData->set_type(ETEAMDATA_PICKUP_MODE);
    pData->set_value(static_cast<DWORD>(m_dwPickUpMode));
  }
}

void Team::toData(TeamData* pData)
{
  if (pData == nullptr)
    return;

  toSummary(pData);

  for (auto v = m_mapMembers.begin(); v != m_mapMembers.end(); ++v)
  {
    if (v->second)
      v->second->toData(pData->add_members());
  }

  for (auto v = m_mapApplys.begin(); v != m_mapApplys.end(); ++v)
  {
    if (v->second)
      v->second->toData(pData->add_applys());
  }
}

void Team::toData(TeamInfo* pInfo)
{
  if (pInfo == nullptr)
    return;

  pInfo->set_teamid(getGUID());
  if (getLeader() != nullptr)
    pInfo->set_leaderid(getLeader()->getGUID());
  pInfo->set_pickupmode(getPickupMode());

  for (auto &v : m_mapMembers)
  {
    if (!v.second) continue;
    TeamMemberInfo* pMember = pInfo->add_member();
    if (pMember != nullptr)
    {
      pMember->set_charid(v.second->getGUID());
      pMember->set_mapid(v.second->getMapID());
      pMember->set_zoneid(v.second->getZoneID());
      pMember->set_gender(v.second->getGender());
      pMember->set_name(v.second->getName());
      pMember->set_guildraidindex(v.second->m_dwGuildRaidIndex);
    }
  }
}

void Team::toData(TeamDataSyncTeamCmd& cmd)
{
  TeamInfo* pInfo = cmd.mutable_info();
  pInfo->set_teamid(getGUID());
  pInfo->set_pickupmode(getPickupMode());
  pInfo->set_name(m_strName);

  TMember* pLeader = getLeader();
  if (pLeader != nullptr)
    pInfo->set_leaderid(pLeader->getGUID());

  for (auto &m : m_mapMembers)
    m.second->toData(pInfo->add_member());
}

void Team::toClientData(TeamData* pData, bool bLeader)
{
  if (pData == nullptr)
    return;

  toSummary(pData);

  for (auto v = m_mapMembers.begin(); v != m_mapMembers.end(); ++v)
  {
    if (v->second)
    {
      v->second->toData(pData->add_members(), true);
      XDBG << "EnterTeam" << getGUID() << getName() << "charid :" << v->second->getGUID() << XEND;
    }
  }

  if (bLeader)
  {
    for (auto v = m_mapApplys.begin(); v != m_mapApplys.end(); ++v)
    {
      if (v->second)
        v->second->toData(pData->add_applys());
    }
  }
}

void Team::toSummaryData(TeamData* pData)
{
  if (pData == nullptr)
    return;

  toSummary(pData);

  TMember* pLeader = getLeader();
  if (pLeader != nullptr)
  {
    TeamMember* pMember = pData->add_members();
    pLeader->toData(pMember);
  }
}

void Team::toSceneData(TeamData* pData)
{
  if (pData == nullptr)
    return;

  toSummary(pData);

  for (auto v = m_mapMembers.begin(); v != m_mapMembers.end(); ++v)
  {
    if (v->second)
      v->second->toData(pData->add_members());
  }
}

void Team::toMatchServerData(TeamData* pData)
{

}

bool Team::canAutoEnter(const SocialUser& rUser)
{
  if (m_eAutoType == EAUTOTYPE_CLOSE || m_eAutoType == EAUTOTYPE_MAX)
    return false;
  if (rUser.baselv() < getMinReqLv() || rUser.baselv() > getMaxReqLv())
    return false;

  bool canEnter = false;
  if (m_eAutoType == EAUTOTYPE_GUILDFRIEND)
  {
    TMember* pLeader = getLeader();
    if (pLeader == nullptr || pLeader->isOnline() == false)
    {
      pLeader = getTempLeader();
      if (pLeader == nullptr || pLeader->isOnline() == false)
        return false;
    }
    if (pLeader->checkRelation(rUser.charid(), ESOCIALRELATION_FRIEND) == false)
    {
      GCharReader pDestChar(thisServer->getRegionID(), rUser.charid());
      if (!pDestChar.getByTeam())
        return false;
      if (pLeader->getGuildID() == 0 || pDestChar.getGuildID() == 0 || pLeader->getGuildID() != pDestChar.getGuildID())
        return false;
    }
    canEnter = true;
  }
  else if (m_eAutoType == EAUTOTYPE_ALL)
  {
    canEnter = true;
  }

  if (!canEnter)
    return false;

  if (getMemberCount() >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
  {
    removeCat(ECATREMOVEMETHOD_EARLY);
    if (getMemberCount() >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
      return false;
  }

  return true;
}

bool Team::addMember(TMember *pMember)
{
  if (!pMember) return false;
  if (pMember->getZoneID() == 0 || pMember->getGUID() == 0)
    return false;

  //check is in tower
  if (pMember->getCatID() == 0)
  {
    //不是猫    
    for (auto &m : m_mapMembers)
    {
      if (m.second && m.second->isOnline() && TowerConfig::getMe().isTower(m.second->getMapID()))
      {
        //send msg to leader
        TMember* pLeader = getLeader();
        if (pLeader && pLeader->isOnline())
        {
          thisServer->sendMsg(pLeader->getZoneID(), pLeader->getGUID(), 1317);      //队伍正在挑战无限塔，暂时无法加入队员
        }
        XERR << "[组队-加入] 队伍有成员在挑战无限塔，无法加入成员" <<"teamid"<< id << "addcharid"<<pMember->getGUID() <<"charid" << m.second->getGUID()<<"在无限塔" << "mapid" << m.second->getMapID() << XEND;
        return false;
      }      
    }   
  }

  removeApply(pMember->getGUID(), false);

  if (!add(pMember))
    return false;

  //if (pMember->getCatID() == 0)
  //  pMember->m_oGCharData.getByTeam();

  if (pMember->m_eJob == ETEAMJOB_MIN)
    pMember->m_eJob = ETEAMJOB_MEMBER;
  pMember->m_dwCreateTime = xTime::getCurSec();
  m_setMemberUpdate.insert(pMember->getGUID());

  if (pMember->getCatID() == 0 && pMember->isOnline() == true)
  {
    EnterTeam cmd;
    toClientData(cmd.mutable_data(), pMember->isLeader());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(pMember->getZoneID(), pMember->getGUID(), send, len);

    TeamDataSyncTeamCmd scmd;
    scmd.set_charid(pMember->getGUID());
    toData(scmd);
    PROTOBUF(scmd, ssend, slen);
    thisServer->sendCmdToSession(ssend, slen);
  }

  setDataMark(ETEAMDATA_MEMBER);

  broadcastMsg(317, MsgParams(pMember->getName()));

  DWORD msgId = m_dwPickUpMode == 0 ? 337 : 338;
  thisServer->sendMsg(pMember->getZoneID(), pMember->getGUID(), msgId);

  queryMemberQuestList(pMember->getGUID());

  pMember->sendRealtimeVoiceID();

  if (isInMatchTeamPws())
    cancelMatchPws();

  return true;
}

bool Team::removeMember(QWORD id)
{
  auto v = m_mapMembers.find(id);
  if (v == m_mapMembers.end())
    return false;

  if (!v->second) return false;

  TMember *pMember = v->second;

  if (v->second->isOnline() == true)
  {
    TeamDataSyncTeamCmd cmd;
    cmd.set_charid(pMember->getGUID());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }

  //bool bLeader = pMember->isLeader();
  //ETeamJob eJob = pMember->getJob();

  // pMember  指针失效
  del(pMember);

  m_setMemberUpdate.insert(id);
  setDataMark(ETEAMDATA_MEMBER);

  TMember* pLeader = getLeader();
  TMember* pTempLeader = getTempLeader();
  if (pLeader == nullptr && pTempLeader == nullptr)
    setLeader(0, ETEAMJOB_LEADER);
  else if (pLeader == nullptr && pTempLeader != nullptr && pTempLeader->isOnline() == true)
    pTempLeader->setJob(ETEAMJOB_LEADER);
  else if (pLeader != nullptr && pTempLeader != nullptr && pLeader->isOnline() == true)
    pTempLeader->setJob(ETEAMJOB_MEMBER);

  handleQuestExitTeam(id);

  // remove cat
  list<TMember*> listCatIDs;
  for (auto &m : m_mapMembers)
  {
    if (m.second->getCatOwnerID() == id)
      listCatIDs.push_back(m.second);
  }
  for (auto &l : listCatIDs)
    removeCat(l->getGUID(), l->getCatID(), false);
  
  //没人立即更新一下
  if (m_mapMembers.empty())
  {
    DWORD curTime = now();
    updateMember(curTime);
  }

  if (isInMatchTeamPws())
    cancelMatchPws();

  return true;
}

bool Team::removeCat(QWORD id, DWORD catid, bool kick)
{
  auto m = m_mapMembers.find(id);
  if (m == m_mapMembers.end())
    return false;

  if (m->second->getCatID() != catid)
    return false;

  QWORD qwOwnerID = m->second->getCatOwnerID();
  del(m->second);
  m_setMemberUpdate.insert(id);

  if (!kick)
    return true;

  CatExitTeamCmd cmd;
  cmd.set_charid(qwOwnerID);
  cmd.set_catid(catid);
  PROTOBUF(cmd, send, len);
  return thisServer->sendCmdToSession(send, len);
}

bool Team::removeCat(ECatRemoveMethod eMethod)
{
  if (eMethod == ECATREMOVEMETHOD_EARLY)
  {
    TMember* pCat = nullptr;
    DWORD dwTime = DWORD_MAX;
    for (auto &m : m_mapMembers)
    {
      if (m.second->getCatID() == 0)
        continue;
      if (m.second->getCreateTime() < dwTime)
      {
        dwTime = m.second->getCreateTime();
        pCat = m.second;
      }
    }
    if (pCat == nullptr)
      return false;

    return removeCat(pCat->getGUID(), pCat->getCatID(), true);
  }

  return false;
}

/*void Team::checkInvalidCat(QWORD qwCharID)
{
  if (getMemberCount() <= MiscConfig::getMe().getTeamCFG().dwMaxMember)
    return;

  map<QWORD, DWORD> mapCat;
  for (auto &m : m_mapMembers)
  {
    if (m.second->getCatOwnerID() == qwCharID)
      mapCat[m.first] = m.second->getCatID();
  }
  for (auto &m : mapCat)
    removeCat(m.first, m.second, false);
}*/

bool Team::setLeader(QWORD qwCharID, ETeamJob eJob)
{
  if (eJob != ETEAMJOB_LEADER && eJob != ETEAMJOB_TEMPLEADER)
    return false;

  if (qwCharID == 0)
  {
    std::multimap<DWORD, TMember*> list;
    for (auto &v : m_mapMembers)
    {
      if (v.second)
      {
        if (v.second->getJob() == eJob)
          v.second->setJob(ETEAMJOB_MEMBER);
        if (v.second->isOnline() == true)
        {
          list.insert(std::make_pair(v.second->getCreateTime(), v.second));
        }
      }
    }

    for (auto &v : list)
    {
      if (v.second)
      {
        v.second->setJob(eJob);
        setApplyUpdate();
        v.second->syncBeLeader();
        broadcastMsg(ESYSTEMMSG_ID_TEAM_LEADER_FIRE, MsgParams(v.second->getName()));
        XLOG << "[队伍-队长交换]" << v.second->getAccid() << v.second->getGUID() << v.second->getProfession() << v.second->getName() << "成为" << m_qwGUID << m_strName << "队长" << XEND;
        break;
      }
    }
    return true;
  }

  TMember* pMember = getMember(qwCharID);
  if (pMember == nullptr || pMember->getJob() == eJob)
    return false;

  QWORD qwOldLeader = 0;
  for (auto &v : m_mapMembers)
  {
    if (v.second)
    {
      if (v.second->getJob() == eJob)
      {
        if(eJob == ETEAMJOB_LEADER && pMember->isOnline())
          v.second->setJob(ETEAMJOB_MEMBER);
        else if(eJob == ETEAMJOB_LEADER && pMember->isOnline() == false)
        {
          v.second->setJob(ETEAMJOB_TEMPLEADER);
          v.second->syncBeLeader();
        }
        else if(eJob == ETEAMJOB_TEMPLEADER && pMember->isOnline())
          v.second->setJob(ETEAMJOB_MEMBER);
        qwOldLeader = v.second->getGUID();
      }
    }
  }
  pMember->setJob(eJob);
  setApplyUpdate();
  pMember->syncBeLeader();
  broadcastMsg(ESYSTEMMSG_ID_TEAM_LEADER_FIRE, MsgParams(pMember->getName()));
  XLOG << "[队伍-队长交换]" << pMember->getAccid() << pMember->getGUID() << pMember->getProfession() << pMember->getName() << "成为" << m_qwGUID << m_strName << "队长" <<"old"<<qwOldLeader<< XEND;

  //log change team leader
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Team_ChangeLeader;
  if (eJob == ETEAMJOB_TEMPLEADER)
    eType = EventType_Team_ChangeTempLeader;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    pMember->getZoneID(),
    pMember->getAccid(),
    pMember->getGUID(),
    eid,
    0,  //charge
    eType, 0, 1);

  PlatLogManager::getMe().TeamLog(thisServer,
    pMember->getAccid(),
    pMember->getGUID(),
    eType,
    eid,
    getGUID(),
    pMember->getGUID()
  );

  return true;
}

bool Team::checkLeaderWhenOnline(QWORD qwCharID)
{
  if (getLeader() != nullptr || getTempLeader() != nullptr)
    return true;

  TMember* pMember = getMember(qwCharID);
  if (pMember == nullptr || pMember->getJob() == ETEAMJOB_LEADER || pMember->getJob() == ETEAMJOB_TEMPLEADER)
    return true;

  return setLeader(qwCharID, ETEAMJOB_LEADER);
}

TMember* Team::getMember(QWORD id)
{
  auto it = m_mapMembers.find(id);
  if (it != m_mapMembers.end())
    return it->second;

  return nullptr;
}

TMember* Team::getLeader()
{
  for (auto v : m_mapMembers)
  {
    if (v.second)
    {
      if (v.second->getJob() == ETEAMJOB_LEADER)
      {
        return v.second;
      }
    }
  }

  return nullptr;
}

TMember* Team::getTempLeader()
{
  for (auto v : m_mapMembers)
  {
    if (v.second)
    {
      if (v.second->getJob() == ETEAMJOB_TEMPLEADER)
      {
        return v.second;
      }
    }
  }

  return nullptr;
}

TMember* Team::getOnlineLeader()
{
  TMember* pLeader = getLeader();
  if (pLeader == nullptr || pLeader->isOnline() == false)
  {
    pLeader = getTempLeader();
    if (pLeader == nullptr || pLeader->isOnline() == false)
      return nullptr;
  }

  return pLeader;
}

DWORD Team::getMemberCount(bool bCatInclude /*= true*/)
{
  DWORD dwCount = 0;
  for (auto &m : m_mapMembers)
  {
    if (bCatInclude || m.second->getCatID() == 0)
      ++dwCount;
  }
  return dwCount;
}

DWORD Team::getOnlineMemberCount() const
{
  DWORD count = 0;
  for (auto v : m_mapMembers)
  {
    if (!v.second) continue;
    if (v.second->isOnline() == false)
      continue;

    ++count;
  }

  return count;
}

bool Team::addApply(TMember *pApply)
{
  if (!pApply) return false;
  if (pApply->getZoneID() == 0 || pApply->getGUID() == 0 || pApply->getJob() != ETEAMJOB_APPLY)
    return false;

  if (m_mapApplys.find(pApply->getGUID()) != m_mapApplys.end())
    return false;

  if (pApply->getBaseLv() < m_dwMinLv || pApply->getBaseLv() > m_dwMaxLv)
    return false;

  m_mapApplys[pApply->getGUID()] = pApply;
  m_setApplyUpdate.insert(pApply->getGUID());

  pApply->m_eJob = ETEAMJOB_APPLY;

  return true;
}

bool Team::removeApply(QWORD id, bool del)
{
  auto it = m_mapApplys.find(id);
  if (it == m_mapApplys.end())
    return false;

  m_setApplyUpdate.insert(id);

  if (del)
  {
    SAFE_DELETE(it->second);
  }

  m_mapApplys.erase(id);

  return true;
}

void Team::clearApply()
{
  for (auto v = m_mapApplys.begin(); v != m_mapApplys.end(); ++v)
  {
    if (v->second)
    {
      m_setApplyUpdate.insert(v->second->getGUID());
      SAFE_DELETE(v->second);
    }
  }
  m_mapApplys.clear();
}

TMember* Team::getApply(QWORD id)
{
  auto it = m_mapApplys.find(id);
  if (it != m_mapApplys.end())
    return it->second;

  return nullptr;
}

bool Team::summon(QWORD id, DWORD raidid)
{
  TMember* pMember = getMember(id);
  if (pMember == nullptr || pMember->isLeader() == false)
    return false;

  TeamSummon cmd;
  cmd.set_raidid(raidid);
  PROTOBUF(cmd, send, len);
  for (auto v = m_mapMembers.begin(); v != m_mapMembers.end(); ++v)
  {
    if (!v->second) continue;
    if (v->second->isLeader() == true || v->second->isOnline() == false)
      continue;

    v->second->sendCmdToMe(send, len);
  }

  return true;
}

void Team::timer(DWORD curTime)
{
  updateData(curTime);
  updateRecord(curTime);
  updateMember(curTime);
  updateApply(curTime);
}

void Team::updateMember(DWORD curTime)
{
  if (m_setMemberUpdate.empty() == true)
    return;

  TeamMemberUpdate cmd;
  TeamMemberUpdateTeamCmd scmd;
  for (auto s = m_setMemberUpdate.begin(); s != m_setMemberUpdate.end(); ++s)
  {
    TMember* pMember = getMember(*s);
    if (pMember != nullptr)
    {
      pMember->toData(cmd.add_updates(), true);
      pMember->toData(scmd.add_updates());
    }
    else
    {
      cmd.add_deletes(*s);
      scmd.add_dels(*s);
    }
  }

  if (cmd.updates_size() > 0 || cmd.deletes_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    broadcastCmd(send, len);
  }
  if (scmd.updates_size() > 0 || scmd.dels_size() > 0)
  {
    for (auto &m : m_mapMembers)
    {
      if (m.second->isOnline() == true)
      {
        scmd.set_charid(m.first);
        PROTOBUF(scmd, ssend, slen);
        thisServer->sendCmdToSession(ssend, slen);
      }
    }
  }

  if (isInPvpRoom())
  {
    PvpTeamMemberUpdateSCmd pvpcmd;
    MatchTeamMemUpdateInfo* pInfo = pvpcmd.mutable_data();
    if (pInfo)
    {
      pInfo->set_isfirst(false);
      for (auto s = m_setMemberUpdate.begin(); s != m_setMemberUpdate.end(); ++s)
      {
        TMember* pMember = getMember(*s);
        if (pMember != nullptr)
        {
          if (pMember->getCatID() == 0)
            pMember->toData(pInfo->add_updates(), true);
        }
        else
        {
          pInfo->add_deletes(*s);
        }
      }
      if (pInfo->updates_size() > 0 || pInfo->deletes_size() > 0)
      {
        pInfo->set_teamid(getGUID());
        pInfo->set_roomid(m_pvpRoomId);
        pInfo->set_zoneid(thisServer->getZoneID());

        PROTOBUF(pvpcmd, send, len);
        bool ret = thisServer->forwardCmdToMatch(send, len);
        XDBG << "[斗技场-同步队伍变化消息到Matchserver] teamid" << id << "roomid" << m_pvpRoomId << "ret" << ret << "msg" << cmd.ShortDebugString() << XEND;
      }
    }
  }
  m_setMemberUpdate.clear();
}

void Team::updateData(DWORD curTime)
{
  if (m_bitset.any() == false && !m_bNameUpdate)
    return;

  TeamDataUpdate cmd;

  // team name
  if (m_bNameUpdate)
  {
    cmd.set_name(m_strName);
    m_bNameUpdate = false;

    if (m_bitset.any() == false)
    {
      PROTOBUF(cmd, send, len);
      broadcastCmd(send, len);
      return;
    }
  }

  std::list<QWORD> list;

  // update overtime apply
  const STeamCFG& rCFG = MiscConfig::getMe().getTeamCFG();
  for (auto v = m_mapApplys.begin(); v != m_mapApplys.end(); ++v)
  {
    if (!v->second) continue;

    if (curTime > v->second->getCreateTime() + rCFG.dwApplyTime)
    {
      m_setApplyUpdate.insert(v->second->getGUID());
      list.push_back(v->second->getGUID());
      continue;
    }
  }
  for (auto &it : list)
    removeApply(it);

  // update team data
  TeamDataUpdateTeamCmd scmd;
  auto add_data = [](TeamSummaryItem* pInfo, ETeamData type, QWORD value)
  {
    if (pInfo == nullptr)
      return;
    pInfo->set_type(type);
    pInfo->set_value(value);
  };
  for (int i = ETEAMDATA_MIN + 1; i < ETEAMDATA_MAX; ++i)
  {
    if (m_bitset.test(i) == false)
      continue;

    ETeamData eType = static_cast<ETeamData>(i);
    string data;
    switch (eType)
    {
      case ETEAMDATA_MIN:
        break;
      case ETEAMDATA_TYPE:
        add_data(cmd.add_datas(), eType, m_pTeamCFG == nullptr ? 0 : m_pTeamCFG->id);
        break;
      case ETEAMDATA_MINLV:
        add_data(cmd.add_datas(), eType, getMinReqLv());
        break;
      case ETEAMDATA_MAXLV:
        add_data(cmd.add_datas(), eType, getMaxReqLv());
        break;
      case ETEAMDATA_OVERTIME:
        break;
      case ETEAMDATA_AUTOACCEPT:
        add_data(cmd.add_datas(), eType, static_cast<DWORD>(m_eAutoType));
        break;
      case ETEAMDATA_PICKUP_MODE:
        add_data(cmd.add_datas(), eType, static_cast<DWORD>(m_dwPickUpMode));
        add_data(scmd.add_datas(), eType, static_cast<DWORD>(m_dwPickUpMode));
        break;
      case ETEAMDATA_MEMBERCOUNT:
      case ETEAMDATA_MEMBER:
      case ETEAMDATA_HELPWANTED:
      case ETEAMDATA_MAX:
        break;
    }
  }

  if (cmd.datas_size() > 0)// || cmd.has_name())
  {
    PROTOBUF(cmd, send, len);
    broadcastCmd(send, len);
#ifdef _DEBUG
    for (int i = 0; i < cmd.datas_size(); ++i)
    {
      const TeamSummaryItem& rItem = cmd.datas(i);
      XDBG << "[队伍-组队数据更新-前端]" << getGUID() << getName() << "type :" << rItem.type() << "value :" << rItem.value() << XEND;
    }
#endif
  }

  if (scmd.datas_size() > 0)
  {
    for (auto &m : m_mapMembers)
    {
      if (m.second->isOnline() == true)
      {
        scmd.set_charid(m.first);
        PROTOBUF(scmd, ssend, slen);
        thisServer->sendCmdToSession(ssend, slen);
      }
    }
#ifdef _DEBUG
    for (int i = 0; i < scmd.datas_size(); ++i)
    {
      const TeamSummaryItem& rItem = scmd.datas(i);
      XDBG << "[队伍-组队数据更新-场景]" << getGUID() << getName() << "type :" << rItem.type() << "value :" << rItem.value() << XEND;
    }
#endif
  }

  m_bitset.reset();
}

void Team::updateRecord(DWORD curTime)
{
  if ((m_recordbitset.any() == false && !m_bNameRecord) || m_dwSaveTime > curTime)
    return;
  m_dwSaveTime = curTime + randBetween(TEAM_RECORD_TIME_MIN, TEAM_RECORD_TIME_MAX);

  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "team");
  if (field == nullptr)
  {
    XERR << "[队伍-存储]" << getGUID() << getName() << "未找到数据库 team 表" << XEND;
    return;
  }
  ThTeamData *pData = new ThTeamData(ThTeamDataAction_Update);
  if (!pData->create(field))
  {
    SAFE_DELETE(pData);
    return;
  }
  xRecord &record = *(pData->m_pRecord);
  pData->m_qwTeamID = m_qwGUID;

  if (m_bNameRecord)
  {
    record.putString("name", getName());
    m_bNameRecord = false;
  }

  set<ETeamData> setDatas;
  for (int i = ETEAMDATA_MIN + 1; i < ETEAMDATA_MAX; ++i)
  {
    if (m_recordbitset.test(i) == false)
      continue;

    ETeamData eType = static_cast<ETeamData>(i);
    setDatas.insert(eType);
    switch (eType)
    {
      case ETEAMDATA_MIN:
        break;
      case ETEAMDATA_TYPE:
        if (m_pTeamCFG != nullptr)
          record.put("teamtype", m_pTeamCFG->id);
        break;
      case ETEAMDATA_MINLV:
        record.put("minlv", getMinReqLv());
        break;
      case ETEAMDATA_MAXLV:
        record.put("maxlv", getMaxReqLv());
        break;
      case ETEAMDATA_OVERTIME:
        record.put("timer", m_dwOverTime);
        break;
      case ETEAMDATA_AUTOACCEPT:
        record.put("autoaccept", m_eAutoType);
        break;
      case ETEAMDATA_PICKUP_MODE:
        record.put("pickupmode", m_dwPickUpMode);
        break;
      case ETEAMDATA_MEMBERCOUNT:
        break;
      case ETEAMDATA_MEMBER:
        {
          string data;
          if (toMemberData(data) == true)
            record.putBin("member", (unsigned char *)(data.c_str()), data.size());
        }
        break;
      case ETEAMDATA_HELPWANTED:
        {
          string data;
          if (toWantedQuestData(data) == true)
            record.putBin("wantedquest", (unsigned char *)(data.c_str()), data.size());
        }
        break;
      case ETEAMDATA_MAX:
        break;
    }
  }

  if (record.m_rs.empty() == false)
  {
    ThTeamDataThread::getMe().add(pData);
  }

  m_recordbitset.reset();
}

void Team::updateApply(DWORD curTime)
{
  // update apply data
  if (m_setApplyUpdate.empty() == true)
    return;

  TeamApplyUpdate cmd;
  for (auto s = m_setApplyUpdate.begin(); s != m_setApplyUpdate.end(); ++s)
  {
    TMember* pApply = getApply(*s);
    if (pApply != nullptr)
      pApply->toData(cmd.add_updates());
    else
      cmd.add_deletes(*s);
  }

  if (cmd.updates_size() > 0 || cmd.deletes_size() > 0)
  {
    for (auto &v : m_mapMembers)
    {
      if (!v.second) continue;
      if (v.second->isLeader() == true && v.second->isOnline() == true)
      {
        PROTOBUF(cmd, send, len);
        v.second->sendCmdToMe(send, len);
        break;
      }
    }
  }

  m_setApplyUpdate.clear();
}

void Team::setApplyUpdate()
{
  m_setApplyUpdate.clear();
  for (auto v = m_mapApplys.begin(); v != m_mapApplys.end(); ++v)
  {
    if (v->second)
      m_setApplyUpdate.insert(v->second->getGUID());
  }
}

void Team::broadcastMsg(DWORD dwMsgID, MsgParams oParams /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/)
{
  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  oParams.toData(cmd);

  PROTOBUF(cmd, send, len);
  broadcastCmd(send, len);
}

void Team::broadcastCmd(const void* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return;

  thisServer->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_TEAM, getGUID(), buf, len);
  /*for (auto &v : m_mapMembers)
  {
    if (v.second)
      v.second->sendCmdToMe(buf, len);
  }*/
}

void Team::broadcastCmdToSessionUser(const void* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return;

  for (auto &v : m_mapMembers)
  {
    if (!v.second) continue;

    if (v.second->isOnline())
      thisServer->sendCmdToSessionUser(v.second->getZoneID(), v.second->getGUID(), buf, len);
  }
}

//------------------tower
bool Team::createTowerScene(const SocialUser& rUser, EnterTower& cmd)
{
  if (m_stTower.bFighting)
    return enterTower(rUser, cmd);

  TMember* pLeader = getLeader();
  if (pLeader == nullptr || pLeader->isLeader() == false || pLeader->isOnline() == false)
    pLeader = getTempLeader();
  if (pLeader == nullptr || pLeader->isLeader() == false || pLeader->isOnline() == false || pLeader->getGUID() != rUser.charid())
  {
    XERR << "[队伍-无限塔创建]" << "teamid"<< m_qwGUID << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "不是队长,无法创建无限塔副本" << cmd.layer() << "层" << XEND;
    return false;
  }

  DWORD limitLv = MiscConfig::getMe().getEndlessTowerCFG().dwLimitUserLv;
  if (pLeader->getBaseLv() < limitLv)
  {
    MsgManager::sendMsg(rUser.charid(), 1315);
    XERR << "[队伍-无限塔创建]" << "teamid" << m_qwGUID << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "等级过低,无法创建无限塔副本" << pLeader->getBaseLv() << limitLv << XEND;
    return false;
  }
  
  const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(cmd.layer());
  if (pCFG == nullptr || cmd.layer() > m_stTower.m_oLeaderInfo.maxlayer()+1)
  {
    XERR << "[队伍-无限塔创建]" << "teamid" << m_qwGUID << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "无限塔层数错误" << cmd.layer() << XEND;
    return false;
  }

  m_stTower.dwCurLayer = cmd.layer();
  m_stTower.dwZoneID = rUser.zoneid();

  TowerSceneCreateSocialCmd scmd;
  scmd.mutable_user()->set_charid(pLeader->getGUID());//CopyFrom(rUser);
  scmd.set_teamid(getGUID());
  scmd.set_layer(cmd.layer());
  PROTOBUF(scmd, send, len);

  XLOG << "[队伍-无限塔创建]" << "teamid" << m_qwGUID << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "创建无限塔副本" << cmd.layer() << "层,通知会话服务器处理" << XEND;
  return thisServer->sendCmdToSession(send, len);
}

bool Team::enterTower(const SocialUser& rUser, EnterTower& cmd)
{
  const DWORD expireTime = 13;

  if (!m_stTower.bFighting)
  {
    XERR << "[队伍-无限塔]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "在无限塔场景未创建前尝试进入" << cmd.layer() << "层" << XEND;
    return false;
  }

  DWORD limitLv = MiscConfig::getMe().getEndlessTowerCFG().dwLimitUserLv;
  if (rUser.baselv() < limitLv)
  {
    XERR << "[队伍-无限塔进入]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "等级过低,无法进入" << rUser.baselv() << limitLv << XEND;
    return false;
  }
  bool pass = false;
  DWORD curSec = now();
  if (cmd.time())
  {
    //check time
    std::stringstream ss;
    ss << rUser.charid() << "_" << cmd.time() << "_" << "rtyuio@#$%^&";
    char sha1[SHA1_LEN + 1];
    bzero(sha1, sizeof(sha1));
    getSha1Result(sha1, ss.str().c_str(), ss.str().size());
    if (cmd.sign() == sha1)
    {
      if (cmd.time() + expireTime <= curSec)
        pass = true;
    }
  }

  if (!pass)
  {
    Cmd::CountDownTickUserCmd message;
    message.set_type(ECOUNTDOWNTYPE_TOWER);
    message.set_tick(expireTime);
    message.set_time(curSec);

    std::stringstream ss;
    ss << rUser.charid() << "_" << curSec << "_" << "rtyuio@#$%^&";
    char sha1[SHA1_LEN + 1];
    bzero(sha1, sizeof(sha1));
    getSha1Result(sha1, ss.str().c_str(), ss.str().size());
    message.set_sign(sha1);

    PROTOBUF(message, send, len);
    thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

    XLOG << "[队伍-无限塔]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "进入无限塔" << cmd.layer() << "层 进入倒计时" << expireTime << "秒" << "curSec" << curSec << "revtime" << cmd.time()<< XEND;
    return true;
  }

  cmd.set_layer(m_stTower.dwCurLayer);
  cmd.set_zoneid(m_stTower.dwZoneID);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSceneUser(rUser.zoneid(), rUser.charid(), send, len);
  XLOG << "[队伍-无限塔]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "进入倒计时结束,通知场景进入无限塔" << cmd.layer() << "层" << XEND;
  return true;
}

void Team::setTowerSceneOpen(DWORD raidid)
{
  EnterTower cmd;
  cmd.set_layer(m_stTower.dwCurLayer);
  cmd.set_zoneid(m_stTower.dwZoneID);
  PROTOBUF(cmd, send, len);
  for (auto &s : m_stTower.setEnterMembers)
  {
    TMember* pMember = getMember(s);
    if (pMember != nullptr && pMember->isOnline() == true)
      thisServer->sendCmdToSceneUser(pMember->getZoneID(), pMember->getGUID(), send, len);
  }

  m_stTower.bFighting = true;
  m_stTower.setEnterMembers.clear();
  m_stTower.dwCurRaidID = raidid;

  XLOG << "[队伍-无限塔]" << cmd.layer() << "层开启" << "所在副本ID:" << raidid << XEND;
}

void Team::setTowerSceneClose(DWORD raidid)
{
  if (raidid == 0 || raidid == m_stTower.dwCurRaidID)
  {
    m_stTower.clear();
    XLOG << "[队伍-无限塔]" << "场景关闭" << XEND;
  }
  else
  {
    XLOG << "[队伍-无限塔], 更换副本, 不设置关闭, 当前副本ID:" << m_stTower.dwCurRaidID << "已关闭的无限塔副本ID:" << raidid << XEND;
  }
}

//--------------- DOJO
//void TDojo::clear()
//{
//  m_dwDojoId = 0;
//  m_qwSponorId = 0;
//  m_sponsorName = "";
//  syncDojoToScene();
//}
//void TDojo::syncDojoToScene()
//{
//  //SyncDojoSessionCmd cmd;
//  //cmd.set_teamguid(m_qwTeamId);
//  //cmd.set_guildid(m_qwGuildId);
//  //cmd.set_sponsorid(m_qwSponorId);
//  //cmd.set_dojoid(m_dwDojoId);
//  //cmd.set_isopen(m_bIsOpen);
//  //PROTOBUF(cmd, send, len);
//  //thisServer->sendCmdToScene(send, len);
//}

bool Team::sponsorDojo(const SocialUser& rUser, Cmd::DojoSponsorCmd& rev)
{
  GCharReader pGChar(thisServer->getRegionID(), rUser.charid());
  if (!pGChar.get()) return false;

  TMember* pTMember = getMember(rUser.charid());
  if (pTMember == nullptr) return false;

  bool ret = false;
  QWORD guildId = pGChar.getGuildID();
  if (!guildId) return false;

  if (pTMember->isLeader())
  {
    TDojo *pDojo = nullptr;
    ESysMsgID eMsg = canSponsor(guildId, rev.dojoid(), rUser);
    if (eMsg != ESYSTEMMSG_ID_MIN)
    {
      if (eMsg != ESYSTEMMSG_ID_DOJO_RUNNING)
        thisServer->sendMsg(rUser.zoneid(), rUser.charid(), eMsg);
      if (eMsg == ESYSTEMMSG_ID_DOJO_BASELV_REQ)
      {
        XERR << "[道场-发起]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
          << "在" << pGChar.getGuildID() << pGChar.getGuildName() << "中发起道场" << rev.dojoid() << "失败,等级不足" << XEND;
        return false;
      }
      pDojo = getDojo(guildId);
      QWORD sponsorId = 0;
      if (pDojo)
        sponsorId = pDojo->m_qwSponorId;
      if (sponsorId == rUser.charid())
        return false;

      XERR << "[道场-发起]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
        << "在" << pGChar.getGuildID() << pGChar.getGuildName() << "中发起道场" << rev.dojoid() << "失败,其他人已经发起:" << sponsorId << XEND;
      ret = false;
    }
    else
      ret = true;

    if (ret)
    {
      pDojo = addDojo(guildId, rUser.charid(), rev.dojoid(), rUser.name());
    }

    if (pDojo)
    {
      rev.set_dojoid(pDojo->m_dwDojoId);
      rev.set_sponsorid(pDojo->m_qwSponorId);
      rev.set_sponsorname(pDojo->m_sponsorName);
    }
  }
  else
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 2908);      //修改：不是队长不可发起道场
    return true;
  }
  rev.set_ret(ret);
  PROTOBUF(rev, send1, len1);
  if (pTMember->sendCmdToMe(send1, len1) == false)
    XERR << "[道场-发起]， 玩家：" << rUser.charid() <<"guildid:"<<guildId<< "发起道场：" << rev.dojoid() << "成功，通知队友 charid :" << pTMember->getGUID() << XEND;

  if (ret)
  {
    for (auto it = m_mapMembers.begin(); it != m_mapMembers.end(); ++it)
    {
      TMember* pMem = it->second;

      if (!pMem || pMem->getGUID() == rUser.charid()) continue;

      GCharReader p(thisServer->getRegionID(), pMem->getGUID());
      if (!p.get())
      {
        XERR << "[道场-发起]， 发起者：" << rUser.charid() << "guildid:" << guildId << "发起道场：" << rev.dojoid() << "通知队友 失败 charid:" << pMem->getGUID()<<"找不到 gchar" << XEND;
        continue;
      }
      if (p.getGuildID() != guildId)
      {
        XERR << "[道场-发起]， 发起者：" << rUser.charid() << "guildid:" << guildId << "发起道场：" << rev.dojoid() << "通知队友 失败 charid:" << pMem->getGUID() << "公会id不一致"<<guildId <<p.getGuildID() << XEND;
        continue; //不是同一个公会的
      }

      Cmd::DojoInviteCmd msg;
      msg.set_dojoid(rev.dojoid());
      msg.set_sponsorid(rUser.charid());
      msg.set_sponsorname(rUser.name());
      PROTOBUF(msg, send1, len1);
      if (pMem->sendCmdToMe(send1, len1) == false)
        XERR << "[道场-发起]， 发起者：" << rUser.charid() <<"guildid:"<<guildId<< "发起道场：" << rev.dojoid() << "通知队友 失败 charid:" << pMem->getGUID() << XEND;
      else
        XINF << "[道场-发起]， 发起者：" << rUser.charid() << "guildid:" << guildId << "发起道场：" << rev.dojoid() << "成功，通知队友 charid:"<<pMem->getGUID() << XEND;
    }
  }
  return true;
}

bool Team::cancelSponsor(const SocialUser& rUser)
{
  GCharReader pGChar(thisServer->getRegionID(), rUser.charid());
  if (!pGChar.get() || !pGChar.getGuildID())
    return false;

  return clearSponsor(pGChar.getGuildID(), rUser.charid());
}

bool Team::setDojoOpen(QWORD guildId)
{
  TDojo* pDojo = getDojo(guildId);
  if (!pDojo)
    return false;
  pDojo->m_bIsOpen = true;
  /* pDojo->syncDojoToScene();*/
  
  EnterDojo cmd;
  cmd.set_dojoid(pDojo->m_dwDojoId);
  cmd.set_userid(pDojo->m_qwSponorId);
  cmd.set_zoneid(pDojo->m_dwZoneId);
  PROTOBUF(cmd, send, len);
  for (auto &v : pDojo->m_inviteAgree)
  {
    TMember* pMem = getMember(v);
    if (pMem)
    {
      thisServer->sendCmdToSceneUser(pMem->getZoneID(), pMem->getGUID(), send, len);
    }
  }
  pDojo->m_inviteAgree.clear(); 
  
  XDBG << "[道场-开启] dojoid:" << pDojo->m_dwDojoId << "temid" << pDojo->m_qwTeamId << "guildid" << pDojo->m_qwGuildId << XEND;
  return true;
}

bool Team::setDojoClose(QWORD guildId)
{
  TDojo* pDojo = getDojo(guildId);
  if (!pDojo)
    return false;
  XDBG << "[道场-关闭] dojoid:" << pDojo->m_dwDojoId << "temid" << pDojo->m_qwTeamId << "guildid" << pDojo->m_qwGuildId << XEND;
  delDojo(guildId);
  return true;
}

TDojo* Team::addDojo(QWORD guildId, QWORD qwSponsorId, DWORD dwDojoId, std::string name)
{
  TDojo* pDojo = mutableDojo(guildId);
  if (pDojo)
  {
    pDojo->m_qwGuildId = guildId;
    pDojo->m_qwTeamId = getGUID();
    pDojo->m_qwSponorId = qwSponsorId;
    pDojo->m_dwDojoId = dwDojoId;
    pDojo->m_sponsorName = name;
    pDojo->m_bIsOpen = false;
  }
  return pDojo;
}

void Team::delDojo(QWORD guildId)
{
  auto it = m_mapDojo.find(guildId);
  if (it != m_mapDojo.end())
    m_mapDojo.erase(guildId);

  /*
  SyncDojoSessionCmd cmd;
  cmd.set_teamguid(getGUID());
  cmd.set_guildid(guildId);
  cmd.set_del(true);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToScene(send, len);*/
}

//清空道场发起
bool Team::clearSponsor(QWORD guildId, QWORD charId)
{
  TDojo* pDojo = getDojo(guildId);
  if (!pDojo)
    return false;
  if (pDojo->m_qwSponorId != charId)
    return false;
  XDBG << "[道场-清除发起] charid:"<<charId<< "dojoid:" << pDojo->m_dwDojoId << "temid" << pDojo->m_qwTeamId << "guildid" << pDojo->m_qwGuildId << XEND;
  if (!pDojo->m_bIsOpen)
    delDojo(guildId);
  return true;
}

ESysMsgID Team::canSponsor(QWORD guildId, DWORD dwDojoID, const SocialUser& rUser)
{
  const SDojoItemCfg* pCFG = DojoConfig::getMe().getDojoItemCfg(dwDojoID);
  if (pCFG == nullptr || rUser.baselv() < MiscConfig::getMe().getGuildDojoCFG().getBaseLvReq(pCFG->dwGroupId))
    return ESYSTEMMSG_ID_DOJO_BASELV_REQ;

  TDojo* pDojo = getDojo(guildId);
  if (pDojo == nullptr)
    return ESYSTEMMSG_ID_MIN;
  if (pDojo->m_dwDojoId == 0)
    return ESYSTEMMSG_ID_MIN;
  return ESYSTEMMSG_ID_DOJO_RUNNING;
}

TDojo* Team::getDojo(QWORD guildId)
{
  auto it = m_mapDojo.find(guildId);
  if (it == m_mapDojo.end())
    return nullptr;
  return &(it->second);
}

TDojo* Team::getDojo(const SocialUser& rUser)
{
  GCharReader pGChar(thisServer->getRegionID(), rUser.charid());
  if (!pGChar.get() || !pGChar.getGuildID())
    return nullptr;
  TDojo *pDojo = getDojo(pGChar.getGuildID());
  return pDojo;
}

TDojo* Team::mutableDojo(QWORD guildId)
{
  auto it = m_mapDojo.find(guildId);
  if (it == m_mapDojo.end())
  {
    TDojo dojo;
    it = m_mapDojo.insert(std::make_pair(guildId, dojo)).first;
  }
  return &(it->second);
}

void Team::getDojoInfo(QWORD guildId, DojoQueryStateCmd& out)
{
  TDojo* pDojo = getDojo(guildId);
  if (pDojo == nullptr)
    return;

  if (!pDojo->m_dwDojoId)
    return;

  if (pDojo->m_bIsOpen)
    out.set_state(DOJOSTATE_OPENED);
  else
    out.set_state(DOJOSTATE_SPONSORED);
  out.set_dojoid(pDojo->m_dwDojoId);
  out.set_sponsorid(pDojo->m_qwSponorId);
  out.set_sponsorname(pDojo->m_sponsorName);
}

bool Team::canDojoFollow(const SocialUser& rMe, QWORD f)
{
  /*if (!pMe)
    return false;

  if (f == 0)   //取消跟随
    return true;
  
  SessionUser* pFollower = SessionUserManager::getMe().getUserByID(f);
  if (!pFollower)
    return false;*/

  /*Guild* pMyGuild = pMe->getGuild();
  Guild* pFollowGuild = pFollower->getGuild();
  if (!pMyGuild || !pFollowGuild || pMyGuild != pFollowGuild)
    return false;*/
  /*const GuildInfo& rMyGuild = pMe->getGuildInfo();
  const GuildInfo& rFollowGuild = pFollower->getGuildInfo();
  if (rMyGuild.id() == 0 || rFollowGuild.id() == 0 || rMyGuild.id() != rFollowGuild.id())
    return false;*/
  //TDojo* pDojo = getDojo(rMyGuild.id()/*pMyGuild->getGUID()*/);
  /*if (!pDojo)
    return false;
  if (pDojo->m_bIsOpen)   //道场已经开始了
    return false;
  if (!pDojo->m_qwSponorId) //道场没人发起
    return false;*/
  return true;
}

void Team::onDojoLeaveTeam(const SocialUser& rUser)
{
  //if (!pUser)
  //  return;
  /*if (!pUser->getGuild())
    return;*/
  //if (pUser->getGuildInfo().id() == 0)
  //  return;
  //clearSponsor(pUser->getGuildInfo().id()/*pUser->getGuild()->getGUID()*/, pUser->id);
}

void Team::onDojoLeaveGuild(const SocialUser& rUser, QWORD guildId)
{
  /*if (!pUser)
    return;
  clearSponsor(guildId, pUser->id);*/
}

bool Team::procInviteDojo(const SocialUser& rUser, DojoReplyCmd& rev)
{
  TDojo* pDojo = getDojo(rUser);
  if (!pDojo || !pDojo->m_qwSponorId)
  {
    if (rev.ereply() == EDOJOREPLY_AGREE)
    {
      TMember* pMem = getMember(rUser.charid());
      if (pMem)
        thisServer->sendMsg(pMem->getZoneID(), rUser.charid(), 2904);    //发起人取消发起
    }
    return true;
  }

  TMember* pSponsor = getMember(pDojo->m_qwSponorId);
  if (pSponsor != nullptr)
  {
    const SDojoItemCfg* pCFG = DojoConfig::getMe().getDojoItemCfg(pDojo->m_dwDojoId);
    if (pCFG == nullptr || rUser.baselv() < MiscConfig::getMe().getGuildDojoCFG().getBaseLvReq(pCFG->dwGroupId))
    {
      XERR << "[道场-答复]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "答应进入" << pDojo->m_qwSponorId << "道场失败,等级不足" << XEND;
      rev.set_ereply(EDOJOREPLY_DISAGREE);
    }

    if (rev.ereply() == EDOJOREPLY_AGREE)
      pDojo->m_inviteAgree.insert(rUser.charid());
    rev.set_userid(rUser.charid());
    PROTOBUF(rev, send, len);
    pSponsor->sendCmdToMe(send, len);
  }
  return true;
}

bool Team::createDojoScene(const SocialUser& rUser,EnterDojo& rev)
{
  TDojo* pDojo = getDojo(rUser);
  if (!pDojo)
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 2906);    //无法加入，道场尚未开启或已经结束。
    return false;
  }
  if (pDojo->m_bIsOpen)
  {   
    return enterDojo(rUser, rev);
  }

  if (pDojo->m_qwSponorId != rUser.charid())
    return false;

  pDojo->m_dwZoneId = rUser.zoneid();  
  DojoCreateSocialCmd cmd;
  cmd.set_charid(rUser.charid());
  cmd.set_dojoid(pDojo->m_dwDojoId);
  cmd.set_guildid(pDojo->m_qwGuildId);
  cmd.set_teamid(pDojo->m_qwTeamId);
  PROTOBUF(cmd, send, len);

  if (thisServer->sendCmdToSession(send, len) == true)
    XLOG << "[队伍-道场]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "创建道场 id :" << pDojo->m_dwDojoId << "成功,发送至会话处理" << XEND;
  else
    XERR << "[队伍-道场]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "创建道场 id :" << pDojo->m_dwDojoId << "失败,消息未发送" << XEND;

  /*for (auto &v : pDojo->m_inviteAgree)
  {
    TMember* pMem = getMember(v);
    if (pMem)
    {
      thisServer->sendCmdToSceneUser(pMem->getZoneID(), pMem->getGUID(), send, len);
    }    
  }
  pDojo->m_inviteAgree.clear();*/
  return true;
}

bool Team::enterDojo(const SocialUser& rUser, EnterDojo& rev)
{
  TDojo* pDojo = getDojo(rUser);
  if (!pDojo)
  {
    XERR << "[队伍-道场]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "进入道场失败,未发现道场" << XEND;
    return false;
  }

  const DWORD expireTime = 12;

  if (!pDojo->m_bIsOpen)
    return false;
  bool pass = false;
  DWORD curSec = now();
  if (rev.time())
  {
    //check time
    std::stringstream ss;
    ss << rUser.charid() << "_" << rev.time() << "_" << "rtyuio@#$%^&";
    char sha1[SHA1_LEN + 1];
    bzero(sha1, sizeof(sha1));
    getSha1Result(sha1, ss.str().c_str(), ss.str().size());
    if (rev.sign() == sha1)
    {
      if (rev.time() + expireTime <= curSec)
        pass = true;
    }
  }
  
  if (!pass)
  {
    Cmd::CountDownTickUserCmd message;
    message.set_type(ECOUNTDOWNTYPE_DOJO);
    message.set_tick(expireTime);
    message.set_extparam(pDojo->m_dwDojoId);
    message.set_time(curSec);
    std::stringstream ss;
    ss << rUser.charid() << "_" << curSec << "_" << "rtyuio@#$%^&";
    char sha1[SHA1_LEN + 1];
    bzero(sha1, sizeof(sha1));
    getSha1Result(sha1, ss.str().c_str(), ss.str().size());    
    message.set_sign(sha1);
    PROTOBUF(message, send, len);
    thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
    XINF << "[道场-进入] 通知客户端倒计时， dojoid：" << pDojo->m_dwDojoId << "charid:" << rUser.charid()<<"zoneid:"<<rUser.zoneid()<<"curSec"<<curSec<<"revtime"<<rev.time()<<expireTime << XEND;
    return true;
  }

  rev.Clear();
  rev.set_dojoid(pDojo->m_dwDojoId);
  rev.set_userid(pDojo->m_qwSponorId);
  rev.set_zoneid(pDojo->m_dwZoneId);
  PROTOBUF(rev, send, len);
  thisServer->sendCmdToSceneUser(rUser.zoneid(), rUser.charid(), send, len);
  XINF << "[道场-进入] 通知Scene， dojoid：" << pDojo->m_dwDojoId << "charid:" << rUser.charid() << "zoneid:" << rUser.zoneid() << XEND;
  return true;
}

DWORD Team::getRaidZoneID(DWORD raidid) const
{
  auto it = m_mapRaidZone.find(raidid);
  if (it == m_mapRaidZone.end()) return 0;

  return it->second;
}

void Team::setRaidZoneID(DWORD raid, DWORD zoneid)
{
  if (!raid || !zoneid) return;

  m_mapRaidZone[raid] = zoneid;
}

void Team::delRaidZoneID(DWORD raid)
{
  m_mapRaidZone.erase(raid);
}

void Team::onLeaderFinishSeal()
{
  if (m_dwLastGoalID == 0 || m_pTeamCFG == nullptr)
    return;
  const TeamGoalBase* pLastGoal = TableManager::getMe().getTeamCFG(m_dwLastGoalID);
  if (pLastGoal == nullptr)
    return;
  if (pLastGoal->getTeamFilter() == ETEAMFILTER_SEAL || m_pTeamCFG->getTeamFilter() != ETEAMFILTER_SEAL)
    return;

  setTeamType(pLastGoal);

  XLOG << "[组队-目标], 队伍目标变化, 队伍:" << m_qwGUID <<" 队长完成封印, 目标由" << m_dwLastGoalID << "变为:" << m_pTeamCFG->id << XEND;
}

void Team::callCatEnter()
{
  CatCallTeamCmd cmd;
  for (auto &m : m_mapMembers)
  {
    cmd.set_charid(m.first);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
}

bool Team::acceptHelpQuest(QWORD qwCharID, DWORD dwQuestID, bool isAbandon)
{
  TMember* pMember = getMember(qwCharID);
  if (pMember == nullptr)
  {
    XERR << "[组队-看板], 找不到成员:" << qwCharID << "队伍:" << m_qwGUID << XEND;
    return false;
  }
  const SWantedItem* pQuest = QuestConfig::getMe().getWantedItemCFG(dwQuestID);
  if (pQuest == nullptr || pQuest->bTeamSync == false)
  {
    XERR << "[组队-看板], 此任务无法帮助完成, 玩家:" << qwCharID << "队伍:" << m_qwGUID << "任务ID:" << dwQuestID << XEND;
    return false;
  }
  UpdateHelpWantedTeamCmd cmd;
  if (isAbandon)
  {
    auto it = m_mapUser2HelpQuest.find(qwCharID);
    if (it == m_mapUser2HelpQuest.end())
      return false;
    if (it->second.find(dwQuestID) == it->second.end())
    {
      XERR << "[组队-看板], 放弃帮助任务, 找不到任务ID:" << dwQuestID << "玩家:" << qwCharID << "队伍:" << m_qwGUID << XEND;
      return false;
    }
    cmd.add_dellist(dwQuestID);
    XLOG << "[组队-看板], 放弃帮助任务:" << dwQuestID << "玩家:" << qwCharID << "队伍:" << m_qwGUID << XEND;

    it->second.erase(dwQuestID);
    if (it->second.empty())
      m_mapUser2HelpQuest.erase(it);
  }
  else
  {
    TSetDWORD& quests = m_mapUser2HelpQuest[qwCharID];
    if (quests.find(dwQuestID) != quests.end())
    {
      XERR << "[组队-看板], 接取帮助任务, 已接取该帮助任务:" << dwQuestID << "玩家:" << qwCharID << "队伍:" << m_qwGUID << XEND;
      return false;
    }
    quests.insert(dwQuestID);
    cmd.add_addlist(dwQuestID);
    if (pQuest)
    {
      for (auto &t : m_mapMembers)
      {
        if (!t.second)
          continue;
        if (t.second->getGUID() == qwCharID)
          continue;
        if (t.second->isOnline() == false)
          continue;
        thisServer->sendMsg(t.second->getZoneID(), t.second->getGUID(), 899, MsgParams(pMember->getName(), pQuest->strName));
      }
    }
    XLOG << "[组队-看板], 接取帮助任务:" << dwQuestID << "玩家:" << qwCharID << "队伍:" << m_qwGUID << XEND;
  }
  if (cmd.dellist_size() > 0 || cmd.addlist_size() > 0)
  {
    setDataMark(ETEAMDATA_HELPWANTED);
    PROTOBUF(cmd, send, len);
    pMember->sendCmdToMe(send, len);
  }
  return true;
}

void Team::queryHelpQuestList(QWORD qwCharID)
{
  TMember* pMember = getMember(qwCharID);
  if (pMember == nullptr)
  {
    XERR << "[组队-看板],请求看板帮助任务, 找不到成员:" << qwCharID << "队伍:" << m_qwGUID << XEND;
    return;
  }
  auto it = m_mapUser2HelpQuest.find(qwCharID);
  if (it == m_mapUser2HelpQuest.end())
    return;

  QueryHelpWantedTeamCmd cmd;
  for (auto &s : it->second)
    cmd.add_questids(s);

  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);
}

void Team::updateMemberQuest(const MemberWantedQuest& msg)
{
  TMember* pMember = getMember(msg.charid());
  if (pMember == nullptr)
  {
    XERR << "[组队-看板], 任务更新, 找不到队员:" << msg.charid() << "队伍:" << m_qwGUID << XEND;
    return;
  }

  auto it = m_mapUser2WantedQuest.find(msg.charid());
  switch(msg.action())
  {
    case EQUESTACTION_MIN:
    case EQUESTACTION_MAX:
      return;
      break;
    case EQUESTACTION_ACCEPT:
      {
        if (it == m_mapUser2WantedQuest.end())
        {
          pair<DWORD, DWORD>& pa = m_mapUser2WantedQuest[msg.charid()];
          pa.first = msg.questid();
          pa.second = msg.step();
          XLOG << "[组队-看板], 接取任务" << msg.questid() << msg.step() << "玩家:" << msg.charid() << "队伍:" << m_qwGUID << XEND;
          break;
        }
        if (it->second.first == msg.questid() && it->second.second == msg.step())
          return;

        XLOG << "[组队-看板], 更新" << "当前任务ID:" << it->second.first << "更新ID:" << msg.questid() << "当前step:" << it->second.second << "更新step:" << msg.step() << "玩家" << msg.charid() << "队伍:" << m_qwGUID << XEND;
        it->second.first = msg.questid();
        it->second.second = msg.step();
      }
      break;
    case EQUESTACTION_ABANDON_GROUP:
    case EQUESTACTION_ABANDON_QUEST:
      {
        if (it == m_mapUser2WantedQuest.end())
        {
          XERR << "[组队-看板], 放弃异常" << "找不到玩家:" << msg.charid() << "队伍:" << m_qwGUID << XEND;
          return;
        }
        XLOG << "[组队-看板], 放弃任务" << "当前ID:" << it->second.first << "放弃ID:" << msg.questid() << "玩家:" << msg.charid() << "队伍:" << m_qwGUID << XEND;
        m_mapUser2WantedQuest.erase(it);
      }
      break;
    case EQUESTACTION_SUBMIT:
      {
        if (it == m_mapUser2WantedQuest.end())
        {
          XERR << "[组队-看板], 提交异常" << "找不到玩家:" << msg.charid() << "队伍:" << m_qwGUID << XEND;
          return;
        }
        if (it->second.first != msg.questid())
        {
          XERR << "[组队-看板], 提交异常, 任务ID错误:" << it->second.first << msg.questid() << "玩家:" << msg.charid() << "队伍:" << m_qwGUID << XEND;
          m_mapUser2WantedQuest.erase(it);
          return;
        }
        XLOG << "[组队-看板], 提交任务, 任务ID:" << msg.questid() << "玩家:" << msg.charid() << "队伍:" << m_qwGUID << XEND;
        m_mapUser2WantedQuest.erase(it);
      }
      break;
    default:
      return;
      break;
  }

  UpdateWantedQuestTeamCmd cmd;
  cmd.mutable_quest()->CopyFrom(msg);
  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(msg.questid());
  if (pCFG == nullptr || pCFG->toStepConfig(cmd.mutable_quest()->mutable_questdata(), msg.step()) == false)
    XERR << "[组队-看板]" << msg.ShortDebugString() << "配置初始化失败" << XEND;

  PROTOBUF(cmd, send, len);
  for (auto &m : m_mapMembers)
  {
    if (!m.second || m.second->getGUID() == msg.charid())
      continue;
    if (m.second && m.second->isOnline())
      m.second->sendCmdToMe(send, len);
  }
  setDataMark(ETEAMDATA_HELPWANTED);
  updateHelpQuest();
}

void Team::handleQuestExitTeam(QWORD qwCharID)
{
  auto it = m_mapUser2HelpQuest.find(qwCharID);
  if (it != m_mapUser2HelpQuest.end())
    m_mapUser2HelpQuest.erase(it);

  auto it2 = m_mapUser2WantedQuest.find(qwCharID);
  if (it2 != m_mapUser2WantedQuest.end())
    m_mapUser2WantedQuest.erase(it2);

  updateHelpQuest();
}

void Team::updateHelpQuest()
{
  TSetDWORD allquest;
  for (auto &m : m_mapUser2WantedQuest)
    allquest.insert(m.second.first);
  auto findquest = [&](DWORD questid) ->bool
  {
    return allquest.find(questid) != allquest.end();
  };

  bool bUpdate = false;
  for (auto it = m_mapUser2HelpQuest.begin(); it != m_mapUser2HelpQuest.end(); )
  {
    for (auto s = it->second.begin(); s != it->second.end(); )
    {
      if (findquest(*s) == false)
      {
        s = it->second.erase(s);
        bUpdate = true;
        continue;
      }
      ++s;
    }

    if (it->second.empty())
    {
      it = m_mapUser2HelpQuest.erase(it);
      continue;
    }
    ++it;
  }
  if (bUpdate) setDataMark(ETEAMDATA_HELPWANTED);
}

void Team::queryMemberQuestList(QWORD qwCharID)
{
  TMember* pMember = getMember(qwCharID);
  if (pMember == nullptr)
  {
    XERR << "[组队-看板],请求队伍成员看板任务, 找不到成员:" << qwCharID << "队伍:" << m_qwGUID << XEND;
    return;
  }

  QuestWantedQuestTeamCmd cmd;
  for (auto &m : m_mapUser2WantedQuest)
  {
    if (m.first == qwCharID)
      continue;
    const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(m.second.first);
    if (pCFG == nullptr)
    {
      XERR << "[组队-看板],请求队伍成员看板任务, questid :" << m.second.first << "未在Table_Quest.txt表中找到" << XEND;
      continue;
    }

    MemberWantedQuest* pQuest = cmd.add_quests();
    if (pQuest == nullptr)
      continue;
    pQuest->set_charid(m.first);
    pQuest->set_questid(m.second.first);
    pQuest->set_step(m.second.second);
    pQuest->set_action(EQUESTACTION_ACCEPT);

    if (m.second.second >= pCFG->vecStepData.size())
    {
      XERR << "[组队-看板],请求队伍成员看板任务, questid :" << m.second.first << "step :" << m.second.second << "超过总步骤数" << pCFG->vecStepData.size() << XEND;
      continue;
    }
    pCFG->toStepConfig(pQuest->mutable_questdata(), m.second.second);
  }

  if (cmd.quests_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    pMember->sendCmdToMe(send, len);
  }
}

bool Team::toWantedQuestData(string& str)
{
  BlobTeamWanted oBlob;
  for (auto &m : m_mapUser2HelpQuest)
  {
    TeamMemberHelpQuest* pMemberHelp = oBlob.add_memberhelp();
    if (pMemberHelp == nullptr)
      continue;
    if (m.second.empty())
      continue;
    if (getMember(m.first) == nullptr)
      continue;
    pMemberHelp->set_charid(m.first);
    for (auto &s : m.second)
    {
      pMemberHelp->add_helpquest(s);
    }
  }
  for (auto &m : m_mapUser2WantedQuest)
  {
    TeamMemberWantedQuest* pMemberQuest = oBlob.add_memberquest();
    if (pMemberQuest == nullptr)
      continue;
    if (getMember(m.first) == nullptr)
      continue;
    pMemberQuest->set_charid(m.first);
    pMemberQuest->set_acceptquest(m.second.first);
    pMemberQuest->set_acceptstep(m.second.second);
  }

  if (oBlob.SerializeToString(&str) == false)
    return false;

  return true;
}

bool Team::fromWantedQuestData(const string& str)
{
  BlobTeamWanted oBlob;
  if (oBlob.ParseFromString(str) == false)
    return false;
  m_mapUser2HelpQuest.clear();
  for (int i = 0; i < oBlob.memberhelp_size(); ++i)
  {
    const TeamMemberHelpQuest& memberhelp = oBlob.memberhelp(i);
    if (getMember(memberhelp.charid()) == nullptr)
      continue;
    TSetDWORD& quest = m_mapUser2HelpQuest[memberhelp.charid()];
    for (int j = 0; j < memberhelp.helpquest_size(); ++j)
    {
      quest.insert(memberhelp.helpquest(j));
    }
  }
  for (int i = 0; i < oBlob.memberquest_size(); ++i)
  {
    const TeamMemberWantedQuest& memberquest = oBlob.memberquest(i);
    if (getMember(memberquest.charid()) == nullptr)
      continue;
    pair<DWORD, DWORD>& pa = m_mapUser2WantedQuest[memberquest.charid()];
    pa.first = memberquest.acceptquest();
    pa.second = memberquest.acceptstep();
  }

  return true;
}

bool Team::sendDataToMatchServer()
{
  if (!isInPvpRoom())
  {
    return false;
  }

  PvpTeamMemberUpdateSCmd cmd;  
  MatchTeamMemUpdateInfo* pInfo = cmd.mutable_data();
  if (pInfo == nullptr)
    return false;

  pInfo->set_zoneid(thisServer->getZoneID());
  pInfo->set_isfirst(true);
  pInfo->set_teamname(m_strName);

  for (auto v = m_mapMembers.begin(); v != m_mapMembers.end(); ++v)
  {
    if (!v->second) continue;
    if (v->second->getCatID() != 0)
      continue;
    v->second->toData(pInfo->add_updates(), true);
  }
  if (pInfo->updates_size() == 0)
  {
    return true;
  }
  pInfo->set_zoneid(thisServer->getZoneID());
  pInfo->set_teamid(getGUID());
  pInfo->set_roomid(m_pvpRoomId);
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->forwardCmdToMatch(send, len);
  XDBG << "[斗技场-转发队伍消息到Matchserver] teamid" << id << "roomid" << m_pvpRoomId << "ret" << ret << "msg" << cmd.ShortDebugString() << XEND;
  return true;
}

// 仅原线的队伍加入了pvp时, 返回true
bool Team::isInPvpRoom()
{
  if (m_pvpRoomId == 0)
    return false;
  
  if (thisServer->getZoneCategory())
    return false;
  if (isMatchCreate())
    return false;
  return true;
}

void Team::setLeaderTowerInfo(const UserTowerInfo& rInfo, QWORD userid)
{
  //if(rInfo.maxlayer() < m_stTower.m_oLeaderInfo.maxlayer())
  //  return;
  TMember* pMember = getMember(userid);
  if(pMember == nullptr || pMember->isLeader() == false)
    return;

  m_stTower.m_oLeaderInfo.CopyFrom(rInfo);
}
bool Team::createPveCardRaid(const SocialUser& rUser, DWORD configid)
{
  if (m_stPveCard.bFighting)
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 118);
    return false;
  }

  TMember* pLeader = getLeader();
  if (pLeader == nullptr || pLeader->isLeader() == false || pLeader->isOnline() == false)
    pLeader = getTempLeader();
  if (pLeader == nullptr || pLeader->isLeader() == false || pLeader->isOnline() == false || pLeader->getGUID() != rUser.charid())
  {
    XERR << "[Pve卡牌副本创建]" << "teamid"<< m_qwGUID << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "不是队长,无法创建" << XEND;
    return false;
  }

  DWORD limitlv = MiscConfig::getMe().getMiscPveCFG().dwLimitLv;
  if (pLeader->getBaseLv() < limitlv)
  {
    XERR << "[Pve卡牌副本创建], 队长等级过低，不可创建, 队伍:" << m_qwGUID << "玩家:" << rUser.name() << rUser.charid() << XEND;
    return false;
  }

  if (PveCardConfig::getMe().getPveRaidCFGByID(configid) == nullptr)
  {
    XERR << "[Pve卡牌副本创建], 非法的ID, 队伍:" << m_qwGUID << "玩家:" << rUser.name() << rUser.charid() << "ID:" << configid << XEND;
    return false;
  }

  m_stPveCard.dwConfigID = configid;

  CardSceneCreateSocialCmd cmd;
  cmd.set_teamid(m_qwGUID);
  cmd.set_userid(rUser.charid());
  cmd.set_configid(configid);

  PROTOBUF(cmd, send, len);

  thisServer->sendCmdToSession(send, len);
  XLOG << "[Pve卡牌副本创建], 发送至session处理, 队伍:" << m_qwGUID << "玩家:" << rUser.name() << rUser.charid() << "PveRaid ID:" << configid << XEND;

  return true;
}

bool Team::onPveCardRaidOpen()
{
  EnterPveCardCmd cmd;
  cmd.set_configid(m_stPveCard.dwConfigID);
  PROTOBUF(cmd, send, len);
  for (auto &s : m_stPveCard.setEnterMembers)
  {
    TMember* pMember = getMember(s);
    if (pMember != nullptr && pMember->isOnline() == true)
      thisServer->sendCmdToSceneUser(pMember->getZoneID(), pMember->getGUID(), send, len);
  }
  m_stPveCard.bFighting = true;
  m_stPveCard.setEnterMembers.clear();

  XLOG << "[队伍-Pve卡牌副本], 副本开启, 队伍:" << m_qwGUID << "ID:" << m_stPveCard.dwConfigID << XEND;
  return true;
}

bool Team::onPveCardRaidClose()
{
  m_stPveCard.clear();
  XLOG << "[队伍-Pve卡牌副本], 副本关闭, 队伍:" << m_qwGUID << "ID:" << m_stPveCard.dwConfigID << XEND;
  return true;
}

STeamRaid* Team::getTeamRaidData(ERaidType eType)
{
  auto it = m_mapTeamRaid.find(eType);
  if(it != m_mapTeamRaid.end())
    return &(it->second);

  return nullptr;
}

void Team::addTeamRaidData(ERaidType eType, STeamRaid stData)
{
  m_mapTeamRaid.emplace(eType, stData);
}

bool Team::getTeamRaidInviteOpen(ERaidType eType)
{
  auto it = m_mapTeamRaid.find(eType);
  if(it != m_mapTeamRaid.end())
    return it->second.bInviteOn;

  return false;
}
void Team::setTeamRaidInviteOpen(ERaidType eType)
{
  auto it = m_mapTeamRaid.find(eType);
  if(it != m_mapTeamRaid.end())
    it->second.bInviteOn = true;
}

bool Team::addTeamRaidEnterMember(ERaidType eType, QWORD userid)
{
  auto it = m_mapTeamRaid.find(eType);
  if(it != m_mapTeamRaid.end())
  {
    it->second.setEnterMembers.emplace(userid);
    return true;
  }

  return false;
}

bool Team::createTeamRaid(const SocialUser& rUser, TeamRaidEnterCmd& cmd)
{
  auto it = m_mapTeamRaid.find(cmd.raid_type());
  if(it == m_mapTeamRaid.end())
    return false;
  if(it->second.bFighting == true)
    return enterTeamRaid(rUser, cmd);

  TMember* pLeader = getOnlineLeader();
  if(pLeader == nullptr || pLeader->getGUID() != rUser.charid())
    return false;

  it->second.dwZoneID = rUser.zoneid();

  TeamRaidSceneCreateSocialCmd scmd;
  scmd.mutable_user()->set_charid(rUser.charid());
  scmd.set_teamid(getGUID());
  scmd.set_raid_type(cmd.raid_type());
  PROTOBUF(scmd, send, len);

  XLOG << "[队伍-队伍副本创建]" << "teamid" << m_qwGUID << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "队伍副本类型: " <<
    cmd.raid_type() << XEND;
  return thisServer->sendCmdToSession(send, len);
}

bool Team::enterTeamRaid(const SocialUser& rUser, TeamRaidEnterCmd& cmd)
{
  auto it = m_mapTeamRaid.find(cmd.raid_type());
  if(it == m_mapTeamRaid.end())
    return false;
  if(it->second.bFighting == false)
    return false;

  const DWORD expireTime = 13;
  bool pass = false;
  DWORD curSec = now();
  if (cmd.time())
  {
    //check time
    std::stringstream ss;
    ss << rUser.charid() << "_" << cmd.time() << "_" << "rtyuio@#$%^&";
    char sha1[SHA1_LEN + 1];
    bzero(sha1, sizeof(sha1));
    getSha1Result(sha1, ss.str().c_str(), ss.str().size());
    if (cmd.sign() == sha1)
    {
      if (cmd.time() + expireTime <= curSec)
        pass = true;
    }
  }

  if (!pass)
  {
    Cmd::CountDownTickUserCmd message;
    message.set_type(ECOUNTDOWNTYPE_ALTMAN);
    message.set_tick(expireTime);
    message.set_time(curSec);

    std::stringstream ss;
    ss << rUser.charid() << "_" << curSec << "_" << "rtyuio@#$%^&";
    char sha1[SHA1_LEN + 1];
    bzero(sha1, sizeof(sha1));
    getSha1Result(sha1, ss.str().c_str(), ss.str().size());
    message.set_sign(sha1);

    PROTOBUF(message, send, len);
    thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

    XLOG << "[队伍-奥特曼]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "进入地球裂隙进入倒计时" << expireTime << "秒" << "curSec" << curSec << "revtime" << cmd.time()<< XEND;
    return true;
  }

  cmd.set_zoneid(it->second.dwZoneID);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSceneUser(rUser.zoneid(), rUser.charid(), send, len);
  return true;
}

bool Team::onTeamRaidOpen(ERaidType eType)
{
  auto it = m_mapTeamRaid.find(eType);
  if(it == m_mapTeamRaid.end())
    return false;

  TeamRaidEnterCmd cmd;
  cmd.set_raid_type(eType);
  cmd.set_zoneid(it->second.dwZoneID);
  PROTOBUF(cmd, send, len);
  for (auto &s : it->second.setEnterMembers)
  {
    TMember* pMember = getMember(s);
    if (pMember != nullptr && pMember->isOnline() == true)
      thisServer->sendCmdToSceneUser(pMember->getZoneID(), pMember->getGUID(), send, len);
  }

  it->second.bFighting = true;
  it->second.setEnterMembers.clear();

  XLOG << "[队伍-副本], 副本开启, 队伍:" << m_qwGUID << "type: " << eType << XEND;
  return true;
}

bool Team::onTeamRaidClose(ERaidType eType)
{
  auto it = m_mapTeamRaid.find(eType);
  if(it != m_mapTeamRaid.end())
  {
    it->second.clear();
    return true;
  }

  return false;
}

const string& Team::getRealtimeVoiceID()
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

void Team::cancelMatchPws()
{
  ExitTeamPwsMatchSCmd cmd;
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_teamid(m_qwGUID);
  cmd.set_etype(m_eMatchingType);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToMatch(send, len);

  m_eMatchingType = EPVPTYPE_MIN;
}

