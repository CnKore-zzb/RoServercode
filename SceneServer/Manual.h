/**
 * @file Manual.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-11-30
 */

#pragma once

#include "SceneManual.pb.h"
#include "xDefine.h"
#include "ManualConfig.h"
#include "NpcConfig.h"

using namespace Cmd;
using std::vector;
using std::string;
using std::set;
using std::pair;

namespace Cmd
{
  class BlobManual;
  class BlobUnsolvedPhoto;
};

const DWORD MAX_MANUAL_PARAM = 3;
const DWORD MANUAL_VERSION = 3;

// data
struct SManualQuest
{
  DWORD dwID = 0;
  DWORD dwProcess = 0;

  const SManualQuestCFG* pCFG = nullptr;

  bool bFinish = false;
  bool bRewardGet = false;

  SManualQuest() {}

  void fromData(const ManualQuest& rItem);
  void toData(ManualQuest* pItem);
};
typedef vector<SManualQuest> TVecManualQuest;
struct SManualSubItem
{
  DWORD dwID = 0;
  DWORD dwStoreID = 0;
  EManualStatus eStatus = EMANUALSTATUS_MIN;
  bool bUnlock = false;
  bool bStore = false;
  ItemData oItem;

  DWORD arrParams[MAX_MANUAL_PARAM] = {0};
  string arrDataParams[MAX_MANUAL_PARAM];
  TVecManualQuest vecQuests;

  SManualSubItem() {}

  void fromData(const ManualSubItem& rItem);
  void toData(ManualSubItem* pItem);

  SManualQuest* getManualQuest(DWORD dwID);
  bool hasManualQuest() { return vecQuests.empty() == false; }
  bool checkPreQuest(const TSetDWORD& setIDs);

  bool setParam(DWORD dwIndex, DWORD dwValue);
  DWORD getParam(DWORD dwIndex) const;

  bool setDataParam(DWORD dwIndex, const string& data);
  const string& getDataParam(DWORD dwIndex) const;
};
typedef vector<SManualSubItem> TVecManualSubItem;
struct SManualItem
{
  DWORD dwVersion = 0;
  EManualType eType = EMANUALTYPE_MIN;

  TVecManualSubItem vecSubItems;
  TVecManualQuest vecQuests;

  SManualItem() {}

  void fromData(const ManualItem& rItem);
  void toData(ManualItem* pItem, bool bClient);
  void toData(ManualVersion* pVersion);

  SManualSubItem* getSubItem(DWORD dwID);
  SManualQuest* getManualQuest(DWORD dwID);
  bool isFinishAll(EManualType eType, DWORD dwID);
  bool hasManualQuest() { return vecQuests.empty() == false; }
  bool checkPreQuest(const TSetDWORD& setIDs);
  bool delSubItem(DWORD dwID);
  bool addSubItem(DWORD dwID, EManualStatus eStatus);
};
typedef vector<SManualItem> TVecManualItem;
struct SManualGroup
{
  DWORD dwID = 0;

  void fromData(const ManualGroup& rGroup);
  void toData(ManualGroup* pGroup);
};
typedef map<DWORD, SManualGroup> TMapManualGroup;
struct SManualData
{
  DWORD dwVersion = 0;

  DWORD dwPoint = 0;
  DWORD dwLevel = 0;
  DWORD dwSkillPoint = 0;
  DWORD dwExchangeTime = 0;

  TVecManualItem vecItems;
  TMapManualGroup mapGroups;

  SManualData() {}

  SManualItem* getManualItem(EManualType eType);
};

// unsolved photo
struct SUnsovledUserPhoto
{
  QWORD qwCharID = 0;
  string strName;

  map<DWORD, UnsolvedPhoto> mapPhotos;

  bool fromData(const UnsolvedUserPhoto& rData);
  bool toData(UnsolvedUserPhoto* pData);
};
typedef map<QWORD, SUnsovledUserPhoto> TMapUnsolvedPhoto;

// manual
class SceneUser;
class Manual
{
  public:
    Manual(SceneUser* pUser);
    ~Manual();

    bool load(const BlobManual& oAcc, const BlobManual& oChar, const BlobUnsolvedPhoto& oPhoto);
    bool save(BlobManual* pAcc, BlobManual* pChar, BlobUnsolvedPhoto* pPhoto);
    void reload();
    void initDefaultItem();
    void initDefaultData();

