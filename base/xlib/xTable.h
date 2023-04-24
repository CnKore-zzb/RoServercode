#pragma once
#include "xDefine.h"
//#include "xNewMem.h"

struct xTable
{
  public:
    xTable(std::string name);
    DWORD getLineNum()const{return m_records.size();}
    DWORD getFieldNum()const{return m_fieldNum;}
    bool read();
    bool parseTable(const char* data, DWORD len);
    bool parseTitle(std::string title, std::vector<std::string>& fs);
    bool parseRecord(std::string data, const std::vector<std::string>& fs);

    template<typename B>
    B getData(DWORD line,std::string fname)const
    {
      if(line>getFieldNum())
        return 0;
      Record::const_iterator it=m_records[line-1].find(fname);
      if(it==m_records[line-1].end())
        return 0 ;
      return atoi(it->second.c_str());
    }

    QWORD getDataLong(DWORD line,std::string fname)const
    {
      if(line>getFieldNum())
        return 0;
      Record::const_iterator it=m_records[line-1].find(fname);
      if(it==m_records[line-1].end())
        return 0 ;
      return atoll(it->second.c_str());
    }

    const char* getDataStr(DWORD line,std::string fname)const
    {
      if(line>getFieldNum())
        return 0;
      Record::const_iterator it=m_records[line-1].find(fname);
      if(it==m_records[line-1].end())
        return 0 ;
      return it->second.c_str();
    }

  private:
    DWORD m_fieldNum;
    std::string m_name;

    typedef std::map<std::string,std::string> Record;
    typedef std::vector<Record> Records;
    Records m_records;
};

