#include "QuestConfig.h"
#include "MiscConfig.h"
#include "RewardConfig.h"
#include "ItemConfig.h"
#include "xServer.h"
#include "CommonConfig.h"
#include "UserConfig.h"
#include "ShopConfig.h"
#include "MenuConfig.h"
#include "AchieveConfig.h"

extern xServer* thisServer;

// config data
bool SQuestCFG::hasProfession(EProfession eProfession) const
{
  if (vecProfession.empty() == true)
    return true;

  return find(vecProfession.begin(), vecProfession.end(), eProfession) != vecProfession.end();
}

// 检测同系并大于等于配置职业
bool SQuestCFG::checkProfession(EProfession eProfession) const
{
  if (vecProfession.empty() == true)
    return true;

  EProfession eProBase = RoleConfig::getMe().getBaseProfession(eProfession);
  for(auto& v : vecProfession)
  {
    if(v == eProfession)
      return true;

    if(v == eProBase)
      return true;

    if(((DWORD)eProfession)/10 == ((DWORD)v)/10 && (DWORD)eProfession >= (DWORD)v)
      return true;
  }

  return false;
}

// 检测分支限制
bool SQuestCFG::checkBranch(DWORD dwBranch) const
{
  // 当前处于无分支状态
  if(0 == dwBranch)
    return true;

  if (vecBranch.empty())
    return true;

  return vecBranch.end() != std::find(vecBranch.begin(), vecBranch.end(), dwBranch);
}

bool SQuestCFG::checkGender(DWORD dwGender) const
{
  if(0 == gender)
    return true;

  return dwGender == gender;
}

void SQuestCFG::initPConfig(QuestPConfig& rConfig, const xLuaData& data) const
{
  rConfig.set_rewardgroup(data.getTableInt("RewardGroup"));
  rConfig.set_subgroup(data.getTableInt("SubGroup"));
  rConfig.set_finishjump(data.getTableInt("FinishJump"));
  rConfig.set_failjump(data.getTableInt("FailJump"));
  rConfig.set_map(data.getTableInt("Map"));
  rConfig.set_whethertrace(data.getTableInt("WhetherTrace"));
  rConfig.set_auto_(data.getTableInt("Auto"));
  rConfig.set_firstclass(data.getTableInt("FirstClass"));
  rConfig.set_class_(data.getTableInt("Class"));
  rConfig.set_level(data.getTableInt("Level"));
  rConfig.set_questname(data.getTableString("QuestName"));
  rConfig.set_name(data.getTableString("Name"));
  rConfig.set_type(data.getTableString("Type"));
  rConfig.set_content(data.getTableString("Content"));
  rConfig.set_traceinfo(data.getTableString("TraceInfo"));

  const xLuaData& param = data.getData("Params");// .getMutableData("Params");
  param.toData(rConfig.mutable_params());

  rConfig.clear_prequest();
  for (auto &v : vecPreQuest)
    rConfig.add_prequest(v);

  rConfig.clear_mustprequest();
  for (auto &v : vecMustPreQuest)
    rConfig.add_mustprequest(v);
}

bool SQuestCFG::toStepConfig(QuestStep* pStep, DWORD step, DWORD language) const
{
  if (pStep == nullptr || step >= vecStepData.size())
    return false;

  pStep->mutable_config()->CopyFrom(vecStepData[step].oConfig);
  pStep->mutable_config()->clear_allrewardid();
  if(language != ELANGUAGE_CHINESE_SIMPLIFIED)
  {
    pStep->mutable_config()->set_questname(CommonConfig::getMe().getTranslateString(pStep->mutable_config()->questname(), language));
    pStep->mutable_config()->set_name(CommonConfig::getMe().getTranslateString(pStep->mutable_config()->name(), language));
    pStep->mutable_config()->set_traceinfo(CommonConfig::getMe().getTranslateString(pStep->mutable_config()->traceinfo(), language));
  }
  pStep->mutable_config()->set_prefixion(CommonConfig::getMe().getTranslateString(MiscConfig::getMe().getQuestCFG().getPrefixion(dwPrefixion), language));

  TSetDWORD setRewardIDs;
  QuestConfig::getMe().collectQuestGroupReward(rewardGroupID, setRewardIDs);
  for (auto &s : setRewardIDs)
    pStep->mutable_config()->add_allrewardid(s);
  return true;
}

bool SQuestCFG::toPreview(QuestStep* pStep, DWORD step, DWORD language /*= ELANGUAGE_CHINESE_SIMPLIFIED*/) const
{
  if (pStep == nullptr || step >= vecStepData.size())
    return false;

  const QuestPConfig& rConfig = vecStepData[step].oConfig;
  QuestPConfig* pConfig = pStep->mutable_config();
  pConfig->set_level(rConfig.level());
  pConfig->set_questname(rConfig.questname());
  pConfig->set_name(rConfig.name());
  pConfig->set_type(rConfig.type());

  if(language != ELANGUAGE_CHINESE_SIMPLIFIED)
  {
    pConfig->set_questname(CommonConfig::getMe().getTranslateString(pStep->mutable_config()->questname(), language));
    pConfig->set_name(CommonConfig::getMe().getTranslateString(pStep->mutable_config()->name(), language));
    pConfig->set_traceinfo(CommonConfig::getMe().getTranslateString(pStep->mutable_config()->traceinfo(), language));
  }

  pConfig->clear_prequest();
  for (int i = 0; i < rConfig.prequest_size(); ++i)
    pConfig->add_prequest(rConfig.prequest(i));
  pConfig->clear_mustprequest();
  for (int i = 0; i < rConfig.mustprequest_size(); ++i)
    pConfig->add_mustprequest(rConfig.mustprequest(i));

  return true;
}

bool SQuestCFG::isInTime(DWORD curTime) const
{
  //if(isInSendTime(curTime) == false)
  //  return false;

  if (starttime == 0 && endtime == 0)
    return true;

  if (starttime != 0 && curTime < starttime)
    return false;
  if (endtime != 0 && curTime > endtime)
    return false;
  return true;
}

bool SQuestCFG::isInSendTime(DWORD curTime) const
{
  if(setValidWeekday.empty() == true)
    return true;

  DWORD wday = xTime::getWeek(curTime);
  auto s = setValidWeekday.find(wday);
  if(s == setValidWeekday.end())
    return false;
  if(xTime::isBetween(startsend.tm_hour, startsend.tm_min, endsend.tm_hour, endsend.tm_min, curTime) == false)
    return false;

  return true;
}

bool SQuestCFG::toData(QuestData* pData, DWORD language) const
{
  if (pData == nullptr)
    return false;
  if (pData->steps_size() != static_cast<int>(vecStepData.size()))
    return false;

  DWORD dwIndex = static_cast<int>(pData->step()) >= pData->steps_size() ? pData->step() - 1 : pData->step();
  toStepConfig(pData->mutable_steps(dwIndex), dwIndex, language);
  return true;
}

/*const SWantedItem* SWantedQuest::getWantedItem(DWORD questid) const
{
  for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
  {
    auto v = find_if(arrCurItem[i].begin(), arrCurItem[i].end(), [questid](const SWantedItem& r) -> bool{
      return r.dwQuestID == questid;
    });
    if (v != arrCurItem[i].end())
      return &(*v);
  }

  for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
  {
    auto v = find_if(arrAllItem[i].begin(), arrAllItem[i].end(), [questid](const SWantedItem& r) -> bool{
      return r.dwQuestID == questid;
    });
    if (v != arrAllItem[i].end())
      return &(*v);
  }

  return nullptr;
}*/

const SWantedQuest* SWantedQuestCFG::getWantedQuest(DWORD dwWantedID) const
{
  auto v = find_if(vecQuest.begin(), vecQuest.end(), [dwWantedID](const SWantedQuest& r) -> bool{
    return r.dwWantedID == dwWantedID;
  });
  if (v != vecQuest.end())
    return &(*v);

  return nullptr;
}

