#include "GZoneManager.h"
#include "GZoneServer.h"
#include "xDBFields.h"
#include "CommonConfig.h"
#include "RedisManager.h"

void GZoneConfig::refreshRedis()
{
  std::string key = RedisManager::getMe().getKeyByParam(m_dwZoneID/10000, EREDISKEYTYPE_ZONE_INFO, m_dwZoneID);
  m_oData.setData("status", m_dwStatus);
  m_oData.setData("mergeid", m_dwMergeID);
  if (RedisManager::getMe().setHash(key, m_oData) == false)
  {
    XERR << "[区信息]" << m_dwZoneID << key << "保存失败" << XEND;
  }
}

void GRegionConfig::refreshRedis()
{
  std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_REGION_INFO, "data");
  m_oData.setData("regionname", m_strRegionName);
  m_oData.setData("opentime", m_strOpenTime);
  m_oData.setData("maintainstart", m_strMaintainStart);
  m_oData.setData("maintainend", m_strMaintainEnd);
  m_oData.setData("content", m_strContent);
  m_oData.setData("tip", m_strTip);
  m_oData.setData("picture", m_strPicture);
  m_oData.setData("version", m_strServerVersion);
  m_oData.setData("min_version", m_strMinServerVersion);
  if (m_mapLanguage2Notice.empty() == false)
  {
    xLuaData noticed;
    for (auto& v : m_mapLanguage2Notice)
    {
      stringstream ss;
      ss << v.first;
      xLuaData& d = noticed.getMutableData(ss.str().c_str());
      d.setData("tip", v.second.m_strTip);
      d.setData("content", v.second.m_strContent);
    }
    stringstream noticess;
    noticed.toJsonString(noticess);
    m_oData.setData("notice", noticess.str());
  }
  else
  {
    m_oData.setData("notice", "");
  }
  if (RedisManager::getMe().setHash(key, m_oData) == false)
  {
    XERR << "[大服信息]" << thisServer->getRegionID() << key << "保存失败" << XEND;
  }
}

GZoneManager::GZoneManager()
{
}

GZoneManager::~GZoneManager()
{
}

void GZoneManager::clear()
{
  for (DWORD i = 0; i < ZONE_STATUS_MAX; i++)
  {
    m_oGZoneVec[i].clear();
  }
  m_oCurCreateZoneID.clear();
}

void GZoneManager::final()
{
  save();
}

bool GZoneManager::load()
{
  loadGRegionConfig();

  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "zone");
  if (!field) return false;

  clear();

  char szWhere[128] = {0};
  snprintf(szWhere, 128, "regionid=%u", thisServer->getRegionID());
  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(field, set, szWhere);
  if (QWORD_MAX == retNum)
  {
    XERR << "[区信息-加载]" << "加载失败" << XEND;
  }

  for (DWORD i = 0; i < retNum; ++i)
  {
    DWORD dwStatus = set[i].get<DWORD>("status");
    if (dwStatus >= ZONE_STATUS_MAX)
    {
      GZoneConfig item;
      item.m_dwZoneID = set[i].get<DWORD>("zoneid");
      item.m_dwStatus = dwStatus;
      item.refreshRedis();
      continue;
    }

    GZoneConfig &item = m_oGZoneVec[dwStatus][set[i].get<DWORD>("zoneid")];

    item.m_dwZoneID = set[i].get<DWORD>("zoneid");
    item.m_dwStatus = dwStatus;
    item.m_dwOpenTime = set[i].get<DWORD>("opentime");
    item.m_dwMergeID = set[i].get<DWORD>("mergeid");
    item.m_dwOpenIndex = set[i].get<DWORD>("openindex");
    item.m_dwActive = set[i].get<DWORD>("active");
    item.m_dwOnline = set[i].get<DWORD>("online");
    item.m_eLanguage = static_cast<ELanguageType>(set[i].get<DWORD>("language"));

    item.refreshRedis();

    if (m_oCurCreateZoneID.find(item.m_eLanguage) == m_oCurCreateZoneID.end())
      m_oCurCreateZoneID[item.m_eLanguage] = 0;

    XLOG << "[区信息-加载]" << "zoneid:" << item.m_dwZoneID << "status:" << item.m_dwStatus << "mergeid:" << item.m_dwMergeID << item.m_dwActive << item.m_dwOnline << item.m_dwOpenIndex << item.m_dwOpenTime << "language:" << item.m_eLanguage << XEND;
  }

  for (DWORD i = 0; i < ZONE_STATUS_MAX; ++i)
  {
    XLOG << "[区信息-加载]" << "状态:" << i << "共加载:" << m_oGZoneVec[i].size() << XEND;
  }

  return true;
}

