#include "GuildArtifact.h"
#include "Guild.h"
#include "GuildMember.h"
#include "ItemConfig.h"
#include "MiscConfig.h"
#include "GuildServer.h"
#include "LuaManager.h"

bool SGuildArtifactItem::toData(GuildArtifactItem* data)
{
  if (data == nullptr)
    return false;
  data->set_guid(strGuid);
  data->set_itemid(dwItemID);
  data->set_distributecount(dwDistributeCount);
  data->set_retrievetime(dwRetrieveTime);
  data->set_ownerid(qwOwnerID);
  return true;
}

bool SGuildArtifactItem::fromData(const GuildArtifactItem& data)
{
  strGuid = data.guid();
  dwItemID = data.itemid();
  dwDistributeCount = data.distributecount();
  dwRetrieveTime = data.retrievetime();
  qwOwnerID = data.ownerid();
  return true;
}
  
bool SGuildArtifactData::toData(GuildArtifactData* data)
{
  if (data == nullptr)
    return false;
  data->set_type(dwType);
  data->set_producecount(dwProduceCount);
  data->set_maxlevel(dwMaxLevel);
  return true;
}

bool SGuildArtifactData::fromData(const GuildArtifactData& data)
{
  dwType = data.type();
  dwProduceCount = data.producecount();
  dwMaxLevel = data.maxlevel();
  return true;
}

GuildArtifactMgr::GuildArtifactMgr(Guild* guild) : m_pGuild(guild), m_oTenMinTimer(10 * MIN_T)
{
}

GuildArtifactMgr::~GuildArtifactMgr()
{
}

bool GuildArtifactMgr::toData(GuildArtifact* data)
{
  if (data == nullptr)
    return false;
  for (auto& v : m_mapGuid2Artifact)
    v.second.toData(data->add_items());
  for (auto& v : m_mapType2ArtifactData)
    v.second.toData(data->add_datas());
  return true;
}

bool GuildArtifactMgr::fromData(const GuildArtifact& data)
{
  for (int i = 0; i < data.items_size(); ++i)
    m_mapGuid2Artifact[data.items(i).guid()].fromData(data.items(i));
  for (int i = 0; i < data.datas_size(); ++i)
    m_mapType2ArtifactData[data.datas(i).type()].fromData(data.datas(i));
  return true;
}

bool GuildArtifactMgr::toGuildInfo(GuildInfo* data)
{
  if (data == nullptr)
    return false;
  for (auto& v : m_mapGuid2Artifact)
    v.second.toData(data->add_artifactitems());
  return true;
}

void GuildArtifactMgr::timer(DWORD cur)
{
  resetDistributeCount();
  retrieve();
}

DWORD GuildArtifactMgr::getArtifactCount(EGuildBuilding type)
{
  DWORD cnt = 0;
  for (auto& v : m_mapGuid2Artifact)
  {
    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(v.second.dwItemID);
    if (cfg && cfg->eBuildingType == type)
      ++cnt;
  }
  return cnt;
}

DWORD GuildArtifactMgr::getArtifactCountByType(DWORD type, const set<string>& except/* = set<string>{}*/)
{
  DWORD cnt = 0;
  for (auto& v : m_mapGuid2Artifact)
  {
    if (except.find(v.first) != except.end())
      continue;
    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(v.second.dwItemID);
    if (cfg && cfg->dwType == type)
      ++cnt;
  }
  return cnt;
}

SGuildArtifactItem* GuildArtifactMgr::getArtifact(const string& guid)
{
  auto it = m_mapGuid2Artifact.find(guid);
  if (it == m_mapGuid2Artifact.end())
    return nullptr;
  return &it->second;
}

SGuildArtifactData* GuildArtifactMgr::getArtifactData(DWORD type)
{
  auto it = m_mapType2ArtifactData.find(type);
  if (it == m_mapType2ArtifactData.end())
    return nullptr;
  return &it->second;
}

