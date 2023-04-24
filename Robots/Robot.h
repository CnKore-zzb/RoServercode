#pragma once
//#include "Define.h"
#include "xEntry.h"
#include "RobotsServer.h"
#include "xTime.h"
#include "UserData.h"
#include "UserCmd.h"

class RobotManager;
class xNetProcessor;

struct xRobotCommand
{
  BYTE cmd;
  BYTE param;
  WORD noncelen = 0;
  BYTE noncedata[0];
  BYTE probuf[0];
};

enum class Robot_State
{
  create      = 0,
  running     = 1,
  quit        = 2,
};

enum class RobotGame_State
{
  create      = 0,
  login       = 1,
  quit        = 2,
};

class Robot : public xEntry
{
  friend class RobotManager;
  public:
    Robot(QWORD accid, DWORD reginid);
    virtual ~Robot();

  public:
    bool sendCmdToServer(void* cmd, unsigned short len);
    bool doServerCmd(xCommand *buf, unsigned short len);

    bool doLoginUserCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUser2Cmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneSkillCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneQuestCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserAchieveCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doInfiniteTowerCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doUserEventCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserItemCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserPhotoCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserFoodCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserAstrolabeCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSessionUserMailCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doActivityCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSessionUserGuildCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserMapCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doFubenCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSessionUserWeatherCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSessionUserSocialityCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserSealCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doErrRegUserCmd(Cmd::UserCmd* buf, unsigned short len);

  public:
    void timer(DWORD cur);
    void connectServer();

  private:
    xNetProcessor *m_pTask = nullptr;

  public:
    QWORD m_qwAccID = 0;
    DWORD m_dwRegionID = 0;
    // 已经连接的区
    DWORD m_dwZoneID = 0;
    // 记录选择的角色
    QWORD m_qwCharID = 0;

  public:
    Robot_State getState() { return m_oRobotState; }
    void setState(Robot_State state);
  private:
    Robot_State m_oRobotState = Robot_State::create;

  public:
    void setBoothOpen(bool open) { m_bBoothOpen = open; }
    bool hasBoothOpen() { return m_bBoothOpen; }

  private:
    DWORD m_dwNonceTime = 0;
    DWORD m_dwNonceIndex = 0;
    xTimer m_oTenSecTimer;
    bool m_bBoothOpen;  // 摆摊用
};
