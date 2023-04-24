#include "ActivityManager.h"
#include "LuaManager.h"
#include "SceneServer.h"
#include "MsgManager.h"
#include "SceneManager.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "Package.h"
#include "ActivityCmd.pb.h"
#include "ActivityStep.h"
#include "ActivityConfig.h"
#include "Scene.h"
#include "SceneManager.h"
#include "SceneNpc.h"
#include "XoclientConfig.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "GMCommandRuler.h"
#include "SceneNpcManager.h"

bool ActivityBase::init(const SActivityCFG* pCfg)
{
  if (!pCfg)
    return false;
  m_dwId = pCfg->m_dwId;
  m_durtaion = pCfg->m_duration;
  //
  for (auto stageIt = pCfg->m_vecStage.begin(); stageIt != pCfg->m_vecStage.end(); stageIt++)
  {
    //add stage
    ActivityStageBase* pStage =  TypeItemFactory<ActivityStageBase>::getMe().create(stageIt->stageNode.content, stageIt->stageNode.oParams);
    if (!pStage)
    {
      XERR << "[活动-初始化] 创建stage失败:" << stageIt->stageNode.content << XEND;
      return false;
    }
    
    //attach node
    for (auto nodeIt = stageIt->vecNode.begin(); nodeIt != stageIt->vecNode.end(); ++nodeIt)
    {
      ActivityNodeBase* pNode = TypeItemFactory<ActivityNodeBase>::getMe().create(nodeIt->content, nodeIt->oParams);
      if (!pNode)
      {
        XERR << "[活动-初始化] 创建node失败:" << nodeIt->content << XEND;
        return false;
      }
      pStage->addNote(nodeIt->type, pNode);
    }    

    if (stageIt->stageNode.content == "stop_stage")
    {
      m_stopStage.push_back(pStage);
    }
    else
    {
      m_stage.push_back(pStage);
    }
  }

  XLOG << "[活动-初始化]" << m_uid <<"act id"<<m_dwId<<"act id"<< m_dwId << m_durtaion << XEND;
  return true;
}

ActivityBase::~ActivityBase()
{
  for (auto it = m_stage.begin(); it != m_stage.end(); ++it)
  {
    if (*it)
      SAFE_DELETE(*it);
  }

  for (auto it = m_stopStage.begin(); it != m_stopStage.end(); ++it)
  {
    if (*it)
      SAFE_DELETE(*it);
  }
  m_stage.clear();
  m_stopStage.clear();
}

bool ActivityBase::isNeedDel()
{
  if (isExpire())
    return true;
  return false;
}

bool ActivityBase::run(DWORD curSec)
{
  if (isNeedDel())
    return true;

  checkState(curSec);
  onStateChange(curSec);

  return true;
}

DWORD ActivityBase::getNpcCount(const TVecDWORD& vec)
{
  DWORD count = 0;
  for (auto &v : vec)
  {
    auto it = m_mapNpcs.find(v);
    if (it != m_mapNpcs.end())
    {
      count +=it->second.size();
    }

    for (auto &q : m_npcQueue)
    {
      if (q.getID() == v)
        count++;
    }
  }
  return count;
}
DWORD ActivityBase::getNpcCount(DWORD npcId)
{
  TVecDWORD vec;
  vec.push_back(npcId);
  return getNpcCount(vec);
}

bool ActivityBase::stop()
{
  m_isStop = true;
  return true;
}

void ActivityBase::onEnterScene(SceneUser* pUser)
{
  if (!pUser) return;
  if (!pUser->getScene()) return;
  if (pUser->getScene() != m_pScene) return;    //同场景才推送

  if(m_mapSpecialEffect.empty() == false)
  {
    for(auto s : m_mapSpecialEffect)
    {
      SpecialEffectCmd cmd;
      cmd.set_dramaid(s.first);
      cmd.set_starttime(s.second);
      PROTOBUF(cmd, send, len);
      pUser->sendCmdToMe(send, len);
    }
  }

  onEnter(pUser);
}

void ActivityBase::onLeaveScene(SceneUser* pUser)
{
  if (!pUser) return;
  if (!pUser->getScene()) return;
  if (pUser->getScene() != m_pScene) return;    //同场景才触发

  onLeave(pUser);
}

void ActivityBase::setActProgressNtfCmd(ActProgressNtfCmd& cmd)
{
  m_cmdCache = cmd;
}

void ActivityBase::onAddNpc(SceneNpc* pNpc)
{
  if (!pNpc)
    return;
  pNpc->setActivityUid(m_uid);

  auto &guidMap = m_mapNpcs[pNpc->getNpcID()];
  guidMap.insert(pNpc->getTempID());

  //
  //if (m_dwId == 1)
  //{
  //  if (pNpc->getNpcID() != 30027 && pNpc->getNpcID() != 30025)
  //  {
  //    pNpc->setNotSyncDead();
  //  }
  //}
}

