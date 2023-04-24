#include "Item.h"
#include "ItemManager.h"
#include "SceneUser.h"
#include "GMCommandRuler.h"
#include "BufferManager.h"
#include "SceneUserManager.h"
#include "MsgManager.h"
#include "LuaManager.h"
#include "SceneNpc.h"
#include "PetConfig.h"

// item base
ItemBase::ItemBase(const SItemCFG* pCFG) : m_pCFG(pCFG)
{

}

ItemBase::~ItemBase()
{

}

void ItemBase::toItemData(ItemData* pData)
{
  if (pData == nullptr)
    return;

  if (m_oItemInfo.count() == 0)
    m_oItemInfo.set_count(1);
  pData->mutable_base()->CopyFrom(m_oItemInfo);
}

void ItemBase::toClientData(EPackType eType, ItemData* pData, SceneUser* pUser)
{
  if (pData == nullptr || pUser == nullptr)
    return;
  toItemData(pData);
  if (pUser->getArrowID() == getTypeID())
  {
    pData->mutable_base()->set_isactive(true);
  }
  pData->mutable_base()->set_ishint(pUser->getPackage().canHint(eType, this));
}

void ItemBase::fromItemData(const ItemData& rData)
{
  m_oItemInfo.CopyFrom(rData.base());

  if (m_oItemInfo.count() == 0)
    m_oItemInfo.set_count(1);
  if (m_oItemInfo.type() == EITEMTYPE_MIN && m_pCFG != nullptr)
    m_oItemInfo.set_type(m_pCFG->eItemType);
  if (m_oItemInfo.equiptype() == EEQUIPTYPE_MIN && m_pCFG != nullptr)
    m_oItemInfo.set_equiptype(m_pCFG->eEquipType);
}

bool ItemBase::init()
{
  if (m_pCFG == nullptr)
    return false;

  m_oItemInfo.set_id(m_pCFG->dwTypeID);
  m_oItemInfo.set_count(1);
  m_oItemInfo.set_createtime(xTime::getCurSec());
  m_oItemInfo.set_type(m_pCFG->eItemType);
  m_oItemInfo.set_quality(m_pCFG->eQualityType);
  m_oItemInfo.set_equiptype(m_pCFG->eEquipType);
  return true;
}

// item
Item::Item(const SItemCFG* pCFG) : ItemBase(pCFG)
{

}

Item::~Item()
{

}

Item* Item::clone()
{
  Item* pItem = NEW Item(m_pCFG);
  if (pItem == nullptr)
    return nullptr;

  pItem->m_oItemInfo = m_oItemInfo;
  return pItem;

}

// item add
ItemStuff::ItemStuff(const SItemCFG* pCFG) : ItemBase(pCFG)
{

}

ItemStuff::~ItemStuff()
{

}

ItemStuff* ItemStuff::clone()
{
  ItemStuff* pStuff = NEW ItemStuff(m_pCFG);
  if (pStuff == nullptr)
    return pStuff;

  pStuff->m_oItemInfo = m_oItemInfo;
  return pStuff;
}

bool ItemStuff::use(SceneUser *pUser, const Cmd::ItemUse& itemUseCmd)
{
  if (pUser == nullptr || m_pCFG == nullptr || pUser->isAlive() == false)
    return false;

  xLuaData oGmData = m_pCFG->oGMData;
  oGmData.setData("GM_Use_Source_Item", getTypeID());

  if (m_pCFG->eTargetType == ETARGETTYPE_MY)
  {
    // process command
    if (GMCommandRuler::getMe().execute(pUser, oGmData) == false)
    {
      XERR << "[物品使用-道具药剂]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "use item :" << m_pCFG->dwTypeID << "cmd run error" << XEND;
      return false;
    }
  }
  else if (m_pCFG->eTargetType == ETARGETTYPE_USER || m_pCFG->eTargetType == ETARGETTYPE_MONSTER || m_pCFG->eTargetType == ETARGETTYPE_USERANDMONSTER)
  {
    if (itemUseCmd.targets_size() == 0)
      return false;
    //use to other user    
    oGmData.setData("GM_Target", 1, true);

    QWORD uid = itemUseCmd.targets(0);
    xSceneEntryDynamic* pEntry = xSceneEntryDynamic::getEntryByID(uid);
    if (!pEntry)
    {
      XERR << "[物品使用-目标], 找不到目标" << "使用者:" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "use item:" << m_pCFG->dwTypeID << "目标玩家不存在 target:" << uid << XEND;
      return false;
    }
    if (m_pCFG->eTargetType == ETARGETTYPE_USER && pEntry->getEntryType() == SCENE_ENTRY_NPC)
    {
      XERR << "[物品使用-目标], 使用者:" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "use item:" << m_pCFG->dwTypeID << "不可以对怪使用, 目标id:" << uid << XEND;
      return false;
    }
    if (m_pCFG->eTargetType == ETARGETTYPE_MONSTER)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (pEntry);
      if (!npc || npc->isMonster() == false)
      {
        XERR << "[物品使用-目标], 使用者:" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "use item:" << m_pCFG->dwTypeID << "不可以非怪对象使用, 目标id:" << uid << XEND;
        return false;
      }
    }

    xPos myPos = pUser->getPos();
    xPos otherPos = pEntry->getPos();
    if (checkDistance(myPos, otherPos, m_pCFG->dwRange) == false)
    {
      XERR << "[物品使用-指定玩家]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "目标：" << pEntry->name << pEntry->id << "距离不满足" << XEND;
      MsgManager::sendMsg(pUser->id, 860);
      return false;
    }
    std::stringstream ss;
    ss << "id" << 1;      // start from 1 
    oGmData.setData(ss.str(), uid, true);

    // process command
    if (GMCommandRuler::getMe().execute(pUser, oGmData) == false)
    {
      XERR << "[物品使用-指定玩家]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "use item :" << m_pCFG->dwTypeID << "cmd run error" << XEND;
      return false;
    }
  }
  return true;
}

