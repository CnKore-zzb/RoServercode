#include <time.h>
#include "xExecutionTime.h"

xTimeTest::xTimeTest(const std::string &logName) : _logName(logName)
{
  clock_gettime(CLOCK_REALTIME, &_tv);
}

xTimeTest::~xTimeTest()
{
  QWORD begin = (QWORD)_tv.tv_sec * 1000000000L + _tv.tv_nsec;
  clock_gettime(CLOCK_REALTIME, &_tv);
  QWORD end = (QWORD)_tv.tv_sec * 1000000000L + _tv.tv_nsec;
  XDBG << "[" << _logName.c_str() << "]," << (end - begin) / 1000 << "微秒" << XEND;
}

void xExecutionTime::inc(const std::string &func, const QWORD total, const bool &addFlag)
{
  _stTimes &st = _times[func];
  ++st.__times;
  st.__total_time += total;
  st.__addFlag = addFlag;
  if (total > st.__max_time)
  {
    st.__max_time = total;
  }
}

DWORD xExecutionTime::reset(const bool force_print)
{
  if (force_print || _log_timer.timeUp(now()))
  {
    QWORD totalTime = 0;
    XLOG << "[执行时间统计]" << (force_print ? "强制" : "时间") << ",间隔:" << _log_timer.getElapse() << ",大小:" << (DWORD)_times.size() << XEND;
    for (_Times_iter it = _times.begin(); it != _times.end(); ++it)
    {
      if (it->second.__times && it->second.__addFlag)
        totalTime += it->second.__total_time;
    }

    for (_Times_iter it = _times.begin(); it != _times.end(); ++it)
    {
      if (it->second.__times && !it->second.__addFlag)
      {
        XLOG << "[执行时间统计]" << it->second.__total_time * 100.0f / totalTime << "%" << it->first.c_str() << it->second.__total_time / 1000L
          << "微秒, " << it->second.__times << "次, " << it->second.__total_time / 1000L / it->second.__times << "微秒/次" << it->second.__max_time / 1000L << XEND;
      }
    }
    XLOG << "[执行时间统计]-------------------------------" << XEND;
    for (_Times_iter it = _times.begin(); it != _times.end(); ++it)
    {
      if (it->second.__times && it->second.__addFlag)
      {
        XLOG << "[执行时间统计]" << it->second.__total_time * 100.0f / totalTime << "%, " << it->first.c_str() << ", " << it->second.__total_time / 1000L <<
          "微秒, " << it->second.__times << "次, " << it->second.__total_time / 1000L / it->second.__times << "微秒/次" << it->second.__max_time / 1000L << XEND;
      }
    }
    XLOG << "[执行时间统计]总时间:" << totalTime / 1000L << "微秒,间隔:" << _log_timer.getElapse() << XEND;
    _times.clear();
    if (_log_timer.getElapse())
    {
      return totalTime / 1000000000L * 100 / _log_timer.getElapse();
    }
  }
  return 0;
}

ExecutionTimeWrap::ExecutionTimeWrap(const std::string &func, bool addFlag) : _func(func), _addFlag(addFlag)
{
  clock_gettime(CLOCK_REALTIME, &_tv);
}

ExecutionTimeWrap::~ExecutionTimeWrap()
{
  QWORD begin = (QWORD)_tv.tv_sec * 1000000000L + _tv.tv_nsec;
  clock_gettime(CLOCK_REALTIME, &_tv);
  QWORD end = (QWORD)_tv.tv_sec * 1000000000L + _tv.tv_nsec;

  _et.inc(_func, end - begin, _addFlag);
}

void xCmdProcessTime::print()
{
  typedef std::multimap<QWORD, std::pair<WORD, DWORD> > TList;
  TList list;

  for (TimeList::iterator it = timeList.begin(); it != timeList.end(); ++it)
  {
    if (it->second.second > 0)
      list.insert(std::make_pair(it->second.first, std::make_pair(it->first, it->second.second)));
  }
  QWORD totalTime = 0;
  XLOG << "[消息处理时间统计],次数    单次时间    总耗时" << XEND;
  for (TList::reverse_iterator it=list.rbegin(); it!=list.rend(); ++it)
  {
    totalTime += it->first;
    XLOG << "[消息处理时间统计],(" << (it->second.first & 0xff) << "," << ((it->second.first >> 8) & 0xff) << "),\t" << it->second.second << "," << it->first / it->second.second << "," << it->first << XEND;
  }
  XLOG << "[消息处理时间统计],总耗时:" << totalTime << XEND;
  timeList.clear();
}

