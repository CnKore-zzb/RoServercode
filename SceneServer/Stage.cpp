#include "Stage.h"
#include "SceneUser.h"
#include "FuBenCmd.pb.h"
#include "SceneManager.h"

std::map<DWORD, StageConfig> UserStage::s_cfg;
std::map<DWORD, StageStepID> UserStage::s_raidStage;

void StageNormal::getLastStep(DWORD &stageID, DWORD &stepID)
{
  stageID = m_dwStageID;
  auto it = m_list.rbegin();
  if (it!=m_list.rend())
  {
    stepID = it->first;
  }
}

void StageHard::getLastStep(DWORD &stageID, DWORD &stepID)
{
  stageID = m_dwStageID;
  auto it = m_list.rbegin();
  if (it!=m_list.rend())
  {
    stepID = it->first;
  }
}

UserStage::UserStage(SceneUser *u):
  m_pUser(u),m_dwResetTime(0)
{
}

UserStage::~UserStage()
{
  for (int i=0; i<STAGE_TYPE_MAX; ++i)
  {
    for (auto it=m_list[i].begin(); it!=m_list[i].end(); ++it)
    {
      SAFE_DELETE(it->second);
    }
    m_list[i].clear();
  }
}

bool UserStage::loadConfig()
{
  std::map<DWORD, DWORD> tmp;
  {
    if (!xLuaTable::getMe().open("Lua/Table/Table_Ectype.txt"))
    {
      XERR << "[表格],加载表格,Table_Ectype.txt,失败" << XEND;
      return false;
    }

    s_cfg.clear();
    s_raidStage.clear();

    xLuaTableData table;
    xLuaTable::getMe().getLuaTable("Table_Ectype", table);

    for (auto it=table.begin(); it!=table.end(); ++it)
    {
      int stageID = it->first;
      StageConfig &item = s_cfg[stageID];
      item.stageID = stageID;
      item.name = it->second.getTableString("name");
      const xLuaData &data = it->second.getData("NormalStageID");
      for (auto iter=data.m_table.begin(); iter!=data.m_table.end(); ++iter)
      {
        // item.subList[iter->second.getInt()];
        tmp[iter->second.getInt()] = stageID;
      }
      const xLuaData &harddata = it->second.getData("HardStageID");
      for (auto iter=harddata.m_table.begin(); iter!=harddata.m_table.end(); ++iter)
      {
        // item.subList[iter->second.getInt()];
        tmp[iter->second.getInt()] = stageID;
      }
      item.normalStarRewardList[it->second.getTableInt("rewardsValue1")] = it->second.getTableInt("rewardID1");
      item.normalStarRewardList[it->second.getTableInt("rewardsValue2")] = it->second.getTableInt("rewardID2");
      item.normalStarRewardList[it->second.getTableInt("rewardsValue3")] = it->second.getTableInt("rewardID3");
      XLOG << "[stage] " << item.stageID << item.name << "(" << it->second.getTableInt("rewardsValue1") << it->second.getTableInt("rewardID1")
        << it->second.getTableInt("rewardsValue2") << it->second.getTableInt("rewardID2") << it->second.getTableInt("rewardsValue3") << it->second.getTableInt("rewardID3") << ")" << XEND;
    }
  }

  /*{
    if (!xLuaTable::getMe().open("Lua/Table/Table_Stage.txt"))
    {
      XERR("[表格],加载表格,Table_Stage.txt,失败");
      return false;
    }
    XLOG("[表格],加载表格Table_Stage.txt");

    xLuaTableData table;
    xLuaTable::getMe().getLuaTable("Table_Stage", table);

    for (auto it=table.begin(); it!=table.end(); ++it)
    {
      if (tmp.find(it->first)==tmp.end()) continue;

      DWORD stageID = tmp[it->first];
      if (s_cfg.find(stageID)==s_cfg.end()) continue;

      StageConfig &item = s_cfg[stageID];

      StageStepConfig subItem;
      subItem.stageID = stageID;
      subItem.stepID = it->second.getTableInt("Step");
      if (!subItem.stepID) continue;

      subItem.name = it->second.getTableString("Name");
      subItem.raidID = it->second.getTableInt("RaidID");
      subItem.baseExp = it->second.getTableInt("BaseExp");
      subItem.jobExp = it->second.getTableInt("JobExp");
      subItem.silver = it->second.getTableInt("Silver");
      subItem.gold = it->second.getTableInt("Gold");

      s_raidStage[subItem.raidID].stageID = subItem.stageID;
      s_raidStage[subItem.raidID].stepID = subItem.stepID;

      DWORD type = it->second.getTableInt("Type");
      s_raidStage[subItem.raidID].type = type;
      if (type)
      {
        item.hardStepList[subItem.stepID] = subItem;
      }
      else
      {
        item.normalStepList[subItem.stepID] = subItem;
      }

      XLOG("[substage],%u,%u,%s,(%u)", subItem.stageID, subItem.stepID, subItem.name.c_str(), subItem.raidID);
    }
  }*/

  return true;
}

StageConfig* UserStage::getStageConfig(DWORD stageID)
{
  auto it = s_cfg.find(stageID);
  if (it==s_cfg.end()) return NULL;

  return &(it->second);
}

