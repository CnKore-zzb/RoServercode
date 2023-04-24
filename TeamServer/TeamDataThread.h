#pragma once
#include <queue>
#include <list>

#include "xThread.h"
#include "xDefine.h"
#include "xSingleton.h"
#include "xNoncopyable.h"
#include "base/xlib/xDBConnPool.h"
#include "base/xlib/xDBFields.h"
#include "base/xlib/xQueue.h"

enum ThTeamDataAction
{
  ThTeamDataAction_NULL             = 0,
  ThTeamDataAction_Update           = 1,
  ThTeamDataAction_Delete           = 2,
  ThTeamDataAction_CreateTeamID       = 3,
};

class Team;

class ThTeamData : private xNoncopyable
{
  public:
    ThTeamData(ThTeamDataAction act);
    virtual ~ThTeamData();

  public:
    xRecord* create(xField *field);

  public:
    ThTeamDataAction m_oAction = ThTeamDataAction_NULL;

    xRecord *m_pRecord = nullptr;

    QWORD m_qwTeamID = 0;

    DWORD m_dwCreateTeamIDNum = 5;
    std::list<QWORD> m_oCreateTeamIDList;
};

typedef std::queue<ThTeamData *> ThTeamDataQueue;

class ThTeamDataThread : public xThread, public xSingleton<ThTeamDataThread>
{
  friend class ThTeamData;
  public:
    ThTeamDataThread();
    virtual ~ThTeamDataThread();

  public:
    bool thread_init();
    void thread_proc();
    virtual void thread_stop();
    void final();

  public:
    void addDataBase(std::string dbname);
    void add(ThTeamData *data);

    ThTeamData *get();
    void pop();

  private:
    xQueue<ThTeamData> m_oPrepareQueue;
    xQueue<ThTeamData> m_oFinishQueue;

  private:
    void check();
    void exec(ThTeamData *data);

  private:
    DBConnPool& getDBConnPool()
    {
      return m_oDBConnPool;
    }
    DBConnPool m_oDBConnPool;
};

inline ThTeamData* ThTeamDataThread::get()
{
  return m_oFinishQueue.get();
}

inline void ThTeamDataThread::pop()
{
  return m_oFinishQueue.pop();
}
