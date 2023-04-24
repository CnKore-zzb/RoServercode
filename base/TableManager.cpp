#include "TableManager.h"
#include "config/MiscConfig.h"

bool TableManager::loadConfig(ConfigType eType)
{
  bool bResult = true;
  if (eType == ConfigType::buffer)
  {
    SAFE_DELETE(m_pBuffBaseM);
    m_pBuffBaseM = new Table<SBuffBase>("Lua/Table/Table_Buffer.txt", "Table_Buffer");
    bResult = m_pBuffBaseM == nullptr ? false : m_pBuffBaseM->load();
  }
  else if (eType == ConfigType::music)
  {
    SAFE_DELETE(m_pMusicBaseM);
    m_pMusicBaseM = new Table<SMusicBase>("Lua/Table/Table_MusicBox.txt", "Table_MusicBox");
    bResult = m_pMusicBaseM == nullptr ? false : m_pMusicBaseM->load();
  }
  /*else if (eType == ConfigType::boss)
  {
    SAFE_DELETE(m_pBossBaseM);
    m_pBossBaseM = new Table<BossBase>("Lua/Table/Table_Boss.txt", "Table_Boss");
    bResult = m_pBossBaseM == nullptr ? false : m_pBossBaseM->load();
  }*/
  else if (eType == ConfigType::haircolor)
  {
    SAFE_DELETE(m_pHairColorM);
    m_pHairColorM = new Table<SHairColor>("Lua/Table/Table_HairColor.txt", "Table_HairColor");
    bResult = m_pHairColorM == nullptr ? false : m_pHairColorM->load();
  }
  else if (eType == ConfigType::mail)
  {
    SAFE_DELETE(m_pMailBaseM);
    m_pMailBaseM = new Table<MailBase>("Lua/Table/Table_Mail.txt", "Table_Mail");
    bResult = m_pMailBaseM == nullptr ? false : m_pMailBaseM->load();
  }
  else if (eType == ConfigType::monsteremoji)
  {
    SAFE_DELETE(m_pMonsterEmoji);
    m_pMonsterEmoji = new Table<MonsterEmojiBase>("Lua/Table/Table_MonsterEmoji.txt", "Table_MonsterEmoji");
    bResult = m_pMonsterEmoji == nullptr ? false : m_pMonsterEmoji->load();
  }
  else if (eType == ConfigType::interlocution)
  {
    SAFE_DELETE(m_pInterlocution);
    m_pInterlocution = new Table<SInterlocution>("Lua/Table/Table_xo_server.txt", "Table_xo_server");
    bResult = m_pInterlocution == nullptr ? false : m_pInterlocution->load();
  }
  else if (eType == ConfigType::bus)
  {
    SAFE_DELETE(m_pBusBaseM);
    m_pBusBaseM = new Table<BusBase>("Lua/Table/Table_Bus.txt", "Table_Bus");
    bResult = m_pBusBaseM == nullptr ? false : m_pBusBaseM->load();
  }
  else if (eType == ConfigType::scenery)
  {
    SAFE_DELETE(m_pSceneryBaseM);
    m_pSceneryBaseM = new Table<SceneryBase>("Lua/Table/Table_Viewspot.txt", "Table_Viewspot");
    bResult = m_pSceneryBaseM == nullptr ? false : m_pSceneryBaseM->load();
  }
  else if (eType == ConfigType::dialog)
  {
    SAFE_DELETE(m_pDialogBaseM);
    m_pDialogBaseM = new Table<DialogBase>("Lua/Table/Table_Dialog.txt", "Table_Dialog");
    bResult = m_pDialogBaseM == nullptr ? false : m_pDialogBaseM->load();
  }
  else if (eType == ConfigType::actionanim)
  {
    SAFE_DELETE(m_pActionAnimBaseM);
    m_pActionAnimBaseM = new Table<SActionAnimBase>("Lua/Table/Table_ActionAnime.txt", "Table_ActionAnime");
    if (m_pActionAnimBaseM == nullptr)
      bResult = false;
    m_pActionAnimBaseM->load();
  }
  else if (eType == ConfigType::expression)
  {
    SAFE_DELETE(m_pExpressionBaseM);
    m_pExpressionBaseM = new Table<SExpression>("Lua/Table/Table_Expression.txt", "Table_Expression");
    bResult = m_pExpressionBaseM == nullptr ? false : m_pExpressionBaseM->load();
  }
  else if (eType == ConfigType::teamgoal)
  {
    SAFE_DELETE(m_pTeamGoalM);
    m_pTeamGoalM = new Table<TeamGoalBase>("Lua/Table/Table_TeamGoals.txt", "Table_TeamGoals");
    bResult = m_pTeamGoalM == nullptr ? false : m_pTeamGoalM->load();
  }
  else if (eType == ConfigType::npcfun)
  {
    SAFE_DELETE(m_pNpcFunctionM);
    m_pNpcFunctionM = new Table<SNpcFunction>("Lua/Table/Table_NpcFunction.txt", "Table_NpcFunction");
    bResult = m_pNpcFunctionM == nullptr ? false : m_pNpcFunctionM->load();
  }
  else if (eType == ConfigType::shadercolor)
  {
    SAFE_DELETE(m_pShaderColorBaseM);
    m_pShaderColorBaseM = new Table<SShaderColor>("Lua/Table/Table_ShaderColor.txt", "Table_ShaderColor");
    bResult = m_pShaderColorBaseM == nullptr ? false : m_pShaderColorBaseM->load();
  }
  else if (eType == ConfigType::tradetype)
  {
    SAFE_DELETE(m_pTradeItemTypeM);
    m_pTradeItemTypeM = new Table<STradeItemTypeData>("Lua/Table/Table_ItemTypeAdventureLog.txt", "Table_ItemTypeAdventureLog");
    bResult = m_pTradeItemTypeM == nullptr ? false : m_pTradeItemTypeM->load();
  }
  else if (eType == ConfigType::timer)
  {
    SAFE_DELETE(m_pTimerBaseM);
    TVecString files;
    TVecString tables;
    const STimerTableCFG& rCFG = MiscConfig::getMe().getTimerTableCFG();
    for (auto v = rCFG.vecTables.begin(); v != rCFG.vecTables.end(); ++v)
    {
      string path = "Lua/Table/";
      path = path + *v;
      path = path + ".txt";

      files.push_back(path);
      tables.push_back(*v);
      XLOG << "[Table_Timer],加载表格," << path << *v << XEND;
    }
    //m_pTimerBaseM = new Table<TimerBase>("Lua/Table/Table_Timer.txt", "Table_Timer");
    m_pTimerBaseM = new Table<TimerBase>(files, tables);
    bResult = m_pTimerBaseM == nullptr ? false : m_pTimerBaseM->load();
  }
  else if (eType == ConfigType::loveletter)
  {
    SAFE_DELETE(m_pLoveLetterM);
    m_pLoveLetterM = new Table<SLoveLetter>("Lua/Table/Table_LoveLetter.txt", "Table_LoveLetter");
    bResult = m_pLoveLetterM == nullptr ? false : m_pLoveLetterM->load();
  }
  else if (eType == ConfigType::speffect)
  {
    SAFE_DELETE(m_pSpEffectM);
    m_pSpEffectM = new Table<SSpEffect>("Lua/Table/Table_SpEffect.txt", "Table_SpEffect");
    bResult = m_pSpEffectM == nullptr ? false : m_pSpEffectM->load();
  }
  else if (eType == ConfigType::cookerlevel)
  {
    SAFE_DELETE(m_pCookerLevel);
    m_pCookerLevel = new Table<SCookerLevel>("Lua/Table/Table_CookerLevel.txt", "Table_CookerLevel");
    bResult = m_pCookerLevel == nullptr ? false : m_pCookerLevel->load();
  }  
  else if (eType == ConfigType::tasterlevel)
  {
    SAFE_DELETE(m_pTasterLevel);
    m_pTasterLevel = new Table<STasterLevel>("Lua/Table/Table_TasterLevel.txt", "Table_TasterLevel");
    bResult = m_pTasterLevel == nullptr ? false : m_pTasterLevel->load();
  }
  else if (eType == ConfigType::effect)
  {
    SAFE_DELETE(m_pEffect);
    m_pEffect = new Table<SEffect>("Lua/Table/Table_Effect.txt", "Table_Effect");
    bResult = m_pEffect == nullptr ? false : m_pEffect->load();
  }
  else if (eType == ConfigType::soundeffect)
  {
    SAFE_DELETE(m_pSoundEffect);
    m_pSoundEffect = new Table<SSoundEffect>("Lua/Table/Table_SoundEffect.txt", "Table_SoundEffect");
    bResult = m_pSoundEffect == nullptr ? false : m_pSoundEffect->load();
  }
  else if (eType == ConfigType::actshortcutpower)
  {
    SAFE_DELETE(m_pActShortcutPowerM);
    m_pActShortcutPowerM = new Table<SActShortcutPower>("Lua/Table/Table_ActivityShortcutPower.txt", "Table_ActivityShortcutPower");
    bResult = m_pActShortcutPowerM == nullptr ? false : m_pActShortcutPowerM->load();
  }
  else if (eType == ConfigType::body)
  {
    SAFE_DELETE(m_pBodyM);
    m_pBodyM = new Table<SBody>("Lua/Table/Table_Body.txt", "Table_Body");
    bResult = m_pBodyM == nullptr ? false : m_pBodyM->load();
  }
  else if (eType == ConfigType::question)
  {
    SAFE_DELETE(m_pQuestionM);
    m_pQuestionM = new Table<SQuestion>("Lua/Table/Table_Question.txt", "Table_Question");
    bResult = m_pQuestionM == nullptr ? false : m_pQuestionM->load();
  }
  else if (eType == ConfigType::cloth)
  {
    SAFE_DELETE(m_pClothM);
    m_pClothM = new Table<SCloth>("Lua/Table/Table_Couture.txt", "Table_Couture");
    bResult = m_pClothM == nullptr ? false : m_pClothM->load();
  }
  else if (eType == ConfigType::exchangeshop)
  {
    SAFE_DELETE(m_pExchangeShopM);
    m_pExchangeShopM = new Table<SExchangeShop>("Lua/Table/Table_ExchangeShop.txt", "Table_ExchangeShop");
    bResult = m_pExchangeShopM == nullptr ? false : m_pExchangeShopM->load();
  }
  else if (eType == ConfigType::exchangeworth)
  { 
    SAFE_DELETE(m_pExchangeWorthM);
    m_pExchangeWorthM = new Table<SExchangeWorth>("Lua/Table/Table_ExchangeWorth.txt", "Table_ExchangeWorth");
    bResult = m_pExchangeWorthM == nullptr ? false : m_pExchangeWorthM->load();
  }
  return bResult;
}

