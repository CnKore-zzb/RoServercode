#include <mysql.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <string>
#include <list>
#include "xServer.h"
#include "xXMLParser.h"
#include "xTools.h"
#include "xSocket.h"
#include "xNetProcessor.h"
#include "SystemCmd.pb.h"
#include "xDBConnPool.h"
#include "BaseConfig.h"
#include "MsgManager.h"
#include "CommonConfig.h"
#include "PlatLogManager.h"

OptArgs xServer::s_oOptArgs;
extern xLog srvLog;

void OptArgs::get(int argc, char* argv[])
{
  //XLOG_SETSILENT;
  int oc = -1;
  while((oc = getopt(argc, argv, "dp:r:n:s:v:x:obt")) != -1)
  {
    switch(oc)
    {
      case 'd':
        {
          daemon(1, 1);
        }
        break;
      case 'p':
        {
          m_strPlatName = optarg;
        }
        break;
      case 'r':
        {
          m_strRegionName = optarg;
        }
        break;
      case 's':
        {
          m_strZoneName = optarg;
        }
        break;
      case 'n':
        {
          m_strServerName = optarg;
        }
        break;
      case 'o':
        {
          m_blOuter = true;
        }
        break;
      case 't':
        {
          m_blTest = true;
        }
        break;
      case 'v':
        {
          m_strVersion = optarg;
        }
        break;
      case 'b':
        {
          m_blBuild = true;
        }
        break;
      case 'x':
        {
          m_strProxyID = optarg;
        }
        break;
      case '?':
        break;
    }
  }

  std::stringstream stream;
  stream.str("");
  if (m_strProxyID.empty())
  {
    stream << "ro_" << m_strPlatName << "_" << m_strRegionName;
    if (!m_strZoneName.empty())
    {
      stream << "_" << m_strZoneName;
    }
  }
  else
  {
    stream << "ro_proxy" << "_s" << m_strProxyID;
  }

  m_strFullName = stream.str();
}

void ServerData::init(OptArgs &args)
{
  m_strPlatName = args.m_strPlatName;
  m_strRegionName = args.m_strRegionName;
  m_strZoneName = args.m_strZoneName;

  if (!m_strPlatName.empty())
  {
    std::stringstream stream;
    stream.str("");
    stream << "ro_" << m_strPlatName;
    m_strPlatDBName = stream.str();
    XLOG << "[平台数据库]" << m_strPlatDBName << XEND;

    if (!m_strRegionName.empty())
    {
      stream << "_" << m_strRegionName;
      m_strRegionDBName = stream.str();
      XLOG << "[大区数据库]" << m_strRegionName << XEND;
    }
    if (!m_strZoneName.empty())
    {
      stream << "_" << m_strZoneName;
      m_strGameDBName = stream.str();
      XLOG << "[区数据库]" << m_strGameDBName << XEND;
    }
    stream.str("");
  }
}

const TIpPortPair* ServerData::getIpPort(const std::string& serverName) const
{
  auto it = m_mapIpPort.find(serverName);
  if (it == m_mapIpPort.end())
  {
    XERR << "[服务器地址] 找不到相应服务器地址"<<serverName << XEND;
    return nullptr;
  }
  return &it->second;
}

void handle_pipe(int s)
{
  XLOG << "[handle_pipe]" << XEND;
}

xServer::xServer(OptArgs &args) : m_oTaskThreadPool(this)
  ,m_oListenThread(this)
  ,m_oOneSecTimer(1)
  ,m_oTenSecTimer(10)
  ,m_oTenMinTimer(600)
  ,m_oClientManager(this)
{
  s_oOptArgs = args;
  m_oServerData.init(args);

  std::stringstream stream;
  stream.str("");

  //srvLog = new xLog();
  //if (srvLog)
  //{
    stream.str("");
    if (args.m_blOuter)
    {
      stream << "/data/rogame/log/";
    }
    else
    {
      stream << "log/";
    }

    if (args.m_strProxyID.empty())
    {
      stream << args.m_strPlatName << "/" << args.m_strRegionName;
      if (!args.m_strZoneName.empty())
      {
        stream << "_" << args.m_strZoneName;
      }
      stream << "/";
    }
    else
    {
      stream << "proxy/" << args.m_strProxyID << "/";
    }

      //srvLog->setLogDir(stream.str().c_str());
    if (!srvLog.init(getServerName(), stream.str().c_str()))
      fprintf(stderr, "[%s],srvLog init failed", getServerName());
  //}
  //else
  //{
  //  fprintf(stderr, "[%s],srvLog init failed", getServerName());
  //}

  struct sigaction action;
  action.sa_handler = handle_pipe;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGPIPE, &action, NULL);

  if (mysql_server_init(0, NULL, NULL))
  {
    XLOG << "[ZoneServer],初始化,mysql_server_init 失败" << XEND;
  }
}

