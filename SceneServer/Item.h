/**
 * @file ItemManager.h
 * @brief item manager
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-05-04
 */

#pragma once

#include <list>
#include "xEntry.h"
#include "ItemConfig.h"

// item base
class SceneUser;
class ItemBase : public xEntry
{
  public:
    ItemBase(const SItemCFG* pCFG);
    virtual ~ItemBase();

    virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) = 0;
    virtual void toItemData(ItemData* pData);
    virtual void fromItemData(const ItemData& rData);
    void toClientData(EPackType eType, ItemData* pData, SceneUser* pUser);

    bool init();

    void setCFG(const SItemCFG* pCFG) { m_pCFG = pCFG; if (m_pCFG) m_oItemInfo.set_id(m_pCFG->dwTypeID); }
    const SItemCFG* getCFG() const { return m_pCFG; }

    void setGUID(const string& id) { m_oItemInfo.set_guid(id); }
    const string& getGUID() const { return m_oItemInfo.guid(); }

    DWORD getTypeID() const { return m_oItemInfo.id(); }

    EItemType getType() const { return m_pCFG == nullptr ? m_oItemInfo.type() : m_pCFG->eItemType; }

    void setCount(DWORD count) { m_oItemInfo.set_count(count); }
    DWORD getCount() const { return m_oItemInfo.count(); }

    //void setQuality(EQualityType eQuality) { if (eQuality > EQUALITYTYPE_MIN && eQuality < EQUALITYTYPE_MAX) m_oItemInfo.set_quality(eQuality); }
    EQualityType getQuality() const { return m_pCFG == nullptr ? m_oItemInfo.quality() : m_pCFG->eQualityType; }

    void setIndex(DWORD index) { m_oItemInfo.set_index(index); }
    DWORD getIndex() const { return m_oItemInfo.index(); }

    void setCreateTime(DWORD time) { m_oItemInfo.set_createtime(time); }
    DWORD getCreateTime() const { return m_oItemInfo.createtime(); }

    void setOverTime(DWORD time) { m_oItemInfo.set_overtime(time); }
    DWORD getOverTime() const { return m_oItemInfo.overtime(); }

    void setCD(QWORD time) { m_oItemInfo.set_cd(time); }
    QWORD getCD() const { return m_oItemInfo.cd(); }

    void setSource(ESource source) { m_oItemInfo.set_source(source); }
    ESource getSource() const { return m_oItemInfo.source(); }

    void setNew(bool bnew) { m_oItemInfo.set_isnew(bnew); }
    bool getNew() const { return m_oItemInfo.isnew(); }

    void setHint(bool bhint) { m_oItemInfo.set_ishint(bhint); }
    bool getHint() const { return m_oItemInfo.ishint(); }

    const ItemInfo& GetItemInfo() { return m_oItemInfo; }
  protected:
    ItemInfo m_oItemInfo;
    const SItemCFG* m_pCFG = nullptr;
};

// item
class Item : public ItemBase, public xObjectPool<Item>
{
  public:
    Item(const SItemCFG* pCFG);
    virtual ~Item();

    virtual Item* clone();
    virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) { return false; }
    virtual void toItemData(ItemData* pData) { if (pData != NULL) ItemBase::toItemData(pData); }
    virtual void fromItemData(const ItemData& rData) { ItemBase::fromItemData(rData); }
};

// item stuff
class ItemStuff : public ItemBase, public xObjectPool<ItemStuff>
{
  public:
    ItemStuff(const SItemCFG* pCFG);
    virtual ~ItemStuff();

    virtual ItemStuff* clone();
    virtual bool use(SceneUser* pUser , const Cmd::ItemUse& itemUseCmd);
    virtual void toItemData(ItemData* pData) { if (pData != NULL) ItemBase::toItemData(pData); }
    virtual void fromItemData(const ItemData& rData) { ItemBase::fromItemData(rData); }
};

// item equip
typedef pair<DWORD, DWORD> TPairRefineCompose;
typedef vector<TPairRefineCompose> TVecRefineCompose;
typedef pair<string, DWORD> TPairEquipCard;
typedef vector<TPairEquipCard> TVecEquipCard;
typedef map<DWORD, TPairEquipCard> TMapEquipCard;
typedef std::list<DecomposeResult> TListDecomposeResult;
class ItemCard;
class ItemEquip : public ItemBase, public xObjectPool<ItemEquip>
{
  public:
    ItemEquip(const SItemCFG* pCFG);
    virtual ~ItemEquip();

