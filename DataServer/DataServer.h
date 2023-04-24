#pragma once

#include "ZoneServer.h"

class DataServer : public ZoneServer
{
  public:
    DataServer(OptArgs &args);
    virtual ~DataServer();

  public:
    virtual const char* getServerTypeString() { return "DataServer"; }
    virtual ServerType getServerType() { return SERVER_TYPE_DATA; }

  protected:
    virtual bool v_init();
    virtual void v_final();
    virtual void v_timetick();
    virtual bool v_stop();

    virtual void v_closeServer(xNetProcessor *np);
    virtual void v_verifyOk(xNetProcessor *np);

  private:
    void initOkServer(ServerTask *np);

  public:
    bool doRecordDataCmd(const BYTE* buf, WORD len);
    bool doGuildCmd(BYTE* buf, WORD len);
};

extern DataServer *thisServer;
