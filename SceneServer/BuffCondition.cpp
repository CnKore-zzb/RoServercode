/**
 * @file BuffCondition.cpp
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-29
 */
#include "BuffCondition.h"
#include "BufferState.h"
#include "RoleDataConfig.h"
#include "ProtoCommon.pb.h"
#include "xSceneEntryDynamic.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SkillItem.h"

float FmData::getFmValue(xSceneEntryDynamic* me, const SBufferData* pData)
{
  if (!me || !pData) return 0.0f;
  UINT fromID = pData->fromID; 
  DWORD calcLv =  pData->lv % 100;
  xSceneEntryDynamic *entry = xSceneEntryDynamic::getEntryByID(fromID); //notice: entry没有判断，允许传入null, 但使用需判断
  float realValue = 0.0f;
  switch(formula)
  {
    case NO_FORMULA:
      {
        realValue = value;
      }
    break;
    case SLV_FORMULA:
      {
        realValue = calcLv * (getParams(0)) + getParams(1);
      }
    break;
    case LEVSAR_FORMULA:
      //a+b*(1+(BaseLv-TargetLv)/100) - StateAbnorResit
      {
        if (!entry) return 0;
        float a = getParams(0);
        float b = getParams(1);
        float bLv = entry->getLevel();
        float tarLv = me->getLevel();
        float sar = 0; //me->getAttr(EATTRTYPE_SAR);
        realValue = a + b * (1 + (bLv - tarLv)/100) - sar;
      }
      break;
    case SLVDEX_FORMULA:
      {
        if (!entry) return 0;
        float a = getParams(0);
        float b = getParams(1);
        float c = getParams(2);
        realValue = entry->getAttr(EATTRTYPE_DEX) * a + calcLv * b + c;
      }
      break;
    case SLVAGI_FORMULA:
      {
        if (!entry) return 0;
        float a = getParams(0);
        float b = getParams(1);
        float c = getParams(2);
        realValue = entry->getAttr(EATTRTYPE_AGI) * a + calcLv * b + c;
      }
      break;
    case HPRECOVER_FORMULA:
      {
        if (!entry) return 0;
        //realValue = entry->getAttr(EATTRTYPE_MAXHP) * entry->getAttr(EATTRTYPE_RESTORESPD);
        realValue = entry->getAttr(EATTRTYPE_RESTORESPD);
      }
      break;
    case HPREDUCE_FORMULA:
      {
        float a = getParams(0);
        float b = getParams(1);
        realValue = me->getAttr(EATTRTYPE_MAXHP) * a + b;
      }
      break;
    case SPRECOVER_FORMULA:
      {
        if (!entry) return 0;
        //realValue = entry->getAttr(EATTRTYPE_MAXSP) * entry->getAttr(EATTRTYPE_SPRESTORESPD);
        realValue = entry->getAttr(EATTRTYPE_SPRESTORESPD);
      }
      break;
    case SPECSKILL_FORMULA:
      {
        if (entry == nullptr)
          return 0;
        SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
        if (pUser == nullptr || pUser->getFighter() == nullptr)
          return 0;
        DWORD skillid = getParams(0) * 1000;
        DWORD lv = pUser->getFighter()->getSkill().getSkillLv(skillid);
        float a = getParams(1);
        float b = getParams(2);
        realValue = lv * a + b;
      }
      break;
    case INSTATUS_FORMULA:
      {
        DWORD sta = getParams(0);
        realValue = (me->m_oBuff.isInStatus(sta) == true) ? 100 : 0;
      }
      break;
    case TURNUNDEAD_FORMULA:
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
        if (npc == nullptr || npc->getRaceType() != ERACE_UNDEAD)
          return 0;

        if (entry == nullptr)
          return 0;

        float lv = calcLv;
        float luk = entry->getAttr(EATTRTYPE_LUK);
        float intt = entry->getAttr(EATTRTYPE_INT);
        float baselv = entry->getLevel();
        float hp = me->getAttr(EATTRTYPE_HP);
        float maxhp = me->getAttr(EATTRTYPE_MAXHP);

        float a = getParams(0);
        float b = getParams(1);
        float c = getParams(2);

        if (c == 0)
          return 0;
        realValue = (lv * a + luk + intt + baselv + (1.0f - hp / maxhp) * b) / c * 100;
      }
      break;
    case GETSTATUS_FORMULA:
      {
        if (entry == nullptr)
          return 0;
        DWORD status = getParams(0);
        float luk2 = me->getAttr(EATTRTYPE_LUK);
        float blv1 = entry->getLevel();
        float blv2 = me->getLevel();
        switch(status)
        {
          case EBUFFSTATUS_POSION:
            {
              float vit2 = me->getAttr(EATTRTYPE_VIT);
              float poisondef2 = me->getAttr(EATTRTYPE_POSIONDEF);
              realValue = 50 * ((200 - vit2 - luk2 / 2) / 200 - poisondef2) * ( 1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_BLOOD:
            {
              float agi2 = me->getAttr(EATTRTYPE_AGI);
              realValue = 50 * ((200 - agi2 - luk2 / 2) / 200) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_BURN:
            {
              float int2 = me->getAttr(EATTRTYPE_INT);
              float blinddef2 = me->getAttr(EATTRTYPE_BLINDDEF);
              realValue = 50 * ((200 - int2 - luk2 / 2) / 200 - blinddef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_DIZZY:
            {
              float vit2 = me->getAttr(EATTRTYPE_VIT);
              float stundef2 = me->getAttr(EATTRTYPE_STUNDEF);
              realValue = 50 * ((200 - vit2 - luk2 / 2) / 200 - stundef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_FREEZE:
            {
              // 对不死属性无效
              if (me->getAttr(EATTRTYPE_DEFATTR) == EELEMENTTYPE_UNDEAD)
              {
                realValue = 0;
                break;
              }
              float mdef2 = me->getAttr(EATTRTYPE_MDEF);
              float freezedef2 = me->getAttr(EATTRTYPE_FREEZEDEF);
              realValue = 50 * ((200 - mdef2 * 0.08 - luk2 / 2) / 200 - freezedef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_STONE:
            {
              // 对不死属性无效
              if (me->getAttr(EATTRTYPE_DEFATTR) == EELEMENTTYPE_UNDEAD)
              {
                realValue = 0;
                break;
              }
              float mdef2 = me->getAttr(EATTRTYPE_MDEF);
              float stonedef2 = me->getAttr(EATTRTYPE_STONEDEF);
              realValue = 50 * ((200 - mdef2 * 0.08 - luk2 / 2) / 200 - stonedef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_SLEEP:
            {
              float int2 = me->getAttr(EATTRTYPE_INT);
              realValue = 50 * ((200 - int2 - luk2 / 2) / 200) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_FEAR:
            {
              float int2 = me->getAttr(EATTRTYPE_INT);
              float chaosdef2 = me->getAttr(EATTRTYPE_CHAOSDEF);
              realValue = 50 * ((200 - int2 - luk2 / 2) / 200 - chaosdef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_NOMOVE:
            {
              float agi2 = me->getAttr(EATTRTYPE_AGI);
              float slowdef2 = me->getAttr(EATTRTYPE_SLOWDEF);
              realValue = 50 * ((200 - agi2 - luk2 / 2) / 200 - slowdef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_SILENCE:
            {
              float vit2 = me->getAttr(EATTRTYPE_VIT);
              float silencedef2 = me->getAttr(EATTRTYPE_SILENCEDEF);
              realValue = 50 * ((200 - vit2 - luk2 / 2) / 200 - silencedef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
          case EBUFFSTATUS_CURSE:
            {
              float dex2 = me->getAttr(EATTRTYPE_DEX);
              float cursedef2 = me->getAttr(EATTRTYPE_CURSEDEF);
              realValue = 50 * ((200 - dex2 - luk2 / 2) / 200 - cursedef2) * (1 + (blv1 - blv2) / 15);
            }
            break;
        }
        realValue = realValue < 0 ? 0 : realValue;
      }
      break;
    case INACTION_FORMULA:
      {
        DWORD actionid = getParams(0);
        if (me->getAction() != actionid)
          return 0;
        if (eType == EATTRTYPE_SP)
          realValue = getParams(1) * me->getAttr(EATTRTYPE_MAXSP);
        else if (eType == EATTRTYPE_HP)
          realValue = getParams(1) * me->getAttr(EATTRTYPE_MAXHP);
      }
      break;
    case FOURTEEN_FORMULA:
      {
        float a = getParams(0);
        float b = getParams(1);
        float c = getParams(2);
        float vit = me->getAttr(EATTRTYPE_VIT);
        realValue = vit * a + calcLv * b + c;
      }
      break;
    case FIFTEEN_FORMULA:
      {
        float a = getParams(0);
        float b = getParams(1);
        float c = getParams(2);
        float maxhp = me->getAttr(EATTRTYPE_MAXHP);
        float maxsp = me->getAttr(EATTRTYPE_MAXSP);
        realValue = maxhp * a + maxsp * b + calcLv * c;
      }
      break;
    case SKILLDAMAGE_FORMULA:
      {
        float a = getParams(0);
        float b = getParams(1);
        realValue = pData->dwDamage * (a * calcLv + b);
      }
      break;
    case HIDE_FORMULA:
      {
        if (entry == nullptr || entry->getAttr(EATTRTYPE_HIDE) == 0)
          return 0;
        float a = getParams(0);
        float b = getParams(1);
        realValue = calcLv * a + b;
      }
      break;
    case EIGHTEEN_FORMULA:
      {
        SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
        if (pUser == nullptr || pUser->getFighter() == nullptr)
          return 0;
        DWORD skillid = getParams(0) * 1000;
        DWORD lv = pUser->getFighter()->getSkill().getSkillLv(skillid);
        DWORD baselv = me->getLevel();
        return lv * baselv * getParams(1) + getParams(2);
      }
      break;
    case NINETEEN_FROMULA:
      {
        if (entry == nullptr)
          return 0;
        float intt = me->getAttr(EATTRTYPE_INT);
        float deltaLv = entry->getLevel() - me->getLevel();
        deltaLv = deltaLv < 0 ? 0 : deltaLv;
        if (getParams(3) == 0) return 0;
        return intt * getParams(0) + calcLv * getParams(1) + deltaLv * getParams(2) / getParams(3);
      }
      break;
    default:
      {
        SLuaParams sParams;
        sParams.init(getParams(0), getParams(1), getParams(2), getParams(3), calcLv);
        sParams.add(pData->dwDamage);
        return LuaManager::getMe().call<float>("calcBuffValue", entry, me, &sParams, DWORD(formula));
      }
    break;
  }
  return realValue;
}

BaseCondition::BaseCondition()
{

}
BaseCondition::~BaseCondition()
{

}

bool BaseCondition::init(xLuaData &data)
{
  bool bCorrect = true;
  if (data.has("profession"))
  {
    xLuaData& profe = data.getMutableData("profession");
    auto f = [this, &bCorrect](const std::string& key, xLuaData &value)
    {
      if (value.getInt()<EPROFESSION_MIN || value.getInt()>EPROFESSION_MAX)
      {
        XERR << "[Buffer-Condition] profession :" << value.getInt() << "不合法" << XEND;
        bCorrect = false;
        return;
      }
      m_setProfession.insert(static_cast<EProfession>(value.getInt()));
    };
    profe.foreach(f);
  }
  if (data.has("equipType"))
  {
    xLuaData& equip = data.getMutableData("equipType");
    auto f = [this, &bCorrect] (const std::string& key, xLuaData& value)
    {
      DWORD t = value.getInt();
      if (t <=  EITEMTYPE_MIN || t >= EITEMTYPE_MAX || EItemType_IsValid(t) == false)
      {
        XERR << "[Buffer-Condition] equipType :" << t << "不合法" << XEND;
        bCorrect = false;
        return;
      }
      m_setEquipType.insert(t);
    };
    equip.foreach(f);
  }
  if (data.has("noMapType"))
  {
    m_setNoMapTypes.clear();
    auto maptypef = [&](const string& k, xLuaData& d)
    {
      m_setNoMapTypes.insert(static_cast<EMapType>(d.getInt()));
    };
    data.getMutableData("noMapType").foreach(maptypef);
  }
  return bCorrect;
}

bool BaseCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  return checkBase(bData.me);
}

bool BaseCondition::checkBase(xSceneEntryDynamic* entry)
{
  if (!m_setProfession.empty())
  {
    SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
    if (!pUser)
      return false;
    if (m_setProfession.find(pUser->getUserSceneData().getProfession()) == m_setProfession.end())
      return false;
  }

  if (!m_setEquipType.empty())
  {
    SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
    if (!pUser)
      return false;
    if (pUser->getPackage().getPackage(EPACKTYPE_EQUIP) == nullptr)
      return false;
    TVecSortItem pVecItems;
    pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->getEquipItems(pVecItems);
    bool find = false;
    for (auto m = pVecItems.begin(); m != pVecItems.end(); ++m)
    {
      //if ((*m)->getType() == m_dwEquipType)
      if (m_setEquipType.find((*m)->getType()) != m_setEquipType.end())
      {
        find = true;
        break;
      }
    }
    if (!find) return false;
  }

  if (!m_setNoMapTypes.empty())
  {
    Scene* pScene = entry ? entry->getScene() : nullptr;
    if (!pScene)
      return false;
    switch(pScene->getSceneType())
    {
      case SCENE_TYPE_PVECARD:
        if (m_setNoMapTypes.find(EMAPTYPE_PVE) != m_setNoMapTypes.end())
          return false;
        break;
      case SCENE_TYPE_GUILD_FIRE:
      case SCENE_TYPE_SUPERGVG:
        if (m_setNoMapTypes.find(EMAPTYPE_GVG) != m_setNoMapTypes.end())
          return false;
        break;
      case SCENE_TYPE_TEAMPWS:
        if (m_setNoMapTypes.find(EMAPTYPE_TEMAPWS) != m_setNoMapTypes.end())
          return false;
        break;
      default:
        if (pScene->isPVPScene())
        {
          if (m_setNoMapTypes.find(EMAPTYPE_PVP) != m_setNoMapTypes.end())
            return false;
        }
        break;
    }
  }

  return true;
}

bool AttrCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;

  bool bCorrect = true;
  auto index = [this, &bCorrect](std::string key, xLuaData &data)
  {
    if (key == "type")
      return;

    DWORD id = RoleDataConfig::getMe().getIDByName(key.c_str());
    if (id == 0 || EAttrType_IsValid(id) == false)
    {
      return;
    }

    if (id && Cmd::EAttrType_IsValid(id))
    {
      Cmd::UserAttrSvr uAttr;
      uAttr.set_type((Cmd::EAttrType)id);
      uAttr.set_value(data.getFloat());
      m_uAttr.push_back(uAttr);
    }
  };
  data.foreach(index);
  return bCorrect;
}
bool AttrCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  for (auto m = m_uAttr.begin(); m != m_uAttr.end(); ++m)
  {
    if (entry->getAttr((*m).type()) < (*m).value())
      return false;
  }
  return true;
}

//装备
bool ItemCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;

  bool bCorrect = true;

  if (data.has("itemTypes")) {
    xLuaData itemTypes = data.getData("itemTypes");
    auto func = [&](std::string key, xLuaData v) {
      if (v.getInt() <= EITEMTYPE_MIN || v.getInt() >= EITEMTYPE_MAX || EItemType_IsValid(v.getInt()) == false) {
        XERR << "[BuffCondition-ItemCondition] itemTypes :不合法" << v.getInt() << XEND;
        bCorrect = false;
        return;
      }
      m_itemTypes.insert(v.getInt());
    };
    itemTypes.foreach(func);
  } else if (data.has("noWeaponTypes")) {
    xLuaData noWeaponTypes = data.getData("noWeaponTypes");
    auto func = [&](std::string key, xLuaData v) {
      if (v.getInt() <= EITEMTYPE_MIN || v.getInt() >= EITEMTYPE_MAX || EItemType_IsValid(v.getInt()) == false) {
        XERR << "[BuffCondition-ItemCondition] noWeaponTypes :不合法" << v.getInt() << XEND;
        bCorrect = false;
        return;
      }
      m_noWeaponTypes.insert(v.getInt());
    };
    noWeaponTypes.foreach(func);
  } else {
    DWORD value = data.getTableInt("typeID");
    if (value != 0 && (value <= EITEMTYPE_MIN || value >= EITEMTYPE_MAX || EItemType_IsValid(value) == false))
    {
      XERR << "[BuffCondition-ItemCondition] typeID :" << value << "不合法" << XEND;
      bCorrect = false;
      return bCorrect;
    }
    itemValue = static_cast<EItemType> (value);

    m_dwItemID = data.getTableInt("itemID");
    if (data.has("profession"))
    {
      xLuaData profe = data.getData("profession");
      auto f = [this, &bCorrect](std::string key, xLuaData &value)
        {
          if (value.getInt()<EPROFESSION_MIN || value.getInt()>EPROFESSION_MAX)
          {
            XERR << "[BuffCondition-ItemCondition] pro :不合法" << value.getInt() << XEND;
            bCorrect = false;
            return;
          }
          m_vecProfession.push_back(static_cast<EProfession>(value.getInt()));
        };
      profe.foreach(f);
    }

    if (m_dwItemID != 0 && ItemConfig::getMe().getItemCFG(m_dwItemID) == nullptr)
    {
      XERR << "[BuffCondition-ItemCondition] itemID :" << m_dwItemID << "未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
      return bCorrect;
    }
    if (data.has("no_items"))
    {
      auto getitem = [&](const string& key, xLuaData& d)
      {
        m_setNoItemIDs.insert(d.getInt());
      };
      data.getMutableData("no_items").foreach(getitem);
    }
  }

  m_bNoWeaponOk = data.getTableInt("no_weapon_ok") == 1;
  m_bDelWhenInvalid = data.getTableInt("delWhenInvalid") == 1;

  return bCorrect;
}

bool ItemCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
  if (!pUser)
    return false;

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return false;

  if (m_bNoWeaponOk)
  {
    ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_WEAPON);
    if (pEquip == nullptr)
      return true;
  }

  if (m_noWeaponTypes.size() > 0)
  {
    ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_WEAPON);
    if (pEquip == nullptr)
      return false;

    if (m_noWeaponTypes.find(pEquip->getType()) == m_noWeaponTypes.end())
      return true;

    return false;
  }

  TVecSortItem pVecItems;
  pEquipPack->getEquipItems(pVecItems);

  if (m_setNoItemIDs.empty() == false)
  {
    for (auto &m : pVecItems)
    {
      if (m_setNoItemIDs.find(m->getTypeID()) != m_setNoItemIDs.end())
        return false;
    }
    return true;
  }

  for (auto m = pVecItems.begin(); m != pVecItems.end(); ++m)
  {
    if (m_itemTypes.size() > 0)
    {
      if (m_itemTypes.find((*m)->getType()) != m_itemTypes.end())
        return true;
    }
    else
    {
      if (itemValue != EITEMTYPE_MIN  && (*m)->getType() != itemValue)
        continue;

      if (m_dwItemID != 0)
      {
        bool bfind = false;
        if (m_dwItemID == (*m)->getTypeID())
          bfind = true;

        if (!bfind)
        {
          ItemEquip* pEquip = dynamic_cast<ItemEquip*> (*m);
          if (pEquip)
          {
            const TMapEquipCard& cardlist = pEquip->getCardList();
            for (auto &c : cardlist)
            {
              const SItemCFG* pItem = ItemManager::getMe().getItemCFG(c.second.second);
              if (pItem && pItem->dwTypeID == m_dwItemID)
              {
                bfind = true;
                break;
              }
            }
          }
        }
        if (!bfind)
          continue;
      }

      if (m_vecProfession.size() > 0)
      {
        if (0 == count(m_vecProfession.begin(), m_vecProfession.end(), pUser->getFighter()->getProfession()))
          continue;
      }
      return true;
    }
  }

  return false;
}

//职业条件
bool ProfesCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;

  bool bCorrect = true;
  xLuaData pros = data.getData("value");
  auto funp = [&](const string& key, xLuaData& xdata)
  {
    DWORD value = xdata.getInt();
    if (value <= EPROFESSION_MIN || value >= EPROFESSION_MAX || EProfession_IsValid(value) == false)
    {
      bCorrect = false;
      return;
    }
    EProfession profes = static_cast<EProfession> (value);
    m_vecProfession.push_back(profes);
  };
  pros.foreach(funp);
  return bCorrect;
}

bool ProfesCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
  if (!pUser)
    return false;
  for (auto v = m_vecProfession.begin(); v != m_vecProfession.end(); ++v)
  {
    if (pUser->getUserSceneData().getProfession() == *v)
      return true;
  }
  return false;
}

//骑乘条件
bool RideCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;

  auto mountf = [&](const string& key, xLuaData& data)
  {
    m_setMountID.insert(data.getInt());
  };
  data.getMutableData("mount").foreach(mountf);

  return true;
}

bool RideCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  SceneUser* user = dynamic_cast<SceneUser*>(bData.me);
  if (user == nullptr)
    return false;

  EquipPackage* pack = dynamic_cast<EquipPackage*>(user->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pack == nullptr)
    return false;

  ItemEquip* equip = pack->getEquip(EEQUIPPOS_MOUNT);
  if (equip == nullptr)
    return false;

  return m_setMountID.find(equip->getTypeID()) != m_setMountID.end();
}

//杀死怪物条件
bool KillCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;

  xLuaData RaceData = data.getData("Value");
  auto RaceIndex  = [this](std::string key, xLuaData &data)
  {
     m_vecRaceType.push_back(static_cast<ERaceType>(data.getInt()));
  };
  RaceData.foreach(RaceIndex);

  m_bNormalSkillOk = data.getTableInt("normal_skill") == 1;
  m_bPhySkillOk = data.getTableInt("physic_skill") == 1;
  m_bMagicSkillOk = data.getTableInt("magic_skill") == 1;
  m_bAnySkillOk = data.getTableInt("all_skill") == 1;

  m_bJustLockSkill = data.getTableInt("just_lockSkill") == 1;
  m_bJustLongSkill = data.getTableInt("just_longSkill") == 1;
  m_bJustNearSkill = data.getTableInt("just_nearSkill") == 1;
  return true;
}

bool KillCondition::isTheRace(xSceneEntryDynamic* pMon, const BaseSkill* pSkill)
{
  if (pMon == nullptr || pSkill == nullptr)
    return false;
  SceneNpc* pNpc = dynamic_cast<SceneNpc*> (pMon);
  if (pNpc == nullptr)
    return false;

  if (m_bJustLockSkill && pSkill->getLogicType() != ESKILLLOGIC_LOCKEDTARGET)
    return false;
  if (m_bMagicSkillOk && pSkill->isMagicSkill() == false)
    return false;

  ERaceType eType = ERACE_MIN;
  for(auto it = m_vecRaceType.begin(); it != m_vecRaceType.end(); ++it)
  {
    if (*it == ERACE_MIN)
      return true;
    eType = static_cast<ERaceType>(pNpc->getRaceType());
    if(*it == eType)
      return true;
  }
  return false;
}

bool AllEquipCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser *pUser = dynamic_cast<SceneUser*>(entry);
  if (nullptr == pUser)
    return false;
  //check user'equip  if all equiped
  TVecSortItem vecEquipItem;
  pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->getEquipItems(vecEquipItem);
  for (auto v = m_vecEquipID.begin(); v != m_vecEquipID.end(); ++v)
  {
    auto it = find_if(vecEquipItem.begin(), vecEquipItem.end(), [&](ItemBase *pItem)
    {
      return (pItem->getTypeID() == *v);
    });

    if (vecEquipItem.end() == it)
      return false;
  }
  return true;
}

bool AllEquipCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;

  bool bCorrect = true;
  xLuaData equipData = data.getData("value");
  auto equipIDF = [this, &bCorrect](std::string key, xLuaData &data)
  {
    if (ItemConfig::getMe().getItemCFG(data.getInt()) == nullptr)
    {
      XERR << "[BuffCondition-AllEquipCondition] value :" << data.getInt() << "未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
      return;
    }
    m_vecEquipID.push_back(data.getInt());
  };
  equipData.foreach(equipIDF);
  return bCorrect;
}

bool AllCardCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;

  bool bCorrect = true;
  xLuaData cardData = data.getData("value");
  auto cardIDF = [this, &bCorrect](std::string key, xLuaData &data)
  {
    if (ItemConfig::getMe().getItemCFG(data.getInt()) == nullptr)
    {
      XERR << "[BuffCondition-AllEquipCondition] value :" << data.getInt() << "未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
      return;
    }
    m_vecCardID.push_back(data.getInt());
  };
  cardData.foreach(cardIDF);
  return bCorrect;
}

bool AllCardCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser *pUser = dynamic_cast<SceneUser*>(entry);
  if (nullptr == pUser)
    return false;
  if (pUser->getPackage().getPackage(EPACKTYPE_EQUIP) == nullptr)
    return false;
  //check if all card has been equiped;
  TVecSortItem vecEquipItem;
  pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->getEquipItems(vecEquipItem);
  DWORD dwEquipCount = 0;

  TVecEquipCard vecCardEquip;
  for (auto v = m_vecCardID.begin(); v != m_vecCardID.end(); ++v)
  {
    vecCardEquip.clear();
    for (auto e = vecEquipItem.begin(); e != vecEquipItem.end(); ++e)
    {
      ItemEquip * pEquipItem = dynamic_cast<ItemEquip *>(*e);
      if (nullptr == pEquipItem)
        continue;
      pEquipItem->getEquipCard(vecCardEquip);
      auto it = find_if(vecCardEquip.begin(), vecCardEquip.end(), [&](TPairEquipCard paircard)
      {
        return  (paircard.second == *v);
      });

      if (vecCardEquip.end() != it)
      {
        dwEquipCount++;
        continue;
      }
    }
  }
  if (dwEquipCount < m_vecCardID.size())
    return false;
  return true;
}

bool EqRefineCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  refineLv = data.getTableInt("refine");
  return true;
}

