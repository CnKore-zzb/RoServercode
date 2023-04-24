#pragma once

#include <list>
#include <vector>
#include "xPool.h"
#include "xScene.h"
#include "SceneDefine.h"
#include "PathFindingTile.h"
#include "GearManager.h"
#include "ExitManager.h"
#include "MapConfig.h"
#include "SceneImage.h"
#include "SceneTreasure.h"
#include "MiscConfig.h"
#include "PhotoCmd.pb.h"
#include "DressUpStageMgr.h"

using std::list;
using std::map;
using std::vector;

class SceneUser;
class SceneNpc;
class Scene;
class xSceneEntryDynamic;
class xLuaData;

#define FINE_WEATHER_ID     5

// npc template
struct SceneNpcTemplate
{
  DWORD m_dwNum = 1;
  NpcDefine m_oDefine;
  SceneNpcTemplate() {}
};

// exit point
struct ExitPoint
{
  DWORD m_dwExitID = 0;
  DWORD m_dwNextBornPoint = 0;
  DWORD m_dwVisible = 1;
  DWORD m_dwQuestID = 0;
  DWORD m_dwPrivate = 0;
  int m_intNextMapID = 0;

  float m_flRange = 0.0f;
  xPos m_oPos;

  ExitPoint() {}
  bool check(const xPos& p) const { return checkDistance(m_oPos, p, 10.0f); }
  bool isVisible(SceneUser *user, Scene *scene=NULL) const;
};

// Seat
struct Seat
{
  DWORD m_dwSeatId = 0;
  float m_flRange = 0.0f;
  xPos m_oSeatPos;

  QWORD m_seatUser = 0;

  bool m_bOpen = true;
  DWORD m_dwSeatTime = 0;
  DWORD m_dwSeatDownTime = 0;

  std::map<DWORD, DWORD> m_mapCost;

  Seat() {}
  void seatDown(QWORD charId);
  void seatUp(QWORD charId);
  bool check(const xPos& p) const { return checkDistance(m_oSeatPos, p, MiscConfig::getMe().getSystemCFG().fMaxSeatDis); }
};

// flag
struct SFlag
{
  DWORD dwFlagID = 0;
  list<xPos> listPos;

  bool isValidPos(const xPos& rPos) const
  {
    auto l = find_if(listPos.begin(), listPos.end(), [rPos](const xPos& r) -> bool{
      return getXZDistance(r, rPos) < 10.0f;
    });
    return l != listPos.end();
  }
};
typedef map<DWORD, SFlag> TMapFlagCFG;

struct SBornPoint
{
  DWORD dwRange = 0;
  xPos oPos;
};

struct SVisibleNpc
{
  QWORD uniqueid = 0;
  DWORD npcid = 0;
  xPos oPos;
};

// wedding frame
enum EFrameType
{
  EFRAMETYPE_MIN = 0,
  EFRAMETYPE_GUILD = 1,
  EFRAMETYPE_WEDDING = 2,
  EFRAMETYPE_MAX = 3,
};
struct SFrame
{
  DWORD dwFrameID = 0;
  xPos oPos;
  EFrameType eType = EFRAMETYPE_MIN;
};
typedef map<DWORD, SFrame> TMapFrameCFG;

// object
class SceneObject
{
  friend class SceneBase;
  friend class SceneManager;
  public:
    SceneObject() {}
    ~SceneObject() {}

    void load(const xLuaData &data);
    void clear() { m_oNpcList.clear(); m_oBornPoints.clear(); m_oExitPoints.clear(); m_oRaidNpcList.clear(); }

    const list<SceneNpcTemplate>& getNpcList() const { return m_oNpcList; }
    vector<xPos> getNpcPos() const;
    const map<DWORD, ExitPoint>& getExitPointList() const { return m_oExitPoints; }
    const map<DWORD, SBornPoint>& getBornPointList() const { return m_oBornPoints; }
    const map<DWORD, SceneNpcTemplate>& getRaidNpcList() const { return m_oRaidNpcList; }

