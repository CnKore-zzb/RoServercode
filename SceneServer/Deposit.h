#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "TableStruct.h"
#include "UserEvent.pb.h"
#include "RecordCmd.pb.h"
#include "ProtoCommon.pb.h"
#include "DepositConfig.h"
#include <list>
using namespace std;

class SceneUser;

typedef std::list<Cmd::QuotaDetail> TListQuotaDetail;
typedef std::list<Cmd::QuotaLog> TListQuotaLog;

class Deposit
{
public:
  Deposit(SceneUser* pUser);
  ~Deposit();

  bool load(const BlobDeposit& data);
  bool save(BlobDeposit *data);

  //
  bool isExpire(const Cmd::DepositTypeData& rData);
  DWORD getExpireTime(EDepositCardType eType) const;

  //10分钟 tick
  void timeTick(DWORD curSec);
  //void checkExpire(DWORD curSec);
  bool useCard(DWORD itemId);
  void sendDataToClient();

  //
  bool hasMonthCard();
  //exp
  DWORD getExp(EFuncType funcType, DWORD in);
  QWORD getGuildHonor(QWORD in);
  // normal drop
  float getNormalDrop(float in);
  //addcittime
  DWORD getAddcitTime(DWORD in);
  // pet work
  DWORD getPetWorkSpaceExtra(DWORD in);

  DWORD getMonthCardNum() const { return static_cast<DWORD>(m_mapType.size()); }

  void refreshQuota();
  bool addQuota(QWORD num, Cmd::EQuotaType type,DWORD mailId = 0, DWORD expireDay = 0);
  bool subQuota(QWORD num, Cmd::EQuotaType type);
  bool lockQuota(QWORD num, Cmd::EQuotaType type);
  QWORD unlockQuota(QWORD num, Cmd::EQuotaType type, bool sub = false);
  bool resell(QWORD num_unlock, QWORD num_lock);

  void clearQuota();
  bool checkQuota(QWORD money);
  QWORD updateQuota();
  QWORD getQuota();
  QWORD getQuotaLock();
  void redisPatch();
  bool everHasQuota() { return m_bEverHasQuota; }

  DWORD getPackageSlot(EPackType eType);
  
  void reqQuotaLog(Cmd::ReqQuotaLogCmd& rev);
  void reqQuotaDetail(Cmd::ReqQuotaDetailCmd& rev);

  void onLine();

private:
  void checkState(Cmd::DepositTypeData& rData);
  //月卡有效
  void onMonthCardValid(Cmd::DepositTypeData& rData);
  //月卡失效
  void onMonthCardInvalid(Cmd::DepositTypeData& rData);
  bool isFirstTimeUse(DWORD itemId);
  void addQuotaLog(Cmd::QuotaLog &log);
  void addQuotaDetail(Cmd::EQuotaType type, Cmd::QuotaDetail &detail);  
  bool isQuotaExpire(DWORD t, DWORD curSec) { return t <= curSec; }
  void convertQuotaShow();

private:
  SceneUser* m_pUser = nullptr;
  std::map<EDepositCardType, Cmd::DepositTypeData> m_mapType;
  std::vector<Cmd::DepositCardData> m_vecCard;
  std::list<Cmd::ChargeData> m_chargeDatas;   //废弃
  QWORD m_qwQuota = 0;
  QWORD m_qwQuotaLock = 0;
  xDayTimer m_dayTimer;
  std::set<DWORD> m_setUsedCard;
  bool m_bEverHasQuota = false;
  bool m_bEverGetItem = false;

  TListQuotaDetail m_listQuotaDetail;
  TListQuotaDetail m_listQuotaDetailShow;   //展示用的
  TListQuotaLog m_listQuotaLog;
};
