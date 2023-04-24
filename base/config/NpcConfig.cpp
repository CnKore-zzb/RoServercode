#include "NpcConfig.h"
#include "xLuaTable.h"
#include "RoleDataConfig.h"
#include "ItemConfig.h"
#include "RewardConfig.h"
#include "MiscConfig.h"

void SNpcFunctionCFG::addNpcFunction(map<DWORD, string>& mapFun)
{
  for (auto&m : mapFun)
  {
    mapRequireFuncs.insert(std::make_pair(m.first, m.second));
  }
}

void SNpcFunctionCFG::delNpcFunction(TSetDWORD& setFun)
{
  for (auto&s : setFun)
  {
    mapRequireFuncs.erase(s);
  }
}

const SNpcSkill* SNpcSkillGroup::getSkill(DWORD dwSkillID) const
{
  auto v = find_if(vecSkill.begin(), vecSkill.end(), [dwSkillID](const SNpcSkill& r) -> bool{
    return r.dwSkillID / ONE_THOUSAND == dwSkillID / ONE_THOUSAND;
  });
  if (v != vecSkill.end())
    return &(*v);
  return nullptr;
}

const SNpcSkillGroup* SNpcCFG::getSkillGroup(DWORD dwIndex) const
{
  auto m = mapSkillGroup.find(dwIndex);
  if (m != mapSkillGroup.end())
    return &m->second;
  return nullptr;
}

bool SNpcCFG::getFeaturesByType(ENpcFeaturesParam eType) const
{
  if(dwFeatures == 0)
    return false;

  DWORD value = static_cast<DWORD>(eType);
  if((dwFeatures&value) != 0)
    return true;

  return false;
}

const TVecDWORD& SNpcFunctionCFG::getFunctionParam(ENpcFunction eType) const
{
  static const TVecDWORD e;
  auto m = mapTypes.find(eType);
  if (m != mapTypes.end())
    return m->second;
  return e;
}

const SReactData* SNpcReaction::getReaction(EReactType eType) const
{
  auto m = mapReactions.find(eType);
  if (m != mapReactions.end())
    return &m->second;
  return nullptr;
}

// npc skill config
const SNpcSkill& SNpcSkillGroup::randOneSkill() const
{
  DWORD dwRand = randBetween(0, dwMaxRate);
  for (auto v = vecSkill.begin(); v != vecSkill.end(); ++v)
  {
    if (dwRand < v->dwRate)
      return *v;
  }

  static const SNpcSkill emptySkill;
  return emptySkill;
}

// config
NpcConfig::NpcConfig()
{

}

NpcConfig::~NpcConfig()
{

}

bool NpcConfig::loadConfig()
{
  bool bResult = true;

  m_mapNpcCFG.clear();
  m_mapCharacterCFG.clear();
  m_mapSkillGroupID2VecMonsterIDs.clear();

  if (loadNpcConfig("Table_Npc") == false)
    bResult = false;
  if (loadNpcConfig("Table_Monster") == false)
    bResult = false;
  if (loadNpcTalkConfig() == false)
    bResult = false;
  if (loadCharacterConfig() == false)
    bResult = false;
  if (loadMonsterSkill() == false)
    bResult = false;
  if (loadMonsterEvoConfig() == false)
    bResult = false;
  if (loadRandomMonsterConfig() == false)
    bResult = false;
  if (loadRaidDeadBossConfig() == false)
    bResult = false;

  return bResult;
}

bool NpcConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto m = m_mapNpcCFG.begin(); m != m_mapNpcCFG.end(); ++m)
  {
    // manual item
    for (auto v = m->second.vecManualItems.begin(); v != m->second.vecManualItems.end(); ++v)
    {
      if (ItemConfig::getMe().getItemCFG(v->id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[NpcConfig] id : " << m->first << " manualitem : " << v->id() << " 未在Table_Item.txt中找到" << XEND;
      }
    }

    // reward
    if (m->second.dwMvpReward != 0 && RewardConfig::getMe().getRewardCFG(m->second.dwMvpReward) == nullptr)
    {
      bCorrect = false;
      XERR << "[NpcConfig] id :" << m->first << "mvpreward :" << m->second.dwMvpReward << "未在Table_Reward.txt表中找到" << XEND;
    }
    for (auto &v : m->second.vecRewardIDs)
    {
      if (RewardConfig::getMe().getRewardCFG(v) == nullptr)
      {
        bCorrect = false;
        XERR << "[NpcConfig] id :" << m->first << "reward :" << v << "未在Table_Reward.txt表中找到" << XEND;
      }
    }
  }

  return bCorrect;
}

const SNpcCFG* NpcConfig::getNpcCFG(DWORD dwID) const
{
  auto m = m_mapNpcCFG.find(dwID);
  if (m != m_mapNpcCFG.end())
    return &m->second;

  return nullptr;
}

