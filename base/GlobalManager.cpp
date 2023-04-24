#include "GlobalManager.h"
#include "xServer.h"
#include "xDBConnPool.h"

extern xServer* thisServer;

GlobalManager::GlobalManager()
{
}

GlobalManager::~GlobalManager()
{
}

bool GlobalManager::loadGlobalData()
{
  bool bResult = true;
  bResult &= loadGlobalBoss();
  return bResult;
}

bool GlobalManager::loadGlobalBoss()
{
  m_oGlobalBoss.Clear();
  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "global");
  if (nullptr == pField)
  {
    XERR << "[世界boss-加载] 获取数据库失败" << XEND;
    return false;
  }

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, "name = \'deadboss\'");
  if (ret == QWORD_MAX)
  {
    XERR << "[世界boss-加载] 查询世界 boss 失败, ret :" << ret << XEND;
    return false;
  }
  if (set.empty() == false)
  {
    const xRecord& record = set[0];
    string data;
    data.assign((const char*)record.getBin("data"), record.getBinSize("data"));
    parseGlobalBoss(data);
  }

  XLOG << "[世界boss-加载] 加载数据" << m_oGlobalBoss.ShortDebugString() << XEND;
  return true;
}

bool GlobalManager::updateGlobalBoss(const Cmd::DeadBossInfo& rInfo)
{
  if (m_oGlobalBoss.charid() != 0)
  {
    XDBG << "[世界boss-变更] 变更数据" << rInfo.ShortDebugString() << "失败,已包含数据" << m_oGlobalBoss.ShortDebugString() << XEND;
    return false;
  }

  m_oGlobalBoss.Clear();
  m_oGlobalBoss.CopyFrom(rInfo);

  if (m_oGlobalBoss.charid() == 0)
  {
    m_oGlobalBoss.Clear();
    XDBG << "[世界boss-变更] 变更数据" << rInfo.ShortDebugString() << "失败,更新后 charid 仍为0" << XEND;
    return false;
  }

  if (strncmp(thisServer->getServerName(), "GlobalServer", 11) != 0)
    return true;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "global");
  if (pField == nullptr)
  {
    XERR << "[世界boss-变更] 变更数据" << rInfo.ShortDebugString() << "失败,获取数据库global信息失败" << XEND;
    return false;
  }

  string data;
  serialGlobalBoss(data);

  xRecord record(pField);
  record.put("name", "deadboss");
  record.putBin("data", (unsigned char*)data.c_str(), data.size());

  QWORD ret = thisServer->getDBConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[世界boss-变更] 变更数据" << rInfo.ShortDebugString() << "失败,插入数据失败, ret :" << ret << XEND;
    return false;
  }

  return true;
}

void GlobalManager::parseGlobalBoss(const string& str)
{
  std::stringstream sstr(str);

  QWORD qwValue = 0;
  DWORD dwValue = 0;
  string strValue;

  m_oGlobalBoss.Clear();

  // charid
  sstr >> qwValue;
  m_oGlobalBoss.set_charid(qwValue);

  // zoneid
  sstr >> dwValue;
  m_oGlobalBoss.set_zoneid(dwValue);

  // time
  sstr >> dwValue;
  m_oGlobalBoss.set_time(dwValue);

  // name
  sstr >> strValue;
  m_oGlobalBoss.set_name(strValue);
}

void GlobalManager::serialGlobalBoss(string& str)
{
  str.clear();

  std::stringstream sstr;
  sstr << m_oGlobalBoss.charid() << " " << m_oGlobalBoss.zoneid() << " " << m_oGlobalBoss.time() << " " << m_oGlobalBoss.name() << " ";
  str = sstr.str();
}

