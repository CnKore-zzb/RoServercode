#pragma once
#include <mysql.h>
#include <string>
#include "xlib/xDefine.h"
//#include "xNewMem.h"
#include "xlib/xTools.h"
#include "xlib/xSingleton.h"
#include "xNoncopyable.h"

/*
   enum enum_field_types 

   MYSQL_TYPE_TINY TINYINT字段
   MYSQL_TYPE_SHORT SMALLINT字段
   MYSQL_TYPE_LONG INTEGER字段
   MYSQL_TYPE_INT24 MEDIUMINT字段
   MYSQL_TYPE_LONGLONG BIGINT字段
   MYSQL_TYPE_DECIMAL DECIMAL或NUMERIC字段
   MYSQL_TYPE_NEWDECIMAL 精度数学DECIMAL或NUMERIC
   MYSQL_TYPE_FLOAT FLOAT字段
   MYSQL_TYPE_DOUBLE DOUBLE或REAL字段
   MYSQL_TYPE_BIT BIT字段
   MYSQL_TYPE_TIMESTAMP TIMESTAMP字段
   MYSQL_TYPE_DATE DATE字段
   MYSQL_TYPE_TIME TIME字段
   MYSQL_TYPE_DATETIME DATETIME字段
   MYSQL_TYPE_YEAR YEAR字段
   MYSQL_TYPE_STRING CHAR字段
   MYSQL_TYPE_VAR_STRING VARCHAR字段
   MYSQL_TYPE_BLOB BLOB或TEXT字段（使用max_length来确定最大长度）
   MYSQL_TYPE_SET SET字段
   MYSQL_TYPE_ENUM ENUM字段
   MYSQL_TYPE_GEOMETRY Spatial字段
   MYSQL_TYPE_NULL NULL-type字段
   MYSQL_TYPE_CHAR 不再重视，用MYSQL_TYPE_TINY取代
   */

class xField
{
  public:
    explicit xField(const std::string& database, const char *table);
    ~xField();

  public:
    bool has(const std::string& str) const
    {
      return (m_list.find(str) != m_list.end());
    }
    enum_field_types get(const std::string& str) const
    {
      auto iter = m_list.find(str);
      if (iter == m_list.end()) return MYSQL_TYPE_NULL;
      return iter->second;
    }
    void clear()
    {
      m_strDatabase.clear();
      m_strTable.clear();
      m_list.clear();
      m_strValid.clear();
    }

  public:
    std::string m_strDatabase;
    std::string m_strTable;
    /* 字段名称  字段类型 */
    std::map<std::string, enum_field_types> m_list;

  public:
    void setValid(const std::string& str)
    {
      m_strValid = str;
    }

    std::string m_strValid;
};

class xData
{
  public:
    xData() {}
    ~xData() {}

  public:
    void put(const std::string& str)
    {
      if (!m_data.empty())
      {
        m_data.clear();
      }
      if (str.empty())
      {
        return;
      }
      m_data.resize(str.size() + 1);
      memcpy(&m_data[0], str.c_str(), str.size() + 1);
    }
    void put(unsigned char *p, unsigned int size)
    {
      if (!p) return;

      if (!m_data.empty())
      {
        m_data.clear();
      }

      m_data.resize(size);
      bcopy(p, (unsigned char *)&m_data[0], size);
    }
    long getLong() const
    {
      if (m_data.empty()) return 0;
      return atol((const char *)&m_data[0]);
    }
    long long getLongLong() const
    {
      if (m_data.empty()) return 0;
      return atoll((const char *)&m_data[0]);
    }
    float getFloat() const
    {
      if (m_data.empty()) return 0;
      return atof((const char *)&m_data[0]);
    }
    int getInt() const
    {
      if (m_data.empty()) return 0;
      return atoi((const char *)&m_data[0]);
    }
    WORD getWORD() const
    {
      if (m_data.empty()) return 0;
      return atoi((const char *)&m_data[0]);
    }
    DWORD getDWORD() const
    {
      if (m_data.empty()) return 0;
      return atoi((const char *)&m_data[0]);
    }
    QWORD getQWORD() const
    {
      if (m_data.empty()) return 0;
      //return atoll((const char *)&m_data[0]);
      char *endptr;
      return strtoull((const char *)&m_data[0], &endptr, 10);

    }
    const char* getString() const
    {
      if (m_data.empty()) return "";
      return (const char *)&m_data[0];
    }
    const unsigned char* getBin() const
    {
      if (m_data.empty()) return (const unsigned char *)"";
      return (const unsigned char *)&m_data[0];
    }
    DWORD getBinSize() const
    {
      return m_data.size();
    }
    DWORD size() const
    {
      return m_data.size();
    }
    void clear()
    {
      m_data.clear();
    }
  private:
    std::vector<unsigned char> m_data;
};

