#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "xTime.h"
#include "RecordCmd.pb.h"
#include <list>
#include <bitset>
#include "xPos.h"
#include "Var.h"
#include "Item.h"
#include "PetConfig.h"

using namespace Cmd;
using std::list;
using std::string;
using std::bitset;
using std::set;

class xSceneEntryDynamic;
class SceneUser;
class SceneNpc;

const DWORD EGG_VERSION = 4;

enum EPetState
{
  EPETSTATAE_NORAML = 1,
  EPETSTATAE_WAIT_BATTLE = 2,
  EPETSTATAE_HAND = 3,
  EPETSTATAE_EATFOOD = 4,
  EPETSTATAE_BETOUCH = 5,
};

struct SScenePetData
{
  // record data
  QWORD qwExp = 0;
  QWORD qwFriendExp = 0;
  QWORD qwRewardExp = 0;

  DWORD dwID = 0;
  DWORD dwLv = 0;
  DWORD dwFriendLv = 0;
  DWORD dwBody = 0;
  DWORD dwReliveTime = 0;
  DWORD dwHp = 0;

  DWORD dwRestoreTime = 0;
  DWORD dwTimeHapply = 0;
  DWORD dwTimeExcite = 0;
  DWORD dwTimeHappiness = 0;
  DWORD dwTimeHapplyGift = 0;
  DWORD dwTimeExciteGift = 0;
  DWORD dwTimeHappinessGift = 0;

  DWORD dwTouchTick = 0;
  DWORD dwFeedTick = 0;

  string strGUID;
  string strName;
  Variable oVar;

  TSetDWORD setSkillIDs;

  ItemEquip* pEquip = nullptr;
  bool bHandStatus = false;
  bool bSkillOff = false;
  DWORD dwVersion = 0;

  // temp data
  bool bLive = false;
  EPetState eState = EPETSTATAE_NORAML;
  QWORD qwTempID = 0;
  DWORD dwTempEndBattleTime = 0;
  DWORD dwLastBattleTime = 0;
  DWORD dwTimeTick = 0;
  DWORD dwActionTouchTick = 0;
  DWORD dwActionFeedTick = 0;
  DWORD dwActionIdleTick = 0;
  bitset<EPETDATA_MAX> obitset;
  const SPetCFG* pCFG = nullptr;
  SceneUser* pOwner = nullptr;

  TSetDWORD setUnlockEquipIDs;
  TSetDWORD setUnlockBodyIDs;

  map<EEquipPos, DWORD> mapInitPos2EquipID; // 宠物换装新加,孵化时生成的默认装扮, pos->道具id
  map<EEquipPos, DWORD> mapCurPos2EquipID;

  DWORD arrActionCD[EPETACTION_MAX] = {0};

  // 吃料理
  DWORD dwEatBeginTime = 0;
  QWORD qwTempFoodNpcID = 0;
  BlobBuffer oBuff;

  bool toEggData(EggData* pData);
  bool fromEggData(const EggData& rData);
  bool toPetData(UserPetData* pData);
  bool fromPetData(const UserPetData& rData);

  bool toClientData(PetInfo* pInfo);
  bool reload();

  void addBaseExp(QWORD exp);
  void addFriendExp(QWORD exp);
  void addRewardExp(QWORD exp);
  void subRewardExp(QWORD exp);

  void baseLevelup();
  void friendLevelup();
  void refreshFriendLevelOpen();

  bool addFeelingTime(EPetDataType eType, DWORD dwTime);

  void setTouchTick(DWORD dwTime) { dwTouchTick = dwTime; obitset.set(EPETDATA_TOUCH_TICK); }
  void setFeedTick(DWORD dwTime) { dwFeedTick = dwTime; obitset.set(EPETDATA_FEED_TICK); }
  void setBody(DWORD dwID) { dwBody = dwID; obitset.set(EPETDATA_BODY); updateData(); }
  void setName(const string& name) { strName = name; obitset.set(EPETDATA_NAME); updateData(); }

  EError canTouch();
  bool touch();
  EError canFeed();
  bool feed();

  bool isEquipUnlocked(DWORD dwID) const { return setUnlockEquipIDs.find(dwID) != setUnlockEquipIDs.end(); }
  bool isBodyUnlocked(DWORD dwID) const { return setUnlockBodyIDs.find(dwID) != setUnlockBodyIDs.end(); }

  bool playAction(EPetAction eAction);
  void processAction(DWORD curSec);

  void updateData();
  void timer(DWORD curSec);

  void patch_1();
  void patch_2();
  void patch_3();
  void patch_4();

  DWORD getEquipIDByDataType(EUserDataType eType) const;
  DWORD getBodyID() const;
};
typedef list<SScenePetData> TListPet;

struct SEquipUnlockInfo
{
  TSetDWORD setUnlockItems;
  std::vector<SpecPetEquip> vecSpecUnlockInfo;;

