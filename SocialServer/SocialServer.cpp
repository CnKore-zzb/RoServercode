#include <iostream>
#include "SocialServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "SocialityManager.h"
#include "GCharManager.h"

//extern ConfigEnum ConfigEnums[];

SocialServer::SocialServer(OptArgs &args):ZoneServer(args),m_oOneSecTimer(1),m_oTenSecTimer(10)
{
}

SocialServer::~SocialServer()
{
}

void SocialServer::v_final()
{
  XLOG << "[SocialServer] v_final" << XEND;
  ZoneServer::v_final();
  SocialityManager::getMe().final();
}

void SocialServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  ZoneServer::v_closeNp(np);
}

void SocialServer::v_timetick()
{
  ZoneServer::v_timetick();

  DWORD curTime = now();
  if (m_oOneSecTimer.timeUp(curTime))
  {
    xTime frameTimer;
    QWORD _e = 0;

    SocialityManager::getMe().loadtimer(curTime);
    if (m_oTenSecTimer.timeUp(curTime))
    {
      SocialityManager::getMe().timer(curTime);
      _e = frameTimer.uElapse();
      if (_e > 30 * 1000)
        XLOG << "[帧耗时]" << "social" << _e << " 微秒" << XEND;
    }

    frameTimer.elapseStart();
    _e = frameTimer.uElapse();
    if (_e > 30 * 1000)
      XLOG << "[帧耗时]" << "guild" << _e << " 微秒" << XEND;
  }
}

void SocialServer::v_closeServer(xNetProcessor *np)
{
    if (!np) return;
}

void SocialServer::v_verifyOk(xNetProcessor *task)
{
  if (!task) return;

  XLOG << "[服务器同步],v_verifyOk" << task->name << getServerName() << XEND;
}

void SocialServer::init_ok()
{
  ZoneServer::init_ok();
}

bool SocialServer::v_init()
{
  const ServerData& rServerData = xServer::getServerData();
  /*const TIpPortPair* pSelfPair = rServerData.getIpPort("SocialServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， SocialServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);*/

  {
    const TIpPortPair* pStatPair = rServerData.getIpPort("StatServer");
    if (pStatPair)
      addClient(ClientType::stat_server, pStatPair->first, pStatPair->second);
  }

  bool bResult = true;
  if (!loadConfig())
  {
    XERR << "[SocialServer] 读取配置失败" << XEND;
    bResult = false;
  }
  if (!addDataBase(REGION_DB, false))
  {
    XERR << "[SocialServer] DBPool :" << REGION_DB << "初始化失败" << XEND;
    bResult = false;
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool SocialServer::loadConfig()
{
  bool bResult = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }

  // config
  if (ConfigManager::getMe().loadSocialConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool SocialServer::sendCmdToMe(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::GlobalForwardCmdSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

bool SocialServer::sendCmdToScene(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToUserSceneSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

bool SocialServer::sendCmdToSceneUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSceneUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

bool SocialServer::sendCmdToSessionUser(DWORD zoneid, QWORD charid, const void *buf, WORD len)
{
  Cmd::ForwardToSessionUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToSession(send, len2);
}

bool SocialServer::sendMsg(DWORD zoneid, QWORD charid, DWORD msgid, MsgParams params /*= MsgParams()*/)
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

bool SocialServer::sendDebugMsg(DWORD zoneid, QWORD charid, const string& str)
{
  if (thisServer->isOuter())
    return false;
  return sendMsg(zoneid, charid, 10, MsgParams(str));
}

bool SocialServer::sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params /*= MsgParams()*/)
{
  ChatWorldMsgSocialCmd cmd;
  cmd.mutable_msg()->set_id(msgid);
  cmd.mutable_msg()->set_type(EMESSAGETYPE_FRAME);
  cmd.mutable_msg()->set_act(EMESSAGEACT_ADD);
  params.toData(*cmd.mutable_msg());

  PROTOBUF(cmd, send, len);
  return sendCmdToSession(send, len);
}

void SocialServer::gcharPatch()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (pField == nullptr)
  {
    XERR << "[加载] 无法获取数据库charbase" << XEND;
    return;
  }
  pField->setValid("charid");

  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, nullptr);
  if (QWORD_MAX == retNum)
  {
    XERR << "[加载] 无法获取数据库charbase" << XEND;
    return;
  }

  for (QWORD q = 0; q < retNum; ++q)
  {
    QWORD charid = set[q].get<QWORD>("charid");
    GCharReader pGChar(thisServer->getRegionID(), charid);
    pGChar.getBySocial();
  }
}

