#pragma once
#include "xServer.h"

struct ProxyInfo
{
  xNetProcessor *m_pNetProcessor = nullptr;
  DWORD m_dwTaskNum = 0;
};

class GProxyServer : public xServer
{
  public:
    GProxyServer(OptArgs &args);
    virtual ~GProxyServer() {}

  public:
    virtual bool isZoneServer() { return false; }
    virtual bool isRegionServer() { return false; }
  public:
    virtual ServerType getServerType() { return SERVER_TYPE_G_PROXY; }

  protected:
    virtual bool v_callback();
    virtual void v_timetick();
    virtual void v_final();
    virtual void v_closeNp(xNetProcessor*);

  private:
    bool init();
    void process();
    void print();

  public:
    virtual bool doCmd(xNetProcessor* np, BYTE* buf, WORD len);

  private:
    std::map<std::string, ProxyInfo> m_oProxyList;
    xTimer m_oOneMinTimer;
};

extern GProxyServer *thisServer;