const SWantedItem* SWantedQuestCFG::getWantedItem(DWORD questid, bool bIncludeAll /*= false*/) const
{
  for (auto v = vecQuest.begin(); v != vecQuest.end(); ++v)
  {
    for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
    {
      for (auto o = v->arrCurItem[i].begin(); o != v->arrCurItem[i].end(); ++o)
      {
        if (o->dwQuestID == questid)
          return &(*o);
      }
    }
  }

  if (bIncludeAll)
  {
    for (auto v = vecQuest.begin(); v != vecQuest.end(); ++v)
    {
      for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
      {
        for (auto o = v->arrAllItem[i].begin(); o != v->arrAllItem[i].end(); ++o)
        {
          if (o->dwQuestID == questid)
            return &(*o);
        }
      }
    }
  }

  return nullptr;
}

void SQuestVersion::findBranchPreQuest(DWORD questid, DWORD id, TVecQuestCFG& vecCFG)
{
  if (questid / QUEST_ID_PARAM != id / QUEST_ID_PARAM)
    return;

  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(id);
  if (pCFG == nullptr)
    return;

  TSetDWORD setIDs;
  for (auto &v : pCFG->vecPreQuest)
    setIDs.insert(v);
  for (auto &v : pCFG->vecMustPreQuest)
    setIDs.insert(v);

  for (auto &s : setIDs)
  {
    const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(s);
    if (pCFG != nullptr)
      vecCFG.push_back(*pCFG);
  }

  for (auto &s : setIDs)
    findBranchPreQuest(questid, s, vecCFG);
}

const SQuestPuzzleCFG* SVersionPuzzleCFG::getActivePuzzleCFG(DWORD index) const
{
  auto m = mapActivePuzzleCFG.find(index);
  if (m != mapActivePuzzleCFG.end())
    return &m->second;
  return nullptr;
}

// config
QuestConfig::QuestConfig()
{

}

QuestConfig::~QuestConfig()
{
}

bool QuestConfig::loadConfig()
{
  bool bResult = true;
  bResult &= loadQuestConfig();
  bResult &= loadWantedQuestConfig();
  bResult &= loadDialogConfig();
  bResult &= loadDailyExpPool();
  bResult &= loadPuzzleConfig();
  bResult &= loadMainStoryConfig();

  return bResult;
}

