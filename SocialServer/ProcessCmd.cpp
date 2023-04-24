#include "SocialServer.h"
#include "SocialCmd.pb.h"
#include "SocialityManager.h"
//#include "GuildManager.h"
#include "MiscConfig.h"
#include "StatisticsDefine.h"
#include "MailManager.h"
#include "PlatLogManager.h"
//#include "ChatManager_SO.h"
#include "TeamCmd.pb.h"
#include "GuildSCmd.pb.h"
#include "ConfigManager.h"
#include "MatchSCmd.pb.h"

bool SocialServer::doSocialCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case SOCIALPARAM_SESSION_FORWARD_SOCIAL_CMD:
      {
        PARSE_CMD_PROTOBUF(SessionForwardSocialCmd, rev);
        if (rev.type() == ECMDTYPE_SOCIALITY)
          SocialityManager::getMe().doUserCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        return true;
      }
      break;
    case SOCIALPARAM_ONLINESTATUS:
      {
        PARSE_CMD_PROTOBUF(OnlineStatusSocialCmd, rev);
        if (rev.online())
          SocialityManager::getMe().addLoad(rev.user());
        else
          SocialityManager::getMe().onUserOffline(rev.user());
      }
      return true;
    case SOCIALPARAM_USER_SYNC_INFO:
      {
        PARSE_CMD_PROTOBUF(UserInfoSyncSocialCmd, rev);
        SocialityManager::getMe().updateUserInfo(rev);
      }
      return true;
    case SOCIALPARAM_USER_DEL_CHAR:
      {
        PARSE_CMD_PROTOBUF(UserDelSocialCmd, rev);
        SocialityManager::getMe().delChar(rev.charid());
      }
      return true;
    case SOCIALPARAM_SOCIAL_DATA_UPDATE:
      {
        PARSE_CMD_PROTOBUF(SocialDataUpdateSocialCmd, rev);
        const SocialDataUpdate& rUpdate = rev.update();
        TSetQWORD setUpdateIDs;
        setUpdateIDs.insert(rev.charid());
        UserSociality* pSocial = SocialityManager::getMe().getUserSociality(rev.charid());
        if (pSocial == nullptr)
          return true;
        for (int i = 0; i < rUpdate.items_size(); ++i)
        {
          const SocialDataItem& rItem = rUpdate.items(i);
          SocialityManager::getMe().updateSocialData(rev.targetid(), setUpdateIDs, rItem.type(), rItem.value(), rItem.data());
          XDBG << "[社交-测试更新]" << rev.targetid() << rItem.ShortDebugString() << XEND;
        }
      }
      return true;
    case SOCIALPARAM_SOCIAL_ADDRELATION:
      {
        PARSE_CMD_PROTOBUF(AddRelationSocialCmd, rev);
        UserSociality* pUserSocial = SocialityManager::getMe().getUserSociality(rev.user().charid());
        if (pUserSocial != nullptr)
        {
          if (rev.relation() == ESOCIALRELATION_TUTOR_APPLY || rev.relation() == ESOCIALRELATION_STUDENT_APPLY)
            SocialityManager::getMe().addRelation(rev.destid(), rev.user().charid(), rev.relation(), rev.check());
          else
            SocialityManager::getMe().addRelation(rev.user().charid(), rev.destid(), rev.relation(), rev.check());
        }
        else
        {
          AddOfflineMsgSocialCmd cmd;
          OfflineMsg* pMsg = cmd.mutable_msg();
          pMsg->set_targetid(rev.user().charid());
          pMsg->set_senderid(rev.destid());
          pMsg->set_itemid(rev.relation());
          pMsg->set_type(EOFFLINEMSG_ADD_RELATION);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
      return true;
    case SOCIALPARAM_SOCIAL_REMOVERELATION:
      {
        PARSE_CMD_PROTOBUF(RemoveRelationSocialCmd, rev);
        UserSociality* pUserSocial = SocialityManager::getMe().getUserSociality(rev.user().charid());
        if (pUserSocial != nullptr)
        {
          SocialityManager::getMe().removeRelation(rev.user().charid(), rev.destid(), rev.relation(), false);
          if (rev.relation() == ESOCIALRELATION_TUTOR)
            SocialityManager::getMe().removeRelation(rev.destid(), rev.user().charid(), ESOCIALRELATION_STUDENT, false);
          else if (rev.relation() == ESOCIALRELATION_STUDENT)
            SocialityManager::getMe().removeRelation(rev.destid(), rev.user().charid(), ESOCIALRELATION_TUTOR, false);
        }
        else
        {
          AddOfflineMsgSocialCmd cmd;
          OfflineMsg* pMsg = cmd.mutable_msg();
          pMsg->set_targetid(rev.user().charid());
          pMsg->set_senderid(rev.destid());
          pMsg->set_itemid(rev.relation());
          pMsg->set_type(EOFFLINEMSG_REMOVE_RELATION);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
      return true;
    case SOCIALPARAM_SOCIAL_UPDATE_RELATIONTIME:
      {
        PARSE_CMD_PROTOBUF(UpdateRelationTimeSocialCmd, rev);
        UserSociality* pSociality = SocialityManager::getMe().getUserSociality(rev.charid());
        if (pSociality == nullptr)
        {
          XERR << "[社交]" << rev.charid() << "更新" << rev.targetid() << rev.relation() << "时间失败,未找到该玩家关系" << XEND;
          return true;
        }
        Sociality* pSocial = pSociality->getSociality(rev.targetid());
        if (pSocial == nullptr)
        {
          XERR << "[社交]" << rev.charid() << "更新" << rev.targetid() << rev.relation() << "时间失败,未找到目标玩家" << XEND;
          return true;
        }
        pSocial->setRelationTime(rev.relation(), rev.time());
        pSociality->addSaveID(rev.targetid());
      }
      return true;
    /*case SOCIALPARAM_SOCIAL_REMOVEFOCUS:
      {
        PARSE_CMD_PROTOBUF(RemoveFocusSocialCmd, rev);
        UserSociality* pUserSocial = SocialityManager::getMe().getUserSociality(rev.user().charid());
        if (pUserSocial != nullptr)
        {
          pUserSocial->removeFocus(rev.destid());
        }
        else
        {
          AddOfflineMsgSocialCmd cmd;
          OfflineMsg* pMsg = cmd.mutable_msg();
          pMsg->set_targetid(rev.user().charid());
          pMsg->set_senderid(rev.destid());
          pMsg->set_type(EOFFLINEMSG_REMOVE_FOCUS);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
      return true;*/
    case SOCIALPARAM_SOCIAL_REMOVESOCIAL:
      {
        PARSE_CMD_PROTOBUF(RemoveSocialitySocialCmd, rev);
        UserSociality* pUserSocial = SocialityManager::getMe().getUserSociality(rev.user().charid());
        if (pUserSocial != nullptr)
          pUserSocial->delSociality(rev.destid());
      }
      return true;
    case SOCIALPARAM_AUTHORIZE_SYNC_INFO:
      {
        PARSE_CMD_PROTOBUF(AuthorizeInfoSyncSocialCmd, rev);
        UserSociality* pUserSocial = SocialityManager::getMe().getUserSociality(rev.charid());
        if (pUserSocial != nullptr)
        {
          pUserSocial->setAuthorize(rev.ignorepwd());
        }
      }
      return true;
    default:
      return false;
  }

  return false;
}

bool SocialServer::doTeamCmd(BYTE* buf, WORD len)
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

        return true;
      }
      break;
    case SERVERTEAMPARAM_LOADLUA:
      {
        PARSE_CMD_PROTOBUF(LoadLuaTeamCmd, message);
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
              if (cfg.isTypeLoad(SOCIAL_LOAD))
                ConfigManager::getMe().loadConfig(cfg);
            }
          }
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (!cfg.isReload())
                continue;
              if (cfg.isTypeLoad(SOCIAL_LOAD))
                ConfigManager::getMe().checkConfig(cfg);
            }
          }
        }

        XLOG << "[策划表-重加载] zoneid:" << thisServer->getZoneID() << "lua:" << message.lua() << "table:" << message.table() << "log:" << message.log() << XEND;
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SocialServer::doGuildCmd(BYTE* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case GUILDSPARAM_USER_GUILDINFO_SYNC:
      {
        PARSE_CMD_PROTOBUF(UserGuildInfoSyncGuildSCmd, message);
        SocialityManager::getMe().updateSocialData(message.charid(), ESOCIALDATA_GUILDNAME, 0, message.guildname());
        SocialityManager::getMe().updateSocialData(message.charid(), ESOCIALDATA_GUILDPORTRAIT, 0, message.guildportrait());

        return true;
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SocialServer::doSessionCmd(const BYTE* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case SESSIONPARAM_GLOBALACTIVITY_START:
      {
        PARSE_CMD_PROTOBUF(GlobalActivityStartSessionCmd, rev);
        if (rev.id() == static_cast<DWORD>(GACTIVITY_RECALL))
          SocialityManager::getMe().setRecallActivityOpen(true);
      }
      break;
    case SESSIONPARAM_GLOBALACTIVITY_STOP:
      {
        PARSE_CMD_PROTOBUF(GlobalActivityStopSessionCmd, rev);
        if (rev.id() == static_cast<DWORD>(GACTIVITY_RECALL))
          SocialityManager::getMe().setRecallActivityOpen(false);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SocialServer::doMatchCmd(const BYTE* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case MATCHSPARAM_TUTOR_OPT:
      {
        PARSE_CMD_PROTOBUF(TutorOptMatchSCmd, rev);

        do
        {
          if (rev.opt() == ETUTOROPT_APPLY)
          {
            UserSociality* pSociality = SocialityManager::getMe().getUserSociality(rev.studentid());
            if (pSociality == nullptr)
            {
              rev.set_result(false);
              XERR << "[导师匹配-进度] 处理" << rev.ShortDebugString() << "失败,学生不在线" << XEND;
              break;
            }
            Sociality* pSocial = pSociality->getSociality(rev.tutorid());
            if (pSocial != nullptr && pSocial->checkRelation(ESOCIALRELATION_TUTOR_APPLY) == true)
            {
              rev.set_result(true);
              break;
            }
            pSociality->setTutorMatch(true);
            rev.set_result(SocialityManager::getMe().addRelation(rev.tutorid(), rev.studentid(), ESOCIALRELATION_TUTOR_APPLY));
            pSociality = SocialityManager::getMe().getUserSociality(rev.studentid());
            if (pSociality != nullptr)
              pSociality->setTutorMatch(false);
          }
          else if (rev.opt() == ETUTOROPT_AGREE)
          {
            rev.set_result(SocialityManager::getMe().addRelation(rev.studentid(), rev.tutorid(), ESOCIALRELATION_TUTOR));
            if (rev.result() == true)
            {
              UserSociality* pSociality = SocialityManager::getMe().getUserSociality(rev.studentid());
              if (pSociality != nullptr && pSociality->getFrame() == false)
              {
                pSociality->setFrame(true);
                pSociality->setFrame(false);
              }
            }
          }
        } while (0);

        rev.set_ret(true);
        PROTOBUF(rev, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      break;
    default:
      return false;
  }

  return true;
}

