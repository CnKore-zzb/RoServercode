#include "AIAction.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "GMCommandRuler.h"
#include "SkillManager.h"
#include "SkillItem.h"
#include "UserBeing.h"
#include "SceneUser.h"
#include "SceneUserManager.h"

//const char LOG_NAME[] = "AIAction";

// AIAction - talk
TalkAIAction::TalkAIAction(const xLuaData& params) : AIAction(params)
{
  talkId = m_oParams.getTableInt("dialog");
}

void TalkAIAction::doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() /*|| !selectByPercent(m_dwPercent)*/) return;

  TalkInfo cmd;
  cmd.set_guid(pNpc->id);
  cmd.set_talkid(talkId);
  PROTOBUF(cmd, send, len);
  pNpc->sendCmdToNine(send, len);
}

// AIAction - summon
SummonNpcAIAction::SummonNpcAIAction(const xLuaData& params) : AIAction(params)
{
  m_oNpcDef.load(m_oParams);
  m_oNpcDef.setSuperAiNpc(true);
  m_bSupply = (m_oParams.getTableInt("supply") != 0);
  m_dwNum = m_oParams.getTableInt("num");
  if (!m_dwNum)
    m_dwNum = 1;
}

void SummonNpcAIAction::doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() /*|| !selectByPercent(m_dwPercent)*/) return;
  bool bFromLv = m_oParams.getTableInt("samelv") == 1;
  bool priAtk = m_oParams.getTableInt("pri_attack_owner") == 1;
  bool sharedam = m_oParams.getTableInt("sharedam") == 1;
  bool saperateLock = m_oParams.getTableInt("saperate_lock") == 1;
  QWORD priAtkUser = priAtk ? pNpc->m_ai.getLastAttacker() : 0;
  bool bTargetPos = m_oParams.getTableInt("target_pos") == 1;
  TVecDWORD vecIDs;
  if (m_oParams.has("ids"))
  {
    m_oParams.getMutableData("ids").getIDList(vecIDs);
    //m_oNpcDef.setID();
  }
  else 
  {
    vecIDs.push_back(m_oNpcDef.getID());
  }

  m_oNpcDef.setID(vecIDs[randBetween(0,vecIDs.size()-1)]);
  DWORD num = m_dwNum;

  if (m_bSupply)
  { 
    do
    {
      //m_oNpcDef.setID(vecIDs[randBetween(0,vecIDs.size()-1)]);
      DWORD randIndex = randBetween(0,vecIDs.size()-1);
      m_oNpcDef.setID(vecIDs[randIndex]);
      num = m_dwNum;

      const TVecServantData& vecServants = pNpc->m_oFollower.getServant();

      for (auto &v : vecServants)
      {
        if (v.bSupply == false)
          continue;
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v.qwTempID);
        if (npc == nullptr || npc->getNpcID() != m_oNpcDef.getID())
          continue;
        num--;
        if(!num)
        {
          vecIDs.erase(vecIDs.begin() + randIndex);
          break;
        }
      }
    } while (!vecIDs.empty() && !num);

    if (vecIDs.empty())
      return;
  }

  if(bTargetPos)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*targetSet.begin());
    if (pEntry == nullptr)
      return;
    m_oNpcDef.setPos(pEntry->getPos());
  }
  else
  {
    m_oNpcDef.setPos(pNpc->getPos());
  }

  m_oNpcDef.setBehaviours(m_oNpcDef.getBehaviours() & ~BEHAVIOUR_OUT_RANGE_BACK);
  m_oNpcDef.m_oVar.dwLayer = pNpc->getEndlessLayer();
  if (sharedam)
    m_oNpcDef.m_oVar.m_dwDefaultHp = pNpc->getAttr(EATTRTYPE_HP);
  if (bFromLv) m_oNpcDef.setLevel(pNpc->getLevel());

  SServantData data;
  data.bShapreDam = sharedam;
  data.bSupply = m_bSupply;
  data.bSaperateLock = saperateLock;
  pNpc->m_oFollower.addServant(m_oNpcDef, data, num, priAtkUser);
  //xPos p = pNpc->getPos();
  //m_oNpcDef.setPos(p);
  //m_oNpcDef.setBehaviours(m_oNpcDef.getBehaviours() & ~BEHAVIOUR_OUT_RANGE_BACK);
  //pNpc->m_oFollower.addServant(m_oNpcDef, m_bSupply, m_dwNum);
}

