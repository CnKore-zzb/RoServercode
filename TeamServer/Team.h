/**
 * @file Team.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-06-12
 */

#pragma once

#include "xDefine.h"
#include "xPool.h"
#include "SessionTeam.pb.h"
#include "ProtoCommon.pb.h"
#include "SessionCmd.pb.h"
#include "TeamCmd.pb.h"
#include "SysMsg.pb.h"
#include "TableStruct.h"
#include "MsgManager.h"
#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <bitset>
#include "Dojo.pb.h"
#include "GCharManager.h"
#include "TeamServer.h"
#include "FuBenCmd.pb.h"
#include "TeamRaidCmd.pb.h"
#include "MatchCCmd.pb.h"

using std::vector;
using std::string;
using std::set;
using std::bitset;
using std::list;
using std::map;
using namespace Cmd;

class Team;

const DWORD TEAM_RECORD_TIME_MIN = 600;
const DWORD TEAM_RECORD_TIME_MAX = 1200;

struct TDojo
{
  QWORD m_qwTeamId = 0;
  QWORD m_qwGuildId = 0;
  QWORD m_qwSponorId = 0;
  std::string m_sponsorName;
  DWORD m_dwDojoId = 0;
  bool m_bIsOpen = false;
  DWORD m_dwZoneId = 0;
  std::set<QWORD> m_inviteAgree;
  //void clear();
  //void syncDojoToScene();
};

struct STeamTower
{
  DWORD dwCurLayer = 0;
  DWORD dwZoneID = 0;

  UserTowerInfo m_oLeaderInfo;
  TSetQWORD setEnterMembers;

  bool bInviteOn = false;
  bool bFighting = false;

  DWORD dwCurRaidID = 0;
  void clear()
  {
    dwCurLayer = 0;
    dwZoneID = 0;
    setEnterMembers.clear();

    bInviteOn = false;
    bFighting = false;
    dwCurRaidID = 0;
  }
};

struct STeamPveCard
{
  DWORD dwConfigID = 0;

  bool bInviteOn = false;
  bool bFighting = false;

  TSetQWORD setEnterMembers;

  void clear()
  {
    dwConfigID = 0;
    bInviteOn = false;
    bFighting = false;
    setEnterMembers.clear();
  }
};

struct STeamRaid
{
  ERaidType eType = ERAIDTYPE_MIN;

  bool bInviteOn = false;
  bool bFighting = false;

  DWORD dwZoneID = 0;

  TSetQWORD setEnterMembers;

  void clear()
  {
    bInviteOn = false;
    bFighting = false;
    dwZoneID = 0;

    setEnterMembers.clear();
  }
};

enum ECatRemoveMethod
{
  ECATREMOVEMETHOD_EARLY = 0,
};

// team member
class TMember
{
  friend class Team;
  public:
    TMember(Team* pTeam, DWORD dwAccid, QWORD qwCharID, DWORD dwZoneID, ETeamJob eJob);
    TMember(Team* pTeam, const TeamMember &data);
    TMember(Team* pTeam, const UserInfo& data, ETeamJob eJob);
    ~TMember();

  private:
    bool initData();
  public:
    void load(const TeamMember& rData);
  public:
    void toData(TeamMember* pData, bool bClient = false);
    void toData(TeamApply* pData);
    void toData(TeamMemberInfo* pInfo);

    void updateData(const UserInfo& rInfo, bool bInit = false);
    GCharReader& getGCharReader() { return m_oGCharData; }

    DWORD getAccid() const { return m_oGCharData.getAccID(); }
    QWORD getGUID() const { return m_oGCharData.getCharID(); }
    DWORD getMapID() const { return m_oGCharData.getMapID(); }
    void setMapID(DWORD mapId)
    {
      if (m_oGCharData.getMapID() != mapId)
      {
        m_oGCharData.setMapID(mapId);
        m_bitset.set(EMEMBERDATA_MAPID);
        updateData();
      }
    }

    void setZoneID(DWORD dwZoneID)
    {
      if (m_oGCharData.getZoneID() != dwZoneID)
      {
        m_oGCharData.setZoneID(dwZoneID);
        m_bitset.set(EMEMBERDATA_ZONEID);
        updateData();
      }
    }
    DWORD getZoneID() const { return m_oGCharData.getZoneID(); }

