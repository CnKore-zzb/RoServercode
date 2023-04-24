/**
 * @file StatMgr.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-09-06
 */

#pragma once

#include "xSingleton.h"
#include "GameStruct.h"

class SceneUser;

class StatMgr : public xSingleton<StatMgr>
{
  friend class xSingleton<StatMgr>;
  private:
    StatMgr() {}
  public:
    virtual ~StatMgr() {}

    void onItemUse(SceneUser* pUser, const Cmd::ItemInfo& rInfo);

    void timer(DWORD curSec);
  private:
    void flushPetWear();
  private:
    TMapPetWearStat m_mapPetWearStat;
    DWORD m_dwFlushTick = 0;
};

