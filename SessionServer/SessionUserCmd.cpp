#include "SessionUser.h"
#include "UserCmd.h"
#include "GuildCmd.pb.h"
#include "SessionUserManager.h"
#include "SessionScene.h"
#include "Boss.h"
#include "MsgManager.h"
#include "InfiniteTower.pb.h"
#include "SessionServer.h"
#include "SessionSceneManager.h"
#include "SessionUserManager.h"
#include "FerrisWheelManager.h"
#include "Dojo.pb.h"
#include "DojoConfig.h"
#include "RedisManager.h"
#include "ChatCmd.pb.h"
#include "SceneTrade.pb.h"
#include "TeamCmd.pb.h"
#include "RecordTrade.pb.h"
#include "RecordCmd.pb.h"
#include "ChatManager_SE.h"
#include "MatchSCmd.pb.h"
#include "MatchCCmd.pb.h"
#include "Authorize.pb.h"
#include "CommonConfig.h"
#include "DepositConfig.h"
#include "GMCommandManager.h"
#include "PveCard.pb.h"
#include "SessionGvg.h"
#include "TeamRaidCmd.pb.h"

bool SessionUser::doSessionUserCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->cmd)
  {
    case SESSION_USER_GUILD_PROTOCMD:
      return doSessionUserGuildCmd(buf, len);
    case SESSION_USER_TEAM_PROTOCMD:
      return doSessionUserTeamCmd(buf, len);
    case SCENE_BOSS_PROTOCMD:
      return BossList::getMe().doUserCmd(this, buf, len);
    case SESSION_USER_SHOP_PROTOCMD:
      return doSessionUserShopCmd(buf, len);
    case INFINITE_TOWER_PROTOCMD:
      return doSessionUserTowerCmd(buf, len);
    case SESSION_USER_MAIL_PROTOCMD:
      return doSessionUserMailCmd(buf, len);
    case SESSION_USER_SOCIALITY_PROTOCMD:
      return doSessionUserSocialCmd(buf, len);
    case DOJO_PROTOCMD:
      return doSessionUserDojoCmd(buf, len);
    case CHAT_PROTOCMD:
      return doSessionUserChatCmd(buf, len);
    case SCENE_USER_CARRIER_PROTOCMD:
      return doCarrierCmd(buf, len);
    case RECORD_USER_TRADE_PROTOCMD:
      return doTradeCmd(buf, len);
    case MATCHC_PROTOCMD:
        return doMatchCmd(buf, len);
    case SESSION_USER_AUTHORIZE_PROTOCMD:
        return doSessionAuthorizeCmd(buf, len);
    case USER_EVENT_PROTOCMD:
        return doUserEventCmd(buf, len);
    case AUCTIONC_PROTOCMD:
      return doAuctionCmd(buf, len);
    case WEDDINGC_PROTOCMD:
      return doUserWeddingCmd(buf, len);
    case SCENE_USER_ITEM_PROTOCMD:
      return doUserItemCmd(buf, len);
    case PVE_CARD_PROTOCMD:
      return doPveCardCmd(buf, len);
    case TEAM_RAID_PROTOCMD:
      return doTeamRaidCmd(buf, len);
  }

  return true;
}

