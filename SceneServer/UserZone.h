#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "config/MapConfig.h"

class SceneUser;
class Scene;
namespace Cmd
{
  class BlobZone;
};

class UserZone : private xNoncopyable
{
  public:
    UserZone(SceneUser *u);
    ~UserZone();

  public:
    void save(Cmd::BlobZone *data);
    void load(const Cmd::BlobZone &data);

  public:
    void gomap(DWORD zoneid, DWORD mapid, GoMapType type, const xPos& rPos = xPos(0, 0, 0));
    bool online(Scene *scene);

    QWORD getRoomId() { return m_qwRoomId; }
    void setRoomId(QWORD roomId) { m_qwRoomId = roomId; }
    DWORD getColorIndex() { return m_dwColorIndex; }
    void setColorIndex(DWORD color) { m_dwColorIndex = color; }
  private:
    DWORD m_dwMapID = 0;
    GoMapType m_oType = GoMapType::Null;

    DWORD m_dwEffectiveTime = 0;

    QWORD m_qwRoomId = 0;       //锞讹痉锞硷炯锞筹尽锞撅景锞凤究锞硷郡id
    DWORD m_dwColorIndex = 0;    
  public:
    DWORD getRaidZoneID(DWORD raid) const;
    void setRaidZoneID(DWORD raid, DWORD zoneid);
    void delRaidZoneID(DWORD raid);
  private:
    std::map<DWORD, DWORD> m_mapRaidZone;
    xPos m_oDestPos;
  private:
    SceneUser *m_pUser = nullptr;
};
