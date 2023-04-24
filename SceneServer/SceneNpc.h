#pragma once

#include "xSceneEntryDynamic.h"
#include "NpcAI.h"
#include "SuperAI.h"
#include "NpcEmoji.h"
#include "TreasureConfig.h"
#include "MiscConfig.h"
#include "NpcConfig.h"
#include "BossConfig.h"
#include "BossConfig.h"

class Scene;

// scene npc
class SceneNpc : public xSceneEntryDynamic
{
  public:
    SceneNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~SceneNpc();

    virtual SCENE_ENTRY_TYPE getEntryType()const {return SCENE_ENTRY_NPC;}
    virtual bool initAttr();
    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);

    void reload();

    // 场景
  public:
    virtual bool enterScene(Scene* scene);
    virtual void leaveScene();
    virtual void sendMeToNine();
    virtual void sendCmdToNine(const void* cmd, DWORD len, GateIndexFilter filter=GateIndexFilter());
    virtual void sendCmdToScope(const void* cmd, DWORD len);

    bool checkNineScreenShow(xSceneEntryDynamic* entry);
    void delMeToNine();
    void delMeToUser(SceneUser* pUser);
    void sendMeToUser(SceneUser* pUser);

    void informUserAdd(SceneUser* pUser);
    void informUserDel(SceneUser* pUser);

    virtual void fillMapNpcData(Cmd::MapNpc *data);

    // 定时器
  protected:
    virtual void onOneSecTimeUp(QWORD curMSec);
    virtual void onFiveSecTimeUp(QWORD curMSec);
    virtual void onOneMinTimeUp(QWORD curMSec);
    virtual void onTenMinTimeUp(QWORD curMSec);
    virtual void onDailyRefresh(QWORD curMSec);
  public:
    virtual void refreshMe(QWORD curMSec);

    // 状态检查
  public:
    void checkEvoState(DWORD cur);
    void checkNightWork(DWORD cur);
    bool evo(const SNpcCFG* pCFG, const NpcDefine& def);

    void setClearState() { if (getStatus() != ECREATURESTATUS_REMOVE) setStatus(ECREATURESTATUS_REMOVE); }
    void removeAtonce();
    void setDeadRemoveAtonce();
    bool canRelive() { return getScene() == nullptr && (getStatus() == ECREATURESTATUS_DEAD || getStatus() == ECREATURESTATUS_IDLE); }
  protected:
    virtual void onNpcDie();
    bool relive();
  private:
    void status_live(QWORD curMSec);
    void status_remove();
    void status_dead(DWORD curSec);
    void status_relive(DWORD curSec);
    void status_leave();
    void status_evo(DWORD curSec);
    void status_suicide();

    // 攻击相关
  protected:
    QWORD m_qwSkillMasterID = 0;
  public:
    void setSkillMasterID(QWORD qwCharID) { m_qwSkillMasterID = qwCharID; }
    QWORD getSkillMasterID() { return m_qwSkillMasterID; }
    //xSceneEntryDynamic* m_pMaster = nullptr;
    //xSceneEntryDynamic* pSkillMaster = nullptr;
    virtual bool beAttack(QWORD damage, xSceneEntryDynamic* target);
    virtual bool canUseSkill(const BaseSkill* skill);

    // 数据
  public:
    virtual bool isImmuneSkill(DWORD skillid);

  protected:
    virtual void fetchChangeData(NpcDataSync& cmd);

    virtual void updateData(DWORD curTime);

    // 属性
  public:
    const SNpcCFG* getCFG() const { return m_pCurCFG; }
    const NpcDefine& getDefine() const { return define; }

    virtual DWORD getLevel() const { return define.getLevel() != 0 ? define.getLevel() : m_pCurCFG == nullptr ? 0 : m_pCurCFG->dwLevel; }
    virtual SQWORD changeHp(SQWORD hp, xSceneEntryDynamic* entry, bool bshare = false, bool bforce = false);
    virtual float getMoveSpeed();

    DWORD getSmileEmoji() const { return m_pCurCFG != nullptr ? m_pCurCFG->dwSmileEmoji : 0; }
    DWORD getSmileAction() const { return m_pCurCFG != nullptr ? m_pCurCFG->dwSmileAction : 0; }

    void setScale(float f);
    DWORD getScale();// { return m_flScale; }

    QWORD getBirthTime() const { return m_birthTime; }
    const xPos& getBirthPos() const { return m_oBirthPos; }

    QWORD getFollowerID() const { return define.m_oVar.m_qwFollowerID; }
    DWORD getCrystal();

    DWORD getShowByType(EUserDataType eType) const;
    ENatureType getNatureType() const { return define.getNatureType() != ENature_MIN ? define.getNatureType() : (m_pCurCFG != nullptr ? m_pCurCFG->eNatureType : ENature_MIN); }

    DWORD getSceneID() const { return m_dwSceneID; }
    void removeTeamSeeLimit() { define.m_oVar.m_qwTeamUserID = 0; m_oOriDefine.m_oVar.m_qwTeamUserID = 0; }
    void randPos(DWORD curSec);
  public:
    NpcDefine define;
    NpcDefine m_oOriDefine;
  protected:
    const SNpcCFG* m_pCurCFG = nullptr;
    const SNpcCFG* m_pOriCFG = nullptr;
    float m_flScale = 0.0f;
    DWORD m_dwDeadCount = 0;
    DWORD m_dwReliveTime = 0;
    QWORD m_birthTime = 0;
    DWORD m_dwDeadTime = 0;
    DWORD m_dwBornTime = 0;
    DWORD m_dwEvoTime = 0;
    DWORD m_dwRandPosTime = 0;
    bool m_bTempJustBirth = false;

    xPos m_oBirthPos;
    DWORD m_dwSceneID = 0;

    bool m_bReliveAtOldPos = false;

    bool m_bEvo = false;
    DWORD m_dwEvoID = 0;
    NpcDefine m_oEvoDef;
  public:
    bool isMonster() const { return NpcConfig::getMe().isMonster(m_pCurCFG->dwID); }
    bool isWeaponPet() const { return m_pCurCFG->eNpcType == ENPCTYPE_WEAPONPET; }
    bool isNoFunBenClear() const { return define.m_oVar.m_qwFollowerID || define.m_oVar.m_qwOwnerID; }
  public:
    void setFollowerFace();
    inline DWORD getSurviveTime()
    {
      DWORD dead = m_dwDeadTime;
      if (!dead)
        dead = now();
      if (dead && m_dwBornTime)
      {
        if (dead > m_dwBornTime)
          return dead - m_dwBornTime;
      }
      return 0;
    }
    void setReliveAtOldPos() { m_bReliveAtOldPos = true; }
    // ai
  public:
    NpcAI m_ai;
    SuperAI m_sai;

  protected:
    DWORD m_dwSetClearTime = 0;
  public:
    bool isBoss() { return m_eBossType == EBOSSTYPE_MVP || m_eBossType == EBOSSTYPE_DEAD || m_eBossType == EBOSSTYPE_WORLD; }
    bool isSummonBySession() { return m_eBossType == EBOSSTYPE_MVP || m_eBossType == EBOSSTYPE_MINI || m_eBossType == EBOSSTYPE_DEAD || m_eBossType == EBOSSTYPE_WORLD; }
    EBossType m_eBossType = EBOSSTYPE_MIN;
  private:
    DWORD m_talkNextTime = 0;
    DWORD m_talkLastTime = 0;
    bool beHitFirst = true;
    vector<pair<DWORD, DWORD>> m_vecTime2Talk;

  public:
    void setTimeTalk(DWORD cur);
    void setTimeTalk(DWORD sayID, DWORD time) { m_vecTime2Talk.push_back(pair<DWORD, DWORD>(time, sayID)); }
    void setTrigTalk(const string& type);
    void sendTalkInfo(DWORD talkid);
    void setClearTime(DWORD time2del) { m_dwSetClearTime = time2del; }

  public:
    // lua 调用
    QWORD getGUID() const { return id; }
    DWORD getNpcID() const { return m_pCurCFG == nullptr ? 0 : m_pCurCFG->dwID; }
    virtual ENpcType getNpcType() const { return m_pCurCFG == nullptr ? ENPCTYPE_MIN : m_pCurCFG->eNpcType; }
    DWORD getNpcZoneType() { return static_cast<DWORD>(m_pCurCFG->eZoneType);}
    EProfession getProfession() { return m_pCurCFG == nullptr ? EPROFESSION_MIN : m_pCurCFG->eProfession; }
    DWORD getSummonType() const { return static_cast<DWORD>(define.getSummonType()); }
    DWORD getDojoLevel() const { return define.m_oVar.dwDojoLevel; }
    DWORD getRoundID() const { return define.m_oVar.dwLabRound; }
    DWORD getSealType() const { return define.m_oVar.dwSealType; }
    DWORD getEndlessLayer() const { return define.m_oVar.dwLayer; }
    DWORD getPeriodCnt() const { return m_pCurCFG ? m_pCurCFG->dwPeriodKillCnt : 0; }
    DWORD getDeadLv() const { return define.getDeadLv(); }
    bool isStarMonster() const { return m_pCurCFG != nullptr ? m_pCurCFG->bStar : false; }
    bool getChangeLinePunish() const { return m_pCurCFG != nullptr ? m_pCurCFG->getFeaturesByType(ENPCFEATURESPARAM_CHANGELINEPUNISH) : false; }
    bool isFakeMini() const { return m_pCurCFG != nullptr ? m_pCurCFG->getFeaturesByType(ENPCFEATURESPARAM_FAKEMINI) : false; }

    float get_x() const { return getPos().x; }
    float get_y() const { return getPos().y; }
    float get_z() const { return getPos().z; }

    MoveAction& getMoveAction() { return m_oMove; }
    NpcAI& getNpcAI() { return m_ai; }

    virtual DWORD getRaceType() { return define.getRaceType() != ERACE_MIN ? static_cast<DWORD>(define.getRaceType()) : (m_pCurCFG == nullptr ? 0 : m_pCurCFG->eRaceType); }
    virtual DWORD getBodySize() const { return define.getShape() != 0 ? define.getShape() : (m_pCurCFG == nullptr ? 0 :m_pCurCFG->figure.bodySize); }
    virtual DWORD getNormalSkill() const { if (m_dwRelpaceNormalSkill) return m_dwRelpaceNormalSkill;
      return define.getNormalSkillID() != 0 ? define.getNormalSkillID() : (m_pCurCFG != nullptr ? m_pCurCFG->dwNormalSkillID : 0); }
    virtual bool bDamageAlways1() { return m_ai.isBeAttack1Max(); }
    virtual SceneUser* getMasterUser() const { return nullptr; }
    virtual DWORD getMasterCurLockID() const { return 0; }
    SceneUser* getScreenUser() const;
    virtual bool isTrapNpc() const { return false; }

    DWORD getMapRewardRatio(DWORD mapid) const;
    DWORD getMapMonsterRatioBuff(DWORD mapid) const;
  public:
    void addUserDamage(QWORD qwUserID, DWORD dwDamage);
    DWORD getBaseExp() const { return define.getBaseExp() != 0 ? define.getBaseExp() : (m_pCurCFG == nullptr ? 0 : static_cast<DWORD>(m_pCurCFG->qwBaseExp)); }
    DWORD getJobExp() const { return define.getJobExp() != 0 ? define.getJobExp() : (m_pCurCFG == nullptr ? 0 : static_cast<DWORD>(m_pCurCFG->qwJobExp)); }
  protected:
    map<QWORD, DWORD> m_mapUserDamage;
    TSetQWORD m_setEventUser;
  public:
    DWORD getBehaviours();

    // 表情
  public:
    NpcEmoji m_oEmoji;
  public:
    void setGearStatus(DWORD id) { m_dwGearStatus = id ;}
    DWORD getGearStatus() { return m_dwGearStatus; }
    void sendGearStatus(xSceneEntryDynamic* entry);
    void playGearStatusToNine(DWORD actionid, bool bSave = true);
    void setSpecialGearStatus(bool special) { m_bSpecialGearStatus = special; }
    bool getSpecialGearStatus() { return m_bSpecialGearStatus; }
  protected:
    DWORD m_dwGearStatus = 0;
    bool m_bSpecialGearStatus = false;

    // 动作、特效
  public:
    virtual void setAction(DWORD actionid) { m_dwActionID = actionid; }
    virtual DWORD getAction() const { return m_dwActionID; }
    void setEffect(const string& effectpath, DWORD effectpos, DWORD index = 1) { m_strEffectPath = effectpath; m_dwEffectPos = effectpos; m_dwEffectIndex = index; }
    void clearEffect() { m_strEffectPath.clear(); m_dwEffectPos = 0; m_dwEffectIndex = 0; }
  private:
    DWORD m_dwEffectPos = 0;
    DWORD m_dwEffectIndex = 0;
    string m_strEffectPath;

  public:
    bool isVisableToAll();
    bool isVisableToSceneUser(SceneUser *);

  protected:
    std::pair<QWORD/*owner*/, DWORD/*last kill time*/> m_oOwner;

  private:
    bool changeGuid();
    void onGuidChange(QWORD oldguid);

  public:
    float calcDropRatio(SceneUser* pUser);

    // 死亡爆怪
  //public:
    //bool turnNewMonster(const NpcDefine& def);
  private:
    void loadAttr();

    // 记录掉血之前的hp
  public:
    DWORD getLastHp() { return m_dwLastHp; }
  private:
    DWORD m_dwLastHp = 0;

  public:
    void setActivityUid(QWORD uid) { m_qwActivityUid = uid; }
    QWORD getActivityUid() { return m_qwActivityUid; }
    std::string getKillerName() { return m_strKillerName; }            //服务器计算的击杀者，如果是mvp则是最后mvp称号的获得者。
    void setNotSyncDead() { m_syncDead = false; }
    bool getSyncDead() { return m_syncDead; }
    void setDeadEffect() { setNotSyncDead(); m_deadEffect = true; }
    void setDeadDelAtonce() { m_bDeadDelAtonce = true; }
  protected:
    QWORD m_qwActivityUid = 0;      //belong to what activity
    std::string m_strKillerName;
    bool m_syncDead = true;
    bool m_deadEffect = false;
    DWORD m_dwActionID = 0;
    bool m_bDeadDelAtonce = false; // 死亡立即删除

  public:
    void setProtect(DWORD interval, DWORD maxhp) { m_dwProtectInterval = interval; m_dwProtectMaxHp = maxhp; m_dwProtectTimeTick = now(); }
  private:
    DWORD m_dwProtectInterval = 0;
    DWORD m_dwProtectMaxHp = 0;
    DWORD m_dwLostHpInProtect = 0;
    DWORD m_dwProtectTimeTick = 0;

  public:
    void addMvpLockUserTime(QWORD charid, QWORD msTime);
    void addMvpRelaHealHp(QWORD charid, DWORD hp, bool isHealSelf);
    void addMvpRelaReliveTimes(QWORD charid);
    void addMvpHitTime(QWORD charid);
    void testPrintMvpInfo(SceneUser* user);
  protected:
    void calcMvpUser();
    void refreshMvpInfo(DWORD cur);
    void sendMvpNotice();
  protected:
    /* mvp kill data record*/
    map<QWORD, SMvpKillInfo> m_mapUser2MvpInfo;
    QWORD m_qwMvpUserID = 0;
    bool m_bHaveSendMvpNotice = false;
    /* mvp kill data record*/
  public:
    void replaceNormalSkill(DWORD skillid) { m_dwRelpaceNormalSkill = skillid; }
    void recoverNormalSkill() { m_dwRelpaceNormalSkill = 0; }
    virtual void useSuperSkill(DWORD skillid, QWORD targetid, xPos pos, bool specpos = false);
  protected:
    DWORD m_dwRelpaceNormalSkill = 0;
  protected:
    virtual void sendExtraInfo(SceneUser* user) {};
  public:
    bool isShareDam() { return m_bShareDamage; }
    void setShareDam(bool flag) { m_bShareDamage = flag; }
  private:
    bool m_bShareDamage = false;
    DWORD getEquipData(EUserDataType eType);
  public:
    void setTempFadeOutTime(DWORD ms) { m_dwTempFadeOutTimeMs = ms; }
  private:
    DWORD m_dwTempFadeOutTimeMs = 0;
    bool m_bDead = false;
  public:
    virtual bool isScreenLimit(SceneUser* pUser);
    virtual bool needCheckScreenLimit();
    virtual bool isHideUser(SceneUser* pUser) { return false; } /*隐身9屏删除*/
    virtual bool needCheckHideUser() { return false; }
  public:
    DWORD getCarryMoney() { return define.getSuperAiNpc() == true ? 0 : m_dwCarryMoney; }
    DWORD getAllCarryMoney() { return m_pCurCFG ? m_pCurCFG->dwCarryMoney : 0; }
    void setCarryMoney(DWORD money) { m_dwCarryMoney = money; }
  private:
    DWORD m_dwCarryMoney = 0;
  public:
    QWORD getLastVisitor() { return m_qwVisitor; }
    void setLastVisitor(QWORD charid) { m_qwVisitor = charid; }
  private:
    QWORD m_qwVisitor = 0;
};

