#pragma once

#include <algorithm>
#include "xlib/xEntry.h"
#include "ProtoCommon.pb.h"
//#include "Define.h"
#include "xLuaTable.h"
#include "config/RoleDataConfig.h"
#include "xPos.h"
#include <sstream>

using namespace Cmd;
using std::string;
using std::vector;
using std::pair;
using std::map;
using std::set;
using std::ostringstream;

struct xTableEntry : public xEntry
{
  public:
    const xLuaData& getData() const
    {
      return m_oData;
    }
    bool has(const char *key) const
    {
      return m_oData.has(key);
    }
    int getTableInt(std::string key) const
    {
      return m_oData.getTableInt(key);
    }
    float getTableFloat(std::string key) const
    {
      return m_oData.getTableFloat(key);
    }
    const char* getTableString(std::string key) const
    {
      return m_oData.getTableString(key);
    }
    const xLuaData& getData(const char *key) const
    {
      return m_oData.getData(key);
    }
    virtual bool init(DWORD key, xLuaData &value, const string& table)
    {
      set_id(key);
      if (!id) return true;
      set_name(value.getTableString("NameZh"));

      m_oData.clear();
      m_oData = value;

      return true;
    }

  private:
    xLuaData m_oData;
};

const float FLOAT_PERCENT = 100.0f;
/*struct SCommand
{
  typedef pair<string, DWORD> TPairCmdParams;
  typedef vector<TPairCmdParams> TVecCmdParams;

  string cmd;
  TVecCmdParams vecParams;

  DWORD getParam(const string& name)
  {
    for (auto v = vecParams.begin(); v != vecParams.end(); ++v)
    {
      if (v->first == name)
        return v->second;
    }

    return 0;
  }
};
typedef vector<SCommand> TVecCommands;

struct SBaseEffect
{
  TVecAttrSvrs vecAttrs;
  TVecCommands vecCommands;

  const SCommand* getCommand(const string& name)
  {
    for (auto v = vecCommands.begin(); v != vecCommands.end(); ++v)
    {
      if (v->cmd == name)
        return &(*v);
    }

    return NULL;
  }

  const string getGMCommand(const string& name)
  {
    const static string emptyStr = "";
    const SCommand* pCommand = getCommand(name);
    if (pCommand == NULL)
      return emptyStr;

    string cmdTemp;
    cmdTemp.append(pCommand->cmd);
    for (auto m = pCommand->vecParams.begin(); m != pCommand->vecParams.end(); ++m)
    {
      cmdTemp.append(" ");
      char strTemp[10] = {0};
      sprintf(strTemp, "%d", m->second);
      cmdTemp.append(strTemp);
    }
    //const static string GMcommand = cmdTemp;
    const string GMcommand = cmdTemp;
    return GMcommand;
  }

  const xLuaData getGMParams(const string& name)
  {
    xLuaData params;
    const SCommand* pCommand = getCommand(name);
    if (pCommand == NULL)
      return params;

    DWORD index = 1;
    for (auto m = pCommand->vecParams.begin(); m != pCommand->vecParams.end(); ++m)
    {
      std::ostringstream str;
      str << index;
      string strIndex = str.str().c_str();

      std::ostringstream str2;
      str2 << m->second;
      string strValue = str2.str().c_str();

      params.setData(strIndex, strValue);
    }

    return params;
  }

  static bool strToUserAttr(const string& attrStr, TVecAttrSvrs& vecAttrs)
  {
    //now 78个有效元素
    const DWORD MAX_DATAUSED = (DWORD) EATTRTYPE_MAX * 2;

    const static string roleDataUsed[MAX_DATAUSED] = 
    {
      "str", "9", "int", "10", "agi", "11", "dex", "12", "vit", "13", "luk", "14",
      "atk", "15", "atkper", "16", "def", "17", "defper", "18", "matk", "19", 
      "matkper", "20", "mdef", "21", "mdefper", "22", "refine", "23", 
      "mrefine", "24", "hit", "25", "flee", "26", "cri", "27", "cridamper", "28", 
      "crires", "29", "cridefper", "30", "parry", "31", "atkspd", "32", 
      "movespd", "33", "castspd", "34", "perfecthit", "35", "perfectflee", "36", 
      "hp", "37", "maxhp", "38", "showatk", "39", "showdef", "40", "showmatk", "41", 
      "showmdef", "42", "showhit", "43", "showflee", "44", "showcri", "45", 
      "showatkspd", "46", "showmovespd", "47"
    };

    vector<string> vecAttrStr;
    stringTok( attrStr, ",", vecAttrStr);
    for (auto m=vecAttrStr.begin(); m!=vecAttrStr.end(); ++m)
    {
      vector<string> vecStr;
      stringTok(*m, "=", vecStr);
      if (vecStr.size()!=2)
      {
          XERR << "[ItemConfig], Table_UseItem, UseEffect错误" << XEND;
          return false;
      }

      string itemStrTemp = vecStr[0];
      DWORD theValue = atoi(vecStr[1].c_str());
      transform(itemStrTemp.begin(), itemStrTemp.end(), itemStrTemp.begin(), ::tolower);

      for (DWORD i = 0; i < MAX_DATAUSED; i += 2)
      {
        if (itemStrTemp.compare( roleDataUsed[i] ) == 0)
        {
          DWORD intType = atoi( roleDataUsed[i+1].c_str() );
          if (EAttrType_IsValid(intType) == false)
            return false;
          EAttrType eType = static_cast <EAttrType> (intType);

          UserAttrSvr uAttr;
          uAttr.set_type(eType);
          uAttr.set_value(theValue);

          if (theValue ==0)
          {
            //return false
          }

          vecAttrs.push_back(uAttr);

          break;
        }
      }

    }

    return true;
  }

  static bool createEffect(SBaseEffect& sEffect, const string& str)
  {
    DWORD lcmd = str.find('{');
    DWORD rcmd = str.rfind('}');
    string attrStr;

    //如果字符串中包含“{}”，即包含cmd指令，则读取cmd指令 
    if (lcmd < str.size() && rcmd < str.size())
    {
      string cmdStr;
      vector<string> vecCmdStr;
      string attrStrTemp;
      string cmdStrTemp;
      cmdStrTemp.assign(str, lcmd, rcmd-lcmd+1);
      attrStrTemp.assign(str, rcmd, str.size()-rcmd+1);
      
      for (auto s = cmdStrTemp.begin(); s != cmdStrTemp.end(); ++s)
      {
        if ((*s) == '{') continue;
        if ((*s) == '}') continue;
        if ((*s) == ' ') continue;
        cmdStr.append(1, *s);
      }
      stringTok(cmdStr, ",", vecCmdStr);
      vecCmdStr.push_back("type=forConvient");
      
      SCommand stCommand;

      for (auto s = vecCmdStr.begin(); s != vecCmdStr.end(); ++s)
      {
        vector<string> vecOneCmd;
        string strLower = *s;
        transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
        stringTok(strLower, "=", vecOneCmd);

        if (vecOneCmd.size() != 2)
        {
          XERR << "[ItemConfig], Table_UseItem, UseEffect错误" << XEND;
          return false;
        }
        string keyStrTemp = vecOneCmd[0];
        //transform(keyStrTemp.begin(), keyStrTemp.end(), keyStrTemp.begin(), ::tolower);
        if (keyStrTemp.compare("type") == 0)
        {
          if (stCommand.vecParams.size() > 0)
            sEffect.vecCommands.push_back(stCommand);

          stCommand.cmd = vecOneCmd[1];
          stCommand.vecParams.clear();
          continue;
        }
        pair<string, DWORD> prTemp(vecOneCmd[0], atoi(vecOneCmd[1].c_str()));
        stCommand.vecParams.push_back(prTemp);
      }

      for (auto s = attrStrTemp.begin(); s != attrStrTemp.end(); ++s)
      {
        if ((*s) == '{') continue;
        if ((*s) == '}') continue;
        if ((*s) == ' ') continue;
        attrStr.append(1, *s);
      }
    }
    
    //如果字符串中无“{}”，即没有cmd指令，则直接处理字符串
    else
    {
      attrStr = str;
    }

    DWORD attrSize = attrStr.size();
    //提出cmd剩余的字符串长度大于2，说明有cmd指令外的其他指令，如 hp = 20,读取该部分
    if (attrSize>2)
    {
      TVecAttrSvrs vecAttrs;
      if (strToUserAttr(attrStr, vecAttrs) == false)
        return false;
      sEffect.vecAttrs.swap(vecAttrs);

    }
    return  true;
  }
};

enum NpcSkillEnumType
{
  NPC_SKILL_ATTACK = 0,
  NPC_SKILL_RECOVER,
  NPC_SKILL_BUFF,
  NPC_SKILL_DEBUFF,
  NPC_SKILL_MAX,
};*/