bool SessionUser::doSessionUserGuildCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  if (isInPollyScene())
  {
    XERR << "[波利乱斗-功能屏蔽] charid" << id << "mapid" << getMapID() << XEND;
    return true;
  }

  switch (cmd->param)
  {
    case GUILDPARAM_INVITEMEMBER:
      {
        PARSE_CMD_PROTOBUF(InviteMemberGuildCmd, rev);
        SessionUser* pTarget = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pTarget == nullptr)
        {
          XERR << "[玩家-公会消息]" << accid << id << getProfession() << name << "邀请" << rev.charid() << "失败,不在线" << XEND;
          break;
        }
        if (pTarget->getSocial().checkRelation(id, ESOCIALRELATION_BLACK) == true || pTarget->getSocial().checkRelation(id, ESOCIALRELATION_BLACK_FOREVER) == true)
        {
          XERR << "[玩家-公会消息]" << accid << id << getProfession() << name << "邀请" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "失败,黑名单" << XEND;
          break;
        }
        sendUserCmdToSocialServer(buf, len, ECMDTYPE_GUILD);
      }
      break;
    case GUILDPARAM_APPLYGUILD:
      {
        PARSE_CMD_PROTOBUF(ApplyGuildGuildCmd, rev);

        GuildApplySocialCmd cmd;
        toData(cmd.mutable_user());
        cmd.set_guildid(rev.guid());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::guild_server, send, len);
      }
      break;
    case GUILDPARAM_PROCESSINVITEMEMBER:
      {
        PARSE_CMD_PROTOBUF(ProcessInviteGuildCmd, rev);

        GuildProcessInviteSocialCmd cmd;
        toData(cmd.mutable_user());
        cmd.set_action(rev.action());
        cmd.set_guildid(rev.guid());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::guild_server, send, len);
      }
      break;
    case GUILDPARAM_ENTERGUILDTERRITORY:
      {
        PARSE_CMD_PROTOBUF(EnterTerritoryGuildCmd, rev);
        rev.set_handid(getHandID());
        PROTOBUF(rev, send1, len1);
        sendUserCmdToSocialServer((const BYTE*)send1, len1, ECMDTYPE_GUILD);
      }
      break;
    case GUILDPARAM_CREATEGUILD:
      {
        if (thisServer->isPvpZone())
        {
          XERR << "[公会-创建公会] pvp 线不可创建公会"<< accid << id << getProfession() << name << XEND;
          return false;
        }
        sendUserCmdToSocialServer(buf, len, ECMDTYPE_GUILD);
      }
      break;
    case GUILDPARAM_RENAME_QUERY:
      {
        if (thisServer->isPvpZone())
        {
          XERR << "[公会-公会改名] pvp 线不可改名"<< accid << id << getProfession() << name << XEND;
          return false;
        }
        sendUserCmdToSocialServer(buf, len, ECMDTYPE_GUILD);
      }
      break;
    case GUILDPARAM_TREASURE_ACTION:
      {
        PARSE_CMD_PROTOBUF(TreasureActionGuildCmd, rev);
        if(ETREASUREACTION_OPEN_GUILD == rev.action())
        {
          const SGuildTreasureCFG* pTreasureCFG = GuildConfig::getMe().getTreasureCFG(rev.treasure().id());
          if(!pTreasureCFG)
          {
            XERR << "[公会宝箱-操作] 获取配置SGuildTreasureCFG失败"<< accid << id << getProfession() << name << rev.treasure().id() << XEND;
            return false;
          }

          if(EGUILDTREASURETYPE_GUILD_BCOIN == pTreasureCFG->eType)
          {
            if(m_oTradeLog.isGiving())
            {
              MsgManager::sendMsg(id, 25317);
              break;
            }
          }

        }

        sendCmdToSceneUser(buf, len);
      }
      break;
    case GUILDPARAM_QUERY_CITYSHOW:
      SessionGvg::getMe().queryCityShowInfo(this);
      break;
    default:
      sendUserCmdToSocialServer(buf, len, ECMDTYPE_GUILD);
      break;
  }

  return true;
}

bool SessionUser::doSessionUserTeamCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  if (cmd->param != TEAMPARAM_EXITTEAM)
  {
    if (isInPollyScene())
    {
      XERR << "[波利乱斗-功能屏蔽] charid" <<id <<"mapid"<< getMapID() <<XEND;
      return true;
    }
  }

  switch (cmd->param)
  {
    case TEAMPARAM_CREATETEAM:
      {
        PARSE_CMD_PROTOBUF(CreateTeam, rev);

        TeamCreateSocialCmd cmd;
        cmd.mutable_team()->CopyFrom(rev);
        toData(cmd.mutable_user());
        PROTOBUF(cmd, send, len);

        thisServer->sendCmdToServer(send, len, "TeamServer");
      }
      break;
    case TEAMPARAM_INVITEMEMBER:
      {
        PARSE_CMD_PROTOBUF(InviteMember, rev);

        if (rev.catid() != 0)
        {
          if (m_oGTeam.getTeamID() != 0 && m_oGTeam.getLeaderID() != id)
          {
            XERR << "[玩家-组队消息]" << accid << id << getProfession() << name << "邀请" << rev.userguid() << "失败,有队伍情况下只有队长才能邀请" << XEND;
            break;
          }
          if (rev.userguid() != id)
          {
            SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.userguid());
            if (pUser == nullptr)
            {
              XERR << "[玩家-组队消息]" << accid << id << getProfession() << name << "邀请" << rev.userguid() << "失败,该玩家不在线" << XEND;
              break;
            }
            pUser->sendCmdToSceneUser(buf, len);
          }
          else
          {
            sendCmdToSceneUser(buf, len);
          }
          break;
        }

        TeamInviteSocialCmd cmd;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.userguid());
        if (pUser == nullptr)
        {
          XERR << "[玩家-组队消息]" << accid << id << getProfession() << name << "邀请" << rev.userguid() << "失败,不在线" << XEND;
          break;
        }
        if (pUser->getSocial().checkRelation(id, ESOCIALRELATION_BLACK) == true || pUser->getSocial().checkRelation(id, ESOCIALRELATION_BLACK_FOREVER) == true)
        {
          MsgManager::sendMsg(this->id, 325, MsgParams(pUser->name));
          XERR << "[玩家-组队消息]" << accid << id << getProfession() << name << "邀请" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "失败,黑名单" << XEND;
          break;
        }

        if (pUser != nullptr)
          pUser->toData(cmd.mutable_beinvite());
        else
          cmd.mutable_beinvite()->set_charid(rev.userguid());
        toData(cmd.mutable_invite());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToServer(send, len, "TeamServer");
      }
      break;
    case TEAMPARAM_PROCESSINVITE:
      {
        PARSE_CMD_PROTOBUF(ProcessTeamInvite, rev);

        TeamProcessInviteSocialCmd cmd;
        cmd.set_type(rev.type());
        toData(cmd.mutable_user());
        cmd.set_leaderid(rev.userguid());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToServer(send, len, "TeamServer");
      }
      break;
    case TEAMPARAM_MEMBERAPPLY:
      {
        PARSE_CMD_PROTOBUF(TeamMemberApply, rev);

        TeamApplySocialCmd cmd;
        toData(cmd.mutable_apply());
        cmd.set_teamid(rev.guid());
        PROTOBUF(cmd, send, len);

        thisServer->sendCmdToServer(send, len, "TeamServer");
      }
      break;
    case TEAMPARAM_KICKMEMBER:
      {
        if (getTeamID() == 0)
          sendCmdToSceneUser(buf, len);
        else
          sendUserCmdToSocialServer(buf, len, ECMDTYPE_TEAM);
      }
      break;
    case TEAMPARAM_QUICKENTER:
      {
        PARSE_CMD_PROTOBUF(QuickEnter, rev);

        TeamQuickEnterSocialCmd cmd;
        toData(cmd.mutable_user());
        cmd.set_type(rev.type());
        cmd.set_set(rev.set());
        PROTOBUF(cmd, send, len);

        thisServer->sendCmdToServer(send, len, "TeamServer");
      }
      break;
    case TEAMPARAM_QUERYMEMBERCAT:
      {
        PARSE_CMD_PROTOBUF(QueryMemberCatTeamCmd, rev);
        const GTeam& rTeam = getTeam();
        if (rTeam.getTeamID() != 0)
        {
          for (auto &m : rTeam.getTeamMemberList())
          {
            SessionUser* pUser = SessionUserManager::getMe().getUserByID(m.first);
            if (pUser != nullptr)
              pUser->sendCmdToSceneUser(buf, len);
          }
        }
        else
          sendCmdToSceneUser(buf, len);
      }
      break;
    default:
      sendUserCmdToSocialServer(buf, len, ECMDTYPE_TEAM);
      break;
  }

  return true;
}

