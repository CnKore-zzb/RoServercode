#include <iostream>
#include "StatServer.h"
#include "StatCmd.pb.h"
#include "StatMgr.h"
#include <iostream>
#include <fstream>
#include <dirent.h>
#include "MailManager.h"
#include "MsgManager.h"
#include "MiscConfig.h"
#include "RecordTrade.pb.h"
#include "ItemConfig.h"
#include "ConfigManager.h"
#include "StatUserData.h"
#include "xNetProcessor.h"

StatServer::StatServer(OptArgs &args):RegionServer(args), m_oTickOneSec(1),m_oTickOneHour(60*60), m_oTickOneDay(7, 20)
{
}

StatServer::~StatServer()
{
}

void StatServer::v_final()
{
  XLOG << "[" << getServerName() << "],v_final" << XEND;
  StatMgr::getMe().v_final();
  RegionServer::v_final();
}

void StatServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  auto it = m_list.find(np);
  if (it != m_list.end())
  {
    m_list.erase(it);
    XLOG << "[连接]" << "删除" << np << XEND;
  }
  RegionServer::v_closeNp(np);
}

void StatServer::v_timetick()
{
  RegionServer::v_timetick();

  for (auto &v : m_list)
  {
    if (v)
    {
      xServer::doCmd(v);
    }
  }

  DWORD curTime = xTime::getCurSec();
  StatMgr::getMe().timeTick(curTime);
  StatUserData::getMe().timeTick(curTime);
  if (m_dbOk)
    MailManager::getMe().checkInvalidMail(curTime);

  if (m_oTickOneSec.timeUp(curTime))
  {
    if (m_oTickOneDay.timeUp(curTime))
    {
      checkTradeLogDel(curTime);
    }
    delDb();
    generateItemCode(curTime);
  }
}

bool StatServer::v_init()
{
  if (!addDataBase(REGION_DB, true))
  {
    XERR << "[数据库Trade],初始化数据库连接失败:" << REGION_DB << XEND;
    return false;
  }
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("StatServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， StatServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);
  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }
  StatMgr::getMe().init();
  StatUserData::getMe().init();
  updateMysql();
  m_dbOk = true;

  MiscConfig::getMe().loadConfig();

  if (!loadConfig())
    return false;

  return true;
}