bool TimeCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  afterTime = data.getTableInt("time");
  fakeDead = (data.getTableInt("fakedead") != 0);
  return true;
}

bool TimeCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  if (fakeDead == true)
  {
    if (entry->getStatus() != ECREATURESTATUS_FAKEDEAD)
    {
      auto pBuff = bData.pBuff;
      if (pBuff == nullptr)
        return false;
      entry->m_oBuff.del(bData.id);
      return false;
    }
  }
  return cur >= bData.addTime + afterTime;
}

DamageCondition::DamageCondition()
{
  trigType = BUFFTRIGGER_DAMAGE;
}

DamageCondition::~DamageCondition()
{

}

bool DamageCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  m_dwDamagePer = data.getTableFloat("value") / 100.0f;
  return true;
}

bool DamageCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  DWORD hpPoint = bData.hpOnAdd * (1.0f - m_dwDamagePer);
  DWORD nowhp = entry->getAttr(EATTRTYPE_HP);
  return nowhp <= hpPoint;
}

AttackCondition::AttackCondition()
{
  trigType = BUFFTRIGGER_ATTACK;
}

AttackCondition::~AttackCondition()
{

}
bool AttackCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  xLuaData& cri = data.getMutableData("criOdds");
  m_stCriOdds.load(cri);
  xLuaData& nom = data.getMutableData("normalOdds");
  m_stNomOdds.load(nom);

  m_bNormalSkillOk = data.getTableInt("normal_skill") == 1;
  m_bPhySkillOk = data.getTableInt("physic_skill") == 1;
  m_bMagicSkillOk = data.getTableInt("magic_skill") == 1;
  m_bAnySkillOk = data.getTableInt("all_skill") == 1;

  m_bJustLockSkill = data.getTableInt("just_lockSkill") == 1;
  m_bJustLongSkill = data.getTableInt("just_longSkill") == 1;
  m_bJustNearSkill = data.getTableInt("just_nearSkill") == 1;
  m_bJustPointSkill = data.getTableInt("just_pointSkill") == 1;
  m_bJustReadySkill = data.getTableInt("just_readySkill") == 1;
  m_bNoNormalSkill = data.getTableInt("no_normalSkill") == 1;
  m_dwNeedAtkAttr = data.getTableInt("need_AtkAttr");
  m_dwNeedDamage = data.getTableInt("need_damage");

  m_fmHpMore.load(data.getMutableData("hp_more"));
  m_fmHpLess.load(data.getMutableData("hp_less"));
  m_fmTarHpThan.load(data.getMutableData("target_hp_more"));
  m_fmTarHpLess.load(data.getMutableData("target_hp_less"));

  if (data.has("need_race"))
  {
    auto getrace = [&](const string& k, xLuaData& d)
    {
      m_setNeedRaces.insert(d.getInt());
    };
    data.getMutableData("need_race").foreach(getrace);
  }

  if (data.has("need_skill"))
  {
    data.getMutableData("need_skill").getIDList(m_setNeedSkillID);
  }
  return true;
};

