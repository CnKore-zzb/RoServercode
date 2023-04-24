#include "SceneNpcManager.h"
#include "DScene.h"
#include "GuidManager.h"
#include "SceneNpc.h"
#include "SceneManager.h"
#include "DressUpStageMgr.h"

SceneNpcManager::SceneNpcManager()
{

}

SceneNpcManager::~SceneNpcManager()
{

}

bool SceneNpcManager::init()
{
  //TODO
  return true;
}

void SceneNpcManager::final()
{
  struct Del: public xEntryCallBack
  {
    SceneNpcManager& thisRef;
    Del(SceneNpcManager& thisRef): thisRef(thisRef) {}
    virtual bool exec(xEntry* e)
    {
      SceneNpc* pNpc = static_cast<SceneNpc*>(e);
      thisRef.delNpc(pNpc);
      return true;
    }
  };

  Del del(*this);
  forEach(del);
}

bool SceneNpcManager::addNpc(SceneNpc* npc)
{
  return addEntry(npc);
}

void SceneNpcManager::delNpc(SceneNpc* npc)
{
  if (npc->getNpcType() == ENPCTYPE_FOOD)
  {
    FoodNpc* pFoodNpc = dynamic_cast<FoodNpc*>(npc);
    if (pFoodNpc)
    {
      auto it = m_mapFoodCount.find(pFoodNpc->getOwnerID());
      if (it != m_mapFoodCount.end() && it->second)
        it->second--;       
    }
  }

  delLeaveSceneNpc(npc);

  removeEntry(npc);
  SAFE_DELETE(npc);
}

/*bool SceneNpcManager::addItem(SceneItem* item)
{
  return addEntry(item);
}*/

/*void SceneNpcManager::delItem(SceneItem* item)
{
  removeEntry(item);
  SAFE_DELETE(item);
}*/

SceneNpc* SceneNpcManager::getNpcByTempID(const UINT tempId)
{
  return (SceneNpc*)getEntryByTempID(tempId);
}

