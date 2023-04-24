#include "Quest.h"
#include "SceneUser.h"
#include "UserEvent.h"
#include "SceneUserManager.h"
#include "SceneServer.h"
#include "SceneManager.h"
#include "MsgManager.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "StatisticsDefine.h"
#include "PatchManager.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "ActivityEventManager.h"
#include "GMCommandRuler.h"
#include "Menu.h"
#include "DeathTransferMapConfig.h"
extern "C"
{
#include "xlib/md5/md5.h"
}

// quest config data
const TPtrBaseStep SQuestCFGEx::getStep(DWORD step) const
{
  if (step >= vecSteps.size())
    return nullptr;

  return vecSteps[step];
}

DWORD SQuestCFGEx::getStepBySubGroup(DWORD subGroup) const
{
  for (size_t i = 0; i < vecSteps.size(); ++i)
  {
    if (vecSteps[i]->getSubGroup() == subGroup)
      return static_cast<DWORD>(i);
  }

  return 0;
}

/*bool SQuestCFGEx::toData(QuestData* pData, DWORD language) const
{
  if (pData == nullptr)
    return false;
  if (pData->steps_size() != static_cast<int>(vecSteps.size()))
    return false;

  DWORD dwIndex = static_cast<int>(pData->step()) >= pData->steps_size() ? pData->step() - 1 : pData->step();
  toStepConfig(pData->mutable_steps(dwIndex), dwIndex, language);
  return true;
}*/

// quest config
QuestManager::QuestManager()
{

}

QuestManager::~QuestManager()
{
}

bool QuestManager::init()
{
  bool bSuccess = true;
  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "questconfig");
  if (!field)
  {
    XERR << "[任务配置-初始化] questconfig未在数据库中找到" << XEND;
    bSuccess = false;
    return bSuccess;
  }

  struct DBQuestCFG
  {
    DWORD dwQuestID = 0;
    DWORD dwVersion = 0;
    string data;
  };
  map<DWORD, DBQuestCFG> mapDBCFG;
  map<DWORD, DBQuestCFG> mapUpdateDBCFG;

  // load db quest config
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[任务配置-初始化] 查询数据库失败 ret :" << ret << XEND;
    bSuccess = false;
    return bSuccess;
  }
  for (DWORD d = 0; d < ret; ++d)
  {
    DWORD dwQuestID = set[d].get<DWORD>("questid");
    auto m = mapDBCFG.find(dwQuestID);
    if (m != mapDBCFG.end())
    {
      XERR << "[任务配置-初始化] 查询到重复任务 questid :" << dwQuestID << XEND;
      bSuccess = false;
      continue;
    }

    DBQuestCFG& rCFG = mapDBCFG[dwQuestID];
    rCFG.dwQuestID = dwQuestID;
    rCFG.dwVersion = set[d].get<DWORD>("version");
    rCFG.data = set[d].getString("txt");
  }

  // check update quest
  std::stringstream stream;
  m_mapQuestCFG.clear();
  const TVecQuestCFG& vecCFG = QuestConfig::getMe().getQuestCFGList();
  for (auto v = vecCFG.begin(); v != vecCFG.end(); ++v)
  {
    SQuestCFGEx stCFG;
    stCFG.id = v->id;
    stCFG.lv = v->lv;
    stCFG.joblv = v->joblv;
    stCFG.manuallv = v->manuallv;
    stCFG.mapid = v->mapid;
    stCFG.rewardGroupID = v->rewardGroupID;
    stCFG.starttime = v->starttime;
    stCFG.endtime = v->endtime;
    stCFG.version = v->version;
    stCFG.eType = v->eType;
    //stCFG.eProfession = v->eProfession;
    stCFG.vecProfession = v->vecProfession;
    stCFG.vecBranch = v->vecBranch;
    stCFG.eDestProfession = v->eDestProfession;
    stCFG.gender = v->gender;
    stCFG.toBranch = v->toBranch;
    stCFG.dwPrefixion = v->dwPrefixion;
    stCFG.name = v->name;
    stCFG.questname = v->questname;
    stCFG.manualVersion = v->manualVersion;

    stCFG.setValidWeekday.insert(v->setValidWeekday.begin(), v->setValidWeekday.end());
    stCFG.startsend = v->startsend;
    stCFG.endsend = v->endsend;

    stCFG.vecPreQuest.insert(stCFG.vecPreQuest.end(), v->vecPreQuest.begin(), v->vecPreQuest.end());
    stCFG.vecMustPreQuest.insert(stCFG.vecMustPreQuest.end(), v->vecMustPreQuest.begin(), v->vecMustPreQuest.end());
    stCFG.cookerLv = v->cookerLv;
    stCFG.tasterLv = v->tasterLv;
    stCFG.vecStepData.insert(stCFG.vecStepData.end(), v->vecStepData.begin(), v->vecStepData.end());
    stCFG.stGuildReq = v->stGuildReq;
    stCFG.stManualReq = v->stManualReq;
    stCFG.eRefreshType = v->eRefreshType;
    stCFG.refreshTimes = v->refreshTimes;

    stream.str("");
    stream << "##";

    for (auto o = v->vecStepData.begin(); o != v->vecStepData.end(); ++o)
    {
      xLuaData data = o->data;
      stream << o->data.getTableString("Content") << "_";
      o->data.getData("Params").toString(stream);

      TPtrBaseStep pStep = createStep(v->id, data.getTableString("Content"), data);
      if (pStep == nullptr)
      {
        XERR << "[任务配置-初始化] questid :" << v->id << "create step :" << o->data.getTableString("Content") << "error" << XEND;
        bSuccess = false;
        continue;
      }

      stCFG.vecSteps.push_back(pStep);
    }
    stream << "##";

    auto m = mapDBCFG.find(stCFG.id);
    if (m == mapDBCFG.end())
    {
      DBQuestCFG& rCFG = mapUpdateDBCFG[stCFG.id];
      rCFG.dwQuestID = stCFG.id;
      stCFG.version = rCFG.dwVersion = 1;
      rCFG.data = stream.str();

      XLOG << "[任务配置-初始化] questid :" << stCFG.id << "发生变化 version : 0 -> 1 old :" << " " << " new :" << stream.str() << XEND;
    }
    else
    {
      stCFG.version = m->second.dwVersion;
      if (m->second.data != stream.str())
      {
        DBQuestCFG& rCFG = mapUpdateDBCFG[stCFG.id];
        rCFG.dwQuestID = stCFG.id;
        stCFG.version = rCFG.dwVersion = m->second.dwVersion + 1;
        rCFG.data = stream.str();
        XLOG << "[任务配置-初始化] questid :" << stCFG.id << "发生变化 version :" << m->second.dwVersion << "->" << rCFG.dwVersion << "old :" << m->second.data << " new :" << stream.str() << XEND;
      }
    }

    m_mapQuestCFG[stCFG.id] = stCFG;
    if(stCFG.setValidWeekday.empty() == false)
    {
      if(stCFG.endtime != 0 && stCFG.endtime <= xTime::getCurSec())
        m_mapOutDateCycleQuestCFG.insert(make_pair(stCFG.id, stCFG));
      else
        m_mapCycleQuestCFG.insert(make_pair(stCFG.id, stCFG));
    }
  }

  // save new config to db
  if (mapUpdateDBCFG.empty() == false)
  {
    xRecordSet set;
    for (auto &m : mapUpdateDBCFG)
    {
      xRecord record(field);
      record.put("questid", m.second.dwQuestID);
      record.put("version", m.second.dwVersion);
      record.putString("txt", m.second.data);
      set.push(record);
    }

    QWORD ret = thisServer->getDBConnPool().exeInsertSet(set, true);
    if (ret == QWORD_MAX)
    {
      XERR << "[任务配置-初始化] 更新数据库失败" << XEND;
      bSuccess = false;
    }
  }

#ifdef _DEBUG
  for (auto &v : m_mapQuestCFG)
  {
    if (v.second.version == 0)
      XDBG << "[任务配置-测试] questid :" << v.first << "version :" << v.second.version << XEND;
  }
#endif

  // random wanted quest
  QuestConfig::getMe().randomWantedQuest(true);
  QuestConfig::getMe().randomMapRandQuest();
  return bSuccess;
}

void QuestManager::setDailyExtra(DWORD dwValue)
{
  m_dwDailyExtra = dwValue;
  SceneUserManager::getMe().syncDailyRate();
  XLOG << "[任务管理-设值] 设置抗击魔潮倍率 :" << dwValue << XEND;
}

const SQuestCFGEx* QuestManager::getQuestCFG(DWORD id) const
{
  auto it = m_mapQuestCFG.find(id);
  if (it == m_mapQuestCFG.end())
    return nullptr;

  return &(it->second);
}

bool QuestManager::createQuestData(QuestData* pData, const SQuestCFGEx* pCFG, SceneUser* pUser, const SWantedItem* pWanted /*= nullptr*/)
{
  if (pData == nullptr || pCFG == nullptr || pUser == nullptr)
    return false;

  TPtrBaseStep pStep = pCFG->getStep(0);
  if (pStep == nullptr)
    return false;

  pData->set_id(pCFG->id);
  pData->set_step(0);
  pData->set_time(xTime::getCurSec());
  pData->set_complete(false);
  pData->set_version(pCFG->version);
  pData->set_acceptlv(pUser->getLevel());

  for (size_t i = 0; i < pCFG->vecSteps.size(); ++i)
  {
    if (pData->add_steps() == nullptr)
      return false;
  }

  if (pCFG->eType == EQUESTTYPE_WANTED)
  {
    if(QuestConfig::getMe().isRealWantedQuest(pCFG->id) == true)
    {
      if (pWanted == nullptr)
        return false;
      pData->set_time(QuestConfig::getMe().getWantedTime());
      for (auto i = pWanted->vecRewards.begin(); i != pWanted->vecRewards.end(); ++i)
      {
        ItemInfo* pItem = pData->add_rewards();
        if (pItem != nullptr)
          pItem->CopyFrom(*i);
      }
      pData->set_finishcount(pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED));
    }
    else
    {
      const SWantedItem* pWantedItem = QuestConfig::getMe().getWantedItemCFG(pCFG->id, true);
      if(pWantedItem == nullptr)
        return false;
      for (auto i = pWantedItem->vecRewards.begin(); i != pWantedItem->vecRewards.end(); ++i)
      {
        ItemInfo* pItem = pData->add_rewards();
        if (pItem != nullptr)
          pItem->CopyFrom(*i);
      }
    }
  }
  else if (pCFG->eType == EQUESTTYPE_GUILD)
  {
#ifdef _DEBUG
    pData->set_time(xTime::getCurSec() + DAY_T);
    return true;
#endif
    const GuildQuest* pQuest = pUser->getGuild().getQuest(pCFG->id);
    if (pQuest == nullptr)
      return false;
    pData->set_time(pQuest->time());
  }

  return true;
}

void QuestManager::collectCanAcceptQuest(SceneUser* pUser, QuestList& rList)
{
  if (pUser == nullptr)
    return;

  for (auto v = m_mapQuestCFG.begin(); v != m_mapQuestCFG.end(); ++v)
  {
    if (v->second.eType == EQUESTTYPE_TRIGGER || v->second.eType == EQUESTTYPE_WANTED || v->second.eType == EQUESTTYPE_GUILD || v->second.eType == EQUESTTYPE_DEAD)
      continue;

    if (pUser->getQuest().canAcceptQuest(v->first) == false)
      continue;

    QuestData* pData = rList.add_list();
    if (pData == nullptr)
      continue;

    if (createQuestData(pData, &(v->second), pUser) == false)
      XERR << "[QuestManager::collectCanAcdeptQuest] create id =" << v->first << "data error!" << XEND;
  }
}

bool QuestManager::isQuestComplete(DWORD id, SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  const TVecDWORD& vecIDs = QuestConfig::getMe().getQuestDetail(id);

  Quest& rQuest = pUser->getQuest();
  for (auto o = vecIDs.begin(); o != vecIDs.end(); ++o)
  {
    if (rQuest.isSubmit(*o) == false)
      return false;
  }

  XLOG << "[大任务完成检查] questid :" << id << "完成" << XEND;
  return true;
}

bool QuestManager::isQuestComplete(const string& version, DWORD id, SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  const SQuestVersion* pCFG = QuestConfig::getMe().getVersionCFG(version);
  if (pCFG == nullptr)
    return false;

  Quest& rQuest = pUser->getQuest();
  auto main = pCFG->mapMainCFG.find(id);
  if (main != pCFG->mapMainCFG.end())
  {
    for (auto &v : pCFG->vecMainCFG)
    {
      if (v.id / QUEST_ID_PARAM == id && rQuest.isSubmit(v.id) == false)
        return false;
    }

    return true;
  }

  auto story = pCFG->mapStoryCFG.find(id);
  if (story != pCFG->mapStoryCFG.end())
  {
    for (auto &v : pCFG->vecStoryCFG)
    {
      if (v.id / QUEST_ID_PARAM == id && rQuest.isSubmit(v.id) == false)
        return false;
    }

    return true;
  }

  return false;
}

void QuestManager::timer(DWORD curTime)
{
  FUN_TIMECHECK_30();
  QuestConfig::getMe().randomWantedQuest();
  QuestConfig::getMe().randomMapRandQuest();
}

void QuestManager::collectWantedQuest(SceneUser* pUser, DWORD dwWantedID, QuestList& rList)
{
  if (pUser == nullptr)
    return;

  // no exist
  const SWantedQuestCFG* pCFG = QuestConfig::getMe().getWantedQuestCFG(pUser->getLevel());
  if (pCFG == nullptr)
    return;
  const SWantedQuest* pWantedCFG = pCFG->getWantedQuest(dwWantedID);
  if (pWantedCFG == nullptr)
    return;
  for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
  {
    for (auto v = pWantedCFG->arrCurItem[i].begin(); v != pWantedCFG->arrCurItem[i].end(); ++v)
    {
      const SQuestCFGEx* pCFG = getQuestCFG(v->dwQuestID);
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_WANTED)
        continue;

      QuestData* pData = rList.add_list();
      if (pData == nullptr)
        continue;

      if (createQuestData(pData, pCFG, pUser, &(*v)) == false)
      {
        XERR << "[QuestManager::collectCanAcdeptQuest] create id =" << v->dwQuestID << "data error!" << XEND;
        continue;
      }
      if (pData->steps_size() != 0)
        pCFG->toStepConfig(pData->mutable_steps(0), 0);
    }
  }

  //活动看板
  const SWantedQuestCFG* pActCFG = QuestConfig::getMe().getActivityWantedQuestCFG(pUser->getLevel());
  if (pActCFG == nullptr)
    return;
  const SWantedQuest* pActWantedCFG = pActCFG->getWantedQuest(dwWantedID);
  if (pActWantedCFG == nullptr)
    return;
  for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
  {
    for (auto v = pActWantedCFG->arrAllItem[i].begin(); v != pActWantedCFG->arrAllItem[i].end(); ++v)
    {
      const SQuestCFGEx* pCFG = getQuestCFG(v->dwQuestID);
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_WANTED)
        continue;
      if (pCFG->isInTime(xTime::getCurSec()) == false || pCFG->isInSendTime(xTime::getCurSec()) == false)
        continue;

      QuestData* pData = rList.add_list();
      if (pData == nullptr)
        continue;

      if (createQuestData(pData, pCFG, pUser, &(*v)) == false)
      {
        XERR << "[QuestManager::collectCanAcdeptQuest] create id =" << v->dwQuestID << "data error!" << XEND;
        continue;
      }
      if (pData->steps_size() != 0)
        pCFG->toStepConfig(pData->mutable_steps(0), 0);
    }
  }
}

