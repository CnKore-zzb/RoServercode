#include "ExchangeShopManager.h"
#include "SceneUser.h"
#include "ExchangeShop.h"
#include "GuidManager.h"

bool ExchangeShopManager::loadConfig()
{
  bool bCorrect = true;
  m_mapID2ExchangeItem.clear();
  m_mapConditionItemSet.clear();
  auto func = [&](const xEntry* pEntry)
  {
    const SExchangeShop* pShopCFG = static_cast<const SExchangeShop *>(pEntry);
    if (pShopCFG == nullptr)
    {
      bCorrect = false;
      return;
    }
    DWORD id = pShopCFG->id;
    if (m_mapID2ExchangeItem.find(id) != m_mapID2ExchangeItem.end())
    {
      XERR << "[ExchangeShop], id =" << id << "id重复" << XEND;
      bCorrect = false;
      return;
    }
    TPtrExchangeItemConfig pShopItem = createShopItem(id, pShopCFG->getData());
    if (pShopItem == nullptr)
    {
      XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败" << XEND;
      bCorrect = false;
      return;
    }
    m_mapID2ExchangeItem[id] = pShopItem;
    pShopItem->registConditionItemSet(m_mapConditionItemSet);
  };

  Table<SExchangeShop>* pShopTable = TableManager::getMe().getExchangeShopCFGListNoConst();
  if (pShopTable == nullptr)
    return false;
  pShopTable->foreachNoConst(func);

  return bCorrect;
}

bool ExchangeShopManager::loadWorthConfig()
{
  bool bCorrect = true;
  m_mapExchangeWorth.clear();
  
  auto worthfunc = [&](const xEntry* pEntry)
  {
    const SExchangeWorth* pWorthCFG = static_cast<const SExchangeWorth*>(pEntry);
    if (pWorthCFG == nullptr)
    {
      bCorrect = false;
      return;
    }
    xLuaData data = pWorthCFG->getData();
    DWORD itemID = data.getTableInt("ItemID");
    xLuaData& goodsid= data.getMutableData("GoodsID");
    xLuaData& worth = data.getMutableData("Worth");

    TVecDWORD vecDWORD;
    auto getvec = [&](const string& key, xLuaData& d)
    {
      vecDWORD.push_back(d.getInt());
    };

    worth.foreach(getvec);
    if (vecDWORD.size() != 2)
    {
      bCorrect = false;
      XERR << "[ExchangeWorth],id =" << pWorthCFG->id << "加载Table_ExchangeWorth.txt失败" << XEND;
      return;
    }
    DWORD exchangeid = vecDWORD[0];
    DWORD exchangenum = vecDWORD[1];

    vecDWORD.clear();
    goodsid.foreach(getvec);

    for (auto& it : vecDWORD)
    {
      std::map<DWORD, std::pair<DWORD, DWORD>>& worthMap = m_mapExchangeWorth[it];
      std::pair<DWORD, DWORD>& pairWorth = worthMap[itemID];
      pairWorth.first = exchangeid;
      pairWorth.second = exchangenum;
    }
  };

  Table<SExchangeWorth>* pWorthTable = TableManager::getMe().getExchangeWorthCFGListNoConst();
  if (pWorthTable== nullptr)
    return false;
  pWorthTable->foreachNoConst(worthfunc);
  return bCorrect;
}

TPtrExchangeItemConfig ExchangeShopManager::getShopItemById(DWORD id)
{
  auto m = m_mapID2ExchangeItem.find(id);
  if (m != m_mapID2ExchangeItem.end())
    return (m->second);
  return nullptr;
}

