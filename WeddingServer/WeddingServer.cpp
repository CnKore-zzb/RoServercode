#include <iostream>
#include "WeddingServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RegCmd.h"
#include "WeddingSCmd.pb.h"
#include "CommonConfig.h"
#include "SceneTrade.pb.h"
#include "WeddingManager.h"
#include "WeddingUserMgr.h"
#include "ChatCmd.pb.h"
#include "SysmsgConfig.h"

WeddingServer::WeddingServer(OptArgs &args):RegionServer(args)
{
}

WeddingServer::~WeddingServer()
{
}

void WeddingServer::v_final()
{
  XLOG << "[" << getServerName() << "],v_final" << XEND;
  RegionServer::v_final();
  WeddingManager::getMe().final();
}

void WeddingServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  RegionServer::v_closeNp(np);
}

void WeddingServer::v_timetick()
{
  RegionServer::v_timetick();
 
  DWORD curTime = xTime::getCurSec();
  
  WeddingManager::getMe().timeTick(curTime);

}

bool WeddingServer::v_init()
{    
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("WeddingServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， WeddingServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);

  if (!loadConfig())
    return false;

  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }

  WeddingManager::getMe().init();

  return true;
}

bool WeddingServer::loadConfig()
{
  bool bResult = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }
  // base配置
  if (ConfigManager::getMe().loadWeddingConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool WeddingServer::sendCmdToClient(QWORD charId, DWORD zoneId, void* buf, WORD len)
{
  Cmd::ForwardWedding2CSCmd cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);
  return thisServer->sendCmdToZone(zoneId, send, len2);
}

bool WeddingServer::forwardCmdToSceneServer(QWORD charId, DWORD zoneId, void* buf, WORD len)
{
  Cmd::ForwardWedding2SSCmd cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToZone(zoneId, send2, len2);
}

void WeddingServer::sendMsg(QWORD charId, DWORD zoneId, DWORD msgid, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/)
{
  if (charId == 0)
    return;
  SysMsg cmd;
  mutableMsg(cmd, msgid, params, eType, eAct);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(charId, zoneId, send, len);
}

void WeddingServer::mutableMsg(SysMsg &cmd, DWORD msgid, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/)
{
  cmd.set_id(msgid);
  cmd.set_type(eType);
  cmd.set_act(eAct);
  params.toData(cmd);
}

//只是通知玩家
bool WeddingServer::sendMail(Cmd::SendMail& cmd)
{
  const MailData& rData = cmd.data();
  PROTOBUF(cmd, send2, len2);
  if (rData.type() == EMAILTYPE_SYSTEM)
  {
    thisServer->sendCmdToAllZone(send2, len2);
  }
  else
  {
    WeddingUser* pUser = WeddingUserMgr::getMe().getWeddingUser(rData.receiveid());
    if (pUser)
    {
      thisServer->sendCmdToZone(pUser->m_dwZoneId, send2, len2);
    }
  }
  return true;
}

void WeddingServer::sendWeddingMsg(QWORD qwCharId, DWORD dwZoneId, const Cmd::WeddingEventMsgCCmd& msg)
{
  AddOfflineMsgSocialCmd cmd;
  OfflineMsg* pMsg = cmd.mutable_msg();
  pMsg->set_targetid(qwCharId);
  pMsg->set_type(EOFFLINEMSG_WEDDING);
  pMsg->mutable_weddingmsg()->CopyFrom(msg);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(dwZoneId, send, len);
}

bool WeddingServer::sendWorldMsg(DWORD zoneid, DWORD msgid, MsgParams params /*= MsgParams()*/)
{
  ChatWorldMsgSocialCmd cmd;
  cmd.mutable_msg()->set_id(msgid);
  cmd.mutable_msg()->set_type(EMESSAGETYPE_FRAME);
  cmd.mutable_msg()->set_act(EMESSAGEACT_ADD);
  params.toData(*cmd.mutable_msg());

  PROTOBUF(cmd, send, len);
  return sendCmdToZone(zoneid, send, len);
}


//100140001->1
//100140002->1a
std::string WeddingServer::getZoneName(DWORD dwZoneId)
{
  return LuaManager::getMe().call<char*>("ZoneNumToString", getClientZoneID(dwZoneId));

  // static vector<string> sVecSuffix = {
  //   "", "a", "c", "d", "e", "f", "g", "h", "k", "m", "n", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "aa", "cc", "dd", "ee", "ff", "gg", "hh", "kk"
  // };

  // DWORD t = dwZoneId % 10000;
  // if (t >= 9000)
  //   return "斗技场线";

  // auto it = m_mapZoneNameCache.find(dwZoneId);
  // if (it != m_mapZoneNameCache.end())
  //   return it->second;

  // DWORD len = sVecSuffix.size();
  // float f = ((float)t) / len;
  // DWORD n1 = std::ceil(f);
  // DWORD n2 = t % len;
  // if (n2 == 0)
  // {
  //   n2 = len;
  // }
  // n2 -= 1;

  // stringstream ss;
  // ss << n1 << sVecSuffix[n2];
  // m_mapZoneNameCache[t] = ss.str();
  // return ss.str();
}

void WeddingServer::onSessionRegist(xNetProcessor *np, DWORD dwZoneId)
{
  auto it = m_setStartedSession.find(dwZoneId);
  if (it != m_setStartedSession.end())
  { //weddingserver 没宕，sessionserver重启了
    //WeddingManager::getMe().onSessionRestart(dwZoneId);
  }
  else
  {
    m_setStartedSession.insert(dwZoneId);
  }

  WeddingManager::getMe().onSessionRestart(dwZoneId);
}