// func npc
class FuncNpc : public SceneNpc, public xObjectPool<FuncNpc>
{
  public:
    FuncNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~FuncNpc();

    virtual bool beAttack(QWORD damage, xSceneEntryDynamic* target);
};

// trap npc
class TrapNpc : public SceneNpc, public xObjectPool<TrapNpc>
{
  public:
    TrapNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~TrapNpc();

    void addTrapSeeUser(xSceneEntryDynamic* pEntry);
    void setTrapSeeAll(){ m_bTrapAllViewed = true; }

    bool canVisible(SceneUser* pUser);
    void setCanImmunedByFieldArea(bool flag) { m_bCanImmunedByFieldArea = flag; }
    bool canImmunedByFieldArea() { return m_bCanImmunedByFieldArea; }

    virtual bool isScreenLimit(SceneUser* pUser);
    virtual bool needCheckScreenLimit() { return true; }
    virtual bool isTrapNpc() const { return true; }
  private:
    TSetQWORD m_setTrapSeeUsers;
    bool m_bTrapAllViewed = false;
    bool m_bCanImmunedByFieldArea = true;
};

// monster npc
class MonsterNpc : public SceneNpc, public xObjectPool<MonsterNpc>
{
  public:
    MonsterNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~MonsterNpc();

    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);
    virtual void fillMapNpcData(Cmd::MapNpc *data);
    virtual void fetchChangeData(NpcDataSync& cmd);
    virtual void onNpcDie();

    const TVecDWORD& getNpcCharacter() const { return m_vecCharac; }

    void addPickItem(const ItemInfo& rInfo);
    void resetPickItem() { m_vecPickItem.clear(); }
    void addStealItem(DWORD item, DWORD count);
    void resetStealItem() { m_vecStealItem2Count.clear(); }
    DWORD calcBaseExp(SceneUser * pUser, DWORD exp);
    DWORD calcJobExp(SceneUser * pUser, DWORD exp);

  protected:
    TVecDWORD m_vecCharac;
    TVecItemInfo m_vecPickItem;
    vector<pair<DWORD, DWORD>> m_vecStealItem2Count;
};

