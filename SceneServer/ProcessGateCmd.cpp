#include "SceneServer.h"
#include "GatewayCmd.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "UserCmd.h"

bool SceneServer::doGatewayCmd(const BYTE* buf, WORD len)
{
  GatewayCmd* cmd = (GatewayCmd*)buf;
  switch (cmd->param)
  {
    case FORWARD_USERCMD_GATEWAYCMD:
      {
        ForwardUserCmdGatewayCmd* rev = (ForwardUserCmdGatewayCmd*)cmd;
        Cmd::UserCmd *cmd = (Cmd::UserCmd *)rev->data;

        m_oMessageStat.start(cmd->cmd, cmd->param);

        SceneUser *pUser = SceneUserManager::getMe().getUserByAccID(rev->accid);
        if (pUser)
        {
          if (!pUser->doUserCmd(cmd, rev->len))
          {
            XERR << "[消息]" << pUser->accid << pUser->id << pUser->name << "处理错误" << cmd->cmd << cmd->param << XEND;
          }
          else
          {
      //      XLOG("[消息],%llu,%s,处理玩家消息 %u,%u",pUser->id, pUser->name, ((xCommand*)rev->data)->cmd, ((xCommand*)rev->data)->param);
          }
        }
        else
        {
          XERR << "[消息处理]" << cmd->cmd << cmd->param << "找不到玩家,accid:" << rev->accid << XEND;
        }
        m_oMessageStat.end();

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}