// quest step
TPtrBaseStep QuestManager::createStep(DWORD id, const string& type, xLuaData& data)
{
  if (type == "visit")
  {
    TPtrBaseStep pStep( new VisitStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "kill")
  {
    TPtrBaseStep pStep( new KillStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "reward")
  {
    TPtrBaseStep pStep( new RewardStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "collect")
  {
    TPtrBaseStep pStep( new CollectStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "guard")
  {
    TPtrBaseStep pStep( new GuardStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "summon")
  {
    TPtrBaseStep pStep( new SummonStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "GM")
  {
    TPtrBaseStep pStep( new GMCmdStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "TestFail")
  {
    TPtrBaseStep pStep( new TestFailStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "use")
  {
    TPtrBaseStep pStep( new UseStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "gather")
  {
    TPtrBaseStep pStep( new GatherStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "delete")
  {
    TPtrBaseStep pStep( new DeleteStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "raid")
  {
    TPtrBaseStep pStep( new RaidStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "camera")
  {
    TPtrBaseStep pStep( new CameraStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "level")
  {
    TPtrBaseStep pStep( new LevelStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "wait")
  {
    TPtrBaseStep pStep( new WaitStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "move")
  {
    TPtrBaseStep pStep( new MoveStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "dialog")
  {
    TPtrBaseStep pStep( new DialogStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "prequest")
  {
    TPtrBaseStep pStep( new PreQuestStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "clearnpc")
  {
    TPtrBaseStep pStep( new ClearNpcStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "mount")
  {
    TPtrBaseStep pStep( new MountStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "selfie")
  {
    TPtrBaseStep pStep( new SelfieStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_team")
  {
    TPtrBaseStep pStep( new CheckTeamStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "remove_money")
  {
    TPtrBaseStep pStep( new RemoveMoneyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_class")
  {
    TPtrBaseStep pStep( new ClassStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_org_class")
  {
    TPtrBaseStep pStep( new OrgClassStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_class_evo")
  {
    TPtrBaseStep pStep( new EvoStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_quest")
  {
    TPtrBaseStep pStep( new CheckQuestStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_group")
  {
    TPtrBaseStep pStep( new CheckGroupStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_item")
  {
    TPtrBaseStep pStep( new CheckItemStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "remove_item")
  {
    TPtrBaseStep pStep( new RemoveItemStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "random_jump")
  {
    TPtrBaseStep pStep( new RandomJumpStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_level")
  {
    TPtrBaseStep pStep( new CheckLevelStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_gear")
  {
    TPtrBaseStep pStep( new CheckGearStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "purify")
  {
    TPtrBaseStep pStep( new PurifyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "action")
  {
    TPtrBaseStep pStep( new ActionStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "skill")
  {
    TPtrBaseStep pStep( new SkillStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "question")
  {
    TPtrBaseStep pStep( new InterStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "guide" ||
           type == "guide_highlight" ||
           type == "guideLockMonster")
  {
    TPtrBaseStep pStep( new GuideStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "guide_check")
  {
    TPtrBaseStep pStep( new GuideCheckStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "empty")
  {
    TPtrBaseStep pStep( new EmptyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_equiplv")
  {
    TPtrBaseStep pStep( new CheckEquipLvStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_money")
  {
    TPtrBaseStep pStep( new CheckMoneyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "option")
  {
    TPtrBaseStep pStep( new OptionStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_option")
  {
    TPtrBaseStep pStep( new CheckOptionStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "hint")
  {
    TPtrBaseStep pStep( new HintStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "seal")
  {
    TPtrBaseStep pStep( new SealStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "equiplv")
  {
    TPtrBaseStep pStep( new EquipLvStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "play_video")
  {
    TPtrBaseStep pStep( new VideoStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "illustration")
  {
    TPtrBaseStep pStep( new IllustrationStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "npcplay")
  {
    TPtrBaseStep pStep( new NpcPlayStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "item")
  {
    TPtrBaseStep pStep( new ItemStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "daily")
  {
    TPtrBaseStep pStep( new DailyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_adventure")
  {
    TPtrBaseStep pStep( new CheckManualStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "adventure")
  {
    TPtrBaseStep pStep( new ManualStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "playmusic")
  {
    TPtrBaseStep pStep( new PlayMusicStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "reward_help")
  {
    TPtrBaseStep pStep( new RewardHelpStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "money")
  {
    TPtrBaseStep pStep( new MoneyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "cat")
  {
    TPtrBaseStep pStep( new CatStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "activity")
  {
    TPtrBaseStep pStep( new ActivityStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "photo")
  {
    TPtrBaseStep pStep( new PhotoStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "item_use")
  {
    TPtrBaseStep pStep( new ItemUseStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "hand")
  {
    TPtrBaseStep pStep( new HandStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "music")
  {
    TPtrBaseStep pStep( new MusicStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "randitem")
  {
    TPtrBaseStep pStep( new RandItemStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "carrier")
  {
    TPtrBaseStep pStep( new CarrierStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "battle")
  {
    TPtrBaseStep pStep( new BattleStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "pet")
  {
    TPtrBaseStep pStep( new PetStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "cookfood")
  {
    TPtrBaseStep pStep( new CookFoodStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "scene")
  {
    TPtrBaseStep pStep( new SceneStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "cook")
  {
    TPtrBaseStep pStep( new CookStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "buff")
  {
    TPtrBaseStep pStep( new BuffStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "tutor")
  {
    TPtrBaseStep pStep( new TutorStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if(type == "christmas")
  {
    TPtrBaseStep pStep( new ChristmasStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if(type == "christmasrun")
  {
    TPtrBaseStep pStep( new ChristmasRunStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "being")
  {
    TPtrBaseStep pStep( new BeingStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if(type == "check_joy")
  {
    TPtrBaseStep pStep( new CheckJoyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if(type == "add_joy")
  {
    TPtrBaseStep pStep( new AddJoyStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if(type == "randdialog")
  {
    TPtrBaseStep pStep( new RandDialogStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return nullptr;
    return pStep;
  }
  else if (type == "play_cg")
  {
    TPtrBaseStep pStep(new CgStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "check_servant")
  {
    TPtrBaseStep pStep(new CheckServantStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "client_plot")
  {
    TPtrBaseStep pStep(new ClientPlotStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "chat")
  {
    TPtrBaseStep pStep(new ChatStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "transfer")
  {
    TPtrBaseStep pStep(new TransferStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "redialog")
  {
    TPtrBaseStep pStep(new ReDialogStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else if (type == "chat_system")
  {
    TPtrBaseStep pStep(new ChatSystemStep(id));
    if (pStep == nullptr || pStep->init(data) == false)
      return pStep;
    return pStep;
  }
  else
  {
    XERR << "[任务配置-创建步骤] questid =" << id << "content =" << type << "未创建" << XEND;
  }

  return nullptr;
}

// quest item
QuestItem::QuestItem(const QuestData& oData, const SQuestCFGEx* pCFG) : m_pCFG(pCFG)
{
  m_oData.CopyFrom(oData);
}

QuestItem::~QuestItem()
{

}

void QuestItem::toData(QuestData* pData)
{
  if (pData == nullptr)
    return;

  pData->CopyFrom(m_oData);
}

void QuestItem::toClientData(QuestData* pData, DWORD language)
{
  if (pData == nullptr)
    return;

  pData->CopyFrom(m_oData);
  if (m_pCFG != nullptr)
    m_pCFG->toData(pData, language);

  TPtrBaseStep pStep = getStepCFG();
  if (pStep != nullptr)
  {
    DialogStep* pDialogStep = dynamic_cast<DialogStep*>(getStepCFG().get());
    if (pDialogStep != nullptr && pDialogStep->isDynamic() == true && pData->params_size() != 0)
      pDialogStep->toDynamicConfig(pData->mutable_steps(pData->step()), pData->params(pData->params_size() - 1));
  }
}

void QuestItem::toSubmitData(QuestData* pData)
{
  if (pData == nullptr)
    return;

  pData->set_id(m_oData.id());
  pData->set_time(m_oData.time());
  for (int i = 0; i < m_oData.steps_size(); ++i)
  {
    QuestStep* pStep = pData->add_steps();
    if (pStep != nullptr)
      pStep->CopyFrom(m_oData.steps(i));
  }

  pData->set_step(m_oData.step());

  for (int i = 0; i < m_oData.rewards_size(); ++i)
  {
    ItemInfo* pItem = pData->add_rewards();
    if (pItem != nullptr)
      pItem->CopyFrom(m_oData.rewards(i));

    break; // only show first one
  }
  pData->set_acceptlv(m_oData.acceptlv());
  pData->set_finishcount(m_oData.finishcount());
  if (m_pCFG != nullptr)
    m_pCFG->toData(pData);
}

EQuestStep QuestItem::getStepType()
{
  TPtrBaseStep pStep = getStepCFG();
  if (pStep == nullptr)
    return EQUESTSTEP_MIN;

  return pStep->getStepType();
}

TPtrBaseStep QuestItem::getStepCFG() const
{
  if (m_pCFG == nullptr || m_oData.step() >= m_pCFG->vecSteps.size())
    return nullptr;

  return m_pCFG->vecSteps[m_oData.step()];
}

QuestStep* QuestItem::getStepData()
{
  DWORD step = m_oData.step();
  DWORD totalstep = m_oData.steps_size();
  if (step >= totalstep)
    return nullptr;

  return m_oData.mutable_steps(m_oData.step());
}

bool QuestItem::addStep()
{
  if (m_pCFG == nullptr)
    return false;
  if (m_oData.step() >= m_pCFG->vecSteps.size())
    return false;

  m_oData.set_step(m_oData.step() + 1);
  return true;
}

bool QuestItem::setStep(DWORD step)
{
  if (step >= m_pCFG->vecSteps.size())
    return false;

  m_oData.set_step(step);
  return true;
}

void QuestItem::collectWantedReward(TVecItemInfo& vecReward)
{
  for (int i = 0; i < m_oData.rewards_size(); ++i)
    vecReward.push_back(m_oData.rewards(i));
}

void QuestItem::collectSParams(TVecString& vecSParams)
{
  for (int i = 0; i < m_oData.names_size(); ++i)
    vecSParams.push_back(m_oData.names(i));
}

void QuestItem::collectQParams(TVecQWORD& vecQParams)
{
  for (int i = 0; i < m_oData.params_size(); ++i)
    vecQParams.push_back(m_oData.params(i));
}

void QuestItem::setQParams(DWORD index, QWORD qwValue)
{
  m_oData.set_params(index, qwValue);
}

QWORD QuestItem::getQParams(DWORD index)
{
  if (m_oData.params_size() == 0 || static_cast<int>(index) > m_oData.params_size())
    return 0;
  return m_oData.params(index);
}

// quest detail
QuestDetailData::QuestDetailData(const QuestDetail& data)
{
  m_oData.CopyFrom(data);
}

QuestDetailData::~QuestDetailData()
{

}

void QuestDetailData::toData(QuestDetail* pData)
{
  if (pData == nullptr)
    return;

  pData->CopyFrom(m_oData);
}

bool QuestDetailData::addDetailID(DWORD id)
{
  if (id == 0)
    return false;

  for (int i = 0; i < m_oData.details_size(); ++i)
  {
    if (m_oData.details(i) == id)
      return false;
  }

  m_oData.add_details(id);
  return true;
}

// quest
Quest::Quest(SceneUser* pUser) : m_pUser(pUser)
{

}

Quest::~Quest()
{

}

bool Quest::load(const BlobQuest& oAccQuest, const BlobQuest& oCharQuest)
{
  // clear list
  m_mapAcceptQuest.clear();
  m_mapCompleteQuest.clear();;
  m_mapSubmitQuest.clear();
  m_mapMapQuest.clear();
  m_vecDetails.clear();
  m_setProcessAccQuest.clear();
  m_setTimerQuestIDs.clear();

  // load acc quest
  for (int i = 0; i < oAccQuest.accept_size(); ++i)
  {
    const QuestData& rAccept = oAccQuest.accept(i);
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(rAccept.id());
    if (pCFG == nullptr)
    {
      XERR << "[任务-加载acc-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << "未在Table_Quest.txt表中找到" << XEND;
      continue;
    }

    TPtrQuestItem pItem( new QuestItem(rAccept, pCFG));
    if (pItem == nullptr)
    {
      XERR << "[任务-加载acc-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << "create error!" << XEND;
      continue;
    }

    bool bComplete = pItem->isComplete();
    if (bComplete)
      m_mapCompleteQuest[pItem->getID()] = pItem;
    else
      m_mapAcceptQuest[pItem->getID()] = pItem;

    string str = bComplete ? "\bcomplete" : "\baccept";
    XDBG << "[任务-加载acc-" << str << "\b]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << XEND;
  }
  for (int i = 0; i < oAccQuest.submit_size(); ++i)
  {
    const QuestData& rSubmit = oAccQuest.submit(i);
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(rSubmit.id());
    if (pCFG == nullptr)
      XERR << "[任务-加载acc-submit]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rSubmit.id() << "未在 Table_Quest.txt 表中找到" << XEND;

    TPtrQuestItem pItem( new QuestItem(rSubmit, pCFG));
    if (pItem == nullptr)
    {
      XERR << "[任务-加载acc-submit]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rSubmit.id() << "create error!" << XEND;
      continue;
    }

    m_mapSubmitQuest[pItem->getID()] = pItem;
    XDBG << "[任务-加载acc-submit]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rSubmit.id() << XEND;
  }
  m_vecDetails.reserve(oAccQuest.detail_size());
  for (int i = 0; i < oAccQuest.detail_size(); ++i)
  {
    const QuestDetail& rQuest = oAccQuest.detail(i);
    if (rQuest.id() != 80001 && rQuest.id() != 39022 && rQuest.id() != 39024)
    {
      m_vecDetails.push_back(QuestDetailData(rQuest));
      XDBG << "[任务-加载acc-detail]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << rQuest.id() << XEND;
    }
  }
  for (int i = 0; i < oAccQuest.process_acc_size(); ++i)
  {
    m_setProcessAccQuest.insert(oAccQuest.process_acc(i));
    XDBG << "[任务-加载acc-process]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << oAccQuest.process_acc(i) << XEND;
  }

  // char quest
  for (int i = 0; i < oCharQuest.accept_size(); ++i)
  {
    const QuestData& rAccept = oCharQuest.accept(i);
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(rAccept.id());
    if (pCFG == nullptr)
    {
      XERR << "[任务-加载char-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << "未在 Table_Quest.txt 表中找到" << XEND;
      continue;
    }

    TPtrQuestItem pItem( new QuestItem(rAccept, pCFG));
    if (pItem == nullptr)
    {
      XERR << "[任务-加载char-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << "create error!" << XEND;
      continue;
    }

    if (QuestConfig::getMe().isAccQuest(pCFG->eType) == true && pItem->getStep() <= 0)
    {
      auto s = m_setProcessAccQuest.find(pCFG->id);
      if (s != m_setProcessAccQuest.end())
      {
        XERR << "[任务-加载char-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << "其他账号在做,被忽略" << XEND;
        continue;
      }
      if (pCFG->eType == EQUESTTYPE_ACC_CHOICE)
      {
        DWORD questid = pCFG->id / QUEST_ID_PARAM;
        auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [questid](const QuestDetailData& r) -> bool{
          return questid == r.getID();
        });
        if (v != m_vecDetails.end())
        {
          XERR << "[任务-加载char-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << "大任务完成,被忽略" << XEND;
          continue;
        }
      }
    }

    if (QuestConfig::getMe().isShareQuest(pCFG->eType) == true)
    {
      XERR << "[任务-加载char-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rAccept.id() << "是acc共享任务,被忽略" << XEND;
      continue;
    }

    if (pItem->isComplete() == true)
    {
      m_mapCompleteQuest[pItem->getID()] = pItem;
    }
    else
    {
      if (rAccept.id() == 350100001)
        m_dwPatchStep = rAccept.step();
      m_mapAcceptQuest[pItem->getID()] = pItem;
      if (pCFG->eType == EQUESTTYPE_DEAD)
        m_setDeadIDs.insert(pCFG->id);
    }
  }
  for (int i = 0; i < oCharQuest.submit_size(); ++i)
  {
    const QuestData& rSubmit = oCharQuest.submit(i);
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(rSubmit.id());
    if (pCFG == nullptr)
      XERR << "[任务-加载char-submit]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rSubmit.id() << "未在 Table_Quest.txt 表中找到" << XEND;
    if (pCFG != nullptr && QuestConfig::getMe().isShareQuest(pCFG->eType) == true)
    {
      XERR << "[任务-加载char-submit]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rSubmit.id() << "是acc共享任务,被忽略" << XEND;
      continue;
    }

    TPtrQuestItem pItem( new QuestItem(oCharQuest.submit(i), pCFG));
    if (pItem == nullptr)
    {
      XERR << "[任务-加载char-submit]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rSubmit.id() << "create error!" << XEND;
      continue;
    }

    if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_DEAD)
      m_setDeadIDs.insert(pCFG->id);

    m_mapSubmitQuest[pItem->getID()] = pItem;
  }
  /*for (int i = 0; i < oCharQuest.detail_size(); ++i)
  {
    const QuestDetail& rQuest = oCharQuest.detail(i);
    if (rQuest.id() != 80001 && rQuest.id() != 39022 && rQuest.id() != 39024 && rQuest.id() != 30006)
      m_vecDetails.push_back(QuestDetailData(rQuest));
  }*/

  m_mapPuzzle.clear();
  const BlobQuestPuzzle& rPuzzle = oCharQuest.puzzle();
  for (int i = 0; i < rPuzzle.puzzles_size(); ++i)
  {
    const QuestPuzzle& puzzle = rPuzzle.puzzles(i);
    SQuestPuzzle& rPuzzle = m_mapPuzzle[puzzle.version()];
    rPuzzle.version = puzzle.version();
    for (int i = 0; i < puzzle.open_puzzles_size(); ++i)
      rPuzzle.setOpen.insert(puzzle.open_puzzles(i));
    for (int i = 0; i < puzzle.unlock_puzzles_size(); ++i)
      rPuzzle.setUnlock.insert(puzzle.unlock_puzzles(i));
  }

  m_dwDailyCount = oCharQuest.dailycount();
  m_dwDailyTCount = oCharQuest.dailytcount();
  m_dwDailyCurExp = oCharQuest.dailyexp();
  m_dwDailyLevel = oCharQuest.dailylevel();

  m_mapMapRandQuest.clear();
  for (int i = 0; i < oCharQuest.maprandquest_size(); ++i)
  {
    const MapQuest& rQuest = oCharQuest.maprandquest(i);
    TSetDWORD& setIDs = m_mapMapRandQuest[rQuest.mapid()];
    for (int j = 0; j < rQuest.questids_size(); ++j)
      setIDs.insert(rQuest.questids(j));
  }

  m_pExpPool = QuestConfig::getMe().getExpPool(m_dwDailyLevel);

  m_dwLastCalcDailyCountTime = oCharQuest.lastcalcdailycounttime();
  m_vecDailyGeted.clear();
  m_vecDailyGeted.reserve(oCharQuest.dailygift_size());
  for (int i = 0; i < oCharQuest.dailygift_size(); ++i)
    m_vecDailyGeted.push_back(oCharQuest.dailygift(i));

  for (int i = 0; i < oCharQuest.mapquest_size(); ++i)
  {
    const MapQuest& rQuest = oCharQuest.mapquest(i);
    for (int j = 0; j < rQuest.questids_size(); ++j)
      m_mapMapQuest[rQuest.mapid()].insert(rQuest.questids(j));
  }

  if (m_pUser->getUserState() == USER_STATE_LOGIN)
    resetQuest(EQUESTRESET_RELOGIN);

  m_setVarReward.clear();
  for (int i = 0; i < oCharQuest.varreward_size(); ++i)
    m_setVarReward.insert(oCharQuest.varreward(i));

  m_setForbidQuest.clear();
  for (int i = 0; i < oCharQuest.forbidquest_size(); ++i)
    m_setForbidQuest.insert(oCharQuest.forbidquest(i));

  checkQuestVersion(true);

  for (auto& v : m_mapAcceptQuest)
    if (v.second->getStepType() == EQUESTSTEP_WAIT || v.second->getStepType() == EQUESTSTEP_TUTOR)
      addQuestToTimer(v.first);

  return true;
}

bool Quest::save(BlobQuest* pAccBlob, BlobQuest* pCharBlob)
{
  if (pAccBlob == nullptr || pCharBlob == nullptr)
  {
    XERR << "[任务-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存失败,数据为空" << XEND;
    return false;
  }

  pAccBlob->Clear();
  pCharBlob->Clear();

  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
    {
      XERR << "[任务-保存-accept]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << m->first << "未在 Table_Quest.txt 表中找到" << XEND;
      continue;
    }
    if (QuestConfig::getMe().isShareQuest(pCFG->eType) == true)
    {
      m->second->toData(pAccBlob->add_accept());
      continue;
    }

    m->second->toData(pCharBlob->add_accept());
    if (QuestConfig::getMe().isAccQuest(pCFG->eType) == true && m->second->getStep() >= 1)
      m_setProcessAccQuest.insert(pCFG->id);
  }
  for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end(); ++m)
  {
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
    {
      XERR << "[任务-保存-complete]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << m->first << "未在 Table_Quest.txt 表中找到" << XEND;
      continue;
    }
    if (QuestConfig::getMe().isShareQuest(pCFG->eType) == true)
    {
      m->second->toData(pAccBlob->add_accept());
      continue;
    }

    if (pCFG == nullptr || m->second->getQuestCFG()->eType == EQUESTTYPE_WANTED)
      m->second->toData(pCharBlob->add_accept());
    else
    {
      QuestData* pAccept = pCharBlob->add_accept();
      if(pAccept != nullptr)
      {
        pAccept->set_id(m->first);
        pAccept->set_time(m->second->getTime());
      }
    }

    if (QuestConfig::getMe().isAccQuest(pCFG->eType) == true && m->second->getStep() >= 1)
      m_setProcessAccQuest.insert(pCFG->id);
  }
  for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end(); ++m)
  {
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
    {
      XERR << "[任务-保存-submit]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "acceptlist questid :" << m->first << "未在 Table_Quest.txt 表中找到" << XEND;
      continue;
    }
    if (QuestConfig::getMe().isShareQuest(pCFG->eType) == true)
    {
      QuestData* pSubmit = pAccBlob->add_submit();
      if(pSubmit != nullptr)
      {
        pSubmit->set_id(m->first);
        pSubmit->set_time(m->second->getTime());
        pSubmit->add_params(m->second->getQParams(0));
      }
      continue;
    }

    if (pCFG == nullptr || (m->second->getQuestCFG()->eType == EQUESTTYPE_WANTED || m->second->getQuestCFG()->eType == EQUESTTYPE_GUILD))
      m->second->toData(pCharBlob->add_submit());
    else
    {
      QuestData* pSubmit = pCharBlob->add_submit();
      if(pSubmit != nullptr)
      {
        pSubmit->set_id(m->first);
        pSubmit->set_time(m->second->getTime());
        pSubmit->add_params(m->second->getQParams(0));
      }
    }

    if (QuestConfig::getMe().isAccQuest(pCFG->eType) == true)
      m_setProcessAccQuest.insert(pCFG->id);
  }

  for (auto v = m_vecDetails.begin(); v != m_vecDetails.end(); ++v)
  {
    QuestDetail* pDetail = pCharBlob->add_detail();
    if (pDetail == nullptr)
    {
      XERR << "[任务-保存-detail]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << v->getID() << "create error!" << XEND;
      continue;
    }
    v->toData(pDetail);
  }

  BlobQuestPuzzle* pBlobPuzzle = pCharBlob->mutable_puzzle();
  for (auto &m : m_mapPuzzle)
  {
    QuestPuzzle* pPuzzle = pBlobPuzzle->add_puzzles();
    pPuzzle->set_version(m.first);
    for (auto &s : m.second.setOpen)
      pPuzzle->add_open_puzzles(s);
    for (auto &s : m.second.setUnlock)
      pPuzzle->add_unlock_puzzles(s);
  }

  pAccBlob->clear_process_acc();
  for (auto &s : m_setProcessAccQuest)
    pAccBlob->add_process_acc(s);

  pCharBlob->set_dailycount(m_dwDailyCount);
  pCharBlob->set_dailytcount(m_dwDailyTCount);
  pCharBlob->set_dailyexp(m_dwDailyCurExp);
  pCharBlob->set_dailylevel(m_dwDailyLevel);
  pCharBlob->set_lastcalcdailycounttime(m_dwLastCalcDailyCountTime);
  pCharBlob->clear_dailygift();
  for (auto v = m_vecDailyGeted.begin(); v != m_vecDailyGeted.end(); ++v)
    pCharBlob->add_dailygift(*v);

  for (auto &m : m_mapMapQuest)
  {
    MapQuest* p = pCharBlob->add_mapquest();
    if (p == nullptr)
    {
      XERR << "[任务-保存-maprand]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "创建mapquest失败" << XEND;
      continue;
    }
    p->set_mapid(m.first);
    for (auto &s : m.second)
      p->add_questids(s);
  }

  pCharBlob->clear_maprandquest();
  for (auto &m : m_mapMapRandQuest)
  {
    MapQuest* pQuest = pCharBlob->add_maprandquest();
    pQuest->set_mapid(m.first);
    for (auto &s : m.second)
      pQuest->add_questids(s);
  }

  pCharBlob->clear_varreward();
  for (auto &s : m_setVarReward)
    pCharBlob->add_varreward(s);

  pCharBlob->clear_forbidquest();
  for (auto &s : m_setForbidQuest)
    pCharBlob->add_forbidquest(s);

  XDBG << "[任务-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << pCharBlob->ByteSize() << XEND;
  return true;
}

bool Quest::loadActivityQuest(const BlobActivityQuest& data)
{
  m_mapActivityQuest.clear();
  for(int i = 0; i < data.activityitems_size(); ++i)
  {
    const ActivityQuestItem& rActivityItem = data.activityitems(i);
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(rActivityItem.questid());
    if(pCFG == nullptr)
    {
      XERR << "[活动任务-加载] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << rActivityItem.questid()
        << "未在Table_Quest.txt表中找到" << XEND;
      continue;
    }
    if(pCFG->isInTime(now()) == false || pCFG->isInTime(rActivityItem.lastquesttime()) == false)
      continue;

    m_mapActivityQuest.emplace(rActivityItem.questid(), rActivityItem);
  }

  resetActivityQuest();
  return true;
}

bool Quest::saveActivityQuest(BlobActivityQuest* data)
{
  if(data == nullptr)
  {
    XERR << "[活动任务-保存] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存失败,数据为空" << XEND;
    return false;
  }

  data->Clear();

  for(auto m = m_mapActivityQuest.begin(); m != m_mapActivityQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(m->second.questid());
    if(pCFG == nullptr)
    {
      XERR << "[任务-保存] 活动任务" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << m->second.questid()
        << "未在Table_Quest.txt表中找到" << XEND;
      continue;
    }

    if(pCFG->isInTime(now()) == false || pCFG->isInTime(m->second.lastquesttime()) == false)
      continue;

    ActivityQuestItem* pItem = data->add_activityitems();
    if(pItem != nullptr)
    {
      pItem->set_questid(m->second.questid());
      pItem->set_finishcount(m->second.finishcount());
      pItem->set_lastquesttime(m->second.lastquesttime());
    }

    XDBG << "[任务-保存] 活动任务" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << data->ByteSize() << XEND;
  }

  return true;
}

bool Quest::reload()
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
    m->second->m_pCFG = QuestManager::getMe().getQuestCFG(m->first);

  for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end(); ++m)
    m->second->m_pCFG = QuestManager::getMe().getQuestCFG(m->first);

  for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end(); ++m)
    m->second->m_pCFG = QuestManager::getMe().getQuestCFG(m->first);

  checkQuestVersion(false);

  if (m_dwDailyLevel != 0)
  {
    m_pExpPool = QuestConfig::getMe().getExpPool(m_dwDailyLevel);
    if (m_pExpPool == nullptr)
    {
      XERR << "[任务-重加载-抗击魔潮]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "m_dailyLevel :" << m_dwDailyLevel <<"m_dwDailyCurExp"<<m_dwDailyCurExp<< "抗魔找不到相应经验池。" << XEND;
      m_dwDailyCurExp = 0;
      m_dwDailyLevel = 0;
    }
  }

  return true;
}

/*void Quest::collectQuestItem(DWORD monsterid, TVecItemInfo& vecItemInfo)
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStepCFG = m->second->getStepCFG();
    if (pStepCFG == nullptr || pStepCFG->getStepType() != EQUESTSTEP_GATHER)
      continue;

    GatherStep* pStep = dynamic_cast<GatherStep*>(pStepCFG.get());
    if (pStep == nullptr)
      continue;

    TVecItemInfo vecItems;
    pStep->collectDropItem(m_pUser, monsterid, vecItems);
    vecItemInfo.insert(vecItemInfo.end(), vecItems.begin(), vecItems.end());
  }
}*/

void Quest::collectQuestReward(DWORD monsterid, TSetDWORD& setReward)
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStepCFG = m->second->getStepCFG();
    if (pStepCFG == nullptr || pStepCFG->getStepType() != EQUESTSTEP_GATHER)
      continue;

    GatherStep* pStep = dynamic_cast<GatherStep*>(pStepCFG.get());
    if (pStep == nullptr)
      continue;

    if (pStep->isPrivateGather())
      continue;

    DWORD dwRewardID = pStep->getRewardID(monsterid);
    if (dwRewardID != 0)
      setReward.insert(dwRewardID);
  }
}

void Quest::collectQuestReward(DWORD monsterid, TSetDWORD& setReward, TSetDWORD& setSelfReward)
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStepCFG = m->second->getStepCFG();
    if (pStepCFG == nullptr || pStepCFG->getStepType() != EQUESTSTEP_GATHER)
      continue;

    GatherStep* pStep = dynamic_cast<GatherStep*>(pStepCFG.get());
    if (pStep == nullptr)
      continue;

    DWORD dwRewardID = pStep->getRewardID(monsterid);
    if (dwRewardID == 0)
      continue;

    if (pStep->isPrivateGather())
      setSelfReward.insert(dwRewardID);
    else
      setReward.insert(dwRewardID);
  }
}

TPtrQuestItem Quest::getQuest(DWORD id)
{
  auto m = m_mapAcceptQuest.find(id);
  if (m != m_mapAcceptQuest.end())
    return m->second;

  m = m_mapCompleteQuest.find(id);
  if (m != m_mapCompleteQuest.end())
    return m->second;

  m = m_mapSubmitQuest.find(id);
  if (m != m_mapSubmitQuest.end())
    return m->second;

  return nullptr;
}

bool Quest::hasAcceptQuest(DWORD groupID)
{
  for (auto &m : m_mapAcceptQuest)
  {
    if (m.second && m.second->getGroupID() == groupID)
      return true;
  }
  return false;
}

DWORD Quest::getSubmitQuestConut() const
{
  DWORD dwCount = 0;
  for (auto &m : m_mapSubmitQuest)
  {
    if (m.second->getQuestCFG() != nullptr && m.second->getQuestCFG()->eType != EQUESTTYPE_WANTED)
      ++dwCount;
  }
  return dwCount;
}

DWORD Quest::getDailyCount()
{
  resetDailyQuest();
  return m_dwDailyCount;
}

DWORD Quest::getDailyTCount()
{
  resetDailyQuest();
  return m_dwDailyTCount;
}

void Quest::setDailyTCount(DWORD count)
{
  resetDailyQuest();
  const SQuestMiscCFG& rCFG = MiscConfig::getMe().getQuestCFG();
  if(count > rCFG.dwMaxDailyCount + m_dwDailyCount)
    count = rCFG.dwMaxDailyCount + m_dwDailyCount;

  m_dwDailyTCount = count;
  queryOtherData(EOTHERDATA_DAILY);
}

DWORD Quest::getDailyExp()
{
  resetDailyQuest();
  return m_dwDailyCurExp;
}

void Quest::reduceDailyExp(DWORD dwExp)
{
  if (m_dwDailyCurExp == 0 || m_pExpPool == nullptr)
    return;

  DWORD dwDropExp = LuaManager::getMe().call<DWORD>("calcDailyExpSub", dwExp);
  if (m_dwDailyCurExp >= dwDropExp)
    m_dwDailyCurExp -= dwDropExp;
  else
    m_dwDailyCurExp = 0;

  float fPercent = 1.0f * (m_pExpPool->dwExp - m_dwDailyCurExp) / m_pExpPool->dwExp;
  for (auto v = m_pExpPool->vecProcessGift.begin(); v != m_pExpPool->vecProcessGift.end(); ++v)
  {
    if (fPercent < v->fRate)
      continue;
    auto r = find(m_vecDailyGeted.begin(), m_vecDailyGeted.end(), v->oItem.id());
    if (r != m_vecDailyGeted.end())
      continue;
    m_vecDailyGeted.push_back(v->oItem.id());
    ItemInfo oItem = v->oItem;
    m_pUser->getPackage().addItem(oItem, EPACKMETHOD_AVAILABLE, false, true, true);//note 任务3d展示
  }

  queryOtherData(EOTHERDATA_DAILY);
  onDaily();
}

bool Quest::initDaily()
{
  if (m_pUser == nullptr)
    return false;
  if (getDailyCount() >= getDailyTCount())
    return false;
  if (hasDailyQuest() == false)
    return false;
  if (m_pExpPool != nullptr)
    return false;

  m_dwDailyLevel = m_pUser->getUserSceneData().getRolelv();
  m_pExpPool = QuestConfig::getMe().getExpPool(m_dwDailyLevel);
  if (m_pExpPool == nullptr)
  {
    XERR << "[任务-接取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "userlv :" << m_dwDailyLevel << "未在 Table_ExpPool.txt 表中找到" << XEND;
    return false;
  }

  m_dwDailyCurExp = m_pExpPool->dwExp;

  XLOG << "[任务-接取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "userlv :" << m_dwDailyLevel << "获得经验池 :" << m_dwDailyCurExp << XEND;
  queryOtherData(EOTHERDATA_DAILY);
  return true;
}

bool Quest::canAcceptQuest(DWORD id)
{
  if (m_pUser == nullptr)
    return false;

  if (getQuest(id) != nullptr)
    return false;

  // patch
  if (PatchManager::getMe().canPatchAccept(m_pUser, id) == false)
    return false;

  // forbid quest
  if (isForbid(id) == true)
    return false;

  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(id);
  if (pCFG == nullptr)
    return false;
  if (pCFG->isInTime(xTime::getCurSec()) == false || pCFG->isInSendTime(xTime::getCurSec()) == false)
    return false;
  if (m_pUser->getLevel() < pCFG->lv)
    return false;
  if (m_pUser->getJobLv() < pCFG->joblv)
    return false;
  if (m_pUser->getManual().getManualLv() < pCFG->manuallv)
    return false;
  if (pCFG->hasProfession(m_pUser->getUserSceneData().getProfession()) == false)
    return false;
  //if (pCFG->eProfession != EPROFESSION_MIN && m_pUser->getUserSceneData().getProfession() != pCFG->eProfession)
  //  return false;

  //check cookerlv exp
  if (pCFG->cookerLv && !m_pUser->getSceneFood().checkCookerLvExp(pCFG->cookerLv))
    return false;

  if (pCFG->tasterLv && !m_pUser->getSceneFood().checkTasterLvExp(pCFG->tasterLv))
    return false;

  if (pCFG->stManualReq.eType != EMANUALTYPE_MIN)
  {
    for (auto &s : pCFG->stManualReq.setIDs)
    {
      if (m_pUser->getManual().getCollectionStatus(s) < pCFG->stManualReq.eStatus)
        return false;
    }
  }

  if (pCFG->vecMustPreQuest.empty() == false)
  {
    for (auto v = pCFG->vecMustPreQuest.begin(); v != pCFG->vecMustPreQuest.end(); ++v)
    {
      auto m = m_mapSubmitQuest.find(*v);
      if (m == m_mapSubmitQuest.end())
        return false;
    }
  }

  if (pCFG->eType == EQUESTTYPE_WANTED)
  {
    if (QuestConfig::getMe().isRealWantedQuest(id) == true)
    {
    if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED) >= MiscConfig::getMe().getQuestCFG().getMaxWantedCount(EWANTEDTYPE_TOTAL))
      return false;
    if (hasWantedQuest() == true)
      return false;
  }
    else
    {
      const SWantedQuestCFG* pActCFG = QuestConfig::getMe().getActivityWantedQuestCFG(m_pUser->getLevel());
      if (pActCFG == nullptr)
        return false;
      const SWantedQuest* pActWantedCFG = pActCFG->getWantedQuest(101);
      if (pActWantedCFG == nullptr)
        return false;
      bool ret = false;
      for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
      {
        for (auto v = pActWantedCFG->arrAllItem[i].begin(); v != pActWantedCFG->arrAllItem[i].end(); ++v)
        {
          if (v->dwQuestID == id)
          {
            ret = true;
            break;
          }
        }
      }
      if(ret == false)
        return false;
    }
  }
  else if (pCFG->eType == EQUESTTYPE_DAILY)
  {
    if (getDailyCount() >= getDailyTCount())
      return false;
    if (hasDailyQuest() == true)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_DAILY_MAP)
  {
    if (hasDailyMapQuest(pCFG->mapid) == true)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_CHILD)
  {
    if (m_pUser->getVar().getVarValue(EVARTYPE_CHILD_QUEST) != 0)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_ACC)
  {
    if (m_setProcessAccQuest.find(id) != m_setProcessAccQuest.end())
      return false;
    if (m_pUser->getVar().getVarValue(EVARTYPE_ACC_QUEST) != 0)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_ACC_NORMAL || pCFG->eType == EQUESTTYPE_ACC_DAILY)
  {
    if (m_setProcessAccQuest.find(id) != m_setProcessAccQuest.end())
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_ACC_CHOICE)
  {
    auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [id](const QuestDetailData& r) -> bool{
      return id / QUEST_ID_PARAM == r.getID();
    });
    if (v != m_vecDetails.end())
      return false;
    if (m_pUser->getVar().getVarValue(EVARTYPE_ACC_CHOICE_QUEST) != 0)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_DAILY_MAPRAND)
  {
    const SDailyPerDay* pQuestCFG = MiscConfig::getMe().getQuestCFG().getDailyCount(pCFG->mapid);
    if (pQuestCFG == nullptr)
      return false;

    TSetDWORD& setIDs = m_mapMapRandQuest[pCFG->mapid];
    auto s = setIDs.find(pCFG->id);
    if (s == setIDs.end())
      return false;

    DWORD dwCount = getDailyRandCount(pCFG->mapid);
    if (dwCount >= pQuestCFG->dwSubmitCount)
      return false;

    DWORD dwAccCount = 0;
    for (auto &s : setIDs)
    {
      if (isAccept(s) == true)
        ++dwAccCount;
    }
    if (dwAccCount >= pQuestCFG->dwAcceptCount)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_SIGN)
  {
    if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_SIGN) != 0)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_DAY)
  {
    if (MiscConfig::getMe().getSystemCFG().isNight(xTime::getCurSec()) == true)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_NIGHT)
  {
    if (MiscConfig::getMe().getSystemCFG().isNight(xTime::getCurSec()) == false)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_ARTIFACT)
  {
    GGuild& rGuild = m_pUser->getGuild();
    bool bAuth = rGuild.hasAuth(EAUTH_ARTIFACT_QUEST);
    if (rGuild.id() == 0 || !bAuth)
      return false;
    const TSetDWORD& setIDs = rGuild.getGQuestSubmitList();
    auto s = find_if(setIDs.begin(), setIDs.end(), [pCFG](DWORD id) -> bool{
      return id / QUEST_ID_PARAM == pCFG->id / QUEST_ID_PARAM;
    });
    if (s != setIDs.end())
    {
      //XDBG << "[任务-接取检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "无法接取任务" << pCFG->id << *s << "已完成" << XEND;
      return false;
    }
    if (rGuild.getBuildingLevel(pCFG->stGuildReq.eType) <= 0)
    {
      //XDBG << "[任务-接取检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "无法接取任务" << pCFG->id << "建筑" << pCFG->stGuildReq.eType << "等级为0" << XEND;
      return false;
    }

    for (auto &v : pCFG->stGuildReq.vecDatas)
    {
      const ItemData* pInfo = rGuild.getArtifactPiece(v.base().id());
      if (pInfo == nullptr || pInfo->base().count() < v.base().count())
      {
        //XDBG << "[任务-接取检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "无法接取任务" << pCFG->id << "材料" << v.ShortDebugString() << "数量不足" << XEND;
        return false;
      }
    }
  }
  else if (pCFG->eType == EQUESTTYPE_WEDDING)
  {
    if (m_pUser->getUserWedding().isMarried() == false)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_WEDDING_DAILY)
  {
    if (m_pUser->getUserWedding().isMarried() == false || m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WEDDINGDAILY) != 0)
      return false;
  }
  else if (pCFG->eType == EQUESTTYPE_DEAD)
  {
    const SDeadMiscCFG& rCFG = MiscConfig::getMe().getDeadCFG();
    if (m_setDeadIDs.size() >= rCFG.dwQuestNum)
      return false;
  }

  if(pCFG->refreshTimes != 0)
  {
    auto m = m_mapActivityQuest.find(id);
    if(m != m_mapActivityQuest.end() && getActivityFinishTimes(id) >= pCFG->refreshTimes)
      return false;
  }

  if (pCFG->vecPreQuest.empty() == true)
    return true;

  for (auto v = pCFG->vecPreQuest.begin(); v != pCFG->vecPreQuest.end(); ++v)
  {
    auto m = m_mapSubmitQuest.find(*v);
    if (m != m_mapSubmitQuest.end())
      return true;
  }

  return false;
}

bool Quest::questAction(EQuestAction eAction, DWORD id, bool bNoCheck /*=false*/)
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return false;

  if (bNoCheck == false &&
      (eAction != EQUESTACTION_QUICK_SUBMIT_BOARD || m_dwQuickFinishBoardID == 0 || m_dwQuickFinishBoardID != id))
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
    if (pNpc == nullptr || pNpc->getCFG() == nullptr || pNpc->getScene() == nullptr || pNpc->getCFG()->stNpcFunc.hasFunction(ENPCFUNCTION_WANTED) == false)
    {
      XERR << "[任务-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "对 id :" << id << "进行" << eAction << "失败,未在正确的npc旁边" << XEND;
      return false;
    }
    float fDist = ::getDistance(pNpc->getPos(), m_pUser->getPos());
    if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius || pNpc->getScene() != m_pUser->getScene())
    {
      XERR << "[任务-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "对 id :" << id << "进行" << eAction << "失败,距离npc过远" << XEND;
      return false;
    }
  }

  if (eAction == EQUESTACTION_ABANDON_GROUP || eAction == EQUESTACTION_ABANDON_QUEST)
  {
    TPtrQuestItem pItem = getQuest(id);
    if (pItem != nullptr)
    {
      TPtrBaseStep pStep = pItem->getStepCFG();
      if (pStep == nullptr || pStep->canAbandon() == false)
      {
        XERR << "[任务-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "对 id :" << id << "进行" << eAction << "失败,当前步骤不允许放弃" << XEND;
        return false;
      }
    }
  }

  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(id);
  if (pCFG == nullptr)
  {
    XERR << "[任务-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "对 id :" << id << "进行" << eAction << "失败,未在Table_Quest.txt表中找到该任务" << XEND;
    return false;
  }
  if (pCFG->eType != EQUESTTYPE_WANTED)
  {
    XERR << "[任务-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "对 id :" << id << "进行" << eAction << "失败,该任务类型不支持玩家操作" << XEND;
    return false;
  }

  switch (eAction)
  {
    case EQUESTACTION_MIN:
      break;
    case EQUESTACTION_ACCEPT:
      return acceptQuest(id);
    case EQUESTACTION_SUBMIT:
      {
        bool result = submitQuest(id);
        if(QuestConfig::getMe().isRealWantedQuest(id) == true)
        inviteFinishBoard(id);
        return result;
      }
    case EQUESTACTION_ABANDON_GROUP:
      return abandonGroup(id);
    case EQUESTACTION_ABANDON_QUEST:
      return abandonQuest(id);
    case EQUESTACTION_QUICK_SUBMIT_BOARD:
      return quickFinishBoard(id);
    case EQUESTACTION_QUICK_SUBMIT_BOARD_TEAM:
      {
        if (quickFinishBoard(id) == false)
          return false;
        inviteFinishBoard(id, true); // 邀请队友快速完成
        return true;
      }
    case EQUESTACTION_MAX:
      break;
  }

  return false;
}

bool Quest::hasMainQuest()
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    if (m->second->isMain() == true)
      return true;
  }
  for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end(); ++m)
  {
    if (m->second->isMain() == true)
      return true;
  }

  return false;
}

bool Quest::hasWantedQuest()
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
      continue;
    if (pCFG->eType == EQUESTTYPE_WANTED)
      return true;
  }
  for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
      continue;
    if (pCFG->eType == EQUESTTYPE_WANTED)
      return true;
  }

  return false;
}

bool Quest::hasDailyQuest()
{
  resetDailyQuest();

  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
      continue;
    if (pCFG->eType == EQUESTTYPE_DAILY)
      return true;
  }
  for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
      continue;
    if (pCFG->eType == EQUESTTYPE_DAILY)
      return true;
  }

  return false;
}

bool Quest::hasDailyMapQuest(DWORD dwMapID)
{
  resetDailyMapQuest();

  auto m = m_mapMapQuest.find(dwMapID);
  if (m == m_mapMapQuest.end())
    return false;

  return static_cast<DWORD>(m->second.size()) >= MiscConfig::getMe().getQuestCFG().dwMaxMapCount;
}

bool Quest::isAccept(DWORD id)
{
  return m_mapAcceptQuest.find(id) != m_mapAcceptQuest.end();
}

bool Quest::isSubmit(DWORD id)
{
  auto m = m_mapSubmitQuest.find(id);
  return m != m_mapSubmitQuest.end();
}

bool Quest::setTrace(DWORD id, bool trace)
{
  auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [id](const QuestDetailData& r) -> bool{
      return id == r.getID();
  });
  if (v == m_vecDetails.end())
    return false;
  if (v->getTrace() == trace)
    return false;

  v->setTrace(trace);

  m_setDetailIDs.insert(id);
  return true;
}

bool Quest::setGroupTrace(DWORD id, bool trace)
{
  auto m = m_mapAcceptQuest.find(id);
  if (m == m_mapAcceptQuest.end())
  {
    XERR << "[任务-group追踪]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置" << id << "追踪状态" << trace << "失败,未在进行中" << XEND;
    return false;
  }
  if (m->second->getTrace() == trace)
  {
    XERR << "[任务-group追踪]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置" << id << "追踪状态" << trace << "失败,设置未变化" << XEND;
    return false;
  }

  m->second->setTrace(trace);
  m_setUpdateIDs.insert(id);

  XLOG << "[任务-group追踪]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置" << id << "追踪状态" << trace << "成功" << XEND;
  update();
  return true;
}

bool Quest::isQuestComplete(DWORD id) const
{
  DWORD questid = id / QUEST_ID_PARAM;
  auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [questid](const QuestDetailData& r) -> bool{
    return questid == r.getID();
  });
  return v != m_vecDetails.end() && v->getComplete() == true;
}

bool Quest::isForbid(DWORD id) const
{
  return m_setForbidQuest.find(id) != m_setForbidQuest.end();
}

bool Quest::checkVarReward(DWORD id) const
{
  return m_setVarReward.find(id) != m_setVarReward.end();
}

bool Quest::addVarReward(DWORD id)
{
  auto s = m_setVarReward.find(id);
  if (s != m_setVarReward.end())
  {
    XERR << "[任务-VarReward]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加 id :" << id << XEND;
    return false;
  }

  m_setVarReward.insert(id);
  XLOG << "[任务-VarReward]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加 id :" << id << XEND;
  return true;
}

void Quest::resetVarReward()
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_REWARD) != 0)
    return;

  m_pUser->getVar().setVarValue(EVARTYPE_QUEST_REWARD, 1);
  m_setVarReward.clear();
  XLOG << "[任务-VarReward]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置了VarReward列表" << XEND;
}

bool Quest::runStep(DWORD id, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  TPtrQuestItem pItem = getQuest(id);
  if (pItem == nullptr || pItem->isComplete() == true)
    return false;

  const SQuestCFGEx* pCFG = pItem->getQuestCFG();
  if (pCFG == nullptr)
  {
    XERR << "[任务-步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行" << id << "失败,未在 Table_Quest.txt 表中找到" << XEND;
    return false;
  }
  if (pCFG->isInTime(xTime::getCurSec()) == false || pCFG->isInSendTime(xTime::getCurSec()) == false)
  {
    XERR << "[任务-步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "执行" << id << "失败,未在指定的时间 start :" << pCFG->starttime << "end :" << pCFG->endtime << XEND;
    return false;
  }

  // 判断分支与职业
  if(!pCFG->checkProfession(m_pUser->getProfession()))
  {
    XLOG << "[任务-步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行" << id << "失败,职业不匹配!" << XEND;
    return false;
  }
  if(!pCFG->checkBranch(m_pUser->getBranch()))
  {
    XLOG << "[任务-步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行" << id << "失败,分支不匹配! branch:" << m_pUser->getBranch() << XEND;
    return false;
  }

  if(!pCFG->checkGender(m_pUser->getUserSceneData().getGender()))
  {
    XLOG << "[任务-步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行" << id << "失败,性别不匹配! gender:" << m_pUser->getUserSceneData().getGender() << XEND;
    return false;
  }

  TPtrBaseStep pStep = pItem->getStepCFG();
  if (pStep == nullptr)
    return false;

  if (pStep->doStep(m_pUser, subGroup, param1, param2, sparam1) == false)
    return false;

  if (canStep(id) == false)
    return false;

  pStep = pItem->getStepCFG();
  if (pStep != nullptr)
    pStep->reset(m_pUser);

  return runStep(id, 0);
}

void Quest::sendWantedQuestList(DWORD wantid)
{
  QuestList list;
  list.set_type(EQUESTLIST_CANACCEPT);

  QuestManager::getMe().collectWantedQuest(m_pUser, wantid, list);
  PROTOBUF(list, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Quest::sendGuildQuestList()
{
  resetGuildQuest();

  if (m_pUser == nullptr || m_pUser->hasGuild() == false)
    return;

  QuestUpdate cmd;
  QuestUpdateItem* pItem = cmd.add_items();
  pItem->set_type(EQUESTLIST_ACCEPT);
  const TMapGuildQuest& mapQuest = m_pUser->getGuild().getQuestList();
  for (auto &m : mapQuest)
  {
    if (getQuest(m.first) != nullptr)
      continue;

    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(m.first);
    if (pCFG == nullptr || pCFG->vecSteps.empty() == true)
      continue;

    QuestData* pData = pItem->add_update();
    pData->set_id(m.first);
    pData->set_time(m.second.time());

    pCFG->toStepConfig(pData->add_steps(), 0);
  }

  if (cmd.items_size() > 0 && cmd.items(0).update_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[任务-公会任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收到任务数据" << cmd.ShortDebugString() << XEND;
}

/*void Quest::sendQuestDetailList()
{
  QuestDetailList cmd;
  for (auto v = m_vecDetails.begin(); v != m_vecDetails.end(); ++v)
  {
    QuestDetail* pDetail = cmd.add_details();
    if (pDetail != nullptr)
      v->toData(pDetail);
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}*/

void Quest::sendCurQuestList(DWORD dwMapID /*= 0*/, bool bClear /*= fasle*/)
{
  // reset quest
  resetActivityQuest();
  resetDailyQuest();
  resetWantedQuest();
  resetDailyDayQuest();
  resetDailyMapQuest();
  resetDailyResetQuest();
  resetAccDailyQuest();
  resetDailyMapRandQuest();
  resetGuildQuest();
  resetCycleQuest();
  resetDeadQuest();

  // run enable quest
  onLogin();

  // collect enable quest
  acceptNewQuest();

  // cmd size
  DWORD dwCmdSize = 0;

  if(bClear)
    m_pUser->getUserSceneData().clearQuestNtf();

  // send accept list
  QuestList cmd;
  cmd.set_type(EQUESTLIST_ACCEPT);
  cmd.set_clear(bClear);
  TSetDWORD setQuestIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    TPtrBaseStep pStepCFG = m->second->getStepCFG();
    if (pCFG == nullptr || pStepCFG == nullptr)
      continue;
    if(!pCFG->checkProfession(m_pUser->getProfession()))
    {
      XDBG << "[任务-数据发送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "职业不匹配不显示 questid:" << pCFG->id << XEND;
      continue;
    }
    if(!pCFG->checkBranch(m_pUser->getBranch()))
    {
      XDBG << "[任务-数据发送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "分支不匹配不显示 branch:" << m_pUser->getBranch() << "questid:" << pCFG->id << XEND;
      continue;
    }
    if(!pCFG->checkGender(m_pUser->getUserSceneData().getGender()))
    {
      XDBG << "[任务-数据发送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "性别不匹配不显示 gender:" << m_pUser->getUserSceneData().getGender() << "questid:" << pCFG->id << XEND;
      continue;
    }
    if (m_pUser->getUserSceneData().isQuestNtf(pCFG->id) == true)
      continue;
    if (pStepCFG->getTraceInfo() == STRING_EMPTY || pCFG->eType == EQUESTTYPE_TALK)
    {
      if (pStepCFG->getMapID() != 0 && pStepCFG->getMapID() != dwMapID)
        continue;
    }
    QuestData* pData = cmd.add_list();
    if (pData == nullptr)
      continue;

    m->second->toClientData(pData);
    setQuestIDs.insert(pCFG->id);
    XDBG << "[任务-数据发送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "发送 accept :" << pData->ShortDebugString() << XEND;

    if (cmd.ByteSize() > TRANS_BUFSIZE - pData->ByteSize())
    {
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      dwCmdSize += cmd.ByteSize();
      cmd.clear_list();
      cmd.set_clear(false);
    }
  }
  if (cmd.list_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
    dwCmdSize += cmd.ByteSize();
  }

  if (m_pUser->getUserSceneData().hasQuestNtf() == false)
  {
    // send complete list
    QuestList cmd1;
    cmd1.set_type(EQUESTLIST_COMPLETE);
    for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end(); ++m)
    {
      QuestData* pData = cmd1.add_list();
      if (pData == nullptr)
        continue;

      m->second->toSubmitData(pData);
      //XDBG << "[任务-数据发送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "complete :" << pData->ShortDebugString() << XEND;
    }
    if (cmd1.list_size() > 0)
    {
      PROTOBUF(cmd1, send1, len1);
      m_pUser->sendCmdToMe(send1, len1);
      dwCmdSize += cmd1.ByteSize();
    }

    // send submit list
    QuestList cmd2;
    cmd2.set_type(EQUESTLIST_SUBMIT);
    for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end(); ++m)
    {
      QuestData* pData = cmd2.add_list();
      if (pData == nullptr)
        continue;

      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG == nullptr || (pCFG->eType != EQUESTTYPE_WANTED && AchieveConfig::getMe().isAchieveQuest(pCFG->id) == false))
        continue;

      m->second->toSubmitData(pData);
      //XDBG << "[任务-数据发送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit :" << pData->ShortDebugString() << XEND;
    }
    if (cmd2.list_size() > 0)
    {
      PROTOBUF(cmd2, send2, len2);
      m_pUser->sendCmdToMe(send2, len2);
      dwCmdSize += cmd2.ByteSize();
    }
  }

  m_setUpdateIDs.clear();
  m_setSubmitUpdateIDs.clear();
  m_setCompleteUpdateIDs.clear();
  for (auto &s : setQuestIDs)
    m_pUser->getUserSceneData().addQuestNtf(s);

  XDBG << "[任务-数据发送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "total size :" << dwCmdSize << XEND;
}

void Quest::acceptNewQuest()
{
  QuestList list;
  QuestManager::getMe().collectCanAcceptQuest(m_pUser, list);
  for (int i = 0; i < list.list_size(); ++i)
    acceptQuest(list.list(i).id());
}

void Quest::acceptTypeQuest(EQuestType eType)
{
  const TVecQuestCFG* pVecCFG = QuestConfig::getMe().getTypeQuestCFGList(eType);
  if (pVecCFG == nullptr)
    return;

  for (auto v = pVecCFG->begin(); v != pVecCFG->end(); ++v)
    acceptQuest(v->id);
}

void Quest::acceptCondQuest(EQuestCond eCond)
{
  const TVecQuestCFG* pVecCFG = QuestConfig::getMe().getCondQuestCFGList(eCond);
  if (pVecCFG == nullptr)
    return;

  for (auto v = pVecCFG->begin(); v != pVecCFG->end(); ++v)
    acceptQuest(v->id);
}

void Quest::removeInvalidQuest()
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
    {
      ++m;
      continue;
    }

    if (pCFG->eDestProfession != EPROFESSION_MIN && pCFG->eDestProfession != m_pUser->getUserSceneData().getProfession())
    {
      m_setUpdateIDs.insert(m->first);
      m = m_mapAcceptQuest.erase(m);
      continue;
    }

    ++m;
  }
}

void Quest::resetQuest(EQuestReset eReset)
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return;

  if (eReset <= EQUESTRESET_MIN || eReset >= EQUESTRESET_MAX)
    return;

  if (eReset == EQUESTRESET_RELOGIN && xTime::getCurSec() - m_pUser->getUserSceneData().getOfflineTime() < MiscConfig::getMe().getQuestCFG().dwResetProtectTime)
    return;
  if (eReset == EQUESTRESET_CHANGEMAP && m_pUser->getScene()->getMapID() == m_pUser->getUserSceneData().getLastMapID())
    return;

  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
      continue;

    TPtrBaseStep pStepCFG = m->second->getStepCFG();
    if (pStepCFG == nullptr || pStepCFG->getResetJump() == 0)
      continue;

    DWORD step = pCFG->getStepBySubGroup(pStepCFG->getResetJump());
    if (setQuestStep(m->first, step) == false)
      continue;
    if (eReset == EQUESTRESET_CHANGEMAP)
      stepUpdate(m->first);
  }
}

void Quest::timer(DWORD curTime)
{
  if(!m_pUser)
    return;

  if(m_bRefresh)
  {
    m_bRefresh = false;
    sendCurQuestList(m_pUser->getMapID(), true);
  }

  // update data
  //update();
  //detailupdate();

  // timer
  onTimer(curTime);
  //onChristmasTimer(curTime);

  // wanted
  resetActivityQuest();
  resetWantedQuest();
  resetDailyDayQuest();
  resetDailyResetQuest();
  resetAccDailyQuest();
  resetDailyMapRandQuest();
  resetDeadQuest();
}

void Quest::onMonsterKill(DWORD monsterID)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr)
      continue;
    if (pStep->getStepType() != EQUESTSTEP_KILL && pStep->getStepType() != EQUESTSTEP_COLLECT && pStep->getStepType() != EQUESTSTEP_BATTLE)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, monsterID);
  }
}

void Quest::onItemAdd(const ItemInfo& rItem)
{
  TVecDWORD vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr || (pStep->getStepType() != EQUESTSTEP_GATHER && pStep->getStepType() != EQUESTSTEP_ITEM && pStep->getStepType() != EQUESTSTEP_MONEY))
      continue;
    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, rItem.id());
  }
}

void Quest::onItemAdd(const string& guid)
{
  TVecDWORD vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_EQUIPLV)
      continue;
    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, 0, 0, guid);
  }
}

void Quest::onBeingLvUp()
{
  TVecDWORD vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_BEING)
      continue;
    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v);
  }
}

void Quest::onChat(const string& key)
{
  set<TPtrQuestItem> setItems;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_CHAT)
      continue;
    setItems.insert(m.second);
  }

  for (auto &s : setItems)
    runStep(s->getID(), 0, 0, 0, key);
}

void Quest::onChatSystem(DWORD channel, QWORD destid, const string& key)
{
  set<TPtrQuestItem> setItems;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_CHAT_SYSTEM)
      continue;
    setItems.insert(m.second);
  }

  for (auto &s : setItems)
    runStep(s->getID(), 0, destid, channel, key);
}

void Quest::onItemUse(QWORD qwTargetID, DWORD dwItemID)
{
  m_setTmpUser.clear();
  m_setTmpNpc.clear();

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(qwTargetID);
  if (pUser != nullptr)
  {
    m_setTmpUser.insert(pUser);
    XDBG << "[任务-道具使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收集玩家 :" << pUser->id << pUser->name << XEND;
  }

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwTargetID);
  if (pNpc != nullptr)
  {
    m_setTmpNpc.insert(pNpc);
    XDBG << "[任务-道具使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收集NPC :" << pNpc->id << pNpc->getNpcID() << pNpc->name << XEND;
  }

  if (m_setTmpUser.empty() == true && m_setTmpNpc.empty() == true)
    return;

  TSetDWORD setIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_ITEMUSE)
      continue;
    setIDs.insert(m.first);
  }

  for (auto &s : setIDs)
  {
    runStep(s, 0, dwItemID);
  }

  m_setTmpUser.clear();
  m_setTmpNpc.clear();
}

void Quest::onPassRaid(DWORD raidid, bool bSuccess)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr)
      continue;
    if (pStep->getStepType() != EQUESTSTEP_RAID)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    auto m = m_mapAcceptQuest.find(*v);
    if (m == m_mapAcceptQuest.end())
      continue;

    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr)
      continue;
    if (pStep->getStepType() != EQUESTSTEP_RAID)
      continue;

    DWORD dwJumpID = bSuccess ? pStep->getFinishJump() : pStep->getFailJump();
    runStep(*v, dwJumpID, raidid);
  }
}

void Quest::onRaidCmd(DWORD questid)
{
  auto it = m_mapAcceptQuest.find(questid);
  if (it==m_mapAcceptQuest.end()) return;

  TPtrBaseStep pStep = it->second->getStepCFG();
  if (pStep == nullptr)
    return;
  if (pStep->getStepType() != EQUESTSTEP_RAID)
    return;

  DWORD raidid = ((RaidStep *)pStep.get())->getRaidID();
  DScene* pDScene = dynamic_cast<DScene*>(m_pUser->getScene());
  if (pDScene != nullptr && pDScene->getRaidID() == raidid)
    return;
  DWORD range = ((RaidStep *)pStep.get())->getRange();
  if (range != 0)
  {
    if (m_pUser->getScene())
    {
      xPos raidpos = m_pUser->getPos();
      const xPos& pos = ((RaidStep *)pStep.get())->getCenterPos();
      if (!pos.empty())
      {
        raidpos = pos;
        if (m_pUser->getScene()->getValidPos(raidpos) == false)
          raidpos = m_pUser->getPos();
      }
      m_pUser->getScene()->m_oImages.add(raidid, m_pUser, raidpos, range);
    }
  }
  else
  {
    SceneManager::getMe().createDScene(CreateDMapParams(m_pUser->id, raidid));
  }
  //m_pUser->createDMap(raidid);
}

void Quest::onLevelup(DWORD baseLv, DWORD jobLv)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr)
      continue;
    if (pStep->getStepType() != EQUESTSTEP_LEVEL)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, baseLv, jobLv);
  }
}

void Quest::onTimer(DWORD curTime)
{
  // runstep可能调用SceneUser::leaveScene(如gm指令类型调用gocity), 导致Quest::timer(0)被调用
  // 故此处curTimer为0时不作处理
  if (curTime == 0)
    return;
  if (m_setTimerQuestIDs.empty() && m_mapTimerQuestUpdate.empty())
    return;

  for (auto& v : m_mapTimerQuestUpdate)
  {
    if (v.second)
      m_setTimerQuestIDs.insert(v.first);
    else
      m_setTimerQuestIDs.erase(v.first);
  }
  m_mapTimerQuestUpdate.clear();

  TSetDWORD temp;
  temp = m_setTimerQuestIDs;
  for (auto& v : temp)
    runStep(v, 0, curTime);
}

void Quest::onTenMinTimeUp(DWORD curTime)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr)
      continue;
    if (pStep->getStepType() != EQUESTSTEP_WAIT && pStep->getStepType() != EQUESTSTEP_TUTOR)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, curTime);
  }
}

void Quest::onChristmasTimer(DWORD curTime)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    if(m->second->getStepType() == EQUESTSTEP_CHRISTMAS_RUN || m->second->getStepType() == EQUESTSTEP_CHRISTMAS)
      vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, curTime);
  }
}

void Quest::onLogin()
{
  TVecDWORD vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->eType == EQUESTTYPE_TRIGGER || pCFG->eType == EQUESTTYPE_TALK)
      continue;
    if (canStep(m->first) == false)
      continue;
    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    if (canStep(*v) == false)
      continue;

    runStep(*v);
  }

  checkOutDateQuest();
}

void Quest::onAction(EUserActionType eType, QWORD id)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr)
      continue;
    if (pStep->getStepType() != EQUESTSTEP_ACTION)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, id, static_cast<DWORD>(eType));
  }
}

void Quest::onInterlocution(DWORD dwInterID, bool bCorrect)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr)
      continue;
    if (pStep->getStepType() != EQUESTSTEP_INTERLOCUTION)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, dwInterID, bCorrect);
  }
}

void Quest::onSeal(DWORD dwSealID)
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_SEAL)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v, 0, dwSealID);
  }
}

void Quest::onManualUnlock()
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_MANUAL)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    runStep(v);
  }
}

void Quest::onDaily()
{
  vector<DWORD> vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_DAILY)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    runStep(*v);
  }
}

void Quest::patch_2016_07_22()
{
  TVecDWORD vecIDs;
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
  {
    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_COLLECT)
      continue;

    vecIDs.push_back(m->first);
  }

  for (auto &v : vecIDs)
  {
    auto m = m_mapAcceptQuest.find(v);
    if (m == m_mapAcceptQuest.end())
      continue;

    TPtrBaseStep pStep = m->second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_COLLECT)
      continue;

    //abandonGroup(v);
    pStep->BaseStep::doStep(m_pUser, pStep->getFinishJump());
  }

  acceptNewQuest();
}

void Quest::patch_2017_09_11()
{
  DWORD evo = m_pUser->getProfession() % 10;
  if (evo >= 2)
    return;

  const TVecDWORD& vecID1s = QuestConfig::getMe().getQuestDetail(1201);
  const TVecDWORD& vecID2s = QuestConfig::getMe().getQuestDetail(1205);
  const TVecDWORD& vecID3s = QuestConfig::getMe().getQuestDetail(1207);
  const TVecDWORD& vecID4s = QuestConfig::getMe().getQuestDetail(1208);

  TVecDWORD vecIDs;
  vecIDs.insert(vecIDs.end(), vecID1s.begin(), vecID1s.end());
  vecIDs.insert(vecIDs.end(), vecID2s.begin(), vecID2s.end());
  vecIDs.insert(vecIDs.end(), vecID3s.begin(), vecID3s.end());
  vecIDs.insert(vecIDs.end(), vecID4s.begin(), vecID4s.end());

  for (auto o = vecIDs.begin(); o != vecIDs.end(); ++o)
  {
    auto a = m_mapAcceptQuest.find(*o);
    if (a != m_mapAcceptQuest.end())
    {
      m_mapAcceptQuest.erase(a);
      XLOG << "[任务-补丁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "接取列表 questid :" << *o << "被重置" << XEND;
    }

    auto c = m_mapCompleteQuest.find(*o);
    if (c != m_mapCompleteQuest.end())
    {
      m_mapCompleteQuest.erase(c);
      XLOG << "[任务-补丁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成列表 questid :" << *o << "被重置" << XEND;
    }

    auto s = m_mapSubmitQuest.find(*o);
    if (s != m_mapSubmitQuest.end())
    {
      m_mapSubmitQuest.erase(s);
      XLOG << "[任务-补丁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "提交列表 questid :" << *o << "被重置" << XEND;
    }
  }

  removeQuestDetail(1207);
  removeQuestDetail(1208);
}

bool Quest::acceptGuildQuest(DWORD id)
{
  if (isAccept(id) == true)
    return true;
  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(id);
  if (pCFG == nullptr)
    return false;
  if (pCFG->eType != EQUESTTYPE_GUILD)
    return true;
  if (m_pUser == nullptr || m_pUser->hasGuild() == false)
    return false;
  if (getQuest(id) != nullptr)
    return false;
  const GuildSMember* pMember = m_pUser->getGuild().getMember(m_pUser->id);
  if (pMember == nullptr || pMember->entertime() + MiscConfig::getMe().getGuildCFG().dwQuestProtectTime > xTime::getCurSec())
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_GUILD_QUEST_TIME);
    return false;
  }
  return acceptQuest(id);
}

bool Quest::acceptQuest(DWORD id, bool bNoCheck /*= false*/)
{
  if (!bNoCheck)
  {
    if (isQuestComplete(id) == true)
    {
      XERR << "[任务-接取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id" << id << "的大任务已完成,跳过该任务" << XEND;
      return false;
    }

    if (canAcceptQuest(id) == false)
      return false;
  }

  auto m = m_mapAcceptQuest.find(id);
  if (m != m_mapAcceptQuest.end())
    return false;

  auto o = m_mapCompleteQuest.find(id);
  if (o != m_mapCompleteQuest.end())
    return false;

  auto s = m_mapSubmitQuest.find(id);
  if (s != m_mapSubmitQuest.end())
    return false;

  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(id);
  if (pCFG == nullptr)
    return false;

  const SWantedItem* pWantedItem = nullptr;
  if (pCFG->eType == EQUESTTYPE_WANTED && QuestConfig::getMe().isRealWantedQuest(id) == true)
  {
    const SWantedQuestCFG* pWantedCFG = QuestConfig::getMe().getWantedQuestCFG(m_pUser->getLevel());
    if (pWantedCFG == nullptr)
      return false;

    if(QuestConfig::getMe().isRealWantedQuest(id) == true)
    {
    pWantedItem = pWantedCFG->getWantedItem(id);
    if (pWantedItem == nullptr)
    {
#ifdef _DEBUG
      MsgManager::sendMsg(m_pUser->id, 10, MsgParams("看板列表未更新"));
#endif
      return false;
    }

    //if (pWantedItem->bTeamSync)
    {
      TeamerQuestUpdateSocialCmd teamcmd;
      MemberWantedQuest* pTeamQuest = teamcmd.mutable_quest();
      pTeamQuest->set_action(EQUESTACTION_ACCEPT);
      pTeamQuest->set_charid(m_pUser->id);
      pTeamQuest->set_questid(id);
      PROTOBUF(teamcmd, send2, len2);
      thisServer->sendCmdToSession(send2, len2);
    }
    const GTeam& rTeam = m_pUser->getTeam();
    for (auto &m : rTeam.getTeamMemberList())
    {
      if (m.second.charid() == m_pUser->id)
        continue;
      MsgManager::sendMsg(m.second.charid(), 898, MsgParams(m_pUser->name, pWantedItem->strName));
    }
  }
    else
    {
      pWantedItem = pWantedCFG->getWantedItem(id, false);
      if (pWantedItem == nullptr)
        return false;
    }
  }
  else if (pCFG->eType == EQUESTTYPE_DAILY_MAP)
  {
    m_mapMapQuest[pCFG->mapid].insert(pCFG->id);
  }

  QuestData oData;
  if (QuestManager::getMe().createQuestData(&oData, pCFG, m_pUser, pWantedItem) == false)
    return false;

  TPtrQuestItem pItem( new QuestItem(oData, pCFG));
  if (pItem == nullptr)
    return false;

  if (pCFG->eType == EQUESTTYPE_ARTIFACT)
    pItem->addqparams(m_pUser->getGuild().id());
  else if (pCFG->eType == EQUESTTYPE_DEAD)
    m_setDeadIDs.insert(pCFG->id);

  m_mapAcceptQuest[pItem->getID()] = pItem;
  m_setUpdateIDs.insert(pItem->getID());

  if (canStep(id))
    runStep(id);

  auto it = m_mapAcceptQuest.find(id);
  if (it != m_mapAcceptQuest.end() && (it->second->getStepType() == EQUESTSTEP_WAIT || it->second->getStepType() == EQUESTSTEP_TUTOR))
    addQuestToTimer(id);

  if (pCFG->eType == EQUESTTYPE_MAIN || pCFG->eType == EQUESTTYPE_BRANCH || pCFG->eType == EQUESTTYPE_STORY || 
      (pCFG->eType == EQUESTTYPE_WANTED && QuestConfig::getMe().isRealWantedQuest(id) == true))
    addQuestDetail(pCFG->getStep(0));

  update();

  if (pCFG->eType == EQUESTTYPE_GUILD)
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_QUEST_ACCEPT);
  else
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_ACCEPT_QUEST);
  XLOG << "[任务-接取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "questid :" << id << "wantedcount :" << m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED) << XEND;

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Quest_Start;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  std::string rewardItem;
  PlatLogManager::getMe().QuestLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    id,
    EQuestType_START,
    0,
    0,
    0,
    rewardItem,
    m_pUser->getLevel());

  return true;
}

bool Quest::abandonGroup(DWORD id, bool bSubmitInclue /*= false*/, bool bNtf /*= true*/)
{
  if (m_pUser == nullptr)
    return false;

  bool bDelete = false;
  m_pUser->m_oFollower.delquest(id);

  auto m = m_mapAcceptQuest.find(id);
  auto o = m_mapCompleteQuest.find(id);

  if (m != m_mapAcceptQuest.end())
  {
    m_mapAcceptQuest.erase(m);
    m_setUpdateIDs.insert(id);
    bDelete = true;
  }
  if (o != m_mapCompleteQuest.end())
  {
    m_mapCompleteQuest.erase(o);
    m_setCompleteUpdateIDs.insert(id);
    bDelete = true;
  }

  if (bSubmitInclue)
  {
    auto s = m_mapSubmitQuest.find(id);
    if (s != m_mapSubmitQuest.end())
    {
      m_mapSubmitQuest.erase(s);
      m_setSubmitUpdateIDs.insert(id);
      bDelete = true;
    }
  }

  //const SWantedItem* pWantedItem = QuestConfig::getMe().getWantedItemCFG(id);
  //if (pWantedItem && pWantedItem->bTeamSync)
  {
    TeamerQuestUpdateSocialCmd teamcmd;
    MemberWantedQuest* pTeamQuest = teamcmd.mutable_quest();
    pTeamQuest->set_action(EQUESTACTION_ABANDON_GROUP);
    pTeamQuest->set_charid(m_pUser->id);
    pTeamQuest->set_questid(id);
    PROTOBUF(teamcmd, send2, len2);
    thisServer->sendCmdToSession(send2, len2);
  }

  if (bDelete && bNtf)
  {
    update();
    XLOG << "[任务-放弃]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "questid :" << id << "wantedcount :" << m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED) << XEND;
  }
  return true;
}

bool Quest::abandonQuest(DWORD id)
{
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->id / QUEST_ID_PARAM != id)
    {
      ++m;
      continue;
    }

    m_pUser->m_oFollower.delquest(pCFG->id);
    m_setUpdateIDs.insert(m->first);
    m = m_mapAcceptQuest.erase(m);
  }
  for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end();)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->id / QUEST_ID_PARAM != id)
    {
      ++m;
      continue;
    }

    m_pUser->m_oFollower.delquest(pCFG->id);
    m_setCompleteUpdateIDs.insert(m->first);
    m = m_mapCompleteQuest.erase(m);
  }

  removeQuestDetail(id / QUEST_ID_PARAM);
  update();
  return true;
}

