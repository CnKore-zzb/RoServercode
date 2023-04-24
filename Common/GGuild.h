/**
 * @file GGuild.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-05-06
 */

#pragma once

#include <list>
#include "xDefine.h"
#include "GuildSCmd.pb.h"
#include "GuildCmd.pb.h"

using std::list;
using std::map;
using std::string;
using namespace Cmd;

typedef map<QWORD, GuildSMember> TMapGGuildMember;
typedef map<DWORD, GuildQuest> TMapGuildQuest;
typedef map<EGuildBuilding, GuildBuilding> TMapGuildBuilding;
typedef map<string, GuildArtifactItem> TMapGuildArtifact;

class GGuild
{
  public:
    GGuild();
    ~GGuild();

    static bool add_gdata(GuildDataUpdate* pData, EGuildData eType, QWORD qwValue, const string& strData = "");
    static bool add_mdata(GuildMemberDataUpdate* pData, EGuildMemberData eType, QWORD qwValue, const string& strData = "");
    static bool isActive(DWORD dwDayStart, DWORD dwOnline, DWORD dwOffline) { return dwOnline >= dwOffline || (dwOnline >= dwDayStart - DAY_T && dwOnline <= dwDayStart + DAY_T); }
    static bool isSame(const GuildPhoto& r1, const GuildPhoto& r2);
    static bool hasAuth(DWORD dwAuth, EAuth eAuth);
    static string getPhotoGUID(const GuildPhoto& r);

    bool toData(GuildInfo* pInfo);
    bool toData(GuildUserInfo* pInfo);
    bool toData(GuildArtifactQuest* pInfo);

    void setAccID(QWORD qwID) { m_qwAccID = qwID; }
    void setCharID(QWORD qwID) { m_qwCharID = qwID; }

    QWORD id() const { return m_oInfo.id(); }
    QWORD lv() const { return m_oInfo.lv(); }
    DWORD zoneid() const { return m_oInfo.zoneid(); }
    DWORD auth() const { return m_oInfo.auth(); }
    DWORD contribute() const { return m_oUserInfo.contribute(); }
    void set_contribute(DWORD dwCon) { m_oUserInfo.set_contribute(dwCon); }
    DWORD giftpoint() const { return m_oUserInfo.giftpoint(); }
    bool create() const { return m_oInfo.create(); }
    bool hasAuth(EAuth eAuth) { return GGuild::hasAuth(m_oInfo.auth(), eAuth); }
    bool isGQuestSubmit(DWORD dwID) const { return m_setGQuest.find(dwID) != m_setGQuest.end(); }
    bool inSuperGvg() const { return m_oInfo.gvg().insupergvg(); }

    const string& name() const { return m_oInfo.name(); }
    const string& portrait() const { return m_oInfo.portrait(); }
    const string& jobname() const { return m_oInfo.jobname(); }

    const GuildUserInfo& getGuildUserInfo() { return m_oUserInfo; }
    const TMapGuildQuest& getQuestList() const { return m_mapQuest; }
    const TMapGuildBuilding& getBuildingList() const { return m_mapBuilding; }
    const TMapGuildArtifact& getArtifactList() const { return m_mapArtifact; }
    const TMapGGuildMember& getMemberList() const { return m_mapMember; }

    const GuildSMember* getChairman() const;
    const GuildSMember* getMember(QWORD id, QWORD accid = 0) const;
    const GuildSMember* randMember(const TSetQWORD& setExclude = TSetQWORD{}) const;
    const GuildQuest* getQuest(DWORD dwQuestID) const;
    const GuildBuilding* getBuilding(EGuildBuilding type) const;
    const GuildArtifactItem* getArtifact(const string& guid) const;
    const ItemData* getArtifactPiece(DWORD dwID) const;
    const TSetDWORD& getGQuestSubmitList() const { return m_setGQuest; }

    void updateInfo(const GuildInfo& rInfo);
    void updateGuild(const GuildInfoSyncGuildSCmd& cmd);
    void updateGuildData(const GuildDataUpdateGuildSCmd& cmd);
    void updateMember(const GuildMemberUpdateGuildSCmd& cmd);
    void updateMemberData(const GuildMemberDataUpdateGuildSCmd& cmd);
    void updateQuest(const GuildQuestUpdateGuildSCmd& cmd);
    void updateUserInfo(const GuildUserInfoSyncGuildCmd& cmd);
    void updateJob(const JobUpdateGuildSCmd& cmd);
    void updateBuilding(const BuildingUpdateGuildSCmd& cmd);
    void updateArtifact(const ArtifactUpdateGuildSCmd& cmd);
    void updateGQuest(const GuildArtifactQuestGuildSCmd& cmd);

    bool isFunctionOpen(EGuildFunction func) { return (m_oInfo.openfunction() & (QWORD(1) << (DWORD(func) - 1))) != 0; }
    DWORD getBuildingLevel(EGuildBuilding type) const;
    DWORD getBuildingNum(DWORD lv) const;

    const TSetString& getNewArtifact() { return m_setNewArtifact; }
    void clearNewArtifact() { m_setNewArtifact.clear(); }
    bool isUserOwnArtifact(QWORD charid, const string& guid) const;
  private:
    QWORD m_qwAccID = 0;
    QWORD m_qwCharID = 0;

    bool m_bInfoInit = false;
    bool m_bUserInfoInit = false;

    GuildInfo m_oInfo;
    GuildUserInfo m_oUserInfo;

    TMapGGuildMember m_mapMember;
    TMapGuildQuest m_mapQuest;
    TSetDWORD m_setGQuest;
    TMapGuildBuilding m_mapBuilding;
    TMapGuildArtifact m_mapArtifact;
    TSetString m_setNewArtifact;
    map<DWORD, ItemData> m_mapPack;
};

