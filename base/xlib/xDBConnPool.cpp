#include <sstream>
#include "xlib/xDBConnPool.h"
#include "xlib/xTools.h"
#include "xlib/xXMLParser.h"
#include "BaseConfig.h"
#include "xlib/xTime.h"

FunctionTime_setInterval(300);

void DBSqlCache::init(int maxCount)
{
  m_maxCount = maxCount;
}

//static 获取key
QWORD DBSqlCache::generateKey(const xRecord &rec, bool updateOnDup)
{
  std::stringstream sql_string;
  sql_string.str("");
  sql_string << "INSERT INTO " << rec.m_field->m_strDatabase << "." << rec.m_field->m_strTable << " (";
  DWORD k = 0;
  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    sql_string << it.first.c_str();
    k++;
  }
  sql_string << ")";
  if (updateOnDup)
    sql_string << "duplicate";

  std::hash<std::string> h;
  size_t key = h(sql_string.str());
  XDBG << "[mysql] str:" << sql_string.str().c_str() << " key:" << (DWORD)key << XEND;;
  return key;
}

bool DBSqlCache::insert(DBConn* pDbConn, const std::string& db, const xRecord &rec, bool updateOnDup)
{
  if (!pDbConn)
    return false;

  if (m_curCount == 0)
  {
    m_strSql.clear();

    std::stringstream sql_string;
    sql_string.str("");
    sql_string << "INSERT INTO " << rec.m_field->m_strDatabase << "." << rec.m_field->m_strTable << " (";
    DWORD k = 0;

    for (auto &it : rec.m_rs)
    {
      if (k)
        sql_string << ",";
      sql_string << it.first.c_str();
      k++;
    }
    sql_string << ") VALUES(";
    k = 0;
    for (auto &it : rec.m_rs)
    {
      if (k)
        sql_string << ",";
      pDbConn->getDataStringByType(rec.getType(it.first), it.second, sql_string);
      k++;
    }
    sql_string << ")";
    m_strSql = sql_string.str();
  }
  else
  {
    std::stringstream sql_string;
    sql_string.str("");
    sql_string << ",(";
    DWORD k = 0;
    for (auto &it : rec.m_rs)
    {
      if (k)
        sql_string << ",";
      pDbConn->getDataStringByType(rec.getType(it.first), it.second, sql_string);
      k++;
    }
    sql_string << ")";
    m_strSql += sql_string.str();
  }
  m_curCount++;

  if (updateOnDup && m_updateSql.empty())
  {
    std::stringstream sql_string;
    sql_string.str("");
    std::string optionstring = " ON DUPLICATE KEY UPDATE ";
    DWORD k = 0;
    for (auto &it : rec.m_rs)
    {
      if (k)
        optionstring += ",";
      optionstring = optionstring + it.first + "=values(" + it.first + ")";
      k++;
    }
    sql_string << optionstring;
    m_updateSql = sql_string.str();
  }

  if (m_curCount >= m_maxCount)
  {
    m_needFlush = true;
    exeSql(pDbConn, db);
  }
  return true;
}

QWORD DBSqlCache::exeSql(DBConn* pDbConn, const std::string& db, bool bForce/*=false*/)
{
  if (!bForce && !m_needFlush)
  {
    return 0;
  }
  
  if (m_strSql.empty())
    return false;
  if (!m_updateSql.empty())
  {
    m_strSql = m_strSql + m_updateSql;
  }
  
  if (!pDbConn) return DBErrReturn;
  if (!pDbConn->reconnect()) return DBErrReturn;
  QWORD ret = pDbConn->exeSql(db, m_strSql.c_str());
  XINF << "[mysql] DBSqlCache exesql count:" <<m_curCount <<"ret:"<<ret << XEND;
  clear();
  return ret;
}

const std::string& DBSqlCache::getSql() { return m_strSql; }

bool DBSqlCache::needFlush() { return m_needFlush; }

void DBSqlCache::clear()
{
  m_strSql.clear();
  m_updateSql.clear();
  m_curCount = 0;
  m_needFlush = false;
}

//--------------------------
//DBSqlCacheMgr

void DBSqlCacheMgr::init(DBConn* pDbConn, int maxCount, const std::string& db)
{
  m_pDbConn = pDbConn;
  m_maxCount = maxCount;
  m_database = db;
}

DBSqlCacheMgr::~DBSqlCacheMgr()
{//析构强制执行
  flush();
}

bool DBSqlCacheMgr::insert(const xRecord &rec, bool updateOnDup /*=false*/)
{
  std::size_t key = DBSqlCache::generateKey(rec, updateOnDup);

  auto it = m_mapCache.find(key);
  if (it == m_mapCache.end())
  {
    DBSqlCache dbCache;
    dbCache.init(m_maxCount);
    it = m_mapCache.insert(std::make_pair(key, dbCache)).first;
  }
  it->second.insert(m_pDbConn, m_database, rec, updateOnDup);
  return true;
}

void DBSqlCacheMgr::flush()
{
  if (!m_pDbConn)
    return;
  if (m_database.empty())
    return;

  for (auto it = m_mapCache.begin(); it != m_mapCache.end(); ++it)
  {
    it->second.exeSql(m_pDbConn, m_database, true);
  }
}

//------------------
DBConn::DBConn()
{
}

