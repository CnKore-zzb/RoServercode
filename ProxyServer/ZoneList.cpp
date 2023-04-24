#include "ZoneList.h"
#include "xDBConnPool.h"
#include "ProxyServer.h"
#include "xDBConnPool.h"
#include "RedisManager.h"
#include "CommonConfig.h"

ZoneList::ZoneList()
{
}

ZoneList::~ZoneList()
{
}

bool ZoneList::getZoneDBInfo(DWORD zoneid, ZoneDBInfo &info)
{
  DWORD cur = now();
  auto it = m_oZoneDBInfoList.find(zoneid);
  if (it != m_oZoneDBInfoList.end())
  {
    if (it->second.m_dwValidTime > cur)
    {
      info = it->second;
      return true;
    }
  }
  xLuaData oData;
  std::string key = RedisManager::getMe().getKeyByParam(zoneid/10000, EREDISKEYTYPE_ZONE_INFO, zoneid);
  if (RedisManager::getMe().getHashAll(key, oData))
  {
    info.m_dwZoneID = zoneid;
    info.m_dwStatus = oData.getTableInt("status");
    info.m_dwMergeID = oData.getTableInt("mergeid");
    info.m_dwValidTime = cur + MIN_T;
    m_oZoneDBInfoList[zoneid] = info;
    return true;
  }
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "zone");  // 先查redis
  if (field)
  {
    char szWhere[128] = {0};
    snprintf(szWhere, 128, "zoneid=%u", zoneid);
    xRecordSet set;
    QWORD retNum = thisServer->getDBConnPool().exeSelect(field, set, szWhere);
    if (QWORD_MAX == retNum || 0 == retNum)
    {
      return false;
    }

    info.m_dwZoneID = set[0].get<DWORD>("zoneid");
    info.m_dwStatus = set[0].get<DWORD>("status");
    info.m_dwMergeID = set[0].get<DWORD>("mergeid");
    info.m_dwValidTime = cur + MIN_T;
    m_oZoneDBInfoList[zoneid] = info;
    return true;
  }
  return false;
}

DWORD ZoneList::getANewZoneID(DWORD regionid, DWORD language)
{
  std::string key = RedisManager::getMe().getKeyByParam(regionid, EREDISKEYTYPE_CREATE_CHAR_ZONEID, language);
  DWORD dwZoneID = 0;
  RedisManager::getMe().getData(key, dwZoneID);
  if (dwZoneID == 0)
    RedisManager::getMe().getData(RedisManager::getMe().getKeyByParam(regionid, EREDISKEYTYPE_CREATE_CHAR_ZONEID, CommonConfig::m_eDefaultLanguage), dwZoneID);
  if (dwZoneID == 0)
    RedisManager::getMe().getData(RedisManager::getMe().getKeyByParam(regionid, EREDISKEYTYPE_CREATE_CHAR_ZONEID, "data"), dwZoneID);
  return dwZoneID;
}

const char* ZoneList::getRegionDBName(DWORD regionid)
{
  auto it = m_mapRegionDB.find(regionid);
  if (it != m_mapRegionDB.end()) return it->second.c_str();

  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "region"); // 加载一次
  if (field)
  {
    field->setValid("regionname,platid");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "regionid = %u", regionid);

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where);
    if (QWORD_MAX!=ret && ret)
    {
      std::stringstream stream;
      stream.str("");
      std::string platname = getPlatName(set[0].get<DWORD>("platid"));
      stream << "ro_" << platname << "_" << set[0].getString("regionname");
      m_mapRegionDB[regionid] = stream.str();
      thisServer->addDataBase(stream.str().c_str(), false);
      XLOG << "[大服数据库],添加" << regionid << stream.str() << XEND;
      return m_mapRegionDB[regionid].c_str();
    }
  }

  return "";
}

std::string ZoneList::getPlatName(DWORD platid)
{
  auto it = m_mapPlatIDName.find(platid);
  if (it != m_mapPlatIDName.end())
  {
    return it->second;
  }
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "platform"); // 加载一次
  if (field)
  {
    field->setValid("platname");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "platid = %u", platid);

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where);
    if (QWORD_MAX!=ret && ret)
    {
      XDBG << "[大服数据库],平台" << platid << set[0].getString("platname") << XEND;
      m_mapPlatIDName[platid] = set[0].getString("platname");
      return set[0].getString("platname");
    }
  }

  return "";
}

