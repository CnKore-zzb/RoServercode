#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneServer.h"
#include "SceneManager.h"
#include "DScene.h"
#include "SceneActManager.h"
#include "GMCommandRuler.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "MiscConfig.h"
#include "MsgManager.h"
#include "SceneItemManager.h"
#include "SessionCmd.pb.h"
#include "RedisManager.h"
#include "PlatLogManager.h"
#include "StatisticsDefine.h"
#include "SceneUser2.pb.h"
#include "TeamSealManager.h"
#include "ActivityEventManager.h"

/*
void SceneTeam::addMemberHandleSeal(QWORD userid)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;

  set<SceneUser*> otherSet;
  for (auto &v : m_vecMembers)
  {
    if (userid == v.getGUID())
      continue;
    SceneUser* pteamer = SceneUserManager::getMe().getUserByID(v.getGUID());
    if (pteamer != nullptr)
      otherSet.insert(pteamer);
  }
  if (otherSet.empty())
    return;

  // exechange map seal info with teamers
  for (auto &s : otherSet)
  {
    if (s == nullptr)
      continue;
    pUser->getSeal().addMySealToUser(s);
    s->getSeal().addMySealToUser(pUser);
  }
  // send team map sealing info to user
  const TVecTeamSealData& sealdata = m_oSeal.getSealData();
  for (auto &r : sealdata)
  {
    Scene* pScene = SceneManager::getMe().getSceneByID(r.dwMapID);
    if (pScene == nullptr)
      continue;
    pUser->getSeal().addUpdateNewSeal(pScene->getMapID(), r.m_stCurItem);
  }
  m_oSeal.sendSealInfo(pUser->id);

  // if current map is sealing
  DWORD mapid = pUser->getScene()->id;
  auto v = find_if(sealdata.begin(), sealdata.end(), [&mapid](const STeamSealData2& rData) ->bool {
      return mapid == rData.dwMapID;
      });
  if (v == sealdata.end())
    return;

  // check in nine and send team seal
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(v->m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;
  pSeal->sendMeToUser(pUser);

  // check in nine and send act
  SceneActBase* pAct = SceneActManager::getMe().getSceneAct(v->qwActID);
  if (pAct == nullptr)
    return;
  pAct->sendMeToUser(pUser);

  // check in nine and send monsters
  for (auto &q : v->m_setCalledMonster)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(q);
    if (pNpc == nullptr)
      continue;
    pNpc->sendMeToUser(pUser);
  }
}

void SceneTeam::delMemberHandleSeal(QWORD userid)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;

  set<SceneUser*> otherSet;
  for (auto &v : m_vecMembers)
  {
    if (userid == v.getGUID())
      continue;
    SceneUser* pteamer = SceneUserManager::getMe().getUserByID(v.getGUID());
    if (pteamer != nullptr)
      otherSet.insert(pteamer);
  }

  // del seal info from each other
  for (auto &s : otherSet)
  {
    if (s == nullptr)
      continue;
    pUser->getSeal().delMySealToUser(s);
    s->getSeal().delMySealToUser(pUser);
  }
  // del team map sealing info to user
  const TVecTeamSealData& sealdata = m_oSeal.getSealData();
  for (auto &r : sealdata)
  {
    Scene* pScene = SceneManager::getMe().getSceneByID(r.dwMapID);
    if (pScene == nullptr)
      continue;
    pUser->getSeal().addUpdateDelSeal(pScene->getMapID(), r.m_stCurItem);
  }

  // if is sealing
  DWORD mapid = pUser->getScene()->id;
  auto v = find_if(sealdata.begin(), sealdata.end(), [&mapid](const STeamSealData2& rData) ->bool {
      return mapid == rData.dwMapID;
      });
  if (v == sealdata.end())
    return;

  // check in nine and del team seal
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(v->m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;
  pSeal->delMeToUser(pUser);

  // for client to del the process timer
  EndSeal cmd;
  cmd.set_success(false);
  if (v->m_stCurItem.pCFG)
    cmd.set_sealid(v->m_stCurItem.pCFG->dwID);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  // check in nine and del act
  SceneActBase* pAct = SceneActManager::getMe().getSceneAct(v->qwActID);
  if (pAct == nullptr)
    return;
  pAct->delMeToUser(pUser);

  // check in nine and del monsters
  for (auto &q : v->m_setCalledMonster)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(q);
    if (pNpc == nullptr)
      continue;
    pNpc->delMeToUser(pUser);
  }
}
*/

// seal
// ----------------------------------------------------------------------------------------------------------------
bool STeamSealData::checkRepairOk()
{
  if (!bFinished)
    return false;
  if (m_setCalledMonster.empty() == false)
    return false;
  return true;
}

bool STeamSealData::checkOk(DWORD curTime)
{
  if (dwFinishTime == 0 || curTime < dwFinishTime)
    return false;

  return true;
}

bool STeamSealData::checkFailure(DWORD curTime)
{
  if (m_setActUserID.empty())
    return true;
  return false;
}

void STeamSealData::clear()
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
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(m_stCurItem.qwSealID);
  if (pSeal != nullptr)
  {
    //pSeal->setClearState();
    pSeal->setClearState();
  }
  // del sealAct
  SceneActBase* pAct = SceneActManager::getMe().getSceneAct(qwActID);
  if (pAct != nullptr)
  {
    pAct->setClearState();
  }
}

// -------------------------------------------------------------------------------------------------------------
SceneTeamSeal::SceneTeamSeal(QWORD teamid) : m_qwTeamID(teamid)
{

}

SceneTeamSeal::~SceneTeamSeal()
{
  for (auto &st : m_vecTeamSealData)
  {
    if (st.m_stCurItem.bSealing)
    {
      failSeal(st.dwMapID);
    }
    st.clear();
  }
}

