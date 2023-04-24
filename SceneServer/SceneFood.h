#pragma  once

#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "SceneFood.pb.h"
#include <map>
#include <vector>
#include <queue>

using namespace std;
using namespace Cmd;

#define   COOKING_DUTAION 5.0
#define   END_DURATION 3

class SceneUser;
class SceneNpc;
class FoodNpc;

struct SFoodSubData
{
  EFoodStatus status = EFOODSTATUS_MIN;
  DWORD dwItemid = 0;
  DWORD dwExp = 0;
  DWORD dwLevel = 0;

  void fromData(const Cmd::FoodSubData& rData);
  void toData(Cmd::FoodSubData* pData);
};

typedef std::vector<SFoodSubData> TVecSFoodSubData;

struct SFoodManualData
{
  EFoodDataType eType = EFOODDATATYPE_MIN;
  TVecSFoodSubData vecSubData;
  void fromData(const Cmd::FoodManualData& rData);
  void toData(Cmd::FoodManualData* pData);
  SFoodSubData* getSubData(DWORD dwItemId);
    
};
typedef map<EFoodDataType, SFoodManualData> TMapFoodManualData;

struct SFoodItemInfo
{
  DWORD dwItemId = 0;
  DWORD dwInvalidTime = 0;
  TVecAttrSvrs vecAttr;

  void fromData(const Cmd::FoodItemInfo& rData);
  void toData(Cmd::FoodItemInfo* pData);
  void toData(Cmd::FoodItemInfo2* pData);
};
typedef std::vector<SFoodItemInfo> TVecFoodItemInfo;


struct SAnimation
{  
  DWORD m_dwNtfTime = 0;
  Cmd::CookStateNtf m_msg;
};

struct SRecipeInfo
{
  DWORD m_dwRecipeId = 0;
  DWORD m_dwMatchCnt = 0;
  DWORD m_dwProductId = 0;
  DWORD m_dwCookSuccessRate = 0;
  DWORD m_dwAvgMaterialLv = 0;
};

class SceneFood
{
public:
  SceneFood(SceneUser* pUser);
  ~SceneFood();
  bool load(const BlobFood& acc_data, const BlobFood& char_data);
  bool save(BlobFood* acc_data, BlobFood* char_data);
  //one sec tick
  void timer(DWORD curSec);
  void sendFoodData();

  void addCookerExp(DWORD exp);
  DWORD getCookerExp() { return m_dwCookerExp; }
  void addTasterExp(DWORD exp);  
  DWORD getTasterExp() { return m_dwTasterExp; }
  bool cookerLvUp();
  DWORD getCookerLv() { return m_dwCookerLv; }
  bool tasterLvUp();
  DWORD getTasterLv() { return m_dwTasterLv; } 
  //增加食材烹饪熟练度经验,增加料理烹饪熟练度经验,增加品尝数量度经验
  void addFoodDataExp(EFoodDataType type, DWORD dwItemId, DWORD exp = 1);  
  DWORD getFoodDataLevel(EFoodDataType type, DWORD dwItemId);
  void onFoodDataLevelUp(EFoodDataType type, SFoodSubData* pSubData);
  bool unlockRecipe(DWORD recipeId);
  bool hasUnlockedRecipe(DWORD recipeId) { return m_setRecipe.find(recipeId) != m_setRecipe.end(); }
  void updateFoodData(EFoodDataType type, SFoodSubData* pSubData);

  //制作
  bool prepareCook(Cmd::PrepareCook& rev);
  bool selectCookType(Cmd::ECookType type);
  bool startCook(Cmd::StartCook& rev); 
  void breakCooking();
  void mutableCookState(Cmd::CookStateMsg* pState);

  //吃
  bool putFood(Cmd::PutFood& rev);
  bool editFoodPower(Cmd::EditFoodPower& rev);
  void queryFoodNpcInfo(QueryFoodNpcInfo& rev);
  bool startEat(Cmd::StartEat& rev);
  void breakEating();
  bool stopEat(FoodNpc* pNpc, bool eatSuccess);
  void onEatFood(FoodNpc* pNpc);
  void onPetEatFood(FoodNpc* pNpc, SceneNpc* pPet);

  void clickManual(Cmd::EFoodDataType type, DWORD itemId);  
  DWORD getCookSuccessRate(DWORD dwFoodId, DWORD dwCookHard, DWORD avgMeterialLv);
  bool calcCookSuccess(DWORD dwSuccessRate);
  bool openFunction();
  void setBagSlot(DWORD slot) { m_dwBagSlot = slot; }
  //获取增加的背包格子数
  DWORD getBagSlot(EPackType packType);
  //获取饱腹度
  DWORD getSatiety();
  void addSatiety(DWORD dwSatiety);
  bool checkSatiety();
  void clearSatiety();
  //获取饱腹度上限
  DWORD getLimitSatiety();
  //获取当前吃的料理数量
  DWORD getEatedFoodNum();
  void updateEatedFoodNum();
  //获取可以吃的料理上限
  DWORD getLimitFood() { return m_dwLimitFood; }
  void updateEatedFood(DWORD curSec);
  DWORD getSaveHP() { return m_dwSaveHp; }
  DWORD getSaveSP() { return m_dwSaveSp; }
  void refreshSlim(DWORD flag);
  //void collectAttr(TVecAttrSvrs& attr);
  void collectAttr();
  bool checkCookerLvExp(DWORD lv);
  bool checkTasterLvExp(DWORD lv);
  DWORD getLvUpCnt(Cmd::EFoodDataType type, DWORD lv);
  void refreshSatiety();
  void onLeaveTeam();
  void addJoyOnEat(DWORD dwFoodId, DWORD dwFoodNum);
  DWORD getUnlockedFood() const;

