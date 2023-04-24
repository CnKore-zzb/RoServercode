/**
 * @file BufferState.cpp
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-20
 */
#include "BufferState.h"
#include "xSceneEntryDynamic.h"
#include "SceneUser2.pb.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneNpc.h"
#include "MiscConfig.h"
#include "RecordCmd.pb.h"
#include "SkillItem.h"
#include "SkillManager.h"
#include "Package.h"
#include "SceneFighter.h"
#include "MsgManager.h"
#include "AstrolabeConfig.h"
#include "FighterSkill.h"
#include "GMCommandRuler.h"
#include "SceneNpcManager.h"

BufferState::BufferState()
{
}

BufferState::~BufferState()
{
}

bool BufferState::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  m_dwID = id;

  // 执行规则, 时间/次数
  buffCondition = buffCond;
  const xLuaData& dataCond = data.getData("Continue");

  m_dwInterval = dataCond.getTableFloat("interval") * 1000; //ms
  m_bDelayDoEffect = dataCond.getTableInt("delay_effect")==1;

  xLuaData datatime = dataCond.getData("time");
  m_FmTime.load(datatime);
  m_bAttackDel = dataCond.getTableInt("attack_release") == 1;
  m_dwAttackCount = dataCond.getTableInt("attack_count");
  m_dwBeAttackCount = dataCond.getTableInt("be_attack_count");
  m_fDamagePer = dataCond.getTableFloat("damage_per");

  if (dataCond.has("count"))
    m_maxCnt = dataCond.getTableInt("count");

  m_bNeedFakeDead = (dataCond.getTableInt("fakedead") != 0);
  // buff添加概率
  xLuaData dataOdds = data.getData("BuffRate").getData("Odds");
  m_FmOdds.load(dataOdds);

  xLuaData& dynamicD = data.getMutableData("DynamicData");
  if (dynamicD.has("EffectOdds"))
    m_FmEffectOdds.load(dynamicD.getMutableData("EffectOdds"));
  if (dynamicD.has("ChangeLayer"))
    m_FmLayer.load(dynamicD.getMutableData("ChangeLayer"));
  if (dynamicD.has("BeAttackCount"))
    m_FmBeAttackCnt.load(dynamicD.getMutableData("BeAttackCount"));
  if (dynamicD.has("DamageMaxPer"))
    m_FmDamagerPer.load(dynamicD.getMutableData("DamageMaxPer"));

  // buff 对象 : 1 self, 2 enemy, 3 team, 4 friend
  DWORD tar = data.getData("BuffTarget").getTableInt("type");
  m_eTargetType = static_cast<EBuffTargetType>(tar);

  xLuaData& bufftarget = data.getMutableData("BuffTarget");
  if (bufftarget.has("range"))
    m_fTargetRange = bufftarget.getTableFloat("range");
  if (bufftarget.has("inner_range"))
    m_fTargetInnerRange = bufftarget.getTableFloat("inner_range");

  m_dwTargetNumLmt = bufftarget.getTableInt("num_limit");
  if (!m_dwTargetNumLmt) m_dwTargetNumLmt = TARGET_NUM_LIMIT;
  if (bufftarget.has("dynamic_range"))
    m_FmRange.load(bufftarget.getMutableData("dynamic_range"));
  m_dwTeamCount = bufftarget.getTableInt("team_count");
  m_dwTargetLmtDistanceCnt = bufftarget.getTableInt("distance_count");
  m_fTargetLmtDistance = bufftarget.getTableFloat("distance");
  m_dwEntryType = bufftarget.getTableInt("entry_type");
  if (bufftarget.has("monster_race"))
  {
    xLuaData &race = bufftarget.getMutableData("monster_race");
    auto get_race = [this](std::string key, xLuaData &data)
    {
      m_setMonsterRace.insert(data.getInt());
  //    XDBG << "[BuffTarget],race" << data.getInt() << XEND;
    };
    race.foreach(get_race);
  }
  m_bIncludeSelf = bufftarget.getTableFloat("no_self") == 0;

  // 是否可以重复添加
  m_bIsOverLay = data.getTableInt("IsOverlay") != 0;

  // buff attr属性信息
  xLuaData& dataEffect = data.getMutableData("BuffEffect");
  auto index = [this](std::string key, xLuaData &data)
  {
    DWORD id = RoleDataConfig::getMe().getIDByName(key.c_str());
    if (id && Cmd::EAttrType_IsValid(id))
    {
      if (id == EATTRTYPE_ATTREFFECT || id == EATTRTYPE_ATTREFFECT2)// || EATTRTYPE_STATEEFFECT)
      {
        auto funa = [this, &id](std::string keyer, xLuaData& dataer)
        {
          FmData st;
          st.eType = (Cmd::EAttrType)id;
          st.value = dataer.getInt();
          m_FmAttrs.push_back(st);
        };
        data.foreach(funa);
      }
      else
      {
        FmData st;
        st.eType = (Cmd::EAttrType)id;
        st.load(data);
        m_FmAttrs.push_back(st);

        if (!m_bDynamicAttr && st.hasDynamicAttr())
          m_bDynamicAttr = true;
      }

      if (!m_bHasAttr && id != EATTRTYPE_HP && id != EATTRTYPE_SP)
        m_bHasAttr = true;
    }
  };
  dataEffect.foreach(index);

  m_dwLimitLayers = dataEffect.getTableInt("limit_layer");
  m_eStatusType = static_cast<EBuffStatusType> (dataEffect.getTableInt("StateEffect"));
  if (m_eStatusType >= EBUFFSTATUS_MAX)
  {
    XERR << "[BufferConfig], id=" << id << "不合法的状态值" << XEND;
    return false;
  }
  m_dwAtkDef_Priority = dataEffect.getTableInt("atk_def_priority");

  // 其他属性
  //m_isgain = data.getData("BuffType").getTableInt("isgain");
  m_dwIconType = data.getTableInt("IconType");

  // 保护时间
  m_dwProtectTime = data.getTableInt("Protect");
  m_eDelType = static_cast<EBuffDelType> (data.getData("Delete").getTableInt("type"));
  m_bNotSave = data.getMutableData("Delete").getTableInt("not_save");
  m_dwNeedAction = data.getMutableData("Delete").getTableInt("need_action");
  m_dwNeedEffectLineID = data.getMutableData("Delete").getTableInt("need_line");

  m_strBuffTips = data.getTableString("Dsc");
  m_dwShaderColor = data.getTableInt("EffectColor");
  m_strBuffName = data.getTableString("BuffName");
  m_dwFailBuff = data.getTableInt("FailBuff");
  if (m_dwFailBuff == id)
  {
    XERR << "[BufferConfig], id=" << id << "FailBuff 不合法" <<XEND;
    return false;
  }
  m_bDeathKeep = data.getTableInt("DeadDelete") == 1;
  m_bMultiTime = data.getTableInt("MultiTime") == 1;
  m_bNeedSourceName = data.getTableInt("NeedSourceName") == 1;
  m_dwTargetMsg = data.getTableInt("TargetMsg");
  m_bOfflineKeep = data.getTableInt("OffLineNotDelete") == 1;
  m_dwOffSetBuffID = data.getTableInt("OffSetBuff");
  m_bLayerDiffTime = data.getTableInt("LayerDiffTime") == 1;
  m_bOverLayerNoAdd = data.getTableInt("OverLayerNoAdd") == 1;
  m_strBuffTypeName = dataEffect.getTableString("type");
  m_bDeathAdd = data.getTableInt("DeadCanAdd") == 1;
  m_bCalcOnce = dataEffect.getTableInt("calc_once")==1;
  m_bIsForceStatus = dataEffect.getTableInt("IsForceStatus") == 1;

  m_dwEndLayerBuff = dataEffect.getTableInt("end_layer_buff");
  auto getextrabuff = [&](const string& key, xLuaData& d)
  {
    m_setEndExtraBuff.insert(d.getInt());
  };
  dataEffect.getMutableData("end_extra_buff").foreach(getextrabuff);

  m_bGainBuff = data.getMutableData("BuffType").getTableInt("isgain");
  m_bCanDisperse = data.getMutableData("BuffType").getTableInt("isdisperse");
  if (m_bCanDisperse && m_FmTime.empty())
  {
    XERR << "[Buff配置], id=" << id << "isdisperse配置错误, 非限时buff不可以被驱散" << XEND;
    return false;
  }
  m_dwResistImmune = data.getTableInt("ResistImmune");

  auto getdelbuffid = [&](const string& key, xLuaData& d)
  {
    m_setDelBuffID.insert(d.getInt());
  };
  data.getMutableData("DelBuffID").foreach(getdelbuffid);

  m_dwStateEffectID = data.getTableInt("BuffStateID");
  m_eIgnoreAttrControl = static_cast<EIgnoreAttrControl>(dataEffect.getTableInt("IgnoreAttrControl"));
  m_bNotDelOffSetBuff = dataEffect.getTableInt("NotDelOffSetBuff") == 1;
  return true;
}

bool BufferState::getMyAttr(xSceneEntryDynamic* me, const SBufferData* pData, TVecAttrSvrs& attrs)
{
  for (auto &v : m_FmAttrs)
  {
    float value = v.getFmValue(me, pData);
    //值插入m_oAttr
    UserAttrSvr attr;
    attr.set_type(v.eType);
    attr.set_value(value);
    attrs.push_back(attr);
  }

  return true;
}

DWORD BufferState::getBuffOdds(xSceneEntryDynamic *me, UINT fromID, DWORD lv)
{
  if (!me) return 0;
  SBufferData tmpdata;
  tmpdata.fromID = fromID;
  tmpdata.lv = lv;
  float odds = m_FmOdds.getFmValue(me, &tmpdata);
  return odds > 0 ? odds : 0;
}

QWORD BufferState::getBuffLastTime(xSceneEntryDynamic *me, UINT fromID, DWORD lv)
{
  if (!me) return 0;
  SBufferData tmpdata;
  tmpdata.fromID = fromID;
  tmpdata.lv = lv;
  return m_FmTime.getFmValue(me, &tmpdata) * ONE_THOUSAND;
}

DWORD BufferState::getLimitAddLayers(xSceneEntryDynamic* me, UINT fromID, DWORD lv)
{
  if (m_dwLimitLayers)
    return m_dwLimitLayers;
  if (m_FmLayer.empty() == false)
  {
    if (!me) return 0;
    SBufferData tmpdata;
    tmpdata.fromID = fromID;
    tmpdata.lv = lv;
    return m_FmLayer.getFmValue(me, &tmpdata);
  }

  return 0;
}

DWORD BufferState::getEffectOdds(xSceneEntryDynamic* me, xSceneEntryDynamic* pTarget, UINT fromID, DWORD lv)
{
  if (m_FmEffectOdds.empty())
    return 100;
  if (me == nullptr || pTarget == nullptr)
    return 0;
  SBufferData tmpdata;
  tmpdata.lv = lv;
  tmpdata.fromID = fromID;
  // 1 : 玩家释放技能, 给怪添加buff, fromid->玩家, me->怪, pTarget->怪  => fromid and me
  // 2 : 玩家自己添加buff, 打怪, fromid->玩家, me->玩家, pTarget->怪 => fromid and pTarget
  if (pTarget->id == fromID)
  {
    return m_FmEffectOdds.getFmValue(me, &tmpdata);
  }
  else if (me->id == fromID)
  {
    return m_FmEffectOdds.getFmValue(pTarget, &tmpdata);
  }

  return 0;
}

const TSetSceneEntrys& BufferState::getBuffTargets(const SBufferData& bData, xSceneEntryDynamic* me, xSceneEntryDynamic* enemy)
{
  m_setTarget.clear();

  if (me == nullptr || me->getScene() == nullptr)
    return m_setTarget;

  float frange = m_fTargetRange;
  if (frange == 0 && !m_FmRange.empty())
    frange = m_FmRange.getFmValue(me, &bData);

  switch(m_eTargetType)
  {
    case EBUFFTARGET_NONE:
      break;
    case EBUFFTARGET_SELF:
      m_setTarget.insert(me);
      break;
    case EBUFFTARGET_ENEMY:
      if (frange == 0 && enemy != nullptr)
      {
        m_setTarget.insert(enemy);
      }
      else
      {
        auto filter_enemy = [this](xSceneEntryDynamic *me, xSceneEntrySet &set)
        {
          for (auto &s : set)
          {
            xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
            if (!pEntry) continue;

            if (pEntry->isDiffZoneNoDam() && me->isNoDamMetalNpc())
            {
              continue;
            }
            if (!m_setMonsterRace.empty())
            {
              if (SCENE_ENTRY_NPC == pEntry->getEntryType())
              {
                if (m_setMonsterRace.find(pEntry->getRaceType()) == m_setMonsterRace.end())
                {
                  continue;
                }
              }
            }
            if (me->isMyEnemy(pEntry))
            {
              m_setTarget.insert(pEntry);
            }

            if (m_dwTargetNumLmt && m_setTarget.size() >= m_dwTargetNumLmt)
            {
              break;
            }
          }
        };
        xSceneEntrySet uSet;
        if (!m_dwEntryType || (m_dwEntryType & EBUFF_TARGET_ENTRY_TYPE_NPC))
        {
          me->getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, me->getPos(), frange, uSet);
          filter_enemy(me, uSet);
          uSet.clear();
          if (m_dwTargetNumLmt && m_setTarget.size() >= m_dwTargetNumLmt)
          {
            break;
          }
        }
        if (!m_dwEntryType || (m_dwEntryType & EBUFF_TARGET_ENTRY_TYPE_USER))
        {
          me->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, me->getPos(), frange, uSet);
          filter_enemy(me, uSet);
          uSet.clear();
        }
      }
      break;
    case EBUFFTARGET_TEAM:
      {
        if (frange == 0)
          break;
        SceneUser* pUser = dynamic_cast<SceneUser*> (me);
        if (pUser)
        {
          if (pUser->getScene() == nullptr)
            break;

          if (m_bIncludeSelf)
            m_setTarget.insert(pUser);
          const TMapGTeamMember& mapMember = pUser->getTeam().getTeamMemberList();
          for (auto &m : mapMember)
          {
            if (m_dwTargetNumLmt && m_setTarget.size() >= m_dwTargetNumLmt)
              break;
            if (!m_bIncludeSelf && m.second.charid() == pUser->id)
              continue;
            SceneUser* pTeamer = SceneUserManager::getMe().getUserByID(m.second.charid());
            if (pTeamer == nullptr || pTeamer->getScene() != pUser->getScene())
              continue;
            float dist = getDistance(pUser->getPos(), pTeamer->getPos());
            if (dist > frange)
              continue;
            m_setTarget.insert(pTeamer);
          }
        }
        else
        {
          SceneNpc* pNpc = dynamic_cast<SceneNpc*> (me);
          if (!pNpc)
            break;
          if (pNpc->isWeaponPet())
          {
            WeaponPetNpc* pWPNpc = dynamic_cast<WeaponPetNpc*>(pNpc);
            if (pWPNpc)
            {
              SceneUser* user = pWPNpc->getMasterUser();
              if (user && getDistance(user->getPos(), pNpc->getPos()) <= frange)
                m_setTarget.insert(user);
            }
          }
        }
      }
      break;
    case EBUFFTARGET_FRIEND:
      {
        if (frange == 0)
          break;

        xSceneEntrySet uSet;
        me->getScene()->getEntryList(me->getPos(), frange, uSet);
        for (auto &s : uSet)
        {
          if (m_dwTargetNumLmt && m_setTarget.size() > m_dwTargetNumLmt)
            break;
          xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
          if (pEntry == nullptr)
            continue;
          if (me->isMyEnemy(pEntry) == false)
          {
            SceneNpc* pNpc = dynamic_cast<SceneNpc*> (pEntry);
            if (pNpc != nullptr && pNpc->getNpcID() < 10000)
              continue;
            m_setTarget.insert(pEntry);
          }
        }
      }
      break;
    case EBUFFTARGET_BUFFADDER:
      {
        xSceneEntryDynamic* entry = xSceneEntryDynamic::getEntryByID(bData.fromID);
        if (entry == nullptr)
          break;
        if (frange && getXZDistance(me->getPos(), entry->getPos()) > frange)
          break;
        m_setTarget.insert(entry);
      }
      break;
    default:
      break;
  }

  if (m_fTargetInnerRange)
  {
    for (auto s = m_setTarget.begin(); s != m_setTarget.end(); )
    {
      if (getXZDistance((*s)->getPos(), me->getPos()) < m_fTargetInnerRange)
      {
        s = m_setTarget.erase(s);
        continue;
      }
      ++s;
    }
  }
  return m_setTarget;
}

const TSetSceneEntrys& BufferState::getBuffTargetsByTargetType(const SBufferData& bData, xSceneEntryDynamic*me, EBuffTargetType eType, xSceneEntryDynamic* enemy, float range)
{
  m_setTarget.clear();

  if (me == nullptr || me->getScene() == nullptr)
    return m_setTarget;

  float frange = m_fTargetRange;
  if (range)
    frange = range;
  else if (frange == 0 && !m_FmRange.empty())
    frange = m_FmRange.getFmValue(me, &bData);

  switch(eType)
  {
    case EBUFFTARGET_NONE:
      break;
    case EBUFFTARGET_SELF:
      m_setTarget.insert(me);
      break;
    case EBUFFTARGET_ENEMY:
      if (frange == 0 && enemy != nullptr)
      {
        m_setTarget.insert(enemy);
      }
      else
      {
        xSceneEntrySet uSet;
        me->getScene()->getEntryList(me->getPos(), frange, uSet);
        for (auto &s : uSet)
        {
          if (m_dwTargetNumLmt && m_setTarget.size() >= m_dwTargetNumLmt)
            break;

          xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
          if (pEntry && pEntry->isDiffZoneNoDam() && me->isNoDamMetalNpc())
            continue;
          if (pEntry && me->isMyEnemy(pEntry))
            m_setTarget.insert(pEntry);
        }
      }
      break;
    case EBUFFTARGET_TEAM:
      {
        if (frange == 0)
          break;
        SceneUser* pUser = dynamic_cast<SceneUser*> (me);
        if (pUser)
        {
          if (pUser->getScene() == nullptr)
            break;

          if (m_bIncludeSelf)
            m_setTarget.insert(pUser);
          const TMapGTeamMember& mapMember = pUser->getTeam().getTeamMemberList();
          for (auto &m : mapMember)
          {
            if (m_dwTargetNumLmt && m_setTarget.size() >= m_dwTargetNumLmt)
              break;
            if (!m_bIncludeSelf && m.second.charid() == pUser->id)
              continue;
            SceneUser* pTeamer = SceneUserManager::getMe().getUserByID(m.second.charid());
            if (pTeamer == nullptr || pTeamer->getScene() != pUser->getScene())
              continue;
            float dist = getDistance(pUser->getPos(), pTeamer->getPos());
            if (dist > frange)
              continue;
            m_setTarget.insert(pTeamer);
          }
        }
        else
        {
          SceneNpc* pNpc = dynamic_cast<SceneNpc*> (me);
          if (!pNpc)
            break;
          if (pNpc->isWeaponPet())
          {
            WeaponPetNpc* pWPNpc = dynamic_cast<WeaponPetNpc*>(pNpc);
            if (pWPNpc)
            {
              SceneUser* user = pWPNpc->getMasterUser();
              if (user && getDistance(user->getPos(), pNpc->getPos()) <= frange)
                m_setTarget.insert(user);
            }
          }
        }
      }
      break;
    case EBUFFTARGET_FRIEND:
      {
        if (frange == 0)
          break;

        xSceneEntrySet uSet;
        me->getScene()->getEntryList(me->getPos(), frange, uSet);
        for (auto &s : uSet)
        {
          if (m_dwTargetNumLmt && m_setTarget.size() > m_dwTargetNumLmt)
            break;
          xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
          if (pEntry == nullptr)
            continue;
          if (me->isMyEnemy(pEntry) == false)
          {
            SceneNpc* pNpc = dynamic_cast<SceneNpc*> (pEntry);
            if (pNpc != nullptr && pNpc->getNpcID() < 10000)
              continue;
            m_setTarget.insert(pEntry);
          }
        }
      }
      break;
    case EBUFFTARGET_BUFFADDER:
      {
        xSceneEntryDynamic* entry = xSceneEntryDynamic::getEntryByID(bData.fromID);
        if (entry == nullptr)
          break;
        if (frange && getXZDistance(me->getPos(), entry->getPos()) > frange)
          break;
        m_setTarget.insert(entry);
      }
      break;
    default:
      break;
  }

  if (m_fTargetInnerRange)
  {
    for (auto s = m_setTarget.begin(); s != m_setTarget.end(); )
    {
      if (getXZDistance((*s)->getPos(), me->getPos()) < m_fTargetInnerRange)
      {
        s = m_setTarget.erase(s);
        continue;
      }
      ++s;
    }
  }
  return m_setTarget;
}

