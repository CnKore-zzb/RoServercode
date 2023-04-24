#include "ActivityStep.h"
#include "ActivityManager.h"
#include "NpcConfig.h"
#include "Scene.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "MsgManager.h"
#include "SceneManager.h"
#include "GMCommandRuler.h"
#include "SkillItem.h"
#include "SceneUser.h"

ActivityStageBase::~ActivityStageBase()
{
  for (auto it = m_seqNode.begin(); it != m_seqNode.end(); ++it)
  {
    if (*it)
      SAFE_DELETE(*it);
  }
  m_seqNode.clear();

  for (auto it = m_parNode.begin(); it != m_parNode.end(); ++it)
  {
    if (*it)
      SAFE_DELETE(*it);
  }
  m_parNode.clear();
  m_lastSeqPos = m_seqNode.begin();
}

EACTIVITY_RET ActivityStageBase::run(DWORD curSec, ActivityBase* pActivity)
{
  if (m_duration != 0)
  {
    if (m_expireTime == 0)
      m_expireTime = curSec + m_duration;

    if (curSec > m_expireTime)  //过期
      return EACTIVITY_RET_OK;
  }
  else
  {
    if (isLoopStage() == false && m_lastSeqPos == m_seqNode.end())
      return EACTIVITY_RET_OK;
  }
  /*else
  {
    if (isLoopStage() == false)
    {
      if (m_expireTime && curSec >= m_expireTime)
        return EACTIVITY_RET_OK;
      m_expireTime = curSec; // 仅执行一次
    }
  }
  */

 
  //XDBG << "[活动-执行] 阶段，curSec:" << curSec << "expire time:" << m_expireTime << "name" << getName() << XEND;

  for (auto it = m_lastSeqPos; it != m_seqNode.end();)
  {
    if (!(*it)) continue;

    // 等待分支执行结束
    if ((*it)->sonStageOver(pActivity) == false)
      break;

    if ((*it)->run(curSec, pActivity) == EACTIVITY_RET_OK)
    {
      it++;
      m_lastSeqPos = it;
    }
    else
    {
      break;
    }
  }


  for (auto it = m_parNode.begin(); it != m_parNode.end(); ++it)
  {
    if (*it)
      (*it)->run(curSec, pActivity);
  }

  //循环stage
  if (isLoopStage() && m_lastSeqPos == m_seqNode.end() && sonStageOver(pActivity))
  {
    resetData();
    pActivity->onResetStage(getIndex());
  }

  return EACTIVITY_RET_CONTINUE;
}

void ActivityStageBase::reset()
{
  m_duration = m_oParams.getTableInt("base_duration");
  m_lastSeqPos = m_seqNode.begin();
}

bool ActivityStageBase::sonStageOver(ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return true;

  for (auto it = m_seqNode.begin(); it != m_seqNode.end(); ++it)
  {
    if ((*it)->sonStageOver(pActivity) == false)
      return false;
  }
  return true;
}

void ActivityStageBase::resetData()
{
  m_lastSeqPos = m_seqNode.begin();
  for (auto &it : m_seqNode)
  {
    it->resetData();
  }
  m_bEnable = false;
  m_bReset = false;
  m_expireTime = 0;
}

void ActivityStageBase::addNote(EACTNODE_TYPE nodeType, ActivityNodeBase*pNode)
{
  if (!pNode)
    return;
  if (nodeType == EACTNODETYPE_SEQ)
    m_seqNode.push_back(pNode);
  else if (nodeType == EACTNODETYPE_PAR)
    m_parNode.push_back(pNode);
  m_lastSeqPos = m_seqNode.begin();
}


/************************************************************************/
/*ActivityNodeBase                                                                      */
/************************************************************************/

void ActivityNodeBase::reset()
{
  m_dwInterval = m_oParams.getTableInt("base_interval");
  m_limitCount = m_oParams.getTableInt("base_limitcount");    //-1  will no limit
  m_dwOkStage = m_oParams.getTableInt("ok_stage");
  m_dwFailStage = m_oParams.getTableInt("fail_stage");
}

bool ActivityNodeBase::sonStageOver(ActivityBase* pActivity)
{
  if (bSelectNode() == false)
    return true;
  if (pActivity == nullptr)
    return true;

  if (m_dwOkStage && pActivity->checkCondionStageOver(m_dwOkStage) == false)
    return false;
  if (m_dwFailStage && pActivity->checkCondionStageOver(m_dwFailStage) == false)
    return false;
  return true;
}

EACTIVITY_RET ActivityNodeBase::run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;

  //XDBG << "[活动-执行-节点] 基类，"<<getName()<< this << pActivity->getUid() << "curSec:" << curSec << "next time:" << m_dwNextTime <<  XEND;
  if (m_dwNextTime > curSec)
    return EACTIVITY_RET_CONTINUE;
  if (m_limitCount > 0 && m_count >= m_limitCount)
   return EACTIVITY_RET_OK;

  EACTIVITY_RET runret = v_run(curSec, pActivity);

  m_dwNextTime = curSec + m_dwInterval;
  m_count++;

  bool btimelimit = false;
  bool btimeup = false;
  if (m_oParams.has("LimitTime"))
  {
    btimelimit = true;
    if (m_dwEndTime == 0)
    {
      m_dwEndTime += m_oParams.getTableInt("LimitTime") + curSec;
    }
    if (curSec >= m_dwEndTime)
      btimeup = true;
  }

  bool bwait = btimelimit && !btimeup;

  if (bSelectNode()) // 条件节点
  {
    if (runret == EACTIVITY_RET_OK)
    {
      pActivity->enableStage(m_dwOkStage);
    }
    else
    {
      if (!bwait)
        pActivity->enableStage(m_dwFailStage);
    }
  }

  if (bwait)
  {
    if (runret == EACTIVITY_RET_OK && m_oParams.getTableInt("success_break_limit") == 1) // 执行成功, 不再等待limittime
      return EACTIVITY_RET_OK;
    return EACTIVITY_RET_CONTINUE;
  }
  if (btimeup)
    return EACTIVITY_RET_OK;

  return runret;
}