bool StatServer::loadConfig()
{
  bool bResult = true;

  // base配置
  if (ConfigManager::getMe().loadStatConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

// 多进程; 执行后，重新加载数据库? 待做
bool StatServer::updateMysql()
{
  if (isOuter()) return true;

  DIR* dirp;
  struct dirent* direntp;
  dirp = opendir( "sql/region/" );
  string tmp ="update";
  string filename;
  std::vector<std::string> sqlFileList;
  if (dirp == nullptr)
    return false;

  while(1)
  {
    direntp = readdir(dirp);
    if (direntp == nullptr)
      break;
    string s = direntp->d_name;
    size_t x = s.find(tmp);
    if (x > 0)
    {
      sqlFileList.push_back(s);
    }
  }
  closedir(dirp);

  if (sqlFileList.empty())
    return true;
  for (auto v = sqlFileList.begin(); v != sqlFileList.end(); ++v)
  {
    if (exeSqlFile((*v).c_str()) == false)
      return false;
  }

  //更新表结构
  addDataBase(REGION_DB, false);
  return true;
}

bool StatServer::exeSqlFile(const char *file)
{
  std::stringstream realfile;
  realfile.str("");
  realfile<<"sql/region/"<<file;
  std::ifstream fin(realfile.str().c_str());
  std::stringstream sqlCmd;
  std::string tmpstr;

  DWORD size = 0;
  size_t fbegin = 0;

  bool bBlockStarted = false;
  bool bBlockEnded = false;
  string strBlockVersion;

  while(getline(fin, tmpstr))
  {
    size = tmpstr.size();
    fbegin = tmpstr.find("###");

    if (fbegin < size)
    {
      tmpstr.erase(0, fbegin + 3);

      // get block
      {
        if (bBlockStarted && strBlockVersion == tmpstr)
        {
          bBlockStarted = false;
          bBlockEnded = true;
        }
        else
        {
          bBlockStarted = true;
          bBlockEnded = false;
          strBlockVersion.clear();
          strBlockVersion = tmpstr;
          sqlCmd.str("");
        }
      }

      if (bBlockEnded)
      {
        char where[128];
        bzero(where, sizeof(where));
        snprintf(where, sizeof(where), "fileList='%s'", strBlockVersion.c_str());

        QWORD num = getDBConnPool().checkExist(REGION_DB, "updateSql", where);
        if(num == 0)
        {
          // do the update
          getDBConnPool().exeSql(REGION_DB, sqlCmd.str().c_str());

          // record version done
          std::stringstream sql_string;
          sql_string.str("");
          sql_string << "INSERT INTO " << REGION_DB << "." << "updateSql" << " (fileList) " << " VALUES ('" << strBlockVersion.c_str() << "')";
          getDBConnPool().exeSql(REGION_DB, sql_string.str().c_str());
        }
        sqlCmd.str("");
      }

      tmpstr.clear();
      continue;
    }

    // get commond list
    size = tmpstr.size();
    fbegin = tmpstr.find('#');
    if (fbegin == 0)
      continue;
    if (fbegin < size)
      tmpstr.erase(fbegin, size - 1);

    size = tmpstr.size();
    fbegin = tmpstr.find("-- ");
    if (fbegin == 0)
      continue;
    if (fbegin < size)
      tmpstr.erase(fbegin, size - 1);

    sqlCmd<< tmpstr;
    tmpstr.clear();
  }
  fin.close();

  return true;
}

bool StatServer::regist(xNetProcessor* np, DWORD zoneid, DWORD serverType)
{
  if (!np) return false;

  if (!inVerifyList(np))
    return false;

  removeVerifyList(np);

  m_list.insert(np);

  XLOG << "[区注册]" << np << XEND;
  return true;
}

bool StatServer::doStatCmd(xNetProcessor *np, const BYTE* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
    case TEST_STAT_CMD:
      {
        return true;
      }
      break;
    case STAT_CMD:
    {
      PARSE_CMD_PROTOBUF(StatCmd, rev);
      StatMgr::getMe().insertData(rev);
      if (rev.type() == ESTATTYPE_SKILL_DAMAGE_USER || rev.type() == ESTATTYPE_SKILL_DAMAGE_MONSTER || rev.type() == ESTATTYPE_FASHION )
        XLOG << "[统计-消息接收] msg:" << rev.ShortDebugString() << XEND;
      return true;
    }
    break;
    case LOG_TRADE_TO_STAT:
    {
      PARSE_CMD_PROTOBUF(TradeToStatLogCmd, rev);
      tradelog(rev);
      return true;
    }
    break;
    case STAT_KILL_MONSTER:
    {
      PARSE_CMD_PROTOBUF(KillMonsterNumStatCmd, rev);
      StatUserData::getMe().addUserKillMonsterCnt(rev);
      return true;
    }
    break;
    case STAT_DAY_GET_ZENY_COUNT:
    {
      PARSE_CMD_PROTOBUF(DayGetZenyCountCmd, rev);
      StatUserData::getMe().updateDayGetZenyCount(rev);
      return true;
    }
    break;
    case STAT_CUR_LEVEL:
    {
      PARSE_CMD_PROTOBUF(StatCurLevel, rev);
      StatUserData::getMe().addTotalLevelDayData(rev);
      return true;
    }
    break;
    case REQ_WORLD_LEVEL:
    {
      PARSE_CMD_PROTOBUF(ReqWorldLevelCmd, rev);
      StatUserData::getMe().sendWorldLevel(rev.zoneid(), np);
      return true;
    }
    break;
    case STAT_PET_WEAR_USECOUNT:
    {
      PARSE_CMD_PROTOBUF(PetWearUseCountStatCmd, rev);
      StatUserData::getMe().addPetWearUseCount(rev);
    }
    default:
      break;
  }
  return false;
}

bool StatServer::broadCmdToAll(const void* data, unsigned short len)
{
  for (auto it = m_list.begin(); it != m_list.end(); ++it)
  {
    if ((*it) == nullptr)
    {
      XERR << "[发送消息-异常] have processer is null" << XEND;
      continue;
    }
    (*it)->sendCmd(data, len);
  }
  return true;
}

bool StatServer::tradelog(const TradeToStatLogCmd& cmd)
{
  switch (cmd.elisttype())
  {
    case ETRADE_SALED_LIST:
      {
        xField* field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_SELLED_LIST);
        if (field == nullptr)
        {
          XERR << "[TradeLog], 读取数据库失败" << REGION_DB << cmd.elisttype() << XEND;
          return false;
        }
        xRecord record(field);
        record.put("logtype", cmd.etype());
        record.put("itemid", cmd.itemid());
        record.put("price", cmd.price());
        record.put("count", cmd.count());
        record.put("sellerid", cmd.sellerid());
        record.put("buyerid", cmd.buyerid());
        record.put("pendingtime", cmd.pendingtime());
        record.put("tradetime", cmd.tradetime());
        record.put("refine_lv", cmd.refinelv());

        std::string strItemData;
        if (cmd.itemdata().has_base())
        {
          cmd.itemdata().SerializeToString(&strItemData);
          if (!strItemData.empty())
          {
            record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());
          }
        }

        QWORD ret = thisServer->getTradeConnPool().exeInsert(record);
        if (ret == QWORD_MAX)
        {
          XERR << "[交易] [插入记录日志失败] database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
          return false;
        }
        else
        {
          XINF << "[交易] [插入记录日志成功] database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
          return true;
        }
      }
      break;
    case ETRADE_BUYED_LIST:
      {
        xField* field = thisServer->getTradeConnPool().getField(REGION_DB, DB_TABLE_BUYED_LIST);
        if (field == nullptr)
        {
          XERR << "[TradeLog], 读取数据库失败" << REGION_DB << cmd.elisttype() << XEND;
          return false;
        }
        xRecord record(field);
        record.put("logtype", cmd.etype());
        record.put("itemid", cmd.itemid());
        record.put("price", cmd.price());
        record.put("count", cmd.count());
        record.put("buyerid", cmd.buyerid());
        record.put("tradetime", cmd.tradetime());
        record.put("refine_lv", cmd.refinelv());
        if (cmd.buyername().empty() == false)
          record.putString("buyname", cmd.buyername());

        std::string strItemData;
        if (cmd.itemdata().has_base())
        {
          cmd.itemdata().SerializeToString(&strItemData);
          if (!strItemData.empty())
          {
            record.putBin("itemdata", (unsigned char*)strItemData.c_str(), strItemData.size());
          }
        }

        QWORD ret = thisServer->getTradeConnPool().exeInsert(record);
        if (ret == QWORD_MAX)
        {
          XERR << "[交易] [插入记录日志失败] database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
          return false;
        }
        else
        {
          XINF << "[交易] [插入记录日志成功] database:" << field->m_strDatabase << "table:" << field->m_strTable << XEND;
          return true;
        }
      }
      break;
    default:
      break;
  }
  return true;
}

