#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "ServerTask.h"
#include "SessionCmd.pb.h"
#include "Boss.h"
#include "SessionUserManager.h"
#include "SessionUser.h"

SessionScene::SessionScene(DWORD i, const char* n, ServerTask *t)
{
  set_id(i);
  set_name(n);
  task = t;
}

SessionScene::~SessionScene()
{
  final();
}

void SessionScene::final()
{
  XLOG << "[注销场景]," << id << "," << name << ",final" << XEND;
}

bool SessionScene::sendCmd(const void* cmd, unsigned short len)const
{
  if(!task) return false;
  return task->sendCmd(cmd, len);
}

const char* SessionScene::getSceneServerName()
{
  if(task)
    return task->getName();
  return NULL;
}

void SessionScene::onClose()
{
  Cmd::DeleteDMapSessionCmd message;
  message.set_mapid(id);
  PROTOBUF(message, send, len);
  sendCmd(send, len);

  BossList::getMe().onSceneClose(this);
  SessionUserManager::getMe().onSceneClose(this);

  if (m_oRaidData.m_oRestrict == ERAIDRESTRICT_TEAM || (m_oRaidData.m_oRestrict==ERAIDRESTRICT_USER_TEAM && m_oRaidData.m_qwTeamID))
  {
    Cmd::DelTeamRaidSocialCmd delmess;
    delmess.set_teamid(m_oRaidData.m_qwTeamID);
    delmess.set_raidid(m_oRaidData.m_dwRaidID);
    PROTOBUF(delmess, send2, len2);
    thisServer->sendCmdToServer(send2, len2, "TeamServer");
  }
}

bool SessionScene::canEnter(SessionUser *pUser)
{
  if (!pUser) return false;
  if (isDScene())
  {
    switch (m_oRaidData.m_oRestrict)
    {
      case ERAIDRESTRICT_SYSTEM://RaidMapRestrict::System:
        {
          if (m_oRaidData.m_oMembers.find(pUser->id)!=m_oRaidData.m_oMembers.end())
          {
            return true;
          }
          return false;
        }
        break;
      case ERAIDRESTRICT_TEAM://RaidMapRestrict::Team:
      case ERAIDRESTRICT_HONEYMOON:
        {
          if (pUser->getTeam().available() && pUser->getTeam().getTeamID() != m_oRaidData.m_qwTeamID)
          {
            return false;
          }
          return true;
        }
        break;
      case ERAIDRESTRICT_PRIVATE://RaidMapRestrict::Private:
        {
          if (pUser->getMapID() == id)
            return false;
          if (SessionSceneManager::getMe().isMyPrivateRaid(pUser->id, id))
            return true;

          return false;
        }
        break;
      case ERAIDRESTRICT_GUILD:
        return true;
      case ERAIDRESTRICT_GUILD_TEAM:
        return true;
      case ERAIDRESTRICT_USER_TEAM:
        return true;
      case ERAIDRESTRICT_PVP_ROOM:
        return true;
      case ERAIDRESTRICT_GUILD_RANDOM_RAID:
        return true;
      case ERAIDRESTRICT_GUILD_FIRE:
        return true;
      case ERAIDRESTRICT_WEDDING:
        return true;
      case ERAIDRESTRICT_MIN:
      case ERAIDRESTRICT_MAX:
        return false;
    }
    return false;
  }
  return true;
}
