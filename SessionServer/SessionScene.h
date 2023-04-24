#pragma once
#include "xEntry.h"
#include "MapConfig.h"

class ServerTask;

struct RaidSceneData
{
  DWORD m_dwRaidID = 0;
  QWORD m_qwTeamID = 0;      // 如果是组队副本 为队伍的ID
  QWORD m_qwCharID = 0;      // 如果是单人副本 为创建者的ID
  QWORD m_qwGuildID = 0;
  DWORD m_dwLayer = 0;
  DWORD m_dwGuildRaidIndex = 0;
  //RaidMapRestrict m_oRestrict = RaidMapRestrict::Null;
  ERaidRestrict m_oRestrict = ERAIDRESTRICT_MIN;
  QWORD m_qwRoomId = 0;     // 如果是pvp 房间副本，为房间id
  std::set<QWORD> m_oMembers;
};

class SessionUser;

class SessionScene : public xEntry
{
  friend class SessionSceneManager;
  public:
    SessionScene(DWORD id, const char* name, ServerTask *task);
    virtual ~SessionScene();

    bool sendCmd(const void* cmd, unsigned short len) const;
    ServerTask * getTask() const { return task; }
    virtual void onClose();
    const char* getSceneServerName();
    bool init() { return true; }

    bool isDScene() { return m_oRaidData.m_dwRaidID != 0; }

    bool canEnter(SessionUser *pUser);

  public:
    RaidSceneData m_oRaidData;

  private:
    SessionScene();
    void final();
    ServerTask *task = nullptr;
};