bool BufferState::needSyncNine() const
{
  if (m_dwStateEffectID)
    return true;
  const TSetDWORD& setpvpbuffs = MiscConfig::getMe().getBuffMiscCFG().setNineSyncBuffs;
  return setpvpbuffs.find(m_dwID) != setpvpbuffs.end();
}

bool BufferState::canContinue(const SBufferData& bData, QWORD curm)
{
  // 基础属性类buff 不执行
  if (m_eBuffType == EBUFFTYPE_ATTRCHANGE)
    return false;

  // check interval
  if (curm < bData.timeTick)
    return false;

  DWORD curCnt = bData.count;
  QWORD endTime = bData.endTime;

  if (m_maxCnt == 0 && endTime == 0)
    return true;

  //执行次数优先，时间次之；count=0，time=0,表示一直可以执行
  if (m_maxCnt == 0)
  {
    if (curm > endTime)
      return false;
  }
  else if (curCnt >= m_maxCnt)
  {
    return false;
  }
  return true;
}

void BufferState::sortInit()
{
  // init 结束后调用一次
  m_bCheckDel = needCheckDel();
  m_bCheckEffect = needCheckEffect();
}

/* 该buff是否需要每帧执行效果*/
bool BufferState::needCheckEffect() const
{
  switch(m_eBuffType)
  {
    case EBUFFTYPE_MIN:
    case EBUFFTYPE_NORM:
    case EBUFFTYPE_ATTRCHANGE:
      return false;
    default:
      return true;
  }
  return false;
}

bool BufferState::needCheckDel() const
{
  if (m_bNeedFakeDead)
    return true;
  if (m_dwNeedAction)
    return true;
  if (m_dwNeedEffectLineID)
    return true;
  if (m_dwAttackCount != 0)
    return true;
  if (m_dwBeAttackCount != 0)
    return true;
  if (!m_FmBeAttackCnt.empty())
    return true;
  if (m_fDamagePer != 0)
    return true;
  if (!m_FmDamagerPer.empty())
    return true;

  switch(m_eDelType)
  {
    case EBUFFDELTYPE_MIN:
    case EBUFFDELTYPE_FOREVER:
      return false;
    case EBUFFDELTYPE_TIME:
    case EBUFFDELTYPE_COUNT:
    case EBUFFDELTYPE_FAKEDEAD:
    case EBUFFDELTYPE_LAYER:
      return true;
  }

  return false;
}

bool BufferState::needDel(const SBufferData& bData, QWORD cur)
{
  //endtime = 0,表示不以时间为终止条件
  if (m_bNeedFakeDead == true)
  {
    if (bData.me != nullptr && (bData.me)->getStatus() == ECREATURESTATUS_FAKEDEAD)
      return false;
    return true;
  }
  if (m_dwNeedAction)
  {
    if (bData.me && bData.me->getAction() != m_dwNeedAction)
      return true;
  }

  // 需要维持连线状态
  if (m_dwNeedEffectLineID)
  {
    bool hasline = false;
    if (bData.me && bData.me->getSpEffect().hasLine(m_dwNeedEffectLineID))
      hasline = true;
    if (!hasline && bData.me)
    {
      xSceneEntryDynamic* fromEntry = xSceneEntryDynamic::getEntryByID(bData.fromID);
      if (fromEntry && fromEntry->getSpEffect().hasLineEntry(m_dwNeedEffectLineID, bData.me->id))
        hasline = true;
    }
    if (!hasline)
      return true;
  }

  if (m_eDelType == EBUFFDELTYPE_MIN)
    return false;

  if (m_dwAttackCount != 0 && bData.dwAttackCount >= m_dwAttackCount)
    return true;

  // 受击次数删除
  if (m_dwBeAttackCount != 0 && bData.dwBeAttackCount >= m_dwBeAttackCount)
    return true;
  if (!m_FmBeAttackCnt.empty())
  {
    DWORD cnt = m_FmBeAttackCnt.getFmValue(bData.me, &bData);
    if (bData.dwBeAttackCount >= cnt)
      return true;
  }

  if (m_fDamagePer != 0)
  {
    if (bData.me == nullptr || bData.me->getAttr(EATTRTYPE_MAXHP) == 0)
      return false;
    float per = bData.dwTotalDamage / bData.me->getAttr(EATTRTYPE_MAXHP);
    if (per >= m_fDamagePer)
      return true;
  }

  if (!m_FmDamagerPer.empty())
  {
    float maxper = m_FmDamagerPer.getFmValue(bData.me, &bData);
    if (bData.me == nullptr || bData.me->getAttr(EATTRTYPE_MAXHP) == 0)
      return false;
    float per = bData.dwTotalDamage / bData.me->getAttr(EATTRTYPE_MAXHP);
    if (per >= maxper)
      return true;
  }

  if (m_eDelType == EBUFFDELTYPE_TIME)
    return cur > bData.endTime;
  if (m_eDelType == EBUFFDELTYPE_COUNT)
  {
    DWORD cnt = m_bIsOverLay ? m_maxCnt * bData.layers : m_maxCnt;
    if (bData.count >= cnt)
      return true;
    else if (bData.endTime != 0)
      return cur > bData.endTime;
  }
  if (bData.layers == 0)
    return true;

  return false;
}

bool BufferState::onStart(SBufferData& bData, DWORD cur)
{
  bData.activeFlag = true;
  return true;
}

void BufferState::onInvalid(SBufferData& bData)
{
  bData.activeFlag = false;
}

bool BufferState::canStart(SBufferData& bData, DWORD cur)
{
  if (m_eBuffType == EBUFFTYPE_MULTITIME)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
    if (user == nullptr)
      return false;
    return user->getUserSceneData().haveEnoughBattleTime();
  }

  if (buffCondition == nullptr)
    return false;

  return buffCondition->checkCondition(bData, cur);
}

/**************************************/
/**************************************/
/**************************************/


void SBufferData::calcAttr()
{
  if (!m_oAttr.empty() && pBuff->isCalcOnce())
    return;
  m_oAttr.clear();
  TVecAttrSvrs attrs;
  if (pBuff->getMyAttr(me, this, attrs))
  {
    for (auto &v : attrs)
    {
      UserAttrSvr attr;
      attr.set_type(v.type());
      if (v.type() == EATTRTYPE_ATTREFFECT || v.type() == EATTRTYPE_STATEEFFECT || v.type() == EATTRTYPE_ATTRFUNCTION || v.type() == EATTRTYPE_ATTREFFECT2 || v.type() == EATTRTYPE_FUNCLIMIT)
        attr.set_value(v.value());
      else
        attr.set_value(v.value() * layers);

      m_oAttr.push_back(attr);
    }
  }
}

const TVecAttrSvrs& SBufferData::getAttr()
{
  if (me && me->id == fromID)
    calcAttr();
  return m_oAttr;
}

DWORD SBufferData::getLimitLayer()
{
  if (me == nullptr || pBuff == nullptr)
    return 0;
  return pBuff->getLimitAddLayers(me, fromID, lv);
}

BufferStateList::BufferStateList(xSceneEntryDynamic *entry)
  :m_pEntry(entry)
{
}

BufferStateList::~BufferStateList()
{
}

void BufferStateList::onAdd(TPtrBufferState buffPtr, QWORD fromid, DWORD lv, bool repeat)
{
  if (buffPtr == nullptr)
    return;

  for(auto s : buffPtr->m_setDelBuffID)
  {
    del(s);
  }

  if (buffPtr->getBuffType() == EBUFFTYPE_HIDE)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
    if (pUser)
    {
      if (pUser->m_oHands.has() && pUser->m_oHands.isMaster())
      {
        SceneUser* pOther = pUser->m_oHands.getOther();
        if (pOther)
        {
          pOther->m_oBuff.add(buffPtr->m_dwID, nullptr, lv);
        }
      }

      std::list<SceneNpc*> npclist;
      pUser->getAllFriendNpcs(npclist);
      for (auto &l : npclist)
        l->m_oBuff.add(buffPtr->m_dwID, nullptr, lv);
    }
  }

  if (!repeat)
  {
    //target msg
    if (buffPtr->m_dwTargetMsg)
    {
      SceneUser* pSrcUser = SceneUserManager::getMe().getUserByID(fromid);
      if (pSrcUser && m_pEntry && m_pEntry->getEntryType() == SCENE_ENTRY_USER)
      {
        MsgParams params;      
        params.addString(pSrcUser->getName());
        MsgManager::sendMsg(m_pEntry->getTempID(), buffPtr->m_dwTargetMsg, params);
      }
    }

    if (buffPtr->getBuffType() == EBUFFTYPE_TRANSFORM)
    {
      auto func = [&](SBufferData& r)
      {
        auto p = r.pBuff;
        if (!p) return;
        if (r.canRemove() && !p->m_bDeathKeep)
        {
          del(r.id);
        }
        else if (p->getBuffType() == EBUFFTYPE_TRANSFORM)
        {
          del(r.id);
        }
      };
      foreach(func);
    }
    if (buffPtr->getBuffType() == EBUFFTYPE_PARTTRANSFORM)
    {
      if (m_bPartTransform == false)
        return;
      BuffPartTransform* pTrans = dynamic_cast<BuffPartTransform*> (buffPtr.get());
      if (pTrans == nullptr)
        return;
      auto m = m_mapType2SetBuff.find(EBUFFTYPE_PARTTRANSFORM);
      if (m != m_mapType2SetBuff.end())
      {
        for (auto &s : m->second)
        {
          SBufferData* pData = getBuffData(s);
          if (pData == nullptr)
            continue;
          BuffPartTransform* pBuff = dynamic_cast<BuffPartTransform*> (pData->pBuff.get());
          if (pBuff == nullptr)
            continue;

          for (auto &it : pTrans->m_mapFigure)
          {
            if (pBuff->has(it.first))
            {
              del(s);
              break;
            }
          }
        }
      }
    }

    if (buffPtr->getBuffType() == EBUFFTYPE_ELEMENT)
    {
      BuffBattleAttr* pSelf = dynamic_cast<BuffBattleAttr*> (buffPtr.get());
      if (pSelf == nullptr || pSelf->isDisOther() == false)
        return;
      auto it = m_mapType2SetBuff.find(EBUFFTYPE_ELEMENT);
      if (it != m_mapType2SetBuff.end())
      {
        for (auto &s : it->second)
        {
          SBufferData* pData = getBuffData(s);
          if (pData == nullptr)
            continue;
          BuffBattleAttr* pBat = dynamic_cast<BuffBattleAttr*> (pData->pBuff.get());
          if (pBat == nullptr)
            continue;
          if (!(pSelf->haveAtkAttr() && pBat->haveAtkAttr())
              && !(pSelf->haveDefAttr() && pBat->haveDefAttr()))
            continue;
          //m.second.activeFlag = false;
          if (pBat->isDisOther())
          {
            pData->activeFlag = false;
            del(s);
          }
        }
      }
    }
    //中毒,冰冻,石化有任何两种效果同时存在时,后者替换前者
    //恐惧可以解除定身和沉默的效果,定身可解除诅咒效果,但是诅咒不能解除
    if (buffPtr->getBuffType() == EBUFFTYPE_STATUS)
    {
      BuffStatusChange *pBuff = (BuffStatusChange *)&(*buffPtr);
      SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
      if (pUser)
      {
        pUser->getEvent().onBeBuffStatus(pBuff->getStatus());
        pUser->playDynamicExpression(EAVATAREXPRESSION_DBUFF_LOOP);
      }
      else
      {
        SceneNpc* pNpc = dynamic_cast<SceneNpc*> (m_pEntry);
        xSceneEntryDynamic* pSrcEntry = xSceneEntryDynamic::getEntryByID(fromid);
        if (pNpc && pSrcEntry)
          pNpc->m_ai.putAttacker(pSrcEntry);
      }
      switch (pBuff->getStatus())
      {
        case EBUFFSTATUS_DIZZY:
          {
            m_pEntry->checkEmoji("BeStun");
            m_pEntry->checkLockMeEmoji("EnemyStun");
            m_pEntry->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_BUFF, fromid, pBuff->getStatus());
            m_pEntry->m_oMove.stopAtonce();
          }
          break;
        case EBUFFSTATUS_SILENCE:
          {
            m_pEntry->checkEmoji("BeSilent");
            m_pEntry->checkLockMeEmoji("EnemySilent");
            m_pEntry->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_BUFF, fromid, pBuff->getStatus());
          }
          break;
        case EBUFFSTATUS_POSION:
          {
            m_pEntry->checkEmoji("BePoison");
            m_pEntry->checkLockMeEmoji("EnemyPoison");
            delStatus(EBUFFSTATUS_FREEZE);
            delStatus(EBUFFSTATUS_STONE);
          }
          break;
          /*
        case BUFFER_STATE_SLOW:
          {
            m_pEntry->checkEmoji("BeSlow");
            m_pEntry->checkLockMeEmoji("EnemySlow");
          }
          break;
          */
        case EBUFFSTATUS_FREEZE:
          {
            m_pEntry->checkEmoji("BeFreeze");
            m_pEntry->checkLockMeEmoji("EnemyFreeze");
            m_pEntry->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_BUFF, fromid, pBuff->getStatus());
            m_pEntry->m_oMove.stopAtonce();
            delStatus(EBUFFSTATUS_POSION);
            delStatus(EBUFFSTATUS_STONE);
          }
          break;
        case EBUFFSTATUS_STONE:
          {
            m_pEntry->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_BUFF, fromid, pBuff->getStatus());
            m_pEntry->m_oMove.stopAtonce();
            delStatus(EBUFFSTATUS_POSION);
            delStatus(EBUFFSTATUS_FREEZE);
          }
          break;
        case EBUFFSTATUS_FEAR:
          {
            m_pEntry->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_BUFF, fromid, pBuff->getStatus());
            delStatus(EBUFFSTATUS_NOMOVE);
            delStatus(EBUFFSTATUS_SILENCE);
          }
          break;
        case EBUFFSTATUS_NOMOVE:
          {
            m_pEntry->m_oMove.stopAtonce();
            delStatus(EBUFFSTATUS_CURSE);
          }
          break;
        case EBUFFSTATUS_SLEEP:
          {
            m_pEntry->m_oMove.stopAtonce();
            m_pEntry->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_BUFF, fromid, pBuff->getStatus());
          }
          break;
        default:
          break;
      }
    }

    if (m_pEntry->id != fromid && m_pEntry->getEntryType() == SCENE_ENTRY_USER)
    {
      SceneUser* user = SceneUserManager::getMe().getUserByID(fromid);
      if (user && user->isMyEnemy(m_pEntry) == false)
      {
        SceneUser* pMe = dynamic_cast<SceneUser*> (m_pEntry);
        if (pMe)
          pMe->onSkillHealMe(user, 0);
      }
    }
  }
}

void BufferStateList::onAddLater(TPtrBufferState buffPtr, QWORD fromid, DWORD level, QWORD endtime, bool ignoreOdds)
{
  if (buffPtr == nullptr)
    return;
  if (canReboundStatus(buffPtr->getStatus()))
  {
    if (!ignoreOdds)
    {
      xSceneEntryDynamic* pFrom = xSceneEntryDynamic::getEntryByID(fromid);
      if(pFrom)
      {
        XDBG << "[Buff-添加], buff rebound:" << buffPtr->getBuffName() << "buff来源:" << pFrom->name << "buff结束时间:" << endtime << XEND;
        pFrom->m_oBuff.add(buffPtr->getID(), m_pEntry, level, 0, endtime, true);
      }
    }
  }

  if (buffPtr->getBuffType() == EBUFFTYPE_STATUS)
  {
    if (m_pEntry->getEntryType() == SCENE_ENTRY_NPC)
    {
      ((SceneNpc*)(m_pEntry))->m_sai.checkSig("onbuff");
    }
  }

  if (m_pEntry->getEntryType() == SCENE_ENTRY_NPC)
  {
    ((SceneNpc*)m_pEntry)->m_sai.checkSig("buff_layer");
  }

  if (m_bLoadingFromUser == false)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
    if (pUser != nullptr)
      pUser->getQuest().onBuff();
  }

  if (buffPtr->m_dwTeamCount > 0)
  {
    SceneUser* pSrc = SceneUserManager::getMe().getUserByID(fromid);
    if (pSrc != nullptr)
    {
      DWORD cnt = 0;
      QWORD minEndTime = QWORD_MAX;
      xSceneEntryDynamic* pMin = nullptr;
      for (auto& v : pSrc->getTeam().getTeamMemberList(true))
      {
        if (fromid == v.second.charid())
          continue;
        xSceneEntryDynamic* t = xSceneEntryDynamic::getEntryByID(v.second.charid());
        if (t == nullptr || t->getScene() != pSrc->getScene() || t->m_oBuff.getBuffFromID(buffPtr->m_dwID) != fromid)
          continue;
        QWORD et = t->m_oBuff.getEndTimeByID(buffPtr->m_dwID);
        if (et > 0)
        {
          ++cnt;
          if (et < minEndTime)
          {
            minEndTime = et;
            pMin = t;
          }
        }
      }
      if (cnt > buffPtr->m_dwTeamCount && pMin != nullptr)
      {
        pMin->m_oBuff.del(buffPtr->m_dwID);
      }
    }
  }
  if (buffPtr->m_dwTargetLmtDistanceCnt > 0)
  {
    SceneUser* pSrc = SceneUserManager::getMe().getUserByID(fromid);
    if (pSrc != nullptr && pSrc->getScene())
    {
      DWORD cnt = 0;
      QWORD minEndTime = QWORD_MAX;
      xSceneEntryDynamic* pMin = nullptr;
      xSceneEntrySet uSet;
      pSrc->getScene()->getEntryList(pSrc->getPos(), buffPtr->m_fTargetLmtDistance, uSet);
      for (auto& u : uSet)
      {
        xSceneEntryDynamic* v = dynamic_cast<xSceneEntryDynamic*>(u);
        if (v == nullptr || v->id == fromid || v->getScene() != pSrc->getScene() || v->m_oBuff.getBuffFromID(buffPtr->m_dwID) != fromid)
          continue;
        QWORD et = v->m_oBuff.getEndTimeByID(buffPtr->m_dwID);
        if (et > 0)
        {
          ++cnt;
          if (et < minEndTime)
          {
            minEndTime = et;
            pMin = v;
          }
        }
      }
      if (cnt > buffPtr->m_dwTargetLmtDistanceCnt && pMin != nullptr)
      {
        pMin->m_oBuff.del(buffPtr->m_dwID);
      }
    }
  }

  checkBuffEnable(BUFFTRIGGER_BUFF);
}

