#include "BoothManager.h"

BoothManager::BoothManager()
{
}

BoothManager::~BoothManager()
{
}

void BoothManager::init()
{
}

bool BoothManager::loadConfig()
{
  return true;
}

bool BoothManager::addMapUser(DWORD zoneId, DWORD sceneId, const Cmd::MapUser& data)
{
  if(hasMapUser(data.guid()))
  {
    XERR << "[摊位-添加] has exist booth, zoneid:" << zoneId << "sceneId:" << sceneId << "data:" << data.ShortDebugString() << "size:" << data.ByteSize() << XEND;
    return false;
  }

  SBoothUser& user = m_mapChar2User[data.guid()];
  user.m_qwCharId = data.guid();
  user.m_dwZoneId = zoneId;
  user.m_dwSceneId = sceneId;
  user.mapUser.CopyFrom(data);

  XLOG << "[摊位-添加] success, zoneid:" << zoneId << "sceneId:" << sceneId << "charId:" << data.guid() << "data:" << data.ShortDebugString() << "size:" << data.ByteSize() << XEND;
  return true;
}

bool BoothManager::updateMapUser(const Cmd::MapUser& data)
{
  if(!hasMapUser(data.guid()))
  {
    XERR << "[摊位-更新] has not exist booth, data:" << data.ShortDebugString() << XEND;
    return false;
  }

  SBoothUser& user = m_mapChar2User[data.guid()];
  user.mapUser.CopyFrom(data);

  XLOG << "[摊位-更新] success, data:" << data.ShortDebugString() << "size:" << data.ByteSize() << XEND;
  return true;
}

bool BoothManager::delMapUser(QWORD charId)
{
  if(!hasMapUser(charId))
  {
    XERR << "[摊位-删除] has not exist booth, charid:" << charId << XEND;
    return false;
  }

  m_mapChar2User.erase(charId);
  XLOG << "[摊位-删除] success, charid:" << charId << XEND;
  return true;
}

bool BoothManager::hasMapUser(QWORD charId)
{
  return m_mapChar2User.end() != m_mapChar2User.find(charId);
}
