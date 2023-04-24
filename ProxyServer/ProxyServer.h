#pragma once
#include "xServer.h"

#define PROXY_SERVER_TCP_TASK_LIMIT 2000

class ProxyServer : public xServer
{
  public:
    ProxyServer(OptArgs &args);
    virtual ~ProxyServer() {}

  public:
    virtual bool isZoneServer() { return false; }
    virtual bool isRegionServer() { return false; }
  public:
    virtual ServerType getServerType() { return SERVER_TYPE_PROXY; }

  protected:
    virtual bool v_callback();
    virtual void v_timetick();
    virtual void v_final();
    virtual void v_closeNp(xNetProcessor*);

  private:
    bool init();
    void process();

  public:
    virtual bool doCmd(xNetProcessor* np, BYTE* buf, WORD len);
    virtual bool doLoginUserCmd(xNetProcessor* np, BYTE* buf, WORD len);
  private:
    bool parseLoginUserCmd(xNetProcessor* np, xCommand* buf, WORD len);

  public:
    std::string& getProxyID() { return s_oOptArgs.m_strProxyID; }
  private:
    std::string m_strGProxyIP;
    DWORD m_dwGProxyPort = 0;
};

extern ProxyServer *thisServer;
