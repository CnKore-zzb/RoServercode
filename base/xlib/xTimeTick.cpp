#include <list>
#include "xlib/xServer.h"
#include "xlib/xNetProcessor.h"
#include "ClientManager.h"
#include "PlatLogManager.h"

void xServer::v_timetick()
{
  DWORD cur = now();

  m_oClientManager.process();

  std::list<xNetProcessor *> templist;
  {
    ScopeWriteLock swl(m_oVerifyLock);
    for (auto iter = m_oVerifyList.begin(); iter!=m_oVerifyList.end(); ++iter)
    {
      if (iter->first)
      {
        templist.push_back(iter->first);
      }
    }
  }

  for (auto iter = templist.begin(); iter!=templist.end(); ++iter)
  {
    if (*iter)
    {
      doCmd(*iter);
    }
  }

  {
    checkCloseList(30);
  }

  templist.clear();
  {
    ScopeWriteLock swl(m_oVerifyLock);
    for (auto iter = m_oVerifyList.begin(); iter!=m_oVerifyList.end(); ++iter)
    {
      if (iter->first && (iter->second + 30)<=cur)
      {
        templist.push_back(iter->first);
      }
    }
  }

  for (std::list<xNetProcessor *>::iterator iter = templist.begin(); iter!=templist.end(); iter++)
  {
    if (*iter)
    {
      addCloseList(*iter, TerminateMethod::terminate_active, "m_oVerifyList超时");
    }
  }


  if (getServerState() == ServerState::run)
  {
    PlatLogManager::getMe().timeTick(cur);
    if (m_oOneSecTimer.timeUp(cur))
    {
      if (m_oTenSecTimer.timeUp(cur))
      {
        m_oClientManager.timer();
        if (m_oTenMinTimer.timeUp(cur))
        {
          m_oMessageStat.print();
        }
      }
    }
  }
}
