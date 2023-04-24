#pragma once
#include <map>
#include <string>
#include <mysql.h>
#include "xlib/xDefine.h"
#include "xlib/xTools.h"
#include "xlib/xDBMeta.h"
#include "xlib/xFunctionTime.h"
#include "xlib/xDBFields.h"
#include "xlib/xNoncopyable.h"
#include "xlib/xSingleton.h"
#include "xLuaTable.h"

using std::string;

#define DBErrReturn QWORD_MAX

enum class DB_CONN_TYPE
{
  DB_CONN_CENTER = 1,
  DB_CONN_PLAT   = 2,
  DB_CONN_GAME   = 3,
  DB_CONN_LOG    = 4,
};

class DBConnAccount
{
  public:
    DBConnAccount()
    {
      clear();
    }
    ~DBConnAccount()
    {
    }

  public:
    void clear()
    {
      m_strServerIP.clear();
      m_strUser.clear();
      m_strPassword.clear();
      m_dwPort = 0;
    }

  public:
    std::string m_strServerIP;

    std::string m_strUser;
    std::string m_strPassword;
    DWORD m_dwPort = 0;
};

class DBConn;
class DBSqlCache
{
  public:
    void init(int maxCount);
    //static 获取key
    static QWORD generateKey(const xRecord &rec, bool updateOnDup);

    bool insert(DBConn* pDbConn, const std::string& db, const xRecord &rec, bool updateOnDup);
    QWORD exeSql(DBConn* pDbConn, const std::string& db, bool bForce=false);
    const std::string& getSql();
    bool needFlush();
    void clear();
  private:
    int m_maxCount = 1;
    std::string  m_strSql;
    int m_curCount = 0;
    std::string m_updateSql;
    bool m_needFlush = false;
};

class DBSqlCacheMgr : public xNoncopyable
{
  public:
    virtual ~DBSqlCacheMgr();

  public:
    void init(DBConn* pDbConn, int maxCount, const std::string& db);

    bool insert(const xRecord &rec, bool updateOnDup = false);
    void flush();

  private:
    std::map<std::size_t/*key*/, DBSqlCache> m_mapCache;

    int m_maxCount = 0;
    DBConn* m_pDbConn;
    std::string m_database;
};

class DBConn : private xNoncopyable
{
  friend class DBConnPool;
  friend class DBSqlCache;
  public:
    DBConn();
    virtual ~DBConn()
    {
      close();
    }

  private:
    bool init(const char *server, const char *user, const char *password, DWORD port, const std::string &database);
    bool reconnect();
    void close();

    QWORD exeSql(const std::string &database, const char *sql);
    QWORD checkExist(const std::string &database, const char *table, const char *where);
    QWORD getNum(const std::string &database, const char *table, const char *where);

    QWORD exeDelete(xField *field, const char *where);
    QWORD exeUpdate(const xRecord &record, const char *where);
    bool getUpdateString(const xRecord &record, const char *where, std::string &str);
    QWORD exeInsert(const xRecord &record, bool isRet, bool duplicate);
    QWORD exeInsertSet(xRecordSet &recset, bool duplicate);
    QWORD exeReplace(const xRecord &record);

    QWORD exeSelect(xField *field, xRecordSet &set, const char *where, const char *extraOpt);

    QWORD exeRawSelect(xField *field, xRecordSet &set, const std::string& sqlStr);
  public:
    const DBConnAccount& getDBConnAccount() { return m_oAccount; }
  private:
    bool loadFields(const std::string &database);
    void getDataStringByType(enum_field_types type, const xData &data, std::stringstream &out);

  private:
    MYSQL *mysql = nullptr;
    DBConnAccount m_oAccount;

  public:
    xField* getField(const std::string& database, const std::string& table)
    {
      return m_oFieldsM.getField(database, table);
    }
  private:
    xFieldsM m_oFieldsM;
};

#define GET_CONN(database) \
  m_pDBConn = getDBConn(database); \
if (nullptr == m_pDBConn) return DBErrReturn; \
if (!m_pDBConn->reconnect()) { if (!addDBConn(database)) return DBErrReturn; }

class DBConnPool
{
  public:
    ~DBConnPool();
    DBConnPool();

  public:
    void final();
    // 插入多条数据时优先使用
    QWORD exeInsertSet(xRecordSet &recset, bool duplicate=false)
    {
      if (recset.empty()) return DBErrReturn;
      const xRecord &rec = recset[0];
      if (!rec.m_field) return DBErrReturn;
      GET_CONN(rec.m_field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->exeInsertSet(recset, duplicate);
    }
    // 插入单条数据
    // duplicate:key有重复数据是否修改
    QWORD exeInsert(const xRecord &record, bool isRet = true, bool duplicate=false)
    {
      if (!record.m_field) return DBErrReturn;
      GET_CONN(record.m_field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->exeInsert(record, isRet, duplicate);
    }
    QWORD exeReplace(const xRecord &record)
    {
      if (!record.m_field) return DBErrReturn;
      GET_CONN(record.m_field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->exeReplace(record);
    }
    QWORD exeUpdate(const xRecord &record, const char *where)
    {
      if (!record.m_field) return DBErrReturn;
      GET_CONN(record.m_field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->exeUpdate(record, where);
    }
    bool getUpdateString(const xRecord &record, const char *where, std::string &str)
    {
      if (!record.m_field) return DBErrReturn;
      GET_CONN(record.m_field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->getUpdateString(record, where, str);
    }
    // 默认select *, 可通过xField的setValid来设置要读取的字段
    QWORD exeSelect(xField *field, xRecordSet &set, const char *where, const char *extraOpt = NULL)
    {
      if (!field) return DBErrReturn;
      GET_CONN(field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->exeSelect(field, set, where, extraOpt);
    }

    QWORD exeRawSelect(xField *field, xRecordSet &set, const std::string& sqlStr)
    {
      if (!field) return DBErrReturn;
      GET_CONN(field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->exeRawSelect(field, set, sqlStr);
    }

    QWORD exeDelete(xField *field, const char *where)
    {
      if (!field) return DBErrReturn;
      GET_CONN(field->m_strDatabase);

      FunctionTime_Scope;
      return m_pDBConn->exeDelete(field, where);
    }
    QWORD exeSql(const std::string database, const char *sql)
    {
      GET_CONN(database);

      FunctionTime_Scope;
      return m_pDBConn->exeSql(database, sql);
    }
    QWORD checkExist(const std::string database, const char *tableName, const char *where)
    {
      GET_CONN(database);

      FunctionTime_Scope;
      return m_pDBConn->checkExist(database, tableName, where);
    }
    QWORD getNum(const std::string database, const char *tableName, const char *where)
    {
      GET_CONN(database);

      FunctionTime_Scope;
      return m_pDBConn->getNum(database, tableName, where);
    }

    xField* getField(const std::string& database, const std::string& table);

  public:
    bool addDBConn(const std::string &database);
    void delDBConn(const std::string &database);
    bool empty() { return m_list.empty(); }
    void reload();
  private:
    DBConn* getDBConn(const std::string &database);
    std::map<std::string, DBConn *> m_list;

    // 临时指针
    DBConn* m_pDBConn = nullptr;

    // config
  public:
    void setConfig(std::vector<xLuaData>& vecIps)
    {
      m_oConfig.clear();
      m_oConfig.reserve(vecIps.size());
      m_oConfig.assign(vecIps.begin(), vecIps.end());
    }
  private:
    std::vector<xLuaData> m_oConfig;
};

#define REGION_DB thisServer->getRegionDBName()
#define PLAT_DB thisServer->getPlatDBName()
