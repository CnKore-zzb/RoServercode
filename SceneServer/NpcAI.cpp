#include "NpcAI.h"
#include "SceneNpc.h"
#include "NpcAIDefine.h"
#include "TableManager.h"
#include "SkillManager.h"
#include "SkillItem.h"
#include "SceneItemManager.h"
#include "ItemManager.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "MiscConfig.h"

NpcAI::NpcAI(SceneNpc *npc):_three_sec(3), _one_sec(1), m_pNpc(npc)
{
  fsm = NEW xStateMachine<SceneNpc>(m_pNpc, NormalNpcState::instance());
  clear();
}

NpcAI::~NpcAI()
{
  SAFE_DELETE(fsm);
}

void NpcAI::timer()
{
  //if (m_pNpc->base->dwID > 10000)
  //{
    if (fsm)
      fsm->update();
  //}
}

void NpcAI::clear()
{
  setCurLockID(0);
  //m_dwNextSkillID = 0;
  m_dwMaxAttackCount = 0;
  m_dwLastAttackTime = 0;
  m_flTargetDis = 0.0f;
  m_attacklist.clear();
  m_setImmuneList.clear();
  m_setJealousList.clear();
  //m_oSkillCD.clear();
  changeState(ENPCSTATE_NORMAL);
  m_qwTauntUser = 0;
  m_qwPriAttackUserID = 0;
  //m_oldHp = m_pNpc->getAttr(EATTRTYPE_HP);
}

bool NpcAI::normal()
{
  if (switchSleepTraget() == true)
  {
    changeState(ENPCSTATE_SLEEP);
    return false;
  }

  // follower
  //if (m_pNpc->m_qwFollowerID != 0)
  if (m_pNpc->getFollowerID() != 0)
  {
    autoMove();
    return true;
  }

  if (fearRun() == true)
    return true;

  // attack
  if (switchAttackTarget(false) == true)
  {
    changeState(ENPCSTATE_ATTACK);
    m_pNpc->m_oEmoji.check("Encount");
    return false;
  }

  // pickup
  if (switchPickTarget() == true)
  {
    changeState(ENPCSTATE_PICKUP);
    return false;
  }

  m_pNpc->m_sai.checkSig("normal");

  // see dead
  DWORD cur = now();
  if (cur > m_dwNextSearchTime)
  {
    m_dwNextSearchTime = cur + 1;
    if (isExpelDead() || isAlertFakeDead())
    {
      const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
      float alertRange = rCFG.dwAlert.dwRange;
      float expelRange = rCFG.dwExpel.dwRange;
      float range = alertRange > expelRange ? alertRange : expelRange;
      xSceneEntrySet uSet;
      if (m_pNpc->getScene() == nullptr)
        return false;
      m_pNpc->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, m_pNpc->getPos(), range, uSet);
      for (auto &s : uSet)
      {
        SceneUser* user = dynamic_cast<SceneUser*> (s);
        if (user == nullptr)
          continue;
        if (user->getStatus() != ECREATURESTATUS_DEAD && user->getStatus() != ECREATURESTATUS_FAKEDEAD)
          continue;
        onSeeDead(user->id);
        return false;
      }
    }
    const SNpcCFG* pCFG = m_pNpc->getCFG();
    if (pCFG == nullptr)
	{
	  XLOG << "[NpcAI]" << "goline114" << XEND;
      return false;
	}
    if (pCFG->stNpcReaction.isReaction(EREACTTYPE_ENEMY))
    {
      xSceneEntrySet uSet;
      DWORD range = FeatureConfig::getMe().getNpcAICFG().stReaction.dwRange;
	  XLOG << "[NpcAI]" << "range:"<< range << XEND;
      m_pNpc->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, m_pNpc->getPos(), range, uSet);
      for (auto &s : uSet)
      {
		XLOG << "[NpcAI]" << "for" << XEND;
        SceneUser* user = dynamic_cast<SceneUser*>(s);
        if (canNormalLock(user) == false)
          continue;
        if (user == nullptr || checkReaction(user, EREACTTYPE_ENEMY) == false)
          continue;
        m_pNpc->checkEmoji("PassiveActive");
        setCurLockID(user->id);
        changeState(ENPCSTATE_ATTACK);
        return false;
      }
    }
    if (isAngerHand())
    {
      DWORD range = FeatureConfig::getMe().getNpcAICFG().stJealous.dwRange;
      xSceneEntrySet uSet;
      m_pNpc->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, m_pNpc->getPos(), range, uSet);
      for (auto &s : uSet)
      {
        SceneUser* user = dynamic_cast<SceneUser*>(s);
        if (user == nullptr || user->m_oHands.has() == false || user->m_oHands.isInWait())
          continue;
        if (m_setJealousList.find(user->id) != m_setJealousList.end())
          continue;
        m_pNpc->m_oMove.stop();

        NpcChangeAngle cmd;
        cmd.set_guid(m_pNpc->id);
        cmd.set_targetid(user->id);
        DWORD angle = calcAngle(m_pNpc->getPos(), user->getPos());
        cmd.set_angle(angle);
        PROTOBUF(cmd, send, len);
        m_pNpc->sendCmdToNine(send, len);

        DWORD emojiID = FeatureConfig::getMe().getNpcAICFG().stJealous.dwEmoji;
        m_pNpc->playEmoji(emojiID);

        m_setJealousList.insert(user->m_oHands.getMasterID());
        m_setJealousList.insert(user->m_oHands.getFollowerID());
      }
    }
  }

  // normal skill
  m_dwCurSkillID = getAttackSkillID();
  //XLOG << "[NPCAI]" << "m_dwCurSkillID:" << m_dwCurSkillID << XEND;
  if (m_dwCurSkillID != 0)
  {
    setCurLockID(m_pNpc->id);
    processUseSkill();
    //changeState(ENPCSTATE_ATTACK);
    return false;
  }

  // move
  if (m_pNpc->m_oMove.empty() && getTerritory() && isMoveable() && m_pNpc->isMonster())
  {
    if (_three_sec.timeUp(now()) == false)
      return true;

    m_pNpc->m_oEmoji.check("Wait");

    DWORD fre = m_pNpc->getCFG() ? m_pNpc->getCFG()->dwMoveFrequency : 15;
    fre = fre != 0 ? fre : 15;
    if (selectByPercent(fre) == false)
    {
      _three_sec.reset();
      return true;
    }

    xPos p;
    m_pNpc->getScene()->getRandPos(m_pNpc->getBirthPos(), getTerritory(), p);
    std::list<xPos> path;
    if (m_pNpc->getScene()->findingPath(m_pNpc->getPos(), p, path, TOOLMODE_PATHFIND_FOLLOW))
    {
      DWORD rand = randBetween(2, 5);
      DWORD i = 0;
      for (auto &it : path)
      {
        p = it;
        if (i++ == rand)
          break;
      }
    }
    moveTo(p);
  }

  autoMove();
  return true;
}

bool NpcAI::attack()
{
  XLOG << "[NPCAI]" << "attack_ok" << XEND;
  if (stateChange() == false)
    return false;
  if (!isAttackBack())
    return false;

  if (m_pNpc->getScene() == nullptr)
    return false;
  if (!m_pNpc->m_oMove.checkActionTime())
    return true;
  if (m_pNpc->isMask())
    return false;
  if (m_pNpc->m_oSkillProcessor.getRunner().getState() != ESKILLSTATE_MIN)
    return true;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(getCurLockID());
  if (pUser != nullptr)
  {
    if (m_pNpc->checkNineScreenShow(pUser) == false && (m_pNpc->getNpcType() != ENPCTYPE_PETNPC && m_pNpc->getNpcType() != ENPCTYPE_WEAPONPET))
    {
      setCurLockID(0);
      return false;
    }
    if (pUser->isAlive() == false)
    {
      setCurLockID(0);
      return false;
    }
    if (isReckless() == false)
    {
      if (pUser->getStatus() == ECREATURESTATUS_FAKEDEAD)
      {
        setCurLockID(0);
        return false;
      }
    }
  }

  if (m_dwCurSkillID == 0)
    m_dwCurSkillID = getAttackSkillID();

  processUseSkill();

  m_pNpc->m_sai.checkSig("attack");

  if (getCurLockID() == 0)
    return false;

  return true;
}

bool NpcAI::pickup()
{
  if (isPickup() == false)
    return false;
  if (m_pNpc == nullptr || m_pNpc->getScene() == nullptr)
    return false;
  MonsterNpc* pNpc = dynamic_cast<MonsterNpc*>(m_pNpc);
  if (pNpc == nullptr)
    return false;
  if (!pNpc->m_oMove.checkActionTime())
    return true;
  if (pNpc->isMask())
    return false;
  if (m_attacklist.empty() == false)
    return false;

  SceneItem* pItem = SceneItemManager::getMe().getSceneItem(getCurItemID());
  if (pItem == nullptr || pItem->getStatus() == ESCENEITEMSTATUS_INVALID)
  {
    m_pNpc->m_oMove.stop();
    setCurItemID(0);
    return false;
  }

  float dist = ::getDistance(m_pNpc->getPos(), pItem->getPos());
  if (m_pNpc->m_oMove.empty() == true && dist > 0.5f)
  {
    xPos p = getPosByDir(pItem->getPos(), m_pNpc->getPos(), 0.2f);
    moveTo(p);
    autoMove();
    return true;
  }

  if (m_pNpc->m_oMove.empty() == false)
  {
    autoMove();
    return true;
  }

  pNpc->addPickItem(pItem->getItemInfo());
  pItem->leaveScene();
  setCurItemID(0);

  return true;
}


bool NpcAI::wait()
{
  return now() < m_dwWaitExitTime;
}

