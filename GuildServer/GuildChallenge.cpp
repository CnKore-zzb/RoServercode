#include "GuildChallenge.h"
#include "Guild.h"
#include "GuildConfig.h"
#include "MiscConfig.h"
#include "GuildServer.h"

bool SGuildChallenge::toData(GuildChallenge* data, GMember* member/* = nullptr*/)
{
  if (data == nullptr)
    return false;
  data->set_id(dwID);
  data->set_progress(dwProgress);
  if (member) // 根据玩家实际领取情况设置reward, 前端用, reward为true表示有奖励可以领取
    data->set_reward(member->hasWelfare(EGUILDWELFARE_CHALLENGE, dwID));
  else
    data->set_reward(bReward);
  data->set_extrareward(bExtraReward);
  return true;
}

bool SGuildChallenge::fromData(const GuildChallenge& data)
{
  dwID = data.id();
  dwProgress = data.progress();
  bReward = data.reward();
  bExtraReward = data.extrareward();
  return true;
}

GuildChallengeMgr::GuildChallengeMgr(Guild* guild) : m_pGuild(guild)
{
}

GuildChallengeMgr::~GuildChallengeMgr()
{
}

bool GuildChallengeMgr::toData(GuildChallengeData* data, GMember* member/* = nullptr*/)
{
  if (data == nullptr)
    return false;
  for (auto& v : m_mapChallenge)
    v.second.toData(data->add_challenges());
  return true;
}

bool GuildChallengeMgr::fromData(const GuildChallengeData& data)
{
  m_mapChallenge.clear();
  for (int i = 0; i < data.challenges_size(); ++i)
    m_mapChallenge[data.challenges(i).id()].fromData(data.challenges(i));
  m_mapID2Index.clear();
  DWORD idx = 0;
  for (auto& v : m_mapChallenge)
    m_mapID2Index[v.first] = idx++;
  return true;
}

bool GuildChallengeMgr::toGuildData(GuildData* data, GMember* member)
{
  if (data == nullptr)
    return false;
  if (member->canChallenge())
  {
    for (auto& v : m_mapChallenge)
      v.second.toData(data->add_challenges(), member);
  }
  return true;
}

void GuildChallengeMgr::timer(DWORD cur)
{
  resetChallenge();
}

void GuildChallengeMgr::open()
{
  resetChallenge();
}

void GuildChallengeMgr::resetChallenge(bool force/* = false*/)
{
  if (force == false && m_pGuild->getMisc().isFunctionOpen(EGUILDFUNCTION_BUILDING) == false)
    return;

  if (force == false && m_pGuild->getMisc().getVarValue(EVARTYPE_GUILD_CHALLENGE_WEEK) == 1)
    return;

  const SGuildCFG* cfg = m_pGuild->getGuildCFG();
  if (cfg == nullptr || cfg->dwChallengeCount <= 0)
    return;

  ChallengeUpdateNtfGuildCmd cmd;
  cmd.set_refreshtime(getNextRefreshTime());

  for (auto& v : m_mapChallenge)
  {
    GuildChallenge* p = cmd.add_dels();
    if (p)
      p->set_id(v.first);
  }
  m_mapChallenge.clear();
  m_mapID2Index.clear();

  TVecDWORD ids, extrarewardids;
  GuildConfig::getMe().randGuildChallenge(ids, extrarewardids, m_pGuild->getLevel(), cfg->dwChallengeCount);
  if (ids.empty() && extrarewardids.empty())
  {
    XERR << "[公会挑战-任务重置]" << m_pGuild->getGUID() << m_pGuild->getName() << m_pGuild->getLevel() << "数量:" << cfg->dwChallengeCount << "任务重置失败,未随机到任务";
    return;
  }

  m_pGuild->getMisc().setVarValue(EVARTYPE_GUILD_CHALLENGE_WEEK, 1);

  XLOG << "[公会挑战-任务重置]" << m_pGuild->getGUID() << m_pGuild->getName() << m_pGuild->getLevel() << "数量:" << cfg->dwChallengeCount << "新任务无额外奖励:";
  for (auto id : ids)
  {
    m_mapChallenge[id].dwID = id;
    m_mapChallenge[id].toData(cmd.add_updates());
    XLOG << id;
  }
  XLOG << "新任务有额外奖励:";
  for (auto id : extrarewardids)
  {
    m_mapChallenge[id].dwID = id;
    m_mapChallenge[id].bExtraReward = true;
    m_mapChallenge[id].toData(cmd.add_updates());
    XLOG << id;
  }
  XLOG << "重置成功" << XEND;
  DWORD idx = 0;
  for (auto& v : m_mapChallenge)
    m_mapID2Index[v.first] = idx++;

  m_pGuild->setMark(EGUILDDATA_MISC);

  PROTOBUF(cmd, send, len);
  m_pGuild->broadcastCmd(send, len);

  TVecGuildMember& members = m_pGuild->getAllMemberList();
  for (auto& m : members)
  {
    m->resetChallenge();
    //m->setRedTip(EREDSYS_GUILD_CHALLENGE_ADD, true);
    m->setRedTip(EREDSYS_GUILD_CHALLENGE_REWARD, false);
  }
}