TPtrExchangeItemConfig ExchangeShopManager::createShopItem(DWORD id, xLuaData data)
{
  DWORD dwExchangeType = data.getTableInt("ExchangeType");
  if (dwExchangeType == EEXCHANGETYPE_COINS)
  {
    TPtrExchangeItemConfig pShop(new CoinsExchangeItemConfig());
    if (pShop == nullptr || !pShop->init(id, data))
      return nullptr;
    return pShop;
  }
  else if (dwExchangeType == EEXCHANGETYPE_FREE)
  {
    TPtrExchangeItemConfig pShop(new FreeExchangeItemConfig());
    if (pShop == nullptr || !pShop->init(id, data))
      return nullptr;
    return pShop;
  }
  else if (dwExchangeType == EEXCHANGETYPE_PROGRESS)
  {
    TPtrExchangeItemConfig pShop(new ProgressExchangeItemConfig());
    if (pShop == nullptr || !pShop->init(id, data))
      return nullptr;
    return pShop;
  }
  else if (dwExchangeType == EEXCHANGETYPE_NO_PROGRESS)
  {
    TPtrExchangeItemConfig pShop(new ItemExchangeItemConfig());
    if (pShop == nullptr || !pShop->init(id, data))
      return nullptr;
    return pShop;
  }
  else if (dwExchangeType == EEXCHANGETYPE_WORTH_PROGRESS)
  {
    TPtrExchangeItemConfig pShop(new WorthExchangeItemConfig());
    if (pShop == nullptr || !pShop->init(id, data))
      return nullptr;
    return pShop;
  }
  return nullptr;
}

const TSetDWORD* ExchangeShopManager::getConditionItemSet(EExchangeShopConditionType cond)
{
  auto it = m_mapConditionItemSet.find(cond);
  if(it != m_mapConditionItemSet.end())
    return &(it->second);
  return nullptr;
}

bool ExchangeShopManager::getExchangeWorthByItemID(DWORD dwGoodsID, DWORD dwItemID, Cmd::ItemInfo& rItem)
{
  auto it = m_mapExchangeWorth.find(dwGoodsID);
  if (it == m_mapExchangeWorth.end())
    return false;
  auto itWorth = it->second.find(dwItemID);
  if (itWorth == it->second.end())
    return false;
  std::pair<DWORD, DWORD>& rWorth = itWorth->second;
  rItem.set_id(rWorth.first);
  rItem.set_count(rWorth.second);
  return true;
}

ExchangeShopItemConfig::ExchangeShopItemConfig()
{
}

ExchangeShopItemConfig::~ExchangeShopItemConfig()
{
}

bool ExchangeShopItemConfig::init(DWORD id, xLuaData& data)
{
  m_dwID = id;

  m_strName = data.getTableString("Name");
  m_bOnOff = data.getTableInt("OnOff") == 1;
  m_dwShopType = data.getTableInt("Type");
  m_bBranch = data.getTableInt("Branch");
  m_bDelayStart = data.getTableInt("DelayStart") == 1;
  m_dwLimitTime = data.getTableInt("LimitTime") * 3600;

  xLuaData& dataStartCond = data.getMutableData("StartCondition");
  if (!createCondition(dataStartCond, m_vecStartCond))
    return false;
  xLuaData& dataEndCond = data.getMutableData("EndCondition");
  if (!createCondition(dataEndCond, m_vecEndCond))
    return false;
  return true;
}

bool ExchangeShopItemConfig::createCondition(xLuaData& condData, TVecShopCond& vecCond)
{
  auto index = [this, &vecCond](std::string key, xLuaData &data)
  {
    if (key == "baselevel")
    {
      TPtrShopCond pShopCond(new BaseLevelExchangeCondition());
      if (pShopCond == nullptr || !pShopCond->init(data))
        return;
      vecCond.push_back(pShopCond);
    }
    else if (key == "joblevel")
    {
      TPtrShopCond pShopCond(new JobLevelExchangeCondition());
      if (pShopCond == nullptr || !pShopCond->init(data))
        return;
      vecCond.push_back(pShopCond);
    }
    else if (key == "hasexchanged")
    {
      TPtrShopCond pShopCond(new HasExchangedExchangeCondition());
      if (pShopCond == nullptr || !pShopCond->init(data))
        return;
      vecCond.push_back(pShopCond);
    }
    else if (key == "higheritem")
    {
      TPtrShopCond pShopCond(new HigherShopItemExchangeCondition());
      if (pShopCond == nullptr || !pShopCond->init(data))
        return;
      vecCond.push_back(pShopCond);
    }
    else if (key == "medalcount")
    {
      TPtrShopCond pShopCond(new MedalCountExchangeCondition());
      if (pShopCond == nullptr || !pShopCond->init(data))
        return;
      vecCond.push_back(pShopCond);
    }
    else if (key == "battletime")
    {
      TPtrShopCond pShopCond(new BattleTimeExchangeCondition());
      if (pShopCond == nullptr || !pShopCond->init(data))
        return;
      vecCond.push_back(pShopCond);
    }
    else if (key == "forever")
    {
      TPtrShopCond pShopCond(new ForeverExchangeCondition());
      if (pShopCond == nullptr || !pShopCond->init(data))
        return;
      vecCond.push_back(pShopCond);
    }
  };
  condData.foreach(index);
  return true;
}

