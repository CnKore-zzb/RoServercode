/*#pragma once
#include "xEntryManager.h"
#include "xTable.h"

template<typename B>
class xTDataMgr : public xEntryManager<xEntryID, xEntryMultiName>
{
  public:
    xTDataMgr(std::string name)
    {
      m_tableName = name; 
    }
    virtual ~xTDataMgr()
    {
      clear();
    }
    bool load()
    {
      clear();
      xTable table(m_tableName);
      if (!table.read() )
        return false;

      DWORD linenum = table.getLineNum();
      for(DWORD n=1;n<=linenum;n++)
      {
        B* data = NEW B;
        data->init(table,n);
        if (!AddEntry(data))
        {
          XERR("[Table],%s,数据错误,%u,%s",m_tableName.c_str(),data->id(),data->name());
          return false;
        }
      }
      XLOG("[Table],%s,表头:%u,读取行数%u/%u",m_tableName.c_str(),table.getFieldNum(),size(),linenum);
      return true;
    }
    void clear()
    {
      xEntryID::Iter it = xEntryID::ets_.begin(), end = xEntryID::ets_.end();
      xEntryID::Iter temp;
      for(;it!=end;it++)
      {
        temp = it++;
        RemoveEntry(temp->second);
        SAFE_DELETE(temp->second);
      }
    }
  private:
    std::string m_tableName;
};
*/