bool SessionUser::doSessionUserShopCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  if (isInPollyScene())
  {
    XERR << "[波利乱斗-功能屏蔽] charid" << id << "mapid" << getMapID() << XEND;
    return true;
  }

  switch (cmd->param)
  {
    case SHOPPARAM_BUYITEM:
    case SHOPPARAM_QUERY_SHOP_CONFIG:
    case SHOPPARAM_QUICKBUY_SHOP_CONFIG:
    case SHOPPARAM_EXCHANGEITEM_CMD:
      sendCmdToSceneUser(buf, len);
      break;
    default:
      return false;
  }

  return true;
}

bool SessionUser::doSessionUserTowerCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case ETOWERPARAM_TEAMTOWERINFO:
      {
        TeamTowerSummaryCmd cmd;
        cmd.set_maxlayer(TowerConfig::getMe().getMaxLayer());
        cmd.set_refreshtime(xTime::getWeekStart(xTime::getCurSec(), 5 * 3600) + 86400 * 7 + 5*3600);
        PROTOBUF(cmd, send, len);
        sendUserCmdToSocialServer((const BYTE*)send, len, ECMDTYPE_TOWER);
      }
      break;
    case ETOWERPARAM_INVITE:
    case ETOWERPARAM_REPLY:
    case ETOWERPARAM_ENTERTOWER:
      sendUserCmdToSocialServer(buf, len, ECMDTYPE_TOWER);
      break;
    default:
      return false;
  }

  return true;
}

bool SessionUser::doSessionUserMailCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch(cmd->param)
  {
    case MAILPARAM_QUERYALLMAIL:
      break;
    case MAILPARAM_UPDATE:
      break;
    case MAILPARAM_GETATTACH:
      {
        PARSE_CMD_PROTOBUF(GetMailAttach, rev);
        m_oMail.getAttach(rev.id());
      }
      break;
  }

  return true;
}

