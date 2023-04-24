#include "AICondition.h"
#include "SceneNpc.h"
#include "Scene.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "DScene.h"
#include "xLuaTable.h"
#include "SkillItem.h"
#include "SkillManager.h"
#include "SceneNpcManager.h"

//const char LOG_NAME[] = "AICondition";

// AICondition - base
AI_RET_ENUM AICondition::checkCondition(SceneNpc* pNpc)
{
  if (pNpc == nullptr)
    return AI_RET_RUN_FAIL;

  DWORD dwNow = xTime::getCurSec();
  if (m_dwNextTime > dwNow)
    return AI_RET_RUN_FAIL;

  //int iRand = randBetween(0, m_oParams.getTableInt("maxrate"));
  //if (iRand > m_oParams.getTableInt("rate"))
  //  return AI_RET_RUN_FAIL;

  DWORD dwMaxCount = m_oParams.getTableInt("count");
  if (dwMaxCount != 0 && m_dwCount >= dwMaxCount)
    return AI_RET_RUN_FAIL;

  m_dwNextTime = dwNow + m_oParams.getTableInt("interval");  //warning 只是检测时间

  //DWORD dwOdds = m_oParams.getTableInt("odds");
  //if (dwOdds != 0 && dwOdds < (DWORD)randBetween(1, 100))
    //return AI_RET_RUN_FAIL;

  return AI_RET_RUN_SUCC;
}

DWORD AICondition::getWeight()
{
  return m_oParams.getTableInt("weight");
}

bool AICondition::checkOdds()
{
  DWORD dwOdds = m_oParams.getTableInt("odds");
  DWORD rand = randBetween(1, 100);
  if (dwOdds != 0 && dwOdds < rand)
    return false;
  return true;
}

void AICondition::reset()
{
  m_dwCount = 0;
  m_dwNextTime = 0;
  m_bNoRandExe = m_oParams.getTableInt("noRandExe") == 1;
}

// AICondition - timer
AI_RET_ENUM TimerAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC)
    return ret;

  if (timeFromBirth != 0)
  {
    QWORD past = xTime::getCurMSec() - pNpc->getBirthTime();
    if (past >= timeFromBirth)
    {
      return AI_RET_RUN_SUCC;
    }
    else
    {
      return AI_RET_COND_FAIL_CONT;
    }
  }
  if (m_oParams.has("hpless"))
  {
    DWORD dwper = m_oParams.getTableInt("hpless");
    DWORD nowhp = pNpc->getAttr(EATTRTYPE_HP);
    DWORD maxhp = pNpc->getAttr(EATTRTYPE_MAXHP);
    if (maxhp == 0)
      return AI_RET_COND_FAIL_CONT;
    if (nowhp * 100 / maxhp > dwper)
      return AI_RET_COND_FAIL_CONT;
  }
  if (m_oParams.has("hpmore"))
  {
    DWORD dwper = m_oParams.getTableInt("hpmore");
    DWORD nowhp = pNpc->getAttr(EATTRTYPE_HP);
    DWORD maxhp = pNpc->getAttr(EATTRTYPE_MAXHP);
    if (maxhp == 0)
      return AI_RET_COND_FAIL_CONT;
    if (nowhp * 100 / maxhp < dwper)
      return AI_RET_COND_FAIL_CONT;
  }
  if (m_oParams.has("needbuff"))
  {
    DWORD needbuff = m_oParams.getTableInt("needbuff");
    if (pNpc->m_oBuff.haveBuff(needbuff) == false)
      return AI_RET_COND_FAIL_CONT;
  }
  if (m_oParams.has("notbuff"))
  {
    DWORD notbuff = m_oParams.getTableInt("notbuff");
    if (pNpc->m_oBuff.haveBuff(notbuff))
      return AI_RET_COND_FAIL_CONT;
  }
  return AI_RET_RUN_SUCC;
}

void TimerAICondition::reset()
{
  AICondition::reset();

  timeFromBirth = m_oParams.getTableInt("time") * 1000;
}

// AICondition - birth
AI_RET_ENUM BirthAICondition::checkCondition(SceneNpc* pNpc)
{
  return AICondition::checkCondition(pNpc);
}

void BirthAICondition::reset()
{
  AICondition::reset();
}

// AICondition - hpless
AI_RET_ENUM HPLessAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (pNpc == nullptr)
    return AI_RET_RUN_FAIL;

  if (pNpc->getAttr(EATTRTYPE_HP) == 0)
    return AI_RET_RUN_FAIL;

  if (hpPercent)
  {
    float maxhp = pNpc->getAttr(EATTRTYPE_MAXHP);
    float hp = pNpc->getAttr(EATTRTYPE_HP);
    if (hpMinPercent == 0)
    {
      if (hp < maxhp * hpPercent / 100)
        return AI_RET_RUN_SUCC;
    }
    else
    {
      if (hp < maxhp * hpPercent / 100 && hp > maxhp * hpMinPercent / 100)
        return AI_RET_RUN_SUCC;
    }
  }

  if (hpLess)
  {
    if (pNpc->getAttr(EATTRTYPE_HP) < hpLess)
    {
      return AI_RET_RUN_SUCC;
    }
  }

  return AI_RET_COND_FAIL_CONT;
}

void HPLessAICondition::reset()
{
  AICondition::reset();
  hpLess = m_oParams.getTableInt("value");
  hpPercent = m_oParams.getTableInt("percent");
  hpMinPercent = m_oParams.getTableInt("min_percent");
}

// AICondition - chantlock  受到施法锁定
AI_RET_ENUM ChantLockAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  const pair<QWORD, DWORD>& pa = pNpc->m_ai.getLockMeUserSkill();
  const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(pa.second);
  if (pSkill == nullptr)
    return AI_RET_COND_FAIL_CONT;

  if (m_oParams.getTableInt("skill_magic") == 1 && pSkill->isMagicSkill() == false)
    return AI_RET_COND_FAIL_CONT;
  if (m_oParams.getTableInt("skill_pointRange") == 1 && pSkill->getLogicType() != ESKILLLOGIC_POINTRANGE)
    return AI_RET_COND_FAIL_CONT;
  if (m_oParams.getTableInt("skill_lockTarget") == 1 && pSkill->getLogicType() != ESKILLLOGIC_LOCKEDTARGET)
    return AI_RET_COND_FAIL_CONT;
  return ret;
}

