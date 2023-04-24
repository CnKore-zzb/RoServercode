#include "UserConfig.h"
#include "xLuaTable.h"
#include "BaseConfig.h"
#include "RoleDataConfig.h"

// role base
bool SRoleBaseCFG::canExchange(EProfession eProfession) const
{
  auto v = find(vecAdvancePro.begin(), vecAdvancePro.end(), eProfession);
  return v != vecAdvancePro.end();
}

bool SRoleBaseCFG::haveSkill(DWORD id) const
{
  for (auto &v : vecEnableSkill)
  {
    auto vf = find_if(v.second.begin(), v.second.end(), [&id](const DWORD &d){
      return id / 1000 == d / 1000;
    });
    if (vf != v.second.end())
      return true;
  }
  return false;
}
bool SRoleBaseCFG::checkGender(Cmd::EGender gender) const
{
  if(Cmd::EGENDER_MIN == eGender)
    return true;

  return gender == eGender;
}

bool SRoleBaseCFG::addSkill(EProfession eProfession, DWORD id)
{
  auto v = find_if(vecEnableSkill.begin(), vecEnableSkill.end(), [eProfession](const TPairRoleSkill& r) -> bool{
    return r.first == eProfession;
  });
  if (v == vecEnableSkill.end())
  {
    TPairRoleSkill p;
    p.first = eProfession;
    p.second.push_back(id);
    vecEnableSkill.push_back(p);
    return true;
  }

  v->second.push_back(id);
  return true;
}

const SUserBaseLvCFG* BaseLevelConfig::getBaseLvCFG(DWORD lv)
{
  auto v = find_if(m_vecBaseLvCFG.begin(), m_vecBaseLvCFG.end(), [lv](const SUserBaseLvCFG& r) -> bool {
    return r.lv == lv;
  });
  if (v != m_vecBaseLvCFG.end())
    return &(*v);

  return NULL;
}

const SUserJobLvCFG* JobLevelConfig::getJobLvCFG(DWORD lv)
{
  auto v = find_if(m_vecJobLvCFG.begin(), m_vecJobLvCFG.end(), [lv](const SUserJobLvCFG& r) -> bool {
    return r.lv == lv;
  });
  if (v != m_vecJobLvCFG.end())
    return &(*v);

  return NULL;
}

const PAttr2Point* AttributePointConfig::getAttrPointCFG(DWORD value)
{
  auto v = find_if(m_vecAttrPoint.begin(), m_vecAttrPoint.end(), [value](const PAttr2Point& r) -> bool {
    return r.first == value;
  });
  if (v != m_vecAttrPoint.end())
    return &(*v);

  return NULL;
}

const SRoleBaseCFG* RoleConfig::getRoleBase(EProfession profession)
{
  auto v = find_if(m_vecRoleBaseCFG.begin(), m_vecRoleBaseCFG.end(), [profession](const SRoleBaseCFG& r) -> bool {
    return r.profession == profession;
  });
  if (v != m_vecRoleBaseCFG.end())
    return &(*v);

  return NULL;
}

