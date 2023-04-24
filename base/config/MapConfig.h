/**
 * @file MapConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-05-10
 */

#pragma once

#include "xSingleton.h"
#include "FuBenCmd.pb.h"
#include "GuildSCmd.pb.h"
#include "xPos.h"

const DWORD MAP_PRONTERA = 1;
const DWORD VIRTUAL_MAP = 10007;

using namespace Cmd;
using std::vector;
using std::string;
using std::map;
using std::pair;

// config data
enum EMapType
{
  EMAPTYPE_MIN = 0,
  EMAPTYPE_PUBLIC = 1,
  EMAPTYPE_PRIVATERAID = 2,
  EMAPTYPE_TEAMRAID = 3,
  EMAPTYPE_GUILDRAID = 4,
  EMAPTYPE_ACTIVERAID = 5,
  EMAPTYPE_MAIN = 6,
  EMAPTYPE_ROOM = 7,
  EMAPTYPE_PVP = 8,
  EMAPTYPE_PVP_ROOM = 9,      //pvp斗技场
  EMAPTYPE_GUILDFIRE = 10,
  EMAPTYPE_MAX
};

enum EnterCondType
{
  EENTERCONDTYPE_NONE = 0,
  EENTERCONDTYPE_ACTIVITY = 1,    //活动地图
};

struct EnterCond
{
  EnterCondType type = EENTERCONDTYPE_NONE;
  DWORD param;
};

enum ERaidRestrict
{
  ERAIDRESTRICT_MIN = 0,
  ERAIDRESTRICT_PRIVATE,
  ERAIDRESTRICT_TEAM,
  ERAIDRESTRICT_SYSTEM,
  ERAIDRESTRICT_GUILD,
  ERAIDRESTRICT_GUILD_TEAM,
  ERAIDRESTRICT_USER_TEAM,
  ERAIDRESTRICT_PVP_ROOM,
  ERAIDRESTRICT_GUILD_RANDOM_RAID,
  ERAIDRESTRICT_GUILD_FIRE,
  ERAIDRESTRICT_WEDDING,
  ERAIDRESTRICT_HONEYMOON,
  ERAIDRESTRICT_MAX,
};
enum EMapFunction
{
  EMAPFUNCTION_MIN = 0,
  EMAPFUNCTION_TRANSFORM = 1,
};

struct SWeatherSkyItem
{
  DWORD dwID = 0;
  DWORD dwRate = 0;
  DWORD dwMaxTime = 0;
  TVecDWORD vecSky;

  SWeatherSkyItem() {}
};
typedef vector<SWeatherSkyItem> TVecWeatherSkyItem;

struct SWeatherSkyCFG
{
  DWORD dwID = 0;
  DWORD dwDestTime = 0;
  DWORD dwMaxRate = 0;

  TVecWeatherSkyItem vecItems;

  SWeatherSkyCFG() {}
};

enum ETransType
{
  ETransType_Normal = 1,
  ETransType_MonthCard = 2,
};

struct SMapMonsterRatioCFG
{
  DWORD dwBuffID = 0;
  DWORD dwRatio = 0;
  TSetDWORD setMonsterIDs;
};

struct SMapCFG
{
  DWORD dwID = 0;
  DWORD dwReliveMap = 0;
  DWORD dwReliveBp = 0;
  DWORD dwArea = 0;
  DWORD dwRange = 0;
  DWORD dwFunction = 0;
  ETransType eTransType;
  DWORD dwTransMoney = 0;
  SWORD swAdventureValue = 0;

  bool bStatic = false;
  bool bPvp = false;
  bool bPreview = false;
  //bool bNoCat = false;
  DWORD dwStaticGroup = 0;
  DWORD dwPetLimit = 0;
  string strMapBgm;

  EMapType eType = EMAPTYPE_MIN;
  EnterCond enterCond;

  string strNameZh;
  string strNameEn;

  TVecDWORD stSkyCFG;
  SWeatherSkyCFG stWeatherCFG;

  TSetDWORD setManualBuffIDs;
  TVecItemInfo vecManualReward;

  SMapMonsterRatioCFG stMapMonsterRatioCFG;

  TSetDWORD setQuestIds;  //在地图中使用苍蝇翅膀需要完成的任务

  int nIndexRangeMin = 0;
  int nIndexRangeMax = 0;

  SMapCFG() {}

  bool hasFunction(EMapFunction eFunction) const;
  bool noCat() const { return dwPetLimit & 1; }
  bool noPet() const { return dwPetLimit & 2; }
  bool noBeing() const { return dwPetLimit & 4; }
  bool noFollower() const { return dwPetLimit & 8; }
  bool noFood() const { return dwPetLimit & 16; }
  bool noHandNpc() const { return dwPetLimit & 32; }