xServer::~xServer()
{
}

void xServer::v_final()
{
  ScopeWriteLock swl(m_oVerifyLock);
  for (auto &it : m_oVerifyList)
  {
    addCloseList(it.first, TerminateMethod::terminate_active, "停服关闭");
  }
  m_oVerifyList.clear();
}

void xServer::addCloseList(xNetProcessor *np, TerminateMethod method, const char *desc)
{
  if (!np) return;

  {
    ScopeWriteLock swl(m_oCloseNpLock);

    if (NPState::disconnect==np->getNpState()) return;

    np->setNpState(NPState::disconnect);
    np->terminate(method, desc);
    m_oTaskThreadPool.del(np);
    np->disconnect();

    removeVerifyList(np);

    m_oCloseNpList[np] = now();
  }
  {
    ScopeWriteLock swl(m_oCloseNpSetLock);
    m_oCloseNpSet.insert(np);
  }
}

void xServer::checkCloseList(DWORD delay)
{
  for (auto &it : m_oDeleteList)
  {
    if (it->isTask() && m_dwTCPTaskNum)
    {
      m_dwTCPTaskNum--;
    }
    XLOG << "[xNetProcessor]" << "m_oDeleteList,删除对象," << it->name << "(" << it->id << ")," << it << "tcp task num:" << m_dwTCPTaskNum << XEND;
    SAFE_DELETE(it);
  }
  m_oDeleteList.clear();

  DWORD cur = now();
  {
    ScopeWriteLock swl(m_oCloseNpLock);
    auto it = m_oCloseNpList.begin();
    auto tmp = it;
    for ( ; it!=m_oCloseNpList.end(); )
    {
      tmp = it++;
      if (delay)
      {
        if (tmp->second + 10 >= cur)
          continue;
      }

      m_oDeleteList.push_back(tmp->first);
      XLOG << "[xNetProcessor]" << "m_oCloseNpList,删除连接," << tmp->first->name << "(" << tmp->first->id << ")," << tmp->first << XEND;

      m_oCloseNpList.erase(tmp);
    }
  }

  std::set<xNetProcessor *> tmpset;
  {
    ScopeWriteLock swl(m_oCloseNpSetLock);
    tmpset.swap(m_oCloseNpSet);
    m_oCloseNpSet.clear();
  }
  for (auto &it : tmpset)
  {
    m_oClientManager.closeNp(it);
    v_closeNp(it);
  }

  if (0 == delay)
  {
    for (auto &it : m_oDeleteList)
    {
      if (it->isTask() && m_dwTCPTaskNum)
      {
        m_dwTCPTaskNum--;
      }
      SAFE_DELETE(it);
    }
    m_oDeleteList.clear();
  }
}

bool xServer::listen()
{
  if (!m_oListenThread.getPort())
  {
    XERR << "[监听]" << "监听端口错误" << XEND;
    return false;
  }

  if (!m_oListenThread.bind())
  {
    XERR << "[监听]" << "listener bind failed" << XEND;
    return false;
  }
  if (!m_oListenThread.thread_start())
  {
    XERR << "[监听]" << "start listener failed" << XEND;
  }

  XLOG << "[监听]" << "开始监听" << m_oListenThread.getPort() << XEND;
  return true;
}

void xServer::setServerState(ServerState s)
{
  if (m_oServerState >= s)
  {
    XLOG << "[ServerState],往回设置" << (DWORD)m_oServerState << "," << (DWORD)s << XEND;
    return;
  }
  m_oServerState = s;
  switch (m_oServerState)
  {
    case ServerState::create:
      {
        XLOG << "[进程状态],创建成功" << XEND;
      }
      break;
    case ServerState::init:
      {
        XLOG << "[进程状态],准备初始化数据" << XEND;
      }
      break;
    case ServerState::run:
      {
        XLOG << "[进程状态],初始化完毕，开始运行" << XEND;
      }
      break;
    case ServerState::save:
      {
        XLOG << "[进程状态],保存数据，即将终止" << XEND;
      }
      break;
    case ServerState::stop:
      {
    //    XLOG("[%s],主循环结束，即将终止");
      }
      break;
    case ServerState::finish:
      {
        XLOG << "[进程状态],进程终止" << XEND;
      }
      break;
    default:
      break;
  }
}

