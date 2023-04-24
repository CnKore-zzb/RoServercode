#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "SceneMap.pb.h"

struct SBoothUser
{
  QWORD m_qwCharId;
  DWORD m_dwZoneId;
  DWORD m_dwSceneId;
  Cmd::MapUser mapUser;
};

class BoothManager : public xSingleton<BoothManager>
{
  public:
    BoothManager();
    ~BoothManager();

  public:
    void init();
    bool loadConfig();

  public:
    bool addMapUser(DWORD zoneId, DWORD sceneId, const Cmd::MapUser& data);
    bool updateMapUser(const Cmd::MapUser& data);
    bool delMapUser(QWORD charId);
    bool hasMapUser(QWORD charId);

  private:
    std::map<QWORD, SBoothUser> m_mapChar2User;
};
