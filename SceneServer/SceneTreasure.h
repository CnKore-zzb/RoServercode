/**
 * @file SceneTreasure.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-09-07
 */

#pragma once

#include "xEntry.h"
#include "TreasureConfig.h"
#include "SceneDefine.h"

using std::set;
class SceneNpc;
class SceneUser;
class Scene;
class TreeNpc;

typedef set<SceneNpc*> TSetTree;

struct STreeGroup
{
  DWORD dwNextTime = 0;
  TSetTree setTree;
  TSetDWORD setIndex;

  STreeGroup() {}
};

class SceneTreasure// : public xEntry
{
  public:
    SceneTreasure(Scene* pScene, const STreasureCFG* pCFG);
    ~SceneTreasure();

    void onTreeDie(SceneNpc* pNpc);
    void onEnterScene(SceneUser* pUser);
    void onLeaveScene(SceneUser* pUser);
    void timer(DWORD curTime);

    void shakeTree(SceneUser* pUser, QWORD qwNpcID);
    void sendTreeList(SceneUser* pUser);
    void goOtherPos(TreeNpc* npc);
  private:
    Scene* m_pScene = nullptr;
    const STreasureCFG* m_pCFG = nullptr;

    STreeGroup m_stGoldTree;
    STreeGroup m_stMagicTree;
    STreeGroup m_stHighTree;
};

