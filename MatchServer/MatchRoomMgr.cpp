#include "MatchRoomMgr.h"
#include "MatchManager.h"
#include "MatchServer.h"
#include "MapConfig.h"
#include "MiscConfig.h"
#include "MatchUserMgr.h"
#include "GCharManager.h"
#include "CommonConfig.h"
#include "RedisManager.h"
#include "ScoreManager.h"

MatchRoomMgr::MatchRoomMgr()
{
}
MatchRoomMgr::~MatchRoomMgr()
{
}

QWORD MatchRoomMgr::getAvailableRoom()
{
  return 0;
}

bool MatchRoomMgr::reqRoomDetail(QWORD charId, DWORD zoneId, ReqRoomDetailCCmd& rev)
{
  MatchRoom * pRoom = getRoomByid(rev.roomid());
  if (pRoom == nullptr)
  {
    return false;
  }

  pRoom->toDetailData(rev.mutable_datail_info());
  PROTOBUF(rev, send, len);
  thisServer->sendCmdToClient(charId, zoneId, send, len);
  return true;
}

/************************************************************************/
/*LLHMatchRoomMgr                                                                      */
/************************************************************************/

LLHMatchRoomMgr::LLHMatchRoomMgr()
{  
  m_maxRoomCount = 0;

  const SPvpCFG* pCfg = MiscConfig::getMe().getPvpCfg(EPVPTYPE_LLH);
  if (pCfg == nullptr)
  {
    XERR << "[斗技场-配置表错误] GameConfig PVPConfig 找不到对应的配置" << EPVPTYPE_LLH << XEND;
    return;
  }
  
  for (auto & m : pCfg->mapRaid)
  {
    DWORD raidId = m.first;
    DWORD count = m.second;
    const SRaidCFG* pRaidCfg = MapConfig::getMe().getRaidCFG(raidId);
    if (pRaidCfg == nullptr)
    {
      XERR << "[斗技场-配置表错误] MapRaid.txt找不到raid配置,raidid" << raidId << XEND;
      continue;
    }

    if (pRaidCfg->eRaidType != ERAIDTYPE_PVP_LLH)
    {
      XERR << "[斗技场-配置表错误] MapRaid.txt表的raid配置类型不是溜溜猴,raidid" << raidId << "raidtype" << pRaidCfg->eRaidType << XEND;
      continue;
    }

    std::stringstream ss;

    for (DWORD i = 0; i < count; ++i)
    {
      MatchRoom* pRoom = nullptr;
      QWORD guid = MatchManager::getMe().generateGuid();

      pRoom = NEW LLHMatchRoom(guid);
      if (pRoom == nullptr)
      {
        continue;
      }
      pRoom->setMatchRoomMgr(this);
      pRoom->setRaidId(raidId);
      pRoom->setMapId(pRaidCfg->dwMapID);
      pRoom->m_dwPeopleLimit = pCfg->dwPeopleLImit;
      ss.str("");
      ss << pRaidCfg->strNameZh << "-" << (i + 1);
      std::string name = ss.str();
      pRoom->setName(name);
      pRoom->setState(EROOMSTATE_FIGHTING);
      m_vecRooms.push_back(pRoom);
      XLOG << "[斗技场-房间创建-溜溜猴] roomid" << pRoom->getGuid() << "mapid" << pRoom->getRaidId() << "房间名字" << name << XEND;
    }
    m_maxRoomCount += count;
  }
}

LLHMatchRoomMgr::~LLHMatchRoomMgr()
{
  for (auto &v : m_vecRooms)
  {
    if (v)
    {
      SAFE_DELETE(v);
    }
  }
}

bool LLHMatchRoomMgr::reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev)
{
  if (rev.roomids_size() == 0)
  {
    for (auto &v : m_vecRooms)
    {
      v->toBriefData(rev.add_room_lists());
    }
  }
  else
  {
    for (int i = 0; i < rev.roomids_size(); ++i)
    {
      MatchRoom* pRoom = getRoomByid(rev.roomids(i));
      if (pRoom == nullptr)
        continue;
      pRoom->toBriefData(rev.add_room_lists());
    }
  }

  PROTOBUF(rev, send, len);
  bool ret = thisServer->sendCmdToClient(charId, zoneId, send, len);
  
  XLOG << "[斗技场-请求房间列表-溜溜猴] " << charId << zoneId << "ret" << ret <<"res"<<rev.ShortDebugString()<< XEND;

  return true;
}

MatchRoom* LLHMatchRoomMgr::getRoomByid(QWORD guid)
{
  for (auto &v : m_vecRooms)
  {
    if (v && v->getGuid() == guid)
    {
      return v;
    }
  }
  return nullptr;
}

bool LLHMatchRoomMgr::joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  if (MatchManager::getMe().isInMatching(charId))
  {
    XLOG << "[溜溜猴-加入], 玩家已在匹配状态中, 不可加入, 玩家:" << charId << XEND;
    return false;
  }

  MatchRoom* pRoom = getRoomByid(rev.roomid());
  if (pRoom == nullptr)
  {
    XERR << "[斗技场-加入房间-溜溜猴] 找不到房间" << charId << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }
  EMatchRetCode retCode = pRoom->checkCanJoin();
  if (retCode != EMATCHRETCODE_OK)
  {
    MatchManager::getMe().sendSysMsg(charId, zoneId, 950);
    XERR << "[斗技场-加入房间-溜溜猴] 房间不可加入" << charId << zoneId << "roomid" << rev.roomid()<<"当前房间人数"<<pRoom->getMemberCount() << "retcode"<< retCode << XEND;
    return false;
  }  
  pRoom->addMember(charId, zoneId, rev);
  
  //MatchManager::getMe().addRoomUser(charId,zoneId, pRoom->getGuid());

  updateRank();
  
  bool ret = MatchManager::getMe().sendPlayerToFighting(zoneId, thisServer->m_dwLLHZoneid, charId, pRoom->getRaidId(), pRoom->getGuid());

  XLOG << "[斗技场-加入房间-溜溜猴] 成功,通知场景进入战斗" << charId << zoneId << "roomid" << rev.roomid() <<"pvpzoneid"<<thisServer->m_dwLLHZoneid <<"当前房间人数" << pRoom->getMemberCount()<<"ret"<<ret << XEND;
  return true;
}

QWORD LLHMatchRoomMgr::getAvailableRoom()
{
  for (auto &v : m_vecRooms)
  {
    if (v->checkCanJoin() == EMATCHRETCODE_OK)
    {
      return v->getGuid();
    }
  }

  return 0;
}

void LLHMatchRoomMgr::updateRank()
{
  std::sort(m_vecRooms.begin(), m_vecRooms.end(), CmpByPeople());
}

/************************************************************************/
/* TeamMatchRoomMgr                                                                     */
/************************************************************************/

TeamMatchRoomMgr::TeamMatchRoomMgr()
{}
TeamMatchRoomMgr::~TeamMatchRoomMgr()
{
}

void TeamMatchRoomMgr::oneSecTick(DWORD curTime)
{
  for (auto it = m_mapRooms.begin(); it != m_mapRooms.end(); ++it)
  {
    if (it->second)
    {
      it->second->timeTick(curTime);
    }
  }
}

MatchRoom* TeamMatchRoomMgr::getRoomByid(QWORD guid)
{
  auto it = m_mapRooms.find(guid);
  if (it == m_mapRooms.end())
    return nullptr;
  return it->second;
}

bool TeamMatchRoomMgr::joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  if (MatchManager::getMe().isInMatching(charId))
  {
    XLOG << "[组队pvp-加入], 玩家已在匹配状态中, 不可加入, 玩家:" << charId << XEND;
    return false;
  }

  //从老的房间踢出队伍
  QWORD qwOldRoomId = MatchManager::getMe().getRoomId(charId, zoneId);
  if (qwOldRoomId)
  {
    MatchRoom* pOldRoom = MatchManager::getMe().getRoomByRoomId(qwOldRoomId);

    TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pOldRoom);
    if (pTeamRoom)
    {
      if (pTeamRoom->getType() != rev.type())
      {
        //有队员正在别的斗技场，请稍后再试（970）
        MatchManager::getMe().sendSysMsg(charId, zoneId, 970);
        XERR << "[斗技场-加入房间] 失败，已经在其他类型的房间内 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
        return false;
      }
      else
      {
        if (pTeamRoom->getGuid() == rev.roomid())
        {
          XERR << "[斗技场-加入房间] 失败，已经在该房间内了 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
          return false;
        }
        
        if (pTeamRoom->getState() == EROOMSTATE_MATCH_SUCCESS)
        {
          XERR << "[斗技场-加入房间] 失败，老房间正在匹配中 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
          return false;
        }

        if (pTeamRoom->isFighting())
        {
          XERR << "[斗技场-加入房间] 失败，老房间已经在战斗了 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
          return false;
        }
        
        RoomTeam* pTeam = pTeamRoom->getTeamByCharId(zoneId,charId);
        if (pTeam)
        {
          kickTeam(pTeamRoom, pTeam);
        }
      }
    }   
  }

  MatchRoom* pRoom = getRoomByid(rev.roomid());  
  if (pRoom == nullptr)
  {
    XERR << "[斗技场-加入房间] 失败，找不到房间" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }

  EMatchRetCode retCode = pRoom->checkCanJoin();
  if (retCode != EMATCHRETCODE_OK)
  {
    //该房间已满，请选择其它房间吧！（950）
    MatchManager::getMe().sendSysMsg(charId, zoneId, 950);
    XERR << "[斗技场-加入房间] 失败，房间已满或者已经开战" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "ret" << retCode << XEND;
    return false;
  }

  if (pRoom->addMember(charId, zoneId, rev) == false)
  {
    XERR << "[斗技场-加入房间] 失败，添加失败" << rev.type() << charId << zoneId << "roomid" << pRoom->getGuid() << XEND;
    return false;
  }  
  MatchManager::getMe().addRoomUser(charId, zoneId, pRoom->getGuid());
  
  //send res to session
  Cmd::NtfJoinRoom cmd;
  cmd.set_roomid(rev.roomid());
  cmd.set_charid(charId);
  cmd.set_teamid(rev.teamid());
  cmd.set_success(true);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(zoneId, send, len);

  if (rev.type() == EPVPTYPE_SMZL)
  {
    //已发起挑战，10s后进入准备阶段（956）    
    SMZLMatchRoom* pSMZLRoom = dynamic_cast<SMZLMatchRoom*>(pRoom);
    if (pSMZLRoom)
    {
      pSMZLRoom->startCountDown(zoneId, charId);
    }
  }

  XLOG << "[斗技场-加入房间] 成功报名，通知teamserver同步组队数据" << rev.type() << charId << zoneId << "roomid" << pRoom->getGuid()<<"teamid"<<rev.teamid() << XEND;
  return true;
}

