#include "PetConfig.h"
#include "ItemConfig.h"
#include "RewardConfig.h"
#include "MiscConfig.h"
#include "xLuaTable.h"

void SPetCFG::getRandomSkill(TSetDWORD& skills) const
{
  for (auto &m : mapRandomSkill2MaxLv)
  {
    DWORD lv = randBetween(1, m.second);
    DWORD skillid = m.first / ONE_THOUSAND * ONE_THOUSAND + lv;
    skills.insert(skillid);
  }
}

void SPetCFG::getFullSkill(TSetDWORD& skills) const
{
  for (auto &m : mapRandomSkill2MaxLv)
  {
    DWORD lv = m.second;
    DWORD skillid = m.first / ONE_THOUSAND * ONE_THOUSAND + lv;
    skills.insert(skillid);
  }
}

float SPetCFG::getAreaAdventureValue(DWORD dwArea) const
{
  auto m = mapAreaAdventureValue.find(dwArea);
  if (m != mapAreaAdventureValue.end())
    return m->second;
  return 0.0f;
}

bool SPetCFG::isMaxSkill(const TSetDWORD& setIDs) const
{
  TSetDWORD fullskills;
  getFullSkill(fullskills);

  for (auto &s : fullskills)
  {
    if (setIDs.find(s) == setIDs.end())
      return false;
  }
  return true;
}

const SPetActionCFG* SPetCFG::getActionCFG(EPetAction eAction) const
{
  auto m = mapActionCFG.find(eAction);
  if (m != mapActionCFG.end())
    return &m->second;
  return nullptr;
}

bool SPetAdventureCondCFG::checkCond(const EggData& rData) const
{
  if (eCond == EPETADVENTURECOND_RACE)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(rData.id());
    if (pCFG == nullptr)
      return false;
    if (vecParams.empty() == true)
      return false;
    return pCFG->eRaceType == vecParams[0];
  }
  else if (eCond == EPETADVENTURECOND_NATURE)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(rData.id());
    if (pCFG == nullptr)
      return false;
    if (vecParams.empty() == true)
      return false;
    return pCFG->eNatureType == vecParams[0];
  }
  else if (eCond == EPETADVENTURECOND_FRIENDLV)
  {
    if (vecParams.empty() == true)
      return false;
    return rData.friendlv() >= vecParams[0];
  }
  else if (eCond == EPETADVENTURECOND_SKILL)
  {
    if (vecParams.size() != 2)
      return false;
    DWORD dwSkillID = 0;
    for (int i = 0; i < rData.skillids_size(); ++i)
    {
      if (rData.skillids(i) / ONE_THOUSAND == vecParams[0])
      {
        dwSkillID = rData.skillids(i) / ONE_THOUSAND;
        break;
      }
    }
    return dwSkillID >= vecParams[1];
  }
  else if (eCond == EPETADVENTURECOND_PETID)
  {
    if (vecParams.empty() == true)
      return false;
    return rData.id() == vecParams[0];
  }

  return false;
}

// pet config
PetConfig::PetConfig()
{

}

PetConfig::~PetConfig()
{

}

bool PetConfig::loadConfig()
{
  bool bCorrect = true;
  if (loadPetConfig() == false)
    bCorrect = false;
  if (loadCatchConfig() == false)
    bCorrect = false;
  if (loadBaseLvConfig() == false)
    bCorrect = false;
  if (loadFriendLvConfig() == false)
    bCorrect = false;
  if (loadPetBehaviorConfig() == false)
    bCorrect = false;
  if (loadPetAdventureConfig() == false)
    bCorrect = false;
  if (loadPetAdventureCondConfig() == false)
    bCorrect = false;
  if (loadPetWorkConfig() == false)
    bCorrect = false;
  if (loadPetComposeConfig() == false)
    bCorrect = false;
  if (loadPetAvatarConfig() == false)
    bCorrect = false;
  return bCorrect;
}

