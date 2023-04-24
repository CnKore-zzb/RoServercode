#pragma once
#include "xServer.h"

class RobotsServer : public xServer
{
  public:
    RobotsServer(OptArgs &args);
    virtual ~RobotsServer() {}

  public:
    virtual bool isZoneServer() { return false; }
    virtual bool isRegionServer() { return false; }
  public:
    virtual ServerType getServerType() { return SERVER_TYPE_ROBOTS; }

  protected:
    virtual bool v_callback();
    virtual void v_timetick();
    virtual void v_final();
    virtual void v_closeNp(xNetProcessor *);

  private:
    bool init();
    void process();

  public:
    virtual bool doCmd(xNetProcessor* np, BYTE* buf, WORD len);
};

extern RobotsServer *thisServer;
