#pragma once
#include <list>
//#include "Define.h"
#include "xEntryManager.h"
#include "xSingleton.h"
#include "xCommand.h"

enum EFuncType
{
  EFUNCTYPE_SKILL = 0,
  EFUNCTYPE_BOOTH = 1,
};

class Robot;
class xNetProcessor;
class RobotManager : public xEntryManager<xEntryID>, public xSingleton<RobotManager>
{
  friend class xSingleton<RobotManager>;

  public:
    virtual ~RobotManager();
  private:
    RobotManager();

  public:
    void process();
    void timer();
    void onClose(xNetProcessor* np);
    void init();

  public:
    bool add(Robot *);
    bool del(Robot *);

  private:
    bool loadConfig();
    bool loadData();

  private:
    xTimer m_oOneSecTimer;
    xTime m_oFrameTimer;
    xTime m_oTickFrameTimer;

  private:
    string m_strProxyIp;
    DWORD m_dwProxyPort = 0;
    DWORD m_dwZoneId = 0;
    QWORD m_qwAccId = 0;
    DWORD m_dwCount = 0;
    DWORD m_dwMapID = 0;
    DWORD m_dwRange = 0;
    Cmd::ScenePos m_posCenter;
    EFuncType m_eFuncType = EFUNCTYPE_SKILL;

  public:
    string getProxyIp() { return m_strProxyIp; }
    DWORD getProxyPort() { return m_dwProxyPort; }
    DWORD getMapID() { return m_dwMapID; }
    EFuncType getFuncType() { return m_eFuncType; }
    void getScenePos(Cmd::ScenePos& pos);
};