bool TeamMatchRoomMgr::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{
  TeamMatchRoom* pRoom = dynamic_cast<TeamMatchRoom*>(getRoomByid(rev.roomid()));
  if (pRoom == nullptr)
  {
    XERR << "[斗技场-房主取消报名] 失败，找不到房间 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }

  if (pRoom->isFighting())
  {
    XERR << "[斗技场-房主取消报名] 失败，正在战斗不可取消 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  } 
  
  RoomTeam* pTeam = pRoom->getTeamByCharId(zoneId, charId);
  if (!pTeam)
  {
    XERR << "[斗技场-房主取消报名] 失败，找不到队伍信息 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }

  QWORD qwLeaderId = pTeam->getOnlineLeader();;
  if (!qwLeaderId)
  {
    XERR << "[斗技场-房主取消报名] 失败，找不到队长 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }

  if (qwLeaderId != charId)
  {
    XERR << "[斗技场-房主取消报名] 失败，不是队长 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid()<<"队长"<< qwLeaderId << XEND;
    return false;
  }

  kickTeam(pRoom, pTeam);

  return true;
}

//warning:pRoom pTeam 可能会被删除
bool TeamMatchRoomMgr::kickTeam(TeamMatchRoom* pRoom, RoomTeam* pTeam)
{
  if (pRoom == nullptr || pTeam == nullptr)
    return false;

  pRoom->kickTeam(pTeam);

  if (pRoom->checkCanJoin() == EMATCHRETCODE_OK)
  {
    m_mapCanJoinRooms[pRoom->getGuid()] = pRoom;
  }
  
  if (pRoom->isEmpty())
  {
    pRoom->reset();
    //华丽金属重复利用
    if (pRoom->getType() == EPVPTYPE_SMZL)
    {
      //删除房间
      m_mapRooms.erase(pRoom->getGuid());
      m_mapCanJoinRooms.erase(pRoom->getGuid());
      m_setName.erase(pRoom->getName());
      SAFE_DELETE(pRoom);
    }   
  }
  return true;
}

//从pvp场景离开
bool TeamMatchRoomMgr::onLeaveRoom(QWORD charId, MatchRoom* pRoom)
{
  if (charId == 0 || pRoom == nullptr)
    return false;

  return true;
}

bool TeamMatchRoomMgr::joinFighting(QWORD charId, DWORD zoneId, JoinFightingCCmd& rev)
{
  QWORD roomId = MatchManager::getMe().getRoomId(charId, zoneId);
  if (roomId != rev.roomid())
  {
    XERR << "[斗技场-加入战斗] 失败, 房间id不一致" << charId << zoneId << "roomid" << rev.roomid() << "serverroomid" << roomId << XEND;
    return false;
  }

  TeamMatchRoom* pRoom = dynamic_cast<TeamMatchRoom*>(getRoomByid(rev.roomid()));
  if (pRoom == nullptr)
  {
    XERR << "[斗技场-加入战斗] 失败, 没找到房间" << charId << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }
  if (pRoom->isFighting() == false)
  {
    XERR << "[斗技场-加入战斗] 失败, 房间不在战斗状态" << charId << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }
  
  RoomTeam* pTeam = pRoom->getTeamByCharId(zoneId, charId);
  if (pTeam == nullptr)
  {
    XERR << "[斗技场-加入战斗] 失败, 没找到队伍" << charId << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }    
 
  DWORD destZoneId = 0;
  switch (rev.type())
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
   
  bool ret = MatchManager::getMe().sendPlayerToFighting(zoneId, destZoneId, charId, pRoom->getRaidId(), pRoom->getGuid(), pTeam->getIndex());

  XLOG << "[斗技场-加入战斗] 成功,传送战斗场景" << charId << zoneId << "roomid" << rev.roomid() << "ret" << ret << XEND;
  return true;
}

bool TeamMatchRoomMgr::closeRoom(TeamMatchRoom* pRoom)
{
  return false;
}

void TeamMatchRoomMgr::setRoomFighting(MatchRoom* pRoom)
{
  if (!pRoom)
    return;
  m_mapCanJoinRooms.erase(pRoom->getGuid());
}

/************************************************************************/
/* SMZLMatchRoomMgr                                                                     */
/************************************************************************/
SMZLMatchRoomMgr::SMZLMatchRoomMgr()
{}
SMZLMatchRoomMgr::~SMZLMatchRoomMgr()
{
}

bool SMZLMatchRoomMgr::reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev)
{
  static DWORD MAX_ROOM_COUNT = 20;
  if (rev.roomids_size() == 0)
  {
    std::vector<MatchRoom*> vecRooms;
    vecRooms.reserve(m_mapCanJoinRooms.size());
    for (auto &m : m_mapCanJoinRooms)
    {
      if (m.second)
      {
        vecRooms.push_back(m.second);
      }                 
    }    
    std::random_shuffle(vecRooms.begin(), vecRooms.end());
    
    if (vecRooms.size() > MAX_ROOM_COUNT)
      vecRooms.resize(MAX_ROOM_COUNT);
    
    MatchRoom* pRoom = MatchManager::getMe().getRoomByCharId(charId, zoneId);
    QWORD filterRoom = 0;
    if (pRoom && pRoom->getType() == EPVPTYPE_SMZL)
    {
      filterRoom = pRoom->getGuid();
      pRoom->toBriefData(rev.add_room_lists());
    }
    for (auto&v : vecRooms)
    {
      //自己的不要了
      if (filterRoom && v->getGuid() == filterRoom)
        continue;
      v->toBriefData(rev.add_room_lists());
    }   
  }
  else
  {
    for (int i = 0; i < rev.roomids_size(); ++i)
    {
      MatchRoom* pRoom = getRoomByid(rev.roomids(i));
      if (pRoom == nullptr)
        continue;
      pRoom->toBriefData(rev.add_room_lists());
    }
  }

  PROTOBUF(rev, send, len);
  bool ret = thisServer->sendCmdToClient(charId, zoneId, send, len);

  XLOG << "[斗技场-请求房间列表-沙漠之狼] " << charId << zoneId << "ret" << ret << "res" << rev.ShortDebugString() << XEND;

  return ret;
}

MatchRoom* SMZLMatchRoomMgr::createRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  if (MatchManager::getMe().isInMatching(charId))
  {
    XLOG << "[沙漠之狼-创建房间], 玩家已在匹配状态中, 不可创建, 玩家:" << charId << XEND;
    return nullptr;
  }

  //check name
  if (rev.name().empty())
  {
    XERR<< "[斗技场-创建房间-沙漠之狼] 失败，房间名不可为空" << charId << zoneId << "roomid" << rev.roomid() << XEND;
    return nullptr;
  }

  auto it = m_setName.find((rev.name()));
  if (it != m_setName.end())
  {
    //队伍名不可用（958）
    MatchManager::getMe().sendSysMsg(charId, zoneId, 958);
    XERR << "[斗技场-创建房间-沙漠之狼] 失败，房间名重复" << charId << zoneId << "roomid" << rev.roomid()<<rev.name() << XEND;
    return nullptr;
  }

  QWORD qwOldRoomId = MatchManager::getMe().getRoomId(charId, zoneId);
  if (qwOldRoomId)
  {
    MatchRoom* pOldRoom = MatchManager::getMe().getRoomByRoomId(qwOldRoomId);

    TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pOldRoom);
    if (pTeamRoom)
    {
      if (pTeamRoom->getType() != rev.type())
      {
        //有队员正在别的斗技场，请稍后再试（970）
        MatchManager::getMe().sendSysMsg(charId, zoneId, 970);
        XERR << "斗技场-加入房间] 失败，已经在其他类型的房间内 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
        return nullptr;
      }
      else
      {
        if (pTeamRoom->getGuid() == rev.roomid())
        {
          XERR << "斗技场-加入房间] 失败，已经在该房间内了 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
          return nullptr;
        }

        if (pTeamRoom->getState() == EROOMSTATE_MATCH_SUCCESS)
        {
          XERR << "斗技场-加入房间] 失败，老房间正在匹配中 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
          return nullptr;
        }

        if (pTeamRoom->isFighting())
        {
          XERR << "斗技场-加入房间] 失败，老房间已经在战斗了 type" << rev.type() << charId << zoneId << "roomid" << rev.roomid() << "oldroomid" << qwOldRoomId << "oldroomtype" << pTeamRoom->getType() << XEND;
          return nullptr;
        }

        RoomTeam* pTeam = pTeamRoom->getTeamByCharId(zoneId, charId);
        if (pTeam)
        {
          kickTeam(pTeamRoom, pTeam);
        }
      }
    }
  }
  
  const SPvpCFG* pCfg = MiscConfig::getMe().getPvpCfg(EPVPTYPE_SMZL);
  if (pCfg == nullptr)
  {
    XERR << "[斗技场-创建房间-沙漠之狼] 失败，找不到gameconfig配置" << charId << zoneId << "roomid" << rev.roomid() << XEND;
    return nullptr;
  }

  QWORD roomId = MatchManager::getMe().generateGuid();
  if (roomId == 0)
  {
    return nullptr;
  }
  SMZLMatchRoom* pRoom = NEW SMZLMatchRoom(roomId);
  if (pRoom == nullptr)
  {
    return nullptr;
  }
  pRoom->setRaidId(pCfg->raidId);
  pRoom->setName(rev.name());
  pRoom->setTeamCount(pCfg->dwTeamLimit);
  pRoom->setMatchRoomMgr(this);
  if (pRoom->addMember(charId, zoneId, rev) == false)
  {
    SAFE_DELETE(pRoom);
    return nullptr;
  }
  m_mapRooms[pRoom->getGuid()] = pRoom;
  m_mapCanJoinRooms[pRoom->getGuid()] = pRoom;
  MatchManager::getMe().addRoomUser(charId, zoneId, pRoom->getGuid());

  //send res to session
  Cmd::NtfJoinRoom cmd;
  cmd.set_roomid(roomId);
  cmd.set_charid(charId);
  cmd.set_teamid(rev.teamid());
  cmd.set_success(true);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(zoneId, send, len);

  m_setName.insert(rev.name());
  XLOG << "[斗技场-创建房间-沙漠之狼] 成功，通知teamserver同步队伍数据" << charId << zoneId << "roomid" << pRoom->getGuid() << XEND;
  return pRoom;
}

bool SMZLMatchRoomMgr::closeRoom(TeamMatchRoom* pRoom)
{
  if (!pRoom)
    return false;
  
  pRoom->reset();
  m_mapRooms.erase(pRoom->getGuid());
  m_mapCanJoinRooms.erase(pRoom->getGuid());
  m_setName.erase(pRoom->getName());
  
  SAFE_DELETE(pRoom);
  return true;
}


/************************************************************************/
/*HLJSMatchRoomMgr                                                                      */
/************************************************************************/
HLJSMatchRoomMgr::HLJSMatchRoomMgr()
{
}
HLJSMatchRoomMgr::~HLJSMatchRoomMgr()
{
}

bool HLJSMatchRoomMgr::reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev)
{
  if (rev.roomids_size() == 0)
  {
    DWORD maxCanJoinRoom = 20;
    DWORD diff = 0;
    if (m_mapCanJoinRooms.size() < 20)
    {
      diff = maxCanJoinRoom - m_mapCanJoinRooms.size();
    }
    
    //create empty room
    for (DWORD i = 0; i < diff; ++i)
    {
      createEmptyRoom();
    }

    //自己的列表
    MatchRoom* pRoom = MatchManager::getMe().getRoomByCharId(charId, zoneId);
    QWORD myRoom = 0;
    bool bAdd = false;
    if (pRoom && pRoom->getType() == EPVPTYPE_HLJS)
    {
      bAdd = true;
      myRoom = pRoom->getGuid();
    }

    DWORD count = 0;
    for (auto &m : m_mapCanJoinRooms)
    {
      //自己的在列表里
      if (myRoom && m.first == myRoom)
      {
        bAdd = false;
      }
      if (m.second)
      {
        count++;
        m.second->toBriefData(rev.add_room_lists());
        if (count >= maxCanJoinRoom)
          break;
      }
    }

    //自己的列表
    if (pRoom && bAdd)
    {
      if (pRoom)
      {
        pRoom->toBriefData(rev.add_room_lists());
      }
    }

  }
  else
  {
    for (int i = 0; i < rev.roomids_size(); ++i)
    {
      MatchRoom* pRoom = getRoomByid(rev.roomids(i));
      if (pRoom == nullptr)
        continue;
      pRoom->toBriefData(rev.add_room_lists());
    }
  }

  PROTOBUF(rev, send, len);
  bool ret = thisServer->sendCmdToClient(charId, zoneId, send, len);

  XLOG << "[斗技场-请求房间列表] type " << rev.type() << "charid" << charId << "zoneid" << zoneId << "ret" << ret << "res" << rev.ShortDebugString() << XEND;

  return ret;
}

bool HLJSMatchRoomMgr::closeRoom(TeamMatchRoom* pRoom)
{
  if (!pRoom)
    return false;
  pRoom->reset();
  m_mapCanJoinRooms[pRoom->getGuid()] = pRoom;
  return true;
}

MatchRoom* HLJSMatchRoomMgr::createEmptyRoom()
{
  const SPvpCFG* pCfg = MiscConfig::getMe().getPvpCfg(EPVPTYPE_HLJS);
  if (pCfg == nullptr)
  {
    XERR << "[斗技场-配置表错误] GameConfig PVPConfig 找不到对应的配置" << EPVPTYPE_HLJS << XEND;
    return nullptr;
  }

  QWORD roomId = MatchManager::getMe().generateGuid();
  if (roomId == 0)
  {
    return nullptr;
  }
  MatchRoom* pRoom = NEW HLJSMatchRoom(roomId);
  if (pRoom == nullptr)
  {
    return nullptr;
  }
  pRoom->setRaidId(pCfg->raidId);
  pRoom->setTeamCount(pCfg->dwTeamLimit);
  pRoom->setState(EROOMSTATE_WAIT_JOIN);  
  pRoom->setMatchRoomMgr(this);
  m_mapRooms[pRoom->getGuid()] = pRoom;
  m_mapCanJoinRooms[pRoom->getGuid()] = pRoom;

  XLOG << "[斗技场-创建空房间-华丽金属] roomid" << pRoom->getGuid() << XEND;
  return pRoom;
}

/************************************************************************/
/*PollyMatchRoomMgr                                                                      */
/************************************************************************/
PollyMatchRoomMgr::PollyMatchRoomMgr()
{
}
PollyMatchRoomMgr::~PollyMatchRoomMgr()
{
}

bool PollyMatchRoomMgr::joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  return pushMatchQueue(charId);
}

bool PollyMatchRoomMgr::pushMatchQueue(QWORD charId)
{
  if (!MatchManager::getMe().isPollyActivityOpen())
  {
    XERR << "[斗技场-波利乱斗] 活动尚未开启，报名,加入匹配队列，charid" << charId << XEND;
    return false;
  }

  if (checkIsInQueue(charId))
    return true;
  m_matchQueue.push_back(charId);
  
  MatchUser* pUser = MatchManager::getMe().getMatchUser(charId);
  if (pUser)
  {
    NtfMatchInfoCCmd cmd;
    cmd.set_etype(EPVPTYPE_POLLY);
    cmd.set_ismatch(true);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(pUser->m_qwCharId, pUser->m_dwCurZoneId, send, len);
  }
  
  XLOG << "[斗技场-波利乱斗] 报名,加入匹配队列，charid" << charId << "匹配队列人数" << m_matchQueue.size() << "重试队列人数" << m_retryQueue.size() << XEND;

  while (createNewRoom())
  {
  }  
  return true;
}

bool PollyMatchRoomMgr::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{
  if (popMatchQueue(charId))
  {
    NtfMatchInfoCCmd cmd;
    cmd.set_etype(EPVPTYPE_POLLY);
    cmd.set_ismatch(false);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);
    return true;
  }
  return false;
}

bool PollyMatchRoomMgr::popMatchQueue(QWORD charId)
{
  auto it = std::find(m_matchQueue.begin(), m_matchQueue.end(), charId);
  if (it == m_matchQueue.end())
    return false;

  m_matchQueue.erase(it);
  XLOG << "[斗技场-波利乱斗] 取消报名,从匹配队列删除，charid" << charId << "匹配队列人数" << m_matchQueue.size() << "重试队列人数" << m_retryQueue.size() << XEND;
  return true;
}

bool PollyMatchRoomMgr::checkIsInQueue(QWORD charId)
{
  auto it = std::find(m_matchQueue.begin(), m_matchQueue.end(), charId);
  if (it == m_matchQueue.end())
    return false;
  return true;
}

bool PollyMatchRoomMgr::createNewRoom()
{
  DWORD people = 10;
  const SPvpCFG* pCfg = MiscConfig::getMe().getPvpCfg(EPVPTYPE_POLLY);
  if (!pCfg)
  {
    XERR << "[斗技场-波利乱斗-配置表错误] 波利乱斗 GameConfig PVPConfig 找不到对应的配置" << EPVPTYPE_POLLY << XEND;
    return false;
  }
  people = pCfg->dwPeopleLImit;
  if (people == 0)
    return false;
  
  if (m_matchQueue.size() < people)
    return false;
  DWORD raidId = pCfg->raidId;
  const SRaidCFG* pRaidCfg = MapConfig::getMe().getRaidCFG(raidId);
  if (pRaidCfg == nullptr)
  {
    XERR << "[斗技场-波利乱斗-配置表错误] MapRaid.txt找不到raid配置,raidid" << raidId << XEND;
    return false;
  }

  if (pRaidCfg->eRaidType != ERAIDTYPE_PVP_POLLY)
  {
    XERR << "[斗技场-波利乱斗-配置表错误] MapRaid.txt表的raid配置类型不是波利乱斗,raidid" << raidId << "raidtype" << pRaidCfg->eRaidType << XEND;
    return false;
  }
    
  MatchRoom* pRoom = nullptr;
  QWORD guid = MatchManager::getMe().generateGuid();
  pRoom = NEW PollyMatchRoom(guid);
  if (pRoom == nullptr)
  {
    return false;
  }
  pRoom->setMatchRoomMgr(this);
  pRoom->setRaidId(raidId);
  pRoom->setMapId(pRaidCfg->dwMapID);
  pRoom->m_dwPeopleLimit = pCfg->dwPeopleLImit;
  pRoom->setState(EROOMSTATE_FIGHTING);
  m_vecRooms.push_back(pRoom);
  
  DWORD zoneId = thisServer->getAZoneId();
  XLOG << "[斗技场-波利乱斗] 人满创建房间，roomid" <<pRoom->getGuid() <<"mapid"<<pRoom->getRaidId() <<"创建房间的线"<<zoneId << XEND;
  DWORD  i = 1;     //index 从1开始
  for (auto it = m_matchQueue.begin(); it != m_matchQueue.end(); )
  {
    QWORD charId = *it; 
    it = m_matchQueue.erase(it);

    MatchUser* pUser = MatchManager::getMe().getMatchUser(charId);
    if (!pUser)
    {
      pushRetryQueue(charId);
      continue;
    }

    NtfMatchInfoCCmd cmd;
    cmd.set_etype(EPVPTYPE_POLLY);
    cmd.set_ismatch(false);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(pUser->m_qwCharId, pUser->m_dwCurZoneId, send, len);

    if (!MatchManager::getMe().sendPlayerToFighting(pUser->m_dwCurZoneId, zoneId, pUser->m_qwCharId, pRoom->getRaidId(), pRoom->getGuid(), i))
    {
      pushRetryQueue(charId);
      continue;
    }
    pUser->m_dwPollyZoneId = zoneId;
    i++;    
  } 
  return true;
}

void PollyMatchRoomMgr::pushRetryQueue(QWORD charId)
{
  m_retryQueue[charId] = now() + 60;
  XLOG << "[斗技场-波利乱斗] 加入重试队列 charid" << charId << "匹配队列人数" << m_matchQueue.size() << "重试队列人数" << m_retryQueue.size() << XEND;
}

void PollyMatchRoomMgr::popRetryQueue(QWORD charId)
{
  DWORD curSec = now();
  auto it = m_retryQueue.find(charId);
  if (it == m_retryQueue.end())
    return;
  
  if (curSec <= it->second)
  {
    XLOG << "[斗技场-波利乱斗] 玩家上线，从重试队列删除加入匹配队列 charid" << charId << "匹配队列人数" << m_matchQueue.size() << "重试队列人数" << m_retryQueue.size() << XEND;
    pushMatchQueue(it->first);
  }
  m_retryQueue.erase(charId);
}

void PollyMatchRoomMgr::clearQueue()
{
  XLOG << "[斗技场-波利乱斗] 活动结束清除队列，匹配队列人数" << m_matchQueue.size() << "重试队列人数" << m_retryQueue.size() << XEND;
    
  NtfMatchInfoCCmd cmd;
  cmd.set_etype(EPVPTYPE_POLLY);
  cmd.set_ismatch(false);
  PROTOBUF(cmd, send, len);
  
  for (auto&v : m_matchQueue)
  {
    MatchUser* pUser = MatchManager::getMe().getMatchUser(v);
    if (pUser)
    {
      thisServer->sendCmdToClient(pUser->m_qwCharId, pUser->m_dwCurZoneId, send, len);
      XDBG << "[斗技场-波利乱斗] 活动结束清除队列，charid" << pUser->m_qwCharId <<"zoneid"<<pUser->m_dwCurZoneId <<"队列大小" << m_matchQueue.size() << XEND;
    }
  }     
  m_matchQueue.clear();
  m_retryQueue.clear();
}

MatchRoom* PollyMatchRoomMgr::getRoomByid(QWORD guid)
{
  for (auto &v : m_vecRooms)
  {
    if (v && v->getGuid() == guid)
    {
      return v;
    }
  }
  return nullptr;
}

