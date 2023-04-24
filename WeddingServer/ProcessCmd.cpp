#include <iostream>
#include "WeddingServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RegCmd.h"
#include "WeddingManager.h"
#include "CommonConfig.h"
#include "SocialCmd.pb.h"
#include "AuctionCCmd.pb.h"
#include "WeddingSCmd.pb.h"
#include "WeddingUserMgr.h"

extern xLog srvLog;

bool WeddingServer::doWeddingCmd(BYTE* buf, WORD len)
{
  FUN_TRACE();
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
  case WEDDINGSPARAM_FORWARD_C2WEDDING:
  {                         
    PARSE_CMD_PROTOBUF(Cmd::ForwardC2WeddingSCmd, rev);
    WeddingManager::getMe().doUserCmd(rev.charid(), rev.name(),rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len());
    return true;
  }
  break;
  case WEDDINGSPARAM_FORWARD_S2WEDDING:
  {
    PARSE_CMD_PROTOBUF(Cmd::ForwardS2WeddingSCmd, rev);
    WeddingManager::getMe().doServerCmd(rev.charid(), rev.name(), rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len());
    return true;
  }
  break;
  default:
    break;
  }
  return true;
}

bool WeddingServer::doSocialCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return true;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
  case SOCIALPARAM_ONLINESTATUS:
  {
    PARSE_CMD_PROTOBUF(OnlineStatusSocialCmd, rev);
    if (rev.online())
      WeddingUserMgr::getMe().onUserOnline(rev.user());
    else
      WeddingUserMgr::getMe().onUserOffline(rev.user());
  }
  return true;
  default:
    return true;
  }

  return true;
}

bool WeddingServer::doSessionCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
  case LOAD_LUA_SESSIONCMD:
  {
    PARSE_CMD_PROTOBUF(LoadLuaSessionCmd, message);
    if (message.has_lua() == true)
    {
      LuaManager::getMe().reload();
    }
    if (message.has_log() == true)
    {
      srvLog.reload();
    }
    if (message.has_table())
    {
      string str = message.table();
      TVecConfigType vec;
      ConfigManager::getMe().getType(str, vec);
      for (auto &it : vec)
      {
        ConfigEnum cfg;
        if (ConfigManager::getMe().getConfig(it, cfg))
        {
          if (!cfg.isReload())
            continue;
          if (cfg.isTypeLoad(WEDDING_LOAD))
            ConfigManager::getMe().loadConfig(cfg);
        }
      }     
    }

    XINF << "[策划表-重加载] zoneid:" << thisServer->getZoneID() << "source serverid" << message.serverid() << "lua:" << message.lua() << "table:" << message.table() << "log:" << message.log() << "allzone:" << message.allzone() << XEND;
    return true;
  }
  default:
    return true;
  }

  return true;
}

bool WeddingServer::doRegCmd(xNetProcessor *np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;
  RegCmd *cmd = (RegCmd *)buf;

  switch (cmd->param)
  {
  case RET_DELETE_CHAR_REGCMD:
  {
    RetDeleteCharRegCmd* rev = (RetDeleteCharRegCmd*)cmd;
    WeddingManager::getMe().onDelChar(rev->id);
    return true;
  }
  break;
  }

  return false;

}