/*// purify npc
class PurifyNpc : public SceneNpc, public xObjectPool<PurifyNpc>
{
  public:
    PurifyNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~PurifyNpc();

    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);
    virtual void fillMapNpcData(Cmd::MapNpc *data);
    virtual void fetchChangeData(NpcDataSync& cmd);
    virtual void onNpcDie(xSceneEntryDynamic* pMaster);

    bool purifyByUser(QWORD userid);
    bool canPurifyBy(QWORD userid) const { return find(m_vecRaidBossUserIDs.begin(), m_vecRaidBossUserIDs.end(), userid) != m_vecRaidBossUserIDs.end(); }
    bool needPurify() { return define.getPurify() > 0; }
    bool canSetClear() { return m_vecRaidBossUserIDs.empty(); }
  private:
    void setRaidBossUser(QWORD userid = 0);
  private:
    TVecQWORD m_vecRaidBossUserIDs;
};*/

// tower npc
class TowerNpc : public SceneNpc
{
  public:
    TowerNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~TowerNpc();

    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);
    virtual void fillMapNpcData(Cmd::MapNpc *data);
    virtual void onNpcDie();

    virtual ENpcType getNpcType() const { return m_eType == ENPCTYPE_MIN ? m_pCurCFG->eNpcType : m_eType; }
    void setNpcType(ENpcType eType);

    virtual DWORD getLevel() const { return define.m_oVar.dwLayer; }
    //virtual void onNpcDie();
  private:
    ENpcType m_eType = ENPCTYPE_MIN;
};

