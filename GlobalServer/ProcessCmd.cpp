#include "GlobalServer.h"
#include "SocialCmd.pb.h"
#include "GCharManager.h"
//#include "SocialityManager.h"
#include "GlobalUserManager.h"
#include "MiscConfig.h"
#include "StatisticsDefine.h"
#include "MailManager.h"
#include "PlatLogManager.h"
#include "TeamCmd.pb.h"
#include "WeddingSCmd.pb.h"
#include "BossSCmd.pb.h"

bool GlobalServer::doSocialCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case SOCIALPARAM_SESSION_FORWARD_SOCIAL_CMD:
      {
        PARSE_CMD_PROTOBUF(SessionForwardSocialCmd, rev);
        if (rev.type() == ECMDTYPE_CHAT)
        {
          /*GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.user().charid());
          if (pUser != nullptr)
            pUser->doChatCmd((const BYTE*)rev.data().c_str(), rev.len());
          else
            XERR << "[社交消息-聊天] charid :" << rev.user().charid() << "不在线" << XEND;*/
        }
      }
      break;
    case SOCIALPARAM_ONLINESTATUS:
      {
        PARSE_CMD_PROTOBUF(OnlineStatusSocialCmd, rev);
        if (rev.online())
          GlobalUserManager::getMe().onUserOnline(rev.user());
        else
          GlobalUserManager::getMe().onUserOffline(rev.user());
      }
      break;
    case SOCIALPARAM_SEND_MAIL:
      {
        PARSE_CMD_PROTOBUF(SendMailSocialCmd, rev);

        {
          const BYTE* buf = (const BYTE*)rev.data().c_str();
          DWORD len = rev.len();
          PARSE_CMD_PROTOBUF(SendMail, rev2);
          //if (rev2.data().type() == EMAILTYPE_NORMAL || rev2.data().type() == EMAILTYPE_TRADE)
          //  MailManager::getMe().insertNormalMailToDB(rev2.mutable_data());
          //else if (rev2.data().type() == EMAILTYPE_SYSTEM)
          //  MailManager::getMe().insertSysMailToDB(rev2.mutable_data());
          //else
          //  return true;

          const MailData& rData = rev2.data();
          PROTOBUF(rev2, send2, len2);
          if (rData.type() == EMAILTYPE_SYSTEM)
          {
            thisServer->sendCmdToAllZone(send2, len2);
          }
          else
          {
            GlobalUser* user = GlobalUserManager::getMe().getGlobalUser(rData.receiveid());
            if (user)
              thisServer->sendCmdToZone(user->zoneid(), send2, len2);
          }
        }
      }
      break;
    case SOCIALPARAM_GLOBAL_FORWARD_CMD:
      {
        PARSE_CMD_PROTOBUF(GlobalForwardCmdSocialCmd, rev);
        XDBG << "[消息-中转] 收到中转消息，" << rev.charid() << XEND;
        GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[消息-中转] 收到中转消息，接收人不在线" << rev.charid() << XEND;
          return false;
        }

        if (thisServer->sendCmdToZone(pUser->zoneid(), (const BYTE*)rev.data().c_str(), rev.len()) == false)
        //if (thisServer->sendCmdToZone(pUser->zoneid(), buf, len) == false)
        {
          XERR << "[消息-中转] 收到中转消息，发送消息给接收人失败" << rev.charid()<< "zoneid" << pUser->zoneid() << XEND;
          return false;
        }
        XLOG << "[消息-中转] 收到中转消息，发送成功" << rev.charid()<<"zoneid"<<pUser->zoneid() << XEND;
        return true;
    }
      break;
    case SOCIALPARAM_GLOBAL_FORWARD_CMD2:
    {
      PARSE_CMD_PROTOBUF(GlobalForwardCmdSocialCmd2, rev);
      XDBG << "[消息-中转] 收到中转消息，" << rev.charid() << XEND;
      GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.charid());
      if (pUser == nullptr)
      {
        XERR << "[消息-中转] 收到中转消息，接收人不在线" << rev.charid() << XEND;
        return false;
      }

      if (thisServer->sendCmdToZone(pUser->zoneid(), (const BYTE*)rev.data().c_str(), rev.len()) == false)
      {
        XERR << "[消息-中转-交易] 收到中转消息，发送消息给接收人失败" << rev.charid() << "zoneid" << pUser->zoneid() << XEND;
        return false;
      }
      XLOG << "[消息-中转-交易] 收到中转消息，发送成功" << rev.charid() << "zoneid" << pUser->zoneid() << XEND;
      return true;
    }
    break;
    case SOCIALPARAM_FORWARD_TO_ALL_SESSION:
      {
        PARSE_CMD_PROTOBUF(ForwardToAllSessionSocialCmd, rev);
        thisServer->sendCmdToAllZone((const void *)rev.data().c_str(), rev.len(), rev.except());
      }
      break;
    case SOCIALPARAM_CHAT_MSG:
      {
        PARSE_CMD_PROTOBUF(ChatSocialCmd, rev);
        GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.ret().id());
        if (pUser != nullptr)
          pUser->doSocialCmd(buf, len);
        else
          XERR << "[社交消息-聊天] charid :" << rev.ret().id() << "不在线" << XEND;
      }
      break;
    /*case SOCIALPARAM_USER_QUERY_INFO:
      {
        PARSE_CMD_PROTOBUF(UserQueryInfoSocialCmd, rev);
        UserInfoSyncSocialCmd cmd;
        cmd.mutable_info()->CopyFrom(rev.info());
        SocialityManager::getMe().updateUserInfo(cmd);
        return true;
      }
      break;*/
    case SOCIALPARAM_SOCIAL_DATA_UPDATE:
      {
        PARSE_CMD_PROTOBUF(SocialDataUpdateSocialCmd, rev);
        GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.charid());
        if (pUser != nullptr)
        {
          rev.set_to_global(false);
          PROTOBUF(rev, send, len1);
          thisServer->sendCmdToZone(pUser->zoneid(), send, len1);
        }
      }
      break;
    case SOCIALPARAM_SOCIAL_ADDRELATION:
      {
        PARSE_CMD_PROTOBUF(AddRelationSocialCmd, rev);
        rev.set_to_global(false);
        PROTOBUF(rev, send, len1);
        GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.user().charid());
        if (pUser != nullptr)
          thisServer->sendCmdToZone(pUser->zoneid(), send, len1);
        else
          thisServer->sendCmdToZone(rev.user().zoneid(), send, len1);
      }
      break;
    case SOCIALPARAM_SOCIAL_REMOVERELATION:
      {
        PARSE_CMD_PROTOBUF(RemoveRelationSocialCmd, rev);
        rev.set_to_global(false);
        PROTOBUF(rev, send, len1);
        GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.user().charid());
        if (pUser != nullptr)
          thisServer->sendCmdToZone(pUser->zoneid(), send, len1);
        else
          thisServer->sendCmdToZone(rev.user().zoneid(), send, len1);
      }
      break;
    /*case SOCIALPARAM_SOCIAL_REMOVEFOCUS:
      {
        PARSE_CMD_PROTOBUF(RemoveFocusSocialCmd, rev);
        rev.set_to_global(false);
        PROTOBUF(rev, send, len1);
        GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.user().charid());
        if (pUser != nullptr)
          thisServer->sendCmdToZone(pUser->zoneid(), send, len1);
        else
          thisServer->sendCmdToZone(rev.user().zoneid(), send, len1);
      }
      break;*/
    case SOCIALPARAM_ADD_OFFLINEMSG:
    {
      PARSE_CMD_PROTOBUF(AddOfflineMsgSocialCmd, rev);
      GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.msg().targetid());
      if (pUser)
        return thisServer->sendCmdToZone(pUser->zoneid(), buf, len);
        
      return thisServer->sendCmdToOneZone(buf, len);
    }
    return true;

    case SOCIALPARAM_SOCIAL_REMOVESOCIAL:
      {
        PARSE_CMD_PROTOBUF(RemoveSocialitySocialCmd, rev);
        rev.set_to_global(false);
        PROTOBUF(rev, send, len1);
        GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.destid());
        if (pUser != nullptr)
          thisServer->sendCmdToZone(pUser->zoneid(), send, len1);
      }
      return true;

    case SOCIALPARAM_SYNC_TUTOR_REWARD:
    {
      PARSE_CMD_PROTOBUF(SyncTutorRewardSocialCmd, rev);
      rev.set_searchuser(false);
      PROTOBUF(rev, send, len1);
      GlobalUser* pUser = GlobalUserManager::getMe().getGlobalUser(rev.charid());
      if (pUser)
      {
        thisServer->sendCmdToZone(pUser->zoneid(), send, len1);
      }
      else
      {
        GlobalUser* student = GlobalUserManager::getMe().getGlobalUser(rev.reward().charid());
        if (student)
          thisServer->sendCmdToZone(student->zoneid(), send, len1);
        else
          thisServer->sendCmdToOneZone(send, len1);
      }
    }
    return true;
    default:
      return false;
  }

  return true;
}

