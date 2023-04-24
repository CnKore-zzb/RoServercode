#include "BossMgr.h"
#include "BossConfig.h"
#include "BossAct.h"
#include "SceneManager.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "BossSCmd.pb.h"
#include "SceneUser.h"

// BossMgr
bool BossMgr::loadConfig()
{
  m_mapBossActCFG.clear();

  auto createf = [&](DWORD dwActID, const SBossStepCFG& rCFG)
  {
    if (dwActID == 0)
      return;

    SBossActCFG& rActCFG = m_mapBossActCFG[dwActID];
    rActCFG.dwActID = dwActID;
    TVecBossStep& vecSteps = rActCFG.vecBossStep;

    switch (rCFG.step)
    {
      case EBOSSSTEP_MIN:
      case EBOSSSTEP_MAX:
      case EBOSSSTEP_END:
        break;
      case EBOSSSTEP_VISIT:
        {
          TPtrBossStep pStep( new BossVisitStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_SUMMON:
        {
          TPtrBossStep pStep( new BossSummonStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_CLEAR:
        {
          TPtrBossStep pStep( new BossClearStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_BOSS:
        {
          TPtrBossStep pStep( new BossBossStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_LIMIT:
        {
          TPtrBossStep pStep( new BossLimitStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_DIALOG:
        {
          TPtrBossStep pStep( new BossDialogStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_STATUS:
        {
          TPtrBossStep pStep( new BossStatusStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_WAIT:
        {
          TPtrBossStep pStep( new BossWaitStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_KILL:
        {
          TPtrBossStep pStep( new BossKillStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_WORLD:
        {
          TPtrBossStep pStep( new BossWorldStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
      case EBOSSSTEP_SHOW:
        {
          TPtrBossStep pStep( new BossShowStep());
          if (pStep == nullptr || pStep->init(rCFG.data) == false)
            return;
          vecSteps.push_back(pStep);
        }
        break;
    }
  };
  BossConfig::getMe().foreachstep(createf);
  return true;
}

bool BossMgr::onSummonBoss(const Cmd::SummonBossBossSCmd& rCmd, DWORD dwGMActID /*= 0*/)
{
  Scene* pScene = SceneManager::getMe().getSceneByID(rCmd.mapid());
  if (pScene == nullptr)
  {
    XERR << "[Boss-事件]" << rCmd.ShortDebugString() << "处理失败,未找到场景" << XEND;
    return false;
  }

  const SBossCFG* pCFG = BossConfig::getMe().getBossCFG(rCmd.npcid());
  if (pCFG == nullptr)
  {
    XERR << "[Boss-事件]" << rCmd.ShortDebugString() << "处理失败,npcid未在 Table_Boss.txt 表中找到" << XEND;
    return false;
  }

  DWORD dwActID = dwGMActID != 0 ? dwGMActID : pCFG->randActID();
  if (dwActID != 0)
  {
    const SBossActCFG* pActCFG = getBossActCFG(dwActID);
    if (pActCFG != nullptr)
      return createAct(pActCFG, rCmd);
    XERR << "[Boss-事件]" << rCmd.ShortDebugString() << "处理中包含actid" << dwActID << "但未在 Table_BossStep.txt 表中找到,直接召唤" << XEND;
  }

  return summonBoss(rCmd);
}

bool BossMgr::createAct(const SBossActCFG* pCFG, const Cmd::SummonBossBossSCmd& rCmd)
{
  if (pCFG == nullptr)
    return false;

  auto m = m_mapBossAct.find(rCmd.mapid());
  if (m != m_mapBossAct.end())
  {
    XERR << "[Boss-创建活动]" << rCmd.ShortDebugString() << "创建活动" << pCFG->dwActID << "失败,该地图上已存在" << XEND;
    return false;
  }

  BossAct* pAct = new BossAct();
  if (pAct == nullptr)
  {
    XERR << "[Boss-创建活动]" << rCmd.ShortDebugString() << "创建活动" << pCFG->dwActID << "失败,创建失败" << XEND;
    return false;
  }
  pAct->setActID(pCFG->dwActID);
  pAct->setBossID(rCmd.npcid());
  pAct->setMapID(rCmd.mapid());
  pAct->setLv(rCmd.lv());
  pAct->setCFG(pCFG);
  m_mapBossAct.insert(std::make_pair(rCmd.mapid(), pAct));

  XLOG << "[Boss-创建活动]" << rCmd.ShortDebugString() << "创建活动" << pCFG->dwActID << "成功" << XEND;

  if (canStep(pAct) == true)
    runStep(pAct, nullptr);
  return true;
}

bool BossMgr::summonBoss(const Cmd::SummonBossBossSCmd& rCmd, const xPos& rPos /*= xPos(0.0f, 0.0f, 0.0f)*/)
{
  Scene* scene = SceneManager::getMe().getSceneByID(rCmd.mapid());
  if (scene == nullptr)
  {
    XERR << "[Boss-召唤]" << rCmd.ShortDebugString() << "召唤Boss失败,未找到场景对象" << XEND;
    return false;
  }

  std::list<SceneNpc *> npclist;
  scene->getSceneNpcByBaseID(rCmd.npcid(), npclist);
  for (auto &s : npclist)
  {
    if (s->isSummonBySession() && s->isAlive())
    {
      XERR << "[Boss-召唤]" << rCmd.ShortDebugString() << "召唤Boss失败,boss已存在" << XEND;
      return false;
    }
  }

  const SBossCFG* base = BossConfig::getMe().getBossCFG(rCmd.npcid());
  if (base == nullptr)
  {
    XERR << "[Boss-召唤]" << rCmd.ShortDebugString() << "召唤Boss失败,boss未在 Table_Boss.txt 表中找到" << XEND;
    return false;
  }

  NpcDefine def;
  def.load(base->getBossDefine());
  def.setID(rCmd.npcid());
  def.setDeadLv(rCmd.lv());
  def.setLife(1);
  xPos p = rPos.empty() == true ? def.getPos() : rPos;
  if (rPos.empty() == true)
    scene->getRandPos(p);
  else
    scene->getValidPos(p);
  def.setPos(p);

  const SBossLvCFG* pLvCFG = base->getLvCFG(def.getDeadLv());
  if (pLvCFG != nullptr)
  {
    for (auto &s : pLvCFG->setSuperAIs)
      def.addSuperAI(s);
    for (auto &s : pLvCFG->setRewardIDs)
      def.addDeadRewardID(s);
  }

  def.setBossType(base->eType);

  SceneNpc *npc = SceneNpcManager::getMe().createNpc(def, scene);
  if (npc == nullptr)
  {
    XERR << "[Boss-召唤]" << rCmd.ShortDebugString() << "召唤Boss失败,创建npc对象失败" << XEND;
    return false;
  }
  //npc->m_eBossType = base->eType;//static_cast<EBossType>(base->getType());

  if (pLvCFG != nullptr)
  {
    for (auto &s : pLvCFG->setBuffIDs)
      npc->m_oBuff.add(s);
  }

  XLOG << "[Boss-召唤]" << rCmd.ShortDebugString() << "召唤Boss成功,pos :(" << npc->getPos().x << npc->getPos().y << npc->getPos().z << ")" << XEND;
  return true;
}

bool BossMgr::runStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  const SBossActCFG* pCFG = pAct->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[Boss-活动推进] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "推进进度失败, 未包含合法的配置表" << XEND;
    return false;
  }

  TPtrBossStep pStep = pAct->getStepCFG();
  if (pStep == nullptr)
  {
    XERR << "[Boss-活动推进] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "推进进度失败, 当前进度不合法" << XEND;
    return false;
  }

  if (pStep->doStep(pAct, pUser) == false)
    return true;
  XLOG << "[Boss-活动推进] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "推进进度成功, 当前进度" << pAct->getStep() << XEND;

  if (pAct->finish() == true)
  {
    auto m = m_mapBossAct.find(pAct->getMapID());
    if (m != m_mapBossAct.end())
    {
      m_setRecycledAct.insert(m->second);
      m_mapBossAct.erase(m);
      XLOG << "[Boss-活动推进] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "推进进度成功, 步骤已全部执行完成,移至回收站" << XEND;
    }
    return true;
  }

  if (canStep(pAct) == false)
    return true;

  return runStep(pAct, pUser);
}

bool BossMgr::runStep(DWORD dwMapID, SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  auto m = m_mapBossAct.find(dwMapID);
  if (m == m_mapBossAct.end())
    return false;

  return runStep(m->second, pUser);
}

bool BossMgr::canUpdate(BossAct* pAct)
{
  if (pAct == nullptr)
    return false;
  const TPtrBossStep pStep = pAct->getStepCFG();
  if (pStep == nullptr)
    return true;
  static const set<EBossStep> setSteps = {EBOSSSTEP_SUMMON, EBOSSSTEP_CLEAR, EBOSSSTEP_BOSS, EBOSSSTEP_LIMIT, EBOSSSTEP_WAIT, EBOSSSTEP_KILL};
  return setSteps.find(pStep->getType()) == setSteps.end();
}

const SBossActCFG* BossMgr::getBossActCFG(DWORD dwID) const
{
  auto m = m_mapBossActCFG.find(dwID);
  if (m != m_mapBossActCFG.end())
    return &m->second;
  return nullptr;
}

void BossMgr::onEnterScene(SceneUser* pUser)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;

  Scene* pScene = pUser->getScene();

  auto m = m_mapBossAct.find(pScene->getMapID());
  if (m != m_mapBossAct.end())
    m->second->notify(pUser);
}

void BossMgr::onKillMonster(SceneUser* pUser, DWORD dwMonsterID)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;

  Scene* pScene = pUser->getScene();

  auto m = m_mapBossAct.find(pScene->getMapID());
  if (m == m_mapBossAct.end())
    return;

  BossAct* pAct = m->second;
  TPtrBossStep pStep = pAct->getStepCFG();

  BossVisitStep* pVisitStep = dynamic_cast<BossVisitStep*>(pStep.get());
  if (pVisitStep != nullptr && pVisitStep->getMonsterID() == dwMonsterID && pAct->getProcess().isEnableChar(pUser->id) == false)
  {
    pAct->getProcess().addEnableChar(pUser->id);
    pAct->notify(pUser);
    MsgManager::sendMsg(pUser->id, 26100);
    return;
  }

  BossKillStep* pKillStep = dynamic_cast<BossKillStep*>(pStep.get());
  if (pKillStep != nullptr && pKillStep->isMonster(dwMonsterID))
  {
    runStep(pAct, pUser);
    return;
  }

  BossShowStep* pShowStep = dynamic_cast<BossShowStep*>(pStep.get());
  if (pShowStep != nullptr)
  {
    runStep(pAct, pUser);
    return;
  }
}

void BossMgr::onStatus(SceneUser* pUser)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;

  Scene* pScene = pUser->getScene();

  auto m = m_mapBossAct.find(pScene->getMapID());
  if (m == m_mapBossAct.end())
    return;

  BossAct* pAct = m->second;
  TPtrBossStep pStep = pAct->getStepCFG();

  BossStatusStep* pStatusStep = dynamic_cast<BossStatusStep*>(pStep.get());
  if (pStatusStep != nullptr && pAct->getProcess().isEnableChar(pUser->id) == false)
  {
    runStep(pAct, pUser);
    XDBG << "[Boss-动作事件]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "状态 status :" << pUser->getStatus() << "被 mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "desttime :" << pAct->getDestTime() << "被捕获" << XEND;
  }
}

void BossMgr::onTimer()
{
  set<BossAct*> setActs;
  for (auto &m : m_mapBossAct)
  {
    TPtrBossStep pStep = m.second->getStepCFG();
    if (pStep != nullptr && (pStep->getType() == EBOSSSTEP_WAIT || pStep->getType() == EBOSSSTEP_WORLD))
      setActs.insert(m.second);
  }
  for (auto &s : setActs)
    runStep(s, nullptr);
}

void BossMgr::onShow(SceneNpc* pNpc)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return;

  auto m = m_mapBossAct.find(pNpc->getScene()->getMapID());
  if (m == m_mapBossAct.end())
    return;

  BossAct* pAct = m->second;

  TPtrBossStep pStep = pAct->getStepCFG();

  BossShowStep* pShowStep = dynamic_cast<BossShowStep*>(pStep.get());
  if (pShowStep != nullptr)
    runStep(pAct, nullptr);
}

void BossMgr::timer(DWORD curSec)
{
  if (m_oOneMinTimer.timeUp(curSec))
  {
    recycleAct();
    removeOverTimeAct(curSec);
  }

  if (m_oOneSecTimer.timeUp(curSec))
  {
    onTimer();
  }
}

void BossMgr::clear()
{
  for (auto &m : m_mapBossAct)
  {
    m_setRecycledAct.insert(m.second);
    BossAct* pAct = m.second;
    XLOG << "[Boss-活动清理] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "被清理,移至回收站" << XEND;
  }
  m_mapBossAct.clear();
}

const string& BossMgr::debugInfo(DWORD dwMapID)
{
  stringstream sstr;
  auto m = m_mapBossAct.find(dwMapID);
  if (m == m_mapBossAct.end())
  {
    sstr << "地图 : " << dwMapID << " 不包含亡者boss玩法";
    m_strDebugInfo = sstr.str();
    return m_strDebugInfo;
  }

  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if (pScene == nullptr)
  {
    sstr << "地图 : " << dwMapID << " 未找到对应服务器进程";
    m_strDebugInfo = sstr.str();
    return m_strDebugInfo;
  }

  BossAct* pAct = m->second;
  sstr << " 地图 : " << dwMapID;
  sstr << " ActID : " << pAct->getActID();
  sstr << " BossID : " << pAct->getBossID();
  sstr << " BossLv : " << pAct->getLv();

  sstr << " 当前步骤 : ";
  TPtrBossStep pStep = pAct->getStepCFG();
  if (pStep == nullptr)
  {
    sstr << "异常";
    m_strDebugInfo = sstr.str();
    return m_strDebugInfo;
  }

  switch (pStep->getType())
  {
    case EBOSSSTEP_MIN:
      sstr << " min";
      break;
    case EBOSSSTEP_VISIT:
      {
        sstr << " visit";
        BossVisitStep* pVisitStep = dynamic_cast<BossVisitStep*>(pStep.get());
        if (pVisitStep != nullptr)
        {
          xSceneEntrySet set;
          pScene->getAllEntryList(SCENE_ENTRY_NPC, set);
          for (auto &s : set)
          {
            SceneNpc* pNpc = dynamic_cast<SceneNpc*>(s);
            if (pNpc != nullptr && pNpc->getNpcID() == pVisitStep->getNpcID())
            {
              string name = pNpc->name;
              const xPos& rPos = pNpc->getPos();
              sstr << " <npcid : " << pNpc->id << " name : " << name.c_str() << " pos :(" << static_cast<SDWORD>(rPos.x) << "," << static_cast<SDWORD>(rPos.y) << "," << static_cast<SDWORD>(rPos.z) << " )>";
              break;
            }
          }
        }
      }
      break;
    case EBOSSSTEP_SUMMON:
      sstr << " summon";
      break;
    case EBOSSSTEP_CLEAR:
      sstr << " clear";
      break;
    case EBOSSSTEP_BOSS:
      sstr << " boss";
      break;
    case EBOSSSTEP_END:
      sstr << " end";
      break;
    case EBOSSSTEP_LIMIT:
      sstr << " limit";
      break;
    case EBOSSSTEP_DIALOG:
      sstr << " dialog";
      break;
    case EBOSSSTEP_STATUS:
      sstr << " status";
      break;
    case EBOSSSTEP_WAIT:
      sstr << " wait";
      break;
    case EBOSSSTEP_KILL:
      {
        sstr << " kill";
        BossKillStep* pKillStep = dynamic_cast<BossKillStep*>(pStep.get());
        if (pKillStep != nullptr)
        {
          xSceneEntrySet set;
          pScene->getAllEntryList(SCENE_ENTRY_NPC, set);
          for (auto &s : set)
          {
            SceneNpc* pNpc = dynamic_cast<SceneNpc*>(s);
            if (pNpc != nullptr && pKillStep->isMonster(pNpc->getNpcID()))
            {
              string name = pNpc->name;
              const xPos& rPos = pNpc->getPos();
              sstr << " <npcid : " << pNpc->id << " name : " << name.c_str() << " pos :(" << static_cast<SDWORD>(rPos.x) << "," << static_cast<SDWORD>(rPos.y) << "," << static_cast<SDWORD>(rPos.z) << " )>";
            }
          }
        }
      }
      break;
    case EBOSSSTEP_WORLD:
      sstr << " world";
      break;
    case EBOSSSTEP_SHOW:
      sstr << " show";
      break;
    case EBOSSSTEP_MAX:
      sstr << " max";
      break;
  }

  m_strDebugInfo = sstr.str();
  return m_strDebugInfo;
}

bool BossMgr::canStep(BossAct* pAct)
{
  if (pAct == nullptr)
    return false;
  const TPtrBossStep pStep = pAct->getStepCFG();
  if (pStep == nullptr)
    return false;
  static const set<EBossStep> setSteps = {EBOSSSTEP_VISIT, EBOSSSTEP_DIALOG, EBOSSSTEP_STATUS};
  return setSteps.find(pStep->getType()) == setSteps.end();
}

void BossMgr::recycleAct()
{
  if (m_setRecycledAct.empty() == true)
    return;

  for (auto &s : m_setRecycledAct)
  {
    BossAct* pAct = s;
    if (pAct == nullptr)
      continue;
    XLOG << "[Boss-活动回收] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "被回收销毁" << XEND;
    SAFE_DELETE(pAct);
  }

  m_setRecycledAct.clear();
}

void BossMgr::removeOverTimeAct(DWORD curSec)
{
  for (auto m = m_mapBossAct.begin(); m != m_mapBossAct.end();)
  {
    BossAct* pAct = m->second;
    if (pAct == nullptr || pAct->getDestTime() == 0 || curSec < pAct->getDestTime())
    {
      ++m;
      continue;
    }

    XLOG << "[Boss-活动时间] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "desttime :" << pAct->getDestTime() << "活动超过当前时间" << curSec << "被回收销毁" << XEND;
    pAct->notifyOverTime();
    m_setRecycledAct.insert(pAct);
    m = m_mapBossAct.erase(m);
  }
}

