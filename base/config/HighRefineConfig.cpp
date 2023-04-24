#include "HighRefineConfig.h"
#include "ItemConfig.h"
#include "xTools.h"

bool SMatCompose::checkPercent(bool main, TVecItemInfo vecItemInfo) const
{
  const std::map<DWORD/*itemid*/, DWORD>* pMap;
  if (main)
    pMap = &mapMainMat;
  else
    pMap = &mapViceMat;
  
  if (pMap == nullptr || pMap->empty())
    return false;
  DWORD percent = 0;
  for (auto&v : vecItemInfo)
  {
    auto it = pMap->find(v.id());
    if (it == pMap->end())
      return false;
    percent += it->second*v.count();
  }
  if (percent < 100)
    return false;
  return true;
}

DWORD SMatCompose::randomCount() const
{
  return randBetween(prCount.first, prCount.second);
}

bool JobAttr::checkJob(EProfession job) const
{
  if (vecJob.empty())
    return true;
  auto it = std::find(vecJob.begin(), vecJob.end(), job);
  if (it != vecJob.end())
    return true;
  return false;
}


HighRefineConfig::HighRefineConfig()
{
}

HighRefineConfig::~HighRefineConfig()
{
}

bool HighRefineConfig::loadConfig()
{
  bool bCorrect = true;
  if (loadMatConfig() == false)
    bCorrect = false;
  if (loadHighRefineConfig() == false)
    bCorrect = false;

  return bCorrect;
}

bool HighRefineConfig::checkConfig()
{
  bool bCorrect = true;
  
  return bCorrect;
}

bool HighRefineConfig::loadMatConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_HighRefineMatCompose.txt"))
  {
    XERR << "[表格],加载表格,Table_HighRefineMatCompose.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_HighRefineMatCompose", table);
  m_mapMatCompose.clear();

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapMatCompose.find(id);
    if (it == m_mapMatCompose.end())
    {
      m_mapMatCompose[id] = SMatCompose();
      it = m_mapMatCompose.find(id);
    }
    if (it == m_mapMatCompose.end())
    {
      XERR << "[Table_HighRefineMatCompose] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }

    SMatCompose& rCfg = it->second;
    rCfg.dwId = id;
    rCfg.dwGroupId = m->second.getTableInt("GroupId");
    rCfg.dwProductId = m->second.getTableInt("ProductId");
    rCfg.prCount.first = m->second.getMutableData("Count").getTableInt("1");
    rCfg.prCount.second = m->second.getMutableData("Count").getTableInt("2");
    rCfg.dwCostZeny = m->second.getTableInt("Cost");

    std::map<DWORD/*itemid*/, DWORD>* pMap;
    auto f = [&](std::string key, xLuaData &data)
    {
      if (pMap)
      {
        (*pMap)[data.getTableInt("1")] += data.getTableInt("2");
      }
    };   
    pMap = &rCfg.mapMainMat;
    m->second.getMutableData("MainMaterial").foreach(f);

    pMap = &rCfg.mapViceMat;
    m->second.getMutableData("ViceMaterial").foreach(f);
  }

  if (bCorrect)
    XLOG << "[Table_HighRefineMatCompose], 成功加载表格Table_HighRefineMatCompose.txt" << XEND;
  return bCorrect;
}

bool HighRefineConfig::loadHighRefineConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_HighRefine.txt"))
  {
    XERR << "[表格],加载表格,Table_HighRefine.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_HighRefine", table);
  m_mapHighRefine.clear();
  m_mapHighRefineLv.clear();

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapHighRefine.find(id);
    if (it == m_mapHighRefine.end())
    {
      m_mapHighRefine[id] = SHighRefine();
      it = m_mapHighRefine.find(id);
    }
    if (it == m_mapHighRefine.end())
    {
      XERR << "[Table_HighRefine] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }
    SHighRefine& rCfg = it->second;
    rCfg.dwId = id;
    DWORD pos = m->second.getTableInt("PosType");
    if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX || !EEquipPos_IsValid(pos))
    {
      XERR << "[Table_HighRefine] 装备位置不合法，id = " << id << "pos" << pos << XEND;
      bCorrect = false;
      continue;
    }
    rCfg.ePos = static_cast<EEquipPos>(pos);
    rCfg.dwLevel = m->second.getTableInt("Level");
    rCfg.dwType = rCfg.dwLevel / 1000;
    rCfg.dwPreLevel = m->second.getTableInt("PreLevel");
    rCfg.dwRefineLevel = m->second.getTableInt("RefineLv");
    auto f = [&](std::string key, xLuaData &data)
    {
      DWORD itemId = data.getTableInt("1");
      DWORD count = data.getTableInt("2");
      ItemInfo info;
      info.set_id(itemId);
      info.set_count(count);
      rCfg.oCost.push_back(info);
    };
    m->second.getMutableData("Cost").foreach(f);  

    JobAttr jobAttr;
    auto collectAttr = [&](std::string key, xLuaData &data)
    {
      if (key == "Job")
      {
        for (auto it = data.m_table.begin(); it != data.m_table.end(); ++it)
        {
          DWORD job = it->second.getInt();
          if (job <= EPROFESSION_MIN || !EProfession_IsValid(job))
          {
            bCorrect = false;
            XERR << "[Table_HighRefine] id : " << m->first << " job : " << job << " 非法" << XEND;
            continue;
          }
          jobAttr.vecJob.push_back(static_cast<EProfession>(job));
        }
        return;
      }

      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[Table_HighRefine] id : " << m->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());      
      jobAttr.vecAttr.push_back(oAttr);
    };
    xLuaData& rEffect = m->second.getMutableData("Effect");

    for (auto it = rEffect.m_table.begin(); it != rEffect.m_table.end(); ++it)
    {
      jobAttr.vecJob.clear();
      jobAttr.vecAttr.clear();
      it->second.foreach(collectAttr);
      if (!jobAttr.vecAttr.empty())
        rCfg.vecAttr.push_back(jobAttr);
    }
    
    auto equalPosF = [&](std::string key, xLuaData &data)
    {
      EEquipPos equalPos = static_cast<EEquipPos>(data.getInt());
      m_mapPos2RealPos[equalPos] = rCfg.ePos;
    };
    m_mapPos2RealPos[rCfg.ePos] = rCfg.ePos;
    m->second.getMutableData("EqualPos").foreach(equalPosF);

    DWORD key = getKey(rCfg.ePos, rCfg.dwLevel);
    m_mapHighRefineLv[key] = rCfg.dwId;
  }

  if (bCorrect)
    XLOG << "[Table_HighRefine], 成功加载表格Table_HighRefine.txt" << XEND;
  return bCorrect;
}
