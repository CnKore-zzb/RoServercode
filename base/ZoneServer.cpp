#include "ZoneServer.h"
#include "xXMLParser.h"
#include "xNetProcessor.h"
#include "SuperCmd.h"
#include <arpa/inet.h>
#include <errno.h>
#include <list>
#include <algorithm>
#include <iostream>
//#include "GlobalConfig.h"
#include "SystemCmd.pb.h"
#include "xCmd.pb.h"
#include "RedisManager.h"
#include "BaseConfig.h"
#include "xNewMem.h"

ZoneServer *thisServer = nullptr;

ZoneServer::ZoneServer(OptArgs &args) : xServer(args),m_oCheckConnectTimer(3),m_oThreeSecTimer(3)
{
  // 初始化服务器类型
  m_oServerConfig["SuperServer"];
  m_oServerConfig["RecordServer"];
  m_oServerConfig["SessionServer"];
  m_oServerConfig["TeamServer"];
  m_oServerConfig["SocialServer"];
  m_oServerConfig["SceneServer"];
  m_oServerConfig["GateServer"];
  m_oServerConfig["DataServer"];

  // 初始化连接
  m_oConnectConfig["SessionServer"].insert("SuperServer");
  m_oConnectConfig["SessionServer"].insert("RecordServer");
  m_oConnectConfig["SessionServer"].insert("TeamServer");
  m_oConnectConfig["SessionServer"].insert("SocialServer");
  m_oConnectConfig["SessionServer"].insert("DataServer");
  m_oConnectConfig["TeamServer"].insert("SuperServer");
  m_oConnectConfig["SocialServer"].insert("SuperServer");
  m_oConnectConfig["TeamServer"].insert("SocialServer");
  m_oConnectConfig["SceneServer"].insert("SuperServer");
  m_oConnectConfig["SceneServer"].insert("SessionServer");
  m_oConnectConfig["SceneServer"].insert("RecordServer");
  m_oConnectConfig["SceneServer"].insert("DataServer");
  m_oConnectConfig["GateServer"].insert("SuperServer");
  m_oConnectConfig["GateServer"].insert("SessionServer");
  m_oConnectConfig["GateServer"].insert("SceneServer");
  m_oConnectConfig["RecordServer"].insert("SuperServer");
  m_oConnectConfig["DataServer"].insert("SuperServer");

  XLOG << "[ZoneServer]" << "初始化" << getPlatformName() << getRegionName() << getZoneName() << XEND;
}

ZoneServer::~ZoneServer()
{
}

bool ZoneServer::v_callback()
{
  switch (getServerState())
  {
    case ServerState::create:
      {
        if (!load())
        {
          XERR << "[服务器连接]" << "初始化失败" << XEND;
          return false;
        }

        setServerState(ServerState::init);

        return true;
      }
      break;
    case ServerState::init:
      {
        if (SERVER_TYPE_SUPER==getServerType() || checkConnectedServer("SuperServer"))
        {
          setServerState(ServerState::connect);

          if (!listen())
          {
            XERR << "[监听]" << "失败" << XEND;
            return false;
          }
          else
          {
            XLOG << "[服务器连接]" << "开始监听" << getServerPort() << XEND;
          }
        }
        return true;
      }
      break;
    case ServerState::connect:
      {
        if (m_oCheckConnectTimer.timeUp(now()))
        {
          if (connect())
          {
            if (false == m_blInit)
            {
              if (!init())
              {
                XERR << "[初始化]" << "失败" << XEND;
                return false;
              }
              m_blInit = true;
            }
            else
            {
              setServerState(ServerState::run);
              init_ok();
            }
          }
        }
      }
      break;
    case ServerState::run:
      {
        return true;
      }
      break;
    case ServerState::stop:
      {
        if (v_stop())
          return false;
        return true;
      }
      break;
    case ServerState::finish:
      {
        return false;
      }
      break;
    default:
      break;
  }
  return true;
}

