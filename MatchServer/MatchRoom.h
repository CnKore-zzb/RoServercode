
#pragma once

#include "xSingleton.h"
#include <list>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "SceneItem.pb.h"
#include "RecordTrade.pb.h"
#include "SceneTrade.pb.h"
#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "ItemConfig.h"
#include "TableManager.h"
#include "MsgManager.h"
#include "MessageStat.h"
#include "xTime.h"
#include "MatchCCmd.pb.h"
#include "MatchSCmd.pb.h"
#include "SocialCmd.pb.h"

using namespace Cmd;

class MatchRoomMgr;


enum EMatchRetCode
{
  EMATCHRETCODE_OK          = 0,
  EMATCHRETCODE_IS_FIGHTING = 1,
  EMATCHRETCODE_IS_FULL     = 2,
};

class MatchRoom
{
public:
  MatchRoom(QWORD guid);
  virtual ~MatchRoom();

  virtual EMatchRetCode checkCanJoin() { return EMATCHRETCODE_OK; }
  virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev) { return false; }
  virtual bool delMember(QWORD charId) { return false; }
  virtual void toBriefData(Cmd::RoomBriefInfo* pRoomInfo);
  virtual void toDetailData(Cmd::RoomDetailInfo* pRoomInfo);
  virtual bool isEmpty() { return false; }
  virtual DWORD getMemberCount()const  { return 0; }
  virtual void setTeamCount(DWORD count) { m_dwMaxTeamCount = count; }
  virtual void onRaidSceneOpen(DWORD zoneid, DWORD sceneid) {};

  virtual void timeTick(DWORD curSec) {};
  QWORD getGuid() const { return m_qwGuid; }
  DWORD getRaidId() { return m_dwRaidId; }
  void setRaidId(DWORD dwRaid) { m_dwRaidId = dwRaid; }
  DWORD getMapId() { return m_dwMapId; }
  void setMapId(DWORD dwMapId) { m_dwMapId = dwMapId; }

  virtual EPvpType getType() = 0;
  virtual void onUserOnline(const SocialUser& rUser) {}
  virtual void onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId) {}
  virtual void onUserOffline(const SocialUser& rUser) {}
  virtual void onUserOfflinePvp(const SocialUser& rUser, DWORD oldZoneId){}
  void setState(Cmd::ERoomState state);
  ERoomState getState() { return m_state; }
  bool isFighting() { return m_state == EROOMSTATE_FIGHTING; }
  
  void setName(const string& name) { m_name = name; }
  const string& getName() { return m_name; }
  void setMatchRoomMgr(MatchRoomMgr* pMgr);
  MatchRoomMgr* getMatchRoomMgr() { return m_pRoomMgr; }
  virtual DWORD getOldZoneId(QWORD charId) { return 0; }

protected:
  QWORD m_qwGuid = 0;
  DWORD m_dwMapId = 0;          //mapraid表里的mapid
  DWORD m_dwRaidId = 0;

  Cmd::ERoomState m_state = EROOMSTATE_WAIT_JOIN;
  string m_name;
  DWORD m_dwZoneId = 0;
  MatchRoomMgr* m_pRoomMgr = nullptr;
  DWORD m_dwMaxTeamCount = 0;

public:
  DWORD m_dwPeopleLimit = 0;
};

class LLHMatchRoom:public MatchRoom
{
public:
  LLHMatchRoom(QWORD guid);
  virtual ~LLHMatchRoom();
  
  virtual EMatchRetCode checkCanJoin();
  virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  virtual bool delMember(QWORD charId);
  virtual void toBriefData(Cmd::RoomBriefInfo* pRoomInfo);
  virtual void toDetailData(Cmd::RoomDetailInfo* pRoomInfo);
  virtual bool isEmpty() { return m_dwCount == 0; }
  virtual void onUserOnline(const SocialUser& rUser);
  virtual void onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId){}
  virtual EPvpType getType() { return Cmd::EPVPTYPE_LLH; }
  virtual DWORD getMemberCount()const  { return m_dwCount; }
  void updateUserCount(DWORD count);
private:
  DWORD m_dwCount = 0;
};

typedef  std::map<QWORD, TeamMember> TMapRoomMember;
class RoomTeam
{
public:
  RoomTeam(DWORD zoneId, QWORD teamId, DWORD index, QWORD roomId, QWORD creator);
  ~RoomTeam();
  const TMapRoomMember& getTeamMembers() { return m_mapTeamMembers; }
  void addTeamMem(const TeamMember& info);
  void delTeamMem(QWORD charId);
  template<typename T> 
  void foreach(T func)
  {
    for (auto it = m_mapTeamMembers.begin(); it != m_mapTeamMembers.end(); ++it)
    {
      func(m_zoneId, it->first);
    }
  }

