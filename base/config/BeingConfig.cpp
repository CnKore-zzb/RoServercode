#include "BeingConfig.h"
#include "xLuaTable.h"

BeingConfig::BeingConfig()
{
}

BeingConfig::~BeingConfig()
{
}

bool BeingConfig::loadConfig()
{
  bool bCorrect = true;

  if (loadBeingConfig() == false)
    bCorrect = false;
  if (loadBeingBaseLvConfig() == false)
    bCorrect = false;

  return bCorrect;
}

bool BeingConfig::checkConfig()
{
  bool bCorrect = true;
  return bCorrect;
}

bool BeingConfig::loadBeingConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Being.txt"))
  {
    XERR << "[生命体配置-效果] 加载配置Table_Being.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Being", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SBeingCFG& cfg = m_mapBeingCFG[m->first];
    cfg.dwID = m->first;
    cfg.strName = m->second.getTableString("Name");
    cfg.dwStaticSkill = m->second.getTableInt("StaticSkill");

    auto skillf = [&](xLuaData& data)
    {
      DWORD skillid = data.getTableInt("1");
      if (skillid <= 0)
        return;
      cfg.mapSkillID2UnlockLv[skillid] = data.getTableInt("2");
    };
    skillf(m->second.getMutableData("Skill_1"));
    skillf(m->second.getMutableData("Skill_2"));
    skillf(m->second.getMutableData("Skill_3"));
    skillf(m->second.getMutableData("Skill_4"));
  }

  if (bCorrect)
    XLOG << "[生命体配置] 成功加载配置Table_Being.txt" << XEND;
  return bCorrect;
}

bool BeingConfig::loadBeingBaseLvConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_BeingBaseLevel.txt"))
  {
    XERR << "[生命体等级信息配置] 加载配置Table_BeingBaseLevel.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  DWORD totalskillpoint = 0;
  xLuaTable::getMe().getLuaTable("Table_BeingBaseLevel", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SBeingBaseLvCFG& cfg = m_mapBeingBaseLvCFG[m->first];
    cfg.dwLv = m->first;
    cfg.qwExp = m->second.getTableQWORD("NeedExp");
    totalskillpoint += m->second.getTableInt("SkillPoint");
    cfg.dwSkillPoint = totalskillpoint;
  }

  if (bCorrect)
    XLOG << "[生命体等级信息配置] 成功加载配置Table_BeingBaseLevel.txt" << XEND;
  return bCorrect;
}

const SBeingCFG* BeingConfig::getBeingCFG(DWORD id) const
{
  auto it = m_mapBeingCFG.find(id);
  if (it == m_mapBeingCFG.end())
    return nullptr;
  return &it->second;
}

const SBeingBaseLvCFG* BeingConfig::getBeingBaseLvCFG(DWORD lv) const
{
  auto it = m_mapBeingBaseLvCFG.find(lv);
  if (it == m_mapBeingBaseLvCFG.end())
    return nullptr;
  return &it->second;
}