void xServer::setVerifyList(xNetProcessor *task, DWORD time)
{
  if (!task) return;

  XDBG_T("[verify_list],添加:%p", task);

  ScopeWriteLock swl(m_oVerifyLock);
  m_oVerifyList[task] = time;
}

void xServer::removeVerifyList(xNetProcessor *task)
{
  if (!task) return;

  XDBG_T("[verify_list],删除:%p", task);

  ScopeWriteLock swl(m_oVerifyLock);
  m_oVerifyList.erase(task);
}

bool xServer::inVerifyList(xNetProcessor *task)
{
  if (!task) return false;

  ScopeWriteLock swl(m_oVerifyLock);
  return m_oVerifyList.find(task)!=m_oVerifyList.end();
}

void xServer::select_th(int epfd, int sock, epoll_event events[])
{
  if (!epfd) return;

  bzero(events, MAX_SERVER_EVENT * sizeof(epoll_event));
  int nfds = epoll_wait(epfd, events, MAX_SERVER_EVENT, 20);
  DWORD cur = now();
  for (int i=0; i < nfds; ++i)
  {
    if (sock)
    {
      if (INVALID_SOCKET != sock && events[i].data.fd == sock)
      {
        sockaddr_in caddr;
        int addrlen = sizeof(caddr);
        bzero(&caddr, sizeof(caddr));
        int cfd = ::accept(sock, (struct sockaddr*)&caddr, (socklen_t *)&addrlen);
        if(-1 == cfd)
        {
          XERR_T("[select],epfd:%u, sock:%u, accept 出错,errno:%s", epfd, sock, strerror(errno));
          sleep(1);
        }
        else
        {
          if (m_dwTCPTaskMaxNum && m_dwTCPTaskNum >= m_dwTCPTaskMaxNum)
          {
            XERR_T("[accept], TCP Task数量超过上限，拒绝连接,%u", m_dwTCPTaskNum);
            SAFE_CLOSE_SOCKET(cfd);
          }
          else
          {
            accept(cfd, caddr);
          }
          usleep(1000);
        }
      }
    }
    else
    {
      xNetProcessor *np = (xNetProcessor *)events[i].data.ptr;
      if (!np || np->isTerminate()) continue;
      if (events[i].events & EPOLLERR)
      {
        XLOG_T("[select],连接错误,%s,%p", np->name, np);
        addCloseList(np, TerminateMethod::terminate_socket_error, "EPOLLERR");
        continue;
      }
      else
      {
        if (events[i].events & EPOLLOUT)
        {
          int ret = np->sendData();
          if (-1 == ret)
          {
            XLOG_T("[select],发送错误,%s,%p", np->name, np);
            addCloseList(np, TerminateMethod::terminate_socket_error, "EPOLLOUT");
            continue;
          }
          else if (ret>0)
          {
            np->addEpoll();
          }
        }
        if (events[i].events & EPOLLIN)
        {
          if (!np->recvData())
          {
            XLOG_T("[select],读取错误,%s,%p", np->name, np);
            addCloseList(np, TerminateMethod::terminate_socket_error, "EPOLLIN");
          }
          np->tick(cur);
        }
      }
    }
  }
}

xNetProcessor* xServer::newTask()
{
  return new xNetProcessor("Task", NET_PROCESSOR_TYPE_TASK);;
}

xNetProcessor* xServer::newClient()
{
  return new xNetProcessor("Client", NET_PROCESSOR_TYPE_CLIENT);
}

xNetProcessor* xServer::newPlatClient()
{
  return new xNetProcessor("Plat_Client", NET_PROCESSOR_TYPE_PLAT_CLIENT);
}

xNetProcessor* xServer::newFluentClient()
{
  return new xNetProcessor("Fluent_Client", NET_PROCESSOR_TYPE_FLUENT_CLIENT);
}

bool xServer::callback()
{
  xTime frameTimer;

  v_timetick();

  if (!v_callback())
    return false;

  QWORD _e = frameTimer.uElapse();
  if (_e < 10000)
  {
    usleep(10000 - _e);
  }
  else
  {
    usleep(1000);
  }

  if (_e > 500000)
  {
    XLOG << "[帧耗时]" << _e << " 微秒" << XEND;
  }

  return true;
}

