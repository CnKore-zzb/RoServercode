#include "Attribute.h"
#include "SceneNpc.h"
#include "SceneUser.h"
#include "UserConfig.h"
#include "RoleDataConfig.h"
#include "NpcConfig.h"
#include "SceneUserManager.h"
#include "MsgManager.h"
#include "RecordCmd.pb.h"
#include "GuildConfig.h"
#include "AttrFunc.h"

SetAttributeValidEnum AttributeValidEnum::s_setAttributeValidEnum;

void AttributeValidEnum::initAttributeValidEnum()
{
  s_setAttributeValidEnum.clear();
  for (size_t i = EATTRTYPE_MIN+1; i < EATTRTYPE_MAX; ++i)
  {
    if (EAttrType_IsValid(i) == false)
      continue;
    s_setAttributeValidEnum.insert(i);
  }
}

// base attribute
Attribute::Attribute(xSceneEntryDynamic* pEntry) : m_pEntry(pEntry)
{
  initAttr(m_vecFinalAttrs);
  initAttr(m_vecFinalPointAttrs);
  initAttr(m_vecSyncFinalAttrs);
  initAttr(m_vecSyncFinalPointAttrs);
  initAttr(m_vecBase);
  //initAttr(m_vecSkill);
  initAttr(m_vecEquip);
  //initAttr(m_vecCard);
  initAttr(m_vecDynamicBuff);
  initAttr(m_vecStaticBuff);
  initAttr(m_vecCharacter);
  initAttr(m_vecGM);
  initAttr(m_vecTimeBuff);
  initAttr(m_vecGuild);
  initAttr(m_vecAstrolabe);
  initAttr(m_vecAchieveTitle);
  initAttr(m_vecFood);
  initAttr(m_vecProfession);

  initAttr(m_vecLastStaticBuff);
  initAttr(m_vecLastDynamicBuff);
  initAttr(m_vecLastEquip);
  initAttr(m_vecLastGuild);
}

Attribute::~Attribute()
{

}

void Attribute::updateAttribute()
{
  if ((m_dwCollectType & ECOLLECTTYPE_BASE) != 0)
    collectBaseAttr();
  //if ((m_dwCollectType & ECOLLECTTYPE_SKILL) != 0)
  //  collectSkillAttr();
  if ((m_dwCollectType & ECOLLECTTYPE_EQUIP) != 0)
    collectEquipAttr();
  //if ((m_dwCollectType & ECOLLECTTYPE_CARD) != 0)
  //  collectCardAttr();
  if ((m_dwCollectType & ECOLLECTTYPE_CHARACTER) != 0)
    collectCharacterAttr();
  if ((m_dwCollectType & ECOLLECTTYPE_GUILD) != 0)
    collectGuildAttr();
  if ((m_dwCollectType & ECOLLECTTYPE_ASTROLABE) != 0)
    collectAstrolabe();
  if ((m_dwCollectType & ECOLLECTTYPE_ACHIEVEMENT) != 0)
    collectAchieveTitle();
  if ((m_dwCollectType & ECOLLECTTYPE_FOOD) != 0)
    collectFood();
  if ((m_dwCollectType & ECOLLECTTYPE_PROFESSION) != 0)
    collectProfession();

  if ((m_dwCollectType & ECOLLECTTYPE_STATIC_BUFF) != 0)
    collectBuffAttr(false);
  if ((m_dwCollectType & ECOLLECTTYPE_DYNAMIC_BUFF) != 0)
    collectBuffAttr(true);

  m_dwCollectType = 0;
}

float Attribute::getBaseAttr(EAttrType eType) const
{
  if (eType >= m_vecBase.size())
    return 0.0f;

  return m_vecBase[eType].value();
}

float Attribute::getOtherAttr(EAttrType eType) const
{
  if (eType >= m_vecFinalAttrs.size())
    return 0.0f;

  return /*m_vecSkill[eType].value() +*/ m_vecEquip[eType].value() + /*m_vecCard[eType].value() +*/ m_vecDynamicBuff[eType].value() + m_vecCharacter[eType].value() + m_vecGM[eType].value() +
    m_vecGuild[eType].value() + m_vecAstrolabe[eType].value() + m_vecAchieveTitle[eType].value() + m_vecFood[eType].value() + m_vecProfession[eType].value() + m_vecStaticBuff[eType].value();
}

float Attribute::getBuffAttr(EAttrType eType) const
{
  if (eType >= m_vecDynamicBuff.size() || eType >= m_vecStaticBuff.size())
    return 0.0f;

  return m_vecDynamicBuff[eType].value() + m_vecStaticBuff[eType].value();
}

float Attribute::getTimeBuffAttr(EAttrType eType) const
{
  if (eType >= m_vecTimeBuff.size())
    return 0.0f;
  return m_vecTimeBuff[eType].value();
}

float Attribute::getPointAttr(EAttrType eType) const
{
  if (eType >= m_vecFinalPointAttrs.size())
    return 0.0f;
  return m_vecFinalPointAttrs[eType].value();
}

float Attribute::getAttr(EAttrType eType) const
{
  if (eType >= m_vecFinalAttrs.size())
    return 0;

  return m_vecFinalAttrs[eType].value();
}

bool Attribute::setAttr(EAttrType eType, float value)
{
  if (eType >= m_vecFinalAttrs.size())
    return false;

  UserAttrSvr& rAttr = m_vecFinalAttrs[eType];
  if (value == rAttr.value())
    return false;
  rAttr.set_value(value);

  m_bitset.set(eType);
  return true;
}

bool Attribute::setPointAttr(EAttrType eType, float value)
{
  if (eType >= m_vecFinalPointAttrs.size())
    return false;

  if (value == m_vecFinalPointAttrs[eType].value())
    return false;
  m_vecFinalPointAttrs[eType].set_value(value);
  return true;
}

bool Attribute::setGMAttr(EAttrType eType, float value)
{
  if (eType >= m_vecGM.size())
    return false;

  m_vecGM[eType].set_value(value);
  m_bitset.set(eType);
  return true;
}

bool Attribute::setShowAttr(EAttrType eType, float value)
{
  auto v = find_if(m_vecShow.begin(), m_vecShow.end(), [eType](const UserAttrSvr& r) -> bool{
    return r.type() == eType;
  });
  if (v != m_vecShow.end())
  {
    v->set_value(value);
  }
  else
  {
    UserAttrSvr oAttr;
    oAttr.set_type(eType);
    oAttr.set_value(value);
    m_vecShow.push_back(oAttr);
  }

  return true;
}

void Attribute::setCollectMark(ECollectType eType)
{
  if (eType == ECOLLECTTYPE_ASTROLABE || eType == ECOLLECTTYPE_ACHIEVEMENT || eType == ECOLLECTTYPE_FOOD || eType == ECOLLECTTYPE_PROFESSION)
    setMark();

  m_dwCollectType |= eType;
  m_bNeedRefreshBuff = true;
}

/*bool Attribute::setGuildAttr(EAttrType eType, float value)
{
  if (eType >= m_vecGuild.size())
    return false;

  m_vecGuild[eType].set_value(m_vecGuild[eType].value() + value);
  //m_bitset.set(eType);
  return true;
}*/

/*bool Attribute::setEquipAttr(EAttrType eType, float value)
{
  if (eType >= m_vecEquip.size())
    return false;

  m_vecEquip[eType].set_value(m_vecEquip[eType].value() + value);
  //m_bitset.set(eType);
  return true;
}*/

void Attribute::setEquipMasterLv(DWORD lv)
{
  if (m_pEntry == nullptr)
    return;

  m_dwEquipMasterLv = lv;
  m_pEntry->setDataMark(EUSERDATATYPE_EQUIPMASTER);
}

