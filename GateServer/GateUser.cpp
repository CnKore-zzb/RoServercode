#include "GateUser.h"
#include "GateUserManager.h"
#include "RegCmd.h"
#include "GatewayCmd.h"
#include "IndexManager.h"
#include "xTime.h"
#include "xCmd.pb.h"
#include "SceneUser.pb.h"
#include "LoginUserCmd.pb.h"
#include "ErrorUserCmd.pb.h"
#include "InfiniteTower.pb.h"
#include "CarrierCmd.pb.h"
#include "BossCmd.pb.h"
#include "Dojo.pb.h"
#include "ChatCmd.pb.h"
#include "ClientCmd.h"
#include "SessionSociality.pb.h"
#include "ChatCmd.pb.h"
#include "UserEvent.pb.h"
#include "Authorize.pb.h"
#include "ChatFilterManager.h"
#include "GuildCmd.pb.h"
#include "WeddingCCmd.pb.h"
#include "PveCard.pb.h"
#include "TeamRaidCmd.pb.h"

GateUser::GateUser(ACCID aid, xNetProcessor* client_task):m_oTickTwentySec(20)
{
  set_tempid(aid);
  accid = aid;
  scene_task_ = NULL;
  setState(GateUser_State::create);

  client_task_ = client_task;
  if (client_task_)
  {
    client_task_->setComp(true);
    char *ip = inet_ntoa(client_task_->getIP());
    if (ip)
    {
      m_strProxyIP = ip;
      XLOG << "[GateUser-创建]" << accid << tempid << ip << ":" << ntohs(client_task_->getPort()) << XEND;
    }
  }
}

GateUser::~GateUser()
{
  OneLevelIndexManager::getMe().remove(this);
  TwoLevelIndexManager::getMe().remove(TWO_LEVEL_INDEX_TYPE_SCREEN, this);

  if (client_task_)
  {
    thisServer->addCloseList(client_task_, TerminateMethod::terminate_active, "GateUser析构");
    client_task_ = NULL;
  }

  XDBG << "[GateUser]" << tempid << id << name << "析构" << XEND;
}

bool GateUser::init(ServerTask *scene_task,const GateUserData& data)
{
  if (!scene_task) return false;
  if (!client_task()) return false;

  set_scene_task(scene_task);
  set_id(data.charid);
  set_name(data.name);

  Cmd::SynTimeUserCmd syn;
  syn.set_servertime(xTime::getCurUSec()/1000);
  PROTOBUF(syn, send, len);
  sendCmdToMe(send, len);

  return true;
}

bool GateUser::sendCmdToMe(void* cmd, unsigned short len)
{
  /*
  if (GateUser_State::quit == getState())
  {
    XDBG("[SendCmdToMe],%llu,%llu,%s,玩家已退出", accid, id, name);
    return false;
  }
  */
  thisServer->getMsgCounter().add(cmd, len);
  if (client_task())
  {
#ifdef _LX_DEBUG
  //  XDBG("[SendCmdToMe],%llu,%llu,%s,发送:%u,%u,%u", accid, id, name, (DWORD)(*(((BYTE *)cmd))), (DWORD)(*(((BYTE *)cmd)+1)), len);
#endif
    if (getState() == GateUser_State::running)
    {
      if (((unsigned char *)cmd)[0] == CLIENT_CMD)
      {
        sendAllCmd();
        return client_task()->sendCmd(cmd, len);
      }
      m_oCmds.put((unsigned char *)cmd, len);
      ++m_oCmds.m_dwSize;
      if (m_oCmds.isFull())
        sendAllCmd();
      return true;
    }
    else
    {
      return client_task()->sendCmd(cmd, len);
    }
  }
  else
  {
#ifdef _LX_DEBUG
    XDBG << "[SendCmdToMe]" << accid << id << name << "发送失败" << XEND;
#endif
  }
  return false;
}

