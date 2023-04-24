#include <iostream>
#include "GuildServer.h"
#include "GuildManager.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "SocialCmd.pb.h"
#include "RedisManager.h"
#include "GuildIconManager.h"

extern ConfigEnum ConfigEnums[];

GuildServer::GuildServer(OptArgs &args):RegionServer(args),m_oOneSecTimer(1),m_oTenSecTimer(10),m_oOneMinTimer(60)
{
}

GuildServer::~GuildServer()
{
}

void GuildServer::v_final()
{
  XLOG << "[GuildServer] v_final" << XEND;
  RegionServer::v_final();
  GuildManager::getMe().final();
  m_oSQLThread.thread_stop();
  m_oSQLThread.thread_join();
}

void GuildServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  RegionServer::v_closeNp(np);
}

void GuildServer::v_timetick()
{
  RegionServer::v_timetick();

  DWORD curTime = now();
  if (m_oOneSecTimer.timeUp(curTime))
  {
    if (m_oOneMinTimer.timeUp(curTime))
    {
      xTime frameTimer;

      GuildManager::getMe().timer(curTime);
      QWORD _e = frameTimer.uElapse();
      if (_e > 30 * 1000)
        XLOG << "[帧耗时]" << "guild" << _e << " 微秒" << XEND;
    }
  }
}

bool GuildServer::v_init()
{
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("GuildServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， GuildServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);

  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }
  const TIpPortPair* pStatPair = rServerData.getIpPort("StatServer");
  if (pStatPair)
  {
    addClient(ClientType::stat_server, pStatPair->first, pStatPair->second);
  }
  if (!loadConfig())
    return false;

  if (!initDBConnPool("DataBase", getRegionDBName(), m_oSQLThread.getDBConnPool()))
  {
    XERR << "[xSQLThread]" << "创建失败" << XEND;
    return false;
  }
  if (!m_oSQLThread.thread_start())
  {
    XERR << "[xSQLThread]" << "创建失败" << XEND;
    return false;
  }

  return true;
}

bool GuildServer::loadConfig()
{
  bool bResult = true;

  // base配置
  ConfigEnums[guild].loadfunc = []()
  {
    GuildManager::getMe().reload(ConfigType::guild);
    return true;
  };
  ConfigEnums[item].loadfunc = []()
  {
    GuildManager::getMe().reload(ConfigType::item);
    return true;
  };
  if (ConfigManager::getMe().loadGuildConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

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

  if(!GuildIconManager::getMe().init())
  {
    XERR << "[GuildIconManager]" << "init error!" << XEND;
    bResult = false;
  }
  // 数据
  if (!GuildManager::getMe().init())
  {
    XERR << "[GuildManager]" << "init error!" << XEND;
    bResult = false;
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool GuildServer::sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GuildServer::sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToUserSceneSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GuildServer::sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSceneUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GuildServer::sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSessionUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
}

bool GuildServer::sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params /*= MsgParams()*/)
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

bool GuildServer::sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params /*= MsgParams()*/)
{
  ChatWorldMsgSocialCmd cmd;
  cmd.mutable_msg()->set_id(msgid);
  cmd.mutable_msg()->set_type(EMESSAGETYPE_FRAME);
  cmd.mutable_msg()->set_act(EMESSAGEACT_ADD);
  params.toData(*cmd.mutable_msg());

  PROTOBUF(cmd, send, len);
  return sendCmdToZone(zoneid, send, len);
}