void Attribute::setRefineMasterLv(DWORD lv)
{
  if (m_pEntry == nullptr)
    return;

  if (m_dwRefineMasterLv == lv)
    return;

  m_dwRefineMasterLv = lv;
  m_pEntry->setDataMark(EUSERDATATYPE_REFINEMASTER);
}

void Attribute::modifyCollect(ECollectType eCollect, const UserAttrSvr& rAttr, EAttrOper eOper /*= EATTROPER_ADD*/)
{
  TVecAttrSvrs* pVecAttr = nullptr;

  switch (eCollect)
  {
    case ECOLLECTTYPE_BASE:
      pVecAttr = &m_vecBase;
      break;
    //case ECOLLECTTYPE_SKILL:
    case ECOLLECTTYPE_EQUIP:
      pVecAttr = &m_vecEquip;
      break;
    // case ECOLLECTTYPE_CARD = 8,
    case ECOLLECTTYPE_STATIC_BUFF:
      pVecAttr = &m_vecStaticBuff;
      break;
    case ECOLLECTTYPE_DYNAMIC_BUFF:
      pVecAttr = &m_vecDynamicBuff;
      break;
    case ECOLLECTTYPE_CHARACTER:
      pVecAttr = &m_vecCharacter;
      break;
    case ECOLLECTTYPE_GUILD:
      pVecAttr = &m_vecGuild;
      break;
    case ECOLLECTTYPE_ASTROLABE:
      pVecAttr = &m_vecAstrolabe;
      break;
    case ECOLLECTTYPE_ACHIEVEMENT:
      pVecAttr = &m_vecAchieveTitle;
      break;
    case ECOLLECTTYPE_FOOD:
      pVecAttr = &m_vecFood;
      break;
    case ECOLLECTTYPE_PROFESSION:
      pVecAttr = &m_vecProfession;
      break;
    case ECOLLECTTYPE_NONE:
      return;
  }
  if (pVecAttr == nullptr)
    return;

  if (rAttr.type() >= pVecAttr->size())
    return;
  TVecAttrSvrs& vecAttr = *pVecAttr;
  UserAttrSvr& rDest = vecAttr[rAttr.type()];
  if (eOper == EATTROPER_ADD)
    rDest.set_value(rDest.value() + rAttr.value());
  else if (eOper == EATTROPER_SET)
    rDest.set_value(rAttr.value());
  else
    return;
  XDBG << "[属性-采集]" << m_pEntry->id << m_pEntry->name << "collect :" << eCollect << "采集属性" << static_cast<DWORD>(rAttr.type()) << rAttr.ShortDebugString() << XEND;
}

bool Attribute::test(EAttrType eType)
{
  return m_bitset.test(eType);
}

bool Attribute::initAttr(TVecAttrSvrs& vecAttrs)
{
  // resize size
  if (vecAttrs.empty() == true)
    vecAttrs.resize(EATTRTYPE_MAX);

  // init
  for (auto &d : AttributeValidEnum::get())
  {
    if (EAttrType_IsValid(d) == false)
      continue;

    vecAttrs[d].set_type(static_cast<EAttrType>(d));
    vecAttrs[d].set_value(0);
  }

  return true;
}

void Attribute::modifyAttr(TVecAttrSvrs& vecLast, const TVecAttrSvrs& vecCur)
{
  if (vecLast.size() < EATTRTYPE_MAX || vecCur.size() < EATTRTYPE_MAX)
    return;

  for (auto &d : AttributeValidEnum::get())
  {
    if (vecLast[d].value() != vecCur[d].value())
    {
      XDBG << "[属性采集] type :" << d << "lastvalue :" << vecLast[d].value() << "newvalue :" << vecCur[d].value() << XEND;
      setMark(static_cast<EAttrType>(d));
    }
  }
  vecLast = vecCur;
}

void Attribute::collectBaseAttr()
{
  initAttr(m_vecBase);
}

/*void Attribute::collectSkillAttr()
{
  initAttr(m_vecSkill);
}*/

void Attribute::collectEquipAttr()
{
  initAttr(m_vecEquip);
}

/*void Attribute::collectCardAttr()
{
  initAttr(m_vecCard);
}*/

void Attribute::collectBuffAttr(bool bDynamic)
{
  TVecAttrSvrs& vecBuffs = bDynamic ? m_vecDynamicBuff : m_vecStaticBuff;
  TVecAttrSvrs& lastVecBuffs = bDynamic ? m_vecLastDynamicBuff : m_vecLastStaticBuff;
  ECollectType eBuffCollectType = bDynamic ? ECOLLECTTYPE_DYNAMIC_BUFF : ECOLLECTTYPE_STATIC_BUFF;

  initAttr(vecBuffs);

  if (bDynamic)
    initAttr(m_vecTimeBuff);

  DWORD curAtkAttrPri = DWORD_MAX;
  DWORD curDefAttrPri = DWORD_MAX;

  // 部分属性不可增加或减少
  TMapAttrControl attrs;
  m_pEntry->m_oBuff.getControlledAttr(attrs);

  auto func = [this, &curAtkAttrPri, &curDefAttrPri, &attrs, &vecBuffs, &eBuffCollectType, &bDynamic](SBufferData& r)
  {
    if (r.activeFlag && r.pBuff)
    {
      EIgnoreAttrControl ignoreAttrControl = r.pBuff->getIgnoreAttrControl();

      const TVecAttrSvrs &vecAttrs = r.getAttr();
      for (auto v = vecAttrs.begin(); v != vecAttrs.end(); ++v)
      {
        if (!attrs.empty() && ignoreAttrControl != EIGNOREATTRCONTROL_ALL)
        {
          auto it = attrs.find(v->type());
          if (it != attrs.end())
          {
            if (ignoreAttrControl != EIGNOREATTRCONTROL_DEC && ((it->second & EATTRCONTROL_IGNORE_DEC) != 0 && v->value() < 0))
              continue;
            if (ignoreAttrControl != EIGNOREATTRCONTROL_INC && ((it->second & EATTRCONTROL_IGNORE_INC) != 0 && v->value() > 0))
              continue;
          }
        }

        switch(v->type())
        {
          case EATTRTYPE_ATKATTR:
          {
            if (r.pBuff->getAtkDefPri() < curAtkAttrPri)
            {
              modifyCollect(eBuffCollectType, *v, EATTROPER_SET);
              curAtkAttrPri = r.pBuff->getAtkDefPri();
            }
            continue;
          }
          break;
          case EATTRTYPE_DEFATTR:
          {
            if (r.pBuff->getAtkDefPri() < curDefAttrPri)
            {
              modifyCollect(eBuffCollectType, *v, EATTROPER_SET);
              curDefAttrPri = r.pBuff->getAtkDefPri();
            }
            continue;
          }
          break;
          // 位属性 : 策划配置 attr = {1, 2, 3, 4}, 用位来配置
          case EATTRTYPE_ATTREFFECT:
          case EATTRTYPE_STATEEFFECT:
          case EATTRTYPE_ATTRFUNCTION:
          case EATTRTYPE_ATTREFFECT2:
          case EATTRTYPE_FUNCLIMIT:
          {
            if ((DWORD)v->value() <= 0 || (DWORD)v->value() >= 31)
              continue;
            UserAttrSvr oAttr;
            oAttr.CopyFrom(*v);
            DWORD value = (DWORD)(vecBuffs[v->type()].value()) | 1 << (DWORD)(v->value() - 1);
            oAttr.set_value(value);
            modifyCollect(eBuffCollectType, oAttr, EATTROPER_SET);
            continue;
          }
          break;
          // 位属性 : 策划配置具体值, attr = 1 / 2 / 3
          case EATTRTYPE_NOEFFECTMOVE:
          {
            UserAttrSvr oAttr;
            oAttr.CopyFrom(*v);
            DWORD value = (DWORD)(vecBuffs[v->type()].value()) | (DWORD)(v->value());
            oAttr.set_value(value);
            modifyCollect(eBuffCollectType, oAttr, EATTROPER_SET);
            continue;
          }
          break;
          default:
          {
            modifyCollect(eBuffCollectType, *v);
            if (bDynamic && r.pBuff->isTimeBuff())
            {
              m_vecTimeBuff[v->type()].set_value(m_vecTimeBuff[v->type()].value() + v->value());
            }
          }
          break;
        }
      }
    }
  };

  if (bDynamic)
    m_pEntry->m_oBuff.foreachDynamicBuff(func);
  else
    m_pEntry->m_oBuff.foreachStaticBuff(func);

  // 转移属性
  TVecAttrSvrs transferattr;
  m_pEntry->getTransferAttr(transferattr);
  for (auto& v : transferattr)
    modifyCollect(eBuffCollectType, v);

  modifyAttr(lastVecBuffs, vecBuffs);
}

