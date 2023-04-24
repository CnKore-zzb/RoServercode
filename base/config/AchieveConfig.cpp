#include "AchieveConfig.h"
#include "ItemConfig.h"
#include "xLuaTable.h"
#include "MiscConfig.h"
#include "QuestConfig.h"

bool SAchieveCFG::checkEnable(EProfession eProfession, EAchieveCond eCond, const TVecQWORD& vecParam /*= TVecQWORD{}*/) const
{
  if (stCondition.setProfession.empty() == false && stCondition.setProfession.find(eProfession) == stCondition.setProfession.end())
    return false;
  if (stCondition.eCond != eCond)
    return false;
  if (stCondition.eCond == EACHIEVECOND_USEITEM || stCondition.eCond == EACHIEVECOND_ITEM || stCondition.eCond == EACHIEVECOND_ITEM_GET || stCondition.eCond == EACHIEVECOND_USER_DAMAGE ||
      stCondition.eCond == EACHIEVECOND_PRODUCE_EQUIP || stCondition.eCond == EACHIEVECOND_EATFOODID)
  {
    if (vecParam.empty() == true || stCondition.vecParams.size() < 2)
      return false;
    return stCondition.vecParams[0] == 0 ? true : stCondition.vecParams[0] == vecParam[0];
  }
  else if (stCondition.eCond == EACHIEVECOND_KILL_MONSTER || stCondition.eCond == EACHIEVECOND_BATTLE)
  {
    if (stCondition.vecParams.empty() == true || vecParam.empty() == true)
      return false;
    if (stCondition.vecParams[0] != 0 && stCondition.vecParams[0] != vecParam[0])
      return false;
    if (stCondition.vecParams[1] != 0 && stCondition.vecParams[1] != vecParam[1])
      return false;
  }
  else if (stCondition.eCond == EACHIEVECOND_QUEST_SUBMIT || stCondition.eCond == EACHIEVECOND_ACHIEVE_FINISH)
  {
    if (vecParam.empty() == true)
      return false;
    for (auto &v : vecParam)
    {
      auto item = find(stCondition.vecParams.begin(), stCondition.vecParams.end(), v);
      if (item == stCondition.vecParams.end())
        return false;
    }
  }
  else if (stCondition.eCond == EACHIEVECOND_USER_CHAT)
  {
    if (vecParam.size() != 2 || stCondition.vecParams.size() != 3)
      return false;
    if (vecParam[0] != stCondition.vecParams[0])
      return false;
    if (stCondition.vecParams[1] != 0 && vecParam[1] != 0)
      return false;
  }
  else if (stCondition.eCond == EACHIEVECOND_PET_FRIENDLV_NUM)
  {
    if (vecParam.size() != 1 || stCondition.vecParams.size() != 2)
      return false;
    return vecParam[0] >= stCondition.vecParams[0];
  }

  return true;
}

bool SAchieveCFG::checkSource(DWORD dwSource) const
{
  switch (stCondition.eCond)
  {
    case EACHIEVECOND_ITEM_GET:
      {
        if(stCondition.vecParams.size() > 2)
        {
          for(DWORD i=2; i < stCondition.vecParams.size(); i++)
          {
            if(dwSource == stCondition.vecParams[i])
              return true;
          }

          return false;
        }
      }
      break;
    default:
      break;
  }

  return true;
}