bool PetConfig::checkConfig()
{
  bool bCorrect = true;

  // pet
  for (auto &m : m_mapPetCFG)
  {
    for (auto &s : m.second.setEquipIDs)
    {
      const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(s);
      if (pCFG == nullptr)
      {
        bCorrect = false;
        XERR << "[宠物-配置] id :" << m.first << "equipid :" << s << "未在 Table_Item.txt 表中找到" << XEND;
        continue;
      }
      if (pCFG->eEquipType == EEQUIPTYPE_MIN)
      {
        bCorrect = false;
        XERR << "[宠物-配置] id :" << m.first << "equipid :" << s << "不是装备" << XEND;
        continue;
      }
    }
  }

  // pet friend level
  for (auto &m : m_mapFriendLvCFG)
  {
    for (auto &level : m.second)
    {
      for (auto &s : level.second.setUnlockEquipIDs)
      {
        if (ItemConfig::getMe().getItemCFG(s) == nullptr)
        {
          bCorrect = false;
          XERR << "[Table_PetFriendLevel] id :" << m.first << "level :" << level.first << "equipid :" << s << "未在 Table_Item.txt 表中找到" << XEND;
          continue;
        }
      }
    }
  }

  // pet adventure
  for (auto &m : m_mapAdventureCFG)
  {
    for (auto &v : m.second.vecItemReq)
    {
      if (ItemConfig::getMe().getItemCFG(v.id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_PetAdventure] id :" << m.first << "Cost :" << v.id() << "未在 Table_Item.txt 表中找到" << XEND;
        continue;
      }
    }
    if (m.second.dwRareReward != 0 && RewardConfig::getMe().getRewardCFG(m.second.dwRareReward) == nullptr)
    {
      bCorrect = false;
      XERR << "[Table_PetAdventure] id :" << m.first << "RareReward :" << m.second.dwRareReward << "未在 Table_Reward.txt 表中找到" << XEND;
      continue;
    }
    for (auto &s : m.second.setRewardIDs)
    {
      if (RewardConfig::getMe().getRewardCFG(s) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_PetAdventure] id :" << m.first << "Reward :" << s << "未在 Table_Reward.txt 表中找到" << XEND;
        continue;
      }
    }
    for (auto &v : m.second.vecMonsterIDs)
    {
      if (NpcConfig::getMe().getNpcCFG(v) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_PetAdventure] id :" << m.first << "monsterid :" << v << "未在 Table_Npc.txt 表中找到" << XEND;
        continue;
      }
    }
  }

  // pet work
  for (auto &m : m_mapPetWorkCFG)
  {
    for (auto &s : m.second.setForbidPets)
    {
      if (getPetCFG(s) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_Pet_WorkSpace] id :" << m.first << "petid :" << s << "未在 Table_Pet.txt 表中找到" << XEND;
        continue;
      }
    }
    for (auto &v : m.second.vecRewardIDs)
    {
      if (RewardConfig::getMe().getRewardCFG(v) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_Pet_WorkSpace] id :" << m.first << "rewardid :" << v << "未在 Table_Reward.txt 表中找到" << XEND;
        continue;
      }
    }
  }

  return bCorrect;
}

bool PetConfig::loadPetConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Pet.txt"))
  {
    XERR << "[Table_Pet], 加载配置Table_Pet.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Pet", table);

  for (auto &m : m_mapPetCFG)
    m.second.blInit = false;

  m_mapItem2PetID.clear();
  m_mapHobbyItem2ComValue.clear();
  for (auto &m : table)
  {
    SPetCFG& stData = m_mapPetCFG[m.first];
    //stData.dwMonsterID = m.second.getTableInt("MonsterID");
    //stData.fFlowDis = m.second.getTableFloat("FollowDistance");
    stData.strName = m.second.getTableString("Name");
    auto comskill = [&](const string& key, xLuaData& d)
    {
      stData.setCommonSkills.insert(d.getInt());
    };
    m.second.getMutableData("CommonSkill").foreach(comskill);

    stData.dwID = m.first;
    stData.dwEggID = m.second.getTableInt("EggID");

    if (m_mapItem2PetID.find(stData.dwEggID) != m_mapItem2PetID.end())
    {
      if (m_mapItem2PetID[stData.dwEggID] != stData.dwID)
      {
        XERR << "[Table_Pet_Capture], 配置表ID:" << m.first << "EggID重复" << stData.dwEggID << XEND;
        bCorrect = false;
        continue;
      }
    }
    m_mapItem2PetID[stData.dwEggID] = stData.dwID;

    DWORD dwType = 0;
    auto dynamicskill = [&](xLuaData& d)
    {
      DWORD skillid = d.getTableInt("1");
      if (skillid == 0)
        return;
      DWORD lv = d.getTableInt("2");
      if (lv == 0)
      {
        XERR << "[Table_Pet], ID:" << m.first << "技能配置错误, 无有效等级, 技能id:" << skillid << XEND;
        bCorrect = false;
        return;
      }
      stData.mapRandomSkill2MaxLv[skillid] = lv;

      if (dwType == 1)
        stData.dwWorkSkillID = skillid / 1000;
    };
    dynamicskill(m.second.getMutableData("Skill_1"));
    dynamicskill(m.second.getMutableData("Skill_2"));
    dynamicskill(m.second.getMutableData("Skill_3"));
    dynamicskill(m.second.getMutableData("Skill_4"));
    dwType = 1, dynamicskill(m.second.getMutableData("Skill_5"));

    auto equipf = [&](const string& key, xLuaData& data)
    {
      stData.setEquipIDs.insert(data.getInt());
    };
    m.second.getMutableData("EquipID").foreach(equipf);

    auto hobbyitem = [&](const string& key, xLuaData& data)
    {
      stData.setHobbyItems.insert(data.getInt());
    };
    m.second.getMutableData("HobbyItem").foreach(hobbyitem);
    DWORD comvalue = m.second.getTableInt("ComPetItemValue");
    for (auto &s : stData.setHobbyItems)
      m_mapHobbyItem2ComValue[s] = comvalue;

    auto areaf = [&](const string& key, xLuaData& data)
    {
      stData.mapAreaAdventureValue[atoi(key.c_str())] = data.getFloat();
    };
    m.second.getMutableData("Area").foreach(areaf);

    stData.bCanEquip = m.second.getTableInt("CanEquip");
    stData.stEquipCondtion.dwFriendLv = m.second.getMutableData("EquipCondition").getTableInt("friendlv");

    stData.blInit = true;
  }
  if (bCorrect)
    XLOG << "[Table_Pet], 加载配置成功" << XEND;
  return bCorrect;
}

