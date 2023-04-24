#include "TeamDataThread.h"
#include "TeamServer.h"
#include "xTime.h"
#include "xServer.h"
#include "xLog.h"
#include <unistd.h>
#include "xServer.h"

ThTeamData::ThTeamData(ThTeamDataAction act) : m_oAction(act)
{
}

ThTeamData::~ThTeamData()
{
  SAFE_DELETE(m_pRecord);
}

xRecord* ThTeamData::create(xField *field)
{
  if (!field) return nullptr;
  if (m_pRecord)
  {
    SAFE_DELETE(m_pRecord);
  }

  m_pRecord = new xRecord(field);
  return m_pRecord;
}

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

ThTeamDataThread::ThTeamDataThread()
{
}

ThTeamDataThread::~ThTeamDataThread()
{
}

void ThTeamDataThread::thread_stop()
{
  final();

  xThread::thread_stop();
}

void ThTeamDataThread::final()
{
}

bool ThTeamDataThread::thread_init()
{
  addDataBase(RO_DATABASE_NAME);
  addDataBase(thisServer->getRegionDBName());

  return true;
}

void ThTeamDataThread::addDataBase(std::string dbname)
{
  xServer::initDBConnPool("DataBase", dbname, m_oDBConnPool);
}

void ThTeamDataThread::thread_proc()
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
      XLOG_T("[ThTeamDataThread] 帧耗时 %llu 微秒", _e);
    }
  }

  check();
  XLOG_T("[ThTeamDataThread] final check");
}

void ThTeamDataThread::add(ThTeamData *pData)
{
  if (!pData) return;

  m_oPrepareQueue.put(pData);

#ifdef _LX_DEBUG
  XLOG_T("[ThTeamDataThread-Push],队列:%u", m_oPrepareQueue.size());
#endif
}

void ThTeamDataThread::check()
{
  ThTeamData *data = m_oPrepareQueue.get();
  while (data)
  {
    m_oPrepareQueue.pop();
    exec(data);

    m_oFinishQueue.put(data);
    data = m_oPrepareQueue.get();

    XLOG_T("[ThTeamDataThread] 完成队列: %u, 等待队列: %u", m_oFinishQueue.size(), m_oPrepareQueue.size());
  }
}

void ThTeamDataThread::exec(ThTeamData *pData)
{
  if (!pData) return;

  switch (pData->m_oAction)
  {
    case ThTeamDataAction_CreateTeamID:
      {
        xField *field = getDBConnPool().getField(REGION_DB, "team");
        if (field == nullptr) return;

        xRecord record(field);
        record.put("zoneid", thisServer->getZoneID());
        for (DWORD i = 0; i < pData->m_dwCreateTeamIDNum; ++i)
        {
          QWORD retcode = getDBConnPool().exeInsert(record);
          if (retcode != QWORD_MAX)
          {
            pData->m_oCreateTeamIDList.push_back(retcode);
            XLOG_T("[队伍-创建],创建id:%llu", retcode);
          }
        }
      }
      break;
    case ThTeamDataAction_Update:
      {
        char where[32] = {0};
        snprintf(where, sizeof(where), "id=%llu", pData->m_qwTeamID);
        if (getDBConnPool().exeUpdate(*(pData->m_pRecord), where) == QWORD_MAX)
        {
          XERR_T("[队伍-存储],%llu,失败", pData->m_qwTeamID);
          return;
        }
        XLOG_T("[队伍-存储],%llu,成功", pData->m_qwTeamID);
      }
      break;
    case ThTeamDataAction_Delete:
      {
        char where[64] = {0};
        snprintf(where, sizeof(where), "id=%llu", pData->m_qwTeamID);
        xField* pField = getDBConnPool().getField(REGION_DB, "team");
        if (pField)
        {
          if (getDBConnPool().exeDelete(pField, where) != QWORD_MAX)
          {
            XLOG_T("[队伍-删除],%llu,成功", pData->m_qwTeamID);
          }
          else
          {
            XERR_T("[队伍-删除],%llu,失败", pData->m_qwTeamID);
          }
        }
        else
        {
          XERR_T("[队伍-删除],%llu,失败", pData->m_qwTeamID);
        }
      }
      break;
    default:
      break;
  }

  // todo
}