// item equip
ItemEquip::ItemEquip(const SItemCFG* pCFG) : ItemBase(pCFG), m_bEquiped(false)
{
  if (pCFG != nullptr)
    m_dwCardSlot = pCFG->dwCardSlot;
}

ItemEquip::~ItemEquip()
{

}

ItemEquip* ItemEquip::clone()
{
  ItemEquip* pEquip = NEW ItemEquip(m_pCFG);
  if (pEquip == nullptr)
    return nullptr;

  pEquip->m_oItemInfo = m_oItemInfo;
  pEquip->m_bEquiped = m_bEquiped;
  pEquip->m_dwStrengthLv = m_dwStrengthLv;
  pEquip->m_dwStrengthLv2 = m_dwStrengthLv2;
  pEquip->m_dwRefineLv = m_dwRefineLv;
  pEquip->m_dwStrengthCost = m_dwStrengthCost;

  return pEquip;
}

void ItemEquip::toItemData(ItemData* pData)
{
  if (pData == nullptr)
    return;

  ItemBase::toItemData(pData);

  pData->set_equiped(m_bEquiped);
  pData->set_battlepoint(getBattlePoint());
  toEquipData(pData->mutable_equip());
  pData->mutable_enchant()->CopyFrom(m_oEncData);
  pData->mutable_previewenchant()->CopyFrom(m_oPreviewEncData);
  pData->mutable_refine()->CopyFrom(m_oRefineData);

  for (auto v = m_mapPos2Card.begin(); v != m_mapPos2Card.end(); ++v)
  {
    CardData* p = pData->add_card();
    if (p == nullptr)
      continue;

    p->set_guid(v->second.first);
    p->set_id(v->second.second);
    p->set_pos(v->first);
  }

  pData->mutable_sender()->CopyFrom(m_oSenderData);
}

void ItemEquip::fromItemData(const ItemData& rData)
{
  ItemBase::fromItemData(rData);

  setEquiped(rData.equiped());
  m_dwBattlePoint = rData.battlepoint();

  fromEquipData(rData.equip());
  if (getRefineLv() == 0 && rData.base().refinelv())
    setRefineLv(rData.base().refinelv());

  m_oEncData.Clear();
  m_oEncData.CopyFrom(rData.enchant());
  m_oPreviewEncData.Clear();
  m_oPreviewEncData.CopyFrom(rData.previewenchant());
  m_oRefineData.CopyFrom(rData.refine());

  bool nopos = false;
  for (int i = 0; i < rData.card_size(); ++i)
  {
    if (rData.card(i).pos() == 0)
    {
      nopos = true;
      break;
    }
  }
  m_mapPos2Card.clear();
  for (int i = 0; i < rData.card_size(); ++i)
  {
    if (nopos)
      m_mapPos2Card[i+1] = TPairEquipCard{rData.card(i).guid(), rData.card(i).id()};
    else
    {
      auto it = m_mapPos2Card.find(rData.card(i).pos());
      if (it != m_mapPos2Card.end())
        XERR << "[装备-卡片加载]" << "卡片pos重复,pos:" << rData.card(i).pos() << it->second.first << it->second.second << rData.card(i).guid() << rData.card(i).id() << XEND;
      else
        m_mapPos2Card[rData.card(i).pos()] = TPairEquipCard{rData.card(i).guid(), rData.card(i).id()};
    }
  }

  m_oSenderData.CopyFrom(rData.sender());
}

void ItemEquip::toEquipData(EquipData* pData)
{
  if (pData == nullptr)
    return;

  pData->set_strengthlv(m_dwStrengthLv);
  pData->set_strengthlv2(m_dwStrengthLv2);
  pData->set_refinelv(m_dwRefineLv);

  pData->set_strengthcost(m_dwStrengthCost);

  pData->set_cardslot(m_dwCardSlot);
  pData->set_damage(m_bDamage);
  pData->set_lv(m_dwLv);
  pData->set_color(m_dwBodyColor);

  for (auto v = m_vecRefineCompose.begin(); v != m_vecRefineCompose.end(); ++v)
  {
    RefineCompose* p = pData->add_refinecompose();
    if (p == nullptr)
      continue;

    p->set_id(v->first);
    p->set_num(v->second);
  }

  for (auto v = m_vecBuffIDs.begin(); v != m_vecBuffIDs.end(); ++v)
  {
    pData->add_buffid(*v);
  }

  pData->set_breakstarttime(m_dwBreakStartTime);
  pData->set_breakendtime(m_dwBreakEndTime);
  
  pData->clear_strengthlv2cost();
  for (auto &v : m_vecStrengthCost)
    pData->add_strengthlv2cost()->CopyFrom(v);
}

