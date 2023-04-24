/**
 * @file SceneFighter.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-06
 */

#pragma once

#include "xPool.h"
#include "FighterSkill.h"

using namespace Cmd;
using std::vector;

typedef vector<SceneFighter*> TVecSceneFighter;

// scene fighter
class SceneUser;
struct SRoleBaseCFG;
class SceneFighter : public xObjectPool<SceneFighter>
{
  public:
    SceneFighter(SceneUser* pUser, const SRoleBaseCFG* pCFG);
    ~SceneFighter();

    void toData(FighterInfo* pInfo);

    FighterSkill& getSkill() { return m_oSkill; }
    SceneUser* getUser() const { return m_pUser; }

    void setRoleCFG(const SRoleBaseCFG* pCFG) { m_pCFG = pCFG; }
    const SRoleBaseCFG* getRoleCFG() const { return m_pCFG; }

    DWORD getUsedPoint() const { return m_dwUsedPoint; }
    DWORD getTotalPoint() const { return m_dwTotalPoint; }
    void setTotalPoint(DWORD point);

    DWORD getHp() const { return m_dwHp; }
    void setHp(DWORD hp);
    DWORD getSp() const { return m_dwSp; }
    void setSp(DWORD sp);

    EProfession getProfession() const { return m_eProfession; }
    void setProfession(EProfession eProfession);

    QWORD getJobExp() const { return m_qwJobExp; }
    void setJobExp(QWORD exp);

    DWORD getJobLv() const { return m_dwJobLv; }
    void setJobLv(DWORD lv);

    bool isBuy() const { return m_bBuy; }

    DWORD getBattlePoint();

    // data
    bool fromRoleData(const UserRoleData& data);
    bool toRoleData(UserRoleData& data);

    bool loadProfessionData(const UserRoleData& rData, bool isFirst = false);
    bool saveProfessionData(UserRoleData* pData);

    // attr point
    bool addAttrPoint(EUserDataType eType, DWORD point);
    bool resetAttrPoint();
    DWORD getAttrPoint(EAttrType eType);

    void checkUnlock();

    void timer(DWORD curTime);

    void syncMaxJobLv(DWORD dwMaxJobLv);
    DWORD getMaxJobLv() const;
    void setMaxJobLv(DWORD lv);
    void reqAddMaxJobCmd(const AddJobLevelItemCmd& cmd);

    DWORD getBranch() { return m_dwBranch; }
    void setBranch(DWORD dwBranch) { m_dwBranch = dwBranch; }
  private:
    SceneUser* m_pUser = nullptr;
    const SRoleBaseCFG* m_pCFG = nullptr;

    QWORD m_qwJobExp = 0;
    DWORD m_dwJobLv = 0;

    DWORD m_dwStrPoint = 0;
    DWORD m_dwIntPoint = 0;
    DWORD m_dwAgiPoint = 0;
    DWORD m_dwDexPoint = 0;
    DWORD m_dwVitPoint = 0;
    DWORD m_dwLukPoint = 0;
    DWORD m_dwTotalPoint = 0;
    DWORD m_dwUsedPoint = 0;

    DWORD m_dwHp = 0;
    DWORD m_dwSp = 0;

    DWORD m_dwBattlePoint = 0;

    DWORD m_dwMaxJobLv = 0;

    EProfession m_eProfession = EPROFESSION_MIN;

    DWORD m_dwBranch = 0; // 所在分支：第一次购买时设置下当前fighter所在分支
    bool m_bBuy; // 是否为购买分支

    FighterSkill m_oSkill;

    TVecDWORD m_vecUnlockLv;
};

