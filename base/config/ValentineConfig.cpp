#include "ValentineConfig.h"
#include "xLuaTable.h"
#include "xTime.h"
#include "MiscConfig.h"

const SValentineCFG* SValentineCFG::getNextOption(DWORD a1, DWORD a2)const
{
  DWORD next = 0;
  if (a1 == 1 and a2 == 1)        //AA
    next = 0;
  else if (a1 == 1 and a2 == 2)   //AB
    next = 1;
  else if (a1 == 2 and a2 == 1)   //BA 
    next = 2;
  else                            //BB
    next = 3;

  if (m_vecOption.size() <= next)
    return nullptr;
  
  DWORD option = m_vecOption[next];

  const SValentineCFG* pCfg = ValentineConfig::getMe().getCfg(m_auguryType, m_dwMonth, option);
  return pCfg;
}

ValentineConfig::ValentineConfig()
{
}

ValentineConfig::~ValentineConfig()
{
}

bool ValentineConfig::loadConfig()
{
  bool bCorrect = true;
  m_mapValentineCFG.clear();
  m_mapVecFirst.clear();
  
  const SAuguryCFG& rCFG = MiscConfig::getMe().getAuguryCfg();
  for (auto it = rCFG.m_typeConfig.begin(); it != rCFG.m_typeConfig.end(); ++it)
  {
    if (monthTable(it->first))
    {
      for (int month = 1; month <= 12; ++month)
      {
        if (loadSubConfig(it->first, it->second.strTableName, month) == false)
        {
          bCorrect = false;
          XERR << "[占卜配置] 加载配置" << it->second.strTableName.c_str() << ".txt失败" << XEND;
        }
      }
    }
    else
    {
      if (loadSubConfig(it->first, it->second.strTableName, 0) == false)
      {
        bCorrect = false;
        XERR << "[占卜配置] 加载配置" << it->second.strTableName.c_str() << ".txt失败" << XEND;
      }
    }
  }
  return bCorrect;
}

bool ValentineConfig::loadSubConfig(DWORD type,  const string& tb, DWORD month)
{ 
  bool bCorrect = true;
  // get path
  string tableName = tb;
  if (monthTable(type))
  {
    stringstream ss;
    ss << tb << "_" << month;    //Table_Augury_2_1.excel
    tableName = ss.str();
  }
  else
  {
    month = 0;
  }

  string path = "Lua/Table/" + tableName + ".txt";
  // load quest config
  if (!xLuaTable::getMe().open(path.c_str()))
  {
    XERR << "[占卜配置] 加载配置" <<month << path << tableName.c_str() <<".txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable(tableName.c_str(), table);
  
  std::map<DWORD, SValentineCFG>& rMapValentineCFG = m_mapValentineCFG[type][month];
  rMapValentineCFG.clear();
  std::vector<DWORD>& rVecFirst = m_mapVecFirst[type][month];
  rVecFirst.clear();
  rVecFirst.reserve(10);

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = rMapValentineCFG.find(id);
    if (it == rMapValentineCFG.end())
    {
      rMapValentineCFG[id] = SValentineCFG();
      it = rMapValentineCFG.find(id);
    }
    if (it == rMapValentineCFG.end())
    {
      XERR << "[占卜配置] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }
    SValentineCFG& rCfg = it->second;
    rCfg.m_dwId = id;
    rCfg.m_type = static_cast<EValentineType>(m->second.getTableInt("Type"));
    rCfg.m_auguryType = type;
    rCfg.m_vecOption.reserve(4);
    rCfg.m_vecOption.push_back(m->second.getTableInt("Option1"));
    rCfg.m_vecOption.push_back(m->second.getTableInt("Option2"));
    rCfg.m_vecOption.push_back(m->second.getTableInt("Option3"));
    rCfg.m_vecOption.push_back(m->second.getTableInt("Option4"));

    rCfg.m_effect = m->second.getTableString("Effect");
    rCfg.m_action = m->second.getTableString("Action");
    rCfg.m_dwMonth = month;

    auto func = [&](const string& key, xLuaData& data)
    {
      rCfg.m_vecReward.push_back(data.getInt());
    };
    m->second.getMutableData("Reward").foreach(func);

    if (rCfg.m_type == EValentineType_First)
    {
      rVecFirst.push_back(id);
    }
  }

  if (bCorrect)
    XLOG << "[占卜配置], 成功加载表格" << month << path << tableName << XEND;
  return bCorrect;
}

bool ValentineConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto it = m_mapValentineCFG.begin(); it != m_mapValentineCFG.end(); ++it)
  {
    for (auto m : it->second)
    {
      for (auto m2 : m.second)
      {
        for (auto v : m2.second.m_vecOption)
        {
          if (v != 0)
          {
            const SValentineCFG* pCfg = getCfg(m2.second.m_type, m2.second.m_dwMonth, v);
            if (pCfg == nullptr)
            {
              bCorrect = false;
              XERR << "[占卜配置], type" << m2.second.m_type << "当前问题" << m2.second.m_dwId<<"month" << m2.second.m_dwMonth << "的选项在配置表中找不到" << v << XEND;
            }
          }
        }
      }     
    }
  }
  return bCorrect;
}

const SValentineCFG* ValentineConfig::getFirstTitleCfg(DWORD type, DWORD month)
{
  if (!monthTable(type))
    month = 0;

  auto it = m_mapVecFirst.find(type);
  if (it == m_mapVecFirst.end())
    return nullptr;
  
  auto it2 = it->second.find(month);
  if (it2 == it->second.end())
    return nullptr;
    
  DWORD* p = randomStlContainer(it2->second);
  if (p == nullptr)
    return nullptr;
  
  return getCfg(type, month, *p);
}

const SValentineCFG* ValentineConfig::getCfg(DWORD type,DWORD month, DWORD id)
{
  if (!monthTable(type))
    month = 0;

  auto it1 = m_mapValentineCFG.find(type);
  if (it1 == m_mapValentineCFG.end())
    return nullptr;
  
  auto it2 = it1->second.find(month);
  if (it2 == it1->second.end())
    return nullptr;

  auto it3 = it2->second.find(id);
  if (it3 == it2->second.end())
    return nullptr;
  return &(it3->second);
}

bool ValentineConfig::monthTable(DWORD type)
{
  if (type == 2)    //星座占卜
    return true;

  return false;
}