bool NpcConfig::loadNpcConfig(const string& file)
{
  string strPath = "Lua/Table/";
  strPath += file;
  strPath += ".txt";
  bool bCorrect = true;
  if (!xLuaTable::getMe().open(strPath.c_str()))
  {
    XERR << "[NpcConfig],加载配置 " << file.c_str() << ".txt 失败" << XEND;
    return false;
  }

  bool bMonster = (file == "Table_Monster");

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable(file.c_str(), table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    if (MiscConfig::getMe().isForbid("Npc", m->first))
      continue;

    // check duplicated
    if (getNpcCFG(m->first) != nullptr)
    {
      XERR << "[NpcConfig] id : " << m->first << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    // check locktype
    DWORD dwLockType = m->second.getTableInt("Condition");
    if (EManualLockMethod_IsValid(dwLockType) == false)
    {
      XERR << "[Npc配置-加载] id :" << m->first << "Condition :" << dwLockType << "不合法" << XEND;
      bCorrect = false;
      continue;
    }

    // create config
    SNpcCFG stCFG;

    stCFG.dwID = m->first;
    stCFG.dwLevel = m->second.getTableInt("Level");
    stCFG.dwBaseAI = m->second.getTableInt("BaseAI");
    stCFG.swAdventureValue = m->second.getTableInt("AdventureValue");
    stCFG.dwGroupID = m->second.getTableInt("GroupID");
    stCFG.dwMvpReward = m->second.getTableInt("Mvp_Reward");
    stCFG.dwHandReward = m->second.getTableInt("Hand_Reward");
    stCFG.dwProtectAtkLv = m->second.getTableInt("PassiveLv");
    stCFG.fScale = m->second.getTableFloat("Scale");
    if (stCFG.fScale == 0.0f)
      stCFG.fScale = 1.0f;
    stCFG.dwCopySkill = m->second.getTableInt("CopySkill");
    stCFG.dwRandPosTime = m->second.getTableInt("RandPosTime");

    if (bMonster)
    {
      DWORD monsterID = m->second.getTableInt("MonsterSkill");
      if (monsterID)
      {
        stCFG.bReplaceSkill = true;
        m_mapSkillGroupID2VecMonsterIDs[monsterID].push_back(m->first);
      }

    }

    stCFG.qwBaseExp = m->second.getTableInt("BaseExp");
    stCFG.qwJobExp = m->second.getTableInt("JobExp");

    stCFG.strName = m->second.getTableString("NameZh");
    stCFG.strMapIcon = m->second.getTableString("MapIcon");

    stCFG.eNpcType = getNpcType(m->second.getTableString("Type"));
    if (stCFG.eNpcType == ENPCTYPE_MIN)
    {
      XERR << "[NpcConfig] id : " << m->first << " Type : " << m->second.getTableString("Type") << " invalid!" << XEND;
      bCorrect = false;
      continue;
    }

    stCFG.eRaceType = getRaceType(m->second.getTableString("Race"));
    stCFG.eZoneType = getZoneType(m->second.getTableString("Zone"));
    stCFG.eNatureType = getNatureType(m->second.getTableString("Nature"));
    stCFG.eProfession = static_cast<EProfession> (m->second.getTableInt("ClassType"));
    if (EProfession_IsValid(stCFG.eProfession) == false)
    {
      XERR << "[NpcConfig], id:" << m->first << "职业类型配置不合法, ClassType =" << stCFG.eProfession << "无效" << XEND;
      bCorrect = false;
    }
    stCFG.eLockType = static_cast<EManualLockMethod>(dwLockType);

    stCFG.dwTowerUnlock = m->second.getTableInt("FloorNum");

    // figure
    stCFG.figure.body = m->second.getTableInt("Body");
    //stCFG.figure.bodyType = m->second.getTableString("Shape");
    stCFG.figure.bodySize = getShape(m->second.getTableString("Shape"));

    stCFG.figure.haircolor = m->second.getTableInt("HeadDefaultColor");
    stCFG.figure.bodycolor = m->second.getTableInt("BodyDefaultColor");
    stCFG.figure.lefthand = m->second.getTableInt("LeftHand");
    stCFG.figure.righthand = m->second.getTableInt("RightHand");
    stCFG.figure.hair = m->second.getTableInt("Hair");
    stCFG.figure.head = m->second.getTableInt("Head");
    stCFG.figure.wing = m->second.getTableInt("Wing");
    stCFG.figure.mount = m->second.getTableInt("Mount");
    stCFG.figure.shadercolor = m->second.getTableInt("ShaderColor");
    stCFG.figure.face = m->second.getTableInt("Face");
    stCFG.figure.tail = m->second.getTableInt("Tail");
    stCFG.figure.eye = m->second.getTableInt("Eye");

    auto rewardf = [&stCFG](std::string key, xLuaData &data)
    {
      stCFG.vecRewardIDs.push_back(data.getInt());
    };
    m->second.getMutableData("Dead_Reward").foreach(rewardf);

    xLuaData& npcfunc = m->second.getMutableData("NpcFunction");
    auto npcfuncf = [&](const string& str, xLuaData& data)
    {
      DWORD dwType = data.getTableInt("type");
      DWORD dwParam = data.getTableInt("param");

      if (dwType != 0)
      {
        TVecDWORD& vecParams = stCFG.stNpcFunc.mapTypes[static_cast<ENpcFunction>(dwType)];
        if (dwType == ENPCFUNCTION_SELL && dwParam == 0)
          vecParams.push_back(ENPCFUNCTIONPARAM_NORMALSELL);
        else
          vecParams.push_back(dwParam);
      }
    };
    npcfunc.foreach(npcfuncf);

    xLuaData& requirefunc = m->second.getMutableData("RequireNpcFunction");
    auto getreqfunc = [&](const string& str, xLuaData& data)
    {
      DWORD dwType = data.getTableInt("type");
      std::stringstream ss;
      data.toJsonString(ss);
      if (dwType != 0)
      {       
        stCFG.stNpcFunc.mapRequireFuncs.insert(std::make_pair(dwType, ss.str()));
        XDBG << "Table_Npc.txt" <<stCFG.dwID << dwType << ss.str() << XEND;
      }
    };
    requirefunc.foreach(getreqfunc);

    // adventure
    xLuaData& adventure = m->second.getMutableData("AdventureReward");
    xLuaData& advbuffid = adventure.getMutableData("buffid");
    auto advbuffidf = [&](const string& str, xLuaData& data)
    {
      stCFG.vecAdvBuffIDs.push_back(data.getInt());
    };
    advbuffid.foreach(advbuffidf);
    xLuaData& manualitem = adventure.getMutableData("item");
    auto manualitemf = [&](const string& str, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      oItem.set_source(ESOURCE_MANUAL);
      stCFG.vecManualItems.push_back(oItem);
    };
    manualitem.foreach(manualitemf);

    const float FLOAT_PERCENT = 100.0f;

    // attr
    UserAttrSvr attr;
    float fParam = 0.0f;

    // 六维属性
    attr.set_type(EATTRTYPE_STR);
    attr.set_value(m->second.getTableInt("Str"));
    stCFG.vecAttrs.push_back(attr);

    attr.set_type(EATTRTYPE_INT);
    attr.set_value(m->second.getTableInt("Int"));
    stCFG.vecAttrs.push_back(attr);

    attr.set_type(EATTRTYPE_VIT);
    attr.set_value(m->second.getTableInt("Vit"));
    stCFG.vecAttrs.push_back(attr);

    attr.set_type(EATTRTYPE_DEX);
    attr.set_value(m->second.getTableInt("Dex"));
    stCFG.vecAttrs.push_back(attr);

    attr.set_type(EATTRTYPE_AGI);
    attr.set_value(m->second.getTableInt("Agi"));
    stCFG.vecAttrs.push_back(attr);

    attr.set_type(EATTRTYPE_LUK);
    attr.set_value(m->second.getTableInt("Luk"));
    stCFG.vecAttrs.push_back(attr);

    // -- atk, matk, def, mdef 计算伤害相关, 不使用show
    fParam = RoleDataConfig::getMe().isPercent("Atk") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableInt("Atk") / fParam);
    attr.set_type(EATTRTYPE_ATK);
    stCFG.vecAttrs.push_back(attr);

    fParam = RoleDataConfig::getMe().isPercent("MAtk") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableInt("MAtk") / fParam);
    attr.set_type(EATTRTYPE_MATK);
    stCFG.vecAttrs.push_back(attr);

    fParam = RoleDataConfig::getMe().isPercent("Def") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableInt("Def") / fParam);
    attr.set_type(EATTRTYPE_DEF);
    stCFG.vecAttrs.push_back(attr);

    fParam = RoleDataConfig::getMe().isPercent("MDef") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableInt("MDef") / fParam);
    attr.set_type(EATTRTYPE_MDEF);
    stCFG.vecAttrs.push_back(attr);
    // -- atk, matk, def, mdef 计算伤害相关, 不使用show

    fParam = RoleDataConfig::getMe().isPercent("Hp") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableInt("Hp") / fParam);
    attr.set_type(EATTRTYPE_MAXHP);
    stCFG.vecAttrs.push_back(attr);

    // hit/flee 允许 负数值
    fParam = RoleDataConfig::getMe().isPercent("Hit") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableFloat("Hit") / fParam);
    attr.set_type(EATTRTYPE_HIT);
    stCFG.vecAttrs.push_back(attr);

    fParam = RoleDataConfig::getMe().isPercent("Flee") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableFloat("Flee") / fParam);
    attr.set_type(EATTRTYPE_FLEE);
    stCFG.vecAttrs.push_back(attr);

    //fParam = RoleDataConfig::getMe().isPercent("AtkSpd") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableFloat("AtkSpd"));
    attr.set_type(EATTRTYPE_ATKSPD);
    stCFG.vecAttrs.push_back(attr);

    fParam = RoleDataConfig::getMe().isPercent("MoveSpd") == true ? FLOAT_PERCENT : 1.0f;
    attr.set_value(m->second.getTableInt("MoveSpd") / fParam);

    attr.set_type(EATTRTYPE_MOVESPD);
    stCFG.vecAttrs.push_back(attr);

    stCFG.fMoveSpd = m->second.getTableInt("MoveSpd") / fParam;

    // normal skill
    xLuaData& normalSkill = m->second.getMutableData("NormalSkill");
    stCFG.dwNormalSkillID = normalSkill.getTableInt("SkillId");

    stCFG.dwSmileEmoji = m->second.getTableInt("Emoji");
    stCFG.dwSmileAction = m->second.getTableInt("Action");

    stCFG.dwBehaviours = m->second.getTableInt("Behaviors");
    stCFG.bHide = m->second.getTableInt("Hide") == 1;
    stCFG.bPredatory = m->second.getTableInt("Predatory") == 1;
    stCFG.bStar = m->second.getTableInt("IsStar") == 1;
    stCFG.dwDefaultGear = m->second.getTableInt("DefaultGear");
    stCFG.bNormalDisplay = m->second.getTableInt("NormalDisplay") == 1;
    stCFG.bNormalHide = m->second.getTableInt("DefaultNotShown") == 1;
    stCFG.dwFeatures = m->second.getTableInt("Features");
    stCFG.dwCarryMoney = m->second.getTableInt("CarryMoney");
    stCFG.dwGender = m->second.getTableInt("Gender");
    if (stCFG.bNormalDisplay && stCFG.bNormalHide)
    {
      bCorrect = false;
      XERR << "[NpcConfig], NormalDisplay 与 DefaultNotShown配置冲突, 不可同时有效, Monster/NpcID:" << m->first << XEND;
    }
    stCFG.bCanBeMonster = m->second.getTableInt("CanBeMonster") == 1;
    stCFG.dwPeriodKillCnt = m->second.getTableInt("KillNum");

    auto getcomrwd = [&](const string& key, xLuaData& d)
    {
      stCFG.setCommonRewards.insert(d.getInt());
    };
    m->second.getMutableData("CommonReward").foreach(getcomrwd);

    auto getfoodrwd = [&](const string& key, xLuaData& d)
    {
      stCFG.setFoodRewards.insert(d.getInt());
    };
    m->second.getMutableData("FoodReward").foreach(getfoodrwd);

    // reaction
    xLuaData& itemdata = m->second.getMutableData("ItemCondition");
    auto getreact = [&](const string& str, xLuaData& data)
    {
      const string& stype = data.getTableString("type");
      EReactType etype = getReactType(stype);
      if (etype == EREACTTYPE_MIN)
        return;
      DWORD type = 1;
      auto getid = [&] (const string& key, xLuaData& dat)
      {
        if (type == 1)
          stCFG.stNpcReaction.mapReactions[etype].vecEquips.push_back(dat.getInt());
        else if (type == 2)
          stCFG.stNpcReaction.mapReactions[etype].vecItems.push_back(dat.getInt());
      };
      xLuaData& equipd = data.getMutableData("equipid");
      equipd.foreach(getid);
      type = 2;
      xLuaData& itemd = data.getMutableData("itemid");
      itemd.foreach(getid);
    };
    itemdata.foreach(getreact);

    xLuaData& attrdata = m->second.getMutableData("AttrCondition");
    auto getreactattr = [&](const string& str, xLuaData& data)
    {
      const string& stype = data.getTableString("type");
      EReactType etype = getReactType(stype);
      if (etype == EREACTTYPE_MIN)
        return;
      DWORD status = data.getTableInt("StateEffect");
      if (status == 0)
        return;
      stCFG.stNpcReaction.mapReactions[etype].dwStatus = status;
    };
    attrdata.foreach(getreactattr);

    xLuaData& transform = m->second.getMutableData("Transform_Skill");
    auto getskill = [&](const string& str, xLuaData& data)
    {
      stCFG.vecTransformSkill.push_back(data.getInt());
    };
    transform.foreach(getskill);

    xLuaData& relevancy = m->second.getMutableData("AdventureRelevancyID");
    auto getrelevancy = [&](const string& str, xLuaData& data)
    {
      stCFG.vecRelevancyIDs.push_back(data.getInt());
    };
    relevancy.foreach(getrelevancy);

    xLuaData& buffd = m->second.getMutableData("Buff");
    auto getbuff = [&](const string& str, xLuaData& data)
    {
      stCFG.vecBuffs.push_back(data.getInt());
    };
    buffd.foreach(getbuff);

    xLuaData& superai = m->second.getMutableData("SuperAI");
    auto superaif = [&](const string& str, xLuaData& data)
    {
      stCFG.setSuperAI.insert(data.getInt());
    };
    superai.foreach(superaif);

    xLuaData& transrec = m->second.getMutableData("TransformRecognition");
    auto transfun = [&](const string& str, xLuaData& data)
    {
      stCFG.setTransRecIDs.insert(data.getInt());
    };
    transrec.foreach(transfun);

    xLuaData& immuneskill = m->second.getMutableData("ImmuneSkill");
    auto getimmune = [&](const string& str, xLuaData& data)
    {
      stCFG.setImmuneSkills.insert(data.getInt());
    };
    immuneskill.foreach(getimmune);

    stCFG.dwChainAtkLmt = m->second.getTableInt("ChainAtkLmt");
    stCFG.dwMoveFrequency = m->second.getTableInt("MoveFrequency");
    // insert to list
    m_mapNpcCFG[stCFG.dwID] = stCFG;

    if (bMonster)
      m_setMonsterIDs.insert(m->first);
  }

  if (bCorrect)
    XLOG << "[NpcConfig] 成功加载" << strPath.c_str() << XEND;;
  return bCorrect;
}

