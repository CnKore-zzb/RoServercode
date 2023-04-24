#pragma once
#include "xThread.h"
#include "xDefine.h"
#include "xQueue.h"
#include "xSingleton.h"
#include "base/UserData.h"
#include "xNoncopyable.h"
#include "xDBConnPool.h"

enum SessionThreadAction
{
  SessionThreadAction_NULL                        = 0,
  SessionThreadAction_Trade_LoadGiveToMe          = 1,
  SessionThreadAction_Trade_LoadGiveToOther       = 2,
  SessionThreadAction_Mail_Load                   = 3,
  SessionThreadAction_Mail_Update                 = 4,
  SessionThreadAction_OfflineMsgTutorReward       = 5,
  SessionThreadAction_OfflineMsg_Insert           = 6,
  SessionThreadAction_OfflineMsg_Delete           = 7,
  SessionThreadAction_OfflineMsg_Load             = 8,
};

class SessionThreadData : private xNoncopyable
{
  public:
    SessionThreadData(QWORD charid, SessionThreadAction action);
    virtual ~SessionThreadData();

    QWORD m_qwCharID = 0;
    SessionThreadAction m_oAction = SessionThreadAction_NULL;

    xRecordSet m_oRetRecordSet;

  public:
    xRecord* createRecord(xField *field);
    xRecord *m_pRecord = nullptr;
    std::string m_strWhere;
    DWORD m_dwMsgLoadType = 0;
    xField *m_pField = nullptr;
};

class SessionThread : public xThread, public xSingleton<SessionThread>
{
  friend class xSingleton<SessionThread>;
  private:
    SessionThread();
  public:
    virtual ~SessionThread();

  public:
    bool thread_init();
    void thread_proc();
    virtual void thread_stop();
    void final();

  public:
    void addDataBase(std::string dbname);

  public:
    SessionThreadData* create(QWORD charid, SessionThreadAction action);
    void add(SessionThreadData *data);
    SessionThreadData* get();
    void pop();

  private:
    xQueue<SessionThreadData> m_oPrepareQueue;
    xQueue<SessionThreadData> m_oFinishQueue;

  private:
    void check();
    void exec(SessionThreadData *data);

  private:
    DBConnPool& getDBConnPool()
    {
      return m_oDBConnPool;
    }
    DBConnPool m_oDBConnPool;
  private:
    DBConnPool& getTradeDBConnPool()
    {
      return m_oTradeDBConnPool;
    }
    DBConnPool m_oTradeDBConnPool;
};
