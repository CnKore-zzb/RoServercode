
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
#include "MatchRoom.h"
#include "xTools.h"

using std::string;
class MatchRoomMgr
{
public:
  MatchRoomMgr();
  virtual ~MatchRoomMgr();

  virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev) = 0;
  bool reqRoomDetail(QWORD charId, DWORD zoneId, ReqRoomDetailCCmd& rev);
  virtual MatchRoom* createRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev) { return nullptr; }
  virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev) { return false; }
  virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev) { return false; }
  virtual bool joinFighting(QWORD charId, DWORD zoneId, JoinFightingCCmd& rev) { return false; }
  virtual bool onLeaveRoom(QWORD charId, MatchRoom* pRoom) { return false; }
  virtual bool kickTeam(TeamMatchRoom* pRoom, RoomTeam* pTeam) { return false; }
  virtual bool closeRoom(TeamMatchRoom* pRoom) { return true; } 
  virtual bool closeRoom(MatchRoom* pRoom) { return true; }
  virtual QWORD getAvailableRoom() ;
  virtual void setRoomFighting(MatchRoom* pRoom) {}
  virtual void onRaidSceneOpen(MatchRoom* pRoom, DWORD zoneid, DWORD sceneid) {};
  
  virtual MatchRoom* getRoomByid(QWORD guid) = 0;
  virtual void updateRank() {}

protected:
};


struct CmpByPeople {   //从人数大到小排序,roomid 从小到大
  bool operator()(const MatchRoom* lhs, const MatchRoom* rhs) {
    if (lhs->getMemberCount() > rhs->getMemberCount())
    {
      return true;
    }
    else if (lhs->getMemberCount() == rhs->getMemberCount())
    {
      return lhs->getGuid() < rhs->getGuid();
    }
    return false;
  }
};

class LLHMatchRoomMgr :public MatchRoomMgr
{
public:
  LLHMatchRoomMgr();
  ~LLHMatchRoomMgr();

  virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev);
  virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  virtual MatchRoom* getRoomByid(QWORD guid);

  virtual QWORD getAvailableRoom();
  
  //更新排序
  void updateRank();
private:  
  std::vector<MatchRoom*> m_vecRooms;  
  DWORD m_maxRoomCount = 60;
};

class TeamMatchRoomMgr :public MatchRoomMgr
{
public:
  TeamMatchRoomMgr();
  ~TeamMatchRoomMgr();

  void oneSecTick(DWORD curTime);

  virtual MatchRoom* getRoomByid(QWORD guid);
  virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);
  virtual bool kickTeam(TeamMatchRoom* pRoom, RoomTeam* pTeam);
  virtual bool onLeaveRoom(QWORD charId, MatchRoom* pRoom);
  virtual bool joinFighting(QWORD charId, DWORD zoneId, JoinFightingCCmd& rev);
  virtual bool closeRoom(TeamMatchRoom* pRoom);
  virtual void setRoomFighting(MatchRoom* pRoom);

protected:
  std::map<QWORD/*guid*/, MatchRoom*> m_mapRooms;
  std::map<QWORD/*guid*/, MatchRoom*> m_mapCanJoinRooms;  //可加入的房间
  std::set<string> m_setName;     //房间名字列表 沙漠之狼用
};

class SMZLMatchRoomMgr :public TeamMatchRoomMgr
{
public:
  SMZLMatchRoomMgr();
  ~SMZLMatchRoomMgr();

  virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev);
  virtual MatchRoom* createRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  virtual bool closeRoom(TeamMatchRoom* pRoom);
  
};

class HLJSMatchRoomMgr :public TeamMatchRoomMgr
{
public:
  HLJSMatchRoomMgr();
  ~HLJSMatchRoomMgr();

  virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev);
  virtual bool closeRoom(TeamMatchRoom* pRoom);
private:
  MatchRoom* createEmptyRoom();
};

class PollyMatchRoomMgr :public MatchRoomMgr
{
public:
  PollyMatchRoomMgr();
  ~PollyMatchRoomMgr();

  virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);
  virtual MatchRoom* getRoomByid(QWORD guid);
  virtual bool closeRoom(MatchRoom* pRoom);
  
  bool checkIsInQueue(QWORD charId);
  bool popMatchQueue(QWORD charId);
  void popRetryQueue(QWORD charId);
  void pushRetryQueue(QWORD charId);
  void clearQueue();

private:
  bool pushMatchQueue(QWORD charId);  
  bool createNewRoom();

  virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev) { return false; }

private:
  
  std::vector<MatchRoom*> m_vecRooms; 
  std::list<QWORD> m_matchQueue;    //匹配队列
  std::map<QWORD/*charid*/, DWORD/*time*/> m_retryQueue;  //掉线重试队列
};