bool SessionUser::doSessionUserSocialCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case SOCIALITYPARAM_OPERATE_QUERY:
      {
        PARSE_CMD_PROTOBUF(OperateQuerySocialCmd, rev);
        queryOperateState(rev);
      }
      break;
    case SOCIALITYPARAM_OPERATE_TAKE:
      {
        PARSE_CMD_PROTOBUF(OperateTakeSocialCmd, rev);
        takeOperateReward(rev);
        return true;
      }
      break;
    case SOCIALITYPARAM_QUERY_CHARGE_VIRGIN:
      {
        PARSE_CMD_PROTOBUF(QueryChargeVirginCmd, rev);

        std::map<DWORD, DWORD> map_virgin;
        DepositConfig::getMe().getVirginList(map_virgin);
        if(0 < map_virgin.size())
        {
          xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charge_virgin");
          if(!field)
            return false;

          std::string strTag = "";
          char where[64] = {0};
          snprintf(where, sizeof(where), "accid=%llu", accid);
          xRecordSet set;
          QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
          if(QWORD_MAX == ret)
            return false;
          if(0 != ret)
            strTag = set[0].getString("tag");

          TVecDWORD vec_tag;
          numTok(strTag, ",", vec_tag);
          for(auto &v : map_virgin)
          {
            auto iter = std::find(vec_tag.begin(), vec_tag.end(), v.second);
            if(vec_tag.end() == iter)
              rev.add_datas(v.first);
          }
        }

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
      }
      break;
    case SOCIALITYPARAM_QUERY_USER_INFO:
      {
        PARSE_CMD_PROTOBUF(QueryUserInfoCmd, rev);
        GCharReader* pTargetGChar = nullptr;
        SessionUser* pTarget = SessionUserManager::getMe().getUserByID(rev.charid());
        GCharReader gChar(thisServer->getRegionID(), rev.charid());
        if(pTarget)
          pTargetGChar = pTarget->getGCharData();
        else
        {
          if(!gChar.getBySocial())
          {
            MsgManager::sendMsg(id, ESYSTEMMSG_ID_QUERY_CLOSE);
            XERR << "[玩家-信息查询]" << accid << id << getProfession() << name << "查询 charid：" << rev.charid() << "失败，未存在该玩家" << XEND;
            break;
          }
          pTargetGChar = &gChar;
        }

        if(!pTargetGChar)
        {
          XERR << "[玩家-信息查询]" << accid << id << getProfession() << name << "查询 charid：" << rev.charid() << "失败，gchar = null" << XEND;
          break;
        }

        SocialData *data = rev.mutable_data();
        if(!data)
          break;
        data->set_guid(pTargetGChar->getCharID());
        data->set_accid(pTargetGChar->getAccID());
        data->set_level(pTargetGChar->getBaseLevel());
        data->set_portrait(pTargetGChar->getPortrait());
        data->set_hair(pTargetGChar->getHair());
        data->set_haircolor(pTargetGChar->getHairColor());
        data->set_body(pTargetGChar->getBody());
        data->set_face(pTargetGChar->getFace());
        data->set_mouth(pTargetGChar->getMouth());
        data->set_eye(pTargetGChar->getEye());
        data->set_profession(pTargetGChar->getProfession());
        data->set_gender(pTargetGChar->getGender());
        data->set_blink(pTargetGChar->getBlink());
        data->set_name(pTargetGChar->getName());
        data->set_guildname(pTargetGChar->getGuildName());

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
      }
      break;
    default:
      sendUserCmdToSocialServer(buf, len, ECMDTYPE_SOCIALITY);
      break;
  }

  return true;
}

bool SessionUser::doSessionUserDojoCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
  case EDOJOPARAM_PANEL_OPER:
  case EDOJOPARAM_SPONSOR:
  case EDOJOPARAM_REPLY:
  case EDOJOPARAM_QUERYSTATE:
  case EDOJOPARAM_ENTERDOJO:
  {
    return sendUserCmdToSocialServer(buf, len, ECMDTYPE_DOJO);
    //thisServer->sendCmdToServer(buf, len, "TeamServer");
  }
  case EDOJOPARAM_DOJO_PUBLIC_INFO:
  case EDOJOPARAM_ADD_MSG:
    return sendUserCmdToSocialServer(buf, len, ECMDTYPE_DOJO_GUILD);
  default:
  return false;
  }

  return true;
}

bool SessionUser::doSessionUserChatCmd(const BYTE* buf, WORD len)
{
  return ChatManager_SE::getMe().doChatCmd(this, buf, len);
}

bool SessionUser::doCarrierCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case MAPPARAM_FERRISWHEEL_INVITE:
      {
        PARSE_CMD_PROTOBUF(FerrisWheelInviteCarrierCmd, rev);
        /*SessionUser* pTarget = SessionUserManager::getMe().getUserByID(rev.targetid());
        if (pTarget == nullptr)
        {
          bool bFind = false;
          const TeamInfo& rTeam = getTeamInfo();
          for (int i = 0; i < rTeam.member_size(); ++i)
          {
            const TeamMemberInfo& rMember = rTeam.member(i);
            if (rMember.charid() == rev.targetid() && rMember.zoneid() != thisServer->getZoneID())
            {
              MsgManager::sendMsg(id, ESYSTEAMSG_ID_FERRIS_INVITE_ZONE);
              bFind = true;
              break;
            }
          }
          if (!bFind)
            MsgManager::sendMsg(id, ESYSTEAMSG_ID_FERRIS_INVITE_ERROR);
          break;
        }*/

        FerrisInviteSessionCmd cmd;
        cmd.set_id(rev.id());
        cmd.set_charid(id);
        cmd.set_targetid(rev.targetid());
        PROTOBUF(cmd, send, len);
        sendCmdToScene(send, len);
      }
      break;
    case MAPPARAM_FERRISWHEEL_PROCESSINVITE:
      {
        PARSE_CMD_PROTOBUF(FerrisWheelProcessInviteCarrierCmd, rev);
        FerrisWheelManager::getMe().processInvite(rev.id(), this, rev.targetid(), rev.action());
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SessionUser::doSocialCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    default:
      return false;
  }

  return true;
}