void Attribute::collectCharacterAttr()
{
  initAttr(m_vecCharacter);
}

void Attribute::collectGuildAttr()
{
  initAttr(m_vecGuild);
}

void Attribute::collectAstrolabe()
{
  initAttr(m_vecAstrolabe);
}

void Attribute::collectAchieveTitle()
{
  initAttr(m_vecAchieveTitle);
}

void Attribute::collectFood()
{
  initAttr(m_vecFood);
}

void Attribute::collectProfession()
{
  initAttr(m_vecProfession);
}

// npc attr
NpcAttribute::NpcAttribute(SceneNpc* pNpc) : Attribute(pNpc), m_pNpc(pNpc)
{

}

NpcAttribute::~NpcAttribute()
{

}

void NpcAttribute::updateAttribute()
{
  if (m_pNpc == nullptr)
    return;

  Attribute::updateAttribute();

  // config attr
  for (auto v = m_vecFinalAttrs.begin(); v != m_vecFinalAttrs.end(); ++v)
  {
    if (v->type() == EATTRTYPE_HP || v->type() == EATTRTYPE_SP)
      continue;

    if (v->type() == EATTRTYPE_ATKATTR || v->type() == EATTRTYPE_DEFATTR)
      continue;
    setAttr(v->type(), m_vecBase[v->type()].value() + getOtherAttr(v->type()));
  }

  float value = 0;
  EAttrType eType = EATTRTYPE_MIN;

  // move speed
  value = getAttr(EATTRTYPE_MOVESPD) * (1 + getAttr(EATTRTYPE_MOVESPDPER));
  eType = EATTRTYPE_MOVESPD;
  setAttr(eType, value);

  // dynamic attr
  LuaManager::getMe().call<void>("calcNpcAttrValue", m_pNpc);

  //noskill = noskill +staunt + ...
  value = getBuffAttr(EATTRTYPE_NOSKILL) + getBuffAttr(EATTRTYPE_TAUNT);
  setAttr(EATTRTYPE_NOSKILL, value);

  //noattack = noattack + fearrun
  value = getBuffAttr(EATTRTYPE_NOATTACK) + getBuffAttr(EATTRTYPE_FEARRUN);
  setAttr(EATTRTYPE_NOATTACK, value);

  // atkattr
  eType = EATTRTYPE_ATKATTR;
  value = getBuffAttr(eType);
  value = (value == 0 || value > 10) ? 5 : value;
  setAttr(EATTRTYPE_ATKATTR, value);

  // defattr
  eType = EATTRTYPE_DEFATTR;
  value = getBuffAttr(eType);
  DWORD configValue = static_cast<DWORD> (m_pNpc->getNatureType());
  configValue = configValue == 0 ? 5 : configValue;
  value = value == 0 ? configValue : value;
  value = (value == 0 || value > 10) ? 5 : value;
  setAttr(eType, value);

  if (m_pNpc->isAlive() == false)
  {
    setAttr(EATTRTYPE_HIDE, 0);
  }

  if (m_pNpc->m_oBuff.haveLimitAttr())
  {
    map<EAttrType, float> maxlimit;
    map<EAttrType, float> minlimit;
    m_pNpc->m_oBuff.getLimitAttrs(minlimit, maxlimit);

    for (auto &m : maxlimit)
    {
      if (getAttr(m.first) > m.second)
        setAttr(m.first, m.second);
    }
    for (auto &m : minlimit)
    {
      if (getAttr(m.first) < m.second)
        setAttr(m.first, m.second);
    }
  }

  // movespd for follower
  if (m_pNpc->define.m_oVar.m_qwFollowerID)
  {
    SceneUser* pMaster = SceneUserManager::getMe().getUserByID(m_pNpc->define.m_oVar.m_qwFollowerID);
    if (pMaster)
    {
      setAttr(EATTRTYPE_MOVESPD, pMaster->m_oFollower.getMoveSpeed(m_pNpc->id));
    }
  }
}

void NpcAttribute::collectSyncAttrCmd(NpcDataSync& rCmd)
{
  bool isMonster = m_pNpc->isMonster();
  // collect hp and sp
  if (m_bitset.test(EATTRTYPE_HP) == true && isMonster)
  {
    add_attr(rCmd.add_attrs(), EATTRTYPE_HP, getAttr(EATTRTYPE_HP));
    m_bitset.flip(EATTRTYPE_HP);
  }
  if (m_bitset.test(EATTRTYPE_SP) == true && isMonster)
  {
    add_attr(rCmd.add_attrs(), EATTRTYPE_SP, getAttr(EATTRTYPE_SP));
    m_bitset.flip(EATTRTYPE_SP);
  }

  if (m_bitset.any() == false && m_dwCollectType == 0)
    return;

  updateAttribute();

  if (!isMonster)
  {
    DWORD d = EATTRTYPE_MOVESPD;
    if (m_bitset.test(d))
      add_attr(rCmd.add_attrs(), m_vecFinalAttrs[d].type(), RoleDataConfig::getMe().needSyncPercent(d) == true ? m_vecFinalAttrs[d].value() * FLOAT_TO_DWORD : m_vecFinalAttrs[d].value());
  }
  else
  {
    for (auto &d : AttributeValidEnum::get())
    {
      if (m_bitset.test(d) == false)
        continue;

      add_attr(rCmd.add_attrs(), m_vecFinalAttrs[d].type(), RoleDataConfig::getMe().needSyncPercent(d) == true ? m_vecFinalAttrs[d].value() * FLOAT_TO_DWORD : m_vecFinalAttrs[d].value());
    }
  }

  m_bitset.reset();
}

void NpcAttribute::toData(MapNpc* cmd)
{
  if (cmd == nullptr)
    return;

  if (!m_pNpc->isMonster())
  {
    if (getAttr(EATTRTYPE_MOVESPD) != 0)
    {
      DWORD d = EATTRTYPE_MOVESPD;
      add_attr(cmd->add_attrs(), m_vecFinalAttrs[d].type(), RoleDataConfig::getMe().needSyncPercent(d) == true ? m_vecFinalAttrs[d].value() * FLOAT_TO_DWORD : m_vecFinalAttrs[d].value());
    }
  }
  else
  {
    for (auto &d : AttributeValidEnum::get())
    {
      if (getAttr(static_cast<EAttrType>(d)) == 0.0f)
        continue;

      add_attr(cmd->add_attrs(), m_vecFinalAttrs[d].type(), RoleDataConfig::getMe().needSyncPercent(d) == true ? m_vecFinalAttrs[d].value() * FLOAT_TO_DWORD : m_vecFinalAttrs[d].value());
    }
  }
}

