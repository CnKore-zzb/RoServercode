#include "GuildWelfare.h"
#include "Guild.h"
#include "GuildMember.h"
#include "MiscConfig.h"

bool SGuildWelfareItem::toData(GuildWelfareItem* data)
{
  if (data == nullptr)
    return false;
  data->set_type(eType);
  if (qwID) data->set_id(qwID);
  if (eSource != ESOURCE_MIN) data->set_source(eSource);
  data->set_rewardid(dwRewardID);
  data->set_createtime(dwCreateTime);
  data->set_overduetime(dwOverdueTime);
  for (auto id : setCharID)
    data->add_charids(id);

  data->set_ownername(strOwnerName);
  data->set_sourceid(dwSourceID);

  data->set_eventguid(dwEventGUID);
  return true;
}

bool SGuildWelfareItem::fromData(const GuildWelfareItem& data)
{
  eType = data.type();
  qwID = data.id();
  eSource = data.source();
  dwRewardID = data.rewardid();
  dwCreateTime = data.createtime();
  dwOverdueTime = data.overduetime();
  for (int i = 0; i < data.charids_size(); ++i)
    setCharID.insert(data.charids(i));

  strOwnerName = data.ownername();
  dwSourceID = data.sourceid();

  dwEventGUID = data.eventguid();
  return true;
}

bool SGuildWelfareItem::canGetWelfare(GMember* member)
{
  if (member == nullptr)
    return false;

  DWORD cur = now();

  // 进公会前发放的奖励不可领, 过期的不可领
  if (member->getEnterTime() > dwCreateTime || cur >= dwOverdueTime)
    return false;

  if (setCharID.find(member->getCharID()) != setCharID.end())
    return false;

  switch (eType)
  {
  case EGUILDWELFARE_BUILDING:
  {
    if (member->canGetBuildingWelfare(static_cast<EGuildBuilding>(qwID)) == false)
      return false;
    break;
  }
  case EGUILDWELFARE_CHALLENGE:
  {
    if (member->canGetChallengeWelfare(dwCreateTime) == false)
      return false;
    break;
  }
  case EGUILDWELFARE_TREASURE:
  {
    DWORD weekstart = xTime::getWeekStart(dwCreateTime, 5 * 3600);
    if(member->getEnterTime() >= weekstart)
      return false;
  }
  break;
  case EGUILDWELFARE_MIN:
  case EGUILDWELFARE_MAX:
    break;
  }

  return true;
}

GuildWelfareMgr::GuildWelfareMgr(Guild* guild) : m_pGuild(guild)
{
}

GuildWelfareMgr::~GuildWelfareMgr()
{
}

bool GuildWelfareMgr::toData(GuildWelfare* data)
{
  if (data == nullptr)
    return false;
  for (auto& v : m_mapWelfare)
    for (auto& w : v.second)
      w.toData(data->add_items());
  return true;
}

bool GuildWelfareMgr::fromData(const GuildWelfare& data)
{
  for (int i = 0; i < data.items_size(); ++i)
  {
    SGuildWelfareItem item;
    if (item.fromData(data.items(i)))
      m_mapWelfare[item.eType].push_back(item);
  }
  return true;
}

bool GuildWelfareMgr::addWelfare(EGuildWelfare type, DWORD rewardid,
    ESource source/* = ESOURCE_MIN*/, QWORD id/* = 0*/, const string& name /*= ""*/, DWORD dwSourceID /*= 0*/, DWORD dwEventGUID /*= 0*/, DWORD dwIndex /*= 0*/)
{
  if (type <= EGUILDWELFARE_MIN || type >= EGUILDWELFARE_MAX || rewardid <= 0)
    return false;

  DWORD cur = now();
  SGuildWelfareItem item;
  item.eType = type;
  item.qwID = id;
  item.eSource = source;
  item.dwRewardID = rewardid;
  item.dwCreateTime = cur;
  item.dwOverdueTime = cur + MiscConfig::getMe().getGuildWelfareCFG().dwOverdueTime;
  item.strOwnerName = name;
  item.dwSourceID = dwSourceID;
  item.dwEventGUID = dwEventGUID;
  item.dwIndex = dwIndex;

  m_mapWelfare[type].push_back(item);
  m_pGuild->setMark(EGUILDDATA_MISC);

  m_dwCurTotalMember = 0;
  TVecGuildMember& members = m_pGuild->getAllMemberList();
  for (auto& m : members)
  {
    if (item.canGetWelfare(m))
    {
      notifyMember(m, true);
      ++m_dwCurTotalMember;
    }
  }

  XLOG << "[公会福利-新增]" << m_pGuild->getGUID() << m_pGuild->getName() << "类型:" << type << "奖励:" << rewardid << "source:" << source << "id:" << id << "发送成功" << XEND;

  removeOverdueWelfare();

  return true;
}

