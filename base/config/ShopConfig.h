/**
 * @file ShopConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-12-16
 */

#pragma once

#include "xSingleton.h"
#include "SessionShop.pb.h"
#include "xTime.h"
#include "BaseConfig.h"

using namespace Cmd;
using std::map;
using std::vector;
using std::string;
using std::set;
using std::pair;

typedef map<DWORD, ShopItem> TMapShopItem;
typedef vector<ShopItem> TVecShopItem;
typedef map<DWORD, TVecShopItem> TMapTypeShopItem;
typedef map<DWORD,DWORD> TMapSkillShop;
typedef map<DWORD, DWORD> TMapItemPrice;
typedef set<pair<DWORD, DWORD>> TSetLevelWeight;
typedef map<DWORD, TSetLevelWeight> TMapItemWeight;
typedef map<pair<DWORD, DWORD>, TMapItemWeight> TMapShop2ItemWeight;
typedef map<DWORD/*itemid*/, TVecShopItem> TMapItemId2ShopItem;
typedef map<DWORD/*goodid*/, DWORD/*pre goodsid*/> TMapPreGoodsID;

const DWORD MAXBASELV_RESETTIME_OFFSET = 5 * 3600;
const DWORD MAXBASELV_RESETTIME = 86400;

struct SShopDiscountActivity
{
  DWORD dwTFStartTime = 0;
  DWORD dwTFEndTime = 0;
  DWORD dwReleaseStartTime = 0;
  DWORD dwReleaseEndTime = 0;
  DWORD dwDiscount = 0;
  DWORD dwCount = 0; // 可打折次数
  vector<DWORD> vecQuestID; // 需完成任意一个任务才可享受打折

  bool isActivityStart() const
  {
    DWORD cur = now();
    switch (BaseConfig::getMe().getInnerBranch())
    {
    case BRANCH_DEBUG:
    case BRANCH_TF:
      if (cur < dwTFStartTime || cur >= dwTFEndTime)
        return false;
      break;
    case BRANCH_PUBLISH:
      if (cur < dwReleaseStartTime || cur >= dwReleaseEndTime)
        return false;
      break;
    default:
      return false;
    }
    return true;
  }

  bool isDiscount() const
  {
    return dwCount > 0 && dwDiscount > 0;
  }
};

// config
class ShopConfig : public xSingleton<ShopConfig>
{
  friend class xSingleton<ShopConfig>;
  private:
    ShopConfig();
  public:
    virtual ~ShopConfig();

    bool loadConfig();
    bool checkConfig();

    const ShopItem* getShopItem(DWORD dwID) const;
    void collectShopItem(DWORD dwType, DWORD dwShopID, TVecShopItem& vecItems);
    const ShopItem* getShopItemBySkill(DWORD dwID) const;
    DWORD getItemPrice(DWORD dwItemID) const;
    const TMapTypeShopItem& getAllShopItem() const { return m_mapType2Item; }
    bool canBuyItemNow(const ShopItem *pItem) const;
    
    const TSetDWORD& getManualSkillList() const { return m_setManualSkillIDs; }
    const TMapShopItem& getShopItemList() const { return m_mapID2Item; }

    bool isValidType(DWORD dwType) const { return m_setValidType.find(dwType) != m_setValidType.end(); }
    bool canDisplay(DWORD dwID) const;

    void getRandomShopItemsByLv(DWORD lv, map<DWORD, DWORD>& items);
    void getRandomShopItemsByAccLv(DWORD acclv, map<DWORD, DWORD>& items);
    //按顺序，普通zeny > 乐园币商店 > B格猫金币商店
    const ShopItem* getShopItemByItemId(DWORD itemId) const;
    const ShopItem* getItem(const TVecShopItem&vec, DWORD itemId) const;
    static bool buyLimit(EShopLimitType limitType);
    const SShopDiscountActivity* getDiscountActivity(DWORD id) const;
    
    void setWeekRand(DWORD dwGroupId);
    bool isInWeekRand(DWORD dwShopId);

    DWORD getPreGoodsID(DWORD dwGoodsId);
  private:
    void randomItems(const TMapItemWeight& items, DWORD lv, DWORD total_count, bool duplicate, map<DWORD, DWORD>& result);

    TMapShopItem m_mapID2Item;
    TMapTypeShopItem m_mapType2Item;
    TMapSkillShop m_mapSkill2Shop;
    TMapItemPrice m_mapItemPrice;
    TSetDWORD m_setValidType;
    TMapShop2ItemWeight m_mapCharShopID2Weight;
    TMapShop2ItemWeight m_mapAccShopID2Weight;
    map<DWORD, SShopDiscountActivity> m_mapItem2DiscountActivity;
    TMapItemId2ShopItem m_mapItemId2Item;
    
    DWORD m_dwWeekRandGroupId = 0;
    TSetDWORD m_setWeekRandShop;
    TSetDWORD m_setManualSkillIDs;

    //触发出售的前置商品ID
    TMapPreGoodsID m_mapPreGoodsID;
};

