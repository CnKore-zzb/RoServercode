#include <iostream>
#include "AuctionServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RegCmd.h"
#include "AuctionSCmd.pb.h"
#include "AuctionManager.h"
#include "CommonConfig.h"
#include "SocialCmd.pb.h"
#include "AuctionCCmd.pb.h"

extern xLog srvLog;

bool AuctionServer::doAuctionCmd(BYTE* buf, WORD len)
{
  FUN_TRACE();
  if (!buf || !len) return false;
  using namespace Cmd;
  xCommand* cmd = (xCommand*)buf;

  //XDBG << "[拍卖行-收到消息] " << cmd->cmd << cmd->param << XEND;

  switch (cmd->param)
  {
  case AUCTIONSPARAM_FORWARD_CCMD2AUCTION:
  {                         
    PARSE_CMD_PROTOBUF(Cmd::ForwardCCmd2Auction, rev);
    AuctionManager::getMe().doUserCmd(rev.charid(), rev.name(),rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len());
    return true;
  }
  case AUCTIONSPARAM_FORWARD_SCMD2AUCTION:
  {
    PARSE_CMD_PROTOBUF(Cmd::ForwardSCmd2Auction, rev);
    AuctionManager::getMe().doServerCmd(rev.charid(), rev.name(), rev.zoneid(), (const BYTE*)rev.data().c_str(), rev.len());
    return true;
  }
  case AUCTIONSPARAM_GM_MODIFY_AUCTION_TIME:
  {
    PARSE_CMD_PROTOBUF(Cmd::GmModifyAuctionTimeSCmd, rev);    
    AuctionManager::getMe().modifyAuctionTime(rev.auction_time());
    return true;
  }
  case AUCTIONSPARAM_GM_STOP_AUCTION:
  {
    AuctionManager::getMe().stopAuction();
    return true;
  }
  }
  return false;
}

bool AuctionServer::doSocialCmd(BYTE* buf, WORD len)
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
      AuctionManager::getMe().onUserOnline(rev.user());
    else
      AuctionManager::getMe().onUserOffline(rev.user());
  }
  return true;
  default:
    return false;
  }

  return false;
}
bool AuctionServer::doSessionCmd(BYTE* buf, WORD len)
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
          if (cfg.isTypeLoad(AUCTION_LOAD))
            ConfigManager::getMe().loadConfig(cfg);
        }
      }     
    }

    XINF << "[策划表-重加载] zoneid:" << thisServer->getZoneID() << "source serverid" << message.serverid() << "lua:" << message.lua() << "table:" << message.table() << "log:" << message.log() << "allzone:" << message.allzone() << XEND;
    return true;
  }
  default:
    return false;
  }

  return false;
}

bool AuctionServer::doTradeCmd(BYTE* buf, WORD len)
{
  if(!buf || !len) return false;
  using namespace Cmd;

  xCommand *cmd = (xCommand*)buf;
  switch(cmd->param)
  {
    case TRADE_PRICE_QUERY_RECORDTRADE:
    {
      PARSE_CMD_PROTOBUF(TradePriceQueryTradeCmd, rev);
      AuctionManager::getMe().updateSignupPrice(rev.batchid(), rev.signup_id(), rev.price());
      return true;
    }
    break;
    default:
      return false;
  }

  return true;
}

