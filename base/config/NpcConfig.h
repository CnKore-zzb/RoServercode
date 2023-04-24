/**
 * @file NpcConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-17
 */

#pragma once

#include "xTools.h"
#include "xSingleton.h"
#include "SceneManual.pb.h"

using std::vector;
using std::map;
using std::pair;
using std::string;
using namespace Cmd;

const DWORD NPC_DREAMMIRROR_1 = 4783;
const DWORD NPC_DREAMMIRROR_2 = 4786;
const DWORD NPC_DREAMMIRROR_3 = 4787;

// config data
enum ENpcType
{
  ENPCTYPE_MIN = 0,
  ENPCTYPE_NPC = 1,
  ENPCTYPE_GATHER = 2,
  ENPCTYPE_MONSTER = 3,
  ENPCTYPE_MINIBOSS = 4,
  ENPCTYPE_MVP = 5,
  ENPCTYPE_SMALLMINIBOSS = 6,   // use for tower monster(rand summon no config) only
  ENPCTYPE_WEAPONPET = 7,
  ENPCTYPE_SKILLNPC = 8,
  ENPCTYPE_CATCHNPC = 9,
  ENPCTYPE_PETNPC = 10,
  ENPCTYPE_FOOD = 11,    //场景上的料理
  ENPCTYPE_BEING = 12,
  ENPCTYPE_FRIEND = 13,    //友军npc
  ENPCTYPE_ELEMENTELF = 14, //贤者元素精灵
  ENPCTYPE_MAX,
};

enum ERaceType
{
  ERACE_MIN = 0,
  ERACE_ANIMAL = 1,
  ERACE_HUMAN = 2,
  ERACE_DEMON = 3,
  ERACE_PLANT = 4,
  ERACE_UNDEAD = 5,
  ERACE_MONSTER = 6,
  ERACE_FISH = 7,
  ERACE_ANGEL = 8,
  ERACE_INSECT = 9,
  ERACE_DRAGON = 10,
  ERACE_MAX = 11
};

enum ENatureType
{
  ENature_MIN = 0,
  ENature_WIND = 1,
  ENature_EARTH = 2,
  ENature_WATER = 3,
  ENature_FIRE = 4,
  ENature_NEUTRAL = 5,
  ENature_HOLY = 6,
  ENature_SHADOW = 7,
  ENature_GHOST = 8,
  ENature_UNDEAD = 9,
  ENature_POISON = 10,
  ENature_MAX
};

enum ENpcZoneType
{
  ENPCZONE_MIN = 0,
  ENPCZONE_FIELD = 1,
  ENPCZONE_TASK = 2,
  ENPCZONE_ENDLESSTOWER = 3,
  ENPCZONE_LABORATORY = 4,
  ENPCZONE_SEAL = 5,
  ENPCZONE_DOJO = 6,
  ENPCZONE_GUILD = 7,
  ENPCZONE_GVGMONSTER = 8,
  ENPCZONE_PVECARD = 9,
  ENPCZONE_MVPBATTLE = 10,
  ENPCZONE_DOJOFIGHT = 11,
  ENPCZONE_DEAD = 12,
  ENPCZONE_WORLD = 13,
  ENPCZONE_RAIDDEADBOSS = 14,
  ENPCZONE_MAX
};

enum ENpcState
{
  ENPCSTATE_MIN = 0,
  ENPCSTATE_NORMAL,
  ENPCSTATE_ATTACK,
  ENPCSTATE_MOVE,
  ENPCSTATE_PICKUP,
  ENPCSTATE_WAIT,
  ENPCSTATE_CAMERA,
  ENPCSTATE_NAUGHTY,
  ENPCSTATE_SMILE,
  ENPCSTATE_SEEDEAD,
  ENPCSTATE_SLEEP,
  ENPCSTATE_GOBACK,
  ENPCSTATE_RUNAWAY,
  ENPCSTATE_MAX
};

enum ESealMonType
{
  ESEALMONTYPE_MIN = 0,
  ESEALMONTYPE_TEAM = 1,
  ESEALMONTYPE_PERSONAL = 2,
  ESEALMONTYPE_ACTIVITY = 3,
  ESEALMONTYPE_MAX,
};

