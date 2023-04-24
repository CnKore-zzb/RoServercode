#include "ChatFilterManager.h"
#include "GateUserManager.h"
#include "CommonConfig.h"
#include "PlatCmd.pb.h"
#include "xTime.h"
#include "xLog.h"
#include "json/json.h"

void CmdInfo::set(QWORD id, const BYTE* c, WORD l)
{
  if (id && c && l)
  {
    m_qwAccID = id;
    m_oData.put((BYTE *)c, l);
  }
}

bool CmdInfo::sendToScene(std::string str)
{
  xCommand *buf = (xCommand *)m_oData.getBin();
  WORD len = m_oData.getBinSize();

  if (!buf || !len) return false;

  GateUser* pUser = GateUserManager::getMe().getUserByAccID(m_qwAccID);
  if (!pUser) return false;


  if (str.empty())
  {
    pUser->forwardScene((UserCmd *)buf, len);
    return true;
  }

  switch (buf->param)
  {
    case CHATPARAM_CHAT:
      {
        PARSE_CMD_PROTOBUF(ChatCmd, rev);
        rev.set_str(str);

        PROTOBUF(rev, send, send_len);
        pUser->forwardScene((UserCmd *)send, send_len);
        XLOG << "[ChatFilterManager] sendToScene: 非法信息, 替换字段:"<< str << "cmd:" << rev.ShortDebugString() << XEND;
      }
      break;
    case CHATPARAM_BARRAGEMSG:
      {
        PARSE_CMD_PROTOBUF(BarrageMsgChatCmd, rev);
        rev.set_str(str);

        PROTOBUF(rev, send, send_len);
        pUser->forwardScene((UserCmd *)send, send_len);
        XLOG << "[ChatFilterManager] sendToScene: 非法信息, 替换字段:"<< str << "cmd:" << rev.ShortDebugString() << XEND;
      }
      break;
    default:
      break;
  }

  return true;
}

bool CmdInfo::sendToSelf()
{
  xCommand *buf = (xCommand *)m_oData.getBin();
  WORD len = m_oData.getBinSize();

  if (!buf || !len) return false;

  GateUser* pUser = GateUserManager::getMe().getUserByAccID(m_qwAccID);
  if (!pUser) return false;

  if (CHATPARAM_BARRAGEMSG == buf->param)
  {
    pUser->sendCmdToMe((UserCmd *)buf, len);
    XLOG << "[ChatFilterManager] sendToSelf: 广告信息,id:" << pUser->id << XEND;
    return true;
  }

  ChatSelfNtf ntf;
  PARSE_CMD_PROTOBUF(ChatCmd, rev);
  ntf.mutable_chat()->CopyFrom(rev);

  PROTOBUF(ntf, send, send_len);
  pUser->forwardScene((UserCmd *)send, send_len);
  XLOG << "[ChatFilterManager] sendToSelf: 广告信息,cmd:" << rev.ShortDebugString() << XEND;

  return true;
}

void ChatFilterManager::init()
{
  m_dwCmd = 0;
  m_bBusy = false;
  m_mapCmd.clear();
  m_list.clear();
}

DWORD ChatFilterManager::generateId()
{
  if (++m_dwCmd >= DWORD_MAX)
  {
    XLOG << "[ChatFilterManager] generateId over DWORD MAX, id:" << m_dwCmd << XEND;
    m_dwCmd = 1;
  }
  return m_dwCmd;
}

void ChatFilterManager::addCmd(DWORD id, QWORD accid, const BYTE *buf, WORD len)
{
  if (CommonConfig::m_dwChatFilterListSize < sizeList())
  {
    pairIT it = m_list.front();
    DWORD front = it.first;
    popList();
    freeCmd(front);
    XLOG << "[ChatFilterManager] addCmd size over, pop front element, size:" << sizeList() << " front:" << front << XEND;
  }

  if (m_mapCmd.end() != m_mapCmd.find(id))
  {
    freeCmd(id);
    XERR << "[ChatFilterManager] addCmd id has exist, id:" << id << XEND;
  }

  CmdInfo &info = m_mapCmd[id];
  info.set(accid, buf, len);

  pushList(pairIT(id, xTime::getCurSec()));
}

CmdInfo* ChatFilterManager::ChatFilterManager::getCmd(DWORD id)
{
  auto it = m_mapCmd.find(id);
  if (m_mapCmd.end() == it)
  {
    return nullptr;
  }

  return &it->second;
}

void ChatFilterManager::delCmd(DWORD id)
{
  auto it = m_mapCmd.find(id);
  if (m_mapCmd.end() == it)
  {
    if (!m_bBusy)
    {
      m_bBusy = true;
      XLOG << "[ChatFilterManager] timer plat is busy, list size:" << sizeList() << XEND;
    }
    return;
  }

  m_mapCmd.erase(it);
}

void ChatFilterManager::freeCmd(DWORD id)
{
  CmdInfo* pInfo = getCmd(id);
  if (!pInfo) return;

  pInfo->sendToScene();
  delCmd(id);
  XDBG << "[ChatFilterManager] freeCmd, id:" << id << XEND;
}