bool SessionUser::doTradeCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  if ((cmd->param != Cmd::ACCEPT_TRADE_PARAM && cmd->param != Cmd::REFUSE_TRADE_PARAM && cmd->param != Cmd::REQ_GIVE_ITEM_INFO_TRADE_PARAM) 
    && isInPollyScene())
  {
    XERR << "[波利乱斗-功能屏蔽] charid" << id << "mapid" << getMapID() << XEND;
    return true;
  }
  
  if (!CommonConfig::getMe().IsTradeServerOpen())
  {
    XERR << "[交易-交易所服]功能暂时关闭" << XEND;
    return false;
  }
  
  if (MiscConfig::getMe().getTradeBlock().isIn(accid))
  {
    XERR << "[交易-交易所服] 功能屏蔽 charid" <<id <<"accid"<<accid << XEND;
    return false;
  }

  switch (cmd->param)
  {
  //case HOT_ITEMID_RECORDTRADE:
  // case BRIEF_PENDING_LIST_RECORDTRADE:
#ifdef _OLD_TRADE      //转发到老的交易所
  case DETAIL_PENDING_LIST_RECORDTRADE:
  case MY_PENDING_LIST_RECORDTRADE:
  case ITEM_SELL_INFO_RECORDTRADE:
  {
    return sendUserCmdToRecordServer(buf, len);
    break;
  }
#endif // _OLD_TRADE

  case MY_TRADE_LOG_LIST_RECORDTRADE:
  {
    PARSE_CMD_PROTOBUF(MyTradeLogRecordTradeCmd, rev);
    return m_oTradeLog.fetchMyTradeLogList(rev);
  }
  case TAKE_LOG_TRADE_PARAM:
  {
    PARSE_CMD_PROTOBUF(TakeLogCmd, rev);
    return m_oTradeLog.takeLog(rev);
  }
  case Cmd::FETCH_NAMEINFO_TRADE_PARAM:
  {
    PARSE_CMD_PROTOBUF(FetchNameInfoCmd, rev);
    m_oTradeLog.fetchNameInfo(rev);
    return true;
  }
  case Cmd::GIVE_TRADE_PARAM:
  {
    PARSE_CMD_PROTOBUF(GiveTradeCmd, rev);
    return m_oTradeLog.give(rev);
  }
  break;
  case Cmd::REQ_GIVE_ITEM_INFO_TRADE_PARAM: //session<-> scene
  {
    PARSE_CMD_PROTOBUF(ReqGiveItemInfoCmd, rev);
    return m_oTradeLog.reqGiveItemInfo(rev);
  }
  break;
  case Cmd::ACCEPT_TRADE_PARAM: //
  {
    PARSE_CMD_PROTOBUF(AcceptTradeCmd, rev);
    return m_oTradeLog.acceptGive(rev);
  }
  break;
  case Cmd::REFUSE_TRADE_PARAM: //
  {
    PARSE_CMD_PROTOBUF(RefuseTradeCmd, rev);
    return m_oTradeLog.refuseGive(rev);
  }
  break;
  case Cmd::CHECK_PACKAGE_SIZE_TRADE_CMD:
  {
    return this->sendCmdToScene(buf, len);
  }
  break;
  case Cmd::QUICK_TAKE_LOG_TRADE_PARAM:
  {
    PARSE_CMD_PROTOBUF(QucikTakeLogTradeCmd, rev);
    m_oTradeLog.quickTakeLog(rev.trade_type());
    return true;
  }
  break;
  case Cmd::QUERY_LOTTERY_GIVE_TRADE_PARAM:
  {
    PARSE_CMD_PROTOBUF(LotteryGiveCmd, rev);
    m_oTradeLog.reqLotteryGive(rev.iteminfo());
    return true;
  }
  break;
  default:
    return sendUserCmdToTradeServer(buf, len);    //send cmd to trade server
  }

  return true;
}

