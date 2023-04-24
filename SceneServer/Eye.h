/**
 * @file Eye.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-09-20
 */

#pragma once

#include "xDefine.h"
#include "RecordCmd.pb.h"

using namespace Cmd;

class SceneUser;
class Eye
{
  public:
    Eye(SceneUser* pUser);
    ~Eye();

    bool load(const BlobEye& rBlob);
    bool save(BlobEye* pBlob);

    DWORD getCurID(bool bReal = false) const;
    bool hasEye(DWORD dwID) const { return m_setUnlockIDs.find(dwID) != m_setUnlockIDs.end(); }

    void sendUnlockList() const;
    void resetEye();

    bool checkAddEye(DWORD dwID);
    bool addNewEye(DWORD dwID);
    bool useEye(DWORD dwID, bool bRealUse = true);
  private:
    void initDefault();
  private:
    SceneUser* m_pUser = nullptr;

    DWORD m_dwCurID = 0;
    std::pair<DWORD, DWORD> m_pairDressUpID;
    TSetDWORD m_setUnlockIDs;
};

