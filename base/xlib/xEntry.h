#ifndef BASE_XLIB_XENTRY_H_
#define BASE_XLIB_XENTRY_H_
#include "xlib/xDefine.h"
#include "xlib/xTools.h"
#include "xlib/xNoncopyable.h"
#include "xPool.h"

struct xEntryC
{
  public:
    xEntryC()
    {
      id = 0;
      bzero(name, sizeof(name));
      tempid = 0;
    }
    virtual ~xEntryC() {}

  public:
    void set_id(UINT i) { id = i; }
    void set_name(const char *n)
    {
      if (NULL == n) return;
      bzero(name, sizeof(name));
      strncpy(name, n, MAX_NAMESIZE);
    }
    void set_tempid(UINT t) { tempid = t; }
    virtual UINT getTempID() const { return tempid; }

  public:
    QWORD id;
    char name[MAX_NAMESIZE];
    QWORD tempid;
};

enum LogType
{
  LOG_TRC,
  LOG_DBG,
  LOG_INF,
  LOG_WRN,
  LOG_ERR,
  LOG_FTL,
};

struct xEntry : public xEntryC, private xNoncopyable
{
  xEntry()
  {
  }
  virtual ~xEntry()
  {
  }
  /*void log(const char* title, LogType type, const char *fmt, ...)
  {
    if (!title) return;
    char buffer[LEN_512];
    bzero(buffer, sizeof(buffer));
    parseFormat(buffer, fmt);
    std::string text(buffer, strlen(buffer)+1);
    switch (type)
    {
      case LOG_TRC:
        {
          XTRC << "[" << title << "]" << id << tempid << name << text << XEND;
          break;
        }
        break;
      case LOG_DBG:
        {
          XDBG << "[" << title << "]" << id << tempid << name << text << XEND;
          break;
        }
        break;
      case LOG_INF:
        {
          XINF << "[" << title << "],%llu,%llu,%s,%s", title, id, tempid, name, text.c_str());
          break;
        }
        break;
      case LOG_WRN:
        {
          XWRN("[%s],%llu,%llu,%s,%s", title, id, tempid, name, text.c_str());
          break;
        }
        break;
      case LOG_ERR:
        {
          XERR("[%s],%llu,%llu,%s,%s", title, id, tempid, name, text.c_str());
          break;
        }
        break;
      case LOG_FTL:
        {
          XFTL("[%s],%llu,%llu,%s,%s", title, id, tempid, name, text.c_str());
          break;
        }
        break;
      default:
        {
          //XLOG("[%s],%llu,%llu,%s,%s", title, id, tempid, name, text.c_str());
          XLOG << "[" << title << "]," << id << "," << tempid << "," << name << "," << text.c_str() << XEND;
        }
        break;
    }
  }*/
};
#endif  // BASE_XLIB_XENTRY_H_
