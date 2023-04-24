#include "MatchManager.h"
#include "MatchServer.h"
#include "CommonConfig.h"
#include "GlobalShopMgr.h"
#include "ScoreManager.h"

MatchManager::MatchManager():m_oneSecTimer(1),m_tenSecTimer(10)
{
}

MatchManager::~MatchManager()
{
  if (m_llhRoomMgr)
    SAFE_DELETE(m_llhRoomMgr);
  if (m_smzlRoomMgr)
    SAFE_DELETE(m_smzlRoomMgr);
  if (m_hljsRoomMgr)
    SAFE_DELETE(m_hljsRoomMgr);
  if (m_pollyRoomMgr)
    SAFE_DELETE(m_pollyRoomMgr);
  if (m_pMvpRoomMgr)
    SAFE_DELETE(m_pMvpRoomMgr);
  if (m_pSuGvgRoomMgr)
    SAFE_DELETE(m_pSuGvgRoomMgr);
  if (m_pTutorRoomMgr)
    SAFE_DELETE(m_pTutorRoomMgr);
  if (m_pTeamPwsMatchRoomMgr)
    SAFE_DELETE(m_pTeamPwsMatchRoomMgr);
  if (m_pTeamPwsRelaxMgr)
    SAFE_DELETE(m_pTeamPwsRelaxMgr);
}

void MatchManager::init()
{
  m_llhRoomMgr = NEW LLHMatchRoomMgr();
  if (m_llhRoomMgr == nullptr)
  {
    return;
  }
  
  m_smzlRoomMgr = NEW SMZLMatchRoomMgr();
  if (m_smzlRoomMgr == nullptr)
    return;   

  m_hljsRoomMgr = NEW HLJSMatchRoomMgr();
  if (m_hljsRoomMgr == nullptr)
  {
    return;
  }

  m_pollyRoomMgr = NEW PollyMatchRoomMgr();
  if (m_pollyRoomMgr == nullptr)
    return;

  m_pSuGvgRoomMgr = NEW SuperGvgMatchRoomMgr();
  if (m_pSuGvgRoomMgr == nullptr)
    return;

  m_pMvpRoomMgr = NEW MvpMatchRoomMgr();
  m_pTutorRoomMgr = NEW TutorMatchRoomMgr();
  m_pTeamPwsMatchRoomMgr = NEW TeamPwsMatchRoomMgr();
  m_pTeamPwsRelaxMgr = NEW TeamPwsRelaxMatchRoomMgr();
}

bool MatchManager::loadConfig()
{
  return true;
}

