#include "StatUserData.h"
#include "StatServer.h"
#include "xDBConnPool.h"
#include "SessionCmd.pb.h"
#include "xNetProcessor.h"
#include "ItemConfig.h"

StatUserData::StatUserData() : m_oOneMinTimer(60)
{

}

StatUserData::~StatUserData()
{
  saveDb();
}

bool SStatkillMonster::saveDb(DWORD date)
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, STAT_KILL_DB);
  if (field == nullptr)
    return false;
  xRecordSet set;
  xRecord rec(field);
  DWORD rank = 1;
  for (auto &l : listMonsterData)
  {
    rec.put("time", date);
    rec.put("rank", rank++);
    rec.put("monsterid", l.dwMonsterID);
    rec.put("num", l.dwKillNum);
    rec.put("userid", l.qwCharID);
    rec.put("zone", l.dwZoneID);
    rec.put("profession", l.dwProfessionID);
    set.push(rec);
  }
  bUpdate = false;
  QWORD ret = thisServer->getDBConnPool().exeInsertSet(set, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[杀怪-统计], 保存失败, 怪物id:" << dwMonsterID << XEND;
    return false;
  }
  return true;
}

bool SDayZenyCount::saveDb(DWORD date)
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, STAT_DAY_GET_ZENY_TOP);
  if (field == nullptr)
      return false;
  xRecord rec(field);

  rec.put("time", date);
  rec.put("charid", dwCharID);
  rec.putString("name", strName);
  rec.put("baselv", dwBaseLv);
  rec.put("joblv", dwJobLv);
  rec.put("profession", dwProfession);
  rec.put("normalzeny", qwNormalZeny);
  rec.put("chargezeny", qwChargeZeny);
  rec.put("totalzeny", qwTotalZeny);

  QWORD ret = thisServer->getDBConnPool().exeInsert(rec, false, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[Zeny-统计], 保存失败, 时间" << date << dwCharID << strName << qwTotalZeny << qwNormalZeny << qwChargeZeny << XEND;
    return false;
  }
  return true;
}

bool SDayZenyCount::loadDb(DWORD date)
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, STAT_DAY_GET_ZENY_TOP);
  if (field == nullptr)
    return false;
  XLOG << "[Zeny-统计], 加载数据库成功" << field->m_strDatabase << field->m_strTable << XEND;

  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "time = %u", date);

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
  if (ret > 1)
  { 
    XERR << "[Zeny-统计], 加载数据数据失败, date:" << date << XEND;
    return false;
  }
  
  for (DWORD i = 0; i < ret; ++i)
  {
    const auto d = set[i];

    dwCharID = d.get<QWORD>("charid");
    strName = d.getString("name");
    dwBaseLv = d.get<DWORD>("baselv");
    dwJobLv = d.get<DWORD>("joblv");
    dwProfession = d.get<DWORD>("profession");
    qwNormalZeny = d.get<QWORD>("normalzeny");
    qwChargeZeny = d.get<QWORD>("chargezeny");
    qwTotalZeny = d.get<QWORD>("totalzeny");
  }
  XLOG << "[Zeny-统计], 加载数据成功, 共加载:" << ret << "条数据" << XEND;
  return true;
}

void SDayZenyCount::clear()
{
  dwCharID = 0;
  strName = "";
  dwBaseLv = 0;
  dwJobLv = 0;
  dwProfession = 0;
  qwNormalZeny = 0;
  qwChargeZeny = 0;
  qwTotalZeny = 0;
}