bool NpcAI::gocamera()
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(getCurLockID());
  if (pUser == nullptr || pUser->getStatus() != ECREATURESTATUS_PHOTO)
  {
    m_pNpc->m_oBuff.del(24380);
    return false;
  }

  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  float angle = pUser->getUserSceneData().getDir();// pUser->getAngle();
  xPos userPos = pUser->getPos();
  xPos tarPos;
  float dis = rCFG.dwGoCamera.dwRangeStop;
  tarPos.x = userPos.x + dis * sin(angle * 3.14 / 180);
  tarPos.z = userPos.z + dis * cos(angle * 3.14 / 180);
  tarPos.y = userPos.y;

  if (getDistance(tarPos, m_pNpc->getPos()) < 0.2f)
  {
    DWORD buff = rCFG.dwSmile.dwBuff;
    DWORD spdBuff = rCFG.dwGoCamera.dwBuff;
    DWORD val = rCFG.dwGoCamera.dwInterval;
    // 卖萌
    DWORD emojiID = m_pNpc->getSmileEmoji();
    DWORD actionID = m_pNpc->getSmileAction();
    if (emojiID != 0)
    {
      m_pNpc->m_oEmoji.play(emojiID);
    }
    if (actionID != 0)
    {
      UserActionNtf cmd;
      cmd.set_charid(m_pNpc->id);
      cmd.set_value(actionID);
      cmd.set_type(EUSERACTIONTYPE_MOTION);
      PROTOBUF(cmd, send2, len2);
      m_pNpc->sendCmdToNine(send2, len2);
    }

    m_pNpc->m_oBuff.add(buff);
    m_pNpc->m_oBuff.del(spdBuff);
    m_dwNextGocameraTime = now() + val;
    setCurLockID(0);
    return false;
  }
  if (m_pNpc->m_oMove.empty() || _three_sec.timeUp(now()))
  {
    moveTo(tarPos);
    _three_sec.reset();
  }

  autoMove();
  return true;
}

void NpcAI::onFocusByCamera(QWORD userid)
{
  if (isGocamera() || isSmileCamera() == false)
    return;
  DWORD cur = now();
  if (cur < m_dwNextSmileTime)
    return;
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr)
    return;

  m_pNpc->m_oMove.stop();

  if (getStateType() == ENPCSTATE_WAIT)
    setStateType(ENPCSTATE_NORMAL);

  // 广播转身
  NpcChangeAngle cmd;
  cmd.set_guid(m_pNpc->id);
  cmd.set_targetid(userid);
  DWORD angle = calcAngle(m_pNpc->getPos(), pUser->getPos());
  cmd.set_angle(angle);
  PROTOBUF(cmd, send, len);
  m_pNpc->sendCmdToNine(send, len);

  //播放表情
  DWORD emojiID = m_pNpc->getSmileEmoji();
  DWORD actionID = m_pNpc->getSmileAction();
  if (emojiID != 0)
  {
    m_pNpc->m_oEmoji.play(emojiID);
  }
  if (actionID != 0)
  {
    UserActionNtf cmd;
    cmd.set_charid(m_pNpc->id);
    cmd.set_value(actionID);
    cmd.set_type(EUSERACTIONTYPE_MOTION);
    PROTOBUF(cmd, send2, len2);
    m_pNpc->sendCmdToNine(send2, len2);
  }
  //存储转身状态
  m_dwAngle = angle;

  // wait 3秒后恢复原本角度
  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  setStateType(ENPCSTATE_SMILE);
  setWaitExitTime(cur + rCFG.dwSmile.dwStayTime);
  changeState(ENPCSTATE_WAIT);

  // 一段时间内不可重复转身卖萌
  m_dwNextSmileTime = cur + rCFG.dwSmile.dwInterval;
}

bool NpcAI::smile()
{
  // 卖萌结束,角度复位
  if (getFormerState() != ENPCSTATE_WAIT)
    return true;
  if (m_pNpc->isMonster() == true)
    return false;
  NpcChangeAngle cmd;
  cmd.set_guid(m_pNpc->id);
  cmd.set_targetid(0);
  cmd.set_angle(m_pNpc->define.getDir());
  PROTOBUF(cmd, send, len);
  m_pNpc->sendCmdToNine(send, len);
  return false;
}

bool NpcAI::sleep()
{
  DWORD curTime = now();
  if (!isNight(curTime))
  {
     changeState(ENPCSTATE_NORMAL);
     return true;
  }

  // attack
  if (getCurLockID())
  {
    changeState(ENPCSTATE_ATTACK);
    m_pNpc->playEmoji(1); //惊叹
    return true;
  }

  if (curTime > m_dwNextSleepActionTime)
  {
    m_pNpc->m_oEmoji.check("BeSleep");

    UserActionNtf cmd;
    cmd.set_charid(m_pNpc->id);
    cmd.set_value(0);
    cmd.set_type(EUSERACTIONTYPE_MOTION);
    PROTOBUF(cmd, send2, len2);
    m_pNpc->sendCmdToNine(send2, len2);
    m_dwNextSleepActionTime = curTime + 5;
  }
  return true;
}

// check be chant reaction
void NpcAI::onBeChantLock(QWORD attackerid)
{
  if (isNaughty()) checkNaughty(attackerid);
  else if (isEndure()) checkEndure(attackerid);
}

/* ------------ 被锁定开霸体，攻击玩家 ----------*/
void NpcAI::checkEndure(QWORD attackerid)
{
  if (isEndure() == false)
    return;
  if (m_dwCanEndureTime > now())
    return;
  const SNpcEndureCFG& rCFG = FeatureConfig::getMe().getNpcAICFG().stEndureCFG;
  m_dwCurSkillID = rCFG.dwSkill;
  m_dwCanEndureTime = now() + rCFG.dwCD;
  setCurLockID(attackerid);
  changeState(ENPCSTATE_ATTACK);
}

/*-------------------------------------顽皮-------------------------------------*/
void NpcAI::checkNaughty(QWORD attackerid)
{
  if (isNaughty() == false || m_pNpc->isAttrCanSkill() == false)
    return;
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(attackerid);
  if (pUser == nullptr)
    return;

  // 重置wait状态, wait 状态用于判断
  if (getStateType() == ENPCSTATE_WAIT)
    setStateType(ENPCSTATE_NORMAL);

  setCurLockID(attackerid);

  m_pNpc->m_oMove.stop();

  // 转向
  NpcChangeAngle cmd1;
  cmd1.set_guid(m_pNpc->id);
  cmd1.set_targetid(pUser->id);
  DWORD angle = calcAngle(m_pNpc->getPos(), pUser->getPos());
  cmd1.set_angle(angle);
  PROTOBUF(cmd1, send1, len1);
  m_pNpc->sendCmdToNine(send1, len1);

  // 惊讶
  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  m_pNpc->m_oEmoji.play(rCFG.dwNaughty.dwFormerEmoji);

  setStateType(ENPCSTATE_NAUGHTY);
  m_pNpc->m_ai.setWaitExitTime(now() + rCFG.dwNaughty.dwResponseTime);
  changeState(ENPCSTATE_WAIT);
}

bool NpcAI::naughty()
{
  if (isNaughty() == false || m_pNpc->isAttrCanSkill() == false)
    return false;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(getCurLockID());
  if (pUser == nullptr)
    return false;

  DWORD cur = now();
  // after waiting one second, useskill "扔石头"
  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  if (getFormerState() == ENPCSTATE_WAIT)
  {
    m_dwFormerState = ENPCSTATE_NAUGHTY;
    DWORD skillid = rCFG.dwNaughty.dwSkillID;
    if (m_pNpc->useSkill(skillid, pUser->id, pUser->getPos()) == false)
    {
      return false;
    }
    _one_sec.reset();
  }
  bool timeIsup = _one_sec.timeUp(cur);
  // judge if break user's skill, if true, do expression
  if (getFormerState() == ENPCSTATE_NAUGHTY)
  {
    if (timeIsup && m_breakUserChant)
    {
      m_breakUserChant = false;
      m_pNpc->m_oEmoji.play(rCFG.dwNaughty.dwLatterEmoji);
      return false;
    }
  }

  if (timeIsup)
  {
    changeState(ENPCSTATE_ATTACK);
    return true;
  }
  return true;
}

void NpcAI::onSeeDead(QWORD userid)
{
  // attack prior
  if (getStateType() != ENPCSTATE_NORMAL || getStateType() == ENPCSTATE_SEEDEAD)
    return;

  if (!isExpelDead() && !isAlertFakeDead())
    return;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr)
    return;

  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();

  DWORD cur = now();
  if (isExpelDead())
  {
    if (pUser->getStatus() != ECREATURESTATUS_DEAD && pUser->getStatus() != ECREATURESTATUS_FAKEDEAD)
      return;
    if (cur < m_dwNextExpelTime)
      return;
    if (getDistance(m_pNpc->getPos(), pUser->getPos()) > rCFG.dwExpel.dwRange)
      return;
  }
  else if (isAlertFakeDead())
  {
    if (pUser->getStatus() != ECREATURESTATUS_FAKEDEAD)
      return;
    if (cur < m_dwNextAlertTime)
      return;
    if (getDistance(m_pNpc->getPos(), pUser->getPos()) > rCFG.dwAlert.dwRange)
      return;
  }

  m_pNpc->m_oMove.stop();
  setCurLockID(userid);
  changeState(ENPCSTATE_SEEDEAD);
}

bool NpcAI::seedead()
{
  if (isExpelDead())
    return expelDead();
  else if (isAlertFakeDead())
    return alertDead();
  return false;
}

bool NpcAI::alertDead()
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(getCurLockID());
  if (pUser == nullptr)
    return false;
  if (pUser->getStatus() != ECREATURESTATUS_FAKEDEAD)
    return false;

  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  if (getFormerState() == ENPCSTATE_NORMAL)
  {
    m_pNpc->m_oEmoji.play(rCFG.dwAlert.dwEmoji);

    setWaitExitTime(rCFG.dwAlert.dwResponseTime + now());
    changeState(ENPCSTATE_WAIT);
    return true;
  }

  DWORD skillid = rCFG.dwAlert.dwSkillID;
  const BaseSkill* pSkillCFG =  SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
    return false;

  float skillDist = pSkillCFG->getLaunchRange(m_pNpc);
  float dis = getDistance(m_pNpc->getPos(), pUser->getPos());
  if (dis > skillDist + 0.05f && skillDist != 0)
  {
    if (m_pNpc->m_oMove.empty())
    {
      moveTo(pUser->getPos());
    }
    autoMove();
    return true;
  }

  pUser->getAchieve().onDead(true, 0);
  m_pNpc->useSkill(skillid, pUser->id, pUser->getPos());
  m_dwNextAlertTime = now() + rCFG.dwAlert.dwInterval;
  setCurLockID(0);
  return false;
}

