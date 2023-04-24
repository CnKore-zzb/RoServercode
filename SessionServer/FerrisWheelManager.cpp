#include "FerrisWheelManager.h"
#include "SessionUser.h"
#include "SessionUserManager.h"
#include "MsgManager.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"

FerrisWheelManager::FerrisWheelManager()
{

}

FerrisWheelManager::~FerrisWheelManager()
{

}

bool FerrisWheelManager::invite(DWORD dwID, SessionUser* pUser, QWORD qwTargetID)
{
  if (pUser == nullptr)
    return false;

  if (pUser->id == qwTargetID)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_USER_OFFLINE);
    return false;
  }

  SFerrisPartner* pPartner = getPartner(dwID, pUser->id, qwTargetID);
  if (pPartner != nullptr)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_FERRIS_INVITE_SEND);
    return false;
  }

  const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(dwID);
  if (pCFG == nullptr)
  {
    XERR << "[约会圣地-邀请]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "id:" << dwID
         << "配置找不到" << XEND;
    return false;
  }
  auto m = m_mapFerrisPartner.find(dwID);
  if (m != m_mapFerrisPartner.end())
  {
    auto v = m->second.find(pUser->id);
    if (v != m->second.end() && v->second.size() >= pCFG->dwInviteMaxCount)
    {
      MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_FERRIS_INVITE_FAIL);
      return false;
    }
  }

  /*const TeamInfo& rTeam = pUser->getTeamInfo();
  for (int i = 0; i < rTeam.member_size(); ++i)
  {
    const TeamMemberInfo& rMember = rTeam.member(i);
    if (rMember.charid() == qwTargetID && rMember.zoneid() != thisServer->getZoneID())
    {
      MsgManager::sendMsg(pUser->id, ESYSTEAMSG_ID_FERRIS_INVITE_ZONE);
      return false;
    }
  }*/

  SessionUser* pTarget = SessionUserManager::getMe().getUserByID(qwTargetID);
  if (pTarget == nullptr)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEAMSG_ID_FERRIS_INVITE_ERROR);
    return false;
  }
  if (pTarget->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK) == true || pTarget->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK_FOREVER) == true)
  {
    XERR << "[约会圣地-邀请]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "邀请" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "失败,黑名单" << "id:" << dwID << XEND;
    return false;
  }
  if (canInvite(dwID, pUser) == false || canInvite(dwID, pTarget) == false)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_FERRIS_INVITE_FAIL);
    return false;
  }

  if (pCFG->bGender == false && pUser->getGender() == pTarget->getGender())
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_FERRIS_INVITE_GENDER_ERROR);
    return false;
  }

  SFerrisPartner stPartner;
  stPartner.qwCharID = qwTargetID;
  stPartner.dwTime = xTime::getCurSec();
  m_mapFerrisPartner[dwID][pUser->id].push_back(stPartner);

  FerrisWheelInviteCarrierCmd cmd;
  cmd.set_targetid(pUser->id);
  cmd.set_targetname(pUser->name);
  cmd.set_id(dwID);
  PROTOBUF(cmd, send, len);
  pTarget->sendCmdToMe(send, len);

  XLOG << "[约会圣地管理-邀请]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "邀请了" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "id:" << dwID << XEND;
  return true;
}

bool FerrisWheelManager::processInvite(DWORD dwID, SessionUser* pUser, QWORD qwTargetID, EFerrisAction eAction)
{
  if (pUser == nullptr)
    return false;
  if (eAction <= EFERRISACTION_MIN || eAction >= EFERRISACTION_MAX || EFerrisAction_IsValid(eAction) == false)
    return false;

  SessionUser* pTarget = SessionUserManager::getMe().getUserByID(qwTargetID);
  if (pTarget == nullptr)
    return false;
  SFerrisPartner* pPartner = getPartner(dwID, pTarget->id, pUser->id);
  if (pPartner == nullptr || canInvite(dwID, pUser) == false || canInvite(dwID, pTarget) == false)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_FERRIS_INVITE_FAIL);
    return false;
  }

  const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(dwID);
  if (pCFG == nullptr)
    return false;

  string strAction;
  if (eAction == EFERRISACTION_DISAGREE)
  {
    strAction = "拒绝";
    MsgManager::sendMsg(pTarget->id, ESYSTEMMSG_ID_FERRIS_INVITE_DISAGREE);
  }
  else if (eAction == EFERRISACTION_AGREE)
  {
    SFerrisReady stReady;
    stReady.qwMaster = qwTargetID;
    stReady.qwTarget = pUser->id;
    stReady.dwTime = xTime::getCurSec();
    m_mapFerrisReady[dwID].push_back(stReady);
    strAction = "答应";

    MsgManager::sendMsg(pTarget->id, ESYSTEMMSG_ID_FERRIS_INVITE_AGREE, MsgParams(pCFG->sName));

    EnterFerrisReadySessionCmd cmd;
    cmd.set_charid(stReady.qwMaster);
    cmd.set_id(dwID);
    PROTOBUF(cmd, send, len);
    pTarget->sendCmdToScene(send, len);
  }
  else
  {
    return false;
  }

  removeInvite(dwID, pUser->id);
  removeInvite(dwID, qwTargetID);

  XLOG << "[约会圣地管理-邀请回复]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << strAction
       << "了" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "的邀请" << "id:" << dwID << XEND;
  return true;
}

