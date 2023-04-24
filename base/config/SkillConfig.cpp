#include "SkillConfig.h"

// config
SkillConfig::SkillConfig()
{

}

SkillConfig::~SkillConfig()
{

}

bool SkillConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Skill.txt"))
  {
    XERR << "[SkillConfig], 加载配置Table_Skill.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Skill", table);
  m_mapSkillCFG.clear();

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    auto o = m_mapSkillCFG.find(m->first);
    if (o != m_mapSkillCFG.end())
    {
      XERR << "[SkillConfig] id = " << m->first << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    // check type
    ESkillType eType = getSkillType(m->second.getTableString("SkillType"));
    if (eType == ESKILLTYPE_MIN)
    {
      XERR << "[SkillConfig] id = " << m->first << " SkillType = " << m->second.getTableString("SkillType") << " not valid!" << XEND;
      bCorrect = false;
      continue;
    }

    // logic
    ESkillLogic eLogic = getSkillLogic(m->second.getTableString("Logic"));

    // camp
    ESkillCamp eCamp = getSkillCamp(m->second.getTableString("Camps"));

    // create config
    SSkillCFG stCFG;
    stCFG.dwID = m->first;
    stCFG.eType = eType;
    stCFG.eLogic = eLogic;
    stCFG.eCamp = eCamp;
    stCFG.oParam = m->second;

    // insert to list
    m_mapSkillCFG[stCFG.dwID] = stCFG;
  }

  if (!xLuaTable::getMe().open("Lua/Table/Table_BuffStateOdds.txt"))
  {
    XERR << "[SkillConfig], 加载配置Table_BuffStateOdds.txt失败" << XEND;
    return false;
  }

  m_mapID2OddsImmune.clear();
  xLuaTableData efftable;
  xLuaTable::getMe().getLuaTable("Table_BuffStateOdds", efftable);
  for (auto m = efftable.begin(); m != efftable.end(); ++m)
  {
    auto it = m_mapID2OddsImmune.find(m->first);
    if (it != m_mapID2OddsImmune.end())
      continue;
    float odds = m->second.getTableFloat("Odds");
    m_mapID2OddsImmune[m->first] = odds;
  }
  if (bCorrect)
    XLOG << "[SkillConfig] 成功加载Table_Skill.txt" << XEND;
  return bCorrect;
}

const SSkillCFG* SkillConfig::getSkillCFG(DWORD dwSkillID) const
{
  auto m = m_mapSkillCFG.find(dwSkillID);
  if (m != m_mapSkillCFG.end())
    return &m->second;

  return nullptr;
}

