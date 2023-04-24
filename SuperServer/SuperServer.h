#ifndef _SUPER_SERVER_H_
#define _SUPER_SERVER_H_
#include "ZoneServer.h"
#include "SuperCmd.h"
#include "GateSuper.pb.h"

class SuperServer : public ZoneServer
{
  friend class GateInfoM;
  public:
    SuperServer(OptArgs &args);
    virtual ~SuperServer();

  public:
    virtual const char* getServerTypeString() { return "SuperServer"; }
    virtual ServerType getServerType() { return SERVER_TYPE_SUPER; }

  protected:
    virtual bool v_init();
    virtual void v_final();
    virtual void v_timetick();
    virtual bool v_stop();

    virtual void v_closeServer(xNetProcessor *np);
    virtual void v_verifyOk(xNetProcessor *np);

  private:
    void initOkServer(ServerTask *np);

  protected:
    virtual bool doGateSuperCmd(xNetProcessor* np, BYTE* buf, WORD len);

  protected:
    bool m_blZoneReady = false;

  private:
    void monitor();
    xTimer m_oMonitorTimer;
    void alter_msg(const string& title, const string& message, const EPushMsg& type);
    void push_tyrantdb(const PushTyrantDbGateSuperCmd& rev);
};

extern SuperServer *thisServer;

#endif