    EProfession getProfession() const { return m_oGCharData.getProfession(); }
    EGender getGender() const { return m_oGCharData.getGender(); }

    DWORD getCreateTime() const { return m_dwCreateTime; }

    void setBaseLv(DWORD lv) { m_oGCharData.setBaseLevel(lv); }
    DWORD getBaseLv() const { return m_oGCharData.getBaseLevel(); }

    void setJob(ETeamJob eJob) { m_eJob = eJob; m_bitset.set(EMEMBERDATA_JOB); updateData(); }
    ETeamJob getJob() const { return m_eJob; }

    void setTarget(QWORD qwTargetID) { m_qwTargetID = qwTargetID; m_bitset.set(EMEMBERDATA_TARGETID); updateData(); }
    QWORD getTarget() const { return m_qwTargetID; }

    void setGuildID(QWORD qwGuildID) { m_oGCharData.setGuildID(qwGuildID); m_bitset.set(EMEMBERDATA_GUILDID); updateData(); }
    QWORD getGuildID() const { return m_oGCharData.getGuildID(); }

    void setCatID(DWORD dwCatID) { m_dwCatID = dwCatID; }
    DWORD getCatID() const { return m_dwCatID; }

    void setCatOwnerID(QWORD qwOwnerID) { m_qwCatOwnerID = qwOwnerID; }
    QWORD getCatOwnerID() const { return m_qwCatOwnerID; }

    void setGuildName(const string& name) { m_oGCharData.setGuildName(name); m_bitset.set(EMEMBERDATA_GUILDNAME); updateData(); }
    const char* getGuildName() const { return m_oGCharData.getGuildName(); }

    void setName(const string& name) { m_oGCharData.setName(name); m_bitset.set(EMEMBERDATA_NAME); updateData(); }

    void setOfflineTime(DWORD dwTime) { m_oGCharData.setOfflineTime(dwTime); m_bitset.set(EMEMBERDATA_OFFLINE); updateData(); }
    void resetOfflineTime(DWORD dwTime) { m_oGCharData.setOfflineTime(dwTime); }
    bool isOnline() const;

    void setAutoFollow(bool bFollow) { m_bAutoFollow = bFollow; m_bitset.set(EMEMBERDATA_AUTOFOLLOW); updateData(); }
    bool getAutoFollow() const { return m_bAutoFollow; }

    void setReliveTime(DWORD dwTime) { m_dwCatReliveTime = dwTime; m_bitset.set(EMEMBERDATA_RELIVETIME); updateData(); }
    DWORD getReliveTime() const { return m_dwCatReliveTime; }

    void setExpireTime(DWORD dwTime) { m_dwCatExpireTime = dwTime; m_bitset.set(EMEMBERDATA_EXPIRETIME); updateData(); }
    DWORD getExpireTime() const { return m_dwCatExpireTime; }

    void setEnterTime(DWORD dwTime) { m_dwEnterTime = dwTime; }
    DWORD getEnterTime() const { return m_dwEnterTime; }

    bool isLeader() const { return ETEAMJOB_LEADER == m_eJob || ETEAMJOB_TEMPLEADER == m_eJob; }
    const char* getName() const { return m_oGCharData.getName(); }

    void setEnsembleSkill(const string& data);
    const TSetDWORD& getEnsembleSkill() const { return m_setEnsembleSkill; }
    void toEnsembleSkillMemberData(MemberData* pData);

    bool sendCmdToMe(const void* buf, WORD len) const;

    void updateSocialData(const SocialListUpdateSocialCmd& cmd);
    bool checkRelation(QWORD qwCharID, ESocialRelation eRelation) { return m_oGCharData.checkRelation(qwCharID, eRelation); }

    void syncBeLeader();
    void sendRealtimeVoiceID();
  private:
    void updateData();
    bool catSync(EMemberData eType) const { return eType == EMEMBERDATA_BASELEVEL || eType == EMEMBERDATA_OFFLINE; }
  private:
    Team* m_pTeam = nullptr;
    GCharReader m_oGCharData;

    QWORD m_qwTargetID = 0;
    QWORD m_qwHandID = 0;
    QWORD m_qwCatOwnerID = 0;
    DWORD m_dwCreateTime = 0;
    DWORD m_dwRaidID = 0;
    DWORD m_dwCatID = 0;
    DWORD m_dwCatReliveTime = 0;
    DWORD m_dwCatExpireTime = 0;
    DWORD m_dwEnterTime = 0;
    ETeamJob m_eJob = ETEAMJOB_MIN;
    bool m_bAutoFollow = true;
    