bool MatchManager::doUserCmd(QWORD charId, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  XDBG << "[斗技场-收到玩家消息]" << cmd->cmd << cmd->param << XEND;

  if (getClientZoneID(zoneId) > 9996)
  {
    XERR << "[斗技场-收到玩家消息] zoneid 非法,charId"<<charId<<"zoneid"<<zoneId << cmd->cmd << cmd->param << XEND;
    return false;
  }

  //MessageStatHelper msgStat(cmd->cmd, cmd->param);
  switch (cmd->param)
  {
  case MATCHCPARAM_REQ_MY_ROOM:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReqMyRoomMatchCCmd, rev);
    return reqMyRoom(charId, zoneId, rev);
  }
  break;
  case MATCHCPARAM_REQ_ROOM_LIST:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReqRoomListCCmd, rev);
    return reqRoomList(charId, zoneId, rev);
  }
  break;
  case MATCHCPARAM_REQ_ROOM_DETAIL:
  {
    PARSE_CMD_PROTOBUF(Cmd::ReqRoomDetailCCmd, rev);
    return reqRoomDetail(charId, zoneId, rev);
  }
  break;
  case MATCHCPARAM_JOIN_ROOM:
  {
    PARSE_CMD_PROTOBUF(Cmd::JoinRoomCCmd, rev);
    return joinRoom(charId, zoneId, rev);
  }
  break;
  case MATCHCPARAM_LEAVE_ROOM:
  {
    PARSE_CMD_PROTOBUF(Cmd::LeaveRoomCCmd, rev);
    return leaveRoom(charId, zoneId, rev);
  }
  break;
  case MATCHCPARAM_JOIN_FIGHTING:
  {
    PARSE_CMD_PROTOBUF(Cmd::JoinFightingCCmd, rev);
    return joinFighting(charId, zoneId, rev);
  }
  break;
  case MATCHCPARAM_REV_CHALLENGE:
  {
    PARSE_CMD_PROTOBUF(Cmd::RevChallengeCCmd, rev);
    
    if (m_smzlRoomMgr == nullptr)
    {
      return false;
    }    
    SMZLMatchRoom* pRoom = dynamic_cast<SMZLMatchRoom*>(m_smzlRoomMgr->getRoomByid(rev.roomid()));

    if (pRoom == nullptr)
    {
      return false;
    }

    pRoom->processChallengeRes(charId, rev.reply());

    return true;
  }
  break;
  case MATCHCPARAM_FIGHT_CONFIRM:
  {
    PARSE_CMD_PROTOBUF(Cmd::FightConfirmCCmd, rev);

    if (m_smzlRoomMgr == nullptr)
    {
      return false;
    }
    SMZLMatchRoom* pRoom = dynamic_cast<SMZLMatchRoom*>(m_smzlRoomMgr->getRoomByid(rev.roomid()));

    if (pRoom == nullptr)
    {
      return false;
    }
    if (rev.reply() == EMATCHREPLY_AGREE)
      pRoom->sendComfirmRes(zoneId, charId, rev.teamid());
    
    XLOG << "[斗技场-玩家确认加入战斗] zoneid" << zoneId << "charid" << charId << "roomid" << rev.roomid() << "teamid" << rev.teamid() << (rev.reply() == EMATCHREPLY_AGREE ? "同意" : "拒绝") << XEND;
    return true;
  }
  break;
  case MATCHCPARAM_OPEN_GLOBAL_SHOP_PANEL:
  {
    PARSE_CMD_PROTOBUF(Cmd::OpenGlobalShopPanelCCmd, rev);
    if (rev.open())
      GlobalShopMgr::getMe().onOpenPanel(charId, zoneId);
    else
      GlobalShopMgr::getMe().onClosePanel(charId);
  }
  return true;
  case MATCHCPARAM_TUTOR_MATCHRESPONSE:
  {
    PARSE_CMD_PROTOBUF(TutorMatchResponseMatchCCmd, rev);
    m_pTutorRoomMgr->response(charId, rev.status());
  }
  return true;
  case MATCHCPARAM_TEAMPWS_PREPARE_UPDATE:
  {
    PARSE_CMD_PROTOBUF(UpdatePreInfoMatchCCmd, rev);
    userBeReady(rev.etype(), charId);
  }
  return true;
  case MATCHCPARAM_TEAMPWS_QUERY_RANK:
  {
    ScoreManager::getMe().queryPwsRankInfo(zoneId, charId);
  }
  return true;
  case MATCHCPARAM_TEAMPWS_QUERY_TEAMINFO:
  {
    PARSE_CMD_PROTOBUF(QueryTeamPwsTeamInfoMatchCCmd, rev);
    ScoreManager::getMe().queryPwsTeamInfo(zoneId, charId, rev);
  }
  return true;
  }
  return false;
}

bool MatchManager::doServerCmd(QWORD charId, std::string name, DWORD zoneId, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  //MessageStatHelper msgStat(cmd->cmd, cmd->param);
  switch (cmd->param)
  {
 
  }
  return false;
}

