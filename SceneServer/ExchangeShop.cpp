#include "ExchangeShop.h"
#include "SceneUser.h"
#include "MiscConfig.h"

ExchangeShop::ExchangeShop(SceneUser* pUser) : m_pUser(pUser)
{
  m_mapExchangeItem.clear();
  m_vecDelIDs.clear();
}

ExchangeShop::~ExchangeShop()
{
}

bool ExchangeShop::load(const BlobExchangeShop& data)
{
  m_bMenuOpen = data.menuopen();
  m_mapExchangeItem.clear();
  for (int i = 0; i < data.items_size(); i++)
  {
    const ExchangeShopItem& item = data.items(i);

    TPtrExchangeItemConfig pConfig = ExchangeShopManager::getMe().getShopItemById(item.id());
    if (pConfig == nullptr || pConfig->isBranch() == true)
    {
      XERR << "[兑换商店-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加载" << item.ShortDebugString() << "被忽略,配置未找到或者属于分支职业" << XEND;
      continue;
    }

    ExchangeShopItemData shopItem;
    if (shopItem.init(item.id()))
    {
      shopItem.fromData(item);
      m_mapExchangeItem[item.id()] = shopItem;
    }
  }

  m_mapItemGetCount.clear();
  for (int i = 0; i < data.itemget_size(); ++i)
  {
    m_mapItemGetCount[data.itemget(i).itemid()] = data.itemget(i).getcount();
  }

  m_setHasExchangedItem.clear();
  for (int i= 0; i < data.exchanged_goods_size(); ++i)
  {
    m_setHasExchangedItem.insert(data.exchanged_goods(i));
  }
  return true;
}

bool ExchangeShop::save(BlobExchangeShop* data)
{
  data->set_menuopen(m_bMenuOpen);
  for (auto& it : m_mapExchangeItem)
  {
    TPtrExchangeItemConfig pConfig = ExchangeShopManager::getMe().getShopItemById(it.first);
    if (pConfig == nullptr || pConfig->isBranch() == true)
    {
      XDBG << "[兑换商店-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存储列表 id :" << it.first << "被忽略,配置未找到或者属于分支职业" << XEND;
      continue;
    }
    ExchangeShopItem* pItem = data->add_items();
    if (pItem == nullptr)
      continue;
    it.second.toData(pItem);
  }

  data->clear_itemget();
  for (auto& it : m_mapItemGetCount)
  {
    ItemGetCount* pGet = data->add_itemget();
    if (pGet != nullptr)
    {
      pGet->set_itemid(it.first);
      pGet->set_getcount(it.second);
    }
  }

  data->clear_exchanged_goods();
  for (auto& it : m_setHasExchangedItem)
  {
    data->add_exchanged_goods(it);
  }
  return true;
}

bool ExchangeShop::loadProfessionData(const Cmd::ProfessionData& data)
{
  // remove old branch item
  for (auto m = m_mapExchangeItem.begin(); m != m_mapExchangeItem.end();)
  {
    TPtrExchangeItemConfig pConfig = m->second.getShopItemConfig();
    if (pConfig == nullptr || pConfig->isBranch() == false)
    {
      ++m;
      continue;
    }

    ExchangeShopItem oItem;
    m->second.toData(&oItem);
    XDBG << "[兑换商店-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << oItem.ShortDebugString() << "被移除" << XEND;
    m = m_mapExchangeItem.erase(m);
  }

  for (int i = 0; i < data.exchange_items_size(); ++i)
  {
    const ExchangeShopItem& item = data.exchange_items(i);

    TPtrExchangeItemConfig pConfig = ExchangeShopManager::getMe().getShopItemById(item.id());
    if (pConfig == nullptr || pConfig->isBranch() == true)
    {
      XERR << "[兑换商店-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加载" << item.ShortDebugString() << "被忽略,配置未找到或者属于分支职业" << XEND;
      continue;
    }

    ExchangeShopItemData shopItem;
    if (shopItem.init(item.id()))
    {
      shopItem.fromData(item);
      m_mapExchangeItem[item.id()] = shopItem;
    }
  }

  resetExchangeShopItems();
  return true;
}

bool ExchangeShop::saveProfessionData(Cmd::ProfessionData& data)
{
  TMapExchangeShopItem mapItem;
  collectBranchItem(mapItem);

  data.clear_exchange_items();
  for (auto &m : mapItem)
  {
    TPtrExchangeItemConfig pConfig = ExchangeShopManager::getMe().getShopItemById(m.first);
    if (pConfig == nullptr || pConfig->isBranch() == true)
    {
      XDBG << "[兑换商店-多职业存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存储列表 id :" << m.first << "被忽略,配置未找到或者属于分支职业" << XEND;
      continue;
    }
    ExchangeShopItem* pItem = data.add_exchange_items();
    if (pItem == nullptr)
      continue;
    m.second.toData(pItem);
  }

  return true;
}