bool PollyMatchRoomMgr::closeRoom(MatchRoom* pRoom)
{
  if (!pRoom)
    return false;

  for (auto it = m_vecRooms.begin(); it != m_vecRooms.end(); ++it)
  {
    if ((*it) == pRoom)
    {
      m_vecRooms.erase(it);
      SAFE_DELETE(pRoom);
      return true;
    }
  }
  return false;
}

/************************************************************************/
/* MvpMatchRoomMgr*/
/************************************************************************/
MvpMatchRoomMgr::MvpMatchRoomMgr()
{

}

MvpMatchRoomMgr::~MvpMatchRoomMgr()
{

}

bool MvpMatchRoomMgr::joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  if (!MatchManager::getMe().isMvpBattleOpen())
  {
    XERR << "[Mvp-竞争战], 活动尚未开启, 无法加入, 玩家:" << charId << XEND;
    return false;
  }
  // 若自己队伍已经在副本中, 则玩家直接可以进入
  QWORD oldRoomID = MatchManager::getMe().getRoomId(charId, zoneId);
  if (oldRoomID)
  {
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(oldRoomID);
    if (pRoom == nullptr)
    {
      XERR << "[Mvp-竞争战], 找不到对应的房间, 玩家:" << charId << "房间id:" << oldRoomID << XEND;
      return false;
    }
    MvpMatchRoom* pMvpRoom = dynamic_cast<MvpMatchRoom*>(pRoom);
    if (pMvpRoom == nullptr)
    {
      // 已报名其他挑战
      MatchManager::getMe().sendSysMsg(charId, zoneId, 970);
      return false;
    }
    if (pMvpRoom->isFighting() == false)
    {
      XERR << "[Mvp-竞争战], 房间不在战斗状态, 不可进入, 玩家:" << charId << "房间:" << oldRoomID << XEND;
      return false;
    }
    if (pMvpRoom->userCanEnter(charId) == false)
    {
      MatchManager::getMe().sendSysMsg(charId, zoneId, 25602);
      XLOG << "[Mvp-竞争战], 玩家队伍匹配成功时, 该玩家尚未进入队伍, 当前不可进入副本, 玩家:" << charId << "房间:" << oldRoomID << XEND;
      return false;
    }
    DWORD destzoneid = pMvpRoom->getZoneID();
    MatchManager::getMe().sendPlayerToFighting(zoneId, destzoneid, charId, pMvpRoom->getRaidId(), pMvpRoom->getGuid(), pMvpRoom->getIndex());
    XLOG << "[Mvp-竞争战], 玩家直接进入副本, 原来线:" << zoneId << "目标线:" << destzoneid << "玩家:" << charId << XEND;
    return true;
  }
  // 若自己队伍不在副本, 则仅能由队长发起报名
  else // session, 已验证, charId为队长
  {
    // 第一次or当前房间已满, 创建新房间
    DWORD cur = now();
    TSetString invalidNames;
    for (int i = 0; i < rev.users_size(); ++i)
    {
      if (userCanJoin(rev.users(i).charid(), cur) == false)
        invalidNames.insert(rev.users(i).name());
    }
    // 存在队友不可进入, 提示队长
    if (!invalidNames.empty())
    {
      for (auto &s : invalidNames)
        MatchManager::getMe().sendSysMsg(charId, zoneId, 25601, MsgParams(s));

      XLOG << "[Mvp-竞争战], 玩家报名, 队伍中存在成员在CD中, 无法进入, 玩家:" << charId << XEND;
      return false;
    }

    MvpMatchRoom* m_pCurMatchRoom = nullptr;
    if (m_qwCurMatchRoomID)
      m_pCurMatchRoom = dynamic_cast<MvpMatchRoom*>(getRoomByid(m_qwCurMatchRoomID));

    if (m_pCurMatchRoom == nullptr || m_pCurMatchRoom->getState() != EROOMSTATE_WAIT_JOIN)
    {
      QWORD guid = MatchManager::getMe().generateGuid();
      m_pCurMatchRoom = NEW MvpMatchRoom(guid, this);
      m_pCurMatchRoom->setBeginMatchTime(now());
      m_mapRooms[m_pCurMatchRoom->getGuid()] = m_pCurMatchRoom;
      m_qwCurMatchRoomID = guid;
    }
    if (m_pCurMatchRoom == nullptr)
      return false;

    // 防止队伍重复报名
    if (m_pCurMatchRoom->getTeam(zoneId, rev.teamid()) != nullptr)
      return false;

    m_pCurMatchRoom->addMember(charId, zoneId, rev);
    MatchManager::getMe().addRoomUser(charId, zoneId, m_pCurMatchRoom->getGuid());

    // ->teamserver, 标记队伍报名状态
    Cmd::NtfJoinRoom cmd;
    cmd.set_roomid(m_pCurMatchRoom->getGuid());
    cmd.set_charid(charId);
    cmd.set_teamid(rev.teamid());
    cmd.set_success(true);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToZone(zoneId, send, len);

    XLOG << "[Mvp-竞争战], 玩家报名成功, 玩家:" << charId << "线:" << zoneId << "队伍:" << rev.teamid() << "房间:" << m_pCurMatchRoom->getGuid() << XEND;

    // 通知队友..todo
  }

  return true;
}

bool MvpMatchRoomMgr::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{
  // 队员跟队长都可以取消报名

  QWORD oldRoomID = MatchManager::getMe().getRoomId(charId, zoneId);
  TeamMatchRoom* pRoom = dynamic_cast<TeamMatchRoom*>(getRoomByid(oldRoomID));
  if (pRoom == nullptr)
  {
    XERR << "[Mvp竞争战-取消报名] 失败，找不到房间 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }

  if (pRoom->isFighting())
  {
    XERR << "[Mvp竞争战-取消报名] 失败，正在战斗不可取消 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  } 
  
  RoomTeam* pTeam = pRoom->getTeamByCharId(zoneId, charId);
  if (!pTeam)
  {
    XERR << "[Mvp竞争战-取消报名] 失败，找不到队伍信息 charid" << charId << "zoneid" << zoneId << "roomid" << rev.roomid() << XEND;
    return false;
  }

  // 通知所有队员, 已被取消报名
  NtfMatchInfoCCmd cmd;
  cmd.set_etype(EPVPTYPE_MVP);
  cmd.set_ismatch(false);
  PROTOBUF(cmd, send, len);
  pTeam->sendCmdToAllMember(send, len, 0);

  TeamMember* pMem = pTeam->getTeamMember(charId);
  if (pMem)
    pTeam->sendMsgToAllMember(7306, MsgParams(pMem->name()));

  kickTeam(pRoom, pTeam); //调用后, pRoom被释放, pTeam被释放
  return true;
}

bool MvpMatchRoomMgr::closeRoom(TeamMatchRoom* pRoom)
{
  if (!pRoom)
    return false;
  
  TSetQWORD users;
  pRoom->getAllUsers(users);
  for (auto &s : users)
  {
    // 清除惩罚时间
    auto it = m_mapUser2ReMatchTime.find(s);
    if (it != m_mapUser2ReMatchTime.end())
      m_mapUser2ReMatchTime.erase(it);
  }

  NtfMatchInfoCCmd cmd;
  cmd.set_etype(EPVPTYPE_MVP);
  cmd.set_isfight(false);
  PROTOBUF(cmd, send, len);
  pRoom->sendCmdToAllMember(send, len);

  pRoom->reset();
  m_mapRooms.erase(pRoom->getGuid());
  
  SAFE_DELETE(pRoom);
  return true;
}

bool MvpMatchRoomMgr::kickTeam(TeamMatchRoom* pRoom, RoomTeam* pTeam)
{
  if (pRoom == nullptr || pTeam == nullptr)
    return false;

  pRoom->kickTeam(pTeam);
  if (pRoom->isEmpty())
  {
    closeRoom(pRoom);
    // pRoom 已释放
  }
  return true;
}

bool MvpMatchRoomMgr::joinFighting(QWORD charId, DWORD zoneId, JoinFightingCCmd& rev)
{
  MatchRoom* pMRoom = nullptr;
  QWORD oldRoomID = MatchManager::getMe().getRoomId(charId, zoneId);
  if (oldRoomID)
  {
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(oldRoomID);
    pMRoom = dynamic_cast<MvpMatchRoom*>(pRoom);
  }

  if (pMRoom == nullptr)
  {
    MatchManager::getMe().sendSysMsg(charId, zoneId, 25609);

    // 通知房间已关闭
    NtfMatchInfoCCmd cmd;
    cmd.set_etype(EPVPTYPE_MVP);
    cmd.set_isfight(false);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);
    XLOG << "[Mvp-竞争战], 队员加入, 房间已关闭, 不可加入, 玩家:" << charId << zoneId << XEND;
    return true;
  }
  JoinRoomCCmd emptyCmd;
  return joinRoom(charId, zoneId, emptyCmd);
}

void MvpMatchRoomMgr::setRoomFighting(MatchRoom* pRoom)
{

}

bool MvpMatchRoomMgr::reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev)
{
  return true;
}

void MvpMatchRoomMgr::onMvpBattleClose()
{
  for (auto &m : m_mapRooms)
  {
    MvpMatchRoom* pRoom = dynamic_cast<MvpMatchRoom*>(m.second);
    if (pRoom)
      pRoom->onBattleClose();

    SAFE_DELETE(m.second);
  }
  m_mapRooms.clear();
  XLOG << "[Mvp-竞争战], mvp竞争战关闭, 清空房间" << XEND;
}

bool MvpMatchRoomMgr::userCanJoin(QWORD charid, DWORD time)
{
  auto it = m_mapUser2ReMatchTime.find(charid);
  if (it != m_mapUser2ReMatchTime.end())
    return time >= it->second;
  return true;
}

void MvpMatchRoomMgr::addUserPunishTime(QWORD charid, DWORD time)
{
  auto it = m_mapUser2ReMatchTime.find(charid);
  if (it != m_mapUser2ReMatchTime.end())
    it->second = time;
  else
    m_mapUser2ReMatchTime[charid] = time;
  XLOG << "[Mvp-竞争战], 玩家记录匹配惩罚时间, 玩家:" << charid << "再次匹配时间:" << time << XEND;
}

void MvpMatchRoomMgr::clearUserPunishTime(QWORD charid)
{
  auto it = m_mapUser2ReMatchTime.find(charid);
  if (it != m_mapUser2ReMatchTime.end())
    m_mapUser2ReMatchTime.erase(it);
  XLOG << "[Mvp-竞争战], 清除玩家匹配惩罚时间, 玩家:" << charid << XEND;
}


//***********************************************************************************
//****************************公会战决战*********************************************
//***********************************************************************************
SuperGvgMatchRoomMgr::SuperGvgMatchRoomMgr()
{

}

SuperGvgMatchRoomMgr::~SuperGvgMatchRoomMgr()
{
  reset();
}

MatchRoom* SuperGvgMatchRoomMgr::getRoomByid(QWORD guid)
{
  auto it = m_mapRooms.find(guid);
  if (it == m_mapRooms.end())
    return nullptr;
  return it->second;
}

bool SuperGvgMatchRoomMgr::joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  QWORD guildid = rev.guildid();
  auto it = m_mapJoinGuilds.find(guildid);
  if (it == m_mapJoinGuilds.end())
  {
    XERR << "[公会战决战-玩家进入], 进入失败, 所在公会未参战, 玩家:" << charId << "公会:" << guildid << XEND;
    return false;
  }
  if (m_eState != EGVGMATCHSTATE_PREFIGHTING && m_eState != EGVGMATCHSTATE_FIGHTING)
  {
    MatchManager::getMe().sendSysMsg(charId, zoneId, 25516);
    XERR << "[公会战决战-玩家进入], 房间尚未开启, 玩家:" << charId << "公会:" << guildid << XEND;
    return false;
  }
  auto m = m_mapRooms.find(it->second.qwRoomID);
  if (m == m_mapRooms.end())
  {
    XERR << "[公会战决战-玩家进入], 进入失败, 找不到房间, 玩家:" << charId << "公会:" << guildid << "房间:" << it->second.qwRoomID << XEND;
    return false;
  }

  if (m->second->getState() != EROOMSTATE_FIGHTING)
  {
    XERR << "[公会战决战-玩家进入], 进入失败, 房间已不可进入, 玩家:" << charId << "公会:" << guildid << "房间:" << it->second.qwRoomID << XEND;
    return false;
  }

  bool isFirstIn = false; // 是否是第一次进入, 若是则copy队伍数据到副本
  auto ituser = m_mapUser2Guild.find(charId);
  if (ituser == m_mapUser2Guild.end())
  {
    m_mapUser2Guild[charId] = guildid;
    isFirstIn = true;
  }

  DWORD destzoneid = m->second->getZoneID();
  DWORD raidid = m->second->getRaidId();
  QWORD roomid = m->second->getGuid();

  DWORD oneSecNum = MiscConfig::getMe().getSuperGvgCFG().dwOneSecJoinUserNum;
  oneSecNum = oneSecNum ? oneSecNum : 20;

  auto waitlist = m_mapWaitUsers.find(destzoneid);
  if (waitlist != m_mapWaitUsers.end())
  {
    auto l = find_if(waitlist->second.begin(), waitlist->second.end(), [charId, zoneId](const SWaitJoinInfo& r) {
        return r.qwCharID == charId && r.dwZoneID == zoneId;
        });
    if (l != waitlist->second.end())
    {
      XERR << "[公会战决战-玩家进入], 玩家已在等待列表中, 不可重复进入, 玩家:" << charId << "公会:" << guildid << "房间:" << it->second.qwRoomID << XEND;
      return false;
    }
  }
  else
  {
    m_mapWaitUsers[destzoneid];
    waitlist = m_mapWaitUsers.find(destzoneid);
  }

  SWaitJoinInfo sinfo;
  sinfo.dwZoneID = zoneId;
  sinfo.dwDestZoneID = destzoneid;
  sinfo.qwCharID = charId;
  sinfo.dwRaidID = raidid;
  sinfo.dwRoomID = roomid;
  sinfo.dwColor = it->second.dwColorIndex;
  waitlist->second.push_back(sinfo);
  if (waitlist->second.size() > oneSecNum)
  {
    DWORD waitsec = waitlist->second.size() / oneSecNum;
    NtfRoomStateCCmd cmd;
    cmd.set_pvp_type(EPVPTYPE_SUGVG);
    cmd.set_state(EROOMSTATE_WAIT_JOIN);
    cmd.set_endtime(now() + waitsec);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(charId, zoneId, send, len);

    MatchManager::getMe().sendSysMsg(charId, zoneId, 25523);
  }
  //MatchManager::getMe().sendPlayerToFighting(zoneId, destzoneid, charId, raidid, roomid, it->second.dwColorIndex);

  // 第一次进入到目标线创建队伍
  if (isFirstIn && rev.teamid() && rev.teammember_size() > 0)
  {
    RoomTeam* pTeam = m->second->getTeam(zoneId, rev.teamid());
    // 队伍中第一个人进入, 先使用队长id去创建队伍
    if (pTeam == nullptr)
    {
      QWORD leaderid = rev.teammember(0); // 队长id
      m->second->addMember(leaderid, zoneId, rev);
      pTeam = m->second->getTeam(zoneId, rev.teamid());

      // 通知原线, 标记队伍已进入决战, 并且请求原线队伍数据到match
      Cmd::NtfJoinRoom cmd;
      cmd.set_roomid(m->first);
      cmd.set_charid(charId);
      cmd.set_teamid(rev.teamid());
      cmd.set_success(true);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToZone(zoneId, send, len);
      XLOG << "[决战-队伍], 队伍第一个人进入, 去新线创建队伍, 玩家:" << charId << zoneId << "目标线:" << m->second->getZoneID() << "队伍:" << rev.teamid() << XEND;
    }
  }

  XLOG << "[公会战决战-玩家进入], 进入成功, 玩家:" << charId << "公会:" << guildid << "原线:" << zoneId << "副本所在线:" << destzoneid << "阵营:" << it->second.dwColorIndex << XEND;

  return true;
}

