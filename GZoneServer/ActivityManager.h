#pragma once

#include "xSingleton.h"
#include "SessionSociality.pb.h"
#include "ActivityEvent.pb.h"

using namespace Cmd;
using std::map;
using std::vector;

struct SActivityInfo
{
  DWORD dwId = 0;
  string strName;
  string strIconUrl;
  DWORD dwBeginTime = 0;
  DWORD dwEndTime = 0;
  string strUrl;
  bool bCountdown = 0;
  string strMd5;
  OperActivityData oData;
  bool updated = true;

  void toOperActivity(OperActivity* pData);
  bool operator==(SActivityInfo& r) { return strMd5 == r.strMd5; }
};

struct SSubActivityInfo
{
  DWORD dwId = 0;
  DWORD dwGroupId = 0;
  string strName;
  DWORD dwBeginTime = 0;
  DWORD dwEndTime = 0;
  DWORD dwPath = 0;
  string strUrl;
  string strPicUrl;
  string strMd5;
  OperSubActivityData oData;
  bool updated = true;

  void toOperSubActivity(OperSubActivity* pData);
  bool operator==(SSubActivityInfo& r) { return strMd5 == r.strMd5; }
};

struct SActivityEventInfo
{
  DWORD dwID = 0;
  string strMd5;
  ActivityEventInfo oInfo;
  bool updated = true;

  void toActivityEventInfo(ActivityEventInfo* pData)
  {
    if (pData)
      pData->CopyFrom(oInfo);
  }
  bool operator==(SActivityEventInfo& r) { return strMd5 == r.strMd5; }
};

class ActivityManager : public xSingleton<ActivityManager>
{
  friend class xSingleton<ActivityManager>;
private:
  ActivityManager();
public:
  virtual ~ActivityManager();

  bool load();
  void updateRedis();
  void notifySession();

private:
  map<DWORD, SActivityInfo> m_mapAct;
  map<DWORD, SSubActivityInfo> m_mapSubAct;
  vector<DWORD> m_vecSubActId;
};

// 活动模板
class ActivityEventManager : public xSingleton<ActivityEventManager>
{
  friend class xSingleton<ActivityEventManager>;
private:
  ActivityEventManager() {}
public:
  virtual ~ActivityEventManager() {}

  bool load();
  void updateRedis();
  void notifySession();
private:
  map<DWORD, SActivityEventInfo> m_mapEvent;
};
