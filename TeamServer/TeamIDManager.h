#pragma once

#include <queue>

#include "xlib/xSingleton.h"

class TeamIDManager : public xSingleton<TeamIDManager>
{
  public:
    TeamIDManager();
    virtual ~TeamIDManager();

  public:
    void load();

  public:
    QWORD getTeamID();
    void putTeamID(QWORD teamid);

  private:
    void createTeamID(DWORD num = 1);
    DWORD size();

  private:
    std::queue<QWORD> m_oTeamIDQueue;
};

inline DWORD TeamIDManager::size()
{
  return m_oTeamIDQueue.size();
}