// AICondition - onbuff  自己处于某个状态 / buff效果
AI_RET_ENUM OnBuffAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (!pNpc || !pNpc->getScene())
    return AI_RET_COND_FAIL_CONT;

  if (pNpc->m_oBuff.isInStatus(m_oParams.getTableInt("stateeffect")))
  {
    return AI_RET_RUN_SUCC_CONT;
  }
  return AI_RET_COND_FAIL_CONT;
}

// AICondition - hpchange
AI_RET_ENUM HPChangeAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (pNpc == nullptr)
    return AI_RET_RUN_FAIL;

  if (pNpc->getAttr(EATTRTYPE_HP) == 0)
    return AI_RET_RUN_FAIL;

  float maxhp = pNpc->getAttr(EATTRTYPE_MAXHP);
  if (maxhp == 0)
    return AI_RET_RUN_FAIL;

  float lastper = pNpc->getLastHp() / maxhp * 100;
  float nowper = pNpc->getAttr(EATTRTYPE_HP) / maxhp * 100;
  if (lastper >= m_fhpLessPercent && nowper <= m_fhpLessPercent && lastper != nowper)
  {
    return AI_RET_RUN_SUCC;
  }

  return AI_RET_COND_FAIL_CONT;
}

void HPChangeAICondition::reset()
{
  AICondition::reset();
  m_fhpLessPercent = m_oParams.getTableFloat("lessPer");
}

// AICondition - beattack
AI_RET_ENUM BeAttackAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (pNpc == nullptr)
    return AI_RET_RUN_FAIL;

  const pair<QWORD, DWORD>& pa = pNpc->m_ai.getAttMeUserSkill();
  const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(pa.second);
  if (pSkill == nullptr)
    return AI_RET_COND_FAIL_CONT;

  if (m_oParams.getTableInt("skill_magic") == 1 && pSkill->isMagicSkill() == false)
    return AI_RET_COND_FAIL_CONT;
  if (m_oParams.getTableInt("skill_physic") == 1 && pSkill->isPhySkill() == false)
    return AI_RET_COND_FAIL_CONT;
  if (m_oParams.getTableInt("skill_long") == 1 && pSkill->isLongSkill() == false)
    return AI_RET_COND_FAIL_CONT;
  if (m_oParams.getTableInt("skill_near") == 1 && pSkill->isLongSkill())
    return AI_RET_COND_FAIL_CONT;

  return AI_RET_RUN_SUCC;
}

void BeAttackAICondition::reset()
{
  AICondition::reset();
}

// AICondition - death
AI_RET_ENUM DeathAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (pNpc == nullptr)
    return AI_RET_RUN_FAIL;

  if (pNpc->isAlive())
    return AI_RET_COND_FAIL_CONT;
  if (pNpc->getAttr(EATTRTYPE_HP) != 0)
    return AI_RET_COND_FAIL_CONT;

  if (m_oParams.getTableInt("killer_transform") == 1)
  {
    QWORD userid = pNpc->m_ai.getLastAttacker();
    SceneUser* user = SceneUserManager::getMe().getUserByID(userid);
    if (user == nullptr || user->getTransform().isInTransform() == false)
      return AI_RET_COND_FAIL_CONT;
  }
  return AI_RET_RUN_SUCC;
}

void DeathAICondition::reset()
{
  AICondition::reset();
}

// AICondtion - kill user
AI_RET_ENUM KillUserAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  return AI_RET_RUN_SUCC;
}

// AICondition - normal status
AI_RET_ENUM NormalAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (m_oParams.getTableInt("time") != 0)
  {
    if (pNpc->m_ai.getCurLockID() != 0)
      return AI_RET_COND_FAIL_CONT;

    DWORD time = pNpc->m_ai.getLastAttackTime();
    if (time == 0)
      time = pNpc->getBirthTime() / ONE_THOUSAND;

    if (now() < time + m_oParams.getTableInt("time"))
      return AI_RET_COND_FAIL_CONT;

    time = pNpc->m_ai.getLastBeAttackTime();
    if (now() < time + m_oParams.getTableInt("time"))
      return AI_RET_COND_FAIL_CONT;
  }

  return AI_RET_RUN_SUCC;
}

// AICondition - attack status
AI_RET_ENUM AttackAICondition::checkCondition(SceneNpc* pNpc)
{
  if (pNpc == nullptr)
    return AI_RET_RUN_FAIL;

  //先检查是否满足战斗时间 满足后从当前时间开始计算interval
  DWORD cur = now();
  DWORD attackStartTime = pNpc->m_ai.getAttackStartTime();
  if (attackStartTime && attackStartTime + timeFromAttack > cur)
    return AI_RET_RUN_FAIL;

  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (m_oParams.getTableInt("noskill_time") != 0)
  {
    DWORD val = m_oParams.getTableInt("noskill_time");
    DWORD atttime = pNpc->m_ai.getLastAttackTime();
    atttime = atttime ? atttime : cur;

    if (cur > val + atttime)
    {
      DWORD locktime = pNpc->m_ai.getLastLockTime() / ONE_THOUSAND;
      if (pNpc->m_ai.getCurLockID() && locktime < atttime)
        return AI_RET_RUN_SUCC;
    }
    return AI_RET_RUN_FAIL;
  }

  if (hpLess == 0 && hpPercent == 0)
    return AI_RET_RUN_SUCC;

  if (pNpc->getAttr(EATTRTYPE_HP) == 0)
    return AI_RET_RUN_FAIL;

  if (hpPercent)
  {
    if (pNpc->getAttr(EATTRTYPE_HP) < pNpc->getAttr(EATTRTYPE_MAXHP) * hpPercent / 100)
    {
      return AI_RET_RUN_SUCC;
    }
  }

  if (hpLess)
  {
    if (pNpc->getAttr(EATTRTYPE_HP) < hpLess)
    {
      return AI_RET_RUN_SUCC;
    }
  }
  return AI_RET_RUN_FAIL;
}
void AttackAICondition::reset()
{
  AICondition::reset();
  hpLess = m_oParams.getTableInt("value");
  hpPercent = m_oParams.getTableInt("percent");
  timeFromAttack = m_oParams.getTableInt("time");
}

