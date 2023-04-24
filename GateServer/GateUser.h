#pragma once
//#include "Define.h"
#include "xEntry.h"
#include "UserData.h"
#include "UserCmd.h"
#include "GateServer.h"
#include "xTime.h"
#include "UserData.h"

class GateUserManager;
class ServerTask;
class xNetProcessor;

using namespace Cmd;

enum class GateUser_State
{
  create      = 0,
  login       = 1,    // 连上服务器
  select_role = 2,    // 选择角色 等待返回
  running     = 3,    // 进入场景
  quit        = 4,    // 进入退出状态
};

class CmdQueue : public xCmdQueue
{
  public:
    CmdQueue()
    {
    }
    virtual ~CmdQueue()
    {
    }
    inline DWORD empty() { return !m_dwSize; }
    bool isFull()
    {
      return m_dwSize >= 10;
    }
    DWORD m_dwSize = 0;
};

class GateUser : public xEntry
{
  public:
    GateUser(ACCID accid, xNetProcessor* client_task);
    virtual ~GateUser();

    bool init(ServerTask *scene,const GateUserData& data);

    bool sendCmdToMe(void* cmd, unsigned short len);
    bool doUserCmd(Cmd::UserCmd *cmd, unsigned short len);
    bool doLoginUserCmd(Cmd::UserCmd* buf, unsigned short len);
    bool doSceneUserCmd(const Cmd::UserCmd* buf, unsigned short len);
    bool doUserEventCmd(const Cmd::UserCmd* buf, unsigned short len);
    bool doWeddingUserCmd(const Cmd::UserCmd* buf, unsigned short len);

    bool doTowerUserCmd(const Cmd::UserCmd* buf, unsigned short len);
    bool doDojoUserCmd(const Cmd::UserCmd* buf, unsigned short len);
    bool doChatUserCmd(const Cmd::UserCmd* buf, unsigned short len);
    bool doGuildCmd(const Cmd::UserCmd* buf, unsigned short len);
    bool doPveCardCmd(const Cmd::UserCmd* buf, unsigned short len);
    bool doTeamRaidCmd(const Cmd::UserCmd* buf, unsigned short len);

    bool forwardScene(const Cmd::UserCmd* cmd, unsigned short len);
    bool forwardSession(const Cmd::UserCmd* cmd, unsigned short len);
  public:
    DWORD getIP()
    {
      if (!client_task()) return 0;
      return *(DWORD *)(&client_task()->getIP());
    }

    //定时器
  public:
    void timer(DWORD cur);

  public:
    void set_scene_task(ServerTask *scene_task) { scene_task_ = scene_task; }
    ServerTask* scene_task() { return scene_task_; }
    void set_client_task(xNetProcessor *client_task) { client_task_ = client_task; }
    xNetProcessor* client_task() { return client_task_; }
  private:
    ServerTask *scene_task_;
    xNetProcessor *client_task_;

  public:
    ACCID accid = 0;

  public:
    GateUser_State getState() { return m_oGateUserState; }
    void setState(GateUser_State state);
  private:
    GateUser_State m_oGateUserState;

  public:
    QWORD m_qwTeamIndex = 0;

  public:
    void sendAllCmd();
  private:
    CmdQueue m_oCmds;

  public:
    xTime m_oLoginTimer;   // 统计登录时间

    // 消息频率大小统计
  public:
    void addCmdCounter(DWORD cmd, DWORD param);
    void printCmdCounter();
    void syncServerTime2Client();
  private:
    std::map<DWORD, DWORD> m_oCmdCounter;
    DWORD m_dwTotalCount = 0;
    xTimer m_oTickTwentySec;

  public:
    std::string m_strProxyIP;
};