void ExchangeShopItemConfig::registConditionItemSet(std::map<DWORD, TSetDWORD>& mapCondItemSet)
{
  for (auto& start_it : m_vecStartCond)
  {
    TSetDWORD& setItem = mapCondItemSet[start_it->getCondType()];
    setItem.insert(getID());
  }

  for (auto& end_it : m_vecEndCond)
  {
    TSetDWORD& setItem = mapCondItemSet[end_it->getCondType()];
    setItem.insert(getID());
  }
}

bool ExchangeShopItemConfig::checkStart(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;
  if (!m_bOnOff)
    return false;
  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter == nullptr)
    return false;
  bool isBuy = pUser->isBuy();
  if (m_bBranch && !isBuy)
    return false;
  bool ret = true;
  for (auto& it : m_vecStartCond)
  {
    ret = ret && it->checkOk(pUser);
    if (!ret)
      break;
  }
  return ret;
}

bool ExchangeShopItemConfig::checkEnd(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;
  bool ret = false;
  for (auto& it : m_vecEndCond)
  {
    ret = ret || it->checkOk(pUser);
  }   
  return ret;
}

bool ExchangeShopItemConfig::checkItemCount(SceneUser* pUser, const TVecItemInfo& vecItems)
{
  if (pUser == nullptr)
    return false;
  return pUser->getPackage().checkItemCount(vecItems, ECHECKMETHOD_NORMAL, EPACKFUNC_EXCHANGESHOP);
}

bool ExchangeShopItemConfig::reduceItem(SceneUser* pUser, const TVecItemInfo& vecChoosedItems)
{
  if (pUser == nullptr)
    return false;
  return pUser->getPackage().reduceItem(vecChoosedItems, ESOURCE_EXCHANGE_SHOP, ECHECKMETHOD_NORMAL, EPACKFUNC_EXCHANGESHOP);
}

bool ExchangeShopItemConfig::addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems)
{
  if (pUser == nullptr)
    return false;
  return pUser->getPackage().addItem(vecExchangedItems, EPACKMETHOD_AVAILABLE);
}

DWORD ExchangeShopItemConfig::getCanExchangeCount(DWORD dwProgress, DWORD dwExchangedCount)
{
  if (m_dwExchangeType != EEXCHANGETYPE_PROGRESS && m_dwExchangeType != EEXCHANGETYPE_WORTH_PROGRESS)
    return 0;
  if (dwProgress > m_vecProgressLimit.size())
    return 0;
  if (dwProgress == 0)
    dwProgress = 1;
  DWORD dwTotalLimit = 0;
  for (DWORD i = dwProgress - 1; i < m_vecProgressLimit.size(); i++)
  {
    dwTotalLimit += m_vecProgressLimit[i];
  }

  if (dwTotalLimit <= dwExchangedCount)
    return 0;
  return dwTotalLimit - dwExchangedCount;
}

CoinsExchangeItemConfig::CoinsExchangeItemConfig()
{
  m_dwExchangeType = EEXCHANGETYPE_COINS;
}

