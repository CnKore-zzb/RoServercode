#include <iostream>
#include "TradeServer.h"
#include "TradeManager.h"
#include "xNetProcessor.h"
#include "TradeManager.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RegCmd.h"
#include "CommonConfig.h"
#include "TradeUserMgr.h"

TradeServer::TradeServer(OptArgs &args):RegionServer(args)
{
}

TradeServer::~TradeServer()
{
}

void TradeServer::v_final()
{
  XLOG << "[" << getServerName() << "],v_final" << XEND;
  RegionServer::v_final();
}

void TradeServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  RegionServer::v_closeNp(np);
}

void TradeServer::v_timetick()
{
  RegionServer::v_timetick();
 
  DWORD curTime = xTime::getCurSec();
  TradeManager::getMe().timeTick(curTime);
}

bool TradeServer::v_init()
{
  if (!addDataBase(REGION_DB, true))
  {
    XERR << "[数据库Trade],初始化数据库连接失败:" << REGION_DB << XEND;
    return false;
  }
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("TradeServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， TradeServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);

  const TIpPortPair* pStatPair = rServerData.getIpPort("StatServer");
  if (pStatPair)
  {
    addClient(ClientType::stat_server, pStatPair->first, pStatPair->second);
  }
  if (!loadConfig())
    return false;

  TradeManager::getMe().init();

  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }
  return true;
}

bool TradeServer::loadConfig()
{
  bool bResult = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }
  // base配置
  if (ConfigManager::getMe().loadTradeConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool TradeServer::doTradeCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    case SESSION_FORWARD_USERCMD_TRADE:
      {
        if (!CommonConfig::getMe().IsTradeServerOpen())
        {
          XERR << "[交易] 交易所服已关闭" << XEND;
          return true;
        }
        PARSE_CMD_PROTOBUF(Cmd::SessionForwardUsercmdTrade, rev);
        return TradeManager::getMe().doTradeUserCmd(rev.charid(), rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len());
      }
      break;
    case SESSION_FORWARD_SCENECMD_TRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::SessionForwardScenecmdTrade, rev);
        return TradeManager::getMe().doTradeServerCmd(rev.charid(), rev.name(), rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len());
      }
      break;
    case SECURITY_CMD_RECORDTRADE:
    {
      PARSE_CMD_PROTOBUF(Cmd::SecurityCmdSceneTradeCmd, rev);
      if (rev.valid() == true)
      {
        TradeManager::getMe().addOneSecurity(rev);
      }
      else
      {
        TradeManager::getMe().delOneSecurity(rev);
      }
      XLOG << "[交易所-收到安全指令] charid" << rev.charid() << "valid" << rev.valid() << rev.ShortDebugString() << XEND;
      return true;
    }
  }
  return false;
}

bool TradeServer::doRegCmd(xNetProcessor *np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;
  RegCmd *cmd = (RegCmd *)buf;

  switch (cmd->param)
  {
  case RET_DELETE_CHAR_REGCMD:
  {
    RetDeleteCharRegCmd* rev = (RetDeleteCharRegCmd*)cmd;
    TradeManager::getMe().delChar(rev->id);
    return true;
  }
  break;
  }
 
  return false;

}

bool TradeServer::doSocialCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
  case SOCIALPARAM_ONLINESTATUS:
  {
    PARSE_CMD_PROTOBUF(OnlineStatusSocialCmd, rev);
    if (rev.online())
      TradeUserMgr::getMe().onUserOnline(rev.user());
    else
      TradeUserMgr::getMe().onUserOffline(rev.user());
  }
  return true;
  default:
    return false;
  }

  return false;
}