void ZoneServer::v_final()
{
  bool ret = true;
  while (ret)
  {
    ret = false;
    for (auto it : m_oServerList)
    {
      for (auto sub_it : it.second)
      {
        if (sub_it.second->getTask())
        {
          if (sub_it.second->getTask()->isTask() && (SERVER_TYPE_SUPER != getServerType()))
          {
            XLOG << "[final],等待" << (sub_it.second)->getTask()->name << "断开连接" << XEND;
            sleep(1);
            ret = true;
          }
          else
          {
            xNetProcessor* np = sub_it.second->getTask();
            sub_it.second->setTask(NULL);
            addCloseList(np, TerminateMethod::terminate_active, "ZoneServer Final");
  //          SAFE_DELETE(sub_it.second);
          }
        }
      }
    }
    checkCloseList();
  }
  for (auto it : m_oServerList)
  {
    for (auto sub_it : it.second)
    {
      SAFE_DELETE(sub_it.second);
    }
  }
  m_oServerList.clear();

  xServer::v_final();
}

void ZoneServer::v_timetick()
{
  xServer::v_timetick();

  process();

  // if (ServerState::run == getServerState())
  {
    if (m_oThreeSecTimer.timeUp(now()))
    {
      connect();
    }
  }
}

void ZoneServer::v_closeNp(xNetProcessor* np) //主线程
{
  ServerTask *task = getServerTask(np);
  if (task)
  {
    XLOG << "[服务器连接]" << np->name << "关闭" << XEND;
    v_closeServer(np);
    task->setTask(nullptr);
  }
}

bool ZoneServer::init()
{
  bool bResult = true;
  const xLuaData& data = getBranchConfig().getData("Redis");
  if (RedisManager::getMe().init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port")) == false)
    bResult = false;

  if (!v_init())
  {
    return false;
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool ZoneServer::load()
{
  if (!addDataBase(RO_DATABASE_NAME, false))
  {
    XERR << "[加载平台信息],初始化数据库连接失败:" << RO_DATABASE_NAME << XEND;
    return false;
  }

  if (loadPlatform())
  {
    XLOG << "[加载平台信息],加载平台ID" << getPlatformID() << getPlatformName() << "版本号:" << getServerVersion() << XEND;
  }
  else
  {
    XERR << "[加载平台信息],加载平台ID失败," << getPlatformName() << XEND;
    return false;
  }

  if (!loadRegionID())
  {
    XERR << "[加载区信息],加载regionid失败" << getPlatformName() << getRegionName() << getZoneName() << XEND;
    return false;
  }

  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "zone");
  if (field)
  {
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "zonename = '%s' and regionid = %u", getZoneName().c_str(), getRegionID());

    xRecordSet set;
    QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX!=ret)
    {
      for (DWORD i=0; i<ret; i++)
      {
        m_oServerData.m_dwZoneID = set[i].get<DWORD>("zoneid");

        strncpy(m_oZoneConfig.m_gcIP, set[i].getString("ip"), MAX_NAMESIZE);
        m_oZoneConfig.m_dwPort = set[i].get<DWORD>("port");

        m_dwZoneCategory = set[i].get<DWORD>("category");

        XLOG << "[加载区信息]" << "加载区信息" << getZoneID() << getZoneName() << "ip:" << m_oZoneConfig.m_gcIP << m_oZoneConfig.m_dwPort << XEND;
        XLOG << "[加载区信息]" << "类别" << m_dwZoneCategory << XEND;

        addServerConfig("SuperServer", "SuperServer", m_oZoneConfig.m_gcIP, m_oZoneConfig.m_dwPort);
        DWORD port = getPort(getServerTypeString(), getServerName());
        if (port)
        {
          setServerPort(port);
          XLOG << "[服务器连接]" << "设置监听端口" << port << XEND;
          if (SERVER_TYPE_SUPER != getServerType())
          {
            addServerConfig(getServerTypeString(), getServerName(), m_oZoneConfig.m_gcIP, port);
          }
        }
        break;
      }
    }
  }

  const ServerData& rServerData = xServer::getServerData();
  if (getServerType() == SERVER_TYPE_SESSION ||
    getServerType() == SERVER_TYPE_SCENE ||
    getServerType() == SERVER_TYPE_RECORD || 
    getServerType() == SERVER_TYPE_TEAM ||
    getServerType() == SERVER_TYPE_SOCIAL)
  {
    {
      const TIpPortPair* pPair = rServerData.getIpPort("StatServer");
      if (pPair)
        addClient(ClientType::stat_server, pPair->first, pPair->second);
    }

    if (xServer::isOuter())
    {
      addClient(ClientType::log_server, "127.0.0.1", 13100);
    }
    else
    {
      // addClient(ClientType::log_server, "127.0.0.1", 13100);
      // addClient(ClientType::log_server, "172.24.15.238", 13100);
    }
  }

  if (SERVER_TYPE_GATE == getServerType())
  {
    addClient(ClientType::plat_server, BaseConfig::getMe().getPlatIp(), BaseConfig::getMe().getPlatPort());
  }

  switch (getServerType())
  {
    case SERVER_TYPE_SUPER:
    case SERVER_TYPE_SESSION:
    case SERVER_TYPE_RECORD:
    case SERVER_TYPE_DATA:
      {
        addDataBase(REGION_DB, false);
      }
      break;
    case SERVER_TYPE_SCENE:
    case SERVER_TYPE_GATE:
      delDataBase();
      break;
    default:
      break;
  }

  return true;
}