void SceneTeamSeal::addMember(QWORD userid)
{
  m_setTeamers.insert(userid);

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;
  // send team map sealing info to user
  for (auto &r : m_vecTeamSealData)
  {
    Scene* pScene = SceneManager::getMe().getSceneByID(r.dwMapID);
    if (pScene == nullptr)
      continue;
    pUser->getSeal().addUpdateNewSeal(pScene->getMapID(), r.m_stCurItem);
  }
  sendSealInfo(pUser->id);

  // if current map is sealing
  DWORD mapid = pUser->getScene()->id;
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [&mapid](const STeamSealData& rData) ->bool {
      return mapid == rData.dwMapID;
      });
  if (v == m_vecTeamSealData.end())
    return;

  // check in nine and send team seal
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(v->m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;
  pSeal->sendMeToUser(pUser);

  // check in nine and send act
  SceneActBase* pAct = SceneActManager::getMe().getSceneAct(v->qwActID);
  if (pAct == nullptr)
    return;
  pAct->sendMeToUser(pUser);

  // check in nine and send monsters
  for (auto &q : v->m_setCalledMonster)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(q);
    if (pNpc == nullptr)
      continue;
    pNpc->sendMeToUser(pUser);
  }
}

void SceneTeamSeal::removeMember(QWORD userid)
{
  m_setTeamers.erase(userid);
  if (m_setTeamers.empty())
  {
    m_dwDestructTime = now();
  }

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;

  // del team map sealing info to user
  for (auto &r : m_vecTeamSealData)
  {
    Scene* pScene = SceneManager::getMe().getSceneByID(r.dwMapID);
    if (pScene == nullptr)
      continue;
    pUser->getSeal().addUpdateDelSeal(pScene->getMapID(), r.m_stCurItem);
  }

  // if is sealing
  DWORD mapid = pUser->getScene()->id;
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [&mapid](const STeamSealData& rData) ->bool {
      return mapid == rData.dwMapID;
      });
  if (v == m_vecTeamSealData.end())
    return;

  // check in nine and del team seal
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(v->m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;
  pSeal->delMeToUser(pUser);

  // for client to del the process timer
  EndSeal cmd;
  cmd.set_success(false);
  if (v->m_stCurItem.pCFG)
    cmd.set_sealid(v->m_stCurItem.pCFG->dwID);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  // remove act user
  v->m_setActUserID.erase(pUser->id);

  // check in nine and del act
  SceneActBase* pAct = SceneActManager::getMe().getSceneAct(v->qwActID);
  if (pAct == nullptr)
    return;
  pAct->delMeToUser(pUser);

  // check in nine and del monsters
  for (auto &q : v->m_setCalledMonster)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(q);
    if (pNpc == nullptr)
      continue;
    pNpc->delMeToUser(pUser);
  }
}

bool SceneTeamSeal::isSealing(DWORD mapid)
{
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [&mapid](const STeamSealData& r) -> bool{
      return r.dwMapID == mapid && r.m_stCurItem.bSealing;
      });
  return v != m_vecTeamSealData.end();
}

SceneNpc* SceneTeamSeal::createSeal(Scene* pScene, const SSealItem& sItem)
{
  if (pScene == nullptr)
    return nullptr;

  NpcDefine oDefine;
  oDefine.setID(MiscConfig::getMe().getSealCFG().dwSealNpcID);
  oDefine.resetting();
  oDefine.setPos(sItem.oPos);
  oDefine.m_oVar.m_qwTeamID = m_qwTeamID;

  // 设置不可点选
  if (pScene->isDScene())
    oDefine.setBehaviours(oDefine.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);

  SceneNpc* pSeal = SceneNpcManager::getMe().createNpc(oDefine, pScene);
  if (pSeal == nullptr)
  {
    XERR << "[组队封印], 创建失败, SealConfig:" << sItem.pCFG->dwID << "TeamID:" << m_qwTeamID << XEND;
    return nullptr;
  }
  XLOG << "[组队封印], 创建封印npc" << pSeal->id << "SealConfig:" << sItem.pCFG->dwID << "TeamID:" << m_qwTeamID << XEND;
  return pSeal;
}

void SceneTeamSeal::openSeal(DWORD dwID, Scene* pRaidScene, xPos pos)
{
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwID);
  if (pCFG == nullptr)
    return;
  if (pCFG->eType != ESEALTYPE_NORMAL)
    return;

  DWORD dwMapID = pCFG->dwMapID;
  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if (pScene == nullptr && pRaidScene == nullptr)
    return;

  Scene* nowScene = pRaidScene != nullptr ? pRaidScene : pScene;
  dwMapID = nowScene->id;

  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [dwMapID](const STeamSealData& r) ->bool{
      return r.dwMapID == dwMapID;
      });
  if (v != m_vecTeamSealData.end())
    return;

  set<SceneUser*> onSceneUsers;
  for (auto &q : m_setTeamers)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(q);
    if (pUser == nullptr || pUser->getScene() != nowScene)
      continue;
    onSceneUsers.insert(pUser);
  }

  xPos outPos;
  if (pos.empty())
  {
    //get random pos
    const TVecSealPos& vecPos = pCFG->vecRefreshPos;
    DWORD size = vecPos.size();
    if (size == 0)
    {
      XERR << "[Seal], 封印配置错误，找不到坐标点，id = " << pCFG->dwID << XEND;
      return;
    }
    DWORD randIndex = randBetween(1, size) - 1;
    xPos pos = vecPos[randIndex].oPos;
    float r = vecPos[randIndex].dwRange;
    if (r == 0 && nowScene->getValidPos(pos, outPos) == false)
    {
      XERR << "[Seal], 召唤封印, 找不到合适的坐标点, id = " << pCFG->dwID << XEND;
      return;
    }
    else if(nowScene->getRandPos(pos, r, outPos) == false)
    {
      XERR << "[Seal], 召唤封印, 找不到合适的坐标点, id = " << pCFG->dwID << XEND;
      return;
    }
    // send to session for save pos
    SetTeamSeal poscmd;
    poscmd.set_teamid(m_qwTeamID);
    poscmd.set_sealid(pCFG->dwID);
    poscmd.set_mapid(pCFG->dwMapID);
    poscmd.set_estatus(ESETSEALSTATUS_SETPOS);
    poscmd.mutable_pos()->set_x(outPos.getX());
    poscmd.mutable_pos()->set_y(outPos.getY());
    poscmd.mutable_pos()->set_z(outPos.getZ());
    PROTOBUF(poscmd, possend, poslen);
    thisServer->sendCmdToSession(possend, poslen);
  }
  else
  {
    outPos = pos;
  }

  // notify teamers seal accept
  if (pRaidScene == nullptr)
  {
    SealAcceptCmd cmdacc;
    cmdacc.set_seal(dwID);
    cmdacc.mutable_pos()->set_x(outPos.getX());
    cmdacc.mutable_pos()->set_y(outPos.getY());
    cmdacc.mutable_pos()->set_z(outPos.getZ());
    PROTOBUF(cmdacc, accsend, acclen);
    for (auto &q : m_setTeamers)
    {
      thisServer->sendCmdToMe(q, accsend, acclen);
    }
  }

  STeamSealData stData;
  stData.dwMapID = dwMapID;
  stData.m_stCurItem.oPos = outPos;
  stData.m_stCurItem.pCFG = pCFG;
  stData.m_stCurItem.bOwnSeal = true;
  if (!onSceneUsers.empty())
  {
    SceneNpc* pSeal = createSeal(nowScene, stData.m_stCurItem);
    if (pSeal == nullptr)
      return;
    stData.m_stCurItem.qwSealID = pSeal->id;
  }
  m_vecTeamSealData.push_back(stData);
  m_dwDestructTime = 0;

  for (auto &s : onSceneUsers)
  {
    if (s)
      s->getSeal().addUpdateNewSeal(nowScene->getMapID(), stData.m_stCurItem);
  }
}

