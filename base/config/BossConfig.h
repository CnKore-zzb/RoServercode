/**
 * @file BossConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-08-08
 */

#pragma once

#include "TableManager.h"
#include "BossCmd.pb.h"

// SBossCFG
enum EBossType
{
  EBOSSTYPE_MIN = 0,
  EBOSSTYPE_MVP = 1,
  EBOSSTYPE_MINI = 2,
  EBOSSTYPE_DEAD = 3,
  EBOSSTYPE_WORLD = 4,
  EBOSSTYPE_MAX = 5,
};

struct SBossLvCFG
{
  DWORD dwLv = 0;

  TSetDWORD setRewardIDs;
  TSetDWORD setBuffIDs;
  TSetDWORD setSuperAIs;
};
typedef map<DWORD, SBossLvCFG> TMapBossLvCFG;
struct SBossCFG : public SBaseCFG
{
  DWORD dwID = 0;
  DWORD dwDeadID = 0;

  DWORD dwSetWeight = 0;
  DWORD dwRandomTime = 0;

  EBossType eType = EBOSSTYPE_MIN;
  string strName;

  TVecDWORD vecMap;
  std::map<DWORD, DWORD> mapReliveTime;

  TVecDWORD vecActIDs;

  xLuaData oBossDefine;
  TMapBossLvCFG mapLvCFG;

  const TVecDWORD& getAllMap() const { return vecMap; }
  const xLuaData& getBossDefine() const { return oBossDefine; }
  DWORD getRandomTime() const { return dwRandomTime; }
  EBossType getType() const { return eType; }

  DWORD getMapID() const;
  DWORD getReliveTime(DWORD mapid) const;

  const SBossLvCFG* getLvCFG(DWORD dwLv) const;
  DWORD randActID() const;
};
typedef std::map<DWORD, SBossCFG> TMapBossCFG;
typedef std::vector<SBossCFG> TVecBossCFG;
typedef std::map<DWORD, TVecBossCFG> TMapTypeBossCFG;

// SBossStep
struct SBossStepCFG
{
  Cmd::EBossStep step = EBOSSSTEP_MIN;
  xLuaData data;
};
typedef std::vector<SBossStepCFG> TVecBossStepCFG;
typedef std::map<DWORD, TVecBossStepCFG> TMapBossStepCFG;

// BossConfig
class BossConfig : public xSingleton<BossConfig>
{
  friend class xSingleton<BossConfig>;
  private:
    BossConfig();
  public:
    virtual ~BossConfig();

    bool loadConfig();
    bool checkConfig();

    template<class T> void foreach(T func)
    { for_each(m_mapBossCFG.begin(), m_mapBossCFG.end(), [func](const TMapBossCFG::value_type& r) { if (r.second.blInit) func(r.second);}); }
    template<class T> void foreach(EBossType eType, T func)
    { auto m = m_mapTypeBossCFG.find(eType); if (m != m_mapTypeBossCFG.end()) for_each(m->second.begin(), m->second.end(), [func](const SBossCFG& r) { if (r.blInit) func(r);}); }
    template<class T> void foreachstep(T func)
    { for_each(m_mapBossStepCFG.begin(), m_mapBossStepCFG.end(), [func](const TMapBossStepCFG::value_type& r) { for (auto &v : r.second) func(r.first, v); }); }

    const SBossCFG* getBossCFG(DWORD dwID) const;
    const SBossCFG* randSetBossCFG(EBossType eType, const TSetDWORD& setExcludeIDs);
  private:
    bool loadBossConfig();
    bool loadDeadBossConfig();
    bool loadBossStepConfig();

    Cmd::EBossStep getBossStep(const string& str);
  private:
    TMapBossCFG m_mapBossCFG;
    TMapTypeBossCFG m_mapTypeBossCFG;
    TMapBossStepCFG m_mapBossStepCFG;
};