/************************************************************************/
/*ActivityNodeNotify                                                                      */
/************************************************************************/
EACTIVITY_RET ActivityNodeNotify::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;

  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  if (m_dwMsgId == ESYSTEMMSG_ID_ACT_GHOST_COMESOON)
  {
    if (!pActivity->getSponsorName().empty())
    {
      if (m_ntfRange == "map")
        MsgManager::sendMapMsg(pScene->getMapID(), m_dwMsgId, MsgParams(pActivity->getSponsorName(), pScene->name));
      else
        MsgManager::sendWorldMsg(m_dwMsgId, MsgParams(pActivity->getSponsorName(), pScene->name));
    }   
  }
  else if (m_dwMsgId == ESYSTEMMSG_ID_ACT_GHOST_END || m_dwMsgId == ESYSTEMMSG_ID_ACT_CAT_COMESOON || m_dwMsgId == ESYSTEMMSG_ID_ACT_CAT_END)
  {
    if (m_ntfRange == "map")
      MsgManager::sendMapMsg(pScene->getMapID(), m_dwMsgId, MsgParams(pScene->name));
    else
      MsgManager::sendWorldMsg(m_dwMsgId, MsgParams(pScene->name));
  }
  else
  {
    if (m_ntfRange == "map")
      MsgManager::sendMapMsg(pScene->getMapID(), m_dwMsgId);
    else
      MsgManager::sendWorldMsg(m_dwMsgId);
  }

  if (m_dwEffectId)
  {  
      MsgManager::sendWorldMsg(m_dwEffectId, MsgParams(), EMESSAGETYPE_MIDDLE_SHOW);      //屏幕中间展示warning
  }

  XLOG << "[活动-执行-节点] "<<getName()<< "curSec:" << curSec << "msg id"<<m_dwMsgId<<"effectid"<<m_dwEffectId << XEND;

  return EACTIVITY_RET_OK;
}

void ActivityNodeNotify::reset()
{
  ActivityNodeBase::reset();
  m_dwMsgId = m_oParams.getTableInt("id");
  m_dwEffectId = m_oParams.getTableInt("effectid");
  m_ntfRange = m_oParams.getTableString("ntfrange");
}
/************************************************************************/
/*ActivityBaseSummon                                                                      */
/************************************************************************/
DWORD ActivityBaseSummon::calcNeedNpcCount(ActivityBase* pActivity)
{
  if (!pActivity)
    return 0;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return 0;

  DWORD playerCount = SceneManager::getMe().getCreatureCount(SCENE_ENTRY_USER, pActivity->getScene()->getMapID());
  DWORD summonedCount = pActivity->getNpcCount(m_vecIds);

  DWORD limitCount = 0;
    
  if (m_dwAllPercent)
    limitCount = playerCount * m_dwAllPercent / 100;

  if (m_dwAllMax && limitCount > m_dwAllMax)
    limitCount = m_dwAllMax;

  if (m_dwAllMin && limitCount < m_dwAllMin)
    limitCount = m_dwAllMin;
  
  if (limitCount && summonedCount >= limitCount)
  {
    XLOG << "[活动-执行-节点] " << getName() << " 活动召唤的npc 数量超过最大限制，无法再召唤" << pActivity->getUid() << pScene->getMapID() << "player count:" << playerCount << "npc count:" << summonedCount << "m_dwAllMax" << m_dwAllMax <<"limitCount"<< limitCount <<"allPercent"<<m_dwAllPercent<<"m_dwAllMin"<<m_dwAllMin << XEND;
    return 0;
  }
  DWORD needCount = 0;  
  if (limitCount)
  {
    needCount = limitCount - summonedCount;
  }

  if (m_dwPercent)
  {
    needCount = playerCount * m_dwPercent / 100;
  }
 
  if (m_dwMax && needCount > m_dwMax)
    needCount = m_dwMax;
  if (needCount < m_dwMin)
    needCount = m_dwMin;

  XLOG << "[活动-执行-节点] " << getName() << "计算需要召唤的的npc数量" << pActivity->getUid() << pScene->getMapID()<<"needCount"<<needCount << "player count:" << playerCount << "npc count:" << summonedCount << "m_dwAllMax" << m_dwAllMax << "limitCount" << limitCount << "allPercent" << m_dwAllPercent << "m_dwAllMin" << m_dwAllMin << XEND;
  return needCount;
}

void ActivityBaseSummon::reset()
{
  auto func = [&](const std::string& key, xLuaData& data)
  {
    m_vecIds.push_back(data.getInt());
  };

  xLuaData oData = m_oParams.getMutableData("id");
  oData.foreach(func);
  m_dwPercent = m_oParams.getTableInt("percent");  //
  m_dwMax = m_oParams.getTableInt("max");  //
  if (m_oParams.has("min"))
    m_dwMin = m_oParams.getTableInt("min");  //
  else
    m_dwMin = 1;

  m_dwPosNpc = m_oParams.getTableInt("posnpc");
  m_dwPosRange = m_oParams.getTableInt("posrange");
  m_dwAllMax = m_oParams.getTableInt("allmax");
  m_dwAllMin = m_oParams.getTableInt("allmin");
  m_dwAllPercent = m_oParams.getTableInt("allpercent");
  m_npcDef.load(m_oParams.getMutableData("npcdef"));

  auto each_f = [&](const string& key, xLuaData& data)
  {
    DWORD mapid = atoi(key.c_str());
    xPos pos;
    pos.x = data.getTableFloat("1");
    pos.y = data.getTableFloat("2");
    pos.z = data.getTableFloat("3");
    m_pos[mapid] = pos;
  };

  xLuaData& rData = m_oParams.getMutableData("pos");
  rData.foreach(each_f);

}