bool NpcConfig::loadNpcTalkConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_NpcTalk.txt"))
  {
    XERR << "[NpcTalkConfig],加载配置Table_NpcTalk.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_NpcTalk", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get npc config
    auto o = m_mapNpcCFG.find(m->first);
    if (o == m_mapNpcCFG.end())
    {
      XERR << "[NpcTalkConfig] id : " << m->first << " 未在 NpcConfig 表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    SNpcCFG& rCFG = o->second;

    xLuaData& norm = m->second.getMutableData("NormalTalk");
    auto getNorm = [&rCFG](const string& key, xLuaData& data)
    {
      rCFG.talk.normalTalkIDs.push_back(data.getInt());
    };
    norm.foreach(getNorm);
    rCFG.talk.talkTime.first = m->second.getData("Time").getTableInt("min");
    rCFG.talk.talkTime.second = m->second.getData("Time").getTableInt("max");

    rCFG.talk.mapType2Talk.clear();
    xLuaData& spec = m->second.getMutableData("SpecialTalk");
    auto getSpec = [&rCFG](const string& key, xLuaData& data)
    {
      TVecDWORD vecIDs;
      auto getIDs = [&vecIDs](std::string key, xLuaData& sondata)
      {
        vecIDs.push_back(sondata.getInt());
      };
      data.foreach(getIDs);
      rCFG.talk.mapType2Talk[key] = vecIDs;
    };
    spec.foreach(getSpec);

    rCFG.talk.mapTalk2Follow.clear();
    xLuaData fol = m->second.getMutableData("Follow");
    auto getfol = [&rCFG](const string& key, xLuaData& data)
    {
      STalkFollow sTalk;
      sTalk.dwTalkID = data.getTableInt("talkid");
      sTalk.dwRange = data.getTableInt("radius");
      auto func = [&sTalk](const string& keyer, xLuaData& dataer)
      {
        if (dataer.has("npc") == false)
          return;
        pair<DWORD, DWORD> pa;
        pa.first = dataer.getTableInt("npc");
        pa.second = dataer.getTableInt("followid");
        sTalk.vecNpcTalkID.push_back(pa);
      };
      data.foreach(func);

      rCFG.talk.mapTalk2Follow[sTalk.dwTalkID] = sTalk;
    };
    fol.foreach(getfol);

    rCFG.talk.odds = m->second.getTableInt("Odds");
  }

  if (bCorrect)
    XLOG << "[NpcTalkConfig] 成功加载Table_NpcTalk.txt" << XEND;
  return true;
}

