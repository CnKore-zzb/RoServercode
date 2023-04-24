#include "StatMgr.h"
#include "xTime.h"
#include "xDBConnPool.h"
#include "StatServer.h"
#include "ItemConfig.h"

size_t StatKeyHash::operator()(const StatKey& a) const
{
  std::stringstream ss;
  ss << a.type << "|" << a.key << "|" << a.subKey << "|" << a.subKey2 << "|" << a.level << "|" << a.time;

  std::string str = ss.str();
  std::hash<std::string> h;
  QWORD key = h(str);
  return key;
}

/************************************************************************/
/* StatBase                                                                     */
/************************************************************************/

StatBase::StatBase(STAT_TYPE type, STAT_SAVE_TYPE saveType) :m_type(type), m_saveType(saveType), m_oTimer(1)
{
#ifdef STAT_TEST
  m_randTime = randBetween(1, 3);
#else
  m_randTime = randBetween(30 * 60, 60 * 60);
#endif // STAT_TEST
}

StatBase::~StatBase()
{
  pushDb();
}

void StatBase::loadDb()
{
  DWORD curSec = now();
  loadTime(curSec);

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_NORMAL);
  if (field)
  {
    XINF << "[统计-加载数据库] 开始加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable << m_type << XEND;

    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "zoneid = %u AND time >= %u AND type = %u", thisServer->getZoneID(), m_time, m_type);

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
    if (ret == QWORD_MAX)
    {
      XERR << "[统计-数据库] 数据库查询错误 table: database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
      return ;
    }
    for (DWORD i = 0; i < ret; ++i)
    {
      STAT_TYPE type = static_cast<STAT_TYPE>(set[i].get<DWORD>("type"));
      QWORD key = set[i].get<QWORD>("skey");
      QWORD subkey = set[i].get<QWORD>("subkey");
      QWORD subkey2 = set[i].get<QWORD>("subkey2");
      DWORD level = set[i].get<DWORD>("level");

      QWORD value1 = set[i].get<QWORD>("value1");
      
      DWORD count = set[i].get<DWORD>("count");
      bool isFloat = set[i].get<bool>("isfloat");
      StatInfo statInfo(type, key, subkey, subkey2, level, value1, count, isFloat);
      StatKey statKey(type, key, subkey, subkey2, level, m_time);
      insertData(statKey, statInfo, false);
    }
    XINF << "[统计-加载数据库] 结束加载数据库 database:" << field->m_strDatabase << "table:" << field->m_strTable.c_str() << m_type << XEND;
  }
  else
  {
    XERR << "[统计-加载数据库] 数据库错误" << XEND;
    return ;
  }
}

void StatBase::saveDb()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_NORMAL);
  if (!field)
    return ;

  xRecord rec(field);
  for (auto it = m_mapResult.begin(); it != m_mapResult.end(); ++it)
  {
    if (!it->second.needUpdate)
      continue;
    rec.put("zoneid", thisServer->getZoneID());
    rec.put("type", it->second.type);
    rec.put("skey", it->second.key);
    rec.put("time", m_time);
    rec.put("subkey", it->second.subKey);
    rec.put("subkey2", it->second.subKey2);
    rec.put("level", it->second.level);
    rec.put("value1", it->second.value1);
    rec.put("count", it->second.count);
    rec.put("isfloat", it->second.isFloat);
    m_recordQueue.push(rec);
    XDBG << "[统计-插入缓存] size:" << m_recordQueue.size() << XEND;
    it->second.needUpdate = false;
  }  
}

void StatBase::timeTick(DWORD curSec)
{
  if (curSec >= m_nextTime)
  {    
    if (checkTimePass(curSec))
    {
      onTimePass(curSec);  //save db and clear
    }
    else
    { //save db per hour
      saveDb();
    }
    m_nextTime =curSec +  m_randTime;
  }
  //exec
  if (m_oTimer.timeUp(curSec))
  {
    delDb(curSec);
  }

  pushDb();
}