/************************************************************************/
/*ActivityNodeSummon                                                                      */
/************************************************************************/
EACTIVITY_RET ActivityNodeSummon::v_run(DWORD curSec, ActivityBase* pActivity)
{
  //if (!pActivity)
  //  return EACTIVITY_RET_OK;
  //Scene* pScene = pActivity->getScene();
  //if (pScene == nullptr)
  //  return EACTIVITY_RET_OK;
  //
  //const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_npcDef.getID());
  //if (pCFG == nullptr)
  //{
  //  XERR << "[活动-执行-节点] " << getName() <<" 召唤npc 失败,找不到相应的npd 配置" <<  pActivity->getUid()<< getName() << m_npcDef.getID() << XEND;
  //  return EACTIVITY_RET_OK;
  //} 
 
  //DWORD summonNpcCount = pActivity->getNpcCount(m_npcDef.getID());
  //if (m_dwAllMax && summonNpcCount >= m_dwAllMax)
  //{
  //  XLOG << "[活动-执行-节点] " << getName() << " 活动召唤的npc 数量超过最大限制，无法再召唤" << pActivity->getUid() << pScene->getMapID() << m_npcDef.getID() << "npc count:" << summonNpcCount<<"maxcount"<< m_dwAllMax << XEND;
  //  return EACTIVITY_RET_OK;
  //}

  //xPos pos;
  //if (m_dwPosNpc)
  //{
  //  SceneNpc* pNpc = pActivity->getSceneNpc(m_dwPosNpc);
  //  if (!pNpc)
  //  {
  //    //TODO xlog
  //    return EACTIVITY_RET_OK;
  //  }
  //  pos = pNpc->getPos();  
  //}
  //DWORD rand = 0;
  //
  //for (DWORD i = 0; i < m_dwCount; ++i)
  //{
  //  if (m_oParams.has("weight"))
  //  {
  //    rand = randBetween(1, 100);
  //    if (rand > (DWORD)m_oParams.getTableInt("weight"))
  //      continue;
  //  }

  //  if (!m_dwPosNpc)
  //  { 
  //    auto it = m_pos.find(pScene->getMapID());
  //    if (it != m_pos.end())
  //    {
  //      pos = it->second;
  //    }
  //    else
  //      pScene->getRandPos(pos);
  //  }
  //  
  //  if (m_oParams.has("posrange"))
  //  {
  //    pScene->getRandPos(pos, m_dwPosRange, pos);
  //  }

  //  m_npcDef.setPos(pos);

  //  if (m_oParams.getTableInt("queue"))
  //  {
  //    pActivity->pushNpc(m_npcDef);
  //  }
  //  else
  //  {
  //    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(m_npcDef, pScene);
  //    if (pNpc)
  //    {
  //      pActivity->onAddNpc(pNpc);
  //      XLOG << "[活动-执行-节点] " << getName()<<" 召唤npc 成功" << pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID()<<
  //      "pos"<<pos.getX()<<pos.getY() <<pos.getZ()<< XEND;
  //    }
  //    else
  //    {
  //      XERR << "[活动-执行-节点] " << getName() << "召唤npc 失败" << pActivity->getUid() << getName() << pCFG->dwID <<
  //        "pos" << pos.getX() << pos.getY() << pos.getZ() << XEND;
  //    }
  //  }
  //}  
  //return EACTIVITY_RET_OK;

  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  DWORD needCount = calcNeedNpcCount(pActivity);
  if (needCount == 0)
    return EACTIVITY_RET_OK;

  DWORD* pNpcId = nullptr;
  xPos pos;
  if (m_dwPosNpc)
  {
    SceneNpc* pNpc = pActivity->getSceneNpc(m_dwPosNpc);
    if (!pNpc)
    {
      return EACTIVITY_RET_OK;
    }
    pos = pNpc->getPos();
  }

  for (DWORD i = 0; i < needCount; ++i)
  {
    //npcid
    pNpcId = randomStlContainer(m_vecIds);        //随机一个npcid
    if (!pNpcId)
    {
      XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败,随机到的npcid 为空" << pActivity->getUid() << getName() << XEND;
      continue;
    }
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(*pNpcId);
    if (pCFG == nullptr)
    {
      XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败,找不到相应的npd 配置" << pActivity->getUid() << getName() << *pNpcId << XEND;
      continue;
    }
    m_npcDef.setID(pCFG->dwID);

    //pos
    if (!m_dwPosNpc)
    {
      auto it = m_pos.find(pScene->getMapID());
      if (it != m_pos.end())
      {
        pos = it->second;
      }
      else
        pScene->getRandPos(pos);
    }
    if (m_oParams.has("posrange"))
    {
      pScene->getRandPos(pos, m_dwPosRange, pos);
    }
    m_npcDef.setPos(pos);
    /*   if (m_oParams.getTableInt("queue"))
    {
    pActivity->pushNpc(m_npcDef);
    }
    else*/
    {
      SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(m_npcDef, pScene);
      if (pNpc)
      {
        pActivity->onAddNpc(pNpc);
        XLOG << "[活动-执行-节点] " << getName() << " 召唤npc 成功" << pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID() <<
          "pos" << pos.getX() << pos.getY() << pos.getZ() << XEND;
      }
      else
      {
        XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败" << pActivity->getUid() << getName() << pCFG->dwID <<
          "pos" << pos.getX() << pos.getY() << pos.getZ() << XEND;
      }
    }
  }
  XLOG << "[活动-执行-节点] " << getName() << " 召唤npc 成功" << pActivity->getUid() << getName() << "召唤个数" << needCount << XEND;
  return EACTIVITY_RET_OK;
}

void ActivityNodeSummon::reset()
{
  //m_npcDef.load(m_oParams.getMutableData("npcdef"));
  //m_dwCount = m_oParams.getTableInt("count");
  //if (m_dwCount == 0) m_dwCount = 1;
  //m_dwPosNpc = m_oParams.getTableInt("posnpc");
  //m_dwWeight = m_oParams.getTableInt("weight");
  //m_dwAllMax = m_oParams.getTableInt("allmax");
  //m_dwPercent = m_oParams.getTableInt("percent");
  //m_dwPosRange = m_oParams.getTableInt("posrange");
  //auto each_f = [&](const string& key, xLuaData& data)
  //{
  //  DWORD mapid = atoi(key.c_str());
  //  xPos pos;
  //  pos.x = data.getTableFloat("1");
  //  pos.y = data.getTableFloat("2");
  //  pos.z = data.getTableFloat("3");
  //  m_pos[mapid] = pos;
  //};

  //xLuaData& rData = m_oParams.getMutableData("pos");
  //rData.foreach(each_f);
}

/************************************************************************/
/*ActivityNodeAttackSummon                                                                      */
/************************************************************************/
EACTIVITY_RET ActivityNodeAttackSummon::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  xPos pos;

  DWORD* pNpcId = nullptr;
  
  xSceneEntrySet set;
  pScene->getAllEntryList(SCENE_ENTRY_USER, set);
  
  if (set.empty())
  {
    return EACTIVITY_RET_OK;
  }

  DWORD needCount = calcNeedNpcCount(pActivity);
  if (needCount == 0)
    return EACTIVITY_RET_OK;
  
  for (DWORD i = 0; i < needCount; ++i)
  {
    xSceneEntry** ppUser = randomStlContainer(set);
    if (ppUser == nullptr || *ppUser == nullptr)
    {
      XERR<<"[活动-执行] 找不到玩家"<<  pActivity->getUid() << XEND;
      continue;
    }
    pNpcId = randomStlContainer(m_vecIds);        //随机一个npcid
    if (!pNpcId)
    {
      XERR << "[活动-执行] 召唤npc 失败,随机到的npcid 为空" <<  pActivity->getUid() << getName() << XEND;
      continue;;
    }
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(*pNpcId);
    if (pCFG == nullptr)
    {
      XERR << "[活动-执行] 召唤npc 失败,找不到相应的npd 配置" <<  pActivity->getUid() << getName() << *pNpcId << XEND;
      continue;;
    }

    m_npcDef.setID(pCFG->dwID);
    xPos pos = (*ppUser)->getPos();
    m_npcDef.setPos(pos);
    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(m_npcDef, pScene);
    if (pNpc)
    {
      pNpc->getNpcAI().setCurLockID((*ppUser)->id);
      pActivity->onAddNpc(pNpc);
      XDBG << "[活动-执行] 召唤npc 成功" <<  pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID() << XEND;
    }
    else
    {
      XERR << "[活动-执行] 召唤npc 失败" <<  pActivity->getUid() << getName() << pCFG->dwID << XEND;
    }
  }
  return EACTIVITY_RET_OK;
}