EProfession RoleConfig::getBaseProfession(EProfession eProfession)
{
  switch (eProfession)
  {
    case EPROFESSION_MIN:
    case EPROFESSION_MAX:
    case EPROFESSION_PET:
      return EPROFESSION_MIN;
    case EPROFESSION_NOVICE:
      return EPROFESSION_NOVICE;
    case EPROFESSION_WARRIOR:
    case EPROFESSION_KNIGHT:
    case EPROFESSION_LORDKNIGHT:
    case EPROFESSION_RUNEKNIGHT:
    case EPROFESSION_CRUSADER:
    case EPROFESSION_PALADIN:
    case EPROFESSION_ROYALGUARD:
      return EPROFESSION_WARRIOR;
    case EPROFESSION_MAGICIAN:
    case EPROFESSION_WIZARD:
    case EPROFESSION_HIGHWIZARD:
    case EPROFESSION_WARLOCK:
    case EPROFESSION_SAGE:
    case EPROFESSION_PROFESSOR:
    case EPROFESSION_SORCERER:
      return EPROFESSION_MAGICIAN;
    case EPROFESSION_THIEF:
    case EPROFESSION_ASSASSIN:
    case EPROFESSION_ASSASSINCROSS:
    case EPROFESSION_GUILLOTINECROSS:
    case EPROFESSION_ROGUE:
    case EPROFESSION_STALKER:
    case EPROFESSION_SHADOWCHASER:
      return EPROFESSION_THIEF;
    case EPROFESSION_ARCHER:
    case EPROFESSION_HUNTER:
    case EPROFESSION_SNIPER:
    case EPROFESSION_RANGER:
    case EPROFESSION_BARD:
    case EPROFESSION_CLOWN:
    case EPROFESSION_MINSTREL:
    case EPROFESSION_DANCER:
    case EPROFESSION_GYPSY:
    case EPROFESSION_WANDERER:
      return EPROFESSION_ARCHER;
    case EPROFESSION_ACOLYTE:
    case EPROFESSION_PRIEST:
    case EPROFESSION_HIGHPRIEST:
    case EPROFESSION_ARCHBISHOP:
    case EPROFESSION_MONK:
    case EPROFESSION_CHAMPION:
    case EPROFESSION_SHURA:
      return EPROFESSION_ACOLYTE;
    case EPROFESSION_MERCHANT:
    case EPROFESSION_BLACKSMITH:
    case EPROFESSION_WHITESMITH:
    case EPROFESSION_MECHANIC:
    case EPROFESSION_ALCHEMIST:
    case EPROFESSION_CREATOR:
    case EPROFESSION_GENETIC:
      return EPROFESSION_MERCHANT;
  }

  return EPROFESSION_MIN;
}

EProfession RoleConfig::getTypeProfession(EProfession eProfession)
{
  switch (eProfession)
  {
    case EPROFESSION_MIN:
    case EPROFESSION_MAX:
    case EPROFESSION_NOVICE:
    case EPROFESSION_WARRIOR:
    case EPROFESSION_MAGICIAN:
    case EPROFESSION_THIEF:
    case EPROFESSION_ARCHER:
    case EPROFESSION_ACOLYTE:
    case EPROFESSION_MERCHANT:
    case EPROFESSION_PET:
      return EPROFESSION_MIN;
    case EPROFESSION_KNIGHT:
    case EPROFESSION_LORDKNIGHT:
    case EPROFESSION_RUNEKNIGHT:
      return EPROFESSION_KNIGHT;
    case EPROFESSION_CRUSADER:
    case EPROFESSION_PALADIN:
    case EPROFESSION_ROYALGUARD:
      return EPROFESSION_CRUSADER;
    case EPROFESSION_WIZARD:
    case EPROFESSION_HIGHWIZARD:
    case EPROFESSION_WARLOCK:
      return EPROFESSION_WIZARD;
    case EPROFESSION_SAGE:
    case EPROFESSION_PROFESSOR:
    case EPROFESSION_SORCERER:
      return EPROFESSION_SAGE;
    case EPROFESSION_ASSASSIN:
    case EPROFESSION_ASSASSINCROSS:
    case EPROFESSION_GUILLOTINECROSS:
      return EPROFESSION_ASSASSIN;
    case EPROFESSION_ROGUE:
    case EPROFESSION_STALKER:
    case EPROFESSION_SHADOWCHASER:
      return EPROFESSION_ROGUE;
    case EPROFESSION_HUNTER:
    case EPROFESSION_SNIPER:
    case EPROFESSION_RANGER:
      return EPROFESSION_HUNTER;
    case EPROFESSION_BARD:
    case EPROFESSION_CLOWN:
    case EPROFESSION_MINSTREL:
      return EPROFESSION_BARD;
    case EPROFESSION_DANCER:
    case EPROFESSION_GYPSY:
    case EPROFESSION_WANDERER:
      return EPROFESSION_DANCER;
    case EPROFESSION_PRIEST:
    case EPROFESSION_HIGHPRIEST:
    case EPROFESSION_ARCHBISHOP:
      return EPROFESSION_PRIEST;
    case EPROFESSION_MONK:
    case EPROFESSION_CHAMPION:
    case EPROFESSION_SHURA:
      return EPROFESSION_MONK;
    case EPROFESSION_BLACKSMITH:
    case EPROFESSION_WHITESMITH:
    case EPROFESSION_MECHANIC:
      return EPROFESSION_BLACKSMITH;
    case EPROFESSION_ALCHEMIST:
    case EPROFESSION_CREATOR:
    case EPROFESSION_GENETIC:
      return EPROFESSION_ALCHEMIST;
  }

  return EPROFESSION_MIN;
}

