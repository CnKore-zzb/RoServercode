#include "SealRepair.h"
#include "SceneNpcManager.h"
#include "SceneActManager.h"
#include "SceneUserManager.h"
#include "SceneManager.h"
#include "SceneItemManager.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneAct.h"
#include "MsgManager.h"
#include "GMCommandRuler.h"
// seal
bool SRepairSeal::checkRepairOk()
{
  if (eStatus != ESealStatus_RepairOk)
    return false;
  if (!m_setCalledMonster.empty())
    return false;
  return true;
}
bool SRepairSeal::checkOk(DWORD curTime)
{
  if (dwFinishTime == 0 || curTime < dwFinishTime)
    return false;

  return true;
}

bool SRepairSeal::checkFailure(DWORD curTime)
{
  //if (curTime > dwSealStopTime)
    //return true;
  //if (pOwnUser != nullptr && getDistance(pOwnUser->getPos(), m_stCurItem.oPos) > 10)
    //return true;
  if (m_setActUserID.empty())
    return true;
  return false;
}

bool SRepairSeal::checkExitAction(DWORD exitTime)
{
  if (eStatus != ESealStatus_Finish)
    return false;
  if (exitTime == now() + MiscConfig::getMe().getSealCFG().dwPreActionTime)
  {
    SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(m_stCurItem.qwSealID);
    if (pSeal != nullptr)
    {
     UserActionNtf message;
     message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
     message.set_value(6001);
     message.set_charid(pSeal->id);
     PROTOBUF(message, gsend, glen);
     pSeal->sendCmdToNine(gsend, glen);
    }
    eStatus = ESealStatus_Exit;
    return true;
  }
  return false;
}

void SRepairSeal::clear()
{
  // clear monsters
  for (auto &q : m_setCalledMonster)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(q);
    if (npc == nullptr)
      continue;
    npc->setClearState();
  }
  // del sealNpc
  /*if (pOwnTeam != nullptr)
  {
    SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(m_stCurItem.qwSealID);
    if (pSeal != nullptr)
    {
      pSeal->setClearState();
    }
  }*/
  // del sealAct
  SceneActBase* pAct = SceneActManager::getMe().getSceneAct(qwActID);
  if (pAct != nullptr)
  {
    pAct->setClearState();
  }
}

void SRepairSeal::callMonster()
{
  DWORD lastValue = intCurSpeed > (int)dwCurValue ? 0 : dwCurValue - (DWORD)intCurSpeed;
  if (m_stCurItem.pCFG->haveMonsterInPoint(lastValue, dwCurValue) == false)
    return;

  if (m_setActUserID.empty())
    return;

  DWORD level = m_stCurItem.pCFG->dwSealLevel;
 // get monsters
  TVecSealMonster vecMonster;
  if (m_stCurItem.pCFG->getMonster(lastValue, dwCurValue, level, vecMonster) == false)
  {
    XERR << "[SealConfig], can't get right monster" << XEND;
    return;
  }
  if (vecMonster.empty())
    return;
  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if (pScene == nullptr)
    return;

  for (auto &stm : vecMonster)
  {
    // check limit max monster num
    //if (m_setCalledMonster.size() >= m_stCurItem.pCFG->dwMaxMonsterNum && stm.isspec == false)
      //continue;

    // call monster
    NpcDefine oDefine;
    oDefine.resetting();
    oDefine.setSearch(10);

    // set belong to
    //if (pOwnTeam != nullptr)
    //  oDefine.m_oVar.m_qwTeamID = pOwnTeam->getGUID();
    if (pOwnUser != nullptr)
    {
      oDefine.m_oVar.m_qwTeamUserID = pOwnUser->id;
      oDefine.m_oVar.m_qwQuestOwnerID = pOwnUser->id;
    }
    else
    {
      XERR << "不合理的封印修复" << XEND;
      return;
    }

    oDefine.setID(stm.dwID);
    oDefine.setLevel(level);

    xPos outPos;
    if (pScene->getRandPos(m_stCurItem.oPos, 5, outPos) == false)
      return;
    oDefine.setPos(outPos);
    oDefine.m_oVar.dwSealType = ESEALMONTYPE_PERSONAL;

    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, pScene);
    if (pNpc == nullptr)
      return;
    m_setCalledMonster.insert(pNpc->id);
  }
}

void SRepairSeal::preFinishSeal()
{
  if (dwFinishTime != 0)
    return;
  dwFinishTime = now() + MiscConfig::getMe().getSealCFG().dwDropDelay;
  if (m_setActUserID.empty())
    return;
  SceneUser* pOneUser = SceneUserManager::getMe().getUserByID(*(m_setActUserID.begin()));
  if (pOneUser == nullptr)
    return;
  // shake screen
  const string& gm = MiscConfig::getMe().getSealCFG().strGMShakeScreen;
  GMCommandRuler::getMe().execute(pOneUser, gm);
  // motion
  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  message.set_value(3001);
  message.set_charid(m_stCurItem.qwSealID);
  PROTOBUF(message, gsend, glen);
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;
  pSeal->setGearStatus(3002);
  for (auto &s : m_setActUserID)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(s);
    if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
    if (pSeal->check2PosInNine(pUser) == false)
      continue;
    pUser->sendCmdToMe(gsend, glen);
  }
}