enum ENpcFunction
{
  ENPCFUNCTION_MIN = 0,
  ENPCFUNCTION_WANTED = 101,
  ENPCFUNCTION_EQUIP_REFINE = 300,
  ENPCFUNCTION_EQUIP_DECOMPOSE = 302,
  ENPCFUNCTION_EQUIP_NORMAL_STRENGTH = 303,
  ENPCFUNCTION_EQUIP_ENCHANT_PRIMARY = 305,
  ENPCFUNCTION_EQUIP_ENCHANT_MEDIUM = 306,
  ENPCFUNCTION_EQUIP_ENCHANT_SENIOR = 307,
  ENPCFUNCTION_EQUIP_UPGRADE = 330,
  ENPCFUNCTION_SHOP_NORMAL = 600,
  ENPCFUNCTION_SHOP_MATERIAL = 610,
  ENPCFUNCTION_SHOP_WEAPON = 700,
  ENPCFUNCTION_SHOP_DEFENCE = 750,
  ENPCFUNCTION_SHOP_GARDEN = 800,
  ENPCFUNCTION_SHOP_FRIENDSHIP = 850,
  ENPCFUNCTION_SHOP_NEWYEAR = 901,
  ENPCFUNCTION_SHOP_HAIR = 950,
  ENPCFUNCTION_SHOP_MANUAL = 1400,
  ENPCFUNCTION_CARD_EXCHANGE = 1700,
  ENPCFUNCTION_CARD_COMPOSE = 1701,
  ENPCFUNCTION_CARD_DECOMPOSE = 1702,
  ENPCFUNCTION_SELL = 2100,
  ENPCFUNCTION_LOTTERY = 2200,
  ENPCFUNCTION_SHOP_RANDOM = 3000,
  ENPCFUNCTION_GUILD_PRAY = 4023,
  ENPCFUNCTION_GUILD_GVGPRAY = 4038,
  ENPCFUNCTION_EQUIP_SPECIAL_STRENGTH = 4500,
  ENPCFUNCTION_EQUIP_SPECIAL_REFINE = 4501,
};
enum ENpcFunctionParam
{
  ENPCFUNCTIONPARAM_NORMALSELL = 1,
  ENPCFUNCTIONPARAM_HIGHSELL = 2,
};

// npc skill config
enum ENpcSkillCond
{
  ENPCSKILLCOND_MIN = 0,
  ENPCSKILLCOND_SELFHPLESS = 1,
  ENPCSKILLCOND_SERVANTHPLESS = 2,
  ENPCSKILLCOND_SERVANTNUMLESS = 3,
  ENPCSKILLCOND_ATTACKTIME = 4,
  ENPCSKILLCOND_SELFRANGE = 5,
  ENPCSKILLCOND_MAX
};

//npc features
enum ENpcFeaturesParam
{
  ENPCFEATURESPARAM_CHANGELINEPUNISH = 1,
  ENPCFEATURESPARAM_CHAINDOUBLE = 2,
  //ENP.. = 4,// only client
  //ENP.. = 8,// only client
  ENPCFEATURESPARAM_FAKEMINI = 16,
  ENPCFEATURESPARAM_USEDEFINEPOS = 32,
};

struct SNpcSkillCond
{
  ENpcSkillCond eCondition = ENPCSKILLCOND_MIN;

  TVecDWORD vecParams;

  SNpcSkillCond() {}
};
typedef vector<SNpcSkillCond> TVecSkillCond;
struct SNpcSkill
{
  DWORD dwSkillID = 0;
  DWORD dwRate = 0;
  DWORD dwShareCD = 0;

  ENpcState eState = ENPCSTATE_MIN;

  TVecSkillCond vecCond;

  SNpcSkill() {}
};
typedef vector<SNpcSkill> TVecNpcSkill;
struct SNpcSkillGroup
{
  DWORD dwGroupID = 0;
  DWORD dwMaxRate = 0;

  TVecNpcSkill vecSkill;