bool TableManager::checkConfig(ConfigType eType)
{
  bool bResult = true;
  if (eType == ConfigType::buffer)
  {
    // 在场景BufferManager已做合法检查
  }
  else if (eType == ConfigType::music)
  {
  }
  else if (eType == ConfigType::boss)
  {
  }
  else if (eType == ConfigType::haircolor)
  {
  }
  else if (eType == ConfigType::mail)
  {
  }
  else if (eType == ConfigType::monsteremoji)
  {
  }
  else if (eType == ConfigType::interlocution)
  {
  }
  else if (eType == ConfigType::bus)
  {
  }
  else if (eType == ConfigType::scenery)
  {
    bResult = m_pSceneryBaseM != nullptr;
    if (bResult)
    {
      auto func = [](const xEntry* entry) {
        const SceneryBase* pBase = dynamic_cast<const SceneryBase*>(entry);
        if (pBase != nullptr)
          TableManager::getMe().m_mapMapID2SceneryCount[pBase->getMapID()]++;
      };
      m_pSceneryBaseM->foreachNoConst(func);
    }
  }
  else if (eType == ConfigType::dialog)
  {
  }
  else if (eType == ConfigType::actionanim)
  {
  }
  else if (eType == ConfigType::expression)
  {
  }
  else if (eType == ConfigType::teamgoal)
  {
  }
  else if (eType == ConfigType::npcfun)
  {
  }
  else if (eType == ConfigType::shadercolor)
  {
  }
  else if (eType == ConfigType::tradetype)
  {
  }
  else if (eType == ConfigType::timer)
  {
  }
  else if (eType == ConfigType::loveletter)
  {
    bResult = m_pLoveLetterM != nullptr;
    if (bResult)
    {
      auto func = [](xEntry* entry) {
        if (entry != nullptr)
        {
          SLoveLetter* pLetter = dynamic_cast<SLoveLetter*>(entry);
          if (pLetter)
          {
            TSetDWORD& rSet = TableManager::getMe().m_mapLoveLetterID[pLetter->getType()];
            rSet.insert(entry->id);

          }
        }
      };
      m_pLoveLetterM->foreachNoConst(func);
    }
  }
  else if (eType == ConfigType::effect)
  {
  }
  else if (eType == ConfigType::soundeffect)
  {
  }
  else if (eType == ConfigType::actshortcutpower)
  {
  }
  else if (eType == ConfigType::body)
  {
  }
  else if (eType == ConfigType::wedding)
  {
  }
  return bResult;
}

DWORD TableManager::getSceneryCount(DWORD dwMapID) const
{
  auto m = m_mapMapID2SceneryCount.find(dwMapID);
  if (m != m_mapMapID2SceneryCount.end())
    return m->second;
  return 0;
}

