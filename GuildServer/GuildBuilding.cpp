#include "GuildBuilding.h"
#include "Guild.h"
#include "MiscConfig.h"
#include "GuildServer.h"
#include "GuildMember.h"

bool SGuildBuilding::toData(GuildBuilding* data)
{
  if (data == nullptr)
    return false;
  data->set_type(eType);
  data->set_level(dwLv);
  data->set_isbuilding(bIsBuilding);
  data->set_nextwelfaretime(dwNextWelfareTime);
  data->set_nextbuildtime(dwNextBuildTime);
  for (auto& v : mapMaterial)
  {
    GuildBuildMaterial* p = data->add_materials();
    if (p)
    {
      p->set_id(v.first);
      p->set_count(v.second);
    }
  }
  return true;
}

bool SGuildBuilding::fromData(const GuildBuilding& data)
{
  eType = data.type();
  dwLv = data.level();
  bIsBuilding = data.isbuilding();
  dwNextWelfareTime = data.nextwelfaretime();
  dwNextBuildTime = data.nextbuildtime();
  for (int i = 0; i < data.materials_size(); ++i)
    mapMaterial[data.materials(i).id()] = data.materials(i).count();
  return true;
}

bool SGuildBuilding::isBuilding()
{
  return bIsBuilding || now() < dwNextBuildTime;
}

GuildBuildingMgr::GuildBuildingMgr(Guild* guild) : m_pGuild(guild)
{
}

GuildBuildingMgr::~GuildBuildingMgr()
{
}

bool GuildBuildingMgr::toData(GuildBuildingData* data)
{
  if (data == nullptr)
    return false;
  for (auto& v : m_mapBuilding)
    v.second.toData(data->add_buildings());
  data->set_version(m_dwVersion);
  return true;
}

bool GuildBuildingMgr::fromData(const GuildBuildingData& data)
{
  for (int i = 0; i < data.buildings_size(); ++i)
  {
    SGuildBuilding b;
    if (b.fromData(data.buildings(i)))
      m_mapBuilding[b.eType] = b;
  }
  for (int i = 1; i < EGUILDBUILDING_MAX; ++i)
  {
    if (EGuildBuilding_IsValid(i))
    {
      EGuildBuilding type = static_cast<EGuildBuilding>(i);
      const SGuildBuildingCFG* cfg = GuildConfig::getMe().getGuildBuildingCFG(type, 0);
      if (cfg == nullptr)
        continue;
      auto it = m_mapBuilding.find(type);
      if (it == m_mapBuilding.end())
      {
        m_mapBuilding[type].eType = type;
        m_pGuild->setMark(EGUILDDATA_MISC);
      }
    }
  }
  m_dwVersion = data.version();

  version();
  return true;
}

void GuildBuildingMgr::version()
{
  for (DWORD i = m_dwVersion; i < BUILDING_VERSION; ++i)
  {
    if (i == 0)
    {
      for (auto& v : m_mapBuilding)
        for (auto& m : v.second.mapMaterial)
          m.second *= 100;
    }
  }
  m_dwVersion = BUILDING_VERSION;
  m_pGuild->setMark(EGUILDDATA_MISC);
}

void GuildBuildingMgr::timer(DWORD cur)
{
  for (auto& v : m_mapBuilding)
    sendWelfare(v.first);
}

void GuildBuildingMgr::openBuildingFunction()
{
  if (m_pGuild->getMisc().isFunctionOpen(EGUILDFUNCTION_BUILDING))
    return;

  m_pGuild->getMisc().openFunction(EGUILDFUNCTION_BUILDING);

  TVecGuildMember& members = m_pGuild->getAllMemberList();
  for (auto& m : members)
    m->setBuildingEffect();
  return;
}

