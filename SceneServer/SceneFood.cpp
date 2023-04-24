#include "SceneFood.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "MiscConfig.h"
#include "RecipeConfig.h"
#include "SceneUser.h"
#include "FoodConfig.h"
#include "TableManager.h"
#include "MsgManager.h"
#include "Menu.h"

/************************************************************************/
/* SFoodSubData                                                                     */
/************************************************************************/
void SFoodSubData::fromData(const FoodSubData& rData)
{
  status = rData.status();
  dwItemid = rData.itemid();
  dwExp = rData.exp();
  dwLevel = rData.level();
}

void SFoodSubData::toData(Cmd::FoodSubData* pData)
{
  if (pData == nullptr)
    return ;
  pData->set_status(status);
  pData->set_exp(dwExp);
  pData->set_itemid(dwItemid);
  pData->set_level(dwLevel);
}

/************************************************************************/
/* SFoodManualData                                                                     */
/************************************************************************/
void SFoodManualData::fromData(const FoodManualData& rData)
{
  eType = rData.type();
  
  for (int i = 0; i < rData.datas_size(); ++i)
  {
    SFoodSubData subData;
    subData.fromData(rData.datas(i));
    vecSubData.push_back(subData);
  }
}

void SFoodManualData::toData(FoodManualData* pData)
{
  if (pData == nullptr)
    return ;
  pData->set_type(eType);

  for (auto& v : vecSubData)
    v.toData(pData->add_datas());
  return ;
}

SFoodSubData* SFoodManualData::getSubData(DWORD dwItemId)
{
  if (vecSubData.empty())
    return nullptr;
  auto it = std::find_if(vecSubData.begin(), vecSubData.end(), [dwItemId](SFoodSubData& a) { return a.dwItemid == dwItemId; });
  if (it == vecSubData.end())
    return nullptr;
  return &(*it);
}

/************************************************************************/
/* SFoodItemInfo                                                                     */
/************************************************************************/
void SFoodItemInfo::fromData(const FoodItemInfo& rData)
{
  dwItemId = rData.itemid();
  dwInvalidTime = rData.invalid_time();
 
  for (int i = 0; i < rData.attrs_size(); ++i)
  {
    UserAttrSvr uAttr;
    uAttr.set_type(rData.attrs(i).type());
    uAttr.set_value(rData.attrs(i).value());
    vecAttr.push_back(uAttr);
  }
}

void SFoodItemInfo::toData(FoodItemInfo* pData)
{
  if (pData == nullptr)
    return ;

  pData->set_itemid(dwItemId);
  pData->set_invalid_time(dwInvalidTime);

  for (auto&v : vecAttr)
  {
    UserAttrSvr* pAttr = pData->add_attrs();
    if (!pAttr)
      continue;
    pAttr->set_type(v.type());
    pAttr->set_value(v.value());
  }
}

void SFoodItemInfo::toData(FoodItemInfo2* pData)
{
  if (pData == nullptr)
    return;

  pData->set_itemid(dwItemId);
  pData->set_invalid_time(dwInvalidTime);

}

/************************************************************************/
/*SceneFood                                                                      */
/************************************************************************/
SceneFood::SceneFood(SceneUser* pUser):m_pUser(pUser)
{
  m_noneStateMsg.set_charid(m_pUser->id);
  CookStateMsg* pState = m_noneStateMsg.mutable_state();
  if (pState)
  {
    pState->set_state(ECOOKSTATE_NONE);
  }
}

SceneFood::~SceneFood()
{
}

bool SceneFood::load(const BlobFood& acc_data, const BlobFood& char_data)
{
  m_dwCookerExp = acc_data.cookerexp();
  m_dwCookerLv = acc_data.cookerlv();
  m_dwTasterExp = acc_data.tasterexp();
  m_dwTasterLv = acc_data.tasterlv();
  m_dwSaveHp = char_data.savehp();
  m_dwSaveSp = char_data.savesp();
  m_dwSatiety = char_data.satiety();

  ////test 
  //if (m_dwTasterLv == 0)
  //  m_dwTasterLv = 1;
  //if (m_dwCookerLv == 0)
  //  m_dwCookerLv = 1;
  //
  for (int i = 0; i < acc_data.recipes_size(); ++i)
  {
    m_setRecipe.insert(acc_data.recipes(i));
  }
  
  for (int i = 0; i < acc_data.manualdata_size(); ++i)
  {
    SFoodManualData& rData = m_mapFoodManualData[acc_data.manualdata(i).type()];
    rData.fromData(acc_data.manualdata(i));    
  }

  m_vecFoodItemInfo.reserve(char_data.iteminfo_size());
  for (int i = 0; i < char_data.iteminfo_size(); ++i)
  {
    SFoodItemInfo itemInfo;
    itemInfo.fromData(char_data.iteminfo(i));
    onAddFoodItem(itemInfo);
  }

  m_vecLastCooked.reserve(char_data.last_cooked_ids_size());
  for (int i = 0; i < char_data.last_cooked_ids_size(); ++i)
  {
    m_vecLastCooked.push_back(char_data.last_cooked_ids(i));
  }
  
  for (int i = 0; i < char_data.limitinfo_size(); ++i)
  {
    FoodLimitInfo itemInfo = char_data.limitinfo(i);
    m_mapFoodlimit.emplace(itemInfo.itemid(), itemInfo);
  }

  prepareAttr();
    
  if (m_dwCookerLv)
  {
    const SCookerLevel* pCookerLvCfg = TableManager::getMe().getCookerLevelCFG(m_dwCookerLv);
    if (!pCookerLvCfg)
      return false;
    m_dwBagSlot = pCookerLvCfg->getRewardBagSlot();
  }
  
  if (m_dwTasterLv)
  {
    const STasterLevel* pCfg = TableManager::getMe().getTasterLevelCFG(m_dwTasterLv);
    if (!pCfg)
      return false;
    m_dwLimitSatiety = pCfg->getFullProgress();
    m_dwLimitFood = pCfg->getAddFoods();
  }
  else
  {//0级默认80点饱腹度
    m_dwLimitSatiety = MiscConfig::getMe().getFoodCfg().dwDefaultSatiey;
    m_dwLimitFood = MiscConfig::getMe().getFoodCfg().dwDefaultLimitFood;
  }

  resetFoodBook();

  XDBG << "[料理-加载] charid" << m_pUser->id << "厨师等级" << m_dwCookerLv << "美食家等级" << m_dwTasterLv << "包包格子" << m_dwBagSlot << "饱腹度" << m_dwSatiety << "msg acc:" << acc_data.ShortDebugString() << "char :" << char_data.ShortDebugString() << XEND;
  return true;
}

void SceneFood::prepareAttr()
{
  auto fun = [&](EFoodDataType type)
  {
    auto it = m_mapFoodManualData.find(type);
    if (it == m_mapFoodManualData.end())
      return;
    //*料理烹饪熟练度达到LvMax时，永久获得属性
    DWORD maxLv = MiscConfig::getMe().getFoodCfg().getMaxLv(type);
    for (auto &v : it->second.vecSubData)
    {     
      if (v.dwLevel == maxLv)
      {
        if (type == EFOODDATATYPE_FOODCOOK)
          m_setCookLvAttr.insert(v.dwItemid);
        else if (type == EFOODDATATYPE_FOODTASTE)
          m_setTasteLvAttr.insert(v.dwItemid);
      }
    }
  };  
  
  fun(EFOODDATATYPE_FOODCOOK);
  fun(EFOODDATATYPE_FOODTASTE);
}

bool SceneFood::save(Cmd::BlobFood* acc_data, Cmd::BlobFood* char_data)
{
  if (acc_data == nullptr || char_data == nullptr)
    return false;
  acc_data->set_cookerexp(m_dwCookerExp);
  acc_data->set_cookerlv(m_dwCookerLv);
  acc_data->set_tasterexp(m_dwTasterExp);
  acc_data->set_tasterlv(m_dwTasterLv);
  char_data->set_savehp(m_dwSaveHp);
  char_data->set_savesp(m_dwSaveSp);
  
  if (getSatiety())
    char_data->set_satiety(m_dwSatiety);

  for (auto&s : m_setRecipe)
  {
    acc_data->add_recipes(s);
  }

  for (auto &m : m_mapFoodManualData)
  {
    m.second.toData(acc_data->add_manualdata());
  }
  
  for (auto& v : m_vecFoodItemInfo)
  {
    v.toData(char_data->add_iteminfo());
  }
  
  for (auto& v : m_vecLastCooked)
  {
    char_data->add_last_cooked_ids(v);
  }

  for (auto& v : m_mapFoodlimit)
  {
    FoodLimitInfo* pInfo = char_data->add_limitinfo();
    if(pInfo != nullptr)
    {
      pInfo->set_itemid(v.second.itemid());
      pInfo->set_num(v.second.num());
      pInfo->set_time(v.second.time());
    }
  }
  
  XLOG << "[料理-保存] charid" << m_pUser->id << "厨师等级" << m_dwCookerLv << "美食家等级" << m_dwTasterLv << "包包格子" << m_dwBagSlot << "数据大小 acc :" << acc_data->ByteSize() << "char :" << char_data->ByteSize() << XEND;
  return true;
}

//one sec tick
void SceneFood::timer(DWORD curSec)
{
  if (m_state == ECOOKSTATE_COOKING)
  {
    if (curSec >= m_dwCookCompleteTime)
    {
      onCookComplte();
    }
    else
    {
      float f = (curSec - (m_dwCookCompleteTime - COOKING_DUTAION)) / COOKING_DUTAION;
      m_dwCookProgress = f * 100;
    }
  }
  if (m_state == ECOOKSTATE_PREPAREING && m_dwCookOverTime)
  {
    if (curSec >= m_dwCookOverTime)
      onCookOver();
  }
  updateEatedFood(curSec);
  
  //
  syncAnimaion(curSec);

  //等待场景属性计算结束
  if (curSec > m_pUser->m_dwEnterSceneTime + 20)
  {
    checkHP();
    checkSP();
  }
}