// mvp 竞争战
class MvpMatchRoomMgr : public TeamMatchRoomMgr
{
  public:
    MvpMatchRoomMgr();
    virtual ~MvpMatchRoomMgr();
    virtual bool kickTeam(TeamMatchRoom* pRoom, RoomTeam* pTeam);
    virtual bool joinFighting(QWORD charId, DWORD zoneId, JoinFightingCCmd& rev);
    virtual void setRoomFighting(MatchRoom* pRoom);
    virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev);

    virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
    virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);
    virtual bool closeRoom(TeamMatchRoom* pRoom);
  public:
    void onMvpBattleClose();
    void addUserPunishTime(QWORD charid, DWORD time);
    void clearUserPunishTime(QWORD charid);
    bool userCanJoin(QWORD charid, DWORD time);
  private:
    QWORD m_qwCurMatchRoomID = 0; // 记录当前正在匹配中的房间
    std::map<QWORD, DWORD> m_mapUser2ReMatchTime; /*报名后一段时间不能再报名*/
};

struct SSuGvgMatchInfo
{
  QWORD qwGuildID = 0;
  DWORD dwZoneID = 0;
  DWORD dwFireCount = 0;
  DWORD dwFireScore = 0;
  string strGuildName;
  string strGuildIcon;

  //match success 后记录所在的房间
  QWORD qwRoomID = 0;
  DWORD dwColorIndex = 0;
};
typedef std::map<QWORD, SSuGvgMatchInfo> TMapSuGvgMatchInfo;

const DWORD SUPERGVG_MATCH_TIME = 30;
class SuperGvgMatchRoomMgr : public MatchRoomMgr
{
  enum EGvgMatchState
  {
    EGVGMATCHSTATE_MIN = 0,          /*未开启状态*/
    EGVGMATCHSTATE_WAITJOIN = 1,     /*等待匹配*/
    EGVGMATCHSTATE_MATCHSUCCESS = 2, /*匹配成功, 等待开启*/
    EGVGMATCHSTATE_PREFIGHTING = 3,     /*允许进入, 不允许战斗*/
    EGVGMATCHSTATE_FIGHTING = 4,     /*战斗中*/
  };
  struct SWaitJoinInfo
  {
    DWORD dwZoneID = 0;
    DWORD dwDestZoneID = 0;
    QWORD qwCharID = 0;
    DWORD dwRaidID = 0;
    DWORD dwRoomID = 0;
    DWORD dwColor = 0;
  };
  public:
    SuperGvgMatchRoomMgr();
    virtual ~SuperGvgMatchRoomMgr();

    virtual MatchRoom* getRoomByid(QWORD guid);
    virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
    virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);
    virtual bool closeRoom(MatchRoom* pRoom);
    virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev) { return false; }
    virtual void onRaidSceneOpen(MatchRoom* pRoom, DWORD zoneid, DWORD sceneid);

  public:
    void oneSecTick(DWORD curTime);
    void joinGuild(const JoinSuperGvgMatchSCmd& rev);
  private:
    bool match();
    void matchError();
    void reset();
  private:
    TMapSuGvgMatchInfo m_mapJoinGuilds;
    DWORD m_dwBeginJoinTime = 0;
    DWORD m_dwTimeTick = 0;
    EGvgMatchState m_eState = EGVGMATCHSTATE_MIN;
    std::map<QWORD, SuperGvgMatchRoom*> m_mapRooms;
    std::map<QWORD, QWORD> m_mapUser2Guild;
    std::map<DWORD, std::list<SWaitJoinInfo>> m_mapWaitUsers; // <destzoneid, info>
};

// tutor match
struct STutorMatcher
{
  QWORD charid = 0;
  DWORD zoneid = 0;

  bool bFindTutor = false;

  EGender eDestGender = EGENDER_MIN;
  EGender eSelfGender = EGENDER_MIN;

  map<EUserDataType, UserData> mapDatas;
  TSetQWORD setBlackIDs;

  void fromData(const TutorMatcher& rMatcher);
  void toData(TutorMatcher* pMatcher);

  const string& getName() const;
};
typedef map<QWORD, STutorMatcher> TMapMatcher;

struct STutorZoneGroup
{
  TSetQWORD setMaleIDs;
  TSetQWORD setFemaleIDs;
};
typedef map<DWORD, STutorZoneGroup> TMapTutorZoneGroup;

enum ETutorProcess
{
  ETUTORPROCESS_RESPONSE = 1,
  ETUTORPROCESS_TUTOR_APPLY = 2,
  ETUTORPROCESS_STUDENT_AGREE = 3,
  ETUTORPROCESS_END = 4,
};
struct STutorMatching
{
  QWORD tutorid = 0;
  QWORD studentid = 0;