void MatchManager::timeTick(DWORD curTime)
{
  if (m_oneSecTimer.timeUp(curTime))
  {
    if (m_smzlRoomMgr)
      m_smzlRoomMgr->oneSecTick(curTime);

    if (m_hljsRoomMgr)
      m_hljsRoomMgr->oneSecTick(curTime);

    if (m_pMvpRoomMgr)
      m_pMvpRoomMgr->oneSecTick(curTime);

    if (m_pSuGvgRoomMgr)
      m_pSuGvgRoomMgr->oneSecTick(curTime);

    if (m_pTeamPwsMatchRoomMgr)
      m_pTeamPwsMatchRoomMgr->oneSecTick(curTime);

    if (m_pTeamPwsRelaxMgr)
      m_pTeamPwsRelaxMgr->oneSecTick(curTime);
  }

  if (m_tenSecTimer.timeUp(curTime))
  {
    if (m_pTutorRoomMgr)
      m_pTutorRoomMgr->timeTick(curTime);
  }
}

MatchRoomMgr* MatchManager::getRoomMgr(Cmd::EPvpType type)
{
  MatchRoomMgr* pRoomMgr = nullptr;
  switch (type)
  {
  case EPVPTYPE_LLH:
    pRoomMgr = m_llhRoomMgr;
    break;
  case EPVPTYPE_SMZL:
    pRoomMgr = m_smzlRoomMgr;
    break;
  case EPVPTYPE_HLJS:
    pRoomMgr = m_hljsRoomMgr;
    break;
  case EPVPTYPE_POLLY:
    pRoomMgr = m_pollyRoomMgr;
    break;
  case EPVPTYPE_MVP:
    pRoomMgr = m_pMvpRoomMgr;
    break;
  case EPVPTYPE_SUGVG:
    pRoomMgr = m_pSuGvgRoomMgr;
    break;
  case EPVPTYPE_TUTOR:
    pRoomMgr = m_pTutorRoomMgr;
    break;
  case EPVPTYPE_TEAMPWS:
    pRoomMgr = m_pTeamPwsMatchRoomMgr;
    break;
  case EPVPTYPE_TEAMPWS_RELAX:
    pRoomMgr = m_pTeamPwsRelaxMgr;
    break;
  default:
    break;
  }
  return pRoomMgr;
}