bool QuestConfig::checkConfig()
{
  const SQuestMiscCFG& rCFG = MiscConfig::getMe().getQuestCFG();

  bool bCorrect = true;

  auto stepcheck = [](const SQuestCFG& rCFG, const TSetDWORD& setExtra) -> bool
  {
    for (auto v = rCFG.vecStepData.begin(); v != rCFG.vecStepData.end(); ++v)
    {
      DWORD dwFinish = v->data.getTableInt("FinishJump");
      DWORD dwFail = v->data.getTableInt("FailJump");
      TVecDWORD vecSelects{ dwFinish, dwFail };
      DWORD dwIndex = 0;
      char szTemp[64] = {0};

      do
      {
        snprintf(szTemp, 64, "client%u", dwIndex++);
        if (v->data.has(szTemp) == false)
          break;

        vecSelects.push_back(v->data.getTableInt(szTemp));
      }while (true);

      for (auto &s : setExtra)
        vecSelects.push_back(s);

      auto hasstep = [](DWORD dwStep, const SQuestCFG& rCFG) -> bool
      {
        if (dwStep == 0)
          return true;

        for (auto o = rCFG.vecStepData.begin(); o != rCFG.vecStepData.end(); ++o)
        {
          DWORD dwSubGroup = o->data.getTableInt("SubGroup");
          if (dwSubGroup == dwStep)
            return true;
        }

        return false;
      };

      for (auto select = vecSelects.begin(); select != vecSelects.end(); ++select)
      {
        if (*select == 0)
          continue;

        if (hasstep(*select, rCFG) == false)
        {
          const string& content = v->data.getTableString("Content");
          XERR << "[任务配置] id :" << rCFG.id << "step :" << content.c_str() << "step :" << *select << "在此任务中不存在" << XEND;
          return false;
        }
      }
    }

    return true;
  };

  for (auto v = m_vecQuestCFG.begin(); v != m_vecQuestCFG.end(); ++v)
  {
    for (auto &q : v->vecPreQuest)
    {
      if (getQuestCFG(q) == nullptr)
      {
        XERR << "[任务配置] id : " << v->id << " prequestid : " << q << " 未在 Table_Quest.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }
    }

    for (auto &q : v->vecMustPreQuest)
    {
      if (getQuestCFG(q) == nullptr)
      {
        XERR << "[任务配置] id : " << v->id << " mustquestid : " << q << " 未在 Table_Quest.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }
    }

    for (auto &item : v->stGuildReq.vecDatas)
    {
      if (ItemConfig::getMe().getItemCFG(item.base().id()) == nullptr)
      {
        XERR << "[任务配置] id :" << v->id << "guild->itemid : " << item.base().id() << " 未在 Table_Item.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }
    }

    TSetDWORD setExtra;
    for (auto o = v->vecStepData.begin(); o != v->vecStepData.end(); ++o)
    {
      xLuaData& param = o->data.getMutableData("Params");

      const string& content = o->data.getTableString("Content");
      if (content == "check_quest")
      {
        const TVecDWORD& vecIDs = QuestConfig::getQuestDetail(v->id / QUEST_ID_PARAM);
        if (vecIDs.empty() == true)
        {
          XERR << "[任务配置] id : " << v->id << ", step : " << content.c_str() << ", bigquestid : " << v->id / QUEST_ID_PARAM << " 不存在" << XEND;
          bCorrect = false;
          continue;
        }
      }
      else if (content == "check_group")
      {
        DWORD dwQuestID = param.getTableInt("id");
        if (getQuestCFG(dwQuestID) == nullptr)
        {
          XERR << "[任务配置] id : " << v->id << ", step : " << content.c_str() << ", questid : " << dwQuestID << " 不存在" << XEND;
          bCorrect = false;
          continue;
        }
      }
      else if (content == "cook")
      {
        auto lvf = [&](const string& key, xLuaData& data)
        {
          DWORD dwLv = atoi(key.c_str());
          const SCookerLevel* pCFG = TableManager::getMe().getCookerLevelCFG(dwLv);
          if (pCFG == nullptr)
          {
            XERR << "[任务配置] id :" << v->id << "step :" << content.c_str() << "cooklv :" << dwLv << "未在 Table_CookerLevel.txt 表中找到" << XEND;
            bCorrect = false;
          }
          setExtra.insert(data.getInt());
        };
        param.getMutableData("lv").foreach(lvf);
      }
    }

    // check step
    if (stepcheck(*v, setExtra) == false)
    {
      bCorrect = false;
      continue;
    }

    // version
    if (v->manualVersion.empty() == false)
    {
      SQuestVersion& rVersion = m_mapQuestVersion[v->manualVersion];
      rVersion.version = v->manualVersion;
      DWORD dwQuestID = v->id / QUEST_ID_PARAM;

      if (rCFG.isManualMain(v->eType) == true)
      {
        rVersion.vecMainCFG.push_back(*v);

        QuestPreview& rPreview = rVersion.mapMainCFG[dwQuestID];
        rPreview.set_questid(dwQuestID);
        rPreview.set_name(v->name);
        rPreview.set_rewardgroup(v->rewardGroupID);
      }
      else if (rCFG.isManualStory(v->eType))
      {
        rVersion.vecStoryCFG.push_back(*v);

        QuestPreview& rPreview = rVersion.mapStoryCFG[dwQuestID];
        rPreview.set_questid(dwQuestID);
        rPreview.set_name(v->name);
        rPreview.set_rewardgroup(v->rewardGroupID);

        TSetDWORD setRewardIDs;
        collectQuestGroupReward(v->rewardGroupID, setRewardIDs);
        rPreview.clear_allrewardid();
        for (auto &s : setRewardIDs)
          rPreview.add_allrewardid(s);
      }
    }
  }

  // collect version - branch shop
  const TMapShopItem& mapCFG = ShopConfig::getMe().getShopItemList();
  map<DWORD, TSetDWORD> mapItemID;
  for (auto &m : mapCFG)
  {
    const ShopItem& rItem = m.second;
    if (rItem.menuid() == 0 || rItem.itemid() == 0)
      continue;
    const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(rItem.menuid());
    if (pCFG == nullptr)
      continue;

    TSetDWORD& setIDs = mapItemID[rItem.itemid()];
    if (pCFG->stCondition.eCond == EMENUCOND_QUEST)
    {
      for (auto &v : pCFG->stCondition.vecParams)
        setIDs.insert(v);
    }
    else if (pCFG->stCondition.eCond == EMENUCOND_ACHIEVE)
    {
      for (auto &v : pCFG->stCondition.vecParams)
      {
        const SAchieveCFG* pCFG = AchieveConfig::getMe().getAchieveCFG(v);
        if (pCFG == nullptr || pCFG->stCondition.eCond != EACHIEVECOND_QUEST_SUBMIT)
          continue;
        for (auto &quest : pCFG->stCondition.vecParams)
          setIDs.insert(quest);
      }
    }
  }
  for (auto &m : mapItemID)
  {
    for (auto &s : m.second)
    {
      const SQuestCFG* pCFG = getQuestCFG(s);
      if (pCFG == nullptr || pCFG->manualVersion.empty() == true)
        continue;
      if (rCFG.isManualMain(pCFG->eType) == true || rCFG.isManualStory(pCFG->eType) == true)
        continue;

      SQuestVersion& rVersion = m_mapQuestVersion[pCFG->manualVersion];
      map<DWORD, TVecQuestCFG>& mapCFG = rVersion.mapBranchCFG[m.first];
      TVecQuestCFG& vecCFG = mapCFG[s];
      vecCFG.push_back(*pCFG);
      rVersion.findBranchPreQuest(s, s, vecCFG);
    }
  }

  // wanted quest check
  for (auto v = m_vecWantedQuestCFG.begin(); v != m_vecWantedQuestCFG.end(); ++v)
  {
    for (auto o = v->vecQuest.begin(); o != v->vecQuest.end(); ++o)
    {
      for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
      {
        for (auto k = o->arrAllItem[i].begin(); k != o->arrAllItem[i].end(); ++k)
        {
          if (RewardConfig::getMe().getRewardCFG(k->dwReward) == nullptr)
          {
            XERR << "[任务配置-看板] id : " << k->dwQuestID << ", reward : " << k->dwReward << " 未在Table_Reward.txt中找到" << XEND;
            bCorrect = false;
            continue;
          }
        }
      }
    }
  }

  // check daily exp pool
  for (auto m = m_mapDailyExpPool.begin(); m != m_mapDailyExpPool.end(); ++m)
  {
    for (auto v = m->second.vecProcessGift.begin(); v != m->second.vecProcessGift.end(); ++v)
    {
      if (ItemConfig::getMe().getItemCFG(v->oItem.id()) == nullptr)
      {
        XERR << "[任务配置-日常] lv : " << m->first << " itemid : " << v->oItem.id() << " 未在 Table_Item.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }
    }
  }

  // check puzzle
  for (auto &m : m_mapQuestPuzzleCFG)
  {
    // active
    for (auto &a : m.second.mapActivePuzzleCFG)
    {
      const SQuestPuzzleCFG& rCFG = a.second;
      for (auto &s : rCFG.setRewards)
      {
        if (RewardConfig::getMe().getRewardCFG(s) == nullptr)
        {
          XERR << "[Table_QuestPuzzle] id :" << m.first << "active indesss" << a.first << "rewardid :" << s << "未在 Table_Reward.txt 表中找到" << XEND;
          bCorrect = false;
        }
      }
    }
    // collect
    for (auto &c : m.second.mapCollectPuzzleCFG)
    {
      const SQuestPuzzleCFG& rCFG = c.second;
      for (auto &s : rCFG.setRewards)
      {
        if (RewardConfig::getMe().getRewardCFG(s) == nullptr)
        {
          XERR << "[Table_QuestPuzzle] id :" << m.first << "active indesss" << c.first << "rewardid :" << s << "未在 Table_Reward.txt 表中找到" << XEND;
          bCorrect = false;
        }
      }
    }
  }

  return bCorrect;
}

const SQuestCFG* QuestConfig::getQuestCFG(DWORD id) const
{
  auto v = find_if(m_vecQuestCFG.begin(), m_vecQuestCFG.end(), [id](const SQuestCFG& r) -> bool{
    return r.id == id;
  });
  if (v != m_vecQuestCFG.end())
    return &(*v);

  return nullptr;
}

const SWantedQuestCFG* QuestConfig::getWantedQuestCFG(DWORD lv) const
{
  auto v = find_if(m_vecWantedQuestCFG.begin(), m_vecWantedQuestCFG.end(), [lv](const SWantedQuestCFG& r) -> bool{
    return lv >= r.lvRange.first && lv <= r.lvRange.second;
  });
  if (v != m_vecWantedQuestCFG.end())
    return &(*v);

  return nullptr;
}

const SWantedQuestCFG* QuestConfig::getActivityWantedQuestCFG(DWORD lv) const
{
  auto v = find_if(m_vecActivityWantedQuestCFG.begin(), m_vecActivityWantedQuestCFG.end(), [lv](const SWantedQuestCFG& r) -> bool{
    return lv >= r.lvRange.first && lv <= r.lvRange.second;
  });
  if (v != m_vecActivityWantedQuestCFG.end())
    return &(*v);

  return nullptr;
}

void QuestConfig::debugVersion()
{
  for (auto &m : m_mapQuestVersion)
  {
    const SQuestVersion& rCFG = m.second;

    for (auto &v : rCFG.vecMainCFG)
      XDBG << "[任务配置-任务收集] version :" << rCFG.version << "主线任务 questid :" << v.id << "level :" << v.lv << "pre :" << v.vecPreQuest << "must :" << v.vecMustPreQuest << XEND;

    for (auto &b : rCFG.mapBranchCFG)
    {
      const map<DWORD, TVecQuestCFG>& mapCFG = b.second;
      for (auto &m : mapCFG)
      {
        const TVecQuestCFG& vecCFG = m.second;
        for (auto &v : vecCFG)
        {
          XDBG << "[任务配置-任务收集] version :" << rCFG.version << "支线任务 itemid :" << b.first
            << "解锁任务 :" << m.first << "包含 questid :" << v.id << "level :" << v.lv << "pre :" << v.vecPreQuest << "must :" << v.vecMustPreQuest << XEND;
        }
      }
    }

    for (auto &m : rCFG.mapMainCFG)
    {
      XDBG << "[任务配置-任务收集] version :" << rCFG.version << "主线预览 questid :" << m.first << "name :" << m.second.name() << "rewardids :";
      for (int i = 0; i < m.second.allrewardid_size(); ++i)
        XDBG << m.second.allrewardid(i);
      XDBG << XEND;
    }

    for (auto &m : rCFG.mapStoryCFG)
    {
      XDBG << "[任务配置-任务收集] version :" << rCFG.version << "诗人预览 questid :" << m.first << "name :" << m.second.name() << "rewardids :";
      for (int i = 0; i < m.second.allrewardid_size(); ++i)
        XDBG << m.second.allrewardid(i);
      XDBG << XEND;
    }
  }
}

const SDailyExpPool* QuestConfig::getExpPool(DWORD lv) const
{
  auto m = m_mapDailyExpPool.find(lv);
  if (m != m_mapDailyExpPool.end())
    return &m->second;

  return nullptr;
}

void QuestConfig::randomWantedQuest(bool isIgnoreTime/*=false*/)
{
  DWORD dwNow = xTime::getCurSec();
  if (!isIgnoreTime)
  {
    if (dwNow < m_dwNextTime)
      return;
  }

  m_dwWantedTime = dwNow;
  const SQuestMiscCFG& rCFG = MiscConfig::getMe().getQuestCFG();
  if (m_dwRefreshIndex >= rCFG.vecWantedRefresh.size())
  {
    m_dwRefreshIndex = 0;
    dwNow += 86400;
  }
  DWORD dwTime = rCFG.vecWantedRefresh.empty() == true ? 0 : rCFG.vecWantedRefresh[m_dwRefreshIndex++];
  m_dwNextTime = xTime::getDayStart(dwNow) + dwTime;

  XLOG << "[任务配置-看板-列表刷新] 在" << dwNow << "进行了列表重置刷新" << XEND;
  srand(m_dwNextTime * ONE_THOUSAND);

  DWORD arrActiveCount[EWANTEDTYPE_MAX] = {0};
  DWORD dwMaxCount = m_dwWantedActiveCount == 0 ? rCFG.getMaxWantedCount(EWANTEDTYPE_TOTAL) : m_dwWantedActiveCount;
  for (int i = EWANTEDTYPE_TOTAL + 1; i < EWANTEDTYPE_MAX; ++i)
    arrActiveCount[i] = randBetween(0, rCFG.getMaxWantedCount(static_cast<EWantedType>(i)));
  for (int i = EWANTEDTYPE_TOTAL + 1; i < EWANTEDTYPE_MAX; ++i)
    dwMaxCount -= arrActiveCount[i];
  arrActiveCount[EWANTEDTYPE_TOTAL] = dwMaxCount;

  for (auto v = m_vecWantedQuestCFG.begin(); v != m_vecWantedQuestCFG.end(); ++v)
  {
    for (auto o = v->vecQuest.begin(); o != v->vecQuest.end(); ++o)
    {
      for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
      {
        if (o->arrAllItem[i].empty() == true)
        {
          XERR << "[任务配置-看板-列表刷新] 刷新minlv :" << v->lvRange.first << "maxlv :" << v->lvRange.second << "wantid :" << o->dwWantedID << "失败, 类型 :" << i << "未含有任何任务" << XEND;
          continue;
        }
        o->arrCurItem[i].clear();
        TVecWantedItem vecAllItem = o->arrAllItem[i];
        DWORD dwIndex = 0;
        while (true)
        {
          if (vecAllItem.empty() == true)
            break;
          if (o->arrCurItem[i].size() >= arrActiveCount[i])
            break;

          DWORD dwRand = randBetween(0, vecAllItem.size() - 1);
          XLOG << "[任务配置-看板-列表刷新] 刷新 minlv :" << v->lvRange.first << "maxlv :" << v->lvRange.second << "wantid :" << o->dwWantedID
            << "随机一个 id :" << vecAllItem[dwRand].dwQuestID << "类型 :" << i << "任务" << XEND;

          srand(m_dwNextTime * ONE_THOUSAND + dwIndex++);
          RewardConfig::getMe().roll(vecAllItem[dwRand].dwReward, RewardEntry(), vecAllItem[dwRand].vecRewards, ESOURCE_QUEST);
          srand(m_dwNextTime * ONE_THOUSAND);
          for (auto &item : vecAllItem[dwRand].vecRewards)
            item.set_source(ESOURCE_WANTEDQUEST);
          o->arrCurItem[i].push_back(vecAllItem[dwRand]);
          vecAllItem.erase(vecAllItem.begin() + dwRand);
        }
      }
    }
  }

  srand(xTime::getCurUSec());

  // verify reward if have replace
  if (strncmp(thisServer->getServerName(), "SceneServer", 11) == 0)
  {
    XLOG << "[任务配置-relpace]" << XEND;
    if (!m_mapIndex2RepRewards.empty())
    {
      for (auto &m : m_mapIndex2RepRewards)
      {
        addReplace(m.first, m.second);
      }
    }
  }
}

bool QuestConfig::randomMapRandQuest()
{
  DWORD dwNow = xTime::getCurSec();
  if (dwNow < m_dwNextMapRandTime)
    return false;

  m_dwNextMapRandTime = xTime::getDayStart(dwNow) + DAY_T + 5 * 3600;

  XLOG << "[任务配置-地图随机-列表刷新] 在" << dwNow << "进行了列表重置刷新" << XEND;
  srand(m_dwNextMapRandTime * ONE_THOUSAND);

  for (auto &m : m_mapMapRandCFG)
  {
    const SDailyPerDay* pCFG = MiscConfig::getMe().getQuestCFG().getDailyCount(m.first);
    if (pCFG == nullptr)
    {
      XERR << "[任务配置-地图随机-列表刷新] mapid :" << m.first << "随机失败,未找到ServerGame_Quest -> dailyrand_per_day配置";
      continue;
    }

    m.second.vecCurCFG.clear();
    TVecQuestCFG vecCFG = m.second.vecAllCFG;

    if (vecCFG.size() < pCFG->dwAcceptCount)
    {
      m.second.vecCurCFG.insert(m.second.vecCurCFG.end(), m.second.vecAllCFG.begin(), m.second.vecAllCFG.end());
      continue;
    }

    std::random_shuffle(vecCFG.begin(), vecCFG.end());
    for (DWORD d = 0; d < pCFG->dwAcceptCount; ++d)
      m.second.vecCurCFG.push_back(vecCFG[d]);

    XLOG << "[任务配置-地图随机-列表刷新] mapid :" << m.first << "随机出任务 :";
    for (auto &v : m.second.vecCurCFG)
      XLOG << v.id;
    XLOG << XEND;
  }

  srand(xTime::getCurUSec());
  return true;
}

void QuestConfig::setWantedActive(bool bActive, DWORD dwCount)
{
  m_bWantedActiveEnable = bActive;
  m_dwWantedActiveCount = dwCount;
  XLOG << "[任务配置-看板设置] active :" << m_bWantedActiveEnable << "maxcount :" << m_dwWantedActiveCount << XEND;
}

const TVecDWORD& QuestConfig::getQuestDetail(DWORD id) const
{
  auto v = find_if(m_vecQuestDetailCFG.begin(), m_vecQuestDetailCFG.end(), [id](const TPairQuestDetail& r) -> bool{
    return r.first == id;
  });
  if (v != m_vecQuestDetailCFG.end())
    return v->second;

  static const TVecDWORD e;
  return e;
}

const TVecQuestCFG* QuestConfig::getTypeQuestCFGList(EQuestType eType) const
{
  auto m = m_mapTypeQuestCFG.find(eType);
  if (m != m_mapTypeQuestCFG.end())
    return &m->second;
  return nullptr;
}

const TVecQuestCFG* QuestConfig::getCondQuestCFGList(EQuestCond eCond) const
{
  auto m = m_mapCondQuestCFG.find(eCond);
  if (m != m_mapCondQuestCFG.end())
    return &m->second;
  return nullptr;
}

bool QuestConfig::loadQuestConfig()
{
  bool bCorrect = true;

  m_vecQuestCFG.clear();
  m_vecQuestDetailCFG.clear();
  const SQuestTableCFG& rCFG = MiscConfig::getMe().getQuestTableCFG();
  for (auto v = rCFG.vecTables.begin(); v != rCFG.vecTables.end(); ++v)
  {
    if (loadQuestTable(*v) == false)
    {
      bCorrect = false;
      XERR << "[任务配置] 加载配置" << (*v).c_str() << ".txt失败" << XEND;
    }
  }

  // init other
  m_mapMapRandCFG.clear();
  m_mapTypeQuestCFG.clear();
  m_mapCondQuestCFG.clear();
  m_mapQuestVersion.clear();
  for (auto &v : m_vecQuestCFG)
  {
    // reward
    for (auto &step : v.vecStepData)
    {
      const string& content = step.data.getTableString("Content");
      if (content == "reward")
        m_mapGroupRewards[v.rewardGroupID].insert(step.data.getMutableData("Params").getTableInt("id"));
    }

    // maprand
    if (v.eType == EQUESTTYPE_DAILY_MAPRAND)
      m_mapMapRandCFG[v.mapid].vecAllCFG.push_back(v);

    m_mapTypeQuestCFG[v.eType].push_back(v);

    // manual
    if (v.stManualReq.eType != EMANUALTYPE_MIN)
      m_mapCondQuestCFG[EQUESTCOND_MANUAL].push_back(v);

    // pre
    for (auto &pre : v.vecPreQuest)
    {
      SQuestPreQuest& rCFG = m_mapPreQuest[pre];
      rCFG.mapTypeQuest[v.eType].push_back(v);
    }
  }

  debugVersion();
  return bCorrect;
}

bool QuestConfig::loadQuestTable(const string& str)
{
  bool bCorrect = true;
  DWORD dwSvrBranch = BaseConfig::getMe().getInnerBranch();

  // get path
  string path = "Lua/Table/";
  path = path + str;
  path = path + ".txt";

  // load quest config
  if (!xLuaTable::getMe().open(path.c_str()))
  {
    XERR << "[任务配置] 加载配置" << str.c_str() << "\b.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable(str.c_str(), table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get quest id
    DWORD questid = m->second.getTableInt("QuestID") * QUEST_ID_PARAM + m->second.getTableInt("GroupID");

    if (MiscConfig::getMe().isForbid("Quest", questid))
      continue;

    // get quest config
    auto v = find_if(m_vecQuestCFG.begin(), m_vecQuestCFG.end(), [questid](const SQuestCFG& cfg){
      return questid == cfg.id;
    });
    if (v == m_vecQuestCFG.end())
    {
      // create config
      SQuestCFG stCFG;

      stCFG.id = questid;
      stCFG.lv = m->second.getTableInt("Level");
      stCFG.joblv = m->second.getTableInt("Joblevel");
      stCFG.manuallv = m->second.getTableInt("Risklevel");
      stCFG.mapid = m->second.getTableInt("Map");
      stCFG.rewardGroupID = m->second.getTableInt("RewardGroupID");
      stCFG.cookerLv = m->second.getTableInt("CookerLv");
      stCFG.tasterLv = m->second.getTableInt("TasterLv");
      stCFG.gender = m->second.getTableInt("Gender");
      stCFG.toBranch = m->second.getTableInt("ToBranch");
      stCFG.dwPrefixion = m->second.getTableInt("Prefixion");
      stCFG.name = m->second.getTableString("Name");
      stCFG.questname = m->second.getTableString("QuestName");
      stCFG.manualVersion = m->second.getTableString("version");

      const string& startparam = dwSvrBranch == BRANCH_TF ? "TFStartTime" : "StartTime";
      const string& starttime = m->second.getTableString(startparam);
      if (starttime.empty() == false)
        parseTime(starttime.c_str(), stCFG.starttime);
      const string& endparam = dwSvrBranch == BRANCH_TF ? "TFEndTime" : "EndTime";
      const string& endtime = m->second.getTableString(endparam);
      if (endtime.empty() == false)
        parseTime(endtime.c_str(), stCFG.endtime);

      xLuaData& weekdata = m->second.getMutableData("WeekTime");
      auto weekfunc = [&](const string& str, xLuaData& data)
      {
        stCFG.setValidWeekday.insert(data.getInt());
      };
      weekdata.getMutableData("weekday").foreach(weekfunc);
      stCFG.startsend.tm_hour = weekdata.getMutableData("starttime").getTableInt("hour");
      stCFG.startsend.tm_min = weekdata.getMutableData("starttime").getTableInt("min");
      stCFG.endsend.tm_hour = weekdata.getMutableData("endtime").getTableInt("hour");
      stCFG.endsend.tm_min = weekdata.getMutableData("endtime").getTableInt("min");

      xLuaData& guild = m->second.getMutableData("guild");
      DWORD dwType = guild.getTableInt("building");
      if (dwType != 0)
      {
        if (dwType <= EGUILDBUILDING_MIN || dwType >= EGUILDBUILDING_MAX || EGuildBuilding_IsValid(dwType) == false)
        {
          XERR << "[任务配置] questid :" << stCFG.id << "guild -> type :" << dwType << "不合法" << XEND;
          bCorrect = false;
          continue;
        }
        stCFG.stGuildReq.eType = static_cast<EGuildBuilding>(dwType);
        stCFG.stGuildReq.dwAuth = guild.getTableInt("auth");
        auto guildf = [&](const string& key, xLuaData& data)
        {
          ItemData oData;
          oData.mutable_base()->set_id(data.getTableInt("1"));
          oData.mutable_base()->set_count(data.getTableInt("2"));
          combineItemData(stCFG.stGuildReq.vecDatas, oData);
          m_setArtifactPiece.insert(oData.base().id());
        };
        guild.getMutableData("itemid").foreach(guildf);
      }

      xLuaData& manual = m->second.getMutableData("Manual");
      DWORD dwManualType = manual.getTableInt("type");
      if (dwManualType != 0)
      {
        if (dwManualType <= EMANUALTYPE_MIN || dwManualType >= EMANUALTYPE_MAX || EManualType_IsValid(dwManualType) == false)
        {
          XERR << "[任务配置] questid :" << stCFG.id << "manual -> type" << dwManualType << "不合法" << XEND;
          bCorrect = false;
          continue;
        }
        DWORD dwManualStatus = manual.getTableInt("status");
        if (dwManualStatus <= EMANUALSTATUS_MIN || dwManualStatus >= EMANUALSTATUS_MAX || EManualStatus_IsValid(dwManualStatus) == false)
        {
          XERR << "[任务配置] questid :" << stCFG.id << "manual -> status" << dwManualStatus << "不合法" << XEND;
          bCorrect = false;
          continue;
        }
        stCFG.stManualReq.eType = static_cast<EManualType>(dwManualType);
        stCFG.stManualReq.eStatus = static_cast<EManualStatus>(dwManualStatus);
      }

      auto manual_idf = [&](const string& key, xLuaData& data)
      {
        stCFG.stManualReq.setIDs.insert(data.getInt());
      };
      manual.getMutableData("id").foreach(manual_idf);

      const string& type = m->second.getTableString("Type");
      stCFG.eType = getQuestType(type);
      if (stCFG.eType == EQUESTTYPE_MIN)
      {
        XERR << "[任务配置] questid :" << stCFG.id << "type :" << type.c_str() << "不合法" << XEND;
        bCorrect = false;
        continue;
      }

      DWORD pro = m->second.getTableInt("FirstClass");
      if (EProfession_IsValid(pro) == false)
      {
        XERR << "[任务配置] questid :" << stCFG.id << "FirstClass :" << pro << "不合法" << XEND;
        bCorrect = false;
        continue;
      }
      stCFG.eDestProfession = static_cast<EProfession>(pro);

      xLuaData& profession = m->second.getMutableData("Class");
      auto professionf = [&](const string& str, xLuaData& data)
      {
        stCFG.vecProfession.push_back(static_cast<EProfession>(data.getInt()));
      };
      profession.foreach(professionf);

      xLuaData& branch = m->second.getMutableData("Branch");
      auto branchf = [&](const string& str, xLuaData& data)
      {
        stCFG.vecBranch.push_back(data.getInt());
      };
      branch.foreach(branchf);

      xLuaData& pre = m->second.getMutableData("PreQuest");
      auto preFunc = [&stCFG](const string& key, xLuaData& data)
      {
        stCFG.vecPreQuest.push_back(data.getInt());
      };
      pre.foreach(preFunc);
      for (auto v = stCFG.vecProfession.begin(); v != stCFG.vecProfession.end(); ++v)
      {
        if (EProfession_IsValid(*v) == false || *v <= EPROFESSION_MIN || *v >= EPROFESSION_MAX)
        {
          XERR << "[任务配置] questid :" << stCFG.id << "Class :" << static_cast<DWORD>(*v) << "不合法" << XEND;
          bCorrect = false;
          continue;
        }
      }

      xLuaData& mustpre = m->second.getMutableData("MustPreQuest");
      auto mustpreFunc = [&stCFG](const string& key, xLuaData& data)
      {
        stCFG.vecMustPreQuest.push_back(data.getInt());
      };
      mustpre.foreach(mustpreFunc);

      xLuaData& activityrefresh = m->second.getMutableData("recycle");
      stCFG.eRefreshType = static_cast<EQuestRefresh>(activityrefresh.getTableInt("timetype"));
      stCFG.refreshTimes = activityrefresh.getTableInt("resettimes");

      // insert and rinit config
      m_vecQuestCFG.push_back(stCFG);
      v = find_if(m_vecQuestCFG.begin(), m_vecQuestCFG.end(), [questid](const SQuestCFG& cfg){
        return questid == cfg.id;
      });
      if (v == m_vecQuestCFG.end())
      {
        XERR << "[任务配置] questid : " << stCFG.id << " 添加失败" << XEND;
        bCorrect = false;
        continue;
      }
    }

    // insert lua data
    SQuestStepCFG stStepCFG;
    stStepCFG.data = m->second;
    v->initPConfig(stStepCFG.oConfig, stStepCFG.data);
    v->vecStepData.push_back(stStepCFG);

    // create detail
    DWORD id = m->second.getTableInt("QuestID");
    auto o = find_if(m_vecQuestDetailCFG.begin(), m_vecQuestDetailCFG.end(), [id](const TPairQuestDetail& r) -> bool{
        return id == r.first;
    });
    if (o == m_vecQuestDetailCFG.end())
    {
      TPairQuestDetail pairDetail;
      pairDetail.first = id;
      pairDetail.second.push_back(questid);
      m_vecQuestDetailCFG.push_back(pairDetail);
    }
    else
    {
      auto s = find(o->second.begin(), o->second.end(), questid);
      if (s == o->second.end())
        o->second.push_back(questid);
    }
  }

  if (bCorrect)
    XLOG << "[任务配置] 成功加载配置" << str.c_str() << ".txt" << XEND;
  return bCorrect;
}

bool QuestConfig::loadWantedQuestConfig()
{
  bool bCorrect = true;

  // load quest config
  if (!xLuaTable::getMe().open("Lua/Table/Table_WantedQuest.txt"))
  {
    XERR << "[任务配置-看板] 加载配置Table_WantedQuest.txt失败" << XEND;
    return false;
  }

  auto loadfunc = [&] (TVecWantedQuestCFG &m_vecCFG, xLuaData wantedData, DWORD questID, DWORD minlv, DWORD maxlv, DWORD dwType) -> bool
  {
    auto o = find_if(m_vecCFG.begin(), m_vecCFG.end(), [minlv, maxlv](const SWantedQuestCFG& r) -> bool{
      return r.lvRange.first == minlv && r.lvRange.second == maxlv;
    });
    if (o == m_vecCFG.end())
    {
      SWantedQuestCFG stCFG;
      stCFG.lvRange.first = minlv;
      stCFG.lvRange.second = maxlv;
      m_vecCFG.push_back(stCFG);
      o = find_if(m_vecCFG.begin(), m_vecCFG.end(), [minlv, maxlv](const SWantedQuestCFG& r) -> bool{
        return r.lvRange.first == minlv && r.lvRange.second == maxlv;
      });
    }
    if (o == m_vecCFG.end())
    {
      XERR << "[任务配置-看板] minlv :" << minlv << "maxlv :" << maxlv << "未在Table_WantedQuest.txt表中找到" << XEND;
      return false;
    }

    DWORD dwWantedID = wantedData.getTableInt("WantedId");
    auto v = find_if(o->vecQuest.begin(), o->vecQuest.end(), [dwWantedID](const SWantedQuest& r) -> bool{
      return r.dwWantedID == dwWantedID;
    });
    if (v == o->vecQuest.end())
    {
      SWantedQuest stQuest;
      stQuest.dwWantedID = dwWantedID;
      o->vecQuest.push_back(stQuest);

      v = find_if(o->vecQuest.begin(), o->vecQuest.end(), [dwWantedID](const SWantedQuest& r) -> bool{
        return r.dwWantedID == dwWantedID;
      });
    }
    if (v == o->vecQuest.end())
    {
      XERR << "[任务配置-看板] minlv :" << minlv << "maxlv :" << maxlv << "wantedid :" << dwWantedID << "添加失败" << XEND;
      return false;
    }

    SWantedItem stItem;
    stItem.dwQuestID = questID;
    stItem.dwBaseExp = wantedData.getTableInt("BaseExp");
    stItem.dwJobExp = wantedData.getTableInt("JobExp");
    stItem.dwGold = wantedData.getTableInt("Gold");
    stItem.dwRob = wantedData.getTableInt("Rob");
    stItem.dwReward = wantedData.getTableInt("Reward");
    stItem.eType = static_cast<EWantedType>(dwType);
    stItem.bTeamSync = wantedData.getTableInt("TeamSync") == 1;
    stItem.strName = wantedData.getTableString("Name");
    stItem.bActivity = wantedData.getTableInt("IsActivity") == 1;
    if(stItem.bActivity == true)
      RewardConfig::getMe().roll(stItem.dwReward, RewardEntry(), stItem.vecRewards, ESOURCE_QUEST);

    v->arrAllItem[stItem.eType].push_back(stItem);
    return true;
  };

  m_vecWantedQuestCFG.clear();
  m_vecActivityWantedQuestCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_WantedQuest", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwType = m->second.getTableInt("Type");
    DWORD minlv = m->second.getMutableData("LevelRange").getTableInt("1");
    DWORD maxlv = m->second.getMutableData("LevelRange").getTableInt("2");
    if (dwType >= EWANTEDTYPE_MAX || EWantedType_IsValid(dwType) == false)
    {
      XERR << "[任务配置-看板] minlv :" << minlv << "maxlv :" << maxlv << "Type :" << dwType << "类型不合法" << XEND;
      bCorrect = false;
      continue;
    }
    bool bActivity = m->second.getTableInt("IsActivity") == 1;
    if(bActivity == false)
      loadfunc(m_vecWantedQuestCFG, m->second, m->first, minlv, maxlv, dwType);
    else
      loadfunc(m_vecActivityWantedQuestCFG, m->second, m->first, minlv, maxlv, dwType);

    //XDBG << "[任务配置-看板] minlv :" << minlv << "maxlv :" << maxlv << "wantedid :" << dwWantedID << "添加 questid :" << stItem.dwQuestID << "Type :" << stItem.eType << XEND;
  }

  if (bCorrect)
    XLOG << "[任务配置-看板] 成功加载配置Table_WantedQuest.txt" << XEND;
  return bCorrect;
}

bool QuestConfig::loadDialogConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Dialog.txt"))
  {
    XERR << "[任务配置-对话] 加载配置Table_Dialog.txt失败" << XEND;
    return false;
  }

  map<DWORD, TVecDWORD> mapID2Selection;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Dialog", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto n = mapID2Selection.find(m->first);
    if (n != mapID2Selection.end())
    {
      XERR << "[任务配置-对话] id : " << m->first << " 重复了" << XEND;
      bCorrect = false;
      return false;
    }

    DWORD dwFirst = 0;
    DWORD dwSecond = 0;
    const string& option = m->second.getTableString("Option");
    for (size_t i = 0; i < option.size(); ++i)
    {
      if (option[i] == ',')
        dwFirst = i;
      else if (option[i] == '}')
        dwSecond = i;

      if (dwFirst != 0 && dwSecond != 0 && dwSecond > dwFirst)
      {
        string subString = option.substr(dwFirst + 1, dwSecond - dwFirst - 1);
        mapID2Selection[m->first].push_back(atoi(subString.c_str()));
        dwFirst = dwSecond = 0;
      }
    }
  }

  // check and init client selects
  for (auto v = m_vecQuestCFG.begin(); v != m_vecQuestCFG.end(); ++v)
  {
    for (auto o = v->vecStepData.begin(); o != v->vecStepData.end(); ++o)
    {
      const string& content = o->data.getTableString("Content");
      if (content == "visit" || content == "dialog")
      {
        xLuaData& param = o->data.getMutableData("Params");
        xLuaData& dialog = param.getMutableData("dialog");
        auto dialogf = [&](const string& str, xLuaData& data)
        {
          DWORD dwDialog = data.getInt();
          auto m = mapID2Selection.find(dwDialog);
          if (m != mapID2Selection.end())
          {
            DWORD dwIndex = 0;
            char szTemp[64] = {0};
            for (auto select = m->second.begin(); select != m->second.end(); ++select)
            {
              snprintf(szTemp, 64, "client%u", dwIndex++);
              o->data.setData(szTemp, *select);
            }
          }
        };
        dialog.foreach(dialogf);
      }
    }
  }

  if (bCorrect)
    XLOG << "[任务配置-对话] 成功加载配置Table_Dialog.txt" << XEND;
  return bCorrect;
}

bool QuestConfig::loadDailyExpPool()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_ExpPool.txt"))
  {
    XERR << "[任务配置-经验池] 加载配置Table_ExpPool.txt失败" << XEND;
    return false;
  }

  m_mapDailyExpPool.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ExpPool", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto o = m_mapDailyExpPool.find(m->first);
    if (o != m_mapDailyExpPool.end())
    {
      XERR << "[任务配置-经验池] level : " << m->first << " 重复了" << XEND;
      bCorrect = false;
      continue;
    }

    SDailyExpPool& rItem = m_mapDailyExpPool[m->first];

    rItem.dwLevel = m->first;
    rItem.dwExp = m->second.getTableInt("Exp");
    rItem.dwSingleCost = m->second.getTableInt("Cost");
    rItem.dwSubmitBaseExp = m->second.getTableInt("QuestBaseExp");
    rItem.dwSubmitJobExp = m->second.getTableInt("QuestJobExp");
    rItem.dwWantedBaseExp = m->second.getTableInt("WantedBaseExp");
    rItem.dwWantedJobExp = m->second.getTableInt("WantedJobExp");
    rItem.dwWantedRob = m->second.getTableInt("Rob");

    xLuaData& item = m->second.getMutableData("ItemID");
    auto itemf = [&](const string& str, xLuaData& data)
    {
      SDailyGift stGift;
      stGift.oItem.set_id(data.getInt());
      stGift.oItem.set_count(1);
      stGift.oItem.set_source(ESOURCE_DAILYQUEST);
      rItem.vecProcessGift.push_back(stGift);
    };
    item.foreach(itemf);
    float f = 1.0f / rItem.vecProcessGift.size();
    for (size_t i = 0; i < rItem.vecProcessGift.size(); ++i)
      rItem.vecProcessGift[i].fRate = f * (i + 1);
  }

  if (bCorrect)
    XLOG << "[任务配置-经验池] 成功加载配置Table_ExpPool.txt" << XEND;
  return bCorrect;
}

