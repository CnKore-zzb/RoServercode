#include "SceneServer.h"
#include "RegCmd.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "xNetProcessor.h"

bool SceneServer::doRegCmd(xNetProcessor* np, BYTE* buf, WORD len)
{
  RegCmd *cmd = (RegCmd *)buf;
  switch (cmd->param)
  {
    case LOGIN_OUT_SCENE_REGCMD:
      {
        LoginOutSceneRegCmd *rev = (LoginOutSceneRegCmd *)cmd;

        SceneUser *pUser = SceneUserManager::getMe().getUserByID(rev->charid);
        if (pUser)
        {
          XLOG << "[注销]" << pUser->accid << pUser->id << pUser->name << "保存数据" << XEND;
          SceneUserManager::getMe().onUserQuit(pUser, rev->bDelete ? UnregType::Delete : UnregType::Normal);
          return true;
        }
        else
        {
          pUser = SceneUserManager::getMe().getLoginUserByID(rev->charid);
          if (pUser)
          {
            //还没有登录完成就退出，不保存
            XLOG << "[注销]" << pUser->accid << pUser->id << pUser->name << "登录完成前就注销" << XEND;

            SceneUserManager::getMe().delLoginUser(pUser);
            SAFE_DELETE(pUser);
          }
          else
          {
            XERR << "[注销]" << "charid:" << rev->charid << "没有找到玩家" << XEND;
          }
          thisServer->sendCmdToRecord(rev, sizeof(*rev));
        }

        return true;
      }
      break;
    default:
      break;
  }
  return true;
}
