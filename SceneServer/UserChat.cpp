#include "UserChat.h"
#include "SceneUser.h"

UserChat::UserChat(SceneUser* pUser) : m_pUser(pUser)
{

}

UserChat::~UserChat()
{

}

bool UserChat::load(const BlobChat& rChat)
{
  resetChat();
  m_listChatCount.clear();
  m_listChatItem.clear();
  m_setSaveList.clear();

  for (int i = 0; i < rChat.counts_size(); ++i)
    m_listChatCount.push_back(rChat.counts(i));
  for (int i = 0; i < rChat.items_size(); ++i)
    m_listChatItem.push_back(rChat.items(i));
  for (int i = 0; i < rChat.savelist_size(); ++i)
    m_setSaveList.insert(rChat.savelist(i));

  return true;
}

bool UserChat::save(BlobChat* pChat)
{
  resetChat();
  if (pChat == nullptr)
  {
    XERR << "[聊天信息-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存失败" << XEND;
    return false;
  }

  pChat->Clear();

  DWORD dwCount = 0;
  DWORD dwMax = MiscConfig::getMe().getCreditCFG().dwChatCalcMaxCount;
  for (auto &s : m_listChatCount)
  {
    if (dwCount >= dwMax)
      break;
    if (s.to_chat() != 0 && s.from_chat() != 0)
    {
      ++dwCount;
      pChat->add_counts()->CopyFrom(s);
    }
  }
  if (dwCount < dwMax)
  {
    for (auto &s : m_listChatCount)
    {
      if (dwCount >= dwMax)
        break;
      if (s.to_chat() != 0 && s.from_chat() != 0)
        continue;

      ++dwCount;
      pChat->add_counts()->CopyFrom(s);
    }
  }
  for (auto &s : m_listChatItem)
    pChat->add_items()->CopyFrom(s);
  for (auto &s : m_setSaveList)
    pChat->add_savelist(s);

  // update chat relation time
  UpdateRelationTimeSocialCmd cmd;
  cmd.set_charid(m_pUser->id);
  for (auto &m : m_mapUpdateList)
  {
    if (m_pUser->getSocial().checkRelation(m.first, ESOCIALRELATION_CHAT) == true)
    {
      cmd.set_targetid(m.first);
      cmd.set_time(m.second);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToSession(send, len);
    }
  }
  m_mapUpdateList.clear();

  XDBG << "[聊天信息-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << pChat->ByteSize() << XEND;
  return true;
}

void UserChat::onChat(const ChatRetCmd& cmd)
{
  resetChat();
  auto s = find_if(m_listChatCount.begin(), m_listChatCount.end(), [&](const ChatCount& rChat) -> bool{
    return rChat.targetid() == cmd.targetid() || rChat.targetid() == cmd.id();
  });
  if (s == m_listChatCount.end())
  {
    ChatCount oCount;
    oCount.set_targetid(cmd.targetid() == m_pUser->id ? cmd.id() : cmd.targetid());
    m_listChatCount.push_back(oCount);
    s = find_if(m_listChatCount.begin(), m_listChatCount.end(), [&](const ChatCount& rChat) -> bool{
      return rChat.targetid() == cmd.targetid() || rChat.targetid() == cmd.id();
    });
    if (s == m_listChatCount.end())
    {
      XERR << "[玩家聊天-聊天]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未发现对方聊天信息" << XEND;
      return;
    }
  }

  DWORD dwNow = xTime::getCurSec();
  if (s->targetid() == cmd.targetid())
  {
    s->set_to_chat(s->to_chat() + 1);
    m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_CHAT, cmd.targetid(), 1);
    m_mapUpdateList[cmd.targetid()] = dwNow;
  }
  else if (s->targetid() == cmd.id())
  {
    s->set_from_chat(s->from_chat() + 1);
    m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_CHAT, s->targetid(), 1);
    m_mapUpdateList[s->targetid()] = dwNow;
  }
  else
  {
    XERR << "[玩家聊天-聊天]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未知错误" << XEND;
    return;
  }

  XDBG << "[玩家聊天-聊天]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "charid :" << s->targetid() << "to :" << s->to_chat() << "from :" << s->from_chat() << XEND;

  if (m_listChatItem.size() < MiscConfig::getMe().getCreditCFG().dwChatSaveMaxCount)
  {
    auto save = m_setSaveList.find(cmd.targetid());
    if (save != m_setSaveList.end())
    {
      ChatItem oItem;
      oItem.set_time(xTime::getCurSec());
      oItem.set_msg(cmd.str());
      m_listChatItem.push_back(oItem);
      XDBG << "[玩家聊天-聊天]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << oItem.ShortDebugString() << XEND;
    }
  }
}

void UserChat::resetChat()
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_CHAT) != 0)
    return;
  m_pUser->getVar().setVarValue(EVARTYPE_CHAT, 1);

  // save to db
  if (m_listChatItem.empty() == false)
  {
    ChatSaveRecordCmd cmd;
    cmd.set_charid(m_pUser->id);
    cmd.set_portrait(m_pUser->getPortrait().getCurPortrait());
    cmd.set_time(xTime::getWeekStart(xTime::getCurSec()));

    BlobChatItem oItem;
    for (auto &s : m_listChatItem)
    {
      s.set_charid(m_pUser->id);
      s.set_name(m_pUser->name);
      oItem.add_items()->CopyFrom(s);
    }

    string data;
    if (oItem.SerializeToString(&data) == false)
    {
      XDBG << "[玩家聊天-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存失败,序列化失败" << XEND;
      return;
    }
    if (compress(data, data) == false)
    {
      XDBG << "[玩家聊天-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存失败,压缩失败" << XEND;
      return;
    }
    cmd.set_data(data.c_str(), data.size());

    PROTOBUF(cmd, send, len);
    if (thisServer->sendCmdToData(send, len) == false)
    {
      XDBG << "[玩家聊天-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存失败,压缩失败" << XEND;
      return;
    }

    m_listChatItem.clear();
  }

  // clear save list
  m_setSaveList.clear();

  // flush chatcount to savelist
  map<DWORD, QWORD> mapSaveList;
  for (auto &l : m_listChatCount)
    mapSaveList[l.to_chat() + l.from_chat()] = l.targetid();
  const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
  DWORD dwCount = 0;
  for (auto &m : mapSaveList)
  {
    if (++dwCount > rCFG.dwChatSaveCharCount)
      continue;
    m_setSaveList.insert(m.second);
    XLOG << "[玩家聊天-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加 charid :" << m.second << "为亲密好友" << XEND;
  }
  m_listChatCount.clear();
}