void GuildArtifactMgr::updateArtifactItem(const set<string>& guids)
{
  if (guids.empty())
    return;

  ArtifactUpdateNtfGuildCmd cmd;
  ArtifactUpdateGuildSCmd scmd;

  for (auto& guid : guids)
  {
    SGuildArtifactItem* item = getArtifact(guid);
    if (item)
    {
      item->toData(cmd.add_itemupdates());
      item->toData(scmd.add_itemupdates());
    }
    else
    {
      cmd.add_itemdels(guid);
      scmd.add_itemdels(guid);
    }
  }

  PROTOBUF(cmd, send, len);
  m_pGuild->broadcastCmd(send, len);

  TVecGuildMember& members = m_pGuild->getAllMemberList();
  for (auto member : members)
  {
    if (member == nullptr || member->isOnline() == false)
      continue;
    scmd.set_charid(member->getCharID());
    PROTOBUF(scmd, ssend, slen);
    thisServer->sendCmdToZone(member->getZoneID(), ssend, slen);
  }

  // 同步到公会场景, 用于处理神器npc的gearstatus
  scmd.set_charid(0);
  scmd.set_guildid(m_pGuild->getGUID());
  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmdToZone(m_pGuild->getZoneID(), ssend, slen);
}

void GuildArtifactMgr::notifyAllData(GMember* member)
{
  if (member == nullptr)
    return;
  ArtifactUpdateNtfGuildCmd cmd;
  for (auto& v : m_mapGuid2Artifact)
    v.second.toData(cmd.add_itemupdates());
  for (auto& v : m_mapType2ArtifactData)
    v.second.toData(cmd.add_dataupdates());
  PROTOBUF(cmd, send, len);
  member->sendCmdToMe(send, len);
}

void GuildArtifactMgr::getUnusedArtifact(DWORD itemid, DWORD count, vector<string>& guids)
{
  if (count <= 0)
    return;
  for (auto& v : m_mapGuid2Artifact)
    if (v.second.dwItemID == itemid && v.second.qwOwnerID == 0)
    {
      guids.push_back(v.second.strGuid);
      count -= 1;
      if (count <= 0)
        break;
    }
  if (count > 0)
    guids.clear();
}

void GuildArtifactMgr::giveback(SGuildArtifactItem* item)
{
  if (item == nullptr)
    return;
  item->giveback();
  m_pGuild->setMark(EGUILDDATA_MISC);
  ItemInfo i;
  i.set_id(item->dwItemID);
  i.set_guid(item->strGuid);
  i.set_count(1);
  i.set_source(ESOURCE_ARTIFACT_DISTRIBUTE);
  if (m_pGuild->getPack().addItem(i) != ESYSTEMMSG_ID_MIN)
    XERR << "[公会神器-返还]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << item->dwItemID << "guid:" << item->strGuid << "返还添加包裹失败" << XEND;
  else
    XLOG << "[公会神器-返还]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << item->dwItemID << "guid:" << item->strGuid << "返还添加包裹成功" << XEND;
}

bool GuildArtifactMgr::getProduceMaterial(DWORD itemid, TVecItemInfo& materials)
{
  const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(itemid);
  if (cfg == nullptr)
    return false;

  DWORD procount = 0;
  SGuildArtifactData* d = getArtifactData(cfg->dwType);
  if (d)
    procount = d->dwProduceCount;
  for (auto& v : cfg->mapMaterial)
  {
    for (auto& m : v.second)
    {
      const SItemCFG* icfg = ItemConfig::getMe().getItemCFG(m.first);
      if (icfg == nullptr)
        return false;
      DWORD count = LuaManager::getMe().call<DWORD>("calcArtifactMaterialItemCount", v.first, m.second, procount);
      if (count <= 0)
        return false;
      if (ItemConfig::getMe().isArtifact(icfg->eItemType))
      {
        vector<string> guids;
        getUnusedArtifact(m.first, count, guids);
        if (guids.size() != count)
          return false;
        for (auto& guid : guids)
        {
          ItemInfo item;
          item.set_guid(guid);
          item.set_id(m.first);
          item.set_count(1);
          materials.push_back(item);
        }
      }
      else
      {
        ItemInfo item;
        item.set_id(m.first);
        item.set_count(count);
        combinItemInfo(materials, TVecItemInfo{item});
      }
    }
  }

  return true;
}

