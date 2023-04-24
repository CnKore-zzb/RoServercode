#pragma once
#include <deque>
//#include "Define.h"
#include "xEntryManager.h"
#include "xSingleton.h"
#include "xCommand.h"

class GateUser;
class xNetProcessor;

class GateUserManager : public xEntryManager<xEntryTempID>, public xSingleton<GateUserManager>
{
  public:
    GateUserManager();
    virtual ~GateUserManager();

  private:
    void final();

  public:
    bool addUser(GateUser *user);
    void delUser(GateUser *user);
    void onUserQuit(GateUser *user);
    void kickUserOnScene(xNetProcessor *np);

    // 请求退出
    void loginOut(GateUser *user, bool bDelete = false);
    void kickUser(GateUser *pUser);

    GateUser* getUserByAccID(QWORD accid);
    bool timer(DWORD cur);
    void process();

    void sendAllCmd(void *data, DWORD len);

  public:
    GateUser* newLoginUser(xNetProcessor* np, QWORD accid, const std::string& version);
    GateUser* getLoginUserByAccID(QWORD accid);
    bool addLoginUser(GateUser *user);
    void delLoginUser(GateUser *user);
  private:
    std::map<QWORD, GateUser *> loginlist;

  private:
    void addWaitForDelUser(GateUser *user);
  private:
    std::deque<std::pair<DWORD, GateUser *> > waitForDelUser;
};
