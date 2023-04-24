#include "MailTemplateManager.h"
#include "SessionServer.h"
#include "SessionSociality.pb.h"

const string MailTemplateManager::MAIL_TEMPLATE_MARK = "__template__:";

bool MailTemplateManager::load(const TSetDWORD& ids)
{
  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "mail_template");
  if (field == nullptr)
  {
    XERR << "[邮件模板-加载]" << "加载失败,未找到mail_template数据库表" << XEND;
    return false;
  }

  stringstream sstr;
  sstr << "id in (";
  for (auto it = ids.begin(); it != ids.end(); ++it)
  {
    if (it != ids.begin())
      sstr << ",";
    sstr << *it;
  }
  sstr << ")";

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, sstr.str().c_str());
  if (ret == QWORD_MAX)
  {
    XERR << "[邮件模板-加载]" << "加载失败,查询失败ret:" << ret << XEND;
    return false;
  }

  for (QWORD i = 0; i < ret; ++i)
  {
    string datastr;
    datastr.assign((const char*)set[i].getBin("data"), set[i].getBinSize("data"));
    MailTemplateData data;
    if (data.ParseFromString(datastr) == false)
    {
      XERR << "[邮件模板-加载]" << "解析data失败,id:" << set[i].get<DWORD>("id") << XEND;
      continue;
    }
    auto& t = m_mapTemplate[set[i].get<DWORD>("id")];
    for (int j = 0; j < data.msgs_size(); ++j)
      t[static_cast<ELanguageType>(data.msgs(j).language())] = data.msgs(j);
  }

  return true;
}

bool MailTemplateManager::getMsg(DWORD id, ELanguageType type, string* title, string* msg)
{
  if (title == nullptr || msg == nullptr)
    return false;
  auto it = m_mapTemplate.find(id);
  if (it == m_mapTemplate.end())
    return false;
  if (it->second.empty())
  {
    *title = STRING_EMPTY;
    *msg = STRING_EMPTY;
    XERR << "[邮件模板-获取内容]" << id << type << "模板数据为空" << XEND;
    return true;
  }
  auto m = it->second.find(type);
  if (m == it->second.end())
  {
    auto v = it->second.find(CommonConfig::m_eDefaultLanguage);
    if (v == it->second.end())
    {
      *title = it->second.begin()->second.title();
      *msg = it->second.begin()->second.msg();
      return true;
    }
    *title = v->second.title();
    *msg = v->second.msg();
    return true;
  }
  *title = m->second.title();
  *msg = m->second.msg();
  return true;
}

DWORD MailTemplateManager::getTemplateID(const string& msg)
{
  if (msg.length() <= MAIL_TEMPLATE_MARK.length() || msg.compare(0, MAIL_TEMPLATE_MARK.length(), MAIL_TEMPLATE_MARK) != 0)
    return 0;
  return atoi(msg.substr(MAIL_TEMPLATE_MARK.length()).c_str());
}
