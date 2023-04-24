#include "XoclientConfig.h"
#include "xLuaTable.h"

XoclientConfig::XoclientConfig()
{
}

XoclientConfig::~XoclientConfig()
{
}

bool XoclientConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_xo.txt"))
  {
    XERR << "[表格],加载表格,Table_xo.txt,失败" << XEND;
    return false;
  }

  m_vecClientCfg.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_xo", table);
  m_mapxoclientCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapxoclientCFG.find(id);
    if (it == m_mapxoclientCFG.end())
    {
      m_mapxoclientCFG[id] = SXoclientCFG();
      it = m_mapxoclientCFG.find(id);
    }
    if (it == m_mapxoclientCFG.end())
    {
      XERR << "[Table_xo] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }
    SXoclientCFG& rCfg = it->second;
    rCfg.m_dwId = id;
    rCfg.m_dwType = m->second.getTableInt("Type");
    if (rCfg.m_dwType == 2)
      m_vecClientCfg.push_back(&rCfg);
  }

  bCorrect = loadServerConfig();

  if (bCorrect)
    XLOG << "[Table_xo], 成功加载表格Table_xo.txt" << XEND;
  return bCorrect;
}

bool XoclientConfig::loadServerConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_xo_server.txt"))
  {
    XERR << "[表格],加载表格,Table_xo_server.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_xo_server", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapxoclientCFG.find(id);   
    if (it == m_mapxoclientCFG.end())
    {
      XERR << "[Table_xo_server] id = " << id << " can not be found in Table_xo_server.txt" << XEND;
      bCorrect = false;
      continue;
    }
    SXoclientCFG& rCfg = it->second;
    rCfg.m_dwAnserId = m->second.getMutableData("Answer").getTableInt("1");  //都是单选 
  }

  if (bCorrect)
    XLOG << "[Table_xo_server], 成功加载表格Table_xo_server.txt" << XEND;
  return bCorrect;
}

const SXoclientCFG* XoclientConfig::getXoclientCFG(DWORD dwID) const
{
  auto m = m_mapxoclientCFG.find(dwID);
  if (m != m_mapxoclientCFG.end())
    return &m->second;

  return nullptr;
}

std::vector<SXoclientCFG*> XoclientConfig::getRandomXoclient(DWORD count)
{
  std::random_shuffle(m_vecClientCfg.begin(), m_vecClientCfg.end());
  std::vector<SXoclientCFG*> ret;
  
  DWORD c = 0;
  for (auto it = m_vecClientCfg.begin(); it != m_vecClientCfg.end(); ++it)
  {
    if (c++ < count)
      ret.push_back(*it);
    else 
      break;
  }  
  return ret;
}