bool BufferStateList::canAdd(TPtrBufferState buffPtr)
{
  if (buffPtr == nullptr)
    return false;

  // 死亡状态不可添加时限性buff
  if (m_pEntry->isAlive() == false && buffPtr->isTimeBuff() && !buffPtr->isDeathCanAdd())
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser)
  {
    // 牵手不可变身
    if (pUser->m_oHands.has() && buffPtr->getBuffType() == EBUFFTYPE_TRANSFORM)
      return false;
  }
  // 无敌 不可添加 debuff
  /*if (m_pEntry->m_blGod || m_pEntry->isGod())
  {
    if (buffPtr->isGainBuff() == false)
      return false;
  }
  */
  if (buffPtr->getStatus() != 0)
  {
    if (isImmuneStatus(buffPtr->getStatus()))
    {
      if (buffPtr->getBuffType() == EBUFFTYPE_STATUS && buffPtr->m_dwResistImmune & 1)
        GMCommandRuler::getMe().effect(m_pEntry, MiscConfig::getMe().getEffectPath().oImmuneEffect);

      return false;
    }
    if (canAddStatus(buffPtr->getStatus()) == false)
      return false;
  }
  if (buffPtr->getBuffType() == EBUFFTYPE_TAUNT)
  {
    if (m_pEntry->getAttr(EATTRTYPE_HIDE) != 0)
      return false;
    if (m_pEntry->getEntryType() == SCENE_ENTRY_NPC && ((SceneNpc*)(m_pEntry))->m_ai.isImmuneTaunt())
      return false;
  }

  //自身带有不能隐身的属性,则无法添加隐身Buff 
  if ((buffPtr->getBuffType() == EBUFFTYPE_HIDE || buffPtr->getBuffType() == EBUFFTYPE_DEEPHIDE) && m_pEntry->isNotHide())
  {
    return false;
  }

  //自身带有免疫增益buff的属性，或者免疫减益buff的属性，则无法添加增益buff或减益buff
  if ((buffPtr->m_bGainBuff == true && buffPtr->m_bCanDisperse == true) && m_pEntry->isImmuneGainBuff())
  {
    return false;
  }
  if ((buffPtr->m_bGainBuff == false && buffPtr->m_bCanDisperse == true) && m_pEntry->isImmuneReductionBuff())
  {
    return false;
  }

  return true;
}

bool BufferStateList::add(DWORD id, xSceneEntryDynamic *fromEntry, DWORD lv, int damage, QWORD endtime, bool ignoreOdds)
{
  fromEntry = fromEntry ? fromEntry : m_pEntry;
  if (m_pEntry == nullptr || id == 0 || fromEntry == nullptr)
    return false;

  TPtrBufferState buffPtr = BufferManager::getMe().getBuffById(id);
  if (buffPtr == nullptr)
  {
    XERR << "[Buff-添加], 找不到对应的buff配置, id:" << id << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
    return false;
  }

  if (m_pEntry->isDiffZoneNoDam() && fromEntry->isNoDamMetalNpc())
    return false;
  if (m_pEntry->isMyEnemy(fromEntry) && (m_pEntry->isNoAttacked() || m_pEntry->isNoEnemySkilled()))
    return false;

  if (canAdd(buffPtr) == false)
    return false;
    
  if (buffPtr->m_bOverLayerNoAdd && buffPtr->isOverlay())
  {
    SBufferData* pData = getBuffData(id);
    if (pData && pData->layers >= pData->getLimitLayer())
      return false;
  }

  if (ignoreOdds == false && (DWORD)randBetween(1, 100) > buffPtr->getBuffOdds(m_pEntry, fromEntry->id, lv))
  {
    // 状态抵抗
    if (buffPtr->m_dwResistImmune & 2)
      GMCommandRuler::getMe().effect(m_pEntry, MiscConfig::getMe().getEffectPath().oResistEffect);

    if (buffPtr->getFailBuff() == 0)
      return false;
    return add(buffPtr->getFailBuff(), fromEntry, lv, damage);
  }

  QWORD curm = xTime::getCurMSec();
  DWORD cur = curm / ONE_THOUSAND;

  // 免疫异常状态
  if (buffPtr->getStatus())
  {
    auto it = m_mapType2SetBuff.find(EBUFFTYPE_RESISTSTATUS);
    if (it != m_mapType2SetBuff.end())
    {
      for (auto &s : it->second)
      {
        SBufferData* pData = getBuffData(s);
        if (pData == nullptr)
          continue;
        BuffResistStatus* pBuff = dynamic_cast<BuffResistStatus*> (pData->pBuff.get());
        if (pBuff == nullptr)
          continue;
        if (pBuff->isImmuneStatus(buffPtr->getStatus()) == false)
          continue;

        // 避免同一个技能两个不同buff同时计数
        if (cur < pData->dwCommonData)
          return false;

        // 检查cd
        if (pBuff->getInterval())
        {
          if (curm < pData->timeTick)
            continue;
          pData->timeTick = curm + pBuff->getInterval();
        }

        pData->dwCommonData = cur + 1; // 避免同一个技能两个不同buff同时计数
        pData->count ++;
        return false;
      }
    }
  }
  // 护盾, 免疫敌方buff
  if (fromEntry->isMyEnemy(m_pEntry))
  {
    auto it = m_mapType2SetBuff.find(EBUFFTYPE_IMMUNE);
    if (it != m_mapType2SetBuff.end() && !it->second.empty())
    {
      DWORD buffid = *(it->second.begin());
      SBufferData* pData = getBuffData(buffid);
      if (pData != nullptr)
      {
        pData->dwBeAttackCount ++;
        return false;
      }
    }
  }

  // check protective time
  auto itpro = m_mapID2Protecttime.find(id);
  if (itpro != m_mapID2Protecttime.end())
  {
    if (cur < itpro->second)
      return false;
  }
  m_mapID2Protecttime[id] = cur + buffPtr->getProtectTime();

  SBufferUpdateData data;
  data.id = id;
  data.fromID = fromEntry != nullptr ? fromEntry->id : m_pEntry->id;
  data.lv = lv;
  data.dwDamage = damage;
  data.endTime = endtime;
  data.bIgnoreOdds = ignoreOdds;

  m_listUpdateData.push_back(make_pair(true, data));
  return true;
}

bool BufferStateList::realAdd(const SBufferUpdateData& stData)
{
  DWORD id = stData.id;
  QWORD fromID = stData.fromID;
  DWORD lv = stData.lv;
  int damage = stData.dwDamage;
  QWORD endtime = stData.endTime;

  xSceneEntryDynamic* pSourceEntry = xSceneEntryDynamic::getEntryByID(fromID);
  if (pSourceEntry == nullptr)
    return false;

  TPtrBufferState buffPtr = BufferManager::getMe().getBuffById(id);
  if (buffPtr == nullptr)
  {
    XERR << "[Buff-添加], 找不到对应的buff配置, id:" << id << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
    return false;
  }

  if (checkOffSet(buffPtr) == true)
  {
    XDBG << "[Buff-抵消], 添加Buff:" << id << "被抵消" << "Buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
    return true;
  }
  QWORD charid = pSourceEntry->id;
  QWORD curm = xTime::getCurMSec();
  DWORD cur = curm / ONE_THOUSAND;

  bool repeat = false;
  DWORD lastLayers = 0;
  QWORD beforeTime = 0;
  SBufferData* pOldData = getBuffData(id);
  if (pOldData != nullptr)
  {
    repeat = pOldData->lv == lv || buffPtr->isOverlay();
    lastLayers = pOldData->layers;
    beforeTime = pOldData->endTime;
  }

  // before add
  onAdd(buffPtr, charid, lv, repeat);

  //add NEW buff
  SBufferData& buffdata = buffPtr->isStaticBuff() ? m_mapID2StaticBuff[id] : m_mapID2BuffData[id];
  buffdata.id = id;
  buffdata.count = 0;
  QWORD lasttime = buffPtr->getBuffLastTime(m_pEntry, charid, lv);
  if (endtime == 0)
  {
    if (repeat && buffPtr->m_bMultiTime && lasttime != 0)
      buffdata.endTime = beforeTime + lasttime;
    else
      buffdata.endTime = (lasttime == 0 ? 0 : curm + lasttime);
  }
  else
  {
    buffdata.endTime = endtime;
  }
  
  // 每层不同结束时间
  if (buffPtr->m_bLayerDiffTime && buffPtr->isOverlay())
    buffdata.queueEndTime.push(curm + lasttime);

  buffdata.lv = lv;
  buffdata.activeFlag = false;
  buffdata.pBuff = buffPtr;
  buffdata.me = m_pEntry;
  buffdata.fromID = charid;
  buffdata.dwDamage = damage;
  buffdata.hpOnAdd = m_pEntry->getAttr(EATTRTYPE_HP);
  buffdata.dwBeAttackCount = 0;
  buffdata.dwAttackCount = 0;
  buffdata.bSyncNine = buffPtr->needSyncNine();

  if (buffPtr->m_bNeedSourceName)
    buffdata.strFromName = pSourceEntry->name;

  addTypeBuff(buffPtr, id);

  if (repeat)
  {
    if (stData.layer)
      buffdata.layers = lastLayers + stData.layer;
    else
      buffdata.layers = buffPtr->isOverlay() ? lastLayers + 1 : 1;

    DWORD limitlayer = buffdata.getLimitLayer();
    if (limitlayer != 0 && buffdata.layers > limitlayer)
    {
      buffdata.layers = limitlayer;
      if (buffdata.queueEndTime.size() > limitlayer)
        buffdata.queueEndTime.pop(); // 刷新最近一层的结束时间
    }
  }
  else
  {
    if (stData.layer)
      buffdata.layers = stData.layer;
    else
      buffdata.layers = 1;
    buffdata.addTime = cur;

    if (buffPtr->m_bDelayDoEffect) // 添加时必须经过interval时间才会执行
      buffdata.timeTick = curm + buffPtr->getInterval();
  }
  buffdata.calcAttr();

  if (buffPtr->getStatus() != 0 && pSourceEntry != m_pEntry && pSourceEntry->m_oBuff.isForceAddStatus(buffPtr->getStatus()))
    buffdata.setStatusNoDisp(); // 表示此状态不能被驱散

  if (!(repeat && buffPtr->isRingBuff()) || buffdata.fromID != m_pEntry->id)
    addUpdate(buffdata);

  bool disabled = m_setDisables.find(id) != m_setDisables.end();
  //添加时检查是否可以立即执行
  if (!disabled && buffPtr->canStart(buffdata, cur))
  {
    buffPtr->onStart(buffdata, cur);
    if ((repeat == false || buffdata.layers != 1 || buffdata.fromID != m_pEntry->id) && (buffPtr->isCalcOnce() == false || repeat == false))
    {
      setDataMark(buffPtr);
    }
  }
  if (buffPtr->isPermanentBuff() && m_bNoBuffMsg == false)
  {
    MsgManager::sendMsg(m_pEntry->id, 552, MsgParams(buffPtr->getBuffTips()));
  }
  if (m_bLoadingFromUser == false && repeat == false)
    XLOG << "[Buff-添加], buff:" << buffPtr->getBuffName() << id << "buff来源:" << pSourceEntry->name << "buff对象:" << m_pEntry->name << m_pEntry->id << "time:" << curm << XEND;

  XDBG << "[Buff-添加], buff:" << buffPtr->getBuffName() << id << "buff来源:" << m_pEntry->name << "buff结束时间:" << buffdata.endTime << XEND;
  onAddLater(buffPtr, fromID, lv, buffdata.endTime, stData.bIgnoreOdds);

  return true;
}

bool BufferStateList::checkOffSet(TPtrBufferState buffPtr)
{
  if (buffPtr == nullptr)
    return false;
  if (buffPtr->m_dwOffSetBuffID == 0)
    return false;
  SBufferData* pData = getBuffData(buffPtr->m_dwOffSetBuffID);
  if (pData == nullptr)
    return false;
  if (buffPtr->m_bNotDelOffSetBuff == false)
    del(pData->id);
  return true;
}

void BufferStateList::addLayers(DWORD id, DWORD layers)
{
  SBufferData* pData = getBuffData(id);
  if (pData != nullptr)
  {
    if (!pData->pBuff)
      return;
    DWORD limitlayer = pData->getLimitLayer();
    if (limitlayer != 0 && pData->layers > limitlayer)
      return;
    pData->layers += layers;
    addUpdate(*pData);
  }
  else
  {
    SBufferUpdateData data;
    data.id = id;
    data.fromID = m_pEntry->id;
    data.layer = layers;

    m_listUpdateData.push_back(make_pair(true, data));
  }

  XLOG << "[Buff-层数添加], buff:" << id << "buff对象:" << m_pEntry->name << m_pEntry->id << "添加层数:" << layers << XEND;
}

bool BufferStateList::active(DWORD id)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr)
    return false;
  if (pData->activeFlag == false)
    pData->activeFlag = true;

  return true;
}

void BufferStateList::checkBuffEnable(EBuffType eType)
{
  auto it = m_mapType2SetBuff.find(eType);
  if (it != m_mapType2SetBuff.end())
    checkBuffEnable(it->second);
}

void BufferStateList::checkBuffEnable(BuffTriggerType eTrigType)
{
  auto it = m_mapTrigType2Set.find(eTrigType);
  if (it != m_mapTrigType2Set.end())
    checkBuffEnable(it->second);
}

void BufferStateList::checkBuffEnable(const TSetDWORD& ids)
{
  DWORD cur = now();
  for (auto &s : ids)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    auto pBuff = pData->pBuff;
    if (pBuff == nullptr)
      continue;
    if (pData->activeFlag == false && pBuff->canStart(*pData, cur))
    {
      pBuff->onStart(*pData, cur);
      setDataMark(pBuff);
      if (pBuff->getBuffType() == EBUFFTYPE_TRANSFORM)
      {
        BuffTransform* transform = dynamic_cast<BuffTransform*>(pBuff.get());
        if (transform)
          transform->doTransform(*pData);
      }
    }
    else if (pData->activeFlag == true && !pBuff->canStart(*pData, cur))
    {
      pBuff->onInvalid(*pData);
      setDataMark(pBuff);

      if (pBuff->getTrigType() == BUFFTRIGGER_EQUIP)
      {
        ItemCondition* pCond = dynamic_cast<ItemCondition*>(pBuff->getCondition().get());
        if (pCond != nullptr && pCond->delWhenInvalid() == true)
          del(pData->id);
      }
    }
  }
}

// attr change
void BufferStateList::onAttrChange()
{
  checkBuffEnable(BUFFTRIGGER_ATTR);
  checkBuffEnable(BUFFTRIGGER_ABNORMAL);

  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;
  pUser->getPackage().refreshEnableBuffs();
}

void BufferStateList::onHpChange()
{
  if (m_pEntry->isAlive() == false)
    return;

  checkBuffEnable(BUFFTRIGGER_DAMAGE);
  checkBuffEnable(BUFFTRIGGER_HPLESSPER);
}

void BufferStateList::onSpChange()
{
  if (m_pEntry->isAlive() == false)
    return;
  checkBuffEnable(BUFFTRIGGER_SPLESSPER);
}

//职业改变
void BufferStateList::onProfesChange(EProfession oldProfes)
{
  if (m_pEntry == nullptr)
    return;
  // 职业变化触发buff
  checkBuffEnable(BUFFTRIGGER_PROFES);
  checkBuffEnable(EBUFFTYPE_CHANGESKILL);

  // 技能变化, 对应buff改变
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;
  TVecDWORD vecSkIDs;

  SceneFighter* pFiter = pUser->getFighter(oldProfes);
  if (pFiter == nullptr)
    return;
  pFiter->getSkill().getCurSkills(vecSkIDs);
  for (auto v = vecSkIDs.begin(); v != vecSkIDs.end(); ++v)
  {
    delSkillBuff(*v);
  }

  vecSkIDs.clear();
  if (pUser->getFighter() == nullptr)
    return;
  pUser->getFighter()->getSkill().getCurSkills(vecSkIDs);
  for (auto v = vecSkIDs.begin(); v != vecSkIDs.end(); ++v)
  {
    addSkillBuff(*v);
  }

  // 职业变化影响装备tip显示
  pUser->getPackage().refreshEnableBuffs();

  // 职业变化, 装备卡片 有效性变化
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return;

  auto func = [&](ItemBase* pBase)
  {
    const ItemEquip* pEquip = dynamic_cast<const ItemEquip*>(pBase);
    if (pEquip == nullptr)
      return;
    bool oldCanEquip = pEquip->canEquip(oldProfes);
    bool nowCanEquip = pEquip->canEquip(pUser->getUserSceneData().getProfession());
    if (oldCanEquip && !nowCanEquip)
      onEquipChange(pBase, EEQUIPOPER_OFF);
    else if (!oldCanEquip && nowCanEquip)
      onEquipChange(pBase, EEQUIPOPER_ON);
  };
  pEquipPack->foreach(func);
}

//装备改变
void BufferStateList::onEquipChange(ItemBase* pItem, EEquipOper oper)
{
  //equip影响已有buff生效
  checkBuffEnable(BUFFTRIGGER_EQUIP);

  if (oper == EEQUIPOPER_ON)
    addEquipBuff(pItem);
  else if (oper == EEQUIPOPER_OFF || oper == EEQUIPOPER_OFFPOS)
    delEquipBuff(pItem);

  //检查坐骑变化 
  onRideMountChange(pItem);
}

void BufferStateList::onRideMountChange(ItemBase* pItem)
{
  if (pItem->getType() != EITEMTYPE_MOUNT)
    return;

  checkBuffEnable(BUFFTRIGGER_MOUNT);
}

// card buff
void BufferStateList::onCardChange(DWORD dwTypeID, bool isAdd)
{
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (!pUser)
    return;
  checkBuffEnable(BUFFTRIGGER_EQUIP);

  const SItemCFG* pItem = ItemManager::getMe().getItemCFG(dwTypeID);
  if (pItem == nullptr)
    return;
  if (isAdd)
    addCardBuff(pItem);
  else
    delCardBuff(pItem);
}

