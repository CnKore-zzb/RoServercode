#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"

class SceneUser;

namespace Cmd
{
  class BlobGears;
};

class UserGear : private xNoncopyable
{
  public:
    UserGear(SceneUser *u);
    ~UserGear();

  public:
    void set(DWORD mapid, DWORD id, DWORD state);
    DWORD get(DWORD mapid, DWORD id);

    void save(Cmd::BlobGears *data);
    void load(const Cmd::BlobGears &data);

    bool isVisible(DWORD mapid, DWORD exitid, DWORD dft)
    {
      auto it = m_oExitStateList.find(mapid);
      if (it==m_oExitStateList.end()) return 0!=dft;

      auto iter = it->second.find(exitid);
      if (iter==it->second.end()) return 0!=dft;

      return iter->second!=0;
    }

    void setExitState(DWORD mapid, DWORD exitid, DWORD visible)
    {
      m_oExitStateList[mapid][exitid] = visible;
    }

  public:
    SceneUser *m_pUser = NULL;

  private:
    std::map<DWORD, std::map<DWORD, DWORD>> m_list;    // scene gearid state
    std::map<DWORD, std::map<DWORD, DWORD>> m_oExitStateList;    // scene exitid state
};
