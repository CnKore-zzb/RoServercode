#pragma once
#include <set>
#include "xDefine.h"
#include "xTime.h"
#include "ZoneServer.h"
#include "MsgManager.h"
#include "SocialCmd.pb.h"

class xNetProcessor;

class TeamServer : public ZoneServer
{
  public:
    TeamServer(OptArgs &args);
    virtual ~TeamServer();

  public:
    virtual const char* getServerTypeString() { return "TeamServer"; }
    virtual ServerType getServerType() { return SERVER_TYPE_TEAM; }

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
    virtual bool doMatchCmd(const BYTE* buf, WORD len);
    bool doChatCmd(const SocialUser& rUser, const BYTE* buf, WORD len);

  private:
    xTimer m_oOneSecTimer;
    xTimer m_oTenSecTimer;

  public:
    virtual bool sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    virtual void broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE t, QWORD index, const void *cmd, WORD len, QWORD excludeID=0, DWORD ip=0);
    bool sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool forwardCmdToMatch(const void *buf, WORD len);
    bool sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params = MsgParams());
    bool sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params = MsgParams());
};

extern TeamServer *thisServer;