bool SAchieveCFG::checkFinish(const TVecQWORD& vecParam /*= TVecQWORD{}*/, DWORD dwProcess /*= 0*/) const
{
  bool bEnable = true;
  switch (stCondition.eCond)
  {
    case EACHIEVECOND_LEVELUP:
    case EACHIEVECOND_ADDFRIEND:
    case EACHIEVECOND_EMOJI:
    case EACHIEVECOND_PHOTO:
    case EACHIEVECOND_SCENERY_COUNT:
    case EACHIEVECOND_MONSTER_PHOTO:
    case EACHIEVECOND_GHOST_PHOTO:
    case EACHIEVECOND_NPC_COUNT:
    case EACHIEVECOND_BATTLE_TIME:
    case EACHIEVECOND_COMPOSE:
    case EACHIEVECOND_REFINE_EQUIP:
    case EACHIEVECOND_REFINE_WEAPON:
    case EACHIEVECOND_STRENGTH:
    case EACHIEVECOND_ENCHANT:
    case EACHIEVECOND_EQUIP_UPGRADE:
    case EACHIEVECOND_GET_ITEM:
    case EACHIEVECOND_WANTEDQUEST:
    case EACHIEVECOND_REPAIR_SEAL:
    case EACHIEVECOND_KPL_TRANS:
    case EACHIEVECOND_KPL_CONSUME:
    case EACHIEVECOND_VEHICLE:
    case EACHIEVECOND_MONEY_TRADE:
    case EACHIEVECOND_MONEY_TRADE_CONSUME:
    case EACHIEVECOND_MANUAL_EQUIP:
    case EACHIEVECOND_USER_BODY:
    case EACHIEVECOND_SCENERY:
    case EACHIEVECOND_USER_TRANSFER:
    case EACHIEVECOND_QUEST:
    case EACHIEVECOND_MONEY_SHOP_BUY:
    case EACHIEVECOND_MONEY_SHOP_SELL:
    case EACHIEVECOND_MONEY_TRADE_BUY:
    case EACHIEVECOND_MONEY_TRADE_ONCEBUY:
    case EACHIEVECOND_MONEY_TRADE_SELL:
    case EACHIEVECOND_MONEY_TRADE_ONCESELL:
    case EACHIEVECOND_TRADE_RECORD:
    case EACHIEVECOND_MONEY_GET:
    case EACHIEVECOND_MONEY_CHARGE:
    case EACHIEVECOND_HELP_QUEST:
    case EACHIEVECOND_TRAVEL:
    case EACHIEVECOND_PET_CAPTURE_SUCCESS:
    case EACHIEVECOND_PET_CAPTURE_FAIL:
    case EACHIEVECOND_PET_BASELV:
    case EACHIEVECOND_PET_FEED:
    case EACHIEVECOND_PET_TOUCH:
    case EACHIEVECOND_PET_GIFT:
    case EACHIEVECOND_PET_HANDWALK:
    case EACHIEVECOND_PET_TIME:
    case EACHIEVECOND_PET_EQUIP:
    case EACHIEVECOND_PET_FRIENDLV:
    case EACHIEVECOND_PET_DEAD:
    case EACHIEVECOND_COOKERLV:
    case EACHIEVECOND_FOODMATERIALLV:
    case EACHIEVECOND_TASTERLV:
    case EACHIEVECOND_COSTSAVEHP:
    case EACHIEVECOND_COSTSAVESP:
    case EACHIEVECOND_PET_ADVENTURE_COUNT:
    case EACHIEVECOND_PET_ADVENTURE_FINISH:
    case EACHIEVECOND_TUTOR_GUIDE:
    case EACHIEVECOND_TUTOR_STUDENT_GRADUATION:
    case EACHIEVECOND_GOLD_APPLE_GAME:
    case EACHIEVECOND_JOIN_POLLY:
    case EACHIEVECOND_GOLD_APPLE_TOTAL:
    case EACHIEVECOND_DEF_PRAY_LV:
    case EACHIEVECOND_ATK_PRAY_LV:
    case EACHIEVECOND_ELEM_PRAY_LV:
    case EACHIEVECOND_PRAY_CARD_USE:
    case EACHIEVECOND_WEDDING_PROPOSE:
    case EACHIEVECOND_WEDDING_RESERVE:
    case EACHIEVECOND_WEDDING_PHOTO:
    case EACHIEVECOND_WEDDING_JOINCEREMONY:
    case EACHIEVECOND_WEDDING_CEREMONY:
    case EACHIEVECOND_WEDDING_DAY:
    case EACHIEVECOND_PET_COMPOSE:
      if (stCondition.vecParams.empty() == true)
      {
        bEnable = false;
        break;
      }
      bEnable = dwProcess >= stCondition.vecParams[0];
      break;
    case EACHIEVECOND_PLAYMUSIC:
    case EACHIEVECOND_FERRISWHEEL:
    case EACHIEVECOND_EXPRESSION:
    case EACHIEVECOND_USER_PORTRAIT:
    case EACHIEVECOND_PHOTO_MONSTER:
    case EACHIEVECOND_KILL_MVP:
    case EACHIEVECOND_HAND:
    case EACHIEVECOND_USER_RUNE:
    case EACHIEVECOND_CAT:
    case EACHIEVECOND_USER_DEAD:
    case EACHIEVECOND_USER_DAMAGE:
    case EACHIEVECOND_USER_HAIR:
    case EACHIEVECOND_REFINE_FAIL:
    case EACHIEVECOND_ITEM_GET:
    case EACHIEVECOND_TOWER_PASS:
    case EACHIEVECOND_COOKFOOD:
    case EACHIEVECOND_EATFOOD:
    case EACHIEVECOND_FOODCOOKLV:
    case EACHIEVECOND_COLLECTION:
    case EACHIEVECOND_EATFOODID:
    case EACHIEVECOND_PET_FRIENDLV_NUM:
      if (stCondition.vecParams.size() != 2)
      {
        bEnable = false;
        break;
      }
      bEnable = dwProcess >= stCondition.vecParams[1];
      break;
    case EACHIEVECOND_CREATEGUILD:
    case EACHIEVECOND_ENTERGUILD:
    case EACHIEVECOND_SEAT:
      break;
    case EACHIEVECOND_MAPMOVE:
    case EACHIEVECOND_USER_TITLE:
    case EACHIEVECOND_PHOTO_USER:
      if (vecParam.empty() == true || stCondition.vecParams.empty() == true)
      {
        bEnable = false;
        break;
      }
      bEnable = vecParam[0] == stCondition.vecParams[0];
      break;
    case EACHIEVECOND_USEITEM:
    case EACHIEVECOND_ITEM:
    case EACHIEVECOND_USE_SKILL:
    case EACHIEVECOND_PRODUCE_EQUIP:
      if (vecParam.empty() == true || stCondition.vecParams.size() != 2)
      {
        bEnable = false;
        break;
      }
      if (stCondition.vecParams[0] != 0 && stCondition.vecParams[0] != vecParam[0])
      {
        bEnable = false;
        break;
      }
      bEnable = dwProcess >= stCondition.vecParams[1];
      break;
    case EACHIEVECOND_KILL_MONSTER:
    case EACHIEVECOND_BATTLE:
      if (vecParam.size() != 2 || stCondition.vecParams.size() != 3)
      {
        bEnable = false;
        break;
      }
      if (stCondition.vecParams[0] != 0 && stCondition.vecParams[0] != vecParam[0])
      {
        bEnable = false;
        break;
      }
      if (stCondition.vecParams[1] != 0 && stCondition.vecParams[1] != vecParam[1])
      {
        bEnable = false;
        break;
      }
      bEnable = dwProcess >= stCondition.vecParams[2];
      break;
    case EACHIEVECOND_QUEST_SUBMIT:
    case EACHIEVECOND_ACHIEVE_FINISH:
      bEnable = dwProcess >= stCondition.vecParams.size();
      break;
    case EACHIEVECOND_USER_CHAT:
      if (vecParam.size() != 2 || stCondition.vecParams.size() != 3)
      {
        bEnable = false;
        break;
      }
      if (vecParam[0] != stCondition.vecParams[0])
      {
        bEnable = false;
        break;
      }
      if (stCondition.vecParams[1] != 0 && vecParam[1] != 0)
      {
        bEnable = false;
        break;
      }
      bEnable = dwProcess >= stCondition.vecParams[2];
      break;
    case EACHIEVECOND_USER_ATTR:
    case EACHIEVECOND_DOJO:
    case EACHIEVECOND_PVP:
    case EACHIEVECOND_MONSTER_DRAW:
    case EACHIEVECOND_MANUAL:
      if (stCondition.vecParams.size() != 3)
      {
        bEnable = false;
        break;
      }
      bEnable = dwProcess >= stCondition.vecParams[2];
      break;
    case EACHIEVECOND_PHOTO_MAN:
      if (stCondition.vecParams.size() != 4)
      {
        bEnable = false;
        break;
      }
      bEnable = dwProcess >= stCondition.vecParams[3];
      break;
    case EACHIEVECOND_WEDDING_PURCHASE:
    case EACHIEVECOND_WEDDING_CARRIER:
    case EACHIEVECOND_WEDDING_DIVORCE:
    case EACHIEVECOND_PROFESSION:
      bEnable = dwProcess >= 1;
      break;
    case EACHIEVECOND_PETSKILL_ALL_LV:
      if (vecParam.size() != 1 || stCondition.vecParams.size() != 1)
      {
        bEnable = false;
        break;
      }
      bEnable = vecParam[0] >= stCondition.vecParams[0];
      break;
    case EACHIEVECOND_MIN:
    case EACHIEVECOND_MAX:
      bEnable = false;
      break;
  }
  return bEnable;
}

