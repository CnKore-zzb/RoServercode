#pragma once

#include <vector>
#include <queue>

#include "xlib/xDefine.h"
#include "xlib/xLog.h"

template <typename ClassT, int MAX_ARRAY_SIZE = 102400>
class xQueue
{
  public:
    xQueue()
    {
    }
    ~xQueue()
    {
      m_blIsValid = false;
      clear();
    }

  public:
    bool put(ClassT *t)
    {
      if (!t || !isValid()) return false;

      if (!putQueueToArray() && !m_oArray[m_dwWriteIndex].first)
      {
        // XLOG_T("[xQueue-put],%p,%u", t, m_dwWriteIndex);
        m_oArray[m_dwWriteIndex].second = t;
        m_oArray[m_dwWriteIndex].first = true;
        ++m_dwWriteIndex;
        m_dwWriteIndex = m_dwWriteIndex % MAX_ARRAY_SIZE;
        return true;
      }
      else
      {
        XLOG_T("[xQueue-put],%p,add queue", t);
        m_oQueue.push(t);
      }
      return true;
    }

    ClassT* get()
    {
      ClassT *pRet = nullptr;
      if (m_oArray[m_dwReadIndex].first)
      {
        pRet = m_oArray[m_dwReadIndex].second;
      }
      return pRet;
    }
    void pop()
    {
      // XLOG_T("[xQueue-pop],%p,%u", m_oArray[m_dwReadIndex].second, m_dwReadIndex);
      m_oArray[m_dwReadIndex].second = nullptr;
      m_oArray[m_dwReadIndex].first = false;
  //    SAFE_DELETE(m_oArray[m_dwReadIndex].second);
      ++m_dwReadIndex;
      m_dwReadIndex = m_dwReadIndex % MAX_ARRAY_SIZE;
    }

    bool isValid() { return m_blIsValid; }

    DWORD size()
    {
      if (m_dwWriteIndex >= m_dwReadIndex)
      {
        return m_dwWriteIndex - m_dwReadIndex;
      }
      else
      {
        return MAX_ARRAY_SIZE - m_dwReadIndex + m_dwWriteIndex;
      }
    }

    bool final_check_queue()
    {
      if (m_oQueue.empty())
      {
        return true;
      }
      XLOG_T("[xQueue-queue],size:%u", m_oQueue.size());
      putQueueToArray();
      return false;
    }

  private:
    // 和pop相比会删除对象
    void erase()
    {
      SAFE_DELETE(m_oArray[m_dwReadIndex].second);
      m_oArray[m_dwReadIndex].first = false;
      ++m_dwReadIndex;
      m_dwReadIndex = m_dwReadIndex % MAX_ARRAY_SIZE;
    }
    void clear()
    {
      while (get())
      {
        erase();
      }
      while (!m_oQueue.empty())
      {
        ClassT *p = m_oQueue.front();
        SAFE_DELETE(p);
        m_oQueue.pop();
      }
    }
    bool putQueueToArray()
    {
      bool isLeft = false;
      while (!m_oQueue.empty())
      {
        if (!m_oArray[m_dwWriteIndex].first)
        {
          m_oArray[m_dwWriteIndex].second = m_oQueue.front();
          m_oArray[m_dwWriteIndex].first = true;
          ++m_dwWriteIndex;
          m_dwWriteIndex = m_dwWriteIndex % MAX_ARRAY_SIZE;
          m_oQueue.pop();
        }
        else
        {
          isLeft = true;
          break;
        }
      }
      return isLeft;
    }

  private:
    std::pair<volatile bool, ClassT *> m_oArray[MAX_ARRAY_SIZE];
    std::queue<ClassT *> m_oQueue;
    DWORD m_dwWriteIndex = 0;
    DWORD m_dwReadIndex = 0;

    volatile bool m_blIsValid = true;

};