struct SHairColor : public xEntry
{
  DWORD colorid = 0;
  vector<pair<DWORD, DWORD>> vecStuff;

  SHairColor() {}
  bool init(DWORD key, xLuaData& value, const string& table)
  {
    set_id(key);
    colorid = key;

    xLuaData money = value.getMutableData("Money");
    auto func = [this](std::string keyer, xLuaData& data)
    {
      pair<DWORD, DWORD> pa;
      pa.first = data.getTableInt("item");
      pa.second = data.getTableInt("num");
      vecStuff.push_back(pa);
    };
    money.foreach(func);
    return true;
  }
};

/*enum EBossType
{
  EBOSSTYPE_MIN = 0,
  EBOSSTYPE_MVP = 1,
  EBOSSTYPE_MINI = 2,
  EBOSSTYPE_MAX
};

struct BossBase : public xTableEntry
{
  EBossType eType = EBOSSTYPE_MIN;
  TVecDWORD vecMap;
  map<DWORD, DWORD> mapReliveTime;

  bool init(DWORD key, xLuaData& value, const string& table)
  {
    if (!xTableEntry::init(key, value, table))
      return false;

    bool ok = true;

    DWORD t = getTableInt("Type");
    if (t <= EBOSSTYPE_MIN || t >= EBOSSTYPE_MAX)
    {
      XERR << "[BossConfig] Table_Boss, id:" << id << "Type配置错误" << XEND;
      ok = false;
      eType = EBOSSTYPE_MIN;
    }
    else
    {
      eType = static_cast<EBossType>(t);
    }

    vecMap.clear();
    if (!value.has("Map"))
    {
      XERR << "[BossConfig] Table_Boss, id:" << id << "Map未配置" << XEND;
      ok = false;
    }
    else
    {
      xLuaData oMaps = value.getMutableData("Map");
      auto f = [&](const string& key, xLuaData& data)
        {
          vecMap.push_back(data.getInt());
        };
      oMaps.foreach(f);
    }

    mapReliveTime.clear();
    if (!value.has("ReliveTime"))
    {
      XERR << "[BossConfig] Table_Boss, id:" << id << "ReliveTime未配置" << XEND;
      ok = false;
    }
    else
    {
      xLuaData oRTime = value.getMutableData("ReliveTime");
      auto f = [&](const string& key, xLuaData& data)
        {
          mapReliveTime[atoi(key.c_str())] = data.getInt();
        };
      oRTime.foreach(f);
    }

    // check
    if (mapReliveTime.size() <= 0 || mapReliveTime.size() != vecMap.size())
    {
      XERR << "[BossConfig] Table_Boss, id:" << id << "ReliveTime与Map配置不匹配" << XEND;
      ok = false;
    }
    else
    {
      for (auto &mapid : vecMap)
      {
        if (mapReliveTime.find(mapid) == mapReliveTime.end())
        {
          XERR << "[BossConfig] Table_Boss, id:" << id << "Map id:" << mapid << "在ReliveTime未找到" << XEND;
          ok = false;
        }
      }
    }

    return ok;
  }

  inline DWORD getMapID() const
  {
    if (vecMap.size() <= 0)
    {
      XERR << "[BossConfig] Table_Boss, id:" << id << "Map未配置" << XEND;
      return 0;
    }
    return vecMap[0];
  }
  inline const TVecDWORD& getAllMap() const
  {
    return vecMap;
  }
  inline DWORD getReliveTime(DWORD mapid) const
  {
    auto it = mapReliveTime.find(mapid);
    if (it == mapReliveTime.end())
    {
      XERR << "[BossConfig] Table_Boss, id:" << id << "ReliveTime map:" << mapid << "未配置" << XEND;
      return 120;
    }
    return it->second;
  }
  inline DWORD getRandomTime() const
  {
    return getTableInt("RandomTime");
  }
  inline const xLuaData& getBossDefine() const
  {
    return getData("BossSetup");
  }
  inline EBossType getType() const
  {
    return eType;
  }
};*/

