#include "SuperAI.h"
#include "SceneNpc.h"
//const char LOG_NAME[] = "SuperAI";

// super ai manager
SuperAIManager::SuperAIManager()
{

}

SuperAIManager::~SuperAIManager()
{
  AIItemFactory<AICondition>::delMe();
  AIItemFactory<AITarget>::delMe();
  AIItemFactory<AIAction>::delMe();
}

bool SuperAIManager::init()
{
  regAIItem();
  return true;
}

void SuperAIManager::regAIItem()
{
  // ************************************ AICondition *******************************************
  {
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,BirthAICondition>("birth"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,DeathAICondition>("death"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,TimerAICondition>("time"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,HPLessAICondition>("hpless"));

    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, RoundHpLessAICondition>("roundhpless"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, ChantLockAICondition>("chantlock"));
    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, RoundUserAICondition>("rounduser"));
    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, RoundOnBuffAICondition>("roundonbuff"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, OnBuffAICondition>("onbuff"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, HPChangeAICondition>("hpchange"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, BeAttackAICondition>("beattack"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, KillUserAICondition>("killuser"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, NormalAICondition>("normal"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, AttackAICondition>("attack"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, DispHideAICondition>("disp_hide"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, SeeDeadAICondition>("seedead"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, BuffLayerAICondition>("buff_layer"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, CameraShotAICondition>("camera_shot"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, CameraLockAICondition>("camera_lock"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, EmployerDieAICondition>("employer_die"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, EmployerReliveAICondition>("employer_relive"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, EmployerDieStatusAICondition>("employer_die_status"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, EmployerActionAICondition>("employer_action"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, NpcFunctionAICondition>("npc_function"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, ServantDieAICondtion>("servant_die"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, VisitAICondition>("visit"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, KillMonsterAIConditon>("killmonster"));
    AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, AlertAIConditon>("alert"));


    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,HpLostPerCondition>("hplostper"));
    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,PosCondition>("atpos"));
    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,UserRadiusCondition>("useradius"));
    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition,NearByCondition>("nearby"));
    //AIItemFactory<AICondition>::getMe().reg(NEW AIItemCreator<AICondition, HpChangeCondition>("hpchange"));
  }

  // ************************************ AITarget *******************************************
  {
    //AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, NoneAITarget>("none"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, SelfAITarget>("self"));
    //AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, AttackerAITarget>("attacker"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, RangeAITarget>("range"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, AttackMeAITarget>("attackme"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, LockMeAITarget>("lockme"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, UserKilledAITarget>("userkilled"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, SprintAITarget>("sprint"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, LockingAITarget>("locking"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, DeadAITarget>("dead"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, HandUserAITarget>("handuser"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, ActionUserAITarget>("actionuser"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, WeaponPetUserAITarget>("weaponpetuser"));
    //AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, MapAITarget>("map"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, BeingMasterAITarget>("beingmaster"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, UsingSkillAITarget>("usingskill"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, VisitorAITarget>("visitor"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, NearMasterAITarget>("near_master"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, BetweenMasterAITarget>("between_master"));
    AIItemFactory<AITarget>::getMe().reg(NEW AIItemCreator<AITarget, BufferAITarget>("buffer"));
  }

  // ************************************ AIAction *******************************************
  {
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, TalkAIAction>("talk"));
    //AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, PostAIAction>("post"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, SummonNpcAIAction>("summon"));
    //AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, RandomGotoAIAction>("randomgoto"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, SkillAIAction>("skill"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, SkillMeAIAction>("skillme"));
    //AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ChangeHpAIAction>("changehp"));
    //AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ClearSelfAIAction>("die"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, GMCmdAIAction>("GM"));
    //AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ClearSelfAIAction>("clearself"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, RouteAIAction>("set_route"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, SupplyAIAction>("supply"));

    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, AttackAIAction>("attack"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, GetawayAIAction>("getaway"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, BuffAIAction>("buff"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, TriggerCondAIAction>("trigger"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ExpressionAIAction>("expression"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ChangeAIAction>("changeai"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, GoPosAIAction>("go_pos"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, TurnMonsterAIAction>("turn_monster"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ChangeTargetAIAction>("change_target"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ReliveDeadAIAction>("relive_dead"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, SetClearAIAction>("set_clear"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ComeToMeAIAction>("cometome"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, RewardMapAIAction>("reward_map"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, BeingSkillAIAction>("being_skill"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, LockTargetAIAction>("lock_target"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, TargetSkillAIAction>("target_skill"));
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, IceSpurtAIAction>("ice_spurt"));
    
    AIItemFactory<AIAction>::getMe().reg(NEW AIItemCreator<AIAction, ChaseAIAction>("chase"));
  }
}

// super ai item
SuperAIItem::SuperAIItem()
{

}

SuperAIItem::~SuperAIItem()
{
  SAFE_DELETE(m_pCondition);
  for (auto v = m_vecTargets.begin(); v != m_vecTargets.end(); ++v)
    SAFE_DELETE(*v);
  m_vecTargets.clear();

  for (auto v = m_vecActions.begin(); v != m_vecActions.end(); ++v)
    SAFE_DELETE(*v);
  m_vecActions.clear();
}

void SuperAIItem::reset()
{
  m_pCondition->reset();
}

bool SuperAIItem::canTrigger(SceneNpc* pNpc) const
{
  if (pNpc == nullptr )
    return false;

  if (!m_bActive)
    return false;

  AI_RET_ENUM ret = m_pCondition->checkCondition(pNpc);
  if (ret != AI_RET_RUN_SUCC && ret != AI_RET_RUN_SUCC_CONT)
    return false;
  return true;
}


AI_RET_ENUM SuperAIItem::exe(SceneNpc* pNpc) const
{
  if (pNpc == nullptr)
    return AI_RET_RUN_FAIL;

  if (m_pCondition->checkOdds() == false)
    return AI_RET_RUN_FAIL;

  bool exeOK = false;
  xSceneEntrySet targetSet;
  for (auto &v : m_vecTargets)
  {
    targetSet.clear();
    v->getEntry(pNpc, m_pCondition, targetSet);
    if (targetSet.empty())
      continue;
    for (auto &v2 : m_vecActions)
    {
      if (v2->getStep() != v->getStep()) // 相同组Target与Action(同Condition, 可以多Target, Action)
        continue;
      if (v2->getPerCent() != 0 && selectByPercent(v2->getPerCent()) == false)
        continue;
      bool haveChangeAI = (v2->getName() == "changeai");
      v2->doAction(pNpc, targetSet);
      if (haveChangeAI)//ai has changed must break
        return AI_RET_RUN_OK_BREAK;
      exeOK = true;
    }
  }
  return exeOK ? AI_RET_RUN_SUCC : AI_RET_RUN_FAIL;

  /*
  for (auto v = m_vecTargets.begin(); v != m_vecTargets.end(); ++v)
    (*v)->getEntry(pNpc, m_pCondition, targetSet);

  if (targetSet.empty() == true)
    return AI_RET_RUN_FAIL;

  // add count
  // if (m_pCondition) m_pCondition->addCount();

  for (auto v = m_vecActions.begin(); v != m_vecActions.end(); ++v)
  {
    AIAction* p = *v;
    if (!p)
      continue;
    if (p->getPerCent() != 0 && selectByPercent(p->getPerCent()) == false)
      continue;
    std::string aiName = p->getName();
    p->doAction(pNpc, targetSet);
    if (aiName == "changeai")
    {//ai has changed must break
      break;
    }
  }

  return AI_RET_RUN_SUCC;
  */
}

DWORD SuperAIItem::getWeight() const
{
  return m_pCondition->getWeight();
}

// super ai
SuperAI::SuperAI(SceneNpc *npc) : m_pNpc(npc)
{

}

SuperAI::~SuperAI()
{
  clear();
}

bool SuperAI::init(const TSetDWORD& setIDs)
{
  // 处理怪物变身ai变化, 记录当前有效的ai
  for (auto &v : m_vecAIItems)
  {
    v->reset();
    v->m_bActive = false;
  }

  for (auto &s : setIDs)
  {
    /*auto it = find_if(m_vecAIItems.begin(), m_vecAIItems.end(), [s](const SuperAIItem* pOneAI) ->bool {
        return pOneAI->m_dwID == s;
        });
    if (it != m_vecAIItems.end())
    {
      (*it)->m_bActive = true;
      continue;
    }
    */
    bool find = false;
    for (auto &v : m_vecAIItems)
    {
      if (v->m_dwID == s)
      {
        v->m_bActive = true;
        find = true;
      }
    }
    if (find)
      continue;

    const SSuperAICFG* pCFG = SuperAIConfig::getMe().getSuperAICFG(s);
    if (pCFG == nullptr)
    {
      XERR << "[怪物superai]" << m_pNpc->id << m_pNpc->getNpcID() << m_pNpc->name << "id:" << s << "未在 Table_SuperAI.txt 表中找到" << XEND;
      continue;
    }

    for (auto &v : pCFG->vecItems)
    {
      SuperAIItem* pItem = NEW SuperAIItem();
      if (pItem == nullptr)
        continue;

      pItem->m_pCondition = AIItemFactory<AICondition>::getMe().create(v.stCondition.content, v.stCondition.oParams);
      if (pItem->m_pCondition == nullptr)
      {
        XERR << "[怪物superai], condition配置错误, ID:" << s << XEND;
        SAFE_DELETE(pItem);
        return false;
      }

      for (auto &o : v.vecTargets)
      {
        AITarget* p = AIItemFactory<AITarget>::getMe().create(o.content, o.oParams);
        if (p == nullptr)
        {
          XERR << "[怪物superai], target配置错误, ID:" << s << XEND;
          SAFE_DELETE(pItem);
          return false;
        }
        pItem->m_vecTargets.push_back(p);
      }

      for (auto &o : v.vecActions)
      {
        AIAction* p = AIItemFactory<AIAction>::getMe().create(o.content, o.oParams);
        if (p == nullptr)
        {
          XERR << "[怪物superai], action配置错误, ID:" << s << XEND;
          SAFE_DELETE(pItem);
          return false;
        }
        pItem->m_vecActions.push_back(p);
      }

      pItem->m_dwID = s;
      pItem->m_bActive = true;
      m_vecAIItems.push_back(pItem);
    }
    XLOG << "[怪物superai] " << m_pNpc->id << m_pNpc->getNpcID() << m_pNpc->name << " id : " << s << " 初始化成功" << XEND;
  }

  return true;
}

void SuperAI::clearTVecSuperAIItem(TVecSuperAIItem &vec)
{
  for (auto it : vec)
  {
    SAFE_DELETE(it);
  }
  vec.clear();
}

bool SuperAI::changeAI(const TSetDWORD& setIDs)
{
  TVecSuperAIItem vecAIItems;

  for (auto &s : setIDs)
  {
    if (0 == s) continue;

    const SSuperAICFG* pCFG = SuperAIConfig::getMe().getSuperAICFG(s);
    if (pCFG == nullptr)
    {
      XERR << "[怪物superai] " << m_pNpc->id << m_pNpc->getNpcID() << m_pNpc->name << " id : " << s << " 未在 Table_SuperAI.txt 表中找到" << XEND;
      continue;
    }

    for (auto &v : pCFG->vecItems)
    {
      SuperAIItem* pItem = NEW SuperAIItem();
      if (pItem == nullptr)
        continue;

      pItem->m_pCondition = AIItemFactory<AICondition>::getMe().create(v.stCondition.content, v.stCondition.oParams);
      if (pItem->m_pCondition == nullptr)
      {
        SAFE_DELETE(pItem);
        clearTVecSuperAIItem(vecAIItems);
        return false;
      }

      for (auto &o : v.vecTargets)
      {
        AITarget* p = AIItemFactory<AITarget>::getMe().create(o.content, o.oParams);
        if (p == nullptr)
        {
          SAFE_DELETE(pItem);
          clearTVecSuperAIItem(vecAIItems);
          return false;
        }
        pItem->m_vecTargets.push_back(p);
      }

      for (auto &o : v.vecActions)
      {
        AIAction* p = AIItemFactory<AIAction>::getMe().create(o.content, o.oParams);
        if (p == nullptr)
        {
          SAFE_DELETE(pItem);
          clearTVecSuperAIItem(vecAIItems);
          return false;
        }
        pItem->m_vecActions.push_back(p);
      }

      pItem->m_dwID = s;
      pItem->m_bActive = true;
      vecAIItems.push_back(pItem);
    }
    XLOG << "[怪物superai] " << m_pNpc->id << m_pNpc->getNpcID() << m_pNpc->name << " id : " << s << " 初始化成功" << XEND;
  }
  //前面没问题才clear
  clear(); 
  m_vecAIItems.insert(m_vecAIItems.begin(), vecAIItems.begin(), vecAIItems.end());
  return true;
}

void SuperAI::clear()
{
  for (auto v = m_vecAIItems.begin(); v != m_vecAIItems.end(); ++v)
    SAFE_DELETE(*v);
  m_vecAIItems.clear();
}

/*
 * SuperAI 执行流程:
 * 一个Condition, 后面可接多个Target, Action, 一个Target后面可接多个Action
 * 多个'同ID, 同Condition名字', 按权重执行
 * 同Condition, 多个Target, Action, 顺序执行, 多Action 顺序执行
 *
 * 根据 AICondition 选择可执行AI列表 (不包含概率检查)
 * 从AI列表中根据权重随机选择一个
 * 检测该AI的执行概率是否满足
 * 执行该AI
 */

typedef std::list<std::pair<DWORD/*weight*/, SuperAIItem*>> TListWeigth2AI;
void SuperAI::checkSig(string name)
{
  std::map<DWORD, TListWeigth2AI> mapID2AIItem;

  // 获得AI权重
  for (auto v = m_vecAIItems.begin(); v != m_vecAIItems.end(); ++v)
  {
    if ((*v)->getConditionName() != name) continue;

    if ((*v)->canTrigger(m_pNpc))
    {
      //不随机的AI直接执行
      if ((*v)->m_pCondition->isNoRandExe())
      {
        AI_RET_ENUM ret = AI_RET_RUN_FAIL;
        ret = (*v)->exe(m_pNpc);
        if (ret == AI_RET_RUN_OK_BREAK) // changeAI, 迭代器可能变化, 退出执行
          return; 
        if (ret == AI_RET_RUN_SUCC)
          (*v)->m_pCondition->addCount();
        continue;
      }
      DWORD weight = (*v)->getWeight();

      TListWeigth2AI& listItemPair = mapID2AIItem[(*v)->m_dwID];
      listItemPair.push_back(std::make_pair(weight, (*v)));
    }
  }
  // AI ID不同时, 分开执行
  for (auto &m : mapID2AIItem)
  {
    DWORD totalWeight = 0;
    for (auto &v : m.second)
    {
      totalWeight += v.first;
      v.first = totalWeight;
    }
    DWORD rand = randBetween(1, totalWeight);
    AI_RET_ENUM ret = AI_RET_RUN_FAIL;
    for (auto &v : m.second)
    {
      if (v.first >= rand)
      {
        ret = v.second->exe(m_pNpc);
        break;
      }
    }
    if (ret == AI_RET_RUN_OK_BREAK) // changeAI, 迭代器可能变化, 退出执行
      break;

    if (ret == AI_RET_RUN_SUCC)
    {
      // 权重一个成功后，其他同类型均计数
      for (auto &v : m.second)
      {
        if (v.second->m_pCondition)
          v.second->m_pCondition->addCount();
      }
    }
  }

  /*
  //根据权重随机一个执行
  DWORD ret = randBetween(0, totalWeight);
  SuperAIItem* pExe = nullptr;
  for (auto it = vecItemPair.begin(); it != vecItemPair.end(); ++it)
  {
    if (ret < it->first)
    {
      pExe = it->second;
      break;
    }
  }

  if (pExe)
  {
    if (AI_RET_RUN_SUCC == pExe->exe(m_pNpc))
    {
      // 权重一个成功后，其他同类型均计数
      for (auto it = vecItemPair.begin(); it != vecItemPair.end(); ++it)
      {
        if (it->second && it->second->m_pCondition)
          it->second->m_pCondition->addCount();
      }
    }
  }
  */
}

bool SuperAI::addAI(DWORD id)
{
  for (auto &v : m_vecAIItems)
  {
    if (v->m_dwID == id)
    {
      XERR << "[怪物—SuperAI], 添加AI重复, 怪物:" << m_pNpc->name << m_pNpc->id << "AI ID:" << id << XEND;
      return false;
    }
  }
  const SSuperAICFG* pCFG = SuperAIConfig::getMe().getSuperAICFG(id);
  if (pCFG == nullptr)
  {
    XERR << "[怪物superai]" << m_pNpc->id << m_pNpc->getNpcID() << m_pNpc->name << "id:" << id << "未在 Table_SuperAI.txt 表中找到" << XEND;
    return false;
  }

  for (auto &v : pCFG->vecItems)
  {
    SuperAIItem* pItem = NEW SuperAIItem();
    if (pItem == nullptr)
      continue;

    pItem->m_pCondition = AIItemFactory<AICondition>::getMe().create(v.stCondition.content, v.stCondition.oParams);
    if (pItem->m_pCondition == nullptr)
    {
      XERR << "[怪物superai], condition配置错误, ID:" << id << XEND;
      SAFE_DELETE(pItem);
      return false;
    }

    for (auto &o : v.vecTargets)
    {
      AITarget* p = AIItemFactory<AITarget>::getMe().create(o.content, o.oParams);
      if (p == nullptr)
      {
        XERR << "[怪物superai], target配置错误, ID:" << id << XEND;
        SAFE_DELETE(pItem);
        return false;
      }
      pItem->m_vecTargets.push_back(p);
    }

    for (auto &o : v.vecActions)
    {
      AIAction* p = AIItemFactory<AIAction>::getMe().create(o.content, o.oParams);
      if (p == nullptr)
      {
        XERR << "[怪物superai], action配置错误, ID:" << id << XEND;
        SAFE_DELETE(pItem);
        return false;
      }
      pItem->m_vecActions.push_back(p);
    }

    pItem->m_dwID = id;
    pItem->m_bActive = true;
    m_vecAIItems.push_back(pItem);
  }
  XLOG << "[怪物superai] " << m_pNpc->id << m_pNpc->getNpcID() << m_pNpc->name << " id : " << id << "添加成功" << XEND;

  return true;
}
