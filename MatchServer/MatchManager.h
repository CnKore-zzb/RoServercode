
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
#include "MatchRoomMgr.h"
#include "SocialCmd.pb.h"

using namespace Cmd;
struct MatchUser
{
  QWORD m_qwCharId = 0;
  DWORD m_dwCurZoneId = 0; //当前线
  DWORD m_dwPollyZoneId = 0;
};


class MatchManager:public xSingleton<MatchManager>
{
public:
  MatchManager();
  ~MatchManager();

  void init();
  bool loadConfig();

  bool doUserCmd(QWORD charId, DWORD zoneId, const BYTE* buf, WORD len);
  bool doServerCmd(QWORD charId, std::string name, DWORD zoneId, const BYTE* buf, WORD len);
  
  void timeTick(DWORD curSec);
  void onUserOnline(const SocialUser& rUser);
  void onUserOffline(const SocialUser& rUser);
  void onUserOnlinePvp(const SocialUser& rUser);
  void onUserOfflinePvp(const SocialUser& rUser);


  bool reqMyRoom(QWORD charId, DWORD zoneId, ReqMyRoomMatchCCmd& rev);
  bool reqRoomList(QWORD charId, DWORD zoneId, ReqRoomListCCmd& rev);
  bool reqRoomDetail(QWORD charId, DWORD zoneId, ReqRoomDetailCCmd& rev);
  bool joinRoom(QWORD charId, DWORD zoneId, JoinRoomCCmd& rev);
  bool leaveRoom(QWORD charId, DWORD zoneId, LeaveRoomCCmd& rev);  
  bool joinFighting(QWORD charId, DWORD zoneId, JoinFightingCCmd& rev);
  bool onLeaveRoom(QWORD charId, DWORD originZoneId);

  MatchRoom* getRoom(Cmd::EPvpType type, QWORD guid);
  MatchRoom* getRoomByRoomId(QWORD roomId);  
  MatchRoom* getRoomByCharId(QWORD charId, DWORD zoneId);
  QWORD getRoomId(QWORD charId, DWORD zoneId);
  void addRoomUser(QWORD charId, DWORD zoneId, QWORD roomId);
  void delRoomUser(QWORD charId, DWORD zoneId, QWORD roomId);
  bool isInRoom(QWORD charId, DWORD zoneId);
  bool isInMatching(QWORD charid);

  //生成房间索引
  QWORD generateGuid();  
  //把玩家传送到战斗场景
  bool sendPlayerToFighting(DWORD fromZoneid, DWORD toZoneId, QWORD charId, DWORD mapId, QWORD roomId, DWORD colorIndex=0);
  void sendSysMsg(QWORD charId, DWORD zoneId, DWORD msgid, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD);
  bool gmRestUserRoom(QWORD charId, DWORD zoneId);
  
  bool isPollyActivityOpen() { return m_bPollyActivity; }
  void openPollyActivity();
  void closePollyActivity();
  MatchUser* getMatchUser(QWORD charId) { auto it = m_mapUser.find(charId); return (it == m_mapUser.end()) ? nullptr : &(it->second); }

  bool isMvpBattleOpen() { return m_bMvpBattleOpen; }
  void openMvpBattle();
  void closeMvpBattle();

  void joinSuperGvg(const JoinSuperGvgMatchSCmd& rev);

  // 组队排位赛
public:
  void openTeamPws(bool bRestart = false);
  void closeTeamPws();
  void joinTeamPws(JoinTeamPwsMatchSCmd& rev);
  void leaveTeamPws(DWORD zoneid, QWORD teamid, EPvpType etype);
  void userLeaveRaid(QWORD charid, EPvpType etype); // 强行退出
  void userBeReady(EPvpType etype, QWORD charid);

public:

  MatchRoomMgr* getRoomMgr(Cmd::EPvpType eTpe);
  bool chekIsOpen(QWORD charId, DWORD zoneId, Cmd::EPvpType eType);

private:
  TeamPwsMatchRoomMgr* getTeamPwsRoomMgr(EPvpType etype);
private:
  xTimer m_oneSecTimer;
  xTimer m_tenSecTimer;
  QWORD m_qwGuidIndex = 0;
  LLHMatchRoomMgr* m_llhRoomMgr = nullptr;
  SMZLMatchRoomMgr* m_smzlRoomMgr = nullptr;
  HLJSMatchRoomMgr* m_hljsRoomMgr = nullptr;
  PollyMatchRoomMgr* m_pollyRoomMgr = nullptr;
  MvpMatchRoomMgr* m_pMvpRoomMgr = nullptr;
  SuperGvgMatchRoomMgr* m_pSuGvgRoomMgr = nullptr;
  TutorMatchRoomMgr* m_pTutorRoomMgr = nullptr;
  TeamPwsMatchRoomMgr* m_pTeamPwsMatchRoomMgr = nullptr;
  TeamPwsMatchRoomMgr* m_pTeamPwsRelaxMgr = nullptr; /*休闲模式*/

  std::map<QWORD/*charid*/, std::map<DWORD/*zone*/, QWORD/*roomid*/>> m_mapUserRoom;
  std::map<QWORD/*charid*/, DWORD /*zone*/> m_mapLastZoneId;      //最近的一次非pvp线

  //new
  bool m_bPollyActivity = false;
  bool m_bMvpBattleOpen = false;
  bool m_bTeamPwsOpen = false;
  
  std::map<QWORD/*charid*/, MatchUser> m_mapUser;
};