void NpcAttribute::collectBaseAttr()
{
  Attribute::collectBaseAttr();

  if (m_pNpc == nullptr)
    return;
  const SNpcCFG* pBase = m_pNpc->getCFG();
  if (pBase == nullptr)
    return;

  for (auto v = pBase->vecAttrs.begin(); v != pBase->vecAttrs.end(); ++v)
    m_vecBase[v->type()].set_value(v->value());
}

void NpcAttribute::collectCharacterAttr()
{
  Attribute::collectCharacterAttr();
  MonsterNpc* pNpc = dynamic_cast<MonsterNpc*>(m_pNpc);
  if (pNpc == nullptr)
    return;

  const TVecDWORD& vecIDs = pNpc->getNpcCharacter();
  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    const TVecAttrSvrs* pAttrs = NpcConfig::getMe().getCharacAttrs(*v);
    if (pAttrs == nullptr)
      return;
    for (auto s = pAttrs->begin(); s != pAttrs->end(); ++s)
      m_vecCharacter[s->type()].set_value(s->value());
  }
}

// user attr
UserAttribute::UserAttribute(SceneUser* pUser) : Attribute(pUser), m_pUser(pUser)
{

}

UserAttribute::~UserAttribute()
{

}

void UserAttribute::updateAttribute()
{
  // get user
  if (m_pUser == nullptr || m_pUser->getFighter() == nullptr)
    return;
  xTime frameDebug;

  Attribute::updateAttribute();

  // transform monster
  if (m_pUser->getTransform().isMonster())
  {
    updateMonsterAttr();
    return;
  }

  // attr value
  EAttrType eType = EATTRTYPE_MIN;
  float value = 0.0f;
  float pointvalue = 0.0f;

  // 六维属性 = 加点+职业加成+装备+卡片+技能+BUFF
  bool bChange = false;
  for (int i = EATTRTYPE_STR; i <= EATTRTYPE_LUK; ++i)
  {
    EAttrType eType = static_cast<EAttrType>(i);
    value = getBaseAttr(eType) + getOtherAttr(eType);
    pointvalue = getBaseAttr(eType);
    setAttr(eType, value);
    setPointAttr(eType, pointvalue);
    bChange |= m_bitset.test(static_cast<EAttrType>(i));
  }

  // 六维属性影响buff中属性计算 ex. BuffATK = DEX * 2 + 2; 六维属性计算结束，重新收集buff Attr
  if (bChange)
    collectBuffAttr(true);

  // calc flee and def affected by locking me monsters' num
  const SLockEffectCFG& lockCFG = MiscConfig::getMe().getBeLockCFG();
  DWORD numlockme = m_pUser->getValidNumLockMe();
  bool bLockRefresh = m_dwLockMeNum != numlockme;
  if (bLockRefresh)
  {
    m_dwLockMeNum = numlockme;
    setMark(EATTRTYPE_DEF);
    setMark(EATTRTYPE_FLEE);
  }

  // calc final result
  //LuaManager::getMe().call<void>("calcUserAttrValue", static_cast<xSceneEntryDynamic*>(m_pUser), m_pUser->getLevel(), m_pUser->getUserSceneData().getProfession());
  if (CommonConfig::m_bLua)
    LuaManager::getMe().call<void>("calcUserAttr", static_cast<xSceneEntryDynamic*>(m_pUser), m_dwAttrIndex);
  else
    AttrFunc::getMe().calcUserAttr(m_pUser, m_dwAttrIndex);

  if (bLockRefresh)
  {
    float fleedecper = lockCFG.getFleeDecPer(numlockme);
    if (fleedecper > 0)
    {
      value = getAttr(EATTRTYPE_FLEE) * (1 - fleedecper);
      setAttr(EATTRTYPE_FLEE, value);
    }

    float defdecper = lockCFG.getDefDecPer(numlockme);
    if (defdecper > 0)
    {
      value = getAttr(EATTRTYPE_DEF) * (1 - defdecper);
      setAttr(EATTRTYPE_DEF, value);
    }
  }

  // set hp
  DWORD maxhp = getAttr(EATTRTYPE_MAXHP);
  DWORD hp = m_pUser->getFighter()->getHp();
  hp = hp > maxhp ? maxhp : hp;
  setAttr(EATTRTYPE_HP, hp);
  // set sp
  DWORD maxsp = getAttr(EATTRTYPE_MAXSP);
  DWORD sp = m_pUser->getFighter()->getSp();
  sp = sp > maxsp ? maxsp : sp;
  setAttr(EATTRTYPE_SP, sp);
  
  // set save hp
  DWORD saveHp = m_pUser->getSceneFood().getSaveHP();
  setAttr(EATTRTYPE_SAVE_HP, saveHp);

  // set save sp
  DWORD saveSp = m_pUser->getSceneFood().getSaveSP();
  setAttr(EATTRTYPE_SAVE_SP, saveSp);

  // atkattr
  eType = EATTRTYPE_ATKATTR;
  value = getBuffAttr(eType);
  // 默认无属性
  value = (value == 0 || value > 10) ? 5 : value;
  setAttr(eType, value);

  // defattr
  eType = EATTRTYPE_DEFATTR;
  value = getBuffAttr(eType);
  // 默认无属性
  value = (value == 0 || value > 10) ? 5 : value;
  setAttr(eType, value);

  // from buff only
  static const vector<EAttrType> vecTypeBuff{
    EATTRTYPE_NOSTIFF, EATTRTYPE_NOATTACK, EATTRTYPE_HIDE, EATTRTYPE_TAUNT, EATTRTYPE_NOSKILL, EATTRTYPE_NOMAGICSKILL,
    EATTRTYPE_DEADSOON, EATTRTYPE_FREEZE, EATTRTYPE_NOEFFECTMOVE, EATTRTYPE_FEARRUN,
    EATTRTYPE_NOMOVE, EATTRTYPE_NOACT, EATTRTYPE_STATEEFFECT, EATTRTYPE_ATTREFFECT,
    EATTRTYPE_NOATTACKED, EATTRTYPE_TRANSFORMID, EATTRTYPE_ATTREFFECT2, EATTRTYPE_FUNCLIMIT,
    EATTRTYPE_MOVECHANT,
  };
  for (auto &v : vecTypeBuff)
  {
    value = getBuffAttr(v);
    setAttr(v, value);
  }

  // from buff, skill, equip, etc.
  static const vector<EAttrType> vecTypeOther{
    EATTRTYPE_IGNOREDEF, EATTRTYPE_IGNOREMDEF, /*EATTRTYPE_DAMREDUC,EATTRTYPE_MDAMREDUC,*/
    EATTRTYPE_DAMINCREASE, EATTRTYPE_MDAMINCREASE, EATTRTYPE_HEALENCPER, EATTRTYPE_BEHEALENCPER,
    EATTRTYPE_WINDDAMPER, EATTRTYPE_BEWINDDAMPER, EATTRTYPE_EARTHDAMPER, EATTRTYPE_BEEARTHDAMPER,
    EATTRTYPE_WATERDAMPER, EATTRTYPE_BEWATERDAMPER, EATTRTYPE_FIREDAMPER, EATTRTYPE_BEFIREDAMPER,
    EATTRTYPE_NEUTRALDAMPER, EATTRTYPE_BENEUTRALDAMPER, EATTRTYPE_HOLYDAMPER, EATTRTYPE_BEHOLYDAMPER,
    EATTRTYPE_SHADOWDAMPER, EATTRTYPE_BESHADOWDAMPER, EATTRTYPE_GHOSTDAMPER, EATTRTYPE_BEGHOSTDAMPER,
    EATTRTYPE_UNDEADDAMPER, EATTRTYPE_BEUNDEADDAMPER, EATTRTYPE_POSIONDAMPER, EATTRTYPE_BEPOSIONDAMPER,
    EATTRTYPE_BRUTEDAMPER, EATTRTYPE_BRUTERESPER, EATTRTYPE_DEMIHUMANDAMPER, EATTRTYPE_DEMIHUMANRESPER,
    EATTRTYPE_DEMONDAMPER, EATTRTYPE_DEMONRESPER, EATTRTYPE_PLANTDAMPER, EATTRTYPE_PLANTRESPER,
    EATTRTYPE_DEADLESSDAMPER, EATTRTYPE_DEADLESSRESPER, EATTRTYPE_FORMLESSDAMPER, EATTRTYPE_FORMLESSRESPER,
    EATTRTYPE_FISHDAMPER, EATTRTYPE_FISHRESPER, EATTRTYPE_ANGLEDAMPER, EATTRTYPE_ANGLERESPER,
    EATTRTYPE_INSECTDAMPER, EATTRTYPE_INSECTRESPER, EATTRTYPE_DRAGONDAMPER, EATTRTYPE_DRAGONRESPER,
    EATTRTYPE_SMALLDAMPER, EATTRTYPE_SMALLRESPER, EATTRTYPE_MIDDAMPER, EATTRTYPE_MIDRESPER,
    EATTRTYPE_BIGDAMPER, EATTRTYPE_BIGRESPER, EATTRTYPE_BOSSDAMPER, EATTRTYPE_BOSSRESPER,
    EATTRTYPE_VAMPIRIC, EATTRTYPE_BEVAMPIRIC, EATTRTYPE_CTCHANGE, EATTRTYPE_CTCHANGEPER,
    EATTRTYPE_CDCHANGE, EATTRTYPE_CDCHANGEPER, EATTRTYPE_SPCOST, EATTRTYPE_SPCOSTPER,
    EATTRTYPE_ATKDISTANCE, EATTRTYPE_ATKDISTANCEPER, EATTRTYPE_ITEMRESTORESPD, EATTRTYPE_ITEMSPRESTORESPD,
    EATTRTYPE_HARMIMMUNE, EATTRTYPE_RESTORESPDPER, EATTRTYPE_SPRESTORESPDPER, EATTRTYPE_REALDAMAGE,
    EATTRTYPE_DAMREBOUND, EATTRTYPE_MDAMREBOUND, EATTRTYPE_ATTRFUNCTION, EATTRTYPE_EQUIPASPD,
    EATTRTYPE_SKILLASPD, EATTRTYPE_HITPER, EATTRTYPE_FLEEPER, EATTRTYPE_STRPER,
    EATTRTYPE_INTPER, EATTRTYPE_AGIPER, EATTRTYPE_DEXPER, EATTRTYPE_VITPER,
    EATTRTYPE_LUKPER, EATTRTYPE_RANGEDAM, EATTRTYPE_SHOTDAMREDUC, EATTRTYPE_LONGDAMREDUC,
    EATTRTYPE_LONGMDAMREDUC, EATTRTYPE_IGNOREEQUIPDEF, EATTRTYPE_CTFIXED, EATTRTYPE_CTFIXEDPER,
    EATTRTYPE_SELLDISCOUNT, EATTRTYPE_BUYDISCOUNT, EATTRTYPE_WINDATK, EATTRTYPE_EARTHATK,
    EATTRTYPE_FIREATK, EATTRTYPE_WATERATK, EATTRTYPE_NEUTRALATK, EATTRTYPE_HOLYATK,
    EATTRTYPE_DARKATK, EATTRTYPE_WINDDEF, EATTRTYPE_EARTHDEF, EATTRTYPE_FIREDEF,
    EATTRTYPE_WATERDEF, EATTRTYPE_NEUTRALDEF, EATTRTYPE_HOLYDEF, EATTRTYPE_DARKDEF,
    EATTRTYPE_SHAPEATKPER, EATTRTYPE_SILENCEATK, EATTRTYPE_SILENCEDEF,
    EATTRTYPE_FREEZEATK, EATTRTYPE_FREEZEDEF, EATTRTYPE_STONEATK, EATTRTYPE_STONEDEF,
    EATTRTYPE_STUNATK, EATTRTYPE_STUNDEF, EATTRTYPE_BLINDATK, EATTRTYPE_BLINDDEF,
    EATTRTYPE_POSIONATK, EATTRTYPE_POSIONDEF, EATTRTYPE_SLOWATK, EATTRTYPE_SLOWDEF,
    EATTRTYPE_CHAOSATK, EATTRTYPE_CHAOSDEF, EATTRTYPE_CURSEATK, EATTRTYPE_CURSEDEF,
    EATTRTYPE_DELAYCDCHANGE, EATTRTYPE_DELAYCDCHANGEPER, EATTRTYPE_DCHANGE, EATTRTYPE_DCHANGEPER,
    EATTRTYPE_SLIM_HEIGHT, EATTRTYPE_SLIM_WEIGHT,
    EATTRTYPE_DAMSPIKE, EATTRTYPE_MDAMSPIKE, EATTRTYPE_MONSTERDAMPER, EATTRTYPE_MONSTERRESPER,
    EATTRTYPE_NPCDAMPER, EATTRTYPE_NPCRESPER,
    EATTRTYPE_BASEEXPPER, EATTRTYPE_JOBEXPPER,
    EATTRTYPE_REFINEDAMREDUC, EATTRTYPE_REFINEMDAMREDUC, EATTRTYPE_ENERGYDAMREDUC, EATTRTYPE_STEELDAMREDUC,
    EATTRTYPE_STEELMDAMREDUC, EATTRTYPE_PROTECTDAMREDUC, EATTRTYPE_PROTECTMDAMREDUC,
    EATTRTYPE_HIDEDAMREDUC, EATTRTYPE_HIDEMDAMREDUC,
    EATTRTYPE_DRAGONDAMREDUC, EATTRTYPE_DRAGONMDAMREDUC, EATTRTYPE_NORMALREALDAM, EATTRTYPE_NORMALMREALDAM,
    EATTRTYPE_SKILLREALDAM, EATTRTYPE_SKILLMREALDAM,
    EATTRTYPE_NORMALATK, EATTRTYPE_NORMALATKDAM, EATTRTYPE_NORMALATKRES,
    EATTRTYPE_SKILLDAM, EATTRTYPE_SKILLRES,
    EATTRTYPE_WINDMDAMREDUC, EATTRTYPE_DEADDAMREDUC, EATTRTYPE_DEADMDAMREDUC,
    EATTRTYPE_GHOSTATK,EATTRTYPE_UNDEADATK,EATTRTYPE_POISONINGATK,
    EATTRTYPE_SLEEPATK, EATTRTYPE_SLEEPDEF, EATTRTYPE_SOLO, EATTRTYPE_ENSEMBLE,
  };
  for (auto &v : vecTypeOther)
  {
    value = getOtherAttr(v);
    setAttr(v, value);
  }

  if (m_pUser->m_oBuff.haveLimitAttr())
  {
    map<EAttrType, float> maxlimit;
    map<EAttrType, float> minlimit;
    m_pUser->m_oBuff.getLimitAttrs(minlimit, maxlimit);

    for (auto &m : maxlimit)
    {
      if (getAttr(m.first) > m.second)
        setAttr(m.first, m.second);
    }
    for (auto &m : minlimit)
    {
      if (getAttr(m.first) < m.second)
        setAttr(m.first, m.second);
    }
  }

  if (m_pUser->m_oBuff.haveForceAttr())
  {
    TVecAttrSvrs forceAttrs;
    m_pUser->m_oBuff.getForceAttr(forceAttrs);
    for (auto &v : forceAttrs)
    {
      setAttr(v.type(), v.value());
    }
  }
  // battlepoint
  SceneFighter* pCurFighter = m_pUser->getFighter();
  if (pCurFighter != nullptr)
    pCurFighter->getBattlePoint();
  XDBG << "[玩家-属性计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "本次使用" << (CommonConfig::m_bLua ? "lua" : "C++") << "第" << m_dwAttrIndex++ << "计算耗时" << frameDebug.uElapse() << "微秒" << XEND;
}

