/**
 * @file GuildConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-05-04
 */

#pragma once

#include <list>
#include "xSingleton.h"
#include "GuildSCmd.pb.h"
#include "TableManager.h"

using namespace Cmd;
using std::map;
using std::string;
using std::vector;
using std::pair;

// config data
struct SGuildCFG
{
  DWORD dwLevel = 0;
  DWORD dwNeedFund = 0;
  DWORD dwLevelupFund = 0;
  DWORD dwAssetMaxPerDay = 0;
  DWORD dwMaxMember = 0;
  DWORD dwViceCount = 0;
  DWORD dwMaintenance = 0;
  DWORD dwMaxPrayLv = 0;
  DWORD dwDonateList = 0;
  DWORD dwDonateRefreshInterval1 = 0;
  DWORD dwDonateRefreshInterval2 = 0;
  DWORD dwDonateRefreshInterval3 = 0;
  DWORD dwDonateRefreshInterval4 = 0;
  DWORD dwQuestTime = 0;
  DWORD dwCommonDivisor = 0;
  DWORD dwChallengeCount = 0;

  TVecItemInfo vecLevelupItem;
  TVecDWORD vecDojoGroup;
  bool isDojoGroupOpen(DWORD dojoGroup) const {
    auto v = find(vecDojoGroup.begin(), vecDojoGroup.end(), dojoGroup);
    return v != vecDojoGroup.end();
  }
  void setCommonDivisor();
  SGuildCFG() {}
};
typedef map<DWORD, SGuildCFG> TMapGuildCFG;

struct SGuildPrayItemCFG
{
  DWORD dwLv = 0;

  TVecAttrSvrs vecAttrs;
  TVecItemInfo vecCosts;
};
struct SGuildPrayCFG : public SBaseCFG
{
  DWORD dwID = 0;
  EPrayType eType = EPRAYTYPE_GODDESS;

  map<DWORD, SGuildPrayItemCFG> mapLvItem;

  SGuildPrayCFG() {}
  const SGuildPrayItemCFG* getItem(DWORD dwLv) const;
  void toData(GuildPrayCFG* pCFG, DWORD dwLv) const;
};
typedef map<DWORD, SGuildPrayCFG> TMapGuildPrayCFG;

typedef vector<pair<DWORD, DWORD>> TVecDayPeriod;

struct SGuildFuncBuildingParam
{
  EGuildBuilding eType = EGUILDBUILDING_MIN;

  DWORD dwShowLv = 0;
  bool bShowWhenBuilding = false;
  bool bShowAfterBuilding = false;
  bool bDisWhenNotBuilding = false;

  DWORD dwDisLv = 99999;

  bool bGearStatus = false;
  DWORD dwBuffWhenLvup = 0;
  DWORD dwCreateDelay = 0;

  bool isShowNpc(const GuildBuilding* b) const;
  bool isDisNpc(const GuildBuilding* b) const;
};
struct SGuildFuncCFG
{
  DWORD dwGuildLv = 0;
  DWORD dwDisGuildLv = 0;
  EGuildFunction eShowFunc = EGUILDFUNCTION_MIN;
  EGuildFunction eDisFunc = EGUILDFUNCTION_MIN;

  DWORD dwUniqueID = 0;
  string strName;

  TVecDWORD vecPeriod;
  TVecDayPeriod vecDayPeriod;
  TSetDWORD setQuestIDs;

  DWORD dwStartTime = 0;
  DWORD dwEndTime = 0;

  SGuildFuncBuildingParam stBuildParam;

  SGuildFuncCFG() {}

  bool isOverTime() const;
  bool isBuildingNpc() const { return stBuildParam.eType != EGUILDBUILDING_MIN; }
};
typedef map<DWORD, SGuildFuncCFG> TMapGuildFuncCFG;

