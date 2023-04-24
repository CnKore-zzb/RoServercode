#include "AchieveProcess.h"
#include "AchieveConfig.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"

AchieveProcess::AchieveProcess()
{
/*enum EAchieveCondMvpType
{
  EACHIEVECONDMVPTYPE_MIN = 0,
  EACHIEVECONDMVPTYPE_TIME,
  EACHIEVECONDMVPTYPE_MVP,
  EACHIEVECONDMVPTYPE_LAST,
  EACHIEVECONDMVPTYPE_HEAL,
  EACHIEVECONDMVPTYPE_DAMAGE,
  EACHIEVECONDMVPTYPE_FIRST,
  EACHIEVECONDMVPTYPE_TARGET,
};*/
}

AchieveProcess::~AchieveProcess()
{

}

DWORD AchieveProcess::getProcess(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParam, DWORD dwOri, DWORD dwAdd)
{
  switch (rCFG.stCondition.eCond)
  {
    case EACHIEVECOND_LEVELUP:
      return process_level(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_USER_ATTR:
      return process_attr(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_ADDFRIEND:
      return process_friend(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PLAYMUSIC:
      return process_music(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_FERRISWHEEL:
      return process_ferriswheel(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_EMOJI:
      return process_emoji(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_HAND:
      return process_hand(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_EXPRESSION:
      return process_expression(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PHOTO_MAN:
      return process_photoman(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PHOTO_MONSTER:
      return process_photomonster(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_USER_BODY:
      return process_body(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_USER_RUNE:
      return process_rune(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_USER_PORTRAIT:
      return process_portrait(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_USER_HAIR:
      return process_hair(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_DOJO:
      return process_dojo(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_ITEM:
      return process_item(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_MONSTER_PHOTO:
      return process_monsterphoto(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_NPC_COUNT:
      return process_npccount(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_SCENERY_COUNT:
      return process_scenerycount(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_SCENERY:
      return process_scenery(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_MONSTER_DRAW:
      return process_monsterdraw(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_KILL_MVP:
      return process_mvp(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PVP:
      return process_pvp(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_CAT:
      return process_cat(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_USER_DEAD:
      return process_dead(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_MANUAL:
      return process_manual(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_QUEST:
      return process_quest(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_MONEY_CHARGE:
      return process_charge(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_REFINE_FAIL:
      return process_refine(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_TOWER_PASS:
      return process_tower(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_SEAT:
      return process_seat(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_TRAVEL:
      return process_travel(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_COOKFOOD:
      return process_cookfood(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_EATFOOD:
      return process_eatfood(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_FOODMATERIALLV:
      return process_foodmateriallvup(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_COOKERLV:
      return process_cookerlvup(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_TASTERLV:
      return process_tasterlvup(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_COSTSAVEHP:
      return process_costsavehp(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_COSTSAVESP:
      return process_costsavesp(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_FOODCOOKLV:
      return process_foodcooklv(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_COLLECTION:
      return process_collection(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PET_ADVENTURE_FINISH:
      return process_petadventurefinish(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PET_BASELV:
      return process_petbaselevel(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PET_FRIENDLV:
      return process_petfriendlevel(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PET_EQUIP:
      return process_petequip(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_TUTOR_GUIDE:
      return process_tutor(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_DEF_PRAY_LV:
      return process_defpraylv(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_ATK_PRAY_LV:
      return process_atkpraylv(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_ELEM_PRAY_LV:
      return process_elempraylv(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_WEDDING_PURCHASE:
    case EACHIEVECOND_WEDDING_PROPOSE:
    case EACHIEVECOND_WEDDING_RESERVE:
    case EACHIEVECOND_WEDDING_PHOTO:
    case EACHIEVECOND_WEDDING_CEREMONY:
    case EACHIEVECOND_WEDDING_CARRIER:
    case EACHIEVECOND_WEDDING_DAY:
    case EACHIEVECOND_WEDDING_DIVORCE:
      return process_wedding(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_PROFESSION:
      return process_profession(pUser, rCFG, vecParam, dwOri, dwAdd);
    case EACHIEVECOND_QUEST_SUBMIT:
      return process_quest_submit(pUser, rCFG, vecParam, dwOri, dwAdd);
    default:
      break;
  }

  return dwOri + dwAdd;
}

void achieve_debug(const TVecQWORD& vecParams)
{
  for (auto &v : vecParams)
    XDBG << v;
  XDBG << XEND;
}

DWORD AchieveProcess::process_level(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getLevel();
}

DWORD AchieveProcess::process_attr(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr)
    return 0;
  if (rCFG.stCondition.vecParams.size() != 3)
    return 0;

  EAttrType eType = static_cast<EAttrType>(rCFG.stCondition.vecParams[1]);
  if (eType <= EATTRTYPE_MIN || eType >= EATTRTYPE_MAX || EAttrType_IsValid(eType) == false)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0)
  {
    SceneFighter* pFighter = pUser->getFighter();
    if (pFighter == nullptr)
      return false;
    return pFighter->getAttrPoint(eType);
  }
  else if (rCFG.stCondition.vecParams[0] == 1)
  {
    return pUser->getAttr(eType);
  }

  return 0;
}

DWORD AchieveProcess::process_friend(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getSocial().getRelationCount(ESOCIALRELATION_FRIEND);
}

DWORD AchieveProcess::process_music(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0)
    return dwOri + dwAdd;
  if (rCFG.stCondition.vecParams[0] == 1)
    return pUser->getShare().getMaxCount(ESHAREDATATYPE_MOST_MUSICCD);

  return 0;
}

DWORD AchieveProcess::process_ferriswheel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return false;

  if (rCFG.stCondition.vecParams[0] == 0)
    return pUser->getShare().getMaxKey(ESHAREDATATYPE_MOST_WHELL);
  if (rCFG.stCondition.vecParams[0] == 1)
    return pUser->getShare().getMaxCount(ESHAREDATATYPE_MOST_WHELL);
  if (rCFG.stCondition.vecParams[0] == 2)
    return dwOri + dwAdd;
  return 0;
}

DWORD AchieveProcess::process_hand(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return false;

  if (rCFG.stCondition.vecParams[0] == 0)
    return dwOri + dwAdd;
  if (rCFG.stCondition.vecParams[0] == 1)
    return dwAdd > MiscConfig::getMe().getAchieveCFG().dwHandTimeLimit ? pUser->getShare().getMaxKey(ESHAREDATATYPE_MOST_HAND_TIMECOUNT) : 0;

  return 0;
}

DWORD AchieveProcess::process_emoji(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return false;
  return pUser->getUserSceneData().getExpressionCount();
}

DWORD AchieveProcess::process_expression(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return false;
  if (rCFG.stCondition.vecParams[0] == 0)
    return dwOri + dwAdd;
  else if (rCFG.stCondition.vecParams[0] == 1)
    return pUser->getShare().getMaxCount(ESHAREDATATYPE_MOST_EXPRESSION);
  return 0;
}

DWORD AchieveProcess::process_photoman(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 4)
    return 0;

  set<SceneUser*> setUsers;
  for (auto &v : vecParams)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(v);
    if (pUser == nullptr)
      continue;
    setUsers.insert(pUser);
  }

  if (rCFG.stCondition.vecParams[0] == 1)
  {
    EProfession eProfession = EPROFESSION_MIN;
    for (auto &s : setUsers)
    {
      if (eProfession == EPROFESSION_MIN)
        eProfession = s->getProfession();
      else
      {
        if (eProfession != s->getProfession())
          return 0;
      }
    }
  }
  else if (rCFG.stCondition.vecParams[0] == 2)
  {
    TSetDWORD setPros;
    for (auto &s : setUsers)
    {
      DWORD dwPro = s->getProfession() * 10 + s->getUserSceneData().getGender();
      auto pro = setPros.find(dwPro);
      if (pro != setPros.end())
        return 0;
      setPros.insert(dwPro);
    }
  }

  if (rCFG.stCondition.vecParams[1] != 0)
  {
    DWORD dwNow = xTime::getCurSec();
    TSetDWORD setExpression;
    for (auto &s : setUsers)
    {
      if (s->getExpression() == 0 || dwNow - s->getExpressionTime() > 3)
        continue;

      if (rCFG.stCondition.vecParams[1] == 1)
      {
        auto item = setExpression.find(s->getExpression());
        if (setExpression.empty() == false && item == setExpression.end())
          return 0;
        setExpression.insert(s->getExpression());
      }
      else if (rCFG.stCondition.vecParams[1] == 2)
      {
        auto item = setExpression.find(s->getExpression());
        if (item != setExpression.end())
          return 0;
        setExpression.insert(s->getExpression());
      }
    }
    if (setExpression.empty() == true)
      return 0;
  }

  if (rCFG.stCondition.vecParams[2] != 0)
  {
    TSetDWORD setAction;
    for (auto &s : setUsers)
    {
      if (s->getAction() == 0)
        continue;
      if (rCFG.stCondition.vecParams[2] == 1)
      {
        auto item = setAction.find(s->getAction());
        if (setAction.empty() == false && item == setAction.end())
          return 0;
        setAction.insert(s->getAction());
      }
      else if (rCFG.stCondition.vecParams[2] == 2)
      {
        auto item = setAction.find(s->getAction());
        if (item != setAction.end())
          return 0;
        setAction.insert(s->getAction());
      }
    }
    if (setAction.empty() == true)
      return 0;
  }

  return setUsers.size();
}

DWORD AchieveProcess::process_photomonster(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return 0;

  set<SceneNpc*> setMonster;
  for (auto &v : vecParams)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(v);
    if (pNpc != nullptr && pNpc->isMonster() == true)
      setMonster.insert(pNpc);
  }

  if (rCFG.stCondition.vecParams[0] == 0)
  {
    return setMonster.size();
  }
  else if (rCFG.stCondition.vecParams[0] == 1)
  {
    map<DWORD, DWORD> mapCount;
    for (auto &s : setMonster)
      mapCount[s->getNpcID()]++;

    DWORD dwProcess = 0;
    for (auto &m : mapCount)
    {
      if (m.second > dwProcess)
        dwProcess = m.second;
    }
    return dwProcess;
  }
  else if (rCFG.stCondition.vecParams[0] == 2)
  {
    TSetDWORD setIDs;
    for (auto &s : setMonster)
      setIDs.insert(s->getNpcID());
    return setIDs.size();
  }

  return 0;
}

DWORD AchieveProcess::process_body(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  GTeam& rTeam = pUser->getTeam();
  if (rTeam.getTeamID() == 0)
    return 0;
  DWORD dwCount = 0;
  const TMapGTeamMember& mapMember = rTeam.getTeamMemberList();
  for (auto &m : mapMember)
  {
    SceneUser* pTeamUser = SceneUserManager::getMe().getUserByID(m.first);
    if (pTeamUser != nullptr && pTeamUser->getUserSceneData().getBodyScale(true) > 100)
      ++dwCount;
  }
  return dwCount;
}

DWORD AchieveProcess::process_rune(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return 0;

  DWORD dwTotal = 0;
  DWORD dwSpecial = 0;
  pUser->getAstrolabes().collectRuneCount(dwTotal, dwSpecial);
  if (rCFG.stCondition.vecParams[0] == 0)
    return dwTotal;
  else if (rCFG.stCondition.vecParams[0] == 1)
    return dwSpecial;
  return 0;
}

DWORD AchieveProcess::process_portrait(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return 0;

  if (rCFG.stCondition.vecParams[0] <= EPORTRAITTYPE_MIN || rCFG.stCondition.vecParams[0] >= EPORTRAITTYPE_MAX)
    return 0;
  return pUser->getPortrait().getPortraitCount(static_cast<EPortraitType>(rCFG.stCondition.vecParams[0]));
}

DWORD AchieveProcess::process_hair(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2 || vecParams.empty() == true)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0 && vecParams[0] == 0)
    return pUser->getHairInfo().getHairCount();
  else if (rCFG.stCondition.vecParams[0] == 1 && vecParams[0] == 1)
    return dwOri + dwAdd;
  return dwOri;
}

DWORD AchieveProcess::process_dojo(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 3)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0)
  {
    if (vecParams.empty() == true || vecParams[0] != 1)
      return dwOri;
    return dwOri + dwAdd;
  }
  if (rCFG.stCondition.vecParams[0] == 1)
  {
    return pUser->getDojo().getCompleteCount();
  }
  if (rCFG.stCondition.vecParams[0] == 2)
  {
    return pUser->getDojo().isGroupPassed(rCFG.stCondition.vecParams[1]) == true ? 1 : 0;
  }

  return 0;
}

DWORD AchieveProcess::process_item(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return 0;
  return pUser->getSpEffect().whoLinedMeCnt();
  //return pUser->getBeUsedItemCount(vecParams[0]);
}

DWORD AchieveProcess::process_monsterphoto(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getManual().getMonsterPhotoNum();
}

DWORD AchieveProcess::process_npccount(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true || vecParams.empty() == true)
    return 0;
  return pUser->getManual().getNumByStatus(static_cast<EManualType>(vecParams[0]), EMANUALSTATUS_UNLOCK_STEP);
}

DWORD AchieveProcess::process_scenerycount(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true || vecParams.empty() == true)
    return 0;
  return pUser->getManual().getNumByStatus(static_cast<EManualType>(vecParams[0]), EMANUALSTATUS_UNLOCK_STEP);
}

DWORD AchieveProcess::process_scenery(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr)
    return 0;

  DWORD dwCount = 0;
  DWORD dwTotal = 0;
  for (auto &v : rCFG.stCondition.vecParams)
  {
    dwCount += pUser->getManual().getSceneryUnlockCount(v);
    dwTotal += TableManager::getMe().getSceneryCount(v);
  }
  return dwCount >= dwTotal ? DWORD_MAX : 0;
}

DWORD AchieveProcess::process_monsterdraw(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 3)
    return 0;

  SAchieveItem* pItem = pUser->getAchieve().getAchieveItem(rCFG.dwID);
  if (pItem == nullptr)
    return 0;

  const TSetQWORD& setLockMeIDs = pUser->getLockMeList();
  if (setLockMeIDs.size() < rCFG.stCondition.vecParams[0])
  {
    pItem->setParams(0, 0);
    //XDBG << "[成就] 数量不足" << XEND;
    return 0;
  }

  if (pItem->arrParams[0] == 0)
  {
    pItem->setParams(0, xTime::getCurSec());
    //XDBG << "[成就] 开始计时了" << XEND;
    return 0;
  }

  if (rCFG.stCondition.vecParams[1] == 0)
  {
    for (auto &s : setLockMeIDs)
    {
      if (pUser->isAttackedMe(s) == true)
      {
        pItem->setParams(0, 0);
        //XDBG << "[成就] 被打了" << XEND;
        return 0;
      }
    }
  }

  DWORD dwNow = xTime::getCurSec();
  if (pItem->getParams(0) + rCFG.stCondition.vecParams[2] > dwNow)
    return 0;

  return rCFG.stCondition.vecParams[2] + 1;
}

DWORD AchieveProcess::process_pvp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 3 || vecParams.empty() == true)
    return 0;

  DScene* pDScene = dynamic_cast<DScene*>(pUser->getScene());
  if (pDScene == nullptr || pDScene->getRaidType() != rCFG.stCondition.vecParams[1])
    return dwOri;

  if (vecParams[0] != rCFG.stCondition.vecParams[0])
    return dwOri;

  return dwOri + dwAdd;
}

DWORD AchieveProcess::process_mvp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2 || vecParams.size() != 2)
    return 0;

  SAchieveItem* pItem = pUser->getAchieve().getAchieveItem(rCFG.dwID);

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(vecParams[1]);
  if (pCFG == nullptr || pCFG->eNpcType != ENPCTYPE_MVP)
    return dwOri;

  if (vecParams[0] != rCFG.stCondition.vecParams[0])
    return dwOri;

  DWORD dwNow = xTime::getCurSec();
  switch (vecParams[0])
  {
    case EMVPSCORETYPE_DAMAGE:
      if (pItem->getParams(0) == 0)
      {
        pItem->setParams(0, dwNow);
        return 1;
      }
      else
      {
        if (pItem->getParams(0) + MiscConfig::getMe().getAchieveCFG().dwMvpTimeLimit < dwNow)
        {
          pItem->setParams(0, dwNow);
          return 1;
        }
      }
      return dwOri + dwAdd;
    case EMVPSCORETYPE_BELOCK:
    case EMVPSCORETYPE_HEAL:
    case EMVPSCORETYPE_TOPDAMAGE:
    case EMVPSCORETYPE_DEADHIT:
    case EMVPSCORETYPE_FIRSTDAM:
    case EMVPSCORETYPE_MAX:
      return dwOri + dwAdd;
  }

  return 0;
}

DWORD AchieveProcess::process_cat(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2 || vecParams.empty() == true)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0)
  {
    SAchieveItem* pItem = pUser->getAchieve().getAchieveItem(rCFG.dwID);
    if (pItem == nullptr)
      return 0;

    DWORD dwMaxExpireTime = pUser->getWeaponPet().getMaxExpireTime();
    if (dwMaxExpireTime == 0)
    {
      DWORD dwValue = pItem->getParams(0) / DAY_T;
      return dwOri > dwValue ? dwOri : dwValue;
    }

    if (vecParams[0] == 1)
    {
      DWORD dwOfflineTime = pUser->getUserSceneData().getOfflineTime();
      DWORD dwOnlineTime = pUser->getUserSceneData().getOnlineTime();
      if (dwOnlineTime < dwOfflineTime)
      {
        DWORD dwValue = pItem->getParams(0) / DAY_T;
        return dwOri > dwValue ? dwOri : dwValue;
      }

      if (dwMaxExpireTime < dwOfflineTime)
      {
        DWORD dwValue = pItem->getParams(0) / DAY_T;
        return dwOri > dwValue ? dwOri : dwValue;
      }
      if (dwMaxExpireTime > dwOfflineTime && dwMaxExpireTime < dwOnlineTime)
      {
        pItem->setParams(0, pItem->getParams(0) + dwMaxExpireTime - dwOfflineTime);
        return pItem->getParams(0) / DAY_T;
      }
      pItem->setParams(0, pItem->getParams(0) + dwOnlineTime - dwOfflineTime);
      return pItem->getParams(0) / DAY_T;
    }
    else if (vecParams[0] == 2)
    {
      pItem->setParams(0, pItem->getParams(0) + DAY_T * dwAdd);
      return dwOri + dwAdd;
    }
  }
  else if (rCFG.stCondition.vecParams[0] == 1)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(vecParams[0]);
    if (pCFG == nullptr || pCFG->eNpcType != ENPCTYPE_MINIBOSS)
      return dwOri;
    return pUser->getWeaponPet().hasActive() == false ? dwOri : dwOri + dwAdd;
  }
  else if (rCFG.stCondition.vecParams[0] == 2)
  {
    return pUser->getWeaponPet().getUnlockCount();
  }

  return 0;
}

DWORD AchieveProcess::process_dead(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return 0;

  SAchieveItem* pItem = pUser->getAchieve().getAchieveItem(rCFG.dwID);
  if (pItem == nullptr)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0)
  {
    if (vecParams[0] != 0)
      return dwOri;
    return dwOri + dwAdd;
  }
  if (rCFG.stCondition.vecParams[0] == 1 && vecParams[1] == 0)
  {
    if (vecParams[0] == 1)
    {
      DWORD dwNow = xTime::getCurSec();
      if (pItem->getParams(0) == 0 || pItem->getParams(0) + MiscConfig::getMe().getAchieveCFG().dwDeadTimeLimit < dwNow)
        pItem->setParams(0, xTime::getCurSec());
      return dwOri;
    }
    else if (vecParams[0] == 0)
    {
      if (pItem->getParams(0) == 0)
        return dwOri;
      DWORD dwNow = xTime::getCurSec();
      if (pItem->getParams(0) + MiscConfig::getMe().getAchieveCFG().dwDeadTimeLimit < dwNow)
      {
        pItem->setParams(0, 0);
        return dwOri;
      }
      return dwOri + dwAdd;
    }
  }
  if (rCFG.stCondition.vecParams[0] == 2)
  {
    if (vecParams[0] != 0)
      return dwOri;
    return dwOri + vecParams[1];
  }

  return dwOri;
}

DWORD AchieveProcess::process_manual(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 3)
    return 0;
  if (rCFG.stCondition.vecParams[0] <= EMANUALTYPE_MIN || rCFG.stCondition.vecParams[0] >= EMANUALTYPE_MAX || EManualType_IsValid(rCFG.stCondition.vecParams[0]) == false)
    return 0;
  return pUser->getManual().getNumByStatus(static_cast<EManualType>(rCFG.stCondition.vecParams[0]), EMANUALSTATUS_UNLOCK, ENPCTYPE_MIN, static_cast<EItemType>(rCFG.stCondition.vecParams[1]));
}

DWORD AchieveProcess::process_quest(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr)
    return 0;
  return pUser->getQuest().getSubmitQuestConut();
}

DWORD AchieveProcess::process_charge(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getUserSceneData().getCharge() * 100;
}

DWORD AchieveProcess::process_refine(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2 || vecParams.empty() == true)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0 && vecParams[0] == 0)
    return dwOri + dwAdd;
  if (rCFG.stCondition.vecParams[0] == 1)
  {
    if (vecParams[0] == 1)
      return 0;
    return dwOri + dwAdd;
  }
  return dwOri;
}

DWORD AchieveProcess::process_tower(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0)
    return dwOri + dwAdd;
  if (rCFG.stCondition.vecParams[0] == 1 && pUser->getTower().getPassCount() == 1)
    return dwOri + dwAdd;
  return dwOri;
}

DWORD AchieveProcess::process_seat(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || pUser->getScene() == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;

  const SceneBase* pBaseScene = pUser->getScene()->getSceneBase();
  if (pBaseScene == nullptr)
    return 0;

  set<SceneUser*> setUsers;
  for (auto &v : rCFG.stCondition.vecParams)
  {
    Seat* pSeat = pBaseScene->getSeat(v);
    if (pSeat == nullptr)
      return 0;
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(pSeat->m_seatUser);
    if (pUser == nullptr)
      return 0;
  }

  for (auto &s : setUsers)
  {
    for (auto &target : setUsers)
    {
      if (s->id == target->id)
        continue;
      if (s->getSocial().checkRelation(target->id, ESOCIALRELATION_FRIEND) == false)
        return 0;
    }
  }

  return 1;
}

DWORD AchieveProcess::process_travel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr)
    return 0;

  TSetQWORD setHandIDs;
  TSetQWORD setWheelIDs;
  TSetQWORD setBarrageIDs;
  TSetQWORD setHandMusicIDs;
  TSetQWORD setCarrierIDs;

  pUser->getShare().collectShareCharID(ESHAREDATATYPE_MOST_HAND, setHandIDs);
  pUser->getShare().collectShareCharID(ESHAREDATATYPE_MOST_WHELL, setWheelIDs);
  pUser->getShare().collectShareCharID(ESHAREDATATYPE_MOST_BARRAGEMSG, setBarrageIDs);
  pUser->getShare().collectShareCharID(ESHAREDATATYPE_MOST_HANDMUSIC, setHandMusicIDs);
  pUser->getShare().collectShareCharID(ESHAREDATATYPE_MOST_CARRIER, setCarrierIDs);

  for (auto &s : setHandIDs)
  {
    auto item = setWheelIDs.find(s);
    if (item == setWheelIDs.end())
      continue;

    item = setBarrageIDs.find(s);
    if (item == setBarrageIDs.end())
      continue;

    item = setHandMusicIDs.find(s);
    if (item == setHandMusicIDs.end())
      continue;

    item = setCarrierIDs.find(s);
    if (item == setCarrierIDs.end())
      continue;

    return 1;
  }

  return 0;
}

DWORD AchieveProcess::process_cookfood(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (vecParams.size() < 1)
    return 0;
  if (pUser == nullptr)
    return 0;
  if (rCFG.stCondition.vecParams.size() < 1 || vecParams.size() < 1)
    return 0;

  if (rCFG.stCondition.vecParams[0] == 0) //成功失败都计数
  {
    return dwOri + dwAdd;
  }
  if (rCFG.stCondition.vecParams[0] == 1 && vecParams[0] == 0)    //失败的计数
  {    
    return dwOri + dwAdd;
  } 
  return dwOri;  
}

DWORD AchieveProcess::process_eatfood(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (rCFG.stCondition.vecParams.size() < 1 || vecParams.size() < 1)
    return 0;
  if (pUser == nullptr)
    return 0;
  if (rCFG.stCondition.vecParams[0] == 0) //0:表示吃料理就计数
  {
    return dwOri + dwAdd;
  }
  if (rCFG.stCondition.vecParams[0] == 1 && vecParams[0] == 1)    //吃别人的料理
  {
    return dwOri + dwAdd;
  }
  return dwOri;
}

DWORD AchieveProcess::process_foodmateriallvup(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (rCFG.stCondition.vecParams.size() < 1)
    return 0;
  if (pUser == nullptr)
    return 0;  
  DWORD cnt = pUser->getSceneFood().getLvUpCnt(EFOODDATATYPE_MATERIAL, 10);  
  
  if (cnt > rCFG.stCondition.vecParams[0])
    return cnt;

  return 0;
}

DWORD AchieveProcess::process_cookerlvup(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (rCFG.stCondition.vecParams.size() < 1)
    return 0;
  if (pUser == nullptr)
    return 0;
  return pUser->getSceneFood().getCookerLv();
}

DWORD AchieveProcess::process_tasterlvup(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (rCFG.stCondition.vecParams.size() < 1)
    return 0;
  if (pUser == nullptr)
    return 0;
  return pUser->getSceneFood().getTasterLv();
}

DWORD AchieveProcess::process_costsavehp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (vecParams.size() < 1)
    return 0;
  if (pUser == nullptr)
    return 0;
  
  return dwOri + vecParams[0];
}

DWORD AchieveProcess::process_costsavesp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (vecParams.size() < 1)
    return 0;
  if (pUser == nullptr)
    return 0;

  return dwOri + vecParams[0];
}

DWORD AchieveProcess::process_foodcooklv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (rCFG.stCondition.vecParams.size() < 2 || vecParams.size() < 2)
    return 0;
  if (pUser == nullptr)
    return 0;
  if (rCFG.stCondition.vecParams[0] != 0 && (rCFG.stCondition.vecParams[0] != vecParams[0]))
  {
    return 0;
  }
  if (vecParams[1] >= rCFG.stCondition.vecParams[1])
  {
    return vecParams[1];
  }
  return 0;
}

DWORD AchieveProcess::process_collection(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.size() != 2)
    return dwOri;

  SManualItem* pItem = pUser->getManual().getManualItem(EMANUALTYPE_COLLECTION);
  if (pItem == nullptr)
    return dwOri;

  DWORD dwCount = 0;
  for (auto &v : pItem->vecSubItems)
  {
    const SAchieveItemCFG* pCFG = AchieveConfig::getMe().getAchieveItemCFG(v.dwID);
    if (pCFG != nullptr && pCFG->eType == rCFG.stCondition.vecParams[0])
      ++dwCount;
  }
  return dwCount;
}

DWORD AchieveProcess::process_petadventurefinish(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  if (pUser->getPetAdventure().isComplete(rCFG.stCondition.vecParams[0]) == false)
    return 0;
  return DWORD_MAX;
}

DWORD AchieveProcess::process_petbaselevel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getUserPet().getMaxBaseLv();
}

DWORD AchieveProcess::process_petfriendlevel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getUserPet().getMaxFriendLv();
}

DWORD AchieveProcess::process_petequip(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getUserPet().getMaxEquipUnlockCount();
}

DWORD AchieveProcess::process_tutor(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;
  return pUser->getSocial().getRelationCount(ESOCIALRELATION_STUDENT_RECENT);
}

DWORD AchieveProcess::getPrayMaxLv(EPrayType eType, SceneUser* pUser)
{
  if (pUser == nullptr)
    return 0;

  DWORD maxlv = 0;
  const GuildUserInfo& rInfo = pUser->getGuild().getGuildUserInfo();
  for (int i = 0; i < rInfo.prays_size(); ++i)
  {
     const GuildMemberPray& pray = rInfo.prays(i);
     const SGuildPrayCFG* pCFG = GuildConfig::getMe().getGuildPrayCFG(pray.pray());
     if (pCFG == nullptr || pCFG->eType != eType)
       continue;
     if (pray.lv() > maxlv)
       maxlv = pray.lv();
  }
  return maxlv;
}

DWORD AchieveProcess::process_atkpraylv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;

  return getPrayMaxLv(EPRAYTYPE_GVG_ATK, pUser);
}

DWORD AchieveProcess::process_defpraylv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;

  return getPrayMaxLv(EPRAYTYPE_GVG_DEF, pUser);
}

DWORD AchieveProcess::process_elempraylv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr || rCFG.stCondition.vecParams.empty() == true)
    return 0;

  return getPrayMaxLv(EPRAYTYPE_GVG_ELE, pUser);
}

DWORD AchieveProcess::process_wedding(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr)
    return 0;

  if (rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_PURCHASE)
  {
    if (rCFG.stCondition.vecParams.empty() == true || vecParams.empty() == true)
      return dwOri;
    auto v = find(rCFG.stCondition.vecParams.begin(), rCFG.stCondition.vecParams.end(), vecParams[0]);
    if (v == rCFG.stCondition.vecParams.end())
      return dwOri;
    return dwOri + dwAdd;
  }
  else if ( rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_PROPOSE || rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_RESERVE || rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_PHOTO ||
            rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_JOINCEREMONY || rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_CEREMONY)
  {
    return dwOri + dwAdd;
  }
  else if (rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_CARRIER)
  {
    if (rCFG.stCondition.vecParams.empty() == true || vecParams.empty() == true)
      return 0;
    if (rCFG.stCondition.vecParams[0] != vecParams[0])
      return 0;
    return 1;
  }
  else if (rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_DAY)
  {
    if (pUser->getUserWedding().isMarried() == false)
      return dwOri;
    DWORD dwNow = xTime::getCurSec();
    DWORD dwEnd = pUser->getUserWedding().getWeddingInfo().endtime();
    if (dwEnd > dwNow)
      return dwOri;
    DWORD dwDay = (dwNow - dwEnd) / DAY_T;
    return dwDay > dwOri ? dwDay : dwOri;
  }
  else if (rCFG.stCondition.eCond == EACHIEVECOND_WEDDING_DIVORCE)
  {
    if (rCFG.stCondition.vecParams.empty() == true || vecParams.empty() == true)
      return 0;
    if (rCFG.stCondition.vecParams[0] != vecParams[0])
      return 0;
    return 1;
  }

  return dwOri;
}

DWORD AchieveProcess::process_profession(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if(!pUser)
    return 0;

  if(rCFG.stCondition.vecParams.empty())
    return 0;

  if(pUser->getProfession() != rCFG.stCondition.vecParams[0])
    return 0;

  return dwOri + dwAdd;
}

DWORD AchieveProcess::process_quest_submit(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd)
{
  if (pUser == nullptr)
    return 0;

  Quest& rQuest = pUser->getQuest();
  DWORD dwCount = 0;
  for (auto &v : rCFG.stCondition.vecParams)
  {
    if (rQuest.isSubmit(v) == true)
      ++dwCount;
  }

  return dwCount;
}