bool ZoneServer::connect()
{
  bool ret = true;
  for (auto it : m_oServerConfig)
  {
    if (isConnectServerType(getServerTypeString(), it.first))
    {
      if (!checkConnectedServer(it.first))
      {
        if (!connectServerByType(it.first))
        {
          ret = false;
        }
      }
    }
  }
  return ret;
}

bool ZoneServer::checkConnect()
{
  for (auto it : m_oServerList)
  {
    for (auto iter : it.second)
    {
      if (iter.second == nullptr) return false;
      if (iter.second->getTask() == nullptr) return false;
    }
  }
  return true;
}

void ZoneServer::process()
{
  for (auto it : m_oServerList)
  {
    for (auto sub_it : it.second)
    {
      if (!sub_it.second) continue;
      if (!sub_it.second->getTask()) continue;

      xServer::doCmd(sub_it.second->getTask());
    }
  }
}

bool ZoneServer::doCmd(xNetProcessor* np, unsigned char* buf, unsigned short len)
{
  if (!np || !buf || !len || len < sizeof(xCommand)) return false;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->cmd)
  {
    case TEAM_PROTOCMD:
      return doTeamCmd(buf, len);
    case CLIENT_CMD:
      return doClientCmd(buf, len);
    case LOG_PROTOCMD:
      return doLogCmd(buf, len);
    case GMTOOLS_PROTOCMD:
      return doGmToolsCmd(np, buf, len);
    //case SCENE_SESSION_CMD:
    //  return doSessionSceneCmd(buf,len);
    case GATEWAY_CMD:
      return doGatewayCmd(buf,len);
    case Cmd::RECORD_DATA_PROTOCMD:
      return doRecordDataCmd(buf, len);
    case Cmd::TRADE_PROTOCMD:
      return doTradeCmd(buf, len);
    case Cmd::MATCHS_PROTOCMD:
      return doMatchCmd(buf, len);
    case Cmd::AUCTIONS_PROTOCMD:
      return doAuctionCmd(buf, len);
    case Cmd::WEDDINGS_PROTOCMD:
      return doWeddingCmd(buf, len);
    case Cmd::SOCIAL_PROTOCMD:
      return doSocialCmd(buf, len);
    case Cmd::GUILD_PROTOCMD:
      return doGuildCmd(buf, len);
    case Cmd::ERROR_USER_PROTOCMD:
      return doErrorUserCmd(buf, len);
    case Cmd::SESSION_PROTOCMD:
      return doSessionCmd(buf, len);
    /*case Cmd::ITEMS_PROTOCMD:
      return doItemSCmd(buf, len);*/
    case Cmd::GATE_SUPER_PROTOCMD:
      return doGateSuperCmd(np, buf, len);
    case Cmd::LOGIN_USER_PROTOCMD:
      return doLoginUserCmd(np, buf, len);
    case REG_CMD:
      return doRegCmd(np, buf, len);
    case BOSSS_PROTOCMD:
      return doBossSCmd(buf, len);
    case Cmd::SYSTEM_PROTOCMD:
      {
        using namespace Cmd;
        switch (cmd->param)
        {
          case REGIST_REGION_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(RegistRegionSystemCmd, message);

              onRegistRegion((ClientType)message.regiontype());
              XLOG << "[公共服连接]" << message.regiontype() << "注册成功" << XEND;

              return true;
            }
            break;
          case SERVER_INIT_OK_SYSCMD:
            {
              ServerTask *task = getServerTask(np);
              if (task)
              {
                initOkServer(task);
                XLOG << "[服务器连接]" << task->getName() << "初始化完成" << XEND;
              }

              return true;
            }
            break;
          case VERIFY_CONN_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(VerifyConnSystemCmd, message);

              if (message.ret())
              {
                if (SERVER_TYPE_SUPER==getServerType())
                {
                  DWORD port = getPort(message.type(), message.name());
                  if (!port) return true;
                  addServerConfig(message.type(), message.name(), getServerIP(), port);
                }

                if (!np->isClient()) np->setName(message.name().c_str()); //need

                verifyServer(np, message.type().c_str(), message.name().c_str());
              }

              return true;
            }
            break;
          case SERVER_LIST_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(ServerListSystemCmd, rev);

              XDBG << "[服务器列表]" << getServerTypeString() << getServerName() << XEND;
              for (int i = 0; i < rev.list_size(); ++i)
              {
                const ServerListSystemCmd_Item &item = rev.list(i);
                addServerConfig(item.type(), item.name(), item.ip(), item.port());
              }

              return true;
            }
            break;
          case SERVER_TIME_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(ServerTimeSystemCmd, rev);

              XDBG << "[系统时间],修正系统时间:" << rev.adjust() << XEND;

              xTime::setAdjust(rev.adjust());

              if (getServerType()==SERVER_TYPE_SUPER)
              {
                sendCmdToServer(buf, len, "SessionServer");
                sendCmdToServer(buf, len, "SceneServer");
                sendCmdToServer(buf, len, "GateServer");
                sendCmdToServer(buf, len, "RecordServer");
                sendCmdToServer(buf, len, "LogServer");
                sendCmdToServer(buf, len, "SocialServer");
              }

              return true;
            }
            break;
          case HEART_BEAT_SYSCMD:
            {
              if (getServerType()==SERVER_TYPE_SUPER)
              {
                ServerTask *task = getServerTask(np);
                if (task)
                {
                  task->reset();
                //  XLOG << "[心跳]" << task->getName() << XEND;
                }
              }
              else
              {
                np->sendCmd(buf, len);
                // XLOG << "[心跳]" << np->name << XEND;
              }
              return true;
            }
            break;
          case COMMON_RELOAD_SYSCMD:
          {
            PARSE_CMD_PROTOBUF(CommonReloadSystemCmd, message);
            commonReload(message);
            return true;
          }
          break;
          default:
            break;
        }
      }
  }

  return false;
}

