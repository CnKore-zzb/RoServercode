#include "SceneBoothManager.h"
#include "Scene.h"
#include "SceneUser.h"


SceneBoothManager::SceneBoothManager()
{
}

SceneBoothManager::~SceneBoothManager()
{
}

bool SceneBoothManager::init()
{
  return true;
}

void SceneBoothManager::final()
{
  auto it = xEntryTempID::ets_.begin();
  auto tmp = it;
  for (; it != xEntryTempID::ets_.end();)
  {
    tmp = it++;
    if(tmp->second)
    {
      SceneBooth* booth = dynamic_cast<SceneBooth*>(tmp->second);
      if(booth)
        delBooth(booth);
    }
  }
}

void SceneBoothManager::timer(QWORD curMSec)
{
  FUN_TIMECHECK_30();
  auto it = xEntryTempID::ets_.begin();
  auto tmp = it;
  for(; it!=xEntryTempID::ets_.end();)
  {
    tmp = it++;
    if(tmp->second)
    {
      SceneBooth* booth = dynamic_cast<SceneBooth*>(tmp->second);
      if(booth)
      {
        booth->timer(curMSec);
      }
    }
  }
}

SceneBooth* SceneBoothManager::getSceneBooth(QWORD charid)
{
  return dynamic_cast<SceneBooth*>(getEntryByTempID(charid));
}

SceneBooth* SceneBoothManager::createSceneBooth(Scene* scene, DWORD zoneId, const Cmd::MapUser& mapUser)
{
  if(!scene) return nullptr;

  xPos pos;
  pos.set(mapUser.pos().x(), mapUser.pos().y(), mapUser.pos().z());
  SceneBooth* booth = NEW SceneBooth(mapUser.guid(), zoneId, scene, pos, mapUser);
  if(!booth)
  {
    XERR << "[摊位创建]摊位创建失败" << mapUser.ShortDebugString() << XEND;
    return nullptr;
  }

  if(!addBooth(booth))
  {
    XERR << "[摊位创建]摊位添加失败" << mapUser.ShortDebugString() << XEND;
    SAFE_DELETE(booth);
    return nullptr;
  }

  if(!booth->enterScene(scene))
  {
    XERR << "[摊位创建]进入场景失败" << mapUser.ShortDebugString() << XEND;
    delBooth(booth);
    return nullptr;
  }

  return booth;
}

bool SceneBoothManager::delBooth(QWORD charid)
{
  SceneBooth* booth = getSceneBooth(charid);
  if(!booth)
    return false;

  return delBooth(booth);
}

bool SceneBoothManager::addBooth(SceneBooth* item)
{
  if(!item || !item->getScene())
    return false;

  DWORD mapId = item->getScene()->getMapID();
  m_mapScene[mapId]++;
  XLOG << "[摊位-增加] sceneid:" << mapId << " count:" << m_mapScene[mapId] << XEND;
  return addEntry(item);
}

bool SceneBoothManager::delBooth(SceneBooth* item)
{
  if(!item || !item->getScene())
    return false;

  DWORD mapId = item->getScene()->getMapID();
  m_mapScene[mapId]--;
  item->leaveScene();
  removeEntry(item);
  SAFE_DELETE(item);
  XLOG << "[摊位-减少] sceneid:" << mapId << " count:" << m_mapScene[mapId] << XEND;
  return true;
}