void ActivityBase::onDelNpc(SceneNpc* pNpc)
{
  if (!pNpc)
    return;
  auto it = m_mapNpcs.find(pNpc->getNpcID());
  if (it == m_mapNpcs.end())
    return;
  auto subIt = it->second.find(pNpc->getTempID());
  if (subIt != it->second.end())
  {
    it->second.erase(subIt);
  }
}

void ActivityBase::setKillerName(DWORD npcId, std::string killer)
{
  m_mapKiller[npcId] = killer;
  XLOG << "[活动-设置-击杀者]" << npcId << killer << XEND;
}

const std::string& ActivityBase::getKillerName(DWORD npcId)
{
  auto it = m_mapKiller.find(npcId);
  if (it != m_mapKiller.end())
    return it->second;
  return STRING_EMPTY;
}

bool ActivityBase::checkHaveNpc(DWORD npcid)
{
  auto it = m_mapNpcs.find(npcid);
  if (it == m_mapNpcs.end())
    return false;
  return it->second.empty() == false;
}

const TSetQWORD& ActivityBase::getNpcGUIDS(DWORD npcid)
{
  auto it = m_mapNpcs.find(npcid);
  if (it == m_mapNpcs.end())
  {
    static const TSetQWORD emptyset;
    return emptyset;
  }
  return it->second;
}

bool ActivityBase::checkState(DWORD curSec)
{
  switch (m_state)
  {
  case  EActivityState_none:
  {
    m_state = EActivityState_start;
    break;
  }
  case EActivityState_start:
  {
    m_state = EActivityState_process;
    break;
  }
  case EActivityState_process:
  {
    if ((m_durtaion != 0 && curSec >= m_endTime) || m_isStop)     //m_duration = 0 always run
      m_state = EActivityState_stop;
    break;
  }
  case EActivityState_stop:
  case EActivityState_expire:
    break;
  }

  return true;
}

bool ActivityBase::onStateChange(DWORD curSec)
{
  if (m_state == EActivityState_start)
  {
    return onStart(curSec);
  }
  
  if (m_state == EActivityState_process)
  {
    return onProcess(curSec);
  }

  if (m_state == EActivityState_stop)
  {
    return onStop(curSec);
  }
  return false;
}

bool ActivityBase::onStart(DWORD curSec)
{
  //m_startTime = curSec;
  m_endTime = m_startTime + m_durtaion;
  XDBG << "[活动-开始]" << "uid:" << m_uid <<"act id"<<m_dwId<< "id:" << m_dwId << "mapid:" << m_mapId << "sec:" << curSec << "start time:" << m_startTime << "end time" << m_endTime << XEND;

  return true;
}

bool ActivityBase::onProcess(DWORD curSec)
{
  //XDBG << "[活动-处理]" <<"uid:"<< m_uid <<"act id"<<m_dwId<< "id:" <<  m_dwId << "mapid:"<< m_mapId << "sec:" << curSec << "start time:" << m_startTime << "end time" << m_endTime << XEND;

  for (auto it = m_stage.begin(); it != m_stage.end(); ++it)
  {
    m_pCurStage = *it;
    if (m_pCurStage && m_pCurStage->isCondtionStage() == false)
    {
      if (m_pCurStage->run(curSec, this) == EACTIVITY_RET_CONTINUE)
        break;
    }
  }
  // 条件stage, 由其他stage 触发执行
  for (auto it = m_stage.begin(); it != m_stage.end(); ++it)
  {
    m_pCurStage = *it;
    if (m_pCurStage && m_pCurStage->isCondtionStage())
    {
      if (m_pCurStage->needReset())
        (m_pCurStage->resetData());
      else if (m_pCurStage->isEnabled())
        m_pCurStage->run(curSec, this);
    }
  }
  summonNpc();
  m_pCurStage = nullptr;
  return true;
}

void ActivityBase::enableStage(DWORD stage)
{
  if (stage == 0)
    return;
  for (auto &it : m_stage)
  {
    if (it && it->isCondtionStage() && it->getIndex() == stage)
      it->enable();
  }
}

bool ActivityBase::checkCondionStageOver(DWORD stage)
{
  for (auto &it : m_stage)
  {
    if (it && it->isCondtionStage() && it->getIndex() == stage)
    {
      if (it->isEnabled())
        return it->isOver();
      else
        return true;
    }
  }
  return false;
}