void MatchManager::onUserOnline(const SocialUser& rUser)
{ 
  MatchUser& rMatchUser = m_mapUser[rUser.charid()];
  rMatchUser.m_qwCharId = rUser.charid();
  rMatchUser.m_dwCurZoneId = rUser.zoneid();
  
  //波利乱斗
  if (isPollyActivityOpen())
  {
    m_pollyRoomMgr->popRetryQueue(rUser.charid());
    NtfMatchInfoCCmd cmd;
    cmd.set_etype(EPVPTYPE_POLLY);
    cmd.set_ismatch(m_pollyRoomMgr->checkIsInQueue(rUser.charid()));
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToClient(rUser.charid(), rUser.zoneid(), send, len);
  }

  if (isMvpBattleOpen())
  {
    // 优先判断当前线报名房间
    MatchRoom* p = getRoomByCharId(rUser.charid(), rUser.zoneid());
    MvpMatchRoom* pMRoom = dynamic_cast<MvpMatchRoom*>(p);
    DWORD joinzone = 0; // 报名时所在线
    if (pMRoom)
    {
      // 当前线匹配中
      if (pMRoom->getState() == EROOMSTATE_WAIT_JOIN)
      {
        NtfMatchInfoCCmd cmd;
        cmd.set_etype(EPVPTYPE_MVP);
        cmd.set_ismatch(true);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToClient(rUser.charid(), rUser.zoneid(), send, len);
      }
      // 当前线匹配成功, 可直接加入
      else if (pMRoom->getState() == EROOMSTATE_FIGHTING)
      {
        NtfMatchInfoCCmd cmd;
        cmd.set_etype(EPVPTYPE_MVP);
        cmd.set_isfight(true);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToClient(rUser.charid(), rUser.zoneid(), send, len);
      }
      joinzone = rUser.zoneid();
    }
    else
    {
      //当前线没报名, 判断是否已在其他线报名
      DWORD lastzoneid = rUser.zoneid();
      auto it = m_mapLastZoneId.find(rUser.charid());
      if (it != m_mapLastZoneId.end())
        lastzoneid = it->second;
      if (lastzoneid != rUser.zoneid())
      {
        p = getRoomByCharId(rUser.charid(), lastzoneid);
        pMRoom = dynamic_cast<MvpMatchRoom*>(p);
        if (pMRoom)
        {
          // 之前的线有报名, 切线后, 在本线取消显示报名状态
          NtfMatchInfoCCmd cmd;
          cmd.set_etype(EPVPTYPE_MVP);
          cmd.set_ismatch(false);
          cmd.set_isfight(false);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToClient(rUser.charid(), rUser.zoneid(), send, len);

          joinzone = lastzoneid;
        }
      }
    }
    // 当前线是副本所在线
    if (pMRoom && pMRoom->getZoneID() == rUser.zoneid())
    {
      pMRoom->onUserOnlinePvp(rUser, joinzone);
    }
  }

  // 退出导师匹配
  TutorMatchResultNtfMatchCCmd cmd;
  cmd.set_status(ETUTORMATCH_STOP);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(rUser.charid(), rUser.zoneid(), send, len);

  MatchRoom* pRoom = nullptr;
  if (thisServer->isInPvpZoneId(rUser.zoneid()))
  {//pvp线
    DWORD &originZoneId = m_mapLastZoneId[rUser.charid()];
    pRoom = getRoomByCharId(rUser.charid(), originZoneId);
    if (!pRoom)
      return;
    pRoom->onUserOnlinePvp(rUser, originZoneId);
    return;
  }
  
  pRoom = getRoomByCharId(rUser.charid(), rUser.zoneid());
  if (!pRoom)
    return;  
  m_mapLastZoneId[rUser.charid()] = rUser.zoneid(); 
  pRoom->onUserOnline(rUser);
  
  pRoom = getRoomByCharId(rUser.charid(), rUser.zoneid());
  if (pRoom)
  {
    ReqMyRoomMatchCCmd reqMyRoom;
    MatchManager::getMe().reqMyRoom(rUser.charid(), rUser.zoneid(), reqMyRoom);
  }
}

void MatchManager::onUserOffline(const SocialUser& rUser)
{
  m_mapUser.erase(rUser.charid());
  GlobalShopMgr::getMe().onClosePanel(rUser.charid());
  //波利乱斗
  if (isPollyActivityOpen())
  {
    if (m_pollyRoomMgr->popMatchQueue(rUser.charid()))
      m_pollyRoomMgr->pushRetryQueue(rUser.charid());
  }

  MatchRoom* pRoom = nullptr;
  if (thisServer->isInPvpZoneId(rUser.zoneid()))
  {//pvp线
    DWORD &originZoneId = m_mapLastZoneId[rUser.charid()];
    pRoom = getRoomByCharId(rUser.charid(), originZoneId);
    if (!pRoom)
      return;
    pRoom->onUserOfflinePvp(rUser, originZoneId);
    return;
  }

  if (m_pTutorRoomMgr != nullptr)
  {
    LeaveRoomCCmd cmd;
    m_pTutorRoomMgr->leaveRoom(rUser.charid(), rUser.zoneid(), cmd);
  }

  pRoom = getRoomByCharId(rUser.charid(), rUser.zoneid());
  if (!pRoom)
    return;

  m_mapLastZoneId[rUser.charid()] = rUser.zoneid();
  pRoom->onUserOffline(rUser);
}


