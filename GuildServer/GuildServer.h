#pragma once
#include <set>
#include "xDefine.h"
#include "xTime.h"
#include "RegionServer.h"
#include "MsgManager.h"
#include "xSQLThread.h"
#include "SocialCmd.pb.h"

class xNetProcessor;

class GuildServer : public RegionServer
{
  public:
    GuildServer(OptArgs &args);
    virtual ~GuildServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_GUILD; }

  protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

  protected:
    bool loadConfig();

  public:
    xSQLThread m_oSQLThread;

  public:
    virtual bool doSocialCmd(BYTE* buf, WORD len);

  protected:
    virtual bool doGuildCmd(BYTE* buf, WORD len);
    bool doChatCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
    virtual bool doSessionCmd(BYTE* buf, WORD len);
  protected:
    xTimer m_oOneSecTimer;
    xTimer m_oTenSecTimer;
    xTimer m_oOneMinTimer;

  public:
    bool sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params = MsgParams());
    bool sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params = MsgParams());
};

extern GuildServer *thisServer;
