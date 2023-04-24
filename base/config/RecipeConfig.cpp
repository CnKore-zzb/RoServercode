#include "RecipeConfig.h"
#include "ItemConfig.h"

void SRecipeCFG::clear()
{
  m_vecLeft.clear();
  m_dwScore = 0;
  m_bMatched = false;
  m_dwMatchedCount = 0;
}

bool SRecipeCFG::matchRecipe(TVecMeterCount& vecMeter)
{
  clear();
  m_vecLeft = vecMeter;
  if (vecMeter.empty())
    return false;
  
  if (m_vecMeterials.empty())
    return false;

  std::vector<SRecipeMaterial> vecMatchMeterials = m_vecMeterials;
  for (auto it = vecMatchMeterials.begin(); it != vecMatchMeterials.end(); ++it)
  { 
    for (DWORD i = 1; i <= m_vecLeft.size(); ++i)
    {
      if (it->num == 0)
        break;

      TMeterCountPair& meterInfo = m_vecLeft[i - 1];
      if (meterInfo.second == 0)
        continue;
      DWORD itemId = meterInfo.first;
      if (it->type == EMaterialType_Item)
      {
        if (itemId != it->key)
          continue;        
      }
      else if (it->type == EMaterialType_ItemType)
      {
        const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(itemId);
        if (pItemCfg == nullptr)
          continue;
        if (pItemCfg->eItemType != it->key)
          continue;
      }
      else
        continue;
      if (it->num <= meterInfo.second)
      {
        meterInfo.second -= it->num;
        it->num = 0;
      }
      else
      {
        it->num -= meterInfo.second;
        meterInfo.second = 0;
      }

      m_dwMatchedCount++;
      m_dwScore = m_dwScore* 10 + i;//分数越小匹配度越高
    }
    /*
    for (DWORD i = 1; i <= vecMeter.size(); ++i)
    {
      if (it->num == 0)
        break;

      DWORD itemId = vecMeter[i - 1];
      if (it->type == EMaterialType_Item)
      {
        if (itemId != it->key)
          continue;        
      }
      else if (it->type == EMaterialType_ItemType)
      {
        const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(itemId);
        if (pItemCfg == nullptr)
          continue;
        if (pItemCfg->eItemType != it->key)
          continue;
      }
      else
        continue;
      it->num--;
      //扣除剩余的
      auto it2 = std::find(m_vecLeft.begin(), m_vecLeft.end(), itemId);
      if (it2 != m_vecLeft.end())
        m_vecLeft.erase(it2);
      m_dwMatchedCount++;
      m_dwScore = m_dwScore* 10 + i;//分数越小匹配度越高
    }
    */
  } 

  for (auto it = m_vecLeft.begin(); it != m_vecLeft.end();)
  {
    if(it->second == 0)
    {
      it = m_vecLeft.erase(it);
      continue;
    }
    it++;
  }
  
  DWORD unmatchedNum = 0;
  for (auto &v : vecMatchMeterials)
    unmatchedNum += v.num;

  m_bMatched = unmatchedNum == 0 ? true : false;
  
  if (m_bMatched)
  {
    XDBG << "[料理-匹配] ，匹配中 id" << m_dwId << "匹配个数" << m_dwMatchedCount << "匹配分数" << m_dwScore << XEND;
  }

  return m_bMatched;
}

DWORD SRecipeCFG::matchRecipeCount(TVecMeterCount& vecMeter)
{
  clear();
  m_vecLeft = vecMeter;
  if (vecMeter.empty())
    return 0;
  
  if (m_vecMeterials.empty())
    return 0;

  DWORD dwMakeCount = DWORD_MAX;
  std::vector<SRecipeMaterial> vecMatchMeterials = m_vecMeterials;
  for (auto it = vecMatchMeterials.begin(); it != vecMatchMeterials.end(); ++it)
  { 
    if (it->num == 0)
      continue;
    DWORD dwTotalCount = 0;
    for (auto& meterInfo : m_vecLeft)
    {
      if (meterInfo.second == 0)
        continue;
      DWORD itemId = meterInfo.first;
      if (it->type == EMaterialType_Item)
      {
        if (itemId != it->key)
        {
          continue;        
        }
      }
      else if (it->type == EMaterialType_ItemType)
      {
        const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(itemId);
        if (pItemCfg == nullptr)
          continue;
        if (pItemCfg->eItemType != it->key)
          continue;
      }
      else
        continue;
      dwTotalCount += meterInfo.second;
      meterInfo.second = 0;
    }
    dwTotalCount = dwTotalCount/it->num;
    if (dwTotalCount < dwMakeCount)
    {
      dwMakeCount = dwTotalCount;
    }
  }

  if (dwMakeCount == DWORD_MAX || dwMakeCount == 0)
  {
    return 0;
  }

  for (auto it = vecMatchMeterials.begin(); it != vecMatchMeterials.end(); ++it)
  {
    it->num = it->num * dwMakeCount;
    for (DWORD i = 1; i <= vecMeter.size(); ++i)
    {
      if (it->num == 0)
        break;

      TMeterCountPair& meterInfo = vecMeter[i - 1];
      if (meterInfo.second == 0)
        continue;
      DWORD itemId = meterInfo.first;
      if (it->type == EMaterialType_Item)
      {
        if (itemId != it->key)
          continue;
      }
      else if (it->type == EMaterialType_ItemType)
      {
        const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(itemId);
        if (pItemCfg == nullptr)
          continue;
        if (pItemCfg->eItemType != it->key)
          continue;
      }
      else
        continue;
      if (it->num <= meterInfo.second)
      {
        meterInfo.second -= it->num;
        it->num = 0;
      }
      else
      {
        it->num -= meterInfo.second;
        meterInfo.second = 0;
      }
    }
  }

  for (auto it = vecMeter.begin(); it != vecMeter.end();)
  {
    if(it->second == 0)
    {
      it = vecMeter.erase(it);
      continue;
    }
    it++;
  }
  
  return dwMakeCount;
}

