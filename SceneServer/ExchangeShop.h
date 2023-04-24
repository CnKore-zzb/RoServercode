#pragma once

#include "xDefine.h"
#include <map>
#include "ExchangeShopManager.h"
#include "SessionShop.pb.h"
#include "RecordCmd.pb.h"

class ExchangeShopItemData;
typedef std::map<DWORD, ExchangeShopItemData> TMapExchangeShopItem;

class ExchangeShop
{
  public:
    ExchangeShop(SceneUser* pUser);
    ~ExchangeShop();

    bool load(const BlobExchangeShop& data);
    bool save(BlobExchangeShop* data);

    bool loadProfessionData(const Cmd::ProfessionData& data);
    bool saveProfessionData(Cmd::ProfessionData& data);

    void timer(QWORD curTime);
    void sendExchangeShopItems();
    void resetExchangeShopItems();
    void reloadExchangeShopConfig();

    void exchange(const ExchangeShopItemCmd& cmd);

    void onBaseLevelUp();
    void onJobLevelUp();
    void onExchangeShopItem();
    void onAddMedalCount();
    void onAddBattleTime();
    void onConditionTrigger(EExchangeShopConditionType cond);
    void onLogin();
    void onItemAdd(const ItemInfo& rInfo);

    void addShopItem(DWORD dwID);
    void delShopItem(DWORD dwID);
    void addToDelList(DWORD dwID);
    bool checkHasItem(DWORD dwID);
    ExchangeShopItemData* getShopItem(DWORD dwID);
    void collectBranchItem(TMapExchangeShopItem& mapItem);

    DWORD getItemGetCount(DWORD itemid);
    bool checkHasExchangedItem(DWORD dwID);
    bool checkMenuOpen();
  private:
    bool isProfessionOpen();
  private:
    SceneUser* m_pUser = nullptr;
    bool m_bMenuOpen = false;
    TMapExchangeShopItem m_mapExchangeItem;
    std::map<DWORD, DWORD> m_mapItemGetCount;
    TSetDWORD m_setHasExchangedItem;
    DWORD last_tick_sec = 0;

    TVecDWORD m_vecDelIDs;
};

class ExchangeShopItemData
{
  public:
    ExchangeShopItemData();
    ~ExchangeShopItemData();
    bool init(DWORD dwID);
    void fromData(const ExchangeShopItem& data);
    void toData(ExchangeShopItem* data);

    TPtrExchangeItemConfig getShopItemConfig() { return m_pItemConfig; }
    void setShopItemConfig(TPtrExchangeItemConfig pConfig) { m_pItemConfig = pConfig; }

    EExchangeStatusType getShopItemStatus() { return m_status; }
    void setShopItemStatus(EExchangeStatusType eStatus) { m_status = eStatus; }
    DWORD getProgress() { return m_dwProgress; }
    void setProgress(DWORD dwProgress) { m_dwProgress = dwProgress; }
    DWORD getExchangedCount() { return m_dwExchangedCount; }
    void setExchangedCount(DWORD dwCount) { m_dwExchangedCount = dwCount; }
    DWORD getLeftTime() { return m_dwLeftTime; }
    void reduceLeftTime(DWORD dwSeconds) {
      DWORD dwTemp = m_dwLeftTime;
      m_dwLeftTime = m_dwLeftTime > dwSeconds ? (m_dwLeftTime - dwSeconds) : 0;
      XDBG << "[兑换商店] id :" << m_dwId << "减少时间" << dwSeconds << "原时间" << dwTemp << "剩余" << m_dwLeftTime << XEND;
    }
    DWORD getDelayTime() { return m_dwDelayTime; }
    void setDelayTime(DWORD dwTime) { m_dwDelayTime = dwTime; }

  private:
    DWORD m_dwId = 0;
    TPtrExchangeItemConfig m_pItemConfig = nullptr;
    EExchangeStatusType m_status = EEXCHANGESTATUSTYPE_OK;
    DWORD m_dwProgress = 0;
    DWORD m_dwExchangedCount = 0;
    DWORD m_dwLeftTime = 0;
    DWORD m_dwDelayTime = 0;
};