bool SceneTeamSeal::beginSeal(SceneUser* pUser, QWORD sealid, EFinishType etype)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [sealid](const STeamSealData& r) -> bool {
      return sealid == r.m_stCurItem.qwSealID;
      });
  if (v == m_vecTeamSealData.end())
    return false;
  return beginSeal(pUser, etype);
}

bool SceneTeamSeal::beginSeal(SceneUser* pUser, EFinishType etype)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;
  DWORD mapid = pUser->getScene()->id;
  auto v = find_if (m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [mapid](const STeamSealData& r) -> bool{
      return r.dwMapID == mapid;
      });

  if (v == m_vecTeamSealData.end() || v->m_stCurItem.bSealing || v->m_stCurItem.qwSealID == 0)
    return false;
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(v->m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return false;

  DWORD r = MiscConfig::getMe().getSealCFG().dwSealRange;
  if (getDistance(pUser->getPos(), v->m_stCurItem.oPos) > r)
  {
    XERR << "[SealTeam], 距离封印过远, 不可修复, user:" << pUser->id << "map:" << mapid << XEND;
    return false;
  }
  XLOG << "[SealTeam], 开始修复组队封印, team:" << m_qwTeamID << "user:" << pUser->id << "map:" << mapid << XEND;

  DWORD dwQucikFinishBuff = MiscConfig::getMe().getSealCFG().dwQuickFinishBuff;
  if(etype == EFINISHTYPE_QUICK && pUser->m_oBuff.haveBuff(dwQucikFinishBuff) == false)
  {
    BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr)
    {
      XERR << "[Seal], 开始修复组队封印, 玩家id:" << pUser->id << ", 地图:" << mapid << ", time:" << now() << "快速完成封印失败,未找到" << EPACKTYPE_MAIN << XEND;
      return false;
    }

    ItemInfo oItem;
    const SealMiscCFG& rCFG = MiscConfig::getMe().getSealCFG();
    for (auto &v : rCFG.vecQuickFinishItems)
    {
      if (v.second == 0 || v.second == mapid)
      {
        if (pMainPack->checkItemCount(v.first) == true)
        {
          oItem.set_id(v.first);
          oItem.set_count(1);
          break;
        }
      }
    }

    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(oItem.id());
    if (pCFG == nullptr)
    {
      XERR << "[SealTeam], 开始修复组队封印, 玩家id:" << pUser->id << ", 地图:" << mapid << ", time:" << now() << "快速完成封印失败,itemid :" << oItem.id() << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }
    if (oItem.id() == 0)
    {
      MsgManager::sendMsg(pUser->id, 3406, MsgParams(pCFG->strNameZh));
      return false;
    }

    pMainPack->reduceItem(oItem.id(), ESOURCE_USEITEM);
    pUser->getPackage().timer(now());
    pUser->m_oBuff.add(rCFG.dwQuickFinishBuff, pUser);
    XLOG << "[SealTeam], 快速完成组队封印, 扣除道具成功:" << m_qwTeamID << "user:" << pUser->id << "map:" << mapid << "dwCurValue" << v->dwCurValue
      << v->qwActID << "itemid: " << oItem.id() << now() << XEND;
  }

  bool bRealBegin = pUser->getScene()->isDScene();
  if (!bRealBegin)
  {
    pUser->getScene()->m_oImages.add(v->m_stCurItem.pCFG->dwDMapID, pUser, pSeal->getPos(), r, v->m_stCurItem.pCFG->dwID);
  }

  if (bRealBegin)
  {
    m_dwDMapID = pUser->getScene()->id;
    m_bSealing = true;

    SceneActBase* pActBase = SceneActManager::getMe().createSceneAct(pUser->getScene(), v->m_stCurItem.oPos, r, m_qwTeamID, EACTTYPE_SEAL);
    if (pActBase == nullptr)
      return false;
    if (!pActBase->enterScene(pUser->getScene()))
    {
      SceneActManager::getMe().delSceneAct(pActBase);
      return false;
    }
    v->qwActID = pActBase->id;
    v->dwSealStopTime = now() + v->m_stCurItem.pCFG->dwMaxTime;
    if(v->dwWaitTime == 0)
      v->dwWaitTime = now() + MiscConfig::getMe().getSealCFG().dwWaitTime;
    if(v->dwWaitTime > now())
      MsgManager::sendMsg(pUser->id, 25445, MsgParams(v->dwWaitTime - now()));
  }
  v->m_stCurItem.bSealing = true;
  v->m_setActUserID.insert(pUser->id);

  //send to current map team users to update mapInfo / change seal gear status
  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  message.set_value(2001);
  message.set_charid(v->m_stCurItem.qwSealID);
  PROTOBUF(message, gsend, glen);
  pSeal->setGearStatus(2002);

  for (auto &t : m_setTeamers)
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(t);
    if (user == nullptr || user->getScene() != pUser->getScene())
      continue;

    user->getSeal().addUpdateNewSeal(pUser->getScene()->getMapID(), v->m_stCurItem);

    user->sendCmdToMe(gsend, glen);
  }
  if (bRealBegin)
  {
    //send to client
    BeginSeal cmd;
    cmd.set_sealid(pSeal->id);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);

    MsgParams params;
    params.addString(pUser->name);
    // send to other team members
    for (auto &t : m_setTeamers)
    {
      sendSealInfo(t);
      if (t == pUser->id)
        continue;
      MsgManager::sendMsg(t, 1601, params);
    }

    // send to session begin status
    SetTeamSeal cmd3;
    cmd3.set_teamid(m_qwTeamID);
    cmd3.set_sealid(v->m_stCurItem.pCFG->dwID);
    cmd3.set_estatus(ESETSEALSTATUS_BEGIN);
    cmd3.set_mapid(pUser->getScene()->id);
    PROTOBUF(cmd3, send3, len3);
    thisServer->sendCmdToSession(send3, len3);
  }
  return true;
}