bool DBConn::init(const char *server, const char *user, const char *password, DWORD port, const std::string &database)
{
  if (!server || !user || !password) return false;

  m_oAccount.m_strServerIP = server;
  m_oAccount.m_strUser = user;
  m_oAccount.m_strPassword = password;
  m_oAccount.m_dwPort = port;

  close();
  mysql = mysql_init(NULL);

  bool ret = false;

  if (m_oAccount.m_strPassword.empty())
  {
    ret = mysql_real_connect(mysql, server, user, NULL, NULL, port, NULL, CLIENT_MULTI_STATEMENTS);
  }
  else
  {
    ret = mysql_real_connect(mysql, server, user, password, NULL, port, NULL, CLIENT_MULTI_STATEMENTS);
  }


  if (ret==false)
  {
    XERR_T("[MYSQL],不能连接mysql server:%s,%s,%s,%s", server, user, password, mysql_error(mysql));
    return false;
  }
  mysql_query(mysql, "SET NAMES utf8mb4");

  loadFields(database);

  return true;
}

bool DBConn::reconnect()
{
  if (mysql_ping(mysql))
  {
    close();
    mysql = mysql_init(NULL);
    bool ret = false;
    if (m_oAccount.m_strPassword.empty())
    {
      ret = mysql_real_connect(mysql, m_oAccount.m_strServerIP.c_str(), m_oAccount.m_strUser.c_str(), NULL, NULL, m_oAccount.m_dwPort, NULL, CLIENT_MULTI_STATEMENTS);
    }
    else
    {
      ret = mysql_real_connect(mysql, m_oAccount.m_strServerIP.c_str(), m_oAccount.m_strUser.c_str(), m_oAccount.m_strPassword.c_str(), NULL, m_oAccount.m_dwPort, NULL, CLIENT_MULTI_STATEMENTS);
    }

    if (ret == false)
    {
      XERR_T("[MYSQL],不能连接mysql server:%s,%s,%s", m_oAccount.m_strServerIP.c_str(), m_oAccount.m_strUser.c_str(), m_oAccount.m_strPassword.c_str());
      return false;
    }
    mysql_query(mysql, "SET NAMES utf8mb4");

    XDBG << "[MYSQL] reconnect success" << XEND;
  }

  return true;
}

void DBConn::close()
{
  if (mysql)
  {
    mysql_close(mysql);
    mysql = nullptr;
  }
}

QWORD DBConn::exeSql(const std::string &database, const char *sql)
{
  if (sql == NULL) return DBErrReturn;

  std::stringstream query_string;
  query_string.str("");

  query_string << "use " << database << ";" << sql;

  DBTimeCheck db_log(query_string.str());
  if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
  {
    XERR << "[MYSQL]," << query_string.str().c_str() << XEND;
    return DBErrReturn;
  }
  return mysql_affected_rows(mysql);
}

QWORD DBConn::exeDelete(xField *field, const char *where)
{
  if (nullptr == field) return DBErrReturn;

  std::stringstream query_string;
  query_string.str("");
  query_string << "DELETE FROM " << field->m_strDatabase << "." << field->m_strTable;
  DBTimeCheck db_log(query_string.str());
  if (NULL != where)
    query_string << " WHERE " << where;
#ifdef _SQL_DEBUG
  XDBG << "[MYSQL DELETE]:" << query_string.str().c_str() << XEND;
#endif
  if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
  {
    XERR << "[MYSQL]," << query_string.str().c_str() << XEND;
    return DBErrReturn;
  }
  return mysql_affected_rows(mysql);
}

QWORD DBConn::exeUpdate(const xRecord &rec, const char *where)
{
  std::stringstream sql_string;
  sql_string.str("");
  sql_string << "UPDATE " << rec.m_field->m_strDatabase << "." << rec.m_field->m_strTable << " SET ";
  std::stringstream dblog_string;
  dblog_string.str("");
  dblog_string << sql_string.str();
  DWORD k = 0;

  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    sql_string << it.first.c_str() << "=";

    switch (rec.getType(it.first))
    {
      case MYSQL_TYPE_VAR_STRING:  // VARCHAR字段
      case MYSQL_TYPE_STRING:  // CHAR字段
        {
          std::string str = it.second.getString();

          if (str.empty() == true)
          {
            sql_string << "'" << "" <<"'";
          }
          else
          {
            char temp_string[str.size() * 2 + 1];
            bzero(temp_string, sizeof(temp_string));

            mysql_real_escape_string(mysql, (char *)temp_string, (const char *)str.c_str(), str.size());

            sql_string << "'" << temp_string << "'";
          }
        }
        break;
      case MYSQL_TYPE_TINY:  // TINYINT字段
      case MYSQL_TYPE_SHORT:  // SMALLINT字段
      case MYSQL_TYPE_INT24:  // MEDIUMINT字段
      case MYSQL_TYPE_DECIMAL:  // DECIMAL或NUMERIC字段
      case MYSQL_TYPE_NEWDECIMAL:  // 精度数学DECIMAL或NUMERIC
        sql_string << it.second.getInt();
        break;
      case MYSQL_TYPE_LONG:  // INTEGER字段
        sql_string << it.second.getLong();
        break;
      case MYSQL_TYPE_LONGLONG:  // BIGINT字段
        sql_string << it.second.getLongLong();
        break;
      case MYSQL_TYPE_FLOAT:  // FLOAT字段
      case MYSQL_TYPE_DOUBLE:  // DOUBLE或REAL字段
        sql_string << it.second.getFloat();
        break;
      case MYSQL_TYPE_BLOB:  // BLOB或TEXT字段（使用max_length来确定最大长度）
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
        {
          if (it.second.size() == 0)
          {
            sql_string << "\'\'";
          }
          else
          {
            std::vector<unsigned char> vec;
            vec.resize(it.second.size() * 2 + 1);
            mysql_real_escape_string(mysql, (char *)&vec[0], (const char *)it.second.getBin(), it.second.size());

            sql_string << "\'" << &vec[0] << "\'";
          }
        }
        break;
      case MYSQL_TYPE_BIT:  // BIT字段
      case MYSQL_TYPE_TIMESTAMP:  // TIMESTAMP字段
      case MYSQL_TYPE_DATE:  // DATE字段
      case MYSQL_TYPE_TIME:  // TIME字段
      case MYSQL_TYPE_DATETIME:  // DATETIME字段
      case MYSQL_TYPE_YEAR:  // YEAR字段
      case MYSQL_TYPE_SET:  // SET字段
      case MYSQL_TYPE_ENUM:  // ENUM字段
      case MYSQL_TYPE_GEOMETRY:  // Spatial字段
      case MYSQL_TYPE_NULL:  // NULL-type字段
      default:
        sql_string << "'" << it.second.getString() << "'";
        break;
    }
    k++;
  }

  DBTimeCheck db_log(dblog_string.str());
  if (NULL != where)
    sql_string << " WHERE " << where;