bool StatUserData::loadDb()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, STAT_KILL_DB);
  if (field == nullptr)
    return false;
  XLOG << "[杀怪-统计], 加载数据库成功" << field->m_strDatabase << field->m_strTable << XEND;

  DWORD date = getFormatDate();
  m_dwCurDataDate = date;
  DWORD offsetdate = getOffsetDate(6 * 60 * 60);
  m_dwZenyCountCurDayDate = offsetdate;
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "time = %u", date);

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[杀怪-统计], 加载数据数据失败, date:" << date << XEND;
    return false;
  }

  for (DWORD i = 0; i < ret; ++i)
  {
    const auto d = set[i];

    SKillMonsterData stData;
    stData.dwMonsterID = d.get<DWORD>("monsterid");
    stData.dwKillNum = d.get<DWORD>("num");
    stData.qwCharID = d.get<QWORD>("userid");
    stData.dwProfessionID = d.get<DWORD>("profession");
    stData.dwZoneID = d.get<DWORD>("zone");
    auto it = m_mapStatKillMonster.find(stData.dwMonsterID);
    if (it == m_mapStatKillMonster.end())
    {
      m_mapStatKillMonster[stData.dwMonsterID].dwMonsterID = stData.dwMonsterID;
      it = m_mapStatKillMonster.find(stData.dwMonsterID);
    }
    it->second.addStatData(stData);
    it->second.bUpdate = false;
  }
  XLOG << "[杀怪-统计], 加载数据成功, 共加载:" << ret << "条数据" << XEND;

  m_oDayZenyCount.loadDb(m_dwZenyCountCurDayDate);
  loadWorldLevelDb();
  return true;
}

bool StatUserData::loadWorldLevelDb()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, STAT_WORLD_LEVEL_DB);
  if (field == nullptr)
    return false;
  XLOG << "[世界等级-统计], 加载数据库成功" << field->m_strDatabase << field->m_strTable << XEND;

  DWORD dwMinCalcLevelTime = now() - MAX_TOTAL_LEVEL_DAY * 86400;
  DWORD dwMinCalcLevelDate = getFormatDate(dwMinCalcLevelTime);
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "time >= %u", dwMinCalcLevelDate);

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[世界等级-统计], 加载数据数据失败, date:" << dwMinCalcLevelDate << XEND;
    return false;
  }

  for (DWORD i = 0; i < ret; ++i)
  {
    const auto d = set[i];
    STotalLevelDayData stData;
    stData.dwDate = d.get<DWORD>("time");
    stData.dwTotalBaseLevel = d.get<DWORD>("totalbaselevel");
    stData.dwTotalBaseLevelCount = d.get<DWORD>("totalbaselevelcount");
    stData.dwTotalJobLevel = d.get<DWORD>("totaljoblevel");
    stData.dwTotalJobLevelCount = d.get<DWORD>("totaljoblevelcount");

    m_mapTotalLevelDayData[stData.dwDate] = stData;
  }

  calcWorldLevel();
  XLOG << "[世界等级-统计], 加载数据成功, 共加载:" << ret << "条数据" << XEND;
  return true;
}

bool StatUserData::saveDb()
{
  for (auto &m : m_mapStatKillMonster)
    m.second.saveDb(m_dwCurDataDate);

  m_oDayZenyCount.saveDb(m_dwZenyCountCurDayDate);
  saveWorldLevelDb();
  savePetWear(m_dwZenyCountCurDayDate);
  return true;
}

bool StatUserData::saveWorldLevelDb()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, STAT_WORLD_LEVEL_DB);
  if (field == nullptr)
    return false;
  xRecordSet set;
  xRecord rec(field);
  for (auto it : m_mapTotalLevelDayData)
  {
    rec.put("time", it.first);
    rec.put("totalbaselevel", it.second.dwTotalBaseLevel);
    rec.put("totalbaselevelcount", it.second.dwTotalBaseLevelCount);
    rec.put("totaljoblevel", it.second.dwTotalJobLevel);
    rec.put("totaljoblevelcount", it.second.dwTotalJobLevelCount);
    set.push(rec);
  }
  QWORD ret = thisServer->getDBConnPool().exeInsertSet(set, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[世界等级-统计], 保存失败" << XEND;
    return false;
  }
  return true;
}

