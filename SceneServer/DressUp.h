/**
 * @file Servant.h
 * @brief 
 * @author liangyongqiang, liangyongqiang@xindong.com
 * @version v1
 * @date 2018-07-17
 */

#pragma once

#include "xDefine.h"
#include "UserData.h"
#include "SceneItem.pb.h"
#include "SceneUser2.pb.h"
#include "DressUpStageMgr.h"

using std::map;

using namespace Cmd;

namespace Cmd
{
  class BlobDressUp;
};

class SceneUser;

class DressUp
{
  public:
    DressUp(SceneUser* pUser);
    ~DressUp();

    DWORD getDressUpStatus() const { return m_dwDressUpStatus; }
    void setDressUpStatus(DWORD status, UserStageInfo* pInfo, QueryStageUserCmd& pCmd);
    DWORD getDressUpEquipID(EEquipPos pos);
    bool addDressUpEquipID(EEquipPos pos, DWORD dwItemID);
    void equipOffAll();
    void equipOnAll();
    void onLeaveScene();
    void onLeaveStage();

    DWORD getUserStageID() const { return m_dwStageID; }
    void sendWaitUserInfo(const UserStageInfo* pInfo);
    void initStageAppearance(const UserStageInfo* pInfo);

    DWORD m_dwStageID = 0;
  private:
    SceneUser* m_pUser = nullptr;

    DWORD m_dwDressUpStatus = 0;
    map<EEquipPos, DWORD> m_mapDressUpEquip;
    map<EEquipPos, DWORD> m_mapDressUpTempEquip;
    map<EEquipPos, string> m_mapUserEquip;
};
