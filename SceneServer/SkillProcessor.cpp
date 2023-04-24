#include "SkillProcessor.h"
#include "SkillManager.h"
#include "SceneNpc.h"
#include "SceneUser.h"
#include "SceneTrapManager.h"
#include "SceneNpcManager.h"
#include "PlatLogManager.h"
#include "CommonConfig.h"

const float SKILL_READY_ERROR = 150; //client <100 时吟唱时间清0
// skillrunner
SkillRunner::SkillRunner()
{

}

SkillRunner::~SkillRunner()
{

}

void SkillRunner::timer(QWORD curTime)
{
  xTime frameTimer;

  run(curTime);

  QWORD e = frameTimer.uElapse();
  if (e >= CommonConfig::m_dwSkillExecPrintTime && m_pEntry)
    XLOG << "[技能超时统计-技能执行], 对象:" << m_pEntry->name << m_pEntry->id << "技能:" << m_oCmd.skillid() << "执行时间:" << e << "微妙" << XEND;
}

void SkillRunner::run(QWORD curTime)
{
  if (m_pSkillCFG == nullptr)
    return;

  if (m_eState != ESKILLSTATE_RUN)
  {
    end();
    return;
  }
  QWORD duration = m_pSkillCFG->getDuration(getEntry());
  if (duration != 0 && curTime > m_qwStartTime + duration)
  {
    end();
    return;
  }
  if (m_dwCount >= m_pSkillCFG->getCount(getEntry()))
  {
    end();
    return;
  }
  if (curTime < m_qwNextTime)
  {
    //end();
    return;
  }

  if (m_pSkillCFG->run(*this) == false)
  {
    end();
    return;
  }

  if (m_dwCount == 1)
  {
    getEntry()->m_oSkillProcessor.addSkillUseInfo(m_pSkillCFG->getSkillID(), curTime);
  }

  m_qwNextTime = curTime + m_pSkillCFG->getInterval();
}

void SkillRunner::reset()
{
  m_qwStartTime = 0;
  m_qwNextTime = 0;
  m_qwChantEndTime = 0;
  m_dwCount = 0;
  m_dwBreakHp = 0;
  m_eState = ESKILLSTATE_MIN;
  m_qwParam1 = 0;
  m_bTrap = false;
  m_bBreak = false;
  m_dwBreaker = 0;
  m_pSkillCFG = nullptr;
  m_bTrapTrigger = false;
  m_setBuffTargets.clear();
  //m_eTransType = ETRANSPORTTYPE_MIN;
  //m_dwTransMapID = 0;
  m_dwArrowID = 0;
  m_qwChantNextTime = 0;
  m_oTargetPos.clear();
  m_bValidBreak = false;
  m_mapBuff2Layers.clear();
  m_bDelStatus = false;

  if (m_pEntry == nullptr)
    return;
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;
  SceneFighter* pf = pUser->getFighter();
  if (pf == nullptr)
    return;
  pf->getSkill().delTempSkill(m_oCmd.skillid());
}

void SkillRunner::end()
{
  if (m_pSkillCFG != nullptr)
    m_pSkillCFG->end(*this);
  //m_eState = ESKILLSTATE_MIN;
  reset();
}

// skillprocessor
SkillProcessor::SkillProcessor(xSceneEntryDynamic* pEntry)
{
  m_oRunner.setEntry(pEntry);
  m_oHelpRunner.setEntry(pEntry);
}

SkillProcessor::~SkillProcessor()
{
  clear();
}