void ActivityBase::onResetStage(DWORD stage)
{
  // stage 重置时, 若有条件stage, 重置其条件stage
  for (auto &it : m_stage)
  {
    if (it && it->isCondtionStage() && it->getMasterStage() == stage)
      it->setResetStatus();
  }
}

void ActivityBase::onNpcDie(SceneNpc* pNpc)
{
  if (pNpc == nullptr || pNpc->getScene() != m_pScene)
    return;
  DWORD npcid = pNpc->getNpcID();
  if (m_setRecordMonsters.find(npcid) != m_setRecordMonsters.end())
  {
    auto it = m_mapKillMonsters.find(npcid);
    if (it == m_mapKillMonsters.end())
      m_mapKillMonsters[npcid] = 1;
    else
      it->second ++;
  }

  if(npcid == 30042)
  {
    ActivityCapra* pCapra = dynamic_cast<ActivityCapra*>(this);
    if(pCapra == nullptr)
      return;
    xSceneEntrySet entrySet;
    m_pScene->getAllEntryList(SCENE_ENTRY_USER, entrySet);
    for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
    {
      SceneUser *pUser = (SceneUser *)(*entryIt);
      if(pUser)
        pUser->getServant().onGrowthFinishEvent(ETRIGGER_CAPRA);
    }
  }
}

void ActivityBase::addSpecialEffect(DWORD dramaid, DWORD starttime) 
{
  m_mapSpecialEffect.insert(std::make_pair(dramaid, starttime));
  XLOG << "[活动-特效] 添加" << "uid:" << m_uid << "act id" << m_dwId << "mapid:" << m_mapId << "dramaid:" << dramaid << "start time:" << starttime << XEND;
}

void ActivityBase::delSpecialEffect(DWORD dramaid)
{
  m_mapSpecialEffect.erase(dramaid);
  XLOG << "[活动-特效] 删除" << "uid:" << m_uid << "act id" << m_dwId << "mapid:" << m_mapId << "dramaid:" << dramaid << "time:" << now() << XEND;
}

DWORD ActivityBase::getKillCount(DWORD npcid)
{
  auto it = m_mapKillMonsters.find(npcid);
  return it != m_mapKillMonsters.end() ? it->second : 0;
}

void ActivityBase::resetKillCount(DWORD npcid)
{
  auto it = m_mapKillMonsters.find(npcid);
  if (it != m_mapKillMonsters.end())
    it->second = 0;
}

