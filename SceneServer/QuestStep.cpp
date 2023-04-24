#include "QuestStep.h"
#include "Quest.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "GMCommandRuler.h"
#include "SceneNpcManager.h"
#include "SceneManager.h"
#include "SkillManager.h"
#include "MapConfig.h"
#include "MsgManager.h"
#include "SceneServer.h"
#include "DScene.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "SceneUserManager.h"
#include "ActivityManager.h"
#include "FoodConfig.h"
#include "RecipeConfig.h"
#include "MailManager.h"

// quest step
BaseStep::BaseStep(DWORD questid) : m_dwQuestID(questid)
{

}

BaseStep::~BaseStep()
{

}

bool BaseStep::init(xLuaData& data)
{
  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(m_dwQuestID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "未在Table_Quest.txt表中找到" << XEND;
    return false;
  }

  m_dwMapID = data.getTableInt("Map");
  m_dwSubGroup = data.getTableInt("SubGroup");
  m_dwFinishJump = data.getTableInt("FinishJump");
  m_dwFailJump = data.getTableInt("FailJump");
  m_dwDetailID = data.getTableInt("Detail");
  m_dwQuestResetJump = data.getTableInt("QuestSave");

  m_strContent = data.getTableString("Content");
  m_strName = data.getTableString("Name");
  m_strTraceInfo = data.getTableString("TraceInfo");

  xLuaData& param = data.getMutableData("Params");
  m_bPos = param.has("pos");
  if (m_bPos)
  {
    xLuaData& pos = param.getMutableData("pos");
    m_oDestPos.x = pos.getTableFloat("1");
    m_oDestPos.y = pos.getTableFloat("2");
    m_oDestPos.z = pos.getTableFloat("3");
  }

  if (m_dwMapID != 0 && MapConfig::getMe().getMapCFG(m_dwMapID) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "mapid :" << m_dwMapID << "未在Table_Map.txt表中找到" << XEND;
    return false;
  }

  m_bReset = param.getTableInt("reset") == 0;
  m_bAbandon = param.getTableInt("mark_team_wanted") != 1;

  m_bSyncTeamWanted = param.getTableInt("mark_team_wanted") == 1;
  m_bTeamCanFinish = param.getTableInt("team_can_finish") == 1;

  DWORD dwAction = param.getTableInt("action");
  if (dwAction == 1)
    m_bDayNightLimit = true;
  else if (dwAction == 2)
    m_bDayNightLimit = true;
  else if (dwAction == 3)
    m_bDayNightLimit = m_bDayNightReset = true;

  /*if (m_bSyncTeamWanted)
  {
    const SWantedItem* pWantedItem = QuestConfig::getMe().getWantedItemCFG(m_dwQuestID);
    if (pWantedItem == nullptr || pWantedItem->bTeamSync == false)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "mapid :" << m_dwMapID << "mark_team_wanted=1, 但Wanted_Quest 未配置TeamSync" << XEND;
      return false;
    }
  }
  */
  DWORD dwIndex = 0;
  DWORD dwSelect = 0;
  char szTemp[64] = {0};

  do
  {
    snprintf(szTemp, 64, "client%u", dwIndex++);
    if (data.has(szTemp) == false)
      break;

    dwSelect = data.getTableInt(szTemp);
    m_vecClientSelects.push_back(dwSelect);
  }while (true);

  return true;
}

void BaseStep::reset(SceneUser* pUser)
{
  if (!m_bReset || pUser == nullptr)
    return;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr || pStep->process() == 0)
    return;
  pStep->set_process(0);
  stepUpdate(pUser);
}

bool BaseStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (subGroup != 0)
  {
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(m_dwQuestID);
    if (pCFG != nullptr)
    {
      DWORD step = pCFG->getStepBySubGroup(subGroup);
      pUser->getQuest().setQuestStep(m_dwQuestID, step);

      XLOG << "[任务步骤-跳跃]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "questid :" << m_dwQuestID << "step :" << m_strContent << "jump subGroup :" << subGroup << "step :" << step << XEND;
      return true;
    }
  }

  XLOG << "[任务步骤-推进]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "do questid :" << m_dwQuestID << "step :" << m_strContent << XEND;
  return pUser->getQuest().addQuestStep(m_dwQuestID);
}

bool BaseStep::stepUpdate(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  return pUser->getQuest().stepUpdate(m_dwQuestID);
}

// quest step - visit
VisitStep::VisitStep(DWORD questid) : BaseStep(questid)
{

}

VisitStep::~VisitStep()
{

}

bool VisitStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  DWORD dwNpcID = param.getTableInt("npc");
  if (dwNpcID != 0)
    m_setNpcIDs.insert(dwNpcID);
  auto npcf = [&](const string& key, xLuaData& data)
  {
    if (data.getInt() != 0)
      m_setNpcIDs.insert(data.getInt());
  };
  param.getMutableData("npc").foreach(npcf);

  for (auto &s : m_setNpcIDs)
  {
    if (NpcConfig::getMe().getNpcCFG(s) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "npc :" << s << "未在Table_Npc.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

// param2为999表示跳过检查直接完成
bool VisitStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (m_bDayNightLimit)
  {
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(m_dwQuestID);
    if (pCFG != nullptr)
    {
      bool bNight = MiscConfig::getMe().getSystemCFG().isNight(xTime::getCurSec());
      if (pCFG->eType == EQUESTTYPE_DAY && bNight)
        return false;
      else if (pCFG->eType == EQUESTTYPE_NIGHT && !bNight)
        return false;
    }
  }

  if (param2 != 999)
  {
    if (MiscConfig::getMe().getSystemCFG().bValidPosCheck)
    {
      SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(pUser->getVisitNpc());
      if (pNpc == nullptr || pNpc->getCFG() == nullptr || pNpc->getScene() == nullptr)
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
          pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未通过npc执行" << XEND;
        return false;
      }
      auto s = m_setNpcIDs.find(pNpc->getNpcID());
      if (s == m_setNpcIDs.end())
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
          pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未在正确的npc执行" << XEND;
        return false;
      }
      float fDist = ::getXZDistance(pNpc->getPos(), pUser->getPos());
      if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius || pNpc->getScene() != pUser->getScene())
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
          pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,距离npc过远" << XEND;
        return false;
      }
    }
    if (m_dwMapID != 0 && pUser->getScene()->getMapID() != m_dwMapID)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
        pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未在正确的地图" << XEND;
      return false;
    }
  }

  if (m_vecClientSelects.empty() == true)
  {
    if (subGroup != 0)
      return false;

    subGroup = m_dwFinishJump;
  }
  else
  {
    auto v = find(m_vecClientSelects.begin(), m_vecClientSelects.end(), subGroup);
    if (v == m_vecClientSelects.end())
      return false;
    if (subGroup == 0)
      subGroup = m_dwFinishJump;
  }

  bool ret = BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
  if (ret)
  {
    //platlog 访问npc
    std::string rewardItem;
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Quest_VisitNpc;
    PlatLogManager::getMe().eventLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eid,
      pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().QuestLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eType,
      eid,
      m_dwQuestID,
      EQuestType_VISITNPC,
      0,
      0,
      0,
      rewardItem,
      pUser->getLevel());

    // 看板队长
    set<SceneUser*> uSet;
    pUser->getQuest().getWantedQuestLeaderTeammate(m_dwQuestID, uSet);
    if (!uSet.empty())
    {
      string npcname;
      if (!m_setNpcIDs.empty())
      {
        const SNpcCFG* pNpcCfg = NpcConfig::getMe().getNpcCFG(*m_setNpcIDs.begin());
        if (pNpcCfg != nullptr)
          npcname = pNpcCfg->strName;
      }
      for (auto &u : uSet)
      {
        if (u->getQuest().runStep(m_dwQuestID, subGroup, param1, 999, sparam1))
          MsgManager::sendMsg(u->id, 4012, MsgParams(npcname));
      }
    }
  }
  return ret;
}

// quest step - kill
KillStep::KillStep(DWORD questid) : BaseStep(questid), m_dwMonsterID(0), m_dwNum(0)
{

}

KillStep::~KillStep()
{

}

bool KillStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwMonsterID = param.getTableInt("monster");
  m_dwGroupID = param.getTableInt("groupId");
  m_dwNum = param.getTableInt("num");

  if (m_dwMonsterID != 0)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwMonsterID);
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "monster :" << m_dwMonsterID << "未在Table_Monster.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool KillStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(param1);
  if (pNpc == nullptr)
    return false;
  const SNpcCFG* pCFG = pNpc->getCFG();
  if (pCFG == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  
  if (m_dwQuestID == 9956)
  {
    XLOG << "[BUG跟踪-任务] charid" << pUser->id << "questId" << m_dwQuestID << "groupid" << m_dwGroupID << "npc" << pCFG->dwID << "npcgroupid" << pCFG->dwGroupID << "killednum" << m_dwNum << XEND;
  }

  if (m_dwGroupID != 0)
  {
    if (m_dwGroupID != pCFG->dwGroupID)
      return false;
  }
  else
  {
    if (pCFG->dwID != m_dwMonsterID)
      return false;
  }

  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;
  pStep->set_process(pStep->process() + 1);
  
  if (m_dwQuestID == 9956)
  {
    XLOG << "[BUG跟踪-任务] charid" << pUser->id << "questId" << m_dwQuestID << "npc" << pCFG->dwID << "killednum" << m_dwNum << "process" << pStep->process() << XEND;
  }

  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - reward
RewardStep::RewardStep(DWORD questid) : BaseStep(questid), m_dwRewardID(0)
{

}

RewardStep::~RewardStep()
{

}

bool RewardStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwRewardID = param.getTableInt("id");
  m_dwBuff = param.getTableInt("buff");
  m_dwSource = param.getTableInt("source");
  m_dwMailID = param.getTableInt("mailid");
  if(param.has("objects"))
    m_dwObjectType = param.getTableInt("objects");
  if(param.has("show"))
    m_dwShow = param.getTableInt("show");

  // get gm command
  m_oGMParams.setData("type", "reward");
  m_oGMParams.setData("id", m_dwRewardID);
  m_oGMParams.setData("2", 1);
  m_oGMParams.setData("show", m_dwShow);
  m_oGMParams.setData("doublesource", 0);
  m_oGMParams.setData("buff", m_dwBuff);
  m_oGMParams.setData("source", m_dwSource);

  if (param.has("method"))
  {
    DWORD dwMethod = param.getTableInt("method");
    if (dwMethod <= EPACKMETHOD_MIN || dwMethod >= EPACKMETHOD_MAX || dwMethod == EPACKMETHOD_CHECK_NOPILE || dwMethod == EPACKMETHOD_CHECK_WITHPILE)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method :" << dwMethod << "不合法" << XEND;
      return false;
    }
    m_oGMParams.setData("method", dwMethod);
  }
  else
  {
    m_oGMParams.setData("method", EPACKMETHOD_NOCHECK);
  }

  const SRewardCFG* pCFG = RewardConfig::getMe().getRewardCFG(m_dwRewardID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "reward:" << m_dwRewardID << "未在Table_Reward.txt表中找到" << XEND;
    return false;
  }

  return true;
}

bool RewardStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(getQuestID());
  if(pCFG && pCFG->eType == EQUESTTYPE_GUILD)
  {
    DWORD activityid = 0;
    /*const SGlobalActivityCFG* pCFG = MiscConfig::getMe().getGlobalActivityCFG(GACTIVITY_GUILD_QUEST);
    if(pCFG != nullptr)
      activityid = pCFG->activityid;*/
    const SGlobalActCFG*pCFG = ActivityConfig::getMe().getGlobalActCFG(GACTIVITY_GUILD_QUEST);
    if (pCFG)
      activityid = pCFG->m_dwId;
    bool isOpen = ActivityManager::getMe().isOpen(activityid);
    if(isOpen == true)
    {
      m_oGMParams.setData("doublesource", static_cast<DWORD>(EDOUBLESOURCE_GUILD_TASK));
      XLOG << "[公会任务], 双倍奖励, 工会任务:" << pUser->name << pUser->id << "subGroup:" << subGroup << "QuestID:" << getQuestID() << XEND;
    }
  }

  bool ret = true;
  if(m_dwObjectType & EREWARD_OBJECT_ME)
    ret = GMCommandRuler::getMe().execute(pUser, m_oGMParams);
  if(m_dwObjectType & EREWARD_OBJECT_VISITER)
  {
    SceneUser* pVisitUser = SceneUserManager::getMe().getUserByID(pUser->getVisitUser());
    if(pVisitUser == nullptr)
      return false;
    ret = GMCommandRuler::getMe().execute(pVisitUser, m_oGMParams);
  }
  if(m_dwObjectType & EREWARD_OBJECT_TEAM)
  {
    for (auto &m : pUser->getTeam().getTeamMemberList())
    {
      const TeamMemberInfo& rMember = m.second;
      SceneUser* pMember = SceneUserManager::getMe().getUserByID(rMember.charid());
      if (pMember)
        ret = GMCommandRuler::getMe().execute(pMember, m_oGMParams);
      else
      {
        GiveRewardSessionCmd cmd;
        cmd.set_charid(rMember.charid());
        cmd.set_rewardid(m_dwRewardID);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
      }
    }
  }
  if(m_dwObjectType & EREWARD_OBJECT_CHAT)
  {
    TVecItemInfo vecItem;
    RewardConfig::getMe().roll(m_dwRewardID, RewardEntry(), vecItem, ESOURCE_CHAT);
    MailManager::getMe().sendMail(pUser->getUserChat().getFriendChatID(), m_dwMailID, vecItem);
    XLOG << "[RewardStep] 聊天邮件奖励" << pUser->accid << pUser->id << pUser->name << "targetid :" << pUser->getUserChat().getFriendChatID() << "reward " <<
      m_dwRewardID << "mail: " << m_dwMailID << XEND;
  }

  if (ret == false)
  {
    XERR << "[RewardStep]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "rewardid :" << m_dwRewardID << "run error" << XEND;
    return false;
  }

  //platlog 奖励   todo 奖励物品
  std::string rewardItem ;
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Quest_Reward;
  PlatLogManager::getMe().eventLog(thisServer,
    pUser->getUserSceneData().getPlatformId(),
    pUser->getZoneID(),
    pUser->accid,
    pUser->id,
    eid,
    pUser->getUserSceneData().getCharge(), eType, 0, 1);

  PlatLogManager::getMe().QuestLog(thisServer,
    pUser->getUserSceneData().getPlatformId(),
    pUser->getZoneID(),
    pUser->accid,
    pUser->id,
    eType,
    eid,
    m_dwQuestID,
    EQuestType_REWARD,
    m_dwRewardID,  
    0,
    0,
    rewardItem,
    pUser->getLevel());

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - collect
CollectStep::CollectStep(DWORD questid) : BaseStep(questid)
{

}

CollectStep::~CollectStep()
{

}

bool CollectStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwNpcID = param.getTableInt("monster");
  m_dwGroupID = param.getTableInt("groupId");
  m_dwNum = param.getTableInt("num");
  if (param.has("reward"))
    m_dwReward = param.getTableInt("reward");
  else
    m_dwReward = 0;

  if (m_dwNpcID != 0)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwNpcID);
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "monster :" << m_dwNpcID << "未在Table_Npc.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool CollectStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(param1);
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  if (m_dwGroupID != 0)
  {
    if (pNpc->getCFG()->dwGroupID != m_dwGroupID)
      return false;
  }
  else
  {
    if (pNpc->getCFG()->dwID != m_dwNpcID)
      return false;
  }

  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;
  pStep->set_process(pStep->process() + 1);

  if (m_dwReward)
  {
  }
  else
  {//20000
    MsgManager::sendMsg(pUser->id, 20000, MsgParams(pNpc->getName()));
  }

  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - summon
SummonStep::SummonStep(DWORD questid) : BaseStep(questid)
{

}

SummonStep::~SummonStep()
{

}

bool SummonStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_oDefine.load(param);

  m_dwNum = param.getTableInt("num");
  m_dwNum = m_dwNum == 0 ? 1 : m_dwNum;
  m_dwUserRange = param.getTableInt("userRange");

  // check monster valid
  if (NpcConfig::getMe().getNpcCFG(m_oDefine.getID()) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id :" << m_oDefine.getID() << "未在Table_Npc.txt表中找到" << XEND;
    return false;
  }
  if (m_dwMapID == 0 && m_dwUserRange == 0)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "mapid :" << m_dwMapID << "userrange :" << m_dwUserRange << "值不合法" << XEND;
    return false;
  }

  return true;
}