bool GuildBuildingMgr::build(GMember* member, EGuildBuilding type)
{
  if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX)
    return false;
  if (member == nullptr)
    return false;

  for (auto& v : m_mapBuilding)
    if (v.second.isBuilding())
    {
      XERR << "[公会建筑-建造]" << m_pGuild->getGUID() << m_pGuild->getName() << type << "建筑:" << v.first << "正在建造" << XEND;
      return false;
    }

  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
  {
    m_mapBuilding[type].eType = type;
    it = m_mapBuilding.find(type);
    if (it == m_mapBuilding.end())
      return false;
  }

  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  if (it->second.dwLv < rCFG.dwBuildingCheckLv && it->second.dwLv >= m_pGuild->getLevel())
  {
    XERR << "[公会建筑-建造]" << m_pGuild->getGUID() << m_pGuild->getName() << type << it->second.dwLv << "公会等级:" << m_pGuild->getLevel() << "当前等级大于等级公会等级" << XEND;
    return false;
  }

  const SGuildBuildingCFG* cfg = GuildConfig::getMe().getGuildBuildingCFG(type, it->second.dwLv);
  if (cfg == nullptr)
  {
    XERR << "[公会建筑-建造]" << m_pGuild->getGUID() << m_pGuild->getName() << type << it->second.dwLv << "配置找不到" << XEND;
    return false;
  }

  if (cfg->bIsMaxLv)
  {
    XERR << "[公会建筑-建造]" << m_pGuild->getGUID() << m_pGuild->getName() << type << it->second.dwLv << "已达到最大等级" << XEND;
    return false;
  }

  if (cfg->eLvupBuildingType != EGUILDBUILDING_MIN && getBuildingLevel(cfg->eLvupBuildingType) < cfg->dwLvupBuildingLv)
  {
    XERR << "[公会建筑-建造]" << m_pGuild->getGUID() << m_pGuild->getName() << type << it->second.dwLv << "依赖其他建筑:" << cfg->eLvupBuildingType
         << "需要等级:" << cfg->dwLvupBuildingLv << "实际等级:" << getBuildingLevel(cfg->eLvupBuildingType) << "等级不够" << XEND;
    return false;
  }

  it->second.bIsBuilding = true;
  it->second.dwNextBuildTime = now() + cfg->dwBuildTime;
  m_pGuild->setMark(EGUILDDATA_MISC);

  if (it->second.dwLv >= 1)
  {
    m_pGuild->getEvent().addEvent(EGUILDEVENT_BUILDING_LVUP, TVecString{member->getName(), cfg->strName});
    m_pGuild->broadcastNpcMsg(MiscConfig::getMe().getGuildBuildingCFG().dwBuildingLvUpNpcID, MiscConfig::getMe().getGuildBuildingCFG().dwBuildingLvUpMsgID, MsgParams(cfg->strName));
  }
  else
  {
    m_pGuild->getEvent().addEvent(EGUILDEVENT_BUILDING_BUILD, TVecString{member->getName(), cfg->strName});
    m_pGuild->broadcastNpcMsg(MiscConfig::getMe().getGuildBuildingCFG().dwBuildingLvUpNpcID, MiscConfig::getMe().getGuildBuildingCFG().dwBuildingBuildMsgID, MsgParams(cfg->strName));
  }

  set<EGuildBuilding> types;
  types.insert(type);
  updateDataToDScene(types);

  return true;
}

bool GuildBuildingMgr::cancelBuild(EGuildBuilding type)
{
  if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX)
    return false;

  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end() || it->second.bIsBuilding == false)
    return false;

  it->second.bIsBuilding = false;
  m_pGuild->setMark(EGUILDDATA_MISC);

  set<EGuildBuilding> types;
  types.insert(type);
  updateDataToDScene(types);

  XLOG << "[公会建筑-取消建造状态]" << m_pGuild->getGUID() << m_pGuild->getName() << type << "取消成功" << XEND;
  return true;
}

