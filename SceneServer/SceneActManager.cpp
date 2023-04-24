#include "SceneActManager.h"
#include "Scene.h"
#include "SceneUser.h"
#include "SceneNpcManager.h"
#include "xLuaTable.h"
#include "GuidManager.h"

SceneActManager::SceneActManager()
{
}

SceneActManager::~SceneActManager()
{

}

bool SceneActManager::init()
{
  //TODO
  return true;
}

void SceneActManager::final()
{
  auto it = xEntryTempID::ets_.begin();
  auto tmp = it;
  for ( ; it!=xEntryTempID::ets_.end(); )
  {
    tmp = it++;
    if (tmp->second)
    {
      SceneActBase* act = dynamic_cast<SceneActBase*> (tmp->second);
      if (act != nullptr)
      {
        delSceneAct(act);
      }
    }
  }
}

/*bool SceneActManager::loadConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Interaction.txt"))
  {
    XERR("[Table_Interaction], 加载Table_Interaction.txt失败");
    return false;
  }
  XLOG("[Interaction], 加载Table_Interaction");

  m_mapID2ActType.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Interaction", table);

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->first;
    if (m_mapID2ActType.find(id) != m_mapID2ActType.end())
      continue;
    int nType = m->second.getTableInt("Synchronization");
    if (nType <= EACTTYPE_MIN || nType >= EACTTYPE_MAX)
    {
      XERR("Table_Interaction, id = %u, Synchronization值不对", id);
      return false;
    }
    EActType eType = static_cast<EActType> (nType);
    m_mapID2ActType[id] = eType;
  }

  return true;
}*/

void SceneActManager::timer(DWORD sec)
{
  FUN_TIMECHECK_30();
  auto it = xEntryTempID::ets_.begin();
  auto tmp = it;
  for ( ; it!=xEntryTempID::ets_.end(); )
  {
    tmp = it++;
    if (tmp->second)
    {
      SceneActBase* act = dynamic_cast<SceneActBase*> (tmp->second);
      if (act != nullptr)
      {
        if (act->needDel())
        {
          delSceneAct(act);
        }
        else
        {
          act->timer(sec);
        }
      }
    }
  }
}

SceneActBase* SceneActManager::getSceneAct(QWORD tempid)
{
  return dynamic_cast<SceneActBase *> (getEntryByTempID(tempid));
}

SceneActBase* SceneActManager::createSceneAct(Scene* scene, xPos pos, DWORD range, QWORD masterID, EActType acttype)
{
  if (scene == nullptr)
  {
    XERR << "[createSceneAct] scene nullptr" << XEND;
    return nullptr;
  }

  if (scene->isValidPos(pos) == false && scene->getValidPos(pos) == false)
  {
    XERR << "[createSceneAct] pos invalid" << XEND;
    return nullptr;
  }

  // get tempid
  DWORD tempid = GuidManager::getMe().getNextActID();
  SceneActBase* pTemp = getSceneAct(tempid);
  if (pTemp != nullptr)
    return pTemp;

  // create scene act
  SceneActBase *pAct = nullptr;
  if (acttype == EACTTYPE_PURIFY)
    pAct = NEW SceneActPurify(tempid, scene, range, pos, masterID);
  else if (acttype == EACTTYPE_SEAL)
    pAct = NEW SceneActSeal(tempid, scene, range, pos, masterID);
  else if (acttype == EACTTYPE_MUSIC)
    pAct = NEW SceneActMusic(tempid, scene, range, pos, masterID);
  else if (acttype == EACTTYPE_EFFECT)
    pAct = NEW SceneActEffect(tempid, scene, range, pos, masterID);
  else if (acttype == EACTTYPE_SCENEEVENT)
    pAct = new SceneActEvent(tempid, scene, range, pos, masterID);
  if (pAct == nullptr)
    return nullptr;
  if (addSceneAct(pAct) == false)
  {
    XERR << "[createSceneAct] add error" << XEND;
    SAFE_DELETE(pAct);
    return nullptr;
  }

  // add to scene
  //pAct->enterScene(scene);

  XLOG << "[Act]" << pAct->tempid << scene->name << "创建act" << XEND;
  return pAct;
}

bool SceneActManager::addSceneAct(SceneActBase* act)
{
  return addEntry(act);
}

void SceneActManager::delSceneAct(SceneActBase* act)
{
  SceneActEffect* pEffect = dynamic_cast<SceneActEffect*>(act);
  if (pEffect)
  {
    auto it = m_mapSceneEffectCount.find(pEffect->getOwnerID());
    if (it != m_mapSceneEffectCount.end() && it->second)
      it->second--;
  }
  removeEntry(act);
  SAFE_DELETE(act);
}


void SceneActManager::addEffectCount(QWORD charId)
{
  m_mapSceneEffectCount[charId]++;
}

DWORD SceneActManager::getEffectCount(QWORD charId)
{
  auto it = m_mapSceneEffectCount.find(charId);
  if (it != m_mapSceneEffectCount.end())
    return it->second;
  return 0;
}
