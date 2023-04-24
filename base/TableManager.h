#pragma once

#include "xEntryManager.h"
#include "xSingleton.h"
#include "xLog.h"
#include "xLuaTable.h"
#include "TableStruct.h"

template <typename T>
class Table : public xEntryManager<xEntryID, xEntryMultiName>
{
  public:
    Table(const char* file, const char *table)
    {
      m_vecFiles.clear();
      m_vecTables.clear();
      m_blAdd = false;

      m_vecFiles.push_back(file);
      m_vecTables.push_back(table);
    }
    Table(const TVecString& files, const TVecString& tables, bool add=false)
    {
      m_vecFiles.clear();
      m_vecTables.clear();

      m_vecFiles = files;
      m_vecTables = tables;
      m_blAdd = add;
    }
    virtual ~Table()
    {
      for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
        SAFE_DELETE(m->second);
      xEntryID::ets_.clear();
      xEntryMultiName::ets_.clear();
    }
    T* getDataByID(DWORD _id)
    {
      return (T *)getEntryByID(_id);
    }
    T* getDataByName(char *_name)
    {
      return (T *)getEntryByName(_name);
    }
    bool load()
    {
      if (m_vecFiles.empty() == true || m_vecTables.empty() == true || m_vecFiles.size() != m_vecTables.size())
        return false;

      // load main
      if (loadMain(m_vecFiles[0], m_vecTables[0]) == false)
        return false;

      // load sub
      for (size_t i = 1; i < m_vecFiles.size(); ++i)
      {
        if (loadSub(m_vecFiles[i], m_vecTables[i]) == false)
          return false;
      }

      return true;
    }
    template<class I> void foreach(I func) const
    {
      for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
        func(m->second);
    }
    template<class I> void foreachNoConst(I func)
    {
      for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
        func(m->second);
    }
  private:
    bool loadMain(const string& file, const string& tab)
    {
      if (!xLuaTable::getMe().open(file.c_str()))
      {
        XERR << "[表格],加载表格," << file << "失败" << XEND;
        return false;
      }
      XLOG << "[表格],加载表格," << file << XEND;

      xLuaTableData table;
      xLuaTable::getMe().getLuaTable(tab.c_str(), table);

      for (auto it=table.begin(); it!= table.end(); ++it)
      {
        bool isNew = false;
        T *base = (T *)getEntryByID(it->first);
        if (!base)
        {
          base = new T;
          isNew = true;
        }
        if (base->init(it->first, it->second, tab.c_str()) == false)
        {
          XERR << "[" << tab << "] key =" << it->first << " init error!" << XEND;
          return false;
        }
        //XDBG("[加载数据],%llu,%s", base->id, base->name);
        if (!base->id)
        {
          XERR << "[" << tab << "] key =" << it->first << " id invalid!" << XEND;
          //return false;
          removeEntry(base);
          SAFE_DELETE(base);
          continue;
        }

        if (isNew && !addEntry(base))
        {
          XERR << "[加载数据]," << base->id << base->name << "加入管理器错误" << XEND;
          return false;
        }
      }
      return true;
    }

    bool loadSub(const string& file, const string& tab)
    {
      if (!xLuaTable::getMe().open(file.c_str()))
      {
        XERR << "[表格],加载表格," << file << ",失败" << XEND;
        return false;
      }
      XLOG << "[表格],加载表格," << file << XEND;

      xLuaTableData table;
      xLuaTable::getMe().getLuaTable(tab.c_str(), table);

      for (auto it=table.begin(); it!= table.end(); ++it)
      {
        DWORD id = it->first;
        bool isNew = false;
        T *base = (T *)getEntryByID(id);
        if (!base)
        {
          base = new T;
          isNew = true;
        }
        if (base == NULL)
        {
          XERR << "[" << tab << "] id =" << id << "can not find in" << m_vecTables[0] << XEND;
          return false;
        }
        if (base->init(it->first, it->second, tab.c_str()) == false)
        {
          XERR << "[" << tab << "] key =" << it->first << "init error!" << XEND;
          return false;
        }
        if (!base->id)
        {
          XERR << "[" << tab << "] key =" << it->first << "id invalid!" << XEND;
          removeEntry(base);
          SAFE_DELETE(base);
          continue;
        }

        if (isNew && !addEntry(base))
        {
          XERR << "[加载数据]" << base->id << base->name << "加入管理器错误" << XEND;
          return false;
        }
        //XDBG("[加载数据],%llu,%s", base->id, base->name);
      }

      return true;
    }
  private:
    TVecString m_vecFiles;
    TVecString m_vecTables;
    bool m_blAdd;
};