bool SummonStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (pUser->getScene())
  {
    /*DWORD mapid = (m_dwMapID != 0 ? m_dwMapID : pUser->getScene()->getMapID());
    Scene* pScene = SceneManager::getMe().getSceneByID(mapid);

    if (pScene == nullptr)
    {
      DScene* pQuestScene = dynamic_cast<DScene*>(pUser->getScene());
      if (pQuestScene != nullptr)
        pScene = pQuestScene;
    }*/

    bool isImageScene = pUser->getScene()->m_oImages.isImageScene();
    if (isImageScene)
    {
      isImageScene = false;
      DScene* pDScene = dynamic_cast<DScene*>(pUser->getScene());
      if (pDScene && pDScene->getRaidID() == m_dwMapID)
        isImageScene = true;
    }

    //if (pScene == nullptr || pScene != pUser->getScene())
    if (pUser->getScene()->getMapID() != m_dwMapID && m_dwMapID != 0 && !isImageScene)
    {
      for (DWORD d = 0; d < m_dwNum; ++d)
      {
        if (pUser->getQuestNpc().addNpc(m_oDefine, m_dwMapID, m_dwQuestID) == false)
        {
          XERR << "[任务步骤-" << m_strContent << "]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "召唤npc :" << m_oDefine.getID() << "不在指定地图" << m_dwMapID << "添加离线失败" << XEND;
          return false;
        }
      }
    }
    else
    {
      m_oDefine.m_oVar.m_qwQuestOwnerID = pUser->id;
      if (m_dwUserRange != 0)
      {
        xPos pos;
        if (pUser->getScene()->getRandPos(pUser->getPos(), m_dwUserRange, pos))
          m_oDefine.setPos(pos);
      }
      Cmd::NtfVisibleNpcUserCmd npccmd;
      for (DWORD d = 0; d < m_dwNum; ++d)
      {
        SceneNpc *npc = SceneNpcManager::getMe().createNpc(m_oDefine, pUser->getScene());
        if (npc == nullptr)
        {
          XERR << "[任务步骤-" << m_strContent << "]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "创建 npc :" << m_oDefine.getID() << " map :" << m_dwMapID << "失败" << XEND;
          return false;
        }

        if (pUser->getQuestNpc().addNpc(npc, m_dwQuestID) == false)
        {
          XERR << "[任务步骤-" << m_strContent << "]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "添加 npc :" << m_oDefine.getID() << " map :" << m_dwMapID << "失败" << XEND;
          return false;
        }

        const SNpcCFG* pNpcCfg = npc->getCFG();
        if(pNpcCfg != nullptr && pNpcCfg->strMapIcon.empty() == false)
        {
          pUser->getScene()->refreshVisibleNpc(npccmd, npc);
        }
      }
      if(npccmd.npcs_size() > 0)
      {
        npccmd.set_type(1);
        PROTOBUF(npccmd, send, len);
        pUser->sendCmdToMe(send, len);
      }
      //platlog 召唤npc
      std::string rewardItem;

      QWORD eid = xTime::getCurUSec();
      EVENT_TYPE eType = EventType_Quest_Summon;
      PlatLogManager::getMe().eventLog(thisServer,
        pUser->getUserSceneData().getPlatformId(),
        pUser->getZoneID(),
        pUser->accid,
        pUser->id,
        eid,
        pUser->getUserSceneData().getCharge(), eType, 0, 1);

      PlatLogManager::getMe().QuestLog(thisServer,
        pUser->getUserSceneData().getPlatformId(),
        pUser->getZoneID(),
        pUser->accid,
        pUser->id,
        eType,
        eid,
        m_dwQuestID,
        EQuestType_SUMMON,
        m_oDefine.getID(),  /*召唤的npc id*/
        0,
        0,
        rewardItem,
        pUser->getLevel());
    }
  }
  else // pUser->getScene() == nullptr , 执行时, 玩家恰好不在场景
  {
    XLOG << "[任务步骤-summon], 玩家不在场景, 在map:" << m_dwMapID << "添加怪物" << XEND;
    for (DWORD d = 0; d < m_dwNum; ++d)
    {
      if (pUser->getQuestNpc().addNpc(m_oDefine, m_dwMapID, m_dwQuestID) == false)
      {
        XERR << "[任务步骤-" << m_strContent << "]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "召唤npc :" << m_oDefine.getID() << "不在指定地图" << m_dwMapID << "添加离线失败" << XEND;
        return false;
      }
    }
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - guard
GuardStep::GuardStep(DWORD questid) : BaseStep(questid)
{

}

GuardStep::~GuardStep()
{

}

bool GuardStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;
  return true;
}

bool GuardStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  return false;
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - gmcmd
GMCmdStep::GMCmdStep(DWORD questid) : BaseStep(questid)
{

}

GMCmdStep::~GMCmdStep()
{

}

bool GMCmdStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(m_dwQuestID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "questid :" << m_dwQuestID << "未在 Table_Quest.txt 表中找到" << XEND;
    return false;
  }

  m_oGMCmd = data.getMutableData("Params");
  m_oGMCmd.setData("questid", m_dwQuestID);
  m_oGMCmd.setData("share", QuestConfig::getMe().isShareQuest(pCFG->eType));
  return true;
}

bool GMCmdStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  bool bSuccess = GMCommandRuler::getMe().execute(pUser, m_oGMCmd);
  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - testfail
TestFailStep::TestFailStep(DWORD questid) : BaseStep(questid)
{

}

TestFailStep::~TestFailStep()
{

}

bool TestFailStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  return true;
}

bool TestFailStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  return pUser->getQuest().setQuestStep(m_dwQuestID, 0);
}

// quest step - use
UseStep::UseStep(DWORD questid) : BaseStep(questid)
{

}

UseStep::~UseStep()
{

}

bool UseStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  const xLuaData& param = data.getData("Params");
  m_dwDistance = param.getTableInt("distance");
  return true;
}

bool UseStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (pUser->getScene()->getMapID() != getMapID())
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
      pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未在地图mapid :" << m_dwMapID << "上执行" << XEND;
    return false;
  }
  if (m_bPos)
  {
    if (MiscConfig::getMe().getSystemCFG().bValidPosCheck)
    {
      float fDist = ::getXZDistance(pUser->getPos(), m_oDestPos);
      if (fDist > m_dwDistance * 1.5f)
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
          pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,距离目标点" << m_oDestPos.x << m_oDestPos.y << m_oDestPos.z
          << "为" << fDist << "超过" << m_dwDistance << XEND;
        return false;
      }
    }
  }
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - gather
GatherStep::GatherStep(DWORD questid) : BaseStep(questid)
{

}

GatherStep::~GatherStep()
{

}

bool GatherStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwMonsterID = param.getTableInt("monster");
  m_dwGroupID = param.getTableInt("groupId");
  m_dwRewardID = param.getTableInt("reward");

  const SRewardCFG* pCFG = RewardConfig::getMe().getRewardCFG(m_dwRewardID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "reward :" << m_dwRewardID << "未在Table_Reward.txt表中找到" << XEND;
    return false;
  }
  if (m_dwMonsterID != 0)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwMonsterID);
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "monster:" << m_dwRewardID << "未在Table_Monster.txt表中找到" << XEND;
      return false;
    }
  }

  m_dwNum = param.getTableInt("num");

  for (auto v = pCFG->vecItems.begin(); v != pCFG->vecItems.end(); ++v)
  {
    for (auto &item : v->vecItems)
      m_vecItemInfo.push_back(item.oItem);
  }
  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
    v->set_count(m_dwNum);
  if (m_vecItemInfo.empty() == true)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "reward:" << m_dwRewardID << "没有随机出任何物品" << XEND;
    return false;
  }

  m_bPrivateGather = param.getTableInt("private_gather") == 1;

  return true;
}

bool GatherStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  // check is gather item
  auto v = find_if(m_vecItemInfo.begin(), m_vecItemInfo.end(), [param1](const ItemInfo& rItem) -> bool{
    return rItem.id() == param1;
  });
  if (v == m_vecItemInfo.end())
    return false;

  // get item config
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(param1);
  if (pCFG == nullptr)
    return false;

  // get quest step
  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;

  if (pCFG->eItemType == EITEMTYPE_QUESTITEMCOUNT)
  {
    pStep->set_process(pStep->process() + 1);
  }
  else
  {
    BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    BasePackage* pQuestPack = pUser->getPackage().getPackage(EPACKTYPE_QUEST);
    if (pMainPack == nullptr || pQuestPack == nullptr)
      return false;

    bool bSuccess = true;
    for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
    {
      DWORD count = pMainPack->getItemCount(v->id());
      count += pQuestPack->getItemCount(v->id());

      pStep->set_process(count);
      if (count < m_dwNum)
      {
        bSuccess = false;
        break;
      }
    }
    if (!bSuccess)
    {
      if (pStep->process() >= m_dwNum)
        pStep->set_process(m_dwNum > 0 ? m_dwNum - 1 : m_dwNum);
    }
    else
      pStep->set_process(m_dwNum);
  }

  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  bool eResult = BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  if (eResult != false)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem != nullptr)
    {
      TPtrBaseStep pStepCFG = pItem->getStepCFG();
      if (pStepCFG != nullptr && pStepCFG->getStepType() == EQUESTSTEP_DELETE)
        pStepCFG->doStep(pUser);
    }
  }

  return eResult;
}

void GatherStep::collectDropItem(SceneUser* pUser, DWORD monsterid, TVecItemInfo& vecItemInfo)
{
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(monsterid);
  if (pCFG == nullptr)
    return;

  if (m_dwGroupID != 0 && m_dwGroupID != pCFG->dwGroupID)
    return;
  if (m_dwMonsterID != 0 && m_dwMonsterID != monsterid)
    return;

  RewardManager::roll(m_dwRewardID, nullptr,  vecItemInfo, ESOURCE_QUEST);
}

DWORD GatherStep::getRewardID(DWORD monsterid)
{
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(monsterid);
  if (pCFG == nullptr)
    return 0;

  if (m_dwGroupID != 0 && m_dwGroupID != pCFG->dwGroupID)
    return 0;
  if (m_dwMonsterID != 0 && m_dwMonsterID != monsterid)
    return 0;

  return m_dwRewardID;
}

// quest step - delete
DeleteStep::DeleteStep(DWORD questid) : BaseStep(questid)
{

}

DeleteStep::~DeleteStep()
{

}

bool DeleteStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  xLuaData& item = param.getMutableData("item");
  ItemInfo oItem;
  oItem.set_id(item.getTableInt("id"));
  oItem.set_count(item.getTableInt("num"));
  oItem.set_source(ESOURCE_QUEST);
  m_vecDeleteItems.push_back(oItem);

  for (auto v = m_vecDeleteItems.begin(); v != m_vecDeleteItems.end(); ++v)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id:" << v->id() << "未在Table_Item.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool DeleteStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
    return false;
  for (auto v = m_vecDeleteItems.begin(); v != m_vecDeleteItems.end(); ++v)
  {
    if (pMainPack->checkItemCount(v->id(), v->count()) == false)
      return false;
  }
  for (auto v = m_vecDeleteItems.begin(); v != m_vecDeleteItems.end(); ++v)
    pMainPack->reduceItem(v->id(), ESOURCE_QUEST, v->count());

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - raid
RaidStep::RaidStep(DWORD questid) : BaseStep(questid)
{

}

RaidStep::~RaidStep()
{

}

bool RaidStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwRaidID = param.getTableInt("id");
  m_dwRange = param.getTableInt("range");
  if (param.has("pos"))
  {
    m_oCenterPos.x = param.getMutableData("pos").getTableFloat("1");
    m_oCenterPos.y = param.getMutableData("pos").getTableFloat("2");
    m_oCenterPos.z = param.getMutableData("pos").getTableFloat("3");
  }
  return true;
}

bool RaidStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (param1 != m_dwRaidID || pUser == nullptr)
    return false;

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - camera
CameraStep::CameraStep(DWORD questid) : BaseStep(questid)
{

}

CameraStep::~CameraStep()
{

}

bool CameraStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  return true;
}

bool CameraStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - level
LevelStep::LevelStep(DWORD questid) : BaseStep(questid)
{

}

LevelStep::~LevelStep()
{

}

bool LevelStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwBaseLevel = param.getTableInt("base");
  m_dwJobLevel = param.getTableInt("job");
  return true;
}

bool LevelStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  if (m_dwBaseLevel > pUser->getUserSceneData().getRolelv() || m_dwJobLevel > pUser->getJobLv())
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - wait
WaitStep::WaitStep(DWORD questid) : BaseStep(questid)
{

}

WaitStep::~WaitStep()
{

}

bool WaitStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwTime = param.getTableInt("time");
  return true;
}

bool WaitStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  // get quest step
  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;
  if (pStep->process() == 0)
    pStep->set_process(xTime::getCurSec());
  if (param1 < m_dwTime + pStep->process())
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - move
MoveStep::MoveStep(DWORD questid) : BaseStep(questid)
{

}

MoveStep::~MoveStep()
{

}

bool MoveStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  const xLuaData& param = data.getData("Params");
  m_dwDistance = param.getTableInt("distance");
  return true;
}

bool MoveStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;
  if (pUser->getScene()->getMapID() != getMapID())
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
      pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未在地图mapid :" << m_dwMapID << "上执行" << XEND;
    return false;
  }
  if (MiscConfig::getMe().getSystemCFG().bValidPosCheck)
  {
    float fDist = ::getXZDistance(pUser->getPos(), m_oDestPos);
    if (fDist > m_dwDistance * 1.5f)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
        pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,距离目标点" << m_oDestPos.x << m_oDestPos.y << m_oDestPos.z
        << "为" << fDist << "超过" << m_dwDistance << XEND;
      return false;
    }
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - dialog
DialogStep::DialogStep(DWORD questid) : BaseStep(questid)
{

}

DialogStep::~DialogStep()
{

}

bool DialogStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  TSetDWORD setIDs;
  auto funcf = [&](const string& key, xLuaData& data)
  {
    setIDs.insert(data.getInt());
  };
  param.getMutableData("dialog").foreach(funcf);
  m_bDynamic = setIDs.empty();
  return true;
}

bool DialogStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (m_bDynamic)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return false;
    TVecQWORD vecIDs;
    pItem->collectQParams(vecIDs);
    if (vecIDs.empty() == true)
      return false;
    DWORD dwID = vecIDs.back();
    DialogBase* pBase = const_cast<DialogBase*>(TableManager::getMe().getDialogCFG(dwID));
    if (pBase == nullptr)
      return false;
    m_vecClientSelects = pBase->getSelectList();
  }

  if (m_vecClientSelects.empty() == true)
  {
    if (subGroup != 0)
      return false;

    subGroup = m_dwFinishJump;
  }
  else
  {
    auto v = find(m_vecClientSelects.begin(), m_vecClientSelects.end(), subGroup);
    if (v == m_vecClientSelects.end())
      return false;
    if (subGroup == 0)
      subGroup = m_dwFinishJump;
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

void DialogStep::toDynamicConfig(QuestStep* pStep, QWORD dialogid)
{
  if (pStep == nullptr || isDynamic() == false)
    return;

  ConfigParam* pParam = pStep->mutable_config()->mutable_params();
  pParam->Clear();

  Param* p = pParam->add_params();
  p->set_key("dialog");

  Param* pSub = p->add_items();
  Param* pItem = pSub->add_items();
  pItem->set_key("1");

  stringstream sstr;
  sstr << dialogid;
  pItem->set_value(sstr.str());
}

// quest step - prequest
PreQuestStep::PreQuestStep(DWORD questid) : BaseStep(questid)
{

}

PreQuestStep::~PreQuestStep()
{

}

bool PreQuestStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwPreQuest = param.getTableInt("id");
  return true;
}

bool PreQuestStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  if (pUser->getQuest().isSubmit(m_dwPreQuest) == false)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - clearnpc
ClearNpcStep::ClearNpcStep(DWORD questid) : BaseStep(questid)
{

}

ClearNpcStep::~ClearNpcStep()
{

}

bool ClearNpcStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwNpcID = param.getTableInt("id");
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwNpcID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id:" << m_dwNpcID << "未在Table_Npc.txt表中找到" << XEND;
    return false;
  }

  DWORD dwMethod = param.getTableInt("type");
  if (dwMethod != ECLEARMETHOD_DEAD && dwMethod != ECLEARMETHOD_LEAVE)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method:" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<EClearMethod>(dwMethod);

  if (param.has("effect") == true)
  {
    m_bEffect = true;
    m_oEffect = param.getMutableData("effect");
  }

  return true;
}

bool ClearNpcStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  if (!m_dwNpcID) return false;

  if (pUser->getScene())
  {
    xSceneEntrySet set;
    pUser->getScene()->getAllEntryList(SCENE_ENTRY_NPC, set);

    Cmd::NtfVisibleNpcUserCmd npccmd;
    for (auto it=set.begin(); it!=set.end(); ++it)
    {
      SceneNpc *npc = (SceneNpc *)(*it);
      if (npc->define.m_oVar.m_qwQuestOwnerID != pUser->id)
        continue;
      if (npc->getNpcID() != m_dwNpcID)
        continue;
      if (m_bEffect)
        GMCommandRuler::effect(npc, m_oEffect);

      if (m_eMethod == ECLEARMETHOD_LEAVE)
        npc->setStatus(ECREATURESTATUS_LEAVE);
      else if (m_eMethod == ECLEARMETHOD_DEAD)
        npc->setClearState();

      const SNpcCFG* pNpcCfg = npc->getCFG();
      if(pNpcCfg != nullptr && pNpcCfg->strMapIcon.empty() == false)
      {
        pUser->getScene()->refreshVisibleNpc(npccmd, npc);
      }
    }
    if(npccmd.npcs_size() > 0)
    {
      npccmd.set_type(0);
      PROTOBUF(npccmd, send, len);
      pUser->sendCmdToMe(send, len);
    }
  }

  pUser->getQuestNpc().delNpc(m_dwNpcID, m_dwQuestID / QUEST_ID_PARAM);

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - mount
MountStep::MountStep(DWORD questid) : BaseStep(questid)
{

}

MountStep::~MountStep()
{

}

bool MountStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD type = param.getTableInt("type");
  if (type <= ERIDETYPE_MIN || type >= ERIDETYPE_MAX)
    return false;

  m_eType = static_cast<ERideType>(param.getTableInt("type"));
  m_dwID = param.getTableInt("id");
  m_dwTime = param.getTableInt("time");

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(m_dwID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id:" << m_dwID << "未在Table_Item.txt表中找到" << XEND;
    return false;
  }

  return true;
}

bool MountStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (m_eType == ERIDETYPE_ON)
  {
    /*MainPackage* pPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
    if (pPack == nullptr)
      return false;*/
    //const string& guid = pPack->getMountGUID(m_dwID);
    //if (guid.empty() == true)
    {
      ItemInfo oItem;
      oItem.set_id(m_dwID);
      oItem.set_count(1);
      oItem.set_source(ESOURCE_QUEST);
      pUser->getPackage().addItem(oItem, EPACKMETHOD_AVAILABLE);
    }
    //pUser->getPackage().ride(m_eType, m_dwID);
  }
  else if (m_eType == ERIDETYPE_OFF)
    pUser->getPackage().ride(m_eType);

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - selfie
SelfieStep::SelfieStep(DWORD questid) : BaseStep(questid)
{

}

SelfieStep::~SelfieStep()
{

}

bool SelfieStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  const xLuaData& param = data.getData("Params");
  m_dwDistance = param.getTableInt("distance");
  return true;
}


// param2为999表示跳过检查直接完成
bool SelfieStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (param2 != 999)
  {
    if (pUser->getScene()->getMapID() != getMapID())
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
        pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未在地图mapid :" << m_dwMapID << "上执行" << XEND;
      return false;
    }
    if (m_bPos)
    {
      if (MiscConfig::getMe().getSystemCFG().bValidPosCheck)
      {
        float fDist = ::getXZDistance(pUser->getPos(), m_oDestPos);
        if (fDist > m_dwDistance * 1.5f)
        {
          XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "玩家" <<
            pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,距离目标点" << m_oDestPos.x << m_oDestPos.y << m_oDestPos.z
               << "为" << fDist << "超过" << m_dwDistance << XEND;
          return false;
        }
      }
    }
  }
  bool ret = BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  if (ret)
  {
    // 看板队长
    set<SceneUser*> uSet;
    pUser->getQuest().getWantedQuestLeaderTeammate(m_dwQuestID, uSet);
    if (!uSet.empty())
    {
      for (auto &u : uSet)
      {
        if (u->getQuest().runStep(m_dwQuestID, subGroup, param1, 999, sparam1))
          MsgManager::sendMsg(u->id, 4014);
      }
    }
  }
  return ret;
}