void RoleConfig::getAdvanceProPath(EProfession pro, TVecProfession& path)
{
  if (path.size() > 10) // 简单防止出现环
    return;
  const SRoleBaseCFG* pCfg = getRoleBase(pro);
  if (pCfg == nullptr)
    return;
  path.push_back(pro);
  getAdvanceProPath(pCfg->ePreProfession, path);
}

RoleConfig::RoleConfig()
{

}

RoleConfig::~RoleConfig()
{

}

bool RoleConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Class.txt"))
  {
    XERR << "[RoleConfig],加载配置Table_Class.txt失败" << XEND;
    return false;
  }

  m_vecRoleBaseCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Class", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SRoleBaseCFG stCFG;

    // profession
    DWORD profession = m->first;
    if (profession <= EPROFESSION_MIN || profession >= EPROFESSION_MAX || EProfession_IsValid(profession) == false)
    {
      //XERR("[Table_Class] id = %d invalid profession!", m->first);
      //bCorrect = false;
      continue;
    }
    stCFG.profession = static_cast<EProfession>(profession);

    // check duplicated
    if (getRoleBase(stCFG.profession) != NULL)
    {
      XERR << "[Table_Class] id = " << m->first << " duplicated!" << XEND;
      bCorrect = false;
      continue;
    }

    // advanceprofession
    xLuaData& advancePro = m->second.getMutableData("AdvanceClass");
    auto advanceProf = [&](const string& key, xLuaData& data)
    {
      DWORD pro = data.getInt();
      if (pro <= EPROFESSION_MIN || pro >= EPROFESSION_MAX || EProfession_IsValid(pro) == false)
      {
        XERR << "[Table_Class] id = " << m->first << " advancepro = " << pro << " invalid!" << XEND;
        return;
      }
      stCFG.vecAdvancePro.push_back(static_cast<EProfession>(pro));
    };
    advancePro.foreach(advanceProf);

    stCFG.maleBody = m->second.getTableInt("MaleBody");
    stCFG.femaleBody = m->second.getTableInt("FemaleBody");
    stCFG.maleEye = m->second.getTableInt("MaleEye");
    stCFG.femaleEye = m->second.getTableInt("FemaleEye");
    stCFG.defaultWeapon = m->second.getTableInt("DefaultWeapon");
    stCFG.maxJobLv = m->second.getTableInt("MaxJobLevel");
    stCFG.maxSkillPos = m->second.getTableInt("UnlockNum");
    stCFG.dwType = m->second.getTableInt("Type");
    stCFG.dwTypeBranch = m->second.getTableInt("TypeBranch");
    stCFG.dwPeakJobLv = m->second.getTableInt("MaxPeak");
    stCFG.eGender = static_cast<Cmd::EGender>(m->second.getTableInt("gender"));

    // base skill
    xLuaData& normalskill = m->second.getMutableData("NormalAttack");
    stCFG.normalSkill = normalskill.getTableInt("1");
    stCFG.strengthSkill = normalskill.getTableInt("2");

    // base enable skill
    xLuaData& enableskill = m->second.getMutableData("Skill");
    auto enableskillf = [&](const string& key, xLuaData& data)
    {
      stCFG.addSkill(stCFG.profession, data.getInt());
    };
    enableskill.foreach(enableskillf);

    // unlock
    xLuaData& unlock = m->second.getMutableData("Clear");
    auto unlockf = [&](const string& key, xLuaData& data)
    {
      SRoleUnlockCFG cfg;
      cfg.dwJobLv = data.getTableInt("job");

      if (data.has("type") == true)
      {
        xLuaData& mail = data.getMutableData("mail");
        cfg.dwMaleMailID = mail.getTableInt("male");
        cfg.dwFemaleMailID = mail.getTableInt("female");
      }
      else
      {
        cfg.dwMaleMailID = cfg.dwFemaleMailID = data.getTableInt("mail");
      }
      stCFG.vecUnlock.push_back(cfg);
    };
    unlock.foreach(unlockf);

    // damage random params
    xLuaData& damparam = m->second.getMutableData("DamRandom");
    TVecFloat tempVec;
    auto damf = [&](const string& key, xLuaData& data)
    {
      tempVec.push_back(data.getFloat());
    };
    damparam.foreach(damf);
    if (tempVec.size() != 2)
    {
      XERR << "[Table_Class], DamRandom 配置错误" << XEND;
      bCorrect = false;
      continue;
    }
    else if (tempVec[0] == 0 && tempVec[1] == 0)
    {
      XERR << "[Table_Class], DamRandom 配置错误" << XEND;
      bCorrect = false;
      continue;
    }
    stCFG.damRandom.first = tempVec[0];
    stCFG.damRandom.second = tempVec[1];

    // name
    stCFG.strName = m->second.getTableString("NameZh");

    // attr
    xLuaData& attr = m->second.getMutableData("UnlockAttr");
    auto collectAttr = [&](std::string key, xLuaData& data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        XERR << "[UserConfig-class] id : " << m->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      stCFG.vecUnlockAttr.push_back(oAttr);
    };
    attr.foreach(collectAttr);

    // ride action
    stCFG.bRideAction = m->second.getTableInt("RideAction") == 1;

    // insert list
    m_vecRoleBaseCFG.push_back(stCFG);
  }

  // include base skill
  for (auto v = m_vecRoleBaseCFG.begin(); v != m_vecRoleBaseCFG.end(); ++v)
  {
    if (v->profession == EPROFESSION_NOVICE)
      continue;

    for (auto p = v->vecAdvancePro.begin(); p != v->vecAdvancePro.end(); ++p)
    {
      EProfession advance = *p;
      auto o = find_if(m_vecRoleBaseCFG.begin(), m_vecRoleBaseCFG.end(), [advance](const SRoleBaseCFG& r) -> bool {
        return r.profession == advance;
      });
      if (o == m_vecRoleBaseCFG.end())
        continue;

      for (auto s = v->vecEnableSkill.begin(); s != v->vecEnableSkill.end(); ++s)
      {
        for (auto ss = s->second.begin(); ss != s->second.end(); ++ss)
          o->addSkill(s->first, *ss);
      }
      for (auto s = o->vecUnlock.begin(); s != o->vecUnlock.end(); ++s)
        s->dwJobLv += v->maxJobLv;
    }
  }

  for (auto v = m_vecRoleBaseCFG.begin(); v != m_vecRoleBaseCFG.end(); ++v)
  {
    for (auto next : v->vecAdvancePro)
    {
      const SRoleBaseCFG* pCfg = getRoleBase(next);
      if (pCfg == nullptr || pCfg->ePreProfession != EPROFESSION_MIN)
      {
        XERR << "[Table_Class] id:" << v->profession << "advancepro:" << next << "前置职业已存在" << XEND;
        bCorrect = false;
        continue;
      }
      const_cast<SRoleBaseCFG*>(pCfg)->ePreProfession = v->profession;
    }
  }

  if(!loadProfessionConfig())
    bCorrect = false;

  if (bCorrect)
    XLOG << "[Table_Class] 成功加载Table_Class.txt" << XEND;
  return bCorrect;
}

