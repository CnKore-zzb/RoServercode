#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "xTime.h"
#include "GTeam.h"

class xSceneEntryDynamic;
class BaseSkill;

enum ESkillStatus
{
  ESKILLSTATUS_MIN = 0,
  ESKILLSTATUS_SWORDBREAK = 1, // 真剑破百道
  ESKILLSTATUS_CATCH = 2, //擒拿
  ESKILLSTATUS_CURSEDCIRCLE = 3, // 咒缚阵
  ESKILLSTATUS_SOLO = 4, // 独奏
  ESKILLSTATUS_ENSEMBLE = 5, // 合奏
};

typedef std::set<xSceneEntryDynamic*> TSetSceneEntrys;

class SkillStatus : private xNoncopyable
{
  public:
    SkillStatus(xSceneEntryDynamic *pEntry);
    virtual ~SkillStatus();

    ESkillStatus getStatus() const { return m_eStatus; }

    bool enterStatus(ESkillStatus eStatus, const TSetSceneEntrys& setOther, const BaseSkill* pSkill);
    bool setEnemyStatus(ESkillStatus eStatus, xSceneEntryDynamic* pMaster, QWORD endTime, DWORD skillid);
    bool inStatus(QWORD nowtime) const { return m_qwStatusEndTime >= nowtime && m_eStatus != ESKILLSTATUS_MIN; }
    bool inStatus(ESkillStatus eStatus) const { return getStatus() == eStatus && inStatus(xTime::getCurMSec()); }

    void onLeaveScene();
    void onDie();
    void onUseSkill();
    void clearStatus() { m_eStatus = ESKILLSTATUS_MIN; m_qwStatusEndTime = 0;}
    void onAttack(xSceneEntryDynamic* target);
    void delStatus(ESkillStatus eStatus);
    const TSetQWORD& getOtherCharID() { return m_setOtherID; }
    void onTeamChange(const GTeam& oldTeam, const GTeam& nowTeam);
    void onSkillReset();
  private:
    void end();
    void onConcertStatusChange();
  private:
    xSceneEntryDynamic *m_pEntry = NULL;

    ESkillStatus m_eStatus = ESKILLSTATUS_MIN;
    bool m_bMaster = false;
    TSetQWORD m_setOtherID;
    DWORD m_dwRelateSkillID = 0;

    QWORD m_qwStatusEndTime = 0;
};


