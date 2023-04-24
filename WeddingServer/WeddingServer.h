#pragma once
#include <set>
#include "xDefine.h"
#include "RegionServer.h"
#include "MsgManager.h"

class xNetProcessor;

class WeddingServer : public RegionServer
{
  public:
    WeddingServer(OptArgs &args);
    virtual ~WeddingServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_WEDDING; }
    virtual void onSessionRegist(xNetProcessor *np, DWORD dwZoneId);
public:
  bool sendCmdToClient(QWORD charId, DWORD zoneId, void* buf, WORD len);
  //通过session发送协议到sceneserver
  bool forwardCmdToSceneServer(QWORD charId, DWORD zoneId, void* buf, WORD len);
  void sendMsg(QWORD charId, DWORD zoneId, DWORD msgid, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD);
  void mutableMsg(SysMsg &cmd, DWORD msgid, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD);
  bool sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params = MsgParams());

  //只是通知玩家，邮件到了
  virtual bool sendMail(Cmd::SendMail& cmd);
  void sendWeddingMsg(QWORD qwCharId, DWORD dwZoneId, const Cmd::WeddingEventMsgCCmd& msg);
  std::string getZoneName(DWORD dwZoneId);
private:
  std::map<DWORD, string> m_mapZoneNameCache;

protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

  protected:
    bool loadConfig();

  protected:
    virtual bool doWeddingCmd(BYTE* buf, WORD len);
    virtual bool doSocialCmd(BYTE* buf, WORD len);
    virtual bool doSessionCmd(BYTE* buf, WORD len);
    virtual bool doRegCmd(xNetProcessor *np, BYTE* buf, WORD len);
    std::set<DWORD> m_setStartedSession;
};

extern WeddingServer *thisServer;
