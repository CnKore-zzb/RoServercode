#include "AstrolabeConfig.h"
#include "RoleDataConfig.h"
#include "ComposeConfig.h"
#include "xLuaTable.h"

const SAstrolabeAttrCFG* SAstrolabeStarCFG::getAttrByProfessionType(DWORD pt) const
{
  auto it = mapAttr.find(pt);
  if (it != mapAttr.end())
    return &it->second;
  return nullptr;
}

AstrolabeConfig::AstrolabeConfig()
{
}

AstrolabeConfig::~AstrolabeConfig()
{
}

bool AstrolabeConfig::loadConfig()
{
  bool bCorrect = true;

  if (loadAstrolabeConfig() == false)
    bCorrect = false;
  if (loadRuneConfig() == false)
    bCorrect = false;
  if (loadRuneSpecConfig() == false)
    bCorrect = false;

  return bCorrect;
}

bool AstrolabeConfig::checkConfig()
{
  bool bCorrect = true;
  // todo
  // for (auto &it : m_mapAstrolabeCFG) {
  //   for (auto &itStar : it.second.mapStar) {
  //     // XERR << "[星盘配置-效果] id:" << it.second.dwId << "starid:" << itStar.second.dwId << XEND;
  //   }
  // }

  return bCorrect;
}

bool AstrolabeConfig::loadAstrolabeConfig()
{
  if (!xLuaTable::getMe().open("client-export/Astrolabe.lua")) {
    XERR << "[星盘配置-位置] 加载配置Astrolabe.lua失败" << XEND;
    return false;
  }

  m_mapAstrolabeCFG.clear();

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Astrolabe", table);
  for (auto m = table.begin(); m != table.end(); ++m) {
    DWORD id = m->second.getTableInt("id");
    if (m_mapAstrolabeCFG.find(id) != m_mapAstrolabeCFG.end()) {
      XERR << "[星盘配置-位置] id:" << id << "星盘重复定义" << XEND;
      bCorrect = false;
      continue;
    }

    SAstrolabeCFG& rCfg = m_mapAstrolabeCFG[id];
    rCfg.dwId = id;
    xLuaData& stars = m->second.getMutableData("stars");
    auto func = [&](const string& key, xLuaData& data) {
      DWORD starid = atoi(key.c_str());
      if (rCfg.mapStar.find(starid) != rCfg.mapStar.end()) {
        XERR << "[星盘配置-位置] id:" << id << "starid:" << starid << "星位重复定义" << XEND;
        bCorrect = false;
        return;
      }
      SAstrolabeStarCFG& rStarCfg = rCfg.mapStar[starid];
      rStarCfg.dwId = starid;

      xLuaData& inner = data.getMutableData("1");
      for (auto v = inner.m_table.begin(); v != inner.m_table.end(); ++v) {
        pair<DWORD, DWORD> conn;
        conn.first = id;
        conn.second = v->second.getInt();
        rStarCfg.vecConn.push_back(conn);
      }

      xLuaData& outer = data.getMutableData("2");
      for (auto v = outer.m_table.begin(); v != outer.m_table.end(); ++v) {
        pair<DWORD, DWORD> conn;
        conn.first = v->second.getInt() / 10000;
        conn.second = v->second.getInt() % 10000;
        rStarCfg.vecConn.push_back(conn);
      }
    };
    stars.foreach(func);

    // unlock
    xLuaData& unlock = m->second.getMutableData("unlock");
    if (unlock.has("lv")) {
      rCfg.dwLevel = unlock.getTableInt("lv");
    } else {
      XERR << "[星盘配置-解锁条件] id:" << id << "未定义解锁等级" << XEND;
    }

    if (unlock.has("evo")) {
      rCfg.dwEvo = unlock.getTableInt("evo");
    } else {
      XERR << "[星盘配置-解锁条件] id:" << id << "未定义职业限制" << XEND;
    }

    if (unlock.has("menuid")) {
      rCfg.dwMenuID = unlock.getTableInt("menuid");
    }
  }

  if (bCorrect)
    XLOG << "[星盘配置-位置] 成功加载配置Astrolabe.lua" << XEND;
  return bCorrect;
}

