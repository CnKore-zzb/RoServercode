#include "FuBenPhase.h"
#include "ConfigManager.h"
#include "DScene.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "SceneUser.h"
#include "PlatLogManager.h"
#include "MsgManager.h"
#include "SceneManager.h"
#include "GuidManager.h"
#include "GMCommandRuler.h"
#include "SceneServer.h"
#include "CarrierCmd.pb.h"
#include "StatisticsDefine.h"
#include "Dojo.pb.h"
#include "GuildRaidConfig.h"
#include "ActivityManager.h"
#include "ActivityEventManager.h"

// fuben phase
bool FuBenPhase::init()
{
  m_dwPhaseStarID = m_data.getTableInt("StarID");
  m_type = m_data.getTableString("Content");
  return true;
}

void FuBenPhase::syncDelThisStep(SceneUser* user)
{
  if (user == nullptr)
    return;
  FubenStepSyncCmd cmd;
  cmd.set_id(m_dwID);
  cmd.set_del(true);
  PROTOBUF(cmd, send, len);

  user->sendCmdToMe(send, len);
  XDBG << "[副本步骤-删除]" << user->accid << user->id << user->getProfession() << user->name << "通知客户端删除步骤" << m_dwID << XEND;
}

void FuBenPhase::initPConfig(RaidPConfig* pConfig)
{
  if (pConfig == nullptr)
    return;

  pConfig->set_raidid(m_data.getTableInt("RaidID"));
  pConfig->set_starid(m_data.getTableInt("StarID"));
  pConfig->set_auto_(m_data.getTableInt("Auto"));
  pConfig->set_whethertrace(m_data.getTableInt("WhetherTrace"));
  pConfig->set_descinfo(m_data.getTableString("DescInfo"));
  pConfig->set_content(m_data.getTableString("Content"));
  pConfig->set_traceinfo(m_data.getTableString("TraceInfo"));

  xLuaData& params = m_data.getMutableData("Params");
  params.toData(pConfig->mutable_params());
}

bool FuBenPhase::checkClient(DScene* s)
{
  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  bool find = false;
  for (auto entryIt : entrySet)
  {
    SceneUser *pUser = (SceneUser *)(entryIt);
    if (pUser && pUser->getClientFubenID() == m_dwID)
    {
      find = true;
      break;
    }
  }
  if (!find)
    return false;

  for (auto entryIt : entrySet)
  {
    SceneUser *pUser = (SceneUser *)(entryIt);
    if (pUser)
    {
      syncDelThisStep(pUser);
      pUser->setClientFubenID(0);
    }
  }
  return true;
}

// story phase
bool StoryFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  /*xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
  }*/

  return false;
}

bool StoryFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

// rush phase
bool RushFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  /*
  if (!_trig_range)
  {
    DWORD cur = now();
    RushManager::getMe().addRush(scene, _rush_id, cur, cur + 7200, true, 0, 0, 0, scene->getOwnerID());
    RushManager::getMe().timer(cur);
    return true;
  }

  xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    SceneUser *user = (SceneUser *)(*entryIt);
    if (checkRadius(user->getPos(), _trig_pos, _trig_range))
    {
      DWORD cur = now();
      RushManager::getMe().addRush(scene, _rush_id, cur, cur + 7200, true, 0, 0, 0, scene->getOwnerID());
      RushManager::getMe().timer(cur);
      return true;
    }
  }
  */

  return false;
}

bool RushFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

// track phase
bool TrackFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  scene->getFuben().m_oVars[getStarID()].m_track.clear();
  scene->getFuben().m_oVars[getStarID()].m_track = m_track;
  scene->getFuben().sendInfo(NULL);

  return true;
}

bool TrackFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_track = m_data.getMutableData("Params").getTableString("text");
  return true;
}
/*bool KillAllFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_NPC, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    SceneNpc *npc = (SceneNpc *)(*entryIt);
    if (!npc->base)
      continue;
    if (!npc->isAlive())
      continue;
    if (m_oNpcIDSet.empty() || m_oNpcIDSet.find(npc->base->id)!=m_oNpcIDSet.end())
      return false;
  }

  return true;
}*/

// killall phase
bool KillAllFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  //if (scene->m_fuben.m_oVars[getStarID()].m_dwKillUniqueID!=m_dwGroupID) return false;
  auto v = find(m_vecGroupID.begin(), m_vecGroupID.end(), scene->getFuben().m_oVars[getStarID()].m_dwKillUniqueID);
  if (v == m_vecGroupID.end() && m_setRandGroupIDs.empty())
    return false;

  VecSceneNpc groupVec;
  for (auto v = m_vecGroupID.begin(); v != m_vecGroupID.end(); ++v)
    scene->getNpcVecByUniqueID(*v, groupVec);
  for (auto &v : m_setRandGroupIDs)
  {
    const TSetDWORD& setRandUIDs = MapConfig::getMe().getRaidMonsterUids(v);
    for (auto &s : setRandUIDs)
      scene->getNpcVecByUniqueID(s, groupVec);
  }
  for (auto v = groupVec.begin(); v != groupVec.end(); ++v)
  {
    SceneNpc* pNpc = (*v);
    if (!pNpc || !pNpc->getCFG())
      continue;
    if (pNpc->isAlive())
      return false;
  }

  xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt : entrySet)
  {
    SceneUser *pUser = (SceneUser *)(entryIt);
    if (pUser)
    {
      syncDelThisStep(pUser);
    }
  }
  return true;
}

bool KillAllFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  bool bCorrect = true;

  DWORD dwMonsterID = m_data.getMutableData("Params").getTableInt("monster");
  if (dwMonsterID != 0)
    m_oNpcIDSet.insert(dwMonsterID);

  auto groupf = [this](const string& str, xLuaData& data)
  {
    m_vecGroupID.push_back(data.getInt());
  };
  m_data.getMutableData("Params").getMutableData("group_id").foreach(groupf);

  auto randgroupf = [this](const string& str, xLuaData& data)
  {
    m_setRandGroupIDs.insert(data.getInt());
  };
  m_data.getMutableData("Params").getMutableData("rand_group_id").foreach(randgroupf);

  for (auto &s : m_oNpcIDSet)
  {
    if (NpcConfig::getMe().getNpcCFG(s) == nullptr)
    {
      XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "monster :" << s << "未在 Table_Monster.txt 表中找到" << XEND;
      bCorrect = false;
      continue;
    }
  }

  for (auto &v : m_vecGroupID)
  {
    if (v == 0)
    {
      XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "group_id :" << v << "不合法" << XEND;
      bCorrect = false;
      continue;
    }
  }

  return bCorrect;
}

// reward phase
bool RewardFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  bool bCorrect = true;

  m_dwRewardID = m_data.getMutableData("Params").getTableInt("id");
  m_blLeave = false;

  if (RewardConfig::getMe().getRewardCFG(m_dwRewardID) == nullptr)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "id :" << m_dwRewardID << "未在 Table_Reward.txt 表中找到" << XEND;
    bCorrect = false;
  }

  return bCorrect;
}

bool RewardFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    SceneUser *pUser = dynamic_cast<SceneUser *>(*entryIt);
    if (pUser != nullptr)
      pUser->getPackage().rollReward(m_dwRewardID, EPACKMETHOD_AVAILABLE);
  }

  if (m_blLeave)
  {
    scene->setCloseTime(now() + scene->getRaidEndTime());
  }

  return true;
}

// summon phase
bool SummonFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  const SceneObject *pObject = scene->getSceneObject();
  if (pObject)
  {
    const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(m_dwGroupID);
    if (pTemplate != nullptr)
    {
      NpcDefine tmpDefine = pTemplate->m_oDefine;
      tmpDefine.setPurify(m_dwPurify);
      SceneNpcManager::getMe().createNpc(scene, tmpDefine, pTemplate->m_dwNum);
    }
  }
  return true;
}

bool SummonFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwGroupID = m_data.getMutableData("Params").getTableInt("group_id");
  m_dwPurify = m_data.getMutableData("Params").getTableInt("purifyValue");

  if (m_dwGroupID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "group_id :" << m_dwGroupID << "不合法" << XEND;
    return false;
  }

  return true;
}

// win phase
bool WinFuBenPhase::exec(DScene *scene)
{
  if (!scene) return false;
  if (FUBEN_RESULT_FAIL==scene->getFuben().m_result) return false;

  LaboratoryScene* pLaboratoryScene = dynamic_cast<LaboratoryScene*>(scene);
  if (pLaboratoryScene != nullptr)
    pLaboratoryScene->finish();

  xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    SceneUser *user = dynamic_cast<SceneUser *>(*entryIt);
    if (user == nullptr)
      continue;

    if (m_dwEffect)
    {
      Cmd::SuccessFuBenUserCmd message;

      if (pLaboratoryScene != nullptr)
      {
        message.set_type(ERAIDTYPE_LABORATORY);
        message.set_param1(pLaboratoryScene->m_dwRoundNum);
        message.set_param2(pLaboratoryScene->m_mapCalcPointList[user->id]);
        message.set_param3(pLaboratoryScene->m_mapPointList[user->id]);
        message.set_param4(pLaboratoryScene->m_mapRewardTimes[user->id]);
      }
      PROTOBUF(message, send, len);
      user->sendCmdToMe(send, len);
    }

    if (!scene->getFuben().m_oStarIDSet.empty())
      user->m_stage.addStar(scene->getRaidID(), scene->getFuben().m_oStarIDSet.size());

    // check event
    user->getEvent().onPassRaid(scene->getRaidID(), true);

    const SServantCFG sSerCFG = MiscConfig::getMe().getServantCFG();
    if(scene->getRaidID() == sSerCFG.dwCemeteryRaid)
      user->getServant().onGrowthFinishEvent(ETRIGGER_CASTLE_RAID);
    else if(scene->getRaidID() == sSerCFG.dwSpaceRaid)
      user->getServant().onGrowthFinishEvent(ETRIGGER_NIGHTMARE_RAID);
    else if(scene->getRaidID() == sSerCFG.dwTerroristRaid)
      user->getServant().onGrowthFinishEvent(ETRIGGER_TERRORIST_RAID);

    {
      //关卡通过日志
      //Cmd::CheckpointLogCmd log;
      //log.set_cid(user->getUserSceneData().getPlatformId()); /*平台id*/
      //log.set_sid(user->getZoneID());
      //log.set_pid(user->id);
      //log.set_eid(0);     //TODO
      //log.set_time(now());
      //log.set_type(1);    //关卡类型，目前没有
      //log.set_cpid(scene->getDMapID()); //关卡ID
      //log.set_result(1);  //失败不打印
      //log.set_star(0);
      //int isPay = ((user->getUserSceneData().getCharge() > 0) ? 1 : 0);
      //log.set_ispay(isPay);
      //log.set_vip(0);
      //log.set_logid(GuidManager::getMe().newGuidStr(user->getZoneID(), 0));
      //log.set_isfirst(0); //是否是首次，目前没记录
      //PROTOBUF(log, send, len);
      //thisServer->sendCmdToLog(send, len);
      PlatLogManager::getMe().levelPassLog(thisServer,user->getUserSceneData().getPlatformId(),
        user->getZoneID(),
        user->id,
        0,
        scene->getMapID(),
        user->getUserSceneData().getCharge(),
        0);
    }
  }

  scene->getFuben().m_result = FUBEN_RESULT_SUCCESS;
  scene->setCloseTime(now() + scene->getRaidEndTime());
  return true;
}

