#include "MatchRoom.h"
#include "MatchManager.h"
#include "MatchSCmd.pb.h"
#include "MatchServer.h"
#include "MiscConfig.h"
#include "ScoreManager.h"

MatchRoom::MatchRoom(QWORD guid) : m_qwGuid(guid)
{
}

MatchRoom::~MatchRoom()
{
}

void MatchRoom::setState(Cmd::ERoomState state)
{
  m_state = state;
}

void MatchRoom::toBriefData(Cmd::RoomBriefInfo* pRoomInfo)
{
  if (pRoomInfo == nullptr)
    return;
  pRoomInfo->set_type(getType());
  pRoomInfo->set_roomid(m_qwGuid);
  pRoomInfo->set_state(m_state);
  pRoomInfo->set_name(m_name);
}

void MatchRoom::toDetailData(Cmd::RoomDetailInfo* pRoomInfo)
{
  if (pRoomInfo == nullptr)
    return;
  pRoomInfo->set_type(getType());
  pRoomInfo->set_roomid(m_qwGuid);
  pRoomInfo->set_state(m_state);
  pRoomInfo->set_name(m_name);
}

  
void MatchRoom::setMatchRoomMgr(MatchRoomMgr* pMgr)
{
  m_pRoomMgr = pMgr;
}

/************************************************************************/
/*LLHMatchRoom                                                                    */
/************************************************************************/
LLHMatchRoom::LLHMatchRoom(QWORD guid):MatchRoom(guid)
{  
}
LLHMatchRoom::~LLHMatchRoom()
{
}

EMatchRetCode LLHMatchRoom::checkCanJoin()
{
  if (m_dwCount >= m_dwPeopleLimit)
  {
    return EMATCHRETCODE_IS_FULL;
  }
  return EMATCHRETCODE_OK;
}

bool LLHMatchRoom::addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  m_dwCount++;
  XLOG << "[斗技场-溜溜猴-加入房间战斗] 房间id" << m_qwGuid << "charid" << charId << "zoneid" << zoneId << "当前房间人数" << m_dwCount << XEND;
  return true;
}

bool LLHMatchRoom::delMember(QWORD charId)
{
  return true;
}

void LLHMatchRoom::toDetailData(Cmd::RoomDetailInfo* pRoomInfo)
{
}

void LLHMatchRoom::toBriefData(Cmd::RoomBriefInfo* pRoomInfo)
{
  if (pRoomInfo == nullptr)
    return;
  
  MatchRoom::toBriefData(pRoomInfo);

  pRoomInfo->set_raidid(getMapId());
  
  DWORD peopleCount = m_dwCount;
  if (m_dwCount > m_dwPeopleLimit)
    peopleCount = m_dwPeopleLimit;

  pRoomInfo->set_player_num(peopleCount);
}

void LLHMatchRoom::onUserOnline(const SocialUser& rUser)
{
}

void LLHMatchRoom::updateUserCount(DWORD count)
{
  DWORD old = m_dwCount;
  m_dwCount = count;
  
  m_pRoomMgr->updateRank();
  XLOG << "[斗技场-溜溜猴-更新房间人数] 房间id" << m_qwGuid << "老的房间人数" << old << "新房间人数" << m_dwCount << XEND;
}

/************************************************************************/
/* RoomTeam                                                                     */
/************************************************************************/

RoomTeam::RoomTeam(DWORD zoneId, QWORD teamId, DWORD index, QWORD roomId, QWORD creator):m_zoneId(zoneId), 
m_teamId(teamId), m_index(index), m_roomId(roomId), m_qwTeamCreator(creator)
{
}

RoomTeam::~RoomTeam()
{
}

void RoomTeam::addTeamMem(const TeamMember& info)
{
  m_mapTeamMembers[info.guid()] = info;
  
  for (int i = 0; i < info.datas_size(); ++i)
  {
    const MemberData& rData = info.datas(i);
    if (rData.type() == EMEMBERDATA_OFFLINE)
    {
      setOffline(info.guid(), rData.value());
    }
    else if (rData.type() == EMEMBERDATA_JOB)
    {
      if (rData.value() == ETEAMJOB_LEADER)
        setLeader(info.guid());
      else if(rData.value() == ETEAMJOB_TEMPLEADER)
        setTempLeader(info.guid());
    }
  }
}

void RoomTeam::delTeamMem(QWORD charId)
{
  m_mapTeamMembers.erase(charId);
  m_setOffline.erase(charId);
}

QWORD RoomTeam::getOnlineLeader()
{
  if (m_qwLeaderId)
    if (!isOffline(m_qwLeaderId))
      return m_qwLeaderId;
  
  if (m_qwTempLeaderId)
    if (!isOffline(m_qwTempLeaderId))
      return m_qwTempLeaderId;
  
  return m_qwTeamCreator;
}

TeamMember* RoomTeam::getTeamMember(QWORD charId)
{
  auto it = m_mapTeamMembers.find(charId);
  if (it == m_mapTeamMembers.end())
    return nullptr;

  return &it->second;
}

bool RoomTeam::sendAllUserToFight(DWORD toZoneId, DWORD raidId, QWORD roomId)
{  
  for (auto &m : m_mapTeamMembers)
  {    
    MatchManager::getMe().sendPlayerToFighting(m_zoneId, toZoneId, m.first, raidId, roomId, m_index);
  }
  return true;
}

void RoomTeam::sendCmdToAllMember(void * buf, WORD len, QWORD except)
{
  for (auto &m : m_mapTeamMembers)
  {
    if (m.first  == except)
      continue;
    thisServer->sendCmdToClient(m.first, m_zoneId, buf, len);
  }
}

void RoomTeam::sendMsgToAllMember(DWORD msgid, const MsgParams& params/* = MsgParams()*/)
{
  for (auto &m : m_mapTeamMembers)
  {
    MatchManager::getMe().sendSysMsg(m.first, m_zoneId, msgid, params);
  }
}

void RoomTeam::toMatchTeamData(MatchTeamData* pData)
{
  if (pData == nullptr)
    return;

  pData->set_name(m_name);
  pData->set_teamid(m_teamId);
  pData->set_index(m_index);
  pData->set_zoneid(m_zoneId);
  
  for (auto &v : m_mapTeamMembers)
  {
    TeamMember* pMem = pData->add_members();
    if (pMem)
    {
      pMem->CopyFrom(v.second);
    }
  }
}

void RoomTeam::setLeader(QWORD charId)
{
  QWORD oldId = m_qwLeaderId;
  m_qwLeaderId = charId;
  XDBG << "[斗技场-队长设置] roomid" << m_roomId << "teamid" << m_teamId << "leaderid" << charId <<"oldleader"<<oldId << XEND;
}

void RoomTeam::setTempLeader(QWORD charId)
{
  QWORD oldId = m_qwTempLeaderId;
  m_qwTempLeaderId = charId;
  XDBG << "[斗技场-临时队长设置] roomid" << m_roomId << "teamid" << m_teamId << "templeaderid" << charId << "oldleader" << oldId << XEND;
}

void RoomTeam::setOffline(QWORD charId, bool bOffline)
{
  if (bOffline)
    m_setOffline.insert(charId);
  else
    m_setOffline.erase(charId);
}

bool RoomTeam::isOffline(QWORD charId)
{
  auto it = m_setOffline.find(charId);
  if (it == m_setOffline.end())
    return false;
  return true;
}

bool RoomTeam::isAllOffline()
{
  DWORD offlineCount = m_setOffline.size();

  XDBG << "[斗技场-检查是否离线] roomid" << m_roomId <<"zoneid"<<m_zoneId<<"memsize"<<m_mapTeamMembers.size()<<"offlinesize"<<offlineCount << XEND;
  if (offlineCount == m_mapTeamMembers.size())
    return true;
  return false;
}

/************************************************************************/
/*TeamMatchRoom                                                                      */
/************************************************************************/
TeamMatchRoom::TeamMatchRoom(QWORD guid) :MatchRoom(guid)
{
}
TeamMatchRoom::~TeamMatchRoom()
{
}