bool SessionUser::doMatchCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  if (isInPollyScene())
  {
    XERR << "[波利乱斗-功能屏蔽] charid" << id << "mapid" << getMapID() << XEND;
    return true;
  }

  XDBG << "[斗技场-收到客户端消息]" << cmd->cmd << cmd->param << XEND;
  switch (cmd->param)
  {
  case MATCHCPARAM_JOIN_ROOM:
  {
    PARSE_CMD_PROTOBUF(JoinRoomCCmd, rev);
    if (rev.type() == EPVPTYPE_LLH || rev.type() == EPVPTYPE_POLLY)
    {
      return sendUserCmdToMatchServer(buf, len);
    }
    if (rev.type() == EPVPTYPE_MVP)
    {
      if (m_oGTeam.getTeamID() == 0)
        break;
      const SMvpBattleCFG& rCFG = MiscConfig::getMe().getMvpBattleCFG();
      const TMapGTeamMember& memlist = m_oGTeam.getTeamMemberList();
      if (memlist.size() < rCFG.dwLimitTeamUserNum)
        break;
      for (auto &m : memlist)
      {
        if (m.second.level() < rCFG.dwLimitUserLv)
          return true;
      }
    }
    if (rev.type() == EPVPTYPE_SUGVG)
    {
      rev.set_guildid(m_oGuild.id());
      rev.set_teamid(m_oGTeam.getTeamID());
      if (rev.teamid())
      {
        rev.add_teammember(m_oGTeam.getTrueLeaderID());
      }
      PROTOBUF(rev, send, len);
      return sendUserCmdToMatchServer((const BYTE*)send, len);
    }
    if (rev.type() == EPVPTYPE_TEAMPWS || rev.type() == EPVPTYPE_TEAMPWS_RELAX)
    {
      bool relax = (rev.type() == EPVPTYPE_TEAMPWS_RELAX);
      // check
      if (m_oGTeam.getTeamID() == 0)
        break;
      if (m_oGTeam.getLeaderID() != this->id)
        break;
      const TMapGTeamMember& memlist = m_oGTeam.getTeamMemberList();
      const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();

      JoinTeamPwsMatchSCmd cmd;
      //DWORD allscore = 0;
      for (auto &m : memlist)
      {
        // 需所有成员在线
        SessionUser* user = SessionUserManager::getMe().getUserByID(m.first);
        if (user == nullptr)
          return true;

        // 次数要求
        if (!relax)
        {
          if (user->getVarValue(EVARTYPE_TEAMPWS_COUNT) >= rCFG.dwWeekMaxCount)
          {
            MsgManager::sendMsg(id, 25902);
            return true;
          }
        }

        // 等级条件
        if (m.second.level() < rCFG.dwRequireLv)
          return true;
        cmd.add_members(m.first);
        //allscore += user->getMatchScore(EPVPTYPE_TEAMPWS);
      }
      // 检查成功, 发送->team 记录状态 -> match
      cmd.set_teamid(m_oGTeam.getTeamID());
      cmd.set_zoneid(thisServer->getZoneID());
      cmd.set_leaderid(this->id);
      cmd.set_etype(rev.type());
      cmd.set_roomid(rev.roomid());
      //if (memlist.size())
        //cmd.set_avescore(allscore / memlist.size());

      PROTOBUF(cmd, teamsend, teamlen);
      thisServer->sendCmdToServer(teamsend, teamlen, "TeamServer");

      XLOG << "[组队排位赛-报名], team:" << m_oGTeam.getTeamID() << "队长:" << this->name << this->id << "报名成功" << XEND;
      return true;
    }
    if (rev.type() == EPVPTYPE_TUTOR)
    {
      TutorMatcher* pMatcher = rev.mutable_matcher();
      GSocial& rSocial = getSocial();
      const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();

      // check tutor valid
      const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_PUNISH, id);
      DWORD dwPunish = 0;
      RedisManager::getMe().getData<DWORD>(key, dwPunish);
      if (dwPunish != 0)
      {
        if (pMatcher->findtutor())
          MsgManager::sendMsg(id, ESYSTEMMSG_ID_TUTOR_PUNISH_TUTOR_S);
        else
          MsgManager::sendMsg(id, ESYSTEMMSG_ID_TUTOR_PUNISH_STUDENT_S);
        return true;
      }

      if (pMatcher->findtutor())
      {
        // check student valid
        DWORD dwBaseLv = getBaseLevel();
        if (dwBaseLv < rCFG.dwStudentBaseLvReq)
        {
          XERR << "[导师匹配-加入]" << accid << id << getProfession() << name << "使用 gender :" << pMatcher->gender() << "findtutor :" << pMatcher->findtutor()
            << "加入匹配失败,未达到学生资格等级" << rCFG.dwStudentBaseLvReq << XEND;
          return true;
        }
        if (dwBaseLv >= rCFG.dwTutorBaseLvReq)
        {
          XERR << "[导师匹配-加入]" << accid << id << getProfession() << name << "使用 gender :" << pMatcher->gender() << "findtutor :" << pMatcher->findtutor()
            << "加入匹配失败,超过导师资格等级" << rCFG.dwTutorBaseLvReq << XEND;
          return true;
        }
        if (rSocial.getRelationCount(ESOCIALRELATION_TUTOR) > 0)
        {
          XERR << "[导师匹配-加入]" << accid << id << getProfession() << name << "使用 gender :" << pMatcher->gender() << "findtutor :" << pMatcher->findtutor() << "加入匹配失败,已拥有导师" << XEND;
          return true;
        }
      }
      else
      {
        if (getBaseLevel() < rCFG.dwTutorBaseLvReq || getGCharData()->getTutor() == false)
        {
          XERR << "[导师匹配-加入]" << accid << id << getProfession() << name << "使用 gender :" << pMatcher->gender() << "findtutor :" << pMatcher->findtutor() << "加入匹配失败,未有导师资格" << XEND;
          return true;
        }
        if (rSocial.getRelationCount(ESOCIALRELATION_STUDENT) >= rCFG.dwMaxStudent)
        {
          XERR << "[导师匹配-加入]" << accid << id << getProfession() << name << "使用 gender :" << pMatcher->gender() << "findtutor :" << pMatcher->findtutor() << "加入匹配失败,超过最大学生数" << XEND;
          return true;
        }
      }

      pMatcher->set_charid(id);
      pMatcher->set_zoneid(thisServer->getZoneID());
      pMatcher->set_selfgender(getGender());

      pMatcher->clear_datas();

      add_data(pMatcher->add_datas(), EUSERDATATYPE_ROLELEVEL, m_oGCharData.getBaseLevel());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_PROFESSION, m_oGCharData.getProfession());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_SEX, m_oGCharData.getGender());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_BODY, m_oGCharData.getBody());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_PORTRAIT, m_oGCharData.getPortrait());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_EYE, m_oGCharData.getEye());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_MOUTH, m_oGCharData.getMouth());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_HAIR, m_oGCharData.getHair());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_HAIRCOLOR, m_oGCharData.getHairColor());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_HEAD, m_oGCharData.getHead());
      add_data(pMatcher->add_datas(), EUSERDATATYPE_NAME, 0, name);

      TSetQWORD setBlackIDs;
      rSocial.collectRelation(ESOCIALRELATION_BLACK, setBlackIDs);
      rSocial.collectRelation(ESOCIALRELATION_BLACK_FOREVER, setBlackIDs);

      pMatcher->clear_blackids();
      for (auto &s : setBlackIDs)
        pMatcher->add_blackids(s);

      PROTOBUF(rev, send, len);
      return sendUserCmdToMatchServer((const BYTE*)send, len);
    }

    if (m_oGTeam.getTeamID() == 0)
    {
      if (rev.type() == EPVPTYPE_SMZL)
        MsgManager::sendMsg(id, 954);
      else if (rev.type() == EPVPTYPE_HLJS)
        MsgManager::sendMsg(id, 959);

      XERR << "[斗技场-报名房间]" << accid << id << getProfession() << name << "加入房间" << rev.type() << rev.roomid() << "失败,当前没有组队" << XEND;
      break;
    }
    if (m_oGTeam.getTeamID() != 0)
    { 
      do 
      {
        QWORD trueLeader = m_oGTeam.getTrueLeaderID();
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(trueLeader);
        if (pUser && pUser->id == id)
          break;
        QWORD tempLeader = m_oGTeam.getTeampLeaderID();
        SessionUser* pTempLeader = SessionUserManager::getMe().getUserByID(tempLeader);
        if (pTempLeader && pTempLeader->id == id)
          break;

        if (rev.type() == EPVPTYPE_SMZL)
          MsgManager::sendMsg(id, 954);
        else if (rev.type() == EPVPTYPE_HLJS)
          MsgManager::sendMsg(id, 959);

        XERR << "[斗技场-报名房间]" << accid << id << getProfession() << name << "加入房间" << rev.type() << rev.roomid() << "失败,有队伍情况下只有队长才能操作"<<"tempLeader"<< tempLeader<<"trueLeader"<< trueLeader << XEND;
        return false;
      } while (0);    
    }

    rev.set_teamid(m_oGTeam.getTeamID());
    const TMapGTeamMember teamMember = m_oGTeam.getTeamMemberList();
    for (auto &v : teamMember)
    {
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(v.first);
      if (pUser)
      {
        rev.add_teammember(v.first);
      }
    }
    if (rev.type() == EPVPTYPE_MVP)
    {
      for (auto &v : teamMember)
      {
        JoinRoomUser* p = rev.add_users();
        if (!p)
          continue;
        p->set_charid(v.first);
        p->set_name(v.second.name());
      }
    }

    PROTOBUF(rev, send1, len1);
    return sendUserCmdToMatchServer((const BYTE* )send1, len1);
  }
  break;
  case MATCHCPARAM_LEAVE_ROOM:
  {
    PARSE_CMD_PROTOBUF(LeaveRoomCCmd, rev);
    if (rev.type() == EPVPTYPE_TEAMPWS || rev.type() == EPVPTYPE_TEAMPWS_RELAX)
    {
      rev.set_teamid(m_oGTeam.getTeamID());
      PROTOBUF(rev, send1, len1);
      sendUserCmdToMatchServer((const BYTE*)send1, len1);
    }
    else
    {
      sendUserCmdToMatchServer(buf, len);
    }
  };
  break;
  case MATCHCPARAM_TEAMPWS_QUERY_TEAMINFO:
  {
    PARSE_CMD_PROTOBUF(QueryTeamPwsTeamInfoMatchCCmd, rev);
    // 有队伍
    if (m_oGTeam.getTeamID())
    {
      const TMapGTeamMember& memlist = m_oGTeam.getTeamMemberList();
      for (auto &m : memlist)
        rev.add_userinfos()->set_charid(m.first);
    }
    else
    {
      rev.add_userinfos()->set_charid(this->id);
    }
    PROTOBUF(rev, send1, len1);
    sendUserCmdToMatchServer((const BYTE*)send1, len1);
  }
  break;
  default:
    return sendUserCmdToMatchServer(buf, len);    //send cmd to match server
  }

  return true;
}