bool Quest::canQuickFinishBoard(DWORD id, ItemInfo& oItem)
{
  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(id);
  if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_WANTED)
  {
    XERR << "[任务-看板快速完成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成 id :" << id << "失败,未找到配置或者不是看板任务" << XEND;
    return false;
  }

  const SWantedItem* pWantedItem = nullptr;
  const SWantedQuestCFG* pWantedCFG = QuestConfig::getMe().getWantedQuestCFG(m_pUser->getLevel());
  if (pWantedCFG == nullptr)
  {
    XERR << "[任务-看板快速完成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成 id :" << id << "失败,玩家baselv :" << m_pUser->getLevel() << "当前未包含看板任务" << XEND;
    return false;
  }
  pWantedItem = pWantedCFG->getWantedItem(id);
  if (pWantedItem == nullptr)
  {
    XERR << "[任务-看板快速完成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成 id :" << id << "失败,该任务未被刷新" << XEND;
    return false;
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[任务-看板快速完成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成 id :" << id << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }

  const SQuestMiscCFG& rCFG = MiscConfig::getMe().getQuestCFG();
  for (auto &v : rCFG.vecBoardQuickFinishItems)
  {
    if (pWantedCFG->lvRange.first >= v.lvrange.first && v.lvrange.second >= pWantedCFG->lvRange.second)
    {
      if (pMainPack->checkItemCount(v.dwItemID) == true)
      {
        oItem.set_id(v.dwItemID);
        oItem.set_count(1);
        break;
      }
    }
  }
  if (oItem.id() == 0)
  {
    XERR << "[任务-看板快速完成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成 id :" << id << "失败,未找到快速完成道具" << XEND;
    return false;
  }
  return true;
}

bool Quest::quickFinishBoard(DWORD id)
{
  ItemInfo oItem;
  if (canQuickFinishBoard(id, oItem) == false)
    return false;

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;

  stringstream sstr;
  sstr << "finishboardquest submit=1 id=" << id;
  if (GMCommandRuler::getMe().execute(m_pUser, sstr.str()) == false)
  {
    XERR << "[任务-看板快速完成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成 id :" << id << "失败,完成失败" << XEND;
    return false;
  }
  pMainPack->reduceItem(oItem.id(), ESOURCE_WANTEDQUEST);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_QUICK_WANTED_QUEST);
  XLOG << "[任务-看板快速完成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成 id :" << id << "成功" << XEND;

  m_dwQuickFinishBoardID = 0;

  return true;
}

