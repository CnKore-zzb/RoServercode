#pragma once
#include "xDefine.h"
#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include "ClientManager.h"

using namespace Cmd;
class SessionUser;

class WorldLevelManager : public xSingleton<WorldLevelManager>
{
  friend class xSingleton<WorldLevelManager>;
  private:
    WorldLevelManager();
  public:
    virtual ~WorldLevelManager();

    void registRegion(ClientType type);
    void processSyncWorldLevelCmd(const SyncWorldLevelSessionCmd& cmd);
    void onUserOnline(SessionUser* pUser);

  public:
    DWORD getBaseWorldLevel() { return m_dwBaseWorldLevel; }
    DWORD getJobWorldLevel() { return m_dwJobWorldLevel; }

  private:
    DWORD m_dwBaseWorldLevel = 0;
    DWORD m_dwJobWorldLevel = 0;
};
