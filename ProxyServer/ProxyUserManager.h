#pragma once
#include <list>
//#include "Define.h"
#include "xEntryManager.h"
#include "xSingleton.h"
#include "xCommand.h"

class ProxyUser;
class xNetProcessor;
class ProxyUserData;

class ProxyUserManager : public xSingleton<ProxyUserManager>
{
  friend class xSingleton<ProxyUserManager>;

  public:
    virtual ~ProxyUserManager();
  private:
    ProxyUserManager();

  public:
    void process();
    void timer();
    void onClose(xNetProcessor* np);
    void doProxyUserData(ProxyUserData *pData);

  public:
    bool add(ProxyUser *);
    bool del(ProxyUser *);

  public:
    void syncTaskNum();

  private:
    std::set<ProxyUser *> m_oProxyUserList;
    xTimer m_oOneSecTimer;
    xTimer m_oFiveSecTimer;

    xTime m_oFrameTimer;
    xTime m_oTickFrameTimer;

    DWORD m_dwCount = 0;
};
