#pragma once

#include <sstream>
#include "xDefine.h"
#include "ProtoCommon.pb.h"
#include "SessionSociality.pb.h"

using std::string;
using std::stringstream;
using Cmd::EProfession;
using Cmd::EPackType;
using Cmd::ESource;
using Cmd::ESocialRelation;
using Cmd::EGender;

enum ELogType
{
  ELOGTYPE_MIN = 0,
  ELOGTYPE_TRC = 1,
  ELOGTYPE_DBG = 2,
  ELOGTYPE_INF = 3,
  ELOGTYPE_WRN = 4,
  ELOGTYPE_ERR = 5,
  ELOGTYPE_FTL = 6,
  ELOGTYPE_MAX = 7,
};

class xPos;

// xLog
class xLog
{
  friend xLog& operator<<(xLog& log, QWORD qwValue);
  friend xLog& operator<<(xLog& log, DWORD dwValue);
  friend xLog& operator<<(xLog& log, int iValue);
  friend xLog& operator<<(xLog& log, float fValue);
  friend xLog& operator<<(xLog& log, unsigned long value);
  friend xLog& operator<<(xLog& log, long long value);
  friend xLog& operator<<(xLog& log, time_t value);
  friend xLog& operator<<(xLog& log, const string& str);
  friend xLog& operator<<(xLog& log, const char* p);
  friend xLog& operator<<(xLog& log, EProfession eProfession);
  friend xLog& operator<<(xLog& log, EPackType eType);
  friend xLog& operator<<(xLog& log, ESource eSource);
  friend xLog& operator<<(xLog& log, ESocialRelation eRelation);
  friend xLog& operator<<(xLog& log, EGender eGender);
  friend xLog& operator<<(xLog& log, void* p);
  friend xLog& operator<<(xLog& log, const TVecItemInfo& vecItem);
  friend xLog& operator<<(xLog& log, const TVecItemData& vecData);
  friend xLog& operator<<(xLog& log, const TSetQWORD& setIDs);
  friend xLog& operator<<(xLog& log, const TSetDWORD& setIDs);
  friend xLog& operator<<(xLog& log, const TVecQWORD& vecIDs);
  friend xLog& operator<<(xLog& log, const TVecDWORD& vecIDs);
  friend xLog& operator<<(xLog& log, const xPos& pos);
  public:
    xLog() {}
    ~xLog() {}

    bool init(const string& servername, const string& logdir = "");
    bool reload();

    void setType(ELogType eType);

    void flush_t(ELogType eType, const string& str);
    stringstream& stream() { return m_stream; }
  private:
    void flush();
  private:
    string m_strServerName;
    string m_strLogDir;
    stringstream m_stream;
    ELogType m_eType = ELOGTYPE_INF;

    bool m_bInit = false;
};

// global
xLog& getLog(ELogType eType);
xLog& getTLog();
xLog& operator<<(xLog& log, QWORD qwValue);
xLog& operator<<(xLog& log, DWORD dwValue);
xLog& operator<<(xLog& log, int iValue);
xLog& operator<<(xLog& log, float fValue);
xLog& operator<<(xLog& log, unsigned long value);
xLog& operator<<(xLog& log, long long value);
xLog& operator<<(xLog& log, time_t value);
xLog& operator<<(xLog& log, const string& str);
xLog& operator<<(xLog& log, const char* p);
xLog& operator<<(xLog& log, EProfession eProfession);
xLog& operator<<(xLog& log, EPackType eType);
xLog& operator<<(xLog& log, ESource eSource);
xLog& operator<<(xLog& log, EGender eGender);
xLog& operator<<(xLog& log, void* p);
xLog& operator<<(xLog& log, const TVecItemInfo& vecItem);
xLog& operator<<(xLog& log, const TVecItemData& vecData);
xLog& operator<<(xLog& log, const TSetQWORD& setIDs);
xLog& operator<<(xLog& log, const TSetDWORD& setIDs);
xLog& operator<<(xLog& log, const TVecQWORD& vecIDs);
xLog& operator<<(xLog& log, const TVecDWORD& vecIDs);
xLog& operator<<(xLog& log, const xPos& pos);

#define XTRC getLog(ELOGTYPE_TRC)
#define XINF getLog(ELOGTYPE_INF)
#define XWRN getLog(ELOGTYPE_WRN)
#define XERR getLog(ELOGTYPE_ERR)
#define XFTL getLog(ELOGTYPE_FTL)
#define XEND "end"
#define XLOG XINF

#ifdef _DEBUG
#define XDBG getLog(ELOGTYPE_DBG)
#else
#define XDBG while (false) getLog(ELOGTYPE_DBG)
#endif

#define XTRC_T(...) getTLog().flush_t(ELOGTYPE_TRC, formatArgs(__VA_ARGS__))
#define XDBG_T(...) getTLog().flush_t(ELOGTYPE_DBG, formatArgs(__VA_ARGS__))
#define XINF_T(...) getTLog().flush_t(ELOGTYPE_INF, formatArgs(__VA_ARGS__))
#define XWRN_T(...) getTLog().flush_t(ELOGTYPE_WRN, formatArgs(__VA_ARGS__))
#define XERR_T(...) getTLog().flush_t(ELOGTYPE_ERR, formatArgs(__VA_ARGS__))
#define XFTL_T(...) getTLog().flush_t(ELOGTYPE_FTL, formatArgs(__VA_ARGS__))
#define XLOG_T XINF_T

