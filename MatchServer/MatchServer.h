#pragma once
#include <set>
#include "xDefine.h"
#include "RegionServer.h"
#include "xSQLThread.h"

class xNetProcessor;

struct PollyZoneInfo
{
  void setUserCount(DWORD count);

  DWORD dwZoneId = 0;
  DWORD dwUserCount = 0;
};
typedef std::map<DWORD/*zoneid*/, PollyZoneInfo> TMapZoneUserCntInfo;

struct CmpByValue1 {   //从小到大排序
  bool operator()(const PollyZoneInfo& lhs, const PollyZoneInfo& rhs) {
    return lhs.dwUserCount < rhs.dwUserCount;
  }
};

class MatchServer : public RegionServer
{
  public:
    MatchServer(OptArgs &args);
    virtual ~MatchServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_MATCH; }

public:
  bool sendCmdToClient(QWORD charId, DWORD zoneId, void* buf, WORD len);
  //通过session发送协议到teamserver
  bool forwardCmdToTeamServer(DWORD zoneId, void* buf, WORD len);
  //通过session发送协议到sceneserver
  bool forwardCmdToSceneServer(QWORD charId, DWORD zoneId, void* buf, WORD len);
  protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_zoneDel(DWORD zoneId);
    virtual void v_timetick();

  protected:
    bool loadConfig();

  protected:
    virtual bool doMatchCmd(BYTE* buf, WORD len);
    virtual bool doSocialCmd(BYTE* buf, WORD len);
    virtual bool doGZoneCmd(BYTE* buf, WORD len);
  public:
    bool isInPvpZoneId(DWORD zoneId);
    void addZoneInfo(DWORD zoneId);
    void delZoneInfo(DWORD zoneId);
    void sortZone();
    DWORD getAZoneId();
    PollyZoneInfo* getZoneInfo(DWORD dwZoneId);
    bool getMinZones(DWORD num, std::list<DWORD>& minZones);
  public:
    DWORD m_dwLLHZoneid = 0;
    DWORD m_dwSMZLZoneid = 0;
    DWORD m_dwHLJSZoneid = 0;

    xSQLThread m_oSQLThread;
  private:
    TMapZoneUserCntInfo m_mapZoneInfo;
    DWORD m_dwPollyZoneid = 0;
    bool n_bNeedSort = false;
    xTimer m_oOneSecTimer;
};

extern MatchServer *thisServer;