void SRepairSeal::finishSeal()
{
    // give reward
  TVecItemInfo vecItemInfo;
  TVecItemInfo item;
  for (auto &v : m_stCurItem.pCFG->vecRewardIDs)
  {
    if (RewardManager::roll(v, nullptr, item, ESOURCE_SEAL))
    {
      for (auto &it : item)
      {
        vecItemInfo.push_back(it);
      }
    }
  }

  DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
  for (auto &q : m_setActUserID)
  {
    Cmd::AddMapItem reward;
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(q);
    if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
    for (size_t i = 0; i < vecItemInfo.size(); ++i)
    {
      xPos dest = m_stCurItem.oPos;
      xPos outpos;
      if (pUser->getScene()->getRandPos(dest, 5, outpos) == false)
        outpos = dest;
      SceneItem* pItem = SceneItemManager::getMe().createSceneItem(pUser->getScene(), vecItemInfo[i], outpos);
      if (pItem != nullptr)
      {
        pItem->addOwner(pUser->id);
        pItem->setViewLimit();
        pItem->fillMapItemData(reward.add_items(), extraTime * (i + 1));
      }
    }
    pUser->addBaseExp(m_stCurItem.pCFG->dwBaseExp, ESOURCE_SEAL);
    pUser->addJobExp(m_stCurItem.pCFG->dwJobExp, ESOURCE_SEAL);
    pUser->getAchieve().onRepairSeal();
    XLOG << "[个人封印], 奖励掉落, 玩家:" << pUser->name << ", " << pUser->id << "BaseExp:" << m_stCurItem.pCFG->dwBaseExp << XEND;
    XLOG << "[个人封印], 奖励掉落, 玩家:" << pUser->name << ", " << pUser->id << "JobExp:" << m_stCurItem.pCFG->dwJobExp << XEND;
    MsgManager::sendMsg(pUser->id, 17, MsgParams(m_stCurItem.pCFG->dwBaseExp, m_stCurItem.pCFG->dwJobExp));

    PROTOBUF(reward, send, len);
    pUser->sendCmdToMe(send, len);
  }

  eStatus = ESealStatus_Finish;
  // notify client
  /*SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;
  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  message.set_value(3001);
  message.set_charid(pSeal->id);
  PROTOBUF(message, gsend, glen);
  if (pOwnUser)
  {
    pOwnUser->sendCmdToMe(gsend, glen);
  }
  else if (pOwnTeam)
  {
    for (auto &t : pOwnTeam->getTeamMembers())
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(t.getGUID());
      if (pUser == nullptr || pSeal->check2PosInNine(pUser) == false)
        continue;
      pUser->sendCmdToMe(gsend, glen);
    }
  }
  pSeal->setGearStatus(3002);
  pSeal->setClearTime(now() + 1);

  clear();
  */
}

void SRepairSeal::failSeal()
{
  clear();
}

void SRepairSeal::process(DWORD curTime)
{
  if (/*pOwnTeam == nullptr && */pOwnUser == nullptr)
    return;

  if (curTime < dwNextProcessTime)
    return;
  dwNextProcessTime = curTime + 1;

  if (dwCurValue != m_stCurItem.pCFG->dwMaxValue)
  {
    int realSpeed = MiscConfig::getMe().getSealCFG().dwSpeed;

    int temp = dwCurValue;
    dwCurValue = temp + realSpeed < 0 ? 0 : temp + realSpeed;
    dwCurValue = dwCurValue > m_stCurItem.pCFG->dwMaxValue ? m_stCurItem.pCFG->dwMaxValue : dwCurValue ;

    if (realSpeed != intCurSpeed)
    {
      intCurSpeed = realSpeed;
      // send to client
      /*SealTimer cmd;
      cmd.set_curvalue(dwCurValue);
      cmd.set_maxvalue(m_stCurItem.pCFG->dwMaxValue);
      cmd.set_stoptime(dwSealStopTime);
      cmd.set_maxtime(m_stCurItem.pCFG->dwMaxTime);
      cmd.set_speed(intCurSpeed);
      PROTOBUF(cmd, send, len);
      if (pOwnTeam)
      {
        for (auto &t : pOwnTeam->getTeamMembers())
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(t.getGUID());
          if (pUser == nullptr)
            continue;
          pUser->sendCmdToMe(send, len);
        }
      }
      else if (pOwnUser)
      {
        pOwnUser->sendCmdToMe(send, len);
      }
      */
    }
    // call monster by process
    if (realSpeed > 0)
      callMonster();
  }
  else if (eStatus == ESealStatus_Process)
  {
    eStatus = ESealStatus_RepairOk;
    // 播放饱和状态
    SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(m_stCurItem.qwSealID);
    if (pSeal == nullptr)
      return;
    UserActionNtf message;
    message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
    message.set_value(5001);
    message.set_charid(pSeal->id);
    PROTOBUF(message, gsend, glen);
    pSeal->setGearStatus(5002);
    if (pOwnUser)
    {
      pOwnUser->sendCmdToMe(gsend, glen);
    }
    /*else if (pOwnTeam)
    {
      for (auto &t : pOwnTeam->getTeamMembers())
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(t.getGUID());
        if (pUser == nullptr || pSeal->check2PosInNine(pUser) == false)
          continue;
        pUser->sendCmdToMe(gsend, glen);
      }
    }*/
  }
  // refresh users in the ACT
  findActUser();
}

void SRepairSeal::findActUser()
{
  if (/*pOwnTeam == nullptr && */pOwnUser == nullptr)
    return;
  m_setActUserID.clear();
  if (pOwnUser != nullptr)
  {
    //if (getDistance(pOwnUser->getPos(), m_stCurItem.oPos) > 10)// || pOwnUser->isAlive() == false)
      //return;
    m_setActUserID.insert(pOwnUser->id);
    return;
  }

  /*if (pOwnTeam == nullptr)
    return;
  for (auto &t : pOwnTeam->getTeamMembers())
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(t.getGUID());
    if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
    if (pUser->getScene()->id != dwMapID)
      continue;
    //if (getDistance(pUser->getPos(), m_stCurItem.oPos) > 10 || pUser->isAlive() == false)
      //continue;
    m_setActUserID.insert(pUser->id);
  }*/
}