void xServer::run()
{
#define FINAL_RETURN { final(); return; }
  // daemon(1,1);

  setServerState(ServerState::create);
  xXMLParser::initSystem();

  srand(xTime::getCurUSec());
  if (!BaseConfig::getMe().loadConfig(s_oOptArgs.m_blTest))
  {
    XERR << "[BaseConfig]" << "加载失败" << XEND;
  }
  loadCommonConfig();
  PlatLogManager::getMe().init(this);

  while (callback());
  setServerState(ServerState::stop);

  setServerState(ServerState::finish);
  v_final();
  m_oListenThread.thread_stop();
  checkCloseList(0);
  //XLOG_END;
  mysql_server_end();
  m_oDBConnPool.final();
  m_oTradeConnPool.final();
  xXMLParser::clearSystem();
  google::protobuf::ShutdownProtobufLibrary();
#undef FINAL_RETURN
}

bool xServer::accept(int sockfd, const sockaddr_in& addr)
{
  xNetProcessor* task = newTask();
  if (!task->accept(sockfd, addr))
  {
    SAFE_DELETE(task);
    return false;
  }

  ++m_dwTCPTaskNum;

  XLOG_T("[accept],%s:%u,connect, new task:%p add verify list, tcp task num:%u", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), task, m_dwTCPTaskNum);

  switch (getServerType())
  {
    case SERVER_TYPE_SUPER:
      {
        setVerifyList(task, now());
        m_oTaskThreadPool.add(task);
      }
      break;
    case SERVER_TYPE_SESSION:
    case SERVER_TYPE_SCENE:
    case SERVER_TYPE_RECORD:
      {
        setVerifyList(task, now());
        m_oTaskThreadPool.add(task);
        /*
        task->m_oThread = new xTaskThread(this);
        if (!task->m_oThread->thread_start())
        {
          XERR << "[accept]" << "start task failed" << XEND;
          SAFE_DELETE(task);
          return false;
        }

        setVerifyList(task, now());
        task->addEpoll(task->m_oThread->getEpfd());
        */
      }
      break;
    case SERVER_TYPE_PROXY:
      {
//#ifdef _ALL_SUPER_GM
        task->m_oSocket.setEnc(true);
//#endif
        setVerifyList(task, now());
        m_oTaskThreadPool.add(task);
      }
      break;
    case SERVER_TYPE_GATE:
    case SERVER_TYPE_STAT:
    default:
      {
        setVerifyList(task, now());
        m_oTaskThreadPool.add(task);
      }
      break;
  }

  return true;
}

void xServer::doCmd(xNetProcessor *np)
{
  if (!np) return;
  CmdPair *cmd = np->getCmd();
  while (cmd)
  {
    if (np->isFluentClient())
    {
      PlatLogManager::getMe().doCmd(cmd->second, cmd->first);
    }
    else
    {
      m_oMessageStat.start(((xCommand *)cmd->second)->cmd, ((xCommand *)cmd->second)->param);
      if (!doCmd(np, cmd->second, cmd->first))
      {
        XDBG << "[消息处理错误]," << inet_ntoa(np->getIP()) << "," << np->getPort() << "," << ((xCommand *)cmd->second)->cmd << "," << ((xCommand *)cmd->second)->param << XEND;
      }
      else
      {
        // XDBG("[消息处理],消息处理,%u,%u", ((xCommand *)cmd->second)->cmd, ((xCommand *)cmd->second)->param);
      }
      m_oMessageStat.end();
    }
    np->popCmd();
    cmd = np->getCmd();
  }
}

const xLuaData& xServer::getBranchConfig()
{
  return BaseConfig::getMe().getBranchData();
}

bool xServer::addDataBase(const string& database, bool isTrade)
{
  if (isTrade)
  {
    if (!initDBConnPool("TradeDataBase", database, m_oTradeConnPool))
    {
      XERR << "[数据库-添加]" << database << "TradeDataBase,添加失败" << XEND;
      return false;
    }
  }
  else
  {
    if (!initDBConnPool("DataBase", database, m_oDBConnPool))
    {
      XERR << "[数据库-添加]" << database << "DataBase,添加失败" << XEND;
      return false;
    }
  }

  return true;
}

