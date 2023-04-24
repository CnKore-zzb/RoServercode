/**
 * @file SceneItem.h
 * @brief map item
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-05-12
 */

#pragma once

#include <vector>
#include "xSceneEntryDynamic.h"
#include "SceneUser.pb.h"
#include "SceneMap.pb.h"

using std::vector;
using std::find;
using namespace Cmd;

const DWORD ITEM_DISAPPEARD_DEFAULT_TIME = 60;
const DWORD ITEM_OWNER_DEFAULT_TIME = 10;
const DWORD ITEM_TIME_PARAM = 1000;

enum ESceneItemStatus
{
  ESCENEITEMSTATUS_VALID,
  ESCENEITEMSTATUS_INVALID
};

enum EItemPickUpMode
{
  EITEMPICKUP_NORMAL = 1,
  EITEMPICKUP_BUFF = 2,
};

class Scene;
class SceneUser;
class SceneItem : public xSceneEntry
{
  public:
    SceneItem(QWORD tempid, Scene *scene, const ItemInfo& rInfo, const xPos& rPos);
    virtual ~SceneItem();

    virtual SCENE_ENTRY_TYPE getEntryType() const { return SCENE_ENTRY_ITEM; }

    virtual void delMeToNine();
    virtual void sendMeToNine();

    void setClearState();

    const ItemInfo& getItemInfo() const { return m_stInfo; }
    //void setStatus(ESceneItemStatus eStatus) { m_eStatus = eStatus; }
    ESceneItemStatus getStatus() const { return m_eStatus; }

    bool canPickup(SceneUser* pUser, EItemPickUpMode eMode = EITEMPICKUP_NORMAL);

    bool enterScene(Scene* scene);
    void leaveScene();

    void addOwner(QWORD ownerid);

    void fillMapItemData(MapItem* data, DWORD extraTime = 0);
    void timer(QWORD curMSec);

    bool viewByUser(QWORD userid);
    void setSourceID(QWORD sourceid) { m_dwSourceID = sourceid; }
    void setViewLimit() { m_dwIsViewLimit = true; }
    void setDispTime(DWORD time) { m_dwDisappearTime = time; }
    void setOwnTime(DWORD time) { m_dwOwnerTime = time; }
    DWORD getOwnTime() const { return m_dwOwnerTime; }
    ENpcType getNpcType() const { return m_npcType; }
    const TVecQWORD& getOwners() const { return m_vecOwners; }
  private:
    void bePickup();
  private:
    ItemInfo m_stInfo;
    TVecQWORD m_vecOwners;

    DWORD m_dwDisappearTime = 0;
    DWORD m_dwOwnerTime = 0;
    DWORD m_dwTimeTick = 0;

    QWORD m_dwSourceID = 0;
    bool m_dwIsViewLimit = false;

    ESceneItemStatus m_eStatus = ESCENEITEMSTATUS_VALID;
    ENpcType m_npcType = ENPCTYPE_MIN;
};

