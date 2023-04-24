/**
 * @file SceneItemManager.h
 * @brief map item
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-05-13
 */

#pragma once

#include "xSingleton.h"
#include "xEntryManager.h"
#include "SceneItem.h"

class Scene;
class SceneUser;
class SceneItem;

class SceneItemManager: private xEntryManager<xEntryTempID>, public xSingleton<SceneItemManager>
{
  friend class xSingleton<SceneItemManager>;

  public:
    SceneItemManager();
    virtual ~SceneItemManager();

    using xEntryManager<xEntryTempID>::size;

    bool init();
    void final();
    void timer(QWORD curMSec);

    SceneItem* getSceneItem(QWORD tempid);
    SceneItem* createSceneItem(Scene* scene, const ItemInfo& rInfo, const xPos& rPos);

    bool pickupSceneItem(SceneUser* pUser, QWORD tempid);
  private:
    bool addItem(SceneItem* item);
    void delItem(SceneItem* item);

    void handleMerchantSkill(SceneUser* pUser, const SceneItem* pItem, DWORD realCount);
  private:
    QWORD m_qwPickupTick = 0;
};