bool QuestConfig::loadPuzzleConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_QuestPuzzle.txt"))
  {
    XERR << "[Table_QuestPuzzle] 加载配置 Table_QuestPuzzle.txt 失败" << XEND;
    return false;
  }

  m_mapQuestPuzzleCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_QuestPuzzle", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    const string& version = m->second.getTableString("version");
    const string& type = m->second.getTableString("type");

    SVersionPuzzleCFG& rCFG = m_mapQuestPuzzleCFG[version];
    DWORD dwIndex = m->second.getTableInt("indexss");
    SQuestPuzzleCFG* pPuzzle = nullptr;

    if (type == "active")
    {
      auto index = rCFG.mapActivePuzzleCFG.find(dwIndex);
      if (index != rCFG.mapActivePuzzleCFG.end())
      {
        bCorrect = false;
        XERR << "[Table_QuestPuzzle] version :" << version << "index :" << dwIndex << "重复了" << XEND;
        continue;
      }

      pPuzzle = &rCFG.mapActivePuzzleCFG[dwIndex];
      pPuzzle->dwIndex = dwIndex;

      TVecQuestCFG& vecQuest = pPuzzle->vecQuestCFG;
      auto questf = [&](const string& key, xLuaData& data)
      {
        const SQuestCFG* pCFG = getQuestCFG(data.getInt());
        if (pCFG == nullptr)
        {
          bCorrect = false;
          XERR << "[Table_QuestPuzzle] version :" << version << "index :" << dwIndex << "questid :" << data.getInt() << "未在 Table_Quest.txt 表中找到" << XEND;
          return;
        }
        vecQuest.push_back(*pCFG);
      };
      m->second.getMutableData("QuestIDs").foreach(questf);
    }
    else if (type == "collect")
    {
      auto index = rCFG.mapCollectPuzzleCFG.find(dwIndex);
      if (index != rCFG.mapCollectPuzzleCFG.end())
      {
        bCorrect = false;
        XERR << "[Table_QuestPuzzle] version :" << version << "index :" << dwIndex << "重复了" << XEND;
        continue;
      }

      pPuzzle = &rCFG.mapCollectPuzzleCFG[dwIndex];
      pPuzzle->dwIndex = dwIndex;
    }
    else
    {
      bCorrect = false;
      XERR << "[Table_QuestPuzzle] version :" << version << "index :" << dwIndex << "type :" << type << "不合法" << XEND;
      continue;
    }

    if (pPuzzle == nullptr)
    {
      bCorrect = false;
      XERR << "[Table_QuestPuzzle] version :" << version << "index :" << dwIndex << "type :" << type << "异常" << XEND;
      continue;
    }

    pPuzzle->setRewards.clear();
    auto rewardf = [&](const string& key, xLuaData& data)
    {
      pPuzzle->setRewards.insert(data.getInt());
    };
    m->second.getMutableData("reward").foreach(rewardf);
  }

  if (bCorrect)
    XLOG << "[Table_QuestPuzzle] 成功加载配置Table_QuestPuzzle.txt" << XEND;
  return bCorrect;
}