// AIAction - gm
void GMCmdAIAction::doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || targetSet.empty())
    return;
  if (m_oParams.getTableInt("self_ai_action") == 1)
  {
    if (m_oParams.has("id1"))
    {
      for (auto &s : targetSet)
      {
        m_oParams.setData("id1", s->id);
        GMCommandRuler::getMe().execute((xSceneEntryDynamic*)pNpc, m_oParams);
      }
    }
    else
    {
      GMCommandRuler::getMe().execute((xSceneEntryDynamic*)pNpc, m_oParams);
    }
    return;
  }
  for (auto m = targetSet.begin(); m != targetSet.end(); ++m)
  {
    if ((*m) /*&& selectByPercent(m_dwPercent)*/)
    {
      m_oParams.setData("GM_ESource", (DWORD)ESOURCE_PICKUP);
      GMCommandRuler::getMe().execute((xSceneEntryDynamic *)(*m), m_oParams);
    }
  }
}

// AIAction - skill
void SkillAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc || targetSet.empty() /*|| !selectByPercent(m_dwPercent)*/)
    return;

  if (targetSet.empty())
    return;

  xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*targetSet.begin());
  if (pEntry == nullptr)
    return;
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(m_dwSkillID);
  if (pSkillCFG == nullptr)
    return;

  float skillidst = pSkillCFG->getLaunchRange(pNpc);
  if (skillidst != 0 && getDistance(pNpc->getPos(), pEntry->getPos()) > skillidst + 0.5)
    return;

  // 死亡时瞬发的技能
  if (pNpc->isAlive() == false && pSkillCFG->getReadyTime(pNpc) == 0)
  {
    pNpc->m_oSkillProcessor.runSkillAtonce(pEntry, m_dwSkillID);
  }
  // 正常释放
  else
  {
    // 陷阱类技能无锁定目标时, 随机位置
    xPos skillpos = pEntry->getPos();
    if (pSkillCFG->getSkillCamp() == ESKILLCAMP_ENEMY && pEntry->id == pNpc->id && pNpc->getScene())
    {
      pNpc->getScene()->getRandPos(pEntry->getPos(), pSkillCFG->getLaunchRange(pNpc), skillpos);
    }
    pNpc->useSuperSkill(m_dwSkillID, pEntry->id, skillpos, true);
  }
}

// AIAction - skill me
void SkillMeAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc || targetSet.empty() /*|| !selectByPercent(m_dwPercent)*/)
    return;

  if (targetSet.empty())
    return;

  SceneNpc* pTarNpc = dynamic_cast<SceneNpc*> (*targetSet.begin());
  if (!pTarNpc)
    return;
  pTarNpc->useSuperSkill(m_dwSkillID, pNpc->id, pNpc->getPos());
}

// AIAction - route
RouteAIAction::RouteAIAction(const xLuaData& params) : AIAction(params)
{
  xLuaData& pos = m_oParams.getMutableData("pos");
  auto posf = [this](const string& key, xLuaData& data)
  {
    xPos p;
    p.x = data.getTableInt("1");
    p.y = data.getTableInt("2");
    p.z = data.getTableInt("3");
    m_vecPos.push_back(p);
  };
  pos.foreach(posf);
}

RouteAIAction::~RouteAIAction()
{

}

void RouteAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (pNpc == nullptr || pNpc->m_oMove.empty() == false || pNpc->getScene() == nullptr)
    return;
  if (m_vecPos.empty() == true)
    return;

  if (m_vecPos.size() > m_dwIndex)
  {
    pNpc->m_ai.moveTo(m_vecPos[m_dwIndex]);
  }

  if (m_bChange)
    ++m_dwIndex;
  else
    --m_dwIndex;

  string mode = m_oParams.getTableString("mode");
  if (mode == "circle")
  {
    if (m_dwIndex >= m_vecPos.size() - 1)
    {
      m_bChange = true;
      m_dwIndex = 0;
    }
  }
  else if (mode == "pingpong" || m_oParams.has("mode") == false)
  {
    if (m_dwIndex >= m_vecPos.size() - 1)
      m_bChange = false;
    else if (m_dwIndex <= 0)
      m_bChange = true;
  }
}

SupplyAIAction::SupplyAIAction(const xLuaData& params) : AIAction(params)
{

}

SupplyAIAction::~SupplyAIAction()
{

}

void SupplyAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->isAlive() || !pNpc->getScene() || targetSet.empty())
    return;
  if(pNpc->m_oFollower.hasServant() == false)
    return;

  /*
  std::vector<std::pair<QWORD, bool>> vecServants;
  pNpc->m_oFollower.getServant(vecServants);
  for (auto v = vecServants.begin(); v != vecServants.end(); ++v)
  {
    if ((*v).second == false)
      continue;
    SceneNpc* pServant = SceneNpcManager::getMe().getNpcByTempID((*v).first);
    if (pServant == nullptr)
      continue;
    if (pServant->canRelive())
    {
      //pServant->setReliveCount(1);
      pServant->setStatus(ECREATURESTATUS_RELIVE);
      // 设置出生位置
      pServant->define.setPos(pNpc->getPos());
      if (pNpc->m_ai.getCurLockID())
        pServant->m_ai.setCurLockID(pNpc->m_ai.getCurLockID());
    }
  }
  */
}

AttackAIAction::AttackAIAction(const xLuaData& params) : AIAction(params)
{
}

AttackAIAction::~AttackAIAction()
{
}

void AttackAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() || targetSet.empty())
    return;

  for (auto it = targetSet.begin(); it != targetSet.end(); ++it)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*it);
    if (pEntry)
      pNpc->m_ai.putAttacker(pEntry);
  }
}

// AIAction -  getaway 逃离目标
GetawayAIAction::GetawayAIAction(const xLuaData& params) : AIAction(params)
{
  m_fDistance = params.getTableFloat("distance");
}

GetawayAIAction::~GetawayAIAction()
{
}

void GetawayAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() || targetSet.empty())
    return;

  for (auto it = targetSet.begin(); it != targetSet.end(); ++it)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*it);
    if (pEntry)
    {
      xPos newPos;
      if (pEntry == pNpc)
      {
        if (pNpc->getScene()->getRandTargetPos(pNpc->getPos(), m_fDistance, newPos) == false)
          break;
      }
      else
      {
        xPos otherPos = pEntry->getPos();
        xPos myOldPos = pNpc->getPos();  
        newPos = getPosByDir(otherPos, myOldPos, m_fDistance);
      }

      pNpc->m_ai.setCurLockID(0);
      pNpc->m_ai.moveTo(newPos);
      pNpc->m_ai.changeState(ENPCSTATE_RUNAWAY);
      break;
    }
  }
}

// AIAction -  buff 获得buff
BuffAIAction::BuffAIAction(const xLuaData& params) : AIAction(params)
{
  m_dwBuffId = params.getTableInt("buffid");
}

BuffAIAction::~BuffAIAction()
{
}

void BuffAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() || targetSet.empty())
    return;

  for (auto it = targetSet.begin(); it != targetSet.end(); ++it)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*it);
    if (!pEntry)continue;
    
    pEntry->m_oBuff.add(m_dwBuffId, pNpc);
  }
}

TriggerCondAIAction::TriggerCondAIAction(const xLuaData& params) : AIAction(params)
{
  m_cond = params.getTableString("cond");
}