bool WinFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwStageID = m_data.getMutableData("Params").getTableInt("stage");
  m_dwStageSubID = m_data.getMutableData("Params").getTableInt("step");
  m_dwEffect = m_data.getMutableData("Params").getTableInt("effect");
  return true;
}

// lose phase
bool LoseFuBenPhase::exec(DScene* pScene)
{
  if (pScene == nullptr)
    return false;

  xSceneEntrySet entrySet;
  pScene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(*entryIt);
    if (pUser == nullptr)
      continue;

    // check event
    pUser->getEvent().onPassRaid(pScene->getRaidID(), false);
  }

  pScene->getFuben().m_result = FUBEN_RESULT_FAIL;
  pScene->setCloseTime(now() + pScene->getRaidEndTime());
  return true;
}

bool LoseFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

// leave phase
bool LeaveFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  Cmd::LeaveFuBenUserCmd message;
  message.set_mapid(scene->id);
  PROTOBUF(message, send, len);
  //Chat::sendCmdArea(scene->id, send, len);

  return true;
}

bool LeaveFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

// kill phase
bool KillFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  FuBen& rFuben = scene->getFuben();
  if (rFuben.m_oVars[getStarID()].m_oKillNpcIDSet.empty() && !rFuben.m_oVars[getStarID()].m_dwGroupID)
  {
    rFuben.m_oVars[getStarID()].m_oKillNpcIDSet = m_oNpcIDSet;
    rFuben.m_oVars[getStarID()].m_dwGroupID = m_dwGroupID;
    return false;
  }

  if (rFuben.m_oVars[getStarID()].m_dwKillNpcNum >= m_dwNum)
  {
    rFuben.m_oVars[getStarID()].m_dwKillNpcNum = 0;
    rFuben.m_oVars[getStarID()].m_oKillNpcIDSet.clear();
    rFuben.m_oVars[getStarID()].m_dwGroupID = 0;
    return true;
  }

  return false;
}

bool KillFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  bool bCorrect = true;

  m_dwNum = m_data.getMutableData("Params").getTableInt("num");

  DWORD dwMonsterID = m_data.getMutableData("Params").getTableInt("monster");
  if (dwMonsterID != 0)
    m_oNpcIDSet.insert(dwMonsterID);
  m_dwGroupID = m_data.getMutableData("Params").getTableInt("group_id");

  for (auto &s : m_oNpcIDSet)
  {
    if (NpcConfig::getMe().getNpcCFG(s) == nullptr)
    {
      XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "monster :" << s << "未在 Table_Monster.txt 表中找到" << XEND;
      bCorrect = false;
      continue;
    }
  }

  if (m_dwNum == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "num :" << m_dwNum << "不合法" << XEND;
    bCorrect = false;
  }

  if (m_dwGroupID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "group_id:" << m_dwGroupID << "不合法" << XEND;
    bCorrect = false;
  }

  return bCorrect;
}

// clearnpc phase
bool ClearNpcFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_NPC, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    SceneNpc *npc = (SceneNpc *)(*entryIt);
    if (npc->isNoFunBenClear())
      continue;
    if (m_oNpcIDSet.find(npc->getNpcID())!=m_oNpcIDSet.end())
    {
      if (m_bEffect)
        GMCommandRuler::effect(npc, m_oEffect);
      if (m_eMethod == ECLEARMETHOD_LEAVE)
        npc->setStatus(ECREATURESTATUS_LEAVE);
      else if (m_eMethod == ECLEARMETHOD_DEAD)
        npc->setClearState();
    }
    if (m_dwGroupID && m_dwGroupID==npc->define.getUniqueID())
    {
      if (m_bEffect)
        GMCommandRuler::effect(npc, m_oEffect);
      if (m_eMethod == ECLEARMETHOD_LEAVE)
        npc->setStatus(ECREATURESTATUS_LEAVE);
      else if (m_eMethod == ECLEARMETHOD_DEAD)
        npc->setClearState();
    }
  }

  return true;
}

bool ClearNpcFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwGroupID = m_data.getMutableData("Params").getTableInt("group_id");
  if (m_dwGroupID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "\b] id :" << m_dwID << "group_id:" << m_dwGroupID << "不合法" << XEND;
    return false;
  }

  DWORD dwMethod = m_data.getMutableData("Params").getTableInt("type");
  if (dwMethod != ECLEARMETHOD_LEAVE && dwMethod != ECLEARMETHOD_DEAD)
  {
    XERR << "[副本配置-步骤" << m_type << "\b] id :" << m_dwID << "type:" << dwMethod << "不合法" << XEND;
    return false;
  }

  if (m_data.getMutableData("Params").has("effect") == true)
  {
    m_bEffect = true;
    m_oEffect = m_data.getMutableData("Params").getMutableData("effect");
  }

  return true;
}

// timer phase
bool TimerFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  if (m_dwQuestID != 0)
  {
    xSceneEntrySet entrySet;
    scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
    if (entrySet.empty() == true)
      return false;

    if (!m_bJustNeedOne)
    {
      for (auto s = entrySet.begin(); s != entrySet.end(); ++s)
      {
        SceneUser* pUser = dynamic_cast<SceneUser*>(*s);
        if (pUser == nullptr)
          return false;
        if (pUser->getQuest().getRecentSubmit() != m_dwQuestID && pUser->getQuest().isSubmit(m_dwQuestID) == false)
          return false;
      }
      return true;
    }
    else
    {
      for (auto s = entrySet.begin(); s != entrySet.end(); ++s)
      {
        SceneUser* pUser = dynamic_cast<SceneUser*>(*s);
        if (pUser == nullptr)
          return false;
        if (pUser->getQuest().getRecentSubmit() == m_dwQuestID || pUser->getQuest().isSubmit(m_dwQuestID))
          return true;
      }
      return false;
    }
  }

  FuBen& rFuben = scene->getFuben();
  if (0==rFuben.m_oVars[getStarID()].m_dwDelayTimer)
  {
    rFuben.m_oVars[getStarID()].m_dwDelayTimer = now() + m_dwDelayTime;
    if (m_dwNotifyID)
    {
      xSceneEntrySet entrySet;
      entrySet.clear();
      scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
      for (auto entryIt : entrySet)
        MsgManager::sendMsg(entryIt->id, m_dwNotifyID, MsgParams(m_dwDelayTime), EMESSAGETYPE_TIME_DOWN);
    }
    return false;
  }

  if (rFuben.m_oVars[getStarID()].m_dwDelayTimer < now())
  {
    rFuben.m_oVars[getStarID()].m_dwDelayTimer = 0;
    return true;
  }

  return false;
}

bool TimerFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwDelayTime = m_data.getMutableData("Params").getTableInt("time");
  m_dwTipFlag = m_data.getMutableData("Params").getTableInt("tip");
  m_dwNotifyID = m_data.getMutableData("Params").getTableInt("notify");
  m_dwQuestID = m_data.getMutableData("Params").getTableInt("quest");
  m_bJustNeedOne = m_data.getMutableData("Params").getTableInt("just_need_one") != 0;

  if (m_dwQuestID != 0)
  {
    if (QuestConfig::getMe().getQuestCFG(m_dwQuestID) == nullptr)
    {
      XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "quest:" << m_dwQuestID << "未在 Table_Quest.txt 表中找到" << XEND;
      return false;
    }
  }
  if (m_dwDelayTime == 0 && m_dwQuestID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "time:" << m_dwDelayTime << "quest:" << m_dwQuestID << "未存在等待条件" << XEND;
    return false;
  }

  return true;
}

// close phase
bool CloseFuBenPhase::exec(DScene *s)
{
  if (s == nullptr)
    return false;
  s->setCloseTime(now() + s->getRaidEndTime());
  return true;
}

bool CloseFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

// visit phase
bool VisitFuBenPhase::exec(DScene *s)
{
  if (checkClient(s) == true)
    return true;
  return false;
}

// dialog phase
bool DialogFuBenPhase::exec(DScene* s)
{
  if (checkClient(s) == true)
    return true;
  return false;
}

// star phase
bool StarFuBenPhase::exec(DScene *s)
{
  if (!s) return true;

  s->getFuben().m_oStarIDSet.insert(m_dwStarID);
  return true;
}

bool StarFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwStarID = m_data.getMutableData("Params").getTableInt("id");
  return true;
}

// gm phase
bool GMCmdFuBenPhase::exec(DScene *s)
{
  if (!s)
    return false;
  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    GMCommandRuler::getMe().execute((xSceneEntryDynamic *)(*entryIt), m_oGmCmd);
    return true;
  }
  s->getAllEntryList(SCENE_ENTRY_NPC, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    GMCommandRuler::getMe().execute((xSceneEntryDynamic *)(*entryIt), m_oGmCmd);
    return true;
  }
  return true;
}

bool GMCmdFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_oGmCmd = m_data.getMutableData("Params");
  return true;
}

// multi gm phase
bool MultiGMFuBenPhase::exec(DScene* s)
{
  if (!s)
    return false;
  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    GMCommandRuler::getMe().execute((xSceneEntryDynamic *)(*entryIt), m_oGmCmd);
  }
  return true;
}

bool MultiGMFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_oGmCmd = m_data.getMutableData("Params");
  return true;
}

// npc gm phase
bool NpcGMFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_oGmCmd = m_data.getMutableData("Params");
  return true;
}

bool NpcGMFuBenPhase::exec(DScene* s)
{
  if (!s)
    return false;
  DWORD npcuid = m_oGmCmd.getTableInt("npcUniqID");
  if (npcuid == 0)
    return false;
  VecSceneNpc groupVec;
  s->getNpcVecByUniqueID(npcuid, groupVec);
  for (auto &npc : groupVec)
  {
    GMCommandRuler::getMe().execute(npc, m_oGmCmd);
  }

  return true;
}

