#include "SceneServer.h"
#include "SceneManager.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "SceneTrapManager.h"
#include "SceneActManager.h"
#include "SceneBoothManager.h"
#include "SceneUser.h"
#include "GatewayCmd.h"
#include "SystemCmd.pb.h"
#include "xLuaTable.h"
#include "TableManager.h"
#include "SkillConfig.h"
#include "Quest.h"
#include "FuBen.h"
#include "Stage.h"
#include "Menu.h"
#include "BufferManager.h"
#include "UserCmd.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "SceneTip.h"
#include "SkillManager.h"
#include "SceneTower.h"
#include "Laboratory.h"
#include "TestQuest.h"
//#include "SceneConfig.h"
#include "MusicBoxManager.h"
#include "RedisManager.h"
#include "ActivityManager.h"
#include "Item.h"
#include "ItemManager.h"
#include "GMCommandRuler.h"
#include "ActivityManager.h"
#include "Var.h"
#include "SuperAI.h"
#include "NpcAI.h"
#include "SceneNpc.h"
#include "PatchManager.h"
#include "SceneUserDataThread.h"
#include "WeddingSCmd.pb.h"
#include "PveCardEffect.h"
#include "UserBeing.h"
#include "SceneShop.h"
#include "ExchangeShopManager.h"
#include "BossMgr.h"

#ifndef _LX_DEBUG
ExecutionTime_setInterval(300);
#else
ExecutionTime_setInterval(60);
#endif

//SceneServer *thisServer = 0;
extern ConfigEnum ConfigEnums[];

SceneNpcManager& getNpcManager()
{
  return SceneNpcManager::getMe();
}

ActivityManager& getScActMgr()
{
  return ActivityManager::getMe();
}

SceneManager& getSceneMgr()
{
  return SceneManager::getMe();
}

GMCommandRuler& getCMCommand()
{
  return GMCommandRuler::getMe();
}

PatchManager& getPatchManager()
{
  return PatchManager::getMe();
}

ItemManager& getItemManager()
{
  return ItemManager::getMe();
}

xSceneEntryDynamic* getEntryObj(QWORD guid)
{
  return xSceneEntryDynamic::getEntryByID(guid);
}
SceneServer::SceneServer(OptArgs &args) : ZoneServer(args), m_oOneSecTimer(1), m_oTimer(10), m_oTickOneMin(60), m_oDayTimer(5,0)
{
}

SceneServer::~SceneServer()
{
}

void SceneServer::v_final()
{
  XLOG << "[" << getServerName() << "],v_final" << XEND;

  SceneUserManager::delMe();

  ActivityManager::delMe();
  BufferManager::delMe();
  SkillManager::delMe();
  BufferManager::delMe();
  QuestManager::delMe();
  SceneManager::delMe();
  SuperAIManager::delMe();
  PveCardManager::delMe();
  DressUpStageMgr::delMe();


  SceneNpcManager::getMe().final();
  SceneTrapManager::getMe().final();
  SceneActManager::getMe().final();
  SceneBoothManager::getMe().final();
  FuBen::final();
  sleep(3);

  SceneUserLoadThread::getMe().thread_stop();
  SceneUserLoadThread::getMe().thread_join();

  SceneUserSaveThread::getMe().thread_stop();
  SceneUserSaveThread::getMe().thread_join();

  ZoneServer::v_final();
}

void SceneServer::v_closeServer(xNetProcessor *np)
{
  if (!np) return;
}

bool SceneServer::loadConfig()
{
  if (!addDataBase(REGION_DB, false))
  {
    XERR << "[数据库],DBPool,初始化失败," << REGION_DB << XEND;
    return false;
  }

  // 配置函数注册
  ConfigEnums[misc].loadfunc = []()
  {
    ItemConfig::getMe().loadLotteryConfig();
    return true;
  };
  ConfigEnums[item].loadfunc = []()
  {
    SceneUserManager::getMe().reloadconfig(item);
    return true;
  };
  ConfigEnums[superai].loadfunc = []()
  {
    return SuperAIManager::getMe().init();
  };
  ConfigEnums[npc].loadfunc = []()
  {
    SceneNpcManager::getMe().reload();
    return true;
  };
  ConfigEnums[skill].loadfunc = []()
  {
    bool b = SkillManager::getMe().init();
    //SceneUserManager::getMe().reloadconfig(skill);
    return b;
  };
  ConfigEnums[scene].loadfunc = []()
  {
    bool b = true;
    if (SceneManager::getMe().loadSceneConfig() == false)
      b = false;
    if (SceneManager::getMe().init() == false)
      b = false;
    return b;
  };
  ConfigEnums[quest].loadfunc = []()
  {
    if (!thisServer->addDataBase(REGION_DB, false))
      return false;
    bool b = QuestManager::getMe().init();
    SceneUserManager::getMe().reloadconfig(ConfigType::quest);
    //SceneNpcManager::getMe().reload();
    thisServer->delDataBase();
    return b;
  };
  ConfigEnums[manual].loadfunc = []()
  {
    SceneUserManager::getMe().reloadconfig(ConfigType::manual);
    return true;
  };
  ConfigEnums[buffer].loadfunc = []()
  {
    bool b = BufferManager::getMe().loadConfig();
    SceneUserManager::getMe().reloadconfig(ConfigType::buffer);
    SceneNpcManager::getMe().reloadconfig(ConfigType::buffer);
    return b;
  };
  ConfigEnums[raid].loadfunc = []()
  {
    return FuBen::loadConfig();
  };
  ConfigEnums[lab].loadfunc = []()
  {
    return Laboratory::loadConfig();
  };
  ConfigEnums[music].loadfunc = []()
  {
    if (!thisServer->addDataBase(REGION_DB, false))
      return false;
    bool b = MusicBoxManager::getMe().init();
    thisServer->delDataBase();
    return b;
  };
  ConfigEnums[baselevel].loadfunc = []()
  {
    SceneUserManager::getMe().reloadconfig(baselevel);
    return true;
  };
  ConfigEnums[joblevel].loadfunc = []()
  {
    SceneUserManager::getMe().reloadconfig(joblevel);
    return true;
  };
  ConfigEnums[pet].loadfunc = []()
  {
    SceneUserManager::getMe().reloadconfig(pet);
    return true;
  };
  ConfigEnums[achieve].loadfunc = []()
  {
    SceneUserManager::getMe().reloadconfig(achieve);
    return true;
  };
  ConfigEnums[being].loadfunc = []()
  {
    SceneUserManager::getMe().reloadconfig(being);
    return true;
  };
  ConfigEnums[pvecard].loadfunc = []()
  {
    PveCardManager::getMe().init();
    return true;
  };
  ConfigEnums[exchangeshop].loadfunc = []()
  {
    bool b = ExchangeShopManager::getMe().loadConfig();
    SceneUserManager::getMe().reloadconfig(ConfigType::exchangeshop);
    return b;
  };
  ConfigEnums[exchangeworth].loadfunc = []()
  {
    return ExchangeShopManager::getMe().loadWorthConfig();
  };
  ConfigEnums[boss].loadfunc = []()
  {
    return BossMgr::getMe().loadConfig();
  };

  // 脚本函数注册
  registerLuaFunc();

  /* 加载顺序不要变 start */
  /* 初始化配置加到ConfigManager
   * 或者 SceneConfig中
   * 注意初始化和重加载
   */
  bool bCorrect = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bCorrect = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }
  // base配置
  if (ConfigManager::getMe().loadSceneConfig() == false)
  {
    bCorrect = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }
  // 场景配置
  /*if (SceneConfig::getMe().loadConfig() == false)
  {
    bCorrect = false;
    CHECK_LOAD_CONFIG("SceneConfig");
  }*/
  /* 加载顺序不要变 end */

  // 名单加载
  if (PatchManager::getMe().loadPatchList() == false)
  {
    bCorrect = false;
    CHECK_LOAD_CONFIG("PatchManager:Quest");
  }

