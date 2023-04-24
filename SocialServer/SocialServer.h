#pragma once
//#include <set>
#include "xDefine.h"
//#include "xTime.h"
#include "ZoneServer.h"
#include "MsgManager.h"

class xNetProcessor;

class SocialServer : public ZoneServer
{
  public:
    SocialServer(OptArgs &args);
    virtual ~SocialServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_SOCIAL; }
    virtual const char* getServerTypeString() { return "SocialServer"; }

  protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

    virtual void v_closeServer(xNetProcessor *np);
    virtual void v_verifyOk(xNetProcessor *task);
    virtual void init_ok();
  protected:
    bool loadConfig();

  protected:
    virtual bool doSocialCmd(const BYTE* buf, WORD len);
    virtual bool doTeamCmd(BYTE* buf, WORD len);
    virtual bool doGuildCmd(BYTE* buf, WORD len);
    virtual bool doSessionCmd(const BYTE* buf, WORD len);
    virtual bool doMatchCmd(const BYTE* buf, WORD len);

  private:
    xTimer m_oOneSecTimer;
    xTimer m_oTenSecTimer;

  public:
    bool sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params = MsgParams());
    bool sendDebugMsg(DWORD zoneid, QWORD charid, const string& str);
    bool sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params = MsgParams());

  public:
    void gcharPatch();
};

extern SocialServer *thisServer;
