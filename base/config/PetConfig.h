#pragma once

#include "xTools.h"
#include "xSingleton.h"
#include "ScenePet.pb.h"
#include "TableManager.h"
#include "NpcConfig.h"
#include "SceneItem.pb.h"

using namespace Cmd;
using std::map;

const DWORD PET_WORK_SPACE_CAPRA = 3;

enum EPetAction
{
  EPETACTION_MIN = 0,
  EPETACTION_PET_BORN,
  EPETACTION_PET_IDLE,
  EPETACTION_PET_TOUCH,
  EPETACTION_PET_FEED,
  EPETACTION_PET_BASELVUP,
  EPETACTION_PET_FRIENDLVUP,
  EPETACTION_PET_DEAD,

  EPETACTION_OWNER_BASELVUP,
  EPETACTION_OWNER_DEAD,
  EPETACTION_OWNER_LOGIN,
  EPETACTION_OWNER_HPLESS,
  EPETACTION_OWNER_KILLMVP,
  EPETACTION_OWNER_KILLMINI,
  // 攻击指定魔物
  EPETACTION_OWNER_CHAT,
  EPETACTION_OWNER_REFINE_SUCCESS,
  EPETACTION_OWNER_REFINE_FAIL,
  EPETACTION_OWNER_PRODUCE_HEAD,
  EPETACTION_OWNER_ENTERGUILD,
  EPETACTION_OWNER_HATCH,
  EPETACTION_OWNER_TOUCH,
  EPETACTION_OWNER_FEED,
  EPETACTION_OWNER_NO_FEED,
  EPETACTION_OWNER_RESTORE,
  EPETACTION_OWNER_GETGIFT,
  EPETACTION_OWNER_HAND,
  EPETACTION_OWNER_EQUIP,
  EPETACTION_OWNER_SENDGIFT,
  EPETACTION_MAX,
};
struct SPetActionCFG : public SBaseCFG
{
  DWORD dwRate = 0;
  DWORD dwCD = 0;

  EPetAction eAction = EPETACTION_MIN;

  TSetDWORD setExpressionIDs;
  TSetDWORD setTalkIDs;
  TSetDWORD setActionIDs;

  SPetActionCFG() { setExpressionIDs.clear(); setTalkIDs.clear(); }
};
typedef map<EPetAction, SPetActionCFG> TMapPetActionCFG;

// 宠物捕捉配置
struct SCatchPetCFG
{
  DWORD dwMonsterID = 0;/*key, 场景怪物ID*/

  DWORD dwCatchNpcID = 0;
  DWORD dwDefaultCatchValue = 0;
  DWORD dwItemAddCatchValue = 0;
  DWORD dwFailDecCatchValue = 0;
  DWORD dwPetID = 0; /*对应宠物id*/
  DWORD dwGiftItemID = 0;/*捕捉赠送道具id*/

  TSetDWORD setAddDialogs;
  TSetDWORD setTryFailDialogs;
  TSetDWORD setValueFailDialogs;
  TSetDWORD setEndDialogs;
  TSetDWORD setIgnoreDialogs;
};
typedef map<DWORD, SCatchPetCFG> TMapCatchPetCFG;

struct SPetEquipCondition
{
  DWORD dwFriendLv = 0;
};

struct SPetCFG : public SBaseCFG
{
  DWORD dwID = 0; /*宠物id*/
  //DWORD dwMonsterID = 0; /*对应场景'怪物'id*/
  //float fFlowDis = 0;
  DWORD dwEggID = 0; /*宠物蛋道具id*/
  DWORD dwWorkSkillID = 0;

  string strName;

  TSetDWORD setCommonSkills;
  TSetDWORD setEquipIDs;
  TSetDWORD setHobbyItems;

  map<DWORD, DWORD> mapRandomSkill2MaxLv;
  map<DWORD, float> mapAreaAdventureValue;
  TMapPetActionCFG mapActionCFG;

  bool bCanEquip = false;
  SPetEquipCondition stEquipCondtion;

  void getRandomSkill(TSetDWORD& skills) const;
  const SPetActionCFG* getActionCFG(EPetAction eAction) const;
  bool canEquip(DWORD dwID) const { return setEquipIDs.find(dwID) != setEquipIDs.end(); }
  void getFullSkill(TSetDWORD& skills) const;
  float getAreaAdventureValue(DWORD dwArea) const;
  bool isMaxSkill(const TSetDWORD& setIDs) const;
};
typedef map<DWORD, SPetCFG> TMapPetCFG;

