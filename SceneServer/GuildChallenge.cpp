#include "GuildChallenge.h"
#include "SceneUser.h"
#include "SceneNpc.h"

GuildChallengeMgr::GuildChallengeMgr(SceneUser* user) : m_pUser(user)
{
}

GuildChallengeMgr::~GuildChallengeMgr()
{
}

bool GuildChallengeMgr::load(const BlobGuildChallenge& data)
{
  m_mapChallenge.clear();
  for (int i = 0; i < data.items_size(); ++i)
  {
    SGuildChallengeData d;
    if (d.fromData(data.items(i)))
      m_mapChallenge[d.dwID] = d;
  }
  return true;
}

bool GuildChallengeMgr::save(BlobGuildChallenge* data)
{
  if (data == nullptr)
    return false;
  for (auto& v : m_mapChallenge)
    v.second.toData(data->add_items());
  return true;
}

void GuildChallengeMgr::sendChallengeToGuild(TVecDWORD ids)
{
  if (m_pUser->hasGuild() == false)
    return;

  ChallengeProgressGuildSCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_guildid(m_pUser->getGuild().id());

  for (auto id : ids)
  {
    auto it = m_mapChallenge.find(id);
    if (it == m_mapChallenge.end())
      continue;
    GuildChallengeItem* p = cmd.add_items();
    if (p == nullptr)
      return;
    it->second.toData(p);
  }

  if (cmd.items_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
}

void GuildChallengeMgr::resetProgress(bool force/* = false*/)
{
  if (force || m_pUser->getVar().getVarValue(EVARTYPE_GUILD_CHALLENGE_WEEK) == 0)
  {
    m_mapChallenge.clear();
    m_pUser->getVar().setVarValue(EVARTYPE_GUILD_CHALLENGE_WEEK, 1);
  }
}

void GuildChallengeMgr::updateProgress(EGuildChallenge type, DWORD progress/* = 1*/, QWORD value/* = 0*/)
{
  if (m_pUser->hasGuild() == false)
    return;

  if (m_pUser->getGuild().isFunctionOpen(EGUILDFUNCTION_BUILDING) == false)
    return;

  // 重置所有进度
  resetProgress();

  const TVecGuildChallengeCFG& tasks = GuildConfig::getMe().getGuildChallengeCFGByType(type);
  TVecDWORD finishids;
  for (auto& task : tasks)
  {
    SGuildChallengeData& item = m_mapChallenge[task.dwID];
    item.dwID = task.dwID;

    if (item.dwProgress >= task.dwSubProgress)
      continue;

    if (!task.checkValid(value))
      continue;

    item.dwProgress += progress;
    XLOG << "[公会挑战-进度]" << m_pUser->accid << m_pUser->id << m_pUser->name << "公会:" << m_pUser->getGuild().id() << m_pUser->getGuild().name()
         << "挑战:" << task.dwID << "增加进度:" << progress << "最新进度:" << item.dwProgress << "更新成功" << XEND;

    if (item.dwProgress >= task.dwSubProgress)
    {
      // 任务完成
      item.dwProgress = task.dwSubProgress;
      finishids.push_back(task.dwID);
    }
  }

  if (finishids.empty() == false)
    sendChallengeToGuild(finishids);
}

// 登录
void GuildChallengeMgr::onLogin()
{
  updateProgress(EGUILDCHALLENGE_LOGIN);
}

// 累加完成40层无限塔
void GuildChallengeMgr::onPassTower(DWORD curlayer)
{
  updateProgress(EGUILDCHALLENGE_ENDLESS_TOWER, 1, curlayer);
}

// 通过n次公会副本
void GuildChallengeMgr::onGuildRaidFinish()
{
  updateProgress(EGUILDCHALLENGE_GUILD_RAID);
}

// 参与gvg
void GuildChallengeMgr::onEnterGVG()
{
  updateProgress( EGUILDCHALLENGE_GVG);
}

// 完成n次封印裂隙
void GuildChallengeMgr::onRepairSeal()
{
  updateProgress(EGUILDCHALLENGE_SEAL);
}

// 完成n个看板任务
void GuildChallengeMgr::onQuestSubmit(DWORD questid)
{
  const SQuestCFGEx* pCfg = QuestManager::getMe().getQuestCFG(questid);
  if (pCfg == nullptr)
    return;
  if (pCfg->eType == EQUESTTYPE_WANTED)
    updateProgress(EGUILDCHALLENGE_WANTED_QUEST);
  else if (pCfg->eType == EQUESTTYPE_GUILD)
    updateProgress(EGUILDCHALLENGE_GUILD_QUEST);
}

// 杀怪
void GuildChallengeMgr::onKillNpc(SceneNpc* pNpc)
{
  if (pNpc == nullptr)
    return;
  if (pNpc->getNpcType() == ENPCTYPE_MINIBOSS)
    updateProgress(EGUILDCHALLENGE_KILL_MINI);
  else if (pNpc->getNpcType() == ENPCTYPE_MVP)
    updateProgress(EGUILDCHALLENGE_KILL_MVP);
}