enum EDonateType
{
  EDONATETYPE_MIN = 0,
  EDONATETYPE_NORMAL = 1,
  EDONATETYPE_HIGHER = 2,
  EDONATETYPE_SILVER = 3,
  EDONATETYPE_BOSS = 4,
  EDONATETYPE_MAX,
};
struct SGuildDonateItem
{
  DWORD dwItemID = 0;
  DWORD dwCount = 0;
};
typedef vector<SGuildDonateItem> TVecGuildDonateItem;
struct SGuildDonateCFG
{
  DWORD dwID = 0;
  DWORD dwWeight = 0;
  DWORD dwNextID = 0;
  DWORD dwIfNoActive = 0;

  pair<DWORD, DWORD> lvrange;
  EDonateType eType = EDONATETYPE_MIN;

  TVecGuildDonateItem vecItems;
  TVecItemInfo vecUserReward;
  TVecItemInfo vecGuildReward;

  SGuildDonateCFG() {}

  const SGuildDonateItem* randDonateItem() const;
};
typedef map<DWORD, SGuildDonateCFG> TMapDonateCFG;

// guild quest
struct SGuildQuestCFG : public SBaseCFG
{
  DWORD dwQuestID = 0;
  DWORD dwType = 0;
  DWORD dwNpcID = 0;
  DWORD dwGuildLv = 0;
  DWORD dwActiveMember = 0;
  DWORD dwTime = 0;

  void toData(GuildQuest* pQuest) const;
};
typedef std::list<SGuildQuestCFG> TListGuildQuestCFG;
typedef map<DWORD, TListGuildQuestCFG> TMapGuildQuestCFG;

// photo frame
enum EGuildFrameType
{
  EGUILDFRAMETYPE_HORIZONTAL = 0,
  EGUILDFRAMETYPE_VERTICAL = 1,
  EGUILDFRAMETYPE_MAX = 2,
};
typedef map<EGuildFrameType, TSetDWORD> TMapPhotoFrameCFG;

// guild job
struct SGuildJobCFG
{
  DWORD dwDefaultAuth = 0;
  DWORD dwDefaultEditAuth = 0;

  DWORD dwReqLv = 0;

  EGuildJob eJob = EGUILDJOB_MIN;
  string name;
};
typedef map<EGuildJob, SGuildJobCFG> TMapGuildJobCFG;

// guild building
struct SGuildEquip
{
};
struct SGuildBuildingCFG
{
  EGuildBuilding eType = EGUILDBUILDING_MIN;
  DWORD dwLv = 0;
  string strName;
  map<DWORD, DWORD> mapMaterial;
  xLuaData oUnlockParam;
  DWORD dwRewardID = 0;
  DWORD dwRewardCycle = 0;
  bool bIsMaxLv = false;
  DWORD dwNpcID = 0;
  DWORD dwBuildTime = 0;

  // 建筑升级需要其他建筑达到特定等级
  EGuildBuilding eLvupBuildingType = EGUILDBUILDING_MIN;
  DWORD dwLvupBuildingLv = 0;

  DWORD dwMaxRefineLv = 0;
  DWORD dwMaxStrengthLv = 0;

  set<EEquipType> setRefineType;
  set<EEquipType> setStrengthType;
  std::map<DWORD, TSetDWORD> mapHRefinePos;

  TSetDWORD setShopID;
  DWORD dwShopType = 0;

  DWORD dwArtifactMaxCount = 0;
  DWORD dwArtifactTypeMaxCount = 0;

  DWORD dwStrengthLvAdd = 0;

  bool canRefine(EEquipType eType) const { return setRefineType.find(eType) != setRefineType.end(); }
  bool canStrength(EEquipType eType) const { return setStrengthType.find(eType) != setStrengthType.end(); }
  bool isHighRefinePosUnlock(EEquipPos pos, DWORD t) const {
    auto it = mapHRefinePos.find(pos);
    if (it == mapHRefinePos.end()) return false;
    return it->second.find(t) != it->second.end();
  }
};
typedef map<DWORD, SGuildBuildingCFG> TMapLv2GuildBuildingCFG;
typedef map<EGuildBuilding, TMapLv2GuildBuildingCFG> TMapGuildBuildingCFG;
typedef map<DWORD, EGuildBuilding> TMapShopType2GuildBuilding;