void SceneTeamSeal::process(STeamSealData& sData, DWORD curTime)
{
  if (curTime < sData.dwNextProcessTime || curTime < sData.dwWaitTime)
    return;

  sData.dwNextProcessTime = curTime + 1;

  if (sData.dwCurValue != sData.m_stCurItem.pCFG->dwMaxValue)
  {
    bool bQuick = false;
    DWORD dwQucikFinishBuff = MiscConfig::getMe().getSealCFG().dwQuickFinishBuff;
    for (auto &t : m_setTeamers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
      if (pUser == nullptr || pUser->getScene() == nullptr)
        continue;
      if(pUser->m_oBuff.haveBuff(dwQucikFinishBuff) == true)
      {
        sData.dwCurValue = sData.m_stCurItem.pCFG->dwMaxValue;
        bQuick = true;
        break;
      }
    }
    if(bQuick == false)
    {
      DWORD realSpeed = MiscConfig::getMe().getSealCFG().dwSpeed;
      sData.dwCurValue += realSpeed;
      sData.dwCurValue = sData.dwCurValue > sData.m_stCurItem.pCFG->dwMaxValue ? sData.m_stCurItem.pCFG->dwMaxValue : sData.dwCurValue ;
      callMonster(sData);
    }
  }
  else if (sData.bFinished == false)
  {
    sData.bFinished = true;

    UserActionNtf message;
    message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
    message.set_value(5001);
    message.set_charid(sData.m_stCurItem.qwSealID);
    PROTOBUF(message, gsend, glen);
    SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(sData.m_stCurItem.qwSealID);
    if (pSeal == nullptr)
      return;
    pSeal->setGearStatus(5002);
    for (auto &t : m_setTeamers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
      if (pUser == nullptr || pUser->getScene() == nullptr)
        continue;
      if (pSeal->check2PosInNine(pUser) == false)
        continue;
      pUser->sendCmdToMe(gsend, glen);
    }
  }
}

void SceneTeamSeal::callMonster(STeamSealData& sData)
{
  DWORD realSpeed = MiscConfig::getMe().getSealCFG().dwSpeed;
  DWORD lastValue = realSpeed > sData.dwCurValue ? 0 : sData.dwCurValue - realSpeed;
  if (sData.m_stCurItem.pCFG->haveMonsterInPoint(lastValue, sData.dwCurValue) == false)
    return;

  if (sData.m_setActUserID.empty())
    return;
  // get monster refrence level
  DWORD level = sData.m_stCurItem.pCFG->dwSealLevel;
  // get monsters
  TVecSealMonster vecMonster;
  if (sData.m_stCurItem.pCFG->getMonster(lastValue, sData.dwCurValue, level, vecMonster) == false)
  {
    XERR << "[SealConfig],无法按配置点找出怪物, config:" << sData.m_stCurItem.pCFG->dwID << XEND;
    return;
  }
  if (vecMonster.empty())
    return;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(*(sData.m_setActUserID.begin()));
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;
  Scene* pScene = pUser->getScene();

  for (auto &stm : vecMonster)
  {
    // check limit max monster num
    if (sData.m_setCalledMonster.size() >= sData.m_stCurItem.pCFG->dwMaxMonsterNum && stm.isspec == false)
      continue;

    // call monster
    NpcDefine oDefine;
    oDefine.resetting();
    oDefine.setSearch(10);
    oDefine.setID(stm.dwID);
    oDefine.setLevel(level);
    oDefine.m_oVar.m_qwTeamID = m_qwTeamID;

    xPos outPos;
    if (pScene->getRandPos(sData.m_stCurItem.oPos, 5, outPos) == false)
    {
      XERR << "[SealTeam], 组队封印召唤怪物, 找不到合适的坐标点召唤怪物" << XEND;
      return;
    }
    oDefine.setPos(outPos);

    oDefine.m_oVar.dwSealType = ESEALMONTYPE_TEAM;
    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, pScene);
    if (pNpc == nullptr)
      return;
    sData.m_setCalledMonster.insert(pNpc->id);
  }
  XLOG << "[SealTeam], 组队封印召唤怪物, team:" << m_qwTeamID << "怪物数量:" << vecMonster.size() << XEND;
}

void SceneTeamSeal::onMonsterDie(SceneNpc* npc)
{
  if (npc == nullptr || npc->getScene() == nullptr)
    return;
  if (npc->define.m_oVar.m_qwTeamID != m_qwTeamID)
    return;
  DWORD mapid = npc->getScene()->id;
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [&mapid](const STeamSealData& rData) ->bool {
      return rData.dwMapID == mapid;
      });
  if (v == m_vecTeamSealData.end())
    return;
  v->m_setCalledMonster.erase(npc->id);
}

