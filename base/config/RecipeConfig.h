#pragma once

#include "xSingleton.h"
#include <map>
#include <vector>
#include "SceneFood.pb.h"

enum EMaterialType
{
  EMaterialType_Item = 1,
  EMaterialType_ItemType = 2,
};
struct SRecipeMaterial
{
  EMaterialType type = EMaterialType_Item;
  DWORD key = 0;
  DWORD num = 0;
};

typedef std::pair<DWORD/*item id*/, DWORD/*item count*/> TMeterCountPair;
typedef std::vector<TMeterCountPair> TVecMeterCount;

struct SRecipeCFG
{
  DWORD m_dwId = 0;
  std::string strName;
  Cmd::ECookType m_cookType;
  DWORD m_dwProduct = 0;
  DWORD m_dwUnlockExp = 0; //第一次解锁获得的厨师经验
  std::vector<SRecipeMaterial> m_vecMeterials;
  std::vector<SRecipeMaterial> m_vecMatchMeterials;
  DWORD m_dwTotalNum = 0;
  bool matchRecipe(TVecMeterCount& vecMeter);
  DWORD matchRecipeCount(TVecMeterCount& vecMeter);  //计算可以制作该食谱的次数
  void clear();

  bool m_bMatched = false;
  DWORD m_dwScore = 0;
  DWORD m_dwMatchedCount = 0;

  //tmp
  TVecMeterCount m_vecLeft;    //匹配后剩余的

  DWORD m_dwRewardid = 0;
  DWORD m_dwRate = 0;
};

struct CmpBySort {   //从大到小排序,score 从小到大
  bool operator()(const SRecipeCFG& lhs, const SRecipeCFG& rhs) {
    if (lhs.m_dwMatchedCount > rhs.m_dwMatchedCount)
      return true;  
    else if (lhs.m_dwMatchedCount == rhs.m_dwMatchedCount)
    {
      if (lhs.m_dwScore < rhs.m_dwScore)
        return true;
      else
        return false;
    }
    else
      return false;
  }
};

// config
class RecipeConfig : public xSingleton<RecipeConfig>
{
  friend class xSingleton<RecipeConfig>;
  private:
    RecipeConfig();
  public:
    virtual ~RecipeConfig();
    bool loadConfig();
    bool checkConfig();

    DWORD matchRecipe(Cmd::ECookType cookType, TVecMeterCount& vecMater, std::vector<SRecipeCFG*>& vecMatchRecipe);
    const SRecipeCFG* getRecipeCFG(DWORD id);

    void collectMaterial(DWORD dwItemID, std::vector<SRecipeMaterial>& vecResult);
  private:
    std::map<DWORD, SRecipeCFG> m_mapRecipes;
    std::map<DWORD, std::vector<SRecipeMaterial>> m_mapID2Material;
};

