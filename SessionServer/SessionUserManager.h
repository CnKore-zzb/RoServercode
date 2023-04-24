#pragma once

#include "xEntryManager.h"
#include "xSingleton.h"
#include "xCommand.h"
#include "ClientManager.h"
#include "ActivityCmd.pb.h"

using std::string;
using std::map;
using std::vector;

class SessionUser;
class ServerTask;
class SessionScene;
namespace Cmd
{
  class DeadBossInfo;
};

typedef map<QWORD, TSetQWORD> TMapSysMailOnlineCache;

#define SEND_USERCOUNT_INTERVAL 5

class SessionUserManager : public xEntryManager<xEntryID, xEntryTempID>,public xSingleton<SessionUserManager>
{
  public:
    SessionUserManager();
    virtual ~SessionUserManager();

    bool addUser(SessionUser* user);
    void delUser(SessionUser* user);

    SessionUser* getUserByAccID(QWORD accid);
    SessionUser* getUserByID(QWORD uid);

    void onUserQuit(SessionUser* user);
    void removeUserOnScene(SessionScene *pScene);

    void loginOutGate(QWORD accid, ServerTask *gt);

    void timer(DWORD curTime);
    void onOneMinTimeUp(QWORD curSec);
    void onFiveMinTimeUp(QWORD curSec);

    template<class T> void foreach(T func)
    {
      for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
        func(m->second);
    }

    void onSceneClose(SessionScene *scene);
    void registRegion(ClientType type);
    void onRegistServer(const string& svrname);

    void addSysMailCache(QWORD qwMailID, QWORD qwCharID);
    void removeCharIDFromSysCache(QWORD qwCharID);

    void addActivityProgress(DWORD activityid, DWORD progress, DWORD starttime = 0, DWORD endtime = 0);
    void delActivityProgress(DWORD activityid, DWORD progress);
    void sendActivityProgress(SessionUser* pUser);

    void updateGlobalBoss(const DeadBossInfo& rInfo);
    void syncGlobalBossScene(ServerTask *task = nullptr);
  private:
    /*打印在线玩家的数目*/
    void logOnlineUser();

    void processSysMail(DWORD curTime);

    xTimer m_oOneMinTimer;

    typedef std::map<DWORD/*区id*/, std::set<QWORD/*charid*/>>  ONLINEUSER_ZONE_MAP_T;
    typedef std::map<DWORD/*渠道id*/, ONLINEUSER_ZONE_MAP_T>  ONLINEUSER_ALL_MAP_T;
    ONLINEUSER_ALL_MAP_T m_oOnlineUserList;

    TMapSysMailOnlineCache m_mapSysMailOnlineCache;
    DWORD m_dwSysOnlineCacheTick = 0;
    std::map<DWORD, ActivityProgress> m_mapActivityProgress;
    DWORD m_dwLastLogTime = 0;
};
