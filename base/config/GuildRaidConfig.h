
#pragma once

#include "xSingleton.h"
#include "xPos.h"
#include <algorithm>

using std::map;
using std::vector;
using std::set;
using std::random_shuffle;
using std::size_t;
using std::swap;
using std::pair;

// ------- static config ------------
enum EGuildRaidMapType
{
  EGUILDRAIDMAP_MIN = 0,
  EGUILDRAIDMAP_ENTRANCE = 1,
  EGUILDRAIDMAP_PASS = 2,
  EGUILDRAIDMAP_NODE = 3,
  EGUILDRAIDMAP_END = 4,
};

// mapid, maptype, exitids
struct SGuildRaidMap
{
  DWORD dwMapID = 0;

  EGuildRaidMapType eMapType = EGUILDRAIDMAP_MIN;
  TSetDWORD setExitIDs;
  bool haveBoss;

  void getRandomExitIDs(TVecDWORD& vecIDs) const;
};
typedef map<DWORD, SGuildRaidMap> TMapGuildRaidMap;
typedef map<EGuildRaidMapType, TVecDWORD> TMapType2GuildRaidIDs;

struct SGuildRaidMonster
{
  DWORD dwNpcID = 0;
  DWORD dwUniqID = 0;
  DWORD dwWeight = 0;
  DWORD dwTeamGroup = 0;

  TSetDWORD setLvs;
  TSetDWORD setBossReward;
};
typedef map<DWORD, SGuildRaidMonster> TMapGuildRaidMonster;
typedef map<DWORD, TSetDWORD> TMapGroupID2Monsters;

typedef vector<SGuildRaidMonster> TVecGuildRaidMonster;
typedef map<DWORD, TVecGuildRaidMonster> TMapUniqID2RaidMonser;
typedef map<DWORD, TMapUniqID2RaidMonser> TMapLv2RaidMonster;

// ------- static config ------------

// -------- dynamic info ----------
struct SGuildRaidLink
{
  DWORD dwMyExitID = 0; // current map ep index

  DWORD dwNextIndex = 0;
  DWORD dwNextMapID = 0;
  DWORD dwNextBornPoint = 0;
  DWORD dwGroup = 0;

  DWORD getMapIndex() const { return dwGroup * ONE_THOUSAND + dwNextIndex; }
};
typedef map<DWORD, SGuildRaidLink> TMapGuildRaidLink;

struct SGuildRaidInfo
{
  DWORD dwIndex = 0;
  DWORD dwMapID = 0;
  DWORD dwGroupID = 0;

  TMapGuildRaidLink mapExit2Link;

  const SGuildRaidLink* getNextMapInfo(DWORD dwExitID) const
  {
    auto it = mapExit2Link.find(dwExitID);
    return it != mapExit2Link.end() ? &(it->second) : nullptr;
  }
  DWORD getMapIndex() const { return dwGroupID * ONE_THOUSAND + dwIndex; }
};
typedef map<DWORD, SGuildRaidInfo> TMapGuildRaidInfo;
typedef map<DWORD, TMapGuildRaidInfo> TMapGuildRaid;
// -------- dynamic info ----------

// 公会战相关配置
typedef map<DWORD, DWORD> TMapTreasure;
struct SGuildCityCFG
{
  DWORD dwCityID = 0;

  DWORD dwMapID = 0;
  //DWORD dwEpID = 0;
  DWORD dwTeleMapID = 0;
  DWORD dwTeleBpID = 0;
  DWORD dwRaidID = 0;
  bool bOpen = false;
  bool bSuper = false; // 是否可以参加公会战决战

  TSetDWORD setGroupRaids;
  map<DWORD, TMapTreasure> mapTreasure;

  xPos oFlag;

  string strName;
  map<DWORD, vector<xPos>> mapSafeMapID2Pos;
  map<DWORD, TSetDWORD> mapLimitMapID2Ep; // 仅防守方可以传送的Ep点

  bool getRecPosAndWidth(DWORD mapid, xPos& p1, xPos& p2, float& w) const;
  bool hasSafaArea(DWORD mapid) const { return mapSafeMapID2Pos.find(mapid) != mapSafeMapID2Pos.end(); }
  bool isDefLimitEp(DWORD mapid, DWORD ep) const // 仅防守方可使用的ep
  {
    auto it = mapLimitMapID2Ep.find(mapid);
    return it != mapLimitMapID2Ep.end() ? (it->second.find(ep) != it->second.end()) : false;
  }