bool SuperGvgMatchRoomMgr::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{
  for (auto &m : m_mapWaitUsers)
  {
    auto it = find_if(m.second.begin(), m.second.end(), [charId](const SWaitJoinInfo& r) {
        return r.qwCharID == charId;
        });
    if (it == m.second.end())
      continue;

    m.second.erase(it);
    break;
  }

  XLOG << "[公会战决战-玩家离开], 玩家:" << charId << "线:" << zoneId << XEND;
  return true;
}

bool SuperGvgMatchRoomMgr::closeRoom(MatchRoom* pRoom)
{
  if (pRoom == nullptr)
    return false;
  QWORD roomid = pRoom->getGuid();
  auto it = m_mapRooms.find(roomid);
  if (it == m_mapRooms.end())
    return false;
  SAFE_DELETE(pRoom);
  m_mapRooms.erase(it);
  XLOG << "[公会战决战-关闭], 关闭房间:" << roomid << XEND;
  return true;
}

void SuperGvgMatchRoomMgr::oneSecTick(DWORD curTime)
{
  switch(m_eState)
  {
    case EGVGMATCHSTATE_MIN:
      break;
    case EGVGMATCHSTATE_WAITJOIN:
      {
        if (curTime < m_dwTimeTick)
          break;
        // do match
        if (match() == false)
        {
          matchError();
          m_eState = EGVGMATCHSTATE_MIN;
          break;
        }
        m_eState = EGVGMATCHSTATE_MATCHSUCCESS;
        //m_dwTimeTick = curTime + 60 * 3; //26 + 3 = 29
        m_dwTimeTick = m_dwBeginJoinTime + MiscConfig::getMe().getSuperGvgCFG().dwMatchToEnterTime;
      }
      break;
    case EGVGMATCHSTATE_MATCHSUCCESS:
      {
        if (curTime < m_dwTimeTick)
          break;
        // do open room
        m_eState = EGVGMATCHSTATE_PREFIGHTING;
        //m_dwTimeTick = curTime + 60; // 29 + 1 = 30
        m_dwTimeTick = m_dwBeginJoinTime + MiscConfig::getMe().getSuperGvgCFG().dwMatchToFireTime;
      }
      break;
    case EGVGMATCHSTATE_PREFIGHTING:
      {
        if (curTime < m_dwTimeTick)
          break;
        // do open fighting
        m_eState = EGVGMATCHSTATE_FIGHTING;
        m_dwTimeTick = curTime + 60 * 30; // 30 + 30 = 60, 持续30min
      }
      break;
    case EGVGMATCHSTATE_FIGHTING:
      {
        if (curTime < m_dwTimeTick)
          break;
        // do stop
        reset();
        m_eState = EGVGMATCHSTATE_MIN;
        m_dwBeginJoinTime = 0;
        m_dwTimeTick = 0;
      }
      break;
  }

  if (!m_mapWaitUsers.empty())
  {
    DWORD oneSecNum = MiscConfig::getMe().getSuperGvgCFG().dwOneSecJoinUserNum;
    oneSecNum = oneSecNum ? oneSecNum : 20;
    for (auto m = m_mapWaitUsers.begin(); m != m_mapWaitUsers.end(); )
    {
      DWORD num = oneSecNum;
      while(!m->second.empty() && num-- > 0)
      {
        SWaitJoinInfo& s = m->second.front();
        MatchManager::getMe().sendPlayerToFighting(s.dwZoneID, s.dwDestZoneID, s.qwCharID, s.dwRaidID, s.dwRoomID, s.dwColor);
        XLOG << "[公会战决战], 玩家出队, 进入副本成功, 玩家信息:" << s.qwCharID << s.dwZoneID << s.dwDestZoneID << s.dwRoomID << XEND;
        m->second.pop_front();
      }
      if (m->second.empty())
      {
        m = m_mapWaitUsers.erase(m);
        continue;
      }
      ++m;
    }
  }
}

void SuperGvgMatchRoomMgr::joinGuild(const JoinSuperGvgMatchSCmd& rev)
{
  if (m_eState == EGVGMATCHSTATE_MIN)
  {
    m_eState = EGVGMATCHSTATE_WAITJOIN;
    m_mapJoinGuilds.clear();
    m_dwBeginJoinTime = rev.begintime() ? rev.begintime() : now();
    m_dwTimeTick = m_dwBeginJoinTime + SUPERGVG_MATCH_TIME;
    XLOG << "[公会战决战], 开始匹配" << XEND;
  }

  QWORD guildid = rev.guildid();
  if (m_eState != EGVGMATCHSTATE_WAITJOIN)
  {
    XERR << "[公会战决战-报名], 报名失败, 当前状态不支持报名, 公会:" << guildid << "匹配状态:" << m_eState << XEND;
    return;
  }

  auto it = m_mapJoinGuilds.find(guildid);
  if (it != m_mapJoinGuilds.end())
  {
    XERR << "[公会战决战-报名], 报名失败, 重复报名, 公会:" << guildid << XEND;
    return;
  }

  SSuGvgMatchInfo& info = m_mapJoinGuilds[guildid];
  info.qwGuildID = guildid;
  info.dwZoneID = rev.zoneid();
  info.strGuildName = rev.guildname();
  info.strGuildIcon = rev.guildicon();
  info.dwFireCount = rev.firecount();
  info.dwFireScore = rev.firescore();

  XLOG << "[公会战决战-报名], 报名成功, 公会:" << guildid << "zone:" << info.dwZoneID << "胜率信息:" << info.dwFireCount << info.dwFireScore << "当前报名数:" << m_mapJoinGuilds.size() << XEND;
}

bool SuperGvgMatchRoomMgr::match()
{
  DWORD size = m_mapJoinGuilds.size();
  if (size < 2)
  {
    XLOG << "[公会战决战-匹配], 参与公会数不足, 匹配失败, 数量:" << size << "参与公会id:";
    for (auto &m : m_mapJoinGuilds)
      XLOG << m.first;
    XLOG << XEND;
    return false;
  }

  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  struct stTempMatchInfo
  {
    map<DWORD, TSetDWORD> mapZoneID2Guilds; // 线->公会id
    TSetDWORD allZones; // 所有的线
  };

  map<DWORD, stTempMatchInfo> mapLevel2MatchInfo; // 段位 -> 线&公会信息
  for (auto &m : m_mapJoinGuilds)
  {
    SSuGvgMatchInfo& info = m.second;
    float avescore = (info.dwFireCount ? ((float)info.dwFireScore / (float)info.dwFireCount) : 0);
    // 黄金段、白银、青铜
    DWORD lv = rCFG.getLevelByScore(avescore);

    stTempMatchInfo& data = mapLevel2MatchInfo[lv];

    // 记录线->公会id
    auto it = data.mapZoneID2Guilds.find(info.dwZoneID);
    if (it != data.mapZoneID2Guilds.end())
      it->second.insert(info.qwGuildID);
    else
      data.mapZoneID2Guilds[info.dwZoneID].insert(info.qwGuildID);

    // 记录所有线
    data.allZones.insert(info.dwZoneID);
  }

  // 同段位优先匹配到一起, 同线尽量不匹配在一起
  auto getARoomGuilds = [&](DWORD num, TVecQWORD& vecIDs, DWORD& lv) -> bool
  {
    lv = 0;
    TSetDWORD recZones;
    // 从低等级开始匹配 mapLevel2MatchInfo key:1,2,3有序
    for (auto iter = mapLevel2MatchInfo.begin(); iter != mapLevel2MatchInfo.end(); )
    {
      if (vecIDs.size() >= num)
        return true;

      TSetDWORD& allZones = iter->second.allZones;
      map<DWORD, TSetDWORD>& mapZoneID2Guilds = iter->second.mapZoneID2Guilds;

      if (allZones.empty())
        return false;
      DWORD zoneid = 0;
      DWORD trynum = 3;
      // 随机选择一条线, 尽量避免同线
      for (DWORD i = 0; i < trynum; ++i)
      {
        auto p = randomStlContainer(allZones);
        if (!p)
          return false;
        zoneid = *p;
        if (recZones.find(zoneid) == recZones.end() || allZones.size() == 1)
          break;
      }

      auto it = mapZoneID2Guilds.find(zoneid);
      if (it == mapZoneID2Guilds.end() || it->second.empty())
        return false;

      auto s = randomStlContainer(it->second);//随机一个公会
      if (!s)
        return false;
      QWORD guildid = *s;

      recZones.insert(zoneid); // 记录当前房间已存在的线id
      vecIDs.push_back(guildid);
      if (iter->first >= lv)
        lv = iter->first; // 不同等级匹配到一起时, 取高等级

      it->second.erase(guildid);
      if (it->second.empty())
      {
        mapZoneID2Guilds.erase(it);
        allZones.erase(zoneid);

        if (mapZoneID2Guilds.empty())
        {
          iter = mapLevel2MatchInfo.erase(iter);
          continue;
        }
      }
    }
    return true;
  };

  DWORD maxRoomSize = (size + 3) / 4;
  std::list<DWORD> minZones;
  if (thisServer->getMinZones(maxRoomSize, minZones) == false)
  {
    XERR << "[公会战决战-匹配], 异常, 找不到可以使用的线" << XEND;
    return false;
  }

  //4n + 9, 4n + 6, 4n + 3
  DWORD tmpsize = size;
  while(tmpsize)
  {
    DWORD gnum = 4; // 房间公会数
    if (tmpsize % 4 != 0)
    {
      gnum = (tmpsize >= 3 ? 3 : tmpsize);
      tmpsize -= gnum;
    }
    else
    {
      gnum = 4;
      tmpsize -= gnum;
    }

    TVecQWORD vecGuildIDs;
    DWORD roomLv = 0;
    if (getARoomGuilds(gnum, vecGuildIDs, roomLv) == false)
    {
      XERR << "[公会战决战—匹配], 匹配异常, 剩余:" << tmpsize << XEND;
      break;
    }
    if (minZones.empty())
    {
      XERR << "[公会战决战-匹配], 匹配异常, 无可用线, 剩余:" << tmpsize << XEND;
      break;
    }
    // create room
    DWORD zoneid = minZones.back();
    minZones.pop_back();

    QWORD guid = MatchManager::getMe().generateGuid();
    SuperGvgMatchRoom* pRoom = NEW SuperGvgMatchRoom(guid, zoneid);
    if (pRoom == nullptr)
      return false;
    pRoom->setGuild(vecGuildIDs);
    pRoom->setLevel(roomLv);
    pRoom->setMatchRoomMgr(this);
    pRoom->setRaidId(rCFG.dwRaidID);
    pRoom->setState(EROOMSTATE_FIGHTING);

    m_mapRooms[guid] = pRoom;
    // 记录 guildid->roomid 对应关系
    XLOG << "[公会战决战-匹配], 成功创建一个房间, 房间:" << guid << "段位:" << roomLv << "公会信息:";
    DWORD colorindex = 1;
    for (auto &s : vecGuildIDs)
    {
      auto it = m_mapJoinGuilds.find(s);
      if (it == m_mapJoinGuilds.end())
        continue;
      it->second.qwRoomID = guid;
      it->second.dwColorIndex = (colorindex ++);
      float avs = (it->second.dwFireCount ? ((float)it->second.dwFireScore / (float)it->second.dwFireCount) : 0);
      XLOG << it->second.strGuildName << it->second.qwGuildID << avs << ",";
    }
    XLOG << XEND;
  }

  return true;
}

void SuperGvgMatchRoomMgr::matchError()
{

}

void SuperGvgMatchRoomMgr::reset()
{
  m_mapJoinGuilds.clear();
  m_mapUser2Guild.clear();
  for (auto &m : m_mapRooms)
  {
    SAFE_DELETE(m.second);
  }
  m_mapRooms.clear();
}

void SuperGvgMatchRoomMgr::onRaidSceneOpen(MatchRoom* pRoom, DWORD zoneid, DWORD sceneid)
{
  if (pRoom == nullptr)
    return;
  SuperGvgMatchRoom* pSRoom = dynamic_cast<SuperGvgMatchRoom*>(pRoom);
  if (pSRoom == nullptr)
    return;

  const TSetQWORD& guilds = pSRoom->getGuildIDs();

  SyncRoomSceneMatchSCmd cmd;
  cmd.set_roomid(pRoom->getGuid());
  cmd.set_zoneid(zoneid);
  cmd.set_sceneid(sceneid);
  cmd.set_level(pSRoom->getLevel());

  cmd.set_roomsize(guilds.size());
  cmd.set_raidtime(m_dwBeginJoinTime);
  for (auto &s : guilds)
  {
    auto it = m_mapJoinGuilds.find(s);
    if (it == m_mapJoinGuilds.end())
      continue;
    SuperGvgRoomData* pData = cmd.add_sugvgdata();
    if (pData == nullptr)
      continue;
    pData->set_guildid(s);
    pData->set_guildname(it->second.strGuildName);
    pData->set_guildicon(it->second.strGuildIcon);
    pData->set_firecount(it->second.dwFireCount);
    pData->set_firescore(it->second.dwFireScore);
    pData->set_color(it->second.dwColorIndex);
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(zoneid, send, len);
}

// tutor match
void STutorMatcher::fromData(const TutorMatcher& rMatcher)
{
  charid = rMatcher.charid();
  zoneid = rMatcher.zoneid();
  bFindTutor = rMatcher.findtutor();

  eDestGender = rMatcher.gender();
  eSelfGender = rMatcher.selfgender();

  mapDatas.clear();
  for (int i = 0; i < rMatcher.datas_size(); ++i)
  {
    const UserData& rData = rMatcher.datas(i);
    mapDatas[rData.type()].CopyFrom(rData);
  }

  setBlackIDs.clear();
  for (int i = 0; i < rMatcher.blackids_size(); ++i)
    setBlackIDs.insert(rMatcher.blackids(i));
}

void STutorMatcher::toData(TutorMatcher* pMatcher)
{
  if (pMatcher == nullptr)
    return;

  pMatcher->set_charid(charid);
  pMatcher->set_zoneid(zoneid);
  pMatcher->set_findtutor(bFindTutor);

  pMatcher->set_gender(eDestGender);
  pMatcher->set_selfgender(eSelfGender);

  pMatcher->clear_datas();
  for (auto &m : mapDatas)
    pMatcher->add_datas()->CopyFrom(m.second);

  pMatcher->clear_blackids();
  for (auto &s : setBlackIDs)
    pMatcher->add_blackids(s);
}

const string& STutorMatcher::getName() const
{
  auto m = mapDatas.find(EUSERDATATYPE_NAME);
  if (m != mapDatas.end())
    return m->second.data();
  return STRING_EMPTY;
}

TutorMatchRoomMgr::TutorMatchRoomMgr()
{

}

TutorMatchRoomMgr::~TutorMatchRoomMgr()
{

}

bool TutorMatchRoomMgr::joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{
  const TutorMatcher& rMatcher = rev.matcher();
  if (zoneId == 0)
  {
    XERR << "[导师匹配-加入] zoneid :" << zoneId << "charid :" << charId << "使用 gender :" << rMatcher.gender() << "findtutor :" << rMatcher.findtutor() << "加入匹配房间失败,zoneid不合法" << XEND;
    return false;
  }
  if (rMatcher.gender() <= EGENDER_MIN || rMatcher.gender() >= EGENDER_MAX || EGender_IsValid(rMatcher.gender()) == false)
  {
    XERR << "[导师匹配-加入] zoneid :" << zoneId << "charid :" << charId << "使用 gender :" << rMatcher.gender() << "findtutor :" << rMatcher.findtutor() << "加入匹配房间失败,gender不合法" << XEND;
    return false;
  }
  if (isMatch(charId) == true)
  {
    XERR << "[导师匹配-加入] zoneid :" << zoneId << "charid :" << charId << "使用 gender :" << rMatcher.gender() << "findtutor :" << rMatcher.findtutor() << "加入匹配房间失败,已在匹配中" << XEND;
    return false;
  }

  STutorMatcher& rNewMatcher = m_mapMatcher[charId];
  rNewMatcher.fromData(rMatcher);

  TMapTutorZoneGroup& mapGroup = rMatcher.findtutor() ? m_mapTutorGroup : m_mapStudentGroup;
  STutorZoneGroup& rGroup = mapGroup[zoneId];
  TSetQWORD& setIDs = rMatcher.selfgender() == EGENDER_MALE ? rGroup.setMaleIDs : rGroup.setFemaleIDs;

  auto s = setIDs.find(charId);
  if (s != setIDs.end())
  {
    XERR << "[导师匹配-加入] zoneid :" << zoneId << "charid :" << charId << "使用 gender :" << rMatcher.gender() << "findtutor :" << rMatcher.findtutor() << "加入匹配房间失败,异常,已在匹配列表中" << XEND;
    return false;
  }

  setIDs.insert(charId);
  m_setMatchIDs.insert(charId);
  matchResultNtf(charId, 0, ETUTORMATCH_START);

  const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_MATCH, charId);
  RedisManager::getMe().setData<DWORD>(key, 1);

  XLOG << "[导师匹配-加入] zoneid :" << zoneId << "charid :" << charId << "使用 gender :" << rMatcher.gender() << "findtutor :" << rMatcher.findtutor() << "加入匹配房间成功" << XEND;
  showlog(zoneId);
  return true;
}

