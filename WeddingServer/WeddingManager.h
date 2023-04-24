
#pragma once

#include "xSingleton.h"
#include <map>
#include <unordered_map>
#include <queue>
#include <vector>
#include <algorithm>
#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "TableManager.h"
#include "MsgManager.h"
#include "MessageStat.h"
#include "xTime.h"
#include "SocialCmd.pb.h"
#include "WeddingCCmd.pb.h"
#include "WeddingSCmd.pb.h"

#define DB_TABLE_WEDDING    "wedding"

class Wedding;
struct WeddingUser;
struct ReserveMgr;

struct DayReserve
{
  DayReserve(ReserveMgr* pMgr);

  void init();
  bool addWedding(Wedding* pWedding);
  void delWedding(Wedding* pWedding);
  EDateStatus getStatus();
  void sendWeddingOneDayList(QWORD qwCharId, DWORD dwZoneId, Cmd::ReqWeddingOneDayListCCmd& cmd);
  bool checkCanReserve(DWORD dwConfigId);
  bool isLock(DWORD dwConfigId);
  bool lock(DWORD dwConfigId);
  bool unlock(DWORD dwConfigId);
  void refreshStatus();
  WeddingOneDayInfo* getOneDayInfo(DWORD dwConfigId);

  DWORD m_dwDate = 0;
  std::map<DWORD/*configid*/, WeddingOneDayInfo> m_mapOneDayInfo;
  DWORD m_dwReservedCnt = 0;
  EDateStatus m_eStatus = EDateStatus_None;
  std::map<DWORD/*configid*/, DWORD/*unlock time*/> m_mapLock;
  ReserveMgr* m_pReserveMgr = nullptr;
};

struct ReserveMgr
{
  void refresh();
  void setStatusRefresh() { m_bStatusRefresh = true; }
  void checkStatusRefresh();
  bool addWedding(Wedding* pWedding);
  void delWedding(Wedding* pWedding);
  bool checkTicket(DWORD date, bool bUseTicket);
  DayReserve* getDayReserve(DWORD date);  
  void sendWeddingDateList(QWORD qwCharid, DWORD dwZoneid, bool bTicket);
  
  DWORD m_dwZoneId = 0;  
  DWORD m_dwStartDate = 0;
  DWORD m_dwEndDate = 0;
  DWORD m_dwNoTicketEndDate = 0;
  std::map<DWORD/*date*/, DayReserve> m_mapDayReserve;
  ReqWeddingDateListCCmd m_oDateListCmdCache;
  ReqWeddingDateListCCmd m_oDateListTicketCmdCache;
  bool m_bStatusRefresh = true;
};

typedef std::map<QWORD/*id*/, Wedding*> TMapId2Wedding;
typedef std::map<QWORD/*date*/, set<Wedding*>> TMapDate2Wedding;
typedef std::map<QWORD/*charid*/, QWORD/*id*/> TMapCharId2Wedding;
typedef std::map<DWORD/*zoneid*/, ReserveMgr> TMapZoneid2ReserveMgr;

class WeddingManager:public xSingleton<WeddingManager>
{
friend class xSingleton<WeddingManager>;
friend class MessageStatHelper;
private:
  WeddingManager();
  ~WeddingManager();
public:
  void init();
  void final();
  bool loadDb();
  void timeTick(DWORD curSec);
  void checkWedingTimeUp(DWORD curSec);
  bool doUserCmd(QWORD charId, const std::string& name, DWORD zoneId, const BYTE* buf, WORD len);
  bool doServerCmd(QWORD charId, const std::string& name, DWORD zoneId, const BYTE* buf, WORD len);

  //预定婚礼场景判定返回了
  bool reserveWedding(WeddingUser*pUser, Cmd::ReserveWeddingDateCCmd& rev);
  //预定场景返回
  bool reserveWeddingSceneRes(Cmd::ReserveWeddingResultSCmd& rev);
  //放弃预定
  bool giveUpReserve(WeddingUser*pUser, Cmd::GiveUpReserveCCmd& rev);
  //结婚
  bool marry(Cmd::MarrySCmd& rev, DWORD dwZoneId);
  //离婚
  bool divorce(QWORD charId, Cmd::ReqDivorceCCmd& rev);

  void onAddWedding(Wedding* pWedding);
  void onDelWedding(Wedding* pWedding);
  Wedding* getWeddingByCharId(QWORD qwCharId);
  Wedding* getWeddingById(QWORD qwId);
  ReserveMgr* getMutableReserveMgr(DWORD dwZoneId);
  void onDelChar(QWORD charId);
  void onSessionRestart(DWORD dwZoneId);

  DWORD getDateWeddingCount(DWORD dwDate) const;
private:
  bool addDateWedding(Wedding* pWedding);
  bool removeDateWedding(Wedding* pWedding);
private:
  void updateRecord(bool force);

  xTimer m_oneSecTimer;
  xTimer m_fiveSecTimer;
  xTimer m_OneMinTimer;
  xTimer m_ThirtyMinTimer;
  xTimer m_oneHourTimer;
  xTimer m_FiveMinTimer;
  
  TMapId2Wedding m_mapAllWedding;
  TMapDate2Wedding m_mapDateWedding;
  TMapCharId2Wedding m_mapChar2Wedding;
  TMapId2Wedding m_mapPreWedding;           //待开始的婚礼列表 
  std::set<QWORD> m_mapCurWedding;           //正在开始的婚礼列表 
  TMapZoneid2ReserveMgr m_mapReserveMgr;    //预定列表管理
  
  MessageStat m_oMessageStat;
};

class MessageStatHelper
{
public:
  MessageStatHelper(DWORD cmd, DWORD param);
  ~MessageStatHelper();
};

class MsgGuard : public xSingleton<MsgGuard>
{
public:
  bool lock(DWORD type, QWORD charId);

  bool unlock(DWORD type, QWORD charId);
private:
  std::unordered_map<DWORD/*key*/, std::unordered_map<QWORD/*charId*/, QWORD/*time*/>> m_mapGuard;
};