/*bool GlobalServer::doTeamCmd(BYTE* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case SERVERTEAMPARAM_SOCIAL_ADDRELATION:
      {
        PARSE_CMD_PROTOBUF(AddRelationTeamCmd, rev);
        SocialityManager::getMe().addRelation(rev.charid(), rev.targetid(), ESOCIALRELATION_TEAM);
      }
      break;
    default:
      return false;
  }

  return true;
}*/

bool GlobalServer::doWeddingCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    case WEDDINGSPARAM_MISSYOU_INVITE:
      {
        PARSE_CMD_PROTOBUF(MissyouInviteWedSCmd, rev);
        GlobalUser* pTarget = GlobalUserManager::getMe().getGlobalUser(rev.charid());
        if (pTarget == nullptr)
        {
          XERR << "[婚姻-好想你]" << rev.charid() << "被配偶邀请回到身边失败,自身不在线" << XEND;
          break;
        }
        thisServer->sendCmdToZone(pTarget->zoneid(), buf, len);
        XLOG << "[婚姻-好想你]" << rev.charid() << "被配偶邀请回到身边,成功发送至该线" << pTarget->zoneid() << "处理" << XEND;
      }
      break;
    default:
      return false;
  }

  return true;
}

bool GlobalServer::doBossSCmd(BYTE* buf, WORD len)
{
  if (!buf || !len)
    return false;

  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    case BOSSSPARAM_DEADBOSS_OPEN:
      {
        PARSE_CMD_PROTOBUF(DeadBossOpenBossSCmd, rev);
        updateGlobalBoss(rev.info());
      }
      break;
    default:
      return false;
  }

  return true;
}