// guild building material
struct SGuildMaterialItemCFG
{
  DWORD dwItemID = 0;
  DWORD dwItemCount = 0;
  DWORD dwWeight = 0;
};
struct SGuildBuildingMaterialCFG
{
  DWORD dwID = 0;
  vector<SGuildMaterialItemCFG> vecItem;
  DWORD dwRewardID = 0;
  DWORD dwFund = 0;

  bool getRandItem(DWORD& itemid, DWORD& itemcount, EGuildBuilding type, QWORD charid, DWORD materialid) const;
};
typedef map<DWORD, SGuildBuildingMaterialCFG> TMapGuildBuildingMaterialCFG;

typedef map<DWORD/*pos*/, DWORD/*lv*/> TMapHighRefinePos2Lv;

// guild challenge
enum EGuildChallenge
{
  EGUILDCHALLENGE_MIN,
  EGUILDCHALLENGE_LOGIN,
  EGUILDCHALLENGE_ENDLESS_TOWER,
  EGUILDCHALLENGE_GUILD_RAID,
  EGUILDCHALLENGE_GUILD_QUEST,
  EGUILDCHALLENGE_GVG,
  EGUILDCHALLENGE_SEAL,
  EGUILDCHALLENGE_WANTED_QUEST,
  EGUILDCHALLENGE_KILL_MVP,
  EGUILDCHALLENGE_KILL_MINI,
  EGUILDCHALLENGE_MAX
};

struct SGuildChallengeCFG
{
  DWORD dwID = 0;
  EGuildChallenge eType = EGUILDCHALLENGE_MIN;
  DWORD dwGroupID = 0;
  DWORD dwTotalProgress = 0;
  DWORD dwSubProgress = 0;
  DWORD dwReward = 0;
  DWORD dwGuildLevel = 0;
  DWORD dwWeight = 0;
  xLuaData oParam;
  string strGroupName;

  bool checkValid(QWORD value) const;
};
typedef map<DWORD, SGuildChallengeCFG> TMapGuildChallengeCFG;
typedef vector<SGuildChallengeCFG> TVecGuildChallengeCFG;
typedef map<EGuildChallenge, TVecGuildChallengeCFG> TMapType2GuildChallenge;
typedef map<DWORD, TVecDWORD> TMapGroup2GuildChallengeID;

struct SArtifactCFG
{
  DWORD dwID = 0;
  string strName;
  DWORD dwType = 0;
  DWORD dwLevel = 0;
  DWORD dwDistributeCount = 0;
  DWORD dwUnlockMsg = 0;
  DWORD dwNpcUniqueID = 0;
  DWORD dwNextLevelID = 0;
  DWORD dwQuestID = 0;
  EGuildBuilding eBuildingType = EGUILDBUILDING_MIN;
  map<DWORD, map<DWORD, DWORD>> mapMaterial;
};
typedef map<DWORD, SArtifactCFG> TMapID2ArtifactCFG;
typedef map<DWORD, DWORD> TMapQuestID2ArtifactID;

// treasure
struct SGuildTreasureCFG : public SBaseCFG
{
  DWORD dwID = 0;
  DWORD dwCityID = 0;
  DWORD dwOrderID = 0;

  EGuildTreasureType eType = EGUILDTREASURETYPE_MIN;

  string strName;

  TSetDWORD setGuildReward;
  map<DWORD, TSetDWORD> mapMemberReward;

  bool collectMemberReward(DWORD dwTime, TSetDWORD& setIDs) const;
};
typedef map<DWORD, SGuildTreasureCFG> TMapGuildTreasureCFG;
typedef vector<SGuildTreasureCFG> TVecGuildTreasureCFG;

// config
class GuildConfig : public xSingleton<GuildConfig>
{
  friend class xSingleton<GuildConfig>;
  private:
    GuildConfig();
  public:
    virtual ~GuildConfig();

    bool loadConfig();
    bool checkConfig();