bool TutorMatchRoomMgr::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{
  bool bRedisOpt = false;
  auto l = find_if(m_listMatching.begin(), m_listMatching.end(), [&](const STutorMatching& r) -> bool{
    return r.tutorid == charId || r.studentid == charId;
  });
  if (l != m_listMatching.end())
  {
    XLOG << "[导师匹配-退出] zoneid :" << zoneId << "charid :" << charId << "正在匹配中,拒绝对方" << XEND;
    response(charId, ETUTORMATCH_REFUSE);
  }

  auto tutor = m_mapTutorGroup.find(zoneId);
  if (tutor != m_mapTutorGroup.end())
  {
    auto s = tutor->second.setMaleIDs.find(charId);
    if (s != tutor->second.setMaleIDs.end())
    {
      XLOG << "[导师匹配-退出] zoneid :" << zoneId << "charid :" << charId << "成功退出男性导师匹配列表" << XEND;
      tutor->second.setMaleIDs.erase(s);
      bRedisOpt = true;
    }
    s = tutor->second.setFemaleIDs.find(charId);
    if (s != tutor->second.setFemaleIDs.end())
    {
      XLOG << "[导师匹配-退出] zoneid :" << zoneId << "charid :" << charId << "成功退出女性导师匹配列表" << XEND;
      tutor->second.setFemaleIDs.erase(s);
      bRedisOpt = true;
    }
  }
  auto student = m_mapStudentGroup.find(zoneId);
  if (student != m_mapStudentGroup.end())
  {
    auto s = student->second.setMaleIDs.find(charId);
    if (s != student->second.setMaleIDs.end())
    {
      XLOG << "[导师匹配-退出] zoneid :" << zoneId << "charid :" << charId << "成功退出男性学生匹配列表" << XEND;
      student->second.setMaleIDs.erase(s);
      bRedisOpt = true;
    }
    s = student->second.setFemaleIDs.find(charId);
    if (s != student->second.setFemaleIDs.end())
    {
      XLOG << "[导师匹配-退出] zoneid :" << zoneId << "charid :" << charId << "成功退出女性学生匹配列表" << XEND;
      student->second.setFemaleIDs.erase(s);
      bRedisOpt = true;
    }
  }

  auto s = m_setMatchIDs.find(charId);
  if (s != m_setMatchIDs.end())
  {
    XLOG << "[导师匹配-退出] zoneid :" << zoneId << "charid :" << charId << "成功退出匹配id列表" << XEND;
    m_setMatchIDs.erase(charId);
    bRedisOpt = true;
  }

  auto matcher = m_mapMatcher.find(charId);
  if (matcher != m_mapMatcher.end())
  {
    XLOG << "[导师匹配-退出] zoneid :" << zoneId << "charid :" << charId << "成功退出匹配信息列表" << XEND;
    matchResultNtf(charId, 0, ETUTORMATCH_STOP);
    m_mapMatcher.erase(matcher);
    bRedisOpt = true;
  }

  if (bRedisOpt)
  {
    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_MATCH, charId);
    RedisManager::getMe().delData(key);
  }
  return true;
}

STutorMatcher* TutorMatchRoomMgr::getMatcher(QWORD charid)
{
  auto m = m_mapMatcher.find(charid);
  if (m != m_mapMatcher.end())
    return &m->second;
  return nullptr;
}

EError TutorMatchRoomMgr::response(QWORD charid, ETutorMatch reply)
{
  if (reply != ETUTORMATCH_AGREE && reply != ETUTORMATCH_REFUSE)
  {
    XERR << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "失败,reply不是应答选项" << XEND;
    return EERROR_FAIL;
  }

  auto l = find_if(m_listMatching.begin(), m_listMatching.end(), [&](const STutorMatching& r) -> bool{
    return r.tutorid == charid || r.studentid == charid;
  });
  if (l == m_listMatching.end())
  {
    XERR << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "失败,未找到匹配者" << XEND;
    return EERROR_FAIL;
  }

  if (l->tutorid == charid)
  {
    l->eTutorResponse = reply;
    if (reply == ETUTORMATCH_AGREE)
      matchResultNtf(0, l->studentid, ETUTORMATCH_AGREE);
  }
  else if (l->studentid == charid)
  {
    l->eStudentResponse = reply;
    if (reply == ETUTORMATCH_AGREE)
      matchResultNtf(l->tutorid, 0, ETUTORMATCH_AGREE);
  }
  else
  {
    XERR << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "失败,未知错误" << XEND;
    return EERROR_FAIL;
  }

  XLOG << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "成功" << XEND;
  if (l->eTutorResponse == ETUTORMATCH_REFUSE || l->eStudentResponse == ETUTORMATCH_REFUSE)
  {
    STutorMatcher* pTutor = getMatcherInfo(l->tutorid);
    if (pTutor != nullptr)
    {
      TMapTutorZoneGroup& mapGroup = pTutor->bFindTutor ? m_mapTutorGroup : m_mapStudentGroup;
      STutorZoneGroup& rGroup = mapGroup[pTutor->zoneid];
      TSetQWORD& setIDs = pTutor->eSelfGender == EGENDER_MALE ? rGroup.setMaleIDs : rGroup.setFemaleIDs;
      setIDs.insert(pTutor->charid);
      m_setMatchIDs.insert(pTutor->charid);
      XLOG << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "成功,有一方拒绝,tutorid :" << pTutor->charid << "重新进入匹配成功" << XEND;
    }
    else
    {
      XERR << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "成功,有一方拒绝,tutorid :" << l->tutorid << "重新进入匹配失败" << XEND;
    }

    STutorMatcher* pStudent = getMatcherInfo(l->studentid);
    if (pStudent != nullptr)
    {
      TMapTutorZoneGroup& mapGroup = pStudent->bFindTutor ? m_mapTutorGroup : m_mapStudentGroup;
      STutorZoneGroup& rGroup = mapGroup[pStudent->zoneid];
      TSetQWORD& setIDs = pStudent->eSelfGender == EGENDER_MALE ? rGroup.setMaleIDs : rGroup.setFemaleIDs;
      setIDs.insert(pStudent->charid);
      m_setMatchIDs.insert(pStudent->charid);
      XLOG << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "成功,有一方拒绝,studentid :" << pStudent->charid << "重新进入匹配成功" << XEND;
    }
    else
    {
      XERR << "[导师匹配-应答] charid :" << charid << "应答 reply :" << reply << "成功,有一方拒绝,tutorid :" << l->studentid << "重新进入匹配失败" << XEND;
    }

    matchResultNtf(l->tutorid, l->studentid, ETUTORMATCH_START);
    m_listMatching.erase(l);

    if (l->eTutorResponse == ETUTORMATCH_REFUSE && pTutor != nullptr && pStudent != nullptr)
      MatchManager::getMe().sendSysMsg(pStudent->charid, pStudent->zoneid, 25449, MsgParams(pTutor->getName()));
    if (l->eStudentResponse == ETUTORMATCH_REFUSE && pTutor != nullptr && pStudent != nullptr)
      MatchManager::getMe().sendSysMsg(pTutor->charid, pTutor->zoneid, 25450, MsgParams(pStudent->getName()));
  }
  else if (l->eTutorResponse == ETUTORMATCH_AGREE && l->eStudentResponse == ETUTORMATCH_AGREE)
  {
    process(l->tutorid, true);
  }

  return EERROR_SUCCESS;
}

void TutorMatchRoomMgr::timeTick(DWORD curTime)
{
  processMatch();

  if (m_dwOverTimeTick < curTime)
  {
    m_dwOverTimeTick = curTime + MIN_T;
    processOverTime();
  }
}

void TutorMatchRoomMgr::processMatch()
{
  xTime frameTime;
  for (auto s = m_setMatchIDs.begin(); s != m_setMatchIDs.end(); ++s)
  {
    if (m_setMatchDoneIDs.find(*s) != m_setMatchDoneIDs.end())
      continue;

    STutorMatcher* pMatcher = getMatcherInfo(*s);
    if (pMatcher == nullptr)
    {
      XERR << "[导师匹配-匹配] charid :" << *s << "匹配失败,未找到匹配者信息" << XEND;
      m_setMatchDoneIDs.insert(*s);
      continue;
    }

    TMapTutorZoneGroup& mapGroup = pMatcher->bFindTutor ? m_mapStudentGroup : m_mapTutorGroup;
    STutorZoneGroup& rGroup = mapGroup[pMatcher->zoneid];

    bool bMatch = matching(rGroup, pMatcher);
    if (!bMatch)
    {
      for (auto &m : mapGroup)
      {
        if (pMatcher->zoneid == m.first)
          continue;

        bMatch = matching(m.second, pMatcher);
        if (bMatch)
          break;
      }
    }

    if (bMatch)
    {
      TMapTutorZoneGroup& mapGroup = !pMatcher->bFindTutor ? m_mapStudentGroup : m_mapTutorGroup;
      STutorZoneGroup& rGroup = mapGroup[pMatcher->zoneid];
      TSetQWORD& setIDs = pMatcher->eSelfGender == EGENDER_MALE ? rGroup.setMaleIDs : rGroup.setFemaleIDs;
      setIDs.erase(pMatcher->charid);
    }

    if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwSocialLoadTime)
    {
      XLOG << "[导师匹配-匹配] 耗时" << frameTime.uElapse() << "微秒" << "超过" << CommonConfig::m_dwSocialLoadTime << "结束当前帧匹配" << XEND;
      break;
    }
  }

  for (auto &s : m_setMatchDoneIDs)
    m_setMatchIDs.erase(s);
  m_setMatchDoneIDs.clear();
}

void TutorMatchRoomMgr::processOverTime()
{
  DWORD dwNow = now();
  TSetQWORD setIDs;
  for (auto l = m_listMatching.begin(); l != m_listMatching.end(); ++l)
  {
    STutorMatching& rMatch = *l;
    if (l->eTutorResponse == ETUTORMATCH_AGREE && l->eStudentResponse == ETUTORMATCH_AGREE)
      continue;
    if (rMatch.dwTick + MIN_T < dwNow)
      setIDs.insert(l->tutorid);
  }
  for (auto &s : setIDs)
  {
    response(s, ETUTORMATCH_REFUSE);
    XLOG << "[导师匹配-超时检查] charid :" << s << "超时未收到应答,自动拒绝" << XEND;
  }
}

void TutorMatchRoomMgr::showlog(DWORD dwZoneID)
{
  STutorZoneGroup& rTutor = m_mapTutorGroup[dwZoneID];
  STutorZoneGroup& rStudent = m_mapStudentGroup[dwZoneID];

  XDBG << "[导师匹配-日志] zoneid :" << dwZoneID << "男性寻找导师人数 :" << rTutor.setMaleIDs.size() << "女性寻找导师人数 :" << rTutor.setFemaleIDs.size()
    << "男性寻找学生人数 :" << rStudent.setMaleIDs.size() << "女性寻找学生人数 :" << rStudent.setFemaleIDs.size() << XEND;
}

void TutorMatchRoomMgr::matchResultNtf(QWORD charid1, QWORD charid2, ETutorMatch eResult)
{
  auto char1 = m_mapMatcher.find(charid1);
  auto char2 = m_mapMatcher.find(charid2);
  if (char1 == m_mapMatcher.end() && char2 == m_mapMatcher.end())
  {
    XERR << "[导师匹配-匹配通知] 通知 charid1 :" << charid1 << "charid2 :" << charid2 << "reuslt :" << eResult << "失败,都不在匹配列表中" << XEND;
    return;
  }

  TutorMatchResultNtfMatchCCmd cmd;
  cmd.set_status(eResult);

  if (char1 != m_mapMatcher.end())
  {
    if (eResult == ETUTORMATCH_MATCH)
    {
      STutorMatcher* pMatcher = getMatcherInfo(charid2);
      if (pMatcher != nullptr)
        pMatcher->toData(cmd.mutable_target());
      else
        XERR << "[导师匹配-匹配通知] 通知 charid1 :" << charid1 << "charid2 :" << charid2 << "reuslt :" << eResult << "中未找到charid2的匹配信息" << XEND;
    }
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(char1->second.charid, char1->second.zoneid, send, len);
    XDBG << "[导师匹配-匹配通知] 通知 charid1 :" << charid1 << "charid2 :" << charid2 << "reuslt :" << cmd.ShortDebugString() << XEND;
  }

  if (char2 != m_mapMatcher.end())
  {
    if (eResult == ETUTORMATCH_MATCH)
    {
      STutorMatcher* pMatcher = getMatcherInfo(charid1);
      if (pMatcher != nullptr)
        pMatcher->toData(cmd.mutable_target());
      else
        XERR << "[导师匹配-匹配通知] 通知 charid1 :" << charid1 << "charid2 :" << charid2 << "reuslt :" << eResult << "中未找到charid1的匹配信息" << XEND;
    }
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(char2->second.charid, char2->second.zoneid, send, len);
    XDBG << "[导师匹配-匹配通知] 通知 charid1 :" << charid1 << "charid2 :" << charid2 << "reuslt :" << cmd.ShortDebugString() << XEND;
  }
}

bool TutorMatchRoomMgr::process(QWORD charid, bool bResult)
{
  auto l = find_if(m_listMatching.begin(), m_listMatching.end(), [&](const STutorMatching& r) -> bool{
    return r.tutorid == charid || r.studentid == charid;
  });
  if (l == m_listMatching.end())
  {
    XERR << "[导师匹配-进度] charid :" << charid << "推进进度失败,未在进度列表中找到该玩家" << XEND;
    return false;
  }

  STutorMatching& rMatching = *l;
  QWORD tutorid = rMatching.tutorid;
  QWORD studentid = rMatching.studentid;

  if (!bResult)
  {
    XERR << "[导师匹配-进度] tutorid :" << rMatching.tutorid << "studentid :" << rMatching.studentid << "推进进度至" << rMatching.eProcess << "结果失败,立即终止匹配" << XEND;
    m_listMatching.erase(l);

    STutorMatcher* pMatcher = getMatcherInfo(studentid);
    if (pMatcher != nullptr)
    {
      LeaveRoomCCmd cmd;
      leaveRoom(pMatcher->charid, pMatcher->zoneid, cmd);
    }
    pMatcher = getMatcherInfo(tutorid);
    if (pMatcher != nullptr)
    {
      LeaveRoomCCmd cmd;
      leaveRoom(pMatcher->charid, pMatcher->zoneid, cmd);
    }
    return false;
  }

  if (rMatching.eProcess == ETUTORPROCESS_RESPONSE)
  {
    rMatching.eProcess = ETUTORPROCESS_TUTOR_APPLY;

    STutorMatcher* pMatcher = getMatcherInfo(rMatching.studentid);
    if (pMatcher == nullptr)
    {
      XERR << "[导师匹配-进度] tutorid :" << rMatching.tutorid << "studentid :" << rMatching.studentid << "推进进度至" << rMatching.eProcess << "未找到studentid :" << rMatching.studentid << "匹配信息" << XEND;
      return false;
    }

    TutorOptMatchSCmd scmd;
    scmd.set_tutorid(rMatching.tutorid);
    scmd.set_studentid(rMatching.studentid);
    scmd.set_opt(ETUTOROPT_APPLY);
    PROTOBUF(scmd, send, len);
    thisServer->sendCmdToZone(pMatcher->zoneid, send, len);
  }
  else if (rMatching.eProcess == ETUTORPROCESS_TUTOR_APPLY)
  {
    rMatching.eProcess = ETUTORPROCESS_STUDENT_AGREE;

    STutorMatcher* pMatcher = getMatcherInfo(rMatching.tutorid);
    if (pMatcher == nullptr)
    {
      XERR << "[导师匹配-进度] tutorid :" << rMatching.tutorid << "studentid :" << rMatching.studentid << "推进进度至" << rMatching.eProcess << "未找到tutorid :" << rMatching.tutorid << "匹配信息" << XEND;
      return false;
    }

    TutorOptMatchSCmd scmd;
    scmd.set_tutorid(rMatching.tutorid);
    scmd.set_studentid(rMatching.studentid);
    scmd.set_opt(ETUTOROPT_AGREE);
    PROTOBUF(scmd, send, len);
    thisServer->sendCmdToZone(pMatcher->zoneid, send, len);
  }
  else if (rMatching.eProcess == ETUTORPROCESS_STUDENT_AGREE)
  {
    m_listMatching.erase(l);
    STutorMatcher* pMatcher = getMatcherInfo(studentid);
    if (pMatcher != nullptr)
    {
      LeaveRoomCCmd cmd;
      leaveRoom(pMatcher->charid, pMatcher->zoneid, cmd);
    }
    pMatcher = getMatcherInfo(tutorid);
    if (pMatcher != nullptr)
    {
      LeaveRoomCCmd cmd;
      leaveRoom(pMatcher->charid, pMatcher->zoneid, cmd);
    }
  }

  XLOG << "[导师匹配-进度] tutorid :" << tutorid << "studentid :" << studentid << "推进进度至" << rMatching.eProcess << XEND;
  return true;
}

