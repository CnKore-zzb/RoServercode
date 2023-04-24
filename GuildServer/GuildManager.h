/**
 * @file GuildManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-06-12
 */

#pragma once

#include "xSingleton.h"
#include "xEntryManager.h"
#include "Guild.h"
#include "SocialCmd.pb.h"
#include "SysMsg.pb.h"
#include "xTime.h"

using std::map;
using namespace Cmd;

typedef map<QWORD, Guild*> TMapUserGuild;
typedef map<string, Guild*> TMapNameGuild;
typedef set<string> TSetGuildName;

// guild offline
struct SGuildOffline
{
  QWORD qwCharID = 0;
  QWORD qwGUILDID = 0;

  DWORD dwContribute = 0;
  DWORD dwTotalContribute = 0;
  DWORD dwGiftPoint = 0;
  DWORD dwExitTime = 0;

  bool bFreeze = false;

  string strPray;
  string strDonate;
  string strVar;
  string strBuilding;

  SGuildOffline() {}
};
typedef map<QWORD, SGuildOffline> TMapUserGuildOffline;

// guild apply
typedef set<Guild*> TSetGuild;
typedef map<QWORD, TSetGuild> TMapUserApplyGuild;

// guild city
typedef map<DWORD, map<DWORD, QWORD>> TMapCityGuild; // <zone, <city, guild>>

// guild manager
class GuildManager : public xEntryManager<xEntryID>, public xSingleton<GuildManager>
{
  public:
    GuildManager();
    virtual ~GuildManager();

    // 工会管理
  public:
    bool addGuild(Guild *p);
    void delGuild(Guild *p);
    Guild* getGuildByID(QWORD qwGuildID);
    Guild* getGuildByUserID(QWORD qwCharID);
    Guild* getGuildByGuildName(const string& strName);

    // 初始化
  public:
    bool init();
    void final();
  private:
    bool loadAllGuildData();
    bool loadOfflineData();
    // 重加载
  public:
    void reload(ConfigType type);

    // 删号
  public:
    void delChar(QWORD qwCharID);

    void onUserOnline(const SocialUser& rUser);
    void onUserOffline(const SocialUser& rUser);
    void updateUserInfo(const UserInfoSyncSocialCmd& cmd);
    void syncPrayToScene(const SocialUser& rUser, GuildOptConType eType, bool bLevelup = false);
    void syncPunishTimeToClient(const SocialUser& rUser);

    void timer(DWORD curTime);

    bool isMember(QWORD userid) { return m_mapUserID2Guild.find(userid) != m_mapUserID2Guild.end(); }
    ESysMsgID checkName(const string& name);
    ESysMsgID addName(const string& name);
    bool removeName(const string& name);

    TSetGuild& getApplyGuild(QWORD qwCharID);

    bool moveGuildZone(DWORD dwFromZone, DWORD dwToZone);

    bool doGuildCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
    bool doUserDojoCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
  private:
    bool queryGuildList(const SocialUser& rUser, const string& keyword, DWORD dwPage);
  public:
    bool createGuild(const UserInfo& rUser, const string& name);

    bool applyGuild(const UserInfo& rUser, QWORD qwGuildID);
    bool processApplyGuild(const SocialUser& rUser, QWORD qwCharID, EGuildAction eAction);
    bool inviteMember(const SocialUser& rUser, QWORD qwCharID);
    bool processInviteMember(const UserInfo& rUser, QWORD qwGuildID, EGuildAction eAction);
    bool setOption(const SocialUser& rUser, const SetGuildOptionGuildCmd& cmd);
    bool kickMember(const SocialUser& rUser, QWORD qwCharID);
    bool changeJob(const SocialUser& rUser, QWORD qwCharID, EGuildJob eJob);
    bool exitGuild(const SocialUser& rUser);
    bool exchangeChair(const SocialUser& rUser, QWORD qwNewChair);
    bool dismissGuild(const SocialUser& rUser, bool bSet);
    bool levelupGuild(const SocialUser& rUser);
    bool donate(const SocialUser& rUser, DWORD dwConfigID, DWORD dwTime);
    bool donateList(const SocialUser& rUser);
    bool setDonateFrame(const SocialUser& rUser, bool bDonate);
    bool enterTerritory(const SocialUser& rUser, QWORD qwHandID);
    bool pray(const SocialUser& rUser, QWORD qwNpcID, DWORD dwPrayID);
    bool levelupEffect(const SocialUser& rUser);
    bool queryPackage(const SocialUser& rUser);
    bool exchangeZone(const SocialUser& rUser, DWORD dwZoneID, bool bSet);
    bool memberExchangeZoneAnswer(const SocialUser& rUser, bool bAgree);
    bool queryEventList(const SocialUser& rUser);