bool NpcAI::expelDead()
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(getCurLockID());
  if (pUser == nullptr)
    return false;
  if (pUser->getStatus() != ECREATURESTATUS_DEAD && pUser->getStatus() != ECREATURESTATUS_FAKEDEAD)
    return false;

  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  DWORD skillid = rCFG.dwExpel.dwSkillID;
  const BaseSkill* pSkillCFG =  SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
    return false;

  float skillDist = pSkillCFG->getLaunchRange(m_pNpc);
  float dis = getDistance(m_pNpc->getPos(), pUser->getPos());
  if (dis > skillDist + 0.05f && skillDist != 0)
  {
    if (m_pNpc->m_oMove.empty())
    {
      moveTo(pUser->getPos());
    }
    autoMove();
    return true;
  }
  m_pNpc->m_oEmoji.play(rCFG.dwExpel.dwEmoji);

  pUser->getAchieve().onDead(true, 0);
  DWORD cur = now();
  m_pNpc->useSkill(skillid, pUser->id, pUser->getPos());
  m_dwNextExpelTime = cur + rCFG.dwExpel.dwInterval;
  DWORD chanttime = pSkillCFG->getReadyTime(m_pNpc);
  if (chanttime != 0)
  {
    setStateType(ENPCSTATE_NORMAL);
    setWaitExitTime(cur + chanttime / 1000 + 1);
    changeState(ENPCSTATE_WAIT);
    return true;
  }
  return false;
}

bool NpcAI::switchAttackTarget(bool switchFlag)
{
  if (m_pNpc == nullptr || m_pNpc->getAttr(EATTRTYPE_NOATTACK) != 0)
  {
	  XLOG << "[switchAttackTarget]" << "1" << XEND;
	  return false;
  }
    

  BYTE mode = getSwitchTargetMode();
  

  xSceneEntrySet targetSet;
  searchAttackTarget(targetSet, getSearchRange());

  xSceneEntryDynamic *pEntry = getAttackTarget(targetSet, mode, switchFlag);
  
  /* if (NPC_SWITCH_TARGET_LAST_ATTACKER == mode)
  {
	  XLOG << "[switchAttackTarget]" << "61" << XEND;
    if (!m_attacklist.empty())
    {
      QWORD tid = m_attacklist.back();
      m_attacklist.pop_back();

      xSceneEntryDynamic *pTarget = xSceneEntryDynamic::getEntryByID(tid);
      if (pTarget)
      {
        if (!checkDistance(pTarget->getPos(), getSearchPos(), getSearchRange()))
        {
          return false;
        }
      }
      else
      {
        return false;
      }

      setCurLockID(tid);
      return true;
    }
  } */
  
  
  //XLOG << "[switchAttackTarget]" << "1pEntryID:"<< "p1" << XEND;
  if (pEntry)
  {
	XLOG << "[switchAttackTarget]" << "2" << XEND;
    setCurLockID(pEntry->id);
    return true;
  }

  bool clearAttackListFlag = false;
  if (/*targetSet.empty() && */!m_attacklist.empty())
  {
	//XLOG << "[switchAttackTarget]" << "3" << XEND;
    for (auto iter = m_attacklist.begin(); iter != m_attacklist.end(); ++iter)
    {
      xSceneEntryDynamic *pTarget = xSceneEntryDynamic::getEntryByID(*iter);
	 //XLOG << "[switchAttackTarget]" << "iter:" << *iter << XEND;
	  //XLOG << "[switchAttackTarget]" << "pTarget:" << pTarget->getPos() << XEND;
      if (pTarget && checkDistance(pTarget->getPos(), getSearchPos(), getPursueDis()) )
      {
		//XLOG << "[switchAttackTarget]" << "targetSet" << XEND;
		XLOG << "[switchAttackTarget]" << "pTarget_id" << pTarget->id << XEND;
        targetSet.insert(pTarget);
      }
    }
    clearAttackListFlag = true;
  }

  pEntry = getAttackTarget(targetSet, mode, switchFlag);
  //XLOG << "[switchAttackTarget]" << "2pEntryID:"<< "p2" << XEND;
  if (pEntry)
  {
	XLOG << "[switchAttackTarget]" << "2pEntryID:"<< pEntry->id << XEND;
	XLOG << "[switchAttackTarget]" << "AAA" << XEND;  
    if (clearAttackListFlag)
	{
		XLOG << "[switchAttackTarget]" << "BBB" << XEND; 
		m_attacklist.erase(pEntry->id);
	}
    setCurLockID(pEntry->id);
    return true;
  }

  xSceneEntryDynamic *pTarget = xSceneEntryDynamic::getEntryByID(getCurLockID());
  if (pTarget)
  {
	XLOG << "[switchAttackTarget]" << "4" << XEND;
    if (checkDistance(pTarget->getPos(), getSearchPos(), getPursueDis()))
	{
		XLOG << "[switchAttackTarget]" << "5" << XEND;
		return true;
	}
      
  }

  setCurLockID(0);
  if (m_dwStateType != ENPCSTATE_NORMAL)
  {
	  XLOG << "[switchAttackTarget]" << "6" << XEND;
	  changeState(ENPCSTATE_NORMAL);
  }
    

  return false;
}

bool NpcAI::switchSleepTraget()
{
  if (isSleep() == false)
    return false;

  if (m_pNpc->getScene() == nullptr)
    return false;

  DWORD curTime = now();
  //check is night
  if (!isNight(curTime))
    return false;

 // check time
  if (curTime < m_dwNormalStartTime + 5)
  {
    return false;
  }

  return true;
}

void NpcAI::changeState(ENpcState type)
{
  if (!fsm) return;
  if (getStateType() == type)
    return;
  m_blAttackState = false;
  m_dwAttackStartTime = 0;
  setStateType(type);
  switch (type)
  {
    case ENPCSTATE_NORMAL:
      setCurLockID(0);
      fsm->changeState(NormalNpcState::instance());
      m_dwNormalStartTime = xTime::getCurSec();
      break;
    case ENPCSTATE_ATTACK:
      {
        if (!isAttackBack()) return;
        m_pNpc->m_oMove.clearPath();
        fsm->changeState(AttackNpcState::instance());
        m_blAttackState = true;
        m_dwAttackStartTime = xTime::getCurSec();
      }
      break;
    case ENPCSTATE_MOVE:
      fsm->changeState(MoveToPosNpcState::instance());
      break;
    case ENPCSTATE_PICKUP:
      fsm->changeState(PickupNpcState::instance());
      break;
    case ENPCSTATE_WAIT:
      fsm->changeState(WaitNpcState::instance());
      break;
    case ENPCSTATE_CAMERA:
      fsm->changeState(CameraNpcState::instance());
      break;
    case ENPCSTATE_NAUGHTY:
      fsm->changeState(NaughtyNpcState::instance());
      break;
    case ENPCSTATE_SMILE:
      fsm->changeState(SmileNpcState::instance());
      break;
    case ENPCSTATE_SEEDEAD:
      fsm->changeState(SeeDeadNpcState::instance());
      break;
    case ENPCSTATE_SLEEP:
      fsm->changeState(SleepNpcState::instance());
      break;
    case ENPCSTATE_GOBACK:
      fsm->changeState(GoBackNpcState::instance());
      break;
    case ENPCSTATE_RUNAWAY:
      fsm->changeState(RunAwayNpcState::instance());
      break;
    default:
      break;
  }
}

BYTE NpcAI::getSwitchTargetMode()
{
  DWORD random = randBetween(1, 100);
  switch (random)
  {
    case 1 ... 50:
      return NPC_SWITCH_TARGET_RANDOM;
    case 51 ... 70:
      return NPC_SWITCH_TARGET_CLOSEST;
    case 71 ... 100:
      return NPC_SWITCH_TARGET_LAST_ATTACKER;
    default:
      break;
  }
  return NPC_SWITCH_TARGET_RANDOM;
}

xPos NpcAI::getSearchPos()
{
  return m_pNpc->getPos();
  return m_pNpc->getBirthPos();
}

DWORD NpcAI::getSearchRange()
{
  if (m_dwSearchRange)
    return m_dwSearchRange;
  if (m_pNpc->define.getSearch())
    return m_pNpc->define.getSearch();

  return 9;
}

BYTE NpcAI::getSkillRange()
{
  /*
  if (!m_dwNextSkillID)
  {
    getAttackSkill(m_dwNextSkillID);
  }
  if (!m_dwNextSkillID) return 0;

  SkillBase *skill = skillBaseM->getDataByID(m_dwNextSkillID);
  if (NULL==skill)
  {
    XERR("[npc攻击],%llu,%llu,%s,技能表无数据:%u", m_pNpc->getTempID(), m_pNpc->id, m_pNpc->name, m_dwNextSkillID);
    return 0;
  }
  */

  return 9;
  // return skill->launchRange;
}

