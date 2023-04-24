#include "GProxyServer.h"
#include "xDBConnPool.h"
#include "xNetProcessor.h"
#include "RedisManager.h"
#include "CommonConfig.h"

GProxyServer::GProxyServer(OptArgs &args) : xServer(args), m_oOneMinTimer(60)
{
}

void GProxyServer::v_final()
{
}

bool GProxyServer::init()
{
  if (!addDataBase(RO_DATABASE_NAME, false))
  {
    XERR << "[加载RO信息],初始化数据库连接失败:" << RO_DATABASE_NAME << XEND;
    return false;
  }
  XLOG << "[加载RO信息],加载数据库" << RO_DATABASE_NAME << XEND;

  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "proxy"); // 启动加载
  if (field)
  {
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "id = 0");

    xRecordSet set;
    QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX != ret)
    {
      if (ret)
      {
        setServerPort(set[0].get<DWORD>("port"));
        XLOG << "[加载Proxy信息],加载port:" << set[0].get<DWORD>("port") << XEND;
      }
    }
    else
    {
      return false;
    }
  }

  if (!listen())
  {
    XERR << "[" << getServerName() << "],监听 Zone 失败" << XEND;
    return false;
  }

  {
    const xLuaData& data = getBranchConfig().getData("Redis");
    RedisManager::getMe().init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port"));
  }

  return true;
}


void GProxyServer::v_closeNp(xNetProcessor* np)
{
  if (!np) return;

  for (auto it = m_oProxyList.begin(); it != m_oProxyList.end(); ++it)
  {
    if (it->second.m_pNetProcessor == np)
    {
      XLOG << "[连接]" << "删除" << it->first << np << XEND;
      m_oProxyList.erase(it);
      break;
    }
  }
}

bool GProxyServer::v_callback()
{
  switch (getServerState())
  {
    case ServerState::create:
      {
        setServerState(ServerState::init);

        return true;
      }
      break;
    case ServerState::init:
      {
        if (!init()) stop();

        setServerState(ServerState::run);

        return true;
      }
      break;
    case ServerState::run:
      {
        return true;
      }
      break;
    case ServerState::stop:
    case ServerState::finish:
      {
        return false;
      }
      break;
    default:
      break;
  }
  return false;
}

void GProxyServer::v_timetick()
{
  xTime oFrameTimer;

  xServer::v_timetick();

  QWORD _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-timetick]," << _e << " 微秒" << XEND;

  oFrameTimer.elapseStart();
  process();
  _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-process]," << _e << " 微秒" << XEND;

  if (m_oOneMinTimer.timeUp(now()))
  {
    print();
  }
}

void GProxyServer::process()
{
  for (auto &it : m_oProxyList)
  {
    xNetProcessor *np = it.second.m_pNetProcessor;
    CmdPair *cmd = np->getCmd();
    while (cmd)
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
      np->popCmd();
      cmd = np->getCmd();
    }
  }
}

void GProxyServer::print()
{
  DWORD total = 0;
  std::map<DWORD, DWORD> groupCount;
  std::map<DWORD, DWORD> portCount;
  for (auto &it : m_oProxyList)
  {
    DWORD groupid = atoi(it.first.c_str()) / 10000 % 10;
    DWORD port = atoi(it.first.c_str()) / 100000;

    total += it.second.m_dwTaskNum;
    groupCount[groupid] += it.second.m_dwTaskNum;
    portCount[port] += it.second.m_dwTaskNum;
    XLOG << "[连接统计]" << "id:" << it.first << "连接数:" << it.second.m_dwTaskNum << XEND;
  }

  for (auto &it : groupCount)
  {
    XLOG << "[分组连接统计]" << "group:" << it.first << "连接数:" << it.second << XEND;
  }
  for (auto &it : portCount)
  {
    XLOG << "[端口连接统计]" << "port:" << it.first << "连接数:" << it.second << XEND;
  }

  XLOG << "[总连接统计]" << "连接数:" << total << XEND;
}
