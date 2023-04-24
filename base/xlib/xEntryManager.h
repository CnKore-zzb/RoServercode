#ifndef BASE_XLIB_XENTRYMANAGER_H_
#define BASE_XLIB_XENTRYMANAGER_H_

#include <tr1/unordered_map>
#include "xlib/xEntry.h"
//#include "xNewMem.h"

namespace std
{
  namespace tr1
  {
    template<>
      inline size_t hash<const char*>::operator()(const char* val) const
      {
        INT h = 0;
        for ( ; *val; ++val)
          h = 5 * h + *val;
        return size_t(h);
      };
  }
}

struct EqualStr
{
  bool operator()(const char* p1, const char*p2) const
  {
    return (strcmp(p1, p2) == 0);
  }
};

struct EqualUint
{
  bool operator()(const UINT &a1, const UINT &a2)const
  {
    return a1 == a2;
  }
};

struct xEntryCallBack
{
  virtual ~xEntryCallBack()
  {
  }
  virtual bool exec(xEntry *e) = 0;
};

template <typename KeyT, typename ValueT, typename EqualT>
struct MultiHash : private xNoncopyable
{
  public:
    MultiHash()
    {
      ets_.clear();
    }
    virtual ~MultiHash()
    {
      for (auto m = ets_.begin(); m != ets_.end(); ++m)
        SAFE_DELETE(m->second);
      ets_.clear();
    }

    bool forEach(xEntryCallBack &c)
    {
      for (Iter iter = ets_.begin(); iter != ets_.end(); )
      {
        Iter tempiter = iter++;
        if (!c.exec(tempiter->second)) return false;
      }
      return true;
    }
    INT size()
    {
      return ets_.size();
    }

  protected:
    typedef std::tr1::unordered_multimap<KeyT, ValueT, std::tr1::hash<KeyT>, EqualT> MultiHashMap;
    typedef typename MultiHashMap::const_iterator ConstIter;
    typedef typename MultiHashMap::iterator Iter;

    MultiHashMap ets_;
};

template <typename KeyT, typename ValueT, typename EqualT, typename CallbackT = xEntryCallBack, typename HashT = std::tr1::hash<KeyT> >
struct LimitHash : private xNoncopyable
{
  public:
    LimitHash()
    {
      ets_.clear();
    }
    virtual ~LimitHash()
    {
      for (auto m = ets_.begin(); m != ets_.end(); ++m)
        SAFE_DELETE(m->second);
      ets_.clear();
    }

    bool forEach(CallbackT &c)
    {
      for (Iter it = ets_.begin(); it != ets_.end(); )
      {
        Iter temp = it++;
        if (!c.exec(temp->second)) return false;
      }
      return true;
    }
    INT size()
    {
      return ets_.size();
    }

    template<class T> bool forEach2(T func)
    {
      for (Iter it = ets_.begin(); it != ets_.end(); )
      {
        Iter temp = it++;
        if (!func(temp->second)) return false;
      }
      return true;
    }

  public:
    typedef std::tr1::unordered_map<KeyT, ValueT, HashT, EqualT> HashMap;
    typedef typename HashMap::const_iterator ConstIter;
    typedef typename HashMap::iterator Iter;

    HashMap ets_;
};

template <int>
struct xEntryNone
{
  public:
    xEntryNone()
    {
    }
    bool push(xEntry* e)
    {
      return true;
    }
    void erase(xEntry* e)
    {
    }
};

struct xEntryID : public LimitHash<UINT, xEntry *, EqualUint>
{
  public:
    xEntryID();
    virtual ~xEntryID() {}

    bool push(xEntry *e);
    void erase(xEntry *e);
    xEntry* getEntryByID(UINT id);
    void find(UINT id, xEntry** e);
};

struct xEntryName : protected LimitHash<const char *, xEntry *, EqualStr>
{
  public:
    xEntryName();
    virtual ~xEntryName() {}

    bool push(xEntry *e);
    void erase(xEntry *e);
    void find(const char* name, xEntry** e);
    xEntry* getEntryByName(const char* name);
};

struct xEntryTempID : protected LimitHash<UINT, xEntry *, EqualUint>
{
  public:
    xEntryTempID();
    virtual ~xEntryTempID() {}

    bool push(xEntry *e);
    void erase(xEntry *e);
    xEntry* getEntryByTempID(UINT id);
    void find(UINT id, xEntry** e);
};

struct xEntryMultiName : protected MultiHash<const char *, xEntry *, EqualStr>
{
  public:
    xEntryMultiName();
    virtual ~xEntryMultiName() {}

    bool push(xEntry *e);

    void erase(xEntry *e);

    void find(const char* name, xEntry** e);

    xEntry* getEntryByName(const char* name);
};

template <typename e1, typename e2 = xEntryNone<1>, typename e3 = xEntryNone<2> >
struct xEntryManager : public e1, protected e2, protected e3
{
  public:
    xEntryManager()
    {
    }
    virtual ~xEntryManager()
    {
    }

    bool addEntry(xEntry *e)
    {
      if (e1::push(e))
      {
        if (e2::push(e))
        {
          if (e3::push(e))
          {
            return true;
          }
          else
          {
            e1::erase(e);
            e2::erase(e);
            return false;
          }
        }
        else
        {
          e1::erase(e);
          return false;
        }
      }
      return false;
    }

    void removeEntry(xEntry *e)
    {
      e1::erase(e);
      e2::erase(e);
      e3::erase(e);
    }

    bool forEach(xEntryCallBack &c)
    {
      return e1::forEach(c);
    }
    
    template<class T> 
    bool forEach2(T func)
    {
      return e1::forEach2(func);
    }

    INT size()
    {
      return e1::size();
    }
};
#endif  // BASE_XLIB_XENTRYMANAGER_H_