bool Quest::queryOtherData(EOtherData eType)
{
  if (m_pUser == nullptr)
    return false;

  if (eType <= EOTHERDATA_MIN || eType >= EOTHERDATA_MAX)
    return false;

  if (eType == EOTHERDATA_DAILY)
  {
    resetDailyQuest();
    QueryOtherData cmd;
    cmd.set_type(eType);
    cmd.mutable_data()->set_param1(m_dwDailyTCount);
    cmd.mutable_data()->set_param2(m_dwDailyCount);
    cmd.mutable_data()->set_param3(m_dwDailyCurExp);
    cmd.mutable_data()->set_param4(QuestManager::getMe().getDailyExtra());

    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  return true;
}

bool Quest::queryWorldQuest()
{
  struct WorldInfo
  {
    bool bMain = false;
    bool bOther = false;
    bool bDaily = false;
  };
  map<DWORD, WorldInfo> mapInfo;

  QueryWorldQuestCmd cmd;
  for (auto &m : m_mapAcceptQuest)
  {
    const SQuestCFGEx* pCFG = m.second->getQuestCFG();
    TPtrBaseStep pStepCFG = m.second->getStepCFG();
    if (pCFG == nullptr || pStepCFG == nullptr)
      continue;

    if (pCFG->eType == EQUESTTYPE_MAIN)
      mapInfo[pStepCFG->getMapID()].bMain = true;
    else if (pCFG->eType == EQUESTTYPE_DAILY)
      mapInfo[pStepCFG->getMapID()].bDaily = true;
    else
      mapInfo[pStepCFG->getMapID()].bOther = true;
  }

  for (auto &m : mapInfo)
  {
    WorldQuest* pQuest = cmd.add_quests();
    if (pQuest == nullptr)
      continue;
    pQuest->set_mapid(m.first);
    pQuest->set_type_main(m.second.bMain);
    pQuest->set_type_branch(m.second.bOther);
    pQuest->set_type_daily(m.second.bDaily);
  }

  if (cmd.quests_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[任务-世界]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询世界任务 :" << cmd.ShortDebugString() << XEND;
  return true;
}

bool Quest::submitQuest(DWORD id)
{
  auto m = m_mapCompleteQuest.find(id);
  if (m == m_mapCompleteQuest.end())
    return false;
  if (m->second->isComplete() == false)
    return false;

  TPtrQuestItem pItem = m->second;

  m_setCompleteUpdateIDs.insert(m->first);
  m_mapCompleteQuest.erase(m);

  const SQuestCFGEx* pCFG = pItem->getQuestCFG();
  if (pCFG != nullptr && canSubmit(pCFG->eType) == true)
  {
    auto o = m_mapSubmitQuest.find(id);
    if (o == m_mapSubmitQuest.end())
    {
      m_mapSubmitQuest[id] = pItem;
      m_setSubmitUpdateIDs.insert(id);
    }
  }

  // 副本任务check
  {
    m_dwRecentSubmitID = id;
    if (m_pUser->getScene() && m_pUser->getScene()->isDScene())
    {
      DScene* pDScene = dynamic_cast<DScene*> (m_pUser->getScene());
      if (pDScene)
        pDScene->getFuben().check("wait");
    }
  }

  DWORD dwBaseExp = 0;
  DWORD dwJobExp = 0;
  // wanted quest
  if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_WANTED && QuestConfig::getMe().isRealWantedQuest(id) == true)
  {
    float fParam = MiscConfig::getMe().getQuestCFG().getWantedParams(m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED));
    const SWantedQuestCFG* pWantedCFG = QuestConfig::getMe().getWantedQuestCFG(m_pUser->getLevel());
    if (pWantedCFG != nullptr)
    {
      const SWantedItem* pWantedItemCFG = pWantedCFG->getWantedItem(id, true);
      if (pWantedItemCFG != nullptr)
      {
        TVecItemInfo vecReward;
        pItem->collectWantedReward(vecReward);

        ItemInfo oItem;
        oItem.set_id(ITEM_GOLD);
        oItem.set_count(static_cast<DWORD>(pWantedItemCFG->dwGold * fParam));
        oItem.set_source(ESOURCE_WANTEDQUEST);
        if (oItem.count() != 0)
          combinItemInfo(vecReward, TVecItemInfo{oItem});

        oItem.set_id(ITEM_ZENY);
        float zenyratio = m_pUser->m_oBuff.getExtraZenyRatio(EEXTRAREWARD_WANTEDQUEST);
        if (zenyratio)
          XLOG << "[回归-Buff], 看板, 额外zeny倍率:" << zenyratio << "玩家:" << m_pUser->name << m_pUser->id << XEND;
        zenyratio += 1;
        oItem.set_count(static_cast<DWORD>(pWantedItemCFG->dwRob * fParam * zenyratio));
        if (oItem.count() != 0)
          combinItemInfo(vecReward, TVecItemInfo{oItem});

        //双倍奖励
        m_pUser->getDoubleReward(EDOUBLEREWARD_WANTEDQUEST, vecReward);

        // 额外奖励/多倍奖励
        DWORD times = 1;
        TVecItemInfo extrareward;
        if (ActivityEventManager::getMe().getReward(m_pUser, EAEREWARDMODE_WANTEDQUEST, m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED) + 1, extrareward, times))
        {
          if (times > 1)
          {
            XLOG << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit quest:" << id << "活动奖励翻倍:" << times << XEND;
            for (auto& v : vecReward)
              v.set_count(v.count() * times);
          }
          if (extrareward.empty() == false)
          {
            XLOG << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit quest:" << id << "活动额外奖励:";
            for (auto& v : extrareward)
              XLOG << v.id() << v.count();
            XLOG << XEND;
            combinItemInfo(vecReward, extrareward);
          }
        }

        m_pUser->getPackage().addItem(vecReward, EPACKMETHOD_AVAILABLE, false, true, true);//note 任务播放3d展示

        XDBG << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "看板系数 :" << fParam << "获得奖励";
        for (auto &v : vecReward)
          XDBG << "item :" << v.ShortDebugString();
        XDBG << XEND;
        //if (pWantedItemCFG->bTeamSync)
        {
          TeamerQuestUpdateSocialCmd teamcmd;
          MemberWantedQuest* pTeamQuest = teamcmd.mutable_quest();
          pTeamQuest->set_action(EQUESTACTION_SUBMIT);
          pTeamQuest->set_charid(m_pUser->id);
          pTeamQuest->set_questid(id);
          PROTOBUF(teamcmd, send2, len2);
          thisServer->sendCmdToSession(send2, len2);
        }
        const GTeam& rTeam = m_pUser->getTeam();
        for (auto &m : rTeam.getTeamMemberList())
        {
          if (m.second.charid() == m_pUser->id)
            continue;
          MsgManager::sendMsg(m.second.charid(), 897, MsgParams(m_pUser->name, pWantedItemCFG->strName));
        }
      }
      else
      {
        XERR << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "提交 questid :" << id << "未获取 lv :" << m_pUser->getLevel() << "等级看板配置中 id :" << id << "的配置,未正确获得奖励" << XEND;
      }
    }
    else
    {
      XERR << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "提交 questid :" << id << "未获取 lv :" << m_pUser->getLevel() << "等级看板配置,未正确获得奖励" << XEND;
    }

    DWORD acceptlv = pItem->getAcceptLv() ? pItem->getAcceptLv() : m_pUser->getLevel();
    const SDailyExpPool* pExpPool = QuestConfig::getMe().getExpPool(acceptlv);
    if (pExpPool != nullptr)
    {
      float expratio = m_pUser->m_oBuff.getExtraExpRatio(EEXTRAREWARD_WANTEDQUEST);
      if (expratio)
        XLOG << "[回归-Buff], 看板, 额外经验倍率:" << expratio << "玩家:" << m_pUser->name << m_pUser->id << XEND;
      expratio += 1;

      m_pUser->addBaseExp(static_cast<DWORD>(pExpPool->dwWantedBaseExp * fParam * expratio), ESOURCE_DAILYQUEST);
      m_pUser->addJobExp(static_cast<DWORD>(pExpPool->dwWantedJobExp * fParam * expratio), ESOURCE_DAILYQUEST);
      dwBaseExp += pExpPool->dwWantedBaseExp * fParam;
      dwJobExp += pExpPool->dwWantedJobExp * fParam;
      MsgManager::sendMsg(m_pUser->id, 17, MsgParams(pExpPool->dwWantedBaseExp * fParam, pExpPool->dwWantedJobExp * fParam));
    }

    removeQuestDetail(pCFG->id / QUEST_ID_PARAM);
    m_pUser->getVar().setVarValue(EVARTYPE_QUEST_WANTED, m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED) + 1);
    m_pUser->getServant().onFinishEvent(ETRIGGER_WANTED_QUEST_DAY);
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_WANTED_QUEST_DAY);
    // 立即更新var -> client
    m_pUser->updateVar();
    m_pUser->refreshDataAtonce();

    // 获得信用度
    const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
    if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED) <= rCFG.dwWQuestTimes)
      m_pUser->getUserSceneData().addCredit(rCFG.dwWQuestValue);

    // 获得额外奖励
    m_pUser->getExtraReward(EEXTRAREWARD_WANTEDQUEST);
    m_pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_WANTED,m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED));

    m_pUser->m_oBuff.onFinishEvent(EEXTRAREWARD_WANTEDQUEST);
    //完成看板任务
    //platlog 
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Complete_Board;
    PlatLogManager::getMe().eventLog(thisServer,
        m_pUser->getUserSceneData().getPlatformId(),
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eid,
        m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().CompleteLog(thisServer,
        m_pUser->getUserSceneData().getPlatformId(),
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eType,
        eid,
        ECompleteType_Board,
        id,
        m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED),/*今天完成次数*/
        1,/*base exp*/
        dwBaseExp,
        m_pUser->getLevel());

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_WANT_QUEST_COUNT, 0, 0, m_pUser->getLevel(), (DWORD) 1);
    DWORD weekCnt = m_pUser->getVar().getAccVarValue(EACCVARTYPE_QUEST_WANTED_WEEK);
    weekCnt++;
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_QUEST_WANTED_WEEK, weekCnt);
    if (weekCnt >= 5)
      m_pUser->stopSendInactiveLog();
  }
  else if(pCFG != nullptr && pCFG->eType == EQUESTTYPE_WANTED && QuestConfig::getMe().isRealWantedQuest(id) == false)
  {
    const SWantedQuestCFG* pWantedCFG = QuestConfig::getMe().getActivityWantedQuestCFG(m_pUser->getLevel());
    if (pWantedCFG != nullptr)
    {
      const SWantedItem* pWantedItemCFG = pWantedCFG->getWantedItem(id, true);
      if (pWantedItemCFG != nullptr)
      {
        TVecItemInfo vecReward;
        pItem->collectWantedReward(vecReward);

        if(pWantedItemCFG->dwRob != 0)
        {
          ItemInfo oItem;
          oItem.set_id(ITEM_ZENY);
          oItem.set_source(ESOURCE_WANTEDQUEST);
          oItem.set_count(static_cast<DWORD>(pWantedItemCFG->dwRob));
          combinItemInfo(vecReward, TVecItemInfo{oItem});
        }

        m_pUser->getPackage().addItem(vecReward, EPACKMETHOD_AVAILABLE, false, true, true);//note 任务播放3d展示

        XDBG << "[任务-提交] 活动看板" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id: " << id << "获得奖励";
        for (auto &v : vecReward)
          XDBG << "item :" << v.ShortDebugString();
        XDBG << XEND;
      }
      else
      {
        XERR << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "提交 questid :" << id << "未获取 lv :" << m_pUser->getLevel() << "等级看板配置中 id :" << id << "的配置,未正确获得奖励" << XEND;
      }
    }
    else
    {
      XERR << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "提交 questid :" << id << "未获取 lv :" << m_pUser->getLevel() << "等级看板配置,未正确获得奖励" << XEND;
    }
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_DAILY)
  {
    // add count first to prevent player levelup to accept daily quest again
    addDailyCount();

    // 完成抗击魔潮，额外奖励
    m_pUser->getExtraReward(EEXTRAREWARD_DAILYMONSTER);
    m_pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_RESIST, getDailyCount());

    m_pUser->getServant().onFinishEvent(ETRIGGER_DAILYMONSTER);
    if (m_pExpPool != nullptr)
    {
      m_pUser->addBaseExp(m_pExpPool->dwSubmitBaseExp, ESOURCE_DAILYQUEST);
      m_pUser->addJobExp(m_pExpPool->dwSubmitJobExp, ESOURCE_DAILYQUEST);
      dwBaseExp += m_pExpPool->dwSubmitBaseExp;
      dwJobExp += m_pExpPool->dwSubmitJobExp;

      ItemInfo oItem;
      oItem.set_id(100);
      oItem.set_count(m_pExpPool->dwWantedRob);
      oItem.set_source(ESOURCE_DAILYQUEST);
      m_pUser->getPackage().addItem(oItem, EPACKMETHOD_AVAILABLE, false, true);
    }
    clearDaily();
    queryOtherData(EOTHERDATA_DAILY);

    //完成抗击魔潮
    //platlog 
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Complete_DailyQuest;
    PlatLogManager::getMe().eventLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eid,
      m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().CompleteLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eType,
      eid,
      ECompleteType_DailyQuest,
      id,
      m_dwDailyCount, /*今天完成次数*/
      1,/*base exp*/
      dwBaseExp,
      m_pUser->getLevel());  //奖励没输出

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_DAILY_COUNT, 0, 0, m_pUser->getLevel(), (DWORD)1);
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_GUILD)
  {
    DWORD oldvalue = m_pUser->getVar().getVarValue(EVARTYPE_GUILD_QUEST);
    m_pUser->getVar().setVarValue(EVARTYPE_GUILD_QUEST, oldvalue + 1);
    m_pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_GUILD_TASK, oldvalue + 1);
    m_pUser->getExtraReward(EEXTRAREWARD_GUILD_QUEST);
    m_pUser->stopSendInactiveLog();
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_CHILD)
  {
    if (QuestManager::getMe().isQuestComplete(id / QUEST_ID_PARAM, m_pUser) == true)
      m_pUser->getVar().setVarValue(EVARTYPE_CHILD_QUEST, 1);
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_ACC)
  {
    if (QuestManager::getMe().isQuestComplete(id / QUEST_ID_PARAM, m_pUser) == true)
      m_pUser->getVar().setVarValue(EVARTYPE_ACC_QUEST, 1);
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_DAILY_MAPRAND)
  {
    removeDailyMapRandQuest(pCFG->mapid);
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_ARTIFACT)
  {
    const SArtifactCFG* pCFG = GuildConfig::getMe().getArtifactCFGByQuestID(id);
    if (pCFG != nullptr)
    {
      stringstream sstr;
      sstr << "guild cmd=quest action=update id=" << id;
      GMCommandRuler::getMe().execute(m_pUser, sstr.str());
      XLOG << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成公会任务" << id << XEND;
    }
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_SIGN)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_QUEST_SIGN, 1);
  }
  else if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_WEDDING_DAILY)
  {
    if (QuestManager::getMe().isQuestComplete(id / QUEST_ID_PARAM, m_pUser) == true)
      m_pUser->getVar().setVarValue(EVARTYPE_QUEST_WEDDINGDAILY, 1);
  }

  updateBigQuestComplete(id);
  update();

  m_pUser->getEvent().onQuestSubmit(id);
  XLOG << "[任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit quest :" << id << XEND;

  if(pCFG != nullptr && pCFG->eRefreshType != EQUESTREFRESH_MIN)
  {
    addActivityFinishTimes(id);
    auto it = m_mapActivityQuest.find(id);
    if(it != m_mapActivityQuest.end() && it->second.finishcount() < pCFG->refreshTimes)
      deleteQuest(id);
  }

  delQuestFromTimer(id);

  //log 
  std::string rewardItem;
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Quest_Stop;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  PlatLogManager::getMe().QuestLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    id,
    EQuestType_STOP,
    0,
    dwBaseExp,
    dwJobExp,
    rewardItem,
    m_pUser->getLevel());
  return true;
}