// move phase
bool MoveFuBenPhase::exec(DScene *s)
{
  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto entryIt : entrySet)
  {
    SceneUser *pUser = (SceneUser *)(entryIt);
    if (checkRadius(pUser->getPos(), m_oPos, m_dwDistance))
      return true;
  }
  return false;
}

bool MoveFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  const xLuaData &p = m_data.getMutableData("Params").getMutableData("pos");
  m_oPos.x = p.getTableFloat("1");
  m_oPos.y = p.getTableFloat("2");
  m_oPos.z = p.getTableFloat("3");
  m_dwDistance = m_data.getMutableData("Params").getTableInt("distance");
  return true;
}


// gear phase
bool GearFuBenPhase::exec(DScene *s)
{
  if (!s) return false;

  if (m_dwStateID==s->m_oGear.get(m_dwGearID))
    return true;

  return false;
}

bool GearFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwGearID = m_data.getMutableData("Params").getTableInt("id");
  m_dwStateID = m_data.getMutableData("Params").getTableInt("state");
  return true;
}

// wait phase
bool WaitStarFuBenPhase::exec(DScene *s)
{
  if (!s) return false;

  FuBen& rFuben = s->getFuben();
  if (rFuben.m_oStarIDSet.find(m_dwStarID) != rFuben.m_oStarIDSet.end())
    return true;

  return false;
}

bool WaitStarFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwStarID = m_data.getMutableData("Params").getTableInt("id");
  return true;
}

// append phase
bool AppendFuBenPhase::exec(DScene *s)
{
  TowerScene* pScene = dynamic_cast<TowerScene*>(s);
  if (nullptr == pScene)
    return false;

  xSceneEntrySet userSet;
  s->getAllEntryList(SCENE_ENTRY_USER, userSet);
  if (userSet.empty() == true)
    return false;

  auto it = userSet.begin();
  SceneUser *pTmpUser = dynamic_cast<SceneUser*>(*it);
  if(nullptr == pTmpUser || pTmpUser->getTeamID() == 0)
    return false;
  const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(pScene->getLayer() + 1);
  if (pCFG == nullptr)
    return true;

  s->getFuben().appendPhase(FuBen::s_cfg[pCFG->dwRaidID]);
  XLOG << "[副本-步骤-append], ID:" << m_dwID << "副本:" << pScene->name << pScene->id << "玩家数量:" << userSet.size() << XEND;
  return true;
}

bool AppendFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwMapRaid = m_data.getMutableData("Params").getTableInt("raid");
  if (m_dwMapRaid != 0)
  {
    if (MapConfig::getMe().getRaidCFG(m_dwMapRaid) == nullptr)
    {
      XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "raid :" << m_dwMapRaid << "未在 Table_MapRaid.txt 表中找到" << XEND;
      return false;
    }
  }
  return true;
}

// enter next layer phase
bool EnterLayerPhase::exec(DScene *s)
{
  TowerScene* pScene = dynamic_cast<TowerScene*>(s);
  if (pScene == nullptr)
    return false;

  xSceneEntrySet npcSet;
  pScene->getAllEntryList(SCENE_ENTRY_NPC, npcSet);
  for (auto it = npcSet.begin(); it != npcSet.end(); ++it)
  {
    SceneNpc *pNpc = dynamic_cast<SceneNpc*>(*it);
    if (nullptr != pNpc && !pNpc->isNoFunBenClear())
      pNpc->setStatus(ECREATURESTATUS_LEAVE);
  }

  // notify client enter next layer
  xSceneEntrySet userSet;
  pScene->getAllEntryList(SCENE_ENTRY_USER, userSet);
  if (userSet.empty() == true)
    return false;

  QWORD qwTeamID = 0;
  DWORD dwCurLayer = pScene->getLayer();
  DWORD dwNextLayer = dwCurLayer + 1;

  const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(dwCurLayer);
  const STowerLayerCFG* pNextCFG = TowerConfig::getMe().getTowerLayerCFG(dwNextLayer);
  if (pCFG == nullptr || pNextCFG == nullptr)
    return false;

  bool nomonster = pScene->getNoMonsterLayer() >= dwNextLayer;

  CreateDMapParams stParam;
  stParam.qwCharID = 0;
  std::stringstream ss;
  for (auto userIt = userSet.begin(); userIt != userSet.end(); ++userIt)
  {
    SceneUser *pUser = dynamic_cast<SceneUser*>(*userIt);
    if (nullptr == pUser)
      continue;

    pUser->updateTeamTower();
    pUser->setOwnSky(pNextCFG->dwSky);
    if (!pNextCFG->strBgm.empty() && pNextCFG->dwRaidID == pCFG->dwRaidID && pCFG->strBgm != pNextCFG->strBgm)
    {
      pUser->replaceBgmToMe(pNextCFG->strBgm);
    }

    if (pUser->isAlive() == false)
      pUser->relive(ERELIVETYPE_TOWER);

    if (dwNextLayer > TowerConfig::getMe().getMaxLayer())
    {
      MsgManager::sendMsg(pUser->id, 1316, MsgParams(dwCurLayer + 1));
      s->setCloseTime(now());
      return false;
      //pUser->gomap(MiscConfig::getMe().getSystemCFG().dwMainCityMapID, GoMapType::Tower);
      //continue;
    }

    if (pCFG->dwRaidID == pNextCFG->dwRaidID)
      MsgManager::sendMsg(pUser->id, 1306, MsgParams(dwCurLayer + 1));

    const SceneBase* pSBase = pScene->getSceneBase();
    if (pSBase == nullptr)
      continue;

    const SceneObject *pSObj = pSBase->getSceneObject(pNextCFG->dwRaidID);
    if (pSObj == nullptr)
      continue;

    // find born point
    xPos pos;
    const xPos* pBorn = pSObj->getBornPoint(1);
    if (pBorn == nullptr)
      continue;
    pos = *pBorn;

    if (nomonster)
    {
      const ExitPoint* pExit = pSObj->getExitPoint(1);
      if (pExit)
      {
        xPos ePos = pExit->m_oPos;
        DWORD i = 0;
        while(i++ < 30)
        {
          if (pScene->getCircleRoundPos(pExit->m_oPos, 5, ePos))
            break;
        }
        pos = ePos;
      }
    }

    if (qwTeamID == 0)
      qwTeamID = pUser->getTeamID();

    if (pCFG->dwRaidID == pNextCFG->dwRaidID)
    {
      ostringstream ostr;
      ostr << pScene->base->getMapInfo().name;
      ostr << dwNextLayer;
      Cmd::ChangeSceneUserCmd cmd;
      cmd.set_mapname(ostr.str());
      cmd.set_mapid(pScene->getMapID());
      cmd.set_dmapid(pScene->getRaidID());
      Cmd::ScenePos *p = cmd.mutable_pos();
      p->set_x(pos.getX());
      p->set_y(pos.getY());
      p->set_z(pos.getZ());

      const SceneObject *pObject = pScene->getSceneObject();
      if (pObject)
      {
        const map<DWORD, ExitPoint>& mapList = pObject->getExitPointList();
        for (auto it : mapList)
        {
          if (!it.second.isVisible(pUser, pScene))
            cmd.add_invisiblexit(it.second.m_dwExitID);
        }
      }
      PROTOBUF(cmd, send, len);
      pUser->sendCmdToMe(send, len);

      pUser->getUserSceneData().setOnlineMapPos(pScene->id, pos);
      pUser->setDataMark(EUSERDATATYPE_RAIDID);

      pUser->m_oBuff.add(MiscConfig::getMe().getSystemCFG().dwGoMapBuff);

      TowerLayerSyncTowerCmd sync;
      sync.set_layer(dwNextLayer);
      PROTOBUF(sync, syncsend, synclen);
      pUser->sendCmdToMe(syncsend, synclen);
      pScene->setChangeScene(true);
    }
    else
    {
      if (pUser->getTeamLeaderID() == pUser->id && stParam.qwCharID  == 0)
      {
        stParam.qwCharID = pUser->id;
      }
      else
      {
        stParam.vecMembers.push_back(pUser->id);
        ss << pUser->id << ":";
        if (pUser->getTeamLeaderID() == pUser->id)
        {
          XERR << "[无限塔-enterlayer] 队伍队长同步异常" << "队伍ID" << qwTeamID << "别人的队长" << stParam.qwCharID << "我认为的队长" << pUser->id << XEND;
        }
      }
    }   
  }
  XLOG << "[无限塔-enterlayer],通关当前层数:" << "teamid" << qwTeamID << "layer" << pScene->getLayer() << "副本信息:" << pScene->name << pScene->id << "玩家数量:" << userSet.size() << "队长" << stParam.qwCharID << "队员" << ss.str() << XEND;

  if (dwNextLayer <= TowerConfig::getMe().getMaxLayer())
  {
    pScene->setLayer(dwNextLayer);
    //else
      //pScene->setLayer(0);

    TowerLayerSyncSocialCmd cmd;
    cmd.set_teamid(qwTeamID);
    cmd.set_layer(pScene->getLayer());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }

  if (pCFG->dwRaidID != pNextCFG->dwRaidID)
  {
    pScene->setChangeRaid(true);
    stParam.dwRaidID = pNextCFG->dwRaidID;
    stParam.dwLayer = dwNextLayer;
    stParam.m_dwNoMonsterLayer = pScene->getNoMonsterLayer();
    return SceneManager::getMe().createDScene(stParam);
  }

  return true;
}

bool EnterLayerPhase::init()
{
  return FuBenPhase::init();
}

