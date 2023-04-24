#pragma once
/************************************************************************/
/* 统计程序                                                                     */
/************************************************************************/
#include "xDefine.h"
#include "StatisticsDefine.h"
#include "xSingleton.h"
#include "xDBConnPool.h"
#include <map>
#include <unordered_map>
#include <string>
#include <functional>

//#define STAT_TEST
#define DB_TABLE_NORMAL   "stat_normal"
#define OFFSET_HOUR       5 * HOUR_T
#define EXPIRE_MAX_TIME   90 * 86400

using namespace Cmd;

struct StatKey 
{
  STAT_TYPE type;
  QWORD key = 0;
  QWORD subKey = 0;
  QWORD subKey2 = 0;
  DWORD level = 0;
  DWORD time = 0;
  StatKey() {}

  StatKey(STAT_TYPE type, QWORD key, QWORD subKey, QWORD subKey2, DWORD level, DWORD time) :type(type),
    key(key),
    subKey(subKey),
    subKey2(subKey2),
    level(level),
    time(time)
  {}
};

struct StatKeyHash
{
  size_t operator()(const StatKey& a) const;
};

struct StatKeyEqual
{
  QWORD operator()(const StatKey& a, const StatKey& b) const
  {
    return a.type == b.type && a.key == b.key && a.subKey == b.subKey && a.level == b.level && a.time == b.time && a.subKey2 == b.subKey2;
  }
};

struct StatInfo
{
  STAT_TYPE type = ESTATTYPE_BEGIN;
  QWORD key = 0;
  QWORD subKey = 0;
  QWORD subKey2 = 0;
  DWORD level = 0;
  QWORD value1 = 0;
  QWORD value2 = 0;
  DWORD count = 0;
  bool isFloat = false;
  bool needUpdate = false;
  StatInfo() {}

  StatInfo(STAT_TYPE type, QWORD key, QWORD subKey, QWORD subKey2, DWORD level, QWORD value1, DWORD count, bool isFloat):type(type),
    key(key),
    subKey(subKey),
    subKey2(subKey2),
    level(level),
    value1(value1),
    count(count),
    isFloat(isFloat)
  {}
};


// std::bind封装一个宏, 简化bind的调用
#define HELP_BIND(ClassType, ObjPtr, MemFun) \
	std::bind(&ClassType::MemFun, ObjPtr, std::placeholders::_1, std::placeholders::_2) 

#define HELP_BIND1(ClassType, ObjPtr, MemFun) \
	std::bind(&ClassType::MemFun, ObjPtr, std::placeholders::_1)

enum STAT_SAVE_TYPE
{
  STAT_SAVE_TYPE_DAY = 1,
  STAT_SAVE_TYPE_WEEK = 2,
};


typedef std::function<void(StatInfo&)> NewHandler;
typedef std::function<void(StatInfo&, const StatInfo&)> AddHandler;

class StatBase
{
public:
  StatBase(STAT_TYPE type, STAT_SAVE_TYPE saveType);
  virtual ~StatBase();
  virtual void loadDb();
  virtual void saveDb();
  void timeTick(DWORD curSec);
  virtual void insertData(const StatKey& key, const StatInfo& info, bool isNew);
  void insertData(const StatCmd& rev);
  virtual void loadTime(DWORD curSec) = 0;
  virtual bool checkTimePass(DWORD curSec) = 0;
  void onTimePass(DWORD curSec);
  virtual void  v_onTimePass(DWORD curSec) {}

  void setNewHandler(NewHandler handler) { m_newHandler = handler; }
  void setAddHandler(AddHandler handler) { m_addHandler = handler; }
  void pushDb();
  void delDb(DWORD curSec);

protected:
  STAT_TYPE m_type;
  STAT_SAVE_TYPE m_saveType;
  std::unordered_map<StatKey/*key*/, StatInfo, StatKeyHash, StatKeyEqual> m_mapResult;
  DWORD m_time = 0;
  NewHandler m_newHandler;
  AddHandler m_addHandler;
  std::queue<xRecord> m_recordQueue;
  DWORD m_nextTime = 0;
  DWORD m_randTime = 0;
  bool m_bDel = true;
  xTimer m_oTimer;
};

class StatDay :public StatBase
{
public:
  StatDay(STAT_TYPE type):StatBase(type, STAT_SAVE_TYPE_DAY)
  {}

private:
  virtual void loadTime(DWORD curSec);
  virtual bool checkTimePass(DWORD curSec);
};

class StatWeek :public StatBase
{
public:
  StatWeek(STAT_TYPE type) :StatBase(type, STAT_SAVE_TYPE_WEEK)
  {}
private:
  virtual void loadTime(DWORD curSec);
  virtual bool checkTimePass(DWORD curSec);
};

class StatSKillDamage : public StatDay
{
public:
  StatSKillDamage(STAT_TYPE type) :StatDay(type)
  {}
  virtual void loadDb() {}
  virtual void saveDb() {}
  virtual void  v_onTimePass(DWORD curSec);
  virtual void insertData(const StatKey& key, const StatInfo& info, bool isNew);

private:
  //QWORD key = 0;    //技能id 
  //QWORD subKey = 0; //攻击玩家id
  //QWORD subKey2 = 0;//目标玩家id，或者怪物id
  //QWORD value1 = 0; //伤害值

  std::map<DWORD/*skillid*/, std::list<StatInfo>> m_mapSKillData;
};

class StatFashion : public StatDay
{
public:
  StatFashion(STAT_TYPE type) :StatDay(type)
  {}
  virtual void loadDb() {}
  virtual void saveDb() {}
  virtual void  v_onTimePass(DWORD curSec);
  virtual void insertData(const StatKey& key, const StatInfo& info, bool isNew);

private:
  //key = 0;    //itemid 
  //value1 = 0; //装备玩家数量
  //value2 = 0; //拥有玩家数量
  std::map<DWORD/*itemid*/, std::pair<DWORD/*装备数量*/, DWORD/*拥有玩家数量*/>> m_mapData;
};


class StatMgr:public xSingleton<StatMgr>
{
public:
  ~StatMgr();
  bool init();
  void v_final();
  void insertData(const StatCmd& rev);
  void timeTick(DWORD curSec);
  void expire();
private:
  void reg(STAT_TYPE type, STAT_SAVE_TYPE saveType, AddHandler);
  bool loadDb();
  bool saveDb();
  bool checkType(STAT_TYPE type) {
    if (type >= ESTATTYPE_BEGIN && type < ESTATTYPE_END)
      return true;
    return false;
  }
  //new handler
  void defNewCallback(StatInfo&);
  //add handler
  void defAddCallback(StatInfo&, const StatInfo&) {}
  void sumAddCallback(StatInfo&, const StatInfo&);
  void avgAddCallback(StatInfo&, const StatInfo&);
  void minAddCallback(StatInfo&, const StatInfo&);
  void maxAddCallback(StatInfo&, const StatInfo&);
  /*只记录最近的*/
  void lastAddCallback(StatInfo&, const StatInfo&);

private:
  std::map<STAT_TYPE, StatBase*> m_mapStat;
};