  DWORD dwTick = 0;

  ETutorMatch eTutorResponse = ETUTORMATCH_MIN;
  ETutorMatch eStudentResponse = ETUTORMATCH_MIN;

  ETutorProcess eProcess = ETUTORPROCESS_RESPONSE;
};
class TutorMatchRoomMgr : public MatchRoomMgr
{
  public:
    TutorMatchRoomMgr();
    virtual ~TutorMatchRoomMgr();

    virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev) { return false; }
    virtual MatchRoom* getRoomByid(QWORD guid) { return nullptr; }

    virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
    virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);

    STutorMatcher* getMatcher(QWORD charid);

    EError response(QWORD charid, ETutorMatch reply);
    void timeTick(DWORD curTime);
  private:
    void processMatch();
    void processOverTime();
    void showlog(DWORD dwZoneID);
    void matchResultNtf(QWORD charid1, QWORD charid2, ETutorMatch eResult);
  public:
    bool process(QWORD charid, bool bResult);
  private:
    bool isMatch(QWORD charid);
    bool matching(STutorZoneGroup& rGroup, STutorMatcher* pMatcher);

    STutorMatcher* getMatcherInfo(QWORD charid);
  private:
    std::list<STutorMatching> m_listMatching;

    TMapMatcher m_mapMatcher;
    TMapTutorZoneGroup m_mapTutorGroup;
    TMapTutorZoneGroup m_mapStudentGroup;
    TSetQWORD m_setMatchIDs;
    TSetQWORD m_setMatchDoneIDs;

    DWORD m_dwOverTimeTick = 0;
};


typedef std::pair<DWORD, QWORD> TeamKey; // zoneid,teamid

/*报名队伍信息, 包括不完整队伍*/
struct SJoinTeamInfo
{
  QWORD qwTeamID = 0; // 原线队伍id
  QWORD qwLeaderid = 0; // 队长
  DWORD dwZoneID = 0; // 所在线
  DWORD dwAveScore = 0; // 队伍均分
  TSetQWORD setUsers; // 所有成员

  DWORD dwJoinTime = 0;

  void fromData(JoinTeamPwsMatchSCmd& cmd)
  {
    qwTeamID = cmd.teamid();
    qwLeaderid = cmd.leaderid();
    dwZoneID = cmd.zoneid();
    dwAveScore = cmd.avescore();
    for (int i = 0; i < cmd.members_size(); ++i)
      setUsers.insert(cmd.members(i));
    dwJoinTime = now();
  }
  void toData(JoinTeamPwsMatchSCmd& cmd)
  {
    cmd.Clear();
    cmd.set_teamid(qwTeamID);
    cmd.set_leaderid(qwLeaderid);
    cmd.set_zoneid(dwZoneID);
    cmd.set_avescore(dwAveScore);
    for (auto &s : setUsers)
      cmd.add_members(s);
  }
  virtual void notifyTeamExit();
  virtual void notifyMemsMatchStatus(bool match, EPvpType e);
  virtual void sendMsgToAllMember(DWORD msgid, const MsgParams& param = MsgParams());
  virtual void sendCmdToAllMember(void* buf, WORD len);
};

/*报名队伍信息, 完整队伍*/
struct SFullTeamInfo : public SJoinTeamInfo
{
  bool bMatchBStatus = false; // 已经入b池匹配状态
  std::list<SJoinTeamInfo> listAllTeams; //不为空, 是组合队伍
  virtual void notifyTeamExit();
  virtual void notifyMemsMatchStatus(bool match, EPvpType e);
  virtual void sendMsgToAllMember(DWORD msgid, const MsgParams& param = MsgParams());
  virtual void sendCmdToAllMember(void* buf, WORD len);
};

// 准备中的队伍信息
struct SPrepareTeamInfo
{
  QWORD qwGuid = 0; // 唯一id(房间)

  DWORD dwExpireTime = 0; // 超时时间
  std::list<SFullTeamInfo> listTeams; // 队伍列表
  TSetQWORD setReadyUsers; // 完成准备的玩家

  template<class T> void foreach(T func)
  {
    for (auto &l : listTeams)
    {
      if (l.listAllTeams.empty())
        func(l);
      else
      {
        for (auto &t : l.listAllTeams)
          func(t);
      }
    }
  }

  void sendCmdToAllUsers(void* buf, WORD len);
};
typedef std::map<QWORD, SPrepareTeamInfo> TMapPrepareTeamInfo;

const vector<vector<TVecDWORD>> VECTOR_SUM_ARRAY = {
  {{}},
  {{1}},
  {{2},{1,1}},
  {{3},{2,1},{1,1,1}},
  {{4},{2,2},{3,1},{2,1,1},{1,1,1,1}},
  {{5},{4,1},{3,2},{3,1,1},{2,2,1},{2,1,1,1},{1,1,1,1,1}}
};

