#include "MsgManager.h"
#include "xServer.h"
#include "GatewayCmd.h"
#include "GuidManager.h"
#include "config/RoleDataConfig.h"
#include "ChatCmd.pb.h"
#include "SocialCmd.pb.h"
#include "SessionCmd.pb.h"
#include "ZoneServer.h"

extern xServer* thisServer;

// msg params
MsgParams::MsgParams()
{

}

MsgParams::MsgParams(DWORD value)
{
  addNumber(value);
}

MsgParams::MsgParams(const string& str)
{
  addString(str);
}

MsgParams::MsgParams(DWORD value1, DWORD value2)
{
  addNumber(value1);
  addNumber(value2);
}

MsgParams::MsgParams(const string& str, DWORD value)
{
  addString(str);
  addNumber(value);
}

MsgParams::MsgParams(const string& str, float value)
{
  addString(str);
  addFloat(value);
}

MsgParams::MsgParams(const RoleData* pData, float value)
{
  if (pData == nullptr)
    return;

  ostringstream ostr;
  ostr.precision(4);
  //ostr.setf(std::ios::fixed);
  if (pData->bPercent)
    ostr << value * 100 << "%";
  else
    ostr << value;

  addString(pData->prop);
  addString(ostr.str());
}

MsgParams::MsgParams(const string& str1, const string& str2)
{
  addString(str1);
  addString(str2);
}

MsgParams::MsgParams(const string& str1, const string& str2, const string& str3)
{
  addString(str1);
  addString(str2);
  addString(str3);
}

MsgParams::MsgParams(const string& str, DWORD value, DWORD value2)
{
  addString(str);
  addNumber(value);
  addNumber(value2);
}

MsgParams::MsgParams(const TVecString& subparams)
{
  for (auto& param : subparams)
    addSubString(param);
}

MsgParams::~MsgParams()
{

}

bool MsgParams::addNumber(DWORD value)
{
  ostringstream ostr;
  ostr << value;

  MsgParam msg;
  msg.set_param(ostr.str());

  m_vecParams.push_back(msg);
  return true;
}

bool MsgParams::addFloat(float value)
{
  ostringstream ostr;
  ostr << value;

  MsgParam msg;
  msg.set_param(ostr.str());

  m_vecParams.push_back(msg);
  return true;
}

bool MsgParams::addString(const string& str)
{
  MsgParam msg;
  msg.set_param(str);

  m_vecParams.push_back(msg);
  return true;
}

bool MsgParams::addSubParam()
{
  MsgParam msg;
  m_vecParams.push_back(msg);
  return true;
}

bool MsgParams::addSubNumber(DWORD value)
{
  ostringstream ostr;
  ostr << value;

  if (m_vecParams.empty())
    if (!addSubParam())
      return false;
  m_vecParams.rbegin()->add_subparams(ostr.str());
  return true;
}

bool MsgParams::addSubFloat(float value)
{
  ostringstream ostr;
  ostr << value;

  if (m_vecParams.empty())
    if (!addSubParam())
      return false;
  m_vecParams.rbegin()->add_subparams(ostr.str());
  return true;
}

bool MsgParams::addSubString(const string& str)
{
  if (m_vecParams.empty())
    if (!addSubParam())
      return false;
  m_vecParams.rbegin()->add_subparams(str);
  return true;
}

bool MsgParams::addLangParam()
{
  MsgParam msg;
  m_vecParams.push_back(msg);
  return true;
}

bool MsgParams::addLangParam(const MsgLangParam& langparam)
{
  if (m_vecParams.empty())
    if (!addLangParam())
      return false;
  MsgLangParam* param = m_vecParams.rbegin()->add_langparams();
  if (param == nullptr)
    return false;
  param->CopyFrom(langparam);
  return true;
}

void MsgParams::toData(SysMsg& rMsg) const
{
  for (auto v = m_vecParams.begin(); v != m_vecParams.end(); ++v)
  {
    MsgParam* pParam = rMsg.add_params();
    if (pParam != nullptr)
      pParam->CopyFrom(*v);
  }
}

std::string MsgParams::fmtString(std::string base, MsgParams params)
{
  if (params.m_vecParams.empty())
    return base;
  
  std::stringstream ss;
  std::string s1= "%s";
  string::size_type a = s1.size();
  string::size_type pos = 0;
  for (auto &v: params.m_vecParams)
  {
    pos = base.find(s1, pos);
    if (pos == string::npos)
      break;
    base.replace(pos, a, v.param());
    string::size_type b = v.param().size();
    pos += b;
  }

  return base;
}

void MsgParams::toNpcChatNtf(NpcChatNtf& ntf) const
{
  for (auto v = m_vecParams.begin(); v != m_vecParams.end(); ++v)
  {
    MsgParam* pParam = ntf.add_params();
    if (pParam != nullptr)
      pParam->CopyFrom(*v);
  }
}

