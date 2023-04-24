#pragma once
#include "xSingleton.h"
#include "xNoncopyable.h"

class UserActive : public xSingleton<UserActive>
{
  friend class xSingleton<UserActive>;

  public:
    virtual ~UserActive();
  private:
    UserActive();

  public:
    void init();
    void add(QWORD charid);
    void timer(DWORD cur);
    void update();
    void updateOnline2MatchServer();
    void setActiveTime(QWORD accid, DWORD t, bool isUpdate = true);

  private:
    void check();
  private:
    // accid 活跃时间
    std::map<QWORD, DWORD> m_list;
    DWORD m_dwCheckTime = 0;

    DWORD m_dwOnlineNum = 0;
    DWORD m_dwActiveNum = 0;
    DWORD m_dwOnlineNumTemp = 0;
};