// quest step - checkteam
CheckTeamStep::CheckTeamStep(DWORD questid) : BaseStep(questid)
{
  m_dwMemberNum = 0;
  m_eSex = ESEXCOND_IGNOER;
}

CheckTeamStep::~CheckTeamStep()
{

}

bool CheckTeamStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;
  xLuaData& param = data.getMutableData("Params");
  m_dwMemberNum = param.getTableInt("player");
  m_dwType = param.getTableInt("type");
  if (ESEXCOND_IGNOER <= param.getTableInt("sex") && param.getTableInt("sex") <= ESEXCOND_DIFFERENT)
    m_eSex = param.getTableInt("sex");
  return true;
}

bool CheckTeamStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  subGroup = pUser->getTeamID() == 0 ? m_dwFailJump : m_dwFinishJump;
  //subGroup = m_dwFinishJump;
  if (subGroup == m_dwFailJump)
    return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);

  if (m_dwMemberNum != static_cast<DWORD>(pUser->getTeam().getTeamMemberList().size()))
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);

  const GTeam& rTeam = pUser->getTeam();
  //  检验人数
  if (m_dwMemberNum != 0)
  {
    DWORD dwNum = m_dwType == 0 ? pUser->getTeamMemberCount(pUser->getScene()->getMapID(), 0) : rTeam.getTeamMemberList().size();
    if (m_dwMemberNum > 0 && dwNum != m_dwMemberNum)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  }
  //  检验性别
  if (m_eSex != ESEXCOND_IGNOER && m_dwMemberNum >= 2)
  {
    vector<TeamMemberInfo> vecMember;
    for (auto &m : rTeam.getTeamMemberList())
    {
      vecMember.push_back(m.second);
      if (vecMember.size() >= 2)
        break;
    }
    if (vecMember.size() < 2)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);

    if (ESEXCOND_SAME == m_eSex && vecMember[0].gender() != vecMember[1].gender())
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    else if (ESEXCOND_DIFFERENT == m_eSex && vecMember[0].gender() == vecMember[1].gender())
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - remove money
RemoveMoneyStep::RemoveMoneyStep(DWORD questid) : BaseStep(questid)
{

}

RemoveMoneyStep::~RemoveMoneyStep()
{

}

bool RemoveMoneyStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwROB = param.getTableInt("zeny");

  DWORD dwType = param.getTableInt("type");
  if (dwType <= EMONEYTYPE_MIN || dwType >= EMONEYTYPE_MAX || EMoneyType_IsValid(dwType) == false)
    return false;
  m_eType = static_cast<EMoneyType>(dwType);
  m_dwCount = param.getTableInt("count");
  return true;
}

bool RemoveMoneyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (m_dwROB != 0)
  {
    if (pUser->checkMoney(EMONEYTYPE_SILVER, m_dwROB) == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  }

  if (m_eType != EMONEYTYPE_MIN)
  {
    if (pUser->checkMoney(m_eType, m_dwCount) == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  }

  if (m_dwROB != 0)
    pUser->subMoney(EMONEYTYPE_SILVER, m_dwROB, ESOURCE_QUEST);
  if (m_eType != EMONEYTYPE_MIN)
    pUser->subMoney(m_eType, m_dwCount, ESOURCE_QUEST);
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - class
ClassStep::ClassStep(DWORD questid) : BaseStep(questid)
{

}

ClassStep::~ClassStep()
{

}

bool ClassStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  DWORD pro = param.getTableInt("class");
  if (pro <= EPROFESSION_MIN || pro >= EPROFESSION_MAX || EProfession_IsValid(pro) == false)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "class:" << pro << "未在Table_Class.txt表中找到" << XEND;
    return false;
  }
  m_eProfession = static_cast<EProfession>(pro);

  m_bExact = param.getTableInt("exact") == 1;
  return true;
}

bool ClassStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  EProfession ePro = pUser->getUserSceneData().getProfession();
  EProfession eBasePro = RoleConfig::getMe().getBaseProfession(ePro);
  EProfession eBase = RoleConfig::getMe().getBaseProfession(m_eProfession);

  bool bSuccess = m_bExact ? ePro == m_eProfession : eBasePro == eBase;
  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - org class
OrgClassStep::OrgClassStep(DWORD questid) : BaseStep(questid)
{

}

OrgClassStep::~OrgClassStep()
{

}

bool OrgClassStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  DWORD pro = param.getTableInt("class");
  if (pro <= EPROFESSION_MIN || pro >= EPROFESSION_MAX || EProfession_IsValid(pro) == false)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "class:" << pro << "未在Table_Class.txt表中找到" << XEND;
    return false;
  }
  m_eProfession = static_cast<EProfession>(pro);

  m_bExact = param.getTableInt("exact") == 1;
  return true;
}

bool OrgClassStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  EProfession eFirst = pUser->getUserSceneData().getDestProfession();
  EProfession eBaseFirst = RoleConfig::getMe().getBaseProfession(eFirst);
  EProfession eBase = RoleConfig::getMe().getBaseProfession(m_eProfession);
  bool bSuccess = m_bExact ? eFirst == m_eProfession : eBaseFirst == eBase;
  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - evo
EvoStep::EvoStep(DWORD questid) : BaseStep(questid)
{

}

EvoStep::~EvoStep()
{

}

bool EvoStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwEvo = param.getTableInt("evo");
  m_bExact = param.getTableInt("exact") == 1;
  return true;
}

bool EvoStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  DWORD pro = pUser->getUserSceneData().getProfession();
  DWORD evo = pro == EPROFESSION_NOVICE ? 0 : pro - pro / 10 * 10;
  bool bSuccess = m_bExact ? evo == m_dwEvo : evo >= m_dwEvo;

  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - check quest
CheckQuestStep::CheckQuestStep(DWORD questid) : BaseStep(questid)
{

}

CheckQuestStep::~CheckQuestStep()
{

}

bool CheckQuestStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwQuestID = param.getTableInt("id");  // id valid check run in QuestConfig::checkConfig
  m_bStart = param.getTableInt("start") == 1;
  m_bFinish = param.getTableInt("finish") == 1;

  return true;
}

bool CheckQuestStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  auto checkstart = [&]()
  {
    const TVecDWORD& vecIDs = QuestConfig::getMe().getQuestDetail(m_dwQuestID);

    Quest& rQuest = pUser->getQuest();
    for (auto o = vecIDs.begin(); o != vecIDs.end(); ++o)
    {
      TPtrQuestItem pItem = rQuest.getQuest(*o);
      if (pItem != nullptr && pItem->isComplete() == false)
        return true;
    }

    return false;
  };

  // one : start-true finish-true
  if (m_bStart && m_bFinish)
  {
    if (QuestManager::getMe().isQuestComplete(m_dwQuestID, pUser) == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }

  // two : start-true finish-false
  if (m_bStart && !m_bFinish)
  {
    if (checkstart() == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }

  // three : start-false finish-false
  if (!m_bStart && !m_bFinish)
  {
    if (checkstart() == true || QuestManager::getMe().isQuestComplete(m_dwQuestID, pUser) == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }

  // four : start-false finish
  if (!m_bStart && m_bFinish)
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);

  return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
}

// quest step - check group
CheckGroupStep::CheckGroupStep(DWORD questid) : BaseStep(questid)
{

}

CheckGroupStep::~CheckGroupStep()
{

}

bool CheckGroupStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwQuestID = param.getTableInt("id");  // id valid check run in QuestConfig::checkConfig
  m_bStart = param.getTableInt("start") == 1;
  m_bFinish = param.getTableInt("finish") == 1;

  m_dwMapID = param.getTableInt("mapid");
  m_dwNum = param.getTableInt("num");
  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod != ECHECKMETHOD_NORMAL && dwMethod != ECHECKMETHOD_DAILYRAND)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method :" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<ECheckMethod>(dwMethod);

  if (m_dwMapID != 0 && MapConfig::getMe().getMapCFG(m_dwMapID) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "mapid :" << m_dwMapID << "未在 Table_Map.txt 表中找到" << XEND;
    return false;
  }

  if (!m_bStart && m_bFinish)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "start : 0, finish : 1 不存在该情况" << XEND;
    return false;
  }

  return true;
}

bool CheckGroupStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  Quest& rQuest = pUser->getQuest();
  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);

  if (m_eMethod == ECHECKMETHOD_NORMAL)
  {
    if (m_bFinish)
    {
      if (pItem == nullptr || rQuest.isSubmit(m_dwQuestID) == false)
        return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    }
    else
    {
      if (pItem != nullptr && rQuest.isSubmit(m_dwQuestID) == true)
        return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    }

    if (m_bStart)
    {
      if (pItem == nullptr || rQuest.isAccept(m_dwQuestID) == false)
        return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    }
    else
    {
      if (pItem != nullptr && rQuest.isAccept(m_dwQuestID) == true)
        return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    }

    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }
  else if (m_eMethod == ECHECKMETHOD_DAILYRAND)
  {
    if (m_dwQuestID != 0)
    {
      if (pUser->getQuest().isSubmit(m_dwQuestID) == false)
        return false;
    }
    else
    {
      if (pUser->getQuest().getDailyRandCount(m_dwMapID, true) < m_dwNum)
        return false;
    }
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }

  return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
}

// quest step - check item
CheckItemStep::CheckItemStep(DWORD questid) : BaseStep(questid)
{

}

CheckItemStep::~CheckItemStep()
{

}

bool CheckItemStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  xLuaData& item = param.getMutableData("item");
  auto itemf = [this](const string& str, xLuaData& data)
  {
    ItemInfo oItem;
    oItem.set_id(data.getTableInt("id"));
    oItem.set_count(data.getTableInt("num"));

    combinItemInfo(m_vecItemInfo, TVecItemInfo{oItem});
  };
  item.foreach(itemf);

  m_bEquip = param.getTableInt("equip") == 1;
  m_bStoreage = param.getTableInt("storage") == 1;
  m_bPStoreage = param.getTableInt("pstorage") == 1;
  m_bCheckAll = param.getTableInt("checkall") == 1;

  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
  {
    if (ItemManager::getMe().getItemCFG(v->id()) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id:" << v->id() << "未在Table_Item.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool CheckItemStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
    return false;

  TVecItemInfo vecItem;
  if (collectPackItem(pUser, EPACKTYPE_MAIN, vecItem) == false)
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  if (collectPackItem(pUser, EPACKTYPE_QUEST, vecItem) == false)
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  if (collectPackItem(pUser, EPACKTYPE_FOOD, vecItem) == false)
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);

  if (m_bEquip)
  {
    if (collectPackItem(pUser, EPACKTYPE_EQUIP, vecItem) == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  }
  if (m_bStoreage)
  {
    if (collectPackItem(pUser, EPACKTYPE_STORE, vecItem) == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  }
  if (m_bPStoreage)
  {
    if (collectPackItem(pUser, EPACKTYPE_STORE, vecItem) == false)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  }

  if (m_bCheckAll)
  {
    bool bSuccess = true;
    for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
    {
      DWORD dwTypeID = v->id();
      auto o = find_if(vecItem.begin(), vecItem.end(), [dwTypeID](const ItemInfo& rItem) -> bool{
        return dwTypeID == rItem.id();
      });
      if (o == vecItem.end() || o->count() < v->count())
      {
        bSuccess = false;
        break;
      }
    }

    subGroup = !bSuccess ? m_dwFailJump : m_dwFinishJump;
    return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
  }

  bool bSuccess = false;
  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
  {
    DWORD dwTypeID = v->id();
    auto o = find_if(vecItem.begin(), vecItem.end(), [dwTypeID](const ItemInfo& rItem) -> bool{
      return dwTypeID == rItem.id();
    });
    if (o != vecItem.end() && o->count() >= v->count())
    {
      bSuccess = true;
      break;
    }
  }

  subGroup = !bSuccess ? m_dwFailJump : m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

bool CheckItemStep::collectPackItem(SceneUser* pUser, EPackType eType, TVecItemInfo& vecResult)
{
  if (pUser == nullptr)
    return false;

  BasePackage* pPack = pUser->getPackage().getPackage(eType);
  if (pPack == nullptr)
    return false;

  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
  {
    DWORD count = pPack->getItemCount(v->id(), ECHECKMETHOD_NORMAL);

    ItemInfo oItem;
    oItem.set_id(v->id());
    oItem.set_count(count);

    combinItemInfo(vecResult, TVecItemInfo{oItem});
  }

  return true;
}

// quest step - remove item
RemoveItemStep::RemoveItemStep(DWORD questid) : BaseStep(questid)
{

}

RemoveItemStep::~RemoveItemStep()
{

}

bool RemoveItemStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod >= EREMOVEMETHOD_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method :" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<ERemoveMethod>(dwMethod);

  xLuaData& item = param.getMutableData("item");
  auto itemf = [this](const string& str, xLuaData& data)
  {
    ItemInfo oItem;

    oItem.set_id(data.getTableInt("id"));
    oItem.set_count(data.getTableInt("num"));

    combinItemInfo(m_vecItemInfo, TVecItemInfo{oItem});
  };
  item.foreach(itemf);

  m_bAll = param.getTableInt("all") == 1;
  m_bTemp = param.getTableInt("temp") == 1;

  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
  {
    if (ItemManager::getMe().getItemCFG(v->id()) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id:" << v->id() << "未在Table_Item.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool RemoveItemStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  BasePackage* pQuestPack = pUser->getPackage().getPackage(EPACKTYPE_QUEST);
  BasePackage* pFoodPack = pUser->getPackage().getPackage(EPACKTYPE_FOOD);
  BasePackage* pTempPack = pUser->getPackage().getPackage(EPACKTYPE_TEMP_MAIN);
  if (pMainPack == nullptr || pQuestPack == nullptr || pTempPack == nullptr)
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);

  if (m_bAll)
  {
    if (m_eMethod == EREMOVEMETHOD_REMOVECOUNT)
    {
      if (pUser->getPackage().reduceItem(m_vecItemInfo, ESOURCE_QUEST, ECHECKMETHOD_NONORMALEQUIP, TSetDWORD{EPACKTYPE_MAIN, EPACKTYPE_QUEST, EPACKTYPE_FOOD}) == false)
        return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    }
    else if (m_eMethod == EREMOVEMETHOD_REMOVEALL)
    {
      for (auto &v : m_vecItemInfo)
      {
        pUser->getPackage().itemRemove(v.id(), EPACKTYPE_MAIN, ESOURCE_QUEST);
        pUser->getPackage().itemRemove(v.id(), EPACKTYPE_QUEST, ESOURCE_QUEST);
        pUser->getPackage().itemRemove(v.id(), EPACKTYPE_FOOD, ESOURCE_QUEST);
        if (m_bTemp)
          pUser->getPackage().itemRemove(v.id(), EPACKTYPE_TEMP_MAIN, ESOURCE_QUEST);
      }
    }
  }
  else
  {
    if (m_eMethod == EREMOVEMETHOD_REMOVECOUNT)
    {
      bool bSuccess = false;
      TVecItemInfo vecCopy = m_vecItemInfo;
      for (auto &v : vecCopy)
      {
        DWORD dwMainCount = pMainPack->getItemCount(v.id());
        DWORD dwQuestCount = pQuestPack->getItemCount(v.id());
        DWORD dwFoodCount = pFoodPack->getItemCount(v.id());
        DWORD dwTempCount = 0;
        if (m_bTemp)
          dwTempCount = pTempPack->getItemCount(v.id());
        if (v.count() > dwMainCount + dwQuestCount + dwFoodCount + dwTempCount)
          continue;

        do
        {
          if (dwMainCount != 0)
          {
            DWORD dwDec = dwMainCount >= v.count() ? v.count() : dwMainCount;
            v.set_count(v.count() - dwDec);
            pMainPack->reduceItem(v.id(), ESOURCE_QUEST, dwDec);
          }
          if (v.count() == 0)
          {
            bSuccess = true;
            break;
          }
          if (dwQuestCount != 0)
          {
            DWORD dwDec = dwQuestCount >= v.count() ? v.count() : dwQuestCount;
            v.set_count(v.count() - dwDec);
            pQuestPack->reduceItem(v.id(), ESOURCE_QUEST, dwDec);
          }
          if (v.count() == 0)
          {
            bSuccess = true;
            break;
          }
          if (dwFoodCount != 0)
          {
            DWORD dwDec = dwFoodCount >= v.count() ? v.count() : dwFoodCount;
            v.set_count(v.count() - dwDec);
            pFoodPack->reduceItem(v.id(), ESOURCE_QUEST, dwDec);
          }
          if (v.count() == 0)
          {
            bSuccess = true;
            break;
          }
          if (dwTempCount != 0)
          {
            DWORD dwDec = dwTempCount >= v.count() ? v.count() : dwTempCount;
            v.set_count(v.count() - dwDec);
            pTempPack->reduceItem(v.id(), ESOURCE_QUEST, dwDec);
          }
          if (v.count() == 0)
          {
            bSuccess = true;
            break;
          }
        } while(0);
        break;
      }
      if (!bSuccess)
        return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    }
    else if (m_eMethod == EREMOVEMETHOD_REMOVEALL)
    {
      ItemInfo* pInfo = randomStlContainer(m_vecItemInfo);
      if (pInfo == nullptr)
        return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);

      pUser->getPackage().itemRemove(pInfo->id(), EPACKTYPE_MAIN, ESOURCE_QUEST);
      pUser->getPackage().itemRemove(pInfo->id(), EPACKTYPE_QUEST, ESOURCE_QUEST);
      pUser->getPackage().itemRemove(pInfo->id(), EPACKTYPE_FOOD, ESOURCE_QUEST);
      if (m_bTemp)
        pUser->getPackage().itemRemove(pInfo->id(), EPACKTYPE_TEMP_MAIN, ESOURCE_QUEST);
    }
  }

  bool ret = BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  if (ret)
  {
    // 看板队长
    set<SceneUser*> uSet;
    pUser->getQuest().getWantedQuestLeaderTeammate(m_dwQuestID, uSet);
    if (!uSet.empty())
    {
      string itemname;
      if (!m_vecItemInfo.empty())
      {
        const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(m_vecItemInfo[0].id());
        if (pItemCfg != nullptr)
          itemname = pItemCfg->strNameZh;
      }
      for (auto &u : uSet)
      {
        if (u->getQuest().runStep(m_dwQuestID, subGroup, param1, param2, sparam1))
          MsgManager::sendMsg(u->id, 4013, MsgParams(itemname));
      }
    }
  }
  return ret;
}

// quest step - random jump
RandomJumpStep::RandomJumpStep(DWORD questid) : BaseStep(questid)
{

}

RandomJumpStep::~RandomJumpStep()
{

}

bool RandomJumpStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  xLuaData& id = param.getMutableData("id");
  auto idf = [this](const string& key, xLuaData& data)
  {
    pair<DWORD, DWORD> p;
    p.first = data.getTableInt("1");
    p.second = data.getTableInt("2");

    m_vecJumpPoint.push_back(p);
  };
  id.foreach(idf);

  sort(m_vecJumpPoint.begin(), m_vecJumpPoint.end(), [](const pair<DWORD, DWORD>& r1, const pair<DWORD, DWORD>& r2) -> bool{
    return r1.second < r2.second;
  });

  DWORD dwLast = 0;
  for (auto v = m_vecJumpPoint.begin(); v != m_vecJumpPoint.end(); ++v)
  {
    v->second += dwLast;
    dwLast = v->second;
    m_dwMaxRand = dwLast;
  }

  return true;
}

bool RandomJumpStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (m_dwSubGroup == 0)
    return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);

  DWORD rand = randBetween(0, m_dwMaxRand);
  for (auto v = m_vecJumpPoint.begin(); v != m_vecJumpPoint.end(); ++v)
  {
    if (rand <= v->second)
    {
      const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(m_dwQuestID);
      if (pCFG != nullptr)
        return pUser->getQuest().setQuestStep(m_dwQuestID, pCFG->getStepBySubGroup(v->first));
    }
  }

  return false;
}