bool AstrolabeConfig::loadRuneConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Rune.txt")) {
    XERR << "[星盘配置-效果] 加载配置Table_Rune.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Rune", table);
  for (auto m = table.begin(); m != table.end(); ++m) {
    DWORD id = m->first / 10000;
    DWORD starid = m->first % 10000;

    auto it = m_mapAstrolabeCFG.find(id);
    if (it == m_mapAstrolabeCFG.end()) {
      XERR << "[星盘配置-效果] id:" << m->first << "未找到对应星盘" << XEND;
      bCorrect = false;
      continue;
    }
    auto itStar = it->second.mapStar.find(starid);
    if (itStar == it->second.mapStar.end()) {
      XERR << "[星盘配置-效果] id:" << m->first << "未找到对应星位" << XEND;
      bCorrect = false;
      continue;
    }

    if (m->second.has("Cost")) {
      auto func = [&](const string& key, xLuaData& data) {
        ItemInfo item;
        item.set_id(data.getTableInt("1"));
        item.set_count(data.getTableInt("2"));
        itStar->second.vecCost.push_back(item);
      };
      m->second.getMutableData("Cost").foreach(func);
    }

    if (m->second.has("ResetCost")) {
      auto func = [&](const string& key, xLuaData& data) {
        ItemInfo item;
        item.set_id(data.getTableInt("1"));
        item.set_count(data.getTableInt("2"));
        itStar->second.vecResetCost.push_back(item);
      };
      m->second.getMutableData("ResetCost").foreach(func);
    }
    itStar->second.dwPetAdventureScore = m->second.getTableInt("StarPoint");

    // 加载效果
    // format: {[1]=4,[2]="魔法防御",[3]=1,[4]={MDef=3}}
    auto funcattr = [&](xLuaData& data) {
      if (!data.has("1")) {
        XERR << "[星盘配置-效果] id:" << m->first << "职业系未配置" << XEND;
        bCorrect = false;
        return;
      }
      DWORD proftype = data.getTableInt("1");

      if (itStar->second.mapAttr.find(proftype) != itStar->second.mapAttr.end()) {
        XERR << "[星盘配置-效果] id:" << m->first << "职业系重复" << XEND;
        bCorrect = false;
        return;
      }

      SAstrolabeAttrCFG& rAttr = itStar->second.mapAttr[proftype];

      // 属性
      if (data.has("4")) {
        auto collect = [&](const string& key, xLuaData& data) {
          DWORD attrtype = RoleDataConfig::getMe().getIDByName(key.c_str());
          if (attrtype <= EATTRTYPE_MIN || attrtype >= EATTRTYPE_MAX || !EAttrType_IsValid(attrtype)) {
            XERR << "[星盘配置-效果] id:" << m->first << "attr:" << key.c_str() << "未在 Table_RoleData.txt 表中找到" << XEND;
            bCorrect = false;
            return;
          }

          UserAttrSvr attr;
          attr.set_type(static_cast<EAttrType>(attrtype));
          attr.set_value(data.getFloat());
          rAttr.vecAttr.push_back(attr);
        };
        data.getMutableData("4").foreach(collect);
      }

      // 特殊效果
      if (data.has("5")) {
        /*auto collect = [&](const string& key, xLuaData& data) {
          rAttr.vecEffect.push_back(data.getInt());
        };
        data.getMutableData("5").foreach(collect);
        */
        DWORD specid = data.getTableInt("5");
        if (specid)
          rAttr.vecEffect.push_back(specid);
      }
    };
    if (m->second.has("Swordman_Attr")) {
      funcattr(m->second.getMutableData("Swordman_Attr"));
    }
    if (m->second.has("Magician_Attr")) {
      funcattr(m->second.getMutableData("Magician_Attr"));
    }
    if (m->second.has("Thief_Attr")) {
      funcattr(m->second.getMutableData("Thief_Attr"));
    }
    if (m->second.has("Archer_Attr")) {
      funcattr(m->second.getMutableData("Archer_Attr"));
    }
    if (m->second.has("Acolyte_Attr")) {
      funcattr(m->second.getMutableData("Acolyte_Attr"));
    }
    if (m->second.has("Merchant_Attr")) {
      funcattr(m->second.getMutableData("Merchant_Attr"));
    }
    if (m->second.has("Monk_Attr")) {
      funcattr(m->second.getMutableData("Monk_Attr"));
    }
    if (m->second.has("Crusader_Attr")) {
      funcattr(m->second.getMutableData("Crusader_Attr"));
    }
    if (m->second.has("Alchemist_Attr")) {
      funcattr(m->second.getMutableData("Alchemist_Attr"));
    }
    if (m->second.has("Rogue_Attr")) {
      funcattr(m->second.getMutableData("Rogue_Attr"));
    }
    if (m->second.has("Bard_Attr")) {
      funcattr(m->second.getMutableData("Bard_Attr"));
    }
    if (m->second.has("Dancer_Attr")) {
      funcattr(m->second.getMutableData("Dancer_Attr"));
    }
    if (m->second.has("Sage_Attr")) {
      funcattr(m->second.getMutableData("Sage_Attr"));
    }
  }

  if (bCorrect)
    XLOG << "[星盘配置-效果] 成功加载配置Table_Rune.txt" << XEND;
  return bCorrect;
}