// laboratory npc
class LabNpc : public SceneNpc, public xObjectPool<LabNpc>
{
  public:
    LabNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~LabNpc();

    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);

    void setLaboratoryPoint(DWORD dwPoint) { m_dwLaboratoryPoint = dwPoint; }
    DWORD getLaboratoryPoint() const { return m_dwLaboratoryPoint; }

    void setRoundID(DWORD dwRoundID) { m_dwRoundID = dwRoundID; }
    DWORD getRoundID() const { return m_dwRoundID; }
  private:
    DWORD m_dwLaboratoryPoint = 0;
    DWORD m_dwRoundID = 0;
};

// seal npc
class SealNpc : public SceneNpc, public xObjectPool<SealNpc>
{
  public:
    SealNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def, DWORD mapid);
    virtual ~SealNpc();

    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);

    virtual DWORD getMapID() const { return getScene() == nullptr ? m_dwTempSealMapID : getScene()->getMapID(); }
    virtual DWORD getLevel() const;
    void setTempMapID(DWORD dwMapID) { m_dwTempSealMapID = dwMapID; }
  private:
    DWORD m_dwTempSealMapID = 0;
};

// music npc
class MusicNpc : public SceneNpc, public xObjectPool<MusicNpc>
{
  public:
    MusicNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~MusicNpc();

