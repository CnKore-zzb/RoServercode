#pragma once

#include "xSingleton.h"
#include "TableStruct.h"
#include "BossSCmd.pb.h"
#include "MiscConfig.h"
#include "BossConfig.h"

class SessionScene;
class SessionUser;

struct SBossCFG;

enum class BossState
{
  WaitRefresh = 1,
  Refreshed = 2,
};

// Boss
class Boss
{
  public:
    Boss(const SBossCFG *b);
    virtual ~Boss();

  public:
    void setState(BossState s)
    {
      if (m_oState == s) return;

      m_oState = s;
      if (base)
      {
        XLOG << "[BOSS]" << id << base->strName << "设置状态" << getStateString(s) << XEND;
      }
    }
    BossState getState()
    {
      return m_oState;
    }
    const char* getStateString(BossState s)
    {
      switch (s)
      {
        case BossState::WaitRefresh:
          {
            return "等待刷新";
          }
          break;
        case BossState::Refreshed:
          {
            return "已刷新";
          }
          break;
        default:
          break;
      }
      return "";
    }
    virtual DWORD getMapID() { return base->getMapID(); }
    virtual DWORD getRefreshTime()
    {
      if (!m_dwDieTime && !m_dwRefreshTime) return 0;
      DWORD t = m_dwDieTime + base->getReliveTime(getMapID()) * MIN_T;
      DWORD d = t > m_dwRefreshTime ? t - m_dwRefreshTime : m_dwRefreshTime - t;
      if (d > MiscConfig::getMe().getBossCFG().dwRefreshBaseTimes)
        return m_dwRefreshTime - m_dwRefreshTime % MIN_T;
      return t;
    }
    void setSetTime(DWORD dwTime) { m_dwSetTime = dwTime; }
    DWORD getSetTime() const { return m_dwSetTime; }

    void setDieTime(DWORD dwTime) { m_dwDieTime = dwTime; }
    DWORD getDieTime() const { return m_dwDieTime; }

    void setDeadLv(DWORD lv) { m_dwDeadLv = lv; }
    DWORD getDeadLv() const { return m_dwDeadLv == 0 ? 1 : m_dwDeadLv; }

    QWORD getLastKillID() const { return (base == nullptr || base->getType() != EBOSSTYPE_DEAD) ? m_qwLastKillerID : m_qwDeadLastKillerID; }
    const string& getLastKiller() const { return (base == nullptr || base->getType() != EBOSSTYPE_DEAD) ? m_strLastKiller : m_strDeadLastKiller; }

    void updateWorldBoss(const WorldBossNtf& cmd) { m_oWorldBossNtf.CopyFrom(cmd); }
    void clearWorldBoss() { m_oWorldBossNtf.Clear(); }
  public:
    DWORD id = 0;     // 配置表的编号
    DWORD m_dwRefreshTime = 0;      // 下次刷新时间 boss 死亡后算出
    DWORD m_dwDieTime = 0;          // 死亡时间
    QWORD m_qwLastKillerID = 0;     // 上次击杀者id
    BossKillerData m_oKillerData;   // 击杀者缓存数据
    DWORD m_dwSetTime = 0;          // 点名时间
    DWORD m_dwSummonTime = 0;       // 召唤时间
    DWORD m_dwDeadLv = 0;           // 亡者boss等级
    DWORD m_dwMapID = 0;

    QWORD m_qwDeadLastKillerID = 0; // 上次亡者击杀者id
    std::string m_strLastKiller;    // 上次击杀者
    std::string m_strDeadLastKiller;// 上次亡者击杀者

    BossState m_oState = BossState::WaitRefresh;

    const SBossCFG *base = nullptr;
    WorldBossNtf m_oWorldBossNtf;
};

// Mini
class Mini : public Boss
{
public:
  Mini(const SBossCFG*b, DWORD mapid);
  virtual ~Mini();

  virtual DWORD getMapID() { return m_dwMapID; }

private:
  DWORD m_dwMapID = 0;
};

// BossList
class BossList : public xSingleton<BossList>
{
  friend class xSingleton<BossList>;
  private:
    BossList();
  public:
    virtual ~BossList();

  public:
    bool doUserCmd(SessionUser *user, const BYTE *buf, WORD len);

  public:
    bool loadConfig();
    bool add(Boss *boss);
    bool add(Mini *mini);
    bool addWorld(Boss *boss);
    Boss* get(DWORD id);
    Boss* getWorld(DWORD id);
    Mini* get(DWORD id, DWORD mapid);

    void onSceneOpen(SessionScene *scene);
    void onSceneClose(SessionScene *scene);
    void onBossDie(DWORD npcid, const char *killer, QWORD killerId, DWORD mapid, bool reset);
    void onBossOpen() { m_bBossFuncFirst = true; if (m_oDeadSetFunc != nullptr) m_oDeadSetFunc(); }
    void onEnterScene(SessionUser* pUser);

    void timer(DWORD cur);
    void refresh(Boss *boss);
    void refresh(Mini *mini);

    void save();
    void load();

    void updateWorldBoss(const WorldBossNtfBossSCmd& cmd);
    void callSetFunc() { if (m_oDeadSetFunc) m_oDeadSetFunc(); }
  private:
    static QWORD getMiniKey(DWORD npcid, DWORD mapid) { return QWORD(npcid) | (QWORD(mapid) << 32); }

    EBossRefreshState getRereshState(DWORD curtime, DWORD refreshtime)
    {
      SQWORD delta = SQWORD(refreshtime) - SQWORD(curtime);
      if (delta < 30 * MIN_T)
        return EBOSSREFRESHSTATE_UPCOMING;
      else if (delta < 1 * HOUR_T)
        return EBOSSREFRESHSTATE_SHORT;
      else
        return EBOSSREFRESHSTATE_LONG;
    }

    DWORD calcRefreshTime(DWORD revivetime)
    {
      DWORD basetimes = MiscConfig::getMe().getBossCFG().dwRefreshBaseTimes;
      return revivetime + (basetimes ? randBetween(1, basetimes) : 0);
      /* int basetimes = MiscConfig::getMe().getBossCFG().dwRefreshBaseTimes; */
      /* basetimes = basetimes > 0 ? basetimes : 1; */
      /* revivetime += randBetween(-(revivetime / basetimes), revivetime / basetimes); */
      /* return revivetime > 0 ? revivetime : randBetween(1, 60); */
    }

  private:
    std::map<DWORD, Boss *> m_list;
    std::map<QWORD, Mini *> m_miniList;
    std::map<QWORD, Boss *> m_worldList;

    xTimeWheel m_oTimeWheel;
    std::function<void()> m_oDeadSetFunc;
    bool m_bBossFuncFirst = false;
    bool m_bBossFuncSecond = false;
};

