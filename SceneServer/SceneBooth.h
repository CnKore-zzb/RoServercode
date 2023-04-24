#pragma once

#include "xSceneEntry.h"
#include "SceneMap.pb.h"

namespace Cmd
{
  class MapUser;
};

class Scene;
class SceneUser;
class SceneBooth : public xSceneEntry
{
  public:
    SceneBooth(QWORD charid, DWORD zoneid, Scene* scene, const xPos& pos, const Cmd::MapUser& mapUser);
    virtual ~SceneBooth();

    virtual SCENE_ENTRY_TYPE getEntryType() const { return SCENE_ENTRY_BOOTH; }

    virtual void sendMeToNine();
    virtual void delMeToNine();

   public:
    bool enterScene(Scene* scene);
    void leaveScene();
    void timer(QWORD curMSec);

    void fillData(Cmd::MapUser* data);

   public:
    void update(const std::string& name);

   private:
    Cmd::MapUser m_oCmd;
    DWORD m_dwZoneId;
};