void ItemEquip::fromEggEquip(const EggEquip& rEquip)
{
  m_oItemInfo.CopyFrom(rEquip.base());

  fromEquipData(rEquip.data());
  if (getRefineLv() == 0 && rEquip.base().refinelv())
    setRefineLv(rEquip.base().refinelv());

  m_oEncData.Clear();
  m_oEncData.CopyFrom(rEquip.enchant());
  m_oPreviewEncData.Clear();
  m_oPreviewEncData.CopyFrom(rEquip.previewenchant());
  m_oRefineData.CopyFrom(rEquip.refine());

  bool nopos = false;
  for (int i = 0; i < rEquip.card_size(); ++i)
  {
    if (rEquip.card(i).pos() == 0)
    {
      nopos = true;
      break;
    }
  }
  m_mapPos2Card.clear();
  for (int i = 0; i < rEquip.card_size(); ++i)
  {
    if (nopos)
      m_mapPos2Card[i+1] = TPairEquipCard{rEquip.card(i).guid(), rEquip.card(i).id()};
    else
    {
      auto it = m_mapPos2Card.find(rEquip.card(i).pos());
      if (it != m_mapPos2Card.end())
        XERR << "[装备-卡片加载]" << "卡片pos重复,pos:" << rEquip.card(i).pos() << it->second.first << it->second.second << rEquip.card(i).guid() << rEquip.card(i).id() << XEND;
      else
        m_mapPos2Card[rEquip.card(i).pos()] = TPairEquipCard{rEquip.card(i).guid(), rEquip.card(i).id()};
    }
  }
}

void ItemEquip::toEggEquip(EggEquip* pEquip)
{
  if (pEquip == nullptr)
    return;

  pEquip->Clear();

  pEquip->mutable_base()->CopyFrom(m_oItemInfo);
  toEquipData(pEquip->mutable_data());

  pEquip->mutable_enchant()->CopyFrom(m_oEncData);
  pEquip->mutable_previewenchant()->CopyFrom(m_oPreviewEncData);
  pEquip->mutable_refine()->CopyFrom(m_oRefineData);

  for (auto v = m_mapPos2Card.begin(); v != m_mapPos2Card.end(); ++v)
  {
    CardData* p = pEquip->add_card();
    if (p == nullptr)
      continue;

    p->set_guid(v->second.first);
    p->set_id(v->second.second);
    p->set_pos(v->first);
  }
}

bool ItemEquip::canEquip(EProfession profession) const
{
  if (m_pCFG == nullptr)
    return false;

  if (m_pCFG->vecEquipPro.empty() == true)
    return true;

  for (auto v = m_pCFG->vecEquipPro.begin(); v != m_pCFG->vecEquipPro.end(); ++v)
  {
    if (*v == profession)
      return true;
  }

  return false;
}

DWORD ItemEquip::getBattlePoint()
{
  return 0;
  if (!m_bBpUpdate)
    return m_dwBattlePoint;

  TVecAttrSvrs vecAttrs;
  vecAttrs.resize(EATTRTYPE_MAX);
  //collectEquipAttr(nullptr, vecAttrs);
  collectEquipAttr(nullptr);

  // battlepoint = atk+def+MAtk+MDef+MaxHp*0.1
  float battlepoint = vecAttrs[EATTRTYPE_ATK].value() +
                      vecAttrs[EATTRTYPE_DEF].value() +
                      vecAttrs[EATTRTYPE_MATK].value() +
                      vecAttrs[EATTRTYPE_MDEF].value() +
                      vecAttrs[EATTRTYPE_MAXHP].value() * 0.1f;

  m_dwBattlePoint = static_cast<DWORD>(battlepoint);
  m_bBpUpdate = false;
  return m_dwBattlePoint;
}

void ItemEquip::setSpecAttr(const char* name, float value)
{
  DWORD id = RoleDataConfig::getMe().getIDByName(name);
  const RoleData* pData = RoleDataConfig::getMe().getRoleData(id);
  if (pData == nullptr)
    return;
  EAttrType eType = static_cast<EAttrType>(id);
  UserAttrSvr uAttr;
  uAttr.set_type(eType);
  uAttr.set_value(value);

  m_vecSpecAttr.push_back(uAttr);
}