void GZoneManager::save()
{
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "zone");
  if (!field)
  {
    XERR << "[区信息-保存]" << "保存失败,找不到zone field" << XEND;
    return;
  }

  for (DWORD i=0; i < ZONE_STATUS_MAX; ++i)
  {
    for (auto it : m_oGZoneVec[i])
    {
      if (it.second.m_blUpdate == false) continue;

      char szWhere[64] = {0};
      snprintf(szWhere, 64, "zoneid=%u", it.first);

      GZoneConfig &item = it.second;
      xRecord record(field);
      record.put("zoneid", it.first);
      record.put("opentime", item.m_dwOpenTime);
      record.put("status", item.m_dwStatus);
      record.put("active", item.m_dwActive);
      record.put("online", item.m_dwOnline);
      record.put("mergeid", item.m_dwMergeID);

      QWORD retNum = thisServer->getDBConnPool().exeUpdate(record, szWhere);
      if (QWORD_MAX == retNum)
      {
        XERR << "[活跃在线]" << it.first << "活跃" << item.m_dwActive<< "在线" << item.m_dwOnline << "存入数据库失败" << XEND;
        continue;
      }
      item.refreshRedis();

      XLOG << "[活跃在线]" << it.first << "活跃" << item.m_dwActive<< "在线" << item.m_dwOnline << XEND;
    }
  }
}

void GZoneManager::update(DWORD zoneid, DWORD active, DWORD online)
{
  for (DWORD i=ZONE_STATUS_RUN; i<=ZONE_STATUS_CLOSE; ++i)
  {
    auto it = m_oGZoneVec[i].find(zoneid);
    if (it != m_oGZoneVec[i].end())
    {
      it->second.m_dwActive = active;
      it->second.m_dwOnline = online;
      it->second.m_blUpdate = true;
      XLOG << "[活跃在线-更新]" << "ZoneID:" << zoneid << "活跃:" << active << "在线:" << online << XEND;
      return;
    }
  }
}

DWORD GZoneManager::getCreateCharZoneID(ELanguageType language)
{
  DWORD cur = now();
  DWORD dwProtectiveTime = cur - CommonConfig::m_dwNewZoneProtectiveTime;
  DWORD active_limit = CommonConfig::m_dwActiveUserMax;
  DWORD online_limit = CommonConfig::m_dwOnlineUserMax;

  std::multimap<DWORD, DWORD> plist;  // 保护时间内的
  std::multimap<DWORD, DWORD> list;   // 其他的线

  DWORD dwMinOnlineZoneID = 0;
  DWORD dwMinOnlineNum = (DWORD)-1;
  for (auto it : m_oGZoneVec[ZONE_STATUS_RUN])
  {
    if (it.second.m_eLanguage != language)
    {
      continue;
    }

    if (it.second.m_dwOnline < dwMinOnlineNum)
    {
      dwMinOnlineNum = it.second.m_dwOnline;
      dwMinOnlineZoneID = it.first;
    }
    if (it.second.m_dwActive >= active_limit) continue;
    if (it.second.m_dwOnline >= online_limit) continue;

    DWORD last = 0;
    if (cur > it.second.m_dwOpenTime)
      last = cur - it.second.m_dwOpenTime;
    if (!checkCreate(last, it.second.m_dwActive, it.second.m_dwOnline))
    {
      continue;
    }

    if (it.second.m_dwOpenTime >= dwProtectiveTime)
    {
      plist.insert(std::make_pair(it.second.m_dwOnline, it.first));
    }
    else
    {
      list.insert(std::make_pair(it.second.m_dwOnline, it.first));
    }
  }
  if (!plist.empty())
  {
    return plist.begin()->second;
  }
  if (!list.empty())
  {
    return list.begin()->second;
  }

  // 找不到看是否有准备开的线
  DWORD dwZoneID = checkOpenZone(language);
  if (dwZoneID) return dwZoneID;
  // 如果都没找到 返回活跃最小的
  return dwMinOnlineZoneID;
}

bool GZoneManager::checkCreate(DWORD last, DWORD active, DWORD online)
{
  for (auto it : CommonConfig::m_oActiveOnlineList)
  {
    if (last < it.first)
    {
      if (active < it.second.dwActive && online < it.second.dwOnline)
      {
        return true;
      }
      return false;
    }
  }
  return true;
}