void ActivityNodeAttackSummon::reset()
{
}

/************************************************************************/
/*ActivityNodeCheckNpcNum                                                                      */
/************************************************************************/
EACTIVITY_RET ActivityNodeCheckNpcNum::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  DWORD count = pActivity->getNpcCount(m_vecIds);
  if (count >= m_dwMinCount)
    return EACTIVITY_RET_OK;
  
  DWORD diff = m_dwMinCount - count;
  
  NpcDefine define;
  xPos pos;
  define.setLife(1);
  define.setDisptime(480);
  DWORD* pNpcId = nullptr;
  for (DWORD i = 0; i < diff; ++i)
  {   
    pNpcId = randomStlContainer(m_vecIds);
    if (!pNpcId)
    {
      XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败,随机到的npcid 为空" <<  pActivity->getUid() << getName() << XEND;
      continue;;
    }
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(*pNpcId);
    if (pCFG == nullptr)
    {
      XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败,找不到相应的npd 配置" <<  pActivity->getUid() << getName() << *pNpcId << XEND;
      continue;;
    }
    define.setID(pCFG->dwID);
    pScene->getRandPos(pos);
    define.setPos(pos);
    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(define, pScene);
    if (pNpc)
    {
      pActivity->onAddNpc(pNpc);
      XDBG << "[活动-执行-节点] " << getName() << " 召唤npc 成功" <<  pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID() << XEND;
    }
    else
    {
      XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败" <<  pActivity->getUid() << getName() << pCFG->dwID << XEND;
    }
  }
  return EACTIVITY_RET_OK;
}

void ActivityNodeCheckNpcNum::reset()
{
  auto func = [&](const std::string& key, xLuaData& data)
  {
    m_vecIds.push_back(data.getInt());
  };

  xLuaData oData = m_oParams.getMutableData("id");
  oData.foreach(func);
  m_dwMinCount = m_oParams.getTableInt("min");
}

/************************************************************************/
/*ActivityNodeQuestSummon                                                                      */
/************************************************************************/
EACTIVITY_RET ActivityNodeQuestSummon::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;
  
  DWORD r = randBetween(1, 100);
  if (r > m_dwWeight)
  {
    XINF << "[活动-执行-节点] " << getName() << " 没有随到npc weight:" <<  pActivity->getUid() << m_dwWeight << XEND;
    return EACTIVITY_RET_OK;
  }

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwNpcId);
  if (pCFG == nullptr)
  {
    XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败,找不到相应的npc 配置" <<  pActivity->getUid() << getName() << m_dwNpcId << XEND;
    return EACTIVITY_RET_OK;
  }

  xPos pos;
  if (pScene->getRandPosAwayNpc(pos, 2.0) == false)
  {
    XINF << "[活动-执行-节点] " << getName() << " 没有随到npc 的位置 " <<  pActivity->getUid() << XEND;
    return EACTIVITY_RET_OK;
  }

  NpcDefine define;
  define.setID(m_dwNpcId);
  define.setLife(1);
  define.setDisptime(m_dwDuration);
  define.setPos(pos);  
  define.setDir(140);

  SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(define, pScene);
  if (pNpc)
  {
    pActivity->onAddNpc(pNpc);
    XDBG << "[活动-执行-节点] " << getName() << "召唤npc 成功" <<  pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID() << XEND;
  }
  else
  {
    XERR << "[活动-执行-节点] " << getName() << " 召唤npc 失败" <<  pActivity->getUid() << getName() << pCFG->dwID << XEND;
  }
  
  return EACTIVITY_RET_OK;
}

void ActivityNodeQuestSummon::reset()
{
  m_dwDuration = m_oParams.getTableInt("duration");  //npc 持续时间
  m_dwNpcId = m_oParams.getTableInt("id");
  m_dwWeight = m_oParams.getTableInt("weight");
}

/************************************************************************/
/*ActivityNodeProgress                                                                     */
/************************************************************************/
EACTIVITY_RET ActivityNodeProgress::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  DWORD progress = m_oParams.getTableInt("id");
  pActivity->setProgress(progress);   
  ActProgressNtfCmd cmd;
  cmd.set_id(pActivity->getId());
  cmd.set_progress(pActivity->getProgress());
  ActivityStageBase* pStage = pActivity->getCurStage();
  if (pStage)
  {
    cmd.set_starttime(pStage->m_expireTime - pStage->m_duration);
    cmd.set_endtime(pStage->m_expireTime);
  }
  pActivity->setActProgressNtfCmd(cmd);
  PROTOBUF(cmd, send, len);
  pScene->sendCmdToAll(send, len);
  
  XDBG << "[活动-执行-节点] " << getName() << " 任务进度通知" << pActivity->getUid() << getName()<<"进度"<<pActivity->getProgress() << XEND;
  if (pActivity->getProgress() == EACTPROGRESS_FAIL || pActivity->getProgress() == EACTPROGRESS_SUCCESS)
  {
    pActivity->stop();
  }

  return EACTIVITY_RET_OK;
}