void SceneTeamSeal::preFinishSeal(STeamSealData& sData)
{
  if (sData.dwFinishTime != 0)
    return;
  sData.dwFinishTime = now() + MiscConfig::getMe().getSealCFG().dwDropDelay;

  if (sData.m_setActUserID.empty())
    return;
  SceneUser* pOneUser = SceneUserManager::getMe().getUserByID(*(sData.m_setActUserID.begin()));
  if (pOneUser == nullptr)
    return;

  // shake screen
  const string& gm = MiscConfig::getMe().getSealCFG().strGMShakeScreen;
  GMCommandRuler::getMe().execute(pOneUser, gm);

  // motion
  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  message.set_value(3001);
  message.set_charid(sData.m_stCurItem.qwSealID);
  PROTOBUF(message, gsend, glen);
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(sData.m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;
  pSeal->setGearStatus(3002);
  for (auto &t : m_setTeamers)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
    if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
    if (pSeal->check2PosInNine(pUser) == false)
      continue;
    pUser->sendCmdToMe(gsend, glen);
  }
}

void SceneTeamSeal::finishSeal(DWORD mapid)
{
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [&mapid](const STeamSealData& r) -> bool{
      return r.dwMapID == mapid;
      });
  if (v == m_vecTeamSealData.end() || v->m_stCurItem.pCFG == nullptr)
    return;

  const SealMiscCFG& rSealCFG = MiscConfig::getMe().getSealCFG();
  DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
  Cmd::AddMapItem reward;
  set<SceneUser*> validUserSet;
  set<SceneUser*> invalidUserSet;
  set<SceneUser*> allUser;

  DWORD maxtimes = rSealCFG.dwMaxDaySealNum + ActivityEventManager::getMe().getExtraTimes(EAEREWARDMODE_SEAL);
  // get valid users
  for (auto &q : v->m_setActUserID)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(q);
    if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
    DWORD sealednum = pUser->getVar().getVarValue(EVARTYPE_SEAL);

    // 完成裂隙获得信用度
    const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
    if (sealednum < rCFG.dwSealTimes)
      pUser->getUserSceneData().addCredit(rCFG.dwSealValue);

    pUser->getVar().setVarValue(EVARTYPE_SEAL, sealednum + 1);
    pUser->getAchieve().onRepairSeal();
    pUser->getTutorTask().onRepairSeal();
    pUser->getGuildChallenge().onRepairSeal();
    pUser->getServant().onFinishEvent(ETRIGGER_REPAIR_SEAL);

    if (sealednum < maxtimes)
    {
      pUser->getExtraReward(EEXTRAREWARD_SEAL);
      pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_SEAL, sealednum + 1);

      // 额外奖励, 裂隙无翻倍奖
      DWORD times = 1;
      TVecItemInfo extrareward;
      if (ActivityEventManager::getMe().getReward(pUser, EAEREWARDMODE_SEAL, sealednum + 1, extrareward, times) && extrareward.empty() == false)
      {
        XLOG << "[SealTeam]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "活动额外奖励:";
        for (auto& v : extrareward)
          XLOG << v.id() << v.count();
        XLOG << XEND;
        pUser->getPackage().addItem(extrareward, EPACKMETHOD_AVAILABLE, false, true, false);
      }
    }

    allUser.insert(pUser);
    if (sealednum >= maxtimes)
    {
      if (sealednum + 1 == maxtimes && pUser->getTeamLeaderID() == pUser->id)
      {
        // send teamserver to change team goal
        LeaderSealFinishSocialCmd teamcmd;
        teamcmd.set_teamid(m_qwTeamID);
        PROTOBUF(teamcmd, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      invalidUserSet.insert(pUser);
      MsgManager::sendMsg(pUser->id, 1621);
      continue;
    }
    MsgManager::sendMsg(pUser->id, 1620, MsgParams(sealednum + 1, maxtimes));

    DWORD buffID = MiscConfig::getMe().getSealCFG().dwQuickFinishBuff;
    if(pUser->m_oBuff.haveBuff(buffID) == true)
    {
      pUser->m_oBuff.del(buffID);
      XLOG << "[SealTeam], 快速完成组队封印，delete:" << m_qwTeamID << "队员:" << pUser->id << pUser->name << buffID << XEND;
    }

    validUserSet.insert(pUser);
    XLOG << "[SealTeam], 组队封印修复完成，team:" << m_qwTeamID << "有效参与队员:" << pUser->id << XEND;

    //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Complete_Seal;
    PlatLogManager::getMe().eventLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eid,
      pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().CompleteLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eType,
      eid,
      ECompleteType_Seal,
      mapid,
      pUser->getVar().getVarValue(EVARTYPE_SEAL),
      0, 0,
      pUser->getLevel());
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_SEAL_COUNT, 0, 0, pUser->getLevel(), (DWORD)1);
  }

  // give reward
  TVecItemInfo vecItemInfo;
  TVecItemInfo item;
  DWORD totalCount = validUserSet.size();  //有效参与人数
  for (auto &d : v->m_stCurItem.pCFG->vecRewardIDs)
  {
    float fRatio = LuaManager::getMe().call<float>("calcSealRewardRatio", totalCount);
    XLOG << "[SealTeam-reward概率] 获取额外掉率" << fRatio << XEND;
    if (RewardManager::roll(d, nullptr, item, ESOURCE_SEAL, fRatio))
    {
      for (auto &it : item)
        vecItemInfo.push_back(it);
    }
  }

  Scene* pScene = SceneManager::getMe().getSceneByID(v->dwMapID);
  if (!validUserSet.empty() && pScene)
  {
    for (size_t i = 0; i < vecItemInfo.size(); ++i)
    {
      xPos dest = v->m_stCurItem.oPos;
      xPos outPos;
      if (pScene->getRandPos(dest, MiscConfig::getMe().getSealCFG().dwRewardRange, outPos) == false)
        outPos = dest;
      SceneItem* pItem = SceneItemManager::getMe().createSceneItem(pScene, vecItemInfo[i], outPos);
      if (pItem != nullptr)
      {
        for (auto &user : validUserSet)
          pItem->addOwner(user->id);
        pItem->setViewLimit();
        pItem->setOwnTime(rSealCFG.dwRewardDispTime);
        pItem->setDispTime(rSealCFG.dwRewardDispTime);
        pItem->fillMapItemData(reward.add_items(), extraTime * (i + 1));
      }
    }
    for (auto &user : validUserSet)
    {
      float ratio = user->m_oBuff.getExtraExpRatio(EEXTRAREWARD_SEAL);
      if (ratio)
        XLOG << "[回归-Buff], 裂隙, 额外经验倍率:" << ratio << "玩家:" << user->name << user->id << XEND;
      ratio += 1;

      user->addBaseExp(v->m_stCurItem.pCFG->dwBaseExp * ratio, ESOURCE_SEAL);
      user->addJobExp(v->m_stCurItem.pCFG->dwJobExp * ratio, ESOURCE_SEAL);

      XLOG << "[SealTeam], 封印奖励, 玩家:" << user->name << user->id <<  "BaseExp=" << v->m_stCurItem.pCFG->dwBaseExp << XEND;
      XLOG << "[SealTeam], 封印奖励, 玩家:" << user->name << user->id << "JobExp=" << v->m_stCurItem.pCFG->dwJobExp << XEND;
      MsgManager::sendMsg(user->id, 17, MsgParams(v->m_stCurItem.pCFG->dwBaseExp, v->m_stCurItem.pCFG->dwJobExp));

      user->m_oBuff.onFinishEvent(EEXTRAREWARD_SEAL);
    }
    XLOG << "[SealTeam], 封印奖励掉落,team:" << m_qwTeamID << "封印config:" << v->m_stCurItem.pCFG->dwID << XEND;

  }

  {
    PROTOBUF(reward, send, len);
    for (auto &user : validUserSet)
    {
      if (user == nullptr)
        continue;
      user->sendCmdToMe(send, len);
    }
  }

  // give help friend reward
  for (auto &user : invalidUserSet)
  {
    user->getHelpReward(validUserSet, EHELPTYPE_SEAL);
  }
  for (auto &user : validUserSet)
  {
    user->getHelpReward(allUser, EHELPTYPE_SEAL, true);
  }

  EndSeal cmd;
  cmd.set_sealid(v->m_stCurItem.pCFG->dwID);
  cmd.set_success(true);
  PROTOBUF(cmd, send, len);
  for (auto &t : m_setTeamers)
  {
    thisServer->sendCmdToMe(t, send, len);

    SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
    if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
    Scene* dScene = SceneManager::getMe().getSceneByID(mapid);
    if (pUser->getScene()->getMapID() != dScene->getMapID())
      continue;
    pUser->getSeal().addUpdateDelSeal(pUser->getScene()->getMapID(), v->m_stCurItem);
  }
  // notify session
  SetTeamSeal cmd2;
  cmd2.set_teamid(m_qwTeamID);
  cmd2.set_sealid(v->m_stCurItem.pCFG->dwID);
  cmd2.set_estatus(ESETSEALSTATUS_FINISH);
  cmd2.set_mapid(v->m_stCurItem.pCFG->dwMapID);
  PROTOBUF(cmd2, send2, len2);
  thisServer->sendCmdToSession(send2, len2);

  if (pScene && pScene->isDScene())
  {
    delSceneImage(pScene);
    DScene* pDScene = dynamic_cast<DScene*>(pScene);
    if (pDScene != nullptr)
    {
      m_dwExitDMapTime = now() + pDScene->getRaidEndTime();
      m_dwDMapID = mapid;
      m_dwCountDownTime = now() + pDScene->getRaidEndTime() - MiscConfig::getMe().getSealCFG().dwCountDownTime;
    }
  }
  m_vecFinishSeal.push_back(*v);
  m_vecTeamSealData.erase(v);
}