bool ActivityBase::onStop(DWORD curSec)
{
  XLOG << "[活动-结束]" << "uid:" << m_uid << "act id" << m_dwId << "mapid:" << m_mapId << "sec:" << curSec << "start time:" << m_startTime << "end time" << m_endTime << XEND;
  
  //call stop stage, only call once
  for (auto it = m_stopStage.begin(); it != m_stopStage.end(); ++it)
  {
    if (*it)
      (*it)->run(curSec, this);
  }

  m_isExpire = true;
  m_state = EActivityState_expire;

  //结束通知session
  {
    ActivityStopSessionCmd cmd;
    cmd.set_uid(m_uid);
    cmd.set_id(m_dwId);
    cmd.set_mapid(m_mapId);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
  return true;
}

void ActivityBase::summonNpc()
{
  if (!m_pScene)
    return;
  if (m_npcQueue.empty())
    return;
  NpcDefine def = m_npcQueue.front();
 
  SceneNpc*pNpc = SceneNpcManager::getMe().createNpc(def, m_pScene);
  if (!pNpc)
  {
    m_npcQueue.pop_front();
    XERR << "[活动-执行-队列召唤] 召唤npc 失败" << getUid() << def.getID() << XEND;
    return;
  }
  m_npcQueue.pop_front();
  onAddNpc(pNpc);
  XLOG << "[活动-执行-队列召唤] 召唤npc 成功" << getUid() << pNpc->getNpcID() << pNpc->getTempID()<<"pos"<<pNpc->getPos().getX()<<pNpc->getPos().getY()<<pNpc->getPos().getZ()<< XEND;
}

SceneNpc* ActivityBase::getSceneNpc(DWORD id)
{
  auto it = m_mapNpcs.find(id);
  if (it == m_mapNpcs.end())
    return nullptr; 
  if (it->second.empty())
    return  nullptr;

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(*(it->second.begin()));
  return pNpc;
}

/************************************************************************/
/*ActivityQuest                                                                     */
/************************************************************************/
ActivityQuest::ActivityQuest(Scene* pScene) :ActivityBase(pScene)
{
  m_reaminReward = MiscConfig::getMe().getQACFG().rewardCount;
}

ActivityQuest::~ActivityQuest()
{
  clear();
}

void ActivityQuest::onAddNpc(SceneNpc* pNpc)
{
  ActivityBase::onAddNpc(pNpc);
    
  refresh();
}

void ActivityQuest::onDelNpc(SceneNpc* pNpc)
{
  ActivityBase::onDelNpc(pNpc); 
  clear();
  XLOG << "[活动-问答] del npc" << m_uid <<"act id"<<m_dwId<< XEND;
}

void ActivityQuest::clear()
{
  m_quests.clear();  

  //QuestActAnswerUserCmd cmd;
  //cmd.set_ret(0);
  //PROTOBUF(cmd, send, len);

  //for (auto it = m_answerUser.begin(); it != m_answerUser.end(); ++it)
  //{
  //  if (it->second.empty())
  //    continue;
  //  SceneUser* pUser = SceneUserManager::getMe().getUserByID(it->first);
  //  if (!pUser)
  //    continue;
  //  pUser->sendCmdToMe(send, len);
  //}
  m_answerUser.clear();
}

void ActivityQuest::refresh()
{
  clear();
  std::vector<SXoclientCFG*> res = XoclientConfig::getMe().getRandomXoclient(5);
  
  for (auto& v : res)
  {
    if (v)
      m_quests[v->m_dwId] = v->m_dwAnserId;
  }

  stringstream ss;
  for (auto &v : m_quests)
    ss << v.first << ":" << v.second << ";";
  XLOG << "[活动-问答] 刷新问题:" << m_uid <<"act id"<<m_dwId<<ss.str() << XEND;
}

bool ActivityQuest::query(SceneUser* pUser, Query& rev)
{
  if (!pUser)
    return false;
  
  if (m_reaminReward == 0)
    return false;

  std::queue<DWORD> q;
  do 
  {
    if (m_quests.empty())
    {
      XERR << "[活动-问答] 暂时没有题目刷出来, act uid" << m_uid <<"act id"<<m_dwId<< "mapid" << m_mapId << "charid:" << pUser->id << XEND;
      return  false;
    }
    if (m_wrongAnswer.find(pUser->id) != m_wrongAnswer.end())
    {
      //msg
      XINF << "[活动-问答] 本次已经回答过, wrong act uid" << m_uid <<"act id"<<m_dwId<< "charid:" << pUser->id << XEND;
      rev.set_ret(EQUERYSTATE_ANSWERED_WRONG);
      break;
    }
    auto it = m_answerUser.find(pUser->id);

    if (it == m_answerUser.end())
    {
      for (auto &v : m_quests)
      {
        q.push(v.first);
      }
      m_answerUser[pUser->id] = q;
    }
    else
    {
      //msg
      XINF << "[活动-问答] 本次已经回答过,right act uid" << m_uid << "act id" << m_dwId <<"mapid"<<m_mapId << "charid:" << pUser->id << XEND;            
      rev.set_ret(EQUERYSTATE_ANSWERED_RIGHT);
      break;
    }
    rev.set_ret(EQUERYSTATE_OK);
  } while (0);

  PROTOBUF(rev, send1, len1);
  pUser->sendCmdToMe(send1, len1);

  if (rev.ret() == EQUERYSTATE_OK)
  {
    NewInter cmd;
    cmd.set_npcid(rev.npcid());
    InterData* pData = cmd.mutable_inter();
    if (!pData)
    {
      XERR << "[活动-问答] protobuf error, act uid" << m_uid <<"act id"<<m_dwId<< "mapid" << m_mapId << "charid:" << pUser->id << XEND;
      return false;
    }
    pData->set_interid(q.front());
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }
  return true;
}

bool ActivityQuest::answer(SceneUser* pUser, Answer cmd)
{
  if (!pUser)
    return false;

  if (m_reaminReward == 0)
    return false;

  if (m_quests.empty())
  {
    XERR << "[活动-问答] 暂时没有题目刷出来, act uid" << m_uid <<"act id"<<m_dwId<< "mapid" << m_mapId << "charid:" << pUser->id << XEND;
    return false;
  }

  cmd.set_source(ESOURCE_QA);

  auto it = m_answerUser.find(pUser->id);
  if (it == m_answerUser.end())
    return false;
  auto &q = it->second;
  if (cmd.interid() != q.front())
  {
    XERR << "[活动-问答] 回答的问题顺序非法, act uid" << m_uid <<"act id"<<m_dwId<< "mapid" << m_mapId << "charid:" << pUser->id << "questid:" << cmd.interid() << "server questid:" << q.front() << XEND;
    return false;
  }

  auto ansIt = m_quests.find(cmd.interid());
  if (ansIt == m_quests.end())
  {
    XERR << "[活动-问答] 异常，找不到问题答案, act uid" << m_uid <<"act id"<<m_dwId<< "mapid" << m_mapId << "charid:" << pUser->id << "questid:" << cmd.interid() << XEND;
    return false;
  }

  if (ansIt->second != cmd.answer())
  {
    XINF << "[活动-问答] 问题回到错误, act uid" << m_uid <<"act id"<<m_dwId<< "mapid" << m_mapId << "charid:" << pUser->id << "questid:" << cmd.interid() << "answer" << cmd.answer() << "right answer:" << ansIt->second << XEND;
    //pUser->getVar().setVarValue(EVARTYPE_ACTIVITY_QUEST, 1);
    m_wrongAnswer.insert(pUser->id);
    cmd.set_correct(false);
  }
  else
  {
    q.pop();
    cmd.set_correct(true);
  }
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  XINF << "[活动-问答] act uid" << m_uid <<"act id"<<m_dwId<<"act id"<<m_dwId<< "mapid" << m_mapId << "charid:" << pUser->id << "questid" << cmd.interid() <<"correct"<<cmd.correct() << XEND;

  if (!cmd.correct())
    return true;

  // next inter
  if (q.empty())
  {
    //pUser->getLevel();
    QWORD rewardId = MiscConfig::getMe().getQACFG().getRewardByLevel(pUser->getLevel());

    TVecItemInfo vecItemInfo;
    if (RewardManager::roll(rewardId, pUser, vecItemInfo, ESOURCE_QA) == false)
    {
      XERR << "[活动-问答] 全部回答正确,奖励随机出错, act uid" << m_uid <<"act id"<<m_dwId<<"act id"<<m_dwId<< "charid:" << pUser->id << "level" << pUser->getLevel() << "reward id"<<rewardId << XEND;
    }
    else
    {
      //pUser->getPackage().addItemAvailable(vecItemInfo);
      pUser->getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);
    }
    m_reaminReward--;
    if (m_reaminReward == 0)
    {
      delAllNpc();
    }
    XINF << "[活动-问答] 全部回答正确, act uid" << m_uid <<"act id"<<m_dwId<<"act id"<<m_dwId<< "charid:" << pUser->id<<"level"<<pUser->getLevel()<<"reward id"<<rewardId<<"remain reward"<<m_reaminReward << XEND;
    return true;
  }

  NewInter msg;
  msg.set_npcid(cmd.npcid());
  InterData* pData = msg.mutable_inter();
  if (!pData)
  {
    XERR << "[活动-问答] protobuf error, act uid" << m_uid <<"act id"<<m_dwId<<"act id"<<m_dwId<< "charid:" << pUser->id << XEND;
    return false;
  }
  {
    pData->set_interid(q.front());
    PROTOBUF(msg, send, len);
    pUser->sendCmdToMe(send, len);
    XINF << "[活动-问答] next inter" << m_uid <<"act id"<<m_dwId<<"act id"<<m_dwId<< "charid:" << pUser->id << "questid" << cmd.interid() << "next inter" << q.front() << XEND;
  }
  return true;
}