EMatchRetCode TeamMatchRoom::checkCanJoin()
{
  if (m_state == EROOMSTATE_READY_FOR_FIGHT || m_state == EROOMSTATE_FIGHTING)
  {
    return EMATCHRETCODE_IS_FIGHTING;
  }

  if (m_vecRoomTeam.size() >= m_dwMaxTeamCount)
  {
    return EMATCHRETCODE_IS_FULL;
  }

  return EMATCHRETCODE_OK;
}

//房间详细信息
void TeamMatchRoom::toDetailData(Cmd::RoomDetailInfo* pRoomInfo)
{
  if (pRoomInfo == nullptr)
    return;
  MatchRoom::toDetailData(pRoomInfo);

  for (auto &v : m_vecRoomTeam)
  {
    v.toMatchTeamData(pRoomInfo->add_team_datas());
  }
}

RoomTeam* TeamMatchRoom::getTeam(DWORD zoneId, QWORD teamId)
{
  for (auto & v : m_vecRoomTeam)
  {
    if (v.getZoneId() == zoneId && v.getTeamId() == teamId)
    {
      return &v;
    }
  }
  return nullptr;
}

RoomTeam* TeamMatchRoom::getTeamByCharId(DWORD zoneId, QWORD charId)
{
  for (auto & v : m_vecRoomTeam)
  {
    if (v.getZoneId() == zoneId)
    {
      TeamMember*pTeamMem = v.getTeamMember(charId);
      if (pTeamMem)
        return &v;
    }
  }
  return nullptr;
}

RoomTeam* TeamMatchRoom::getTeamByNewTeamId(QWORD teamId)
{
  if (teamId == 0)
    return nullptr;
  for (auto &v : m_vecRoomTeam)
  {
    if (v.getNewTeamId() == teamId)
    {
      return &(v);
    }
  }
  return nullptr;
}

bool TeamMatchRoom::updateTeamMem(const MatchTeamMemUpdateInfo& rev)
{
  RoomTeam* pTeam = getTeam(rev.zoneid(), rev.teamid());
  if (pTeam == nullptr)
  {
    XERR << "[斗技场-队伍信息更新] 找不到报名队伍，roomid" << m_qwGuid << "zoneid" << rev.zoneid() << "teamid" << rev.teamid() << XEND;
    return false;
  }

  for (int i = 0; i < rev.updates_size(); ++i)
  {
    const TeamMember& memInfo = rev.updates(i);
    pTeam->addTeamMem(memInfo);
    MatchManager::getMe().addRoomUser(memInfo.guid(),rev.zoneid(), m_qwGuid);
    onTeamAddMember(pTeam, memInfo.guid(), rev.zoneid());
  }
  
  //sync to client
  {
    PvpTeamMemberUpdateCCmd clientCmd;
    if (clientCmd.mutable_data())
    {
      clientCmd.mutable_data()->CopyFrom(rev);
      clientCmd.mutable_data()->set_index(pTeam->getIndex());
    }
    PROTOBUF(clientCmd, send, len);
    sendCmdToAllMember(send, len);
  }

  if (rev.isfirst())
  {
    pTeam->setName(rev.teamname());
    pTeam->setInitOk();
    onOneTeamOk(pTeam);    
    return true;
  }

  for (int i = 0; i < rev.deletes_size(); ++i)
  {
    QWORD delCharId = rev.deletes(i);
    pTeam->delTeamMem(delCharId);
    MatchManager::getMe().delRoomUser(delCharId, rev.zoneid(), m_qwGuid);
    onTeamDelMember(pTeam, delCharId, rev.zoneid());
  }

  if (pTeam->isEmpty())
  {
    // delTeam();
    m_pRoomMgr->kickTeam(this, pTeam);
  }
  return true;
}

void TeamMatchRoom::onTeamDelMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId)
{
  if (pTeam == nullptr)
    return;

  if (m_state == EROOMSTATE_FIGHTING)
  {
    //踢掉pvp线的玩家
    if (pTeam->isTeamInPvp())
    {
      kickPvpTeamUser(pTeam, charId);
    }
  } // join -> teamserver -> syncteamdata,
}

bool TeamMatchRoom::updateTeamMemData(const MatchTeamMemDataUpdateInfo& rev)
{ 
  RoomTeam* pTeam = getTeam(rev.zoneid(), rev.teamid());
  if (pTeam == nullptr)
  {
    XERR << "[斗技场-队员信息更新] 找不到报名队伍，roomid" << m_qwGuid << "zoneid" << rev.zoneid() << "teamid" << rev.teamid() << XEND;
    return false;
  }
  
  TeamMember* pTeamMember = pTeam->getTeamMember(rev.charid());
  if (!pTeamMember)
  {
    return false;
  } 

  for (int i = 0; i < rev.members_size(); ++i)
  {
    const MemberData& rData = rev.members(i);
    switch (rData.type())
    {
    case EMEMBERDATA_OFFLINE:
      /*m->second.set_raidid(rData.value());*/
      pTeam->setOffline(rev.charid(), rData.value());
      updateOneData(pTeamMember, rData.type(), rData.value());
      break;
    case EMEMBERDATA_JOB:
      if (rData.value() == ETEAMJOB_LEADER)
        pTeam->setLeader(rev.charid());
      else if (rData.value() == ETEAMJOB_TEMPLEADER)
        pTeam->setTempLeader(rev.charid());
      updateOneData(pTeamMember, rData.type(), rData.value());
      break;
    default:
      break;
    }
  }

  //sync to client
  { 
    PvpMemberDataUpdateCCmd clientCmd;
    if (clientCmd.mutable_data())
    {
      clientCmd.mutable_data()->CopyFrom(rev);
    }
    PROTOBUF(clientCmd, send, len);
    sendCmdToAllMember(send, len);
  }

  XDBG << "斗技场-队员信息更新]" << rev.charid() << "更新队伍成员数据" << rev.ShortDebugString() << XEND; 
  if (!isFighting())
  {
    if (pTeam->isAllOffline())
    {
      m_pRoomMgr->kickTeam(this, pTeam);
    }
  }

  return true;
}

void TeamMatchRoom::updateOneData(TeamMember* pTeamMember, EMemberData eType, QWORD qwValue)
{
  if (!pTeamMember)
    return;
  
  for (int i = 0; i < pTeamMember->datas_size(); ++i)
  {
    MemberData* pData = pTeamMember->mutable_datas(i);
    if (pData && pData->type() == eType)
    {
      pData->set_value(qwValue);
      continue;
    }
  }  
}

bool TeamMatchRoom::checkIsFull()
{
  if (m_vecRoomTeam.size() == m_dwMaxTeamCount)
    return true;
  return false;
}

void TeamMatchRoom::ntfRoomState()
{
  Cmd::NtfRoomStateCCmd cmd;

  PROTOBUF(cmd, send, len);
  sendCmdToAllMember(send, len);
}

void TeamMatchRoom::sendCmdToAllMember(void * buf, WORD len)
{
  TSetDWORD exceptSet;
  return sendCmdToAllMember(buf, len, exceptSet);
}

void TeamMatchRoom::sendCmdToAllMember(void * buf, WORD len, const TSetDWORD&exceptSet)
{
  for (auto &v : m_vecRoomTeam)
  {
    const TMapRoomMember& roomMembers = v.getTeamMembers();
    for (auto &m : roomMembers)
    {
      if (!exceptSet.empty())
        if (exceptSet.find(m.second.guid()) != exceptSet.end())
          continue;

      bool ret = thisServer->sendCmdToClient(m.second.guid(), v.getZoneId(), buf, len);
      XDBG << "[斗技场] 发送协议到客户端 charid" << m.second.guid() << "zoneid" << v.getZoneId() << "ret" << ret << XEND;
    }
  }
}

void TeamMatchRoom::onOneTeamOk(RoomTeam* pTeam)
{
  if (!pTeam)
    return;

  //同步房间信息给队伍所有人

  ReqMyRoomMatchCCmd reqMyRoom;
  reqMyRoom.set_type(getType());
  ReqRoomDetailCCmd reqRoomDetail;
  reqRoomDetail.set_type(getType());
  reqRoomDetail.set_roomid(m_qwGuid);
  
  const TMapRoomMember& members =  pTeam->getTeamMembers();
  for (auto &m : members)
  {
    m.second.guid();
    MatchManager::getMe().reqMyRoom(m.second.guid(), pTeam->getZoneId(), reqMyRoom);
    MatchManager::getMe().reqRoomDetail(m.second.guid(), pTeam->getZoneId(), reqRoomDetail);
  }  
}