bool GuildArtifactMgr::produce(GMember* member, DWORD itemid, bool noreduceitem/* = false*/)
{
  if (member == nullptr)
    return false;
  if (m_pGuild->getMisc().hasAuth(member->getJob(), EAUTH_ARTIFACT_PRODUCE) == false)
  {
    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 4019);
    return false;
  }

  if (ItemConfig::getMe().getItemCFG(itemid) == nullptr)
    return false;
  const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(itemid);
  if (cfg == nullptr)
    return false;
  const SGuildBuildingCFG* buildingcfg = m_pGuild->getMisc().getBuilding().getBuildingCFG(cfg->eBuildingType);
  if (buildingcfg == nullptr)
    return false;

  if (cfg->dwQuestID && m_pGuild->getMisc().getQuest().isSubmit(cfg->dwQuestID) == false)
    return false;

  set<string> delguids;
  if (noreduceitem == false)
  {
    TVecItemInfo materials;
    if (getProduceMaterial(itemid, materials) == false)
      return false;
    for (auto& v : materials)
      if (v.guid().empty() == false)
        delguids.insert(v.guid());
    if (getArtifactCount(cfg->eBuildingType) >= buildingcfg->dwArtifactMaxCount + delguids.size() || getArtifactCountByType(cfg->dwType, delguids) >= buildingcfg->dwArtifactTypeMaxCount)
      return false;
    if (m_pGuild->getPack().reduceItem(materials, ESOURCE_ARTIFACT_PRODUCE) != ESYSTEMMSG_ID_MIN)
      return false;
  }

  if (addArtifact(itemid) == false)
    return false;
  removeArtifact(delguids);

  SGuildArtifactData& data = m_mapType2ArtifactData[cfg->dwType];
  data.dwType = cfg->dwType;
  data.dwProduceCount += 1;
  if (data.dwMaxLevel < cfg->dwLevel)
  {
    data.dwMaxLevel = cfg->dwLevel;
    const SArtifactCFG* nextcfg = GuildConfig::getMe().getArtifactCFG(cfg->dwNextLevelID);
    if (nextcfg)
      thisServer->sendMsg(member->getZoneID(), member->getCharID(), nextcfg->dwUnlockMsg);
  }

  m_pGuild->setMark(EGUILDDATA_MISC);

  ArtifactUpdateNtfGuildCmd cmd;
  data.toData(cmd.add_dataupdates());
  PROTOBUF(cmd, send, len);
  m_pGuild->broadcastCmd(send, len);

  m_pGuild->getEvent().addEvent(EGUILDEVENT_ARTIFACT_PRODUCE, TVecString{member->getName(), cfg->strName});

  const SArtifactBuildingMiscCFG* misccfg = MiscConfig::getMe().getArtifactCFG().getBuildingCFG(cfg->eBuildingType);
  if (misccfg)
    m_pGuild->broadcastNpcMsg(misccfg->dwProduceNpcID, misccfg->dwProduceMsgID, MsgParams(cfg->strName, member->getName()));

  XLOG << "[公会神器-打造]" << m_pGuild->getGUID() << m_pGuild->getName() << "玩家:" << member->getAccid() << member->getCharID() << member->getName() << "itemid:" << itemid << "打造成功" << XEND;
  return true;
}

bool GuildArtifactMgr::addArtifact(DWORD itemid)
{
  ItemInfo aitem;
  aitem.set_id(itemid);
  aitem.set_count(1);
  aitem.set_source(ESOURCE_ARTIFACT_PRODUCE);
  if (m_pGuild->getPack().addItem(aitem) != ESYSTEMMSG_ID_MIN)
    return false;

  string guid = m_pGuild->getPack().getGUIDByType(itemid);
  if (guid.empty())
    return false;
  if (m_mapGuid2Artifact.find(guid) != m_mapGuid2Artifact.end())
    return false;
  SGuildArtifactItem& item = m_mapGuid2Artifact[guid];
  item.strGuid = guid;
  item.dwItemID = itemid;
  updateArtifactItem(set<string>{guid});

  m_pGuild->setMark(EGUILDDATA_MISC);

  XLOG << "[公会神器-添加]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << itemid << "guid:" << guid << "添加成功" << XEND;
  return true;
}