  SNpcSkillGroup() {}

  const SNpcSkill& randOneSkill() const;
  const SNpcSkill* getSkill(DWORD dwSkillID) const;
};
typedef map<DWORD, SNpcSkillGroup> TMapNpcSkillGroup;

struct STalkFollow
{
  DWORD dwTalkID = 0;
  DWORD dwRange = 0;
  vector<pair<DWORD, DWORD>> vecNpcTalkID;
};

struct NpcTalk
{
  DWORD folRadius = 0;
  DWORD odds = 0;
  TVecDWORD followerIDs;
  TVecDWORD normalTalkIDs;
  map<DWORD, STalkFollow> mapTalk2Follow;
  pair<DWORD, DWORD> talkTime;
  map<string, TVecDWORD> mapType2Talk;

  DWORD getTalkByType(const string& key) const
  {
    auto m = mapType2Talk.find(key);
    if (m == mapType2Talk.end())
      return 0;
    TVecDWORD vecIDs = m->second;
    DWORD size = vecIDs.size();
    if (size == 0) return 0;
    DWORD index = randBetween(0, 1000) % size;
    return vecIDs[index];
  }
};

struct NpcFigure
{
  DWORD body = 0;
  DWORD haircolor = 0;
  DWORD bodycolor = 0;
  DWORD lefthand = 0;
  DWORD righthand = 0;
  DWORD hair = 0;
  DWORD head = 0;
  DWORD wing = 0;
  DWORD mount = 0;
  DWORD bodySize = 0;
  DWORD shadercolor = 0;
  DWORD face = 0;
  DWORD tail = 0;
  DWORD eye = 0;
};

struct SNpcFunctionCFG
{
  map<ENpcFunction, TVecDWORD> mapTypes;

  //TSetDWORD setRequireFuncs;
  map<DWORD, string> mapRequireFuncs;
  SNpcFunctionCFG() {}
  const TVecDWORD& getFunctionParam(ENpcFunction eType) const;
  void addNpcFunction(map<DWORD, string>& mapFun);
  void delNpcFunction(TSetDWORD& setFun);
  bool hasFunction(ENpcFunction eType) const { return mapTypes.find(eType) != mapTypes.end(); }
};

enum EReactType
{
  EREACTTYPE_MIN = 0,
  EREACTTYPE_FRIEND = 1,
  EREACTTYPE_ENEMY = 2,
};

struct SReactData
{
  DWORD dwStatus = 0;

  TVecDWORD vecItems;
  TVecDWORD vecEquips;
};

struct SNpcReaction
{
  map<EReactType, SReactData> mapReactions;

  bool isReaction(EReactType etype) const { return mapReactions.find(etype) != mapReactions.end(); }
  SNpcReaction() {}
  const SReactData* getReaction(EReactType eType) const;
};

struct SNpcCFG
{
  DWORD dwID = 0;
  DWORD dwLevel = 0;
  DWORD dwNormalSkillID = 0;
  DWORD dwBaseAI = 0;
  DWORD dwGroupID = 0;
  DWORD dwMvpReward = 0;
  DWORD dwEvoID = 0;
  DWORD dwEvoTime = 0;
  DWORD dwEvoRate = 0;
  DWORD dwHandReward = 0;
  DWORD dwProtectAtkLv = 0; // 与define并列,这个判断优先 2016-10-30 15:32 申林&老邢
  DWORD dwCopySkill = 0;
  DWORD dwRandPosTime = 0;
  bool bReplaceSkill = false;

  SDWORD swAdventureValue = 0;

  QWORD qwBaseExp = 0;
  QWORD qwJobExp = 0;

  float fMoveSpd = 0.0f;
  float fScale = 0.0f;

  ENpcType eNpcType = ENPCTYPE_MIN;
  ERaceType eRaceType = ERACE_MIN;
  ENpcZoneType eZoneType = ENPCZONE_MIN;
  ENatureType eNatureType = ENature_MIN;
  EProfession eProfession = EPROFESSION_MIN;
  EManualLockMethod eLockType = EMANUALLOCKMETHOD_MIN;