bool NpcConfig::loadCharacterConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Character.txt"))
  {
    XERR << "[NpcCharacter], 加载配置Table_Character.txt失败" << XEND;
    return false;
  }

  m_mapCharacterCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Character", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    if (getCharacterCFG(m->first) != nullptr)
    {
      XERR << "[NpcCharacter] id : " << m->first << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    SNpcCharacterCFG stCFG;
    stCFG.dwID = m->first;

    auto index = [&stCFG](const string& key, xLuaData &data)
    {
      DWORD id = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (id && Cmd::EAttrType_IsValid(id))
      {
        Cmd::UserAttrSvr uAttr;
        uAttr.set_type((Cmd::EAttrType)id);
        uAttr.set_value(data.getFloat());
        stCFG.nAttrs.push_back(uAttr);
        if (EATTRTYPE_MAXHP == id)
        {
          Cmd::UserAttrSvr sAttr;
          sAttr.set_type(EATTRTYPE_HP);
          sAttr.set_value(data.getFloat());
          stCFG.nAttrs.push_back(sAttr);
        }
      }
    };
    m->second.getMutableData("Effect").foreach(index);

    auto index2 = [&stCFG](const string& key, xLuaData &data)
    {
      DWORD deID = data.getInt();
      stCFG.mutexIDs.push_back(deID);
    };
    m->second.getMutableData("MutexID").foreach(index2);

    m_mapCharacterCFG[stCFG.dwID] = stCFG;
  }

  if (bCorrect)
    XLOG << "[NpcCharacter] 成功加载Table_Character.txt" << XEND;
  return true;
}

