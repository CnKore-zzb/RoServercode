/**
 * @file Hair.h
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-24
 */

#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "TableStruct.h"

namespace Cmd
{
  class BlobHair;
};

class SceneUser;
class Hair
{
  public:
    Hair(SceneUser* pUser);
    ~Hair();

    bool load(const BlobHair& data);
    bool save(BlobHair *data);

    DWORD getCurHair(bool bReal = false);
    DWORD getCurHairColor();

    DWORD getRealHair() const { return m_dwCurHair; }
    DWORD getRealHairColor() const { return m_dwCurColor; }

    DWORD getHairCount() const { return m_setUnlockHair.size(); }

    void sendAllUnlockHairs();
    void resetHair();

    bool checkAddHair(DWORD id);
    bool addNewHair(DWORD id);
    bool useColor(DWORD id, bool bRealUse = true);
    bool useHair(DWORD id, bool bRealUse = true);

    bool useHairFree(DWORD id);
    bool useColorFree(DWORD id);

    bool hasHair(DWORD id) const;
  private:
    SceneUser* m_pUser = nullptr;

    DWORD m_dwCurHair = 0;
    DWORD m_dwCurColor = 0;
    std::pair<DWORD, DWORD> m_pairHair;
    std::pair<DWORD, DWORD> m_pairHairColor;
    TSetDWORD m_setUnlockHair;
};
