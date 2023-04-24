#include "BossAct.h"
#include "BossMgr.h"
#include "MsgManager.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneManager.h"
#include "BossSCmd.pb.h"

// BossAct
TPtrBossStep BossAct::getStepCFG()
{
  if (m_pCFG == nullptr)
    return nullptr;
  return m_dwStep >= m_pCFG->vecBossStep.size() ? nullptr : m_pCFG->vecBossStep[m_dwStep];
}

bool BossAct::finish() const
{
  return m_pCFG == nullptr ? true : m_dwStep >= m_pCFG->vecBossStep.size();
}

void BossAct::notify(SceneUser* pUser)
{
  if (m_pCFG == nullptr || BossMgr::getMe().canUpdate(this) == false)
    return;
  if (getDestTime() == 0 || now() >= getDestTime())
    return;

  TPtrBossStep pStep = getStepCFG();

  StepSyncBossCmd cmd;
  cmd.set_actid(getActID());
  cmd.set_step(pStep == nullptr ? EBOSSSTEP_END : pStep->getType());
  if (pStep != nullptr)
    pStep->toParams(cmd.mutable_params());

  if (pStep != nullptr && pStep->getType() == EBOSSSTEP_VISIT)
  {
    BossVisitStep* pVisitStep = dynamic_cast<BossVisitStep*>(pStep.get());
    if (pVisitStep != nullptr)
    {
      Param* pParam = cmd.mutable_params()->add_params();
      pParam->set_key("dialog_select");
      if (pUser != nullptr)
      {
        pParam->set_value(pVisitStep->hasSelect() == true ? (getProcess().isEnableChar(pUser->id) == true ? "1" : "2") : "1");
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
        XDBG << "[Boss-活动通知] mapid :" << getMapID() << "actid :" << getActID() << "通知玩家" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "结果" << cmd.ShortDebugString() << XEND;
        return;
      }
      pParam->set_value(pVisitStep->hasSelect() == true ? "2" : "1");
    }
  }
  else if (pStep != nullptr && pStep->getType() == EBOSSSTEP_DIALOG)
  {
    PROTOBUF(cmd, send, len);
    if (pUser != nullptr)
    {
      if (isActUser(pUser->id) == true)
      {
        pUser->sendCmdToMe(send, len);
        XDBG << "[Boss-活动通知] mapid :" << getMapID() << "actid :" << getActID() << "通知玩家" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "结果" << cmd.ShortDebugString() << XEND;
      }
    }
    else
    {
      for (auto &s : m_setActUserIDs)
      {
        SceneUser* pActUser = SceneUserManager::getMe().getUserByID(s);
        if (pActUser == nullptr || pActUser->getScene() == nullptr || pActUser->getScene()->id != getMapID())
          continue;
        pActUser->sendCmdToMe(send, len);
        XDBG << "[Boss-活动通知] mapid :" << getMapID() << "actid :" << getActID() << "通知玩家" << pActUser->accid << pActUser->id << pActUser->getProfession() << pActUser->name << "结果" << cmd.ShortDebugString() << XEND;
      }
    }
    return;
  }

  if (pUser == nullptr)
  {
    PROTOBUF(cmd, send, len);
    MsgManager::sendMapCmd(getMapID(), send, len);
    XDBG << "[Boss-活动通知] mapid :" << getMapID() << "actid :" << getActID() << "地图广播,结果" << cmd.ShortDebugString() << XEND;
  }
}

void BossAct::notifyOverTime()
{
  const SBossActCFG* pCFG = getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[Boss-活动通知] mapid :" << getMapID() << "actid :" << getActID() << "desttime :" << getDestTime() << "超时,地图广播结束失败,未包含正确的配置" << XEND;
    return;
  }

  for (DWORD d = m_dwStep; d < pCFG->vecBossStep.size(); ++d)
  {
    TPtrBossStep pStep = pCFG->vecBossStep[d];
    if (pStep != nullptr && pStep->getType() == EBOSSSTEP_CLEAR)
      pStep->doStep(this, nullptr);
  }

  Cmd::BossDieBossSCmd scmd;
  scmd.set_npcid(getBossID());
  scmd.set_mapid(getMapID());
  scmd.set_reset(true);
  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmdToSession(ssend, slen);

  StepSyncBossCmd cmd;
  cmd.set_actid(getActID());
  cmd.set_step(EBOSSSTEP_END);
  PROTOBUF(cmd, send, len);
  MsgManager::sendMapCmd(getMapID(), send, len);

  MsgManager::sendMapMsg(getMapID(), 26112);
  XDBG << "[Boss-活动通知] mapid :" << getMapID() << "actid :" << getActID() << "desttime :" << getDestTime() << "超时,地图广播结束" << cmd.ShortDebugString() << XEND;
}

const xPos* BossAct::getNpcPos(DWORD dwNpcID)
{
  auto m = m_mapNpcPos.find(dwNpcID);
  if (m != m_mapNpcPos.end())
    return &m->second;
  return nullptr;
}