void StatBase::pushDb()
{
  xRecordSet set;
  static DWORD SET_MAX_SIZE = 1000;
  while (m_recordQueue.size())
  {
    xRecord rec = m_recordQueue.front();
    set.push(rec);
    m_recordQueue.pop();
    if (set.size() >= SET_MAX_SIZE)
      break;
  }

  if (set.size() > 0)
  {
    QWORD retcode = thisServer->getDBConnPool().exeInsertSet(set, true);
    if (retcode == QWORD_MAX)
    {
      XERR << "[统计-保存数据库]" << "失败 type:" << m_type  << "size:" << set.size() << "remain queue size:" << m_recordQueue.size() << XEND;
    }
    else
    {
      XLOG << "[统计-保存数据库]" << "成功 type:"<< m_type <<"size:" << set.size() <<"remain queue size:"<<m_recordQueue.size() << XEND;
    }
  }
}

void StatBase::onTimePass(DWORD curSec)
{
  saveDb();  
  v_onTimePass(curSec);
  m_mapResult.clear();
  loadTime(curSec);
  m_bDel = true;  
}

void StatBase::delDb(DWORD curSec)
{
  if (m_bDel == false)
    return;
  //delay 2 hour
  if (curSec <= m_time + 2 * HOUR_T)
  {
    return;
  }

#ifdef STAT_TEST
  return;
#endif // STAT_TEST
  
  static DWORD LIMIT_COUNT = 5000;
  //del db
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_TABLE_NORMAL);
  if (field)
  {
    QWORD affectRows = 0;
    DWORD t = m_time - 60 * DAY_T;     //60天前的数据删掉
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "zoneid = %u AND type = %u AND time <= %u limit %u ", thisServer->getZoneID(), m_type, t, LIMIT_COUNT);
    affectRows = thisServer->getDBConnPool().exeDelete(field, where);
    XLOG << "[统计-数据库] 删除之前的数据 where " << where << "affect rows" << affectRows << XEND;
    if (affectRows != QWORD_MAX && affectRows < LIMIT_COUNT)
      m_bDel = false;
  }
}

void StatBase::insertData(const StatKey& key, const StatInfo& info, bool isNew)
{
  auto it = m_mapResult.find(key);
  if (it == m_mapResult.end())
  {
    it = m_mapResult.insert(std::make_pair(key, info)).first;
    if (isNew)
      m_newHandler(it->second);
    return;
  }
  m_addHandler(it->second, info);
}

void StatBase::insertData(const StatCmd& rev)
{
  DWORD curSec = now();
  if (checkTimePass(curSec))
    onTimePass(curSec);

  STAT_TYPE type = static_cast<STAT_TYPE>(rev.type());
  StatKey statKey;
  statKey.type = type;
  statKey.key = rev.key();
  statKey.subKey = rev.subkey();
  statKey.subKey2 = rev.subkey2();
  statKey.level = rev.level();
  statKey.time = m_time;

  StatInfo statInfo;
  statInfo.type = type;
  statInfo.key = rev.key();
  statInfo.subKey = rev.subkey();
  statInfo.subKey2 = rev.subkey2();
  statInfo.level = rev.level();
  statInfo.value1 = rev.value1();
  statInfo.value2 = rev.value2();
  statInfo.isFloat = rev.isfloat();
  statInfo.needUpdate = true;
  insertData(statKey, statInfo, true);
}

/************************************************************************/
/* StatDay                                                                     */
/************************************************************************/
void StatDay::loadTime(DWORD curSec)
{
  m_time = xTime::getDayStart(curSec - OFFSET_HOUR);
}

bool StatDay::checkTimePass(DWORD curSec)
{
#ifdef STAT_TEST
  return true;
#endif

  DWORD t = xTime::getDayStart(curSec - OFFSET_HOUR);
  if (t >= m_time + DAY_T)
    return true;
  return false;
}

/************************************************************************/
/* StatWeek                                                                   */
/************************************************************************/
void StatWeek::loadTime(DWORD curSec)
{
  m_time = xTime::getWeekStart(curSec - OFFSET_HOUR);
}

bool StatWeek::checkTimePass(DWORD curSec)
{
#ifdef STAT_TEST
  return true;
#endif
  DWORD t = xTime::getWeekStart(curSec - OFFSET_HOUR);
  if (t >= m_time + WEEK_T)
    return true;
  return false;
}