// tower reward phase
bool TowerRewardPhase::exec(DScene *s)
{
  TowerScene* pScene = dynamic_cast<TowerScene*>(s);
  if (pScene == nullptr)
    return false;

  Cmd::ExitPointState cmd;
  cmd.set_exitid(1);
  cmd.set_visible(1);
  PROTOBUF(cmd, send, len);

  const vector<pair<DWORD, float>>& vecRewardIDs = pScene->getAllRewardIDs();

  xSceneEntrySet userSet;
  pScene->getAllEntryList(SCENE_ENTRY_USER, userSet);
  std::set<SceneUser*> validUserSet;
  std::set<SceneUser*> invalidUserSet;
  std::set<SceneUser*> allUser;
  const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(pScene->getLayer());
  if (pCFG == nullptr)
  {
    XERR << "[无限塔奖励], 好不到对应层数配置, 层数:" << pScene->getLayer() << XEND;
    return false;
  }

  for (auto userIt = userSet.begin(); userIt != userSet.end(); ++userIt)
  {
    SceneUser *pUser = dynamic_cast<SceneUser*>(*userIt);
    if (nullptr == pUser || pUser->getTeamID() == 0)
      continue;

    pUser->sendCmdToMe(send, len);

    TVecItemInfo vecItemInfo;

    bool bReward = pUser->getTower().isRewarded(pScene->getLayer());
    if (!bReward)
    {
      rollAllReward(pUser, vecItemInfo, vecRewardIDs, pScene->getLayer());
      pUser->getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);

      pScene->summonUserRewardBox(pUser, false);
      pUser->getTower().passLayer(pScene->getLayer());
      pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_ENDLESS, pScene->getLayer());

      validUserSet.insert(pUser);
    }
    else
    {
      invalidUserSet.insert(pUser);

    }
    allUser.insert(pUser);

    TSetDWORD setMonsterIDs;
    for (auto v = pCFG->vecCurMonster.begin(); v != pCFG->vecCurMonster.end(); ++v)
      setMonsterIDs.insert(v->dwMonsterID);

    for(auto &m : setMonsterIDs)
    {
      const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m);
      if(pUser != nullptr && pCFG != nullptr && pCFG->vecRelevancyIDs.empty() == false)
      {
        for(auto &c : pCFG->vecRelevancyIDs)
        {
          pUser->getManual().onKillMonster(c);
          if(!bReward)
            pUser->getManual().onKillProcess(c);
        }
      }
    }

    // 奖励,装置默认unqueid = 1
    //if (!pUser->getTower().isRewarded(pTeam->getTower().getCurLayer()))
    //  GMCommandRuler::getMe().execute(pUser, "gear id=1 state=2");
  }

  if (pCFG->dwMvpCount != 0)
  {
    for (auto &user : invalidUserSet)
    {
      user->getHelpReward(validUserSet, EHELPTYPE_TOWER_MVP);
      user->stopSendInactiveLog();
    }
    for (auto &user : validUserSet)
    {
      user->getHelpReward(allUser, EHELPTYPE_TOWER_MVP, true);
    }
  }
  else if (pCFG->dwMiniCount != 0)
  {
    for (auto &user : invalidUserSet)
    {
      user->getHelpReward(validUserSet, EHELPTYPE_TOWER_MINI);
      user->stopSendInactiveLog();
    }
    for (auto &user : validUserSet)
    {
      user->getHelpReward(allUser, EHELPTYPE_TOWER_MINI, true);
    }
  }

  XLOG << "[无限塔-奖励], 副本信息:" << pScene->name << pScene->id << "层数:" << pScene->getLayer() << "当前玩家数量:" << userSet.size() << "奖励数量:" << pScene->getAllRewardIDs().size() << XEND;
  pScene->clearAllRewardIDs();// 清空掉落
  return true;
}

bool TowerRewardPhase::init()
{
  return FuBenPhase::init();
}

void TowerRewardPhase::rollAllReward(SceneUser* pUser, TVecItemInfo& vecItemInfo, const vector<pair<DWORD, float>> vecRewardIDs, DWORD layer)
{
    if (nullptr == pUser)
      return;

    const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(layer);
    if(pCFG == nullptr)
      return;

    TVecItemInfo vecReward;
    for (auto v = vecRewardIDs.begin(); v != vecRewardIDs.end(); ++v)
    {
      vecReward.clear();
      if (RewardManager::roll(v->first, pUser, vecReward, ESOURCE_TOWER, v->second) == true)
      {
        combinItemInfo(vecItemInfo, vecReward);
        XDBG << "[无限塔-怪物奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "在 layer :" << layer << "获得 reward :" << v->first << "ratio:" << v->second << XEND;
      }
    }

    vecReward.clear();
    if (RewardManager::roll(pCFG->dwRewardID, pUser, vecReward ,ESOURCE_TOWER) == true)
    {
      combinItemInfo(vecItemInfo, vecReward);
      XLOG << "[无限塔-塔楼奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "在 layer :" << layer << "获得 reward :" << pCFG->dwRewardID << "ratio: 1.0" << XEND;
    }

    pUser->getDoubleReward(EDOUBLEREWARD_ENDLESS, vecItemInfo);
    // 无限塔多倍奖励
    DWORD times = 0;
    TVecItemInfo extrareward;
    ActivityEventManager::getMe().getReward(pUser, EAEREWARDMODE_TOWER, layer, extrareward, times);
    if (times > 1)
    {
      for (auto &v : vecItemInfo)
      {
        v.set_count(v.count() * times);
        XLOG << "[无限塔-多倍奖励], 玩家:" << pUser->name << pUser->id << "层数:" << layer << "奖励:" << v.id() << "倍数:" << times << XEND;
      }
    }
    if (!extrareward.empty())
    {
      for (auto &v : extrareward)
        XLOG << "[无限塔-额外奖励], 玩家:" << pUser->name << pUser->id << "层数:" << layer << "奖励:" << v.id() << "数量:" << v.count() << XEND;
      combinItemInfo(vecItemInfo, extrareward);
    }

    float zenyratio = pUser->m_oBuff.getExtraZenyRatio(EEXTRAREWARD_ENDLESS);
    if (zenyratio)
    {
      XLOG << "[回归-Buff], 无限塔, 额外zeny倍率:" << zenyratio << "玩家:" << pUser->name << pUser->id << XEND;
      zenyratio += 1;
      for (auto &v : vecItemInfo)
      {
        if (v.id() == ITEM_ZENY)
          v.set_count(v.count() * zenyratio);
      }
      pUser->m_oBuff.onFinishEvent(EEXTRAREWARD_ENDLESS);
    }

    //pScene->summonUserRewardBox(pUser, false);      //是否需要特殊处理
    //pUser->getTower().passLayer(pScene->getLayer());
    //TVecItemInfo vecItem = vecItemInfo;
    //pUser->getPackage().addItemAvailable(vecItem);
    //pUser->getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);

    //pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_ENDLESS, layer);
}

// tower timer phase
bool TowerTimerPhase::exec(DScene *s)
{
  TowerScene* pScene = dynamic_cast<TowerScene*>(s);
  if (pScene == nullptr || pScene->isChangeScene())
    return false;

  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (true == entrySet.empty())
    return false;

  const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(pScene->getLayer());
  if (pCFG == nullptr)
    return false;

  const SceneBase* pSBase = SceneManager::getMe().getDataByID(s->getMapID());
  if (pSBase == nullptr)
    return false;

  DWORD dwCount = 0;
  const SceneObject *pSObj = pSBase->getSceneObject(pCFG->dwRaidID);
  if (pSObj)
  {
    // find exit point
    const ExitPoint* pPoint = pSObj->getExitPoint(1);
    if (pPoint == nullptr)
      return true;

    for (auto entryIt : entrySet)
    {
      SceneUser *pUser = (SceneUser *)(entryIt);
      if (checkRadius(pUser->getPos(), pPoint->m_oPos , m_dwDistance))
        dwCount++;
    }
  }

  STimerDownState & rTimerDownState = s->getFuben().getTimerDown();
  switch (rTimerDownState.eTimerState)
  {
    case ETIMERDOWNSTATE_STOP:
      {
        if (dwCount > 0)
        {
          rTimerDownState.eTimerState = ETIMERDOWNSTATE_RUN;
          rTimerDownState.dwTimerSec = 5;
          rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
        }
        else if (dwCount >= entrySet.size())
        {
          rTimerDownState.eTimerState = ETIMERDWONSTATE_PASS;
        }
        break;
      }
    case ETIMERDOWNSTATE_RUN:
      {
        if (dwCount == 0)
        {
          rTimerDownState.eTimerState = ETIMERDOWNSTATE_STOP;
          rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
          rTimerDownState.dwTimerSec = 5;
          // 取消倒计时
          for (auto entryIt : entrySet)
            MsgManager::sendMsg(entryIt->id, 1201, MsgParams(5), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
          break;
        }
        else if (dwCount >= entrySet.size())
        {
          rTimerDownState.eTimerState = ETIMERDWONSTATE_PASS;
          break;
        }
        // time count
        // notify client show time count down
        DWORD dwCurTimeSec = xTime::getCurSec();
        if (dwCurTimeSec <= rTimerDownState.dwRefreshTimeSec)
          break;

        if (0 == rTimerDownState.dwTimerSec)
        {
          rTimerDownState.eTimerState = ETIMERDWONSTATE_PASS;
          rTimerDownState.dwTimerSec = 5;
          break;
        }
        if (rTimerDownState.dwTimerSec == 5)
        {
          for (auto entryIt : entrySet)
            MsgManager::sendMsg(entryIt->id, 1201, MsgParams(5), EMESSAGETYPE_TIME_DOWN);
        }
        rTimerDownState.dwTimerSec--;
        rTimerDownState.dwRefreshTimeSec = dwCurTimeSec;
        break;
      }
    case ETIMERDWONSTATE_PASS:
      {
        // 取消倒计时
        for (auto entryIt : entrySet)
          MsgManager::sendMsg(entryIt->id, 1201, MsgParams(5), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
        rTimerDownState.eTimerState = ETIMERDOWNSTATE_STOP;
        rTimerDownState.dwTimerSec = 5;
        rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
        XLOG << "[无限塔-倒计时], 倒计时结束, 副本:" << pScene->name << pScene->id << "当前层数:" << pScene->getLayer() << XEND;
        return true;
        break;
      }
    default:
      break;
  }
  return false;
}

bool TowerTimerPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  const xLuaData &p = m_data.getMutableData("Params").getMutableData("pos");
  m_dwDistance = m_data.getMutableData("Params").getTableInt("distance");
  m_oExitPos.x = p.getTableFloat("1");
  m_oExitPos.y = p.getTableFloat("2");
  m_oExitPos.z = p.getTableFloat("3");

  if (0 == m_dwDistance)
    m_dwDistance = 1; //default

  return true;
}

// tower summon phase
bool TowerSummonPhase::exec(DScene *s)
{
  TowerScene* pScene = dynamic_cast<TowerScene*>(s);
  if (pScene == nullptr)
    return false;

  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (true == entrySet.empty())
    return false;

  bool noMonster = pScene->getLayer() <= pScene->getNoMonsterLayer();

  const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(pScene->getLayer());
  if (pCFG == nullptr)
    return false;
  s->getFuben().m_bCheckAgain = false;

  const SceneObject* pObject = nullptr;
  const SceneBase* pSBase = SceneManager::getMe().getDataByID(s->getMapID());
  if (pSBase == nullptr)
    return false;
  pObject = pSBase->getSceneObject(pCFG->dwRaidID);
  if (pObject == nullptr)
    return false;

  const SceneNpcTemplate* pRaidNpc = pObject->getRaidNpcTemplate(m_dwGroupID);
  if (pRaidNpc != nullptr)
  {
    NpcDefine tmpDefine = pRaidNpc->m_oDefine;

    //创建原有配置的npc
    if (0 < tmpDefine.getID() && tmpDefine.getID() != 1209)
      SceneNpcManager::getMe().createNpc(s, tmpDefine, pRaidNpc->m_dwNum);
  }

  for (auto v = pCFG->vecCurMonster.begin(); v != pCFG->vecCurMonster.end(); ++v)
  {
    const SceneNpcTemplate* pRaidNpc = pObject->getRaidNpcTemplate(v->dwUniqueID);
    if (pRaidNpc != nullptr)
    {
      NpcDefine tmpDefine = pRaidNpc->m_oDefine;

      //创建原有配置的npc
      if (0 < tmpDefine.getID() && tmpDefine.getID() != 1209)
        SceneNpcManager::getMe().createNpc(s, tmpDefine, pRaidNpc->m_dwNum);

      // 当前层不招小怪
      if (noMonster)
      {
        const SNpcCFG* npccfg = NpcConfig::getMe().getNpcCFG(v->dwMonsterID);
        if (npccfg && npccfg->eNpcType != ENPCTYPE_MVP && npccfg->eNpcType != ENPCTYPE_MINIBOSS)
        {
          TowerScene::addMonsterReward(v->dwMonsterID, pScene->getAllRewardIDs(), pScene->getLayer(), s->getMapID(), tmpDefine.getSuperAiNpc());
          continue;
        }
      }

      // 添加额外的AI
      if (pCFG->mapNpcExtraAIs.empty() == false)
      {
        const SNpcCFG* npccfg = NpcConfig::getMe().getNpcCFG(v->dwMonsterID);
        if (npccfg != nullptr)
        {
          const TSetDWORD& setids = pCFG->getAIsByType(npccfg->eNpcType);
          for (auto &s : setids)
            tmpDefine.addExtraAI(s);
        }
      }

      // 创建怪物池中的NPC
      tmpDefine.setID(v->dwMonsterID);
      tmpDefine.m_oVar.dwLayer = pScene->getLayer();
      SceneNpcManager::getMe().createNpc(tmpDefine, s);
    }
  }

  /*// summon small mini boss
  if (pCFG->vecCurMonster.empty() == false)
  {
    DWORD dwRand = randBetween(0, static_cast<DWORD>(pCFG->vecCurMonster.size() - 1));
    const STowerMonsterCFG& rMonsterCFG = pCFG->vecCurMonster[dwRand];
    const SceneNpcTemplate* pMini = pObject->getRaidNpcTemplate(rMonsterCFG.dwUniqueID);
    if (pMini != nullptr)
    {
      NpcDefine tmpDefine = pMini->m_oDefine;

      //创建原有配置的npc
      if (0 < tmpDefine.getID())
        SceneNpcManager::getMe().createNpc(s, tmpDefine, pMini->m_dwNum);

      const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
      tmpDefine.setScaleMin(rCFG.pairMiniScale.first);
      tmpDefine.setScaleMax(rCFG.pairMiniScale.second);

      // 创建怪物池中的NPC
      tmpDefine.setID(rMonsterCFG.dwMonsterID);
      tmpDefine.m_oVar.dwLayer = pScene->getLayer();
      tmpDefine.m_oVar.dwSpecial = ENPCTYPE_SMALLMINIBOSS;
      SceneNpcManager::getMe().createNpc(tmpDefine, s);
    }
  }
  else
  {
    XERR << "[副本步骤-mini怪召唤] 怪物列表 :" << static_cast<DWORD>(pCFG->vecCurMonster.size()) << XEND;
  }*/

  // summon box
  for (auto v : entrySet)
    pScene->summonUserRewardBox(dynamic_cast<SceneUser*>(v), true);

  // 设置宝箱装置的状态
  /*for (auto u : entrySet)
  {
    SceneUser *pUser = (SceneUser*)u;
    if (nullptr == pUser)
      continue;
    DWORD dwState = pUser->getTower().isRewarded(pTeam->getTower().getCurLayer()) == true ? 2 : 1;
    if (0 != dwState)
      s->m_oGear.set(1, dwState, pUser);
  }*/
  return true;
}

bool TowerSummonPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwGroupID = m_data.getMutableData("Params").getTableInt("group_id");
  if (m_dwGroupID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "group_id:" << m_dwGroupID << "不合法" << XEND;
    return false;
  }
  return true;
}