bool PetConfig::loadCatchConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Pet_Capture.txt"))
  {
    XERR << "[Table_Pet_Capture], 加载配置Table_Pet_Capture.txt失败" << XEND;
    return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Pet_Capture", table);
  m_mapCatchPetCFG.clear();
  //m_mapCatchNpc2Monster.clear();
  for (auto &m : table)
  {
    if (m_mapCatchPetCFG.find(m.first) != m_mapCatchPetCFG.end())
    {
      XERR << "[Table_Pet_Capture], 配置表ID:" << m.first << "重复" << XEND;
      bCorrect = false;
      continue;
    }
    SCatchPetCFG& stData = m_mapCatchPetCFG[m.first];
    stData.dwMonsterID = m.first;
    stData.dwCatchNpcID = m.second.getTableInt("CatchNpcID");
    stData.dwDefaultCatchValue = m.second.getTableInt("DefaultCatchValue");
    stData.dwItemAddCatchValue = m.second.getTableInt("ItemAddCatchValue");
    stData.dwFailDecCatchValue = m.second.getTableInt("FailDecCatchValue");
    stData.dwPetID = m.second.getTableInt("PetID");
    stData.dwGiftItemID = m.second.getTableInt("GiftItemID");
    /*if (m_mapCatchNpc2Monster.find(stData.dwCatchNpcID) != m_mapCatchNpc2Monster.end())
    {
      if (m_mapCatchNpc2Monster[stData.dwCatchNpcID] != m.first)
      {
        XERR << "[Table_Pet_Capture], 配置表ID:" << m.first << "CatchNpcID重复" << stData.dwCatchNpcID << XEND;
        bCorrect = false;
        continue;
      }
    }
    m_mapCatchNpc2Monster[stData.dwCatchNpcID] = m.first;
    */

    DWORD index = 1;
    auto getset = [&](const string& key, xLuaData& d)
    {
      if (index == 1)
        stData.setAddDialogs.insert(d.getInt());
      else if (index == 2)
        stData.setTryFailDialogs.insert(d.getInt());
      else if (index == 3)
        stData.setValueFailDialogs.insert(d.getInt());
      else if (index == 4)
        stData.setEndDialogs.insert(d.getInt());
      else if (index == 5)
        stData.setIgnoreDialogs.insert(d.getInt());
    };
    index = 1;
    m.second.getMutableData("AddDialog").foreach(getset);
    index = 2;
    m.second.getMutableData("TryFailDialog").foreach(getset);
    index = 3;
    m.second.getMutableData("ValueFailDialog").foreach(getset);
    index = 4;
    m.second.getMutableData("TimeDisFailDialog").foreach(getset);
    index = 5;
    m.second.getMutableData("IgnoreDialog").foreach(getset);
  }
  if (bCorrect)
    XLOG << "[Table_Pet_Capture], 加载配置成功" << XEND;
  return bCorrect;
}

bool PetConfig::loadBaseLvConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_PetBaseLevel.txt"))
  {
    XERR << "[Table_PetBaseLevel], 加载配置Table_PetBaseLevel.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapBaseLvCFG)
    m.second.blInit = false;

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_PetBaseLevel", table);

  QWORD qwLastExp = 0;
  for (auto &m : table)
  {
    SPetBaseLvCFG& rCFG = m_mapBaseLvCFG[m.first];

    rCFG.dwLv = m.first;
    rCFG.qwExp = m.second.getTableQWORD("NeedExp");
    rCFG.qwNewCurExp = m.second.getTableQWORD("NeedExp_2");
    rCFG.qwNewTotExp += rCFG.qwNewCurExp + qwLastExp;
    qwLastExp = rCFG.qwNewTotExp;
    rCFG.blInit = true;
  }

  if (bCorrect)
    XLOG << "[Table_PetBaseLevel], 加载配置成功" << XEND;
  return bCorrect;
}