// msg manager
TVecBroadcastInfo MsgManager::m_vecBroadcastInfo;
MsgManager::MsgManager()
{

}

MsgManager::~MsgManager()
{

}

void MsgManager::sendMsg(QWORD qwCharID, DWORD dwMsgID, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/, DWORD dwDelay /*= 0*/)
{
  if (!thisServer->isZoneServer()) return;

  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  cmd.set_act(eAct);
  cmd.set_delay(dwDelay);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  ((ZoneServer *)thisServer)->sendCmdToMe(qwCharID, send, len);
}

void MsgManager::sendMsgMust(QWORD qwCharID, DWORD dwMsgID, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/, DWORD dwDelay /*= 0*/)
{
  if (!thisServer->isZoneServer()) return;

  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  cmd.set_act(eAct);
  cmd.set_delay(dwDelay);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  if (((ZoneServer *)thisServer)->sendCmdToMe(qwCharID, send, len))
    return;

  //add offline
  AddOfflineMsgSocialCmd cmd2;
  cmd2.mutable_msg()->set_type(EOFFLINEMSG_SYS2);
  cmd2.mutable_msg()->set_targetid(qwCharID);
  cmd2.mutable_msg()->mutable_syscmd()->CopyFrom(cmd);

  PROTOBUF(cmd2, send2, len2);

  std::string serverName(thisServer->getServerName());
  //session server
  if (thisServer->getServerType() == SERVER_TYPE_SESSION)
  {
    ((ZoneServer *)thisServer)->sendCmd(ClientType::global_server, send2, len2);
    return;
  }

  //scene server
  //场景没有连数据库
  if (thisServer->getServerType() == SERVER_TYPE_SCENE)
  {
    ForwardRegionSessionCmd cmd3;
    cmd3.set_region_type(DWORD(ClientType::global_server));
    cmd3.set_data(send2, len2);
    cmd3.set_len(len2);
    PROTOBUF(cmd3, send3, len3);
    ((ZoneServer *)thisServer)->sendCmdToSession(send3, len3);
    return;
  }
}


void MsgManager::sendDebugMsg(QWORD qwCharID, const string& str)
{
  if (thisServer->isOuter())
    return;
  sendMsg(qwCharID, 10, MsgParams(str));
}

void MsgManager::sendRoomMsg(DWORD dwRoomID, DWORD dwMsgID, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/)
{
  if (!thisServer->isZoneServer()) return;

  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  ((ZoneServer *)thisServer)->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_ROOM, dwRoomID, send, len);
}

void MsgManager::sendMapMsg(DWORD dwMapID, DWORD dwMsgID, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/)
{
  if (!thisServer->isZoneServer()) return;

  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  ((ZoneServer *)thisServer)->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_MAP, dwMapID, send, len);
}

void MsgManager::sendWorldMsg(DWORD dwMsgID, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/)
{
  if (!thisServer->isZoneServer()) return;

  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  params.toData(cmd);

  PROTOBUF(cmd, send, len);
  BUFFER_CMD(forward, ForwardAllUserGatewayCmd);

  forward->len = len;
  bcopy(send, forward->data, (DWORD)len);

  ((ZoneServer *)thisServer)->sendCmdToServer(forward, sizeof(ForwardAllUserGatewayCmd)+len, "GateServer");
}

void MsgManager::sendMapCmd(DWORD dwMapID, const void* cmd, unsigned short len)
{
  if (!thisServer->isZoneServer()) return;
  ((ZoneServer *)thisServer)->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_MAP, dwMapID, cmd, len);
}

void MsgManager::sendWorldCmd(const void* cmd, unsigned short len)
{
  if (!thisServer->isZoneServer()) return;
  BUFFER_CMD(forward, ForwardAllUserGatewayCmd);

  forward->len = len;
  bcopy(cmd, forward->data, (DWORD)len);

  ((ZoneServer *)thisServer)->sendCmdToServer(forward, sizeof(ForwardAllUserGatewayCmd)+len, "GateServer");
}

void MsgManager::alter_msg(const string& strTitle, const string& strMsg, const EPushMsg& event)
{
  if (!thisServer->isZoneServer()) return;
  if (((ZoneServer *)thisServer)->getServerType() == SERVER_TYPE_SUPER)
    return;

  AlterMsgGateSuperCmd cmd;
  cmd.set_title(strTitle);
  cmd.set_msg(strMsg);
  cmd.set_event(event);
  PROTOBUF(cmd, send, len);
  ((ZoneServer *)thisServer)->sendCmdToServer(send, len, "SuperServer");
}

