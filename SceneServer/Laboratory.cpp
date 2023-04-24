#include "Laboratory.h"
#include "xLuaTable.h"

std::map<DWORD, LaboratoryConfig> Laboratory::s_cfg;

Laboratory::Laboratory()
{
}

Laboratory::~Laboratory()
{
}

bool Laboratory::loadConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Laboratory.txt"))
  {
    XERR << "[LaboratoryConfig], 加载配置Table_Laboratory.txt失败" << XEND;
    return false;
  }

  s_cfg.clear();

  MinMax level;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Laboratory", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    const xLuaData &data = m->second.getData("Level");
    level.min = data.getTableInt("1");
    level.max = data.getTableInt("2");
    LaboratoryConfig &cfg = s_cfg[level.getID()];
    cfg.level = level;

    LaboratoryNpc lnpc;
    lnpc.m_dwGroupID = m->second.getTableInt("UniqueID");
    lnpc.m_dwNpcID = m->second.getTableInt("Monster");
    lnpc.m_dwPoint = m->second.getTableInt("Score");

    cfg.npclist[m->second.getTableInt("MonsterGroup")].push_back(lnpc);
  }

  XLOG << "[LaboratoryConfig],加载配置Table_Laboratory.txt成功,共加载:" << s_cfg.size() << XEND;
  return true;
}