CoinsExchangeItemConfig::~CoinsExchangeItemConfig()
{
}

bool CoinsExchangeItemConfig::init(DWORD id, xLuaData& data)
{
  m_dwID = id;

  TVecDWORD vecDWORD;
  auto getvec = [&](const string& key, xLuaData& d)
  {
    vecDWORD.push_back(d.getInt());
  };
  xLuaData& dataCost = data.getMutableData("Cost");
  dataCost.foreach(getvec);
  if (vecDWORD.size() != 2)
  {
    XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败" << XEND;
    return false;
  }
  m_dwCostID = vecDWORD[0];
  m_dwCostNum = vecDWORD[1];

  if (m_dwCostID == 0 || m_dwCostNum == 0)
  {
    XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败,消耗为空" << XEND;
    return false;
  }

  xLuaData& dataReward = data.getMutableData("Item");
  vecDWORD.clear();
  dataReward.foreach(getvec);
  if (vecDWORD.size() != 2)
  {
    XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败" << XEND;
    return false;
  }
  m_dwGetID = vecDWORD[0];
  m_dwGetNum = vecDWORD[1];

  m_bAutoUse = data.getTableInt("AutoUse") == 1;
  if (m_dwGetID == 0 || m_dwGetNum == 0)
  {
    XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败,奖励为空" << XEND;
    return false;
  }

  return ExchangeShopItemConfig::init(id, data);
}

bool CoinsExchangeItemConfig::checkItemCount(SceneUser* pUser, const TVecItemInfo& vecItems)
{
  //只检查costid costnum
  if (pUser == nullptr)
    return false;
  if (vecItems.size() > 0)
    return false;
  if (m_dwCostID == 0)
    return false;
  BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;
  return pMainPack->checkItemCount(m_dwCostID, m_dwCostNum);
}

void CoinsExchangeItemConfig::calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems)
{
  ItemInfo item;
  item.set_id(m_dwGetID);
  item.set_count(m_dwGetNum);
  combinItemInfo(vecExchangedItems, item);
}

bool CoinsExchangeItemConfig::reduceItem(SceneUser* pUser, const TVecItemInfo& vecChoosedItems)
{
  if (pUser == nullptr)
    return false;
  BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;
  pMainPack->reduceItem(m_dwCostID, ESOURCE_EXCHANGE_SHOP, m_dwCostNum);
  return true;
}

bool CoinsExchangeItemConfig::addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems)
{
  if (pUser == nullptr)
    return false;

  ExchangeShopItemData* pShopItem = pUser->getExchangeShop().getShopItem(m_dwID);
  if (pShopItem == nullptr)
    return false;
  pShopItem->setShopItemStatus(EEXCHANGESTATUSTYPE_EMPTY);

  if (m_bAutoUse)
  {   
    BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr)
      return false;
    for (auto& it : vecExchangedItems)
    {   
      string guid = GuidManager::getMe().newGuidStr(pUser->getZoneID(), pUser->getUserSceneData().getOnlineMapID());
      if (ItemManager::getMe().isGUIDValid(guid) == false)
        return false;
      ItemData oData;
      oData.mutable_base()->set_id(it.id());
      oData.mutable_base()->set_count(it.count());
      oData.mutable_base()->set_guid(guid);
      oData.mutable_base()->set_source(ESOURCE_EXCHANGE_SHOP);

      if (!pMainPack->addItem(oData, EPACKMETHOD_NOCHECK))
        return false;
      Cmd::ItemUse usecmd;
      usecmd.set_itemguid(guid);
      usecmd.set_count(it.count());
      pUser->useItem(usecmd);
    }
    return true;
  } 
  return ExchangeShopItemConfig::addItem(pUser, vecExchangedItems);
}

FreeExchangeItemConfig::FreeExchangeItemConfig()
{
  m_dwExchangeType = EEXCHANGETYPE_FREE;
}

FreeExchangeItemConfig::~FreeExchangeItemConfig()
{
}