//void ItemEquip::collectEquipAttr(SceneUser* pUser, TVecAttrSvrs& vecAttrs)
void ItemEquip::collectEquipAttr(SceneUser* pUser)
{
  //if (vecAttrs.empty() == true)
  //  return;
  if (m_pCFG == nullptr || pUser == nullptr)
    return;
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  if (getIndex() != rCFG.getValidEquipPos(static_cast<EEquipPos>(getIndex()), getEquipType()))
    return;
  if (ItemConfig::getMe().isArtifact(m_pCFG->eItemType) && (pUser == nullptr || pUser->hasGuild() == false || pUser->getGuild().isUserOwnArtifact(pUser->id, getGUID()) == false))
    return;
  Attribute* pAttr = pUser->getAttribute();
  if (pAttr == nullptr)
    return;

  for (auto v = m_mapPos2Card.begin(); v != m_mapPos2Card.end(); ++v)
  {
    const SItemCFG* pCardCFG = ItemManager::getMe().getItemCFG(v->second.second);
    if (pCardCFG == nullptr)
      continue;

    for (auto c = pCardCFG->vecCardBase.begin(); c != pCardCFG->vecCardBase.end(); ++c)
      pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, *c);
      /*{
      float value = vecAttrs[c->type()].value() + c->value();
      vecAttrs[c->type()].set_type(c->type());
      vecAttrs[c->type()].set_value(value);
    }*/
  }

  for (int i = 0; i < m_oEncData.attrs_size(); ++i)
  {
    const RoleData* pData = RoleDataConfig::getMe().getRoleData(m_oEncData.attrs(i).type());
    if (pData == nullptr)
      continue;

    const EnchantAttr& rAttr = m_oEncData.attrs(i);
    UserAttrSvr oAttr;
    oAttr.set_type(rAttr.type());

    if (pData->bPercent)
      oAttr.set_value(double(rAttr.value()) / FLOAT_TO_DWORD);
    else
      oAttr.set_value(rAttr.value());
    pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, oAttr);

    /*if (pData->bPercent)
      vecAttrs[m_oEncData.attrs(i).type()].set_value(vecAttrs[m_oEncData.attrs(i).type()].value() + double(m_oEncData.attrs(i).value()) / FLOAT_TO_DWORD);
    else
      vecAttrs[m_oEncData.attrs(i).type()].set_value(vecAttrs[m_oEncData.attrs(i).type()].value() + m_oEncData.attrs(i).value());*/
  }

  // 被破坏的装备仅保留附魔/插卡的属性
  if (isBroken())
    return;

  clearBrokenTime();
  if (m_pCFG->isLotteryEquip())
  {
    if (pUser)
    {
      const LevelData*pLevelData = m_pCFG->getLevelData(pUser->getLevel());
      if (pLevelData)
      {
        for (auto v = pLevelData->vecEquipBase.begin(); v != pLevelData->vecEquipBase.end(); ++v)
          pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, *v);
          /*{
          float value = vecAttrs[v->type()].value() + v->value();
          vecAttrs[v->type()].set_type(v->type());
          vecAttrs[v->type()].set_value(value);
        }*/
      }
    }
  }
  else
  {
    for (auto v = m_pCFG->vecEquipBase.begin(); v != m_pCFG->vecEquipBase.end(); ++v)
      pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, *v);
    //{
    //  float value = vecAttrs[v->type()].value() + v->value();
    //  vecAttrs[v->type()].set_type(v->type());
    //  vecAttrs[v->type()].set_value(value);
    //}
  }

  for (auto v = m_pCFG->vecEquipStrength.begin(); v != m_pCFG->vecEquipStrength.end(); ++v)
  {
    UserAttrSvr oAttr;
    oAttr.CopyFrom(*v);
    oAttr.set_value(v->value() * getStrengthLv());
    pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, oAttr);
  }
    /*{
    float value = vecAttrs[v->type()].value() + v->value() * getStrengthLv();
    vecAttrs[v->type()].set_type(v->type());
    vecAttrs[v->type()].set_value(value);
  }*/
  LuaManager::getMe().call<void>("calcStrengthAttr", static_cast<xSceneEntryDynamic*>(pUser), getQuality(), getEquipType(), getStrengthLv2());

  for (auto v = m_pCFG->vecEquipRefine.begin(); v != m_pCFG->vecEquipRefine.end(); ++v)
  {
    UserAttrSvr oAttr;
    oAttr.CopyFrom(*v);
    oAttr.set_value(v->value() * getRefineLv());
    pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, oAttr);
  }
    /*{
    float value = vecAttrs[v->type()].value() + v->value() * getRefineLv();
    vecAttrs[v->type()].set_type(v->type());
    vecAttrs[v->type()].set_value(value);
  }*/

  m_vecSpecAttr.clear();
  LuaManager::getMe().call<void>("CalcSpecEquipAttr", this, getTypeID(), getRefineLv());
  for (auto &v : m_vecSpecAttr)
    pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, v);
    /*{
    float value = v.value() + vecAttrs[v.type()].value();
    vecAttrs[v.type()].set_value(value);
  }*/

  if (pUser)
  {
    //pUser->getHighRefine().collectEquipAttr(static_cast<EEquipPos>(getIndex()), getRefineLv() ,vecAttrs);
    pUser->getHighRefine().collectEquipAttr(static_cast<EEquipPos>(getIndex()), getRefineLv());
  }
  if (pUser && pUser->getScene() && (pUser->getScene()->isPVPScene() || pUser->getScene()->isGvg()))
  {
    for (auto v = m_pCFG->vecPVPEquipBase.begin(); v != m_pCFG->vecPVPEquipBase.end(); ++v)
      pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, *v);
      /*{
      float value = vecAttrs[v->type()].value() + v->value();
      vecAttrs[v->type()].set_type(v->type());
      vecAttrs[v->type()].set_value(value);
    }*/
  }
}

void ItemEquip::collectEquipSkill(TVecDWORD& vecSkillIDs) const
{
  vecSkillIDs.clear();
  if (m_pCFG == nullptr)
    return;

  // collect skill ids to do...
}

bool ItemEquip::canDecompose() const
{
  return getCardList().empty() == true && getLv() <= 0 && getStrengthLv() <= 0 && m_oEncData.type() == EENCHANTTYPE_MIN;
}

void ItemEquip::addRefineCompose(DWORD id, DWORD count /*= 1*/)
{
  auto v = find_if(m_vecRefineCompose.begin(), m_vecRefineCompose.end(), [id](const TPairRefineCompose& r) -> bool{
    return r.first == id;
  });
  if (v == m_vecRefineCompose.end())
  {
    TPairRefineCompose p;
    p.first = id;
    p.second = count;
    m_vecRefineCompose.push_back(p);
    return;
  }

  v->second += count;
}