bool NpcAI::searchAttackTarget(xSceneEntrySet &set, BYTE range, BYTE mode)
{
  Scene *scene = m_pNpc->getScene();
  if (!scene) return false;

  if (!mode) mode = NPC_SEARCH_PASSIVE;
  if (m_pNpc->define.getSearch())
  {
    mode = NPC_SEARCH_USER_ACTIVE;
    if (m_pNpc->define.m_oVar.m_qwTeamID)
      mode = NPC_SEARCH_TEAM_ACTIVE;
    if (m_pNpc->define.m_oVar.m_qwQuestOwnerID)
      mode = NPC_SEARCH_PERSONAL_ACTIVE;
    if (m_pNpc->getNpcType() == ENPCTYPE_FRIEND)
      mode = NPC_SEARCH_NPC_ACTIVE;
  }
  switch (mode)
  {
    case NPC_SEARCH_NULL:
      return false;
    case NPC_SEARCH_USER_ACTIVE:    //主动搜索玩家
      {
        scene->getEntryListInBlock(SCENE_ENTRY_USER, getSearchPos(), range, set);
        removeAllyInEntryList(set);
      }
      break;
    case NPC_SEARCH_NPC_ACTIVE:     //主动搜索NPC(不同国家)
      {
        scene->getEntryListInBlock(SCENE_ENTRY_NPC, getSearchPos(), range, set);
        removeAllyInEntryList(set);
      }
      break;
    case NPC_SEARCH_ALL_ACTIVE:     //主动搜索玩家和NPC(不同国家)
      {
        scene->getEntryListInBlock(SCENE_ENTRY_USER, getSearchPos(), range, set);
        scene->getEntryListInBlock(SCENE_ENTRY_NPC, getSearchPos(), range, set);
        removeAllyInEntryList(set);
      }
      break;
    case NPC_SEARCH_NPC_USER:       //优先搜索NPC再玩家(不同国家)
      {
        scene->getEntryListInBlock(SCENE_ENTRY_NPC, getSearchPos(), range, set);
        removeAllyInEntryList(set);
        if (set.empty())
        {
          scene->getEntryListInBlock(SCENE_ENTRY_USER, getSearchPos(), range, set);
          removeAllyInEntryList(set);
        }
      }
      break;
    case NPC_SEARCH_USER_NPC:       //优先搜索玩家再NPC(不同国家)
      {
        scene->getEntryListInBlock(SCENE_ENTRY_USER, getSearchPos(), range, set);
        removeAllyInEntryList(set);
        if (set.empty())
        {
          scene->getEntryListInBlock(SCENE_ENTRY_NPC, getSearchPos(), range, set);
          removeAllyInEntryList(set);
        }
      }
      break;
    case NPC_SEARCH_PASSIVE:        //被动搜索
      break;
    case NPC_SEARCH_TEAM_ACTIVE:    //主动搜索所属某个队伍的玩家
      {
        scene->getEntryListInBlock(SCENE_ENTRY_USER, getSearchPos(), range, set);
        removeAllyInEntryList(set);
        for (auto s = set.begin(); s != set.end(); )
        {
          SceneUser* user = dynamic_cast<SceneUser*> (*s);
          auto tmp = s++;
          if (user == nullptr || user->getTeamID() != m_pNpc->define.m_oVar.m_qwTeamID)
          {
            set.erase(tmp);
            continue;
          }
        }
      }
      break;
    case NPC_SEARCH_PERSONAL_ACTIVE: // 只攻击某个玩家
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(m_pNpc->define.m_oVar.m_qwQuestOwnerID);
        if (pUser == nullptr)
          break;
        set.clear();
        if (getDistance(m_pNpc->getPos(), pUser->getPos()) <= range)
          set.insert(pUser);
      }
      break;
  }
  return true;
}

xSceneEntryDynamic* NpcAI::getAttackTarget(xSceneEntrySet &set, BYTE mode, bool switchFlag)
{
  // get prior list if have
  verifyAttackList(set);

  xSceneEntryDynamic *pEntry = NULL;
  // DWORD min = DWORD_MIN;
  DWORD max = DWORD_MAX;
  for (xSceneEntrySet::const_iterator iter = set.begin(); iter != set.end(); ++iter)
  {
    xSceneEntryDynamic *entry = (xSceneEntryDynamic *)(*iter);
    if (entry && entry->getEntryType() == SCENE_ENTRY_NPC)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (entry);
      if (!npc) continue;
      if (npc->isWeaponPet())
        continue;
      if (npc->getNpcType() == ENPCTYPE_SKILLNPC)
        continue;
      if (npc->getNpcType() == ENPCTYPE_PETNPC)
        continue;
      if (npc->getNpcType() == ENPCTYPE_BEING)
        continue;
      if (npc->getNpcType() == ENPCTYPE_ELEMENTELF)
        continue;
    }

    if (entry && entry->isAlive() && !entry->isMask() && (!switchFlag || (getCurLockID() != entry->id))
        && m_pNpc->isMyEnemy(entry))
    {
      switch (mode)
      {
        case NPC_SWITCH_TARGET_RANDOM:   //随机
        case NPC_SWITCH_TARGET_LOWEST_LEVEL:     //等级最低者
        case NPC_SWITCH_TARGET_HIGHEST_LEVEL:    //等级最高者
        case NPC_SWITCH_TARGET_MOST_HP:      //防御最低者
        case NPC_SWITCH_TARGET_HIGHEST_DAM: //攻击力最高者（它造成了最大的伤害）
        case NPC_SWITCH_TARGET_LEAST_HP:     //血量最少者
        case NPC_SWITCH_TARGET_LAST_ATTACKER:
          {
            DWORD rand = randBetween(1, 100);
            if (rand < max)
            {
              max = rand;
              pEntry = entry;
            }
          }
          break;
        case NPC_SWITCH_TARGET_CLOSEST:  //距离最近者
          {
            DWORD dist = getDistance(entry->getPos(), m_pNpc->getPos());
            if (dist < max)
            {
              max = dist;
              pEntry = entry;
            }
          }
          break;
        case NPC_SWITCH_TARGET_TEAM:
          {
            SceneNpc* pNpc = dynamic_cast<SceneNpc*>(entry);
            if (pNpc != nullptr && pNpc->define.getID() == m_pNpc->define.getID() && m_pNpc->m_ai.getCurLockID() != 0)
            {
              pEntry = entry;
            }
          }
          break;
        default:
          break;
      }
    }
  }
  /*if (m_pNpc->base->dwID == 20002)
  {
    if (pEntry && m_attacklist.find(pEntry->id) != m_attacklist.end())
      return pEntry;

    SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
    if (pUser && pUser->getPackage().getItemCount(12309) != 0)
    {
      if (hasImmuneID(pUser->id) == false)
      {
        addImmuneID(pUser->id);
        m_pNpc->checkEmoji("Birth");
      }
      return nullptr;
    }
  }*/
  /*if(pEntry)
  {
    if (!checkDistance(m_pNpc->getBirthPos(), pEntry->getPos(), getSearchRange()))
      return nullptr;
  }*/
  return pEntry;
}

void NpcAI::removeAllyInEntryList(xSceneEntrySet &targetSet)
{
  xSceneEntrySet::iterator iter = targetSet.begin(), delIter;
  while (iter != targetSet.end())
  {
    xSceneEntryDynamic *pEntry = (xSceneEntryDynamic *)(*iter);
    delIter = iter++;
    if (!canNormalLock(pEntry))
    {
      targetSet.erase(delIter);
      continue;
    }
    if (hasImmuneID(pEntry->id) && checkReaction(pEntry, EREACTTYPE_FRIEND))
    {
      targetSet.erase(delIter);
      continue;
    }
    /*if (m_pNpc->getNpcType() == ENPCTYPE_MINIBOSS || m_pNpc->getNpcType() == ENPCTYPE_MVP)
    {
      // ver01 : 修改为等级低于配置等级生效 申林 20161030
      if (m_pNpc->define.getAttSafeLv() != 0 && pEntry->getLevel() < m_pNpc->define.getAttSafeLv())//pEntry->getLevel() + m_pNpc->define.getAttSafeLv() < m_pNpc->getLevel())
      {
        targetSet.erase(delIter);
        continue;
      }
    }
    */
    if (getCurLockID() != pEntry->id)
    {
      const SNpcCFG* pCFG = m_pNpc->getCFG();
      if (m_qwPriAttackUserID != pEntry->id && pCFG != nullptr && pCFG->dwProtectAtkLv != 0 && pEntry->getLevel() < pCFG->dwProtectAtkLv)//pEntry->getLevel() + pCFG->dwProtectAtkLv < m_pNpc->getLevel())
      {
        targetSet.erase(delIter);
        continue;
      }

      if (isNoTransformAtt())
      {
        SceneUser* user = dynamic_cast<SceneUser*> (pEntry);
        if (user && user->getTransform().isMonster())
        {
          targetSet.erase(delIter);
          continue;
        }
      }
      if (m_setIgnoreProfession.empty() == false)
      {
        SceneUser* user = dynamic_cast<SceneUser*> (pEntry);
        if (user)
        {
          if (m_setIgnoreProfession.find((DWORD)user->getProfession()) != m_setIgnoreProfession.end())
          {
            targetSet.erase(delIter);
            continue;
          }
        }
      }
      if (pEntry->isNoActiveMonster() && m_pNpc->getNpcType() != ENPCTYPE_MVP)
      {
        targetSet.erase(delIter);
        continue;
      }
    }
  }
}

void NpcAI::autoMove()
{
  if (!isMoveable()) return;

  m_pNpc->m_oMove.heartbeat(xTime::getCurUSec() / ONE_THOUSAND);
}

bool NpcAI::checkRePathFinding(xPos pos)
{
  if (m_pNpc->m_oMove.empty()) return true;

  xPos target = m_pNpc->m_oMove.getStraightPoint();

  float d1 = getDistance(pos, m_pNpc->getPos());              //npc到玩家距离
  float d2 = getDistance(target, m_pNpc->getPos());     //npc目标到自己距离
  float d3 = getDistance(target, pos);          //玩家到npc目标点距离

  return d2 + d3 > d1;
}