    virtual void fillMapNpcData(Cmd::MapNpc *data);

    void setMusicData(DWORD dwID, DWORD dwStartTime, QWORD qwCharID, bool bLoop = false);

    DWORD getMusicID() const { return m_dwMusicID; }
    DWORD getStartTime() const { return m_dwStartTime; }
    QWORD getDemanID() const { return m_qwDemanID; }
    bool getLoop() const { return m_bLoop; }
  private:
    DWORD m_dwMusicID = 0;
    DWORD m_dwStartTime = 0;
    QWORD m_qwDemanID = 0;
    bool m_bLoop = false; // 循环播放
};

// dojo npc
class DojoNpc : public SceneNpc, public xObjectPool<DojoNpc>
{
  public:
    DojoNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~DojoNpc();

    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);

    void setDojoLevel(DWORD dojoLevel) { m_dwDojoLevel = dojoLevel; }
    DWORD getDojoLevel() const { return m_dwDojoLevel; }
  private:
    DWORD m_dwDojoLevel = 0;
};

// tree npc
class TreeNpc : public SceneNpc, public xObjectPool<TreeNpc>
{
  public:
    TreeNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~TreeNpc();

    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);
    virtual void fillMapNpcData(Cmd::MapNpc *data);
    virtual void fetchChangeData(NpcDataSync& cmd);
    virtual void onNpcDie();
    virtual void onFiveSecTimeUp(QWORD curMSec);

    void refreshTreeClearTime();
    ETreeType getTreeType() const { return m_eTreeType; }
    void setTreeType(ETreeType eType) { m_eTreeType = eType; }
    DWORD getTreeIndex() const { return m_dwIndex; }
    void addTreeIndex();
    ETreeStatus getTreeStatus();

    void setPosIndex(DWORD dwPosIndex) { m_dwPosIndex = dwPosIndex; }
    DWORD getPosIndex() const { return m_dwPosIndex; }
    void refreshTreeStatus();
    void setNextMoveTime(DWORD sec) { m_dwMoveNextTime = sec; }
  private:
    ETreeType m_eTreeType = ETREETYPE_MIN;
    ETreeStatus m_eTreeStatus = ETREESTATUS_MIN;
    DWORD m_dwIndex = 0;
    DWORD m_dwPosIndex = 0;
    DWORD m_dwMoveNextTime = 0;
    DWORD m_dwLastRefreshTime = 0;
};

