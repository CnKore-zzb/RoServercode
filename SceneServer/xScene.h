#pragma once
#include "xEntry.h"
#include "SceneDefine.h"
#include "xSceneEntryIndex.h"
#include "xPos.h"

class xSceneEntryDynamic;

class xScene : public xEntry, public xSceneEntryIndex
{
  public:
    enum SceneState
    {
      SCENE_STATE_CREATE,
      SCENE_STATE_VERTIFY,
      SCENE_STATE_RUN,
      SCENE_STATE_WAIT_CLOSE,
      SCENE_STATE_PRE_CLOSE,
      SCENE_STATE_CLOSE
    };

    xScene();
    virtual ~xScene();
    virtual SCENE_TYPE getSceneType() const = 0;
    void setState(SceneState s) { state = s; }
    SceneState getState() { return state; }
    void regSucceed() { setState(xScene::SCENE_STATE_RUN); }

  private:
    SceneState state;
};
