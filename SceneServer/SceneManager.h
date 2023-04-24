#pragma once

#include "xEntryManager.h"
#include "xSingleton.h"
#include "Scene.h"
#include "SessionCmd.pb.h"

using namespace Cmd;

class SceneManager : public xEntryManager<xEntryID, xEntryName>,public xSingleton<SceneManager>
{
  friend class xSingleton<SceneManager>;
  public:
    virtual ~SceneManager();
  private:
    SceneManager();

    // 场景管理器
  public:
    bool addScene(Scene* scene);
    void delScene(Scene* scene);
    void closeScene(Scene *scene);
    Scene* getSceneByID(DWORD sid);
    Scene* getSceneByName(const char *name);

    // 初始化  加载地图配置  加载静态地图
    bool init();
    bool loadSceneConfig();
    bool loadClientExport(SceneBase *base);
    bool loadSceneInfo(SceneBase *base);
    bool loadNavmesh(SceneBase *base);
    bool loadSceneSeat(SceneBase* base);

    // 创建一个地图(静态)
    Scene* createScene(SceneBase *base);
    Scene* createVirtualScene();

    // 副本地图
    bool createDScene(const CreateDMapParams& params);
    Scene* createDScene(const CreateRaidMapSessionCmd &data);

    DWORD getCreatureCount(SCENE_ENTRY_TYPE eType, DWORD dwMapID);

    void timer(QWORD curMSec);

    void registerAllMapWhenConnect();
    void closeSceneByType(SCENE_TYPE eType);

    // 配置
  public:
    const SceneBase* getDataByID(DWORD id)
    {
      auto it=m_baseCfg.find(id);
      if (it!=m_baseCfg.end())
        return it->second;
      return NULL;
    }
    std::map<DWORD, SceneBase *> m_baseCfg;

  public:
    const TSetDWORD& getMapByFlag(DWORD flagid) const;
    void getAllSceneByType(SCENE_TYPE eType, std::set<Scene*> scenes) const;
  private:
    std::map<DWORD, TSetDWORD> m_mapFlag2MapID; /*公会旗帜地图ID*/
  private:
    void clear();
    void checkSummonNpc(Scene* pScene);
  public:
    std::map<std::string, PathFindingTile *> m_oPathFindingList;
    
    std::map<DWORD/*raidid*/, xLuaData> m_mapDsceneSummon;
};

