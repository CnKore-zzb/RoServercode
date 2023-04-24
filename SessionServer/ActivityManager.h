#pragma once

#include "xSingleton.h"
#include "SessionSociality.pb.h"
#include "SessionUser.h"
#include "SessionCmd.pb.h"

using namespace Cmd;

class ActivityManager : public xSingleton<ActivityManager>
{
  friend class xSingleton<ActivityManager>;
private:
  ActivityManager();
public:
  virtual ~ActivityManager();

  bool load();
  void updateActivity();
  void updateShortcutPower();
  void notifyUser(SessionUser* pUser);
  void notifyAllUser();
  void onUserOnline(SessionUser* pUser) { notifyUser(pUser); }

private:
  OperActivityNtfSocialCmd m_oActivity;
};

// 活动模板
class ActivityEventManager : public xSingleton<ActivityEventManager>
{
  friend class xSingleton<ActivityEventManager>;
private:
  ActivityEventManager() {}
public:
  virtual ~ActivityEventManager() {}

  bool load();
  void updateEvent();
  void notifyUser(SessionUser* pUser);
  void notifyAllUser();
  void notifyScene(ServerTask* task);
  void notifyAllScene();
  void onUserOnline(SessionUser* pUser) { notifyUser(pUser); }

private:
  ActivityEventNtfSessionCmd m_oAEToScene;
  ActivityEventNtf m_oAEToClient;
};
