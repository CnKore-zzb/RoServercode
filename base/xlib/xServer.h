#pragma once
#include <netinet/in.h>
#include <sys/epoll.h>
#include <map>
#include <list>
#include "xNoncopyable.h"
#include "xDefine.h"
#include "xTime.h"
#include "xMutex.h"
#include "xListenThread.h"
#include "xLuaTable.h"
#include "xDBConnPool.h"
#include "xTaskThreadPool.h"
#include "MessageStat.h"
#include "ClientManager.h"
#include "SystemCmd.pb.h"

#define RO_DATABASE_NAME "ro_global"
#define CHECK_LOAD_CONFIG(log)\
{ XERR << "[加载配置失败]" << log << XEND;\
}

class xNetProcessor;
//class TerminateMethod;

enum ServerType
{
  SERVER_TYPE_NULL = 0,
  SERVER_TYPE_SUPER,
  SERVER_TYPE_SESSION,
  SERVER_TYPE_SCENE,
  SERVER_TYPE_GATE,
  SERVER_TYPE_RECORD,
  SERVER_TYPE_PROXY,
  SERVER_TYPE_STAT,
  SERVER_TYPE_TRADE,
  SERVER_TYPE_GLOBAL,
  SERVER_TYPE_TEAM,
  SERVER_TYPE_GUILD,
  SERVER_TYPE_SOCIAL,
  SERVER_TYPE_GZONE,
  SERVER_TYPE_MATCH,
  SERVER_TYPE_AUCTION,
  SERVER_TYPE_ROBOTS,
  SERVER_TYPE_G_PROXY,
  SERVER_TYPE_DATA,
  SERVER_TYPE_WEDDING,
};

enum ZoneCategory : DWORD
{
  ZoneCategory_PVP_LLH = 0,   // 溜溜猴     1
  ZoneCategory_PVP_SMZL = 1,   // 沙漠之狼   2
  ZoneCategory_PVP_HLSJ = 2,   // 华丽水晶   4
  ZoneCategory_MAX = 3,
};


class OptArgs
{
  public:
    void get(int argc, char* argv[]);

  public:
    std::string m_strServerName;
    std::string m_strPlatName;
    std::string m_strRegionName;
    std::string m_strZoneName;
    bool m_blOuter = false;   // 是否是发布版
    std::string m_strVersion;    // 记录版本号
    bool m_blBuild = false;    // 只创建NavMesh,不启动
    std::string m_strProxyID;      // proxy的idcommonReload
    std::string m_strFullName;
    bool m_blTest = false;    // 是否是外网测试
};

typedef std::pair<std::string/*ip*/, DWORD/*port*/> TIpPortPair;

struct ServerData
{
  public:
    void init(OptArgs &args);
    const TIpPortPair* getIpPort(const std::string& serverName) const;
  public:
    DWORD m_dwPlatID = 0;
    string m_strPlatName;
    DWORD m_dwRegionID = 0;
    string m_strRegionName;
    DWORD m_dwZoneID = 0;
    string m_strZoneName;

    string m_strServerVersion;

    string m_strPlatDBName;
    string m_strRegionDBName;
    string m_strGameDBName;

    //string m_strStatIp;
    //DWORD m_dwStatPort = 0;
    //string m_strTradeIp;
    //DWORD m_dwTradePort = 0;
    //string m_strSocialIp = "127.0.0.1";
    //DWORD m_dwSocialPort = 6666;
    std::map<std::string/*ServerName*/, TIpPortPair> m_mapIpPort;
};

enum class ServerState
{
  create          = 0,
  init            = 1,  // 加载配置 初始化
  connect         = 2,  // 连接其他服务器
  run             = 3,  // 正常运行
  save            = 4,  // 保存数据，仅场景进程用
  stop            = 5,  // 准备停机
  finish          = 6,  // 完成停机
};

class xServer : private xNoncopyable
{
  public:
    xServer(OptArgs &args);
    virtual ~xServer();

  public:
    virtual ServerType getServerType() = 0;
    const char *getServerName() const { return s_oOptArgs.m_strServerName.c_str(); }
    virtual bool isZoneServer() = 0;
    virtual bool isRegionServer() = 0;

    // 进程状态
  public:
    ServerState getServerState() { return m_oServerState; }
    void stop() { if (m_oServerState < ServerState::stop) setServerState(ServerState::stop); }
  protected:
    void setServerState(ServerState s);
  private:
    volatile ServerState m_oServerState = ServerState::create;

    // 主进程
  public:
    void run();
    // 回调函数
  protected:
    bool callback();

    // TCP Task
  public:
    void select_th(int epfd, int sock, epoll_event evs[]);
  private:
    bool accept(int sockfd, const sockaddr_in &addr);
  public:
    xNetProcessor* newTask();
    xNetProcessor* newClient();
    xNetProcessor* newPlatClient();
    xNetProcessor* newFluentClient();

