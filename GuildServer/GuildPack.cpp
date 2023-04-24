#include "GuildPack.h"
#include "Guild.h"
#include "MiscConfig.h"
#include "QuestConfig.h"
#include "GuidManager.h"
#include "PlatLogManager.h"
#include "GuildServer.h"

GuildPack::GuildPack(Guild *pGuild) : m_pGuild(pGuild)
{
}

GuildPack::~GuildPack()
{
}

void GuildPack::init()
{
  switch (m_dwInitStatus)
  {
    case GUILD_BLOB_INIT_NULL:
      {
        XDBG << "[工会-包裹]" << m_pGuild->getName() << "初始化失败，没有数据" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_COPY_DATA:
      {
        fromPackString(m_oBlobPack);
        m_dwInitStatus = GUILD_BLOB_INIT_OK;
        XDBG << "[工会-包裹]" << m_pGuild->getName() << "初始化成功" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_OK:
      break;
    default:
      break;
  }
}

void GuildPack::setBlobPackString(const char* str, DWORD len)
{
  if (GUILD_BLOB_INIT_OK == m_dwInitStatus)
  {
    XERR << "[工会-包裹]" << m_pGuild->getGUID() << m_pGuild->getName() << "初始化成功后，设置数据" << XEND;
    return;
  }
  m_dwInitStatus = GUILD_BLOB_INIT_COPY_DATA;
  m_oBlobPack.assign(str, len);
  XDBG << "[工会-包裹]" << m_pGuild->getName() << "设置blob string" << XEND;
}

bool GuildPack::toBlobPackString(string& str)
{
  if (GUILD_BLOB_INIT_COPY_DATA == m_dwInitStatus)
  {
    str.assign(m_oBlobPack.c_str(), m_oBlobPack.size());
    return true;
  }

  if (GUILD_BLOB_INIT_OK != m_dwInitStatus) return true;

  BlobGuildPack oBlob;
  for (auto &m : m_mapID2Item)
    oBlob.add_data()->CopyFrom(m.second.oData);

  if (oBlob.SerializeToString(&str) == false)
  {
    XERR << "[公会仓库-保存]" << m_pGuild->getGUID() << m_pGuild->getName() << "保存失败,序列化失败" << XEND;
    return false;
  }

  XLOG << "[公会仓库-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "保存成功,数据大小 :" << oBlob.ByteSize() << XEND;
  return true;
}

bool GuildPack::fromPackString(const string& str)
{
  BlobGuildPack oBlob;
  if (oBlob.ParseFromString(str) == false)
  {
    XERR << "[公会仓库-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载失败,反序列化失败" << XEND;
    return false;
  }

  for (int i = 0; i < oBlob.item_size(); ++i)
  {
    ItemInfo* pItem = oBlob.mutable_item(i);
    if (pItem->id() == MiscConfig::getMe().getGuildCFG().dwAssetItemID)
    {
      m_pGuild->addAsset(pItem->count(), true);
      continue;
    }
    if (pItem->guid().empty() == true)
      pItem->set_guid(GuidManager::getMe().newGuidStr(m_pGuild->getGUID(), 9999));
    m_mapID2Item[pItem->guid()].oData.mutable_base()->CopyFrom(*pItem);
    m_mapType2Item[pItem->id()].insert(pItem->guid());
  }

  for (int i = 0; i < oBlob.data_size(); ++i)
  {
    const ItemData& rData = oBlob.data(i);
    SGuildItem& rItem = m_mapID2Item[rData.base().guid()];
    rItem.oData.CopyFrom(rData);
    rItem.pCFG = ItemConfig::getMe().getItemCFG(rData.base().id());
    m_mapType2Item[rData.base().id()].insert(rData.base().guid());
  }

  return true;
}

void GuildPack::toData(QueryPackGuildCmd& cmd)
{
  for (auto &m : m_mapID2Item)
    cmd.add_items()->CopyFrom(m.second.oData);
  XDBG << "[公会仓库-请求]" << m_pGuild->getGUID() << m_pGuild->getName() << "查询 :" << cmd.ShortDebugString() << XEND;
}

void GuildPack::fetch(const TSetString& setIDs, PackUpdateGuildCmd& cmd)
{
  for (auto &s : setIDs)
  {
    SGuildItem* pItem = getItem(s);
    if (pItem != nullptr)
      cmd.add_updates()->CopyFrom(pItem->oData);
    else
      cmd.add_dels(s);
  }
}

void GuildPack::update(bool bNoCache /*= false*/)
{
  if (m_setUpdateIDs.empty() == true)
    return;

  PackUpdateGuildCmd cmd;
  fetch(m_setUpdateIDs, cmd);
  if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    TVecGuildMember& vecMembers = m_pGuild->getAllMemberList();
    for (auto &v : vecMembers)
    {
      if (v->isOnline() == false)
        continue;
      if (bNoCache || v->getFrameStatus() == true)
      {
        v->sendCmdToMe(send, len);
        XDBG << "[公会仓库-更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "成员" << v->getAccid() << v->getCharID() << v->getProfession() << v->getName()
          << "收到仓库更新" << cmd.ShortDebugString() << XEND;
        continue;
      }
      for (auto &s : m_setUpdateIDs)
        v->addPackUpdate(s);
    }

    XLOG << "[公会仓库-更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "更新仓库" << cmd.ShortDebugString() << XEND;
  }

  m_setUpdateIDs.clear();
}

string GuildPack::getGUIDByType(DWORD dwTypeID)
{
  auto m = m_mapType2Item.find(dwTypeID);
  if (m == m_mapType2Item.end())
    return STRING_EMPTY;

  string guid;
  DWORD dwTime = 0;
  for (auto &s : m->second)
  {
    SGuildItem* pItem = getItem(s);
    if (pItem == nullptr)
      continue;
    if (pItem->getCreateTime() > dwTime)
    {
      dwTime = pItem->getCreateTime();
      guid = s;
    }
  }

  return guid;
}

DWORD GuildPack::getItemCount(DWORD dwTypeID)
{
  auto m = m_mapType2Item.find(dwTypeID);
  if (m == m_mapType2Item.end())
    return 0;

  DWORD dwTotal = 0;
  for (auto &s : m->second)
  {
    SGuildItem* pItem = getItem(s);
    if (pItem != nullptr)
      dwTotal += pItem->getCount();
  }

  return dwTotal;
}

SGuildItem* GuildPack::getItem(const string& guid)
{
  auto m = m_mapID2Item.find(guid);
  if (m == m_mapID2Item.end())
  {
    m_setUpdateIDs.insert(guid);
    return nullptr;
  }
  return &m->second;
}

ESysMsgID GuildPack::addItem(const ItemInfo& rInfo)
{
  ItemData oData;
  oData.mutable_base()->CopyFrom(rInfo);
  return addItem(oData);
}

ESysMsgID GuildPack::addItem(const TVecItemInfo& vecInfo)
{
  TVecItemData vecData;
  for (auto &v : vecInfo)
  {
    ItemData oData;
    oData.mutable_base()->CopyFrom(v);
    combineItemData(vecData, oData);
  }
  return addItem(vecData);
}

ESysMsgID GuildPack::addItem(const ItemData& rData)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rData.base().id());
  if (pCFG == nullptr)
  {
    XERR << "[公会仓库-添加物品]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << rData.ShortDebugString() << "失败,未在 Table_Item.txt 表中找到" << XEND;
    return ESYSTEMMSG_ID_PACK_FULL;
  }

  if (pCFG->eItemType == EITEMTYPE_ASSET)
  {
    m_pGuild->addAsset(rData.base().count(), false, rData.base().source());
    return ESYSTEMMSG_ID_MIN;
  }

  if (pCFG->dwMaxNum == 0)
  {
    XERR << "[公会仓库-添加物品]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << rData.ShortDebugString() << "失败,堆叠为0" << XEND;
    return ESYSTEMMSG_ID_PACK_FULL;
  }

  if (ItemConfig::getMe().isGuild(pCFG->eItemType) == false)
  {
    XERR << "[公会仓库-添加物品]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << rData.ShortDebugString() << "失败,该道具无法添加到公会仓库" << XEND;
    return ESYSTEMMSG_ID_PACK_FULL;
  }

  SQWORD sqwChanged = 0;

  auto additem = [&](const ItemData& rData)
  {
    string guid = rData.base().guid();
    if (guid.empty() == true)
      guid = GuidManager::getMe().newGuidStr(m_pGuild->getGUID(), 9999);

    // check duplicate
    auto o = m_mapID2Item.find(guid);
    if (o != m_mapID2Item.end())
    {
      XERR << "[公会仓库-添加物品]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << rData.ShortDebugString() << "失败,guid :" << guid << "已存在" << XEND;
      return;
    }

    SGuildItem& rNewItem = m_mapID2Item[guid];
    rNewItem.oData.CopyFrom(rData);
    rNewItem.oData.mutable_base()->set_guid(guid);
    rNewItem.setCreateTime(xTime::getCurSec());
    if (rNewItem.getCount() == 0)
      rNewItem.setCount(1);
    rNewItem.pCFG = pCFG;

    m_mapType2Item[rData.base().id()].insert(guid);
    m_setUpdateIDs.insert(guid);
    sqwChanged += rNewItem.getCount();

    XLOG << "[公会仓库-添加物品]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << rData.ShortDebugString() << "成功,单独创建一个堆叠为" << rData.base().count() << "个的道具" << XEND;
  };

  ItemData oData;
  oData.CopyFrom(rData);

  auto m = m_mapType2Item.find(oData.base().id());
  if (m != m_mapType2Item.end())
  {
    for (auto &s : m->second)
    {
      auto item = m_mapID2Item.find(s);
      if (item == m_mapID2Item.end())
        continue;

      if (pCFG->dwMaxNum == 1)
        continue;

      SGuildItem& rItem = item->second;
      DWORD dwLeft = pCFG->dwMaxNum > rItem.getCount() ? pCFG->dwMaxNum - rItem.getCount() : 0;
      if (dwLeft == 0)
        continue;

      if (oData.base().count() < dwLeft)
      {
        rItem.setCount(rItem.getCount() + oData.base().count());
        m_setUpdateIDs.insert(rItem.getGUID());
        sqwChanged += oData.base().count();

        XLOG << "[公会仓库-添加物品]" << m_pGuild->getGUID() << m_pGuild->getName()
          << "添加" << oData.ShortDebugString() << "成功,在" << rItem.getGUID() << "堆叠了" << oData.base().count() << "个,总" << rItem.getCount() << "个" << XEND;
        oData.mutable_base()->set_count(0);
        continue;
      }

      rItem.setCount(rItem.getCount() + dwLeft);
      m_setUpdateIDs.insert(rItem.getGUID());
      sqwChanged += dwLeft;
      XLOG << "[公会仓库-添加物品]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "添加" << oData.ShortDebugString() << "成功,在" << rItem.getGUID() << "堆叠了" << dwLeft << "个,总" << rItem.getCount() << "个" << XEND;
      oData.mutable_base()->set_count(rData.base().count() - dwLeft);
    }
  }

  // add NEW item
  ItemData oCopy;
  oCopy.CopyFrom(oData);
  while (oData.base().count() > 0)
  {
    if (pCFG->dwMaxNum >= oData.base().count())
    {
      oCopy.mutable_base()->set_count(oData.base().count());
      oData.mutable_base()->set_count(0);
    }
    else
    {
      oCopy.mutable_base()->set_count(pCFG->dwMaxNum);
      oData.mutable_base()->set_count(oData.base().count() - pCFG->dwMaxNum);
    }

    additem(oCopy);
  }

  update();
  if (QuestConfig::getMe().isArtifactPiece(rData.base().id()) == true)
    m_pGuild->getMisc().getQuest().questSyncToZone();

  m_pGuild->setMark(EGUILDDATA_PACK);

  itemLog(oData.base().id(), sqwChanged, getItemCount(oData.base().id()), oData.base().source());

  return ESYSTEMMSG_ID_MIN;
}