    virtual ItemEquip* clone();
    virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) { return false; }
    virtual void toItemData(ItemData* pData);
    virtual void fromItemData(const ItemData& rData);
    void toEquipData(EquipData* pData);

    void fromEggEquip(const EggEquip& rEquip);
    void toEggEquip(EggEquip* pEquip);

    virtual bool canEquip(EProfession profession) const;
    DWORD getBattlePoint();
    //void collectEquipAttr(SceneUser* pUser, TVecAttrSvrs& vecAttrs);
    void collectEquipAttr(SceneUser* pUser);
    void collectEquipSkill(TVecDWORD& vecSkillIDs) const;

    bool canDecompose() const;

    void setEquiped(bool equip) { m_bEquiped = equip; }
    bool getEquiped() const { return m_bEquiped; }

    EEquipType getEquipType() const { return m_pCFG == nullptr ? m_oItemInfo.equiptype() : m_pCFG->eEquipType; }

    void setStrengthLv(DWORD lv) { m_dwStrengthLv = lv; m_bBpUpdate = true; }
    DWORD getStrengthLv() const { return m_dwStrengthLv; }
    void setStrengthLv2(DWORD lv) { m_dwStrengthLv2 = lv; m_bBpUpdate = true; }
    DWORD getStrengthLv2() const { return m_dwStrengthLv2; }
    void setRefineLv(DWORD lv) { m_dwRefineLv = lv; m_bBpUpdate = true; }
    DWORD getRefineLv() const { return m_dwRefineLv; }
    void setLv(DWORD lv) { m_dwLv = lv; }
    DWORD getLv() const { return m_dwLv; }

    void addStrengthCost(DWORD cost) { m_dwStrengthCost += cost; }
    void addStrengthCost(const TVecItemInfo& vecItems) { combinItemInfo(m_vecStrengthCost, vecItems); }
    void resetStrengthCost() { m_dwStrengthCost = 0; }
    void resetStrength2Cost() { m_vecStrengthCost.clear(); }
    DWORD getStrengthCost() const { return m_dwStrengthCost; }
    const TVecItemInfo& getStrengthItemCost() const { return m_vecStrengthCost; }

    void addRefineCompose(DWORD id, DWORD count = 1);
    void resetRefineCompose() { m_vecRefineCompose.clear(); }
    const TVecRefineCompose& getRefineCompose() const { return m_vecRefineCompose; }

    void setDamageStatus(bool status) { m_bDamage = status; }
    bool isDamaged() const { return m_bDamage; }

    void addRepairCount() { m_oRefineData.set_repaircount(m_oRefineData.repaircount() + 1); }
    DWORD getRepairCount() const { return m_oRefineData.repaircount(); }
    void setLastRefineFail(bool bfail) { m_oRefineData.set_lastfail(bfail); }
    bool isLastRefineFail() const { return m_oRefineData.lastfail(); }

    bool hasCard(ItemCard* pCard);
    bool canAddCard(ItemCard* pCard, DWORD pos, bool notReplace);
    bool addCard(ItemCard* pCard, DWORD pos);
    bool removeCard(const string& guid);
    bool removeCardByPos(DWORD pos);
    bool EquipedCard() const { return !m_mapPos2Card.empty(); }
    const TPairEquipCard* getCard(const string& guid) const;
    const TMapEquipCard& getCardList() const { return m_mapPos2Card; }
    const TPairEquipCard* getCardByPos(DWORD pos) const;
    const TPairEquipCard* getCardAndPos(const string& guid, DWORD& pos) const;
    DWORD getEmptyCardPos() const;

    void setCardSlot(DWORD count) { m_dwCardSlot = count; }
    DWORD getCardSlot() const { return m_dwCardSlot == 0 ? (m_pCFG == nullptr ? 0 : m_pCFG->dwCardSlot) : m_dwCardSlot; }
    void getEquipCard(TVecEquipCard & rVecEquipCard) const;

    bool checkBuffID(SceneUser* pUser, DWORD id);
    bool checkBuffChange(SceneUser* pUser);

    EnchantData& getEnchantData() { return m_oEncData; }
    EnchantData& getPreviewEnchantData() { return m_oPreviewEncData; }
    bool checkSameEnchant(const EnchantData& other);
    bool isEnchant() const { return m_oEncData.attrs_size() != 0 || m_oEncData.extras_size() != 0; }

    void setSpecAttr(const char* name, float value); /*仅lua 调用*/

    void setDecomposeItem(const TListDecomposeResult& vecItem) { m_listDecomposeItem = vecItem; }
    const TListDecomposeResult& getDecomposeItem() const { return m_listDecomposeItem; }

    void setDecomposeResult(EDecomposeResult eResult) { m_eDecomposeResult = eResult; }
    EDecomposeResult getDecomposeResult() const { return m_eDecomposeResult; }

    bool canGenderEquip(EGender gender) const;
    DWORD getBodyColor() const { return m_dwBodyColor; }
    void setBodyColor(DWORD color) { m_dwBodyColor = color; }

    bool canBeMaterial() const;
    bool canBeUpgradeMaterial() const;
    bool canBeQuickSell() const;
    bool canBeComposeMaterial() const;

    bool breakEquip(DWORD duration); // 破坏装备
    bool fixBrokenEquip(); // 修理装备
    bool isBroken(); // 装备是否被破坏
    DWORD getRestBreakDuration();
    void clearBrokenTime() { m_dwBreakStartTime = 0; m_dwBreakEndTime = 0; }
    
    bool canGive() { return m_oSenderData.charid() == 0; }
    void setSender(QWORD charId, string& name);
  public:
    bool checkSignUp(DWORD& dwPointOut, DWORD& dwBuffOut) const;
  private:
    void fromEquipData(const EquipData& rData);
  private:
    bool m_bEquiped = false;
    bool m_bBpUpdate = true;
    bool m_bDamage = false;

    DWORD m_dwStrengthLv = 0;
    DWORD m_dwStrengthLv2 = 0;
    DWORD m_dwRefineLv = 0;
    DWORD m_dwLv = 0;
    DWORD m_dwBattlePoint = 0;
    DWORD m_dwStrengthCost = 0;
    DWORD m_dwCardSlot = 0;
    DWORD m_dwBreakStartTime = 0;
    DWORD m_dwBreakEndTime = 0;

    EDecomposeResult m_eDecomposeResult = EDECOMPOSERESULT_MIN;

    TVecDWORD m_vecBuffIDs;

    TVecRefineCompose m_vecRefineCompose;
    TMapEquipCard m_mapPos2Card;
    TListDecomposeResult m_listDecomposeItem;

    EnchantData m_oEncData;
    EnchantData m_oPreviewEncData;
    TVecAttrSvrs m_vecSpecAttr;
    RefineData m_oRefineData;
    DWORD m_dwBodyColor = 0;

    TVecItemInfo m_vecStrengthCost;
    SenderData m_oSenderData;
};