void ZoneServer::sendVerify(xNetProcessor* np, bool ret)
{
  if (!np) return;
  using namespace Cmd;

  VerifyConnSystemCmd message;
  message.set_ret(ret);
  message.set_type(getServerTypeString());
  message.set_name(getServerName());

  PROTOBUF(message, send, len);
  np->sendCmd(send, len);
}

bool ZoneServer::verifyServer(xNetProcessor* np, const char* t, const char* n)
{
  if (!np) return false;

  bool ret = true;

  if (inVerifyList(np))
  {
    ServerTask *ta = getConnectedServer(t, n);
    if (ta)
    {
      if (ta->getTask())
      {
        XERR << "[服务器连接]" << t << n << "已经在ServerList,重复连接" << XEND;
        ret = false;
      }
    }
    else
    {
      if ((m_oServerConfig.find(t) == m_oServerConfig.end())
          || (m_oServerConfig[t].find(n) == m_oServerConfig[t].end()))
      {
        XERR << "[服务器连接]" << t << n << "不在ServerConfig中" << XEND;
        ret = false;
      }
    }
  }
  else
  {
    XERR << "[服务器连接]" << t << n << "不在VerifyList中" << XEND;
    ret = false;
  }

  if (np->isTask())
  {
    sendVerify(np, ret);
  }

  XLOG << "[服务器连接]" << t << n << "验证" << (ret ? "成功" : "失败") << XEND;
  if (ret)
  {
    np->setNpState(NPState::establish);

    removeVerifyList(np);

    if (m_oServerList.find(t) != m_oServerList.end())
    {
      if (m_oServerList[t].find(n) != m_oServerList[t].end())
      {
        m_oServerList[t][n]->setTask(np);
        verifyOk(np);
        XLOG << "[服务器连接]" << t << n << "添加到ServerList" << np << XEND;
      }
      else
      {
        XERR << "[服务器连接]" << t << n << "ServerList无名字" << XEND;
      }
    }
    else
    {
      XERR << "[服务器连接]" << t << n << "ServerList无类型" << XEND;
    }
  }
  else
  {
    addCloseList(np, TerminateMethod::terminate_active, "ZoneServer验证失败");
  }
  return ret;
}