// AICondition - disp hide
AI_RET_ENUM DispHideAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  return AI_RET_RUN_SUCC;
}

// AICondition - see dead
AI_RET_ENUM SeeDeadAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  return AI_RET_RUN_SUCC;
}

AI_RET_ENUM BuffLayerAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  DWORD buffid = m_oParams.getTableInt("id");
  DWORD layer = m_oParams.getTableInt("layer");
  if (buffid == 0 || layer == 0)
    return AI_RET_COND_FAIL_CONT;

  DWORD nowlayer = pNpc->m_oBuff.getLayerByID(buffid);
  if (nowlayer < layer)
    return AI_RET_COND_FAIL_CONT;
  return AI_RET_RUN_SUCC;
}

// AICondition - camera shot
AI_RET_ENUM CameraShotAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  return AI_RET_RUN_SUCC;
}

// AICondition - camera lock
AI_RET_ENUM CameraLockAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  return AI_RET_RUN_SUCC;
}

// AICondition - weaponpet user die
AI_RET_ENUM EmployerDieAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (pNpc->isWeaponPet() == false)
    return AI_RET_COND_FAIL_CONT;

  return AI_RET_RUN_SUCC;
}

// AICondition - weaponpet user die
AI_RET_ENUM EmployerReliveAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  if (pNpc->isWeaponPet() == false)
    return AI_RET_COND_FAIL_CONT;

  return AI_RET_RUN_SUCC;
}

// AICondition - weaponpet user die for a time
AI_RET_ENUM EmployerDieStatusAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  WeaponPetNpc* pPetNpc = dynamic_cast<WeaponPetNpc*> (pNpc);
  if (pPetNpc == nullptr)
    return AI_RET_COND_FAIL_CONT;

  SceneUser* pMasterUser = pPetNpc->getMasterUser();
  if (pMasterUser == nullptr || pMasterUser->isAlive())
    return AI_RET_COND_FAIL_CONT;

  DWORD time = m_oParams.getTableInt("time");
  DWORD interval = m_oParams.getTableInt("interval");
  if (time == 0 || interval == 0)
    return AI_RET_COND_FAIL_CONT;

  DWORD cur = now();
  DWORD userdietime = pMasterUser->getDieTime();

  if (cur < userdietime + time)
    m_bTimeAlreadyOk = false;

  if (m_bTimeAlreadyOk == false && cur >= userdietime + time)
  {
    m_bTimeAlreadyOk = true;
    return AI_RET_RUN_SUCC;
  }

  return AI_RET_COND_FAIL_CONT;
}

AI_RET_ENUM EmployerActionAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  WeaponPetNpc* pPetNpc = dynamic_cast<WeaponPetNpc*> (pNpc);
  if (pPetNpc == nullptr)
    return AI_RET_COND_FAIL_CONT;

  SceneUser* pMasterUser = pPetNpc->getMasterUser();
  if (pMasterUser == nullptr || pMasterUser->isAlive() == false)
    return AI_RET_COND_FAIL_CONT;

  DWORD action = m_oParams.getTableInt("actionid");
  if (action == pMasterUser->getAction())
    return AI_RET_RUN_SUCC;

  return AI_RET_COND_FAIL_CONT;
}

AI_RET_ENUM NpcFunctionAICondition::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;
  DWORD functionid = m_oParams.getTableInt("functionid");
  if (functionid && functionid == pNpc->m_sai.getFunctionID())
    return AI_RET_RUN_SUCC;
  return AI_RET_COND_FAIL_CONT;
}

AI_RET_ENUM ServantDieAICondtion::checkCondition(SceneNpc* pNpc)
{
  // 不检查基类interval，避免小怪几乎同时死亡时检测失败
  if (pNpc == nullptr)
    return AI_RET_COND_FAIL_CONT;
  if (pNpc->m_oFollower.hasServant() == false)
    return AI_RET_RUN_SUCC;

  return AI_RET_COND_FAIL_CONT;
}

// AICondition - visit
AI_RET_ENUM VisitAICondition::checkCondition(SceneNpc* pNpc)
{
  return AICondition::checkCondition(pNpc);
}

void VisitAICondition::reset()
{
  AICondition::reset();
}

//AIConditon - kill monster
AI_RET_ENUM KillMonsterAIConditon::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;

  return AI_RET_RUN_SUCC;
}

//AICondition - alert
AI_RET_ENUM AlertAIConditon::checkCondition(SceneNpc* pNpc)
{
  AI_RET_ENUM ret = AICondition::checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return ret;
  if (pNpc == nullptr)
    return AI_RET_COND_FAIL_CONT;

  const string &type = m_oParams.getTableString("type");
  SDWORD num = m_oParams.getTableInt("num");
  DWORD range = m_oParams.getTableInt("range");

  if (!range)
    return AI_RET_COND_FAIL_CONT;

  SCENE_ENTRY_TYPE entryType = SCENE_ENTRY_USER;
  bool m_bJustMonster = false;

  if (type == "player")
    entryType = SCENE_ENTRY_USER;
  else if (type == "monster")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustMonster = true;
  }
  else 
    return AI_RET_COND_FAIL_CONT;

  xSceneEntrySet uSet;
  pNpc->getScene()->getEntryListInBlock(entryType,pNpc->getPos(),range,uSet);
  for (auto &s : uSet)
  {
    if (entryType == SCENE_ENTRY_USER)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(s);
      if (pUser && pUser->isAlive())
        num--;
    }
    else if (entryType == SCENE_ENTRY_NPC)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*>(s);
      if (npc == pNpc)
        continue;
      if (npc && npc->isAlive())
      {
        if (m_bJustMonster && npc->isMonster())
          num--;
        else if (!m_bJustMonster)
          num--;
      }
    }
    if (num <= 0)
      break;
  }
  if (num > 0)
    return AI_RET_COND_FAIL_CONT;
  
  return AI_RET_RUN_SUCC;
}