// tower killall phase
bool TowerKillAllPhase::exec(DScene *s)
{
  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_NPC, entrySet);

  for (auto it = entrySet.begin(); it != entrySet.end(); ++it)
  {
    SceneNpc *pNpc = dynamic_cast<SceneNpc*>(*it);
    if (nullptr == pNpc)
      continue;
    if (pNpc->define.m_oVar.m_qwFollowerID)
      continue;
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(pNpc->define.getID());
    if (nullptr == pCFG)
      continue;
    if (pCFG->eNpcType == ENPCTYPE_MONSTER || pCFG->eNpcType == ENPCTYPE_MINIBOSS || pCFG->eNpcType == ENPCTYPE_MVP)
    {
      if (pNpc->getStatus() != ECREATURESTATUS_DEAD)
        return false;
    }
  }

  return true;
}

bool TowerKillAllPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

// player num phase
bool PlayerNumPhase::exec(DScene *s)
{
  xSceneEntrySet userSet;
  s->getAllEntryList(SCENE_ENTRY_USER, userSet);
  if (userSet.size() < m_dwNum)
    return false;
  return true;
}

bool PlayerNumPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwNum = m_data.getMutableData("Params").getTableInt("num");
  if (0 == m_dwNum)
    m_dwNum = 1;
  return true;
}

// carrier phase
bool CarrierFuBenPhase::exec(DScene *s)
{
  return true;
  if (m_sType == "create")
  {
    xSceneEntrySet userSet;
    s->getAllEntryList(SCENE_ENTRY_USER, userSet);
    for (auto it = userSet.begin(); it != userSet.end(); ++it)
    {
      SceneUser *pUser = dynamic_cast<SceneUser*>(*it);
      if (nullptr != pUser)
      {
        pUser->m_oCarrier.clear();
        // 创建载具
        GMCommandRuler::getMe().execute(pUser, m_sGMCmd);
        // 保存载具信息
        s->getFuben().m_stCarrierInfo.dwCarrierID = pUser->m_oCarrier.m_oData.m_dwCarrierID;
        s->getFuben().m_stCarrierInfo.qwMasterID = pUser->m_oCarrier.m_oData.m_qwMasterID;

        return true;
      }
    }
  }
  else if (m_sType == "join")
  {
    // 非 MasterID jion载具
    JoinCarrierUserCmd cmd;
    cmd.set_masterid(s->getFuben().m_stCarrierInfo.qwMasterID);
    cmd.set_carrierid(s->getFuben().m_stCarrierInfo.dwCarrierID);
    cmd.set_agree(true);
    PROTOBUF(cmd, send, len);
    xSceneEntrySet userSet;
    s->getAllEntryList(SCENE_ENTRY_USER, userSet);
    for (auto it = userSet.begin(); it != userSet.end(); ++it)
    {
      SceneUser *pUser = dynamic_cast<SceneUser*>(*it);
      if (nullptr != pUser && pUser->id != s->getFuben().m_stCarrierInfo.qwMasterID)
      {
        pUser->doUserCmd((const Cmd::UserCmd *)send, len);
      }
    }
  }
  else if (m_sType == "move")
  {
    xSceneEntrySet userSet;
    s->getAllEntryList(SCENE_ENTRY_USER, userSet);
    for (auto it = userSet.begin(); it != userSet.end(); ++it)
    {
      SceneUser *pUser = dynamic_cast<SceneUser*>(*it);
      if (nullptr != pUser && pUser->id == s->getFuben().m_stCarrierInfo.qwMasterID)
      {
        GMCommandRuler::getMe().execute(pUser, m_sGMCmd);
        return true;
      }
    }
  }

  return true;
}

bool CarrierFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_sType = m_data.getMutableData("Params").getTableString("type");
  m_sGMCmd = m_data.getMutableData("Params").getTableString("GM");
  return true;
}

// timer down phase
bool TimerDownPhase::exec(DScene *s)
{

  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);

  STimerDownState & rTimerDownState = s->getFuben().getTimerDown();
  switch (rTimerDownState.eTimerState)
  {
    case ETIMERDOWNSTATE_STOP:
      {
          rTimerDownState.eTimerState = ETIMERDOWNSTATE_RUN;
          rTimerDownState.dwTimerSec = 5;
          rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
          break;
      }
    case ETIMERDOWNSTATE_RUN:
      {
        // time count
        // notify client show time count down
        DWORD dwCurTimeSec = xTime::getCurSec();
        if (dwCurTimeSec <= rTimerDownState.dwRefreshTimeSec)
          break;

        if (0 == rTimerDownState.dwTimerSec)
        {
          rTimerDownState.eTimerState = ETIMERDWONSTATE_PASS;
          rTimerDownState.dwTimerSec = 5;
          break;
        }
        if (rTimerDownState.dwTimerSec == 5)
        {
          for (auto entryIt : entrySet)
          {
            SceneUser *pUser = (SceneUser *)(entryIt);
            if (nullptr == pUser)
              continue;
            MsgManager::sendMsg(pUser->id, 1201, MsgParams(5), EMESSAGETYPE_TIME_DOWN);
          }
        }
        rTimerDownState.dwTimerSec--;
        rTimerDownState.dwRefreshTimeSec = dwCurTimeSec;
        break;
      }
    case ETIMERDWONSTATE_PASS:
      {
        // 取消倒计时
        for (auto entryIt : entrySet)
          MsgManager::sendMsg(entryIt->id, 1201, MsgParams(5), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
        rTimerDownState.eTimerState = ETIMERDOWNSTATE_STOP;
        rTimerDownState.dwTimerSec = 5;
        rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
        return true;
        break;
      }
    default:
      break;
  }

  return true;
}

bool TimerDownPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwTimeSec = 5;
  m_dwTimeSec = m_data.getMutableData("Params").getTableInt("sec");
  return true;
}

// change monster character phase
bool ChangeMonsterCharacPhase::exec(DScene *s)
{
  return true;
}

bool ChangeMonsterCharacPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

// laboratory monster phase
bool LaboratoryMonsterPhase::exec(DScene *scene)
{
  LaboratoryScene* pScene = dynamic_cast<LaboratoryScene*>(scene);
  if (pScene == nullptr)
    return false;

  bool &isStart = pScene->getFuben().m_oVars[getStarID()].m_blStart;
  if (false == isStart)
  {
    pScene->summon(m_dwRoundID);
    isStart = true;
    return false;
  }
  else
  {
    xSceneEntrySet entrySet;
    pScene->getAllEntryList(SCENE_ENTRY_NPC, entrySet);
    for (auto &it : entrySet)
    {
      LabNpc *npc = dynamic_cast<LabNpc *>(it);
      if (npc != nullptr && npc->getLaboratoryPoint())
        return false;
    }

    isStart = false;

    return true;
  }

  return true;
}

