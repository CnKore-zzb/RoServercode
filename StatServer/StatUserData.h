#pragma once
#include "xDefine.h"
#include "StatisticsDefine.h"
#include "xSingleton.h"
#include "xDBConnPool.h"
#include <list>
#include "GameStruct.h"

#define STAT_KILL_DB "stat_kill_monster"
#define STAT_DAY_GET_ZENY_TOP "day_get_zeny_top"
#define STAT_WORLD_LEVEL_DB "world_level_stat"
#define STAT_PET_WEAR "stat_pet_wear"
const DWORD MAX_KILL_MONSTER_NUM = 100;
const DWORD MAX_TOTAL_LEVEL_DAY = 14;
const DWORD MAX_AVR_LEVEL_DAY = 7;

using namespace std;
using namespace Cmd;
class xNetProcessor;

struct SKillMonsterData
{
  DWORD dwMonsterID = 0;

  DWORD dwKillNum = 0;
  QWORD qwCharID = 0;
  DWORD dwZoneID = 0;
  DWORD dwProfessionID = 0;
};
typedef std::list<SKillMonsterData> TListKillMonsterData;
struct SStatkillMonster
{
  DWORD dwMonsterID = 0;
  TListKillMonsterData listMonsterData; // 记录100条数据

  bool bUpdate = false; // 标记新数据
  DWORD dwNextUpdateTime = 0; // 下次更新时间戳
  DWORD dwRandUpdateTime = 0; // 随机更新间隔

  bool checkNoAdd(DWORD num)
  {
    return !listMonsterData.empty() && (listMonsterData.rbegin()->dwKillNum >= num);
  }
  void addStatData(const SKillMonsterData& data);
  void setRandUpdateTime()
  {
    dwRandUpdateTime = randBetween(30*60, 60*60);
  }
  bool saveDb(DWORD date);
};
typedef map<DWORD, SStatkillMonster> TMapStatKillMonster;

struct SDayZenyCount
{
  QWORD dwCharID = 0;
  string strName = "";
  DWORD dwBaseLv = 0;
  DWORD dwJobLv = 0;
  DWORD dwProfession = 0;
  QWORD qwNormalZeny = 0;
  QWORD qwChargeZeny = 0;
  QWORD qwTotalZeny = 0;

  void clear();
  bool saveDb(DWORD date);
  bool loadDb(DWORD date);
};

struct STotalLevelDayData
{
  DWORD dwDate = 0;
  DWORD dwTotalBaseLevel = 0;
  DWORD dwTotalBaseLevelCount = 0;
  DWORD dwTotalJobLevel = 0;
  DWORD dwTotalJobLevelCount = 0;
};
typedef map<DWORD, STotalLevelDayData> TMapTotalLevelDayData;

class StatUserData : public xSingleton<StatUserData>
{
  public:
    StatUserData();
    virtual ~StatUserData();

    bool init();
    void timeTick(DWORD curTime);

    void addUserKillMonsterCnt(const KillMonsterNumStatCmd& cmd);
    void updateDayGetZenyCount(const DayGetZenyCountCmd& cmd);
    void addTotalLevelDayData(const StatCurLevel& cmd);
    void sendWorldLevel(DWORD dwZoneID = 0, xNetProcessor *np = nullptr);
    void addPetWearUseCount(const PetWearUseCountStatCmd& cmd);
  private:
    bool loadDb();
    bool loadWorldLevelDb();
    bool saveDb();
    bool saveWorldLevelDb();

    void calcWorldLevel();
    bool savePetWear(DWORD date);
  private:
    xTimer m_oOneMinTimer;
    TMapStatKillMonster m_mapStatKillMonster;
    DWORD m_dwCurDataDate = 0;
    SDayZenyCount m_oDayZenyCount;
    DWORD m_dwZenyCountCurDayDate = 0;
    TMapTotalLevelDayData m_mapTotalLevelDayData;
    DWORD m_dwAvrWorldBaseLevel = 0;
    DWORD m_dwAvrWorldJobLevel = 0;
    TMapPetWearStat m_mapPetWearStat;
};

