/**
 * @file GTeam.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-03-09
 */
#pragma once

#include "xDefine.h"
#include "TeamCmd.pb.h"

using std::map;
using std::string;
using namespace Cmd;

typedef map<QWORD, TeamMemberInfo> TMapGTeamMember;

class GTeam
{
  public:
    GTeam();
    ~GTeam();

    static bool add_mdata(MemberData* pData, EMemberData eType, QWORD qwValue, const string& strData = "");

    bool toData(TeamInfo* pInfo);

    void setCharID(QWORD qwCharID) { m_qwCharID = qwCharID; }
    void clear();
    bool available() { return (m_bInfoInit == true); }

    void updateTeam(const TeamDataSyncTeamCmd& cmd);
    void updateTeamData(const TeamDataUpdateTeamCmd& cmd);
    void updateMember(const TeamMemberUpdateTeamCmd& cmd);
    void updateMemberData(const MemberDataUpdateTeamCmd& cmd);

    const TMapGTeamMember& getTeamMemberList(bool includeCat = false) const { return includeCat ? m_mapMember : m_mapMemberNoCat; }
    const TeamMemberInfo* getTeamMember(QWORD qwCharID) const;

    QWORD getTeamID() const { return m_qwTeamID; }
    QWORD getLeaderID() const { return m_qwLeaderID; }
    QWORD getTrueLeaderID()const { return m_qwTrueLeaderID; }
    QWORD getTeampLeaderID()const { return m_qwTempLeaderID; }
    DWORD getPickupMode() const { return m_dwPickupMode; }
    const string& getName()const { return m_strName; }
    const string& getLeaderName() const;
    bool isPickupShare() const { return m_dwPickupMode == 1; }
  private:
    void updateNoCatMembers();
  private:
    QWORD m_qwCharID = 0;
    QWORD m_qwTeamID = 0;
    QWORD m_qwLeaderID = 0;
    QWORD m_qwTrueLeaderID = 0;
    QWORD m_qwTempLeaderID = 0;
    DWORD m_dwPickupMode = 0;
    string m_strName;

    bool m_bInfoInit = false;

    TMapGTeamMember m_mapMember;
    TMapGTeamMember m_mapMemberNoCat;
};