bool AttackCondition::isCheckOk(xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy, const BaseSkill* pSkill, DamageType damtype, const SBufferData* pData)
{
  if (attacker == nullptr || enemy == nullptr || pSkill == nullptr)
    return false;

  if (checkBase(attacker) == false)
    return false;

  // 除普攻技能
  if (m_bNoNormalSkill && pSkill->isNormalSkill(attacker))
    return false;

  // 仅锁定单体技能
  if (m_bJustLockSkill && pSkill->getLogicType() != ESKILLLOGIC_LOCKEDTARGET)
    return false;

  // 仅远程技能
  if (m_bJustLongSkill && pSkill->isLongSkill() == false)
    return false;

  // 仅近战技能
  if (m_bJustNearSkill && pSkill->isLongSkill() == true)
    return false;

  // 仅区域型技能
  if (m_bJustPointSkill && pSkill->isPointSkill() == false)
    return false;

  // 攻击目标血量
  if (!m_fmTarHpThan.empty() || !m_fmTarHpLess.empty())
  {
    float hp = enemy->getAttr(EATTRTYPE_HP);
    float maxhp = enemy->getAttr(EATTRTYPE_MAXHP);
    float per = (maxhp != 0 ? hp/maxhp : 0);
    if (!m_fmTarHpThan.empty())
    {
      float hppermore = m_fmTarHpThan.getFmValue(attacker, pData);
      if (per < hppermore)
        return false;
    }
    if (!m_fmTarHpLess.empty())
    {
      float hpperless = m_fmTarHpLess.getFmValue(attacker, pData);
      if (per > hpperless)
        return false;
    }
  }

  // 自身血量
  if (!m_fmHpMore.empty() || !m_fmHpLess.empty())
  {
    float hp = attacker->getAttr(EATTRTYPE_HP);
    float maxhp = attacker->getAttr(EATTRTYPE_MAXHP);
    float per = (maxhp != 0 ? hp/maxhp : 0);
    if (!m_fmHpMore.empty())
    {
      float hpmore = m_fmHpMore.getFmValue(attacker, pData);
      if (per < hpmore)
        return false;
    }
    if (!m_fmHpLess.empty())
    {
      float hpless = m_fmHpLess.getFmValue(attacker, pData);
      if (per > hpless)
        return false;
    }
  }

  // 种族限制
  if (!m_setNeedRaces.empty())
  {
    DWORD race = enemy->getRaceType();
    if (m_setNeedRaces.find(race) == m_setNeedRaces.end())
      return false;
  }

  // 指定属性
  if (m_dwNeedAtkAttr != 0)
  {
    DWORD myAtkAttr = attacker->getTempAtkAttr();
    myAtkAttr = myAtkAttr == 0 ? 5 : myAtkAttr; // 默认无属性5
    if (myAtkAttr != m_dwNeedAtkAttr)
      return false;
  }

  // 伤害达到某数值
  if (m_dwNeedDamage && pData && (DWORD)pData->dwDamage < m_dwNeedDamage)
    return false;

  // 指定技能
  if (!m_setNeedSkillID.empty())
  {
    DWORD id = pSkill->getSkillID() / ONE_THOUSAND;
    return m_setNeedSkillID.find(id) != m_setNeedSkillID.end();
  }

  // 任意技能
  if (m_bAnySkillOk)
    return true;

  // 物理技能
  if (m_bPhySkillOk && pSkill->isPhySkill())
    return true;

  // 魔法技能
  if (m_bMagicSkillOk && pSkill->isMagicSkill())
    return true;

  // 普攻技能
  if (m_bNormalSkillOk && pSkill->isNormalSkill(attacker))
  {
    if (m_stNomOdds.empty() && m_stCriOdds.empty())
      return true;

    DWORD criodds = m_stCriOdds.getFmValue(enemy, pData);
    DWORD nomodds = m_stNomOdds.getFmValue(enemy, pData);
    DWORD odds = randBetween(1, 100);
    if (damtype == DAMAGE_TYPE_CRITICAL)
    {
      return criodds >= odds;
    }
    else
    {
      return nomodds >= odds; 
    }
  }

  return false;
}