// base lv
struct SPetBaseLvCFG : public SBaseCFG
{
  DWORD dwLv = 0;
  QWORD qwExp = 0;
  QWORD qwNewTotExp = 0;
  QWORD qwNewCurExp = 0;

  SPetBaseLvCFG() { dwLv = qwExp = qwNewTotExp = qwNewCurExp = 0; }
};
typedef map<DWORD, SPetBaseLvCFG> TMapPetBaseLvCFG;

// friend lv
struct SPetFriendLvCFG : public SBaseCFG
{
  DWORD dwPetID = 0;
  DWORD dwLv = 0;
  QWORD qwExp = 0;
  DWORD dwRewardID = 0;

  TSetDWORD setUnlockEquipIDs;
  TSetDWORD setUnlockBodyIDs;

  bool bCanAdventure = false;

  SPetFriendLvCFG() { dwPetID = dwLv = qwExp = 0; bCanAdventure = false; }

  bool isUnlockEquip(DWORD dwID) const { return setUnlockEquipIDs.find(dwID) != setUnlockEquipIDs.end(); }
  bool isUnlockBody(DWORD dwID) const { return setUnlockBodyIDs.find(dwID) != setUnlockBodyIDs.end(); }
};
typedef map<DWORD, SPetFriendLvCFG> TMapFriendLvCFG;
typedef map<DWORD, TMapFriendLvCFG> TMapPetFriendCFG;

// adventure cond
enum EPetAdventureCond
{
  EPETADVENTURECOND_MIN = 0,
  EPETADVENTURECOND_RACE = 1,
  EPETADVENTURECOND_NATURE = 2,
  EPETADVENTURECOND_FRIENDLV = 3,
  EPETADVENTURECOND_SKILL = 4,
  EPETADVENTURECOND_PETID = 5,
  EPETADVENTURECOND_MAX,
};
struct SPetAdventureCondCFG : public SBaseCFG
{
  DWORD dwID = 0;
  EPetAdventureCond eCond = EPETADVENTURECOND_MIN;
  TVecQWORD vecParams;

  bool checkCond(const EggData& rData) const;
};
typedef map<DWORD, SPetAdventureCondCFG> TMapPetAdventureCondCFG;

// adventure
enum EPetAdventureType
{
  EPETADVENTURETYPE_MIN = 0,
  EPETADVENTURETYPE_ONCE = 1,
  EPETADVENTURETYPE_TIME = 2,
  EPETADVETTURETYPE_MAX = 3,
};
struct SPetAdventureCFG : public SBaseCFG
{
  DWORD dwID = 0;
  DWORD dwReqLv = 0;
  DWORD dwNeedTime = 0;
  DWORD dwBattleTimeReq = 0;
  DWORD dwAreaReq = 0;
  DWORD dwMaxPet = 0;
  DWORD dwRareReward = 0;
  DWORD dwTime = 0;
  DWORD dwUnlockArea = 0;
  DWORD dwPetBaseExp = 0;

  TVecItemInfo vecItemReq;

  EQualityType eQuality = EQUALITYTYPE_MIN;
  EPetAdventureType eType = EPETADVENTURETYPE_MIN;

  TSetDWORD setCondIDs;
  TSetDWORD setRewardIDs;

  TVecDWORD vecMonsterIDs;
};
typedef vector<SPetAdventureCFG> TVecPetAdventureCFG;
typedef map<DWORD, SPetAdventureCFG> TMapPetAdventureCFG;
typedef map<DWORD, TVecPetAdventureCFG> TMapAreaPetAdventureCFG;
typedef map<EQualityType, DWORD> TMapPetAdventureQuality;

// work
enum EPetWorkCond
{
  EPETWORKCOND_MIN = 0,
  EPETWORKCOND_MENU = 1,
  EPETWORKCOND_MAX,
};
struct SPetWorkCond
{
  EPetWorkCond eCond = EPETWORKCOND_MIN;
  TVecDWORD vecParams;
};
struct SPetWorkCFG : public SBaseCFG
{
  DWORD dwID = 0;
  DWORD dwPetLvReq = 0;
  DWORD dwFrequency = 0;
  DWORD dwMaxReward = 0;
  DWORD dwActID = 0;

  TSetDWORD setForbidPets;
  //TSetDWORD setOpenMenuIDs;
  //TSetDWORD setUnlockMenuIDs;
  TVecDWORD vecRewardIDs;

  SPetWorkCond stOpenCond;
  SPetWorkCond stUnlockCond;