// ---------------------------------- AITarget ------------------------------
// AITarget - self
bool SelfAITarget::getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (pNpc)// && selectByPercent(m_dwPercent))
  {
    targetSet.insert(pNpc);
    return true;
  }
  return false;
}

// AITarget - range
RangeAITarget::RangeAITarget(const xLuaData& params) : AITarget(params)
{
  m_dwRadius = m_oParams.getTableInt("range");
  m_dwMinRadius = m_oParams.getTableInt("minrange");

  m_dwCountMax = m_oParams.getTableInt("count");
  m_bSelectMinHSp = m_oParams.getTableInt("selectMinSHp") == 1;
  m_bSelectMinHp = m_oParams.getTableInt("selectMinHp") == 1;
  m_dwPriorExpression = m_oParams.getTableInt("prior_expression");
  m_fHpper = m_oParams.getTableFloat("hpPercent");
  m_bNeedCanLock = m_oParams.getTableInt("can_lock");
  m_bMap = m_oParams.getTableInt("map") == 1;

  xLuaData& statedata = m_oParams.getMutableData("stateeffect");
  xLuaData& luaData = m_oParams.getMutableData("npcid");
  DWORD type = 1;
  auto func = [&](const string& key, xLuaData& data)
  {
    if (data.getInt() == 0)
      return;

    if (type == 1)
      m_setNpcIds.insert(data.getInt());
    else if (type == 2)
      m_setStates.insert(data.getInt());
  };
  type = 1;
  luaData.foreach(func);
  type = 2;
  statedata.foreach(func);

  if (m_oParams.has("prior_profession"))
    m_oParams.getMutableData("prior_profession").getIDList(m_vecPriorProfession);
}