bool FreeExchangeItemConfig::init(DWORD id, xLuaData& data)
{
  m_dwID = id;
  TVecDWORD vecDWORD;
  auto getvec = [&](const string& key, xLuaData& d)
  {
    vecDWORD.push_back(d.getInt());
  };

  xLuaData& dataReward = data.getMutableData("Item");
  vecDWORD.clear();
  dataReward.foreach(getvec);
  if (vecDWORD.size() != 2)
  {
    XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败" << XEND;
    return false;
  }     
  m_dwGetID = vecDWORD[0];
  m_dwGetNum = vecDWORD[1];

  m_bAutoUse = data.getTableInt("AutoUse") == 1;
  
  return ExchangeShopItemConfig::init(id, data);
}

bool FreeExchangeItemConfig::checkItemCount(SceneUser* pUser, const TVecItemInfo& vecItems)
{
  if (pUser == nullptr)
    return false;
  return true;
}

void FreeExchangeItemConfig::calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems)
{
  vecChoosedItems.clear();
  ItemInfo item;
  item.set_id(m_dwGetID);
  item.set_count(m_dwGetNum);
  combinItemInfo(vecExchangedItems, item);
}

bool FreeExchangeItemConfig::reduceItem(SceneUser* pUser, const TVecItemInfo& vecChoosedItems)
{
  if (pUser == nullptr)
    return false;
  return true;
}

bool FreeExchangeItemConfig::addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems)
{
  if (pUser == nullptr)
    return false;

  ExchangeShopItemData* pShopItem = pUser->getExchangeShop().getShopItem(m_dwID);
  if (pShopItem == nullptr)
    return false;
  pShopItem->setShopItemStatus(EEXCHANGESTATUSTYPE_EMPTY);

  if (m_bAutoUse)
  {
    BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr)
      return false;
    for (auto& it : vecExchangedItems)
    {
      string guid = GuidManager::getMe().newGuidStr(pUser->getZoneID(), pUser->getUserSceneData().getOnlineMapID());
      if (ItemManager::getMe().isGUIDValid(guid) == false)
        return false;
      ItemData oData;
      oData.mutable_base()->set_id(it.id());
      oData.mutable_base()->set_count(it.count());
      oData.mutable_base()->set_guid(guid);
      oData.mutable_base()->set_source(ESOURCE_EXCHANGE_SHOP);

      if (!pMainPack->addItem(oData, EPACKMETHOD_NOCHECK))
        return false;
      Cmd::ItemUse usecmd;
      usecmd.set_itemguid(guid);
      usecmd.set_count(it.count());
      pUser->useItem(usecmd);
    }
    return true;
  }
  return ExchangeShopItemConfig::addItem(pUser, vecExchangedItems);
}

ProgressExchangeItemConfig::ProgressExchangeItemConfig()
{
  m_dwExchangeType = EEXCHANGETYPE_PROGRESS;
}

ProgressExchangeItemConfig::~ProgressExchangeItemConfig()
{
}

bool ProgressExchangeItemConfig::init(DWORD id, xLuaData& data)
{
  m_dwID = id;

  m_vecProgressLimit.clear();
  auto getvec = [&](const string& key, xLuaData& d)
  {
    m_vecProgressLimit.push_back(d.getInt());
  };

  xLuaData& exchangeLimit = data.getMutableData("ExchangeLimit");
  exchangeLimit.foreach(getvec);

  if (m_vecProgressLimit.size() == 0)
  {
    XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败" << XEND;
    return false;
  }

  return ExchangeShopItemConfig::init(id, data);
}

