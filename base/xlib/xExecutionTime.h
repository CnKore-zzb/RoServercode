#ifndef _X_EXECUTIONTIME_H_
#define _X_EXECUTIONTIME_H_

#include "xTime.h"
#include "xDefine.h"
#include "xLog.h"
#include <map>
#include <string.h>

class xTimeTest //简单的执行时间测试函数
{
  public:
    xTimeTest(const std::string &logName);
    ~xTimeTest();
  private:
    std::string _logName;
    struct timespec _tv;
};

class xExecutionTime
{
  public:
    xExecutionTime(const DWORD &interval) : _log_timer(interval) {}
    ~xExecutionTime() {}
    void inc(const std::string &func, const QWORD total, const bool &addFlag);
    DWORD reset(const bool force_print);

  private:
    struct _stTimes
    {
      DWORD __times; //次数
      QWORD __total_time; //总耗时
      QWORD __max_time;
      bool __addFlag;
      _stTimes() { bzero(this, sizeof(*this)); }
    };
    typedef std::map<std::string, _stTimes> _Times;
    typedef _Times::iterator _Times_iter;
    _Times _times;
    xTimer _log_timer;
};

class ExecutionTimeWrap
{
  public:
    ExecutionTimeWrap(const std::string &func, bool addFlag = true);
    ~ExecutionTimeWrap();

    static xExecutionTime _et;

  private:
    const std::string _func;
    struct timespec _tv;
    bool _addFlag;
};

#define ExecutionTime_setInterval(intv)\
  xExecutionTime ExecutionTimeWrap::_et(intv);

#define ExecutionTime_Scope\
  ExecutionTimeWrap _exec_time_scope(__FILE__ ":" _S(__LINE__))

#define ExecutionTime_Scope_NoAdd\
  ExecutionTimeWrap _exec_time_scope(__FILE__ ":" _S(__LINE__), false)

#define ExecutionTime_Reset(flag)\
  ExecutionTimeWrap::_et.reset(flag);

class xCmdProcessTime
{
  public:
    xCmdProcessTime() {}
    ~xCmdProcessTime() {}

    void addTime(WORD cmd, QWORD elapse) { timeList[cmd].first += elapse;  timeList[cmd].second ++; }
    void print();

  private:
    typedef std::map<WORD, std::pair<QWORD, DWORD> > TimeList;
    TimeList timeList;
};

#endif