bool PetConfig::loadFriendLvConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Pet_FriendLevel.txt"))
  {
    XERR << "[Table_Pet_FriendLevel], 加载配置Table_Pet_FriendLevel.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapFriendLvCFG)
  {
    for (auto &lv : m.second)
      lv.second.blInit = false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Pet_FriendLevel", table);

  for (auto &m : table)
  {
    auto pet = m_mapPetCFG.find(m.first);
    if (pet == m_mapPetCFG.end())
    {
      bCorrect = false;
      XERR << "[Table_Pet_FriendLevel] id :" << m.first << "未在 Table_Pet.txt 表中找到" << XEND;
      continue;
    }

    TMapFriendLvCFG& mapCFG = m_mapFriendLvCFG[m.first];

    DWORD dwIndex = 1;
    while (true)
    {
      stringstream sstr;
      sstr << "AmityReward_" << dwIndex;

      if (m.second.has(sstr.str()) == false)
        break;

      SPetFriendLvCFG& rCFG = mapCFG[dwIndex];

      if (m.second.has(sstr.str()) == true)
      {
        rCFG.dwPetID = m.first;
        rCFG.dwLv = dwIndex;

        xLuaData& level = m.second.getMutableData(sstr.str().c_str());
        rCFG.qwExp = level.getTableQWORD("1");
        rCFG.dwRewardID = level.getTableInt("2");
      }

      sstr.str("");
      sstr << "Event_" << dwIndex;

      if (m.second.has(sstr.str()) == true)
      {
        auto eventf = [&](const string& key, xLuaData& data)
        {
          const string& name = data.getTableString("1");

          if (name == "equip")
          {
            DWORD dwIndex = 2;
            while (true)
            {
              stringstream s;
              s << dwIndex++;
              DWORD dwValue = data.getTableInt(s.str());
              if (dwValue == 0)
                break;
              rCFG.setUnlockEquipIDs.insert(dwValue);
            }
          }
          else if (name == "body")
          {
            DWORD dwIndex = 2;
            while (true)
            {
              stringstream s;
              s << dwIndex++;
              DWORD dwValue = data.getTableInt(s.str());
              if (dwValue == 0)
                break;
              rCFG.setUnlockBodyIDs.insert(dwValue);
            }
          }
          else if (name == "adventure")
          {
            rCFG.bCanAdventure = true;
          }
        };
        m.second.getMutableData(sstr.str().c_str()).foreach(eventf);
      }

      rCFG.blInit = true;
      ++dwIndex;
    }
  }

  for (auto &m : m_mapFriendLvCFG)
  {
    TSetDWORD setLastEquipIDs;
    TSetDWORD setLastBodyIDs;
    bool bLastAdventure = false;
    for (auto &level : m.second)
    {
      for (auto &s : setLastEquipIDs)
        level.second.setUnlockEquipIDs.insert(s);
      for (auto &s : setLastBodyIDs)
        level.second.setUnlockBodyIDs.insert(s);

      if (bLastAdventure)
        level.second.bCanAdventure = bLastAdventure;
      else if (level.second.bCanAdventure)
        bLastAdventure = level.second.bCanAdventure;

      setLastBodyIDs = level.second.setUnlockEquipIDs;
      setLastBodyIDs = level.second.setUnlockBodyIDs;
    }
  }

  if (bCorrect)
    XLOG << "[Table_Pet_FriendLevel], 加载配置成功" << XEND;
  return bCorrect;
}

bool PetConfig::loadPetBehaviorConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Pet_Behavior.txt"))
  {
    XERR << "[Table_Pet_Behavior], 加载配置Table_Pet_Behavior.txt失败" << XEND;
    return false;
  }

  auto action_init = [](SPetActionCFG& rCFG, xLuaData& data)
  {
    auto emojif = [&](const string& key, xLuaData& data)
    {
      rCFG.setExpressionIDs.insert(data.getInt());
    };
    data.getMutableData("emoji").foreach(emojif);

    auto dialogf = [&](const string& key, xLuaData& data)
    {
      rCFG.setTalkIDs.insert(data.getInt());
    };
    data.getMutableData("dialog").foreach(dialogf);

    auto actionf = [&](const string& key, xLuaData& data)
    {
      rCFG.setActionIDs.insert(data.getInt());
    };
    data.getMutableData("action").foreach(actionf);

    xLuaData& rate = data.getMutableData("rate");
    rCFG.dwRate = rate.getTableInt("1");
    rCFG.dwCD = rate.getTableInt("2");
  };

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Pet_Behavior", table);
  for (auto &m : table)
  {
    auto pet = m_mapPetCFG.find(m.first);
    if (pet == m_mapPetCFG.end())
    {
      bCorrect = false;
      XERR << "[Table_Pet_Behavior] id :" << m.first << "未在 Table_Pet.txt 表中找到" << XEND;
      continue;
    }

    action_init(pet->second.mapActionCFG[EPETACTION_PET_BORN], m.second.getMutableData("born"));
    action_init(pet->second.mapActionCFG[EPETACTION_PET_IDLE], m.second.getMutableData("SitAround"));
    action_init(pet->second.mapActionCFG[EPETACTION_PET_TOUCH], m.second.getMutableData("NeedTouch"));
    action_init(pet->second.mapActionCFG[EPETACTION_PET_FEED], m.second.getMutableData("NeedBefed"));
    action_init(pet->second.mapActionCFG[EPETACTION_PET_BASELVUP], m.second.getMutableData("LevelUP"));
    action_init(pet->second.mapActionCFG[EPETACTION_PET_FRIENDLVUP], m.second.getMutableData("FriendUp"));
    action_init(pet->second.mapActionCFG[EPETACTION_PET_DEAD], m.second.getMutableData("Dead"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_BASELVUP], m.second.getMutableData("PlayerLevelUP"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_DEAD], m.second.getMutableData("PlayerDead"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_LOGIN], m.second.getMutableData("PlayerLogin"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_HPLESS], m.second.getMutableData("PlayerHP40"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_KILLMVP], m.second.getMutableData("PlayerKillMVP"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_KILLMINI], m.second.getMutableData("PlayerKillmini"));
    // 攻击制定魔物 to do...
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_CHAT], m.second.getMutableData("PlayerChat"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_REFINE_SUCCESS], m.second.getMutableData("RefineSucceed"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_REFINE_FAIL], m.second.getMutableData("RefineSucceedFall"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_PRODUCE_HEAD], m.second.getMutableData("MakeHeadwear"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_ENTERGUILD], m.second.getMutableData("EnterGuild"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_HATCH], m.second.getMutableData("Hatch"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_TOUCH], m.second.getMutableData("BeStroke"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_FEED], m.second.getMutableData("BeFeed"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_NO_FEED], m.second.getMutableData("BeFullFeed"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_RESTORE], m.second.getMutableData("Recycle"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_GETGIFT], m.second.getMutableData("TakeGift"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_HAND], m.second.getMutableData("SpecialMutual"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_EQUIP], m.second.getMutableData("BeQuip"));
    action_init(pet->second.mapActionCFG[EPETACTION_OWNER_SENDGIFT], m.second.getMutableData("GivePresent"));
  }

  if (bCorrect)
    XLOG << "[Table_Pet_Behavior], 加载配置成功" << XEND;
  return bCorrect;
}

