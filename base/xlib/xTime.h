#pragma once
#include <time.h>
#include <string>
#include "xDefine.h"
#include <vector>
#include <utility>

class xTime
{
  public:
    xTime();
    ~xTime();

    // 1秒 = 1000毫秒 = 1000000微秒

    // 微秒
    static QWORD getCurUSec();
    // 毫秒
    static QWORD getCurMSec();
    // 秒
    static DWORD getCurSec();

    static time_t getMinuteStart(time_t time);
    static time_t getDayStart(time_t time);
    static time_t getDayStart(time_t time, DWORD offsetSec);
    static time_t getDayStartAddDay(time_t time, DWORD offsetSec, DWORD day);
    static time_t getDayStart(DWORD year, DWORD month, DWORD day);
    static time_t getWeekStart(time_t time, DWORD offsetSec = 0);
    static time_t getMonthStart(time_t time, DWORD offsetSec = 0);
    static bool isSameDay(time_t t1, time_t t2);
    static bool isSameWeek(time_t t1,time_t t2);
    static bool isSameMonth(time_t t1,time_t t2, DWORD offsetSec/*=0*/);
    static bool isCurMonth(DWORD dwYear, DWORD dwMonth, DWORD offsetSec = 5*3600);
    static bool isValidDate(DWORD year, DWORD month, DWORD day);  //检查日期是否合法
    static bool isLeapYear(DWORD year);
    static int getDay(time_t time);
    static int getMonth(time_t time);
    static int getYear(time_t time);
    static int getWeek(time_t time);
    static int getHour(time_t time);
    static int getMin(time_t time);
    static bool isBetween(WORD beginHour,WORD beginMin,WORD endHour,WORD endMin,DWORD curTime);
    // 获取两个时间相差多少天， t1 < t2 为负值
    static int getDiffDay(time_t t1, time_t t2, DWORD offsetSec=0);

    // 只能往后设置
    static bool setTime(std::string str);
    static void setAdjust(DWORD adj)
    {
      adjust = adj;
    }
    // 修正的秒数
    static DWORD adjust;

    void now();

    void elapseStart();

    DWORD elapse();
    QWORD milliElapse();
    QWORD uElapse();

    bool elapse(DWORD s);
    bool milliElapse(QWORD m);
    bool uElapse(QWORD u);

    //cur均以微秒为单位
    bool elapse(DWORD s,QWORD cur);
    bool milliElapse(QWORD m,QWORD cur);
    bool uElapse(QWORD u,QWORD cur);

    QWORD usec;
    QWORD _elapse;
};

inline DWORD now()
{
  return xTime::getCurSec();
}

inline void getLocalTime(tm &_tm)
{
  time_t _t = now();
  localtime_r(&_t, &_tm);
}

inline void getLocalTime(tm &_tm, const time_t &_t)
{
  localtime_r(&_t, &_tm);
}

inline void getLocalTime(tm &_tm, const DWORD &_t)
{
  time_t t = _t;
  localtime_r(&t, &_tm);
}

