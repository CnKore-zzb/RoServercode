/**
 * @file AchieveProcess.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-07-10
 */

#pragma once

#include "xSingleton.h"
#include "GuildConfig.h"

class SceneUser;
struct SAchieveCFG;

class AchieveProcess : public xSingleton<AchieveProcess>
{
  friend class xSingleton<AchieveProcess>;
  private:
    AchieveProcess();
  public:
    virtual ~AchieveProcess();

    DWORD getProcess(SceneUser* pUser, const SAchieveCFG&, const TVecQWORD& vecParam, DWORD dwOri, DWORD dwAdd);
  private:
    DWORD process_level(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_attr(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_friend(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_music(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_ferriswheel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_hand(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_emoji(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_expression(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_photoman(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_photomonster(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_body(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_rune(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_portrait(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_hair(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_dojo(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_item(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_monsterphoto(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_npccount(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_scenerycount(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_scenery(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_monsterdraw(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_pvp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_mvp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_cat(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_dead(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_manual(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_quest(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_charge(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_refine(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_tower(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_seat(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_travel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_cookfood(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_eatfood(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_foodmateriallvup(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_cookerlvup(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_tasterlvup(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_costsavehp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_costsavesp(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_foodcooklv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_collection(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_petadventurefinish(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_petbaselevel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_petfriendlevel(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_petequip(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_tutor(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_atkpraylv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_defpraylv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_elempraylv(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_profession(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
    DWORD process_quest_submit(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
  private:
    DWORD getPrayMaxLv(EPrayType eType, SceneUser* pUser);
    DWORD process_wedding(SceneUser* pUser, const SAchieveCFG& rCFG, const TVecQWORD& vecParams, DWORD dwOri, DWORD dwAdd);
};