bool Quest::addQuestStep(DWORD id)
{
  auto m = m_mapAcceptQuest.find(id);
  if (m == m_mapAcceptQuest.end())
    return false;

  // add detailid
  addQuestDetail(m->second->getStepCFG());

  bool deltimer = false;
  if (m->second->getStepType() == EQUESTSTEP_WAIT || m->second->getStepType() == EQUESTSTEP_TUTOR)
    deltimer = true;

  // add step
  DWORD dwOldSubGroup = m->second->getStepCFG()->getSubGroup();
  if (m->second->addStep() == false)
    return false;

  if (deltimer)
    delQuestFromTimer(id);

  // check complete
  if (m->second->isNoStep() == true)
  {
    TPtrQuestItem pItem = m->second;
    m_mapAcceptQuest.erase(m);
    m_setUpdateIDs.insert(id);

    pItem->setComplete(true);
    m_mapCompleteQuest[id] = pItem;
    m_setCompleteUpdateIDs.insert(id);

    // submit quest
    const SQuestCFGEx* pCFG = pItem->getQuestCFG();
    if (pCFG != nullptr && pCFG->eType != EQUESTTYPE_WANTED)
      submitQuest(id);
    else
      update();
  }
  else
  {
    if (m->second->getStepType() == EQUESTSTEP_WAIT || m->second->getStepType() == EQUESTSTEP_TUTOR)
      addQuestToTimer(id);

    DWORD dwCurSubGroup = m->second->getStepCFG()->getSubGroup();
    if (dwOldSubGroup != dwCurSubGroup)
    {
      if (finishQuest(id) == true)
      {
        m_pUser->getEvent().onQuestSubmit(id);
        acceptNewQuest();
      }
    }
    else
    {
      stepUpdate(m->first);
      const SQuestCFG* pCFG = m->second->getQuestCFG();
      if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_DAILY_MAPRAND)
        removeDailyMapRandQuest(pCFG->mapid);
    }
  }

  return true;
}

bool Quest::setQuestStep(DWORD id, DWORD step)
{
  auto m = m_mapAcceptQuest.find(id);
  if (m == m_mapAcceptQuest.end())
    return false;
  if (step == 0 && m->second->getStep() == step)
    return false;
  bool deltimer = false;
  if (m->second->getStepType() == EQUESTSTEP_WAIT || m->second->getStepType() == EQUESTSTEP_TUTOR)
    deltimer = true;
  if (m->second->setStep(step) == false)
    return false;

  if (m->second->getStepType() == EQUESTSTEP_WAIT || m->second->getStepType() == EQUESTSTEP_TUTOR)
    addQuestToTimer(id);
  else if (deltimer)
    delQuestFromTimer(id);

  const SQuestCFG* pCFG = m->second->getQuestCFG();
  if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_DAILY_MAPRAND)
    removeDailyMapRandQuest(pCFG->mapid);

  stepUpdate(m->first);

  return true;
}

bool Quest::addForbidQuest(DWORD dwQuestID)
{
  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(dwQuestID);
  if (pCFG == nullptr)
  {
    XERR << "[任务-禁止任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加禁止id :" << dwQuestID << "失败,未在 Table_Quest.txt 表中找到" << XEND;
    return false;
  }

  auto s = m_setForbidQuest.find(dwQuestID);
  if (s == m_setForbidQuest.end())
  {
    m_setForbidQuest.insert(dwQuestID);

    auto accept = m_mapAcceptQuest.find(dwQuestID);
    if (accept != m_mapAcceptQuest.end())
    {
      m_mapAcceptQuest.erase(accept);
      m_setUpdateIDs.insert(dwQuestID);
      XLOG << "[任务-禁止任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加禁止id :" << dwQuestID << "成功,从accept列表中移除" << XEND;
    }

    auto complete = m_mapCompleteQuest.find(dwQuestID);
    if (complete != m_mapCompleteQuest.end())
    {
      m_mapCompleteQuest.erase(complete);
      m_setCompleteUpdateIDs.insert(dwQuestID);
      XLOG << "[任务-禁止任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加禁止id :" << dwQuestID << "成功,从complete列表中移除" << XEND;
    }

    auto submit = m_mapSubmitQuest.find(dwQuestID);
    if (submit != m_mapSubmitQuest.end())
    {
      m_mapSubmitQuest.erase(submit);
      m_setSubmitUpdateIDs.insert(dwQuestID);
      XLOG << "[任务-禁止任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加禁止id :" << dwQuestID << "成功,从submit列表中移除" << XEND;
    }
  }

  XLOG << "[任务-禁止任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加禁止id :" << dwQuestID << "成功" << XEND;
  return true;
}


bool Quest::removeForbidQuest(DWORD dwQuestID)
{
  auto s = m_setForbidQuest.find(dwQuestID);
  if (s != m_setForbidQuest.end())
  {
    m_setForbidQuest.erase(s);
    acceptNewQuest();
    XLOG << "[任务-禁止任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "移除禁止id :" << dwQuestID << "成功" << XEND;
    return true;
  }

  XLOG << "[任务-禁止任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "移除禁止id :" << dwQuestID << "成功,该id未在禁止列表中" << XEND;
  return true;
}

bool Quest::addChoiceQuest(DWORD dwQuestID)
{
  auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [&](const QuestDetailData& r) -> bool{
    return dwQuestID == r.getID();
  });
  if (v != m_vecDetails.end())
  {
    XERR << "[任务-选择任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加选择id :" << dwQuestID << "失败,该id已添加" << XEND;
    return false;
  }

  QuestDetail oData;
  QuestDetailData detail(oData);
  detail.setID(dwQuestID);
  detail.setTime(xTime::getCurSec());
  m_vecDetails.push_back(detail);

  m_pUser->getVar().setVarValue(EVARTYPE_ACC_CHOICE_QUEST, 1);
  XLOG << "[任务-选择任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加选择id :" << dwQuestID << "成功" << XEND;
  return true;
}

bool Quest::removeChoiceQuest(DWORD dwQuestID)
{
  auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [&](const QuestDetailData& r) -> bool{
    return dwQuestID == r.getID();
  });
  if (v != m_vecDetails.end())
  {
    XERR << "[任务-选择任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "移除选择id :" << dwQuestID << "失败,未存在" << XEND;
    return false;
  }

  m_vecDetails.erase(v);
  XLOG << "[任务-选择任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "移除选择id :" << dwQuestID << "成功" << XEND;
  return true;
}

bool Quest::processQuest(EQuestMethod eMethod, DWORD dwQuest)
{
  if (eMethod == EQUESTMETHOD_DEL_ACCEPT)
  {
    auto m = m_mapAcceptQuest.find(dwQuest);
    if (m != m_mapAcceptQuest.end())
    {
      XLOG << "[任务-处理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "accept :" << dwQuest << "被移除" << XEND;
      m_mapAcceptQuest.erase(m);
    }
  }
  else if (eMethod == EQUESTMETHOD_ADD_SUBMIT)
  {
    auto m = m_mapSubmitQuest.find(dwQuest);
    if (m == m_mapSubmitQuest.end())
    {
      const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(dwQuest);
      if (pCFG == nullptr)
      {
        XERR << "[任务-处理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit :" << dwQuest << "添加失败,未在 Table_Quest.txt 表中找到" << XEND;
        return false;
      }

      QuestData oData;
      oData.set_id(dwQuest);

      TPtrQuestItem pItem( new QuestItem(oData, pCFG));
      m_mapSubmitQuest[dwQuest] = pItem;

      XLOG << "[任务-处理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit :" << dwQuest << "添加成功" << XEND;
    }
  }
  else
  {
    XERR << "[任务-处理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "method :" << eMethod << "未知类型" << XEND;
    return false;
  }

  return true;
}

bool Quest::stepUpdate(DWORD id)
{
  if (m_pUser == nullptr)
    return false;

  if (canUpdate(id) == false)
    return false;

  auto m = m_mapAcceptQuest.find(id);
  if (m == m_mapAcceptQuest.end())
    return false;

  QuestStep* pStep = m->second->getStepData();
  TPtrBaseStep step = m->second->getStepCFG();
  if (pStep == nullptr || step == nullptr)
    return false;

  TVecString vecSParams;
  TVecQWORD vecQParams;
  if (step->getStepType() == EQUESTSTEP_VISIT || step->getStepType() == EQUESTSTEP_DIALOG)
  {
    m->second->collectSParams(vecSParams);
    m->second->collectQParams(vecQParams);

    if (vecSParams.empty() == false)
    {
      pStep->clear_names();
      for (auto &v : vecSParams)
        pStep->add_names(v);
    }
    if (vecQParams.empty() == false)
    {
      pStep->clear_params();
      for (auto &v : vecQParams)
        pStep->add_params(v);
    }
  }

  QuestStepUpdate cmd;
  cmd.set_id(m->first);
  cmd.set_step(m->second->getStep());
  cmd.mutable_data()->CopyFrom(*pStep);
  if (m->second->getQuestCFG() == nullptr || m->second->getQuestCFG()->toStepConfig(cmd.mutable_data(), cmd.step()) == false)
    XERR << "[任务-步骤更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步步骤" << cmd.ShortDebugString() << "配置初始化失败" << XEND;

  DialogStep* pDialogStep = dynamic_cast<DialogStep*>(step.get());
  if (pDialogStep != nullptr && pDialogStep->isDynamic() == true && vecQParams.empty() == false)
    pDialogStep->toDynamicConfig(cmd.mutable_data(), vecQParams.back());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  if (step->isSyncTeamWanted())
  {
    TeamerQuestUpdateSocialCmd teamcmd;
    MemberWantedQuest* pTeamQuest = teamcmd.mutable_quest();
    pTeamQuest->set_action(EQUESTACTION_ACCEPT);
    pTeamQuest->set_charid(m_pUser->id);
    pTeamQuest->set_questid(m->first);
    pTeamQuest->set_step(m->second->getStep());
    PROTOBUF(teamcmd, send2, len2);
    thisServer->sendCmdToSession(send2, len2);
  }
  XDBG << "[任务-步骤更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步步骤" << cmd.ShortDebugString() << XEND;
  return true;
}

bool Quest::canUpdate(DWORD id)
{
  if (m_pUser == nullptr)
    return false;

  auto m = m_mapAcceptQuest.find(id);
  if (m == m_mapAcceptQuest.end())
    return false;

  static const set<EQuestStep> setStep = {  EQUESTSTEP_VISIT, EQUESTSTEP_KILL, EQUESTSTEP_COLLECT, EQUESTSTEP_USE, EQUESTSTEP_GATHER, EQUESTSTEP_RAID, EQUESTSTEP_LEVEL,
                                            EQUESTSTEP_MOVE, EQUESTSTEP_DIALOG, EQUESTSTEP_SELFIE, EQUESTSTEP_PURIFY, EQUESTSTEP_ACTION, EQUESTSTEP_SKILL, EQUESTSTEP_INTERLOCUTION,
                                            EQUESTSTEP_GUIDE, EQUESTSTEP_GUIDE_HIGHLIGHT, EQUESTSTEP_SEAL, EQUESTSTEP_EQUIPLV, EQUESTSTEP_VIDEO,
                                            EQUESTSTEP_ILLUSTRATION, EQUESTSTEP_ITEM, EQUESTSTEP_DAILY, EQUESTSTEP_MANUAL, EQUESTSTEP_GUIDELOCKMONSTER, EQUESTSTEP_MONEY, EQUESTSTEP_OPTION,
                                            EQUESTSTEP_PHOTO, EQUESTSTEP_ITEMUSE, EQUESTSTEP_HAND, EQUESTSTEP_MUSIC, EQUESTSTEP_CARRIER, EQUESTSTEP_BATTLE, EQUESTSTEP_PET, EQUESTSTEP_COOKFOOD,
                                            EQUESTSTEP_COOK, EQUESTSTEP_BUFF, EQUESTSTEP_TUTOR, EQUESTSTEP_CHECKGROUP, EQUESTSTEP_RAND_DIALOG, EQUESTSTEP_CG, EQUESTSTEP_CLIENTPLOT, EQUESTSTEP_CHAT, EQUESTSTEP_TRANSFER
                                         };
  EQuestStep eStep = m->second->getStepType();
  auto s = setStep.find(eStep);
  if (s == setStep.end())
    return false;
  return true;
}

