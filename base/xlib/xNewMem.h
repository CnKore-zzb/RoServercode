#pragma once
#include <stdlib.h>
#include <map>

#ifdef _DEBUG
#define _MYNEW_
#define NEW new(__FILE__, __LINE__)
#else
#define NEW new
#endif

struct Meminfo
{
  std::string file;
  int line;
  Meminfo()
  {
    line = 0;
  }
};
struct MemSta
{
  public:
    static void add_use_mem(const void* p, const char* file, int line);
    static void del_use_mem(const void* p);
    static void printLeakMem();
  private:
    typedef std::map<const void*, Meminfo> MEM;
    static MEM Mems;
};

#ifdef _MYNEW_
#define SAFE_DELETE(p) do { MemSta::del_use_mem(p); delete p; p = NULL;} while(false)
#define SAFE_DELETE_VEC(p) do { MemSta::del_use_mem(p); delete[] p; p = NULL;} while(false)
#else
#define SAFE_DELETE(p) do { delete p; p = NULL;} while(false)
#define SAFE_DELETE_VEC(p) do {delete[] p; p = NULL;} while(false)
#endif

inline void * operator new(size_t size, const char *file, int line)// throw (std::bad_alloc)
{
  void *p = operator new(size);
#ifdef _MYNEW_
  MemSta::add_use_mem(p,file,line);
#endif
  return p;
}

inline void* operator new[](size_t size, const char *file, int line)
{
  void * p = operator new[](size);
#ifdef _MYNEW_
  MemSta::add_use_mem(p,file,line);
#endif
  return p;
}
inline void operator delete(void* p, const char *file, int line)//throw ()
{
  operator delete(p);
}

inline void operator delete [](void* p, const char *file, int line)//throw ()
{
  operator delete[](p);
}