enum ConfigType
{
  roledata = 0,
  misc,
  item,
  reward,
  superai,
  npc,
  role,
  attributepoint,
  baselevel,
  joblevel,
  compose,
  menu,
  portrait,
  quest,
  skill,
  feature,
  seal,
  tower,
  manual,
  shop,
  tip,
  buffer,
  music,
  scene,
  raid,
  lab,
  boss,
  haircolor,
  mail,
  monsteremoji,
  interlocution,
  bus,
  scenery,
  dialog,
  actionanim,
  expression,
  teamgoal,
  guild,
  dojo,
  npcfun,
  avatar,
  shadercolor,
  tradetype,
  treasure,
  activity,
  xoclient,
  timer,
  operatereward,
  speffect,
  deposit,
  valentine,
  weaponpet,
  loveletter,
  guildraid,
  achieve,
  dateland,
  astrolabe,
  pet,
  food,
  recipe,
  cookerlevel,
  tasterlevel,
  effect,
  soundeffect,
  actshortcutpower,
  body,
  tutor,
  action,
  being,
  highrefine,
  wedding,
  question,
  pvecard,
  servant,
  cloth,
  sysmsg,
  transfer,
  exchangeshop,
  exchangeworth,
  dead,
  max,
};

const DWORD BUS_FERRISWHEEL = 7;
const DWORD BUS_DIVORCEROLLER = 22;

// TableManager
class TableManager : public xSingleton<TableManager>
{
  friend class xSingleton<TableManager>;
  private:
    TableManager() {}
  public:
    virtual ~TableManager() {}

    bool loadConfig(ConfigType eType);
    bool checkConfig(ConfigType eType);

    const Table<SBuffBase>* getBufferCFGList() const { return m_pBuffBaseM; }
    //const Table<BossBase>* getBossCFGList() const { return m_pBossBaseM; }
    const Table<SceneryBase>* getSceneryCFGList() const { return m_pSceneryBaseM; }
    const Table<SActionAnimBase>* getActionAnimCFGList() const { return m_pActionAnimBaseM; }
    const Table<SExpression>* getExpressionCFGList() const { return m_pExpressionBaseM; }
    const Table<STradeItemTypeData>* getTradeItemTypeCFGList() const { return m_pTradeItemTypeM; }
    const Table<SExchangeShop>* getExchangeShopCFGList() const { return m_pExchangeShopM; }
    const Table<SExchangeWorth>* getExchangeWorthCFGList() const { return m_pExchangeWorthM; }

    //Table<BossBase>* getBossCFGListNoConst() { return m_pBossBaseM; }
    Table<SBuffBase>* getBufferCFGListNoConst() { return m_pBuffBaseM; }
    Table<STradeItemTypeData>* getTradeItemTypeCFGListNoConst() { return m_pTradeItemTypeM; }
    Table<SActionAnimBase>* getActionAnimCFGListNoConst() { return m_pActionAnimBaseM; }
    Table<SExpression>* getExpressionCFGListNoConst() { return m_pExpressionBaseM; }
    Table<SExchangeShop>* getExchangeShopCFGListNoConst() { return m_pExchangeShopM; }
    Table<SExchangeWorth>* getExchangeWorthCFGListNoConst() { return m_pExchangeWorthM; }