/************************************************************************/
/* StatSKillDamage                                                                   */
/************************************************************************/
void StatSKillDamage::insertData(const StatKey& key, const StatInfo& info, bool isNew)
{ 
  std::list<StatInfo>& rList = m_mapSKillData[info.key];
  rList.push_back(info);
}
void StatSKillDamage::v_onTimePass(DWORD curSec)
{
  //计算，输出结果
  if (m_mapSKillData.empty())
  {
    XLOG << "[统计-技能伤害] 没有数据，type" << m_type << XEND;
    return;
  }
  //最大伤害的几成
  auto getRange = [&](DWORD damage, DWORD maxDamage)
  {
    return damage / maxDamage * 10;
  };

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "stat_result");
    
  for (auto&m : m_mapSKillData)
  {
    StatInfo* pMaxInfo = nullptr;
    for (auto&l : m.second)
    {
      if (pMaxInfo == nullptr)
      {
        pMaxInfo = &l;
        continue;
      }
      if (l.value1 > pMaxInfo->value1)
        pMaxInfo = &l;
    }

    std::map<DWORD,DWORD> mapCnt;
    for (auto &l : m.second)
    {
      DWORD n = getRange(l.value1, pMaxInfo->value1);
      mapCnt[n]++;
    }

    stringstream ss;
    for (auto it = mapCnt.begin(); it != mapCnt.end(); ++it)
    {
      ss << " 伤害段数" << it->first << "人数" << it->second;
    }
    XLOG << "[统计-技能伤害] type" << m_type << "技能" << m.first << "最高伤害" << pMaxInfo->value1 << "释放玩家id" << pMaxInfo->subKey << "目标id" << pMaxInfo->subKey2 <<"伤害"<< ss.str() <<XEND;
    
    if (field)
    {
      xRecord record(field);
      record.put("type", m_type); 
      record.put("time", m_time);
      record.put("skey", m.first);  //技能id
      record.put("subkey", 0);
      record.put("q1", pMaxInfo->value1);   //最高伤害
      record.put("q2", pMaxInfo->subKey);   //释放id
      record.put("q3", pMaxInfo->subKey2);  //目标id
      record.put("q4", mapCnt[0]);
      record.put("q5", mapCnt[1]);
      record.put("q6", mapCnt[2]);
      record.put("q7", mapCnt[3]);
      record.put("q8", mapCnt[4]);
      record.put("q9", mapCnt[5]);
      record.put("q10", mapCnt[6]);
      record.put("q11", mapCnt[7]);
      record.put("q12", mapCnt[8]);
      record.put("q13", mapCnt[9]);
      record.put("q14", mapCnt[10]);
      QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, true);
      if (ret == QWORD_MAX)
      {
        XERR << "[统计-技能伤害] 插入数据库失败 type" << m_type << "技能" << m.first << "最高伤害" << pMaxInfo->value1 << "释放玩家id" << pMaxInfo->subKey << "目标id" << pMaxInfo->subKey2 << XEND;
      }
    }  
  }  
  m_mapSKillData.clear();
}

/************************************************************************/
/* StatFashion                                                                   */
/************************************************************************/
void StatFashion::insertData(const StatKey& key, const StatInfo& info, bool isNew)
{
  auto &pr = m_mapData[info.key];
  pr.first += info.value1;
  pr.second += info.value2;
}
void StatFashion::v_onTimePass(DWORD curSec)
{
  //计算，输出结果
  if (m_mapData.empty())
    return;
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "stat_result");
  for (auto&m : m_mapData)
  {
    if (field)
    {
      xRecord record(field);
      record.put("type", m_type);
      record.put("time", m_time);
      record.put("skey", m.first);          //道具id
      record.put("subkey", 0);
      const SItemCFG*pCfg = ItemConfig::getMe().getItemCFG(m.first);
      if (!pCfg)
        continue;

      record.put("q1", pCfg->eItemType);   //itemtype
      record.put("q2", m.second.first);     //装备人数
      record.put("q3", m.second.second);    //拥有人数
         
      QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, true);
      if (ret == QWORD_MAX)
      {
        XERR << "[统计-时装装备] 插入数据库失败 type" << m_type << "itemid" << m.first << m.second.first <<m.second.second << XEND;
        continue;
      }
      XLOG << "[统计-时装装备] type" << m_type << "道具id" << m.first <<"itemtype" <<pCfg->eItemType << "装备人数" << m.second.first << "拥有人数" << m.second.second << XEND;
    }
  }
  m_mapData.clear();
}


/************************************************************************/
/* StatMgr                                                                     */
/************************************************************************/
StatMgr::~StatMgr()
{
  v_final();
}