// team goal
enum ETeamFilter
{
  ETEAMFILTER_MIN = 0,
  ETEAMFILTER_MAP = 1,
  ETEAMFILTER_TOWER = 2,
  ETEAMFILTER_SEAL = 3,
  ETEAMFILTER_LABORATORY = 4,
  ETEAMFILTER_MAPRANGE = 5,
  ETEAMFILTER_DOJO = 6,
  ETEAMFILTER_CHAT = 10,
  ETEAMFILTER_MAX
};
struct TeamGoalBase : public xTableEntry
{
  inline void collectFilterValue(TVecDWORD& vecFilter) const
  {
    const xLuaData& filterValue = getData("FilterValue");
    xLuaData filterCopy = filterValue;
    auto filterValuef = [&](const string& key, xLuaData& data)
    {
      vecFilter.push_back(data.getInt());
    };
    filterCopy.foreach(filterValuef);
  }

  inline ETeamFilter getTeamFilter() const
  {
    DWORD dwFilter = getTableInt("Filter");
    if (dwFilter == 1)
      return ETEAMFILTER_MAP;
    else if (dwFilter == 2)
      return ETEAMFILTER_TOWER;
    else if (dwFilter == 3)
      return ETEAMFILTER_SEAL;
    else if (dwFilter == 4)
      return ETEAMFILTER_LABORATORY;
    else if (dwFilter == 5)
      return ETEAMFILTER_MAPRANGE;
    else if (dwFilter == 6)
      return ETEAMFILTER_DOJO;
    else if (dwFilter == 10)
      return ETEAMFILTER_CHAT;

    return ETEAMFILTER_MIN;
  }
};

