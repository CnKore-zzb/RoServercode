#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "BeingConfig.h"
#include "SceneNpc.h"
#include "SceneBeing.pb.h"
#include "RecordCmd.pb.h"
#include "AstrolabeConfig.h"

using namespace Cmd;

const DWORD BEING_VERSION = 1;

enum EBeingState
{
  EBEINGSTATE_NORMAL = 1,
  EBEINGSTATE_WAIT_BATTLE = 2,
};

struct SBeingSkillData
{
  DWORD dwID = 0;
  DWORD dwPos = 0;
  DWORD dwRuneSpecID = 0;
  bool bSelectSwitch = false;
  bool bLearn = false;
  bool bActive = false;
  bool bNotReset = false; // 为true(目前为进阶添加的技能), 表示重置时不被完全重置, 仅设置为1级

  bool fromSkillItem(const SkillItem& data);
  bool toSkillItem(SkillItem* data);
  bool toClientSpecSkillInfo(BeingSkillData* data);
  const SRuneSpecCFG* getRuneSpecCFG();
  DWORD calcUsedSkillPoint();
};

struct SSceneBeingData
{
  DWORD dwID = 0;
  DWORD dwLv = 0;
  DWORD dwHp = 0;
  bool bLive = false;
  QWORD qwExp = 0;
  DWORD dwUsedSkillPoint = 0;
  bool bBattle = false;
  BlobBuffer oBuff;
  map<DWORD, SBeingSkillData> mapSkillItem;
  TSetDWORD setBuffs;
  DWORD dwBody = 0;
  TSetDWORD setBodyList;

  // 动态数据
  QWORD qwTempID = 0;
  DWORD dwTempEndBattleTime = 0;
  DWORD dwLastBattleTime = 0;
  SceneUser* pUser = nullptr;
  const SBeingCFG* pCFG = nullptr;
  EBeingState eState = EBEINGSTATE_NORMAL;
  bitset<EPETDATA_MAX> obitset;

  DWORD dwVersion = 0;

  bool fromData(const UserBeingData& data);
  bool toData(UserBeingData* data);
  bool reload();

  bool toClientSkillData(BeingSkillData* data, bool skill);
  bool toClientInfoData(BeingInfo* data);

  void addBaseExp(QWORD exp, bool notify = true);
  void updateData();
  DWORD getMaxSkillPoint();
  void updateAttribute();
  DWORD getSkillLv(DWORD skillgroupid);
  bool hasSkill(DWORD skillgroupid);
  SBeingSkillData* getSkillByGroupID(DWORD skillgroupid);
  const SRuneSpecCFG* getRuneSpecCFG(DWORD skillid);
  void unlockSkill(BeingSkillData* data);
  bool hasEquipedSkill();
  bool hasNormalSkillEquiped();
  void addBuff(TSetDWORD ids);
  void delBuff(TSetDWORD ids);
  bool onRuneReset(BeingSkillData* data, DWORD specid);
  bool isRuneSpecSelected(DWORD specid);
  DWORD calcUsedSkillPoint(); // 遍历技能计算实际的消耗

  void patch_1();
};

class UserBeing : private xNoncopyable
{
public:
  UserBeing(SceneUser* user);
  virtual ~UserBeing();

public:
  bool load(const BlobUserBeing& data);
  bool save(BlobUserBeing* data);

  bool loadProfessionData(const BlobUserBeing& data);

  bool reload();
  void timer(DWORD cur);

  SSceneBeingData* getBeingData(DWORD id);
  SSceneBeingData* getCurBeingData() { return m_dwCurBeingID != 0 ? getBeingData(m_dwCurBeingID) : nullptr; }
  SceneNpc* getCurBeingNpc();
  bool isBeingPresent(DWORD beingid) { SSceneBeingData* p = getCurBeingData(); return p ? (p->dwID == beingid && p->bLive && p->qwTempID) : false; }
  void sendData();
  void createAllBeing();
  DWORD getCurBeingID() { return m_dwCurBeingID; }
  QWORD getMasterCurLockID() { return m_qwMasterCurLockID; }
  QWORD getCurBeingGUID() { SceneNpc* npc = getCurBeingNpc(); return npc ? npc->getGUID() : 0; }

  void changeState(SSceneBeingData* being, EBeingState state);
  void updateState(SSceneBeingData* being);

  bool addBeing(DWORD id);
  bool enterScene(DWORD id, DWORD hppercent = 0);
  bool leaveScene(DWORD id);
  void addBaseExp(QWORD exp);
  bool summon(DWORD id);
  bool revive(DWORD hppercent);
  bool addBufftoBeing(DWORD beingid, const TSetDWORD& buffids, xSceneEntryDynamic* fromEntry = nullptr, DWORD lv = 0);

  void onUserEnterScene();
  void onUserLeaveScene();
  void onUserMove(const xPos& dest);
  void onUserMoveTo(const xPos& dest);
  void onUserGoTo(const xPos& dest);
  void onUserDie();
  void onUserAttack(xSceneEntryDynamic* enemy);
  void onBeingDie(SceneNpc* npc);
  void onUserSkillLevelUp(DWORD skillid);
  void onUserSkillReset();
  void onUserAttrChange();
  void onProfesChange();
  void onRuneReset(DWORD specid);
  void onRuneReset();

  void addSkillPoint(DWORD value);
  void decSkillPoint(DWORD value);
  DWORD getSkillPoint() { return m_dwSkillPoint; }
  void sendSkillData(bool skill = true);
  bool skillLevelUp(BeingSkillLevelUp& cmd);
  void addBuff(DWORD beingid, TSetDWORD ids);
  void delBuff(DWORD beingid, TSetDWORD ids);
  void switchRune(DWORD beingid, DWORD skillid, bool isopen);
  void selectRuneSpecID(DWORD beingid, DWORD familySkillID, DWORD runespecid);
  void equipSkill(const EquipSkill& cmd);
  void resetAllSkill(DWORD beingid);
  bool addSkill(DWORD beingid, DWORD skillid);
  bool checkHasSkill(DWORD beingid, DWORD skillid);
  bool checkHasSameSkill(DWORD beingid);

  DWORD getRuneSpecSkillID(DWORD beingid, DWORD skillid);
  int getRuneSpecRange(DWORD beingid, DWORD skillid);
  const SRuneSpecCFG* getRuneSpecCFG(DWORD beingid, DWORD skillid);
  bool isRuneSpecSelected(DWORD specid);

  void sendBeingInfo();
  bool handleBeingSwitchState(BeingSwitchState& cmd);
  bool handleBeingOffCmd(BeingOffCmd& cmd, bool force = false);

  bool changeBody(DWORD beingid, DWORD body);
  bool addBody(DWORD beingid, DWORD body);
  DWORD getBody(DWORD beingid);
  const string& getName(DWORD beingid);
  void fixUsedSkillPoint();
private:
  void resetSkill();
  bool isAlchemist();
  void fixBeingData();

  SceneUser* m_pUser = nullptr;
  map<DWORD, SSceneBeingData> m_mapID2Being;
  DWORD m_dwCurBeingID = 0;
  DWORD m_dwSkillPoint = 0;
  QWORD m_qwMasterCurLockID = 0;
  DWORD m_dwTempEndBattleTime = 0;
};
