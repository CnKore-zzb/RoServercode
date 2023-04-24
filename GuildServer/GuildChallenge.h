#pragma once

#include "xNoncopyable.h"
#include "GuildConfig.h"

using std::map;

class Guild;
class GMember;

struct SGuildChallenge
{
  DWORD dwID = 0;
  DWORD dwProgress = 0;
  bool bReward = false;
  bool bExtraReward = false;

  bool toData(GuildChallenge* data, GMember* member = nullptr);
  bool fromData(const GuildChallenge& data);
};

class GuildChallengeMgr : private xNoncopyable
{
public:
  GuildChallengeMgr(Guild* guild);
  virtual ~GuildChallengeMgr();

  bool toData(GuildChallengeData* data, GMember* member = nullptr);
  bool fromData(const GuildChallengeData& data);
  bool toGuildData(GuildData* data, GMember* member);

  void timer(DWORD cur);

  void open();
  void updateProgress(GMember* member, const vector<GuildChallengeItem*>& items, DWORD progress = 1, bool checkmember = true);
  void notifyAllData(GMember* member, bool nochallenge = false);
  void resetChallenge(bool force = false);
  DWORD getNextRefreshTime() { return xTime::getWeekStart(now(), 5 * HOUR_T) + 7 * DAY_T + 5 * HOUR_T; }

  void onAddMember(GMember* member);
private:

  Guild* m_pGuild = nullptr;

  map<DWORD, SGuildChallenge> m_mapChallenge;
  map<DWORD, DWORD> m_mapID2Index;
  vector<DWORD> m_vecChallengeIDIndex;
};