void TeamMatchRoom::timeTick(DWORD curSec)
{
  v_timeTick(curSec);
}

//warning:pTeam 在函数里会被删除
bool TeamMatchRoom::kickTeam(RoomTeam* pTeam, bool bDelTeam/*=true*/)
{
  if (pTeam == nullptr)
    return false;

  KickTeamCCmd msg;
  msg.set_type(getType());
  msg.set_roomid(m_qwGuid);
  msg.set_teamid(pTeam->getTeamId());
  msg.set_zoneid(pTeam->getZoneId());

  PROTOBUF(msg, send, len);
  sendCmdToAllMember(send, len);

  const TMapRoomMember& members = pTeam->getTeamMembers();
  for (auto &m : members)
  {
    MatchManager::getMe().delRoomUser(m.first, pTeam->getZoneId(),m_qwGuid);
  }

  //通知teamserver 不再同步队伍信息到Matchserver

  {
    NtfLeaveRoom ntfLeave;
    ntfLeave.set_teamid(pTeam->getTeamId());
    ntfLeave.set_roomid(getGuid());
    PROTOBUF(ntfLeave, send, len);
    bool ret = thisServer->sendCmdToZone(pTeam->getZoneId(), send, len);

    XLOG << "[斗技场-踢出退伍] 通知teamserver，type" << getType() << "roomid" << getGuid() << "发起者" << pTeam->getName() << "发起者zoneid" << pTeam->getZoneId() << "发起者队伍id" << pTeam->getTeamId() << "ret" << ret << XEND;
  }

  XLOG << "[斗技场-踢出退伍] type" << getType() << "roomid" << getGuid() << "发起者" << pTeam->getName() << "发起者zoneid" << pTeam->getZoneId() << "发起者队伍id" << pTeam->getTeamId() << XEND;
  
  onOneTeamDel(pTeam);

  //del team
  if (bDelTeam)
  {
    for (auto it = m_vecRoomTeam.begin(); it != m_vecRoomTeam.end(); ++it)
    {
      if (&(*it) == pTeam)
      {
        m_vecRoomTeam.erase(it);
        break;
      }
    }
  }

  return true;
}

bool TeamMatchRoom::reset()
{
  for (auto it = m_vecRoomTeam.begin(); it != m_vecRoomTeam.end(); ++it)
  {
    kickTeam(&(*it), false);
  }  
  m_vecRoomTeam.clear();
  setState(EROOMSTATE_WAIT_JOIN);
  setTeamCount(m_dwMaxTeamCount);
  return true;
}

bool TeamMatchRoom::createNewTeam(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return false;

  QWORD qwLeader = pTeam->getOnlineLeader();
  if (!qwLeader)
  {
    XERR << "[斗技场-创建队伍] 失败，找不到队长 type" << getType() << "roomid" << getGuid() << "teamid" << pTeam->getTeamId() << "zoneid" << pTeam->getZoneId() << XEND;
    return false;
  }

  CreateTeamMatchSCmd cmd;
  cmd.set_roomid(getGuid());
  cmd.set_charid(qwLeader);
  cmd.set_zoneid(pTeam->getZoneId());
  cmd.set_name(pTeam->getName());
  cmd.set_teamid(pTeam->getTeamId());
  cmd.set_pvptype(getType());

  PROTOBUF(cmd, send, len);
  
  DWORD destZoneId = 0;
  switch (getType())
  {
  case EPVPTYPE_SMZL:
    destZoneId = thisServer->m_dwSMZLZoneid;
    break;
  case EPVPTYPE_HLJS:
    destZoneId = thisServer->m_dwHLJSZoneid;
    break;
  case EPVPTYPE_MVP:
    {
      MvpMatchRoom* pRoom = dynamic_cast<MvpMatchRoom*>(this);
      if (pRoom == nullptr)
        return false;
      destZoneId = pRoom->getZoneID();
    }
    break;
  case EPVPTYPE_SUGVG:
    {
      SuperGvgMatchRoom* pRoom = dynamic_cast<SuperGvgMatchRoom*>(this);
      if (pRoom == nullptr)
        return false;
      destZoneId = pRoom->getZoneID();
    }
    break;
  case EPVPTYPE_TEAMPWS:
  case EPVPTYPE_TEAMPWS_RELAX:
    {
      TeamPwsMatchRoom* pRoom = dynamic_cast<TeamPwsMatchRoom*> (this);
      if (pRoom == nullptr)
        return false;
      destZoneId = pRoom->getZoneID();
    }
    break;
  default:
    break;
  }
  
  bool ret = thisServer->forwardCmdToTeamServer(destZoneId, send, len);

  XLOG << "[斗技场-创建队伍] 成功，发送到Teamserver type" << getType() << "roomid" << getGuid() << "teamid" << pTeam->getTeamId() << "zoneid" << pTeam->getZoneId() << "发送到" << destZoneId << "ret" << ret << XEND;
  return ret;
}

bool TeamMatchRoom::applyNewTeam(RoomTeam* pTeam)
{
 /* if (pTeam == nullptr)
    return false;

  TeamMember* pLeader = pTeam->getTeamLeader();
  if (!pLeader)
  {
    XERR << "[斗技场-创建队伍] 失败，找不到队长 type" << getType() << "roomid" << getGuid() << "teamid" << pTeam->getTeamId() << "zoneid" << pTeam->getZoneId() << XEND;
    return false;
  }

  CreateTeamMatchSCmd cmd;
  cmd.set_roomid(getGuid());
  cmd.set_charid(pLeader->guid());
  cmd.set_zoneid(pTeam->getZoneId());
  cmd.set_name(pTeam->getName());
  cmd.set_teamid(pTeam->getTeamId());

  PROTOBUF(cmd, send, len);

  DWORD destZoneId = 0;
  switch (getType())
  {
  case EPVPTYPE_SMZL:
    destZoneId = thisServer->m_dwSMZLZoneid;
    break;
  case EPVPTYPE_HLJS:
    destZoneId = thisServer->m_dwHLJSZoneid;
    break;
  default:
    break;
  }

  bool ret = thisServer->forwardCmdToTeamServer(destZoneId, send, len);

  XLOG << "[斗技场-创建队伍] 成功，发送到Teamserver type" << getType() << "roomid" << getGuid() << "teamid" << pTeam->getTeamId() << "zoneid" << pTeam->getZoneId() << "发送到" << destZoneId << "ret" << ret << XEND;
  return ret;*/
  return true;
}

void TeamMatchRoom::kickPvpTeamUser(RoomTeam* pTeam, QWORD charId)
{
  if (!pTeam)
    return;

  //kick team
  KickTeamMatchSCmd cmd;
  cmd.set_teamid(pTeam->getNewTeamId());
  cmd.set_charid(charId);
  PROTOBUF(cmd, send, len);
  DWORD destZoneId = 0;
  switch (getType())
  {
  case EPVPTYPE_SMZL:
    destZoneId = thisServer->m_dwSMZLZoneid;
    break;
  case EPVPTYPE_HLJS:
    destZoneId = thisServer->m_dwHLJSZoneid;
    break;
  case EPVPTYPE_MVP:
    {
      MvpMatchRoom* pRoom = dynamic_cast<MvpMatchRoom*>(this);
      if (pRoom == nullptr)
        return;
      destZoneId = pRoom->getZoneID();
    }
    break;
  case EPVPTYPE_SUGVG:
    {
      // 决战副本队伍状态与原副本状态不同步
      return;
      /*SuperGvgMatchRoom* pRoom = dynamic_cast<SuperGvgMatchRoom*>(this);
      if (pRoom == nullptr)
        return;
      destZoneId = pRoom->getZoneID();
      */
    }
    break;
  default:
    break;
  }
  bool ret = thisServer->forwardCmdToTeamServer(destZoneId, send, len);
  XLOG << "[斗技场-踢掉pvp线队伍玩家] 发送到Teamserver type" << getType() << "roomid" << getGuid() << "oldteamid" << pTeam->getTeamId() << "newteamid" << pTeam->getNewTeamId() << "zoneid" << pTeam->getZoneId()<<"charid"<<charId << "发送到" << destZoneId << "ret" << ret << XEND;
  return;
}