void ExchangeShop::timer(QWORD curTime)
{
  if (last_tick_sec == 0 || last_tick_sec > curTime)
  {
    last_tick_sec = curTime;
    return;
  }

  TVecDWORD vecUpdateIDs;
  TVecDWORD vecDelIDs;
  if (m_vecDelIDs.size() > 0)
  {
    for (auto& it : m_vecDelIDs)
    {
      delShopItem(it);
      vecDelIDs.push_back(it);
    }
    m_vecDelIDs.clear();
  }

  for (auto& it : m_mapExchangeItem)
  {
    if (it.second.getLeftTime() > 0)
    {
      it.second.reduceLeftTime(curTime - last_tick_sec);
      if (it.second.getLeftTime() == 0)
      {
        it.second.setShopItemStatus(EEXCHANGESTATUSTYPE_UNLOOK);
        XLOG << "[兑换商店-timer]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << it.first << "到期了" << XEND;
      }
    }
    else if (it.second.getDelayTime() > 0 && it.second.getDelayTime() <= curTime)
    {
      it.second.setDelayTime(0);
      it.second.setShopItemStatus(EEXCHANGESTATUSTYPE_OK);
      vecUpdateIDs.push_back(it.first);
      XLOG << "[兑换商店-timer]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << it.first << "延迟出现时间到" << XEND;
    }
  }
  for (auto it = m_mapExchangeItem.begin(); it != m_mapExchangeItem.end();)
  {
    if (it->second.getShopItemStatus() == EEXCHANGESTATUSTYPE_UNLOOK)
    {
      vecDelIDs.push_back(it->first);
      it = m_mapExchangeItem.erase(it);
      XLOG << "[兑换商店-timer]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << it->first << "消失状态，删除" << XEND;
    }
    else
    {
      it++;
    }
  }

  if (vecUpdateIDs.size() > 0 || vecDelIDs.size() > 0)
  {
    UpdateExchangeShopData update_cmd;
    for (auto& it : vecUpdateIDs)
    {
      auto itShopItem = m_mapExchangeItem.find(it);
      if (itShopItem != m_mapExchangeItem.end())
      {
        itShopItem->second.toData(update_cmd.add_items());
      }
    }
    for (auto& it : vecDelIDs)
    {
      update_cmd.add_del_items(it);
    }
    update_cmd.set_menu_open(checkMenuOpen());
    PROTOBUF(update_cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  last_tick_sec = curTime;
}

void ExchangeShop::sendExchangeShopItems()
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_EXCHANGESHOP) == true)
    return;
  if (isProfessionOpen() == true)
  {
    bool bIsBuy = m_pUser->isBuy();
    UpdateExchangeShopData update_cmd;
    for (auto& it : m_mapExchangeItem)
    {
      TPtrExchangeItemConfig pConfig = it.second.getShopItemConfig();
      if (pConfig == nullptr)
          continue;
      if (bIsBuy == pConfig->isBranch())
        it.second.toData(update_cmd.add_items());
    }
    if (update_cmd.items_size() > 0)
    {
      update_cmd.set_menu_open(checkMenuOpen());
      PROTOBUF(update_cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      XDBG << "[兑换商店-同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步列表数据为" << update_cmd.ShortDebugString() << XEND;
    }
  }

  //上线检查
  onConditionTrigger(EEXCHANGECONDITION_BASE_LEVEL);
  onConditionTrigger(EEXCHANGECONDITION_JOB_LEVEL);
  onConditionTrigger(EEXCHANGECONDITION_MEDAL_COUNT);
  onConditionTrigger(EEXCHANGECONDITION_FOREVER);
}