RecipeConfig::RecipeConfig()
{
}

RecipeConfig::~RecipeConfig()
{
}

bool RecipeConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Recipe.txt"))
  {
    XERR << "[表格],加载表格,Table_Recipe.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Recipe", table);
  m_mapRecipes.clear();

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapRecipes.find(id);
    if (it == m_mapRecipes.end())
    {
      m_mapRecipes[id] = SRecipeCFG();
      it = m_mapRecipes.find(id);
    }
    if (it == m_mapRecipes.end())
    {
      XERR << "[Table_Recipe] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }
    SRecipeCFG& rCfg = it->second;
    rCfg.m_dwId = id;
    rCfg.strName = m->second.getTableString("Name");
    rCfg.m_cookType = static_cast<ECookType>(m->second.getTableInt("Type"));
    rCfg.m_dwProduct = m->second.getTableInt("Product");

    auto func = [&](const string& key, xLuaData& data)
    { 
      SRecipeMaterial material;
      material.type = static_cast<EMaterialType> (data.getTableInt("1"));
      material.key = data.getTableInt("2");
      material.num = data.getTableInt("3");
      rCfg.m_dwTotalNum += material.num;
      rCfg.m_vecMeterials.push_back(material);
    };
    m->second.getMutableData("Material").foreach(func);

    rCfg.m_dwUnlockExp = m->second.getTableInt("UnlockExp");
    rCfg.m_dwRewardid = m->second.getMutableData("ExtraReward").getTableInt("rewardid");
    rCfg.m_dwRate = m->second.getMutableData("ExtraReward").getTableInt("rate");
  }

  for (auto &m : m_mapRecipes)
    m_mapID2Material[m.second.m_dwProduct] = m.second.m_vecMeterials;

  if (bCorrect)
    XLOG << "[Table_Recipe], 成功加载表格Table_Recipe.txt" << XEND;
  return bCorrect;
}

bool RecipeConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto it = m_mapRecipes.begin(); it != m_mapRecipes.end(); ++it)
  {
    //TODO    
  }
  return bCorrect;
}

DWORD RecipeConfig::matchRecipe(Cmd::ECookType cookType, TVecMeterCount& vecMater, std::vector<SRecipeCFG*>& vecMatchRecipe)
{
  if (vecMater.empty())
    return 0;
  std::vector<SRecipeCFG> vecRank;
  vecRank.reserve(10);
  for (auto &m : m_mapRecipes)
  {
    if (m.second.m_cookType != cookType)
      continue;
    SRecipeCFG& rCfg = m.second;
    if (rCfg.matchRecipe(vecMater))
      vecRank.push_back(m.second);
  }
  
  if (vecRank.empty())
    return 0;

  std::sort(vecRank.begin(), vecRank.end(), CmpBySort()); 
  for (auto& it : vecRank)
  {
    SRecipeCFG* pCfg = const_cast<SRecipeCFG*>(getRecipeCFG(it.m_dwId));
    if (pCfg)
    {
      vecMatchRecipe.push_back(pCfg);
    }
  }
  //if (pCfg)
  //{
  //  pCfg->matchRecipe(vecMater);
  //}
  return vecRank.size();
}

const SRecipeCFG* RecipeConfig::getRecipeCFG(DWORD id)
{
  auto it = m_mapRecipes.find(id);
  if (it == m_mapRecipes.end())
    return nullptr;
  return &it->second;
}

void RecipeConfig::collectMaterial(DWORD dwItemID, std::vector<SRecipeMaterial>& vecResult)
{
  vecResult.clear();

  auto m = m_mapID2Material.find(dwItemID);
  if (m != m_mapID2Material.end())
    vecResult.insert(vecResult.end(), m->second.begin(), m->second.end());
}

