/**
 * @file BufferManager.cpp
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-29
 */
#include "BufferManager.h"
#include "BufferState.h"
#include "ConfigManager.h"

bool BufferManager::loadConfig()
{
  bool bCorrect = true;
  m_mapID2Buff.clear();
  auto func = [&](const xEntry* pEntry)
  {
    const SBuffBase* pBuffCFG = static_cast<const SBuffBase *>(pEntry);
    if (pBuffCFG == nullptr)
      return;
    DWORD id = pBuffCFG->id;
    if (m_mapID2Buff.find(id) != m_mapID2Buff.end())
    {
      XERR << "[Buffer], id =" << id << "id重复" << XEND;
      bCorrect = false;
      return;
    }
    const xLuaData& dataCond = pBuffCFG->getData("Condition");
    TPtrBuffCond pBuffCondition = createBuffCondition(dataCond);
    if (pBuffCondition == nullptr)
    {
      XERR << "[Buffer], id =" << id << "Condition配置错误, 加载Table_Buffer.txt失败" << XEND;
      bCorrect = false;
      return;
      //return false;
    }
    TPtrBufferState pBuffState = createBuffer(id, pBuffCFG->getData(), pBuffCondition);
    if (pBuffState == nullptr)
    {
      XERR << "[Buffer],id =" << id << "加载Table_Buffer.txt失败" << XEND;
      bCorrect = false;
      return;
      //return false;
    }
    pBuffState->sortInit();
    m_mapID2Buff[id] = pBuffState;
  };

  Table<SBuffBase>* pBuffTable = TableManager::getMe().getBufferCFGListNoConst();
  if (pBuffTable == nullptr)
    return false;
  pBuffTable->foreachNoConst(func);

  /*
  if (!xLuaTable::getMe().open("Lua/Table/Table_Buffer.txt"))
  {
    XERR << "[Buffer], 加载Table_Buffer.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  m_mapID2Buff.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Buffer", table);

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->first;
    if (m_mapID2Buff.find(id) != m_mapID2Buff.end())
    {
      XERR << "[Buffer], id =" << id << "id重复" << XEND;
      bCorrect = false;
      continue;
    }
    xLuaData& dataCond = m->second.getMutableData("Condition");
    TPtrBuffCond pBuffCondition = createBuffCondition(dataCond);
    if (pBuffCondition == nullptr)
    {
      XERR << "[Buffer], id =" << id << "Condition配置错误, 加载Table_Buffer.txt失败" << XEND;
      bCorrect = false;
      continue;
      //return false;
    }
    TPtrBufferState pBuffState = createBuffer(id, m->second, pBuffCondition);
    if (pBuffState == nullptr)
    {
      XERR << "[Buffer],id =" << id << "加载Table_Buffer.txt失败" << XEND;
      bCorrect = false;
      continue;
      //return false;
    }
    m_mapID2Buff[id] = pBuffState;
  }
  if (bCorrect)
    XLOG << "[Buffer], 加载配置Table_Buffer.txt" << XEND;

  // load for check
  g_pBuffBaseM->load();

  XLOG << "gsj buffsize " << g_pBuffBaseM->size() << XEND;
  */
  return bCorrect;
}

