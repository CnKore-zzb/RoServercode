#pragma once
#include "xSingleton.h"
#include <list>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "WeddingManager.h"
#include "WeddingSCmd.pb.h"
#include "WeddingManual.h"
#include <bitset>

enum ESyncState
{
  ESyncState_None = 0,
  ESyncState_Notice =1,       
  ESyncState_StartWedding = 2,     //婚礼进行时
  ESyncState_StopWedding = 3,     //婚礼进行时
};

enum EWeddingData
{
  EWEDDINGDATA_MIN = 0,
  EWEDDINGDATA_MANUAL = 1,
  EWEDDINGDATA_MAX = 2,
};

#ifdef _DEBUG
const DWORD WEDDING_RECORD_TICK_TIME = 10;
#else
const DWORD WEDDING_RECORD_TICK_TIME = 300;
#endif

class Wedding
{
public:
  Wedding();
  Wedding(QWORD qwId, DWORD dwZoneId, DWORD dwData, DWORD dwConfigId, QWORD qwCharId1, QWORD qwCharId2);
  ~Wedding();

  void fromRecord(const xRecord& record);
  void toRecord(xRecord& record, bool bId =true);
  void multableWeddingInfo(WeddingInfo* pInfo);
  void mutableBirefWeddingInfo(BirefWeddingInfo* pInfo);

  QWORD getId() { return m_qwId; }
  EWeddingStatus getStatus() { return m_eStatus; }
  ESyncState getWeddingState() const { return m_eSyncState; }
  QWORD getCharId1() { return m_qwCharId1; }
  QWORD getCharId2() { return m_qwCharId2; }
  DWORD getConfigId() { return m_dwConfigId; }
  DWORD getZoneId() { return m_dwZoneId; }
  DWORD getDate() { return m_dwDate; }
  QWORD getOtherCharId(QWORD qwCharId) { return m_qwCharId1 == qwCharId ? m_qwCharId2 : m_qwCharId1;}
  const string& getName(QWORD qwCharId);
  WeddingManual& getManual() { return m_oManual; }
  bool isMine(QWORD charId) { return m_qwCharId1 == charId ? true : (m_qwCharId2 == charId); }
  DWORD getStartTime() { return m_dwStartTime; }
  DWORD getEndTime() { return m_dwEndTime; }

  void timeTick(DWORD curSec);
  bool checkCanStart(DWORD curSec);
  bool checkCanStop(DWORD curSec);
  bool checkTimeOut(DWORD curSec);
  void processStartCurWedding();
  void sendStartWeding();
  void processStopCurWedding();
  bool processTimeOut(bool bSys);

  void updateRecord();
  void setUpdate(EWeddingData type) { m_bitsetRecord.set(type); }

  //处理订婚
  bool processReserve(DWORD dwZoneId, DWORD dwData, DWORD dwConfigId, QWORD qwCharId1, QWORD qwCharId2, bool bTicket);
  //处理去掉订婚
  bool processGiveupReserve(WeddingUser*pUser);
  //处理结婚
  bool processMarry(const Cmd::MarrySCmd& cmd);
  //处理离婚成功
  bool processDivorce(QWORD qwCharId, Cmd::EGiveUpType type);
  //可否放弃订婚
  bool canGiveupReserve(QWORD qwCharId);
  //可否结婚
  bool canMarry(QWORD charId1, QWORD charId2, DWORD zoneId);
  //可否单方面离婚
  bool canSingleDevore();
  void onUserOnline(WeddingUser* pUser);
  void onUserOffline(QWORD qwCharid);
  void setOfflineTime(QWORD qwCharId);
  DWORD getOfflineTime(QWORD qwCharId);
  bool isWeddingTime(DWORD curSec = 0);
  bool rename(QWORD charid, const string& name);

private:
  void fillCharData();
  void sendWeddingMsg2Scene(QWORD qwCharId, Cmd::WeddingEventMsgCCmd& msg);
  
private:
  QWORD m_qwId = 0;
  DWORD m_dwZoneId = 0;
  DWORD m_dwDate = 0;
  DWORD m_dwStartTime = 0;
  DWORD m_dwEndTime = 0;
  DWORD m_dwConfigId = 0;
  QWORD m_qwCharId1 = 0;
  QWORD m_qwCharId2 = 0;
  EWeddingStatus m_eStatus = EWeddingStatus_None;
  ESyncState m_eSyncState = ESyncState_None;
  WeddingManual m_oManual;
  CharData m_oChar1;
  CharData m_oChar2;

  DWORD m_dwMsgTime = 0;
  bool m_bCanSingleDivorce = false;
  DWORD m_dwOfflineTime1 = 0;
  DWORD m_dwOfflineTime2 = 0;

  std::bitset<EWEDDINGDATA_MAX> m_bitsetRecord;
};
