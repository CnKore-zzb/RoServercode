#include <iostream>
#include "MatchServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RegCmd.h"
#include "MatchSCmd.pb.h"
#include "MatchManager.h"
#include "CommonConfig.h"
#include "SocialCmd.pb.h"
#include "MatchCCmd.pb.h"
#include "GZoneCmd.pb.h"
#include "GlobalShopMgr.h"
#include "BoothManager.h"
#include "MatchGMTest.h"
#include "ScoreManager.h"

bool MatchServer::doSocialCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
  case SOCIALPARAM_ONLINESTATUS:
  {
     PARSE_CMD_PROTOBUF(OnlineStatusSocialCmd, rev);
     if (rev.online())
     {
       MatchManager::getMe().onUserOnline(rev.user());
       ScoreManager::getMe().onUserOnline(rev.user().zoneid(), rev.user().charid());
     }
     else
     {
       MatchManager::getMe().onUserOffline(rev.user());
     }
  }
  return true;
  default:
    return false;
  }

  return false;
}

bool MatchServer::doMatchCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;

  XDBG << "[斗技场-收到消息] " << cmd->cmd << cmd->param << XEND;

  switch (cmd->param)
  {
  case MATCHSPARAM_REG_PVP_ZONE:
  {
    PARSE_CMD_PROTOBUF(Cmd::RegPvpZoneMatch, rev);
    bool isPvpZone = false;
    if (isZoneCategory(rev.category(), ZoneCategory_PVP_LLH))
    {
      isPvpZone = true;
      m_dwLLHZoneid = rev.zoneid();
      XLOG << "[斗技场-线注册] 溜溜猴" << rev.zoneid() << XEND;
    }

    if (isZoneCategory(rev.category(), ZoneCategory_PVP_SMZL))
    {
      isPvpZone = true;
      m_dwSMZLZoneid = rev.zoneid();
      XLOG << "[斗技场-线注册] 沙漠之狼" << rev.zoneid() << XEND;
    }

    if (isZoneCategory(rev.category(), ZoneCategory_PVP_HLSJ))
    {
      isPvpZone = true;
      m_dwHLJSZoneid = rev.zoneid();
      XLOG << "[斗技场-线注册] 华丽金属" << rev.zoneid() << XEND;
    }  
    if (!isPvpZone)
    {
      thisServer->addZoneInfo(rev.zoneid());
      XLOG << "[斗技场-线注册] 非pvp线注册" << rev.zoneid() << XEND;
    }
  }
  break;

  case MATCHSPARAM_SESSION_FORWARD_CCMD_MATCH:
  {                         
    PARSE_CMD_PROTOBUF(Cmd::SessionForwardCCmdMatch, rev);
    return MatchManager::getMe().doUserCmd(rev.charid(), rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len());
  }
  case MATCHSPARAM_SESSION_FORWARD_SCMD_MATCH:
  {
    PARSE_CMD_PROTOBUF(Cmd::SessionForwardSCmdMatch, rev);
    return MatchManager::getMe().doServerCmd(rev.charid(), rev.name(), rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len()); 
  }
  case MATCHSPARAM_PVP_MEMBERUPDATE:
  {
    //
    PARSE_CMD_PROTOBUF(Cmd::PvpTeamMemberUpdateSCmd, rev);
    XDBG << "[斗技场-收到teamserver队伍人数同步]" << rev.ShortDebugString() << XEND;
    const MatchTeamMemUpdateInfo& rInfo = rev.data();
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(rInfo.roomid());
    TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pRoom);
    if (pTeamRoom == nullptr)
    {
      XERR << "[斗技场-收到teamserver同步],找不到房间 roomid" << rInfo.roomid() << "teamid" << rInfo.teamid() << XEND;
      return false;
    }
    pTeamRoom->updateTeamMem(rInfo);
    return true;
  }
  break;
  case MATCHSPARAM_PVP_MEMBERDATAUPDATE:
  {
    //
    PARSE_CMD_PROTOBUF(Cmd::PvpMemberDataUpdateSCmd, rev);
    XDBG << "[斗技场-收到teamserver队员数据同步]" << rev.ShortDebugString() << XEND;
    const MatchTeamMemDataUpdateInfo& rInfo = rev.data();
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(rInfo.roomid());
    TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pRoom);
    if (pTeamRoom == nullptr)
    {
      XERR << "[斗技场-收到teamserver队员数据同步],找不到房间 roomid" << rInfo.roomid() << "teamid" << rInfo.teamid() << XEND;
      return false;
    }
    pTeamRoom->updateTeamMemData(rInfo);
    return true;
  }
  break;
  case MATCHSPARAM_LEAVE_PVP_MAP:             //场景玩家离开场景
  {
    PARSE_CMD_PROTOBUF(Cmd::LeavePvpMap, rev);
    return MatchManager::getMe().onLeaveRoom(rev.charid(), rev.originzoneid());
  }
  break;
  case MATCHSPARAM_CREATE_TEAM:
  {
    PARSE_CMD_PROTOBUF(Cmd::CreateTeamMatchSCmd, rev);
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(rev.roomid());
   
    TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pRoom);
    if (pTeamRoom)
    {
      return pTeamRoom->createNewTeamRes(rev.zoneid(), rev.teamid(), rev.charid(), rev.new_teamid());
    }      
  }
  break;
  case MATCHSPARAM_SYNC_RAIDSCENE:
  {
    PARSE_CMD_PROTOBUF(Cmd::SyncRaidSceneMatchSCmd, rev);
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(rev.roomid());
    if (!pRoom)
      return false;
    
    if (pRoom->getType() == EPVPTYPE_LLH)
    {
      LLHMatchRoom* pLLHRoom = dynamic_cast<LLHMatchRoom*>(pRoom);
      if (pLLHRoom)
      {
        pLLHRoom->updateUserCount(rev.count());
      }
    }
    else if (pRoom->getType() == EPVPTYPE_SMZL || pRoom->getType() == EPVPTYPE_HLJS)
    {
      TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pRoom);
      if (pTeamRoom && rev.open() == false)
      {
        pTeamRoom->getMatchRoomMgr()->closeRoom(pTeamRoom);
      }
    }
    else if (pRoom->getType() == EPVPTYPE_POLLY)
    {
      //PollyZoneInfo* pZoneInfo = thisServer->getZoneInfo(rev.zoneid());
      //if (pZoneInfo && rev.open() == false)
      //  pZoneInfo->updateCount(false);
      pRoom->getMatchRoomMgr()->closeRoom(pRoom);
    }
    else if (pRoom->getType() == EPVPTYPE_MVP)
    {
      if (rev.open() == false)
      {
        TeamMatchRoom* pTeamRoom = dynamic_cast<TeamMatchRoom*>(pRoom);
        pRoom->getMatchRoomMgr()->closeRoom(pTeamRoom);
      }
      else
        pRoom->onRaidSceneOpen(rev.zoneid(), rev.sceneid());
    }
    else if (pRoom->getType() == EPVPTYPE_SUGVG)
    {
      if (rev.open() == false)
        pRoom->getMatchRoomMgr()->closeRoom(pRoom);
      else
        pRoom->getMatchRoomMgr()->onRaidSceneOpen(pRoom, rev.zoneid(), rev.sceneid());
    }
    else if (pRoom->getType() == EPVPTYPE_TEAMPWS || pRoom->getType() == EPVPTYPE_TEAMPWS_RELAX)
    {
      if (rev.open() == false)
        pRoom->getMatchRoomMgr()->closeRoom(pRoom);
      else
        pRoom->getMatchRoomMgr()->onRaidSceneOpen(pRoom, rev.zoneid(), rev.sceneid());
    }
    return true;
  }
  break;
  case MATCHSPARAM_KICK_TEAM: //pvp线踢出玩家，外面的线让他进去
  {
    PARSE_CMD_PROTOBUF(Cmd::KickTeamMatchSCmd, rev);
    TeamMatchRoom* pRoom = dynamic_cast<TeamMatchRoom*>(MatchManager::getMe().getRoomByRoomId(rev.roomid()));
    if (!pRoom)
      return false;
    pRoom->kickOldTeamUser(rev);
  }
  break;
  case MATCHSPARAM_KICK_USER:
  {
    PARSE_CMD_PROTOBUF(Cmd::KickUserFromPvpMatchSCmd, rev);
    
    bool ret = MatchManager::getMe().gmRestUserRoom(rev.charid(), rev.zoneid());
    
    XLOG << "[斗技场-GM] 清除玩家房间 charid" << rev.charid() << "zoneid" << rev.zoneid() << "ret" << ret << XEND;
  }
  break;
  case MATCHSPARAM_POLLY_ACTIVITY:
  {
    PARSE_CMD_PROTOBUF(Cmd::ActivityMatchSCmd, rev);
    if (rev.etype() == EPVPTYPE_POLLY)
    {
      if (rev.open())
        MatchManager::getMe().openPollyActivity();
      else
        MatchManager::getMe().closePollyActivity();

      XLOG << "[斗技场-波利乱斗] 波利乱斗活动消息" << rev.open() << XEND;
    }
    else if (rev.etype() == EPVPTYPE_MVP)
    {
      if (rev.open())
        MatchManager::getMe().openMvpBattle();
      else
        MatchManager::getMe().closeMvpBattle();
      XLOG << "[Mvp-竞争战], 开启状态:" << rev.open() << XEND;
    }
    else if (rev.etype() == EPVPTYPE_TEAMPWS)
    {
      if (rev.open())
        MatchManager::getMe().openTeamPws(rev.server_restart());
      else
        MatchManager::getMe().closeTeamPws();
      XLOG << "[组队排位赛], 开启状态:" << rev.open() << XEND;
    }
  }
  break;
  case MATCHSPARAM_QUERY_SOLD_CNT:
  {
    PARSE_CMD_PROTOBUF(Cmd::QuerySoldCntMatchSCmd, rev);
    GlobalShopMgr::getMe().querySoldCnt(rev);
  }
  break;
  case MATCHSPARAM_CHECK_CAN_BUY:
  {
    PARSE_CMD_PROTOBUF(Cmd::CheckCanBuyMatchSCmd, rev);
    GlobalShopMgr::getMe().checkSoldCnt(rev);
  }
  break;
  case MATCHSPARAM_ADD_BUY_CNT:
  {
    PARSE_CMD_PROTOBUF(Cmd::AddBuyCntMatchSCmd, rev);
    GlobalShopMgr::getMe().addSoldCnt(rev);
  }
  break;
  case MATCHSPARAM_SUPERGVG_JOIN:
  {
    PARSE_CMD_PROTOBUF(JoinSuperGvgMatchSCmd, rev);
    MatchManager::getMe().joinSuperGvg(rev);
  }
  break;
  case MATCHSPARAM_CLEAR_MVPCD:
  {
    PARSE_CMD_PROTOBUF(ClearMvpCDMatchSCmd, rev);
    MatchRoom* pRoom = MatchManager::getMe().getRoomByRoomId(rev.roomid());
    MvpMatchRoom* pMRoom = dynamic_cast<MvpMatchRoom*>(pRoom);
    if (pMRoom)
      pMRoom->clearTeamPunishCD(rev.teamid());
  }
  break;
  case MATCHSPARAM_TUTOR_OPT:
  {
    PARSE_CMD_PROTOBUF(TutorOptMatchSCmd, rev);
    TutorMatchRoomMgr* pMgr = dynamic_cast<TutorMatchRoomMgr*>(MatchManager::getMe().getRoomMgr(EPVPTYPE_TUTOR));
    if (pMgr != nullptr)
      pMgr->process(rev.tutorid(), rev.result());
  }
  break;
  case MATCHSPARAM_TUTOR_BLACK_UPDATE:
  {
    PARSE_CMD_PROTOBUF(TutorBlackUpdateMatchSCmd, rev);
    TutorMatchRoomMgr* pMgr = dynamic_cast<TutorMatchRoomMgr*>(MatchManager::getMe().getRoomMgr(EPVPTYPE_TUTOR));
    if (pMgr == nullptr)
    {
      XERR << "[导师匹配-匹配信息] charid :" << rev.charid() << "更新黑名单信息失败,未找到匹配房间管理" << XEND;
      break;
    }
    STutorMatcher* pMatcher = pMgr->getMatcher(rev.charid());
    if (pMatcher == nullptr)
    {
      XDBG << "[导师匹配-匹配信息] charid :" << rev.charid() << "更新黑名单信息失败,未在匹配中" << XEND;
      break;
    }
    pMatcher->setBlackIDs.clear();
    for (int i = 0; i < rev.blackids_size(); ++i)
      pMatcher->setBlackIDs.insert(rev.blackids(i));
    XLOG << "[导师匹配-匹配信息] charid :" << rev.charid() << "更新黑名单信息成功,黑名单列表" << pMatcher->setBlackIDs << XEND;
  }
  break;
  case MATCHSPARAM_USER_BOOTH_REQ:
  {
    PARSE_CMD_PROTOBUF(UserBoothReqMatchSCmd, rev);

    if(EBOOTHOPER_OPEN == rev.oper())
    {
      if(!BoothManager::getMe().addMapUser(rev.zoneid(), rev.sceneid(), rev.user()))
      {
        XERR << "[摆摊同步-开启]开启失败！" << rev.ShortDebugString() << XEND;
        break;
      }
    }
    else if(EBOOTHOPER_CLOSE == rev.oper())
    {
      if(!BoothManager::getMe().delMapUser(rev.user().guid()))
      {
        XERR << "[摆摊同步-关闭]关闭失败！" << rev.ShortDebugString() << XEND;
        break;
      }
    }
    else if(EBOOTHOPER_UPDATE == rev.oper())
    {
      if(!BoothManager::getMe().updateMapUser(rev.user()))
      {
        XERR << "[摆摊同步-更新]更新失败！" << rev.ShortDebugString() << XEND;
        break;
      }
    }

    UserBoothNTFMatchSCmd cmd;
    cmd.set_zoneid(rev.zoneid());
    cmd.set_sceneid(rev.sceneid());
    cmd.set_oper(rev.oper());
    cmd.mutable_user()->CopyFrom(rev.user());

    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToAllZone(send, len, rev.zoneid());
    XLOG << "[摆摊同步]摆摊信息同步至所有服，除了zone:" << rev.zoneid() << rev.ShortDebugString() << XEND;
  }
  break;
  case MATCHSPARAM_SCENE_GM_TEST:
  {
#ifdef _DEBUG
    PARSE_CMD_PROTOBUF(SceneGMTestMatchSCmd, rev);
    MatchGMTest::getMe().gmTest(rev);
#endif
  }
  break;
  case MATCHSPARAM_UPDATE_SCORE:
  {
    // scene->session->match 更新积分
    PARSE_CMD_PROTOBUF(UpdateScoreMatchSCmd, rev);
    ScoreManager::getMe().updateScore(rev);
  }
  break;
  case MATCHSPARAM_JOIN_TEAMPWS:
  {
    PARSE_CMD_PROTOBUF(JoinTeamPwsMatchSCmd, rev);
    MatchManager::getMe().joinTeamPws(rev);
  }
  break;
  case MATCHSPARAM_LEAVE_TEAMPWS:
  {
    PARSE_CMD_PROTOBUF(ExitTeamPwsMatchSCmd, rev);
    MatchManager::getMe().leaveTeamPws(rev.zoneid(), rev.teamid(), rev.etype());
  }
  break;
  case MATCHSPARAM_USER_FORCE_LEAVE:
  {
    PARSE_CMD_PROTOBUF(UserLeaveRaidMatchSCmd, rev);
    MatchManager::getMe().userLeaveRaid(rev.charid(), rev.etype());
  }
  break;
  default:
    return false;
  }
  return true;
}

bool MatchServer::doGZoneCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
  case GZONEPARAM_UPDATEA_ACTIVE_ONLINE:
  {
    PARSE_CMD_PROTOBUF(UpdateActiveOnlineGZoneCmd, rev);
    
    PollyZoneInfo* pZoneInfo = getZoneInfo(rev.zoneid());
    if (!pZoneInfo)
      return true;
    if (pZoneInfo->dwUserCount == rev.online())
      return true;
    pZoneInfo->setUserCount(rev.online());
    n_bNeedSort = true;
    return true;
  }
  break;
  default:
    return false;
  }

  return true;
}
