#pragma once
#include "xServer.h"
#include "xNetProcessor.h"
#include "ServerTask.h"
#include "MsgCounter.h"
#include "GMTools.pb.h"

// 服务器进程的ip端口
struct ServerConfig
{
  std::string ip;
  int port = 0;

  const ServerConfig &operator=(const ServerConfig &s)
  {
    ip = s.ip;
    port = s.port;
    return *this;
  }
};

// zoneid = plat编号*10000*10000 + region编号 * 10000 + zone编号
// platid = plat编号
// regionid = plat编号 * 10000 + region编号
// charid = zoneid * 100000000 + char编号
struct ZoneConfig
{
  // 服务器的IP端口配置 zonelist
  char m_gcIP[MAX_NAMESIZE];
  DWORD m_dwPort;

  ZoneConfig()
  {
    bzero(this, sizeof(*this));
  }
};

enum ZoneServerPortIndex : int
{
  SuperServerIndex = 0,
  LogServerIndex = 1,
  RecordServerIndex = 2,
  SessionServerIndex = 3,
  TeamServerIndex = 4,
  SocialServerIndex = 6,
  DataServerIndex = 7,
  SceneServerMin = 10,
  SceneServerMax = 29,
  GateServerMin = 30,
  GateServerMax = 49,
};

class ZoneServer : public xServer
{
  public:
    ZoneServer(OptArgs &args);
    virtual ~ZoneServer();

  public:
    virtual const char* getServerTypeString() = 0;
    virtual bool isZoneServer() { return true; }
    virtual bool isRegionServer() { return false; }

    // 主循环 从xServer继承
  protected:
    virtual bool v_callback();
    virtual bool v_init() { return true; }
    virtual void v_final();
    virtual void v_timetick();
    virtual void v_closeNp(xNetProcessor *);

    // 初始化
  private:
    bool init();
  public:
    bool load();

    // 连接服务器
  private:
    bool connect();
    bool checkConnect();

  private:
    xTimer m_oCheckConnectTimer;
    xTimer m_oThreeSecTimer;

    // 消息处理
  private:
    void process();
  protected:
    virtual bool doCmd(xNetProcessor* np, BYTE* buf, WORD len);

  protected:
    virtual void v_closeServer(xNetProcessor*) {}

    // 验证服务器
  private:
    void sendVerify(xNetProcessor*, bool ret);
    bool verifyServer(xNetProcessor* np, const char* t, const char* n);  //主线程
    // 服务器验证成功
  private:
    void verifyOk(xNetProcessor* task);
  protected:
    virtual void v_verifyOk(xNetProcessor* task) {}

    // 服务器初始化完成
  protected:
    virtual void init_ok();
    // 服务器初始化完成
  protected:
    virtual void initOkServer(ServerTask *task)
    {
      if (task) task->setState(ServerTask_State::okay);
    }
    bool m_blInit = false;

    // 服务器连接
  public:
    ServerTask* getConnectedServer(std::string t, std::string n = "");
    bool getConnectedServer(std::string t, std::string n, std::vector<ServerTask *> &list);
    ServerTask* getServerTask(xNetProcessor *np);
  protected:
    bool checkConnectedServer(std::string t, std::string n = "");
  private:
    bool connectServerByType(std::string t);
  protected:
    // ServerType ServerName ServerTask
    typedef std::map<std::string, std::map<std::string, ServerTask *>> ServerTaskList;
    ServerTaskList m_oServerList;
    typedef std::vector<ServerTask *> ServerTaskVec;

    // 服务器进程 配置
  public:
    const char* getServerIP();
    int getServerPort();
    INT getAServerIP(const char* type, const char *name);
    INT getAServerPort(const char* type, const char *name);
    DWORD getPort(std::string type, std::string name);
  private:
    void addServerConfig(std::string type, std::string name, std::string ip, DWORD port);
  protected:
    typedef std::map<std::string, std::map<std::string, ServerConfig>> ServerConfigList;
    ServerConfigList m_oServerConfig;

    // 服务器连接配置
  private:
    bool isConnectServerType(std::string f, std::string t);
  private:
    std::map<std::string, std::set<std::string>> m_oConnectConfig;

    // 区和平台信息
  protected:
    ZoneConfig m_oZoneConfig;

    // 消息统计
  public:
    MsgCounter& getMsgCounter() { return m_oMsgCounter; }
  protected:
    MsgCounter m_oMsgCounter;

    // 消息发送
  public:
    virtual void broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE t, QWORD index, const void *cmd, WORD len, QWORD excludeID=0, DWORD ip=0) {}
    bool sendCmdToServer(const void* data, unsigned short len, const char* type, const char* name = "");
    bool sendCmdToOtherServer(const void* data, unsigned short len, const char* type, const char* exclude);
    bool sendCmdToSession(const void* data, unsigned short len) { return sendCmdToServer(data, len, "SessionServer"); }
    bool sendCmdToAllScene(const void* data, unsigned short len) { return sendCmdToServer(data, len, "SceneServer"); }
    bool sendCmdToRecord(const void* data, unsigned short len) { return sendCmdToServer(data, len, "RecordServer"); }
    bool sendCmdToSuper(const void* data, unsigned short len) { return sendCmdToServer(data, len, "SuperServer"); }
    bool sendCmdToGate(const void* data, unsigned short len) { return sendCmdToServer(data, len, "GateServer"); }
    bool sendCmdToData(const void* data, unsigned short len) { return sendCmdToServer(data, len, "DataServer"); }
    virtual bool sendCmdToMe(QWORD charid, const void* data, unsigned short len) { return false; }
    virtual bool sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len) { return false; }

    // 消息处理
  public:
    virtual bool doLogCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doGmToolsCmd(xNetProcessor* np, const BYTE* buf, WORD len) { return false; }
    virtual bool doRegCmd(xNetProcessor *np, BYTE* buf, WORD len) { return false; }
    virtual bool doLoginUserCmd(xNetProcessor* np, BYTE* buf, WORD len) { return false; }
    virtual bool doRecordDataCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doTradeCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doMatchCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doAuctionCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doGatewayCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doErrorUserCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doSessionCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doGateSuperCmd(xNetProcessor *np, BYTE* buf, WORD len) { return false; }
    virtual bool doSocialCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doClientCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doTeamCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doGuildCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doWeddingCmd(const BYTE* buf, WORD len) { return false; }
    //virtual bool doItemSCmd(const BYTE* buf, WORD len) { return false; }
    virtual bool doBossSCmd(const BYTE* buf, WORD len) { return false; }

  public:
    virtual void onRegistRegion(ClientType type) {}
};