// 战斗猫, 帮助玩家战斗
class WeaponPetNpc : public SceneNpc, public xObjectPool<WeaponPetNpc>
{
  public:
    WeaponPetNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~WeaponPetNpc();

    virtual void onNpcDie();
    virtual DWORD getLevel() const;
    virtual void updateData(DWORD curTime);

    QWORD getOwnerID() const { return define.m_oVar.m_qwOwnerID; }
    DWORD getWeaponPetID() const { return define.getWeaponPetID(); }
    virtual bool isMyTeamMember(QWORD id);
  public:
    virtual SceneUser* getMasterUser() const;
    virtual void setAction(DWORD actionid);
    //m_qwMasterUserID = 0;
};

class SkillNpc : public SceneNpc, public xObjectPool<SkillNpc>
{
  public:
    SkillNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~SkillNpc();

  public:
    SceneUser* getMasterUser() const;
    void setMasterUser(QWORD id) { m_qwMasterID = id; }
    void setRelatedSkill(DWORD skillid);
    DWORD getSkillID() { return m_dwRelatedSkillID; }
    virtual void onNpcDie();
  public:
    void onTriggered(SceneUser* user);
  protected:
    virtual void sendExtraInfo(SceneUser* user);
  public:
    virtual bool isScreenLimit(SceneUser* pUser);
    virtual bool needCheckScreenLimit() { return true; }
  private:
    QWORD m_qwMasterID = 0;
    DWORD m_dwRelatedSkillID = 0;
};