bool SkillProcessor::setActiveSkill(const SkillBroadcastUserCmd& oCmd, bool bFromQueue)
{
  xSceneEntryDynamic* pEntry = m_oRunner.getEntry();
  if (pEntry == nullptr)
    return false;

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(oCmd.skillid());
  if (pSkillCFG == nullptr)
  {
    XERR << "[技能-释放] " << pEntry->id << ", " << pEntry->name << ", skillid : " << oCmd.skillid() << " 未在Table_Skill.txt表中找到" << XEND;
    return false;
  }

  SkillRunner& curRunner = (m_oRunner.getState() == ESKILLSTATE_CHANT && pEntry->isCanMoveChant() && pSkillCFG->isNormalSkill(pEntry)) ? m_oHelpRunner : m_oRunner;

  QWORD qwEnsemblePartner = 0;
  if (pSkillCFG->getSkillType() == ESKILLTYPE_ENSEMBLE)
  {
    const EnsembleSkill* pEnsemble = dynamic_cast<const EnsembleSkill*>(pSkillCFG);
    if (pEnsemble == nullptr)
      return false;
    DWORD replaceskillid = 0;
    xSceneEntryDynamic* pPartner = pEnsemble->getPartner(pEntry, replaceskillid);
    if (pPartner == nullptr)
      return false;

    SceneUser* pUser = dynamic_cast<SceneUser*>(pEntry);
    if (pUser == nullptr || pUser->getFighter() == nullptr)
      return false;
    if (pUser->getFighter()->getSkill().isSkillEnable(oCmd.skillid()) == false)
      return false;

    qwEnsemblePartner = pPartner->id;
    const BaseSkill* pReplaceSkillCFG = SkillManager::getMe().getSkillCFG(replaceskillid);
    if (pReplaceSkillCFG == nullptr)
      return false;
    if (bFromQueue == false)
    {
      pSkillCFG = pReplaceSkillCFG;
      const_cast<SkillBroadcastUserCmd&>(oCmd).set_skillid(replaceskillid);
    }
  }

  if (pEntry->canUseSkill(pSkillCFG) == false)
  {
    curRunner.reset();
    return false;
  }

  if (pSkillCFG->canUseToTarget(pEntry, oCmd) == false)
  {
    curRunner.reset();
    return false;
  }

  SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
  if (pUser)
  {
    if (pSkillCFG->isNormalSkill(pEntry))
      m_oNormalQueueCnt.second ++;
    else
      m_oNoNormalQueueCnt.second ++;
  }

  QWORD curMSec = xTime::getCurMSec();
  // 技能队列
  if (pUser)
  {
    if (!bFromQueue)
      m_stMoveCheckData.bWaitCheck = false;

    auto putQueue = [&](ESkillQueueType eType, QWORD qwTime = 0)
    {
      if (bFromQueue)
      {
        m_bQueueStillQueue = true;
        return;
      }
      if (m_oQueueCmd.size() > 5)
      {
        while(!m_oQueueCmd.empty())
          m_oQueueCmd.pop();
        XERR << "[技能-队列], 技能队列过长, 异常, 清空队列, 玩家:" << pEntry->name << ", " << pEntry->id << XEND;
      }
      SSkillQueueAtom stAtom;
      stAtom.qwTimeOut = qwTime;
      stAtom.eType = eType;
      stAtom.oSkillCmd.CopyFrom(oCmd);
      stAtom.qwTimeIn = curMSec;
      m_oQueueCmd.push(stAtom);
      XDBG << "[技能-队列], 技能消息入队, 玩家:" << pEntry->name << ", " << pEntry->id << ", 技能:" << oCmd.skillid() << ", 时间:" << qwTime << ", 队列size:" << m_oQueueCmd.size() << eType << oCmd.data().number() <<XEND;
    };
    if (curRunner.getState() != ESKILLSTATE_MIN && curRunner.getCFG() != pSkillCFG) // 上一技能未结束
    {
      putQueue(ESKILLQUEUETYPE_WAITSTATE);
      return false;
    }
    //if (pSkillCFG->isNormalSkill(pEntry) && !checkActionTime(xTime::getCurMSec())) // 普攻, 攻速过快
    //bool noNeedCheck = (m_bLastSkillIsNormal && pSkillCFG->isNormalSkill(pEntry) == false); // 允许普攻后立即接技能
    //if (!noNeedCheck)
    bool  noNeedCheck = m_setServerTrigSkills.find(oCmd.skillid()) != m_setServerTrigSkills.end();

    if (!noNeedCheck && !checkActionTime(curMSec))
    {
      if (pSkillCFG->isNormalSkill(pEntry))
      {
        putQueue(ESKILLQUEUETYPE_NORMALSKILL, m_qwNextActionTime);
        return false;
      }
      else
      {
        if (CommonConfig::m_dwSkillDelayMs != 0)
        {
          putQueue(ESKILLQUEUETYPE_NORMALSKILL, m_qwNextActionTime);
          return false;
        }
      }
    }
    if (!bFromQueue && !m_oQueueCmd.empty()) // 队列中已有等待
    {
      putQueue(ESKILLQUEUETYPE_WAITSTATE);
      return false;
    }
    if (curRunner.getCFG() == pSkillCFG && 0 < curRunner.getStartTime()) // 吟唱过快
    {
      if (curRunner.getState() != ESKILLSTATE_CHANT)
      {
        XERR << "[技能], 释放非法, 未经过吟唱直接释放" << pEntry->name << pEntry->id << "技能:" << pSkillCFG->getSkillID() << XEND;
        return false;
      }
      QWORD qwCurMSec = curMSec;
      QWORD qwReadyTime = curRunner.getCmd().chanttime();
      QWORD qwReadyTimeErr = qwReadyTime > CHANT_ERROR ? qwReadyTime - CHANT_ERROR : 0;
      QWORD qwStartTime = curRunner.getStartTime() - TIME_SKILL_OFFSET_MSEC;
      if (qwCurMSec < qwStartTime + qwReadyTimeErr)
      {
        putQueue(ESKILLQUEUETYPE_CHANT, qwStartTime + qwReadyTime);
        return false;
      }
    }
    if (curRunner.getState() != ESKILLSTATE_CHANT && pUser->m_oCDTime.skilldone(pSkillCFG->getSkillID()) == false) // CD 未到
    {
      putQueue(ESKILLQUEUETYPE_CD, pUser->m_oCDTime.getSkillCDEndTime(pSkillCFG->getSkillID()));
      return false;
    }
    // 位置同步
    // master client(A), server(S), slave client(B)
    // A -> send(move, move, skill) -> S, S -> send(move, move, skill) -> B
    //pUser->m_oMove.forceSync();

    bool noCheckSkillDis = false;
    if (pSkillCFG->getSkillType() == ESKILLTYPE_TOUCHPETSKILL)// spec.抚摸宠物技能, 位置等条件不检查
      noCheckSkillDis = true;

    if (!noCheckSkillDis)
    {
      if (pUser->m_oMove.empty() == false)
      {
        if (pSkillCFG->getReadyTime(pEntry) != 0 && pUser->isCanMoveChant())
          pUser->m_oMove.moveOnce();
        else if (!bFromQueue && pSkillCFG->getLogicType() == ESKILLLOGIC_LOCKEDTARGET && oCmd.data().hitedtargets_size())
        {
          // 服务端超前客户端移动, 释放技能前, 矫正一次位置
          xPos pos = pUser->m_oMove.getDestPos();
          xSceneEntryDynamic* target = getOneTarget(oCmd);
          if (target && getXZDistance(pUser->getPos(), target->getPos()) < getXZDistance(pos, target->getPos()))
          {
            pUser->m_oMove.moveOnce();
          }
        }
        // 带位移的技能, 释放前, 矫正一次位置 (服务端客户端, 可能寻路不一致, 但目标坐标一致)
        else if (!bFromQueue && pSkillCFG->hasAttMove())
        {
          xPos pos = pUser->getPos();
          xPos destpos = pUser->m_oMove.getDestPos();
          xPos lastdestpos = pUser->m_oMove.getLastDestPos();
          float clientDis = getXZDistance(destpos, lastdestpos);
          float serverDis = getXZDistance(pos, lastdestpos);
          if (serverDis < clientDis) // a ... b .. c, 目标点c, 从a到c, 在b点停止放技能
            pUser->m_oMove.moveOnce();

          if (pUser->m_oMove.empty() == false)
          {
            float dis = getXZDistance(pos, destpos);
            if (m_stMoveCheckData.checkScenePos() && dis < 5)
            {
              pUser->setScenePos(destpos);
              pUser->m_oMove.stop();
              // 记录瞬移次数
              if (dis > 1)
                m_stMoveCheckData.oPairAttMoveCheck.second ++;

              XDBG << "[玩家-技能位移], 瞬移位置, 玩家:" << pUser->name << pUser->id << "设置前坐标:" << pos << "设置后坐标:" << destpos << "技能:" << oCmd.skillid() << XEND;
            }
            else
            {
              XERR << "[玩家-技能位移], 强制设置位置, 玩家:" << pUser->name << pUser->id << "技能:" << oCmd.skillid () << "服务端位置:" << pos << "客户端位置:" << destpos
              << "技能次数:" << m_stMoveCheckData.oPairAttMoveCheck.first << "瞬移次数:" << m_stMoveCheckData.oPairAttMoveCheck.second << XEND;
              pUser->m_oMove.stopAtonce();
              m_stMoveCheckData.oPairAttMoveCheck.first = m_stMoveCheckData.oPairAttMoveCheck.second = 0;
            }
          }
          m_stMoveCheckData.oPairAttMoveCheck.first += 1;
        }

        if (!(pSkillCFG->getReadyTime(pEntry) != 0 && pUser->isCanMoveChant()))
        {
          putQueue(ESKILLQUEUETYPE_SKILLDIS);
          return false;
        }
      }

      // 检查释放距离合法性(有吟唱的技能第二阶段不检查)
      if (curRunner.getCFG() != pSkillCFG)
      {
        ECheckDisResult eResult = checkSkillDistance(oCmd);
        if (eResult != ECHECKDISRESULT_SUCCESS)
        {
          if (eResult == ECHECKDISRESULT_DISFAIL)
            putQueue(ESKILLQUEUETYPE_SKILLDIS);
          return false;
        }
        QWORD targetid = oCmd.data().hitedtargets_size() ? oCmd.data().hitedtargets(0).charid() : 0;
        if (pSkillCFG->checkUseItem(pUser, targetid) == false)
        {
          //breakSkill(pUser->id);
          sendBreakSkill(pSkillCFG->getSkillID());
          curRunner.reset();
          return false;
        }
      }
    }

    //pUser->m_oMove.forceSync();
    if (pUser->getStatus() == ECREATURESTATUS_FAKEDEAD && pSkillCFG->getSkillType() != ESKILLTYPE_FAKEDEAD)
      pUser->setStatus(ECREATURESTATUS_LIVE);

    // 下坐骑
    const SRoleBaseCFG* pCFG = pUser->getRoleBaseCFG();
    if (pCFG && !(pUser->isAttackCanMount() && pCFG->bRideAction) && pSkillCFG->getSkillType() != ESKILLTYPE_FLASH)
    {
      EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
      if (pEquipPack)
      {
        ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_MOUNT);
        if (pEquip && pEquip->getType() != EITEMTYPE_BARROW && MiscConfig::getMe().getItemCFG().isMachineMount(pEquip->getTypeID()) == false)
          pUser->getPackage().equipOff(EEQUIPPOS_MOUNT);
      }
    }

    if (curRunner.getCFG() != pSkillCFG)
    {
      pUser->getHandNpc().onUseSkill();
      pUser->getEvent().onUseSkill(pSkillCFG);
    }
  }

  curRunner.getEntry()->setAction(0);
  curRunner.setCFG(pSkillCFG);
  curRunner.setCmd(oCmd);
  curRunner.setEnsemblePartner(qwEnsemblePartner);

  SkillBroadcastUserCmd& rCmd = curRunner.getCmd();
  rCmd.set_charid(curRunner.getEntry()->id);
  rCmd.set_skillid(pSkillCFG->getSkillID());

  DWORD dwOffset = pEntry->getEntryType() == SCENE_ENTRY_USER ? TIME_SKILL_OFFSET_MSEC : 0;
  if (pSkillCFG->getPetID() != 0 && pUser && pUser->getPet().getPartnerID())
  {
    rCmd.set_petid(pUser->getPet().getPartnerID());
    // 服务器触发技能 不设延迟时间
    dwOffset = 0;
  }

  if (m_oRunner.getState() == ESKILLSTATE_CHANT && pEntry->isCanMoveChant() && pSkillCFG->isNormalSkill(pEntry))
  {
    curRunner.setState(ESKILLSTATE_RUN);
    runMainRunner(curMSec, m_oHelpRunner);
    return true;
  }

  if (!pSkillCFG->isNormalSkill(pEntry))
  {
    if (pUser)
    {
      //plat log
      QWORD eid = xTime::getCurUSec();
      EVENT_TYPE eventType = EventType_UseSkill;
      PlatLogManager::getMe().eventLog(thisServer,
        pUser->getUserSceneData().getPlatformId(),
        pUser->getZoneID(),
        pUser->accid,
        pUser->id,
        eid,
        pUser->getUserSceneData().getCharge(), eventType, 0, 1);

      PlatLogManager::getMe().UseSkillLog(thisServer,
        pUser->getUserSceneData().getPlatformId(),
        pUser->getZoneID(),
        pUser->accid,
        pUser->id,
        eventType,
        eid,/*eid*/
        pSkillCFG->getSkillID());
      
    }
  }

  // add normal skill delay according atkspd
  // 放在check释放合法之后
  /*if (pSkillCFG->isNormalSkill(pEntry))
  {
    float atkSpd = pEntry->getAttr(EATTRTYPE_ATKSPD);
    DWORD add = ATTACK_SPEED_BASE;
    if (atkSpd)
      add /= atkSpd;
    addActionTime(add);
  }
  else
  {
    //addActionTime(CommonConfig::m_dwSkillDelayMs);
  }
  */

  float readytime = pSkillCFG->getReadyTime(pEntry);
  if (readytime != 0 && oCmd.data().number() == ESKILLNUMBER_ATTACK && curRunner.getState() != ESKILLSTATE_CHANT)
  {
    if (readytime > SKILL_READY_ERROR) //小数点等导致的容错
    {
      curRunner.reset();
      XERR << "[技能], 释放非法, 未经过吟唱直接释放" << pEntry->name << pEntry->id << "技能:" << pSkillCFG->getSkillID() << XEND;
      return false;
    }
    readytime = 0;
  }

  if (oCmd.data().number() == ESKILLNUMBER_ATTACK || oCmd.data().number() == ESKILLNUMBER_CAIJI || readytime == 0)
  {
    curRunner.setState(ESKILLSTATE_RUN);
    pEntry->getBuff().onChantStatusChange();
    // 无吟唱, 但有持续时间的技能, 需要设置开始时间
    if (readytime == 0)
      curRunner.setStartTime(curMSec + dwOffset);
    return true;
  }

  // 同步吟唱时间
  if (readytime != 0)
    curRunner.getCmd().set_chanttime(readytime);

  if (pSkillCFG->prepare(curRunner) == false)
  {
    curRunner.reset();
    return false;
  }

  curRunner.setNextTime(0);
  curRunner.setCount(0);
  curRunner.setStartTime(curMSec + dwOffset);
  curRunner.setState(ESKILLSTATE_CHANT);

  // 吟唱期间不可移动
  if (!pEntry->isCanMoveChant())
    pEntry->m_oMove.addActionTime(readytime);

  addCDBeforeChant(pSkillCFG);
  pEntry->getBuff().onChantStatusChange();

  return true;
}