bool GuildBuildingMgr::clearBuildCD(EGuildBuilding type)
{
  if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX)
    return false;

  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return false;

  it->second.dwNextBuildTime = 0;
  m_pGuild->setMark(EGUILDDATA_MISC);

  set<EGuildBuilding> types;
  types.insert(type);
  updateDataToDScene(types);

  XLOG << "[公会建筑-清除建造cd]" << m_pGuild->getGUID() << m_pGuild->getName() << type << "清除成功" << XEND;
  return true;
}

bool GuildBuildingMgr::isBuilding(EGuildBuilding type)
{
  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return false;
  return it->second.isBuilding();
}

bool GuildBuildingMgr::canSubmitMaterial(EGuildBuilding type, const TMapMaterial& material)
{
  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return false;
  if (it->second.isBuilding() == false)
    return false;

  const SGuildBuildingCFG* cfg = GuildConfig::getMe().getGuildBuildingCFG(type, it->second.dwLv);
  if (cfg == nullptr)
    return false;

  if (cfg->bIsMaxLv)
    return false;

  for (auto& submit : material)
  {
    auto need = cfg->mapMaterial.find(submit.first);
    if (need == cfg->mapMaterial.end())
      return false;
    if (submit.second > need->second)
      return false;
    auto cost = it->second.mapMaterial.find(submit.first);
    if (cost != it->second.mapMaterial.end() && cost->second / 100 + submit.second > need->second)
      return false;
  }

  return true;
}

bool GuildBuildingMgr::submitMaterial(EGuildBuilding type, const TMapMaterial& material, GMember* member/* = nullptr*/, DWORD submitinc/* = 0*/)
{
  if (canSubmitMaterial(type, material) == false)
    return false;

  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return false;

  const SGuildBuildingCFG* cfg = GuildConfig::getMe().getGuildBuildingCFG(type, it->second.dwLv);
  if (cfg == nullptr)
    return false;

  DWORD fund = 0, total_count = 0;;
  for (auto& submit : material)
  {
    auto cost = it->second.mapMaterial.find(submit.first);
    if (cost == it->second.mapMaterial.end())
      it->second.mapMaterial[submit.first] = 0;
    it->second.mapMaterial[submit.first] += 100.0 * submit.second * (1.0 + submitinc / 100.0);

    total_count += submit.second;

    const SGuildBuildingMaterialCFG* cfg = GuildConfig::getMe().getGuildBuildingMaterial(submit.first);
    if (cfg)
      fund += cfg->dwFund * submit.second;
  }

  bool lvup = true;
  for (auto& need : cfg->mapMaterial)
  {
    auto cost = it->second.mapMaterial.find(need.first);
    if (cost == it->second.mapMaterial.end() || cost->second / 100 < need.second)
    {
      lvup = false;
      break;
    }
  }

  if (member)
  {
    member->addSubmitCount(type, total_count);

    GuildEventM& rEvent = m_pGuild->getEvent();
    rEvent.addEvent(EGUILDEVENT_BUILDING_SUPPLY, TVecString{member->getName(), cfg->strName});
  }

  if (lvup)
  {
    levelup(type, 1);
  }
  else
  {
    set<EGuildBuilding> types;
    types.insert(type);
    updateDataToDScene(types);
  }

  m_pGuild->setMark(EGUILDDATA_MISC);

  XLOG << "[公会建筑-提交材料]" << m_pGuild->getGUID() << m_pGuild->getName() << "建筑:" << type << "提交加成:" << submitinc << "材料:";
  for (auto& v : material)
    XLOG << v.first << v.second;
  XLOG << "当前已提交材料:";
  for (auto& v : it->second.mapMaterial)
    XLOG << v.first << v.second / 100.0f;
  XLOG << "提交成功" << XEND;

  if (fund > 0)
  {
    m_pGuild->addAsset(fund, false, ESOURCE_GUILD_SUBMIT_MATERIAL);
    if (member)
      thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3718, MsgParams{fund});
    XLOG << "[公会建筑-提交材料]" << m_pGuild->getGUID() << m_pGuild->getName() << "增加资金:" << fund << XEND;
  }

  if (member)
    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3720);

  return true;
}