UseSkillCondition::UseSkillCondition()
{
  trigType = BUFFTRIGGER_USESKILL;
}

UseSkillCondition::~UseSkillCondition()
{

}

bool UseSkillCondition::isCheckOk(const BaseSkill* pSkill, xSceneEntryDynamic* attacker)
{
  // 除普攻技能
  if (m_bNoNormalSkill && pSkill->isNormalSkill(attacker))
    return false;

  // 仅锁定单体技能
  if (m_bJustLockSkill && pSkill->getLogicType() != ESKILLLOGIC_LOCKEDTARGET)
    return false;

  // 仅远程技能
  if (m_bJustLongSkill && pSkill->isLongSkill() == false)
    return false;

  // 仅近战技能
  if (m_bJustNearSkill && pSkill->isLongSkill() == true)
    return false;

  // 仅区域型技能
  if (m_bJustPointSkill && pSkill->isPointSkill() == false)
    return false;

  // 仅吟唱技能
  if (m_bJustReadySkill)
  {
    if (pSkill->getLeadType() != ESKILLLEADTYPE_ONE && pSkill->getLeadType() != ESKILLLEADTYPE_TWO)
      return false;
    if (pSkill->getReadyTime(attacker, true) == 0)
      return false;
  }

  // 指定技能
  if (!m_setNeedSkillID.empty())
  {
    DWORD id = pSkill->getSkillID() / ONE_THOUSAND;
    return m_setNeedSkillID.find(id) != m_setNeedSkillID.end();
  }

  // 任意技能
  if (m_bAnySkillOk)
    return true;

  // 物理技能
  if (m_bPhySkillOk && pSkill->isPhySkill())
    return true;

  // 魔法技能
  if (m_bMagicSkillOk && pSkill->isMagicSkill())
    return true;

  // 普攻技能
  if (m_bNormalSkillOk && pSkill->isNormalSkill(attacker))
    return true;

  return false;
}

