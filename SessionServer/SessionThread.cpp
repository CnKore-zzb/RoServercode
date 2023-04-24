#include "SessionThread.h"
#include "xTime.h"
#include "xServer.h"
#include "xLog.h"
#include <unistd.h>
#include "xServer.h"
#include "SessionServer.h"

SessionThreadData::SessionThreadData(QWORD charid, SessionThreadAction action) : m_qwCharID(charid), m_oAction(action)
{
}

SessionThreadData::~SessionThreadData()
{
  if (m_pRecord)
  {
    SAFE_DELETE(m_pRecord);
  }
  if (m_pField)
  {
    SAFE_DELETE(m_pField);
  }
}

xRecord* SessionThreadData::createRecord(xField *field)
{
  if (!field) return nullptr;

  m_pRecord = new xRecord(field);

  return m_pRecord;
}

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

SessionThread::SessionThread()
{
}

SessionThread::~SessionThread()
{
}

void SessionThread::thread_stop()
{
  final();

  xThread::thread_stop();
}

void SessionThread::final()
{
}

bool SessionThread::thread_init()
{
  addDataBase(REGION_DB);

  return true;
}

void SessionThread::addDataBase(std::string dbname)
{
  xServer::initDBConnPool("DataBase", dbname, m_oDBConnPool);
  xServer::initDBConnPool("TradeDataBase", dbname, m_oTradeDBConnPool);
  XLOG_T("[SessionThread] 初始化数据库 %s", dbname.c_str());
}

void SessionThread::thread_proc()
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
      XLOG_T("[SessionThread] 帧耗时 %llu 微秒", _e);
    }
  }

  check();
  XLOG_T("[SessionThread] final check");
}

SessionThreadData* SessionThread::create(QWORD charid, SessionThreadAction action)
{
  return (new SessionThreadData(charid, action));
}

void SessionThread::add(SessionThreadData *pData)
{
  if (!pData) return;

  m_oPrepareQueue.put(pData);

#ifdef _LX_DEBUG
  XLOG_T("[SessionThread-Push],队列:%u", m_oPrepareQueue.size());
#endif
}

void SessionThread::check()
{
  SessionThreadData *data = m_oPrepareQueue.get();
  while (data)
  {
    m_oPrepareQueue.pop();
    exec(data);

    m_oFinishQueue.put(data);
    data = m_oPrepareQueue.get();

    XLOG_T("[SessionThread] 完成队列: %u, 等待队列: %u", m_oFinishQueue.size(), m_oPrepareQueue.size());
  }
}