#ifdef _SQL_DEBUG
  XDBG << "[MYSQL UPDATE],size:" << static_cast<QWORD>(sql_string.str().size()) << ",sql:" << sql_string.str().c_str() << XEND;
#endif
  /* send SQL query */
  int result = mysql_real_query(mysql, sql_string.str().c_str(), sql_string.str().size());
  if (result)
  {
    XERR << "[MYSQL]," << sql_string.str().c_str() << XEND;
    return DBErrReturn;
  }
  return mysql_affected_rows(mysql);
}

bool DBConn::getUpdateString(const xRecord &rec, const char *where, std::string &str)
{
  std::stringstream sql_string;
  sql_string.str("");
  sql_string << "UPDATE " << rec.m_field->m_strDatabase << "." << rec.m_field->m_strTable << " SET ";
  DWORD k = 0;

  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    sql_string << it.first.c_str() << "=";

    switch (rec.getType(it.first))
    {
      case MYSQL_TYPE_VAR_STRING:  // VARCHAR字段
      case MYSQL_TYPE_STRING:  // CHAR字段
        {
          std::string str = it.second.getString();

          if (str.empty() == true)
          {
            sql_string << "'" << "" <<"'";
          }
          else
          {
            char temp_string[str.size() * 2 + 1];
            bzero(temp_string, sizeof(temp_string));

            mysql_real_escape_string(mysql, (char *)temp_string, (const char *)str.c_str(), str.size());

            sql_string << "'" << temp_string << "'";
          }
        }
        break;
      case MYSQL_TYPE_TINY:  // TINYINT字段
      case MYSQL_TYPE_SHORT:  // SMALLINT字段
      case MYSQL_TYPE_INT24:  // MEDIUMINT字段
      case MYSQL_TYPE_DECIMAL:  // DECIMAL或NUMERIC字段
      case MYSQL_TYPE_NEWDECIMAL:  // 精度数学DECIMAL或NUMERIC
        sql_string << it.second.getInt();
        break;
      case MYSQL_TYPE_LONG:  // INTEGER字段
        sql_string << it.second.getLong();
        break;
      case MYSQL_TYPE_LONGLONG:  // BIGINT字段
        sql_string << it.second.getLongLong();
        break;
      case MYSQL_TYPE_FLOAT:  // FLOAT字段
      case MYSQL_TYPE_DOUBLE:  // DOUBLE或REAL字段
        sql_string << it.second.getFloat();
        break;
      case MYSQL_TYPE_BLOB:  // BLOB或TEXT字段（使用max_length来确定最大长度）
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
        {
          if (it.second.size() == 0)
          {
            sql_string << "\'\'";
          }
          else
          {
            std::vector<unsigned char> vec;
            vec.resize(it.second.size() * 2 + 1);
            mysql_real_escape_string(mysql, (char *)&vec[0], (const char *)it.second.getBin(), it.second.size());

            sql_string << "\'" << &vec[0] << "\'";
          }
        }
        break;
      case MYSQL_TYPE_BIT:  // BIT字段
      case MYSQL_TYPE_TIMESTAMP:  // TIMESTAMP字段
      case MYSQL_TYPE_DATE:  // DATE字段
      case MYSQL_TYPE_TIME:  // TIME字段
      case MYSQL_TYPE_DATETIME:  // DATETIME字段
      case MYSQL_TYPE_YEAR:  // YEAR字段
      case MYSQL_TYPE_SET:  // SET字段
      case MYSQL_TYPE_ENUM:  // ENUM字段
      case MYSQL_TYPE_GEOMETRY:  // Spatial字段
      case MYSQL_TYPE_NULL:  // NULL-type字段
      default:
        sql_string << "'" << it.second.getString() << "'";
        break;
    }
    k++;
  }

  if (NULL != where)
    sql_string << " WHERE " << where;
#ifdef _SQL_DEBUG
  XDBG << "[MYSQL UPDATE],size:" << static_cast<QWORD>(sql_string.str().size()) << ",sql:" << sql_string.str().c_str() << XEND;
#endif
  str = sql_string.str();
  return true;
}

