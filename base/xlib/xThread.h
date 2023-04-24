#pragma once
#include <pthread.h>
#include "xNoncopyable.h"

class xThread : private xNoncopyable
{
  public:
    enum ThreadState
    {
      THREAD_INIT = 0,   //after new, before start()
      THREAD_RUN,        //in while()
      THREAD_STOP,       //stopping
      THREAD_FINISH,     //can delete
    };

    xThread();
    virtual ~xThread() {}

    bool thread_start();
    virtual bool thread_init() { return true; }
    virtual void thread_stop();
    void thread_join();

    void thread_setState(ThreadState s) { state = s; }
    ThreadState thread_getState() { return state; }

    static void *thread_run(void *param);

  protected:
    volatile ThreadState state;
    virtual void thread_proc() = 0;

  protected:
    pthread_t pid;
};
