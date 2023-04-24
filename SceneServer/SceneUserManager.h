#pragma once

#include "xEntryManager.h"
#include "xSingleton.h"
#include "UserData.h"
#include "ConfigManager.h"

class SceneUser;
struct LoginOutSceneRegCmd;
class SceneUserDataLoad;
namespace Cmd
{
  class DeadBossInfo;
};

class SceneUserManager : public xEntryManager<xEntryID,xEntryTempID>, public xSingleton<SceneUserManager>
{
  friend class xSingleton<SceneUserManager>;
  public:
    virtual ~SceneUserManager();

    bool init();

    bool addUser(SceneUser* user);
    void delUser(SceneUser* user);
    SceneUser* getUserByID(QWORD uid);
    SceneUser* getUserByAccID(QWORD accid);

    bool addLoginUser(SceneUser* user);
    void delLoginUser(SceneUser* user);
    SceneUser* getLoginUserByID(QWORD uid);

    bool timer(QWORD curMSec);
    void load();
    void onUserQuit(SceneUser* user, UnregType type);

    void reloadconfig(ConfigType type);
    void syncWantedQuest();
    void syncDailyRate();

    void updateWeather(DWORD dwMapID, DWORD dwWeather);
    void updateSky(DWORD dwMapID, DWORD dwSky, DWORD sec);

    void setSvrMaxBaseLv(SceneUser* pUser);
    void setBaseWorldLevel(DWORD dwBaseWorldLevel) { m_dwBaseWorldLevel = dwBaseWorldLevel; }
    DWORD getBaseWorldLevel() { return m_dwBaseWorldLevel; }
    void setJobWorldLevel(DWORD dwJobWorldLevel) { m_dwJobWorldLevel = dwJobWorldLevel; }
    DWORD getJobWorldLevel() { return m_dwJobWorldLevel; }
    void updateGlobalBoss(const DeadBossInfo& rInfo);
  private:
    bool loadSvrMaxBaseLv();
    SceneUserManager();
    typedef std::map<QWORD,SceneUser*> LOGINLIST;
    LOGINLIST loginList;

    DWORD m_dwSvrMaxBaseLv = 0;

    DWORD m_dwBaseWorldLevel = 0;
    DWORD m_dwJobWorldLevel = 0;

  private:
    void login(SceneUserDataLoad *data);

  private:
    DWORD m_dwGroup = 0;
};

