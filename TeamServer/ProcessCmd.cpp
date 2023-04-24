#include "TeamServer.h"
#include "SocialCmd.pb.h"
#include "MiscConfig.h"
#include "StatisticsDefine.h"
#include "MailManager.h"
#include "PlatLogManager.h"
#include "Team.h"
#include "TeamManager.h"
#include "UserCmd.h"
#include "TeamCmd.pb.h"
#include "GCharManager.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "GTeam.h"
#include "MatchSCmd.pb.h"

extern xLog srvLog;

bool TeamServer::doSocialCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case SOCIALPARAM_DEL_TEAM_RAID:
      {
        PARSE_CMD_PROTOBUF(DelTeamRaidSocialCmd, rev);
        Team *pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam)
        {
          pTeam->delRaidZoneID(rev.raidid());
        }
        return true;
      }
      break;
    case SOCIALPARAM_GO_TEAM_RAID:
      {
        PARSE_CMD_PROTOBUF(GoTeamRaidSocialCmd, rev);
        Team *pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam)
        {
          DWORD zoneid = pTeam->getRaidZoneID(rev.raidid());
          if (zoneid)
          {
            rev.set_raidzoneid(zoneid);
            PROTOBUF(rev, send, len2);
            sendCmdToSession(send, len2);
            return true;
          }
          else
          {
            pTeam->setRaidZoneID(rev.raidid(), rev.myzoneid());
            rev.set_raidzoneid(rev.myzoneid());
            PROTOBUF(rev, send, len2);
            sendCmdToSession(send, len2);
          }
        }
        return true;
      }
      break;
    case SOCIALPARAM_SESSION_FORWARD_SOCIAL_CMD:
      {
        PARSE_CMD_PROTOBUF(SessionForwardSocialCmd, rev);
        if (rev.type() == ECMDTYPE_TEAM)
          TeamManager::getMe().doTeamCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        else if (rev.type() == ECMDTYPE_DOJO)
          TeamManager::getMe().doDojoCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        else if (rev.type() == ECMDTYPE_TOWER)
          TeamManager::getMe().doTowerCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        else if (rev.type() == ECMDTYPE_CHAT)
        {
          doChatCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        }
        else if (rev.type() == ECMDTYPE_PVECARD)
          TeamManager::getMe().doPveCardCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        else if (rev.type() == ECMDTYPE_TEAMRAID)
          TeamManager::getMe().doTeamRaidCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
      }
      return true;
    case SOCIALPARAM_ONLINESTATUS:
      {
        PARSE_CMD_PROTOBUF(OnlineStatusSocialCmd, rev);
        if (rev.online())
        {
          TeamManager::getMe().onUserOnline(rev.user());
        }
        else
        {
          TeamManager::getMe().onUserOffline(rev.user());
        }
      }
      return true;
    case SOCIALPARAM_USER_SYNC_INFO:
      {
        PARSE_CMD_PROTOBUF(UserInfoSyncSocialCmd, rev);
        TeamManager::getMe().updateUserInfo(rev);
      }
      return true;
    case SOCIALPARAM_USER_DEL_CHAR:
      {
        PARSE_CMD_PROTOBUF(UserDelSocialCmd, rev);
        TeamManager::getMe().delChar(rev.charid());
      }
      return true;
    case SOCIALPARAM_TEAM_CREATE:
      {
        PARSE_CMD_PROTOBUF(TeamCreateSocialCmd, rev);
        TeamManager::getMe().createOneTeam(rev.team().name(), rev.user(), rev.team().type(), rev.team().minlv(), rev.team().maxlv(), rev.team().autoaccept());
      }
      return true;
    case SOCIALPARAM_TEAM_INVITE:
      {
        PARSE_CMD_PROTOBUF(TeamInviteSocialCmd, rev);
        TeamManager::getMe().inviteMember(rev.invite(), rev.beinvite());
      }
      return true;
    case SOCIALPARAM_TEAM_PROCESSINVITE:
      {
        PARSE_CMD_PROTOBUF(TeamProcessInviteSocialCmd, rev);
        TeamManager::getMe().processInviteMember(rev.type(), rev.user(), rev.leaderid());
      }
      return true;
    case SOCIALPARAM_TEAM_APPLY:
      {
        PARSE_CMD_PROTOBUF(TeamApplySocialCmd, rev);
        TeamManager::getMe().applyTeam(rev.apply(), rev.teamid());

        return true;
      }
      break;
    case SOCIALPARAM_TEAM_QUICKENTER:
      {
        PARSE_CMD_PROTOBUF(TeamQuickEnterSocialCmd, rev);
        TeamManager::getMe().quickEnter(rev.user(), rev.type(), rev.set());

        return true;
      }
      break;
    case SOCIALPARAM_DOJO_STATE_NTF:
      {
        PARSE_CMD_PROTOBUF(DojoStateNtfSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (!pTeam)
        {
          return false;
        }
        if (rev.state() == EDOJOSTATE_OPEN)
          return pTeam->setDojoOpen(rev.guildid());
        else if (rev.state() == EDOJOSTATE_CLOSE)
          return pTeam->setDojoClose(rev.guildid());
        return false;
      }
      return true;
    case SOCIALPARAM_TOWER_SYNC_LEADERINFO:
      {
        PARSE_CMD_PROTOBUF(TowerLeaderInfoSyncSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.user().charid());
        if (pTeam != nullptr)
          pTeam->setLeaderTowerInfo(rev.info(), rev.user().charid());

        return true;
      }
      break;
    case SOCIALPARAM_TOWER_SYNC_SCENEINFO:
      {
        PARSE_CMD_PROTOBUF(TowerSceneSyncSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam != nullptr)
        {
          if (rev.state() == EDOJOSTATE_OPEN)
            pTeam->setTowerSceneOpen(rev.raidid());
          else if (rev.state() == EDOJOSTATE_CLOSE)
            pTeam->setTowerSceneClose(rev.raidid());
        }

        return true;
      }
      break;
    case SOCIALPARAM_TOWER_SYNC_LAYER:
      {
        PARSE_CMD_PROTOBUF(TowerLayerSyncSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam != nullptr)
          pTeam->setCurTowerLayer(rev.layer());

        return true;
      }
      break;
    case SOCIALPARAM_TEAM_SEAL_FIHISH:
      {
        PARSE_CMD_PROTOBUF(LeaderSealFinishSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam)
          pTeam->onLeaderFinishSeal();

        return true;
      }
      break;
    case SOCIALPARAM_SOCIAL_LIST_UPDATE:
      {
        PARSE_CMD_PROTOBUF(SocialListUpdateSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
        if (pTeam != nullptr)
        {
          TMember* pMember = pTeam->getMember(rev.charid());
          if (pMember != nullptr)
            pMember->updateSocialData(rev);
        }
      }
      break;
    case SOCIALPARAM_TEAM_QUEST_UPDATE:
      {
        PARSE_CMD_PROTOBUF(TeamerQuestUpdateSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.quest().charid());
        if (pTeam)
        {
          pTeam->updateMemberQuest(rev.quest());
        }
      }
      break;
    case SOCIALPARAM_PVECARD_SCENEINFO:
      {
        PARSE_CMD_PROTOBUF(CardSceneSyncSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam == nullptr)
          break;
        if (rev.open())
          pTeam->onPveCardRaidOpen();
        else
          pTeam->onPveCardRaidClose();
      }
      break;
    case SOCIALPARAM_SYNC_TEAMRAID:
      {
        PARSE_CMD_PROTOBUF(TeamRaidSceneSyncSocialCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam == nullptr)
          break;
        if (rev.open())
          pTeam->onTeamRaidOpen(static_cast<ERaidType>(rev.raid_type()));
        else
          pTeam->onTeamRaidClose(static_cast<ERaidType>(rev.raid_type()));
      }
      break;
    default:
      return false;
  }

  return false;
}

