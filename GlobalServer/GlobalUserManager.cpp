#include "GlobalUserManager.h"
#include "UserCmd.h"
#include "xCommand.h"
#include "xTime.h"
#include "SysMsg.pb.h"
#include "GlobalServer.h"
#include "StatisticsDefine.h"
#include "PlatLogManager.h"

// GlobalUser
GlobalUser::GlobalUser(const SocialUser& rUser) : m_oUser(rUser)
{

}

GlobalUser::~GlobalUser()
{

}

bool GlobalUser::doSocialCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
   return false;

  switch (cmd->param)
  {
    case SOCIALPARAM_CHAT_MSG:
      {
        PARSE_CMD_PROTOBUF(ChatSocialCmd, rev);

        // process chat
        switch (rev.ret().channel())
        {
          case ECHAT_CHANNEL_FRIEND:
            {
              GlobalUser* pTarget = GlobalUserManager::getMe().getGlobalUser(rev.ret().targetid());
              if (pTarget == nullptr)
              {
                AddOfflineMsgSocialCmd offcmd;
                offcmd.mutable_msg()->set_targetid(m_oUser.charid());
                offcmd.mutable_msg()->mutable_chat()->CopyFrom(rev.ret());
                offcmd.mutable_msg()->set_type(EOFFLINEMSG_USER);
                PROTOBUF(offcmd, offsend, offlen);
                thisServer->sendCmdToZone(m_oUser.zoneid(), offsend, offlen);

                ChatRetCmd cmd;
                cmd.set_id(m_oUser.charid());
                cmd.set_targetid(rev.ret().targetid());
                cmd.set_channel(rev.ret().channel());
                cmd.set_msgid(ESYSTEMMSG_ID_CHAT_OFFLINE);
                PROTOBUF(cmd, send, len);
                thisServer->sendCmdToMe(m_oUser.zoneid(), m_oUser.charid(), send, len);
              }
              else
              {
                rev.set_to_global(false);
                PROTOBUF(rev, send, len);
                thisServer->sendCmdToZone(pTarget->zoneid(), send, len);
              }            
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      return false;
  }

  return true;
}

// GlobalUserManager
GlobalUserManager::GlobalUserManager()
{

}

GlobalUserManager::~GlobalUserManager()
{

}

void GlobalUserManager::onUserOnline(const SocialUser& rUser)
{
  auto m = m_mapGlobalUser.find(rUser.charid());
  if (m != m_mapGlobalUser.end())
    return;

  GlobalUser* pUser = NEW GlobalUser(rUser);
  if (pUser == nullptr)
    return;
  m_mapGlobalUser[rUser.charid()] = pUser;
}

void GlobalUserManager::onUserOffline(const SocialUser& rUser)
{
  auto m = m_mapGlobalUser.find(rUser.charid());
  if (m == m_mapGlobalUser.end())
    return;

  GlobalUser* pUser = m->second;
  m_mapGlobalUser.erase(m);
  SAFE_DELETE(pUser);
}

GlobalUser* GlobalUserManager::getGlobalUser(QWORD qwCharID)
{
  auto m = m_mapGlobalUser.find(qwCharID);
  if (m != m_mapGlobalUser.end())
    return m->second;
  return nullptr;
}

