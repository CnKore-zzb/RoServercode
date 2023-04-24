#pragma once
extern "C"
{
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}
#include <string>
#include "xSingleton.h"
#include "xDefine.h"
#include "xDBFields.h"
#include "json/json.h"
#include "ProtoCommon.pb.h"

using namespace Cmd;

class xLuaData
{
  public:
    xLuaData()
    {
    }
    ~xLuaData()
    {
    }

  public:
    void clear()
    {
      m_data.clear();
      m_table.clear();
    }
    void copyFrom(const xLuaData &data)
    {
      m_data = data.m_data;
      m_table = data.m_table;
    }

    // 数据类
  public:
    template<typename T>
      void set(T value, bool isNumber=false)
      {
        std::stringstream str;
        str.str("");

        str << value;

        m_data.put(str.str());
        m_isNumber = isNumber;
      }

    int getInt() const
    {
      return m_data.getInt();
    }
    float getFloat() const
    {
      return m_data.getFloat();
    }
    const char* getString() const
    {
      return m_data.getString();
    }
    QWORD getQWORD() const
    {
      return m_data.getQWORD();
    }
    bool isNumber() const
    {
      return m_isNumber;
    }
    DWORD getSize() const
    {
      return m_data.size();
    }

  private:
    xData m_data;
    // 用于生成Json格式时，是否需要引号
    bool m_isNumber = false;

    // table类
  public:
    template<typename T>
      void setData(std::string key, T value, bool isNumber=false)
      {
        m_table[key].set<T>(value, isNumber);
      }

    template<typename T>
      void setDataForEach(std::string key, T value)
      {
        for (auto it=m_table.begin(); it!=m_table.end(); ++it)
        {
          it->second.m_table[key].set<T>(value);
        }
      }

    int getTableInt(std::string key) const
    {
      auto it = m_table.find(key);
      if (it!=m_table.end())
        return it->second.getInt();
      return 0;
    }
    QWORD getTableQWORD(std::string key) const
    {
      auto it = m_table.find(key);
      if (it != m_table.end())
        return it->second.getQWORD();
      return 0;
    }
    float getTableFloat(std::string key) const
    {
      auto it = m_table.find(key);
      if (it!=m_table.end())
        return it->second.getFloat();
      return 0.0f;
    }
    const char* getTableString(std::string key) const
    {
      auto it = m_table.find(key);
      if (it!=m_table.end())
        return it->second.getString();
      return "";
    }

    const xLuaData& getData(const char *key) const
    {
      if (key)
      {
        auto it = m_table.find(key);
        if (it!=m_table.end())
          return it->second;
      }
      //xLuaData *d = new xLuaData;
      //return *d;
      static const xLuaData empty;
      return empty;
    }

    xLuaData& getMutableData(const char *key)
    {
      return m_table[key];
    }

    void set(std::string &key, xLuaData &data)
    {
      m_table[key] = data;
    }

    bool has(std::string key) const
    {
      return m_table.find(key)!=m_table.end();
    }

    template<class T> void foreach(T func)
    {
      for (auto it=m_table.begin(); it!=m_table.end(); ++it)
      {
        func(it->first, it->second);
      }
    }
    bool isTable() const
    {
      return !m_table.empty();
    }
    void toString(std::stringstream& stream) const
    {
      //std::stringstream stream;
      //stream << "~";
      if (isTable())
      {
        bool isFirst = true;
        for (auto it=m_table.begin(); it!=m_table.end(); ++it)
        {
          if (isFirst)
          {
            isFirst = false;
          }
          else
          {
          }
          stream << it->first << "=";
          if (it->second.isTable())
          {
            stream << "{";
            it->second.toString(stream);
            stream << "}" << " ";
          }
          else
          {
            stream << it->second.getString() <<" ";
          }
        }
      }
      //stream << "~";
    }
    void toData(ConfigParam* pConfig) const;
    void toData(Param* pParam) const;
    void toJsonString(std::stringstream &stream) const;
    bool fromJsonString(std::string str);
    bool parseFromString(std::string str, const char *table);
    void getIDList(TSetDWORD &set);
    void getIDList(TVecDWORD &vec);
    void setIDList(const char* key, const TSetDWORD &set);
    std::map<std::string, xLuaData> m_table;
  private:
    void getJsonValue(Json::Value root);
    void toJsonData(std::stringstream &stream, bool isFirst = false) const;
};

typedef std::map<DWORD, xLuaData> xLuaTableData;
typedef std::map<std::string, xLuaData> xLuaTableSData;

class xLuaTable : public xSingleton<xLuaTable>
{
  friend class xSingleton<xLuaTable>;
  public:
    ~xLuaTable();
  private:
    xLuaTable();

  public:
    bool open(const char *file);
    bool load(std::string str, const char *table, xLuaData &out);
    bool getLuaTable(const char *table, xLuaTableData &out);
    bool getLuaData(const char *table, xLuaData &out);
    bool getMultiTable(const char *table, LuaMultiTable &out);
    bool getTable(const char *table, DWORD key, std::map<std::string, std::string> &out);
    bool getTableValue(const char *table, DWORD key, const char *field, DWORD &out);
    bool getValue(const char* name, DWORD &out);

  private:
    // bool getLuaTable(const char *table, xLuaData &out); 使用
    bool getSubTable(xLuaData &out);
    bool getValue(xLuaData &data);
    bool filterTable(const char* table, DWORD id);

  public:
    void test();

  private:
    lua_State *m_state;
};