bool NpcConfig::loadMonsterSkill()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_MonsterSkill.txt"))
  {
    XERR << "[MonsterSkill], 加载配置Table_MonsterSkill.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_MonsterSkill", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {

    //读MonsterID
    DWORD dwMonsterID = m->second.getTableInt("MonsterID");

    //读技能数据
    xLuaData& state = m->second.getMutableData("Status");
    ENpcState eState = getNpcState(state.getTableString("type"));
    if (eState <= ENPCSTATE_MIN || eState >= ENPCSTATE_MAX)
    {
      XERR << "[MonsterSkill] monsterid : " << dwMonsterID << " state invalid" << XEND;
      bCorrect = false;
      continue;
    }

    SNpcSkill stItem;
    stItem.dwSkillID = m->second.getTableInt("Release");
    stItem.dwRate = m->second.getTableInt("Odds");
    stItem.eState = eState;
    stItem.dwShareCD = m->second.getTableInt("ShareCD");

    auto conditionf = [&, this](xLuaData& condition)
    {
      if (condition.has("type") == false)
        return;

      SNpcSkillCond stCond;

      stCond.eCondition = getSkillCondition(condition.getTableString("type"));
      if (stCond.eCondition <= ENPCSKILLCOND_MIN || stCond.eCondition >= ENPCSKILLCOND_MAX)
      {
        XERR << "[MonsterSkill] monsterid : " << dwMonsterID << " condition : " << condition.getTableString("type") << " error" << XEND;
        return;
      }

      xLuaData& param = condition.getMutableData("param");
      auto paramf = [&](const string& str, xLuaData& data)
      {
        stCond.vecParams.push_back(data.getInt());
      };
      param.foreach(paramf);

      stItem.vecCond.push_back(stCond);
    };

    xLuaData& condition1 = m->second.getMutableData("Condition1");
    conditionf(condition1);
    xLuaData& condition2 = m->second.getMutableData("Condition2");
    conditionf(condition2);
    xLuaData& condition3 = m->second.getMutableData("Condition3");
    conditionf(condition3);
    xLuaData& condition4 = m->second.getMutableData("Condition4");
    conditionf(condition4);

    DWORD groupid = m->second.getTableInt("Priority");

    //查map
    std::vector<DWORD> monsterIDs;
    auto it = m_mapSkillGroupID2VecMonsterIDs.find(dwMonsterID);
    if (it != m_mapSkillGroupID2VecMonsterIDs.end())
      monsterIDs = it->second;
    monsterIDs.push_back(dwMonsterID);

    for (auto &id : monsterIDs)
    {
      //读MonsterCFG
      auto o = m_mapNpcCFG.find(id);

      if (o == m_mapNpcCFG.end())
      {
        XERR << "[MonsterSkill] monsterid : " << id << " 未在 Table_Monster.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }

      if (o->second.bReplaceSkill && id == dwMonsterID)
        continue;

      //SNpcSkillGroup stGroup;
      SNpcSkillGroup& stGroup = o->second.mapSkillGroup[groupid];

      //存技能数据
      stGroup.vecSkill.push_back(stItem);
    }
  }

  for (auto m = m_mapNpcCFG.begin(); m != m_mapNpcCFG.end(); ++m)
  {
    TMapNpcSkillGroup& mapList = m->second.mapSkillGroup;
    for (auto skill = mapList.begin(); skill != mapList.end(); ++skill)
    {
      if (skill->second.vecSkill.size() == 1)
      {
        skill->second.dwMaxRate = 10000;
      }
      else
      {
        for (auto o = skill->second.vecSkill.begin(); o != skill->second.vecSkill.end(); ++o)
        {
          o->dwRate += skill->second.dwMaxRate;
          skill->second.dwMaxRate = o->dwRate;
        }
      }
      //XLOG("[MonsterSkill] id = %u group = %u skillcount = %u maxrate = %u", m->first, skill->first, static_cast<DWORD>(skill->second.vecSkill.size()), skill->second.dwMaxRate);
    }
  }

  if (bCorrect)
    XLOG << "[MonsterSkill] 成功加载Table_MonsterSkill.txt" << XEND;
  return bCorrect;
}

bool NpcConfig::loadMonsterEvoConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_MonsterEvolution.txt"))
  {
    XERR << "[魔物进化-加载配置], 加载配置Table_MonsterEvolution.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_MonsterEvolution", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // find monster config
    auto monster = m_mapNpcCFG.find(m->first);
    if (monster == m_mapNpcCFG.end())
    {
      XERR << "[魔物进化-加载配置] MonsterID : " << m->first << " 未在 Table_Monster.txt 中找到" << XEND;
      bCorrect = false;
      continue;
    }

    DWORD dwEvoID = m->second.getTableInt("EvoID");
    auto item = m_mapNpcCFG.find(dwEvoID);
    if (item == m_mapNpcCFG.end())
    {
      XERR << "[魔物进化-加载配置] EvoID : " << dwEvoID << " 未在 Table_Monster.txt 中找到" << XEND;
      bCorrect = false;
      continue;
    }

    monster->second.dwEvoID = dwEvoID;

    xLuaData& time = m->second.getMutableData("Time");
    monster->second.dwEvoTime = time.getTableInt("1") * 60;
    monster->second.dwEvoRate = time.getTableInt("2");
  }

  if (bCorrect)
    XLOG << "[魔物进化-加载配置] 成功加载 Table_MonsterEvolution.txt 配置" << XEND;
  return bCorrect;
}