void ProgressExchangeItemConfig::calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems)
{
  if (pUser == nullptr)
    return;
  ExchangeShopItemData* pShopItem = pUser->getExchangeShop().getShopItem(m_dwID);
  if (pShopItem == nullptr)
    return;
  DWORD dwProgress = pShopItem->getProgress();
  DWORD dwExchangedCount = pShopItem->getExchangedCount();
  for (auto& it : vecChoosedItems)
  {
    DWORD dwCanExchangeCount = getCanExchangeCount(dwProgress, dwExchangedCount);
    if (dwCanExchangeCount == 0)
    {
      it.set_count(0);
      continue;
    }
    ItemInfo item;
    if (!ExchangeShopManager::getMe().getExchangeWorthByItemID(m_dwID, it.id(), item))
    {
      it.set_count(0);
      continue;
    }
    DWORD dwExchangeCount = it.count() * item.count();
    if (dwExchangeCount > dwCanExchangeCount)
    {
      it.set_count(dwExchangeCount/item.count());
      dwExchangeCount = it.count() * item.count();
    }
    item.set_count(dwExchangeCount);
    combinItemInfo(vecExchangedItems, item);
    dwExchangedCount += dwExchangeCount;
  }

  for (auto it = vecChoosedItems.begin(); it != vecChoosedItems.end();)
  {
    if ((*it).count() == 0)
      it = vecChoosedItems.erase(it);
    else
      it++;
  }

  for (auto it = vecExchangedItems.begin(); it != vecExchangedItems.end();)
  {
    if ((*it).count() == 0)
      it = vecExchangedItems.erase(it);
    else
      it++;
  }
}

bool ProgressExchangeItemConfig::addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems)
{
  if (pUser == nullptr)
    return false;
  ExchangeShopItemData* pShopItem = pUser->getExchangeShop().getShopItem(m_dwID);
  if (pShopItem == nullptr)
    return false;
  DWORD dwProgress = pShopItem->getProgress();
  DWORD dwExchangedCount = pShopItem->getExchangedCount();
  DWORD dwGetCount = 0;
  for (auto& it : vecExchangedItems)
  {
    dwGetCount += it.count();
  }
  
  if (dwProgress == 0)
    dwProgress = 1;
  DWORD dwTotalLimit = dwGetCount + dwExchangedCount;
  DWORD i = 0;
  for (i = dwProgress - 1; i < m_vecProgressLimit.size(); i++)
  {
    if (dwTotalLimit >= m_vecProgressLimit[i])
    {
      dwTotalLimit -= m_vecProgressLimit[i];
    }
    else
    {
      dwProgress = i + 1;
      dwExchangedCount = dwTotalLimit;
      break;
    }
  }

  if (i == m_vecProgressLimit.size())
  {
    dwProgress = i;
    dwExchangedCount = 0;
    pShopItem->setShopItemStatus(EEXCHANGESTATUSTYPE_EMPTY);
  }
  pShopItem->setProgress(dwProgress);
  pShopItem->setExchangedCount(dwExchangedCount);

  return ExchangeShopItemConfig::addItem(pUser, vecExchangedItems);
}

ItemExchangeItemConfig::ItemExchangeItemConfig()
{
  m_dwExchangeType = EEXCHANGETYPE_NO_PROGRESS;
}

ItemExchangeItemConfig::~ItemExchangeItemConfig()
{
}

bool ItemExchangeItemConfig::init(DWORD id, xLuaData& data)
{
  m_dwID = id;

  return ExchangeShopItemConfig::init(id, data);
}

void ItemExchangeItemConfig::calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems)
{
  if (pUser == nullptr)
    return;
  ExchangeShopItemData* pShopItem = pUser->getExchangeShop().getShopItem(m_dwID);
  if (pShopItem == nullptr)
    return;
  for (auto& it : vecChoosedItems)
  {
    ItemInfo item;
    if (!ExchangeShopManager::getMe().getExchangeWorthByItemID(m_dwID, it.id(), item))
    {
      it.set_count(0);
      continue;
    }
    DWORD dwExchangeCount = it.count() * item.count();
    item.set_count(dwExchangeCount);
    combinItemInfo(vecExchangedItems, item);
  }

  for (auto it = vecChoosedItems.begin(); it != vecChoosedItems.end();)
  {
    if ((*it).count() == 0)
      it = vecChoosedItems.erase(it);
    else 
      it++;
  }

  for (auto it = vecExchangedItems.begin(); it != vecExchangedItems.end();)
  {
    if ((*it).count() == 0)
      it = vecExchangedItems.erase(it);
    else
      it++;
  }
}

