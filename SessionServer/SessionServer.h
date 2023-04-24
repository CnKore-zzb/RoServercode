#pragma once
#include "ZoneServer.h"
#include "xDBConnPool.h"
//#include "Define.h"
#include "ErrorUserCmd.pb.h"
#include "ConfigManager.h"
#include "BaseConfig.h"

class xNetProcessor;
class SessionUser;
class SessionScene;
struct LoginOutRegCmd;
class SessionThreadData;

class SessionServer : public ZoneServer
{
  public:
    SessionServer(OptArgs &args);
    virtual ~SessionServer();

  public:
    virtual const char* getServerTypeString() { return "SessionServer"; }
    virtual ServerType getServerType() { return SERVER_TYPE_SESSION; }

  protected:
    virtual bool v_init();
    virtual void v_final();
    virtual bool v_stop();
    virtual void v_timetick();
    virtual void v_closeServer(xNetProcessor *np);
    virtual void v_verifyOk(xNetProcessor *task);
    virtual void init_ok();
    virtual void initOkServer(ServerTask *task);
    virtual void onRegistRegion(ClientType type);

  private:
    bool loadConfig();
    void registerLuaFunc();

  public:
    virtual bool doRegCmd(xNetProcessor *np, BYTE* buf, WORD len);
    //virtual bool doSessionSceneCmd(const BYTE* buf, WORD len);
    virtual bool doErrorUserCmd(const BYTE* buf, WORD len);
    virtual bool doSessionCmd(const BYTE* buf, WORD len);
    virtual bool doTradeCmd(const BYTE* buf, WORD len);
    virtual bool doMatchCmd(const BYTE* buf, WORD len);
    virtual bool doAuctionCmd(const BYTE* buf, WORD len);
    virtual bool doSocialCmd(const BYTE* buf, WORD len);
    virtual bool doGmToolsCmd(xNetProcessor* np, const BYTE* buf, WORD len);
    virtual bool doGatewayCmd(const BYTE* buf, WORD len);
    virtual bool doLogCmd(const BYTE* buf, WORD len);
    virtual bool doClientCmd(const BYTE* buf, WORD len);
    virtual bool doTeamCmd(BYTE* buf, WORD len);
    virtual bool doGuildCmd(BYTE* buf, WORD len);
    virtual bool doWeddingCmd(const BYTE* buf, WORD len);
    virtual bool doBossSCmd(const BYTE* buf, WORD len);

  public:
    void broadcastScene(unsigned char *buf, unsigned short len);
    virtual void broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE t, QWORD index, const void *cmd, WORD len, QWORD excludeID=0, DWORD ip = 0);
    virtual bool sendCmdToMe(QWORD charid, const void* data, unsigned short len);
    bool sendCmdToLoginUser(QWORD accid, const void* data, unsigned short len, ServerTask *task);
    bool sendCmdToWorldUser(QWORD charid, const void *buf, WORD len, Cmd::EDir dir);
  public:
    bool onSceneOpen(SessionScene *scene);
    void loginErr(QWORD accid, Cmd::RegErrRet ret,ServerTask* net);
    void processLoginOutRegCmd(LoginOutRegCmd *rev, Cmd::RegErrRet eErr = REG_ERR_DUPLICATE_LOGIN);

  private:
    xTimer m_oTickOneSec;
    xTimer m_oTickOneMin;
    xTimer m_oTickOneHour;
    DWORD m_dwMatchServerConnectCount = 0;

  private:
    void processSessionThread();
    void doSessionThreadData(SessionThreadData *pData);
};

extern SessionServer *thisServer;