bool ItemEquip::hasCard(ItemCard* pCard)
{
  return false;
  if (pCard == nullptr)
    return true;

  // check guid
  const string& guid = pCard->getGUID();
  auto v = find_if(m_mapPos2Card.begin(), m_mapPos2Card.end(), [guid](const TMapEquipCard::value_type& r) -> bool{
    return r.second.first == guid;
  });
  if (v != m_mapPos2Card.end())
    return true;

  // check same type
  const SItemCFG* pCFG = pCard->getCFG();
  if (pCFG == nullptr)
    return true;
  DWORD type = pCFG->dwCardType;
  v = find_if(m_mapPos2Card.begin(), m_mapPos2Card.end(), [type](const TMapEquipCard::value_type& r) -> bool{
    const SItemCFG* pCardCFG = ItemManager::getMe().getItemCFG(r.second.second);
    if (pCardCFG == nullptr)
      return true;

    return pCardCFG->dwCardType == type;
  });

  return v != m_mapPos2Card.end();
}

bool ItemEquip::canAddCard(ItemCard* pCard, DWORD pos, bool notReplace)
{
  if (pCard == nullptr || pos == 0)
    return false;
  if (notReplace && getCardByPos(pos) != nullptr)
    return false;
  if (hasCard(pCard) == true)
    return false;
  if (notReplace && m_mapPos2Card.size() >= getCardSlot())
    return false;

  const SItemCFG* pCardCFG = pCard->getCFG();
  if (pCardCFG == nullptr)
    return false;
  if (MiscConfig::getMe().canEquip(pCardCFG->dwCardPosition, getType()) == false)
    return false;
  return true;
}

bool ItemEquip::addCard(ItemCard* pCard, DWORD pos)
{
  if (canAddCard(pCard, pos, true) == false)
    return false;
  m_mapPos2Card[pos] = TPairEquipCard(pCard->getGUID(), pCard->getTypeID());
  return true;
}

bool ItemEquip::removeCard(const string& guid)
{
  auto v = find_if(m_mapPos2Card.begin(), m_mapPos2Card.end(), [guid](const TMapEquipCard::value_type& r) -> bool{
    return r.second.first == guid;
  });
  if (v == m_mapPos2Card.end())
    return false;

  m_mapPos2Card.erase(v);
  return true;
}

bool ItemEquip::removeCardByPos(DWORD pos)
{
  auto it = m_mapPos2Card.find(pos);
  if (it == m_mapPos2Card.end())
    return false;
  m_mapPos2Card.erase(it);
  return true;
}

const TPairEquipCard* ItemEquip::getCard(const string& guid) const
{
  auto v = find_if(m_mapPos2Card.begin(), m_mapPos2Card.end(), [guid](const TMapEquipCard::value_type& r) -> bool{
    return r.second.first == guid;
  });
  if (v != m_mapPos2Card.end())
    return &v->second;

  return nullptr;
}

const TPairEquipCard* ItemEquip::getCardAndPos(const string& guid, DWORD& pos) const
{
  auto v = find_if(m_mapPos2Card.begin(), m_mapPos2Card.end(), [guid](const TMapEquipCard::value_type& r) -> bool{
    return r.second.first == guid;
  });
  if (v != m_mapPos2Card.end())
  {
    pos = v->first;
    return &v->second;
  }

  return nullptr;
}

void ItemEquip::getEquipCard(TVecEquipCard & rVecEquipCard) const
{
  for(auto v = m_mapPos2Card.begin(); v != m_mapPos2Card.end(); ++v)
    rVecEquipCard.push_back(v->second);

}

const TPairEquipCard* ItemEquip::getCardByPos(DWORD pos) const
{
  auto it = m_mapPos2Card.find(pos);
  if (it == m_mapPos2Card.end())
    return nullptr;
  return &it->second;
}

DWORD ItemEquip::getEmptyCardPos() const
{
  if (m_mapPos2Card.size() >= getCardSlot())
    return 0;
  for (DWORD i = 1; i <= getCardSlot(); ++i)
    if (m_mapPos2Card.find(i) == m_mapPos2Card.end())
      return i;
  return 0;
}

bool ItemEquip::checkBuffID(SceneUser* pUser, DWORD id)
{
  TPtrBufferState buffPtr = BufferManager::getMe().getBuffById(id);
  if (buffPtr == nullptr)
    return false;

  TPtrBuffCond buffCond = buffPtr->getCondition();
  if (buffCond == nullptr)
    return false;

  if (buffCond->getTrigType() == BUFFTRIGGER_ATTR || buffCond->getTrigType() == BUFFTRIGGER_PROFES)
  {
    SBufferData bData;
    bData.me = pUser;
    return buffCond->checkCondition(bData, 0);
  }

  return true;
}