    //DWORD m_dwHp = 0;
    //DWORD m_dwMaxHp = 0;
    //DWORD m_dwSp = 0;
    //DWORD m_dwMaxSp = 0;
    DWORD m_dwGuildRaidIndex = 0;
    bitset<EMEMBERDATA_MAX> m_bitset;
    TSetDWORD m_setEnsembleSkill;
};
typedef map<QWORD, TMember*> TMapTeamMember;

// team
class Team : public xEntry
{
  friend class TeamManager;
  public:
    Team(QWORD guid, DWORD zoneid);
    virtual ~Team();

  public:
    bool add(TMember *p);
    void del(TMember *p);

    bool fromMemberData(const string& str);
    bool toMemberData(string& str);

    bool fromData(const xRecord& record);
    bool toData(xRecord& record);

    void toSummary(TeamData* pData);
    void toData(TeamData* pData);
    void toData(TeamInfo* pInfo);
    void toData(TeamDataSyncTeamCmd& cmd);
    void toClientData(TeamData* pData, bool bLeader);
    void toSummaryData(TeamData* pData);
    void toSceneData(TeamData* pData);
    void toMatchServerData(TeamData* pData);

    void clearUpdate() { m_bitset.reset(); m_setApplyUpdate.clear(); }

    QWORD getGUID() const { return m_qwGUID; }
    void setGUID(QWORD guid) { m_qwGUID = guid; set_id(guid); }
    DWORD getZoneID() const { return thisServer->getZoneID(); }

    void setTeamType(const TeamGoalBase* pTeamCFG) { if (pTeamCFG == nullptr) return; if( m_pTeamCFG) m_dwLastGoalID = m_pTeamCFG->id; m_pTeamCFG = pTeamCFG; setDataMark(ETEAMDATA_TYPE); broadcastMsg(316, MsgParams(m_pTeamCFG->name)); }
    const TeamGoalBase* getTeamType() const { return m_pTeamCFG; }

    void setMinReqLv(DWORD lv) { m_dwMinLv = lv; setDataMark(ETEAMDATA_MINLV); }
    DWORD getMinReqLv() const { return m_dwMinLv; }
    void setMaxReqLv(DWORD lv) { m_dwMaxLv = lv; setDataMark(ETEAMDATA_MAXLV); }
    DWORD getMaxReqLv() const { return m_dwMaxLv; }

    void setOverTime(DWORD dwOverTime) { m_dwOverTime = dwOverTime; }
    DWORD getOverTime() const { return m_dwOverTime; }

    void setName(const string& name) { m_strName = name; m_bNameUpdate = true; m_bNameRecord = true; broadcastMsg(315, MsgParams(m_strName)); }
    const string& getName() const { return m_strName; }

    void setAuto(EAutoType eAuto) { m_eAutoType = eAuto; setDataMark(ETEAMDATA_AUTOACCEPT); }
    EAutoType getAuto() const { return m_eAutoType; }

    void setPickupMode(DWORD mode) { m_dwPickUpMode = mode; setDataMark(ETEAMDATA_PICKUP_MODE); }
    DWORD getPickupMode() const { return m_dwPickUpMode; }

    void setDataMark(ETeamData eType) { m_bitset.set(eType); m_recordbitset.set(eType); }

    bool canAutoEnter(const SocialUser& rUser);

    // tower
    void setLeaderTowerInfo(const UserTowerInfo& rInfo, QWORD userid);  // { m_stTower.m_oLeaderInfo.CopyFrom(rInfo); }
    const UserTowerInfo& getLeaderTowerInfo() const { return m_stTower.m_oLeaderInfo; }

    void setCurTowerLayer(DWORD dwLayer) { m_stTower.dwCurLayer = dwLayer; }
    DWORD getCurTowerLayer() const { return m_stTower.dwCurLayer; }

    void addTowerEnterMember(QWORD qwCharID) { m_stTower.setEnterMembers.insert(qwCharID); }
    bool createTowerScene(const SocialUser& rUser, EnterTower& cmd);
    bool enterTower(const SocialUser& rUser, EnterTower& cmd);

    void setTowerSceneOpen(DWORD raidid);
    void setTowerSceneClose(DWORD raidid);

