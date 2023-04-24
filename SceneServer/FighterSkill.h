/**
 * @file FighterSkill.h
 * @brief user skill
 * @author liuxin, liuxin@xindong.com
 * @version v1
 * @date 2015-05-05
 */

#pragma once

#include <string>
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "SceneSkill.pb.h"
#include "SkillConfig.h"

using std::string;
using std::vector;
using std::set;
using std::pair;
using std::map;
using namespace Cmd;

#define ONE_JOB_POINT 40
#define MONTH_CARD_SKILL_POS 7    //月卡技能槽位置

class SceneFighter;
class BaseSkill;

const DWORD SKILL_SOURCEID = 999;
const DWORD SKILL_MAX_CUTS = 6;
const vector<ESkillShortcut> VEC_SHORTCUT_EXTEND = vector<ESkillShortcut>{ESKILLSHORTCUT_EXTEND, ESKILLSHORTCUT_EXTEND_2, ESKILLSHORTCUT_EXTEND_3};

// skill consume
struct SSkillConsume
{
  //DWORD itemID = 0;
  DWORD nexttime = 0;

  void toData(SkillConsume* pConsume, SceneFighter* pSF);
  void fromData(const SkillConsume& rConsume, SceneFighter* pSF);
  void reset();
};

// skill item
struct SSkillItem
{
  DWORD dwID = 0;
  DWORD dwPos = 0;
  DWORD dwCD = 0;
  DWORD dwAutoPos = 0;

  bool bActive = false;
  bool bLearn = false;
  bool bShadow = false;

  EProfession eProfession = EPROFESSION_MIN;
  ESource eSource = ESOURCE_MIN;

  SSkillConsume consume;
  const BaseSkill* pSkillCFG = nullptr;

  DWORD dwSourceid = 0;
  DWORD dwRuneSpecID = 0;
  DWORD dwReplaceID = 0;
  DWORD dwExtraLv = 0;
  bool bRuneOpened = true;
  QWORD qwOwnerID = 0;

  map<ESkillShortcut, DWORD> mapShortcut;

  SSkillItem() {}

  bool toData(SkillItem* pItem, SceneFighter* pSF);
  bool fromData(const SkillItem& rItem, SceneFighter* pSF);
  bool isRealget() const;
};
typedef vector<SSkillItem> TVecSkillItem;

// skill data
struct SSkillData
{
  DWORD dwUsedPoint = 0;
  DWORD dwMaxPos = 0;
  DWORD dwPrimaryPoint = 0;

  EProfession eProfession = EPROFESSION_MIN;

  TVecSkillItem vecSkillItem;

  SSkillData() {}

  void toData(SkillData* pData, SceneFighter* pSF);
  void toClientData(SkillData* pData, SceneFighter* pSF);
  void fromData(const SkillData& rData, SceneFighter* pSF);

  SSkillItem* getSkillItem(DWORD id, DWORD sourceid = 0, ESource eSource = ESOURCE_MIN);
  SSkillItem* getSkillByType(DWORD id);

  bool isSkillEnable(DWORD id);
  bool isSkillFamilyEnable(DWORD familyid);

  bool removeSkill(DWORD id);
};
typedef vector<SSkillData> TVecSkillData;

// skill pos
//typedef pair<DWORD, DWORD> TPairSkillPos;
//typedef vector<TPairSkillPos> TVecSkillPos;
struct SPosData
{
  DWORD dwPos = 0;

  DWORD dwID = 0;
  DWORD dwSourceid = 0;

  void clear() { dwID = dwSourceid = 0; }
};
typedef vector<SPosData> TVecPosSkill;
typedef map<ESkillShortcut, TVecPosSkill> TMapSkillShortcut;

struct SSpecSkillInfo
{
  DWORD dwID = 0;

  map<EAttrType, float> mapAttrValue;
  map<DWORD, pair<int, float>> mapItem2NumAndPer;

  int intChCount = 0; // 持续次数
  int intChTarCount = 0; // 技能目标数量
  float fChRange = 0; // 技能范围
  float fChDurationPer = 0; // 持续时间
  float fChReady = 0; // type=1 吟唱时间
  int intChLimitCnt = 0; // 限制数量
  bool bNeedNoItem = false; // 若为true,则技能不需要消耗道具

  void clearData();
  bool checkClear();
  void toData(SpecSkillInfo* pInfo);
};
typedef map<DWORD, SSpecSkillInfo> TMapSpecSkillInfo;

// fighter skill
class FighterSkill
{
  public:
    FighterSkill(SceneFighter* pFighter);
    ~FighterSkill();