  void toMatchTeamData(MatchTeamData* pData);

  //获取队伍内成员个数
  DWORD getTeamMemCount() { return m_mapTeamMembers.size(); }
  DWORD getZoneId() const { return m_zoneId; }
  QWORD getTeamId() const { return m_teamId; }
  DWORD getIndex() const { return m_index; }
  void setIndex(DWORD index) { m_index = index; }
  //获取领队
  //TeamMember* getTeamLeader();
  TeamMember* getTeamMember(QWORD charId);
  void setName(const string& name) { m_name = name; }
  const string& getName() { return m_name; }
  bool isEmpty() const { return m_mapTeamMembers.empty(); }
  bool isTeamInPvp() const { return m_teamInPvp; }
  void setTeamInPvp() { m_teamInPvp = true; }
  QWORD getNewTeamId() { return m_newTeamId; }
  void setNewTeamId(QWORD leaderId, QWORD teamId) { m_teamCreator = leaderId; m_newTeamId = teamId; }
  QWORD getTeamCreator() { return m_teamCreator; }  
  bool sendAllUserToFight(DWORD toZoneId, DWORD raidId, QWORD roomId);
  void sendCmdToAllMember(void * buf, WORD len, QWORD except);
  void sendMsgToAllMember(DWORD msgid, const MsgParams& params = MsgParams());
  
  QWORD getOnlineLeader();
  QWORD getLeaderId() { return m_qwLeaderId; }
  void setLeader(QWORD charId);
  void setTempLeader(QWORD charId);
  void setOffline(QWORD charId, bool bOffline);
  bool isOffline(QWORD charId);
  bool isAllOffline();

  void setInitOk() { m_bInitOk = true; }
  bool initOk() { return m_bInitOk; }

private:
  DWORD m_zoneId = 0;  
  QWORD m_teamId = 0;
  DWORD m_index = 0;
  QWORD m_roomId = 0;
  QWORD m_newTeamId = 0;
  string m_name;
  TMapRoomMember m_mapTeamMembers;
  std::set<QWORD> m_setOffline;
  QWORD m_teamCreator = 0;
  bool m_teamInPvp = false;
  QWORD m_qwLeaderId = 0;       //队长
  QWORD m_qwTempLeaderId = 0;   //临时队长
  QWORD m_qwTeamCreator = 0;
  bool m_bInitOk = false; /*创建后, 收到teamserver发来的队伍信息*/
};

class TeamMatchRoom :public MatchRoom
{
public:
  TeamMatchRoom(QWORD guid);
  virtual ~TeamMatchRoom();

  virtual EMatchRetCode checkCanJoin();
  virtual EPvpType getType() { return Cmd::EPVPTYPE_SMZL; }
  virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev) { return false;}
  virtual bool delMember(QWORD charId) { return false;}
  //房间简单信息
  virtual void toBriefData(Cmd::RoomBriefInfo* pRoomInfo) { return; }
  //房间详细信息
  virtual void toDetailData(Cmd::RoomDetailInfo* pRoomInfo);
  virtual bool isEmpty() { return m_vecRoomTeam.empty(); }
  virtual void timeTick(DWORD curSec);
  virtual void onUserOnline(const SocialUser& rUser);
  virtual void onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId);
  virtual void onUserOffline(const SocialUser& rUser);
  virtual void onUserOfflinePvp(const SocialUser& rUser, DWORD oldZoneId);
  
  bool updateTeamMem(const MatchTeamMemUpdateInfo& rev);
  bool updateTeamMemData(const MatchTeamMemDataUpdateInfo& rev);
  void updateOneData(TeamMember* pTeamMember, EMemberData eType, QWORD qwValue);
  RoomTeam* getTeam(DWORD zoneId, QWORD teamId);
  RoomTeam* getTeamByCharId(DWORD zoneId, QWORD charId);
  //RoomTeam* getTeamByCharId(QWORD charId);
  RoomTeam* getTeamByNewTeamId(QWORD teamId);
  virtual void onOneTeamOk(RoomTeam* pTeam);
  virtual void onOneTeamDel(RoomTeam* pTeam) {}
  //新队伍创建好
  virtual void onNewTeamOk(RoomTeam* pTeam) {}

  virtual void setTeamCount(DWORD count);
  DWORD getIndex();
  void lockIndex(DWORD i);
  void unlockIndex(DWORD i);

  virtual void onTeamAddMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId) {}
  virtual void onTeamDelMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId);

  //去新线创建队伍
  bool createNewTeam(RoomTeam* pTeam);
  bool applyNewTeam(RoomTeam* pTeam);
  //创建队伍返回
  bool createNewTeamRes(DWORD zoneId, QWORD oldTeamId, QWORD leaderId, QWORD newTeamId);
  //房主拒绝挑战踢出挑战者队伍
  bool kickTeam(RoomTeam* pTeam, bool bDelTeam=true);
  virtual bool reset();
  //队伍人数是否够
  bool checkIsFull();
  //是否可以传送玩家进入战斗
  void ntfRoomState();
  void sendCmdToAllMember(void * buf, WORD len);
  void sendCmdToAllMember(void * buf, WORD len, const TSetDWORD&exceptSet);
  void kickPvpTeamUser(RoomTeam* pTeam, QWORD charId);
  void kickOldTeamUser(Cmd::KickTeamMatchSCmd& rev);

  void sendRoomStat();
  void getAllUsers(TSetQWORD& userids);

  virtual void v_timeTick(DWORD curSec)=0;