//攻击别人时 给自己或敌方加buff buff来着装备，卡片等
void BufferStateList::onAttack(xSceneEntryDynamic* enemy, const BaseSkill* pSkill, DamageType damtype, DWORD damage)
{
  if (enemy == nullptr || enemy->getScene() == nullptr)
    return;
  for (auto &it : m_mapID2BuffData)
  {
    SBufferData* pData = getBuffData(it.first);
    if (pData == nullptr || pData->canNotTrig)
    {
      continue;
    }
    auto p = it.second.pBuff;
    if (p == nullptr || !it.second.activeFlag)
      continue;
    if (p->m_bAttackDel)
    {
      del(it.first);
    }
    if (p->m_dwAttackCount)
    {
      it.second.dwAttackCount ++;
    }

    if (p->getTrigType() == BUFFTRIGGER_ATTACK)
    {
      if (p->canContinue(it.second, 0) == false)
        continue;
      it.second.dwDamage = damage;
      AttackCondition* pCond = dynamic_cast<AttackCondition*> (p->getCondition().get());
      if (pCond && pCond->isCheckOk(m_pEntry, enemy, pSkill, damtype, &(it.second)))
      {
        pData->setCondSkillID(pSkill->getSkillID());
        p->doBuffEffect(m_pEntry, p->getBuffTargets(it.second, m_pEntry, enemy), it.second);
        pData->setCondSkillID(0);
      }
    }

    if (p->getTrigType() == BUFFTRIGGER_USESKILLKILL)
    {
      it.second.dwDamage = damage;
      UseSkillKillCondition* pcond = dynamic_cast<UseSkillKillCondition*> (p->getCondition().get());
      if (pcond && pcond->isCheckOk(pSkill, m_pEntry, enemy))
      {
        p->doBuffEffect(m_pEntry, p->getBuffTargets(it.second, m_pEntry, enemy), it.second);
      }
    }

    if (enemy->isAlive() == false && enemy->getEntryType() == SCENE_ENTRY_NPC)
    {
      if (p->getTrigType() == BUFFTRIGGER_KILLNPC)
      {
        it.second.dwDamage = damage;
        KillCondition* pcond = dynamic_cast<KillCondition*> (p->getCondition().get());
        if (pcond->isTheRace(enemy, pSkill))
          p->doBuffEffect(m_pEntry, p->getBuffTargets(it.second, m_pEntry), it.second);
      }

      if (p->getBuffType() == EBUFFTYPE_ROBREWARD)
      {
        BuffRobReward* pRob = dynamic_cast<BuffRobReward*>(p.get());
        if (pRob == nullptr)
          continue;
        DWORD layer = it.second.layers;
        pRob->onKillMonster((SceneUser*)m_pEntry, (SceneNpc*)enemy, it.second);

        if (layer != it.second.layers)
          addUpdate(it.second);
      }
    }
  }
}

//被攻击时 给自己或敌人添加buff; 影响已有buff生效
void BufferStateList::onAttackMe(xSceneEntryDynamic* attacker, const BaseSkill* pSkill, DamageType damtype, DWORD damage)
{
  if (attacker == nullptr || attacker->isAlive() == false || m_pEntry->isAlive() == false || pSkill == nullptr)
    return;

  // 影响已有buff生效, 受到攻击时产生作用的buff (吸血盾类型), 计数
  for (auto &it : m_mapID2BuffData)
  {
    auto p = it.second.pBuff;
    if (p == nullptr || !it.second.activeFlag)
      continue;

    if (p->m_dwBeAttackCount != 0 || p->m_FmBeAttackCnt.empty() == false)
      it.second.dwBeAttackCount ++;
    if (p->m_fDamagePer != 0 || p->m_FmDamagerPer.empty() == false)
      it.second.dwTotalDamage += damage;

    if (p->getTrigType() != BUFFTRIGGER_BEATTACK)
      continue;
    if (p->canContinue(it.second, 0) == false)
      continue;
    it.second.dwDamage = damage;
    BeAttackCondition* pCond = dynamic_cast<BeAttackCondition*> (p->getCondition().get());
    if (pCond && pCond->isCheckOk(attacker, m_pEntry, pSkill, damtype, &(it.second)))
    {
      p->doBuffEffect(m_pEntry, p->getBuffTargets(it.second, m_pEntry, attacker), it.second);
    }
  }

  if (damage > 0)
  {
    // 被攻击时删除冰冻、睡眠状态
    delStatus(EBUFFSTATUS_FREEZE);
    delStatus(EBUFFSTATUS_SLEEP);
    delStatus(EBUFFSTATUS_STONE);
  }
}

//坐骑改变 影响buff生效
void BufferStateList::onMountChange()
{
  checkBuffEnable(BUFFTRIGGER_EQUIP);
}

// 做表情
void BufferStateList::onEmoji(DWORD id)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_HANDEMOJI);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->pBuff == nullptr)
        continue;
      if (pData->activeFlag == false)
        continue;
      BuffHandEmoji* pBuff = dynamic_cast<BuffHandEmoji*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      pBuff->onPlayEmoji(id, *pData);
    }
  }
}

// 复活
void BufferStateList::onRelive()
{
  auto it = m_mapTrigType2Set.find(BUFFTRIGGER_REBORN);
  if (it != m_mapTrigType2Set.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      if (pData->activeFlag == false)
        continue;
      auto p = pData->pBuff;
      if (p == nullptr)
        continue;
      p->doBuffEffect(m_pEntry, p->getBuffTargets(*pData, m_pEntry), *pData);
    }
  }

  // 复活之后添加buff
  const SReliveCFG& rCFG = MiscConfig::getMe().getReliveCFG();
  for (auto &v : rCFG.vecBuff)
    add(v);
}

void BufferStateList::refreshBuffAtonce()
{
  if (m_bClear)
  {
    clear();
    m_bClear = false;
  }

  if (m_listUpdateData.empty() == false)
  {
    TListUpdateData tmplist;
    tmplist.swap(m_listUpdateData);
    for (auto &s : tmplist)
    {
      if (s.first == true)
        realAdd(s.second);
      else
        realDel(s.second.id, s.second.bAllLayer, s.second.layer);
    }
  }
}

void BufferStateList::update(QWORD curm)
{
  if (m_bClear)
  {
    clear();
    m_bClear = false;
  }

  TListUpdateData tmplist;
  if (m_listUpdateData.empty() == false)
  {
    tmplist.swap(m_listUpdateData);
    for (auto &s : tmplist)
    {
      if (s.first == true)
        realAdd(s.second);
      else
        realDel(s.second.id, s.second.bAllLayer, s.second.layer);
    }
  }

  for (auto m = m_mapID2BuffData.begin(); m != m_mapID2BuffData.end();++m)
  {
    auto p = m->second.pBuff;
    if (p == nullptr || p->needCheckDel() == false)
      continue;

    if (p->needDel(m->second, curm)) //buff 消失
      del(m->first, true);
    else if (p->m_bLayerDiffTime && m->second.queueEndTime.empty() == false)
    {
      QWORD minendtime = m->second.queueEndTime.front();
      if (curm >= minendtime)
        del(m->first);
    }
  }

  if (m_listUpdateData.empty() == false)
  {
    tmplist.clear();
    tmplist.swap(m_listUpdateData);
    for (auto &s : tmplist)
    {
      if (s.first == true)
        realAdd(s.second);
      else
        realDel(s.second.id, s.second.bAllLayer, s.second.layer);
    }
  }

  if (m_setUpdateActive.empty() == false)
    updateDisable(curm / ONE_THOUSAND);

  if (m_pEntry->isAlive())
  {
    DWORD execTimeOut = CommonConfig::m_dwBuffExecPrintTime;
    xTime allBuffFrame;
    xTime frameTimer;
    for (auto it = m_mapID2BuffData.begin(); it != m_mapID2BuffData.end(); ++it)
    {
      auto p = it->second.pBuff;
      if (p == nullptr || p->getCondition() == nullptr || p->needCheckEffect() == false)
        continue;

      frameTimer.elapseStart();
      if (it->second.activeFlag && p->canContinue(it->second, curm) && p->getCondition()->canContinue())
      {
        p->doBuffEffect(m_pEntry, p->getBuffTargets(it->second, m_pEntry), it->second);

        QWORD e = frameTimer.uElapse();
        if (e >= execTimeOut)
          XLOG << "[Buff-执行超时统计], 对象:" << m_pEntry->name << m_pEntry->id << "buff:" << it->first << "执行时间:" << e << "微秒" << XEND;

        if (m_pEntry->getStatus() == ECREATURESTATUS_DEAD)
          break;
      }
    }
    QWORD e = allBuffFrame.uElapse();
    if (e >= execTimeOut)
      XLOG << "[Buff-执行超时统计], 执行所有buff超时, 对象:" << m_pEntry->name << m_pEntry->id << "buff列表大小:" << m_mapID2BuffData.size() << "执行时间:" << e << "微妙" << XEND;
  }

  // 变身后删除skill, equip  buff
  if (m_bDelSkillEquipBuff)
  {
    m_bDelSkillEquipBuff = false;
    delSkillEquipBuff();

    SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
    if (user)
      user->getPackage().ride(ERIDETYPE_OFF);
  }
  // 变身结束,添加skill, equip buff
  if (m_bAddSkillEquipBuff)
  {
    addSkillEquipBuff();
    m_bAddSkillEquipBuff = false;
  }

  if (m_pEntry->getAttribute() && m_pEntry->getAttribute()->checkNeedRefreshBuff())
  {
    m_pEntry->getAttribute()->setRefreshBuff();
    m_pEntry->refreshDataAtonce();
  }
  // update buffer to nine
  updateBuffToNine();
  updatePermanentBuff();
}

void BufferStateList::updateBuffStatus(DWORD cur)
{
  if (m_pEntry->isOnPvp() == false && m_pEntry->isOnGvg() == false)
    return;

  if (cur >= m_dwStatusTimeTick)
  {
    m_dwStatusTimeTick = cur + 1;

    if (m_pEntry->getEntryType() == SCENE_ENTRY_USER)
    {
      const SBuffStatusCFG& rCFG = MiscConfig::getMe().getBuffStatusCFG();
      DWORD state = m_pEntry->getAttr(EATTRTYPE_STATEEFFECT);

      for (auto &m : rCFG.mapBuffStatusData)
      {
        DWORD status = m.first;
        auto it = m_mapStatus2Record.find(status);
        if (it == m_mapStatus2Record.end())
        {
          pair<DWORD, DWORD>& pa = m_mapStatus2Record[status];
          pa.first = 0;
          pa.second = cur + m.second.dwPeriod;
        }
        else
        {
          if (state & (1 << (status - 1)))
          {
            it->second.first += 1;
            if (it->second.first >= m.second.dwMaxTime)
              delStatus(status);
          }
          if (cur >= it->second.second)
          {
            it->second.first = 0;
            it->second.second = cur + m.second.dwPeriod;
          }
        }
      }
    }
  }
}

bool BufferStateList::canAddStatus(DWORD status) const
{
  if (m_pEntry->getEntryType() == SCENE_ENTRY_NPC)
    return true;

  if (m_pEntry->isOnPvp() || m_pEntry->isOnGvg())
  {
    auto it = m_mapStatus2Record.find(status);
    if (it == m_mapStatus2Record.end())
      return true;
    const SBuffStatusCFG& rCFG = MiscConfig::getMe().getBuffStatusCFG();
    auto m = rCFG.mapBuffStatusData.find(status);
    if (m == rCFG.mapBuffStatusData.end())
      return true;

    return it->second.first < m->second.dwMaxTime;
  }

  return true;
}

void BufferStateList::timer(QWORD curm)
{
  // 100ms 刷新一次
  if (m_qwTimer >= curm / 100)
    return;
  m_qwTimer = curm / 100;

  update(curm);

  if (m_qwOneSecond < curm / 1000)
  {
    m_qwOneSecond = curm / 1000;
    updateOneSecond(curm);
  }
}

void BufferStateList::updateBuffToNine()
{
  UserBuffNineSyncCmd sync;
  UserBuffNineSyncCmd syncself;
  sync.set_guid(m_pEntry->id);
  syncself.set_guid(m_pEntry->id);

  if (m_eHideState == EHideState_Hide)
  {
    //hide
    for (auto& s : m_setClientHideBuff)
    {
      if (!haveBuff(s))
        continue;

      sync.add_dels(s);
      XDBG << "[Buff-删除-隐藏], 通知客户端Buff删除, 对象:" << m_pEntry->name << m_pEntry->id << "id=" << s << XEND;
    }
    m_eHideState = EHideState_Hided;
  }
  else if (m_eHideState == EHideState_Show)
  {
    //show
    for (auto& s : m_setClientHideBuff)
    {
      SBufferData* pBuffData = getBuffData(s);
      if (pBuffData == nullptr)
        continue;
      auto& v = *pBuffData;
      BufferData* pData = sync.add_updates();
      if (pData != nullptr)
      {
        pData->set_id(v.id);
        pData->set_layer(v.layers);
        if (!v.activeFlag)  pData->set_active(v.activeFlag);
        if (!v.strFromName.empty()) pData->set_fromname(v.strFromName);
        if (v.fromID != m_pEntry->id) pData->set_fromid(v.fromID);

        QWORD endtime = v.endTime;
        if (v.queueEndTime.empty() == false)
          endtime = v.queueEndTime.front();
        pData->set_time(endtime);
        if (v.lv != 0) pData->set_level(v.lv % 100);
      }
      XDBG << "[Buff-更新-展示], 通知客户端Buff更新, 对象:" << m_pEntry->name << m_pEntry->id << "id=" << v.id << "结束时间:" << (pData ? pData->time() : 0) << "层数:" << v.layers << XEND;
    }
    m_setClientHideBuff.clear();
    m_eHideState = EHideState_None;
  }   

  for (auto &v : m_vecAddUpdates)
  {
    if (!haveBuff(v.id))
    {
      if (v.bSyncNine) // 需同步9屏
        sync.add_dels(v.id);
      else
        syncself.add_dels(v.id);

      XDBG << "[Buff-删除], 通知客户端Buff删除, 对象:" << m_pEntry->name << m_pEntry->id << "id=" << v.id << XEND;
      continue;
    }
    //隐藏
    if (m_setClientHideBuff.find(v.id) != m_setClientHideBuff.end())
    {
      continue;
    }

    BufferData* pData = nullptr;
    if (v.bSyncNine)// 需同步9屏
      pData = sync.add_updates();
    else
      pData = syncself.add_updates();

    if (pData != nullptr)
    {
      pData->set_id(v.id);
      pData->set_layer(v.layers);
      if (!v.activeFlag)  pData->set_active(v.activeFlag);
      if (!v.strFromName.empty()) pData->set_fromname(v.strFromName);
      if (v.fromID != m_pEntry->id) pData->set_fromid(v.fromID);

      QWORD endtime = v.endTime;
      if (v.queueEndTime.empty() == false)
        endtime = v.queueEndTime.front();
      pData->set_time(endtime);
      if (v.lv != 0) pData->set_level(v.lv % 100);
    }
    XDBG << "[Buff-更新], 通知客户端Buff更新, 对象:" << m_pEntry->name << m_pEntry->id << "id=" << v.id << "结束时间:" << (pData ? pData->time() : 0) << "层数:" <<  v.layers << XEND;
  }

  if (m_bIsInStatus && (sync.dels_size() > 0 || syncself.dels_size() > 0))
  {
    TSetDWORD setIDs;
    getStatus(setIDs);
    if (setIDs.empty() == true)
    {
      m_bIsInStatus = false;
      SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
      if (pUser != nullptr)
        pUser->playDynamicExpression(EAVATAREXPRESSION_MIN);
    }
  }

  if (sync.updates_size() > 0 || sync.dels_size() > 0)
  {
    PROTOBUF(sync, send, len);
    m_pEntry->sendCmdToNine(send, len);
  }
  if (syncself.updates_size() > 0 || syncself.dels_size() > 0)
  {
    PROTOBUF(syncself, sendself, lenself);
    m_pEntry->sendCmdToMe(sendself, lenself);
  }
  m_vecAddUpdates.clear();
}

void BufferStateList::updatePermanentBuff()
{
  if (m_bNoBuffMsg)
    m_bNoBuffMsg = false;
  if (m_setPmtBuffUpdates.empty())
    return;
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;
  // notify client
  BuffForeverCmd cmd;
  for (auto &s : m_setPmtBuffUpdates)
  {
    BufferData* pData = cmd.add_buff();
    if (pData == nullptr)
      continue;
    SBufferData* p = getBuffData(s);
    if (p != nullptr)
    {
      pData->set_id(s);
      pData->set_layer(p->layers);
    }
    else
    {
      pData->set_id(s);
      pData->set_layer(0);
    }
  }
  PROTOBUF(cmd, send, len);
  m_pEntry->sendCmdToMe(send, len);

  m_setPmtBuffUpdates.clear();
}

bool BufferStateList::isInStatus(DWORD statusID)
{
  bool find = false;
  auto func = [&](SBufferData& r)
  {
    if (r.activeFlag == false)
      return;
    if (r.pBuff && r.pBuff->getStatus() == statusID)
      find = true;
  };
  foreach(func);
  return find;
}

bool BufferStateList::isImmuneStatus(DWORD status)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_DSTATUS);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      auto p = pData->pBuff;
      if (p == nullptr)
        continue;
      BuffImmuneStatus* pStatus = dynamic_cast<BuffImmuneStatus*> (p.get());
      if (pStatus == nullptr)
        return false;
      if (pStatus->isImmuneStatus(status))
        return true;
    }
  }
  return false;
}

DWORD BufferStateList::getTransform(EUserDataType uType)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_TRANSFORM);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffTransform* pTran = dynamic_cast<BuffTransform*> (pData->pBuff.get());
      if (pTran == nullptr) return 0;
      return pTran->getTransform(uType);
    }
  }
  return 0;
}

//删除 指定类型的buff
bool BufferStateList::delBuffByType(EBuffType type)
{
  auto it = m_mapType2SetBuff.find(type);
  if (it == m_mapType2SetBuff.end())
    return false;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    del(s);
    if (pData->pBuff->getBuffType() == EBUFFTYPE_HIDE)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
      if (pUser)
      {
        if (pUser->m_oHands.has() && pUser->m_oHands.isMaster())
        {
          SceneUser* pOther = pUser->m_oHands.getOther();
          if (pOther)
          {
            pOther->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
          }
        }
        std::list<SceneNpc*> npclist;
        pUser->getAllFriendNpcs(npclist);
        for (auto &l : npclist)
          l->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
      }
    }
  }
  return true;
}

bool BufferStateList::delSkillBuff(DWORD skillid)
{
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
    return false;
  if (pSkillCFG->haveDirectBuff() == false)
    return false;
  const TSetDWORD& sfbuffs = pSkillCFG->getSelfBuffs();
  for (auto v = sfbuffs.begin(); v != sfbuffs.end(); ++v)
  {
    del(*v);
    XLOG << "[Buff-技能], 删除技能buff, 技能id:" << skillid << "buffid:" << *v << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
  }
  return true;
}

bool BufferStateList::addSkillBuff(DWORD skillid)
{
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
    return false;
  if (pSkillCFG->haveDirectBuff() == false)
    return false;
  const TSetDWORD sfbuffs = pSkillCFG->getSelfBuffs();
  for (auto v = sfbuffs.begin(); v != sfbuffs.end(); ++v)
  {
    if (add(*v, m_pEntry, skillid))
      XDBG << "[Buff-技能], 添加技能buff, 技能id:" << skillid << "buffid:" << *v << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
  }

  return true;
}