bool RangeAITarget::getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene())
    return false;

  SCENE_ENTRY_TYPE entryType = SCENE_ENTRY_USER;
  bool m_bJustMonster = false;
  bool m_bJustNormalMonster = false;
  bool m_bJustMvp = false;
  bool m_bJustMini = false;
  bool m_bJustPetnpc = false;
  bool m_bJustWeaponpet = false;
  bool m_bInNormalStatus = m_oParams.getTableInt("normal_status") == 1;
  bool m_bMasterUser = false;

  const string& type = m_oParams.getTableString("type");
  if (type == "player")
  {
    entryType = SCENE_ENTRY_USER;
  }
  else if (type == "masteruser")
  {
    m_bMasterUser = true;
  }
  else if (type == "npc")
  {
    entryType = SCENE_ENTRY_NPC;
  }
  else if (type == "monster")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustMonster = true;
  }
  else if (type == "normal_monster")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustNormalMonster = true;
  }
  else if (type == "mvp")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustMvp = true;
  }
  else if (type == "mini")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustMini = true;
  }
  else if (type == "petnpc")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustPetnpc = true;
  }
  else if (type == "weaponpet")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustWeaponpet = true;
  }

  xSceneEntrySet uSet;
  if (m_bMap)
  {
    if (m_bMasterUser)
    {
      SceneUser* master = pNpc->getMasterUser();
      if (master)
        uSet.insert(master);
    }
    else
      pNpc->getScene()->getAllEntryList(entryType, uSet);
  }
  else
  {
    if (m_bMasterUser)
    {
      SceneUser* master = pNpc->getMasterUser();
      if (master)
      {
        float distance = getDistance(pNpc->getPos(), master->getPos());
        if (distance < m_dwRadius && distance > m_dwMinRadius)
          uSet.insert(master);
      }
    }
    else
    {
      if(!m_dwMinRadius)
        pNpc->getScene()->getEntryListInBlock(entryType, pNpc->getPos(), m_dwRadius, uSet);
      else 
        pNpc->getScene()->getEntryListInRing(entryType, pNpc->getPos(), m_dwMinRadius, m_dwRadius, uSet);
    }
  }
  if (m_bJustMonster)
  {
    for (auto it = uSet.begin(); it != uSet.end(); )
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (*it);
      if (npc == nullptr || npc->isMonster())
      {
        ++it;
        continue;
      }
      it = uSet.erase(it);
    }
  }
  if (m_bJustNormalMonster)
  {
    for (auto it = uSet.begin(); it != uSet.end(); )
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (*it);
      if (npc == nullptr || npc->getNpcType() != ENPCTYPE_MONSTER)
      {
        it = uSet.erase(it);
        continue;
      }
      ++it;
    }
  }
  if (m_bJustMvp)
  {
    for (auto it = uSet.begin(); it != uSet.end(); )
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (*it);
      if (npc == nullptr || npc->getNpcType() != ENPCTYPE_MVP)
      {
        it = uSet.erase(it);
        continue;
      }
      ++it;
    }
  }
  if (m_bJustMini)
  {
    for (auto it = uSet.begin(); it != uSet.end(); )
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (*it);
      if (npc == nullptr || npc->getNpcType() != ENPCTYPE_MINIBOSS)
      {
        it = uSet.erase(it);
        continue;
      }
      ++it;
    }
  }
  if (m_bJustPetnpc)
  {
    for (auto it = uSet.begin(); it != uSet.end(); )
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (*it);
      if (npc == nullptr || npc->getNpcType() != ENPCTYPE_PETNPC)
      {
        it = uSet.erase(it);
        continue;
      }
      ++it;
    }
  }
  if (m_bJustWeaponpet)
  {
    for (auto it = uSet.begin(); it != uSet.end(); )
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (*it);
      if (npc == nullptr || npc->getNpcType() != ENPCTYPE_WEAPONPET)
      {
        it = uSet.erase(it);
        continue;
      }
      ++it;
    }
  }

  // 指定npc base->id
  if (!m_setNpcIds.empty())
  {
    for (auto m = uSet.begin(); m != uSet.end(); )
    {
      SceneNpc *npc = dynamic_cast<SceneNpc*>((*m));
      if (!npc)
      {
        ++m;
        continue;
      }
      if (m_setNpcIds.find(npc->getNpcID()) != m_setNpcIds.end())
      {
        ++m;
        continue;
      }
      m = uSet.erase(m);
    }
  }

  if (m_bInNormalStatus)
  {
    for (auto m = uSet.begin(); m != uSet.end(); )
    {
      SceneNpc *npc = dynamic_cast<SceneNpc*>((*m));
      if (!npc)
      {
        ++m;
        continue;
      }
      if (npc->m_ai.getStateType() == ENPCSTATE_NORMAL)
      {
        ++m;
        continue;
      }
      m = uSet.erase(m);
    }
  }

  // 获得sp或hp百分比最小的entry
  auto getMinHpSp = [&](const xSceneEntrySet& set, bool hasSp) -> xSceneEntryDynamic*
  {
    xSceneEntryDynamic* pHpMinTar = nullptr;
    float minHpPer = 100.0;
    for (auto m = set.begin(); m != set.end(); ++m)
    {
      xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (*m);
      if (!pEntry || pEntry->isAlive() == false)
        continue;
      float hp = static_cast<DWORD> (pEntry->getAttr(EATTRTYPE_HP));
      float maxhp = static_cast<DWORD> (pEntry->getAttr(EATTRTYPE_MAXHP));
      float perc = (maxhp != 0 ? (hp / maxhp * 100) : 100);
      if (hasSp)
      {
        float sp = static_cast<DWORD> (pEntry->getAttr(EATTRTYPE_SP));
        float maxsp = static_cast<DWORD> (pEntry->getAttr(EATTRTYPE_MAXSP));
        float perc2 = (maxsp != 0 ? (sp / maxsp * 100) : 100);
        perc = perc < perc2 ? perc : perc2;
      }
      if (perc >= 0 && perc < 100)
      {
        targetSet.insert(*m);
        if (perc < minHpPer)
        {
          pHpMinTar = pEntry;
          minHpPer = perc;
        }
      }
    }
    return pHpMinTar;
  };

  targetSet.clear();
  // 优先做过指定表情的玩家
  if (m_dwPriorExpression != 0)
  {
    for (auto &s : uSet)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*> (s);
      if (!pUser || pUser->getExpression() != m_dwPriorExpression)
        continue;
      targetSet.insert(s);
      pUser->setExpression(0);
    }
  }
  if (targetSet.empty())
    targetSet.insert(uSet.begin(), uSet.end());

  // 优先选择hp最小的玩家
  if (m_bSelectMinHp)
  {
    xSceneEntryDynamic* entry = getMinHpSp(targetSet, false);
    targetSet.clear();
    if (entry)
      targetSet.insert(entry);
    return true;
  }
  // 优先sp或hp最小的玩家
  if (m_bSelectMinHSp)
  {
    xSceneEntryDynamic* entry = getMinHpSp(targetSet, true);
    targetSet.clear();
    if (entry)
      targetSet.insert(entry);
    return true;
  }

  // 血量限制
  if (m_fHpper != 0)
  {
    for (auto s = targetSet.begin(); s != targetSet.end(); )
    {
      xSceneEntryDynamic* entry = dynamic_cast<xSceneEntryDynamic*> (*s);
      if (entry && entry->getAttr(EATTRTYPE_MAXHP) != 0)
      {
        float per = entry->getAttr(EATTRTYPE_HP) / entry->getAttr(EATTRTYPE_MAXHP) * 100;
        if (per <= m_fHpper)
        {
          ++s;
          continue;
        }
      }
      s = targetSet.erase(s);
    }
  }
  // 处于buff状态
  if (!m_setStates.empty())
  {
    for (auto s = targetSet.begin(); s != targetSet.end(); )
    {
      xSceneEntryDynamic* entry = dynamic_cast<xSceneEntryDynamic*> (*s);
      bool find = false;
      for (auto &status : m_setStates)
      {
        if (entry && entry->m_oBuff.isInStatus(status))
        {
          find = true;
          break;
        }
      }
      if (find)
      {
        ++s;
        continue;
      }
      s = targetSet.erase(s);
    }
  }
  if (m_bNeedCanLock)
  {
    for (auto it = targetSet.begin(); it != targetSet.end(); )
    {
      xSceneEntryDynamic* tar = dynamic_cast<xSceneEntryDynamic*> (*it);
      if (tar == nullptr || pNpc->m_ai.canAI2Attack(tar) == false)
      {
        it = targetSet.erase(it);
        continue;
      }
      ++it;
    }
  }
  // 优先选取职业
  if (m_vecPriorProfession.empty() == false)
  {
    auto getprior = [&](DWORD pro) -> DWORD
    {
      DWORD i = 0;
      for (auto &v : m_vecPriorProfession)
      {
        if (v == pro)
          return i;
        i++;
      }
      return DWORD_MAX;
    };
    if (targetSet.size() > 1)
    {
      xSceneEntryDynamic* entry = nullptr;
      DWORD mPrior = DWORD_MAX;
      for (auto &s : targetSet)
      {
        SceneUser* user = dynamic_cast<SceneUser*>(s);
        if (!user)
          continue;
        DWORD prior = getprior(user->getProfession());
        if (prior < mPrior)
        {
          mPrior = prior;
          entry = user;
        }
      }
      if (entry)
      {
        targetSet.clear();
        targetSet.insert(entry);
      }
    }
  }
  // 数量限制
  if (m_dwCountMax != 0 && targetSet.size() > m_dwCountMax)
  {
    std::vector<xSceneEntry*> vecEntry;
    for (auto &s : targetSet)
      vecEntry.push_back(s);
    targetSet.clear();

    std::random_shuffle(vecEntry.begin(), vecEntry.end());
    DWORD index = 0;
    for (auto &v : vecEntry)
    {
      if (index >= m_dwCountMax)
        break;
      targetSet.insert(v);
    }
  }

  return true;
}

// AITarget - attack me
AttackMeAITarget::AttackMeAITarget(const xLuaData& params) : AITarget(params)
{

}