  TVecDWORD vecRewardIDs;
  TVecDWORD vecAdvBuffIDs;
  TVecAttrSvrs vecAttrs;

  TVecItemInfo vecManualItems;

  string strName;
  string strMapIcon;

  TMapNpcSkillGroup mapSkillGroup;

  NpcTalk talk;
  NpcFigure figure;

  DWORD dwSmileEmoji = 0;
  DWORD dwSmileAction = 0;
  DWORD dwBehaviours = 0;
  DWORD dwTowerUnlock = 0;
  DWORD dwChainAtkLmt = 0;
  DWORD dwMoveFrequency = 0;
  DWORD dwDefaultGear = 0;
  DWORD dwPeriodKillCnt = 0;
  DWORD dwFeatures = 0;
  DWORD dwCarryMoney = 0;
  bool bHide = false;
  bool bPredatory = false;
  bool bStar = false;
  bool bNormalDisplay = false;
  bool bNormalHide = false;
  bool bCanBeMonster = false; // 针对Table_Npc
  DWORD dwGender = 0;

  SNpcFunctionCFG stNpcFunc;

  SNpcReaction stNpcReaction;
  TVecDWORD vecTransformSkill;
  TVecDWORD vecBuffs;

  TSetDWORD setSuperAI;
  TSetDWORD setTransRecIDs;
  TSetDWORD setImmuneSkills;
  TSetDWORD setCommonRewards;
  TSetDWORD setFoodRewards;

  TVecDWORD vecRelevancyIDs;

  SNpcCFG() {}
  const SNpcSkillGroup* getSkillGroup(DWORD dwIndex) const;

  bool isTransRecIDs(DWORD dwID) const { return setTransRecIDs.find(dwID) != setTransRecIDs.end(); }
  bool isFieldMonster() const { return !bStar && eNpcType != ENPCTYPE_MINIBOSS && eNpcType != ENPCTYPE_MVP; }

  bool getFeaturesByType(ENpcFeaturesParam eType) const;

  DWORD getShowByType(EUserDataType eDataType) const
  {
    switch(eDataType)
    {
      case EUSERDATATYPE_BODY:
        return figure.body;
      case EUSERDATATYPE_LEFTHAND:
        return figure.lefthand;
      case EUSERDATATYPE_RIGHTHAND:
        return figure.righthand;
      case EUSERDATATYPE_HEAD:
        return figure.head;
      case EUSERDATATYPE_BACK:
        return figure.wing;
      case EUSERDATATYPE_HAIR:
        return figure.hair;
      case EUSERDATATYPE_HAIRCOLOR:
        return figure.haircolor;
      case EUSERDATATYPE_FACE:
        return figure.face;
      case EUSERDATATYPE_TAIL:
        return figure.tail;
      case EUSERDATATYPE_MOUNT:
        return figure.mount;
      default:
        break;
    }
    return 0;
  }
};
typedef map<DWORD, SNpcCFG> TMapNpcCFG;

struct SNpcCharacterCFG
{
  DWORD dwID = 0;

  TVecDWORD mutexIDs;
  TVecAttrSvrs nAttrs;

  SNpcCharacterCFG() {}
};
typedef map<DWORD, SNpcCharacterCFG> TMapNpcCharacterCFG;

// ----------------------------------------------------------副本随机招怪配置
struct SRandomMonsterCFG
{
  DWORD dwMonsterID = 0;

  ENpcType eNpcType = ENPCTYPE_MIN;
  map<DWORD, TSetDWORD> mapMapID2GroupIDs; // 可以出现的地图id->uniqueid

  DWORD dwWeight = 0; // 配置表raw weight

  TSetDWORD setBossReward; // 击杀奖励
};
typedef map<DWORD, SRandomMonsterCFG> TMapRandMonsterCFG;
typedef vector<pair<DWORD, DWORD>> TVecRandMonsterWeight;
typedef map<DWORD, map<DWORD, TVecRandMonsterWeight>> TMapMapUnique2WeightInfo;// (mapid, (uniqueid, vector<monsterid, format weight>))
typedef map<DWORD, map<ENpcType, TSetDWORD>> TMapMapNpcType2UniqueID; // (mapid, (npctype, uniqueids))
// ----------------------------------------------------------副本随机招怪配置