// mail
const DWORD MAIL_WEDDING_SEND = 12107;
struct MailBase : public xTableEntry
{

};

// interlocution
struct SInterlocution : public xTableEntry
{
  bool isCorrect(DWORD dwAnswer) const
  {
    const xLuaData& answer = getData("Answer");
    DWORD dwCorrect = answer.getTableInt("1");
    return dwCorrect == dwAnswer;
  }
};

struct MonsterEmojiItem
{
  DWORD emoji = 0;
  DWORD per = 0;
  DWORD cd = 0;
};

struct MonsterEmojiBase : public xTableEntry
{
  bool getItem(const char* t, MonsterEmojiItem &item) const
  {
    if (!t) return false;

    if (!has(t))
      return false;

    const xLuaData &data = getData(t);
    item.emoji = data.getTableInt("1");
    item.per = data.getTableInt("2");
    item.cd = data.getTableInt("3");

    return 0!=item.emoji;
  }
};

struct BusBase : public xTableEntry
{
  bool getPos(DWORD id, xPos &p) const
  {
    if (!has("DropPos"))
      return false;

    std::string str = std::to_string(id);
    const xLuaData &data = getData("DropPos");
    if (data.has(str))
    {
      const xLuaData &pos = data.getData(str.c_str());
      p.x = pos.getTableFloat("1");
      p.y = pos.getTableFloat("2");
      p.z = pos.getTableFloat("3");
      return true;
    }
    return false;
  }
  DWORD getType() const
  {
    return getTableInt("Type");
  }

  bool isWedding() const
  {
    return getType() == 4;
  }
};

struct SceneryBase : public xTableEntry
{
  inline DWORD getMapID() const
  {
    return getTableInt("MapName");
  }
  inline DWORD getMapIndex()
  {
    return getTableInt("MapNum");
  }
  inline DWORD getRewardID()
  {
    if (!has("Reward")) return 0;
    return getTableInt("Reward");
  }

  inline bool getPos(xPos& pos) const
  {
    if (!has("Coordinate"))
      return false;

    const xLuaData& p = getData("Coordinate");
    pos.x = p.getTableInt("1");
    pos.y = p.getTableInt("2");
    pos.z = p.getTableInt("3");

    return true;
  }

  inline DWORD getAdvectureValue() const
  {
    return getTableInt("AdventureValue");
  }

  inline void collectAdvBuffID(TVecDWORD& vecIDs) const
  {
    const xLuaData& adventure = getData("AdventureReward");
    const xLuaData& buff = adventure.getData("buffid");
    xLuaData oBuff = buff;
    auto bufff = [&](const string& str, xLuaData& data)
    {
      vecIDs.push_back(data.getInt());
    };
    oBuff.foreach(bufff);
  }
  inline void collectAdvItems(TVecItemInfo& vecItems) const
  {
    vecItems.clear();

    const xLuaData& adventure = getData("AdventureReward");
    const xLuaData& item = adventure.getData("item");
    xLuaData oItem = item;
    auto itemf = [&](const string& str, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      oItem.set_source(ESOURCE_MANUAL);
      vecItems.push_back(oItem);
    };
    oItem.foreach(itemf);
  }

