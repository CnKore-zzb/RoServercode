#include "PortraitConfig.h"
#include "xLuaTable.h"

// config
PortraitConfig::PortraitConfig()
{

}

PortraitConfig::~PortraitConfig()
{

}

bool PortraitConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_HeadImage.txt"))
  {
    XERR << "[PortraitConfig],加载配置Table_HeadImage.txt失败" << XEND;
    return false;
  }

  m_vecPortraitCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_HeadImage", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    if (getPortraitCFG(m->first) != nullptr)
    {
      XERR << "[PortraitConfig] id : " << m->first << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    // create config
    SPortraitCFG stCFG;
    stCFG.dwID = m->first;

    DWORD type = m->second.getTableInt("Type");
    if (type <= EPORTRAITTYPE_MIN || type >= EPORTRAITTYPE_MAX)
    {
      XERR << "[PortraitConfig] id : " << m->first << " Type : " << type << " invalid" << XEND;
      bCorrect = false;
      continue;
    }
    stCFG.eType = static_cast<EPortraitType>(type);

    DWORD gender = m->second.getTableInt("sex");
    if (gender != 0)
    {
      if (gender <= EGENDER_MIN || gender >= EGENDER_MAX || EGender_IsValid(gender) == false)
      {
        XERR << "[PortraitConfig] id : " << m->first << " sex : " << gender << " invalid" << XEND;
        bCorrect = false;
        continue;
      }
      stCFG.eGender = static_cast<EGender>(gender);
    }

    // insert to list
    m_vecPortraitCFG.push_back(stCFG);
  }

  if (bCorrect)
    XLOG << "[PortraitConfig] 成功加载Table_HeadImage.txt" << XEND;
  return bCorrect;
}

const SPortraitCFG* PortraitConfig::getPortraitCFG(DWORD dwID) const
{
  auto v = find_if(m_vecPortraitCFG.begin(), m_vecPortraitCFG.end(), [dwID](const SPortraitCFG& r) -> bool{
    return r.dwID == dwID;
  });
  if (v != m_vecPortraitCFG.end())
    return &(*v);

  return nullptr;
}