bool TeamServer::doTeamCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case SERVERTEAMPARAM_FORWARD_ALL_CMD:
      {
        PARSE_CMD_PROTOBUF(ForwardAllServerTeamCmd, rev);

        Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
        if (pTeam != nullptr)
        {
          pTeam->broadcastCmd((const BYTE *)rev.data().c_str(), rev.len());
        }

        return true;
      }
      break;
    case SERVERTEAMPARAM_UPDATE_GUILD:
      {
        PARSE_CMD_PROTOBUF(UpdateGuildServerTeamCmd, rev);

        Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
        if (pTeam != nullptr)
        {
          TMember* pMember = pTeam->getMember(rev.charid());
          if (pMember != nullptr)
          {
            pMember->setGuildID(rev.guildid());
            pMember->setGuildName(rev.guildname());
          }
        }
        return true;
      }
      break;
    case SERVERTEAMPARAM_LOADLUA:
      {
        PARSE_CMD_PROTOBUF(LoadLuaTeamCmd, message);
        bool bNtf = false;
        if (message.has_lua() == true)
        {
          LuaManager::getMe().reload();
          bNtf = true;
        }
        if (message.has_log() == true)
        {
          srvLog.reload();
          bNtf = true;
        }
        if (message.has_table())
        {
          string str = message.table();
          TVecConfigType vec;
          ConfigManager::getMe().getType(str, vec);
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (!cfg.isReload())
                continue;
              if (cfg.isSessionLoad())
                ConfigManager::getMe().loadConfig(cfg);
              if (cfg.isSceneLoad())
                bNtf = true;
            }
          }
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (!cfg.isReload())
                continue;
              if (cfg.isSessionLoad())
                ConfigManager::getMe().checkConfig(cfg);
            }
          }
        }

        if (bNtf)
        {
        }

        XINF << "[策划表-重加载] zoneid:" << thisServer->getZoneID() << "lua:" << message.lua() << "table:" << message.table() << "log:" << message.log() << XEND;
        return true;
      }
      break;
    case SERVERTEAMPARAM_CAT_ENTERTEAM:
      {
        PARSE_CMD_PROTOBUF(CatEnterTeamCmd, message);
        Team* pTeam = TeamManager::getMe().getTeamByUserID(message.charid());
        if (pTeam == nullptr)
          return true;

        TSetDWORD failEnterCatIDs;
        for (int i = 0; i < message.cats_size(); ++i)
        {
          const MemberCat& rCat = message.cats(i);
          if (pTeam->getMemberCount() >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
          {
            XERR << "[队伍消息-猫入队]" << rCat.ShortDebugString() << "进入" << pTeam->getGUID() << pTeam->getName() << "失败,超过最大成员" << XEND;
            failEnterCatIDs.insert(rCat.catid());
            continue;
          }

          TMember* pMember = pTeam->getMember(rCat.id());
          if (pMember != nullptr)
          {
            XERR << "[队伍消息-猫入队]" << rCat.ShortDebugString() << "进入" << pTeam->getGUID() << pTeam->getName() << "失败,已在队伍中" << XEND;
            failEnterCatIDs.insert(rCat.catid());
            continue;
          }

          pMember = NEW TMember(pTeam, 0, rCat.id(), thisServer->getZoneID(), ETEAMJOB_MEMBER);
          if (pMember == nullptr)
          {
            XERR << "[队伍消息-猫入队]" << rCat.ShortDebugString() << "进入" << pTeam->getGUID() << pTeam->getName() << "失败,创建失败" << XEND;
            failEnterCatIDs.insert(rCat.catid());
            continue;
          }

          pMember->setCatOwnerID(message.charid());
          pMember->setCatID(rCat.catid());
          pMember->setOfflineTime(0);
          pMember->setBaseLv(rCat.lv());
          pMember->setReliveTime(rCat.relivetime());
          pMember->setExpireTime(rCat.expiretime());
          pMember->setName(rCat.name());
          pTeam->addMember(pMember);
          XLOG << "[队伍消息-猫入队]" << rCat.ShortDebugString() << "进入" << pTeam->getGUID() << pTeam->getName() << "成功" << XEND;
        }

        // 进队失败, 通知场景
        for (auto &s : failEnterCatIDs)
        {
          CatExitTeamCmd cmd;
          cmd.set_charid(message.charid());
          cmd.set_catid(s);
          cmd.set_enterfail(true);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
      break;
    case SERVERTEAMPARAM_CAT_FIRE:
      {
        PARSE_CMD_PROTOBUF(CatFireTeamCmd, message);
        Team* pTeam = TeamManager::getMe().getTeamByUserID(message.charid());
        if (pTeam == nullptr)
          return true;
        pTeam->removeCat(message.npcid(), message.catid(), true);
      }
      return true;
    case SERVERTEAMPARAM_CAT_ENTEROWNTEAM:
      {
        PARSE_CMD_PROTOBUF(CatEnterOwnTeamCmd, message);
        Team* pTeam = TeamManager::getMe().getTeamByUserID(message.charid());
        if (pTeam != nullptr)
          return true;
        EnterTeam cmd;
        cmd.mutable_data()->CopyFrom(message.data());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToMe(thisServer->getZoneID(), message.charid(), send, len);
      }
      return true;
    default:
      return false;
  }

  return false;
}

bool TeamServer::doChatCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0 || rUser.charid() == 0 || rUser.zoneid() == 0)
    return false;

  switch (cmd->param)
  {
    case CHATPARAM_PLAYEXPRESSION:
      {
        PARSE_CMD_PROTOBUF(PlayExpressionChatCmd, rev);
        Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
        if (pTeam != nullptr)
          pTeam->broadcastCmd(buf, len);
        else
          thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), buf, len);
        XLOG << "[聊天管理-表情播放]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "播放表情 :" << rev.expressionid() << XEND;

        return true;
      }
      break;
    case CHATPARAM_QUERY_REALTIME_VOICE_ID:
      {
        Team* pTeam = TeamManager::getMe().getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
        {
          XERR << "[聊天管理-查询实时语音房间号]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "没有组队" << XEND;
          return true;
        }
        TMember* pMember = pTeam->getMember(rUser.charid());
        if (pMember == nullptr)
        {
          XERR << "[聊天管理-查询实时语音房间号]" << pTeam->getGUID() << "玩家:" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "找不到成员" << XEND;
          return true;
        }
        pMember->sendRealtimeVoiceID();
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

bool TeamServer::doMatchCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;
  
  XDBG << "[斗技场-收到消息]" << cmd->cmd << cmd->param << XEND;

  switch (cmd->param)
  {
  case MATCHSPARAM_NTF_JOIN_ROOM:
  {
    PARSE_CMD_PROTOBUF(NtfJoinRoom, rev);

    Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
    if (pTeam == nullptr)
    {
      //xerr
      XLOG << "[斗技场-报名]失败，玩家没有组队" << rev.charid()<<"roomid"<<rev.roomid() << XEND;
      return false;
    }  
    pTeam->setPvpRoomId(rev.roomid());
    pTeam->sendDataToMatchServer();

    XLOG << "[斗技场-报名]成功，发送队伍信息到MatchServer" << rev.charid() << "roomid" << rev.roomid() << XEND;
    return true;
  }
  break;
  case MATCHSPARAM_NTF_LEAVE_ROOM:
  {
    PARSE_CMD_PROTOBUF(NtfLeaveRoom, rev);

    Team* pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
    if (pTeam == nullptr)
    {
      //xerr
      XLOG << "[斗技场-离开房间]失败，找不到队伍" << rev.teamid() << "roomid" << rev.roomid() << XEND;
      return false;
    }
    pTeam->setPvpRoomId(0);
    pTeam->setMatchCreate(false);

    XLOG << "[斗技场-离开房间]成功 " << rev.teamid() << "roomid" << rev.roomid() << XEND;
    return true;
  }
  break;
  case MATCHSPARAM_CREATE_TEAM:
  {
    PARSE_CMD_PROTOBUF(CreateTeamMatchSCmd, rev);
    if (rev.zoneid() != thisServer->getZoneID())
    {
      TeamManager::getMe().removeMember(rev.charid());

      QWORD newTeamId = 0;
      do 
      {
        UserInfo userInfo;
        SocialUser* pUser = userInfo.mutable_user();
        if (pUser == nullptr)
          break;
        pUser->set_charid(rev.charid());
        pUser->set_zoneid(thisServer->getZoneID());
        Team* pTeam = TeamManager::getMe().createOneTeam(rev.name(), userInfo, ETEAMFILTER_TOWER, 1, 100, EAUTOTYPE_ALL);
        if (pTeam)
        {
          newTeamId = pTeam->getGUID();
          pTeam->setPvpRoomId(rev.roomid());
          pTeam->setMatchCreate(true, rev.pvptype());
        }
      } while (0);
      rev.set_new_teamid(newTeamId);
    }
    else
    {
      Team* pOldTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
      if (pOldTeam == nullptr)
      {
        XERR << "[斗技场-创建队伍], 异常, 找不到玩家所在队伍, 玩家:" << rev.charid() << XEND;
        return false;
      }
      pOldTeam->setPvpRoomId(rev.roomid());
      rev.set_new_teamid(pOldTeam->getGUID());
      pOldTeam->setMatchCreate(true, rev.pvptype());
    }

    PROTOBUF(rev, send, len);
    bool ret = thisServer->forwardCmdToMatch(send, len);
    XLOG << "[斗技场-创建队伍] 返回给MatchServer oldteamid " << rev.teamid() <<"oldzoneid"<<rev.zoneid()<< "roomid" << rev.roomid() <<"newteamid"<<rev.new_teamid()<<"发送ret"<<ret << XEND;

    return true;
  }
  break;
  case MATCHSPARAM_APPLY_TEAM:
  {
    PARSE_CMD_PROTOBUF(ApplyTeamMatchSCmd, rev);

    Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
    if (pTeam)
    {
      if (rev.teamid() == pTeam->getGUID())
      {
        XLOG << "[斗技场-申请加入队伍] teamid " << rev.teamid() << "charid" << rev.charid() << "zoneid" << rev.zoneid() << XEND;
        return true;
      }
      TeamManager::getMe().removeMember(rev.charid());
    }

    UserInfo userInfo;
    SocialUser* pUser = userInfo.mutable_user();
    if (pUser == nullptr)
      break;
    pUser->set_charid(rev.charid());
    pUser->set_zoneid(rev.zoneid());

    bool ret = TeamManager::getMe().applyTeamForce(userInfo, rev.teamid());
    XLOG << "[斗技场-申请加入队伍] teamid " << rev.teamid() << "charid"<<rev.charid() <<"zoneid"<<rev.zoneid()<< "ret" << ret << XEND;
    return true;
  }
  break;
  case MATCHSPARAM_KICK_TEAM:
  {
    PARSE_CMD_PROTOBUF(KickTeamMatchSCmd, rev);

    Team* pTeam = TeamManager::getMe().getTeamByUserID(rev.charid());
    if (!pTeam)
    {
      XERR << "[斗技场-踢出队伍] 失败,找不到队伍 teamid" << rev.teamid() << "charid" << rev.charid() << "zoneid" << rev.zoneid() << XEND;
      return false;
    }
    if (rev.teamid() != pTeam->getGUID())
    {
      XERR << "[斗技场-踢出队伍] 失败，队伍teamid不一致 teamid" << rev.teamid() << "charid" << rev.charid() << "zoneid" << rev.zoneid() << "serverteamid" << pTeam->getGUID() << XEND;
      return false;
    }

    TeamManager::getMe().removeMember(rev.charid());

    ExitTeam cmd;
    cmd.set_teamid(pTeam->getGUID());
    PROTOBUF(cmd, send, len);
    DWORD dwZoneID = thisServer->getZoneID();
    thisServer->sendCmdToMe(dwZoneID, rev.charid(), send, len);
    thisServer->sendMsg(dwZoneID, rev.charid(), 314);

    XLOG << "[斗技场-踢出队伍] 成功，teamid " << rev.teamid() << "charid" << rev.charid() << "zoneid" << rev.zoneid() << XEND;
    return true;
  }
  break;
  case MATCHSPARAM_JOIN_TEAMPWS:
  {
    // session->team->match
    PARSE_CMD_PROTOBUF(JoinTeamPwsMatchSCmd, rev);
    Team *pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
    if (pTeam == nullptr)
      return true;
    if (pTeam->isInMatchTeamPws())
      return true;
    for (int i = 0; i < rev.members_size(); ++i)
    {
      if (pTeam->getMember(rev.members(i)) == nullptr || TeamManager::getMe().isOnline(rev.members(i)) == false)
        return true;
    }
    //pTeam->setMatchingType(rev.etype());
    thisServer->forwardCmdToMatch(buf, len);
    XLOG << "[组队排位赛], 设置队伍报名状态, 队伍" << rev.teamid() << XEND;
    return true;
  }
  break;
  case MATCHSPARAM_LEAVE_TEAMPWS:
  {
    PARSE_CMD_PROTOBUF(ExitTeamPwsMatchSCmd, rev);
    Team *pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
    if (pTeam == nullptr)
      return true;
    pTeam->setMatchingType(EPVPTYPE_MIN);
    XLOG << "[组队排位赛], 取消队伍报名状态, 队伍:" << rev.teamid() << XEND;
  }
  break;
  case MATCHSPARAM_CONFIRM_TEAMMATCH:
  {
    PARSE_CMD_PROTOBUF(ConfirmTeamMatchSCmd, rev);
    Team *pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
    if (pTeam == nullptr)
      return true;
    pTeam->setMatchingType(rev.etype());
    XLOG << "[组队匹配模式], 设置队伍报名状态, 队伍" << rev.teamid() << "匹配类型:" << rev.etype() << XEND;
  }
  break;
  default:
    return false;
  }

  return false;
}