void DBConn::getDataStringByType(enum_field_types type, const xData &data, std::stringstream &out)
{
  switch (type)
  {
    case MYSQL_TYPE_VAR_STRING:  // VARCHAR字段
    case MYSQL_TYPE_STRING:  // CHAR字段
      {
        std::string str = data.getString();
        if (str.empty() == true)
        {
          out << "'" << "" <<"'";
        }
        else
        {
          char temp_string[str.size() * 2 + 1];
          bzero(temp_string, sizeof(temp_string));

          mysql_real_escape_string(mysql, (char *)temp_string, (const char *)str.c_str(), str.size());

          out << "'" << temp_string << "'";
        }
      }
      break;
    case MYSQL_TYPE_TINY:  // TINYINT字段
    case MYSQL_TYPE_SHORT:  // SMALLINT字段
    case MYSQL_TYPE_INT24:  // MEDIUMINT字段
    case MYSQL_TYPE_DECIMAL:  // DECIMAL或NUMERIC字段
    case MYSQL_TYPE_NEWDECIMAL:  // 精度数学DECIMAL或NUMERIC
      out << data.getInt();
      break;
    case MYSQL_TYPE_LONG:  // INTEGER字段
      out << data.getLong();
      break;
    case MYSQL_TYPE_LONGLONG:  // BIGINT字段
      out << data.getQWORD();
      break;
    case MYSQL_TYPE_FLOAT:  // FLOAT字段
    case MYSQL_TYPE_DOUBLE:  // DOUBLE或REAL字段
      out << data.getFloat();
      break;
    case MYSQL_TYPE_BLOB:  // BLOB或TEXT字段（使用max_length来确定最大长度）
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
      {
        if (data.size() == 0)
        {
          out << "\'\'";
        }
        else
        {
          std::vector<unsigned char> vec;
          vec.resize(data.size() * 2 + 1);
          mysql_real_escape_string(mysql, (char *)&vec[0], (const char *)data.getBin(), data.size());

          out << "\'" << &vec[0] << "\'";
        }
      }
      break;
    case MYSQL_TYPE_BIT:  // BIT字段
    case MYSQL_TYPE_TIMESTAMP:  // TIMESTAMP字段
    case MYSQL_TYPE_DATE:  // DATE字段
    case MYSQL_TYPE_TIME:  // TIME字段
    case MYSQL_TYPE_DATETIME:  // DATETIME字段
    case MYSQL_TYPE_YEAR:  // YEAR字段
    case MYSQL_TYPE_SET:  // SET字段
    case MYSQL_TYPE_ENUM:  // ENUM字段
    case MYSQL_TYPE_GEOMETRY:  // Spatial字段
    case MYSQL_TYPE_NULL:  // NULL-type字段
    default:
      out << "'" << data.getString() << "'";
      break;
  }
}

QWORD DBConn::exeInsert(const xRecord &rec, bool isRet, bool duplicate)
{
  std::stringstream sql_string;
  sql_string.str("");
  sql_string << "INSERT INTO " << rec.m_field->m_strDatabase << "." << rec.m_field->m_strTable << " (";
  std::stringstream dblog_string;
  dblog_string.str("");
  dblog_string << sql_string.str();
  DWORD k = 0;

  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    sql_string << it.first.c_str();
    k++;
  }

  sql_string << ") VALUES(";
  k = 0;

  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    getDataStringByType(rec.getType(it.first), it.second, sql_string);
    k++;
  }
  sql_string << ")" ;
#ifdef _SQL_DEBUG
  XDBG << "[MYSQL INSERT]," << sql_string.str().c_str() << XEND;
#endif
  if (duplicate)
  {
    std::string optionstring = "ON DUPLICATE KEY UPDATE ";
    k = 0;
    for (auto &it : rec.m_rs)
    {
      if (k)
        optionstring += ",";
      optionstring = optionstring + it.first + "=values(" + it.first + ")";
      k++;
    }
    sql_string << optionstring;
  }
  /* send SQL query */
  DBTimeCheck db_log(dblog_string.str());
  if (mysql_real_query(mysql, sql_string.str().c_str(), sql_string.str().size()))
  {
    XERR << "[MYSQL]query error! " << sql_string.str().c_str() << XEND;
    return DBErrReturn;
  }
  if (isRet)
    return mysql_insert_id(mysql);
  else
    return mysql_affected_rows(mysql);
}

QWORD DBConn::exeInsertSet(xRecordSet &recset, bool duplicate)
{
  if (recset.empty()) return DBErrReturn;

  const xRecord &rec = recset[0];

  std::stringstream sql_string;
  sql_string.str("");
  sql_string << "INSERT INTO " << rec.m_field->m_strDatabase << "." << rec.m_field->m_strTable << " (";
  std::stringstream dblog_string;
  dblog_string.str("");
  dblog_string << sql_string.str();

  DWORD k = 0;
  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    sql_string << it.first.c_str();
    k++;
  }
  sql_string << ") VALUES";

  DWORD num = recset.size();
  for (DWORD recordnum = 0; recordnum < num; ++recordnum)
  {
    sql_string << "(";
    k = 0;

    for (auto &it : recset[recordnum].m_rs)
    {
      if (k)
        sql_string << ",";
      getDataStringByType(rec.getType(it.first), it.second, sql_string);
      k++;
    }

    if ((recordnum+1) == num)
      sql_string << ")";
    else
      sql_string << "),";
  }

  if (duplicate)
  {
    std::string optionstring = "ON DUPLICATE KEY UPDATE ";
    k = 0;
    for (auto &it : rec.m_rs)
    {
      if (k)
        optionstring += ",";
      optionstring = optionstring + it.first + "=values(" + it.first + ")";
      k++;
    }
    sql_string << optionstring;
  }

