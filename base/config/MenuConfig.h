/**
 * @file MenuConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-17
 */

#pragma once

#include "xSingleton.h"
#include "TableManager.h"

using std::vector;
using std::string;

const DWORD MENUEVENT_UNLOCKSHOP_PARAM_COUNT = 5;

// config data
enum EMenuID
{
  EMENUID_SKILL = 1,
  EMENUID_MANUAL = 23,
  EMENUID_LABORATORY = 24,
  EMENUID_ENDLESSTOWER = 26,
  EMENUID_MANUAL_MONSTER = 28,
  EMENUID_MANUAL_CARD = 29,
  EMENUID_MANUAL_NPC = 30,
  EMENUID_MANUAL_FASHION = 31,
  EMENUID_MANUAL_MOUNT = 32,
  EMENUID_MANUAL_SCENERY = 34,
  EMENUID_KANJIMOCHAO = 36,
  EMENUID_FOOD = 75,
  EMENUID_FOOD_PACK = 77,
  EMENUID_PEAK_LEVEL = 450,
  EMENUID_DEAD = 451,
  EMENUID_PET_WORK_UNLOCK = 1907,
  EMENUID_PET_WORK_MANUAL = 8101,
  EMENUID_TUTOR_STUDENT = 9000,
  EMENUID_TUTOR_TUTOR = 9001,
  EMENUID_WEDDING_RESERVE = 6000,     //¶©»é¹¦ÄÜ
  EMENUID_SERVANT = 3050,
  EMENUID_QUEST_MANUAL = 3051,
  EMENUID_CLOTHCOLOR = 9300,
  EMENUID_PROFESSION = 9006,
};
enum EMenuCond
{
  EMENUCOND_MIN = 0,
  EMENUCOND_BASE_LEVEL,
  EMENUCOND_JOB_LEVEL,
  EMENUCOND_OTHER,
  EMENUCOND_INGUILD,
  EMENUCOND_QUEST,
  EMENUCOND_SKILL,
  EMENUCOND_ACHIEVE,
  EMENUCOND_PET,
  EMENUCOND_TITLE,
  EMENUCOND_MANUALGROUP,
  EMENUCOND_MANUALUNLOCK,
  EMENUCOND_COOKLV,
  EMENUCOND_UNLOCK,
  EMENUCOND_EVO,
  EMENUCOND_TOWERLAYER,
  EMENUCOND_MANUALLEVEL,
  EMENUCOND_PVPCOIN,
  EMENUCOND_WEDDING,
  EMENUCOND_PROFESSION,
  EMENUCOND_TEAMPWS,
  EMENUCOND_MAX,
};
struct SMenuCondition
{
  EMenuCond eCond = EMENUCOND_MIN;

  TVecDWORD vecParams;

  SMenuCondition() {}
};
enum EMenuEvent
{
  EMENUEVENT_MIN = 0,
  EMENUEVENT_SKILLGRID = 1,
  EMENUEVENT_UNLOCKSHOP = 2,
  EMENUEVENT_UNLOCKMANUAL = 3,
  EMENUEVENT_SCENERY = 4,
  EMENUEVENT_AUTOSKILL = 5,
  EMENUEVENT_UNLOCKACTION = 6,
  EMENUEVENT_UNLOCKEXPRESSION = 7,
  EMENUEVENT_UNLOCKHAIR = 8,
  EMENUEVENT_EXTENDSKILL = 9,
  EMENUEVENT_SEENPC = 10,
  EMENUEVENT_HIDENPC = 11,
  EMENUEVENT_UNLOCKCATNUM = 12,
  EMENUEVENT_UNLOCKCATID = 13,
  EMENUEVENT_UNLOCKEYE = 14,
  EMENUEVENT_ADDMAXJOBLEVEL = 15,
  EMENUEVENT_ADDSHOPCNT = 16,
  EMENUEVENT_ADDITEM = 17,
  EMENUEVENT_MAX
};
enum EMenuShopEvent
{
  EMENUSHOPEVENT_SHOP = 0,
  EMENUSHOPEVENT_MANUAL = 1,
  EMENUSHOPEVENT_ALL = 2
};
struct SMenuEvent
{
  EMenuEvent eEvent = EMENUEVENT_MIN;

  TVecDWORD vecParams;

  SMenuEvent() {}
};
struct SMenuCFG : public SBaseCFG
{
  DWORD id = 0;

  bool acc = false;

  SMenuCondition stCondition;
  SMenuEvent stEvent;

  SMenuCFG()
  {
    SBaseCFG();
  }
};
typedef vector<SMenuCFG> TVecMenuCFG;
typedef map<DWORD, SMenuCFG> TMapMenuCFG;
typedef map<EMenuCond, TVecMenuCFG> TMapCondMenuCFG;
typedef map<EMenuEvent, TVecMenuCFG> TMapEventMenuCFG;

// config
class MenuConfig : public xSingleton<MenuConfig>
{
  public:
    MenuConfig();
    virtual ~MenuConfig();

    bool loadConfig();
    bool checkConfig();

    const SMenuCFG* getMenuCFG(DWORD id);
    const TMapMenuCFG& getMenuList() const { return m_mapMenuCFG; }
    const TVecMenuCFG* getCondMenuList(EMenuCond eCond) const;
    const TVecMenuCFG* getEventMenuList(EMenuEvent eEvent) const;

    DWORD getMenuIDByQuestID(DWORD dwQuestID) const;
  private:
    EMenuEvent getMenuEvent(const string& str) const;
  private:
    TMapMenuCFG m_mapMenuCFG;
    TMapCondMenuCFG m_mapCondMenuCFG;
    TMapEventMenuCFG m_mapEventMenuCFG;

    map<DWORD, DWORD> m_mapQuest2Menu;
};