bool StatUserData::savePetWear(DWORD date)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, STAT_PET_WEAR);
  if (pField == nullptr)
  {
    XERR << "[统计-宠物装扮] 时间 :" << date << "保存失败,未找到" << STAT_PET_WEAR << "数据库表" << XEND;
    return false;
  }

  map<EQualityType, map<DWORD, DWORD>> mapStat;
  for (auto &m : m_mapPetWearStat)
  {
    const SPetWearStat& rStat = m.second;
    for (auto &item : rStat.mapQualityCount)
    {
      map<DWORD, DWORD>& mapCount = mapStat[item.first];
      ++mapCount[item.second];
    }
  }

  bool bSave = false;
  auto put_record = [&](EQualityType eType, const string& key, xRecord& record)
  {
    map<DWORD, DWORD>& mapCount = mapStat[eType];
    if (mapCount.empty() == true)
      return;

    Json::Value value;
    for (auto &m : mapCount)
    {
      stringstream sstr;
      string count1;
      sstr << m.first;
      count1 = sstr.str();

      value[count1] = m.second;
    }

    Json::FastWriter writer;
    record.putString(key.c_str(), writer.write(value));

    bSave = true;
  };

  if (!bSave)
    return true;

  xRecord record(pField);
  record.put("time", date);

  put_record(EQUALITYTYPE_WHITE, "white", record);
  put_record(EQUALITYTYPE_GREEN, "green", record);
  put_record(EQUALITYTYPE_BLUE, "blue", record);
  put_record(EQUALITYTYPE_PURPLE, "purple", record);
  put_record(EQUALITYTYPE_ORANGE, "orange", record);
  put_record(EQUALITYTYPE_GOLD, "gold", record);
  put_record(EQUALITYTYPE_DARKGOLD, "darkgold", record);

  QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[统计-宠物装扮] 时间 :" << date << "保存失败,ret :" << ret << XEND;
    return false;
  }

  return true;
}

void StatUserData::calcWorldLevel()
{
  QWORD qwTotalBaseLevel = 0;
  QWORD qwTotalBaseLevelCount = 0;
  QWORD qwTotalJobLevel = 0;
  QWORD qwTotalJobLevelCount = 0;

  DWORD dwCurTime = now();
  for (DWORD i = 0; i < MAX_AVR_LEVEL_DAY; i++)
  {
    dwCurTime -= 86400;
    DWORD dwFormatDate = getFormatDate(dwCurTime);
    auto it = m_mapTotalLevelDayData.find(dwFormatDate);
    if (it == m_mapTotalLevelDayData.end())
      continue;
    qwTotalBaseLevel += it->second.dwTotalBaseLevel;
    qwTotalBaseLevelCount += it->second.dwTotalBaseLevelCount;
    qwTotalJobLevel += it->second.dwTotalJobLevel;
    qwTotalJobLevelCount += it->second.dwTotalJobLevelCount;
  }

  m_dwAvrWorldBaseLevel = 0;
  m_dwAvrWorldJobLevel = 0;
  if (qwTotalBaseLevelCount > 0)
    m_dwAvrWorldBaseLevel = (DWORD)(qwTotalBaseLevel / qwTotalBaseLevelCount);
  if (qwTotalJobLevelCount > 0)
    m_dwAvrWorldJobLevel = (DWORD)(qwTotalJobLevel / qwTotalJobLevelCount);

  XLOG << "[世界等级-计算]" << "平均base等级" << m_dwAvrWorldBaseLevel << "平均job等级" << m_dwAvrWorldJobLevel << XEND;
}

