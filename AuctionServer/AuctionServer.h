#pragma once
#include <set>
#include "xDefine.h"
#include "RegionServer.h"
#include "MsgManager.h"

class xNetProcessor;

class AuctionServer : public RegionServer
{
  public:
    AuctionServer(OptArgs &args);
    virtual ~AuctionServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_AUCTION; }

public:
  bool sendCmdToClient(QWORD charId, DWORD zoneId, void* buf, WORD len);
  //通过session发送协议到sceneserver
  bool forwardCmdToSceneServer(QWORD charId, DWORD zoneId, void* buf, WORD len);
  //通过session发送协议到tradeserver
  bool forwardCmdToTradeServer(DWORD zoneId, void* buf, WORD len);
  void sendMsg(QWORD charId, DWORD zoneId, DWORD msgid, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD);
  void mutableMsg(SysMsg &cmd, DWORD msgid, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD);

protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

  protected:
    bool loadConfig();

  protected:
    virtual bool doAuctionCmd(BYTE* buf, WORD len);
    virtual bool doSocialCmd(BYTE* buf, WORD len);
    virtual bool doSessionCmd(BYTE* buf, WORD len);
    virtual bool doTradeCmd(BYTE* buf, WORD len);

};

extern AuctionServer *thisServer;
