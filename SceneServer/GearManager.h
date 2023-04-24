#pragma once
#include <list>
#include "xDefine.h"

class SceneUser;
class SceneNpc;
class Scene;

struct Gear
{
  DWORD id = 0;
  QWORD tempid = 0;
  DWORD state = 0;
  bool isPrivate = false;
};

class GearManager
{
  public:
    GearManager(Scene *s);
    ~GearManager();

  public:
    void add(SceneNpc *npc);
    void set(DWORD gearID, DWORD state, SceneUser *user);
    DWORD get(DWORD gearID, SceneUser *user=NULL);

    void send(SceneUser *user, std::list<DWORD> &list);

  private:
    std::map<DWORD, Gear> m_oGearList;
    Scene *m_pScene = NULL;
};