bool QuestConfig::loadMainStoryConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_MainStory.txt"))
  {
    XERR << "[Table_MainStory] 加载配置 Table_MainStory.txt 失败" << XEND;
    return false;
  }

  m_mapMainStoryCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_MainStory", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto item = m_mapMainStoryCFG.find(m->first);
    if (item != m_mapMainStoryCFG.end())
    {
      bCorrect = false;
      XERR << "[Table_MainStory] id :" << m->first << "重复了" << XEND;
      continue;
    }

    SQuestMainStoryCFG& rCFG = m_mapMainStoryCFG[m->first];
    rCFG.dwID = m->first;

    auto questf = [&](const string& key, xLuaData& data)
    {
      DWORD dwQuestID = data.getInt();
      if (getQuestCFG(dwQuestID) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_MainStory] id :" << m->first << "questid :" << dwQuestID << "未在 Table_Quest.txt 表中找到" << XEND;
        return;
      }
      rCFG.setQuestIDs.insert(dwQuestID);
    };
    m->second.getMutableData("QuestID").foreach(questf);
  }

  if (bCorrect)
    XLOG << "[Table_MainStory] 成功加载配置 Table_MainStory.txt" << XEND;
  return bCorrect;
}

bool QuestConfig::addReplace(DWORD index, const TSetDWORD& setRwds)
{
  // 生成reward
  TVecItemInfo vecItem;
  for (auto &s : setRwds)
    RewardConfig::getMe().roll(s, RewardEntry(), vecItem, ESOURCE_QUEST);

  for (auto &v : m_vecWantedQuestCFG)
  {
    DWORD tmpindex = 0;
    bool tmpfind = false;
    for (auto &v1 : v.vecQuest)
    {
      if (tmpfind) break;
      for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
      {
        if (tmpfind) break;
        for (auto &v2 : v1.arrCurItem[i])
        {
          tmpindex ++;
          if (tmpindex == index)
          {
            v2.vecRewards.clear();
            v2.vecRewards.assign(vecItem.begin(), vecItem.end());
            tmpfind = true;
            break;
          }
        }
      }
    }
  }

  TSetDWORD copySet;
  copySet.insert(setRwds.begin(), setRwds.end());

  TSetDWORD& rset = m_mapIndex2RepRewards[index];
  rset.clear();
  rset.insert(copySet.begin(), copySet.end());
  XLOG << "[看板任务-reward替换], 添加替换, index:" << index << "新奖励ID:";
  for (auto &s :copySet)
    XLOG << s;
  XLOG << XEND;

  return true;
}

