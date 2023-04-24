#pragma once

#include "xDefine.h"
#include "xPos.h"
#include <list>
#include <bitset>
#include "ScenePet.pb.h"
#include "RecordCmd.pb.h"
#include "WeaponPetConfig.h"

using namespace Cmd;
using std::map;
using std::list;
using std::bitset;
using std::string;

class Scene;
class SceneUser;

enum EWPPetState
{
  EWPPETSTATAE_NORAML = 1,       // 正常跟随雇主战斗或移动
  EWPPETSTATAE_WAIT_BATTLE = 2,  // 雇主停止战斗一段时间, 等待雇主或队友战斗消息, (切回1) 2->1 or（切到3) 2->3
  EWPPETSTATAE_TEAMER = 3,       // 跟随队友战斗或移动
  EWPPETSTATAE_HAND = 4,         // 建立牵手状态
};

struct SWeaponPetData
{
  DWORD dwID = 0;
  DWORD dwHp = 0;
  DWORD dwReliveTime = 0;
  DWORD dwExpireTime = 0;

  bool bLive = false; // 未进入休整期
  bool bActive = false; // 在队伍中
  bool bInvalid = false; // 雇佣到期, 仅存在列表中用于前端展示

  bool bClearStatus = false; // 等待删除
  //QWORD qwTempID = 0;
  //const SWeaponPetCFG* pCFG = nullptr;
  string strName;

  void toData(WeaponPetData* pData);
  void fromData(const WeaponPetData& rData);
  void toData(MemberCat* pCat, SceneUser* pUser) const;
  void toData(TeamMember* pMember, SceneUser* pUser) const;
  QWORD getTempID(QWORD userid) const { return userid * ONE_THOUSAND + dwID; }

  DWORD dwTempEndBattleTime = 0;
  bool notIn() const {return !bLive || !bActive || bInvalid; }

  bitset<EMEMBERDATA_MAX> obitset;
  void setMark(EMemberData eData) { obitset.set(eData); }

  DWORD dwLastBattleTime = 0;
  QWORD qwFollowTeamerID = 0;
  EWPPetState eState = EWPPETSTATAE_NORAML;
  //void changeState(EWPPetState newState) { if (eState != EWPPETSTATAE_TEAMER) qwFollowTeamerID = 0; eState = newState; }

  bool bInHandStatus = false;
};
typedef list<SWeaponPetData> TListPetData;

class xSceneEntryDynamic;
class SceneNpc;
class WeaponPet
{
  public:
    WeaponPet(SceneUser* pUser);
    virtual ~WeaponPet();

  public:
    void load(const BlobWeaponPet& data);
    void save(BlobWeaponPet* pPet);

    void toData(MemberCatUpdateTeam& cmd);
  public:
    bool checkAddMoney(DWORD id, EEmployType eType, bool& bNeedZeny, DWORD&zenyCount, DWORD& dwCostItemId, DWORD& dwCostItemNum);
    bool add(DWORD id, EEmployType eType);
    bool del(DWORD id);
    bool enable(DWORD id);
    bool disable(DWORD id, bool bToRest = true);
    bool has() const { return m_oListPetData.empty() == false; }
    bool hasActive() const;
    const TListPetData& getList() const { return m_oListPetData; }
    const SWeaponPetData* getData(DWORD dwID) const;
    void getPetNpcs(std::list<SceneNpc*>& list) const;
    void broadcastCmdToTeam(void* buf, WORD len);
    void updateDataToTeam();
    void updateDataToUser(QWORD qwTargetID);
    void setMark(DWORD id, EMemberData eData);
    void queryCatPrice(QueryOtherData& cmd);
    DWORD getMaxExpireTime() const;
  private:
    void fetchTeamData(SWeaponPetData& rData, MemberDataUpdate& cmd);
  public:
    void timer(DWORD curSec);
  public:
    void onUserMove(const xPos& dest);
    void onUserGoTo(const xPos& dest);
    void onUserAttack(xSceneEntryDynamic* enemy);
    void onUserEnterScene();
    void onUserLeaveScene();
    void onNpcDie(SceneNpc* npc);
    void onUserDie();
    void onUserRelive();
    void onUserAction(DWORD id);
    void onAdd(const SWeaponPetData& rData);
    void onEnterOwnTeam(bool isLeaveTeam = false);
    void onEnterTeam(bool isEnterTeam, bool isOnline);
    void onUserLevelUp();
    void onTeamerAttack(SceneUser* pTeamer, xSceneEntryDynamic* enemy);
    void onUserMoveTo(const xPos& dest);
    void onEnterTeamFail(DWORD id);
    void onUserStopAction();

  public:
    //bool checkSize() const { return m_oListPetData.size() < m_dwMaxPetSize; }
    DWORD getUnlockCount() const { return m_setUnlockIDs.size(); }
    bool checkUnlock(DWORD id) const { return m_setUnlockIDs.find(id) != m_setUnlockIDs.end(); }
    bool checkUnConflict(DWORD id) const;
    bool checkCanAdd(DWORD id) const;
    bool checkCanActive(DWORD id) const;
    bool checkSize(DWORD id) const;

    void unlock(DWORD id);
    void setMaxSize(DWORD size);
    void decMaxSize();
    DWORD getMaxSize() const { return m_dwMaxPetSize; }
    bool isMyPet(QWORD id) const;
  public:
    void inviteHand(QWORD qwCatGUID);
    void breakHand(QWORD qwCatGUID);
    void breakHand();
    bool haveHandCat() const;
    bool isBuildHand(QWORD qwCatGUID) const;
    void leaveHand(QWORD qwCatGUID);
  public:
    void setExpire(DWORD id); /* 测试使用, 设置猫到期状态*/
  private:
    void enterScene(SWeaponPetData& stData);
    void leaveScene(SWeaponPetData& stData);
    void onDel(SWeaponPetData& stData, bool bRealDel = false);
    void onRlive(SWeaponPetData& stData);
    void enterRest(SWeaponPetData& stData);
    void checkTeam();
  private:
    //bool getMoveList(const xPos& dest, std::list<xPos>& poslist);
    bool onMove(const xPos& dest, const SWeaponPetData& stData, DWORD index);
    void changeState(SWeaponPetData& stData, EWPPetState eState);
    void sendHandStatus(bool bBuild, QWORD qwCatGUID);
    void sendHandBreak(QWORD qwCatGUID);
    void sendHandMark(QWORD qwCatGUID); /*通知前端牵手状态, 用于图标展示*/
  private:
    TListPetData m_oListPetData;
    SceneUser* m_pUser = nullptr;
    DWORD m_dwMaxPetSize = 0;
    TSetDWORD m_setUnlockIDs;
};
