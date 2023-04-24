#ifndef _X_FUNCTIONTIME_H_
#define _X_FUNCTIONTIME_H_

#include "xTime.h"
#include "xDefine.h"
#include "xLog.h"
#include <map>
#include <string>

class xFunctionTime
{
  public:
    xFunctionTime(const DWORD &interval) : _log_timer(interval) {}
    ~xFunctionTime() {}
    void inc(const QWORD &addr, const QWORD &total);
    void reset(const bool force_print);

  private:
    struct _stTimes
    {
      DWORD __times; //次数
      QWORD __total_time; //总耗时
      _stTimes() { bzero(this, sizeof(*this)); }
    };
    typedef std::map<QWORD, _stTimes> _Times;
    typedef _Times::iterator _Times_iter;
    _Times _times;

    xTimer _log_timer;
};

class FunctionTimeWrap
{
  public:
    FunctionTimeWrap(const QWORD &addr);
    ~FunctionTimeWrap();

    static xFunctionTime _ft;

  private:
    const QWORD _addr;
    struct timespec _tv;
};

#define FunctionTime_setInterval(intv)\
  xFunctionTime FunctionTimeWrap::_ft(intv);

#define FunctionTime_Scope\
  FunctionTimeWrap _func_time_scope(reinterpret_cast<QWORD>(__builtin_return_address(0)))

#define FunctionTime_Reset(flag)\
  FunctionTimeWrap::_ft.reset(flag);

#endif
