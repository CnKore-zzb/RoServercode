#include "xTaskThreadPool.h"
#include "xServer.h"
#include "xTaskThread.h"
#include "xNetProcessor.h"

xTaskThreadPool::xTaskThreadPool(xServer *s):m_pServer(s)
{
}

xTaskThreadPool::~xTaskThreadPool()
{
  for (auto &it : m_list)
  {
    it->m_pThread->thread_stop();
    SAFE_DELETE(it->m_pThread);
    SAFE_DELETE(it);
  }
  m_list.clear();
}

TaskThreadItem* xTaskThreadPool::newTaskThread()
{
  if (!m_pServer) return nullptr;

  auto thread = new xTaskThread(m_pServer);
  if (!thread->thread_start())
  {
    XERR_T("[Task线程池], 创建失败");
    SAFE_DELETE(thread);
    return nullptr;
  }

  auto item = new TaskThreadItem;
  item->m_pThread = thread;
  m_list.push_front(item);

  XLOG_T("[Task线程池],创建成功,epfd:%u,总数量:%u", item->m_pThread->getEpfd(), m_list.size());

  return item;
}

TaskThreadItem* xTaskThreadPool::getATaskThreadItem()
{
  for (auto &it : m_list)
  {
    if (it->m_dwConnectNum < TASK_THREAD_MAX_CONNECT_NUM)
    {
      return it;
    }
  }

  return newTaskThread();
}

bool xTaskThreadPool::add(xNetProcessor *np)
{
  if (!np) return false;

  auto item = getATaskThreadItem();

  if (item)
  {
    np->addEpoll(item->m_pThread->getEpfd());
    ++item->m_dwConnectNum;
    XLOG_T("[Task线程池],%u,分配,epfd:%u, 当前线程连接数:%u, 总连接数:%u", np->getEpollFD(), item->m_pThread->getEpfd(), item->m_dwConnectNum, getCount());
    return true;
  }

  return false;
}

void xTaskThreadPool::del(xNetProcessor *np)
{
  if (!np) return;

  for (auto &it : m_list)
  {
    if (np->getEpollFD() == it->m_pThread->getEpfd())
    {
      if (it->m_dwConnectNum)
      {
        it->m_dwConnectNum--;
        XLOG_T("[Task线程池],%u,回收,epfd:%u, 当前线程连接数:%u, 总连接数:%u", np->getEpollFD(), it->m_pThread->getEpfd(), it->m_dwConnectNum, getCount());
      }
      return;
    }
  }
}

DWORD xTaskThreadPool::getCount()
{
  DWORD count = 0;
  for (auto &it : m_list)
  {
    count += it->m_dwConnectNum;
  }
  return count;
}