bool SessionUser::doAuctionCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  if (isInPollyScene())
  {
    XERR << "[波利乱斗-功能屏蔽] charid" << id << "mapid" << getMapID() << XEND;
    return true;
  }

  XDBG << "[拍卖行-收到客户端消息]" << cmd->cmd << cmd->param << XEND;
  
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_AUCTION) == true)
  {
    XLOG << "[拍卖行-拍卖行已关闭] charid" << id << XEND;
    return true;
  }

  switch (cmd->param)
  {
  case AUCTIONCPARAM_REQ_AUCTION_RECORD:
  {
    PARSE_CMD_PROTOBUF(ReqAuctionRecordCCmd, rev);
    m_oAuction.reqAuctionRecord(rev);
  }
  break;
  case AUCTIONCPARAM_REQ_MY_TRADED_PRICE:
  {
    PARSE_CMD_PROTOBUF(ReqMyTradedPriceCCmd, rev);
    m_oAuction.reqMyTradedPrice(rev);
  }
  break;
  case AUCTIONCPARAM_REQ_ACUTION_INFO:
  {
    AuctionMgr::getMe().openPanel(this);
    return sendUserCmdToAuctionServer(buf, len);    //send cmd to auction server
  }
  break;
  case AUCTIONCPARAM_OPEN_AUCTION_PANEL:
  {
    PARSE_CMD_PROTOBUF(Cmd::OpenAuctionPanelCCmd, rev);
    if (rev.open())
    {
      AuctionMgr::getMe().openPanel(this);
    }
    else
    {
      AuctionMgr::getMe().closePanel(this);
    }
    return sendUserCmdToAuctionServer(buf, len);    //send cmd to auction server
  }
  break;
  default:
    return sendUserCmdToAuctionServer(buf, len);    //send cmd to auction server
  }

  return true;
}


