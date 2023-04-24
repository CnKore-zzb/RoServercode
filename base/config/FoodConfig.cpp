#include "FoodConfig.h"
#include "ItemConfig.h"
#include "xTools.h"

FoodConfig::FoodConfig()
{
}

FoodConfig::~FoodConfig()
{
}

bool FoodConfig::loadConfig()
{
  bool bCorrect = true;
  if (loadFoodConfig() == false)
    bCorrect = false;

  return bCorrect;
}

bool FoodConfig::checkConfig()
{
  bool bCorrect = true;
  
  for (auto it = m_mapFoodCfg.begin(); it != m_mapFoodCfg.end(); ++it)
  {
  }
  return bCorrect;
}

bool FoodConfig::loadFoodConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Food.txt"))
  {
    XERR << "[表格],加载表格,Table_Food.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Food", table);
  m_mapFoodCfg.clear();
  m_mapNpc2Food.clear();

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapFoodCfg.find(id);
    if (it == m_mapFoodCfg.end())
    {
      m_mapFoodCfg[id] = SFoodConfg();
      it = m_mapFoodCfg.find(id);
    }
    if (it == m_mapFoodCfg.end())
    {
      XERR << "[Table_Food] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }
    SFoodConfg& rCfg = it->second;
    rCfg.m_dwId = id;
    rCfg.m_strName = m->second.getTableString("Name");
    rCfg.m_dwCookLv = m->second.getTableInt("CookLevel");
    rCfg.m_dwCookHard = m->second.getTableInt("CookHard");
    rCfg.m_dwCookerExp = m->second.getTableInt("CookerExp");
    rCfg.m_dwTasterExp = m->second.getTableInt("TasterExp");
    rCfg.m_dwFullProgress = m->second.getTableInt("FullProgress");
    rCfg.m_height = m->second.getTableInt("Height");
    rCfg.m_weight = m->second.getTableInt("Weight");
    rCfg.m_dwSaveHP = m->second.getTableInt("SaveHP");
    rCfg.m_dwSaveSP = m->second.getTableInt("SaveSP");
    rCfg.m_dwHealHP = m->second.getTableInt("HealHP");
    rCfg.m_dwHealSP = m->second.getTableInt("HealSP");
    rCfg.m_dwDuration = m->second.getTableInt("Duration");
    rCfg.m_dwNpcId = m->second.getTableInt("NpcId");
    rCfg.m_bMultiEat = m->second.getTableInt("MultiEat");
    rCfg.m_dwTotalStep = m->second.getTableInt("TotalStep");
    rCfg.m_dwStepDuration = m->second.getTableInt("StepDuration");
    rCfg.m_dwHatReward = m->second.getTableInt("HatReward");
    auto f = [&](std::string key, xLuaData &data)
    {
      DWORD buffId = data.getInt();
      rCfg.m_vecBuff.push_back(buffId);     
    };     
    m->second.getMutableData("BuffEffect").foreach(f);

    xLuaData& limitdata = m->second.getMutableData("CycleLimit");
    rCfg.m_eLimitType = static_cast<ELimitCyleType>(limitdata.getTableInt("1"));
    rCfg.m_dwLimitNum = limitdata.getTableInt("2");

    // attr
    xLuaData& cookAttr = m->second.getMutableData("CookLvAttr");
    auto collectAttr1 = [&](std::string key, xLuaData &data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[Table_Food] CookLvAttr id : " << m->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      rCfg.m_vecCookLvAttr.push_back(oAttr);
    };
    cookAttr.foreach(collectAttr1);

    xLuaData& tasteAttr = m->second.getMutableData("TasteLvAttr");
    auto collectAttr2 = [&](std::string key, xLuaData &data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[Table_Food] TasteLvAttr id : " << m->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      rCfg.m_vecTasteLvAttr.push_back(oAttr);
    };
    tasteAttr.foreach(collectAttr2);

    m_mapNpc2Food[rCfg.m_dwNpcId] = rCfg.m_dwId;
  }

  if (bCorrect)
    XLOG << "[Table_Food], 成功加载表格Table_Food.txt" << XEND;
  return bCorrect;
}

SFoodConfg* FoodConfig::getFoodCfg(DWORD dwItemId)
{
  auto it = m_mapFoodCfg.find(dwItemId);
  if (it == m_mapFoodCfg.end())
    return nullptr;
  return &(it->second);
}
