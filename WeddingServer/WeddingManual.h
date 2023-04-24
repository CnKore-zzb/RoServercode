#pragma once

#include "xDefine.h"
#include <string>
#include "WeddingSCmd.pb.h"
#include "xTime.h"

using namespace Cmd;
using std::string;
using std::map;
using std::set;

class Wedding;

struct SWeddingInvitee
{
  QWORD qwCharID = 0;
  string strName;
  DWORD dwInviteTime = 0;

  bool fromData(const WeddingInvitee& data)
  {
    qwCharID = data.charid();
    strName = data.name();
    dwInviteTime = data.invitetime();
    return true;
  }
  bool toData(WeddingInvitee* data)
  {
    if (data == nullptr)
      return false;
    data->set_charid(qwCharID);
    data->set_name(strName);
    data->set_invitetime(dwInviteTime);
    return true;
  }
};

class WeddingManual
{
public:
  WeddingManual(Wedding* wedding);
  ~WeddingManual() {}

  bool fromString(const string& data);
  bool toString(string& data);
  bool fromData(const WeddingManualInfo& data);
  bool toData(WeddingManualInfo* data, bool invitee = true);
  bool toClientWeddingManual(ClientWeddingManual* data, DWORD who);

  void init(bool toppackage);

  bool buyPackage(QWORD charid, const BuyWeddingPackageCCmd& cmd);
  bool buyRing(QWORD charid, const BuyWeddingRingCCmd& cmd);
  bool buyRing(DWORD id, bool ntf);
  bool invite(QWORD operatorid, const TSetQWORD& charids);
  bool uploadPhoto(QWORD charid, DWORD index, DWORD time);

  SWeddingInvitee* getInvitee(QWORD charid);
  void notifyInvitee(const set<QWORD>& charids);
  void notifyManual();

  bool handleBuyServiceWeddingSCmd(const BuyServiceWeddingSCmd& cmd);
  void handleUpdateWeddingManualCCmd(QWORD charid);

  void onUserOffline(QWORD charid);
  void onWeddingStart();
  //通知删除邀请函
  void delInvitation();
  void saveWeddingItem(const TVecItemData& items);

  void rename1(const string& name) { m_strName1 = name; }
  void rename2(const string& name) { m_strName2 = name; }
private:
  void addFreePackage(bool ntf);
  void lockBuyService() { m_dwBuyServiceLock = now() + 60; }
  void unlockBuyService() { m_dwBuyServiceLock = 0; }
  bool isBuyServiceLock() { return now() <= m_dwBuyServiceLock; }
  bool buyService(DWORD id, bool ntf);
  void clearService() { m_setServiceID.clear(); }

  string getWeddingTimeString(DWORD starttime, DWORD endtime)
  {
    char str[128];
    bzero(str, sizeof(str));
    struct tm tm, tm1;
    getLocalTime(tm, (time_t)starttime);
    getLocalTime(tm1, (time_t)endtime);
    snprintf(str, sizeof(str), "%u.%u.%u %02u:%02u~%02u:%02u", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm1.tm_hour, tm1.tm_min);
    return str;
  }

  Wedding* m_pWedding = nullptr;
  bool m_bNtf1 = false;
  bool m_bNtf2 = false;
  DWORD m_dwBuyServiceLock = 0;

  TSetDWORD m_setServiceID;
  DWORD m_dwRingID = 0;
  DWORD m_dwPhotoIndex1 = 0;
  DWORD m_dwPhotoIndex2 = 0;
  DWORD m_dwPhotoTime1 = 0;
  DWORD m_dwPhotoTime2 = 0;
  map<QWORD, SWeddingInvitee> m_mapInvitee;
  string m_strName1;
  string m_strName2;
  TVecItemData m_vecItemRecord;
};