DWORD SAchieveCFG::getProcess() const
{
  DWORD dwProcess = 0;
  switch (stCondition.eCond)
  {
    case EACHIEVECOND_LEVELUP:
    case EACHIEVECOND_ADDFRIEND:
    case EACHIEVECOND_EMOJI:
    case EACHIEVECOND_PHOTO:
    case EACHIEVECOND_SCENERY_COUNT:
    case EACHIEVECOND_MONSTER_PHOTO:
    case EACHIEVECOND_GHOST_PHOTO:
    case EACHIEVECOND_NPC_COUNT:
    case EACHIEVECOND_BATTLE_TIME:
    case EACHIEVECOND_COMPOSE:
    case EACHIEVECOND_REFINE_EQUIP:
    case EACHIEVECOND_REFINE_WEAPON:
    case EACHIEVECOND_STRENGTH:
    case EACHIEVECOND_ENCHANT:
    case EACHIEVECOND_EQUIP_UPGRADE:
    case EACHIEVECOND_GET_ITEM:
    case EACHIEVECOND_WANTEDQUEST:
    case EACHIEVECOND_REPAIR_SEAL:
    case EACHIEVECOND_KPL_TRANS:
    case EACHIEVECOND_KPL_CONSUME:
    case EACHIEVECOND_VEHICLE:
    case EACHIEVECOND_MONEY_TRADE:
    case EACHIEVECOND_MONEY_TRADE_CONSUME:
    case EACHIEVECOND_MANUAL_EQUIP:
    case EACHIEVECOND_USER_BODY:
    case EACHIEVECOND_SCENERY:
    case EACHIEVECOND_USER_TRANSFER:
    case EACHIEVECOND_QUEST:
    case EACHIEVECOND_MONEY_SHOP_BUY:
    case EACHIEVECOND_MONEY_SHOP_SELL:
    case EACHIEVECOND_MONEY_TRADE_BUY:
    case EACHIEVECOND_MONEY_TRADE_ONCEBUY:
    case EACHIEVECOND_MONEY_TRADE_SELL:
    case EACHIEVECOND_MONEY_TRADE_ONCESELL:
    case EACHIEVECOND_TRADE_RECORD:
    case EACHIEVECOND_MONEY_GET:
    case EACHIEVECOND_MONEY_CHARGE:
    case EACHIEVECOND_HELP_QUEST:
    case EACHIEVECOND_TRAVEL:
    case EACHIEVECOND_PET_CAPTURE_SUCCESS:
    case EACHIEVECOND_PET_CAPTURE_FAIL:
    case EACHIEVECOND_PET_BASELV:
    case EACHIEVECOND_PET_FEED:
    case EACHIEVECOND_PET_TOUCH:
    case EACHIEVECOND_PET_GIFT:
    case EACHIEVECOND_PET_HANDWALK:
    case EACHIEVECOND_PET_TIME:
    case EACHIEVECOND_PET_EQUIP:
    case EACHIEVECOND_PET_FRIENDLV:
    case EACHIEVECOND_PET_DEAD:
    case EACHIEVECOND_COOKERLV:
    case EACHIEVECOND_FOODMATERIALLV:
    case EACHIEVECOND_TASTERLV:
    case EACHIEVECOND_COSTSAVEHP:
    case EACHIEVECOND_COSTSAVESP:
    case EACHIEVECOND_PET_ADVENTURE_COUNT:
    case EACHIEVECOND_PET_ADVENTURE_FINISH:
    case EACHIEVECOND_TUTOR_GUIDE:
    case EACHIEVECOND_TUTOR_STUDENT_GRADUATION:
    case EACHIEVECOND_GOLD_APPLE_GAME:
    case EACHIEVECOND_JOIN_POLLY:
    case EACHIEVECOND_GOLD_APPLE_TOTAL:
    case EACHIEVECOND_DEF_PRAY_LV:
    case EACHIEVECOND_ATK_PRAY_LV:
    case EACHIEVECOND_ELEM_PRAY_LV:
    case EACHIEVECOND_PRAY_CARD_USE:
    case EACHIEVECOND_WEDDING_PROPOSE:
    case EACHIEVECOND_WEDDING_RESERVE:
    case EACHIEVECOND_WEDDING_PHOTO:
    case EACHIEVECOND_WEDDING_JOINCEREMONY:
    case EACHIEVECOND_WEDDING_CEREMONY:
    case EACHIEVECOND_WEDDING_DAY:
    case EACHIEVECOND_PET_COMPOSE:
      if (stCondition.vecParams.empty() == true)
        break;
      dwProcess = stCondition.vecParams[0];
      break;
    case EACHIEVECOND_CREATEGUILD:
    case EACHIEVECOND_ENTERGUILD:
    case EACHIEVECOND_MAPMOVE:
    case EACHIEVECOND_USER_TITLE:
    case EACHIEVECOND_SEAT:
    case EACHIEVECOND_PHOTO_USER:
    case EACHIEVECOND_PETSKILL_ALL_LV:
      break;
    case EACHIEVECOND_USEITEM:
    case EACHIEVECOND_ITEM:
    case EACHIEVECOND_PLAYMUSIC:
    case EACHIEVECOND_FERRISWHEEL:
    case EACHIEVECOND_USE_SKILL:
    case EACHIEVECOND_EXPRESSION:
    case EACHIEVECOND_USER_PORTRAIT:
    case EACHIEVECOND_PHOTO_MONSTER:
    case EACHIEVECOND_KILL_MVP:
    case EACHIEVECOND_HAND:
    case EACHIEVECOND_USER_RUNE:
    case EACHIEVECOND_CAT:
    case EACHIEVECOND_USER_DEAD:
    case EACHIEVECOND_USER_DAMAGE:
    case EACHIEVECOND_USER_HAIR:
    case EACHIEVECOND_REFINE_FAIL:
    case EACHIEVECOND_ITEM_GET:
    case EACHIEVECOND_TOWER_PASS:
    case EACHIEVECOND_COOKFOOD:
    case EACHIEVECOND_EATFOOD:
    case EACHIEVECOND_FOODCOOKLV:
    case EACHIEVECOND_PRODUCE_EQUIP:
    case EACHIEVECOND_COLLECTION:
    case EACHIEVECOND_EATFOODID:
    case EACHIEVECOND_PET_FRIENDLV_NUM:
      if (stCondition.vecParams.size() != 2)
        break;
      dwProcess = stCondition.vecParams[1];
      break;
    case EACHIEVECOND_USER_CHAT:
    case EACHIEVECOND_KILL_MONSTER:
    case EACHIEVECOND_BATTLE:
      if (stCondition.vecParams.size() != 3)
        break;
      dwProcess = stCondition.vecParams[2];
      break;
    case EACHIEVECOND_QUEST_SUBMIT:
    case EACHIEVECOND_ACHIEVE_FINISH:
    case EACHIEVECOND_PROFESSION:
      dwProcess = stCondition.vecParams.size();
      break;
    case EACHIEVECOND_USER_ATTR:
    case EACHIEVECOND_DOJO:
    case EACHIEVECOND_PVP:
    case EACHIEVECOND_MONSTER_DRAW:
    case EACHIEVECOND_MANUAL:
      if (stCondition.vecParams.size() != 3)
        break;
      dwProcess = stCondition.vecParams[2];
      break;
    case EACHIEVECOND_PHOTO_MAN:
      if (stCondition.vecParams.size() != 4)
        break;
      dwProcess = stCondition.vecParams[3];
      break;
    case EACHIEVECOND_WEDDING_PURCHASE:
    case EACHIEVECOND_WEDDING_CARRIER:
    case EACHIEVECOND_WEDDING_DIVORCE:
      dwProcess = 1;
      break;
    case EACHIEVECOND_MIN:
    case EACHIEVECOND_MAX:
      break;
  }
  return dwProcess;
}

