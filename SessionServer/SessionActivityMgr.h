/**
  @breaf: Session 活动管理
  */

#pragma once

#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include <list>
#include <map>
#include <string>
#include <xTime.h>
#include "ActivityConfig.h"
#include "ClientManager.h"
#include "DepositConfig.h"
#include "ActivityCmd.pb.h"

#define DB_GLOBALACT_RECORD "globalact_record"

using namespace Cmd;

class SessionUser;

struct ActivityItem
{
  DWORD m_id = 0;
  EActivityType m_type = EACTIVITYTYPE_MAX;
  QWORD m_uid = 0;
  DWORD m_mapId = 0;
  DWORD m_startTime = 0;
  DWORD m_endTime = 0;
  bool m_bStop = false;
  DWORD m_dwPath = 0; //寻路
  TVecDWORD m_vecUnshowMap;
  
  void packProto(Cmd::StartActCmd& cmd);

};

struct GlobalActivityItem
{
  DWORD m_dwId = 0;
  GlobalActivityType m_eType = GACTIVITY_MIN;
  DWORD m_dwTimerId = 0;
  DWORD getDepositId(); //获取影响到的充值id
};

class SessionActivityMgr : public xSingleton<SessionActivityMgr>
{
  friend class xSingleton<SessionActivityMgr>;
  private:
    SessionActivityMgr();
  public:
    virtual ~SessionActivityMgr();

    void init();
    void reload();
    void timer(DWORD curSec);
    void testAndSet(ActivityTestAndSetSessionCmd rev);    
    void stop(const ActivityStopSessionCmd& rev);
    bool sendActStatusUser(SessionUser* pUser, ActivityItem& actItem);
    void onUserOnline(SessionUser* pUser);
    bool addGlobalActivity(DWORD id, DWORD timerId);
    bool delGlobalActivity(DWORD id);
    bool isOpen(DWORD id);
    void notifyGlobalAct(GlobalActivityItem* pGlobalAct, SessionUser* pUser, bool startcmd);
    void registRegion(ClientType type);
    GlobalActivityItem* getGlobalActivity(DWORD id);
    void getActivityEffect(const SGlobalActCFG* pCFG, DWORD&dwRate);
    bool getUserActivityCnt(const SGlobalActCFG* pCfg, QWORD qwAccid, QWORD qwCharId, DWORD dwStartTime, DWORD&count);
    bool getUserActivityCntFromDb(const SGlobalActCFG* pCfg, QWORD qwAccid, QWORD qwCharId, DWORD dwStartTime, DWORD&count);
    bool getUserAllActivityCnt(QWORD qwAccid, QWORD qwCharId, std::map<DWORD, DWORD>&mapCnt);
    bool addUserActivityCnt(DWORD activityId, QWORD qwAccid, QWORD qwCharId);
    bool checkUserGlobalActCnt(GlobalActivityType actType, const SDeposit* pCFG, QWORD qwAccid, QWORD qwCharid, const SGlobalActCFG** pGlobalActCfg);

  private:
    DWORD calcActivityCount(EActivityType type, DWORD mapId);
    void checkStartActivity(DWORD curSec);
    void checkStopActivity(DWORD curSec);
    ActivityItem* startActivity(DWORD id, DWORD mapId, bool bNotify, QWORD charId = 0);
    QWORD generateIndex();
  private:
    bool needWorldNtf(EActivityType type)
    {
      /*if (type == EACTIVITYTYPE_BCAT || type == EACTIVITYTYPE_CRAZYGHOST || type == EACTIVITYTYPE_XMAS || type == EACTIVITYTYPE_TEST
        || type == EACTIVITYTYPE_SPRINTFESTIVAL || type == EACTIVITYTYPE_READBAG) 
        return true;
      else return false; */
      return true;
    }
    bool checkCanStart(DWORD id, QWORD charId, DWORD mapId);
    bool checkMaintainTime(DWORD id);
  private:
    bool m_bInit = false;
    QWORD m_index = 0;
    std::map<DWORD/*map id*/, std::list<ActivityItem>> m_mapActvity;
    std::map<DWORD/*activity data id*/, xTimer2> m_mapTimers;
    std::list<std::pair<DWORD/*id*/, DWORD/*mapid*/>> m_listRetry;
    //std::set<DWORD> m_setGlobalActivity;
    std::map<DWORD, GlobalActivityItem> m_mapGlobalActivity;
    std::map<DWORD/*deposit id*/, TSetDWORD> m_mapCharge2GlobalAct;
};

