#include "SceneUserDataThread.h"
#include "xTime.h"
#include "xServer.h"
#include "xLog.h"
#include <unistd.h>

SceneUserLoadThread::SceneUserLoadThread()
{
}

SceneUserLoadThread::~SceneUserLoadThread()
{
}

void SceneUserLoadThread::thread_stop()
{
  final();

  xThread::thread_stop();
}

void SceneUserLoadThread::final()
{
}

bool SceneUserLoadThread::thread_init()
{

  return true;
}

void SceneUserLoadThread::thread_proc()
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
      XLOG_T("[SceneUserLoadThread] 帧耗时 %llu 微秒", _e);
    }
  }

  check();
  //XLOG << "[xSQLThread]" << "final check" << XEND;
  XLOG_T("[SceneUserLoadThread] final check");
}

void SceneUserLoadThread::add(Cmd::RecordUserData &data)
{
  SceneUserDataLoad *pData = NEW SceneUserDataLoad();
  pData->setRecordUserData(data);
  m_oPrepareQueue.put(pData);
#ifdef _LX_DEBUG
  XLOG_T("[SceneUserLoadThread-Push],队列:%u", m_oPrepareQueue.size());
#endif
}

void SceneUserLoadThread::check()
{
  SceneUserDataLoad *pData = m_oPrepareQueue.get();
  while (pData)
  {
    m_oPrepareQueue.pop();
#ifdef _LX_DEBUG
    XLOG_T("[SceneUserLoadThread-Pop],队列:%u", m_oPrepareQueue.size());
#endif
    exec(pData);
    m_oFinishQueue.put(pData);
#ifdef _LX_DEBUG
    XLOG_T("[SceneUserLoadThread-Finish-Push],队列:%u", m_oFinishQueue.size());
#endif

    pData = m_oPrepareQueue.get();
  }
}

void SceneUserLoadThread::exec(SceneUserDataLoad *pData)
{
  if (!pData) return;

  pData->fromCmdToData();
}

SceneUserDataLoad* SceneUserLoadThread::get()
{
  return m_oFinishQueue.get();
}

void SceneUserLoadThread::pop()
{
  m_oFinishQueue.pop();
#ifdef _LX_DEBUG
  XLOG_T("[SceneUserLoadThread-Finish-Pop],队列:%u", m_oFinishQueue.size());
#endif
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

SceneUserSaveThread::SceneUserSaveThread()
{
}

SceneUserSaveThread::~SceneUserSaveThread()
{
}

void SceneUserSaveThread::thread_stop()
{
  final();

  xThread::thread_stop();
}

void SceneUserSaveThread::final()
{
}

bool SceneUserSaveThread::thread_init()
{

  return true;
}

void SceneUserSaveThread::thread_proc()
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
      XLOG_T("[SceneUserSaveThread] 帧耗时 %llu 微秒", _e);
    }
  }

  check();
  //XLOG << "[xSQLThread]" << "final check" << XEND;
  XLOG_T("[SceneUserSaveThread] final check");
}

void SceneUserSaveThread::add(SceneUserDataSave *pData)
{
  if (!pData) return;
  m_oPrepareQueue.put(pData);
#ifdef _LX_DEBUG
  XLOG_T("[SceneUserSaveThread-Push], 队列:%u", m_oPrepareQueue.size());
#endif
}

void SceneUserSaveThread::check()
{
  SceneUserDataSave *pData = m_oPrepareQueue.get();
  while (pData)
  {
    m_oPrepareQueue.pop();
#ifdef _LX_DEBUG
    XLOG_T("[SceneUserSaveThread-Pop],队列:%u", m_oPrepareQueue.size());
#endif
    exec(pData);
    SAFE_DELETE(pData);

    pData = m_oPrepareQueue.get();
  }
}

void SceneUserSaveThread::exec(SceneUserDataSave *pData)
{
  if (!pData) return;

  pData->fromDataToCmd();
}