// quest step - check level
CheckLevelStep::CheckLevelStep(DWORD questid) : BaseStep(questid)
{

}

CheckLevelStep::~CheckLevelStep()
{

}

bool CheckLevelStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwBaseLv = param.getTableInt("base");
  m_dwJobLv = param.getTableInt("job");
  return true;
}

bool CheckLevelStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  subGroup = m_dwBaseLv > pUser->getUserSceneData().getRolelv() || m_dwJobLv > pUser->getJobLv() ? m_dwFailJump : m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - check gear
CheckGearStep::CheckGearStep(DWORD questid) : BaseStep(questid)
{

}

CheckGearStep::~CheckGearStep()
{

}

bool CheckGearStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwGearID = param.getTableInt("id");
  m_dwStateID = param.getTableInt("state");
  return true;
}

bool CheckGearStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (pUser->getScene() == nullptr)
    return false;

  subGroup = m_dwStateID!=pUser->getScene()->m_oGear.get(m_dwGearID) ? m_dwFailJump : m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - purify
PurifyStep::PurifyStep(DWORD questid) : BaseStep(questid)
{

}

PurifyStep::~PurifyStep()
{

}

bool PurifyStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  return true;
}

bool PurifyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - action
ActionStep::ActionStep(DWORD questid) : BaseStep(questid)
{

}

ActionStep::~ActionStep()
{

}

bool ActionStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_qwID = param.getTableInt("id");
  m_dwType = param.getTableInt("type");

  m_dwObjects = param.getTableInt("objects");
  m_dwNpcID = param.getTableInt("npcid");
  m_dwRelateion = param.getTableInt("relation");
  m_dwRange = param.getTableInt("distance");
  if (param.has("pos"))
  {
    m_oCenterPos.x = param.getMutableData("pos").getTableFloat("1");
    m_oCenterPos.y = param.getMutableData("pos").getTableFloat("2");
    m_oCenterPos.z = param.getMutableData("pos").getTableFloat("3");
  }

  if (param.has("map"))
  {
    auto mapfunc = [&](const string& key, xLuaData& data)
    {
      if (data.getInt() != 0)
        m_vecMapID.emplace_back(data.getInt());
    };
    param.getMutableData("map").foreach(mapfunc);
  }

  if (m_dwNpcID == 0 && m_dwObjects == 2)
  {
    auto gettypes = [&](const string& key, xLuaData& data)
    {
      const string& name = data.getString();
      ENpcType eType = NpcConfig::getMe().getNpcType(name);
      if (eType == ENPCTYPE_MIN)
      {
        XERR << "[GM-add_extra_reward], 怪物类型配置错误" << name << XEND;
        return;
      }
      m_setNpcTypes.insert(eType);
    };
    param.getMutableData("npctype").foreach(gettypes);

    auto getzonetypes = [&](const string& key, xLuaData& data)
    {
      const string& name = data.getString();
      ENpcZoneType eType = NpcConfig::getMe().getZoneType(name);
      if (eType == ENPCZONE_MIN)
      {
        XERR << "[GM-add_extra_reward], 怪物区域配置错误" << name << XEND;
        return;
      }
      m_setZoneTypes.insert(eType);
    };
    param.getMutableData("zonetype").foreach(getzonetypes);

    auto getraces = [&](const string& key, xLuaData& data)
    {
      m_setRaceTypes.insert(NpcConfig::getMe().getRaceType(data.getString()));
    };
    param.getMutableData("racetype").foreach(getraces);

    auto getnatures = [&](const string& key, xLuaData& data)
    {
      m_setNatureTypes.insert(NpcConfig::getMe().getNatureType(data.getString()));
    };
    param.getMutableData("naturetype").foreach(getnatures);
  }
  return true;
}

bool ActionStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if(m_dwType != param2)
    return false;

  if (m_qwID != param1)
    return false;

  if(m_dwObjects == 1)
  {
    SceneUser* pVisitUser = SceneUserManager::getMe().getUserByID(pUser->getVisitUser());
    if(pVisitUser == nullptr)
      return false;
    if(m_dwRelateion == 1 && pUser->getSocial().checkRelation(pVisitUser->id, ESOCIALRELATION_FRIEND) == false)
      return false;
    else if(m_dwRelateion == 2 && pUser->getSocial().checkRelation(pVisitUser->id, ESOCIALRELATION_TEAM) == false)
      return false;
    else if(m_dwRelateion == 3 && pUser->getGuild().id() != pVisitUser->getGuild().id())
      return false;
    else if(m_dwRelateion == 4 && pUser->getUserWedding().getWeddingParnter() != pVisitUser->id)
      return false;
    else if(m_dwRelateion == 5 && pUser->getUserSceneData().getGender() == pVisitUser->getUserSceneData().getGender())
      return false;
  }
  else if(m_dwObjects == 2)
  {
    SceneNpc* pNpc = pUser->getVisitNpcObj();
    if(pNpc == nullptr)
      return false;
    if(m_dwNpcID != 0)
    {
      if(pNpc->getNpcID() != m_dwNpcID)
        return false;
    }
    else
    {
      const SNpcCFG* pCFG = pNpc->getCFG();
      if(pCFG == nullptr)
        return false;
      if(m_setZoneTypes.empty() == false)
      {
        auto it = m_setZoneTypes.find(pCFG->eZoneType);
        if(it == m_setZoneTypes.end())
          return false;
      }
      if(m_setNpcTypes.empty() == false)
      {
        auto it = m_setNpcTypes.find(pCFG->eNpcType);
        if(it == m_setNpcTypes.end())
          return false;
      }
      if(m_setRaceTypes.empty() == false)
      {
        auto it = m_setRaceTypes.find(pCFG->eRaceType);
        if(it == m_setRaceTypes.end())
          return false;
      }
      if(m_setNatureTypes.empty() == false)
      {
        auto it = m_setNatureTypes.find(pCFG->eNatureType);
        if(it == m_setNatureTypes.end())
          return false;
      }
    }
  }

  if(m_vecMapID.empty() == false)
  {
    if(pUser->getScene() == nullptr)
      return false;
    auto it = find(m_vecMapID.begin(), m_vecMapID.end(), pUser->getScene()->getMapID());
    if(it == m_vecMapID.end())
      return false;
  }

  if(m_dwRange != 0 && getDistance(pUser->getPos(), m_oCenterPos) > m_dwRange)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - skill
SkillStep::SkillStep(DWORD questid) : BaseStep(questid)
{

}

SkillStep::~SkillStep()
{

}

bool SkillStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  xLuaData& npc = param.getMutableData("npc");
  auto npcf = [this](const string& str, xLuaData& data)
  {
    m_vecNpcIDs.push_back(data.getInt());
  };
  npc.foreach(npcf);
  if (m_vecNpcIDs.empty() == true)
    return false;
  for (auto v = m_vecNpcIDs.begin(); v != m_vecNpcIDs.end(); ++v)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(*v);
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "npc :" << *v << "未在Table_Npc.txt表中找到" << XEND;
      return false;
    }
  }

  m_dwSkillID = param.getTableInt("skillid");
  const BaseSkill* pCFG = SkillManager::getMe().getSkillCFG(m_dwSkillID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "skillid :" << m_dwSkillID << "未在Table_Skill.txt表中找到" << XEND;
    return false;
  }

  DWORD dwStatus = param.getTableInt("status");
  if (dwStatus <= ESKILLSTATUS_MIN || dwStatus >= ESKILLSTATUS_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "status :" << dwStatus << "不存在" << XEND;
    return false;
  }
  m_eStatus = static_cast<ESkillStatus>(dwStatus);

  m_oDestPos.x = param.getMutableData("skillpos").getTableInt("1");
  m_oDestPos.y = param.getMutableData("skillpos").getTableInt("2");
  m_oDestPos.z = param.getMutableData("skillpos").getTableInt("3");

  return true;
}

bool SkillStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  if (pUser->getScene() == nullptr)
    return false;

  xSceneEntrySet set;
  pUser->getScene()->getAllEntryList(SCENE_ENTRY_NPC, set);
  for (auto it = set.begin(); it != set.end(); ++it)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(*it);
    if (pNpc == nullptr || pNpc->define.m_oVar.m_qwQuestOwnerID != pUser->id)
      continue;

    SkillBroadcastUserCmd cmd;
    cmd.set_charid(pNpc->id);
    cmd.set_skillid(m_dwSkillID);

    cmd.mutable_data()->mutable_pos()->set_x(m_oDestPos.getX());
    cmd.mutable_data()->mutable_pos()->set_y(m_oDestPos.getY());
    cmd.mutable_data()->mutable_pos()->set_z(m_oDestPos.getZ());

    if (m_eStatus == ESKILLSTATUS_CHANT)
      cmd.mutable_data()->set_number(ESKILLNUMBER_CHANT);
    else if (m_eStatus == ESKILLSTATUS_RUN)
      cmd.mutable_data()->set_number(ESKILLNUMBER_ATTACK);
    else if (m_eStatus == ESKILLSTATUS_STOP)
      cmd.mutable_data()->set_number(-1);

    PROTOBUF(cmd, send, len);
    pNpc->sendCmdToNine(send, len);
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - interlocution
InterStep::InterStep(DWORD questid) : BaseStep(questid)
{

}

InterStep::~InterStep()
{

}

bool InterStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwInterID = param.getTableInt("id");

  const SInterlocution* pCFG = TableManager::getMe().getInterCFG(m_dwInterID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id :" << m_dwInterID << "在Table_xo_server.txt表中找到" << XEND;
    return false;
  }

  return true;
}

void InterStep::reset(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return;
  pStep->set_process(0);

  BaseStep::reset(pUser);
}

bool InterStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;
  if (pStep->process() == 0)
  {
    pUser->getInterlocution().addInterlocution(m_dwInterID, ESOURCE_QUEST);
    pStep->set_process(1);
    return false;
  }

  if (m_dwInterID != param1)
    return false;
  subGroup = param2 == 0 ? m_dwFailJump : m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}


// quest step - guid
GuideStep::GuideStep(DWORD questid) : BaseStep(questid)
{

}

GuideStep::~GuideStep()
{

}

bool GuideStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  const string& str = data.getTableString("Content");
  if (str == "guide")
    m_eGuideStep = EQUESTSTEP_GUIDE;
  else if (str == "guide_highlight")
    m_eGuideStep = EQUESTSTEP_GUIDE_HIGHLIGHT;
  else if (str == "guideLockMonster")
    m_eGuideStep = EQUESTSTEP_GUIDELOCKMONSTER;
  
  if (m_eGuideStep == EQUESTSTEP_MIN)
    return false;

  return true;
}

bool GuideStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  DWORD dwSubGroup = subGroup == 0 ? m_dwFinishJump : subGroup;
  if (dwSubGroup != m_dwFinishJump && dwSubGroup != m_dwFailJump)
    return false;

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - guid check
GuideCheckStep::GuideCheckStep(DWORD questid) : BaseStep(questid)
{

}

GuideCheckStep::~GuideCheckStep()
{

}

bool GuideCheckStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwAttrPoint = param.getTableInt("attrpoint");
  m_dwSkillPoint = param.getTableInt("skillpoint");
  m_dwRob = param.getTableInt("rob");
  m_dwSkillID = param.getTableInt("skillid");

  xLuaData& skilllv = param.getMutableData("skilllv");
  auto skillvf = [&](const string& key, xLuaData& data)
  {
    DWORD dwSkillID = data.getTableInt("1");
    DWORD dwSkillLv = data.getTableInt("2");

    m_vecSkillLv.push_back(pair<DWORD, DWORD>(dwSkillID, dwSkillLv));
  };
  skilllv.foreach(skillvf);

  if (m_dwSkillID != 0)
  {
    const SSkillCFG* pSkillCFG = SkillConfig::getMe().getSkillCFG(m_dwSkillID);
    if (pSkillCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "skillid :" << m_dwSkillID << "在Table_Skill.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool GuideCheckStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  bool bSuccess = false;
  if (m_dwAttrPoint != 0)
  {
    SceneFighter* pFighter = pUser->getFighter();
    if (pFighter == nullptr)
      return false;
    bSuccess = pFighter->getTotalPoint() >= m_dwAttrPoint;
  }

  if (m_dwSkillPoint != 0)
  {
    SceneFighter* pFighter = pUser->getFighter();
    if (pFighter == nullptr)
      return false;
    bSuccess = pFighter->getSkill().getSkillPoint() >= m_dwSkillPoint;
  }

  if (m_dwRob != 0)
    bSuccess = pUser->checkMoney(EMONEYTYPE_SILVER, m_dwRob);

  if (m_dwSkillID != 0)
    bSuccess = pUser->getLearnedSkillByID(m_dwSkillID);

  if (m_vecSkillLv.empty() == false)
  {
    bSuccess = true;
    SceneFighter* pFighter = pUser->getFighter();
    if (pFighter == nullptr)
      return false;
    for (auto v = m_vecSkillLv.begin(); v != m_vecSkillLv.end(); ++v)
    {
      if (pFighter->getSkill().getSkillLv(v->first) >= v->second)
      {
        bSuccess = false;
        break;
      }
    }
  }

  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - empty
EmptyStep::EmptyStep(DWORD questid) : BaseStep(questid)
{

}

EmptyStep::~EmptyStep()
{

}

bool EmptyStep::init(xLuaData& data)
{
  return BaseStep::init(data);
}

void EmptyStep::reset(SceneUser* pUser)
{
  BaseStep::reset(pUser);
}

bool EmptyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - check strength
CheckEquipLvStep::CheckEquipLvStep(DWORD questid) : BaseStep(questid)
{

}

CheckEquipLvStep::~CheckEquipLvStep()
{

}

bool CheckEquipLvStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwTypeID = param.getTableInt("id");
  m_pairLv.first = param.getTableInt("min");
  m_pairLv.second = param.getTableInt("max");

  m_bEquip = param.getTableInt("equip") == 0;
  m_bStoreage = param.getTableInt("storage") == 1;

  if (m_dwTypeID != 0)
  {
    if (ItemConfig::getMe().getItemCFG(m_dwTypeID) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id :" << m_dwTypeID << "未在Table_Item.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool CheckEquipLvStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  BasePackage* pPackage = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pPackage == nullptr)
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  if (pPackage->hasItem(m_dwTypeID, m_pairLv.first, m_pairLv.second) == true)
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);

  if (m_bEquip)
  {
    BasePackage* pPackage = pUser->getPackage().getPackage(EPACKTYPE_EQUIP);
    if (pPackage == nullptr)
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    if (pPackage->hasItem(m_dwTypeID, m_pairLv.first, m_pairLv.second) == true)
      return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }

  //if (m_bStoreage)
  //{
  //  BasePackage* pPackage = pUser->getPackage().getPackage(EPACKTYPE_STORE);
  //  if (pPackage == nullptr)
  //    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
  //  if (pPackage->hasItem(m_dwTypeID, m_pairLv.first, m_pairLv.second) == true)
  //    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  //}

  return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
}

// quest step - check money
CheckMoneyStep::CheckMoneyStep(DWORD questid) : BaseStep(questid)
{

}

CheckMoneyStep::~CheckMoneyStep()
{

}

bool CheckMoneyStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwType = param.getTableInt("type");
  if (dwType <= EMONEYTYPE_MIN || dwType >= EMONEYTYPE_MAX || EMoneyType_IsValid(dwType) == false)
    return false;
  m_eType = static_cast<EMoneyType>(dwType);

  m_dwCount = param.getTableInt("count");

  return true;
}

bool CheckMoneyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  DWORD dwCount = 0;

  switch (m_eType)
  {
    case EMONEYTYPE_MIN:
      break;
    case EMONEYTYPE_DIAMOND:
      dwCount = pUser->getUserSceneData().getDiamond();
      break;
    case EMONEYTYPE_SILVER:
      if (pUser->getUserSceneData().getSilver() > 0)
        dwCount = pUser->getUserSceneData().getSilver();
      break;
    case EMONEYTYPE_GOLD:
      dwCount = pUser->getUserSceneData().getGold();
      break;
    case EMONEYTYPE_GARDEN:
      dwCount = pUser->getUserSceneData().getGarden();
      {
        BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
        if (pMainPack != nullptr)
          dwCount = pMainPack->getItemCount(ITEM_GARDEN);
      }
      // 这里特殊处理,乐园币移动到背包,不删除原type
      //dwCount = pUser->getUserSceneData().getFriendShip();
      break;
    case EMONEYTYPE_FRIENDSHIP:
      {
        BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
        if (pMainPack != nullptr)
          dwCount = pMainPack->getItemCount(ITEM_FRIENDSHIP);
      }
      // 这里特殊处理,友情之证移动到背包,不删除原type
      //dwCount = pUser->getUserSceneData().getFriendShip();
      break;
    case EMONEYTYPE_CONTRIBUTE:
    case EMONEYTYPE_GUILDASSET:
      dwCount = 0;
      break;
    case EMONEYTYPE_MANUALSKILL:
      dwCount = pUser->getManual().getSkillPoint();
      break;
    case EMONEYTYPE_PVPCOIN:
      dwCount = pUser->getUserSceneData().getPvpCoin();
      break;
    case EMONEYTYPE_LOTTERY:
      dwCount = pUser->getUserSceneData().getLotteryCoin();
      break;
    case EMONEYTYPE_GUILDHONOR:
      dwCount = pUser->getUserSceneData().getGuildHonor();
      break;
    case EMONEYTYPE_DEADCOIN:
      dwCount = pUser->getUserSceneData().getDeadCoin();
      break;
    case EMONEYTYPE_MAX:
      break;
  }

  subGroup = m_dwCount > dwCount ? m_dwFailJump : m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step : option