bool GuildWelfareMgr::getWelfare(GMember* member)
{
  if (member == nullptr || member->isOnline() == false)
    return false;

  QWORD charid = member->getCharID();
  SendWelfareGuildSCmd cmd;
  bool challengereward = false;
  for (auto& v : m_mapWelfare)
  {
    for (auto& w : v.second)
    {
      if (w.canGetWelfare(member) == false)
        continue;

      w.addUser(charid);
      GuildWelfareItem* p = cmd.add_items();
      if (p == nullptr)
        continue;
      p->set_rewardid(w.dwRewardID);
      p->set_source(w.eSource);

      // 不同类型的福利在领取成功后的处理
      switch (w.eType)
      {
        case EGUILDWELFARE_BUILDING:
          {
            EGuildBuilding type = static_cast<EGuildBuilding>(w.qwID);
            member->setBuildingNextWelfareTime(type, m_pGuild->getMisc().getBuilding().getNextWelfareTime(type));
          }
          break;
        case EGUILDWELFARE_CHALLENGE:
          {
            challengereward = true;
          }
          break;
        case EGUILDWELFARE_TREASURE:
          {
            if (p)
            {
              p->set_ownername(w.strOwnerName);
              p->set_sourceid(w.dwSourceID);
              p->set_eventguid(w.dwEventGUID);
              p->set_index(w.dwIndex);
            }
          }
          break;
        case EGUILDWELFARE_MIN:
        case EGUILDWELFARE_MAX:
          break;
      }
    }
  }

  if (challengereward)
    m_pGuild->getMisc().getChallenge().notifyAllData(member);

  if (cmd.items_size() <= 0)
    return false;

  m_pGuild->setMark(EGUILDDATA_MISC);

  cmd.set_charid(member->getCharID());
  PROTOBUF(cmd, send, len);
  member->sendCmdToZone(send, len);

  notifyMember(member, false);

  XLOG << "[公会福利-领奖]" << m_pGuild->getGUID() << m_pGuild->getName() << "玩家:" << member->getAccid() << member->getCharID() << member->getName() << "福利:";
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    XLOG << cmd.items(i).rewardid() << cmd.items(i).source();
  }
  XLOG << "领取成功" << XEND;
  return true;
}

void GuildWelfareMgr::notifyMember(GMember* member, bool add)
{
  if (member == nullptr || member->isOnline() == false)
    return;

  WelfareNtfGuildCmd cmd;
  cmd.set_welfare(add);
  PROTOBUF(cmd, send, len);
  member->sendCmdToMe(send, len);
}

void GuildWelfareMgr::notifyMember(GMember* member)
{
  if (member == nullptr)
    return;

  for (auto& v : m_mapWelfare)
    for (auto& w : v.second)
      if (w.canGetWelfare(member))
      {
        notifyMember(member, true);
        return;
      }
}

bool GuildWelfareMgr::isMemberHasWelfare(GMember* member, EGuildWelfare type, QWORD id)
{
  if (member == nullptr)
    return false;
  auto it = m_mapWelfare.find(type);
  if (it == m_mapWelfare.end())
    return false;
  for (auto& v : it->second)
  {
    if (v.qwID == id)
      return v.canGetWelfare(member);
  }
  return false;
}

void GuildWelfareMgr::removeOverdueWelfare()
{
  DWORD cur = now();

  for (auto& v : m_mapWelfare)
  {
    for (auto it = v.second.begin(); it != v.second.end(); )
    {
      if (cur >= it->dwOverdueTime)
      {
        XLOG << "[公会福利-过期清理]" << m_pGuild->getGUID() << m_pGuild->getName() << "类型:" << v.first << "奖励:" << it->dwRewardID << "source:" << it->eSource << "id:" << it->qwID << "清理成功" << XEND;
        it = v.second.erase(it);
        m_pGuild->setMark(EGUILDDATA_MISC);
        continue;
      }
      ++it;
    }
  }
}

SGuildWelfareItem* GuildWelfareMgr::getWelfareItem(EGuildWelfare eType, DWORD dwEventGUID)
{
  auto m = m_mapWelfare.find(eType);
  if (m == m_mapWelfare.end())
    return nullptr;

  vector<SGuildWelfareItem>& vecItems = m->second;
  auto v = find_if(vecItems.begin(), vecItems.end(), [dwEventGUID](const SGuildWelfareItem& rItem) -> bool{
    return rItem.dwEventGUID == dwEventGUID;
  });
  if (v == vecItems.end())
    return nullptr;

  return &(*v);
}