//#ifndef _LX_DEBUG
  if (xServer::isOuter() == false)
    TestQuest::print();
//#endif

  delDataBase();

  return BaseConfig::getMe().configCheck() == true ? bCorrect : true;
}

bool SceneServer::v_init()
{
  AttributeValidEnum::initAttributeValidEnum();
  bool ret = false;
  ret =  loadConfig();
  if (ret == false) return false;

  ActivityManager::getMe().init();
  SceneUserManager::getMe().init();

  if (!SceneUserLoadThread::getMe().thread_start())
  {
    XERR << "[SceneUserLoadThread]" << "创建失败" << XEND;
    return false;
  }
  if (!SceneUserSaveThread::getMe().thread_start())
  {
    XERR << "[SceneUserSaveThread]" << "创建失败" << XEND;
    return false;
  }

  return ret;
}

void SceneServer::init_ok()
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SCENEUSER_NUM, thisServer->getZoneID(), thisServer->getServerName());
  RedisManager::getMe().delData(key);

  ZoneServer::init_ok();
}

void SceneServer::registerLuaFunc()
{
  LuaManager::getMe().setregister([](){
    lua_State* _L = LuaManager::getMe().getLuaState();

    GLOBAL(getNpcManager);
    GLOBAL(getScActMgr);
    GLOBAL(getSceneMgr);
    GLOBAL(getCMCommand);
    GLOBAL(getPatchManager);
    GLOBAL(getItemManager);
    GLOBAL(getEntryObj);

    // SceneNpcManager
    CLASS_ADD(SceneNpcManager);
    CLASS_FUNC(SceneNpcManager, createNpcLua);

    // ActivityManager
    /*CLASS_ADD(ActivityManager);
    CLASS_FUNC(ActivityManager, sendBCatInform);
    CLASS_FUNC(ActivityManager, sendBCatStart);
    CLASS_FUNC(ActivityManager, sendBCatUFOPos);*/

    // SceneManager
    CLASS_ADD(SceneManager);
    CLASS_FUNC(SceneManager, getCreatureCount);

    // GMCommandRuler
    CLASS_ADD(GMCommandRuler);
    CLASS_FUNC(GMCommandRuler, luaGMCmd);
    CLASS_FUNC(GMCommandRuler, finishQuest);

    // PatchManager
    CLASS_ADD(PatchManager);
    CLASS_FUNC(PatchManager, isPatchChar);

    // ItemManager
    CLASS_ADD(ItemManager);
    CLASS_FUNC(ItemManager, createItemLua);

    // attr
    ENUM(EATTRTYPE_STR);
    ENUM(EATTRTYPE_INT);
    ENUM(EATTRTYPE_AGI);
    ENUM(EATTRTYPE_DEX);
    ENUM(EATTRTYPE_VIT);
    ENUM(EATTRTYPE_LUK);

    ENUM(EATTRTYPE_ATK);
    ENUM(EATTRTYPE_ATKPER);
    ENUM(EATTRTYPE_DEF);
    ENUM(EATTRTYPE_DEFPER);
    ENUM(EATTRTYPE_MATK);
    ENUM(EATTRTYPE_MATKPER);
    ENUM(EATTRTYPE_MDEF);
    ENUM(EATTRTYPE_MDEFPER);
    ENUM(EATTRTYPE_MAXHP);
    ENUM(EATTRTYPE_MAXHPPER);
    ENUM(EATTRTYPE_MAXSP);
    ENUM(EATTRTYPE_MAXSPPER);
    ENUM(EATTRTYPE_HP);
    ENUM(EATTRTYPE_SP);
    ENUM(EATTRTYPE_HIT);
    ENUM(EATTRTYPE_FLEE);
    ENUM(EATTRTYPE_CRI);
    ENUM(EATTRTYPE_CRIRES);
    ENUM(EATTRTYPE_CRIDAMPER);
    ENUM(EATTRTYPE_CRIDEFPER);

    ENUM(EATTRTYPE_MOVESPD);
    ENUM(EATTRTYPE_CASTSPD);
    ENUM(EATTRTYPE_RESTORESPD);
    ENUM(EATTRTYPE_SPRESTORESPD);
    ENUM(EATTRTYPE_ITEMRESTORESPD);
    ENUM(EATTRTYPE_ITEMSPRESTORESPD);

    ENUM(EATTRTYPE_REFINE);
    ENUM(EATTRTYPE_MREFINE);

    ENUM(EATTRTYPE_SHOWATK);
    ENUM(EATTRTYPE_SHOWDEF);
    ENUM(EATTRTYPE_SHOWMATK);
    ENUM(EATTRTYPE_SHOWMDEF);
    ENUM(EATTRTYPE_SHOWMAXHP);
    ENUM(EATTRTYPE_SHOWMAXSP);
    ENUM(EATTRTYPE_SHOWHIT);
    ENUM(EATTRTYPE_SHOWFLEE);
    ENUM(EATTRTYPE_SHOWCRI);
    ENUM(EATTRTYPE_SHOWCRIRES);
    ENUM(EATTRTYPE_SHOWATKSPD);
    ENUM(EATTRTYPE_SHOWMOVESPD);
    ENUM(EATTRTYPE_SHOWCASTSPD);
    ENUM(EATTRTYPE_SHOWRESTORESPD);

    // buff type
    ENUM(EBUFFTYPE_ROBREWARD);
    ENUM(EBUFFTYPE_MULTITIME);
    // damage need
    ENUM(EATTRTYPE_IGNOREDEF);
    ENUM(EATTRTYPE_IGNOREMDEF);

    ENUM(EATTRTYPE_DAMREDUC);
    ENUM(EATTRTYPE_MDAMREDUC);

    ENUM(EATTRTYPE_DAMREBOUND);
    ENUM(EATTRTYPE_MDAMREBOUND);
    ENUM(EATTRTYPE_DAMINCREASE);
    ENUM(EATTRTYPE_MDAMINCREASE);

    ENUM(EATTRTYPE_HEALENCPER);
    ENUM(EATTRTYPE_BEHEALENCPER);

    ENUM(EATTRTYPE_FIREDAMPER);
    ENUM(EATTRTYPE_BEFIREDAMPER);

    ENUM(EATTRTYPE_WINDDAMPER);
    ENUM(EATTRTYPE_EARTHDAMPER);
    ENUM(EATTRTYPE_FIREDAMPER);
    ENUM(EATTRTYPE_WATERDAMPER);
    ENUM(EATTRTYPE_NEUTRALDAMPER);
    ENUM(EATTRTYPE_HOLYDAMPER);
    ENUM(EATTRTYPE_SHADOWDAMPER);
    ENUM(EATTRTYPE_BEWINDDAMPER);
    ENUM(EATTRTYPE_BEEARTHDAMPER);
    ENUM(EATTRTYPE_BEFIREDAMPER);
    ENUM(EATTRTYPE_BEWATERDAMPER);
    ENUM(EATTRTYPE_BENEUTRALDAMPER);
    ENUM(EATTRTYPE_BEHOLYDAMPER);
    ENUM(EATTRTYPE_BESHADOWDAMPER);
    ENUM(EATTRTYPE_BRUTEDAMPER);
    ENUM(EATTRTYPE_DEMIHUMANDAMPER);
    ENUM(EATTRTYPE_DEMONDAMPER);
    ENUM(EATTRTYPE_PLANTDAMPER);
    ENUM(EATTRTYPE_DEADLESSDAMPER);
    ENUM(EATTRTYPE_FORMLESSDAMPER);
    ENUM(EATTRTYPE_FISHDAMPER);
    ENUM(EATTRTYPE_ANGLEDAMPER);
    ENUM(EATTRTYPE_INSECTDAMPER);
    ENUM(EATTRTYPE_DRAGONDAMPER);

    ENUM(EATTRTYPE_BRUTERESPER);
    ENUM(EATTRTYPE_DEMIHUMANRESPER);
    ENUM(EATTRTYPE_DEMONRESPER);
    ENUM(EATTRTYPE_PLANTRESPER);
    ENUM(EATTRTYPE_DEADLESSRESPER);
    ENUM(EATTRTYPE_FORMLESSRESPER);
    ENUM(EATTRTYPE_FISHRESPER);
    ENUM(EATTRTYPE_ANGLERESPER);
    ENUM(EATTRTYPE_INSECTRESPER);
    ENUM(EATTRTYPE_DRAGONRESPER);

    ENUM(EATTRTYPE_SMALLDAMPER);
    ENUM(EATTRTYPE_SMALLRESPER);
    ENUM(EATTRTYPE_MIDDAMPER);
    ENUM(EATTRTYPE_MIDRESPER);
    ENUM(EATTRTYPE_BIGDAMPER);
    ENUM(EATTRTYPE_BIGRESPER);

    ENUM(EATTRTYPE_BOSSDAMPER);
    ENUM(EATTRTYPE_BOSSRESPER);

    ENUM(EATTRTYPE_GHOSTDAMPER);
    ENUM(EATTRTYPE_UNDEADDAMPER);
    ENUM(EATTRTYPE_POSIONDAMPER);
    ENUM(EATTRTYPE_BEGHOSTDAMPER);
    ENUM(EATTRTYPE_BEUNDEADDAMPER);
    ENUM(EATTRTYPE_BEPOSIONDAMPER);
    ENUM(EATTRTYPE_ATKATTR);
    ENUM(EATTRTYPE_DEFATTR);
    ENUM(EATTRTYPE_HIDE);
    ENUM(EATTRTYPE_STATEEFFECT);
    ENUM(EATTRTYPE_ATTREFFECT);
    ENUM(EATTRTYPE_ATTREFFECT2);

    // npc type
    ENUM(ENPCTYPE_NPC);
    ENUM(ENPCTYPE_GATHER);
    ENUM(ENPCTYPE_MONSTER);
    ENUM(ENPCTYPE_MINIBOSS);
    ENUM(ENPCTYPE_MVP);
    ENUM(ENPCTYPE_SMALLMINIBOSS);
    ENUM(ENPCTYPE_WEAPONPET);
    ENUM(ENPCTYPE_SKILLNPC);
    ENUM(ENPCTYPE_PETNPC);
    ENUM(ENPCTYPE_BEING);
    ENUM(ENPCTYPE_ELEMENTELF);

    // npc zone type
    ENUM(ENPCZONE_FIELD);
    ENUM(ENPCZONE_TASK);
    ENUM(ENPCZONE_ENDLESSTOWER);
    ENUM(ENPCZONE_LABORATORY);
    ENUM(ENPCZONE_SEAL);
    ENUM(ENPCZONE_DOJO);
    ENUM(ENPCZONE_GUILD);
    ENUM(ENPCZONE_GVGMONSTER);
    ENUM(ENPCZONE_DEAD);
    ENUM(ENPCZONE_WORLD);

    ENUM(ERACE_MONSTER);

    // act type
    ENUM(EACTIVITYTYPE_BCAT);

    // entry type
    ENUM(SCENE_ENTRY_USER);

    // entry type
    ENUM(EVARTYPE_QUEST_WANTED);
    ENUM(EVARTYPE_QUEST_WANTED_RESET);
    ENUM(EVARTYPE_SEAL);
    ENUM(EVARTYPE_LABORATORY);
    ENUM(EVARTYPE_ALTMAN_KILL);

    // manual type
    ENUM(EMANUALTYPE_FASHION);
    ENUM(EMANUALTYPE_MONSTER);
    ENUM(EMANUALTYPE_NPC);
    ENUM(EMANUALTYPE_SCENERY);

    // manual status
    ENUM(EMANUALSTATUS_UNLOCK_CLIENT);

    // achieve cond
    ENUM(EACHIEVECOND_SCENERY_COUNT);
    ENUM(EACHIEVECOND_MONSTER_PHOTO);
    ENUM(EACHIEVECOND_NPC_COUNT);
    ENUM(EACHIEVECOND_COMPOSE);

    // portrait type
    ENUM(EPORTRAITTYPE_PETPORTRAIT);

    // money type
    ENUM(EMONEYTYPE_GARDEN);

    // source type
    ENUM(ESOURCE_QUEST);

    // pack type
    ENUM(EPACKTYPE_MAIN);
    ENUM(EPACKTYPE_EQUIP);
    ENUM(EPACKTYPE_FASHION);
    ENUM(EPACKTYPE_FASHIONEQUIP);
    ENUM(EPACKTYPE_CARD);
    ENUM(EPACKTYPE_STORE);
    ENUM(EPACKTYPE_PERSONAL_STORE);
    ENUM(EPACKTYPE_TEMP_MAIN);
    ENUM(EPACKTYPE_BARROW);
    ENUM(EPACKTYPE_QUEST);

    // relation type
    ENUM(ESOCIALRELATION_TUTOR);
    ENUM(ESOCIALRELATION_STUDENT);

    // quest
    ENUM(EQUESTMETHOD_DEL_ACCEPT);
    ENUM(EQUESTMETHOD_ADD_SUBMIT);

    // attr oper
    ENUM(EATTROPER_ADD);
    ENUM(EATTROPER_SET);

    // attr collect
    ENUM(ECOLLECTTYPE_EQUIP);
    ENUM(ECOLLECTTYPE_GUILD);

    // moveaction
    CLASS_ADD(MoveAction);
    CLASS_FUNC(MoveAction, empty);

    // NpcAI
    CLASS_ADD(NpcAI);
    CLASS_FUNC(NpcAI, moveToPos);

    // SceneNpc
    CLASS_ADD(SceneNpc);
    CLASS_FUNC(SceneNpc, getNpcID);
    CLASS_FUNC(SceneNpc, getNpcType);
    CLASS_FUNC(SceneNpc, getNpcZoneType);
    CLASS_FUNC(SceneNpc, setPointAttr);
    CLASS_FUNC(SceneNpc, getLevel);
    CLASS_FUNC(SceneNpc, getAttr);
    CLASS_FUNC(SceneNpc, setAttr);
    CLASS_FUNC(SceneNpc, getBaseExp);
    CLASS_FUNC(SceneNpc, getJobExp);
    CLASS_FUNC(SceneNpc, getOtherAttr);
    CLASS_FUNC(SceneNpc, getBaseAttr);
    CLASS_FUNC(SceneNpc, getProfession);
    CLASS_FUNC(SceneNpc, getMapID);
    CLASS_FUNC(SceneNpc, getSummonType);

    CLASS_FUNC(SceneNpc, getDojoLevel);
    CLASS_FUNC(SceneNpc, getRoundID);
    CLASS_FUNC(SceneNpc, getSealType);
    CLASS_FUNC(SceneNpc, getEndlessLayer);
    CLASS_FUNC(SceneNpc, isStarMonster);

    CLASS_FUNC(SceneNpc, getMoveAction);
    CLASS_FUNC(SceneNpc, getNpcAI);

    CLASS_FUNC(SceneNpc, get_x);
    CLASS_FUNC(SceneNpc, get_y);
    CLASS_FUNC(SceneNpc, get_z);
    CLASS_FUNC(SceneNpc, setClearState);
    CLASS_FUNC(SceneNpc, getGUID);
    CLASS_FUNC(SceneNpc, getPeriodCnt);
    CLASS_FUNC(SceneNpc, getMasterUser);
    CLASS_FUNC(SceneNpc, getChangeLinePunish);
    CLASS_FUNC(SceneNpc, isFakeMini);

    CLASS_FUNC(SceneNpc, getMapRewardRatio);
    CLASS_FUNC(SceneNpc, getDeadLv);

    // xSceneEntryDynamic
    CLASS_ADD(xSceneEntryDynamic);
    CLASS_FUNC(xSceneEntryDynamic, getAttr);
    CLASS_FUNC(xSceneEntryDynamic, getBuffAttr);
    CLASS_FUNC(xSceneEntryDynamic, getTimeBuffAttr);
    CLASS_FUNC(xSceneEntryDynamic, getPointAttr);
    CLASS_FUNC(xSceneEntryDynamic, getOtherAttr);
    CLASS_FUNC(xSceneEntryDynamic, getBaseAttr);
    CLASS_FUNC(xSceneEntryDynamic, setShowAttr);
    //CLASS_FUNC(xSceneEntryDynamic, setGuildAttr);
    //CLASS_FUNC(xSceneEntryDynamic, setEquipAttr);
    CLASS_FUNC(xSceneEntryDynamic, setAttr);
    CLASS_FUNC(xSceneEntryDynamic, setPointAttr);
    CLASS_FUNC(xSceneEntryDynamic, getBodySize);
    CLASS_FUNC(xSceneEntryDynamic, getLevel);
    CLASS_FUNC(xSceneEntryDynamic, isImmuneSkill);
    CLASS_FUNC(xSceneEntryDynamic, getRaceType);
    //CLASS_FUNC(xSceneEntryDynamic, isBoss);
    CLASS_FUNC(xSceneEntryDynamic, getName);
    CLASS_FUNC(xSceneEntryDynamic, isMyEnemy);
    CLASS_FUNC(xSceneEntryDynamic, getWeaponType);
    CLASS_FUNC(xSceneEntryDynamic, isIgnoreBodyDam);
    CLASS_FUNC(xSceneEntryDynamic, isValidAttrName);
    CLASS_FUNC(xSceneEntryDynamic, getStrAttr);
    CLASS_FUNC(xSceneEntryDynamic, setTempDamageType);
    CLASS_FUNC(xSceneEntryDynamic, getSkillLv);
    CLASS_FUNC(xSceneEntryDynamic, getRandomNum);
    CLASS_FUNC(xSceneEntryDynamic, getNormalSkill);
    CLASS_FUNC(xSceneEntryDynamic, getProfession);
    CLASS_FUNC(xSceneEntryDynamic, isOnPvp);
    CLASS_FUNC(xSceneEntryDynamic, isOnGvg);
    CLASS_FUNC(xSceneEntryDynamic, bDamageAlways1);
    CLASS_FUNC(xSceneEntryDynamic, getArrowID);
    CLASS_FUNC(xSceneEntryDynamic, getRaidType);
    CLASS_FUNC(xSceneEntryDynamic, getSkillStatus);
    CLASS_FUNC(xSceneEntryDynamic, isAlive);
    CLASS_FUNC(xSceneEntryDynamic, isReliveByOther);
    //CLASS_FUNC(xSceneEntryDynamic, getItemUseCnt);
    CLASS_FUNC(xSceneEntryDynamic, isBuffLayerEnough);
    CLASS_FUNC(xSceneEntryDynamic, addBuffDamage);
    CLASS_FUNC(xSceneEntryDynamic, getMapID);
    CLASS_FUNC(xSceneEntryDynamic, getNpcObject);
    CLASS_FUNC(xSceneEntryDynamic, getUserObject);
    CLASS_FUNC(xSceneEntryDynamic, getGuid);
    CLASS_FUNC(xSceneEntryDynamic, setTempAtkAttr);
    CLASS_FUNC(xSceneEntryDynamic, getBuff);
    CLASS_FUNC(xSceneEntryDynamic, getDisWithEntry);
    CLASS_FUNC(xSceneEntryDynamic, doExtraDamage);
    CLASS_FUNC(xSceneEntryDynamic, isInRaid);
    CLASS_FUNC(xSceneEntryDynamic, delSkillBuff);
    CLASS_FUNC(xSceneEntryDynamic, getTempAtkAttr);
    CLASS_FUNC(xSceneEntryDynamic, setMissStillBuff);
    CLASS_FUNC(xSceneEntryDynamic, inGuildZone);
    CLASS_FUNC(xSceneEntryDynamic, getRangeEnemy);
    CLASS_FUNC(xSceneEntryDynamic, addBuff);
    CLASS_FUNC(xSceneEntryDynamic, inSuperGvg);
    CLASS_FUNC(xSceneEntryDynamic, modifyCollect);
    CLASS_FUNC(xSceneEntryDynamic, testAttr);
    CLASS_FUNC(xSceneEntryDynamic, getEvo);
    CLASS_FUNC(xSceneEntryDynamic, canUseWingOfFly);
    CLASS_FUNC(xSceneEntryDynamic, canTeamUseWingOfFly);

    // SceneUser
    CLASS_ADD(SceneUser);
    CLASS_FUNC(SceneUser, setLevel);
    CLASS_FUNC(SceneUser, getLevel);
    CLASS_FUNC(SceneUser, getAttr);
    CLASS_FUNC(SceneUser, getProfession);
    CLASS_FUNC(SceneUser, getTeamMemberCount);
    CLASS_FUNC(SceneUser, hasPatchLoad);
    CLASS_FUNC(SceneUser, addPatchLoad);
    CLASS_FUNC(SceneUser, getMenu);
    CLASS_FUNC(SceneUser, getManual);
    CLASS_FUNC(SceneUser, getUserSceneData);
    CLASS_FUNC(SceneUser, getQuest);
    CLASS_FUNC(SceneUser, getCurFighter);
    CLASS_FUNC(SceneUser, getAddictTime);
    CLASS_FUNC(SceneUser, getTotalBattleTime);
    CLASS_FUNC(SceneUser, getTempID);
    CLASS_FUNC(SceneUser, getCharIDString);
    CLASS_FUNC(SceneUser, getEquipedItemNum);
    CLASS_FUNC(SceneUser, getMainPackageItemNum);
    CLASS_FUNC(SceneUser, getHairInfo);
    CLASS_FUNC(SceneUser, getJobLv);
    CLASS_FUNC(SceneUser, getVar);
    CLASS_FUNC(SceneUser, getTitle);
    CLASS_FUNC(SceneUser, getScenery);
    CLASS_FUNC(SceneUser, getDepositAddictTime);
    CLASS_FUNC(SceneUser, getPortrait);
    CLASS_FUNC(SceneUser, getTeamAddictRatio);
    CLASS_FUNC(SceneUser, getFreyja);
    CLASS_FUNC(SceneUser, getChatParam);
    CLASS_FUNC(SceneUser, getPackage);
    CLASS_FUNC(SceneUser, getZoneID);
    CLASS_FUNC(SceneUser, getEvent);
    CLASS_FUNC(SceneUser, getEquipID);
    CLASS_FUNC(SceneUser, getAchieve);
    CLASS_FUNC(SceneUser, getEquipRefineLv);
    CLASS_FUNC(SceneUser, getEquipCardNum);
    CLASS_FUNC(SceneUser, getKillCount);
    CLASS_FUNC(SceneUser, getBaseObject);
    CLASS_FUNC(SceneUser, getRuneSpecNum);
    CLASS_FUNC(SceneUser, redisPatch);    
    CLASS_FUNC(SceneUser, showMonthCardErrorLog);
    CLASS_FUNC(SceneUser, addMoney);
    CLASS_FUNC(SceneUser, getTip);
    CLASS_FUNC(SceneUser, getPracticeReward);
    CLASS_FUNC(SceneUser, getAppleNum);
    CLASS_FUNC(SceneUser, isBeingPresent);
    CLASS_FUNC(SceneUser, isEquipForceOff);
    CLASS_FUNC(SceneUser, getMapTeamPros);
    CLASS_FUNC(SceneUser, hasMonthCard);
    CLASS_FUNC(SceneUser, isBattleTired);
    CLASS_FUNC(SceneUser, isSkillEnable);
    CLASS_FUNC(SceneUser, getQuestNpc);
    CLASS_FUNC(SceneUser, getCookerLv);
    CLASS_FUNC(SceneUser, getTasterLv);
    CLASS_FUNC(SceneUser, getTower);
    CLASS_FUNC(SceneUser, patch_OperateReward);
    CLASS_FUNC(SceneUser, getBeingGUID);
    CLASS_FUNC(SceneUser, isRide);
    CLASS_FUNC(SceneUser, isPartnerID);
    CLASS_FUNC(SceneUser, getUserBeing);
    CLASS_FUNC(SceneUser, getServant);
    CLASS_FUNC(SceneUser, getSceneShop);
    CLASS_FUNC(SceneUser, hasAccPatchLoad);
    CLASS_FUNC(SceneUser, addAccPatchLoad);
    CLASS_FUNC(SceneUser, getAccIDString);
    CLASS_FUNC(SceneUser, getNormalDrop);
    CLASS_FUNC(SceneUser, getCurElementElfID);
    CLASS_FUNC(SceneUser, getEnsemblePartner);

    // SceneFighter
    CLASS_ADD(SceneFighter);
    CLASS_FUNC(SceneFighter, getSkill);

    // FighterSkill
    CLASS_ADD(FighterSkill);
    CLASS_FUNC(FighterSkill, removeSkill);
    CLASS_FUNC(FighterSkill, resetSkill);
    CLASS_FUNC(FighterSkill, refreshEnableSkill);
    CLASS_FUNC(FighterSkill, isSkillEnable);
    CLASS_FUNC(FighterSkill, checkSkillPointIllegal);
    CLASS_FUNC(FighterSkill, addSkill);
    CLASS_FUNC(FighterSkill, fixEvo4Skill);

    // menu
    CLASS_ADD(Menu);
    CLASS_FUNC(Menu, refreshNewMenu);
    CLASS_FUNC(Menu, processMenuEvent);
    CLASS_FUNC(Menu, isOpen);

    // manual
    CLASS_ADD(Manual);
    CLASS_FUNC(Manual, patch1);
    CLASS_FUNC(Manual, patch2);
    CLASS_FUNC(Manual, patch3);
    CLASS_FUNC(Manual, addSkillPoint);
    CLASS_FUNC(Manual, onEnterMap);
    CLASS_FUNC(Manual, addPoint);
    CLASS_FUNC(Manual, getCollectionStatus);
    CLASS_FUNC(Manual, removeItem);
    CLASS_FUNC(Manual, onItemAdd);
    CLASS_FUNC(Manual, getNumByStatus);
    CLASS_FUNC(Manual, delManualItem);
    CLASS_FUNC(Manual, addManualItem);
    CLASS_FUNC(Manual, getFashionStatus);

    // quest
    CLASS_ADD(Quest);
    CLASS_FUNC(Quest, patch_2016_07_22);
    CLASS_FUNC(Quest, patch_2017_09_11);
    CLASS_FUNC(Quest, isAccept);
    CLASS_FUNC(Quest, isSubmit);
    CLASS_FUNC(Quest, finishQuest);
    CLASS_FUNC(Quest, finishBigQuest);
    CLASS_FUNC(Quest, getPatchStep);
    CLASS_FUNC(Quest, acceptQuest);
    CLASS_FUNC(Quest, processQuest);
    CLASS_FUNC(Quest, getQuestStep);
    CLASS_FUNC(Quest, setQuestStep);

    // userscenedata
    CLASS_ADD(UserSceneData);
    CLASS_FUNC(UserSceneData, getGender);
    CLASS_FUNC(UserSceneData, setFollowerID);
    CLASS_FUNC(UserSceneData, addMapArea);
    CLASS_FUNC(UserSceneData, isNewMap);
    CLASS_FUNC(UserSceneData, addCredit);
    CLASS_FUNC(UserSceneData, getFriendShip);
    CLASS_FUNC(UserSceneData, setFriendShip);
    CLASS_FUNC(UserSceneData, getOnlineMapID);

    // hair
    CLASS_ADD(Hair);
    CLASS_FUNC(Hair, getRealHairColor);
    CLASS_FUNC(Hair, useColorFree);
    CLASS_FUNC(Hair, getRealHair);
    CLASS_FUNC(Hair, useHairFree);

    // equip
    CLASS_ADD(ItemEquip);
    CLASS_FUNC(ItemEquip, setSpecAttr);

    CLASS_ADD(ItemBase);

    // title
    CLASS_ADD(Title);
    CLASS_FUNC(Title, hasTitle);
    CLASS_FUNC(Title, addTitle);

    // portrait
    CLASS_ADD(Portrait);
    CLASS_FUNC(Portrait, usePortrait);
    CLASS_FUNC(Portrait, patch1);

    // freyja
    CLASS_ADD(Freyja);
    CLASS_FUNC(Freyja, isVisible);
    CLASS_FUNC(Freyja, addFreyja);

    CLASS_ADD(SLuaParams);
    CLASS_FUNC(SLuaParams, getParams);
    CLASS_FUNC(SLuaParams, add);
    CLASS_FUNC(SLuaParams, addKeyValue);

    CLASS_ADD(Variable);
    CLASS_FUNC(Variable, getVarValue);
    CLASS_FUNC(Variable, setVarValue);

    // UserScenery
    CLASS_ADD(UserScenery);
    CLASS_FUNC(UserScenery, add);

    CLASS_ADD(SChatParams);
    CLASS_FUNC(SChatParams, putMsg);
    CLASS_FUNC(SChatParams, popMsg);
    CLASS_FUNC(SChatParams, find);
    CLASS_FUNC(SChatParams, resetSize);
    CLASS_FUNC(SChatParams, setInvalidTime);
    CLASS_FUNC(SChatParams, getInvalidTime);
    CLASS_FUNC(SChatParams, setInvalidCount);
    CLASS_FUNC(SChatParams, addInvalidCount);
    CLASS_FUNC(SChatParams, getInvalidCount);

    // Package
    CLASS_ADD(Package);
    CLASS_FUNC(Package, getPackage);
    CLASS_FUNC(Package, checkItemCountByID);
    CLASS_FUNC(Package, getEquipPackage);
    CLASS_FUNC(Package, patch_equip_strengthlv);
    CLASS_FUNC(Package, fixEnchantAttr);
    CLASS_FUNC(Package, clearEnchantBuffid);
    CLASS_FUNC(Package, getPackSlot);
    CLASS_FUNC(Package, getPackSlotUsed);
    CLASS_FUNC(Package, itemModify);
    CLASS_FUNC(Package, itemRemove);
    CLASS_FUNC(Package, addStrengthCost);
    CLASS_FUNC(Package, luaAddItem);

    CLASS_ADD(BasePackage);
    CLASS_FUNC(BasePackage, addItemObj);

    CLASS_ADD(EquipPackage);
    CLASS_FUNC(EquipPackage, getEquipedItemNum);

    // UserEvent
    CLASS_ADD(UserEvent);
    CLASS_FUNC(UserEvent, onMenuOpen);

    CLASS_ADD(BufferStateList);
    CLASS_FUNC(BufferStateList, haveBuff);
    CLASS_FUNC(BufferStateList, getBuffListByName);
    CLASS_FUNC(BufferStateList, getBuffFromID);
    CLASS_FUNC(BufferStateList, getLayerByID);
    CLASS_FUNC(BufferStateList, getEndTimeByID);
    CLASS_FUNC(BufferStateList, getLayerByID);
    CLASS_FUNC(BufferStateList, getLevelByID);

    // skill lua param
    CLASS_ADD(SLuaSkillParam);
    CLASS_FUNC(SLuaSkillParam, addShareDam);
    CLASS_FUNC(SLuaSkillParam, getSkillID);
    CLASS_FUNC(SLuaSkillParam, getTargetsNum);
    CLASS_FUNC(SLuaSkillParam, getArrowID);
    CLASS_FUNC(SLuaSkillParam, haveShareDam);
    CLASS_FUNC(SLuaSkillParam, getBuffLayer);

    // lua array
    CLASS_ADD(SLuaNumberArray)
    CLASS_FUNC(SLuaNumberArray, getDWArraySize);
    CLASS_FUNC(SLuaNumberArray, getDWValueByIndex);

    // Achieve
    CLASS_ADD(Achieve);
    CLASS_FUNC(Achieve, onLevelup);
    CLASS_FUNC(Achieve, onExpressAdd);
    CLASS_FUNC(Achieve, onManual);
    CLASS_FUNC(Achieve, onPassDojo);
    CLASS_FUNC(Achieve, onEquip);
    CLASS_FUNC(Achieve, onQuestSubmit);
    CLASS_FUNC(Achieve, onAchieveFinishLua);
    CLASS_FUNC(Achieve, onPortrait);
    CLASS_FUNC(Achieve, onRune);
    CLASS_FUNC(Achieve, onHair);
    CLASS_FUNC(Achieve, onCat);
    CLASS_FUNC(Achieve, patch_1);
    CLASS_FUNC(Achieve, char_version_0);
    CLASS_FUNC(Achieve, isFinishAchieve);

    // SceneGameTip
    CLASS_ADD(SceneGameTip);
    CLASS_FUNC(SceneGameTip, patch_1);

    // UserQuestNpc
    CLASS_ADD(UserQuestNpc);
    CLASS_FUNC(UserQuestNpc, setQuestStatus);

    // SceneTower
    CLASS_ADD(SceneTower);
    CLASS_FUNC(SceneTower, addEverPassLayer);

    // UserBeing
    CLASS_ADD(UserBeing);
    CLASS_FUNC(UserBeing, addSkill);
    CLASS_FUNC(UserBeing, resetAllSkill);
    CLASS_FUNC(UserBeing, checkHasSkill);
    CLASS_FUNC(UserBeing, checkHasSameSkill);

    // Servant
    CLASS_ADD(Servant);
    CLASS_FUNC(Servant, checkForeverRecommend);

    // SceneShop
    CLASS_ADD(SceneShop);
    CLASS_FUNC(SceneShop, getShopItemCount);
    CLASS_FUNC(SceneShop, setShopItemCount);
  });
}

