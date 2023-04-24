#ifndef _SAN_SERVER_H_
#define _SAN_SERVER_H_
#include "ZoneServer.h"
#include "xNetProcessor.h"
#include "xDBConnPool.h"
#include "xSQLThread.h"

class RecordServer : public ZoneServer
{
  public:
    RecordServer(OptArgs &args);
    virtual ~RecordServer();

  public:
    virtual const char* getServerTypeString() { return "RecordServer"; }
    virtual ServerType getServerType() { return SERVER_TYPE_RECORD; }

  protected:
    virtual bool v_init();
    virtual bool v_stop();
    virtual void v_final();
    virtual void v_verifyOk(xNetProcessor *task);
    virtual void v_closeServer(xNetProcessor *np);
    virtual void v_timetick();

  private:
    bool loadConfig();

  protected:
    virtual bool doRegCmd(xNetProcessor* np, BYTE* buf, WORD len);
    virtual bool doGatewayCmd(const BYTE* buf, WORD len);
    virtual bool doRecordDataCmd(const BYTE* buf, WORD len);
    virtual bool doTradeCmd(const BYTE* buf, WORD len);
    virtual bool doGuildCmd(BYTE* buf, WORD len);
    // virtual bool doRecordTradeCmd(const BYTE* buf, WORD len);

  public:
    void patch();
  private:
    xTimer m_oTickOneHour;

  public:
    xSQLThread m_oRollbackThread;
};

extern RecordServer *thisServer;

#endif