bool QuestConfig::delReplace(DWORD index)
{
  auto it = m_mapIndex2RepRewards.find(index);
  if (it == m_mapIndex2RepRewards.end())
  {
    XERR << "[看板任务-reward替换], 找不到第" << index << "个index的替换" << XEND;
    return false;
  }
  XLOG << "[看板任务-reward替换], 删除替换, index:" << index << "奖励ID:";
  for (auto &s : it->second)
    XLOG << s;
  XLOG << XEND;
  m_mapIndex2RepRewards.erase(it);

  for (auto &v : m_vecWantedQuestCFG)
  {
    DWORD tmpindex = 0;
    bool tmpfind = false;
    for (auto &v1 : v.vecQuest)
    {
      if (tmpfind) break;
      for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
      {
        if (tmpfind) break;
        for (auto &v2 : v1.arrCurItem[i])
        {
          tmpindex ++;
          if (tmpindex == index)
          {
            v2.vecRewards.clear();
            RewardConfig::getMe().roll(v2.dwReward, RewardEntry(), v2.vecRewards, ESOURCE_QUEST);
            tmpfind = true;
            break;
          }
        }
      }
    }
  }
  return true;
}

EQuestType QuestConfig::getQuestType(const string& type) const
{
  if (type == "main")
    return EQUESTTYPE_MAIN;
  else if (type == "branch")
    return EQUESTTYPE_BRANCH;
  else if (type == "talk")
    return EQUESTTYPE_TALK;
  else if (type == "trigger")
    return EQUESTTYPE_TRIGGER;
  else if (type == "wanted")
    return EQUESTTYPE_WANTED;
  else if (type == "daily")
    return EQUESTTYPE_DAILY;
  else if (type == "story")
    return EQUESTTYPE_STORY;
  else if (type == "daily_1")
    return EQUESTTYPE_DAILY_1;
  else if (type == "daily_3")
    return EQUESTTYPE_DAILY_3;
  else if (type == "daily_7")
    return EQUESTTYPE_DAILY_7;
  else if (type == "daily_map")
    return EQUESTTYPE_DAILY_MAP;
  else if (type == "scene")
    return EQUESTTYPE_SCENE;
  else if (type == "head")
    return EQUESTTYPE_HEAD;
  else if (type == "raid_talk")
    return EQUESTTYPE_RAIDTALK;
  else if (type == "satisfaction")
    return EQUESTTYPE_SATISFACTION;
  else if (type == "elite")
    return EQUESTTYPE_ELITE;
  else if (type == "ccrasteham")
    return EQUESTTYPE_CCRASTEHAM;
  else if (type == "story_ccrasteham")
    return EQUESTTYPE_STORY_CCRASTEHAM;
  else if (type == "guild")
    return EQUESTTYPE_GUILD;
  else if (type == "child")
    return EQUESTTYPE_CHILD;
  else if (type == "daily_reset")
    return EQUESTTYPE_DAILY_RESET;
  else if (type == "acc")
    return EQUESTTYPE_ACC;
  else if (type == "acc_normal")
    return EQUESTTYPE_ACC_NORMAL;
  else if (type == "acc_daily")
    return EQUESTTYPE_ACC_DAILY;
  else if (type == "acc_choice")
    return EQUESTTYPE_ACC_CHOICE;
  else if (type == "acc_main")
    return EQUESTTYPE_ACC_MAIN;
  else if (type == "acc_branch")
    return EQUESTTYPE_ACC_BRANCH;
  else if (type == "acc_satisfaction")
    return EQUESTTYPE_ACC_SATISFACTION;
  else if (type == "acc_daily_1")
    return EQUESTTYPE_ACC_DAILY_1;
  else if (type == "acc_daily_3")
    return EQUESTTYPE_ACC_DAILY_3;
  else if (type == "acc_daily_7")
    return EQUESTTYPE_DAILY_7;
  else if (type == "acc_reset")
    return EQUESTTYPE_ACC_DAILY_RESET;
  else if (type == "daily_maprand")
    return EQUESTTYPE_DAILY_MAPRAND;
  else if (type == "daily_box")
    return EQUESTTYPE_DAILY_BOX;
  else if (type == "artifact")
    return EQUESTTYPE_ARTIFACT;
  else if (type == "sign")
    return EQUESTTYPE_SIGN;
  else if (type == "day")
    return EQUESTTYPE_DAY;
  else if (type == "night")
    return EQUESTTYPE_NIGHT;
  else if (type == "wedding")
    return EQUESTTYPE_WEDDING;
  else if (type == "wedding_daily")
    return EQUESTTYPE_WEDDING_DAILY;
  else if (type == "capra")
    return EQUESTTYPE_CAPRA;
  else if (type == "dead")
    return EQUESTTYPE_DEAD;

  return EQUESTTYPE_MIN;
}