bool MatchManager::reqMyRoom(QWORD charId, DWORD zoneId, ReqMyRoomMatchCCmd& rev)
{
  //MatchRoomMgr* pRoomMgr = getRoomMgr(rev.type());
  //if (pRoomMgr == nullptr)
  //{
  //  XERR << "[斗技场-请求我的房间], 找不到房间管理器" << charId << zoneId << "type" << rev.type() << XEND;
  //  return false;
  //}
  //
  //QWORD roomId = getRoomId(charId, zoneId);
  //MatchRoom* pRoom = pRoomMgr->getRoomByid(roomId);

  MatchRoom* pRoom = getRoomByCharId(charId, zoneId);
  if (pRoom)
  {
    pRoom->toBriefData(rev.mutable_brief_info());
  }
  else
  {
    XLOG << "[斗技场-请求我的房间], 没找到房间" << charId << zoneId << "type" << rev.type()<<"没有房间"<< XEND;
  }  

  PROTOBUF(rev, send, len);
  bool ret = thisServer->sendCmdToClient(charId, zoneId, send, len);
    
  XLOG << "[斗技场-请求我的房间] type " << rev.type() << "charid" << charId << "zoneid" << zoneId << "ret" << ret << "res" << rev.ShortDebugString() << XEND;
  return true;
}

bool MatchManager::reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev)
{
  MatchRoomMgr* pRoomMgr = getRoomMgr(rev.type());
  if (pRoomMgr == nullptr)
  {
    return false;
  }
  pRoomMgr->reqRoomList(charId, zoneId, rev);
  return true;
}

bool MatchManager::reqRoomDetail(QWORD charId, DWORD zoneId, ReqRoomDetailCCmd& rev)
{
  MatchRoom* pRoom = getRoom(rev.type() ,rev.roomid());
  if (pRoom == nullptr)
  {
    return false;
  }
  
  pRoom->toDetailData(rev.mutable_datail_info());
  
  PROTOBUF(rev, send, len);
  thisServer->sendCmdToClient(charId, zoneId, send, len);
  
  XDBG << "[斗技场-请求房间详情] type" << rev.type() << "charid" << charId << "zoneid" << zoneId << "res" << rev.ShortDebugString() << XEND;

  return true;
}

bool MatchManager::joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev)
{ 
  //rev.set_type(EPVPTYPE_POLLY);

  XDBG << "[斗技场-加入房间消息] charid" << charId << "zoneid" << zoneId <<"roomid"<<rev.roomid()<<"teamid"<<rev.teamid()<< rev.ShortDebugString() << XEND;   

  if (chekIsOpen(charId, zoneId, rev.type()) == false)
    return false;

  MatchRoomMgr* pRoomMgr = getRoomMgr(rev.type());

  if (pRoomMgr == nullptr)
    return false;

  if (rev.isquick())
  {
    QWORD guid = pRoomMgr->getAvailableRoom();
    rev.set_roomid(guid);
    XLOG << "[斗技场-加入房间]找到快速加入的房间 " << charId << "zoneid" << zoneId << "type" << rev.type() << "roomid" << rev.roomid() << XEND;
  }

  bool ret = true;
  do 
  {
    if (rev.type() == EPVPTYPE_POLLY)
    {      
      ret = pRoomMgr->joinRoom(charId, zoneId, rev);
      break;
    }
    else if (rev.type() == EPVPTYPE_MVP)
    {
      ret = pRoomMgr->joinRoom(charId, zoneId, rev);
      break;
    }
    else if (rev.type() == EPVPTYPE_SUGVG)
    {
      ret = pRoomMgr->joinRoom(charId, zoneId, rev);
      break;
    }
    else if (rev.type() == EPVPTYPE_TUTOR)
    {
      ret = pRoomMgr->joinRoom(charId, zoneId, rev);
      break;
    }
    else if (rev.type() == EPVPTYPE_TEAMPWS)
    {
      ret = pRoomMgr->joinRoom(charId, zoneId, rev);
      break;
    }
    else if (rev.type() == EPVPTYPE_TEAMPWS_RELAX)
    {
      ret = pRoomMgr->joinRoom(charId, zoneId, rev);
      break;
    }

    if (rev.roomid() == 0)
    {
      if (rev.type() == EPVPTYPE_LLH || rev.type() == EPVPTYPE_HLJS)
      {
        //不可创建房间
        XERR << "[斗技场-加入房间] 失败,不可创建房间，roomid 为0" << charId << zoneId << "type" << rev.type() << "roomid" << rev.roomid() << XEND;
        ret = false;
        break;
      }

      if (pRoomMgr->createRoom(charId, zoneId, rev) == nullptr)
      {
        XERR << "[斗技场-创建房间] 失败" << charId << zoneId << "type" << rev.type() << "roomid" << rev.roomid() << XEND;
        ret = false;
        break;
      }
      ret = true;
      break;
    }
    else if (pRoomMgr->joinRoom(charId, zoneId, rev) == false)
    {
      XERR << "[斗技场-加入房间] 失败" << charId << zoneId << "type" << rev.type() << "roomid" << rev.roomid() << XEND;
      ret = false;
      break;
    }

  } while (0);

  //join res
  rev.clear_teammember();
  rev.set_ret(ret);
  PROTOBUF(rev, send, len);
  thisServer->sendCmdToClient(charId, zoneId, send, len);
  return ret;
}