    void queryManualData(EManualType eType);
    void queryUnsolvedPhoto();
    void sendManualData();
    DWORD getManualLv() const { return m_stData.dwLevel; }
    DWORD getManualPoint() const { return m_stData.dwPoint; }
    const SManualLvCFG* getManualLvCFG() const { return m_pLvCFG; }

    bool checkSkillPoint(DWORD dwPoint) const { return m_stCharData.dwSkillPoint >= dwPoint; }
    DWORD getSkillPoint() const { return m_stCharData.dwSkillPoint; }
    bool subSkillPoint(DWORD dwPoint);
    void addSkillPoint(DWORD dwPoint, bool bNotify = true);

    void onEnterMap(DWORD dwMapID, bool bRealOpen = true);
    void onKillMonster(DWORD dwMonsterID, bool bRealKill = true);
    void onKillProcess(DWORD dwMonsterID);
    void onItemAdd(DWORD dwItemID, bool bRealAdd = true, bool bNotify = true, bool bUsed = false, ESource source = ESOURCE_NORMAL);
    void onPhoto(DWORD dwMonsterID);
    void onPhotoProcess(DWORD dwMonsterID);
    void onScenery(DWORD dwSceneryID, bool bRealAdd = true);
    void onSceneryUpload(DWORD dwSceneryID, DWORD dwAngleZ, DWORD dwTime);
    void onMonthCardLock();
    void onPetHatch(DWORD dwPetID, bool bRealAdd);

    EManualStatus getMapStatus(DWORD dwMapID);
    EManualStatus getItemStatus(DWORD dwItemID);
    EManualStatus getFashionStatus(DWORD dwFashionID);
    EManualStatus getMonsterStatus(DWORD dwMonsterID);
    EManualStatus getCollectionStatus(DWORD dwCollectID);

    bool getMonsterLock(DWORD dwMonsterID);

    //bool getAchieveReward(DWORD dwID);
    bool unlock(EManualType eType, DWORD dwID);
    bool getQuestReward(DWORD dwQuestID);
    bool removeItem(EManualType eType, DWORD dwID);

    DWORD getNumByStatus(EManualType eType, EManualStatus eStatus, ENpcType eMonster = ENPCTYPE_MIN, EItemType eItemType = EITEMTYPE_MIN);
    DWORD getMonsterPhotoNum();
    DWORD getSceneryUnlockCount(DWORD dwMapID);
    DWORD getPetAdventureScore(EManualType eType);

    SManualItem* getManualItem(EManualType eType) { return m_stData.getManualItem(eType); }

    // temp patch1
    void patch1();
    void patch2();
    void patch3();

    bool storeItem(EManualType eType, const string& guid);
    bool getItem(EManualType eType, DWORD dwItemID);
    DWORD getMonthCard();
    bool delManualItem(EManualType eType, DWORD dwItemID);
    bool addManualItem(EManualType eType, DWORD dwItemID, EManualStatus eStatus, bool bCheck = false);
    DWORD getUnlockNum(EManualType eType, DWORD feature);
    DWORD getConstantUnlockNum(EManualType eType, TSetDWORD setID);

    bool groupAction(EGroupAction eAction, DWORD dwGroupID);
    //void collectAttr(TVecAttrSvrs& attrs);
    void collectAttr();
  private:
    void sendAllVersion();
    void sendPoint();
  public:
    void sendSkillPoint();
  private:
    void sendLevel();
    void refreshGroup();

    //void onPointChange(bool bNotify = true);

    //bool checkAchieve(const SAchieveCFG& rCFG);
    bool acceptQuest(EManualType eType, DWORD dwID, SManualItem* pItem, SManualSubItem* pSubItem);
    bool checkLock(EManualLockMethod eMethod, bool bRealAdd, bool bKill, bool bUsed, ESource source);
  public:
    bool unsolvedPhoto(const UpdateSolvedPhotoManualCmd& cmd);
    void addPoint(DWORD dwPoint, EManualType eType, DWORD dwID, bool bNotify = true);
  private:
    void levelup();

    void version_update();
    void version_1();
    void version_2();
    void version_3();
  public:
    void calcSkillPoint(bool bSync = false);
  public:
    bool addAttributes(EManualType eType);  //GM√¸¡Ó
  public:
    bool leveldown(DWORD dwLevel);
  public:
    void addLotteryFashion();
  private:
    SceneUser* m_pUser = nullptr;
    const SManualLvCFG* m_pLvCFG = nullptr;

    SManualData m_stData;
    SManualData m_stCharData;

    TMapUnsolvedPhoto m_mapUser2Photo;
};