bool ChatFilterManager::checkFilter(GateUser* pUser, const UserCmd* buf, WORD len)
{
  if (!pUser || !buf || !len) return false;

  DWORD id = generateId();
  addCmd(id, pUser->accid, (BYTE *)buf, len);

  TextCheck data;
  data.set_param(id);
  data.set_project("ro");
  data.set_uid(std::to_string(pUser->id));
  data.set_name(pUser->name);
  data.set_createtime(xTime::getCurSec());

  Json::FastWriter fw;
  Json::Value ext;
  ext["sid"] = thisServer->getRegionID();
  ext["cid"] = thisServer->getPlatformID();
  data.set_ext(fw.write(ext));

  switch (buf->param)
  {
    case CHATPARAM_CHAT:
      {
        PARSE_CMD_PROTOBUF(ChatCmd, rev);
        data.set_content(rev.str());
        data.set_group((WORD)rev.channel());
      }
      break;
    case CHATPARAM_BARRAGEMSG:
      {
        PARSE_CMD_PROTOBUF(BarrageMsgChatCmd, rev);
        data.set_content(rev.str());
        data.set_group((WORD)ECHAT_CHANNEL_BARRAGE);
      }
      break;
    default:
      {
        return false;
      }
  }

  WORD len1 = data.ByteSize();
  char send[len1 + sizeof(PlatPacketHead)];
  PlatPacket *packet = (PlatPacket*)send;
  constructInPlace<PlatPacket>(packet);
  packet->ph.cmd = 1;
  packet->ph.flags = 1;
  packet->ph.len = len1 + 1;

  if(!data.SerializeToArray(packet->data, len1))
    return false;

  thisServer->sendCmdToPlat(send, len1 + sizeof(PlatPacketHead));
  return true;
}

bool ChatFilterManager::doCmd(BYTE* buf, WORD len)
{
  if(!buf || !len) return false;

  PlatPacket *packet = (PlatPacket*)buf;
  if(!packet) return false;

  switch(packet->ph.cmd)
  {
    case BOOLEAN:
      {
        Boolean data;
        data.ParseFromArray(packet->data, packet->getDataSize());
        delCmd(data.param());
        XLOG << "[ChatFilterManager]" << "doCmd:BOOLEAN" << "opaque:" << data.param() << "code:" << data.code() << "message:" << data.message() << XEND;
        break;
      }
    case TEXT_CHECK_RESULT:
      {
        TextCheckResult data;
        data.ParseFromArray(packet->data, packet->getDataSize());
        CmdInfo* pInfo = getCmd(data.param());
        if(!pInfo)
        {
          delCmd(data.param());
          return false;
        }
        switch(data.type())
        {
          case ERESULT_NORMAL:
          case ERESULT_WARN:
          case ERESULT_REPEAT:
            {
              pInfo->sendToScene();
              break;
            }
          case ERESULT_ADVERT:
            {
              pInfo->sendToSelf();
              break;
            }
          case ERESULT_FORBID:
            {
              pInfo->sendToScene(data.content());
              break;
            }
          case ERESULT_BLACK:
            {
              XLOG << "[ChatFilterManager] doCmd: 黑名单，屏蔽信息 id:" << data.param() << XEND;
              break;
            }
          default:
            XERR << "[ChatFilterManager] doCmd: 无效类型，type:" << data.type() << "rev:" << data.ShortDebugString() << XEND;
            break;
        }
        delCmd(data.param());
        break;
      }
    default:
      return false;
  }
  return true;
}

void ChatFilterManager::timer(DWORD cur)
{
  pairIT it;
  while(sizeList())
  {
    it = m_list.front();
    if(it.second + CommonConfig::m_dwChatFilterTimeout >= cur)
      return;

    popList();
    freeCmd(it.first);
  }

  if(0 == sizeList() && m_bBusy)
  {
    m_bBusy = false;
    XLOG << "[ChatFilterManager] timer plat is not busy, list size:" << sizeList() << XEND;
  }
}

/*
string ChatFilterManager::getChat()
{
  std::stringstream ss;
  int num = randBetween(1,10);
  for(int i=1; i<=num; i++)
  {
    ss << arrFilter[randBetween(0, 9)];
  }

  return ss.str();
}

void ChatFilterManager::setTest(GateUser* p, ChatCmd* c)
{
  if(nullptr == p || nullptr == c)
    return;

  m_pUser = p;
  m_cmd = *c;

  test();
}

void ChatFilterManager::test()
{
  if(nullptr == m_pUser)
    return;

  TVecString vec;
  for(int i=0; i<=100; i++)
  {
    vec.push_back(getChat());
  }

  for(auto &it : vec)
  {
    m_cmd.set_channel((EGameChatChannel)randBetween(1,10));
    m_cmd.set_str(it);
    if(isBusy())
      continue;
    checkFilter(m_pUser, &m_cmd);
  }

  XLOG << "[ChatFilterManager] test call" << XEND;
}
*/