bool RoleConfig::loadProfessionConfig()
{
  if(!xLuaTable::getMe().open("Lua/Table/Table_Branch.txt"))
  {
    XERR << "[RoleConfig],加载配置Table_Branch.txt失败" << XEND;
    return false;
  }

  m_mapBranchCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Branch", table);
  for(auto m = table.begin(); m != table.end(); ++m)
  {
    SBranchCFG stBranchCFG;
    stBranchCFG.id = m->second.getTableInt("id");
    stBranchCFG.baseId = m->second.getTableInt("base_id");
    stBranchCFG.itemId = m->second.getTableInt("item");
    stBranchCFG.taskProfession = m->second.getTableInt("task_profession");
    stBranchCFG.taskPeak = m->second.getTableInt("peak_task");
    stBranchCFG.professionPeak = m->second.getTableInt("peak_profession");
    stBranchCFG.giftBranch = m->second.getTableInt("gift_branch");
    stBranchCFG.eGender = static_cast<Cmd::EGender>(m->second.getTableInt("gender"));

    string strProfessions = m->second.getTableString("profession_list");
    numTok(strProfessions, ",", stBranchCFG.vecProfession);
    string strDelTasks = m->second.getTableString("remove_task_list");
    numTok(strDelTasks, ",", stBranchCFG.vecDelTask);
    string strJobSkills = m->second.getTableString("job_skill_list");
    numTok(strJobSkills, ",", stBranchCFG.vecJobSkill);
    string strBrothers = m->second.getTableString("brother_list");
    numTok(strBrothers, ",", stBranchCFG.vecBrotherId);

    m_mapBranchCFG[stBranchCFG.id] = stBranchCFG;
  }

  return true;
}