TriggerCondAIAction::~TriggerCondAIAction()
{
}

void TriggerCondAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() || targetSet.empty())
    return;

  for (auto it = targetSet.begin(); it != targetSet.end(); ++it)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(*it);
    if (!pNpc) continue;
    pNpc->m_sai.checkSig(m_cond);
  }
}

// AIAction -  expression 使用表情
ExpressionAIAction::ExpressionAIAction(const xLuaData& params) : AIAction(params)
{
  m_strExpression = params.getTableString("expression");
}

ExpressionAIAction::~ExpressionAIAction()
{
}

void ExpressionAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() || targetSet.empty())
    return;

  for (auto it = targetSet.begin(); it != targetSet.end(); ++it)
  {
    xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*it);
    if (!pEntry)
      continue;

    pEntry->checkEmoji(m_strExpression.c_str());

  }
}
// AIAction -  changeai 改变AI
ChangeAIAction::ChangeAIAction(const xLuaData& params) : AIAction(params)
{
  xLuaData& luaData = m_oParams.getMutableData("ai");
  auto func = [this](const string& key, xLuaData& data)
  {
    m_setAI.insert(data.getInt());
  };
  luaData.foreach(func);
}

ChangeAIAction::~ChangeAIAction()
{
}

void ChangeAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet)
{
  if (!pNpc || !pNpc->getScene() || targetSet.empty())
    return;

  for (auto it = targetSet.begin(); it != targetSet.end(); ++it)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(*it);
    if (!pNpc)
      continue;
    pNpc->m_sai.changeAI(m_setAI);    
  }
}

// AIAction - go to pos
GoPosAIAction::GoPosAIAction(const xLuaData& params) : AIAction(params)
{
  xLuaData& luaData = m_oParams.getMutableData("pos");
  m_destPos.x = luaData.getTableInt("1");
  m_destPos.y = luaData.getTableInt("2");
  m_destPos.z = luaData.getTableInt("3");
}

GoPosAIAction::~GoPosAIAction()
{

}

void GoPosAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc || !pNpc->getScene() || !pNpc->isAlive())
    return;
  pNpc->m_ai.moveTo(m_destPos);
}

// AIAction - turn monster
TurnMonsterAIAction::TurnMonsterAIAction(const xLuaData& params) : AIAction(params)
{
  m_oNpcDef.load(m_oParams);
}

TurnMonsterAIAction::~TurnMonsterAIAction()
{

}

void TurnMonsterAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc)
    return;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_oNpcDef.getID());
  if (pCFG == nullptr)
    return;
  pNpc->evo(pCFG, m_oNpcDef);
  //pNpc->turnNewMonster(m_oNpcDef);
}

// AIAction - change target
ChangeTargetAIAction::ChangeTargetAIAction(const xLuaData& params) : AIAction(params)
{

}

ChangeTargetAIAction::~ChangeTargetAIAction()
{

}

void ChangeTargetAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc)
    return;

  pNpc->m_ai.changeTarget();
}

// AIAction - relive dead
ReliveDeadAIAction::ReliveDeadAIAction(const xLuaData& params) : AIAction(params)
{

}

ReliveDeadAIAction::~ReliveDeadAIAction()
{

}

void ReliveDeadAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return;
  for (auto &s : targetSet)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
    if (npc == nullptr || npc->isAlive())
      continue;
    if (npc->canRelive())
    {
      npc->setStatus(ECREATURESTATUS_RELIVE);
    }
    npc->setReliveAtOldPos();
    break;
  }
}

// AIAction - 消失
SetClearAIAction::SetClearAIAction(const xLuaData& params) : AIAction(params)
{

}

SetClearAIAction::~SetClearAIAction()
{

}

void SetClearAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return;

  if (pNpc->getDefine().getLife() == 0)
    return;

  for (auto &s : targetSet)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
    if (npc == nullptr || npc->getScene() == nullptr)
      continue;
    npc->setStatus(ECREATURESTATUS_LEAVE);
  }
}