bool Quest::canStep(DWORD id)
{
  if (m_pUser == nullptr)
    return false;

  auto m = m_mapAcceptQuest.find(id);
  if (m == m_mapAcceptQuest.end())
    return false;

  static const set<EQuestStep> setStep = {  EQUESTSTEP_VISIT, EQUESTSTEP_USE, EQUESTSTEP_WAIT, EQUESTSTEP_MOVE, EQUESTSTEP_DIALOG, EQUESTSTEP_SELFIE,
                                            EQUESTSTEP_PURIFY, EQUESTSTEP_ACTION, EQUESTSTEP_GUIDE, EQUESTSTEP_GUIDE_HIGHLIGHT, EQUESTSTEP_VIDEO, EQUESTSTEP_ILLUSTRATION,
                                            EQUESTSTEP_GUIDELOCKMONSTER, EQUESTSTEP_SCENE, EQUESTSTEP_CG, EQUESTSTEP_CLIENTPLOT,
                                         };
  EQuestStep eStep = m->second->getStepType();
  auto s = setStep.find(eStep);
  if (s != setStep.end())
    return false;

  return true;
}

bool Quest::checkStep(DWORD id, DWORD step)
{
  if (step == 0)
    return true;

  auto m = m_mapAcceptQuest.find(id);
  if (m == m_mapAcceptQuest.end())
    return false;

  return m->second->getStep() == step;
}

DWORD Quest::getQuestStep(DWORD questid) const
{
  auto m = m_mapAcceptQuest.find(questid);
  if (m == m_mapAcceptQuest.end())
    return false;
  return m->second->getStep();
}

bool Quest::canSubmit(EQuestType eType) const
{
  static EQuestType types[] = { EQUESTTYPE_MAIN, EQUESTTYPE_BRANCH, EQUESTTYPE_STORY, EQUESTTYPE_WANTED, EQUESTTYPE_DAILY_1, EQUESTTYPE_DAILY_3, EQUESTTYPE_DAILY_7, EQUESTTYPE_DAILY_MAP,
                                EQUESTTYPE_SCENE, EQUESTTYPE_HEAD, EQUESTTYPE_SATISFACTION, EQUESTTYPE_ELITE, EQUESTTYPE_CCRASTEHAM, EQUESTTYPE_STORY_CCRASTEHAM, EQUESTTYPE_GUILD,
                                EQUESTTYPE_CHILD, EQUESTTYPE_DAILY_RESET, EQUESTTYPE_ACC, EQUESTTYPE_ACC_NORMAL, EQUESTTYPE_ACC_DAILY, EQUESTTYPE_ACC_CHOICE,
                                EQUESTTYPE_ACC_MAIN, EQUESTTYPE_ACC_BRANCH, EQUESTTYPE_ACC_SATISFACTION, EQUESTTYPE_ACC_DAILY_1, EQUESTTYPE_ACC_DAILY_RESET, EQUESTTYPE_DAILY_MAPRAND,
                                EQUESTTYPE_DAILY_BOX, EQUESTTYPE_SIGN, EQUESTTYPE_DAY, EQUESTTYPE_NIGHT, EQUESTTYPE_ARTIFACT, EQUESTTYPE_WEDDING, EQUESTTYPE_WEDDING_DAILY, EQUESTTYPE_CAPRA, EQUESTTYPE_DEAD };
  for (EQuestType e : types)
  {
    if (eType == e)
      return true;
  }

  return false;
}

bool Quest::canDetail(EQuestType eType) const
{
  static EQuestType types[] = {
    EQUESTTYPE_GUILD, EQUESTTYPE_DAILY_RESET, EQUESTTYPE_ACC_DAILY, EQUESTTYPE_DAILY_1, EQUESTTYPE_DAILY_3, EQUESTTYPE_DAILY_7, EQUESTTYPE_DAILY_MAP,
    EQUESTTYPE_ACC_CHOICE, EQUESTTYPE_DAILY_MAPRAND, EQUESTTYPE_DAILY_BOX, EQUESTTYPE_DAILY_RESET, EQUESTTYPE_ACC_DAILY_RESET
  };
  for (EQuestType e : types)
  {
    if (eType == e)
      return false;
  }

  return true;
}

bool Quest::finishBoardQuest(DWORD id, bool skipLastStep)
{
  if (canFinishBoard(id, skipLastStep) == false)
    return false;

  if (skipLastStep == true)
  {
    auto it = m_mapAcceptQuest.find(id);
    if (it == m_mapAcceptQuest.end())
      return false;

    TPtrQuestItem pItem = it->second;
    m_mapAcceptQuest.erase(it);
    m_setUpdateIDs.insert(id);

    pItem->setComplete(true);
    m_mapCompleteQuest[id] = pItem;
    m_setCompleteUpdateIDs.insert(id);
  }

  bool ret = submitQuest(id);
  if (ret)
    XLOG << "[组队-看板完成], 玩家:" << m_pUser->name << m_pUser->id << "任务:" << id << XEND;
  return ret;
}

bool Quest::canFinishBoard(DWORD id, bool skipLastStep)
{
  if (isSubmit(id) == true)
    return false;

  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(id);
  if (pCFG == nullptr)
    return false;

  if (pCFG->eType != EQUESTTYPE_WANTED)
    return false;

  if (skipLastStep == true)
  {
    auto it = m_mapAcceptQuest.find(id);
    if (it == m_mapAcceptQuest.end())
      return false;

    TPtrBaseStep pStep = it->second->getStepCFG();
    if (pStep == nullptr || pStep->isTeamCanFinish() == false)
      return false;
  }

  return true;
}

void Quest::inviteFinishBoard(DWORD id, bool isquickfinish/* = false*/)
{
  if (m_pUser->getTeamID() == 0 || m_pUser->getTeamLeaderID() != m_pUser->id)
    return;

  // check skill
  if (m_pUser->getFighter() == nullptr)
    return;
  if (m_pUser->getFighter()->getSkill().isSkillEnable(50040001) == false)
    return;

  InviteAcceptQuestCmd cmd;
  cmd.set_leaderid(m_pUser->id);
  cmd.set_questid(id);
  cmd.set_leadername(m_pUser->name);
  cmd.set_issubmit(true);
  cmd.set_isquickfinish(isquickfinish);
  DWORD time = now() + 300;
  cmd.set_time(time);

  char sign[1024];
  bzero(sign, sizeof(sign));
  std::stringstream ss;
  ss << m_pUser->id << "@" << id << "@" << time << "_" << "#$%^&";
  upyun_md5(ss.str().c_str(), ss.str().size(), sign);
  cmd.set_sign(sign);
  PROTOBUF(cmd, send, len);
  for (auto &m : m_pUser->getTeam().getTeamMemberList())
  {
    if (m.first == m_pUser->id)
      continue;
    thisServer->forwardCmdToSceneUser(m.first, send, len);
  }
}

void Quest::checkQuestVersion(bool bLoad /*= false*/)
{
  set<TPtrQuestItem> setItems;
  for (auto &m : m_mapAcceptQuest)
  {
    const SQuestCFG* pCFG = m.second->getQuestCFG();
    if (pCFG != nullptr && pCFG->version == m.second->getVersion())
      continue;
    setItems.insert(m.second);
  }

  bool bRefresh = setItems.empty() == false;
  bool bClearDaily = false;
  for (auto &s : setItems)
  {
    if (s == nullptr)
      continue;
    TPtrQuestItem pItem = s;
    const SQuestCFG* pCFG = pItem->getQuestCFG();

    if (pCFG != nullptr)
    {
      if (QuestConfig::getMe().isAccQuest(pCFG->eType) == true && pItem->getStep() > 0 && m_setProcessAccQuest.find(pCFG->id) != m_setProcessAccQuest.end())
      {
        XLOG << "[任务-版本检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pItem->getID() << "版本发生变化,任务为acc任务,标记被重置" << XEND;
        m_setProcessAccQuest.erase(pCFG->id);
      }

      if (pCFG->eType == EQUESTTYPE_DEAD)
      {
        XLOG << "[任务-版本检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pItem->getID() << "版本发生变化,任务为dead任务,标记被重置" << XEND;
        m_setDeadIDs.erase(pCFG->id);
      }
      else if (pItem->getStepType() == EQUESTSTEP_DAILY)
      {
        XLOG << "[任务-版本检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pItem->getID() << "版本发生变化,当前步骤正为daily,抗击魔潮进行刷新" << XEND;
        bClearDaily = true;
      }
      else if (pCFG->eType == EQUESTTYPE_DAILY_MAPRAND)
      {
        const SDailyPerDay* pQuestCFG = MiscConfig::getMe().getQuestCFG().getDailyCount(pCFG->mapid);
        if (pQuestCFG != nullptr)
        {
          XLOG << "[任务-版本检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pItem->getID() << "版本发生变化,任务为daily_maprand,标记被重置" << XEND;
          TSetDWORD& setIDs = m_mapMapRandQuest[pCFG->mapid];
          setIDs.erase(pCFG->id);
        }
      }
      XLOG << "[任务-版本检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pItem->getID() << "版本号不同,重新做old :" << pItem->getVersion() << "new :" << pCFG->version << XEND;
    }

    abandonGroup(pItem->getID(), false, !bLoad);
  }

  if (bClearDaily)
  {
    m_dwDailyCurExp = 0;
    m_pExpPool = nullptr;
  }
  if (bRefresh && !bLoad)
    acceptNewQuest();
}

bool Quest::finishQuest(DWORD id)
{
  if (isSubmit(id) == true)
    return false;

  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(id);
  if (pCFG == nullptr)
    return false;

  const SWantedItem* pWantedItem = nullptr;
  if (pCFG->eType == EQUESTTYPE_WANTED)
  {
    const SWantedQuestCFG* pWantedCFG = QuestConfig::getMe().getWantedQuestCFG(m_pUser->getLevel());
    if (pWantedCFG == nullptr)
      return false;
    pWantedItem = pWantedCFG->getWantedItem(id);
    if (pWantedItem == nullptr)
      return false;
  }

  TPtrQuestItem pItem = nullptr;
  auto m = m_mapAcceptQuest.find(id);
  if (m != m_mapAcceptQuest.end())
  {
    pItem = m->second;
    m_mapAcceptQuest.erase(m);
    m_setUpdateIDs.insert(id);
  }
  if (pItem == nullptr)
  {
    QuestData oData;
    if (QuestManager::getMe().createQuestData(&oData, pCFG, m_pUser, pWantedItem) == false)
      return false;
    pItem = TPtrQuestItem( new QuestItem(oData, pCFG));
  }

  while (pItem->isNoStep() == false)
    pItem->addStep();

  pItem->setComplete(true);
  m_mapCompleteQuest[id] = pItem;
  return submitQuest(id);
}

bool Quest::finishBigQuest(DWORD id, bool bRefresh /*= true*/)
{
  const TVecDWORD& vecIDs = QuestConfig::getMe().getQuestDetail(id);
  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    if (finishQuest(*v) == false)
      XERR << "[任务-完成大任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "任务 id :" << *v << "完成失败" << XEND;
  }

  if (bRefresh)
    acceptNewQuest();
  return true;
}

bool Quest::addQuestDetail(const TPtrBaseStep pStep)
{
  if (pStep == nullptr)
    return false;

  const SQuestCFG* pCFG = QuestManager::getMe().getQuestCFG(pStep->getQuestID());
  if (pCFG == nullptr || canDetail(pCFG->eType) == false)
    return false;

  DWORD questid = pStep->getQuestID() / QUEST_ID_PARAM;
  auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [questid](const QuestDetailData& r) -> bool{
    return questid == r.getID();
  });
  if (v == m_vecDetails.end())
  {
    QuestDetail oData;
    QuestDetailData detail(oData);
    detail.setID(questid);
    detail.setTime(xTime::getCurSec());
    detail.setMap(pStep->getMapID());
    detail.setComplete(false);
    detail.setTrace(true);
    detail.addDetailID(pStep->getDetailID());

    m_vecDetails.push_back(detail);
    v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [questid](const QuestDetailData& r) -> bool{
      return questid == r.getID();
    });
  }
  else
  {
    v->addDetailID(pStep->getDetailID());
  }
  if (v == m_vecDetails.end())
    return false;

  m_setDetailIDs.insert(v->getID());

  return true;
}

bool Quest::removeQuestDetail(DWORD id)
{
  auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [id](const QuestDetailData& r) -> bool{
    return id == r.getID();
  });
  if (v == m_vecDetails.end())
    return false;

  m_vecDetails.erase(v);

  m_setDetailIDs.insert(id);
  return true;
}

bool Quest::updateBigQuestComplete(DWORD id)
{
  DWORD questid = id / QUEST_ID_PARAM;
  auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [questid](const QuestDetailData& r) -> bool{
    return questid == r.getID();
  });
  if (v == m_vecDetails.end())
    return false;

  if (v->getComplete() == true)
    return false;

  v->setComplete(QuestManager::getMe().isQuestComplete(questid, m_pUser));
  m_setDetailIDs.insert(questid);

  if (v->getComplete() == true)
    m_pUser->getQuestNpc().delNpcQuest(questid);
  return true;
}

bool Quest::removeQuest(EQuestList eList, EQuestType eType)
{
  TMapQuest* pMapQuest = nullptr;
  TSetDWORD* pSetUpdateIDs = nullptr;

  if (eList == EQUESTLIST_ACCEPT)
  {
    pMapQuest = &m_mapAcceptQuest;
    pSetUpdateIDs = &m_setUpdateIDs;
  }
  else if (eList == EQUESTLIST_COMPLETE)
  {
    pMapQuest = &m_mapCompleteQuest;
    pSetUpdateIDs = &m_setCompleteUpdateIDs;
  }
  else if (eList == EQUESTLIST_SUBMIT)
  {
    pMapQuest = &m_mapSubmitQuest;
    pSetUpdateIDs = &m_setSubmitUpdateIDs;
  }

  if (pMapQuest == nullptr || pSetUpdateIDs == nullptr)
  {
    XERR << "[任务-删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "删除 list :" << eList << "type :" << eType << "失败,获取到列表" << XEND;
    return false;
  }

  for (auto m = pMapQuest->begin(); m != pMapQuest->end();)
  {
    if (m->second == nullptr)
    {
      ++m;
      continue;
    }
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->eType != eType)
    {
      ++m;
      continue;
    }
    pSetUpdateIDs->insert(m->first);
    m = pMapQuest->erase(m);
  }

  if (pSetUpdateIDs->empty() == true)
  {
    XDBG << "[任务-删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "删除 list :" << eList << "type :" << eType << "成功,但未找到可删除任务" << XEND;
    return false;
  }

  update();
  XLOG << "[任务-删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "删除 list :" << eList << "type :" << eType << "成功,任务" << pSetUpdateIDs << "被删除" << XEND;
  return true;
}

void Quest::update()
{
  QuestUpdate cmd;
  auto initlist = [&](EQuestList eType)
  {
    TSetDWORD* pUpdateIDs = nullptr;
    TMapQuest* mapQuest = nullptr;

    if (eType == EQUESTLIST_ACCEPT)
    {
      pUpdateIDs = &m_setUpdateIDs;
      mapQuest = &m_mapAcceptQuest;
    }
    else if (eType == EQUESTLIST_COMPLETE)
    {
      pUpdateIDs = &m_setCompleteUpdateIDs;
      mapQuest = &m_mapCompleteQuest;
    }
    else if (eType == EQUESTLIST_SUBMIT)
    {
      pUpdateIDs = &m_setSubmitUpdateIDs;
      mapQuest = &m_mapSubmitQuest;
    }
    else
    {
      return;
    }

    if (pUpdateIDs == nullptr || mapQuest == nullptr)
      return;

    if (pUpdateIDs->empty() == true)
      return;

    QuestUpdateItem* pItem = cmd.add_items();
    pItem->set_type(eType);
    for (auto &s : *pUpdateIDs)
    {
      auto m = mapQuest->find(s);
      if (m != mapQuest->end())
      {
        QuestData* pData = pItem->add_update();
        if (pData != nullptr)
          m->second->toClientData(pData);
        continue;
      }

      pItem->add_del(s);
    }

    pUpdateIDs->clear();
  };

  initlist(EQUESTLIST_ACCEPT);
  initlist(EQUESTLIST_COMPLETE);
  initlist(EQUESTLIST_SUBMIT);

  if (cmd.items_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);

#ifdef _DEBUG
    for (int i = 0; i < cmd.items_size(); ++i)
    {
      string type;
      if (cmd.items(i).type() == EQUESTLIST_ACCEPT)
        type = "\b已接";
      if (cmd.items(i).type() == EQUESTLIST_COMPLETE)
        type = "\b完成";
      if (cmd.items(i).type() == EQUESTLIST_SUBMIT)
        type = "\b提交";

      for (int j = 0; j < cmd.items(i).update_size(); ++j)
        XDBG << "[任务同步-" << type << "\b]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :更新" << cmd.items(i).update(j).ShortDebugString() << XEND;
      for (int j = 0; j < cmd.items(i).del_size(); ++j)
        XDBG << "[任务同步-" << type << "\b]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :删除" << cmd.items(i).del(j) << XEND;
    }
#endif
  }
}

void Quest::detailupdate()
{
  if (m_setDetailIDs.empty() == true)
    return;

  QuestDetailUpdate cmd;
  for (auto s = m_setDetailIDs.begin(); s != m_setDetailIDs.end(); ++s)
  {
    DWORD dwQuestID = *s;
    auto v = find_if(m_vecDetails.begin(), m_vecDetails.end(), [dwQuestID](const QuestDetailData& r) -> bool{
      return dwQuestID == r.getID();
    });
    if (v != m_vecDetails.end())
    {
      QuestDetail* detail = cmd.add_detail();
      if (detail != nullptr)
        v->toData(detail);
    }
    else
    {
      QuestDetail* detail = cmd.add_del();
      if (detail != nullptr)
        detail->set_id(*s);
    }
  }

  if (cmd.detail_size() > 0 || cmd.del_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);

    for (int i = 0; i < cmd.detail_size(); ++i)
      XDBG << "[任务同步-信息]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << cmd.detail(i).id() << "complete :" << cmd.detail(i).complete() << "更新" << XEND;
    for (int i = 0; i < cmd.del_size(); ++i)
      XDBG << "[任务同步-信息]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << cmd.del(i).id () << "删除" << XEND;
  }

  XDBG << "[任务同步-信息] -------------------------------------------------------" << XEND;
  m_setDetailIDs.clear();
}

void Quest::addDailyCount()
{
  if (m_pUser == nullptr)
    return;

  resetDailyQuest();
  if (getDailyCount() >= getDailyTCount())
    return;

  m_dwDailyCount += 1;
}

void Quest::clearDaily()
{
  if (m_pUser == nullptr)
    return;

  m_pExpPool = nullptr;
  m_dwDailyLevel = 0;
  m_dwDailyCurExp = 0;
  m_vecDailyGeted.clear();
}

void Quest::resetDailyQuest()
{
  if (m_pUser == nullptr)
    return;

  if (m_pUser->getMenu().isOpen(EMENUID_KANJIMOCHAO) == false)
  {
    XLOG << "[任务-抗击魔潮重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "功能尚未开放" << XEND;
    return;
  }

  DWORD curTime = now();
  DWORD diffDay = 0;
  if (m_dwLastCalcDailyCountTime == 0)
  {
    diffDay = 1;
  }
  else
  {
    DWORD offsetSec = 5 * 3600;   //每天5点算
    diffDay = xTime::getDiffDay(curTime, m_dwLastCalcDailyCountTime, offsetSec);
  }

  if (diffDay <= 0)
    return;

  if (m_dwDailyTCount > m_dwDailyCount)
    m_dwDailyTCount -= m_dwDailyCount;
  else
    m_dwDailyTCount = 0;
  m_dwDailyCount = 0;
  m_dwLastCalcDailyCountTime = curTime;

  const SQuestMiscCFG& rCFG = MiscConfig::getMe().getQuestCFG();

  while (diffDay--)
  {
    if (m_dwDailyTCount < rCFG.dwMaxDailyCount)
    {
      m_dwDailyTCount += rCFG.dwDailyIncrease;
      if (m_dwDailyTCount >= rCFG.dwMaxDailyCount)
        m_dwDailyTCount = rCFG.dwMaxDailyCount;
    }
  }

  XLOG << "[任务-抗击魔潮重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "当前完成次数 :" << m_dwDailyCount << "总次数 :" << m_dwDailyTCount << XEND;
}

void Quest::resetDailyDayQuest()
{
  if (m_pUser == nullptr)
    return;

  auto remove = [&](EQuestType eType)
  {
    for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG == nullptr || pCFG->eType != eType)
      {
        ++m;
        continue;
      }

      XLOG << "[任务-daily重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "类型" << eType << "id :" << m->first << "从submit被重置" << XEND;
      removeQuestDetail(m->first / QUEST_ID_PARAM);
      m_setSubmitUpdateIDs.insert(m->first);
      m = m_mapSubmitQuest.erase(m);
    }
  };

  bool bUpdate = false;

  // char
  if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_DAILY_1) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_QUEST_DAILY_1, 1);
    remove(EQUESTTYPE_DAILY_1);
    bUpdate = true;
  }
  if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_DAILY_3) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_QUEST_DAILY_3, 1);
    remove(EQUESTTYPE_DAILY_3);
    bUpdate = true;
  }
  if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_DAILY_7) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_QUEST_DAILY_7, 1);
    remove(EQUESTTYPE_DAILY_7);
    bUpdate = true;
  }

  // acc
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_QUEST_DAILY_1) == 0)
  {
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_QUEST_DAILY_1, 1);
    remove(EQUESTTYPE_ACC_DAILY_1);
    bUpdate = true;
  }
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_QUEST_DAILY_3) == 0)
  {
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_QUEST_DAILY_3, 1);
    remove(EQUESTTYPE_ACC_DAILY_3);
    bUpdate = true;
  }
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_QUEST_DAILY_7) == 0)
  {
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_QUEST_DAILY_7, 1);
    remove(EQUESTTYPE_ACC_DAILY_7);
    bUpdate = true;
  }

  if (bUpdate)
  {
    acceptNewQuest();
    update();
  }
}

void Quest::resetDailyMapQuest()
{
  if (m_pUser == nullptr || m_pUser->getVar().getVarValue(EVARTYPE_QUEST_DAILY_MAP) != 0)
    return;

  for (auto m = m_mapMapQuest.begin(); m != m_mapMapQuest.end();)
  {
    for (auto s = m->second.begin(); s != m->second.end();)
    {
      if (isSubmit(*s) == true)
      {
        s = m->second.erase(s);
        continue;
      }
      ++s;
    }

    if (m->second.empty() == true)
    {
      m = m_mapMapQuest.erase(m);
      continue;
    }

    ++m;
  }

  m_pUser->getVar().setVarValue(EVARTYPE_QUEST_DAILY_MAP, 1);
  update();
  acceptNewQuest();
  XLOG << "[任务-地图任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行地图任务重置" << XEND;
}