#if defined(_SQL_DEBUG)
  XDBG << "[MYSQL INSERT]," << sql_string.str().c_str() << XEND;
#endif
  /* send SQL query */
  DBTimeCheck db_log(dblog_string.str());
  if (mysql_real_query(mysql, sql_string.str().c_str(), sql_string.str().size()))
  {
    XERR << "[MYSQL]query error! " << (DWORD)sql_string.str().size() << "," << sql_string.str().c_str() << XEND;
    for (DWORD i=0; i<recset.size(); ++i)
    {
      exeInsert(recset[i], false, duplicate);
    }
    return DBErrReturn;
  }
  return mysql_affected_rows(mysql);
}

QWORD DBConn::exeReplace(const xRecord &rec)
{
  std::stringstream sql_string;
  sql_string.str("");
  sql_string << "REPLACE INTO " << rec.m_field->m_strDatabase << "." << rec.m_field->m_strTable << " (";
  std::stringstream dblog_string;
  dblog_string.str("");
  dblog_string << sql_string.str();
  DWORD k = 0;

  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    sql_string << it.first.c_str();
    k++;
  }

  sql_string << ") VALUES(";
  k = 0;

  for (auto &it : rec.m_rs)
  {
    if (k)
      sql_string << ",";
    getDataStringByType(rec.getType(it.first), it.second, sql_string);
    k++;
  }
  sql_string << ")" ;
#ifdef _SQL_DEBUG
  XDBG << "[MYSQL REPLACE]" << sql_string.str() << XEND;
#endif
  /* send SQL query */
  DBTimeCheck db_log(dblog_string.str());
  if (mysql_real_query(mysql, sql_string.str().c_str(), sql_string.str().size()))
  {
    XERR << "[MYSQL]query error!" << sql_string.str() << XEND;
    return DBErrReturn;
  }
  return mysql_affected_rows(mysql);
}

QWORD DBConn::exeSelect(xField *field, xRecordSet &set, const char *where, const char *extraOpt)
{
  if (field == NULL) return DBErrReturn;

  std::stringstream query_string;
  query_string.str("");
  if (field->m_strValid.empty())
  {
    // 不用 select *
    // query_string << "SELECT * FROM " << field->m_table_name.c_str();
    query_string << "SELECT ";
    bool isFirst = true;
    for (auto &it : field->m_list)
    {
      if (isFirst)
      {
        isFirst = false;
      }
      else
      {
        query_string << ",";
      }
      query_string << it.first;
    }
    query_string << " FROM " << field->m_strDatabase << "." << field->m_strTable;
  }
  else
  {
    query_string << "SELECT " << field->m_strValid << " FROM " << field->m_strDatabase << "." << field->m_strTable;
    field->m_strValid.clear();
  }
  std::stringstream dblog_string;
  dblog_string.str("");
  dblog_string << query_string.str();
  if (NULL != where)
    query_string << " WHERE " << where;
  if (NULL != extraOpt)
    query_string << " " << extraOpt;
#ifdef _DEBUG
  XDBG << "[MYSQL SELECT]," << query_string.str().c_str() << XEND;
#endif
  MYSQL_RES *res;
  MYSQL_ROW row;
  DBTimeCheck db_log(dblog_string.str());
  if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
  {
    XERR << "[MYSQL]query error! " << query_string.str().c_str() << XEND;
    return DBErrReturn;
  }
  res = mysql_store_result(mysql);
  if (!res) 
  {
    XERR << "[MYSQL]query error! " << query_string.str().c_str() << "res null" << XEND;
    return DBErrReturn;
  }
  int num_fields = mysql_num_fields(res);
  std::vector<std::string> vec;
  vec.resize(num_fields);
  MYSQL_FIELD *fd;
  for (int i = 0 ; (fd = mysql_fetch_field(res)); i++)
  {
    // std::string str;
    // str = fd->name;
    vec[i] = fd->name;
#ifdef _SQL_DEBUG
    XDBG << "[SELECT]," << i << "," << num_fields << "," << fd->name << "," << fd->type << XEND;
#endif
  }
  int ret = (int)mysql_num_rows(res);
  if (ret)
  {
    set.reserve(ret);
  }
  unsigned long *lengths;
  while ((row = mysql_fetch_row(res)) != NULL)
  {
    lengths = mysql_fetch_lengths(res);
    xRecord record(field);
    for (int i = 0; i < num_fields; ++i)
    {
      switch (field->get(vec[i]))
      {
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_STRING:
          {
	    if (row[i] == nullptr)
	      record.putString(vec[i], "");
	    else
	      record.putString(vec[i], (char *)row[i]);
          }
          break;
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
          {
            record.put(vec[i], atoi(row[i]));
            // record.put(vec[i], (char *)row[i]);
          }
          break;
        case MYSQL_TYPE_LONG:
          {
            record.put(vec[i], atol(row[i]));
            // record.put(vec[i], (char *)row[i]);
          }
          break;
        case MYSQL_TYPE_LONGLONG:
          {
            char *pchar = nullptr;
            QWORD v = strtoull(row[i], &pchar, 10);
            record.put(vec[i], v);
            // record.put(vec[i], (char *)row[i]);
          }
          break;
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
          {
            record.put(vec[i], atof(row[i]));
            // record.put(vec[i], (char *)row[i]);
          }
          break;
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
          {
            record.putBin(vec[i], (unsigned char *)row[i], lengths[i]);
          }
          break;
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_SET:
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_GEOMETRY:
        case MYSQL_TYPE_NULL:
        default:
          {
            // record.put(vec[i], (char *)row[i]);
          }
          break;
      }
    }
    set.push(record);
  }

  mysql_free_result(res);
  return ret;
}

