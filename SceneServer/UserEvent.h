#pragma once

#include "xDefine.h"
#include "SceneItem.pb.h"
#include "SessionCmd.pb.h"
#include "SceneUser2.pb.h"
#include "xDefine.h"

class SceneUser;
class SceneNpc;
class ItemBase;
class BaseSkill;

using std::string;
using std::vector;
using std::pair;
using namespace Cmd;
/**
* @brief 用于以玩家为中心的各系统间事件通知解耦合使N * N的依赖关系简化为N * 1 * N
*/
class UserEvent
{
  public:
    UserEvent(SceneUser* user);
    ~UserEvent();

    void onLogin();
    void onUseSkill(const BaseSkill* pSkill);
    void onKillNpc(SceneNpc* pNpc, bool bSelf, bool count);
    void onEnterScene();
    void onLeaveScene();
    void onMove(float dis);
    void onVisitNpc(SceneNpc *npc);
    void onEnterSceneEnd();
    //其他事件

    void onItemAdd(const ItemInfo& rInfo);
    void onItemAdd(const string& guid, DWORD itemID, ESource source);
    void onPassRaid(DWORD raidid, bool bSuccess);
    void onBaseLevelup(DWORD newLv, DWORD oldLv);
    void onJobLevelup(DWORD oldLv, DWORD newLv);
    void onQuestSubmit(DWORD questid);
    void onEquipChange(EEquipOper oper, const string& guid);
    void onEquipExchange(const string& guidOld, const string& guidNew);
    void onCardChange(DWORD dwTypeID, bool isAdd);
    void onProfesChange(EProfession oldProfes);
    void onMountChange();
    void onMenuOpen(DWORD id);
    void onRolePointChange();
    void onSkillPointChange();
    void onAction(EUserActionType eType, QWORD id);
    void onAttrChange();
    void onRelive(EReliveType eType);
    void onCameraChange();
    void onFocusNpc(const TVecQWORD& vecIDs);
    void onBeBreakSkill(QWORD attackerid);
    void onStatusChange();
    void onEquipStrength(const string& guid);
    void onBeBuffStatus(DWORD status);
    void onUserDie();
    void onItemDelte(const ItemBase* pItem);
    void onEnterGuild(DWORD qwGuildID, bool bCreate);
    void onOpenCamera();
    void onPlayMusic(DWORD dwMusciId);
    void onAddSkill(DWORD skillId);
    void onDelSkill(DWORD skillId);
    void onTrigNpcFunction(QWORD npcguid, DWORD funcid);
    void onUnlockRuneSpecial(DWORD specID);
    void onUnlockRune(DWORD id, bool bSpecial);
    void onResetRuneSpecial(DWORD specID);
    void onItemUsed(DWORD itemid, DWORD count);
    void onItemBeUsed(DWORD itemid);
    void onSkillLevelUp(DWORD skillid);
    void onSkillReset();
    void onEnterGVG();
    void onPetHatch();
    void onReceiveEventMail(const UserEventMailCmd& event);
    void onPassTower();
    void onManualLevelup();
    void onMoneyChange(EMoneyType eType);
    void onChangeGender();
    void onProfessionChange(Cmd::EProfession eProfessionOld); // 转职
    void onProfessionAdd();
    void onLoadRecord();
    void onAddBattleTimeInBattle(DWORD dwTime);
    void onBoothOpen();
    void onWeddingUpdate();
  private:
    SceneUser* m_pUser = nullptr;
    DWORD m_dwCameraTimetick = 0;
};