bool BufferStateList::del(DWORD id, bool allLayer /*=false*/)
{
  SBufferUpdateData data;
  data.id = id;
  data.bAllLayer = allLayer;
  data.layer = 1;
  m_listUpdateData.push_back(make_pair(false, data));

  return true;
}

bool BufferStateList::delLayer(DWORD id, DWORD layer)
{
  SBufferUpdateData data;
  data.id = id;
  data.layer = layer;
  m_listUpdateData.push_back(make_pair(false, data));
  return true;
}

bool BufferStateList::realDel(DWORD id, bool bAllLayer, DWORD layer)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr)
    return false;
  auto p = pData->pBuff;
  if (p == nullptr)
    return false;
  p->ondel(m_pEntry);

  // 结束时 剩余额外层数(火狩)
  if (p->m_dwEndLayerBuff && pData->layers)
    add(p->m_dwEndLayerBuff);
  // 结束时 添加额外buff
  if (p->m_setEndExtraBuff.empty() == false)
  {
    xSceneEntryDynamic* pFrom = xSceneEntryDynamic::getEntryByID(pData->fromID);
    if (pFrom == nullptr)
      pFrom = m_pEntry;
    for (auto &s : p->m_setEndExtraBuff)
      add(s, pFrom, pData->lv, pData->dwDamage);
  }

  if (!bAllLayer && pData->layers > layer && p->isOverlay())
  {
    pData->layers -= layer;
    // 删除结束时间最小的一层
    if (p->m_bLayerDiffTime && pData->queueEndTime.empty() == false)
      pData->queueEndTime.pop();

    addUpdate(*pData);
  }
  else
  {
    addUpdate(*pData);
    delTypeBuff(p, id);
    auto it = m_mapID2BuffData.find(id);
    if (it != m_mapID2BuffData.end())
    {
      m_mapID2BuffData.erase(it);
    }
    else
    {
      auto it_2 = m_mapID2StaticBuff.find(id);
      if (it_2 != m_mapID2StaticBuff.end())
        m_mapID2StaticBuff.erase(it_2);
    }
  }
  setDataMark(p);

  if (p->getBuffType() == EBUFFTYPE_TRANSFORM)
  {
    if (haveBuffType(EBUFFTYPE_TRANSFORM) == false)
    {
      MsgManager::sendMsg(m_pEntry->id, 831);
      setClearState();
    }
  }

  checkBuffEnable(BUFFTRIGGER_BUFF);
  p->ondelLater(m_pEntry);
  
  XLOG << "[Buff-删除], buff:" << p->getBuffName() << id << "buff 对象:" << m_pEntry->name << m_pEntry->id << XEND;
  return true;
}

bool BufferStateList::addEquipBuff(ItemBase* pItem)
{
  if (!pItem)
    return false;

  ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
  if(pEquip == nullptr)// || pEquip->isDamaged())
    return false;

  if (m_pEntry == nullptr)
    return false;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  if (pUser->getTransform().isMonster())
    return false;

  if (pItem->getCFG() == nullptr)
    return false;

  addEquipBuffBreakValid(pItem);
  addEquipBuffBreakInvalid(pItem);

  return true;
}

// 装备被破坏后依然有效的buff
bool BufferStateList::addEquipBuffBreakValid(ItemBase *pItem)
{
  if (!pItem)
    return false;

  ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
  if(pEquip == nullptr)// || pEquip->isDamaged())
    return false;

  if (m_pEntry == nullptr)
    return false;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  if (pUser->getTransform().isMonster())
    return false;

  if (pItem->getCFG() == nullptr)
    return false;

  TVecEquipCard vecCardItems;
  pEquip->getEquipCard(vecCardItems);
  for( auto v = vecCardItems.begin(); v != vecCardItems.end(); ++v)
  {
    const SItemCFG* pItem = ItemManager::getMe().getItemCFG((*v).second);
    //  pUser->getPackage().getItem((*v).first);
    if (pItem == nullptr)
      return false;
    addCardBuff(pItem);
  }

  // equip enchant
  EnchantData& rData = pEquip->getEnchantData();
  const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(rData.type());
  if (pCFG != nullptr)
  {
    for (int i = 0; i < rData.extras_size(); ++i)
    {
      const SEnchantAttr* pAttr = pCFG->getEnchantAttr(rData.extras(i).configid());
      if (pAttr == nullptr || pAttr->vecExtraCondition.size() < 2)
        continue;

      if (pAttr->vecExtraCondition[0] == EENCHANTEXTRACON_REFINELV && pEquip->getRefineLv() >= pAttr->vecExtraCondition[1])
        add(rData.extras(i).buffid());
    }
  }
  return true;
}

// 装备被破坏后失效的buff
bool BufferStateList::addEquipBuffBreakInvalid(ItemBase *pItem)
{
  if (!pItem)
    return false;

  ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
  if(pEquip == nullptr)// || pEquip->isDamaged())
    return false;

  if (m_pEntry == nullptr)
    return false;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  if (pUser->getTransform().isMonster())
    return false;

  if (pItem->getCFG() == nullptr)
    return false;

  if (pEquip->isBroken())
    return true;

  const TVecDWORD& buffs = pItem->getCFG()->vecBuffIDs;
  for (auto v = buffs.begin(); v != buffs.end(); ++v)
  {
    if (add(*v, m_pEntry, 0))
      XDBG << "[Buff-装备], 添加装备buff, 装备id:" << pItem->getTypeID() << "buffid: " << *v << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
  }

  // equip refine
  const std::vector<pair<DWORD, TVecDWORD>>& refine2buffs = pItem->getCFG()->vecRefine2Buff;
  for (auto &v : refine2buffs)
  {
    if (pEquip->getRefineLv() >= v.first)
    {
      for (auto &b : v.second)
      {
        if (add(b, m_pEntry, pEquip->getRefineLv()))
          XDBG << "[Buff-装备-精练], 添加装备精炼buff, 装备id:" << pItem->getTypeID() << "buffid:" << b << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
      }
    }
  }

  // equip upgrade
  DWORD equiplv = pEquip->getLv();
  if (equiplv > 0)
  {
    for (DWORD lv = 1; lv <= equiplv; ++lv)
    {
      const SEquipUpgradeCFG* pUpgradeCFG = pItem->getCFG()->getUpgradeCFG(lv);
      if (pUpgradeCFG && pUpgradeCFG->dwBuffID)
      {
        DWORD pro = pUser->getProfession();
        DWORD evo = pro == EPROFESSION_NOVICE ? 0 : pro % 10;
        if (evo >= pUpgradeCFG->dwEvoReq)
          add(pUpgradeCFG->dwBuffID);
      }
    }
  }

  // equipsuit
  BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_EQUIP);
  if (pPack == nullptr)
    return false;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*> (pPack);
  if (pEquipPack == nullptr)
    return false;

  const TSetDWORD& suitset = ItemManager::getMe().getSuitIDs(pItem->getTypeID());
  if (!suitset.empty())
  {
    for (auto &d : suitset)
    {
      DWORD suitnum = pEquipPack->getEquipSuitNum(pUser, d);
      const SSuitCFG* pCFG = ItemManager::getMe().getSuitCFG(d);
      if (pCFG == nullptr)
        return true;

      TVecDWORD suitbuffs;

      for (DWORD i = 1; i <= suitnum; ++i)
      {
        pCFG->getBuffs(i, suitbuffs);
      }

      for (auto v = suitbuffs.begin(); v != suitbuffs.end(); ++v)
      {
        if (add(*v, m_pEntry, 0))
          XDBG << "[Buff-装备-套装], 添加装备套装buff, buffid:" << *v << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
      }

      // 套装精炼buff
      addSuitRefineBuff(pUser, pCFG, pEquip, pEquipPack);
    }
  }

  addEquipBuffPVP(pEquip);

  return true;
}

bool BufferStateList::addEquipBuffPVP(ItemEquip *pEquip)
{
  if (!m_pEntry || !pEquip || !pEquip->getCFG())
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (!pUser)
    return false;

  if (pUser->getScene() && (pUser->getScene()->isPVPScene() || pUser->getScene()->isGvg()))
  {
    const TVecDWORD& pvpbuffs = pEquip->getCFG()->vecPVPBuffIDs;
    for (auto v = pvpbuffs.begin(); v != pvpbuffs.end(); ++v)
    {
      if (add(*v, m_pEntry, 0))
        XDBG << "[Buff-装备], 添加装备pvpbuff, 装备id:" << pEquip->getTypeID() << "buffid: " << *v << "buff对象:" << pUser->name << pUser->id << XEND;
    }
  }

  return true;
}

bool BufferStateList::delEquipBuff(ItemBase* pItem)
{
  //equip & card 添加或删除 自身对应buff
  if (!pItem)
    return false;
  ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
  if(nullptr == pEquip)
    return false;
  if (m_pEntry == nullptr)
    return false;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  if (pItem->getCFG() == nullptr)
    return false;

  delEquipBuffBreakValid(pItem);
  delEquipBuffBreakInvalid(pItem);

  return true;
}

// 装备被破坏后依然有效的buff
bool BufferStateList::delEquipBuffBreakValid(ItemBase* pItem)
{
  if (!pItem)
    return false;
  ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
  if(nullptr == pEquip)
    return false;
  if (m_pEntry == nullptr)
    return false;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  if (pItem->getCFG() == nullptr)
    return false;

  TVecEquipCard vecCardItems;
  pEquip->getEquipCard(vecCardItems);

  //erase card buff if equiped card
  for( auto v = vecCardItems.begin(); v != vecCardItems.end(); ++v)
  {
    const SItemCFG* pItem = ItemManager::getMe().getItemCFG((*v).second);
    if (pItem == nullptr)
      return false;
    delCardBuff(pItem);
  }

  // del equip enchant
  EnchantData& rData = pEquip->getEnchantData();
  const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(rData.type());
  if (pCFG != nullptr)
  {
    for (int i = 0; i < rData.extras_size(); ++i)
    {
      const SEnchantAttr* pAttr = pCFG->getEnchantAttr(rData.extras(i).configid());
      if (pAttr == nullptr || pAttr->vecExtraCondition.size() < 2)
        continue;

      if (pAttr->vecExtraCondition[0] == EENCHANTEXTRACON_REFINELV && pEquip->getRefineLv() >= pAttr->vecExtraCondition[1])
        del(rData.extras(i).buffid());
    }
  }

  return true;
}

// 装备被破坏后失效的buff
bool BufferStateList::delEquipBuffBreakInvalid(ItemBase* pItem, bool bWhenBreak/* = false*/)
{
  if (!pItem)
    return false;
  ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
  if(nullptr == pEquip)
    return false;
  if (m_pEntry == nullptr)
    return false;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  if (pItem->getCFG() == nullptr)
    return false;

  if (bWhenBreak == false && pEquip->isBroken())
    return true;

  const TVecDWORD& buffs = pItem->getCFG()->vecBuffIDs;
  for (auto v = buffs.begin(); v != buffs.end(); ++v)
  {
    del(*v);
  }

  // equip refine
  const std::vector<pair<DWORD, TVecDWORD>>& refine2buffs = pItem->getCFG()->vecRefine2Buff;
  for (auto &v : refine2buffs)
  {
    for (auto &b : v.second)
      del(b);
  }

  // del upgrade buff
  DWORD equiplv = pEquip->getLv();
  if (equiplv > 0)
  {
    for (DWORD lv = 1; lv <= equiplv; ++lv)
    {
      const SEquipUpgradeCFG* pUpgradeCFG = pItem->getCFG()->getUpgradeCFG(lv);
      if (pUpgradeCFG && pUpgradeCFG->dwBuffID)
      {
        del(pUpgradeCFG->dwBuffID);
      }
    }
  }

  // equip suit
  BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_EQUIP);
  if (pPack == nullptr)
    return false;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*> (pPack);
  if (pEquipPack == nullptr)
    return false;

  const TSetDWORD& suitset = ItemManager::getMe().getSuitIDs(pItem->getTypeID());
  if (!suitset.empty())
  {
    DWORD nowcnt = pEquipPack->getEquipedItemNum(pItem->getTypeID(), true);
    if (nowcnt == 0)
    {
      for (auto &d : suitset)
      {
        DWORD suitnum = pEquipPack->getEquipSuitNum(pUser, d);
        const SSuitCFG* pCFG = ItemManager::getMe().getSuitCFG(d);
        if (pCFG == nullptr)
          return true;

        TVecDWORD suitbuffs;

        for (DWORD i = suitnum + 1; i < 10; ++i)
          pCFG->getBuffs(suitnum + 1, suitbuffs);

        for (auto v = suitbuffs.begin(); v != suitbuffs.end(); ++v)
        {
          del(*v);
        }

        // 套装精炼buff
        delSuitRefineBuff(pUser, pCFG, pEquip, pEquipPack);
      }
    }
  }

  delEquipBuffPVP(pEquip);

  return true;
}

bool BufferStateList::delEquipBuffPVP(ItemEquip* pEquip)
{
  if (!m_pEntry || !pEquip || !pEquip->getCFG())
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (!pUser)
    return false;

  if (pUser->getScene() && (pUser->getScene()->isPVPScene() || pUser->getScene()->isGvg()))
  {
    const TVecDWORD& pvpbuffs = pEquip->getCFG()->vecPVPBuffIDs;
    for (auto v = pvpbuffs.begin(); v != pvpbuffs.end(); ++v)
    {
      del(*v);
    }
  }

  return true;
}

bool BufferStateList::addCardBuff(const SItemCFG* pItem)
{
  //equip & card 添加或删除 自身对应buff
  if (!pItem)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  if (pUser->getTransform().isMonster())
    return false;
  const TVecDWORD& buffIDs = pItem->vecBuffIDs;

  for (auto v = buffIDs.begin(); v != buffIDs.end(); ++v)
  {
    if (add(*v, m_pEntry, 0))
      XDBG << "[Buff-卡片], 添加卡片buff, 卡片id:" << pItem->dwTypeID << "buffid:" << *v << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
  }

  // equip suit
  BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_EQUIP);
  if (pPack == nullptr)
    return false;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*> (pPack);
  if (pEquipPack == nullptr)
    return false;

  const TSetDWORD& suitset = ItemManager::getMe().getSuitIDs(pItem->dwTypeID);
  if (!suitset.empty())
  {
    for (auto &d : suitset)
    {
      DWORD suitnum = pEquipPack->getCardSuitNum(pUser, d);
      const SSuitCFG* pCFG = ItemManager::getMe().getSuitCFG(d);
      if (pCFG == nullptr)
        return true;

      TVecDWORD suitbuffs;

      for (DWORD i = 1; i <= suitnum; ++i)
      {
        pCFG->getBuffs(i, suitbuffs);
      }

      for (auto v = suitbuffs.begin(); v != suitbuffs.end(); ++v)
      {
        if (add(*v, m_pEntry, 0))
          XDBG << "[Buff-卡片-套装], 添加卡片套装buff, buffid:" << *v << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
      }
    }
  }
  return true;
}

bool BufferStateList::delCardBuff(const SItemCFG* pItem)
{
  //equip & card 添加或删除 自身对应buff
  if (!pItem)
    return false;

  const TVecDWORD& buffIDs = pItem->vecBuffIDs;

  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return false;

  for (auto v = buffIDs.begin(); v != buffIDs.end(); ++v)
  {
    SBufferData* pData = getBuffData(*v);
    if (pData != nullptr)
    {
      if (pData->pBuff->isOverlay() == false && pUser->getPackage().hasEquipedCard(pItem->dwTypeID))
        continue;
    }

    del(*v);
  }
  // equip suit
  BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_EQUIP);
  if (pPack == nullptr)
    return false;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*> (pPack);
  if (pEquipPack == nullptr)
    return false;

  const TSetDWORD& suitset = ItemManager::getMe().getSuitIDs(pItem->dwTypeID);
  if (!suitset.empty())
  {
    for (auto &d : suitset)
    {
      DWORD suitnum = pEquipPack->getCardSuitNum(pUser, d);
      const SSuitCFG* pCFG = ItemManager::getMe().getSuitCFG(d);
      if (pCFG == nullptr)
        return true;

      TVecDWORD suitbuffs;

      for (DWORD i = suitnum + 1; i < 10; ++i)
        pCFG->getBuffs(i, suitbuffs);

      for (auto v = suitbuffs.begin(); v != suitbuffs.end(); ++v)
      {
        del(*v);
      }
    }
  }

  return true;
}

// 装备精炼等级变化
void BufferStateList::onEquipRefineChange(ItemBase *pItem, bool success)
{
  ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
  if (pEquip == nullptr)
    return;

  SceneUser *pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (pUser == nullptr)
    return;

  if (pUser->getTransform().isMonster())
    return;

  EquipPackage *pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return;

  // 道具未装备
  if (pEquipPack->getItem(pEquip->getGUID()) == nullptr)
    return;

  const TSetDWORD& suitset = ItemManager::getMe().getSuitIDs(pItem->getTypeID());
  if (!suitset.empty()) {
    for (auto &d : suitset) {
      const SSuitCFG* pCFG = ItemManager::getMe().getSuitCFG(d);
      if (pCFG == nullptr)
        return;

      if (success) {
        addSuitRefineBuff(pUser, pCFG, pEquip, pEquipPack);
      } else {
        delSuitRefineBuff(pUser, pCFG, pEquip, pEquipPack);
      }
    }
  }

  // equip enchant
  EnchantData& rData = pEquip->getEnchantData();
  const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(rData.type());
  if (pCFG != nullptr)
  {
    for (int i = 0; i < rData.extras_size(); ++i)
    {
      const SEnchantAttr* pAttr = pCFG->getEnchantAttr(rData.extras(i).configid());
      if (pAttr == nullptr || pAttr->vecExtraCondition.size() < 2)
        continue;

      if (pAttr->vecExtraCondition[0] == EENCHANTEXTRACON_REFINELV && pEquip->getRefineLv() >= pAttr->vecExtraCondition[1])
        add(rData.extras(i).buffid());
      else
        del(rData.extras(i).buffid());
    }
  }
}

bool BufferStateList::addSuitRefineBuff(SceneUser* pUser, const SSuitCFG* pCFG, ItemEquip* pEquip, EquipPackage* pEquipPack)
{
  if (!pUser || !pCFG || !pEquip || !pEquipPack)
    return false;

  // 不满足套装
  if (!pEquipPack->isSuitValid(pCFG->id))
    return false;

  TMapItemEquip suitItems;
  pEquipPack->getSuitEquipItems(pUser, pCFG->id, suitItems);
  if (!suitItems.empty()) {
    TVecDWORD buffs;

    // 套装全部装备精炼等级达到条件可加buff
    bool addSuitBuff = true;
    for (auto &v : suitItems) {
      if (v.second->getRefineLv() < pCFG->dwRefineLevel) {
        addSuitBuff = false;
        break;
      }

      // 装备精炼等级达到条件可加buff
      if (v.second->getRefineLv() >= pCFG->getEquipRefineLv(v.second->getTypeID())) {
        pCFG->getRefineEquipBuffs(v.second->getTypeID(), buffs);
      }
    }
    if (addSuitBuff) {
      pCFG->getRefineBuffs(buffs);
    }

    for (auto v : buffs) {
      if (!haveBuff(v) && add(v, m_pEntry, 0))
        XDBG << "[Buff-装备-套装], 添加装备套装精炼buff, buffid:" << v << "buff对象:" << m_pEntry->name << m_pEntry->id << XEND;
    }

    return true;
  }

  return false;
}