    bool load(const UserSkillData& data);
    bool save(UserSkillData* pData);
    void reload();

    bool loadAcc(const BlobShareSkill& data);
    bool saveAcc(BlobShareSkill* pData);

    bool isSkillEnable(DWORD id);
    bool isSkillEquiped(DWORD id) const;
    bool isSkillFamilyEquiped(DWORD familyid) const;
    bool isSkillFamilyEnable(DWORD familyid);
    bool checkSkill(DWORD dwSkillID);
    bool canEquip(ESkillType eType) const;
    bool isInSlot(DWORD skillid, DWORD sourcid) const;
    bool hasSkill(DWORD id, DWORD sourceid);

    void sendData();
    void sendSkillData();
    void sendValidPos();
    void toData(SkillValidPos* pData);

    DWORD getSkillPoint() const { return m_dwTotalPoint; }
    void setSkillPoint(DWORD point, ESource eSource);

    DWORD getUsedPoint() const;
    DWORD getSkillBTPoint();

    DWORD getMaxPos() const { return m_dwMaxPos; }
    void setMaxPos(DWORD dwPos);
    void decMaxPos();

    DWORD getAutoMaxPos() const { return m_dwAutoMaxPos; }
    void setAutoMaxPos(DWORD dwPos);
    void decAutoMaxPos();

    DWORD getMaxExtendPos() const { return m_dwMaxExtendPos; }
    void setMaxExtendPos(DWORD dwPos);
    void decMaxExtendPos();

    DWORD getNormalSkill();
    DWORD getSkillLevel(DWORD skillid) const { return skillid - skillid / 100 * 100; }

    void refreshEnableSkill();

    bool addSkill(DWORD skillid, DWORD sourceid, ESource eSource = ESOURCE_EQUIP, bool needpos = true, bool bNotify = true);
    bool removeSkill(DWORD skillid, DWORD sourceid, ESource eSource = ESOURCE_EQUIP, bool onlyCheckID = false);
    bool resetShopSkill();

    void addTempSkill(DWORD id, bool free) { m_setTempIDs.insert(id); if (free) m_setTempFreeIDs.insert(id); }
    void delTempSkill(DWORD id) { m_setTempIDs.erase(id); m_setTempFreeIDs.erase(id); }
    bool isFreeSkill(DWORD skillid) { return m_setTempFreeIDs.find(skillid) != m_setTempFreeIDs.end(); }

    bool levelupSkill(const LevelupSkill& cmd);
    bool resetSkill();

    //void refershConsume(DWORD id);
    void refresheConsume();
    void updateConsume(DWORD cur);
    void update(DWORD cur);
    void clearUpdate() { m_vecUpdateIDs.clear(); }

    void getCurSkills(TVecDWORD& vecIDs);
    DWORD getSkillLv(DWORD skillid);
    void getEquipSkill(DWORD oneid, TVecDWORD& vecids);
    bool addEquipSkill(FighterSkill& oldFs);

    SSkillData* getSkillData(EProfession eProfession);

    DWORD getPos(DWORD skillid, DWORD sourceid);
    DWORD getAutoPos(DWORD skillid, DWORD sourceid);
    DWORD getExtendPos(DWORD skillid, DWORD sourceid);

    void forceEnableSkill(DWORD skillid);

    void getSkillPosInfo(TVecPosSkill& vecSkillPosInfo);

    bool checkSkillPointIllegal();

    void replaceNormalSkill(DWORD dwNewSkill);
    void restoreNormalSkill();
    DWORD getReplaceNormalSkill() { return m_dwReplaceNormalSkillID; }

    // 影响单个技能参数
    float getSkillAttr(DWORD skillid, EAttrType etype);
    pair<int, float> getCostItemInfo(DWORD skillid, DWORD itemid);
    bool haveSpecSkill(DWORD skillid) { return m_mapSpecSkillInfo.find(skillid/ONE_THOUSAND) != m_mapSpecSkillInfo.end(); }
    float getDurationPer(DWORD skillid); // 获取技能持续时间变化
    float getChangeRange(DWORD skillid); // 获取技能范围变化
    float getChangeReady(DWORD skillid); // type=1, 吟唱时间变化
    int getChangeCount(DWORD skillid); // 获取技能次数变化
    int getChangeTarCount(DWORD skillid); // 获取技能目标数量变化
    int getLimitCount(DWORD skillid); // 获取限制目标数量变化
    void markSpecSkill(DWORD familySkillID) { m_setSpecSkillUpdates.insert(familySkillID); }
    void sendSpecSkillInfo();
    const SSpecSkillInfo* getSpecSkillInfo(DWORD skillid) const;
    // 影响单个技能参数
    
