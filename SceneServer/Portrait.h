/**getCurPortrait
 * @file Portrait.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-07-27
 */

#pragma once

#include "PortraitConfig.h"

using namespace Cmd;
namespace Cmd
{
  class BlobPortrait;
};

// portrait
class SceneUser;
class Portrait
{
  public:
    Portrait(SceneUser* pUser);
    ~Portrait();

    bool load(const BlobPortrait &acc_data, const BlobPortrait& char_data);
    bool save(BlobPortrait *acc_data, BlobPortrait* char_data);

    DWORD getCurPortrait(bool isReal= false) const;
    DWORD getCurFrame() const { return m_dwCurFrame; }
    DWORD getPortraitCount(EPortraitType eType) const;

    bool checkAddItems(DWORD id);
    bool addNewItems(DWORD id, bool bNotify = true);

    void sendAllUnlockItems();
    void refreshEnableItems();

    bool usePortrait(DWORD id, bool bNotify = true);
    bool useFrame(DWORD id, bool bNotify = true);

    bool patch1();
  private:
    bool addNewPortrait(const SPortraitCFG* pCFG, bool bNotify);
    bool addNewFrame(const SPortraitCFG* pCFG, bool bNotify);
  private:
    SceneUser* m_pUser = nullptr;

    DWORD m_dwCurPortrait = 0;
    DWORD m_dwCurFrame = 0;

    TVecDWORD m_vecUnlockPortrait;
    TVecDWORD m_vecUnlockFrame;
};

