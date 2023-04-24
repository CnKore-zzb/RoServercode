#include "ComposeConfig.h"
#include "ItemConfig.h"
#include "xLuaTable.h"

const SProductCFG* SProductRandomCFG::random() const
{
  if (vecProduct.size() == 1)
  {
    return &vecProduct[0];
  }

  DWORD r = randBetween(1, dwTotalWeight); 
  for (auto&v : vecProduct)
  {
    if (r <= v.dwOffset)
    {
      return &v;
    }
  }
  XERR << "[时装碎片合成-随机出错]" << r << "totalweight" << dwTotalWeight << XEND;
  return nullptr;
}

const SProductCFG* SComposeCFG::getProductCFG(EGender gender) const
{ 
  if (randomProduct.vecProduct.empty() == false )
  {
    if (femaleRandomProduct.vecProduct.empty() == false)
    {
      if (gender == EGENDER_MALE)
        return randomProduct.random();
      else
        return femaleRandomProduct.random();
    }
    else
      return randomProduct.random();
  }
  return &stProduct;
}

// config
ComposeConfig::ComposeConfig()
{

}

ComposeConfig::~ComposeConfig()
{

}

bool ComposeConfig::loadConfig()
{
  bool bCorrect = true;

  // load quest config
  if (!xLuaTable::getMe().open("Lua/Table/Table_Compose.txt"))
  {
    XERR << "[ComposeConfig],加载配置Table_Compose.txt失败" << XEND;
    return false;
  }

  m_mapComposeCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Compose", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    if (getComposeCFG(m->first) != nullptr)
    {
      XERR << "[ComposeConfig] id = " << m->first << " duplicated!" << XEND;
      bCorrect = false;
      continue;
    }

    DWORD dwType = m->second.getTableInt("Type");
    if (dwType <= ECOMPOSETYPE_MIN || dwType >= ECOMPOSETYPE_MAX)
    {
      XERR << "[合成配置-加载] id : " << m->first << " type : " << dwType << " 不合法的类型" << XEND;
      bCorrect = false;
      continue;
    }

    // create config
    SComposeCFG stCFG;

    stCFG.dwID = m->first;
    stCFG.dwRate = m->second.getTableInt("Rate");

    stCFG.dwROB = m->second.getTableInt("ROB");
    stCFG.dwGold = m->second.getTableInt("Gold");
    stCFG.dwDiamond = m->second.getTableInt("Diamond");

    stCFG.eType = static_cast<EComposeType>(dwType);

    xLuaData& cost = m->second.getMutableData("BeCostItem");
    auto costf = [&stCFG](const string& key, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("id"));
      oItem.set_count(data.getTableInt("num"));
      combinItemInfo(stCFG.vecMaterial, TVecItemInfo{oItem});
    };
    cost.foreach(costf);

    xLuaData& consume = m->second.getMutableData("FailStayItem");
    bool bConsumeFail = true;
    auto consumef = [&](const string& key, xLuaData& data)
    {
      DWORD dwIndex = data.getInt();
      if (dwIndex == 0)
        return;
      dwIndex -= 1;
      if (dwIndex >= stCFG.vecMaterial.size())
      {
        XERR << "[合成配置-加载] id : " << stCFG.dwID << " FailStayItem index : " << dwIndex + 1 << " 不合法" << XEND;
        bCorrect = false;
        bConsumeFail = false;
        return;
      }
      stCFG.vecConsume.push_back(stCFG.vecMaterial[dwIndex]);
      stCFG.vecMaterial[dwIndex].set_id(0);
    };
    consume.foreach(consumef);
    if (!bConsumeFail)
      continue;

    for (auto v = stCFG.vecMaterial.begin(); v != stCFG.vecMaterial.end();)
    {
      if (v->id() == 0)
        v = stCFG.vecMaterial.erase(v);
      else
        ++v;
    }

    xLuaData& product = m->second.getMutableData("Product");
    stCFG.stProduct.dwTypeID = product.getTableInt("id");
    stCFG.stProduct.dwCardSlot = product.getTableInt("cardslot");
    if (product.has("num"))
      stCFG.stProduct.dwNum = product.getTableInt("num");
    else
      stCFG.stProduct.dwNum = 1;

    xLuaData& criproduct = m->second.getMutableData("GreatProduct");
    stCFG.stCriProduct.dwTypeID = criproduct.getTableInt("id");
    stCFG.stCriProduct.dwCardSlot = criproduct.getTableInt("cardslot");
    if (criproduct.has("num"))
      stCFG.stCriProduct.dwNum = criproduct.getTableInt("num");
    else
      stCFG.stCriProduct.dwNum = 1;
    
    auto randomF1 = [&](const string& key, xLuaData& data) {
      SProductCFG product;
      product.dwTypeID = data.getTableInt("id");
      product.dwNum = 1;
      product.dwWeight = data.getTableInt("weight");
      stCFG.randomProduct.dwTotalWeight += product.dwWeight;
      product.dwOffset = stCFG.randomProduct.dwTotalWeight;
      stCFG.randomProduct.vecProduct.push_back(product);
    };
    m->second.getMutableData("RandomProduct").foreach(randomF1);

    auto randomF2 = [&](const string& key, xLuaData& data) {
      SProductCFG product;
      product.dwTypeID = data.getTableInt("id");
      product.dwNum = 1;
      product.dwWeight = data.getTableInt("weight");
      stCFG.femaleRandomProduct.dwTotalWeight += product.dwWeight;
      product.dwOffset = stCFG.femaleRandomProduct.dwTotalWeight;
      stCFG.femaleRandomProduct.vecProduct.push_back(product);
    };
    m->second.getMutableData("FemaleRandomProduct").foreach(randomF2);

    stCFG.bDynamicRate = m->second.getTableInt("DynamicRate") == 1;
    stCFG.dwNeedMenuID = m->second.getTableInt("MenuID");
    stCFG.dwCategory = m->second.getTableInt("Category");
    // insert to list
    m_mapComposeCFG[stCFG.dwID] = stCFG;
  }

  return bCorrect;
}

const SComposeCFG* ComposeConfig::getComposeCFG(DWORD dwID) const
{
  auto m = m_mapComposeCFG.find(dwID);
  if (m != m_mapComposeCFG.end())
    return &m->second;

  return nullptr;
}

DWORD ComposeConfig::getOriMaterialEquip(DWORD dwProductID) const
{
  for (auto &m :m_mapComposeCFG)
  {
    if (m.second.stProduct.dwTypeID == dwProductID && m.second.dwCategory == 6)
      return m.second.vecMaterial.empty() == true ? 0 : m.second.vecMaterial.begin()->id();
  }
  return 0;
}

bool ComposeConfig::checkConfig()
{
  return true;
}