bool PetConfig::loadPetAdventureConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Pet_Adventure.txt"))
  {
    XERR << "[Table_Pet_Adventure], 加载配置Table_Pet_Adventure.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapAdventureCFG)
    m.second.blInit = false;
  m_mapAreaAdventureCFG.clear();

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Pet_Adventure", table);
  for (auto &m : table)
  {
    DWORD dwQuality = m.second.getTableInt("Quality");
    if (dwQuality <= EQUALITYTYPE_MIN || dwQuality >= EQUALITYTYPE_MAX || EQualityType_IsValid(dwQuality) == false)
    {
      bCorrect = false;
      XERR << "[Table_Pet_Adventure] id :" << m.first << "quality :" << dwQuality << "不合法" << XEND;
      continue;
    }
    DWORD dwType = m.second.getTableInt("QuestType");
    if (dwType <= EPETADVENTURECOND_MIN || dwType >= EPETADVENTURECOND_MAX)
    {
      bCorrect = false;
      XERR << "[Table_Pet_Adventure] id :" << m.first << "type :" << dwType << "不合法" << XEND;
      continue;
    }

    SPetAdventureCFG& rCFG = m_mapAdventureCFG[m.first];

    rCFG.dwID = m.first;
    rCFG.dwReqLv = m.second.getTableInt("Level");
    rCFG.dwNeedTime = m.second.getTableInt("ConsumeTime");
    rCFG.dwBattleTimeReq = m.second.getTableInt("CostFightTime");
    rCFG.dwAreaReq = m.second.getTableInt("BigArea");
    rCFG.dwMaxPet = m.second.getTableInt("PetNum");
    rCFG.dwRareReward = m.second.getTableInt("RareReward");
    rCFG.dwTime = m.second.getTableInt("Times");
    rCFG.dwUnlockArea = m.second.getTableInt("UnlockArea");
    rCFG.dwPetBaseExp = m.second.getTableInt("Baseexp");

    auto rewardf = [&](const string& key, xLuaData& data)
    {
      rCFG.setRewardIDs.insert(data.getInt());
    };
    m.second.getMutableData("Reward").foreach(rewardf);

    xLuaData& item = m.second.getMutableData("Cost");
    DWORD dwItemID = item.getTableInt("id");
    auto itemnumf = [&](const string& key, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(dwItemID);
      oItem.set_count(data.getInt());
      rCFG.vecItemReq.push_back(oItem);
    };
    item.getMutableData("num").foreach(itemnumf);

    rCFG.eQuality = static_cast<EQualityType>(dwQuality);
    rCFG.eType = static_cast<EPetAdventureType>(dwType);

    auto condf = [&](const string& key, xLuaData& data)
    {
      rCFG.setCondIDs.insert(data.getInt());
    };
    m.second.getMutableData("Condition").foreach(condf);

    auto monsteridf = [&](const string& key, xLuaData& data)
    {
      rCFG.vecMonsterIDs.push_back(data.getInt());
    };
    m.second.getMutableData("MonsterReward").foreach(monsteridf);

    rCFG.blInit = true;
  }

  for (auto &m : m_mapAdventureCFG)
    m_mapAreaAdventureCFG[m.second.dwAreaReq].push_back(m.second);

  if (bCorrect)
    XLOG << "[Table_Pet_Adventure],成功加载配置" << XEND;
  return bCorrect;
}

