/**
 * @file SceneShop.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-08-24
 */

#pragma once

#include <vector>
#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "SessionCmd.pb.h"
#include "SessionShop.pb.h"
#include "GuildConfig.h"

using namespace Cmd;
using std::pair;
using std::vector;
using std::map;

namespace Cmd
{
  class BlobShopGotItem;
};

typedef pair<DWORD, DWORD> TPairShopGotItem;
typedef vector<TPairShopGotItem> TVecShopGotItem;

enum EShopType
{
  EShopType_Drawing = 800,        //商店图纸
  EShopType_Charge = 913,
  EShopType_VENDING_MACHINE = 924,        //自动贩卖机
  EShopType_Cloth = 961,
  EShopType_RandomDay = 3000,     //每日随机
  EShopType_RandomDayByLv = 3003, // 按照等级按权重每日刷新
  EShopType_RandomDayByAccLv = 3004, // 按照账号下所有角色最大等级按权重每日刷新, 角色间共用
  EShopType_RandomDayAcc = 3007,      //每日随机，账号共享
  EShopType_RandomWeek = 3008,      //每周全服随机
  EShopType_ForClient = 4000, //用于方便前端复用商店ui,后端处理逻辑不在SceneShop中的商店
};

class SceneUser;
class SceneShop
{
  public:
    SceneShop(SceneUser* pUser);
    ~SceneShop();

    bool load(const BlobShopGotItem& data);
    bool save(BlobShopGotItem* data);
    bool loadAcc(const BlobShopGotItem& data);
    bool saveAcc(BlobShopGotItem* data);
    bool queryGotItem();
    bool buyItem(DWORD dwShopID, DWORD dwCount, DWORD dwPrice = 0, DWORD dwPrice2 = 0, bool fromMatch=false);
    void queryConfigByType(QueryShopConfigCmd &rev);
    void queryQuickBuy(QueryQuickBuyConfigCmd &rev);
    bool packShopItem(const ShopItem& from, ShopItem& to, DWORD& screen, const SGuildBuildingCFG* pGBuildingCFG);
    void onLevelup(DWORD oldLv);
    void addAccBuyLimitCnt(DWORD dwShopID, DWORD dwCount);
    DWORD getShopItemCount(DWORD id);
    void setShopItemCount(DWORD id, DWORD count, bool ntf);
  private:
    void resetShopItemCount(bool notify);
    bool resetShopItemCountAcc();
    bool resetShopItemCountMonth();
    void addShopItemCount(DWORD id, DWORD count = 1);
    void randomShop();
    void randomShopAcc();
    DWORD getShopItemMaxCount(DWORD shopType, DWORD dwShopId);
    void sendGotItem();

    DWORD getRefreshLv();
    void updateRefreshLv(DWORD oldLv);
    DWORD getRandShopItemMaxCount(DWORD id);
    DWORD getAccRandShopItemMaxCount(DWORD id);
    DWORD getDiscountActItemCount(DWORD id);
    void addDiscountActItemCount(DWORD id, DWORD value);

    bool checkShowShopItem(const ShopItem& pItem);
    bool checkShowNextItem(const ShopItem& pItem);
    void updateShopConfigByPreItem(const ShopItem& pItem);

    bool canBuyItemNow(const ShopItem *pItem);
  private:
    SceneUser* m_pUser = nullptr;

    TVecShopGotItem m_vecShopGotItem;           //角色日限购
    TVecShopGotItem m_vecShopGotItemWeek;       //角色周限购
    TVecShopGotItem m_vecUserShopGotItemMonth;  //角色月限购

    TVecShopGotItem m_vecAccShopGotItemWeek;    //账号周限购
    TVecShopGotItem m_vecAccShopGotItemMonth;   //账号月限购
    

    std::map<DWORD/*shopid*/, DWORD/*limit count*/> m_mapShopRand;
    std::map<DWORD/*shopid*/, DWORD/*limit count*/> m_mapShopRandAcc;

    TVecShopGotItem m_vecAccShopGotItem;        //每日账号限购

    DWORD m_dwRefreshLv = 0;
    DWORD m_dwRefreshLvResetTime = 0;
    map<DWORD, DWORD> m_mapLvRandShopItem;
    map<DWORD, DWORD> m_mapAccLvRandShopItem;
    TVecShopGotItem m_vecAccShopGotItemAlways;    //永远不清除
    map<DWORD, DWORD> m_mapAccDiscountActItem; // 折扣活动购买数量
    map<DWORD, DWORD> m_mapAccAddLimitCnt; // 增加的购买次数<shopid, count>
    map<DWORD, DWORD> m_mapLimitItem;   //限制道具购买数量
};
