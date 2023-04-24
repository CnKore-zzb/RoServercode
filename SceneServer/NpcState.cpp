#include "NpcState.h"
#include "Scene.h"
#include "SceneNpc.h"
#include "FeatureConfig.h"
//const char LOG_NAME[] = "NpcState";

void NormalNpcState::execute(SceneNpc *npc)
{
  if (npc == nullptr || npc->getAttr(EATTRTYPE_NOACT) != 0 || npc->getAttr(EATTRTYPE_FREEZE) != 0)
    return;

  npc->m_ai.normal();
}

void AttackNpcState::execute(SceneNpc *npc)
{
  if (npc == nullptr || npc->getAttr(EATTRTYPE_NOACT) != 0 || npc->getAttr(EATTRTYPE_FREEZE) != 0)
    return;

  if (npc->m_ai.attack() == false && npc->m_ai.getStateType() == ENPCSTATE_ATTACK)
    npc->m_ai.changeState(ENPCSTATE_NORMAL);
}

void AttackNpcState::exit(SceneNpc *npc)
{
  if (npc != nullptr && npc->m_ai.getStateType() != ENPCSTATE_WAIT)
    npc->m_ai.setCurLockID(0);
}

void MoveToPosNpcState::execute(SceneNpc *npc)
{
}

void MoveToPosNpcState::exit(SceneNpc *npc)
{
}

void PickupNpcState::execute(SceneNpc *npc)
{
  if (npc == nullptr || npc->getAttr(EATTRTYPE_NOACT) != 0)
    return;

  if (npc->m_ai.pickup() == false)
    npc->m_ai.changeState(ENPCSTATE_NORMAL);
}

void PickupNpcState::exit(SceneNpc *npc)
{
  if (npc != nullptr)
    npc->m_ai.setCurItemID(0);
}

void WaitNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  if (npc->m_ai.wait() == false)
  {
    npc->m_ai.changeState(npc->m_ai.getFormerState());
    /*if (npc->m_ai.getCurLockID() != 0)
      npc->m_ai.changeState(NpcStateTypeAttack);
    else
      npc->m_ai.changeState(ENPCSTATE_NORMAL);
      */
  }
}
void WaitNpcState::exit(SceneNpc* npc)
{

}

void CameraNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  if (npc->m_ai.gocamera() == false)
    npc->m_ai.changeState(ENPCSTATE_NORMAL);
}

void CameraNpcState::exit(SceneNpc *npc)
{

}

void NaughtyNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  if (npc->m_ai.naughty() == false)
    npc->m_ai.changeState(ENPCSTATE_NORMAL);
}

void NaughtyNpcState::exit(SceneNpc* npc)
{

}

void SmileNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  if (npc->m_ai.smile() == false)
    npc->m_ai.changeState(ENPCSTATE_NORMAL);
}

void SmileNpcState::exit(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  npc->m_ai.setCurLockID(0);
}

void SeeDeadNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  if (npc->m_ai.seedead() == false)
    npc->m_ai.changeState(ENPCSTATE_NORMAL);
}

void SeeDeadNpcState::exit(SceneNpc* npc)
{
}

void SleepNpcState::enter(SceneNpc* npc)
{
  //set hp
  if (npc == nullptr)
    return;

  SQWORD maxHp = npc->getAttr(EATTRTYPE_MAXHP);
  npc->setAttr(EATTRTYPE_HP, maxHp);
  npc->m_oMove.stop();
}

void SleepNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  npc->m_ai.sleep();
}

void GoBackNpcState::enter(SceneNpc* npc)
{
  if (npc == nullptr || npc->getScene() == nullptr)
    return;

  // 避免返回时, 恰好被添加了状态类buff
  if (npc->m_ai.isMoveable() == false)
  {
    npc->m_ai.changeState(npc->m_ai.getFormerState());
    return;
  }

  const TVecDWORD& vecbuff = FeatureConfig::getMe().getNpcAICFG().vecGoBackBuff;
  for (auto &v : vecbuff)
  {
    npc->m_oBuff.add(v);
  }

  npc->m_oMove.stop();
  xPos pos = npc->getPos();
  if (npc->define.getTerritory())
  {
    npc->getScene()->getRandPos(npc->getBirthPos(), npc->define.getTerritory(), pos);
  }
  else
  {
    pos = npc->getBirthPos();
  }
  npc->m_ai.moveTo(pos);
}

void GoBackNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  npc->m_ai.goback();
}

void GoBackNpcState::exit(SceneNpc *npc)
{
  if (npc == nullptr)
    return;
  const TVecDWORD& vecbuff = FeatureConfig::getMe().getNpcAICFG().vecGoBackBuff;
  for (auto &v : vecbuff)
  {
    npc->m_oBuff.del(v);
  }
}

void RunAwayNpcState::enter(SceneNpc* npc)
{
  DWORD time = FeatureConfig::getMe().getNpcAICFG().stAIParams.dwRunAwayTime + now();
  npc->m_ai.setRunAwayStopTime(time);
}

void RunAwayNpcState::execute(SceneNpc* npc)
{
  if (npc == nullptr || npc->getScene() == nullptr)
    return;
  if (npc->m_ai.runaway() == false)
    npc->m_ai.changeState(ENPCSTATE_NORMAL);
}