void ZoneServer::verifyOk(xNetProcessor* np)
{
  v_verifyOk(np);

  if (SERVER_TYPE_SUPER==getServerType())
  {
    Cmd::ServerListSystemCmd message;
    for (auto it : m_oServerConfig)
    {
      for (auto iter : it.second)
      {
        Cmd::ServerListSystemCmd_Item *item = message.add_list();
        item->set_type(it.first);
        item->set_name(iter.first);
        item->set_ip(iter.second.ip);
        item->set_port(iter.second.port);
      }
    }
    PROTOBUF(message, send, len);
    std::vector<ServerTask *> list;
    if (getConnectedServer("", "", list))
    {
      for (auto &it : list)
      {
        it->sendCmd(send, len);
      }
    }
  }
  else
  {
    ServerTask *task = getServerTask(np);
    if (task)
    {
      task->setState(ServerTask_State::okay);
      XLOG << "[服务器连接]" << np->name << "连接成功" << XEND;
    }
    else
    {
      XERR << "[服务器连接]" << np->name << "初始化成功,未找到对应的ServerTask" << XEND;
    }
  }
}

// 连接完所有服务器后调用一次
void ZoneServer::init_ok()
{
  Cmd::ServerInitOkConnSystemCmd cmd;
  cmd.set_name(getServerName());
  PROTOBUF(cmd, send, len);
  sendCmdToServer(send, len, "");

  std::cout << "[" << getServerName() << "],可以开始游戏了" << std::endl;
  XLOG << "[服务器]" << "init_ok" << XEND;
}

//return first if more than one
ServerTask* ZoneServer::getConnectedServer(std::string t, std::string n)
{
  ServerTaskList tempList;

  if (t != "")
  {
    auto it = m_oServerList.find(t);
    // 没有类型t的
    if (it == m_oServerList.end()) return NULL;

    tempList.insert(*it);
  }
  else
  {
    // 全部类型
    tempList = m_oServerList;
  }

  for (auto type_it : tempList)
  {
    if (n != "")
    {
      auto task_it = type_it.second.find(n);
      if (task_it == type_it.second.end()) return NULL;
      if (task_it->second->getTask())
      {
        return task_it->second;
      }
    }
    else
    {
      for (auto task_it : type_it.second)
      {
        if (task_it.second->getTask())
        {
          return task_it.second;
        }
      }
    }
  }
  return NULL;
}