void Quest::resetDailyResetQuest()
{
  bool bUpdate = false;
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_QUEST_DAILY_RESET) == 0)
  {
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_QUEST_DAILY_RESET, 1);

    for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_ACC_DAILY_RESET)
      {
        XLOG << "[任务-共享任务(acc_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "accept questid :" << m->first << "被重置" << XEND;
        m_setUpdateIDs.insert(m->first);
        removeQuestDetail(m->first / QUEST_ID_PARAM);
        m = m_mapAcceptQuest.erase(m);
        bUpdate = true;
        continue;
      }
      ++m;
    }
    for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_ACC_DAILY_RESET)
      {
        XLOG << "[任务-共享任务(acc_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "complete questid :" << m->first << "被重置" << XEND;
        m_setCompleteUpdateIDs.insert(m->first);
        removeQuestDetail(m->first / QUEST_ID_PARAM);
        m = m_mapCompleteQuest.erase(m);
        bUpdate = true;
        continue;
      }
      ++m;
    }
    for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG != nullptr && (pCFG->eType == EQUESTTYPE_DAILY_RESET || pCFG->eType == EQUESTTYPE_ACC_DAILY_RESET))
      {
        XLOG << "[任务-共享任务(acc_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit questid :" << m->first << "被重置" << XEND;
        m_setSubmitUpdateIDs.insert(m->first);
        removeQuestDetail(m->first / QUEST_ID_PARAM);
        m = m_mapSubmitQuest.erase(m);
        bUpdate = true;
        continue;
      }
      ++m;
    }
    XLOG << "[任务-共享任务(acc_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << xTime::getCurSec() << "进行角色重置" << XEND;
  }

  if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_DAILY_RESET) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_QUEST_DAILY_RESET, 1);

    for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG != nullptr && (pCFG->eType == EQUESTTYPE_DAILY_RESET || pCFG->eType == EQUESTTYPE_DAILY_BOX))
      {
        XLOG << "[任务-角色任务(daily_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "accept questid :" << m->first << "被重置" << XEND;
        m_setUpdateIDs.insert(m->first);
        removeQuestDetail(m->first / QUEST_ID_PARAM);
        m = m_mapAcceptQuest.erase(m);
        bUpdate = true;
        continue;
      }
      ++m;
    }
    for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG != nullptr && (pCFG->eType == EQUESTTYPE_DAILY_RESET || pCFG->eType == EQUESTTYPE_DAILY_BOX))
      {
        XLOG << "[任务-角色任务(daily_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "complete questid :" << m->first << "被重置" << XEND;
        m_setCompleteUpdateIDs.insert(m->first);
        removeQuestDetail(m->first / QUEST_ID_PARAM);
        m = m_mapCompleteQuest.erase(m);
        bUpdate = true;
        continue;
      }
      ++m;
    }
    for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG != nullptr && (pCFG->eType == EQUESTTYPE_DAILY_RESET || pCFG->eType == EQUESTTYPE_DAILY_BOX))
      {
        XLOG << "[任务-角色任务(daily_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit questid :" << m->first << "被重置" << XEND;
        m_setSubmitUpdateIDs.insert(m->first);
        removeQuestDetail(m->first / QUEST_ID_PARAM);
        m = m_mapSubmitQuest.erase(m);
        bUpdate = true;
        continue;
      }
      ++m;
    }
    XLOG << "[任务-角色任务(daily_reset)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << xTime::getCurSec() << "进行角色重置" << XEND;
  }

  if (bUpdate)
  {
    update();
    acceptNewQuest();
  }
}

void Quest::resetAccDailyQuest()
{
  bool bUpdate = false;
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_DAILY_QUEST) == 0)
  {
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_DAILY_QUEST, 1);

    for (auto s = m_setProcessAccQuest.begin(); s != m_setProcessAccQuest.end();)
    {
      const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(*s);
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_ACC_DAILY)
      {
        ++s;
        continue;
      }

      XLOG << "[任务-共享任务(acc_daily)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "acc questid :" << *s << "被重置" << XEND;
      s = m_setProcessAccQuest.erase(s);
      bUpdate = true;
    }
    XLOG << "[任务-共享任务(acc_daily)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << xTime::getCurSec() << "进行账号重置" << XEND;
  }

  if (m_pUser->getVar().getVarValue(EVARTYPE_ACCDAILY_QUEST) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_ACCDAILY_QUEST, 1);

    for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_ACC_DAILY)
      {
        ++m;
        continue;
      }
      XLOG << "[任务-共享任务(acc_daily)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "accept questid :" << m->first << "被重置" << XEND;
      m_setUpdateIDs.insert(m->first);
      m = m_mapAcceptQuest.erase(m);
      bUpdate = true;
    }
    for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_ACC_DAILY)
      {
        ++m;
        continue;
      }
      XLOG << "[任务-共享任务(acc_daily)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "complete questid :" << m->first << "被重置" << XEND;
      m_setCompleteUpdateIDs.insert(m->first);
      m = m_mapCompleteQuest.erase(m);
      bUpdate = true;
    }
    for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end();)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_ACC_DAILY)
      {
        ++m;
        continue;
      }
      XLOG << "[任务-共享任务(acc_daily)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "submit questid :" << m->first << "被重置" << XEND;
      m_setSubmitUpdateIDs.insert(m->first);
      m = m_mapSubmitQuest.erase(m);
      bUpdate = true;
    }
    XLOG << "[任务-共享任务(acc_daily)]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << xTime::getCurSec() << "进行角色重置" << XEND;
  }

  if (bUpdate)
  {
    update();
    acceptNewQuest();
  }
}

void Quest::resetDailyMapRandQuest()
{
  if (m_pUser == nullptr || m_pUser->getVar().getVarValue(EVARTYPE_DAILY_MAPRAND) != 0)
    return;
  m_pUser->getVar().setVarValue(EVARTYPE_DAILY_MAPRAND, 1);

  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
  {
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_DAILY_MAPRAND)
    {
      ++m;
      continue;
    }

    XLOG << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << m->first << "从accept中重置" << XEND;

    m_setUpdateIDs.insert(m->first);
    m = m_mapAcceptQuest.erase(m);
  }
  for (auto m = m_mapCompleteQuest.begin(); m != m_mapCompleteQuest.end();)
  {
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_DAILY_MAPRAND)
    {
      ++m;
      continue;
    }

    XLOG << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << m->first << "从complete中重置" << XEND;

    m_setCompleteUpdateIDs.insert(m->first);
    m = m_mapCompleteQuest.erase(m);
  }
  for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end();)
  {
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_DAILY_MAPRAND)
    {
      ++m;
      continue;
    }

    XLOG << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << m->first << "从submit中重置" << XEND;

    m_setSubmitUpdateIDs.insert(m->first);
    m = m_mapSubmitQuest.erase(m);
  }

  m_mapMapRandQuest.clear();
  const TMapMapRandQuestCFG& mapCFG = QuestConfig::getMe().getMapRandQuestList();
  for (auto &m : mapCFG)
  {
    const SDailyPerDay* pQuestCFG = MiscConfig::getMe().getQuestCFG().getDailyCount(m.first);
    if (pQuestCFG == nullptr)
      continue;

    TSetDWORD& setIDs = m_mapMapRandQuest[m.first];
    for (auto &v : m.second.vecCurCFG)
    {
      DWORD dwCount = getDailyRandCount(m.first);
      if (dwCount >= pQuestCFG->dwAcceptCount)
        break;
      setIDs.insert(v.id);
    }
  }
  for (auto &m : m_mapMapRandQuest)
  {
    XLOG << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功 随机 mapid :" << m.first;
    for (auto &s : m.second)
      XLOG << s;
    XLOG << XEND;
  }

  update();
  acceptNewQuest();

  XLOG << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行任务重置" << XEND;
}

void Quest::removeDailyMapRandQuest(DWORD dwMapID)
{
  const SDailyPerDay* pQuestCFG = MiscConfig::getMe().getQuestCFG().getDailyCount(dwMapID);
  if (pQuestCFG == nullptr)
  {
    XERR << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成移除失败,未找到地图" << dwMapID << "ServerGame配置" << XEND;
    return;
  }

  if (getDailyRandCount(dwMapID) < pQuestCFG->dwSubmitCount)
    return;

  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
  {
    const SQuestCFG* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_DAILY_MAPRAND || m->second->getStep() >= 1 || pCFG->mapid != dwMapID)
    {
      ++m;
      continue;
    }

    XLOG << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "完成移除,已完成" << pQuestCFG->dwSubmitCount << "个" << m->first << "被移除" << XEND;
    m_setUpdateIDs.insert(m->first);
    m = m_mapAcceptQuest.erase(m);
  }

  update();
  XLOG << "[任务-宝箱任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成移除成功" << XEND;
}

void Quest::resetCycleQuest()
{
  DWORD curTime = now();

  const TMapQuestCFGEx& mapCFG = QuestManager::getMe().getCycleList();
  for(auto s : mapCFG)
  {
    bool bInSendTime = s.second.isInSendTime(curTime);

    auto m = m_mapAcceptQuest.find(s.first);
    if(m != m_mapAcceptQuest.end())
    {
      bool sameday = xTime::isSameDay(m->second->getTime(), curTime);
      if(bInSendTime == false || sameday == false)
      {
        deleteQuest(s.first);
        continue;
      }
    }

    auto p = m_mapCompleteQuest.find(s.first);
    if(p != m_mapCompleteQuest.end())
    {
      bool sameday = xTime::isSameDay(p->second->getTime(), curTime);
      if(bInSendTime == false || sameday == false)
      {
        deleteQuest(s.first);
        continue;
      }
    }

    auto q = m_mapSubmitQuest.find(s.first);
    if(q != m_mapSubmitQuest.end())
    {
      bool sameday = xTime::isSameDay(q->second->getTime(), curTime);
      if(bInSendTime == false || sameday == false)
      {
        deleteQuest(s.first);
        continue;
      }
    }

    if(s.second.isInTime(curTime) == false || bInSendTime == false)
      m_setProcessAccQuest.erase(s.first);
  }

  for(auto s : mapCFG)
  {
    if(s.second.isInTime(curTime) == true && s.second.isInSendTime(curTime) == true)
    {
      auto acc = m_mapAcceptQuest.find(s.first);
      if(acc != m_mapAcceptQuest.end())
        continue;

      auto com = m_mapCompleteQuest.find(s.first);
      if(com != m_mapCompleteQuest.end())
        continue;

      auto sub = m_mapSubmitQuest.find(s.first);
      if(sub != m_mapSubmitQuest.end())
        continue;

      acceptQuest(s.first);
    }
  }
}

void Quest::resetGuildQuest()
{
  DWORD dwNow = xTime::getCurSec();

  auto func = [&](EQuestList eType)
  {
    TMapQuest* pMapQuest = nullptr;
    TSetDWORD* pSetUpdateIDs = nullptr;

    if (eType == EQUESTLIST_ACCEPT)
    {
      pMapQuest = &m_mapAcceptQuest;
      pSetUpdateIDs = &m_setUpdateIDs;
    }
    else if (eType == EQUESTLIST_COMPLETE)
    {
      pMapQuest = &m_mapCompleteQuest;
      pSetUpdateIDs = &m_setCompleteUpdateIDs;
    }
    else if (eType == EQUESTLIST_SUBMIT)
    {
      pMapQuest = &m_mapSubmitQuest;
      pSetUpdateIDs = &m_setSubmitUpdateIDs;
    }

    if (pMapQuest == nullptr || pSetUpdateIDs == nullptr)
      return;

    for (auto m = pMapQuest->begin(); m != pMapQuest->end();)
    {
      if (m->second == nullptr)
      {
        ++m;
        continue;
      }
      const SQuestCFG* pCFG = m->second->getQuestCFG();
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_GUILD)
      {
        ++m;
        continue;
      }
      if (m_pUser->hasGuild() == true && dwNow < m->second->getTime())
      {
        ++m;
        continue;
      }
      pSetUpdateIDs->insert(m->first);
      m = pMapQuest->erase(m);
    }
  };

  func(EQUESTLIST_ACCEPT);
  func(EQUESTLIST_COMPLETE);
  func(EQUESTLIST_SUBMIT);

  update();
  XLOG << "[任务-公会任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << dwNow << "进行公会任务刷新" << XEND;
}

void Quest::resetArtifactQuest()
{
  GGuild& rGuild = m_pUser->getGuild();
  bool bAuth = rGuild.hasAuth(EAUTH_ARTIFACT_QUEST);
  if (rGuild.id() == 0 || !bAuth)
  {
    removeQuest(EQUESTLIST_ACCEPT, EQUESTTYPE_ARTIFACT);
    removeQuest(EQUESTLIST_COMPLETE, EQUESTTYPE_ARTIFACT);
    removeQuest(EQUESTLIST_SUBMIT, EQUESTTYPE_ARTIFACT);
    XLOG << "[任务-神器任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "所在公会" << rGuild.id() << "权限" << bAuth << "不满足条件,任务被重置删除" << XEND;
    return;
  }

  const TVecQuestCFG* pVecCFG = QuestConfig::getMe().getTypeQuestCFGList(EQUESTTYPE_ARTIFACT);
  if (pVecCFG == nullptr)
  {
    XERR << "[任务-神器任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未能获取神器任务列表" << XEND;
    return;
  }
  for (auto &v : *pVecCFG)
  {
    auto a = m_mapAcceptQuest.find(v.id);
    if (a != m_mapAcceptQuest.end() && (!bAuth || a->second->getQParams(0) != rGuild.id()))
    {
      m_setUpdateIDs.insert(a->first);
      a = m_mapAcceptQuest.erase(a);
    }
    auto c = m_mapCompleteQuest.find(v.id);
    if (c != m_mapCompleteQuest.end() && (!bAuth || c->second->getQParams(0) != rGuild.id()))
    {
      m_setCompleteUpdateIDs.insert(c->first);
      c = m_mapCompleteQuest.erase(c);
    }
    auto s = m_mapSubmitQuest.find(v.id);
    if (s != m_mapSubmitQuest.end() && (!bAuth || s->second->getQParams(0) != rGuild.id()))
    {
      m_setSubmitUpdateIDs.insert(s->first);
      s = m_mapSubmitQuest.erase(s);
    }
    if (m_setUpdateIDs.empty() == false || m_setCompleteUpdateIDs.empty() == false || m_setSubmitUpdateIDs.empty() == false)
    {
      XLOG << "[任务-神器任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "公会或权限变更, 以下任务被重置, 可接" << m_setUpdateIDs << "完成" << m_setCompleteUpdateIDs << "提交" << m_setSubmitUpdateIDs << XEND;
      update();
    }
  }

  if (!bAuth)
    return;

  // remove submit quest
  const TSetDWORD& setSubmitIDs = rGuild.getGQuestSubmitList();
  for (auto &submit : setSubmitIDs)
  {
    const TVecDWORD& vecIDs = QuestConfig::getMe().getQuestDetail(submit / QUEST_ID_PARAM);
    for (auto &v : vecIDs)
    {
      auto a = m_mapAcceptQuest.find(v);
      if (a != m_mapAcceptQuest.end())
      {
        m_setUpdateIDs.insert(a->first);
        a = m_mapAcceptQuest.erase(a);
      }
      auto c = m_mapCompleteQuest.find(v);
      if (c != m_mapCompleteQuest.end())
      {
        m_setCompleteUpdateIDs.insert(c->first);
        c = m_mapCompleteQuest.erase(c);
      }
      auto s = m_mapSubmitQuest.find(v);
      if (s != m_mapSubmitQuest.end())
      {
        m_setSubmitUpdateIDs.insert(s->first);
        s = m_mapSubmitQuest.erase(s);
      }
      if (m_setUpdateIDs.empty() == false || m_setCompleteUpdateIDs.empty() == false || m_setSubmitUpdateIDs.empty() == false)
      {
        XLOG << "[任务-神器任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "submit :" << submit << "已完成,以下任务被重置,可接" << m_setUpdateIDs << "完成" << m_setSubmitUpdateIDs << "提交" << m_setSubmitUpdateIDs << XEND;
        update();
      }
    }
  }

  // accept new quest
  acceptTypeQuest(EQUESTTYPE_ARTIFACT);
}

void Quest::resetQuestTime(DWORD curTime)
{
  TSetDWORD setChristmas;
  bool bNight = MiscConfig::getMe().getSystemCFG().isNight(curTime);
  for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end();)
  {
    if (m->second == nullptr)
    {
      ++m;
      continue;
    }
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr)
    {
      ++m;
      continue;
    }

    if (pCFG->eType == EQUESTTYPE_DAY || pCFG->eType == EQUESTTYPE_NIGHT)
    {
      TPtrBaseStep pStep = m->second->getStepCFG();
      if (pStep == nullptr || pStep->canDayNightReset() == false)
      {
        ++m;
        continue;
      }
      if ((pCFG->eType == EQUESTTYPE_DAY && !bNight) || (pCFG->eType == EQUESTTYPE_NIGHT && bNight))
      {
        ++m;
        continue;
      }
    }
    else
    {
      if (pCFG->isInTime(curTime) == true && pCFG->isInSendTime(curTime) == true)
      {
        ++m;
        continue;
      }
    }

    if(m->first == QUEST_CHRISTMAS)
    {
      setChristmas.insert(m->first);
      ++m;
      continue;
    }

    m_pUser->getQuestNpc().delNpcQuest(m->first / QUEST_ID_PARAM, m->first % QUEST_ID_PARAM);
    m_setUpdateIDs.insert(m->first);

    XLOG << "[任务-时间检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "id :" << m->first << "type :" << pCFG->eType << "不在有效时间内,被删除,当前在" << (bNight ? "夜里" : "白天") << "不在有效时间内,被删除" << XEND;
    m = m_mapAcceptQuest.erase(m);
  }

  for(auto m : setChristmas)
  {
    doStepWithoutCheck(m);
    XLOG << "[任务-圣诞任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "id :" << m<< "活动结束后执行" << XEND;
  }

  acceptNewQuest();
  update();
  XDBG << "[任务-时间检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "time :" << curTime << "进行有效时间检查" << XEND;
}

void Quest::resetWeddingQuest(bool bWedding)
{
  if (!bWedding)
  {
    removeQuest(EQUESTLIST_ACCEPT, EQUESTTYPE_WEDDING);
    removeQuest(EQUESTLIST_COMPLETE, EQUESTTYPE_WEDDING);
    removeQuest(EQUESTLIST_SUBMIT, EQUESTTYPE_WEDDING);

    removeQuest(EQUESTLIST_ACCEPT, EQUESTTYPE_WEDDING_DAILY);
    removeQuest(EQUESTLIST_COMPLETE, EQUESTTYPE_WEDDING_DAILY);
    removeQuest(EQUESTLIST_SUBMIT, EQUESTTYPE_WEDDING_DAILY);
    XLOG << "[任务-结婚任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "没有婚姻关系,任务被重置删除" << XEND;
    return;
  }

  acceptTypeQuest(EQUESTTYPE_WEDDING);
  acceptTypeQuest(EQUESTTYPE_WEDDING_DAILY);
  XLOG << "[任务-结婚任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建立婚姻关系,刷新结婚任务" << XEND;
}

void Quest::refreshCollectPuzzles()
{
  TVecItemInfo vecReward;
  const TMapVersionPuzzleCFG& mapCFG = QuestConfig::getMe().getVersionPuzzleList();
  for (auto &m : mapCFG)
  {
    const SQuestVersion* pVersionCFG = QuestConfig::getMe().getVersionCFG(m.first);
    if (pVersionCFG == nullptr)
      continue;

    SQuestPuzzle& rPuzzle = m_mapPuzzle[m.first];
    const TMapQuestPuzzleCFG& mapPuzzle = m.second.getCollectPuzzleList();
    for (auto &collect : mapPuzzle)
    {
      auto p = rPuzzle.setUnlock.find(collect.first);
      if (p != rPuzzle.setUnlock.end())
        continue;

      const SQuestPuzzleCFG& rCFG = collect.second;
      if (rPuzzle.setOpen.size() < rCFG.dwIndex)
        continue;

      for (auto &s : rCFG.setRewards)
      {
        TVecItemInfo vecSingle;
        if (RewardManager::roll(s, m_pUser, vecSingle, ESOURCE_PUZZLE) == false)
        {
          XLOG << "[任务-碎片收集]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->getName()
            << "收集 version :" << m.first << "id :" << collect.first << "num :" << rCFG.dwIndex << "的任务达成,但 reward :" << s << "随机失败" << XEND;
          continue;
        }
        combinItemInfo(vecReward, vecSingle);
      }

      rPuzzle.setUnlock.insert(collect.first);
      XLOG << "[任务-碎片收集]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->getName()
        << "收集 version :" << m.first << "id :" << collect.first << "num :" << rCFG.dwIndex << "的任务达成" << XEND;
    }
  }

  if (vecReward.empty() == false)
    m_pUser->getPackage().addItem(vecReward, EPACKMETHOD_AVAILABLE);
  XLOG << "[任务-碎片收集]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->getName() << "进行碎片收集刷新" << XEND;
}

void Quest::resetDeadQuest()
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_DEAD) == true)
    return;
  Variable& rVar = m_pUser->getVar();
  if (rVar.getVarValue(EVARTYPE_DEAD_QUEST) != 0)
    return;
  rVar.setVarValue(EVARTYPE_DEAD_QUEST, 1);

  removeQuest(EQUESTLIST_ACCEPT, EQUESTTYPE_DEAD);
  removeQuest(EQUESTLIST_COMPLETE, EQUESTTYPE_DEAD);
  removeQuest(EQUESTLIST_SUBMIT, EQUESTTYPE_DEAD);

  m_setDeadIDs.clear();

  const SDeadMiscCFG& rCFG = MiscConfig::getMe().getDeadCFG();
  const TVecQuestCFG* pVecCFG = QuestConfig::getMe().getTypeQuestCFGList(EQUESTTYPE_DEAD);
  if (pVecCFG == nullptr)
  {
    XERR << "[任务-亡者任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置失败,未找到亡者任务库" << XEND;
    return;
  }

  DWORD dwIndex = 0;
  map<DWORD, const SQuestCFG*> mapResult;
  const TVecQuestCFG& vecCFG = *pVecCFG;
  while (++dwIndex < 10 && mapResult.size() < rCFG.dwQuestNum)
  {
    DWORD dwRand = randBetween(0, vecCFG.size() - 1);
    const SQuestCFG* pCFG = &vecCFG[dwRand];
    auto m = mapResult.find(pCFG->id);
    if (m != mapResult.end())
      continue;
    mapResult[pCFG->id] = pCFG;
  }

  for (auto &m : mapResult)
    acceptQuest(m.first);
  XLOG << "[任务-亡者任务]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功" << XEND;
}

void Quest::resetWantedQuest()
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED_RESET) != 0)
    return;

  for (auto m = m_mapSubmitQuest.begin(); m != m_mapSubmitQuest.end();)
  {
    const SQuestCFGEx* pCFG = m->second->getQuestCFG();
    if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_WANTED || QuestConfig::getMe().isRealWantedQuest(pCFG->id) == false)
    {
      ++m;
      continue;
    }

    removeQuestDetail(m->first / QUEST_ID_PARAM);
    m_setSubmitUpdateIDs.insert(m->first);
    m = m_mapSubmitQuest.erase(m);
  }

  m_pUser->getVar().getVarValue(EVARTYPE_QUEST_WANTED);
  m_pUser->getVar().setVarValue(EVARTYPE_QUEST_WANTED_RESET, 1);
  update();
}

void Quest::onLeaveScene()
{
  if (m_pUser->getScene() == nullptr)
    return;
  if (m_pUser->getScene()->isDScene())
  {
    TVecDWORD vecIDs;
    for (auto m = m_mapAcceptQuest.begin(); m != m_mapAcceptQuest.end(); ++m)
    {
      const SQuestCFGEx* pCFG = m->second->getQuestCFG();
      if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_RAIDTALK)
        continue;
      vecIDs.push_back(m->first);
    }
    for (auto &v : vecIDs)
    {
      abandonGroup(v);
    }
    if (!vecIDs.empty())
      update();
  }
}

void Quest::onPlayMusic(DWORD dwMusicID)
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_PLAY_MUSIC)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    runStep(v, 0, dwMusicID);
  }
}

void Quest::onMoneyChange()
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_MONEY)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    if (canStep(v) == true)
      runStep(v);
  }
}

void Quest::onTowerPass()
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_OPTION)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &s : vecIDs)
  {
    if (canStep(s) == true)
      runStep(s);
  }
}

void Quest::onPhoto(const TSetQWORD& setGUIDs)
{
  m_setTmpUser.clear();
  m_setTmpNpc.clear();

  for (auto &s : setGUIDs)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(s);
    if (pUser != nullptr)
    {
      m_setTmpUser.insert(pUser);
      XDBG << "[任务-拍照]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收集玩家 :" << pUser->id << pUser->name << XEND;
      continue;
    }

    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(s);
    if (pNpc != nullptr)
    {
      m_setTmpNpc.insert(pNpc);
      XDBG << "[任务-拍照]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收集NPC :" << pNpc->id << pNpc->getNpcID() << pNpc->name << XEND;
    }
  }

  if (m_setTmpUser.empty() == true && m_setTmpNpc.empty() == true)
    return;

  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_PHOTO)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    if (canStep(v) == true)
      runStep(v);
  }

  m_setTmpUser.clear();
  m_setTmpNpc.clear();
}

