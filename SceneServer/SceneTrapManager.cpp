#include "SceneTrapManager.h"
#include "Scene.h"
#include "SceneUser.h"
#include "SceneNpcManager.h"
#include "GuidManager.h"

SceneTrapManager::SceneTrapManager()
{
}

SceneTrapManager::~SceneTrapManager()
{

}

bool SceneTrapManager::init()
{
  //TODO
  return true;
}

void SceneTrapManager::final()
{
  auto it = xEntryTempID::ets_.begin();
  auto tmp = it;
  for ( ; it!=xEntryTempID::ets_.end(); )
  {
    tmp = it++;
    if (tmp->second)
    {
      SceneTrap* trap = dynamic_cast<SceneTrap*> (tmp->second);
      if (trap != nullptr)
        delTrap(trap);
    }
  }
}

void SceneTrapManager::timer(QWORD curMSec)
{
  FUN_TIMECHECK_30();
  auto it = xEntryTempID::ets_.begin();
  auto tmp = it;
  for ( ; it!=xEntryTempID::ets_.end(); )
  {
    tmp = it++;
    if (tmp->second)
    {
      SceneTrap* trap = dynamic_cast<SceneTrap*>(tmp->second);
      if (trap != nullptr)
      {
        if (trap->m_blNeedClear)
        {
          delTrap(trap);
        }
        else
        {
          trap->timer(curMSec);
        }
      }
    }
  }
}

SceneTrap* SceneTrapManager::getSceneTrap(QWORD tempid)
{
  return dynamic_cast<SceneTrap *>(getEntryByTempID(tempid));
}

SceneTrap* SceneTrapManager::createSceneTrap(Scene* scene, Cmd::SkillBroadcastUserCmd &cmd, QWORD masterID, const BaseSkill *skill)
{
  if (scene == NULL || skill == NULL)
    return NULL;

  xPos p;
  switch(skill->getLogicType())
  {
    case ESKILLLOGIC_SELFRANGE:
      {
        xSceneEntryDynamic* pMaster = xSceneEntryDynamic::getEntryByID(masterID);
        if (pMaster)
          p = pMaster->getPos();
      }
      break;
    default:
      if (cmd.data().has_pos())
        p.set(cmd.data().pos().x(), cmd.data().pos().y(), cmd.data().pos().z());
      break;
  }
  if (scene->isValidPos(p) == false)
    return nullptr;

  // get tempid
  DWORD tempid = GuidManager::getMe().getNextTrapID();
  SceneTrap* pTemp = getSceneTrap(tempid);
  if (pTemp != nullptr)
    return pTemp;

  // create scene item
  SceneTrap* pTrap = nullptr;
  switch(skill->getTrapType())
  {
    case ESKILLTRAPTYPE_MIN:
      pTrap = NEW SceneTrap(tempid, scene, p, cmd, masterID, skill);
      break;
    case ESKILLTRAPTYPE_TRANS:
      pTrap = NEW TransportTrap(tempid, scene, p, cmd, masterID, skill);
      break;
  }

  if (pTrap == NULL)
    return NULL;
  if (addTrap(pTrap) == false)
  {
    SAFE_DELETE(pTrap);
    return NULL;
  }

  // add to scene
  if (!pTrap->enterScene(scene))
  {
    XERR << "[Trap]" << tempid << scene->name << "创建trap失败" << XEND;
    delTrap(pTrap);
    return nullptr;
  }

  XLOG << "[Trap]" << pTrap->tempid << scene->name << "创建trap" << XEND;
  return pTrap;
}

bool SceneTrapManager::addTrap(SceneTrap* item)
{
  return addEntry(item);
}

void SceneTrapManager::delTrap(SceneTrap* item)
{
  removeEntry(item);
  SAFE_DELETE(item);
}
