#include <iostream>
#include "TeamServer.h"
#include "TeamManager.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "SocialCmd.pb.h"
#include "RedisManager.h"
#include "MatchSCmd.pb.h"
#include "TeamDataThread.h"
#include "TeamIDManager.h"

extern ConfigEnum ConfigEnums[];

TeamServer::TeamServer(OptArgs &args):ZoneServer(args),m_oOneSecTimer(1),m_oTenSecTimer(10)
{
}

TeamServer::~TeamServer()
{
}

void TeamServer::v_final()
{
  XLOG << "[TeamServer] v_final" << XEND;
  ZoneServer::v_final();
  TeamManager::getMe().final();

  ThTeamDataThread::getMe().thread_stop();
  ThTeamDataThread::getMe().thread_join();
}

void TeamServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  ZoneServer::v_closeNp(np);
}

void TeamServer::v_timetick()
{
  ZoneServer::v_timetick();

  DWORD curTime = now();
  if (m_oOneSecTimer.timeUp(curTime))
  {
    xTime frameTimer;

    TeamManager::getMe().timer(curTime);
    QWORD _e = frameTimer.uElapse();
    if (_e > 30 * 1000)
      XLOG << "[帧耗时]" << "team" << _e << " 微秒" << XEND;
  }
}

void TeamServer::v_closeServer(xNetProcessor *np)
{
    if (!np) return;
}

void TeamServer::v_verifyOk(xNetProcessor *task)
{
  if (!task) return;

  XLOG << "[服务器同步],v_verifyOk" << task->name << getServerName() << XEND;
}

void TeamServer::init_ok()
{
  ZoneServer::init_ok();
}

bool TeamServer::v_init()
{
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pStatPair = rServerData.getIpPort("StatServer");
  if (pStatPair)
  {
    addClient(ClientType::stat_server, pStatPair->first, pStatPair->second);
  }
  if (!loadConfig())
    return false;

  if (!ThTeamDataThread::getMe().thread_start())
  {
    XERR << "[ThTeamDataThread]" << "创建失败" << XEND;
    return false;
  }
  TeamIDManager::getMe().load();

  return true;
}

bool TeamServer::loadConfig()
{
  bool bResult = true;

  if (!addDataBase(REGION_DB, false))
  {
    XERR << "[数据库],DBPool,初始化失败," << REGION_DB << XEND;
    return false;
  }

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }

  ConfigEnums[teamgoal].loadfunc = []()
  {
    TeamManager::getMe().reload();
    XLOG << "[组队]" << "加载" << XEND;
    return true;
  };

  if (ConfigManager::getMe().loadTeamConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  const xLuaData& data = getBranchConfig().getData("Redis");
  if (RedisManager::getMe().init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port")) == false)
  {
    XERR << "[RedisManager]" << "init error!" << XEND;
    bResult = false;
  }

  // 数据
  if (!TeamManager::getMe().loadAllDataFromDB())
  {
    XERR << "[TeamManager]" << "init error!" << XEND;
    bResult = false;
  }
  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool TeamServer::sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

void TeamServer::broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE t, QWORD index, const void *cmd, WORD len, QWORD excludeID/*=0*/, DWORD ip/*=0*/)
{
  BroadcastCmdTeamCmd message;
  message.set_type(t);
  message.set_id(index);
  message.set_data(cmd, len);
  message.set_len(len);
  PROTOBUF(message, msend, mlen);
  sendCmdToSession(msend, mlen);
}

bool TeamServer::sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToUserSceneSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

bool TeamServer::sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSceneUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

bool TeamServer::sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSessionUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

bool TeamServer::sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params /*= MsgParams()*/)
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

bool TeamServer::sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params /*= MsgParams()*/)
{
  ChatWorldMsgSocialCmd cmd;
  cmd.mutable_msg()->set_id(msgid);
  cmd.mutable_msg()->set_type(EMESSAGETYPE_FRAME);
  cmd.mutable_msg()->set_act(EMESSAGEACT_ADD);
  params.toData(*cmd.mutable_msg());

  PROTOBUF(cmd, send, len);
  return sendCmdToSession(send, len);
}

bool TeamServer::forwardCmdToMatch(const void *buf, WORD len)
{
  SessionForwardTeamMatch cmd;
  cmd.set_data(buf, len);
  cmd.set_len(len);

  PROTOBUF(cmd, send2, len2);
  return sendCmdToSession(send2, len2);
}
