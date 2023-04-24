#pragma once
#include <set>
#include "xDefine.h"
#include "xTime.h"
#include "RegionServer.h"
#include "MsgManager.h"
#include "GuildSCmd.pb.h"

class xNetProcessor;
namespace Cmd
{
  class DeadBossInfo;
};

class GlobalServer : public RegionServer
{
  public:
    GlobalServer(OptArgs &args);
    virtual ~GlobalServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_GLOBAL; }

  protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

  protected:
    bool loadConfig();

  protected:
    virtual bool doSocialCmd(BYTE* buf, WORD len);
    virtual bool doWeddingCmd(BYTE* buf, WORD len);
    virtual bool doBossSCmd(BYTE* buf, WORD len);
    //virtual bool doTeamCmd(BYTE* buf, WORD len);
    //virtual bool doGuildCmd(BYTE* buf, WORD len);

  private:
    void updateGlobalBoss(const Cmd::DeadBossInfo& rInfo);
  private:
    xTimer m_oOneSecTimer;
    xTimer m_oTenSecTimer;

  public:
    bool sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len);
    bool sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params = MsgParams());
    bool sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params = MsgParams());
    bool sendWorldMsg(DWORD msgid, MsgParams params = MsgParams());
};

extern GlobalServer *thisServer;
