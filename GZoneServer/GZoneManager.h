#pragma once
#include "xSingleton.h"
#include "xNoncopyable.h"
#include "xLuaTable.h"
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
  ZONE_STATUS_MAX     = 4,
};

struct GZoneConfig
{
  DWORD m_dwZoneID = 0;
  DWORD m_dwStatus = 0;
  DWORD m_dwOpenTime = 0;
  DWORD m_dwMergeID = 0;
  DWORD m_dwOpenIndex = 0;
  DWORD m_dwActive = 0;
  DWORD m_dwOnline = 0;
  bool m_blUpdate = false;
  ELanguageType m_eLanguage;

  xLuaData m_oData;
  void refreshRedis();
};

struct RegionNoticeInfo
{
  std::string m_strTip;
  std::string m_strContent;
};

struct GRegionConfig
{
  DWORD m_dwPlatID = 0;
  std::string m_strRegionName;
  std::string m_strOpenTime;
  std::string m_strMaintainStart;
  std::string m_strMaintainEnd;
  std::string m_strContent;
  std::string m_strTip;
  std::string m_strPicture;
  std::string m_strServerVersion;
  std::string m_strMinServerVersion;

  std::map<ELanguageType, RegionNoticeInfo> m_mapLanguage2Notice;

  xLuaData m_oData;
  void refreshRedis();
};

class GZoneManager : public xSingleton<GZoneManager>
{
  friend class xSingleton<GZoneManager>;

  public:
    virtual ~GZoneManager();
  private:
    GZoneManager();

  public:
    void final();
    bool load();
    void save();
    void timer();

  public:
    void update(DWORD zoneid, DWORD active, DWORD online);

  private:
    void clear();
    DWORD getCreateCharZoneID(ELanguageType language);
    bool checkCreate(DWORD last, DWORD active, DWORD online);
    DWORD checkOpenZone(ELanguageType language);
    void getServerVersion(DWORD platid, std::string& version, std::string& min);

  private:
    std::map<DWORD, GZoneConfig> m_oGZoneVec[ZONE_STATUS_MAX];
    std::map<ELanguageType, DWORD> m_oCurCreateZoneID;
    DWORD m_dwDefaultCreateZoneID = 0;

  public:
    void loadGRegionConfig();
  private:
    GRegionConfig m_oGRegion;
};
