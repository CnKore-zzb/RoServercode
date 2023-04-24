#pragma once
#include "xThread.h"
#include "xDefine.h"
#include "xSocket.h"
#include "xTime.h"
#include <netinet/in.h>
#include <sys/epoll.h>

class xServer;

class xListenThread : public xThread
{
  public:
    xListenThread(xServer *s);
    virtual ~xListenThread();

  public:
    bool start() { return INVALID_SOCKET != m_nListenSock; }
    bool thread_init();
    void thread_proc();

    bool bind();
    virtual bool accept(int sockfd, const sockaddr_in &addr) { return true; }

  public:
    int getPort() { return m_nListenPort; }
    void setPort(int p) { m_nListenPort = p; }

    int getSock() { return m_nListenSock; }
    void setSock(int s) { m_nListenSock = s; }

    int getEpfd() { return m_nListenEpfd; }
    void setEpfd(int e) { m_nListenEpfd = e; }

  private:
    int m_nListenPort = 0;
    int m_nListenSock = INVALID_SOCKET;
    int m_nListenEpfd = INVALID_SOCKET;
    struct epoll_event listen_ev[MAX_SERVER_EVENT];

    xServer *pServer = nullptr;
};