bool BufferStateList::delSuitRefineBuff(SceneUser* pUser, const SSuitCFG* pCFG, ItemEquip* pEquip, EquipPackage* pEquipPack)
{
  if (!pUser || !pCFG || !pEquip || !pEquipPack)
    return false;

  bool removeSuitBuff = false, removeEquipBuff = true, removeAllBuff = false;

  if (!pEquipPack->isSuitValid(pCFG->id))  // 不满足套装, 移除所有相关buff
    removeAllBuff = true;
  else {
    const TSetItemBase equips = pEquipPack->getItemBaseList(pEquip->getTypeID());
    for (auto item : equips) {
      ItemEquip* e = dynamic_cast<ItemEquip*>(item);
      if (e == nullptr)
        continue;
      if (e->getRefineLv() >= pCFG->getEquipRefineLv(pEquip->getTypeID())) {
        removeEquipBuff = false;
        break;
      }
    }

    TMapItemEquip suitItems;
    pEquipPack->getSuitEquipItems(pUser, pCFG->id, suitItems);
    if (!suitItems.empty() && suitItems.find(pEquip->getTypeID()) != suitItems.end()) {
      for (auto &v : suitItems) {
        if (v.second->getRefineLv() < pCFG->dwRefineLevel) {
          removeSuitBuff = true;
          break;
        }
      }
    } else {
      removeSuitBuff = true;
    }
  }

  TVecDWORD buffs;
  if (removeAllBuff)
    pCFG->getRefineAllBuffs(buffs);
  else {
    if (removeEquipBuff) {
      pCFG->getRefineEquipBuffs(pEquip->getTypeID(), buffs);
    }
    if (removeSuitBuff) {
      pCFG->getRefineBuffs(buffs);
    }
  }

  for (auto v : buffs) {
    del(v);
  }

  return true;
}

void BufferStateList::loadEquipBuff()
{
  if (m_pEntry == nullptr)
    return;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;

  TVecSortItem pVecItems;
  if (pUser->getPackage().getPackage(EPACKTYPE_EQUIP) == nullptr)
    return;
  pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->getEquipItems(pVecItems);
  if (pVecItems.empty())
    return;
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  for (auto v = pVecItems.begin(); v != pVecItems.end(); ++v)
  {
    //if (getIndex() != rCFG.getValidEquipPos(static_cast<EEquipPos>(getIndex()), getEquipType()))
        //return;
    ItemEquip* pEquip = dynamic_cast<ItemEquip*> (*v);
    if (!pEquip)
      continue;
    if (pEquip->getIndex() != rCFG.getValidEquipPos(static_cast<EEquipPos>(pEquip->getIndex()), pEquip->getEquipType()))
      continue;
    addEquipBuff(*v);
  }
}

void BufferStateList::loadSkillBuff()
{
  if (m_pEntry == nullptr)
    return;
  SceneUser *pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;

  TVecDWORD vecSkIDs;
  //if (pUser->getFighter() == nullptr)
  // return
  //pUser->getFighter()->getSkill().getCurSkills(vecSkIDs);

  if (pUser->getFighter(pUser->getUserSceneData().getProfession()) == nullptr)
    return;
  pUser->getFighter(pUser->getUserSceneData().getProfession())->getSkill().getCurSkills(vecSkIDs);
  for (auto v = vecSkIDs.begin(); v != vecSkIDs.end(); ++v)
  {
    addSkillBuff(*v);
  }
}

void BufferStateList::loadRuneBuff()
{
  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (user == nullptr || user->getFighter() == nullptr)
    return;
  TSetDWORD setRuneSpecIDs;
  user->getAstrolabes().getEffectIDs(setRuneSpecIDs);
  for (auto &s : setRuneSpecIDs)
  {
    const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(s);
    if (pRuneCFG == nullptr)
      continue;
    if (pRuneCFG->setBuffIDs.empty())
      continue;
    if (pRuneCFG->eType == ERUNESPECTYPE_SELECT && user->getFighter()->getSkill().isRuneSpecSelected(s) == false)
      continue;
    DWORD num = user->getAstrolabes().getEffectCnt(s);
    for (auto &d : pRuneCFG->setBuffIDs)
    {
      for (DWORD i = 0; i < num ; ++i)
        add(d);
    }
  }
}

void BufferStateList::loadManualBuff()
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (pUser == nullptr)
    return;

  static const set<EManualType> setTypes = {EMANUALTYPE_FASHION, EMANUALTYPE_CARD, EMANUALTYPE_EQUIP, EMANUALTYPE_ITEM, EMANUALTYPE_MOUNT, EMANUALTYPE_COLLECTION, EMANUALTYPE_MONSTER, EMANUALTYPE_NPC, EMANUALTYPE_PET,
    EMANUALTYPE_MAP, EMANUALTYPE_SCENERY};

  Manual& rManual = pUser->getManual();
  for (auto &s : setTypes)
  {
    SManualItem* pItem = rManual.getManualItem(s);
    if (pItem == nullptr)
      continue;
    for (auto &v : pItem->vecSubItems)
    {
      SManualSubItem& rSubItem = v;

      TVecDWORD vecAdvBuffIDs;
      TVecDWORD vecStoreBuffIDs;

      if (s == EMANUALTYPE_FASHION || s == EMANUALTYPE_CARD || s == EMANUALTYPE_EQUIP || s == EMANUALTYPE_ITEM || s == EMANUALTYPE_MOUNT || s == EMANUALTYPE_COLLECTION)
      {
        const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(rSubItem.dwID);
        if (pCFG != nullptr)
        {
          if (rSubItem.eStatus >= EMANUALSTATUS_UNLOCK_STEP)
            vecAdvBuffIDs.insert(vecAdvBuffIDs.end(), pCFG->vecAdvBuffIDs.begin(), pCFG->vecAdvBuffIDs.end());
          if (rSubItem.bStore)
            vecStoreBuffIDs.insert(vecStoreBuffIDs.end(), pCFG->vecStoreBuffIDs.begin(), pCFG->vecStoreBuffIDs.end());
        }
      }
      else if (s == EMANUALTYPE_MONSTER || s == EMANUALTYPE_NPC || s == EMANUALTYPE_PET)
      {
        const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(rSubItem.dwID);
        if (pCFG != nullptr && rSubItem.eStatus >= EMANUALSTATUS_UNLOCK_STEP)
          vecAdvBuffIDs.insert(vecAdvBuffIDs.end(), pCFG->vecAdvBuffIDs.begin(), pCFG->vecAdvBuffIDs.end());
      }
      else if (s == EMANUALTYPE_MAP)
      {
        const SMapCFG* pBase = MapConfig::getMe().getMapCFG(rSubItem.dwID);
        if (pBase != nullptr && rSubItem.eStatus >= EMANUALSTATUS_UNLOCK_STEP)
        {
          for (auto v = pBase->setManualBuffIDs.begin(); v != pBase->setManualBuffIDs.end(); ++v)
            vecAdvBuffIDs.push_back(*v);
        }
      }
      else if (s == EMANUALTYPE_SCENERY)
      {
        const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(rSubItem.dwID);
        if (pBase != nullptr && rSubItem.eStatus >= EMANUALSTATUS_UNLOCK_STEP)
        {
          TVecDWORD vecIDs;
          pBase->collectAdvBuffID(vecIDs);
          vecAdvBuffIDs.insert(vecAdvBuffIDs.end(), vecIDs.begin(), vecIDs.end());
        }
      }

      for (auto &adv : vecAdvBuffIDs)
      {
        add(adv);
        m_bNoBuffMsg = true;
      }
      for (auto &store : vecStoreBuffIDs)
      {
        add(store);
        m_bNoBuffMsg = true;
      }
    }
  }
}

void BufferStateList::loadConfigBuff()
{
  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();
  for (auto v = rCFG.vecBuffs.begin(); v != rCFG.vecBuffs.end(); ++v)
    add(*v, m_pEntry);
  DWORD profession = m_pEntry->getProfession();
  const TSetDWORD& classbuf = rCFG.getClassInitBuff(profession);
  for (auto &s : classbuf)
    add(s);
}

void BufferStateList::load(const Cmd::BlobBuffer &data)
{
  int size = data.list_size();
  if (!size) return;

  if (m_pEntry == nullptr)
    return;

  if (m_pEntry->getEntryType() == SCENE_ENTRY_USER)
  {
    m_mapID2BuffData.clear();
    m_mapID2StaticBuff.clear();
  }
  DWORD cur = xTime::getCurMSec();

  for (int i=0; i<size; ++i)
  {
    const Cmd::BufferStateBlob &bd = data.list(i);
    if (bd.endtime() != 0 && bd.endtime() < cur) continue;
    DWORD id = bd.id();
    TPtrBufferState buffPtr = BufferManager::getMe().getBuffById(id);
    if (buffPtr != nullptr)
    {
      if (buffPtr->m_bNotSave)
        continue;
      SBufferData buffData;
      buffData.id = id;
      buffData.endTime = bd.endtime();
      if (buffPtr->m_bOfflineKeep && bd.sparetime() != 0) // 下线不计时类buff
      {
        buffData.endTime = xTime::getCurMSec() + bd.sparetime();
      }
      buffData.lv = bd.lv();
      buffData.activeFlag = bd.actflag();
      buffData.fromID = bd.fromid();
      buffData.count = bd.count();
      buffData.hpOnAdd = bd.hponadd();
      buffData.addTime = bd.addtime();
      buffData.pBuff = buffPtr;
      buffData.me = m_pEntry;
      buffData.layers = (bd.layers() != 0 ? bd.layers() : 1);
      buffData.dwCommonData = bd.commmondata();
      buffData.dwAttackCount = bd.attackcount();
      buffData.dwBeAttackCount = bd.beatkcount();
      buffData.dwTotalDamage = bd.totaldamage();
      if (bd.has_fromname())
        buffData.strFromName = bd.fromname();
      for (int i = 0; i < bd.attrs_size(); ++i)
      {
        UserAttrSvr uAttr;
        uAttr.set_type(bd.attrs(i).type());
        uAttr.set_value(bd.attrs(i).value());
        buffData.m_oAttr.push_back(uAttr);
      }
      for (int i = 0; i < bd.vecdata_size(); ++i)
      {
        buffData.vecCommonData.push_back(bd.vecdata(i));
      }
      for (int i = 0; i< bd.setendtime_size(); ++i)
        buffData.queueEndTime.push(bd.setendtime(i));
      buffData.timeTick = bd.timetick();
      buffData.bSyncNine = buffPtr->needSyncNine();

      if (buffPtr->isStaticBuff())
        m_mapID2StaticBuff[id] = buffData;
      else
        m_mapID2BuffData[id] = buffData;

      addTypeBuff(buffPtr, id);

      //addUpdate(id) 不需要调用, 在AddMapUser/AddMapNpc中通知前端

      if (buffPtr->isPermanentBuff() && buffData.activeFlag)
      {
        m_setPmtBuffUpdates.insert(id);
      }
    }
    else
    {
      XLOG << "[Buff], 玩家:" << m_pEntry->name << m_pEntry->id << "buff id =" << id << "找不到对应的配置" << XEND;
    }
  }
}

// call after skill, equip etc. are initializeed
void BufferStateList::loadFromUser()
{
  if (m_pEntry == nullptr)
    return;
  loadConfigBuff();
  loadEquipBuff();
  loadSkillBuff();
  loadRuneBuff();
  loadManualBuff();

  // 立即添加
  m_bLoadingFromUser = true;    // 部分realAdd中需要执行的操作延后或者完全不执行
  refreshBuffAtonce();
  m_bLoadingFromUser = false;

  // buff全部添加完成后, 再执行quest的buff事件
  SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (pUser)
    pUser->getQuest().onBuff();

  DWORD cur = now();
  static const std::set<EBuffType> setNoRestartBuff = { EBUFFTYPE_MULTITIME };
  auto func = [&](SBufferData& r)
  {
    auto p = r.pBuff;
    if (p == nullptr)
      return;
    if (setNoRestartBuff.find(p->getBuffType()) != setNoRestartBuff.end())
      return;
    if (p->canStart(r, cur))
    {
      p->onStart(r, cur);
      setDataMark(p);
    }
  };
  foreach(func);

  m_pEntry->refreshDataAtonce();
}

void BufferStateList::save(Cmd::BlobBuffer *data)
{
  data->Clear();

  auto func = [&](SBufferData& r)
  {
    auto p = r.pBuff;
    if (p == nullptr)
      return;
    if (r.endTime == 0 && !p->isPermanentBuff() && !p->isDelByLayer())
      return;
    if (p->m_bNotSave)
      return;
    if (p->getBuffType() == EBUFFTYPE_FORCEATTR && r.endTime == 0)
      return;

    if (m_listUpdateData.empty() == false)
    {
      bool todel = false;
      for (auto &s : m_listUpdateData)
      {
        if (s.first == false && s.second.id == r.id)
        {
          todel = true;
          break;
        }
      }
      if (todel) return;
    }
    Cmd::BufferStateBlob *pData = data->add_list();
    pData->set_id(r.id);
    pData->set_endtime(r.endTime);
    pData->set_lv(r.lv);
    pData->set_actflag(r.activeFlag);
    pData->set_fromid(r.fromID);

    if (p->isDelByCount())
      pData->set_count(r.count);
    if (p->getInterval())
      pData->set_timetick(r.timeTick);

    pData->set_addtime(r.addTime);
    pData->set_hponadd(r.hpOnAdd);
    pData->set_commmondata(r.dwCommonData);
    pData->set_layers(r.layers);
    pData->set_totaldamage(r.dwTotalDamage);
    pData->set_attackcount(r.dwAttackCount);
    pData->set_beatkcount(r.dwBeAttackCount);
    if (p->m_bOfflineKeep)
    {
      QWORD curm = xTime::getCurMSec();
      if (r.endTime > curm)
        pData->set_sparetime(r.endTime - curm);
    }
    if (r.strFromName.empty() == false)
      pData->set_fromname(r.strFromName);
    // 来自他人的buff， 存储属性
    if (r.fromID != m_pEntry->id)
    {
      for (DWORD i = 0; i < r.m_oAttr.size(); ++i)
      {
        UserAttrSvr* pAttr = pData->add_attrs();
        pAttr->set_type(r.m_oAttr[i].type());
        pAttr->set_value(r.m_oAttr[i].value());
      }
    }
    if (p->m_bLayerDiffTime)
    {
      std::queue<QWORD> que(r.queueEndTime);
      while(!que.empty())
      {
        pData->add_setendtime(que.front());
        que.pop();
      }
    }
    for (auto &v : r.vecCommonData)
    {
      pData->add_vecdata(v);
    }
  };
  foreach(func);

  SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (pUser != nullptr)
    XDBG << "[Buff-保存]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "数据大小:" << data->ByteSize() << XEND;
}

void BufferStateList::clear(const TSetDWORD& excepts/* = TSetDWORD{}*/)
{
  auto func = [&](SBufferData& r)
  {
    auto p = r.pBuff;
    if (p == nullptr)
      return;
    if (r.canRemove() && !p->m_bDeathKeep && excepts.find(r.id) == excepts.end())
    {
      del(r.id, true);
    }
  };
  foreach(func);
}

bool BufferStateList::delStatus(const TSetDWORD& setStatus)
{
  bool deled = false;
  auto func = [&](SBufferData& r)
  {
    auto p = r.pBuff;
    if (p == nullptr)
      return;

    if (r.isStatusNoDisp()) // 表示此buff不可被驱散
      return;

    DWORD s = p->getStatus();
    if (s && setStatus.find(s) != setStatus.end())
    {
      del(r.id, true);
      deled = true;
    }
  };
  foreach(func);
  return deled;
}

bool BufferStateList::delStatus(DWORD statusID)
{
  bool deled = false;
  auto func = [&](SBufferData& r)
  {
    auto p = r.pBuff;
    if (p == nullptr)
      return;

    if (r.isStatusNoDisp()) // 表示此buff不可被驱散
      return;

    if (statusID == 0 && p->getStatus() != 0)
    {
      del(r.id, true);
      deled = true;
    }
    else if (statusID != 0 && p->getStatus() == statusID)
    {
      del(r.id, true);
      deled = true;
    }
  };
  foreach(func);
  return deled;
}

void BufferStateList::onUseSkill(const BaseSkill* pSkill)
{
  auto it = m_mapTrigType2Set.find(BUFFTRIGGER_USESKILL);
  if (it != m_mapTrigType2Set.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      auto p = pData->pBuff;
      if (p == nullptr)
        continue;
      UseSkillCondition* pCond = dynamic_cast<UseSkillCondition*> (p->buffCondition.get());
      if (pCond == nullptr)
        continue;
      if (pCond->isCheckOk(pSkill, m_pEntry))
      {
        pData->setCondSkillID(pSkill->getSkillID());
        p->doBuffEffect(m_pEntry, p->getBuffTargets(*pData, m_pEntry, nullptr), *pData);
        pData->setCondSkillID(0);
      }
    }
  }
}

void BufferStateList::onChantStatusChange()
{
  checkBuffEnable(BUFFTRIGGER_CHANT);
}

void BufferStateList::enableBuff(DWORD id)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr || pData->activeFlag)
    return;
  pData->activeFlag = true;
  setDataMark(pData->pBuff);
}

void BufferStateList::disableBuff(DWORD id)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr || pData->activeFlag == false)
    return;
  pData->activeFlag = false;
  setDataMark(pData->pBuff);
}

void BufferStateList::getStatus(set<DWORD>& stSet)
{
  auto func = [&](SBufferData& r)
  {
    if (r.activeFlag == false)
      return;
    DWORD status = r.pBuff->getStatus();
    if (status)
      stSet.insert(status);
  };
  foreach(func);
}

void BufferStateList::enableType(EBuffType eType)
{
  auto it = m_mapType2SetBuff.find(eType);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData != nullptr)
        pData->activeFlag = true;
    }
  }
}

void BufferStateList::disableType(EBuffType eType)
{
  auto it = m_mapType2SetBuff.find(eType);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData != nullptr)
        pData->activeFlag = false;
    }
  }
}

DWORD BufferStateList::getStatusAttr()
{
  set<DWORD> stSet;
  getStatus(stSet);
  if (stSet.empty())
    return 0;
  auto pow = [](DWORD num) -> DWORD
  {
    DWORD result = 1;
    for (DWORD i = 1; i < num; ++i)
      result *= 2;
    return result;
  };
  DWORD status = 0;
  for (auto &d : stSet)
  {
    status += pow(d);
  }
  return status;
}

void BufferStateList::addUpdate(const SBufferData& upData)
{
  auto it = find_if(m_vecAddUpdates.begin(), m_vecAddUpdates.end(), [&](const SBufferData& r) -> bool{
      return upData.id == r.id;
      });
  if (it != m_vecAddUpdates.end())
    m_vecAddUpdates.erase(it);

  m_vecAddUpdates.push_back(upData);
}