//return if have
bool ZoneServer::getConnectedServer(std::string t, std::string n, std::vector<ServerTask *> &list)
{
  ServerTaskList tempList;

  if (t != "")
  {
    auto it = m_oServerList.find(t);
    if (it == m_oServerList.end()) return false;
    tempList.insert(*it);
  }
  else
  {
    tempList = m_oServerList;
  }

  for (auto type_it : tempList)
  {
    if (n != "")
    {
      auto task_it = type_it.second.find(n);
      if (task_it != type_it.second.end() && task_it->second->getTask())
      {
        list.push_back(task_it->second);
      }
    }
    else
    {
      for (auto task_it : type_it.second)
      {
        if (task_it.second->getTask())
        {
          list.push_back(task_it.second);
        }
      }
    }
  }
  return !list.empty();
}

ServerTask* ZoneServer::getServerTask(xNetProcessor *np)
{
  if (!np) return NULL;

  for (auto it : m_oServerList)
  {
    for (auto iter : it.second)
    {
      if (iter.second == nullptr)
        continue;
      if (np == iter.second->getTask())
        return iter.second;
    }
  }
  return NULL;
}

bool ZoneServer::checkConnectedServer(std::string t, std::string n)
{
  if ("" == t) return false;

  auto it = m_oServerList.find(t);
  if (it == m_oServerList.end())
    return false;

  if (it->second.empty())
    return false;

  if ("" != n)
  {
    auto iter = it->second.find(n);
    if (iter == it->second.end())
      return false;

    return nullptr != iter->second->getTask();
  }

  for (auto iter : it->second)
  {
    if (!iter.second->getTask()) return false;
  }

  return true;
}

// return all connected ok
bool ZoneServer::connectServerByType(std::string t)
{
  ServerConfigList tempList;

  auto it = m_oServerConfig.find(t);
  if (it == m_oServerConfig.end()) return false;
  tempList.insert(*it);

  if (t != "")
  {
    auto it = m_oServerConfig.find(t);
    if (it == m_oServerConfig.end()) return false;
    tempList.insert(*it);
  }
  else
  {
    tempList = m_oServerConfig;
  }

  bool ret = true;
  for (auto type_it : tempList)
  {
    for (auto s_it : type_it.second)
    {
      if (getConnectedServer(type_it.first, s_it.first))
        continue;

      ret = false;

      xNetProcessor* c = newClient();
      if (!c)
      {
        XERR << "[服务器连接]" << "connectServerByType" << t << "newClient failed" << XEND;
        continue;
      }
      c->setName(s_it.first.c_str());

      if (!c->connect(s_it.second.ip.c_str(), s_it.second.port))
      {
        XERR << "[服务器连接]" << "connectServerByType" << t << "failed" << s_it.second.ip << ":" << s_it.second.port << "," << strerror(errno) << XEND;
        SAFE_DELETE(c);
        continue;
      }

      setVerifyList(c, time(0));

      m_oTaskThreadPool.add(c);

      XDBG << "[服务器连接]" << "连接" << t << "成功" << s_it.second.ip << ":" << s_it.second.port << c << XEND;

      sendVerify(c, true);
    }
  }
  return ret;
}

const char *ZoneServer::getServerIP()
{
  auto type_it = m_oServerConfig.find(getServerTypeString());
  if (type_it != m_oServerConfig.end())
  {
    auto name_it = type_it->second.find(getServerName());
    if (name_it != type_it->second.end())
      return name_it->second.ip.c_str();
  }
  return "";
}

int ZoneServer::getServerPort()
{
  auto type_it = m_oServerConfig.find(getServerTypeString());
  if (type_it != m_oServerConfig.end())
  {
    auto name_it = type_it->second.find(getServerName());
    if (name_it != type_it->second.end())
      return name_it->second.port;
  }
  return 0;
}

INT ZoneServer::getAServerIP(const char* type, const char *name)
{
  auto type_it = m_oServerConfig.find(type);
  if (type_it != m_oServerConfig.end())
  {
    auto name_it = type_it->second.find(name);
    if (name_it != type_it->second.end())
      return inet_addr(name_it->second.ip.c_str());
  }
  return 0;
}