const SBranchCFG* RoleConfig::getBranchCFG(DWORD id)
{
  auto item = m_mapBranchCFG.find(id);
  if(m_mapBranchCFG.end() == item)
    return nullptr;
  return &(item->second);
}

AttributePointConfig::AttributePointConfig()
{

}

AttributePointConfig::~AttributePointConfig()
{

}

bool AttributePointConfig::loadConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_AttributePoint.txt"))
  {
    XERR << "[AttributePointConfig],加载配置Table_AttributePoint.txt失败" << XEND;
    return false;
  }

  m_vecAttrPoint.clear();
  LuaMultiTable table;
  xLuaTable::getMe().getMultiTable("Table_AttributePoint", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    PAttr2Point attr;

    attr.first = atoi(m->first.c_str());
    attr.second = atoi(m->second["NeedPoint"].c_str());
    if(attr.second == 0)
    {
      XERR << "[AttributePointConfig],加载配置Table_AttributePoint.txt失败,点数为0" << attr.first << attr.second << XEND;
      return false;
    }

    m_vecAttrPoint.push_back(attr);
  }

  return true;
}

BaseLevelConfig::BaseLevelConfig()
{

}

BaseLevelConfig::~BaseLevelConfig()
{

}

bool BaseLevelConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_BaseLevel.txt"))
  {
    XERR << "[BaseLevelConfig],加载配置Table_BaseLevel.txt失败" << XEND;
    return false;
  }

  m_vecBaseLvCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_BaseLevel", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    if (getBaseLvCFG(m->first) != nullptr)
    {
      XERR << "[BaseLevel] lv = " << m->first << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    // create config
    SUserBaseLvCFG stCFG;

    stCFG.lv = m->first;
    stCFG.needExp = m->second.getTableInt("NeedExp");
    stCFG.point = m->second.getTableInt("AddPoint");

    m_vecBaseLvCFG.push_back(stCFG);
  }

  if (bCorrect)
    XLOG << "[BaseLevelConfig] 成功加载Table_BaseLevel.txt" << XEND;
  return bCorrect;
}