void GuildArtifactMgr::removeArtifact(const set<string>& guids)
{
  if (guids.empty())
    return;
  set<string> dels;
  for (auto& guid : guids)
  {
    auto it = m_mapGuid2Artifact.find(guid);
    if (it == m_mapGuid2Artifact.end())
      continue;
    XLOG << "[公会神器-删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << it->second.dwItemID << "guid:" << guid << "删除成功" << XEND;
    m_mapGuid2Artifact.erase(it);
    dels.insert(guid);
    m_pGuild->setMark(EGUILDDATA_MISC);
  }
  updateArtifactItem(dels);
}

bool GuildArtifactMgr::operate(GMember* member, EArtifactOptType type, const set<string>& guids, QWORD charid)
{
  switch (type)
  {
  case EARTIFACTOPTTYPE_DISTRIBUTE: // 分配
  {
    if (distribute(member, guids, charid) == false)
      return false;
    break;
  }
  case EARTIFACTOPTTYPE_RETRIEVE: // 收回
  {
    if (retrieve(member, guids) == false)
      return false;
    break;
  }
  case EARTIFACTOPTTYPE_RETRIEVE_CANCEL: // 取消收回
  {
    if (cancelRetrieve(member, guids) == false)
      return false;
    break;
  }
  case EARTIFACTOPTTYPE_GIVEBACK: // 归还
  {
    if (giveback(member, guids) == false)
      return false;
    break;
  }
  default:
    return false;
  }

  return true;
}

void GuildArtifactMgr::resetDistributeCount()
{
  if (m_pGuild->getMisc().getVarValue(EVARTYPE_ARTIFACT_DISTRIBUTE_DAY) == 1)
    return;

  TSetString guids;
  for (auto& v : m_mapGuid2Artifact)
  {
    if (v.second.dwDistributeCount > 0)
    {
      v.second.dwDistributeCount = 0;
      guids.insert(v.second.strGuid);
    }
  }
  if (guids.empty() == false)
    updateArtifactItem(guids);
  m_pGuild->getMisc().setVarValue(EVARTYPE_ARTIFACT_DISTRIBUTE_DAY, 1);
  m_pGuild->setMark(EGUILDDATA_MISC);
}

// 分配神器
bool GuildArtifactMgr::distribute(GMember* member, const set<string>& guids, QWORD targetid)
{
  if (member == nullptr)
    return false;

  if (m_pGuild->getMisc().hasAuth(member->getJob(), EAUTH_ARTIFACT_OPT) == false)
  {
    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 4021);
    return false;
  }

  GMember* target = m_pGuild->getMember(targetid);
  if (target == nullptr)
  {
    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 4024);
    return false;
  }

  resetDistributeCount();

  set<string> updates;
  for (auto& guid : guids)
  {
    SGuildArtifactItem* item = getArtifact(guid);
    if (item == nullptr)
      continue;
    if (item->qwOwnerID)
      continue;

    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(item->dwItemID);
    if (cfg == nullptr)
      continue;

    const SItemCFG* itemcfg = ItemConfig::getMe().getItemCFG(item->dwItemID);
    if (itemcfg == nullptr)
      continue;
    if (itemcfg->vecEquipPro.empty() == false)
    {
      bool canequip = false;
      for (auto pro : itemcfg->vecEquipPro)
        if (pro == target->getProfession())
        {
          canequip = true;
          break;
        }
      if (canequip == false)
        continue;
    }

    if (item->dwDistributeCount >= cfg->dwDistributeCount)
      continue;

    if (m_pGuild->getPack().reduceItem(item->strGuid, 1, ESOURCE_ARTIFACT_DISTRIBUTE) != ESYSTEMMSG_ID_MIN)
      continue;

    item->dwDistributeCount += 1;
    item->qwOwnerID = target->getCharID();

    m_pGuild->getEvent().addEvent(EGUILDEVENT_ARTIFACT_DISTRIBUTE, TVecString{member->getName(), cfg->strName, target->getName()});

    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3802);
    if (target->isOnline())
      thisServer->sendMsg(target->getZoneID(), target->getCharID(), 3803, MsgParams{member->getName(), cfg->strName});

    updates.insert(guid);

    XLOG << "[公会神器-分配]" << m_pGuild->getGUID() << m_pGuild->getName() << "guid:" << guid << "itemid:" << item->dwItemID << "charid:" << member->getCharID() << "分配成功" << XEND;
  }

  if (updates.empty() == false)
  {
    m_pGuild->setMark(EGUILDDATA_MISC);
    updateArtifactItem(updates);
    return true;
  }
  return false;
}