void NpcAI::checkAutoBack()
{
  DWORD returnDis = getReturnDis();
  if (!returnDis) return;
  if (m_pNpc->isWeaponPet()) return;
  if (m_pNpc->getNpcType() == ENPCTYPE_PETNPC) return;
  if (m_pNpc->getNpcType() == ENPCTYPE_BEING) return;
  if (m_pNpc->getNpcType() == ENPCTYPE_ELEMENTELF) return;

  if (!checkDistance(m_pNpc->getPos(), m_pNpc->getBirthPos(), returnDis))
  {
    // 返回时直接找寻攻击目标
    if (fsm->getCurState() == AttackNpcState::instance())
    {
      xSceneEntryDynamic *pEntry = NULL;
      for (auto iter = m_attacklist.begin(); iter != m_attacklist.end(); ++iter)
      {
        xSceneEntryDynamic *pTarget = xSceneEntryDynamic::getEntryByID(*iter);
        if (pTarget && checkDistance(m_pNpc->getBirthPos(), pTarget->getPos(), getSearchRange()) && m_pNpc->isMyEnemy(pTarget))
        {
          pEntry = pTarget;
          break;
        }
      }

      if (pEntry)
      {
        setCurLockID(pEntry->id);
        return;
      }
    }

    xPos p;
    if (m_pNpc->getScene()->getRandTargetPos(m_pNpc->getBirthPos(), getTerritory(), p))
      moveTo(p);

    m_attacklist.clear();
    changeState(ENPCSTATE_NORMAL);
  }
}

void NpcAI::moveTo(xPos p)
{
  if (!isMoveable()) return;

  if (m_pNpc->getScene() == nullptr)
    return;
  if (m_pNpc->getScene()->getValidPos(p) == false)
    return;

  m_pNpc->m_oMove.setFinalPoint(p);

  if (m_pNpc->m_oFollower.hasServant())
  {
    m_pNpc->m_oFollower.moveToMaster(p);
  }
}

void NpcAI::moveToPos(float x, float y, float z)
{
  xPos pos(x, y, z);
  moveTo(pos);
}

BYTE NpcAI::getReturnDis()
{
  if (!isOutRangeBack()) return 100;

  xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(getCurLockID());
  if (pEntry)
  {
    if (SCENE_ENTRY_USER==pEntry->getEntryType())
    {
      return getSearchRange() + getTerritory() + 5;
    }
  }

  return getSearchRange() + getTerritory();
}

DWORD NpcAI::getPursueDis()
{
  if (!m_pNpc || !m_pNpc->getCFG())
    return 0;
  const SNpcLeaveBattle& stCFG = FeatureConfig::getMe().getNpcAICFG().stLeaveBattle;
  return m_pNpc->getNpcType() == ENPCTYPE_MVP ? stCFG.dwMvpRange : stCFG.dwNormalRange;
}

bool NpcAI::switchPickTarget()
{
  if (isPickup() == false)
    return false;

  Scene *scene = m_pNpc->getScene();
  if (scene == nullptr)
    return false;

  //return false;  // 暂时屏蔽

  xSceneEntrySet set;
  scene->getEntryListInBlock(SCENE_ENTRY_ITEM, getSearchPos(), getSearchRange(), set);
  if (set.empty() == true)
    return false;

  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  for (auto s = set.begin(); s != set.end(); ++s)
  {
    SceneItem *pItem = dynamic_cast<SceneItem*>(*s);
    if (pItem == nullptr)
      continue;

    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(pItem->getItemInfo().id());
    if (pCFG == nullptr || pCFG->eItemType == EITEMTYPE_GOLD || pCFG->eItemType == EITEMTYPE_SILVER || pCFG->eItemType == EITEMTYPE_DIAMOND)
      continue;

    float dist = ::getDistance(m_pNpc->getPos(), pItem->getPos());
    if (dist > rCFG.dwPickupRange)
      continue;

    setCurItemID(pItem->tempid);
    return true;
  }

  return false;
}

QWORD NpcAI::getCurLockID()
{
  if (m_qwTauntUser && m_qwTauntUser == m_qwCurLockID)
  {
    xSceneEntryDynamic *pTauntEntry = xSceneEntryDynamic::getEntryByID(m_qwTauntUser);
    if (pTauntEntry == nullptr || pTauntEntry->isAlive() == false)
      m_pNpc->m_oBuff.removeTauntBuffByFromID(m_qwTauntUser);
  }
  return m_qwCurLockID;
}

void NpcAI::setCurLockID(QWORD id)
{
  if (m_qwCurLockID!=id)
  {
    // 被嘲讽时只能攻击当前玩家
    if (m_qwTauntUser != 0 && m_qwTauntUser != id)
    {
      return;
    }
    // add or del user beLock
    if (m_qwCurLockID != 0)
    {
      SceneUser* oldUser = SceneUserManager::getMe().getUserByID(m_qwCurLockID);
      if (oldUser != nullptr)
      {
        oldUser->delLockMe(m_pNpc->id);

        if (m_pNpc->getNpcType() == ENPCTYPE_MVP)
          m_pNpc->addMvpLockUserTime(m_qwCurLockID, xTime::getCurMSec() - m_qwLockTimeTick);
      }
    }
    if (id != 0)
    {
      SceneUser* newUser = SceneUserManager::getMe().getUserByID(id);
      if (newUser)
      {
        if (m_pNpc->define.m_oVar.m_qwTeamID)
        {
          if (newUser->getTeamID() != m_pNpc->define.m_oVar.m_qwTeamID)
            return;
        }
        newUser->addLockMe(m_pNpc->id);
        m_qwLockTimeTick = xTime::getCurMSec();
      }
      else
      {
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(id);
        // 生命体/宠物可以放嘲讽
        if (npc && (npc->isWeaponPet() || npc->getNpcType() == ENPCTYPE_SKILLNPC || (npc->getNpcType() == ENPCTYPE_PETNPC && m_qwTauntUser == 0 && getMasterTauntUser() == 0) || (npc->getNpcType() == ENPCTYPE_BEING && m_qwTauntUser == 0 && getMasterTauntUser() == 0) || npc->getNpcType() == ENPCTYPE_ELEMENTELF))
        {
          m_attacklist.erase(id);
          return;
        }
      }
    }

    // 失去目标
    if (m_qwCurLockID && !id)
    {
      m_pNpc->m_oEmoji.check("LoseTarget");
    }
    // 切换目标
    if (m_qwCurLockID && id)
    {
      m_pNpc->m_oEmoji.check("SwitchTarget");
    }

    m_qwLastLockID = m_qwCurLockID;
    m_qwCurLockID = id;
    if (m_pNpc->m_oFollower.hasServant())
    {
      m_pNpc->m_oFollower.setLockByMaster(id);
    }
   // XLOG("[LOCK],%llu,%s,set:%llu", m_pNpc->id, m_pNpc->name, id);
  }
}

void NpcAI::setCurItemID(QWORD id)
{
  if (m_qwCurItemID != id)
  {
    m_qwCurItemID = id;
    XLOG << "[ItemLock]" << m_pNpc->id << m_pNpc->name << "set:" << id << XEND;
  }
}

void NpcAI::resetCurSkill()
{
  m_dwCurSkillID = 0;
  m_dwCurIndex = 0;
  m_eStateChange = m_eSkillState;
  m_eSkillState = ENPCSTATE_MIN;
}

bool NpcAI::stateChange()
{
  if (m_eStateChange == ENPCSTATE_MIN)
    return true;
  if (m_eStateChange == getStateType())
  {
    m_eStateChange = ENPCSTATE_MIN;
    return true;
  }

  changeState(m_eStateChange);
  m_eStateChange = ENPCSTATE_MIN;
  return false;
}

bool NpcAI::processUseSkill()
{
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(m_dwCurSkillID);
  if (pSkillCFG == nullptr)
  {
    setCurLockID(0);
    return false;
  }

  if (m_pNpc->getScene() == nullptr)
  {
    setCurLockID(0);
    return false;
  }

  // change target
  if (isSwitchAttack() == true)
  {
    if (_three_sec.timeUp(now()))
    {
      if (selectByPercent(20))
        switchAttackTarget(true);
      else
        _three_sec.reset();
    }
  }

  if (checkPursueNeedBack())
  {
    changeState(ENPCSTATE_GOBACK);
    setCurLockID(0);
    return true;
  }
  xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(getCurLockID());
  if (attackNeedGoBack(pEntry))
  {
    setCurLockID(0);
    checkAutoBack();
    return false;
  }

  float dist = ::getDistance(m_pNpc->getPos(), pEntry->getPos());
  float skillDist = pSkillCFG->getLaunchRange(m_pNpc);

  if (dist > skillDist + 0.05f && skillDist != 0)
  {
    //if ((m_dwLastAttackTime && (m_dwLastAttackTime + MAX_NPC_PURSUIT_TIME < curTime)) || getTerritory() == 0)
    //{
      //setCurLockID(0);
      //return false;
    //}

    if (isConfigCanMove() == false)
    {
      if (!m_attacklist.empty())
      {
        setCurLockID(0);
        resetCurSkill();
        return false;
      }
      resetCurSkill();
      return true;
    }
    // 战斗猫, 锁定的目标离开主人视野时, 取消锁定
    if (m_pNpc->isWeaponPet() || m_pNpc->getNpcType() == ENPCTYPE_PETNPC || m_pNpc->getNpcType() == ENPCTYPE_BEING || m_pNpc->getNpcType() == ENPCTYPE_ELEMENTELF)
    {
      SceneUser* user = m_pNpc->getMasterUser();
      if (!user)
        return false;
      if (getDistance(user->getPos(), pEntry->getPos()) > 20)
      {
        setCurLockID(0);
        return false;
      }
    }

    if (m_pNpc->m_oMove.empty() == true || getDistance(m_pNpc->m_oMove.getDestPos(), pEntry->getPos()) > 3)// || (m_flTargetDis && (m_flTargetDis<dist) && checkRePathFinding(pEntry->getPos())))
    {
      xPos p = getPosByDir(pEntry->getPos(), m_pNpc->getPos(), 0.8f);
      moveTo(p);
    }

    m_flTargetDis = dist;

    //moveSingleStep = true;
    autoMove();
    return true;
  }
  else
  {
    if (m_pNpc->m_oMove.empty() == false || m_bMarkPosNeedSync)
    {
      m_pNpc->m_oMove.clearPath();
      m_bMarkPosNeedSync = false;
    }
  }

  if (!hasImmuneID(pEntry->id) && checkReaction(pEntry, EREACTTYPE_FRIEND))
  {
    NpcChangeAngle cmd;
    cmd.set_guid(m_pNpc->id);
    cmd.set_targetid(pEntry->id);
    DWORD angle = calcAngle(m_pNpc->getPos(), pEntry->getPos());
    cmd.set_angle(angle);
    PROTOBUF(cmd, send, len);
    m_pNpc->sendCmdToNine(send, len);

    m_pNpc->checkEmoji("ActivePassive");
    addImmuneID(pEntry->id);
    setCurLockID(0);
    return true;
  }


  xPos skillpos = pEntry->getPos();
  if (pEntry == m_pNpc && pSkillCFG->getSkillCamp() == ESKILLCAMP_ENEMY)
  {
    m_pNpc->getScene()->getRandPos(m_pNpc->getPos(), 5, skillpos);
  }
  if (m_pNpc->useSkill(m_dwCurSkillID, pEntry->id, skillpos) == true)
  {
    m_dwLastAttackTime = now();
    if (m_dwCurSkillID != m_pNpc->getNormalSkill())
      addCD();
    m_pNpc->m_oEmoji.check("Attack");
  }
  resetCurSkill();
  return true;
}