/************************************************************************/
/*ActivityNodeCheckEnd                                                                      */
/************************************************************************/
void ActivityNodeCheckEnd::reset()
{
  std::string strType = m_oParams.getTableString("type");
  if (strType == "npcdie")
  {
    m_type = ECHECKTYPE_NPCDIE;
  }
  else if (strType == "moneycat")
  {
    m_type = ECHECKTYPE_MONEYCAT;
  }
  else
  {
    m_type = ECHECKTYPE_NONE;
  }

  m_dwParam1 = m_oParams.getTableInt("param1");
  m_dwProgess = m_oParams.getTableInt("progress");
  m_dwMsgId = m_oParams.getTableInt("msgid");
  m_ntfRange = m_oParams.getTableString("ntfrange");
}
EACTIVITY_RET ActivityNodeCheckEnd::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  if (m_type == ECHECKTYPE_NPCDIE)
  {
    //SceneNpc*pNpc = pActivity->getSceneNpc(m_dwParam1);
    std::string strKiller = pActivity->getKillerName(m_dwParam1);//广播击杀者。
    if (strKiller == "")
    {
       return EACTIVITY_RET_OK;
    }
    if (m_ntfRange == "map")
    {
      MsgManager::sendMapMsg(pScene->getMapID(), m_dwMsgId, MsgParams(strKiller));
    }
    else
      MsgManager::sendWorldMsg(m_dwMsgId, MsgParams(strKiller));

    XLOG << "[活动-击杀者]" << m_dwParam1 << strKiller << m_dwMsgId << XEND;
  }
  if (m_type == ECHECKTYPE_MONEYCAT)
  {
    ActivityMoneyCat* pMoneyCat = dynamic_cast<ActivityMoneyCat*> (pActivity);
    if (pMoneyCat == nullptr)
      return EACTIVITY_RET_OK;
    SceneNpc* pNpc = pActivity->getSceneNpc(m_dwParam1);
    if (pNpc)
      pNpc->setClearState();
    pActivity->stop();

    return EACTIVITY_RET_OK;
  }

  pActivity->setProgress(m_dwProgess);
  ActProgressNtfCmd cmd;
  cmd.set_id(pActivity->getId());
  cmd.set_progress(pActivity->getProgress());
  PROTOBUF(cmd, send, len);
  pScene->sendCmdToAll(send, len);
  
  XLOG << "[活动-执行-节点] " << getName() << " 任务提前结束" << pActivity->getUid() << getName()<<"type"<<m_type<<"param1"<<m_dwParam1 << XEND;

  pActivity->stop();                //活动提前结束

  return EACTIVITY_RET_OK;
}

/************************************************************************/
/*ActivityNodeMove                                                                      */
/************************************************************************/
void ActivityNodeMove::reset()
{
  m_dwNpcId = m_oParams.getTableInt("id");

  auto each_f = [&](const string& key, xLuaData& data)
  {
    DWORD mapid = atoi(key.c_str());
    std::vector<xPos>& vec = m_pos[mapid];    
    for (auto it = data.m_table.begin(); it != data.m_table.end(); ++it)
    {
      xPos pos;
      pos.x = it->second.getTableFloat("1");
      pos.y = it->second.getTableFloat("2");
      pos.z = it->second.getTableFloat("3");
      vec.push_back(pos);
    }
  };

  xLuaData& rData = m_oParams.getMutableData("pos");
  rData.foreach(each_f);
}
EACTIVITY_RET ActivityNodeMove::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;
  
  SceneNpc* pNpc = pActivity->getSceneNpc(m_dwNpcId);
  if (!pNpc)
    return EACTIVITY_RET_OK;

  if (!pNpc->getMoveAction().empty())
    return EACTIVITY_RET_OK;

  auto it = m_pos.find(pScene->getMapID());
  if (it == m_pos.end())
  {
    return EACTIVITY_RET_OK;
  }  
  std::vector<xPos>& vecPos = it->second;
  
  if (vecPos.empty())
  {
    //log
    return EACTIVITY_RET_OK;
  }
  if (m_index >= vecPos.size())
    m_index = 0;
  xPos pos = vecPos[m_index++];
  xPos oldPos = pNpc->getPos();

  pNpc->m_ai.moveTo(pos);
  xPos newPos = pNpc->getPos();
  XLOG << "[活动-执行-节点] " << getName() << " 移动npc 成功" << pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID()<<pos.getX()<<pos.getY()<<pos.getZ()<<"old pos"<<oldPos.getX()<<oldPos.getY()<<oldPos.getZ()<<"newpos"<<newPos.getX()<<newPos.getY()<<newPos.getZ()<<"index"<<m_index << XEND;
  Cmd::BCatUFOPosActCmd cmd;
  cmd.mutable_pos()->set_x(oldPos.getX());
  cmd.mutable_pos()->set_y(oldPos.getY());
  cmd.mutable_pos()->set_z(oldPos.getZ());
  PROTOBUF(cmd, send, len);
  pScene->sendCmdToAll(send, len);

  return EACTIVITY_RET_OK;
}

/************************************************************************/
/*ActivityTurnMonster                                                                      */
/************************************************************************/
void ActivityTurnMonster::reset()
{
  m_npcDef.load(m_oParams.getMutableData("npcdef")); 
  m_dwOldNpc = m_oParams.getTableInt("oldid");  //
}

EACTIVITY_RET ActivityTurnMonster::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;
  
  xPos pos;
  SceneNpc* pNpc = pActivity->getSceneNpc(m_dwOldNpc);
  if (pNpc)
  {
    pos = pNpc->getPos();
    pNpc->setClearState();
  }
  else
  {
    pScene->getRandPos(pos);
  }

  m_npcDef.setPos(pos);
  SceneNpc* pNewNpc = SceneNpcManager::getMe().createNpc(m_npcDef, pScene);
  if (pNewNpc)
  {
    pActivity->onAddNpc(pNewNpc);
    XLOG << "[活动-执行-节点] " << getName() << " npc 变身 成功" << pActivity->getUid() << m_dwOldNpc << pNewNpc->getNpcID() << pNewNpc->getTempID() << XEND;
  }
  else 
    XLOG << "[活动-执行-节点] " << getName() << " npc 变身 失败" << pActivity->getUid() << m_dwOldNpc << m_npcDef.getID()<<XEND;

  return EACTIVITY_RET_OK;
}

/************************************************************************/
/*ActivityNodeKill                                                                      */
/************************************************************************/

EACTIVITY_RET ActivityNodeKill::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  xPos pos;
  SceneNpc* pNpc = pActivity->getSceneNpc(m_oParams.getTableInt("id"));
  if (pNpc)
  {
    if (pNpc->getNpcID() == 30025 || pNpc->getNpcID() == 30039)
      pNpc->setDeadEffect();

    const SNpcCFG* pCFG = pNpc->getCFG();
    if (pCFG != nullptr && pCFG->eNpcType == ENPCTYPE_NPC)
      pNpc->removeAtonce();
    else
      pNpc->setClearState();
    XLOG << "[活动-执行-节点] " << getName() << " kill npc 成功" << pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID() << XEND;
  }
  return EACTIVITY_RET_OK;
}

/************************************************************************/
/*ActivityNodeSceneGm                                                                      */
/************************************************************************/