void ActivityQuest::delAllNpc()
{
  TSetQWORD setNpcGuid;
  for (auto &v : m_mapNpcs)
  {
    for (auto &subV : v.second)
    {
      setNpcGuid.insert(subV);
    }
  }

  for (auto & v : setNpcGuid)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(v);
    if (pNpc)
    {
      XINF << "[活动-问答] 奖励领取完，删除npc, act uid" << m_uid <<"act id"<<m_dwId<<"act id"<<m_dwId<<  "remain reward" << m_reaminReward<<"npc id"<<v<< XEND;
      pNpc->removeAtonce();
    }
  }
}


/************************************************************************/
// ActivityMoneyCat
/************************************************************************/
ActivityMoneyCat::ActivityMoneyCat(Scene* pScene) : ActivityBase(pScene)
{

}

ActivityMoneyCat::~ActivityMoneyCat()
{

}

bool ActivityMoneyCat::checkMoneyEnd()
{
  DWORD maxmoney = MiscConfig::getMe().getMoneyCatCFG().dwMaxMoney;
  return m_dwCurMoney >= maxmoney;
}

void ActivityMoneyCat::addUserMoney(const string& name, DWORD money)
{
  auto it = m_mapName2Money.find(name);
  if (it != m_mapName2Money.end())
  {
    it->second += money;
    return;
  }

  m_mapName2Money[name] = money;
}

string ActivityMoneyCat::getMaxMoneyUser()
{
  DWORD max = 0;
  string name = "";
  for (auto &m : m_mapName2Money)
  {
    if (m.second >= max)
    {
      max = m.second;
      name = m.first;
    }
  }

  return name;
}

