#include <iostream>
#include "AuctionServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RegCmd.h"
#include "AuctionSCmd.pb.h"
#include "CommonConfig.h"
#include "SceneTrade.pb.h"
#include "AuctionManager.h"

extern ConfigEnum ConfigEnums[];

AuctionServer::AuctionServer(OptArgs &args):RegionServer(args)
{
}

AuctionServer::~AuctionServer()
{
}

void AuctionServer::v_final()
{
  XLOG << "[" << getServerName() << "],v_final" << XEND;
  RegionServer::v_final();
}

void AuctionServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  RegionServer::v_closeNp(np);
}

void AuctionServer::v_timetick()
{
  RegionServer::v_timetick();
 
  DWORD curTime = xTime::getCurSec();
  AuctionManager::getMe().timeTick(curTime);
}

bool AuctionServer::v_init()
{
  if (!addDataBase(REGION_DB, true))
  {
    XERR << "[数据库Trade],初始化数据库连接失败:" << REGION_DB << XEND;
    return false;
  }
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("AuctionServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， AuctionServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);

  if (!loadConfig())
    return false;

  AuctionManager::getMe().init();

  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }
  return true;
}

bool AuctionServer::loadConfig()
{
  bool bResult = true;

  ConfigEnums[item].loadfunc = []()
  {
    return AuctionManager::getMe().loadConfig();
  };


  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }
  // base配置
  if (ConfigManager::getMe().loadAuctionConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool AuctionServer::sendCmdToClient(QWORD charId, DWORD zoneId, void* buf, WORD len)
{
  Cmd::SessionToMeRecordTrade cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);
  return thisServer->sendCmdToZone(zoneId, send, len2);
}

bool AuctionServer::forwardCmdToSceneServer(QWORD charId, DWORD zoneId, void* buf, WORD len)
{
  Cmd::ForwardAuction2SCmd cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToZone(zoneId, send2, len2);
}

bool AuctionServer::forwardCmdToTradeServer(DWORD zoneId, void* buf, WORD len)
{
  Cmd::SessionForwardScenecmdTrade cmd;
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToZone(zoneId, send2, len2);
}

void AuctionServer::sendMsg(QWORD charId, DWORD zoneId, DWORD msgid, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/)
{
  if (charId == 0)
    return;
  SysMsg cmd;
  mutableMsg(cmd, msgid, params, eType, eAct);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(charId, zoneId, send, len);
}

void AuctionServer::mutableMsg(SysMsg &cmd, DWORD msgid, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/)
{
  cmd.set_id(msgid);
  cmd.set_type(eType);
  cmd.set_act(eAct);
  params.toData(cmd);
}