// 组队排位赛
class TeamPwsMatchRoomMgr : public MatchRoomMgr
{
  public:
    TeamPwsMatchRoomMgr(bool relax = false, DWORD raidid = 0);
    virtual ~TeamPwsMatchRoomMgr();

    virtual MatchRoom* getRoomByid(QWORD guid);
    virtual bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev) { return true; }
    virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);
    virtual bool closeRoom(MatchRoom* pRoom);
    virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev) { return false; }
    virtual void onRaidSceneOpen(MatchRoom* pRoom, DWORD zoneid, DWORD sceneid);

  public:
    virtual void oneSecTick(DWORD curTime);

   public:
     const static vector<vector<map<DWORD,DWORD>>> VECTOR_FOR_MATCH;
   public:
     static vector<vector<map<DWORD,DWORD>>> createVecMap()
     {
       vector<vector<map<DWORD,DWORD>>> vecmap;
       for (auto &v : VECTOR_SUM_ARRAY)
       {
         vector<map<DWORD,DWORD>> vec;
 
         for (auto &v1 : v)
         {
           map<DWORD,DWORD> m;
           for (auto &value : v1)
           {
             m[value] ++;
           }
           vec.push_back(m);
         }
         vecmap.push_back(vec);
       }
       return vecmap;
     }

  public:
    void onBattleOpen();
    virtual bool teamJoin(JoinTeamPwsMatchSCmd& cmd, bool bRetry = false/*准备失败的队伍*/);
    virtual bool teamLeave(DWORD zoneid, QWORD teamid);
    virtual bool userBeReady(QWORD charid);
    virtual bool isInMatching(QWORD charid);
    void punishUserLeaveTeam(QWORD charid);
    bool isTeamInMatch(DWORD zoneid, QWORD teamid);
    bool isUserInPrepare(QWORD charid);
  protected:
    const map<DWORD,DWORD>& findMatchInfo(DWORD needsize);
    void resetMatchInfo();
    void joinBPool(const TeamKey& key);
    void matchBPool(DWORD group1, DWORD group2);
    void enterPrepare(const TeamKey& key1, const TeamKey& key2);
    void checkPrepareTimeOut(DWORD curTime);
    void checkMatchTimeOut(DWORD curTime);
    bool doTeamExit(DWORD zoneid, QWORD teamid);
  protected:
    bool m_bRelaxMode = false;
    bool m_bFireOpen = false;
    DWORD m_dwRelaxRaidID = 0; // 指定的休闲模式副本id
    EPvpType m_ePvpType = EPVPTYPE_TEAMPWS;
    std::map<TeamKey, SJoinTeamInfo> m_mapMissingTeams; // 所有不完整的队伍
    std::map<TeamKey, SFullTeamInfo> m_mapFullTeams; // 完整的队伍
    std::map<DWORD, std::list<TeamKey>> m_mapNum2MissingTeams; // 队伍玩家数->不完整队伍列表
    vector<std::list<TeamKey>> m_vecAMatchPool; // A匹配池
    vector<pair<DWORD,std::list<TeamKey>>> m_vecBMatchPool; // B匹配池
    TMapPrepareTeamInfo m_mapPrepareTeamInfo; // 等待准备进入的列表
    std::map<QWORD, QWORD> m_mapUserID2PrepareGuid; // 玩家->准备房间guid
  protected:
    // 房间列表
    std::map<QWORD, TeamPwsMatchRoom*> m_mapRooms;
    std::map<QWORD, DWORD> m_mapUser2PunishCD; // 惩罚cd
    TSetQWORD m_setRoomUsers; // 房间中的玩家
};

// 休闲模式
class TeamPwsRelaxMatchRoomMgr : public TeamPwsMatchRoomMgr
{
  public:
    TeamPwsRelaxMatchRoomMgr();
    virtual ~TeamPwsRelaxMatchRoomMgr();

    virtual MatchRoom* getRoomByid(QWORD guid);
    virtual bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);
    virtual bool closeRoom(MatchRoom* pRoom);
    virtual bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev) { return false; }
    virtual void onRaidSceneOpen(MatchRoom* pRoom, DWORD zoneid, DWORD sceneid);

  public:
    virtual void oneSecTick(DWORD curTime);
  public:
    virtual bool teamJoin(JoinTeamPwsMatchSCmd& cmd, bool bRetry = false/*准备失败的队伍*/);
    virtual bool teamLeave(DWORD zoneid, QWORD teamid);
    virtual bool userBeReady(QWORD charid);
    virtual bool isInMatching(QWORD charid);
  private:
    map<DWORD, TeamPwsMatchRoomMgr*> m_mapRaid2RelaxMgrs;
};

