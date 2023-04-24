/**
 * @file BossMgr.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-08-16
 */

#pragma once

#include "xSingleton.h"
#include "BossStep.h"

class BossAct;
class SceneUser;
class SceneNpc;
namespace Cmd
{
  class SummonBossBossSCmd;
};

// BossActCFG
typedef std::vector<TPtrBossStep> TVecBossStep;
struct SBossActCFG
{
  DWORD dwActID = 0;
  TVecBossStep vecBossStep;
};
typedef std::map<DWORD, SBossActCFG> TMapBossActCFG;
typedef std::map<DWORD, BossAct*> TMapBossAct;

// BossMgr
class BossMgr : public xSingleton<BossMgr>
{
  friend class xSingleton<BossMgr>;
  private:
    BossMgr() : m_oOneSecTimer(1), m_oOneMinTimer(60) {}
  public:
    virtual ~BossMgr() {}

    bool loadConfig();
    bool onSummonBoss(const Cmd::SummonBossBossSCmd& rCmd, DWORD dwGMActID = 0);
    bool createAct(const SBossActCFG* pCFG, const Cmd::SummonBossBossSCmd& rCmd);
    bool summonBoss(const Cmd::SummonBossBossSCmd& rCmd, const xPos& rPos = xPos(0.0f, 0.0f, 0.0f));
    bool runStep(BossAct* pAct, SceneUser* pUser);
    bool runStep(DWORD dwMapID, SceneUser* pUser);
    bool canUpdate(BossAct* pAct);

    const SBossActCFG* getBossActCFG(DWORD dwID) const;

    void onEnterScene(SceneUser* pUser);
    void onKillMonster(SceneUser* pUser, DWORD dwMonsterID);
    void onStatus(SceneUser* pUser);
    void onTimer();
    void onShow(SceneNpc* pNpc);

    void timer(DWORD curSec);
    void clear();

    const string& debugInfo(DWORD dwMapID);
  private:
    bool canStep(BossAct* pAct);
    void recycleAct();
    void removeOverTimeAct(DWORD curSec);
  private:
    xTimer m_oOneSecTimer;
    xTimer m_oOneMinTimer;

    TMapBossActCFG m_mapBossActCFG;
    TMapBossAct m_mapBossAct;

    set<BossAct*> m_setRecycledAct;

    string m_strDebugInfo;
};

