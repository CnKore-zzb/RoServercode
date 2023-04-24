#include "PveCardEffect.h"
#include "DScene.h"
#include "SceneNpcManager.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "GMCommandRuler.h"

PveCardManager::PveCardManager()
{

}

PveCardManager::~PveCardManager()
{
  clear();
}

void PveCardManager::clear()
{
  if (m_mapPveCardEffects.empty())
    return;
  for (auto &m : m_mapPveCardEffects)
    SAFE_DELETE(m.second);
}

bool PveCardManager::init()
{
  clear(); //重加载时, 先清除原有对象

  bool bCorrect = true;
  const TMapPveCardEffectCFG& mapcfg = PveCardConfig::getMe().getCardEffects();
  for (auto &m : mapcfg)
  {
    PveCardTargetBase* pTarget = createTargetObj(&(m.second));
    if (pTarget == nullptr)
    {
      XERR << "[PveCard-创建失败], 未识别的目标类型, id:" << m.first << XEND;
      bCorrect = false;
      continue;
    }

    PveCardBase* pCardEffect = createEffectObj(&(m.second), pTarget);
    if (pCardEffect == nullptr)
    {
      XERR << "[PveCard-创建失败], 未识别的效果类型, id:" << m.first << XEND;
      bCorrect = false;
      SAFE_DELETE(pTarget);
      continue;
    }

    m_mapPveCardEffects[m.first] = pCardEffect;
  }

  return bCorrect;
}

PveCardTargetBase* PveCardManager::createTargetObj(const SPveCardEffectCFG* pCFG)
{
  if (pCFG == nullptr)
    return nullptr;
  switch(pCFG->eTargetType)
  {
    case EPVECARDTARGET_MIN:
    {
      PveCardTargetBase* pTarget = new PveCardTargetBase(pCFG);
      return pTarget;
    }
    break;
    case EPVECARDTARGET_RANDOMUSER:
    {
      PveCardTargetBase* pTarget = new PveCardTargetRandomUser(pCFG);
      return pTarget;
    }
    break;
    case EPVECARDTARGET_ALLUSER:
    {
      PveCardTargetBase* pTarget = new PveCardTargetAllUser(pCFG);
      return pTarget;
    }
    break;
    default:
      break;
  }

  return nullptr;
}

PveCardBase* PveCardManager::createEffectObj(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget)
{
  if (pCFG == nullptr || pTarget == nullptr)
    return nullptr;
  switch(pCFG->eEffectType)
  {
    case EPVECARDEFFECT_MIN:
      break;
    case EPVECARDEFFECT_GM:
      {
        PveCardBase* pEffect = new PveCardGM(pCFG, pTarget);
        return pEffect;
      }
      break;
    case EPVECARDEFFECT_SUMMON:
      {
        PveCardBase* pEffect = new PveCardSummon(pCFG, pTarget);
        return pEffect;
      }
      break;
    case EPVECARDEFFECT_PVESUMMON:
      {
        PveCardBase* pEffect = new PveCardPveSummon(pCFG, pTarget);
        return pEffect;
      }
      break;
    case EPVECARDEFFECT_SCENEGM:
      {
        PveCardBase* pEffect = new PveCardSceneGM(pCFG, pTarget);
        return pEffect;
      }
      break;
    case EPVECARDEFFECT_ADDBUFF:
      {
        PveCardBase* pEffect = new PveCardAddBuff(pCFG, pTarget);
        return pEffect;
      }
      break;
    default:
      break;
  }

  return nullptr;
}

PveCardBase* PveCardManager::getCardEffect(DWORD cardid)
{
  auto it = m_mapPveCardEffects.find(cardid);
  if (it == m_mapPveCardEffects.end())
    return nullptr;
  return it->second;
}

//****************************************************************************
//*********************PveCardTarget******************************************
//****************************************************************************
PveCardTargetBase::PveCardTargetBase(const SPveCardEffectCFG* pCFG) : m_pEffectCFG(pCFG)
{

}

PveCardTargetBase::~PveCardTargetBase()
{

}

// 随机玩家
PveCardTargetRandomUser::PveCardTargetRandomUser(const SPveCardEffectCFG* pCFG) : PveCardTargetBase(pCFG)
{

}

PveCardTargetRandomUser::~PveCardTargetRandomUser()
{

}

bool PveCardTargetRandomUser::getTarget(DScene* pScene, xSceneEntrySet& targetSet)
{
  if (pScene == nullptr)
    return false;
  if (m_pEffectCFG == nullptr)
    return false;

  pScene->getAllEntryList(SCENE_ENTRY_USER, targetSet);

  // 血量低于 hp_less_limit
  if (m_pEffectCFG->oTargetParam.has("hp_less_limit"))
  {
    float per = m_pEffectCFG->oTargetParam.getTableFloat("hp_less_limit");
    for (auto s = targetSet.begin(); s != targetSet.end(); )
    {
      float maxhp = ((xSceneEntryDynamic*)(*s))->getAttr(EATTRTYPE_MAXHP);
      if (maxhp == 0)
        break;
      float hp = ((xSceneEntryDynamic*)(*s))->getAttr(EATTRTYPE_HP);
      if (hp / maxhp > per)
      {
        s = targetSet.erase(s);
        continue;
      }
      ++s;
    }
  }

  // 血量高于 hp_more_limit
  if (m_pEffectCFG->oTargetParam.has("hp_more_limit"))
  {
    float per = m_pEffectCFG->oTargetParam.getTableFloat("hp_more_limit");
    for (auto s = targetSet.begin(); s != targetSet.end(); )
    {
      float maxhp = ((xSceneEntryDynamic*)(*s))->getAttr(EATTRTYPE_MAXHP);
      if (maxhp == 0)
        break;
      float hp = ((xSceneEntryDynamic*)(*s))->getAttr(EATTRTYPE_HP);
      if (hp / maxhp < per)
      {
        s = targetSet.erase(s);
        continue;
      }
      ++s;
    }
  }

  if (m_pEffectCFG->oTargetParam.has("num"))
  {
    DWORD num = m_pEffectCFG->oTargetParam.getTableInt("num");
    if (num < targetSet.size())
    {
      xSceneEntrySet tmpset = targetSet;
      targetSet.clear();
      for (DWORD i = 0; i < num; ++i)
      {
        auto s = randomStlContainer(tmpset);
        if (!s)
          break;
        tmpset.erase(*s);
        targetSet.insert(*s);
      }
    }
  }

  return true;
}