OptionStep::OptionStep(DWORD questid) : BaseStep(questid)
{

}

OptionStep::~OptionStep()
{

}

bool OptionStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwLayer = param.getTableInt("layer");
  return true;
}

bool OptionStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (m_dwLayer != 0 && pUser->getTower().getMaxLayer() < m_dwLayer)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - check option
CheckOptionStep::CheckOptionStep(DWORD questid) : BaseStep(questid)
{

}

CheckOptionStep::~CheckOptionStep()
{

}

bool CheckOptionStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  if (param.has("savemap") == true)
  {
    m_dwSaveMap = param.getTableInt("savemap");
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(m_dwSaveMap);
    if (pBase == nullptr)
    {
      if (MapConfig::getMe().isUnopenMap(m_dwSaveMap) == false)
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "savemap :" << m_dwSaveMap << "在Table_Map.txt表中找到" << XEND;
        return false;
      }
      else
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "savemap :" << m_dwSaveMap << "在Table_Map.txt表中未开放, 请相关策划注意" << XEND;
      }
    }
  }

  if (param.has("activemap") == true)
  {
    m_dwActiveMap = param.getTableInt("activemap");
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(m_dwActiveMap);
    if (pBase == nullptr)
    {
      if (MapConfig::getMe().isUnopenMap(m_dwSaveMap) == false)
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "activemap :" << m_dwActiveMap << "未在Table_Map.txt表中找到" << XEND;
        return false;
      }
      else
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "activemap :" << m_dwActiveMap << "在Table_Map.txt表中未开放,请相关策划注意" << XEND;
      }
    }
  }

  if (param.has("partner") == true)
  {
    m_dwPartnerID = param.getTableInt("partner");
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwPartnerID);
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "partner :" << m_dwPartnerID << "未在Table_Npc.txt表中找到" << XEND;
      return false;
    }
  }

  if (param.has("hand") == true)
  {
    DWORD dwHand = param.getTableInt("hand");
    if (dwHand <= EHANDSTATUS_MIN || dwHand >= EHANDSTATUS_MAX)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "hand :" << dwHand << "值不合法" << XEND;
      return false;
    }
    m_eHandStatus = static_cast<EHandStatus>(dwHand);
  }

  if (param.has("manual") == true)
  {
    xLuaData& manual = param.getMutableData("manual");

    m_dwManualMonsterID = manual.getTableInt("monster");
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwManualMonsterID);
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "manualmonster :" << m_dwManualMonsterID << "未在Table_Npc.txt表中找到" << XEND;
      return false;
    }

    DWORD dwStatus = manual.getTableInt("status");
    if (dwStatus <= EMANUALSTATUS_MIN || dwStatus >= EMANUALSTATUS_MAX || EManualStatus_IsValid(dwStatus) == false)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "manualstatus :" << dwStatus << "不合法" << XEND;
      return false;
    }
    m_eMonsterStatus = static_cast<EManualStatus>(dwStatus);
    m_dwMonsterUnlock = manual.getTableInt("lock");
  }

  if (param.has("var") == true)
  {
    xLuaData& var = param.getMutableData("var");
    DWORD dwVarType = var.getTableInt("type");
    if (dwVarType <= EVARTYPE_MIN || dwVarType >= EVARTYPE_MAX || EVarType_IsValid(dwVarType) == false)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "var :" << dwVarType << "不合法" << XEND;
      return false;
    }
    m_eVarType = static_cast<EVarType>(dwVarType);
    m_dwVarValue = var.getTableInt("value");
  }

  if (param.has("monthcard") == true)
    m_bHasMonthCard = param.getTableInt("monthcard") == 1;

  m_bGenderCheck = param.has("gender_check");

  m_setMarital.clear();
  if (param.has("marital") == true)
  {
    auto maritalf = [&](const string& key, xLuaData& data)
    {
      DWORD dwMarital = data.getInt();
      if (EMARITAL_IsValid(dwMarital) == false)
      {
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "marital :" << dwMarital << "不合法" << XEND;
        return false;
      }
      m_setMarital.insert(static_cast<EMARITAL>(dwMarital));
      return true;
    };
    param.getMutableData("marital").foreach(maritalf);
  }

  return true;
}

bool CheckOptionStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  bool bSuccess = true;

  do
  {
    if (m_dwSaveMap != 0)
    {
      bSuccess = pUser->getUserSceneData().getSaveMap() == m_dwSaveMap;
      if (!bSuccess)
        break;
    }

    if (m_dwActiveMap != 0)
    {
      bSuccess = pUser->getManual().getMapStatus(m_dwActiveMap) == EMANUALSTATUS_UNLOCK;
      if (!bSuccess)
        break;
    }

    if (m_dwPartnerID != 0)
    {
      bSuccess = pUser->getPet().getPartnerID() == m_dwPartnerID;
      if (!bSuccess)
        break;
    }

    if (m_eHandStatus != EHANDSTATUS_MIN)
    {
      if (m_eHandStatus == EHANDSTATUS_HAND)
        bSuccess = pUser->m_oHands.has();
      else if (m_eHandStatus == EHANDSTATUS_NOHAND)
        bSuccess = !pUser->m_oHands.has();
      if (!bSuccess)
        break;
    }

    if (m_dwManualMonsterID != 0)
    {
      bSuccess = pUser->getManual().getMonsterStatus(m_dwManualMonsterID) >= m_eMonsterStatus;
      if (bSuccess && m_dwMonsterUnlock != 0)
      {
        bool bLock = pUser->getManual().getMonsterLock(m_dwManualMonsterID);
        if (bLock && m_dwMonsterUnlock != 1)
          bSuccess = false;
        else if (!bLock && m_dwMonsterUnlock != 2)
          bSuccess = false;
      }
      if (!bSuccess)
        break;
    }

    if (m_eVarType != EVARTYPE_MIN)
    {
      bSuccess = pUser->getVar().getVarValue(m_eVarType) == m_dwVarValue;
      if (!bSuccess)
        break;
    }

    if (m_bHasMonthCard)
    {
      bSuccess = pUser->getDeposit().hasMonthCard();
      if (!bSuccess)
        break;
    }

    if (m_bGenderCheck)
    {
      bSuccess = pUser->getUserSceneData().getGender() == EGENDER_MALE;
      if (!bSuccess)
        break;
    }

    if (m_setMarital.empty() == false)
    {
      bSuccess = m_setMarital.find(pUser->getUserWedding().getMaritalState()) != m_setMarital.end();
      if (!bSuccess)
        break;
    }
  } while (0);

  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - hint
HintStep::HintStep(DWORD questid) : BaseStep(questid)
{


}

HintStep::~HintStep()
{

}

bool HintStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwMsgID = param.getTableInt("msgid");

  m_dwMapID = param.getTableInt("mapid");
  if (m_dwMapID != 0)
  {
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(m_dwMapID);
    if (pBase == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "mapid :" << m_dwMapID << "值不合法" << XEND;
      return false;
    }
  }

  m_strParam = param.getTableString("param");
  return true;
}

bool HintStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  MsgParams oParams;

  if (m_dwMapID != 0)
  {
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(m_dwMapID);
    if (pBase != nullptr)
      oParams.addString(pBase->strNameZh);
  }
  if (m_strParam.empty() == false)
    oParams.addString(m_strParam);

  MsgManager::sendMsg(pUser->id, m_dwMsgID, oParams);
  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - seal
SealStep::SealStep(DWORD questid) : BaseStep(questid)
{

}

SealStep::~SealStep()
{

}

bool SealStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwSealID = param.getTableInt("id");
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(m_dwSealID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id :" << m_dwSealID << "值不合法" << XEND;
    return false;
  }

  return true;
}

bool SealStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;
  if (pStep->process() == 0)
  {
    // begin seal this
    pUser->getSeal().addSeal(m_dwSealID);
    pStep->set_process(1);
    return false;
  }

  if (param1 != m_dwSealID)
    return false;

  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - str
EquipLvStep::EquipLvStep(DWORD questid) : BaseStep(questid)
{

}

EquipLvStep::~EquipLvStep()
{

}

bool EquipLvStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwTypeID = param.getTableInt("id");
  m_pairLv.first = param.getTableInt("min");
  m_pairLv.second = param.getTableInt("max");

  m_bEquip = param.getTableInt("equip") == 0;
  m_bStoreage = param.getTableInt("storage") == 1;

  if (m_dwTypeID != 0)
  {
    if (ItemConfig::getMe().getItemCFG(m_dwTypeID) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id :" << m_dwTypeID << "未在Table_Item.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool EquipLvStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  auto check = [&](const EPackType eType)
  {
    BasePackage* pPackage = pUser->getPackage().getPackage(eType);
    if (pPackage == nullptr)
      return false;
    if (sparam1.empty() == false)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pPackage->getItem(sparam1));
      if (pEquip == nullptr)
        return false;
      if (m_dwTypeID != 0 && pEquip->getTypeID() != m_dwTypeID)
        return false;
      if (pEquip->getStrengthLv() < m_pairLv.first || pEquip->getStrengthLv() > m_pairLv.second)
        return false;
    }
    else
    {
      if (pPackage->hasItem(m_dwTypeID, m_pairLv.first, m_pairLv.second) == false)
        return false;
    }
    return true;
  };

  bool bSuccess = check(EPACKTYPE_MAIN);

  if (m_bEquip)
    bSuccess = check(EPACKTYPE_EQUIP);

  //if (m_bStoreage)
  //  bSuccess = check(EPACKTYPE_STORE);

  if (!bSuccess)
    return false;

  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - video
VideoStep::VideoStep(DWORD questid) : BaseStep(questid)
{

}

VideoStep::~VideoStep()
{

}

bool VideoStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  return true;
}

bool VideoStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - illustration
IllustrationStep::IllustrationStep(DWORD questid) : BaseStep(questid)
{

}

IllustrationStep::~IllustrationStep()
{

}

bool IllustrationStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  return true;
}

bool IllustrationStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - npcplay
NpcPlayStep::NpcPlayStep(DWORD questid) : BaseStep(questid)
{

}

NpcPlayStep::~NpcPlayStep()
{

}

bool NpcPlayStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  m_oGMCmd = data.getMutableData("Params");
  m_dwQuestNpcID = data.getData("Params").getTableInt("questnpc");
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwQuestNpcID);
  if (pCFG == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "questnpc :" << m_dwQuestNpcID << "未在Table_Npc.txt表中找到" << XEND;
    return false;
  }
  return true;
}

bool NpcPlayStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  std::set<SceneNpc*> npcset;
  pUser->getQuestNpc().getCurMapNpc(npcset);
  for (auto &pNpc : npcset)
  {
    if (pNpc == nullptr || pNpc->getScene() == nullptr || pNpc->getNpcID() != m_dwQuestNpcID)
      continue;
    GMCommandRuler::getMe().execute(pNpc, m_oGMCmd);
  }
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - item
ItemStep::ItemStep(DWORD questid) : BaseStep(questid)
{

}

ItemStep::~ItemStep()
{

}

bool ItemStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  xLuaData& item = param.getMutableData("item");
  auto itemf = [this](const string& str, xLuaData& data)
  {
    ItemInfo oItem;
    oItem.set_id(data.getTableInt("id"));
    oItem.set_count(data.getTableInt("num"));

    combinItemInfo(m_vecItemInfo, TVecItemInfo{oItem});
  };
  item.foreach(itemf);

  m_bEquip = param.getTableInt("equip") == 1;
  m_bTemp = param.getTableInt("temp") == 1;

  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
  {
    if (ItemManager::getMe().getItemCFG(v->id()) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id :" << v->id() << "未在Table_Item.txt表中找到" << XEND;
      return false;
    }
  }

  return true;
}

bool ItemStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  Package& rPackage = pUser->getPackage();
  TVecItemInfo vecItem;
  static const set<EPackType> setTypes = set<EPackType>{EPACKTYPE_MAIN, EPACKTYPE_QUEST, EPACKTYPE_FOOD, EPACKTYPE_PET};
  for (auto &s : setTypes)
  {
    if (collectItem(vecItem, rPackage.getPackage(s)) == false)
      return false;
  }

  if (m_bEquip && collectItem(vecItem, rPackage.getPackage(EPACKTYPE_EQUIP)) == false)
    return false;
  if (m_bTemp && collectItem(vecItem, rPackage.getPackage(EPACKTYPE_TEMP_MAIN)) == false)
    return false;

  bool bSuccess = true;
  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
  {
    DWORD dwTypeID = v->id();
    auto o = find_if(vecItem.begin(), vecItem.end(), [dwTypeID](const ItemInfo& rItem) -> bool{
      return dwTypeID == rItem.id();
    });
    if (o == vecItem.end() || o->count() < v->count())
    {
      bSuccess = false;
      break;
    }
  }

  if (!bSuccess)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

bool ItemStep::collectItem(TVecItemInfo& vecItem, BasePackage* pPack)
{
  if (pPack == nullptr)
    return false;

  for (auto v = m_vecItemInfo.begin(); v != m_vecItemInfo.end(); ++v)
  {
    ItemInfo oItem;
    oItem.set_id(v->id());
    oItem.set_count(pPack->getItemCount(v->id()));
    combinItemInfo(vecItem, oItem);
  }

  return true;
}

// quest step - daily
DailyStep::DailyStep(DWORD questid) : BaseStep(questid)
{

}

DailyStep::~DailyStep()
{

}

bool DailyStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  return true;
}

bool DailyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  Quest& rQuest = pUser->getQuest();
  if (rQuest.getDailyExp() > 0)
    return false;

  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - check manual
CheckManualStep::CheckManualStep(DWORD questid) : BaseStep(questid)
{

}

CheckManualStep::~CheckManualStep()
{

}

bool CheckManualStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;
  xLuaData& param = data.getMutableData("Params");

  DWORD manualtype = param.getTableInt("manual_type");
  if (manualtype <= EMANUALTYPE_MIN || manualtype >= EMANUALTYPE_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "manual_type :" << manualtype << "不合法" << XEND;
    return false;
  }
  eType = static_cast<EManualType> (manualtype);

  DWORD status = param.getTableInt("status");
  if (status <= EMANUALSTATUS_MIN || status >= EMANUALSTATUS_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "status :" << status << "不合法" << XEND;
    return false;
  }
  eStatus = static_cast<EManualStatus> (status);

  DWORD npctype = param.getTableInt("monster_type");
  if (npctype != 0)
  {
    if (npctype <= ENPCTYPE_MIN && npctype >= ENPCTYPE_MAX)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "monster_type :" << npctype << "不合法" << XEND;
      return false;
    }
    eMonsterType = static_cast<ENpcType> (npctype);
  }
  dwNeedNum = param.getTableInt("num");
  return true;
}

bool CheckManualStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  bool bSuccess = pUser->getManual().getNumByStatus(eType, eStatus, eMonsterType) >= dwNeedNum;
  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - manual
ManualStep::ManualStep(DWORD questid) : BaseStep(questid)
{

}

ManualStep::~ManualStep()
{

}

bool ManualStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;
  xLuaData& param = data.getMutableData("Params");

  DWORD manualtype = param.getTableInt("manual_type");
  if (manualtype <= EMANUALTYPE_MIN || manualtype >= EMANUALTYPE_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "manual_type :" << manualtype << "不合法" << XEND;
    return false;
  }
  eType = static_cast<EManualType> (manualtype);

  DWORD status = param.getTableInt("status");
  if (status <= EMANUALSTATUS_MIN || status >= EMANUALSTATUS_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "status :" << status << "不合法" << XEND;
    return false;
  }
  eStatus = static_cast<EManualStatus> (status);

  DWORD npctype = param.getTableInt("monster_type");
  if (npctype != 0)
  {
    if (npctype <= ENPCTYPE_MIN && npctype >= ENPCTYPE_MAX)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "monster_type :" << npctype << "不合法" << XEND;
      return false;
    }
    eMonsterType = static_cast<ENpcType> (npctype);
  }
  dwNeedNum = param.getTableInt("num");

  m_dwID = param.getTableInt("id");
  if (m_dwID != 0 && ItemConfig::getMe().getItemCFG(m_dwID) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "id:" << m_dwID << "未在 Table_Item.txt 表中找到" << XEND;
    return false;
  }

  return true;
}

bool ManualStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (eType == EMANUALTYPE_MONSTER)
  {
    DWORD nowNum = pUser->getManual().getNumByStatus(eType, eStatus, eMonsterType);

    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return false;

    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return false;

    pStep->set_process(nowNum);

    if (nowNum < dwNeedNum)
    {
      BaseStep::stepUpdate(pUser);
      return false;
    }
  }
  else
  {
    EManualStatus status = eType == EMANUALTYPE_COLLECTION ? pUser->getManual().getCollectionStatus(m_dwID) : pUser->getManual().getItemStatus(m_dwID);
    if (status < eStatus)
      return false;
  }

  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - playmusic
PlayMusicStep::PlayMusicStep(DWORD questid) : BaseStep(questid), m_dwMusicID(0), m_dwNum(0)
{
}

PlayMusicStep::~PlayMusicStep()
{
}

bool PlayMusicStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwMusicID = param.getTableInt("musicId");
  m_dwNum = param.getTableInt("num");

  bool bCorrect = true;

  if (m_dwMusicID != 0)
  {
    const SMusicBase* pMusicBase = TableManager::getMe().getMusicCFG(m_dwMusicID);
    if (pMusicBase == nullptr)
      return false;
    if (pMusicBase == nullptr)
    {
      bCorrect = false;
    }
  }
  else
    bCorrect = false;

  if (!bCorrect)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "musicid :" << m_dwMusicID << "未在Table_MusicBox.txt表中找到" << XEND;
    return false;
  }  
  return true;
}