WorthExchangeItemConfig::WorthExchangeItemConfig()
{
  m_dwExchangeType = EEXCHANGETYPE_WORTH_PROGRESS;
}

WorthExchangeItemConfig::~WorthExchangeItemConfig()
{
}

bool WorthExchangeItemConfig::init(DWORD id, xLuaData& data)
{
  m_dwID = id;

  m_vecProgressLimit.clear();
  auto getvec = [&](const string& key, xLuaData& d)
  {
    m_vecProgressLimit.push_back(d.getInt());
  };

  xLuaData& exchangeLimit = data.getMutableData("ExchangeLimit");
  exchangeLimit.foreach(getvec);

  if (m_vecProgressLimit.size() == 0)
  {
    XERR << "[ExchangeShop],id =" << id << "加载Table_ExchangeShop.txt失败" << XEND;
    return false;
  }

  return ExchangeShopItemConfig::init(id, data);
}

void WorthExchangeItemConfig::calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems)
{
  vecExchangedItems.clear();

  if (pUser == nullptr)
    return;
  ExchangeShopItemData* pShopItem = pUser->getExchangeShop().getShopItem(m_dwID);
  if (pShopItem == nullptr)
    return;
  DWORD dwCanExchange = getCanExchangeCount(pShopItem->getProgress(), pShopItem->getExchangedCount());
  if (dwCanExchange == 0)
    return;

  XDBG << "[ExchangeShopManager-Worth] id :" << m_dwID << "可兑换次数" << dwCanExchange << XEND;
  for (auto& it : vecChoosedItems)
  {
    ItemInfo item;
    if (!ExchangeShopManager::getMe().getExchangeWorthByItemID(m_dwID, it.id(), item))
    {
      it.set_count(0);
      continue;
    }
    DWORD dwExchangeCount = it.count() * item.count();
    if (dwExchangeCount > dwCanExchange)
    {
      XERR << "[ExchangeShopManager-Worth] id :" << m_dwID << "物品" << it.ShortDebugString() << "可兑换 itemid :" << item.id() << "count :" << dwExchangeCount << "失败,超过可兑换次数" << dwCanExchange << XEND;
      it.set_count(0);
      break;
    }
    dwCanExchange -= dwExchangeCount;
    item.set_count(dwExchangeCount);
    combinItemInfo(vecExchangedItems, item);
    XDBG << "[ExchangeShopManager-Worth] id :" << m_dwID << "可兑换 itemid :" << item.id() << "count :" << dwExchangeCount << "可兑换次数" << dwCanExchange << XEND;
  }

  for (auto it = vecChoosedItems.begin(); it != vecChoosedItems.end();)
  {
    if ((*it).count() == 0)
      it = vecChoosedItems.erase(it);
    else 
      it++;
  }

  for (auto it = vecExchangedItems.begin(); it != vecExchangedItems.end();)
  {
    if ((*it).count() == 0)
      it = vecExchangedItems.erase(it);
    else
      it++;
  }
}

bool WorthExchangeItemConfig::addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems)
{
  if (pUser == nullptr)
    return false;
  ExchangeShopItemData* pShopItem = pUser->getExchangeShop().getShopItem(m_dwID);
  if (pShopItem == nullptr)
    return false;
  DWORD dwExchangedCount = pShopItem->getExchangedCount();
  DWORD dwGetCount = 0;
  for (auto& it : vecExchangedItems)
    dwGetCount += it.count();
  pShopItem->setExchangedCount(dwExchangedCount + dwGetCount);
  if (getCanExchangeCount(pShopItem->getProgress(), pShopItem->getExchangedCount()) <= 0)
    pShopItem->setShopItemStatus(EEXCHANGESTATUSTYPE_EMPTY);
  return ExchangeShopItemConfig::addItem(pUser, vecExchangedItems);
}