void StatServer::checkTradeLogDel(DWORD curSec)
{
    //3天前的已领取记录
    DWORD offset = curSec - MiscConfig::getMe().getTradeCFG().dwLogTime;

    m_dwSellDelOffset = offset;
    m_dwBuydelOffset = offset;
    
    //一天前的公式正在购买记录
    m_dwPublicityBuyingOffset = curSec - DAY_T;


    //30天前的未领取记录
    m_dwCanTakeOffsetS = curSec - DAYS_30_T;
    m_dwCanTakeOffsetB = curSec - DAYS_30_T;
}

void StatServer::delDb()
{
#ifndef _OLD_TRADE
  return;   //使用新的交易所 立即退出
#endif // _OLD_TRADE

  if (!m_dbOk)
    return;

  static DWORD LIMIT_COUNT = 1000;
  auto delf = [&](std::string table, DWORD &offset) {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, table);
    if (!field)
    {
      XERR << "[交易-清空记录] 失败， 获取不到数据库， table：" << table << XEND;
      return;
    }

    char where[128];
    bzero(where, sizeof(where));
    //以领取的清除掉  status 0 未领取
    snprintf(where, sizeof(where), "tradetime <=%u and status = 1 limit %u", offset, LIMIT_COUNT);

    QWORD affectRows = thisServer->getTradeConnPool().exeDelete(field, where);
    if (affectRows == QWORD_MAX)
      XERR << "[交易-清空记录] 失败， 数据库操作失败" << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
    else
      XLOG << "[交易-清空记录] 成功" << "database：" << field->m_strDatabase << "table：" << field->m_strTable << "ago" << offset << "affect rows" << affectRows << XEND;

    if (affectRows != QWORD_MAX && affectRows < LIMIT_COUNT)
      offset = 0;
  };

  auto delBuying = [&](std::string table, DWORD &offset) {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, table);
    if (!field)
    {
      XERR << "[交易-清空记录-购买中] 失败， 获取不到数据库， table：" << table << XEND;
      return;
    }

    char where[128];
    bzero(where, sizeof(where));

    snprintf(where, sizeof(where), "tradetime <=%u and logtype=%u limit %u", offset, EOperType_PublicityBuying, LIMIT_COUNT);

    QWORD affectRows = thisServer->getTradeConnPool().exeDelete(field, where);
    if (affectRows == QWORD_MAX)
      XERR << "[交易-清空记录-购买中] 失败， 数据库操作失败" << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
    else
      XLOG << "[交易-清空记录-购买中] 成功" << "database：" << field->m_strDatabase << "table：" << field->m_strTable << "ago" << offset << "affect rows" << affectRows << XEND;

    if (affectRows != QWORD_MAX && affectRows < LIMIT_COUNT)
      offset = 0;
  };

  auto delAll = [&](std::string table, DWORD &offset) {
    xField *field = thisServer->getTradeConnPool().getField(REGION_DB, table);
    if (!field)
    {
      XERR << "[交易-清空记录-30天所有记录] 失败， 获取不到数据库， table：" << table << XEND;
      return;
    }

    char where[128];
    bzero(where, sizeof(where));
    //30天前的全清除
    snprintf(where, sizeof(where), "tradetime <=%u limit %u", offset, LIMIT_COUNT);

    QWORD affectRows = thisServer->getTradeConnPool().exeDelete(field, where);
    if (affectRows == QWORD_MAX)
      XERR << "[交易-清空记录-30天所有记录] 失败， 数据库操作失败" << "database：" << field->m_strDatabase << "table：" << field->m_strTable << XEND;
    else
      XLOG << "[交易-清空记录-30天所有记录] 成功" << "database：" << field->m_strDatabase << "table：" << field->m_strTable << "ago" << offset << "affect rows" << affectRows << XEND;

    if (affectRows != QWORD_MAX && affectRows < LIMIT_COUNT)
      offset = 0;
  };

  if (m_dwSellDelOffset)
    delf(DB_TABLE_SELLED_LIST, m_dwSellDelOffset);
  if (m_dwBuydelOffset)
    delf(DB_TABLE_BUYED_LIST, m_dwBuydelOffset);

  
  if (m_dwPublicityBuyingOffset)
    delBuying(DB_TABLE_BUYED_LIST, m_dwPublicityBuyingOffset);

  if (m_dwCanTakeOffsetB)
    delAll(DB_TABLE_BUYED_LIST, m_dwCanTakeOffsetB);

  if (m_dwCanTakeOffsetS)
    delAll(DB_TABLE_SELLED_LIST, m_dwCanTakeOffsetS);
}

