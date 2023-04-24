#include "UserMap.h"
#include "SceneUser.h"
#include "MiscConfig.h"

UserMap::UserMap(SceneUser *user) : m_pUser(user)
{
}

UserMap::~UserMap()
{
}

void UserMap::setLastStaticMap(DWORD id, const xPos &pos)
{
  m_pUser->getUserSceneData().setLastSMapIdPos(id, pos);
}

DWORD UserMap::getLastStaticMapID() const
{
  return m_pUser->getUserSceneData().getLastSMapId();
}
const xPos& UserMap::getLastStaticMapPos() const
{
  return m_pUser->getUserSceneData().getLastSPos();
}

void UserMap::gotoLastStaticMap()
{
  DWORD mapid = getLastStaticMapID();
  if (!mapid)
  {
    const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
    mapid = rCFG.dwNewCharMapID;
  }
  if (m_pUser->getScene() && m_pUser->getScene()->id == mapid)
  {
    return;
  }
  m_pUser->gomap(mapid, GoMapType::GoBack, m_pUser->getUserSceneData().getLastSPos());
}
