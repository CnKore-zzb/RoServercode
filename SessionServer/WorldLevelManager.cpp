#include "WorldLevelManager.h"
#include "SessionUser.h"
#include "SessionServer.h"
#include "StatCmd.pb.h"

WorldLevelManager::WorldLevelManager()
{
}

WorldLevelManager::~WorldLevelManager()
{
}

void WorldLevelManager::registRegion(ClientType type)
{
  if (type == ClientType::stat_server)
  {
    ReqWorldLevelCmd message;
    message.set_zoneid(thisServer->getZoneID());
    PROTOBUF(message, send, len);
    thisServer->sendCmd(type, send, len);
  }
}

void WorldLevelManager::processSyncWorldLevelCmd(const SyncWorldLevelSessionCmd& cmd)
{
  m_dwBaseWorldLevel = cmd.base_worldlevel();
  m_dwJobWorldLevel = cmd.job_worldlevel();
  XLOG << "[世界等级-同步]" << "base世界等级" << m_dwBaseWorldLevel << "job世界等级" << m_dwJobWorldLevel;
}

void WorldLevelManager::onUserOnline(SessionUser* pUser)
{
  if (pUser == nullptr)
    return;
  SyncWorldLevelSessionCmd cmd;
  cmd.set_charid(pUser->id);
  cmd.set_base_worldlevel(m_dwBaseWorldLevel);
  cmd.set_job_worldlevel(m_dwJobWorldLevel);

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToScene(send, len);
}
