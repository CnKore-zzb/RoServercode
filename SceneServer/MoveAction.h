#pragma once

#include <memory>
#include <list>
#include "xDefine.h"
#include "xPos.h"
#include "xTime.h"

#define SYNC_POS_DIS 5

class xSceneEntryDynamic;
struct ExitPoint;

class MoveAction
{
  public:
    MoveAction(xSceneEntryDynamic* entry);
    ~MoveAction();

  private:
    xSceneEntryDynamic *m_pEntry;

  public:
    void heartbeat(QWORD curMSec);
    bool setStraightPoint(xPos p);
    xPos getStraightPoint();
    bool setFinalPoint(xPos p);
    xPos getDestPos() { return m_destFinal; }
    xPos getLastDestPos() { return m_lastDestFinal; }
    bool forceSync();
    void moveOnce();/*执行一次有效移动*/
  public:
    bool empty()
    {
      return m_path.empty() && m_fixpath.empty();
    }
    void clearPath();
    void stop();
    bool action(QWORD curMSec);
    void stopAtonce();

    void clear()
    {
      m_path.clear();
      m_fixpath.clear();
      m_qwNextActionTime = 0;
    }

  private:
    void move(xPos pos);
    void sendPos(xPos pos);
    xPos m_dest;
    xPos m_destFinal;
    xPos m_lastDestFinal;

    std::list<xPos> m_path;
    std::list<xPos> m_fixpath;

  public:
    bool checkActionTime()
    {
      return (xTime::getCurUSec()/1000) >= m_qwNextActionTime;
    }
    bool checkActionTime(QWORD cur) const
    {
      return cur >= m_qwNextActionTime;
    }
    void addActionTime(DWORD speed);
    void addBeHitDelayTime(DWORD speed);
  private:
    QWORD getNextActionTime()
    {
      return m_qwNextActionTime;
    }
    void addNextActionTime(QWORD add)
    {
      m_qwNextActionTime += add;
    }
    void setNextActionTime(QWORD add)
    {
      m_qwNextActionTime = add;
    }
    QWORD m_qwNextActionTime = 0;

  public:
    void setExit(DWORD id, xPos &p)
    {
      m_dwExitID = id;
      m_oExitPos = p;
    }
    void checkExit();
    void doExit(const ExitPoint &point);

  private:
    DWORD m_dwExitID;
    xPos m_oExitPos;
    std::pair<QWORD, QWORD> m_oPairBegin2EndHitDelay;
};