    const SGuildCFG* getGuildCFG(DWORD dwLv) const;
    const SGuildPrayCFG* getGuildPrayCFG(DWORD dwPrayID) const;
    const SGuildFuncCFG* getGuildFuncCFG(DWORD dwID) const;
    const SGuildDonateCFG* getGuildDonateCFG(DWORD dwID) const;
    const SGuildDonateCFG* randGuildDonateCFG(DWORD dwLv, EDonateType eType) const;
    const TMapGuildFuncCFG& getGuildFuncList() const { return m_mapGuildFuncCFG; }
    const TMapGuildPrayCFG& getGuildPrayList() const { return m_mapGuildPrayCFG; }

    const SGuildQuestCFG* getGuildQuestCFG(DWORD dwQuestID) const;
    const SGuildQuestCFG* randomGuildQuest(DWORD dwGuildLv, DWORD dwActiveMember, const TListGuildQuestCFG& setExclude);

    const SGuildJobCFG* getGuildJobCFG(EGuildJob eJob) const;
    const TMapGuildJobCFG& getGuildJobList() const { return m_mapJobCFG; }

    bool isSuitableFrame(DWORD dwID, DWORD dwAngleZ) const;
    EGuildFrameType getPhotoFrameType(DWORD dwID) const;
    EGuildFrameType getAngleFrameType(DWORD dwAngleZ) const;
    const TMapPhotoFrameCFG& getPhotoFrameList() const { return m_mapPhotoFrameCFG; }

    DWORD getMaxPrayLv() const;
    const SGuildBuildingCFG* getGuildBuildingCFG(EGuildBuilding type, DWORD lv) const;
    const SGuildBuildingMaterialCFG* getGuildBuildingMaterial(DWORD id) const;
    EGuildBuilding getGuildBuildingByShopType(DWORD shoptype) const;

    const SGuildChallengeCFG* getGuildChallengeCFG(DWORD id) const;
    const TVecGuildChallengeCFG& getGuildChallengeCFGByType(EGuildChallenge type) const;
    void randGuildChallenge(TVecDWORD& ids, TVecDWORD& extrarewardids, DWORD guildlv, DWORD count) const;
    const TVecDWORD& getGuildChallengeIDByGroup(DWORD groupid) const;

    const SArtifactCFG* getArtifactCFG(DWORD id) const;
    const SArtifactCFG* getArtifactCFGByQuestID(DWORD id) const;

    const SGuildTreasureCFG* getTreasureCFG(DWORD id) const;
    const TVecGuildTreasureCFG& getGuildTreasureCFGList() const { return m_vecGuildTreasureCFG; }
  private:
    bool loadGuildConfig();
    bool loadGuildPray();
    bool loadGuildFunc();
    bool loadGuildDonate();
    bool loadGuildQuestConfig();
    bool loadGuildPhotoFrame();
    bool loadGuildJob();
    bool loadGuildBuilding();
    bool loadGuildBuildingMaterial();
    bool loadGuildChallenge();
    bool loadArtifact();
    bool loadGuildTreasure();

    EGuildChallenge getChallengeType(const string& type);
  private:
    TMapGuildCFG m_mapGuildCFG;
    TMapGuildPrayCFG m_mapGuildPrayCFG;
    TMapGuildFuncCFG m_mapGuildFuncCFG;
    TMapDonateCFG m_mapDonateCFG;
    TMapGuildQuestCFG m_mapQuestCFG;
    TMapPhotoFrameCFG m_mapPhotoFrameCFG;
    TMapGuildJobCFG m_mapJobCFG;
    TMapGuildBuildingCFG m_mapBuildingCFG;
    TMapGuildBuildingMaterialCFG m_mapBuildingMaterialCFG;
    //TMapHighRefinePos2Lv m_mapHighRefinePos2Lv;
    TMapGuildChallengeCFG m_mapChallengeCFG;
    TMapType2GuildChallenge m_mapType2Challenge;
    TMapGroup2GuildChallengeID m_mapGroup2ChallengeID;
    TMapShopType2GuildBuilding m_mapShopType2Building;
    TMapID2ArtifactCFG m_mapID2ArtifactCFG;
    TMapQuestID2ArtifactID m_mapQuestID2ArtifactID;
    TMapGuildTreasureCFG m_mapTreasureCFG;
    TVecGuildTreasureCFG m_vecGuildTreasureCFG;
};