bool NpcConfig::loadRandomMonsterConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_RandomMonster.txt"))
  {
    XERR << "[RandomMonster-加载配置], 加载配置Table_RandomMonster.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_RandomMonster", table);

  m_mapRandomMonsterCFG.clear();
  m_mapMapUnique2WeightInfo.clear();
  m_mapMapNpcType2UniqueID.clear();

  for (auto &m : table)
  {
    SRandomMonsterCFG& rCFG = m_mapRandomMonsterCFG[m.first];
    rCFG.dwMonsterID = m.first;
    auto getmapgrp = [&](const string& k, xLuaData& d)
    {
      DWORD mapid = atoi(k.c_str());
      TSetDWORD& grpids = rCFG.mapMapID2GroupIDs[mapid];
      d.getIDList(grpids);
    };
    m.second.getMutableData("MapGroupID").foreach(getmapgrp);

    rCFG.dwWeight = m.second.getTableInt("Weight");
    ENpcType eNpcType = getNpcType(m.second.getTableString("MonsterType"));
    if (eNpcType == ENPCTYPE_MIN)
    {
      XERR << "[Table_RandomMonster], MonsterType配置错误, id:" << m.first << XEND;
      bCorrect = false;
      continue;
    }
    rCFG.eNpcType = eNpcType;
    m.second.getMutableData("BossReward").getIDList(rCFG.setBossReward);

    for (auto &it : rCFG.mapMapID2GroupIDs)
    {
      // it.first : mapid
      map<DWORD, TVecRandMonsterWeight>& mapUid2Weight = m_mapMapUnique2WeightInfo[it.first];
      for (auto &g : it.second)
      {
        TVecRandMonsterWeight& vec = mapUid2Weight[g];

        pair<DWORD, DWORD> paID2Weight;
        paID2Weight.first = rCFG.dwMonsterID;
        paID2Weight.second = (vec.empty() ? rCFG.dwWeight : rCFG.dwWeight + vec.rbegin()->second);
        vec.push_back(paID2Weight);
      }
      map<ENpcType, TSetDWORD>& mapType2Uids = m_mapMapNpcType2UniqueID[it.first];
      TSetDWORD& setids = mapType2Uids[rCFG.eNpcType];
      setids.insert(it.second.begin(), it.second.end());
    }
  }

  if (bCorrect)
    XLOG << "[Table_RandomMonster], 加载配置成功" << XEND;
  return bCorrect;
}

