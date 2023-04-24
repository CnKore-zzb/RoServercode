#pragma once

#include "xSingleton.h"
#include "SceneStruct.h"
#include "xEntryManager.h"
#include "SceneAct.h"

class Scene;
class SceneUser;

class SceneActManager: private xEntryManager<xEntryTempID>, public xSingleton<SceneActManager>
{
  friend class xSingleton<SceneActManager>;

  public:
    SceneActManager();
    virtual ~SceneActManager();

    using xEntryManager<xEntryTempID>::size;

    bool init();
    void final();
    void timer(DWORD sec);

    //bool loadConfig();

    SceneActBase* getSceneAct(QWORD tempid);
    SceneActBase* createSceneAct(Scene* scene, xPos pos, DWORD range, QWORD masterID, EActType acttype);

    void addEffectCount(QWORD charId);
    DWORD getEffectCount(QWORD charId);

  public:
    void delSceneAct(SceneActBase* act);
  private:
    bool addSceneAct(SceneActBase* act);
    std::map<QWORD/*charid*/, DWORD/*count*/> m_mapSceneEffectCount;
};