BeAttackCondition::BeAttackCondition()
{
  trigType = BUFFTRIGGER_BEATTACK;
}

BeAttackCondition::~BeAttackCondition()
{

}

bool BeAttackCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  m_bNormalSkillOk = data.getTableInt("normal_skill") == 1;
  m_bPhySkillOk = data.getTableInt("physic_skill") == 1;
  m_bMagicSkillOk = data.getTableInt("magic_skill") == 1;
  m_bAnySkillOk = data.getTableInt("all_skill") == 1;
  m_bJustLockSkill = data.getTableInt("just_lockSkill") == 1;
  m_bJustLongSkill = data.getTableInt("just_longSkill") == 1;
  m_bJustNearSkill = data.getTableInt("just_nearSkill") == 1;
  m_bNoNormalSkill = data.getTableInt("no_normalSkill") == 1;
  xLuaData& skilldata = data.getMutableData("skill");
  auto func = [&](const string& str, xLuaData& data)
  {
    m_vecSkillIDs.push_back(data.getInt());
  };
  skilldata.foreach(func);
  m_dwNeedAtkAttr = data.getTableInt("need_AtkAttr");
  m_dwLessDistance = data.getTableInt("less_distance");
  auto func2 = [&](const string& key, xLuaData& data)
  {
    m_setNotSkillIDs.insert(data.getInt());
  };
  data.getMutableData("not_skill").foreach(func2);

  m_fmHpMore.load(data.getMutableData("hp_more"));
  m_fmHpLess.load(data.getMutableData("hp_less"));

  if (data.has("need_race"))
  {
    auto getrace = [&](const string& k, xLuaData& d)
    {
      m_setNeedRaces.insert(d.getInt());
    };
    data.getMutableData("need_race").foreach(getrace);
  }

  m_bNeedMiss = data.getTableInt("miss") == 1;
  return true;
}