void SAchieveCFG::toQuest(DWORD dwIndex, DWORD dwQuestID, AchieveQuest& rQuest)
{
  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(dwQuestID);
  if (pCFG == nullptr)
    return;

  if (dwIndex / QUEST_ID_PARAM != dwQuestID / QUEST_ID_PARAM)
  {
    AchieveConfig::getMe().addAchieveQuest(pCFG->id);
    AchieveQuest* pQuest = rQuest.add_pre();
    pQuest->set_id(pCFG->id);
    pQuest->set_name(pCFG->name);
  }

  TVecDWORD vecPreIDs;
  vecPreIDs.insert(vecPreIDs.end(), pCFG->vecPreQuest.begin(), pCFG->vecPreQuest.end());
  vecPreIDs.insert(vecPreIDs.end(), pCFG->vecMustPreQuest.begin(), pCFG->vecMustPreQuest.end());

  for (auto &v : vecPreIDs)
    toQuest(dwIndex, v, mapPreQuest[dwIndex]);
}

// config
AchieveConfig::AchieveConfig()
{

}

AchieveConfig::~AchieveConfig()
{

}

bool AchieveConfig::loadConfig()
{
  bool bResult = true;
  if (loadAchievementConfig() == false)
    bResult = false;
  if (loadAchievementItemConfig() == false)
    bResult = false;

  return bResult;
}