bool SkillProcessor::useBuffSkill(xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy, DWORD skillid, bool isActive, bool free, bool ignoreCD)
{
  if (attacker == nullptr || enemy == nullptr || attacker->getScene() == nullptr || enemy->getScene() == nullptr)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (enemy);
  if(user && user->getDressUp().getDressUpStatus() != 0)
    return false;

  if (isActive && (m_oRunner.getState() == ESKILLSTATE_CHANT && !attacker->isCanMoveChant()))
    return false;

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
  {
    XERR << "[技能-buff释放] " << attacker->id << ", " << attacker->name << ", skillid : " << skillid << " 未在Table_Skill.txt表中找到" << XEND;
    return false;
  }
  if (pSkillCFG->getSkillType() == ESKILLTYPE_PASSIVE)
    return false;

  //if (!attacker->m_oCDTime.skilldone(skillid)) return false;
  // check and add cd, diff with normal skill
  DWORD cd = pSkillCFG->getCD(attacker);
  DWORD delay = pSkillCFG->getDelayCD(attacker);
  cd = cd > delay ? cd : delay;
  QWORD curm = xTime::getCurMSec();
  auto it = m_mapBuffSkillCD.find(skillid);

  if (!ignoreCD) 
  {
    if (it == m_mapBuffSkillCD.end())
    {
      m_mapBuffSkillCD[skillid] = curm + cd;
    }
    else
    {
      if (curm < it->second)
      {
        return false;
      }
      it->second = curm + cd;
    }
  }

  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (isActive == false)
  {
    SkillRunner tempRunner;
    tempRunner.setCFG(pSkillCFG);
    tempRunner.setEntry(attacker);

    Cmd::SkillBroadcastUserCmd cmd;
    cmd.set_skillid(skillid);
    Cmd::PhaseData *pData = cmd.mutable_data();
    Cmd::ScenePos *p = pData->mutable_pos();
    xPos pos = enemy->getPos();
    p->set_x(pos.getX());
    p->set_y(pos.getY());
    p->set_z(pos.getZ());
    Cmd::HitedTarget *pT = pData->add_hitedtargets();
    pT->set_charid(enemy->id);

    if (pSkillCFG->getPetID() != 0 && pUser)
    {
      cmd.set_petid(pUser->getPet().getPartnerID());
    }

    pData->set_number(ESKILLNUMBER_RET);

    tempRunner.setCmd(cmd);
    tempRunner.setNoTarget();

    //addCDBeforeRun(pSkillCFG);

    tempRunner.setState(ESKILLSTATE_RUN);
    tempRunner.setStartTime(curm);
    if (pSkillCFG->isContinueSkill() == false)
      pSkillCFG->run(tempRunner);
    else
      m_listExtraRunner.push_back(tempRunner);

    return true;
  }

  if (pUser == nullptr)
  {
    if (attacker->m_oMove.empty() == false)
      return false;
    //clearActionTime();
    return attacker->useSkill(skillid, enemy->id, enemy->getPos());
  }

  Cmd::SkillBroadcastUserCmd cmd;
  cmd.set_petid(-1); // -1 表示服务器触发技能

  cmd.set_charid(attacker->id);
  cmd.set_skillid(skillid);

  Cmd::PhaseData *pData = cmd.mutable_data();
  if (pSkillCFG->getLogicType() == ESKILLLOGIC_LOCKEDTARGET || pSkillCFG->getLogicType() == ESKILLLOGIC_MISSILE)
  {
    Cmd::HitedTarget *pT = pData->add_hitedtargets();
    pT->set_charid(enemy->id);
    bool bCheckDis = true;
    if (enemy->getEntryType() == SCENE_ENTRY_NPC)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*>(enemy);
      if (pNpc && !pNpc->isMonster())
        bCheckDis = false;
    }
    if (bCheckDis && (getXZDistance(enemy->getPos(), attacker->getPos()) > pSkillCFG->getLaunchRange(attacker)))
      return false;
  }

  Cmd::ScenePos *p = pData->mutable_pos();
  xPos pos = enemy->getPos();
  p->set_x(pos.getX());
  p->set_y(pos.getY());
  p->set_z(pos.getZ());

  SceneFighter* pf = pUser->getFighter();
  if (pf != nullptr)
    pf->getSkill().addTempSkill(skillid, free);
  m_setServerTrigSkills.insert(skillid);

  PROTOBUF(cmd, send, len);
  attacker->sendCmdToMe(send, len);

  return true;
}

