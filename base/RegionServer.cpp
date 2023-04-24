#include <signal.h>
#include "RegionServer.h"
#include "xDBConnPool.h"
#include "xNetProcessor.h"
#include "SystemCmd.pb.h"

RegionServer::RegionServer(OptArgs &args) : xServer(args)
{
}

void RegionServer::collectZoneIDs(TSetDWORD& setIDs) const
{
  setIDs.clear();
  for (auto &m : m_oZoneList)
    setIDs.insert(m.first);
}

void RegionServer::v_final()
{
  for (auto &it : m_oZoneList)
  {
    addCloseList(it.second, TerminateMethod::terminate_active, "停服关闭");
  }
  m_oZoneList.clear();
}

bool RegionServer::v_callback()
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

void RegionServer::v_timetick()
{
  m_blAcceptZone = true;
  m_oFrameTime.elapseStart();

  xServer::v_timetick();

  process();
}

void RegionServer::v_closeNp(xNetProcessor* np)
{
  if (!np) return;

  for (auto it = m_oZoneList.begin(); it != m_oZoneList.end(); ++it)
  {
    if (it->second == np)
    {
      v_zoneDel(it->first);
      m_oZoneList.erase(it);
      XLOG << "[连接]" << "删除" << np << XEND;
      break;
    }
  }
}

bool RegionServer::init()
{
  if (!addDataBase(RO_DATABASE_NAME, false))
  {
    XERR << "[加载RO信息],初始化数据库连接失败:" << RO_DATABASE_NAME << XEND;
    return false;
  }
  XLOG << "[加载RO信息],加载数据库" << RO_DATABASE_NAME << XEND;

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

  addDataBase(getRegionDBName(), false);

  if (getServerType() == SERVER_TYPE_TRADE ||
    getServerType() == SERVER_TYPE_GLOBAL || 
    getServerType() == SERVER_TYPE_TEAM ||
    getServerType() == SERVER_TYPE_GUILD)
  {
    if (xServer::isOuter())
    {
      addClient(ClientType::log_server, "127.0.0.1", 13100);
    }
    else
    {
      //addClient(ClientType::log_server, "127.0.0.1", 13100);
      // addClient(ClientType::log_server, "172.24.15.238", 13100);
    }
  }

  return v_init();
}