bool AchieveConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto &m : m_mapAchieveCFG)
  {
    for (auto &s : m.second.setPreAchIDs)
    {
      if (getAchieveCFG(s) == nullptr)
      {
        XERR << "[冒险手册-成就配置] id :" << m.first << "前置成就" << s << "未在 Table_Achievement.txt 表中找到" << XEND;
        bCorrect = false;
      }
    }
  }

  for (auto &m : m_mapAchItemCFG)
  {
    if (ItemConfig::getMe().getItemCFG(m.first) == nullptr)
    {
      bCorrect = false;
      XERR << "[成就-物品配置] id :" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
    }
  }

  return bCorrect;
}

const TSetAchieveSubType& AchieveConfig::getAchSubTypeList(EAchieveType eType) const
{
  static const TSetAchieveSubType e;
  auto m = m_mapType2SubCFG.find(eType);
  if (m != m_mapType2SubCFG.end())
    return m->second;
  return e;
}

const TVecAchieveCFG& AchieveConfig::getAchieveCond(EAchieveCond eCond) const
{
  const static TVecAchieveCFG emptylist;
  auto m = m_mapCondAchCFG.find(eCond);
  if (m != m_mapCondAchCFG.end())
    return m->second;
  return emptylist;
}

const SAchieveCFG* AchieveConfig::getAchieveCFG(DWORD dwID) const
{
  auto m = m_mapAchieveCFG.find(dwID);
  if (m != m_mapAchieveCFG.end() && m->second.blInit)
    return &m->second;

  return nullptr;
}

const SAchieveItemCFG* AchieveConfig::getAchieveItemCFG(DWORD dwID) const
{
  auto m = m_mapAchItemCFG.find(dwID);
  if (m != m_mapAchItemCFG.end() && m->second.blInit)
    return &m->second;
  return nullptr;
}

EAchieveType AchieveConfig::getAchieveType(EAchieveSubType eSubType) const
{
  for (auto &m : m_mapType2SubCFG)
  {
    auto s = find(m.second.begin(), m.second.end(), eSubType);
    if (s != m.second.end())
      return m.first;
  }
  return EACHIEVETYPE_MIN;
}