void UserAttribute::collectSyncAttrCmd(UserSyncCmd& rCmd, UserDataSync& session, UserNineSyncCmd& nine)
{
  // add sync attr
  auto addsync = [&](EAttrType eType, float value, float extra)
  {
    UserAttr* pAttr = rCmd.add_attrs();
    UserAttr* pPointAttr = rCmd.add_pointattrs();
    if (pAttr == nullptr || pPointAttr == nullptr)
      return;

    int dvalue = floor(value);
    float fvalue = value - dvalue;

    pAttr->set_type(eType);
    pAttr->set_value(floor(fvalue * 1000 + 0.5) / 1000 + dvalue);

    dvalue = floor(extra);
    fvalue = extra - dvalue;

    pPointAttr->set_type(eType);
    pPointAttr->set_value(floor(fvalue * 1000 + 0.5) / 1000 + dvalue);
  };

  // add sesssion attr
  auto addsession = [&](EAttrType type, float value) {
    vector<EAttrType> vecSessionAttr{EATTRTYPE_HP, EATTRTYPE_MAXHP, EATTRTYPE_SP, EATTRTYPE_MAXSP};
    auto v = find(vecSessionAttr.begin(), vecSessionAttr.end(), type);
    if (v == vecSessionAttr.end())
      return;

    UserAttr* attr = session.add_attrs();
    if (attr == nullptr)
      return;

    attr->set_type(type);
    attr->set_value(static_cast<SDWORD>(value));
  };

  // add nine attr
  auto addnine = [&](EAttrType type, float value) {
    UserAttr* attr = nine.add_attrs();
    if (attr == nullptr)
      return;

    attr->set_type(type);
    attr->set_value(static_cast<SDWORD>(value));
   /* if (type ==  EATTRTYPE_SLIM_HEIGHT || type == EATTRTYPE_SLIM_WEIGHT || type == EATTRTYPE_SAVE_HP || type == EATTRTYPE_SAVE_SP)
      XLOG << "料理-属性同步九屏属性"  << type << value << XEND;*/
  };

  // collect hp and sp
  if (m_bitset.test(EATTRTYPE_HP) == true)
  {
    if (needSync(EATTRTYPE_HP) == true)
    {
      addsync(EATTRTYPE_HP, getAttr(EATTRTYPE_HP), 0.0f);
      addsession(EATTRTYPE_HP, getAttr(EATTRTYPE_HP));
      addnine(EATTRTYPE_HP, getAttr(EATTRTYPE_HP));
    }
    m_bitset.flip(EATTRTYPE_HP);
  }
  if (m_bitset.test(EATTRTYPE_SP) == true)
  {
    if (needSync(EATTRTYPE_SP) == true)
    {
      addsync(EATTRTYPE_SP, getAttr(EATTRTYPE_SP), 0.0f);
      addsession(EATTRTYPE_SP, getAttr(EATTRTYPE_SP));
      addnine(EATTRTYPE_SP, getAttr(EATTRTYPE_SP));
    }
    m_bitset.flip(EATTRTYPE_SP);
  }
  if (m_bitset.test(EATTRTYPE_TRANSFORMID) == true)
  {
    if (needSync(EATTRTYPE_TRANSFORMID) == true)
    {
      addsync(EATTRTYPE_TRANSFORMID, getAttr(EATTRTYPE_TRANSFORMID), 0.0f);
      addnine(EATTRTYPE_TRANSFORMID, getAttr(EATTRTYPE_TRANSFORMID));
    }
    m_bitset.flip(EATTRTYPE_TRANSFORMID);
  }

  // check dirty data exist
  if (m_bitset.any() == false && m_dwCollectType == 0)
    return;

  updateAttribute();

  // collect sync data
  EAttrType eType = EATTRTYPE_MIN;
  float value = 0.0f;
  float extra = 0.0f;
  for (auto &d : AttributeValidEnum::get())
  {
    eType = static_cast<EAttrType>(d);
    if (m_bitset.test(d) == false || needSync(eType) == false)
      continue;

    if (RoleDataConfig::getMe().needSyncPercent(d))
    {
      value = getAttr(eType) * FLOAT_TO_DWORD;
      extra = getPointAttr(eType) * FLOAT_TO_DWORD;
    }
    else
    {
      value = getAttr(eType);
      extra = getPointAttr(eType);
    }

    addsync(eType, value, extra);
    addsession(eType, value);
    addnine(eType, value);
  }

  m_bitset.reset();

  // attr change
  m_pUser->getEvent().onAttrChange();
}

