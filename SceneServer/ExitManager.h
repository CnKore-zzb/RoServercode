#pragma once
#include <list>
#include "xDefine.h"

class SceneUser;
class Scene;

class ExitManager
{
  public:
    ExitManager(Scene *s);
    ~ExitManager();

  public:
    bool get(DWORD id, DWORD &state)
    {
      auto it = m_oExitsState.find(id);
      if (it==m_oExitsState.end()) return false;

      state = it->second;
      return true;
    }

    void set(DWORD id, DWORD state)
    {
      m_oExitsState[id] = state;
    }

  private:
    std::map<DWORD, DWORD> m_oExitsState;
    //Scene *m_pScene = NULL;
};
