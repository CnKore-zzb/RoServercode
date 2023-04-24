#include "AvatarConfig.h"
#include "xLuaTable.h"
#include "SkillConfig.h"

AvatarConfig::AvatarConfig()
{

}

AvatarConfig::~AvatarConfig()
{

}

bool AvatarConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Avatar.txt"))
  {
    XERR << "[表情配置-加载],加载配置Table_Avatar.txt失败" << XEND;
    return false;
  }

  m_mapAvatarCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Avatar", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    if (m->first <= EAVATAREXPRESSION_MIN || m->first >= EAVATAREXPRESSION_MAX)
    {
      bCorrect = false;
      XERR << "[表情配置-加载] id : " << m->first << " 不合法的表情" << XEND;
      continue;
    }

    SAvatarCFG stCFG;
    stCFG.eExpression = static_cast<EAvatarExpression>(m->first);
    stCFG.dwRate = m->second.getTableInt("Probability");
    stCFG.dwAdventureSkill = m->second.getTableInt("Adventureskills");

    m_mapAvatarCFG[stCFG.eExpression] = stCFG;
  }

  if (bCorrect)
    XLOG << "[表情配置-加载] 成功加载Table_Avatar.txt配置" << XEND;
  return bCorrect;
}

bool AvatarConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto &m : m_mapAvatarCFG)
  {
    if (m.second.dwAdventureSkill != 0 && SkillConfig::getMe().getSkillCFG(m.second.dwAdventureSkill) == nullptr)
    {
      bCorrect = false;
      XERR << "[表情配置-检查] id : " <<  m.first << " skillid : " << m.second.dwAdventureSkill << " 未在 Table_Skill.txt 表中找到" << XEND;
    }
  }

  return bCorrect;
}

const SAvatarCFG* AvatarConfig::getAvatarCFG(EAvatarExpression eExpression) const
{
  auto m = m_mapAvatarCFG.find(eExpression);
  if (m != m_mapAvatarCFG.end())
    return &m->second;

  return nullptr;
}

