#include <iostream>
#include "GlobalServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
//#include "SocialityManager.h"
#include "SocialCmd.pb.h"
//#include "DojoMgr.h"
#include "RedisManager.h"
#include "GlobalManager.h"

extern ConfigEnum ConfigEnums[];

GlobalServer::GlobalServer(OptArgs &args):RegionServer(args),m_oOneSecTimer(1),m_oTenSecTimer(10)
{
}

GlobalServer::~GlobalServer()
{
}

void GlobalServer::v_final()
{
  XLOG << "[GlobalServer] v_final" << XEND;
  RegionServer::v_final();
  //SocialityManager::getMe().final();
  //GuildManager::getMe().final();
}

void GlobalServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  RegionServer::v_closeNp(np);
}

void GlobalServer::v_timetick()
{
  RegionServer::v_timetick();

  DWORD curTime = now();
  if (m_oOneSecTimer.timeUp(curTime))
  {
    xTime frameTimer;
    QWORD _e = 0;

    if (m_oTenSecTimer.timeUp(curTime))
    {
      //SocialityManager::getMe().timer(curTime);
      _e = frameTimer.uElapse();
      if (_e > 30 * 1000)
        XLOG << "[帧耗时]" << "social" << _e << " 微秒" << XEND;
    }

    frameTimer.elapseStart();
    //GuildManager::getMe().timer(curTime);
    _e = frameTimer.uElapse();
    if (_e > 30 * 1000)
      XLOG << "[帧耗时]" << "guild" << _e << " 微秒" << XEND;
  }
}

bool GlobalServer::v_init()
{
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("GlobalServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， GlobalServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);

  const TIpPortPair* pStatPair = rServerData.getIpPort("StatServer");
  if (pStatPair)
    addClient(ClientType::stat_server, pStatPair->first, pStatPair->second);

  const TIpPortPair* pGuildPair = rServerData.getIpPort("GuildServer");
  if (pGuildPair)
    addClient(ClientType::guild_server, pGuildPair->first, pGuildPair->second);

  if (!loadConfig())
    return false;
  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }

  return true;
}

bool GlobalServer::loadConfig()
{
  bool bResult = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }

  const xLuaData& data = getBranchConfig().getData("Redis");
  if (RedisManager::getMe().init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port")) == false)
  {
    XERR << "[RedisManager]" << "init error!" << XEND;
    bResult = false;
  }

  // config
  if (ConfigManager::getMe().loadSocialConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  // 数据
  /*if (!SocialityManager::getMe().init())
  {
    XERR << "[SocialityManager]" << "init error!" << XEND;
    bResult = false;
  }*/
  if (GlobalManager::getMe().loadGlobalData() == false)
  {
    XERR << "[GlobalData] load error!" << XEND;
    bResult = false;
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

void GlobalServer::updateGlobalBoss(const Cmd::DeadBossInfo& rInfo)
{
  if (GlobalManager::getMe().updateGlobalBoss(rInfo) == false)
    return;

  DeadBossOpenSyncBossSCmd scmd;
  scmd.mutable_info()->CopyFrom(rInfo);
  PROTOBUF(scmd, ssend, slen);
  bool bSuccess = thisServer->sendCmdToAllZone(ssend, slen);

  thisServer->sendWorldMsg(26114, MsgParams(rInfo.name()));
  XLOG << "[世界boss-变更] 变更数据" << rInfo.ShortDebugString() << "成功,同步到全线" << (bSuccess ? "成功" : "失败") << XEND;
}

bool GlobalServer::sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GlobalServer::sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToUserSceneSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GlobalServer::sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSceneUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GlobalServer::sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSessionUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GlobalServer::sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params /*= MsgParams()*/)
{
  SysMsg cmd;
  cmd.set_id(msgid);
  cmd.set_type(EMESSAGETYPE_FRAME);
  cmd.set_act(EMESSAGEACT_ADD);
  cmd.set_delay(0);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  return sendCmdToMe(zoneid, charid, send, len);
}

bool GlobalServer::sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params /*= MsgParams()*/)
{
  ChatWorldMsgSocialCmd cmd;
  cmd.mutable_msg()->set_id(msgid);
  cmd.mutable_msg()->set_type(EMESSAGETYPE_FRAME);
  cmd.mutable_msg()->set_act(EMESSAGEACT_ADD);
  params.toData(*cmd.mutable_msg());

  PROTOBUF(cmd, send, len);
  return sendCmdToZone(zoneid, send, len);
}

bool GlobalServer::sendWorldMsg(DWORD msgid, MsgParams params /*= MsgParams()*/)
{
  ChatWorldMsgSocialCmd cmd;
  cmd.mutable_msg()->set_id(msgid);
  cmd.mutable_msg()->set_type(EMESSAGETYPE_FRAME);
  cmd.mutable_msg()->set_act(EMESSAGEACT_ADD);
  params.toData(*cmd.mutable_msg());

  PROTOBUF(cmd, send, len);
  return sendCmdToAllZone(send, len);
}