QWORD DBConn::exeRawSelect(xField *field, xRecordSet &set, const std::string& sqlStr)
{
  if (field == NULL) return DBErrReturn;

#ifdef _SQL_DEBUG
  XDBG << "[MYSQL SELECT]" << sqlStr.c_str() << XEND;
#endif
  MYSQL_RES *res = nullptr;
  MYSQL_ROW row;

  std::stringstream query_string;
  query_string.str("");

  query_string << "use " << field->m_strDatabase << ";" << sqlStr.c_str();
  DBTimeCheck db_log(query_string.str());
  if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
  {
    XERR << "[MYSQL]query error! " << query_string.str().c_str() << " error:" << mysql_error(mysql) << XEND;
    return DBErrReturn;
  }

  if(!mysql_next_result(mysql))
    res = mysql_store_result(mysql);
  if (!res) 
  {
    XERR << "[MYSQL]query error! " << query_string.str().c_str() << "res null" << XEND;
    return DBErrReturn;
  }
  int num_fields = mysql_num_fields(res);
  std::vector<std::string> vec;
  vec.resize(num_fields);
  MYSQL_FIELD *fd;
  for (int i = 0; (fd = mysql_fetch_field(res)); i++)
  {
    // std::string str;
    // str = fd->name;
    vec[i] = fd->name;
#ifdef _SQL_DEBUG
    XDBG << "[SELECT]," << i << "," << num_fields << "," << fd->name << "," << fd->type << XEND;
#endif
  }
  int ret = (int)mysql_num_rows(res);
  if (ret)
  {
    set.reserve(ret);
  }
  unsigned long *lengths;
  while ((row = mysql_fetch_row(res)) != NULL)
  {
    lengths = mysql_fetch_lengths(res);
    xRecord record(field);
    for (int i = 0; i < num_fields; ++i)
    {
      switch (field->get(vec[i]))
      {
      case MYSQL_TYPE_VAR_STRING:
      case MYSQL_TYPE_STRING:
      {
        record.putString(vec[i], (char *)row[i]);
      }
      break;
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_NEWDECIMAL:
      {
        record.put(vec[i], atoi(row[i]));
        // record.put(vec[i], (char *)row[i]);
      }
      break;
      case MYSQL_TYPE_LONG:
      {
        record.put(vec[i], atol(row[i]));
        // record.put(vec[i], (char *)row[i]);
      }
      break;
      case MYSQL_TYPE_LONGLONG:
      {
        record.put(vec[i], atoll(row[i]));
        // record.put(vec[i], (char *)row[i]);
      }
      break;
      case MYSQL_TYPE_FLOAT:
      case MYSQL_TYPE_DOUBLE:
      {
        record.put(vec[i], atof(row[i]));
        // record.put(vec[i], (char *)row[i]);
      }
      break;
      case MYSQL_TYPE_BLOB:
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
      {
        record.putBin(vec[i], (unsigned char *)row[i], lengths[i]);
      }
      break;
      case MYSQL_TYPE_BIT:
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_DATE:
      case MYSQL_TYPE_TIME:
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_YEAR:
      case MYSQL_TYPE_SET:
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_GEOMETRY:
      case MYSQL_TYPE_NULL:
      default:
      {
        // record.put(vec[i], (char *)row[i]);
      }
      break;
      }
    }
    set.push(record);
  }

  mysql_free_result(res);
  return ret;
}

QWORD DBConn::checkExist(const std::string &database, const char *table, const char *where)
{
  if (NULL == table) return DBErrReturn;

  std::stringstream query_string;
  query_string.str("");
  query_string << "SELECT 1 FROM " << database << "." << table;
  std::stringstream dblog_string;
  dblog_string.str("");
  dblog_string << query_string.str();
  if (NULL != where)
    query_string << " WHERE " << where;
  DBTimeCheck db_log(dblog_string.str());
  if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
  {
    XERR << "[MYSQL]," << query_string.str().c_str() << XEND;
    return DBErrReturn;
  }
  MYSQL_RES *res = mysql_store_result(mysql);;
  DWORD num = mysql_num_rows(res);
  mysql_free_result(res);
  return num;
}

QWORD DBConn::getNum(const std::string &database, const char *table, const char *where)
{
  if (NULL == table) return DBErrReturn;

  std::stringstream query_string;
  query_string.str("");
  query_string << "SELECT count(*) FROM " << database << "." << table;
  std::stringstream dblog_string;
  dblog_string.str("");
  dblog_string << query_string.str();
  if (NULL != where)
    query_string << " WHERE " << where;
#if defined(_SQL_DEBUG)
  XDBG << "[MYSQL GETNUM]," << query_string.str().c_str() << XEND;
#endif
  DBTimeCheck db_log(dblog_string.str());
  if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
  {
    XERR << "[MYSQL]," << query_string.str().c_str() << XEND;
    return DBErrReturn;
  }
  QWORD num = DBErrReturn;
  MYSQL_RES *res = mysql_store_result(mysql);
  if (!res)
  {
    XERR << "[MYSQL]query error! " << query_string.str().c_str() << "res null" << XEND;
    return DBErrReturn;
  }
  MYSQL_ROW row = mysql_fetch_row(res);
  if (row != NULL && mysql_num_fields(res) > 0)
    num = atoll(row[0]);
  mysql_free_result(res);
  return num;
}

