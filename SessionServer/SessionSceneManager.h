#pragma once
#include <list>
#include "xEntryManager.h"
#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include "ServerTask.h"

#define MIN_USERNUM_CHANGE_SCENE 200
#define MIN_DIFVALUE_CHANGE_SCENE 20

using std::map;

class SessionScene;
class SessionUser;
class xNetProcessor;


enum ESceneStat
{
  ESCENESTAT_NO = 0,
  ESCENESTAT_CREATING = 1,
  ESCENESTAT_OK = 2,
};

struct SSceneInfo
{
  DWORD dwSceneID = 0;
  DWORD dwRaidID = 0;
  DWORD dwTime = 0;
  QWORD qwRoomId = 0;

  ESceneStat stat = ESCENESTAT_NO;

  GuildInfo oGuildInfo;
  SSceneInfo() {}
};
typedef map<DWORD, SSceneInfo> TMapSceneInfo;

// scene manager
class SessionSceneManager : public xEntryManager<xEntryID, xEntryName>, public xSingleton<SessionSceneManager>
{
  friend class xSingleton<SessionSceneManager>;
  private:
    SessionSceneManager();
  public:
    virtual ~SessionSceneManager();

  public:
    void final();
    bool addScene(SessionScene* scene);
    void delScene(SessionScene* scene);
    SessionScene* getSceneByID(DWORD id);
    SessionScene* getSceneByName(const char *name);
    SessionScene* getSceneByGuildID(QWORD qwGuildID);
    SessionScene* createScene(DWORD id, const char* name, ServerTask* task);

    void delOffLineScene();
    SessionScene* mapRedirect(SessionScene *scene);

  public:
    DWORD createRaidMap(Cmd::CreateRaidMapSessionCmd &cmd, bool isRetry = false);
    DWORD getDMapIndex();
    void putDMapIndex(DWORD id);
    ServerTask* getASceneServer(QWORD userid = 0, const Cmd::RaidMapData* pRaidData = nullptr);

  public:
    void onRaidSceneOpen(SessionScene *scene);
    void onRaidSceneClose(SessionScene *scene);

    std::string generateGuildTeamKey(QWORD guildId, QWORD teamId);
    std::string genGuildRandRaidKey(QWORD guildid, QWORD teamid);
  public:
    void closeGuildRaidGroup(QWORD guildid, QWORD teamid, DWORD index);
  public:
    bool isMyPrivateRaid(QWORD charid, DWORD sceneid)
    {
      auto it = m_oPrivateRaidList.find(charid);
      if (it == m_oPrivateRaidList.end()) return false;

      for (auto &m : it->second)
      {
        if (m.second.dwSceneID == sceneid)
          return true;
      }
      return false;
    }

    DWORD getMapIDByDMap(DWORD id) const;

    void clearPvpRoom();
    void clearOnePvpRoom(SessionScene *pScene);

  private:
    void pushRetryList(QWORD sceneId, Cmd::CreateRaidMapSessionCmd& cmd);
    void popRetryList(QWORD sceneId);
    bool isSuperGvgRaid(const Cmd::RaidMapData* pRaidData) const;

  private:
    map<QWORD, TMapSceneInfo> m_oTeamRaidList;              // teamID sceneID
    map<QWORD, TMapSceneInfo> m_oPrivateRaidList;           // charID sceneID
    map<DWORD, TMapSceneInfo> m_oSystemRaidList;            // sceneID members
    map<QWORD, TMapSceneInfo> m_oGuildRaidList;             // guildID members
    map<QWORD, TMapSceneInfo> m_oUserTeamRaidList;          // userID sceneID, team can enter
    map<QWORD, TMapSceneInfo> m_oPvpRoomRaidList;
    map<std::string, TMapSceneInfo> m_oGuildTeamRaidList;   // guild team sceneID
    map<DWORD, DWORD> m_mapDMapRaid;
       
    std::map<QWORD/*sceneid*/, std::list<Cmd::CreateRaidMapSessionCmd>> m_mapRetryCmd;
    map<std::string, TMapSceneInfo> m_oGuildRandomRaidList; // randindex & guildteamid , sceneID
    map<DWORD, TMapSceneInfo> m_oGuildFireRaidList;   // guild city index
    map<QWORD, TMapSceneInfo> m_oWeddingRaidList;
};