  DWORD getMonsterRatioBuff(DWORD npcid) const;
  DWORD getMonsterRewardRatio(DWORD npcid) const;
};
typedef map<DWORD, SMapCFG> TMapMapCFG;

struct SRaidCFG
{
  DWORD dwRaidID = 0;
  DWORD dwMapID = 0;

  DWORD dwRaidLimitTime = 0;
  DWORD dwRaidTimeNoUser = 0;
  DWORD dwRaidEndWait = 0;

  bool bShowAllNpc = false;
  bool bQuestFail = false;

  ERaidType eRaidType = ERAIDTYPE_MIN;
  ERaidRestrict eRestrict = ERAIDRESTRICT_MIN;

  string strNameZh;
  string strNameEn;

  bool bNoPlayGoMap = false;
  DWORD dwLeaveImageTime = 0;
  DWORD dwLimitLv = 0;

  SRaidCFG() {}
};
typedef map<DWORD, SRaidCFG> TMapRaidCFG;

enum class GoMapType
{
  Null = 0,
  System = 1,
  RemoveTeamMemeber = 2,
  Freyja = 3,
  Tower = 4,                // 无限塔
  ExitPoint = 5,
  KickUser = 6,
  Follow = 7,               // 跟随
  Quest = 8,                // 任务追踪
  Laboratory = 9,           // 研究所
  GoCity = 10,              // 回城
  Relive = 11,
  Skill = 12,
  GM = 13,
  GMFollow = 14,
  GoPVP = 15,
  Carrier = 16,
  Hands = 17,
  Image = 18,
  GoBack = 19,
};

struct CreateDMapParams
{
  QWORD qwCharID = 0;
  QWORD qwGuildID = 0;

  DWORD dwGuildLv = 0;
  DWORD dwRaidID = 0;
  DWORD m_dwImageRange = 0;
  DWORD m_dwSealID = 0;
  DWORD m_dwDojoId = 0;
  DWORD dwLayer = 0;
  QWORD m_qwRoomId = 0;       //pvp房间id, 婚礼id
  DWORD m_dwGuildRaidIndex = 0;
  DWORD m_dwNpcId = 0;
  DWORD m_dwNoMonsterLayer = 0; // 无限塔不招小怪的最大层数

  ERaidRestrict eRestrict = ERAIDRESTRICT_MIN;

  GoMapType m_oType = GoMapType::Null;

  xPos m_oEnterPos;
  xPos m_oImageCenter;

  TVecQWORD vecMembers;

  GuildInfo oGuildInfo;

  CreateDMapParams() {}
  CreateDMapParams(QWORD charid, DWORD raidid) : qwCharID(charid), dwRaidID(raidid) {}
};

struct SRaidMonsterCFG
{
  DWORD dwGroupID = 0;
  DWORD dwWeight = 0;
  map<DWORD, TVecDWORD> mapUniqueID2NpcIDs;
};
typedef vector<SRaidMonsterCFG> TVecRaidMonster;
typedef map<DWORD, TVecRaidMonster> TMapRaidMonsterCFG;

// config
class MapConfig : public xSingleton<MapConfig>
{
  friend class xSingleton<MapConfig>;
  private:
    MapConfig();
  public:
    virtual ~MapConfig();

    bool loadConfig();
    bool checkConfig();

    bool isRaidMap(DWORD dwID) const { return getRaidCFG(dwID) != nullptr; }
    bool isUnopenMap(DWORD dwID) const { return m_setUnopenMapIDs.find(dwID) != m_setUnopenMapIDs.end(); }

    const SMapCFG* getMapCFG(DWORD dwMapID) const;
    const SRaidCFG* getRaidCFG(DWORD dwRaidID) const;

    const TMapMapCFG& getMapCFGList() const { return m_mapMapCFG; }
    DWORD getItemImageRaid(DWORD mapId) const;
    void getRaidMonster(DWORD groupid, map<DWORD, DWORD>& outUid2NpcID) const;
    const TSetDWORD& getRaidMonsterUids(DWORD dwGroupID) const;

    void addTimerKey(QWORD dwKeyID) { m_setTimerKeyIDS.insert(dwKeyID); }
    bool checkHaveTimerKey(QWORD dwKeyID) const { return m_setTimerKeyIDS.find(dwKeyID) != m_setTimerKeyIDS.end(); }
  private:
    bool loadMapConfig();
    bool loadRaidMapConfig();
    bool loadRaidMonsterConfig();
  private:
    TMapMapCFG m_mapMapCFG;
    TMapRaidCFG m_mapRaidCFG;
    TMapRaidMonsterCFG m_mapRaidMonsterCFG;
    map<DWORD, TSetDWORD> m_mapRaidMonsterUids;

    TSetDWORD m_setUnopenMapIDs;
    std::map<DWORD/*mapid*/, DWORD/*raidid*/> m_mapItemImage;

    TSetQWORD m_setTimerKeyIDS;
};