void Quest::onHand(QWORD qwTargetID, DWORD dwTime)
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_HAND)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &s : vecIDs)
  {
    auto m = m_mapAcceptQuest.find(s);
    if (m == m_mapAcceptQuest.end())
      continue;
    runStep(s, qwTargetID, dwTime);
  }
}

void Quest::onMusic(DWORD dwTime)
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_MUSIC)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    if (canStep(v) == true)
      runStep(v, 0, dwTime);
  }
}

void Quest::onCarrier(const TSetTmpUser& setUser, DWORD dwCarrierID)
{
  m_setTmpUser = setUser;
  if (m_setTmpUser.empty() == true)
    return;

  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_CARRIER)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    if (canStep(v) == true)
      runStep(v, 0, dwCarrierID);
  }

  m_setTmpUser.clear();
}

void Quest::onPetAdd()
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_PET)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    if (canStep(v) == true)
      runStep(v);
  }
}

void Quest::onCookFood(DWORD dwFoodId)
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_COOKFOOD)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    runStep(v, 0, dwFoodId);
  }
}

void Quest::onEnterScene()
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_SCENE)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
    runStep(v);
}

void Quest::onBuff()
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_BUFF)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
    runStep(v);
}

void Quest::onQuestSubmit()
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_CHECKGROUP)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
    runStep(v);
}

void Quest::onTransfer(DWORD dwFromid,DWORD dwToid)
{
  TVecDWORD vecIDs;
  for (auto &m : m_mapAcceptQuest)
  {
    TPtrBaseStep pStep = m.second->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_TRANSFER)
      continue;
    vecIDs.push_back(m.first);
  }

  for (auto &v : vecIDs)
  {
    runStep(v, 0, dwFromid, dwToid);
  }

}

DWORD Quest::getDailyRandCount(DWORD dwMapID, bool bSubmit /*= false*/)
{
  auto m = m_mapMapRandQuest.find(dwMapID);
  if (m == m_mapMapRandQuest.end())
    return 0;

  DWORD dwCount = 0;
  for (auto &s : m->second)
  {
    if (bSubmit)
    {
      if (isSubmit(s) == true)
        ++dwCount;
      continue;
    }

    TPtrQuestItem pItem = getQuest(s);
    if (pItem == nullptr)
      continue;
    if (isSubmit(s) == true || pItem->getStep() >= 1 || pItem->isComplete() == true)
      ++dwCount;
  }
  return dwCount;
}

bool Quest::getWantedQuest(std::pair<DWORD, DWORD>& id2step, bool bSyncTeam /*=false*/) const
{
  if (!m_pUser)
    return 0;
  for (auto &m : m_mapAcceptQuest)
  {
    if (!m.second || !m.second->getQuestCFG())
      continue;
    if (m.second->getQuestCFG()->eType == EQUESTTYPE_WANTED)
    {
      if (bSyncTeam)
      {
        const SWantedItem* pWantedItem = QuestConfig::getMe().getWantedItemCFG(m.second->getID());
        if (pWantedItem == nullptr)
          return false;
        //if (!pWantedItem->bTeamSync)
          //return false;
      }

      id2step.first = m.second->getID();
      id2step.second = m.second->getStep();
      return true;
    }
  }

  return false;
}

// 获取看板队长技能可影响的队友
void Quest::getWantedQuestLeaderTeammate(DWORD questid, set<SceneUser*> &teammate)
{
  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(questid);
  if (pCFG != nullptr && pCFG->eType == EQUESTTYPE_WANTED && m_pUser->isWantedQuestLeader())
  {
    teammate.clear();
    xSceneEntrySet uset;
    m_pUser->getScene()->getEntryListInNine(SCENE_ENTRY_USER, m_pUser->getPos(), uset);
    for (auto &s : uset)
    {
      SceneUser* u = dynamic_cast<SceneUser*>(s);
      if (u != nullptr && u->id != m_pUser->id && u->getTeamID() != 0 && u->getTeamID() == m_pUser->getTeamID() &&
          u->getUserSceneData().getFollowerID() != 0 && u->getUserSceneData().getFollowerID() == m_pUser->id)
        teammate.insert(u);
    }
  }
}

bool Quest::doStepWithoutCheck(DWORD id)
{
  TPtrQuestItem pItem = getQuest(id);
  if (pItem == nullptr || pItem->isComplete() == true)
    return false;

  const SQuestCFGEx* pCFG = pItem->getQuestCFG();
  if (pCFG == nullptr)
  {
    XERR << "[任务-步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行" << id << "失败,未在 Table_Quest.txt 表中找到" << XEND;
    return false;
  }

  TPtrBaseStep pStep = pItem->getStepCFG();
  if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_CHRISTMAS_RUN)
    return false;

  return pStep->doStep(m_pUser);
}

bool Quest::deleteQuest(DWORD dwQuestID)
{
  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(dwQuestID);
  if (pCFG == nullptr)
    return false;

  DWORD curTime = xTime::getCurSec();
  bool bRemove = false;
  auto m = m_mapAcceptQuest.find(dwQuestID);
  if(m != m_mapAcceptQuest.end())
  {
    XLOG << "[任务-删除] 删除m_mapAcceptQuest" << m_pUser->id << m_pUser->name << "id :" << m->first << m->second->getTime() << curTime << XEND;
    m_setUpdateIDs.insert(m->first);
    removeQuestDetail(m->first / QUEST_ID_PARAM);
    m_mapAcceptQuest.erase(m);
    bRemove = true;
  }

  auto p = m_mapCompleteQuest.find(dwQuestID);
  if(p != m_mapCompleteQuest.end())
  {
    XLOG << "[任务-删除] 删除m_mapCompleteQuest" << m_pUser->id << m_pUser->name << "id :" << p->first << p->second->getTime() << curTime << XEND;
    m_setCompleteUpdateIDs.insert(p->first);
    removeQuestDetail(p->first / QUEST_ID_PARAM);
    m_mapCompleteQuest.erase(p);
    bRemove = true;
  }

  auto q = m_mapSubmitQuest.find(dwQuestID);
  if(q != m_mapSubmitQuest.end())
  {
    XLOG << "[任务-删除] 删除m_mapSubmitQuest" << m_pUser->id << m_pUser->name << "id :" << q->first << q->second->getTime() << curTime << XEND;
    m_setSubmitUpdateIDs.insert(q->first);
    removeQuestDetail(q->first / QUEST_ID_PARAM);
    m_mapSubmitQuest.erase(q);
    bRemove = true;
  }

  m_setProcessAccQuest.erase(dwQuestID);
  if(bRemove == true)
  {
    update();
    if(pCFG->eType != EQUESTTYPE_WANTED)
      acceptTypeQuest(pCFG->eType);
    XLOG << "[任务-删除] " << m_pUser->id << m_pUser->name << "id :" << dwQuestID << curTime << XEND;
  }

  return true;
}

bool Quest::checkOutDateQuest()
{
  const TMapQuestCFGEx& mapCFG = QuestManager::getMe().getOutDateCycleList();
  for(auto s : mapCFG)
    deleteQuest(s.first);

  return true;
}

void Quest::addActivityFinishTimes(DWORD id)
{
  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(id);
  if (pCFG == nullptr || pCFG->refreshTimes == 0)
    return;

  DWORD curTime = now();
  auto m = m_mapActivityQuest.find(id);
  if(m != m_mapActivityQuest.end())
  {
    DWORD dwFinish = m->second.finishcount();
    switch (pCFG->eRefreshType)
    {
      case EQUESTREFRESH_DAY:
        {
          bool sameday = xTime::isSameDay(m->second.lastquesttime() - 5 * HOUR_T, curTime - 5 * HOUR_T);
          if(sameday == true)
            m->second.set_finishcount(dwFinish + 1);
          else
            m->second.set_finishcount(1);

          m->second.set_lastquesttime(curTime);
        }
        break;
      case EQUESTREFRESH_WEEK:
        {
          bool sameday = xTime::isSameWeek(m->second.lastquesttime() - 5 * HOUR_T, curTime - 5 * HOUR_T);
          if(sameday == true)
            m->second.set_finishcount(dwFinish + 1);
          else
            m->second.set_finishcount(1);

          m->second.set_lastquesttime(curTime);
        }
        break;
      case EQUESTREFRESH_PERIOD:
        {
          m->second.set_finishcount(dwFinish + 1);
          m->second.set_lastquesttime(curTime);
        }
        break;
      default:
        break;
    }
  }
  else
  {
    ActivityQuestItem stItem;
    stItem.set_questid(id);
    stItem.set_finishcount(1);
    stItem.set_lastquesttime(curTime);
    m_mapActivityQuest.emplace(id, stItem);
  }

  XLOG << "[活动任务-提交]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "quest :" << id << XEND;
}

DWORD Quest::getActivityFinishTimes(DWORD id)
{
  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(id);
  if (pCFG == nullptr || pCFG->refreshTimes == 0)
    return 0;

  DWORD curTime = now();
  auto m = m_mapActivityQuest.find(id);
  if(m != m_mapActivityQuest.end())
  {
    DWORD dwFinish = m->second.finishcount();
    switch (pCFG->eRefreshType)
    {
      case EQUESTREFRESH_DAY:
        {
          bool sameday = xTime::isSameDay(m->second.lastquesttime() - 5 * HOUR_T, curTime - 5 * HOUR_T);
          if(sameday == true)
            return dwFinish;
          else
          {
            m->second.set_lastquesttime(0);
            m->second.set_finishcount(0);
            return 0;
          }
        }
        break;
      case EQUESTREFRESH_WEEK:
        {
          bool sameday = xTime::isSameWeek(m->second.lastquesttime() - 5 * HOUR_T, curTime - 5 * HOUR_T);
          if(sameday == true)
            return dwFinish;
          else
          {
            m->second.set_finishcount(0);
            m->second.set_lastquesttime(0);
            return 0;
          }
        }
        break;
      case EQUESTREFRESH_PERIOD:
        {
          return dwFinish;
        }
        break;
      default:
        break;
    }
  }

  return 0;
}

void Quest::resetActivityQuest()
{
  DWORD curTime = now();
  for(auto m = m_mapActivityQuest.begin(); m != m_mapActivityQuest.end();)
  {
    bool ret = false;
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(m->second.questid());
    if(pCFG == nullptr)
    {
      ++m;
      continue;
    }

    if(pCFG->isInTime(now()) == false || pCFG->isInTime(m->second.lastquesttime()) == false)
      ret = true;

    if (pCFG->eRefreshType == EQUESTREFRESH_DAY)
    {
      bool sameday = xTime::isSameDay(m->second.lastquesttime() - 5 * HOUR_T, curTime - 5 * HOUR_T);
      if (sameday == false)
        ret = true;
    }
    else if (pCFG->eRefreshType == EQUESTREFRESH_WEEK)
    {
      bool sameweek = xTime::isSameWeek(m->second.lastquesttime() - 5 * HOUR_T, curTime - 5 * HOUR_T);
      if (sameweek == false)
        ret = true;
    }
    else if (pCFG->eRefreshType == EQUESTREFRESH_PERIOD)
    {
    }

    if(ret == true)
    {
      XLOG << "[活动任务-删除] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid: " << m->second.questid() << XEND;
      deleteQuest(m->second.questid());
      m_mapActivityQuest.erase(m++);
    }
    else
      ++m;
  }
}

bool Quest::isTeamFinishBoardQuestCD()
{
  return m_dwTeamFinishBoardQuestTime && now() <= m_dwTeamFinishBoardQuestTime + MiscConfig::getMe().getQuestCFG().dwTeamFinishBoardQuestCD;
}

void Quest::queryManualData(const string& version)
{
  const SQuestVersion* pCFG = QuestConfig::getMe().getVersionCFG(version);
  if (pCFG == nullptr)
  {
    XERR << "[任务-任务手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询 ver :" << version << "数据失败,没有该版本" << XEND;
    return;
  }

  xTime frameDebug;
  TSetDWORD setIDs;

  QueryManualQuestCmd cmd;
  cmd.set_version(version);

  QuestManual* pManual = cmd.mutable_manual();
  pManual->set_version(version);

  // manual - main
  TSetDWORD setAftIDs;
  bool bMainShow = false;
  QuestManualMain* pMain = pManual->mutable_main();
  for (auto &v : pCFG->vecMainCFG)
  {
    if (isSubmit(v.id) == true)
    {
      QuestManualItem* pItem = pMain->add_items();
      pItem->set_type(EQUESTLIST_SUBMIT);
      pItem->mutable_data()->set_id(v.id);
      continue;
    }

    if (bMainShow)
      continue;

    const SQuestCFG& rCFG = v;
    TPtrQuestItem pQuest = getQuest(rCFG.id);
    if (pQuest == nullptr)
      continue;

    QuestManualItem* pItem = pMain->add_items();
    pItem->set_type(EQUESTLIST_ACCEPT);

    QuestData* pData = pItem->mutable_data();
    pQuest->toClientData(pData);
    setAftIDs.insert(rCFG.id);

    bMainShow = true;

    TPtrBaseStep pStep = pQuest->getStepCFG();
    if (pStep == nullptr || pStep->getStepType() != EQUESTSTEP_ITEM)
      continue;

    TVecDWORD vecPreIDs;
    vecPreIDs.insert(vecPreIDs.end(), rCFG.vecPreQuest.begin(), rCFG.vecPreQuest.end());
    vecPreIDs.insert(vecPreIDs.end(), rCFG.vecMustPreQuest.begin(), rCFG.vecMustPreQuest.end());
    for (auto &v : vecPreIDs)
    {
      const SQuestPreQuest* pPreCFG = QuestConfig::getMe().getQuestCFGByPre(v);
      if (pPreCFG == nullptr)
      {
        XDBG << "[任务-任务手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询 ver :" << version << "数据,main :" << rCFG.id << "的前置任务" << v << "无后续任务" << XEND;
        continue;
      }
      const TVecQuestCFG& vecCFG = pPreCFG->getQuestList(EQUESTTYPE_DAILY_1);
      for (auto &v : vecCFG)
      {
        TPtrQuestItem pSubQuest = getQuest(v.id);
        if (pSubQuest != nullptr)
        {
          QuestManualItem* pSub = pItem->add_subs();
          pSub->set_type(isSubmit(v.id) == true ? EQUESTLIST_SUBMIT : EQUESTLIST_ACCEPT);
          if (pSub->type() == EQUESTLIST_ACCEPT)
          {
            pSubQuest->toClientData(pSub->mutable_data());
            continue;
          }
          pSub->mutable_data()->set_id(rCFG.id);
          setIDs.insert(rCFG.id);
        }
      }
    }
  }
  if (!bMainShow)
  {
    for (auto &v : pCFG->vecMainCFG)
    {
      if (isSubmit(v.id) == false)
        continue;
      if (bMainShow)
        break;

      const SQuestPreQuest* pPreCFG = QuestConfig::getMe().getQuestCFGByPre(v.id);
      if (pPreCFG == nullptr)
        continue;
      const TVecQuestCFG& vecCFG = pPreCFG->getQuestList(EQUESTTYPE_MAIN);
      for (auto &aft : vecCFG)
      {
        if (isSubmit(aft.id) == true)
          continue;

        const SQuestCFG& rCFG = aft;
        QuestManualItem* pItem = pMain->add_items();
        QuestData* pData = pItem->mutable_data();

        pItem->set_type(EQUESTLIST_CANACCEPT);
        pData->set_id(rCFG.id);
        rCFG.toPreview(pData->add_steps(), 0, m_pUser->getLanguage());

        for (auto &pre : rCFG.vecPreQuest)
          setIDs.insert(pre);
        for (auto &pre : rCFG.vecMustPreQuest)
          setIDs.insert(pre);

        bMainShow = true;
      }
    }
  }
  if (!bMainShow)
  {
    for (auto &v : pCFG->vecMainCFG)
    {
      if (isSubmit(v.id) == true)
        continue;
      if (bMainShow)
        break;

      const SQuestCFG& rCFG = v;
      if (v.vecPreQuest.empty() == false || v.vecMustPreQuest.empty() == false)
        continue;
      if (v.lv == 0)
        continue;

      QuestManualItem* pItem = pMain->add_items();
      QuestData* pData = pItem->mutable_data();

      pItem->set_type(EQUESTLIST_CANACCEPT);
      pData->set_id(rCFG.id);
      rCFG.toPreview(pData->add_steps(), 0, m_pUser->getLanguage());

      for (auto &pre : rCFG.vecPreQuest)
        setIDs.insert(pre);
      for (auto &pre : rCFG.vecMustPreQuest)
        setIDs.insert(pre);

      bMainShow = true;
    }
  }

  auto m = m_mapPuzzle.find(version);
  if (m != m_mapPuzzle.end())
  {
    QuestPuzzle* pPuzzle = pMain->mutable_puzzle();
    const SQuestPuzzle& rPuzzle = m->second;
    for (auto &s : rPuzzle.setOpen)
      pPuzzle->add_open_puzzles(s);
    for (auto &s : rPuzzle.setUnlock)
      pPuzzle->add_unlock_puzzles(s);
  }

  const TMapQuestMainStoryCFG& mapCFG = QuestConfig::getMe().getMainStoryCFGList();
  for (auto &m : mapCFG)
  {
    auto s = find_if(m.second.setQuestIDs.begin(), m.second.setQuestIDs.end(), [&](DWORD dwID) -> bool{
      auto v = find_if(pCFG->vecMainCFG.begin(), pCFG->vecMainCFG.end(), [&](const SQuestCFG& r) -> bool{
        return r.id == dwID;
      });
      return v != pCFG->vecMainCFG.end() && isSubmit(dwID) == true;
    });
    if (s != m.second.setQuestIDs.end())
      pMain->set_mainstoryid(m.first);
  }

  const SQuestMiscCFG& rMiscCFG = MiscConfig::getMe().getQuestCFG();
  for (auto &m : pCFG->mapMainCFG)
  {
    if (version == "1.0")
    {
      map<string, TSetDWORD> mapIDs;
      const TVecDWORD& vecIDs = QuestConfig::getMe().getQuestDetail(m.second.questid());
      for (auto &v : vecIDs)
      {
        const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(v);
        if (pCFG != nullptr && rMiscCFG.isManualMain(pCFG->eType) == true)
          mapIDs[pCFG->questname].insert(v);
      }
      for (auto &m : mapIDs)
      {
        TSetDWORD& setIDs = m.second;
        if (setIDs.empty() == true)
          continue;
        QuestPreview* p = pMain->add_previews();
        p->set_questid(*setIDs.begin() / QUEST_ID_PARAM);
        p->set_name(CommonConfig::getMe().getTranslateString(m.first, m_pUser->getLanguage()));
        p->set_complete(true);
        for (auto &s : setIDs)
        {
          if (isSubmit(s) == false)
          {
            p->set_complete(false);
            break;
          }
        }
      }
      continue;
    }

    QuestPreview* p = pMain->add_previews();
    p->CopyFrom(m.second);
    p->set_complete(QuestManager::getMe().isQuestComplete(version, m.first, m_pUser));
    p->clear_rewardgroup();
    p->clear_allrewardid();
  }

  // manual - branch
  QuestManualBranch* pBranch = pManual->mutable_branch();
  for (auto &m : pCFG->mapBranchCFG)
  {
    QuestShop* pShop = pBranch->add_shops();
    pShop->set_itemid(m.first);

    const map<DWORD, TVecQuestCFG>& mapCFG = m.second;
    for (auto &item : mapCFG)
    {
      QuestManualItem* pItem = pShop->add_quests();
      QuestData* pData = pItem->mutable_data();

      if (isSubmit(item.first) == true)
      {
        pItem->set_type(EQUESTLIST_SUBMIT);
        pData->set_id(item.first);

        setIDs.insert(item.first);
        continue;
      }

      const TVecQuestCFG& vecCFG = item.second;
      bool bBranchShow = false;
      for (auto &v : vecCFG)
      {
        const SQuestCFG& rCFG = v;
        TPtrQuestItem pQuest = getQuest(rCFG.id);
        if (pQuest != nullptr && isSubmit(rCFG.id) == false)
        {
          pItem->set_type(EQUESTLIST_ACCEPT);
          pQuest->toClientData(pData);
          bBranchShow = true;
          break;
        }
      }
      if (bBranchShow)
        continue;

      const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(item.first);
      if (pCFG == nullptr)
      {
        XERR << "[任务-任务手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "查询 ver :" << version << "数据收集支线itemid :" << m.first << "questid :" << item.first << "未在 Table_Quest.txt 表中找到" << XEND;
        continue;
      }

      pItem->set_type(EQUESTLIST_CANACCEPT);
      pData->set_id(pCFG->id);
      pCFG->toPreview(pData->add_steps(), 0, m_pUser->getLanguage());

      QuestPConfig* pConfig = pData->mutable_steps(0)->mutable_config();
      pConfig->clear_prequest();
      pConfig->clear_mustprequest();

      if (vecCFG.empty() == false)
      {
        const SQuestCFG& rPre = vecCFG.back();
        pConfig->set_level(rPre.lv);
        for (auto &v : rPre.vecPreQuest)
        {
          pConfig->add_prequest(v);
          setIDs.insert(v);
        }
        for (auto &v : rPre.vecMustPreQuest)
        {
          pConfig->add_prequest(v);
          setIDs.insert(v);
        }
      }
    }
  }

  // manual - story
  QuestManualStory* pStory = pManual->mutable_story();
  for (auto &m : pCFG->mapStoryCFG)
  {
    QuestPreview* p = pStory->add_previews();
    p->CopyFrom(m.second);
    p->set_name(CommonConfig::getMe().getTranslateString(p->name(), m_pUser->getLanguage()));
    p->set_complete(QuestManager::getMe().isQuestComplete(version, m.first, m_pUser));
    if (p->complete() == true)
      p->clear_allrewardid();
  }

  for (auto &s : setIDs)
  {
    const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(s);
    if (pCFG != nullptr)
    {
      QuestName* pName = pManual->add_prequest();
      pName->set_id(pCFG->id);
      pName->set_name(CommonConfig::getMe().getTranslateString(pCFG->questname, m_pUser->getLanguage()));
    }
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[任务-任务手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "查询 ver :" << version << "数据成功,耗时" << frameDebug.uElapse() << "数据大小" << cmd.ByteSize() << "数据" << cmd.ShortDebugString() << XEND;
}

void Quest::openPuzzle(const string& version, DWORD dwID)
{
  const SVersionPuzzleCFG* pCFG = QuestConfig::getMe().getQuestPuzzleCFG(version);
  if (pCFG == nullptr)
  {
    XERR << "[任务-激活碎片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活" << version << dwID << "的碎片失败,version未在 Table_QuestPuzzle.txt 表中找到" << XEND;
    return;
  }

  const SQuestPuzzleCFG* pPuzzleCFG = pCFG->getActivePuzzleCFG(dwID);
  if (pPuzzleCFG == nullptr || pPuzzleCFG->vecQuestCFG.empty())
  {
    XERR << "[任务-激活碎片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活" << version << dwID << "的碎片失败,version中未包含任务" << XEND;
    return;
  }

  const TVecQuestCFG& vecCFG = pPuzzleCFG->vecQuestCFG;
  SQuestPuzzle& rPuzzle = m_mapPuzzle[version];
  auto s = rPuzzle.setOpen.find(pPuzzleCFG->dwIndex);
  if (s != rPuzzle.setOpen.end())
  {
    XERR << "[任务-激活碎片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活" << version << dwID << "的碎片失败,该碎片已激活" << XEND;
    return;
  }

  for (auto &v : vecCFG)
  {
    if (isSubmit(v.id) == false)
    {
      XERR << "[任务-激活碎片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活" << version << dwID << "的碎片失败,碎片已激活" << XEND;
      return;
    }
  }

  TVecItemInfo vecRewards;
  for (auto &s : pPuzzleCFG->setRewards)
  {
    TVecItemInfo vecSingle;
    if (RewardManager::roll(s, m_pUser, vecSingle, ESOURCE_PUZZLE) == false)
    {
      XERR << "[任务-激活碎片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活" << version << dwID << "的碎片失败,reward :" << s << "随机失败" << XEND;
      return;
    }
  }
  m_pUser->getPackage().addItem(vecRewards, EPACKMETHOD_AVAILABLE);
  rPuzzle.setOpen.insert(pPuzzleCFG->dwIndex);
  refreshCollectPuzzles();

  OpenPuzzleQuestCmd cmd;
  cmd.set_version(version);
  cmd.set_id(pPuzzleCFG->dwIndex);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[任务-激活碎片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活" << version << dwID << "的碎片成功" << XEND;
}