EACTIVITY_RET ActivityNodeSceneGm::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;
   
  if (GMCommandRuler::getMe().scene_execute(pScene, m_oParams) == false)
  {
    XERR << "[活动-执行-节点] " << getName() << "执行gm, 失败" << pActivity->getUid() << getName() << XEND;
    return EACTIVITY_RET_OK;
  }  

  XLOG << "[活动-执行-节点] " << getName() << "执行gm，成功" << pActivity->getUid() << getName() << XEND;
  return EACTIVITY_RET_OK;
}

/************************************************************************/
/**ActivityNodeNpcGm*/
/************************************************************************/

EACTIVITY_RET ActivityNodeNpcGm::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (!pActivity)
    return EACTIVITY_RET_OK;
  DWORD npcid = m_oParams.getTableInt("npcids");

  const TSetQWORD& setguids = pActivity->getNpcGUIDS(npcid);
  for (auto &s : setguids)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s);
    if (npc == nullptr)
      continue;

    if (GMCommandRuler::getMe().execute(npc, m_oParams) == false)
    {
      XERR << "[活动-执行-节点] " << getName() << "执行gm, 失败" << pActivity->getUid() << getName() << XEND;
      return EACTIVITY_RET_OK;
    }
  }

  XLOG << "[活动-执行-节点] " << getName() << "执行gm，成功" << pActivity->getUid() << getName() << npcid << setguids.size() << XEND;
  return EACTIVITY_RET_OK;
}

// 金钱攻击
ActivityNodeMoneyHit::ActivityNodeMoneyHit(const xLuaData& params) : ActivityNodeBase(params)
{
  m_dwNpcID = params.getTableInt("npcid");
  m_dwRange = params.getTableInt("range");
  m_dwSkillID = params.getTableInt("skill");
  m_dwLayerBuff = params.getTableInt("layer_buff");
  m_dwLayerLimit = params.getTableInt("layer_limit");
  m_dwStep = params.getTableInt("step");
  m_dwDizzyBuff = params.getTableInt("dizzy_buff");
  m_dwMsgID = params.getTableInt("msgid");
  auto getpres = [&](const string& key, xLuaData& data)
  {
    m_setExpressions.insert(data.getInt());
  };
  m_oParams.getMutableData("expression").foreach(getpres);
}

EACTIVITY_RET ActivityNodeMoneyHit::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;
  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  SceneNpc* pNpc = pActivity->getSceneNpc(m_dwNpcID);
  if (pNpc == nullptr)
    return EACTIVITY_RET_OK;

  ActivityMoneyCat* pMoneyCat = dynamic_cast<ActivityMoneyCat*> (pActivity);
  if (pMoneyCat == nullptr)
    return EACTIVITY_RET_OK;

  if (pMoneyCat->checkMoneyEnd())
  {
    pActivity->stop();
    return EACTIVITY_RET_OK;
  }

  xSceneEntrySet userset;
  pScene->getEntryListInBlock(SCENE_ENTRY_USER, pNpc->getPos(), m_dwRange, userset);
  if (userset.empty())
    return EACTIVITY_RET_OK;

  for (auto s = userset.begin(); s != userset.end(); )
  {
    SceneUser* user = dynamic_cast<SceneUser*> (*s);
    if (user == nullptr || user->m_oBuff.haveBuff(m_dwDizzyBuff))
    {
      s = userset.erase(s);
      continue;
    }
    DWORD userpre = user->getExpression();
    if (!m_setExpressions.empty() && m_setExpressions.find(userpre) == m_setExpressions.end())
    {
      s = userset.erase(s);
      continue;
    }
    DWORD layer = user->m_oBuff.getLayerByID(m_dwLayerBuff);
    if (layer >= m_dwLayerLimit)
    {
      s = userset.erase(s);
      continue;
    }
    ++s;
  }

  if (userset.empty())
    return EACTIVITY_RET_OK;

  xSceneEntry** ppEntry = randomStlContainer(userset);
  if (ppEntry == nullptr)
    return EACTIVITY_RET_OK;
  SceneUser* findUser = dynamic_cast<SceneUser*> (*ppEntry);
  if (findUser == nullptr)
    return EACTIVITY_RET_OK;

  findUser->m_oBuff.addLayers(m_dwLayerBuff, 1);
  findUser->setExpression(0);

  const SMoneyCatCFG& rCFG = MiscConfig::getMe().getMoneyCatCFG();
  DWORD maxmoney = rCFG.dwMaxMoney;
  DWORD getmoney = rCFG.getRandMoney(m_dwStep);
  DWORD nowmoney = pMoneyCat->getMoney();
  getmoney = getmoney > maxmoney - nowmoney? maxmoney - nowmoney : getmoney;
  pMoneyCat->setMoney(getmoney + nowmoney);

  findUser->addMoney(EMONEYTYPE_SILVER, getmoney, ESOURCE_MONEYCAT);
  pMoneyCat->addUserMoney(findUser->name, getmoney);
  XLOG << "[招财猫],猫:" << pNpc->id << "地图:" << pScene->name << pScene->getMapID() << "砸中玩家" << findUser->name << findUser->id << "获得zeny:" << getmoney << "当前已发放zeny:" << getmoney + nowmoney << XEND;

  // 表现砸中效果
  //pNpc->useSkill(m_dwSkillID, findUser->id, findUser->getPos());
  SkillBroadcastUserCmd cmd;
  cmd.set_skillid(m_dwSkillID);
  cmd.set_charid(pNpc->id);
  Cmd::PhaseData *pData = cmd.mutable_data();
  if (pData)
  {
    pData->set_number(ESKILLNUMBER_ATTACK);
    HitedTarget* pHit = pData->add_hitedtargets();
    if (pHit)
    {
      pHit->set_type(DAMAGE_TYPE_CRITICAL);
      pHit->set_charid(findUser->id);
      pHit->set_damage(getmoney);
    }
  }
  PROTOBUF(cmd, send, len);
  pNpc->sendCmdToNine(send, len);

  findUser->m_oBuff.add(m_dwDizzyBuff);

  if (m_dwMsgID)
    MsgManager::sendMsg(findUser->id, m_dwMsgID, MsgParams(getmoney));

  return EACTIVITY_RET_OK;
}

// normal summon monster
ActivityNodeNormalSummon::ActivityNodeNormalSummon(const xLuaData& params) : ActivityNodeBase(params)
{
  auto getnpcs = [&](const string& key, xLuaData& data)
  {
    DWORD map = data.getTableInt("mapid");
    if (map == 0)
      return;
    std::pair<NpcDefine, DWORD>& npc2num = m_mapMapID2Npc[map];
    npc2num.first.load(data);
    npc2num.second = 1;
    if (data.has("num"))
      npc2num.second = data.getTableInt("num");
  };
  m_oParams.foreach(getnpcs);
}

