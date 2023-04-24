/**
 * @file Shortcut.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-11-04
 */

#pragma once

#include "RecordCmd.pb.h"
#include "xDefine.h"

using namespace Cmd;
using std::vector;
using std::set;

typedef vector<ShortcutItem> TVecShortcut;

class SceneUser;
class Shortcut
{
  public:
    Shortcut(SceneUser* pUser);
    ~Shortcut();

    bool load(const BlobShortcut& rData);
    bool save(BlobShortcut* pData);

    void refreshShortcut();
    void onItemAdd(const ItemInfo& rItem);
    void timer(DWORD curTime);

    void sendAllShortcut();
    bool putShortcut(const ShortcutItem& rItem);
  private:
    void initShortcut();

    ShortcutItem* getShortcutItem(DWORD dwPos);
  private:
    SceneUser* m_pUser = nullptr;

    TVecShortcut m_vecShortcut;
    TVecDWORD m_vecFirstAuto;

    set<DWORD> m_setChangePos;
};