void StatUserData::sendWorldLevel(DWORD dwZoneId/* = 0*/, xNetProcessor *np/* =nullptr*/)
{
  SyncWorldLevelSessionCmd cmd;
  cmd.set_base_worldlevel(m_dwAvrWorldBaseLevel);
  cmd.set_job_worldlevel(m_dwAvrWorldJobLevel);

  PROTOBUF(cmd, send, len);
  if (dwZoneId > 0 && np != nullptr)
  {
    bool ret = np->sendCmd(send, len);
    XLOG << "[世界等级-同步] 同步到session" << dwZoneId << "np" << np << "ret" << ret << XEND;
  }
  else
  {
    bool ret = thisServer->broadCmdToAll(send, len);
    XLOG << "[世界等级-同步] 同步到所有session" << "ret" << ret << XEND;
  }
}

bool StatUserData::init()
{
  if (loadDb() == false)
    return false;
  return true;
}

void StatUserData::timeTick(DWORD curTime)
{
  if (m_oOneMinTimer.timeUp(curTime))
  {
    DWORD nowdate = getFormatDate();
    DWORD offsetSec = 6 * 60 * 60;
    DWORD nowoffsetdate = getOffsetDate(offsetSec);
    if (nowdate != m_dwCurDataDate) //隔天
    {
      for (auto &m : m_mapStatKillMonster)
        m.second.saveDb(m_dwCurDataDate);
      saveWorldLevelDb();
      calcWorldLevel();
      sendWorldLevel();
      m_dwCurDataDate = nowdate;
      XLOG << "[杀怪-统计], 隔天保存, 当前时间:" << m_dwCurDataDate << XEND;
      XLOG << "[世界等级-统计], 隔天保存, 当前时间:" << m_dwCurDataDate << XEND;
      m_mapStatKillMonster.clear();
    }
    else
    {
      for (auto &m : m_mapStatKillMonster)
      {
        if (m.second.bUpdate && m.second.dwNextUpdateTime >= curTime)
          m.second.saveDb(m_dwCurDataDate);
      }
      saveWorldLevelDb();
    }

    if (nowoffsetdate != m_dwZenyCountCurDayDate)
    {
      m_oDayZenyCount.saveDb(m_dwZenyCountCurDayDate);
      savePetWear(m_dwZenyCountCurDayDate);
      XLOG << "[Zeny-统计], 隔天保存, 当前日期:" << m_dwZenyCountCurDayDate << "当前时间:" << curTime << "最大获取数量:" << m_oDayZenyCount.qwTotalZeny << m_oDayZenyCount.dwCharID << XEND;
      m_oDayZenyCount.clear();
      m_mapPetWearStat.clear();
      m_dwZenyCountCurDayDate = nowoffsetdate;
    }
    else
    {
      m_oDayZenyCount.saveDb(m_dwZenyCountCurDayDate);
      savePetWear(m_dwZenyCountCurDayDate);
    }
  }
}

void StatUserData::addUserKillMonsterCnt(const KillMonsterNumStatCmd& cmd)
{
  //auto addKillData = []
  for (int i = 0; i < cmd.killmonster_size(); ++i)
  {
    DWORD monsterid = cmd.killmonster(i).monsterid();
    DWORD num = cmd.killmonster(i).killnum();

    auto it = m_mapStatKillMonster.find(monsterid);
    if (it == m_mapStatKillMonster.end())
    {
      m_mapStatKillMonster[monsterid].dwMonsterID = monsterid;
      it = m_mapStatKillMonster.find(monsterid);
    }
    if (it->second.checkNoAdd(num))
      continue;

    SKillMonsterData stData;
    stData.dwMonsterID = monsterid;
    stData.dwKillNum = num;
    stData.qwCharID = cmd.userid();
    stData.dwZoneID = cmd.zoneid();
    stData.dwProfessionID = cmd.professionid();

    it->second.addStatData(stData);
  }
}