void GateUser::sendAllCmd()
{
  if (m_oCmds.empty()) return;
  if (GateUser_State::quit == getState()) return;
  if (!client_task()) return;

  DWORD total = 0;

  BUFFER_CMD(send, Cmd::CmdSetUserCmd);
  WORD size = sizeof(Cmd::CmdSetUserCmd);

  CmdPair *pair = m_oCmds.get();
  while (pair)
  {
    if (total + pair->first + size > MAX_CMD_SET_PACKSIZE)
    {
      if (send->num)
      {
        client_task()->sendCmd(send, total + size);

        total = 0;
        bzero(send, sizeof(*send));
        constructInPlace<Cmd::CmdSetUserCmd>(send);
      }
      else
      {
        client_task()->sendCmd(pair->second, pair->first);
        m_oCmds.erase();
        m_oCmds.m_dwSize--;
        pair = m_oCmds.get();
        continue;
      }
    }

    *(WORD *)(send->data + total) = pair->first;
    total += sizeof(WORD);
    bcopy(pair->second, send->data + total, pair->first);
    total += pair->first;

    ++send->num;

    m_oCmds.erase();
    m_oCmds.m_dwSize--;
    pair = m_oCmds.get();
  }
  if (send->num > 1)
  {
    client_task()->sendCmd(send, total + size);
  }
  else if (total >= sizeof(WORD))
  {
    client_task()->sendCmd(send->data + sizeof(WORD), total - sizeof(WORD));
  }
}

void GateUser::timer(DWORD cur)
{
  if (m_oTickTwentySec.timeUp(cur))
  {
    if (m_dwTotalCount >= 320)
    {
      printCmdCounter();
    }
    m_oCmdCounter.clear();
    m_dwTotalCount = 0;
  }
}

bool GateUser::doUserCmd(Cmd::UserCmd *cmd, unsigned short len)
{
  if (!cmd || !len) return true;
  if ((GateUser_State::running!=getState()))
  {
    if (LOGIN_USER_CMD != cmd->cmd)
    {
      return true;
    }
  }

  if (thisServer->m_oCmdFiler.find(*cmd)!=thisServer->m_oCmdFiler.end()) return true;

  switch (cmd->cmd)
  {
    case LOGIN_USER_CMD:
      {
        return doLoginUserCmd((Cmd::UserCmd *)cmd, len);
      }
      break;
    case Cmd::SCENE_USER_PROTOCMD:
    case Cmd::SCENE_USER2_PROTOCMD:
    case Cmd::SCENE_USER_SKILL_PROTOCMD:
    case Cmd::SCENE_USER_QUEST_PROTOCMD:
    case Cmd::SCENE_USER_MAP_PROTOCMD:
    case Cmd::SCENE_USER_PET_PROTOCMD:
    case Cmd::FUBEN_PROTOCMD:
    case Cmd::SCENE_USER_ACHIEVE_PROTOCMD:
    case Cmd::SCENE_USER_TIP_PROTOCMD:
    case Cmd::SCENE_USER_CHATROOM_PROTOCMD:
    case Cmd::SCENE_USER_INTER_PROTOCMD:
    case Cmd::SCENE_USER_MANUAL_PROTOCMD:
    case Cmd::SCENE_USER_SEAL_PROTOCMD:
    case Cmd::SCENE_USER_AUGURY_PROTOCMD:
    case Cmd::SCENE_USER_ASTROLABE_PROTOCMD:
    case Cmd::SCENE_USER_PHOTO_PROTOCMD:
    case Cmd::SCENE_USER_FOOD_PROTOCMD:
    case Cmd::SCENE_USER_TUTOR_PROTOCMD:
    case Cmd::SCENE_USER_BEING_PROTOCMD:
      {
        return doSceneUserCmd((Cmd::UserCmd*)cmd, len);
      }
      break;
    case Cmd::SCENE_USER_ITEM_PROTOCMD:
      {
        switch(cmd->param)
        {
          case ITEMPARAM_GIVE_WEDDING_DRESS:
            return false;
            //return forwardSession((Cmd::UserCmd*)cmd, len);
          default:
            return doSceneUserCmd((Cmd::UserCmd*)cmd, len);
        }
        break;
      }
    case Cmd::SCENE_USER_CARRIER_PROTOCMD:
      {
        switch (cmd->param)
        {
          case MAPPARAM_START_FERRISWHEEL:
          case MAPPARAM_INVITE_CARRIER:
          case MAPPARAM_CATCH_USER_JOIN_CARRIER:
          case MAPPARAM_RET_JOIN_CARRIER:
            return false;
          default:
            return doSceneUserCmd((Cmd::UserCmd*)cmd, len);
        }
      }
      break;
    case Cmd::SCENE_BOSS_PROTOCMD:
      {
        switch (cmd->param)
        {
          case Cmd::BOSS_POS_USER_CMD:
          case Cmd::BOSS_KILL_USER_CMD:
            return false;
          case Cmd::BOSS_STEP_SYNC:
            return forwardScene((Cmd::UserCmd*)cmd, len);
          default:
            return forwardSession((Cmd::UserCmd*)cmd, len);
        }
      }
      break;
    case USER_EVENT_PROTOCMD:
      {
        return doUserEventCmd((Cmd::UserCmd*)cmd, len);
      }
      break;
    case SESSION_USER_GUILD_PROTOCMD:
      return doGuildCmd((Cmd::UserCmd*)cmd, len);
    case SESSION_USER_TEAM_PROTOCMD:
    case SESSION_USER_SHOP_PROTOCMD:
    case SESSION_USER_MAIL_PROTOCMD:
      return forwardSession((Cmd::UserCmd*)cmd, len);
    case RECORD_USER_TRADE_PROTOCMD:
      return forwardSession((Cmd::UserCmd*)cmd, len);
    case Cmd::INFINITE_TOWER_PROTOCMD:
      return doTowerUserCmd(cmd, len);
    case Cmd::DOJO_PROTOCMD:
      return doDojoUserCmd(cmd, len);
    case Cmd::CHAT_PROTOCMD:
      return doChatUserCmd(cmd, len);
    case Cmd::MATCHC_PROTOCMD:
      return forwardSession(cmd, len);
    case Cmd::AUCTIONC_PROTOCMD:
      return forwardSession(cmd, len);
    case SESSION_USER_SOCIALITY_PROTOCMD:
      {
        if (cmd->param >= 100)
          return false;

        return forwardSession((Cmd::UserCmd*)cmd, len);
      }
      break;
    case Cmd::SESSION_USER_AUTHORIZE_PROTOCMD:
      return forwardSession((Cmd::UserCmd*)cmd, len);
    case Cmd::WEDDINGC_PROTOCMD:
      return doWeddingUserCmd((Cmd::UserCmd*)cmd, len);
    case Cmd::PVE_CARD_PROTOCMD:
      return doPveCardCmd(cmd, len);
    case Cmd::TEAM_RAID_PROTOCMD:
      return doTeamRaidCmd(cmd, len);
    default:
      break;
  }
  return false;
}

