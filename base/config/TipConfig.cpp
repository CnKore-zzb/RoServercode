#include "TipConfig.h"
#include "xLuaTable.h"
#include "MiscConfig.h"

// config
TipConfig::TipConfig()
{

}

TipConfig::~TipConfig()
{

}

bool TipConfig::loadConfig()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_RedTip.txt"))
  {
    XERR << "[RedTip], 加载Table_RedTip.txt失败" << XEND;
    return false;
  }

  m_vecTipCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_RedTip", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    if (MiscConfig::getMe().isForbid("RedTip", m->first))
      continue;

    DWORD id = m->first;
    if (id <= EREDSYS_MIN || id >= EREDSYS_MAX || ERedSys_IsValid(id) == false)
    {
      XERR << "[RedTipConfig] id = " << id << " invalid" << XEND;
      bCorrect = false;
      continue;
    }
    ERedSys eType = static_cast<ERedSys>(id);

    if (getTipConfig(eType) != nullptr)
    {
      XERR << "[RedTipConfig] id = " << id << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }
    DWORD localremote = m->second.getTableInt("LocalRemote");

    STipCFG stCFG;
    stCFG.eRedType = eType;
    stCFG.dwLocalRemote = localremote;

    m_vecTipCFG.push_back(stCFG);
  }

  XLOG << "[RedTipConfig] 成功加载Table_RedTip.txt" << XEND;
  return bCorrect;
}

bool TipConfig::checkConfig()
{
  return true;
}

const STipCFG* TipConfig::getTipConfig(ERedSys eType) const
{
  auto v = find_if(m_vecTipCFG.begin(), m_vecTipCFG.end(), [eType](const STipCFG& rCFG) -> bool{
    return rCFG.eRedType == eType;
  });
  if (v != m_vecTipCFG.end())
    return &(*v);

  return nullptr;
}

