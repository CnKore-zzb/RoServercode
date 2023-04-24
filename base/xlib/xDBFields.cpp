#include "xlib/xDBFields.h"

xField::xField(const string& database, const char *table)
{
  clear();

  m_strDatabase = database;
  m_strTable = table;
}

xField::~xField()
{
}
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
xRecord::xRecord(xField *field)
{
  m_field = field;
}

xRecord::~xRecord()
{
  m_rs.clear();
}
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/

xFieldsM::~xFieldsM()
{
  final();
}

void xFieldsM::final()
{
    for (auto it : m_list)
    {
      for (auto iter : it.second)
      {
        SAFE_DELETE(iter.second);
      }
      it.second.clear();
    }
    m_list.clear();
}

bool xFieldsM::addField(xField *field)
{
  if (!field) return false;

  auto it = m_list[field->m_strDatabase].find(field->m_strTable);
  if (it != m_list[field->m_strDatabase].end())
  {
    SAFE_DELETE(it->second);
  }
  m_list[field->m_strDatabase][field->m_strTable] = field;
  return true;
}

xField* xFieldsM::getField(const std::string& database, const std::string& table)
{
  auto it = m_list.find(database);
  if (it==m_list.end())
  {
    return nullptr;
  }

  auto iter = it->second.find(table);
  if (iter==it->second.end())
  {
    return nullptr;
  }

  return iter->second;
}