bool GateUser::doLoginUserCmd(Cmd::UserCmd* buf, unsigned short len)
{
  if (!buf || !len) return true;
  using namespace Cmd;

  switch (buf->param)
  {
    case SERVERTIME_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(ServerTimeUserCmd, rev);
        rev.set_time(xTime::getCurMSec());

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);

        return true;
      }
      break;
    case HEART_BEAT_USER_CMD:
      {
        sendCmdToMe((void *)buf, len);
        sendAllCmd();

        return true;
      }
      break;
    case DELETE_CHAR_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::DeleteCharUserCmd, rev);

        DelCharRegCmd send;
        send.accid = accid;
        send.id = rev.id();
        thisServer->sendCmdToServer(&send, sizeof(send), "SessionServer");

        XLOG << "[删除角色]" << accid << rev.id() << XEND;

        return true;
      }
      break;
    case CREATE_CHAR_USER_CMD:
      {
        if (GateUser_State::login!=getState()) return true;

        PARSE_CMD_PROTOBUF(Cmd::CreateCharUserCmd, message);

        /*RegErrRet eErr = REG_ERR_SUCC;
        if ((message.role_sex()!=EGENDER_FEMALE) && (message.role_sex()!=EGENDER_MALE))
          eErr = REG_ERR_RELOGIN_OVERTIME;
        if (message.profession() <= EPROFESSION_MIN || message.profession() >= EPROFESSION_MAX)
          eErr = REG_ERR_RELOGIN_OVERTIME;*/

        CreateCharRegCmd send;
        send.accid = accid;
        send.zoneID = thisServer->getZoneID();
        //strncpy(send.name, thisServer->getFullName(str.c_str(), zoneID).c_str(), MAX_NAMESIZE);
        strncpy(send.name, message.name().c_str(), MAX_NAMESIZE);
        send.role_sex = message.role_sex();
        send.role_career = message.profession();
        send.role_hairtype = message.hair();
        send.role_haircolor = message.haircolor();
        send.role_clothcolor = message.clothcolor();
        send.sequence = message.sequence();
        strncpy(send.gateName, thisServer->getServerName(), MAX_NAMESIZE);
        thisServer->sendCmdToServer(&send, sizeof(send), "SessionServer");

        return true;
      }
      break;
    case SELECT_ROLE_USER_CMD:
      {
        if (GateUser_State::select_role != getState()) return true;

        PARSE_CMD_PROTOBUF(Cmd::SelectRoleUserCmd, message);

        if (!message.id()) return true;
        //加入gateUserManager

        set_id(message.id());
        set_name(message.name().c_str());

        Cmd::LoginResultUserCmd cmd;
        cmd.set_ret(REG_ERR_SUCC);
        PROTOBUF(cmd, send1, len1);
        sendCmdToMe(send1, len1);

        LoginRegCmd send;
        send.id = message.id();
        send.accid = accid;
        strncpy(send.name, message.name().c_str(), MAX_NAMESIZE);
        strncpy(send.gateName, thisServer->getServerName(), sizeof(send.gateName));
        strncpy(send.deviceid, message.deviceid().c_str(), sizeof(send.deviceid));
        strncpy(send.phone, message.extradata().phone().c_str(), sizeof(send.phone));
        send.safeDevice = message.extradata().safedevice();

        send.ip = getIP();
        send.platformID = thisServer->getPlatformID();;

        send.ignorepwd = message.ignorepwd();
        strncpy(send.password, message.password().c_str(), sizeof(send.password));
        send.resettime = message.resettime();
        send.language = message.language();
        send.realAuthorized = message.realauthorized();
        send.maxbaselv = message.maxbaselv();

        thisServer->sendCmdToServer(&send, sizeof(send), "SessionServer");

        XLOG << "[登录]" << accid << send.id << send.name << "选择角色" <<"实名认证"<<message.realauthorized() << XEND;
        syncServerTime2Client();
        return true;
      }
      break;
    case SYNC_AUTHORIZE_GATE_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::SyncAuthorizeGateCmd, rev);

        SyncAuthorizeToSession cmd;
        cmd.set_ignorepwd(rev.ignorepwd());
        cmd.set_password(rev.password());
        cmd.set_resettime(rev.resettime());
        PROTOBUF(cmd, send, len);
        forwardSession((Cmd::UserCmd*)send, len);
        XLOG << "[安全密码-同步网关]" << accid << "ignorepwd" << rev.ignorepwd() << "resettime" << rev.resettime() << XEND;

        return true;
      }
      break;
    case REAL_AUTHORIZE_SEERVER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::RealAuthorizeServerCmd, message);
        SyncRealAuthorizeToSession cmd;
        cmd.set_authorized(message.authorized());
        PROTOBUF(cmd, send, len);
        forwardSession((Cmd::UserCmd*)send, len);
        XLOG << "[实名认证-同步网关]" << accid << message.authorized() << XEND;
      }
      break;
    default:
      break;
  }
  return true;
}