typedef struct _FoodEaterInfo
{
  DWORD m_dwTime;
  DWORD m_dwProgress;
} FoodEaterInfo;

// 场景上的料理
class FoodNpc : public SceneNpc, public xObjectPool<FoodNpc>
{
public:
  FoodNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
  virtual ~FoodNpc();
  
  virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);
  virtual void updateData(DWORD curTime) {}
  void setFoodId(DWORD dwItemId) { m_dwItemId = dwItemId; }
  DWORD getFoodId() { return m_dwItemId; }
  void setFoodNum(DWORD dwItemNum) { m_dwItemNum = dwItemNum; }
  DWORD getFoodNum() { return m_dwItemNum; }
  void setFoodTotalNum(DWORD dwItemNum) { m_dwItemTotalNum = dwItemNum; }
  DWORD getFoodTotalNum() { return m_dwItemTotalNum; }

  void setOwnerID(QWORD ownerId, const string& ownerName);
  QWORD getOwnerID() const { return m_qwOwnerId; }
  bool checkOwner(QWORD id) { return m_qwOwnerId == id; }
  void setPower(Cmd::EEatPower power) { m_power = power; }
  void changePower(SceneUser* pOwner, Cmd::EEatPower power);
  bool addEater(SceneUser* pUser);
  bool delEater(SceneUser* pUser);
  DWORD getEaterCount() { return m_eaters.size(); }

  bool checkCanEat(SceneUser* pUser);
  void onOneSecTimeUp(QWORD curMSec);  
  void onEatSuccess(QWORD id);
  void onEatOver();
  void reset();
  bool isSysSummon() { return m_bSys; }
public:
  bool addPetEater(SceneNpc* pPet);
  bool delPetEater(SceneNpc* pPet);
  bool checkPetCanEat(SceneNpc* pPet);
private:
  DWORD calcFoodNumProgress(DWORD dwTotalStep);
  void setStatus1(DWORD dwProgress);
  void setStatus2(DWORD dwProgress);
  void updateClearTime();
  void delIter();
  bool isInDel(QWORD charId) { return m_setDel.find(charId) != m_setDel.end(); }
public:
  virtual bool isScreenLimit(SceneUser* pUser);
  virtual bool needCheckScreenLimit() { return true; }
public: 
  DWORD m_dwItemId = 0;
  DWORD m_dwItemNum = 0;
  DWORD m_dwItemTotalNum = 0;
  QWORD m_qwOwnerId = 0;
  string m_strOwnerName ;
  Cmd::EEatPower m_power = EEATPOWR_ALL;
  typedef std::pair<QWORD/*charid*/, FoodEaterInfo/*time and progress*/> TEaterPair;
  std::vector<TEaterPair> m_eaters;
  DWORD m_dwProgress = 0;
  bool m_bOver = false;
  std::set<QWORD> m_setDel;
  bool m_bSys = true;    //是否是系统招出来的料理
};


enum ECatchPetState
{
  ECATCHPETSTATE_MIN = 0,
  ECATCHPETSTATE_SKILL = 1,
  ECATCHPETSTATE_TRANSFORM = 2,
  ECATCHPETSTATE_CARTOON = 3,
  ECATCHPETSTATE_RESULT = 4,
  //ECATCHPETSTATE_END = 4,
};

