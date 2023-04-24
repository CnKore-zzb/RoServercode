#include "GuildQuest.h"
#include "QuestConfig.h"
#include "xLog.h"
#include "Guild.h"
#include "GuildServer.h"

GuildQuestMgr::GuildQuestMgr(Guild* pGuild) : m_pGuild(pGuild)
{

}

GuildQuestMgr::~GuildQuestMgr()
{
}

void GuildQuestMgr::fromData(const BlobGQuest& quest)
{
  map<DWORD, DWORD> mapQuest;
  mapQuest[393480001] = 393480006;
  mapQuest[393430001] = 393430005;
  mapQuest[393390001] = 393390005;
  mapQuest[393410001] = 393410005;
  mapQuest[393400001] = 393400005;
  mapQuest[393420001] = 393420004;
  mapQuest[393440001] = 393440006;
  mapQuest[393460001] = 393460004;
  mapQuest[393450001] = 393450003;
  mapQuest[393470001] = 393470003;

  for (int i = 0; i < quest.submit_size(); ++i)
  {
    auto m = mapQuest.find(quest.submit(i));
    if (m != mapQuest.end())
    {
      m_setSubmitIDs.insert(m->second);
      XLOG << "[公会任务-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "完成了旧版本任务" << m->first << "直接完成新版本任务" << m->second << XEND;
    }
    else
      m_setSubmitIDs.insert(quest.submit(i));
  }

  XDBG << "[公会任务-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载成功" << XEND;
}

void GuildQuestMgr::toData(BlobGQuest* quest)
{
  if (quest == nullptr)
    return;

  quest->Clear();
  for (auto &s : m_setSubmitIDs)
    quest->add_submit(s);

  XDBG << "[公会任务-保存]" << m_pGuild->getGUID() << m_pGuild->getName() << "保存成功,数据大小 :" << quest->ByteSize() << XEND;
}

void GuildQuestMgr::toData(QueryGQuestGuildCmd& cmd)
{
  cmd.clear_submit_quests();
  for (auto &s : m_setSubmitIDs)
    cmd.add_submit_quests(s);
}

void GuildQuestMgr::questSyncToZone(QWORD charid /*= 0*/)
{
  if (charid == 0)
  {
    const TVecGuildMember& vecMember = m_pGuild->getMemberList();
    for (auto &v : vecMember)
    {
      if (m_pGuild->getMisc().hasAuth(v->getJob(), EAUTH_ARTIFACT_QUEST) == false)
        continue;
      GuildArtifactQuestGuildSCmd cmd;
      cmd.set_charid(v->getCharID());
      m_pGuild->toData(cmd.mutable_quest());
      PROTOBUF(cmd, send, len);
      if (v->sendCmdToZone(send, len) == false)
        continue;
      XLOG << "[公会任务-同步]" << m_pGuild->getGUID() << m_pGuild->getName() << "成员" << v->getAccid() << v->getCharID() << v->getProfession() << v->getName() << "同步公会任务" << cmd.ShortDebugString() << XEND;
    }
    return;
  }

  GMember* pMember = m_pGuild->getMember(charid);
  if (pMember == nullptr || m_pGuild->getMisc().hasAuth(pMember->getJob(), EAUTH_ARTIFACT_QUEST) == false)
    return;

  GuildArtifactQuestGuildSCmd cmd;
  cmd.set_charid(pMember->getCharID());
  m_pGuild->toData(cmd.mutable_quest());
  PROTOBUF(cmd, send, len);
  pMember->sendCmdToZone(send, len);

  XLOG << "[公会任务-同步]" << m_pGuild->getGUID() << m_pGuild->getName()
    << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "同步公会任务" << cmd.ShortDebugString() << XEND;
}

void GuildQuestMgr::updateQuest(QWORD charid, DWORD questid)
{
  if (isSubmit(questid) == true)
    return;

  GMember* pMember = m_pGuild->getMember(charid);
  const SArtifactCFG* pCFG = GuildConfig::getMe().getArtifactCFGByQuestID(questid);
  if (pCFG && pMember && pCFG->dwUnlockMsg)
  {
    thisServer->sendMsg(pMember->getZoneID(), pMember->getCharID(), pCFG->dwUnlockMsg);

    const SQuestCFG* pQuestCFG = QuestConfig::getMe().getQuestCFG(questid);
    if (pQuestCFG != nullptr)
    {
      GuildEventM& rEvent = m_pGuild->getEvent();
      rEvent.addEvent(EGUILDEVENT_ARTIFACT_UNLOCK, TVecString{pMember->getName(), pQuestCFG->name, pCFG->strName});
    }
  }

  m_setSubmitIDs.insert(questid);
  questSyncToZone();
  XLOG << "[公会任务-更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "成功" << charid << "更新任务 id :" << questid << "为完成状态" << XEND;
}