bool MatchManager::leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev)
{

  MatchRoomMgr* pRoomMgr = getRoomMgr(rev.type());
  if (pRoomMgr == nullptr)
  {
    return false;
  }

  pRoomMgr->leaveRoom(charId, zoneId, rev);

  XLOG << "[斗技场-玩家主动取消报名] charId" << charId << "type" << rev.type() << "roomid" << rev.roomid() << XEND;
  return true;
}

bool MatchManager::joinFighting(QWORD charId, DWORD zoneId, JoinFightingCCmd& rev)
{ 
  if (chekIsOpen(charId, zoneId, rev.type()) == false)
    return false;

  MatchRoomMgr* pRoomMgr = getRoomMgr(rev.type());
  if (pRoomMgr == nullptr)
  {
    return false;
  }

  pRoomMgr->joinFighting(charId, zoneId, rev);

  return true;
}

//玩家从场景离开
bool MatchManager::onLeaveRoom(QWORD charId,DWORD originZoneId)
{
  return true;
}

MatchRoom* MatchManager::getRoom(Cmd::EPvpType type, QWORD roomId)
{
  switch (type)
  {
  case EPVPTYPE_LLH:
    return m_llhRoomMgr->getRoomByid(roomId);
  case EPVPTYPE_SMZL:
    return m_smzlRoomMgr->getRoomByid(roomId);
  case EPVPTYPE_HLJS:
    return m_hljsRoomMgr->getRoomByid(roomId);
  default:
    break;
  }
  return nullptr;
}

QWORD MatchManager::generateGuid()
{
  if (m_qwGuidIndex == 0)
  {
    //m_qwGuidIndex = now();
    m_qwGuidIndex = 1;
  }
  else
    m_qwGuidIndex++;
  return m_qwGuidIndex;
}

bool MatchManager::sendPlayerToFighting(DWORD fromZoneid, DWORD toZoneId, QWORD charId, DWORD mapId, QWORD roomId, DWORD colorIndex/*=0*/)
{
  if (toZoneId == 0)
  {
    return false;
  }

  EnterPvpMapSCmdMatch cmd;
  cmd.set_charid(charId);
  cmd.set_dest_zoneid(toZoneId);
  cmd.set_raidid(mapId);
  cmd.set_room_guid(roomId);
  cmd.set_colorindex(colorIndex);

  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmdToZone(fromZoneid, send, len);
  XLOG << "[斗技场-传送玩家进副本] fromzoneid" << fromZoneid << "tozoneid" << toZoneId << "charid" << charId << "raidid" << mapId << "roomid" << roomId << "ret" << ret<<"index"<<colorIndex << XEND;
  return ret;
}