bool DBConn::loadFields(const std::string &database)
{
  {
    std::stringstream query_string;
    query_string << "use " << database << ";";
    DBTimeCheck db_log(query_string.str());
    if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
    {
      XERR << "[MYSQL]query error! " << query_string.str().c_str() << XEND;
      return DBErrReturn;
    }
  }
  {
    std::stringstream query_string;
    query_string << "show tables";
    DBTimeCheck db_log(query_string.str());
    if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
    {
      XERR << "[MYSQL]query error! " << query_string.str().c_str() << XEND;
      return DBErrReturn;
    }
  }

  std::set<std::string> list;
  MYSQL_RES *res = mysql_store_result(mysql);
  if (!res)
  {
    XERR << "[MYSQL]query error! show tables res null" << XEND;
    return DBErrReturn;
  }
  MYSQL_ROW row;
  MYSQL_FIELD *fd;
  int num_fields = mysql_num_fields(res);
  while ((row = mysql_fetch_row(res)) != NULL)
  {
    for (int i = 0; i < num_fields; ++i)
    {
      if (row[i])
      {
        xField *field = new xField(database.c_str(), row[i]);
        m_oFieldsM.addField(field);
        list.insert((const char *)row[i]);
      }
    }
  }
  int ret = (int)mysql_num_rows(res);
  mysql_free_result(res);

  for (auto &it : list)
  {
    xField *field = m_oFieldsM.getField(database, it);
    if (field)
    {
      std::stringstream query_string;
      query_string.str("");
      query_string << "SELECT * FROM " << database << "." << it << " LIMIT 0";
      DBTimeCheck db_log(query_string.str());
      if (mysql_real_query(mysql, query_string.str().c_str(), query_string.str().size()))
      {
        XERR << "[MYSQL]query error! " << query_string.str().c_str() << XEND;
        continue;
      }
      res = mysql_store_result(mysql);
      for (int i = 0 ; (fd = mysql_fetch_field(res)); i++)
      {
#ifdef _SQL_DEBUG
        XDBG << "[表-filed],:" << it.c_str() << "," << fd->name << "," << fd->type << XEND;
#endif
        field->m_list[fd->name] = fd->type;
      }
      mysql_free_result(res);
    }
  }

  return ret;
}

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

DBConnPool::DBConnPool()
{
  XLOG << "[MYSQL_TYPE_TINY]," << MYSQL_TYPE_TINY << XEND;
  XLOG << "[MYSQL_TYPE_SHORT]," << MYSQL_TYPE_SHORT << XEND;
  XLOG << "[MYSQL_TYPE_LONG]," << MYSQL_TYPE_LONG << XEND;
  XLOG << "[MYSQL_TYPE_INT24]," << MYSQL_TYPE_INT24 << XEND;
  XLOG << "[MYSQL_TYPE_LONGLONG]," << MYSQL_TYPE_LONGLONG << XEND;
  XLOG << "[MYSQL_TYPE_DECIMAL]," << MYSQL_TYPE_DECIMAL << XEND;
  XLOG << "[MYSQL_TYPE_NEWDECIMAL]," << MYSQL_TYPE_NEWDECIMAL << XEND;
  XLOG << "[MYSQL_TYPE_FLOAT]," << MYSQL_TYPE_FLOAT << XEND;
  XLOG << "[MYSQL_TYPE_DOUBLE]," << MYSQL_TYPE_DOUBLE << XEND;
  XLOG << "[MYSQL_TYPE_BIT]," << MYSQL_TYPE_BIT << XEND;
  XLOG << "[MYSQL_TYPE_TIMESTAMP]," << MYSQL_TYPE_TIMESTAMP << XEND;
  XLOG << "[MYSQL_TYPE_DATE]," << MYSQL_TYPE_DATE << XEND;
  XLOG << "[MYSQL_TYPE_TIME]," << MYSQL_TYPE_TIME << XEND;
  XLOG << "[MYSQL_TYPE_DATETIME]," << MYSQL_TYPE_DATETIME << XEND;
  XLOG << "[MYSQL_TYPE_YEAR]," << MYSQL_TYPE_YEAR << XEND;
  XLOG << "[MYSQL_TYPE_STRING]," << MYSQL_TYPE_STRING << XEND;
  XLOG << "[MYSQL_TYPE_VAR_STRING]," << MYSQL_TYPE_VAR_STRING << XEND;
  XLOG << "[MYSQL_TYPE_BLOB]," << MYSQL_TYPE_BLOB << XEND;
  XLOG << "[MYSQL_TYPE_TINY_BLOB]," << MYSQL_TYPE_TINY_BLOB << XEND;
  XLOG << "[MYSQL_TYPE_MEDIUM_BLOB]," << MYSQL_TYPE_MEDIUM_BLOB << XEND;
  XLOG << "[MYSQL_TYPE_LONG_BLOB]," << MYSQL_TYPE_LONG_BLOB << XEND;
  XLOG << "[MYSQL_TYPE_SET]," << MYSQL_TYPE_SET << XEND;
  XLOG << "[MYSQL_TYPE_ENUM]," << MYSQL_TYPE_ENUM << XEND;
  XLOG << "[MYSQL_TYPE_GEOMETRY]," << MYSQL_TYPE_GEOMETRY << XEND;
  XLOG << "[MYSQL_TYPE_NULL]," << MYSQL_TYPE_NULL << XEND;
}