void NpcAI::addCD()
{
  if (m_dwCurSkillID == 0 || m_pNpc->getCFG() == nullptr)
    return;
  const SNpcSkillGroup* pGroup = m_pNpc->getCFG()->getSkillGroup(m_dwCurIndex);
  if (pGroup == nullptr)
    return;
  const SNpcSkill* pSkill = pGroup->getSkill(m_dwCurSkillID);
  if (pSkill == nullptr)
    return;

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(m_dwCurSkillID);
  if (pSkillCFG == nullptr)
    return;

  m_pNpc->m_oCDTime.add(m_dwCurIndex, pSkill->dwShareCD * ONE_THOUSAND, CD_TYPE_SKILLDEALY);
  m_pNpc->m_oCDTime.add(m_dwCurSkillID, pSkillCFG->getCD(m_pNpc), CD_TYPE_SKILL);
}

bool NpcAI::checkSkillCondition(const SNpcSkill& rItem)
{
  if (rItem.eState != getStateType())
    return false;

  auto check = [this](const SNpcSkillCond& rItem)
  {
    switch (rItem.eCondition)
    {
      case ENPCSKILLCOND_MIN:
        return false;
      case ENPCSKILLCOND_SELFHPLESS:
        if (rItem.vecParams.empty() == true)
          return false;
        return m_pNpc->getAttr(EATTRTYPE_HP) / m_pNpc->getAttr(EATTRTYPE_MAXHP) * 100.0f < rItem.vecParams[0];
      case ENPCSKILLCOND_SERVANTHPLESS:
        {
          if (m_pNpc->m_oFollower.hasServant() == false || rItem.vecParams.empty() == true)
            return false;
          TVecQWORD vecIDs;
          m_pNpc->m_oFollower.getServantIDs(vecIDs);
          for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
          {
            SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(*v);
            if (pNpc != nullptr && (pNpc->getAttr(EATTRTYPE_HP) / pNpc->getAttr(EATTRTYPE_MAXHP) * 100.0f) < rItem.vecParams[0])
              return true;
          }
        }
        break;
      case ENPCSKILLCOND_SERVANTNUMLESS:
        {
          if (m_pNpc->m_oFollower.hasServant() == false || rItem.vecParams.empty() == true)
            return false;
          TVecQWORD vecIDs;
          m_pNpc->m_oFollower.getServantIDs(vecIDs);
          return static_cast<DWORD>(vecIDs.size()) < rItem.vecParams[0];
        }
        break;
      case ENPCSKILLCOND_ATTACKTIME:
        {
          if (rItem.vecParams.size() != 2)
            return false;
          DWORD dwNow = xTime::getCurSec();
          DWORD dwDeltaTime = dwNow - m_dwNormalStartTime;
          if (dwDeltaTime > rItem.vecParams[1])
          {
            m_dwNormalStartTime = 0;
            m_dwAttackStartTime = dwNow;
          }

          return dwNow > m_dwAttackStartTime + rItem.vecParams[0];
        }
        break;
      case ENPCSKILLCOND_SELFRANGE:
        {
          if (rItem.vecParams.empty() == true)
            return false;
          Scene* pScene = m_pNpc->getScene();
          if (pScene == nullptr)
            return false;
          xSceneEntrySet set;
          pScene->getEntryListInBlock(SCENE_ENTRY_USER, m_pNpc->getPos(), rItem.vecParams[0], set);
          for (auto s = set.begin(); s != set.end(); ++s)
          {
            SceneUser* pUser = dynamic_cast<SceneUser*>(*s);
            if (pUser != nullptr)
              return true;
          }
        }
        break;
      case ENPCSKILLCOND_MAX:
        return false;
    }

    return false;
  };

  for (auto v = rItem.vecCond.begin(); v != rItem.vecCond.end(); ++v)
  {
    if (check(*v) == false)
      return false;
  }

  return true;
}

DWORD NpcAI::getAttackSkillID()
{
  Scene* pScene = m_pNpc->getScene();
  if (pScene == nullptr || m_pNpc->getCFG() == nullptr)
  {
	XLOG << "[getAttackSkillID]" << "1" << XEND;
	return 0;
  }
    

  if (m_pNpc->isAttrCanSkill() == false)
  {
	XLOG << "[getAttackSkillID]" << "2" << XEND;
	return 0;
  };

  if (m_pNpc->getAttr(EATTRTYPE_TAUNT) == 1)
  {
	XLOG << "[getAttackSkillID]" << "3" << XEND;
	return m_pNpc->getNormalSkill();
  }
    

  if (m_pNpc->isAttrOnlyNormalSkill() == false)
  {
	//循环出这里
	//XLOG << "[getAttackSkillID]" << "4" << XEND;
    const TMapNpcSkillGroup& mapGroup = m_pNpc->getCFG()->mapSkillGroup;
    for (auto m = mapGroup.begin(); m != mapGroup.end(); ++m)
    {
      if (m_pNpc->m_oCDTime.skilldone(m->first) == false)
        continue;

      const SNpcSkill& oneSkill = m->second.randOneSkill();
      if (oneSkill.dwSkillID == 0)
        continue;

      if (checkSkillCondition(oneSkill) == false)
        continue;

      if (m_pNpc->m_oCDTime.skilldone(oneSkill.dwSkillID) == false)
        continue;

      m_dwCurIndex = m->first;

      //XLOG("[NpcAI::getAttackSkillID] npc = %u name = %s use specialskill = %u", m_pNpc->base->dwID, m_pNpc->name, oneSkill.dwSkillID);
      m_eSkillState = oneSkill.eState;

      // 不同宠物技能等级不同
      if (m_pNpc->getNpcType() == ENPCTYPE_PETNPC)
      {
        PetNpc* pPet = dynamic_cast<PetNpc*> (m_pNpc);
        if (pPet == nullptr)
          continue;
        return pPet->getModifiedSkill(oneSkill.dwSkillID);
      }
      return oneSkill.dwSkillID;
    }
  }

  if (getStateType() == ENPCSTATE_ATTACK && m_qwCurLockID != m_pNpc->id)
  {
	XLOG << "[getAttackSkillID]" << "5" << XEND;
	return m_pNpc->getNormalSkill();
  }
  return 0;
}

bool NpcAI::fearRun()
{
  // fearrun
  if (m_pNpc->getAttr(EATTRTYPE_FEARRUN) != 0 && isMoveable())
  {
    if (fearRunPos.empty())
      fearRunPos = m_pNpc->getPos();
    DWORD cur = now();
    float maxRange = 5.0f; // 跑动范围
    if (cur > m_dwNextSetPosTime || m_pNpc->m_oMove.empty())
    {
      m_pNpc->m_oMove.clearPath();

      xPos p = m_pNpc->getPos();
      xPos tarPos;
      DWORD count = 30;
      while(count--)
      {
        float direct = randBetween(1, 360);
        direct = direct * 3.14 / 180;

        tarPos.x = fearRunPos.x + maxRange * sin(direct);
        tarPos.y = p.y;
        tarPos.z = fearRunPos.z + maxRange * cos(direct);

        if (m_pNpc->getScene() && m_pNpc->getScene()->getValidPos(tarPos) == true)
        {
          m_pNpc->m_oMove.setFinalPoint(tarPos);
          break;
        }
      }

      m_dwNextSetPosTime = cur + 2;
    }
    autoMove();
    return true;
  }
  fearRunPos.clear();
  return false;
}

DWORD NpcAI::getTerritory()
{
  return m_pNpc->define.getTerritory();
}

void NpcAI::beAttack(xSceneEntryDynamic* attacker)
{
  XLOG << "[beAttack]" << "ok" << XEND;
  // 锁定其他目标, 非攻击状态(机警, 抢镜等)时, 取消锁定当前目标
  if (m_qwCurLockID && m_dwStateType != ENPCSTATE_ATTACK && m_dwStateType != ENPCSTATE_NORMAL)
    {
		setCurLockID(0);
		XLOG << "[beAttack]" << "1" << XEND;
	}

  if (!getCurLockID() || (isSwitchBeAttack() && selectByPercent(20)))
  {
	XLOG << "[beAttack]" << "2" << XEND;
    if (!getCurLockID())
    {
	  XLOG << "[beAttack]" << "3" << XEND;
      m_pNpc->m_oEmoji.check("EncountPassive");
    }
    switchAttackTarget(true);
  }

  XLOG << "[beAttack]" << "getCurLockID:" << getCurLockID() << XEND;
  if (getCurLockID())
  {
	XLOG << "[beAttack]" << "4" << XEND;
    // 若锁定攻击者, 立即切到攻击状态
    if (attacker && (getCurLockID() == attacker->id))
	{
		XLOG << "[beAttack]" << "5" << XEND;
		changeState(ENPCSTATE_ATTACK);
	}
      

    if (m_pNpc->m_oFollower.hasServant())
    {
	  XLOG << "[beAttack]" << "6" << XEND;
      m_pNpc->m_oFollower.setLockByMaster(getCurLockID());
    }
  }

  // 记录最近被攻击的时间
  m_dwLastBeAttackTime = now();

  // check team attack
  if (attacker == nullptr || attacker->getScene() == nullptr)
    return;
  if (isTeamAttack() && getCurLockID() != 0)
  {
    DWORD cur = now();
    const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
    DWORD emojiID = rCFG.dwTeamAttack.dwEmojiID;
    DWORD waittime = rCFG.dwTeamAttack.dwResponseTime;
    DWORD range = rCFG.dwTeamAttack.dwRange;
    xSceneEntrySet set;
    attacker->getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, getSearchPos(), range, set);

    for (auto s = set.begin(); s != set.end(); ++s)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*>(*s);
      if (pNpc != nullptr && pNpc->m_ai.getCurLockID() == 0 && pNpc->define.getID() == m_pNpc->define.getID())
      {
        pNpc->m_oMove.stop();
        pNpc->m_ai.setCurLockID(getCurLockID());

        m_pNpc->m_oEmoji.play(emojiID);

        pNpc->m_ai.setStateType(ENPCSTATE_ATTACK);
        pNpc->m_ai.setWaitExitTime(cur + waittime);
        pNpc->m_ai.changeState(ENPCSTATE_WAIT);
      }
    }
  }
}