void ExchangeShop::resetExchangeShopItems()
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_EXCHANGESHOP) == true)
    return;
  ResetExchangeShopDataShopCmd cmd;
  if (isProfessionOpen() == true)
  {
    bool bIsBuy = m_pUser->isBuy();
    for (auto &m : m_mapExchangeItem)
    {
      TPtrExchangeItemConfig pConfig = m.second.getShopItemConfig();
      if (pConfig == nullptr)
          continue;
      if (bIsBuy == pConfig->isBranch())
        m.second.toData(cmd.add_items());
    }
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[兑换商店-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置列表数据为" << cmd.ShortDebugString() << XEND;

  onConditionTrigger(EEXCHANGECONDITION_BASE_LEVEL);
  onConditionTrigger(EEXCHANGECONDITION_JOB_LEVEL);
  onConditionTrigger(EEXCHANGECONDITION_MEDAL_COUNT);
}

void ExchangeShop::reloadExchangeShopConfig()
{
  for (auto& it : m_mapExchangeItem)
  {
    it.second.setShopItemConfig(ExchangeShopManager::getMe().getShopItemById(it.first));
  }

  for (auto it = m_mapExchangeItem.begin(); it != m_mapExchangeItem.end();)
  {
    if (it->second.getShopItemConfig() == nullptr || !it->second.getShopItemConfig()->isOn())
    {
      it = m_mapExchangeItem.erase(it);
    }
    else
    {
      it++;
    }
  }
}

void ExchangeShop::onLogin()
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_EXCHANGESHOP) == true)
    return;
  //金质勋章
  auto it = m_mapItemGetCount.find(ITEM_MEDAL);
  if (it == m_mapItemGetCount.end())
  {
    DWORD itemcount = m_pUser->getPackage().collectItemCount(ITEM_MEDAL);
    TVecItemInfo total_item;
    m_pUser->getAstrolabes().getTotalCost(total_item);
    for (auto& vec_it : total_item)
    {
      if (vec_it.id() == ITEM_MEDAL)
      {
        itemcount += vec_it.count();
      }
    }

    m_mapItemGetCount[ITEM_MEDAL] = itemcount;
  }

  //检查当前的商品配置是不是失效状态
  for (auto it = m_mapExchangeItem.begin(); it != m_mapExchangeItem.end();)
  {
    TPtrExchangeItemConfig pShopItemConfig = it->second.getShopItemConfig();
    if (pShopItemConfig == nullptr || !pShopItemConfig->isOn())
    {
      it = m_mapExchangeItem.erase(it);
    }
    else
    {
      it++;
    }
  }

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr && pFighter->getBranch() != 0)
  {
    Cmd::ProfessionData* pData = m_pUser->m_oProfession.getProfessionData(pFighter->getBranch());
    if (pData != nullptr)
    {
      for (int i = 0; i < pData->exchange_items_size(); i++)
      {
        const ExchangeShopItem& item = pData->exchange_items(i);

        TPtrExchangeItemConfig pConfig = ExchangeShopManager::getMe().getShopItemById(item.id());
        if (pConfig == nullptr || pConfig->isBranch() == false)
        {
          XERR << "[兑换商店-登陆加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "分支加载" << item.ShortDebugString() << "被忽略,配置未找到或者属于主职业" << XEND;
          continue;
        }

        ExchangeShopItemData shopItem;
        if (shopItem.init(item.id()))
        {
          shopItem.fromData(item);
          m_mapExchangeItem[item.id()] = shopItem;
        }
      }
    }
  }
}

void ExchangeShop::onItemAdd(const ItemInfo& rInfo)
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_EXCHANGESHOP) == true)
    return;
  auto it = m_mapItemGetCount.find(rInfo.id());
  if (it != m_mapItemGetCount.end())
  {
    it->second = it->second + rInfo.count();
  }
}

DWORD ExchangeShop::getItemGetCount(DWORD itemid)
{
  auto it = m_mapItemGetCount.find(itemid);
  if (it != m_mapItemGetCount.end())
  {
    return it->second;
  }
  return 0;
}

bool ExchangeShop::checkHasExchangedItem(DWORD dwID)
{
  return m_setHasExchangedItem.count(dwID) > 0;
}

bool ExchangeShop::checkMenuOpen()
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_EXCHANGESHOP) == true)
    return false;
  if (!m_bMenuOpen)
  {
    m_bMenuOpen = true;
    return false;
  }
  return true;
}

bool ExchangeShop::isProfessionOpen()
{
  const SExchangeShopCFG& rCFG = MiscConfig::getMe().getExchangeShopCFG();
  return rCFG.isProOpen(m_pUser->getProfession());
}