    void setTowerInviteOpen(bool bOpen) { m_stTower.bInviteOn = bOpen; }
    bool getTowerInviteOpen() const { return m_stTower.bInviteOn; }
    void setTowerFighting(bool b) { m_stTower.bFighting = b; };
    bool getTowerFighting() const { return m_stTower.bFighting; }
  private:
    STeamTower m_stTower;
  public:
    TMember* getMember(QWORD id);
    TMember* getLeader();
    TMember* getTempLeader();
    TMember* getOnlineLeader();
    void setMemberUpdate(QWORD id) { m_setMemberUpdate.insert(id); }
    DWORD getMemberCount(bool bCatInclude = true);
    DWORD getOnlineMemberCount() const;
    const TMapTeamMember& getTeamMembers() const { return m_mapMembers; }

    bool addApply(TMember *pApply);
    bool removeApply(QWORD id, bool del = true);
    void clearApply();
    TMember* getApply(QWORD id);
    const TMapTeamMember& getApplyList() const { return m_mapApplys; }

    bool summon(QWORD id, DWORD raidid);
    void timer(DWORD curTime);

    //dojo
    bool sponsorDojo(const SocialUser& pUser, Cmd::DojoSponsorCmd& rev);
    bool cancelSponsor(const SocialUser& pUser);
    TDojo* addDojo(QWORD guildId, QWORD qwSponsorId, DWORD dwDojoId, std::string name);
    void delDojo(QWORD guildId);
    bool clearSponsor(QWORD guildId, QWORD charId);
    ESysMsgID canSponsor(QWORD guildId, DWORD dwDojoID, const SocialUser& rUser);
    void getDojoInfo(QWORD guildId, DojoQueryStateCmd& out);
    bool canDojoFollow(const SocialUser& pMe, QWORD f);
    TDojo* getDojo(QWORD guildId);
    TDojo* getDojo(const SocialUser &pUser);
    bool setDojoOpen(QWORD guildId);
    bool setDojoClose(QWORD guildId);
    void onDojoLeaveGuild(const SocialUser& pUser, QWORD guilgId);
    //道场邀请同意
    bool procInviteDojo(const SocialUser& rUser, DojoReplyCmd& rev);
    bool createDojoScene(const SocialUser& rUser, EnterDojo& rev);
    bool enterDojo(const SocialUser& rUser, EnterDojo& rev);

    // 队长完成5次封印
    void onLeaderFinishSeal();
    void callCatEnter();
  private:
    TDojo* mutableDojo(QWORD guildId);
    void onDojoLeaveTeam(const SocialUser& rUser);
  public:
    bool addMember(TMember *pMember);
    bool removeMember(QWORD id);
    bool removeCat(QWORD id, DWORD catid, bool kick);
    bool removeCat(ECatRemoveMethod eMethod);
    //void checkInvalidCat(QWORD qwCharID);
  private:
    bool setLeader(QWORD qwCharID, ETeamJob eJob);
    bool checkLeaderWhenOnline(QWORD qwCharID);

    void updateMember(DWORD curTime);
    void updateData(DWORD curTime);
    void updateRecord(DWORD curTime);
    void updateApply(DWORD curTime);

    void setApplyUpdate();

  public:
    DWORD getRaidZoneID(DWORD raid) const;
    void setRaidZoneID(DWORD raid, DWORD zoneid);
    void delRaidZoneID(DWORD raid);

    bool getRedTip() { return m_bRedTip; }
    void setRedTip(bool red) { m_bRedTip = red; }
  public:
    bool acceptHelpQuest(QWORD qwCharID, DWORD dwQuestID, bool isAbandon);
    void queryHelpQuestList(QWORD qwCharID);
    void updateMemberQuest(const MemberWantedQuest& msg);
    void queryMemberQuestList(QWORD qwCharID);
  private:
    void updateHelpQuest();
    void handleQuestExitTeam(QWORD qwCharID);
    bool toWantedQuestData(string& str);
    bool fromWantedQuestData(const string& str);

  public:
    void broadcastMsg(DWORD dwMsgID, MsgParams oParams = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME);
    void broadcastCmd(const void* buf, WORD len);
    void broadcastCmdToSessionUser(const void* buf, WORD len);
  private:
    const TeamGoalBase* m_pTeamCFG = nullptr;
    DWORD m_dwLastGoalID = 0;