DWORD ActivityMoneyCat::getUserMoney(const string& name)
{
  auto it = m_mapName2Money.find(name);
  if (it != m_mapName2Money.end())
    return it->second;

  return 0;
}

/************************************************************************/
/* ActivityCapra                                                                     */
/************************************************************************/

ActivityCapra::ActivityCapra(Scene* pScene) : ActivityBase(pScene)
{

}

ActivityCapra::~ActivityCapra()
{

}

bool ActivityCapra::onEnter(SceneUser* pUser)
{
  if(!pUser)
    return false;

  if(EACTPROGRESS_1 == m_progress)
  {
    TVecDWORD vecQuests = MiscConfig::getMe().getCapraActivityCFG().m_vecAddQuests;
    for(auto& q : vecQuests)
    {
      pUser->getQuest().acceptQuest(q, true);
    }
  }

  return true;
}

bool ActivityCapra::onLeave(SceneUser* pUser)
{
  if(!pUser)
    return false;

  TVecDWORD vecQuests = MiscConfig::getMe().getCapraActivityCFG().m_vecDelQuests;
  for(auto& q : vecQuests)
  {
    pUser->getQuest().abandonGroup(q, true);
  }

  TVecDWORD vecBuffs = MiscConfig::getMe().getCapraActivityCFG().m_vecDelBuffs;
  for(auto& b : vecBuffs)
  {
    pUser->m_oBuff.del(b, true);
  }
  pUser->m_oBuff.update(xTime::getCurMSec());

  return true;
}

/************************************************************************/
/*ActivityBCat                                                                 */
/************************************************************************/
/*bool ActivityBCat::onStart(DWORD curSec)
{
  ActivityBase::onStart(curSec);
  LuaManager::getMe().call<void>("activitystatus", m_dwId, m_mapId, true, m_startTime, m_endTime);
  return true;
}

bool ActivityBCat::onProcess(DWORD curSec)
{
  LuaManager::getMe().call<void>("active_scene_timer", curSec);
  return true;
}

bool ActivityBCat::onStop(DWORD curSec)
{
  LuaManager::getMe().call<void>("activitystatus", m_dwId, m_mapId, false);

  ActivityBase::onStop(curSec);
  return true;
}*/

/************************************************************************/
/*ActivityManager                                                                      */
/************************************************************************/
ActivityManager::ActivityManager()
{
}

ActivityManager::~ActivityManager()
{
  for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
  {
    if (it->second)
      SAFE_DELETE(it->second);
  }
  m_mapActvity.clear();

  TypeItemFactory<ActivityStageBase>::delMe();
  TypeItemFactory<ActivityNodeBase>::delMe();
}

void ActivityManager::init()
{
  //stage
  TypeItemFactory<ActivityStageBase>::getMe().reg( NEW TypeCreator<ActivityStageBase, ActivityStageBase>("stage"));
  TypeItemFactory<ActivityStageBase>::getMe().reg( NEW TypeCreator<ActivityStageBase, ActivityStageBase>("stop_stage"));
  TypeItemFactory<ActivityStageBase>::getMe().reg( NEW TypeCreator<ActivityStageBase, ActivityStageLoop>("par_stage"));

  //node
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeNotify>("notify"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeWait>("wait"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeSummon>("summon"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeNormalSummon>("normal_summon"));
  //TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeRandomSummon>("randomsummon"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeAttackSummon>("attacksummon"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeCheckNpcNum>("checknpcnum"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeQuestSummon>("questsummon"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeProgress>("progress"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeCheckEnd>("checkend"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeMove>("move"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityTurnMonster>("turnmonster"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeKill>("kill"));
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeSceneGm>("scenegm")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeMoneyHit>("money_hit")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeNotifyMoneyHit>("notify_money_hit")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityCheckNpcDie>("check_npcdie")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityHaveBuff>("check_buff")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityMarkRecordNpc>("record_monster")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityKillCnt>("kill_count")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityPlayDialog>("playdialog")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivitySupplySummon>("supply_summon")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeTimeSummon>("time_summon")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityNodeNpcGm>("npc_gm")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivitySpecialEffect>("spcialeffct")); 
  TypeItemFactory<ActivityNodeBase>::getMe().reg( NEW TypeCreator<ActivityNodeBase, ActivityCheckNpcHp>("check_npchp")); 
}

/*void ActivityManager::sendBCatStart(DWORD dwMapID, DWORD dwStartTime, DWORD dwEndTime)
{

}
void ActivityManager::sendBCatInform(DWORD dwMsgID, DWORD dwMapID, DWORD dwEffectType)
{
  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if (pScene == nullptr)
  {
    XERR << "[活动-B格猫公告] mapid :" << dwMapID << "不存在" << XEND;
    return;
  }

  MsgManager::sendWorldMsg(dwMsgID, MsgParams(pScene->name));

  if (EEffectType_IsValid(dwEffectType) == false)
  {
    XERR << "[活动-B格猫公告] effecttype :" << dwEffectType << "不合法" << XEND;
    return;
  }

  EffectUserCmd cmd;
  cmd.set_effecttype(static_cast<EEffectType>(dwEffectType));
  PROTOBUF(cmd, send, len);
  MsgManager::sendWorldCmd(send, len);
}

void ActivityManager::sendBCatUFOPos(DWORD dwMapID, float x, float y, float z)
{
  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if (pScene == nullptr)
    return;

  xPos pos(x, y, z);
  Cmd::BCatUFOPosActCmd cmd;
  cmd.mutable_pos()->set_x(pos.getX());
  cmd.mutable_pos()->set_y(pos.getY());
  cmd.mutable_pos()->set_z(pos.getZ());
  PROTOBUF(cmd, send, len);

  xSceneEntrySet set;
  pScene->getAllEntryList(SCENE_ENTRY_USER, set);
  for (auto &s : set)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(s);
    if (pUser != nullptr)
      pUser->sendCmdToMe(send, len);
  }
}*/

void ActivityManager::timer(DWORD curTime)
{
  FUN_TIMECHECK_30();
   if (thisServer->getServerState() != ServerState::run)
     return;
 
   //run
   for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
   {
     if (it->second)
       it->second->run(curTime);
   }
  
   //del expire
   for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); )
   {
     if (!it->second)
     {
       it = m_mapActvity.erase(it);
       continue;
     }
     if (it->second->isNeedDel())
     {
       XLOG << "[活动-删除] del:" << it->second->getUid() << XEND;
       syncActivityToSession(it->second->getId(), false);
       SAFE_DELETE(it->second);
       it = m_mapActvity.erase(it);
     }
     else
       it++;
   }
}