void GuildChallengeMgr::updateProgress(GMember* member, const vector<GuildChallengeItem*>& items, DWORD progress/* = 1*/, bool checkmember/* = true*/)
{
  if (member == nullptr)
    return;

  if (checkmember && member->canChallenge() == false)
    return;

  vector<SGuildChallenge*> challenges;
  set<string> ntfname;
  for (auto item : items)
  {
    if (item == nullptr)
      continue;
    auto it = m_mapChallenge.find(item->id());
    if (it == m_mapChallenge.end())
      continue;
    if (it->second.bReward)
      continue;
    const SGuildChallengeCFG* cfg = GuildConfig::getMe().getGuildChallengeCFG(item->id());
    if (cfg == nullptr)
      continue;
    if (item->progress() < cfg->dwSubProgress)
      continue;

    auto s = m_mapID2Index.find(item->id());
    if (s == m_mapID2Index.end())
      continue;

    if (checkmember && member->isChallengeFinished(s->second))
      continue;
    if (checkmember)
      member->setChallengeFinished(s->second);

    it->second.dwProgress += progress;
    XLOG << "[公会挑战-进度更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "成员:" << member->getAccid() << member->getCharID() << member->getName()
         << "挑战:" << item->id() << "子进度:" << item->progress() << "总进度:" << it->second.dwProgress << "更新成功" << XEND;

    ntfname.insert(cfg->strGroupName);

    if (it->second.dwProgress >= cfg->dwTotalProgress)
    {
      it->second.dwProgress = cfg->dwTotalProgress;
      it->second.bReward = true;
      m_pGuild->getMisc().getWelfare().addWelfare(EGUILDWELFARE_CHALLENGE, cfg->dwReward, ESOURCE_GUILD_CHALLENGE, cfg->dwID);
      DWORD extrareward = 0;
      if (it->second.bExtraReward)
      {
        extrareward = MiscConfig::getMe().getGuildChallengeCFG().dwExtraReward;
        if (extrareward)
          m_pGuild->getMisc().getWelfare().addWelfare(EGUILDWELFARE_CHALLENGE, extrareward, ESOURCE_GUILD_CHALLENGE, cfg->dwID);
      }
      XLOG << "[公会挑战-进度更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "成员:" << member->getAccid() << member->getCharID() << member->getName()
           << "挑战:" << cfg->dwID << "奖励:" << cfg->dwReward << "额外奖励:" << extrareward << XEND;
    }
    challenges.push_back(&it->second);
  }

  if (challenges.empty() == false)
  {
    m_pGuild->setMark(EGUILDDATA_MISC);

    TVecGuildMember& members = m_pGuild->getAllMemberList();
    for (auto& m : members)
    {
      ChallengeUpdateNtfGuildCmd cmd;
      cmd.set_refreshtime(getNextRefreshTime());
      for (auto c : challenges)
      {
        if (c == nullptr)
          continue;
        c->toData(cmd.add_updates(), m);
      }

      bool redtip = false;
      for (int i = 0; i < cmd.updates_size(); ++i)
        if (cmd.updates(i).reward())
        {
          redtip = true;
          break;
        }
      if (redtip)
        m->setRedTip(EREDSYS_GUILD_CHALLENGE_REWARD, true);

      if (m->isOnline() && m->canChallenge())
      {
        PROTOBUF(cmd, send, len);
        m->sendCmdToMe(send, len);
      }
    }
  }

  if (ntfname.empty() == false)
  {
    for (auto& name : ntfname)
      thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3717, MsgParams{name});
  }
}

void GuildChallengeMgr::notifyAllData(GMember* member, bool nochallenge/* = false*/)
{
  if (member == nullptr || member->isOnline() == false)
    return;

  ChallengeUpdateNtfGuildCmd cmd;
  cmd.set_refreshtime(getNextRefreshTime());

  if (nochallenge == false && member->canChallenge())
  {
    for (auto& v : m_mapChallenge)
      v.second.toData(cmd.add_updates(), member);

    bool redtip = false;
    for (int i = 0; i < cmd.updates_size(); ++i)
      if (cmd.updates(i).reward())
      {
        redtip = true;
        break;
      }
    member->setRedTip(EREDSYS_GUILD_CHALLENGE_REWARD, redtip);
  }

  PROTOBUF(cmd, send, len);
  member->sendCmdToMe(send, len);
}

void GuildChallengeMgr::onAddMember(GMember* member)
{
  if (member == nullptr || member->isOnline() == false || member->canChallenge())
    return;

  ChallengeUpdateNtfGuildCmd cmd;
  cmd.set_refreshtime(getNextRefreshTime());

  for (auto& v : m_mapChallenge)
  {
    GuildChallenge* p = cmd.add_dels();
    if (p)
      p->set_id(v.first);
  }

  PROTOBUF(cmd, send, len);
  member->sendCmdToMe(send, len);
}