  bool isForbid(DWORD dwPetID) const { return setForbidPets.find(dwPetID) != setForbidPets.end(); }
};
typedef map<DWORD, SPetWorkCFG> TMapPetWorkCFG;

struct SPetComposeMaterial
{
  DWORD dwPetID = 0;
  DWORD dwBaseLv = 0;
  DWORD dwFriendLv = 0;
};

struct SPetComposeCFG : public SBaseCFG
{
  DWORD dwPetID = 0;
  DWORD dwZenyCost = 0;
  vector<SPetComposeMaterial> vecPetMaterials;
};
typedef map<DWORD, SPetComposeCFG> TMapPetComposeCFG;

struct SPetAvatarCFG
{
  DWORD dwBodyID = 0;
  map<EEquipPos, TSetDWORD> mapPos2ItemIDs;
  map<EEquipPos, TSetDWORD> mapPos2RandomList;
};
typedef map<DWORD, SPetAvatarCFG> TMapPetAvatarCFG;

// pet config
class PetConfig : public xSingleton<PetConfig>
{
  friend class xSingleton<PetConfig>;
  private:
    PetConfig();
  public:
    virtual ~PetConfig();

    bool loadConfig();
    bool checkConfig();

    bool loadPetConfig();
    bool loadCatchConfig();
    bool loadBaseLvConfig();
    bool loadFriendLvConfig();
    bool loadPetBehaviorConfig();
    bool loadPetAdventureConfig();
    bool loadPetAdventureCondConfig();
    bool loadPetWorkConfig();
    bool loadPetComposeConfig();
    bool loadPetAvatarConfig();
  public:
    const SCatchPetCFG* getCatchCFGByMonster(DWORD monsterid) const;/*根据场景怪物id*/
    //const SCatchPetCFG* getCatchCFGByNpc(DWORD npcid) const;/*根据交互npcid*/
    const SPetCFG* getPetCFG(DWORD petid) const;

    DWORD getPetIDByItem(DWORD itemid) const;
    DWORD getItemIDByPet(DWORD petid) const;
    DWORD getComPetValueByItem(DWORD itemid) const { auto it = m_mapHobbyItem2ComValue.find(itemid); return it != m_mapHobbyItem2ComValue.end() ? it->second : 0; }

    const SPetBaseLvCFG* getPetBaseLvCFG(DWORD dwLv) const;
    const SPetFriendLvCFG* getFriendLvCFG(DWORD dwPetID, DWORD dwLv) const;
    const SPetAdventureCFG* getAdventureCFG(DWORD dwID) const;
    const SPetAdventureCondCFG* getAdventureCondCFG(DWORD dwID) const;
    const SPetWorkCFG* getWorkCFG(DWORD dwID) const;
    const SPetComposeCFG* getComposeCFG(DWORD dwID) const;
    const SPetAvatarCFG* getAvatarCFGByBody(DWORD dwBody) const;

    const TMapPetCFG& getPetCFGList() const { return m_mapPetCFG; }
    const TMapPetWorkCFG& getWorkCFGList() const { return m_mapPetWorkCFG; }

    bool collectEnableAdventure(TVecPetAdventureCFG& vecCFG, DWORD dwMaxArea);
    bool isUnlockEquip(DWORD dwPetID, DWORD dwID) const;
    bool isUnlockBody(DWORD dwPetID, DWORD dwID) const;
    bool isComposePet(DWORD petid) const { return m_mapPetComposeCFG.find(petid) != m_mapPetComposeCFG.end(); }
  private:
    EPetAdventureCond getPetAdventureCond(const string& str) const;
    EPetWorkCond getPetWorkCond(const string& str) const;
  private:
    TMapCatchPetCFG m_mapCatchPetCFG;
    map<DWORD, DWORD> m_mapItem2PetID;/*方便查询*/
    //map<DWORD, DWORD> m_mapCatchNpc2Monster;/*方便查询*/

    TMapPetCFG m_mapPetCFG;

    TMapPetBaseLvCFG m_mapBaseLvCFG;
    TMapPetFriendCFG m_mapFriendLvCFG;
    TMapPetAdventureCFG m_mapAdventureCFG;
    TMapAreaPetAdventureCFG m_mapAreaAdventureCFG;
    TMapPetAdventureCondCFG m_mapAdventureCondCFG;
    TMapPetWorkCFG m_mapPetWorkCFG;
    TMapPetComposeCFG m_mapPetComposeCFG;
    TMapPetAvatarCFG m_mapPetAvatarCFG;
    map<DWORD, DWORD> m_mapHobbyItem2ComValue; /*道具->融合宠物友情度*/
};