void RegionServer::process()
{
  for (auto &it : m_oZoneList)
  {
    xNetProcessor *np = it.second;
    CmdPair *cmd = np->getCmd();
    while (cmd)
    {
      m_oMessageStat.start(((xCommand *)cmd->second)->cmd, ((xCommand *)cmd->second)->param);
      if (!doRegionCmd(np, cmd->second, cmd->first))
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

bool RegionServer::doCmd(xNetProcessor* np, unsigned char* buf, unsigned short len)
{
  if (!np || !buf || !len || len < sizeof(xCommand)) return false;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->cmd)
  {
    case Cmd::STAT_PROTOCMD:
      {
        return doStatCmd(np, buf, len);
      }
      break;
    case Cmd::SYSTEM_PROTOCMD:
      {
        switch (cmd->param)
        {
          case REGIST_REGION_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(RegistRegionSystemCmd, message);

              if (regist(np, message.zoneid(), message.servertype()))
              {
                if (message.client())
                {
                  message.set_client(0);
                  PROTOBUF(message, send, slen);
                  np->sendCmd(send, slen);
                }
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
      break;
    default:
      break;
  }
  return false;
}

bool RegionServer::doRegionCmd(xNetProcessor *np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len || len < sizeof(xCommand)) return false;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->cmd)
  {
    case Cmd::SYSTEM_PROTOCMD:
      {
        switch (cmd->param)
        {
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
      break;
    case Cmd::GZONE_PROTOCMD:
      {
        return doGZoneCmd(buf, len);
      }
      break;
    case Cmd::TEAM_PROTOCMD:
      {
        return doTeamCmd(buf, len);
      }
      break;
    case REG_CMD:
      {
        return doRegCmd(np, buf, len);
      }
    case Cmd::SOCIAL_PROTOCMD:
      {
        return doSocialCmd(buf, len);
      }
      break;
    case Cmd::STAT_PROTOCMD:
      {
        return doStatCmd(np, buf, len);
      }
      break;
    case Cmd::TRADE_PROTOCMD:
      {
        return doTradeCmd(buf, len);
      }
      break;
    case Cmd::GUILD_PROTOCMD:
      {
        return doGuildCmd(buf, len);
      }
      break;
    case Cmd::MATCHS_PROTOCMD:
      {
        return doMatchCmd(buf, len);
      }
      break;
    case Cmd::AUCTIONS_PROTOCMD:
      {
        return doAuctionCmd(buf, len);
      }
      break;
    case Cmd::WEDDINGS_PROTOCMD:
      return doWeddingCmd(buf, len);
    case Cmd::SESSION_PROTOCMD:
      return doSessionCmd(buf, len);
    case Cmd::BOSSS_PROTOCMD:
      return doBossSCmd(buf, len);
    default:
      break;
  }
  return false;
}

bool RegionServer::sendCmdToZone(DWORD zoneid, const void* data, unsigned short len)
{
  auto it = m_oZoneList.find(zoneid);
  if (it == m_oZoneList.end())
  {
    XERR << "[发送消息-异常] could not find zoneid processer, zoneid:" << zoneid << XEND;
    return false;
  }

  if (!it->second)
  {
    XERR << "[发送消息-异常] zoneid processer is null, zoneid:" << zoneid << XEND;
    return false;
  }
  return it->second->sendCmd(data, len);
}

bool RegionServer::sendCmdToOneZone(const void* data, unsigned short len)
{
  if (m_oZoneList.empty())
    return false;
  if (m_oZoneList.begin()->second == nullptr)
    return false;
  
  return m_oZoneList.begin()->second->sendCmd(data, len);
}

bool RegionServer::sendCmdToAllZone(const void* data, unsigned short len, DWORD except/* = 0*/)
{
  for (auto it = m_oZoneList.begin(); it != m_oZoneList.end(); ++it)
  {
    if (!it->second)
    {
      XERR << "[发送消息-异常] zoneid processer is null, zoneid:" << it->first << XEND;
      continue;
    }
    if (except && it->first == except)
      continue;
    it->second->sendCmd(data, len);
  }
  return true;
}

bool RegionServer::regist(xNetProcessor* np, DWORD zoneid, DWORD serverType)
{
  if (!np) return false;

  if (!inVerifyList(np))
  {
    return false;
  }

  if (getServerType() ==  SERVER_TYPE_GLOBAL && (!m_blAcceptZone || m_oFrameTime.uElapse() > 2000000))
  {
    return false;
  }

  removeVerifyList(np);

  switch (serverType)
  {
    case SERVER_TYPE_STAT:
      {
        m_oClientManager.add(ClientType::stat_server, np);
        XLOG << "[服务注册]" << (DWORD)ClientType::stat_server << np << XEND;
        return true;
      }
      break;
    case SERVER_TYPE_TRADE:
      {
        m_oClientManager.add(ClientType::trade_server, np);
        XLOG << "[服务注册]" << (DWORD)ClientType::trade_server << np << XEND;
        return true;
      }
      break;
    case SERVER_TYPE_GLOBAL:
      {
        m_oClientManager.add(ClientType::global_server, np);
        XLOG << "[服务注册]" << (DWORD)ClientType::global_server << np << XEND;
        return true;
      }
      break;
    case SERVER_TYPE_GUILD:
      {
        m_oClientManager.add(ClientType::guild_server, np);
        XLOG << "[服务注册]" << (DWORD)ClientType::guild_server << np << XEND;
      }
      break;
    case SERVER_TYPE_MATCH:
    {
      m_oClientManager.add(ClientType::match_server, np);
      XLOG << "[服务注册]" << (DWORD)ClientType::match_server << np << XEND;
      return true;
    }
    break;
    case SERVER_TYPE_PROXY:
      break;
    default:
      {
        if (zoneid)
        {
          auto it = m_oZoneList.find(zoneid);
          if (it != m_oZoneList.end())
          {
            XERR << "[区注册]" << zoneid << np << "重复添加" << XEND;
            addCloseList(np, TerminateMethod::terminate_active, "区注册重复添加");
            return false;
          }
          else
          {
            m_oZoneList[zoneid] = np;
            onSessionRegist(np, zoneid);
          }
          m_blAcceptZone = false;
          XLOG << "[区注册]" << zoneid << np << XEND;
          return true;
        }
      }
  }
  return false;
}
