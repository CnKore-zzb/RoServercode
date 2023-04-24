#pragma once

#include "xSingleton.h"
#include "ProtoCommon.pb.h"
#include "xPos.h"

using namespace Cmd;

class xSceneEntryDynamic;
class SceneUser;
class SceneNpc;
class Scene;

using namespace std;

struct UserStageInfo
{
  TSetQWORD setUsers;
  map<EUserDataType, DWORD> mapAttr;
  DWORD dwTime = 0;
};

class DressUpStageMgr: public xSingleton<DressUpStageMgr>
{
  friend class xSingleton<DressUpStageMgr>;
  private:
    DressUpStageMgr();
  public:
    virtual ~DressUpStageMgr();

    void init();
    void initAppearance(SceneNpc* pNpc);
    void timer(DWORD curTime);
    void sendStageInfo(SceneUser* pUser, DWORD stageid);
    void refreshStageInfo(DWORD stageid);
    void requestAddLine(SceneUser* pUser, DWORD stageid, DWORD mode);
    void addLineUp(SceneUser* pUser, SceneUser* pMate, DWORD stageid);
    bool showUsers(DWORD stageid, UserStageInfo* pInfo);
    bool addToWaitUser(DWORD stageid, UserStageInfo* pInfo);
    void checkLineUp(DWORD stageid);
    void leaveDressStage(SceneUser* pUser);
    void onMatesLeave(SceneUser* pUser, QWORD userid);
    void changeStageAppearance(SceneUser* pUser, DWORD stageid, EUserDataType eType, DWORD value, SceneNpc* pNpc);
    void changeMyStage(SceneUser* pUser, DWORD stageid, EUserDataType eType, DWORD value, SceneNpc* pNpc);
    UserStageInfo* getUserStageInfo(SceneUser* pUser);
    SceneNpc* getStageNpc(DWORD stageid);
    bool isInStageRange(xSceneEntryDynamic* pEntry, xPos pos, DWORD npcid, Scene* pScene) const;
    bool checkStageDistance(const xPos& pos, DWORD distance, Scene* pScene);

  private:
    TSetDWORD m_setStages;
    map<DWORD, vector<UserStageInfo>> m_mapWaitUsers;
    map<DWORD, UserStageInfo> m_mapDisplayStage;
};