bool LaboratoryMonsterPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwRoundID = m_data.getMutableData("Params").getTableInt("round_id");
  return true;
}

// monster count phase
bool MonsterCountPhase::exec(DScene *s)
{
  if (!s) return true;

  s->getFuben().setMonsterCountShow(true);
  notify(s);
  return true;
}

bool MonsterCountPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}

void MonsterCountPhase::notify(DScene *scene, bool isClose/*=false*/)
{
  if (!scene || scene->getFuben().getMonsterCountShow() == false)
    return;

  xSceneEntrySet entrySet;
  scene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (!entrySet.empty())
  {
    DWORD count = 0;
    if (!isClose)
    {
      count = scene->getAllAliveMonsterNum();
    }
    Cmd::MonsterCountUserCmd message;
    message.set_num(count);
    PROTOBUF(message, send, len);
    MsgManager::sendMapCmd(scene->id, send, len);
    if (count == 0 && !isClose)
    {
      scene->getFuben().setMonsterCountShow(false);
    }
  }
}

// beattack phase
bool BeAttackPhase::exec(DScene* s)
{
  if (s == nullptr)
    return false;

  if (FUBEN_RESULT_SUCCESS==s->getFuben().m_result) return false;

  xSceneEntrySet set;
  s->getAllEntryList(SCENE_ENTRY_USER, set);
  if (set.empty() == true)
    return false;
  SceneUser* pUser = dynamic_cast<SceneUser*>(*set.begin());
  if (pUser == nullptr)
    return false;

  return pUser->getFubenBeAttackCount() >= m_dwBeAttackCount;
}

bool BeAttackPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  xLuaData& params = m_data.getMutableData("Params");
  m_dwBeAttackCount = params.getTableInt("beattack");
  return true;
}

// Dojo reward phase
bool DojoRewardPhase::exec(DScene *s)
{
  if (!s)
    return false;

  DojoScene* pScene = dynamic_cast<DojoScene*>(s);
  if (pScene == nullptr) return false;

  if (pScene->getRaidType() != ERAIDTYPE_DOJO)
  {
    XERR << "[道场]"<< "raid:" << s->getRaidID() << "eRaidType:" << s->getRaidType() << "不是道场类型" << XEND; 
    return false;
  }

  xSceneEntrySet userSet;
  pScene->getAllEntryList(SCENE_ENTRY_USER, userSet);
  if (userSet.empty())
    return true;

  std::set<SceneUser*> vecFirstUser;
  std::set<SceneUser*> vecHelpUser;
  std::set<SceneUser*> allUser;
  DWORD dwDojoId = pScene->getDojoId();

  const SDojoItemCfg* pCFG = DojoConfig::getMe().getDojoItemCfg(dwDojoId);
  if (!pCFG)
  {
    XERR << "[道场] dojoid:" << dwDojoId << "找不到配置表" << XEND;
    return true;
  }
  for (auto userIt = userSet.begin(); userIt != userSet.end(); ++userIt)
  {
    SceneUser *pUser = dynamic_cast<SceneUser*>(*userIt);
    if (nullptr == pUser)
      continue;
    if (pUser->getDojo().isOpen(pCFG->dwGroupId) && !pUser->getDojo().isPassed(dwDojoId))
    { //功能开启，并且之前没通过
      vecFirstUser.insert(pUser);
    }
    else
    {
      vecHelpUser.insert(pUser);
    }
    allUser.insert(pUser);
    pUser->getServant().onFinishEvent(ETRIGGER_DOJO);
    pUser->getServant().onGrowthFinishEvent(ETRIGGER_DOJO);
  }
  auto logFun = [&](SceneUser* pUser, EDOJOPASS_TYPE passType, TVecItemInfo& rVecItemInfo) {
    if (pUser == nullptr)
      return;

    DojoRewardCmd cmd;
    cmd.set_dojoid(dwDojoId);
    cmd.set_passtype(static_cast<Cmd::EPassType>(passType));
    for (auto &v : rVecItemInfo)
    {
      ItemInfo* pItemInfo = cmd.add_items();
      if (pItemInfo)
      {
        pItemInfo->set_id(v.id());
        pItemInfo->set_count(v.count());
      }
    }
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);

    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Dojo;
    PlatLogManager::getMe().eventLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eid,
      pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().DojoLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eType,
      eid,
      dwDojoId,
      s->getRaidID(),
      passType,
      pUser->getLevel()
    );

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_DOJO_COUNT, 0, dwDojoId, pUser->getLevel(), (DWORD)1);
    pUser->stopSendInactiveLog();
  };

  TVecItemInfo vecItemInfo;
  TVecItemInfo vecReward;

  if (vecFirstUser.empty())
  { // 没有人首次通关
    XINF << "[道场] dojoid:" << dwDojoId << "没有人首次通关" << XEND;

    for (auto it = vecHelpUser.begin(); it != vecHelpUser.end(); ++it)
    {
      SceneUser *pUser = *it;
      if (!pUser)
        continue;
      logFun(pUser, EDOJOPASSTYPE_NORMAL, vecItemInfo);
    }
    return true;
  } 

  //首次通关奖励
  for (auto it = vecFirstUser.begin(); it != vecFirstUser.end(); ++it)
  {
    SceneUser *pUser = *it;
    if (!pUser)
      continue;
    pUser->getDojo().passDojo(dwDojoId);
    vecItemInfo.clear();
    if (RewardManager::roll(pCFG->dwFirstReward, pUser, vecReward, ESOURCE_DOJOFIRST) == true)
      combinItemInfo(vecItemInfo, vecReward);

    // 活动模板额外/翻倍奖励
    getActivityEventExtraReward(pUser, dwDojoId, vecItemInfo);

    //
    for (auto &v : vecItemInfo)
    {
      if (v.id() == ITEM_CONTRIBUTE) //公会贡献
      {
        stringstream sstr;
        sstr << "guild cmd=addasset num=" << v.count() << " nocheck=0" << " source=" << static_cast<DWORD>(ESOURCE_DOJOFIRST);
        GMCommandRuler::getMe().execute(pUser, sstr.str());
        /*xLuaData data;
        data.setData("num", v.count());
        GMCommandRuler::addasset(pUser, data);*/
        XLOG << "[道场-公会-资金] 首次通关奖励,公会增加，accid" << pUser->accid << "charid" << pUser->id << "num" << v.count() << XEND;
        break;
      }
    }
    TVecItemInfo vecItem = vecItemInfo;
    //pUser->getPackage().addItemAvailable(vecItem, false, false);    
    pUser->getPackage().addItem(vecItem, EPACKMETHOD_AVAILABLE, false, false);
    logFun(pUser, EDOJOPASSTYPE_FIRST, vecItemInfo);
    std::stringstream ss;
    for (auto &it : vecItemInfo)
    {
      ss << it.id() << "," << it.count() << ";";
    }
    XLOG << "[道场-奖励] 首次通关" << pUser->accid<<pUser->id << "dojoid:" << dwDojoId << "item:" << ss.str() <<"guild level"<< pUser->getGuild().lv() << XEND;
  }

  //协助通关奖励
  for (auto it = vecHelpUser.begin(); it != vecHelpUser.end(); ++it)
  {
    SceneUser *pUser = *it;
    if (!pUser)
      continue;
    vecItemInfo.clear();
    if (RewardManager::roll(pCFG->dwHelpReward, pUser, vecReward, ESOURCE_DOJOHELP) == true)
      combinItemInfo(vecItemInfo, vecReward);

    // 活动模板额外/翻倍奖励
    getActivityEventExtraReward(pUser, dwDojoId, vecItemInfo);

    for (auto &v : vecItemInfo)
    {
      if (v.id() == ITEM_CONTRIBUTE)
      {
        stringstream sstr;
        sstr << "guild cmd=addasset num=" << v.count() << " nocheck=0" << " source=" << static_cast<DWORD>(ESOURCE_DOJOHELP);
        GMCommandRuler::getMe().execute(pUser, sstr.str());
        /*xLuaData data;
        data.setData("num", v.count());
        GMCommandRuler::addasset(pUser, data);*/
        XLOG << "[道场-公会-资金] 协助通关奖励,公会增加，accid" << pUser->accid << "charid" << pUser->id << "num" << v.count() << XEND;
        break;
      }
    }
    TVecItemInfo vecItem = vecItemInfo;
    //pUser->getPackage().addItemAvailable(vecItem, false, false);
    pUser->getPackage().addItem(vecItem, EPACKMETHOD_AVAILABLE, false, false);
    logFun(pUser, EDOJOPASSTYPE_HELP, vecItemInfo);
    std::stringstream ss;
    for (auto &it : vecItemInfo)
    {
      ss << it.id() << "," << it.count() << ";";
    }
    pUser->getHelpReward(vecFirstUser, EHELPTYPE_DOJO);
    pUser->getAchieve().onPassDojo(true);
    XLOG << "[道场-奖励] 协助通关" << pUser->accid <<pUser->id<< "dojoid:" << dwDojoId << "item:" << ss.str() << "guild level" << pUser->getGuild().lv() << XEND;
  }
  for (auto &user : vecFirstUser)
  {
    user->getHelpReward(allUser, EHELPTYPE_DOJO, true);
  }
  return true;
}

// 活动模板额外/翻倍奖励
void DojoRewardPhase::getActivityEventExtraReward(SceneUser* user, DWORD dojoid, TVecItemInfo& rewards)
{
  if (user == nullptr)
    return;

  DWORD times = 0;
  TVecItemInfo extrareward;
  if (ActivityEventManager::getMe().getReward(user, EAEREWARDMODE_GUILDDOJO, 1, extrareward, times) == false)
    return;
  if (times > 1)
  {
    for (auto& v : rewards)
      v.set_count(v.count() * times);
    XLOG << "[道场-活动模板] 多倍奖励" << user->accid << user->id << user->name << "dojoid:" << dojoid << "奖励倍数:" << times << XEND;
  }
  if (!extrareward.empty())
  {
    XLOG << "[道场-活动模板] 额外奖励" << user->accid << user->id << user->name << "dojoid:" << dojoid << "额外奖励:";
    for (auto& v : extrareward)
      XLOG << v.id() << v.count();
    XLOG << XEND;
    combinItemInfo(rewards, extrareward);
  }
}

bool DojoRewardPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  return true;
}