SceneNpc* SceneNpcManager::createNpc(const NpcDefine& define, Scene* pScene)
{
  if (pScene == nullptr)
    return nullptr;

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(define.getID());
  if (pCFG == nullptr)
    return nullptr;

  QWORD tempid = 0;
  if (pCFG->eNpcType == ENPCTYPE_WEAPONPET)
  {
    if (define.m_oVar.m_qwOwnerID == 0 || define.getWeaponPetID() == 0)
      return nullptr;
    tempid = define.m_oVar.m_qwOwnerID * ONE_THOUSAND + define.getWeaponPetID();
    SceneNpc* pPet = getNpcByTempID(tempid);
    if (pPet && pPet->getStatus() == ECREATURESTATUS_CLEAR)
    {
      SceneNpcManager::getMe().delNpc(pPet);
    }
  }
  else
  {
    tempid = GuidManager::getMe().getNextNpcID();
  }
  if (tempid == 0)
    return nullptr;

  SceneNpc* pNpc = getNpcByTempID(tempid);
  if (pNpc != nullptr)
    return pNpc;

  switch(pCFG->eNpcType)
  {
    case ENPCTYPE_MIN:
    case ENPCTYPE_NPC:
    case ENPCTYPE_FRIEND:
      if (pCFG->dwID == MiscConfig::getMe().getSystemCFG().dwMusicBoxNpc || pCFG->dwID == MiscConfig::getMe().getJoyLimitCFG().dwMusicNpcID)
        pNpc = NEW MusicNpc(tempid, pCFG, define);
      else if (pCFG->dwID == 1800 || pCFG->dwID == 1801 || pCFG->dwID == 1802)
        pNpc = NEW TreeNpc(tempid, pCFG, define);
      else if (pCFG->bCanBeMonster)
        pNpc = NEW MonsterNpc(tempid, pCFG, define);
      else
      {
        const STrapNpcCFG& rCFG = MiscConfig::getMe().getTrapNpcCFG();
        if (rCFG.isTrapNpc(pCFG->dwID) == true)
          pNpc = NEW TrapNpc(tempid, pCFG, define);
        else
          pNpc = NEW FuncNpc(tempid, pCFG, define);
      }
      break;
    case ENPCTYPE_GATHER:
    case ENPCTYPE_MONSTER:
    case ENPCTYPE_MINIBOSS:
    case ENPCTYPE_MVP:
    case ENPCTYPE_SMALLMINIBOSS:   // use for tower monster(rand summon no config) only
      if (pCFG->eZoneType == ENPCZONE_ENDLESSTOWER)
        pNpc = NEW TowerNpc(tempid, pCFG, define);
      else if (pCFG->eZoneType == ENPCZONE_LABORATORY)
        pNpc = NEW LabNpc(tempid, pCFG, define);
      else if (pCFG->eZoneType == ENPCZONE_SEAL)
        pNpc = NEW SealNpc(tempid, pCFG, define, pScene->getMapID());
      else if (pCFG->eZoneType == ENPCZONE_DOJO)
        pNpc = NEW DojoNpc(tempid, pCFG, define);
      else if (pCFG->eZoneType == ENPCZONE_DEAD)
        pNpc = NEW DeadNpc(tempid, pCFG, define);
      else if (pCFG->eZoneType == ENPCZONE_WORLD)
        pNpc = NEW WorldNpc(tempid, pCFG, define);
      //else if (define.getPurify() > 0)
      //  pNpc = NEW PurifyNpc(tempid, pCFG);
      else
        pNpc = NEW MonsterNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_WEAPONPET:
      pNpc = NEW WeaponPetNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_SKILLNPC:
      pNpc = NEW SkillNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_CATCHNPC:
      pNpc = NEW CatchPetNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_PETNPC:
      pNpc = NEW PetNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_FOOD:
      pNpc = NEW FoodNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_BEING:
      pNpc = NEW BeingNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_ELEMENTELF:
      pNpc = NEW ElementElfNpc(tempid, pCFG, define);
      break;
    case ENPCTYPE_MAX:
      break;
  }

  if (pNpc == nullptr)
  {
    return nullptr;
  }
  if (pNpc->init(pCFG, define) == false || addNpc(pNpc) == false)
  {
    XERR << "[Npc-创建], 创建失败, npc:" << define.getID() << "地图:" << pScene->id << XEND;
    SAFE_DELETE(pNpc);
    return nullptr;
  }

  if (define.isGear())
    pScene->m_oGear.add(pNpc);

  if (!pNpc->enterScene(pScene))
  {
    XERR << "[NPC管理-创建]" << pNpc->id << pNpc->getNpcID() << pNpc->name << "创建失败:" << pScene->id << pScene->name << XEND;
    delNpc(pNpc);
    return nullptr;
  }

  XLOG << "[NPC管理-创建]" << pNpc->id << pNpc->getNpcID() << pNpc->name << "被创建在地图 :"<< pScene->id << pScene->name << "位置 :" << "(" << pNpc->getPos().x << pNpc->getPos().y << pNpc->getPos().z << ")" << XEND;
  return pNpc;
}

SceneNpc* SceneNpcManager::createNpcLua(SLuaParams& luaParam)
{
  xLuaData& data = luaParam.m_oData;
  DWORD mapid = data.getTableInt("map");
  Scene* pScene = SceneManager::getMe().getSceneByID(mapid);
  if (pScene == nullptr)
    return nullptr;
  NpcDefine define;
  define.load(data);
  float x = data.getTableFloat("pos_x");
  float y = data.getTableFloat("pos_y");
  float z = data.getTableFloat("pos_z");
  xPos pos(x, y, z);
  if (!pScene->isValidPos(pos))
    return nullptr;
  define.setPos(pos);

  return createNpc(define, pScene);
}

/*SceneNpc* SceneNpcManager::createNpcLua(DWORD dwNpcID, DWORD dwMapID, DWORD dwTerritory, DWORD dwBehavior, DWORD dispTime, float x, float y, float z)
{
  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if (pScene == nullptr)
    return nullptr;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwNpcID);
  if (pCFG == nullptr)
    return nullptr;

  NpcDefine define;
  define.setID(pCFG->dwID);
  define.setPos(xPos(x, y, z));
  define.setTerritory(dwTerritory);
  define.setBehaviours(dwBehavior);
  define.setLife(1);
  define.setDisptime(dispTime);
  return createNpc(define, pScene);
}
*/

UINT SceneNpcManager::createNpc(Scene* scene, const NpcDefine& define, UINT num)
{
  if (!num) num = 1;
  DWORD createNum = 0;
  for (UINT i = 0; i < num; ++i)
  {
    if (createNpc(define, scene))
      ++createNum;
  }
  return createNum;
}