void TeamMatchRoom::kickOldTeamUser(Cmd::KickTeamMatchSCmd& rev)
{
  RoomTeam* pTeam = getTeamByNewTeamId(rev.teamid());
  if (!pTeam)
  {
    XERR << "[斗技场-踢掉原来线队伍玩家] 失败，找不到队伍 roomid" << getGuid() << "newteamid" << rev.teamid() << "踢" << rev.charid() << XEND;
    return;
  }
  
  //kick team
  KickTeamMatchSCmd cmd;
  cmd.set_teamid(pTeam->getTeamId());
  cmd.set_charid(rev.charid());
  PROTOBUF(cmd, send, len);
  DWORD destZoneId = pTeam->getZoneId();
  bool ret = thisServer->forwardCmdToTeamServer(destZoneId, send, len);
  XLOG << "[斗技场-踢掉原来线队伍玩家] 成功，找不到队伍 roomid" << getGuid() << "newteamid" << rev.teamid() << "踢" << rev.charid()<<"发送到zoenid"<<destZoneId<<"ret"<<ret << XEND;
  return;
}

bool TeamMatchRoom::createNewTeamRes(DWORD zoneId, QWORD oldTeamId, QWORD leaderId, QWORD newTeamId)
{
  RoomTeam* pTeam = getTeam(zoneId, oldTeamId);
  if (!pTeam)
  {
    XERR << "[斗技场-创建队伍-TeamServer返回] 失败，找不到队伍" << getType() << "roomid" << getGuid() << "teamid" << oldTeamId << "zoneid" << zoneId << "leaderid" << leaderId << "newTeamid" << newTeamId << XEND;
    return false;
  }

  pTeam->setNewTeamId(leaderId, newTeamId);

  XLOG << "[斗技场-创建队伍-TeamServer返回] 成功" << getType() << "roomid" << getGuid() << "teamid" << oldTeamId << "zoneid" << zoneId << "leaderid" << leaderId << "newTeamid" << newTeamId << XEND;

  onNewTeamOk(pTeam);

  return true;
}

void TeamMatchRoom::onUserOnline(const SocialUser& rUser)
{
}

void TeamMatchRoom::onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId)
{
  if (!isFighting())
    return;
  
  RoomTeam* pTeam = getTeamByCharId(oldZoneId, rUser.charid());
  if (!pTeam)
  {
    return;
  }
  if (!pTeam->isTeamInPvp())
  {
    return;
  }

  //apply team
  ApplyTeamMatchSCmd cmd;
  cmd.set_teamid(pTeam->getNewTeamId());
  cmd.set_charid(rUser.charid());
  cmd.set_zoneid(rUser.zoneid());
  PROTOBUF(cmd, send, len);
  DWORD destZoneId = 0;
  switch (getType())
  {
  case EPVPTYPE_SMZL:
    destZoneId = thisServer->m_dwSMZLZoneid;
    break;
  case EPVPTYPE_HLJS:
    destZoneId = thisServer->m_dwHLJSZoneid;
    break;
  case EPVPTYPE_MVP:
    {
      MvpMatchRoom* pRoom = dynamic_cast<MvpMatchRoom*>(this);
      if (pRoom == nullptr)
        return;
      destZoneId = pRoom->getZoneID();
    }
    break;
  default:
    break;
  }
  bool ret = thisServer->forwardCmdToTeamServer(destZoneId, send, len);
  XLOG << "[斗技场-申请加入队伍] 发送到Teamserver type" << getType() << "charid" << rUser.charid() << "roomid" << getGuid() << "oldteamid" << pTeam->getTeamId() << "newteamid" << pTeam->getNewTeamId() << "zoneid" << pTeam->getZoneId() << "发送到" << destZoneId << "ret" << ret << XEND;

  //send index to scene 不需要
  //{
  //  SyncTeamInfoMatchSCmd cmd;
  //  cmd.set_teamid(pTeam->getNewTeamId());
  //  cmd.set_charid(rUser.charid());
  //  cmd.set_index(pTeam->getIndex());
  //  PROTOBUF(cmd, send, len);
  //  bool ret = thisServer->forwardCmdToSceneServer(rUser.charid(), destZoneId, send, len);

  //  XLOG << "[斗技场-同步队伍所以到场景] 发送到SceneServer type" << getType() << "charid" << rUser.charid() << "roomid" << getGuid() << "oldteamid" << pTeam->getTeamId() << "newteamid" << pTeam->getNewTeamId()<<"index"<< pTeam->getIndex() << "zoneid" << pTeam->getZoneId() << "发送到" << destZoneId << "ret" << ret << XEND;
  //}
}

void TeamMatchRoom::onUserOffline(const SocialUser& rUser)
{
}

void TeamMatchRoom::onUserOfflinePvp(const SocialUser& rUser, DWORD oldZoneId)
{

}

void TeamMatchRoom::setTeamCount(DWORD count)
{
  m_dwMaxTeamCount = count;
  if (m_dwMaxTeamCount == 0)
    return;
  for (DWORD i = 1; i <= m_dwMaxTeamCount; ++i)
  {
    m_setIndex.insert(i);
  }
}

DWORD TeamMatchRoom::getIndex()
{
  if (m_setIndex.empty())
    return 0;

  return *(m_setIndex.begin());
}

void TeamMatchRoom::lockIndex(DWORD i)
{
  m_setIndex.erase(i);
}

void TeamMatchRoom::unlockIndex(DWORD i)
{
  m_setIndex.insert(i);
}

void TeamMatchRoom::sendRoomStat()
{
  NtfRoomStateCCmd cmd;
  cmd.set_pvp_type(getType());
  cmd.set_roomid(m_qwGuid);
  cmd.set_state(m_state);
  
  PROTOBUF(cmd, send, len); 
  sendCmdToAllMember(send, len);
  XDBG << "[斗技场-同步房间状态给所有玩家] type" << getType() << "roomid" << m_qwGuid << "state" << m_state << XEND;
}

void TeamMatchRoom::getAllUsers(TSetQWORD& userids)
{
  for (auto &v : m_vecRoomTeam)
  {
    const TMapRoomMember& mems = v.getTeamMembers();
    for (auto &u : mems)
      userids.insert(u.first);
  }
}

/************************************************************************/
/*SMZLMatchRoom                                                                      */
/************************************************************************/
SMZLMatchRoom::SMZLMatchRoom(QWORD guid) :TeamMatchRoom(guid)
{
}
SMZLMatchRoom::~SMZLMatchRoom()
{
}

//
void SMZLMatchRoom::v_timeTick(DWORD curSec)
{
  if (m_state == EROOMSTATE_MATCH_SUCCESS)
  {
    if (m_challengeReplyTime && curSec > m_challengeReplyTime)
    {
      //房主拒绝挑战,踢掉第二个队伍
      XLOG << "[斗技场-加入战斗-沙漠之狼] 房主10秒内未接受挑战，默认接受挑战踢出第二个队伍, roomid" << getGuid() << "tozoneid" << thisServer->m_dwSMZLZoneid << "raidid" << getRaidId() << XEND;
      processChallengeRes(0, EMATCHREPLY_REFUSE);
    }
  }  
}

void SMZLMatchRoom::onOneTeamOk(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;
  
  TeamMatchRoom::onOneTeamOk(pTeam);
  
  if (!checkIsFull())
    return;
  challenge();
}

void SMZLMatchRoom::onOneTeamDel(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;
  if (isFighting())
  {
    return;
  }
  unlockIndex(pTeam->getIndex());
  setState(EROOMSTATE_WAIT_JOIN);
  sendRoomStat();
  stopCountDown();
}

void SMZLMatchRoom::onNewTeamOk(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;
  m_dwOkTeamCount++;
  pTeam->setTeamInPvp();
  
  XLOG << "[斗技场-沙漠之狼]，新队伍创建完毕 roomid" << m_qwGuid << "teamid" << pTeam->getTeamId() << "newteamid" << pTeam->getNewTeamId() << "teamzoneid" << pTeam->getZoneId() << "index" << pTeam->getIndex() << "raidid" << getRaidId() <<"okteamcount"<<m_dwOkTeamCount << XEND;
  
  if (m_dwOkTeamCount == m_dwMaxTeamCount)
  {
    sendConfirm();
    m_dwOkTeamCount = 0;
  }
}