bool PlayMusicStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (param1 != m_dwMusicID)
    return false;

  const SMusicBase* pMusicBase = TableManager::getMe().getMusicCFG(param1);
  if (pMusicBase == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;
  pStep->set_process(pStep->process() + 1);

  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

RewardHelpStep::RewardHelpStep(DWORD questid) : BaseStep(questid)
{

}

RewardHelpStep::~RewardHelpStep()
{

}


bool RewardHelpStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  dwGuildRewardCnt = param.getTableInt("guild");
  dwFriendRewardCnt = param.getTableInt("friend");

  return true;
}

bool RewardHelpStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  sendHelpReward(pUser);
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

void RewardHelpStep::sendHelpReward(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  std::set<SceneUser*> helpSet;
  helpSet.insert(pUser);
  std::set<SceneUser*> otherSet;

  const GTeam& rTeam = pUser->getTeam();
  for (auto &m : rTeam.getTeamMemberList())
  {
    const TeamMemberInfo& info = m.second;
    if (info.charid() == pUser->id)
      continue;
    SceneUser* user = SceneUserManager::getMe().getUserByID(info.charid());
    if (user == nullptr || user->check2PosInNine(pUser) == false)
      continue;
    otherSet.insert(user);
    //if (user->getQuest().hasAcceptQuest(m_dwQuestID / QUEST_ID_PARAM))
      //continue;
    bool bSelf = user->getQuest().hasAcceptQuest(m_dwQuestID / QUEST_ID_PARAM);
    if (dwFriendRewardCnt != 0 || dwGuildRewardCnt != 0)
      user->getHelpReward(helpSet, EHELPTYPE_QUESTREWARD, bSelf, dwFriendRewardCnt, dwGuildRewardCnt);
  }
  pUser->getHelpReward(otherSet, EHELPTYPE_QUESTREWARD, true, dwFriendRewardCnt, 0);
}

// money
MoneyStep::MoneyStep(DWORD questid) : BaseStep(questid)
{

}

MoneyStep::~MoneyStep()
{

}

bool MoneyStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  const string& method = param.getTableString("method");
  if (method == "check_wait")
    m_eMethod = EMONEYMETHOD_CHECK_WAIT;
  else if (method == "check_now")
    m_eMethod = EMONEYMETHOD_CHECK_NOW;
  else if (method == "sub")
    m_eMethod = EMONEYMETHOD_SUB;
  else
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "method:" << method << "不合法" << XEND;
    return false;
  }

  DWORD dwType = param.getTableInt("moneytype");
  if (dwType <= EMONEYTYPE_MIN || dwType >= EMONEYTYPE_MAX || EMoneyType_IsValid(dwType) == false)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "moneytype:" << dwType << "不合法" << XEND;
    return false;
  }
  m_eType = static_cast<EMoneyType>(dwType);

  m_dwNum = param.getTableInt("num");
  return true;
}

bool MoneyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  DWORD dwSubGroup = 0;
  if (m_eMethod == EMONEYMETHOD_CHECK_NOW)
  {
    dwSubGroup = pUser->checkMoney(m_eType, m_dwNum) ? m_dwFinishJump : m_dwFailJump;
  }
  else if (m_eMethod == EMONEYMETHOD_CHECK_WAIT)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return false;
    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return false;

    bool bSuccess = false;
    DWORD dwNum = 0;
    do
    {
      // 特殊处理,友情之证从背包扣除
      if (m_eType == EMONEYTYPE_FRIENDSHIP)
      {
        BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
        if (pMainPack == nullptr)
        {
          XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "moneytype:" << m_eType
            << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,获取" << EPACKTYPE_MAIN << "失败" << XEND;
          break;
        }
        dwNum = pMainPack->getItemCount(ITEM_FRIENDSHIP);
        if (dwNum < m_dwNum)
        {
          XLOG << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "moneytype:" << m_eType
            << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行成功,数量不足" << XEND;
          break;
        }
        bSuccess = true;
      }
      else
      {
        switch (m_eType)
        {
          case EMONEYTYPE_MIN:
          case EMONEYTYPE_MAX:
            break;
          case EMONEYTYPE_DIAMOND:
            dwNum = pUser->getUserSceneData().getDiamond();
            break;
          case EMONEYTYPE_SILVER:
            dwNum = pUser->getUserSceneData().getSilver();
            break;
          case EMONEYTYPE_GOLD:
            dwNum = pUser->getUserSceneData().getGold();
            break;
          case EMONEYTYPE_GARDEN:
            break;//m_oUserSceneData.getGarden() >= value;
          case EMONEYTYPE_MANUALSKILL:
            dwNum = pUser->getManual().getSkillPoint();
            break;
          case EMONEYTYPE_FRIENDSHIP:
            break;//m_oUserSceneData.getFriendShip() >= value;
          case EMONEYTYPE_CONTRIBUTE:
            dwNum = pUser->getGuild().contribute();
            break;
          case EMONEYTYPE_GUILDASSET:
            break;
          case EMONEYTYPE_PVPCOIN:
            dwNum = pUser->getUserSceneData().getPvpCoin();
            break;
          case EMONEYTYPE_LOTTERY:
            dwNum = pUser->getUserSceneData().getLotteryCoin();
            break;
          case EMONEYTYPE_GUILDHONOR:
            dwNum = pUser->getUserSceneData().getGuildHonor();
            break;
          case EMONEYTYPE_DEADCOIN:
            dwNum = pUser->getUserSceneData().getDeadCoin();
            break;
        }

        if (pUser->checkMoney(m_eType, m_dwNum) == false)
          break;
        bSuccess = true;
      }
    } while (0);

    if (!bSuccess)
    {
      if (pStep->process() != dwNum)
      {
        pStep->set_process(dwNum);
        BaseStep::stepUpdate(pUser);
      }
      return false;
    }

    dwSubGroup = m_dwFinishJump;
  }
  else if (m_eMethod == EMONEYMETHOD_SUB)
  {
    // 特殊处理,友情之证从背包扣除
    if (m_eType == EMONEYTYPE_FRIENDSHIP)
    {
      do
      {
        BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
        if (pMainPack == nullptr)
        {
          XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "moneytype:" << m_eType
            << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,获取" << EPACKTYPE_MAIN << "失败" << XEND;
          dwSubGroup = m_dwFailJump;
          break;
        }
        if (pMainPack->getItemCount(ITEM_FRIENDSHIP) < m_dwNum)
        {
          XLOG << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "moneytype:" << m_eType
            << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行成功,数量不足" << XEND;
          dwSubGroup = m_dwFailJump;
          break;
        }
        pMainPack->reduceItem(ITEM_FRIENDSHIP, ESOURCE_QUEST, m_dwNum);
        dwSubGroup = m_dwFinishJump;
      }while (0);
    }
    else
    {
      dwSubGroup = pUser->subMoney(m_eType, m_dwNum, ESOURCE_QUEST) ? m_dwFinishJump : m_dwFailJump;
    }
  }

  return BaseStep::doStep(pUser, dwSubGroup, param1, param2, sparam1);
}

// cat
CatStep::CatStep(DWORD questid) : BaseStep(questid)
{

}

CatStep::~CatStep()
{

}

bool CatStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  const string& method = param.getTableString("method");
  if (method == "check_money")
  {
    m_eMethod = ECATMETHOD_CHECK_MONEY;
  }
  else if (method == "check_hire")
  {
    m_eMethod = ECATMETHOD_CHECK_HIRE;
  }
  else if (method == "check_max")
  {
    m_eMethod = ECATMETHOD_CHECK_MAX;
  }
  else if (method == "hire")
  {
    m_eMethod = ECATMETHOD_HIRE;
  }
  else
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "method:" << method << "不合法" << XEND;
    return false;
  }

  m_dwCatID = param.getTableInt("id");
  if (WeaponPetConfig::getMe().getCFG(m_dwCatID) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "id:" << m_dwCatID << "未在 Table_MercenaryCat.txt 表中找到" << XEND;
    return false;
  }
  DWORD dwType = static_cast<EEmployType>(param.getTableInt("type"));
  if (dwType <= EEMPLOYTYPE_MIN || dwType >= EEMPLOYTYPE_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "type:" << dwType << "不合法" << XEND;
    return false;
  }
  m_eType = static_cast<EEmployType>(dwType);
  // check

  return true;
}

bool CatStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (m_eMethod == ECATMETHOD_CHECK_MONEY)
  {
    do
    {
      bool bZeny = true;
      DWORD dwZenyCount = 0;
      DWORD dwCostItemId = 0;
      DWORD dwCostItemNum = 0;
      if (pUser->getWeaponPet().checkAddMoney(m_dwCatID, m_eType, bZeny, dwZenyCount, dwCostItemId, dwCostItemNum) == false)
      {
        subGroup = m_dwFailJump;
        break;
      }
      subGroup = m_dwFinishJump;
    } while(0);
  }
  else if (m_eMethod == ECATMETHOD_CHECK_HIRE)
    subGroup = pUser->getWeaponPet().checkCanAdd(m_dwCatID) == false ? m_dwFailJump : m_dwFinishJump;
  else if (m_eMethod == ECATMETHOD_CHECK_MAX)
    subGroup = m_dwFinishJump;
    //subGroup = pUser->getWeaponPet().checkSize() == false ? m_dwFailJump : m_dwFinishJump;
  else if (m_eMethod == ECATMETHOD_HIRE)
    subGroup = pUser->getWeaponPet().add(m_dwCatID, m_eType);

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// activity
ActivityStep::ActivityStep(DWORD questid) : BaseStep(questid)
{

}

ActivityStep::~ActivityStep()
{

}

bool ActivityStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  auto idf = [&](const string& key, xLuaData& data)
  {
    m_setManualIDs.insert(data.getInt());
  };
  param.getMutableData("ids").foreach(idf);

  m_dwNum = param.getTableInt("num");
  m_bPhoto = param.getTableInt("photo") == 1;

  DWORD dwType = param.getTableInt("type");
  if (dwType <= EMANUALTYPE_MIN || dwType >= EMANUALTYPE_MAX || EManualType_IsValid(dwType) == false)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "type:" << dwType << "不是合法的冒险类型" << XEND;
    return false;
  }
  m_eType = static_cast<EManualType>(dwType);

  DWORD dwStatus = param.getTableInt("status");
  if (EManualStatus_IsValid(dwStatus) == false)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "status:" << dwStatus << "不是合法的冒险状态" << XEND;
    return false;
  }
  m_eStatus = static_cast<EManualStatus>(dwStatus);

  const string& method = param.getTableString("method");
  if (method == "manual_full")
  {
    m_eMethod = EACTMETHOD_MANUAL_FULL;
  }
  else if (method == "manual_num")
  {
    m_eMethod = EACTMETHOD_MANUAL_NUM;
  }
  else
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "method:" << method << "不是合法的类型" << XEND;
    return false;
  }

  // check
  if (m_eType == EMANUALTYPE_MONSTER)
  {
    bool bSuccess = true;
    for (auto &s : m_setManualIDs)
    {
      if (NpcConfig::getMe().getNpcCFG(s) == nullptr)
      {
        bSuccess = false;
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "id:" << s << "未在 Table_Npc.txt or Table_Monster.txt 表中找到" << XEND;
      }
    }
    if (!bSuccess)
      return false;
  }
  else if (m_eType == EMANUALTYPE_CARD || m_eType == EMANUALTYPE_ITEM || m_eType == EMANUALTYPE_EQUIP)
  {
    bool bSuccess = true;
    for (auto &s : m_setManualIDs)
    {
      if (ItemConfig::getMe().getItemCFG(s) == nullptr)
      {
        bSuccess = false;
        XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "id:" << s << "未在 Table_Item.txt 表中找到" << XEND;
      }
    }
    if (!bSuccess)
      return false;
  }

  return true;
}

bool ActivityStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  SManualItem* pItem = pUser->getManual().getManualItem(m_eType);
  if (pItem == nullptr)
    return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);

  bool bSuccess = true;
  if (m_eMethod == EACTMETHOD_MANUAL_FULL)
  {
    for (auto &s : m_setManualIDs)
    {
      SManualSubItem* pSubItem = pItem->getSubItem(s);
      if (pSubItem == nullptr)
      {
        bSuccess = false;
        break;
      }
      if (pSubItem->eStatus < m_eStatus)
      {
        bSuccess = false;
        break;
      }
      if (m_bPhoto && !pSubItem->bUnlock)
      {
        bSuccess = false;
        break;
      }
    }
  }
  else if (m_eMethod == EACTMETHOD_MANUAL_NUM)
  {
    DWORD dwCount = 0;
    for (auto &s : m_setManualIDs)
    {
      SManualSubItem* pSubItem = pItem->getSubItem(s);
      if (pSubItem == nullptr)
        continue;
      if (pSubItem->eStatus < m_eStatus)
        continue;
      if (m_bPhoto && !pSubItem->bUnlock)
        continue;
      ++dwCount;
    }
    bSuccess = dwCount >= m_dwNum;
  }
  else
  {
    bSuccess = false;
  }

  return BaseStep::doStep(pUser, bSuccess ? m_dwFinishJump : m_dwFailJump, param1, param2, sparam1);
}

// photo
PhotoStep::PhotoStep(DWORD questid) : BaseStep(questid)
{

}

PhotoStep::~PhotoStep()
{

}

bool PhotoStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwNum = param.getTableInt("num");
  m_bFocus = param.getTableInt("focus") != 0;
  m_bGuild = param.getTableInt("guild") != 0;
  m_bGroup = param.getTableInt("group") != 0;
  if (m_bGroup)
    m_bSelf = param.getTableInt("self") != 0;
  if (m_bPos)
    m_dwRange = param.getTableInt("range");

  DWORD dwStatus = param.getTableInt("status");
  if (dwStatus != 0)
  {
    if (dwStatus <= ECREATURESTATUS_MIN || dwStatus >= ECREATURESTATUS_MAX || ECreatureStatus_IsValid(dwStatus) == false)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "status :" << dwStatus << "不合法" << XEND;
      return false;
    }
    m_eStatus = static_cast<ECreatureStatus>(dwStatus);
  }

  m_dwActionID = param.getTableInt("action");
  if (m_dwActionID != 0)
  {
    const SActionAnimBase* pCFG = TableManager::getMe().getActionAnimCFG(m_dwActionID);
    if (pCFG == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "action :" << m_dwActionID << "未在 Table_ActionAnime.txt 表中找到" << XEND;
      return false;
    }
  }

  m_dwCreatureID = param.getTableInt("creature_id");
  if (m_dwCreatureID != 0)
  {
    if (NpcConfig::getMe().getNpcCFG(m_dwCreatureID) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "creature_id:" << m_dwCreatureID << "未在 Table_Npc.txt or Table_Monster.txt 表中找到" << XEND;
      return false;
    }
  }
  return true;
}

void PhotoStep::reset(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  if (m_bGuild && m_bFocus)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return;
    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return;
    const GuildSMember* pTarget = pUser->getGuild().randMember();//TSetQWORD{pUser->id});
    if (pTarget == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机公会成员失败" << XEND;
      return;
    }
    pStep->add_names(pTarget->name());
    BaseStep::stepUpdate(pUser);
  }

  BaseStep::reset(pUser);
}

bool PhotoStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (m_dwMapID != 0 && pUser->getScene()->getMapID() != m_dwMapID)
  {
    XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,不在指定地图" << XEND;
    return false;
  }
  if (m_bPos)
  {
    float fDist = ::getXZDistance(pUser->getPos(), m_oDestPos);
    if (fDist > m_dwRange)
    {
      XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,距离指定地点过远" << XEND;
      return false;
    }
  }

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;

  const TSetTmpUser setUser = pUser->getQuest().getTmpUser();
  const TSetTmpNpc& setNpc = pUser->getQuest().getTmpNpc();
  if (setUser.empty() == true && setNpc.empty() == true)
    return false;

  DWORD dwProcess = 0;
  if (m_bGuild)
  {
    if (pUser->hasGuild() == false)
    {
      XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,自己不是公会成员" << XEND;
      return false;
    }
    auto s = find_if(setUser.begin(), setUser.end(), [&](SceneUser* p) -> bool{
      if (m_bGroup && !m_bSelf && p->id == pUser->id)
        return false;
      if (p->hasGuild() == false)
        return false;
      if (p->getGuild().id() != pUser->getGuild().id())
        return false;
      if (m_eStatus != ECREATURESTATUS_MIN && p->getStatus() != m_eStatus)
        return false;
      if (m_dwActionID != 0 && p->getAction() != m_dwActionID)
        return false;
      if (m_dwMapID != 0 && (p->getScene() == nullptr || p->getScene()->getMapID() != m_dwMapID))
        return false;
      if (m_bPos)
      {
        float fDist = ::getXZDistance(p->getPos(), m_oDestPos);
        if (fDist > m_dwRange)
        {
          XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,距离指定地点过远" << XEND;
          return false;
        }
      }
      dwProcess += 1;
      return true;
    });
    if (s == setUser.end())
    {
      XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,未含有符合成员" << XEND;
      return false;
    }
  }
  if (m_bFocus)
  {
    auto s = find_if(setUser.begin(), setUser.end(), [&](const SceneUser* p) -> bool{
      if (p->hasGuild() == false)
        return false;
      string name = p->name;
      for (int i = 0; i < pStep->names_size(); ++i)
      {
        if (name == pStep->names(i))
          return true;
      }
      return false;
    });
    if (s == setUser.end())
    {
      XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,不是指定玩家 :";
      for (int i = 0; i < pStep->names_size(); ++i)
        XDBG << pStep->names(i);
      XDBG << XEND;
      return false;
    }
  }
  if (m_dwCreatureID != 0)
  {
    auto s = find_if(setNpc.begin(), setNpc.end(), [&](const SceneNpc* p) -> bool{
      return p != nullptr && p->getNpcID() == m_dwCreatureID;
    });
    if (s == setNpc.end())
    {
      XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,不是指定NPC :" << m_dwCreatureID << XEND;
      return false;
    }
  }

  dwProcess = m_bGroup == true ? dwProcess : 1;
  pStep->set_process(pStep->process() + dwProcess);
  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  bool ret = BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  if (ret)
  {
    // 看板队长
    set<SceneUser*> uSet;
    pUser->getQuest().getWantedQuestLeaderTeammate(m_dwQuestID, uSet);
    if (!uSet.empty())
    {
      TSetQWORD setGuid;
      for (auto &v : setUser)
        setGuid.insert(v->id);
      for (auto &v : setNpc)
        setGuid.insert(v->getTempID());
      for (auto &u : uSet)
      {
        u->getQuest().onPhoto(setGuid);
        MsgManager::sendMsg(u->id, 4014);
      }
    }
  }
  return ret;
}

// item use
ItemUseStep::ItemUseStep(DWORD questid) : BaseStep(questid)
{

}