DBConnPool::~DBConnPool()
{
  final();
}

void DBConnPool::final()
{
  m_pDBConn = nullptr;

  std::set<DBConn *> tmp;
  for (auto &it : m_list)
  {
    tmp.insert(it.second);
  }
  m_list.clear();
  for (auto &it : tmp)
  {
    DBConn *p = it;
    SAFE_DELETE(p);
  }
}

xField* DBConnPool::getField(const std::string& database, const std::string& table)
{
  m_pDBConn = getDBConn(database);
  if (m_pDBConn)
  {
    xField *p = m_pDBConn->getField(database, table);
    if (p)
    {
      return p;
    }
    else
    {
      XERR_T("[Field],查找失败,%s:%s", database.c_str(), table.c_str());
    }
  }
  else
  {
    XERR_T("[Field],查找失败,找不到conn,%s:%s", database.c_str(), table.c_str());
  }
  return nullptr;
}

bool DBConnPool::addDBConn(const std::string &database)
{
  if (m_oConfig.empty())
  {
    XERR_T("[DBConnPool],初始化失败,没有配置");
    return false;
  }

  std::vector<xLuaData> cfg;

  // 找到对应数据库的地址
  for (auto it : m_oConfig)
  {
    if (it.getTableString("database") == database)
    {
      cfg.push_back(it);
    }
  }

  // 没单独设置取通用的
  if (cfg.empty())
  {
    for (auto it : m_oConfig)
    {
      if (!it.has("database"))
      {
        cfg.push_back(it);
      }
    }
  }

  if (cfg.empty())
  {
    XERR_T("[DBConnPool],初始化失败,参数为空,找不到配置:%s", database.c_str());
    return false;
  }

  std::random_shuffle(cfg.begin(), cfg.end());

  auto connIt = m_list.find(database);
  if (connIt != m_list.end())
  {
    // 找到database库的连接
    // 如果有相同的地址，则不处理
    // 否则重新连接
    DBConn *pDBConn = connIt->second;
    if (pDBConn)
    {
      const DBConnAccount &account = pDBConn->getDBConnAccount();
      for (auto it : cfg)
      {
        if ((account.m_strServerIP == it.getTableString("ip")) && ((int)account.m_dwPort == it.getTableInt("port")))
        {
          XLOG_T("[DBConnPool],初始化DBConn成功,重复的连接:%s,%u,%s", account.m_strServerIP.c_str(), account.m_dwPort, database.c_str());
          // 找到地址
          return true;
        }
      }
      // 重新连接
      for (auto it : cfg)
      {
        if (pDBConn->init(it.getTableString("ip"), it.getTableString("user"), it.getTableString("password"), it.getTableInt("port"), database))
        {
          return true;
        }
      }
      XERR_T("[DBConnPool],DBConn初始化失败,%s", database.c_str());
      return false;
    }
  }
  else
  {
  }

  // 没有相同的库则查找相同的地址
  for (auto &it : m_list)
  {
    DBConn *pDBConn = it.second;
    if (pDBConn)
    {
      const DBConnAccount &account = pDBConn->getDBConnAccount();
      for (auto &iter : cfg)
      {
        if ((account.m_strServerIP == iter.getTableString("ip")) && ((int)account.m_dwPort == iter.getTableInt("port")))
        {
          m_list[database] = pDBConn;
          pDBConn->loadFields(database);
          XLOG_T("[DBConnPool],初始化DBConn成功,server,%s:%u:%s,共用连接:%s", account.m_strServerIP.c_str(), account.m_dwPort, database.c_str(), it.first.c_str());
          return true;
        }
      }
    }
  }

  // 都没有则新建
  DBConn *pDBConn = new DBConn;
  for (auto it : cfg)
  {
    if (pDBConn->init(it.getTableString("ip"), it.getTableString("user"), it.getTableString("password"), it.getTableInt("port"), database))
    {
      m_list[database] = pDBConn;
      XLOG_T("[DBConnPool],初始化DBConn成功,server,%s:%u:%s", it.getTableString("ip"), it.getTableInt("port"), database.c_str());
      return true;
    }
  }

  XERR_T("[DBConnPool],DBConn初始化失败,%s", database.c_str());
  SAFE_DELETE(pDBConn);
  return false;
}

DBConn* DBConnPool::getDBConn(const std::string &database)
{
  auto it = m_list.find(database);
  if (it != m_list.end())
  {
    return it->second;
  }
  XERR_T("[DBConnPool],找不到对应的连接,%s", database.c_str());
  return nullptr;
}

void DBConnPool::delDBConn(const std::string &database)
{
  DBConn *pDBConn = nullptr;
  auto it = m_list.find(database);
  if (it != m_list.end())
  {
    pDBConn = it->second;
    m_list.erase(it);
    XLOG_T("[DBConnPool],断开连接,%s", database.c_str());
  }

  if (pDBConn)
  {
    for (auto &it : m_list)
    {
      if (it.second == pDBConn)
      {
        return;
      }
    }
    SAFE_DELETE(pDBConn);
    XLOG_T("[DBConnPool],删除对象:%s", database.c_str());
  }
}

void DBConnPool::reload()
{
  std::set<std::string> tmp;
  for (auto it : m_list)
  {
    tmp.insert(it.first);
  }
  for (auto it : tmp)
  {
    addDBConn(it);
  }
}
