#pragma once

#include "xSingleton.h"
#include "ProtoCommon.pb.h"

using std::map;
using std::vector;
using std::set;

enum ETutorTaskType
{
  ETUTORTASKTYPE_MIN,
  ETUTORTASKTYPE_BOARD_QUEST,
  ETUTORTASKTYPE_REPAIR_SEAL,
  ETUTORTASKTYPE_USE_ITEM,
  ETUTORTASKTYPE_BE_USED_ITEM,
  ETUTORTASKTYPE_PASS_TOWER,
  ETUTORTASKTYPE_GUILD_RAID_FINISH,
  ETUTORTASKTYPE_LABORATORY_FINISH,
  ETUTORTASKTYPE_PASS_TOWER_LAYER,
  ETUTORTASKTYPE_MAX
};

struct STutorTaskCFG
{
  DWORD dwID = 0;
  ETutorTaskType eType = ETUTORTASKTYPE_MIN;
  DWORD dwTotalProgress = 0;
  map<DWORD, DWORD> mapStudentReward;
  DWORD dwTeacherReward = 0;
  DWORD dwProficiency = 0;
  DWORD dwResetTime = 0;
  string strName;

  set<DWORD> setValues;
  DWORD dwFlag = 0;

  DWORD getStudentReward(DWORD lv) const
  {
    for (auto& v : mapStudentReward)
      if (lv <= v.first)
        return v.second;
    return 0;
  }
  bool checkValid(QWORD value) const;
};

struct STutorGrowRewardCFG
{
  DWORD dwProficiency = 0;
  DWORD dwStudentReward = 0;
  DWORD dwTeacherReward = 0;
};

typedef vector<STutorTaskCFG> TVecTutorTaskCFG;
typedef map<DWORD, STutorGrowRewardCFG> TMapTutorGrowRewardCFG;

class TutorConfig : public xSingleton<TutorConfig>
{
  friend class xSingleton<TutorConfig>;
private:
  TutorConfig();
public:
  virtual ~TutorConfig();

  bool loadConfig();
  bool checkConfig();

  bool loadTask();
  bool loadGrowReward();

  const STutorTaskCFG* getTutorTaskCFG(DWORD dwID);
  const TVecTutorTaskCFG& getTutorTaskCFGByType(ETutorTaskType eType);
  const STutorGrowRewardCFG* getTutorGrowRewardCFG(DWORD dwLevel);
  DWORD getProficiencyByLv(DWORD dwLevel);
  const TMapTutorGrowRewardCFG& getAllGrowRewardCFG() { return m_mapTutorGrowRewardCFG; }

private:
  ETutorTaskType getTaskType(const string& type);

  map<DWORD, STutorTaskCFG> m_mapTutorTaskCFG;
  map<ETutorTaskType, TVecTutorTaskCFG> m_vecType2TutorTaskCFG;
  TMapTutorGrowRewardCFG m_mapTutorGrowRewardCFG;
};