bool TutorMatchRoomMgr::isMatch(QWORD charid)
{
  auto m = m_mapMatcher.find(charid);
  if (m != m_mapMatcher.end())
    return true;

  auto l = find_if(m_listMatching.begin(), m_listMatching.end(), [&](const STutorMatching& r) -> bool{
    return r.tutorid == charid || r.studentid == charid;
  });
  return l != m_listMatching.end();
}

bool TutorMatchRoomMgr::matching(STutorZoneGroup& rGroup, STutorMatcher* pMatcher)
{
  if (pMatcher == nullptr)
    return false;

  xTime frameDebug;
  TSetQWORD& setIDs = pMatcher->eDestGender == EGENDER_MALE ? rGroup.setMaleIDs : rGroup.setFemaleIDs;
  if (setIDs.empty() == true)
    return false;

  TSetQWORD setRandIDs;
  for (auto &s : setIDs)
  {
    if (pMatcher->setBlackIDs.find(s) != pMatcher->setBlackIDs.end())
      continue;
    STutorMatcher* pTarget = getMatcherInfo(s);
    if (pTarget == nullptr)
      continue;
    if (pTarget->setBlackIDs.find(pMatcher->charid) != pTarget->setBlackIDs.end())
      continue;
    if (pTarget->eDestGender != pMatcher->eSelfGender)
      continue;
    setRandIDs.insert(s);
  }

  QWORD* p = randomStlContainer(setRandIDs);
  if (p == nullptr)
    return false;

  STutorMatching stMatching;

  stMatching.tutorid = pMatcher->bFindTutor ? *p : pMatcher->charid;
  stMatching.studentid = pMatcher->bFindTutor ? pMatcher->charid : *p;
  stMatching.dwTick = now();
  stMatching.eProcess = ETUTORPROCESS_RESPONSE;

  setIDs.erase(*p);

  auto l = find_if(m_listMatching.begin(), m_listMatching.end(), [&](const STutorMatching& r) -> bool{
      return r.tutorid == stMatching.tutorid || r.studentid == stMatching.tutorid;
      });
  if (l != m_listMatching.end())
  {
    XERR << "[导师匹配-匹配] charid :" << stMatching.tutorid << "异常,在处理列表中存在,移除" << XEND;
    l = m_listMatching.end();
  }
  l = find_if(m_listMatching.begin(), m_listMatching.end(), [&](const STutorMatching& r) -> bool{
      return r.tutorid == stMatching.studentid || r.studentid == stMatching.studentid;
      });
  if (l != m_listMatching.end())
  {
    XERR << "[导师匹配-匹配] charid :" << stMatching.tutorid << "异常,在处理列表中存在,移除" << XEND;
    l = m_listMatching.end();
  }

  m_setMatchDoneIDs.insert(stMatching.tutorid);
  m_setMatchDoneIDs.insert(stMatching.studentid);

  m_listMatching.push_back(stMatching);
  matchResultNtf(stMatching.tutorid, stMatching.studentid, ETUTORMATCH_MATCH);
  XLOG << "[导师匹配-匹配] zoneid :" << pMatcher->zoneid << "charid :" << pMatcher->charid << "和 charid :" << *p << "匹配成功,耗时" << frameDebug.uElapse() << "进入处理列表" << XEND;
  return true;
}

STutorMatcher* TutorMatchRoomMgr::getMatcherInfo(QWORD charid)
{
  auto m = m_mapMatcher.find(charid);
  if (m != m_mapMatcher.end())
    return &m->second;
  return nullptr;
}

/***************************************************************************/
/*********************组队排位赛********************************************/
/***************************************************************************/
void SJoinTeamInfo::notifyTeamExit()
{
  ExitTeamPwsMatchSCmd teamcmd;
  teamcmd.set_zoneid(dwZoneID);
  teamcmd.set_teamid(qwTeamID);
  PROTOBUF(teamcmd, teamsend, teamlen);
  thisServer->forwardCmdToTeamServer(dwZoneID, teamsend, teamlen);
}

void SJoinTeamInfo::notifyMemsMatchStatus(bool match, EPvpType e)
{
  NtfMatchInfoCCmd message;
  message.set_etype(e);
  message.set_ismatch(match);
  PROTOBUF(message, send, len);
  for (auto &s : setUsers)
    thisServer->sendCmdToClient(s, dwZoneID, send, len);
}

void SJoinTeamInfo::sendMsgToAllMember(DWORD msgid, const MsgParams& param)
{
  for (auto &s : setUsers)
  {
    MatchManager::getMe().sendSysMsg(s, dwZoneID, msgid, param);
  }
}

void SJoinTeamInfo::sendCmdToAllMember(void* buf, WORD len)
{
  for (auto &s : setUsers)
  {
    thisServer->sendCmdToClient(s, dwZoneID, buf, len);
  }
}

void SFullTeamInfo::notifyTeamExit()
{
  if (listAllTeams.empty())
  {
    SJoinTeamInfo::notifyTeamExit();
  }
  else
  {
    for (auto &l : listAllTeams)
      l.notifyTeamExit();
  }
}

void SFullTeamInfo::notifyMemsMatchStatus(bool match, EPvpType e)
{
  if (listAllTeams.empty())
  {
    SJoinTeamInfo::notifyMemsMatchStatus(match, e);
  }
  else
  {
    for (auto &l : listAllTeams)
      l.notifyMemsMatchStatus(match, e);
  }
}

void SFullTeamInfo::sendMsgToAllMember(DWORD msgid, const MsgParams& param)
{
  if (listAllTeams.empty())
  {
    SJoinTeamInfo::sendMsgToAllMember(msgid, param);
  }
  else
  {
    for (auto &l : listAllTeams)
      l.sendMsgToAllMember(msgid, param);
  }
}

void SFullTeamInfo::sendCmdToAllMember(void* buf, WORD len)
{
  if (listAllTeams.empty())
  {
    SJoinTeamInfo::sendCmdToAllMember(buf, len);
  }
  else
  {
    for (auto &l : listAllTeams)
      l.sendCmdToAllMember(buf, len);
  }
}

void SPrepareTeamInfo::sendCmdToAllUsers(void* buf, WORD len)
{
  for (auto &l : listTeams)
  {
    l.sendCmdToAllMember(buf, len);
  }
}

const vector<vector<map<DWORD,DWORD>>> TeamPwsMatchRoomMgr::VECTOR_FOR_MATCH = TeamPwsMatchRoomMgr::createVecMap();

TeamPwsMatchRoomMgr::TeamPwsMatchRoomMgr(bool relax, DWORD raidid) : m_bRelaxMode(relax), m_dwRelaxRaidID(raidid)
{
  if (m_bRelaxMode)
  {
    m_ePvpType = EPVPTYPE_TEAMPWS_RELAX;;
    m_bFireOpen = true;
    resetMatchInfo();
  }
  else
  {
    m_ePvpType = EPVPTYPE_TEAMPWS;
  }
}

TeamPwsMatchRoomMgr::~TeamPwsMatchRoomMgr()
{
  for (auto &m : m_mapRooms)
    SAFE_DELETE(m.second);
  m_mapRooms.clear();
}

MatchRoom* TeamPwsMatchRoomMgr::getRoomByid(QWORD guid)
{
  auto it = m_mapRooms.find(guid);
  return it != m_mapRooms.end() ? it->second : nullptr;
}

void TeamPwsMatchRoomMgr::onBattleOpen()
{
  if (m_bFireOpen)
    return;
  m_bFireOpen = true;
  resetMatchInfo();
}

void TeamPwsMatchRoomMgr::resetMatchInfo()
{
  m_vecAMatchPool.clear();
  m_vecBMatchPool.clear();

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode);
  DWORD size = rCFG.vecMatchScoreGroup.size();
  m_vecAMatchPool.reserve(size);
  m_vecBMatchPool.reserve(size);
  std::list<TeamKey> l;
  for (DWORD i = 0; i < size; ++i)
    m_vecAMatchPool.push_back(l);
  pair<DWORD,std::list<TeamKey>> pa;
  pa.first = 0;
  for (DWORD i = 0; i < size; ++i)
    m_vecBMatchPool.push_back(pa);

  m_mapMissingTeams.clear();
  m_mapFullTeams.clear();
  m_mapNum2MissingTeams.clear();
}

const map<DWORD,DWORD>& TeamPwsMatchRoomMgr::findMatchInfo(DWORD needsize)
{
  static const map<DWORD,DWORD> emptymap;
  if (needsize >= VECTOR_FOR_MATCH.size())
    return emptymap;
  for (auto &oneMap : VECTOR_FOR_MATCH[needsize])
  {
    bool find = true;
    for(auto &m : oneMap)
    {
      if (m_mapNum2MissingTeams[m.first].size() < m.second)
      {
        find = false;
        break;
      }
    }
    if (find)
      return oneMap;
  }
  return emptymap;
}

bool TeamPwsMatchRoomMgr::teamJoin(JoinTeamPwsMatchSCmd& cmd, bool bRetry /*=false*/)
{
  if (!m_bFireOpen)
    return false;
  // 获取积分
  if (!bRetry && !cmd.avescore() && cmd.members_size())
  {
    DWORD allscore = 0;
    for (int i = 0; i < cmd.members_size(); ++i)
      allscore += ScoreManager::getMe().getPwsScoreByCharID(cmd.members(i));
    cmd.set_avescore(allscore / cmd.members_size());
  }

  TeamKey key(cmd.zoneid(), cmd.teamid());
  if (m_mapMissingTeams.find(key) != m_mapMissingTeams.end() || m_mapFullTeams.find(key) != m_mapFullTeams.end())
  {
    XERR << "[组队排位赛-报名], 队伍已报名, 队伍:" << cmd.teamid() << "玩家:" << cmd.leaderid() << "休闲模式:" << m_bRelaxMode << XEND;
    return false;
  }

  // 惩罚cd
  for (int i = 0; i < cmd.members_size(); ++i)
  {
    auto it = m_mapUser2PunishCD.find(cmd.members(i));
    if (it != m_mapUser2PunishCD.end() && it->second > now())
    {
      MatchManager::getMe().sendSysMsg(cmd.leaderid(), cmd.zoneid(), 25931);
      XLOG << "[组队排位赛-报名], 玩家处于惩罚CD中, 不可报名, 队伍:" << cmd.zoneid() << cmd.teamid() << "玩家:" << it->first << "休闲模式:" << m_bRelaxMode << XEND;
      return false;
    }
    MatchRoom* pOldRoom = MatchManager::getMe().getRoomByCharId(cmd.members(i), cmd.zoneid());
    if (pOldRoom)
    {
      TeamPwsMatchRoom* pTRoom = dynamic_cast<TeamPwsMatchRoom*> (pOldRoom);
      if (pTRoom)
        MatchManager::getMe().sendSysMsg(cmd.leaderid(), cmd.zoneid(), 25931);

      XERR << "[组队排位赛-报名], 成员已处于房间中, 不可报名, 队伍:" << cmd.teamid() << "队长:" << cmd.leaderid() << "成员:" << cmd.members(i) << "休闲模式:" << m_bRelaxMode << XEND;
      return false;
    }
  }

  // 通知teamserver, 标记报名状态
  ConfirmTeamMatchSCmd retcmd;
  retcmd.set_etype(m_ePvpType);
  retcmd.set_teamid(cmd.teamid());
  PROTOBUF(retcmd, retsend, retlen);
  thisServer->forwardCmdToTeamServer(cmd.zoneid(), retsend, retlen);

  XLOG << "[组队排位赛-报名], 收到队伍报名, 队伍线:" << cmd.zoneid() << "队伍id:" << cmd.teamid() << "人数:" << cmd.members_size() << "均分:" << cmd.avescore() << "当前完整队伍报名数:" << m_mapFullTeams.size() << "不完整队伍数:" << m_mapMissingTeams.size() << "休闲模式:" << m_bRelaxMode << XEND;


  /*
  if (m_mapMissingTeams.size() % 10 == 5)
  {
    for (auto &m : m_mapNum2MissingTeams)
    {
      XLOG << "TEst, " << m.first << m.second.size();
    }
    XLOG << "休闲模式:" << m_bRelaxMode << XEND;

    for (auto &m : m_mapMissingTeams)
    {
      XLOG << "miss info:" << m.second.setUsers.size() << m.first.first << m.first.second;
    }
    XLOG << "休闲模式:" << m_bRelaxMode << XEND;
  }
  */

  // 通知队伍, 报名成功
  NtfMatchInfoCCmd message;
  message.set_etype(m_ePvpType);
  message.set_ismatch(true);
  PROTOBUF(message, send, len);
  for (int i = 0; i < cmd.members_size(); ++i)
    thisServer->sendCmdToClient(cmd.members(i), cmd.zoneid(), send, len);

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode);
  auto oneFullTeamJoin = [&](const SFullTeamInfo& data)
  {
    if (m_vecAMatchPool.empty() || m_vecAMatchPool.size() != m_vecBMatchPool.size())
      return;

    // A 池
    DWORD group = rCFG.getGroupByScore(data.dwAveScore);
    if (group + 1 > m_vecAMatchPool.size()) //异常
      group = m_vecAMatchPool.size() - 1;

    TeamKey key = std::make_pair(data.dwZoneID, data.qwTeamID);
    auto& alist = m_vecAMatchPool[group];

    if (bRetry)
      alist.push_front(key);
    else
      alist.push_back(key);

    m_mapFullTeams[key] = data;

    if (alist.size() == rCFG.dwCacheMatchNum)
    {
      // create room
      vector<TeamKey> vec;
      vec.reserve(rCFG.dwCacheMatchNum);
      for (auto &l : alist)
        vec.push_back(l);
      for (DWORD i = 0; i < (vec.size() / 2 * 2); i += 2)
      {
        enterPrepare(vec[i], vec[i+1]);
        XLOG << "[组队排位赛-A池匹配], 匹配成功, 队伍1:" << vec[i].first << vec[i].second << "队伍2:" << vec[i+1].first << vec[i+1].second << "休闲模式:" << m_bRelaxMode << XEND;
      }

      // erase match data
      for (auto &l : alist)
      {
        auto it = m_mapFullTeams.find(l);
        if (it != m_mapFullTeams.end())
          m_mapFullTeams.erase(it);
      }
      alist.clear();
      m_vecBMatchPool[group].second.clear();
      m_vecBMatchPool[group].first = 0;
      return;
    }
  };

  DWORD teamMaxUser = MiscConfig::getMe().getTeamCFG().dwMaxMember;
  if (cmd.members_size() < (int)teamMaxUser)
  {
    SJoinTeamInfo info;
    info.fromData(cmd);

    DWORD spare = teamMaxUser - cmd.members_size();
    const map<DWORD,DWORD>& mapinfo = findMatchInfo(spare);
    if (mapinfo.empty())
    {
      // not find, 暂存
      m_mapMissingTeams[key] = info;

      // 优先
      if (bRetry)
        m_mapNum2MissingTeams[cmd.members_size()].push_front(key);
      else
        m_mapNum2MissingTeams[cmd.members_size()].push_back(key);
    }
    else
    {
      // find, 组队
      SFullTeamInfo fullinfo;
      fullinfo.listAllTeams.push_back(info);

      for (auto &m : mapinfo)
      {
        auto& list = m_mapNum2MissingTeams[m.first];
        if (list.size() < m.second)
        {
          // 异常
          XERR << "[组队排位赛-队伍匹配], 队伍数不足, 匹配异常!!!" << "休闲模式:" << m_bRelaxMode << XEND;
          break;
        }
        for (DWORD i = 0; i < m.second; ++i)
        {
          auto it = m_mapMissingTeams.find(list.front());
          if (it == m_mapMissingTeams.end())
          {
            //异常
            XERR << "[组队排位赛-队伍匹配], 找不到队伍, 匹配异常!!!" << "休闲模式:" << m_bRelaxMode << XEND;
            break;
          }
          fullinfo.listAllTeams.push_back(it->second);

          m_mapMissingTeams.erase(it);
          list.pop_front();
        }
      }

      // 选择组合队伍中一个队伍为主队伍
      DWORD maxnum = 0;
      DWORD minjointime = DWORD_MAX;
      DWORD sumscore = 0;
      XLOG << "[组队排位赛-队伍匹配], 队伍匹配成功, 队伍个数:" << fullinfo.listAllTeams.size();
      for (auto &l : fullinfo.listAllTeams)
      {
        XLOG << "队伍:" << l.dwZoneID << l.qwTeamID << "均分:" << l.dwAveScore;

        DWORD num = l.setUsers.size();
        sumscore += l.dwAveScore * num;

        if (num > maxnum || ((num == maxnum && l.dwJoinTime < minjointime)))
        {
          fullinfo.qwTeamID = l.qwTeamID;
          fullinfo.qwLeaderid = l.qwLeaderid;
          fullinfo.dwZoneID = l.dwZoneID;

          maxnum = num;
          minjointime = l.dwJoinTime;
        }
        for (auto &s : l.setUsers)
          fullinfo.setUsers.insert(s);
      }
      fullinfo.dwAveScore = sumscore / teamMaxUser;
      fullinfo.dwJoinTime = now();
      XLOG << "主队伍:" << fullinfo.dwZoneID << fullinfo.qwTeamID << "匹配后均分:" << fullinfo.dwAveScore << "休闲模式:" << m_bRelaxMode << XEND;

      // 等价于新完整队伍报名
      oneFullTeamJoin(fullinfo);
    }
  }
  else
  {
    SFullTeamInfo info;
    info.fromData(cmd);
    oneFullTeamJoin(info);
  }

  return true;
}