ENpcType NpcConfig::getNpcType(const string& str) const
{
  if (str == "GatherNPC")
    return ENPCTYPE_GATHER;
  if (str == "MINI")
    return ENPCTYPE_MINIBOSS;
  if (str == "MVP")
    return ENPCTYPE_MVP;
  if (str == "Monster")
    return ENPCTYPE_MONSTER;
  if (str == "WeaponPet")
    return ENPCTYPE_WEAPONPET;
  if (str == "SkillNpc")
    return ENPCTYPE_SKILLNPC;
  if (str == "CatchNpc")
    return ENPCTYPE_CATCHNPC;
  if (str == "PetNpc")
    return ENPCTYPE_PETNPC;
  if (str == "FoodNpc")
    return ENPCTYPE_FOOD;
  if (str == "BeingNpc")
    return ENPCTYPE_BEING;
  if (str == "FriendNpc")
    return ENPCTYPE_FRIEND;
  if (str == "ElementElfNpc")
    return ENPCTYPE_ELEMENTELF;
  return ENPCTYPE_NPC;
}

ENpcZoneType NpcConfig::getZoneType(const string& str) const
{
  if (str == "Field")
    return ENPCZONE_FIELD;
  else if (str == "Task")
    return ENPCZONE_TASK;
  else if (str == "EndlessTower")
    return ENPCZONE_ENDLESSTOWER;
  else if (str == "Laboratory")
    return ENPCZONE_LABORATORY;
  else if (str == "Repair")
    return ENPCZONE_SEAL;
  else if (str == "Dojo")
    return ENPCZONE_DOJO;
  else if (str == "Guild")
    return ENPCZONE_GUILD;
  else if (str == "GVGmonster")
    return ENPCZONE_GVGMONSTER;
  else if (str == "PveCard")
    return ENPCZONE_PVECARD;
  else if (str == "MVPBattle")
    return ENPCZONE_MVPBATTLE;
  else if (str == "Dojo_Fight")
    return ENPCZONE_DOJOFIGHT;
  else if (str == "Dead")
    return ENPCZONE_DEAD;
  else if (str == "World")
    return ENPCZONE_WORLD;
  else if (str == "Raid_DeadBoss")
    return ENPCZONE_RAIDDEADBOSS;

  return ENPCZONE_MIN;
}

ENatureType NpcConfig::getNatureType(const string& str) const
{
  if (str == "Wind")
    return ENature_WIND;
  else if (str == "Earth")
    return ENature_EARTH;
  else if (str == "Water")
    return ENature_WATER;
  else if (str == "Fire")
    return ENature_FIRE;
  else if (str == "Neutral")
    return ENature_NEUTRAL;
  else if (str == "Holy")
    return ENature_HOLY;
  else if (str == "Shadow")
    return ENature_SHADOW;
  else if (str == "Ghost")
    return ENature_GHOST;
  else if (str == "Undead")
    return ENature_UNDEAD;
  else if (str == "Poison")
    return ENature_POISON;

  return ENature_MIN;
}

const SNpcCharacterCFG* NpcConfig::getCharacterCFG(DWORD dwID) const
{
  auto m = m_mapCharacterCFG.find(dwID);
  if (m != m_mapCharacterCFG.end())
    return &m->second;

  return nullptr;
}

bool NpcConfig::collectCharacter(TVecDWORD& vecIDs)
{
  DWORD size = m_mapCharacterCFG.size();
  if (!size)
    return false;
  DWORD cha1 = randBetween(1, size);
  vecIDs.push_back(cha1);

  if (randBetween(1, 100) <= 50)
    return true;
  const TVecDWORD& deIDs = m_mapCharacterCFG[cha1].mutexIDs;
  while(true)
  {
    DWORD cha2 = randBetween(1, size);
    bool tFlag = true;
    for (auto m = deIDs.begin(); m != deIDs.end(); ++m)
    {
      if (cha2 == (*m))
        tFlag = false;
    }
    if (tFlag)
    {
      vecIDs.push_back(cha2);
      break;
    }
  }// danger if can't find!!!
  return true;
}

const TVecAttrSvrs* NpcConfig::getCharacAttrs(DWORD id)
{
  auto m = m_mapCharacterCFG.find(id);
  if (m != m_mapCharacterCFG.end())
    return &m_mapCharacterCFG[id].nAttrs;

  return nullptr;
}

ERaceType NpcConfig::getRaceType(const string& str) const
{
  if("Formless" == str)        //无形
    return ERACE_MONSTER;
  if("Undead" == str)         //不死族
    return ERACE_UNDEAD;
  if("Brute" == str)         //动物
    return ERACE_ANIMAL;
  if("Plant" == str)          //植物
    return ERACE_PLANT;
  if("Insect" == str)         //昆虫
    return ERACE_INSECT;
  if("Fish" == str)           //鱼贝
    return ERACE_FISH;
  if("Demon" == str)          //恶魔
    return ERACE_DEMON;
  if("DemiHuman" == str)          //人类
    return ERACE_HUMAN;
  if("Angel" == str)          //天使
    return ERACE_ANGEL;
  if("Dragon" == str)         //龙族
    return ERACE_DRAGON;

  return ERACE_MIN;
}