void SkillProcessor::showSkill(DWORD skillid)
{
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_oRunner.getEntry());
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr || pSkillCFG->getSkillType() == ESKILLTYPE_PASSIVE)
    return;
  xSceneEntryDynamic* target = nullptr;
  if (pSkillCFG->getLogicType() == ESKILLLOGIC_LOCKEDTARGET)
  {
    float skilldist = pSkillCFG->getLaunchRange(pUser);
    CHECK_DIS_PARAM(skilldist, checkdist);
    xSceneEntrySet uset;
    pUser->getScene()->getEntryList(pUser->getPos(), checkdist, uset);
    for (auto &s : uset)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*>(s);
      if (npc && npc->isMonster())
      {
        target = (xSceneEntryDynamic*)npc;
        break;
      }
    }
  }
  else
  {
    target = pUser;
  }
  if (target == nullptr)
    return;

  SceneFighter* pf = pUser->getFighter();
  if (pf != nullptr)
    pf->getSkill().addTempSkill(skillid, true);
  pUser->useSkill(skillid, target->id, target->getPos());
  return;

  /*
  Cmd::SkillBroadcastUserCmd cmd;
  //cmd.set_petid(-1); // -1 表示服务器触发技能

  cmd.set_charid(pUser->id);
  cmd.set_skillid(skillid);

  Cmd::PhaseData *pData = cmd.mutable_data();
  Cmd::ScenePos *p = pData->mutable_pos();
  xPos pos = pUser->getPos();
  xPos outPos;
  if (pUser->getScene()->getRandPos(pos, 3, outPos) == false)
    return;
  p->set_x(outPos.getX());
  p->set_y(outPos.getY());
  p->set_z(outPos.getZ());

  SceneFighter* pf = pUser->getFighter();
  if (pf != nullptr)
    pf->getSkill().addTempSkill(skillid, true);
  setActiveSkill(cmd);
  //PROTOBUF(cmd, send, len);
  //pUser->sendCmdToNine(send, len);
  */
}

bool SkillProcessor::breakSkill(EBreakSkillType eType, QWORD qwBreakerID, DWORD param, bool bFromClient)
{
  if (m_oRunner.getState() != ESKILLSTATE_CHANT)
    return false;

  const BaseSkill* pSkillCFG = m_oRunner.getCFG();
  xSceneEntryDynamic* pEntry = m_oRunner.getEntry();
  if (pSkillCFG == nullptr || pEntry == nullptr)
  {
    m_oRunner.reset();
    return false;
  }

  // 蝴蝶翅膀仅死亡打断
  if (pSkillCFG->getSkillType() == ESKILLTYPE_TRANSPORT && eType != EBREAKSKILLTYPE_DEAD)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser *> (pEntry);
  bool realbreak = false;
  switch(eType)
  {
    case EBREAKSKILLTYPE_HP:
      {
        if (pEntry->isSkillNoBreak())
          return false;
        if (pSkillCFG->getLeadType() == ESKILLLEADTYPE_ONE || pSkillCFG->getLeadType() == ESKILLLEADTYPE_LEAD)
          return false;
        if (pSkillCFG->noHpBreak())
          return false;
        m_oRunner.addBreakHp(param);
        float damper = 0.1f;
        if (pEntry->getEntryType() == SCENE_ENTRY_USER)
          damper = LuaManager::getMe().call<float>("CalcBreakSkillDamPer", (SceneUser*)(pEntry));
        if (m_oRunner.getBreakHp() >= pEntry->getAttr(EATTRTYPE_MAXHP) * damper)
          realbreak = true;
        if (realbreak && pUser)
        {
          QWORD qwReadyTime = m_oRunner.getCmd().chanttime();
          QWORD qwStartTime = m_oRunner.getStartTime() - TIME_SKILL_OFFSET_MSEC;
          QWORD curMSec = xTime::getCurMSec();
          if (qwStartTime + qwReadyTime >= curMSec && qwStartTime + qwReadyTime <= CommonConfig::m_dwSkillBreakErr + curMSec)
          {
            realbreak = false;
            //XDBG << "打断失败, 技能时间" << qwReadyTime << qwStartTime << curMSec << "差:" << qwStartTime + qwReadyTime -curMSec << XEND;
          }
        }
      }
      break;
    case EBREAKSKILLTYPE_BUFF:
      {
        DWORD limit = pSkillCFG->getBuffBreakLimit();
        if (limit & param)
          return false;
        realbreak = true;
        for (auto &v : m_listRunner)
        {
          if (v.getCFG() && (v.getCFG()->getSkillType() == ESKILLTYPE_SOLO || v.getCFG()->getSkillType() == ESKILLTYPE_ENSEMBLE))
            v.setDelStatus();
        }
      }
      break;
    case EBREAKSKILLTYPE_SYSTEM:
      realbreak = true;
      break;
    case EBREAKSKILLTYPE_SKILL:
      realbreak = true;
      break;
    case EBREAKSKILLTYPE_DEAD:
      realbreak = true;
      break;
    default:
      break;
  }
  if (!realbreak)
    return false;

  SkillBroadcastUserCmd cmd;
  cmd.set_skillid(pSkillCFG->getSkillID());
  cmd.set_charid(pEntry->id);
  cmd.mutable_data()->set_number(ESKILLNUMBER_BREAK);

  if (!bFromClient)
    cmd.set_petid(-1); // client need

  if (qwBreakerID != 0)
    m_oRunner.setBreaker(qwBreakerID);

  if (pUser != nullptr)
  {
    pUser->getEvent().onBeBreakSkill(m_oRunner.getBreaker());//, pSkillCFG->getSkillID());
  }

  m_oRunner.setState(ESKILLSTATE_MIN);
  pEntry->getBuff().onChantStatusChange();
  pSkillCFG->onBeBreak(m_oRunner);

  PROTOBUF(cmd, send, len);
  pEntry->sendCmdToNine(send, len);
  m_oRunner.reset();
  pEntry->checkEmoji("BeBreak");

  return true;
}