MatchRoom* MatchManager::getRoomByRoomId(QWORD roomId)
{
  MatchRoom* pRoom = m_llhRoomMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;
  pRoom = m_smzlRoomMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;

  pRoom = m_hljsRoomMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;

  pRoom = m_pollyRoomMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;

  pRoom = m_pMvpRoomMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;
    
  pRoom = m_pSuGvgRoomMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;

  pRoom = m_pTeamPwsMatchRoomMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;

  pRoom = m_pTeamPwsRelaxMgr->getRoomByid(roomId);
  if (pRoom)
    return pRoom;
    
  return pRoom;
}

QWORD MatchManager::getRoomId(QWORD charId, DWORD zoneId)
{
  auto it = m_mapUserRoom.find(charId);
  if (it == m_mapUserRoom.end())
    return 0;
  
  auto subIt = it->second.find(zoneId);
  if (subIt == it->second.end())
    return 0;
  return subIt->second;
}

MatchRoom* MatchManager::getRoomByCharId(QWORD charId, DWORD zoneId)
{
  QWORD roomId = getRoomId(charId, zoneId);

  return getRoomByRoomId(roomId);
}

void MatchManager::addRoomUser(QWORD charId, DWORD zoneId, QWORD roomId)
{
  std::map<DWORD, QWORD>& tempMap = m_mapUserRoom[charId];
  tempMap[zoneId] = roomId;
}

void MatchManager::delRoomUser(QWORD charId, DWORD zoneId, QWORD roomId)
{
  auto it = m_mapUserRoom.find(charId);
  if (it == m_mapUserRoom.end())
    return;
  it->second.erase(zoneId);
  if (it->second.empty())
    m_mapUserRoom.erase(charId);
}

bool MatchManager::isInRoom(QWORD charId, DWORD zoneId)
{
  return getRoomId(charId, zoneId);
}


void MatchManager::sendSysMsg(QWORD charId, DWORD zoneId, DWORD msgid, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/)
{
  if (charId == 0)
    return;

  SysMsg cmd;
  cmd.set_id(msgid);
  cmd.set_type(eType);
  cmd.set_act(eAct);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(charId, zoneId, send, len);
}

bool MatchManager::chekIsOpen(QWORD charId, DWORD zoneId, Cmd::EPvpType eType)
{
  if (eType == EPVPTYPE_LLH)
  {
    if (CommonConfig::getMe().m_bPvpLLH == false)
    {
      sendSysMsg(charId, zoneId, 980);
      XLOG << "[斗技场] 溜溜猴 功能关闭" << XEND;
      return false;
    }
  }
  else if (eType == EPVPTYPE_SMZL)
  {
    if (CommonConfig::getMe().m_bPvpSMZL == false)
    {
      sendSysMsg(charId, zoneId, 981);
      XLOG << "[斗技场] 沙漠之狼 功能关闭" << XEND;
      return false;
    }
  }
  else if (eType == EPVPTYPE_HLJS)
  {
    if (CommonConfig::getMe().m_bPvpHLJS == false)
    {
      sendSysMsg(charId, zoneId, 982);
      XLOG << "[斗技场] 华丽金属 功能关闭" << XEND;
      return false;
    }
  }
  else if (eType == EPVPTYPE_MVP)
  {
    return true;
  }
  else if (eType == EPVPTYPE_TEAMPWS)
  {
    return true;
  }
  else if (eType == EPVPTYPE_TEAMPWS_RELAX)
  {
    return true;
  }

  return true;
}

bool MatchManager::gmRestUserRoom(QWORD charId, DWORD zoneId)
{
  auto it = m_mapUserRoom.find(charId);
  if (it == m_mapUserRoom.end())
    return true;
  
  
  std::set<QWORD> setRoomId;
  for (auto & m : it->second)
  {
    setRoomId.insert(m.second);   
  }

  for (auto &s : setRoomId)
  {
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(s);
    if (!pRoom)
      return false;
    TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pRoom);
    if (pTeamRoom)
    {
      pTeamRoom->getMatchRoomMgr()->closeRoom(pTeamRoom);
      XLOG << "[斗技场-GM] 清除玩家房间charid" << charId << "roomid" << s << XEND;
    }
  }
 
  return true;
}