void SMZLMatchRoom::onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId)
{
  TeamMatchRoom::onUserOnlinePvp(rUser, oldZoneId);
  return;
}

bool SMZLMatchRoom::addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  DWORD index = getIndex();
  RoomTeam team(zoneId, rev.teamid(), index, m_qwGuid, charId);
  m_vecRoomTeam.push_back(team);
  lockIndex(index);
  return true;
}

bool SMZLMatchRoom::challenge()
{
  //向他发起挑战
  
  RoomTeam& firtTeam = m_vecRoomTeam[0];
  RoomTeam& secondTeam = m_vecRoomTeam[1];

  QWORD qwLeader = firtTeam.getOnlineLeader();
  if (qwLeader == 0)
  {
    //
    XERR << "[斗技场-挑战] 找不到队长,teamid" << firtTeam.getTeamId() << XEND;
    return false;
  }

  RevChallengeCCmd msg;
  msg.set_type(getType());
  msg.set_roomid(m_qwGuid);
  msg.set_challenger(secondTeam.getName());
  msg.set_challenger_zoneid(secondTeam.getZoneId()); 

  const TMapRoomMember& members = secondTeam.getTeamMembers();
  for (auto &m : members)
  {
    msg.add_members()->CopyFrom(m.second);
  }
  
  PROTOBUF(msg, send, len);  
  bool ret = thisServer->sendCmdToClient(qwLeader, firtTeam.getZoneId(), send, len);

  XLOG << "[斗技场-发起挑战] 通知房主队长，type" << getType() << "roomid" << getGuid() << "接受者" << qwLeader << "zoneid" << firtTeam.getZoneId() << "发起者" << secondTeam.getName() << "发起者zoneid" << secondTeam.getZoneId() << "发起者队伍id" << secondTeam.getTeamId() << "ret"<<ret<< XEND;
  
  m_challengeReplyTime = now() + 11;
  setState(EROOMSTATE_MATCH_SUCCESS);
  sendRoomStat();
  return true;
}

bool SMZLMatchRoom::processChallengeRes(QWORD charId, EMatchReply reply)
{
  if (!checkIsFull())
  {
    if (reply == EMATCHREPLY_AGREE)
    {
      //TODO msg
    }
    m_challengeReplyTime = 0;
    setState(EROOMSTATE_WAIT_JOIN);
    sendRoomStat();
    
    XERR << "[斗技场-挑战-沙漠之狼] 另一个队伍已经离开房间 type" << getType() << "roomid" << m_qwGuid << "charid" << charId<<"当前房间人数"<<m_vecRoomTeam.size() << XEND;
    return false;
  }

  if (reply == EMATCHREPLY_REFUSE)
  {
    RoomTeam& secondTeam = m_vecRoomTeam[1];  
    kickTeam(&secondTeam);
    m_challengeReplyTime = 0;
    setState(EROOMSTATE_WAIT_JOIN);
    sendRoomStat();
    return true;
  }  
  //
  stopCountDown();
  m_challengeReplyTime = 0;
  
  //
  //队长去新线创建队伍
  
  RoomTeam& firstTeam = m_vecRoomTeam[0];
  RoomTeam& secondTeam = m_vecRoomTeam[1];  
  createNewTeam(&firstTeam);
  createNewTeam(&secondTeam);
 
  return true;
}

void SMZLMatchRoom::sendConfirm()
{
  //两个队长进去，其他人等待确认
  if (!checkIsFull())
    return;
  
  RoomTeam& firstTeam = m_vecRoomTeam[0];
  RoomTeam& secondTeam = m_vecRoomTeam[1];
  MatchManager::getMe().sendPlayerToFighting(firstTeam.getZoneId(), thisServer->m_dwSMZLZoneid, firstTeam.getOnlineLeader(), getRaidId(), getGuid(), firstTeam.getIndex());
  MatchManager::getMe().sendPlayerToFighting(secondTeam.getZoneId(), thisServer->m_dwSMZLZoneid, secondTeam.getOnlineLeader(), getRaidId(), getGuid(), secondTeam.getIndex());
  //send confirm 
  FightConfirmCCmd cmd;
  cmd.set_type(getType());
  cmd.set_roomid(getGuid());
  cmd.set_teamid(secondTeam.getTeamId());

  //接受者
  cmd.set_challenger(secondTeam.getName());
  PROTOBUF(cmd, send, len);
  firstTeam.sendCmdToAllMember(send, len, firstTeam.getOnlineLeader());

  //挑战者
  cmd.set_challenger(firstTeam.getName());
  PROTOBUF(cmd, send2, len2);
  secondTeam.sendCmdToAllMember(send2, len2, secondTeam.getOnlineLeader());

  m_confiremReplyTime = now() + 10 + 1;   //10秒确认时间。
  setState(EROOMSTATE_FIGHTING);
  getMatchRoomMgr()->setRoomFighting(this);
  sendRoomStat();
  XLOG << "[斗技场-加入战斗] 发送确认给客户端。roomid" << getGuid() << XEND;
}

void SMZLMatchRoom::sendComfirmRes(DWORD zoneId, QWORD charId, QWORD teamId)
{
  DWORD curSec = now();
  if (m_confiremReplyTime == 0)
    return;
  if (curSec > m_confiremReplyTime)
    return;
  
  RoomTeam* pTeam = getTeam(zoneId, teamId);
  if (pTeam == nullptr)
  {
    return;
  }
  if (pTeam->getTeamMember(charId) == nullptr)
  {
    return;
  }  
 
  MatchManager::getMe().sendPlayerToFighting(pTeam->getZoneId(), thisServer->m_dwSMZLZoneid, charId, getRaidId(), getGuid(), pTeam->getIndex());
  
  XLOG << "[斗技场-沙漠之狼-加入战斗] 玩家确认加入 roomid" << getGuid() << "zoneid" << zoneId << "charid" << charId << "teamid" << teamId << XEND;
}

//房间简单信息
void SMZLMatchRoom::toBriefData(Cmd::RoomBriefInfo* pRoomInfo)
{
  if (pRoomInfo == nullptr)
    return;
  MatchRoom::toBriefData(pRoomInfo);

  if (m_vecRoomTeam.empty())
  {
    XERR << "[斗技场-异常]" << m_qwGuid << "size 0 " << XEND;
    return;
  }
  pRoomInfo->set_zoneid(m_vecRoomTeam[0].getZoneId());
}

bool SMZLMatchRoom::reset()
{
  TeamMatchRoom::reset();
  m_dwCountEndTime = 0;
  m_challengeReplyTime = 0;     //发起挑战回复截止时间
  m_confiremReplyTime = 0;      //进入房间确认截止时间
  m_dwOkTeamCount = 0;
  return true;
}

void SMZLMatchRoom::onTeamAddMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId)
{
  if (m_state == EROOMSTATE_READY_FOR_FIGHT)
  {
    if (m_confiremReplyTime > now())
    {
      //send confirm
      FightConfirmCCmd cmd;
      cmd.set_type(getType());
      cmd.set_roomid(getGuid());
      PROTOBUF(cmd, send, len);           
      thisServer->sendCmdToClient(charId, zoneId, send, len);
    }
  }
}

void SMZLMatchRoom::startCountDown(DWORD zoneId, QWORD charId)
{
  m_qwCountDownCharid = charId;
  m_dwCountDownZoneId = zoneId;
  MatchManager::getMe().sendSysMsg(m_qwCountDownCharid, m_dwCountDownZoneId, 956, MsgParams(10), EMESSAGETYPE_TIME_DOWN);
}

void SMZLMatchRoom::stopCountDown()
{
  if (m_dwCountDownZoneId)
    MatchManager::getMe().sendSysMsg(m_qwCountDownCharid, m_dwCountDownZoneId, 956, MsgParams(10), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
  m_dwCountDownZoneId = 0;
}

/************************************************************************/
/*HLJSMatchRoom                                                                      */
/************************************************************************/
HLJSMatchRoom::HLJSMatchRoom(QWORD guid) :TeamMatchRoom(guid)
{
}
HLJSMatchRoom::~HLJSMatchRoom()
{
}

bool HLJSMatchRoom::addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  DWORD index = getIndex();
  RoomTeam team(zoneId, rev.teamid(), index, m_qwGuid, charId);
  m_vecRoomTeam.push_back(team);
  
  lockIndex(index);
  return true;
}

