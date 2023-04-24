#pragma once

#include "xNoncopyable.h"
#include "GuildConfig.h"

using std::map;

class Guild;
class GMember;

typedef map<DWORD, DWORD> TMapMaterial;
struct SGuildBuilding
{
  EGuildBuilding eType = EGUILDBUILDING_MIN;
  DWORD dwLv = 0;
  bool bIsBuilding = false;
  TMapMaterial mapMaterial;
  DWORD dwNextWelfareTime = 0;
  DWORD dwNextBuildTime = 0;

  bool toData(GuildBuilding* data);
  bool fromData(const GuildBuilding& data);
  bool isBuilding();
};

const DWORD BUILDING_VERSION = 1;

class GuildBuildingMgr : private xNoncopyable
{
public:
  GuildBuildingMgr(Guild* guild);
  virtual ~GuildBuildingMgr();

  bool toData(GuildBuildingData* data);
  bool fromData(const GuildBuildingData& data);

  void timer(DWORD cur);

  bool isSubmitLock() { return now() < m_dwSubmitTime + 30; }
  DWORD lockSubmit() { m_dwSubmitTime = now(); return ++m_dwSubmitCounter; }
  void unlockSubmit() { m_dwSubmitTime = 0; }
  DWORD getSubmitCounter() { return m_dwSubmitCounter; }
  DWORD getLastSubmitTime() { return m_dwSubmitTime; }

  void openBuildingFunction();
  bool build(GMember* member, EGuildBuilding type);
  bool isBuilding(EGuildBuilding type);
  bool submitMaterial(EGuildBuilding type, const TMapMaterial& material, GMember* member = nullptr, DWORD submitinc = 0);
  bool canSubmitMaterial(EGuildBuilding type, const TMapMaterial& material);
  void updateDataToDScene(const set<EGuildBuilding> types);
  void levelup(EGuildBuilding type, DWORD lv);
  bool clearBuildCD(EGuildBuilding type);
  bool cancelBuild(EGuildBuilding type);
  DWORD getBuildingLevel(EGuildBuilding type);

  void sendWelfare(EGuildBuilding type);
  DWORD getNextWelfareTime(EGuildBuilding type);

  void onAddMember(GMember* member);

  const SGuildBuildingCFG* getBuildingCFG(EGuildBuilding type) { return GuildConfig::getMe().getGuildBuildingCFG(type, getBuildingLevel(type)); }
  void querySubmitRank(EGuildBuilding type, GMember* member);
  void updateSubmitRank(EGuildBuilding type, GMember* member);
  void clearSubmitRank(EGuildBuilding type);

private:
  void initSubmitRank(EGuildBuilding type);
  void version();

  Guild* m_pGuild = nullptr;

  map<EGuildBuilding, SGuildBuilding> m_mapBuilding;
  DWORD m_dwSubmitCounter = 0;
  DWORD m_dwSubmitTime = 0;
  map<EGuildBuilding, QueryBuildingRankGuildCmd> m_mapBuildingSubmitRank;
  DWORD m_dwVersion = 0;
};