DWORD MsgManager::addBroadInfo(DWORD action, DWORD dwId, const string& starttime, const string& str, DWORD dwMsgID, DWORD dwInterval, DWORD dwCount, const MsgParam& langparam)
{
  if (!thisServer->isZoneServer()) return 0;
  if (action == 0) //del
  {
    if (delBroad(dwId) == false) {
      return 0;
    }
    return dwId;
  }

  if (action == 1)  //modify
  {
    for (auto it = m_vecBroadcastInfo.begin(); it != m_vecBroadcastInfo.end(); it++)
    {
      if (it->dwID == dwId)
      {
        it->dwMsgID = dwMsgID;
        it->dwInterval = dwInterval;
        it->dwMaxCount = dwCount;
        parseTime(starttime.c_str(), it->dwStartTime);
        it->oParams.clear();
        if (langparam.langparams_size() > 0)
        {
          for (int i = 0; i < langparam.langparams_size(); ++i)
            it->oParams.addLangParam(langparam.langparams(i));
        }
        else
        {
          it->oParams.addString(str);
        }
        XLOG << "[信息管理-修改广播] id:" << dwId << "starttime :" << starttime << "msg :" << str << "msgid :" << dwMsgID << "interval :" << dwInterval << "count :" << dwCount << "langparam:" << langparam.ShortDebugString() << XEND;
        return dwId;
      }
    }
    return 0;
  }

  //add  new
  SBroadcastInfo stInfo;
  if (dwId == 0)
    stInfo.dwID = GuidManager::getMe().getNextBroadID();
  else
    stInfo.dwID = dwId;
  stInfo.dwMsgID = dwMsgID;
  parseTime(starttime.c_str(), stInfo.dwStartTime);
  stInfo.dwInterval = dwInterval;
  stInfo.dwMaxCount = dwCount;
  if (langparam.langparams_size() > 0)
  {
    for (int i = 0; i < langparam.langparams_size(); ++i)
      stInfo.oParams.addLangParam(langparam.langparams(i));
  }
  else
  {
    stInfo.oParams.addString(str);
  }
  m_vecBroadcastInfo.push_back(stInfo);

  XLOG << "[信息管理-添加广播] id:" << stInfo.dwID << "starttime :" << starttime << "msg :" << str << "msgid :" << dwMsgID << "interval :" << dwInterval << "count :" << dwCount << XEND;
  return stInfo.dwID;
}

bool MsgManager::delBroad(DWORD dwId)
{
  if (!thisServer->isZoneServer()) return false;

  for (auto it = m_vecBroadcastInfo.begin(); it != m_vecBroadcastInfo.end(); it++) {
    if (it->dwID == dwId)
    {
      m_vecBroadcastInfo.erase(it);
      XLOG << "[信息管理-删除广播] 成功 id :" << dwId << XEND;
      return true;
    }
  }
  XLOG << "[信息管理-删除广播] 失败 id :" << dwId << "找不到" << XEND;
  return false;
}

void MsgManager::timer(DWORD dwCurTime)
{
  if (!thisServer->isZoneServer()) return;
  processBroadcast(dwCurTime);
}

void MsgManager::processBroadcast(DWORD dwCurTime)
{
  if (!thisServer->isZoneServer()) return;
  for (auto v = m_vecBroadcastInfo.begin(); v != m_vecBroadcastInfo.end();)
  {
    if (v->dwCount >= v->dwMaxCount)
    {
      XLOG << "[信息管理-删除广播] starttime :" << v->dwStartTime << "id :" << v->dwMsgID << "interval :" << v->dwInterval << "count :" << v->dwCount << "maxcount :" << v->dwMaxCount << XEND;
      v = m_vecBroadcastInfo.erase(v);
      continue;
    }

    if (v->dwStartTime < dwCurTime)
    {
      sendWorldMsg(v->dwMsgID, v->oParams);
      v->dwCount += 1;
      v->dwStartTime = dwCurTime + v->dwInterval;
      XLOG << "[信息管理-广播放送] starttime :" << v->dwStartTime << "id :" << v->dwMsgID << "interval :" << v->dwInterval << "count :" << v->dwCount << "maxcount :" << v->dwMaxCount << XEND;
    }

    ++v;
  }
}

string RealtimeVoiceManager::getRandID()
{
  static vector<char> letter = {
    '1', '2','3','4','5','6','7','8','9',
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  };
  stringstream ss;
  for (int i = 0; i < 16; ++i)
    ss << letter[randBetween(0, sizeof(letter) - 1)];
  return ss.str();
}

const string& RealtimeVoiceManager::getID()
{
  int retry = 20;
  while (retry--)
  {
    const string& id = getRandID();
    if (m_setUsedID.find(id) != m_setUsedID.end())
      continue;
    auto p = m_setUsedID.insert(id);
    return *p.first;
  }
  return STRING_EMPTY;
}