void BufferStateList::delSkillEquipBuff()
{
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;
  TVecDWORD vecSkIDs;

  SceneFighter* pFiter = pUser->getFighter();
  if (pFiter == nullptr)
    return;
  pFiter->getSkill().getCurSkills(vecSkIDs);
  for (auto v = vecSkIDs.begin(); v != vecSkIDs.end(); ++v)
  {
    delSkillBuff(*v);
  }

  // 职业变化, 装备卡片 有效性变化
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return;

  auto func = [&](ItemBase* pBase)
  {
    const ItemEquip* pEquip = dynamic_cast<const ItemEquip*>(pBase);
    if (pEquip == nullptr)
      return;
    delEquipBuff(pBase);
  };
  pEquipPack->foreach(func);
}

void BufferStateList::addSkillEquipBuff()
{
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;
  if (pUser->getTransform().isMonster())
    return;

  TVecDWORD vecSkIDs;

  SceneFighter* pFiter = pUser->getFighter();
  if (pFiter == nullptr)
    return;
  pFiter->getSkill().getCurSkills(vecSkIDs);
  for (auto v = vecSkIDs.begin(); v != vecSkIDs.end(); ++v)
  {
    addSkillBuff(*v);
  }

  // 职业变化, 装备卡片 有效性变化
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return;

  auto func = [&](ItemBase* pBase)
  {
    const ItemEquip* pEquip = dynamic_cast<const ItemEquip*>(pBase);
    if (pEquip == nullptr)
      return;
    onEquipChange(pBase, EEQUIPOPER_ON);
  };
  pEquipPack->foreach(func);
}

bool BufferStateList::hasPartTransform(EUserDataType eType)
{
  if (!m_bPartTransform) return false;
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_PARTTRANSFORM);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffPartTransform* pBuff = dynamic_cast<BuffPartTransform*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      if (pBuff->has(eType))
        return true;
    }
  }
  return false;
}

DWORD BufferStateList::getPartTransform(EUserDataType eType)
{
  if (!m_bPartTransform) return false;
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_PARTTRANSFORM);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffPartTransform* pBuff = dynamic_cast<BuffPartTransform*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      if (pBuff->has(eType))
        return pBuff->get(eType, m_pEntry);
    }
  }
  return 0;
}

bool BufferStateList::isLayerEnough(EBuffType eType)
{
  auto it = m_mapType2SetBuff.find(eType);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData)
        return pData->layers >= pData->getLimitLayer();
    }
  }
  return false;
}

void BufferStateList::getAllSkillSpec(SSpecSkillInfo& info)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_AFFACTSKILL);
  if (it == m_mapType2SetBuff.end())
    return;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr || pData->activeFlag == false)
      continue;
    BuffAffactSkill* pASBuff = dynamic_cast<BuffAffactSkill*> (pData->pBuff.get());
    if (pASBuff == nullptr)
      continue;
    if (!pASBuff->isAllSkill())
      continue;
    pASBuff->getSpecSkillInfo(info, pData->layers);
  }
}

void BufferStateList::addBuffDamage(float damage)
{
  if (damage <= 0)
    return;

  for (auto &it : m_mapID2BuffData)
  {
    if (!it.second.activeFlag)
      continue;
    auto p = it.second.pBuff;
    if (p == nullptr)
      continue;

    if (p->m_fDamagePer != 0 || !p->m_FmDamagerPer.empty())
      it.second.dwTotalDamage += damage;
  }
}

void BufferStateList::setDataMark(TPtrBufferState buffPtr)
{
  if (!buffPtr)
    return;

  if (buffPtr->needRefreshAllBuff())
  {
    m_pEntry->setCollectMarkAllBuff();
    m_pEntry->refreshDataAtonce();
  }
  else if (buffPtr->hasAttr())
  {
    if (buffPtr->isStaticBuff())
      m_pEntry->setCollectMark(ECOLLECTTYPE_STATIC_BUFF);
    else
      m_pEntry->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
    m_pEntry->refreshDataAtonce();
  }

  if (buffPtr->isPermanentBuff())
    m_setPmtBuffUpdates.insert(buffPtr->getID());
}

DWORD BufferStateList::getShaderColor()
{
  DWORD result = 0;
  DWORD weight = 0;
  auto func = [&](SBufferData& r)
  {
    if (r.activeFlag == false)
      return;
    auto p = r.pBuff;
    if (p == nullptr || p->getShaderColor() == 0)
      return;
    const SShaderColor* pCFG = TableManager::getMe().getShaderColorCFG(p->getShaderColor());
    if (pCFG == nullptr)
      return;
    if (pCFG->dwWeight >= weight)
    {
      result = pCFG->dwColorID;
      weight = pCFG->dwWeight;
    }
  };
  foreach(func);
  return result;
}


void BufferStateList::reloadAllBuff()
{
  if (!m_pEntry) return;

  auto loadMapBuff = [&](std::map<DWORD, SBufferData>& mapBuffs)
  {
    for (auto m = mapBuffs.begin(); m != mapBuffs.end();)
    {
      TPtrBufferState buffPtr = BufferManager::getMe().getBuffById(m->first);
      if (buffPtr == nullptr)
      {
        addUpdate(m->second);
        XLOG << "[Buff-Reload], buff重新加载配置, 删除无效buff" << m_pEntry->name << m_pEntry->id << "buffid:" << m->first << XEND;

        //delTypeBuff(buffPtr->getBuffType(), m->first);
        for (auto it = m_mapType2SetBuff.begin(); it != m_mapType2SetBuff.end(); )
        {
          if (it->second.find(m->first) != it->second.end())
          {
            it->second.erase(m->first);
            if (it->second.empty())
            {
              it = m_mapType2SetBuff.erase(it);
              continue;
            }
          }
          ++it;
        }

        m = mapBuffs.erase(m);
        continue;
      }
      else
      {
        m->second.pBuff = buffPtr;
        setDataMark(buffPtr);
        ++m;
      }
    }
  };
  loadMapBuff(m_mapID2BuffData);
  loadMapBuff(m_mapID2StaticBuff);
}

void BufferStateList::getForceAttr(TVecAttrSvrs& attrs)
{
  auto m = m_mapType2SetBuff.find(EBUFFTYPE_FORCEATTR);
  if (m != m_mapType2SetBuff.end())
  {
    for (auto &s : m->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      const TVecAttrSvrs& tempattrs = pData->getAttr();
      for (auto &v : tempattrs)
      {
        auto it = find_if(attrs.begin(), attrs.end(), [&](const UserAttrSvr& r) -> bool{
            return r.type() == v.type();
        });
        if (it != attrs.end())
        {
          it->set_value(it->value() + v.value());
          continue;
        }
        attrs.push_back(v);
      }
    }
  }
}

void BufferStateList::setForceAttr(bool flag)
{
  if (m_bHaveForceAttr == flag)
    return;
  m_bHaveForceAttr = flag;
  m_pEntry->setCollectMark(ECOLLECTTYPE_NONE);
  m_pEntry->refreshDataAtonce();
}

void BufferStateList::onLeaveScene()
{
  del(120244);

  delBuffByType(EBUFFTYPE_FORCEATTR);

  if (m_pEntry->getEntryType() == SCENE_ENTRY_USER)
    update(xTime::getCurMSec());
}

void BufferStateList::onClientMove()
{
  SceneUser* pUser = dynamic_cast<SceneUser*> (m_pEntry);
  if (pUser == nullptr)
    return;
  if (pUser->getTransform().isInTransform())
  {
    for (auto &m : m_mapID2BuffData)
    {
      auto p = m.second.pBuff;
      if (p == nullptr || p->getBuffType() != EBUFFTYPE_TRANSFORM)
        continue;
      BuffTransform* pTransBuff = dynamic_cast<BuffTransform*> (p.get());
      if (pTransBuff == nullptr)
        continue;
      if (pTransBuff->isMoveDel())
        del(m.first);
      break;
    }
  }

  auto hidebuff = m_mapType2SetBuff.find(EBUFFTYPE_HIDE);
  if (hidebuff != m_mapType2SetBuff.end())
  {
    for (auto &s : hidebuff->second)
    {
      auto m = m_mapID2BuffData.find(s);
      if (m == m_mapID2BuffData.end())
        continue;
      BuffHiding* pBuff = dynamic_cast<BuffHiding*>(m->second.pBuff.get());
      if (pBuff)
        pBuff->onClientMove(m->second);
    }
  }
}

void BufferStateList::onEnterScene()
{
  SceneNpc* pNpc = dynamic_cast<SceneNpc*> (m_pEntry);
  if (pNpc && pNpc->getCFG())
  {
    m_mapID2BuffData.clear();
    m_mapID2StaticBuff.clear();
    pNpc->m_ai.initAddBuff();
    for (auto &v : pNpc->getCFG()->vecBuffs)
    {
      add(v);
    }
    DWORD dwBodyID = SceneNpcManager::getMe().getChangeBodyID(pNpc->getNpcID());
    if(dwBodyID != 0)
      add(dwBodyID);
    m_pEntry->setCollectMarkAllBuff();
    m_pEntry->refreshDataAtonce();

    const DWORD dwRatioBuff = pNpc->getMapMonsterRatioBuff(pNpc->getMapID());
    if(dwRatioBuff != 0)
      pNpc->getBuff().add(dwRatioBuff);
  }

  checkBuffEnable(BUFFTRIGGER_MAPTYPE);
  m_pEntry->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  m_pEntry->refreshDataAtonce();
}

void BufferStateList::onBeShoot()
{
  auto it = m_mapTrigType2Set.find(BUFFTRIGGER_BESHOOT);
  if (it != m_mapTrigType2Set.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->pBuff == nullptr)
        continue;
      auto p = pData->pBuff;
      p->doBuffEffect(m_pEntry, p->getBuffTargets(*pData, m_pEntry), *pData);
    }
  }
}

DWORD BufferStateList::getLayerByID(DWORD id)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr)
    return 0;
  return pData->layers;
}

void BufferStateList::addTypeBuff(TPtrBufferState buffPtr, DWORD id)
{
  if (buffPtr == nullptr)
    return;
  auto it = m_mapType2SetBuff.find(buffPtr->getBuffType());
  if (it != m_mapType2SetBuff.end())
    it->second.insert(id);
  else
    m_mapType2SetBuff[buffPtr->getBuffType()].insert(id);

  auto t = m_mapTrigType2Set.find(buffPtr->getTrigType());
  if (t != m_mapTrigType2Set.end())
    t->second.insert(id);
  else
    m_mapTrigType2Set[buffPtr->getTrigType()].insert(id);

}

void BufferStateList::delTypeBuff(TPtrBufferState buffPtr, DWORD id)
{
  if (buffPtr == nullptr)
    return;
  auto it = m_mapType2SetBuff.find(buffPtr->getBuffType());
  if (it != m_mapType2SetBuff.end())
  {
    it->second.erase(id);
    if (it->second.empty())
      m_mapType2SetBuff.erase(it);
  }
  auto t = m_mapTrigType2Set.find(buffPtr->getTrigType());
  if (t != m_mapTrigType2Set.end())
  {
    t->second.erase(id);
    if (t->second.empty())
      m_mapTrigType2Set.erase(t);
  }
}

void BufferStateList::onGetMonsterExp(SceneNpc* npc, DWORD baseExp, DWORD jobExp)
{
  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (!user || !user->getUserSceneData().haveEnoughBattleTime())
    return;

  auto it = m_mapType2SetBuff.find(EBUFFTYPE_MULTITIME);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffMultiTime* pBuff = dynamic_cast<BuffMultiTime*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      pBuff->onGetExp(*pData, npc, baseExp, jobExp);
    }
  }
  auto it2 = m_mapType2SetBuff.find(EBUFFTYPE_MULTIEXP);
  if (it2 != m_mapType2SetBuff.end())
  {
    for (auto &s : it2->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffMultiExp* pBuff = dynamic_cast<BuffMultiExp*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      pBuff->onGetExp(*pData, npc, baseExp, jobExp);
    }
  }
}

void BufferStateList::onPickUpItem(ItemInfo& oItem)
{
  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (!user || !user->getUserSceneData().haveEnoughBattleTime())
    return;

  auto it = m_mapType2SetBuff.find(EBUFFTYPE_MULTITIME);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffMultiTime* pBuff = dynamic_cast<BuffMultiTime*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      pBuff->onPickUpItem(*pData, oItem);
    }
  }

  auto ited = m_mapType2SetBuff.find(EBUFFTYPE_EXTRADROP);
  if (ited != m_mapType2SetBuff.end())
  {
    for (auto &s : ited->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffExtraDrop* pBuff = dynamic_cast<BuffExtraDrop*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      pBuff->onPickUpItem(*pData, oItem);
    }
  }

  //美食家的餐刀
  {
    auto ited = m_mapType2SetBuff.find(EBUFFTYPE_COOKER_KINFE);
    if (ited != m_mapType2SetBuff.end())
    {
      for (auto &s : ited->second)
      {
        SBufferData* pData = getBuffData(s);
        if (pData == nullptr || pData->activeFlag == false)
          continue;
        BuffCookerKnife* pBuff = dynamic_cast<BuffCookerKnife*> (pData->pBuff.get());
        if (pBuff == nullptr)
          continue;
        pBuff->onPickUpItem(*pData, oItem);
      }
    }
  }
}

void BufferStateList::onAddBattleTime(DWORD time)
{
  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (!user)
    return;

  auto it = m_mapType2SetBuff.find(EBUFFTYPE_MULTITIME);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffMultiTime* pBuff = dynamic_cast<BuffMultiTime*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      DWORD oldlayers = pData->layers;
      pBuff->onAddBattleTime(*pData, time);
      if (oldlayers != pData->layers)
        addUpdate(*pData);
    }
  }

  auto itrr = m_mapType2SetBuff.find(EBUFFTYPE_ROBREWARD);
  if (itrr != m_mapType2SetBuff.end()) {
    for (auto &s : itrr->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffRobReward* pBuff = dynamic_cast<BuffRobReward*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      DWORD oldlayers = pData->layers;
      pBuff->onAddBattleTime(*pData, time);
      if (oldlayers != pData->layers)
        addUpdate(*pData);
    }
  }
}

void BufferStateList::onBattleStatusChange()
{
  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (!user)
    return;

  bool bNormal = user->getUserSceneData().haveEnoughBattleTime();
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_MULTITIME);
  if (it == m_mapType2SetBuff.end())
    return;

  DWORD priorID = 0;
  if (bNormal && it->second.size() > 1)
  {
    DWORD earlytime = 0;
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      // 优先 使后添加的 生效
      if (pData->addTime > earlytime)
      {
        earlytime = pData->addTime;
        priorID = s;
      }
    }
  }

  for (auto &s : it->second)
  {
    if (priorID && s != priorID)
      continue;
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    if (pData->activeFlag != bNormal)
    {
      pData->activeFlag = bNormal;
      addUpdate(*pData);
      DWORD leftTime = (pData->layers + 59) / 60;
      if (leftTime != 0 && !bNormal)
        MsgManager::sendMsg(user->id, 518, MsgParams(leftTime));
    }
  }
}

float BufferStateList::getBodyScalePer()
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_CHANGESCALE);
  if (it == m_mapType2SetBuff.end())
    return 0;
  float per = 1;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr || pData->activeFlag == false)
      continue;
    BuffChangeScale* pBuff = dynamic_cast<BuffChangeScale*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    if (pBuff->getChangePer())
    {
      per *= pow(pBuff->getChangePer(), pData->layers);
      continue;
    }
    if (pBuff->getAddPer())
    {
      float myper = pBuff->getAddPer() * pData->layers + 1;
      myper = myper < 0.1 ? 0.1 : myper;
      per *= myper;
    }
  }
  return per;
}

void BufferStateList::onTeamChange()
{
  checkBuffEnable(BUFFTRIGGER_TEAMNUM);
}

DWORD BufferStateList::getPackageSlot(EPackType eType)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_PACKAGESLOT);
  if (it == m_mapType2SetBuff.end())
    return 0;
  DWORD num = 0;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffPackageSlot* pSlotBuff = dynamic_cast<BuffPackageSlot*> (pData->pBuff.get());
    if (pSlotBuff == nullptr)
      continue;
    num += pSlotBuff->getSlotNum(eType) * pData->layers;
  }

  return num;
}

void BufferStateList::getSpecSkillInfo(DWORD familySkillID, SSpecSkillInfo& info)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_AFFACTSKILL);
  if (it == m_mapType2SetBuff.end())
    return;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr || pData->activeFlag == false)
      continue;
    BuffAffactSkill* pASBuff = dynamic_cast<BuffAffactSkill*> (pData->pBuff.get());
    if (pASBuff == nullptr)
      continue;
    if (pASBuff->haveSkill(pData->me, familySkillID) == false)
      continue;
    pASBuff->getSpecSkillInfo(info, pData->layers);
  }
}

void BufferStateList::onHandChange()
{
  checkBuffEnable(BUFFTRIGGER_HAND);
}

SDWORD BufferStateList::onDropBaseExp(ESource source)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_DROPEXP);
  if (it == m_mapType2SetBuff.end())
    return 0;

  SDWORD rate = 0;

  for (auto &s : it->second) {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;

    BuffDropExp* pBuff = dynamic_cast<BuffDropExp*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;

    rate += pBuff->onDropBaseExp(source);
  }

  return rate;
}

void BufferStateList::onBeingChange()
{
  checkBuffEnable(BUFFTRIGGER_NPCPRESENT);
}

void BufferStateList::onChangeMap()
{
  checkBuffEnable(BUFFTRIGGER_MAP);
}

void BufferStateList::onChangeElement()
{
  checkBuffEnable(BUFFTRIGGER_ELEMENTELF);
}

DWORD BufferStateList::getMultiTimeRate()
{
  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (!user)
    return 0;

  auto it = m_mapType2SetBuff.find(EBUFFTYPE_MULTITIME);
  if (it != m_mapType2SetBuff.end()) {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffMultiTime* pBuff = dynamic_cast<BuffMultiTime*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      return pBuff->getRate();
    }
  }

  return 0;
}

DWORD BufferStateList::getRobRewardRate()
{
  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (!user)
    return 0;

  auto it = m_mapType2SetBuff.find(EBUFFTYPE_ROBREWARD);
  if (it != m_mapType2SetBuff.end()) {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr || pData->activeFlag == false)
        continue;
      BuffRobReward* pBuff = dynamic_cast<BuffRobReward*> (pData->pBuff.get());
      if (pBuff == nullptr)
        continue;
      return pBuff->getRate();
    }
  }

  return 0;
}

// Lua 调用, get buff list
void BufferStateList::getBuffListByName(const char* name, SLuaNumberArray& result)
{
  if (name == nullptr)
    return;

  TSetDWORD ids;
  auto func = [&](SBufferData& r)
  {
    auto p = r.pBuff;
    if (p == nullptr)
      return;
    const string& myname = p->getBuffTypeName();
    if (myname.empty())
      return;
    if (strcmp(myname.c_str(), name) == 0)
      ids.insert(r.id);
  };
  foreach(func);
  result.setDWORD(ids);
}

