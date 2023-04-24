#pragma once

#include "xNoncopyable.h"
#include "GuildConfig.h"

class Guild;
class GMember;

struct SGuildWelfareItem
{
  EGuildWelfare eType = EGUILDWELFARE_MIN;
  QWORD qwID = 0;
  ESource eSource = ESOURCE_MIN;
  DWORD dwRewardID = 0;
  DWORD dwCreateTime = 0;
  DWORD dwOverdueTime = 0;
  set<QWORD> setCharID;

  string strOwnerName;
  DWORD dwSourceID = 0;
  DWORD dwEventGUID = 0;
  DWORD dwIndex = 0;

  bool toData(GuildWelfareItem* data);
  bool fromData(const GuildWelfareItem& data);
  bool canGetWelfare(GMember* member);
  void addUser(QWORD charid) { setCharID.insert(charid); }
};

class GuildWelfareMgr : private xNoncopyable
{
public:
  GuildWelfareMgr(Guild* guild);
  virtual ~GuildWelfareMgr();

  bool toData(GuildWelfare* data);
  bool fromData(const GuildWelfare& data);

  bool addWelfare(EGuildWelfare type, DWORD rewardid, ESource source = ESOURCE_MIN, QWORD id = 0, const string& name = "", DWORD dwSourceID = 0, DWORD dwEventGUID = 0, DWORD dwIndex = 0);
  bool getWelfare(GMember* member);
  void notifyMember(GMember* member, bool add);
  void notifyMember(GMember* member);
  bool isMemberHasWelfare(GMember* member, EGuildWelfare type, QWORD id);
  void removeOverdueWelfare();

  DWORD getCurTotalMember() const { return m_dwCurTotalMember; }
  SGuildWelfareItem* getWelfareItem(EGuildWelfare eType, DWORD dwEventGUID);
private:
  Guild* m_pGuild = nullptr;

  map<EGuildWelfare, vector<SGuildWelfareItem>> m_mapWelfare;
  DWORD m_dwCurTotalMember = 0;
};