void MatchManager::openPollyActivity()
{
  if (m_bPollyActivity)
    return;
  m_bPollyActivity = true;

}

void MatchManager::closePollyActivity()
{
  if (m_bPollyActivity == false)
    return;  
  m_bPollyActivity = false;

  //clear 
  m_pollyRoomMgr->clearQueue();
}

void MatchManager::openMvpBattle()
{
  if (m_bMvpBattleOpen)
    return;
  m_bMvpBattleOpen = true;
  XLOG << "[Mvp-竞争战], 匹配服开启战斗" << XEND;
}

void MatchManager::closeMvpBattle()
{
  if (!m_bMvpBattleOpen)
    return;
  m_bMvpBattleOpen = false;
  XLOG << "[Mvp-竞争战], 匹配服关闭战斗" << XEND;

  m_pMvpRoomMgr->onMvpBattleClose();
}

void MatchManager::joinSuperGvg(const JoinSuperGvgMatchSCmd& rev)
{
  if (m_pSuGvgRoomMgr == nullptr)
    return;
  m_pSuGvgRoomMgr->joinGuild(rev);
}

void MatchManager::openTeamPws(bool bRestart /*=false*/)
{
  if (m_bTeamPwsOpen)
    return;
  m_bTeamPwsOpen = true;
  m_pTeamPwsMatchRoomMgr->onBattleOpen();

  if (!bRestart)
    ScoreManager::getMe().addPwsCount();
  XLOG << "[组队排位赛], 匹配服开启战斗" << XEND;
}

void MatchManager::closeTeamPws()
{
  if (!m_bTeamPwsOpen)
    return;
  m_bTeamPwsOpen = false;
  XLOG << "[组队排位赛], 匹配服关闭战斗" << XEND;
}

void MatchManager::joinTeamPws(JoinTeamPwsMatchSCmd& rev)
{
  auto p = getTeamPwsRoomMgr(rev.etype());
  if (!p)
    return;
  p->teamJoin(rev);
}

void MatchManager::leaveTeamPws(DWORD zoneid, QWORD teamid, EPvpType etype)
{
  auto p = getTeamPwsRoomMgr(etype);
  if (!p)
    return;
  p->teamLeave(zoneid, teamid);
}

void MatchManager::userLeaveRaid(QWORD charid, EPvpType etype)
{
  if (etype == EPVPTYPE_TEAMPWS && m_pTeamPwsMatchRoomMgr)
    m_pTeamPwsMatchRoomMgr->punishUserLeaveTeam(charid);
}

TeamPwsMatchRoomMgr* MatchManager::getTeamPwsRoomMgr(EPvpType etype)
{
  if (etype == EPVPTYPE_TEAMPWS)
    return m_pTeamPwsMatchRoomMgr;
  if (etype == EPVPTYPE_TEAMPWS_RELAX)
    return m_pTeamPwsRelaxMgr;
  return nullptr;
}

void MatchManager::userBeReady(EPvpType etype, QWORD charid)
{
  switch(etype)
  {
    case EPVPTYPE_TEAMPWS:
    case EPVPTYPE_TEAMPWS_RELAX:
      {
        auto p = getTeamPwsRoomMgr(etype);
        if (!p)
          break;
        p->userBeReady(charid);
      }
      break;
    default:
      break;
  }
}

bool MatchManager::isInMatching(QWORD charid)
{
  if (m_pTeamPwsMatchRoomMgr->isInMatching(charid))
    return true;
  if (m_pTeamPwsRelaxMgr->isInMatching(charid))
    return true;

  return false;
}