TPtrBuffCond BufferManager::createBuffCondition(xLuaData data)
{
  string type = data.getTableString("type");
  //string name = data.getTableString("name");
  //DWORD value = data.getTableInt("value");
  if (type == "")
  {
    TPtrBuffCond pCond(new BaseCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Attr")
  {
    TPtrBuffCond pCond(new AttrCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Equip")
  {
    TPtrBuffCond pCond(new ItemCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Profession")
  {
    TPtrBuffCond pCond(new ProfesCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "KillRace")
  {
    TPtrBuffCond pCond(new KillCondition());
    if(pCond->init(data))
      return pCond;
  }
  else if (type == "RidingStatus")
  {
    TPtrBuffCond pCond(new RideCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "EquipRefine")
  {
    TPtrBuffCond pCond(new EqRefineCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "AfterTime")
  {
    TPtrBuffCond pCond(new TimeCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "DamagePer")
  {
    TPtrBuffCond pCond(new DamageCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Attack")
  {
    TPtrBuffCond pCond(new AttackCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "BeAttack")
  {
    TPtrBuffCond pCond(new BeAttackCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Status")
  {
    TPtrBuffCond pCond(new StatusCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "HpPerLess")
  {
    TPtrBuffCond pCond(new HpLessPerCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "SpPerLess")
  {
    TPtrBuffCond pCond(new SpLessPerCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Reborn")
  {
    TPtrBuffCond pCond(new RebornCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "BeShoot")
  {
    TPtrBuffCond pCond(new BeShootCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "TeamNum")
  {
    TPtrBuffCond pCond(new TeamNumCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Hand")
  {
    TPtrBuffCond pCond(new HandCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "UseSkill")
  {
    TPtrBuffCond pCond(new UseSkillCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Distance")
  {
    TPtrBuffCond pCond(new DistanceCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "HasBuff")
  {
    TPtrBuffCond pCond(new HasBuffCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "NpcPresent")
  {
    TPtrBuffCond pCond(new NpcPresentCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "MapType")
  {
    TPtrBuffCond pCond(new MapTypeCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "UseSkillKill")
  {
    TPtrBuffCond pCond(new UseSkillKillCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Suspend")
  {
    TPtrBuffCond pCond(new SuspendCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Gender")
  {
    TPtrBuffCond pCond(new GenderCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Map")
  {
    TPtrBuffCond pCond(new MapBuffCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Abnormal")
  {
    TPtrBuffCond pCond(new AbnormalStateCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "HasElement")
  {
    TPtrBuffCond pCond(new ElementElfCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "ChantStatus")
  {
    TPtrBuffCond pCond(new ChantStatusCondition());
    if (pCond->init(data))
      return pCond;
  }
  else if (type == "Concert")
  {
    TPtrBuffCond pCond(new ConcertCondition());
    if (pCond->init(data))
      return pCond;
  }
  return nullptr;
}

TPtrBufferState BufferManager::createBuffer(DWORD id, xLuaData data, TPtrBuffCond buffCond)
{
  string type = data.getData("BuffEffect").getTableString("type");
  //TPtrBufferState pBuff = nullptr;
  if (type == "" || type =="AttrChange")
  {
    TPtrBufferState pBuff(new BuffAttrChange());
    //pBuff = (TPtrBufferState)(new BuffAttrChange());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "HSPChange")
  {
    TPtrBufferState pBuff(new BuffHSPChange());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Hide")
  {
    TPtrBufferState pBuff(new BuffHiding());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type =="BeatBack")
  {
    TPtrBufferState pBuff(new BuffBeatBack());
    if (pBuff ==nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if(type =="HpRecover")
  {
    TPtrBufferState pBuff(new BuffRecoverHp());
    if (pBuff ==nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if(type =="SpRecover")
  {
    TPtrBufferState pBuff(new BuffSpRecover());
    if (pBuff ==nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "UseSkill")
  {
    TPtrBufferState pBuff(new BuffUseSkill());
    if (pBuff ==nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Transform")
  {
    TPtrBufferState pBuff(new BuffTransform());
    if (pBuff ==nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DropItem")
  {
    TPtrBufferState pBuff(new BuffDropItem());
    if(pBuff == nullptr || !pBuff->init(id,data,buffCond))
       return nullptr;
    return pBuff;
  }
  else if (type == "AllowRiding")
  {
    TPtrBufferState pBuff(new BuffAllowRide());
    if (pBuff ==nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Disperse")
  {
    TPtrBufferState pBuff(new BuffDriveout());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AddBuff")
  {
    TPtrBufferState pBuff(new BuffAddBuff());
    if (pBuff ==nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "GetSkill")
  {
    TPtrBufferState pBuff(new BuffGetSkill());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "StatusChange")
  {
    TPtrBufferState pBuff(new BuffStatusChange());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "PlayAction")
  {
    TPtrBufferState pBuff(new BuffPlayAction());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Disable")
  {
    TPtrBufferState pBuff(new BuffDisable());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "BattleAttr")
  {
    TPtrBufferState pBuff(new BuffBattleAttr());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DelMe")
  {
    TPtrBufferState pBuff(new BuffDelMe());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "HpReduce")
  {
    TPtrBufferState pBuff(new BuffHpReduce());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DelBuff")
  {
    TPtrBufferState pBuff(new BuffDelBuff());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DelStatus")
  {
    TPtrBufferState pBuff(new BuffDelStatus());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ImmuneStatus")
  {
    TPtrBufferState pBuff(new BuffImmuneStatus());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "SeeTrap")
  {
    TPtrBufferState pBuff(new BuffSeeTrap());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "PartTransform")
  {
    TPtrBufferState pBuff(new BuffPartTransform());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "RobReward")
  {
    TPtrBufferState pBuff(new BuffRobReward());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "HandEmoji")
  {
    TPtrBufferState pBuff(new BuffHandEmoji());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ChangeDefine")
  {
    TPtrBufferState pBuff(new BuffChangeDefine());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ForceAttr")
  {
    TPtrBufferState pBuff(new BuffForceAttr());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Treasure")
  {
    TPtrBufferState pBuff(new BuffTreasure());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "LoseTarget")
  {
    TPtrBufferState pBuff(new BuffLoseTarget());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "PriorAttack")
  {
    TPtrBufferState pBuff(new BuffPriorAttack());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "HpProtect")
  {
    TPtrBufferState pBuff(new BuffHpProtect());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "MultiTime")
  {
    TPtrBufferState pBuff(new BuffMultiTime());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ChangeScale")
  {
    TPtrBufferState pBuff(new BuffChangeScale());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ChangeSkill")
  {
    TPtrBufferState pBuff(new BuffChangeSkill());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ShareDam")
  {
    TPtrBufferState pBuff(new BuffShareDam());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "BreakEquip")
  {
    TPtrBufferState pBuff(new BuffBreakEquip());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "PackageSlot")
  {
    TPtrBufferState pBuff(new BuffPackageSlot());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AffactSkill")
  {
    TPtrBufferState pBuff(new BuffAffactSkill());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AffactTrade")
  {
    TPtrBufferState pBuff(new BuffTradeInfo());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AffactBooth")
  {
    TPtrBufferState pBuff(new BuffBoothInfo());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ExtraDrop")
  {
    TPtrBufferState pBuff(new BuffExtraDrop());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DropExp")
  {
    TPtrBufferState pBuff(new BuffDropExp());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "BuffSpeedUp")
  {
    TPtrBufferState pBuff(new BuffSpeedUp());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "MarkHeal")
  {
    TPtrBufferState pBuff(new BuffMarkHeal());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "TrigTrap")
  {
    TPtrBufferState pBuff(new BuffTrigTrap());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ReplaceSkill")
  {
    TPtrBufferState pBuff(new BuffReplaceSkill());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "LimitAttr")
  {
    TPtrBufferState pBuff(new BuffAttrLimit());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AutoBlock")
  {
    TPtrBufferState pBuff(new BuffAutoBlock());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "GM")
  {
    TPtrBufferState pBuff(new BuffGM());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "SkillLevel")
  {
    TPtrBufferState pBuff(new BuffSkillLevel());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "LimitSkill")
  {
    TPtrBufferState pBuff(new BuffLimitSkill());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AddSkillCD")
  {
    TPtrBufferState pBuff(new BuffSkillCD());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "BuffTransfer")
  {
    TPtrBufferState pBuff(new BuffTransfer());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "LimitUseItem")
  {
    TPtrBufferState pBuff(new BuffLimitUseItem());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Undead")
  {
    TPtrBufferState pBuff(new BuffUndead());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ResistStatus")
  {
    TPtrBufferState pBuff(new BuffResistStatus());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DropFoodReward")
  {
    TPtrBufferState pBuff(new BuffDropFoodReward());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DelSpEffect")
  {
    TPtrBufferState pBuff(new BuffDelSpEffect());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ForceStatus")
  {
    TPtrBufferState pBuff(new BuffForceStatus());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "MultiExp")
  {
    TPtrBufferState pBuff(new BuffMultiExp());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "GvgPartReward")
  {
    TPtrBufferState pBuff(new BuffGvgReward());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "PickUpItem")
  {
    TPtrBufferState pBuff(new BuffPickUp());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DelSkillStatus")
  {
    TPtrBufferState pBuff(new BuffDelSkillStatus());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ImmuneAttack")
  {
    TPtrBufferState pBuff(new BuffImmuneAttack());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DeepHide")
  {
    TPtrBufferState pBuff(new BuffDeepHide());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "OffEquip")
  {
    TPtrBufferState pBuff(new BuffOffEquip());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ProtectEquip")
  {
    TPtrBufferState pBuff(new BuffProtectEquip());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "FixEquip")
  {
    TPtrBufferState pBuff(new BuffFixEquip());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "SkillExtraTarget")
  {
    TPtrBufferState pBuff(new BuffSkillTarget());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ExtraReward")
  {
    TPtrBufferState pBuff(new BuffExtraReward());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Scenery")
  {
    TPtrBufferState pBuff(new BuffScenery());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "HPStorage")
  {
    TPtrBufferState pBuff(new BuffHPStorage());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "SPStorage")
  {
    TPtrBufferState pBuff(new BuffSPStorage());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "PetAdventure") 
  {
    TPtrBufferState pBuff(new BuffPetAdventure());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "StatusRebound")
  {
    TPtrBufferState pBuff(new BuffStatusRebound());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "WeaponBlock")
  {
    TPtrBufferState pBuff(new BuffWeaponBlock());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AttrControl")
  {
    TPtrBufferState pBuff(new BuffAttrControl());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "RideWolf")
  {
    TPtrBufferState pBuff(new BuffRideWolf());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ForbidEquip")
  {
    TPtrBufferState pBuff(new BuffForbidEquip());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "UnlockRecipe")
  {
    TPtrBufferState pBuff(new BuffUnlockRecipe());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Chase")
  {
    TPtrBufferState pBuff(new BuffChase());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "AttrTransfer")
  {
    TPtrBufferState pBuff(new BuffAttrTransfer());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "ImmuneTaunt")
  {
    TPtrBufferState pBuff(new BuffImmuneTaunt());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "SPExchange")
  {
    TPtrBufferState pBuff(new BuffSPExchange());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "DHide")
  {
    TPtrBufferState pBuff(new BuffDHide());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "CheckAddLine")
  {
    TPtrBufferState pBuff(new BuffCheckAddLine());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "NoMapExit")
  {
    TPtrBufferState pBuff(new BuffNoMapExit());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "Infect")
  {
    TPtrBufferState pBuff(new BuffInfect());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  else if (type == "")
  {
    TPtrBufferState pBuff(new BufferState());
    if (pBuff == nullptr || !pBuff->init(id, data, buffCond))
      return nullptr;
    return pBuff;
  }
  return nullptr;
}

TPtrBufferState BufferManager::getBuffById(DWORD id)
{
  auto m = m_mapID2Buff.find(id);
  if (m != m_mapID2Buff.end())
    return (m->second);
  return nullptr;
}

DWORD BufferManager::getBuffStatus(DWORD buffid)
{
  TPtrBufferState buffPtr = getBuffById(buffid);
  if (buffPtr == nullptr)
    return 0;
  BuffStatusChange* psc = dynamic_cast <BuffStatusChange*> (buffPtr.get());
  if (psc == nullptr)
    return 0;
  return psc->getStatus();
}

