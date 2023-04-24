/**
 * @file Menu.h
 * @brief function
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-07-10
 */

#pragma once

#include "RecordCmd.pb.h"
#include "SessionShop.pb.h"
#include "xDefine.h"
#include "xSingleton.h"
#include "MenuConfig.h"

using std::vector;
using namespace Cmd;

// menu
class SceneUser;
class Menu
{
  friend class GMCommandRuler;
  public:
    Menu(SceneUser* pUser);
    ~Menu();

    bool load(const BlobMenu &acc_data, const BlobMenu& char_data);
    bool save(BlobMenu *acc_data, BlobMenu* char_data);

    bool isOpen(DWORD id);

    void sendAllMenu();
    void refreshNewMenu(EMenuCond eCond = EMENUCOND_MIN);
    void open(DWORD id);
    void close(DWORD id);

    void checkBackoffSkillMenu();
  private:
    void collectValidMenu(TVecDWORD& vecMenus);
    void collectValidMenu(EMenuCond eCond, TVecDWORD& vecMenus);
    bool checkEnable(const SMenuCFG& rCFG);

    void version_update();
    void version_1();
  public:
    void processMenuEvent(DWORD id);
  private:
    SceneUser* m_pUser = nullptr;

    TSetDWORD m_setValidMenus;
};