bool SessionUser::doSessionAuthorizeCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch(cmd->param)
  {
    case SET_AUTHORIZE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(SetAuthorizeUserCmd, rev);
        m_oAuthorize.setPwd(rev.password(),rev.oldpwd());
      }
      break;
    case RESET_AUTHORIZE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(ResetAuthorizeUserCmd, rev);
        m_oAuthorize.resetPwd(rev.reset());
      }
      break;
    case SYNC_AUTHORIZE_TO_SESSION:
      {
        PARSE_CMD_PROTOBUF(SyncAuthorizeToSession, rev);
        m_oAuthorize.setAuthorize(rev.password(), rev.ignorepwd(), rev.resettime(), true);
        m_oAuthorize.syncInfoToSocial();
      }
      break;
    case SYNC_REAL_AUTHORIZE_TO_SESSION:
      {
        PARSE_CMD_PROTOBUF(SyncRealAuthorizeToSession, rev);
        m_oAuthorize.setRealAuthorize(rev.authorized());
        sendCmdToSceneUser(buf, len);
      }
      break;
  }

  return true;
}

bool SessionUser::doUserEventCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case USER_EVENT_CHARGE_QUERY:
    {
      PARSE_CMD_PROTOBUF(ChargeQueryCmd, rev);
      bool ret = false;
      DWORD outCount = 0;
      DWORD ym = 0;
      do {
        const SDeposit* pCfg = DepositConfig::getMe().getSDeposit(rev.data_id());
        if (pCfg == nullptr)
        {
          ret = false;
          break;
        }       
        const SGlobalActCFG* pActCfg = nullptr;
        bool bUpdateLimit = false;
        if (GMCommandManager::getMe().canBuy(accid, id, pCfg, outCount, ym, false, &pActCfg, bUpdateLimit) == false)
        {
          ret = false;
          break;
        }
        ret = true;
      } while (0);
      rev.set_charged_count(outCount);
      rev.set_ret(ret);
      PROTOBUF(rev, send, len);
      XINF << "[充值-检测] charid" << id << "data_id" << rev.data_id() << "ret" << ret << "已充值次数" << outCount << "ym" << ym << XEND;
      sendCmdToMe(send, len);
      break;
    }
    case USER_EVENT_QUERY_CHARGE_CNT:
    {
      PARSE_CMD_PROTOBUF(QueryChargeCnt, rev);
      sendChargeCnt();
      break;
    }
    case USER_EVENT_LOVELETTER_USE:
    {
      PARSE_CMD_PROTOBUF(LoveLetterUse, rev);
      processLoveLetter(rev);
      break;
    }
    default:
      return false;
  }
  return true;
}

bool SessionUser::doUserWeddingCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  { 
  //case WEDDINGCPARAM_REQ_WEDDINGDATE_LIST:
  //  return sendUserCmdToWeddingServer(buf, len);    //send cmd to wedding server
  default:
    return sendUserCmdToWeddingServer(buf, len);    //send cmd to wedding server
  }
  return true;
}

bool SessionUser::doUserItemCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
  case ITEMPARAM_GIVE_WEDDING_DRESS:
    {
      if(m_oTradeLog.isGiving())
      {
        MsgManager::sendMsg(id, 25318);
        break;
      }

      sendCmdToSceneUser(buf, len);
    }
    break;
  default:
    return false;
  }
  return true;
}


bool SessionUser::doPveCardCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;
  switch(cmd->param)
  {
    case EPVE_INVITE_TEAM_CMD:
    case EPVE_REPLY_TEAM_CMD:
    case EPVE_ENTER_RAID_CMD:
      sendUserCmdToTeamServer(buf, len, ECMDTYPE_PVECARD);
      break;
    default:
      break;
  }
  return true;
}

bool SessionUser::doTeamRaidCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;
  switch(cmd->param)
  {
    case TEAMRAIDPARAM_INVITE:
    case TEAMRAIDPARAM_REPLY:
    case TEAMRAIDPARAM_ENTER:
      sendUserCmdToTeamServer(buf, len, ECMDTYPE_TEAMRAID);
      break;
    default:
      break;
  }
  return true;
}