protected:
  std::vector<RoomTeam> m_vecRoomTeam;
  std::set<DWORD/*index*/> m_setIndex;   //1 2 3 
};

class SMZLMatchRoom :public TeamMatchRoom
{
public:
  SMZLMatchRoom(QWORD guid);
  virtual ~SMZLMatchRoom();

  virtual EPvpType getType() { return Cmd::EPVPTYPE_SMZL; }
  virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  virtual bool delMember(QWORD charId) { return false; }
  virtual void toBriefData(Cmd::RoomBriefInfo* pRoomInfo);
  virtual void v_timeTick(DWORD curSec);
  //一个队伍的数据全部同步玩家
  virtual void onOneTeamOk(RoomTeam* pTeam);
  //一个队伍玩家全部离开
  virtual void onOneTeamDel(RoomTeam* pTeam);
  virtual void onNewTeamOk(RoomTeam* pTeam);

  virtual void onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId);
  virtual bool reset();
  virtual void onTeamAddMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId);

  //向房主挑战
  bool challenge(); 
  bool processChallengeRes(QWORD charId, EMatchReply reply);
  //房主拒绝挑战踢出挑战者队伍
  void sendConfirm();
  void sendComfirmRes(DWORD zoneId, QWORD charId, QWORD teamId);
  void startCountDown(DWORD zoneId, QWORD charId);
  void stopCountDown();
private:
  DWORD m_dwCountEndTime = 0;
  DWORD m_challengeReplyTime = 0;     //发起挑战回复截止时间
  DWORD m_confiremReplyTime = 0;      //进入房间确认截止时间
  DWORD m_dwOkTeamCount = 0;
  
  QWORD m_qwCountDownCharid = 0;
  DWORD m_dwCountDownZoneId = 0;
};

class HLJSMatchRoom :public TeamMatchRoom
{
public:
  HLJSMatchRoom(QWORD guid);
  virtual ~HLJSMatchRoom();

  virtual EPvpType getType() { return Cmd::EPVPTYPE_HLJS; }
  virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  virtual bool delMember(QWORD charId) { return false; }
  virtual void toBriefData(Cmd::RoomBriefInfo* pRoomInfo);
  virtual void v_timeTick(DWORD curSec);

  //一个队伍的数据全部同步玩家
  virtual void onOneTeamOk(RoomTeam* pTeam);
  //一个队伍玩家全部离开
  virtual void onOneTeamDel(RoomTeam* pTeam);
  virtual void onNewTeamOk(RoomTeam* pTeam);
  virtual void onTeamAddMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId);
  virtual void onTeamDelMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId);

  virtual void onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId);
  virtual bool reset();

  void startCountdown();
  void stopCountdown();  
private: 
  DWORD m_dwCountEndTime;
};

class PollyMatchRoom :public MatchRoom
{
public:
  PollyMatchRoom(QWORD guid);
  virtual ~PollyMatchRoom();

  virtual EPvpType getType() { return Cmd::EPVPTYPE_POLLY; }

};

class MvpMatchRoom : public TeamMatchRoom
{
  public:
    MvpMatchRoom(QWORD guid, MatchRoomMgr* pRoomMgr);
    virtual ~MvpMatchRoom();

    virtual EPvpType getType() { return EPVPTYPE_MVP; }
    virtual void v_timeTick(DWORD curSec);

    virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
    virtual void onNewTeamOk(RoomTeam* pTeam);

    virtual void onOneTeamOk(RoomTeam* pTeam); // join -> teamserver -> syncteamdata,
    virtual void onOneTeamDel(RoomTeam* pTeam);

