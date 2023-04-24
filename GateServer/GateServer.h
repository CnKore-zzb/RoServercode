#ifndef _SAN_SERVER_H_
#define _SAN_SERVER_H_
#include <map>
#include <set>
#include "ZoneServer.h"
#include "xFlowStatistics.h"
#include "xExecutionTime.h"

class GateUser;
class xNetProcessor;
class xTaskThread;
namespace Cmd
{
  class LoginData;
  struct UserCmd;
};

struct GMNetProcesser
{ 
  void updateTime(DWORD curSec);

  xNetProcessor* pNetProcess;
  DWORD expiretime = 0;
};

class GateServer : public ZoneServer
{
  friend class GateUserManager;
  public:
    GateServer(OptArgs &args);
    virtual ~GateServer();

  public:
    virtual const char* getServerTypeString() { return "GateServer"; }
    virtual ServerType getServerType() { return SERVER_TYPE_GATE; }

  protected:
    virtual bool v_init();
    virtual void v_final();
    virtual void v_timetick();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_closeServer(xNetProcessor *np);

  protected:
    virtual bool doLoginUserCmd(xNetProcessor* np, BYTE* buf, WORD len);
    virtual bool doRegCmd(xNetProcessor* np, BYTE* buf, WORD len);
    virtual bool doGatewayCmd(const BYTE* buf, WORD len);
    virtual bool doErrorUserCmd(const BYTE* buf, WORD len);
    virtual bool doGmToolsCmd(xNetProcessor *np, const BYTE* buf, WORD len);
    virtual bool doGateSuperCmd(xNetProcessor *np, BYTE* buf, WORD len);

  private:
    xTimer m_oOneSecTimer;
    xTimer m_oTenMinTimer;

  public:
    std::set<Cmd::UserCmd> m_oCmdFiler;

  public:
    virtual bool doCmd(xNetProcessor *np, BYTE *buf, WORD len);
  private:
    GMNetProcesser* getGMCon(QWORD id);
    GMNetProcesser* addGMCon(xNetProcessor* pNet);
    void delGMCon(QWORD id);
    void processGMOverTime(DWORD curTime);

    std::map <QWORD/*id*/, GMNetProcesser> m_mapGMCon;
    QWORD m_qwGMId = 0;
};

extern GateServer *thisServer;

#endif