bool StatMgr::init()
{ 
  // NEW handler
  
  //add handler
  reg(ESTATTYPE_MONSTER_KILL, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_MONSTER_ITEM_REWARD, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_MONSTER_EXP_REWARD, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_MIN_TRADE_PRICE, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, minAddCallback));
  reg(ESTATTYPE_AVG_TRADE_PRICE, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, avgAddCallback));
  reg(ESTATTYPE_MAX_TRADE_PRICE, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, maxAddCallback));
  reg(ESTATTYPE_TRADE_SELL, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_TRADE_BUY, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_WANT_QUEST_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_SEAL_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_DAILY_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_YJS_TRY_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_YJS_PASS_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_ROB_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_NORMAL_KILL_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_MINI_KILL_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_MVP_KILL_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_REFINE_USER_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_REFINE_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_REFINE_SUCCESS_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_REFINE_FAIL_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_REFINE_DAMAGE_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_REPAIR_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_REFINE_LEVEL_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_ENCHANT_USER_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_ENCHANT_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_TOWER_COUNT, STAT_SAVE_TYPE_WEEK, HELP_BIND(StatMgr, this, sumAddCallback));            //按周统计
  reg(ESTATTYPE_DOJO_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_ITEM_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_PROPS_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_INCOME_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_CONSUME_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_DIE_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_TEAM_TIME, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_NANMEN_TIME, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_CREATE_GUILD_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_JOIN_GUILD_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_CHAT_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_PHOTO_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_DANMU_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_CARIIER_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_MAX_ATTR, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, maxAddCallback));
  reg(ESTATTYPE_AVG_ATTR, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, avgAddCallback));

  reg(ESTATTYPE_SKILL_LEARN, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_HEAL_COUNT_USER_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_REMAIN_ZENY_SUM, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, lastAddCallback));

  reg(ESTATTYPE_ONLINE_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_ONLINE_TIME, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_RESET_POINT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_RESET_POINT_SUM, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_HEAL_COUNT_USER_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_HEAL_COUNT_SUM, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_STRENGTH_USER_COUNT, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_STRENGTH_SUM, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_EQUIP_REFINE_DAMAGE_COUNT,  STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_EQUIP_COMPOSE_COUNT,  STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_EQUIP_EXCHANGE_COUNT ,  STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  reg(ESTATTYPE_VISIT_CAT_COUNT,  STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_VISIT_CAT_USER_COUNT,  STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_SKILL_DAMAGE_USER, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_SKILL_DAMAGE_MONSTER, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));
  reg(ESTATTYPE_FASHION, STAT_SAVE_TYPE_DAY, HELP_BIND(StatMgr, this, sumAddCallback));

  loadDb();
  expire();
  return true;
}

void StatMgr::v_final()
{
  saveDb();
  for (auto it = m_mapStat.begin(); it != m_mapStat.end(); ++it)
  {
    if (it->second)
    {
      SAFE_DELETE(it->second);
    }
  }
  m_mapStat.clear();
}

bool StatMgr::loadDb()
{
  for (auto it = m_mapStat.begin(); it != m_mapStat.end(); ++it)
  {
    if (it->second)
    {
      it->second->loadDb();
    }
  }  
  return true;
}

bool StatMgr::saveDb()
{  
  for (auto it = m_mapStat.begin(); it != m_mapStat.end(); ++it)
  {
    if (it->second)
    {
      it->second->saveDb();
    }
  }
  return true;
}

//帧 调用
void StatMgr::timeTick(DWORD curSec)
{ 
  for (auto it = m_mapStat.begin(); it != m_mapStat.end(); ++it)
  {
    if (it->second)
    {
      it->second->timeTick(curSec);
    }
  }
}