  void fromData(const PetEquipUnlockInfo& oData);
  void toData(PetEquipUnlockInfo* pData);
};

class UserPet : private xNoncopyable
{
  public:
    UserPet(SceneUser* user);
    virtual ~UserPet();

  public:
    bool load(const BlobUserPet& oData);
    bool save(BlobUserPet* pData);

    bool reload();
    bool reloadItem();

    void timer(DWORD cur);

    bool addPet(const EggData& eggdata, bool bFirst);
    bool toEgg(DWORD petid);
    bool del(DWORD petid);
  public:
    bool preCatchPet(SceneNpc* npc); /* 怪变交互npc */
    QWORD getCatchPetID() const { return m_qwCatchPetID; }
    void setCatchPetID(QWORD npcguid); //{ m_qwCatchPetID = npcguid; } /*断线重登时设置*/
    void clearCatchPetID(QWORD npcguid);// { if (m_qwCatchPetID == npcguid) m_qwCatchPetID = 0; }
    void giveGiftCatchNpc(QWORD npcguid);
  public:
    void addBaseExp(QWORD dwExp);
    void addFriendExp(QWORD qwExp);
    void addRewardExp(QWORD qwExp);

    DWORD getMaxBaseLv() const;
    DWORD getMaxFriendLv() const;
    DWORD getMaxEquipUnlockCount() const;

    bool touch(QWORD qwPetID);
    bool feed(DWORD dwPetID);
    bool sendGift(DWORD dwPetID, const string& itemguid);
    bool getGift(DWORD dwPetID);

    bool equip(const EquipOperPetCmd& cmd);
    bool resetSkill(const ResetSkillPetCmd& cmd);
    bool unlockExtraBody(DWORD dwPetID, DWORD dwBodyID);
    void playAction(EPetAction eAction);

    DWORD getPetCount(DWORD dwBaseLv, DWORD dwFriendLv) const;
    bool changeName(DWORD dwPetID, const string& name);
    void switchSkill(DWORD dwPetID, bool open);

    bool compose(const ComposePetCmd& cmd);
    bool unlockWear(DWORD itemid);
    bool unlockWear(DWORD itemid, DWORD body, EEquipPos pos);
    bool changeWear(const ChangeWearPetCmd& cmd);
    void onManualOffEquip(DWORD itemid);
  public:
    void onUserEnterScene();
    void onUserLeaveScene();
    void onPetDie(SceneNpc* npc);
    void onUserAttack(xSceneEntryDynamic* enemy);
    void onUserMove(const xPos& dest);
    void onUserMoveTo(const xPos& dest);
    void onUserGoTo(const xPos& dest);
    void onUserDie();
    void onSeeFood(SceneNpc* foodNpc);
    void onStopFood(SceneNpc* foodNpc);
    void onUserOffCarrier();
    void onUserStartTouch(QWORD npcguid);
    void onUserStopTouch();
    void testhand(bool build);
  public:
    void inviteHand(QWORD qwPetID);
    void breakHand();
    bool handPet() const;
    SceneNpc* getPetNpc() const;
    SScenePetData* getPetData(DWORD dwPetID);
    TListPet& getPetList() { return m_listPetData; }
    void collectPetList(TSetDWORD& setIDs);
    bool hasPet(DWORD dwPetID, DWORD dwBaseLv, DWORD dwFriendLv) const;
    static EUserDataType getDataTypeByPos(EEquipPos epos);
    DWORD getFirendFullNum() const;
  public:
    DWORD getBaseLv(QWORD petguid) const;
  private:
    bool enterScene(SScenePetData& stData, bool bFirst = false, bool bFromEgg = false);
    bool leaveScene(SScenePetData& stData, bool bToEgg = false);
    void changeState(SScenePetData& stData, EPetState eState);
    void onMove(SScenePetData& stData, const xPos& dest);

    void sendHandStatus(bool bBuild, QWORD qwPetGUID);
    void sendHandBreak(QWORD qwPetGUID);
    void sendHandMark(QWORD qwPetGUID); /*通知前端牵手状态, 用于图标展示*/

    void sendPetInfo();
    void sendPetDel(DWORD petid);
    void updatePetState(SScenePetData& stData);
    //void checkChangeLevel(SScenePetData& stData);
    void sendUnlockInfo();
    bool checkWearUnlock(DWORD itemid, DWORD body, EEquipPos pos);
  public:
    void sendBattlePets();
  private:
    SceneUser* m_pUser = nullptr;
    QWORD m_qwCatchPetID = 0;
    TListPet m_listPetData;
    TVecItemInfo m_vecGotItem;

    // 解锁的装备信息
    SEquipUnlockInfo m_oEquipUnlockInfo;

    DWORD m_dwPetAchieveTick = 0;
};

