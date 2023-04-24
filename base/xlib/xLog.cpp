#include "xLog.h"
#include "LuaManager.h"
#include "xPos.h"
//#include "xTools.h"

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/dailyrollingfileappender.h>

xLog srvLog;
using namespace log4cxx;

const char* getPackName(EPackType eType)
{
  if (eType == EPACKTYPE_MAIN)
    return "背包";
  else if (eType == EPACKTYPE_EQUIP)
    return "装备栏";
  else if (eType == EPACKTYPE_FASHION)
    return "时装";
  else if (eType == EPACKTYPE_FASHIONEQUIP)
    return "时装栏";
  else if (eType == EPACKTYPE_CARD)
    return "卡册";
  else if (eType == EPACKTYPE_STORE)
    return "通用仓库";
  else if (eType == EPACKTYPE_PERSONAL_STORE)
    return "个人仓库";
  else if (eType == EPACKTYPE_TEMP_MAIN)
    return "临时背包";
  else if (eType == EPACKTYPE_BARROW)
    return "手推车";
  else if (eType == EPACKTYPE_QUEST)
    return "任务";
  else if (eType == EPACKTYPE_FOOD)
    return "料理包";
  else if (eType == EPACKTYPE_PET)
    return "宠物包";

  return "无";
}

// global
xLog& getLog(ELogType eType)
{
  srvLog.setType(eType);
  return srvLog;
}

xLog& getTLog()
{
  return srvLog;
}

xLog& operator<<(xLog& log, QWORD qwValue)
{
  log.m_stream << qwValue << " ";
  return log;
}

xLog& operator<<(xLog& log, DWORD dwValue)
{
  log.m_stream << dwValue << " ";
  return log;
}

xLog& operator<<(xLog& log, int iValue)
{
  log.m_stream << iValue << " ";
  return log;
}

xLog& operator<<(xLog& log, float fValue)
{
  log.m_stream << fValue << " ";
  return log;
}

xLog& operator<<(xLog& log, unsigned long value)
{
  log.m_stream << value << " ";
  return log;
}

xLog& operator<<(xLog& log, long long value)
{
  log.m_stream << value << " ";
  return log;
}

xLog& operator<<(xLog& log, time_t value)
{
  log.m_stream << value << " ";
  return log;
}

xLog& operator<<(xLog& log, string const& str)
{
  if (str == "end")
    log.flush();
  else
    log.m_stream << str << " ";
  return log;
}

xLog& operator<<(xLog& log, const char* p)
{
  if (p == nullptr)
    return log;
  if (strcmp(p, "end") == 0)
    log.flush();
  else
    log.m_stream << p << " ";
  return log;
}

xLog& operator<<(xLog& log, EProfession eProfession)
{
  if (LuaManager::getMe().getLuaState() == nullptr)
    return log;
  const char* p = LuaManager::getMe().call<const char*>("getProName", eProfession);
  log.m_stream << (p == nullptr ? "" : p) << "(" << static_cast<DWORD>(eProfession) << ")" << " ";
  return log;
}

xLog& operator<<(xLog& log, EPackType eType)
{
  const char* p = getPackName(eType);
  log.m_stream << (p == nullptr ? "" : p) << "(" << static_cast<DWORD>(eType) << ")" << " ";
  return log;
}

xLog& operator<<(xLog& log, ESource eSource)
{
  if (LuaManager::getMe().getLuaState() == nullptr)
    return log;
  const char* p = LuaManager::getMe().call<const char*>("getSourceName", eSource);
  log.m_stream << (p == nullptr ? "" : p) << "(" << static_cast<DWORD>(eSource) << ")" << " ";
  return log;
}

xLog& operator<<(xLog& log, EGender eGender)
{
  if (LuaManager::getMe().getLuaState() == nullptr)
    return log;
  const char* p = LuaManager::getMe().call<const char*>("getGenderName", eGender);
  log.m_stream << (p == nullptr ? "" : p) << "(" << static_cast<DWORD>(eGender) << ")" << " ";
  return log;
}

xLog& operator<<(xLog& log, ESocialRelation eRelation)
{
  switch (eRelation)
  {
    case ESOCIALRELATION_MIN:
      log.m_stream << "min(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_FRIEND:
      log.m_stream << "好友(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_MERRY:
      log.m_stream << "婚姻(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_CHAT:
      log.m_stream << "聊天(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_TEAM:
      log.m_stream << "最近队友(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_APPLY:
      log.m_stream << "好友申请(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_BLACK:
      log.m_stream << "黑名单(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_BLACK_FOREVER:
      log.m_stream << "永久黑名单(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_TUTOR:
      log.m_stream << "导师(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_TUTOR_APPLY:
      log.m_stream << "导师申请(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_STUDENT:
      log.m_stream << "学生(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_STUDENT_APPLY:
      log.m_stream << "学生申请(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_STUDENT_RECENT:
      log.m_stream << "最近学生(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_TUTOR_PUNISH:
      break;
    case ESOCIALRELATION_TUTOR_CLASSMATE:
      log.m_stream << "同学(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_RECALL:
      log.m_stream << "召回(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_BERECALL:
      log.m_stream << "被召回(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
    case ESOCIALRELATION_MAX:
      log.m_stream << "max(" << static_cast<DWORD>(eRelation) << ")" << " ";
      break;
  }
  return log;
}