bool GateUser::doSceneUserCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  if (buf == nullptr || len == 0)
    return false;

  using namespace Cmd;
  if (buf->cmd == SCENE_USER_CARRIER_PROTOCMD)
  {
    switch (buf->param)
    {
      case MAPPARAM_FERRISWHEEL_INVITE:
      case MAPPARAM_FERRISWHEEL_PROCESSINVITE:
        forwardSession(buf, len);
        return true;
    }
  }

  return forwardScene(buf,len);
}

bool GateUser::forwardScene(const Cmd::UserCmd* cmd, unsigned short len)
{
  if (!scene_task()) return false;
  if (len > MAX_USER_DATA_SIZE)
    XERR << "[玩家消息]" << cmd->cmd << cmd->param << "消息太大:" << len << XEND;
  DWORD totalLen = len + sizeof(ForwardUserCmdGatewayCmd);
  BUFFER_CMD_SIZE(rev, ForwardUserCmdGatewayCmd, totalLen);
  rev->accid = accid;
  rev->len = len;
  bcopy(cmd, rev->data, (DWORD)len);
  return scene_task()->sendCmd(rev, totalLen);
}

bool GateUser::forwardSession(const Cmd::UserCmd* cmd, unsigned short len)
{
  if (!cmd) return false;
  if (len > MAX_USER_DATA_SIZE)
    XERR << "[玩家消息]" << cmd->cmd << cmd->param << "消息太大:" << len << XEND;
  DWORD totalLen = len+sizeof(ForwardUserCmdGatewayCmd);
  BUFFER_CMD_SIZE(rev, ForwardUserCmdGatewayCmd, totalLen);
  rev->accid = accid;
  rev->len = len;
  bcopy(cmd, rev->data, (DWORD)len);
  return thisServer->sendCmdToSession(rev, totalLen);
}