  bool collectTreasure(DWORD dwTimes, TMapTreasure& mapResult) const
  {
    mapResult.clear();
    if (mapTreasure.empty() == true)
      return false;
    auto m = mapTreasure.find(dwTimes);
    if (m == mapTreasure.end())
    {
      auto mm = mapTreasure.rbegin();
      if (dwTimes < mm->first)
        return false;
      mapResult = std::move(mapTreasure.rbegin()->second);
      return true;
    }

    mapResult = std::move(m->second);
    return true;
  }
};
typedef map<DWORD, SGuildCityCFG> TMapGuildCityCFG;

class GuildRaidConfig : public xSingleton<GuildRaidConfig>
{
  friend class xSingleton<GuildRaidConfig>;

  private:
    GuildRaidConfig();
  public:
    virtual ~GuildRaidConfig();

    bool loadConfig();
    bool checkConfig();

    bool createGuildRaid(bool bIgnoreTime = false);
  private:
    bool loadMapConfig();
    bool loadMonsterConfig();
    bool loadGuildCityConfig();

  // raid
  public:
    const SGuildRaidInfo* getGuildRaidInfo(DWORD dwRaidIndex) const;
    const SGuildRaidInfo* getGuildEntrance(DWORD dwRaidGroup) const;
  public:
    bool isGuildRaidScene(DWORD dwMapID) const { return m_mapGuildRaidMap.find(dwMapID) != m_mapGuildRaidMap.end(); }

  // monster
  public:
    bool getRandomMonster(DWORD lv, DWORD uid, TSetDWORD& setNpcs) const;
    bool getRandomMonster(DWORD lv, const TSetDWORD& uids, TSetDWORD& setNpcs, DWORD srandGuildID = 0) const;
    bool isBossMonster(DWORD npcid) const;
    const TSetDWORD& getBossReward(DWORD npcid) const;

  private:
    bool createGuildRaidRoute(DWORD group, DWORD& idx, TVecDWORD& nodeNums, TVecDWORD& haveBosss, DWORD depth, EGuildRaidMapType type, SGuildRaidInfo* preInfo, SGuildRaidLink* preLink);
    const SGuildRaidMap* getGuildRaidMap(DWORD id) const;
    const SGuildRaidMap* getRandGuildRaidMapByType(EGuildRaidMapType type, bool haveBoss, DWORD linkNum) const;
  private:
    // static config
    TMapGuildRaidMap m_mapGuildRaidMap;
    TMapType2GuildRaidIDs m_mapType2GuildRaids;

    DWORD m_dwMaxPassLinkNode;
    DWORD m_dwMaxLinkNum;

    // dynamic data
    TMapGuildRaid m_mapGuildRaid;

    // raid monster
    TMapGuildRaidMonster m_mapGuildRaidMonster;
    TMapLv2RaidMonster m_mapLv2RaidMonster;
    TMapGroupID2Monsters m_mapGroup2Mosnter;

    DWORD m_dwNextCreatTime = 0;

  public:
    bool resetRaid(DWORD timeversion);
    void clearResetVersion() { m_dwResetTimeVersion = 0; }
    DWORD getResetVersion() { return m_dwResetTimeVersion; }
  private:
    DWORD getSRandTime() const;
  private:
    DWORD m_dwResetTimeVersion = 0;


    /*公会战相关*/
  public:
    const SGuildCityCFG* getGuildCityCFG(DWORD cityid) const;
    //const SGuildCityCFG* getGuildCityCFGByEP(DWORD mapid, DWORD epid) const;
    const SGuildCityCFG* getGuildCityCFGByRaid(DWORD raidid) const;
    bool collectCityID(DWORD dwMapID, TSetDWORD& setIDs) const;
    const TMapGuildCityCFG& getGuildCityCFGList() const { return m_mapGuildCityCFG; }
  private:
    TMapGuildCityCFG m_mapGuildCityCFG;
    map<DWORD, TSetDWORD> m_mapID2CityID;
};