    const ExitPoint* getExitPoint(DWORD id) const;
    const xPos* getBornPoint(DWORD id) const;
    const SceneNpcTemplate* getNpcTemplate(DWORD dwID) const;
    const SceneNpcTemplate* getRaidNpcTemplate(DWORD dwID) const;
    const SBornPoint* getSBornPoint(DWORD id) const;

  private:
    list<SceneNpcTemplate> m_oNpcList;
    map<DWORD, SBornPoint> m_oBornPoints;
    map<DWORD, ExitPoint> m_oExitPoints;
    map<DWORD, SceneNpcTemplate> m_oRaidNpcList;
};

// base
class SceneBase
{
  friend class SceneManager;
  public:
    SceneBase();
    ~SceneBase();

    bool init(const SMapCFG* pCFG);
    bool loadFlag(xLuaData& data);
    bool loadFrame(xLuaData& data);
    bool loadNoFlyPoints(xLuaData& data);

    inline bool isStaticMap() const { return m_pCFG == nullptr ? false : m_pCFG->bStatic; }
    inline DWORD getStaticGroup() const { return m_pCFG == nullptr ? 0 : m_pCFG->dwStaticGroup; }
    inline bool isPvPMap() const { return m_pCFG == nullptr ? false : m_pCFG->bPvp; }
    inline bool isPreview() const { return m_pCFG == nullptr ? false : m_pCFG->bPreview; }
    inline bool isTransformMap() const { return m_pCFG == nullptr ? false : m_pCFG->hasFunction(EMAPFUNCTION_TRANSFORM); }
    inline DWORD getReliveMapID() const { return m_pCFG == nullptr ? 0 : m_pCFG->dwReliveMap; }
    inline DWORD getReliveBp() const { return m_pCFG == nullptr ? 0 : m_pCFG->dwReliveBp; }
    inline DWORD getMapID() const { return m_pCFG == nullptr ? 0 : m_pCFG->dwID; }

    Seat* getSeat(DWORD seatId) const;
    const SFlag* getFlag(DWORD dwFlagID) const;
    const SceneObject* getSceneObject(DWORD raid) const;
    const SMapCFG* getCFG() const { return m_pCFG; }
    PathFindingTile* getPathFinding() const { return m_pPathfinding; }
    const MapInfo& getMapInfo() const { return m_oInfo; }
    const TMapFlagCFG& getFlagInfo() const { return m_mapFlagCFG; }

    const SFrame* getFrameCFG(DWORD dwFrameID) const;
    const TMapFrameCFG& getFrameListCFG() const { return m_mapFrameCFG; }

    bool getRandPos(const xPos& pos, float r, xPos& newpos) const
    {
      const MapInfo& rInfo = getMapInfo();

      int i = 0;
      while (i++ < 30)
      {
        float x = randFBetween(pos.x - r, pos.x + r);
        float z = randFBetween(pos.z - r, pos.z + r);
        float y = randFBetween(rInfo.range.yMin, rInfo.range.yMax);
        //float y = pos.y;
        newpos = xPos(x, y, z);
        if (m_pPathfinding && m_pPathfinding->getValidPos(newpos, newpos))
          return true;
      }
      newpos = pos;
      return true;
    }

  private:
    const SMapCFG* m_pCFG = nullptr;
    PathFindingTile *m_pPathfinding = nullptr;
    MapInfo m_oInfo;

    // raid SceneObject
    map<DWORD, SceneObject> m_oObjectList;
    // 静态地图
    SceneObject m_oObject;
    // PVP地图
    SceneObject m_oObjectPVP;

    map<DWORD, Seat> m_oSeats;
    TMapFlagCFG m_mapFlagCFG;
    TMapFrameCFG m_mapFrameCFG;
};

enum EENVMODE
{
  EENVMODE_NONE = 1,  //无
  EENVMODE_ALLDAY =2,   //全天
  EENVMODE_DAY = 3,     //白天
  EENVMODE_NIGHT = 4    //黑夜
};
struct EnvSetting
{
  EENVMODE mode = EENVMODE_NONE;
  DWORD skyid = 0;
  DWORD weatherid = 0;
};
typedef vector<GuildPhoto> TVecGuildPhoto;
typedef list<GuildPhoto> TListGuildPhoto;