bool AchieveConfig::loadAchievementConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Achievement.txt"))
  {
    XERR << "[Table_Achievement.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto &m : m_mapAchieveCFG)
    m.second.blInit = false;

  m_mapCondAchCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Achievement", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    if (MiscConfig::getMe().isForbid("Achievement", m->first))
      continue;

    DWORD dwType = m->second.getTableInt("SubGroup");
    if (dwType == 0)
      continue;

    if (dwType > EACHIEVETYPE_MIN && dwType < EACHIEVETYPE_MAX && EAchieveType_IsValid(dwType) == true)
    {
      if (m->first <= EACHIEVESUBTYPE_MIN || m->first >= EACHIEVESUBTYPE_MAX || EAchieveSubType_IsValid(m->first) == false)
      {
        bCorrect = false;
        XERR << "[冒险手册-成就配置] id :" << m->first << "不是合法的二级目录"<< XEND;
        continue;
      }
      m_mapType2SubCFG[static_cast<EAchieveType>(dwType)].insert(static_cast<EAchieveSubType>(m->first));
      continue;
    }

    if (dwType <= EACHIEVESUBTYPE_MIN || dwType >= EACHIEVESUBTYPE_MAX || EAchieveSubType_IsValid(dwType) == false)
    {
      bCorrect = false;
      XERR << "[冒险手册-成就配置] id :" << m->first << "SubGroup :" << dwType << "不是合法的二级目录"<< XEND;
      continue;
    }

    set<EProfession> setProfession;
    auto professionf = [&](const string& key, xLuaData& data)
    {
      DWORD dwProfession = data.getInt();
      if (dwProfession <= EPROFESSION_MIN || dwProfession >= EPROFESSION_MAX || EProfession_IsValid(dwProfession) == false)
      {
        bCorrect = false;
        XERR << "[冒险手册-成就配置] id :" << m->first << "profession :" << dwProfession << "不合法" << XEND;
        return;
      }
      setProfession.insert(static_cast<EProfession>(dwProfession));
    };
    m->second.getMutableData("occupation").foreach(professionf);

    EAchieveCond eCondition = getAchieveCondition(m->second.getTableString("behavior"));
    if (eCondition == EACHIEVECOND_MIN)
    {
      bCorrect = false;
      XERR << "[冒险手册-成就配置] id :" << m->first << "behavior :" << m->second.getTableString("behavior") << "不合法" << XEND;
      continue;
    }

    auto item = m_mapAchieveCFG.find(m->first);
    if (item == m_mapAchieveCFG.end())
    {
      m_mapAchieveCFG[m->first] = SAchieveCFG();
      item = m_mapAchieveCFG.find(m->first);
      if (item == m_mapAchieveCFG.end())
      {
        bCorrect = false;
        XERR << "[冒险手册-成就配置] id :" << m->first << "未成功创建" << XEND;
        continue;
      }
    }

    SAchieveCFG& rCFG = item->second;

    rCFG.dwID = m->first;
    rCFG.dwManualExp = m->second.getTableInt("RewardExp");
    rCFG.eSubType = static_cast<EAchieveSubType>(dwType);
    rCFG.stCondition.setProfession = setProfession;
    rCFG.stCondition.eCond = eCondition;
    rCFG.bVisibility = m->second.getTableInt("Visibility") == 1;

    auto pref = [&](const string& key, xLuaData& data)
    {
      rCFG.setPreAchIDs.insert(data.getInt());
    };
    m->second.getMutableData("preach").foreach(pref);

    auto paramf = [&](const string& key, xLuaData& data)
    {
      rCFG.stCondition.vecParams.push_back(data.getInt());
      if (eCondition == EACHIEVECOND_QUEST_SUBMIT)
      {
        AchieveQuest& rQuest = rCFG.mapPreQuest[data.getInt()];

        const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(data.getInt());
        if (pCFG == nullptr)
          return;
        AchieveConfig::getMe().addAchieveQuest(pCFG->id);
        rQuest.set_id(pCFG->id);
        rQuest.set_name(pCFG->name);
        rCFG.toQuest(data.getInt(), data.getInt(), rQuest);
      }
    };
    m->second.getMutableData("time").foreach(paramf);

    auto rewardf = [&](const string& key, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      combinItemInfo(rCFG.vecReward, TVecItemInfo{oItem});
    };
    m->second.getMutableData("RewardItems").foreach(rewardf);

    /*
    xLuaData& rewardfd = m->second.getMutableData("FunctionReward");
    if (rewardfd.has("type"))
    {
      EAchieveRwdFunction eRwdFun = getRewardFuction(rewardfd.getTableString("type"));
      if (eRwdFun == EACHIEVEREWARD_MIN)
      {
        XERR << "[冒险手册-成就配置] id:" << m->first << "FunctionReward, 类型配置错误" << XEND;
        bCorrect = false;
      }
      else
      {
        rCFG.stRewardFunction.eRewardFunction = eRwdFun;
        rCFG.stRewardFunction.dwParam1 = rewardfd.getTableInt("param1");
        rCFG.stRewardFunction.dwParam2 = rewardfd.getTableInt("param2");
      }
    }
    */

    rCFG.blInit = true;
  }

  for (auto &m : m_mapAchieveCFG)
  {
    m.second.eType = getAchieveType(m.second.eSubType);
    m_mapCondAchCFG[m.second.stCondition.eCond].push_back(m.second);
  }

  if (bCorrect)
    XLOG << "[冒险手册-成就配置] 成功加载Table_Achievement.txt" << XEND;
  return bCorrect;
}

bool AchieveConfig::loadAchievementItemConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_AchievementItem.txt"))
  {
    XERR << "[Table_AchievementItem.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto &m : m_mapAchItemCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_AchievementItem", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwType = m->second.getTableInt("Type");
    if (dwType <= EACHIEVEITEMTYPE_MIN || dwType >= EACHIEVEITEMTYPE_MAX)
    {
      bCorrect = false;
      XERR << "[成就-物品配置] id :" << m->first << "type :" << dwType << "不合法" << XEND;
      continue;
    }

    SAchieveItemCFG& rCFG = m_mapAchItemCFG[m->first];

    rCFG.dwItemID = m->first;
    rCFG.eType = static_cast<EAchieveItemType>(dwType);
    rCFG.blInit = true;
  }

  if (bCorrect)
    XLOG << "[成就-物品配置] 成功加载 Table_AchievementItem.txt" << XEND;
  return bCorrect;
}