bool ItemEquip::checkBuffChange(SceneUser *pUser)
{
  if (pUser == nullptr || m_pCFG == nullptr)
    return false;

  const TVecDWORD& vecAllBuffIDs = m_pCFG->vecBuffIDs;
  //vecAllBuffIDs.insert(vecAllBuffIDs.end(), m_pCFG->vecBuffIDs.begin(), m_pCFG->vecBuffIDs.end());

  DWORD oldsize = m_vecBuffIDs.size();
  m_vecBuffIDs.clear();

  for (auto v = vecAllBuffIDs.begin(); v != vecAllBuffIDs.end(); ++v)
  {
    if(checkBuffID(pUser, *v))
    {
      m_vecBuffIDs.push_back(*v);
    }
  }

  for (auto v = m_pCFG->vecRefine2Buff.begin(); v != m_pCFG->vecRefine2Buff.end(); ++v)
  {
    if ((*v).first > m_dwRefineLv)
      continue;
    for (auto b = (*v).second.begin(); b != (*v).second.end(); ++b)
    {
      m_vecBuffIDs.push_back(*b);
      if (pUser->getPackage().getPackage(EPACKTYPE_EQUIP) != nullptr && pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->getItem(getGUID()))
        pUser->m_oBuff.add(*b, pUser, m_dwRefineLv);
    }
  }

  return oldsize != m_vecBuffIDs.size();
}

bool ItemEquip::checkSameEnchant(const EnchantData& other)
{
  if (m_oEncData.type() != other.type())
    return false;

  if (m_oEncData.attrs_size() != other.attrs_size())
    return false;

  if (m_oEncData.extras_size() != other.extras_size())
    return false;

  //attrs
  {
    auto sortAttr = [](const EnchantData& data, std::map<EAttrType, DWORD>& out) {
      for (int i = 0; i < data.attrs_size(); ++i)
      {
        const EnchantAttr& rAttr = data.attrs(i);
        out[rAttr.type()] = rAttr.value();
      }
    };

    std::map<EAttrType, DWORD>  myAttrs;
    std::map<EAttrType, DWORD>  otherAttrs;
    sortAttr(m_oEncData, myAttrs);
    sortAttr(other, otherAttrs);

    if (myAttrs.size() != otherAttrs.size())
      return false;

    for (auto it1 = myAttrs.begin(), it2 = otherAttrs.begin(); it1 != myAttrs.end(); ++it1, ++it2)
    {
      if (it1->first != it2->first)
        return false;
      if (it1->second != it2->second)
        return false;
    }
  }

  //extras
  //attrs
  {
    auto sortExtra = [](const EnchantData& data, std::map<DWORD, DWORD>& out) {
      for (int i = 0; i < data.extras_size(); ++i)
      {
        const EnchantExtra& r = data.extras(i);
        out[r.configid()] = r.buffid();
      }
    };

    std::map<DWORD, DWORD>  myExtra;
    std::map<DWORD, DWORD>  otherExtra;
    sortExtra(m_oEncData, myExtra);
    sortExtra(other, otherExtra);

    if (myExtra.size() != otherExtra.size())
      return false;

    for (auto it1 = myExtra.begin(), it2 = otherExtra.begin(); it1 != myExtra.end(); ++it1, ++it2)
    {
      if (it1->first != it2->first)
        return false;
      if (it1->second != it2->second)
        return false;
    }
  }

  return true;
}

bool ItemEquip::canGenderEquip(EGender gender) const
{
  return m_pCFG != nullptr && (m_pCFG->eSexEquip == EGENDER_MIN || m_pCFG->eSexEquip == gender);
}

bool ItemEquip::canBeMaterial() const
{
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  return getCardList().empty() == true && getLv() <= 0 && getStrengthLv() <= 0 && getRefineLv() <= rCFG.dwMaxMaterialRefineLv && m_oEncData.type() == EENCHANTTYPE_MIN;
}

bool ItemEquip::canBeUpgradeMaterial() const
{
  return canBeMaterial() == true && isDamaged() == false;
}

bool ItemEquip::canBeQuickSell() const
{
  return getCardList().empty() == true && getLv() <= 0 && getStrengthLv() <= 0 && getRefineLv() < QUICK_SELL_REFINE_LESS && m_oEncData.type() == EENCHANTTYPE_MIN && isDamaged() == false;
}

bool ItemEquip::canBeComposeMaterial() const
{
  return getCardList().empty() == true && getStrengthLv() <= 0 && getStrengthLv2() <= 0 && isDamaged() == false;
}

bool ItemEquip::breakEquip(DWORD duration)
{
  if (duration <= 0)
    return false;
  DWORD cur = now();
  m_dwBreakStartTime = cur;
  m_dwBreakEndTime = cur + duration;
  return true;
}

bool ItemEquip::fixBrokenEquip()
{
  m_dwBreakStartTime = 0;
  m_dwBreakEndTime = 0;
  return true;
}

bool ItemEquip::isBroken()
{
  DWORD cur = now();
  return m_dwBreakStartTime <= cur && cur < m_dwBreakEndTime;
}

DWORD ItemEquip::getRestBreakDuration()
{
  DWORD cur = now();
  return cur > m_dwBreakEndTime ? 0 : m_dwBreakEndTime - cur;
}

void ItemEquip::setSender(QWORD charId, string& name)
{
  if (charId == 0)
    return ;
  m_oSenderData.set_charid(charId);
  m_oSenderData.set_name(name);
}