EACTIVITY_RET ActivityNodeNormalSummon::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;

  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  auto it = m_mapMapID2Npc.find(pScene->getMapID());
  if (it == m_mapMapID2Npc.end())
    return EACTIVITY_RET_OK;

  for (DWORD i = 0; i < it->second.second; ++i)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(it->second.first, pScene);
    if (pNpc)
    {
      pActivity->onAddNpc(pNpc);
      XLOG << "[活动-执行-节点] 召唤npc 成功" << pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID() << pNpc->name << XEND;
    }
  }
  return EACTIVITY_RET_OK;
}

ActivitySupplySummon::ActivitySupplySummon(const xLuaData& params) : ActivityNodeNormalSummon(params)
{
  auto getlimit = [&](const string& key, xLuaData& data)
  {
    DWORD map = data.getTableInt("mapid");
    if (map == 0)
      return;
    DWORD limitcnt = data.getTableInt("limit_count");
    m_mapMap2LimitCnt[map] = limitcnt;
  };
  m_oParams.foreach(getlimit);
}

EACTIVITY_RET ActivitySupplySummon::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;

  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  auto it = m_mapMapID2Npc.find(pScene->getMapID());
  if (it == m_mapMapID2Npc.end())
    return EACTIVITY_RET_OK;
  auto itl = m_mapMap2LimitCnt.find(pScene->getMapID());
  if (itl == m_mapMap2LimitCnt.end())
    return EACTIVITY_RET_OK;

  DWORD nowcnt = pActivity->getNpcCount(it->second.first.getID()); // 当前存在数量
  DWORD limitcnt = itl->second; // 存在数量上限
  if (nowcnt >= limitcnt)
    return EACTIVITY_RET_OK;

  DWORD count = it->second.second; //配置一次召唤次数
  count = count <= limitcnt - nowcnt ? count : limitcnt - nowcnt;

  for (DWORD i = 0; i < count; ++i)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(it->second.first, pScene);
    if (pNpc)
    {
      pActivity->onAddNpc(pNpc);
      XLOG << "[活动-执行-节点] 召唤npc 成功" << pActivity->getUid() << getName() << pNpc->getNpcID() << pNpc->getTempID() << pNpc->name << XEND;
    }
  }

  return EACTIVITY_RET_OK;
}

ActivityNodeTimeSummon::ActivityNodeTimeSummon(const xLuaData& params) : ActivityNodeNormalSummon(params)
{
  auto getlimit = [&](const string& key, xLuaData& data)
  {
    DWORD map = data.getTableInt("mapid");
    if (map == 0)
      return;
    if (data.has("week"))
    {
      DWORD week = data.getTableInt("week");
      m_mapMap2WeekLimit[map] = week;
    }
  };
  m_oParams.foreach(getlimit);
  m_dwWeekLoopCnt = m_oParams.getTableInt("week_loop");
}

EACTIVITY_RET ActivityNodeTimeSummon::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;

  Scene* pScene = pActivity->getScene();
  if (pScene == nullptr)
    return EACTIVITY_RET_OK;

  auto it = m_mapMap2WeekLimit.find(pScene->getMapID());
  if (it == m_mapMap2WeekLimit.end())
    return EACTIVITY_RET_OK;

  if (m_dwWeekLoopCnt == 0)
    return EACTIVITY_RET_OK;

  DWORD needweek = it->second;

  DWORD weekstart = xTime::getWeekStart(curSec, 3600 * 5);
  DWORD weekcnt = weekstart / (3600 * 24 * 7);
  weekcnt %= m_dwWeekLoopCnt;

  if (needweek != weekcnt)
    return EACTIVITY_RET_OK;

  return ActivityNodeNormalSummon::v_run(curSec, pActivity);
}

ActivityNodeNotifyMoneyHit::ActivityNodeNotifyMoneyHit(const xLuaData& params) : ActivityNodeBase(params)
{
  m_dwMsgID = params.getTableInt("msgid");
}

EACTIVITY_RET ActivityNodeNotifyMoneyHit::v_run(DWORD curSec, ActivityBase* pActivity)
{
  ActivityMoneyCat* pMoneyCat = dynamic_cast<ActivityMoneyCat*> (pActivity);
  if (!pMoneyCat)
    return EACTIVITY_RET_OK;
  Scene* pScene = pMoneyCat->getScene();
  if (!pScene)
    return EACTIVITY_RET_OK;

  string maxname = pMoneyCat->getMaxMoneyUser();
  DWORD maxmoney = pMoneyCat->getUserMoney(maxname);

  XLOG << "[招财猫], 地图:" << pScene->name << pScene->getMapID() << "最大玩家:" << maxname << "money:" << maxmoney << XEND;

  MsgParams param;
  param.addString(maxname);
  param.addString(pScene->name);
  param.addNumber(maxmoney);
  MsgManager::sendMapMsg(pScene->getMapID(), m_dwMsgID, param);

  return EACTIVITY_RET_OK;
}

ActivityCheckNpcDie::ActivityCheckNpcDie(const xLuaData& params) : ActivityNodeBase(params)
{
  auto getid = [&](const string& key, xLuaData& d)
  {
    m_setNpcIDs.insert(d.getInt());
  };
  m_oParams.getMutableData("npc_id").foreach(getid);
}

EACTIVITY_RET ActivityCheckNpcDie::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;
  bool findnpc = false;
  for (auto &v : m_setNpcIDs)
  {
    if (pActivity->checkHaveNpc(v))
    {
      findnpc = true;
      break;
    }
  }
  return findnpc ? EACTIVITY_RET_CONTINUE: EACTIVITY_RET_OK;
}

ActivityCheckNpcNum::ActivityCheckNpcNum(const xLuaData& params) : ActivityNodeBase(params)
{
  m_dwNpcID = params.getTableInt("npc_id");
  m_dwNum = params.getTableInt("num");
}

EACTIVITY_RET ActivityCheckNpcNum::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;
  DWORD num = pActivity->getNpcCount(m_dwNpcID);
  if(num > m_dwNum)
    return EACTIVITY_RET_CONTINUE;
  else
    return EACTIVITY_RET_OK;
}

ActivityCheckStage::ActivityCheckStage(const xLuaData& params) : ActivityNodeBase(params)
{
  auto getstage = [&](const string& key, xLuaData& d)
  {
    m_setStages.insert(d.getInt());
  };
  m_oParams.getMutableData("stage").foreach(getstage);
}