bool AttackMeAITarget::getEntry(SceneNpc *pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene())
    return false;

  //距离最近的攻击者
  if (m_oParams.getTableInt("nearest"))
  {    
    TSetQWORD& attackList = pNpc->m_ai.getAttackList();

    if (attackList.empty())
      return false;

    float fDis = 0;
    float minDis = 1000;
    xSceneEntryDynamic* pTar = nullptr;
    for (auto iter = attackList.begin(); iter != attackList.end(); ++iter)
    {
      xSceneEntryDynamic *pTarget = xSceneEntryDynamic::getEntryByID(*iter);
      if (!pTarget || !pTarget->isAlive())
        continue;
      fDis = getDistance(pTarget->getPos(), pNpc->getPos());
      if (fDis < minDis)
      {
        minDis = fDis;
        pTar = pTarget;
      }
    }
    if (!pTar || pTar->getScene() != pNpc->getScene())
      return false;
    targetSet.insert(pTar);
    return true;
  }

  QWORD attackerid = pNpc->m_ai.getLastAttacker();
  if (attackerid == 0)
    return false;
  xSceneEntryDynamic* pAttacker = xSceneEntryDynamic::getEntryByID(attackerid);
  if (!pAttacker || pAttacker->getScene() != pNpc->getScene())
    return false;
  targetSet.insert(pAttacker);
  return true;
}

// AITarget - skill lock me
LockMeAITarget::LockMeAITarget(const xLuaData& params) : AITarget(params)
{
}

bool LockMeAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene())
    return false;

  QWORD lockerid = pNpc->m_ai.getLockMeUserSkill().first;
  SceneUser* user = SceneUserManager::getMe().getUserByID(lockerid);
  if (user && user->getScene() == pNpc->getScene())
  {
    targetSet.insert(user);
    return true;
  }

  return false;
}

// AITarget - user killed
UserKilledAITarget::UserKilledAITarget(const xLuaData& params) : AITarget(params)
{

}

bool UserKilledAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene())
    return false;
  SceneUser* user = SceneUserManager::getMe().getUserByID(pNpc->m_ai.getMyKillUser());
  if (!user || user->isAlive() == true)
    return false;
  targetSet.insert(user);

  return true;
}

// AITarget - attacklist, 攻击我的列表
SprintAITarget::SprintAITarget(const xLuaData& params) : AITarget(params)
{
}

bool SprintAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene())
    return false;
  TSetQWORD& attackList = pNpc->m_ai.getAttackList();
  if (attackList.empty())
    return false;

  float fDis = 0;
  float minDis = 1000;
  DWORD range = m_oParams.getTableInt("range");
  xSceneEntryDynamic* pTar = nullptr;
  for (auto iter = attackList.begin(); iter != attackList.end(); ++iter)
  {
    xSceneEntryDynamic *pTarget = xSceneEntryDynamic::getEntryByID(*iter);
    if (!pTarget || !pTarget->isAlive())
      continue;
    fDis = getDistance(pTarget->getPos(), pNpc->getPos());
    if (fDis <= range)
      continue;
    if (fDis < minDis)
    {
      minDis = fDis;
      pTar = pTarget;
    }
  }
  if (!pTar || pTar->getScene() != pNpc->getScene())
    return false;

  SceneUser* user = dynamic_cast<SceneUser*>(pTar);
  if (!user || user->isAlive() == true)
    return false;
  targetSet.insert(user);
  return true;
}

// AITarget - 当前锁定的玩家
LockingAITarget::LockingAITarget(const xLuaData& params) : AITarget(params)
{

}

bool LockingAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (pNpc == nullptr || pNpc->m_ai.getCurLockID() == 0)
    return false;

  SceneUser* user = SceneUserManager::getMe().getUserByID(pNpc->m_ai.getCurLockID());
  if (user == nullptr || user->getScene() != pNpc->getScene())
    return false;
  float range = m_oParams.getTableInt("range");
  if (range > 0)
  {
    if (getDistance(pNpc->getPos(), user->getPos()) > range)
      return false;
  }

  targetSet.insert(user);
  return true;
}

// AITarget - 死亡的目标
DeadAITarget::DeadAITarget(const xLuaData& params) : AITarget(params)
{
  m_dwRange = m_oParams.getTableInt("range");
  xLuaData& luaData = m_oParams.getMutableData("npcid");
  auto func = [this](const string& key, xLuaData& data)
  {
    m_setNpcIds.insert(data.getInt());
  };
  luaData.foreach(func);
}

bool DeadAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return false;

  DWORD range = m_oParams.getTableInt("range");
  if (range == 0)
    return false;

  SCENE_ENTRY_TYPE entryType = SCENE_ENTRY_USER;
  const string& name = m_oParams.getTableString("type");

  if (name == "player")
    entryType = SCENE_ENTRY_USER;
  else if (name == "npc")
    entryType = SCENE_ENTRY_NPC;
  else
    return false;

  xSceneEntrySet uSet;
  pNpc->getScene()->getEntryListInBlock(entryType, pNpc->getPos(), range, uSet);
  for (auto &s : uSet)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
    if (pEntry == nullptr || pEntry ->isAlive())
      continue;

    if (!m_setNpcIds.empty())
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (!npc || m_setNpcIds.find(npc->getNpcID()) == m_setNpcIds.end())
        continue;
    }

    targetSet.insert(s);
  }

  return !targetSet.empty();
}


// AITarget - 牵手的玩家
HandUserAITarget::HandUserAITarget(const xLuaData& params) : AITarget(params)
{

}

bool HandUserAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet &targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return false;

  DWORD range = m_oParams.getTableInt("range");
  if (range == 0)
    return false;

  xSceneEntrySet uSet;
  pNpc->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, pNpc->getPos(), range, uSet);

  for (auto &s : uSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user == nullptr)
      continue;
    if (user->m_oHands.has() && user->m_oHands.isMaster())
      targetSet.insert(s);
  }

  return !targetSet.empty();
}

