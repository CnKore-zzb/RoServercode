#include "UserMessage.h"
#include "SceneUser.h"

UserMessage::UserMessage(SceneUser *pUser) : m_pUser(nullptr)
{
  m_pUser = pUser;
}

bool UserMessage::load(const BlobChatMsg& oBlob)
{
  m_listPreMsg.clear();
  for (int i = 0; i < oBlob.preset_size(); ++i)
  {
    const PresetMsg& rMsg = oBlob.preset(i);
    TPresetMsg preMsg;
    preMsg.first = rMsg.msgid();
    preMsg.second = rMsg.msg();
    m_listPreMsg.push_back(preMsg);
  }

  XLOG << "[玩家message-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加载了" << m_listPreMsg.size() << "条文本" << XEND;
  return true;
}

bool UserMessage::save(BlobChatMsg* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  for (auto v = m_listPreMsg.begin(); v != m_listPreMsg.end(); ++v)
  {
    PresetMsg *pMsg = pBlob->add_preset();
    if (nullptr != pMsg)
    {
      pMsg->set_msgid(v->first);
      pMsg->set_msg(v->second);
    }
  }
  XDBG << "[玩家message-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存了" << pBlob->preset_size() << "条文本 数据大小 :" << pBlob->ByteSize() << XEND;
  return true;
}

bool UserMessage::addPreMsg(const string& sPreMsg)
{
  TPresetMsg pairMsg;
  pairMsg.first = m_listPreMsg.size()+1;
  pairMsg.second = sPreMsg;
  m_listPreMsg.push_back(pairMsg);

  XLOG << "[玩家message-添加文本] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 添加了文本 : " << sPreMsg.c_str() << XEND;
  return true;
}

bool UserMessage::toClient()
{
  if (nullptr == m_pUser)
    return false;
 // if (m_listPreMsg.size() < 1)
 //   return true;

  Cmd::PresetMsgCmd cmd;
  for (auto v = m_listPreMsg.begin(); v != m_listPreMsg.end(); ++v)
  {
    //if (v->second.size() < 2)
    //  continue;

    cmd.add_msgs(v->second);
#ifdef _TYH_DEBUG
    XDBG("[UserMessage],id:%u, msg:%s", v->first, v->second.c_str());
#endif
  }
  //if (cmd.msgs_size() > 0 )
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[玩家message-数据同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步了" << m_listPreMsg.size() << "条文本" << XEND;
  return true;
}

bool UserMessage::resetMsgs(const Cmd::PresetMsgCmd & cmd)
{
  m_listPreMsg.clear();
  for (int i = 0; i < cmd.msgs_size(); i++)
    addPreMsg(cmd.msgs(i));
  XLOG << "[玩家message-设置文本] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 设置了 %u 条文本" << cmd.msgs_size() << XEND;
  return true;
}