bool PetConfig::loadPetAdventureCondConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Pet_AdventureCond.txt"))
  {
    XERR << "[Table_Pet_AdventureCond], 加载配置Table_Pet_AdventureCond.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapAdventureCondCFG)
    m.second.blInit = false;
  m_mapAdventureCondCFG.clear();

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Pet_AdventureCond", table);
  for (auto &m : table)
  {
    EPetAdventureCond eCond = getPetAdventureCond(m.second.getTableString("TypeID"));
    if (eCond == EPETADVENTURECOND_MIN)
    {
      bCorrect = false;
      XERR << "[Table_Pet_AdventureCond] id :" << m.first << "Condition key :" << m.second.getTableString("TypeID") << "不合法" << XEND;
      continue;
    }

    TVecQWORD vecParams;
    bool bParamCorrect = true;
    auto paramf = [&](const string& key, xLuaData& data)
    {
      if (eCond == EPETADVENTURECOND_RACE)
      {
        ERaceType eType = NpcConfig::getMe().getRaceType(data.getString());
        if (eType == ERACE_MIN)
        {
          bCorrect = bParamCorrect = false;
          XERR << "[Table_Pet_AdventureCond] id :" << m.first << "TypeID :" << key  << "param :" << data.getString() << "不合法" << XEND;
          return;
        }
        vecParams.push_back(eType);
      }
      else if (eCond == EPETADVENTURECOND_NATURE)
      {
        ENatureType eType = NpcConfig::getMe().getNatureType(data.getString());
        if (eType == ENature_MIN)
        {
          bCorrect = bParamCorrect = false;
          XERR << "[Table_Pet_AdventureCond] id :" << m.first << "TypeID :" << key  << "param :" << data.getString() << "不合法" << XEND;
          return;
        }
        vecParams.push_back(eType);
      }
      else if (eCond == EPETADVENTURECOND_FRIENDLV)
      {
        vecParams.push_back(data.getInt());
      }
      else if (eCond == EPETADVENTURECOND_SKILL)
      {
        vecParams.push_back(data.getTableInt("1"));
        vecParams.push_back(data.getTableInt("2"));
      }
      else if (eCond == EPETADVENTURECOND_PETID)
      {
        vecParams.push_back(data.getInt());
      }
    };
    m.second.getMutableData("Param").foreach(paramf);

    if (!bParamCorrect)
      continue;

    SPetAdventureCondCFG& rCFG = m_mapAdventureCondCFG[m.first];

    rCFG.dwID = m.first;
    rCFG.eCond = eCond;
    rCFG.vecParams.swap(vecParams);

    rCFG.blInit = true;
  }

  if (bCorrect)
    XLOG << "[Table_Pet_AdventureCond],成就加载配置" << XEND;
  return bCorrect;
}

