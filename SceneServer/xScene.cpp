#include "xScene.h"
#include <algorithm>
#include "SceneStruct.h"
#include "SceneUser.h"
#include "xSceneEntryDynamic.h"

xScene::xScene()
{
  state = SCENE_STATE_CREATE;
}

xScene::~xScene()
{
}