bool BeAttackCondition::isCheckOk(xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy, const BaseSkill* pSkill, DamageType damtype, const SBufferData* pData)
{
  if (attacker == nullptr || enemy == nullptr || pSkill == nullptr)
    return false;

  if (checkBase(enemy) == false)
    return false;

  // 正常情况, miss不生效
  if (!m_bNeedMiss && damtype == DAMAGE_TYPE_MISS)
    return false;

  // 除普攻技能
  if (m_bNoNormalSkill && pSkill->isNormalSkill(attacker))
    return false;

  // 仅锁定单体技能
  if (m_bJustLockSkill && pSkill->getLogicType() != ESKILLLOGIC_LOCKEDTARGET)
    return false;

  // 仅远程技能
  if (m_bJustLongSkill && pSkill->isLongSkill() == false)
    return false;

  // 仅近战技能
  if (m_bJustNearSkill && pSkill->isLongSkill() == true)
    return false;

  // 指定属性
  if (m_dwNeedAtkAttr != 0)
  {
    DWORD myAtkAttr = attacker->getTempAtkAttr();
    myAtkAttr = myAtkAttr == 0 ? 5 : myAtkAttr; // 默认无属性5
    if (myAtkAttr != m_dwNeedAtkAttr)
      return false;
  }

  // miss时生效
  if (m_bNeedMiss && damtype != DAMAGE_TYPE_MISS)
    return false;

  // 自身血量限制
  if (!m_fmHpMore.empty() || !m_fmHpLess.empty())
  {
    float hp = enemy->getAttr(EATTRTYPE_HP);
    float maxhp = enemy->getAttr(EATTRTYPE_MAXHP);
    float per = (maxhp != 0 ? hp/maxhp : 0);
    if (!m_fmHpMore.empty())
    {
      float hpmore = m_fmHpMore.getFmValue(enemy, pData);
      if (per < hpmore)
        return false;
    }
    if (!m_fmHpLess.empty())
    {
      float hpless = m_fmHpLess.getFmValue(enemy, pData);
      if (per > hpless)
        return false;
    }
  }

  // 种族限制
  if (!m_setNeedRaces.empty())
  {
    DWORD race = attacker->getRaceType();
    if (m_setNeedRaces.find(race) == m_setNeedRaces.end())
      return false;
  }

  // 指定距离之内
  if (m_dwLessDistance != 0 && checkDistance(attacker->getPos(), enemy->getPos(), m_dwLessDistance) == false)
    return false;

  // 指定技能不生效
  if (!m_setNotSkillIDs.empty() && m_setNotSkillIDs.find(pSkill->getSkillID() / ONE_THOUSAND) != m_setNotSkillIDs.end())
    return false;

  // 指定技能
  if (!m_vecSkillIDs.empty())
    return find(m_vecSkillIDs.begin(), m_vecSkillIDs.end(), pSkill->getSkillID() / ONE_THOUSAND) != m_vecSkillIDs.end();

  // 任意技能
  if (m_bAnySkillOk)
    return true;

  // 物理技能
  if (m_bPhySkillOk && pSkill->isPhySkill())
    return true;

  // 魔法技能
  if (m_bMagicSkillOk && pSkill->isMagicSkill())
    return true;

  // 普攻技能
  if (m_bNormalSkillOk && pSkill->isNormalSkill(attacker))
    return true;

  return false;
}

