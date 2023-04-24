#include "xTime.h"
#include <time.h>
#include <sys/time.h>
#include "xTools.h"

DWORD xTime::adjust = 0;

xTime::xTime()
{
  now();
  _elapse = usec;
}

xTime::~xTime()
{
}

void xTime::now()
{
  usec = getCurUSec();
}

DWORD xTime::getCurSec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return tv.tv_sec + adjust;
}

QWORD xTime::getCurUSec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return ((QWORD)tv.tv_sec + adjust) * ONE_MILLION + tv.tv_usec;
}

QWORD xTime::getCurMSec()
{
  return getCurUSec()/ONE_THOUSAND;
}

time_t xTime::getMinuteStart(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return time - tm.tm_sec;
}

time_t xTime::getDayStart(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return time - (tm.tm_hour * 60 + tm.tm_min) * 60 - tm.tm_sec;//直接计算比使用mktime函数效率更高
}

time_t xTime::getDayStart(time_t time, DWORD offsetSec)
{
  time_t newTime = time - offsetSec;
  return getDayStart(newTime);
}

time_t xTime::getDayStartAddDay(time_t time, DWORD offsetSec, DWORD day)
{
  time_t newTime = time - offsetSec;
  return getDayStart(newTime) + day * DAY_T;
}

time_t xTime::getDayStart(DWORD year, DWORD month, DWORD day)
{
  if (year < 1970 || month < 1 || month > 12 || day > 31)
    return 0;

  struct tm tm;
  bzero(&tm, sizeof(tm));
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  return mktime(&tm);
}

//以周一早上0点为起始时间,offset 
time_t xTime::getWeekStart(time_t time, DWORD offsetSec/*=0*/)
{
  struct tm tm;
  time_t newTime = time - offsetSec;
  getLocalTime(tm, newTime);
  return newTime - ((((tm.tm_wday + 6) % 7) * 24 + tm.tm_hour) * 60 + tm.tm_min) * 60 - tm.tm_sec;
}

time_t xTime::getMonthStart(time_t time, DWORD offsetSec/*=0*/)
{
  struct tm tm;
  time_t newTime = time - offsetSec;
  getLocalTime(tm, newTime);
  return newTime - (((tm.tm_mday - 1) * 24 + tm.tm_hour) * 60 + tm.tm_min) * 60 - tm.tm_sec;
}

