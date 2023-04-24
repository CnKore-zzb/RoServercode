#pragma once

#include "xSingleton.h"
#include <map>
#include "MatchSCmd.pb.h"
#include "SessionShop.pb.h"

#define DB_TABLE_GLOBAL_SHOP    "global_shop"

struct SoldCnt
{
  DWORD dwSoldCnt = 0;
  DWORD dwLockCnt = 0;
  
  DWORD getCount() { return dwSoldCnt + dwLockCnt; }
  void setCount(DWORD cnt) { dwSoldCnt = cnt; }
  void lockCnt(DWORD cnt) 
  {
    dwLockCnt += cnt;
  }
  void unlockCnt(DWORD cnt)
  {
    if (dwLockCnt >= cnt)
      dwLockCnt -= cnt;
    else
      dwLockCnt = 0;
  }
};

class GlobalShopMgr :public xSingleton<GlobalShopMgr>
{
public:
  GlobalShopMgr();
  virtual ~GlobalShopMgr();
  
  void timeTick(DWORD curTime);
  void loadDb();
  void saveDb();
  
  void querySoldCnt(Cmd::QuerySoldCntMatchSCmd& rev);
  void checkSoldCnt(Cmd::CheckCanBuyMatchSCmd& cmd);
  void addSoldCnt(Cmd::AddBuyCntMatchSCmd& cmd);
  SoldCnt* getMutableSoldCnt(DWORD id);
  void onOpenPanel(QWORD qwCharId, DWORD dwZoneid);
  void onClosePanel(QWORD qwCharId);
  void updateCache();
private:
  Cmd::QueryShopSoldCountCmd m_cacheCmd;
  bool m_bInit = false;
  std::map<DWORD/*id*/, SoldCnt> m_mapSoldCnt;
  bool m_bSaveDb = false;
  DWORD m_dwNextSaveTime = 0;
  std::map<QWORD/*charid*/, DWORD/*zoneid*/> m_mapOpenUser;
};