void StatUserData::updateDayGetZenyCount(const DayGetZenyCountCmd& cmd)
{
  if (m_oDayZenyCount.qwTotalZeny < cmd.normal_zeny() + cmd.charge_zeny())
  {
    m_oDayZenyCount.dwCharID = cmd.userid();
    m_oDayZenyCount.strName = cmd.username();
    m_oDayZenyCount.dwBaseLv = cmd.baselv();
    m_oDayZenyCount.dwJobLv = cmd.joblv();
    m_oDayZenyCount.dwProfession = cmd.profession();
    //20180706 cmd里normal_zeny字段修改为当天总共获得的zeny
    //cmd里charge_zeny字段修改为当天通过充值购买、B格猫金币购买和打开礼包3841-10,000,000 Zeny 后获得的zeny数量
    m_oDayZenyCount.qwNormalZeny = cmd.normal_zeny() > cmd.charge_zeny() ? (cmd.normal_zeny() - cmd.charge_zeny()) : 0;
    m_oDayZenyCount.qwChargeZeny = cmd.charge_zeny();
    m_oDayZenyCount.qwTotalZeny = cmd.normal_zeny();
  }
}

void StatUserData::addTotalLevelDayData(const StatCurLevel& cmd)
{
  if (cmd.last_offlinetime() > 0)
  {
    DWORD dwLastDate = getFormatDate(cmd.last_offlinetime());
    auto it = m_mapTotalLevelDayData.find(dwLastDate);
    if (it != m_mapTotalLevelDayData.end())
    {
      it->second.dwTotalBaseLevel = it->second.dwTotalBaseLevel > cmd.last_baselv() ? it->second.dwTotalBaseLevel - cmd.last_baselv() : 0;
      it->second.dwTotalBaseLevelCount = it->second.dwTotalBaseLevelCount > 0 ? it->second.dwTotalBaseLevelCount - 1 : 0;
      it->second.dwTotalJobLevel = it->second.dwTotalJobLevel > cmd.last_joblv() ? it->second.dwTotalJobLevel - cmd.last_joblv() : 0;
      it->second.dwTotalJobLevelCount = it->second.dwTotalJobLevelCount > 0 ? it->second.dwTotalJobLevelCount - 1 : 0;
    }
  }
  
  DWORD dwCurDate = getFormatDate(cmd.cur_time());
  STotalLevelDayData& oData = m_mapTotalLevelDayData[dwCurDate];
  oData.dwDate = dwCurDate;
  oData.dwTotalBaseLevel += cmd.cur_baselv();
  oData.dwTotalBaseLevelCount += 1;
  oData.dwTotalJobLevel += cmd.cur_joblv();
  oData.dwTotalJobLevelCount += 1;
}

void StatUserData::addPetWearUseCount(const PetWearUseCountStatCmd& cmd)
{
  for (int i = 0; i < cmd.wears_size(); ++i)
  {
    const PetWear& rWear = cmd.wears(i);
    if (rWear.types_size() != rWear.counts_size())
    {
      XERR << "[统计-宠物装扮] wear :" << rWear.types_size() << "count :" << rWear.counts_size() << "数量不一致,被忽略,忽略数据" << rWear.ShortDebugString() << XEND;
      continue;
    }

    SPetWearStat& rStat = m_mapPetWearStat[rWear.charid()];
    for (int j = 0; j < rWear.types_size(); ++j)
      rStat.mapQualityCount[rWear.types(j)] += rWear.counts(j);
  }
  XDBG << "[统计-宠物装扮] 统计数据" << cmd.ShortDebugString() << XEND;
}

void SStatkillMonster::addStatData(const SKillMonsterData& data)
{
  if (listMonsterData.size() >= MAX_KILL_MONSTER_NUM)
    listMonsterData.pop_back();

  auto it = listMonsterData.begin();
  for (auto s = listMonsterData.begin(); s != listMonsterData.end(); ++s)
  {
    if (s->dwKillNum < data.dwKillNum)
    {
      it = s;
      break;
    }
  }
  listMonsterData.insert(it, data);
  if (!bUpdate)
  {
    bUpdate = true;
    if (dwRandUpdateTime == 0)
      setRandUpdateTime();
    dwNextUpdateTime = now() + dwRandUpdateTime;
  }
}