JobLevelConfig::JobLevelConfig()
{

}

JobLevelConfig::~JobLevelConfig()
{

}

bool JobLevelConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_JobLevel.txt"))
  {
    XERR << "[JobLevelConfig],加载配置Table_JobLevel.txt失败" << XEND;
    return false;
  }

  m_vecJobLvCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_JobLevel", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    if (getJobLvCFG(m->first) != nullptr)
    {
      XERR << "[JobLevel] lv = " << m->first << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    // create config
    SUserJobLvCFG stCFG;

    // need exp
    stCFG.lv = m->first;
    DWORD dwNeedExp = m->second.getTableInt("JobExp");
    if(BaseConfig::getMe().getInnerBranch() == BRANCH_TF)
      stCFG.needExp = std::ceil(dwNeedExp/3);
    else
      stCFG.needExp = dwNeedExp;

    m_vecJobLvCFG.push_back(stCFG);
  }

  if (bCorrect)
    XLOG << "[JobLevel] 成功加载Table_JobLevel" << XEND;
  return bCorrect;
}

bool RoleConfig::isNormalSkill(DWORD skillid)
{
  auto v = find_if(m_vecRoleBaseCFG.begin(), m_vecRoleBaseCFG.end(), [&skillid](const SRoleBaseCFG& r) ->bool{
      return r.normalSkill == skillid;
      });
  return v != m_vecRoleBaseCFG.end();
}

bool RoleConfig::isStrengthSkill(DWORD skillid)
{
  auto v = find_if(m_vecRoleBaseCFG.begin(), m_vecRoleBaseCFG.end(), [&skillid](const SRoleBaseCFG& r) ->bool{
      return r.strengthSkill == skillid;
      });
  return v != m_vecRoleBaseCFG.end();
}

bool RoleConfig::isFirstProfession(EProfession profession)
{
  const SRoleBaseCFG* pCfg = getRoleBase(EPROFESSION_NOVICE);
  if(pCfg == nullptr)
    return false;

  for(auto &s : pCfg->vecAdvancePro)
  {
    if(s == profession)
      return true;
  }

  return false;
}

DWORD RoleConfig::getProfessionNum(std::set<EProfession>& setPro)
{
  DWORD size = setPro.size();
  if (size == 1)
    return 1;
  if (size == 2)
  {
    EProfession p1 = *(setPro.begin());
    EProfession p2 = *(setPro.rbegin());

    EProfession pb1 = getBaseProfession(p1);
    EProfession pb2 = getBaseProfession(p2);
    if (pb1 != pb2)
    {
      return 2;
    }
    else
    {
      DWORD n1 = (DWORD)p1;
      DWORD n2 = (DWORD)p2;
      // 有基础职业
      if (n1 % 10 == 1 || n2 % 10 == 1)
        return 1;
      // 无基础职业判断是否不同分支
      else
        return n1 / 10 == n2 / 10 ? 1 : 2;
    }
  }

  // 职业数大于2时
  TSetDWORD setbranch;
  for (auto s = setPro.begin(); s != setPro.end(); )
  {
    DWORD n = (DWORD)(*s);
    if (n == 1  || n % 10 == 1)
    {
      setbranch.insert(n);
      s = setPro.erase(s);
      continue;
    }
    ++s;
  }

  for (auto &s : setPro)
  {
    DWORD b = (DWORD)getBaseProfession(s);
    if (setbranch.find(b) != setbranch.end())
    {
      DWORD n = (DWORD)s / 10 + 2;
      setbranch.erase(b);
      setbranch.insert(n);
    }
    else
    {
      DWORD n = (DWORD)s / 10 + 2;
      setbranch.insert(n);
    }
  }

  return setbranch.size();
}
