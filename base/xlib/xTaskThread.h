#pragma once
#include "xThread.h"
#include "xDefine.h"
#include "xSocket.h"
#include "xTime.h"
#include <netinet/in.h>
#include <sys/epoll.h>

class xServer;

class xTaskThread : public xThread
{
  public:
    xTaskThread(xServer *s);
    virtual ~xTaskThread();

  public:
    bool thread_init();
    void thread_proc();
    virtual void thread_stop();
    void final();

    void setName(const char *n)
    {
      if (!n) return;
      bzero(name, sizeof(name));
      strncpy(name, n, MAX_NAMESIZE-1);
    }

    int getEpfd() { return m_nListenEpfd; }

  private:
    int m_nListenEpfd = INVALID_SOCKET;
    struct epoll_event m_oEvents[MAX_SERVER_EVENT];

    xServer *pServer = nullptr;

    char name[MAX_NAMESIZE];
};