void ExchangeShop::exchange(const ExchangeShopItemCmd& cmd)
{
  if (m_pUser == nullptr)
    return;
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_EXCHANGESHOP) == true)
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << cmd.id() << "失败,功能屏蔽中" << XEND;
    return;
  }
  if (isProfessionOpen() == false)
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << cmd.id() << "失败,当前职业" << m_pUser->getProfession() << "未开放追赶功能" << XEND;
    return;
  }
  DWORD dwShopID = cmd.id();
  if (!checkHasItem(dwShopID))
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "失败,该商品未出现" << XEND;
    return;
  }
  ExchangeShopItemData* pShopItem = getShopItem(dwShopID);
  if (pShopItem == nullptr)
    return;

  if (pShopItem->getShopItemConfig() == nullptr)
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "失败,未找到该商品的配置" << XEND;
    return;
  }

  bool bIsBuy = m_pUser->isBuy();
  if (bIsBuy != pShopItem->getShopItemConfig()->isBranch())
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "兑换" << dwShopID << "失败,buy :" << bIsBuy << "branch :" << pShopItem->getShopItemConfig()->isBranch() << "不匹配" << XEND;
    return;
  }

  if (pShopItem->getShopItemStatus() != EEXCHANGESTATUSTYPE_OK)
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "失败,该商品已不能兑换" << (DWORD)pShopItem->getShopItemStatus() << XEND;
    return;
  }

  TVecItemInfo vecChoosedItem;
  for (int i = 0; i < cmd.items_size(); i++)
  {
    ItemInfo item;
    item.set_id(cmd.items(i).id());
    item.set_count(cmd.items(i).num());
    combinItemInfo(vecChoosedItem, item);
  }

  //检查材料数量
  if (pShopItem->getShopItemConfig()->checkItemCount(m_pUser, vecChoosedItem) == false)
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "失败,检查需要的材料数量失败" << XEND;
    return;
  }

  //计算兑换的物品数量
  TVecItemInfo vecExchangedItem;
  pShopItem->getShopItemConfig()->calcExchangedItems(m_pUser, vecChoosedItem, vecExchangedItem);
  if (vecExchangedItem.size() == 0)
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "失败,可兑换数是0" << XEND;
    return;
  }

  //扣除材料
  if (!pShopItem->getShopItemConfig()->reduceItem(m_pUser, vecChoosedItem))
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "失败,扣除材料失败" << XEND;
    return;
  }

  //获得物品
  if (!pShopItem->getShopItemConfig()->addItem(m_pUser, vecExchangedItem))
  {
    XERR << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "失败,添加兑换的物品失败" << XEND;
    return;
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  UpdateExchangeShopData update_cmd;
  ExchangeShopItem* pUpdateShopItem = update_cmd.add_items();
  if (pUpdateShopItem != nullptr)
  {
    pShopItem->toData(pUpdateShopItem);
    update_cmd.set_menu_open(checkMenuOpen());
    PROTOBUF(update_cmd, update_send, update_len);
    m_pUser->sendCmdToMe(update_send, update_len);
  }

  //记录获得的商品ID
  if (pShopItem->getShopItemConfig()->isBranch() == false && (pShopItem->getShopItemStatus() == EEXCHANGESTATUSTYPE_EMPTY || pShopItem->getShopItemStatus() == EEXCHANGESTATUSTYPE_UNLOOK))
  {
    m_setHasExchangedItem.insert(dwShopID);
    onExchangeShopItem();
  }
  
  XLOG << "[兑换商店-兑换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "兑换" << dwShopID << "成功" << XEND;
}

void ExchangeShop::onBaseLevelUp()
{
  onConditionTrigger(EEXCHANGECONDITION_BASE_LEVEL);
}

void ExchangeShop::onJobLevelUp()
{
  onConditionTrigger(EEXCHANGECONDITION_JOB_LEVEL);
}

void ExchangeShop::onExchangeShopItem()
{
  onConditionTrigger(EEXCHANGECONDITION_HAS_EXCHANGED);
}

void ExchangeShop::onAddMedalCount()
{
  onConditionTrigger(EEXCHANGECONDITION_MEDAL_COUNT);
}

void ExchangeShop::onAddBattleTime()
{
  onConditionTrigger(EEXCHANGECONDITION_BATTLE_TIME);
}