const SAstrolabeStarCFG* AstrolabeConfig::getAstrolabeStar(DWORD id, DWORD starid) const
{
  auto it = m_mapAstrolabeCFG.find(id);
  if (it == m_mapAstrolabeCFG.end())
    return nullptr;
  auto itStar = it->second.mapStar.find(starid);
  if (itStar == it->second.mapStar.end())
    return nullptr;
  return &itStar->second;
}

const SAstrolabeCFG* AstrolabeConfig::getAstrolabe(DWORD id) const
{
  auto it = m_mapAstrolabeCFG.find(id);
  if (it == m_mapAstrolabeCFG.end())
    return nullptr;
  return &it->second;
}

bool AstrolabeConfig::loadRuneSpecConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_RuneSpecial.txt")) {
    XERR << "[星盘配置-效果] 加载配置Table_RuneSpecial.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_RuneSpecial", table);
  m_mapRuneSpecCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SRuneSpecCFG stCFG;

    stCFG.dwID = m->first;
    DWORD dtype = m->second.getTableInt("Type");
    if (dtype <= ERUNESPECTYPE_MIN || dtype >= ERUNESPECTYPE_MAX)
    {
      XERR << "[Table_RuneSpecial], id:" << m->first << "Type =" << dtype << "不合法" << XEND;
      bCorrect = false;
      continue;
    }
    stCFG.eType = static_cast<ERuneSpecType> (dtype);

    xLuaData skilld = m->second.getMutableData("SkillID");
    auto getskillid = [&](const string& key, xLuaData& d)
    {
      stCFG.setSkillIDs.insert(d.getInt());
    };
    skilld.foreach(getskillid);
    xLuaData& specdata = m->second.getMutableData("SpecialEffect");
    if (specdata.has("buffid"))
    {
      auto getbuff = [&](const string& key, xLuaData& d)
      {
        stCFG.setBuffIDs.insert(d.getInt());
      };
      specdata.getMutableData("buffid").foreach(getbuff);
    }

    stCFG.strName = m->second.getTableString("RuneName");

    if (m->second.has("BeingEffect"))
    {
      stCFG.dwBeingSkillPoint = m->second.getMutableData("BeingEffect").getTableInt("skillpoint");

      auto getbuff = [&](const string& k, xLuaData& d)
      {
        DWORD beingid = atoi(k.c_str());
        auto f = [&](const string& k1, xLuaData& d1)
        {
          stCFG.mapBeingBuff[beingid].insert(d1.getInt());
        };
        d.foreach(f);
      };
      m->second.getMutableData("BeingEffect").getMutableData("buffid").foreach(getbuff);

      stCFG.dwBeingSkillID = m->second.getMutableData("BeingEffect").getTableInt("skillid");
      stCFG.intRange = m->second.getMutableData("BeingEffect").getTableInt("range");

      auto itemcostf = [&](const string& k, xLuaData& d)
      {
        stCFG.mapBeingItemCosts[atoi(k.c_str())] = d.getInt();
      };
      m->second.getMutableData("BeingEffect").getMutableData("item_cost").foreach(itemcostf);
    }

    m_mapRuneSpecCFG[m->first] = stCFG;
  }

  if (bCorrect)
    XLOG << "[Table_RuneSpecial-加载], Table_RuneSpecial.txt 加载成功" << XEND;
  return bCorrect;
}

const SRuneSpecCFG* AstrolabeConfig::getRuneSpecCFG(DWORD id) const
{
  auto it = m_mapRuneSpecCFG.find(id);
  if (it == m_mapRuneSpecCFG.end())
    return nullptr;

  return &it->second;
}

