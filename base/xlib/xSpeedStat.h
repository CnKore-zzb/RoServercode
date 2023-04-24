#pragma once
#include "xlib/xDefine.h"
#include "xlib/xEntry.h"

class xSpeedStat : public xEntry
{
  public:
    xSpeedStat(QWORD id, const char* name);
    virtual ~xSpeedStat();

  public:
    void add(QWORD len);

  private:
    QWORD m_qwBytesCount = 0;
    QWORD m_qwLastCheckTime = 0;
};