void HLJSMatchRoom::startCountdown()
{
  setState(EROOMSTATE_READY_FOR_FIGHT);
  m_dwCountEndTime = now() + 10;  
  sendRoomStat();
  
  for (auto &v : m_vecRoomTeam)
  {
    const TMapRoomMember& roomMembers = v.getTeamMembers();
    for (auto &m : roomMembers)
    {   
      MatchManager::getMe().sendSysMsg(m.second.guid(), v.getZoneId(), 956, MsgParams(10), EMESSAGETYPE_TIME_DOWN);
    }
  }

  XDBG << "[斗技场-华丽金属-进入倒计时开始] roomid" << m_qwGuid << XEND;
}

void HLJSMatchRoom::stopCountdown()
{
  setState(EROOMSTATE_WAIT_JOIN);
  m_dwCountEndTime = 0;
  sendRoomStat();

  for (auto &v : m_vecRoomTeam)
  {
    const TMapRoomMember& roomMembers = v.getTeamMembers();
    for (auto &m : roomMembers)
    {
      MatchManager::getMe().sendSysMsg(m.second.guid(), v.getZoneId(), 956, MsgParams(10), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
    }
  }
  XDBG << "[斗技场-华丽金属-进入倒计时终止] roomid" << m_qwGuid << XEND;
}

void HLJSMatchRoom::v_timeTick(DWORD curSec)
{
 if (m_state == EROOMSTATE_READY_FOR_FIGHT)
 {
   if (curSec > m_dwCountEndTime)
   {
     XDBG << "[斗技场-华丽金属]进入倒计时结束，创建队伍 roomid" <<m_qwGuid<< XEND;
     setState(EROOMSTATE_FIGHTING);    
     getMatchRoomMgr()->setRoomFighting(this);
     for (auto &v : m_vecRoomTeam)
     {
       //create NEW team
       createNewTeam(&v);
     }
   }
 }
}

void HLJSMatchRoom::onOneTeamOk(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;

  TeamMatchRoom::onOneTeamOk(pTeam);

  //已加入华丽金属抢夺战房间等待（960）  
  const TMapRoomMember& temMem = pTeam->getTeamMembers();
  for (auto it = temMem.begin(); it != temMem.end(); ++it)
  {
    MatchManager::getMe().sendSysMsg(it->first, pTeam->getZoneId(), 960);
  }

  if (!checkIsFull())
  {
    return;
  }

  //播放倒计时 20s
  startCountdown();
}

void HLJSMatchRoom::onOneTeamDel(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;
  
  if (isFighting())
  {
    return;
  }
    //free index
  unlockIndex(pTeam->getIndex());
  
  if (m_state == EROOMSTATE_READY_FOR_FIGHT)
  {
    stopCountdown();
  }

  setState(EROOMSTATE_WAIT_JOIN);
  m_dwCountEndTime = 0;
  sendRoomStat();
}

void HLJSMatchRoom::onNewTeamOk(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;
  pTeam->setTeamInPvp();
  pTeam->sendAllUserToFight(thisServer->m_dwHLJSZoneid, getRaidId(), m_qwGuid);

  XLOG << "[斗技场-华丽金属]，新队伍创建完毕，传送队员到场景 roomid" << m_qwGuid << "destzoneid" << thisServer->m_dwHLJSZoneid << "teamid" << pTeam->getTeamId() << "newteamid" << pTeam->getNewTeamId() << "teamzoneid" << pTeam->getZoneId() << "index" << pTeam->getIndex() << "raidid" << getRaidId() << XEND;
}

void HLJSMatchRoom::onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId)
{
  TeamMatchRoom::onUserOnlinePvp(rUser, oldZoneId);

  //if (!isFighting())
  //  return;

  //RoomTeam* pTeam = getTeamByCharId(rUser.charid());
  //if (!pTeam)
  //{
  //  return;
  //}
  //if (!pTeam->isTeamInPvp())
  //{
  //  return;
  //}

  ////apply team
  //ApplyTeamMatchSCmd cmd;
  //cmd.set_teamid(pTeam->getNewTeamId());
  //cmd.set_charid(rUser.charid());
  //cmd.set_zoneid(rUser.zoneid());
  //PROTOBUF(cmd, send, len);
  //DWORD destZoneId = 0;
  //switch (getType())
  //{
  //case EPVPTYPE_SMZL:
  //  destZoneId = thisServer->m_dwSMZLZoneid;
  //  break;
  //case EPVPTYPE_HLJS:
  //  destZoneId = thisServer->m_dwHLJSZoneid;
  //  break;
  //default:
  //  break;
  //}
  //bool ret = thisServer->forwardCmdToTeamServer(destZoneId, send, len);
  //XLOG << "[斗技场-申请加入队伍] 发送到Teamserver type" << getType() << "charid"<<rUser.charid()<< "roomid" << getGuid() << "oldteamid" << pTeam->getTeamId() << "newteamid" << pTeam->getNewTeamId() << "zoneid" << pTeam->getZoneId() << "发送到" << destZoneId << "ret" << ret << XEND;

  ////send index to scene
  //{
  //  SyncTeamInfoMatchSCmd cmd;
  //  cmd.set_teamid(pTeam->getNewTeamId());
  //  cmd.set_charid(rUser.charid());
  //  cmd.set_index(pTeam->getIndex());
  //  PROTOBUF(cmd, send, len);
  //  bool ret = thisServer->forwardCmdToSceneServer(rUser.charid(), destZoneId, send, len);
  //  
  //  XLOG << "[斗技场-同步队伍所以到场景] 发送到SceneServer type" << getType() << "charid" << rUser.charid() << "roomid" << getGuid() << "oldteamid" << pTeam->getTeamId() << "newteamid" << pTeam->getNewTeamId() << "zoneid" << pTeam->getZoneId() << "发送到" << destZoneId << "ret" << ret << XEND;
  //}
}

//房间简单信息
void HLJSMatchRoom::toBriefData(Cmd::RoomBriefInfo* pRoomInfo)
{
  if (pRoomInfo == nullptr)
    return;
  MatchRoom::toBriefData(pRoomInfo);

  //TODO 缓存
  for (auto&v : m_vecRoomTeam)
  {
    DWORD teamMemCount = v.getTeamMemCount();
    switch (v.getIndex())
    {
    case 1:
      pRoomInfo->set_num1(teamMemCount);
      break;
    case 2:
      pRoomInfo->set_num2(teamMemCount);
      break;
    case 3:
      pRoomInfo->set_num3(teamMemCount);
      break;
    default:
      break;
    }
  }
}

bool HLJSMatchRoom::reset()
{
  TeamMatchRoom::reset();
  m_dwCountEndTime = 0;
  return true;
}

void HLJSMatchRoom::onTeamAddMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId)
{
  if (m_state == EROOMSTATE_READY_FOR_FIGHT)
  {
    if (m_dwCountEndTime > now())
    {
      DWORD leftSec = m_dwCountEndTime - now();
      MatchManager::getMe().sendSysMsg(charId, zoneId, 956, MsgParams(leftSec), EMESSAGETYPE_TIME_DOWN);
    }
  }  
}