ESkillType SkillConfig::getSkillType(const string& str)
{
  if (str == "Passive")
    return ESKILLTYPE_PASSIVE;
  else if (str == "Attack")
    return ESKILLTYPE_ACTIVE;
  else if (str == "Buff")
    return ESKILLTYPE_BUFF;
  else if (str == "Heal")
    return ESKILLTYPE_HEAL;
  else if (str == "Telesport")
    return ESKILLTYPE_TELESPORT;
  else if (str == "BowlingBash")
    return ESKILLTYPE_BOWLINGBASH;
  else if (str == "Purify")
    return ESKILLTYPE_PURIFY;
  else if (str == "Transport")
    return ESKILLTYPE_TRANSPORT;
  else if (str == "Collect")
    return ESKILLTYPE_COLLECT;
  else if (str == "Summon")
    return ESKILLTYPE_SUMMON;
  else if (str == "Suicide")
    return ESKILLTYPE_SUICIDE;
  else if (str == "Flash")
    return ESKILLTYPE_FLASH;
  else if (str == "FakeDead")
    return ESKILLTYPE_FAKEDEAD;
  else if (str == "Expel")
    return ESKILLTYPE_EXPEL;
  else if (str == "Reborn")
    return ESKILLTYPE_REBIRTH;
  else if (str == "TrapSkill")
    return ESKILLTYPE_TRAPSKILL;
  else if (str == "Repair")
    return ESKILLTYPE_REPAIR;
  else if (str == "BeatBack")
    return ESKILLTYPE_BEATBACK;
  else if (str == "Barrier")
    return ESKILLTYPE_TRAPBARRIER;
  else if (str == "TriggerTrap")
    return ESKILLTYPE_TRIGGERTRAP;
  else if (str == "StealSkill")
    return ESKILLTYPE_STEAL;
  else if (str == "Function")
    return ESKILLTYPE_FUNCTION;
  else if (str == "GroupReborn")
    return ESKILLTYPE_GRPREBIRTH;
  else if (str == "MarkHealSkill")
    return ESKILLTYPE_MARKHEAL;
  else if (str == "LeadSkill")
    return ESKILLTYPE_LEAD;
  else if (str == "TrapNpc")
    return ESKILLTYPE_TRAPNPCSKILL;
  else if (str == "SwordBreak")
    return ESKILLTYPE_SWORDBREAK;
  else if (str == "ShowSkill")
    return ESKILLTYPE_SHOWSKILL;
  else if (str == "TouchPet")
    return ESKILLTYPE_TOUCHPETSKILL;
  else if (str == "Eat")
    return ESKILLTYPE_SHOWSKILL;
  else if (str == "PoliAttack")
    return ESKILLTYPE_POLIATTACK;
  else if (str == "Blink")
    return ESKILLTYPE_BLINK;
  else if (str == "TrapMonster")
    return ESKILLTYPE_TRAPMONSTER;
  else if (str == "SummonBeing")
    return ESKILLTYPE_SUMMONBEING;
  else if (str == "ReviveBeing")
    return ESKILLTYPE_REVIVEBEING;
  else if (str == "HellPlant")
    return ESKILLTYPE_HELLPLANT;
  else if (str == "BeingBuff")
    return ESKILLTYPE_BEINGBUFF;
  else if (str == "UseBeingSkill")
    return ESKILLTYPE_USEBEINGSKILL;
  else if (str == "RandomSkill")
    return ESKILLTYPE_RANDOMSKILL;
  else if (str == "StealMoney")
    return ESKILLTYPE_STEALMONEY;
  else if (str == "Seize")
    return ESKILLTYPE_SEIZE;
  else if (str == "Control")
    return ESKILLTYPE_CONTROL;
  else if (str == "RemoveTrap")
    return ESKILLTYPE_REMOVETRAP;
  else if (str == "CopySkill")
    return ESKILLTYPE_COPYSKILL;
  else if (str == "SpaceLeap")
    return ESKILLTYPE_SPACELEAP;
  else if (str == "FastRestore")
    return ESKILLTYPE_FASTRESTORE;
  else if (str == "GoMap")
    return ESKILLTYPE_GOMAP;
  else if (str == "Wedding")
    return ESKILLTYPE_WEDDING;
  else if (str == "ClearEffect")
    return ESKILLTYPE_CLEAREFFECT;
  else if (str == "CursedCircle")
    return ESKILLTYPE_CURSEDCIRCLE;
  else if (str == "RideChange")
    return ESKILLTYPE_RIDECHANGE;
  else if (str == "Revive")
    return ESKILLTYPE_REVIVE;
  else if (str == "SummonElement")
    return ESKILLTYPE_SUMMONELEMENT;
  else if (str == "ElementTrap")
    return ESKILLTYPE_ELEMENTTRAP;
  else if (str == "Solo")
    return ESKILLTYPE_SOLO;
  else if (str == "Ensemble")
    return ESKILLTYPE_ENSEMBLE;
  else if (str == "StopConcert")
    return ESKILLTYPE_STOPCONCERT;
  return ESKILLTYPE_MIN;
}

ESkillLogic SkillConfig::getSkillLogic(const string& str)
{
  if (str == "SkillAddBuff")
    return ESKILLLOGIC_ADDBUFF;
  else if (str == "SkillForwardRect")
    return ESKILLLOGIC_FORWARDRECT;
  else if (str == "SkillLockedTarget" || str == "SkillStraightLine")
    return ESKILLLOGIC_LOCKEDTARGET;
  else if (str == "SkillSelfRange")
    return ESKILLLOGIC_SELFRANGE;
  else if (str == "SkillPointRange")
    return ESKILLLOGIC_POINTRANGE;
  else if (str == "SkillPointRangeIntervalEffective")
    return ESKILLLOGIC_POINTRANGEEFFECTIVE;
  else if (str == "SkillPointRect")
    return ESKILLLOGIC_POINTRECT;
  else if (str == "SkillMissile")
    return ESKILLLOGIC_MISSILE;
  else if (str == "SkillRandomRange")
    return ESKILLLOGIC_RANDOMRANGE;

  return ESKILLLOGIC_MIN;
}

ESkillCamp SkillConfig::getSkillCamp(const string& str)
{
  if (str == "Friend")
    return ESKILLCAMP_FRIEND;
  else if (str == "Enemy")
    return ESKILLCAMP_ENEMY;
  else if (str == "Team")
    return ESKILLCAMP_TEAM;

  return ESKILLCAMP_MIN;
}