void GuildBuildingMgr::levelup(EGuildBuilding type, DWORD lv)
{
  if (lv <= 0)
    return;

  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return;
  const SGuildBuildingCFG* cfg = GuildConfig::getMe().getGuildBuildingCFG(type, it->second.dwLv);
  if (cfg == nullptr || cfg->bIsMaxLv)
    return;
  if (GuildConfig::getMe().getGuildBuildingCFG(type, it->second.dwLv + lv) == nullptr)
  {
    XERR << "[公会建筑-升级]" << m_pGuild->getGUID() << m_pGuild->getName() << "建筑:" << type << "等级:" << it->second.dwLv << "增加等级:" << lv << "配置找不到" << XEND;
    return;
  }

  DWORD prelv = it->second.dwLv;

  it->second.dwLv += lv;
  it->second.mapMaterial.clear();
  it->second.bIsBuilding = false;
  m_pGuild->setMark(EGUILDDATA_MISC);

  if (it->second.dwLv > 1)
    m_pGuild->broadcastNpcMsg(MiscConfig::getMe().getGuildBuildingCFG().dwBuildingLvUpNpcID, MiscConfig::getMe().getGuildBuildingCFG().dwBuildingLvUpFinishMsgID, MsgParams(cfg->strName));
  else
    m_pGuild->broadcastNpcMsg(MiscConfig::getMe().getGuildBuildingCFG().dwBuildingLvUpNpcID, MiscConfig::getMe().getGuildBuildingCFG().dwBuildingBuildFinishMsgID, MsgParams(cfg->strName));

  set<EGuildBuilding> types;
  types.insert(type);
  TVecGuildMember& members = m_pGuild->getAllMemberList();
  for (auto& m : members)
  {
    // if (m->isOnline())
    m->buildingLevelUpEffect(types, true);
    m->resetSubmitCountTotal(type);
  }

  // 清空排行榜
  clearSubmitRank(type);

  if (prelv <= 0)
    sendWelfare(type);

  updateDataToDScene(types);
  m_pGuild->getMisc().getQuest().questSyncToZone();

  XLOG << "[公会建筑-升级]" << m_pGuild->getGUID() << m_pGuild->getName() << "建筑:" << type << "等级:" << it->second.dwLv << "建筑升级" << XEND;
}

void GuildBuildingMgr::updateDataToDScene(const set<EGuildBuilding> types)
{
  BuildingUpdateGuildSCmd cmd;
  cmd.set_guildid(m_pGuild->getGUID());

  for (auto& type : types)
  {
    auto it = m_mapBuilding.find(type);
    if (it != m_mapBuilding.end())
      it->second.toData(cmd.add_updates());
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len);

  cmd.set_guildid(0);
  TVecGuildMember& members = m_pGuild->getAllMemberList();
  for (auto& m : members)
  {
    if (m->isOnline() && m_pGuild->getMisc().hasAuth(m->getJob(), EAUTH_ARTIFACT_QUEST))
    {
      cmd.set_charid(m->getCharID());
      PROTOBUF(cmd, send1, len1);
      m->sendCmdToZone(send1, len1);
    }
  }
}

void GuildBuildingMgr::sendWelfare(EGuildBuilding type)
{
  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return;

  DWORD cur = now();
  if (cur < it->second.dwNextWelfareTime)
    return;

  const SGuildBuildingCFG* cfg = GuildConfig::getMe().getGuildBuildingCFG(type, it->second.dwLv);
  if (cfg == nullptr)
    return;

  if (cfg->dwRewardID)
  {
    if (m_pGuild->getMisc().getWelfare().addWelfare(EGUILDWELFARE_BUILDING, cfg->dwRewardID, ESOURCE_GUILD_BUILDING_WELFARE, DWORD(type)) == false)
    {
      XLOG << "[公会-建筑]" << m_pGuild->getGUID() << m_pGuild->getName() << "建筑福利:" << cfg->dwRewardID << "发放失败" << XEND;
      return;
    }
    it->second.dwNextWelfareTime = cur + cfg->dwRewardCycle;
    XLOG << "[公会-建筑]" << m_pGuild->getGUID() << m_pGuild->getName() << "建筑:" << type << "建筑福利:" << cfg->dwRewardID << "下一次发放时间:" << it->second.dwNextWelfareTime << "发放成功" << XEND;
  }
}

