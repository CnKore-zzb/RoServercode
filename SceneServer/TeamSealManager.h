#pragma once

#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include "xDefine.h"
#include "Seal.h"

using namespace Cmd;
using std::map;
using std::vector;
using std::set;


class SceneUser;
class Scene;

struct STeamSealData
{
  DWORD dwMapID = 0;
  DWORD dwSealStopTime = 0;
  QWORD qwActID = 0;

  DWORD dwCurValue = 0;
  DWORD dwNextProcessTime = 0;
  bool bFinished = false;
  DWORD dwFinishTime = 0;
  DWORD dwWaitTime = 0;

  SSealItem m_stCurItem;

  set<QWORD> m_setActUserID;
  set<QWORD> m_setCalledMonster;

  bool checkRepairOk();
  bool checkOk(DWORD curTime);
  bool checkFailure(DWORD curTime);
  void clear();

  STeamSealData() {}
};
typedef vector<STeamSealData> TVecTeamSealData2;

class SceneTeamSeal
{
  public:
    SceneTeamSeal(QWORD teamid);
    ~SceneTeamSeal();

    bool isSealing (DWORD mapid);

    void timer(DWORD curTime);

    void onMonsterDie(SceneNpc* npc);

    //void addSealData(DWORD dwID, xPos pos);

    void sendSealInfo(QWORD userid);

    void openSeal(DWORD dwID, Scene* pRaidScene = nullptr, xPos pos = xPos());
    bool beginSeal(SceneUser* pUser, EFinishType etype = EFINISHTYPE_NORMAL);
    bool beginSeal(SceneUser* pUser, QWORD sealid, EFinishType etype = EFINISHTYPE_NORMAL);

    void onEnterScene(SceneUser* pUser);
    void onLeaveScene(SceneUser* pUser);
    void onUserOffline(SceneUser* pUser);

    void onOverRaidSeal(DWORD mapid);
    void clearSealData(DWORD destSeal);

    DWORD getDestructTime() { return m_dwDestructTime; }
    void setDelStatus() { m_dwDestructTime = now(); }
    void addMember(QWORD userid);
    void removeMember(QWORD userid);

  private:
    void process(STeamSealData& sData, DWORD curTime);
    void callMonster(STeamSealData& sData);

    void finishSeal(DWORD mapid);
    void failSeal(DWORD mapid);
    void preFinishSeal(STeamSealData& sData);

    void delSceneImage(Scene* pScene);

    void kickDMapUser();
    SceneNpc* createSeal(Scene* pScene, const SSealItem& sItem);

  private:
    QWORD m_qwTeamID = 0;

    TVecTeamSealData2 m_vecTeamSealData;
    TVecTeamSealData2 m_vecFinishSeal;

    DWORD m_dwExitDMapTime = 0;
    DWORD m_dwDMapID = 0;
    DWORD m_dwCountDownTime = 0;

    DWORD m_dwDestructTime = 0;
    TSetQWORD m_setTeamers;
    bool m_bSealing = false;
};

typedef map<QWORD, SceneTeamSeal*> TMapTeamSeal;
class TeamSealManager : public xSingleton<TeamSealManager>
{
  friend class xSingleton<TeamSealManager>;
  private:
    TeamSealManager();
  public:
    virtual ~TeamSealManager();
  public:
    void timer(DWORD curSec);
  public:
    SceneTeamSeal* getTeamSealByID(QWORD guid);
  public:
    SceneTeamSeal* createOneTeamSeal(QWORD teamid);
  private:
    TMapTeamSeal m_mapTeamSeal;
};