void HLJSMatchRoom::onTeamDelMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId)
{
  TeamMatchRoom::onTeamDelMember(pTeam, charId, zoneId);

  if (m_state == EROOMSTATE_READY_FOR_FIGHT)
  {
    if (m_dwCountEndTime > now())
    {
      DWORD leftSec = m_dwCountEndTime - now();
      MatchManager::getMe().sendSysMsg(charId, zoneId, 956, MsgParams(leftSec), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
    }
  }
}

PollyMatchRoom::PollyMatchRoom(QWORD guid) :MatchRoom(guid)
{
}

PollyMatchRoom::~PollyMatchRoom()
{}

/************************************************************************/
/*MvpMatchRoom*/
/************************************************************************/
MvpMatchRoom::MvpMatchRoom(QWORD guid, MatchRoomMgr* pRoomMgr) : TeamMatchRoom(guid)
{
  m_pRoomMgr = pRoomMgr;
}

MvpMatchRoom::~MvpMatchRoom()
{
  reset();
}

/*
 * EROOMSTATE_WAIT_JOIN = 1;        //等待加入
 * EROOMSTATE_MATCH_SUCCESS = 2;    //匹配成功，10s后进入战斗准备
 * EROOMSTATE_READY_FOR_FIGHT = 3;  //10s 战斗准备
 * EROOMSTATE_FIGHTING = 4;         //战斗中
 * EROOMSTATE_END = 5;              //结束---
 *
*/
void MvpMatchRoom::v_timeTick(DWORD curSec)
{
  if (m_state == EROOMSTATE_WAIT_JOIN)
  {
    if (!m_setKickTeams.empty())
    {
      for (auto &p : m_setKickTeams)
      {
        auto it = find_if(m_vecRoomTeam.begin(), m_vecRoomTeam.end(), [&](const RoomTeam& r) -> bool
        {
          return &r == p;
        });
        /*避免野指针*/
        if (it != m_vecRoomTeam.end())
          kickTeam(p);
      }
      m_setKickTeams.clear();
    }

    if (checkCanStart(curSec))
    {
      onStart();
    }
    else
    {
      for (auto m = m_mapTeam2MatchTimeOut.begin(); m != m_mapTeam2MatchTimeOut.end(); )
      {
        // 匹配超时
        if (curSec < m->second)
        {
          ++m;
          continue;
        }
        RoomTeam* pTeam = nullptr;
        for (auto &v : m_vecRoomTeam)
        {
          if (v.getTeamId() != m->first)
            continue;
          pTeam = &(v);
        }
        if (pTeam == nullptr)
        {
          m = m_mapTeam2MatchTimeOut.erase(m);
          continue;
        }
        NtfMatchInfoCCmd cmd;
        cmd.set_etype(EPVPTYPE_MVP);
        cmd.set_ismatch(false);
        PROTOBUF(cmd, send, len);
        pTeam->sendCmdToAllMember(send, len, 0);
        pTeam->sendMsgToAllMember(25604);
        XLOG << "[Mvp-竞争者], 匹配超时, 踢出队伍, 房间:" << m_qwGuid << "队伍:" << pTeam->getTeamId() << XEND;

        kickTeam(pTeam);
        m = m_mapTeam2MatchTimeOut.erase(m);
        continue;
      }
    }
  }
}

void MvpMatchRoom::onNewTeamOk(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;

  pTeam->setTeamInPvp();
  pTeam->sendAllUserToFight(m_dwZoneID, getRaidId(), m_qwGuid);
  XLOG << "[Mvp-竞争战], 新队伍创建完毕, 房间:" << m_qwGuid << "队伍id:" << pTeam->getTeamId() << "新队伍id:" << pTeam->getNewTeamId() << "旧的队伍所在线:" << pTeam->getZoneId() << XEND;
}

bool MvpMatchRoom::checkCanStart(DWORD time)
{
  if (m_vecRoomTeam.empty() || time < m_dwBeginMatchTime)
    return false;
  return MiscConfig::getMe().getMvpBattleCFG().checkMatchOk(time - m_dwBeginMatchTime, m_vecRoomTeam.size());
}

bool MvpMatchRoom::addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  //DWORD index = getIndex();
  RoomTeam team(zoneId, rev.teamid(), 0, m_qwGuid, charId);
  m_vecRoomTeam.push_back(team);
  m_mapTeam2MatchTimeOut[rev.teamid()] = now() + MiscConfig::getMe().getMvpBattleCFG().dwMaxMatchTime;
  //lockIndex(index);
  return true;
}

// match->team, join; team->match, syncteamdata; recv teamdata:
void MvpMatchRoom::onOneTeamOk(RoomTeam* pTeam)
{
  // 已匹配完成, 添加惩罚时间
  if (!pTeam)
    return;
  if (m_state == EROOMSTATE_FIGHTING)
  {
    MvpMatchRoomMgr* pMgr = dynamic_cast<MvpMatchRoomMgr*> (m_pRoomMgr);
    if (pMgr != nullptr)
    {
      const TMapRoomMember& mems = pTeam->getTeamMembers();
      for (auto &u : mems)
      {
        pMgr->addUserPunishTime(u.first, m_dwPunishTime);
        m_setRoomUsers.insert(u.first);
      }
    }

    createNewTeam(pTeam);
  }
}

void MvpMatchRoom::onOneTeamDel(RoomTeam* pTeam)
{
}

void MvpMatchRoom::onTeamAddMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId)
{
  // 通知匹配等待状态
  if (m_state == EROOMSTATE_WAIT_JOIN)
  {
    MvpMatchRoomMgr* pMgr = dynamic_cast<MvpMatchRoomMgr*> (m_pRoomMgr);
    if (pMgr != nullptr)
    {
      // 惩罚中的玩家加入队伍, 导致队伍匹配失败
      if (pMgr->userCanJoin(charId, now()) == false)
      {
        NtfMatchInfoCCmd cmd;
        cmd.set_etype(EPVPTYPE_MVP);
        cmd.set_ismatch(false);
        PROTOBUF(cmd, send, len);
        pTeam->sendCmdToAllMember(send, len, charId);

        TeamMember* pMem = pTeam->getTeamMember(charId);
        if (pMem)
        {
          pTeam->sendMsgToAllMember(25601, MsgParams(pMem->name()));
        }
        m_setKickTeams.insert(pTeam);/*待踢出*/
        return;
      }
    }

    NtfMatchInfoCCmd cmd;
    cmd.set_etype(EPVPTYPE_MVP);
    cmd.set_ismatch(true);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);
    MatchManager::getMe().sendSysMsg(charId, zoneId, 7309);
  }
}

void MvpMatchRoom::onTeamDelMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId)
{
  TeamMatchRoom::onTeamDelMember(pTeam, charId, zoneId);

  NtfMatchInfoCCmd cmd;
  cmd.set_etype(EPVPTYPE_MVP);
  cmd.set_ismatch(false);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(charId, zoneId, send, len);
}

void MvpMatchRoom::onRaidSceneOpen(DWORD zoneid, DWORD sceneid)
{
  // 通知副本匹配的队伍数量
  SyncRoomSceneMatchSCmd cmd;
  cmd.set_roomid(m_qwGuid);
  cmd.set_zoneid(zoneid);
  cmd.set_sceneid(sceneid);

  cmd.set_roomsize(m_vecRoomTeam.size());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(zoneid, send, len);
}

void MvpMatchRoom::onBattleClose()
{
  if (m_state == EROOMSTATE_WAIT_JOIN)
  {
    for (auto &v : m_vecRoomTeam)
    {
      NtfMatchInfoCCmd cmd;
      cmd.set_etype(EPVPTYPE_MVP);
      cmd.set_ismatch(false);
      PROTOBUF(cmd, send, len);
      v.sendCmdToAllMember(send, len, 0);
    }
  }
}

void MvpMatchRoom::onStart()
{
  const SMvpTeamNumMatchCFG* pCFG = MiscConfig::getMe().getMvpBattleCFG().getMatchRoomInfo(m_vecRoomTeam.size());
  if (pCFG == nullptr)
  {
    XERR << "[Mvp-竞争战], 匹配失败, 找不到对应配置, 房间:" << m_qwGuid << "队伍数量:" << m_vecRoomTeam.size() << XEND;
    setState(EROOMSTATE_FIGHTING);
    return;
  }

  MvpMatchRoomMgr* pMgr = dynamic_cast<MvpMatchRoomMgr*> (m_pRoomMgr);
  if (pMgr == nullptr)
    return;

  m_dwZoneID = thisServer->getAZoneId();
  m_dwRaidId = pCFG->dwRaidID;
  m_dwPunishTime = pCFG->dwMatchPunishTime + now();

  DWORD index = 1;
  for (auto &v : m_vecRoomTeam)
  {
    v.setIndex(index++);

    // 若队伍已在match初始化完成(依赖team), 添加惩罚时间;若未初始化完成, 在初始化完成时,添加惩罚
    if (v.initOk())
    {
      createNewTeam(&v); // 到新线去创建队伍
      const TMapRoomMember& mems = v.getTeamMembers();
      for (auto &u : mems)
      {
        pMgr->addUserPunishTime(u.first, m_dwPunishTime);
        m_setRoomUsers.insert(u.first);
      }
    }

    NtfMatchInfoCCmd cmd;
    cmd.set_etype(EPVPTYPE_MVP);
    cmd.set_ismatch(false);
    cmd.set_isfight(true);
    PROTOBUF(cmd, send, len);
    v.sendCmdToAllMember(send, len, 0);
  }
  setState(EROOMSTATE_FIGHTING);
  XLOG << "[Mvp-竞争战], 匹配成功, 房间:" << m_qwGuid << "队伍数量:" << m_vecRoomTeam.size() << "匹配耗时:" << now() - m_dwBeginMatchTime << "前往新线:" << m_dwZoneID << "副本:" << m_dwRaidId << XEND;
}

