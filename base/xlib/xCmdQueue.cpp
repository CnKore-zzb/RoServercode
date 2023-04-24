#include "xlib/xCmdQueue.h"

xCmdQueue::xCmdQueue()
{
  is_valid_ = true;
  m_dwWriteIndex = 0;
  m_dwReadIndex = 0;
}

xCmdQueue::~xCmdQueue()
{
  is_valid_ = false;
  clear();
}

void xCmdQueue::clear()
{
  while (get())
  {
    erase();
  }
  while (!m_oCmdQueue.empty())
  {
    CmdPair &cmd_pair = m_oCmdQueue.front();
    _mt_alloc.deallocate(cmd_pair.second, cmd_pair.first);
    m_oCmdQueue.pop();
  }
}

bool xCmdQueue::put(unsigned char *cmd, unsigned short len)
{
  if (!cmd || !len || !is_valid()) return false;
  BYTE *buf = (BYTE *)_mt_alloc.allocate(len);
  //BYTE *buf = (BYTE *)_mt_alloc.allocate(len);
  if (buf)
  {
    bcopy(cmd, buf, len);
    if (!putQueueToArray() && !m_oCmdArray[m_dwWriteIndex].first)
    {
      m_oCmdArray[m_dwWriteIndex].second.first = len;
      m_oCmdArray[m_dwWriteIndex].second.second = buf;
      m_oCmdArray[m_dwWriteIndex++].first = true;
      m_dwWriteIndex = m_dwWriteIndex % QUEUE_SIZE;
      return true;
    }
    else
    {
      m_oCmdQueue.push(std::make_pair(len, buf));
    }
    return true;
  }
  return false;
}

CmdPair* xCmdQueue::get()
{
//  if (m_dwReadIndex == m_dwWriteIndex) return NULL;

  CmdPair *ret = NULL;
  if (m_oCmdArray[m_dwReadIndex].first)
  {
    ret = &(m_oCmdArray[m_dwReadIndex].second);
  }
  return ret;
}

void xCmdQueue::erase()
{
  m_oCmdArray[m_dwReadIndex].first = false;
  _mt_alloc.deallocate(m_oCmdArray[m_dwReadIndex].second.second, m_oCmdArray[m_dwReadIndex].second.first);
  ++m_dwReadIndex;
  m_dwReadIndex = m_dwReadIndex % QUEUE_SIZE;
}

bool xCmdQueue::putQueueToArray()
{
  bool isLeft = false;
  while (!m_oCmdQueue.empty())
  {
    if (!m_oCmdArray[m_dwWriteIndex].first)
    {
      m_oCmdArray[m_dwWriteIndex].second = m_oCmdQueue.front();
      m_oCmdArray[m_dwWriteIndex++].first = true;
      m_dwWriteIndex = m_dwWriteIndex % QUEUE_SIZE;
      m_oCmdQueue.pop();
    }
    else
    {
      isLeft = true;
      break;
    }
  }
  return isLeft;
}
