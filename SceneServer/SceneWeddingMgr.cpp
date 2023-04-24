#include "SceneWeddingMgr.h"
#include "WeddingConfig.h"
#include "GMCommandRuler.h"
#include "SceneManager.h"
#include "SceneActManager.h"
#include "DScene.h"

void SceneWeddingMgr::startWedding(Cmd::StartWeddingSCmd& cmd)
{
  //先stop
  if (m_oCurWedding.id())
  {
    StopWeddingSCmd stopCmd;
    stopCmd.set_id(m_oCurWedding.id());
    stopWedding(stopCmd);
  }    
  m_oCurWedding = cmd.weddinginfo();

  m_setSceneEventIDs.clear();

  TSetDWORD effids;
  const WeddingManualInfo& manual = m_oCurWedding.manual();
  for (int i = 0; i < manual.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* pCFG = WeddingConfig::getMe().getWeddingServiceCFG(manual.serviceids(i));
    if (pCFG == nullptr || pCFG->eType != EWEDDINGSERVICE_PACKAGE)
      continue;

    for (auto &s : pCFG->setServiceID)
    {
      if (effids.find(s) != effids.end())
        continue;
      effids.insert(s);
      doServiceEffect(s);
      XLOG << "[婚礼-开始], 执行套餐效果, 婚礼id:" << m_oCurWedding.id() << "套餐:" << manual.serviceids(i) << s << XEND;
    }
  }

  XLOG << "[婚礼-开始] " << m_oCurWedding.id() << m_oCurWedding.charid1() << m_oCurWedding.charid2() << XEND;
}

void SceneWeddingMgr::stopWedding(Cmd::StopWeddingSCmd& cmd)
{ 
  if (m_oCurWedding.id() && m_oCurWedding.id() != cmd.id())
  {
    XLOG << "[婚礼-结束] " << cmd.id() << "已处理过结束" << XEND;
    return;
  }
  m_oCurWedding.Clear();

  if (!m_setSceneEventIDs.empty())
  {
    for (auto &s : m_setSceneEventIDs)
    {
      SceneActBase* pAct = SceneActManager::getMe().getSceneAct(s);
      if (pAct)
        pAct->setClearState();
    }
    m_setSceneEventIDs.clear();
  }

  if (m_qwCurWeddingScenID)
  {
    Scene* pScene = SceneManager::getMe().getSceneByID(m_qwCurWeddingScenID);
    WeddingScene* pWScene = dynamic_cast<WeddingScene*> (pScene);
    if (pWScene)
      pWScene->setCloseTime(now());
    m_qwCurWeddingScenID = 0;
  }

  Scene* pScene = SceneManager::getMe().getSceneByID(MAP_PRONTERA);
  if (pScene != nullptr)
    pScene->resetPhotoWall(EFRAMETYPE_WEDDING);

  XLOG << "[婚礼-结束] " << cmd.id() << XEND;
}

void SceneWeddingMgr::updateManual(QWORD id, const WeddingManualInfo& info)
{
  if (id != m_oCurWedding.id())
    return;

  TSetDWORD oldserIDS;

  const WeddingManualInfo& manual = m_oCurWedding.manual();
  for (int i = 0; i < manual.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* pCFG = WeddingConfig::getMe().getWeddingServiceCFG(manual.serviceids(i));
    if (pCFG == nullptr || pCFG->eType != EWEDDINGSERVICE_PACKAGE)
      continue;

    for (auto &s : pCFG->setServiceID)
      oldserIDS.insert(s);
  }

  TSetDWORD newserIDs;
  for (int i = 0; i < info.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* pCFG = WeddingConfig::getMe().getWeddingServiceCFG(info.serviceids(i));
    if (pCFG == nullptr || pCFG->eType != EWEDDINGSERVICE_PACKAGE)
      continue;

    for (auto &s : pCFG->setServiceID)
    {
      if (oldserIDS.find(s) == oldserIDS.end())
        newserIDs.insert(s);
    }
  }

  for (auto &s : newserIDs)
  {
    doServiceEffect(s);
    if (m_oCurWedding.status() == EWeddingStatus_Married)
      doMarrySuccessEffect(s);

    XLOG << "[婚礼-手册更新], 执行套餐效果, 婚礼id:" << m_oCurWedding.id() << "效果:" << s << XEND;
  }

  Scene* pScene = SceneManager::getMe().getSceneByID(m_qwCurWeddingScenID);
  if (pScene)
  {
    WeddingScene* pWScene = dynamic_cast<WeddingScene*>(pScene);
    if (pWScene)
    {
      pWScene->onAddService(newserIDs);
      pWScene->updateManual(info);
    }
  }
  m_oCurWedding.mutable_manual()->CopyFrom(info);
}

void SceneWeddingMgr::doServiceEffect(DWORD id)
{
  const SWeddingServiceCFG* p = WeddingConfig::getMe().getWeddingServiceCFG(id);
  if (!p || p->eType != EWEDDINGSERVICE_PLAN)
    return;
  for (auto &d : p->vecEffectGMData)
  {
    DWORD mapid = d.getTableInt("map");
    if (MapConfig::getMe().isRaidMap(mapid) == false)
    {
      Scene* pScene = SceneManager::getMe().getSceneByID(mapid);
      if (pScene)
        GMCommandRuler::getMe().scene_execute(pScene, d);
    }
  }
}

void SceneWeddingMgr::doMarrySuccessEffect(DWORD id)
{
  const SWeddingServiceCFG* p = WeddingConfig::getMe().getWeddingServiceCFG(id);
  if (!p || p->eType != EWEDDINGSERVICE_PLAN)
    return;
  for (auto &d : p->vecSuccessGMData)
  {
    DWORD mapid = d.getTableInt("map");
    if (MapConfig::getMe().isRaidMap(mapid) == false)
    {
      Scene* pScene = SceneManager::getMe().getSceneByID(mapid);
      if (pScene)
        GMCommandRuler::getMe().scene_execute(pScene, d);
    }
  }
}

bool SceneWeddingMgr::hasServiceID(DWORD id) const
{
  const WeddingManualInfo& manual = m_oCurWedding.manual();
  for (int i = 0; i < manual.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* pCFG = WeddingConfig::getMe().getWeddingServiceCFG(manual.serviceids(i));
    if (pCFG == nullptr)
      continue;

    if (pCFG->setServiceID.find(id) != pCFG->setServiceID.end())
      return true;
  }
  return false;
}

void SceneWeddingMgr::onMarrySuccess(QWORD id)
{
  if (m_oCurWedding.id() != id)
    return;

  m_oCurWedding.set_status(EWeddingStatus_Married);

  TSetDWORD effids;
  const WeddingManualInfo& manual = m_oCurWedding.manual();
  for (int i = 0; i < manual.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* pCFG = WeddingConfig::getMe().getWeddingServiceCFG(manual.serviceids(i));
    if (pCFG == nullptr || pCFG->eType != EWEDDINGSERVICE_PACKAGE)
      continue;

    for (auto &s : pCFG->setServiceID)
    {
      if (effids.find(s) != effids.end())
        continue;
      effids.insert(s);
      doMarrySuccessEffect(s);
      XLOG << "[婚礼-成功], 执行套餐效果, 婚礼id:" << m_oCurWedding.id() << "套餐:" << manual.serviceids(i) << s << XEND;
    }
  }
}

