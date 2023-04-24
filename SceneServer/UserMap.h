#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "xPos.h"

class SceneUser;

class UserMap : private xNoncopyable
{
  public:
    UserMap(SceneUser *user);
    virtual ~UserMap();

  public:
    void save();
    void load();

  public:
    void setLoginMapID(DWORD id)
    {
      m_dwLoginMapID = id;
    }
    DWORD getLoginMapID()
    {
      return m_dwLoginMapID;
    }
  private:
    DWORD m_dwLoginMapID = 0;    // 登录时会话同步的mapid

    // 最后一次静态地图
  public:
    void setLastStaticMap(DWORD id, const xPos &pos);
    DWORD getLastStaticMapID() const;
    const xPos& getLastStaticMapPos() const;

  public:
    void gotoLastStaticMap();

  private:
    SceneUser *m_pUser = nullptr;
};