// Dojo summon phase
bool DojoSummonPhase::exec(DScene *s)
{
  if (!s)
    return true;
  DojoScene* pScene = dynamic_cast<DojoScene*>(s);
  if (pScene == nullptr)
  {
    return false;
  }
  if (pScene->getRaidType() != ERAIDTYPE_DOJO)
  {
    XERR << "[道场-summon] dojoid:" << pScene->getDojoId() << "raid:" << pScene->getRaidID() << "eRaidType:" << pScene->getRaidType() << "不是道场类型" << XEND;    return true;
    return false;
  }

  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (0 == entrySet.size())
    return false;

  const SDojoItemCfg* pCFG = DojoConfig::getMe().getDojoItemCfg(pScene->getDojoId());
  if (pCFG == nullptr)
    return false;
  
  const SceneObject* pObject = nullptr;
  const SceneBase* pSBase = SceneManager::getMe().getDataByID(s->getMapID());
  if (pSBase == nullptr)
    return false;
  pObject = pSBase->getSceneObject(pCFG->dwRapid);
  if (pObject == nullptr)
    return false;

  const SceneNpcTemplate* pRaidNpc = pObject->getRaidNpcTemplate(m_dwGroupID);
  if (pRaidNpc != nullptr)
  {
    for (DWORD i = 0; i < pRaidNpc->m_dwNum; i++) 
    {
      NpcDefine tmpDefine = pRaidNpc->m_oDefine;
      tmpDefine.m_oVar.dwDojoLevel = pCFG->dwLevel;
      SceneNpcManager::getMe().createNpc(tmpDefine, s);
    }
  }
  return true;
}

bool DojoSummonPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwGroupID = m_data.getMutableData("Params").getTableInt("group_id");
  if (m_dwGroupID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "group_id:" << m_dwGroupID << "不合法" << XEND;
    return false;
  }
  return true;
}

// rand summon phase
bool UniqRandSummonFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  const SceneObject *pObject = scene->getSceneObject();
  if (pObject == nullptr)
  {
    XERR << "[副本步骤-UniqRandSummon], 场景object 为空" << m_dwID << m_dwGroupID << XEND;
    return false;
  }
  DWORD npcid = NpcConfig::getMe().getRandMonsterByGroup(scene->getMapID(), m_dwGroupID);

  if (npcid == 0)
  {
    XERR << "[副本步骤-UniqRandSummon], 无法随机到怪物" << m_dwID << m_dwGroupID << XEND;
    return true;
  }
  const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(m_dwGroupID);
  if (pTemplate != nullptr)
  {
    NpcDefine tmpDefine = pTemplate->m_oDefine;
    tmpDefine.setID(npcid);
    tmpDefine.setUniqueID(m_dwGroupID);
    SceneNpcManager::getMe().createNpc(scene, tmpDefine, pTemplate->m_dwNum);
  }
  return true;
}

bool UniqRandSummonFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwGroupID = m_data.getMutableData("Params").getTableInt("group_id");

  if (m_dwGroupID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "group_id :" << m_dwGroupID << "不合法" << XEND;
    return false;
  }

  return true;
}

// rand summon phase
bool RandSummonFuBenPhase::exec(DScene *scene)
{
  if (!scene) return true;

  const SceneObject *pObject = scene->getSceneObject();
  if (pObject == nullptr)
  {
    XERR << "[副本步骤-RandSummon], 场景object 为空" << m_dwID << m_dwGroupID << XEND;
    return false;
  }
  std::map<DWORD, DWORD> outUid2NpcID;
  MapConfig::getMe().getRaidMonster(m_dwGroupID, outUid2NpcID);
  if (outUid2NpcID.empty())
  {
    XERR << "[副本步骤-RandSummon], 无法随机到怪物" << m_dwID << m_dwGroupID << XEND;
    return true;
  }
  for (auto &m : outUid2NpcID)
  {
    const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(m.first);
    if (pTemplate != nullptr)
    {
      NpcDefine tmpDefine = pTemplate->m_oDefine;
      tmpDefine.setID(m.second);
      tmpDefine.setUniqueID(m.first);
      SceneNpcManager::getMe().createNpc(scene, tmpDefine, pTemplate->m_dwNum);
    }
  }
  return true;
}

bool RandSummonFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwGroupID = m_data.getMutableData("Params").getTableInt("group_id");

  if (m_dwGroupID == 0)
  {
    XERR << "[副本配置-步骤" << m_type << "] id :" << m_dwID << "group_id :" << m_dwGroupID << "不合法" << XEND;
    return false;
  }

  return true;
}
bool CollectFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;

  auto groupf = [this](const string& str, xLuaData& data)
  {
    m_setGroupIDs.insert(data.getInt());
  };
  m_data.getMutableData("Params").getMutableData("group_id").foreach(groupf);

  return true;
}

bool CollectFuBenPhase::exec(DScene* scene)
{
  if (!scene) return true;

  VecSceneNpc groupVec;
  for (auto&s : m_setGroupIDs)
    scene->getNpcVecByUniqueID(s, groupVec);

  if (groupVec.empty())
    return false;

  for (auto &v : groupVec)
  {
    if (!v)
      continue;
    if (v->isAlive())
      return false;
  }

  xSceneEntrySet userset;
  scene->getAllEntryList(SCENE_ENTRY_USER, userset);
  for (auto &s : userset)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (!user)
      continue;
    syncDelThisStep(user);
  }
  return true;
}

// client plot phase
bool ClientPlotFuBenPhase::exec(DScene* s)
{
  if (checkClient(s) == true)
    return true;
  return false;
}

// use phase
bool UseFuBenPhase::exec(DScene *s)
{
  return checkClient(s);

}

bool UseFuBenPhase::init()
{
  return FuBenPhase::init();
}

// 古城裂隙奖励
bool SealRewardFuBenPhase::exec(DScene* s)
{
  if (s == nullptr)
    return false;
  xSceneEntrySet entrySet;
  s->getAllEntryList(SCENE_ENTRY_USER, entrySet);

  const SealMiscCFG& rSealCFG = MiscConfig::getMe().getSealCFG();
  //const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
  std::set<SceneUser*> validUserSet;
  std::set<SceneUser*> invalidUserSet;
  std::set<SceneUser*> allUser;

  for (auto &p : entrySet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (p);
    if (!user)
      continue;

    allUser.insert(user);
    DWORD sealednum = user->getVar().getVarValue(EVARTYPE_SEAL);
    if (sealednum >= rSealCFG.dwMaxDaySealNum)
    {
      invalidUserSet.insert(user);
    }
    else
    {
      validUserSet.insert(user);
    }
    //if (sealednum < rCFG.dwSealTimes)
      //pUser->getUserSceneData().addCredit(rCFG.dwSealValue);
    user->getVar().setVarValue(EVARTYPE_SEAL, sealednum + 1);
    //if (sealednum < rSealCFG.dwMaxDaySealNum)
        //pUser->getExtraReward(EEXTRAREWARD_SEAL);
    if (m_bTaskExtraReward && sealednum < rSealCFG.dwMaxDaySealNum)
      user->getTaskExtraReward(ETASKEXTRAREWARDTYPE_SEAL, sealednum + 1);
    user->getServant().onFinishEvent(ETRIGGER_REPAIR_SEAL);
  }
  for (auto &user : validUserSet)
  {
    DWORD sealnum = user->getVar().getVarValue(EVARTYPE_SEAL);
    MsgParams msg;
    msg.addNumber(sealnum);
    msg.addNumber(rSealCFG.dwMaxDaySealNum);
    MsgManager::sendMsg(user->id, 1622, msg);

    for (auto &d : m_setRewardIDs)
    {
      user->getPackage().rollReward(d, EPACKMETHOD_AVAILABLE, false, true, false);
      XLOG << "[古城裂隙-奖励], 玩家:" << user->name << user->id << "奖励ID:" << d << XEND;
    }
    user->getHelpReward(allUser, EHELPTYPE_SEAL, true);

    // 额外奖励, 裂隙无翻倍奖
    DWORD times = 1;
    TVecItemInfo extrareward;
    if (ActivityEventManager::getMe().getReward(user, EAEREWARDMODE_SEAL, sealnum, extrareward, times) && extrareward.empty() == false)
    {
      XLOG << "[古城裂隙-奖励]" << user->accid << user->id << user->getProfession() << user->name << "活动额外奖励:";
      for (auto& v : extrareward)
        XLOG << v.id() << v.count();
      XLOG << XEND;
      user->getPackage().addItem(extrareward, EPACKMETHOD_AVAILABLE, false, true, false);
    }
  }
  for (auto &user : invalidUserSet)
  {
    MsgManager::sendMsg(user->id, 1623);
    XLOG << "[古城裂隙-奖励], 玩家:" << user->name << user->id << "今天次数已达上限, 不可领取奖励" << XEND;
    user->getHelpReward(validUserSet, EHELPTYPE_SEAL);
  }
  XLOG << "[古城裂隙-奖励], 完成, 副本ID:" << s->name << s->id << "raid id:" << m_dwID << XEND;

  return true;
}

bool SealRewardFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;

  xLuaData& rds = m_data.getMutableData("Params").getMutableData("id");
  auto getid = [&](const string& key, xLuaData& d)
  {
    m_setRewardIDs.insert(d.getInt());
  };
  rds.foreach(getid);

  m_bTaskExtraReward = m_data.getMutableData("Params").getTableInt("task_extra_reward") == 1;
  return true;
}

bool GRaidSummonFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;
  auto getGRP = [&](const string& key, xLuaData& d)
  {
    m_setGroupIDs.insert(d.getInt());
  };
  m_data.getMutableData("Params").getMutableData("group_id").foreach(getGRP);
  if (m_setGroupIDs.empty())
  {
    XERR << "[副本配置], id:" << m_dwID << "group_id为空" << XEND;
    return false;
  }
  auto getuid = [&](const string& key, xLuaData& d)
  {
    m_setUIDs.insert(d.getInt());
  };
  m_data.getMutableData("Params").getMutableData("uid").foreach(getuid);
  if (m_setUIDs.empty())
  {
    XERR << "[副本配置], id:" << m_dwID << "uid为空" << XEND;
    return false;
  }

  m_bKeepGuildSame = m_data.getMutableData("Params").getTableInt("guild_same") == 1;
  return true;
}