void TeamPwsMatchRoomMgr::oneSecTick(DWORD curTime)
{
  if (!m_bFireOpen)
    return;

  // 检查匹配超时
  checkMatchTimeOut(curTime);

  // 检查准备超时
  checkPrepareTimeOut(curTime);
}

void TeamPwsMatchRoomMgr::checkMatchTimeOut(DWORD curTime)
{
  if (m_mapMissingTeams.empty() && m_mapFullTeams.empty())
    return;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode);
  // 超时未匹配队伍成功, 取消
  for (auto m = m_mapMissingTeams.begin(); m != m_mapMissingTeams.end();  )
  {
    if (curTime >= m->second.dwJoinTime + rCFG.dwMaxTeamMatchTime)
    {
      XLOG << "[组队排位赛-队伍匹配], 队伍匹配超时, 删除队伍, 队伍线:" << m->second.dwZoneID << "队伍id:" << m->second.qwTeamID << "休闲模式:" << m_bRelaxMode << XEND;

      auto& list = m_mapNum2MissingTeams[m->second.setUsers.size()];
      auto l = find(list.begin(), list.end(), m->first);
      if (l != list.end())
        list.erase(l);

      m->second.sendMsgToAllMember(25925);
      m->second.notifyTeamExit();
      m->second.notifyMemsMatchStatus(false, m_ePvpType);
      m = m_mapMissingTeams.erase(m);
      continue;
    }
    ++m;
  }

  auto rmlist = [&](const DWORD& score, const TeamKey& key)
  {
    DWORD group = rCFG.getGroupByScore(score);
    if (group + 1 < m_vecAMatchPool.size())
    {
      auto it = find(m_vecAMatchPool[group].begin(), m_vecAMatchPool[group].end(), key);
      if (it != m_vecAMatchPool[group].end())
        m_vecAMatchPool[group].erase(it);
    }
    if (group + 1 < m_vecBMatchPool.size())
    {
      auto it = find(m_vecBMatchPool[group].second.begin(), m_vecBMatchPool[group].second.end(), key);
      if (it != m_vecBMatchPool[group].second.end())
        m_vecBMatchPool[group].second.erase(it);
    }
  };

  // 超时未匹配比赛
  std::list<TeamKey> newBlist;
  for (auto m = m_mapFullTeams.begin(); m != m_mapFullTeams.end(); )
  {
    // 超时未匹配成功
    if (curTime >= m->second.dwJoinTime + rCFG.dwMaxFireMatchTime)
    {
      rmlist(m->second.dwAveScore, m->first);
      XLOG << "[组队排位赛-比赛匹配], 匹配超时, 队伍:" << m->second.dwZoneID << m->second.qwTeamID << "休闲模式:" << m_bRelaxMode << XEND;
      m->second.notifyTeamExit();
      m->second.notifyMemsMatchStatus(false, m_ePvpType);
      if (m_bRelaxMode)
        m->second.sendMsgToAllMember(25907);
      else
        m->second.sendMsgToAllMember(25905);
      m = m_mapFullTeams.erase(m);
      continue;
    }

    // A池超时, 进B池
    if (m->second.bMatchBStatus == false && curTime >= m->second.dwJoinTime + rCFG.dwMaxAPoolMatchTime)
    {
      newBlist.push_back(m->first);
      m->second.bMatchBStatus = true;
    }

    ++m;
  }

  if (!newBlist.empty())
  {
    for (auto& l : newBlist)
      joinBPool(l);
  }

  // B 池超时, 强制匹配
  for (DWORD i = 0; i < m_vecBMatchPool.size(); )
  {
    auto& v = m_vecBMatchPool[i];
    if (curTime >= v.first + rCFG.dwMaxBPoolMatchTime && v.second.size() > 1)
    {
      XLOG << "[组队排位赛-B池匹配], 创建时间超过:" << rCFG.dwMaxBPoolMatchTime << "分组:" << i << "开始强制匹配" << "休闲模式:" << m_bRelaxMode << XEND;
      if (i + 1 < m_vecBMatchPool.size())
      {
        matchBPool(i, i + 1);
        i += 2;
      }
      else if (i)
      {
        matchBPool(i, i - 1);
        i += 2;
      }
      else
      {
        ++i;
      }
      continue;
    }
    ++i;
  }
}

void TeamPwsMatchRoomMgr::matchBPool(DWORD group1, DWORD group2)
{
  DWORD size = m_vecBMatchPool[group1].second.size() + m_vecBMatchPool[group2].second.size();
  DWORD exceptGrp = group1;
  TeamKey exceptKey;
  // 总数为奇数
  if (size % 2 == 1)
  {
    if (m_vecBMatchPool[group1].second.size() % 2 == 1)
    {
      exceptKey = m_vecBMatchPool[group1].second.back();
      m_vecBMatchPool[group1].second.pop_back();
      exceptGrp = group1;
    }
    else
    {
      exceptKey = m_vecBMatchPool[group2].second.back();
      m_vecBMatchPool[group2].second.pop_back();
      exceptGrp = group2;
    }
  }
  vector<TeamKey> vec;
  vec.reserve(size / 2 * 2);
  for (auto &k : m_vecBMatchPool[group1].second)
    vec.push_back(k);
  for (auto &k : m_vecBMatchPool[group2].second)
    vec.push_back(k);
  std::random_shuffle(vec.begin(), vec.end());

  // create room
  for (DWORD i = 0; i < (vec.size() / 2 * 2); i+=2)
  {
    enterPrepare(vec[i], vec[i+1]);
    XLOG << "[组队排位赛-B池匹配], B池匹配成功, 队伍1:" << vec[i].first << vec[i].second << "队伍2:" << vec[i+1].first << vec[i+1].second << "休闲模式:" << m_bRelaxMode << XEND;
  }

  // erase match data
  auto eraseData = [&](DWORD g)
  {
    for (auto &l : m_vecBMatchPool[g].second)
    {
      auto s = find(m_vecAMatchPool[g].begin(), m_vecAMatchPool[g].end(), l);
      if (s != m_vecAMatchPool[g].end())
        m_vecAMatchPool[g].erase(s);

      auto it = m_mapFullTeams.find(l);
      if (it != m_mapFullTeams.end())
        m_mapFullTeams.erase(it);
    }
    m_vecBMatchPool[g].second.clear();
    m_vecBMatchPool[g].first = 0;
  };
  eraseData(group1);
  eraseData(group2);

  // 奇数剩余一个, 放回, 重置匹配池创建时间
  if (size % 2 == 1)
  {
    auto it = m_mapFullTeams.find(exceptKey);
    if (it != m_mapFullTeams.end())
    {
      m_vecBMatchPool[exceptGrp].first = it->second.dwJoinTime;
      m_vecBMatchPool[exceptGrp].second.push_back(exceptKey);
    }
  }
  XLOG << "[组队排位赛-B池匹配], B池匹配, 分组:" << group1 << group2 << "休闲模式:" << m_bRelaxMode << XEND;
}

void TeamPwsMatchRoomMgr::joinBPool(const TeamKey& key)
{
  auto it = m_mapFullTeams.find(key);
  if (it == m_mapFullTeams.end())
    return;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode);
  DWORD group = rCFG.getGroupByScore(it->second.dwAveScore);
  DWORD poolsize = m_vecBMatchPool.size();
  if (m_vecBMatchPool.empty() || group + 1 > poolsize)
    return;

  if (m_vecBMatchPool[group].second.empty())
    m_vecBMatchPool[group].first = now();
  m_vecBMatchPool[group].second.push_back(key);
  XLOG << "[组队排位赛-比赛匹配], 队伍进入B池, 队伍:" << key.first << key.second << "休闲模式:" << m_bRelaxMode << XEND;

  // group, group-1 数量达到要求
  if (group && m_vecBMatchPool[group - 1].second.size() + m_vecBMatchPool[group].second.size() == rCFG.dwCacheMatchNum)
  {
    matchBPool(group - 1, group);
    return;
  }

  // group, group+1 数量达到要求
  if (group + 1 < poolsize && m_vecBMatchPool[group + 1].second.size() + m_vecBMatchPool[group].second.size() == rCFG.dwCacheMatchNum)
  {
    matchBPool(group, group + 1);
    return;
  }
}

void TeamPwsMatchRoomMgr::enterPrepare(const TeamKey& key1, const TeamKey& key2)
{
  auto it = m_mapFullTeams.find(key1);
  if (it == m_mapFullTeams.end())
    return;
  auto it2 = m_mapFullTeams.find(key2);
  if (it2 == m_mapFullTeams.end())
    return;

  QWORD guid = MatchManager::getMe().generateGuid();

  SPrepareTeamInfo& info = m_mapPrepareTeamInfo[guid];
  info.qwGuid = guid;
  info.listTeams.push_back(it->second);
  info.listTeams.push_back(it2->second);
  info.dwExpireTime = now() + MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode).dwMaxPrepareTime;

  TeamPwsPreInfoMatchCCmd cmd;
  cmd.set_etype(m_ePvpType);
  for (auto &l : info.listTeams)
  {
    TeamPwsPreInfo* pTeamInfo = cmd.add_teaminfos();
    if (pTeamInfo == nullptr)
      continue;
    for (auto &s : l.setUsers)
    {
      pTeamInfo->add_charids(s);
      m_mapUserID2PrepareGuid[s] = guid;
    }
  }
  PROTOBUF(cmd, send, len);
  info.sendCmdToAllUsers(send, len);

  XLOG << "[组队排位赛-比赛匹配], 匹配成功,创建房间, 队伍1:" << key1.first << key1.second  << "队伍2:" << key2.first << key2.second << "guid:" << guid << "休闲模式:" << m_bRelaxMode << XEND;
}

void TeamPwsMatchRoomMgr::checkPrepareTimeOut(DWORD curTime)
{
  if (m_mapPrepareTeamInfo.empty())
    return;

  auto checkSet = [](const TSetQWORD& all, const TSetQWORD& part) -> bool
  {
    for (auto &s : part)
    {
      if (all.find(s) == all.end())
        return false;
    }
    return true;
  };

  // teamjoin 会添加m_mapPrepareTeamInfo
  std::list<JoinTeamPwsMatchSCmd> listRetryInfo;
  JoinTeamPwsMatchSCmd cmd;

  for (auto m = m_mapPrepareTeamInfo.begin(); m != m_mapPrepareTeamInfo.end();  )
  {
    if (curTime >= m->second.dwExpireTime)
    {
      XLOG << "[组队排位赛-准备超时], 房间:" << m->second.qwGuid << "重新回到匹配的队伍:";
      // 超时未准备
      for (auto &l : m->second.listTeams)
      {
        // 非组合队伍
        if (l.listAllTeams.empty())
        {
          // 当前小队中都准备了, 满6人队伍
          if (checkSet(m->second.setReadyUsers, l.setUsers))
          {
            l.toData(cmd);
            listRetryInfo.push_back(cmd);
            l.notifyMemsMatchStatus(true, m_ePvpType);
            XLOG << "队伍:" << l.dwZoneID << l.qwTeamID;
          }
          else
          {
            l.notifyTeamExit();
            l.notifyMemsMatchStatus(false, m_ePvpType);
          }
        }
        // 组合队伍
        else
        {
          for (auto &t : l.listAllTeams)
          {
            // 当前小队中都准备了, 不满6人队伍
            if (checkSet(m->second.setReadyUsers, t.setUsers))
            {
              t.toData(cmd);
              listRetryInfo.push_back(cmd);
              t.notifyMemsMatchStatus(true, m_ePvpType);
              XLOG << "队伍:" << l.dwZoneID << l.qwTeamID;
            }
            else
            {
              t.notifyTeamExit();
              t.notifyMemsMatchStatus(false, m_ePvpType);
            }
          }
        }
      }
      XLOG << "休闲模式:" << m_bRelaxMode << XEND;
      m = m_mapPrepareTeamInfo.erase(m);
      continue;
    }
    ++m;
  }

  if (!listRetryInfo.empty())
  {
    for (auto &l : listRetryInfo)
      teamJoin(l, true);
  }
}

bool TeamPwsMatchRoomMgr::isUserInPrepare(QWORD charid)
{
  return m_mapUserID2PrepareGuid.find(charid) != m_mapUserID2PrepareGuid.end();
}

