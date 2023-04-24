#include "xTaskThread.h"
#include "xServer.h"
#include "xNetProcessor.h"
#include <arpa/inet.h>

xTaskThread::xTaskThread(xServer *s) : pServer(s)
{
  bzero(name, sizeof(name));
}

xTaskThread::~xTaskThread()
{
}

void xTaskThread::thread_stop()
{
  final();

  xThread::thread_stop();
}

void xTaskThread::final()
{
  if (INVALID_SOCKET != m_nListenEpfd)
  {
    SAFE_CLOSE_SOCKET(m_nListenEpfd);
  }
}

bool xTaskThread::thread_init()
{
  m_nListenEpfd = epoll_create(MAX_SERVER_EVENT);
  if (m_nListenEpfd < 0)
  {
    XERR_T("[TaskThread],epoll_create() failed, %s", strerror(errno));
    m_nListenEpfd = INVALID_SOCKET;
    return false;
  }
  XLOG_T("[TaskThread],创建epfd:%u", m_nListenEpfd);

  return true;
}

void xTaskThread::thread_proc()
{
  static QWORD MIN_RECURSIVE_TIME = 30 * 1000;
  static QWORD MAX_RECURSIVE_TIME = 100 * 1000;

  thread_setState(THREAD_RUN);

  while (thread_getState()==xThread::THREAD_RUN)
  {
    xTime frameTimer;

    pServer->select_th(m_nListenEpfd, 0, m_oEvents);

    QWORD _e = frameTimer.uElapse();
    if (_e < MIN_RECURSIVE_TIME)
    {
      usleep(MIN_RECURSIVE_TIME - _e);
    }
    else if (_e > MAX_RECURSIVE_TIME)
    {
      //XLOG << "[TaskThread]" << m_nListenEpfd << "帧耗时" << _e << "微秒" << XEND;
      XLOG_T("[TaskThread] %u 帧耗时 %llu 微秒", m_nListenEpfd, _e);
    }
  }
}