EACTIVITY_RET ActivityCheckStage::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;
  for (auto &v : m_setStages)
  {
    if (pActivity->checkCondionStageOver(v) == false)
      return EACTIVITY_RET_CONTINUE;
  }
  return EACTIVITY_RET_OK;
}

ActivityHaveBuff::ActivityHaveBuff(const xLuaData& params) : ActivityNodeBase(params)
{
  auto getid = [&](const string& key, xLuaData& d)
  {
    m_setNpcIDs.insert(d.getInt());
  };
  m_oParams.getMutableData("npc_id").foreach(getid);
  m_dwBuffID = m_oParams.getTableInt("buff");
}

EACTIVITY_RET ActivityHaveBuff::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;

  bool allhavebuff = true;
  for (auto &v : m_setNpcIDs)
  {
    const TSetQWORD& setguids = pActivity->getNpcGUIDS(v);
    for (auto &s : setguids)
    {
      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s);
      if (npc == nullptr)
        continue;
      if (npc->m_oBuff.haveBuff(m_dwBuffID) == false)
      {
        allhavebuff = false;
        break;
      }
    }
  }
  return allhavebuff ? EACTIVITY_RET_OK : EACTIVITY_RET_CONTINUE;
}

ActivityMarkRecordNpc::ActivityMarkRecordNpc(const xLuaData& params) : ActivityNodeBase(params)
{
  auto getid = [&](const string& key, xLuaData& d)
  {
    m_setNpcIDs.insert(d.getInt());
  };
  m_oParams.getMutableData("npc_id").foreach(getid);
}

EACTIVITY_RET ActivityMarkRecordNpc::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;

  for (auto &s : m_setNpcIDs)
  {
    pActivity->markRecordMonster(s);
  }

  return EACTIVITY_RET_OK;
}

ActivityKillCnt::ActivityKillCnt(const xLuaData& params) : ActivityNodeBase(params)
{
  m_dwNpcID = m_oParams.getTableInt("id");
  m_dwCnt = m_oParams.getTableInt("count");
}

EACTIVITY_RET ActivityKillCnt::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;

  if (pActivity->getKillCount(m_dwNpcID) >= m_dwCnt)
  {
    pActivity->resetKillCount(m_dwNpcID);
    return EACTIVITY_RET_OK;
  }

  return EACTIVITY_RET_CONTINUE;
}

ActivityPlayDialog::ActivityPlayDialog(const xLuaData& params) : ActivityNodeBase(params)
{

}

EACTIVITY_RET ActivityPlayDialog::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;
  if (pActivity->getScene() == nullptr)
    return EACTIVITY_RET_OK;

  DWORD dialogid = m_oParams.getTableInt("dialogid");
  if (TableManager::getMe().getDialogCFG(dialogid) == nullptr)
  {
    XERR << "[Activity-playdialog], dialogid = " << dialogid << "非法" << XEND;
    return EACTIVITY_RET_OK;
  }

  UserActionNtf cmd;
  cmd.set_type(EUSERACTIONTYPE_DIALOG);
  cmd.set_value(dialogid);
  if (m_oParams.has("center_pos") && m_oParams.has("range"))
  {
    float range = m_oParams.getTableFloat("range");
    xPos pos;
    pos.x = m_oParams.getMutableData("center_pos").getTableFloat("1");
    pos.y = m_oParams.getMutableData("center_pos").getTableFloat("2");
    pos.z = m_oParams.getMutableData("center_pos").getTableFloat("3");

    xSceneEntrySet userset;
    pActivity->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, pos, range, userset);
    for (auto &s : userset)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (user == nullptr)
        continue;
      cmd.set_charid(s->id);
      PROTOBUF(cmd, send, len);
      user->sendCmdToMe(send, len);
    }
  }

  else if (m_oParams.getTableInt("map") ==1)
  {
    xSceneEntrySet userset;
    pActivity->getScene()->getAllEntryList(SCENE_ENTRY_USER, userset);
    for (auto &s : userset)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (user == nullptr)
        continue;
      cmd.set_charid(s->id);
      PROTOBUF(cmd, send, len);
      user->sendCmdToMe(send, len);
    }
  }

  return EACTIVITY_RET_OK;
}

ActivitySpecialEffect::ActivitySpecialEffect(const xLuaData& params) : ActivityNodeBase(params)
{
  m_dwDramaID = m_oParams.getTableInt("dramaid");
  m_dwOpen = m_oParams.getTableInt("open");
}

EACTIVITY_RET ActivitySpecialEffect::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;
  if (pActivity->getScene() == nullptr)
    return EACTIVITY_RET_OK;

  if(m_dwOpen == 0)
    pActivity->delSpecialEffect(m_dwDramaID);
  else
  {
    DWORD starttime = now();
    pActivity->addSpecialEffect(m_dwDramaID, starttime);
    Scene* pScene = pActivity->getScene();
    if (pScene == nullptr)
      return EACTIVITY_RET_OK;

    SpecialEffectCmd cmd;
    cmd.set_dramaid(m_dwDramaID);
    cmd.set_starttime(starttime);
    PROTOBUF(cmd, send, len);
    pScene->sendCmdToAll(send, len);
  }

  return EACTIVITY_RET_OK;
}

ActivityCheckNpcHp::ActivityCheckNpcHp(const xLuaData& params) : ActivityNodeBase(params)
{
  m_dwNpcID = m_oParams.getTableInt("npc_id");
}

EACTIVITY_RET ActivityCheckNpcHp::v_run(DWORD curSec, ActivityBase* pActivity)
{
  if (pActivity == nullptr)
    return EACTIVITY_RET_OK;
  const TSetQWORD& guid = pActivity->getNpcGUIDS(m_dwNpcID);
  if (guid.empty())
    return EACTIVITY_RET_OK;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(*(guid.begin()));
  if (npc == nullptr)
    return EACTIVITY_RET_OK;

  float maxhp = npc->getAttr(EATTRTYPE_MAXHP);
  float per = maxhp ? (npc->getAttr(EATTRTYPE_HP)) / maxhp : 0;
  if (m_oParams.has("hp_less"))
  {
    float lessper = m_oParams.getTableFloat("hp_less");
    if (per > lessper)
      return EACTIVITY_RET_CONTINUE;
  }
  if (m_oParams.has("hp_more"))
  {
    float moreper = m_oParams.getTableFloat("hp_more");
    if (per < moreper)
      return EACTIVITY_RET_CONTINUE;
  }
  return EACTIVITY_RET_OK;
}