// scene
class Scene : public xScene
{
  public:
    Scene(DWORD sID, const char* sName, const SceneBase *sBase);
    virtual ~Scene();

    virtual bool init();

    virtual SCENE_TYPE getSceneType() const = 0;
    virtual bool isSScene() const = 0;
    virtual bool isDScene() const = 0;

    virtual void entryAction(QWORD curMSec);
    virtual bool isPVPScene();
    virtual bool isDPvpScene() { return false; }
    virtual bool isMatchScene() { return false; }
    virtual bool isHideUser() { return false; }
    virtual bool isPollyScene() { return false; }
    virtual bool isTowerScene() { return false; }
    virtual bool isAltmanScene() { return false; }
    virtual bool isUserCanFireScene() { return base && base->isTransformMap(); }
    virtual bool isGvg() { return false; }
    virtual bool isSuperGvg() { return false; }
    virtual bool canUseGoToPos() { return true; }
    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user) { m_oTreasure.onLeaveScene(user); }
    virtual void onLeaveScene(SceneUser* user) {};

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer) { m_oTreasure.onTreeDie(npc); }
    virtual void onNpcBeAttack(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp){};
    virtual void onNpcBeHeal(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp){};
    virtual void onReliveUser(SceneUser* user, SceneUser* reliver) {}; //reliver复活user

    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer) {}
    virtual void onNpcDieReward(SceneNpc* npc, SceneUser* user) {} // user为奖励获取者,并不一定是击杀者

    DWORD getMapID() const { return base == nullptr ? 0 : base->getMapID(); }
    const SceneObject* getSceneObject();
    const SceneBase* getSceneBase() const { return base; }
    const SMapCFG* getBaseCFG() const { return base ? base->getCFG(): nullptr; }
    void sendCmdToAll(const void *cmd, WORD len);
    void setBgm(Cmd::EBgmType type, bool play, DWORD times, const std::string& bgmPath);

    bool getValidPos(xPos &pos)
    {
      if (base->getPathFinding())
        return base->getPathFinding()->getValidPos(pos, pos);
      return false;
    }
    bool getValidPos(xPos pos, xPos &out)
    {
      if (base->getPathFinding())
        return base->getPathFinding()->getValidPos(pos, out);
      return false;
    }
    bool getRandPos(const xPos& pos, float r, xPos& newpos)
    {
      /*if (base->getPathFinding())
        return base->getPathFinding()->findRandomPoint(pos, r, newpos);
        return false;*/

      if (base == nullptr)
        return false;
      const MapInfo& rInfo = base->getMapInfo();

      int i = 0;
      while (i++ < 30)
      {
        float x = randFBetween(pos.x - r, pos.x + r);
        float z = randFBetween(pos.z - r, pos.z + r);
        float y = randFBetween(rInfo.range.yMin, rInfo.range.yMax);
        //float y = pos.y;
        newpos = xPos(x, y, z);
        if (rInfo.isNoFly(newpos) == true)
          continue;
        if (getValidPos(newpos))
          return true;
      }

      newpos = pos;
      return true;
    }
    /*在以centerPos为圆心, radius为半径(不计算y轴距离)圆内, 随机一个坐标点*/
    bool getRandPosInCircle(const xPos& centerPos, float radius, xPos& newPos)
    {
      if (base == nullptr)
        return false;
      const MapInfo& rInfo = base->getMapInfo();

      bool bNearly = radius <= 10;

      int i = 0;
      while (i++ < 30)
      {
        float angle = randBetween(1, 360);
        float r = randFBetween(0, radius);
        newPos.x = centerPos.x + r * cos(angle * 3.14 / 180);
        newPos.z = centerPos.z + r * sin(angle * 3.14 / 180);
        if (bNearly)
          newPos.y = randFBetween(centerPos.y - radius * 0.2, centerPos.y + radius * 0.2);
        else
          newPos.y = randFBetween(rInfo.range.yMin, rInfo.range.yMax);
        if (getValidPos(newPos))
          return true;
      }
      newPos = centerPos;
      return true;
    }
    bool getRandPosOverRange(const xPos& pos, float r, xPos& tpos);
    bool getRandTargetPos(xPos pos, float r, xPos& tpos)
    {
      if (base->getPathFinding())
        return base->getPathFinding()->findRandomPoint(pos, r, tpos);
      return false;
    }
    bool getRandPos(xPos &pos)
    {
      if (base == nullptr)
        return false;

      const MapInfo& rInfo = base->getMapInfo();
      int i = 0;
      while (i++ < 100)
      {
        float x = randFBetween(rInfo.range.xMin, rInfo.range.xMax);
        float z = randFBetween(rInfo.range.zMin, rInfo.range.zMax);
        float y = randFBetween(rInfo.range.yMin, rInfo.range.yMax);
        pos = xPos(x, y, z);
        if (rInfo.isNoFly(pos) == true)
          continue;
        if (getValidPos(pos))
        {
          return true;
        }
      }
      return false;
    }

    bool getRandPosAwayNpc(xPos &pos, float r)
    {
      if (base == nullptr)
        return false;
      const SceneObject* pSceneObj = base->getSceneObject(0);
      if (pSceneObj == nullptr)
        return false;
      vector<xPos> vecPos = pSceneObj->getNpcPos();

      DWORD i = 0;
      bool ret = false;
      while (i++ < 100)
      {
        if (getRandPos(pos) == false)
          continue;
        ret = true;
        for (auto &v : vecPos)
        {
          if (getDistance(pos, v) < r)
          {
            ret = false;
            break;
          }
        }
        if (ret)
          return true;
      }
      return false;
    }

    bool getCircleRoundPos(const xPos& oPos, float r, xPos& newPos)
    {
      DWORD angle = randBetween(0, 360);
      newPos.x = oPos.x + r * cos(angle);
      newPos.y = oPos.y;
      newPos.z = oPos.z + r * sin(angle);
      return getValidPos(newPos);
    }

    //正面dir位置
    bool getPosByDir(const xPos& oPos, float r, DWORD dir, xPos& newPos)
    {
      DWORD angle = dir / 1000;
      newPos.x = oPos.x + r * sin(angle * 3.14 / 180);
      newPos.y = oPos.y;
      newPos.z = oPos.z + r * cos(angle * 3.14 / 180);
      return getValidPos(newPos, newPos);
    }

    bool resetPosHeight(xPos &pos)
    {
      if (base->getPathFinding())
        return base->getPathFinding()->resetPosHeight(pos);
      return false;
    }
    bool findingPath(xPos f, xPos t, std::list<xPos> &path, ToolMode mode=TOOLMODE_PATHFIND_STRAIGHT);

    bool addQuestsAllUser(TVecDWORD& vecQuests);
    bool delQuestsAllUser(TVecDWORD& vecQuests);
    void kickAllUser(bool bForceAll = false);
  public:
    void addKickList(SceneUser* user);
  private:
    void kickUser(SceneUser* user);
  public:
    void timer(QWORD curMSec);
    void onFiveSecTimeUp(DWORD curSec);
  public:
    const SceneBase *base = nullptr;
    GearManager m_oGear;
    ExitManager m_oExit;
    SceneImage m_oImages;

    // active screen
  public:
    inline void addActiveNineScreen(const xPos &pos) { getNineScreen(xPos2xPosI(pos), m_oActiveScreenSet); }
    inline void addActiveScreen(const xPos &pos) { m_oActiveScreenSet.insert(xPos2xPosI(pos)); }
  private:
    TSetDWORD m_oActiveScreenSet;

  public:
    void getSceneNpcByBaseID(DWORD baseid, std::list<SceneNpc *> &npclist);
  public:
    DWORD getWeather() const { return m_dwWeatherId; }
    DWORD getSky() const { return m_dwCurSkyId; }
    inline bool canChangeSky(DWORD curSec);

    void setEnvSetting(DWORD mode, DWORD skyId, DWORD weatherId);
  private:
    void changeSkyForce(DWORD skyId, DWORD duration, DWORD curSec = 0);

  private:
    bool initNpc();
    bool hideNpc(DWORD npcId);
  private:
    SWeatherSkyCFG m_stWeatherCFG;
    TVecDWORD m_stSkyCFG;
  public:
    SceneTreasure& getSceneTreasure() { return m_oTreasure; }
  private:
    SceneTreasure m_oTreasure;
    DWORD m_dwCurSkyId = 0;
    DWORD m_dwNextSkyAutoChangeTime = 0;
    DWORD m_dwWeatherId = 0;
    xTimer m_oOneSecTimer;
    xTimer m_oFiveSecTimer;
    EnvSetting m_envSetting;
    TSetDWORD m_setSeat;
  public:
    void addSummonNpc(QWORD qwID) { m_setSummonedNpcs.insert(qwID); }
    void delSummonNpc(QWORD qwID) { m_setSummonedNpcs.erase(qwID); }
    void onNpcGuidChange(QWORD qwOldID, QWORD qwNewID);
    const set<QWORD>& getSummonedNpcs() { return m_setSummonedNpcs; }
    void addVisibleNpc(SceneNpc* pNpc);
    void delVisibleNpc(SceneNpc* pNpc);
    void refreshVisibleNpc(Cmd::NtfVisibleNpcUserCmd& cmd, SceneNpc* pNpc);
    void sendVisibleNpc(SceneUser* pUser, bool show = true);
    std::map<QWORD, SVisibleNpc>& getVisibleNpcs() { return m_mapVisibleNpcs; }
    void addSeat(DWORD seatid);
    void delSeat(DWORD seatid);
    void sendSeatToUser(SceneUser* user);
    virtual bool isFreeSkill(DWORD skillid) { return false; }
    virtual bool isForbidSkill(DWORD skillid) { return false; }
  private:
    set<QWORD> m_setSummonedNpcs;
    bool m_bgmPlay = false;
    Cmd::EBgmType m_bgmType = EBGM_TYPE_QUEST;
    std::string m_bgmPath;    //bgm 
    std::map<QWORD, SVisibleNpc> m_mapVisibleNpcs;

  public:
    std::set<QWORD> m_oKickUserList;
    // 相框
  public:
    virtual bool queryFrame(SceneUser* pUser, DWORD dwFrameID);
    virtual bool frameAction(SceneUser* pUser, const FrameActionPhotoCmd& cmd);
    virtual void sendPhotoWall(SceneUser* pUser = nullptr);
    virtual void refreshPhotoWall(DWORD curSec);
    virtual void resetPhotoWall(EFrameType eType);
    bool modifyPhoto(SceneUser* pUser, GuildPhoto& r);
  private:
    map<DWORD, TListGuildPhoto> m_mapFramePhoto;
    map<DWORD, DWORD> m_mapFrameIndex;
    DWORD m_dwFrameTick = 0;
   public:
     virtual void addReliveLeaveUser(QWORD charId) {}
     bool canEnter(SceneUser* pUser);
     bool isNoramlScene();
   private:
     DWORD m_dwKickUserTimeTick = 0; // 副本关闭, 踢出所有玩家.踢人间隔时间戳
};