INT ZoneServer::getAServerPort(const char* type, const char *name)
{
  auto type_it = m_oServerConfig.find(type);
  if (type_it != m_oServerConfig.end())
  {
    auto name_it = type_it->second.find(name);
    if (name_it != type_it->second.end())
      return name_it->second.port;
  }
  return 0;
}

DWORD ZoneServer::getPort(std::string type, std::string name)
{
  DWORD base = getAServerPort("SuperServer", "SuperServer");
  if (!base) return 0;

  if (type == "SuperServer")
  {
    return base;
  }
  else if (type == "TeamServer")
  {
    return base + ZoneServerPortIndex::TeamServerIndex;
  }
  else if (type == "SocialServer")
  {
    return base + ZoneServerPortIndex::SocialServerIndex;
  }
  else if (type == "SessionServer")
  {
    return base + ZoneServerPortIndex::SessionServerIndex;
  }
  else if (type == "SceneServer")
  {
    DWORD index = atoi(name.c_str() + 11);
    if (!index) return 0;
    index--;
    if (index <= (ZoneServerPortIndex::SceneServerMax - ZoneServerPortIndex::SceneServerMin))
    {
      return base + ZoneServerPortIndex::SceneServerMin + index;
    }
  }
  else if (type == "GateServer")
  {
    DWORD index = atoi(name.c_str() + 10);
    if (!index) return 0;
    index--;
    if (index <= (ZoneServerPortIndex::GateServerMax - ZoneServerPortIndex::GateServerMin))
    {
      return base + ZoneServerPortIndex::GateServerMin + index;
    }
  }
  else if (type == "LogServer")
  {
    return base + ZoneServerPortIndex::LogServerIndex;
  }
  else if (type == "RecordServer")
  {
    return base + ZoneServerPortIndex::RecordServerIndex;
  }
  else if (type == "DataServer")
  {
    return base + ZoneServerPortIndex::DataServerIndex;
  }

  return 0;
}

void ZoneServer::addServerConfig(std::string type, std::string name, std::string ip, DWORD port)
{
  auto it = m_oServerConfig.find(type);
  if (it != m_oServerConfig.end())
  {
    auto iter = it->second.find(name);
    if (iter != it->second.end())
    {
      XDBG << "[服务器连接]" << "重复添加" << type << name << ip << port << XEND;
      return;
    }
  }
  ServerConfig &item = m_oServerConfig[type][name];
  item.ip = ip;
  item.port = port;
  XLOG << "[服务器连接]" << type << name << "添加ServerConfig" << ip << port << XEND;

  if (isConnectServerType(type, getServerTypeString()) || isConnectServerType(getServerTypeString(), type))
  {
    if (!getConnectedServer(type, name))
    {
      if (!m_oServerList[type][name])
      {
        m_oServerList[type][name] = new ServerTask();
        XLOG << "[服务器连接]" << type << name << "ServerList设置要连接的服务器" << XEND;
      }
    }
  }
}

bool ZoneServer::isConnectServerType(std::string f, std::string t)
{
  auto it = m_oConnectConfig.find(f);
  if (it == m_oConnectConfig.end())
  {
    return false;
  }
  return it->second.find(t) != it->second.end();
}

bool ZoneServer::sendCmdToServer(const void* data, unsigned short len, const char* type, const char* name)
{
  ServerTaskVec list;
  if (!getConnectedServer(type, name != NULL ? name : "", list))
    return false;

  for (auto &it : list)
  {
    it->sendCmd(data, len);
  }
  return true;
}

bool ZoneServer::sendCmdToOtherServer(const void* data, unsigned short len, const char* type, const char* exclude)
{
  ServerTaskVec list;
  if (!getConnectedServer(type, "", list))
    return false;

  for (auto &it : list)
  {
    if (it->isValid() && strcmp(it->getName(), exclude) != 0)
      it->sendCmd(data, len);
  }
  return true;
}
