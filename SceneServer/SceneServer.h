#pragma once
#include <list>
#include "ZoneServer.h"
#include "xExecutionTime.h"
#include "xNetProcessor.h"
//#include "Define.h"
#include "ErrorUserCmd.pb.h"
#include "MsgManager.h"

struct GatewayCmd;
class SceneUser;
class xNetProcessor;

class SceneServer : public ZoneServer
{
  public:
    SceneServer(OptArgs &args);
    ~SceneServer();

  public:
    virtual const char* getServerTypeString() { return "SceneServer"; }
    virtual ServerType getServerType() { return SERVER_TYPE_SCENE; }

  protected:
    virtual bool v_init();
    virtual void v_final();
    virtual void v_timetick();
    virtual void v_closeServer(xNetProcessor *np);
    virtual void v_verifyOk(xNetProcessor *task);
    virtual void init_ok();

  public:
    bool loadConfig();
  private:
    void registerLuaFunc();

  public:
    virtual bool doRecordDataCmd(const BYTE* buf, WORD len);
    virtual bool doTradeCmd(const BYTE* buf, WORD len);
    virtual bool doMatchCmd(const BYTE* buf, WORD len);
    virtual bool doAuctionCmd(const BYTE* buf, WORD len);
    virtual bool doGatewayCmd(const BYTE* buf, WORD len);
    //virtual bool doSessionSceneCmd(const BYTE* buf, WORD len); 
    virtual bool doSessionCmd(const BYTE* buf, WORD len);
    virtual bool doRegCmd(xNetProcessor* np, BYTE* buf, WORD len);
    virtual bool doGmToolsCmd(xNetProcessor* np, const BYTE* buf, WORD len);
    virtual bool doSocialCmd(const BYTE* buf, WORD len);
    virtual bool doTeamCmd(BYTE* buf, WORD len);
    virtual bool doGuildCmd(BYTE* buf, WORD len);
    virtual bool doWeddingCmd(const BYTE* buf, WORD len);
    //virtual bool doItemSCmd(const BYTE* buf, WORD len);
    virtual bool doBossSCmd(const BYTE* buf, WORD len);

  public:
    virtual bool sendCmdToMe(QWORD charid, const void* data, unsigned short len);
    bool forwardCmdToSceneUser(QWORD charid, const void* data, unsigned short len);
    bool forwardCmdToSessionUser(QWORD charid, const void* data, unsigned short len);
    bool forwardCmdToSessionTrade(QWORD charid, const std::string& name, const void* data, unsigned short len);
    bool forwardCmdToUserScene(QWORD charid, const void* data, unsigned short len);
    bool forwardCmdToAuction(QWORD charid, const std::string& name, const void* data, unsigned short len);
    bool forwardCmdToWorldUser(QWORD charid, const void *buf, WORD len);
    bool sendSysMsgToWorldUser(QWORD qwCharID, DWORD dwMsgID, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD, DWORD dwDelay = 0);
    bool sendSCmdToWeddingServer(QWORD charid, const std::string& name, const void* data, unsigned short len);
    bool sendUserCmdToWeddingServer(QWORD charid, const std::string& name,  const void* buf, WORD len);

    virtual void broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE t, QWORD index, const void *cmd, WORD len, QWORD excludeID=0, DWORD ip=0);

  public:
    void logErr(SceneUser* user,QWORD uid,const char* name,Cmd::RegErrRet ret,bool delFromMgr);
    void loginErr(QWORD accid, DWORD zoneID, Cmd::RegErrRet ret, xNetProcessor* net);

  private:
    xTimer m_oOneSecTimer;
    xTimer m_oTimer;
    xTimer m_oTickOneMin;
    xDayTimer m_oDayTimer;

  public:
    void test();
};

extern SceneServer *thisServer;