    // TCP 连接限制 只处理Task
    // 为0 则不限制
  protected:
    inline void setTCPTaskMax(DWORD num)
    {
      m_dwTCPTaskMaxNum = num;
    }
  private:
    DWORD m_dwTCPTaskNum = 0;
    DWORD m_dwTCPTaskMaxNum = 0;

public:
  bool isZoneCategory(DWORD dwZoneCategory ,ZoneCategory category)
  {
    return (dwZoneCategory & (1 << category)) != 0;
  }
  bool isZoneCategory(ZoneCategory category)
  {
    return isZoneCategory(m_dwZoneCategory, category);
  }
  bool isPvpZone();
  DWORD getZoneCategory() { return m_dwZoneCategory; }
protected:
  DWORD m_dwZoneCategory = 0;

    // TCP 连接缓存
  public:
    void setVerifyList(xNetProcessor *t, DWORD);
  protected:
    void removeVerifyList(xNetProcessor *task);
    bool inVerifyList(xNetProcessor *task);
  private:
    xRWLock m_oVerifyLock;
    std::map<xNetProcessor *, DWORD> m_oVerifyList;  // task:time

    // 处理TCP断开
  public:
    void addCloseList(xNetProcessor *np, TerminateMethod method, const char *desc);
  protected:
    void checkCloseList(DWORD delay=30);
    virtual void v_closeNp(xNetProcessor *np) = 0;
  private:
    xRWLock m_oCloseNpLock;
    std::map<xNetProcessor *, DWORD> m_oCloseNpList;  // np:time 删除列表
    xRWLock m_oCloseNpSetLock;
    std::set<xNetProcessor *> m_oCloseNpSet;  // closenp 调用

    // TCP连接池
  public:
    xTaskThreadPool& getTaskThreadPool() { return m_oTaskThreadPool; }
  protected:
    xTaskThreadPool m_oTaskThreadPool;

    // 监听
  protected:
    bool listen();
    void setServerPort(int port) { m_oListenThread.setPort(port); }
    int getServerPort() { return m_oListenThread.getPort(); }
  private:
    xListenThread m_oListenThread;

    // 主循环 虚函数
  protected:
    virtual bool v_init() { return true; }
    virtual bool v_callback() { return true; };
    virtual void v_final();
    virtual bool v_stop() { return true; }
    virtual void v_timetick();

    // 消息处理
  public:
    void doCmd(xNetProcessor *np);
    virtual bool doCmd(xNetProcessor *np, BYTE *buf, WORD len) = 0;

    // 命令行参数
  public:
    static OptArgs s_oOptArgs;
    static bool isOuter() { return s_oOptArgs.m_blOuter; }
    static std::string& getVersion() { return s_oOptArgs.m_strVersion; }
    static std::string& getFullName() { return s_oOptArgs.m_strFullName; }

    // 配置文件
  public:
    static const xLuaData& getBranchConfig();
    void commonReload(const CommonReloadSystemCmd& rev);
    void loadCommonConfig();
    void checkDbCfgChange(const string& name, DBConnPool &pool);
    // 数据库
  public:
    inline DBConnPool& getDBConnPool()
    {
      return m_oDBConnPool;
    }
  private:
    DBConnPool m_oDBConnPool;

  public:
    inline DBConnPool& getTradeConnPool()
    {
      return m_oTradeConnPool;
    }
  private:
    DBConnPool m_oTradeConnPool;

  public:
    bool addDataBase(const string& database, bool isTrade);
    void delDataBase() { m_oDBConnPool.final(); m_oTradeConnPool.final(); }
    static bool initDBConnPool(const string& name, const string& database, DBConnPool &pool);

    void loadDb();
    virtual void v_loadDb();

    // 定时器
  private:
    xTimer m_oOneSecTimer;
    xTimer m_oTenSecTimer;
    xTimer m_oTenMinTimer;

    // 消息统计
  public:
    MessageStat m_oMessageStat;

    // 区服相关数据
  public:
    const string& getPlatformName() const { return m_oServerData.m_strPlatName; }
    DWORD getPlatformID() const { return m_oServerData.m_dwPlatID; }
    const string& getRegionName() const { return m_oServerData.m_strRegionName; }
    DWORD getRegionID() const { return m_oServerData.m_dwRegionID; }
    const string& getZoneName() const { return m_oServerData.m_strZoneName; }
    DWORD getZoneID() const { return m_oServerData.m_dwZoneID; }
    const string& getServerVersion() const { return m_oServerData.m_strServerVersion; }

    const string& getPlatDBName() const { return m_oServerData.m_strPlatDBName; }
    const string& getRegionDBName() const { return m_oServerData.m_strRegionDBName; }
    const string& getGameDBName() const { return m_oServerData.m_strGameDBName; }

    const ServerData& getServerData() { return m_oServerData; };
  protected:
    bool loadPlatform();
    bool loadRegionID();
    bool loadRegionSvrList();
  protected:
    ServerData m_oServerData;

    // 外部连接
  public:
    bool sendCmd(ClientType type, const void *buf, DWORD len);
    bool sendCmdToLog(const void* data, DWORD len) { return sendCmd(ClientType::log_server,data, len); }
    bool sendCmdToPlat(const void* data, unsigned short len) { return sendCmd(ClientType::plat_server,data, len); }

  protected:
    void addClient(ClientType type, std::string ip, DWORD port);
  protected:
    ClientManager m_oClientManager;

  private:
    std::list<xNetProcessor *> m_oDeleteList;
    std::set<string> m_setDatabase;
};