class xFieldsM : private xNoncopyable
{
  public:
    xFieldsM() { m_list.clear(); }
    virtual ~xFieldsM();

  public:
    void final();

  public:
    bool addField(xField *field);
    xField* getField(const std::string& database, const std::string& table);

  private:
    // database table xfield
    std::map<std::string, std::map<std::string, xField *>> m_list;
};

class xRecord
{
  public :
    explicit xRecord(xField* field);
    ~xRecord();

  public:
    inline enum_field_types getType(const std::string &str) const
    {
      if (!m_field) return MYSQL_TYPE_NULL;
      return m_field->get(str);
    }

    template<typename T>
      void put(const std::string& type, T value)
      {
        if (!m_field->has(type)) return;

        std::stringstream str;
        str.str("");

        str << value;

        m_rs[type].put(str.str());
        return;
      }

    template<typename T>
      T get(const std::string type) const
      {
        switch (m_field->get(type))
        {
          case MYSQL_TYPE_VAR_STRING:  // VARCHAR字段
          case MYSQL_TYPE_STRING:  // CHAR字段
            {
              // return m_rs[type].getString();
            }
            break;
          case MYSQL_TYPE_TINY:  // TINYINT字段
          case MYSQL_TYPE_SHORT:  // SMALLINT字段
          case MYSQL_TYPE_INT24:  // MEDIUMINT字段
          case MYSQL_TYPE_DECIMAL:  // DECIMAL或NUMERIC字段
          case MYSQL_TYPE_NEWDECIMAL:  // 精度数学DECIMAL或NUMERIC
            {
              auto iter = m_rs.find(type);
              if (iter == m_rs.end()) return T();
              return iter->second.getInt();
            }
            break;
          case MYSQL_TYPE_LONG:  // INTEGER字段
            {
              auto iter = m_rs.find(type);
              if (iter == m_rs.end()) return T();
              return iter->second.getLong();
            }
            break;
          case MYSQL_TYPE_LONGLONG:  // BIGINT字段
            {
              auto iter = m_rs.find(type);
              if (iter == m_rs.end()) return T();
              return iter->second.getQWORD();
            }
            break;
          case MYSQL_TYPE_FLOAT:  // FLOAT字段
          case MYSQL_TYPE_DOUBLE:  // DOUBLE或REAL字段
            {
              auto iter = m_rs.find(type);
              if (iter == m_rs.end()) return T();
              return iter->second.getFloat();
            }
            break;
          case MYSQL_TYPE_BLOB:  // BLOB或TEXT字段（使用max_length来确定最大长度）
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
            {
              // return m_rs[type].getString();
            }
            break;
        }
        return 0;
      }

    void putString(const std::string& type, const std::string& value)
    {
      if (!m_field->has(type)) return;

      m_rs[type].put(value);
    }

    const char* getString(const std::string& type) const
    {
      auto iter = m_rs.find(type);
      if (iter == m_rs.end()) return "";
      return iter->second.getString();
    }

    void putBin(const std::string& type, unsigned char *p, unsigned int size)
    {
      if (!m_field->has(type)) return;

      m_rs[type].put(p, size);
    }

    const unsigned char* getBin(const std::string& type) const
    {
      auto iter = m_rs.find(type);
      if (iter == m_rs.end()) return (const unsigned char *)"";
      return iter->second.getBin();
    }
    DWORD getBinSize(const std::string& type) const
    {
      auto iter = m_rs.find(type);
      if (iter == m_rs.end()) return 0u;
      return iter->second.getBinSize();
    }

  public:
    std::map<std::string, xData> m_rs;
    typedef std::map<std::string, xData>::iterator iter;
    typedef std::map<std::string, xData>::const_iterator const_iter;
    xField *m_field;
};

class xRecordSet
{
  public :
    xRecordSet() {}
    ~xRecordSet() {}

    void push(const xRecord &rec) { m_list.push_back(rec); }
    const xRecord& operator[](int num) { return m_list[num]; }
    DWORD size() const { return m_list.size(); }
    bool empty() const { return m_list.empty(); }
    void clear() { m_list.clear(); }
    void reserve(DWORD size) { m_list.reserve(size); }

  private :
    std::vector<xRecord> m_list;
};