    const SBuffBase* getBufferCFG(DWORD dwID) const { return m_pBuffBaseM == nullptr ? nullptr : dynamic_cast<const SBuffBase*>(m_pBuffBaseM->getDataByID(dwID)); }
    const SMusicBase* getMusicCFG(DWORD dwID) const { return m_pMusicBaseM == nullptr ? nullptr : dynamic_cast<const SMusicBase*>(m_pMusicBaseM->getDataByID(dwID)); }
    //const BossBase* getBossCFG(DWORD dwID) const { return m_pBossBaseM == nullptr ? nullptr : dynamic_cast<const BossBase*>(m_pBossBaseM->getDataByID(dwID)); }
    const SHairColor* getHairColorCFG(DWORD dwID) const { return m_pHairColorM == nullptr ? nullptr : dynamic_cast<const SHairColor*>(m_pHairColorM->getDataByID(dwID)); }
    const MailBase* getMailCFG(DWORD dwID) const { return m_pMailBaseM == nullptr ? nullptr : dynamic_cast<const MailBase*>(m_pMailBaseM->getDataByID(dwID)); }
    const MonsterEmojiBase* getMonsterEmojiCFG(DWORD dwID) const { return m_pMonsterEmoji == nullptr ? nullptr : dynamic_cast<const MonsterEmojiBase*>(m_pMonsterEmoji->getDataByID(dwID)); }
    const SInterlocution* getInterCFG(DWORD dwID) const { return m_pInterlocution == nullptr ? nullptr : dynamic_cast<const SInterlocution*>(m_pInterlocution->getDataByID(dwID)); }
    const BusBase* getBusCFG(DWORD dwID) const { return m_pBusBaseM == nullptr ? nullptr : dynamic_cast<const BusBase*>(m_pBusBaseM->getDataByID(dwID)); }
    const SceneryBase* getSceneryCFG(DWORD dwID) const { return m_pSceneryBaseM == nullptr ? nullptr : dynamic_cast<const SceneryBase*>(m_pSceneryBaseM->getDataByID(dwID)); }
    const DialogBase* getDialogCFG(DWORD dwID) const { return m_pDialogBaseM == nullptr ? nullptr : dynamic_cast<const DialogBase*>(m_pDialogBaseM->getDataByID(dwID)); }
    const SActionAnimBase* getActionAnimCFG(DWORD dwID) const { return m_pActionAnimBaseM == nullptr ? nullptr : dynamic_cast<const SActionAnimBase*>(m_pActionAnimBaseM->getDataByID(dwID)); }
    const SExpression* getExpressionCFG(DWORD dwID) const { return m_pExpressionBaseM == nullptr ? nullptr : dynamic_cast<const SExpression*>(m_pExpressionBaseM->getDataByID(dwID)); }
    const TeamGoalBase* getTeamCFG(DWORD dwID) const { return m_pTeamGoalM == nullptr ? nullptr : dynamic_cast<const TeamGoalBase*>(m_pTeamGoalM->getDataByID(dwID)); }
    const SNpcFunction* getNpcFuncCFG(DWORD dwID) const { return m_pNpcFunctionM == nullptr ? nullptr : dynamic_cast<const SNpcFunction*>(m_pNpcFunctionM->getDataByID(dwID)); }
    const SShaderColor* getShaderColorCFG(DWORD dwID) const { return m_pShaderColorBaseM == nullptr ? nullptr : dynamic_cast<const SShaderColor*>(m_pShaderColorBaseM->getDataByID(dwID)); }
    const STradeItemTypeData* getTradeItemCFG(DWORD dwID) const { return m_pTradeItemTypeM == nullptr ? nullptr : dynamic_cast<const STradeItemTypeData*>(m_pTradeItemTypeM->getDataByID(dwID)); }
    const SSpEffect* getSpEffectCFG(DWORD dwID) const { return m_pSpEffectM == nullptr ? nullptr : dynamic_cast<const SSpEffect*>(m_pSpEffectM->getDataByID(dwID)); }
    const TimerBase* getTimerCFG(DWORD dwID) const { return m_pTimerBaseM == nullptr ? nullptr : dynamic_cast<const TimerBase*>(m_pTimerBaseM->getDataByID(dwID)); }
    const SLoveLetter* getLoveLetterCFG(DWORD dwID) const { return m_pLoveLetterM == nullptr ? nullptr : dynamic_cast<const SLoveLetter*>(m_pLoveLetterM->getDataByID(dwID)); }
    const SCookerLevel* getCookerLevelCFG(DWORD dwID) const { return m_pCookerLevel == nullptr ? nullptr : dynamic_cast<const SCookerLevel*>(m_pCookerLevel->getDataByID(dwID)); }
    const STasterLevel* getTasterLevelCFG(DWORD dwID) const { return m_pTasterLevel == nullptr ? nullptr : dynamic_cast<const STasterLevel*>(m_pTasterLevel->getDataByID(dwID)); }
    const SEffect* getEffectCFG(DWORD dwID) const { return m_pEffect == nullptr ? nullptr : dynamic_cast<const SEffect*>(m_pEffect->getDataByID(dwID)); }
    const SSoundEffect* getSoundEffectCFG(DWORD dwID) const { return m_pSoundEffect == nullptr ? nullptr : dynamic_cast<const SSoundEffect*>(m_pSoundEffect->getDataByID(dwID)); }
    const SActShortcutPower* getActShortcutPowerCFG(DWORD dwID) const { return m_pActShortcutPowerM == nullptr ? nullptr : dynamic_cast<const SActShortcutPower*>(m_pActShortcutPowerM->getDataByID(dwID)); }
    const SBody* getBody(DWORD dwID) const { return m_pBodyM == nullptr ? nullptr : dynamic_cast<const SBody*>(m_pBodyM->getDataByID(dwID)); }
    const SQuestion* getQuestion(DWORD dwID) const { return m_pQuestionM == nullptr ? nullptr : dynamic_cast<const SQuestion*> (m_pQuestionM->getDataByID(dwID)); }
    const SCloth* getClothCFG(DWORD dwID) const { return m_pClothM == nullptr ? nullptr : dynamic_cast<const SCloth*>(m_pClothM->getDataByID(dwID)); }
    const SExchangeShop* getExchangeShopCFG(DWORD dwID) const { return m_pExchangeShopM == nullptr ? nullptr : dynamic_cast<const SExchangeShop*>(m_pExchangeShopM->getDataByID(dwID)); }
    const SExchangeWorth* getExchangeWorthCFG(DWORD dwID) const { return m_pExchangeWorthM == nullptr ? nullptr : dynamic_cast<const SExchangeWorth*>(m_pExchangeWorthM->getDataByID(dwID)); }

