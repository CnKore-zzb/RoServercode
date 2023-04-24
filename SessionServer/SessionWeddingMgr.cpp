#include "SessionWeddingMgr.h"
#include "SessionServer.h"
#include "WeddingConfig.h"
#include "GMCommandManager.h"

void SessionWeddingMgr::startWedding(Cmd::StartWeddingSCmd& cmd)
{
  //先stop
  if (m_oCurWedding.id())
  {
    StopWeddingSCmd stopCmd;
    stopCmd.set_id(m_oCurWedding.id());
    stopWedding(stopCmd);
  }
    
  m_oCurWedding = cmd.weddinginfo();

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToAllScene(send, len);

  /*
  const WeddingManualInfo& manual = m_oCurWedding.manual();
  for (int i = 0; i < manual.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* pCFG = WeddingConfig::getMe().getWeddingServiceCFG(manual.serviceids(i));
    if (pCFG == nullptr || pCFG->eType != EWEDDINGSERVICE_PACKAGE)
      continue;

    for (auto &s : pCFG->setServiceID)
    {
      const SWeddingServiceCFG* p = WeddingConfig::getMe().getWeddingServiceCFG(s);
      if (!p || p->eType != EWEDDINGSERVICE_PLAN)
        continue;

      XLOG << "[婚礼-开始], 执行套餐效果, 婚礼id:" << m_oCurWedding.id() << "套餐:" << manual.serviceids(i) << s << XEND;
      GMCommandManager::getMe().execute(p->oEffectGMData, TSetDWORD{});
    }
  }
  */

  XLOG << "[婚礼-开始] 同步到场景" << m_oCurWedding.id() << m_oCurWedding.charid1() << m_oCurWedding.charid2() << XEND;
}

void SessionWeddingMgr::stopWedding(Cmd::StopWeddingSCmd& cmd)
{ 
  if (m_oCurWedding.id() && m_oCurWedding.id() != cmd.id())
  {
    StopWeddingSCmd stopCmd;
    stopCmd.set_id(m_oCurWedding.id()); 
    stopWedding(stopCmd);
  }
  m_oCurWedding.Clear();
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToAllScene(send, len);
  XLOG << "[婚礼-结束] 同步到场景" << cmd.id() << XEND;
}

void SessionWeddingMgr::syncWeddingInfo2Scene(ServerTask* pTask)
{
  if (pTask == nullptr)
    return;

  string name = pTask->getTask() != nullptr ? pTask->getTask()->name : "null";
  string info;
  if (m_oCurWedding.id() != 0)
  {
    StartWeddingSCmd cmd;
    cmd.mutable_weddinginfo()->CopyFrom(m_oCurWedding);
    PROTOBUF(cmd, send, len);
    pTask->sendCmd(send, len);
    info = cmd.ShortDebugString();
  }
  else
  {
    StopWeddingSCmd cmd;
    cmd.set_id(m_oCurWedding.id());
    PROTOBUF(cmd, send, len);
    pTask->sendCmd(send, len);
    info = cmd.ShortDebugString();
  }

  XLOG << "[婚礼-同步] 同步到场景" << name << "info :" << info << XEND;
}