void QuestConfig::collectQuestGroupReward(DWORD dwGroupID, TSetDWORD& setRewardIDs)
{
  setRewardIDs.clear();
  auto m = m_mapGroupRewards.find(dwGroupID);
  if (m != m_mapGroupRewards.end())
    setRewardIDs.insert(m->second.begin(), m->second.end());
}

bool QuestConfig::isAccQuest(EQuestType eType) const
{
  return eType == EQUESTTYPE_ACC || eType == EQUESTTYPE_ACC_NORMAL || eType == EQUESTTYPE_ACC_DAILY || eType == EQUESTTYPE_ACC_CHOICE || isShareQuest(eType);
}

bool QuestConfig::isShareQuest(EQuestType eType) const
{
  return eType == EQUESTTYPE_ACC_MAIN || eType == EQUESTTYPE_ACC_BRANCH || eType == EQUESTTYPE_ACC_SATISFACTION || eType == EQUESTTYPE_ACC_DAILY_1 ||
    eType == EQUESTTYPE_ACC_DAILY_3 || eType == EQUESTTYPE_ACC_DAILY_7 || eType == EQUESTTYPE_ACC_DAILY_RESET;
}

const SWantedItem* QuestConfig::getWantedItemCFG(DWORD questid, bool bActivity/*= false*/ ) const
{
  TVecWantedQuestCFG vecQuesCFG;
  if(bActivity == false)
    vecQuesCFG = m_vecWantedQuestCFG;
  else
    vecQuesCFG = m_vecActivityWantedQuestCFG;
  for (auto &l : vecQuesCFG)
  {
    for (auto v = l.vecQuest.begin(); v != l.vecQuest.end(); ++v)
    {
      for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
      {
        for (auto o = v->arrAllItem[i].begin(); o != v->arrAllItem[i].end(); ++o)
        {
          if (o->dwQuestID == questid)
            return &(*o);
        }
      }
    }
  }
  return nullptr;
}

bool QuestConfig::isRealWantedQuest(DWORD id)
{
  const SQuestCFG* pCFG = getQuestCFG(id);
  if(pCFG == nullptr || pCFG->eType != EQUESTTYPE_WANTED)
    return false;

  const SWantedItem* pWantCFG = getWantedItemCFG(id);
  if(pWantCFG == nullptr || pWantCFG->bActivity == true)
    return false;

  return true;
}

const SQuestVersion* QuestConfig::getVersionCFG(const string& version) const
{
  auto m = m_mapQuestVersion.find(version);
  if (m != m_mapQuestVersion.end())
    return &m->second;
  return nullptr;
}

const SQuestPreQuest* QuestConfig::getQuestCFGByPre(DWORD questid) const
{
  auto m = m_mapPreQuest.find(questid);
  if (m != m_mapPreQuest.end())
    return &m->second;
  return nullptr;
}

const SVersionPuzzleCFG* QuestConfig::getQuestPuzzleCFG(const string& version) const
{
  auto m = m_mapQuestPuzzleCFG.find(version);
  if (m != m_mapQuestPuzzleCFG.end())
    return &m->second;
  return nullptr;
}