ESysMsgID GuildPack::addItem(const TVecItemData& vecData)
{
  // check slot

  for (auto &v : vecData)
    addItem(v);

  return ESYSTEMMSG_ID_MIN;
}

ESysMsgID GuildPack::reduceItem(const string& guid, DWORD dwCount /*= 1*/, ESource eSource /*= ESOURCE_NORMAL*/)
{
  auto m = m_mapID2Item.find(guid);
  if (m == m_mapID2Item.end())
  {
    XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << guid << "失败,未找到该物品" << XEND;
    return ESYSTEMMSG_ID_PACK_FULL;
  }
  if (m->second.getCount() < dwCount)
  {
    XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << guid << "失败,数量不足,total :" << m->second.getCount() << "扣除" << dwCount << XEND;
    return ESYSTEMMSG_ID_PACK_FULL;
  }

  DWORD itemid = m->second.getItemID();
  m->second.setCount(m->second.getCount() - dwCount);

  if (m->second.getCount() == 0)
  {
    m_mapID2Item.erase(m);
    for (auto m = m_mapType2Item.begin(); m != m_mapType2Item.end();)
    {
      for (auto s = m->second.begin(); s != m->second.end();)
      {
        if (*s == guid)
          s = m->second.erase(s);
        else
          ++s;
      }

      if (m->second.empty() == true)
        m = m_mapType2Item.erase(m);
      else
        ++m;
    }
  }
  m_setUpdateIDs.insert(guid);
  update();
  m_pGuild->setMark(EGUILDDATA_PACK);
  XLOG << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << guid << dwCount << eSource << "成功" << XEND;

  itemLog(itemid, -SQWORD(dwCount), getItemCount(itemid), eSource);

  return ESYSTEMMSG_ID_MIN;
}

