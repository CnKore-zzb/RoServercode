#pragma once

#include "xSceneEntry.h"
#include "SkillItem.h"

enum ETrapType
{
  ETRAPTYPE_NORMAL = 1,
  ETRAPTYPE_BARRIER = 2,
};

namespace Cmd
{
  class MapTrap;
};

class Scene;
class SceneUser;
class SceneTrap : public xSceneEntry
{
  public:
    SceneTrap(QWORD tempid, Scene* scene, const xPos& pos, Cmd::SkillBroadcastUserCmd &cmd, QWORD master, const BaseSkill *skill);
    virtual ~SceneTrap();

    virtual SCENE_ENTRY_TYPE getEntryType() const { return SCENE_ENTRY_TRAP; }

    virtual void delMeToNine();
    virtual void sendMeToNine();

    bool enterScene(Scene* scene);
    void fillMapTrapData(Cmd::MapTrap* data);
    void leaveScene();

    xSceneEntryDynamic* getMaster();
    SceneUser* getScreenUser();

    //   void fillTrapData(MapTrap* data);
    void timer(QWORD curMSec);
    virtual bool action(QWORD msec);

    void setClearState()
    {
      if (!m_blNeedClear)
      {
        leaveScene();
        m_blNeedClear = true;
      }
    }
    void setCount(DWORD count) { m_dwCount = count; }
    DWORD getCount() const { return m_dwCount; }
    ETrapType getType() const { return m_eTrapType; }
    bool canImmunedByFieldArea() { return m_bCanImmunedByFieldArea; }
    const BaseSkill* getSkill() const { return m_skill; }

  public:
    bool m_blNeedClear = false;

  protected:
    QWORD m_dwMasterID = 0;
    const BaseSkill *m_skill = NULL;
    DWORD m_dwCount = 0;
    QWORD m_dwEndTime = 0;
    Cmd::SkillBroadcastUserCmd m_oCmd;
    bool m_bCanImmunedByFieldArea = true;

    // 参数
    DWORD m_dwInterval = 0;
    ETrapType m_eTrapType = ETRAPTYPE_NORMAL;
};

class TransportTrap : public SceneTrap
{
  public:
    TransportTrap(QWORD tempid, Scene* scene, const xPos& pos, Cmd::SkillBroadcastUserCmd& cmd, QWORD masterID, const BaseSkill* skill);
    virtual ~TransportTrap();

    virtual bool action(QWORD curMSec);

  private:
    QWORD m_qwNextTime = 0;
};