// lua 调用, 获取指定buff来源
QWORD BufferStateList::getBuffFromID(DWORD buffid)
{
  SBufferData* pData = getBuffData(buffid);
  if (pData == nullptr)
    return 0;
  return pData->fromID;
}

void BufferStateList::changeBuffSpeed(DWORD buffid, int speedMs)
{
  SBufferData* pData = getBuffData(buffid);
  if (pData == nullptr)
    return;
  pData->timeTick -= speedMs;
}

void BufferStateList::delBuffByGain(bool bGain, DWORD num)
{
  TVecDWORD vecSelect;
  auto func = [&](SBufferData& r)
  {
    if (r.pBuff && r.pBuff->m_bCanDisperse && r.pBuff->m_bGainBuff == bGain)
      vecSelect.push_back(r.id);
  };
  foreach(func);
  if (vecSelect.size() > num)
  {
    std::random_shuffle(vecSelect.begin(), vecSelect.end());
    vecSelect.resize(num);
  }
  for (auto &v : vecSelect)
    del(v, true);
}

bool BufferStateList::haveLimitAttr() const
{
  return m_mapType2SetBuff.find(EBUFFTYPE_ATTRLIMIT) != m_mapType2SetBuff.end();
}

void BufferStateList::getLimitAttrs(map<EAttrType, float>& minattrs, map<EAttrType, float>& maxattrs)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_ATTRLIMIT);
  if (it == m_mapType2SetBuff.end())
    return;
  minattrs.clear();
  maxattrs.clear();
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr || pData->activeFlag == false)
      continue;
    BuffAttrLimit* p = dynamic_cast<BuffAttrLimit*> (pData->pBuff.get());
    if (p == nullptr)
      continue;
    const map<EAttrType, float>& maxvalue = p->getLimitMaxAttrs();
    for (auto &m1 : maxvalue)
    {
      auto it1 = maxattrs.find(m1.first);
      if (it1 != maxattrs.end() && m1.second < it1->second) // 上限, 取小的
        it1->second = m1.second;
      else
        maxattrs[m1.first] = m1.second;
    }
    const map<EAttrType, float>& minvalue = p->getLimitMinAttrs();
    for (auto &m2 : minvalue)
    {
      auto it2 = minattrs.find(m2.first);
      if (it2 != minattrs.end() && m2.second > it2->second) // 下限, 取大的
        it2->second = m2.second;
      else
        minattrs[m2.first] = m2.second;
    }
  }
}

DWORD BufferStateList::getAutoBlockStiff()
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_AUTOBLOCK);
  if (it == m_mapType2SetBuff.end())
    return 0;
  for (auto& v : it->second)
  {
    SBufferData* pData = getBuffData(v);
    if (pData == nullptr)
      continue;
    BuffAutoBlock* pBuff = dynamic_cast<BuffAutoBlock*>(pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    return pBuff->getStiff();
  }
  return 0;
}

DWORD BufferStateList::getWeaponBlockStiff()
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_WEAPONBLOCK);
  if (it == m_mapType2SetBuff.end())
    return 0;
  for (auto& v : it->second)
  {
    SBufferData* pData = getBuffData(v);
    if (pData == nullptr)
      continue;
    BuffWeaponBlock* pBuff = dynamic_cast<BuffWeaponBlock*>(pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    return pBuff->getStiff();
  }
  return 0;
}

void BufferStateList::updateOneSecond(QWORD curm)
{
  DWORD cur = curm / ONE_THOUSAND;
  auto it = m_mapTrigType2Set.find(BUFFTRIGGER_DISTANCE);
  if (it != m_mapTrigType2Set.end())
  {
    for (auto& id : it->second)
    {
      SBufferData* pData = getBuffData(id);
      if (pData == nullptr)
        continue;
      auto p = pData->pBuff;
      if (p == nullptr || p->buffCondition == nullptr)
        continue;
      if (p->buffCondition->checkCondition(*pData, cur) == false)
        del(id);
    }
  }
  // 时间触发开始的buff
  auto itset = m_mapTrigType2Set.find(BUFFTRIGGER_TIME);
  if (itset != m_mapTrigType2Set.end())
  {
    for (auto &s : itset->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      if (pData->activeFlag)
        continue;
      auto p = pData->pBuff;
      if (p->canStart(*pData, cur))
        p->onStart(*pData, cur);
    }
  }

  updateBuffStatus(cur);
}

DWORD BufferStateList::getEndTimeByID(DWORD id)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr)
    return 0;
  QWORD cur = xTime::getCurMSec();
  return pData->endTime > cur ? (pData->endTime - cur) : 0;
}

QWORD BufferStateList::getBuffDelTime(DWORD id)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr)
    return 0;
  return pData->endTime;
}

bool BufferStateList::haveLimitSkill() const
{
  return m_mapType2SetBuff.find(EBUFFTYPE_LIMITSKILL) != m_mapType2SetBuff.end();
}

/*
const TSetDWORD& BufferStateList::getLimitSkill()
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_LIMITSKILL);
  if (it == m_mapType2SetBuff.end())
  {
    static const TSetDWORD emptyset;
    return emptyset;
  }

  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffLimitSkill* pLimit = dynamic_cast<BuffLimitSkill*> (pData->pBuff.get());
    if (pLimit == nullptr)
      continue;
    return pLimit->getLimitSkill();
  }
  static const TSetDWORD emptyset;
  return emptyset;
}
*/

bool BufferStateList::isSkillNotLimited(DWORD skillid)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_LIMITSKILL);
  if (it == m_mapType2SetBuff.end())
  {
    return true;
  }

  bool checknotuse = false, checkuse = false;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffLimitSkill* pLimit = dynamic_cast<BuffLimitSkill*> (pData->pBuff.get());
    if (pLimit == nullptr)
      continue;
    // 限制可以使用的一些技能
    if(pLimit->getLimitSkill().find(skillid) != pLimit->getLimitSkill().end())
      return true;
    // 限制不能使用的一些技能
    if (pLimit->getLimitNotSkill().empty() == false)
      checknotuse = true;
    else
      checkuse = true;
  }

  // 存在可使用技能时, 不检测不可使用技能
  if (checkuse)
    return false;

  if (checknotuse)
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      BuffLimitSkill* pLimit = dynamic_cast<BuffLimitSkill*> (pData->pBuff.get());
      if (pLimit == nullptr)
        continue;
      if(pLimit->getLimitNotSkill().find(skillid) != pLimit->getLimitNotSkill().end())
        return false;
    }
    return true;
  }

  return false;
}

QWORD BufferStateList::getLimitSkillTarget(DWORD skillId)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_LIMITSKILL);
  if (it == m_mapType2SetBuff.end())
    return 0;

  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffLimitSkill* pLimit = dynamic_cast<BuffLimitSkill*> (pData->pBuff.get());
    if (pLimit == nullptr)
      continue;
    if (pLimit->getLimitNotSkill().empty() == false)
      continue;
    auto& setSkillIDs = pLimit->getLimitSkill();
    bool bFound = setSkillIDs.find(skillId  / ONE_THOUSAND ) != setSkillIDs.end();
    if (bFound && pLimit->m_dwIgnoreTarget)
      continue;
    return pData->fromID;
  }
  return 0;
}

bool BufferStateList::canUseItem(DWORD itemtype)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_LIMITUSEITEM);
  if (it == m_mapType2SetBuff.end())
    return true;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffLimitUseItem* pLimit = dynamic_cast<BuffLimitUseItem*> (pData->pBuff.get());
    if (pLimit == nullptr)
      continue;

    if (pLimit->canUseItem(itemtype) == false)
      return false;
  }

  return true;
}

void BufferStateList::updateDisable(DWORD cur)
{
  if (m_setUpdateActive.empty())
    return;
  m_setDisables.clear();
  auto itset = m_mapType2SetBuff.find(EBUFFTYPE_DISABLE);
  if (itset != m_mapType2SetBuff.end())
  {
    for (auto &s : itset->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      BuffDisable* pDis = dynamic_cast<BuffDisable*> (pData->pBuff.get());
      if (pDis)
      {
        const TSetDWORD& ids = pDis->getDisIDs();
        m_setDisables.insert(ids.begin(), ids.end());
      }
    }
  }

  for (auto &s : m_setUpdateActive)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    if (pData->activeFlag)
    {
      if (m_setDisables.find(s) != m_setDisables.end())
      {
        pData->activeFlag = false;
        setDataMark(pData->pBuff);
      }
    }
    else if (pData->pBuff->canStart(*pData, cur))
    {
      if (m_setDisables.find(s) == m_setDisables.end())
      {
        pData->activeFlag = true;
        setDataMark(pData->pBuff);
      }
    }
  }

  m_setUpdateActive.clear();
}
// 免疫一次致死攻击
void BufferStateList::onApproachDie()
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_UNDEAD);
  if (it == m_mapType2SetBuff.end())
    return;
  QWORD curm = xTime::getCurMSec();
  DWORD cur = curm / ONE_THOUSAND;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    if (cur < pData->dwCommonData) // 避免被群攻时 一直计数
      break;
    BuffUndead* pBuff = dynamic_cast<BuffUndead*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    DWORD buff = pBuff->getProtectBuff();
    if (buff)
      add(buff);

    pData->dwCommonData = cur + 1; // 避免被群攻时 一直计数
    pData->count ++;
    pData->timeTick = curm + pBuff->getInterval(); // add cd
    break;
  }
}

bool BufferStateList::isUndead()
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_UNDEAD);
  if (it == m_mapType2SetBuff.end())
    return false;

  QWORD curm = xTime::getCurMSec();
  DWORD cur = curm / ONE_THOUSAND;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;

    // 被群攻时
    if (cur < pData->dwCommonData)
      return true;

    // check CD
    if (curm < pData->timeTick)
      return false;
    return true;
  }
  return false;
}

DWORD BufferStateList::getLevelByID(DWORD id)
{
  SBufferData* pData = getBuffData(id);
  if (pData == nullptr)
    return 0;
  return pData->lv % 100;
}

void BufferStateList::onKillMonster(SceneNpc* npc, EBuffType buffType)
{
  if (!npc) return;

  SceneUser* user = dynamic_cast<SceneUser*> (m_pEntry);
  if (!user || !user->getScene())
    return;

  auto it = m_mapType2SetBuff.find(buffType);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto &s : it->second)
    {
      SBufferData* pData = getBuffData(s);
      if (pData == nullptr)
        continue;
      if (pData->activeFlag == false)
        continue;
      BuffDropFoodReward* pFood = dynamic_cast<BuffDropFoodReward*> (pData->pBuff.get());
      if (pFood == nullptr)
        continue;
      DWORD layer = pData->layers;
      pFood->onKillMonster(user, npc, *pData);
      if (layer != pData->layers)
        addUpdate(*pData);
    }
  }
}

bool BufferStateList::isForceAddStatus(DWORD status)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_FORCESTATUS);
  if (it == m_mapType2SetBuff.end())
    return false;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffForceStatus* pBuff = dynamic_cast<BuffForceStatus*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    if (pBuff->haveStatus(status))
      return true;
  }

  return false;
}

QWORD BufferStateList::getSkillExtraTarget(DWORD skillid, const TSetQWORD& nowTargets)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_SKILLTARGET);
  if (it == m_mapType2SetBuff.end())
    return 0;

  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffSkillTarget* pBuff = dynamic_cast<BuffSkillTarget*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    return pBuff->getTarget(*pData, skillid, nowTargets);
  }

  return 0;
}


void BufferStateList::clientHideBuff(DWORD buffid)
{
  m_setClientHideBuff.insert(buffid);
  m_eHideState = EHideState_Hide;
  XLOG << "[Buff-添加隐藏], 对象:" << m_pEntry->name << m_pEntry->id << "buffid=" << buffid << XEND;
}

void BufferStateList::clearClientHideBuff()
{
  m_eHideState = EHideState_Show;
  XLOG << "[Buff-取消隐藏], 对象:" << m_pEntry->name << m_pEntry->id << "size=" << m_setClientHideBuff.size() << XEND;
}

bool BufferStateList::checkHasHideBuff()
{
  bool bHide = false;
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_HIDE);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto& v : it->second)
    {
      SBufferData* pData = getBuffData(v);
      if (pData == nullptr)
        continue;
      if (pData->activeFlag == true)
      {
        bHide = true;
        break;
      }
    }
  }

  if (!bHide)
  {
    auto itDeep = m_mapType2SetBuff.find(EBUFFTYPE_DEEPHIDE);
    if (itDeep != m_mapType2SetBuff.end())
    {
      for (auto& v : itDeep->second) 
      {
        SBufferData* pData = getBuffData(v);
        if (pData == nullptr)
          continue;
        if (pData->activeFlag == true)
        {
          bHide = true;
          break;
        }
      }
    }
  }

  return bHide;
}

float BufferStateList::getExtraExpRatio(EExtraRewardType eType)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_EXTRAREWARD);
  if (it == m_mapType2SetBuff.end())
    return 0;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffExtraReward* pBuff = dynamic_cast<BuffExtraReward*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    if (pBuff->getRewardType() == eType)
      return pBuff->getExtraExpRatio();
  }
  return 0;
}

float BufferStateList::getExtraZenyRatio(EExtraRewardType eType)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_EXTRAREWARD);
  if (it == m_mapType2SetBuff.end())
    return 0;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffExtraReward* pBuff = dynamic_cast<BuffExtraReward*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    if (pBuff->getRewardType() == eType)
      return pBuff->getExtraZenyRatio();
  }
  return 0;
}

void BufferStateList::onFinishEvent(EExtraRewardType eType)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_EXTRAREWARD);
  if (it == m_mapType2SetBuff.end())
    return;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;

    BuffExtraReward* pBuff = dynamic_cast<BuffExtraReward*> (pData->pBuff.get());
    if (pBuff == nullptr)
      continue;
    if (pBuff->getRewardType() != eType)
      continue;

    if (pData->layers > 0)
      pData->layers --;

    if (pData->layers == 0)
      del(s);
    else
      addUpdate(*pData);
  }
}

bool BufferStateList::hasSceneryID(DWORD sceneryid)
{
  if (sceneryid == 0)
    return false;
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_SCENERY);
  if (it == m_mapType2SetBuff.end())
    return false;
  for (auto& v : it->second)
  {
    SBufferData* pData = getBuffData(v);
    if (pData == nullptr)
      continue;
    BuffScenery* buff = dynamic_cast<BuffScenery*>(pData->pBuff.get());
    if (!buff)
      continue;
    if (buff->getSceneryID() == sceneryid)
      return true;
  }
  return false;
}

void BufferStateList::removeTauntBuffByFromID(QWORD fromid)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_TAUNT);
  if (it == m_mapType2SetBuff.end())
    return;
  for (auto& s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr || pData->fromID != fromid)
      continue;
    del(s, true);
  }
}

SBufferData* BufferStateList::getBuffData(DWORD id)
{
  auto it = m_mapID2BuffData.find(id);
  if (it != m_mapID2BuffData.end())
    return &(it->second);
  auto it2 = m_mapID2StaticBuff.find(id);
  if (it2 != m_mapID2StaticBuff.end())
    return &(it2->second);
  return nullptr;
}

void BufferStateList::getVecBuffDataByBuffType(EBuffType eType, TVecSBuffData& vecBuffData)
{
  auto its = m_mapType2SetBuff.find(eType);
  if (its == m_mapType2SetBuff.end())
    return;
  for (auto &id: its->second)
  {
    auto it = m_mapID2BuffData.find(id);
    if (it != m_mapID2BuffData.end())
    {
      vecBuffData.push_back(it->second);
      continue;
    }
    auto it2 = m_mapID2StaticBuff.find(id);
    if (it2 != m_mapID2StaticBuff.end())
    {
      vecBuffData.push_back(it2->second);
    }
  }
  return;
}

bool BufferStateList::canReboundStatus(EBuffStatusType status)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_STATUSREBOUND);
  if (it == m_mapType2SetBuff.end())
    return false;
  for (auto& s: it->second) 
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr)
      continue;
    BuffStatusRebound* pBuff = dynamic_cast<BuffStatusRebound*>(pData->pBuff.get());
    if (nullptr == pBuff)
      continue;
    if (pBuff->canRebound(status))
      return true;
  }
  return false;
}

void BufferStateList::getControlledAttr(TMapAttrControl& attrs)
{
  attrs.clear();
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_ATTRCONTROL);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto& v : it->second)
    {
      SBufferData* pData = getBuffData(v);
      if (pData == nullptr)
        continue;
      if (pData->activeFlag == false)
        continue;
      BuffAttrControl* buff = dynamic_cast<BuffAttrControl*>(pData->pBuff.get());
      if (buff == nullptr)
        continue;
      for (auto& v : buff->getAttr())
      {
        auto ait = attrs.find(v.first);
        if (ait == attrs.end())
          attrs[v.first] = v.second;
        else
          ait->second |= v.second;
      }
    }
  }
}
void BufferStateList::onChangeGender()
{
  checkBuffEnable(BUFFTRIGGER_GENDER);
}

bool BufferStateList::isEquipForbid(EPackType pack, EEquipPos pos, EEquipOper oper)
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_FORBIDEQUIP);
  if (it != m_mapType2SetBuff.end())
  {
    for (auto& v : it->second)
    {
      SBufferData* pData = getBuffData(v);
      if (pData == nullptr)
        continue;
      if (pData->activeFlag == false)
        continue;
      BuffForbidEquip* buff = dynamic_cast<BuffForbidEquip*>(pData->pBuff.get());
      if (buff == nullptr)
        continue;
      switch (oper)
      {
      case EEQUIPOPER_ON:
      case EEQUIPOPER_PUTFASHION:
        if (buff->isEquipForbidOn(pack, pos))
          return true;
        continue;
      case EEQUIPOPER_OFF:
      case EEQUIPOPER_OFFFASHION:
        if (buff->isEquipForbidOff(pack, pos))
          return true;
        continue;
      default:
        return false;
      }
    }
  }
  return false;
}

void BufferStateList::onSkillStatusChange()
{
  auto it = m_mapTrigType2Set.find(BUFFTRIGGER_CONCERT);
  if (it != m_mapTrigType2Set.end())
  {
    DWORD cur = now();
    for (auto &id : it->second)
    {
      SBufferData* pData = getBuffData(id);
      if (pData == nullptr)
        continue;
      auto p = pData->pBuff;
      if (p == nullptr)
        continue;
      if (pData->activeFlag == false && p->canStart(*pData, cur))
      {
        p->onStart(*pData, cur);
        setDataMark(p);
      }
      else if (pData->activeFlag == true && !p->canStart(*pData, cur))
      {
        p->onInvalid(*pData);
        setDataMark(p);
      }
    }
  }
}

bool BufferStateList::hasConcertAffactSkillBuff()
{
  auto it = m_mapType2SetBuff.find(EBUFFTYPE_AFFACTSKILL);
  if (it == m_mapType2SetBuff.end())
    return false;
  for (auto &s : it->second)
  {
    SBufferData* pData = getBuffData(s);
    if (pData == nullptr || pData->activeFlag == false)
      continue;
    BuffAffactSkill* pASBuff = dynamic_cast<BuffAffactSkill*>(pData->pBuff.get());
    if (pASBuff == nullptr)
      continue;
    if (pASBuff->getLastConcertSkill())
      return true;
  }
  return false;
}
