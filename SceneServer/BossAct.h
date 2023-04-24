/**
 * @file BossAct.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-08-16
 */

#pragma once

#include "xDefine.h"
#include "BossStep.h"

struct SBossActCFG;
class SceneUser;

struct SActProcess
{
  QWORD qwParam = 0;
  DWORD dwProcess = 0;

  TSetQWORD setCharIDs;

  void clear() { dwProcess = 0; setCharIDs.clear(); }

  bool isEnableChar(QWORD qwCharID) { return setCharIDs.find(qwCharID) != setCharIDs.end(); }
  void addEnableChar(QWORD qwCharID) { if (isEnableChar(qwCharID) == false) setCharIDs.insert(qwCharID); }
  void removeEnableChar(QWORD qwCharID) { if (isEnableChar(qwCharID) == true) setCharIDs.erase(qwCharID); }
};

// BossAct
class BossAct
{
  public:
    BossAct() {}
    ~BossAct() {}

    void setActID(DWORD dwActID) { m_dwActID = dwActID; }
    DWORD getActID() const { return m_dwActID; }

    void setBossID(DWORD dwBossID) { m_dwBossID = dwBossID; }
    DWORD getBossID() const { return m_dwBossID; }

    void setMapID(DWORD dwMapID) { m_dwMapID = dwMapID; }
    DWORD getMapID() const { return m_dwMapID; }

    void setLv(DWORD dwLv) { m_dwLv = dwLv; }
    DWORD getLv() const { return m_dwLv; }

    void setCFG(const SBossActCFG* pCFG) { m_pCFG = pCFG; }
    const SBossActCFG* getCFG() const { return m_pCFG; }

    TPtrBossStep getStepCFG();
    bool finish() const;

    void notify(SceneUser* pUser = nullptr);
    void notifyOverTime();

    void addStep() { ++m_dwStep; m_stProcess.clear(); notify(); }
    DWORD getStep() { return m_dwStep; }

    void setDestTime(DWORD dwTime) { m_dwDestTime = dwTime; }
    DWORD getDestTime() const { return m_dwDestTime; }

    SActProcess& getProcess() { return m_stProcess; }

    void addNpcPos(DWORD dwNpcID, const xPos& rPos) { m_mapNpcPos[dwNpcID] = rPos; }
    const xPos* getNpcPos(DWORD dwNpcID);

    void addActUser(QWORD qwCharID) { m_setActUserIDs.insert(qwCharID); }
    bool isActUser(QWORD qwCharID) { return m_setActUserIDs.find(qwCharID) != m_setActUserIDs.end(); }
  private:
    const SBossActCFG* m_pCFG = nullptr;

    DWORD m_dwActID = 0;
    DWORD m_dwBossID = 0;
    DWORD m_dwMapID = 0;
    DWORD m_dwLv = 0;
    DWORD m_dwStep = 0;
    DWORD m_dwDestTime = 0;

    SActProcess m_stProcess;

    std::map<DWORD, xPos> m_mapNpcPos;
    TSetQWORD m_setActUserIDs;
};

