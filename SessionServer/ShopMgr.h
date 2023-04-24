#pragma once

#include "xSingleton.h"
#include "TableStruct.h"
#include "SessionCmd.pb.h"
#include <list>

class SessionScene;
class SessionUser;
class ServerTask;

class ShopMgr:public xSingleton<ShopMgr>
{
  public:
    ShopMgr();
    virtual ~ShopMgr();
    
    void load();
    void save();
    void refresh(DWORD curSec);
    void timeTick(DWORD curSec);
    void syncToSceneServer(ServerTask *task =nullptr);
    void sceneStart(ServerTask *task);
  private:
    DWORD m_dwRefreshTime = 0;
    std::list<DWORD> m_vecOldGroupId;
    DWORD m_curGoupId = 0;
};