EAchieveCond AchieveConfig::getAchieveCondition(const string& str)
{
  if (str == "levelup")
    return EACHIEVECOND_LEVELUP;
  else if (str == "addfriend")
    return EACHIEVECOND_ADDFRIEND;
  else if (str == "playmusic")
    return EACHIEVECOND_PLAYMUSIC;
  else if (str == "ferriswheel")
    return EACHIEVECOND_FERRISWHEEL;
  else if (str == "hand")
    return EACHIEVECOND_HAND;
  else if (str == "emoji")
    return EACHIEVECOND_EMOJI;
  else if (str == "expression")
    return EACHIEVECOND_EXPRESSION;
  else if (str == "useitem")
    return EACHIEVECOND_USEITEM;
  else if (str == "item")
    return EACHIEVECOND_ITEM;
  else if (str == "establishguild")
    return EACHIEVECOND_CREATEGUILD;
  else if (str == "joinguild")
    return EACHIEVECOND_ENTERGUILD;
  else if (str == "mapmove")
    return EACHIEVECOND_MAPMOVE;
  else if (str == "film")
    return EACHIEVECOND_SCENERY_COUNT;
  else if (str == "scenery")
    return EACHIEVECOND_SCENERY;
  else if (str == "filmnum")
    return EACHIEVECOND_PHOTO;
  else if (str == "photoman")
    return EACHIEVECOND_PHOTO_MAN;
  else if (str == "photomonster")
    return EACHIEVECOND_PHOTO_MONSTER;
  else if (str == "monsterfilm")
    return EACHIEVECOND_MONSTER_PHOTO;
  else if (str == "ghostfilm")
    return EACHIEVECOND_GHOST_PHOTO;
  else if (str == "visitnpc")
    return EACHIEVECOND_NPC_COUNT;
  else if (str == "killmonster")
    return EACHIEVECOND_KILL_MONSTER;
  else if (str == "useskill")
    return EACHIEVECOND_USE_SKILL;
  else if (str == "fighttime")
    return EACHIEVECOND_BATTLE_TIME;
  else if (str == "equipment")
    return EACHIEVECOND_COMPOSE;
  else if (str == "refiningequipment")
    return EACHIEVECOND_REFINE_EQUIP;
  else if (str == "weaponsrefining")
    return EACHIEVECOND_REFINE_WEAPON;
  else if (str == "refiningfailure")
    return EACHIEVECOND_REFINE_FAIL;
  else if (str == "strengthenequipment")
    return EACHIEVECOND_STRENGTH;
  else if (str == "enchantment")
    return EACHIEVECOND_ENCHANT;
  else if (str == "unlockequipment")
    return EACHIEVECOND_MANUAL_EQUIP;
  else if (str == "upgradeequipment")
    return EACHIEVECOND_EQUIP_UPGRADE;
  else if (str == "getcard")
    return EACHIEVECOND_GET_ITEM;
  else if (str == "Commissioned")
    return EACHIEVECOND_WANTEDQUEST;
  else if (str == "repairseal")
    return EACHIEVECOND_REPAIR_SEAL;
  else if (str == "endlesstower")
    return EACHIEVECOND_TOWER_PASS;
  else if (str == "transfernum")
    return EACHIEVECOND_KPL_TRANS;
  else if (str == "kpl_consume")
    return EACHIEVECOND_KPL_CONSUME;
  else if (str == "dojo")
    return EACHIEVECOND_DOJO;
  else if (str == "vehicle")
    return EACHIEVECOND_VEHICLE;
  else if (str == "tradingrevenue")
    return EACHIEVECOND_MONEY_TRADE;
  else if (str == "transactioncost")
    return EACHIEVECOND_MONEY_TRADE_CONSUME;
  else if (str == "questfinish")
    return EACHIEVECOND_QUEST_SUBMIT;
  else if (str == "achievementfinish")
    return EACHIEVECOND_ACHIEVE_FINISH;
  else if (str == "attr")
    return EACHIEVECOND_USER_ATTR;
  else if (str == "chat")
    return EACHIEVECOND_USER_CHAT;
  else if (str == "body")
    return EACHIEVECOND_USER_BODY;
  else if (str == "rune")
    return EACHIEVECOND_USER_RUNE;
  else if (str == "monster")
    return EACHIEVECOND_MONSTER_DRAW;
  else if (str == "portrait")
    return EACHIEVECOND_USER_PORTRAIT;
  else if (str == "hair")
    return EACHIEVECOND_USER_HAIR;
  else if (str == "title")
    return EACHIEVECOND_USER_TITLE;
  else if (str == "mvp")
    return EACHIEVECOND_KILL_MVP;
  else if (str == "pvp")
    return EACHIEVECOND_PVP;
  else if (str == "cat")
    return EACHIEVECOND_CAT;
  else if (str == "dead")
    return EACHIEVECOND_USER_DEAD;
  else if (str == "damage")
    return EACHIEVECOND_USER_DAMAGE;
  else if (str == "trans")
    return EACHIEVECOND_USER_TRANSFER;
  else if (str == "manual")
    return EACHIEVECOND_MANUAL;
  else if (str == "quest")
    return EACHIEVECOND_QUEST;
  else if (str == "shop_buy")
    return EACHIEVECOND_MONEY_SHOP_BUY;
  else if (str == "shop_sell")
    return EACHIEVECOND_MONEY_SHOP_SELL;
  else if (str == "trade_buy")
    return EACHIEVECOND_MONEY_TRADE_BUY;
  else if (str == "trade_oncebuy")
    return EACHIEVECOND_MONEY_TRADE_ONCEBUY;
  else if (str == "trade_sell")
    return EACHIEVECOND_MONEY_TRADE_SELL;
  else if (str == "trade_oncesell")
    return EACHIEVECOND_MONEY_TRADE_ONCESELL;
  else if (str == "trade_record")
    return EACHIEVECOND_TRADE_RECORD;
  else if (str == "getmoney")
    return EACHIEVECOND_MONEY_GET;
  else if (str == "charge")
    return EACHIEVECOND_MONEY_CHARGE;
  else if (str == "item_get")
    return EACHIEVECOND_ITEM_GET;
  else if (str == "seat")
    return EACHIEVECOND_SEAT;
  else if (str == "help_quest")
    return EACHIEVECOND_HELP_QUEST;
  else if (str == "travel")
    return EACHIEVECOND_TRAVEL;
  else if (str == "pet_get")
    return EACHIEVECOND_PET_CAPTURE_SUCCESS;
  else if (str == "pet_run")
    return EACHIEVECOND_PET_CAPTURE_FAIL;
  else if (str == "pet_lv")
    return EACHIEVECOND_PET_BASELV;
  else if (str == "pet_feed")
    return EACHIEVECOND_PET_FEED;
  else if (str == "pet_touch")
    return EACHIEVECOND_PET_TOUCH;
  else if (str == "pet_gift")
    return EACHIEVECOND_PET_GIFT;
  else if (str == "pet_hand")
    return EACHIEVECOND_PET_HANDWALK;
  else if (str == "pet_time")
    return EACHIEVECOND_PET_TIME;
  else if (str == "pet_equip")
    return EACHIEVECOND_PET_EQUIP;
  else if (str == "pet_friend")
    return EACHIEVECOND_PET_FRIENDLV;
  else if (str == "pet_dead")
    return EACHIEVECOND_PET_DEAD;
  else if (str == "cookfood")
    return EACHIEVECOND_COOKFOOD;
  else if (str == "eatfood")
    return EACHIEVECOND_EATFOOD;
  else if (str == "foodmateriallv")
    return EACHIEVECOND_FOODMATERIALLV;
  else if (str == "cookerlv")
    return EACHIEVECOND_COOKERLV;
  else if (str == "tasterlv")
    return EACHIEVECOND_TASTERLV;
  else if (str == "costsavehp")
    return EACHIEVECOND_COSTSAVEHP;
  else if (str == "costsavesp")
    return EACHIEVECOND_COSTSAVESP;
  else if (str == "foodcooklv")
    return EACHIEVECOND_FOODCOOKLV;
  else if (str == "produce_equip")
    return EACHIEVECOND_PRODUCE_EQUIP;
  else if (str == "collection")
    return EACHIEVECOND_COLLECTION;
  else if (str == "pet_adventure_finish")
    return EACHIEVECOND_PET_ADVENTURE_FINISH;
  else if (str == "pet_adventure_count")
    return EACHIEVECOND_PET_ADVENTURE_COUNT;
  else if (str == "tutor_graduation")
    return EACHIEVECOND_TUTOR_GUIDE;
  else if (str == "student_graduation")
    return EACHIEVECOND_TUTOR_STUDENT_GRADUATION;
  else if (str == "gold_apple_game")
    return EACHIEVECOND_GOLD_APPLE_GAME;
  else if (str == "join_polly")
    return EACHIEVECOND_JOIN_POLLY;
  else if (str == "gold_apple")
    return EACHIEVECOND_GOLD_APPLE_TOTAL;
  else if (str == "pray_def_lv")
    return EACHIEVECOND_DEF_PRAY_LV;
  else if (str == "pray_atk_lv")
    return EACHIEVECOND_ATK_PRAY_LV;
  else if (str == "pray_elem_lv")
    return EACHIEVECOND_ELEM_PRAY_LV;
  else if (str == "cost_pray_card")
    return EACHIEVECOND_PRAY_CARD_USE;
  else if (str == "wedding_purchase")
    return EACHIEVECOND_WEDDING_PURCHASE;
  else if (str == "wedding_propose")
    return EACHIEVECOND_WEDDING_PROPOSE;
  else if (str == "wedding_reserver")
    return EACHIEVECOND_WEDDING_RESERVE;
  else if (str == "wedding_photo")
    return EACHIEVECOND_WEDDING_PHOTO;
  else if (str == "wedding_joinceremony")
    return EACHIEVECOND_WEDDING_JOINCEREMONY;
  else if (str == "wedding_ceremony")
    return EACHIEVECOND_WEDDING_CEREMONY;
  else if (str == "wedding_carrier")
    return EACHIEVECOND_WEDDING_CARRIER;
  else if (str == "wedding_time")
    return EACHIEVECOND_WEDDING_DAY;
  else if (str == "wedding_divorce")
    return EACHIEVECOND_WEDDING_DIVORCE;
  else if (str == "photouser")
    return EACHIEVECOND_PHOTO_USER;
  else if (str == "eatfoodid")
    return EACHIEVECOND_EATFOODID;
  else if (str == "profession")
    return EACHIEVECOND_PROFESSION;
  else if (str == "pet_compose")
    return EACHIEVECOND_PET_COMPOSE;
  else if (str == "pet_skill_max")
    return EACHIEVECOND_PETSKILL_ALL_LV;
  else if (str == "pet_friend_lv_num")
    return EACHIEVECOND_PET_FRIENDLV_NUM;
  return EACHIEVECOND_MIN;
}

/*EAchieveRwdFunction AchieveConfig::getRewardFuction(const string& str)
{
  if (str == "addShopCnt")
    return EACHIEVEREWARD_ADDSHOPCNT;

  return EACHIEVEREWARD_MIN;
}

*/