  inline DWORD getType() const
  {
    return getTableInt("Type");
  }
};

struct DialogBase : public xTableEntry
{
  bool bInit = false;

  TVecDWORD vecSelects;
  inline const char * getText()
  {
    return getTableString("Text");
  }

  const TVecDWORD& getSelectList()
  {
    if (!bInit)
      init2();

    return vecSelects;
  }

  void init2()
  {
    DWORD dwFirst = 0;
    DWORD dwSecond = 0;
    const string& option = getTableString("Option");
    for (size_t i = 0; i < option.size(); ++i)
    {
      if (option[i] == ',')
        dwFirst = i;
      else if (option[i] == '}')
        dwSecond = i;

      if (dwFirst != 0 && dwSecond != 0 && dwSecond > dwFirst)
      {
        string subString = option.substr(dwFirst + 1, dwSecond - dwFirst - 1);
        vecSelects.push_back(atoi(subString.c_str()));
        dwFirst = dwSecond = 0;
      }
    }

    bInit = true;
  }
};

// 点唱机
struct SMusicBase : public xTableEntry
{

};

// 动作
struct SActionAnimBase : public xTableEntry
{

};

// 表情
struct SExpression : public xTableEntry
{

};

// 公会
struct SGuildBase : public xTableEntry
{

};


//NpcFunction
struct SNpcFunction : public xTableEntry
{
  inline DWORD getRaid() const
  {
    const xLuaData& data = getData("Parama");
    return data.getTableInt("raid");
  }
};

// 身体变色
struct SShaderColor : public xEntry
{
  DWORD dwColorID = 0;
  DWORD dwWeight = 0;

  SShaderColor() {}

  bool init(DWORD key, xLuaData& value, const string& table)
  {
    set_id(key);
    dwColorID = key;
    dwWeight = value.getTableInt("Weight");
    return true;
  }
};

//交易所物品分类显示 -- ItemTypeAdventureLog.xlsx
struct STradeItemTypeData :public xTableEntry
{
  inline std::string getName() const
  {
    return getTableString("Name");
  }

  inline DWORD getType() const
  {
    return getTableInt("type");
  }
  inline DWORD getHotSale() const
  {
    return getTableInt("HotSale");
  }
  inline DWORD getJobOption() const
  {
    return getTableInt("JobOption");
  }
  inline DWORD getExchangeOrder() const
  {
    return getTableInt("ExchangeOrder");
  }
};

// 定时活动
struct TimerBase : public xTableEntry
{
  inline std::string getStartTime() const
  {
    return getTableString("Start");
  }
  inline const xLuaData& getStartCmd() const
  {
    return getData().getData("StartCmd");
  }
  inline const xLuaData& getRestartCmd() const
  {
    return getData().getData("RestartCmd");
  }

  inline DWORD getRepeat() const
  {
    return getTableInt("Repeat");
  }
  inline DWORD getInterval() const
  {
    return getTableInt("Interval");
  }
  inline const xLuaData& getReapeatCmd() const
  {
    return getData().getData("ReapeatCmd");
  }

  inline std::string getEndTime() const
  {
    return getTableString("End");
  }
  inline const xLuaData& getEndCmd() const
  {
    return getData().getData("EndCmd");
  }
  inline std::string getInfo() const
  {
    return getTableString("Info");
  }
  inline DWORD getServerManager() const
  {
    return getTableInt("ServerManager");
  }
};

// buff
struct SBuffBase : public xTableEntry
{

};

// 特殊特效
struct SSpEffect : public xTableEntry
{
  inline DWORD getType() const
  {
    return getTableInt("Type");
  }
  inline const DWORD getDuration() const
  {
    return getTableInt("Duration");
  }

  inline xLuaData getOnAdd() const
  {
    return getData("OnAdd");
  }

  inline xLuaData getOnDel() const
  {
    return getData("OnDel");
  }
  inline bool isDelWhenDieOrNotSameMap() const
  {
    return getTableInt("DeathDel") == 1;
  }
  inline bool isOutRangeDel() const
  {
    return has("MaxDistance");
  }
  inline float getMaxDis() const
  {
    return getTableFloat("MaxDistance");
  }
};

// 情书
struct SLoveLetter : public xTableEntry
{
  inline const DWORD getType() const
  {
    return getTableInt("Type");
  }
  inline const string getContent() const
  {
    return getTableString("Letter");
  }
};

