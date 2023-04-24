/**
 * @file Package.h
 * @brief package
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-04-24
 */

#pragma once

#include "xDefine.h"
#include "xPool.h"
#include "Item.h"
#include "MiscConfig.h"
#include "RecordCmd.pb.h"
#include "SceneItem.pb.h"
#include "ProtoCommon.pb.h"
#include "AuctionSCmd.pb.h"
#include "RewardConfig.h"

using namespace Cmd;
using std::string;
using std::vector;
using std::set;
using std::pair;

class SceneUser;
class xSceneEntryDynamic;
class OtherFactor;
const DWORD PACKAGE_VERSION = 7;

enum EPackMethod
{
  EPACKMETHOD_MIN = 0,
  EPACKMETHOD_NOCHECK = 1,
  EPACKMETHOD_CHECK_WITHPILE = 2,
  EPACKMETHOD_CHECK_NOPILE = 3,
  EPACKMETHOD_AVAILABLE = 4,
  EPACKMETHOD_MAX
};
enum ECheckMethod
{
  ECHECKMETHOD_MIN = 0,
  ECHECKMETHOD_NORMAL = 1,
  ECHECKMETHOD_NONORMALEQUIP = 2,
  ECHECKMETHOD_UPGRADE = 3,
  ECHECKMEHHOD_MAX,
};

class RewardManager
{
  private:
    RewardManager() {}
    ~RewardManager() {}

  public:
    static bool roll(DWORD id, SceneUser* pUser, TVecItemInfo& vecItemInfo, ESource source, float fRatio = 1.0f, DWORD dwMapID = 0);
    /**
     * 暂时仅用于宠物冒险,逻辑与roll基本一致
     */
    static bool roll_adventure(DWORD id, SceneUser* pUser, TVecItemInfo& vecItemInfo, ESource source, const OtherFactor& sFactor, float fRatio = 1.0f, DWORD dwMapID = 0);
};

// package
class BasePackage;
typedef vector<pair<DWORD, DWORD>> TVecItemCount;
typedef std::map<DWORD/*item id*/, time_t/*frost time*/> TMapFrost;
typedef map<EPackType, TVecItemInfo> TMapReduce;

struct SItemGetCount
{
  DWORD dwCount = 0;
  std::map<ESource, DWORD> mapSource2Count;
};
typedef std::map<DWORD, SItemGetCount> TMapItemGetCount;

struct SEquipPosData
{
  EEquipPos ePos = EEQUIPPOS_MIN;
  DWORD dwOffStartTime = 0;
  DWORD dwOffEndTime = 0;
  DWORD dwProtectTime = 0;
  DWORD dwProtectAlways = 0;
  string strGuid;

  bool fromData(const EquipPosData& data) {
    ePos = data.pos();
    dwOffStartTime = data.offstarttime();
    dwOffEndTime = data.offendtime();
    dwProtectTime = data.protecttime();
    dwProtectAlways = data.protectalways();

    if (!data.recordguid().empty())
      strGuid = data.recordguid();
    return true;
  }

  bool toData(EquipPosData* data) {
    if (!data)
      return false;
    data->set_pos(ePos);
    data->set_offstarttime(dwOffStartTime);
    data->set_offendtime(dwOffEndTime);
    data->set_protecttime(dwProtectTime);
    data->set_protectalways(dwProtectAlways);

    if (!strGuid.empty())
      data->set_recordguid(strGuid);
    return true;
  }
};
typedef map<EEquipPos, SEquipPosData> TMapEquipPos;
typedef map<EPackType, TVecItemData> TMapPackMoveItem;

typedef vector<Cmd::EquipInfo> TVecEquipInfo;
typedef map<EPackType, TVecEquipInfo> TMapType2Equip;

static const vector<EPackType> TVEC_REDUCE_PACKS = vector<EPackType>{EPACKTYPE_MAIN, EPACKTYPE_BARROW, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_TEMP_MAIN, EPACKTYPE_QUEST, EPACKTYPE_FOOD, EPACKTYPE_PET};

class EquipPackage;
class Package
{
  public:
    Package(SceneUser* pUser);
    ~Package();