void SessionThread::exec(SessionThreadData *pData)
{
  if (!pData) return;

  switch (pData->m_oAction)
  {
    case SessionThreadAction_OfflineMsg_Load:
      {
        xField* pField = getDBConnPool().getField(REGION_DB, "offmsg");
        if (pField == nullptr)
        {
          XERR_T("[邮件-加载],%llu,加载失败,未找到mail数据库表", pData->m_qwCharID);
          return;
        }

        QWORD retNum = getDBConnPool().exeSelect(pField, pData->m_oRetRecordSet, pData->m_strWhere.c_str());
        if (QWORD_MAX == retNum)
        {
          XERR_T("[离线消息-加载],%llu,加载失败,查询失败 ret:%u", pData->m_qwCharID, retNum);
          return;
        }
        XLOG << "[离线消息-加载] 结束加载数据库 database:" << pField->m_strDatabase << "table:" << pField->m_strTable << "成功加载：" << retNum << "条" << XEND;
      }
      break;
    case SessionThreadAction_OfflineMsg_Delete:
      {
        if (!pData->m_pRecord) return;
        if (!pData->m_pRecord->m_field) return;
        if (pData->m_strWhere.empty()) return;

        QWORD retcode = getDBConnPool().exeDelete(pData->m_pRecord->m_field, pData->m_strWhere.c_str());
        if (retcode == QWORD_MAX)
        {
          XERR_T("[离线消息-删除] offmsg失败 %s", pData->m_strWhere.c_str());
          return;
        }
      }
      break;
    case SessionThreadAction_OfflineMsg_Insert:
      {
        if (!pData->m_pRecord) return;

        QWORD retcode = getDBConnPool().exeInsert(*(pData->m_pRecord));
        if (retcode == QWORD_MAX)
        {
          XERR_T("[离线消息-保存] %llu 失败", pData->m_qwCharID);
          return;
        }
      }
      break;
    case SessionThreadAction_OfflineMsgTutorReward:
      {
        if (!pData->m_pRecord) return;

        QWORD ret = getDBConnPool().exeReplace(*(pData->m_pRecord));
        if (ret == QWORD_MAX)
        {
          XERR_T("[导师奖励-离线数据保存] %llu 失败", pData->m_qwCharID);
          return;
        }

        XLOG_T("[导师奖励-离线数据保存] %llu 成功", pData->m_qwCharID);
      }
      break;
    case SessionThreadAction_Mail_Load:
      {
        xField* pField = getDBConnPool().getField(REGION_DB, "mail");
        if (pField == nullptr)
        {
          XERR_T("[邮件-加载],%llu,加载失败,未找到mail数据库表", pData->m_qwCharID);
          return;
        }

        stringstream sstr;
        char szWhere[LEN_128] = {0};
        //snprintf(szWhere, sizeof(szWhere), "receiveid = %llu and id > %llu", m_pUser->id, s);
        snprintf(szWhere, sizeof(szWhere), "receiveid = %llu", pData->m_qwCharID);

        QWORD retNum = getDBConnPool().exeSelect(pField, pData->m_oRetRecordSet, szWhere);
        if (QWORD_MAX == retNum)
        {
          XERR_T("[邮件-加载],%llu,加载失败,查询失败 ret:%u", pData->m_qwCharID, retNum);
          return;
        }
      }
      break;
    case SessionThreadAction_Trade_LoadGiveToMe:
      {
        pData->m_pField = new xField(REGION_DB, DB_TABLE_BUYED_LIST);
        if (!pData->m_pField) return;
        pData->m_pField->m_list["id"] = MYSQL_TYPE_LONG;
        pData->m_pField->m_list["char_id"] = MYSQL_TYPE_LONG;
        pData->m_pField->m_list["take_status"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["player_name"] = MYSQL_TYPE_STRING;
        pData->m_pField->m_list["item_id"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["count"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["price"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["item_data"] = MYSQL_TYPE_BLOB;
        pData->m_pField->m_list["expire_time"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["anonymous"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["background"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["content"] = MYSQL_TYPE_STRING;

        char sql[LEN_512];
        bzero(sql, sizeof(sql));
        snprintf(sql, sizeof(sql), "select t1.id as id, t1.char_id as char_id, t1.take_status as take_status,t1.player_name as player_name, t1.item_id as item_id, t1.count as count,t1.price as price, t1.item_data as item_data, COALESCE(t2.expire_time, 0) as expire_time,COALESCE(t2.anonymous, 0) as anonymous, COALESCE(t2.background, 0) as background,COALESCE(t2.content, \"\") as content from %s as t1 left join %s as t2 on t1.id=t2.record_id where t2.receiver_id=%llu", DB_TABLE_BUYED_LIST, DB_TABLE_GIFT, pData->m_qwCharID);
        QWORD ret = getTradeDBConnPool().exeRawSelect(pData->m_pField, pData->m_oRetRecordSet, sql);
        if (ret == QWORD_MAX)
        {
          XERR << "[交易-数据库-赠送] DB_TABLE_BUYED_LIST数据库错误 charid：" << pData->m_qwCharID << "table: " << DB_TABLE_BUYED_LIST << XEND;
          return;
        }
      }
      break;
    case SessionThreadAction_Trade_LoadGiveToOther:
      {
        pData->m_pField = new xField(REGION_DB, DB_TABLE_BUYED_LIST);
        if (!pData->m_pField) return;
        pData->m_pField->m_list["id"] = MYSQL_TYPE_LONG;
        pData->m_pField->m_list["take_status"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["item_id"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["count"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["price"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["expire_time"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["background"] = MYSQL_TYPE_NEWDECIMAL;
        pData->m_pField->m_list["receiver_name"] = MYSQL_TYPE_STRING;

        char sql[LEN_512];
        bzero(sql, sizeof(sql));
        snprintf(sql, sizeof(sql), "select t1.id as id, t1.take_status as take_status,t1.item_id as item_id, t1.count as count, t1.price as price, COALESCE(t2.expire_time,0) as expire_time,COALESCE(t2.background, 0) as background,COALESCE(t2.receiver_name,\"\") as receiver_name from %s as t1 left join %s as t2 on  t1.id=t2.record_id where t1.char_id=%llu", DB_TABLE_BUYED_LIST, DB_TABLE_GIFT, pData->m_qwCharID);
        QWORD ret = getTradeDBConnPool().exeRawSelect(pData->m_pField, pData->m_oRetRecordSet, sql);

        if (ret == QWORD_MAX)
        {
          XERR << "[交易-数据库-赠送] DB_TABLE_BUYED_LIST数据库错误 charid：" << pData->m_qwCharID << "table: " << DB_TABLE_BUYED_LIST << XEND;
          return;
        }
      }
      break;
    case SessionThreadAction_NULL:
    default:
      break;
  }
}

SessionThreadData* SessionThread::get()
{
  return m_oFinishQueue.get();
}

void SessionThread::pop()
{
  return m_oFinishQueue.pop();
}