bool ActivityManager::preProcess(const Cmd::ActivityTestAndSetSessionCmd& rev)
{
  const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(rev.id());
  if (!pCfg)
  {
    XERR << "[活动-预处理] 找不到活动策划表" << "uid:" << rev.uid() << "id:" << rev.id() << "mapid:" << rev.mapid() << XEND;
    return false;
  }
  
  string type = pCfg->m_condition.getTableString("type");
  if (type == "item")
  {
    DWORD itemId = pCfg->m_condition.getTableInt("id");
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (!pUser)
    {
      XERR << "[活动-预处理] 发起活动的玩家下线了" << "uid:" << rev.uid() << "id:" << rev.id() << "mapid:" << rev.mapid()<<rev.charid() << XEND;
      return false;
    }
    
    BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pPack)
    {
      if (!pPack->checkItemCount(itemId, 1))
      {
        XERR << "[活动-预处理] 玩家背包数量异常" << rev.uid() << rev.id() << rev.mapid() << rev.charid()<<"itemid"<<itemId << XEND;
        return false;
      } 
      pPack->reduceItem(itemId, ESOURCE_CRAZYGHOST);
    }
  }
  return true;
}

void ActivityManager::startActivity(const Cmd::ActivityTestAndSetSessionCmd& rev)
{
  if (rev.ret() == 0)
    return ;
  if (preProcess(rev) == false)
    return;

  ActivityBase* p = createActivity(rev);
  if (p)
  {
    syncActivityToSession(rev.id(), true);
    XLOG << "[活动-创建] 创建成功" << "uid:" << rev.uid() << "id:" << rev.id() << "mapid:" << rev.mapid() << XEND;
  }
}

void ActivityManager::onEnterScene(SceneUser* pUser)
{
  if (!pUser)
    return;
  for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
  {
    if (it->second)
      it->second->onEnterScene(pUser);
  }
}

void ActivityManager::onLeaveScene(SceneUser* pUser)
{
  if (!pUser) return;
  for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
  {
    if (it->second)
      it->second->onLeaveScene(pUser);
  }
}

