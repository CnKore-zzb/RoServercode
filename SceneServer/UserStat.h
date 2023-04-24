#pragma once

#include "xDefine.h"
#include "Var.h"
#include "StatisticsDefine.h"
#include "RecordCmd.pb.h"
#include "Var.pb.h"
#include <map>
using namespace std;

class SceneUser;

struct SSkillDamage
{
  DWORD dwDamage;
  QWORD qwTargetId;
};

typedef std::map<DWORD/*skillId*/, SSkillDamage> TMapSKillId2Damage;
typedef std::map<STAT_TYPE, TMapSKillId2Damage> TMapSKillType2Damage;


class UserStat
{
public:
  UserStat(SceneUser* pUser);
  ~UserStat();

  void load(const BlobStatVar& oBlob);
  void save(BlobStatVar* pBlob);
  void onLogin();
  bool checkAndSet(STAT_TYPE type, QWORD key=0);

  void sendStatLog(STAT_TYPE type, QWORD key, QWORD subKey, DWORD level, DWORD value1, bool isFloat = false);
  void sendStatLog2(STAT_TYPE type, QWORD key, QWORD subKey, QWORD subKey2, DWORD level, DWORD value1, bool isFloat = false);
  void addMonsterCnt(DWORD monsterid, DWORD cnt);
  void sendDayGetZenyCountLog(QWORD qwNormalZeny, QWORD qwChargeZeny);

  void checkAndSendCurLevelToStat();
private:
  SceneUser* m_pUser = nullptr;

public:
  DWORD getVarValue(STAT_TYPE eType, const string& key);
  void setVarValue(const string& key, DWORD value);
private:
  EVarTimeType getTimeType(STAT_TYPE type);
  string getKey(STAT_TYPE type, QWORD key);
  void  statFashion();

public:
  void setSkillDamage(STAT_TYPE type, DWORD dwSkillId, DWORD newValue, QWORD qwTarget);
  void sendSkillDamage();
private:
  //StatVar m_oVar[string];
  std::map<string/*key*/, StatVar> m_oVar;

  TMapSKillType2Damage m_mapSkillDamage;
  map<DWORD, DWORD> m_mapKillMonsterNum; // 记录一天一分钟杀怪的最多数量,<monsterid, num>, monstertid= 0 时, 记录的时1分钟杀怪(不区分怪物id)的总数量
};

