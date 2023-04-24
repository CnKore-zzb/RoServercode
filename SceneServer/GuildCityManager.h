/**
 * @file GuildCityManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-10-09
 */

#pragma once

#include "xSingleton.h"
#include "RecordCmd.pb.h"
#include "Scene.h"
#include "DScene.h"
#include "FuBenCmd.pb.h"
#include "MsgManager.h"

using std::map;
using std::vector;
using std::set;
using namespace Cmd;

class SceneNpc;
class SceneUser;
class xSceneEntryDynamic;
class GuildCity
{
  friend class GuildFireScene;
  public:
    GuildCity(DWORD cityid);
    virtual ~GuildCity();

    void timer(DWORD curSec);
    void onFireOpen();
    void onFireClose();
    void addScene(GuildFireScene* s) { m_setScenes.insert(s); }
    void delScene(GuildFireScene* s) { m_setScenes.erase(s); }
    void userEnter(SceneUser* user);
    void userLeave(SceneUser* user);
    void onNpcDie(SceneNpc* npc, xSceneEntryDynamic* killer);
    void onNpcBeAttack(SceneNpc* npc, bool bheal = false);
    void onGuildNameChange(const string& newname);

    EGuildFireState getState() { return m_eState; }
    QWORD getDefenseGuildID() { return m_qwDefenseGuildID; }
    bool checkKickUser(SceneUser* user);

    void handleResult(EGuildCityResult result, QWORD guildid);
    void setMetalNpcID(QWORD npcid) { m_qwGlamMetalGuid = npcid; }
    void sendMsgToCity(DWORD msgid, const MsgParams& params = MsgParams());
    void updateStateToSession();

    void init(bool fire);

    void onSuperGvgTimeNear();
    void onSuperGvgTimeCome();
  private:
    void endFire(EGuildFireState eNextState);
    void sendDangerStatus(bool danger);
    void sendCalmStatus(bool calm);
    void clearAllUser();
    void clearAttUser();
    void resetFuben();
    void startFuben();
    void sendCmdToCity(const void* cmd, DWORD len);
    void sendResult(EGuildFireResult result);
    void reward();
    void updateShowState();
  private:
    DWORD m_dwCityID = 0;

    QWORD m_qwDefenseGuildID = 0;
    string m_strDefenseName;

    QWORD m_qwGlamMetalGuid = 0;
    DWORD m_dwMetalNpcHpPer = 0;
    DWORD m_dwLastBeAttackT = 0;
    DWORD m_dwEndCalmTime = 0;
    bool m_bInDangerTime = false;
    DWORD m_dwEndDangerTime = 0;
    EGuildFireState m_eState = EGUILDFIRE_PEACE; // 当前状态
    EGuildFireState m_eNextState = EGUILDFIRE_MIN; // 标记冷静期结束后, 进入的状态

    EGCityState m_eClientShowState = EGCITYSTATE_MIN;//客户端显示状态
    DWORD m_dwAttUserNum = 0;
    DWORD m_dwDefUserNum = 0;

    bool m_bReFire = false;
    DWORD m_dwWaitResultTime = 0;
    //QWORD m_qwLatestGuildID = 0;

    std::set<GuildFireScene*> m_setScenes;

    map<QWORD, QWORD> m_mapUser2Guild;
    TSetDWORD m_setNeedKickUsers;
};

class GuildCityManager : public xSingleton<GuildCityManager>
{
  friend class xSingleton<GuildCityManager>;
  private:
    GuildCityManager();
  public:
    virtual ~GuildCityManager();

    void collectCityInfo(DWORD dwMapID, vector<GuildCityInfo>& setResult);
    void updateCityInfoFromGuild(const GuildCityActionGuildSCmd& cmd);
    void updateCityInfoFromGuild(const CityDataUpdateGuildSCmd& cmd);
    void updateCityInfoToGuild(DWORD dwFlagID, QWORD qwGuildID, EGuildCityStatus eStatus = EGUILDCITYSTATUS_OCCUPY);
    void updateCityInfoResult(DWORD dwFlagID, QWORD qwGuildID, EGuildCityResult eResult);
    const GuildCityInfo* getCityInfo(DWORD cityid);
    const GuildCityInfo* getCityInfoByGuild(QWORD guildid);
    const GuildCityInfo* getRealCityInfoByGuild(QWORD guildid);
    bool isInFire() { return m_bFire; }
    DWORD getFireStartTime() { return m_dwStartFireTime; }
    DWORD getFireStopTime() { return m_dwEndFireTime; }
    DWORD getSuperGvgStartTime() { return m_dwTestSuperGvgBeginTime ? m_dwTestSuperGvgBeginTime : m_dwSuperGvgBeginTime; }// 若有测试时间优先返回

    void openFireAtonce(bool super, DWORD supertime);
    void stopFireAtonce();
    bool isCityInFire(DWORD cityid);

    bool setTestSuperGvgBeginTime(DWORD time);
    void userOnLogin(SceneUser* user);
  private:
    void syncCityInfo(DWORD dwFlagID);
    void onFireOpen();
    void onFireClose();
  private:
    map<DWORD, GuildCityInfo> m_mapCityInfo;

  /*公会城池战斗*/
  public:
    void timer(DWORD curSec);
    GuildCity* getCityByID(DWORD cityid);
    GuildCity* createCity(DWORD cityid);
  private:
    map<DWORD, GuildCity*> m_mapGuildCity;
    bool m_bFire = false;
    bool m_bSuFire = false;
    DWORD m_dwStartFireTime = 0;
    DWORD m_dwEndFireTime = 0;
    DWORD m_dwSuperGvgBeginTime = 0; /*开始报名匹配的时间, +5 * 60 = 开战时间*/
    DWORD m_dwSuperGvgNoticeTime = 0;
    DWORD m_dwTestSuperGvgBeginTime = 0; /*GM指定决战开启时间*/
};