    DWORD randLoveLetter(DWORD type)
    {
      auto it = m_mapLoveLetterID.find(type);
      if (it == m_mapLoveLetterID.end())
        return 0;
      TSetDWORD& rSet = it->second;
      if (rSet.empty())
        return 0;
      return *randomStlContainer(rSet);
    }
    DWORD getSceneryCount(DWORD dwMapID) const;
  private:
    Table<SBuffBase>* m_pBuffBaseM = nullptr;
    Table<SMusicBase>* m_pMusicBaseM = nullptr;
    //Table<BossBase>* m_pBossBaseM = nullptr;
    Table<SHairColor>* m_pHairColorM = nullptr;
    Table<MailBase>* m_pMailBaseM = nullptr;
    Table<MonsterEmojiBase>* m_pMonsterEmoji = nullptr;
    Table<SInterlocution>* m_pInterlocution = nullptr;
    Table<BusBase>* m_pBusBaseM = nullptr;
    Table<SceneryBase>* m_pSceneryBaseM = nullptr;
    Table<DialogBase>* m_pDialogBaseM = nullptr;
    Table<SActionAnimBase>* m_pActionAnimBaseM = nullptr;
    Table<SExpression>* m_pExpressionBaseM = nullptr;
    Table<TeamGoalBase>* m_pTeamGoalM = nullptr;
    Table<SNpcFunction>* m_pNpcFunctionM = nullptr;
    Table<SShaderColor>* m_pShaderColorBaseM = nullptr;
    Table<STradeItemTypeData>* m_pTradeItemTypeM = nullptr;
    Table<SSpEffect>* m_pSpEffectM = nullptr;
    Table<SLoveLetter>* m_pLoveLetterM = nullptr;
    Table<SActShortcutPower>* m_pActShortcutPowerM = nullptr;

    std::map<DWORD/*type*/, TSetDWORD> m_mapLoveLetterID;
    map<DWORD, DWORD> m_mapMapID2SceneryCount;
    Table<SCookerLevel>* m_pCookerLevel = nullptr;
    Table<STasterLevel>* m_pTasterLevel = nullptr;
    Table<SEffect>* m_pEffect = nullptr;
    Table<SSoundEffect>* m_pSoundEffect = nullptr;
    Table<SBody>* m_pBodyM = nullptr;
    Table<SQuestion>* m_pQuestionM = nullptr;
    Table<SCloth>* m_pClothM = nullptr;
    Table<SExchangeShop>* m_pExchangeShopM = nullptr;
    Table<SExchangeWorth>* m_pExchangeWorthM = nullptr;
  public:
    Table<TimerBase>* m_pTimerBaseM = nullptr;
};

struct SBaseCFG
{
  bool blInit = false;
  SBaseCFG() {}
};