// AITarget - 做表情或动作的玩家
ActionUserAITarget::ActionUserAITarget(const xLuaData& params) : AITarget(params)
{
  DWORD type = 1;
  auto getids = [&](const string& key, xLuaData& d)
  {
    if (type == 1)
      m_setExpressions.insert(d.getInt());
    else if (type == 2)
      m_setActions.insert(d.getInt());
  };
  type = 1;
  m_oParams.getMutableData("expression").foreach(getids);
  type = 2;
  m_oParams.getMutableData("action").foreach(getids);
}

bool ActionUserAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return false;

  DWORD range = m_oParams.getTableInt("range");
  if (range == 0)
    return false;

  if (m_setExpressions.empty() && m_setActions.empty())
    return false;

  xSceneEntrySet uSet;
  pNpc->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, pNpc->getPos(), range, uSet);

  targetSet.clear();
  for (auto& s : uSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user == nullptr)
      return false;
    if (m_setExpressions.empty() == false)
    {
      if (m_setExpressions.find(user->getExpression()) == m_setExpressions.end())
        continue;
    }
    if (m_setActions.empty() == false)
    {
      if (m_setActions.find(user->getAction()) == m_setActions.end())
        continue;
    }
    targetSet.insert(s);
  }

  return !targetSet.empty();
}

WeaponPetUserAITarget::WeaponPetUserAITarget(const xLuaData& params) : AITarget(params)
{
  m_dwRange = params.getTableInt("range");
}

bool WeaponPetUserAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return false;

  /*WeaponPetNpc* pPetNpc = dynamic_cast<WeaponPetNpc*> (pNpc);
  if (pPetNpc == nullptr)
    return false;
    */
  if (pNpc->getNpcType() != ENPCTYPE_WEAPONPET && pNpc->getNpcType() != ENPCTYPE_PETNPC)
    return false;
  SceneUser* pMasterUser = pNpc->getMasterUser();
  if (pMasterUser == nullptr)
    return false;

  std::set<SceneUser*> teamers;
  if (m_oParams.getTableInt("include_team"))
    teamers = pMasterUser->getTeamSceneUser();

  if (teamers.empty())
    teamers.insert(pMasterUser);

  std::list<xSceneEntryDynamic*> tarlist;
  for (auto &s : teamers)
  {
    if (getDistance(pNpc->getPos(), s->getPos()) > (float)m_dwRange)
      continue;

    if (s == pMasterUser)
      tarlist.push_front(s);
    else
      tarlist.push_back(s);
  }
  if (m_oParams.getTableInt("include_pet"))
  {
    std::list<SceneNpc*> petlist;
    for (auto &s : teamers)
    {
      petlist.clear();
      s->getWeaponPet().getPetNpcs(petlist);
      for (auto &p : petlist)
      {
        if (getDistance(pNpc->getPos(), p->getPos()) > (float)m_dwRange)
          continue;
        tarlist.push_back(p);
      }
    }
  }
  if (tarlist.empty())
    return false;

  if (m_oParams.has("hprangeper"))
  {
    float min = m_oParams.getMutableData("hprangeper").getTableFloat("1");
    float max = m_oParams.getMutableData("hprangeper").getTableFloat("2");
    for (auto s = tarlist.begin(); s != tarlist.end(); )
    {
      float maxhp = (*s)->getAttr(EATTRTYPE_MAXHP);
      if (maxhp == 0)
        return false;
      float per = (*s)->getAttr(EATTRTYPE_HP) / maxhp * 100;
      if (per >= min && per <= max)
      {
        ++s;
        continue;
      }
      s = tarlist.erase(s);
    }
  }

  if (tarlist.empty())
    return false;

  targetSet.insert(*(tarlist.begin()));
  return true;
}

BeingMasterAITarget::BeingMasterAITarget(const xLuaData& params) : AITarget(params)
{
  auto f = [&](const string& key, xLuaData& d)
  {
    m_setBuff.insert(d.getInt());
  };
  m_oParams.getMutableData("buffid").foreach(f);
  m_bLockTarget = m_oParams.getTableInt("locktarget") == 1;
  m_bMasterWhenLock = m_oParams.getTableInt("materwhenlock") == 1;
}

bool BeingMasterAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return false;

  xSceneEntryDynamic* pTarget = nullptr;
  if (m_bLockTarget)
  {
    //BeingNpc* p = dynamic_cast<BeingNpc*>(pNpc);
    SceneUser* master = pNpc->getMasterUser();
    if (master && pNpc->getMasterCurLockID() != 0)
    {
      if (m_bMasterWhenLock)
        pTarget = master;
      else
        //pTarget = xSceneEntryDynamic::getEntryByID(master->getUserBeing().getMasterCurLockID());
        pTarget = xSceneEntryDynamic::getEntryByID(pNpc->getMasterCurLockID());
    }
  }
  else
  {
    pTarget = pNpc->getMasterUser();
  }

  if (pTarget == nullptr)
    return false;

  if (m_oParams.has("hprangeper"))
  {
    float min = m_oParams.getMutableData("hprangeper").getTableFloat("1");
    float max = m_oParams.getMutableData("hprangeper").getTableFloat("2");
    float maxhp = pTarget->getAttr(EATTRTYPE_MAXHP);
    if (maxhp == 0)
      return false;
    float per = pTarget->getAttr(EATTRTYPE_HP) / maxhp * 100;
    if (per < min || per > max)
      return false;
  }
  if (m_setBuff.empty() == false)
  {
    bool havebuff = false;
    for (auto v : m_setBuff)
      if (pTarget->m_oBuff.haveBuff(v))
      {
        havebuff = true;
        break;
      }
    if (havebuff)
      return false;
  }

  targetSet.insert(pTarget);
  return true;
}

UsingSkillAITarget::UsingSkillAITarget(const xLuaData& params) : AITarget(params)
{
  auto f = [&](const string& key, xLuaData& data)
  {
    if (data.getInt() != 0)
      m_setSkillIDs.insert(data.getInt());
  };
  m_oParams.getMutableData("skills").foreach(f);
  m_dwRange = params.getTableInt("range");
  m_bFirstChant = params.getTableInt("first_chant") == 1;
}