StageStepConfig* UserStage::getStageStepConfig(DWORD stageID, DWORD stepID, StageType type)
{
  auto it = s_cfg.find(stageID);
  if (it==s_cfg.end()) return NULL;

  switch (type)
  {
    case STAGE_TYPE_NORMAL:
      {
        auto iter = it->second.normalStepList.find(stepID);
        if (iter==it->second.normalStepList.end()) return NULL;
        return &(iter->second);
      }
      break;
    case STAGE_TYPE_HARD:
      {
        auto iter = it->second.hardStepList.find(stepID);
        if (iter==it->second.hardStepList.end()) return NULL;
        return &(iter->second);
      }
      break;
    default:
      break;
  }
  return NULL;
}

void UserStage::load(const Cmd::BlobStage &stage)
{
  m_dwResetTime = stage.resettime();

  int size = stage.list_size();
  for (int i=0; i<size; ++i)
  {
    const Cmd::StageBlobItem &item = stage.list(i);
    if (item.normalist_size())
    {
      StageNormal *pStg = (StageNormal *)addStage(item.stageid(), STAGE_TYPE_NORMAL);
      if (pStg)
      {
        for (int j=0; j<item.normalist_size(); ++j)
        {
          pStg->m_list[item.normalist(j).stepid()] = item.normalist(j).star();
        }
        for (int j=0; j<item.gets_size(); ++j)
        {
          pStg->m_reward.insert(item.gets(j));
        }
      }
    }
    if (item.hardlist_size())
    {
      StageHard *pStg = (StageHard *)addStage(item.stageid(), STAGE_TYPE_HARD);
      if (pStg)
      {
        for (int j=0; j<item.hardlist_size(); ++j)
        {
          pStg->m_list[item.hardlist(j).stepid()].m_dwRestTime = item.hardlist(j).time();
          pStg->m_list[item.hardlist(j).stepid()].m_dwFinish = item.hardlist(j).finish();
        }
      }
    }
  }
}

void UserStage::save(Cmd::BlobStage *data)
{
  if (!data) return;
  data->Clear();
  data->set_resettime(m_dwResetTime);

  for (auto it=m_list[STAGE_TYPE_NORMAL].begin(); it!=m_list[STAGE_TYPE_NORMAL].end(); ++it)
  {
    StageNormal *pStg = (StageNormal *)(it->second);
    if (!pStg) continue;

    Cmd::StageBlobItem *pItem = data->add_list();
    pItem->set_stageid(it->first);
    for (auto iter=pStg->m_reward.begin(); iter!=pStg->m_reward.end(); ++iter)
    {
      pItem->add_gets(*iter);
    }

    for (auto iter=pStg->m_list.begin(); iter!=pStg->m_list.end(); ++iter)
    {
      Cmd::StageStepNormalBlob *pStep = pItem->add_normalist();
      pStep->set_stepid(iter->first);
      pStep->set_star(iter->second);
    }

    StageHard *pHard = (StageHard *)getStage(it->first, STAGE_TYPE_HARD);
    if (pHard)
    {
      for (auto iter=pHard->m_list.begin(); iter!=pHard->m_list.end(); ++iter)
      {
        Cmd::StageStepHardBlob *pStep = pItem->add_hardlist();
        pStep->set_stepid(iter->first);
        pStep->set_finish(iter->second.m_dwFinish);
        pStep->set_time(iter->second.m_dwRestTime);
      }
    }
  }

  XDBG << "[舞台-保存] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 数据大小 : " << data->ByteSize() << XEND;
}

Stage* UserStage::getStage(DWORD stageID, StageType type)
{
  if (type >= STAGE_TYPE_MAX) return NULL;

  auto it = m_list[type].find(stageID);
  if (it==m_list[type].end()) return NULL;

  return it->second;
}

Stage* UserStage::addStage(DWORD stageID, StageType type)
{
  if (type >= STAGE_TYPE_MAX || !stageID) return NULL;

  auto it = m_list[type].find(stageID);
  if (it!=m_list[type].end()) return it->second;

  Stage *pStg = NULL;
  switch (type)
  {
    case STAGE_TYPE_NORMAL:
      {
        pStg = NEW StageNormal();
      }
      break;
    case STAGE_TYPE_HARD:
      {
        pStg = NEW StageHard();
      }
      break;
    default:
      break;
  }

  if (!pStg) return NULL;

  pStg->m_dwStageID = stageID;

  m_list[type][stageID] = pStg;

  return pStg;
}

void UserStage::getNextStep(DWORD &stageID, DWORD &stepID, StageType type)
{
  stageID = 1;
  stepID = 0;

  if (type >= STAGE_TYPE_MAX) return;

  auto it = m_list[type].rbegin();
  if (it==m_list[type].rend()) return;

  if (it->second)
    it->second->getLastStep(stageID, stepID);
}