xLog& operator<<(xLog& log, void* p)
{
  if (p == nullptr)
    return log;
  log.m_stream << p << " ";
  return log;
}

xLog& operator<<(xLog& log, const TVecItemInfo& vecItem)
{
  for (auto &v : vecItem)
    log.m_stream << v.ShortDebugString() << " ";
  return log;
}

xLog& operator<<(xLog& log, const TVecItemData& vecData)
{
  for (auto &v : vecData)
    log.m_stream << v.ShortDebugString() << " ";
  return log;
}

xLog& operator<<(xLog& log, const TSetQWORD& setIDs)
{
  for (auto &s : setIDs)
    log.m_stream << s << " ";
  return log;
}

xLog& operator<<(xLog& log, const TSetDWORD& setIDs)
{
  for (auto &s : setIDs)
    log.m_stream << s << " ";
  return log;
}

xLog& operator<<(xLog& log, const TVecQWORD& vecIDs)
{
  for (auto &v : vecIDs)
    log.m_stream << v << " ";
  return log;
}

xLog& operator<<(xLog& log, const TVecDWORD& vecIDs)
{
  for (auto &v : vecIDs)
    log.m_stream << v << " ";
  return log;
}

xLog& operator<<(xLog& log, const xPos& pos)
{
  log.m_stream << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
  return log;
}

// xlog
bool xLog::init(const string& servername, const string& dir /*= ""*/)
{
  m_strServerName = servername;
  m_strLogDir = dir;

  char servername_env[256] = {0};
  snprintf(servername_env, 256, "servername=%s%s", m_strLogDir.empty() == true ? "log/" : m_strLogDir.c_str(), m_strServerName.c_str());
  if (putenv(servername_env) != 0)
    return false;

#ifdef _DEBUG
  PropertyConfigurator::configure("logconfig_debug.properties");
#else
  PropertyConfigurator::configure("logconfig_release.properties");
#endif
  setlocale(LC_ALL, "en_US.UTF-8");

  m_bInit = true;
  return true;
}

bool xLog::reload()
{
  char servername_env[256] = {0};
  snprintf(servername_env, 256, "servername=%s%s", m_strLogDir.empty() == true ? "log/" : m_strLogDir.c_str(), m_strServerName.c_str());
  if (putenv(servername_env) != 0)
    return false;

#ifdef _DEBUG
  PropertyConfigurator::configure("logconfig_debug.properties");
#else
  PropertyConfigurator::configure("logconfig_release.properties");
#endif
  setlocale(LC_ALL, "en_US.UTF-8");
  return true;
}

void xLog::setType(ELogType eType)
{
  if (eType <= ELOGTYPE_MIN || eType >= ELOGTYPE_MAX)
    return;

  if (m_eType != eType)
    flush();

  m_eType = eType;
}

void xLog::flush()
{
  if (!m_bInit)
    return;
  if (m_eType <= ELOGTYPE_MIN || m_eType >= ELOGTYPE_MAX)
    return;

  switch(m_eType)
  {
    case ELOGTYPE_MIN:
      break;
    case ELOGTYPE_TRC:
      LOG4CXX_TRACE(Logger::getRootLogger(), LOG4CXX_STR(m_stream.str().c_str()));
      break;
    case ELOGTYPE_DBG:
      LOG4CXX_DEBUG(Logger::getRootLogger(), LOG4CXX_STR(m_stream.str().c_str()));
      break;
    case ELOGTYPE_INF:
      LOG4CXX_INFO(Logger::getRootLogger(), LOG4CXX_STR(m_stream.str().c_str()));
      break;
    case ELOGTYPE_WRN:
      LOG4CXX_WARN(Logger::getRootLogger(), LOG4CXX_STR(m_stream.str().c_str()));
      break;
    case ELOGTYPE_ERR:
      LOG4CXX_ERROR(Logger::getRootLogger(), LOG4CXX_STR(m_stream.str().c_str()));
      break;
    case ELOGTYPE_FTL:
      LOG4CXX_FATAL(Logger::getRootLogger(), LOG4CXX_STR(m_stream.str().c_str()));
      break;
    case ELOGTYPE_MAX:
      break;
  }

  m_eType = ELOGTYPE_MIN;
  m_stream.clear();
  m_stream.str("");
}

void xLog::flush_t(ELogType eType, const string& str)
{
  switch(eType)
  {
    case ELOGTYPE_MIN:
      break;
    case ELOGTYPE_TRC:
      LOG4CXX_TRACE(Logger::getRootLogger(), LOG4CXX_STR(str.c_str()));
      break;
    case ELOGTYPE_DBG:
      LOG4CXX_DEBUG(Logger::getRootLogger(), LOG4CXX_STR(str.c_str()));
      break;
    case ELOGTYPE_INF:
      LOG4CXX_INFO(Logger::getRootLogger(), LOG4CXX_STR(str.c_str()));
      break;
    case ELOGTYPE_WRN:
      LOG4CXX_WARN(Logger::getRootLogger(), LOG4CXX_STR(str.c_str()));
      break;
    case ELOGTYPE_ERR:
      LOG4CXX_ERROR(Logger::getRootLogger(), LOG4CXX_STR(str.c_str()));
      break;
    case ELOGTYPE_FTL:
      LOG4CXX_FATAL(Logger::getRootLogger(), LOG4CXX_STR(str.c_str()));
      break;
    case ELOGTYPE_MAX:
      break;
  }
}