void SceneTeamSeal::failSeal(DWORD mapid)
{
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [&mapid](const STeamSealData& r) -> bool{
      return r.dwMapID == mapid;
      });
  if (v == m_vecTeamSealData.end())
    return;

  XLOG << "[SealTeam], 组队封印修复失败,team:" << m_qwTeamID << "封印config:" << v->m_stCurItem.pCFG->dwID << XEND;

  // notify client
  EndSeal cmd;
  cmd.set_success(false);
  PROTOBUF(cmd, send, len);
  for (auto &t : m_setTeamers)
  {
    thisServer->sendCmdToMe(t, send, len);

    SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
    if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
    Scene* pScene = SceneManager::getMe().getSceneByID(mapid);
    if (pScene == nullptr)
      continue;
    if (pUser->getScene()->getMapID() != pScene->getMapID())
      continue;
    pUser->getSeal().addUpdateDelSeal(pUser->getScene()->getMapID(), v->m_stCurItem);
  }
  delSceneImage(SceneManager::getMe().getSceneByID(mapid));

  // notify session
  SetTeamSeal cmd2;
  cmd2.set_teamid(m_qwTeamID);
  cmd2.set_sealid(v->m_stCurItem.pCFG->dwID);
  cmd2.set_mapid(v->m_stCurItem.pCFG->dwMapID);
  cmd2.set_estatus(ESETSEALSTATUS_FAIL);
  PROTOBUF(cmd2, send2, len2);
  thisServer->sendCmdToSession(send2, len2);

  // del seal
  v->clear();
  m_vecTeamSealData.erase(v);
}

