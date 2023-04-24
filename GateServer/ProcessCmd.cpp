#include <algorithm>
#include "GateServer.h"
#include "xlib/xSha1.h"
#include "xNetProcessor.h"
#include "GateUserManager.h"
#include "GateUser.h"
#include "RegCmd.h"
#include "GatewayCmd.h"
#include "IndexManager.h"
#include "LoginUserCmd.pb.h"
#include "ErrorUserCmd.pb.h"
#include "BaseConfig.h"
#include "GMTools.pb.h"
#include "ClientCmd.h"
#include "GateSuper.pb.h"

bool GateServer::doLoginUserCmd(xNetProcessor* np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;

  using namespace Cmd;

  Cmd::UserCmd *cmd = (Cmd::UserCmd *)buf;
  switch (cmd->param)
  {
    case KICK_PARAM_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::KickParamUserCmd, rev);

        LoginOutRegCmd send;
        send.id = rev.charid();
        send.accid = rev.accid();
        send.kick = true;
        thisServer->sendCmdToSession(&send,sizeof(send));

        return true;
      }
      break;
    case CREATE_CHAR_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::CreateCharUserCmd, message);

        GateUser *pLoginUser = GateUserManager::getMe().newLoginUser(np, message.accid(), message.version());
        if (!pLoginUser) return true;

        pLoginUser->setState(GateUser_State::login);
        pLoginUser->doUserCmd((Cmd::UserCmd *)buf, len);

        return true;
      }
      break;
    case DELETE_CHAR_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::DeleteCharUserCmd, message);

        GateUser *pLoginUser = GateUserManager::getMe().newLoginUser(np, message.accid(), message.version());
        if (!pLoginUser) return true;

        pLoginUser->setState(GateUser_State::login);
        pLoginUser->doUserCmd((Cmd::UserCmd *)buf, len);

        return true;
      }
      break;
    case SELECT_ROLE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::SelectRoleUserCmd, message);

        GateUser *pLoginUser = GateUserManager::getMe().newLoginUser(np, message.accid(), message.version());
        if (!pLoginUser) return true;

        pLoginUser->setState(GateUser_State::select_role);
        pLoginUser->m_oLoginTimer.elapseStart();
        pLoginUser->doUserCmd((Cmd::UserCmd *)buf, len);

        return true;
      }
      break;
    case GM_DELETE_CHAR_USER_CMD:
      {
#ifdef _DEBUG
        PARSE_CMD_PROTOBUF(Cmd::GMDeleteCharUserCmd, rev);

        GateUser* pUser = GateUserManager::getMe().getUserByAccID(rev.accid());
        if (pUser != nullptr)
          GateUserManager::getMe().loginOut(pUser, true);
#endif
      }
      return true;
    case SYNC_AUTHORIZE_GATE_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::SyncAuthorizeGateCmd, message);

        GateUser *pLoginUser = GateUserManager::getMe().newLoginUser(np, message.accid(), message.version());
        if (!pLoginUser) return true;

        pLoginUser->setState(GateUser_State::login);
        pLoginUser->doUserCmd((Cmd::UserCmd *)buf, len);

        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