    virtual void onTeamAddMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId);
    virtual void onTeamDelMember(RoomTeam* pTeam, QWORD charId, DWORD zoneId);

    virtual void onRaidSceneOpen(DWORD zoneid, DWORD sceneid);
    //virtual void onUserOnline(const SocialUser& rUser);
    //virtual void onUserOnlinePvp(const SocialUser& rUser, DWORD oldZoneId) {}
  public:
    void setBeginMatchTime(DWORD time) { m_dwBeginMatchTime = time; }
    bool checkCanStart(DWORD time);

    DWORD getZoneID() { return m_dwZoneID; }
    void setZoneID(DWORD zone) { m_dwZoneID = zone; }

    void onBattleClose();
    bool userCanEnter(QWORD userid) const { return m_setRoomUsers.find(userid) != m_setRoomUsers.end(); }
    void clearTeamPunishCD(QWORD newteamid);
  private:
    void onStart();
  private:
    DWORD m_dwBeginMatchTime = 0;
    //DWORD m_dwRaidID = 0; /*每个房间对应一个随机的副本ID*/
    DWORD m_dwZoneID = 0; /*匹配成功后, 当前各线负载, 选择一条负载低得线创建*/
    DWORD m_dwPunishTime = 0; /*当前房间进入后的惩罚时间*/
    map<QWORD, DWORD> m_mapTeam2MatchTimeOut; /*记录队伍匹配超时时间戳*/
    TSetQWORD m_setRoomUsers;/*记录匹配成功时的玩家,后续入队不可进入房间*/
    std::set<RoomTeam*> m_setKickTeams; /*待踢出的队伍*/
};

class SuperGvgMatchRoom : public TeamMatchRoom
{
  public:
    SuperGvgMatchRoom(QWORD guid, DWORD zoneid);
    virtual ~SuperGvgMatchRoom();

    virtual EPvpType getType() { return Cmd::EPVPTYPE_SUGVG; }
    virtual void v_timeTick(DWORD curSec){};

    void setGuild(const TVecQWORD& ids);
    DWORD getZoneID() const { return m_dwZoneID; }

    void setLevel(DWORD level) { m_dwLevel = level; }
    DWORD getLevel() const { return m_dwLevel; }

    const TSetQWORD& getGuildIDs() { return m_setGuildIDs; }

    virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
    virtual void onNewTeamOk(RoomTeam* pTeam);

    virtual void onOneTeamOk(RoomTeam* pTeam); // join -> teamserver -> syncteamdata,
  private:
    DWORD m_dwZoneID = 0;/*匹配成功后, 当前各线负载, 选择一条负载低得线创建*/
    DWORD m_dwLevel = 0; /*段位，3,2,1:黄金,白银,青铜*/
    TSetQWORD m_setGuildIDs;/*当前房间参与的公会*/
};

class TeamPwsMatchRoom : public TeamMatchRoom
{
  struct SPwsNewTeamInfo
  {
    QWORD qwTeamID = 0;
    QWORD qwLeaderID = 0;
    DWORD dwColor = 0;
    TSetQWORD setMems;
  };

  public:
    TeamPwsMatchRoom(QWORD guid, DWORD zoneid, EPvpType etype);
    virtual ~TeamPwsMatchRoom();

    virtual EPvpType getType() { return m_eType; }
    virtual void v_timeTick(DWORD curSec){};

    virtual bool addMember(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev){ return true; }
    virtual void onNewTeamOk(RoomTeam* pTeam);

    virtual void onOneTeamOk(RoomTeam* pTeam); // join -> teamserver -> syncteamdata,
    virtual bool reset();
  public:
    // zoneid 队长原线, teamid, 队长原队id
    void addTeamInfo(QWORD leaderid, const TSetQWORD& members, DWORD zoneid);
    bool addMember(QWORD charId, DWORD zoneId, QWORD teamId);
    void formatTeamInfo(SyncRoomSceneMatchSCmd& cmd);
    DWORD getZoneID() { return m_dwZoneID; }
    void getAllUsers(TSetQWORD& users);
  private:
    DWORD m_dwZoneID = 0;
    EPvpType m_eType = EPVPTYPE_TEAMPWS;
    //std::map<QWORD, TSetQWORD> m_mapLeader2TeamInfo; // leader->team users, users包括组合队伍成员
    std::map<QWORD, map<DWORD,TSetQWORD>> m_mapLeader2TeamInfo;
    std::map<QWORD, SPwsNewTeamInfo> m_mapTeamID2NewTeamInfo; //新队伍id->(颜色,玩家列表)
    DWORD m_dwOkTeamCount = 0;
};