ItemUseStep::~ItemUseStep()
{

}

bool ItemUseStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_oItem.set_id(param.getTableInt("id"));
  if (ItemConfig::getMe().getItemCFG(m_oItem.id()) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "id:" << m_oItem.id() << "未在 Table_Item.txt 表中找到" << XEND;
    return false;
  }

  xLuaData& mapdata = param.getMutableData("map");
  auto mapfunc = [&](const string& key, xLuaData& data)
  {
    m_setAvailableMap.emplace(data.getInt());
  };
  mapdata.foreach(mapfunc);

  if(param.has("itemtype"))
    eItemType = static_cast<EItemType>(param.getTableInt("itemtype"));

  m_dwNum = param.getTableInt("num");
  m_bFocus = param.getTableInt("focus") != 0;
  m_bGuild = param.getTableInt("guild") != 0;
  return true;
}

void ItemUseStep::reset(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  BaseStep::reset(pUser);

  if (m_bGuild && m_bFocus)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return;
    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return;
    const GuildSMember* pTarget = pUser->getGuild().randMember();//TSetQWORD{pUser->id});
    if (pTarget == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机公会成员失败" << XEND;
      return;
    }
    pStep->add_names(pTarget->name());
    BaseStep::stepUpdate(pUser);
  }
}

bool ItemUseStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (m_oItem.id() != 0 && param1 != m_oItem.id())
    return false;

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(param1);
  if(pCFG == nullptr)
    return false;
  if(eItemType != EITEMTYPE_MIN && pCFG->eItemType != eItemType)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;

  if(m_setAvailableMap.empty() == false && m_setAvailableMap.find(pUser->getScene()->getMapID()) == m_setAvailableMap.end())
    return false;

  const TSetTmpUser setUser = pUser->getQuest().getTmpUser();
  const TSetTmpNpc& setNpc = pUser->getQuest().getTmpNpc();
  if (setUser.empty() == true && setNpc.empty() == true)
    return false;

  SceneUser* pTargetUser = *setUser.begin();
  if (pTargetUser == nullptr)
    return false;

  if (m_bGuild)
  {
    if (pTargetUser->hasGuild() == false || pUser->hasGuild() == false)
      return false;
    if (pTargetUser->getGuild().id() != pUser->getGuild().id())
      return false;
  }
  if (m_bFocus)
  {
    bool bFind = false;
    string targetname = pTargetUser->name;
    for (int i = 0; i < pStep->names_size(); ++i)
    {
      if (pStep->names(i) == targetname)
      {
        bFind = true;
        break;
      }
    }
    if (!bFind)
      return false;
  }

  for (int i = 0; i < pStep->params_size(); ++i)
  {
    if (pTargetUser->id == pStep->params(i))
      return false;
  }
  pStep->set_process(pStep->process() + 1);
  pStep->add_params(pTargetUser->id);
  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// hand
HandStep::HandStep(DWORD questid) : BaseStep(questid)
{

}

HandStep::~HandStep()
{

}

bool HandStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwTime = param.getTableInt("time");
  m_bFocus = param.getTableInt("focus") != 0;
  m_bGuild = param.getTableInt("guild") != 0;
  return true;
}

void HandStep::reset(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  BaseStep::reset(pUser);

  if (m_bGuild && m_bFocus)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return;
    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return;
    const GuildSMember* pTarget = pUser->getGuild().randMember();//TSetQWORD{pUser->id});
    if (pTarget == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机公会成员失败" << XEND;
      return;
    }
    pStep->add_names(pTarget->name());
    BaseStep::stepUpdate(pUser);
  }
}

bool HandStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;

  SceneUser* pTargetUser = SceneUserManager::getMe().getUserByID(param1);
  if (pTargetUser == nullptr)
    return false;

  if (m_bGuild)
  {
    if (pTargetUser->hasGuild() == false)
      return false;
  }
  if (m_bFocus)
  {
    bool bFind = false;
    string targetname = pTargetUser->name;
    for (int i = 0; i < pStep->names_size(); ++i)
    {
      if (pStep->names(i) == targetname)
      {
        bFind = true;
        break;
      }
    }
    if (!bFind)
      return false;
  }
  if (param2 < m_dwTime)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// music
MusicStep::MusicStep(DWORD questid) : BaseStep(questid)
{

}

MusicStep::~MusicStep()
{

}

bool MusicStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwTime = param.getTableInt("time");
  m_dwNum = param.getTableInt("num");
  m_bFocus = param.getTableInt("focus") != 0;
  m_bGuild = param.getTableInt("guild") != 0;
  m_bTeam = param.getTableInt("team") != 0;
  return true;
}

void MusicStep::reset(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  BaseStep::reset(pUser);

  if (m_bGuild && m_bFocus)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return;
    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return;
    const GuildSMember* pTarget = pUser->getGuild().randMember();//TSetQWORD{pUser->id});
    if (pTarget == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机公会成员失败" << XEND;
      return;
    }
    pStep->add_params(pTarget->charid());
    pStep->add_names(pTarget->name());
    BaseStep::stepUpdate(pUser);
    XDBG << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机公会成员" << pTarget->charid() << pTarget->name() << XEND;
  }
}

bool MusicStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;

  GTeam& rTeam = pUser->getTeam();
  GGuild& rGuild = pUser->getGuild();

  TSetQWORD setIDs;
  if (m_bTeam && rTeam.getTeamID() != 0)
  {
    const TMapGTeamMember& mapMember = rTeam.getTeamMemberList();
    for (auto &m : mapMember)
      setIDs.insert(m.first);
  }
  if (m_bGuild && pUser->hasGuild() == true)
  {
    const TMapGGuildMember& mapMember = rGuild.getMemberList();
    for (auto &m : mapMember)
    {
      if (m.second.onlinetime() >= m.second.offlinetime())
        setIDs.insert(m.first);
    }
  }
  if (m_bFocus)
  {
    auto s = find_if(setIDs.begin(), setIDs.end(), [&](QWORD id) -> bool{
      for (int i = 0; i < pStep->params_size(); ++i)
      {
        if (id == pStep->params(i))
          return true;
      }
      return false;
    });
    if (s == setIDs.end())
    {
      XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,不是指定玩家" << XEND;
      return false;
    }
  }

  DWORD dwProcess = 0;
  for (auto &s : setIDs)
  {
    SceneUser* pTarget = SceneUserManager::getMe().getUserByID(s);
    if (pTarget == nullptr || pUser->check2PosInNine(pTarget) == false)
      continue;
    ++dwProcess;
  }

  if (param1 < m_dwTime || m_dwNum > dwProcess)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// rand item
RandItemStep::RandItemStep(DWORD questid) : BaseStep(questid)
{

}

RandItemStep::~RandItemStep()
{

}

bool RandItemStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  xLuaData& item = param.getMutableData("item");
  auto itemf = [&](const string& key, xLuaData& data)
  {
    ItemInfo oItem;
    oItem.set_id(data.getTableInt("1"));
    oItem.set_count(data.getTableInt("2"));
    combinItemInfo(m_vecItemInfo, TVecItemInfo{oItem});
  };
  item.foreach(itemf);

  bool bCorrect = true;
  for (auto &v : m_vecItemInfo)
  {
    if (ItemConfig::getMe().getItemCFG(v.id()) == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "id:" << v.id() << "未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
    }
  }

  m_dwNum = param.getTableInt("num");
  if (m_dwNum > m_vecItemInfo.size())
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "num:" << m_dwNum << "数量大于道具库" << XEND;
    bCorrect = false;
  }

  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod >= ERANDITEMMETHOD_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "method:" << dwMethod << "不合法" << XEND;
    bCorrect = false;
  }
  m_eMethod = static_cast<ERandItemMethod>(dwMethod);

  return bCorrect;
}

bool RandItemStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  if (m_eMethod == ERANDITEMMETHOD_RAND)
  {
    TSetDWORD setIndex;
    for (DWORD d = 0; d < m_dwNum; ++d)
    {
      DWORD dwIndex = 0;
      DWORD dwMax = 40;
      while (dwIndex++ < dwMax)
      {
        DWORD dwRand = randBetween(0, m_vecItemInfo.size() - 1);
        auto s = find(setIndex.begin(), setIndex.end(), dwRand);
        if (s != setIndex.end())
          continue;
        setIndex.insert(dwRand);
        break;
      }
    }

    subGroup = m_dwFailJump;
    for (auto &s : setIndex)
    {
      if (s >= m_vecItemInfo.size())
        continue;
      const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(m_vecItemInfo[s].id());
      if (pCFG == nullptr)
        continue;

      DWORD dwCount = m_vecItemInfo[s].count();
      stringstream sstr;
      sstr << dwCount;

      pItem->addsparams(sstr.str());
      pItem->addsparams(pCFG->strNameZh);

      pItem->addqparams(pCFG->dwTypeID);
      pItem->addqparams(dwCount);

      // check food
      if (pCFG->eItemType == EITEMTYPE_FOOD)
      {
        sstr.str("");
        vector<SRecipeMaterial> vecMaterial;
        RecipeConfig::getMe().collectMaterial(pCFG->dwTypeID, vecMaterial);
        for (size_t i = 0; i < vecMaterial.size(); ++i)
        {
          SRecipeMaterial& r = vecMaterial[i];
          if (r.type == EMaterialType_Item)
          {
            const SItemCFG* pMaterialCFG = ItemConfig::getMe().getItemCFG(r.key);
            if (pMaterialCFG != nullptr)
              sstr << pMaterialCFG->strNameZh;
          }
          else if (r.type == EMaterialType_ItemType)
          {
            const SItemErrorCFG* pCFG = ItemConfig::getMe().getItemErrorCFG(static_cast<EItemType>(r.key));
            if (pCFG != nullptr)
              sstr << pCFG->name;
          }

          if (i != vecMaterial.size() - 1)
            sstr << "、";
        }
        pItem->addsparams(sstr.str());
      }

      XDBG << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "随机出信息" << pCFG->strNameZh << dwCount << "随机出参数" << pCFG->dwTypeID << dwCount << XEND;
      subGroup = m_dwFinishJump;
    }
  }
  else if (m_eMethod == ERANDITEMMETHOD_CHECK)
  {
    TVecQWORD vecQParams;
    pItem->collectQParams(vecQParams);

    TVecItemInfo vecItemInfo;
    for (size_t i = 0; i < vecQParams.size(); i += 2)
    {
      ItemInfo oItem;
      oItem.set_id(vecQParams[i]);
      oItem.set_count(vecQParams[i + 1]);
      combinItemInfo(vecItemInfo, TVecItemInfo{oItem});
    }

    if (pUser->getPackage().checkItemCount(vecItemInfo, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_QUEST_RANDITEM) == false)
      subGroup = m_dwFailJump;
    else
      subGroup = m_dwFinishJump;
  }
  else if (m_eMethod == ERANDITEMMETHOD_REMOVE)
  {
    TVecQWORD vecQParams;
    pItem->collectQParams(vecQParams);

    TVecItemInfo vecItemInfo;
    for (size_t i = 0; i < vecQParams.size(); i += 2)
    {
      ItemInfo oItem;
      oItem.set_id(vecQParams[i]);
      oItem.set_count(vecQParams[i + 1]);
      combinItemInfo(vecItemInfo, TVecItemInfo{oItem});
    }

    Package& rPackage = pUser->getPackage();
    if (rPackage.checkItemCount(vecItemInfo, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_QUEST_RANDITEM) == false)
    {
      subGroup = m_dwFailJump;
    }
    else
    {
      rPackage.reduceItem(vecItemInfo, ESOURCE_QUEST, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_QUEST_RANDITEM);
      subGroup = m_dwFinishJump;
    }
  }
  else
  {
    subGroup = m_dwFailJump;
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// carrier
CarrierStep::CarrierStep(DWORD questid) : BaseStep(questid)
{

}

CarrierStep::~CarrierStep()
{

}

void CarrierStep::reset(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  if (m_bGuild && m_bFocus)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return;
    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return;
    const GuildSMember* pTarget = pUser->getGuild().randMember();//TSetQWORD{pUser->id});
    if (pTarget == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机公会成员失败" << XEND;
      return;
    }
    pStep->add_names(pTarget->name());
    BaseStep::stepUpdate(pUser);
  }

  BaseStep::reset(pUser);
}

bool CarrierStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwCarrierID = param.getTableInt("carrierid");
  if (TableManager::getMe().getBusCFG(m_dwCarrierID) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "carrierid:" << m_dwCarrierID << "未在 Table_Bus.txt 表中找到" << XEND;
    return false;
  }

  m_dwNum = param.getTableInt("num");
  m_bFocus = param.getTableInt("focus") != 0;
  m_bGuild = param.getTableInt("guild") != 0;
  m_bTeam = param.getTableInt("team") != 0;

  return true;
}

bool CarrierStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (m_dwCarrierID != param1)
    return false;

  if (m_bGuild && pUser->hasGuild() == false)
  {
    XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,自己不是公会成员" << XEND;
    return false;
  }
  if (m_bTeam && pUser->getTeam().getTeamID() == 0)
  {
    XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,自己不是组队成员" << XEND;
    return false;
  }

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;

  const TSetTmpUser setUser = pUser->getQuest().getTmpUser();
  const TSetTmpNpc& setNpc = pUser->getQuest().getTmpNpc();
  if (setUser.empty() == true && setNpc.empty() == true)
    return false;

  DWORD dwProcess = 0;
  auto s = find_if(setUser.begin(), setUser.end(), [&](SceneUser* p) -> bool{
    if (p->id == pUser->id)
      return false;
    if (m_bGuild && (p->hasGuild() == false || p->getGuild().id() != pUser->getGuild().id()))
      return false;
    if (m_bTeam && p->getTeam().getTeamID() == 0)
      return false;
    if (pUser->getTeam().getTeamID() != p->getTeam().getTeamID())
      return false;
    dwProcess += 1;
    return true;
  });
  if (s == setUser.end())
  {
    XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,未含有符合成员" << XEND;
    return false;
  }
  if (m_bFocus)
  {
    auto s = find_if(setUser.begin(), setUser.end(), [&](const SceneUser* p) -> bool{
      if (m_bGuild && p->hasGuild() == false)
        return false;
      string name = p->name;
      for (int i = 0; i < pStep->names_size(); ++i)
      {
        if (name == pStep->names(i))
          return true;
      }
      return false;
    });
    if (s == setUser.end())
    {
      XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,不是指定玩家 :";
      for (int i = 0; i < pStep->names_size(); ++i)
        XDBG << pStep->names(i);
      XDBG << XEND;
      return false;
    }
  }

  if (dwProcess < m_dwNum)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// battle
BattleStep::BattleStep(DWORD questid) : BaseStep(questid)
{

}

BattleStep::~BattleStep()
{

}

bool BattleStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwMonsterID = param.getTableInt("monster");
  if (NpcConfig::getMe().getNpcCFG(m_dwMonsterID) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "monsterid:" << m_dwMonsterID << "未在 Table_Monster.txt 表中找到" << XEND;
    return false;
  }

  m_dwNum = param.getTableInt("num");
  m_bFocus = param.getTableInt("focus") != 0;
  m_bGuild = param.getTableInt("guild") != 0;
  return true;
}

void BattleStep::reset(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  if (m_bGuild && m_bFocus)
  {
    TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
    if (pItem == nullptr)
      return;
    QuestStep* pStep = pItem->getStepData();
    if (pStep == nullptr)
      return;
    const GuildSMember* pTarget = pUser->getGuild().randMember();//TSetQWORD{pUser->id});
    if (pTarget == nullptr)
    {
      XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机公会成员失败" << XEND;
      return;
    }
    pStep->add_names(pTarget->name());
    BaseStep::stepUpdate(pUser);
  }

  BaseStep::reset(pUser);
}

bool BattleStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (pUser->getTeam().getTeamID() == 0)
  {
    XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,自己不是组队成员" << XEND;
    return false;
  }
  if (m_bGuild && pUser->hasGuild() == false)
  {
    XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,自己不是公会成员" << XEND;
    return false;
  }

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;
  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;

  bool bFind = false;
  const TMapGTeamMember& mapMember = pUser->getTeam().getTeamMemberList();
  for (auto &m : mapMember)
  {
    SceneUser* p = SceneUserManager::getMe().getUserByID(m.second.charid());
    if (p == nullptr)
      continue;
    if (m_bGuild && (p->hasGuild() == false || pUser->getGuild().id() != p->getGuild().id()))
      continue;
    if (pUser->getTeam().getTeamID() != p->getTeam().getTeamID())
      continue;
    if (pUser->getScene() != p->getScene())
      continue;
    if (m_bFocus && (pStep->names_size() == 0 || m.second.name() != pStep->names(0)))
      continue;
    bFind = true;
    break;
  }
  if (!bFind)
  {
    XDBG << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,不是指定玩家 :";
    for (int i = 0; i < pStep->names_size(); ++i)
      XDBG << pStep->names(i);
    XDBG << XEND;
    return false;
  }

  pStep->set_process(pStep->process() + 1);
  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// pet
PetStep::PetStep(DWORD questid) : BaseStep(questid)
{

}

PetStep::~PetStep()
{

}

bool PetStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod <= EPETMETHOD_MIN || dwMethod >= EPETMETHOD_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "method :" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<EPetMethod>(dwMethod);

  m_dwBaseLv = param.getTableInt("baselv");
  m_dwFriendLv = param.getTableInt("friendlv");
  m_dwNum = param.getTableInt("num");
  return true;
}

bool PetStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (m_eMethod == EPETMETHOD_FRIENDLV_WAIT)
  {
    if (pUser->getUserPet().getPetCount(m_dwBaseLv, m_dwFriendLv) < m_dwNum)
      return false;

    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }
  else if (m_eMethod == EPETMETHOD_FRIENDLV_NOW)
  {
    subGroup = pUser->getUserPet().getPetCount(m_dwBaseLv, m_dwFriendLv) < m_dwNum ? m_dwFailJump : m_dwFinishJump;
    return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
  }
  else
  {
    XERR << "[任务-进度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "推进" << m_strContent << "questid :" << m_dwQuestID << "失败,method :" << m_eMethod << "不合法" << XEND;
    return false;
  }

  return true;
}

// cookfood
CookFoodStep::CookFoodStep(DWORD questid) : BaseStep(questid)
{

}

CookFoodStep::~CookFoodStep()
{

}

bool CookFoodStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwFoodId = param.getTableInt("id");
  if (m_dwFoodId && FoodConfig::getMe().getFoodCfg(m_dwFoodId) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "id:" << m_dwFoodId << "未在 Table_Food.txt 表中找到" << XEND;
    return false;
  }
  m_dwNum = param.getTableInt("num");
  if (m_dwNum == 0)
    m_dwNum = 0;
  return true;
}

