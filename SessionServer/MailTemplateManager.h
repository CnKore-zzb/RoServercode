#pragma once

#include "xSingleton.h"
#include "CommonConfig.h"
#include "SessionMail.pb.h"

using std::map;
using std::string;

class MailTemplateManager : public xSingleton<MailTemplateManager>
{
public:
  MailTemplateManager() {}
  virtual ~MailTemplateManager() {}

  const static string MAIL_TEMPLATE_MARK;

  void clear() { m_mapTemplate.clear(); }
  bool load(const TSetDWORD& ids);
  bool getMsg(DWORD id, ELanguageType type, string* title, string* msg);
  DWORD getTemplateID(const string& msg);

private:
  map<DWORD, map<ELanguageType, MailMsg>> m_mapTemplate;
};
