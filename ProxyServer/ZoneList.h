#pragma once
#include "xSingleton.h"
#include "xNoncopyable.h"
#include "CommonConfig.h"

/*
 * status
 * 0 初始状态
 * 1 设置为可以使用，其他线满后status设为2
 * 2 可以创建角色
 * 3 服务器关闭,不再进入角色 当前线的玩家分配到其他线
 */

enum
{
  ZONE_STATUS_CREATE  = 0,
  ZONE_STATUS_READY   = 1,
  ZONE_STATUS_RUN     = 2,
  ZONE_STATUS_CLOSE   = 3,
};

struct ZoneDBInfo
{
  DWORD m_dwZoneID = 0;
  DWORD m_dwStatus = 0;
  DWORD m_dwMergeID = 0;

  DWORD m_dwValidTime = 0;
};

struct RegionNoticeInfo
{
  std::string m_strTip;
  std::string m_strContent;
};

struct RegionStatusInfo
{
  DWORD m_dwPlatID = 0;
  bool m_blOpen = false;
  bool m_blMaintain = false;
  std::string m_strContent;
  std::string m_strTip;
  std::string m_strPicture;
  std::string m_strServerVersion;
  std::string m_strMinServerVersion;

  DWORD m_dwValidTime = 0;

  std::map<DWORD, RegionNoticeInfo> m_mapLanguage2Notice;
  const string& getTip(DWORD language) const {
    if (m_strTip.empty() == false)
      return m_strTip;
    if (m_mapLanguage2Notice.empty())
      return STRING_EMPTY;
    auto it = m_mapLanguage2Notice.find(language);
    if (it == m_mapLanguage2Notice.end())
    {
      auto m = m_mapLanguage2Notice.find(CommonConfig::m_eDefaultLanguage);
      if (m == m_mapLanguage2Notice.end())
        return m_mapLanguage2Notice.begin()->second.m_strTip;
      return m->second.m_strTip;
    }
    return it->second.m_strTip;
  }
  const string& getContent(DWORD language) const {
    if (m_strContent.empty() == false)
      return m_strContent;
    if (m_mapLanguage2Notice.empty())
      return STRING_EMPTY;
    auto it = m_mapLanguage2Notice.find(language);
    if (it == m_mapLanguage2Notice.end())
    {
      auto m = m_mapLanguage2Notice.find(CommonConfig::m_eDefaultLanguage);
      if (m == m_mapLanguage2Notice.end())
        return m_mapLanguage2Notice.begin()->second.m_strContent;
      return m->second.m_strContent;
    }
    return it->second.m_strContent;
  }
};

class ZoneList : public xSingleton<ZoneList>
{
  friend class xSingleton<ZoneList>;

  public:
    virtual ~ZoneList();
  private:
    ZoneList();

  public:
    DWORD getANewZoneID(DWORD regionid, DWORD language);

    // 大服数据
  public:
    bool getRegionStatus(DWORD regionid, RegionStatusInfo &info);
    bool getMinServerVersion(RegionStatusInfo &info);
  private:
    std::map<DWORD, RegionStatusInfo> m_oRegionInfoList;

  public:
    const char* getRegionDBName(DWORD regionid);
  private:
    // region dbname
    std::map<DWORD, std::string> m_mapRegionDB;

    // 平台数据
  private:
    std::string getPlatName(DWORD platid);
  private:
    std::map<DWORD, std::string> m_mapPlatIDName;

    // 线信息
  public:
    bool getZoneDBInfo(DWORD zoneid, ZoneDBInfo &info);
  private:
    std::map<DWORD, ZoneDBInfo> m_oZoneDBInfoList;
};
