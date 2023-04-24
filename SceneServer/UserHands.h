#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "xTime.h"

class SceneUser;
struct xPos;

namespace Cmd
{
  class BlobHands;
};
using std::string;

class UserHands : private xNoncopyable
{
  public:
    UserHands(SceneUser *u);
    ~UserHands();

  public:
    void save(Cmd::BlobHands *data);
    void load(const Cmd::BlobHands &data);

    void clear()
    {
      //m_bMaster = 0;
      m_qwOtherID = 0;
      m_bWaitBuild = false;
      //m_pOther = nullptr;
      //m_qwForceJoinUserID = 0;
    }
    // 牵手 主驾驶调用
    //bool join(SceneUser *user);
    // 分手
    void breakup(bool isOffLine = false);
    void breakByOther(bool isOffLine = false);

    void move(xPos p);

    inline bool has() { return m_qwOtherID != 0; }
    inline bool isInWait() { return m_bWaitBuild; }

    bool checkInTeam(QWORD charid);

    QWORD getMasterID();
    inline bool isMaster() { return m_bMaster; }
    QWORD getFollowerID();
    inline bool isFollower() { return has() && !m_bMaster; }

    void leaveScene();
    void enterScene();
    void userOnline();
    void userOffline();

    bool canBeAttack();

    void send(bool bBuild);

    void timeTick(DWORD curSec);
  public:

    SceneUser* getOther();
    //SceneUser* m_pOther = nullptr;
  public:
    void setWaitStatus() { m_bWaitBuild = true; }
    void setBuildStaus() { m_bWaitBuild = false; }
    void setMaster(QWORD id);
    void setFollower(QWORD id);
    QWORD getHandFollowID();
    QWORD getOtherID() { return m_qwOtherID; }

    void changeHandStatus(bool bBuild, QWORD otherid);

  private:
    //void startCalcHandTime(SceneUser* pMaster);
    void startCalcHandTime();
    void calcHandTime(DWORD curSec);
    void stopCalcHandTime(QWORD qwOtherID = 0);

  private:
    SceneUser *m_pUser = NULL;
    bool m_bWaitBuild = true;
    DWORD m_bMaster = 0;
    QWORD m_qwOtherID = 0;

    QWORD m_qwForceJoinUserID = 0;

    bool bCalcHandTimeing = false;

    xTimer m_oTimer;

  public:
    string m_strLastName;         //最近牵手的人名
    DWORD m_dwHandTimeLen = 0;    //累计牵手的时长
    DWORD m_dwRewardTimeLen = 0;  //是否领过奖励

    DWORD m_dwLastHandTime = 0;   //最近牵手的时间点
    DWORD m_dwOneMinTipsTime = 0;
};