// 全部玩家
PveCardTargetAllUser::PveCardTargetAllUser(const SPveCardEffectCFG* pCFG) : PveCardTargetBase(pCFG)
{

}

PveCardTargetAllUser::~PveCardTargetAllUser()
{

}

bool PveCardTargetAllUser::getTarget(DScene* pScene, xSceneEntrySet &targetSet)
{
  if (!pScene)
    return false;
  pScene->getAllEntryList(SCENE_ENTRY_USER, targetSet);
  return true;
}

//****************************************************************************
//*********************PveCardEffect******************************************
//****************************************************************************

PveCardBase::PveCardBase(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget) : m_pEffectCFG(pCFG), m_pTarget(pTarget)
{
  
}

PveCardBase::~PveCardBase()
{
  SAFE_DELETE(m_pTarget);
}

// pve summon
PveCardPveSummon::PveCardPveSummon(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget) : PveCardBase(pCFG, pTarget)
{

}

PveCardPveSummon::~PveCardPveSummon()
{

}

bool PveCardPveSummon::doEffect(DScene* pScene)
{
  PveCardScene* pCardScene = dynamic_cast<PveCardScene*> (pScene);
  if (pCardScene == nullptr)
    return false;
  if (m_pEffectCFG == nullptr)
    return false;

  NpcDefine def;
  def.load(m_pEffectCFG->oEffectParam);
  const xPos& pos = pCardScene->getCurCardPos();
  def.setPos(pos);

  DWORD num = 1;
  if (m_pEffectCFG->oEffectParam.has("num"))
    num = m_pEffectCFG->oEffectParam.getTableInt("num");

  SceneNpcManager::getMe().createNpc(pScene, def, num);
  XLOG << "[PveCardPveSummon], 招怪成功, 副本:" << pScene->name << pScene->id << "怪物:" << def.getID() << "数量:" << num << XEND;

  return true;
}

// summon
PveCardSummon::PveCardSummon(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget) : PveCardBase(pCFG, pTarget)
{

}

PveCardSummon::~PveCardSummon()
{

}

bool PveCardSummon::doEffect(DScene* pScene)
{
  if (m_pEffectCFG == nullptr)
    return false;

  NpcDefine def;
  def.load(m_pEffectCFG->oEffectParam);

  DWORD num = 1;
  if (m_pEffectCFG->oEffectParam.has("num"))
    num = m_pEffectCFG->oEffectParam.getTableInt("num");

  SceneNpcManager::getMe().createNpc(pScene, def, num);
  XLOG << "[PveCardSummon], 招怪成功, 副本:" << pScene->name << pScene->id << "怪物:" << def.getID() << "数量:" << num << XEND;

  return true;
}

// add buff
PveCardAddBuff::PveCardAddBuff(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget) : PveCardBase(pCFG, pTarget)
{

}

PveCardAddBuff::~PveCardAddBuff()
{

}

bool PveCardAddBuff::doEffect(DScene* pScene)
{
  if (!pScene || !m_pTarget || !m_pEffectCFG)
    return false;
  xSceneEntrySet targets;
  m_pTarget->getTarget(pScene, targets);

  if (targets.empty())
    return true;
  TSetDWORD buffids;
  xLuaData paramd = m_pEffectCFG->oEffectParam;
  paramd.getMutableData("id").getIDList(buffids);
  if (buffids.empty())
    return true;

  for (auto &s : targets)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
    if (pEntry == nullptr)
      continue;
    for (auto &d : buffids)
      pEntry->m_oBuff.add(d);
  }

  return true;
}

// gm
PveCardGM::PveCardGM(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget) : PveCardBase(pCFG, pTarget)
{

}

PveCardGM::~PveCardGM()
{

}

bool PveCardGM::doEffect(DScene* pScene)
{
  if (!pScene || !m_pTarget || !m_pEffectCFG)
    return false;
  xSceneEntrySet targets;
  m_pTarget->getTarget(pScene, targets);

  if (targets.empty())
    return true;

  for (auto &s : targets)
  {
    GMCommandRuler::getMe().execute((xSceneEntryDynamic*)s, m_pEffectCFG->oEffectParam);
  }

  return true;
}

// scene gm
PveCardSceneGM::PveCardSceneGM(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget) : PveCardBase(pCFG, pTarget)
{

}

PveCardSceneGM::~PveCardSceneGM()
{

}

bool PveCardSceneGM::doEffect(DScene* pScene)
{
  if (!pScene || !m_pEffectCFG)
    return false;

  return GMCommandRuler::getMe().scene_execute(pScene, m_pEffectCFG->oEffectParam);
}

