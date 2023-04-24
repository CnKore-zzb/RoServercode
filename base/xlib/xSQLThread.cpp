#include "xSQLThread.h"
#include "xTime.h"
#include "xServer.h"
#include "xLog.h"
#include <unistd.h>

xSQLThread::xSQLThread()
{
}

xSQLThread::~xSQLThread()
{
}

bool xSQLThread::init(const string& name, const string& database)
{
  return xServer::initDBConnPool(name, database, m_oDBConnPool);
}

void xSQLThread::thread_stop()
{
  final();

  xThread::thread_stop();
}

void xSQLThread::final()
{
}

bool xSQLThread::thread_init()
{

  return true;
}

void xSQLThread::thread_proc()
{
  static QWORD MIN_RECURSIVE_TIME = 30 * 1000;
  static QWORD MAX_RECURSIVE_TIME = 100 * 1000;

  thread_setState(THREAD_RUN);

  while (thread_getState()==xThread::THREAD_RUN)
  {
    xTime frameTimer;

    // todo
    check();

    QWORD _e = frameTimer.uElapse();
    if (_e < MIN_RECURSIVE_TIME)
    {
      usleep(MIN_RECURSIVE_TIME - _e);
    }
    else if (_e > MAX_RECURSIVE_TIME)
    {
      //XLOG << "[xSQLThread]" << "帧耗时" << _e << "微秒" << XEND;
      XLOG_T("[xSQLThread] 帧耗时 %llu 微秒", _e);
    }
  }

  check();
  while (!m_oQueue.final_check_queue())
  {
    check();
  }
  //XLOG << "[xSQLThread]" << "final check" << XEND;
  XLOG_T("[xSQLThread] final check");
}

xSQLAction* xSQLThread::create(xField *field)
{
  if (!field) return nullptr;

  return (new xSQLAction(field));
}

void xSQLThread::add(xSQLAction *pAct)
{
  if (!pAct) return;

  m_oQueue.put(pAct);
#ifdef _LX_DEBUG
  XLOG << "[xSQLThread-Push]" << "队列:" << m_oQueue.size() << XEND;
  XLOG << "[xSQLThread]" << pAct->m_oRecord.m_field->m_strDatabase << pAct->m_oRecord.m_field->m_strTable << pAct->m_strWhere.c_str() << XEND;
#endif
}

void xSQLThread::check()
{
  xSQLAction *data = m_oQueue.get();
  while (data)
  {
    m_oQueue.pop();
    exec(data);

    SAFE_DELETE(data);

    data = m_oQueue.get();

    XLOG_T("[xSQLThread] 队列: %u", m_oQueue.size());
  }
}

void xSQLThread::exec(xSQLAction *pAct)
{
  if (!pAct) return;
  QWORD ret = 0;
  switch (pAct->m_eType)
  {
    case xSQLType_Insert:
      {
        ret = m_oDBConnPool.exeInsert(pAct->m_oRecord, true);
        if (ret == QWORD_MAX)
        {
          if (pAct->m_oRecord.m_field)
          {
            XERR_T("[xSQLThread] Insert Error %s", pAct->m_oRecord.m_field->m_strTable.c_str());
          }
          else
          {
            XERR_T("[xSQLThread] Insert Errors");
          }
        }
      }
      break;
    case xSQLType_Update:
      {
        if (pAct->m_strWhere.size())
        {
          ret = m_oDBConnPool.exeUpdate(pAct->m_oRecord, pAct->m_strWhere.c_str());
        }
        else
        {
          ret = m_oDBConnPool.exeUpdate(pAct->m_oRecord, nullptr);
        }
        if (ret == QWORD_MAX)
        {
          if (pAct->m_oRecord.m_field)
          {
            XERR_T("[xSQLThread] Update Error %s", pAct->m_oRecord.m_field->m_strTable.c_str());
          }
          else
          {
            XERR_T("[xSQLThread] Update Error");
          }
        }
      }
      break;
    case xSQLType_Delete:
      {
        if (pAct->m_oRecord.m_field)
        {
          if (pAct->m_strWhere.size())
          {
            ret = m_oDBConnPool.exeDelete(pAct->m_oRecord.m_field, pAct->m_strWhere.c_str());
          }
          else
          {
            ret = m_oDBConnPool.exeDelete(pAct->m_oRecord.m_field, nullptr);
          }
        }
      }
      break;
    case xSQLType_Replace:
      {
        ret = m_oDBConnPool.exeReplace(pAct->m_oRecord);
        if (ret == QWORD_MAX)
        {
          XERR_T("[xSQLThread] Replace Error, %s", pAct->m_oRecord.m_field->m_strTable.c_str());
        }
      }
      break;
    case xSQLType_Max:
    default:
      break;
  }
}
