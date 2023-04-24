#pragma once
#include <set>

template <typename P, typename T>
class xSignal
{
  public:
    xSignal() {}
    ~xSignal() {}

    void connect(P p) { pList.push_back(p); };
    void disconnect(P p) { pList.erase(p); };

    void clear()
    {
      for (PIter it = pList.begin(); it != pList.end(); it++)
      {
        (*it).clear();
      }
    }

    void operator()() const
    {
      for (PIter it = pList.begin(); it != pList.end(); it++)
      {
        (*it)();
      }
    };

    void operator()(T t)
    {
      PIter iter = pList.begin(), temp;
      while (iter != pList.end())
      {
        temp = iter++;
        AI_RET_ENUM ret = (*temp)(t);
        if (ret == AI_RET_RUN_SUCC_CONT || ret == AI_RET_COND_FAIL_CONT)
          continue;
        if (ret == AI_RET_COND_FAIL)
          return;

        (*temp).clear();
        pList.erase(temp);
      }
    };

  private:
    typedef typename std::list<P> PList;
    typedef typename std::list<P>::iterator PIter;
    PList pList;
};