bool TeamPwsMatchRoomMgr::userBeReady(QWORD charid)
{
  auto it = m_mapUserID2PrepareGuid.find(charid);
  if (it == m_mapUserID2PrepareGuid.end())
  {
    XERR << "[组队排位赛-玩家准备], 找不到玩家报名信息, 玩家:" << charid << "休闲模式:" << m_bRelaxMode << XEND;
    return false;
  }

  auto m = m_mapPrepareTeamInfo.find(it->second);
  if (m == m_mapPrepareTeamInfo.end())
  {
    XERR << "[组队排位赛-玩家准备], 找不到玩家房间信息, 玩家:" << charid << "休闲模式:" << m_bRelaxMode << XEND;
    return false;
  }

  m->second.setReadyUsers.insert(charid);
  UpdatePreInfoMatchCCmd cmd;
  cmd.set_charid(charid);
  cmd.set_etype(m_ePvpType);
  PROTOBUF(cmd, send, len);
  m->second.sendCmdToAllUsers(send, len);

  XLOG << "[组队排位赛-玩家准备], 准备完成, 玩家:" << charid << "房间:" << m->second.qwGuid << "休闲模式:" << m_bRelaxMode << XEND;

  // 所有人准备完成
  DWORD teamMaxUser = MiscConfig::getMe().getTeamCFG().dwMaxMember;
  if (m->second.setReadyUsers.size() == 2 * teamMaxUser)
  {
    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode);

    DWORD tarZoneid = thisServer->getAZoneId();
    TeamPwsMatchRoom* pRoom = NEW TeamPwsMatchRoom(m->second.qwGuid, tarZoneid, m_ePvpType);
    if (pRoom == nullptr)
      return false;

    pRoom->setMatchRoomMgr(this);
    if (m_bRelaxMode && m_dwRelaxRaidID)
    {
      pRoom->setRaidId(m_dwRelaxRaidID);
    }
    else
    {
      auto p = randomStlContainer(rCFG.setRaidIDs);
      if (!p)
        return false;
      pRoom->setRaidId(*p);
    }
    m_mapRooms[m->second.qwGuid] = pRoom;

    DWORD color = 1;
    for (auto &l : m->second.listTeams)
    {
      // 请求主队 队伍信息到match
      Cmd::NtfJoinRoom cmd;
      cmd.set_roomid(m->second.qwGuid);
      cmd.set_charid(l.qwLeaderid);
      cmd.set_teamid(l.qwTeamID);
      cmd.set_success(true);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToZone(l.dwZoneID, send, len);

      // 添加队长, 预创建队伍
      pRoom->addMember(l.qwLeaderid, l.dwZoneID, l.qwTeamID);
      //pRoom->addTeamInfo(l.qwLeaderid, l.setUsers);

      // 非组合队伍
      if (l.listAllTeams.empty())
      {
        l.notifyTeamExit();
        pRoom->addTeamInfo(l.qwLeaderid, l.setUsers, l.dwZoneID);
        for (auto &s : l.setUsers)
        {
          //MatchManager::getMe().sendPlayerToFighting(l.dwZoneID, tarZoneid, s, rCFG.dwRaidID, m->second.qwGuid, color);
          m_setRoomUsers.insert(s);
          XLOG << "[组队排位赛-玩家进入成功], 玩家:" << s << "原线:" << l.dwZoneID << "目标线:" << tarZoneid << "旧的队伍:" << l.qwTeamID << "房间:" << m->second.qwGuid << "休闲模式:" << m_bRelaxMode << XEND;
        }
      }
      // 组合队伍
      else
      {
        for (auto &t : l.listAllTeams)
        {
          t.notifyTeamExit();
          pRoom->addTeamInfo(l.qwLeaderid, t.setUsers, t.dwZoneID);
          for (auto &s : t.setUsers)
          {
            //MatchManager::getMe().sendPlayerToFighting(t.dwZoneID, tarZoneid, rCFG.dwRaidID, m->second.qwGuid, color);
            m_setRoomUsers.insert(s);
            XLOG << "[组队排位赛-玩家进入成功], 玩家:" << s << "原线:" << t.dwZoneID << "目标线:" << tarZoneid << "旧的队伍:" << t.qwTeamID << "房间:" << m->second.qwGuid << "休闲模式:" << m_bRelaxMode << XEND;
          }
        }
      }

      for (auto &s : l.setUsers)
      {
        // 删除索引
        auto it = m_mapUserID2PrepareGuid.find(s);
        if (it != m_mapUserID2PrepareGuid.end())
          m_mapUserID2PrepareGuid.erase(it);
      }

      color ++;
    }

    // 删除准备信息
    m_mapPrepareTeamInfo.erase(m);
  }
  return true;
}

bool TeamPwsMatchRoomMgr::closeRoom(MatchRoom* pRoom)
{
  TeamPwsMatchRoom* pTRoom = dynamic_cast<TeamPwsMatchRoom*> (pRoom);
  if (pTRoom == nullptr)
    return false;
  QWORD roomid = pTRoom->getGuid();
  auto it = m_mapRooms.find(roomid);
  if (it == m_mapRooms.end())
    return false;

  // 清除记录
  TSetQWORD users;
  pTRoom->getAllUsers(users);
  for (auto &s : users)
    m_setRoomUsers.erase(s);

  SAFE_DELETE(pTRoom);
  m_mapRooms.erase(it);
  XLOG << "[组队排位赛-关闭], 关闭房间:" << roomid << "休闲模式:" << m_bRelaxMode << XEND;
  return true;
}

// teamserver -> match, 成员变化等, 不需要回teamserver, 设置状态
bool TeamPwsMatchRoomMgr::teamLeave(DWORD zoneid, QWORD teamid)
{
  if (doTeamExit(zoneid, teamid))
  {
    XLOG << "[组队排位赛-队伍状态变化], 退出报名, 队伍:" << zoneid << teamid << "休闲模式:" << m_bRelaxMode << XEND;
    return true;
  }
  else
  {
    XERR << "[组队排位赛-队伍状态变化], 退出报名异常, 队伍:" << zoneid << teamid << "休闲模式:" << m_bRelaxMode << XEND;
    return false;
  }
  return true;
}

bool TeamPwsMatchRoomMgr::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{
  if (doTeamExit(zoneId, rev.teamid()))
  {
    // 客户端退出, 通知teamserver更新状态
    ExitTeamPwsMatchSCmd cmd;
    cmd.set_zoneid(zoneId);
    cmd.set_teamid(rev.teamid());
    PROTOBUF(cmd, send, len);
    thisServer->forwardCmdToTeamServer(zoneId, send, len);
    XLOG << "[组队排位赛-退出], 玩家主动取消, 队伍退出报名, 队伍:" << zoneId << rev.teamid() << "玩家:" << charId << "休闲模式:" << m_bRelaxMode << XEND;
    return true;
  }
  else
  {
    XERR << "[组队排位赛-退出异常], 玩家主动取消, 队伍退出报名, 队伍:" << zoneId << rev.teamid() << "玩家:" << charId << "休闲模式:" << m_bRelaxMode << XEND;
    return false;
  }
  return true;
}

bool TeamPwsMatchRoomMgr::isTeamInMatch(DWORD zoneid, QWORD teamid)
{
  TeamKey key = std::make_pair(zoneid, teamid);
  if (m_mapMissingTeams.find(key) != m_mapMissingTeams.end())
    return true;
  if (m_mapFullTeams.find(key) != m_mapFullTeams.end())
    return true;
  for (auto f = m_mapFullTeams.begin(); f != m_mapFullTeams.end(); ++f)
  {
    if (f->second.listAllTeams.empty())
      continue;
    auto t = find_if(f->second.listAllTeams.begin(), f->second.listAllTeams.end(), [&zoneid, &teamid](const SJoinTeamInfo& info) -> bool{
        return info.dwZoneID == zoneid && info.qwTeamID == teamid;
        });
    if (t != f->second.listAllTeams.end())
      return true;
  }

  for (auto p = m_mapPrepareTeamInfo.begin(); p != m_mapPrepareTeamInfo.end(); ++p)
  {
    bool find = false;
    auto findfunc = [&zoneid, &teamid, &find](const SJoinTeamInfo& info)
    {
      if (info.dwZoneID == zoneid && info.qwTeamID == teamid)
        find = true;
    };
    p->second.foreach(findfunc);
    if (find)
      return true;
  }
  return false;
}

bool TeamPwsMatchRoomMgr::doTeamExit(DWORD zoneid, QWORD teamid)
{
  TeamKey key = std::make_pair(zoneid, teamid);

  auto it = m_mapMissingTeams.find(key);
  if (it != m_mapMissingTeams.end())
  {
    DWORD size = it->second.setUsers.size();
    auto &teamlist = m_mapNum2MissingTeams[size];
    auto l = find(teamlist.begin(), teamlist.end(), key);
    if (l != teamlist.end())
      teamlist.erase(l);

    it->second.notifyMemsMatchStatus(false, m_ePvpType);
    m_mapMissingTeams.erase(it);
    XLOG << "[组队排位赛-取消报名], 当前队伍匹配队伍中, 队伍:" << zoneid << teamid << "休闲模式:" << m_bRelaxMode << XEND;
    return true;
  }

  auto erasepool = [&](const TeamKey& k)
  {
    for (auto &v : m_vecAMatchPool)
    {
      auto l = find(v.begin(), v.end(), k);
      if (l != v.end())
      {
        v.erase(l);
        break;
      }
    }
    for (auto &v : m_vecBMatchPool)
    {
      auto l = find(v.second.begin(), v.second.end(), k);
      if (l != v.second.end())
      {
        v.second.erase(l);
        break;
      }
    }
  };
  auto m = m_mapFullTeams.find(key);
  if (m != m_mapFullTeams.end())
  {
    erasepool(key);
    m->second.notifyMemsMatchStatus(false, m_ePvpType);
    m_mapFullTeams.erase(m);
    XLOG << "[组队排位赛-取消报名], 当前队伍匹配比赛中, 队伍:" << zoneid << teamid << "休闲模式:" << m_bRelaxMode << XEND;
    return true;
  }

  // 导致组合队伍解散, 重新匹配
  std::list<JoinTeamPwsMatchSCmd> listRetryInfo;
  JoinTeamPwsMatchSCmd cmd;

  for (auto f = m_mapFullTeams.begin(); f != m_mapFullTeams.end(); ++f)
  {
    if (f->second.listAllTeams.empty())
      continue;
    auto t = find_if(f->second.listAllTeams.begin(), f->second.listAllTeams.end(), [&zoneid, &teamid](const SJoinTeamInfo& info) -> bool{
        return info.dwZoneID == zoneid && info.qwTeamID == teamid;
        });
    if (t != f->second.listAllTeams.end())
    {
      t->notifyMemsMatchStatus(false, m_ePvpType);

      f->second.listAllTeams.erase(t);
      for (auto &l : f->second.listAllTeams)
      {
        l.toData(cmd);
        listRetryInfo.push_back(cmd);
      }

      erasepool(f->first);
      m_mapFullTeams.erase(f);
      break;
    }
  }

  if (listRetryInfo.empty())
  {
    for (auto p = m_mapPrepareTeamInfo.begin(); p != m_mapPrepareTeamInfo.end(); ++p)
    {
      bool find = false;
      auto findfunc = [&zoneid, &teamid, &find](const SJoinTeamInfo& info)
      {
        if (info.dwZoneID == zoneid && info.qwTeamID == teamid)
          find = true;
      };
      p->second.foreach(findfunc);
      if (find)
      {
        auto getother = [&](SJoinTeamInfo& info)
        {
          if (info.dwZoneID == zoneid && info.qwTeamID == teamid)
          {
            info.notifyMemsMatchStatus(false, m_ePvpType);
            return;
          }
          info.notifyMemsMatchStatus(true, m_ePvpType);

          info.toData(cmd);
          listRetryInfo.push_back(cmd);
        };
        p->second.foreach(getother);

        m_mapPrepareTeamInfo.erase(p);
        break;
      }
    }
  }

  if (!listRetryInfo.empty())
  {
    XLOG << "[组队排位赛-取消报名], 当前队伍已进入组合队伍, 取消, 导致其他队伍重新报名, 队伍:" << zoneid << teamid << "休闲模式:" << m_bRelaxMode << XEND;

    for (auto &l : listRetryInfo)
      teamJoin(l, true);

    return true;
  }

  XERR << "[组队排位赛-取消报名], 找不到队伍信息, 队伍:" << zoneid << teamid << "休闲模式:" << m_bRelaxMode << XEND;
  return false;
}

void TeamPwsMatchRoomMgr::onRaidSceneOpen(MatchRoom* pRoom, DWORD zoneid, DWORD sceneid)
{
  if (pRoom == nullptr)
    return;
  TeamPwsMatchRoom* pSRoom = dynamic_cast<TeamPwsMatchRoom*>(pRoom);
  if (pSRoom == nullptr)
    return;

  SyncRoomSceneMatchSCmd cmd;
  cmd.set_roomid(pRoom->getGuid());
  cmd.set_zoneid(zoneid);
  cmd.set_sceneid(sceneid);
  cmd.set_pvptype(m_ePvpType);
  pSRoom->formatTeamInfo(cmd);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(zoneid, send, len);
}

void TeamPwsMatchRoomMgr::punishUserLeaveTeam(QWORD charid)
{
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode);
  m_mapUser2PunishCD[charid] = now() + rCFG.dwLeaveTeamPunishCD;
  ScoreManager::getMe().updatePwsUserScore(charid, rCFG.intLeaveTeamPunishScore);

  XLOG << "[组队排位赛-玩家添加惩罚], 玩家:" << charid << "CD:" << rCFG.dwLeaveTeamPunishCD << "积分:" << rCFG.intLeaveTeamPunishScore << "休闲模式:" << m_bRelaxMode << XEND;
}

bool TeamPwsMatchRoomMgr::isInMatching(QWORD charid)
{
  for (auto &m : m_mapMissingTeams)
  {
    if (m.second.setUsers.find(charid) != m.second.setUsers.end())
      return true;
  }
  for (auto &m : m_mapFullTeams)
  {
    if (m.second.setUsers.find(charid) != m.second.setUsers.end())
      return true;
  }
  for (auto &m : m_mapPrepareTeamInfo)
  {
    for (auto &l : m.second.listTeams)
    {
      if (l.setUsers.find(charid) != l.setUsers.end())
        return true;
    }
  }

  return m_setRoomUsers.find(charid) != m_setRoomUsers.end();
}

/********************组队排位赛休闲模式*********************/
/***********************************************************/
/***********************************************************/
TeamPwsRelaxMatchRoomMgr::TeamPwsRelaxMatchRoomMgr() : TeamPwsMatchRoomMgr(true)
{
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelaxMode);
  for (auto &s : rCFG.setRaidIDs)
  {
    TeamPwsMatchRoomMgr* pOneRaidMgr = NEW TeamPwsMatchRoomMgr(true, s);
    if (pOneRaidMgr)
      m_mapRaid2RelaxMgrs[s] = pOneRaidMgr;
  }
}

TeamPwsRelaxMatchRoomMgr::~TeamPwsRelaxMatchRoomMgr()
{
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    SAFE_DELETE(m.second);
  }
  m_mapRaid2RelaxMgrs.clear();
}

MatchRoom* TeamPwsRelaxMatchRoomMgr::getRoomByid(QWORD guid)
{
  MatchRoom* p = nullptr;
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    p = m.second->getRoomByid(guid);
    if (p)
      break;
  }
  return p;
}

bool TeamPwsRelaxMatchRoomMgr::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    if (m.second->isTeamInMatch(zoneId, rev.teamid()))
    {
      m.second->leaveRoom(charId, zoneId, rev);
      return true;
    }
  }
  return false;
}

bool TeamPwsRelaxMatchRoomMgr::closeRoom(MatchRoom* pRoom)
{
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    if (m.second->closeRoom(pRoom))
      return true;
  }
  return false;
}

void TeamPwsRelaxMatchRoomMgr::onRaidSceneOpen(MatchRoom* pRoom, DWORD zoneid, DWORD sceneid)
{
  if (!pRoom)
    return;
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    if (m.second->getRoomByid(pRoom->getGuid()))
    {
      m.second->onRaidSceneOpen(pRoom, zoneid, sceneid);
      break;
    }
  }
}

bool TeamPwsRelaxMatchRoomMgr::teamJoin(JoinTeamPwsMatchSCmd& cmd, bool bRetry)
{
  DWORD raidid = cmd.roomid();
  auto it = m_mapRaid2RelaxMgrs.find(raidid);
  if (it == m_mapRaid2RelaxMgrs.end())
    return false;
  it->second->teamJoin(cmd);
  return true;
}

bool TeamPwsRelaxMatchRoomMgr::teamLeave(DWORD zoneid, QWORD teamid)
{
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    if (m.second->isTeamInMatch(zoneid, teamid))
    {
      m.second->teamLeave(zoneid, teamid);
      return true;
    }
  }
  return false;
}

bool TeamPwsRelaxMatchRoomMgr::userBeReady(QWORD charid)
{
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    if (m.second->isUserInPrepare(charid))
    {
      m.second->userBeReady(charid);
      return true;
    }
  }
  return false;
}

void TeamPwsRelaxMatchRoomMgr::oneSecTick(DWORD curTime)
{
  for (auto &m : m_mapRaid2RelaxMgrs)
    m.second->oneSecTick(curTime);
}

bool TeamPwsRelaxMatchRoomMgr::isInMatching(QWORD charid)
{
  for (auto &m : m_mapRaid2RelaxMgrs)
  {
    if (m.second->isInMatching(charid))
      return true;
  }
  return false;
}