bool PetConfig::loadPetWorkConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Pet_WorkSpace.txt"))
  {
    XERR << "[Table_Pet_WorkSpace], 加载配置Table_Pet_WorkSpace.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapPetWorkCFG)
    m.second.blInit = false;

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Pet_WorkSpace", table);
  for (auto &m : table)
  {
    SPetWorkCFG& rCFG = m_mapPetWorkCFG[m.first];

    rCFG.dwID = m.first;
    rCFG.dwPetLvReq = m.second.getTableInt("Level");
    rCFG.dwFrequency = m.second.getTableInt("Frequency");
    rCFG.dwMaxReward = m.second.getTableInt("Max_reward");
    rCFG.dwActID = m.second.getTableInt("ActID");

    m.second.getMutableData("Work_limit").getIDList(rCFG.setForbidPets);
    //m.second.getMutableData("OpenMenu").getIDList(rCFG.setOpenMenuIDs);
    //m.second.getMutableData("UnlockMenu").getIDList(rCFG.setUnlockMenuIDs);
    m.second.getMutableData("Reward").getIDList(rCFG.vecRewardIDs);

    xLuaData& open = m.second.getMutableData("OpenMenu");
    rCFG.stOpenCond.eCond = getPetWorkCond(open.getTableString("type"));
    if (rCFG.stOpenCond.eCond != EPETWORKCOND_MIN)
      open.getMutableData("params").getIDList(rCFG.stOpenCond.vecParams);

    xLuaData& unlock = m.second.getMutableData("UnlockMenu");
    rCFG.stUnlockCond.eCond = getPetWorkCond(unlock.getTableString("type"));
    if (rCFG.stUnlockCond.eCond != EPETWORKCOND_MIN)
      unlock.getMutableData("params").getIDList(rCFG.stUnlockCond.vecParams);

    rCFG.blInit = true;
  }

  if (bCorrect)
    XLOG << "[Table_Pet_WorkSpace] 成功加载配置" << XEND;
  return bCorrect;
}

bool PetConfig::loadPetComposeConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_PetCompose.txt"))
  {
    XERR << "[Table_PetCompose], 加载配置Table_PetCompose.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapPetComposeCFG)
    m.second.blInit = false;

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_PetCompose", table);
  for (auto &m : table)
  {
    SPetComposeCFG& rCFG = m_mapPetComposeCFG[m.first];
    rCFG.dwPetID = m.first;
    rCFG.vecPetMaterials.clear();
    rCFG.blInit = true;
    auto getmaterial = [&](xLuaData& d)
    {
      if (d.has("id") == false)
        return;
      SPetComposeMaterial s;
      s.dwPetID = d.getTableInt("id");
      s.dwBaseLv = d.getTableInt("baselv");
      s.dwFriendLv = d.getTableInt("friendlv");
      rCFG.vecPetMaterials.push_back(s);
    };
    getmaterial(m.second.getMutableData("MaterialPet1"));
    getmaterial(m.second.getMutableData("MaterialPet2"));
    getmaterial(m.second.getMutableData("MaterialPet3"));

    rCFG.dwZenyCost = m.second.getTableInt("ZenyCost");
  }
  if (bCorrect)
    XLOG << "[Table_PetCompose] 加载成功" << XEND;

  return bCorrect;
}

bool PetConfig::loadPetAvatarConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_PetAvatar.txt"))
  {
    XERR << "[Table_PetAvatar], 加载配置Table_PetAvatar.txt失败" << XEND;
    return false;
  }
  m_mapPetAvatarCFG.clear();

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_PetAvatar", table);

  map<string, EEquipPos> str2Epos;
  str2Epos["Head"] = EEQUIPPOS_HEAD;
  str2Epos["Wing"] = EEQUIPPOS_BACK;
  str2Epos["Tail"] = EEQUIPPOS_TAIL;
  str2Epos["Face"] = EEQUIPPOS_FACE;
  str2Epos["Mouth"] = EEQUIPPOS_MOUTH;

  for (auto &m : table)
  {
    SPetAvatarCFG& rCFG = m_mapPetAvatarCFG[m.first];
    rCFG.dwBodyID = m.first;

    for (auto &s : str2Epos)
    {
      if (m.second.has(s.first))
      {
        auto getitems = [&](const string& k, xLuaData& d)
        {
          DWORD itemid = d.getTableInt("id");
          rCFG.mapPos2ItemIDs[s.second].insert(itemid);
          if (d.getTableInt("IsRandom") == 1)
            rCFG.mapPos2RandomList[s.second].insert(itemid);
        };
        m.second.getMutableData(s.first.c_str()).foreach(getitems);
      }
    }
  }

  if (bCorrect)
    XLOG << "[Table_PetAvatar], 加载配置成功" << XEND;
  return bCorrect;
}

const SCatchPetCFG* PetConfig::getCatchCFGByMonster(DWORD monsterid) const
{
  auto it = m_mapCatchPetCFG.find(monsterid);
  if (it != m_mapCatchPetCFG.end())
    return &it->second;
  return nullptr;
}

/*const SCatchPetCFG* PetConfig::getCatchCFGByNpc(DWORD npcid) const
{
  auto m = m_mapCatchNpc2Monster.find(npcid);
  if (m == m_mapCatchNpc2Monster.end())
    return nullptr;
  return getCatchCFGByMonster(m->second);
}
*/

