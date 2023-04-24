#pragma once
#include <set>
#include "xDefine.h"
#include "RegionServer.h"

class xNetProcessor;

class TradeServer : public RegionServer
{
  public:
    TradeServer(OptArgs &args);
    virtual ~TradeServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_TRADE; }

  protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

  protected:
    bool loadConfig();

  protected:
    virtual bool doTradeCmd(BYTE* buf, WORD len);
    virtual bool doSocialCmd(BYTE* buf, WORD len);
    virtual bool doRegCmd(xNetProcessor *np, BYTE* buf, WORD len);
};

extern TradeServer *thisServer;