void SkillProcessor::timer(QWORD curTime)
{
  if (m_bClear)
  {
    m_bClear = false;
    clear();
    return;
  }
  xSceneEntryDynamic* pEntry = m_oRunner.getEntry();
  if (!pEntry)
    return;

  DWORD execTimeOut = CommonConfig::m_dwSkillExecPrintTime;
  xTime frameTimer;

  if (!m_oQueueCmd.empty())// && getRunner().getState() == ESKILLSTATE_MIN)
  {
    auto it = m_oQueueCmd.front();
    m_bQueueStillQueue = false;
    DWORD quesize = m_oQueueCmd.size();

    switch(it.eType)
    {
      case ESKILLQUEUETYPE_NORMALSKILL:
        {
          if (curTime >= it.qwTimeOut && (m_oRunner.getState() == ESKILLSTATE_MIN || (m_oRunner.getState() == ESKILLSTATE_CHANT && pEntry->isCanMoveChant())))
          {
            setActiveSkill(it.oSkillCmd, true);
            if (!m_bQueueStillQueue)
            {
              XDBG << "[技能-队列], 出队, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
              m_oQueueCmd.pop();
            }
            else
            {
              XDBG << "[技能-队列], 出队失败, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
            }
          }
        }
        break;
      case ESKILLQUEUETYPE_CHANT:
      case ESKILLQUEUETYPE_CD:
        {
          if (curTime >= it.qwTimeOut)
          {
            setActiveSkill(it.oSkillCmd, true);
            if (!m_bQueueStillQueue)
            {
              XDBG << "[技能-队列], 出队, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
              m_oQueueCmd.pop();
            }
            else
            {
              XDBG << "[技能-队列], 出队失败, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
            }
          }
        }
        break;
      case ESKILLQUEUETYPE_WAITSTATE:
        {
          if (m_oRunner.getState() == ESKILLSTATE_MIN || (m_oRunner.getCFG() && m_oRunner.getCFG()->getSkillID() == it.oSkillCmd.skillid()) || (m_oRunner.getState() == ESKILLSTATE_CHANT && pEntry->isCanMoveChant()))
          {
            setActiveSkill(it.oSkillCmd, true);
            if (!m_bQueueStillQueue)
            {
              XDBG << "[技能-队列], 出队, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
              m_oQueueCmd.pop();
            }
            else
            {
              XDBG << "[技能-队列], 出队失败, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
            }
          }
        }
        break;
      case ESKILLQUEUETYPE_SKILLDIS:
        {
          if (m_oRunner.getState() == ESKILLSTATE_MIN || (m_oRunner.getState() == ESKILLSTATE_CHANT && pEntry->isCanMoveChant()))
          {
            setActiveSkill(it.oSkillCmd, true);
            if (!m_bQueueStillQueue)
            {
              XDBG << "[技能-队列], 出队, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
              m_oQueueCmd.pop();
              //m_dwSkillDisErrCnt = 0;
            }
            else
            {
              /*m_dwSkillDisErrCnt ++;
              if (m_dwSkillDisErrCnt >= CommonConfig::m_dwSkillDisQueueMax)
              {
                XERR << "[技能-队列], 出队异常, 开挂嫌疑, 连续多次释放距离非法, 删除技能, 玩家:" << pEntry->name << pEntry->id << "技能:" << it.oSkillCmd.skillid() << XEND;
                m_dwSkillDisErrCnt = 0;
                m_oQueueCmd.pop();
                break;
              }
              */
              XDBG << "[技能-队列], 出队失败, 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
            }
          }
        }
        break;
      case ESKILLQUEUETYPE_MIN:
        break;
      default:
        break;
    }
    if (m_oQueueCmd.empty() == false && m_oQueueCmd.size() == quesize)
    {
      it = m_oQueueCmd.front();
      if (it.qwTimeIn + CommonConfig::m_dwSkillQueueDelayMs < curTime)
      {
        XDBG << "[技能-队列], 出队超时, 删除该技能. 玩家:" << pEntry->name << ", " << pEntry->id << "技能:" << it.oSkillCmd.skillid() << "队列类型:" << it.eType << XEND;
        if (pEntry->getNormalSkill() == it.oSkillCmd.skillid())
          m_oNormalQueueCnt.first ++;
        else
          m_oNoNormalQueueCnt.first ++;
        XDBG << "[技能-队列], 当前超时统计:" << pEntry->name << pEntry->id << m_oNormalQueueCnt.first << m_oNormalQueueCnt.second << m_oNoNormalQueueCnt.first << m_oNoNormalQueueCnt.second << XEND;
        m_oQueueCmd.pop();
        //m_dwSkillDisErrCnt = 0;
      }
    }
  }

  QWORD e = frameTimer.uElapse();
  if (e >= execTimeOut)
    XLOG << "[技能超时统计-出队], 对象:" << pEntry->name << pEntry->id << "执行时间:" << e << "微妙" << "队首技能:" << m_oQueueCmd.front().oSkillCmd.skillid() << XEND;

  if (m_oQueueNpcSkill.empty() == false)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (pEntry);
    if (npc && npc->isAttrCanSkill() && npc->m_oMove.checkActionTime() && m_oRunner.getState() == ESKILLSTATE_MIN)
    {
      auto it = m_oQueueNpcSkill.front();
      npc->useSkill(it.dwSkillID, it.qwTargetID, it.oSkillPos, it.bSpecPos);
      m_oQueueNpcSkill.pop();
    }
  }

  frameTimer.elapseStart();
  runMainRunner(curTime, m_oRunner);

  e = frameTimer.uElapse();
  if (e >= execTimeOut)
    XLOG << "[技能超时统计-MainRunner], 对象:" << pEntry->name << pEntry->id << "技能:" << m_oRunner.getCmd().skillid() << "执行时间:" << e << "微妙" << XEND;

  runSubRunner(curTime);
}

void SkillProcessor::runMainRunner(QWORD curTime, SkillRunner& runner)
{
  const BaseSkill* pCFG = runner.getCFG();
  if (pCFG == nullptr)
    return;

  switch (runner.getState())
  {
    case ESKILLSTATE_MIN:
      break;
    case ESKILLSTATE_CHANT:
      {
        if (pCFG->chant(runner) == false)
        {
          runner.reset();
          break;
        }

        QWORD chanterr = pCFG->isCostBeforeChant() ? TIME_SKILL_OFFSET_MSEC : 0; // 引导技能等(吟唱期间产生技能效果), 吟唱结束不等客户端发送
        if (curTime + chanterr > runner.getStartTime() + runner.getCmd().chanttime())
        {
          runner.setState(ESKILLSTATE_RUN);
          xSceneEntryDynamic* pEntry = runner.getEntry();
          if (pEntry != nullptr)
            pEntry->getBuff().onChantStatusChange();
        }
      }
      break;
    case ESKILLSTATE_RUN:
      {
        xSceneEntryDynamic* pEntry = runner.getEntry();
        if (pEntry != nullptr)
        {
          // 超出释放距离打断技能
          float param = pEntry->getEntryType() == SCENE_ENTRY_USER ? 2 : 1.5;
          if (pCFG->isOutRangeBreak() && runner.getCmd().data().hitedtargets_size() >= 1)
          {
            xSceneEntryDynamic* penemy = xSceneEntryDynamic::getEntryByID(runner.getCmd().data().hitedtargets(0).charid());
            if (penemy == nullptr)
            {
              sendBreakSkill(pCFG->getSkillID());
              runner.reset();
              XLOG << "[技能], 目标不存在, 不可攻击, attacer = " << pEntry->id << ", enemy = " << runner.getCmd().data().hitedtargets(0).charid() << XEND;
              break;
            }
            float dis = getXZDistance(penemy->getPos(), pEntry->getPos());
            float launch = pCFG->getLaunchRange(pEntry);
            if (penemy && dis > launch * param)
            {
              sendBreakSkill(pCFG->getSkillID());
              runner.reset();
              XLOG << "[技能], 目标距离过远, 不可攻击, attacer =  " << pEntry->id << ", enemy = " << penemy->id << ", 双方距离:" << dis << ", 技能释放距离:" << launch << XEND;
              break;
            }
            if (penemy && penemy->getAttr(EATTRTYPE_HIDE) != 0)
            {
              sendBreakSkill(pCFG->getSkillID());
              runner.reset();
              XLOG << "[技能], 目标已经隐身, 不可攻击, attacer = " << pEntry->id << ", enemy = " << penemy->id << XEND;
              break;
            }
          }
          SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
          if (((pUser && pUser->getTransform().isMonster() == false) || (dynamic_cast<BeingNpc*>(pEntry) != nullptr)) && pCFG->isCostBeforeChant() == false)
          {
            // 避免吟唱期间, 道具被消耗
            if (pCFG->checkSkillCost(pEntry) == false)
            {
              runner.reset();
              break;
            }
            pCFG->doSkillCost(pEntry, runner);
          }
          addCDBeforeRun(pCFG);

          if (pCFG->isNormalSkill(pEntry))
          {
            float atkSpd = pEntry->getAttr(EATTRTYPE_ATKSPD);
            DWORD add = ATTACK_SPEED_BASE;

            // 攻速修正
            if (atkSpd)
              atkSpd = (1.0 / (1.0 / atkSpd + 0.1)) * 1.05;

            if (atkSpd)
              add /= atkSpd;
            pEntry->m_oMove.addActionTime(add);
            addActionTime(add);
            //m_bLastSkillIsNormal = true;
          }
          // 怪物释放技能攻击动作1s延时
          //else if (pEntry->getEntryType() == SCENE_ENTRY_NPC)
          else
          {
            if (m_setServerTrigSkills.find(pCFG->getSkillID()) != m_setServerTrigSkills.end())
            {
              m_setServerTrigSkills.erase(pCFG->getSkillID());
            }
            else if (!pCFG->isSkillNoDelay())// 非服务端触发技能添加延时
            {
              pEntry->m_oMove.addActionTime(ATTACK_SPEED_BASE);

              //if (m_bLastSkillIsNormal)
                //clearActionTime(); // 普攻后允许立即接技能
              addActionTime(CommonConfig::m_dwSkillDelayMs);
            }
            //m_bLastSkillIsNormal = false;
          }
        }

        limitSkillNum(pCFG);
        m_listRunner.push_back(runner);
        runner.reset();
      }
      break;
    default:
      break;
  }
}

