#pragma once
#include <queue>
#include "xThread.h"
#include "xDefine.h"
#include "xDBConnPool.h"
#include "xQueue.h"

enum xSQLType
{
  xSQLType_Insert   = 0,
  xSQLType_Update   = 1,
  xSQLType_Delete   = 2,
  xSQLType_Replace  = 3,
  xSQLType_Max      = 4,
};

class xSQLAction
{
  friend class xSQLThread;
  public:
    xSQLAction(xField *field) : m_oRecord(field)
    {
    }
    ~xSQLAction()
    {
    }

  public:
    xSQLType m_eType = xSQLType_Max;
    std::string m_strWhere;
    xRecord m_oRecord;
};

class xSQLThread : public xThread
{
  public:
    xSQLThread();
    virtual ~xSQLThread();

  public:
    bool init(const string& name, const string& database);
    bool thread_init();
    void thread_proc();
    virtual void thread_stop();
    void final();

  public:
    DBConnPool& getDBConnPool()
    {
      return m_oDBConnPool;
    }
  private:
    DBConnPool m_oDBConnPool;

  public:
    static xSQLAction* create(xField *field);
    void add(xSQLAction *act);
  private:
    xQueue<xSQLAction> m_oQueue;

  private:
    void check();
    void exec(xSQLAction *act);
};