EProfession NpcConfig::getProfession(const string& str) const
{
  if ("standard" == str)
    return EPROFESSION_WARRIOR;
  if ("knight" == str)
    return EPROFESSION_KNIGHT;
  if ("master" == str)
    return EPROFESSION_MAGICIAN;
  if ("robbers" == str)
    return EPROFESSION_THIEF;
  if ("archer" == str)
    return EPROFESSION_ARCHER;
  if ("priest" == str)
    return EPROFESSION_PRIEST;

  return EPROFESSION_MIN;
}

ENpcSkillCond NpcConfig::getSkillCondition(const string& str) const
{
  if ("Self_HpLess" == str)
    return ENPCSKILLCOND_SELFHPLESS;
  if ("Slave_HpLess" == str)
    return ENPCSKILLCOND_SERVANTHPLESS;
  if ("Slave_Num" == str)
    return ENPCSKILLCOND_SERVANTNUMLESS;
  if ("Attack_Time" == str)
    return ENPCSKILLCOND_ATTACKTIME;
  if ("Self_Range" == str)
    return ENPCSKILLCOND_SELFRANGE;

  return ENPCSKILLCOND_MIN;
}

ENpcState NpcConfig::getNpcState(const string& str) const
{
  if ("Attack" == str)
    return ENPCSTATE_ATTACK;
  if ("Normal" == str)
    return ENPCSTATE_NORMAL;

  return ENPCSTATE_MIN;
}

EReactType NpcConfig::getReactType(const string& str) const
{
  if ("friend" == str)
    return EREACTTYPE_FRIEND;
  else if ("enemy" == str)
    return EREACTTYPE_ENEMY;

  return EREACTTYPE_MIN;
}

DWORD NpcConfig::getShape(const string& str) const
{
  if (str == "S")
    return 1;
  else if (str == "M")
    return 2;
  else if (str == "L")
    return 3;
  return 0;
}

DWORD NpcConfig::getRandMonsterByGroup(DWORD mapid, DWORD grpid) const
{
  auto it = m_mapMapUnique2WeightInfo.find(mapid);
  if (it == m_mapMapUnique2WeightInfo.end())
    return 0;
  auto m = it->second.find(grpid);
  if (m == it->second.end() || m->second.empty())
    return 0;

  DWORD maxweight = m->second.rbegin()->second;
  DWORD rand = randBetween(1, maxweight);
  for (auto &v : m->second)
  {
    if (v.second >= rand)
      return v.first;
  }
  return 0;
}

bool NpcConfig::getRandomMonsterByType(DWORD mapid, ENpcType eType, DWORD num, TVecDWORD& vecIDs) const // 返回uniqueid
{
  auto it = m_mapMapNpcType2UniqueID.find(mapid);
  if (it == m_mapMapNpcType2UniqueID.end())
    return false;
  auto it2 = it->second.find(eType);
  if (it2 == it->second.end() || it2->second.size() < num)
    return false;

  TVecDWORD vecids;
  for (auto &s : it2->second)
    vecids.push_back(s);

  if (num < it2->second.size())
    std::random_shuffle(vecids.begin(), vecids.end());

  for (DWORD i = 0; i < num; ++i)
  {
    vecIDs.push_back(vecids[i]);
  }

  return true;
}

const SRandomMonsterCFG* NpcConfig::getRandomMonsterCFG(DWORD monsterid) const
{
  auto it = m_mapRandomMonsterCFG.find(monsterid);
  if (it == m_mapRandomMonsterCFG.end())
    return nullptr;
  return &(it->second);
}


bool NpcConfig::loadRaidDeadBossConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_RaidDeadBoss.txt"))
  {
    XERR << "[RaidDeadBoss-加载配置], 加载配置Table_RaidDeadBoss.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_RaidDeadBoss", table);

  m_mapRaidDeadBossCFG.clear();
  m_mapType2Lv2DeadBossIDs.clear();

  for (auto &m : table)
  {
    SRaidDeadBossCFG cfg;
    cfg.dwMonsterID = m.first;
    cfg.eRaidBossType = static_cast<ERaidDeadBossType> (m.second.getTableInt("Type"));
    cfg.dwLevel = m.second.getTableInt("Level");
    m.second.getMutableData("Rewards").getIDList(cfg.setRewards);

    m_mapRaidDeadBossCFG[m.first] = cfg;
    m_mapType2Lv2DeadBossIDs[cfg.eRaidBossType][cfg.dwLevel].insert(cfg.dwMonsterID);
  }

  if (bCorrect)
    XLOG << "[RaidDeadBoss-加载配置], 加载配置Table_RaidDeadBoss.txt成功" << XEND;

  return bCorrect;
}

const SRaidDeadBossCFG* NpcConfig::getRaidDeadBossCFG(DWORD monsterid) const
{
  auto it = m_mapRaidDeadBossCFG.find(monsterid);
  return it != m_mapRaidDeadBossCFG.end() ? &(it->second) : nullptr;
}

DWORD NpcConfig::getOneRandomRaidDeadBoss(ERaidDeadBossType eType, DWORD lv) const
{
  auto it = m_mapType2Lv2DeadBossIDs.find(eType);
  if (it == m_mapType2Lv2DeadBossIDs.end())
    return 0;
  auto m = it->second.find(lv);
  if (m == it->second.end())
    return 0;
  auto p = randomStlContainer(m->second);
  if (!p)
    return 0;
  return *p;
}

