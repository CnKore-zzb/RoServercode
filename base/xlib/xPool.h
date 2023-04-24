/**
 * @file xPool.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-08-26
 */

#pragma once

#include <stdlib.h>
#include <vector>
#include "xSingleton.h"

using std::vector;

template<typename T, int DEFAULT_SIZE = 10>
class xPool : public xSingleton<xPool<T, DEFAULT_SIZE>>
{
  friend class xSingleton<xPool<T, DEFAULT_SIZE>>;
  private:
    xPool() {}
  public:
    virtual ~xPool() {}

    void* alloc_mem()
    {
      if (m_vecMem.empty())
        init();
      if (m_vecMem.empty() == true)
        return nullptr;
      void* p = m_vecMem.back();
      m_vecMem.pop_back();
      return p;
    }
    void free_mem(void* p)
    {
      if (p == nullptr)
        return;

      m_vecMem.push_back(p);
    }
  private:
    void init()
    {
      for (int i = 0; i < DEFAULT_SIZE; ++i)
      {
        void* p = malloc(sizeof(T));
        if (p != nullptr)
          m_vecMem.push_back(p);
      }
    }
  private:
    vector<void*> m_vecMem;
};

template<typename T, int DEFAULT_SIZE = 10>
class xObjectPool
{
  /*public:
    inline void* operator new(size_t size) throw()
    {
      if (size == 0)
        return nullptr;
      return xPool<T, DEFAULT_SIZE>::getMe().alloc_mem();
    }
    inline void operator delete(void* p, size_t size)
    {
      if (p == nullptr || size == 0)
        return;
      xPool<T, DEFAULT_SIZE>::getMe().free_mem(p);
    }*/
};

