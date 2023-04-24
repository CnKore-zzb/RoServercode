#pragma once
#include <list>
#include "xDefine.h"
#include "xLog.h"

class xServer;
class xTaskThread;
class xNetProcessor;

// #define TASK_THREAD_MAX_CONNECT_NUM 320
#define TASK_THREAD_MAX_CONNECT_NUM 128

struct TaskThreadItem
{
  xTaskThread *m_pThread = nullptr;
  volatile DWORD m_dwConnectNum = 0;
};

class xTaskThreadPool
{
  public:
    xTaskThreadPool(xServer *s);
    ~xTaskThreadPool();

  public:
    bool add(xNetProcessor *np);
    void del(xNetProcessor *np);

  private:
    TaskThreadItem* newTaskThread();
    TaskThreadItem* getATaskThreadItem();
    DWORD getCount();

  private:
    std::list<TaskThreadItem *> m_list;
    xServer *m_pServer = nullptr;
};