    QWORD m_qwGUID = 0;
    DWORD m_dwZoneID = 0;

    DWORD m_dwMinLv = 0;
    DWORD m_dwMaxLv = 0;

    DWORD m_dwOverTime = 0;

    DWORD m_dwTick = 0;

    EAutoType m_eAutoType = EAUTOTYPE_CLOSE;

    bool m_bNameUpdate = false;
    bool m_bNameRecord = false;
    bool m_bTowerOpen = false;
    bool m_bTowerFighting = false;
    DWORD m_dwPickUpMode = 0;         //0 自由拾取，1 组队均分
    DWORD m_dwSaveTime = 0;

    string m_strName;

    TMapTeamMember m_mapMembers;
    TSetQWORD m_setMemberUpdate;
    TMapTeamMember m_mapApplys;
    TSetQWORD m_setApplyUpdate;

    bitset<ETEAMDATA_MAX> m_bitset;
    bitset<ETEAMDATA_MAX> m_recordbitset;

    std::map<QWORD, TDojo> m_mapDojo;
    std::map<DWORD, DWORD> m_mapRaidZone;

    std::map<QWORD, pair<DWORD, DWORD>> m_mapUser2WantedQuest; // user -> pair <questid, stepid>
    std::map<QWORD, TSetDWORD> m_mapUser2HelpQuest;

  public:
    bool isInPvpRoom();
    void setPvpRoomId(QWORD roomId) { m_pvpRoomId = roomId; }
    QWORD getPvpRoomId() { return m_pvpRoomId;  }
    bool sendDataToMatchServer();

    bool isMatchCreate() { return m_bMatchCreate; }
    void setMatchCreate(bool flag, EPvpType e = EPVPTYPE_MIN) {  m_bMatchCreate = flag; m_eMatchPvpType = e;}
    bool isMatchNoAddMember() { if (m_eMatchPvpType == EPVPTYPE_TEAMPWS || m_eMatchPvpType == EPVPTYPE_TEAMPWS_RELAX) return true; return false; }
    EPvpType getMatchPvpType() { return m_eMatchPvpType; }
  private:
    QWORD m_pvpRoomId = 0;
    bool m_bRedTip = false;
    bool m_bMatchCreate = false;
    EPvpType m_eMatchPvpType = EPVPTYPE_MIN;

  //Pve
  public:
    void setPveInviteOpen(bool open) { m_stPveCard.bInviteOn = open; }
    bool isPveInviteOpen() { return m_stPveCard.bInviteOn; }
    void addPveEnterMember(QWORD userid) { m_stPveCard.setEnterMembers.insert(userid); }
    void cancelEnterPve() { m_stPveCard.setEnterMembers.clear(); setPveInviteOpen(false); }

    bool createPveCardRaid(const SocialUser& rUser, DWORD configid);
    bool isPveFighting() { return m_stPveCard.bFighting; }

    bool onPveCardRaidOpen();
    bool onPveCardRaidClose();
  private:
    STeamPveCard m_stPveCard;

    //TeamRaid
  public:
    STeamRaid* getTeamRaidData(ERaidType eType);
    void addTeamRaidData(ERaidType eType, STeamRaid stData);
    bool getTeamRaidInviteOpen(ERaidType eType);
    void setTeamRaidInviteOpen(ERaidType eType);
    bool addTeamRaidEnterMember(ERaidType eType, QWORD userid);

    bool createTeamRaid(const SocialUser& rUser, TeamRaidEnterCmd& cmd);
    bool enterTeamRaid(const SocialUser& rUser, TeamRaidEnterCmd& cmd);
    bool onTeamRaidOpen(ERaidType eType);
    bool onTeamRaidClose(ERaidType eType);
  private:
    map<ERaidType, STeamRaid> m_mapTeamRaid;
  private:
    string m_strRealtimeVoiceID;
  public:
    const string& getRealtimeVoiceID();

    // 组队排位赛
  public:
    void setMatchingType(EPvpType type) { m_eMatchingType = type; }
    bool isInMatchTeamPws() { return m_eMatchingType == EPVPTYPE_TEAMPWS || m_eMatchingType == EPVPTYPE_TEAMPWS_RELAX; }
    void cancelMatchPws();
  private:
    EPvpType m_eMatchingType = EPVPTYPE_MIN; // 正在匹配的pvp类型
};