void SceneTeamSeal::timer(DWORD curTime)
{
  if (m_vecTeamSealData.empty())
  {
    m_dwDestructTime = now() + 60;
  }

  if (!m_bSealing)
    return;
  if (curTime + MiscConfig::getMe().getSealCFG().dwPreActionTime == m_dwExitDMapTime)
  {
    for (auto &v : m_vecFinishSeal)
    {
      SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(v.m_stCurItem.qwSealID);
      if (pSeal != nullptr)
      {
        UserActionNtf message;
        message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
        message.set_value(6001);
        message.set_charid(v.m_stCurItem.qwSealID);
        PROTOBUF(message, gsend, glen);
        pSeal->sendCmdToNine(gsend, glen);
      }
    }
    m_vecFinishSeal.clear();
  }

  if (curTime == m_dwCountDownTime)
  {
    m_dwCountDownTime = 0;
    for (auto &t : m_setTeamers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
      if (pUser == nullptr || pUser->getScene() == nullptr || pUser->getScene()->id != m_dwDMapID)
        continue;

      MsgManager::sendMsg(pUser->id, 1619, MsgParams(MiscConfig::getMe().getSealCFG().dwCountDownTime), EMESSAGETYPE_TIME_DOWN);
    }
  }

  if (m_dwExitDMapTime != 0 && curTime >= m_dwExitDMapTime)
  {
    for (auto &t : m_setTeamers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
      if (pUser == nullptr || pUser->getScene() == nullptr || pUser->getScene()->id != m_dwDMapID)
        continue;

      if (pUser->isAlive() == false)
      {
        pUser->relive(ERELIVETYPE_RETURN);
        continue;
      }
      pUser->gomap(pUser->getScene()->getMapID(), GoMapType::Image, pUser->getPos());
    }
    m_dwExitDMapTime = 0;
    m_dwDMapID = 0;
  }

  if (m_vecTeamSealData.empty())
    return;

  TVecDWORD m_vecTempOkMap;
  TVecDWORD m_vecTempFailMap;
  for (auto &sData : m_vecTeamSealData)
  {
    if (sData.m_stCurItem.bSealing == false)
      continue;
    Scene* pScene = SceneManager::getMe().getSceneByID(sData.dwMapID);
    if (pScene == nullptr || pScene->isDScene() == false)
      continue;

    if (sData.checkFailure(curTime) == true)
    {
      m_vecTempFailMap.push_back(sData.dwMapID);
      continue;
    }

    process(sData, curTime);

    if (sData.checkRepairOk())
    {
      preFinishSeal(sData);
    }
    if (sData.checkOk(curTime) == true)
    {
      m_vecTempOkMap.push_back(sData.dwMapID);
    }
  }

  if (m_vecTempOkMap.empty() && m_vecTempFailMap.empty())
    return;
  // do success process
  for (auto &d : m_vecTempOkMap)
    finishSeal(d);
  // do failure process
  for (auto &d : m_vecTempFailMap)
    failSeal(d);
}

void SceneTeamSeal::sendSealInfo(QWORD userid)
{
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [](const STeamSealData& rData) ->bool {
      return rData.dwSealStopTime != 0;
      });
  if (v == m_vecTeamSealData.end())
    return;

  bool bSpeed = true;
  DWORD buffID = MiscConfig::getMe().getSealCFG().dwQuickFinishBuff;
  for(auto &it : m_setTeamers)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(it);
    if(pUser == nullptr)
      continue;
    if(pUser->m_oBuff.haveBuff(buffID) == true)
    {
      bSpeed = false;
      break;
    }
  }

  SealTimer cmd;
  cmd.set_curvalue(v->dwCurValue);
  cmd.set_maxvalue(v->m_stCurItem.pCFG->dwMaxValue);

  cmd.set_maxtime(v->m_stCurItem.pCFG->dwMaxTime);
  cmd.set_stoptime(v->dwSealStopTime);

  if(bSpeed == true)
    cmd.set_speed(MiscConfig::getMe().getSealCFG().dwSpeed);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(userid, send, len);
}

void SceneTeamSeal::onEnterScene(SceneUser* pUser)
{
  if (pUser == nullptr ||pUser->getScene() == nullptr || m_vecTeamSealData.empty())
    return;
  if (pUser->getTeamID() != m_qwTeamID)
    return;
  const GTeam& rTeam = pUser->getTeam();
  for (auto &m : rTeam.getTeamMemberList())
  {
    m_setTeamers.insert(m.second.charid());
  }
  sendSealInfo(pUser->id);

  DWORD mapid = pUser->getScene()->id;
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [&](const STeamSealData& r) -> bool{
      return r.dwMapID == mapid;
      });
  if (v == m_vecTeamSealData.end())
    return;
  if (v->m_stCurItem.qwSealID == 0)
  {
    xPos pos;
    pos = v->m_stCurItem.oPos;
    SceneNpc* pSeal = createSeal(pUser->getScene(), v->m_stCurItem);
    if (pSeal == nullptr)
      return;
    v->m_stCurItem.qwSealID = pSeal->id;
    v->m_stCurItem.oPos = pSeal->getPos();
  }
  pUser->getSeal().addUpdateNewSeal(pUser->getScene()->getMapID(), v->m_stCurItem);

  if (pUser->getScene()->isDScene())
  {
    EFinishType etype = EFINISHTYPE_NORMAL;
    DWORD buffID = MiscConfig::getMe().getSealCFG().dwQuickFinishBuff;
    for(auto &it : m_setTeamers)
    {
      SceneUser* pEnterUser = SceneUserManager::getMe().getUserByID(it);
      if(pEnterUser == nullptr)
        continue;
      if(pEnterUser->m_oBuff.haveBuff(buffID) == true && pUser->m_oBuff.haveBuff(buffID) == false)
      {
        pUser->m_oBuff.add(buffID, pUser);
        break;
      }
    }
    if(pUser->m_oBuff.haveBuff(buffID) == true)
      etype = EFINISHTYPE_QUICK;
    beginSeal(pUser, etype);
    v->m_setActUserID.insert(pUser->id);
  }
}