void SkillProcessor::runSubRunner(QWORD curTime)
{
  if (m_oRunner.getEntry() && m_oRunner.getEntry()->isAlive() == false && m_oRunner.getEntry()->getEntryType() == SCENE_ENTRY_NPC)
    return;
  for (auto v = m_listRunner.begin(); v != m_listRunner.end();)
  {
    if (v->getState() == ESKILLSTATE_MIN)
    {
      v = m_listRunner.erase(v);
      continue;
    }
    if (v->needDel())
    {
      v->end();
      v = m_listRunner.erase(v);
      continue;
    }

    v->timer(curTime);
    ++v;
  }
  if (!m_listExtraRunner.empty())
  {
    for (auto &v : m_listExtraRunner)
    {
      m_listRunner.push_back(v);
    }
    m_listExtraRunner.clear();
  }
}

void SkillProcessor::clear()
{
  TVecQWORD vecTrapIDs;
  QWORD qid = m_oRunner.getParam1();
  if (qid != 0)
    vecTrapIDs.push_back(qid);

  m_oRunner.reset();
  m_oHelpRunner.reset();

  for (auto v = m_listRunner.begin(); v != m_listRunner.end(); ++v)
  {
    QWORD id = v->getParam1();
    if (id != 0)
      vecTrapIDs.push_back(id);

    v->reset();
  }
  m_listRunner.clear();
  m_listExtraRunner.clear();

  for (auto &v : vecTrapIDs)
  {
    SceneTrap* pTrap = SceneTrapManager::getMe().getSceneTrap(v);
    if (pTrap != nullptr)
    {
      pTrap->setClearState();
      continue;
    }
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(v);
    if (pNpc != nullptr)
    {
      pNpc->setClearState();
      continue;
    }
  }
  clearTransInfo();

  while(m_oQueueCmd.empty() == false)
    m_oQueueCmd.pop();
  m_stMoveCheckData.clear();
}

DWORD SkillProcessor::getLastUseTime(DWORD skillid) const
{
  if (skillid > 1000)
    skillid /= 1000;
  auto m = m_mapSkill2Time.find(skillid);
  if (m != m_mapSkill2Time.end())
    return m->second;
  return 0;
}

void SkillProcessor::addSkillUseInfo(DWORD skillid, QWORD time)
{
  skillid /= 1000;
  time /= ONE_THOUSAND;
  auto m = m_mapSkill2Time.find(skillid);
  if (m == m_mapSkill2Time.end())
  {
    m_mapSkill2Time[skillid] = time;
    return;
  }
  m->second = time;
}

void SkillProcessor::addCD(const BaseSkill* pSkillCFG)
{
  if (pSkillCFG == nullptr)
    return;
  xSceneEntryDynamic* pEntry = m_oRunner.getEntry();
  if (pEntry == nullptr)
    return;

  // 卡片 装备 触发技能 不加cd
  SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
  if (pUser && pUser->getFighter() && pUser->getFighter()->getSkill().isFreeSkill(pSkillCFG->getSkillID()))
    return;

  DWORD delaycd = pSkillCFG->getDelayCD(pEntry);
  DWORD cd = pSkillCFG->getCD(pEntry);
  cd = delaycd > cd ? delaycd : cd;
  pEntry->m_oCDTime.add(pSkillCFG->getSkillID(), cd, CD_TYPE_SKILL);
  if (pSkillCFG->getDelayCD(pEntry) == 0)
    return;

  if (pUser && pUser->getFighter())
  {
    TVecDWORD vecIDs;
    pUser->getFighter()->getSkill().getCurSkills(vecIDs);
    DWORD normalskill = pUser->getNormalSkill();
    for (auto &v : vecIDs)
    {
      if (v == pSkillCFG->getSkillID() || normalskill == v)
        continue;
      pEntry->m_oCDTime.add(v, pSkillCFG->getDelayCD(pEntry), CD_TYPE_SKILLDEALY);
    }
  }
}

void SkillProcessor::addCDBeforeRun(const BaseSkill* pSkillCFG)
{
  if (pSkillCFG == nullptr || pSkillCFG->isCDBeforeChant() == true || pSkillCFG->getSkillType() == ESKILLTYPE_FAKEDEAD)
    return;
  addCD(pSkillCFG);
}

void SkillProcessor::addCDBeforeChant(const BaseSkill* pSkillCFG)
{
  if (pSkillCFG == nullptr || pSkillCFG->isCDBeforeChant() == false || pSkillCFG->getSkillType() == ESKILLTYPE_FAKEDEAD)
    return;
  addCD(pSkillCFG);
}

void SkillProcessor::addActionTime(DWORD time)
{
  QWORD cur = xTime::getCurMSec();
  if (m_qwNextActionTime < cur)
    m_qwNextActionTime = cur + time;
  else
    m_qwNextActionTime += time;
}