ESysMsgID GuildPack::reduceItem(DWORD dwItemID, DWORD dwCount /*= 1*/, ESource eSource /*= ESOURCE_NORMAL*/)
{
  auto m = m_mapType2Item.find(dwItemID);
  if (m == m_mapType2Item.end())
  {
    XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << dwItemID << "失败,未找到该物品" << XEND;
    return ESYSTEMMSG_ID_PACK_FULL;
  }

  DWORD dwTotal = 0;
  TSetString setItems;
  for (auto &s : m->second)
  {
    auto item = m_mapID2Item.find(s);
    if (item == m_mapID2Item.end())
    {
      m_setUpdateIDs.insert(s);
      continue;
    }
    dwTotal += item->second.getCount();
    setItems.insert(item->second.getGUID());
  }
  if (dwCount > dwTotal)
  {
    XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << dwItemID << "失败,数量不足,总数量" << dwTotal << "需要扣除" << dwCount << XEND;
    return ESYSTEMMSG_ID_PACK_FULL;
  }

  for (auto &s : setItems)
  {
    auto item = m_mapID2Item.find(s);
    if (item == m_mapID2Item.end())
    {
      m_setUpdateIDs.insert(s);
      continue;
    }
    if (item->second.getCount() > dwCount)
    {
      reduceItem(item->second.getGUID(), dwCount, eSource);
      XLOG << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "删除" << dwItemID << "成功,在guid :" << item->first << "减少" << dwCount << "个,剩余" << item->second.getCount() << XEND;
      return ESYSTEMMSG_ID_MIN;
    }

    dwCount -= item->second.getCount();
    XLOG << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName()
      << "删除" << dwItemID << "成功,guid :" << item->second.oData.ShortDebugString() << "被删除,剩余" << dwCount << "待删除" << XEND;
    reduceItem(item->second.getGUID(), item->second.getCount(), eSource);
  }

  return ESYSTEMMSG_ID_MIN;
}