ComeToMeAIAction::ComeToMeAIAction(const xLuaData& params) : AIAction(params)
{
  m_fDistance = params.getTableFloat("distance");
}

ComeToMeAIAction::~ComeToMeAIAction()
{

}

void ComeToMeAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return;

  for (auto &s : targetSet)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
    if (!npc)
      continue;
    if (getDistance(npc->getPos(), pNpc->getPos()) <= m_fDistance)
      continue;
    xPos pos;
    pNpc->getScene()->getRandPos(pNpc->getPos(), m_fDistance, pos);
    npc->m_ai.moveTo(pos);
  }
}

RewardMapAIAction::RewardMapAIAction(const xLuaData& params) : AIAction(params)
{
  auto getrwd = [&](const string& key, xLuaData& d)
  {
    SAIRewardData stData;
    stData.dwMinUserNum = d.getTableInt("minuser");
    stData.dwMaxUserNum = d.getTableInt("maxuser");
    auto getids = [&](const string& keyer, xLuaData& der)
    {
      stData.setRewards.insert(der.getInt());
    };
    d.getMutableData("id").foreach(getids);
    m_vecRewards.push_back(stData);
  };
  m_oParams.getMutableData("reward").foreach(getrwd);
  m_fDropRange = m_oParams.getTableFloat("drop_range");
  m_fUserRange = m_oParams.getTableInt("user_range");
}

RewardMapAIAction::~RewardMapAIAction()
{

}

void RewardMapAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return;

  xSceneEntrySet userset;
  pNpc->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, pNpc->getPos(), m_fUserRange, userset);
  DWORD size = userset.size();

  auto it = find_if(m_vecRewards.begin(), m_vecRewards.end(), [&](const SAIRewardData& r) ->bool{
    return r.dwMinUserNum <= size && r.dwMaxUserNum >= size;
  });

  if (it == m_vecRewards.end())
  {
    XERR << "[AI-RewardMap], 根据玩家数量:" << size << "找不到奖励配置, 怪物:" << pNpc->name << pNpc->id << XEND;
    return;
  }

  xLuaData gmdata;
  gmdata.setData("type", "dropreward");
  gmdata.setData("range", m_fDropRange);
  for (auto &s : it->setRewards)
  {
    gmdata.setData("id", s);
    XLOG << "[AI-RewardMap], 怪物:" << pNpc->name << pNpc->id << "掉落奖励ID:" << s << XEND;

    GMCommandRuler::getMe().execute(pNpc, gmdata);
  }
}

// AIAction - being_skill
void BeingSkillAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc || targetSet.empty())
    return;

  xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*targetSet.begin());
  if (pEntry == nullptr)
    return;

  BeingNpc* beingNpc = dynamic_cast<BeingNpc*>(pNpc);
  if (beingNpc == nullptr)
    return;
  SceneUser* master = beingNpc->getMasterUser();
  if (master == nullptr)
    return;
  SSceneBeingData* being = master->getUserBeing().getBeingData(pNpc->getDefine().getID());
  if (being == nullptr || being->bBattle == false)
    return;
  SBeingSkillData* skill = being->getSkillByGroupID(m_dwSkillGroupID);
  if (skill == nullptr || skill->dwPos <= 0)
    return;

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skill->dwID);
  if (pSkillCFG == nullptr)
    return;
  float skillidst = pSkillCFG->getLaunchRange(pNpc);
  if (skillidst != 0 && getDistance(pNpc->getPos(), pEntry->getPos()) > skillidst + 0.5)
    return;

  // 死亡时瞬发的技能
  if (pNpc->isAlive() == false && pSkillCFG->getReadyTime(pNpc) == 0)
  {
    pNpc->m_oSkillProcessor.runSkillAtonce(pEntry, skill->dwID);
  }
  // 正常释放
  else
  {
    // 陷阱类技能无锁定目标时, 随机位置
    xPos skillpos = pEntry->getPos();
    if (pSkillCFG->getSkillCamp() == ESKILLCAMP_ENEMY && pEntry->id == pNpc->id && pNpc->getScene())
    {
      pNpc->getScene()->getRandPos(pEntry->getPos(), pSkillCFG->getLaunchRange(pNpc), skillpos);
    }
    pNpc->useSuperSkill(skill->dwID, pEntry->id, skillpos, true);
  }
}