bool FerrisWheelManager::ready(const EnterFerrisReadySessionCmd& cmd)
{
  QWORD id = cmd.charid();
  auto itReady = m_mapFerrisReady.find(cmd.id());
  if (itReady == m_mapFerrisReady.end())
    return false;
  auto v = find_if(itReady->second.begin(), itReady->second.end(), [id](const SFerrisReady& r) -> bool {
      return r.qwMaster == id;
    });
  if (v == itReady->second.end())
    return false;

  if (cmd.msgid() != 0)
  {
    itReady->second.erase(v);
    if (cmd.msgid() != 10)
    {
      if (cmd.msgid() == ESYSTEMMSG_ID_FERRIS_INVITE_ITEM_ERROR)
      {
        const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(cmd.id());
        if (pCFG != nullptr)
          MsgManager::sendMsg(cmd.charid(), cmd.msgid(), MsgParams(pCFG->sName));
      }
      else
        MsgManager::sendMsg(cmd.charid(), cmd.msgid());
    }
    return false;
  }

  SessionUser* pMaster = SessionUserManager::getMe().getUserByID(id);
  SessionUser* pTarget = SessionUserManager::getMe().getUserByID(v->qwTarget);
  if (pMaster == nullptr || pTarget == nullptr)
  {
    itReady->second.erase(v);
    return false;
  }

  v->dwTime = xTime::getCurSec();
  XLOG << "[约会圣地管理-准备]" << pMaster->accid << pMaster->id << pMaster->getProfession() << pMaster->name << "准备完毕,开始和" << v->qwTarget << "进入id:" << cmd.id() << XEND;
  return true;
}

void FerrisWheelManager::timer(DWORD curTime)
{
  for (auto it = m_mapFerrisPartner.begin(); it != m_mapFerrisPartner.end(); ++it)
  {
    const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(it->first);
    if (pCFG == nullptr)
      continue;

    for (auto m = it->second.begin(); m != it->second.end();)
    {
      for (auto v = m->second.begin(); v != m->second.end();)
      {
        if (curTime > v->dwTime + pCFG->dwInviteOverTime + pCFG->dwCountDown)
        {
          MsgManager::sendMsg(m->first, ESYSTEMMSG_ID_FERRIS_INVITE_OVERTIME);
          v = m->second.erase(v);
          continue;
        }
        ++v;
      }

      if (m->second.empty() == true)
      {
        m = it->second.erase(m);
        continue;
      }
      ++m;
    }
  }

  for (auto it = m_mapFerrisReady.begin(); it != m_mapFerrisReady.end(); ++it)
  {
    const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(it->first);
    if (pCFG == nullptr)
      continue;

    for (auto v = it->second.begin(); v != it->second.end();)
    {
      if (curTime > v->dwTime + pCFG->dwEnterWaitTime)
      {
        const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(pCFG->dwRaidID);
        if (pRaidCFG == nullptr)
        {
          XERR << "[约会圣地管理-进入]" << v->qwMaster << v->qwTarget << "id:" << it->first << "raidid:" << pCFG->dwRaidID << "未发现约会圣地地图" << XEND;
        }
        else
        {
          CreateRaidMapSessionCmd raidcmd;
          RaidMapData* pData = raidcmd.mutable_data();
          pData->set_raidid(pRaidCFG->dwRaidID);
          pData->add_memberlist(v->qwMaster);
          pData->add_memberlist(v->qwTarget);
          pData->set_restrict(pRaidCFG->eRestrict);

          SessionSceneManager::getMe().createRaidMap(raidcmd);
        }

        v = it->second.erase(v);
        continue;
      }
      ++v;
    }
  }
}

SFerrisPartner* FerrisWheelManager::getPartner(DWORD dwID, QWORD qwMaster, QWORD qwPartner)
{
  auto m = m_mapFerrisPartner.find(dwID);
  if (m == m_mapFerrisPartner.end())
    return nullptr;
  auto n = m->second.find(qwMaster);
  if (n == m->second.end())
    return nullptr;
  auto v = find_if(n->second.begin(), n->second.end(), [qwPartner](const SFerrisPartner& r) -> bool{
    return r.qwCharID == qwPartner;
  });
  if (v != n->second.end())
    return &(*v);

  return nullptr;
}

bool FerrisWheelManager::removeInvite(DWORD dwID, QWORD qwCharID)
{
  auto m = m_mapFerrisPartner.find(dwID);
  if (m == m_mapFerrisPartner.end())
    return false;
  auto v = m->second.find(qwCharID);
  if (v == m->second.end())
    return false;
  m->second.erase(v);
  return true;
}

bool FerrisWheelManager::canInvite(DWORD dwID, SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;

  const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(dwID);
  if (pCFG == nullptr)
    return false;

  SessionScene* pScene = pUser->getScene();
  if (pScene == nullptr || pScene->m_oRaidData.m_dwRaidID == pCFG->dwRaidID)
    return false;

  auto it = m_mapFerrisReady.find(dwID);
  if (it == m_mapFerrisReady.end())
    return true;

  DWORD id = pUser->id;
  auto v = find_if(it->second.begin(), it->second.end(), [id](const SFerrisReady& r) -> bool{
    return r.qwMaster == id || r.qwTarget == id;
  });
  return v == it->second.end();
}