void SceneServer::v_verifyOk(xNetProcessor *task)
{
  if (!task) return;

  if (strncmp(task->name,"SessionServer", 10)==0)
  {
    SceneManager::getMe().registerAllMapWhenConnect();
  }
  XLOG << "[服务器同步],v_verifyOk" << task->name << getServerName() << XEND;
}

void SceneServer::broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE t, QWORD index, const void *cmd, WORD len, QWORD excludeID, DWORD ip)
{
  DWORD sendlen = sizeof(BroadcastOneLevelIndexGatewayCmd) + len;
  BUFFER_CMD_SIZE(forward, BroadcastOneLevelIndexGatewayCmd, sendlen);
  forward->indexT = t;
  forward->i = index;
  forward->len = len;
  forward->ip = ip;
  forward->exclude = excludeID;
  bcopy(cmd, forward->data, (DWORD)len);
  sendCmdToGate(forward, sendlen);
}

void SceneServer::logErr(SceneUser* user, QWORD uid, const char* name, Cmd::RegErrRet ret, bool delFromMgr)
{
  XLOG << "[登录]," << uid << (name ? name : "") << "登录失败,ret:" << ret << XEND;

  Cmd::RegErrUserCmd message;
  message.set_charid(uid);
  message.set_ret(ret);
  PROTOBUF(message, send, len);
  sendCmdToSession(send, len);
  if (user)
  {
    if (delFromMgr)
      SceneUserManager::getMe().onUserQuit(user, UnregType::Normal);
    else
      SAFE_DELETE(user);
  }
}