void SceneFood::sendFoodData()
{
  checkResetLimitNum();

  QueryFoodManualData cmd;
  cmd.set_cookerexp(m_dwCookerExp);
  cmd.set_cookerlv(m_dwCookerLv);
  cmd.set_tasterexp(m_dwTasterExp);
  cmd.set_tasterlv(m_dwTasterLv);

  
  for (auto &m : m_mapFoodManualData)
  {
    m.second.toData(cmd.add_items());
  }  
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  {
    FoodInfoNtf cmd;      
    for (auto &s : m_setRecipe)
    {
      cmd.add_recipeids(s);
    }    
    for (auto& v : m_vecLastCooked)
    {
      cmd.add_last_cooked_foods(v);
    }

    for (auto &v : m_vecFoodItemInfo)
    {
      Cmd::FoodItemInfo2* pInfo = cmd.add_eat_foods();
      v.toData(pInfo);
    }
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void SceneFood::addCookerExp(DWORD exp)
{
  if (!exp) return;
  DWORD newLv = m_dwCookerLv + 1;
  SCookerLevel* pNewCfg = const_cast<SCookerLevel*>(TableManager::getMe().getCookerLevelCFG(newLv));
  if (pNewCfg == nullptr)
    return;
  if (m_dwCookerExp >= pNewCfg->getNeedExp())
  {
    MsgManager::sendMsg(m_pUser->id, 3512);
    return;
  }
  m_dwCookerExp += exp;
  
  if (m_dwCookerExp > pNewCfg->getNeedExp())
    m_dwCookerExp = pNewCfg->getNeedExp();
  
  m_pUser->setDataMark(EUSERDATATYPE_COOKER_EXP);
  m_pUser->refreshDataAtonce();
  m_pUser->getQuest().acceptNewQuest();
  MsgManager::sendMsg(m_pUser->id, 3508, MsgParams(exp));
  XLOG << "[料理-经验]，增加厨师经验 charid" << m_pUser->id << "增加" << exp << "增加后经验" << m_dwCookerExp << "厨师等级" << m_dwCookerLv << XEND;
}

void SceneFood::addTasterExp(DWORD exp) 
{
  if (!exp) return;
  DWORD newLv = m_dwTasterLv + 1;
  const STasterLevel* pNewCfg = TableManager::getMe().getTasterLevelCFG(newLv);
  if (pNewCfg == nullptr)
    return;
  if (m_dwTasterExp >= pNewCfg->getNeedExp())
  {
    MsgManager::sendMsg(m_pUser->id, 3512);
    return;
  }
  m_dwTasterExp += exp;

  if (m_dwTasterExp > pNewCfg->getNeedExp())
    m_dwTasterExp = pNewCfg->getNeedExp();

  m_pUser->setDataMark(EUSERDATATYPE_TASTER_EXP);
  m_pUser->refreshDataAtonce();
  m_pUser->getQuest().acceptNewQuest();
  MsgManager::sendMsg(m_pUser->id, 3509, MsgParams(exp));
  XLOG << "[料理-经验]，增加美食家经验 charid" << m_pUser->id << "增加" << exp << "增加后经验" << m_dwTasterExp << "美食家等级" << m_dwTasterLv << XEND;
}

bool SceneFood::cookerLvUp()
{
  DWORD oldLv = m_dwCookerLv;
  DWORD newLv = m_dwCookerLv + 1;
  SCookerLevel* pNewCfg = const_cast<SCookerLevel*>(TableManager::getMe().getCookerLevelCFG(newLv));
  if (pNewCfg == nullptr)
    return false;
  if (m_dwCookerExp < pNewCfg->getNeedExp())
    return false;
  m_dwCookerExp -= pNewCfg->getNeedExp();
  m_dwCookerLv = newLv;

  //增加包包格子数
  setBagSlot(pNewCfg->getRewardBagSlot());
  m_pUser->getPackage().refreshMaxSlot();

  //称号
  m_pUser->getTitle().addTitle(pNewCfg->getTitle());

  m_pUser->setDataMark(EUSERDATATYPE_COOKER_EXP);
  m_pUser->setDataMark(EUSERDATATYPE_COOKER_LV);
  m_pUser->refreshDataAtonce();
  m_pUser->getQuest().acceptNewQuest();
  m_pUser->getAchieve().onCookerLvUp(m_dwCookerLv);
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_COOKLV);

  //
  const TVecDWORD& vecRecipe = pNewCfg->getUnlockRecipe();

  for (auto it = vecRecipe.begin(); it != vecRecipe.end(); ++it)
  {
    unlockRecipe(*it);
  }

  XLOG << "[料理-升级] 厨师等级升级 charid" << m_pUser->id << "原等级" << oldLv << "当前等级" << m_dwCookerLv << "当前经验" << m_dwCookerExp <<"额外解锁食谱个数" << vecRecipe.size() << XEND;
  return true;
}

bool SceneFood::tasterLvUp()
{
  DWORD oldLv = m_dwTasterLv;
  DWORD newLv = m_dwTasterLv + 1;
  const STasterLevel* pNewCfg = TableManager::getMe().getTasterLevelCFG(newLv);
  if (pNewCfg == nullptr)
    return false;
  if (m_dwTasterExp < pNewCfg->getNeedExp())
    return false;
  m_dwLimitSatiety = pNewCfg->getFullProgress();
  m_dwLimitFood = pNewCfg->getAddFoods();
  m_dwTasterExp -= pNewCfg->getNeedExp();
  m_dwTasterLv = newLv;

  //称号
  m_pUser->getTitle().addTitle(pNewCfg->getTitle());

  m_pUser->setDataMark(EUSERDATATYPE_TASTER_EXP);
  m_pUser->setDataMark(EUSERDATATYPE_TASTER_LV);
  m_pUser->refreshDataAtonce();
  m_pUser->getQuest().acceptNewQuest();
  m_pUser->getAchieve().onTasterLvUp(m_dwTasterLv);
  XLOG << "[料理-升级] 美食家等级升级 charid" << m_pUser->id << "原等级" << oldLv << "当前等级" << m_dwTasterLv << "当前经验" << m_dwTasterExp << "饱腹度上限" << m_dwLimitSatiety << "享用料理上限" << m_dwLimitFood << XEND;
  return true;
}

void SceneFood::addFoodDataExp(EFoodDataType type, DWORD dwItemId, DWORD exp/* = 1*/)
{
  const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(dwItemId);
  if (!pItemCfg)
    return;

  DWORD msgId = 0;
  switch (type)
  {
  case EFOODDATATYPE_MATERIAL:
    break;
  case EFOODDATATYPE_FOODCOOK:
    msgId = 3500;
    break;
  case EFOODDATATYPE_FOODTASTE:
    msgId = 3501;
    break;
  default:
    break;
  }

  //黑暗料理不弹提示
  if (MiscConfig::getMe().getFoodCfg().isDarkFood(dwItemId))
    msgId = 0;
  //“xxx烹饪熟练度+x”
  if (msgId)
    MsgManager::sendMsg(m_pUser->id, msgId, MsgParams(pItemCfg->strNameZh, exp));

  auto it = m_mapFoodManualData.find(type);
  if (it == m_mapFoodManualData.end())
  {
    SFoodManualData manualData;
    manualData.eType = type;
    it = m_mapFoodManualData.insert(std::make_pair(type, manualData)).first;
  }    
  SFoodSubData* pSubData = it->second.getSubData(dwItemId);
  if (pSubData == nullptr)
  {
    SFoodSubData subData;
    subData.status = EFOODSTATUS_ADD;
    subData.dwItemid = dwItemId;
    subData.dwExp = exp;
    //黑暗料理只进冒险手册不升级
    if (MiscConfig::getMe().getFoodCfg().isDarkFood(dwItemId))
      subData.dwLevel = 0;
    else
      subData.dwLevel = 1;

    it->second.vecSubData.push_back(subData);    
    pSubData = it->second.getSubData(dwItemId);
    if (pSubData == nullptr)
      return;
    /*
    //First add
    updateFoodData(type, &subData);
    */
    XLOG << "[料理-添加] charid" << m_pUser->id << "type" << type << "itemid" << subData.dwItemid << "level" << subData.dwLevel << "status" << subData.status << "exp" << subData.dwExp << XEND;
    //return;
  }

  //黑暗料理只进冒险手册不升级
  if (MiscConfig::getMe().getFoodCfg().isDarkFood(dwItemId))
  {
    updateFoodData(type, pSubData);
    return;
  }

  pSubData->dwExp += exp;
  DWORD needExp = MiscConfig::getMe().getFoodCfg().getExpPerLv(type);
  DWORD maxLv = MiscConfig::getMe().getFoodCfg().getMaxLv(type);
  //满级经验截断
  if (pSubData->dwLevel >= maxLv)
  {
    pSubData->dwExp = 0;
  }

  updateFoodData(type, pSubData);
    
  while (pSubData->dwExp >= needExp && pSubData->dwLevel < maxLv)
  {
    pSubData->dwLevel += 1;
    pSubData->dwExp -= needExp; 
    //level up
    onFoodDataLevelUp(type, pSubData);
  }  
}

void SceneFood::onFoodDataLevelUp(EFoodDataType type, SFoodSubData* pSubData)
{
  if (!pSubData)
    return;

  switch (type)
  {
  case EFOODDATATYPE_MATERIAL:
    m_pUser->getAchieve().onFoodMaterialLvUp(pSubData->dwItemid, pSubData->dwLevel);
    break;
  case EFOODDATATYPE_FOODCOOK:
  {
    m_pUser->getAchieve().onFoodCookLvUp(pSubData->dwItemid, pSubData->dwLevel);

    //*料理烹饪熟练度达到LvMax时，永久获得属性
    DWORD maxLv = MiscConfig::getMe().getFoodCfg().getMaxLv(EFOODDATATYPE_FOODCOOK);
    if (pSubData->dwLevel == maxLv)
    {
      m_setCookLvAttr.insert(pSubData->dwItemid);
      m_pUser->setCollectMark(ECOLLECTTYPE_FOOD);
    }       
    break;
  }
  case EFOODDATATYPE_FOODTASTE:
  {
    //*料理品尝熟练度达到LvMax时，永久获得属性
    DWORD maxLv = MiscConfig::getMe().getFoodCfg().getMaxLv(EFOODDATATYPE_FOODTASTE);
    if (pSubData->dwLevel == maxLv)
    {
      m_setTasteLvAttr.insert(pSubData->dwItemid);
      m_pUser->setCollectMark(ECOLLECTTYPE_FOOD);
    }
    break;
  }
  default:
    break;
  }
  updateFoodData(type, pSubData);
  XLOG << "[料理-升级] charid" << m_pUser->id << "type" << type << "itemid" << pSubData->dwItemid << "level" << pSubData->dwLevel <<"status" << pSubData->status << "exp" << pSubData->dwExp << XEND;
}

void SceneFood::updateFoodData(EFoodDataType type, SFoodSubData* pSubData)
{
  if (!pSubData)
    return;
  NewFoodDataNtf cmd;
  FoodManualData* pData = cmd.add_items();
  if (pData == nullptr)
    return;
  pData->set_type(type);
  pSubData->toData(pData->add_datas());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

DWORD SceneFood::getFoodDataLevel(EFoodDataType type, DWORD dwItemId)
{
  //默认是1级
  auto it = m_mapFoodManualData.find(type);
  if (it == m_mapFoodManualData.end())
    return 1;
  SFoodSubData* pSubData = it->second.getSubData(dwItemId);
  if (pSubData == nullptr)
    return 1;
  
  return pSubData->dwLevel;
}

bool SceneFood::unlockRecipe(DWORD recipeId)
{
  const SRecipeCFG* pRecipeCfg = RecipeConfig::getMe().getRecipeCFG(recipeId);
  if (pRecipeCfg == nullptr)
    return false;

  auto it = m_setRecipe.find(recipeId);
  if (it != m_setRecipe.end())
    return false;

  m_setRecipe.insert(recipeId);

  //给经验
  if (pRecipeCfg->m_dwUnlockExp)
    addCookerExp(pRecipeCfg->m_dwUnlockExp);

  UnlockRecipeNtf cmd;
  cmd.set_recipe(recipeId);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[料理-解锁食谱] charid" << m_pUser->id << "食谱id" << recipeId << XEND;

  return true;
}

bool SceneFood::prepareCook(Cmd::PrepareCook& rev)
{
  //打断吃料理
  breakEating();

  if (rev.start() == true)
  {
    if (m_state != ECOOKSTATE_NONE)
    {
      XERR << "[料理-制作] 失败，状态非法 charid" << m_pUser->id << "当前状态" << m_state << XEND;
      return false;
    }
    if (m_dwCookerLv == 0)
    {
      XERR << "[料理-制作] 失败，厨师等级不够 charid" << m_pUser->id << XEND;
      return false;
    }        
    m_state = ECOOKSTATE_PREPAREING;
    m_cookType = ECOOKTYPE_MIN;
    clearQueue();
    ntfCookState(0);

    XDBG << "[料理]打开料理界面 charid" << m_pUser->id << XEND;
  }
  else
  {
    breakCooking();
    XDBG << "[料理]关闭料理界面 charid" << m_pUser->id << XEND;
  }
  return true;
}

bool SceneFood::selectCookType(Cmd::ECookType type)
{
  if (m_state != ECOOKSTATE_PREPAREING)
    return false;
  if (isCookTypeValid(type) == false)
    return false;
  if (m_cookType == type)
    return true;    
  QWORD curMSec = xTime::getCurMSec(); 
  if (m_qwSelectCD > curMSec)
    return false;
  m_qwSelectCD = curMSec + 1500;
    
  m_cookType = type;
  XDBG << "[料理]选择炊具 charid" << m_pUser->id << "cooktype" << m_cookType << XEND;
  ntfCookState(0);
  return true;
}

bool SceneFood::checkCookRecipe(Cmd::ECookType dwCookType, const TSetDWORD& setClientRecipes)
{
  //由于以前只根据五个食材ID匹配一次食谱，现在改为多次匹配食谱
  //记录未匹配的食材ID和数量，根据当前未匹配的食材ID进行匹配，匹配到食谱后，更新未匹配的食材ID和数量，循环进行。（每次匹配前，可对比上一轮食材ID，若一致，则匹配的食谱也一致，可避免重复的匹配调用。）
  //匹配到的食谱对比客户端推送的食谱，不一致，则本次操作视为失败
  //记录剩余的(一般为不能单独制作任何料理)食材，制作时不要扣除。
  TVecDWORD vecCurMatchMaterialIds;
  DWORD dwMatchCnt = 0;
  std::vector<SRecipeCFG*> vecMatchRecipe;
  DWORD recipeCount = RecipeConfig::getMe().matchRecipe(dwCookType, m_vecItemCount, vecMatchRecipe);
  if (recipeCount == 0 || vecMatchRecipe.size() == 0)
  {
    XERR << "[料理-食谱匹配] 失败，找不到食谱 charid" << m_pUser->id <<"cooktype" << dwCookType << XEND;
    return false;
  }

  for(auto& it : vecMatchRecipe)
  {
    SRecipeCFG*pCfg = const_cast<SRecipeCFG*>(it);
    if (pCfg == nullptr)
      continue;
    
    const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(pCfg->m_dwProduct);
    if (pFoodCfg == nullptr)
      continue;

    vecCurMatchMaterialIds.clear();
    for (auto &m : m_vecItemCount)
    {
      vecCurMatchMaterialIds.push_back(m.first);
    }
    /*
    SRecipeCFG*pCfg = const_cast<SRecipeCFG*>(RecipeConfig::getMe().matchRecipe(dwCookType, m_vecItemCount));
    if (pCfg == nullptr)
    {
      break;
    }
    */
    //matchRecipeCount()会改变m_vecItemCount
    dwMatchCnt = pCfg->matchRecipeCount(m_vecItemCount);
    if (dwMatchCnt == 0)
    {
      continue;
      //XERR << "[料理-食谱匹配] 失败，匹配到的食谱制作材料不足 charid" << m_pUser->id << "clientrecipe count" << setClientRecipes.size() << "recipeid" << pCfg->m_dwId << "score" << pCfg->m_dwScore << "cooktype" << dwCookType << XEND;
      //return false;
    }
    
    if (setClientRecipes.count(pCfg->m_dwId) == 0)
    {
      XERR << "[料理-食谱匹配] 失败，食谱和客户端不一致 charid" << m_pUser->id << "clientrecipe count" << setClientRecipes.size() << "recipeid" << pCfg->m_dwId << "score" << pCfg->m_dwScore << "cooktype" << dwCookType << XEND;
      return false;
    }

    XLOG << "[料理-食谱匹配] 成功， charid" << m_pUser->id << "recipeid" << pCfg->m_dwId << "score" << pCfg->m_dwScore << "cooktype" << dwCookType << "match count" << dwMatchCnt<< XEND;

    TRecipePair pRecipe;
    pRecipe.first = pCfg->m_dwId;
    SRecipeInfo& recipeInfo = pRecipe.second;
    recipeInfo.m_dwRecipeId = pCfg->m_dwId;
    recipeInfo.m_dwMatchCnt = dwMatchCnt;
    recipeInfo.m_dwProductId = pCfg->m_dwProduct;
    //计算成功率
    DWORD dwAvgMaterialLv = 0;    //食材平均熟练度等级
    for (auto& v : vecCurMatchMaterialIds)
      dwAvgMaterialLv += getFoodDataLevel(EFOODDATATYPE_MATERIAL, v);
    dwAvgMaterialLv = dwAvgMaterialLv / vecCurMatchMaterialIds.size();
    recipeInfo.m_dwAvgMaterialLv = dwAvgMaterialLv;
    recipeInfo.m_dwCookSuccessRate = getCookSuccessRate(pFoodCfg->m_dwId, pFoodCfg->m_dwCookHard, dwAvgMaterialLv);

    m_vecRecipes.push_back(pRecipe);

    if(m_vecItemCount.empty())
    {
      break;
    }
  }

  if (m_vecRecipes.empty())
  {
    XERR << "[料理-食谱匹配] 失败，找不到食谱 charid" << m_pUser->id <<"cooktype" << dwCookType << XEND;
    return false;
  }

  //把剩余的料理 从食材里剔除
  for (auto& it : m_vecItemCount)
  {
    DWORD count = it.second;
    for (auto& p : m_vecMaterialInfo)
    {
      if (count == 0)
        break;
      if (p.first != it.first)
        continue;
      if (p.second.count() == 0)
        continue;
      
      if (p.second.count() > count)
      {
        p.second.set_count(p.second.count() - count);
        count = 0;
      }
      else
      {
        count -= p.second.count();
        p.second.set_count(0);
      }
    }
  }
  for (auto it = m_vecMaterialInfo.begin(); it != m_vecMaterialInfo.end();)
  {
    if(it->second.count() == 0)
    {
      it = m_vecMaterialInfo.erase(it);
      continue;
    }
    it++;
  }

  return true;
}

bool SceneFood::startCook(Cmd::StartCook& rev)
{
  if (m_state != ECOOKSTATE_PREPAREING)
    return false;
  if (isCookTypeValid(rev.cooktype()) && m_cookType != rev.cooktype())
  {
    selectCookType(rev.cooktype());
  }

  TSetDWORD setRecipes;
  for (int i = 0; i < rev.recipes_size(); i++)
  {
    setRecipes.insert(rev.recipes(i));
  }

  m_vecRecipes.clear();
  m_vecMaterialInfo.clear();
  m_vecItemCount.clear();
  TSetString setGuid;
  for (int i = 0; i < rev.material_size(); i++)
  {
    const BriefItemInfo& rInfo = rev.material(i);
    if (setGuid.count(rInfo.guid()) > 0)
    {
      XERR << "[料理-制作] 失败，食材里有重复物品 charid" << m_pUser->id <<"cooktype" << rev.cooktype() << "item guid" << rInfo.guid() << "material" << rev.ShortDebugString() << XEND;
      return false;
    }
    setGuid.insert(rInfo.guid());

    TMaterialInfoPair p;
    p.first = rInfo.itemid();
    ItemInfo& itemInfo = p.second;
    itemInfo.set_id(rInfo.itemid());
    itemInfo.set_count(rInfo.num());
    itemInfo.set_guid(rInfo.guid());
    m_vecMaterialInfo.push_back(p);

    TItemCountPair pItem;
    pItem.first = rInfo.itemid();
    pItem.second = rInfo.num();
    m_vecItemCount.push_back(pItem);
    XLOG << "[料理-制作]" << m_pUser->id << m_pUser->name << "选择食材" << rInfo.itemid() << rInfo.num() << "guid" << rInfo.guid() << XEND;
  }

  //check 食材
  BasePackage* pPkg = m_pUser->getPackage().getPackage(EPACKTYPE_FOOD);
  if (pPkg == nullptr)
    return false;
  for (auto &m : m_vecMaterialInfo)
  {
    if (!pPkg->checkItemCount(m.second.guid(), m.second.count()))
    {
      breakCooking();
      return false;
    }
  }

  //check 食谱
  if (!checkCookRecipe(rev.cooktype(), setRecipes))
  {
    XERR << "[料理-制作] 失败，检查食谱失败 charid" << m_pUser->id <<"cooktype" << rev.cooktype() << "material" << rev.ShortDebugString() << XEND;
    breakCooking();
    return false;
  }

  m_bSkipAnimation = rev.skipanimation();
  if (m_bSkipAnimation)
    m_dwCookCompleteTime = now() + 0;
  else
    m_dwCookCompleteTime = now() + COOKING_DUTAION;

  m_state = ECOOKSTATE_COOKING;
  m_dwCookProgress = 0;
  
  clearQueue();
  ntfCookState(COOKING_DUTAION);
  XLOG << "[料理] 开始制作料理 charid" << m_pUser->id << "匹配到的食谱数量" << m_vecRecipes.size() << "msg" << rev.ShortDebugString() << XEND;
  return true;
}

bool SceneFood::onCookComplte()
{
  //检查食材数量
  BasePackage* pPkg = m_pUser->getPackage().getPackage(EPACKTYPE_FOOD);
  if (pPkg == nullptr)
  {
    breakCooking();
    return false;
  }
  for (auto &m : m_vecMaterialInfo)
  {
    if (!pPkg->checkItemCount(m.second.guid(), m.second.count()))
    {
      breakCooking();
      return false;
    }
  }

  for (auto &m : m_vecMaterialInfo)
  {
    //扣除背包食材
    pPkg->reduceItem(m.second.guid(), ESOURCE_COOK_FOOD, m.second.count());
    //增加食材熟练度
    addFoodDataExp(EFOODDATATYPE_MATERIAL, m.second.id(), m.second.count() * 1);
    XLOG << "[料理]制作完成" << m_pUser->id << m_pUser->name << "扣除食材" << m.second.id() << m.second.count() << m.second.guid() << XEND;
  }

  const SFoodMiscCFG rFoodMiscCFG = MiscConfig::getMe().getFoodCfg();
  m_vecProductInfo.clear();
  for (auto& it : m_vecRecipes)
  {
    DWORD dwRecipeId = it.first;
    if (dwRecipeId == 0)
    {
      breakCooking();
      return false;
    }
    
    const SRecipeCFG* pRecipeCfg = RecipeConfig::getMe().getRecipeCFG(dwRecipeId);
    if (pRecipeCfg == nullptr)
    {
      breakCooking();
      return false;
    }
    const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(it.second.m_dwProductId);
    if (!pFoodCfg)
    {
      breakCooking();
      return false;
    }

    DWORD cnt = it.second.m_dwMatchCnt;
    DWORD exp = 0;
    DWORD successNum = 0;
    DWORD darkFoodNum = 0;
    while (cnt--)
    {
      m_bCookSuccess = calcCookSuccess(it.second.m_dwCookSuccessRate);

      //获得料理
      if (!m_bCookSuccess)  //获得黑暗料理
      {
        darkFoodNum++;
      }
      else
      {
        successNum++;
        //计算经验衰减
        SCookerLevel* pCookerLvCfg = const_cast<SCookerLevel*>(TableManager::getMe().getCookerLevelCFG(m_dwCookerLv));
        DWORD tmpexp = pFoodCfg->m_dwCookerExp;
        if (pCookerLvCfg)
        { 
          DWORD rate = pCookerLvCfg->getExpRate(pFoodCfg->m_dwCookHard);
          tmpexp = tmpexp * rate / 100;
        }
        exp += tmpexp;
      }
    }

    auto add_item = [&](DWORD productId, DWORD foodNum, bool bSuccess) -> bool
    {
      if (foodNum == 0)
        return true;
      ItemInfo itemInfo;
      itemInfo.set_id(productId);
      itemInfo.set_count(foodNum);
      itemInfo.set_source(ESOURCE_COOK_FOOD);

      //添加料理成品进背包
      if (m_pUser->getPackage().addItem(itemInfo, EPACKMETHOD_AVAILABLE)  == false)
      { 
        XERR << "[料理-制作] 添加料理成品失败: charid" << m_pUser->id << "料理id" << productId << "数量" << foodNum << XEND;
        breakCooking();
        return false;
      }
      m_pUser->getAchieve().onCookFood(foodNum, bSuccess);
      m_pUser->getQuest().onCookFood(productId);
      addLastCooked(productId, foodNum);
      
      auto it = find_if(m_vecProductInfo.begin(), m_vecProductInfo.end(),  [productId](const TItemCountPair& r){
          return r.first == productId;
          });
      if (it != m_vecProductInfo.end())
      {
        it->second += foodNum;
      }
      else
      {
        TItemCountPair pItem;
        pItem.first = productId;
        pItem.second = foodNum;
        m_vecProductInfo.push_back(pItem);
      }
      return true;
    };

    if (!add_item(rFoodMiscCFG.dwDarkFood, darkFoodNum, false))
      return false;
    if (!add_item(pFoodCfg->m_dwId, successNum, true))
      return false;

    //解锁食谱
    unlockRecipe(dwRecipeId);
    //增加料理熟练度经验
    addFoodDataExp(EFOODDATATYPE_FOODCOOK, pFoodCfg->m_dwId, it.second.m_dwMatchCnt * 1);
    //增加烹饪经验
    addCookerExp(exp);

    if(darkFoodNum != 0)
      addFoodDataExp(EFOODDATATYPE_FOODCOOK, rFoodMiscCFG.dwDarkFood, 1);

    m_pUser->getServant().onFinishEvent(ETRIGGER_COOKFOOD);
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_COOKFOOD);

    XLOG << "[料理] 料理制作成功 charid" << m_pUser->id << "食谱" << dwRecipeId <<"匹配次数" << it.second.m_dwMatchCnt << "产出料理" << pFoodCfg->m_dwId << "黑暗料理数量" << darkFoodNum << XEND;
    //额外奖励
    if(m_bCookSuccess)
    {
      bool bRandOk = pRecipeCfg->m_dwRate >= (DWORD)randBetween(1, 100);
      if(bRandOk)
      {
        m_pUser->getPackage().rollReward(pRecipeCfg->m_dwRewardid, EPACKMETHOD_AVAILABLE, false, true);
        XLOG <<  "[料理], 额外奖励" << m_pUser->id << "食谱" << dwRecipeId << "奖励ID" << pRecipeCfg->m_dwRewardid << XEND;
      }
    }
  }

  m_state = ECOOKSTATE_COMPLETE;
  m_dwCookCompleteTime = 0;
  ntfCookState(END_DURATION);

  //第一次后就可以跳过动画了
  m_pUser->getUserSceneData().addFirstActionDone(EFIRSTACTION_COOKFOOD);

  XLOG << "[料理] 料理制作成功 charid" << m_pUser->id << "食谱数量" << m_vecRecipes.size() << XEND;
  ECookType oldType = m_cookType;
  reset();
  m_state = ECOOKSTATE_PREPAREING;
  m_cookType = oldType;
  if (m_bSkipAnimation)
    m_dwCookOverTime = now() + 0;
  else
    m_dwCookOverTime = now() + END_DURATION;

  return true;
}

bool SceneFood::onCookOver()
{
  //弹出获得窗口，展示料理模型确认料理的属性  
  ItemShow cmd;
  for (auto& it : m_vecProductInfo)
  {
    ItemInfo* pItemInfo = cmd.add_items();
    if (pItemInfo != nullptr)
    {
      pItemInfo->set_id(it.first);
      pItemInfo->set_count(it.second);
    }
    XLOG << "[料理] 料理制作结束 charid" << m_pUser->id <<"产出料理" << it.first<< "数量" << it.second << XEND;
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  ntfCookState(0);
  m_dwCookOverTime = 0;
  XLOG << "[料理] 料理制作结束 charid" << m_pUser->id << "产出料理种类" << m_vecProductInfo.size() << XEND;
  m_vecProductInfo.clear();
  return true;
}

void SceneFood::breakCooking()
{
  if (!m_animationQueue.empty())
  {
    //清空队列，发送打断状态
    clearQueue();
    
    PROTOBUF(m_noneStateMsg, send, len);
    //发给九屏不发给自己
    m_pUser->sendCmdToNine(send, len, GateIndexFilter(m_pUser->id));
  }

  if (m_state == ECOOKSTATE_NONE)
    return;
  if (m_state == ECOOKSTATE_COMPLETE)
    return;

  m_state = ECOOKSTATE_NONE;
  ntfCookState(0);
  reset();
}

void SceneFood::mutableCookState(Cmd::CookStateMsg* pState)
{
  if (pState == nullptr)
    return;
  if (m_bSkipAnimation)
  {
    *pState = m_curSyncStateMsg;
  }
  else
    mutableCookStateInner(pState);
}

void SceneFood::mutableCookStateInner(Cmd::CookStateMsg* pState)
{
  if (pState == nullptr)
    return;
  switch (m_state)
  {
  case ECOOKSTATE_NONE:
    break;
  case ECOOKSTATE_PREPAREING:
  {
    if (m_cookType > ECOOKTYPE_MIN)
      pState->set_cooktype(m_cookType);
    break;
  }
  case ECOOKSTATE_COOKING:
  {
    break;
  }
  case ECOOKSTATE_COMPLETE:
  {
    pState->set_success(m_bCookSuccess);
    for (auto& it : m_vecProductInfo)
    {
      pState->add_foodid(it.first);
    }
    break;
  }
  default:
    break;
  }
  pState->set_state(m_state);

}

void SceneFood::ntfCookState(DWORD dwTime)
{
  Cmd::CookStateNtf cmd;
  cmd.set_charid(m_pUser->id);
  CookStateMsg* pState = cmd.mutable_state(); 
  mutableCookStateInner(pState);
  bool bQueue = false;
  bool bMy = false;
  switch (m_state)
  {
  case ECOOKSTATE_NONE:
    bMy = true;
    bQueue = true;
    break;
  case ECOOKSTATE_PREPAREING:
    bMy = true;
    bQueue = false;
    break;
  case ECOOKSTATE_COOKING:
    if (m_bSkipAnimation)
      bQueue = true;
    else
      bMy = true;
    break;
  case ECOOKSTATE_COMPLETE:
    if (m_bSkipAnimation)
      bQueue = true;
    else
      bMy = true;
    break;
  default:
    break;
  }

  if (bQueue)
  {
    SAnimation s;
    s.m_dwNtfTime = now() + dwTime;
    s.m_msg = cmd;
    m_animationQueue.push(s);
  }
  if (bMy)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToNine(send, len);
  }
}

bool SceneFood::isCookTypeValid(Cmd::ECookType type)
{
  if (type > ECOOKTYPE_MIN && type < ECOOKTYPE_MAX)
    return true;
  return false;
}

void SceneFood::reset()
{
  m_state = ECOOKSTATE_NONE; 
  m_cookType = ECOOKTYPE_MIN;
  m_bCookSuccess = false;
  m_vecRecipes.clear();
  m_vecMaterialInfo.clear();
  m_vecItemCount.clear();
}

bool SceneFood::putFood(Cmd::PutFood& rev)
{
  //check scene
  if (m_pUser->getScene() == nullptr)
    return false;  

  // 某些地图不可以放置料理
  if (m_pUser->getScene()->getBaseCFG() && m_pUser->getScene()->getBaseCFG()->noFood())
    return false;

  if (rev.foodguid().empty())
    return false;

  if (!EEatPower_IsValid(rev.power()))
    return false;

  //check count
  DWORD foodCount = SceneNpcManager::getMe().getFoodNpcCount(m_pUser->id);
  if (foodCount >= MiscConfig::getMe().getFoodCfg().dwMaxPutFoodCount)
  {
    MsgManager::sendMsg(m_pUser->id, 3510);
    XLOG <<"[料理-摆放] 次数超过限制，charid" << m_pUser->id << "foodguid" << rev.foodguid() << "已摆放次数" <<foodCount <<"限制次数"<< MiscConfig::getMe().getFoodCfg().dwMaxPutFoodCount << XEND;
    return false;
  }

  EPackType eType = m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == true ? EPACKTYPE_FOOD : EPACKTYPE_MAIN;
  BasePackage* pPkg = m_pUser->getPackage().getPackage(eType);
  if (pPkg == nullptr)
    return false;

  DWORD dwFoodNum = rev.foodnum() == 0 ? 1 : rev.foodnum();
  if (rev.peteat())
  {
    dwFoodNum = 1;
  }
  if (!pPkg->checkItemCount(rev.foodguid(), dwFoodNum))
  {
    XLOG <<"[料理-摆放] 物品不足，charid" << m_pUser->id << "foodguid" << rev.foodguid() << "摆放数量" << dwFoodNum << XEND;
    return false;
  }

  ItemBase* pFood = pPkg->getItem(rev.foodguid());
  if (pFood == nullptr || pFood->getCount() < dwFoodNum)
    return false;
  SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(pFood->getTypeID());
  if (pFoodCfg == nullptr)
    return false;
  
  NpcDefine def;
  def.setID(pFoodCfg->m_dwNpcId);
  //def.setBehaviours(def.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);
  def.setTerritory(0);
  DWORD dir = m_pUser->getUserSceneData().getDir();
  xPos destPos;
  //玩家对面0.5米摆放
  m_pUser->getScene()->getPosByDir(m_pUser->getPos(), 0.5, dir, destPos);
  def.setPos(destPos);
  def.m_oVar.m_qwOwnerID = m_pUser->id;
  FoodNpc*pNpc = dynamic_cast<FoodNpc*>(SceneNpcManager::getMe().createNpc(def, m_pUser->getScene()));
  if (pNpc == nullptr)
  {
    XERR << "[料理-摆放] 创建npc失败，道具已扣除 charid"<< m_pUser->id <<"foodguid"<< rev.foodguid() << "料理id" << pFoodCfg->m_dwId << XEND;
    return false;
  }
  pNpc->setFoodId(pFoodCfg->m_dwId);
  pNpc->setFoodNum(dwFoodNum);
  pNpc->setFoodTotalNum(dwFoodNum);
  pNpc->setPower(rev.power());
  pNpc->setOwnerID(m_pUser->id, m_pUser->name);
  //扣道具
  pPkg->reduceItem(pFood->getGUID(), ESOURCE_EAT_FOOD, dwFoodNum);

  //onPutFood(pNpc);
  XLOG << "[料理-摆放] 成功 charid" << m_pUser->id << "料理npcguid" << pNpc->getGUID() << "料理npcid" << pNpc->getNpcID() << "料理道具id" << pFoodCfg->m_dwId << "料理道具guid" << rev.foodguid() << "料理数量" << dwFoodNum;

  if (rev.peteat())
  {
    m_pUser->getUserPet().onSeeFood(pNpc);
    if (rev.foodnum() > 1)
    {
      MsgManager::sendMsg(m_pUser->id, 25429);
    }
    XLOG << "选择宠物享用" << XEND;
  }
  else if (rev.power() == EEATPOWR_SELF)
  {
    if (pNpc->addEater(m_pUser) == false)
    {
      XLOG << "选择自己享用失败" << XEND;
      return false;
    }

    m_qwEatingFoodNpc = pNpc->getGUID();
    m_dwEatingFoodNum = dwFoodNum;
    XLOG << "选择自己享用" << XEND;
  }
  return true;
}

bool SceneFood::editFoodPower(Cmd::EditFoodPower& rev)
{
  if (rev.npcguid() == 0)
    return false;
  
  FoodNpc* pNpc = dynamic_cast<FoodNpc*> (SceneNpcManager::getMe().getNpcByTempID(rev.npcguid()));
  if (!pNpc) return false;
  
  if (!pNpc->checkOwner(m_pUser->id))
    return false;
  pNpc->changePower(m_pUser, rev.power());

  XLOG << "[吃料理-编辑权限] 成功， charid" << m_pUser->id << "npcid" <<rev.npcguid() << "itemid" << pNpc->getFoodId() << " NEW power" << rev.power() << XEND;
  return true;
 }

void SceneFood::queryFoodNpcInfo(QueryFoodNpcInfo& rev)
{
  FoodNpc* pNpc = dynamic_cast<FoodNpc*> (SceneNpcManager::getMe().getNpcByTempID(rev.npcguid()));
  if (!pNpc)
    return ;

  rev.set_eating_people(pNpc->getEaterCount());
  rev.set_itemid(pNpc->getFoodId());
  rev.set_ownerid(pNpc->getOwnerID());
  rev.set_itemnum(pNpc->getFoodNum());
  
  PROTOBUF(rev, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool SceneFood::startEat(Cmd::StartEat& rev)
{
  // 宠物吃料理
  if (rev.pet())
  {
    FoodNpc* pNpc = dynamic_cast<FoodNpc*> (SceneNpcManager::getMe().getNpcByTempID(rev.npcguid()));
    if (pNpc == nullptr || pNpc->checkCanEat(m_pUser) == false)
      return false;
    m_pUser->getUserPet().onSeeFood(pNpc);
    return true;
  }

  if (m_qwEatingFoodNpc)
    return false;

  if (rev.npcguid() == 0)
    return false; 
  //if (m_dwTasterLv == 0)
  //{
  //  MsgManager::sendMsg(m_pUser->id, 3507);
  //}

  //check power
  FoodNpc* pNpc = dynamic_cast<FoodNpc*> (SceneNpcManager::getMe().getNpcByTempID(rev.npcguid()));
  if (pNpc == nullptr)
    return false;

  if (!pNpc->checkCanEat(m_pUser))
  {
    MsgManager::sendMsg(m_pUser->id, 3504);
    XERR << "[料理-吃料理] 失败，权限不够 charid" << m_pUser->id << "npcid" << pNpc->getNpcID() << "npcguid" << pNpc->getGUID() << XEND;
    return false;
  }
  //check 饱腹度
  if (checkSatiety() == false)
  {
    MsgManager::sendMsg(m_pUser->id, 3505);
    return false;
  }
  if(pNpc->getNpcID() == MiscConfig::getMe().getFoodCfg().dwChristmasCake && m_pUser->getVar().getVarValue(EVARTYPE_CHRISTMAS_CAKE) != 0)
  {
    MsgManager::sendMsg(m_pUser->id, 3612);
    return false;
  }

  if (pNpc->addEater(m_pUser) == false)
    return false;

  m_qwEatingFoodNpc = rev.npcguid();
  m_dwEatingFoodNum = rev.eatnum();
  return true;
}

void SceneFood::breakEating()
{
  if (m_qwEatingFoodNpc == 0)
    return;
  FoodNpc* pNpc = dynamic_cast<FoodNpc*>(SceneNpcManager::getMe().getNpcByTempID(m_qwEatingFoodNpc));
  if (!pNpc)
    return;

  stopEat(pNpc, false);
}

bool SceneFood::stopEat(FoodNpc* pNpc, bool eatSuccess)
{
  if (pNpc == nullptr)
    return false;
  if (m_qwEatingFoodNpc == 0)
    return false;
  if (m_qwEatingFoodNpc != pNpc->getGUID())
    return false;
  
  UserActionNtf cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_type(EUSERACTIONTYPE_EXPRESSION);
  if (eatSuccess)
    cmd.set_value(20);
  else
    cmd.set_value(1);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  pNpc->delEater(m_pUser);
  m_qwEatingFoodNpc = 0;
  m_dwEatingFoodNum = 0;
  return true;
}

void SceneFood::onEatFood(FoodNpc* pNpc)
{
  if (!pNpc)
    return;

  if (pNpc->getFoodNum() < m_dwEatingFoodNum)
  {
    m_dwEatingFoodNum = pNpc->getFoodNum();
  }

  bool bIsDarkFood = MiscConfig::getMe().getFoodCfg().isDarkFood(pNpc->getFoodId());

  SFoodItemInfo sItemInfo;
  sItemInfo.dwItemId = pNpc->getFoodId();

  const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(sItemInfo.dwItemId);
  if (!pFoodCfg)
    return; 

  //DWORD dwFoodTasteLv = getFoodDataLevel(EFOODDATATYPE_FOODTASTE, pFoodCfg->m_dwId);
  DWORD dwDuration = pFoodCfg->m_dwDuration;
  sItemInfo.dwInvalidTime = now() + dwDuration;

  // f04280 计算n个料理带来的效果 统一添加
  //回复hp 
  m_pUser->changeHp(pFoodCfg->m_dwHealHP * m_dwEatingFoodNum, m_pUser);
  //回复sp
  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr)
    pFighter->setSp(pFoodCfg->m_dwHealSP * m_dwEatingFoodNum + pFighter->getSp());
  //储存hp sp
  DWORD saveHP = pFoodCfg->m_dwSaveHP * m_dwEatingFoodNum;
  DWORD saveSP = pFoodCfg->m_dwSaveSP * m_dwEatingFoodNum;  
  //TODO for test
  //saveHP = 5000000;
  //saveSP = 2000000;
  setSaveHPSP(saveHP, saveSP);
  
  //增加饱腹度
  int satiety = pFoodCfg->m_dwFullProgress * m_dwEatingFoodNum;
  addSatiety(satiety);
  
  if (m_dwTasterLv)
  {
    //增加美食家经验
    addTasterExp(pFoodCfg->m_dwTasterExp * m_dwEatingFoodNum);
  }

  //修身效果,词缀的修身效果已经放在属性里了
  if (pFoodCfg->m_height != 0)
  {
    UserAttrSvr attr;
    attr.set_type(EATTRTYPE_SLIM_HEIGHT);
    attr.set_value(pFoodCfg->m_height);
    ////TODO fortest
    //attr.set_value(15);
    sItemInfo.vecAttr.push_back(attr);
  }

  /* f04280 重复代码
  if (pFoodCfg->m_weight)
  {
    UserAttrSvr attr;
    attr.set_type(EATTRTYPE_SLIM_WEIGHT);
    attr.set_value(pFoodCfg->m_weight);
    ////TODO fortest
    //attr.set_value(15);
    sItemInfo.vecAttr.push_back(attr);
  }
  */
  
  //删掉当前被顶掉的料理
  if (!bIsDarkFood)
  {
    DWORD dwCurFoodNum = m_vecFoodItemInfo.size();
    DWORD dwNeedDelFoodNum = (dwCurFoodNum + m_dwEatingFoodNum) > m_dwLimitFood ? ((dwCurFoodNum + m_dwEatingFoodNum) - m_dwLimitFood) : 0;
    auto it = m_vecFoodItemInfo.begin();

    Cmd::UpdateFoodInfo updateCmd;
    for (DWORD i = 0; i < dwNeedDelFoodNum; i++)
    {
      if (it == m_vecFoodItemInfo.end())
        break;
      const SFoodConfg* pCfg = FoodConfig::getMe().getFoodCfg((*it).dwItemId);
      if (pCfg)
      {
        for (auto &v : pCfg->m_vecBuff)
        { 
          m_pUser->getBuff().del(v);
        }
      }

      updateCmd.add_del_eat_foods(it->dwItemId);
      onDelFoodItem(*it);
      it = m_vecFoodItemInfo.erase(it);
    }

    //添加最终有效的料理
    dwCurFoodNum = m_vecFoodItemInfo.size();
    DWORD dwNeedAddFoodNum = m_dwLimitFood > dwCurFoodNum ? m_dwLimitFood - dwCurFoodNum : 0;

    DWORD j = 0;
    for (DWORD i = 0 ; i < m_dwEatingFoodNum; i++)
    {
      if (j < dwNeedAddFoodNum)
      {
        for (auto &v : pFoodCfg->m_vecBuff)
        {
          //m_pUser->getBuff().addFoodBuff(v, dwDuration, extraBuffEffect);
          m_pUser->getBuff().add(v);
        }

        onAddFoodItem(sItemInfo);

        sItemInfo.toData(updateCmd.add_eat_foods());
      }
      j++;
    }

    updateEatedFoodNum();
    PROTOBUF(updateCmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  else if (m_dwEatingFoodNum > 0)
  {
    //吃黑暗料理  无论一次吃多少个，只有一个生效。
    for (auto &v : pFoodCfg->m_vecBuff)
    {
      m_pUser->getBuff().add(v);
    }
  }
  addJoyOnEat(pNpc->m_dwItemId, m_dwEatingFoodNum);
  addFoodLimitNum(sItemInfo.dwItemId);
  if (m_dwTasterLv)
  {
    //系统招出来的不增加美食熟练度    2017-09-15 彭冉提
    if (!pNpc->isSysSummon())
      addFoodDataExp(EFOODDATATYPE_FOODTASTE, pFoodCfg->m_dwId, m_dwEatingFoodNum * 1);
  }

  m_pUser->getAchieve().onEatFood(pNpc->getFoodId(), m_dwEatingFoodNum, pNpc->getOwnerID() != m_pUser->id);
  if(pNpc->getNpcID() == MiscConfig::getMe().getFoodCfg().dwChristmasCake)
    m_pUser->getVar().setVarValue(EVARTYPE_CHRISTMAS_CAKE, 1);

  //播放特效
  m_pUser->effect(1);
  m_pUser->getServant().onFinishEvent(ETRIGGER_EAT_FOOD);

  pNpc->setFoodNum(pNpc->getFoodNum() - m_dwEatingFoodNum);
  
  XLOG << "[料理-吃完] 吃下料理获得料理效果 charid" << m_pUser->id << "foodid" << sItemInfo.dwItemId << "foodnum" << m_dwEatingFoodNum << "储备HP" <<m_dwSaveHp <<"储备SP" <<m_dwSaveSp << "Npc剩余料理数量" << pNpc->getFoodNum() << XEND;
}

void SceneFood::clickManual(Cmd::EFoodDataType type, DWORD itemId)
{
  if (type != EFOODDATATYPE_FOODCOOK && type != EFOODDATATYPE_FOODTASTE)
    return;
  
  auto it = m_mapFoodManualData.find(type);
  if (it == m_mapFoodManualData.end())
    return;

  SFoodSubData* pSubData = it->second.getSubData(itemId);
  if (pSubData == nullptr)
    return;
  if (pSubData->status != EFOODSTATUS_ADD)
  {
    XERR << "[料理-冒险手册解锁] 点击解锁，状态不一致，charid" << m_pUser->id << "type" << type << "itemid" << itemId << "服务器状态" << pSubData->status << XEND;
    return;
  }
  
  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[料理-冒险手册解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << type << "id :" << itemId << "status :" << (pSubData == nullptr ? EFOODSTATUS_MIN : pSubData->status) << "解锁失败,未找到包裹" << XEND;
    return ;
  }
  TVecItemInfo vecRewardItems;

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(itemId);
  if (pCFG == nullptr)
  {
    XERR << "[料理-冒险手册解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << type << "id :" << itemId << "status :" << (pSubData == nullptr ? EFOODSTATUS_MIN : pSubData->status) << "解锁失败,未在Table_Item.txt表中找到" << XEND;
    return ;
  }

  vecRewardItems = pCFG->vecManualItems;
  for (auto &s : pCFG->setAdvRewardIDs)
  {
    TVecItemInfo vecReward;
    if (RewardManager::roll(s, m_pUser, vecReward, ESOURCE_MANUAL) == false)
    {
      XERR << "[料理-冒险手册解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << type << "id :" << itemId << "status :" << (pSubData == nullptr ? EFOODSTATUS_MIN : pSubData->status) << "解锁失败," << s << "随机奖励失败" << XEND;
      return ;
    }
    combinItemInfo(vecRewardItems, vecReward);
  }
  if (!vecRewardItems.empty() && pMainPack->checkAddItem(vecRewardItems) == false)
  {
    XERR << "[料理-冒险手册解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << type << "id :" << itemId << "status :" << (pSubData == nullptr ? EFOODSTATUS_MIN : pSubData->status) << "解锁失败,奖励超过背包上限" << XEND;
    return ;
  }

  pSubData->status = EFOODSTATUS_CLICKED;

  if (pCFG->swAdventureValue > 0)
  {
    m_pUser->getManual().addPoint(pCFG->swAdventureValue, EMANUALTYPE_MIN, itemId);
  }
  if (vecRewardItems.empty() == false)
    m_pUser->getPackage().addItem(vecRewardItems, EPACKMETHOD_AVAILABLE);

  for (auto v = pCFG->vecAdvBuffIDs.begin(); v != pCFG->vecAdvBuffIDs.end(); ++v)
    m_pUser->m_oBuff.add(*v);
    
  NewFoodDataNtf cmd;
  FoodManualData* pData = cmd.add_items();
  if (pData == nullptr)
    return;
  pData->set_type(type);
  pSubData->toData(pData->add_datas());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getMenu().refreshNewMenu(EMENUCOND_MANUALUNLOCK);
  XLOG << "[料理-冒险手册解锁] 成功点击解锁，charid" << m_pUser->id << "type" << type << "itemid" << itemId << "服务器状态" << pSubData->status << "增加冒险积分" << pCFG->swAdventureValue << XEND;
}

bool SceneFood::openFunction()
{
  const SFoodMiscCFG& rCfg = MiscConfig::getMe().getFoodCfg();
  if (rCfg.dwSkillId)
    m_pUser->addSkill(rCfg.dwSkillId, 0, ESOURCE_SHOP);

  ////学习后立即获得“美食家的餐刀”，用来获取食材
  //{
  //  ItemInfo itemInfo;
  //  itemInfo.set_id(rCfg.dwCookerKnife);
  //  itemInfo.set_count(1);
  //  m_pUser->getPackage().addItemAvailable(itemInfo);
  //}  
  ////  学习后立即获得“厨师帽”头饰
  //{
  //  ItemInfo itemInfo;
  //  itemInfo.set_id(rCfg.dwCookerHat);
  //  itemInfo.set_count(1);
  //  m_pUser->getPackage().addItemAvailable(itemInfo);
  //}

  //升到1级
  if (m_dwCookerLv == 0)
    cookerLvUp();

  //升到1级
  if (m_dwTasterLv == 0)
    tasterLvUp();
  XLOG << "[料理-开启料理功能] charid" << m_pUser->id << "厨师等级" << m_dwCookerLv << "美食家等级" << m_dwTasterLv << "料理技能" << rCfg.dwSkillId << XEND;
  return true;
}

DWORD SceneFood::getBagSlot(EPackType packType)
{
  return packType == EPACKTYPE_MAIN ? m_dwBagSlot : 0;
}

DWORD SceneFood::getCookSuccessRate(DWORD dwFoodId, DWORD dwCookHard, DWORD avgMaterialLv)
{
  /**每级料理制作书，增加成功率5%，共10级
    *食材平均熟练度，每级 + 3 %
    *烹饪熟练度，每级 + 3 %
    *如果料理的烹饪难度超出厨师等级，每高一级成功率减一半*/
  //DWORD left = dwCookHard;
  //float totalRate = 100.0;
  //while (left > m_dwCookerLv)
  //{
  //  totalRate = totalRate / 2;
  //  left--;
  //}
  //
  //for (DWORD i = 1; i < m_dwCookerLv; ++i)
  //{
  //  const SCookerLevel*pCfg = TableManager::getMe().getCookerLevelCFG(i);
  //  if (pCfg)
  //  {
  //    totalRate += totalRate * pCfg->getSuccessRate() / 100;
  //  }
  //}
  //
  ////食材平均熟练度，每级 + 3 %
  //totalRate = calcAddValue(avgMaterialLv, totalRate, 3);
  ////烹饪熟练度，每级 + 3 %
  //DWORD dwLv = getFoodDataLevel(EFOODDATATYPE_FOODCOOK, dwFoodId);
  //totalRate = calcAddValue(dwLv, totalRate, 3);
  //
  //if (randBetween(1, 100) < totalRate)
  //  m_bCookSuccess = true;
  //else
  //  m_bCookSuccess = false;

  DWORD extraRate = 0;
  const SCookerLevel*pCfg = TableManager::getMe().getCookerLevelCFG(m_dwCookerLv);
  if (pCfg)
  {
    extraRate = pCfg->getSuccessRate() ; //百分比
  }
  DWORD dwLv = getFoodDataLevel(EFOODDATATYPE_FOODCOOK, dwFoodId); 
  DWORD dwRate = LuaManager::getMe().call<DWORD>("calcCookSuccessRate", m_dwCookerLv, dwLv, dwCookHard, avgMaterialLv, extraRate);
  XDBG << "[料理-计算烹饪成功率] charid" << m_pUser->id << "foodid" << dwFoodId << "厨师等级" << m_dwCookerLv << "料理熟练度等级" << dwLv << "料理烹饪难度" << dwCookHard << "平均食材熟练度等级" << avgMaterialLv << "额外加成成功率" << extraRate << "最终成功率" << dwRate << XEND;
  return dwRate;
}

bool SceneFood::calcCookSuccess(DWORD dwSuccessRate)
{
  DWORD r = randBetween(1, 1000);
  if (r <= dwSuccessRate)
    m_bCookSuccess = true;
  else
    m_bCookSuccess = false;
  return m_bCookSuccess;
}

DWORD SceneFood::getSatiety()
{
  return 0;
  /* f04280 去掉饱食度概念
  DWORD curSec = now();
  if (curSec >= m_dwSatiety)
    return 0;
  if (MiscConfig::getMe().getFoodCfg().fSatietyRate == 0)
    return 0;

  DWORD r = (m_dwSatiety - curSec) / MiscConfig::getMe().getFoodCfg().fSatietyRate;
  return r;
  */
}

void SceneFood::addSatiety(DWORD dwSatiety)
{
  /* f04280 去掉饱食度概念
  DWORD old = getSatiety();
  DWORD curSec = now();   
  DWORD r = dwSatiety * MiscConfig::getMe().getFoodCfg().fSatietyRate;
  
  if (curSec >= m_dwSatiety)
    m_dwSatiety = curSec + r;
  else
    m_dwSatiety += r;
    
  MsgManager::sendMsg(m_pUser->id, 3506, MsgParams(dwSatiety));
  m_pUser->setDataMark(EUSERDATATYPE_SATIETY);
  m_pUser->refreshDataAtonce(); 

  XLOG << "[料理-饱腹度] 增加饱腹度，charid" << m_pUser->id << "增加" << dwSatiety << "增加前的饱腹度" <<  old  << "增加后的饱腹度" << getSatiety() << "上限" << m_dwLimitSatiety << XEND;
  */
}

void SceneFood::clearSatiety()
{
  m_dwSatiety = 0;
  // f04280 去掉饱食度概念
  //m_pUser->setDataMark(EUSERDATATYPE_SATIETY);
  //m_pUser->refreshDataAtonce();
  //XLOG << "[料理-饱腹度] 清空饱腹度，charid" << m_pUser->id << XEND;
}

bool SceneFood::checkSatiety()
{
  return true;
  /* f04280 去掉饱食度概念
  DWORD old = getSatiety();
  DWORD limit = getLimitSatiety();
  if (old >= limit)
  {
    XERR << "[料理-饱腹度] 饱腹度已经达到上限，不可吃料理，charid" << m_pUser->id << "当前饱腹度" << old << "上限" << limit << XEND;
    return false;
  }
  return true;
  */
}

DWORD SceneFood::getLimitSatiety()
{
  return m_dwLimitSatiety;
  //const STasterLevel* pCfg = TableManager::getMe().getTasterLevelCFG(m_dwTasterLv);
  //if (pCfg == nullptr)
  //  return 0;

  //return pCfg->getFullProgress();
}

DWORD SceneFood::getEatedFoodNum()
{
  return m_vecFoodItemInfo.size();
}

void SceneFood::updateEatedFoodNum()
{
  m_pUser->setDataMark(EUSERDATATYPE_SATIETY);
  m_pUser->refreshDataAtonce();
}

DWORD SceneFood::calcAddValue(DWORD dwLv, DWORD dwBase, DWORD dwRate)
{
  float a = dwBase;
  DWORD dwTotalRate = 0;
  while (dwLv--)
  {
    dwTotalRate += dwRate / 100;
  }
  a = a + (a * dwTotalRate);
  return (DWORD)a;
}

void SceneFood::setSaveHPSP(DWORD dwHP, DWORD dwSP)
{
  bool changed = false;
  if (dwHP != 0)
  {
    m_dwSaveHp += dwHP;      
    m_pUser->setAttr(EATTRTYPE_SAVE_HP, m_dwSaveHp);
    changed = true;
  }

  if (dwSP != 0)
  {
    m_dwSaveSp += dwSP;
    m_pUser->setAttr(EATTRTYPE_SAVE_HP, m_dwSaveSp);
    changed = true;
  }

  if (!changed)
    return;
  m_pUser->refreshDataAtonce();
}

void SceneFood::updateEatedFood(DWORD curSec)
{
  //ntf client
  Cmd::UpdateFoodInfo updateCmd;

  for (auto it = m_vecFoodItemInfo.begin(); it != m_vecFoodItemInfo.end();)
  {
    if (curSec >= it->dwInvalidTime)
    {
      updateCmd.add_del_eat_foods(it->dwItemId);
      onDelFoodItem(*it);
      it = m_vecFoodItemInfo.erase(it);
      continue;
    }
    ++it;
  }
  
  if (updateCmd.del_eat_foods_size() > 0)
  {
    updateEatedFoodNum();
    PROTOBUF(updateCmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void SceneFood::onAddFoodItem(SFoodItemInfo& rItemInfo)
{
  //const SFoodConfg* pCfg = FoodConfig::getMe().getFoodCfg(rItemInfo.dwItemId);
  //if (!pCfg)
  //  return;   
  //
  //m_height += pCfg->m_height;
  //m_weight += pCfg->m_weight;

  //if (m_pUser->getUserSceneData().getOption(EOPTIONTYPE_USE_SLIM))
  //{
  //  m_pUser->setAttr(EATTRTYPE_SLIM_HEIGHT, m_height);
  //  m_pUser->setAttr(EATTRTYPE_SLIM_WEIGHT, m_weight);
  //}

  m_vecFoodItemInfo.push_back(rItemInfo);

  m_pUser->getAttribute()->setCollectMark(ECOLLECTTYPE_FOOD);
  m_pUser->refreshDataAtonce();
}

void SceneFood::onDelFoodItem(SFoodItemInfo& rItemInfo)
{
  //const SFoodConfg* pCfg = FoodConfig::getMe().getFoodCfg(rItemInfo.dwItemId);
  //if (!pCfg)
  //  return;
  //if (m_height >= pCfg->m_height)
  //  m_height -= pCfg->m_height;
  //else
  //  m_height = 0;
  //if (m_weight >= pCfg->m_weight)
  //  m_weight -= pCfg->m_weight;
  //else
  //  m_weight = 0;

  //if (m_pUser->getUserSceneData().getOption(EOPTIONTYPE_USE_SLIM))
  //{
  //  m_pUser->setAttr(EATTRTYPE_SLIM_HEIGHT, m_height);
  //  m_pUser->setAttr(EATTRTYPE_SLIM_WEIGHT, m_weight);
  //}
  
  m_pUser->getAttribute()->setCollectMark(ECOLLECTTYPE_FOOD);
  m_pUser->refreshDataAtonce();
  XLOG << "料理-到期 charid" << m_pUser->id << "foodid" << rItemInfo.dwItemId << "到期时间" << rItemInfo.dwInvalidTime << XEND;
}

void SceneFood::refreshSlim(DWORD flag)
{
  m_pUser->getAttribute()->setCollectMark(ECOLLECTTYPE_FOOD);
  m_pUser->refreshDataAtonce();
}

//void SceneFood::collectAttr(TVecAttrSvrs& attr)
void SceneFood::collectAttr()
{
  Attribute* pAttr = m_pUser->getAttribute();
  if (pAttr == nullptr)
    return;
  //if (attr.empty())
  //  return;
  
  for (auto& v : m_vecFoodItemInfo)
  {
    if (v.vecAttr.empty())
      continue;
    
    for (auto&subV : v.vecAttr)
    {
      //TODO fortest
      if ((subV.type() == EATTRTYPE_SLIM_HEIGHT || subV.type() == EATTRTYPE_SLIM_WEIGHT) && !m_pUser->getUserSceneData().getOption(EOPTIONTYPE_USE_SLIM))
      {
      }
      else
        //attr[subV.type()].set_value(attr[subV.type()].value() + subV.value());
        pAttr->modifyCollect(ECOLLECTTYPE_FOOD, subV);
    }
  }
    
  //永久属性
  for (auto &s : m_setCookLvAttr)
  {
    const SFoodConfg* pCfg = FoodConfig::getMe().getFoodCfg(s);
    if (!pCfg || pCfg->m_vecCookLvAttr.empty())
    {
      continue;
    }
    for (auto &a : pCfg->m_vecCookLvAttr)
      pAttr->modifyCollect(ECOLLECTTYPE_FOOD, a);
    /*{
      float value = attr[a.type()].value() + a.value();
      attr[a.type()].set_value(value);
    }*/
    XDBG << "[料理-永久属性] 烹饪熟练度等级添加" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "料理id" << s << XEND;
  }

  for (auto &s : m_setTasteLvAttr)
  {
    const SFoodConfg* pCfg = FoodConfig::getMe().getFoodCfg(s);
    if (!pCfg || pCfg->m_vecTasteLvAttr.empty())
    {
      continue;
    }
    for (auto &a : pCfg->m_vecTasteLvAttr)
      pAttr->modifyCollect(ECOLLECTTYPE_FOOD, a);
    /*{
      float value = attr[a.type()].value() + a.value();
      attr[a.type()].set_value(value);
    }*/
    XDBG << "[料理-永久属性] 品尝熟练度等级添加" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "料理id" << s << XEND;
  }
}

void SceneFood::checkHP()
{
  if (!m_pUser->getScene())
    return;
  if (m_pUser->isInPollyScene())
    return;
  if (!m_pUser->isAlive())
    return;

  if (!m_dwSaveHp)
    return;
  if (!m_pUser->getUserSceneData().getOption(EOPTIONTYPE_USE_SAVE_HP))
    return;

  SQWORD curHp = m_pUser->getAttribute()->getAttr(EATTRTYPE_HP);
  SQWORD maxHp = m_pUser->getAttribute()->getAttr(EATTRTYPE_MAXHP);   
  float rate = curHp / (float)maxHp;
  if (rate >= 0.3)
    return;

  //type='heal',hptype=1,hpvalue={385,400},formula=4
  DWORD value = 400;
  value = LuaManager::getMe().call<float>("CalcItemHealValue", value, m_pUser, 4);
  value = std::min(m_dwSaveHp, value);
  m_dwSaveHp -= value;
  m_pUser->getAttribute()->setAttr(EATTRTYPE_SAVE_HP, m_dwSaveHp); //changHp 的时候会刷新
  m_pUser->changeHp(value, m_pUser);
  m_pUser->getAchieve().onUseSaveHp(value);

  XLOG << "[料理-自动回复hp] charid" << m_pUser->id << "回复前hp" << curHp << "回复" << value << "剩余储备hp" << m_dwSaveHp << XEND;
}

void SceneFood::checkSP()
{
  if (!m_pUser->getScene())
    return;
  if (m_pUser->isInPollyScene())
    return;  
  if (!m_pUser->isAlive())
    return;
  if (!m_dwSaveSp)
    return;
  if (!m_pUser->getUserSceneData().getOption(EOPTIONTYPE_USE_SAVE_SP))
    return;

  SQWORD curSp = m_pUser->getAttribute()->getAttr(EATTRTYPE_SP);
  SQWORD maxSp = m_pUser->getAttribute()->getAttr(EATTRTYPE_MAXSP);
  float rate = curSp / (float)maxSp;
  if (rate >= 0.3)
    return;

  //type='heal',sptype=1,spvalue={30,50},formula=5
  DWORD value = 50;
  value = LuaManager::getMe().call<float>("CalcItemHealValue", value, m_pUser, 5);
  value = std::min(m_dwSaveSp, value);
  m_dwSaveSp -= value;
  m_pUser->getAttribute()->setAttr(EATTRTYPE_SAVE_HP, m_dwSaveSp); //changHp 的时候会刷新
  
  SceneFighter* pFighter = m_pUser->getFighter();
  if (!pFighter)
    return;
  pFighter->setSp(value + pFighter->getSp());

  m_pUser->getAchieve().onUseSaveSp(value);
  XLOG << "[料理-自动回复sp] charid" << m_pUser->id << "回复前hp" << curSp << "回复" << value << "剩余储备sp" << m_dwSaveSp << XEND;
}

bool SceneFood::checkCookerLvExp(DWORD lv)
{
  if (lv == 0)
    return true;

  if (m_dwCookerLv != lv - 1)
  {
    XDBG << "[料理-任务] 不可下发任务，厨师等级不达标 charid" << m_pUser->id << "厨师等级" << m_dwCookerLv << "任务等级" << lv - 1 << XEND;
    return false;
  }
  
  const SCookerLevel* pCfg = TableManager::getMe().getCookerLevelCFG(lv);
  if (!pCfg)
    return false;
  if (m_dwCookerExp < pCfg->getNeedExp())
  {
    XDBG << "[料理-任务] 不可下发任务，厨师经验不达标 charid" << m_pUser->id << "厨师经验" << m_dwCookerExp << "任务等级" << lv << "需要经验" << pCfg->getNeedExp() << XEND;
    return false;
  }
  XDBG << "[料理-任务] 下发任务返回帧 charid" << m_pUser->id << "厨师经验" << m_dwCookerExp << "任务等级" << lv << "需要经验" << pCfg->getNeedExp() << XEND;
  return true;
}

bool SceneFood::checkTasterLvExp(DWORD lv)
{
  if (lv == 0)
    return true;

  if (m_dwTasterLv != lv - 1)
  {
    XDBG << "[料理-任务] 不可下发任务，美食家等级不达标 charid" << m_pUser->id << "美食家等级" << m_dwTasterLv << "任务等级" << lv - 1 << XEND;
    return false;
  }

  const STasterLevel * pCfg = TableManager::getMe().getTasterLevelCFG(lv);
  if (!pCfg)
    return false;
  if (m_dwTasterExp < pCfg->getNeedExp())
  {
    XDBG << "[料理-任务] 不可下发任务，美食家经验不达标 charid" << m_pUser->id << "美食家经验" << m_dwTasterExp << "任务等级" << lv << "需要经验" << pCfg->getNeedExp() << XEND;
    return false;
  }
  XDBG << "[料理-任务] 下发任务返回真 charid" << m_pUser->id << "美食家经验" << m_dwTasterExp << "任务等级" << lv << "需要经验" << pCfg->getNeedExp() << XEND;
  return true;
}

void SceneFood::syncAnimaion(DWORD curSec)
{
  if (m_animationQueue.empty())
    return;
  if (m_animationQueue.front().m_dwNtfTime > curSec)
    return;
  
  PROTOBUF(m_animationQueue.front().m_msg, send, len);
  m_pUser->sendCmdToNine(send, len, GateIndexFilter(m_pUser->id));
  m_curSyncStateMsg = m_animationQueue.front().m_msg.state();
  m_animationQueue.pop();
  XDBG << "[料理-异步状态同步] charid" << m_pUser->id << "状态" << m_curSyncStateMsg.state() << XEND;
}

void SceneFood::addLastCooked(DWORD dwProductId, DWORD dwNum)
{
  DWORD dwCookNum = dwNum;
  for (DWORD i = 0; i < dwCookNum; i++)
  {
    m_vecLastCooked.push_back(dwProductId);
  }
  
  DWORD limit = MiscConfig::getMe().getFoodCfg().dwMaxLastCooked;
  while (m_vecLastCooked.size() > limit && !m_vecLastCooked.empty())
  {
    m_vecLastCooked.erase(m_vecLastCooked.begin());
  }
  if (m_vecLastCooked.empty())
    return;
  
  //ntf client
  Cmd::UpdateFoodInfo updateCmd;

  dwCookNum = dwCookNum > limit ? limit : dwCookNum;
  for (DWORD i = 0; i < dwCookNum; i++)
  {
    updateCmd.add_last_cooked_foods(dwProductId);
  }
  
  PROTOBUF(updateCmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

DWORD SceneFood::getLvUpCnt(Cmd::EFoodDataType type, DWORD lv)
{
  auto it = m_mapFoodManualData.find(type);
  if (it == m_mapFoodManualData.end())
    return 0;
  
  DWORD cnt = 0;
  for (auto&v : it->second.vecSubData)
  {
    if (v.dwLevel >= lv)
      cnt++;
  }
  return cnt;
}

void SceneFood::refreshSatiety()
{
  /* f04280 去掉饱食度概念
  DWORD old = getSatiety();
  if (m_dwSatiety == old)
    return;

  m_pUser->setDataMark(EUSERDATATYPE_SATIETY);
  */
}

void SceneFood::onPutFood(FoodNpc* pNpc)
{
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return;
  if (pNpc == nullptr)
    return;
  m_pUser->getUserPet().onSeeFood(pNpc);

  xSceneEntrySet set;
  pScene->getEntryListInBlock(SCENE_ENTRY_USER, pNpc->getPos(), 10, set);
  for (auto &s : set)
  {
    if (s == m_pUser)
      continue;
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user && pNpc->checkCanEat(user))
      user->getUserPet().onSeeFood(pNpc);
  }
}

void SceneFood::resetFoodBook()
{
  if (m_dwCookerLv == 0)
    return;

  const DWORD MAX_BOOK_LV = 10;
  const DWORD arrBooks[] = {
    ITEM_FOODBOOK_LV_1,
    ITEM_FOODBOOK_LV_2,
    ITEM_FOODBOOK_LV_3,
    ITEM_FOODBOOK_LV_4,
    ITEM_FOODBOOK_LV_5,
    ITEM_FOODBOOK_LV_6,
    ITEM_FOODBOOK_LV_7,
    ITEM_FOODBOOK_LV_8,
    ITEM_FOODBOOK_LV_9,
    ITEM_FOODBOOK_LV_10,
  };

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[料理-料理书]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置料理书失败,未发现" << EPACKTYPE_MAIN << XEND;
    return;
  }

  if (m_dwCookerLv > MAX_BOOK_LV)
  {
    XERR << "[料理-料理书]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置料理书失败,厨师等级超过" << MAX_BOOK_LV << "级" << XEND;
    return;
  }

  for (DWORD lv = 0; lv < 10; ++lv)
  {
    if (lv + 1 < m_dwCookerLv)
    {
      if (pMainPack->checkItemCount(arrBooks[lv]) == true)
      {
        pMainPack->reduceItem(arrBooks[lv], ESOURCE_COOK_FOOD);
        XLOG << "[料理-料理书]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "重置料理书, lv :" << lv + 1 << "低于厨师等级" << m_dwCookerLv << "itemid :" << arrBooks[lv] << "从" << EPACKTYPE_MAIN << "移除" << XEND;
      }
    }
  }

  if (pMainPack->checkItemCount(arrBooks[m_dwCookerLv - 1]) == false)
  {
    ItemData oData;
    oData.mutable_base()->set_id(arrBooks[m_dwCookerLv - 1]);
    oData.mutable_base()->set_count(1);
    if (pMainPack->addItem(oData, EPACKMETHOD_NOCHECK) == false)
    {
      XERR << "[料理-料理书]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "重置料理书失败,厨师等级" << m_dwCookerLv << "添加对应料理书" << arrBooks[m_dwCookerLv - 1] << "失败" << XEND;
    }
    XLOG << "[料理-料理书]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "重置料理书,厨师等级" << m_dwCookerLv << "添加对应料理书" << arrBooks[m_dwCookerLv - 1] << XEND;
  }
}

void SceneFood::onPetEatFood(FoodNpc* pNpc, SceneNpc* pPet)
{
  if (pNpc == nullptr || pPet == nullptr)
    return;

  DWORD foodid = pNpc->getFoodId();
  DWORD foodnum = pNpc->getFoodNum();
  if (foodnum == 0)
    return;

  const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(foodid);
  if (!pFoodCfg)
    return; 

  for (auto &v : pFoodCfg->m_vecBuff)
  {
    pPet->getBuff().add(v);
  }

  pNpc->setFoodNum(foodnum - 1);

  SScenePetData* pPetData = m_pUser->getUserPet().getPetData(pPet->getNpcID());
  if (pPetData != nullptr)
    pPetData->feed();
}


void SceneFood::onLeaveTeam()
{
  if (m_qwEatingFoodNpc == 0)
    return;
  FoodNpc* pNpc = dynamic_cast<FoodNpc*>(SceneNpcManager::getMe().getNpcByTempID(m_qwEatingFoodNpc));
  if (!pNpc)
    return;
  if (!pNpc->checkCanEat(m_pUser))
  {
    stopEat(pNpc, false);
    XLOG << "[料理-离开队伍打断吃料理] charid" << m_pUser->id << "npcguid" << m_qwEatingFoodNpc << XEND;
  }
}

void SceneFood::addJoyOnEat(DWORD dwFoodId, DWORD dwFoodNum)
{
  DWORD dwAddValue = MiscConfig::getMe().getFoodValue(dwFoodId);
  if(dwAddValue != 0)
  {
    m_pUser->getUserSceneData().addJoyValue(JOY_ACTIVITY_FOOD, dwAddValue * dwFoodNum);
  }
}

DWORD SceneFood::getUnlockedFood() const
{
  DWORD dwNum = 0;
  for(auto &it : m_mapFoodManualData)
  {
    if (it.first != EFOODDATATYPE_FOODCOOK && it.first != EFOODDATATYPE_FOODTASTE)
      continue;
    for(auto &iter : it.second.vecSubData)
    {
      if(iter.status == EFOODSTATUS_CLICKED)
        ++dwNum;
    }
  }

  return dwNum;
}

bool SceneFood::checkFoodLimitNum(DWORD dwItemid)
{
  const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(dwItemid);
  if (!pFoodCfg)
    return false;

  if(pFoodCfg->m_dwLimitNum == 0)
    return true;

  DWORD curTime = xTime::getCurSec();
  if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_DAY)
  {
    auto it = m_mapFoodlimit.find(dwItemid);
    if(it != m_mapFoodlimit.end())
    {
      bool bSameDay = xTime::isSameDay(it->second.time() - 5*HOUR_T, curTime - 5*HOUR_T);
      if(bSameDay == true && it->second.num() >= pFoodCfg->m_dwLimitNum)
      {
        MsgManager::sendMsg(m_pUser->id, 25806, MsgParams(pFoodCfg->m_strName));
        return false;
      }
    }
  }
  else if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_WEEK)
  {
  }
  else if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_MONTH)
  {
  }

  return true;
}

void SceneFood::addFoodLimitNum(DWORD dwItemid)
{
  const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(dwItemid);
  if (!pFoodCfg)
    return;

  if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_MIN)
    return;

  DWORD curTime = xTime::getCurSec();
  if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_DAY)
  {
    auto it = m_mapFoodlimit.find(dwItemid);
    if(it != m_mapFoodlimit.end())
    {
      bool bSameDay = xTime::isSameDay(it->second.time() - 5*HOUR_T, curTime - 5*HOUR_T);
      if(bSameDay == true)
        it->second.set_num(it->second.num() + 1);
      else
      {
        it->second.set_num(1);
        it->second.set_time(curTime);
      }
    }
    else
    {
      FoodLimitInfo info;
      info.set_itemid(dwItemid);
      info.set_time(curTime);
      info.set_num(1);
      m_mapFoodlimit.emplace(dwItemid, info);
    }
  }
  else if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_WEEK)
  {
  }
  else if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_MONTH)
  {
  }
}

void SceneFood::checkResetLimitNum()
{
  DWORD curTime = xTime::getCurSec();
  auto it = m_mapFoodlimit.begin();
  for(; it != m_mapFoodlimit.end(); )
  {
    const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(it->first);
    if (!pFoodCfg || pFoodCfg->m_eLimitType == ELIMIT_CYCLE_MIN)
    {
      m_mapFoodlimit.erase(it++);
      continue;
    }

    if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_DAY)
    {
      bool bSameDay = xTime::isSameDay(it->second.time() - 5*HOUR_T, curTime - 5*HOUR_T);
      if(bSameDay == false)
      {
        m_mapFoodlimit.erase(it++);
        continue;
      }
    }
    else if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_WEEK)
    {
    }
    else if(pFoodCfg->m_eLimitType == ELIMIT_CYCLE_MONTH)
    {
    }

    ++it;
  }
}