DWORD GuildBuildingMgr::getNextWelfareTime(EGuildBuilding type)
{
  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return 0;
  return it->second.dwNextWelfareTime;
}

void GuildBuildingMgr::onAddMember(GMember* member)
{
  if (member == nullptr)
    return;
  if (m_pGuild->getMisc().isFunctionOpen(EGUILDFUNCTION_BUILDING))
    member->setBuildingEffect();
}

DWORD GuildBuildingMgr::getBuildingLevel(EGuildBuilding type)
{
  auto it = m_mapBuilding.find(type);
  if (it == m_mapBuilding.end())
    return 0;
  return it->second.dwLv;
}

void GuildBuildingMgr::querySubmitRank(EGuildBuilding type, GMember* member)
{
  if (member == nullptr)
    return;
  auto it = m_mapBuildingSubmitRank.find(type);
  if (it == m_mapBuildingSubmitRank.end())
  {
    initSubmitRank(type);
    it = m_mapBuildingSubmitRank.find(type);
    if (it == m_mapBuildingSubmitRank.end())
      return;
  }
  PROTOBUF(it->second, send, len);
  member->sendCmdToMe(send, len);
}

void GuildBuildingMgr::initSubmitRank(EGuildBuilding type)
{
  if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX)
    return;

  auto it = m_mapBuildingSubmitRank.find(type);
  if (it != m_mapBuildingSubmitRank.end())
    return;

  QueryBuildingRankGuildCmd& cmd = m_mapBuildingSubmitRank[type];
  cmd.set_type(type);
  TVecGuildMember& members = m_pGuild->getAllMemberList();
  for (auto& m : members)
  {
    const UserGuildBuilding* data = m->getBuildingData(type);
    if (data == nullptr || data->submitcounttotal() <= 0)
      continue;
    BuildingSubmitRankItem* item = cmd.add_items();
    if (item == nullptr)
      continue;
    item->set_charid(m->getCharID());
    item->set_submitcounttotal(data->submitcounttotal());
    item->set_submittime(data->submittime());
  }
}

void GuildBuildingMgr::updateSubmitRank(EGuildBuilding type, GMember* member)
{
  if (member == nullptr)
    return;
  const UserGuildBuilding* data = member->getBuildingData(type);
  if (data == nullptr || data->submitcounttotal() <= 0)
    return;
  auto it = m_mapBuildingSubmitRank.find(type);
  if (it == m_mapBuildingSubmitRank.end())
  {
    initSubmitRank(type);
    it = m_mapBuildingSubmitRank.find(type);
    if (it == m_mapBuildingSubmitRank.end())
      return;
  }
  for (int i = 0; i < it->second.items_size(); ++i)
  {
    if (it->second.items(i).charid() == member->getCharID())
    {
      it->second.mutable_items(i)->set_submitcounttotal(data->submitcounttotal());
      it->second.mutable_items(i)->set_submittime(data->submittime());
      return;
    }
  }
  BuildingSubmitRankItem* item = it->second.add_items();
  if (item == nullptr)
    return;
  item->set_charid(member->getCharID());
  item->set_submitcounttotal(data->submitcounttotal());
  item->set_submittime(data->submittime());
}

void GuildBuildingMgr::clearSubmitRank(EGuildBuilding type)
{
  auto it = m_mapBuildingSubmitRank.find(type);
  if (it == m_mapBuildingSubmitRank.end())
    return;
  it->second.clear_items();
}