bool GRaidSummonFuBenPhase::exec(DScene* s)
{
  if (s == nullptr)
    return false;
  GuildRaidScene* pScene = dynamic_cast<GuildRaidScene*> (s);
  if (pScene == nullptr)
    return false;

  //DWORD group = pScene->getMapIndex() / ONE_THOUSAND;
  DWORD raidid = pScene->getRaidID();
  const SceneBase* scenebase = SceneManager::getMe().getDataByID(raidid);
  if (scenebase == nullptr)
    return false;

  const SceneObject* pObject = scenebase->getSceneObject(raidid);
  if (pObject == nullptr)
    return false;

  const SGuildRaidCFG& rGRCFG = MiscConfig::getMe().getGuildRaidCFG();
  DWORD raidgroup = rGRCFG.getLvByGroup(pScene->getMapIndex());

  TSetDWORD setNpcs;
  DWORD srandGuildID = 0;
  if (m_bKeepGuildSame)
  {
    DWORD uidindex = 0;
    if (!m_setUIDs.empty())
      uidindex = *(m_setUIDs.begin());

    srandGuildID = pScene->getGuildID() + pScene->getMapIndex() * 100 + uidindex * 10;
  }
  if (GuildRaidConfig::getMe().getRandomMonster(raidgroup, m_setGroupIDs, setNpcs, srandGuildID) == false)
    return false;
  TVecDWORD vecNpcs;
  for (auto &s : setNpcs)
    vecNpcs.push_back(s);

  auto getOneID = [&](DWORD index) -> DWORD
  {
    if (vecNpcs.size() == 0)
      return 0;
    index %= vecNpcs.size();
    return vecNpcs[index];
  };

  DWORD tmpindex = 0;
  for (auto &d : m_setUIDs)
  {
    const SceneNpcTemplate* pRaidNpc = pObject->getRaidNpcTemplate(d);
    if (pRaidNpc == nullptr)
      continue;
    NpcDefine tmpDefine = pRaidNpc->m_oDefine;
    tmpDefine.setID(getOneID(tmpindex));
    ++ tmpindex;

    if (GuildRaidConfig::getMe().isBossMonster(tmpDefine.getID()))
      pScene->setSummonBossID(tmpDefine.getID());

    SceneNpcManager::getMe().createNpc(tmpDefine, s);
  }

  return true;
}

bool GRaidRewardFuBenPhase::init()
{
  if (FuBenPhase::init() == false)
    return false;

  m_dwRewardMsg = m_data.getMutableData("Params").getTableInt("reward_msg");
  return true;
}

bool GRaidRewardFuBenPhase::exec(DScene* s)
{
  GuildRaidScene* pScene = dynamic_cast<GuildRaidScene*> (s);
  if (pScene == nullptr)
    return false;

  DWORD bossid = pScene->getSummonBossID();
  if (bossid == 0)
  {
    XERR << "[公会副本], 奖励, 找不到boss, 副本:" << pScene->getRaidID() << "index:" << pScene->getMapIndex() << XEND;
    return false;
  }

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(bossid);
  if(pCFG == nullptr)
  {
    XERR << "[公会副本], 奖励, 找不到boss配置, 副本:" << pScene->getRaidID() << "index:" << pScene->getMapIndex() << XEND;
    return false;
  }

  xSceneEntrySet entrySet;
  pScene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (entrySet.empty())
    return false;

  DWORD mapindex= pScene->getMapIndex();
  const SGuildRaidCFG& rGRCFG = MiscConfig::getMe().getGuildRaidCFG();
  DWORD npcgateid= rGRCFG.getNpcIDByGroup(mapindex);

  std::set<SceneUser*> validUserSet;
  std::set<SceneUser*> invalidUserSet;
  for (auto &s : entrySet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user && user->getUserSceneData().hasGotGRaidReward(npcgateid, mapindex) == false)
      validUserSet.insert(user);
    else if (user && user->getUserSceneData().hasGotGRaidReward(npcgateid, mapindex) == true)
      invalidUserSet.insert(user);
  }
  if (validUserSet.empty())
    return true;

  const TSetDWORD& setrwds = GuildRaidConfig::getMe().getBossReward(bossid);
  if (setrwds.empty())
    return false;

  for (auto &user : validUserSet)
  {
    TVecItemInfo vecItemInfo;
    TVecItemInfo vecReward;
    for (auto &d : setrwds)
    {
      if (RewardManager::roll(d, user, vecReward, ESOURCE_GUILDRAID, rGRCFG.fRewardRatio))
      {
        DWORD activityid = 0;
        DWORD times = 1;
        /*     const SGlobalActivityCFG* pCFG = MiscConfig::getMe().getGlobalActivityCFG(GACTIVITY_GUILD_FUBEN);
             if(pCFG != nullptr)
             {
               activityid = pCFG->activityid;
               times = pCFG->times;
             }*/
        const SGlobalActCFG*pCFG = ActivityConfig::getMe().getGlobalActCFG(GACTIVITY_GUILD_FUBEN);
        if (pCFG)
        {
          activityid = pCFG->m_dwId;
          times = pCFG->getParam(0);
        }
        bool isOpen = ActivityManager::getMe().isOpen(activityid);
        if(isOpen)
        {
          for (auto &item : vecReward)
          {
            DWORD count = item.count() * times;
            item.set_count(count);
          }
          XLOG << "[公会副本], 双倍奖励, 工会副本:" << user->name << user->id << "index:" << mapindex << "副本:" << pScene->getRaidID() << "Boss:" << bossid << XEND;
        }
        combinItemInfo(vecItemInfo, vecReward);
      }
    }
    // extra reward
    TSetDWORD setExtraRwds;
    if (RewardConfig::getMe().getNpcExtraRwd(pCFG, setExtraRwds, false))
    {
      for (auto &s : setExtraRwds)
      {
        vecReward.clear();
        if (RewardManager::roll(s, user, vecReward, ESOURCE_GUILDRAID, 1))
        {
          combinItemInfo(vecItemInfo, vecReward);
          XLOG << "[公会副本-额外掉落], 玩家:" << user->name << user->id << "副本:" << mapindex << pScene->getRaidID() << "Boss:" << bossid << "奖励:" << s << XEND;
        }
      }
    }

    // 活动模板额外/翻倍奖励
    {
      DWORD times = 0;
      TVecItemInfo extrareward;
      if (ActivityEventManager::getMe().getReward(user, EAEREWARDMODE_GUILDRAID, 1, extrareward, times))
      {
        if (times > 1)
        {
          for (auto& v : vecItemInfo)
            v.set_count(v.count() * times);
          XLOG << "[公会副本-活动模板] 多倍奖励, 工会副本:" << user->name << user->id << "index:" << mapindex << "副本:" << pScene->getRaidID() << "Boss:" << bossid << "奖励倍数:" << times << XEND;
        }
        if (!extrareward.empty())
        {
          XLOG << "[公会副本-活动模板] 额外奖励, 工会副本:" << user->name << user->id << "index:" << mapindex << "副本:" << pScene->getRaidID() << "Boss:" << bossid << "额外奖励:";
          for (auto& v : extrareward)
            XLOG << v.id() << v.count();
          XLOG << XEND;
          combinItemInfo(vecItemInfo, extrareward);
        }
      }
    }

    user->getUserSceneData().markGotGRaidReward(npcgateid, mapindex);
    if (user->getUserSceneData().getGRaidKilledBossCnt(npcgateid) >= rGRCFG.dwBossMapNum)
    {
      user->getTutorTask().onGuildRaidFinish();
      user->getGuildChallenge().onGuildRaidFinish();
    }
    for(auto &c : pCFG->vecRelevancyIDs)
    {
      user->getManual().onKillMonster(c);
      user->getManual().onKillProcess(c);
    }
    if (m_dwRewardMsg)
      MsgManager::sendMsg(user->id, m_dwRewardMsg);
    pScene->summonRewardBox(user);
    user->getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);
    user->stopSendInactiveLog();
    user->getServant().onFinishEvent(ETRIGGER_GUILD_FUBEN);
    XLOG << "[公会副本], 奖励获得, 玩家:" << user->name << user->id << "index:" << mapindex << "副本:" << pScene->getRaidID() << "Boss:" << bossid << XEND;
  }

  for (auto &user : invalidUserSet)
  {
    user->getHelpReward(validUserSet, EHELPTYPE_GUILDRAID);
  }

  return true;
}

bool GuildFireCheckMonster::exec(DScene* s)
{
  GuildFireScene* pGScene = dynamic_cast<GuildFireScene*> (s);
  if (pGScene == nullptr)
    return false;
  if (pGScene->checkMonsterCity())
    return true;
  return false;
}

bool GuildFireSummonMetal::exec(DScene* s)
{
  GuildFireScene* pGScene = dynamic_cast<GuildFireScene*> (s);
  if (pGScene == nullptr)
    return true;
  bool fire = pGScene->getFireState() != EGUILDFIRE_PEACE;
  DWORD groupid = fire ? m_dwFireGroupID : m_dwPeaceGroupID;
  const SceneObject *pObject = pGScene->getSceneObject();
  if (pObject)
  {
    const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(groupid);
    if (pTemplate != nullptr)
    {
      NpcDefine tmpDefine = pTemplate->m_oDefine;
      if (fire)
        tmpDefine.m_oVar.m_qwGuildID = pGScene->getDefenseGuildID();
      SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(tmpDefine, pGScene);
      if (pNpc == nullptr)
      {
        XERR << "[公会据点-华丽金属], 召唤失败" << pGScene->name << pGScene->id << XEND;
        return true;
      }
      pGScene->onSummonMetalNpc(pNpc->id);
      XLOG << "[公会据点-华丽金属], 召唤成功, 地图" << pGScene->name << pGScene->id << "据点:" << pGScene->getCityID() << "华丽金属:" << pNpc->id << XEND;
    }
  }
  return true;
}

bool GuildFireSummonMetal::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwFireGroupID = m_data.getMutableData("Params").getTableInt("group_id");
  m_dwPeaceGroupID = m_data.getMutableData("Params").getTableInt("peace_group_id");
  return true;
}

bool AltmanRewardPhase::init()
{
  return FuBenPhase::init();
}

// altman reward phase
bool AltmanRewardPhase::exec(DScene *s)
{
  AltmanScene* pScene = dynamic_cast<AltmanScene*>(s);
  if (pScene == nullptr)
    return false;

  pScene->reward();
  return true;
}

bool CheckSummonDeadBossNpc::init()
{
  if (FuBenPhase::init() == false)
    return false;
  m_dwNpcUniqueID = m_data.getMutableData("Params").getTableInt("group_id");
  return true;
}

bool CheckSummonDeadBossNpc::exec(DScene* s)
{
  if (s == nullptr)
    return false;

  if (s->checkSummonDeadBoss() == false)
    return true;

  const SceneObject *pObject = s->getSceneObject();
  if (pObject)
  {
    const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(m_dwNpcUniqueID);
    if (pTemplate != nullptr)
    {
      SceneNpcManager::getMe().createNpc(pTemplate->m_oDefine, s);;
    }
  }

  return true;
}