// 厨师等级
struct SCookerLevel : public xTableEntry
{
  inline const DWORD getLevel() const
  {
    return getTableInt("Level");
  }
  inline const DWORD getNeedExp() const
  {
    return getTableInt("NeedExp");
  }
  inline const DWORD getRewardItem() const
  {
    return getTableInt("RewardItem");
  }
  inline const DWORD getRewardBagSlot() const
  {
    return getTableInt("RewardBagSlot");
  }
  inline const DWORD getSuccessRate() const
  {
    return getTableInt("SuccessRate");
  }
  inline const DWORD getTitle() const
  {
    return getTableInt("Title");
  }

  inline DWORD getExpRate(DWORD dwLv) 
  {   
    init2();
    auto it = m_mapExpRate.find(dwLv);
    if (it == m_mapExpRate.end())
      return 100;
    return it->second;
  }

  const TVecDWORD& getUnlockRecipe() 
  {
    init2();
    return m_vecUnlockRecipe;
  }
  
  void init2()
  {
    if (inited == true)
      return;

    xLuaData d = getData("ExpRate");
    auto dataf = [this](const string& key, xLuaData& data)
    {
      DWORD lv = atoi(key.c_str());
      DWORD rate = data.getInt();
      m_mapExpRate[lv] = rate;
    };
    d.foreach(dataf);
    
    xLuaData d2 = getData("Recipe");
    auto dataf2 = [this](const string& key, xLuaData& data)
    {
      m_vecUnlockRecipe.push_back(data.getInt());
      XDBG << "wld" << data.getInt() << XEND;
    };
    d2.foreach(dataf2);
    inited = true;
  }

public:
  map<DWORD, DWORD> m_mapExpRate;
  TVecDWORD m_vecUnlockRecipe;
  bool inited = false;
};

// 美食家等级
struct STasterLevel : public xTableEntry
{
  inline const DWORD getLevel() const
  {
    return getTableInt("Level");
  }
  inline const DWORD getNeedExp() const
  {
    return getTableInt("NeedExp");
  }
  inline const DWORD getFullProgress() const
  {
    return getTableInt("FullProgress");
  }
  inline const DWORD getTitle() const
  {
    return getTableInt("Title");
  }
  inline const DWORD getAddFoods() const
  {
    return getTableInt("AddBuffs");
  }
};

// 特效
struct SEffect : public xTableEntry
{
  inline const xLuaData& getGMData() const
  {
    return getData("GMData");
  }
};

//音效
struct SSoundEffect : public xTableEntry
{
  inline const xLuaData& getGMData() const
  {
    return getData("GMData");
  }
};

// 活动面板寻路
struct SActShortcutPower : public xTableEntry
{
  inline DWORD getType() const
  {
    return getTableInt("Type");
  }
  inline string getEvent() const
  {
    return getTableString("Event");
  }
};

// body
struct SBody : public xTableEntry
{

};

// 答题
struct SQuestion : public xTableEntry
{
  inline DWORD getRightAnswer() const
  {
    return getTableInt("Right");
  }
  const char* getQuestionStr() const
  {
    return getTableString("Title");
  }
  const char* getAnswerStr(DWORD answer) const
  {
    auto it = m_mapAnswerID2Str.find(answer);
    return it != m_mapAnswerID2Str.end() ? it->second.c_str() : nullptr;
  }

  virtual bool init(DWORD key, xLuaData &value, const string& table)
  {
    if (xTableEntry::init(key, value, table) == false)
      return false;

    m_mapAnswerID2Str.clear();

    auto getstr = [&](const string& k, xLuaData& d)
    {
      DWORD answer = d.getTableInt("id");
      const char* str = d.getTableString("content");
      if (answer && str)
        m_mapAnswerID2Str[answer] = str;
    };
    value.getMutableData("Option").foreach(getstr);

    return true;
  }

  private:
    std::map<DWORD, string> m_mapAnswerID2Str;
};

// 服装
struct SCloth : public xTableEntry
{
  inline EGender getGender() const { return static_cast<EGender>(getTableInt("Sex")); }
  inline EProfession getProfession() const { return static_cast<EProfession>(getTableInt("ClassID")); }
  inline DWORD getClothID() const { return getTableInt("ClothColor"); }
  inline string getName() const { return getTableString("NameZh"); }
  inline bool isOnSale() const { return getTableInt("OnSale") == 1; }
};

//兑换商店
struct SExchangeShop : public xTableEntry
{
};

//兑换材料权重配置
struct SExchangeWorth : public xTableEntry
{
};
