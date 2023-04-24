/**
 * @file SealConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-11-24
 */

#pragma once

#include "xSingleton.h"
#include "SceneSeal.pb.h"
#include "xPos.h"

using std::vector;
using std::map;
using std::string;
using std::pair;
using namespace Cmd;

// config data
struct SealPos
{
  DWORD dwRange = 0;

  xPos oPos;

  SealPos() {}
};
typedef vector<SealPos> TVecSealPos;

struct SealMonster
{
  DWORD dwID = 0;
  DWORD dwAppearLv = 0;
  DWORD dwPoint = 0;
  DWORD dwTeamID = 0;
  bool isspec = false;

  SealMonster() {}
};
typedef vector<SealMonster> TVecSealMonster;

typedef vector<pair<DWORD, DWORD>> TVecPaRate2Point;
struct SealCFG
{
  DWORD dwID = 0;
  DWORD dwMapID = 0;
  DWORD dwMaxTime = 0;
  DWORD dwMaxValue = 0;
  DWORD dwPosRange = 0;
  //DWORD dwRewardID = 0;
  DWORD dwPreQuest = 0;
  DWORD dwMaxMonsterNum = 0;
  DWORD dwSealLevel = 0;
  DWORD dwDMapID = 0;
  TVecDWORD vecRewardIDs;
  DWORD dwBaseExp = 0;
  DWORD dwJobExp = 0;

  ESealType eType = ESEALTYPE_MIN;

  TVecPaRate2Point vecRate2Point;
  TVecPaRate2Point vecSpcRate2Point;

  TVecSealMonster vecMonster;
  TVecSealPos vecRefreshPos;

  bool haveMonsterInPoint(DWORD lastValue, DWORD nowValue) const;

  bool getMonster(DWORD lastValue, DWORD nowValue, DWORD userLevel, TVecSealMonster& outVec) const;
  bool getMonsterByPoint(DWORD point, DWORD level, bool isspec, TVecSealMonster& outVec) const;

  bool findPoint(DWORD leftPoint, const TVecSealMonster& toolInVec, TVecSealMonster& outVec) const;
  void getVecLessThan(DWORD point, const TVecSealMonster& toolInVec, TVecSealMonster& toolVec) const;

  SealCFG() {}
};
typedef map<DWORD, SealCFG> TMapSealCFG;

// config
class SealConfig : public xSingleton<SealConfig>
{
  friend class xSingleton<SealConfig>;
  private:
    SealConfig();
  public:
    virtual ~SealConfig();

    bool loadConfig();

    const SealCFG* getSealCFG(DWORD dwID) const;
    const SealCFG* getSealCFGByQuest(DWORD dwQuestID) const;
    bool checkConfig() const;
  private:
    bool loadRepairSealConfig();
    bool loadSealMonsterConfig();
  private:
    TMapSealCFG m_mapSealCFG;
    TVecSealMonster m_vecSealMonster;
};