bool UsingSkillAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr || m_dwRange == 0 || m_setSkillIDs.empty())
    return false;

  xSceneEntrySet uSet;
  pNpc->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, pNpc->getPos(), m_dwRange, uSet);

  targetSet.clear();
  QWORD minChantEndTime = QWORD_MAX;
  xSceneEntry* pTarget = nullptr;
  for (auto& s : uSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*>(s);
    if (user == nullptr)
      continue;

    DWORD skillid = user->m_oSkillProcessor.getRunner().getCFG() ? user->m_oSkillProcessor.getRunner().getCFG()->getSkillID() : 0;
    auto it = m_setSkillIDs.find(skillid);
    if (it == m_setSkillIDs.end())
      continue;

    if (m_bFirstChant)
    {
      if (user->m_oSkillProcessor.getRunner().getState() != ESKILLSTATE_CHANT || user->m_oSkillProcessor.getRunner().getChantEndTime() >= minChantEndTime)
        continue;

      minChantEndTime = user->m_oSkillProcessor.getRunner().getChantEndTime();
      pTarget = s;
    }
    else
    {
      targetSet.insert(s);
    }
  }

  if (pTarget)
    targetSet.insert(pTarget);

  return !targetSet.empty();
}

VisitorAITarget::VisitorAITarget(const xLuaData& params) : AITarget(params)
{
}

bool VisitorAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return false;

  QWORD targetid = pNpc->getLastVisitor();
  if (targetid <= 0)
    return false;

  xSceneEntryDynamic* pTarget = xSceneEntryDynamic::getEntryByID(targetid);
  if (pTarget == nullptr || pTarget->isAlive() == false)
    return false;

  targetSet.insert(pTarget);
  return true;
}

NearMasterAITarget::NearMasterAITarget(const xLuaData& params) : AITarget(params)
{
  m_fNearRange = params.getTableFloat("range");
}

bool NearMasterAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->define.m_oVar.m_qwNpcOwnerID == 0)
    return false;

  SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(pNpc->define.m_oVar.m_qwNpcOwnerID);
  if (pMaster == nullptr)
    return false;

  if (getXZDistance(pMaster->getPos(), pNpc->getPos()) < m_fNearRange)
  {
    targetSet.insert(pNpc);
    targetSet.insert(pMaster);
    return true;
  }
  return false;
}

BetweenMasterAITarget::BetweenMasterAITarget(const xLuaData& params) : AITarget(params)
{
  m_fWidth = params.getTableFloat("width");
}

bool BetweenMasterAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->define.m_oVar.m_qwNpcOwnerID == 0)
    return false;
  Scene* pScene = pNpc->getScene();
  if (pScene == nullptr)
    return false;

  SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(pNpc->define.m_oVar.m_qwNpcOwnerID);
  if (pMaster == nullptr || pMaster->check2PosInNine(pNpc) == false)
    return false;

  xSceneEntrySet set;
  pScene->getEntryIn2Pos(pNpc->getPos(), pMaster->getPos(), m_fWidth, set);

  for (auto &s : set)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user && pNpc->isMyEnemy(user) && pNpc->canAttack(user))
      targetSet.insert(user);
  }

  return !targetSet.empty();
}

//AITarget - 携带BUFF者
BufferAITarget::BufferAITarget(const xLuaData& params) : AITarget(params)
{
  m_dwBuffid = params.getTableInt("buffid");
  m_dwRange = params.getTableInt("range");
  m_dwNum = params.getTableInt("num");
  m_sType = params.getTableString("type");
}

bool BufferAITarget::getEntry(SceneNpc* pNpc, AICondition* pAICond, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr || m_dwRange == 0 ||  m_dwBuffid == 0)
    return false;

  SCENE_ENTRY_TYPE entryType = SCENE_ENTRY_USER;

  bool m_bJustMonster = false;
  bool m_bJustNormalMonster = false;
  bool m_bJustMvp = false;
  bool m_bJustMini = false;
  bool m_bJustPetnpc = false;
  bool m_bJustWeaponpet = false;

  if (m_sType == "player")
  {
    entryType = SCENE_ENTRY_USER;
  }
  else if (m_sType == "npc")
  {
    entryType = SCENE_ENTRY_NPC;
  }
  else if (m_sType == "monster")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustMonster = true;
  }
  else if (m_sType == "normal_monster")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustNormalMonster = true;
  }
  else if (m_sType == "mvp")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustMvp = true;
  }
  else if (m_sType == "mini")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustMini = true;
  }
  else if (m_sType == "petnpc")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustPetnpc = true;
  }
  else if (m_sType == "weaponpet")
  {
    entryType = SCENE_ENTRY_NPC;
    m_bJustWeaponpet = true;
  }
  else 
    return false;

  xSceneEntrySet uSet;
  pNpc->getScene()->getEntryListInBlock(entryType,pNpc->getPos(),m_dwRange,uSet);
  for (auto &s :uSet)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(s);
    if (pEntry == nullptr || !pEntry->isAlive())
      continue;

    if (!pEntry->m_oBuff.haveBuff(m_dwBuffid))
      continue;

    if (entryType == SCENE_ENTRY_NPC)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*>(s);
      if (npc == nullptr)
        continue;
      if (m_bJustMonster && npc->isMonster() == false)
        continue;
      if (m_bJustNormalMonster && npc->getNpcType() != ENPCTYPE_MONSTER)
        continue;
      if (m_bJustMvp && npc->getNpcType() != ENPCTYPE_MVP)
        continue;
      if (m_bJustMini && npc->getNpcType() != ENPCTYPE_MINIBOSS)
        continue;
      if (m_bJustPetnpc && npc->getNpcType() != ENPCTYPE_PETNPC)
        continue;
      if (m_bJustWeaponpet && npc->getNpcType() != ENPCTYPE_WEAPONPET)
        continue;
    }

    targetSet.insert(s);
    if(m_dwNum && targetSet.size() >= m_dwNum)
      return true;
  }
  return !targetSet.empty();
}