void MvpMatchRoom::clearTeamPunishCD(QWORD newteamid)
{
  RoomTeam* pTeam = getTeamByNewTeamId(newteamid);
  if (pTeam == nullptr)
  {
    XERR << "[Mvp-竞争战], 清除CD失败, 找不到队伍信息, 房间:" << m_qwGuid << "队伍:" << newteamid << XEND;
    return;
  }
  MvpMatchRoomMgr* pMgr = dynamic_cast<MvpMatchRoomMgr*> (m_pRoomMgr);
  if (pMgr == nullptr)
    return;
  const TMapRoomMember& mems = pTeam->getTeamMembers();
  for (auto &u : mems)
  {
    pMgr->clearUserPunishTime(u.first);
  }
  XLOG << "[Mvp-竞争战], 清除队伍CD, 房间:" << m_qwGuid << "队伍:" << newteamid << XEND;
}


//***********************************************************************************
//****************************公会战决战*********************************************
//***********************************************************************************
SuperGvgMatchRoom::SuperGvgMatchRoom(QWORD guid, DWORD zoneid) : TeamMatchRoom(guid), m_dwZoneID(zoneid)
{

}

SuperGvgMatchRoom::~SuperGvgMatchRoom()
{
  TeamMatchRoom::reset();
}

void SuperGvgMatchRoom::setGuild(const TVecQWORD& ids)
{
  m_setGuildIDs.clear();
  m_setGuildIDs.insert(ids.begin(), ids.end());
}

bool SuperGvgMatchRoom::addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  RoomTeam team(zoneId, rev.teamid(), 0, m_qwGuid, charId);
  m_vecRoomTeam.push_back(team);
  return true;
}

void SuperGvgMatchRoom::onOneTeamOk(RoomTeam* pTeam)
{
  // 收到原线队伍信息后, 去新线去创建队伍
  createNewTeam(pTeam);
}

void SuperGvgMatchRoom::onNewTeamOk(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;

  //决战副本队伍状态与原副本状态不同步
  //pTeam->setTeamInPvp();

  SuperGvgMatchRoomMgr* pMgr = dynamic_cast<SuperGvgMatchRoomMgr*> (m_pRoomMgr);
  if (pMgr == nullptr)
    return;
  const TMapRoomMember& mems = pTeam->getTeamMembers();
  ApplyTeamMatchSCmd cmd;
  cmd.set_teamid(pTeam->getNewTeamId());
  cmd.set_zoneid(m_dwZoneID);
  for (auto &m : mems)
  {
    if (m.first == pTeam->getTeamCreator())
      continue;
    cmd.set_charid(m.first);
    PROTOBUF(cmd, send, len);
    thisServer->forwardCmdToTeamServer(m_dwZoneID, send, len);
  }
}

/*************************************************************************************/
/***********************组队排位赛****************************************************/
/*************************************************************************************/

TeamPwsMatchRoom::TeamPwsMatchRoom(QWORD guid, DWORD zoneid, EPvpType etype) : TeamMatchRoom(guid), m_dwZoneID(zoneid), m_eType(etype)
{

}

TeamPwsMatchRoom::~TeamPwsMatchRoom()
{
  reset();
}

void TeamPwsMatchRoom::addTeamInfo(QWORD leaderid, const TSetQWORD& members, DWORD zoneid)
{
  auto& m = m_mapLeader2TeamInfo[leaderid];
  auto& setuser = m[zoneid];
  setuser.insert(members.begin(), members.end());
}

bool TeamPwsMatchRoom::addMember(QWORD charId, DWORD zoneId, QWORD teamId)
{
  RoomTeam team(zoneId, teamId, 0, m_qwGuid, charId);
  m_vecRoomTeam.push_back(team);
  return true;
}

void TeamPwsMatchRoom::onOneTeamOk(RoomTeam* pTeam)
{
  // 收到原线队伍信息后, 去新线去创建队伍
  createNewTeam(pTeam);
}

void TeamPwsMatchRoom::onNewTeamOk(RoomTeam* pTeam)
{
  if (pTeam == nullptr)
    return;

  //排位赛副本队伍状态与原副本状态不同步
  //pTeam->setTeamInPvp();

  auto it = m_mapLeader2TeamInfo.find(pTeam->getTeamCreator());
  if (it == m_mapLeader2TeamInfo.end())
    return;

  m_dwOkTeamCount++;

  // 队员强制申请进队
  ApplyTeamMatchSCmd cmd;
  cmd.set_teamid(pTeam->getNewTeamId());
  cmd.set_zoneid(m_dwZoneID);

  SPwsNewTeamInfo& info = m_mapTeamID2NewTeamInfo[cmd.teamid()];
  info.qwTeamID = cmd.teamid();
  info.qwLeaderID = it->first;
  for (auto &m : it->second)
  {
    for (auto &s : m.second)
    {
      info.setMems.insert(s);
      if (s == pTeam->getTeamCreator())
        continue;
      cmd.set_charid(s);
      PROTOBUF(cmd, send, len);
      thisServer->forwardCmdToTeamServer(m_dwZoneID, send, len);
    }
  }

  if (m_dwOkTeamCount == m_mapLeader2TeamInfo.size())
  {
    DWORD index = 1;
    map<QWORD, QWORD> tmpcolor;
    for (auto &m : m_mapTeamID2NewTeamInfo)
    {
      m.second.dwColor = index ++;
      tmpcolor[m.second.qwLeaderID] = m.second.dwColor;
    }

    for (auto &m : m_mapLeader2TeamInfo)
    {
      DWORD color = tmpcolor[m.first];
      for (auto &t : m.second)
      {
        for (auto &s : t.second)
        {
          MatchManager::getMe().addRoomUser(s, t.first, m_qwGuid);
          MatchManager::getMe().sendPlayerToFighting(t.first, m_dwZoneID, s, getRaidId(), m_qwGuid, color);
          XLOG << "[组队排位赛-创建队伍完成], 玩家进入副本, 玩家:" << s << "原线:" << t.first << "目标线:" << m_dwZoneID << "房间:" << m_qwGuid << XEND;
        }
      }
    }
  }
}

void TeamPwsMatchRoom::formatTeamInfo(SyncRoomSceneMatchSCmd& cmd)
{
  for (auto &m : m_mapTeamID2NewTeamInfo)
  {
    auto p = cmd.add_pwsdata();
    if (!p)
      continue;
    p->set_teamid(m.first);
    p->set_color(m.second.dwColor);
    for (auto &s : m.second.setMems)
    {
      auto user = p->add_users();
      if (!user)
        continue;
      user->set_charid(s);
      DWORD score = ScoreManager::getMe().getPwsScoreByCharID(s);
      user->set_score(score);
    }
  }
}

bool TeamPwsMatchRoom::reset()
{
  TeamMatchRoom::reset();

  for (auto &m : m_mapLeader2TeamInfo)
  {
    for (auto &t : m.second)
    {
      for (auto &s : t.second)
      {
        MatchManager::getMe().delRoomUser(s, t.first, m_qwGuid);
      }
    }
  }
  for (auto &m : m_mapTeamID2NewTeamInfo)
  {
    NtfLeaveRoom ntfLeave;
    ntfLeave.set_teamid(m.first);
    ntfLeave.set_roomid(getGuid());
    PROTOBUF(ntfLeave, send, len);
    thisServer->sendCmdToZone(m_dwZoneID, send, len);
  }
  return true;
}

void TeamPwsMatchRoom::getAllUsers(TSetQWORD& users)
{
  for (auto &m : m_mapTeamID2NewTeamInfo)
    users.insert(m.second.setMems.begin(), m.second.setMems.end());
}