StatusCondition::StatusCondition()
{
}

StatusCondition::~StatusCondition()
{

}

bool StatusCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  m_dwStatus = data.getTableInt("Status");
  return true;
}

bool StatusCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  if (bData.me == nullptr)
    return false;
  return bData.me->m_oBuff.isInStatus(m_dwStatus);
}

HpLessPerCondition::HpLessPerCondition()
{
  trigType = BUFFTRIGGER_HPLESSPER;
}

HpLessPerCondition::~HpLessPerCondition()
{

}

bool HpLessPerCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  m_fHpLessPer = data.getTableFloat("value");
  if (m_fHpLessPer == 0)
    return false;
  if (data.has("invalid_value"))
    m_fHpMoreInvalidPer = data.getTableFloat("invalid_value");
  return true;
}

bool HpLessPerCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  if (bData.me == nullptr)
    return false;
  float maxhp = bData.me->getAttr(EATTRTYPE_MAXHP);
  float hp = bData.me->getAttr(EATTRTYPE_HP);

  // 已生效时, 若存在失效百分比配置, 优先判断
  if (m_fHpMoreInvalidPer > 0 && bData.activeFlag)
    return (DWORD)hp * 100 <  (DWORD)maxhp * m_fHpMoreInvalidPer;

  return hp / maxhp <= m_fHpLessPer / 100;
}

SpLessPerCondition::SpLessPerCondition()
{
  trigType = BUFFTRIGGER_SPLESSPER;
}

SpLessPerCondition::~SpLessPerCondition()
{

}

bool SpLessPerCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  m_fSpLessPer = data.getTableFloat("value");
  if (m_fSpLessPer == 0)
    return false;
  if (data.has("invalid_value"))
    m_fSpMoreInvalidPer = data.getTableFloat("invalid_value");
  return true;
}

bool SpLessPerCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  if (bData.me == nullptr)
    return false;
  float maxsp = bData.me->getAttr(EATTRTYPE_MAXSP);
  float sp = bData.me->getAttr(EATTRTYPE_SP);

  // 已生效时, 若存在失效百分比配置, 优先判断
  if (m_fSpMoreInvalidPer > 0 && bData.activeFlag)
    return (DWORD)sp * 100 <  (DWORD)maxsp * m_fSpMoreInvalidPer;

  return sp / maxsp <= m_fSpLessPer / 100;
}


// reborn condition
RebornCondition::RebornCondition()
{
  trigType = BUFFTRIGGER_REBORN;
}

RebornCondition::~RebornCondition()
{

}

// be shoot condition
BeShootCondition::BeShootCondition()
{
  trigType = BUFFTRIGGER_BESHOOT;
}

BeShootCondition::~BeShootCondition()
{

}

// 同一地图内组队人数
bool TeamNumCondition::init(xLuaData &data)
{
  if (BaseCondition::init(data) == false)
    return false;
  m_dwNum = data.getTableInt("value");
  return true;
}

bool TeamNumCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*>(entry);
  if (pUser == nullptr)
    return false;

  std::set<SceneUser*> teamSceneUsers = pUser->getTeamSceneUser();
  return teamSceneUsers.size() >= m_dwNum;
}

HandCondition::HandCondition()
{
  trigType = BUFFTRIGGER_HAND;
}

HandCondition::~HandCondition()
{

}

bool HandCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr)
    return false;
  return user->m_oHands.has() && user->m_oHands.isInWait() == false;
}

// 超出距离不生效
DistanceCondition::~DistanceCondition()
{
}

bool DistanceCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  // format: distance=x, teammate=1
  m_dwDistance = data.getTableInt("distance");
  m_bTeammate = data.getTableInt("teammate") == 1;
  return true;
}

bool DistanceCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic *src = xSceneEntryDynamic::getEntryByID(bData.fromID);
  if (src == nullptr || src->getScene() == nullptr || bData.me == nullptr || bData.me->getScene() == nullptr)
    return false;
  if (m_bTeammate && src->isMyTeamMember(bData.me->id) == false)
    return false;
  return src->getScene() == bData.me->getScene() && checkDistance(src->getPos(), bData.me->getPos(), m_dwDistance);
}