// 过期处理
void StatMgr::expire()
{
  DWORD curTime = xTime::getCurSec();
  XLOG << "[过期处理-开始] time:" << curTime << XEND;

  // 删除指定数据
  auto del_data = [&](std::string table, std::string where)
  {
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, table);
    if(!field)
    {
      XERR << "[过期处理-删除数据] 失败,获取xField失败  table:" << table << "where:" << where << XEND;
      return;
    }

    QWORD rows = thisServer->getDBConnPool().exeDelete(field, where.c_str());
    if(QWORD_MAX == rows)
    {
      XERR << "[过期处理-删除数据] 失败,删除邮件失败  table:" << table << "where:" << where << XEND;
      return;
    }

    XLOG << "[过期处理-删除数据] 删除邮件成功，table:" << table << "where:" << where << "rows:" << rows << XEND;
  };

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "sysmail");
  if (pField == nullptr)
  {
    XERR << "[过期处理-sysmail] 查询失败,未找到sysmail数据库表" << XEND;
    return;
  }

  char where[64];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "time + %u <= %u", EXPIRE_MAX_TIME, curTime);

  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, where);
  if (QWORD_MAX == retNum)
  {
    XERR << "[过期处理-sysmail] 查询失败 ret :" << retNum << XEND;
    return;
  }

  // 删除过期sysmail关联的mail
  for (QWORD q = 0; q < retNum; ++q)
  {
    QWORD qwSysID = set[q].get<QWORD>("sysid");
    char where_sysid[64];
    bzero(where_sysid, sizeof(where_sysid));
    snprintf(where_sysid, sizeof(where_sysid), "sysid = %llu ", qwSysID);

    del_data("mail", where_sysid);
  }

  // 删除过期sysmail
  del_data("sysmail", where);

  // 删除过期mail
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "time + %u <= %u and mailtype != 2 ", EXPIRE_MAX_TIME, curTime);
  del_data("mail", where);

  // 删除过期offmsg
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "time + %u <= %u ", EXPIRE_MAX_TIME, curTime);
  del_data("offmsg", where);

  XLOG << "[过期处理-结束] time:" << xTime::getCurSec() << XEND;
}

void StatMgr::insertData(const StatCmd& rev)
{
  STAT_TYPE type = static_cast<STAT_TYPE>(rev.type());
  if (!checkType(type))
    return;
  auto it = m_mapStat.find(type);
  if (it == m_mapStat.end())
    return;
  
  StatBase* pBase = it->second;
  if (!pBase)
    return;
  pBase->insertData(rev);
}

void StatMgr::reg(STAT_TYPE type, STAT_SAVE_TYPE saveType, AddHandler handler)
{
  auto it = m_mapStat.find(type);
  if (it != m_mapStat.end())
    return;
  StatBase* pBase = nullptr;

  if (type == ESTATTYPE_SKILL_DAMAGE_USER || type == ESTATTYPE_SKILL_DAMAGE_MONSTER)
  {
    pBase = NEW StatSKillDamage(type);
  }
  else if (type == ESTATTYPE_FASHION)
  {
    pBase = NEW StatFashion(type);
  }
  else
  {
    if (saveType == STAT_SAVE_TYPE_DAY)
      pBase = NEW StatDay(type);
    if (saveType == STAT_SAVE_TYPE_WEEK)
      pBase = NEW StatWeek(type);
  }
  if (!pBase)
    return;
  pBase->setNewHandler(HELP_BIND1(StatMgr, this, defNewCallback));
  pBase->setAddHandler(handler);
  m_mapStat[type] = pBase;
}

// NEW handler
void StatMgr::defNewCallback(StatInfo& aInfo)
{
  aInfo.count = 1;
}

//add handler
void StatMgr::sumAddCallback(StatInfo&aInfo, const StatInfo&bInfo)
{
  aInfo.value1 += bInfo.value1;
  aInfo.count ++;
  aInfo.needUpdate = true;
}

void StatMgr::avgAddCallback(StatInfo&aInfo, const StatInfo&bInfo)
{
  aInfo.value1 = (aInfo.value1 * aInfo.count + bInfo.value1) / (aInfo.count + 1);   //新的平均值
  aInfo.count++;
  aInfo.needUpdate = true;
}

void StatMgr::minAddCallback(StatInfo&aInfo, const StatInfo&bInfo)
{
  if (aInfo.value1 > bInfo.value1)
  {
    aInfo.value1 = bInfo.value1;
    aInfo.needUpdate = true;
    aInfo.count++;
  }
}

void StatMgr::maxAddCallback(StatInfo&aInfo, const StatInfo&bInfo)
{
  if (aInfo.value1 < bInfo.value1)
  {
    aInfo.value1 = bInfo.value1;
    aInfo.needUpdate = true;
    aInfo.count++;
  }
}

void StatMgr::lastAddCallback(StatInfo&aInfo, const StatInfo&bInfo)
{
  aInfo.value1 = bInfo.value1;
  aInfo.needUpdate = true;
  aInfo.count++;
}