void SceneNpcManager::timer(QWORD curMSec)
{
  FUN_TIMECHECK_30();
  xSceneEntrySet clearNpcSet;
  xSceneEntrySet removeNpcSet;
  for (auto &iter : m_oLeaveSceneNpc)
  {
    SceneNpc *npc = dynamic_cast<SceneNpc *>(iter);
    if (npc == nullptr)
      continue;

    if (npc->getStatus() == ECREATURESTATUS_CLEAR)
    {
      clearNpcSet.insert(iter);
    }
    else
    {
      npc->refreshMe(curMSec);
      if (npc->getScene() != nullptr)
        removeNpcSet.insert(iter);
    }
  }
  if (!removeNpcSet.empty())
  {
    for (auto iter : removeNpcSet)
      m_oLeaveSceneNpc.erase(dynamic_cast<SceneNpc*>(iter));
  }
  if (!clearNpcSet.empty())
  {
    for (auto &iter : clearNpcSet)
    {
      SceneNpcManager::getMe().delNpc((SceneNpc *)iter);
    }
  }
}

void SceneNpcManager::reload()
{
  struct SCallback : public xEntryCallBack
  {
    virtual bool exec(xEntry* e)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*>(e);
      if (pNpc == nullptr)
        return false;

      pNpc->reload();
      return true;
    }
  };

  SCallback oCallback;
  forEach(oCallback);
}

bool SceneNpcManager::addObject(SceneNpc* pNpc)
{
  return addEntry(pNpc);
}

void SceneNpcManager::removeObject(SceneNpc* pNpc)
{
  removeEntry(pNpc);
}

void SceneNpcManager::addLeaveSceneNpc(SceneNpc *npc)
{
  if (!npc) return;

  m_oLeaveSceneNpc.insert(npc);
}

void SceneNpcManager::delLeaveSceneNpc(SceneNpc *npc)
{
  if (!npc) return;

  m_oLeaveSceneNpc.erase(npc);
}

void SceneNpcManager::reloadconfig(ConfigType type)
{
  struct SCallback : public xEntryCallBack
  {
    ConfigType ctype;
    SCallback(ConfigType type){
      ctype = type;
    }

    virtual bool exec(xEntry* e)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*>(e);
      if (pNpc == nullptr)
        return false;

      switch(ctype)
      {
        case ConfigType::buffer:
          pNpc->m_oBuff.reloadAllBuff();
          break;
        default:
          break;
      }
      return true;
    }
  };

  SCallback oCallback(type);
  forEach(oCallback);
}

void SceneNpcManager::delUniqueID(Scene* scene, DWORD dwUniqueID)
{
  if (scene == nullptr)
    return;
  struct SCallback : public xEntryCallBack
  {
    Scene* m_scene;
    DWORD m_uniqueid;
    SCallback(Scene* scene, DWORD dwUniqueID){
      m_scene = scene;
      m_uniqueid = dwUniqueID;
    };
    virtual bool exec(xEntry* e)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (e);
      if (npc == nullptr || npc->getScene() != m_scene)
        return true;
      if (npc->define.getUniqueID() == m_uniqueid)
        npc->setStatus(ECREATURESTATUS_LEAVE);

      return true;
    }
  };

  SCallback oCallback(scene, dwUniqueID);
  foreach(oCallback);
  XLOG << "[场景npc删除], 地图:" << scene->name << scene->id << "npc uniqueid:" << dwUniqueID << XEND;
}

void SceneNpcManager::addFoodNpcCount(QWORD charId)
{
  m_mapFoodCount[charId]++;
}

DWORD SceneNpcManager::getFoodNpcCount(QWORD charId)
{
  auto it = m_mapFoodCount.find(charId);
  if (it != m_mapFoodCount.end())
    return it->second;
  return 0;
}

bool SceneNpcManager::addChangeBodyNpc(QWORD npcid, DWORD buffid)
{
  auto it = m_mapChangeBody.find(npcid);
  if (it != m_mapChangeBody.end())
    return false;

  m_mapChangeBody.insert(make_pair(npcid,buffid));
  XLOG << "[变身NPC], 添加 " << "npcid: "<< npcid << "buffid: " << buffid << XEND;
  return true;
}

bool SceneNpcManager::delChangeBodyNpc(QWORD npcid)
{
  auto it = m_mapChangeBody.find(npcid);
  if (it == m_mapChangeBody.end())
    return false;

  m_mapChangeBody.erase(it);
  XLOG << "[变身NPC], 删除 " << "npcid: "<< npcid << XEND;
  return true;
}

DWORD SceneNpcManager::getChangeBodyID(QWORD npcid)
{
  return 0;   //功能暂时不开放

  auto it = m_mapChangeBody.find(npcid);
  if (it == m_mapChangeBody.end())
    return 0;

  return it->second;
}
