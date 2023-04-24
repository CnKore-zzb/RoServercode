#pragma once

#include "xSingleton.h"
#include "SceneStruct.h"
#include "xEntryManager.h"
#include "SceneTrap.h"

class Scene;
class SceneUser;

class SceneTrapManager: private xEntryManager<xEntryTempID>, public xSingleton<SceneTrapManager>
{
  friend class xSingleton<SceneTrapManager>;

  public:
    SceneTrapManager();
    virtual ~SceneTrapManager();

    using xEntryManager<xEntryTempID>::size;

    bool init();
    void final();
    void timer(QWORD curMSec);

    SceneTrap* getSceneTrap(QWORD tempid);
    SceneTrap* createSceneTrap(Scene* scene, Cmd::SkillBroadcastUserCmd &cmd, QWORD masterID, const BaseSkill *skill);

  private:
    bool addTrap(SceneTrap* item);
    void delTrap(SceneTrap* item);
};