void SceneTeamSeal::onLeaveScene(SceneUser* pUser)
{
  if (pUser == nullptr ||pUser->getScene() == nullptr || m_vecTeamSealData.empty())
    return;

  DWORD mapid = pUser->getScene()->id;
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [mapid](const STeamSealData& r) -> bool{
      return r.dwMapID == mapid;
      });
  if (v == m_vecTeamSealData.end() || v->m_stCurItem.qwSealID == 0)
    return;

  pUser->getSeal().addUpdateDelSeal(pUser->getScene()->getMapID(), v->m_stCurItem);
  pUser->getSeal().update();
  v->m_setActUserID.erase(pUser->id);

  for (auto &t : m_setTeamers)
  {
    if (t == pUser->id)
      continue;
    SceneUser* pteamer = SceneUserManager::getMe().getUserByID(t);
    if (pteamer == nullptr || pteamer->getScene() != pUser->getScene())
      continue;
    return;
  }

  if (pUser->getScene()->isDScene())
  {
    delSceneImage(pUser->getScene());
    m_dwDMapID = 0;
    m_dwExitDMapTime = 0;
    m_dwCountDownTime = 0;
    m_bSealing = false;
  }

  if (pUser->getScene()->isDScene())
  {
    failSeal(v->dwMapID);
  }
  else if(!v->m_stCurItem.bSealing)
  {
    SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(v->m_stCurItem.qwSealID);
    if (pSeal)
    {
      pSeal->setClearState();
    }
    v->m_stCurItem.qwSealID = 0;
  }
}

void SceneTeamSeal::onUserOffline(SceneUser* pUser)
{
  if (!pUser)
    return;
  if (pUser->getTeamLeaderID() == pUser->id)
  {
    // 队长切线, 放弃本线封印
    if (pUser->getUserSceneData().getZoneID() != thisServer->getZoneID())
    {
      SetTeamSeal cmd;
      cmd.set_teamid(m_qwTeamID);
      cmd.set_estatus(ESETSEALSTATUS_INVALID);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToSession(send, len);
    }
  }
}

void SceneTeamSeal::delSceneImage(Scene* pScene)
{
  if (pScene == nullptr || pScene->isDScene() == false)
    return;
  DScene* pDScene = dynamic_cast<DScene*> (pScene);
  if (pDScene == nullptr)
    return;

  Scene* pRealScene = SceneManager::getMe().getSceneByID(pScene->getMapID());
  if (pRealScene)
  {
    pRealScene->m_oImages.del(m_qwTeamID, pDScene->getRaidID());
    onOverRaidSeal(pScene->getMapID());
    return;
  }

  DelSceneImage cmd;
  cmd.set_guid(m_qwTeamID);
  cmd.set_realscene(pScene->getMapID());
  cmd.set_etype(ESCENEIMAGE_SEAL);
  cmd.set_raid(pDScene->getRaidID());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

void SceneTeamSeal::onOverRaidSeal(DWORD mapid)
{
  if (m_vecTeamSealData.empty())
    return;
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [mapid](const STeamSealData& r) -> bool{
      return r.dwMapID == mapid;
      });
  if (v == m_vecTeamSealData.end())
    return;
  for (auto &t : m_setTeamers)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(t);
    if (pUser == nullptr || pUser->getScene() == nullptr || pUser->getScene()->id != mapid)
      continue;
    pUser->getSeal().addUpdateDelSeal(mapid, v->m_stCurItem);
  }
  v->clear();
  m_vecTeamSealData.erase(v);
}

void SceneTeamSeal::clearSealData(DWORD destSeal)
{
  auto v = find_if(m_vecTeamSealData.begin(), m_vecTeamSealData.end(), [destSeal](const STeamSealData& r) ->bool {
      return r.m_stCurItem.pCFG->dwID != destSeal;
      });
  if (v != m_vecTeamSealData.end())
  {
    v->clear();
    Scene* pScene = SceneManager::getMe().getSceneByID(v->m_stCurItem.pCFG->dwMapID);
    if (pScene)
    {
      for (auto &t : m_setTeamers)
      {
        SceneUser* pTeamer = SceneUserManager::getMe().getUserByID(t);
        if (pTeamer == nullptr || pTeamer->getScene() != pScene)
          continue;
        pTeamer->getSeal().addUpdateDelSeal(pScene->getMapID(), v->m_stCurItem);
      }
    }
    m_vecTeamSealData.erase(v);
  }
}

/*
void SceneTeamSeal2::addSealData(DWORD dwID, xPos pos)
{
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwID);
  if (pCFG == nullptr)
    return;

  m_vecTeamSealData.clear();
  STeamSealData2 stData;
  stData.dwMapID = pCFG->dwMapID;
  stData.m_stCurItem.pCFG = pCFG;
  stData.m_stCurItem.oPos = pos;
  stData.m_stCurItem.bOwnSeal = true;

  m_vecTeamSealData.push_back(stData);
}
*/

TeamSealManager::TeamSealManager()
{

}

TeamSealManager::~TeamSealManager()
{
  for (auto &m : m_mapTeamSeal)
  {
    SAFE_DELETE(m.second);
  }
  m_mapTeamSeal.clear();
}

void TeamSealManager::timer(DWORD curSec)
{
  FUN_TIMECHECK_30();
  for (auto m = m_mapTeamSeal.begin(); m != m_mapTeamSeal.end(); )
  {
    if (m->second->getDestructTime() !=0 && curSec >= m->second->getDestructTime())
    {
      SAFE_DELETE(m->second);
      m = m_mapTeamSeal.erase(m);
      continue;
    }
    m->second->timer(curSec);
    ++m;
  }
}

SceneTeamSeal*::TeamSealManager::getTeamSealByID(QWORD guid)
{
  auto it = m_mapTeamSeal.find(guid);
  if (it != m_mapTeamSeal.end())
    return it->second;
  return nullptr;
}

SceneTeamSeal* TeamSealManager::createOneTeamSeal(QWORD teamid)
{
  auto it = m_mapTeamSeal.find(teamid);
  if (it == m_mapTeamSeal.end())
  {
    SceneTeamSeal* pTeamSeal = NEW SceneTeamSeal(teamid);
    if (pTeamSeal == nullptr)
      return nullptr;
    m_mapTeamSeal[teamid] = pTeamSeal;
    it = m_mapTeamSeal.find(teamid);
  }

  if (it == m_mapTeamSeal.end())
    return nullptr;

  return it->second;
}
