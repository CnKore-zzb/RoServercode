#pragma once

#include "xSingleton.h"
#include "xEntryManager.h"
#include "SceneBooth.h"

class Scene;
class SceneUser;

class SceneBoothManager: private xEntryManager<xEntryTempID>, public xSingleton<SceneBoothManager>
{
  friend class xSingleton<SceneBoothManager>;

  public:
    SceneBoothManager();
    virtual ~SceneBoothManager();

  public:
    bool init();
    void final();
    void timer(QWORD curMSec);

    SceneBooth* getSceneBooth(QWORD charid);
    SceneBooth* createSceneBooth(Scene* scene, DWORD zondId, const Cmd::MapUser& mapUser);
    bool delBooth(QWORD charid);

  public:
    DWORD getCountBySceneId(DWORD sceneId)
    {
      if(m_mapScene.end() == m_mapScene.find(sceneId))
        return 0;

      return m_mapScene[sceneId];
    }

  private:
    bool addBooth(SceneBooth* item);
    bool delBooth(SceneBooth* item);

  private:
    std::map<DWORD, DWORD> m_mapScene; // <sceneid, size>
};