LockTargetAIAction::LockTargetAIAction(const xLuaData& params) : AIAction(params)
{
}

void LockTargetAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr || targetSet.empty())
    return;

  for (auto& s : targetSet)
  {
    xSceneEntryDynamic* entry = dynamic_cast<xSceneEntryDynamic*>(s);
    if (entry && entry->isAlive())
    {
      pNpc->m_ai.setCurLockID(s->id);
      return;
    }
  }
}

void TargetSkillAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc || targetSet.empty())
    return;

  xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*targetSet.begin());
  if (pEntry == nullptr)
    return;
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(m_dwSkillID);
  if (pSkillCFG == nullptr)
    return;

  pEntry->m_oSkillProcessor.useBuffSkill(pEntry, pNpc, pSkillCFG->getSkillID(), true);
}

IceSpurtAIAction::IceSpurtAIAction(const xLuaData& params) : AIAction(params)
{
  m_fWidth = params.getTableFloat("width");
  m_dwMasterSkillID = params.getTableInt("master_skill");
  m_dwEnemySkillID = params.getTableInt("enemy_skill");
}

IceSpurtAIAction::~IceSpurtAIAction()
{

}

void IceSpurtAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (pNpc == nullptr || pNpc->define.m_oVar.m_qwNpcOwnerID == 0)
    return;
  Scene* pScene = pNpc->getScene();
  if (pScene == nullptr)
    return;

  SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(pNpc->define.m_oVar.m_qwNpcOwnerID);
  if (pMaster == nullptr || pMaster->check2PosInNine(pNpc) == false)
    return;

  xSceneEntrySet set;
  pScene->getEntryIn2Pos(pNpc->getPos(), pMaster->getPos(), m_fWidth, set);

  SceneUser* enemyUser = nullptr;
  float mindis = 1000.0f;
  for (auto &s : set)
  {
    SceneUser* user = dynamic_cast<SceneUser*>(s);
    if (user && pNpc->canAttack(user) && getXZDistance(user->getPos(), pNpc->getPos()) < mindis)
      enemyUser = user;
  }

  if (enemyUser)
  {
    pNpc->useSuperSkill(m_dwEnemySkillID, enemyUser->id, enemyUser->getPos());
  }
  else
  {
    pNpc->useSuperSkill(m_dwMasterSkillID, pMaster->id, pMaster->getPos());
  }
}

// AIAction - chase
ChaseAIAction::ChaseAIAction(const xLuaData& params) : AIAction(params)
{
  //m_bCancel = params.getTableInt("cancel");
  //m_lastTime = params.getTableInt("Time") * 1000;
}

ChaseAIAction::~ChaseAIAction()
{

}

void ChaseAIAction::doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet)
{
  if (!pNpc || !pNpc->getScene() || !pNpc->isAlive())
    return;
  if (targetSet.empty())
    return;


  QWORD curTargetID = pNpc->m_ai.getCurLockID();

  SceneUser* target = nullptr;
  float minDistance = 1000.0f;
  for (auto &v : targetSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*>(v);
    if (!user)
      continue;

    if (user->getTempID() == curTargetID)
    {
      SceneUser* curTarget = SceneUserManager::getMe().getUserByID(curTargetID);
      if(curTarget)
      {
        target = curTarget;
        break;
      }
    }
    
    float distance = getXZDistance(user->getPos(),pNpc->getPos());
    if (distance < minDistance)
    {
      minDistance = distance;
      target = user;
    }
  }
  pNpc->m_ai.setCurLockID(target->getTempID());
}