// call one time when create NPC
void NpcAI::initAddBuff()
{
  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  if (isGhost())
  {
    for (auto &v : rCFG.stGhost.vecBuff)
      m_pNpc->m_oBuff.add(v);
    // add ghost buff
  }
  if (isDemon())
  {
    m_pNpc->m_oBuff.add(rCFG.dwDemon.dwBirthBuff);
    // add demon buff
  }
  if (isFly())
  {
    m_pNpc->m_oBuff.add(rCFG.dwFly.dwBirthBuff);
    // add fly buff
  }

  // add self buff
  const SNpcCFG* pCFG = m_pNpc->getCFG();
  if (pCFG != nullptr && pCFG->vecBuffs.empty() == false)
  {
    for (auto &v : pCFG->vecBuffs)
    {
      m_pNpc->m_oBuff.add(v, m_pNpc);
    }
  }
}

bool NpcAI::isMoveable()
{
  if (m_pNpc == nullptr)
    return false;
  if (m_pNpc->define.m_oVar.m_qwFollowerID != 0)
    return true;
  if (isConfigCanMove() == false)
    return false;
  if (m_pNpc->getMoveSpeed() <= 0)
    return false;
  if (m_pNpc->getAttr(EATTRTYPE_NOMOVE))
    return false;
  if (m_pNpc->getAttr(EATTRTYPE_NOACT))
    return false;
  return true;
}

bool NpcAI::isConfigCanMove()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_MOVE_ABLE) &&  m_pNpc->getBaseAttr(EATTRTYPE_MOVESPD) != 0);
}

bool NpcAI::isAttackBack()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_ATTACK_BACK) && !m_pNpc->getAttr(EATTRTYPE_NOATTACK));
}

bool NpcAI::isPickup()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_PICKUP));
}

bool NpcAI::isTeamAttack()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_TEAM_ATTACK));
}

bool NpcAI::isSwitchAttack()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_SWITCH_ATTACK) && !m_pNpc->m_oBuff.haveBuffType(EBUFFTYPE_CHASE));
}

bool NpcAI::isSwitchBeAttack()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_SWITCH_BE_ATTACK) && !m_pNpc->m_oBuff.haveBuffType(EBUFFTYPE_CHASE));
}

bool NpcAI::isBeAttack1Max()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_BE_ATTACK_1_MAX));
}

bool NpcAI::isBeChantAttack()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_BE_CHANT_ATTACK));
}

bool NpcAI::isOutRangeBack()
{
  if (m_pNpc->isWeaponPet() || m_pNpc->getNpcType() == ENPCTYPE_PETNPC || m_pNpc->getNpcType() == ENPCTYPE_BEING || m_pNpc->getNpcType() == ENPCTYPE_ELEMENTELF)
    return false;
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_OUT_RANGE_BACK));
}

bool NpcAI::isGocamera()
{
  if (!isSmileCamera())
    return false;
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_STEAL_CAMERA));
}

bool NpcAI::isSmileCamera()
{
  return m_pNpc->getSmileEmoji() != 0 || m_pNpc->getSmileAction() != 0;
}

bool NpcAI::isAlertFakeDead()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_ALERT));
}

bool NpcAI::isExpelDead()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_EXPEL));
}

bool NpcAI::isFly()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_FLY));
}

bool NpcAI::isDemon()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_DEMON));
}

bool NpcAI::isGhost()
{
  return ((0!=(m_pNpc->getBehaviours() & BEHAVIOUR_GHOST)) || (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_GHOST_2)));
}

bool NpcAI::isAttackGhost()
{
  return ((0!=(m_pNpc->getBehaviours() & BEHAVIOUR_GHOST_2)));
}

bool NpcAI::isNaughty()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_NAUGHTY));
}

bool NpcAI::isNotSkillSelect()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_NOT_SKILL_SELECT));
}

bool NpcAI::isReckless()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_RECKLESS));
}

bool NpcAI::isAngerHand()
{
  return (0!=(m_pNpc->getBehaviours() & BEHAVIOUR_JEALOUS));
}

bool NpcAI::isSleep()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_SLEEP));
}

bool NpcAI::isStatusAttack()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_STATUSATTACK));
}

bool NpcAI::isDHide()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_DHIDE));
}

bool NpcAI::isNightWork()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_NIGHTWORK));
}

bool NpcAI::isEndure()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_ENDURE));
}

bool NpcAI::isNoTransformAtt()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_NOTRANSFORM_ATT));
}

bool NpcAI::isImmuneTaunt()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_IMMUNETAUNT));
}

bool NpcAI::isFarSearch()
{
  return (0 != (m_pNpc->getBehaviours() & BEHAVIOUR_FARSEARCH));
}

bool NpcAI::checkReaction(xSceneEntryDynamic* pEntry, EReactType eType)
{
  if (m_pNpc == nullptr)
    return false;

  const SNpcCFG* pCFG = m_pNpc->getCFG();
  if (pCFG == nullptr)
    return false;
  const SReactData* pData = pCFG->stNpcReaction.getReaction(eType);
  if (pData == nullptr)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (pEntry);
  if (user == nullptr)
    return false;

  // item attract attack
  MainPackage* pMainPack = dynamic_cast<MainPackage*> (user->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack != nullptr)
  {
    for (auto &v : pData->vecItems)
    {
      if (pMainPack->getItemCount(v, ECHECKMETHOD_NORMAL) != 0)
      {
        return true;
      }
    }
  }
  // equip attract attack
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(user->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack != nullptr)
  {
    for (auto &v : pData->vecEquips)
    {
      if (pEquipPack->getItemCount(v, ECHECKMETHOD_NORMAL) != 0)
      {
        return true;
      }
    }
  }
  // state attract attack
  if (pData->dwStatus)
  {
    if ((DWORD)(user->getAttr(EATTRTYPE_STATEEFFECT)) & (pData->dwStatus))
    {
      return true;
    }
  }
  return false;
}

bool NpcAI::canNormalLock(xSceneEntryDynamic* pEntry)
{
  if (pEntry == nullptr)
  {
	XLOG << "[canNormalLock]" << "1_false" << XEND;
    return false;
  }
	
  if (m_pNpc->isMyEnemy(pEntry) == false)
  {
	XLOG << "[canNormalLock]" << "2_false" << XEND;
    return false;
  }
  if (pEntry->isAlive() == false)
  {
	XLOG << "[canNormalLock]" << "3_false" << XEND;
    return false;
  }
  if (pEntry->getAttr(EATTRTYPE_HIDE) != 0 && !isDHide())
  {
	XLOG << "[canNormalLock]" << "4_false" << XEND;
    return false;
  }
  if (pEntry->isNoAttacked())
  {
	XLOG << "[canNormalLock]" << "5_false" << XEND;
    return false;
  }
  if (pEntry->isNoEnemySkilled())
  {
	XLOG << "[canNormalLock]" << "6_false" << XEND;
    return false;
  }
  if (pEntry->getStatus() == ECREATURESTATUS_FAKEDEAD && !isReckless())
  {
	XLOG << "[canNormalLock]" << "7_false" << XEND;
    return false;
  }
  if (m_pNpc->checkNineScreenShow(pEntry) == false && (m_pNpc->getNpcType() != ENPCTYPE_PETNPC && m_pNpc->getNpcType() != ENPCTYPE_WEAPONPET))
  {
	XLOG << "[canNormalLock]" << "8_false" << XEND;
    return false;
  }
  if (pEntry->isNoActiveMonster() && m_pNpc->getNpcType() != ENPCTYPE_MVP)
  {
	XLOG << "[canNormalLock]" << "9_false" << XEND;
    return false;
  }

  SceneUser* user = dynamic_cast<SceneUser*> (pEntry);
  //if (user && !user->m_oHands.canBeAttack())
    //return false;
  if (user && user->getTransform().isMonster() && m_pNpc->getCFG() != nullptr)
  {
    if (m_pNpc->getCFG()->isTransRecIDs(user->getTransform().getMonsterID()) == true)
    {
		XLOG << "[canNormalLock]" << "10_false" << XEND;
		return false;
	}
  }
  if (m_pNpc->getCFG()->dwChainAtkLmt && user && user->getAttackMeSize() >= m_pNpc->getCFG()->dwChainAtkLmt)
  {
	XLOG << "[canNormalLock]" << "11_false" << XEND;
    return false;
  }

  if (SCENE_ENTRY_NPC == pEntry->getEntryType())
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (pEntry);
    // 怪物无法锁定战斗猫
    if (npc && npc->isWeaponPet())
    {
		XLOG << "[canNormalLock]" << "12_false" << XEND;
		return false;
	}
    // 怪物无法锁定技能npc
    if (npc && npc->getNpcType() == ENPCTYPE_SKILLNPC)
    {
		XLOG << "[canNormalLock]" << "13_false" << XEND;
		return false;
	}
    if (npc && npc->getNpcType() == ENPCTYPE_PETNPC)
    {
		XLOG << "[canNormalLock]" << "14_false" << XEND;
		return false;
	}
    if (npc && npc->getNpcType() == ENPCTYPE_BEING)
    {
		XLOG << "[canNormalLock]" << "15_false" << XEND;
		return false;
	}
    if (npc && npc->getNpcType() == ENPCTYPE_ELEMENTELF)
    {
		XLOG << "[canNormalLock]" << "16_false" << XEND;
		return false;
	}
  }
  return true;
}