void UserStage::send()
{
  Cmd::WorldStageUserCmd message;
  auto list = m_list[STAGE_TYPE_NORMAL];
  for (auto it=list.begin(); it!=list.end(); ++it)
  {
    StageNormal *pStg = (StageNormal *)(it->second);
    if (!pStg) continue;

    Cmd::WorldStageItem *item = message.add_list();
    item->set_id(it->first);
    item->set_star(pStg->getStarNum());

    for (auto iter=pStg->m_reward.begin(); iter!=pStg->m_reward.end(); ++iter)
    {
      item->add_getlist(*iter);
    }
  }
  DWORD stageID = 0;
  DWORD stepID = 0;
  getNextStep(stageID, stepID, STAGE_TYPE_NORMAL);
  if (getStageStepConfig(stageID, stepID+1, STAGE_TYPE_NORMAL))
  {
    stepID++;
  }
  else
  {
    stageID++;
    stepID = 1;
  }
  Cmd::StageStepItem *pCur = message.add_curinfo();
  pCur->set_stageid(stageID);
  pCur->set_stepid(stepID);
  pCur->set_type(0);
  PROTOBUF(message, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserStage::send(DWORD stageID)
{
  Cmd::StageStepUserCmd message;
  message.set_stageid(stageID);
  StageNormal *pStg = (StageNormal *)getStage(stageID, STAGE_TYPE_NORMAL);
  if (pStg)
  {
    for (auto iter=pStg->m_list.begin(); iter!=pStg->m_list.end(); ++iter)
    {
      Cmd::StageNormalStepItem *item = message.add_normalist();
      item->set_stepid(iter->first);
      item->set_star(iter->second);
    }
  }
  StageHard *pHard = (StageHard *)getStage(stageID, STAGE_TYPE_HARD);
  if (pHard)
  {
    for (auto iter=pHard->m_list.begin(); iter!=pHard->m_list.end(); ++iter)
    {
      Cmd::StageHardStepItem *item = message.add_hardlist();
      item->set_stepid(iter->first);
      item->set_challengetime(iter->second.m_dwRestTime);
      item->set_finish(iter->second.m_dwFinish);
    }
  }
  PROTOBUF(message, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserStage::start(DWORD stageID, DWORD stepID, DWORD type)
{
  StageStepConfig *pCfg = getStageStepConfig(stageID, stepID, (StageType)type);
  if (pCfg)
  {
    SceneManager::getMe().createDScene(CreateDMapParams(m_pUser->id, pCfg->raidID));
    //m_pUser->createDMap(pCfg->raidID);
  }
}

void UserStage::addStar(DWORD dmapID, DWORD starNum)
{
  if (s_raidStage.find(dmapID)==s_raidStage.end()) return;

  StageStepID &item = s_raidStage[dmapID];

  if (STAGE_TYPE_NORMAL==item.type)
  {
    StageNormal *pStg = (StageNormal *)addStage(item.stageID, STAGE_TYPE_NORMAL);
    if (pStg)
    {
      if (starNum > pStg->m_list[item.stepID])
      {
        pStg->m_list[item.stepID] = starNum;
      }
    }
  }
  else if (STAGE_TYPE_HARD==item.type)
  {
    /*
    StageNormal *pStg = (StageNormal *)addStage(item.stageID, STAGE_TYPE_NORMAL);
    if (pStg)
    {
      if (starNum > pStg->m_list[item.stepID])
      {
        pStg->m_list[item.stepID] = starNum;
      }
    }
    */
  }

  Cmd::StageStepStarUserCmd message;
  message.set_stageid(item.stageID);
  message.set_stepid(item.stepID);
  message.set_type(item.type);
  message.set_star(starNum);
  PROTOBUF(message, send, len);
  m_pUser->sendCmdToMe(send, len);

  StageStepConfig *pCfg = getStageStepConfig(item.stageID, item.stepID, (StageType)item.type);
  if (pCfg)
  {
    m_pUser->addBaseExp(pCfg->baseExp, ESOURCE_FUBEN);
    m_pUser->addJobExp(pCfg->jobExp, ESOURCE_FUBEN);
    m_pUser->addMoney(EMONEYTYPE_SILVER, pCfg->silver, ESOURCE_FUBEN);
    m_pUser->addMoney(EMONEYTYPE_GOLD, pCfg->gold, ESOURCE_FUBEN);
  }
}

void UserStage::getReward(DWORD stageID, DWORD starID)
{
  StageConfig *pCfg = getStageConfig(stageID);
  if (!pCfg) return;

  if (pCfg->normalStarRewardList.find(starID)==pCfg->normalStarRewardList.end())
  {
    return;
  }
  StageNormal *pStg = (StageNormal *)getStage(stageID, STAGE_TYPE_NORMAL);
  if (!pStg) return;

  // 星数不足
  if (pStg->getStarNum() < starID) return;

  // 已领奖
  if (pStg->m_reward.find(starID)!=pStg->m_reward.end()) return;

  pStg->m_reward.insert(starID);

  m_pUser->getPackage().rollReward(pCfg->normalStarRewardList[starID], EPACKMETHOD_AVAILABLE);

  Cmd::GetRewardStageUserCmd message;
  message.set_stageid(stageID);
  message.set_starid(starID);
  PROTOBUF(message, send, len);
  m_pUser->sendCmdToMe(send, len);
}