ActivityBase* ActivityManager::createActivity(const Cmd::ActivityTestAndSetSessionCmd& rev)
{
  const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(rev.id());
  if (!pCfg)
  {
    XERR << "[活动-创建] 找不到对应id 的配置表" <<"uid:"<<rev.uid()<< "id:"<<rev.id() <<"mapid:"<<rev.mapid()<< XEND;
    return nullptr;
  }

  Scene* pScene = SceneManager::getMe().getSceneByID(rev.mapid());
  if (pScene == nullptr)
  {
    XERR << "[活动-创建] 找不到对应mapid 的场景" << "uid:" << rev.uid() << "id:" << rev.id() << "mapid:" << rev.mapid() << XEND;
    return nullptr;
  }
  ActivityBase* pActivity = nullptr;    
  EActivityType type = pCfg->m_actType;
  switch (type)
  {
  case EACTIVITYTYPE_QUEST:
    pActivity = NEW ActivityQuest(pScene);
    break;
  case EACTIVITYTYPE_MONEYCAT:
    pActivity = NEW ActivityMoneyCat(pScene);
    break;
  case EACTIVITYTYPE_CAPRA_IN:
    pActivity = new ActivityCapra(pScene);
    break;
  default:
    pActivity = NEW ActivityBase(pScene);
    break;
  }

  if (!pActivity)
  {
    XERR << "[活动-创建] 分配内存失败" << "uid:" << rev.uid() << "id:" << rev.id() << "mapid:" << rev.mapid() << XEND;
    return nullptr;
  }
  pActivity->setStartTime(rev.starttime());
  pActivity->setUid(rev.uid());
  pActivity->setMapId(rev.mapid());
  if (rev.charid())
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
      pActivity->setSponsorName(pUser->getName());
  }
  if (!pActivity->init(pCfg))
  {
    XERR << "[活动-创建] 初始化失败" << "uid:" << rev.uid() << "id:" << rev.id() << "mapid:" << rev.mapid() << XEND;
    SAFE_DELETE(pActivity);
    return nullptr;
  }

  auto it = m_mapActvity.find(rev.uid());
  if(m_mapActvity.end() != it)
  {
    printf("the same key for list, %s, %d \n", __FILE__, __LINE__);
    SAFE_DELETE(it->second);
  }

  m_mapActvity[rev.uid()] = pActivity;
  return pActivity;
}

ActivityBase* ActivityManager::getActivityByUid(QWORD id)
{
  auto it = m_mapActvity.find(id);
  if (it == m_mapActvity.end())
    return nullptr;

  return it->second;
}

void ActivityManager::stopActivity(DWORD id)
{
  for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
  {
    if (!it->second)
      continue;
    if (it->second->getId() == id)
    {
      it->second->stop();
    }
  }
}

void ActivityManager::onNpcDie(SceneNpc* pNpc)
{
  for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
  {
    if (!it->second)
      continue;
    it->second->onNpcDie(pNpc);
  }
}

bool ActivityManager::addGlobalActivity(DWORD id)
{
  std::set<DWORD>::iterator iter = m_setGlobalActivity.find(id);
  if(iter != m_setGlobalActivity.end())
  {
    XLOG << "[全服活动-Scene] 重新开启" << id << "已经开启" << XEND;
    return true;
  }

  m_setGlobalActivity.insert(id);
  XLOG << "[全服活动-Scene] 开启成功" << "添加" <<id << XEND;
  return true;
}
bool ActivityManager::delGlobalActivity(DWORD id)
{
  std::set<DWORD>::iterator iter = m_setGlobalActivity.find(id);
  if(iter == m_setGlobalActivity.end())
  {
    XERR << "[全服活动-Scene] 重新结束" << id << "已经结束" << XEND;
    return true;
  }

  m_setGlobalActivity.erase(iter);
  XLOG << "[全服活动-Scene] 结束成功" << "删除" << id << XEND;
  return true;
}

bool ActivityManager::isOpen(DWORD id, bool isGlobal)
{
  if(isGlobal == true)
  {
    std::set<DWORD>::iterator iter = m_setGlobalActivity.find(id);
    if(iter != m_setGlobalActivity.end())
      return true;
  }
  else
  {
    std::set<DWORD>::iterator iter = m_setActivity.find(id);
    if(iter != m_setActivity.end())
      return true;
  }

  return false;
}

bool ActivityManager::addActivity(DWORD id)
{
  std::set<DWORD>::iterator iter = m_setActivity.find(id);
  if(iter != m_setActivity.end())
  {
    XLOG << "[活动-Scene] 重新开启" << id << "已经开启" << XEND;
    return true;
  }

  m_setActivity.insert(id);
  XLOG << "[活动-Scene] 开启成功" << "添加" <<id << XEND;
  return true;
}
bool ActivityManager::delActivity(DWORD id)
{
  std::set<DWORD>::iterator iter = m_setActivity.find(id);
  if(iter == m_setActivity.end())
  {
    XERR << "[活动-Scene] 重新结束" << id << "已经结束" << XEND;
    return true;
  }

  m_setActivity.erase(iter);
  XLOG << "[活动-Scene] 结束成功" << "删除" << id << XEND;
  return true;
}

void ActivityManager::syncActivityToSession(DWORD id, bool open)
{
  NotifyActivitySessionCmd cmd;
  cmd.set_actid(id);
  cmd.set_open(open);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}