ECheckDisResult SkillProcessor::checkSkillDistance(const SkillBroadcastUserCmd& oCmd)
{
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(oCmd.skillid());
  if (pSkillCFG == nullptr)
    return ECHECKDISRESULT_FAIL;
  xSceneEntryDynamic* attacker = m_oRunner.getEntry();
  if (attacker == nullptr)
    return ECHECKDISRESULT_FAIL;

  xPos oPos;
  oPos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());
  float skilldist = pSkillCFG->getLaunchRange(attacker);
  float errdist = CommonConfig::m_fSkillSyncDis * attacker->getAttr(EATTRTYPE_MOVESPD);
  //CHECK_DIS_PARAM(skilldist, checkdist);

  switch (pSkillCFG->getLogicType())
  {
    case ESKILLLOGIC_FORWARDRECT:
      break;
    case ESKILLLOGIC_POINTRANGE: 
    case ESKILLLOGIC_POINTRECT:
    case ESKILLLOGIC_RANDOMRANGE:
      {
        // 区域型技能施法距离不限制
        /*if (attacker->isPointSkillFar())
          break;*/

        float dist = getXZDistance(oPos, attacker->getPos());
        if (attacker->isPointSkillFar())
          skilldist += 5;

        if (dist > skilldist + errdist)
        {
          XERR << "[技能], 释放距离不合法, 攻击者:" << attacker->name << ", " << attacker->id << ", 距离:" << dist << ", 技能距离:" << skilldist << ", 技能:" << pSkillCFG->getSkillID() << XEND;
          return ECHECKDISRESULT_DISFAIL;
        }
      }
      break;
    case ESKILLLOGIC_MIN:
    case ESKILLLOGIC_LOCKEDTARGET:
    case ESKILLLOGIC_MISSILE:
      {
        if (pSkillCFG->getLogicType() == ESKILLLOGIC_MIN)
        {
          if (pSkillCFG->haveNoTargets())
            break;
          if (oCmd.data().hitedtargets_size() == 0)
            break;
        }

        if (oCmd.data().hitedtargets_size() == 0)
        {
          XDBG << "[技能], 目标已死亡, 无法释放技能, 攻击者:" << attacker->name << ", " << attacker->id << ", 技能:" << pSkillCFG->getSkillID() << XEND;
          return ECHECKDISRESULT_FAIL;
        }
        xSceneEntryDynamic* enemy = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(0).charid());
        if (enemy == nullptr || enemy->getScene() != attacker->getScene())
        {
          QWORD enemyid = oCmd.data().hitedtargets(0).charid();
          if (m_qwInvalidAttackID == enemyid)
          {
            m_dwInvalidTimes++;
            if (m_dwInvalidTimes >= 10)
            {
              Cmd::DeleteEntryUserCmd cmd;
              cmd.add_list(m_qwInvalidAttackID);
              PROTOBUF(cmd, send, len);
              attacker->sendCmdToMe(send, len);
              m_qwInvalidAttackID = 0;
              m_dwInvalidTimes = 0;
            }
          }
          else
          {
            m_qwInvalidAttackID = enemyid;
            m_dwInvalidTimes = 1;
          }

          XDBG << "[技能], 目标已不在场景上, 玩家:" << attacker->name << ", " << attacker->id << ", 技能:" << pSkillCFG->getSkillID() << "enemy id:" << enemyid << XEND;
          return ECHECKDISRESULT_FAIL;
        }
        float dist = getXZDistance(enemy->getPos(), attacker->getPos());
        if (enemy->m_oMove.empty() == false)
        {
          float spd = enemy->getAttr(EATTRTYPE_MOVESPD);
          if (spd)
            dist -= CommonConfig::m_fEnemySpedErrDis / spd;
        }
        if (dist > skilldist + errdist)
        {
          XERR << "[技能], 释放距离不合法, 攻击者:" << attacker->name << ", " << attacker->id << ", 距离:" << dist << ", 技能距离:" << skilldist << ", 技能:" << pSkillCFG->getSkillID() << XEND;
          return ECHECKDISRESULT_DISFAIL;
        }

        QWORD dwTargetId = attacker->m_oBuff.getLimitSkillTarget(oCmd.skillid());
        if (dwTargetId)
        {
          if (enemy->id != dwTargetId)
            return ECHECKDISRESULT_FAIL;
        }
        if (pSkillCFG->getSkillCamp() == ESKILLCAMP_TEAM)
        {
          if (attacker->isMyTeamMember(enemy->id) == false || attacker == enemy)
            return ECHECKDISRESULT_FAIL;
        }
      }
      break;
    default:
      break;
  }
  return ECHECKDISRESULT_SUCCESS;
}

void SkillProcessor::limitSkillNum(const BaseSkill* pSkillCFG)
{
  if (pSkillCFG == nullptr)
    return;

  DWORD limitcnt = pSkillCFG->getLimitCount(m_oRunner.getEntry());
  if (limitcnt == 0)
    return;

  DWORD num = 0;
  for (auto &v : m_listRunner)
  {
    if (v.getCFG() == pSkillCFG)
      ++ num;
  }

  if (num >= limitcnt)
  {
    QWORD mintime = QWORD_MAX;
    for (auto &v : m_listRunner)
    {
      if (v.getCFG() == pSkillCFG && v.getStartTime() < mintime)
        mintime = v.getStartTime();
    }
    for (auto v = m_listRunner.begin(); v != m_listRunner.end(); ++v)
    {
      if (v->getCFG() == pSkillCFG && v->getStartTime() == mintime)
      {
        v->end();
        m_listRunner.erase(v);
        break;
      }
    }
  }

  auto func = [&](ESkillType eType, DWORD dwLimit)
  {
    DWORD curnum = 0;
    QWORD mintime = QWORD_MAX;
    for (auto &v : m_listRunner)
    {
      if (v.getCFG() && v.getCFG()->getSkillType() == eType)
      {
        ++curnum;
        if (v.getStartTime() < mintime)
          mintime = v.getStartTime();
      }
    }

    if(curnum >= dwLimit)
    {
      for (auto v = m_listRunner.begin(); v != m_listRunner.end(); ++v)
      {
        if (v->getCFG() && v->getCFG()->getSkillType() == eType && v->getStartTime() == mintime)
        { 
          v->end();
          m_listRunner.erase(v);
          break;
        }
      }
    }
  };

  // 陷阱类, 总个数限制
  if (pSkillCFG->getSkillType() == ESKILLTYPE_TRAPSKILL)
  {
    DWORD limit = LuaManager::getMe().call<DWORD> ("calcFormulaValue", m_oRunner.getEntry(), m_oRunner.getEntry(), 3);
    func(ESKILLTYPE_TRAPSKILL, limit);
  }

  //贤者系只能存在一个元素领域技能
  if (pSkillCFG->getSkillType() == ESKILLTYPE_ELEMENTTRAP)
  {
    func(ESKILLTYPE_ELEMENTTRAP, 1);
  }
}

void SkillProcessor::triggerTrap(QWORD id)
{
  if (m_oRunner.getParam1() == id)
  {
    m_oRunner.setTrapTrigger();
    return;
  }
  for (auto &v : m_listRunner)
  {
    if (v.getParam1() == id)
    {
      v.setTrapTrigger();
      return;
    }
  }
}

void SkillProcessor::triggerTrapByType(const TSetDWORD& setNpcid)
{
  if (isTrapThisType(m_oRunner.getParam1(),setNpcid)){
    m_oRunner.setTrapTrigger();
  }
  for (auto &v : m_listRunner){
    if (isTrapThisType(v.getParam1(),setNpcid)){
      v.setTrapTrigger();
    }
  }
}

void SkillProcessor::triggerAllTrap()
{
  if (m_oRunner.getCFG() && m_oRunner.getCFG()->getSkillType() == ESKILLTYPE_TRAPSKILL)
    m_oRunner.setTrapTrigger();
  for (auto &v : m_listRunner)
  {
    v.setTrapTrigger();
  }
}

//判断陷阱是否是某一类
bool SkillProcessor::isTrapThisType(DWORD tempid, const TSetDWORD& setNpcid)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(tempid);
  if(pNpc == nullptr || setNpcid.find(pNpc->getNpcID()) == setNpcid.end()){
    return false;
  }
  return true;
}

void SkillProcessor::runSkillAtonce(xSceneEntryDynamic* pTarget, DWORD skillid)
{
  // 怪物喊话立即放技能
  if (m_oRunner.getEntry() == nullptr || m_oRunner.getEntry()->getEntryType() != SCENE_ENTRY_NPC || pTarget == nullptr)
    return;

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
    return;
  SkillRunner tempRunner;
  tempRunner.setCFG(pSkillCFG);
  tempRunner.setEntry(m_oRunner.getEntry());

  Cmd::SkillBroadcastUserCmd cmd;
  cmd.set_skillid(skillid);
  Cmd::PhaseData *pData = cmd.mutable_data();
  Cmd::ScenePos *p = pData->mutable_pos();
  xPos pos = pTarget->getPos();
  p->set_x(pos.getX());
  p->set_y(pos.getY());
  p->set_z(pos.getZ());
  Cmd::HitedTarget *pT = pData->add_hitedtargets();
  pT->set_charid(pTarget->id);

  tempRunner.setCmd(cmd);
  pSkillCFG->run(tempRunner);
}

void SkillProcessor::sendBreakSkill(DWORD skillid)
{
  if (!m_oRunner.getEntry())
    return;
  SkillBroadcastUserCmd cmd;
  cmd.set_skillid(skillid);
  cmd.set_charid(m_oRunner.getEntry()->id);
  cmd.mutable_data()->set_number(ESKILLNUMBER_BREAK);
  cmd.set_petid(-1); // client need
  PROTOBUF(cmd, send, len);
  m_oRunner.getEntry()->sendCmdToNine(send, len);
}