bool CookFoodStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;
  
  if (m_dwFoodId && param1 != m_dwFoodId)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;

  QuestStep* pStep = pItem->getStepData();
  if (pStep == nullptr)
    return false;
  pStep->set_process(pStep->process() + 1);

  if (pStep->process() < m_dwNum)
  {
    BaseStep::stepUpdate(pUser);
    return false;
  }

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// scene
SceneStep::SceneStep(DWORD questid) : BaseStep(questid)
{

}

SceneStep::~SceneStep()
{

}

bool SceneStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwMapID = param.getTableInt("map");

  if (MapConfig::getMe().getMapCFG(m_dwMapID) == nullptr)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "map :" << m_dwMapID << "未在Table_Map.txt表中找到" << XEND;
    return false;
  }

  return true;
}

bool SceneStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  subGroup = pUser->getScene()->getMapID() == m_dwMapID ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// cook
CookStep::CookStep(DWORD questid) : BaseStep(questid)
{

}

CookStep::~CookStep()
{

}

bool CookStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod <= ECOOKMETHOD_MIN || dwMethod >= ECOOKTYPE_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method :" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<ECookMethod>(dwMethod);

  if (m_eMethod == ECOOKMETHOD_LV)
  {
    auto lvf = [&](const string& key, xLuaData& data)
    {
      m_mapLv2Step[atoi(key.c_str())] = data.getInt();
    };
    param.getMutableData("lv").foreach(lvf);

    XDBG << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "lvinfo :";
    for (auto &m : m_mapLv2Step)
      XDBG << m.first << m.second;
    XDBG << XEND;
  }

  return true;
}

bool CookStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if (m_eMethod == ECOOKMETHOD_LV)
  {
    subGroup = 1;
    auto m = m_mapLv2Step.find(pUser->getSceneFood().getCookerLv());
    if (m != m_mapLv2Step.end())
      subGroup = m->second;
  }
  else
  {
    return false;
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// buff
BuffStep::BuffStep(DWORD questid) : BaseStep(questid)
{

}

BuffStep::~BuffStep()
{

}

bool BuffStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod <= EBUFFMETHOD_MIN || dwMethod >= EBUFFMETHOD_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method :" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<EBuffMethod>(dwMethod);

  m_dwBuffID = param.getTableInt("buffid");
  return true;
}

bool BuffStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (m_eMethod == EBUFFMETHOD_CHECK)
  {
    subGroup = pUser->m_oBuff.haveBuff(m_dwBuffID) == true ? m_dwFinishJump : m_dwFailJump;
    return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
  }
  else if (m_eMethod == EBUFFMETHOD_WAIT)
  {
    if (pUser->m_oBuff.haveBuff(m_dwBuffID) == false)
      return false;
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }

  return false;
}

// social
TutorStep::TutorStep(DWORD questid) : BaseStep(questid)
{

}

TutorStep::~TutorStep()
{

}

bool TutorStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod <= ETUTORMETHOD_MIN || dwMethod >= ETUTORMETHOD_MAX)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method :" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<ETutorMethod>(dwMethod);

  m_bTutor = param.getTableInt("need_tutor") != 0;
  return true;
}

bool TutorStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  DWORD dwCount = pUser->getSocial().getRelationCount(ESOCIALRELATION_TUTOR);
  DWORD dwTime = pUser->getSocial().getRelationTime(0, ESOCIALRELATION_TUTOR);

  if (m_eMethod == ETUTORMETHOD_WAIT)
  {
    if ((m_bTutor && dwCount == 0) || (!m_bTutor && dwCount != 0))
      return false;
    if (m_bTutor && (param1 < dwTime || param1 - dwTime < MiscConfig::getMe().getTutorCFG().dwStudentGraduationTime))
      return false;
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }
  else if (m_eMethod == ETUTORMETHOD_CHECK)
  {
    if ((m_bTutor && dwCount == 0) || (!m_bTutor && dwCount != 0))
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    if (m_bTutor && (param1 < dwTime || param1 - dwTime < MiscConfig::getMe().getTutorCFG().dwStudentGraduationTime))
      return BaseStep::doStep(pUser, m_dwFailJump, param1, param2, sparam1);
    return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
  }
  else if (m_eMethod == ETUTORMETHOD_GRADUATION)
  {
    subGroup = pUser->getTutorTask().graduation() == true ? m_dwFinishJump : m_dwFailJump;
    return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
  }

  return false;
}

// christmas
ChristmasStep::ChristmasStep(DWORD questid) : BaseStep(questid)
{

}

ChristmasStep::~ChristmasStep()
{

}

bool ChristmasStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwPreQuestID = param.getTableInt("quest");
  auto dataf = [this](const string& key, xLuaData& data)
  {
    DWORD time = 0;
    const string& starttime = data.getTableString("time");
    if (starttime.empty() == false)
      parseTime(starttime.c_str(), time);

    if(time != 0)
    {
      vecStartTime.push_back(time);
      XLOG << "[圣诞节任务]-配置chirtsmas" << "questid :" << m_dwQuestID << "prequestid:" << m_dwPreQuestID << "time:" << starttime << "time1:" << time << XEND;
    }
  };
  param.getMutableData("starttime").foreach(dataf);

  return true;
}

bool ChristmasStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if(pUser->getQuest().isAccept(m_dwPreQuestID) == false)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  TPtrQuestItem pPreItem = pUser->getQuest().getQuest(m_dwPreQuestID);
  if (pItem == nullptr || pPreItem == nullptr)
    return false;

  if(pPreItem->getStep() == 0)
    return false;

  TVecQWORD vecQParams;
  pItem->collectQParams(vecQParams);
  if(vecQParams.size() != 0)
    return false;

  //DWORD dwTime = xTime::getCurSec();
  for(auto s : vecStartTime)
  {
    //if(dwTime < s)
    {
      pItem->addqparams(s);
      XLOG << "[圣诞节任务]-执行" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "questid :" << m_dwQuestID << "time:" << s << XEND;
    }
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

ChristmasRunStep::ChristmasRunStep(DWORD questid) : BaseStep(questid)
{

}

ChristmasRunStep::~ChristmasRunStep()
{

}

bool ChristmasRunStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  auto dataf = [this](const string& key, xLuaData& data)
  {
    DWORD time = 0;
    DWORD mailid = data.getTableInt("mailid");
    const string& starttime = data.getTableString("time");
    if (starttime.empty() == false)
      parseTime(starttime.c_str(), time);

    if(time != 0 && reward != 0)
    {
      m_mapReward.insert(std::make_pair(time, mailid));
      XLOG << "[圣诞节任务]-配置chirtsmasrun" << "questid :" << m_dwQuestID << "time:" << starttime << "time1:" << time << "mailid:" << mailid << XEND;
    }
  };
  param.getMutableData("starttime").foreach(dataf);

  return true;
}

bool ChristmasRunStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if(pUser->getQuest().isAccept(m_dwQuestID) == false)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;

  DWORD dwTime = xTime::getCurSec();
  TVecQWORD vecQParams;
  pItem->collectQParams(vecQParams);
  for(DWORD i = 0; i < vecQParams.size(); i++)
  {
    auto it = m_mapReward.find(vecQParams[i]);
    if(it != m_mapReward.end() && dwTime >= it->first)
    {
      MailManager::getMe().sendMail(pUser->id, it->second);
      pItem->setQParams(i, 0);
      XLOG << "[圣诞节任务]-领奖" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "questid :" << m_dwQuestID << "mailid:" << it->second << XEND;
    }
  }

  TVecQWORD vec;
  pItem->collectQParams(vec);
  for(auto s: vec)
  {
    if(s != 0)
      return false;
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

BeingStep::BeingStep(DWORD questid) : BaseStep(questid)
{

}

BeingStep::~BeingStep()
{

}

bool BeingStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwBeingID = param.getTableInt("beingid");
  m_dwBeingLv = param.getTableInt("beinglv");

  m_bFailStay = param.getTableInt("fail_stay") == 1;

  return true;
}

bool BeingStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  bool checkOK = false;
  const SSceneBeingData* pBData = nullptr;
  if (m_dwBeingID != 0)
    pBData = pUser->getUserBeing().getBeingData(m_dwBeingID);
  else
    pBData = pUser->getUserBeing().getCurBeingData();

  if (pBData != nullptr)
  {
    checkOK = true;

    if (m_dwBeingLv)
    {
      if (pBData->dwLv >= m_dwBeingLv)
        checkOK = true;
      else
        checkOK = false;
    }
  }

  if (m_bFailStay)
  {
    if (!checkOK)
      return false;
  }

  subGroup = checkOK ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}
// quest step - check joy
CheckJoyStep::CheckJoyStep(DWORD questid) : BaseStep(questid)
{

}

CheckJoyStep::~CheckJoyStep()
{

}

bool CheckJoyStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwType = param.getTableInt("type");
  if (dwType <= JOY_ACTIVITY_MIN || dwType >= JOY_ACTIVITY_MAX || EJoyActivityType_IsValid(dwType) == false)
    return false;
  m_eType = static_cast<EJoyActivityType>(dwType);

  return true;
}

bool CheckJoyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;

  DWORD dwCount = pUser->getUserSceneData().getJoyByType(m_eType);

  subGroup = dwCount >= MiscConfig::getMe().getJoyLimitByType(m_eType) ? m_dwFailJump : m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - add joy
AddJoyStep::AddJoyStep(DWORD questid) : BaseStep(questid)
{

}

AddJoyStep::~AddJoyStep()
{

}

bool AddJoyStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwType = param.getTableInt("type");
  if (dwType <= JOY_ACTIVITY_MIN || dwType >= JOY_ACTIVITY_MAX || EJoyActivityType_IsValid(dwType) == false)
    return false;
  m_eType = static_cast<EJoyActivityType>(dwType);
  m_dwAddJoy = param.getTableInt("value");

  return true;
}

bool AddJoyStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;

  bool bSuccess = pUser->getUserSceneData().addJoyValue(m_eType, m_dwAddJoy);
  subGroup = (bSuccess == false) ? m_dwFailJump : m_dwFinishJump;

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// rand dialog
RandDialogStep::RandDialogStep(DWORD questid) : BaseStep(questid)
{

}

RandDialogStep::~RandDialogStep()
{

}

bool RandDialogStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  DWORD dwOrder = 0;
  xLuaData& param = data.getMutableData("Params");
  xLuaData& dialog = param.getMutableData("dialog");
  auto dialogfunc = [&](const string& key, xLuaData& data)
  {
    auto addfunc = [&](const string& subkey, xLuaData& subdata)
    {
      m_mapDialog[dwOrder].push_back(subdata.getInt());
    };
    data.foreach(addfunc);
    ++dwOrder;
  };
  dialog.foreach(dialogfunc);


  return true;
}

bool RandDialogStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;

  DWORD order = 0;
  TVecQWORD vecQParams;
  pItem->collectQParams(vecQParams);
  if(vecQParams.empty() == true)
  {
    pItem->addqparams(order);
    pItem->addqparams(order);
  }
  else
  {
    order = vecQParams[0];
  }

  auto s = m_mapDialog.find(order);
  if(s == m_mapDialog.end() || s->second.empty() == true)
    return false;

  DWORD num = 0;
  num = randBetween(0, s->second.size()-1);
  DWORD dwRandID = m_mapDialog[order][num];
  order = (order + 1) % m_mapDialog.size();
  pItem->setQParams(0, order);
  pItem->setQParams(1, dwRandID);

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - cg
CgStep::CgStep(DWORD questid) : BaseStep(questid)
{

}

CgStep::~CgStep()
{

}

bool CgStep::init(xLuaData& data)
{
  return BaseStep::init(data);
}

bool CgStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  subGroup = m_dwFinishJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - check servant
CheckServantStep::CheckServantStep(DWORD questid) : BaseStep(questid)
{

}

CheckServantStep::~CheckServantStep()
{

}

bool CheckServantStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwServantID = param.getTableInt("id");
  return true;
}

bool CheckServantStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  bool bSuccess = false;
  if (pUser->getServant().getServantID() == m_dwServantID)
    bSuccess = true;

  subGroup = bSuccess ? m_dwFinishJump : m_dwFailJump;
  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - client_plot
ClientPlotStep::ClientPlotStep(DWORD questid) : BaseStep(questid)
{

}

ClientPlotStep::~ClientPlotStep()
{

}

bool ClientPlotStep::init(xLuaData& data)
{
  return BaseStep::init(data);
}

bool ClientPlotStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1); 
}

// quest step - chat
ChatStep::ChatStep(DWORD questid) : BaseStep(questid)
{
}

ChatStep::~ChatStep()
{
}

bool ChatStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  auto keyf = [&](const string& key, xLuaData& data)
  {
    m_setKey.insert(data.getString());
  };
  m_setKey.clear();
  param.getMutableData("key").foreach(keyf);

  for (auto &s : m_setKey)
    XDBG << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "key :" << s << XEND;
  return true;
}

bool ChatStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  TSetString setKeys;
  for (auto &s : m_setKey)
  {
    const string& t = CommonConfig::getMe().getTranslateString(s, pUser->getLanguage());
    if (t.empty() == true)
      return false;
    setKeys.insert(t);
  }
  auto s = setKeys.find(sparam1);
  if (s == setKeys.end())
    return false;
  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

// quest step - transfer
TransferStep::TransferStep(DWORD questid) : BaseStep(questid)
{
}

TransferStep::~TransferStep()
{
}

bool TransferStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_dwFromTransferid = param.getTableInt("fromNpcId");
  m_dwToTransferid = param.getTableInt("toNpcId");

  return true;

}

bool TransferStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  // 传送阵相关任务的判断
  if(m_dwFromTransferid != 0 && m_dwToTransferid != 0){
    if(m_dwFromTransferid != param1 || m_dwToTransferid != param2){
      return false;
    }
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - redialog
ReDialogStep::ReDialogStep(DWORD questid) : BaseStep(questid)
{
}

ReDialogStep::~ReDialogStep()
{
}

bool ReDialogStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  DWORD dwMethod = param.getTableInt("method");
  if (dwMethod != EREDIALOGMETHOD_RAND && dwMethod != EREDIALOGMETHOD_CLEAR)
  {
    XERR << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "method :" << dwMethod << "不合法" << XEND;
    return false;
  }
  m_eMethod = static_cast<EReDialogMethod>(dwMethod);
  m_bExclude = param.getTableInt("exclude") == 1;
  m_dwTimes = param.getTableInt("times");

  m_setDialogIDs.clear();
  auto dialogf = [&](const string& key, xLuaData& data)
  {
    m_setDialogIDs.insert(data.getInt());
  };
  param.getMutableData("dialog").foreach(dialogf);
  return true;
}

bool ReDialogStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  TPtrQuestItem pItem = pUser->getQuest().getQuest(m_dwQuestID);
  if (pItem == nullptr)
    return false;

  subGroup = m_dwFinishJump;
  if (m_eMethod == EREDIALOGMETHOD_RAND)
  {
    TVecQWORD vecIDs;
    pItem->collectQParams(vecIDs);
    if (vecIDs.size() >= m_dwTimes)
    {
      subGroup = m_dwFailJump;
    }
    else
    {
      while (true)
      {
        DWORD* p = randomStlContainer(m_setDialogIDs);
        if (p == nullptr)
        {
          subGroup = m_dwFailJump;
          break;
        }

        if (m_bExclude)
        {
          auto v = find(vecIDs.begin(), vecIDs.end(), *p);
          if (v != vecIDs.end())
            continue;
        }

        pItem->addqparams(*p);
        break;
      }
    }
  }
  else if (m_eMethod == EREDIALOGMETHOD_CLEAR)
  {
    pItem->clearqparams();
  }
  else
  {
    subGroup = m_dwFailJump;
  }

  return BaseStep::doStep(pUser, subGroup, param1, param2, sparam1);
}

// quest step - chat system
ChatSystemStep::ChatSystemStep(DWORD questid) : BaseStep(questid)
{
}

ChatSystemStep::~ChatSystemStep()
{
}

bool ChatSystemStep::init(xLuaData& data)
{
  if (BaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  auto keyf = [&](const string& key, xLuaData& data)
  {
    m_vecKey.emplace_back(data.getString());
  };
  m_vecKey.clear();
  param.getMutableData("key").foreach(keyf);

  m_dwRelation = param.getTableInt("relation");
  m_dwMultiKeyWord = param.getTableInt("multi_keyword");
  m_dwOrder = param.getTableInt("order");
  m_dwChannel = param.getTableInt("channel");
  if(EGameChatChannel_IsValid(m_dwChannel) == false)
    return false;

  for (auto &s : m_vecKey)
    XDBG << "[任务步骤-" << m_strContent << "] questid :" << m_dwQuestID << "group :" << m_dwSubGroup << "key :" << s << XEND;
  return true;
}

bool ChatSystemStep::doStep(SceneUser* pUser, DWORD subGroup /*= 0*/, QWORD param1 /*= 0*/, QWORD param2 /*= 0*/, const string& sparam1 /*= ""*/)
{
  if (pUser == nullptr)
    return false;

  if(param2 != m_dwChannel)
    return false;

  if(m_dwRelation == ESOCIALRELATION_MERRY)
  {
    if(pUser->getUserWedding().checkMarryRelation(param1) == false)
      return false;
  }
  else if(m_dwRelation != 0 && pUser->getSocial().checkRelation(param1, static_cast<ESocialRelation>(m_dwRelation)) == false)
      return false;

  TVecString vecKeys;
  for (auto &s : m_vecKey)
  {
    const string& t = CommonConfig::getMe().getTranslateString(s, pUser->getLanguage());
    if (t.empty() == true)
      return false;
    vecKeys.emplace_back(t);
  }

  bool ret = false;
  if(m_dwMultiKeyWord == 0)
  {
    for (auto &s : vecKeys)
    {
      string::size_type idx = sparam1.find(s);
      if(idx != string::npos)
      {
        ret = true;
        break;
      }
    }
  }
  else
  {
    string::size_type temp = 0;
    for (auto &s : vecKeys)
    {
      string::size_type idx = sparam1.find(s);
      if(idx != string::npos && idx >= temp)
      {
        if(m_dwOrder != 0)
          temp = idx;
      }
      else
        return false;
    }

    ret = true;
  }
  if(ret == false)
    return false;

  return BaseStep::doStep(pUser, m_dwFinishJump, param1, param2, sparam1);
}

