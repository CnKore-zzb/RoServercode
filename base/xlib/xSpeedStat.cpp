#include "xSpeedStat.h"
#include "xlib/xTime.h"
#include "xlib/xLog.h"

xSpeedStat::xSpeedStat(QWORD id, const char* name)
{
  set_id(id);
  set_name(name);

  m_qwLastCheckTime = xTime::getCurMSec();
}

xSpeedStat::~xSpeedStat()
{
  add(0);
}

void xSpeedStat::add(QWORD len)
{
  QWORD curMS = xTime::getCurMSec();

  if (!m_qwLastCheckTime || (curMS < m_qwLastCheckTime))
  {
    m_qwLastCheckTime = curMS;
    m_qwBytesCount = len;
    return;
  }

  m_qwBytesCount += len;

  QWORD delay = curMS - m_qwLastCheckTime;
  if (delay >= ONE_THOUSAND)
  {

    if (m_qwBytesCount >= LEN_1024)
    {
      QWORD rate = m_qwBytesCount * ONE_THOUSAND / delay / LEN_1024;
      if (rate > 10)
      {
        XLOG_T("[速率],%llu,%s, %llu kb/s, %llu, %llu", id, name, rate, m_qwBytesCount, delay);
      }
    }

    m_qwBytesCount = 0;
    m_qwLastCheckTime = curMS;
  }
}