    bool load(const BlobPack& oAccData, const BlobPack& oData/*, const string& oStore*/);
    bool save(BlobPack* pAccBlob, BlobPack* pBlob);
    void reload();

    bool loadProfessionData(const Cmd::ProfessionData& data, bool isNeedPutOn = true);
    bool saveProfessionData(Cmd::ProfessionData& data);

    bool getCardAnim() const { return m_bCardOperationAnim; }
    void refreshSlot();

    bool toClient(EPackType eType, PackageItem& rItem);
    void sendPackData(EPackType eType);

    bool addItemCount(ItemBase* pBase);
    bool decItemCount(DWORD id);
    DWORD getItemCount(DWORD id);

    DWORD collectItemCount(DWORD id);

    bool checkAndFillItemData(ItemData& rInfo);

    bool addItem(ItemInfo rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    bool addItem(TVecItemInfo vecInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    bool addItem(ItemData rData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    bool addItem(TVecItemData vecData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);

    void luaAddItem(DWORD itemid, DWORD count, DWORD luaParam);

    bool checkItemCount(const ItemInfo& oItem, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, const TSetDWORD& setPack = {EPACKTYPE_MAIN, EPACKTYPE_QUEST});
    bool reduceItem(const ItemInfo& oItem, ESource eSource, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, const TSetDWORD& setPack = {EPACKTYPE_MAIN, EPACKTYPE_QUEST});
    bool checkItemCount(const TVecItemInfo& vecItem, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, const TSetDWORD& setPack = {EPACKTYPE_MAIN, EPACKTYPE_QUEST});
    bool reduceItem(const TVecItemInfo& vecItem, ESource eSource, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, const TSetDWORD& setPack = {EPACKTYPE_MAIN, EPACKTYPE_QUEST});

    bool checkItemCount(const ItemInfo& oItem, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);
    bool reduceItem(const ItemInfo& oItem, ESource eSource, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);
    bool checkItemCount(const TVecItemInfo& vecItem, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);
    bool reduceItem(const TVecItemInfo& vecItem, ESource eSource, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);

    bool checkItemCount(const string& guid, DWORD count, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);
    bool reduceItem(const string& guid, DWORD count, ESource eSource, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);

    bool rollReward(DWORD dwRewardID, EPackMethod = EPACKMETHOD_NOCHECK,
        bool bShow = false, bool bInform = true, bool bForceShow = false, DWORD times = 0, bool bUnlock = false, ESource source=ESOURCE_REWARD);

    BasePackage* getPackage(EPackType eType);
    ItemBase* getItem(const string& guid);
    ItemBase* getItem(const string& guid, BasePackage** pPkg);
    ItemBase* getTradeSellItem(const string& guid, BasePackage**pPkg);
    EquipPackage* getEquipPackage();

    bool checkDel(const string& guid);
    bool equip(EEquipOper oper, EEquipPos ePos, const string& guid, bool bTransfer = false, bool bWait = false, DWORD dwCount = 0);
    bool sellItem(const SellItem& cmd);
    bool strength(EStrengthType type, const string& guid, DWORD count);
    bool produce(DWORD id, QWORD qwNpcID, EProduceType eType, bool bQucik = false);
    //bool refine(const string& guid, DWORD composeid, QWORD npcid, bool safeRefine, bool oneclick = false);
    bool refine(const EquipRefine& cmd, bool oneclick = false);
    bool refineForce(const string& guid, DWORD newLv, DWORD composeId, DWORD count);
    bool decompose(const string& guid, bool bKeepOriEquip, DWORD& returnMoney);
    bool queryEquipData(const string& guid);
    bool browsePackage(EPackType eType, bool bNotify = true);
    bool equipcard(ECardOper oper, const string& cardguid, const string& equipguid, DWORD addpos = 0);
    bool restore(const RestoreEquipItemCmd& cmd);
    bool exchangeCard(ExchangeCardItemCmd& cmd);
    bool hatchEgg(const EggHatchPetCmd& cmd);
    bool restoreEgg(const EggRestorePetCmd& cmd);
    void useItemCode(UseCodItemCmd& cmd);
    void useItemCodeSessionRes(const UseItemCodeSessionCmd& cmd);
    void reqUsedItemCode();
    void reqUsedItemCodeSessionRes(const ReqUsedItemCodeSessionCmd& cmd);
    EError equipCompose(const EquipComposeItemCmd& cmd);

    /*移除装备上的卡片*/
    bool removeEquipcard(const string& cardguid, const string& equipguid);
    bool equipAllCardOff(const string& equipguid);
    bool ride(ERideType eType, DWORD dwID = 0);
    bool repair(const string& targetGuid, const string& stuffid);
    bool saveHint(DWORD dwItemID);
    bool saveOnce(DWORD dwItemID);
    bool enchant(EEnchantType eType, const string& guid);
    bool specialEnchant(const string& guid);
    bool clearEnchantBuffid(char* guid);
    bool processEnchant(bool bSave, const string& guid);
    bool exchange(const string& guid);
    bool upgrade(const string& guid);
    bool Decompose(const string& guid);
    bool queryDecomposeResult(const string& guid);

    bool equipOff(EEquipPos ePos, bool bSlotCheck = false);
    bool equipOff(const string& guid, int index = 0, bool bSlotCheck = false);
    bool equipOffAritifact(); // 拆卸神器

    bool canHint(EPackType eType, ItemBase* pItem);
    bool isHint(DWORD dwItemID) { return m_setHintItemIDs.find(dwItemID) != m_setHintItemIDs.end(); }
    bool isOnce(DWORD dwItemID) { return m_setOnceItemIDs.find(dwItemID) != m_setOnceItemIDs.end(); }

    void refreshEnableBuffs(ItemBase* pItem = nullptr);
    void timer(DWORD curSec);

    // arrow
    void changeArrow(DWORD typeID);
    DWORD getArrowTypeID() const { return m_dwActiveArrow; }

    //Trade
    /*交易，出售物品*/
    bool tradeAddItem(const TradeItemBaseInfo& tradeBaseInfo);
    /*交易，出售物品*/
    bool tradePreReduceItem(TradeItemBaseInfo* tradeBaseInfo, ESource eSource);
    bool tradeReduceItem(TradeItemBaseInfo* tradeBaseInfo, ESource eSource);    
    //拍卖行获得钱或者物品
    bool auctionAddItem(const Cmd::TakeRecordSCmd& rev);

    // 每天使用次数
    DWORD getVarUseCnt(DWORD itemid);
    void setVarUseCnt(DWORD itemid, DWORD value);
    void sendVarUseCnt(DWORD itemid);

    // 道具获得次数
    DWORD getVarGetCnt(DWORD itemid, ESource source);
    void addVarGetCnt(DWORD itemid, DWORD value, ESource source);
    void sendVarGetCnt(DWORD itemid, ESource source);
    DWORD updateVarRealGetCnt(const SItemCFG* pCFG, DWORD itemid, DWORD dwCount, ESource source, EPackType packtype);

    // return strengh cost
    void backStrengthCost(ItemEquip* pEquip);
    void addStrengthCost(DWORD dwItemID, DWORD dwCount);

    // 开关 仓库
    bool setStoreStatus(bool open);

    void refreshMaxSlot();

    bool checkItemCountByID(DWORD id, DWORD count, EPackType eType);
    //强制更新插卡洞的数量
    void setCardSlotForGM(DWORD itemId);
    void checkInvalidEquip(bool bNotify = false);
    void fixInvalidEnchant();
    void patch_equip_strengthlv();

    //void clearOffStoreLock() { m_dwOffStoreLock = 0; }
    void fixEnchantAttr();

    //bool canStoreOpen() { return m_dwStoreTick + 10 < now(); }
    //void setStoreTick(DWORD dwTime) { m_dwStoreTick = dwTime; }

    // lua 调用
    DWORD getPackSlotUsed(DWORD packtype);
    DWORD getPackSlot(DWORD packtype);

    // 相关操作
    void itemModify(DWORD dwOldID, DWORD dwNewID);
    void itemRemove(DWORD dwItemID, EPackType eType, ESource eSource);
    bool isMultipleUse() const { return m_multipleUse; }
    void startMultipleUse() { m_multipleUse = true; }
    void stopMultipleUse();
    bool addShowItems(const ItemInfo& rInfo, bool bShow, bool bInform, bool bForceShow);
    bool addShowItems(const ItemInfo& rInfo, bool bShow, bool bInform, bool bForceShow, TVecItemInfo& rShowItems, TVecItemInfo& rInfoItems);
    void showItems(TVecItemInfo& rShowItems, TVecItemInfo& rInfoItems);

    bool breakEquip(EEquipPos pos, DWORD duration, xSceneEntryDynamic* entry); // 破坏装备
    bool forceOffEquip(EEquipPos pos, DWORD duration, xSceneEntryDynamic* entry); // 脱卸装备
    bool protectEquip(EEquipPos pos, DWORD duration, bool always); // 保护装备位
    bool recoverOffEquip(EEquipPos pos = EEQUIPPOS_MIN); //恢复脱卸的装备
    bool recoverOffEquipByTime(DWORD time); // 恢复一定时间内被脱卸的装备
    void cancelAlwaysProtectEquip(EEquipPos pos); // 取消装备位永久保护
    bool fixBrokenEquip(EEquipPos pos); // 修理被破坏的装备
    bool isEquipProtected(EEquipPos pos); // 装备是否处于保护中
    bool isEquipForceOff(EEquipPos pos); // 装备是否被脱卸
    bool hasEquipForceOff(); // 判断是否有装备被脱卸
    void sendEquipPosData(EEquipPos pos = EEQUIPPOS_MIN);
    void addEquipAttrAction(const string& guid, DWORD duration);
    void equipOffInValidEquip();
    EError resetGenderEquip(EGender eGender);

    bool colorBody(EPackType pkgType, DWORD color);

    void addPackMoveItem(EPackType eTo, const ItemData& rData) { combineItemData(m_mapPackMoveItem[eTo], rData); }
    void refreshDeleteTimeItems();

    void setOperItem(const string& guid);
    const string& getOperItem() const { return m_strOperItem; }

    bool hasEquipedCard(DWORD cardid);

    bool hasWeddingManual(QWORD qwWeddingID);
    //赠送婚纱
    bool giveWeddingDress(Cmd::GiveWeddingDressCmd& rev);
    bool hasPet(DWORD dwPetID, DWORD dwBaseLv, DWORD dwFriendLv);

    void resetPetWorkItem();
    void resetFoodPackItem();
    void resetFoodItem();
    void resetQuestManualItem();
    void resetPetPackItem();
    void resetPetItem();
    bool cardDecompose(const map<string, DWORD>& cardguids, TVecItemInfo& outitem);

    void onAddSkill(DWORD dwSkillID);
    void checkResetRestoreBookItem(DWORD dwSkillID);

    EError quickStoreItem(const QuickStoreItemCmd& cmd);
    EError quickSellItem(const QuickSellItemCmd& cmd);
    EError enchantTrans(const EnchantTransItemCmd& cmd);
    // reward保底
    TMapRewardSafetyItem& getRewardSafetyMap() { return m_mapRewardSafetyItem; }
    RewardSafetyItem* getRewardSafetyItem(DWORD id) { auto it = m_mapRewardSafetyItem.find(id); return it == m_mapRewardSafetyItem.end() ? nullptr : &it->second; }
    bool clearRewardSafetyItem(DWORD id);
  private:
    void version_update();
    void version_1();
    void version_2();
    void version_3();
    void version_4();
    void version_5();
    void version_6();
    void version_7();

    void resetQuestPackItem();
    void resetPackMoveItem();

    bool canReduce(EPackType eType) const;
    bool makeReduceList(TVecItemInfo vecItem, TMapReduce& mapReduce, ECheckMethod eMethod, const TSetDWORD& setPack);
    bool makeReduceList(const string& guid, DWORD count, TMapReduce& mapReduce, ECheckMethod eMethod, const TSetDWORD& setPack);
  private:
    bool createPackage();

    bool equipOn(EEquipPos ePos, const string& guid, bool bTransfer = false);
    bool equipOffAll();
    bool equipPutFashion(EEquipPos ePos, const string& guid, bool bTransfer = false);
    bool equipOffFashion(const string& guid, int index = 0);
    bool putStore(const string& guid, DWORD dwCount);
    bool offstore(const string& guid, DWORD dwCount);
    bool putPStore(const string& guid, DWORD dwCount);
    bool offPStore(const string& guid, DWORD dwCount);
    bool offTemp(const string& guid);
    bool putBarrow(const string& guid, DWORD dwCount);
    bool offBarrow(const string& guid, DWORD dwCount);
    bool equipDressUpOn(EEquipPos ePos, const string& guid);
    bool equipDressUpOff(EEquipPos ePos, const string& guid);

    bool cardOn(const string& cardguid, const string& equipguid, DWORD addpos);

    /*bDelCard:卸载的同时删除掉卡片*/
    bool cardOff(const string& cardguid, const string& equipguid, DWORD removepos, bool bDelCard = false);

    bool transfer(ItemEquip* pFrom, ItemEquip* pTo);

    void updateFashionPackage();

  private:
    BasePackage* m_pPackage[EPACKTYPE_MAX];

    TVecItemCount m_vecItemCount;
    TVecDWORD m_vecItemChangeIDs;
    TSetDWORD m_setHintItemIDs;
    TSetDWORD m_setOnceItemIDs;

    TVecItemCount m_vecItemUseCount;
    TMapItemGetCount m_mapItemGetCount;
    SceneUser* m_pUser = nullptr;

    DWORD m_dwActiveArrow = 0;
    DWORD m_dwDataVersion = 0;
    bool m_bStoreOpened = false;
    //DWORD m_dwOffStoreLock = 0;
    DWORD m_dwStoreTick = 0;

    bool m_bCardOperationAnim = false;
    bool m_multipleUse = false;
    TVecItemInfo m_vecShowItems;
    TVecItemInfo m_vecInformItems;
    TMapEquipPos m_mapEquipPos;

    TVecItemInfo m_vecStrengthCost;

    TMapPackMoveItem m_mapPackMoveItem;

    string m_strOperItem;

    TMapRewardSafetyItem m_mapRewardSafetyItem;
};

// base package
typedef vector<ItemBase*> TVecSortItem;
typedef map<string, ItemBase*> TMapPackItem;
typedef set<ItemBase*> TSetItemBase;
typedef map<DWORD, TSetItemBase> TMapTypeItem;
typedef map<string, ItemData> TMapInvalidItem;
class BasePackage
{
  friend class Package;
  friend class GMCommandRuler;
  public:
    BasePackage(SceneUser* pUser);
    virtual ~BasePackage();

    virtual EPackType getPackageType() const = 0;
    virtual bool toClient(PackageItem& rItem);
    virtual bool fromData(const PackageData& rData);

    template<class T> void foreach(T func) { for_each(m_mapID2Item.begin(), m_mapID2Item.end(), [func](const TMapPackItem::value_type& r) {func(r.second);}); }
    const TMapInvalidItem& getInvalidItem() const { return m_mapID2InvalidItem; }

    void setMaxIndex(DWORD index) { m_dwMaxIndex = index; }
    DWORD getMaxIndex() const { return m_dwMaxIndex; }
    DWORD getNextIndex() { m_dwMaxIndex += 1; return m_dwMaxIndex; }

    DWORD getMaxSlot();
    DWORD getLastMaxSlot() const { return m_dwLastMaxSlot; }
    bool isFull() { return m_mapID2Item.size() >= getMaxSlot(); }
    void refreshSlot() { m_bRefreshSlot = true; }
    DWORD getUsedSlot() { return m_mapID2Item.size(); }

    //void getEquipIDs(TVecDWORD& vecItemID);
    void getEquipItems(TVecSortItem& pVecItems);
    void clear();

    void setUpdateIDs(const string& guid) { m_setUpdateIDs.insert(guid); }
    void removeUpdateIDs(const string& guid) { m_setUpdateIDs.erase(guid); }
    void clearUpdateIDs() { m_setUpdateIDs.clear(); }
    const set<string>& getUpdateIDs() const { return m_setUpdateIDs; }

    const string& getGUIDByType(DWORD dwTypeID) const;
    bool setCDByType(DWORD dwType);
    const TSetItemBase& getItemBaseList(DWORD dwTypeID) const;

    bool fourceShowItem(EItemType itemType, ESource source);
    bool setCardSlotForGM(DWORD itemId);

    virtual bool checkAddItemObj(const ItemBase* pBase);
    virtual void addItemObj(ItemBase* pBase, bool bInit = false, ESource source = ESOURCE_PACKAGE);
    virtual ItemBase* getItem(const string& guid);
    virtual DWORD getItemCount(DWORD id, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP);
    virtual bool hasItem(DWORD id, DWORD minstrengthlv, DWORD maxstrengthlv);

    // 新添加方式
    virtual bool checkAddItem(const ItemInfo& rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE);
    virtual bool checkAddItem(const TVecItemInfo& vecInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE);
    virtual bool checkAddItem(const ItemData& rData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE);
    virtual bool checkAddItem(const TVecItemData& vecData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE);

    virtual bool addItem(ItemInfo& rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    virtual bool addItem(TVecItemInfo& vecInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    virtual bool addItem(ItemData& rData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    virtual bool addItem(TVecItemData& vecData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);

    virtual bool addItem(const ItemInfo& rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false)
    { ItemInfo oInfo; oInfo.CopyFrom(rInfo); return addItem(oInfo, eMethod, bShow, bInform, bForceShow);}
    virtual bool addItem(const TVecItemInfo& vecInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false)
    { TVecItemInfo oInfo = vecInfo; return addItem(oInfo, eMethod, bShow, bInform, bForceShow);}
    virtual bool addItem(const ItemData& rData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false)
    { ItemData oData; oData.CopyFrom(rData); return addItem(oData, eMethod, bShow, bInform, bForceShow);}
    virtual bool addItem(const TVecItemData& vecData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false)
    { TVecItemData oData = vecData; return addItem(oData, eMethod, bShow, bInform, bForceShow);}

    virtual bool checkItemCount(DWORD itemid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
    virtual void reduceItem(DWORD itemid, ESource source, DWORD count = 1, bool bDelete = true, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
    virtual bool checkItemCount(const string& guid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
    virtual void reduceItem(const string& guid, ESource source, DWORD count = 1, bool bDelete = true, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
    virtual bool checkItemCount(const TVecItemInfo& vecItems, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
    virtual void reduceItem(const TVecItemInfo& vecItems, ESource source, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);

    virtual void packageSort(SceneUser* pUser);
    virtual ItemEquip* getEquip(EEquipPos ePos) { return nullptr; }

    virtual void refreshDeleteTimeItems();
    virtual void collectInValidEquip(TSetString& setIDs) {}
    void modifyOverTime(ItemBase* pBase);

    //只做平台日志用
    void setLogSrc(ESource src) { m_logSource = src; }
    void clearLogSrc() { m_logSource = ESOURCE_NORMAL; }
    const TSetString& getAllArtifact() { return m_setArtifactGuid; }

    void show()
    {
      XLOG << getPackageType();
      for (auto &m : m_mapID2Item)
      {
        ItemData oData;
        m.second->toItemData(&oData);
        XLOG << oData.ShortDebugString();
      }
      XLOG << XEND;
    }
  public:
    DWORD m_logRefineLv = 0;    //精炼等级 只做平台日志用
  public:
    virtual void update(DWORD curSec);
  protected:
    virtual void addShowItems(const ItemInfo& rInfo, bool bShow, bool bInform,bool bForceShow=false);
    virtual void showItems();
    virtual void addDelInformItems(DWORD id, DWORD count, ESource eSource);

    void modifyEgg(EggData* pEgg);

    void addGarbageItem(ItemBase* pBase);
    void resetGarbageItem();
  private:
    //物品获取日志
    void itemLog(const ItemInfo& rInfo, DWORD after, ESource source);
    //物品使用日志
    void propsLog(DWORD id, DWORD  decCount, DWORD after, ESource source, const std::string& jsonStr=std::string());
  protected:
    TMapPackItem m_mapID2Item;
    TMapTypeItem m_mapTypeID2Item;
    TMapInvalidItem m_mapID2InvalidItem;
    TVecItemInfo m_vecShowItems;
    TVecItemInfo m_vecInformItems;
    TVecItemInfo m_vecInformDelItems;
    TSetItemBase m_setGarbageItems;

    TSetString m_setUpdateIDs;

    SceneUser* m_pUser = nullptr;

    DWORD m_dwMaxIndex = 0;
    DWORD m_dwLastMaxSlot = 0;
    bool m_bShowItem = false;
    ESource m_logSource = ESOURCE_NORMAL;
    bool m_bRefreshSlot = false;

    TSetString m_setArtifactGuid;
    map<string, DWORD> m_mapDeleteID2Item;
};

// main package
class MainPackage : public BasePackage, public xObjectPool<MainPackage>
{
  public:
    MainPackage(SceneUser* pUser);
    virtual ~MainPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_MAIN; }

    const string& getMountGUID(DWORD dwID) const;
    bool checkSignUp(const std::string& guid, DWORD& dwPointOut, DWORD& dwBuffOut);
};

// equip package
typedef map<DWORD, ItemEquip*> TMapItemEquip;
class EquipPackage : public BasePackage, public xObjectPool<EquipPackage>
{
  public:
    EquipPackage(SceneUser* pUser);
    virtual ~EquipPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_EQUIP; }
    virtual bool fromData(const PackageData& rData);

    virtual bool canEquip(SceneUser* pUser, const ItemBase* pBase, bool bNotice = false);
    virtual bool setEquip(SceneUser* pUser, EEquipPos ePos, ItemBase* pBase, bool bInit = false);
    virtual ItemEquip* getEquip(EEquipPos ePos);

    // 屏蔽基类接口
    virtual bool checkAddItem(const ItemInfo& rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItem(const TVecItemInfo& vecInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItem(const ItemData& rData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItem(const TVecItemData& vecData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItemObj(const ItemBase* pBase) { return false; }

    virtual bool checkItemCount(DWORD itemid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL) { return false; }
    virtual bool checkItemCount(const string& guid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL) { return false; }
    virtual bool checkItemCount(const TVecItemInfo& vecItems, ECheckMethod eMethod = ECHECKMETHOD_NORMAL) { return false; }

    virtual void packageSort(SceneUser* pUser) {}

    virtual void refreshDeleteTimeItems() {}
    virtual void collectInValidEquip(TSetString& setIDs);

    DWORD getEquipSuitNum(SceneUser* pUser, DWORD suitid);
    DWORD getCardSuitNum(SceneUser* pUser, DWORD suitid);

    DWORD getEquipedItemNum(DWORD itemid, bool checkBreak = false);

    // 获取套装装备
    void getSuitEquipItems(SceneUser* pUser, DWORD suitid, TMapItemEquip& mapData);
    // 是否满足suitid套装
    bool isSuitValid(DWORD suitid);
    // 计算套装加成属性
    //void collectSuitAttr(TVecAttrSvrs& vecAttrs);
    void collectSuitAttr();

    bool hasBraokenEquip();
};

// fashion package
class FashionPackage : public BasePackage, public xObjectPool<FashionPackage>
{
  public:
    FashionPackage(SceneUser* pUser);
    virtual ~FashionPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_FASHION; }

    bool checkFashion(DWORD itemid);
    void addFashion(DWORD itemid);
    void decFashion(DWORD itemid);

    // 屏蔽基类接口
    virtual bool checkAddItem(const ItemInfo& rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItem(const TVecItemInfo& vecInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItem(const ItemData& rData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItem(const TVecItemData& vecData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE) { return false; }
    virtual bool checkAddItemObj(const ItemBase* pBase) { return false; }

    virtual bool checkItemCount(DWORD itemid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL) { return false; }
    virtual bool checkItemCount(const string& guid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL) { return false; }
    virtual bool checkItemCount(const TVecItemInfo& vecItems, ECheckMethod eMethod = ECHECKMETHOD_NORMAL) { return false; }

    virtual void packageSort(SceneUser* pUser) {}
};

// fashion equip package
class FashionEquipPackage : public BasePackage, public xObjectPool<FashionEquipPackage>
{
  public:
    FashionEquipPackage(SceneUser* pUser);
    virtual ~FashionEquipPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_FASHIONEQUIP; }
    virtual bool fromData(const PackageData& rData);

    virtual bool canEquip(SceneUser* pUser, const ItemBase* pBase);
    virtual bool setEquip(SceneUser* pUser, EEquipPos ePos, ItemBase* pBase, bool bInit = false);
    virtual ItemEquip* getEquip(EEquipPos ePos);

    virtual void refreshDeleteTimeItems() {}
    virtual void collectInValidEquip(TSetString& setIDs);
};

// store package
class StorePackage : public BasePackage, public xObjectPool<StorePackage>
{
  public:
    StorePackage(SceneUser* pUser);
    virtual ~StorePackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_STORE; }

    /*void updateToData(std::function<void(EError)> func);
    void doFunc(DWORD dwSessionID, EError eError);
  private:
    map<DWORD, std::function<void(EError)>> m_mapRecallFuncs;*/
};

// personal store package
class PersonalStorePackage : public BasePackage, public xObjectPool<PersonalStorePackage>
{
  public:
    PersonalStorePackage(SceneUser* pUser);
    virtual ~PersonalStorePackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_PERSONAL_STORE; }
};

// temp main package
class TempMainPackage : public BasePackage, public xObjectPool<TempMainPackage>
{
  friend class Package;
  public:
    TempMainPackage(SceneUser* pUser);
    virtual ~TempMainPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_TEMP_MAIN; }

    virtual bool checkAddItemObj(const ItemBase* pBase);

    virtual bool addItem(ItemInfo& rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    virtual bool addItem(TVecItemInfo& vecInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    virtual bool addItem(ItemData& rData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);
    virtual bool addItem(TVecItemData& vecData, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE, bool bShow = false, bool bInform = true, bool bForceShow = false);

    virtual void reduceItem(const string& guid, ESource source, DWORD count = 1, bool bDelete = true, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);

    //void refreshOverTimeItems();
    virtual void refreshDeleteTimeItems();
  protected:
    virtual void update(DWORD curSec);
  private:
    DWORD m_dwNextRefreshTime = 0;
};

// barrow package
class BarrowPackage : public BasePackage, public xObjectPool<BarrowPackage>
{
  public:
    BarrowPackage(SceneUser* pUser);
    virtual ~BarrowPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_BARROW; }

    virtual bool checkAddItem(const ItemInfo& rInfo, EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE);
};

// quest package
class QuestPackage : public BasePackage, public xObjectPool<QuestPackage>
{
  public:
    QuestPackage(SceneUser* pUser);
    virtual ~QuestPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_QUEST; }
};

// food package
class FoodPackage : public BasePackage, public xObjectPool<FoodPackage>
{
  public:
    FoodPackage(SceneUser* pUser);
    virtual ~FoodPackage();

    virtual EPackType getPackageType() const { return EPACKTYPE_FOOD; }

    virtual DWORD getItemCount(DWORD id, ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP);

    virtual bool checkItemCount(DWORD itemid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
    virtual bool checkItemCount(const string& guid, DWORD count = 1, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
    virtual bool checkItemCount(const TVecItemInfo& vecItems, ECheckMethod eMethod = ECHECKMETHOD_NORMAL);
};

// pet package
class PetPackage : public BasePackage, public xObjectPool<PetPackage>
{
  public:
    PetPackage(SceneUser* pUser) : BasePackage(pUser) {}
    virtual ~PetPackage() {}

    virtual EPackType getPackageType() const { return EPACKTYPE_PET; }
};