bool GateServer::doRegCmd(xNetProcessor* np, BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  RegCmd *cmd = (RegCmd *)buf;

  switch (cmd->param)
  {
    case CREATE_OK_CHAR_REGCMD:
      {
        CreateOKCharRegCmd *rev = (CreateOKCharRegCmd *)buf;
        GateUser *pLoginUser = GateUserManager::getMe().getLoginUserByAccID(rev->accid);
        if (pLoginUser)
        {
          pLoginUser->setState(GateUser_State::select_role);

          Cmd::RefreshSnapshotClientUserCmd send;
          pLoginUser->sendCmdToMe(&send, sizeof(send));
        }
        return true;
      }
      break;
    case RET_DELETE_CHAR_REGCMD:
      {
        RetDeleteCharRegCmd *rev = (RetDeleteCharRegCmd *)cmd;
        GateUser *pLoginUser = GateUserManager::getMe().getLoginUserByAccID(rev->accid);
        if (pLoginUser)
        {
          if (rev->ret)
          {
            Cmd::RefreshSnapshotClientUserCmd send;
            pLoginUser->sendCmdToMe(&send, sizeof(send));
          }
          else
          {
            Cmd::RegErrUserCmd errCmd;
            errCmd.set_ret(REG_ERR_DELETE_ERROR);
            PROTOBUF(errCmd, errSend, errLen);
            pLoginUser->sendCmdToMe(errSend, errLen);
          }
        }
        return true;
      }
      break;
    case LOGIN_OUT_GATE_REGCMD:
      {
        LoginOutGateRegCmd *rev = (LoginOutGateRegCmd *)cmd;
        GateUser *gUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if (gUser)
        {
          GateUserManager::getMe().onUserQuit(gUser);
        }
        return true;
      }
      break;
    case RET_CREATE_CHAR_REGCMD:
      {
        RetCreateCharRegCmd *rev = (RetCreateCharRegCmd *)cmd;
        GateUser *pUser = GateUserManager::getMe().getLoginUserByAccID(rev->accid);
        if (pUser)
        {
          if (rev->ret)
          {
            Cmd::RegErrUserCmd errCmd;
            errCmd.set_ret(static_cast<RegErrRet>(rev->ret));

            PROTOBUF(errCmd, errSend, errLen);
            pUser->sendCmdToMe(errSend, errLen);
          }
        }
        return true;
      }
      break;
    case USER_RENAME_REGCMD:
      {
        UserRenameRegCmd *rev = (UserRenameRegCmd*)cmd;
        GateUser *pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if(pUser)
        {
          Cmd::RefreshSnapshotClientUserCmd send;
          pUser->sendCmdToMe(&send, sizeof(send));
        }
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

bool GateServer::doGatewayCmd(const BYTE* buf, WORD len)
{
  GatewayCmd *cmd = (GatewayCmd *)buf;
  switch (cmd->param)
  {
    case CMD_FILTER_GATEWAYCMD:
      {
        CmdFilterGatewayCmd *rev = (CmdFilterGatewayCmd *)cmd;

        if (!rev->cmd || !rev->param ||!rev->type)
        {
          return true;
        }

        Cmd::UserCmd command(rev->cmd, rev->param);
        switch (rev->type)
        {
          case 1:
            {
              m_oCmdFiler.insert(command);
              XLOG << "[消息过滤],禁止:" << rev->cmd << rev->param << "size:" << m_oCmdFiler.size() << XEND;
            }
            break;
          case 2:
            {
              m_oCmdFiler.erase(command);
              XLOG << "[消息过滤],解除:" << rev->cmd << rev->param << "size:" << m_oCmdFiler.size() << XEND;
            }
            break;
          default:
            break;
        }

        return true;
      }
      break;
    case BROADCAST_ONE_LEVEL_INDEX_GATEWAYCMD:
      {
        BroadcastOneLevelIndexGatewayCmd *rev = (BroadcastOneLevelIndexGatewayCmd *)cmd;

        OneLevelIndexManager::getMe().broadcastCmd(rev->indexT, rev->i, rev->data, rev->len, rev->exclude, rev->ip);
#ifdef _LX_DEBUG
     //   XDBG("[广播索引],indexT:%u, i:%u, cmd:%u, param:%u", rev->indexT, rev->i, *((BYTE *)rev->data), *((BYTE *)(rev->data + 1)));
#endif

        return true;
      }
      break;
    case SYN_TEAM_USERCMD_GATEWAYCMD:
      {
        SynTeamUserCmdGatewayCmd *rev = (SynTeamUserCmdGatewayCmd *)cmd;
        GateUser *pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if(pUser)
        {
          pUser->m_qwTeamIndex = rev->teamid;
        }
        return true;
      }
      break;
    case ADD_ONE_LEVEL_INDEX_GATEWAYCMD:
      {
        AddOneLevelIndexGatewayCmd *rev = (AddOneLevelIndexGatewayCmd *)cmd;
        GateUser* pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if(pUser)
        {
          OneLevelIndexManager::getMe().addIndex(rev->indexT, rev->i, pUser);
#ifdef _LX_DEBUG
       //   XDBG("[添加索引],indexT:%u, i:%u", rev->indexT, rev->i);
#endif
        }
        return true;
      }
      break;
    case DEL_ONE_LEVEL_INDEX_GATEWAYCMD:
      {
        DelOneLevelIndexGatewayCmd *rev = (DelOneLevelIndexGatewayCmd *)cmd;
        GateUser* pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if (pUser)
        {
          OneLevelIndexManager::getMe().remove(rev->indexT, rev->i, pUser);
#ifdef _LX_DEBUG
          XDBG << "[删除索引],indexT:" << rev->indexT << "i:" << rev->i << XEND;
#endif
        }
        return true;
      }
      break;
    case BROADCAST_TWO_LEVEL_INDEX_GATEWAYCMD:
      {
        BroadcastTwoLevelIndexGatewayCmd *rev = (BroadcastTwoLevelIndexGatewayCmd *)cmd;

        for (int i=0; i<rev->num; i++)
        {
          TwoLevelIndexManager::getMe().broadcastCmd(TWO_LEVEL_INDEX_TYPE_SCREEN, rev->i, rev->i2s[i], rev->data, rev->len, rev->filter);
        }
        return true;
      }
      break;
    case ADD_TWO_LEVEL_INDEX_GATEWAYCMD:
      {
        AddTwoLevelIndexGatewayCmd *rev = (AddTwoLevelIndexGatewayCmd *)cmd;
        GateUser* pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if(pUser)
          TwoLevelIndexManager::getMe().addIndex(TWO_LEVEL_INDEX_TYPE_SCREEN, rev->i, rev->i2, pUser);
        return true;
      }
      break;
    case DEL_TWO_LEVEL_INDEX_GATEWAYCMD:
      {
        DelTwoLevelIndexGatewayCmd *rev = (DelTwoLevelIndexGatewayCmd *)cmd;
        GateUser* pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if(pUser)
          TwoLevelIndexManager::getMe().remove(TWO_LEVEL_INDEX_TYPE_SCREEN, rev->i, rev->i2, pUser);
        return true;
      }
      break;
    case FORWARD_TO_USER_GATEWAY_CMD:
      {
        ForwardToUserGatewayCmd* rev = (ForwardToUserGatewayCmd*)cmd;
        GateUser *pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if (pUser)
        {
          pUser->sendCmdToMe(rev->data, rev->len);
          // XDBG("[转发消息],accid:%llu,cmd:%u,param:%u", rev->accid, *((BYTE *)rev->data), *((BYTE *)(rev->data + 1)));
          return true;
        }

        return true;
      }
      break;
    case FORWARD_TO_LOGIN_USER_GATEWAY_CMD:
      {
        ForwardToLoginUserGatewayCmd* rev = (ForwardToLoginUserGatewayCmd *)cmd;
        GateUser *pUser = GateUserManager::getMe().getLoginUserByAccID(rev->accid);
        if (pUser)
        {
          pUser->sendCmdToMe(rev->data, rev->len);
          return true;
        }

        return true;
      }
      break;
    case USER_DATA_GATEWAYCMD:
      {
        UserDataGatewayCmd *rev = (UserDataGatewayCmd *)cmd;
        GateUser *pUser = GateUserManager::getMe().getUserByAccID(rev->accid);
        if (!pUser)
        {
          XERR << "[同步玩家数据]" << rev->accid << "找不到玩家" << XEND;
          return true;
        }
        pUser->setState(GateUser_State::running);
        ServerTask *net = getConnectedServer("SceneServer", rev->data.sceneServerName);
        if (!net)
        {
          XERR << "[同步玩家数据]" << pUser->accid << pUser->id << pUser->name << "找不到玩家所在场景" << rev->data.sceneServerName << XEND;
          GateUserManager::getMe().kickUser(pUser);
          return true;
        }
        pUser->init(net, rev->data);

        XLOG << "[同步玩家数据]" << pUser->accid << pUser->id << pUser->name << "成功" << XEND;

        return true;
      }
      break;
    case GATEUSER_ONLINE_GATEWAYCMD:
      {
        GateUserOnlineGatewayCmd *rev = (GateUserOnlineGatewayCmd *)cmd;

        GateUser *pUser = GateUserManager::getMe().getLoginUserByAccID(rev->accid);
        if (!pUser)
        {
          XERR << "[完成登录]" << rev->accid << "找不到玩家" << XEND;
          return true;
        }

        GateUserManager::getMe().delLoginUser(pUser);
        if (!GateUserManager::getMe().addUser(pUser))
        {
          GateUserManager::getMe().kickUser(pUser);
          XERR << "[完成登录]" << pUser->accid << pUser->id << pUser->name << "添加到管理器失败" << XEND;
        }
        XLOG << "[完成登录]" << pUser->accid << pUser->id << pUser->name << "添加到管理器,耗时:" << pUser->m_oLoginTimer.uElapse() << "当前网关人数:" << (DWORD)GateUserManager::getMe().size() << XEND;

        return true;
      }
    case FORWARD_ALL_USER_GATEWAYCMD:
      {
        ForwardAllUserGatewayCmd *rev = (ForwardAllUserGatewayCmd *)cmd;

        GateUserManager::getMe().sendAllCmd(rev->data, rev->len);

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

bool GateServer::doErrorUserCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case REG_ERR_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(RegErrUserCmd, message);

        GateUser *pUser = GateUserManager::getMe().getLoginUserByAccID(message.accid());
        if (pUser)
        {
          pUser->sendCmdToMe((void *)buf, len);
          XLOG << "[登录错误]" << pUser->accid << "错误码:" << message.ret() << XEND;
          GateUserManager::getMe().kickUser(pUser);
        }
        return true;
      }
      break;
    case REG_KICK_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(KickUserErrorCmd, message);
        GateUser *pUser = GateUserManager::getMe().getUserByAccID(message.accid());
        if (pUser == nullptr)
          pUser = GateUserManager::getMe().getLoginUserByAccID(message.accid());
        if (pUser != nullptr)
        {
          XLOG << "[踢人错误]" << pUser->accid << "被踢下线" << XEND;
          GateUserManager::getMe().kickUser(pUser);
        }

        return true;
      }
    default:
      break;
  }

  return false;
}

//xNetProcessor* pGMNet = nullptr;
bool GateServer::doGmToolsCmd(xNetProcessor *np, const BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case EXEC_GM_CMD:
      {
        /*if (pGMNet != nullptr)
        {
          RetExecGMCmd cmd;
          cmd.set_ret("一个GM指令正在执行中");
          PROTOBUF(cmd, send, len);
          np->sendCmd(send, len);
          pGMNet = nullptr;
          return true;
        }*/
        GMNetProcesser* pGM = addGMCon(np);
        if (pGM == nullptr)
          return true;

        PARSE_CMD_PROTOBUF(ExecGMCmd, message);

        std::stringstream ss;
        ss.str("");
        ss << "r0!@#$%^&" << message.time() << message.serverid();
        if (!checkSha1(message.sign().c_str(), ss.str().c_str(), ss.str().size()))
        {
          return true;
        }
        if (thisServer->isOuter())
        {
          DWORD delay = abs((int)message.time() - (int)now());
          if (delay >= 60)
          {
            XLOG << "[GM],超时,time:" << message.time() << "now:" << now() << XEND;
            return true;
          }
        }

        message.set_conid(pGM->pNetProcess->tempid);       

        XLOG << "[GM],time:" << message.time() << "now:" << now() << XEND;
        XLOG << "[GM],act:" << message.act() << "data:" << message.data() << "sign=" << message.sign() <<"conid"<<message.conid() << XEND;
        
        PROTOBUF(message, buf, len);
        thisServer->sendCmdToSession(buf, len);
        return true;
      }
      break;
    case RET_EXEC_GM_CMD:
      {
        PARSE_CMD_PROTOBUF(RetExecGMCmd, message);
        GMNetProcesser* pGMNet = getGMCon(message.conid());
        if (pGMNet == nullptr || pGMNet->pNetProcess == nullptr)
        {
          XERR << "[GM] 找不到gm连接" << message.conid() << XEND;
          return true;
        }

        pGMNet->pNetProcess->sendCmd(buf, len);
      }
      return true;
    default:
      break;
  }

  return false;
}

bool GateServer::doGateSuperCmd(xNetProcessor *np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case GATEPARAM_FORWARD_TO_GATEUSER:
      {
        PARSE_CMD_PROTOBUF(ForwardToGateUserCmd, message);

        for (int i = 0; i < message.accids_size(); ++i)
        {
          GateUser *pUser = GateUserManager::getMe().getUserByAccID(message.accids(i));
          if (pUser)
          {
            pUser->sendCmdToMe((void *)message.data().c_str(), message.data().size());
          }
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}