const SPetCFG* PetConfig::getPetCFG(DWORD petid) const
{
  auto it = m_mapPetCFG.find(petid);
  if (it != m_mapPetCFG.end() && it->second.blInit)
    return &it->second;
  return nullptr;
}

DWORD PetConfig::getPetIDByItem(DWORD itemid) const
{
  auto it = m_mapItem2PetID.find(itemid);
  if (it != m_mapItem2PetID.end())
    return it->second;
  return 0;
}

DWORD PetConfig::getItemIDByPet(DWORD petid) const
{
  for (auto &m : m_mapItem2PetID)
  {
    if (petid == m.second)
      return m.first;
  }
  return 0;
}

const SPetBaseLvCFG* PetConfig::getPetBaseLvCFG(DWORD dwLv) const
{
  auto m = m_mapBaseLvCFG.find(dwLv);
  if (m != m_mapBaseLvCFG.end() && m->second.blInit)
    return &m->second;
  return nullptr;
}

const SPetFriendLvCFG* PetConfig::getFriendLvCFG(DWORD dwPetID, DWORD dwLv) const
{
  auto m = m_mapFriendLvCFG.find(dwPetID);
  if (m == m_mapFriendLvCFG.end())
    return nullptr;

  auto lv = m->second.find(dwLv);
  if (lv != m->second.end() && lv->second.blInit)
    return &lv->second;

  return nullptr;
}

const SPetAdventureCFG* PetConfig::getAdventureCFG(DWORD dwID) const
{
  auto m = m_mapAdventureCFG.find(dwID);
  if (m != m_mapAdventureCFG.end() && m->second.blInit)
    return &m->second;
  return nullptr;
}

const SPetAdventureCondCFG* PetConfig::getAdventureCondCFG(DWORD dwID) const
{
  auto m = m_mapAdventureCondCFG.find(dwID);
  if (m != m_mapAdventureCondCFG.end() && m->second.blInit)
    return &m->second;
  return nullptr;
}

const SPetWorkCFG* PetConfig::getWorkCFG(DWORD dwID) const
{
  auto m = m_mapPetWorkCFG.find(dwID);
  if (m != m_mapPetWorkCFG.end())
    return &m->second;
  return nullptr;
}

bool PetConfig::collectEnableAdventure(TVecPetAdventureCFG& vecCFG, DWORD dwMaxArea)
{
  vecCFG.clear();
  for (auto &m : m_mapAdventureCFG)
  {
    if (m.second.dwAreaReq > dwMaxArea)
      continue;
    vecCFG.push_back(m.second);
  }
  return true;
}

bool PetConfig::isUnlockEquip(DWORD dwPetID, DWORD dwID) const
{
  auto m = m_mapFriendLvCFG.find(dwPetID);
  if (m == m_mapFriendLvCFG.end())
    return false;
  for (auto &lv : m->second)
  {
    if (lv.second.isUnlockEquip(dwID) == true)
      return true;
  }
  return false;
}

bool PetConfig::isUnlockBody(DWORD dwPetID, DWORD dwID) const
{
  auto m = m_mapFriendLvCFG.find(dwPetID);
  if (m == m_mapFriendLvCFG.end())
    return false;
  for (auto &lv : m->second)
  {
    if (lv.second.isUnlockBody(dwID) == true)
      return true;
  }
  return false;
}

EPetAdventureCond PetConfig::getPetAdventureCond(const string& str) const
{
  if (str == "Race")
    return EPETADVENTURECOND_RACE;
  else if (str == "Nature")
    return EPETADVENTURECOND_NATURE;
  else if (str == "Friendly")
    return EPETADVENTURECOND_FRIENDLV;
  else if (str == "Skill")
    return EPETADVENTURECOND_SKILL;
  else if (str == "PetID")
    return EPETADVENTURECOND_PETID;

  return EPETADVENTURECOND_MIN;
}

EPetWorkCond PetConfig::getPetWorkCond(const string& str) const
{
  if (str == "menu")
    return EPETWORKCOND_MENU;

  return EPETWORKCOND_MIN;
}

const SPetComposeCFG* PetConfig::getComposeCFG(DWORD dwID) const
{
  auto it = m_mapPetComposeCFG.find(dwID);
  if (it != m_mapPetComposeCFG.end() && it->second.blInit)
    return &(it->second);
  return nullptr;
}

const SPetAvatarCFG* PetConfig::getAvatarCFGByBody(DWORD dwBody) const
{
  auto it = m_mapPetAvatarCFG.find(dwBody);
  if (it != m_mapPetAvatarCFG.end())
    return &(it->second);
  return nullptr;
}