ESysMsgID GuildPack::reduceItem(const TVecItemInfo& vecItems, ESource eSource)
{
  TVecItemData vecData;
  for (auto &v : vecItems)
  {
    ItemData oData;
    oData.mutable_base()->CopyFrom(v);
    combineItemData(vecData, oData);
  }
  return reduceItem(vecData, eSource);
}

ESysMsgID GuildPack::reduceItem(const TVecItemData& vecDatas, ESource eSource)
{
  for (auto &v : vecDatas)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.base().id());
    if (pCFG == nullptr)
    {
      XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << v.ShortDebugString() << "失败,未在 Table_Item.txt 表中找到" << XEND;
      return ESYSTEMMSG_ID_PACK_FULL;
    }
    if (ItemConfig::getMe().isArtifact(pCFG->eItemType) && v.base().guid().empty() == true)
    {
      XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << v.ShortDebugString() << "失败,神器未指定guid" << XEND;
      return ESYSTEMMSG_ID_PACK_FULL;
    }

    if (v.base().guid().empty() == false)
    {
      auto item = m_mapID2Item.find(v.base().guid());
      if (item == m_mapID2Item.end())
      {
        XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << v.ShortDebugString() << "失败,未找到guid道具" << XEND;
        return ESYSTEMMSG_ID_PACK_FULL;
      }
      if (item->second.getCount() < v.base().count())
      {
        XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName()
          << "删除" << v.ShortDebugString() << "失败,数量不足,total :" << item->second.getCount() << "需要扣除" << v.base().count() << XEND;
        return ESYSTEMMSG_ID_PACK_FULL;
      }
    }
    else
    {
      auto item = m_mapType2Item.find(v.base().id());
      if (item == m_mapType2Item.end())
      {
        XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除" << v.ShortDebugString() << "失败,未找到道具" << XEND;
        return ESYSTEMMSG_ID_PACK_FULL;
      }
      DWORD dwTotal = 0;
      for (auto &s : item->second)
      {
        auto m = m_mapID2Item.find(s);
        if (m == m_mapID2Item.end())
        {
          m_setUpdateIDs.insert(s);
          continue;
        }
        dwTotal += m->second.getCount();
      }
      if (dwTotal < v.base().count())
      {
        XERR << "[公会仓库-物品删除]" << m_pGuild->getGUID() << m_pGuild->getName()
          << "删除" << v.ShortDebugString() << "失败,数量不足,total :" << dwTotal << "需要扣除" << v.base().count() << XEND;
        return ESYSTEMMSG_ID_PACK_FULL;
      }
    }
  }

  for (auto &v : vecDatas)
  {
    if (v.base().guid().empty() == false)
      reduceItem(v.base().guid(), v.base().count(), eSource);
    else
      reduceItem(v.base().id(), v.base().count(), eSource);
  }

  return ESYSTEMMSG_ID_MIN;
}

void GuildPack::itemLog(DWORD dwItemID, SQWORD sqwChanged, DWORD dwCount, ESource eSource)
{
  PlatLogManager::getMe().GuildItemLog(thisServer, m_pGuild->getGUID(), dwItemID, sqwChanged, dwCount, eSource);
}