bool NpcAI::attackNeedGoBack(xSceneEntryDynamic* pLockTar)
{
  if (pLockTar == nullptr)
    return true;
  if (pLockTar->isNoAttacked())
    return true;
  if (pLockTar->isNoEnemySkilled())
    return false;
  if (pLockTar->getAttr(EATTRTYPE_HIDE) != 0 && !isDHide())
    return true;
  if (!pLockTar->isAlive() || pLockTar->isMask())
    return true;

  if (m_pNpc->define.m_oVar.m_qwNpcOwnerID)
  {
    SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(m_pNpc->define.m_oVar.m_qwNpcOwnerID);
    if (pMaster && pMaster->m_ai.getCurLockID() == pLockTar->id)
      return false;
  }
  if (m_pNpc->isWeaponPet())
    return false;
  if (m_pNpc->getNpcType() == ENPCTYPE_PETNPC)
    return false;
  if (m_pNpc->getNpcType() == ENPCTYPE_BEING)
    return false;
  if (m_pNpc->getNpcType() == ENPCTYPE_ELEMENTELF)
    return false;

  if (isFarSearch() == false)
  {
    if (m_pNpc->getScene() != pLockTar->getScene() || !checkDistance(pLockTar->getPos(), m_pNpc->getPos(), 15))
      return true;

    /*SceneUser* user = dynamic_cast<SceneUser*> (pLockTar);
    if (user && !user->m_oHands.canBeAttack())
      return false;*/

    // 超出一定距离, 并且一段时间内没有受到攻击
    const SNpcLeaveBattle& stCFG = FeatureConfig::getMe().getNpcAICFG().stLeaveBattle;
    DWORD leavetime = m_pNpc->getNpcType() == ENPCTYPE_MVP ? stCFG.dwMvpTime : stCFG.dwNormalTime;
    DWORD range = m_pNpc->getNpcType() == ENPCTYPE_MVP ? stCFG.dwMvpRange : stCFG.dwNormalRange;
    if (getDistance(pLockTar->getPos(), m_pNpc->getPos()) > range && (now() >= m_dwLastBeAttackTime + leavetime || !m_attacklist.empty()))
      return true;
  }

  return false;
}

bool NpcAI::isNight(time_t time)
{
  return MiscConfig::getMe().getSystemCFG().isNight(time);
}

void NpcAI::onSeeStatus(SceneUser* pUser, DWORD status)
{
  if (!isStatusAttack())
    return;
  if (getStateType() != ENPCSTATE_NORMAL && getStateType() != ENPCSTATE_ATTACK)
    return;

  DWORD skill = FeatureConfig::getMe().getNpcAICFG().stStatusAttack.getSkillByStatus(status);
  if (skill == 0)
    return;
  DWORD realskill = 0;
  if (m_pNpc->getCFG() != nullptr)
  {
    const TMapNpcSkillGroup& mapGroup = m_pNpc->getCFG()->mapSkillGroup;
    for (auto &m : mapGroup)
    {
      auto v = find_if(m.second.vecSkill.begin(), m.second.vecSkill.end(), [&skill](const SNpcSkill& r)
      {
        return r.dwSkillID / ONE_THOUSAND == skill;
      });
      if (v != m.second.vecSkill.end())
      {
        realskill = v->dwSkillID;
        break;
      }
    }
  }

  if (realskill == 0)
    return;
  if (m_pNpc->m_oCDTime.skilldone(realskill) == false)
    return;

  m_dwCurSkillID = realskill;
  setCurLockID(pUser->id);
  changeState(ENPCSTATE_ATTACK);
}

bool NpcAI::checkPursueNeedBack()
{
  if (m_pNpc->define.getPursue() == 0)
    return false;
  if (getDistance(m_pNpc->getPos(), m_pNpc->getBirthPos()) < (float)m_pNpc->define.getPursue())
    return false;
  if (m_pNpc->define.getPursueTime())
    return now() > m_pNpc->define.getPursueTime() + m_dwLastBeAttackTime;

  return true;
}

bool NpcAI::goback()
{
  if (m_pNpc->m_oMove.empty())// && getDistance(m_pNpc->getBirthPos(), m_pNpc->getPos()) < m_pNpc->define.getPursue())
  {
    changeState(ENPCSTATE_NORMAL);
    m_attacklist.clear();
    setCurLockID(0);
    /*const TVecDWORD& vecBuff = FeatureConfig::getMe().getNpcAICFG().vecGoBackBuff;
    for (auto &v : vecBuff)
    {
      m_pNpc->m_oBuff.del(v);
    }
    */
    NpcChangeAngle cmd;
    cmd.set_guid(m_pNpc->id);
    cmd.set_targetid(0);
    cmd.set_angle(m_pNpc->define.getDir());
    PROTOBUF(cmd, send, len);
    m_pNpc->sendCmdToNine(send, len);
    return true;
  }
  autoMove();
  return true;
}

void NpcAI::verifyAttackList(xSceneEntrySet& set)
{
  if (set.empty())
    return;

  if (set.size() > 1 && m_bInChangeTarget)
  {
    m_bInChangeTarget = false;
    for (auto s = set.begin(); s != set.end(); ++s)
    {
      if ((*s)->id == m_qwLastLockID)
      {
        set.erase(s);
        break;
      }
    }
  }

  if (m_qwPriAttackUserID)
  {
    auto it = find_if(set.begin(), set.end(), [&](xSceneEntry*  s) ->bool{
      return m_qwPriAttackUserID == s->id;
    });
    if (it != set.end())
    {
      xSceneEntry* tmp = *it;
      set.clear();
      set.insert(tmp);
      return;
    }
    xSceneEntrySet tmpset;
    for (auto &s : set)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (!user)
        continue;
      if (user->isMyTeamMember(m_qwPriAttackUserID))
        tmpset.insert(s);
    }
    if (!tmpset.empty())
    {
      set.clear();
      set.insert(tmpset.begin(), tmpset.end());
    }
    return;
  }
  if (!m_setPriorProfession.empty())
  {
    xSceneEntrySet tmpset;
    for (auto &s : set)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (!user)
        continue;
      DWORD pro = user->getProfession();
      if (m_setPriorProfession.find(pro) != m_setPriorProfession.end())
        tmpset.insert(s);
    }
    if (!tmpset.empty())
    {
      set.clear();
      set.insert(tmpset.begin(), tmpset.end());
      return;
    }
  }

  if (m_ePriorGender != EGENDER_MIN)
  {
    xSceneEntrySet tmpset;
    for (auto &s : set)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (!user)
        continue;
      if (user->getUserSceneData().getGender() == m_ePriorGender)
        tmpset.insert(s);
    }
    if (!tmpset.empty())
    {
      set.clear();
      for (auto &s : tmpset)
      {
        set.insert(s);
      }
      return;
    }
  }
}

bool NpcAI::runaway()
{
  if (m_pNpc->m_oMove.empty() || now() > m_dwStopRunAwayTime)
    return false;

  autoMove();
  return true;
}

void NpcAI::onNpcDie()
{
  if (m_qwCurLockID != 0 && m_pNpc->getNpcType() == ENPCTYPE_MVP)
    m_pNpc->addMvpLockUserTime(m_qwCurLockID, xTime::getCurMSec() - m_qwLockTimeTick);
}

void NpcAI::putAttacker(xSceneEntryDynamic* pEntry)
{
  if (pEntry == nullptr)
  {
	  XLOG << "[putAttacker]" << "1" << XEND;
	  return;
  }
    
  if (m_pNpc->isWeaponPet())
  {
	  XLOG << "[putAttacker]" << "2" << XEND;
	  return;
  }
  if (m_pNpc->getNpcType() == ENPCTYPE_PETNPC)
  {
	  XLOG << "[putAttacker]" << "3" << XEND;
	  return;
  }
  if (m_pNpc->getNpcType() == ENPCTYPE_BEING)
  {
	  XLOG << "[putAttacker]" << "4" << XEND;
	  return;
  }
  if (m_pNpc->getNpcType() == ENPCTYPE_ELEMENTELF)
  {
	  XLOG << "[putAttacker]" << "5" << XEND;
	  return;
  }
  m_attacklist.insert(pEntry->id);
  /*
  SceneUser* user = dynamic_cast<SceneUser*> (pEntry);
  if (user && user->getTeamID())
  {
    std::set<SceneUser*> teamusers = user->getTeamSceneUser();
    for (auto &s : teamusers)
    {
      if (s->id == user->id)
        continue;
      m_attacklist.insert(s->id);
    }
  }
  */
}

bool NpcAI::canAI2Attack(xSceneEntryDynamic* target)
{
  if (target == nullptr)
    return false;
  if (getCurLockID() == target->id)
    return true;
  if (m_attacklist.find(target->id) != m_attacklist.end())
    return true;

  if (canNormalLock(target) == false)
    return false;
  if (m_pNpc->getCFG() && m_pNpc->getCFG()->dwProtectAtkLv < target->getLevel())
    return false;

  return true;
}

QWORD NpcAI::getMasterTauntUser()
{
  if (m_pNpc->define.m_oVar.m_qwNpcOwnerID == 0)
    return 0;
  SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(m_pNpc->define.m_oVar.m_qwNpcOwnerID);
  if (pMaster == nullptr)
    return 0;
  return pMaster->m_ai.getTauntUser();
}