void UserAttribute::collectBaseAttr()
{
  // get user
  if (m_pUser == nullptr)
    return;

  if (m_pUser->getTransform().isMonster())
  {
    Attribute::collectBaseAttr();
    collectMonsterAttr();
    return;
  }

  // collect old six attr
  map<EAttrType, float> mapOldAttr;
  for (int i = EATTRTYPE_STR; i <= EATTRTYPE_LUK; ++i)
    mapOldAttr[static_cast<EAttrType>(i)] = m_vecBase[static_cast<EAttrType>(i)].value();

  initAttr(m_vecBase);

  // get cur fighter
  SceneFighter* pCurFighter = m_pUser->getFighter();
  if (pCurFighter == nullptr)
    return;

  // get attr config
  const SRoleBaseCFG* pCFG = m_pUser->getRoleBaseCFG();
  if (pCFG == nullptr)
    return;

  // attr value
  float value = 0.0f;
  map<EAttrType, float> mapDatas;

  // 六维属性 = 加点+职业加成+技能+装备+卡片+BUFF floor((joblv-10)/160*(属性系数-0.5)+0.5,1)+10
  EAttrType eType = EATTRTYPE_STR;
  value = LuaManager::getMe().call<float>("calcattr", pCurFighter->getAttrPoint(eType), pCurFighter->getJobLv(), pCFG->dwTypeBranch, "Str");
  m_vecBase[eType].set_value(value);
  float oldvalue = mapOldAttr[eType];
  if (oldvalue != 0.0f && value > oldvalue)
    mapDatas[eType] = value - oldvalue;

  eType = EATTRTYPE_INT;
  value = LuaManager::getMe().call<float>("calcattr", pCurFighter->getAttrPoint(eType), pCurFighter->getJobLv(), pCFG->dwTypeBranch, "Int");
  m_vecBase[eType].set_value(value);
  oldvalue = mapOldAttr[eType];
  if (oldvalue != 0.0f && value > oldvalue)
    mapDatas[eType] = value - oldvalue;

  eType = EATTRTYPE_AGI;
  value = LuaManager::getMe().call<float>("calcattr", pCurFighter->getAttrPoint(eType), pCurFighter->getJobLv(), pCFG->dwTypeBranch, "Agi");
  m_vecBase[eType].set_value(value);
  oldvalue = mapOldAttr[eType];
  if (oldvalue != 0.0f && value > oldvalue)
    mapDatas[eType] = value - oldvalue;

  eType = EATTRTYPE_DEX;
  value = LuaManager::getMe().call<float>("calcattr", pCurFighter->getAttrPoint(eType), pCurFighter->getJobLv(), pCFG->dwTypeBranch, "Dex");
  m_vecBase[eType].set_value(value);
  oldvalue = mapOldAttr[eType];
  if (oldvalue != 0.0f && value > oldvalue)
    mapDatas[eType] = value - oldvalue;

  eType = EATTRTYPE_VIT;
  value = LuaManager::getMe().call<float>("calcattr", pCurFighter->getAttrPoint(eType), pCurFighter->getJobLv(), pCFG->dwTypeBranch, "Vit");
  m_vecBase[eType].set_value(value);
  oldvalue = mapOldAttr[eType];
  if (oldvalue != 0.0f && value > oldvalue)
    mapDatas[eType] = value - oldvalue;

  eType = EATTRTYPE_LUK;
  value = LuaManager::getMe().call<float>("calcattr", pCurFighter->getAttrPoint(eType), pCurFighter->getJobLv(), pCFG->dwTypeBranch, "Luk");
  m_vecBase[eType].set_value(value);
  oldvalue = mapOldAttr[eType];
  if (oldvalue != 0.0f && value > oldvalue)
    mapDatas[eType] = value - oldvalue;

  // msg hint
  for (auto m = mapDatas.begin(); m != mapDatas.end(); ++m)
  {
    const RoleData* pData = RoleDataConfig::getMe().getRoleData(m->first);
    if (pData != nullptr)
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_SHOWATTR, MsgParams(pData, m->second));
  }
}

