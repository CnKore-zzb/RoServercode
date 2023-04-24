#pragma once

#include "xPool.h"
#include "xSingleton.h"
#include "SceneStruct.h"
#include "xEntryManager.h"
#include "NpcConfig.h"
#include "ConfigManager.h"
#include "LuaManager.h"

class Scene;
class SceneNpc;

struct NpcDefine;

#define NPC_GROUP 4

class SceneNpcManager: private xEntryManager<xEntryTempID>, public xSingleton<SceneNpcManager>
{
  friend class xSingleton<SceneNpcManager>;
  friend class Scene;
  public:
    SceneNpcManager();
    virtual ~SceneNpcManager();

    using xEntryManager<xEntryTempID>::size;

    bool init();
    void final();
    void timer(QWORD curMSec);
    void reload();
    void reloadconfig(ConfigType type);

    bool addObject(SceneNpc* pNpc);
    void removeObject(SceneNpc* pNpc);

    SceneNpc* getNpcByTempID(const UINT tempId);
    SceneNpc* createNpc(const NpcDefine& define, Scene* pScene);
    SceneNpc* createNpcLua(SLuaParams& luaParam);
    UINT createNpc(Scene* scene, const NpcDefine& define, UINT num = 1);

    bool deadNpcTimer(const DWORD curTime);
    void foreach(xEntryCallBack& callback) { forEach(callback);}
    void delUniqueID(Scene* scene, DWORD dwUniqueID);
    void addFoodNpcCount(QWORD charId);
    DWORD getFoodNpcCount(QWORD charId);

    const std::set<SceneNpc *>& getLeaveNpcs() { return m_oLeaveSceneNpc; }
  private:
    bool addNpc(SceneNpc* npc);
    void delNpc(SceneNpc* npc);
  public:
    DWORD getGroup() const
    {
      return m_dwGroup;
    }
    void addGroup()
    {
      m_dwGroup = (m_dwGroup + 1) % NPC_GROUP;
    }
  private:
    DWORD m_dwGroup = 0;

  public:
    void addLeaveSceneNpc(SceneNpc *npc);
    void delLeaveSceneNpc(SceneNpc *npc);

    bool addChangeBodyNpc(QWORD npcid, DWORD buffid);
    bool delChangeBodyNpc(QWORD npcid);
    DWORD getChangeBodyID(QWORD npcid);
  private:
    std::set<SceneNpc *> m_oLeaveSceneNpc;
    std::map<QWORD/*charid*/, DWORD/*count*/> m_mapFoodCount;
    std::map<QWORD/*npcid*/, DWORD/*buffid*/> m_mapChangeBody;
};