const DWORD MONTH_DAY_NUM[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
bool xTime::isValidDate(DWORD year, DWORD month, DWORD day)
{
  if (month < 1 || month > 12 || day < 1 || day > 31)
    return false;

  return day <= MONTH_DAY_NUM[month] + ((month == 2 && isLeapYear(year)) ? 1 : 0);
}

bool xTime::isLeapYear(DWORD year)
{
  return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

bool xTime::isSameDay(time_t t1, time_t t2)
{
  return getDayStart(t1)==getDayStart(t2);
}
bool xTime::isSameWeek(time_t t1,time_t t2)
{
  return getWeekStart(t1)==getWeekStart(t2); 
}
bool xTime::isSameMonth(time_t t1,time_t t2, DWORD offsetSec/*=0*/)
{
  return getMonthStart(t1, offsetSec)==getMonthStart(t2, offsetSec);
}

bool xTime::isCurMonth(DWORD dwYear, DWORD dwMonth, DWORD offsetSec/* = 5*3600*/)
{
  DWORD curSec = ::now();
  curSec -= offsetSec;
  DWORD year = xTime::getYear(curSec);
  DWORD month = xTime::getMonth(curSec);
  if (year == dwYear && month == dwMonth)
    return true;
  return false;
}

int xTime::getDay(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return tm.tm_mday;
}

int xTime::getMonth(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return tm.tm_mon+1;
}

int xTime::getYear(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return tm.tm_year+1900;
}

int xTime::getWeek(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return tm.tm_wday;
}

int xTime::getHour(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return tm.tm_hour;
}

int xTime::getMin(time_t time)
{
  struct tm tm;
  getLocalTime(tm, time);
  return tm.tm_min;
}

void xTime::elapseStart()
{
  _elapse = getCurUSec();
}

DWORD xTime::elapse()
{
  return (DWORD)(uElapse() / ONE_MILLION);
}

bool xTime::elapse(DWORD s)
{
  if (elapse()>=s)
  {
    _elapse += s * ONE_MILLION;
    while (elapse()>=s)
      _elapse += s * ONE_MILLION;
    return true;
  }
  return false;
}

QWORD xTime::milliElapse()
{
  return uElapse()/1000;
}

bool xTime::milliElapse(QWORD m)
{
  if (milliElapse()>=m)
  {
    _elapse += m*1000;
    while (elapse()>=m)
      _elapse += m*1000;
    return true;
  }
  return false;
}

QWORD xTime::uElapse()
{
  QWORD cur = getCurUSec();

  if (cur>=_elapse)
    return cur-_elapse;
  else
    return 0;
}

bool xTime::uElapse(QWORD u)
{
  if (uElapse()>=u)
  {
    _elapse += u;
    while (elapse()>=u)
      _elapse += u;
    return true;
  }
  return false;
}

bool xTime::elapse(DWORD s,QWORD cur)
{
  s = s * ONE_MILLION;
  if( cur >= (_elapse+s) )
  {
    _elapse += s;
    while( cur >= (_elapse+s) )
      _elapse += s;
    return true;
  }
  return false;
}

bool xTime::milliElapse(QWORD m,QWORD cur)
{
  m = m*1000;
  if( cur>=(_elapse+m) )
  {
    _elapse += m;
    while( cur>=(_elapse+m) )
      _elapse += m;
    return true;
  }
  return false;
}

bool xTime::uElapse(QWORD u,QWORD cur)
{
  if( cur>=(_elapse+u) )
  {
    _elapse += u;
    while( cur>=(_elapse+u) )
      _elapse += u; 
    return true;
  }
  return false;
}

bool xTime::isBetween(WORD beginHour,WORD beginMin,WORD endHour,WORD endMin,DWORD curTime)
{
  DWORD dayElapse = curTime - getDayStart(curTime);
  return (dayElapse>=(DWORD)(beginHour*60*60+beginMin*60) && dayElapse<=(DWORD)(endHour*60*60+endMin*60));
}

int xTime::getDiffDay(time_t t1, time_t t2, DWORD offsetSec /*=0*/)
{
  time_t d1 = getDayStart(t1, offsetSec);
  time_t d2 = getDayStart(t2, offsetSec);

  int day = (d1 - d2) / (24 * 3600);
  return day;
}

std::string getAscTime(const DWORD &dwTime)
{
  char str[128];
  bzero(str, sizeof(str));

  struct tm tm;
  getLocalTime(tm, (time_t)time);

  snprintf(str, sizeof(str), "%u-%u-%u %u:%u", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);

  return str;
}

bool xTime::setTime(std::string str)
{
  DWORD value = 0;
  parseTime(str.c_str(), value);
  if ((DWORD)-1==value) return false;

  DWORD cur = getCurSec();
  if (cur > value)
  {
    return false;
  }
  else
  {
    DWORD add = value - cur;
    adjust += add;
    XLOG << "[系统时间],设置系统时间为:" << str.c_str() << ",adjust:" << adjust << XEND;
  }

  return true;
}

bool xDayTimer::timeUp(struct tm &tm_cur)
{
  if (_last_done != tm_cur.tm_yday)
  {
    if (tm_cur.tm_hour == _hour)
    {
      if (tm_cur.tm_min <= _delay)
      {
        _last_done = tm_cur.tm_yday;
        return true;
      }
      _last_done = tm_cur.tm_yday;
    }
    return false;
  }
  return false;
}

bool xDayTimer::timeUp(QWORD curSec)
{
  struct tm *pLocalTime = localtime((const time_t*)&curSec);
  if (pLocalTime == nullptr)
    return false;
  return timeUp(*pLocalTime);
}

xTimer2::xTimer2(std::vector<struct tm>& tms, DWORD delay)
{
  m_vecTms.clear();

  for (auto &v : tms)
  {
    m_vecTms.push_back(std::make_pair(v,0));
  }
  m_delay = delay;
}

bool xTimer2::timeUp(QWORD curSec, struct tm* pTm)
{
  struct tm *pLocalTime = localtime((const time_t*)&curSec);
  if (pLocalTime == nullptr)
    return false;
  return timeUp(*pLocalTime, curSec, pTm);
}

bool xTimer2::timeUp(struct tm &tm_cur, QWORD curSec, struct tm* pTm)
{
  bool ret = false;
  for (auto &v : m_vecTms)
  {
    if (m_delay != 0 && curSec <= v.second +m_delay )
      continue;
    
    struct tm t;
    t = tm_cur;         //waring v.first counld not be zone
    if (v.first.tm_mday >= 0) t.tm_mday = v.first.tm_mday;
    //if (v.first.tm_wday >= 0) t.tm_wday = v.first.tm_wday;
    if (v.first.tm_hour >= 0) t.tm_hour = v.first.tm_hour;
    if (v.first.tm_min >= 0) t.tm_min = v.first.tm_min;
    if (v.first.tm_sec >= 0) t.tm_sec = v.first.tm_sec;

    QWORD ttime = mktime(&t);     //忽略传进去的 tm_wday tm_yday

    if (v.first.tm_wday >= 0)
    {
      if (t.tm_wday != v.first.tm_wday)
        continue;
    }
    
    if (curSec >= ttime && curSec <= ttime + m_delay)
    {
      v.second = curSec;
      ret = true;
      XDBG << "[定时器-到点] " << "mday:" << v.first.tm_mday << "wday:" << v.first.tm_wday << "hour:" << v.first.tm_hour << "min:" << v.first.tm_min << "sec:" << v.first.tm_sec << XEND;
      if (pTm)
      {
        *pTm = v.first;
      }
      break;
    }  
  }
  return ret;
}


Benchmarck::Benchmarck(const std::string &name, DWORD p, const std::string &desc) :m_name(name),m_desc(desc)
{
  m_p = p;
}

void Benchmarck::Output(const std::string &subName)
{
  QWORD msec = m_time.uElapse() / 1000;
  if (msec < m_limit)
    return;
  XLOG << "[Benchmarck]" << "性能测试," << m_name << "subName" << subName << ",参数:" << m_p << ",耗时" << m_time.uElapse() / 1000 << "毫秒"<<",详细"<<m_desc << XEND;
  m_time.elapseStart();
}

Benchmarck::~Benchmarck()
{
  Output("结尾");
}

DBTimeCheck::DBTimeCheck(const std::string &desc) :m_desc(desc)
{
  m_starttime = xTime::getCurMSec();
}

DBTimeCheck::~DBTimeCheck()
{
  DWORD endtime = xTime::getCurMSec();
  DWORD lasttime = endtime - m_starttime;
  if (lasttime >= kDBMaxLogTime)
    XLOG << "[DBTimeCheck]" << " 耗时：" << lasttime << "毫秒" << "数据库语句: " << m_desc << XEND;
}

FunTimeCheck::FunTimeCheck(const std::string &file, const std::string &func, DWORD line, DWORD nSec) : m_file(file),
  m_func(func),
  m_line(line),
  m_limitMSec(nSec)
{
  m_frameTimer.elapseStart();
}

FunTimeCheck::~FunTimeCheck()
{
  QWORD _e = m_frameTimer.uElapse();
  if (_e >= m_limitMSec)
    XLOG << "[FunTimeCheck]" << m_file << m_func << m_line << _e << "微秒" << XEND;
}

// xTime wheel
xTimeWheel::xTimeWheel(bool pass /*= false*/) : m_bPass(pass)
{
  m_dwStartDay = xTime::getDayStart(now());
}

xTimeWheel::~xTimeWheel()
{
}

void xTimeWheel::addWheel(const SWheel& rWheel)
{
  DWORD dwNow = now();
  m_vecWheels.push_back(rWheel);

  SWheel& rBack = m_vecWheels.back();
  if (!m_bPass)
  {
    DWORD dwWheelStart = xTime::getDayStart(dwNow) + rWheel.dwTime;
    if (dwNow >= dwWheelStart)
      rBack.bDone = true;
  }
  XDBG << "[TimeWheel-新增] done :" << rBack.bDone << "time :" << rBack.dwTime << XEND;
}

void xTimeWheel::timer(DWORD curSec)
{
  if (curSec - m_dwStartDay >= DAY_T)
    reset();

  for (auto &v : m_vecWheels)
  {
    if (v.bDone || m_dwStartDay + v.dwTime > curSec)
      continue;
    if (v.func)
      v.func();
    v.bDone = true;
    XDBG << "[TimeWheel-执行] startday :" << m_dwStartDay << "time :" << v.dwTime << "被执行" << XEND;
  }
}

void xTimeWheel::reset()
{
  m_dwStartDay = xTime::getDayStart(now());
  for (auto &v : m_vecWheels)
  {
    v.bDone = false;
    XDBG << "[TimeWheel-重置] startday :" << m_dwStartDay << "time :" << v.dwTime << "被重置" << XEND;
  }
}