void SceneServer::loginErr(QWORD accid, DWORD zoneID, Cmd::RegErrRet ret, xNetProcessor* net)
{
  if (!net) return;
  Cmd::RegErrUserCmd cmd;
  cmd.set_accid(accid);
  cmd.set_zoneid(zoneID);
  cmd.set_ret(ret);
  PROTOBUF(cmd, send, len);
  net->sendCmd(send, len);
}

void SceneServer::test()
{
}

bool SceneServer::sendCmdToMe(QWORD charid, const void* data, unsigned short len)
{
  SceneUser *pUser = SceneUserManager::getMe().getUserByID(charid);
  if (pUser)
  {
    pUser->sendCmdToMe(data, len);
    return true;
  }

  ForwardUserSessionCmd cmd;
  cmd.set_charid(charid);
  cmd.set_data(data, len);
  PROTOBUF(cmd, send, len1);
  sendCmdToSession(send, len1);
  return true;
}

bool SceneServer::forwardCmdToSceneUser(QWORD charid, const void* data, unsigned short len)
{
  SceneUser *pUser = SceneUserManager::getMe().getUserByID(charid);
  if (pUser)
  {
    return pUser->doUserCmd((const Cmd::UserCmd *)data, len);
  }

  ForwardUserSceneSessionCmd cmd;
  cmd.set_charid(charid);
  cmd.set_data(data, len);
  PROTOBUF(cmd, send, len1);
  sendCmdToSession(send, len1);
  return true;
}

