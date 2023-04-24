#pragma once

#include "xDefine.h"
#include <map>
#include <set>
#include <memory>
#include "ExchangeShopCondition.h"

using std::string;
class ExchangeShopItemConfig;

typedef std::shared_ptr<BaseExchangeShopCondition> TPtrShopCond;
typedef std::vector<TPtrShopCond> TVecShopCond;
typedef std::shared_ptr<ExchangeShopItemConfig> TPtrExchangeItemConfig;
typedef std::map<DWORD, TPtrExchangeItemConfig> TMapExchangeItem;
typedef std::map<DWORD/*商品ID*/, std::map<DWORD/*item id*/, std::pair<DWORD, DWORD>/*兑换的物品ID和数量*/>> TMapExchangeWorth;

enum EExchangeType
{
  EEXCHANGETYPE_COINS = 1,   //货币兑换
  EEXCHANGETYPE_FREE = 2,    //免费兑换
  EEXCHANGETYPE_PROGRESS = 3, //多阶段兑换,带进度条
  EEXCHANGETYPE_NO_PROGRESS = 4, //普通物品兑换
  EEXCHANGETYPE_WORTH_PROGRESS = 5,//带汇率进度
};

class ExchangeShopManager: public xSingleton<ExchangeShopManager>
{
  public:
    bool loadConfig();
    bool loadWorthConfig();
    TPtrExchangeItemConfig getShopItemById(DWORD id);
    TPtrExchangeItemConfig createShopItem(DWORD id, xLuaData data);
    const TSetDWORD* getConditionItemSet(EExchangeShopConditionType cond);
    bool getExchangeWorthByItemID(DWORD dwGoodsID, DWORD dwItemID, Cmd::ItemInfo& rItem);

  private:
    TMapExchangeItem m_mapID2ExchangeItem;
    TMapExchangeWorth m_mapExchangeWorth;
    std::map<DWORD/*EExchangeShopConditionType*/, TSetDWORD> m_mapConditionItemSet;
};

class ExchangeShopItemConfig
{
  public:
    ExchangeShopItemConfig();
    virtual ~ExchangeShopItemConfig();
    virtual bool init(DWORD id, xLuaData& data);
    bool createCondition(xLuaData& data, TVecShopCond& vecCond);
    void registConditionItemSet(std::map<DWORD, TSetDWORD>& mapCondItemSet);
  
  public:
    virtual bool checkStart(SceneUser*);
    virtual bool checkEnd(SceneUser*);
    virtual bool checkItemCount(SceneUser* pUser, const TVecItemInfo& vecItems);
    virtual void calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems) {}
    virtual bool reduceItem(SceneUser* pUser, const TVecItemInfo& vecChoosedItems);
    virtual bool addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems);

  public:
    DWORD getID() { return m_dwID; }
    string getName() { return m_strName; }
    bool isOn() { return m_bOnOff; }
    DWORD getExchangeType() { return m_dwExchangeType; }
    DWORD getLimitTime() { return m_dwLimitTime; }
    bool delayStart() {return m_bDelayStart; }
    DWORD getCanExchangeCount(DWORD dwProgress, DWORD dwExchangedCount);
    bool isBranch() const { return m_bBranch; }
  
  protected:
    DWORD m_dwID = 0;  //商品配置ID
    string m_strName = "";  //商品名字
    bool m_bOnOff = false;  //生效状态
    DWORD m_dwExchangeType = 0;  //兑换类型 EExchangeType
    DWORD m_dwShopType = 0;  //商品类型  策划自定义 同系列的类型一致
    bool m_bDelayStart = false; //是否延迟出现
    DWORD m_dwLimitTime = 0;  //兑换限时 单位天
    DWORD m_dwCostID = 0;    //消耗的物品ID
    DWORD m_dwCostNum = 0;   //消耗的数量
    DWORD m_dwGetID = 0;    //兑换获得的物品ID
    DWORD m_dwGetNum = 0;   //兑换获得的数量
    bool m_bAutoUse = 0;    //获得后是否自动使用
    bool m_bBranch = false;
    
    TVecDWORD m_vecProgressLimit;  //每阶段的兑换上限，对兑换类型是EEXCHANGETYPE_PROGRESS的有用
    TVecShopCond m_vecStartCond;  //出现条件列表
    TVecShopCond m_vecEndCond;   //消失条件列表
};

class CoinsExchangeItemConfig : public ExchangeShopItemConfig
{
  public:
    CoinsExchangeItemConfig();
    virtual ~CoinsExchangeItemConfig();
    virtual bool init(DWORD id, xLuaData& data);
    virtual bool checkItemCount(SceneUser* pUser, const TVecItemInfo& vecItems);
    virtual void calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems);
    virtual bool reduceItem(SceneUser* pUser, const TVecItemInfo& vecChoosedItems);
    virtual bool addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems);
};

class FreeExchangeItemConfig : public ExchangeShopItemConfig
{
  public:
    FreeExchangeItemConfig();
    virtual ~FreeExchangeItemConfig();
    virtual bool init(DWORD id, xLuaData& data);
    virtual bool checkItemCount(SceneUser* pUser, const TVecItemInfo& vecItems);
    virtual void calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems);
    virtual bool reduceItem(SceneUser* pUser, const TVecItemInfo& vecChoosedItems);
    virtual bool addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems);
};

class ProgressExchangeItemConfig : public ExchangeShopItemConfig
{
  public:
    ProgressExchangeItemConfig();
    virtual ~ProgressExchangeItemConfig();
    virtual bool init(DWORD id, xLuaData& data);
    virtual void calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems);
    virtual bool addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems);
};

class ItemExchangeItemConfig : public ExchangeShopItemConfig
{
  public:
    ItemExchangeItemConfig();
    virtual ~ItemExchangeItemConfig();
    virtual bool init(DWORD id, xLuaData& data);
    virtual void calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems);
};

class WorthExchangeItemConfig : public ExchangeShopItemConfig
{
  public:
    WorthExchangeItemConfig();
    virtual ~WorthExchangeItemConfig();
    virtual bool init(DWORD id, xLuaData& data);
    virtual void calcExchangedItems(SceneUser* pUser, TVecItemInfo& vecChoosedItems, TVecItemInfo& vecExchangedItems);
    virtual bool addItem(SceneUser* pUser, const TVecItemInfo& vecExchangedItems);
};

