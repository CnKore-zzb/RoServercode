#pragma once

#include "xSingleton.h"
#include <map>
#include <vector>
#include "xLuaTable.h"

enum ELimitCyleType
{
  ELIMIT_CYCLE_MIN = 0,
  ELIMIT_CYCLE_DAY = 1,
  ELIMIT_CYCLE_WEEK = 2,
  ELIMIT_CYCLE_MONTH = 3,
};

struct SFoodConfg
{
  DWORD m_dwId = 0;
  string m_strName;
  DWORD m_dwCookLv = 0;   //制作等级
  DWORD m_dwCookHard = 0;
  DWORD m_dwCookerExp = 0;
  DWORD m_dwTasterExp = 0;
  DWORD m_dwFullProgress = 0;
  int m_height = 0;
  int m_weight = 0;
  DWORD m_dwSaveHP = 0;
  DWORD m_dwSaveSP = 0;
  xLuaData m_oBuff;
  DWORD m_dwHealHP = 0;
  DWORD m_dwHealSP = 0;
  DWORD m_dwDuration = 0;    //效果时长
  DWORD m_dwNpcId = 0;
  DWORD m_bMultiEat = 0;     //0：单人份 1：多人份
  DWORD m_dwTotalStep = 0;
  DWORD m_dwStepDuration = 0;
  DWORD m_dwHatReward = 0;
  TVecDWORD m_vecBuff;  
  TVecAttrSvrs m_vecCookLvAttr;
  TVecAttrSvrs m_vecTasteLvAttr;
  ELimitCyleType m_eLimitType = ELIMIT_CYCLE_MIN;
  DWORD m_dwLimitNum = 0;
};

// config
class FoodConfig : public xSingleton<FoodConfig>
{
  friend class xSingleton<FoodConfig>;
private:
  FoodConfig();
public:
  virtual ~FoodConfig();
  bool loadConfig();
  bool checkConfig();

  SFoodConfg* getFoodCfg(DWORD dwItemId);
  DWORD getFoodIdByNpcId(DWORD dwNpcId) { auto it = m_mapNpc2Food.find(dwNpcId); if (it == m_mapNpc2Food.end()) return 0; else return it->second; }

private:
  bool loadFoodConfig();

private:
  std::map<DWORD, SFoodConfg> m_mapFoodCfg;
  std::map<DWORD/*npcid*/, DWORD/*foodid*/> m_mapNpc2Food;
};