void UserAttribute::collectEquipAttr()
{
  initAttr(m_vecEquip);

  if (m_pUser->getTransform().isMonster())
    return;

  // get equip package
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  FashionPackage* pFashionPack = dynamic_cast<FashionPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHION));
  if (pEquipPack == nullptr || pFashionPack == nullptr)
    return;

  // collect equip attr
  DWORD strengthMasterlv = 999;
  DWORD refineMasterlv = 999;
  bitset<EEQUIPTYPE_MAX> equipset;
  //map<DWORD, DWORD> mapSuitID2Count;
  TVecDWORD vecSkillIDs;
  auto func = [&](ItemBase* pBase)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip == nullptr || pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == false)
      return;
    //if (ItemManager::getMe().isFashion(pEquip->getEquipType()) == bExcludeFashion)
      //return;

    //const SItemCFG* pCFG = pEquip->getCFG();
    //if (pCFG == nullptr)
      //return;

    if (ItemManager::getMe().isFashion(pEquip->getEquipType()) == false)
    {
      // master
      if (pEquip->getStrengthLv() < strengthMasterlv)
        strengthMasterlv = pEquip->getStrengthLv();
      if (pEquip->getRefineLv() < refineMasterlv)
        refineMasterlv = pEquip->getRefineLv();

      // suit
      equipset.set(pEquip->getEquipType());
      //mapSuitID2Count[ItemManager::getMe().getSuitID(pEquip->getTypeID())]++;
    }

    // attrs
    //pEquip->collectEquipAttr(m_pUser, m_vecEquip);
    pEquip->collectEquipAttr(m_pUser);

    /*
    // enchant - extra
    EnchantData& rData = (const_cast<ItemEquip*>(pEquip))->getEnchantData();
    const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(rData.type());
    if (pCFG != nullptr)
    {
      for (int i = 0; i < rData.extras_size(); ++i)
      {
        const SEnchantAttr* pAttr = pCFG->getEnchantAttr(rData.extras(i).configid());
        if (pAttr == nullptr || pAttr->vecExtraCondition.size() < 2)
          continue;

        if (pAttr->vecExtraCondition[0] == EENCHANTEXTRACON_REFINELV && pEquip->getRefineLv() >= pAttr->vecExtraCondition[1])
          m_pUser->m_oBuff.add(rData.extras(i).buffid());
      }
    }
    */
  };
  /*pEquipPack->foreach(func);

  bExcludeFashion = false;
  pFashionPack->foreach(func);
  */

  pEquipPack->foreach(func);

  // 计算套装属性加成
  //pEquipPack->collectSuitAttr(m_vecEquip);
  pEquipPack->collectSuitAttr();

  // fashion collect - 2016-04-19 申林移除Table_EquipFashion.txt表
  //pFashionPack->getFashionAttr(m_vecEquip);

  // suit attr
  /*for (auto m = mapSuitID2Count.begin(); m != mapSuitID2Count.end(); ++m)
  {
    const SSuitCFG* pCFG = ItemManager::getMe().getSuitCFG(m->first);
    if (pCFG == nullptr)
      continue;

    const TVecAttrSvrs& vecAttrs = pCFG->getSuitAttr(m->second);
    for (auto v = vecAttrs.begin(); v != vecAttrs.end(); ++v)
    {
      DWORD value = m_vecEquip[v->type()].value() + v->value();
      m_vecEquip[v->type()].set_value(value);
    }
  }*/

  // master strength and refine attr
  auto master = [equipset]() -> bool
  {
    static EEquipType type[] = {EEQUIPTYPE_WEAPON, EEQUIPTYPE_SHIELD, EEQUIPTYPE_ARMOUR, EEQUIPTYPE_ROBE, EEQUIPTYPE_SHOES, EEQUIPTYPE_ACCESSORY};
    for (EEquipType e : type)
    {
      if (equipset.test(e) == false)
        return false;
    }

    return true;
  };
  if (master() == true)
  {
    const SMasterCFG* pCFG = ItemManager::getMe().getMasterCFG(EMASTERTYPE_STRENGTH, strengthMasterlv);
    if (pCFG != nullptr)
    {
      setEquipMasterLv(pCFG->dwLv);
      for (auto v = pCFG->vecAttrs.begin(); v != pCFG->vecAttrs.end(); ++v)
        modifyCollect(ECOLLECTTYPE_EQUIP, *v);
        /*{
        DWORD value = m_vecEquip[v->type()].value() + v->value();
        m_vecEquip[v->type()].set_value(value);
      }*/
    }
    else
    {
      setEquipMasterLv(0);
    }

    pCFG = ItemManager::getMe().getMasterCFG(EMASTERTYPE_REFINE, refineMasterlv);
    if (pCFG != nullptr)
    {
      setRefineMasterLv(pCFG->dwLv);
      for (auto v = pCFG->vecAttrs.begin(); v != pCFG->vecAttrs.end(); ++v)
        modifyCollect(ECOLLECTTYPE_EQUIP, *v);
        /*{
        DWORD value = m_vecEquip[v->type()].value() + v->value();
        m_vecEquip[v->type()].set_value(value);
      }*/
    }
    else
    {
      setRefineMasterLv(0);
    }
  }

  modifyAttr(m_vecLastEquip, m_vecEquip);
}