void GuildArtifactMgr::clearDistributeCount()
{
  set<string> guids;
  for (auto& v : m_mapGuid2Artifact)
  {
    if (v.second.dwDistributeCount > 0)
    {
      v.second.dwDistributeCount = 0;
      guids.insert(v.first);
    }
  }
  if (guids.empty() == false)
  {
    m_pGuild->setMark(EGUILDDATA_MISC);
    updateArtifactItem(guids);
  }
}

bool GuildArtifactMgr::retrieve(GMember* member, const set<string>& guids)
{
  if (member == nullptr || m_pGuild->getMisc().hasAuth(member->getJob(), EAUTH_ARTIFACT_OPT) == false)
    return false;

  DWORD cur = now(), cd = MiscConfig::getMe().getArtifactCFG().dwRetrieveCD;
  set<string> updates;
  for (auto& guid : guids)
  {
    SGuildArtifactItem* item = getArtifact(guid);
    if (item == nullptr)
      continue;
    if (item->qwOwnerID == 0 || item->dwRetrieveTime > 0)
    {
      thisServer->sendMsg(member->getZoneID(), member->getCharID(), 4023);
      continue;
    }

    item->dwRetrieveTime = cur + cd;

    GMember* owner = m_pGuild->getMember(item->qwOwnerID);
    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(item->dwItemID);
    if (cfg && owner)
      m_pGuild->getEvent().addEvent(EGUILDEVENT_ARTIFACT_RETRIEVE, TVecString{member->getName(), owner->getName(), cfg->strName});

    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3799, MsgParams{cd});
    if (cfg && owner && owner->isOnline())
    {
      MsgParams params(cfg->strName, member->getName());
      params.addNumber(cd);
      thisServer->sendMsg(owner->getZoneID(), owner->getCharID(), 3801, params);
    }

    updates.insert(guid);

    XLOG << "[公会神器-收回]" << m_pGuild->getGUID() << m_pGuild->getName() << "guid:" << guid << "收回时间:" << item->dwRetrieveTime << "收回成功" << XEND;
  }

  if (updates.empty() == false)
  {
    m_pGuild->setMark(EGUILDDATA_MISC);
    updateArtifactItem(updates);
    return true;
  }
  return false;
}

void GuildArtifactMgr::retrieve(bool force/* = false*/)
{
  DWORD cur = now();
  set<string> guids;
  bool checkrmmember = m_oTenMinTimer.timeUp(now());
  for (auto& v : m_mapGuid2Artifact)
  {
    if (v.second.dwRetrieveTime > 0 && (force || (cur >= v.second.dwRetrieveTime && ItemConfig::getMe().getItemCFG(v.second.dwItemID))))
    {
      giveback(&v.second);
      guids.insert(v.first);
      continue;
    }
    if (checkrmmember && v.second.qwOwnerID && m_pGuild->getMember(v.second.qwOwnerID) == nullptr)
    {
      XLOG << "[公会神器-收回定时器]" << m_pGuild->getGUID() << m_pGuild->getName() << "guid:" << v.second.strGuid << "owner:" << v.second.qwOwnerID << "成员不存在,回收神器" << XEND;
      giveback(&v.second);
      guids.insert(v.first);
      continue;
    }
  }
  if (guids.empty() == false)
  {
    m_pGuild->setMark(EGUILDDATA_MISC);
    updateArtifactItem(guids);
  }
}