void StatServer::generateItemCode(DWORD curSec)
{
  FUN_TRACE();
  if (m_dwItemCodeTime && curSec < m_dwItemCodeTime)
    return;

  QWORD qwCodeCnt = 0;
  /*xField *pField = xFieldsM::getMe().getField(REGION_DB, "item_code");
  if (!pField)
  {
    XERR << "[道具-礼包码] 找不到数据库表" << "item_code" << XEND;
    return;
  }
  
  char where[64];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "state=0");
  
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, where ,NULL);
  if ((QWORD)-1 == ret)
  {
    XERR << "[道具-礼包码] 查询数据库出错 ret:" << ret << XEND;
    return;
  }  
  qwCodeCnt = set.size();
  std::set<string> setTemp;

  for (QWORD i = 0; i < qwCodeCnt; i++)
  {
    setTemp.insert(set[i].getString("code"));
  }*/

  xField field(REGION_DB, "item_code");
  field.m_list["total_count"] = MYSQL_TYPE_NEWDECIMAL;

  char table[64];
  bzero(table, sizeof(table));
  snprintf(table, sizeof(table), "%s.%s", field.m_strDatabase.c_str(), field.m_strTable.c_str());

  char sql[256];
  bzero(sql, sizeof(sql));
  snprintf(sql, sizeof(sql), "select count(*) as total_count from %s where state=0 and `type`=4000", table);
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeRawSelect(&field, set, sql);
  if ((QWORD)-1 == ret)
  {
    XERR << "[道具-礼包码] 查询数据库出错 ret:" << ret << XEND;
    return;
  }
  qwCodeCnt = set[0].get<QWORD>("total_count");

  std::set<string> setTemp;
  m_dwItemCodeTime = curSec + 2*60;

  if (qwCodeCnt > 10000)
  {
    XLOG << "[道具-礼包码] 充足不需要生成" <<qwCodeCnt << XEND;
    return;
  }
  
  //生成礼包码
  static std::vector<char> sVecBaseCode = {'2','3','4','5','6','7','8','9',
    'a','b','c','d','e','f','g','h','i','j','k','m','n','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z'
  };
  
  auto getCode = [&]()->string
  {
    stringstream ss;
    for (int i = 0; i < 8; i++)
    {
      DWORD pos = randBetween(0, sizeof(sVecBaseCode) - 1);
      ss << sVecBaseCode[pos];
    }
    return ss.str();
  };

  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "item_code");
  if (!pField)
  {
    XERR << "[道具-礼包码] 找不到数据库表" << "item_code" << XEND;
    return;
  }
  DWORD cnt = 50000;  
  DWORD tryCnt = cnt * 2;
  xRecordSet recordSet;
  xRecord record(pField);
  while (cnt && tryCnt--)
  {
    string code = getCode();
    if (setTemp.find(code) != setTemp.end())
      continue;
    setTemp.insert(code);
    record.putString("code", code);
    record.put("type", 4000);
    recordSet.push(record);   
    cnt--;
    
    if (recordSet.size() >= 500)
    {
      QWORD ret = thisServer->getDBConnPool().exeInsertSet(recordSet);
      if (ret == QWORD_MAX)
      {
        XERR << "[道具-礼包码] 插入数据库出错 recordSet.size" << recordSet.size() << XEND;
        cnt += recordSet.size();
        recordSet.clear();
        continue;
      }
      recordSet.clear();
    }
  }  

  if (!recordSet.empty())
  {
    QWORD ret = thisServer->getDBConnPool().exeInsertSet(recordSet);
    if (ret == QWORD_MAX)
    {
      XERR << "[道具-礼包码] 插入数据库出错 recordSet.size"<< recordSet.size() << XEND;
    }
  }
}
