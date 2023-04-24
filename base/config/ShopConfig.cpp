#include "ShopConfig.h"
#include "xLuaTable.h"
#include "ItemConfig.h"
#include "SkillConfig.h"
#include "MiscConfig.h"

// config
ShopConfig::ShopConfig()
{

}

ShopConfig::~ShopConfig()
{

}

bool ShopConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Shop.txt"))
  {
    XERR << "[ShopConfig],加载配置Table_Quest.txt失败" << XEND;
    return false;
  }

  m_mapID2Item.clear();
  m_mapType2Item.clear();
  m_mapSkill2Shop.clear();
  m_mapItemId2Item.clear();
  m_mapPreGoodsID.clear();

  string addDateName, removeDateName;
  switch (BaseConfig::getMe().getInnerBranch())
  {
  case BRANCH_TF:
    addDateName = "TFAddDate";
    removeDateName = "TFRemoveDate";
    break;
  default:
    addDateName = "AddDate";
    removeDateName = "RemoveDate";
    break;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Shop", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    auto o = m_mapID2Item.find(m->first);
    if (o != m_mapID2Item.end())
    {
      XERR << "[ShopConfig] id = " << m->first << " duplicated!" << XEND;
      bCorrect = false;
      continue;
    }

    // check limit type
    DWORD type = m->second.getTableInt("LimitType");
    if (EShopLimitType_IsValid(type) == false)
    {
      XERR << "[ShopConfig] id = " << m->first << " limittype = " << type << " error!" << XEND;
      bCorrect = false;
      continue;
    }
    EShopLimitType eLimitType = static_cast<EShopLimitType>(type);

    // create shopitem
    ShopItem oItem;

    oItem.set_id(m->first);
    oItem.set_lv(m->second.getTableInt("BaseLv"));

    oItem.set_moneycount(m->second.getTableInt("ItemCount"));
    oItem.set_moneycount2(m->second.getTableInt("ItemCount2"));

    oItem.set_starttime(m->second.getTableInt("StartTime"));
    oItem.set_endtime(m->second.getTableInt("EndTime"));

    oItem.set_discount(m->second.getTableInt("Discount"));
    oItem.set_shoptype(m->second.getTableInt("type"));

    oItem.set_moneyid(m->second.getTableInt("ItemID"));
    oItem.set_moneyid2(m->second.getTableInt("ItemID2"));
    oItem.set_limittype(eLimitType);
    oItem.set_maxcount(m->second.getTableInt("LimitNum"));
    oItem.set_shopid(m->second.getTableInt("ShopID"));
    oItem.set_screen(m->second.getTableInt("Screen"));
    oItem.set_menuid(m->second.getTableInt("MenuID"));
    oItem.set_business(m->second.getTableInt("business"));
    oItem.set_shoporder(m->second.getTableInt("ShopOrder"));
    oItem.set_ifmsg(m->second.getTableInt("IfMsg"));
    oItem.set_lockarg(m->second.getTableString("MenuDes"));
    oItem.set_producenum(m->second.getTableInt("ProduceNum"));
    oItem.set_nextgoodsid(m->second.getTableInt("NextGoodsID"));
    oItem.set_itemtype(m->second.getTableInt("ItemType"));

    DWORD source = m->second.getTableInt("Source");
    if (EShopSource_IsValid(source) == false)
    {
      XERR << "[ShopConfig] id = " << m->first << " source = " << source << " error!" << XEND;
      bCorrect = false;
      source = ESHOPSOURCE_USER;
    }
    oItem.set_source(static_cast<EShopSource>(source));

    auto calctime = [](const char *str, DWORD &time) -> bool {
      struct tm tm1;
      if (sscanf(str, "%4d/%2d/%2d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday) != 3)
        return false;
      tm1.tm_hour = 5;
      tm1.tm_min = 0;
      tm1.tm_sec = 0;
      tm1.tm_year -= 1900;
      tm1.tm_mon--;
      tm1.tm_isdst = -1;
      time_t t = mktime(&tm1);
      if (t == -1)
        return false;
      time = t;
      return true;
    };

    // 上架时间
    string sadddate = m->second.getTableString(addDateName);
    DWORD adddate = 0;
    if (!m->second.has(addDateName) || sadddate == "")
      adddate = 0;
    else
      if (calctime(sadddate.c_str(), adddate) == false)
      {
        XERR << "[ShopConfig] id:" << m->first << addDateName << sadddate << "格式错误" << XEND;
        bCorrect = false;
        adddate = DWORD_MAX;
      }
    oItem.set_adddate(adddate);

    // 下架时间
    string sremovedate = m->second.getTableString(removeDateName);
    DWORD removedate = 0;
    if (!m->second.has(removeDateName) || sremovedate == "")
      removedate = DWORD_MAX;
    else
      if (calctime(sremovedate.c_str(), removedate) == false)
      {
        XERR << "[ShopConfig] id:" << m->first << removeDateName << sremovedate << "格式错误" << XEND;
        bCorrect = false;
        removedate = DWORD_MAX;
      }
    oItem.set_removedate(removedate);

    if ((adddate != 0 || removedate !=0) && adddate >= removedate)
    {
      XERR << "[ShopConfig] id:" << m->first << addDateName << sadddate << removeDateName << sremovedate << "配置错误" << XEND;
      bCorrect = false;
    }

    oItem.set_display(m->second.getTableInt("Display"));

    // load items
    xLuaData& item = m->second.getMutableData("items");
    oItem.set_itemid(item.getTableInt("itemID"));
    DWORD skillid = item.getTableInt("SkillID");
    oItem.set_skillid(skillid);
    oItem.set_haircolorid(item.getTableInt("hairColorID"));
    if (item.has("clothColorID") == false && oItem.itemid() == 0 && oItem.skillid() == 0 && oItem.haircolorid() == 0)
    {
      XERR << "[ShopConfig] id = " << m->first << " itemID and skillID and haircolorid and all 0 error!" << XEND;
      bCorrect = false;
      continue;
    }
    if (item.has("clothColorID") == true)
    {
      oItem.set_clothcolorid(item.getTableInt("clothColorID"));
      if (oItem.clothcolorid() == 0)
      {
        oItem.clear_clothcolorids();
        auto clothf = [&](const string& key, xLuaData& data)
        {
          oItem.add_clothcolorids(data.getInt());
        };
        item.getMutableData("clothColorID").foreach(clothf);
      }

      if (oItem.clothcolorid() == 0 && oItem.clothcolorids_size() == 0)
      {
        XERR << "[ShopConfig] id = " << m->first << "没有clothcolorid配置" << XEND;
        bCorrect = false;
        continue;
      }
      if (oItem.clothcolorid() == 0 && (oItem.clothcolorids_size() != 2 || oItem.clothcolorids(0) > oItem.clothcolorids(1)))
      {
        XERR << "[ShopConfig] id = " << m->first << "clothcolorid配置不正确" << XEND;
        bCorrect = false;
        continue;
      }
    }
    if (oItem.skillid() != 0)
      m_setManualSkillIDs.insert(oItem.skillid());
    oItem.set_num(item.getTableInt("num"));

    // load des
    oItem.set_des(m->second.getTableString("des"));
    oItem.set_levdes(m->second.getTableString("LevelDes"));
    oItem.set_precost(m->second.getTableInt("PreCost"));

    // 折扣活动
    if (m->second.has("DiscountActivity") && m->second.getMutableData("DiscountActivity").has("discount"))
    {
      xLuaData& actdata = m->second.getMutableData("DiscountActivity");
      DWORD tfst = 0, tfet = 0, rst, ret;
      if (actdata.has("tf_starttime") && calctime(actdata.getTableString("tf_starttime"), tfst) == false)
      {
        XERR << "[ShopConfig] id = " << m->first << "DiscountActivity开始时间配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      if (actdata.has("tf_endtime") && calctime(actdata.getTableString("tf_endtime"), tfet) == false)
      {
        XERR << "[ShopConfig] id = " << m->first << "DiscountActivity结束时间配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      if ((tfst != 0 || tfet != 0) && tfst >= tfet)
      {
        XERR << "[ShopConfig] id = " << m->first << "DiscountActivity开始/结束时间配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      if (actdata.has("release_starttime") && calctime(actdata.getTableString("release_starttime"), rst) == false)
      {
        XERR << "[ShopConfig] id = " << m->first << "DiscountActivity开始时间配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      if (actdata.has("release_endtime") && calctime(actdata.getTableString("release_endtime"), ret) == false)
      {
        XERR << "[ShopConfig] id = " << m->first << "DiscountActivity结束时间配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      if ((rst != 0 || ret !=0) && rst >= ret)
      {
        XERR << "[ShopConfig] id = " << m->first << "DiscountActivity开始/结束时间配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      m_mapItem2DiscountActivity[m->first].dwTFStartTime = tfst;
      m_mapItem2DiscountActivity[m->first].dwTFEndTime = tfet;
      m_mapItem2DiscountActivity[m->first].dwReleaseStartTime = rst;
      m_mapItem2DiscountActivity[m->first].dwReleaseEndTime = ret;
      m_mapItem2DiscountActivity[m->first].dwDiscount = actdata.getTableInt("discount");
      m_mapItem2DiscountActivity[m->first].dwCount = actdata.getTableInt("count");
      auto questf = [&](const string& k, xLuaData& d)
      {
        m_mapItem2DiscountActivity[m->first].vecQuestID.push_back(d.getInt());
      };
      actdata.getMutableData("questid").foreach(questf);
    }

    // 显示方式(是否要显示已解锁,已存入等)
    xLuaData& preData = m->second.getMutableData("Presentation");
    Presentation* pPre = oItem.mutable_presentation();
    if(preData.has("type"))
      pPre->set_presenttype(EPresentType(preData.getTableInt("type")));
    if(preData.has("msgid"))
      pPre->set_msgid(preData.getTableInt("msgid"));
    if(preData.has("menuID"))
    {
      auto getMenuid = [&](const string& str, xLuaData& data)
      {
        pPre->add_menuid(data.getInt());
      };
      preData.getMutableData("menuID").foreach(getMenuid);
    }

    // insert to list
    m_mapID2Item[m->first] = oItem;
    m_mapType2Item[oItem.shoptype()].push_back(oItem);
    m_setValidType.insert(oItem.shoptype());
    if (oItem.itemid())
    {
      m_mapItemId2Item[oItem.itemid()].push_back(oItem);
    }

    if(skillid != 0)
      m_mapSkill2Shop.insert(std::make_pair(skillid,m->first));

    // item price
    if (oItem.moneyid() == EMONEYTYPE_SILVER)
      m_mapItemPrice[oItem.id()] += oItem.moneycount();
    if (oItem.moneyid2() == EMONEYTYPE_SILVER)
      m_mapItemPrice[oItem.id()] += oItem.moneycount2();
    
    auto limitlvf = [&](xLuaData& data, TSetLevelWeight& w)
    {
      auto f = [&](const string& str, xLuaData& data)
      {
        w.insert(pair<DWORD, DWORD>(atoi(str.c_str()), data.getInt()));
      };
      data.foreach(f);
    };
    if (m->second.has("AccLimitLv"))
      limitlvf(m->second.getMutableData("AccLimitLv"), m_mapAccShopID2Weight[pair<DWORD, DWORD>(oItem.shoptype(), oItem.shopid())][m->first]);
    if (m->second.has("CharLimitLv"))
      limitlvf(m->second.getMutableData("CharLimitLv"), m_mapCharShopID2Weight[pair<DWORD, DWORD>(oItem.shoptype(), oItem.shopid())][m->first]);

    if (oItem.limittype() != ESHOPLIMITTYPE_MIN && oItem.nextgoodsid() > 0)
      m_mapPreGoodsID[oItem.nextgoodsid()] = oItem.id();

  }

  if (bCorrect)
    XLOG << "[ShopConfig] 成功加载Table_Shop.txt" << XEND;
  return bCorrect;
}

bool ShopConfig::checkConfig()
{
  bool bCorrect = true;

  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
  {
    const ShopItem& rItem = m->second;
    if (rItem.itemid() != 0)
    {
      if (ItemConfig::getMe().getItemCFG(rItem.itemid()) == nullptr)
      {
        XERR << "[商店配置-检查] type :" << rItem.shoptype() << " itemid :" << rItem.itemid() << "未在 Table_Item.txt 表中找到" << XEND;
        bCorrect = false;
      }
    }
    if (rItem.skillid() != 0)
    {
      if (SkillConfig::getMe().getSkillCFG(rItem.skillid()) == nullptr)
      {
        XERR << "[商店配置-检查] type :" << rItem.shoptype() << " skillid :" << rItem.skillid() << "未在 Table_Skill.txt 表中找到" << XEND;
        bCorrect = false;
      }
    }
    if (ItemConfig::getMe().getItemCFG(rItem.moneyid()) == nullptr)
    {
      XERR << "[商店配置-检查] type :" << rItem.shoptype() << "moneytype :" << rItem.moneyid() << "未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
    }
    if (rItem.precost() && ItemConfig::getMe().getItemCFG(rItem.precost()) == nullptr)
    {
      XERR << "[商店配置-检查] type :" << rItem.shoptype() << "precost :" << rItem.precost() << "未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
    }
    if (rItem.clothcolorid() != 0 && TableManager::getMe().getClothCFG(rItem.clothcolorid()) == nullptr)
    {
      XERR << "[商店配置-检查] type :" << rItem.shoptype() << "clothcolorID :" << rItem.clothcolorid() << "未在 Table_Couture.txt 表中找到" << XEND;
      bCorrect = false;
    }
    if (rItem.clothcolorids_size() == 2)
    {
      for (DWORD d = rItem.clothcolorids(0); d <= rItem.clothcolorids(1); ++d)
      {
        if (TableManager::getMe().getClothCFG(d) == nullptr)
        {
          XERR << "[商店配置-检查] type :" << rItem.shoptype() << "clothcolorID段中 :" << rItem.clothcolorid() << "未在 Table_Couture.txt 表中找到" << XEND;
          bCorrect = false;
        }
      }
    }
  }

  //检查是否存在环形关系配置,并且确保每一个nextgoods确实存在于商品表里
  std::set<DWORD> setPreGoodsID;
  for(auto it = m_mapPreGoodsID.begin(); it != m_mapPreGoodsID.end(); ++it)
  {
    const ShopItem* pItem = getShopItem(it->first);
    if(pItem == nullptr)
    {
      bCorrect = false;
      XERR << "[商店配置-检查] id :" << it->second << " 的 NextGoodsID: " << it->first << " 未在 Table_Shop.txt 表中找到" << XEND;
    }

    setPreGoodsID.clear();
    setPreGoodsID.insert(it->first);
    auto m = m_mapPreGoodsID.find(it->second);
    while(m != m_mapPreGoodsID.end())
    {
      if(setPreGoodsID.count(m->first) > 0)
      {
        bCorrect = false;
        XERR << "[商店配置-检查] id :" << m->first << " 所在的前后关系链中存在环形关系" << XEND;
        break;
      }

      setPreGoodsID.insert(m->first);
      m = m_mapPreGoodsID.find(m->second);
    }
  }

  return bCorrect;
}

const ShopItem* ShopConfig::getShopItem(DWORD dwID) const
{
  auto m = m_mapID2Item.find(dwID);
  if (m != m_mapID2Item.end())
    return &m->second;

  return nullptr;
}

const ShopItem* ShopConfig::getShopItemBySkill(DWORD dwID) const
{
  auto m = m_mapSkill2Shop.find(dwID);
  if(m != m_mapSkill2Shop.end())
    return getShopItem(m->second);

  return nullptr;
}

DWORD ShopConfig::getItemPrice(DWORD dwItemID) const
{
  auto m = m_mapItemPrice.find(dwItemID);
  if (m != m_mapItemPrice.end())
    return m->second;
  return 0;
}

void ShopConfig::collectShopItem(DWORD dwType, DWORD dwShopID, TVecShopItem& vecItems)
{
  auto m = m_mapType2Item.find(dwType);
  if (m == m_mapType2Item.end())
    return;

  vecItems.clear();
  for (auto v = m->second.begin(); v != m->second.end(); ++v)
  {
    if (v->shopid() == dwShopID)
      vecItems.push_back(*v);
  }
}

bool ShopConfig::canDisplay(DWORD dwID) const
{
  auto m = m_mapID2Item.find(dwID);
  if (m != m_mapID2Item.end())
    return m->second.display() != 0;

  return false;
}

bool ShopConfig::canBuyItemNow(const ShopItem *pItem) const
{
  if (pItem == nullptr)
    return false;
  DWORD cur = now();
  if (cur < pItem->adddate() || cur >= pItem->removedate())
    return false;
  return true;
}

void ShopConfig::getRandomShopItemsByLv(DWORD lv, map<DWORD, DWORD>& items)
{
  for (auto& v : m_mapCharShopID2Weight)
  {
    const SShopInfo* pInfo = MiscConfig::getMe().getShopInfo(v.first.first, v.first.second);
    if (pInfo == nullptr)
      continue;
    randomItems(v.second, lv, pInfo->dwCount, pInfo->bDuplicate, items);
  }
}

void ShopConfig::getRandomShopItemsByAccLv(DWORD acclv, map<DWORD, DWORD>& items)
{
  for (auto& v : m_mapAccShopID2Weight)
  {
    const SShopInfo* pInfo = MiscConfig::getMe().getShopInfo(v.first.first, v.first.second);
    if (pInfo == nullptr)
      continue;
    randomItems(v.second, acclv, pInfo->dwCount, pInfo->bDuplicate, items);
  }
}

// 按权重随机商品
void ShopConfig::randomItems(const TMapItemWeight& items, DWORD lv, DWORD total_count, bool duplicate, map<DWORD, DWORD>& result)
{
  DWORD total_weight = 0;
  vector<pair<DWORD, DWORD>> all;
  for (auto& v : items)
  {
    for (auto& m : v.second)
    {
      if (lv <= m.first)
      {
        if (m.second > 0)
        {
          total_weight += m.second;
          all.push_back(pair<DWORD, DWORD>(v.first, m.second));
        }
        break;
      }
    }
  }
  if (all.empty())
    return;

  while (total_count--)
  {
    DWORD w = randBetween(1, total_weight), t = 0;
    auto it = all.begin();
    while (it != all.end())
    {
      t += it->second;
      if (w <= t)
        break;
      ++it;
    }
    if (it != all.end())
    {
      auto item = result.find(it->first);
      if (item == result.end())
        result[it->first] = 1;
      else if (duplicate)
        result[it->first] += 1;
      if (!duplicate)
      {
        if (total_weight >= it->second)
          total_weight -= it->second;
        else
        {
          total_weight = 0;
          break;
        }
        all.erase(it);
      }
    }
  }

  for (auto& v : result)
  {
    const ShopItem* pItem = getShopItem(v.first);
    if (pItem == nullptr)
      v.second = 0;
    else
      v.second *= pItem->maxcount();
  }
}

const SShopDiscountActivity* ShopConfig::getDiscountActivity(DWORD id) const
{
  auto it = m_mapItem2DiscountActivity.find(id);
  if (it == m_mapItem2DiscountActivity.end())
    return nullptr;
  return &it->second;
}

const ShopItem* ShopConfig::getShopItemByItemId(DWORD itemId) const
{
  auto it = m_mapItemId2Item.find(itemId);
  if (it == m_mapItemId2Item.end())
    return nullptr;
  if (it->second.empty())
    return nullptr;  
  
  const ShopItem* pItem = nullptr;
  pItem = getItem(it->second, 100);
  if (pItem)
    return pItem;

  pItem = getItem(it->second, 110);
  if (pItem)
    return pItem;

  pItem = getItem(it->second, 151);
  if (pItem)
    return pItem;

  // pItem = getItem(it->second, EMONEYTYPE_MIN);
  return pItem;
}

const ShopItem* ShopConfig::getItem(const TVecShopItem&vec, DWORD itemId) const
{
  for (auto&v : vec)
  {
    //单货币且没有限购

    if (v.moneyid() != itemId)
      continue;
    
    if (v.moneyid2() == 0 && !buyLimit(v.limittype()))
      return &v;
  }
  return nullptr;
};

bool ShopConfig::buyLimit(EShopLimitType limitType)
{
  if (limitType > ESHOPLIMITTYPE_MIN && limitType < ESHOPLIMITTYPE_MAX)
  {
    if ((limitType & ESHOPLIMITTYPE_USER) != 0 ||
      (limitType & ESHOPLIMITTYPE_ACC_USER) != 0 ||
      (limitType & ESHOPLIMITTYPE_ACC_USER_ALWAYS) != 0 ||
      (limitType & ESHOPLIMITTYPE_USER_WEEK) != 0 ||
      (limitType & ESHOPLIMITTYPE_USER_MONTH) != 0 ||
      (limitType & ESHOPLIMITTYPE_ACC_WEEK) != 0 ||
      (limitType & ESHOPLIMITTYPE_ACC_MONTH) != 0 
      )
      return true;
  }
  return false;
}

void ShopConfig::setWeekRand(DWORD dwGroupId)
{
  m_dwWeekRandGroupId = dwGroupId;
  const SShopCFG& rCfg = MiscConfig::getMe().getShopCFG();
  auto it = rCfg.mapWeekShopRand.find(m_dwWeekRandGroupId);
  if (it == rCfg.mapWeekShopRand.end())
  {
    XERR << "[SHOP] 刷新,找不到groupid" <<m_dwWeekRandGroupId << XEND;
    return;
  }

  for (auto& v : it->second.vecShopId)
  {
    m_setWeekRandShop.insert(v);
  }
  XLOG << "[SHOP] 刷新成功，groupid" << m_dwWeekRandGroupId << XEND;
}

bool ShopConfig::isInWeekRand(DWORD dwShopId)
{
  auto it = MiscConfig::getMe().getShopCFG().setWeekShopRand.find(dwShopId);
  if (it != MiscConfig::getMe().getShopCFG().setWeekShopRand.end())
  {
    return m_setWeekRandShop.find(dwShopId) != m_setWeekRandShop.end();
  }
  //不是随机
  return true;
}

DWORD ShopConfig::getPreGoodsID(DWORD dwGoodsId)
{
  auto it = m_mapPreGoodsID.find(dwGoodsId);
  if (it == m_mapPreGoodsID.end())
  {
    return 0;
  }
  return it->second;
}

