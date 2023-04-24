/**
 * @file ComposeConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-16
 */

#pragma once

#include "xSingleton.h"
#include "SceneItem.pb.h"

using std::map;
using std::string;
using namespace Cmd;

// config data
enum EComposeType
{
  ECOMPOSETYPE_MIN = 0,
  ECOMPOSETYPE_PACKAGE = 1,
  ECOMPOSETYPE_NPC = 2,
  ECOMPOSETYPE_TRADER = 3,
  ECOMPOSETYPE_ALCHEMIST = 4,
  ECOMPOSETYPE_RUNEKNIGHT = 5,
  ECOMPOSETYPE_MAX
};
struct SProductCFG
{
  DWORD dwTypeID = 0;
  DWORD dwCardSlot = 0;
  DWORD dwNum = 0;
  DWORD dwWeight = 0;
  DWORD dwOffset = 0; //tmp
  SProductCFG() {}
};

struct SProductRandomCFG
{
  std::vector<SProductCFG> vecProduct;
  DWORD dwTotalWeight = 0;
  const SProductCFG* random()const;
};

struct SComposeCFG
{
  DWORD dwID = 0;
  DWORD dwRate = 0;

  DWORD dwROB = 0;
  DWORD dwGold = 0;
  DWORD dwDiamond = 0;

  DWORD dwCategory = 0;
  DWORD dwNeedMenuID = 0;
  bool bDynamicRate = false;

  EComposeType eType = ECOMPOSETYPE_MIN;

  TVecItemInfo vecMaterial;
  TVecItemInfo vecConsume;

  SProductCFG stProduct;
  SProductCFG stCriProduct;
  SProductRandomCFG randomProduct;
  SProductRandomCFG femaleRandomProduct;
  const SProductCFG* getProductCFG(EGender gender) const;

  SComposeCFG() {}
};
typedef map<DWORD, SComposeCFG> TMapComposeCFG;

// config
class ComposeConfig : public xSingleton<ComposeConfig>
{
  friend class xSingleton<ComposeConfig>;
  private:
    ComposeConfig();
  public:
    virtual ~ComposeConfig();

    bool loadConfig();

    const SComposeCFG* getComposeCFG(DWORD dwID) const;
    DWORD getOriMaterialEquip(DWORD dwProductID) const;
  private:
    bool checkConfig();
  private:
    TMapComposeCFG m_mapComposeCFG;
};

