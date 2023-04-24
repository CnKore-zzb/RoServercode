#pragma once

#include "xSingleton.h"
#include "xNoncopyable.h"
#include "TableStruct.h"
#include "ServerTask.h"

enum class TimerType
{
  null = 0,
  start,
  restart,
  reapeat,
  end,
};

enum class TimerState
{
  start,
  end,
  wait,
};

class Timer : private xNoncopyable
{
  public:
    Timer();
    ~Timer();

  public:
    void init(const TimerBase *base);
    void check(DWORD cur);
    bool inTime(DWORD cur);
    void exec(TimerType type, ServerTask *task = nullptr);

    void setState(TimerState state);
    TimerState getState() { return m_oState; }

    DWORD getCmdKeyID();
    DWORD getStartTime() { return m_dwStartTime; }
    DWORD getEndTime() { return m_dwEndTime; }
  private:
    const TimerBase *m_pBase = nullptr;
    DWORD m_dwStartTime = 0;
    DWORD m_dwEndTime = 0;
    TimerState m_oState = TimerState::wait;
    DWORD m_dwRepeatTime = 0;
};

class TimerM : public xSingleton<TimerM>
{
  friend class xSingleton<TimerM>;
  public:
    virtual ~TimerM();
  private:
    TimerM();

  public:
    void sceneStart(ServerTask *task);
    bool load();
    void timer(DWORD curTime);
    bool getStartEndTime(DWORD dwId, DWORD& dwStartTime, DWORD& dwEndTime);
  private:
    void add(const TimerBase *base);

  private:
    std::map<DWORD, Timer> m_list;
    std::set<std::string> m_setOkScene;     //已经连上的scene
};
