/**
 * @file TeamManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-06-12
 */

#pragma once

#include "xPool.h"
#include "xSingleton.h"
#include "Team.h"
#include "xEntryManager.h"

using std::map;
using std::pair;

typedef map<QWORD, QWORD> TMapUserTeam;
typedef map<DWORD, set<QWORD>> TMapTypeTeam;
typedef pair<QWORD, DWORD> TPairTeamInvite;
typedef vector<TPairTeamInvite> TVecTeamInvite;
typedef map<QWORD, TVecTeamInvite> TMapUserInviteTeam;
typedef map<QWORD, UserInfo> TMapInviteInfo;
typedef map<QWORD, set<QWORD>> TMapUserApplyTeam;
typedef vector<pair<SocialUser, DWORD>> TVecLeaderChangeCache;

class ThTeamData;

struct SQuickUser
{
  QWORD qwCharID = 0;
  DWORD dwEndTime = 0;
  const TeamGoalBase* pBase = nullptr;
};
typedef map<QWORD, SQuickUser> TMapQuickUser;

// team manager
class TeamManager : public xEntryManager<xEntryID>, public xSingleton<TeamManager>
{
  public:
    TeamManager();
    virtual ~TeamManager();

    // 初始化
  public:
    bool loadAllDataFromDB();
    void final();
    void reload();
    // 组队数据
  public:
    bool addTeam(Team *team);
    void removeTeam(Team *team);
    Team* getTeamByID(QWORD id);
    Team* getTeamByUserID(QWORD userid);

  public:
    void delChar(QWORD qwCharID);

    bool isTeamMember(QWORD id) const { return m_mapUserID2Team.find(id) != m_mapUserID2Team.end(); }

  public:
    void onUserOnline(const SocialUser& rUser);
    void onUserOffline(const SocialUser& rUser);
    void updateUserInfo(const UserInfoSyncSocialCmd& cmd);
    void checkQuickEnter(Team* pTeam);

    bool addInviteMember(const UserInfo& rInvite, QWORD qwBeInviteID);
    bool clearInviteMember(QWORD userid);
    DWORD getInviteMemberCount(QWORD userid);
    TPairTeamInvite* getInvite(QWORD inviteid, QWORD userid);
    void timer(DWORD curTime);
    bool doTeamCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
    bool doDojoCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
    bool doTowerCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
    void doThTeamData(ThTeamData *pData);
    bool doPveCardCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
    bool doTeamRaidCmd(const SocialUser& rUser, const BYTE* buf, WORD len);
  private:
    bool queryTeamList(const SocialUser& rUser, DWORD dwType, DWORD dwPage, DWORD dwLv);
  public:
    Team* createOneTeam(const string& name, const UserInfo& rLeader, DWORD dwType, DWORD dwMinLv, DWORD dwMaxLv, EAutoType eAutoType);
    bool inviteMember(const UserInfo& rInvite, SocialUser rBeInvite);
    bool processInviteMember(ETeamInviteType eType, const UserInfo& pUser, QWORD qwLeaderID);
    bool applyTeam(const UserInfo& rApply, QWORD teamid);
    bool applyTeamForce(const UserInfo& rApply, QWORD teamid);
    bool processApplyTeam(ETeamApplyType eType, const SocialUser& pLeader, QWORD id);
    bool kickMember(const SocialUser& pLeader, QWORD userid);
    bool kickCat(const SocialUser& pLeader, QWORD userid, DWORD catid);
    bool exchangeLeader(const SocialUser& pLeader, QWORD userid);
    bool exitTeam(const SocialUser& pUser);
    bool lockTarget(const SocialUser& pUser, QWORD targetID);
    bool summon(const SocialUser& pUser, DWORD raidid);
    bool clearApplyList(const SocialUser& pUser);
    bool quickEnter(const UserInfo& pUser, DWORD dwType, bool bSet);
    bool setTeamOption(const SocialUser& pUser, const SetTeamOption& cmd);
    bool queryUserTeamInfo(const SocialUser& pUser, QWORD qwCharID);
    bool setMemberOption(const SocialUser& rUser, const SetMemberOptionTeamCmd& cmd);
    bool isOnline(QWORD charId) { return m_setOnlineUser.find(charId) != m_setOnlineUser.end(); }
  private:
    bool addMember(Team* pTeam, TMember *pMember, bool force = false);
  public:
    bool removeMember(QWORD id);

    bool addApplyTeam(QWORD qwCharID, Team* pTeam);
    bool removeApplyTeam(QWORD qwCharID, Team* pTeam);
    bool removeApplyTeam(Team* pTeam);

    bool checkTeam(const TeamGoalBase* pTeamCFG, Team* pTeam);
    void updateQuickUser(DWORD curSec);

    const UserInfo* getInviteInfo(QWORD qwCharID) const;

    bool addLeaderChangeCache(const SocialUser& rUser);
    bool removeLeaderChangeCache(QWORD qwCharID);
    void updateLeaderChangeCache(DWORD curTime);
    void resetTempLeader(Team* pTeam);

    void syncRedTip(QWORD teamid, QWORD userid, bool add);
  private:
    TMapUserTeam m_mapUserID2Team;
    TMapTypeTeam m_mapType2Team;
    TMapUserInviteTeam m_mapUserID2InviteTeam;
    TMapInviteInfo m_mapInviteInfo;
    TMapUserApplyTeam m_mapUserApplyTeam;
    TMapQuickUser m_mapQuickEnterUser;
    TVecLeaderChangeCache m_vecLeaderChangeCache;

    std::map<QWORD, std::map<DWORD, DWORD>> m_oQueryTimeList;
    std::set<QWORD /*charid*/> m_setOnlineUser;
};