class CatchPetNpc : public SceneNpc, public xObjectPool<CatchPetNpc>
{
  public:
    CatchPetNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~CatchPetNpc();
  public:
    bool catchMe();
    void onCatchOther();
    void addCatchValue(DWORD value);
    void onStopCatch();

    SceneUser* getMasterUser() const;
    void setNoDisperse() { m_dwDisperseTime = 0; }
    bool checkFullValue() { return m_dwCatchValue >= 100; }

    bool initMasterNpc(DWORD monsterID);
    DWORD getMasterNpcID() { return m_dwMasterNpcID; }
    void onUserLeaveScene();
  protected:
    virtual bool init(const SNpcCFG* pCFG, const NpcDefine &def);
    virtual void onOneSecTimeUp(QWORD curMSec);
    virtual void sendExtraInfo(SceneUser* user);
    virtual void onNpcDie();
  private:
    void processCatch(QWORD curMSec);
    void changeCatchState(ECatchPetState eState, DWORD stateEndTime) { m_eCatchState = eState; m_dwCatchTimerTick = stateEndTime; }
  private:
    DWORD m_dwCatchValue = 0; // 捕获值

    DWORD m_dwDisperseTime = 0; // 删除时间
    DWORD m_dwLastInteractionTime = 0; // 上一次交互时间
    DWORD m_dwOfflineDelTime = 0; // 玩家下线设置删除时间, 回来后清空(掉线保护)

    bool m_bCatching = false;
    bool m_bCatchResult = false;

    ECatchPetState m_eCatchState = ECATCHPETSTATE_MIN;
    DWORD m_dwCatchTimerTick = 0;
    DWORD m_dwMasterNpcID = 0;
};

class PetNpc: public SceneNpc, public xObjectPool<CatchPetNpc>
{
  public:
    PetNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~PetNpc();
  public:
    virtual void onNpcDie();
    virtual SceneUser* getMasterUser() const;
  public:
    void addActiveSkills(TSetDWORD& skills) { m_setActiveSkills.insert(skills.begin(), skills.end()); }
    void resetActiveSkills(TSetDWORD& skills);// { m_setActiveSkills.clear(); m_setActiveSkills.insert(skills.begin(), skills.end()); }
    DWORD getModifiedSkill(DWORD skillid) const;
  public:
    virtual DWORD getSkillLv(DWORD skillGroupID);
    virtual void useSuperSkill(DWORD skillid, QWORD targetid, xPos pos, bool specpos = false);
    virtual bool isScreenLimit(SceneUser* pUser);
    virtual bool needCheckScreenLimit() { return true; }
    virtual bool isHideUser(SceneUser* pUser);
    virtual bool needCheckHideUser() { return true; }

    virtual bool inGuildZone();
    virtual bool inSuperGvg();
  private:
    TSetDWORD m_setActiveSkills;
};

// 生命体
class BeingNpc : public SceneNpc, public xObjectPool<BeingNpc>
{
  public:
    BeingNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~BeingNpc();

    virtual SceneUser* getMasterUser() const;
    virtual DWORD getMasterCurLockID() const;
    virtual void onNpcDie();
    virtual bool canUseSkill(const BaseSkill* skill);
  public:
    virtual DWORD getSkillLv(DWORD skillGroupID);
    virtual bool isScreenLimit(SceneUser* pUser);
    virtual bool needCheckScreenLimit() { return true; }
    virtual bool isHideUser(SceneUser* pUser);
    virtual bool needCheckHideUser() { return true; }

  public:
    virtual bool inGuildZone();
    virtual bool inSuperGvg();
};

//贤者召唤的 元素精灵
class ElementElfNpc : public SceneNpc, public xObjectPool<ElementElfNpc>
{
  public:
    ElementElfNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def);
    virtual ~ElementElfNpc();

    virtual SceneUser* getMasterUser() const;
    virtual DWORD getMasterCurLockID() const;
};

// 亡者boss
class DeadNpc : public MonsterNpc, public xObjectPool<DeadNpc>
{
  public:
    DeadNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : MonsterNpc(id, pCFG, def) {}
    virtual ~DeadNpc() {}
};

// 世界boss
class WorldNpc : public MonsterNpc, public xObjectPool<DeadNpc>
{
  public:
    WorldNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : MonsterNpc(id, pCFG, def) {}
    virtual ~WorldNpc() {}
};