void UserAttribute::collectGuildAttr()
{
  Attribute::collectGuildAttr();

  //if (m_pUser->getGuildID() == 0)
    //return;

  const GuildUserInfo& rInfo = m_pUser->getGuild().getGuildUserInfo();
  for (int i = 0; i < rInfo.prays_size(); ++i)
  {
    const GuildMemberPray& pray = rInfo.prays(i);

    LuaManager::getMe().call<void>("calcGuildPrayAttr", dynamic_cast<xSceneEntryDynamic*>(m_pUser), pray.pray(), pray.lv());

    // 移除祈祷属性仅在pvp/gvg生效的限制
    //if (m_pUser->getScene() == nullptr || (m_pUser->getScene()->isPVPScene() == false && m_pUser->getScene()->getSceneType() != SCENE_TYPE_GUILD_FIRE))
      //continue;

    const SGuildPrayCFG* pCFG = GuildConfig::getMe().getGuildPrayCFG(pray.pray());
    if (pCFG != nullptr)
    {
      const SGuildPrayItemCFG* pItemCFG = pCFG->getItem(pray.lv());
      if (pItemCFG != nullptr)
      {
        for (auto &v : pItemCFG->vecAttrs)
          modifyCollect(ECOLLECTTYPE_GUILD, v);
          //m_vecGuild[v.type()].set_value(m_vecGuild[v.type()].value() + v.value());
      }
    }
  }
  if (m_bPrayLevelup)
  {
    for (auto &v : m_vecGuild)
    {
      if (v.value() == 0)
        continue;
      if (m_vecGuild[v.type()].value() > m_vecLastGuild[v.type()].value())
      {
        const RoleData* pData = RoleDataConfig::getMe().getRoleData(v.type());
        if (pData != nullptr)
          MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_SHOWATTR, MsgParams(pData, m_vecGuild[v.type()].value() - m_vecLastGuild[v.type()].value()));
      }
    }
  }
  modifyAttr(m_vecLastGuild, m_vecGuild);
}

void UserAttribute::updateMonsterAttr()
{
  for (auto v = m_vecFinalAttrs.begin(); v != m_vecFinalAttrs.end(); ++v)
  {
    if (v->type() == EATTRTYPE_HP || v->type() == EATTRTYPE_SP)
      continue;

    setAttr(v->type(), m_vecBase[v->type()].value() + getOtherAttr(v->type()));
  }

  // for hp change
  if (getAttr(EATTRTYPE_HP) > getAttr(EATTRTYPE_MAXHP))
    setAttr(EATTRTYPE_HP, getAttr(EATTRTYPE_MAXHP));

  EAttrType eType;
  float value = 0;
  // atkattr
  eType = EATTRTYPE_ATKATTR;
  value = getBuffAttr(eType);
  value = (value <= 0 || value > 10) ? 5 : value;
  setAttr(EATTRTYPE_ATKATTR, value);

  const SNpcCFG* pBase = NpcConfig::getMe().getNpcCFG(m_pUser->getTransform().getMonsterID());
  if (pBase == nullptr)
    return;
  // defattr
  eType = EATTRTYPE_DEFATTR;
  value = getBuffAttr(eType);
  DWORD configValue = static_cast<DWORD> (pBase->eNatureType);
  configValue = (configValue <= 0 || configValue > 10) ? 5 : configValue;
  value = (value <= 0 || value > 10) ? configValue : value;
  setAttr(eType, value);
}

void UserAttribute::collectMonsterAttr()
{
  DWORD baseid = m_pUser->getTransform().getMonsterID();
  const SNpcCFG* pBase = NpcConfig::getMe().getNpcCFG(baseid);
  if (pBase == nullptr)
    return;

  for (auto v = pBase->vecAttrs.begin(); v != pBase->vecAttrs.end(); ++v)
    m_vecBase[v->type()].set_value(v->value());
}

bool UserAttribute::needSync(EAttrType eType)
{
  UserAttrSvr& rFinal = m_vecFinalAttrs[eType];
  UserAttrSvr& rExtra = m_vecFinalPointAttrs[eType];

  UserAttrSvr& rSyncFinal = m_vecSyncFinalAttrs[eType];
  UserAttrSvr& rSyncExtra = m_vecFinalPointAttrs[eType];

  bool bSync = rFinal.value() != rSyncFinal.value() || rExtra.value() != rSyncExtra.value();
  if (!bSync)
    return false;

  rSyncFinal.set_value(rFinal.value());
  rSyncExtra.set_value(rExtra.value());
  return true;
}

void UserAttribute::setSyncAttr(EAttrType eType, float fValue)
{
  if (eType >= m_vecSyncFinalAttrs.size())
    return;
  m_vecSyncFinalAttrs[eType].set_value(fValue);
}

void UserAttribute::collectAstrolabe()
{
  Attribute::collectAstrolabe();
  //m_pUser->getAstrolabes().collectAttr(m_vecAstrolabe);
  m_pUser->getAstrolabes().collectAttr();
}

void UserAttribute::collectAchieveTitle()
{
  Attribute::collectAchieveTitle();
  //m_pUser->getTitle().collectAttr(m_vecAchieveTitle);
  //m_pUser->getManual().collectAttr(m_vecAchieveTitle);
  m_pUser->getTitle().collectAttr();
  m_pUser->getManual().collectAttr();
}

void UserAttribute::collectFood()
{
  Attribute::collectFood();
  //m_pUser->getSceneFood().collectAttr(m_vecFood);
  m_pUser->getSceneFood().collectAttr();
}

void UserAttribute::collectProfession()
{
  Attribute::collectProfession();
  //m_pUser->m_oProfession.collectAttr(m_vecProfession);
  m_pUser->m_oProfession.collectAttr();
}

bool UserAttribute::fromBlobAttr(const BlobAttr& rBlob)
{
  for (int i = EATTRTYPE_NOSKILL; i <= EATTRTYPE_FUNCLIMIT; ++i)
    setSyncAttr(static_cast<EAttrType>(i), -1.0f);
  return true;
}

bool UserAttribute::toBlobAttr(BlobAttr* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  for (auto &i : AttributeValidEnum::get())
  {
    Cmd::UserAttrSvr* pData = pBlob->add_datas();
    if (pData == nullptr)
    {
      XERR << "[属性-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << i << "protobuf error" << XEND;
      continue;
    }
    pData->CopyFrom(m_vecFinalAttrs[i]);
  }

  XDBG << "[属性-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << pBlob->ByteSize() << XEND;
  return true;
}

bool UserAttribute::toPreviewBlobAttr(BlobAttr* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  auto save_attr = [&](EAttrType eType)
  {
    Cmd::UserAttrSvr* pData = pBlob->add_datas();
    if (pData == nullptr)
    {
      XERR << "[属性预览-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "protobuf error" << XEND;
      return;
    }

    float fvalue = getAttr(eType);
    if (RoleDataConfig::getMe().needSyncPercent(eType))
    {
      fvalue = fvalue * FLOAT_TO_DWORD;
    }

    int dvalue = floor(fvalue);
    fvalue = fvalue - dvalue;

    pData->set_type(eType);
    pData->set_value(floor(fvalue * 1000 + 0.5) / 1000 + dvalue);
  };

  save_attr(EATTRTYPE_STR);
  save_attr(EATTRTYPE_INT);
  save_attr(EATTRTYPE_AGI);
  save_attr(EATTRTYPE_DEX);
  save_attr(EATTRTYPE_VIT);
  save_attr(EATTRTYPE_LUK);

  save_attr(EATTRTYPE_MAXHP);
  save_attr(EATTRTYPE_MAXHPPER);
  save_attr(EATTRTYPE_MAXSP);
  save_attr(EATTRTYPE_MAXSPPER);
  save_attr(EATTRTYPE_ATK);
  save_attr(EATTRTYPE_ATKPER);
  save_attr(EATTRTYPE_DEF);
  save_attr(EATTRTYPE_DEFPER);
  save_attr(EATTRTYPE_MATK);
  save_attr(EATTRTYPE_MATKPER);
  save_attr(EATTRTYPE_MDEF);
  save_attr(EATTRTYPE_MDEFPER);
  save_attr(EATTRTYPE_CRI);
  save_attr(EATTRTYPE_ATKSPD);

  return true;
}

