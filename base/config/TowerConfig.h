/**
 * @file TowerConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-12-08
 */

#pragma once

#include "xSingleton.h"
#include "InfiniteTower.pb.h"
#include "SessionCmd.pb.h"
#include "MiscConfig.h"
#include "NpcConfig.h"

using namespace Cmd;
using std::vector;
using std::pair;
using std::map;
using std::set;

// config data
struct STowerMonsterCFG
{
  DWORD dwMonsterID = 0;

  DWORD dwUnlockCondition = 0;
  DWORD dwUniqueID = 0;

  bool bUnLock = false;

  pair<DWORD, DWORD> pairLayer;

  STowerMonsterCFG() {}
};
typedef vector<STowerMonsterCFG> TVecTowerMonsterCFG;

struct STowerLayerCFG
{
  DWORD dwID = 0;
  DWORD dwRaidID = 0;
  DWORD dwSky = 0;

  DWORD dwNormalType = 0;
  DWORD dwNormalCount = 0;
  DWORD dwMiniCount = 0;
  DWORD dwMvpCount = 0;
  DWORD dwRewardID = 0;

  TVecTowerMonsterCFG vecCurMonster;
  TVecTowerMonsterCFG vecAllMonster;

  map<ENpcType, TSetDWORD> mapNpcExtraAIs;

  const TSetDWORD& getAIsByType(ENpcType eType) const
  {
    auto it = mapNpcExtraAIs.find(eType);
    if (it != mapNpcExtraAIs.end())
      return it->second;
    static const TSetDWORD emptyset;
    return emptyset;
  }

  string strBgm;

  STowerLayerCFG() {}

  void toData(TowerLayer* pLayer) const;
};
typedef map<DWORD, STowerLayerCFG> TMapTowerLayerCFG;

// config
class TowerConfig : public xSingleton<TowerConfig>
{
  friend class xSingleton<TowerConfig>;
  private:
    TowerConfig();
  public:
    virtual ~TowerConfig();

    DWORD getMaxLayer() const { return m_dwMaxLayer; }
    //DWORD getClearTime() const { return m_dwClearTime; }
    //void resetClearTime() { m_dwClearTime = 0; }
    const TMapTowerLayerCFG& getTowerConfigList() const { return m_mapTowerLayerCFG; }
    const set<DWORD>& getKillMonsterList() const { return m_setKillMonsterIDs; }

    bool loadConfig();
    bool checkConfig();
    bool initDataFromDB(const TowerInfo& rBlob);
    bool unlockMonster(DWORD dwDeadMonster);

    const STowerLayerCFG* getTowerLayerCFG(DWORD dwLayer) const;
    bool isTower(DWORD dwRaidID) const;

    void generateMonster();
    void updateConfig(const TowerInfo& cmd);
  private:
    bool loadEndLessMonsterConfig();
    bool loadEndLessTowerConfig();

    void makeMonster(const TVecTowerMonsterCFG& vecAll, DWORD dwNum, DWORD typeNum, TVecTowerMonsterCFG& vecResult);
  private:
    TMapTowerLayerCFG m_mapTowerLayerCFG;

    DWORD m_dwMaxLayer = 0;
    //DWORD m_dwClearTime = 0;

    TSetDWORD m_setKillMonsterIDs;
    TSetDWORD m_setTowerRaidIDs;
};

