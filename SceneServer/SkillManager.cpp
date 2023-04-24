#include "SkillManager.h"

SkillManager::SkillManager()
{

}

SkillManager::~SkillManager()
{
  clear();
}

bool SkillManager::init()
{
  if (m_mapSkillItemCFG.empty() == false)
    return reload();

  bool bCorrect = true;
  clear();
  const TMapSkillCFG& mapCFG = SkillConfig::getMe().getSkillCFGList();
  for (auto m = mapCFG.begin(); m != mapCFG.end(); ++m)
  {
    // check duplicated
    auto o = m_mapSkillItemCFG.find(m->first);
    if (o != m_mapSkillItemCFG.end())
    {
      XERR << "[SkillManager::init] id :" << m->first << "duplicate" << XEND;
      bCorrect = false;
      continue;
    }

    // create skill item
    xLuaData oParam = m->second.oParam;
    BaseSkill* pSkillItem = createSkill(m->second.dwID, m->second.eType, m->second.eLogic, m->second.eCamp, oParam);
    if (pSkillItem == nullptr)
    {
      XERR << "[SkillManager::init] id :" << m->first << "create error!" << XEND;
      bCorrect = false;
      continue;
    }

    // insert to list
    m_mapSkillItemCFG[m->first] = pSkillItem;
  }

  return bCorrect;
}

BaseSkill* SkillManager::createSkill(DWORD dwID, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp, xLuaData& params)
{
  BaseSkill* pSkill = nullptr;

  switch(eType)
  {
    case ESKILLTYPE_MIN:
      break;
    case ESKILLTYPE_ACTIVE:
      {
        pSkill = NEW AttackSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_BUFF:
      {
        pSkill = NEW BuffSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_PASSIVE:
      {
        pSkill = NEW PassiveSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_HEAL:
      {
        pSkill = NEW HealSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_TELESPORT:
      {
        pSkill = NEW TelesportSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_BOWLINGBASH:
      {
        pSkill = NEW BowBashSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_PURIFY:
      /*{
        pSkill = NEW PurifySkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }*/
      break;
    case ESKILLTYPE_TRANSPORT:
      {
        pSkill = NEW TransportSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_COLLECT:
      {
        pSkill = NEW CollectSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SUMMON:
      {
        pSkill = NEW SummonSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SUICIDE:
      {
        pSkill = NEW SuicideSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_FLASH:
      {
        pSkill = NEW FlashSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_FAKEDEAD:
      {
        pSkill = NEW FakeDeadSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_EXPEL:
      {
        pSkill = NEW ExpelSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_REBIRTH:
      {
        pSkill = NEW ReBirthSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_TRAPSKILL:
      {
        pSkill = NEW TrapSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_REPAIR:
      {
        pSkill = NEW RepairSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_BEATBACK:
      {
        pSkill = NEW BeatBackSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_TRAPBARRIER:
      {
        pSkill = NEW TrapBarrierSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_TRIGGERTRAP:
      {
        pSkill = NEW TriggerTrapSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_STEAL:
      {
        pSkill = NEW StealSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_FUNCTION:
      {
        pSkill = NEW FunctionSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_GRPREBIRTH:
      {
        pSkill = NEW GroupBirthSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_MARKHEAL:
      {
        pSkill = NEW MarkHealSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_LEAD:
      {
        pSkill = NEW LeadSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_TRAPNPCSKILL:
      {
        pSkill = NEW TrapNpcSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SWORDBREAK:
      {
        pSkill = NEW SwordBreakSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SHOWSKILL:
      {
        pSkill = NEW ShowSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_TOUCHPETSKILL:
      {
        pSkill = NEW TouchPetSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_POLIATTACK:
      {
        pSkill = NEW PoliAttackSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_STEALMONEY:
      {
        pSkill = NEW StealMoneySkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SUMMONBEING:
      {
        pSkill = NEW SummonBeingSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SEIZE:
      {
        pSkill = NEW SeizeSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_BLINK:
      {
        pSkill = NEW BlinkSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_CONTROL:
      {
        pSkill = NEW ControlSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_REVIVEBEING:
      {
        pSkill = NEW ReviveBeingSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_REMOVETRAP:
      {
        pSkill = NEW RemoveTrapSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_TRAPMONSTER:
      {
        pSkill = NEW TrapMonsterSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_HELLPLANT:
      {
        pSkill = NEW HellPlantSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_BEINGBUFF:
      {
        pSkill = NEW BeingBuffSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_USEBEINGSKILL:
      {
        pSkill = NEW UseBeingSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_RANDOMSKILL:
      {
        pSkill = NEW RandomSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_COPYSKILL:
      {
        pSkill = NEW CopySkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_FASTRESTORE:
      {
        pSkill = new FastRestoreSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SPACELEAP:
      {
        pSkill = new SpaceLeapSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_GOMAP:
      {
        pSkill = new GoMapSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_WEDDING:
      {
        pSkill = new WeddingSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_CLEAREFFECT:
      {
        pSkill = NEW ClearEffectSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_CURSEDCIRCLE:
      {
        pSkill = NEW CursedCircleSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_RIDECHANGE:
      {
        pSkill = NEW RideChangeSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_REVIVE:
      {
        pSkill = NEW ReviveSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SUMMONELEMENT:
      {
        pSkill = NEW SummonElementSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_SOLO:
      {
        pSkill = NEW SoloSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_ELEMENTTRAP:
      {
        pSkill = NEW ElementTrapSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_ENSEMBLE:
      {
        pSkill = NEW EnsembleSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_STOPCONCERT:
      {
        pSkill = NEW StopConcertSkill(dwID, eType, eLogic, eCamp);
        if (pSkill == nullptr)
          return pSkill;
        if (pSkill->init(params) == false)
          SAFE_DELETE(pSkill);
      }
      break;
    case ESKILLTYPE_MAX:
      break;
  }

  return pSkill;
}

void SkillManager::clear()
{
  for (auto m = m_mapSkillItemCFG.begin(); m != m_mapSkillItemCFG.end(); ++m)
    SAFE_DELETE(m->second);
  m_mapSkillItemCFG.clear();
}

const BaseSkill* SkillManager::getSkillCFG(DWORD dwID) const
{
  auto m = m_mapSkillItemCFG.find(dwID);
  if (m != m_mapSkillItemCFG.end())
    return m->second;

  return nullptr;
}

bool SkillManager::reload()
{
  const TMapSkillCFG& mapCFG = SkillConfig::getMe().getSkillCFGList();
  for (auto &m : mapCFG)
  {
    auto it = m_mapSkillItemCFG.find(m.first);
    // NEW skill
    if (it == m_mapSkillItemCFG.end())
    {
      xLuaData oParam = m.second.oParam;
      BaseSkill* pSkillItem = createSkill(m.second.dwID, m.second.eType, m.second.eLogic, m.second.eCamp, oParam);
      if (pSkillItem == nullptr)
      {
        XERR << "[SkillManager-Reload] id :" << m.first << "create error!" << XEND;
        continue;
      }
      m_mapSkillItemCFG[m.first] = pSkillItem;
      XLOG << "[SkillManager-Reload], 新技能, id:" << m.first << "加载成功" << XEND;
    }
    // old skill
    else
    {
      BaseSkill* pSkill = it->second;
      xLuaData oParam = m.second.oParam;
      if (pSkill == nullptr || pSkill->init(oParam) == false)
      {
        XERR << "[SkillManager-Reload], 技能重加载, id:" << m.first << "加载失败" << XEND;
        continue;
      }
    }
  }
  XLOG << "[SkillManager-Reload], 技能重加载, 加载成功" << XEND;
  return true;
}