bool xServer::initDBConnPool(const string& name, const string& database, DBConnPool &pool)
{
  xLuaData data = getBranchConfig().getData(name.c_str());
  std::vector<xLuaData> vecips;
  auto initdb = [&](const string& key, xLuaData& d)
  {
    vecips.push_back(d);
  };
  data.foreach(initdb);
  if (!vecips.empty())
  {
    pool.setConfig(vecips);
    return pool.addDBConn(database);
  }
  else
  {
    XERR_T("[数据库-添加],%s,%s,添加失败,可选数据库列表为空", name.c_str(), database.c_str());
    return false;
  }
  return false;
}

bool xServer::loadPlatform()
{
  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "platform");
  if (field)
  {
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "platname = '%s'", getPlatformName().c_str());

    xRecordSet set;
    QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX!=ret && ret)
    {
      m_oServerData.m_dwPlatID = set[0].get<DWORD>("platid");
      m_oServerData.m_strServerVersion = set[0].getString("version");
      return true;
    }
    else
    {
      XERR << "[加载平台信息],加载平台ID失败," << getPlatformName() << XEND;
    }
  }
  return false;
}

bool xServer::loadRegionID()
{
  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "region");
  if (field)
  {
    field->setValid("regionid");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "regionname = '%s' and platid = %u", xServer::getRegionName().c_str(), m_oServerData.m_dwPlatID);

    xRecordSet set;
    QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX == ret)
    {
      XERR << "[加载大服信息]" << "失败" << xServer::getRegionName() << m_oServerData.m_dwPlatID << XEND;
      return false;
    }
    if (ret)
    {
      m_oServerData.m_dwRegionID = set[0].get<DWORD>("regionid");
      XLOG << "[加载大服信息]" << m_oServerData.m_dwRegionID << xServer::getRegionName() << m_oServerData.m_dwPlatID << XEND;
      return loadRegionSvrList();
    }
  }

  return false;
}

bool xServer::loadRegionSvrList()
{
  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "region_svrlist");
  if (field)
  {
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "regionid = %u", m_oServerData.m_dwRegionID);

    xRecordSet set;
    QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[加载公共服]" << "失败" << m_oServerData.m_dwRegionID << XEND;
      return false;
    }
    for (QWORD i = 0; i < ret; ++i)
    {
      TIpPortPair p;
      p.first = set[i].getString("ip");
      p.second = set[i].get<DWORD>("port");
      m_oServerData.m_mapIpPort[set[i].getString("servertype")] = p;
      XLOG << "[加载公共服]" << set[i].getString("servertype") << p.first << p.second << XEND;
    }
    return true;
  }
  return false;
}

void xServer::addClient(ClientType type, std::string ip, DWORD port)
{
  m_oClientManager.add(type, ip, port);
  XLOG << "[客户端]" << "添加" << (DWORD)type << ip << port << XEND;
}

bool xServer::sendCmd(ClientType type, const void *buf, DWORD len)
{
  if (!m_oClientManager.sendCmd(type, (BYTE *)buf, len))
  {
    // XERR << "[客户端]" << "发送失败" << (DWORD)type << XEND;
    return false;
  }
  return true;
}

void xServer::commonReload(const CommonReloadSystemCmd& rev)
{
  if (rev.type() == EComLoadType_Lua)
    loadCommonConfig();
  else if (rev.type() == EComLoadType_Db)
    loadDb();
  else if (rev.type() == EComLoadType_BranchConfig)
  {
    BaseConfig::getMe().loadConfig();
    checkDbCfgChange("DataBase", m_oDBConnPool);
    checkDbCfgChange("TradeDataBase", m_oTradeConnPool);
  }
}

void xServer::loadCommonConfig()
{
  CommonConfig::getMe().loadConfig();
}

void xServer::checkDbCfgChange(const string& name, DBConnPool &pool)
{
  if (pool.empty()) return;

  xLuaData data = getBranchConfig().getData(name.c_str());
  std::vector<xLuaData> vecips;
  auto initdb = [&](const string& key, xLuaData& d)
  {
    vecips.push_back(d);
  };
  data.foreach(initdb);
  if (vecips.empty())
    return;

  pool.setConfig(vecips);
  pool.reload();

  XLOG << "[BranchConfig重加载-数据库ip变化], 重加载数据database成功 " << name << XEND;
}

void xServer::loadDb()
{
  v_loadDb();
}

void xServer::v_loadDb()
{
}

bool xServer::isPvpZone()
{
  if (isZoneCategory(ZoneCategory_PVP_LLH) == true)
    return true;
  if (isZoneCategory(ZoneCategory_PVP_HLSJ) == true)
    return true;
  if (isZoneCategory(ZoneCategory_PVP_SMZL) == true)
    return true;
  return false;
}