void GateUser::setState(GateUser_State state)
{
  m_oGateUserState = state;
  switch (m_oGateUserState)
  {
    case GateUser_State::create:
      XLOG << "[玩家状态设置]" << id << name << this << "创建" << XEND;
      break;
    case GateUser_State::login:
      XLOG << "[玩家状态设置]" << id << name << this << "登录" << XEND;
      break;
    case GateUser_State::select_role:
      XLOG << "[玩家状态设置]" << id << name << this << "选择角色" << XEND;
      break;
    case GateUser_State::running:
      XLOG << "[玩家状态设置]" << id << name << this << "运行" << XEND;
      break;
    case GateUser_State::quit:
      XLOG << "[玩家状态设置]" << id << name << this << "退出" << XEND;
      break;
    default:
      break;
  }
}

bool GateUser::doTowerUserCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  switch(buf->param)
  {
    case ETOWERPARAM_TEAMTOWERINFO:
    case ETOWERPARAM_INVITE:
    case ETOWERPARAM_REPLY:
    case ETOWERPARAM_ENTERTOWER:
      forwardSession(buf, len);
      break;
    case ETOWERPARAM_TOWERINFO:
      forwardScene(buf,len);
      break;
   default:
    break;
  }
  return true;
}

bool GateUser::doDojoUserCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  switch (buf->param)
  {
  case EDOJOPARAM_DOJO_PUBLIC_INFO:
  case EDOJOPARAM_PANEL_OPER:
  case EDOJOPARAM_REPLY:
  case EDOJOPARAM_QUERYSTATE:
  case EDOJOPARAM_ENTERDOJO:
    forwardSession(buf, len);
    break;
  case EDOJOPARAM_SPONSOR:
  case EDOJOPARAM_ADD_MSG:
  case EDOJOPARAM_DOJO_PRIVATE_INFO:
    forwardScene(buf, len);
    break;
  default:
    break;
  }
  return true;
}

bool GateUser::doChatUserCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  if (buf == nullptr)
    return false;

  switch (buf->param)
  {
    case CHATPARAM_PLAYEXPRESSION:
    case CHATPARAM_CHAT_RET:
      XLOG << "此消息应该是服务器发送给客户端的" << XEND;
      break;
    case CHATPARAM_CHAT_SELF:
      XLOG << "此消息应该是gate发送给scene的" << XEND;
      break;
    case CHATPARAM_QUERYUSERINFO:
    case CHATPARAM_QUERY_REALTIME_VOICE_ID:
      forwardSession(buf, len);
      break;
    case CHATPARAM_QUERY_VOICE:
    case CHATPARAM_QUERYITEMDATA:
    case CHATPARAM_BARRAGE:
    case CHATPARAM_GET_VOICEID:
      forwardScene(buf, len);
      break;
    case CHATPARAM_BARRAGEMSG:
      {
        if(ChatFilterManager::getMe().isBusy())
        {
          forwardScene(buf, len);
        }
        else
        {
          ChatFilterManager::getMe().checkFilter(this, buf, len);
        }
        break;
      }
    case CHATPARAM_CHAT:
      {
        PARSE_CMD_PROTOBUF(ChatCmd, cmd);
        if(ChatFilterManager::getMe().isBusy() || cmd.voice().size())
        {
          forwardScene(buf, len);
        }
        else
        {
          ChatFilterManager::getMe().checkFilter(this, buf, len);
        }
        break;
      }
    default:
      return false;
  }

  return true;
}