HasBuffCondition::HasBuffCondition()
{
  trigType = BUFFTRIGGER_BUFF;
}

HasBuffCondition::~HasBuffCondition()
{

}

bool HasBuffCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;
  auto getid = [&](const string& key, xLuaData& d)
  {
    m_setNeedBuffIDs.insert(d.getInt());
  };

  auto getNoNeedid = [&](const string& key, xLuaData& d)
  {
    m_setNotNeedBuffIDs.insert(d.getInt());
  };

  data.getMutableData("buffid").foreach(getid);
  data.getMutableData("nobuffid").foreach(getNoNeedid);
  
  return true;
}

bool HasBuffCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  if (BaseCondition::checkCondition(bData, cur) == false)
    return false;
  xSceneEntryDynamic* me = bData.me;
  if (me == nullptr)
    return false;
  
  for (auto &s : m_setNeedBuffIDs)
  {
    if (me->m_oBuff.haveBuff(s) == false)
      return false;
  }

  for(auto &s : m_setNotNeedBuffIDs)
  {
    if(me->m_oBuff.haveBuff(s)){
      return false;
    }
  }

  return true;
}

bool NpcPresentCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;
  m_dwBeingID = data.getTableInt("beingid");
  return true;
}

bool NpcPresentCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  if (BaseCondition::checkCondition(bData, cur) == false)
    return false;
  SceneUser* me = dynamic_cast<SceneUser*>(bData.me);
  if (me == nullptr)
    return false;
  return me->isBeingPresent(m_dwBeingID);
}

MapTypeCondition::MapTypeCondition()
{
  trigType = BUFFTRIGGER_MAPTYPE;
}

MapTypeCondition::~MapTypeCondition()
{

}

bool MapTypeCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;
  auto maptypef = [&](const string& k, xLuaData& d)
  {
    m_setMapType.insert(static_cast<EMapType>(d.getInt()));
  };
  data.getMutableData("typeID").foreach(maptypef);
  return true;
}

bool MapTypeCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  if (BaseCondition::checkCondition(bData, cur) == false)
    return false;
  if (m_setMapType.empty())
    return true;
  xSceneEntryDynamic* me = bData.me;
  if (me == nullptr || me->getScene() == nullptr)
    return false;
  if (me->getScene()->isGvg())
  {
    if (!hasMapType(EMAPTYPE_GVG))
      return false;
  }
  else if (me->getScene()->isPVPScene())
  {
    if (!hasMapType(EMAPTYPE_PVP))
      return false;
  }
  else if (!hasMapType(EMAPTYPE_PVE))
    return false;
  return true;
}

UseSkillKillCondition::UseSkillKillCondition()
{
  trigType = BUFFTRIGGER_USESKILLKILL;
}

UseSkillKillCondition::~UseSkillKillCondition()
{

}

bool UseSkillKillCondition::isCheckOk(const BaseSkill* pSkill, xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy)
{
  if (enemy == nullptr || enemy->isAlive())
    return false;

  return UseSkillCondition::isCheckOk(pSkill, attacker);
}

SuspendCondition::SuspendCondition()
{
  trigType = BUFFTRIGGER_SUSPEND;
}

SuspendCondition::~SuspendCondition()
{

}

bool SuspendCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (!entry)
    return false;
  return !entry->isSuspend();
}
bool GenderCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;
  DWORD gender = data.getTableInt("gender");
  if (gender <= EGENDER_MIN || gender >= EGENDER_MAX || EGender_IsValid(gender) == false)
    return false;
  m_eGender = static_cast<EGender>(gender);
  return true;
}

bool GenderCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  SceneUser* user = dynamic_cast<SceneUser*>(bData.me);
  if (user)
    return m_eGender == user->getUserSceneData().getGender();
  return true;
}

// 指定地图生效
MapBuffCondition::~MapBuffCondition()
{
}

bool MapBuffCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;

  auto funp = [&](const string& key, xLuaData& xdata)
  {
    m_setMapId.insert(xdata.getInt());
  };
  data.getMutableData("id").foreach(funp);

  return true;
}

bool MapBuffCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
  if (!pUser)
    return false;
  
  if (!pUser->getScene())
    return false;
  DWORD mapId = pUser->getScene()->getMapID();
  auto it = m_setMapId.find(mapId);
  if (it == m_setMapId.end())
    return false;
  return true;
}

AbnormalStateCondition::AbnormalStateCondition()
{
  trigType = BUFFTRIGGER_ABNORMAL;
}

AbnormalStateCondition::~AbnormalStateCondition()
{

}

bool AbnormalStateCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (!entry)
    return false;

  return entry->getAttr(EATTRTYPE_STATEEFFECT);
  
}

ElementElfCondition::~ElementElfCondition()
{
}

bool ElementElfCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
      return false;
  m_dwElementElfId = data.getTableInt("elementid");
  if (m_dwElementElfId == 0)
    return false;
  return true;
}

bool ElementElfCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
  if (!pUser)
    return false;
  if (pUser->getCurElementElfID() != m_dwElementElfId)
    return false;
  return true;
}

ChantStatusCondition::~ChantStatusCondition()
{
}

bool ChantStatusCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;
  m_bMoveChant = data.getTableInt("move_chant") == 1;
  return true;
}

bool ChantStatusCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
  if (!pUser)
    return false;

  if (pUser->getSkillStatus() == ESKILLSTATE_CHANT)
  {
    if (!m_bMoveChant || pUser->isCanMoveChant())
      return true;
    else
      return false;
  }
  else
    return false;

  return true;
}

bool ConcertCondition::init(xLuaData& data)
{
  if (BaseCondition::init(data) == false)
    return false;
  m_dwStyle = data.getTableInt("style");
  return true;
}

bool ConcertCondition::checkCondition(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* entry = bData.me;
  if (entry == nullptr)
    return false;

  switch (m_dwStyle)
  {
  case 1:
    return entry->m_oSkillStatus.inStatus(ESKILLSTATUS_SOLO);
  case 2:
    return entry->m_oSkillStatus.inStatus(ESKILLSTATUS_ENSEMBLE);
  case 3:
    return entry->m_oSkillStatus.inStatus(ESKILLSTATUS_SOLO) || entry->m_oSkillStatus.inStatus(ESKILLSTATUS_ENSEMBLE);
  default:
    return false;
  }
  return false;
}
