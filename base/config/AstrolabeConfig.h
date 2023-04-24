#pragma once

#include "xSingleton.h"

using namespace Cmd;
using std::map;
using std::vector;
using std::pair;
using std::set;

struct SAstrolabeAttrCFG
{
  TVecAttrSvrs vecAttr; // 基础属性
  TVecDWORD vecEffect; // 特殊效果
};

typedef map<DWORD, SAstrolabeAttrCFG> TMapAstrolabeAttrCFG;

struct SAstrolabeStarCFG
{
  DWORD dwId = 0;
  vector<pair<DWORD, DWORD>> vecConn; // 连接星位
  TVecItemInfo vecCost; // 解锁消耗道具
  TMapAstrolabeAttrCFG mapAttr; // 激活效果
  TVecItemInfo vecResetCost; // 重置消耗道具

  DWORD dwPetAdventureScore = 0;
  const SAstrolabeAttrCFG* getAttrByProfessionType(DWORD pt) const;
};

typedef map<DWORD, SAstrolabeStarCFG> TMapAstrolabeStarCFG;

struct SAstrolabeCFG
{
  DWORD dwId = 0;
  TMapAstrolabeStarCFG mapStar;
  DWORD dwLevel = 0;
  DWORD dwEvo = 0;
  DWORD dwMenuID = 0;

  bool canActivate(DWORD level, DWORD evo) const { return level >= dwLevel && evo >= dwEvo; }
};

typedef map<DWORD, SAstrolabeCFG> TMapAstrolabeCFG;

enum ERuneSpecType
{
  ERUNESPECTYPE_MIN = 0,
  ERUNESPECTYPE_FOREVER = 1,
  ERUNESPECTYPE_SELECT = 2,
  ERUNESPECTYPE_GETSKILL = 3,
  ERUNESPECTYPE_MAX,
};

struct SRuneSpecCFG
{
  DWORD dwID = 0;
  ERuneSpecType eType = ERUNESPECTYPE_MIN;
  TSetDWORD setSkillIDs;
  string strName;

  TSetDWORD setBuffIDs;

  DWORD dwBeingSkillPoint = 0;
  map<DWORD, TSetDWORD> mapBeingBuff;
  DWORD dwBeingSkillID = 0;
  int intRange = 0;
  map<DWORD, int> mapBeingItemCosts;
};
typedef map<DWORD, SRuneSpecCFG> TMapRuneSpecCFG;

class AstrolabeConfig : public xSingleton<AstrolabeConfig>
{
  friend class xSingleton<AstrolabeConfig>;
private:
  AstrolabeConfig();
public:
  virtual ~AstrolabeConfig();

  bool loadConfig();
  bool checkConfig();

  bool loadAstrolabeConfig();
  bool loadRuneConfig();
  bool loadRuneSpecConfig();

  const SAstrolabeStarCFG* getAstrolabeStar(DWORD id, DWORD starid) const;
  const SAstrolabeCFG* getAstrolabe(DWORD id) const;
  const SRuneSpecCFG* getRuneSpecCFG(DWORD id) const;
  const TMapAstrolabeCFG& getAllAstrolable() const { return m_mapAstrolabeCFG; }

private:
  TMapAstrolabeCFG m_mapAstrolabeCFG;
  TMapRuneSpecCFG m_mapRuneSpecCFG;
};