inline void parseTime(const char *str, DWORD &time)
{
  struct tm tm1;
  sscanf(str, "%4d-%2d-%2d %2d:%2d:%2d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday, &tm1.tm_hour, &tm1.tm_min, &tm1.tm_sec);
  tm1.tm_year -= 1900;
  tm1.tm_mon--;
  tm1.tm_isdst = -1;

  time = mktime(&tm1);
}

/*20180511*/
inline DWORD getFormatDate()
{
  DWORD cur = now();
  return xTime::getYear(cur) * 10000 + xTime::getMonth(cur) * 100 + xTime::getDay(cur);
}

inline DWORD getFormatDate(DWORD dwTime)
{
  return xTime::getYear(dwTime) * 10000 + xTime::getMonth(dwTime) * 100 + xTime::getDay(dwTime);
}

/* 获取带偏移时间的日期，比如偏移时间是早上五点，大于早上五点的时间算作今天，小于早上五点的时间算作昨天 */
inline DWORD getOffsetDate(DWORD dwOffsetSec)
{
  DWORD cur = now();
  DWORD curDayStart = xTime::getDayStart(cur);
  if (curDayStart + dwOffsetSec <= cur)
  {
    return xTime::getYear(cur) * 10000 + xTime::getMonth(cur) * 100 + xTime::getDay(cur);
  }
  else
  {
    cur -= 24 * 60 * 60;
    return xTime::getYear(cur) * 10000 + xTime::getMonth(cur) * 100 + xTime::getDay(cur);
  }
}

class xTimer
{
  public:
    xTimer(DWORD t) { elapse = t; }
    ~xTimer(){}
    bool timeUp(QWORD curSec) { return time.elapse(elapse, curSec * ONE_MILLION); }
    void reset() { time.elapseStart(); }
    inline DWORD getElapse() const { return elapse; }
  private:
    xTime time;
    DWORD elapse;
};

class xMilliTimer
{
  public:
    xMilliTimer(DWORD t) { elapse = t; }
    ~xMilliTimer(){}
    bool timeUp(QWORD cur) { return time.milliElapse(elapse,cur); }
    void reset() { time.elapseStart(); }
  private:
    xTime time;
    DWORD elapse;
};

//定时器  以天为周期  可设置每天几点执行一次
class xDayTimer
{
  public:
    xDayTimer(BYTE h, BYTE d=5)
    {
      _hour = h;
      _last_done = 0;
      _delay = d;
    }
    ~xDayTimer() {}
    bool timeUp(struct tm &tm_cur);
    bool timeUp(QWORD curSec);
  private:
    BYTE _hour;
    SWORD _last_done;
    BYTE _delay;
};

//定时器
class xTimer2
{
public:
  xTimer2(std::vector<struct tm>& tms, DWORD delay = 10);

  bool timeUp(QWORD curSec, struct tm* pTm);
private:
  bool timeUp(struct tm &tm_cur, QWORD curSec, struct tm* pTm);
  DWORD m_delay;
  std::vector<std::pair<struct tm, QWORD>> m_vecTms;  
};

std::string getAscTime(const DWORD &dwTime);

class Benchmarck
{
public:
  Benchmarck(const std::string &name, DWORD p1, DWORD limitMS) :m_name(name), m_p(p1), m_desc(""), m_limit(limitMS) {};
  Benchmarck(const std::string &name, DWORD p1, const std::string &desc);
  ~Benchmarck();
  void Output(const std::string &subName);
private:
  std::string m_name;
  DWORD m_p;
  std::string m_desc;
  DWORD m_limit = 0;    //毫秒
  xTime m_time;
};

const static DWORD kDBMaxLogTime = 100;
class DBTimeCheck
{
public:
  DBTimeCheck(const std::string &desc);
  ~DBTimeCheck();
private:
  DWORD m_starttime;    //毫秒
  std::string m_desc;
};

class FunTimeCheck
{
public:
  FunTimeCheck(const std::string &file, const std::string &func, DWORD line, DWORD m_limiMSec);
  ~FunTimeCheck();
private:
  xTime m_frameTimer;
  std::string m_file;
  std::string m_func;
  DWORD m_line;
  DWORD m_limitMSec = 0;
};

struct SWheel
{
  DWORD dwTime = 0;
  bool bDone = false;

  std::function<void()> func;
};
class xTimeWheel
{
  public:
    xTimeWheel(bool pass = false);
    ~xTimeWheel();

    bool empty() const { return m_vecWheels.empty(); }
    void clear() { m_vecWheels.clear(); }

    void addWheel(const SWheel& rWheel);
    void timer(DWORD curSec);
  private:
    void reset();
  private:
    std::vector<SWheel> m_vecWheels;

    DWORD m_dwStartDay = 0;
    bool m_bPass = false;
};


//microsec:超过这个微妙才输出
#define FUN_TIMECHECK(microsec) FunTimeCheck ftc(__FILE__,__FUNCTION__,__LINE__, microsec)
#define FUN_TIMECHECK_30() FUN_TIMECHECK(30*1000)

#define FUN_TRACE() FUN_TIMECHECK(30*1000)