// 转发到玩家所在场景
bool SceneServer::forwardCmdToUserScene(QWORD charid, const void* data, unsigned short len)
{
  SceneUser *pUser = SceneUserManager::getMe().getUserByID(charid);
  if (pUser)
  {
    return doSessionCmd((const BYTE *)data, len);
  }

  ForwardUserSceneSvrSessionCmd cmd;
  cmd.set_charid(charid);
  cmd.set_data(data, len);
  PROTOBUF(cmd, send, len1);
  sendCmdToSession(send, len1);
  return true;
}

bool SceneServer::forwardCmdToSessionUser(QWORD charid, const void* data, unsigned short len)
{
  ForwardUserSessionSessionCmd cmd;
  cmd.set_charid(charid);
  cmd.set_data(data, len);
  PROTOBUF(cmd, send, len1);
  sendCmdToSession(send, len1);
  return true;
}

bool SceneServer::forwardCmdToWorldUser(QWORD charid, const void *buf, WORD len)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(charid);
  if (pUser)
  {
    pUser->sendCmdToMe(buf, len);
  }
  else
  {
    GlobalForwardCmdSocialCmd cmd;
    cmd.set_charid(charid);
    cmd.set_data(buf, len);
    cmd.set_len(len);
    PROTOBUF(cmd, send, len2);
    thisServer->sendCmdToSession(send, len2);
  }
  return true;
}

bool SceneServer::sendSysMsgToWorldUser(QWORD qwCharID, DWORD dwMsgID, const MsgParams& params /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/, EMessageActOpt eAct /*= EMESSAGEACT_ADD*/, DWORD dwDelay /*= 0*/)
{
  SysMsg cmd;
  cmd.set_id(dwMsgID);
  cmd.set_type(eType);
  cmd.set_act(eAct);
  cmd.set_delay(dwDelay);
  params.toData(cmd);
  PROTOBUF(cmd, send, len);
  return forwardCmdToWorldUser(qwCharID, send, len);
}

//服务器间消息到weddingserver
bool SceneServer::sendSCmdToWeddingServer(QWORD charid, const std::string& name, const void* buf, unsigned short len)
{
  Cmd::ForwardS2WeddingSCmd cmd;
  cmd.set_charid(charid);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_name(name);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToSession(send2, len2);
}

//玩家消息到weddinser
bool SceneServer::sendUserCmdToWeddingServer(QWORD charid, const std::string& name, const void* buf, WORD len)
{
  Cmd::ForwardC2WeddingSCmd cmd;
  cmd.set_charid(charid);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_name(name);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToSession(send2, len2);
}