// sscene
class SScene : public Scene
{
  public:
    SScene(DWORD sID, const char* sName, SceneBase* sBase) : Scene(sID, sName, sBase) {}
    virtual ~SScene() {}

    virtual SCENE_TYPE getSceneType() const { return SCENE_TYPE_STATIC; }
    virtual bool isSScene() const { return true; }
    virtual bool isDScene() const { return false; }
};

// dscene
#include "FuBen.h"
const DWORD NOUSERPROTECTTICK = 60;
class DScene : public Scene
{
  public:
    DScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG);
    virtual ~DScene() {}

    virtual bool init();

    virtual bool isSScene() const { return false; }
    virtual bool isDScene() const { return true; }
    virtual void addDamageUser(QWORD attackID, QWORD suffererID, DWORD damage){}
    virtual void addHealUser(QWORD id, QWORD healID, DWORD hp){}
    virtual void onKillUser(SceneUser* pKiller, SceneUser* pDeath){}
    virtual void checkCombo(QWORD id, DWORD curSec){}

    virtual void entryAction(QWORD curMSec);

    virtual void userEnter(SceneUser *user);
    virtual void userLeave(SceneUser *user);

    virtual void onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer);
    virtual void onUserDie(SceneUser *user, xSceneEntryDynamic *killer);

    virtual void onClose();

    ERaidType getRaidType() const { return m_pRaidCFG == nullptr ? ERAIDTYPE_MIN : m_pRaidCFG->eRaidType; }
    ERaidRestrict getRaidRestrict() const { return m_pRaidCFG == nullptr ? ERAIDRESTRICT_MIN : m_pRaidCFG->eRestrict; }

    DWORD getRaidLimitTime() const { return m_pRaidCFG == nullptr ? 0 : m_pRaidCFG->dwRaidLimitTime; }
    DWORD getRaidEndTime() const { return m_pRaidCFG == nullptr ? 0 : m_pRaidCFG->dwRaidEndWait; }
    DWORD getRaidNoUserTime() const { return m_pRaidCFG == nullptr ? 0 : m_pRaidCFG->dwRaidTimeNoUser; }
    DWORD getRaidID() const { return m_pRaidCFG == nullptr ? 0 : m_pRaidCFG->dwRaidID; }
    const SRaidCFG* getRaidCFG() const { return m_pRaidCFG; }

    bool isShowAllNpc() const { return m_pRaidCFG == nullptr ? false : m_pRaidCFG->bShowAllNpc; }
    bool isNoPlayGoMap() const { return m_pRaidCFG == nullptr ? false : m_pRaidCFG->bNoPlayGoMap; }
    bool isQuestFail() const { return m_pRaidCFG == nullptr ? false : m_pRaidCFG->bQuestFail; }

    DWORD getCloseTime() const { return m_dwCloseTimeNoUser != 0 ? m_dwCloseTimeNoUser : m_dwCloseTime; }
    void setCloseTime(DWORD dwTime, bool bNoUser = false) { if (bNoUser) m_dwCloseTimeNoUser = dwTime; else m_dwCloseTime = dwTime; }

    FuBen& getFuben() { return m_oFuben; }

    //全员死亡是否需要倒计时
    inline bool needCountDown();
    bool isAllDead();
    void stopCountDown();
    void startCountDown();
    virtual void addReliveLeaveUser(QWORD charId) { m_setReliveLeaveUser.insert(charId); }

    DWORD getAllAliveMonsterNum();

    virtual bool isFreeSkill(DWORD skillid) { return m_setFreeSkills.find(skillid) != m_setFreeSkills.end(); }
    void addFreeSkill(DWORD skillid) { m_setFreeSkills.insert(skillid); }
    void delFreeSkill(DWORD skillid) { m_setFreeSkills.erase(skillid); }

    virtual bool isForbidSkill(DWORD skillid) { return m_setForbidSkills.find(skillid) != m_setForbidSkills.end(); }
    void addForbidSkill(DWORD skillid) { m_setForbidSkills.insert(skillid); }
    void delForbidSkill(DWORD skillid) { m_setForbidSkills.erase(skillid); }

    virtual void onNpcDieReward(SceneNpc* npc, SceneUser* user);
    void inviteSummonDeadBoss(SceneUser* user);
    void replySummonDeadBoss(SceneUser* user, bool agree);
    void doSummonDeadBoss();
    virtual void getRandomDeadBoss(std::list<pair<DWORD,DWORD>>& bosslist){};
    virtual bool checkSummonDeadBoss() { return false; }
  protected:
    void setCloseState();
  protected:
    const SRaidCFG* m_pRaidCFG = nullptr;

    DWORD m_dwCloseTimeNoUser = 0;
    DWORD m_dwCloseTime = 0;
    DWORD m_dwNoUserProtectTick = 0;

    FuBen m_oFuben;
    std::set<QWORD> m_setReliveLeaveUser;
    TSetDWORD m_setFreeSkills; // 在当前副本中可以释放的技能
    TSetDWORD m_setForbidSkills; // 在当前副本中禁用的技能

    // 队长发起, 等待队员同意
    TSetQWORD m_setWaitReplyUsers;
    DWORD m_dwWaitWaitReplyTime = 0;
    TSetQWORD m_setDeadBossSummonUsers;
};