bool ZoneList::getRegionStatus(DWORD regionid, RegionStatusInfo &info)
{
  DWORD cur = now();
  auto it = m_oRegionInfoList.find(regionid);
  if (it != m_oRegionInfoList.end())
  {
    if (it->second.m_dwValidTime > cur)
    {
      info = it->second;
      return true;
    }
  }

  xLuaData oData;
  std::string key = RedisManager::getMe().getKeyByParam(regionid, EREDISKEYTYPE_REGION_INFO, "data");
  if (RedisManager::getMe().getHashAll(key, oData))
  {
    DWORD cur = now();
    DWORD openTime = 0;
    std::string openStr = oData.getTableString("opentime");
    info.m_dwPlatID = oData.getTableInt("platid");
    info.m_strContent = oData.getTableString("content");
    info.m_strTip = oData.getTableString("tip");
    info.m_strPicture = oData.getTableString("picture");
    info.m_strServerVersion = oData.getTableString("version");
    info.m_strMinServerVersion = oData.getTableString("min_version");
    if (openStr != "")
    {
      parseTime(oData.getTableString("opentime"), openTime);
    }
    if (cur >= openTime)
    {
      info.m_blOpen = true;
    }
    else
    {
      info.m_blOpen = false;
    }

    DWORD startTime = 0;
    DWORD endTime = 0;
    std::string msStr = oData.getTableString("maintainstart");
    if (msStr != "")
    {
      parseTime(msStr.c_str(), startTime);
    }
    std::string meStr = oData.getTableString("maintainend");
    if (meStr != "")
    {
      parseTime(meStr.c_str(), endTime);
    }

    if ((startTime && startTime < cur) && (endTime && endTime > cur))
    {
      info.m_blMaintain = true;
    }
    else
    {
      info.m_blMaintain = false;
    }
    info.m_dwValidTime = cur + MIN_T;

    info.m_mapLanguage2Notice.clear();
    const std::string& noticestr = oData.getTableString("notice");
    if (noticestr.empty() == false)
    {
      xLuaData noticed;
      if (noticed.fromJsonString(noticestr))
      {
        auto noticef = [&info](const string& k, xLuaData& d)
          {
            RegionNoticeInfo& ninfo = info.m_mapLanguage2Notice[atoi(k.c_str())];
            ninfo.m_strTip = d.getTableString("tip");
            ninfo.m_strContent = d.getTableString("content");
          };
        noticed.foreach(noticef);
      }
    }

    m_oRegionInfoList[regionid] = info;
    return true;
  }

  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "region");  // 先查redis
  if (field)
  {
    field->setValid("platid,opentime,maintainstart,maintainend,content,tip,picture");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "regionid=%u", regionid);

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX!=ret && ret)
    {
      DWORD cur = now();
      DWORD openTime = 0;
      std::string openStr = set[0].getString("opentime");
      info.m_dwPlatID = set[0].get<DWORD>("platid");
      info.m_strContent = set[0].getString("content");
      info.m_strTip = set[0].getString("tip");
      info.m_strPicture = set[0].getString("picture");
      if (openStr != "")
      {
        parseTime(set[0].getString("opentime"), openTime);
      }
      if (cur >= openTime)
      {
        info.m_blOpen = true;
      }
      else
      {
        info.m_blOpen = false;
      }

      DWORD startTime = 0;
      DWORD endTime = 0;
      std::string msStr = set[0].getString("maintainstart");
      if (msStr != "")
      {
        parseTime(msStr.c_str(), startTime);
      }
      std::string meStr = set[0].getString("maintainend");
      if (meStr != "")
      {
        parseTime(meStr.c_str(), endTime);
      }

      if ((startTime && startTime < cur) && (endTime && endTime > cur))
      {
        info.m_blMaintain = true;
      }
      else
      {
        info.m_blMaintain = false;
      }
      info.m_dwValidTime = cur + MIN_T;

      // 多语言公告加载
      info.m_mapLanguage2Notice.clear();
      xField* noticefield = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "region_notice");
      if (noticefield)
      {
        noticefield->setValid("language,tip,content");
        char where1[128];
        bzero(where1, sizeof(where1));
        snprintf(where1, sizeof(where1), "regionid=%u", regionid);

        xRecordSet set1;
        QWORD ret1 = thisServer->getDBConnPool().exeSelect(noticefield, set1, (const char *)where1, NULL);
        if (QWORD_MAX!=ret1 && ret1)
        {
          for (QWORD i = 0; i < ret1; ++i)
          {
            RegionNoticeInfo& ninfo = info.m_mapLanguage2Notice[set1[i].get<DWORD>("language")];
            ninfo.m_strTip = set1[i].getString("tip");
            ninfo.m_strContent = set1[i].getString("content");
          }
        }
      }

      m_oRegionInfoList[regionid] = info;
      return getMinServerVersion(info);
    }
  }
  return false;
}

bool ZoneList::getMinServerVersion(RegionStatusInfo &info)
{
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "platform"); // 暂时没用
  if (field)
  {
    field->setValid("version,min_version");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "platid=%u", info.m_dwPlatID);

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX!=ret && ret)
    {
      info.m_strMinServerVersion = set[0].getString("min_version");
      info.m_strServerVersion = set[0].getString("version");
      return true;
    }
  }
  return false;
}
