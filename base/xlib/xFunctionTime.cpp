#include <time.h>
#include "xFunctionTime.h"

void xFunctionTime::inc(const QWORD &addr, const QWORD &total)
{
  _stTimes &st = _times[addr];
  ++st.__times;
  st.__total_time += total;
}

void xFunctionTime::reset(const bool force_print)
{
  if ((force_print || _log_timer.timeUp(now())) && !_times.empty())
  {
    XDBG << "[函数时间统计]" << (force_print ? "强制" : "时间") << "间隔:" << _log_timer.getElapse() << "大小:" << _times.size() << XEND;
    for (_Times_iter it = _times.begin(); it != _times.end(); ++it)
    {
      if (it->second.__times)
      {
        /*
           XDBG("[函数时间统计]地址:%llu, %llu毫秒, %u次, %u毫秒/次", it->first, 
           it->second.__total_time / 1000000L, it->second.__times, 
           it->second.__total_time / 1000000L / it->second.__times);
        // */
        XDBG << "[函数时间统计]地址:" << it->first << it->second.__total_time / 1000L << "微秒"
          << it->second.__times << "次" << it->second.__total_time / 1000L / it->second.__times << "微秒/次" << XEND;
      }
    }
    _times.clear();
  }
}

FunctionTimeWrap::FunctionTimeWrap(const QWORD &addr) : _addr(addr)
{
  clock_gettime(CLOCK_REALTIME, &_tv);
}

FunctionTimeWrap::~FunctionTimeWrap()
{
  QWORD begin = (QWORD)_tv.tv_sec * 1000000000L + _tv.tv_nsec;
  clock_gettime(CLOCK_REALTIME, &_tv);
  QWORD end = (QWORD)_tv.tv_sec * 1000000000L + _tv.tv_nsec;

  _ft.inc(_addr, end - begin);
}