bool GuildArtifactMgr::cancelRetrieve(GMember* member, const set<string>& guids)
{
  if (member == nullptr || m_pGuild->getMisc().hasAuth(member->getJob(), EAUTH_ARTIFACT_OPT) == false)
    return false;

  set<string> updates;
  for (auto& guid : guids)
  {
    SGuildArtifactItem* item = getArtifact(guid);
    if (item == nullptr)
      continue;
    if (item->qwOwnerID == 0 || item->dwRetrieveTime <= 0)
      continue;

    item->dwRetrieveTime = 0;

    updates.insert(guid);

    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3804);
    GMember* owner = m_pGuild->getMember(item->qwOwnerID);
    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(item->dwItemID);
    if (cfg && owner && owner->isOnline())
      thisServer->sendMsg(owner->getZoneID(), owner->getCharID(), 3805, MsgParams{member->getName(), cfg->strName});

    XLOG << "[公会神器-取消收回]" << m_pGuild->getGUID() << m_pGuild->getName() << "guid:" << guid << "取消收回成功" << XEND;
  }

  if (updates.empty() == false)
  {
    m_pGuild->setMark(EGUILDDATA_MISC);
    updateArtifactItem(updates);
    return true;
  }
  return false;
}

bool GuildArtifactMgr::giveback(GMember* member, const set<string>& guids)
{
  if (member == nullptr)
    return false;

  set<string> updates;
  for (auto& guid : guids)
  {
    SGuildArtifactItem* item = getArtifact(guid);
    if (item == nullptr)
      continue;

    GMember* owner = m_pGuild->getMember(item->qwOwnerID);
    if (owner == nullptr || owner->getCharID() != member->getCharID())
      continue;

    if (ItemConfig::getMe().getItemCFG(item->dwItemID) == nullptr)
      continue;

    giveback(item);

    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(item->dwItemID);
    if (cfg)
    {
      m_pGuild->getEvent().addEvent(EGUILDEVENT_ARTIFACT_GIVEBACK, TVecString{member->getName(), cfg->strName});
      thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3788, MsgParams{member->getName(), cfg->strName});
    }

    updates.insert(guid);

    XLOG << "[公会神器-归还]" << m_pGuild->getGUID() << m_pGuild->getName() << "guid:" << guid << "归还成功" << XEND;
  }

  if (updates.empty() == false)
  {
    m_pGuild->setMark(EGUILDDATA_MISC);
    updateArtifactItem(updates);
    return true;
  }
  return false;
}

void GuildArtifactMgr::onRemoveMember(GMember* member)
{
  if (member == nullptr)
    return;

  set<string> guids;
  for (auto& v : m_mapGuid2Artifact)
    if (v.second.qwOwnerID == member->getCharID())
      guids.insert(v.first);
  if (guids.empty() == false)
    giveback(member, guids);
}

// 若神器记录显示未分配, 公会包裹却找不到神器, 则自动添加神器到公会包裹, 反之亦然
void GuildArtifactMgr::fixPack(bool force/* = false*/)
{
  if (force == false && m_bFixed)
    return;

  for (auto& v : m_mapGuid2Artifact)
  {
    if (v.second.qwOwnerID)     // 已被分配的神器
    {
      SGuildItem* item = m_pGuild->getPack().getItem(v.second.strGuid);
      if (item)
      {
        if (m_pGuild->getPack().reduceItem(v.second.strGuid, 1, ESOURCE_ARTIFACT_DISTRIBUTE) != ESYSTEMMSG_ID_MIN)
          XERR << "[公会神器-修复包裹]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << v.second.dwItemID << "guid:" << v.second.strGuid << "从包裹删除失败" << XEND;
        else
          XLOG << "[公会神器-修复包裹]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << v.second.dwItemID << "guid:" << v.second.strGuid << "从包裹删除成功" << XEND;
      }
    }
    else                        // 未分配的神器
    {
      SGuildItem* item = m_pGuild->getPack().getItem(v.second.strGuid);
      if (item == nullptr)
      {
        ItemInfo i;
        i.set_id(v.second.dwItemID);
        i.set_guid(v.second.strGuid);
        i.set_count(1);
        i.set_source(ESOURCE_ARTIFACT_DISTRIBUTE);
        if (m_pGuild->getPack().addItem(i) != ESYSTEMMSG_ID_MIN)
          XERR << "[公会神器-修复包裹]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << v.second.dwItemID << "guid:" << v.second.strGuid << "添加包裹失败" << XEND;
        else
          XLOG << "[公会神器-修复包裹]" << m_pGuild->getGUID() << m_pGuild->getName() << "itemid:" << v.second.dwItemID << "guid:" << v.second.strGuid << "添加包裹成功" << XEND;
      }
    }
  }
  m_bFixed = true;
}