  bool checkFoodLimitNum(DWORD dwItemid);
  void addFoodLimitNum(DWORD dwItemid);
  void checkResetLimitNum();
private:
  void mutableCookStateInner(Cmd::CookStateMsg* pState);
  void ntfCookState(DWORD dwDelay);
  bool onCookComplte();
  bool onCookOver();
  bool isCookTypeValid(Cmd::ECookType type);
  void reset();
  DWORD calcAddValue(DWORD dwLv, DWORD dwBase, DWORD dwRate);
  void setSaveHPSP(DWORD dwHP, DWORD dwSP);
  void onAddFoodItem(SFoodItemInfo& rItemInfo);
  void onDelFoodItem(SFoodItemInfo& rItemInfo);
  void checkHP();
  void checkSP();
  void syncAnimaion(DWORD curSec);
  void clearQueue()
  {
    std::queue<SAnimation> empty;
    std::swap(m_animationQueue, empty);
    m_curSyncStateMsg.Clear();
  }
  void addLastCooked(DWORD dwProductId, DWORD dwNum);
  void prepareAttr();
  void onPutFood(FoodNpc* pNpc);
  void resetFoodBook();
  void addMatchedRecipe(DWORD dwRecipe, DWORD dwMatchCount, DWORD dwProductId);
  bool campDWORDVector(TVecDWORD& vector1, TVecDWORD& vector2);
  bool checkCookRecipe(Cmd::ECookType dwCookType, const TSetDWORD& setClientRecipes);
private:
  SceneUser* m_pUser = nullptr;
  //存储
  DWORD m_dwCookerExp = 0;      //厨师等级
  DWORD m_dwCookerLv = 0;       //厨师经验
  DWORD m_dwTasterExp = 0;      //美食家经验
  DWORD m_dwTasterLv = 0;       //美食家等级  
  TMapFoodManualData m_mapFoodManualData;
  TVecFoodItemInfo m_vecFoodItemInfo;   //正在生效的料理
  DWORD m_dwLimitFood = 0;      //正在生效的料理上限
  // f04280 饱腹度概念已废弃
  DWORD m_dwSatiety = 0;        //吃的饱腹度
  DWORD m_dwLimitSatiety = 0;   //饱腹度上限
  TVecDWORD m_vecLastCooked;    //最近制作的料理

  DWORD m_dwSaveHp = 0;
  DWORD m_dwSaveSp = 0;
  std::set<DWORD> m_setRecipe;    //解锁的配方
  DWORD m_dwBagSlot = 0;          //
  
  //temp
  Cmd::ECookState m_state = ECOOKSTATE_NONE;
  Cmd::ECookType m_cookType = ECOOKTYPE_MIN;
  DWORD m_dwCookProgress = 0;   //制作进度
  bool m_bCookSuccess = false;
  DWORD m_dwCookCompleteTime = 0;
  DWORD m_dwCookOverTime = 0;
  QWORD m_qwEatingFoodNpc = 0;      //正在吃的料理npc
  DWORD m_dwEatingFoodNum = 0;      //正在吃的料理数量
  typedef std::pair<DWORD/*recipe id*/, SRecipeInfo/*match info*/> TRecipePair;
  std::vector<TRecipePair> m_vecRecipes;  //匹配到的食谱列表
  typedef std::pair<DWORD/*item id*/, ItemInfo> TMaterialInfoPair;
  std::vector<TMaterialInfoPair> m_vecMaterialInfo; //匹配使用的食材
  typedef std::pair<DWORD/*item id*/, DWORD/*item count*/> TItemCountPair;
  std::vector<TItemCountPair> m_vecItemCount; //记录每个食材的数量
  std::vector<TItemCountPair> m_vecProductInfo;
  QWORD m_qwSelectCD = 0;     //选择炊具cd时间
  bool m_bSkipAnimation = false;

  std::queue<SAnimation> m_animationQueue;
  Cmd::CookStateNtf m_noneStateMsg;
  Cmd::CookStateMsg m_curSyncStateMsg;
  TSetDWORD m_setCookLvAttr;
  TSetDWORD m_setTasteLvAttr;

  map<DWORD, FoodLimitInfo> m_mapFoodlimit;
};
