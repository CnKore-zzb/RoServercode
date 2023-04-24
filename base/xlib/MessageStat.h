#pragma once
#include "xDefine.h"
#include "xTime.h"

struct stMessageStat
{
  QWORD m_qwDuration = 0;
  DWORD m_dwCount = 0;
  QWORD m_qwMaxTime = 0;
};

class MessageStat
{
  public:
    MessageStat();
    ~MessageStat();

  public:
    void setFlag(const std::string& flag) { m_strFlag = flag; }
    void start(DWORD cmd, DWORD param);
    void end();

  public:
    void print();

  private:
    std::map<DWORD, stMessageStat> m_oStatList;

    xTime m_oFrameTimer;
    DWORD m_dwMessageKey = 0;
    std::string m_strFlag;
};
