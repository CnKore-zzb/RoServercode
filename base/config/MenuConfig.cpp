#include "MenuConfig.h"
#include "ShopConfig.h"
#include "QuestConfig.h"
#include "ManualConfig.h"
#include "MapConfig.h"
#include "NpcConfig.h"
#include "xLuaTable.h"
#include "ItemConfig.h"
#include "AchieveConfig.h"
#include "SkillConfig.h"
#include "MiscConfig.h"

// config
MenuConfig::MenuConfig()
{

}

MenuConfig::~MenuConfig()
{

}

bool MenuConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Menu.txt"))
  {
    XERR << "[MenuConfig], 加载配置Table_Menu.txt失败" << XEND;
    return false;
  }

  m_mapCondMenuCFG.clear();
  m_mapEventMenuCFG.clear();
  m_mapQuest2Menu.clear();
  for (auto& m : m_mapMenuCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Menu", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    if (MiscConfig::getMe().isForbid("Menu", m->second.getTableInt("id")))
    {
      XERR << "[MenuConfig] id :" << m->first << "被屏蔽" << XEND;
      continue;
    }

    SMenuCFG& rCFG = m_mapMenuCFG[m->first];

    rCFG.id = m->first;
    rCFG.acc = m->second.getTableInt("Acc") == 1;

    xLuaData& event = m->second.getMutableData("event");
    rCFG.stEvent.eEvent = getMenuEvent(event.getTableString("type"));

    rCFG.stEvent.vecParams.clear();
    xLuaData& param = event.getMutableData("param");
    param.getIDList(rCFG.stEvent.vecParams);

    SMenuCondition& rCond = rCFG.stCondition;
    rCond.vecParams.clear();

    xLuaData& condition = m->second.getMutableData("Condition");
    if (condition.has("level") == true)
    {
      rCond.eCond = EMENUCOND_BASE_LEVEL;
      rCond.vecParams.push_back(condition.getTableInt("level"));
    }
    else if (condition.has("job") == true)
    {
      rCond.eCond = EMENUCOND_JOB_LEVEL;
      rCond.vecParams.push_back(condition.getTableInt("job"));
    }
    else if (condition.has("guild") == true)
    {
      rCond.eCond = EMENUCOND_INGUILD;
      rCond.vecParams.push_back(condition.getTableInt("guild"));
    }
    else if (condition.has("quest") == true)
    {
      rCond.eCond = EMENUCOND_QUEST;
      condition.getMutableData("quest").getIDList(rCond.vecParams);
    }
    else if (condition.has("skill") == true)
    {
      rCond.eCond = EMENUCOND_SKILL;
      condition.getMutableData("skill").getIDList(rCond.vecParams);
    }
    else if (condition.has("achieve") == true)
    {
      rCond.eCond = EMENUCOND_ACHIEVE;
      condition.getMutableData("achieve").getIDList(rCond.vecParams);
    }
    else if (condition.has("pet") == true)
    {
      rCond.eCond = EMENUCOND_PET;
      xLuaData& pet = condition.getMutableData("pet");
      rCond.vecParams.push_back(pet.getTableInt("1"));
      rCond.vecParams.push_back(pet.getTableInt("2"));
    }
    else if (condition.has("title") == true)
    {
      rCond.eCond = EMENUCOND_TITLE;
      condition.getMutableData("title").getIDList(rCond.vecParams);
    }
    else if (condition.has("manualgroup") == true)
    {
      rCond.eCond = EMENUCOND_MANUALGROUP;
      condition.getMutableData("manualgroup").getIDList(rCond.vecParams);
    }
    else if (condition.has("manualunlock") == true)
    {
      rCond.eCond = EMENUCOND_MANUALUNLOCK;
      xLuaData& unlock = condition.getMutableData("manualunlock");
      auto unlockfunc = [&](const string& key, xLuaData& data)
      {
        DWORD eType = data.getTableInt("1");
        DWORD dwNum = data.getTableInt("2");
        DWORD dwFeature = data.getTableInt("3");
        rCond.vecParams.push_back(eType);
        rCond.vecParams.push_back(dwNum);
        rCond.vecParams.push_back(dwFeature);
      };
      unlock.foreach(unlockfunc);
    }
    else if (condition.has("other") == true)
    {
      rCond.eCond = EMENUCOND_OTHER;
      rCond.vecParams.push_back(condition.getTableInt("other"));
    }
    else if (condition.has("cooklv") == true)
    {
      rCond.eCond = EMENUCOND_COOKLV;
      condition.getMutableData("cooklv").getIDList(rCond.vecParams);
    }
    else if (condition.has("unlock") == true)
    {
      rCond.eCond = EMENUCOND_UNLOCK;

      xLuaData& unlock = condition.getMutableData("unlock");
      rCond.vecParams.push_back(unlock.getTableInt("itemid"));

      xLuaData& pet = unlock.getMutableData("petlv");
      pet.getIDList(rCond.vecParams);
    }
    else if (condition.has("evo") == true)
    {
      rCond.eCond = EMENUCOND_EVO;
      rCond.vecParams.push_back(condition.getTableInt("evo"));
    }
    else if (condition.has("towerlayer") == true)
    {
      rCond.eCond = EMENUCOND_TOWERLAYER;
      rCond.vecParams.push_back(condition.getTableInt("towerlayer"));
    }
    else if (condition.has("manuallevel") == true)
    {
      rCond.eCond = EMENUCOND_MANUALLEVEL;
      rCond.vecParams.push_back(condition.getTableInt("manuallevel"));
    }
    else if (condition.has("pvpcoin") == true)
    {
      rCond.eCond = EMENUCOND_PVPCOIN;
      rCond.vecParams.push_back(condition.getTableInt("pvpcoin"));
    }
    else if (condition.has("wedding") == true)
    {
      rCond.eCond = EMENUCOND_WEDDING;
      rCond.vecParams.push_back(condition.getTableInt("wedding"));
    }
    else if (condition.has("pro") == true)
    {
      rCond.eCond = EMENUCOND_PROFESSION;

      xLuaData& pro = condition.getMutableData("pro");
      rCond.vecParams.push_back(pro.getTableInt("count"));
    }
    else if (condition.has("teampws") == true)
    {
      rCond.eCond = EMENUCOND_TEAMPWS;
      xLuaData& d = condition.getMutableData("teampws");
      rCond.vecParams.push_back(d.getTableInt("1")); // season
      rCond.vecParams.push_back(d.getTableInt("2")); // rank
    }
    else
    {
      bCorrect = false;
      XERR << "[MenuConfig] id :" << m->first << "未知Condition类型" << XEND;
      continue;
    }

    rCFG.blInit = true;
    m_mapCondMenuCFG[rCond.eCond].push_back(rCFG);
    if (rCFG.stEvent.eEvent != EMENUEVENT_MIN)
      m_mapEventMenuCFG[rCFG.stEvent.eEvent].push_back(rCFG);
    if (rCond.eCond == EMENUCOND_QUEST)
    {
      for (auto &v : rCond.vecParams)
        m_mapQuest2Menu[v] = rCFG.id;
    }
  }

  for (auto &m : m_mapCondMenuCFG)
    XDBG << "[MenuConfig] cond :" << m.first << "包含" << m.second.size() << "个配置" << XEND;

  if (bCorrect)
    XLOG << "[MenuConfig] 成功加载配置Table_Menu.txt" << XEND;
  return bCorrect;
}

