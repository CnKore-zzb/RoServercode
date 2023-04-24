#pragma once
#include "xDefine.h"
#include "xTime.h"
#include "RegionServer.h"

class xNetProcessor;

class GZoneServer : public RegionServer
{
  public:
    GZoneServer(OptArgs &args);
    virtual ~GZoneServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_GZONE; }

  protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

  protected:
    bool loadConfig();

  protected:
    virtual bool doGZoneCmd(BYTE* buf, WORD len);

  private:
    xTimer m_oTenSecTimer;
    xTimer m_oOneMinTimer;
    xTimer m_oTenMinTimer;
    xTimer m_oFiveMinTimer;

  public:
    bool sendCmdToProxy(DWORD proxyid, const void *buf, WORD len);
};

extern GZoneServer *thisServer;