    // 获取特定技能的参数
    SSpecSkillInfo* getSpecWithoutGlobal(DWORD);
  
    void selectRuneSpecID(DWORD familySkillID, DWORD runespecid);
    void switchRune(DWORD skillid, bool isopen);
    void onRuneReset(DWORD runespecid);

    void validMonthCardSkillPos();
    void invalidMonthCardSkillPos();
    bool isRightAutoPos(DWORD pos) const;
    bool isRuneSpecSelected(DWORD runespecid);
    bool isAutoPosValid(DWORD pos) const;
    DWORD getRuneSelectID(DWORD familySkillID);

    void setReplaceSkill(DWORD oldFamilyID, DWORD newFamilyID);
    void delReplaceSkill(DWORD oldFamilyID, DWORD newFamilyID);
    DWORD getReplaceSkill(DWORD dwFamilyID);

    bool changeSkill(DWORD oldskillid, DWORD newskillid);
    bool recoverSkill(DWORD oldskillid);
    DWORD getOrinalSkill(DWORD newskillid) const;

    void addExtraSkillLv(DWORD familyID, DWORD lv);
    void decExtraSkillLv(DWORD familyID, DWORD lv);

    void resetWeddingSkill(bool bMarry);
    void onDeleteChar(QWORD charid);
    void fixEvo4Skill();

    bool equipSkill(const EquipSkill& cmd);
    bool equipSkill(ESkillShortcut eType, DWORD dwSkillID, DWORD dwSourceID);
    bool equipSkill(ESkillShortcut eType, DWORD dwSkillID, DWORD dwSourceID, DWORD dwPos);

    SPosData* getShortcut(ESkillShortcut eType, DWORD dwPos);
    SPosData* getShortcut(ESkillShortcut eType, DWORD dwSkillID, DWORD dwSourceID);
    bool removeShortcut(ESkillShortcut eType, DWORD dwSkillID, DWORD dwSourceID);
    bool isExtend(ESkillShortcut eCut) const { return find(VEC_SHORTCUT_EXTEND.begin(), VEC_SHORTCUT_EXTEND.end(), eCut) != VEC_SHORTCUT_EXTEND.end(); }
    
    const SSpecSkillInfo* getAllSkillSpec() { return &m_sAllSkillInfo; }

    DWORD getLastConcertSkillID() { return m_dwLastConcertSkillID; }
    void setLastConcertSkillID(DWORD id);
  private:
    SPosData* getPosSkill(DWORD pos, bool autoOrNormal, bool bExtend = false);
    void addPosUpdate(DWORD skillid, DWORD sourceid);

    void addupdate(EProfession, DWORD id, DWORD sourceid = 0);
    bool replaceSkill(DWORD oldid, DWORD newid);

    void updateSpecSkill();
    void updateAllSkill();
    void collectSpecSkill(DWORD familySkillID);

    void onSelectRuneSpecID(DWORD runespecid);
    void onOffRuneSpecID(DWORD dwRuneSpecID);
    void onClientChangeRune(DWORD dwRuneSpecID, bool bOpen);

    bool checkInBranch(EProfession eProfession);
    void initShortcut();
  private:
    SceneFighter* m_pFighter = nullptr;

    DWORD m_dwTotalPoint = 0;
    DWORD m_dwMaxPos = 0;
    DWORD m_dwAutoMaxPos = 0;
    DWORD m_dwMaxExtendPos = 0;

    DWORD m_dwReplaceNormalSkillID = 0;

    TVecSkillData m_vecSkillData;
    TVecSkillData m_vecUpdateIDs;

    SSkillData m_stOtherData;
    set<DWORD> m_setUpdateIDs;

    set<DWORD> m_setTempIDs;
    set<DWORD> m_setTempFreeIDs;

    bool m_bMonthCardSkillValid = false;
    TMapSkillShortcut m_mapShortcut;

    TMapSpecSkillInfo m_mapSpecSkillInfo;
    TSetDWORD m_setSpecSkillUpdates;
    SSpecSkillInfo m_sAllSkillInfo; // 对所有技能生效的影响
    map<DWORD, DWORD> m_mapTempReplaceRecord; /*key:newskill grp, value:oldskill grp*/
    map<DWORD, DWORD> m_mapRecordReplaceID; /*new skilll id, old skill id, need record*/

    DWORD m_dwLastConcertSkillID = 0;
};