// ----------------------------------------------------------副本亡者boss
enum ERaidDeadBossType
{
  ERAID_DEADBOSSTYPE_MIN = 0,
  ERAID_DEADBOSSTYPE_MVPBATTLE = 1,
  ERAID_DEADBOSSTYPE_PVECARD = 2,
  ERAID_DEADBOSSTYPE_TOWER = 3,
  ERAID_DEADBOSSTYPE_GUILD = 4,
};
struct SRaidDeadBossCFG
{
  DWORD dwMonsterID = 0;
  ERaidDeadBossType eRaidBossType = ERAID_DEADBOSSTYPE_MIN;
  DWORD dwLevel = 0;
  TSetDWORD setRewards;
};
typedef map<DWORD, SRaidDeadBossCFG> TMapRaidDeadBossCFG;
typedef map<ERaidDeadBossType, map<DWORD, TSetDWORD>> TMapType2Lv2DeadBossIDs; // (type,(lv, cfg))
// ----------------------------------------------------------副本亡者boss

// config
class NpcConfig : public xSingleton<NpcConfig>
{
  friend class xSingleton<NpcConfig>;
  friend class RewardConfig;
  private:
    NpcConfig();
  public:
    virtual ~NpcConfig();

    bool loadConfig();
    bool checkConfig();

    const SNpcCFG* getNpcCFG(DWORD dwID) const;
    const SNpcCharacterCFG* getCharacterCFG(DWORD dwID) const;

    bool collectCharacter(TVecDWORD& vecIDs);
    const TVecAttrSvrs* getCharacAttrs(DWORD id);

    ENpcType getNpcType(const string& str) const;
    ENpcZoneType getZoneType(const string& str) const;
    ERaceType getRaceType(const string& str) const;
    ENatureType getNatureType(const string& str) const;
    DWORD getShape(const string& str) const;
  private:
    bool loadNpcConfig(const string& file);
    bool loadNpcTalkConfig();
    bool loadCharacterConfig();
    bool loadMonsterSkill();
    bool loadMonsterEvoConfig();
    bool loadRandomMonsterConfig();
    bool loadRaidDeadBossConfig();

    EProfession getProfession(const string& str) const;
    ENpcSkillCond getSkillCondition(const string& str) const;
    ENpcState getNpcState(const string& str) const;
    EReactType getReactType(const string& str) const;
  private:
    TMapNpcCFG m_mapNpcCFG;
    TMapNpcCharacterCFG m_mapCharacterCFG;
    std::map<DWORD, std::vector<DWORD>> m_mapSkillGroupID2VecMonsterIDs;


  public:
    bool isMonster(DWORD id) const { return m_setMonsterIDs.find(id) != m_setMonsterIDs.end(); }
  private:
    TSetDWORD m_setMonsterIDs;

  public:
    bool getRandomMonsterByType(DWORD mapid, ENpcType eType, DWORD num, TVecDWORD& vecIDs) const;
    DWORD getRandMonsterByGroup(DWORD mapid, DWORD grpid) const;
    const SRandomMonsterCFG* getRandomMonsterCFG(DWORD monsterid) const;
  private:
    TMapRandMonsterCFG m_mapRandomMonsterCFG;
    TMapMapUnique2WeightInfo m_mapMapUnique2WeightInfo;//(mapid, (uniqueid, <monsterid, format weight>))
    TMapMapNpcType2UniqueID m_mapMapNpcType2UniqueID; //(mapid, (npctype, uniqueid))

    // 副本亡者boss
  public:
    const SRaidDeadBossCFG* getRaidDeadBossCFG(DWORD monsterid) const;
    DWORD getOneRandomRaidDeadBoss(ERaidDeadBossType eType, DWORD lv) const;
  private:
    TMapRaidDeadBossCFG m_mapRaidDeadBossCFG;
    TMapType2Lv2DeadBossIDs m_mapType2Lv2DeadBossIDs;
};

