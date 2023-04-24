#include "xThread.h"
#include "xTools.h"
#include <unistd.h>

xThread::xThread()
{
  thread_setState(THREAD_INIT);
  pid = 0;
}

bool xThread::thread_start()
{
  if (!thread_init()) return false;

  int ret = pthread_create(&pid, NULL, &thread_run, (void *)this);
  if (ret == 0)
  {
    //setState(THREAD_RUN);
    //pthread_join(pid, NULL);
    XLOG << "创建新线程成功, id:" << (int)pid << XEND;
    return true;
  }
  else
  {
    XERR << "创建新线程失败 err=" << ret << XEND;
    return false;
  }
}

void xThread::thread_stop()
{
  if (thread_getState()==THREAD_INIT)
  {
    thread_setState(THREAD_FINISH);
  }

  while (thread_getState()!=THREAD_FINISH)
  {
    if (thread_getState()==THREAD_RUN)
      thread_setState(THREAD_STOP);
    sleep(1);
  }
}

void *xThread::thread_run(void *param)
{
  xThread *t = (xThread *)param;
  t->thread_proc();
  t->thread_setState(THREAD_FINISH);
  pthread_detach(pthread_self());
  return 0;
}

void xThread::thread_join()
{
  if (pid)
  {
    pthread_join(pid, NULL);
  }
}