bool ItemEquip::checkSignUp(DWORD& dwPointOut, DWORD& dwBuffOut) const
{
  if(!m_pCFG) return false;

  // 检测条数
  if((DWORD)m_oEncData.attrs_size() < MiscConfig::getMe().getAuctionMiscCFG().dwEnchantAttrCount)
    return false;

  // 检测good条数
  DWORD dwAttrGoodCount = 0;
  const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(m_oEncData.type());
  if(!pCFG) return false;
  for(int i=0; i<m_oEncData.attrs_size(); ++i)
  {
    if(pCFG->isGoodEnchant(m_oEncData.attrs(i), m_pCFG->eItemType, dwPointOut))
      dwAttrGoodCount++;
  }

  if(m_oEncData.extras_size() >= 1)
    dwBuffOut = m_oEncData.extras(0).buffid();

  // 附魔数或buff数满足一个即可
  if(dwAttrGoodCount >= MiscConfig::getMe().getAuctionMiscCFG().dwEnchantAttrValuableCount)
    return true;

  if(m_oEncData.extras_size() >= (int)MiscConfig::getMe().getAuctionMiscCFG().dwEnchantBuffExtraCount)
    return true;

  return false;
}

void ItemEquip::fromEquipData(const EquipData& rData)
{
  setStrengthLv(rData.strengthlv());
  setStrengthLv2(rData.strengthlv2());
  setRefineLv(rData.refinelv());

  setCardSlot(rData.cardslot());

  addStrengthCost(rData.strengthcost());
  setDamageStatus(rData.damage());
  setLv(rData.lv());
  setBodyColor(rData.color());

  for (int i = 0; i < rData.refinecompose_size(); ++i)
    m_vecRefineCompose.push_back(TPairRefineCompose{rData.refinecompose(i).id(), rData.refinecompose(i).num()});
  for (int i = 0; i < rData.buffid_size(); ++i)
    m_vecBuffIDs.push_back(rData.buffid(i));

  m_dwBreakStartTime = rData.breakstarttime();
  m_dwBreakEndTime = rData.breakendtime();

  m_vecStrengthCost.clear();
  for (int i = 0; i < rData.strengthlv2cost_size(); ++i)
    combinItemInfo(m_vecStrengthCost, rData.strengthlv2cost(i));
}

// item treasure
ItemTreasure::ItemTreasure(const SItemCFG* pCFG) : ItemBase(pCFG)
{

}

ItemTreasure::~ItemTreasure()
{

}

ItemBase* ItemTreasure::clone()
{
  ItemTreasure* pTrea = NEW ItemTreasure(m_pCFG);
  if (pTrea == nullptr)
    return nullptr;

  pTrea->m_oItemInfo = m_oItemInfo;
  return pTrea;
}

void ItemTreasure::fromItemData(const ItemData& rData)
{
  ItemBase::fromItemData(rData);
}

bool ItemTreasure::use(SceneUser *pUser, const Cmd::ItemUse& itemUseCmd)
{
  if (pUser == nullptr || m_pCFG == nullptr)
    return false;

  if (GMCommandRuler::getMe().execute(pUser, m_pCFG->oGMData) == false)
  {
    XERR << "[物品使用-宝箱]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "use item :" << m_pCFG->dwTypeID << "cmd run error" << XEND;
    return false;
  }

  return true;
}

// item card
ItemCard::ItemCard(const SItemCFG* pCFG) : ItemBase(pCFG)
{

}

ItemCard::~ItemCard()
{

}


ItemCard* ItemCard::clone()
{
  ItemCard* pCard = NEW ItemCard(m_pCFG);
  if (pCard == nullptr)
    return nullptr;

  pCard->m_oItemInfo = m_oItemInfo;
  return pCard;
}

void ItemCard::toItemData(ItemData* pData)
{
  ItemBase::toItemData(pData);
}

void ItemCard::fromItemData(const ItemData& rData)
{
  ItemBase::fromItemData(rData);
}

// item
ItemLetter::ItemLetter(const SItemCFG* pCFG) : ItemBase(pCFG)
{
}

ItemLetter::~ItemLetter()
{
}

ItemLetter* ItemLetter::clone()
{
  ItemLetter* pItemLetter = NEW ItemLetter(m_pCFG);
  if (pItemLetter == nullptr)
    return nullptr;

  pItemLetter->m_oItemInfo = m_oItemInfo;
  pItemLetter->m_oLetterData = m_oLetterData;
  return pItemLetter;
}

void ItemLetter::toItemData(ItemData* pData)
{
  if (pData != NULL)
  {
    ItemBase::toItemData(pData);
    pData->mutable_letter()->CopyFrom(m_oLetterData);
  }
}

void ItemLetter::fromItemData(const ItemData& rData) 
{
  ItemBase::fromItemData(rData);
  m_oLetterData.CopyFrom(rData.letter());
}

// item egg
ItemEgg::ItemEgg(const SItemCFG* pCFG) : ItemEquip(pCFG)
{

}

ItemEgg::~ItemEgg()
{

}

ItemEgg* ItemEgg::clone()
{
  ItemEgg* pEgg = NEW ItemEgg(m_pCFG);
  if (pEgg == nullptr)
    return nullptr;

  pEgg->m_oItemInfo.CopyFrom(m_oItemInfo);
  pEgg->m_oEggData.CopyFrom(m_oEggData);
  return pEgg;
}

void ItemEgg::toItemData(ItemData* pData)
{
  if (pData == nullptr)
    return;

  ItemBase::toItemData(pData);
  pData->mutable_egg()->CopyFrom(m_oEggData);
}

void ItemEgg::fromItemData(const ItemData& rData)
{
  ItemBase::fromItemData(rData);
  m_oEggData.CopyFrom(rData.egg());
}