    bool addMember(Guild* pGuild, const SocialUser& rUser, EGuildJob eJob);
    bool removeMember(Guild* pGuild, QWORD qwCharID, bool bFreeze);

    bool addApplyGuild(QWORD qwCharID, Guild* pGuild);
    bool removeApplyGuild(QWORD qwCharID, Guild* pGuild, bool addMember);
    bool removeApplyGuild(Guild* pGuild);

    QWORD getCityOwner(DWORD dwZoneID, DWORD dwCityID);
    void setCityOwner(DWORD dwZoneID, DWORD dwCityID, QWORD qwGuildID);

    bool subContribute(const SocialUser& rUser, DWORD dwCon);
    bool addContribute(const SocialUser& rUser, DWORD dwCon, DWORD dwSource);
    bool applyGuildReward(const SocialUser& rUser, DWORD id);
    bool frameStatus(const SocialUser& rUser, bool bOpen);
    bool modifyAuth(const SocialUser& rUser, const ModifyAuthGuildCmd & cmd);
    bool cityAction(const SocialUser& rUser, ECityAction eAction);

    bool openFunction(const SocialUser& rUser, EGuildFunction func);
    bool build(const SocialUser& rUser, EGuildBuilding building);
    bool submitMaterial(const SocialUser& rUser, const SubmitMaterialGuildCmd& cmd);
    void querySubmitCount(const SocialUser& rUser, EGuildBuilding type);
    void buildingLevelUpEffect(const SocialUser& rUser, const BuildingLvupEffGuildCmd& cmd);

    void getWelfare(const SocialUser& rUser);

    void produceArtifact(const SocialUser& rUser, const ArtifactProduceGuildCmd& cmd);
    void optArtifact(const SocialUser& rUser, const ArtifactOptGuildCmd& cmd);

    bool queryGQuest(const SocialUser& rUser);
    bool processOfflineGM();
    bool queryBuildingSubmitRank(const SocialUser& rUser, EGuildBuilding type);
    bool queryTreasureResult(const SocialUser& rUser, const QueryTreasureResultGuildCmd& cmd);
    void openArtifactFixPack(bool open) { m_bArtifactFixPack = open; }
    bool openRealtimeVoice(const SocialUser& rUser, QWORD memberid, bool open);

    void addAdjustCharID(QWORD charid) { m_setAdjustMemberIDs.insert(charid); }
  private:
    bool obtainOfflineData(Guild* pGuild, GMember* pMember);
    bool updateOffline(DWORD curTime, bool bFinal = false);
    void fixGuildPrayRestore();
    void adjustMemberRedis();
    const SGuildOffline* getOfflineData(QWORD charid) const;
  public:
    bool checkRename(const SocialUser& rUser, const string& name);
    bool rename(const SocialUser& rUser, const string& name);

  private:
    TMapUserGuild m_mapUserID2Guild;
    TMapNameGuild m_mapName2Guild;
    TMapUserGuildOffline m_mapUserGuildOffline;
    TMapUserApplyGuild m_mapUserApplyGuild;

    TSetGuildName m_setGuildName;
    TSetQWORD m_setGOfflineUpdate;

    TSetQWORD m_setAdjustMemberIDs;
    TMapCityGuild m_mapCityGuild;

    //xDayTimer m_oDayTimer;
    DWORD m_dwOfflineRecordTick = 0;
    bool m_bArtifactFixPack = false;
  private:
    void patch_1();
    void patch_2();
    void patch();
};

