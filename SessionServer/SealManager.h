#pragma once

#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include "xDefine.h"
#include "xPool.h"
#include "ProtoCommon.pb.h"
#include "SessionCmd.pb.h"
#include "TableStruct.h"
#include "SessionTeam.pb.h"
#include <vector>
#include <string>
#include <set>

using namespace Cmd;
using std::vector;
using std::map;

class SessionUser;
struct SSealData2
{
  // team data
  QWORD qwTeamID = 0;
  QWORD qwLeaderID = 0;
  TSetQWORD setTeamerIDs;

  // db data
  DWORD dwConfigID = 0;
  ScenePos xSealPos;
  QWORD qwLastUserOnlineTime = 0;

  // temp data
  DWORD dwBeginTime = 0;
  DWORD dwNextCreateTime = 0;
  bool bLeaderNoSeal = false;
  DWORD dwSealingMapID = 0;

  // 清空标记
  bool bNeedClear = false;

  // 存储标记
  bool bSaveUpdate = false;

  // 重新启动时, 发送场景创建标志
  bool bHaveSendScene = true;

  void loadTeamInfo(const SetTeamSeal& data);
  void sendSealInfoToUser(SessionUser* pUser);
  void update(DWORD curTime);
  void delSealToScene();
  void clear() { dwConfigID = 0; dwBeginTime = 0; dwNextCreateTime = 0; bLeaderNoSeal = false; bNeedClear = true; }

  void fromData(const TeamSealData& data);
  void toData(TeamSealData* pData);
  void saveData();

  SSealData2() {}
};
typedef map<QWORD, SSealData2> TMapTeam2SealData;

class SealManager : public xSingleton<SealManager>
{
  friend class xSingleton<SealManager>;
  private:
    SealManager();
  public:
    virtual ~SealManager();

  public:
    bool loadDataFromDB();

    void setSeal(SetTeamSeal& scmd);

    void addMember(QWORD teamid, QWORD userid);
    void removeMember(QWORD teamid, QWORD userid);
    void changeLeader(QWORD teamid, QWORD newleader);
    void onUserOnline(SessionUser* pUser);
    void onUserOffline(SessionUser* pUser);

    void timer(DWORD curTime);
  private:
    TMapTeam2SealData m_mapTeam2SealData;
};