void ItemEgg::patch_1()
{
  map<DWORD, DWORD> mapBodyMonster;

  mapBodyMonster[10153] = 700010;
  mapBodyMonster[10157] = 700020;
  mapBodyMonster[10155] = 700030;
  mapBodyMonster[10161] = 700040;
  //mapBodyMonster[?????] = 700050;
  mapBodyMonster[10164] = 700060;
  mapBodyMonster[10158] = 700070;
  mapBodyMonster[10162] = 700080;
  mapBodyMonster[10159] = 700090;
  //mapBodyMonster[?????] = 700100;
  //mapBodyMonster[?????] = 700110;
  //mapBodyMonster[?????] = 700120;
  //mapBodyMonster[?????] = 700130;
  mapBodyMonster[20023] = 700140;
  mapBodyMonster[10165] = 700150;
  mapBodyMonster[10183] = 700160;

  for (int i = 0; i < m_oEggData.unlock_body_size(); ++i)
  {
    auto it = mapBodyMonster.find(m_oEggData.unlock_body(i));
    if (it != mapBodyMonster.end())
    {
      XLOG << "[宠物蛋-补丁], body替换, 原body:" << m_oEggData.unlock_body(i) << "替换后body:" << it->second << XEND;
      m_oEggData.set_unlock_body(i, it->second);
    }
  }
  auto it = mapBodyMonster.find(m_oEggData.body());
  if (it != mapBodyMonster.end())
  {
    XLOG << "[宠物蛋-补丁], 穿戴body替换, 原body:" << m_oEggData.body() << "替换后body:" << it->second << XEND;
    m_oEggData.set_body(it->second);
  }
}

bool ItemEgg::canSell() const
{
  const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(m_oEggData.id());
  if (pCFG == nullptr)
    return false;

  TSetDWORD myskills;
  for (int i = 0; i < m_oEggData.skillids_size(); ++i)
    myskills.insert(m_oEggData.skillids(i));

  // 满级技能
  TSetDWORD fullskills;
  pCFG->getFullSkill(fullskills);
  for (auto &s : fullskills)
  {
    if (myskills.find(s) == myskills.end())
      return false;
  }
  return true;
}

DWORD ItemEgg::getWorkSkillID(const EggData& rEgg)
{
  const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(rEgg.id());
  if (pCFG == nullptr)
    return 0;

  for (int i = 0; i < rEgg.skillids_size(); ++i)
  {
    if (pCFG->dwWorkSkillID == rEgg.skillids(i) / 1000)
      return rEgg.skillids(i);
  }
  return 0;
}

bool ItemEgg::canEquip(EProfession profession) const
{
  if (ItemEquip::canEquip(profession) == false)
    return false;

  const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(getPetID());
  if (pCFG == nullptr || pCFG->bCanEquip == false)
    return false;

  if (pCFG->stEquipCondtion.dwFriendLv && getFriendLv() < pCFG->stEquipCondtion.dwFriendLv)
    return false;

  return true;
}

// item
ItemCode::ItemCode(const SItemCFG* pCFG) : ItemBase(pCFG)
{
}

ItemCode::~ItemCode()
{
}

ItemCode* ItemCode::clone()
{
  ItemCode* pItemCode = NEW ItemCode(m_pCFG);
  if (pItemCode == nullptr)
    return nullptr;

  pItemCode->m_oItemInfo = m_oItemInfo;
  pItemCode->m_oCodeData = m_oCodeData;
  return pItemCode;
}

void ItemCode::toItemData(ItemData* pData)
{
  if (pData != NULL)
  {
    ItemBase::toItemData(pData);
    pData->mutable_code()->CopyFrom(m_oCodeData);
  }
}

void ItemCode::fromItemData(const ItemData& rData)
{
  ItemBase::fromItemData(rData);
  m_oCodeData.CopyFrom(rData.code());
}

bool ItemCode::canUse() const
{
  return m_oCodeData.code().empty();
}

bool ItemCode::needReqUsed() const
{
  if (!m_oCodeData.code().empty() && m_oCodeData.used() == false)
    return true;
  return false;
}

void ItemCode::setCode(const string& code)
{
  if (code.empty())
    return;
  m_oCodeData.set_code(code);
}

// item wedding
ItemWedding::ItemWedding(const SItemCFG* pCFG) : ItemBase(pCFG)
{
}

ItemWedding::~ItemWedding()
{
}

ItemWedding* ItemWedding::clone()
{
  ItemWedding* pItemWedding = new ItemWedding(m_pCFG);
  if (pItemWedding == nullptr)
    return nullptr;

  pItemWedding->m_oItemInfo = m_oItemInfo;
  pItemWedding->m_oWeddingData = m_oWeddingData;
  return pItemWedding;
}

void ItemWedding::toItemData(ItemData* pData)
{
  if (pData == nullptr)
    return;
  ItemBase::toItemData(pData);
  pData->mutable_wedding()->CopyFrom(m_oWeddingData);
}

void ItemWedding::fromItemData(const ItemData& rData)
{
  ItemBase::fromItemData(rData);
  m_oWeddingData.CopyFrom(rData.wedding());
}

bool ItemWedding::uploadPhoto(DWORD index, DWORD time)
{
  m_oWeddingData.set_photoidx(index);
  m_oWeddingData.set_phototime(time);
  return true;
}

bool ItemWedding::isNotifyWeddingStart()
{
  if (m_oWeddingData.notified())
    return false;
  DWORD cur = now();
  return cur >= m_oWeddingData.starttime() && cur < m_oWeddingData.endtime();
}