void ExchangeShop::onConditionTrigger(EExchangeShopConditionType cond)
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_EXCHANGESHOP) == true)
    return;
  const TSetDWORD* pSetCondItem = ExchangeShopManager::getMe().getConditionItemSet(cond);
  if (pSetCondItem == nullptr)
    return;
  UpdateExchangeShopData update_cmd;
  bool bNeedSend = false;
  for (auto& it : (*pSetCondItem))
  {
    TPtrExchangeItemConfig pShopItemConfig = ExchangeShopManager::getMe().getShopItemById(it);
    if (pShopItemConfig)
    {
      if (checkHasItem(it))
      {
        if (pShopItemConfig->checkEnd(m_pUser))
        {
          addToDelList(it);
          ExchangeShopItemData* pShopItem = getShopItem(it);
          if (pShopItem && pShopItem->getShopItemStatus() == EEXCHANGESTATUSTYPE_OK)
          {
            pShopItem->setShopItemStatus(EEXCHANGESTATUSTYPE_EMPTY);
            pShopItem->toData(update_cmd.add_items());
            bNeedSend = true;
          }
        }
      }
      else
      {
        if(pShopItemConfig->checkStart(m_pUser))
        {
          addShopItem(it);
          ExchangeShopItemData* pShopItem = getShopItem(it);
          if (pShopItem != nullptr)
          {
            pShopItem->toData(update_cmd.add_items());
          }
          bNeedSend = true;
        }
      }
    }
  }

  if (isProfessionOpen() == false)
    bNeedSend = false;

  if (bNeedSend)
  {
    update_cmd.set_menu_open(checkMenuOpen());
    PROTOBUF(update_cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void ExchangeShop::addShopItem(DWORD dwID)
{
  ExchangeShopItemData& shopItem = m_mapExchangeItem[dwID];
  shopItem.init(dwID);
  XLOG << "[兑换商店-添加商品]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << dwID << XEND;
}

void ExchangeShop::delShopItem(DWORD dwID)
{
  auto it = m_mapExchangeItem.find(dwID);
  if (it != m_mapExchangeItem.end())
    m_mapExchangeItem.erase(it);
  XLOG << "[兑换商店-删除商品]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << dwID << XEND;
}

void ExchangeShop::addToDelList(DWORD dwID)
{
  auto it = find(m_vecDelIDs.begin(), m_vecDelIDs.end(), dwID);
  if (it != m_vecDelIDs.end())
  {
    XLOG << "[兑换商店-添加删除商品重复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << dwID << XEND;
    return;
  }
  m_vecDelIDs.push_back(dwID);
  XLOG << "[兑换商店-添加删除商品]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商品" << dwID << XEND;
}

bool ExchangeShop::checkHasItem(DWORD dwID)
{
  auto it = m_mapExchangeItem.find(dwID);
  return it != m_mapExchangeItem.end();
}

ExchangeShopItemData* ExchangeShop::getShopItem(DWORD dwID)
{
  auto it = m_mapExchangeItem.find(dwID);
  if (it != m_mapExchangeItem.end())
  {
    return &(it->second);
  }
  return nullptr;
}

void ExchangeShop::collectBranchItem(TMapExchangeShopItem& mapItem)
{
  mapItem.clear();
  for (auto &m : m_mapExchangeItem)
  {
    TPtrExchangeItemConfig pConfig = m.second.getShopItemConfig();
    if (pConfig == nullptr || pConfig->isBranch() == false)
      continue;
    mapItem[m.first] = m.second;
  }
}

ExchangeShopItemData::ExchangeShopItemData()
{
}

ExchangeShopItemData::~ExchangeShopItemData()
{
}

bool ExchangeShopItemData::init(DWORD dwID)
{
  m_dwId = dwID;

  m_pItemConfig = ExchangeShopManager::getMe().getShopItemById(dwID);
  if (m_pItemConfig == nullptr)
    return false;
  m_dwProgress = 1;
  m_status = EEXCHANGESTATUSTYPE_OK;
  m_dwLeftTime = m_pItemConfig->getLimitTime();
  m_dwDelayTime = 0;
  if (m_pItemConfig->delayStart())
  {
    m_status = EEXCHANGESTATUSTYPE_DELAY;
    const SExchangeShopCFG& shopConfig = MiscConfig::getMe().getExchangeShopCFG();
    m_dwDelayTime = randBetween(shopConfig.dwMinDelayTime, shopConfig.dwMaxDelayTime) + now();
  }
  return true;
}

void ExchangeShopItemData::fromData(const ExchangeShopItem& data)
{
  m_status = data.status();
  m_dwProgress = data.progress();
  m_dwExchangedCount = data.exchange_count();
  m_dwLeftTime = data.left_time();
  m_dwDelayTime = data.delay_time();
}

void ExchangeShopItemData::toData(ExchangeShopItem* data)
{
  if (data == nullptr)
    return;
  data->set_id(m_dwId);
  data->set_status(m_status);
  data->set_progress(m_dwProgress);
  data->set_exchange_count(m_dwExchangedCount);
  data->set_left_time(m_dwLeftTime);
  data->set_delay_time(m_dwDelayTime);
}