void SkillProcessor::useTransportSkill(SceneUser* user, ETransportType eType, DWORD mapid/* = 0*/, const xPos* pTransPos /*= nullptr*/)
{
  if (user == nullptr || user->getScene() == nullptr)
    return;
  if (m_oRunner.getState() == ESKILLSTATE_CHANT)
    return;

  DWORD skillid = MiscConfig::getMe().getNewRoleCFG().dwTransSkill;
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
    return;
  Cmd::SkillBroadcastUserCmd cmd;
  cmd.set_petid(-1); // -1 表示服务器触发技能

  cmd.set_charid(user->id);
  cmd.set_skillid(skillid);

  m_oRunner.setTransInfo(eType, mapid, pTransPos);

  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

void SkillProcessor::addBeSkillTime(DWORD skillGroup, QWORD nexttime)
{
  auto it = m_mapBeSkill2Time.find(skillGroup);
  if (it != m_mapBeSkill2Time.end())
  {
    it->second = nexttime;
    return;
  }
  m_mapBeSkill2Time[skillGroup] = nexttime;
}

QWORD SkillProcessor::getNextBeSkillTime(DWORD skillGroup)
{
  auto it = m_mapBeSkill2Time.find(skillGroup);
  if (it != m_mapBeSkill2Time.end())
    return it->second;
  return 0;
}

void SkillProcessor::getTrapNpcs(DWORD familySkillID, TSetQWORD& setnpc) const
{
  for (auto &v : m_listRunner)
  {
    if (!v.getCFG())
      continue;
    if (v.getCFG()->getSkillID() / ONE_THOUSAND != familySkillID)
      continue;
    if (v.getParam1())
      setnpc.insert(v.getParam1());
  }
}

void SkillProcessor::getAllTrapNpcs(std::set<SceneNpc*>& npcset) const
{
  for (auto &v : m_listRunner)
  {
    if (!v.getParam1())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v.getParam1());
    if (npc)
      npcset.insert(npc);
  }
}

void SkillProcessor::onSkillNpcDie(QWORD npcguid)
{
  for (auto &v : m_listRunner)
  {
    if (v.getParam1() == npcguid)
      v.setState(ESKILLSTATE_END);
  }
}

void SkillProcessor::addQueueNpcSkill(DWORD skillid, QWORD targetid, xPos pos, bool specpos)
{
  SNpcSkillQuene oSkill;
  oSkill.dwSkillID = skillid;
  oSkill.qwTargetID = targetid;
  oSkill.oSkillPos = pos;
  oSkill.bSpecPos = specpos;

  m_oQueueNpcSkill.push(oSkill);
  if (m_oQueueNpcSkill.size() >= 3)
    m_oQueueNpcSkill.pop();
}

void SkillProcessor::onTriggerNpc(QWORD npcguid, SceneUser* user)
{
  for (auto &v : m_listRunner)
  {
    if (v.getParam1() == npcguid)
    {
      const TrapNpcSkill* pSkill = dynamic_cast<const TrapNpcSkill*> (v.getCFG());
      if (pSkill)
        pSkill->onTriggerNpc(v, user);
    }
  }
}

xSceneEntryDynamic* SkillProcessor::getOneTarget(const SkillBroadcastUserCmd& oCmd)
{
  if (oCmd.data().hitedtargets_size() == 0)
    return nullptr;
  return xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(0).charid());
}

void SkillProcessor::delTrapSkill(QWORD trapnpcid)
{
  for (auto &v : m_listRunner)
  {
    if (v.getParam1() == trapnpcid)
      v.setDelStatus();
  }
}

void SkillProcessor::addSkillMove(DWORD skillid, float distance, const xPos& curPos)
{
  m_stMoveCheckData.dwSkillID = skillid;
  m_stMoveCheckData.fMoveDistance = distance;
  m_stMoveCheckData.oOriPos = curPos;
  m_stMoveCheckData.bWaitCheck = true;
}


void SkillProcessor::checkSkillMove(DWORD skillid, const ScenePos& pos)
{
  if (!m_stMoveCheckData.bWaitCheck)
    return;
  m_stMoveCheckData.bWaitCheck = false;

  if (m_stMoveCheckData.dwSkillID != skillid)
    return;

  xSceneEntryDynamic* pEntry = m_oRunner.getEntry();
  if (!pEntry)
    return;
  Scene* pScene = pEntry->getScene();
  if (pScene == nullptr)
    return;

  xPos destPos;
  destPos.set(pos.x(), pos.y(), pos.z());
  if (pScene->getValidPos(destPos) == false)
  {
    XERR << "[技能位置矫正], 位置非法, 玩家:" << pEntry->name << pEntry->id << "技能:" << skillid << "位置:" << destPos << XEND;
    return;
  }

  if (getXZDistance(pEntry->getPos(), destPos) > 1)
  {
    float clientdis = getXZDistance(m_stMoveCheckData.oOriPos, destPos);
    if (clientdis > m_stMoveCheckData.fMoveDistance + 1)
    {
      XERR << "[技能位置矫正], 客户端移动距离过远, 玩家:" << pEntry->name << pEntry->id << "技能:" << skillid << "客户端距离:" << clientdis << "技能距离:" << m_stMoveCheckData.fMoveDistance << XEND;
      return;
    }

    // 客户端比较近, 直接设置
    if (clientdis < getXZDistance(m_stMoveCheckData.oOriPos, pEntry->getPos()))
    {
      XDBG << "[技能-位置矫正], 客户端移动距离小于服务端, 重设玩家位置, 玩家:" << pEntry->name << pEntry->id << "技能:" << skillid << "技能前位置:" << m_stMoveCheckData.oOriPos << "服务端位置:" << pEntry->getPos() << "客户端位置:" << destPos << XEND;

      pEntry->setScenePos(destPos);
    }
    // 客户端较远, 检查寻路距离
    else
    {
      std::list<xPos> list;
      // 寻路失败
      if (pScene->findingPath(destPos, m_stMoveCheckData.oOriPos, list, TOOLMODE_PATHFIND_FOLLOW) == false)
      {
        XERR << "[技能位置矫正], 客户端坐标无法到达, 玩家:" << pEntry->name << pEntry->id << "技能:" << skillid << "位置:" << destPos << "玩家实际位置:" << pEntry->getPos() << XEND;
        return;
      }
      // 穿墙
      xPos prepos = list.front();
      float movedis = 0;
      for (auto &s : list)
      {
        movedis += getXZDistance(s, prepos);
        prepos = s;
      }
      if (movedis > clientdis + 3)
      {
        XERR << "[技能位置矫正], 客户端坐标不可直接到达, 玩家:" << pEntry->name << pEntry->id << "技能:" << skillid << "位置:" << destPos << "玩家实际位置:" << pEntry->getPos() << XEND;
        return;
      }

      XDBG << "[技能-位置矫正], 客户端移动距离大于服务端, 重设玩家位置, 玩家:" << pEntry->name << pEntry->id << "技能:" << skillid << "技能前位置:" << m_stMoveCheckData.oOriPos << "服务端位置:" << pEntry->getPos() << "客户端位置:" << destPos << XEND;
      pEntry->setScenePos(destPos);
    }
  }
}

void SkillProcessor::endSkill(DWORD skillid)
{
  if (m_oRunner.getCFG() && m_oRunner.getCFG()->getSkillID() == skillid)
    m_oRunner.setDelStatus();
  for (auto &v : m_listRunner)
  {
    if (v.getCFG() && v.getCFG()->getSkillID() == skillid)
      v.setDelStatus();
  }
}