bool MenuConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto m = m_mapMenuCFG.begin(); m != m_mapMenuCFG.end(); ++m)
  {
    const SMenuCFG& rCFG = m->second;
    const SMenuCondition& rCond = rCFG.stCondition;

    // check quest
    if (rCond.eCond == EMENUCOND_QUEST)
    {
      for (auto o = rCond.vecParams.begin(); o != rCond.vecParams.end(); ++o)
      {
        const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(*o);
        if (pCFG == nullptr)
        {
          XERR << "[Menu配置-检查] id : " << rCFG.id << " questid : " << *o << " 未在 Table_Quest.txt 表中找到" << XEND;
          bCorrect = false;
          continue;
        }
      }
    }
    // check skill
    if (rCond.eCond == EMENUCOND_SKILL)
    {
      for (auto &s : rCond.vecParams)
      {
        if (SkillConfig::getMe().getSkillCFG(s) == nullptr)
        {
          XERR << "[Menu配置-检查] id :" << rCFG.id << "skillid : " << s << " 未在 Table_Skill.txt 表中找到" << XEND;
          bCorrect = false;
          continue;
        }
      }
    }
    // check achieve
    if (rCond.eCond == EMENUCOND_ACHIEVE)
    {
      for (auto &s : rCond.vecParams)
      {
        if (AchieveConfig::getMe().getAchieveCFG(s) == nullptr)
        {
          XERR << "[Menu配置-检查] id :" << rCFG.id << "achieveid : " << s << " 未在 Table_Achievement.txt 表中找到" << XEND;
          bCorrect = false;
          continue;
        }
      }
    }

    // check event
    if (rCFG.stEvent.eEvent == EMENUEVENT_UNLOCKSHOP)
    {
      if (rCFG.stEvent.vecParams.size() != MENUEVENT_UNLOCKSHOP_PARAM_COUNT)
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockshop 参数数量错误" << XEND;
        bCorrect = false;
        continue;
      }
      if (ShopConfig::getMe().isValidType(rCFG.stEvent.vecParams[0]) == false)
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockshop shoptype : " << rCFG.stEvent.vecParams[0] << " error" << XEND;
        bCorrect = false;
        continue;
      }
      TVecShopItem vecItems;
      ShopConfig::getMe().collectShopItem(rCFG.stEvent.vecParams[0], rCFG.stEvent.vecParams[1], vecItems);
      if (vecItems.empty() == true)
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " 未在 Table_Shop.txt 发现相应的商品" << XEND;
        bCorrect = false;
        continue;
      }
      if (rCFG.stEvent.vecParams[2] < EMENUSHOPEVENT_SHOP || rCFG.stEvent.vecParams[2] > EMENUSHOPEVENT_ALL)
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " param3 : " <<  rCFG.stEvent.vecParams[2] << " 不合法的事件";
        bCorrect = false;
        continue;
      }
    }
    else if (rCFG.stEvent.eEvent == EMENUEVENT_UNLOCKMANUAL)
    {
      if (rCFG.stEvent.vecParams.size() < 2)
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockmanual 参数数量错误" << XEND;
        bCorrect = false;
        continue;
      }
      if (rCFG.stEvent.vecParams[0] != EMANUALTYPE_FASHION && rCFG.stEvent.vecParams[0] != EMANUALTYPE_MAP &&
          rCFG.stEvent.vecParams[0] != EMANUALTYPE_MONSTER && rCFG.stEvent.vecParams[0] != EMANUALTYPE_COLLECTION)
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockmanual param1 : " << rCFG.stEvent.vecParams[0] << " 不合法的冒险类型" << XEND;
        bCorrect = false;
        continue;
      }
      if (rCFG.stEvent.vecParams[0] == EMANUALTYPE_MAP)
      {
        for (size_t i = 1; i < rCFG.stEvent.vecParams.size(); ++i)
        {
          const SMapCFG* pBase = MapConfig::getMe().getMapCFG(rCFG.stEvent.vecParams[i]);
          if (pBase == nullptr)
          {
            if (MapConfig::getMe().isUnopenMap(rCFG.stEvent.vecParams[i]) == false)
            {
              XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockmanual manualtype : " << rCFG.stEvent.vecParams[0] << " mapid : " << rCFG.stEvent.vecParams[i] << " 未在 Table_Map.txt 表中找到" << XEND;
              bCorrect = false;
            }
            else
            {
              XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockmanual manualtype : " << rCFG.stEvent.vecParams[0] << " mapid : " << rCFG.stEvent.vecParams[i] << " 未开启 请相关策划注意" << XEND;
            }
            continue;
          }
        }
      }
      else if (rCFG.stEvent.vecParams[0] == EMANUALTYPE_MONSTER || rCFG.stEvent.vecParams[0] == EMANUALTYPE_PET)
      {
        for (size_t i = 1; i < rCFG.stEvent.vecParams.size(); ++i)
        {
          const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(rCFG.stEvent.vecParams[i]);
          if (pCFG == nullptr)
          {
            XERR << "[Menu配置-检查] id :" << rCFG.id << "type = unlockmanual manualtype :" << rCFG.stEvent.vecParams[0] << "monsterid :" << rCFG.stEvent.vecParams[i] << "未在 Table_Monster.txt 表中找到" << XEND;
            bCorrect = false;
            break;
          }
        }
      }
      else if(rCFG.stEvent.vecParams[0] == EMANUALTYPE_CARD)
      {
        for (size_t i = 1; i < rCFG.stEvent.vecParams.size(); ++i)
        {
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rCFG.stEvent.vecParams[i]);
          if (pCFG == nullptr)
          {
            XERR << "[Menu配置-检查] id :" << rCFG.id << "type = unlockmanual manualtype :" << rCFG.stEvent.vecParams[0] << "itemid :" << rCFG.stEvent.vecParams[i] << "未在 Table_Item.txt 表中找到" << XEND;
            bCorrect = false;
            break;
          }
        }
      }
    }
    else if (rCFG.stEvent.eEvent == EMENUEVENT_UNLOCKACTION)
    {
      if (rCFG.stEvent.vecParams.empty())
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockaction 参数数量错误" << XEND;
        bCorrect = false;
        continue;
      }
      for (size_t i = 0; i < rCFG.stEvent.vecParams.size(); ++i)
      {
        const SActionAnimBase* pBase = TableManager::getMe().getActionAnimCFG(rCFG.stEvent.vecParams[i]);
        if (pBase == nullptr)
        {
          XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockaction param : " << rCFG.stEvent.vecParams[i] << " invalid!" << XEND;
          bCorrect = false;
          break;
        }
      }
    }
    else if (rCFG.stEvent.eEvent == EMENUEVENT_UNLOCKEXPRESSION)
    {
      if (rCFG.stEvent.vecParams.empty())
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockexpression 参数数量错误" << XEND;
        bCorrect = false;
        continue;
      }
      for (size_t i = 0; i < rCFG.stEvent.vecParams.size(); ++i)
      {
        const SExpression* pBase = TableManager::getMe().getExpressionCFG(rCFG.stEvent.vecParams[i]);
        if (pBase == nullptr)
        {
          XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockexpression param = " << rCFG.stEvent.vecParams[i] << " invalid!" << XEND;
          bCorrect = false;
          break;
        }
      }
    }
    else if (rCFG.stEvent.eEvent == EMENUEVENT_UNLOCKHAIR || rCFG.stEvent.eEvent == EMENUEVENT_UNLOCKEYE)
    {
      if (rCFG.stEvent.vecParams.empty())
      {
        XERR << "[Menu配置-检查] id : " << rCFG.id << " type = unlockhair 参数数量错误" << XEND;
        bCorrect = false;
        continue;
      }
      for (auto &hair : rCFG.stEvent.vecParams)
      {
        const SItemCFG* pCFG = ItemConfig::getMe().getHairCFG(hair);
        if (pCFG == nullptr)
        {
          XERR << "[Menu配置-检查] id : " << rCFG.id << " type" << rCFG.stEvent.eEvent << "param = " << hair << " invalid" << XEND;
          bCorrect = false;
        }
      }
    }
  }

  return bCorrect;
}

