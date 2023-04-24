#include "xTable.h"
#include "xTools.h"

xTable::xTable(std::string name)
{
  m_fieldNum = 0;
  m_name = name;
}

bool xTable::read()
{
  FILE *fp=NULL;
  if ((fp=fopen( m_name.c_str(), "r"))==NULL)
  {
    XERR << "[Table],找不到文件" << m_name.c_str() << XEND;
    return false;
  }

  fseek(fp,0,SEEK_END);
  DWORD len=ftell(fp)+1;
  char *content = new char[len];
  if(!content)
  {
    XERR << "[Table]," << m_name.c_str() << ",new content failed" << XEND;
    fclose(fp);
    return false;
  }
  bzero(content,len);
  fseek(fp,0,SEEK_SET);
  DWORD realLen=fread(content,1,len,fp);

  parseTable(content, realLen);
  SAFE_DELETE_VEC(content);
  fclose(fp);
  return true;
}

bool xTable::parseTable(const char* data, DWORD len)
{
  std::vector<std::string> linestr;
  const char* pc = data;
  const char* pbegin = pc;
  DWORD recordlen = 0;
  while (pc<=(data+len))
  {
    if(strncmp(pc,"\r\n",2) == 0)
    {
      recordlen = pc-pbegin;
      if(recordlen)
      {
        char *linecontent = new char[recordlen+1];
        bzero(linecontent,recordlen+1);
        bcopy(pbegin, linecontent, recordlen);
        std::string temp(linecontent);
        linestr.push_back(temp);
        SAFE_DELETE_VEC(linecontent);
        pbegin = pc+2;
        pc += 2;
      }
    }
    else
      ++pc;
  }
#ifdef _WUWENJUAN_DEBUG
  XLOG << "[Table]," << m_name.c_str() << ",read " << linestr.size() << " line" << XEND;
#endif

  if (linestr.empty())
  {
    XERR << "[Table]," << m_name.c_str() << ",empty" << XEND;
    return false;
  }

  //解析表头
  std::vector<std::string> fields;
  if (!parseTitle(linestr[0],fields))
  {
    XERR << "[Table]," << m_name.c_str() << ",解析表头失败" << XEND;
    return false;
  }

  for(DWORD n=1;n<linestr.size();n++)
  {
    if( !parseRecord(linestr[n],fields) )
    {
      XERR << "[Table]," << m_name.c_str() << ",解析数据失败,第" << n << "行" << XEND;
      return false;
    }
  }
  return true;
}

bool xTable::parseTitle(std::string title, std::vector<std::string>& fs)
{
  stringTok(title,"\t",fs);
  if(fs.empty())
  {
    XERR << "[Table]," << m_name.c_str() << ",解析表头失败，表头为空" << XEND;
    return false;
  }
  else
  {
    m_fieldNum = fs.size();
    return true;
  }
}

bool xTable::parseRecord(std::string data, const std::vector<std::string>& fs)
{
  std::vector<std::string> datavec;
  stringTok(data, "\t", datavec);
  if (fs.size() != datavec.size())
  {
    XERR << "[Table]," << m_name.c_str() << ",表头(" << (DWORD)fs.size() << ")与数据(" << (DWORD)datavec.size() << ")数量不一致" << XEND;
    return false;
  }
  Record record;
  for (DWORD n=0;n<fs.size();n++)
  {
    record.insert(std::make_pair(fs[n],datavec[n]));
  }
  m_records.push_back(record);
  return true;
}