DWORD GZoneManager::checkOpenZone(ELanguageType language)
{
  XLOG << "[自动开服]" << "开始检测" << XEND;

  DWORD dwRetZoneID = 0;

  std::map<DWORD, std::set<DWORD>> list;
  for (auto it : m_oGZoneVec[ZONE_STATUS_READY])
  {
    if (it.second.m_eLanguage != language)
    {
      continue;
    }
    list[it.second.m_dwOpenIndex].insert(it.first);
  }
  DWORD count = 0;
  for (auto it : list)
  {
    for (auto iter : it.second)
    {
      DWORD dwRetZoneID = iter;
      m_oGZoneVec[ZONE_STATUS_READY][dwRetZoneID].m_dwStatus = ZONE_STATUS_RUN;
      m_oGZoneVec[ZONE_STATUS_READY][dwRetZoneID].m_dwOpenTime = now();
      m_oGZoneVec[ZONE_STATUS_READY][dwRetZoneID].m_blUpdate = true;
      m_oGZoneVec[ZONE_STATUS_RUN][dwRetZoneID] = m_oGZoneVec[ZONE_STATUS_READY][dwRetZoneID];
      m_oGZoneVec[ZONE_STATUS_READY].erase(dwRetZoneID);
      XLOG << "[自动开服]" << "ZoneID:" << dwRetZoneID << XEND;
      if (++count >= CommonConfig::m_dwOnceOpenZoneNum)
      {
        return dwRetZoneID;
      }
    }
  }

  return dwRetZoneID;
}

void GZoneManager::timer()
{
  for (auto& it : m_oCurCreateZoneID)
  {
    DWORD dwCreateZoneID = getCreateCharZoneID(it.first);
    if (dwCreateZoneID != it.second)
    {
      it.second = dwCreateZoneID;
      std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_CREATE_CHAR_ZONEID, it.first);
      RedisManager::getMe().setData(key, it.second);
    }
  }
  // 设置一个默认zoneid, 防止proxy取id时, 对应语言的id全都没有
  if (m_oCurCreateZoneID.empty() == false && m_oCurCreateZoneID.begin()->second != m_dwDefaultCreateZoneID)
  {
    m_dwDefaultCreateZoneID = m_oCurCreateZoneID.begin()->second;
    std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_CREATE_CHAR_ZONEID, "data");
    RedisManager::getMe().setData(key, m_dwDefaultCreateZoneID);
  }
}

void GZoneManager::loadGRegionConfig()
{
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "region");
  if (!field) return;

  field->setValid("platid,regionname,opentime,maintainstart,maintainend,content,tip,picture");
  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "regionid=%u", thisServer->getRegionID());

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
  if (QWORD_MAX!=ret && ret)
  {
    m_oGRegion.m_dwPlatID = set[0].get<DWORD>("platid");
    m_oGRegion.m_strOpenTime = set[0].getString("opentime");
    m_oGRegion.m_strContent = set[0].getString("content");
    m_oGRegion.m_strTip = set[0].getString("tip");
    m_oGRegion.m_strPicture = set[0].getString("picture");
    m_oGRegion.m_strMaintainStart = set[0].getString("maintainstart");
    m_oGRegion.m_strMaintainEnd = set[0].getString("maintainend");
    m_oGRegion.m_strRegionName = set[0].getString("regionname");
    getServerVersion(m_oGRegion.m_dwPlatID, m_oGRegion.m_strServerVersion, m_oGRegion.m_strMinServerVersion);

    // 多语言公告加载
    m_oGRegion.m_mapLanguage2Notice.clear();
    xField* noticefield = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "region_notice");
    if (noticefield)
    {
      noticefield->setValid("language,tip,content");
      char where1[128];
      bzero(where1, sizeof(where1));
      snprintf(where1, sizeof(where1), "regionid=%u", thisServer->getRegionID());

      xRecordSet set1;
      QWORD ret1 = thisServer->getDBConnPool().exeSelect(noticefield, set1, (const char *)where1, NULL);
      if (QWORD_MAX!=ret1 && ret1)
      {
        for (QWORD i = 0; i < ret1; ++i)
        {
          RegionNoticeInfo& ninfo = m_oGRegion.m_mapLanguage2Notice[static_cast<ELanguageType>(set1[i].get<DWORD>("language"))];
          ninfo.m_strTip = set1[i].getString("tip");
          ninfo.m_strContent = set1[i].getString("content");
        }
      }
    }

    m_oGRegion.refreshRedis();
  }
}

void GZoneManager::getServerVersion(DWORD platid, std::string& version, std::string& min)
{
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "platform");
  if (field)
  {
    field->setValid("version,min_version");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "platid=%u", platid);

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX!=ret && ret)
    {
      version = set[0].getString("version");
      min = set[0].getString("min_version");
    }
  }
}