const SMenuCFG* MenuConfig::getMenuCFG(DWORD id)
{
  auto m = m_mapMenuCFG.find(id);
  if (m != m_mapMenuCFG.end() && m->second.blInit)
    return &m->second;
  return nullptr;
}

const TVecMenuCFG* MenuConfig::getCondMenuList(EMenuCond eCond) const
{
  auto m = m_mapCondMenuCFG.find(eCond);
  if (m != m_mapCondMenuCFG.end())
    return &m->second;
  return nullptr;
}

const TVecMenuCFG* MenuConfig::getEventMenuList(EMenuEvent eEvent) const
{
  auto m = m_mapEventMenuCFG.find(eEvent);
  if (m != m_mapEventMenuCFG.end())
    return &m->second;
  return nullptr;
}

DWORD MenuConfig::getMenuIDByQuestID(DWORD dwQuestID) const
{
  auto m = m_mapQuest2Menu.find(dwQuestID);
  return m != m_mapQuest2Menu.end() ? m->second : 0;
}

EMenuEvent MenuConfig::getMenuEvent(const string& str) const
{
  if (str == "skillgrid")
    return EMENUEVENT_SKILLGRID;
  else if (str == "unlockshop")
    return EMENUEVENT_UNLOCKSHOP;
  else if (str == "unlockmanual")
    return EMENUEVENT_UNLOCKMANUAL;
  else if (str == "scenery")
    return EMENUEVENT_SCENERY;
  else if (str == "AutoSkill")
    return EMENUEVENT_AUTOSKILL;
  else if (str == "unlockaction")
    return EMENUEVENT_UNLOCKACTION;
  else if (str == "unlockexpression")
    return EMENUEVENT_UNLOCKEXPRESSION;
  else if (str == "unlockhair")
    return EMENUEVENT_UNLOCKHAIR;
  else if (str == "unlockeye")
    return EMENUEVENT_UNLOCKEYE;
  else if (str == "ExtendSkill")
    return EMENUEVENT_EXTENDSKILL;
  else if (str == "SeeNpc")
    return EMENUEVENT_SEENPC;
  else if (str == "HideNpc")
    return EMENUEVENT_HIDENPC;
  else if (str == "unlock_cat_num")
    return EMENUEVENT_UNLOCKCATNUM;
  else if (str == "unlock_cat_id")
    return EMENUEVENT_UNLOCKCATID;
  else if (str == "add_shop_cnt")
    return EMENUEVENT_ADDSHOPCNT;
  else if (str == "addjoblv")
    return EMENUEVENT_ADDMAXJOBLEVEL;
  else if (str == "additem")
    return EMENUEVENT_ADDITEM;

  return EMENUEVENT_MIN;
}

