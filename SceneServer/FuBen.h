#pragma once

#include <list>
#include "FuBenPhase.h"

using std::string;
using std::list;
using std::map;

class DScene;
class SceneUser;

typedef list<FuBenPhase *> FuBenPhaseList;
typedef map<DWORD, FuBenPhaseList> FuBenEntry;

class FuBen
{
  public:
    FuBen(DScene *qscene);
    ~FuBen();

    /******   静态配置   **********/
  public:
    static void final();
    static bool loadConfig();
    static std::map<DWORD, FuBenEntry> s_cfg;
    /*static bool isFuBen(DWORD id)
    {
      return s_cfg.find(id)!=s_cfg.end();
    }*/

    /******   静态配置   **********/

  public:
    bool active() { return !m_phase.empty(); }
    void timer(QWORD msec);

    bool add(DWORD fuben_id);

    // 遍历检查
  private:
    void check();
    void check(FuBenPhaseList &list);

    // 检查单个事件
  public:
    void check(const std::string& t, DWORD starID=0);
    void check(DWORD id);
  private:
    void check(FuBenPhaseList &list, const std::string &t);
    bool m_blCheck = false;

  public:
    void enter(SceneUser *user);
    void leave(SceneUser *user);
    void die(SceneUser *user);
  private:
    void fail();
  public:
    void sendInfo(SceneUser *user);
    void setID(DWORD star, DWORD id);
    //bool isStarFinish(DWORD starID);
    void syncStep(FuBenPhase* pPhase, SceneUser* user = nullptr);

  public:
    bool appendPhase(FuBenEntry & rFuBenEntry);

  public:
    void clear();
    bool clearRaid();
    bool restart();

    //变量
  public:
    std::map<DWORD, FuBenPhaseVar> m_oVars;

    // 已获得的star
    TSetDWORD m_oStarIDSet;
    // 副本完成状态
    FuBenResult m_result;

    //倒计时
  public:
    STimerDownState& getTimerDown() {return m_oTimerDown;}
    STimerDownState m_oTimerDown;
    bool m_bCheckAgain;
    // 副本载具
    SFuBenCarrierInfo m_stCarrierInfo;
    // 是否提示怪物数量
  public:
    void setMonsterCountShow(bool bShow) { m_blMonsterCount = bShow; }
    bool getMonsterCountShow() const { return m_blMonsterCount; }
  private:
    bool m_blMonsterCount = false;
  private:
    DScene *m_pQuestScene = nullptr;
    FuBenEntry m_phase;
};