// item treasure
class ItemTreasure : public ItemBase, public xObjectPool<ItemTreasure>
{
  public:
    ItemTreasure(const SItemCFG* pCFG);
    virtual ~ItemTreasure();

    virtual ItemBase* clone();
    virtual void toItemData(ItemData* pData) { if (pData != nullptr) ItemBase::toItemData(pData); }
    virtual void fromItemData(const ItemData& rData);

    virtual bool use(SceneUser *pUser, const Cmd::ItemUse& itemUseCmd);
};

// item card
class ItemCard : public ItemBase, public xObjectPool<ItemCard>
{
  public:
    ItemCard(const SItemCFG* pCFG);
    virtual ~ItemCard();

    virtual ItemCard* clone();
    virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) { return false; }
    virtual void toItemData(ItemData* pData);
    virtual void fromItemData(const ItemData& rData);
};

// item letter
class ItemLetter : public ItemBase, public xObjectPool<Item>
{
  public:
    ItemLetter(const SItemCFG* pCFG);
    virtual ~ItemLetter();

    virtual ItemLetter* clone();
    virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) { return false; }
    virtual void toItemData(ItemData* pData);
    virtual void fromItemData(const ItemData& rData);
  private:
    LoveLetterData m_oLetterData;
};

// item egg
class ItemEgg : public ItemEquip, public xObjectPool<ItemEgg>
{
  public:
    ItemEgg(const SItemCFG* pCFG);
    virtual ~ItemEgg();

    virtual ItemEgg* clone();
    virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) { return false; }
    virtual void toItemData(ItemData* pData);
    virtual void fromItemData(const ItemData& rData);

    virtual bool canEquip(EProfession profession) const;

    DWORD getPetID() const { return m_oEggData.id(); }
    DWORD getLevel() const { return m_oEggData.lv(); }
    DWORD getFriendLv() const { return m_oEggData.friendlv(); }
    const string& getName() const { return m_oEggData.name(); }

    EggData& getEggData() { return m_oEggData; }
    bool canSell() const;

    static DWORD getWorkSkillID(const EggData& rEgg);

    void patch_1();
  private:
    EggData m_oEggData;
};

// item code
class ItemCode : public ItemBase, public xObjectPool<Item>
{
public:
  ItemCode(const SItemCFG* pCFG);
  virtual ~ItemCode();

  virtual ItemCode* clone();
  virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) { return false; }
  virtual void toItemData(ItemData* pData);
  virtual void fromItemData(const ItemData& rData);
  
  bool canUse() const;
  bool needReqUsed() const;
  void setCode(const string& code);
  void setExchanged(bool b) { m_oCodeData.set_used(b);}
  bool isExchanged() const { return m_oCodeData.used(); }
  const string& getCode() { return m_oCodeData.code(); }
private:
  CodeData m_oCodeData;
};

// item wedding
class ItemWedding : public ItemBase, public xObjectPool<Item>
{
public:
  ItemWedding(const SItemCFG* pCFG);
  virtual ~ItemWedding();

  virtual ItemWedding* clone();
  virtual bool use(SceneUser* pUser, const Cmd::ItemUse& itemUseCmd) { return false; }
  virtual void toItemData(ItemData* pData);
  virtual void fromItemData(const ItemData& rData);

  bool uploadPhoto(DWORD index, DWORD time);
  const WeddingData& getWeddingData() const { return m_oWeddingData; }
  bool isNotifyWeddingStart();
  void setNotified() { m_oWeddingData.set_notified(true); }

  void setID(QWORD qwGUID) { m_oWeddingData.set_id(qwGUID); }
  void setMyName(const string& name) { m_oWeddingData.set_myname(name); }
  void setPartnerName(const string& name) { m_oWeddingData.set_partnername(name); }
private:
  WeddingData m_oWeddingData;
};

