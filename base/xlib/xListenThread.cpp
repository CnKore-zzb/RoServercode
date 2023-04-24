#include "xListenThread.h"
#include "xServer.h"
#include "xNetProcessor.h"
#include <arpa/inet.h>

xListenThread::xListenThread(xServer *s) : pServer(s)
{
}

xListenThread::~xListenThread()
{
  thread_stop();

  if (INVALID_SOCKET != m_nListenSock)
    SAFE_CLOSE_SOCKET(m_nListenSock);
  if (INVALID_SOCKET != m_nListenEpfd)
    SAFE_CLOSE_SOCKET(m_nListenEpfd);
}

bool xListenThread::thread_init()
{
  return true;
}

bool xListenThread::bind()
{
  m_nListenEpfd = epoll_create(MAX_SERVER_EVENT);
  if (m_nListenEpfd < 0)
  {
    XERR_T("[监听],epoll_create() failed,%s", strerror(errno));
    m_nListenEpfd = INVALID_SOCKET;
    return false;
  }
  XLOG_T("[监听],创建epfd:%u", m_nListenEpfd);
  if ((m_nListenSock = socket(AF_INET, SOCK_STREAM, 0)) <0)
  {
    XERR_T("[监听],socket() failed,%s", strerror(errno));
    m_nListenSock = INVALID_SOCKET;
    return false;
  }
  XLOG_T("[监听],创建socket成功,%u", m_nListenSock);

  int re = 1;
  setsockopt(m_nListenSock, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(re));

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(m_nListenPort);
  if (::bind(m_nListenSock, (sockaddr *) &addr, sizeof(addr)) == -1)
  {
    XERR_T("[监听],bind %s:%u failed %s", inet_ntoa(addr.sin_addr), m_nListenPort, strerror(errno));
    return false;
  }
  XLOG_T("[监听],bind %s:%u", inet_ntoa(addr.sin_addr), m_nListenPort);

  if(::listen(m_nListenSock, 256) == -1)
  {
    XERR_T("[监听],listen failed,%s", strerror(errno));
    return false;
  }

  epoll_event event;
  bzero(&event, sizeof(event));
  event.data.fd = m_nListenSock;
  event.events = EPOLLIN | EPOLLERR;
  epoll_ctl(m_nListenEpfd, EPOLL_CTL_ADD, m_nListenSock, &event);

  XLOG_T("[xListenThread],add epoll,sock:%u,epfd:%u", m_nListenSock, m_nListenEpfd);

  return true;
}

void xListenThread::thread_proc()
{
  QWORD MIN_RECURSIVE_TIME = 30 * 1000;
  QWORD MAX_RECURSIVE_TIME = 100 * 1000;

  thread_setState(THREAD_RUN);

  while (thread_getState()==xThread::THREAD_RUN)
  {
    xTime frameTimer;

    pServer->select_th(m_nListenEpfd, m_nListenSock, listen_ev);

    QWORD e = frameTimer.uElapse();
    if (e < MIN_RECURSIVE_TIME)
    {
      usleep(MIN_RECURSIVE_TIME - e);
    }
    else if (e > MAX_RECURSIVE_TIME)
    {
      //XLOG << "[xListenThread]" << "帧耗时" << e << "微秒" << XEND;
      XLOG_T("[xListenThread] 帧耗时 %llu 微秒", e);
    }
  }
}