bool GateUser::doGuildCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  if (buf == nullptr)
    return false;

  switch (buf->param)
  {
    default:
      forwardSession(buf, len);
      break;
  }

  return true;
}

bool GateUser::doUserEventCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  switch(buf->param)
  {
    case USER_EVENT_CHARGE_QUERY:
    case USER_EVENT_QUERY_CHARGE_CNT:
    case USER_EVENT_LOVELETTER_USE:
      forwardSession(buf, len);
      break;
    case USER_EVENT_MAIL:
      break; //服务器内部使用
    default:
      forwardScene(buf, len);
      break;
  }
  return true;
}

bool GateUser::doWeddingUserCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  switch (buf->param)
  {
  case WEDDINGCPARAM_CHECK_CAN_RESERVE:
  case WEDDINGCPARAM_REQ_WEDDINGDATE_LIST:
  case WEDDINGCPARAM_RESERVE_WEDDINGDATE:   //预定婚礼，先发到场景处理
  case WEDDINGCPARAM_REPLY_RESERVE_WEDDINGDATE:
  case WEDDINGCPARAM_INVITE_WEDDING:
  case WEDDINGCPARAM_REPLY_WEDDING:
  case WEDDINGCPARAM_GOTO_WEDDINGPOS:
  case WEDDINGCPARAM_ANSWER:
  case WEDDINGCPARAM_QUESTION_SWITCH:
  case WEDDINGCPARAM_ENTER_ROLLER_COASTER:  //进火山车副本
  case WEDDINGCPARAM_DIVORCE_ROLLER_COASTER_INVITE:
  case WEDDINGCPARAM_DIVORCE_ROLLER_COASTER_REPLY:
  case WEDDINGCPARAM_REQ_DIVORCE:           //请求离婚
  case WEDDINGCPARAM_ENTER_WEDDINGMAP:
  case WEDDINGCPARAM_MISSYOU_INVITE:
  case WEDDINGCPARAM_MISSYOU_REPLY:
  case WEDDINGCPARAM_CARRIER:
    forwardScene(buf, len);
    break;
  default:
    forwardSession(buf, len);
    break;
  }
  return true;
}

bool GateUser::doPveCardCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  switch(buf->param)
  {
    case EPVE_INVITE_TEAM_CMD:
    case EPVE_REPLY_TEAM_CMD:
    case EPVE_ENTER_RAID_CMD:
      forwardSession(buf, len);
      break;
    default:
      forwardScene(buf, len);
      break;
  }

  return true;
}

bool GateUser::doTeamRaidCmd(const Cmd::UserCmd* buf, unsigned short len)
{
  switch(buf->param)
  {
    case TEAMRAIDPARAM_INVITE:
    case TEAMRAIDPARAM_REPLY:
    case TEAMRAIDPARAM_ENTER:
      forwardSession(buf, len);
      break;
    default:
      forwardScene(buf, len);
      break;
  }

  return true;
}

void GateUser::addCmdCounter(DWORD cmd, DWORD param)
{
  m_oCmdCounter[(cmd << 16) + param]++;
  ++m_dwTotalCount;
}

void GateUser::printCmdCounter()
{
  XERR << "[消息数量过多]" << id << name << "总数:" << m_dwTotalCount << XEND;
  for (auto &it : m_oCmdCounter)
    XERR << "[消息数量过多]" << id << name << "("<< (it.first >> 16) << (it.first & 0xffff) << "),数量:" << it.second << XEND;
}

void GateUser::syncServerTime2Client()
{
  ServerTimeUserCmd message;
  message.set_time(xTime::getCurMSec());
  PROTOBUF(message, send, len);
  sendCmdToMe(send, len);
}
