#include "SkillItem.h"
#include "SkillProcessor.h"
#include "SkillConfig.h"
#include "xSceneEntryDynamic.h"
#include "SceneTrapManager.h"
#include "SceneNpcManager.h"
#include "SceneActManager.h"
#include "SceneItemManager.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "GMCommandRuler.h"
#include "BufferManager.h"
#include "LuaManager.h"
#include "SceneManager.h"
#include "SceneUserManager.h"
#include "MsgManager.h"
#include "Package.h"
#include "FeatureConfig.h"
#include "DScene.h"
#include <iostream>
#include <fstream>
#include "StatisticsDefine.h"
#include "SkillManager.h"
#include "CommonConfig.h"
#include "GuildRaidConfig.h"
#include "AstrolabeConfig.h"
#include "Menu.h"
#include "ChatManager_SC.h"

static void sendMeSkillMessage(SceneUser* pUser, const string& str1, const string& str2, const string& str3, const string& str4)
{
  Cmd::ChatRetCmd message;
  message.set_channel(ECHAT_CHANNEL_WORLD);
  message.set_name(pUser->name);
  message.set_id(pUser->id);
  message.set_portrait(pUser->getPortrait().getCurPortrait());
  message.set_frame(pUser->getPortrait().getCurFrame());
  message.set_rolejob(pUser->getFighter()->getProfession());

  message.set_str(str1);
  PROTOBUF(message, send1, len1);
  pUser->sendCmdToMe(send1, len1);

  message.set_str(str2);
  PROTOBUF(message, send2, len2);
  pUser->sendCmdToMe(send2, len2);

  message.set_str(str3);
  PROTOBUF(message, send3, len3);
  pUser->sendCmdToMe(send3, len3);

  message.set_str(str4);
  PROTOBUF(message, send4, len4);
  pUser->sendCmdToMe(send4, len4);
}

// base skill
BaseSkill::BaseSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : m_dwSkillID(id), m_eType(eType), m_eLogic(eLogic), m_eCamp(eCamp)
{
  set_id(id);
}

BaseSkill::~BaseSkill()
{

}

// test print damage
DWORD BaseSkill::printDamage(xSceneEntryDynamic* atter, xSceneEntryDynamic* defer, const char* message1, const char* message2,const char* message3,const char* message4, DWORD type)
{
  SceneUser* attUser = dynamic_cast<SceneUser*> (atter);
  if (attUser != nullptr && (type == 1 || type == 3))
    sendMeSkillMessage(attUser, message1, message2, message3, message4);
  SceneUser* defUser = dynamic_cast<SceneUser*> (defer);
  if (defUser != nullptr && (type == 2 || type == 3))
    sendMeSkillMessage(defUser, message1, message2, message3, message4);
  return 0;
}

bool BaseSkill::isNormalSkill(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr)
    return false;
  SceneNpc* npc = dynamic_cast<SceneNpc*> (attacker);
  if (npc != nullptr)
  {
    return npc->getNormalSkill() == m_dwSkillID;
  }
  else
  {
    SceneUser* user = dynamic_cast<SceneUser*> (attacker);
    if (user)
      return user->getNormalSkill() == m_dwSkillID;
  }
  return false;
}

float BaseSkill::getReadyTime(xSceneEntryDynamic* attacker, bool bOriginal /*=false*/) const
{
  if (attacker == nullptr)
    return 0;
  float rtime = 0;
  switch (m_stLeadType.eType)
  {
    case ESKILLLEADTYPE_MIN:
      return 0.0f;
    case ESKILLLEADTYPE_ONE:
      {
        if (!bOriginal && attacker->isSkillNoReady())
          return 0;

        float readytime = m_stLeadType.fReadyTime;
        SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
        if (pUser && pUser->getFighter())
          readytime += pUser->getFighter()->getSkill().getChangeReady(m_dwSkillID);
        if (readytime < 0)
          return 0;
        rtime = readytime * ONE_THOUSAND;
        break;
      }
    case ESKILLLEADTYPE_TWO:
      {
        if (!bOriginal && attacker->isSkillNoReady())
          return 0;

        float ctchange = attacker->getAttr(EATTRTYPE_CTCHANGE);
        float ctchangeper = attacker->getAttr(EATTRTYPE_CTCHANGEPER);
        float ctfixed = attacker->getAttr(EATTRTYPE_CTFIXED);
        float ctfixeder = attacker->getAttr(EATTRTYPE_CTFIXEDPER);
        float castSpd = attacker->getAttr(EATTRTYPE_CASTSPD);
        SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
        if (pUser && pUser->getFighter() && pUser->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
        {
          ctchange += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_CTCHANGE);
          ctchangeper += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_CTCHANGEPER);
          ctfixed += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_CTFIXED);
          ctfixeder += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_CTFIXEDPER);
          castSpd += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_CASTSPD);
        }
        float changetime = m_stLeadType.fFCT * (1 + ctchangeper) + ctchange - castSpd;
        changetime = changetime < 0 ? 0 : changetime;
        float stabletime = (m_stLeadType.fCCT + ctfixed);
        if (stabletime < 0)
          stabletime = 0;
        stabletime *= (1 + ctfixeder);
        stabletime = stabletime < 0 ? 0 : stabletime;
        float time = (changetime + stabletime) * ONE_THOUSAND;
        rtime = time >= 0 ? time : 0;
        break;
      }
    case ESKILLLEADTYPE_THREE:
      rtime = m_stLeadType.fDuration * ONE_THOUSAND;
      break;
    case ESKILLLEADTYPE_LEAD:
      {
        float dchange = attacker->getAttr(EATTRTYPE_DCHANGE);
        float dchangeper = attacker->getAttr(EATTRTYPE_DCHANGEPER);
        SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
        if (pUser && pUser->getFighter() && pUser->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
        {
          dchange += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_DCHANGE);
          dchangeper += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_DCHANGEPER);
        }
        rtime = (m_stLeadType.fDCT + dchange) * (1 + dchangeper) * ONE_THOUSAND;
        break;
      }
    case ESKILLLEADTYPE_MAX:
      return 0.0f;
  }

  return rtime;
}

/*
bool BaseSkill::canBeBreak(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr)
    return false;
  if (m_stLeadType.eType == ESKILLLEADTYPE_ONE)
    return false;
  if (m_stLeadType.eType == ESKILLLEADTYPE_THREE)
    return true;
  if (getSkillType() == ESKILLTYPE_TRANSPORT)
    return false;
  // attreffect 二进制第5位表示吟唱不会中断
  return !attacker->isSkillNoBreak();
}
*/

float BaseSkill::getLaunchRange(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr)
    return 0;
  float disper = attacker->getAttr(EATTRTYPE_ATKDISTANCEPER);
  float disbase = attacker->getAttr(EATTRTYPE_ATKDISTANCE);
  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser && pUser->getFighter() && pUser->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
  {
    disper += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_ATKDISTANCEPER);
    disbase += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_ATKDISTANCE);
  }
  float dis = m_dwLaunchRange * (1 + disper) + disbase;
  return dis >= 0 ? dis : 0;
}

DWORD BaseSkill::getSpCost(SceneUser* pUser) const
{
  if (pUser == nullptr)
    return 0;
  float spper = pUser->getAttr(EATTRTYPE_SPCOSTPER);
  float spcost = pUser->getAttr(EATTRTYPE_SPCOST);
  if (pUser->getFighter() && pUser->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
  {
    spper += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_SPCOSTPER);
    spcost += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_SPCOST);
  }

  float sp = m_stSkillCost.dwSp * (1 + spper) + spcost;
  return sp >= 0 ? sp : 0;
}

// return ms
DWORD BaseSkill::getCD(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr)
    return 0;
  if (m_bFixCD)
    return m_fCD >= 0 ? m_fCD * ONE_THOUSAND : 0;
  float cdchange = attacker->getAttr(EATTRTYPE_CDCHANGE);
  float cdchangeper = attacker->getAttr(EATTRTYPE_CDCHANGEPER);
  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser && pUser->getFighter() && pUser->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
  {
    cdchange += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_CDCHANGE);
    cdchangeper += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_CDCHANGEPER);
  }
  float cd = ((float) m_fCD * (1 + cdchangeper) + cdchange) * ONE_THOUSAND;
  //if (attacker->getEntryType() == SCENE_ENTRY_USER)
    //cd -= CD_ERROR;
  return cd >= 0 ? cd : 0;
}

// return ms
DWORD BaseSkill::getDelayCD(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr)
    return 0;
  float cdchange = attacker->getAttr(EATTRTYPE_DELAYCDCHANGE);
  float cdchangeper = attacker->getAttr(EATTRTYPE_DELAYCDCHANGEPER);
  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser && pUser->getFighter() && pUser->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
  {
    cdchange += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_DELAYCDCHANGE);
    cdchangeper += pUser->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_DELAYCDCHANGEPER);
  }
  float cd = ((float) m_fDelayCD * (1 + cdchangeper) + cdchange) * ONE_THOUSAND;
  return cd >= 0 ? cd : 0;
}

QWORD BaseSkill::getDuration(xSceneEntryDynamic* attacker) const
{
  if (isTrap() == false)
    return 0;
  if (attacker == nullptr || attacker->getEntryType() == SCENE_ENTRY_NPC)
    return m_stLogicParam.qwDuration;
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->getFighter() == nullptr)
    return m_stLogicParam.qwDuration;
  return m_stLogicParam.qwDuration * (1 + user->getFighter()->getSkill().getDurationPer(m_dwSkillID));
}

DWORD BaseSkill::getCount(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr || attacker->getEntryType() == SCENE_ENTRY_NPC)
    return m_stLogicParam.dwCount;
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->getFighter() == nullptr)
    return m_stLogicParam.dwCount;
  int changecnt = user->getFighter()->getSkill().getChangeCount(m_dwSkillID);
  changecnt += m_stLogicParam.dwCount;
  return changecnt >= 0 ? changecnt : 0;
}

DWORD BaseSkill::getLimitCount(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr || attacker->getEntryType() == SCENE_ENTRY_NPC)
    return m_stLogicParam.dwMaxSkillCount;
  SceneUser* user = dynamic_cast<SceneUser*>(attacker);
  if (user == nullptr || user->getFighter() == nullptr)
    return m_stLogicParam.dwMaxSkillCount;
  int cnt = user->getFighter()->getSkill().getLimitCount(m_dwSkillID);
  cnt += m_stLogicParam.dwMaxSkillCount;
  return cnt >= 0 ? cnt : 0;
}

DWORD BaseSkill::getTargetNumLimit(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr || attacker->getEntryType() == SCENE_ENTRY_NPC)
    return m_stLogicParam.dwRangeMaxNum;
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->getFighter() == nullptr)
    return m_stLogicParam.dwRangeMaxNum;

  int changecnt = user->getFighter()->getSkill().getChangeTarCount(m_dwSkillID);
  changecnt += m_stLogicParam.dwRangeMaxNum;
  return changecnt >= 0 ? changecnt : 0;
}

/*DWORD BaseSkill::getRelSkillLv(xSceneEntryDynamic* attacker, DWORD index) const
{
  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser == nullptr || pUser->getFighter() == nullptr)
    return 0;

  if (index == 1)
    return pUser->getFighter()->getSkill().getSkillLv(m_relatedSkill_1);
  if (index == 2)
    return pUser->getFighter()->getSkill().getSkillLv(m_relatedSkill_2);
  if (index == 3)
    return pUser->getFighter()->getSkill().getSkillLv(m_relatedSkill_3);

  return 0;
}*/

bool BaseSkill::checkCondition(xSceneEntryDynamic* entry) const
{
  if (entry == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*>(entry);
  if (pUser == nullptr)
    return false;
  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter == nullptr)
    return false;

  if (m_stCondition.dwJobLv != 0 && pFighter->getJobLv() < m_stCondition.dwJobLv)
    return false;

  for (auto v = m_stCondition.vecPreSkillIDs.begin(); v != m_stCondition.vecPreSkillIDs.end(); ++v)
  {
    if (pFighter->getSkill().checkSkill(*v) == false)
      return false;
  }

  if (m_stCondition.dwMenuID != 0)
  {
    if (pUser->getMenu().isOpen(m_stCondition.dwMenuID) == false)
      return false;
  }

  if (m_stCondition.dwTitleID != 0)
  {
    if (pUser->getTitle().hasTitle(m_stCondition.dwTitleID) == false)
      return false;
  }

  if (m_stCondition.vecAllQuest.empty() == false)
  {
    for (auto v = m_stCondition.vecAllQuest.begin(); v != m_stCondition.vecAllQuest.end(); ++v)
    {
      if (pUser->getQuest().isSubmit(*v) == false)
        return false;
    }
  }

  if (m_stCondition.vecOneQuest.empty() == true)
    return true;

  for (auto v = m_stCondition.vecOneQuest.begin(); v != m_stCondition.vecOneQuest.end(); ++v)
  {
    if (pUser->getQuest().isSubmit(*v) == true)
      return true;
  }

  return false;
}

bool BaseSkill::init(xLuaData& params)
{
  // name
  m_strName = params.getTableString("NameZh");
  // lead type
  xLuaData& leadtype = params.getMutableData("Lead_Type");
  m_stLeadType.eType = static_cast<ESkillLeadType>(leadtype.getTableInt("type"));
  m_stLeadType.fReadyTime = leadtype.getTableFloat("ReadyTime");
  m_stLeadType.fDuration = leadtype.getTableFloat("duration");
  m_stLeadType.fCCT = leadtype.getTableFloat("CCT");
  m_stLeadType.fFCT = leadtype.getTableFloat("FCT");
  m_stLeadType.fDCT = leadtype.getTableFloat("DCT");

  m_dwLaunchRange = params.getTableFloat("Launch_Range");
  m_fCD = params.getTableFloat("CD");
  m_dwNextSkillID = params.getTableInt("NextID");
  m_dwBreakSkillID = params.getTableInt("NextBreakID");
  m_dwDeadSkillID = params.getTableInt("NextNewID");
  m_dwDeadLv = params.getTableInt("NewLevel");
  m_dwLevelCost = params.getTableInt("Cost");
  //m_dwSpcost = params.getTableInt("SpCost");
  m_fDelayCD = params.getTableFloat("DelayCD");
  m_bNearNormalSkill = params.getTableInt("Launch_Type") == 1;
  m_bCanEquipAuto = !(params.getTableInt("AutoCondition_Groove") == 1);
  m_bNoOverlay = params.getTableInt("NoOverlayEffect") == 1;
  m_bFixCD = params.getTableInt("FixCD") == 1;

  // logic param
  xLuaData& logicParam = params.getMutableData("Logic_Param");
  m_stLogicParam.m_oParam = logicParam;
  m_stLogicParam.fRange = logicParam.getTableFloat("range");
  m_stLogicParam.fWidth = logicParam.getTableFloat("width");
  m_stLogicParam.fDistance = logicParam.getTableFloat("distance");
  m_stLogicParam.dwCount = logicParam.getTableInt("count");
  if (m_stLogicParam.dwCount == 0)
    m_stLogicParam.dwCount = logicParam.getTableInt("hit");
  m_stLogicParam.dwCount = m_stLogicParam.dwCount == 0 ? 1 : m_stLogicParam.dwCount;
  m_stLogicParam.dwHitTime = logicParam.getTableInt("hit");
  m_stLogicParam.qwInterval = logicParam.getTableFloat("interval") * ONE_THOUSAND;
  m_stLogicParam.qwDuration = logicParam.getTableFloat("duration") * ONE_THOUSAND;
  m_stLogicParam.dwPetID = logicParam.getTableInt("petID");
  m_stLogicParam.fOffect = logicParam.getTableFloat("forward_offset");
  m_stLogicParam.bIsCountTrap = logicParam.getTableInt("isCountTrap") != 0;
  m_stLogicParam.bIsTimeTrap = logicParam.getTableInt("isTimeTrap") != 0;
  m_stLogicParam.bIsNpcTrap = logicParam.getTableInt("isNpcTrap") != 0;
  m_stLogicParam.dwTrapNpcID = logicParam.getTableInt("npcid");
  m_stLogicParam.dwRangeMaxNum = logicParam.getTableInt("range_num");
  m_stLogicParam.dwMaxSkillCount = logicParam.getTableInt("max_count");
  m_stLogicParam.bSelectTarget = logicParam.getTableInt("select_target") == 1;
  m_stLogicParam.bIncludeSelf = logicParam.getTableInt("include_self") == 1;

  m_stLogicParam.bClientNoSelect = logicParam.getTableInt("no_select") == 1;
  m_stLogicParam.m_dwDispHideType = logicParam.getTableInt("disperse_hide");
  m_stLogicParam.fTeamRange = logicParam.getTableFloat("team_range");
  if (logicParam.getMutableData("pre_attack").getTableInt("no_track") == 1)
    m_stLogicParam.m_oParam.setData("no_track", 1);
  m_stLogicParam.m_bForceHeal = logicParam.getTableInt("ForceHeal") == 1;
  m_stLogicParam.m_bIsMagicMachine = logicParam.getTableInt("is_magic_machine") == 1;
  m_stLogicParam.m_bImmunedBySuspend = logicParam.getTableInt("suspend_can_immune") == 1;
  m_stLogicParam.m_bNotImmunedByFieldArea = logicParam.getTableInt("fieldarea_cannot_immune") == 1;
  m_stLogicParam.m_bSelectHide = logicParam.getTableInt("select_hide") == 1;

  if (logicParam.has("invalid_target_type"))
  {
    const string& npctype = logicParam.getTableString("invalid_target_type");
    m_stLogicParam.eInvalidTarType = NpcConfig::getMe().getNpcType(npctype);
  }
  // get trap type
  if (logicParam.has("trap_type"))
  {
    string traptype = logicParam.getTableString("trap_type");
    if (traptype == "Transport")
      m_stLogicParam.eTrapType = ESKILLTRAPTYPE_TRANS;
    else
      m_stLogicParam.eTrapType = ESKILLTRAPTYPE_MIN;
  }
  m_stLogicParam.dwSuckSpType = logicParam.getTableInt("suck_sp_type");
  m_stLogicParam.m_bBuffOnlyMajor = logicParam.getTableInt("buff_only_major");
  m_stLogicParam.m_bNoHitBackMajor = logicParam.getTableInt("no_repel_major");
  m_stLogicParam.m_bZeroDamHitBack = logicParam.getTableInt("zero_damage_hitback");
  
  m_stLogicParam.m_vecChatid.clear();
  if(logicParam.has("chat_msg"))
  {
    auto getmsgid = [&](const string& key, xLuaData& data)
    {
      m_stLogicParam.m_vecChatid.push_back(data.getInt());
    };
    logicParam.getMutableData("chat_msg").foreach(getmsgid);
  };

  xLuaData& damtime = params.getMutableData("DamTime");
  m_dwDamTime = damtime.getTableInt("value");
  m_dwDamTime = m_dwDamTime == 0 ? 1 : m_dwDamTime;

  const string& trap = logicParam.getTableString("trap_effect");
  if (trap == "FireWall")
    m_eTrap = ESKILLTRAP_FIREWALL;

  xLuaData& hitEff = params.getMutableData("HitEffects");
  auto hitf = [this](const string& key, xLuaData& data)
  {
    float backdis = data.getTableFloat("distance");
    string direct = data.getTableString("direction");
    m_fHitBack = (direct == "back" ? backdis : 0 - backdis);
    m_fHitSpeed = data.getTableFloat("speed");
    m_bPvpNoHitBack = data.getTableInt("no_pvp") == 1;
  };
  hitEff.foreach(hitf);

  xLuaData& attEff = params.getMutableData("AttackEffects");
  auto attf = [this](const string& key, xLuaData& data)
  {
    float movedis = data.getTableFloat("distance");
    string direct = data.getTableString("direction");
    m_fAttMove = (direct == "back" ? 0 - movedis : movedis);
  };
  attEff.foreach(attf);

  xLuaData& buffD = params.getMutableData("Buff");

  xLuaData& selfBuffD = buffD.getMutableData("self");
  xLuaData& enemyBuffD = buffD.getMutableData("enemy");
  xLuaData& teamBuffD = buffD.getMutableData("team");
  xLuaData& friendBuffD = buffD.getMutableData("friend");
  xLuaData& selfSkillBuffD = buffD.getMutableData("self_skill");
  xLuaData& petSelfD = buffD.getMutableData("petself");
  xLuaData& petTeamD = buffD.getMutableData("petteam");
  xLuaData& petMasterD = buffD.getMutableData("pet_master");
  xLuaData& beingSelfD = buffD.getMutableData("beingself");
  xLuaData& guildBuffD = buffD.getMutableData("guild");
  xLuaData& selfOnDamageBuffD = buffD.getMutableData("selfondamage");
  xLuaData& selfOnceD = buffD.getMutableData("selfonce");

  m_mapBuff2MaxNum.clear();
  m_mapPvpBuff2MaxNum.clear();
  bool pvp = false;
  auto getbufflmt = [&](const string& key, xLuaData& d)
  {
    DWORD buffid = d.getTableInt("id");
    DWORD num = d.getTableInt("num");
    DWORD fmtype = d.getTableInt("num_type");
    if (!pvp)
    {
      pair<DWORD, DWORD>& pa = m_mapBuff2MaxNum[buffid];
      pa.first = num;
      pa.second = fmtype;
    }
    else
    {
      pair<DWORD, DWORD>& pa = m_mapPvpBuff2MaxNum[buffid];
      pa.first = num;
      pa.second = fmtype;
    }
  };
  buffD.getMutableData("limit_num").foreach(getbufflmt);

  xLuaData& pvpBuff = params.getMutableData("Pvp_buff");
  pvp = true;
  pvpBuff.getMutableData("limit_num").foreach(getbufflmt);

  DWORD tar = 0;

  m_selfbuffs.clear();
  m_enemybuffs.clear();
  m_teambuffs.clear();
  m_friendbuffs.clear();
  m_selfSkillBuffs.clear();
  m_setPetSelfBuffs.clear();
  m_setPetTeamBuffs.clear();
  m_setPetMasterBuffs.clear();
  m_setBeingSelfBuffs.clear();
  m_setGuildBuffs.clear();
  m_setSelfOnDamageBuffs.clear();
  m_setSelfOnceBuffs.clear();
  m_pvpSelfBuffs.clear();
  m_pvpEnemyBuffs.clear();
  m_pvpTeamBuffs.clear();
  m_pvpFriendBuffs.clear();
  m_pvpSelfSkillBuffs.clear();
  m_pvpPetSelfBuffs.clear();
  m_pvpPetTeamBuffs.clear();
  m_pvpPetMasterBuffs.clear();
  m_pvpGuildBuffs.clear();
  m_pvpSelfOnDamageBuffs.clear();
  m_pvpSetSelfOnceBuffs.clear();
  bool bCorrect = true;
  auto funbuff = [&](const std::string& key, xLuaData& data)
  {
    DWORD buffid = data.getInt();
    if (TableManager::getMe().getBufferCFG(buffid) == nullptr)
    {
      bCorrect = false;
      XERR << "[技能-Buff], 配置错误, 技能ID:" << m_dwSkillID << "Buffid:" << buffid << "未在Table_Buffer表中找到" << XEND;
      return;
    }
    if (tar == 1)
      m_selfbuffs.insert(data.getInt());
    else if (tar == 2)
      m_enemybuffs.insert(data.getInt());
    else if (tar == 3)
      m_teambuffs.insert(data.getInt());
    else if (tar == 4)
      m_friendbuffs.insert(data.getInt());
    else if (tar == 5)
      m_selfSkillBuffs.insert(data.getInt());
    else if (tar == 6)
      m_setPetSelfBuffs.insert(data.getInt());
    else if (tar == 7)
      m_setPetTeamBuffs.insert(data.getInt());
    else if (tar == 8)
      m_setPetMasterBuffs.insert(data.getInt());
    else if (tar == 9)
      m_setBeingSelfBuffs.insert(data.getInt());
    else if (tar == 10)
      m_setGuildBuffs.insert(data.getInt());
    else if (tar == 11)
      m_setSelfOnDamageBuffs.insert(data.getInt());
    else if (tar == 12)
      m_setSelfOnceBuffs.insert(data.getInt());

    else if (tar == 21)
      m_pvpSelfBuffs.insert(data.getInt());
    else if (tar == 22)
      m_pvpEnemyBuffs.insert(data.getInt());
    else if (tar == 23)
      m_pvpTeamBuffs.insert(data.getInt());
    else if (tar == 24)
      m_pvpFriendBuffs.insert(data.getInt());
    else if (tar == 25)
      m_pvpSelfSkillBuffs.insert(data.getInt());
    else if (tar == 26)
      m_pvpPetSelfBuffs.insert(data.getInt());
    else if (tar == 27)
      m_pvpPetTeamBuffs.insert(data.getInt());
    else if (tar == 28)
      m_pvpPetMasterBuffs.insert(data.getInt());
    else if (tar == 29)
      m_pvpGuildBuffs.insert(data.getInt());
    else if (tar == 30)
      m_pvpSelfOnDamageBuffs.insert(data.getInt());
    else if (tar == 31)
      m_pvpSetSelfOnceBuffs.insert(data.getInt());
  };
  tar = 1;
  selfBuffD.foreach(funbuff);
  tar = 2;
  enemyBuffD.foreach(funbuff);
  tar = 3;
  teamBuffD.foreach(funbuff);
  tar = 4;
  friendBuffD.foreach(funbuff);
  tar = 5;
  selfSkillBuffD.foreach(funbuff);
  tar = 6;
  petSelfD.foreach(funbuff);
  tar = 7;
  petTeamD.foreach(funbuff);
  tar = 8;
  petMasterD.foreach(funbuff);
  tar = 9;
  beingSelfD.foreach(funbuff);
  tar = 10;
  guildBuffD.foreach(funbuff);
  tar = 11;
  selfOnDamageBuffD.foreach(funbuff);
  tar = 12;
  selfOnceD.foreach(funbuff);

  tar = 21;
  pvpBuff.getMutableData("self").foreach(funbuff);
  tar = 22;
  pvpBuff.getMutableData("enemy").foreach(funbuff);
  tar = 23;
  pvpBuff.getMutableData("team").foreach(funbuff);
  tar = 24;
  pvpBuff.getMutableData("friend").foreach(funbuff);
  tar = 25;
  pvpBuff.getMutableData("self_skill").foreach(funbuff);
  tar = 26;
  pvpBuff.getMutableData("petself").foreach(funbuff);
  tar = 27;
  pvpBuff.getMutableData("petteam").foreach(funbuff);
  tar = 28;
  pvpBuff.getMutableData("pet_master").foreach(funbuff);
  tar = 29;
  pvpBuff.getMutableData("guild").foreach(funbuff);
  tar = 30;
  pvpBuff.getMutableData("selfondamage").foreach(funbuff);
  tar = 31;
  pvpBuff.getMutableData("selfonce").foreach(funbuff);

  // attrtype
  m_eAttrType = static_cast<ESkillAttrType>(params.getTableInt("RollType"));

  xLuaData& damage = params.getMutableData("Damage");
  auto damagef = [this](const string& key, xLuaData& data)
  {
    m_bHaveDamage = data.getTableInt("type") != 0;
    m_dwAtkAttrType = data.getTableInt("elementparam");
  };
  damage.foreach(damagef);

  xLuaData& condition = params.getMutableData("Contidion");
  m_stCondition.dwJobLv = condition.getTableInt("joblv");
  m_stCondition.dwMenuID = condition.getTableInt("menu");
  m_stCondition.dwTitleID = condition.getTableInt("riskid");
  if (m_stCondition.dwMenuID != 0 && MenuConfig::getMe().getMenuCFG(m_stCondition.dwMenuID) == nullptr)
  {
    XERR << "[BaseSkill::init] skillid = " << m_dwSkillID << " condition menu = " << m_stCondition.dwMenuID << " invalid" << XEND;
    return false;
  }
  if (m_stCondition.dwTitleID != 0 && ItemManager::getMe().getItemCFG(m_stCondition.dwTitleID) == nullptr)
  {
    XERR << "[BaseSkill::init] skillid = " << m_dwSkillID << " condition title = " <<m_stCondition.dwMenuID << " invalid" << XEND;
    return false;
  }

  m_stCondition.vecAllQuest.clear();
  m_stCondition.vecOneQuest.clear();
  m_stCondition.vecPreSkillIDs.clear();
  // skill condition
  xLuaData& allquest = condition.getMutableData("allquestid");
  auto allquestf = [this](const string& key, xLuaData& data)
  {
    m_stCondition.vecAllQuest.push_back(data.getInt());
  };
  allquest.foreach(allquestf);
  xLuaData& onequest = condition.getMutableData("onequestid");
  auto onequestf = [this](const string& key, xLuaData& data)
  {
    m_stCondition.vecOneQuest.push_back(data.getInt());
  };
  onequest.foreach(onequestf);
  if (condition.getTableInt("skillid") != 0)
    m_stCondition.vecPreSkillIDs.push_back(condition.getTableInt("skillid"));

  m_stRealseConds.vecConds.clear();
  // release cond
  xLuaData& relecond = params.getMutableData("PreCondition");
  auto getcond = [&](const string& key, xLuaData& data)
  {
    if (!data.has("type"))
      return;
    DWORD nType = data.getTableInt("type");
    if (nType <= ERELEASECOND_MIN || nType >= ERELEASECOND_MAX)
    {
      XERR << "[Table_Skill] id=" << m_dwSkillID << ", precondition 配置错误" << XEND;
      return;
    }
    SSkillReleaseCond cond;
    cond.eType = static_cast<EReleaseCond>(nType);
    switch(cond.eType)
    {
      case ERELEASECOND_USESKILL:
        cond.param1 = data.getTableInt("skillid");
        cond.param2 = data.getTableInt("time");
        break;
      case ERELEASECOND_EQUIP:
        cond.param1 = data.getTableInt("itemtype");
        cond.param2 = data.getTableInt("itemid");
        break;
      case ERELEASECOND_HPLESS:
        cond.param1 = data.getTableInt("value");
        break;
      case ERELEASECOND_ATTR:
        {
          auto funattr = [&](std::string key, xLuaData &data)
          {
            DWORD id = RoleDataConfig::getMe().getIDByName(key.c_str());
            if (id && Cmd::EAttrType_IsValid(id))
            {
              cond.uAttr.set_type((Cmd::EAttrType)id);
              cond.uAttr.set_value(data.getFloat());
            }
          };
          data.foreach(funattr);
        }
        break;
      case ERELEASECOND_PET:
        {
          cond.param1 = data.getMutableData("id").getTableInt("1");
          cond.param2 = data.getMutableData("id").getTableInt("2");
          break;
        }
      case ERELEASECOND_BUFF:
        {
          cond.param1 = data.getTableInt("id");
        }
        break;
      case ERELEASECOND_HAVESKILL:
        {
          cond.param1 = data.getTableInt("skillid");
        }
        break;
      case ERELEASECOND_BEING:
        {
          if (data.getTableInt("not_exist") == 1)
            cond.param1 = 1;
          else if (data.getTableInt("died") == 1)
            cond.param1 = 2;
          else if (data.getTableInt("alive") > 0)
          {
            cond.param1 = 3;
            cond.param2 = data.getTableInt("alive");
          }
        }
        break;
      case ERELEASECOND_OFFEQUIP:
        {
          cond.param1 = data.getTableInt("value");
        }
        break;
      case ERELEASECOND_DAMEQUIP:
        {
          cond.param1 = data.getTableInt("value");
        }
        break;
      default:
        break;
    }
    m_stRealseConds.vecConds.push_back(cond);
  };
  relecond.foreach(getcond);
  m_stRealseConds.bNeedBoth = relecond.getTableInt("both") != 0;
  m_stRealseConds.dwReleaseLuaFunc = relecond.getTableInt("ProType");

  // skill cost
  m_stSkillCost.vecBuff2Num.clear();
  m_stSkillCost.vecItem2Num.clear();
  xLuaData& costdata = params.getMutableData("SkillCost");
  auto getcost = [&](const string& key, xLuaData& data)
  {
    if (data.has("itemID"))
    {
      pair<DWORD, DWORD> pa;
      pa.first = data.getTableInt("itemID");
      pa.second = data.getTableInt("num");
      m_stSkillCost.vecItem2Num.push_back(pa);
    }
    else if (data.has("buffID"))
    {
      pair<DWORD, DWORD> pa;
      pa.first = data.getTableInt("buffID");
      pa.second = data.getTableInt("num");
      m_stSkillCost.vecBuff2Num.push_back(pa);
    }
  };
  costdata.foreach(getcost);
  m_stSkillCost.dwSp = costdata.getTableInt("sp");
  m_stSkillCost.dwHp = costdata.getTableInt("hp");
  m_stSkillCost.dwDynamicCostType = costdata.getTableInt("costtype");
  m_stSkillCost.dwMaxSpPer = costdata.getTableInt("maxspper");
  // skill extra cost
  m_vecExtraSkillCost.clear();
  xLuaData& extradata = params.getMutableData("StrengthenCost");
  auto extracost = [&](const string& key, xLuaData& data)
  {
    if (data.has("type") == false)
      return;
    DWORD dType = data.getTableInt("type");
    ESkillCost eType = static_cast<ESkillCost>(dType);
    if (eType <= ESKILLCOST_MIN || eType >= ESKILLCOST_MAX)
    {
      XERR << "[技能], 技能消耗配置错误,技能id:" << m_dwSkillID << ", 未知消耗类型:" << dType << XEND;
      return;
    }
    SExtraSkillCost stCost;
    stCost.eType = eType;
    stCost.dwCostID = data.getTableInt("id");
    stCost.dwCostNum = data.getTableInt("num");
    m_vecExtraSkillCost.push_back(stCost);
  };
  extradata.foreach(extracost);

  m_dwSuperUse = params.getTableInt("SuperUse");
  m_dwSuperUseEff = params.getTableInt("SuperUseEffect");
  m_dwForbidUse = params.getTableInt("ForbidUse");

  m_setIgnoreProfession.clear();
  auto targetfilterf = [&](const string& key, xLuaData& data)
  {
    DWORD pro = data.getInt();
    if (pro <= EPROFESSION_MIN || pro >= EPROFESSION_MAX || EProfession_IsValid(pro) == false)
      return;
    m_setIgnoreProfession.insert(static_cast<EProfession>(pro));
  };
  params.getMutableData("TargetFilter").foreach(targetfilterf);

  m_bNoHpBreak = params.getTableInt("NoHpBreak") == 1;
  m_dwBuffBreakLimit = params.getTableInt("BuffBreakLimit");
  m_bForceDelHide = logicParam.getTableInt("force_del_hide") == 1;
  m_bShareSkill = params.getTableInt("Share") == 1;
  m_bHaveBuff = havebuff();
  return bCorrect;
}

bool BaseSkill::checkCanUse(xSceneEntryDynamic* entry) const
{
  if (entry == nullptr)
    return false;
  SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
  auto checkOneCond = [&](const SSkillReleaseCond& cond) -> bool
  {
    switch(cond.eType)
    {
      case ERELEASECOND_USESKILL:
        {
          if (pUser == nullptr) // npc 释放技能 不加判断
            return true;
          DWORD skillid = cond.param1;
          DWORD interval = cond.param2;
          QWORD lastUseTime = entry->m_oSkillProcessor.getLastUseTime(skillid);
          return now() <= interval + lastUseTime;
        }
        break;
      case ERELEASECOND_EQUIP:
        {
          if (pUser == nullptr)
            return true; // npc 释放人物技能 不加判断
          if (pUser->getTransform().isMonster())
            return true;
          DWORD itemtype = cond.param1;
          DWORD itemid   = cond.param2;
          TVecSortItem pVecItems;
          pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->getEquipItems(pVecItems);
          if (itemtype > 0)
          {
            for (auto m = pVecItems.begin(); m != pVecItems.end(); ++m)
            {
              if((*m) && (*m)->getType() == itemtype)
                return true;
            }
          }else if(itemid > 0)
          {
            for (auto m = pVecItems.begin(); m != pVecItems.end(); ++m)
            {
              if ((*m) && (*m)->getTypeID() == itemid)
                return true;
            }
            
          }
          return false;
        }
        break;
      case ERELEASECOND_HPLESS:
        {
          float hp = entry->getAttr(EATTRTYPE_HP);
          float maxhp = entry->getAttr(EATTRTYPE_MAXHP);
          float per = cond.param1;
          return hp/maxhp * 100 <= per;
        }
        break;
      case ERELEASECOND_ATTR:
        {
          if (pUser == nullptr) // npc 释放技能 不加判断
            return true;
          EAttrType eType = cond.uAttr.type();
          if (eType == EATTRTYPE_ATTREFFECT || eType == EATTRTYPE_ATTRFUNCTION || eType == EATTRTYPE_ATTREFFECT2 || eType == EATTRTYPE_STATEEFFECT)
          {
            if (cond.uAttr.value() < 1 || cond.uAttr.value() > 32)
              return false;
            return (DWORD)(entry->getAttr(eType)) & (1 << (DWORD)(cond.uAttr.value() - 1));
          }
          float value = entry->getAttr(eType);
          return value >= cond.uAttr.value();
        }
        break;
      case ERELEASECOND_PET:
        {
          if (pUser == nullptr)
            return false; // npc 不可释放宠物技能
          DWORD pet = pUser->getPet().getPartnerID();
          if (pet == 0)
            return false;
          return pet == cond.param1 || pet == cond.param2;
        }
        break;
      case ERELEASECOND_BUFF:
        {
          if (pUser == nullptr) // npc 释放技能 不加判断
            return true;
          return pUser->m_oBuff.haveBuff(cond.param1);
        }
        break;
      case ERELEASECOND_HAVESKILL:
        {
          if (pUser == nullptr)
            return true;
          return pUser->getSkillLv(cond.param1);
        }
        break;
      case ERELEASECOND_BEING:
        {
          if (pUser == nullptr)
            return true;
          SSceneBeingData* being = pUser->getUserBeing().getCurBeingData();
          if (cond.param1 == 1) // 生命体不在场
            return being == nullptr;
          else if (cond.param1 == 2) // 生命体死亡
            return being != nullptr && being->bLive == false;
          else if (cond.param1 == 3) // 生命体在场活着
          {
            if (cond.param2 == 1)
              return being != nullptr && being->bLive == true;
            else
              return being != nullptr && being->bLive == true && being->dwID == cond.param2;
          }
          return false;
        }
        break;
      case ERELEASECOND_OFFEQUIP:
        {
          if (pUser == nullptr)
            return true;
          return pUser->getPackage().hasEquipForceOff();
        }
        break;
      case ERELEASECOND_DAMEQUIP:
        {
          if (pUser == nullptr)
            return true;
          EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
          if (pEquipPack == nullptr)
            return false;
          return pEquipPack->hasBraokenEquip();
        }
        break;
      default:
        return true;
        break;
    }
  };

  if (m_stRealseConds.vecConds.empty() == false)
  {
    // &&
    if (m_stRealseConds.bNeedBoth)
    {
      for (auto &v : m_stRealseConds.vecConds)
      {
        if (checkOneCond(v) == false)
          return false;
      }
      return true;
    }

    // ||
    else
    {
      for (auto &v : m_stRealseConds.vecConds)
      {
        if (checkOneCond(v))
          return true;
      }
      return false;
    }
  }

  // 动态前置条件检测
  if (m_stRealseConds.dwReleaseLuaFunc)
  {
    if (LuaManager::getMe().call<bool>("checkPrecondtion", m_stRealseConds.dwReleaseLuaFunc, entry, m_dwSkillID) == false)
      return false;
  }

  if (m_dwForbidUse)
  {
    if (m_dwForbidUse & 1)
    {
      // 公会战不可使用
      if (entry->getScene() && entry->getScene()->isGvg())
        return false;
    }
    if (m_dwForbidUse & 2)
    {
      // mvp 竞争战不可使用
      if (entry->getScene() && entry->getScene()->getSceneType() == SCENE_TYPE_MVPBATTLE)
        return false;
    }
    if (m_dwForbidUse & 4)
    {
      // 组队排位赛不能使用
      if(entry->getScene() && entry->getScene()->getSceneType() == SCENE_TYPE_TEAMPWS)
        return false;
    }
  }
  return true;
}

void BaseSkill::getSkillItemCost(map<DWORD, int>& item2num, SceneUser* pUser, DWORD beingid) const
{
  if (pUser == nullptr)
    return;

  if (beingid)
  {
    const SRuneSpecCFG* cfg = pUser->getUserBeing().getRuneSpecCFG(beingid, m_dwSkillID);
    if (cfg)
    {
      for (auto &v : m_stSkillCost.vecItem2Num)
      {
        int count = v.second;
        auto it = cfg->mapBeingItemCosts.find(v.first);
        if (it != cfg->mapBeingItemCosts.end())
          count += it->second;
        item2num[v.first] = count;
      }
      for (auto &m : cfg->mapBeingItemCosts)
      {
        if (item2num.find(m.first) != item2num.end())
          continue;
        item2num[m.first] = m.second;
      }
      return;
    }
  }

  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter == nullptr)
    return;

  const SSpecSkillInfo* pInfo = pFighter->getSkill().getSpecWithoutGlobal(m_dwSkillID);
  // 获取对全部技能的影响
  const SSpecSkillInfo* pAllInfo = pFighter->getSkill().getAllSkillSpec();

  // 若bNeedNoItem为true,则不需要消耗道具
  if((pInfo != nullptr && pInfo->bNeedNoItem)||(pAllInfo != nullptr && pAllInfo->bNeedNoItem)){
    return;
  }

  for (auto &v : m_stSkillCost.vecItem2Num)
  {
    item2num[v.first] = v.second;
  }

  if ( pInfo !=nullptr && pInfo->mapItem2NumAndPer.empty() == false )
  {
    for (auto &m : pInfo->mapItem2NumAndPer)
    {
      auto it = item2num.find(m.first);
      if( it != item2num.end())
      {
        int count = (it->second + m.second.first) * (1 + m.second.second);
        it->second = count;
      }
      else
      {
        int count = m.second.first * (1 + m.second.second);
        item2num[m.first] = count;
      }
    }
  }
  
  if( pAllInfo != nullptr && pAllInfo->mapItem2NumAndPer.empty() == false )
  {
    for (auto &m : pAllInfo->mapItem2NumAndPer)
    {
      auto it = item2num.find(m.first);
      if( it != item2num.end())
      {
        int count = ( it->second + m.second.first) * (1 + m.second.second);
        it->second = count;
      }
      else
      {
        int count = m.second.first * (1 + m.second.second);
        item2num[m.first] = count;
      }
    }
  }

  //去掉消耗小于等于0的item
  for (auto it = item2num.begin(); it != item2num.end();)
  {
    if (it->second <= 0)
      it = item2num.erase(it);
    else 
      ++it;
  }
}

bool BaseSkill::checkSkillCost(xSceneEntryDynamic* attacker) const
{
  DWORD beingid = 0;
  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser == nullptr || pUser->getTransform().isMonster())
  {
    if (pUser == nullptr)
    {
      BeingNpc* pBeingNpc = dynamic_cast<BeingNpc*>(attacker);
      if (pBeingNpc != nullptr)
        pUser = pBeingNpc->getMasterUser();
      if (pUser == nullptr)
        return true;
      beingid = pBeingNpc->getDefine().getID();
    }
    else
      return true;
  }
  if (pUser->getFighter() && pUser->getFighter()->getSkill().isFreeSkill(m_dwSkillID))
    return true;
  if (pUser->getSp() < getSpCost(pUser))
    return false;
  if (m_stSkillCost.dwHp && pUser->getAttr(EATTRTYPE_HP) <= m_stSkillCost.dwHp)
    return false;
  if (m_stSkillCost.dwMaxSpPer && pUser->getSp() < pUser->getAttr(EATTRTYPE_MAXSP) * m_stSkillCost.dwMaxSpPer / 100.0)
    return false;

  BasePackage* pPackage = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pPackage == nullptr)
    return false;

  map<DWORD, int> item2num;
  getSkillItemCost(item2num, pUser, beingid);
  if (item2num.empty() == false)
  {
    for (auto &m : item2num)
    {
      if (m.second == 0)
        continue;
      if (pPackage->checkItemCount(m.first, (DWORD)m.second) == false)
        return false;
    }
  }
  for (auto &v : m_stSkillCost.vecBuff2Num)
  {
    if (pUser->m_oBuff.getLayerByID(v.first) < v.second)
      return false;
  }
  return true;
}

bool BaseSkill::checkUseItem(xSceneEntryDynamic* entry, QWORD targetid) const
{
  if (m_stSkillCost.vecItem2Num.empty())
    return true;
  SceneUser* user = dynamic_cast<SceneUser*> (entry);
  if (!user) return true;

  for (auto &v : m_stSkillCost.vecItem2Num)
  {
    if (user->canUseItem(v.first, targetid, true) == false)
      return false;
  }
  return true;
}

void BaseSkill::doSkillCost(xSceneEntryDynamic* attacker, SkillRunner& oRunner) const
{
  DWORD beingid = 0;
  SceneUser* pUser = dynamic_cast<SceneUser*>(attacker);
  if (pUser == nullptr || pUser->getTransform().isMonster())
  {
    if (pUser == nullptr)
    {
      BeingNpc* pBeingNpc = dynamic_cast<BeingNpc*>(attacker);
      if (pBeingNpc != nullptr)
        pUser = pBeingNpc->getMasterUser();
      if (pUser == nullptr)
        return;
      beingid = pBeingNpc->getDefine().getID();
    }
    else
      return;
  }

  // 卡片, 装备 触发技能
  if (pUser->getFighter() && pUser->getFighter()->getSkill().isFreeSkill(m_dwSkillID))
    return;

  // sp
  if (pUser->getSp() >= getSpCost(pUser))
    pUser->setSp(pUser->getSp() - getSpCost(pUser));

  // hp
  if (m_stSkillCost.dwHp)
    pUser->changeHp(-(int)m_stSkillCost.dwHp, pUser);

  // maxspper
  if (m_stSkillCost.dwMaxSpPer)
  {
    float spcost = pUser->getAttr(EATTRTYPE_MAXSP) * m_stSkillCost.dwMaxSpPer / 100.0;
    if (pUser->getSp() >= spcost)
      pUser->setSp(pUser->getSp() - spcost);
  }

  // item cost
  BasePackage* pPackage = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pPackage)
  {
    map<DWORD, int> item2num;
    getSkillItemCost(item2num, pUser, beingid);
    if (item2num.empty() == false)
    {
      for (auto &m : item2num)
      {
        if (oRunner.getCmd().data().hitedtargets_size() > 0)
        {
          SceneUser* user = SceneUserManager::getMe().getUserByID(oRunner.getCmd().data().hitedtargets(0).charid());
          if (user)
          {
            user->getEvent().onItemBeUsed(m.first);
          }
          pUser->getQuest().onItemUse(oRunner.getCmd().data().hitedtargets(0).charid(), m.first);
        }
        pUser->getEvent().onItemUsed(m.first, (DWORD)m.second);

        pPackage->reduceItem(m.first, ESOURCE_ACTSKILL, (DWORD)m.second);
        const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(m.first);
        if (!pCFG) continue;
        /* 技能使用不计道具CD, 1218,和伟冲协商修改
        if (ItemConfig::getMe().isUseItem(pCFG->eItemType))
        {
          const string& oneguid = pPackage->getGUIDByType(m.first);
          ItemBase* pBase = pPackage->getItem(oneguid);
          if (pBase)
            pBase->setCD(xTime::getCurMSec());
          pUser->m_oCDTime.add(pCFG->dwTypeID, pCFG->dwCD, CD_TYPE_ITEM);
        }
        */
        MsgManager::sendMsg(pUser->id, 3055, MsgParams(pCFG->strNameZh, (DWORD)m.second));
      }
    }
  }
  // buff cost
  for (auto &v : m_stSkillCost.vecBuff2Num)
  {
    oRunner.setBuffLayer(v.first, pUser->m_oBuff.getLayerByID(v.first));

    if (v.second)
      pUser->m_oBuff.delLayer(v.first, v.second);
  }
  // extra cost, 不影响能否释放
  for (auto &v : m_vecExtraSkillCost)
  {
    switch(v.eType)
    {
      case ESKILLCOST_MIN:
        break;
      case ESKILLCOST_ARROW:
        {
          DWORD curArrow = pUser->getArrowID();
          if (curArrow == 0)
            continue;
          BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
          if (pMainPack == nullptr)
            return;
          if (pMainPack->checkItemCount(curArrow, v.dwCostNum))
          {
            oRunner.setArrowID(curArrow);
            pMainPack->reduceItem(curArrow, ESOURCE_ACTSKILL, v.dwCostNum);
            XDBG << "[箭矢消耗], 玩家:" << pUser->name << pUser->accid << pUser->id << ", 技能:" << m_dwSkillID << ", 箭矢消耗数量:" << v.dwCostNum << XEND;
          }
        }
        break;
      case ESKILLCOST_BUFF:
        {
          oRunner.setBuffLayer(v.dwCostID, pUser->m_oBuff.getLayerByID(v.dwCostID));
          pUser->m_oBuff.delLayer(v.dwCostID, v.dwCostNum);
        }
        break;
      case ESKILLCOST_MAX:
        break;
    }
  }

  // 动态消耗, 由lua实现
  if (m_stSkillCost.dwDynamicCostType)
    LuaManager::getMe().call<void>("doDynamicCost", attacker, m_stSkillCost.dwDynamicCostType, m_dwSkillID);
}

// ------------------- 技能释放 -----------------

bool BaseSkill::prepare(SkillRunner& oRunner) const
{
  if (isCostBeforeChant())
  {
    xSceneEntryDynamic* attacker = oRunner.getEntry();
    if (checkAttacker(attacker))
      doSkillCost(attacker, oRunner);
  }
  sendSkillStatusChant(oRunner);
  return true;
}

bool BaseSkill::chant(SkillRunner& oRunner) const
{
  return true;
}

void BaseSkill::handleDamage(xSceneEntryDynamic* enemy, xSceneEntryDynamic* attacker, HitedTarget* hit, DWORD targetsNum, SkillRunner& oRunner) const
{
  if (!enemy || !attacker)
    return;

  float total = 0;
  float damage = 0;

  //DWORD arrow = oRunner.getArrowID();
  SLuaSkillParam skillparam;
  skillparam.m_dwSkillID = m_dwSkillID;
  skillparam.m_dwTargetNumAndIndex = targetsNum;
  skillparam.m_dwArrowID = oRunner.getArrowID();
  skillparam.m_bShareDam = enemy->m_oBuff.haveBuffType(EBUFFTYPE_SHAREDAM);
  if (oRunner.haveBuffLayer())
  {
    const map<DWORD, DWORD>& buffs = oRunner.getBuffLayers();
    for (auto &m : buffs)
      skillparam.m_mapBuff2Layer[m.first] = m.second;
  }
  
  damage = LuaManager::getMe().call<float> ("exeDamage", attacker, enemy, &skillparam);

  if (damage >= 1000000)
  {
    XERR << "[技能-伤害异常], 伤害值:" << damage << "攻击者:" << attacker->name << attacker->id << "被攻击者:" << enemy->name << enemy->id << XEND;
    auto printbuf = [&](SBufferData& r)
    {
      XLOG << r.id;
    };
    XLOG << "攻击者当前buff:";
    attacker->m_oBuff.foreach(printbuf);
    XLOG << XEND;

    XLOG << "受击者当前buff:";
    enemy->m_oBuff.foreach(printbuf);
    XLOG << XEND;
  }

  if (skillparam.m_mapID2ShareDam.empty() == false)
  {
    for (auto &m : skillparam.m_mapID2ShareDam)
    {
      ShareDamTarget* pShareTarget = hit->add_sharetargets();
      if (pShareTarget == nullptr)
        continue;
      pShareTarget->set_charid(m.first);
      pShareTarget->set_damage(m.second.first);
      pShareTarget->set_type(m.second.second);
    }
  }

  int eType = enemy->getTempDamageType();
  if (eType != DAMAGE_TYPE_INVALID)
  {
    hit->set_type(eType);
    // lua 返回none 类型 表示无效伤害
    if (eType == DAMAGE_TYPE_NONE)
      hit->set_type(DAMAGE_TYPE_INVALID);
  }
  total += damage;

  switch (hit->type())
  {
    case DAMAGE_TYPE_NORMAL:
      {
        //if (total == 0)
          //hit->set_type(DAMAGE_TYPE_MISS);
      }
      break;
    case DAMAGE_TYPE_CRITICAL:
      break;
    case DAMAGE_TYPE_MISS:
      {
        total = 0;
      }
      break;
    case DAMAGE_TYPE_BARRIER:
      {
        if (!enemy->getScene())
          break;
        xSceneEntrySet uSet;
        attacker->getScene()->getEntryListInBlock(SCENE_ENTRY_TRAP, enemy->getPos(), m_stLogicParam.fRange + 1, uSet);
        for (auto &s : uSet)
        {
          SceneTrap* pTrap = dynamic_cast<SceneTrap*> (s);
          if (pTrap == nullptr || pTrap->getType() != ETRAPTYPE_BARRIER)
            continue;
          if (!pTrap->getMaster() || pTrap->getMaster()->isMyTeamMember(enemy->id)== false)
            continue;
          pTrap->setCount(pTrap->getCount() + 1);
        }
      }
      break;
    case DAMAGE_TYPE_AUTOBLOCK:
      {
        DWORD stiff = enemy->m_oBuff.getAutoBlockStiff();
        if (stiff > 0)
          enemy->m_oMove.addBeHitDelayTime(stiff);
      }
      break;
    case DAMAGE_TYPE_WEAPONBLOCK:
      {
        DWORD stiff = enemy->m_oBuff.getWeaponBlockStiff();
        if (stiff > 0)
          enemy->m_oMove.addBeHitDelayTime(stiff);
      }
      break;
    default:
      break;
  }

  // test 秒人攻击
  if (attacker->m_blKiller)
  {
    total = total >=0 ? enemy->getAttr(EATTRTYPE_HP) : 0 - enemy->getAttr(EATTRTYPE_HP);
    hit->set_type(DAMAGE_TYPE_NORMAL);
  }

  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser != nullptr)
  {
    hit->set_damage(total);
    /*// 客户端直接释放的技能, 伤害值取客户端数据, 服务器仅做验证
    if (isClientNoSelect() == false)
    {
      float client_toal = hit->damage();
      if (abs(hit->damage() - total) >  DAMAGE_ERROR)
      {
        if (!attacker->m_blKiller && !getPetID()) // petid != 0 
          XERR << "[技能], 伤害计算错误, 玩家id: " << attacker->name << ", " << attacker->id << ", 技能id = " << m_dwSkillID << ", 客户端伤害: " << client_toal << ", 服务端伤害: " << (int)(total) << XEND;
        // 若出错以服务器计算为准
        hit->set_damage(total);
      }
    }
    // 服务端触发的技能, 伤害取服务器数据
    else
    {
      hit->set_damage(total);
    }
    */

    if (damage > 0)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*>(enemy);
      if (pNpc)
        pUser->m_oUserStat.setSkillDamage(ESTATTYPE_SKILL_DAMAGE_MONSTER, m_dwSkillID, damage, pNpc->getNpcID());
      else
        pUser->m_oUserStat.setSkillDamage(ESTATTYPE_SKILL_DAMAGE_USER, m_dwSkillID, damage, enemy->id);
    }
  }
  // 怪物发动技能,以服务器为准
  else
  {
    hit->set_damage(total);
  }
}

bool BaseSkill::collectDamage(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (checkAttacker(attacker) == false)
    return false;

  // 设置随机数游标
  //if (oCmd.random() != 0)
  attacker->setRandIndex(oCmd.random());

  // copy data
  Cmd::PhaseData *pData = oCmd.mutable_data();
  PhaseData data(oCmd.data());
  pData->clear_hitedtargets();

  set<QWORD>& buffTargets = oRunner.getBuffTargets();
  int size = data.hitedtargets_size();
  for (int i = 0; i < size; ++i)
  {
    HitedTarget* pHit = data.mutable_hitedtargets(i);
    if (pHit == nullptr)
      continue;
    xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(pHit->charid());
    if (pEntry == nullptr || pEntry->isAlive() == false)
    {
      XLOG << "[Skill] 被攻击的目标不在场景或已死亡,attacker:" << attacker->id << ", enemy:" << pHit->charid() << ", skillid:" << m_dwSkillID << XEND;
      continue;
    }

    bool isEnemy = attacker->isMyEnemy(pEntry);

    if (isEnemy == true  && m_eCamp == ESKILLCAMP_FRIEND)
      continue;
    if (isEnemy == false && m_eCamp == ESKILLCAMP_ENEMY)
      continue;

    // 治愈类
    if (isEnemy == false)
    {
      handleDamage(pEntry, attacker, pHit, size * ONE_THOUSAND + i + 1, oRunner);
      if (pHit->damage() < 0)
      {
        if ((DWORD)pEntry->getAttr(EATTRTYPE_HP) >= (DWORD)pEntry->getAttr(EATTRTYPE_MAXHP))
        {
          pHit->set_type(DAMAGE_TYPE_INVALID);
          SceneUser* user = dynamic_cast<SceneUser*>(pEntry);
          if (user)
            user->onSkillHealMe(attacker, 0);
        }
      }
      pData->add_hitedtargets()->CopyFrom(*pHit);
      continue;
    }
    // 采集
    if (ESKILLNUMBER_CAIJI == oCmd.data().number())
    {
      pData->add_hitedtargets()->CopyFrom(*pHit);
      continue;
    }


    if (attacker->canAttack(pEntry) == false)
      continue;
    handleDamage(pEntry, attacker, pHit, size * ONE_THOUSAND + i + 1, oRunner);

    // 类型为invalid时, 表示该技能对该敌人无任何你作用
    if (pHit->type() == DAMAGE_TYPE_INVALID && pHit->sharetargets_size() <= 0)
    {
      buffTargets.erase(pHit->charid());// 对于敌人命中才会添加buff
      continue;
    }

    // 暴击 表情
    if (pHit->type() == DAMAGE_TYPE_CRITICAL)
    {
      attacker->checkEmoji("Critical");
      pEntry->checkEmoji("BeCritical");
    }

    // miss
    if (pHit->damage() == 0)
    {
      pEntry->checkEmoji("Miss");
      attacker->checkEmoji("EnemyMiss");
      bool berase = true;
      if (pHit->sharetargets_size() > 0)  // 分担伤害时buff效果依然有效
        berase = false;
      if (attacker->isMissStillBuff()) // miss依然需要添加buff
        berase = false;

      if (berase)
        buffTargets.erase(pHit->charid());// 对于敌人命中才会添加buff
    }

    pData->add_hitedtargets()->CopyFrom(*pHit);

    // 位置同步
    if (m_fHitBack != 0 && pHit->damage() > 0 && isClientNoSelect() && pEntry->isNoHitBack() == false)
    {
      if (checkNoHitback(attacker) == false)
      {
        Cmd::GoToUserCmd message;
        message.set_charid(pEntry->id);
        ScenePos *p = message.mutable_pos();
        p->set_x(pEntry->getPos().getX());
        p->set_y(pEntry->getPos().getY());
        p->set_z(pEntry->getPos().getZ());
        PROTOBUF(message, send, len);
        pEntry->sendCmdToNine(send, len);
      }
    }
  }

  if (attacker->m_oBuff.haveBuffType(EBUFFTYPE_SKILLTARGET))
  {
    TSetQWORD tars;
    for (int i = 0; i < data.hitedtargets_size(); ++i)
      tars.insert(data.hitedtargets(i).charid());
    QWORD extraid = attacker->m_oBuff.getSkillExtraTarget(m_dwSkillID, tars);
    if (extraid)
    {
      xSceneEntryDynamic* target = xSceneEntryDynamic::getEntryByID(extraid);
      if (target)
      {
        HitedTarget oHit;
        oHit.set_charid(extraid);
        handleDamage(target, attacker, &oHit, size * ONE_THOUSAND + 101, oRunner);
        if (oHit.damage())
        {
          if (oHit.damage() > 0)
            oHit.set_type(DAMAGE_TYPE_NORMAL);
          else
            oHit.set_type(DAMAGE_TYPE_HEAL);

          SkillBroadcastUserCmd cmd;
          cmd.set_skillid(m_dwSkillID);
          cmd.set_charid(attacker->id);
          cmd.mutable_data()->add_hitedtargets()->CopyFrom(oHit);
          cmd.mutable_data()->set_number(ESKILLNUMBER_RET);
          PROTOBUF(cmd, send, len);
          target->sendCmdToNine(send, len);
          target->changeHp(0 - oHit.damage(), attacker);
        }
      }
    }
  }
  // 技能额外目标
  if (attacker->getExtraSkillTarget(m_dwSkillID) != 0)
  {
    QWORD qwTarID = attacker->getExtraSkillTarget(m_dwSkillID);
    bool alreadyIn = false;
    for (int i = 0; i < data.hitedtargets_size(); ++i)
    {
      if (data.hitedtargets(i).charid() == qwTarID)
      {
        alreadyIn = true;
        break;
      }
    }

    if (!alreadyIn)
    {
      xSceneEntryDynamic* target = xSceneEntryDynamic::getEntryByID(qwTarID);
      if (target && target->isAlive() && target->check2PosInNine(attacker))
      {
        HitedTarget oHit;
        oHit.set_charid(target->id);
        handleDamage(target, attacker, &oHit, size * ONE_THOUSAND + 101, oRunner);
        if (oHit.damage())
        {
          if (oHit.damage() > 0)
            oHit.set_type(DAMAGE_TYPE_NORMAL);
          else
            oHit.set_type(DAMAGE_TYPE_HEAL);

          SkillBroadcastUserCmd cmd;
          cmd.set_skillid(m_dwSkillID);
          cmd.set_charid(attacker->id);
          cmd.mutable_data()->add_hitedtargets()->CopyFrom(oHit);
          cmd.mutable_data()->set_number(ESKILLNUMBER_RET);
          PROTOBUF(cmd, send, len);
          target->sendCmdToNine(send, len);
          target->changeHp(0 - oHit.damage(), attacker);
        }
      }
    }
  }

  return true;
}

bool BaseSkill::collectTarget(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  if (isTrap())
  {
    SceneTrap* pTrap = SceneTrapManager::getMe().getSceneTrap(oRunner.getParam1());
    if (pTrap)
    {
      oCmd.mutable_data()->mutable_pos()->set_x(pTrap->getPos().getX());
      oCmd.mutable_data()->mutable_pos()->set_y(pTrap->getPos().getY());
      oCmd.mutable_data()->mutable_pos()->set_z(pTrap->getPos().getZ());
    }
  }

  xPos oPos;
  oPos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());

  // 引导技能均以玩家为中心
  if (isLeadSkill())
    oPos = attacker->getPos();

  Cmd::PhaseData *pData = oCmd.mutable_data();
  PhaseData data(oCmd.data());
  pData->clear_hitedtargets();
  oRunner.getBuffTargets().clear();

  //DWORD maxtarnum = m_stLogicParam.dwRangeMaxNum;
  //if (maxtarnum)
  DWORD maxtarnum = getTargetNumLimit(attacker);

  auto buffadd = [&] (const xSceneEntrySet& set)
  {
    bool skill_only_enemy = (m_eCamp == ESKILLCAMP_ENEMY);
    bool skill_only_friend = (m_eCamp == ESKILLCAMP_FRIEND);

    if (m_stLogicParam.m_bBuffOnlyMajor)
    {
      for (int i = 0; i < pData->hitedtargets_size(); ++i)
      {
        oRunner.getBuffTargets().insert(pData->hitedtargets(i).charid());
        break;
      }
      return;
    }

    // 技能目标 add into Buff目标
    for (int i = 0; i < pData->hitedtargets_size(); ++i)
      oRunner.getBuffTargets().insert(pData->hitedtargets(i).charid());
    // 除去技能目标额外的Buff目标
    DWORD counter = 0;
    for (auto &s : set)
    {
      xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
      if (pEntry == nullptr)
        continue;
      if (pEntry->getEntryType() == SCENE_ENTRY_NPC && ((SceneNpc*)pEntry)->isMonster() == false)
        continue;
      if (maxtarnum != 0 && counter > maxtarnum)
        break;
      if (skill_only_enemy && attacker->isMyEnemy(pEntry) == false)
      {
        oRunner.getBuffTargets().insert(s->id);
        counter ++;
      }
      else if (skill_only_friend && attacker->isMyEnemy(pEntry) == true)
      {
        oRunner.getBuffTargets().insert(s->id);
        counter ++;
      }
      else if (m_stLogicParam.bIncludeSelf && attacker == pEntry)
      {
        oRunner.getBuffTargets().insert(s->id);
        counter ++;
      }
    }
  };
  // notice:client sends only skill targets(not buff targets) to server
  auto checkAdd = [&] (xSceneEntrySet& set, xSceneEntrySet& checkset, QWORD lockerid = 0) ->bool
  {
    bool skill_only_enemy = (m_eCamp == ESKILLCAMP_ENEMY);
    bool skill_only_friend = (m_eCamp == ESKILLCAMP_FRIEND);
    bool checkok = true;
    bool rangeSkillByServer = attacker->getScene() && attacker->getScene()->isHideUser() && isRangeSkill();

    // extra targets(spc.)
    if (m_stLogicParam.fTeamRange != 0)
    {
      SceneUser* user = dynamic_cast<SceneUser*>(attacker);
      if (user && user->getTeamID() != 0)
      {
        const GTeam& rTeam = user->getTeam();
        for (auto &m : rTeam.getTeamMemberList(true))
        {
          xSceneEntryDynamic* p = xSceneEntryDynamic::getEntryByID(m.second.charid());
          if (p == nullptr || p->getScene() != user->getScene())
            continue;
          if (getXZDistance(user->getPos(), p->getPos()) < m_stLogicParam.fTeamRange * 1.2)
          {
            set.insert(p);
            checkset.insert(p);
          }
        }
      }
      if (user)
      {
        std::list<SceneNpc*> petlist;
        user->getWeaponPet().getPetNpcs(petlist);
        for (auto &s : petlist)
        {
          set.insert(s);
          checkset.insert(s);
        }
      }
    }
    if (m_stLogicParam.eInvalidTarType != ENPCTYPE_MIN)
    {
      for (auto s = set.begin(); s != set.end(); )
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (*s);
        if (npc && npc->getNpcType() == m_stLogicParam.eInvalidTarType)
        {
          s = set.erase(s);
          continue;
        }
        ++s;
      }
      for (auto s = checkset.begin(); s != checkset.end(); )
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (*s);
        if (npc && npc->getNpcType() == m_stLogicParam.eInvalidTarType)
        {
          s = checkset.erase(s);
          continue;
        }
        ++s;
      }
    }
    if (!m_setIgnoreProfession.empty())
    {
      for (auto s = set.begin(); s != set.end();)
      {
        SceneUser* pUser = dynamic_cast<SceneUser*>(*s);
        if (pUser != nullptr && m_setIgnoreProfession.find(pUser->getProfession()) != m_setIgnoreProfession.end())
        {
          s = set.erase(s);
          continue;
        }
        ++s;
      }
      for (auto s = checkset.begin(); s != checkset.end();)
      {
        SceneUser* pUser = dynamic_cast<SceneUser*>(*s);
        if (pUser != nullptr && m_setIgnoreProfession.find(pUser->getProfession()) != m_setIgnoreProfession.end())
        {
          s = checkset.erase(s);
          continue;
        }
        ++s;
      }
    }
    // 只能对队友使用(除去自己)
    if (m_eCamp == ESKILLCAMP_TEAM)
    {
      for (auto s = set.begin(); s != set.end(); )
      {
        if (attacker->isMyTeamMember((*s)->id) == false || attacker == *s)
        {
          s = set.erase(s);
          continue;
        }
        ++s;
      }
      for (auto s = checkset.begin(); s != checkset.end(); )
      {
        if (attacker->isMyTeamMember((*s)->id) == false || attacker == *s)
        {
          s = checkset.erase(s);
          continue;
        }
        ++s;
      }
    }

    if (isTrap())
    {
      for (auto s = set.begin(); s != set.end(); )
      {
        xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*s);
        if (pEntry)
        {
          if (m_stLogicParam.m_bImmunedBySuspend && pEntry->isSuspend())
          {
            s = set.erase(s);
            continue;
          }

          if (!m_stLogicParam.m_bNotImmunedByFieldArea && pEntry->isImmuneTrap())
          {
            s = set.erase(s);
            continue;
          }
        }
        ++s;
      }
      for (auto s = checkset.begin(); s != checkset.end(); )
      {
        xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*s);
        if (pEntry)
        {
          if (m_stLogicParam.m_bImmunedBySuspend && pEntry->isSuspend())
          { 
            s = checkset.erase(s);
            continue;
          }

          if (!m_stLogicParam.m_bNotImmunedByFieldArea && pEntry->isImmuneTrap())
          {
            s = checkset.erase(s);
            continue;
          }
        }
        ++s;
      }
    }

    if (m_stLogicParam.m_bIsMagicMachine)
    {
      for (auto s = set.begin(); s != set.end(); )
      {
        xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*s);
        if (pEntry->isBeMagicMachine() == false)
        {
          s = set.erase(s);
          continue;
        }
        ++s;
      }
      for (auto s = checkset.begin(); s != checkset.end(); )
      {
        xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*s);
        if (pEntry->isBeMagicMachine() == false)
        {
          s = checkset.erase(s);
          continue;
        }
        ++s;
      }
    }

    // select by server
    if (isClientNoSelect() || attacker->getEntryType() == SCENE_ENTRY_NPC || oRunner.getState() == ESKILLSTATE_PREPARE || oRunner.noTarget() || rangeSkillByServer)
    {
      if (m_stLogicParam.bIncludeSelf)
        pData->add_hitedtargets()->set_charid(attacker->id);

      SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
      SceneNpc* pNpc = dynamic_cast<SceneNpc*> (attacker);
      std::list<xSceneEntry*> targetlist;
      if (!skill_only_enemy && !skill_only_friend)
      {
        for (auto &s : set)
        {
          xSceneEntryDynamic* entry = dynamic_cast<xSceneEntryDynamic*> (s);
          if (!entry)
            continue;
          if (attacker->isMyEnemy(entry))
            targetlist.push_back(s);
          else
            targetlist.push_front(s);
        }
      }
      else
      {
        for (auto &s : set)
        {
          if (lockerid && s->id == lockerid)
            targetlist.push_front(s);
          else
            targetlist.push_back(s);
        }
      }

      auto canAdd = [&](xSceneEntry* s) -> bool
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
        if (pUser != nullptr && npc != nullptr)
        {
          // exclude monsters that can't be seen by this user
          if (npc->checkNineScreenShow(pUser) == false)
          {
            ENpcType npctype = npc->getNpcType();
            bool ignoreHide = npctype == ENPCTYPE_SKILLNPC && npc->isMyEnemy(pUser);
            if (!ignoreHide) ignoreHide = (npctype == ENPCTYPE_PETNPC || npctype == ENPCTYPE_WEAPONPET);

            if (!ignoreHide)
              return false;
          }

          // skill npc 不可被加血
          if (npc->getNpcType() == ENPCTYPE_SKILLNPC && npc->isMyEnemy(pUser) == false)
            return false;
        }

        // just monster
        if (npc && npc->isMonster() == false)
          return false;

        // if the user can't see the npc , npc can't attack this user
        SceneUser* user = dynamic_cast<SceneUser*> (s);
        if (pNpc != nullptr && user != nullptr && pNpc->checkNineScreenShow(user) == false)
        {
          if (pNpc->getNpcType() != ENPCTYPE_PETNPC && pNpc->getNpcType() != ENPCTYPE_WEAPONPET)
            return false;
        }
        // if is noattacked, continue
        if (((xSceneEntryDynamic*)(s))->isNoAttacked())
          return false;

        xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*> (s);
        if (pEntry == nullptr)
          return false;

        if (pEntry->isNoEnemySkilled() && pEntry->isMyEnemy(attacker))
          return false;

        if (m_bNoOverlay && m_stLogicParam.qwInterval)
        {
          QWORD nexttime = pEntry->m_oSkillProcessor.getNextBeSkillTime(getFamilyID());
          if (nexttime > xTime::getCurMSec() + 100)
            return false;
        }
        xSceneEntryDynamic* pRealAttacker = getRealAttacker(attacker);
        if (pRealAttacker == nullptr)
          return false;

        bool selfok = m_stLogicParam.bIncludeSelf && pRealAttacker == pEntry;
        if (!selfok)
        {
        if (skill_only_enemy && pRealAttacker->isMyEnemy(pEntry) == false)
          return false;
        if (skill_only_friend && pRealAttacker->isMyEnemy(pEntry) == true)
          return false;
        }

        return true;
      };

      bool getLockTarget = false;
      if (oRunner.getLockTargetID() != 0)
      {
        for (auto &s : targetlist)
          if (s && s->id == oRunner.getLockTargetID())
          {
            if (canAdd(s))
            {
              pData->add_hitedtargets()->set_charid(s->id);
              getLockTarget = true;
            }
            break;
          }
      }

      if (getLockTarget == false)
      {
        for (auto &s : targetlist)
        {
          // range_num 目标上限
          if (maxtarnum != 0 && pData->hitedtargets_size() >= (int)maxtarnum)
            break;
          if (canAdd(s))
            pData->add_hitedtargets()->set_charid(s->id);
        }
      }
    }
    // select by client, server just to check 
    else
    {
      if (!m_stLogicParam.m_bSelectHide)
      {
        for (auto s = set.begin(); s != set.end(); )
        {
          xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*s);
          if (pEntry && pEntry->getBuff().checkHasHideBuff())
          {
            s = set.erase(s);
            continue;
          }
          ++s;
        }

        for (auto s = checkset.begin(); s != checkset.end(); )
        {
          xSceneEntryDynamic* pEntry = dynamic_cast<xSceneEntryDynamic*>(*s);
          if (pEntry && pEntry->getBuff().checkHasHideBuff())
          {
            s = checkset.erase(s);
            continue;
          }
          ++s;
        }
      }

      std::set<QWORD> setids;
      for (auto &s : checkset)
        setids.insert(s->id);

      for (int i = 0; i < data.hitedtargets_size(); ++i)
      {
        if (setids.find(data.hitedtargets(i).charid()) != setids.end())
        {
          pData->add_hitedtargets()->CopyFrom(data.hitedtargets(i));
          if (maxtarnum != 0 && pData->hitedtargets_size() >= (int)maxtarnum)
            break;
          continue;
        }
        checkok = false;
      }
      // 额外选择client no view target, ex: SkillNpc
      if (attacker->getScene() && attacker->getScene()->isPVPScene() && attacker->getEntryType() == SCENE_ENTRY_USER)
      {
        for (auto &s : set)
        {
          if (maxtarnum != 0 && pData->hitedtargets_size() >= (int)maxtarnum)
            break;
          SkillNpc* pNpc = dynamic_cast<SkillNpc*> (s);
          if (pNpc && pNpc->isMyEnemy(attacker))
          {
            pData->add_hitedtargets()->set_charid(s->id);
          }
        }
      }
    }

    // add buff targets
    buffadd(set);

    return checkok;
  };

  //float rangeError = isClientNoSelect() ? 0 : RANGE_ERROR;
  switch (m_eLogic)
  {
    case ESKILLLOGIC_FORWARDRECT://  SkillLogic::ForwardRect: client 选择目标
      {
        //pData->clear_hitedtargets();
        xSceneEntrySet set, checkset;
        float dir = 0;
        xPos oStartPos = attacker->getPos();
        if (attacker->getScene()->isPollyScene() && getXZDistance(oStartPos, oPos) < 3)
          oStartPos = oPos;

        xPos oEndPos = attacker->getPos();
        if (attacker->getEntryType() == SCENE_ENTRY_NPC)
        {
          dir = calcAngle(oStartPos, oPos);
        }
        else
        {
          SceneUser* user = dynamic_cast<SceneUser*> (attacker);
          if (user)
          {
            //dir = user->getUserSceneData().getDir() / ONE_THOUSAND;
            dir = (float)oCmd.data().dir() / ONE_THOUSAND;
            dir = dir / 180.0f * 3.14;
          }
        }

        float delta = abs(m_stLogicParam.fOffect);
        oStartPos.x = oStartPos.x + delta * sin(dir);
        oStartPos.z = oStartPos.z + delta * cos(dir);
        oEndPos.x = oStartPos.x + m_stLogicParam.fDistance * sin(dir);
        oEndPos.z = oStartPos.z + m_stLogicParam.fDistance * cos(dir);
        attacker->getScene()->getEntryIn2Pos(oStartPos, oEndPos, m_stLogicParam.fWidth, set);

        attacker->getScene()->getEntryList(attacker->getPos(), m_stLogicParam.fDistance + abs(m_stLogicParam.fOffect) + RANGE_ERROR, checkset);

        if (checkAdd(set, checkset) == false)
          XLOG << "技能目标选择不在合理范围, userid = " << attacker->id << ", skillid = " << m_dwSkillID << XEND;
        Cmd::ScenePos *p = oCmd.mutable_data()->mutable_pos();
        p->set_x(oPos.getX());
        p->set_y(oPos.getY());
        p->set_z(oPos.getZ());
      }
      break;
    case ESKILLLOGIC_MIN:
    case ESKILLLOGIC_LOCKEDTARGET:// SkillLogic::LockedTarget: client 选择目标
      {
        if (data.hitedtargets_size() == 0)
          break;
        xSceneEntrySet set, checkset;
        xSceneEntryDynamic* pBeLocker = xSceneEntryDynamic::getEntryByID(data.hitedtargets(0).charid());
        if (pBeLocker == nullptr || pBeLocker->getScene() == nullptr)
          break;
        set.insert(pBeLocker);
        checkset.insert(pBeLocker);

        // select_target = 1, skillnone, 可选择两个目标
        if (m_eLogic == ESKILLLOGIC_MIN && m_stLogicParam.bSelectTarget && data.hitedtargets_size() >= 2)
        {
          xSceneEntryDynamic* pEntry = xSceneEntryDynamic::getEntryByID(data.hitedtargets(1).charid());
          if (pEntry)
          {
            CHECK_DIS_PARAM(getLaunchRange(attacker), checkdist);
            if (getXZDistance(attacker->getPos(), pEntry->getPos()) < checkdist)
            {
              set.insert(pEntry);
              checkset.insert(pEntry);
            }
          }
        }

        float fRange = m_stLogicParam.fRange;
        if (attacker->getEntryType() == SCENE_ENTRY_USER)
        {
          SceneUser* user = dynamic_cast<SceneUser*> (attacker);
          if (user && user->getFighter())
            fRange += user->getFighter()->getSkill().getChangeRange(m_dwSkillID);
          fRange = fRange < 0 ? 0 : fRange;
        }
        else if (attacker->getEntryType() == SCENE_ENTRY_NPC)
        {
          BeingNpc* npc = dynamic_cast<BeingNpc*>(attacker);
          if (npc)
          {
            SceneUser* master = npc->getMasterUser();
            if (master)
            {
              fRange += master->getUserBeing().getRuneSpecRange(npc->getDefine().getID(), m_dwSkillID);
              fRange = fRange < 0 ? 0 : fRange;
            }
          }
        }
        if (fRange != 0)
        {
          pBeLocker->getScene()->getEntryList(pBeLocker->getPos(), fRange, set);
          pBeLocker->getScene()->getEntryList(pBeLocker->getPos(), fRange + RANGE_ERROR, checkset);
        }
        if (checkAdd(set, checkset, pBeLocker->id) == false)
          XLOG << "技能目标选择不在合理范围, userid = " << attacker->id << ", skillid = " << m_dwSkillID << XEND;
      }
      break;
    case ESKILLLOGIC_MISSILE:// SkillLogic::Missile: client 选择目标
      if (data.hitedtargets_size() == 1)
      {
        pData->add_hitedtargets()->CopyFrom(data.hitedtargets(0));
        oRunner.getBuffTargets().insert(data.hitedtargets(0).charid());
      }
      break;
    case ESKILLLOGIC_SELFRANGE:// SkillLogic::SelfRange: client 选择目标
      {
        xPos searchPos = isTrap() ? oPos : attacker->getPos();
        xSceneEntrySet set, checkset;
        float fRange = m_stLogicParam.fRange;
        if (attacker->getEntryType() == SCENE_ENTRY_USER)
        {
          SceneUser* user = dynamic_cast<SceneUser*> (attacker);
          if (user && user->getFighter())
            fRange += user->getFighter()->getSkill().getChangeRange(m_dwSkillID);
          fRange = fRange < 0 ? 0 : fRange;
        }
        else if (attacker->getEntryType() == SCENE_ENTRY_NPC)
        {
          BeingNpc* npc = dynamic_cast<BeingNpc*>(attacker);
          if (npc)
          {
            SceneUser* master = npc->getMasterUser();
            if (master)
            {
              fRange += master->getUserBeing().getRuneSpecRange(npc->getDefine().getID(), m_dwSkillID);
              fRange = fRange < 0 ? 0 : fRange;
            }
          }
        }
        attacker->getScene()->getEntryList(searchPos, fRange + RANGE_ERROR, checkset);
        attacker->getScene()->getEntryList(searchPos, fRange, set);

        if (checkAdd(set, checkset) == false)
        {
          XLOG << "技能目标选择不在合理范围, userid = " << attacker->id << ", skillid = " << m_dwSkillID << XEND;
        }
      }
      break;
    case ESKILLLOGIC_POINTRANGE:// SkillLogic::PointRange: client 选择目标
      {
        xSceneEntrySet set, checkset;
        float fRange = m_stLogicParam.fRange;
        if (attacker->getEntryType() == SCENE_ENTRY_USER)
        {
          SceneUser* user = dynamic_cast<SceneUser*> (attacker);
          if (user && user->getFighter())
            fRange += user->getFighter()->getSkill().getChangeRange(m_dwSkillID);
          fRange = fRange < 0 ? 0 : fRange;
        }
        else if (attacker->getEntryType() == SCENE_ENTRY_NPC)
        {
          BeingNpc* npc = dynamic_cast<BeingNpc*>(attacker);
          if (npc)
          {
            SceneUser* master = npc->getMasterUser();
            if (master)
            {
              fRange += master->getUserBeing().getRuneSpecRange(npc->getDefine().getID(), m_dwSkillID);
              fRange = fRange < 0 ? 0 : fRange;
            }
          }
        }
        attacker->getScene()->getEntryList(oPos, fRange + RANGE_ERROR, checkset);
        attacker->getScene()->getEntryList(oPos, fRange, set);

        if (checkAdd(set, checkset) == false)
        {
          XLOG << "技能目标选择不在合理范围, userid = " << attacker->id << ", skillid = " << m_dwSkillID << XEND;
        }
      }
      break;
    case ESKILLLOGIC_POINTRECT:// SkillLogic::PointRect:
      {
        xSceneEntrySet set, checkset;
        attacker->getScene()->getEntryList(oPos, m_stLogicParam.fWidth + RANGE_ERROR, m_stLogicParam.fDistance + RANGE_ERROR, checkset);
        attacker->getScene()->getEntryList(oPos, m_stLogicParam.fWidth, m_stLogicParam.fDistance, set);

        if (checkAdd(set, checkset) == false)
        {
          XLOG << "技能目标选择不在合理范围, userid = " << attacker->id << ", skillid = " << m_dwSkillID << XEND;
        }
        Cmd::ScenePos *p = oCmd.mutable_data()->mutable_pos();
        p->set_x(oPos.getX());
        p->set_y(oPos.getY());
        p->set_z(oPos.getZ());
      }
      break;
    case ESKILLLOGIC_RANDOMRANGE:
      {
        Scene* pScene = attacker->getScene();
        if (pScene == nullptr)
          break;
        if (oRunner.getTargetPos().empty())
          oRunner.setTargetPos(oPos);

        float effrange = m_stLogicParam.m_oParam.getTableFloat("effect_range");
        float centerrange = m_stLogicParam.fRange > effrange ? m_stLogicParam.fRange - effrange : 0;

        const xPos& centerPos = oRunner.getTargetPos();
        xPos pos;
        if (pScene->getRandPos(centerPos, centerrange, pos) == false)
          break;
        oCmd.mutable_data()->mutable_pos()->set_x(pos.getX());
        oCmd.mutable_data()->mutable_pos()->set_y(pos.getY());
        oCmd.mutable_data()->mutable_pos()->set_z(pos.getZ());

        xSceneEntrySet set;
        pScene->getEntryList(pos, effrange, set);
        checkAdd(set, set);
      }
      break;
    default:
      break;
  }

  return true;
}

void BaseSkill::sendSkillStatusChant(SkillRunner& oRunner) const
{
  float fReadyTime = oRunner.getCmd().chanttime();
  if (fReadyTime <= 0.0f)
    return;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker == nullptr)
    return;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  if (attacker->isCanMoveChant())
  {
    oCmd.mutable_data()->set_number(ESKILLNUMBER_MOVECHANT);
  }
  else
  {
    oCmd.mutable_data()->set_number(ESKILLNUMBER_CHANT);
  }
  PROTOBUF(oCmd, send, len);
  attacker->sendCmdToNine(send, len);

  attacker->checkEmoji("Cast");
}

// 同步受击
void BaseSkill::sendSkillStatusHit(SkillRunner& oRunner) const
{
  if (m_bHaveDamage == false)
    return;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  if (oCmd.data().hitedtargets_size() == 0)
    return;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker == nullptr || attacker->getScene() == nullptr)
    return;

  oCmd.mutable_data()->set_number(ESKILLNUMBER_RET);
  PROTOBUF(oCmd, send, len);
  attacker->getScene()->sendCmdToNine(attacker->getPos(), send, len);
}

void BaseSkill::sendSkillStatusRun(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker == nullptr)
    return;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  if (oCmd.data().number() == 0 || oCmd.data().number() == 3)
    oCmd.mutable_data()->set_number(ESKILLNUMBER_ATTACK);
  if (isTrap() && oRunner.getTrap())
  {
    oCmd.mutable_data()->set_number(ESKILLNUMBER_RET);
    if (oCmd.data().hitedtargets_size() == 0)
      return;
    //oCmd.clear_charid();
  }

  auto sendcmd = [&](SkillBroadcastUserCmd& cmd)
  {
    PROTOBUF(cmd, send, len);
    // 受击消息, 忽略玩家分组
    if (cmd.data().number() == ESKILLNUMBER_RET && attacker->getScene())
      attacker->getScene()->sendCmdToNine(attacker->getPos(), send, len);
    else
      attacker->sendCmdToNine(send, len);
  };

  // 此类型(延迟型)第一次需同步attack阶段, 后续同步受击阶段
  if (m_eLogic == ESKILLLOGIC_RANDOMRANGE)
  {
    if (oRunner.getCount() == 0)
    {
      oCmd.mutable_data()->set_number(ESKILLNUMBER_ATTACK);
      sendcmd(oCmd);
    }
    oCmd.mutable_data()->set_number(ESKILLNUMBER_RET);
  }

  Cmd::PhaseData *pData = oCmd.mutable_data();
  PhaseData data(oCmd.data());
  pData->clear_hitedtargets();
  for (int i  = 0; i < data.hitedtargets_size(); ++i)
  {
    if (data.hitedtargets(i).type() == DAMAGE_TYPE_INVALID && data.hitedtargets(i).sharetargets_size() <= 0)
      continue;
    pData->add_hitedtargets()->CopyFrom(data.hitedtargets(i));
  }

  SkillBroadcastUserCmd clientCmd;;
  clientCmd.CopyFrom(oCmd);

  if (!isClientNeedTarget()) // 对于某些特殊技能, 如闪光灯技能, 不适合此规则
  {
    clientCmd.mutable_data()->clear_hitedtargets();
  }
  // 公会战等, 范围类技能由服务端选择目标, 同步自己客户端受击
  if (attacker->getScene() && attacker->getScene()->isHideUser() && isRangeSkill())
  {
    SkillBroadcastUserCmd clientCmd2;;
    clientCmd2.CopyFrom(clientCmd);

    clientCmd.mutable_data()->clear_hitedtargets();
    sendcmd(clientCmd);

    clientCmd2.mutable_data()->set_number(ESKILLNUMBER_RET);
    sendcmd(clientCmd2);
  }
  else
  {
    sendcmd(clientCmd);
  }
}

bool BaseSkill::checkAttacker(xSceneEntryDynamic* &attacker) const
{
  if (attacker == nullptr || attacker->getScene() == nullptr)
    return false;
  return true;
}

xSceneEntryDynamic* BaseSkill::getRealAttacker(xSceneEntryDynamic* attacker) const
{
  if (checkAttacker(attacker) == false)
    return nullptr;
  SceneNpc* pSlave = dynamic_cast<SceneNpc*> (attacker);
  if (pSlave != nullptr && pSlave->getSkillMasterID() != 0)
  {
    attacker = xSceneEntryDynamic::getEntryByID(pSlave->getSkillMasterID());
  }
  return attacker;
}

bool BaseSkill::checkNoHitback(xSceneEntryDynamic* attacker) const
{
  if (attacker == nullptr || attacker->getScene() == nullptr)
    return false;

  if (m_fHitBack > 0 && attacker->getScene()->isGvg())
    return true;
  return false;
}

bool BaseSkill::checkHitback(int damage, DamageType etype) const
{
  switch(etype)
  {
    case DAMAGE_TYPE_INVALID:
    case DAMAGE_TYPE_CAIJI:
    case DAMAGE_TYPE_NONE:
    case DAMAGE_TYPE_MISS:
    case DAMAGE_TYPE_IMMUNE:
    case DAMAGE_TYPE_BARRIER:
    case DAMAGE_TYPE_AUTOBLOCK:
    case DAMAGE_TYPE_WEAPONBLOCK:
      return false;
    case DAMAGE_TYPE_NORMAL:
    case DAMAGE_TYPE_CRITICAL:
    case DAMAGE_TYPE_HEAL:
    case DAMAGE_TYPE_NORMALSP:
    case DAMAGE_TYPE_TREATSP:
      return damage != 0 ? true : m_stLogicParam.m_bZeroDamHitBack;
  }

  return false;
}

// -------- 技能流程 --------

// buff 处理
bool BaseSkill::addBuff(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (checkAttacker(attacker) == false)
    return false;
  if (!m_bHaveBuff)
    return false;

  auto getDamage = [&](const QWORD& id) -> int
  {
    for (int i = 0; i < oRunner.getCmd().data().hitedtargets_size(); ++ i)
    {
      if (oRunner.getCmd().data().hitedtargets(i).charid() == id)
        return oRunner.getCmd().data().hitedtargets(i).damage();
    }
    return 0;
  };
  // collect entry target
  xSceneEntrySet friendSet, enemySet, teamSet, guildSet;
  bool bSelfInSkill = false; // 标记自己是否在技能目标内

  SceneUser* pMe = dynamic_cast<SceneUser*> (attacker);
  set<QWORD>& qwSet = oRunner.getBuffTargets();
  for (auto &qw : qwSet)
  {
    xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(qw);
    if (pEntry == nullptr)
      continue;

    if (attacker == pEntry)
    {
      bSelfInSkill = true;
    }
    else if (attacker->isMyEnemy(pEntry) && !pEntry->isImmuneSkill(m_dwSkillID / 1000))
    {
      enemySet.insert(pEntry);
    }
    else
    {
      if (pMe && pMe->isMyTeamMember(qw))
        teamSet.insert(pEntry);
      else
        friendSet.insert(pEntry);

      if (pMe && pMe->hasGuild())
      {
        SceneUser* enemy = dynamic_cast<SceneUser*>(pEntry);
        if (enemy && enemy->getGuild().id() == pMe->getGuild().id())
          guildSet.insert(pEntry);
      }
    }
  }

  bool haveEffect = false;
  // gvg也和pvp同样处理
  bool bPvp = attacker->isOnPvp() || attacker->isOnGvg();

  // 添加一次的buff
  if (oRunner.isFirst())
  {
    if (!bPvp)
    {
      if (!m_setSelfOnceBuffs.empty())
      {
        for (auto &s : m_setSelfOnceBuffs)
          attacker->m_oBuff.add(s);
      }
    }
    else
    {
      if (!m_pvpSetSelfOnceBuffs.empty())
      {
        for (auto &s : m_pvpSetSelfOnceBuffs)
          attacker->m_oBuff.add(s);
      }
    }
  }

  auto funcadd = [&](const TSetDWORD& setBuffs, const xSceneEntrySet& setTars)
  {
    if (setBuffs.empty() || setTars.empty())
      return;

    // 获取buff目标限制
    auto getlmtnum = [&](const DWORD& id, const map<DWORD, pair<DWORD, DWORD>>& maplmt) -> DWORD
    {
      auto it = maplmt.find(id);
      if (it == maplmt.end())
        return 0;
      if (it->second.first)
        return it->second.first;
      return LuaManager::getMe().call<DWORD> ("calcFormulaValue", attacker, attacker, it->second.second);
    };

    for (auto &d : setBuffs)
    {
      // 限制buff目标数量
      DWORD lmtnum = bPvp ? getlmtnum(d, m_mapPvpBuff2MaxNum) : getlmtnum(d, m_mapBuff2MaxNum);

      if (lmtnum && setTars.size() > lmtnum)
      {
        std::vector<xSceneEntry*> vecset;
        for (auto &s : setTars)
          vecset.push_back(s);
        std::random_shuffle(vecset.begin(), vecset.end());
        for (DWORD i = 0; i < lmtnum; ++i)
        {
          xSceneEntryDynamic* p = dynamic_cast<xSceneEntryDynamic*> (vecset[i]);
          if (p == nullptr)
            continue;
          bool addok = p->m_oBuff.add(d, attacker, m_dwSkillID, getDamage(p->id));
          if (p->isAlive() == false && !addok)
            continue;
          haveEffect = true;
        }
      }
      else
      {
        for (auto &p : setTars)
        {
          bool addok = ((xSceneEntryDynamic *)(p))->m_oBuff.add(d, attacker, m_dwSkillID, getDamage(p->id));
          if (((xSceneEntryDynamic*)p)->isAlive() == false && !addok)
            continue;
          haveEffect = true;
        }
      }
    }
  };

  if (!bPvp)
  {
    for (auto m = m_selfbuffs.begin(); m != m_selfbuffs.end(); ++m)
    {
      oRunner.getEntry()->m_oBuff.add(*m, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
      haveEffect = true;
    }

    if (bSelfInSkill)
    {
      for (auto m = m_selfSkillBuffs.begin(); m != m_selfSkillBuffs.end(); ++m)
      {
        oRunner.getEntry()->m_oBuff.add(*m, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
        haveEffect = true;
      }
    }
    funcadd(m_enemybuffs, enemySet);
    funcadd(m_teambuffs, teamSet);// team buff 是除去自己的队友
    funcadd(m_friendbuffs, friendSet);
    funcadd(m_setGuildBuffs, guildSet);
    if (m_setPetMasterBuffs.empty() == false)
    {
      PetNpc* pPet = dynamic_cast<PetNpc*> (attacker);
      if (pPet)
      {
        SceneUser* pUser = pPet->getMasterUser();
        if (pUser)
        {
          for (auto &s : m_setPetMasterBuffs)
          {
            pUser->m_oBuff.add(s, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
          }
        }
      }
      BeingNpc* pBeing = dynamic_cast<BeingNpc*>(attacker);
      if (pBeing)
      {
        SceneUser* pUser = pBeing->getMasterUser();
        if (pUser)
        {
          for (auto &s : m_setPetMasterBuffs)
          {
            pUser->m_oBuff.add(s, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
          }
        }
      }

      ElementElfNpc* pElementElf = dynamic_cast<ElementElfNpc*>(attacker);
      if (pElementElf)
      {
        SceneUser* pUser = pElementElf->getMasterUser();
        if (pUser)
        {
          for (auto &s : m_setPetMasterBuffs)
          {
            pUser->m_oBuff.add(s, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
          }
        }
      }
    }

    // 战斗猫 buff
    if (pMe)
    {
      std::list<SceneNpc*> petnpcs;
      pMe->getWeaponPet().getPetNpcs(petnpcs);
      for (auto &s : petnpcs)
      {
        for (auto &q : m_setPetSelfBuffs)
        {
          s->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(s->id));
        }
        for (auto &q : m_setPetTeamBuffs)
        {
          if (teamSet.find(s) != teamSet.end())
            s->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(s->id));
        }
      }
      SceneNpc* beingNpc = pMe->getUserBeing().getCurBeingNpc();
      if (beingNpc)
      {
        for (auto &q : m_setPetSelfBuffs)
        {
          beingNpc->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(beingNpc->id));
        }
        for (auto &q : m_setBeingSelfBuffs)
        {
          beingNpc->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(beingNpc->id));
        }
      }
    }
  }
  else
  {
    for (auto m = m_pvpSelfBuffs.begin(); m != m_pvpSelfBuffs.end(); ++m)
    {
      oRunner.getEntry()->m_oBuff.add(*m, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
      haveEffect = true;
    }
    if (bSelfInSkill)
    {
      for (auto m = m_pvpSelfSkillBuffs.begin(); m != m_pvpSelfSkillBuffs.end(); ++m)
      {
        oRunner.getEntry()->m_oBuff.add(*m, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
        haveEffect = true;
      }
    }
    funcadd(m_pvpEnemyBuffs, enemySet);
    funcadd(m_pvpTeamBuffs, teamSet);
    funcadd(m_pvpFriendBuffs, friendSet);
    funcadd(m_pvpGuildBuffs, guildSet);

    if (m_pvpPetMasterBuffs.empty() == false)
    {
      PetNpc* pPet = dynamic_cast<PetNpc*> (attacker);
      if (pPet)
      {
        SceneUser* pUser = pPet->getMasterUser();
        if (pUser)
        {
          for (auto &s : m_pvpPetMasterBuffs)
          {
            pUser->m_oBuff.add(s, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
          }
        }
      }
      BeingNpc* pBeing = dynamic_cast<BeingNpc*>(attacker);
      if (pBeing)
      {
        SceneUser* pUser = pBeing->getMasterUser();
        if (pUser)
        {
          for (auto &s : m_setPetMasterBuffs)
          {
            pUser->m_oBuff.add(s, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
          }
        }
      }
      ElementElfNpc* pElementElf = dynamic_cast<ElementElfNpc*>(attacker);
      if (pElementElf)
      {
        SceneUser* pUser = pElementElf->getMasterUser();
        if (pUser)
        {
          for (auto &s : m_pvpPetMasterBuffs)
          {
            pUser->m_oBuff.add(s, attacker, m_dwSkillID, getDamage(oRunner.getEntry()->id));
          }
        }
      }
    }

    // 战斗猫 buff
    if (pMe)
    {
      std::list<SceneNpc*> petnpcs;
      pMe->getWeaponPet().getPetNpcs(petnpcs);
      for (auto &s : petnpcs)
      {
        for (auto &q : m_pvpPetSelfBuffs)
        {
          s->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(s->id));
        }
        for (auto &q : m_pvpPetTeamBuffs)
        {
          if (teamSet.find(s) != teamSet.end())
            s->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(s->id));
        }
      }
      SceneNpc* beingNpc = pMe->getUserBeing().getCurBeingNpc();
      if (beingNpc)
      {
        for (auto &q : m_setPetSelfBuffs)
        {
          beingNpc->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(beingNpc->id));
        }
        for (auto &q : m_setBeingSelfBuffs)
        {
          beingNpc->m_oBuff.add(q, attacker, m_dwSkillID, getDamage(beingNpc->id));
        }
      }
    }
  }

  return haveEffect;
}

// 技能效果
bool BaseSkill::runEffect(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (checkAttacker(attacker) == false)
    return false;
  xPos pos = attacker->getPos();
  switch (m_eLogic)
  {
    case ESKILLLOGIC_SELFRANGE:// SkillLogic::SelfRange: client 选择目标
    case ESKILLLOGIC_FORWARDRECT://  SkillLogic::ForwardRect: client 选择目标
    case ESKILLLOGIC_POINTRANGE:// SkillLogic::PointRange: client 选择目标
    case ESKILLLOGIC_POINTRECT:// SkillLogic::PointRect:
    case ESKILLLOGIC_RANDOMRANGE:
      if (oCmd.data().has_pos())
      {
        pos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());
      }
      break;
    case ESKILLLOGIC_MIN:
    case ESKILLLOGIC_LOCKEDTARGET:// SkillLogic::LockedTarget: client 选择目标
    case ESKILLLOGIC_MISSILE:// SkillLogic::Missile: client 选择目标
    default:
      break;
  }

  auto moveto = [&](xSceneEntryDynamic* entry, float distance, float dir)
  {
    if (entry == nullptr || entry->getScene() == nullptr || distance <= 0.1)
      return;
    xPos tarPos = entry->getPos();
    float angle = dir / 180.f * 3.14;
    float maxdiserr = CommonConfig::m_fSkillMoveDisErr;
    xPos tmpPos;
    xPos lastPos;
    while(distance > 0)
    {
      float dis = distance >=1 ? 1 : distance;
      distance -= 1;

      lastPos = tarPos;
      tarPos.x = tarPos.x + dis * sin(angle);
      tarPos.z = tarPos.z + dis * cos(angle);
      tmpPos = tarPos;
      if (entry->getScene()->getValidPos(tarPos) == false)
      {
        tarPos = lastPos;
        break;
      }
      //if (std::abs(tarPos.x - tmpPos.x) >= maxdiserr || std::abs(tarPos.z - tmpPos.z) >= maxdiserr)
      if (getXZDistance(tarPos, tmpPos) > maxdiserr)
      {
        tarPos = lastPos;
        break;
      }
    }
    if (getXZDistance(entry->getPos(), tarPos) > 0.1)
      entry->setScenePos(tarPos);
  };

  auto hitback = [&](xSceneEntryDynamic* pEnemy) -> bool
  {
    if (pEnemy == nullptr || pEnemy->getScene() == nullptr || m_fHitBack == 0)
      return false;

    if (pEnemy->getScene()->isPVPScene())
    {
      if (m_bPvpNoHitBack)
        return false;
    }
    if (pEnemy->isNoHitBack())
      return false;
    SceneNpc* npc = dynamic_cast<SceneNpc*> (pEnemy);
    if (npc != nullptr)
    {
      if (!(npc->getBehaviours() & BEHAVIOUR_MOVE_ABLE))
        return false;
      npc->m_ai.setPosSync();
    }
    pEnemy->m_oSkillProcessor.breakSkill(attacker->id);

    xPos tarPos = pEnemy->getPos();
    float dir = calcAngle(pos, tarPos);
    float dis = abs(m_fHitBack);
    // move "forward", 最多拉到攻击者面前
    if (m_fHitBack < 0)
    {
      float dis2 = getXZDistance(tarPos, pos);
      dis = dis2 < dis ? dis2 : dis;
    }

    dir = m_fHitBack > 0 ? dir : dir + 180;
    //XLOG << "gsj attacer pos:" << attacker->getPos().x << attacker->getPos().y << attacker->getPos().z << XEND;
    //XLOG << "gsj skill pos:" << pos.x << pos.y << pos.z << XEND;
    //XLOG << "gsj enemy pos 1" << pEnemy->getPos().x << pEnemy->getPos().y << pEnemy->getPos().z << XEND;
    moveto(pEnemy, dis, dir);
    //XLOG << "gsj enemy pos 2" << pEnemy->getPos().x << pEnemy->getPos().y << pEnemy->getPos().z << XEND;
    //XLOG << "--------" << XEND;
    return true;
  };

  auto attmove = [&](xSceneEntryDynamic* self)
  {
    if (self == nullptr || self->getScene() == nullptr || m_fAttMove == 0)
      return;
    if (self->getEntryType() == SCENE_ENTRY_NPC && ((SceneNpc*)self)->m_ai.isConfigCanMove() == false)
      return;
    if (self->isNoAttMove())
      return;

    float dir = oCmd.data().dir() / ONE_THOUSAND;
    float dis = abs(m_fAttMove);
    dir = m_fAttMove > 0 ? dir : dir + 180;

    self->m_oSkillProcessor.addSkillMove(m_dwSkillID, dis, self->getPos());
    moveto(self, dis, dir);
  };

  // 技能自身移动效果
  if (m_fAttMove != 0)
    attmove(attacker);

  // 冲锋
  if (m_stLogicParam.m_oParam.getTableInt("no_track") == 1)
  {
    xPos destpos;
    if (oCmd.data().has_pos())
      destpos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());
    Scene* pScene = attacker->getScene();
    if (pScene && pScene->getValidPos(destpos))
    {
      if (attacker->getEntryType() == SCENE_ENTRY_USER)
      {
        bool isLegalPos = false;
        float launchrange = getLaunchRange(attacker);
        for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
        {
          xSceneEntryDynamic* pTar = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(i).charid());
          if (pTar && getXZDistance(pTar->getPos(), destpos) < 3 && getXZDistance(destpos, attacker->getPos()) <= launchrange + 3)
          {
            isLegalPos = true;
            break;
          }
        }
        if (isLegalPos == false)
        {
          XERR << "[技能], 冲锋位置非法" << "玩家:" << attacker->name << attacker->id << "技能:" << m_dwSkillID << XEND;
          return false;
        }
      }
      attacker->setScenePos(destpos);
    }
  }

  // 附带 gm 指令
  bool hasSelfGM = m_stLogicParam.m_oParam.has("GM") && m_stLogicParam.m_oParam.getData("GM").has("id1") == false;
  bool hasOtherGM = m_stLogicParam.m_oParam.has("GM") && m_stLogicParam.m_oParam.getData("GM").has("id1") == true;
  if (hasSelfGM)
  {
    GMCommandRuler::getMe().execute(attacker, m_stLogicParam.m_oParam.getData("GM"));
  }
  else if (hasOtherGM)
  {
    PhaseData* pData = oCmd.mutable_data();
    if (pData == nullptr)
      return false;
    xLuaData gmdata = m_stLogicParam.m_oParam.getData("GM");
    for (int i = 0; i < pData->hitedtargets_size(); ++i)
    {
      gmdata.setData("id1", pData->hitedtargets(i).charid());
      GMCommandRuler::getMe().execute(attacker, gmdata);
    }
  }

  if (m_stLogicParam.m_oParam.has("interval_effect"))
  {
    xLuaData effdata = m_stLogicParam.m_oParam.getData("interval_effect");
    effdata.setData("type", "effect");
    effdata.setData("skillid", m_dwSkillID);
    xLuaData& posdata = effdata.getMutableData("pos");
    posdata.setData("1", pos.x);
    posdata.setData("2", pos.y);
    posdata.setData("3", pos.z);
    GMCommandRuler::getMe().execute(attacker, effdata);
  }

  bool bPvp = attacker->isOnPvp() || attacker->isOnGvg();
  auto hitMe = [&](const Cmd::HitedTarget& hited, xSceneEntryDynamic* pTarget)
  {
    if (pTarget == nullptr)
      return;
    pTarget->attackMe(hited.damage(), attacker);
    if (hited.sharetargets_size() != 0)
    {
      for (int i = 0; i < hited.sharetargets_size(); ++i)
      {
        xSceneEntryDynamic* pShareTarget = xSceneEntryDynamic::getEntryByID(hited.sharetargets(i).charid());
        if (pShareTarget == nullptr)
          continue;
        pShareTarget->attackMe(hited.sharetargets(i).damage(), attacker);
        DWORD delaytime = 0;
        if (hited.sharetargets(i).type() == DAMAGE_TYPE_AUTOBLOCK)
          delaytime = pShareTarget->m_oBuff.getAutoBlockStiff();
        else if (hited.sharetargets(i).type() == DAMAGE_TYPE_WEAPONBLOCK)
          delaytime = pShareTarget->m_oBuff.getWeaponBlockStiff();
        else
          delaytime = m_dwDamTime * BEATTACK_SPEED_BASE;
        if (delaytime > 0)
          pShareTarget->m_oMove.addBeHitDelayTime(delaytime);
      }
    }
    if (hited.damage() > 0)
    {
      if (!bPvp)
      {
        for (auto buffid : m_setSelfOnDamageBuffs)
        {
          attacker->m_oBuff.add(buffid, attacker, m_dwSkillID, hited.damage());
        }
      }
      else
      {
        for (auto buffid : m_pvpSelfOnDamageBuffs)
        {
          attacker->m_oBuff.add(buffid, attacker, m_dwSkillID, hited.damage());
        }
      }
    }
  };

  // 伤害 & 击退效果
  if (m_bHaveDamage)
  {
    PhaseData* pData = oCmd.mutable_data();
    if (pData == nullptr)
      return false;
    for (int i=0; i<pData->hitedtargets_size(); ++i)
    {
      const Cmd::HitedTarget &hited = pData->hitedtargets(i);

      xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(hited.charid());
      if (!pEntry || !pEntry->getScene())
        continue;

      bool bSpSkill = (hited.type() == DAMAGE_TYPE_NORMALSP || hited.type() == DAMAGE_TYPE_TREATSP);
      if (!bSpSkill)
      {
        if (hited.damage()>=0)
        {
          if (attacker->getEntryType() == SCENE_ENTRY_USER)
            XDBG << "[技能], 玩家发动攻击技能, 技能:" << m_dwSkillID << ", 伤害:" << hited.damage() << ", 攻击者:" << attacker->name << ", " << attacker->id << ", 被攻击者:" << pEntry->name << ", " << pEntry->id << XEND;
          //pEntry->attackMe(hited.damage(), attacker);
          hitMe(hited, pEntry);
          pEntry->onBeSkillAttack(this, attacker);

          // suck blood
          float suckblood = hited.damage() * (attacker->getAttr(EATTRTYPE_VAMPIRIC) + pEntry->getAttr(EATTRTYPE_BEVAMPIRIC));
          if (suckblood > 0)
          {
            attacker->changeHp(suckblood, attacker);
            attacker->sendBuffDamage(0 - suckblood);
          }
          // be reflected damage
          if (pEntry->canAttack(attacker))
          {
            float reflectP = 0.0f;
            if (m_eAttrType == ESKILLATTRTYPE_NORMAL)
              reflectP = pEntry->getAttr(EATTRTYPE_DAMREBOUND);
            else if (m_eAttrType == ESKILLATTRTYPE_MAGIC)
              reflectP = pEntry->getAttr(EATTRTYPE_MDAMREBOUND);
            float reflectblood = hited.damage() * reflectP;
            if (reflectblood > 0)
            {
              attacker->changeHp(0 - reflectblood, pEntry);
              if (attacker->getStatus() == ECREATURESTATUS_DEAD && attacker->getEntryType() == SCENE_ENTRY_USER)
              {
                ((SceneUser*)attacker)->setKillerName(pEntry->getName());
              }
              attacker->sendBuffDamage(reflectblood);
            }
          }

          // 先onAttackMe 后 onAttack, (ex:攻击时给敌人概率眩晕5秒, 受到攻击时解除眩晕)
          pEntry->m_oBuff.onAttackMe(attacker, this, (DamageType)hited.type(), hited.damage());
          attacker->m_oBuff.onAttack(pEntry, this, (DamageType)hited.type(), hited.damage());
          attacker->m_oSkillStatus.onAttack(pEntry);


          SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
          if (pUser)
          {
            pUser->addHitMe(attacker->id, hited.damage());
            pUser->setAction(0);
            if (pUser->getStatus() == ECREATURESTATUS_FAKEDEAD && hited.damage() > 0)
              pUser->setStatus(ECREATURESTATUS_LIVE);
          }

          if(m_fHitBack != 0 && checkHitback(hited.damage(), (DamageType)hited.type()) && checkNoHitback(attacker) == false)
          {
            // must put before m_oMove.addActionTime, if not, invalid 
            bool nohit = false;
            if (m_stLogicParam.m_bNoHitBackMajor && i == 0)
              nohit = true;

            bool bHitBack = (!nohit ? hitback(pEntry) : false);

            DWORD delayTime = 0;
            if (bHitBack)
            {
              DWORD backtime = m_fHitSpeed != 0 ? abs(m_fHitBack) / m_fHitSpeed * ONE_THOUSAND : 0;
              delayTime += backtime;

              float moveSpd = pEntry->getMoveSpeed();
              if (moveSpd)
              {
                float moveTime = MOVE_SPEED_BASE * 1 / moveSpd; // * 1, 与MoveAction::move 一致, 延迟一帧(1米)移动的时间
                delayTime += moveTime;
              }
            }
            if (m_dwDamTime != 0 && pEntry->getAttr(EATTRTYPE_NOSTIFF) == 0)
            {
              delayTime += m_dwDamTime * BEATTACK_SPEED_BASE;
            }
            if (delayTime)
              pEntry->m_oMove.addBeHitDelayTime(delayTime);
              //pEntry->m_oMove.addActionTime(delayTime);
            //XLOG << "gsj, addactiontime :" << delayTime << xTime::getCurMSec() << XEND;

            pEntry->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
          }

          if (hited.damage() != 0)
          {
            // skill suck blood
            float skillsuck = LuaManager::getMe().call<float>("getSuckBlood", attacker, hited.damage(), m_dwSkillID);
            if (skillsuck != 0)
            {
              attacker->changeHp(skillsuck, attacker);
              attacker->sendBuffDamage(0 - skillsuck);
            }
          }
        }
        else
        {
          DWORD deltaHp = pEntry->getAttr(EATTRTYPE_MAXHP) - pEntry->getAttr(EATTRTYPE_HP);
          pEntry->changeHp(0-hited.damage(), attacker, false, m_stLogicParam.m_bForceHeal);
          pEntry->checkEmoji("BeHeal");
          if (SCENE_ENTRY_USER==pEntry->getEntryType())
          {
            SceneUser *pUser = (SceneUser *)pEntry;
            pUser->lockMeCheckEmoji("EnemyHeal");
            pUser->playDynamicExpression(EAVATAREXPRESSION_SMILE);

            DWORD validhp = 0 - hited.damage();
            validhp = validhp < deltaHp ? validhp : deltaHp;
            pUser->onSkillHealMe(attacker, validhp);

            SceneNpc* pNpc = dynamic_cast<SceneNpc*>(attacker);
            if (pNpc && pNpc->getNpcID() == 1058)
            {
              pUser->getUserSceneData().addHealCount();
            }
          }
        }
        if (m_bNoOverlay && hited.damage() != 0 && m_stLogicParam.qwInterval)
        {
          pEntry->m_oSkillProcessor.addBeSkillTime(getFamilyID(), xTime::getCurMSec() + m_stLogicParam.qwInterval);
        }
      }

      // 吸蓝技能
      else
      {
        float spDam = hited.damage();
        if (spDam == 0)
          continue;
        float sp = pEntry->getAttr(EATTRTYPE_SP);
        float maxSp = pEntry->getAttr(EATTRTYPE_MAXSP);
        sp -= spDam;
        sp = sp < 0 ? 0 : sp;
        sp = sp > maxSp ? maxSp : sp;
        SceneUser* user = dynamic_cast<SceneUser*>(pEntry);
        if (user)
        {
          user->setSp(sp);
        }
        else
        {
          pEntry->setAttr(EATTRTYPE_SP, sp);
          pEntry->refreshDataAtonce();
        }
      }
    }
  }

  if (m_stLogicParam.dwSuckSpType)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (attacker);
    if (user)
    {
      for (auto &s : oRunner.getBuffTargets())
      {
        xSceneEntryDynamic* pTarget = xSceneEntryDynamic::getEntryByID(s);
        if (pTarget == nullptr || pTarget->getScene() != attacker->getScene())
          continue;
        int sp = LuaManager::getMe().call<DWORD> ("calcFormulaValue", attacker, pTarget, m_stLogicParam.dwSuckSpType);
        int nowsp = user->getSp();
        nowsp += sp;
        nowsp = nowsp < 0 ? 0 : nowsp;
        user->setSp(nowsp);
        user->sendSpDamage(-sp);
      }
    }
  }

  // 技能释放时发送聊天消息
  if(!m_stLogicParam.m_vecChatid.empty() && oRunner.isFirst())
  {
    SceneUser* user = dynamic_cast<SceneUser*> (attacker);
    if(user)
    {
      int random = randBetween(0,m_stLogicParam.m_vecChatid.size()-1);

      ChatManager_SC::getMe().skillChat(user, m_stLogicParam.m_vecChatid[random]);
    }
  }
  
  return true;
}

// 生成陷阱
bool BaseSkill::trap(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  if (isTrap() == false)
    return false;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  if (isNpcTrap() == true)
  {
  }
  else
  {
    SceneTrap* pTrap = SceneTrapManager::getMe().createSceneTrap(attacker->getScene(), oCmd, attacker->id, this);
    if (pTrap == nullptr)
      return false;
    oRunner.setParam1(pTrap->id);
  }

  oCmd.mutable_data()->set_number(ESKILLNUMBER_ATTACK);
  oCmd.mutable_data()->clear_hitedtargets();
  sendSkillStatusRun(oRunner);

  oRunner.setTrap(true);
  return true;
}

// 技能结束
void BaseSkill::end(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (attacker == nullptr)
    return;

  if (oRunner.getParam1())
  {
    SceneTrap* pTrap = SceneTrapManager::getMe().getSceneTrap(oRunner.getParam1());
    if (pTrap != nullptr)
    {
      pTrap->setClearState();
    }

    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(oRunner.getParam1());
    if (pNpc != nullptr)
    {
      if (oRunner.getCount() == 0)
        pNpc->setClearState();
    }
  }
}

// 技能运行流程
bool BaseSkill::run(SkillRunner& oRunner) const
{
  // trap
  if (isTrap() && oRunner.getTrap() == false)
  {
    if (trap(oRunner) == false)
      return false;
  }

  // 获得技能目标
  if (haveNoTargets() == false)
  {
    if (collectTarget(oRunner) == false)
      return false;
  }

  // 计算伤害
  if (m_bHaveDamage)
  {
    if (collectDamage(oRunner) == false)
      return false;
  }

  // 处理CommonFun返回, 实现特殊效果
  runSpecEffect(oRunner);

  // 通知前端伤害信息 & 技能运行阶段同步
  sendSkillStatusRun(oRunner);

  // 技能效果, 掉血, 位移
  //if (m_bHaveDamage)
  {
    if (runEffect(oRunner) == false)
      return false;
  }

  // 添加buff
  bool haveBuffEffect = false;
  if (m_bHaveBuff)
  {
    haveBuffEffect = addBuff(oRunner);
  }

  // 计数
  if (isTimeTrap())
  {
    if (haveBuffEffect || oRunner.getCmd().data().hitedtargets_size() != 0)
      oRunner.setCount(oRunner.getCount() + 1);
  }
  else
  {
    oRunner.setCount(oRunner.getCount() + 1);
  }

  if (m_bForceDelHide)
  {
    xSceneEntryDynamic* attacker = oRunner.getEntry();
    if (attacker)
      attacker->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
  }
  oRunner.setIsFirst(false);

  return true;
}



// attack skill
AttackSkill::AttackSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

AttackSkill::~AttackSkill()
{

}

bool AttackSkill::prepare(SkillRunner& oRunner) const
{
  BaseSkill::prepare(oRunner);

  collectTarget(oRunner);
  for (int i = 0; i < oRunner.getCmd().data().hitedtargets_size(); i++)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(oRunner.getCmd().data().hitedtargets(i).charid());
    if (npc == nullptr || oRunner.getEntry() == nullptr)
      continue;
    npc->m_ai.onBeChantLock(oRunner.getEntry()->id);
    npc->m_ai.addLockMeUserSkill(oRunner.getEntry()->id, m_dwSkillID);
    npc->m_sai.checkSig("chantlock");
    npc->m_oEmoji.check("BeCast");
  }
  return true;
}

bool AttackSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;

  // 使用攻击类技能时，消除隐匿状态
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  //attacker = getRealAttacker(attacker);
  if (checkAttacker(attacker) == false)
    return false;

  if (m_stLogicParam.m_dwDispHideType == 0)
  {
    attacker->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
    attacker->m_oBuff.delBuffByType(EBUFFTYPE_DEEPHIDE);
  }
  else
  {
    // 概率不删除隐匿
    DWORD rate = LuaManager::getMe().call<DWORD> ("calcFormulaValue", attacker, attacker, m_stLogicParam.m_dwDispHideType);
    if (rate <= (DWORD)randBetween(1, 100))
    {
      attacker->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
      attacker->m_oBuff.delBuffByType(EBUFFTYPE_DEEPHIDE);
    }
  }

  return true;
}

// buff skill
BuffSkill::BuffSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

BuffSkill::~BuffSkill()
{

}

// passive skill
PassiveSkill::PassiveSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

PassiveSkill::~PassiveSkill()
{

}

// heal skill
HealSkill::HealSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

HealSkill::~HealSkill()
{

;}

// 待删除
BowBashSkill::BowBashSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

BowBashSkill::~BowBashSkill()
{

}

// 待删除
RepairSkill::RepairSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

RepairSkill::~RepairSkill()
{

}


// ------- 瞬移技能 -----------
TelesportSkill::TelesportSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

TelesportSkill::~TelesportSkill()
{

}

bool TelesportSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_bContinueAttack = params.getMutableData("Logic_Param").getTableInt("continueAttack") == 1;
  return true;
}

bool TelesportSkill::getRandPos(xSceneEntryDynamic* attacker, xPos& p) const
{
  if (attacker == nullptr)
    return false;
  Scene* scene = attacker->getScene();
  if (scene == nullptr)
    return false;
  p = attacker->getPos();
  DWORD count = 0;
  while (count++ < 5)
  {
    if (scene->getRandPos(attacker->getPos(), m_dwLaunchRange, p) == true)
      break;
  }
  return true;
}

bool TelesportSkill::prepare(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  Scene* scene = attacker->getScene();
  if (scene == nullptr)
    return false;

  xPos tarpos;
  if (getRandPos(attacker, tarpos) == false)
    return false;
  oRunner.setTargetPos(tarpos);

  // send chant effect
  sendSkillStatusChant(oRunner);
  return true;
}

bool TelesportSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  // send original pos effect
  xLuaData data;
  data.setData("effect", MiscConfig::getMe().getEffectPath().strTeleportEffect);
  data.setData("posbind", 1);
  GMCommandRuler::effect(attacker, data);

  // send skill status
  collectTarget(oRunner);
  SkillBroadcastUserCmd oCmd = oRunner.getCmd();
  oRunner.getCmd().mutable_data()->clear_hitedtargets();
  sendSkillStatusRun(oRunner);

  if (oRunner.getTargetPos().empty())
  {
    xPos p;
    if (getRandPos(attacker, p) == false)
      return false;
    oRunner.setTargetPos(p);
    attacker->goTo(p);
  }
  else
  {
    attacker->goTo(oRunner.getTargetPos());
  }

  // send dest pos effect
  data.clear();
  data.setData("effect", MiscConfig::getMe().getEffectPath().strTeleportEffect);
  data.setData("posbind", 1);
  GMCommandRuler::effect(attacker, data);

  if (haveNoTargets() == false)
  {
    for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
    {
      if (oCmd.data().hitedtargets(i).charid() == attacker->id)
        continue;
      xSceneEntryDynamic* entry = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(i).charid());
      if (entry && entry->getEntryType() == SCENE_ENTRY_NPC)
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (entry);
        if (!npc || npc->isMonster() == false || npc->m_ai.isConfigCanMove() == false)
          continue;
      }
      if (entry && entry->getScene() && entry->getScene() == attacker->getScene())
      {
        xPos pos;
        //float dis = getXZDistance(entry->getPos(), attacker->getPos());
        if (entry->getScene()->getRandPos(oRunner.getTargetPos(), 1, pos))
        {
          data.clear();
          data.setData("effect", MiscConfig::getMe().getEffectPath().strTeleportEffect);
          data.setData("posbind", 1);
          GMCommandRuler::effect(attacker, data);

          entry->goTo(pos);

          data.clear();
          data.setData("effect", MiscConfig::getMe().getEffectPath().strTeleportEffect);
          data.setData("posbind", 1);
          GMCommandRuler::effect(attacker, data);
        }
      }
    }
  }

  /*if (m_eLogic == ESKILLLOGIC_LOCKEDTARGET && oCmd.data().hitedtargets_size() == 1)
  {
    // 挟持
    xSceneEntryDynamic* entry = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(0).charid());
    if (entry && entry->getScene() && entry->getScene() == attacker->getScene() && entry->isMyEnemy(attacker))
    {
      xPos pos;
      if (entry->getScene()->getRandPos(oRunner.getTargetPos(), 1, pos))
        entry->goTo(pos);
    }
  }
  */

  oRunner.setCount(oRunner.getCount() + 1);

  SceneNpc* pNpc = dynamic_cast<SceneNpc*> (attacker);
  if (pNpc && pNpc->m_oFollower.hasServant())
  {
    TVecQWORD vecSers;
    pNpc->m_oFollower.getServantIDs(vecSers);
    for (auto &v : vecSers)
    {
      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v);
      if (!npc || !npc->getScene() || !npc->isAlive())
        continue;
      xPos pos;
      DWORD keepdis = FeatureConfig::getMe().getNpcAICFG().stServant.dwKeepDis;
      if (npc->getScene()->getRandPos(oRunner.getTargetPos(), keepdis, pos))
      {
        data.clear();
        data.setData("effect", MiscConfig::getMe().getEffectPath().strTeleportEffect);
        data.setData("posbind", 1);
        GMCommandRuler::effect(npc, data);

        npc->goTo(pos);

        data.clear();
        data.setData("effect", MiscConfig::getMe().getEffectPath().strTeleportEffect);
        data.setData("posbind", 1);
        GMCommandRuler::effect(npc, data);
      }
    }
  }
  return true;
}

void TelesportSkill::end(SkillRunner& oRunner) const
{
  BaseSkill::end(oRunner);

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return;
  SceneNpc* pNpc = dynamic_cast<SceneNpc*>(attacker);
  if (pNpc != nullptr)
  {
    if (m_eCamp == ESKILLCAMP_ENEMY)
    {
      pNpc->m_ai.setCurLockID(oRunner.getParam1());
      pNpc->m_ai.changeState(ENPCSTATE_ATTACK);
    }
    else
    {
      if (m_bContinueAttack == false)
      {
        pNpc->m_ai.setCurLockID(0);
        pNpc->m_ai.changeState(ENPCSTATE_NORMAL);
      }
    }
  }
}
// ------- 瞬移技能 -----------


/*// ------- 净化技能 -----------
PurifySkill::PurifySkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{
  m_isConsumer = true;
}

PurifySkill::~PurifySkill()
{

}

bool PurifySkill::prepare(SkillRunner& oRunner) const
{
  return purify(oRunner, true);
}

bool PurifySkill::run(SkillRunner& oRunner) const
{
  return purify(oRunner, false);
}

bool PurifySkill::purify(SkillRunner& oRunner, bool haveChant) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (!pUser || !pUser->getScene())
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  const PhaseData& data = oCmd.data();

  collectTarget(oRunner);
  if (data.hitedtargets_size() == 0)
    return false;
  PurifyNpc *pRaidBoss = nullptr;
  for (int i = 0; i < data.hitedtargets_size(); ++i)
  {
    pRaidBoss = dynamic_cast<PurifyNpc*>(SceneNpcManager::getMe().getNpcByTempID(data.hitedtargets(i).charid()));
    if (!pRaidBoss || !pRaidBoss->getScene() || !pRaidBoss->needPurify())
      continue;
    break;
  }
  if (!pRaidBoss) return false;

  if (pRaidBoss->canPurifyBy(attacker->id) == false)
    return false;

  // 消耗净化值
  DWORD purify = pRaidBoss->define.getPurify();
  if (pUser->checkPurify(purify) == false)
    return false;
  pUser->subPurify(purify);

  // 通知客户端净化成功
  if (haveChant)
    sendSkillStatusChant(oRunner);
  else
    sendSkillStatusRun(oRunner);

  // 掉落物品
  TVecItemInfo vecItemInfo;
  for (auto v = pRaidBoss->base->vecRewardIDs.begin(); v != pRaidBoss->base->vecRewardIDs.end(); ++v)
  {
    TVecItemInfo item;
    if (RewardConfig::getMe().roll(*v, EPROFESSION_MIN, item, ESOURCE_REWARD) == false)
      continue;
    combinItemInfo(vecItemInfo, item);
  }
  Cmd::AddMapItem cmd;
  const SPurifyCFG& purifyCFG = MiscConfig::getMe().getSPurifyCFG();
  DWORD extraTime = purifyCFG.dwItemDropInterval;
  DWORD dispTime = purifyCFG.dwItemDisTime;
  for (size_t i = 0; i < vecItemInfo.size(); ++i)
  {
    xPos dest = pRaidBoss->getPos();

    SceneItem* pItem = SceneItemManager::getMe().createSceneItem(pRaidBoss->getScene(), vecItemInfo[i], dest);
    if (pItem != nullptr)
    {
        pItem->addOwner(attacker->id);
        pItem->setDispTime(dispTime);
        pItem->setViewLimit();
        pItem->setSourceID(pRaidBoss->id);
        pItem->fillMapItemData(cmd.add_items(), extraTime * (i + 1));
    }
    XLOG << "[Reward], 净化奖励掉落, 玩家: " << pUser->id << ", 地图: " << pUser->getScene()->getMapID() << ", 奖励id: " << vecItemInfo[i].id() << XEND;
  }
  //仅自己可见
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  //清除标志，不可再次净化
  pRaidBoss->purifyByUser(attacker->id);
  xSceneEntrySet actSet;
  pRaidBoss->getScene()-> getEntryListInBlock(SCENE_ENTRY_ACT, pRaidBoss->getPos(), 3,  actSet);
  for (auto s = actSet.begin(); s != actSet.end(); ++s)
  {
    SceneActPurify* pAct = dynamic_cast<SceneActPurify*> (*s);
    if (!pAct || pAct->getMaster() != pRaidBoss)
      continue;
    pAct->purifyByUser(attacker->id);
    break;
  }

  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}
// ------- 净化技能 -----------*/


// ------- 回城技能 -----------
TransportSkill::TransportSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

TransportSkill::~TransportSkill()
{

}

bool TransportSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_bIncludeFollower = params.getMutableData("Logic_Param").getTableInt("include_follower") == 1;
  return true;
}

bool TransportSkill::run(SkillRunner& oRunner) const
{
  sendSkillStatusRun(oRunner);

  SceneUser* pUser = dynamic_cast<SceneUser*>(oRunner.getEntry());
  if (pUser == nullptr)
    return false;

  DWORD mapid = pUser->getUserSceneData().getSaveMap();
  const xPos* pPos = oRunner.getTransPos();
  GoMapType type = GoMapType::Null;

  // 区分蝴蝶翅膀和传送阵
  if (m_stSkillCost.vecItem2Num.empty() && oRunner.getTransType() != ETRANSPORTTYPE_TRANSFER)
  {
    Scene* pScene = pUser->getScene();
    if (pScene != nullptr && pScene->getSceneBase() != nullptr)
    {
      DScene* pDScene = dynamic_cast<DScene*>(pScene);
      if (pDScene && (pDScene->getRaidType() == ERAIDTYPE_GUILD || pDScene->isPVPScene()))
      {
        mapid = pUser->getUserMap().getLastStaticMapID();
        pPos = &pUser->getUserMap().getLastStaticMapPos();
      }
      else if (pDScene && pDScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
      {
        GuildFireScene* pGScene = dynamic_cast<GuildFireScene*> (pDScene);
        if (pGScene)
        {
          if (pGScene->getDefenseGuildID() && pGScene->getDefenseGuildID() == pUser->getGuild().id())
          {
            const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
            mapid = rCFG.dwTerritory;
          }
          else
          {
            DWORD cityid = pGScene->getCityID();
            const SGuildCityCFG* pCityCFG = GuildRaidConfig::getMe().getGuildCityCFG(cityid);
            if (pCityCFG)
            {
              mapid = pCityCFG->dwMapID;
            }
          }
        }
      }
      else
      {
        mapid = pScene->getSceneBase()->getReliveMapID();
        const SceneBase* pBase = SceneManager::getMe().getDataByID(mapid);
        if (pBase != nullptr)
        {
          DWORD raidId = 0;
          if (pBase->isStaticMap())
            raidId = 0;
          else
            raidId = mapid;

          const SceneObject* pObject = pBase->getSceneObject(raidId);
          if (pObject != nullptr)
            pPos = pObject->getBornPoint(pScene->getSceneBase()->getReliveBp());
        }
      }
      type = GoMapType::GM;
    }
    if (oRunner.getTransMap() != 0)
      mapid = oRunner.getTransMap();
  }
  else
  {
    type = GoMapType::Skill;
  }

  oRunner.setCount(oRunner.getCount() + 1);

  switch(oRunner.getTransType())
  {
    case ETRANSPORTTYPE_GUILD:
      {
        const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
        pUser->getUserZone().gomap(pUser->getGuild().zoneid(), rCFG.dwTerritory, GoMapType::Null);
      }
      break;
    case ETRANSPORTTYPE_TRANSFER:
      {
        mapid = oRunner.getTransMap();
        pUser->gomap(mapid, type, pPos == nullptr ? xPos() : *pPos);
      }
      break;
    default:
      {
        Scene* pScene = pUser->getScene();
        std::set<SceneUser*> userset = pUser->getTeamSceneUser();

        pUser->gomap(mapid, type, pPos == nullptr ? xPos() : *pPos);
        if (m_bIncludeFollower)
        {
          userset.erase(pUser);
          for (auto &s : userset)
          {
            if (!s || s->getScene() != pScene)
              continue;
            if (s->getUserSceneData().getFollowerID() != pUser->id)
              continue;
            if (s->isAlive() == false)
              continue;
            s->gomap(mapid, type, pPos == nullptr ? xPos() : *pPos);
            MsgManager::sendMsg(s->id, 354, MsgParams(pUser->name));
          }
        }
      }
      break;
  }
  pUser->m_oSkillProcessor.clearTransInfo();
  return true;
}

// ------- 回城技能 -----------


// ------- 采集技能 -----------
CollectSkill::CollectSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

CollectSkill::~CollectSkill()
{

}

bool CollectSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  auto f = [&](const string& key, xLuaData& data)
  {
    if (data.getInt() != 0)
      m_setBuffIDs.insert(data.getInt());
  };
  xLuaData& paramd = params.getMutableData("Logic_Param");
  paramd.getMutableData("buffs").foreach(f);

  if (paramd.has("collect_type"))
  {
    DWORD dtype = paramd.getTableInt("collect_type");
    m_eCollectType = static_cast<ECollctSkillType>(dtype);
  }
  return true;
}

bool CollectSkill::run(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  if (oCmd.data().hitedtargets_size() == 0)
    return false;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(oCmd.data().hitedtargets(0).charid());
  if (npc == nullptr || npc->isAlive() == false || npc->isMonster())
    return false;

  switch(m_eCollectType)
  {
    case ECOLLECTSKILL_MIN:
      npc->attackMe(0 - npc->getAttr(EATTRTYPE_MAXHP), oRunner.getEntry());
      break;
    case ECOLLECTSKILL_MAGICBALL:
      {
        TeamPwsScene* pScene = dynamic_cast<TeamPwsScene*>(attacker->getScene());
        if (pScene == nullptr)
          return false;
        SceneUser* user = dynamic_cast<SceneUser*> (attacker);
        if (user == nullptr)
          return false;
        pScene->onUserCollectBall(user, npc);
      }
      break;
    case ECOLLECTSKILL_BUFFITEM:
      {
        TeamPwsScene* pScene = dynamic_cast<TeamPwsScene*>(attacker->getScene());
        if (pScene == nullptr)
          return false;
        SceneUser* user = dynamic_cast<SceneUser*> (attacker);
        if (user == nullptr)
          return false;

        pScene->onUserCollectBuff(user, npc);
      }
      break;
    default:
      return false;
  }

  for (auto buffid : m_setBuffIDs)
    attacker->m_oBuff.add(buffid, attacker);
  return true;
}
// ------- 采集技能 -----------

// ------- "召唤其他技能"类技能 -------
SummonSkill::SummonSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

SummonSkill::~SummonSkill()
{

}

bool SummonSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwSumSkillID = params.getData("Logic_Param").getTableInt("SkillID");
  m_dwRateType = params.getData("Logic_Param").getTableInt("RateType");

  auto f = [&](const string& key, xLuaData& data)
  {
    SOtherSkillItem item;
    item.dwSumSkillID = data.getTableInt("SkillID");
    item.dwRateType = data.getTableInt("RateType");
    m_vecOtherSkill.push_back(item);
  };
  params.getMutableData("Logic_Param").getMutableData("OtherSkills").foreach(f);
  return true;
}

bool SummonSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  if (BaseSkill::run(oRunner) == false)
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  if (oCmd.data().hitedtargets_size() == 0)
      return false;

  xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(0).charid());
  if (pEntry == nullptr || pEntry->isAlive() == false)
    return false;

  // summon another skill
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(m_dwSumSkillID);
  if (pSkillCFG && pSkillCFG->checkCanUse(attacker))
  {
    DWORD rate = LuaManager::getMe().call<DWORD> ("calcFormulaValue", attacker, pEntry, m_dwRateType);
    DWORD randrate = randBetween(1, 100);
    if (randrate <= rate)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (attacker);
      if (user && user->getFighter())
        user->getFighter()->getSkill().addTempSkill(m_dwSumSkillID, true);
      if (attacker->useSkill(m_dwSumSkillID, pEntry->id, attacker->getPos()))
        return true;
    }
  }
  
  for(auto& item: m_vecOtherSkill)
  {
    const BaseSkill* pOtherSkillCFG = SkillManager::getMe().getSkillCFG(item.dwSumSkillID);
    if (pOtherSkillCFG && pOtherSkillCFG->checkCanUse(attacker))
    {
      DWORD itemRate = LuaManager::getMe().call<DWORD> ("calcFormulaValue", attacker, pEntry, item.dwRateType);
      DWORD itemRandrate = randBetween(1, 100);
      if (itemRandrate <= itemRate)
      {
        SceneUser* user = dynamic_cast<SceneUser*> (attacker);
        if (user && user->getFighter())
          user->getFighter()->getSkill().addTempSkill(item.dwSumSkillID, true);
        if (attacker->useSkill(item.dwSumSkillID, pEntry->id, attacker->getPos()))
          return true;
      }
    }
  }

  return true;
}
// ------- "召唤其他技能"类技能 -------

// ------- "自杀攻击技能" -------
SuicideSkill::SuicideSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

SuicideSkill::~SuicideSkill()
{

}

bool SuicideSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  // suicide
  SceneNpc* pNpc = dynamic_cast<SceneNpc*>(attacker);
  if (pNpc != nullptr && pNpc->isAlive())
  {
    pNpc->setStatus(ECREATURESTATUS_SUICIDE);
  }
  else
  {
    attacker->changeHp(0 - attacker->getAttr(EATTRTYPE_MAXHP), attacker);
  }
  return true;
}
// ------- "自杀攻击技能" -------

// --------  闪光灯技能 ------------
FlashSkill::FlashSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

FlashSkill::~FlashSkill()
{

}

bool FlashSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  ghostBuff = params.getData("Buff").getData("ghost").getTableInt("1");
  demonBuff = params.getData("Buff").getData("demon").getTableInt("1");
  return true;
}

bool FlashSkill::run(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  SceneUser* pAttacker = dynamic_cast<SceneUser*>(attacker);
  if (pAttacker == nullptr)
    return false;

  //拍照统计
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_PHOTO_COUNT, 0, 0, pAttacker->getLevel(), (DWORD)1);
  pAttacker->getShare().addNormalData(ESHAREDATATYPE_S_PHOTOCOUNT, 1);
  pAttacker->getAchieve().onPhoto(EACHIEVECOND_PHOTO);

  Cmd::PhaseData *pData = oCmd.mutable_data();
  PhaseData data(oCmd.data());
  pData->clear_hitedtargets();
  bool bGhostPhoto = false;
  TSetQWORD setIDs;
  for (int i = 0; i < data.hitedtargets_size(); ++i)
  {
    setIDs.insert(data.hitedtargets(i).charid());
    QWORD tarid = data.hitedtargets(i).charid();
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(tarid);
    if (npc == nullptr)
      continue;

    // 幽灵现身10s, 受到百分比伤害
    if (pAttacker->isShootGhost())
    {
      // 隐匿中
      if (npc->m_ai.isGhost() && npc->getAttr(EATTRTYPE_HIDE) > 0)
      {
        npc->m_oBuff.add(ghostBuff, attacker, 0);

        if (npc->m_ai.isAttackGhost())
          npc->attackMe(0, attacker);
        if (pAttacker->isCameraDizzy())
        {
          npc->m_oBuff.add(demonBuff, attacker, 0);
        }
        if (!bGhostPhoto)
        {
          pAttacker->getAchieve().onPhoto(EACHIEVECOND_GHOST_PHOTO);
          bGhostPhoto = true;
        }
      }
      npc->m_oBuff.onBeShoot();
    }
    // 恶魔眩晕5秒
    if (pAttacker->isCameraDizzy())
    {
      if (npc->m_ai.isDemon())
      {
        npc->m_oBuff.add(demonBuff, attacker, 0);
      }
    }

    // unlock attr
    if (pAttacker != nullptr)
    {
      pAttacker->getManual().onPhoto(npc->getNpcID());
      pAttacker->getManual().onPhotoProcess(npc->getNpcID());

      const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(npc->getNpcID());
      if(pCFG != nullptr && pCFG->vecRelevancyIDs.empty() == false)
      {
        for(auto m = pCFG->vecRelevancyIDs.begin(); m != pCFG->vecRelevancyIDs.end(); ++m)
        {
          pAttacker->getManual().onPhoto(*m);
          pAttacker->getManual().onPhotoProcess(*m);
        }
      }
    }

    npc->m_sai.checkSig("camera_shot");
  }
  /*oCmd.mutable_data()->set_number(ESKILLNUMBER_RET);
  oCmd.set_charid(0);

  // send skill run
  PROTOBUF(oCmd, send, len);
  attacker->sendCmdToNine(send, len);
  */

  /*for (int i = 0; i < pData->hitedtargets_size(); ++i)
  {
    QWORD tarid = pData->hitedtargets(i).charid();
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(tarid);
    if (npc == nullptr)
      continue;
    npc->attackMe(pData->hitedtargets(i).damage(), attacker);
  }
  */
  oRunner.setCount(oRunner.getCount() + 1);

  pAttacker->getAchieve().onPhoto(EACHIEVECOND_PHOTO_MAN, setIDs);
  pAttacker->getAchieve().onPhoto(EACHIEVECOND_PHOTO_MONSTER, setIDs);
  pAttacker->getQuest().onPhoto(setIDs);
  return true;
}
// --------  闪光灯技能 ------------


// --------  装死技能 --------------
FakeDeadSkill::FakeDeadSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

FakeDeadSkill::~FakeDeadSkill()
{

}

bool FakeDeadSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  if (attacker->getStatus() == ECREATURESTATUS_FAKEDEAD)
  {
    attacker->setStatus(ECREATURESTATUS_LIVE);
    attacker->refreshDataAtonce();
    oRunner.setCount(oRunner.getCount() + 1);
  }
  else
  {
    // setstatus put before addbuff
    attacker->setStatus(ECREATURESTATUS_FAKEDEAD);
    attacker->refreshDataAtonce();

    if (BaseSkill::run(oRunner) == false)
      return false;

    if (SCENE_ENTRY_USER==attacker->getEntryType())
    {
      SceneUser *pUser = (SceneUser *)attacker;
      pUser->lockMeCheckEmoji("EnemyPretendDie");
      pUser->getAchieve().onUseSkill(getFamilyID());
    }
  }
  return true;
}
// --------  装死技能 --------------


// -------- 驱逐回城技能 ----------
ExpelSkill::ExpelSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

ExpelSkill::~ExpelSkill()
{

}

bool ExpelSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  if (oCmd.data().hitedtargets_size() != 1)
    return false;

  QWORD targetid = oCmd.data().hitedtargets(0).charid();
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(targetid);
  if (pUser == nullptr)
    return false;

  if (pUser->getStatus() == ECREATURESTATUS_DEAD)
  {
    pUser->relive(ERELIVETYPE_RETURN);

    if (attacker->getEntryType() == SCENE_ENTRY_USER)
    {
      ((SceneUser*)(attacker))->getUserGvg().onExpelOther();
    }
  }
  else if (pUser->getStatus() == ECREATURESTATUS_FAKEDEAD)
  {
    DWORD mapid = attacker->getScene()->base->getReliveMapID();
    pUser->gomap(mapid, GoMapType::Skill);
    pUser->setStatus(ECREATURESTATUS_LIVE);
    pUser->refreshDataAtonce();
  }

  return true;
}
// -------- 驱逐回城技能 ----------

// -------- 复活术 -------- 
ReBirthSkill::ReBirthSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

ReBirthSkill::~ReBirthSkill()
{

}

bool ReBirthSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_bIgnoreGVGCd = params.getData("Logic_Param").getTableInt("ignore_gvg_cd") == 1;
  return true;
}

bool ReBirthSkill::canUseToTarget(xSceneEntryDynamic* attacker, const SkillBroadcastUserCmd& oCmd) const
{
  if (BaseSkill::canUseToTarget(attacker, oCmd) == false)
    return false;

  if (checkAttacker(attacker) == false)
    return false;

  if (m_eLogic == ESKILLLOGIC_LOCKEDTARGET)
  {
    if (oCmd.data().hitedtargets_size() != 1)
      return false;
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(oCmd.data().hitedtargets(0).charid());
    if (!pUser || pUser->isAlive()) return false;
    if (pUser->inReliveStatus()) return false;
  }

  return true;
}

bool ReBirthSkill::prepare(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  if (m_eLogic == ESKILLLOGIC_LOCKEDTARGET && oRunner.getCmd().data().hitedtargets_size() == 1)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(oRunner.getCmd().data().hitedtargets(0).charid());
    if (!pUser || pUser->isAlive()) return false;
    pUser->addReliveMeUser(attacker->id);
  }
  sendSkillStatusChant(oRunner);
  return true;
}

void ReBirthSkill::onBeBreak(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker == nullptr)
    return;
  if (m_eLogic == ESKILLLOGIC_LOCKEDTARGET && oRunner.getCmd().data().hitedtargets_size() == 1)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(oRunner.getCmd().data().hitedtargets(0).charid());
    if (!pUser) return;
    pUser->delReliveMeUser(attacker->id);
  }
}

bool ReBirthSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  Scene* pScene = attacker->getScene();
  if (pScene == nullptr)
    return false;
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  collectTarget(oRunner);

  PhaseData data(oCmd.data());
  oCmd.mutable_data()->clear_hitedtargets();
  // attack targets
  PhaseData* pData = oCmd.mutable_data();

  SceneUser* reliver = dynamic_cast<SceneUser*> (attacker);
  for (int i = 0; i < data.hitedtargets_size(); ++i)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(data.hitedtargets(i).charid());
    if (pUser == nullptr || pUser->getStatus() != ECREATURESTATUS_DEAD)
      continue;
    if (attacker->isMyEnemy(pUser))
      continue;

    if (m_bIgnoreGVGCd == false && (pScene->isGvg() || pScene->getSceneType() == SCENE_TYPE_MVPBATTLE || pScene->getSceneType() == SCENE_TYPE_TEAMPWS))
    {
      DWORD relivecd = pUser->getReliveCD();
      if (relivecd && now() < pUser->getDieTime() + relivecd)
      {
        pUser->setWaitRelive(attacker->id, pUser->getDieTime() + relivecd);
        pData->add_hitedtargets()->CopyFrom(data.hitedtargets(i));
        DieTimeCountEventCmd cmd;
        cmd.set_time(pUser->getDieTime() + relivecd - now());
        cmd.set_name(attacker->name);
        PROTOBUF(cmd, send2, len2);
        pUser->sendCmdToMe(send2, len2);

        pUser->setDataMark(EUSERDATATYPE_STATUS);
        pUser->refreshDataAtonce();
        continue;
      }
    }
    pUser->delReliveMeUser(attacker->id);
    pUser->relive(ERELIVETYPE_SKILL, reliver);
    pData->add_hitedtargets()->CopyFrom(data.hitedtargets(i));
  }
  addBuff(oRunner);
  sendSkillStatusRun(oRunner);

  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}
// -------- 复活术 --------

// ------- NPC 陷阱类技能 -----------
TrapSkill::TrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

TrapSkill::~TrapSkill()
{

}

bool TrapSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwTrapSkillID = params.getData("Logic_Param").getTableInt("skillid");
  if (m_dwTrapSkillID == 0)
    return false;
  return true;
}

bool TrapSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  //attacker->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);

  if (oRunner.getTrap() == false)
  {
    if (trap(oRunner) == false)
      return false;
  }

  return trapfire(oRunner);
}

bool TrapSkill::trap(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  xPos po;
  if (oCmd.data().has_pos() == false)
  {
    po = attacker->getPos();
  }
  else
  {
    po.x = oCmd.data().pos().x() / 1000.0f;
    po.y = oCmd.data().pos().y() / 1000.0f;
    po.z = oCmd.data().pos().z() / 1000.0f;
  }

  NpcDefine def;
  def.setID(m_stLogicParam.dwTrapNpcID);
  def.resetting();
  //def.setBehaviours(BEHAVIOUR_NOT_SKILL_SELECT);
  def.setSearch(0);
  def.setRange(0);
  def.setPos(po);
  def.m_oVar.m_qwOwnerID = attacker->id;

  SceneNpc *pNpc = SceneNpcManager::getMe().createNpc(def, attacker->getScene());
  if (pNpc == nullptr)
    return false;
  TrapNpc* pTrapNpc = dynamic_cast<TrapNpc*>(pNpc);
  if (pTrapNpc == nullptr)
    return false;
  pTrapNpc->setSkillMasterID(attacker->id);
  pTrapNpc->addTrapSeeUser(attacker);
  // protect
  pTrapNpc->setClearTime(100 + now());

  oRunner.setParam1(pNpc->id);
  oCmd.mutable_data()->set_number(ESKILLNUMBER_ATTACK);
  oCmd.mutable_data()->clear_hitedtargets();
  sendSkillStatusRun(oRunner);

  oRunner.setTrap(true);
  return true;
}

bool TrapSkill::trapfire(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  // 判断伤害类陷阱 不会被目标引爆
  if (attacker->m_oSkillProcessor.isTrapHoldOn())
  {
    const BaseSkill* pRealSkill = SkillManager::getMe().getSkillCFG(m_dwTrapSkillID);
    if (pRealSkill && pRealSkill->haveDamage())
      return true;
  }

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  collectTarget(oRunner);

  PhaseData data(oCmd.data());
  oCmd.mutable_data()->clear_hitedtargets();
  PhaseData* pData = oCmd.mutable_data();
  for (int i = 0; i < data.hitedtargets_size(); ++i)
  {
    xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(data.hitedtargets(i).charid());
    if (pEntry == nullptr || pEntry->isAlive() == false)
      continue;
    if (m_stLogicParam.m_bImmunedBySuspend && pEntry->isSuspend())
      continue;
    if (!m_stLogicParam.m_bNotImmunedByFieldArea && pEntry->isImmuneTrap())
      continue;
    //if (attacker->isMyEnemy(pEntry) == false || attacker->canAttack(pEntry) == false)
      //continue;
    pData->add_hitedtargets()->CopyFrom(data.hitedtargets(i));
  }

  if (pData->hitedtargets_size() == 0 && !oRunner.isTrapTriggered())
    return true;

  TrapNpc* mattacker = dynamic_cast<TrapNpc*>(SceneNpcManager::getMe().getNpcByTempID(oRunner.getParam1()));
  if (mattacker == nullptr)
    return false;
  mattacker->setTrapSeeAll();
  mattacker->sendMeToNine();

  xPos oPos;
  oPos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());

  if (pData->hitedtargets_size() != 0)
  {
    if (mattacker->useSkill(m_dwTrapSkillID, pData->hitedtargets(0).charid(), oPos) == false)
      return false;

    xSceneEntryDynamic* pememy = xSceneEntryDynamic::getEntryByID(pData->hitedtargets(0).charid());
    if (pememy != nullptr && pememy->getMoveSpeed() > 0)
    {
      bool canmove = true;
      if (pememy->getEntryType() == SCENE_ENTRY_NPC && ((SceneNpc*)pememy)->m_ai.isMoveable() == false)
        canmove = false;
      if (canmove)
        pememy->goTo(oPos);
    }
  }
  else if (oRunner.isTrapTriggered())
  {
    if (mattacker->useSkill(m_dwTrapSkillID, 0, oPos) == false)
      return false;
  }

  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}
// ------- NPC 陷阱类技能 -----------


// ------ 反击技能 ---------
BeatBackSkill::BeatBackSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

BeatBackSkill::~BeatBackSkill()
{

}

bool BeatBackSkill::prepare(SkillRunner& oRunner) const
{
  addBuff(oRunner);
  sendSkillStatusChant(oRunner);
  return true;
}

bool BeatBackSkill::run(SkillRunner& oRunner) const
{
  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  for (auto &v : m_selfbuffs)
  {
    attacker->m_oBuff.del(v);
  }
  return true;
}

void BeatBackSkill::onBeBreak(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return;

  auto delBuffAtonce = [&]()
  {
    for (auto &v : m_selfbuffs)
      attacker->m_oBuff.del(v);
  };

  // breaker == 0表示非近战普攻技能打断
  if (oRunner.getBreaker() == 0)
  {
    XLOG << "[反击技能], 技能结束, 非近战普攻技能打断, 攻击者:" << attacker->id << XEND;
    delBuffAtonce();
    return;
  }
  xSceneEntryDynamic* breaker = xSceneEntryDynamic::getEntryByID(oRunner.getBreaker());
  if (breaker == nullptr || breaker->getScene() == nullptr || breaker->isAlive() == false)
  {
    delBuffAtonce();
    return;
  }

  // get normalskill
  DWORD normalskillid = 0;
  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser)
  {
    const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(pUser->getUserSceneData().getProfession());
    if (pCFG == nullptr)
      return;
    normalskillid = pCFG->normalSkill;
  }
  else
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (attacker);
    if (npc == nullptr)
      return;
    normalskillid = npc->getNormalSkill();
  }

  if (normalskillid == 0)
    return;

  // 使用普攻反击
  if (pUser)
    attacker->m_oSkillProcessor.useBuffSkill(attacker, breaker, normalskillid, true);
  else
    attacker->useSkill(normalskillid, breaker->id, breaker->getPos());

  XDBG << "[反击技能], 技能生效, 攻方:" << attacker->id << ", 敌方:" << breaker->id << XEND;
}
// ------ 反击技能 ---------

// ------ 暗之屏障 --------
TrapBarrierSkill::TrapBarrierSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

TrapBarrierSkill::~TrapBarrierSkill()
{

}

bool TrapBarrierSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;
  SceneTrap* pTrap = SceneTrapManager::getMe().getSceneTrap(oRunner.getParam1());
  if (pTrap == nullptr)
    return false;
  oRunner.setCount(pTrap->getCount());
  return true;
}

void TrapBarrierSkill::end(SkillRunner& oRunner) const
{
  BaseSkill::end(oRunner);

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (checkAttacker(attacker) == false)
    return;

  xSceneEntrySet friendSet, enemySet, teamSet;
  set<QWORD>& qwSet = oRunner.getBuffTargets();
  for (auto &qw : qwSet)
  {
    xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(qw);
    if (pEntry == nullptr)
      continue;

    else if (attacker->isMyEnemy(pEntry))
    {
      enemySet.insert(pEntry);
    }
    else
    {
      if (attacker->isMyTeamMember(qw))
        teamSet.insert(pEntry);
      else
        friendSet.insert(pEntry);
    }
  }

  for (auto m = m_selfSkillBuffs.begin(); m != m_selfSkillBuffs.end(); ++m)
    attacker->m_oBuff.del(*m);

  for (auto m = m_enemybuffs.begin(); m != m_enemybuffs.end(); ++m)
  {
    for (auto iter = enemySet.begin(); iter != enemySet.end(); ++iter)
    {
      ((xSceneEntryDynamic *)(*iter))->m_oBuff.del(*m);
    }
  }
  for (auto m = m_teambuffs.begin(); m != m_teambuffs.end(); ++m)
  {
    for (auto iter = teamSet.begin(); iter != teamSet.end(); ++iter)
    {
      ((xSceneEntryDynamic *)(*iter))->m_oBuff.del(*m);
    }
  }
  for (auto m = m_friendbuffs.begin(); m != m_friendbuffs.end(); ++m)
  {
    for (auto iter = friendSet.begin(); iter != friendSet.end(); ++iter)
    {
      ((xSceneEntryDynamic *)(*iter))->m_oBuff.del(*m);
    }
  }
}
// ------ 暗之屏障 --------

// ------ 引爆技能 --------
TriggerTrapSkill::TriggerTrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

TriggerTrapSkill::~TriggerTrapSkill()
{

}

bool TriggerTrapSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false){
    return false;
  }
  xLuaData& logicParam = params.getMutableData("Logic_Param");
  xLuaData& trapnpcids = logicParam.getMutableData("trapnpcid");
  auto getid = [&](const string& key, xLuaData& data){
    m_setNpcid.insert(data.getInt());
  };
  trapnpcids.foreach(getid);
  return true;
}

bool TriggerTrapSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (checkAttacker(attacker) == false)
    return false;

  DWORD rune1 = m_stLogicParam.m_oParam.getTableInt("rune1");
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user && user->getRuneSpecNum(rune1) != 0)
  {
    DWORD rune1buff = m_stLogicParam.m_oParam.getTableInt("rune1_buff");
    if (attacker->m_oSkillProcessor.isTrapHoldOn() == false)
    {
      attacker->m_oSkillProcessor.setTrapHoldOn(true);
      attacker->m_oBuff.add(rune1buff);
      return true;
    }
    else
    {
      attacker->m_oBuff.del(rune1buff);
      attacker->m_oSkillProcessor.setTrapHoldOn(false);

      // 若set中为空则触发全部陷阱
      triggerTraps(attacker, m_setNpcid.empty());
      return true;
    }
  }

  // 若set中为空则触发全部陷阱
  triggerTraps(attacker, m_setNpcid.empty());
  return true;
}
void TriggerTrapSkill::triggerTraps(xSceneEntryDynamic* attacker, bool isAll /*= true*/) const
{
// 立即刷新buff属性, 避免伤害计算时, buff属性尚未生效
  attacker->m_oBuff.update(xTime::getCurMSec());
  attacker->updateAttribute();
  if(isAll){
    attacker->m_oSkillProcessor.triggerAllTrap();
  }
  else{
    attacker->m_oSkillProcessor.triggerTrapByType(m_setNpcid);
  }
}
// ------ 偷窃技能 ---------
StealSkill::StealSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

StealSkill::~StealSkill()
{

}

bool StealSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  xLuaData& itemdata = params.getMutableData("Logic_Param").getMutableData("steal");
  auto getitem = [&](const string& key, xLuaData& data)
  {
    pair<DWORD, DWORD> pa;
    pa.first = data.getTableInt("item");
    pa.second = data.getTableInt("count");
    m_vecItem2Count.push_back(pa);
  };
  itemdata.foreach(getitem);
  m_dwExpression = params.getMutableData("Logic_Param").getTableInt("expression");
  return true;
}

bool StealSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  MonsterNpc* npc = dynamic_cast<MonsterNpc*> (attacker);
  if (!npc)
    return false;

  if (oRunner.getCmd().data().hitedtargets_size() != 1)
    return false;
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(oRunner.getCmd().data().hitedtargets(0).charid());
  if (!pUser)
    return false;
  BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
    return false;

  bool find = false;
  for (auto &p : m_vecItem2Count)
  {
    DWORD count = pPack->getItemCount(p.first);
    if (count != 0)
    {
      count = p.second <= count ? p.second : count;
      pPack->reduceItem(p.first, ESOURCE_MONSTERAI, count);
      npc->addStealItem(p.first, count);
      find = true;
      break;
    }
  }
  oRunner.setCount(oRunner.getCount() + 1);

  if (!find)
  {
    oRunner.getCmd().mutable_data()->mutable_hitedtargets(0)->set_type(DAMAGE_TYPE_MISS);
    sendSkillStatusRun(oRunner);
    return true;
  }

  npc->playEmoji(m_dwExpression);
  oRunner.getCmd().mutable_data()->mutable_hitedtargets(0)->set_type(DAMAGE_TYPE_NORMAL);
  sendSkillStatusRun(oRunner);
  return true;
}

FunctionSkill::FunctionSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

FunctionSkill::~FunctionSkill()
{

}

GroupBirthSkill::GroupBirthSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

GroupBirthSkill::~GroupBirthSkill()
{

}

bool GroupBirthSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  Scene* pScene = attacker->getScene();
  if (pScene == nullptr)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (pUser->getTeamID() == 0)
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  PhaseData data(oCmd.data());
  oCmd.mutable_data()->clear_hitedtargets();
  // attack targets
  PhaseData* pData = oCmd.mutable_data();
  std::set<SceneUser*> teamset = pUser->getTeamSceneUser();
  for (auto s = teamset.begin(); s != teamset.end(); ++s)
  {
    if ((*s)->isAlive())
      continue;
    if (getXZDistance(pUser->getPos(), (*s)->getPos()) > m_stLogicParam.fRange)
      continue;

    pData->add_hitedtargets()->set_charid((*s)->id);
    oRunner.getBuffTargets().insert((*s)->id);

    if (pScene->isGvg() || pScene->getSceneType() == SCENE_TYPE_MVPBATTLE || pScene->getSceneType() == SCENE_TYPE_TEAMPWS)
    {
      DWORD relivecd = (*s)->getReliveCD();
      if (relivecd && now() < (*s)->getDieTime() + relivecd)
      {
        (*s)->setWaitRelive(attacker->id, (*s)->getDieTime() + relivecd);
        DieTimeCountEventCmd cmd;
        cmd.set_time((*s)->getDieTime() + relivecd - now());
        cmd.set_name(attacker->name);
        PROTOBUF(cmd, send2, len2);
        (*s)->sendCmdToMe(send2, len2);

        (*s)->setDataMark(EUSERDATATYPE_STATUS);
        (*s)->refreshDataAtonce();
        continue;
      }
    }
    (*s)->relive(ERELIVETYPE_SKILL, pUser);
    xPos pos = pUser->getPos();
    pUser->getScene()->getRandPos(pos, 5, pos);
    (*s)->goTo(pos);
  }
  addBuff(oRunner);
  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}


MarkHealSkill::MarkHealSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

MarkHealSkill::~MarkHealSkill()
{

}

bool MarkHealSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  collectTarget(oRunner);

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  PhaseData data(oCmd.data());

  DWORD skillid = m_stLogicParam.m_oParam.getTableInt("skillid");
  if (skillid == 0)
    return false;
  attacker->clearExtraSkillTarget(skillid);
  for (int i = 0; i < data.hitedtargets_size(); ++i)
  {
    attacker->setExtraSkillTarget(data.hitedtargets(i).charid(), skillid);
    break;
  }
  addBuff(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);

  return true;
}

LeadSkill::LeadSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

LeadSkill::~LeadSkill()
{

}

bool LeadSkill::prepare(SkillRunner& oRunner) const
{
  if (BaseSkill::prepare(oRunner) == false)
    return false;
  return true;
}

bool LeadSkill::chant(SkillRunner& oRunner) const
{
  // check interval
  if (m_stLogicParam.qwInterval)
  {
    QWORD curMSec = xTime::getCurMSec();
    if (curMSec < oRunner.getNextChantTime())
      return true;
    oRunner.setNextChantTime(curMSec + m_stLogicParam.qwInterval);
  }

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  auto breakSelf = [&]()
  {
    SkillBroadcastUserCmd cmd;
    cmd.set_skillid(m_dwSkillID);
    cmd.set_charid(attacker->id);
    cmd.mutable_data()->set_number(ESKILLNUMBER_BREAK);
    cmd.set_petid(-1);
    PROTOBUF(cmd, send, len);
    attacker->sendCmdToNine(send, len);
  };

  // 引导目标离开施法范围
  if (m_eLogic == ESKILLLOGIC_LOCKEDTARGET)
  {
    if (oRunner.getCmd().data().hitedtargets_size() != 1)
      return false;
    QWORD tarid = oRunner.getCmd().data().hitedtargets(0).charid();
    xSceneEntryDynamic* pTar = xSceneEntryDynamic::getEntryByID(tarid);
    if (pTar == nullptr || pTar->getScene() != attacker->getScene())
    {
      breakSelf();
      return false;
    }
    if (getXZDistance(attacker->getPos(), pTar->getPos()) > m_dwLaunchRange * 1.5)
    {
      breakSelf();
      return false;
    }
  }

  //BaseSkill::run(oRunner);
  // 获得技能目标
  if (haveNoTargets() == false)
  {
    if (collectTarget(oRunner) == false)
      return false;
  }

  // 计算伤害
  if (m_bHaveDamage)
  {
    if (collectDamage(oRunner) == false)
      return false;
    sendSkillStatusHit(oRunner);
  }

  if (runEffect(oRunner) == false)
    return false;

  // 添加buff
  bool haveBuffEffect = false;
  if (m_bHaveBuff)
  {
    haveBuffEffect = addBuff(oRunner);
  }

  if (isTimeTrap())
  {
    if (haveBuffEffect || oRunner.getCmd().data().hitedtargets_size() != 0)
      oRunner.setCount(oRunner.getCount() + 1);
  }
  else
  {
    oRunner.setCount(oRunner.getCount() + 1);
  }

  return true;
}

bool LeadSkill::run(SkillRunner& oRunner) const
{
  return false; // 引导技能此阶段直接跳过
}

void LeadSkill::end(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker == nullptr)
    return;
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  oCmd.mutable_data()->set_number(ESKILLNUMBER_CAIJI);
  PROTOBUF(oCmd, send, len);
  attacker->sendCmdToNine(send, len);
}

TrapNpcSkill::TrapNpcSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{
}

TrapNpcSkill::~TrapNpcSkill()
{
}

bool TrapNpcSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  string str = m_stLogicParam.m_oParam.getTableString("function_type");
  if (str == "Bi_Transport")
  {
    m_eFuncType = ETRAPNPCFUNC_BTRANS;
    m_stLogicParam.qwInterval = m_stLogicParam.qwInterval ? m_stLogicParam.qwInterval : 500; // 设置默认运行间隔
  }

  return true;
}

bool TrapNpcSkill::trap(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  xPos po;
  if (oCmd.data().has_pos() == false)
  {
    po = attacker->getPos();
  }
  else
  {
    po.x = oCmd.data().pos().x() / 1000.0f;
    po.y = oCmd.data().pos().y() / 1000.0f;
    po.z = oCmd.data().pos().z() / 1000.0f;
  }

  NpcDefine def;
  def.setID(m_stLogicParam.dwTrapNpcID);
  def.resetting();
  def.setSearch(0);
  def.setRange(0);
  def.setPos(po);
  def.setTerritory(0);
  def.m_oVar.m_qwOwnerID = attacker->id;

  switch(m_eFuncType)
  {
    case ETRAPNPCFUNC_MIN:
      break;
    case ETRAPNPCFUNC_BTRANS:
      def.m_oVar.m_qwTeamUserID = attacker->id; // 队友可见
      break;
    default:
      break;
  }
  SceneNpc *pNpc = SceneNpcManager::getMe().createNpc(def, attacker->getScene());
  if (pNpc == nullptr)
    return false;
  pNpc->setClearTime(100 + now());

  SkillNpc* pSkillNpc = dynamic_cast<SkillNpc*>(pNpc);
  if (pSkillNpc == nullptr)
    return false;
  pSkillNpc->setMasterUser(attacker->id);
  pSkillNpc->setRelatedSkill(m_dwSkillID);

  oRunner.setParam1(pNpc->id);
  oCmd.mutable_data()->set_number(ESKILLNUMBER_ATTACK);
  oCmd.mutable_data()->clear_hitedtargets();
  sendSkillStatusRun(oRunner);

  oRunner.setTrap(true);
  return true;
}

bool TrapNpcSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  if (oRunner.getTrap() == false)
  {
    if (trap(oRunner) == false)
      return false;
  }
  /*
  switch(m_eFuncType)
  {
    case ETRAPNPCFUNC_MIN:
      break;
    case ETRAPNPCFUNC_BTRANS:
      {
        Scene* pScene = attacker->getScene();
        if (pScene == nullptr)
          break;
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(oRunner.getParam1());
        if (npc == nullptr || npc->getScene() != pScene || npc->isAlive() == false)
          return false;
        SceneUser* user = dynamic_cast<SceneUser*> (attacker);
        if (user == nullptr)
          break;
        std::set<SceneUser*> teamset = user->getTeamSceneUser();
        if (teamset.empty()) teamset.insert(user);
        std::set<SceneUser*> targets;
        for (auto &s : teamset)
        {
          if (getXZDistance(npc->getPos(), s->getPos()) < m_stLogicParam.fRange)
            targets.insert(s);
        }
        if (targets.empty())
          break;

        TSetQWORD npcids;
        attacker->m_oSkillProcessor.getTrapNpcs(m_dwSkillID / ONE_THOUSAND, npcids);
        npcids.erase(npc->id);

        SceneNpc* pCoupleNpc = nullptr;
        for (auto &q : npcids)
        {
          SceneNpc* p = SceneNpcManager::getMe().getNpcByTempID(q);
          if (p == nullptr)
            continue;
          if (pCoupleNpc != nullptr && pCoupleNpc->getBirthTime() > p->getBirthTime())
            continue;
          pCoupleNpc = p;
        }

        if (pCoupleNpc == nullptr || pCoupleNpc->getScene() != pScene)
          break;
        xPos pos1 = npc->getPos(); // 当前传送阵位置
        xPos pos2 = pCoupleNpc->getPos(); // 目标传送阵位置
        xPos pos;
        int tmpcnt = 0;
        bool found = false;
        while (tmpcnt++ < 30)
        {
          pScene->getCircleRoundPos(pos2, m_stLogicParam.fRange + 1, pos);
          if (getXZDistance(pos1, pos) > m_stLogicParam.fRange)
          {
            found = true;
            break;
          }
        }
        if (!found) break;
        for (auto &s : targets)
          s->goTo(pos);
      }
      break;
    default:
      break;
  }

*/
  return true;
}

void TrapNpcSkill::end(SkillRunner& oRunner) const
{
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(oRunner.getParam1());
  if (npc && npc->getNpcType() == ENPCTYPE_SKILLNPC)
    npc->removeAtonce();
}

void TrapNpcSkill::onTriggerNpc(SkillRunner& oRunner, SceneUser* user) const
{
  if (user == nullptr)
    return;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return;

  switch(m_eFuncType)
  {
    case ETRAPNPCFUNC_MIN:
      break;
    case ETRAPNPCFUNC_BTRANS:
      {
        Scene* pScene = attacker->getScene();
        if (pScene == nullptr)
          break;
        if (attacker->isMyTeamMember(user->id) == false || user->getScene() != pScene)
          break;

        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(oRunner.getParam1());
        if (npc == nullptr || npc->getScene() != pScene || npc->isAlive() == false)
          return;
        if (getXZDistance(npc->getPos(), user->getPos()) > m_stLogicParam.fRange + 2)
        {
          xPos dest = user->m_oMove.getDestPos();
          if (getXZDistance(dest, npc->getPos()) > m_stLogicParam.fRange + 2 || getXZDistance(npc->getPos(), user->getPos()) > m_stLogicParam.fRange + 4)
          {
            XDBG << "[双向传送-距离非法], 传送阵位置:" << npc->getPos().x << npc->getPos().y << npc->getPos().z << "玩家位置:" << user->getPos().x
              << user->getPos().y << user->getPos().z << "距离:" << getXZDistance(npc->getPos(), user->getPos()) << XEND;
            break;
          }
        }

        TSetQWORD npcids;
        attacker->m_oSkillProcessor.getTrapNpcs(m_dwSkillID / ONE_THOUSAND, npcids);
        npcids.erase(npc->id);

        SceneNpc* pCoupleNpc = nullptr;
        for (auto &q : npcids)
        {
          SceneNpc* p = SceneNpcManager::getMe().getNpcByTempID(q);
          if (p == nullptr)
            continue;
          if (pCoupleNpc != nullptr && pCoupleNpc->getBirthTime() > p->getBirthTime())
            continue;
          pCoupleNpc = p;
        }

        if (pCoupleNpc == nullptr || pCoupleNpc->getScene() != pScene)
          break;
        xPos pos1 = npc->getPos(); // 当前传送阵位置
        xPos pos2 = pCoupleNpc->getPos(); // 目标传送阵位置
        xPos pos;
        int tmpcnt = 0;
        bool found = false;
        while (tmpcnt++ < 30)
        {
          pScene->getCircleRoundPos(pos2, m_stLogicParam.fRange + 1, pos);
          if (getXZDistance(pos1, pos) > m_stLogicParam.fRange)
          {
            found = true;
            break;
          }
        }
        if (!found) break;
        user->goTo(pos);
        XDBG << "[双向传送], 释放者:" << attacker->name << attacker->id << "队友:" << user->name << user->id << "瞬移成功, npcid:" << npc->id << XEND;
      }
      break;
    default:
      break;
  }
}

SwordBreakSkill::SwordBreakSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

SwordBreakSkill::~SwordBreakSkill()
{

}

void SwordBreakSkill::onBeBreak(SkillRunner& oRunner) const
{
  if (oRunner.isValidBreak() == false)
    return;
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return;

  xSceneEntryDynamic* breaker = xSceneEntryDynamic::getEntryByID(oRunner.getBreaker());
  if (breaker == nullptr || breaker->getScene() != attacker->getScene())
    return;
  DWORD limitbuff = m_stLogicParam.m_oParam.getData("SkillStatus").getTableInt("limitSkillBuff");
  attacker->m_oSkillStatus.enterStatus(ESKILLSTATUS_SWORDBREAK, TSetSceneEntrys{breaker}, this);
  attacker->m_oBuff.add(limitbuff, breaker, m_dwSkillID);
  for (auto &s : m_selfbuffs)
    attacker->m_oBuff.add(s, attacker, m_dwSkillID);
  for (auto &s : m_enemybuffs)
    breaker->m_oBuff.add(s, attacker, m_dwSkillID);
}

bool SwordBreakSkill::run(SkillRunner& oRunner) const
{
  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

ShowSkill::ShowSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

ShowSkill::~ShowSkill()
{

}

bool ShowSkill::run(SkillRunner& oRunner) const
{
  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

TouchPetSkill::TouchPetSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

TouchPetSkill::~TouchPetSkill()
{

}

bool TouchPetSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->isAlive() == false)
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();

  if (oCmd.data().hitedtargets_size() != 1)
    return false;
  QWORD targetid = oCmd.data().hitedtargets(0).charid();
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(targetid);
  if (npc == nullptr)
    return false;
  if (npc->getNpcType() != ENPCTYPE_PETNPC || npc->define.m_oVar.m_qwOwnerID != attacker->id)
    return false;

  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);

  user->getUserPet().onUserStopTouch();
  user->getUserPet().touch(npc->id);
  return true;
}

bool TouchPetSkill::prepare(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->isAlive() == false)
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();

  if (oCmd.data().hitedtargets_size() != 1)
    return false;
  QWORD targetid = oCmd.data().hitedtargets(0).charid();
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(targetid);
  if (npc == nullptr)
    return false;
  if (npc->getNpcType() != ENPCTYPE_PETNPC || npc->define.m_oVar.m_qwOwnerID != attacker->id)
    return false;

  Scene* pScene = npc->getScene();
  if (pScene == nullptr)
    return false;

  xPos pos;
  pos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());
  if (getXZDistance(pos, npc->getPos()) < 5 && pScene->getValidPos(pos))
    npc->goTo(pos);
    //npc->m_ai.moveTo(pos);
  user->getUserPet().onUserStartTouch(targetid);

  return BaseSkill::prepare(oRunner);
}

void TouchPetSkill::onBeBreak(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->isAlive() == false)
    return;
  user->getUserPet().onUserStopTouch();
}

void TouchPetSkill::end(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->isAlive() == false)
    return;
  user->getUserPet().onUserStopTouch();
}

PoliAttackSkill::PoliAttackSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

PoliAttackSkill::~PoliAttackSkill()
{

}

void PoliAttackSkill::runSpecEffect(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return;

  PollyScene* pScene = dynamic_cast<PollyScene*> (attacker->getScene());
  if (pScene == nullptr)
    return;

  Cmd::AddMapItem cmd;
  auto dropitem = [&](SceneUser* user, DWORD itemid)
  {
    if (itemid == 0)
      return;
    xPos pos = user->getPos();
    pScene->getRandPos(user->getPos(), 0.5, pos);
    ItemInfo item;
    item.set_id(itemid);
    item.set_count(1);
    SceneItem* pItem = SceneItemManager::getMe().createSceneItem(pScene, item, pos);
    if (pItem != nullptr)
    {
      pItem->fillMapItemData(cmd.add_items(), MiscConfig::getMe().getSceneItemCFG().dwDropInterval);
    }
  };
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  const SPoliFireCFG& rCFG = MiscConfig::getMe().getPoliFireCFG();
  for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
  {
    if (oCmd.data().hitedtargets(i).type() == DAMAGE_TYPE_MISS)
      continue;
    SceneUser* user = SceneUserManager::getMe().getUserByID(oCmd.data().hitedtargets(i).charid());
    if (user == nullptr)
      continue;

    DWORD dropnum = oCmd.data().hitedtargets(i).damage();
    DWORD nownum = pScene->getScore(user);
    if (dropnum > nownum)
      dropnum = nownum;

    // 掉落金苹果
    for (DWORD i = 0; i < dropnum; ++i)
      dropitem(user, rCFG.dwAppleItemID);

    // 金苹果数为0时, 掉落所有技能
    nownum = nownum - dropnum;
    pScene->setScore(user, nownum);
    pScene->markDropApple(user);
    if (nownum == 0)
    {
      TVecDWORD skills;
      user->getTransform().getPoliFireDropSkills(skills);
      for (auto &s : skills)
      {
        DWORD itemid = rCFG.getItemBySkill(s);
        if (itemid)
          dropitem(user, itemid);
      }
      user->getTransform().clearSkill();
    }

    oCmd.mutable_data()->mutable_hitedtargets(i)->set_damage(0);
  }

  if (cmd.items_size())
  {
    PROTOBUF(cmd, send, len);
    attacker->sendCmdToNine(send, len);
  }
}


BlinkSkill::BlinkSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

BlinkSkill::~BlinkSkill()
{

}

bool BlinkSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  sendSkillStatusRun(oRunner);

  xLuaData data;
  data.setData("effect", MiscConfig::getMe().getEffectPath().strTeleportEffect);
  data.setData("posbind", 1);
  GMCommandRuler::effect(attacker, data);

  Scene* pScene = attacker->getScene();
  if (pScene == nullptr)
    return false;
  xPos pos = attacker->getPos();
  EBlinkType etype = static_cast<EBlinkType> (m_stLogicParam.m_oParam.getData("blink").getTableInt("type"));
  switch(etype)
  {
    case EBLINKFORWARD:
      {
        SceneUser* user = dynamic_cast<SceneUser*> (attacker);
        if (user == nullptr)
          return false;
        float range = m_stLogicParam.m_oParam.getData("blink").getTableFloat("forward_dis");
        float dir = user->getUserSceneData().getDir() / ONE_THOUSAND;
        float radian = dir / 180.0f * 3.14;
        pos.x = pos.x + range * sin(radian);
        pos.z = pos.z + range * cos(radian);
        if (pScene->getValidPos(pos) == false)
          return false;
      }
      break;
  }

  attacker->goTo(pos);
  GMCommandRuler::effect(attacker, data);

  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

TrapMonsterSkill::TrapMonsterSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

TrapMonsterSkill::~TrapMonsterSkill()
{

}

bool TrapMonsterSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  Scene* pScene = attacker->getScene();
  if (pScene == nullptr)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr)
    return false;

  sendSkillStatusRun(oRunner);

  xPos summonpos = attacker->getPos();
  xLuaData effdata = m_stLogicParam.m_oParam.getData("summon");
  ESummonPosType etype = static_cast<ESummonPosType> (effdata.getTableInt("pos_type"));
  switch(etype)
  {
    case ESUMMONPOS_FORWARD:
      {
        float dis = effdata.getTableFloat("dis");
        float dir = user->getUserSceneData().getDir() / ONE_THOUSAND;
        float radian = dir / 180.0f * 3.14;
        summonpos.x = summonpos.x + dis * sin(radian);
        summonpos.z = summonpos.z + dis * cos(radian);
        if (pScene->getValidPos(summonpos) == false)
          return false;
      }
      break;
    case ESUMMONPOS_BACK:
      {
        float dis = effdata.getTableFloat("dis");
        float dir = user->getUserSceneData().getDir() / ONE_THOUSAND;
        float radian = dir / 180.0f * 3.14;
        summonpos.x = summonpos.x - dis * sin(radian);
        summonpos.z = summonpos.z - dis * cos(radian);
        if (pScene->getValidPos(summonpos) == false)
          return false;
      }
      break;
  }
  DWORD monsterid = effdata.getTableInt("monsterid");
  DWORD time = effdata.getTableInt("time");
  NpcDefine def;
  def.setID(monsterid);
  def.resetting();
  def.setBehaviours(BEHAVIOUR_NOT_SKILL_SELECT);
  def.setRange(0);
  def.setPos(summonpos);
  def.setDisptime(now() + time);
  SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, pScene);
  if (npc == nullptr)
    return false;
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

SummonBeingSkill::SummonBeingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{
}

SummonBeingSkill::~SummonBeingSkill()
{
}

bool SummonBeingSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  auto f = [&](const string& key, xLuaData& data)
  {
    if (data.getInt() != 0)
      m_setBeingIDs.insert(data.getInt());
  };
  params.getMutableData("Logic_Param").getMutableData("being_ids").foreach(f);
  return true;
}

bool SummonBeingSkill::run(SkillRunner& oRunner) const
{
  SceneUser* user = dynamic_cast<SceneUser*>(oRunner.getEntry());
  if (user == nullptr || user->isAlive() == false)
    return false;

  DWORD beingid = user->getUserSceneData().getSkillOptValue(ESKILLOPTION_SUMMONBEING);
  if (beingid == 0)
    return false;

  if (m_setBeingIDs.find(beingid) == m_setBeingIDs.end())
    return false;

  if (user->getUserBeing().summon(beingid) == false)
    return false;
  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

ReviveBeingSkill::ReviveBeingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{
}

ReviveBeingSkill::~ReviveBeingSkill()
{
}

bool ReviveBeingSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwHpPercent = params.getData("Logic_Param").getTableInt("hp_percent");
  return true;
}

bool ReviveBeingSkill::run(SkillRunner& oRunner) const
{
  SceneUser* user = dynamic_cast<SceneUser*>(oRunner.getEntry());
  if (user == nullptr || user->isAlive() == false)
    return false;

  if (user->getUserBeing().revive(m_dwHpPercent) == false)
    return false;
  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

HellPlantSkill::HellPlantSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{
}

HellPlantSkill::~HellPlantSkill()
{
}

bool HellPlantSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwTrapSkillID = params.getData("Logic_Param").getTableInt("skillid");
  if (m_dwTrapSkillID == 0)
    return false;
  return true;
}

bool HellPlantSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  attacker->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);

  if (oRunner.getTrap() == false)
  {
    if (trap(oRunner) == false)
      return false;
  }

  return trapfire(oRunner);
}

void HellPlantSkill::end(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (attacker == nullptr)
    return;

  if (oRunner.getParam1())
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(oRunner.getParam1());
    if (pNpc != nullptr)
    {
      if (oRunner.getCount() >= m_stLogicParam.dwHitTime)
        pNpc->setClearTime(now() + 1);
      else
        pNpc->setClearState();
    }
  }
}

bool HellPlantSkill::trap(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  xPos po;
  if (oCmd.data().has_pos() == false)
  {
    po = attacker->getPos();
  }
  else
  {
    po.x = oCmd.data().pos().x() / 1000.0f;
    po.y = oCmd.data().pos().y() / 1000.0f;
    po.z = oCmd.data().pos().z() / 1000.0f;
  }

  NpcDefine def;
  def.setID(m_stLogicParam.dwTrapNpcID);
  def.resetting();
  def.setSearch(0);
  def.setRange(0);
  def.setPos(po);
  def.setBehaviours(def.getBehaviours() & ~BEHAVIOUR_NOT_SKILL_SELECT);

  SceneNpc *pNpc = SceneNpcManager::getMe().createNpc(def, attacker->getScene());
  if (pNpc == nullptr)
    return false;
  TrapNpc* pTrapNpc = dynamic_cast<TrapNpc*>(pNpc);
  if (pTrapNpc == nullptr)
    return false;
  pTrapNpc->setTrapSeeAll();
  pTrapNpc->sendMeToNine();
  pTrapNpc->setSkillMasterID(attacker->id);
  // pTrapNpc->addTrapSeeUser(attacker);
  // protect
  pTrapNpc->setClearTime(100 + now());

  oRunner.setParam1(pNpc->id);
  oCmd.mutable_data()->set_number(ESKILLNUMBER_ATTACK);
  oCmd.mutable_data()->clear_hitedtargets();
  sendSkillStatusRun(oRunner);

  oRunner.setTrap(true);
  return true;
}

bool HellPlantSkill::trapfire(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  collectTarget(oRunner);

  PhaseData data(oCmd.data());
  oCmd.mutable_data()->clear_hitedtargets();
  PhaseData* pData = oCmd.mutable_data();
  for (int i = 0; i < data.hitedtargets_size(); ++i)
  {
    xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(data.hitedtargets(i).charid());
    if (pEntry == nullptr || pEntry->isAlive() == false)
      continue;
    if (m_stLogicParam.m_bImmunedBySuspend && pEntry->isSuspend())
      continue;
    if (!m_stLogicParam.m_bNotImmunedByFieldArea && pEntry->isImmuneTrap())
      continue;
    pData->add_hitedtargets()->CopyFrom(data.hitedtargets(i));
    break;
  }

  if (pData->hitedtargets_size() <= 0 && !oRunner.isTrapTriggered())
    return true;

  if (pData->hitedtargets_size() > 0)
    oRunner.setLockTargetID(pData->hitedtargets(0).charid());

  TrapNpc* mattacker = dynamic_cast<TrapNpc*>(SceneNpcManager::getMe().getNpcByTempID(oRunner.getParam1()));
  if (mattacker == nullptr)
    return false;

  xPos oPos;
  oPos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());

  if (pData->hitedtargets_size() != 0)
  {
    if (mattacker->useSkill(m_dwTrapSkillID, pData->hitedtargets(0).charid(), oPos) == false)
      return false;

    // xSceneEntryDynamic* pememy = xSceneEntryDynamic::getEntryByID(pData->hitedtargets(0).charid());
    // if (pememy != nullptr && pememy->getMoveSpeed() > 0)
    // {
    //   bool canmove = true;
    //   if (pememy->getEntryType() == SCENE_ENTRY_NPC && ((SceneNpc*)pememy)->m_ai.isMoveable() == false)
    //     canmove = false;
    //   if (canmove)
    //     pememy->goTo(oPos);
    // }
    oRunner.setCount(oRunner.getCount() + 1);
    return true;
  }

  return false;
}

BeingBuffSkill::BeingBuffSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{
}

BeingBuffSkill::~BeingBuffSkill()
{
}

bool BeingBuffSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;

  auto selff = [&](const string& key, xLuaData& data)
  {
    if (data.getInt() != 0)
      m_setSelfBuff.insert(data.getInt());
  };
  params.getMutableData("Logic_Param").getMutableData("self_buff").foreach(selff);

  auto beingf = [&](const string& key, xLuaData& data)
  {
    auto f = [&](const string& k, xLuaData& d)
    {
      DWORD monsterid = atoi(key.c_str()), buffid = d.getInt();
      if (monsterid && buffid)
        m_mapBeingBuff[monsterid].insert(buffid);
    };
    data.foreach(f);
  };
  params.getMutableData("Logic_Param").getMutableData("being_buff").foreach(beingf);

  return true;
}

bool BeingBuffSkill::run(SkillRunner& oRunner) const
{
  SceneUser* user = dynamic_cast<SceneUser*>(oRunner.getEntry());
  if (user == nullptr || user->isAlive() == false)
    return false;

  SSceneBeingData* being = user->getUserBeing().getCurBeingData();
  if (being == nullptr)
    return false;

  auto it = m_mapBeingBuff.find(being->dwID);
  if (it == m_mapBeingBuff.end())
    return false;

  if (user->getUserBeing().addBufftoBeing(being->dwID, it->second, user, m_dwSkillID) == false)
    return false;

  for (auto id : m_setSelfBuff)
    user->m_oBuff.add(id, user, m_dwSkillID);

  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

UseBeingSkill::UseBeingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

UseBeingSkill::~UseBeingSkill()
{

}

bool UseBeingSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwBeingSkillID = params.getData("Logic_Param").getTableInt("SkillID");
  return true;
}

bool UseBeingSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  if (BaseSkill::run(oRunner) == false)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*>(attacker);
  if (user == nullptr)
    return false;
  SSceneBeingData* being = user->getUserBeing().getCurBeingData();
  if (being == nullptr)
    return false;

  if (being->hasSkill(m_dwBeingSkillID / 1000) == false)
    return false;

  SceneNpc* beingNpc = user->getUserBeing().getCurBeingNpc();
  if (beingNpc == nullptr || beingNpc->isAlive() == false)
    return false;

  xSceneEntryDynamic *pEntry = nullptr;
  if (m_eLogic == ESKILLLOGIC_LOCKEDTARGET)
  {
    SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
    if (oCmd.data().hitedtargets_size() == 0)
      return false;
    pEntry = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(0).charid());
  }
  else
  {
    pEntry = attacker;
  }

  if (pEntry == nullptr || pEntry->isAlive() == false)
    return false;

  beingNpc->useSuperSkill(m_dwBeingSkillID, pEntry->id, attacker->getPos());
  oRunner.setCount(oRunner.getCount() + 1);

  return true;
}

RandomSkill::RandomSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

RandomSkill::~RandomSkill()
{

}

bool RandomSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  auto f = [&](const string& key, xLuaData& data)
  {
    m_vecSkillIDs.push_back(data.getInt());
  };
  params.getMutableData("Logic_Param").getMutableData("random_skill_ids").foreach(f);
  return true;
}

bool RandomSkill::run(SkillRunner& oRunner) const
{
  if (m_vecSkillIDs.empty())
    return false;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  BeingNpc* beingNpc = dynamic_cast<BeingNpc*>(attacker);
  if (beingNpc == nullptr || beingNpc->isAlive() == false)
    return false;
  SceneUser* user = beingNpc->getMasterUser();
  if (user == nullptr)
    return false;

  DWORD skillid = 0;
  DWORD skillgroupid = user->getUserBeing().getRuneSpecSkillID(beingNpc->getDefine().getID(), m_dwSkillID);
  if (skillgroupid != 0)
  {
    auto it = find_if(m_vecSkillIDs.begin(), m_vecSkillIDs.end(), [&](DWORD id){
        return id / ONE_THOUSAND == skillgroupid;
      });
    if (it != m_vecSkillIDs.end())
      skillid = *it;
  }

  if (skillid <= 0)
    skillid = m_vecSkillIDs[randBetween(0, m_vecSkillIDs.size() - 1)];

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  if (oCmd.data().hitedtargets_size() == 0)
      return false;

  xSceneEntryDynamic *pEntry = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(0).charid());
  if (pEntry == nullptr || pEntry->isAlive() == false)
    return false;

  beingNpc->useSkill(skillid, pEntry->id, attacker->getPos());
  oRunner.setCount(oRunner.getCount() + 1);

  return true;
}

/******* 偷钱技能********/
StealMoneySkill::StealMoneySkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

StealMoneySkill::~StealMoneySkill()
{

}

bool StealMoneySkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr || user->isAlive() == false)
    return false;

  collectTarget(oRunner);

  sendSkillStatusRun(oRunner);

  DWORD formula = m_stLogicParam.m_oParam.getTableInt("rate_type");
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(oCmd.data().hitedtargets(i).charid());
    if (npc == nullptr)
      continue;
    DWORD all = npc->getAllCarryMoney();
    if (all == 0)
      continue;
    float per = LuaManager::getMe().call<float> ("calcFormulaValue", attacker, (xSceneEntryDynamic*)npc, formula);
    if (per == 0)
      continue;
    DWORD cur = npc->getCarryMoney();
    if (cur == 0)
    {
      XLOG << "[偷窃技能], 玩家:" << user->name << user->id << "技能:" << m_dwSkillID << "怪物:" << npc->name << npc->id << "已被偷空" << XEND;
      MsgManager::sendMsg(user->id, 2518);
      continue;
    }

    DWORD money = all * per;
    money = cur < money ? cur : money;
    if (money == 0)
      continue;
    npc->setCarryMoney(cur - money);
    user->addMoney(EMONEYTYPE_SILVER, money, ESOURCE_MONSTERKILL);
    // send msg
    {
      MsgParams param;
      param.addNumber(100);
      param.addNumber(100);
      param.addNumber(money);
      MsgManager::sendMsg(user->id, 6, param);
    }
    user->markHitFieldMonster();
    XLOG << "[偷窃技能], 玩家:" << user->name << user->id << "技能:" << m_dwSkillID << "怪物:" << npc->name << npc->id << "获取zeny:" << money << "剩余zeny:" << cur - money << XEND;
  }

  oRunner.setCount(oRunner.getCount() + 1);

  return true;
}

/*胁持技能*/
SeizeSkill::SeizeSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

SeizeSkill::~SeizeSkill()
{

}

bool SeizeSkill::run(SkillRunner& oRunner) const
{
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  attacker = getRealAttacker(attacker);
  if (checkAttacker(attacker) == false)
    return false;

  Scene* pScene = attacker->getScene();
  if (pScene == nullptr)
    return false;

  if (BaseSkill::run(oRunner) == false)
    return false;

  attacker->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
  attacker->m_oBuff.delBuffByType(EBUFFTYPE_DEEPHIDE);

  float range = m_stLogicParam.m_oParam.getTableFloat("move_range");
  bool attackerMove = true;
  xPos oldpos = attacker->getPos();
  for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
  {
    if (oCmd.data().hitedtargets(i).damage() == 0)
      continue;
    // 造成伤害后, 产生胁持移动效果
    xSceneEntryDynamic* tar = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(i).charid());
    if (tar == nullptr)
      continue;

    SceneNpc* npc = dynamic_cast<SceneNpc*>(tar);
    if (npc && (npc->getNpcType() == ENPCTYPE_MVP || npc->getNpcType() == ENPCTYPE_MINIBOSS))
      continue;

    if (npc && npc->m_ai.isConfigCanMove() == false)
      continue;
    if (attackerMove)
    {
      attackerMove = false;
      xPos pos;
      if (pScene->getRandPosOverRange(attacker->getPos(), range, pos) == false)
      {
        XERR << "[胁持技能], 瞬移, 释放者:" << attacker->name << attacker->id << "未随机到合法位置" << XEND;
        break;
      }
      attacker->goTo(pos);
      XLOG << "[胁持技能], attacker:" << attacker->name << attacker->id << "移动到位置" << pos.x << pos.y << pos.z << XEND;
    }

    // 目标
    float dis = getXZDistance(oldpos, tar->getPos());
    xPos pos = attacker->getPos();
    pScene->getRandPos(attacker->getPos(), dis, pos);
    tar->goTo(pos);
    XLOG << "[胁持技能], target:" << tar->name << tar->id << "移动到位置" << pos.x << pos.y << pos.z << XEND;
  }

  return true;
}

// 擒拿
ControlSkill::ControlSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

ControlSkill::~ControlSkill()
{

}

bool ControlSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  collectTarget(oRunner);
  if (oCmd.data().hitedtargets_size() == 0)
    return false;

  attacker->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
  attacker->m_oBuff.delBuffByType(EBUFFTYPE_DEEPHIDE);

  sendSkillStatusRun(oRunner);

  bool haveEffect = false; // 擒拿目标有效时, 才会添加双方buff
  TSetSceneEntrys targets;
  for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
  {
    xSceneEntryDynamic* tar = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(i).charid());
    if (!tar)
      continue;
    SceneNpc* npc = dynamic_cast<SceneNpc*> (tar);
    if (npc && (npc->getNpcType() == ENPCTYPE_MVP || npc->getNpcType() == ENPCTYPE_MINIBOSS))
      continue;
    targets.insert(tar);
    haveEffect = true;
  }
  attacker->m_oSkillStatus.enterStatus(ESKILLSTATUS_CATCH, targets, this);

  if (haveEffect)
    addBuff(oRunner);

  oRunner.setCount(oRunner.getCount() + 1);

  return true;
}

RemoveTrapSkill::RemoveTrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

RemoveTrapSkill::~RemoveTrapSkill()
{

}

bool RemoveTrapSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  Scene* pScene = attacker->getScene();
  if (pScene == nullptr)
    return false;
  xSceneEntrySet tarset;
  switch(m_eLogic)
  {
    case ESKILLLOGIC_POINTRANGE:
      {
        float fRange = m_stLogicParam.fRange;
        if (attacker->getEntryType() == SCENE_ENTRY_USER)
        {
          SceneUser* user = dynamic_cast<SceneUser*> (attacker);
          if (user && user->getFighter())
            fRange += user->getFighter()->getSkill().getChangeRange(m_dwSkillID);
          fRange = fRange < 0 ? 0 : fRange;
        }
        SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
        xPos oPos;
        oPos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());
        pScene->getEntryList(oPos, fRange, tarset);
      }
      break;
    default:
      break;
  }

  sendSkillStatusRun(oRunner);

  DWORD formula = m_stLogicParam.m_oParam.getTableInt("num_type");
  DWORD trapnum = LuaManager::getMe().call<DWORD> ("calcFormulaValue", attacker, attacker, formula);

  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  for (auto &s : tarset)
  {
    if (trapnum == 0)
      continue;
    TrapNpc* npc = dynamic_cast<TrapNpc*> (s);
    if (npc == nullptr)
      continue;
    xSceneEntryDynamic* pMaster = xSceneEntryDynamic::getEntryByID(npc->getSkillMasterID());
    if (pMaster == nullptr || pMaster->isMyEnemy(attacker) == false)
      continue;
    if (user && npc->canVisible(user) == false)
      continue;
    pMaster->m_oSkillProcessor.delTrapSkill(npc->id);
    XLOG << "[陷阱移除], attacker:" << attacker->name << attacker->id << "陷阱:" << npc->id << "主人:" << pMaster->name << pMaster->id << XEND;
    trapnum --;
  }

  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

CopySkill::CopySkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

CopySkill::~CopySkill()
{

}

bool CopySkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr)
    return false;

  BaseSkill::run(oRunner);

  DWORD replaceid = m_stLogicParam.m_oParam.getTableInt("replaceid");
  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(oCmd.data().hitedtargets(i).charid());
    if (npc == nullptr)
    {
      SceneUser* tuser = SceneUserManager::getMe().getUserByID(oCmd.data().hitedtargets(i).charid());
      if (tuser)
        MsgManager::sendMsg(user->id, 12005);
      continue;
    }
    DWORD skillid = npc->getCFG() ? npc->getCFG()->dwCopySkill : 0;
    if (skillid == 0)
    {
      MsgManager::sendMsg(user->id, 12001);
      continue;
    }
    if ( m_dwSkillID % ONE_THOUSAND < skillid % ONE_THOUSAND)
    {
      MsgManager::sendMsg(user->id, 12000);
      continue;
    }
    user->getFighter()->getSkill().changeSkill(replaceid, skillid);
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(skillid);
    if (pSkill)
      MsgManager::sendMsg(user->id, 12002, MsgParams(pSkill->getName()));
    XLOG << "[CopySkill], 玩家:" << user->name << user->id << "成功抄袭怪物技能, 怪物:" << npc->name << npc->id << "技能:" << skillid << XEND;
  }
  return true;

}

FastRestoreSkill::FastRestoreSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

FastRestoreSkill::~FastRestoreSkill()
{

}

bool FastRestoreSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;

  m_dwPanaceaItemID = m_stLogicParam.m_oParam.getTableInt("panacea_item");
  m_dwRepairItemID = m_stLogicParam.m_oParam.getTableInt("repair_item");
  xLuaData& data = m_stLogicParam.m_oParam.getMutableData("panacea_heal");
  data.getIDList(m_setPanaceaHeal);
  return true;
}

bool FastRestoreSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (attacker);
  if (user == nullptr)
    return false;

  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);

  bool haveEffect = false;
  // 恢复一定时间内脱卸的装备
  DWORD configtime = m_stLogicParam.m_oParam.getTableInt("equipoff_time");
  configtime = configtime ? configtime : 60;
  if (user->getPackage().recoverOffEquipByTime(configtime))
  {
    MsgManager::sendMsg(user->id, 25302);
    haveEffect = true;
  }

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(user->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return false;
  BasePackage* pMainPack = user->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;
  do
  {
    if (pEquipPack->hasBraokenEquip())
    {
      haveEffect = true;
      const string& oneguid = pMainPack->getGUIDByType(m_dwRepairItemID);
      if (oneguid.empty())
      {
        // msg
        MsgManager::sendMsg(user->id, 25304);
        break;
      }
      if (user->m_oCDTime.done(CD_TYPE_ITEM, m_dwRepairItemID) == false)
      {
        // msg
        MsgManager::sendMsg(user->id, 25305);
        break;
      }

      ItemBase* pItem = pMainPack->getItem(oneguid);
      if (pItem == nullptr)
        break;
      const SItemCFG* pCFG = pItem->getCFG();
      if (pCFG == nullptr)
        break;
      GMCommandRuler::getMe().execute(user, pCFG->oGMData);

      pMainPack->reduceItem(oneguid, ESOURCE_USEITEM);

      const string& oneguid1 = pMainPack->getGUIDByType(m_dwRepairItemID);
      ItemBase* pBase = pMainPack->getItem(oneguid1);
      if (pBase)
        pBase->setCD(xTime::getCurMSec());

      user->m_oCDTime.add(pCFG->dwTypeID, pCFG->dwCD, CD_TYPE_ITEM);

      //msg
      MsgManager::sendMsg(user->id, 25303);
    }
    break;
  }
  while(true);

  bool hasheal = false;
  for (auto &s : m_setPanaceaHeal)
  {
    if (user->m_oBuff.isInStatus(s))
    {
      if (user->m_oBuff.isForceAddStatus(s) == false)
        hasheal = true;
    }
  }

  if (hasheal)
  {
    haveEffect = true;
    do
    {
      const string& oneguid = pMainPack->getGUIDByType(m_dwPanaceaItemID);
      if (oneguid.empty())
      {
        //msg
        MsgManager::sendMsg(user->id, 25307);
        break;
      }
      if (user->m_oCDTime.done(CD_TYPE_ITEM, m_dwPanaceaItemID) == false)
      {
        // msg
        break;
      }

      ItemBase* pItem = pMainPack->getItem(oneguid);
      if (pItem == nullptr)
        break;
      const SItemCFG* pCFG = pItem->getCFG();
      if (pCFG == nullptr)
        break;
      GMCommandRuler::getMe().execute(user, pCFG->oGMData);

      pMainPack->reduceItem(oneguid, ESOURCE_USEITEM);

      const string& oneguid1 = pMainPack->getGUIDByType(m_dwPanaceaItemID);
      ItemBase* pBase = pMainPack->getItem(oneguid1);
      if (pBase)
        pBase->setCD(xTime::getCurMSec());

      if (pCFG)
        user->m_oCDTime.add(pCFG->dwTypeID, pCFG->dwCD, CD_TYPE_ITEM);

      MsgManager::sendMsg(user->id, 25306);
      //msg
      break;
    }
    while(true);
  }
  if (!haveEffect)
    MsgManager::sendMsg(user->id, 25301);

  return true;
}

SpaceLeapSkill::SpaceLeapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

SpaceLeapSkill::~SpaceLeapSkill()
{

}

bool SpaceLeapSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*>(attacker);
  if (user == nullptr || user->getScene() == nullptr)
    return false;

  BaseSkill::run(oRunner);

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  xPos po;
  po.x = oCmd.data().pos().x() / 1000.0f;
  po.y = oCmd.data().pos().y() / 1000.0f;
  po.z = oCmd.data().pos().z() / 1000.0f;
  if (getXZDistance(user->getPos(), po) >  m_dwLaunchRange)
    return false;
  std::list<xPos> path;
  if (user->getScene()->findingPath(user->getPos(), po, path) == false)
    return false;

  user->goTo(po);

  oRunner.setCount(oRunner.getCount() + 1);
  XLOG << "[闪现技能], 玩家:" << user->name << user->id << "目标点:" << po.x << po.y << po.z << "技能:" << m_dwSkillID << "释放成功" << XEND;
  return true;
}

GoMapSkill::GoMapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

GoMapSkill::~GoMapSkill()
{

}

bool GoMapSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwMapID = params.getMutableData("Logic_Param").getTableInt("map_id");
  m_eTransType = static_cast<ETransportType>(params.getMutableData("Logic_Param").getTableInt("transport_type"));
  return true;
}

bool GoMapSkill::run(SkillRunner& oRunner) const
{
  sendSkillStatusRun(oRunner);

  SceneUser* pUser = dynamic_cast<SceneUser*>(oRunner.getEntry());
  if (pUser == nullptr)
    return false;

  oRunner.setCount(oRunner.getCount() + 1);

  switch (m_eTransType)
  {
    case ETRANSPORTTYPE_WEDDING_HONEYMOON:
      {
        DWORD weddingZoneID = pUser->getUserWedding().getWeddingInfo().zoneid();
        if (weddingZoneID == 0)
          return false;
        if (weddingZoneID != thisServer->getZoneID())
          pUser->m_oBuff.add(MiscConfig::getMe().getSystemCFG().dwZoneBossLimitBuff);

        pUser->getUserZone().gomap(weddingZoneID, m_dwMapID, GoMapType::Null);
      }
      break;
    default:
      return false;
  }
  return true;
}

// 婚姻
WeddingSkill::WeddingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

WeddingSkill::~WeddingSkill()
{

}

bool WeddingSkill::init(xLuaData& params)
{
  return BaseSkill::init(params);
}

bool WeddingSkill::run(SkillRunner& oRunner) const
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(oRunner.getEntry());
  if (pUser == nullptr || pUser->getUserWedding().isMarried() == false)
    return false;

  UserWedding& rWedding = pUser->getUserWedding();
  QWORD qwParnterID = rWedding.getWeddingParnter();
  if (qwParnterID == 0)
  {
    XERR << "[婚姻-好想你]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "邀请配偶回到身边失败,未有配偶" << XEND;
    return false;
  }
  Scene* pScene = pUser->getScene();
  if (pScene == nullptr)
  {
    XERR << "[婚姻-好想你]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "邀请配偶回到身边失败,未在场景中" << XEND;
    return false;
  }
  DScene* pDScene = dynamic_cast<DScene*>(pScene);
  if (pDScene != nullptr)
  {
    MsgManager::sendMsg(pUser->id, 9649);
    XERR << "[婚姻-好想你]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "邀请配偶回到身边失败,副本中不能邀请" << XEND;
    return false;
  }

  ParnterInfo oInfo;

  oInfo.set_mapid(pScene->id);
  oInfo.set_zoneid(thisServer->getZoneID());
  oInfo.set_x(pUser->getPos().x);
  oInfo.set_y(pUser->getPos().y);
  oInfo.set_z(pUser->getPos().z);

  SceneUser* pParnter = SceneUserManager::getMe().getUserByID(qwParnterID);
  if (pParnter != nullptr)
  {
    pParnter->getUserWedding().setMissInfo(oInfo);

    MissyouInviteWedCCmd cmd;
    PROTOBUF(cmd, send, len);
    pParnter->sendCmdToMe(send, len);
    XLOG << "[婚姻-好想你]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "邀请配偶回到身边成功" << XEND;
  }
  else
  {
    GCharReader oChar(thisServer->getRegionID(), qwParnterID);
    if (oChar.getByWedding() == false)
    {
      XERR << "[婚姻-好想你]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "邀请配偶回到身边,未在自身SceneServer发现配偶,邀请失败,获取redis数据失败" << XEND;
      return false;
    }
    if (oChar.getOnlineTime() < oChar.getOfflineTime())
    {
      MsgManager::sendMsg(pUser->id, 9653);
      return false;
    }
    if (CommonConfig::m_bJumpZoneCheckLevel && thisServer->getZoneID() != oChar.getZoneID())
    {
      DWORD dwMaxLv = ChatManager_SC::getMe().getZoneMaxBaseLv(thisServer->getZoneID());
      if (oChar.getBaseLevel() > dwMaxLv)
      {
        MsgManager::sendMsg(pUser->id, 9655);
        return false;
      }
    }

    MissyouInviteWedSCmd scmd;

    scmd.set_charid(qwParnterID);
    scmd.mutable_info()->CopyFrom(oInfo);
    scmd.set_trans(true);

    PROTOBUF(scmd, ssend, slen);
    thisServer->sendCmdToSession(ssend, slen);

    XLOG << "[婚姻-好想你]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "邀请配偶回到身边,未在自身SceneServer发现配偶,成功发送至SessionServer查询" << XEND;
  }

  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

bool WeddingSkill::canUseToTarget(xSceneEntryDynamic* attacker, const SkillBroadcastUserCmd& oCmd) const
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(attacker);
  if (pUser == nullptr || pUser->getUserWedding().isMarried() == false)
    return false;

  UserWedding& rWedding = pUser->getUserWedding();
  QWORD qwParnterID = rWedding.getWeddingParnter();
  if (qwParnterID == 0)
    return false;
  Scene* pScene = pUser->getScene();
  if (pScene == nullptr)
    return false;
  DScene* pDScene = dynamic_cast<DScene*>(pScene);
  if (pDScene != nullptr)
  {
    MsgManager::sendMsg(pUser->id, 9649);
    return false;
  }

  SceneUser* pParnter = SceneUserManager::getMe().getUserByID(qwParnterID);
  if (pParnter == nullptr)
  {
    GCharReader oChar(thisServer->getRegionID(), qwParnterID);
    if (oChar.getByWedding() == false)
      return false;
    if (oChar.getOnlineTime() < oChar.getOfflineTime())
    {
      MsgManager::sendMsg(pUser->id, 9653);
      return false;
    }
  }

  return true;
}

// 清除技能效果
ClearEffectSkill::ClearEffectSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

ClearEffectSkill::~ClearEffectSkill()
{

}

bool ClearEffectSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  xLuaData& logicParam = params.getMutableData("Logic_Param");
  m_dwClearNum = logicParam.getTableInt("clear_num");
  if (logicParam.has("forceClear"))
    logicParam.getMutableData("forceClear").getIDList(m_setForceClear);
  return true;
}

bool ClearEffectSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker == nullptr)
    return false;
  Scene* scene = attacker->getScene();
  if (scene == nullptr)
    return false;

  xPos pos;
  pos.set(oRunner.getCmd().data().pos().x(), oRunner.getCmd().data().pos().y(), oRunner.getCmd().data().pos().z());
  xSceneEntrySet targets;
  scene->getEntryListInBlock(SCENE_ENTRY_NPC, pos, m_stLogicParam.fRange, targets);
  scene->getEntryListInBlock(SCENE_ENTRY_TRAP, pos, m_stLogicParam.fRange, targets);

  sendSkillStatusRun(oRunner);

  DWORD num = m_dwClearNum;
  for (auto& v : targets)
  {
    if (num <= 0)
      break;

    bool del = false;
    do
    {
      TrapNpc* npc = dynamic_cast<TrapNpc*>(v);
      if (npc && npc->canImmunedByFieldArea())
      {
        xSceneEntryDynamic* master = xSceneEntryDynamic::getEntryByID(npc->getSkillMasterID());
        if (master && master->isMyEnemy(attacker))
        {
          master->m_oSkillProcessor.delTrapSkill(npc->id);
          del = true;
          XLOG << "[技能-清除效果] attacker:" << attacker->name << attacker->id << "TrapNpc:" << npc->id << "主人:" << master->name << master->id << XEND;
        }
        break;
      }
      SceneTrap* trap = dynamic_cast<SceneTrap*>(v);
      if (trap && (trap->canImmunedByFieldArea() || m_setForceClear.find(trap->getSkill()->getSkillID()/1000) != m_setForceClear.end()))
      {
        xSceneEntryDynamic* master = trap->getMaster();
        if (master && master->isMyEnemy(attacker))
        {
          master->m_oSkillProcessor.delTrapSkill(trap->id);
          del = true;
          XLOG << "[技能-清除效果] attacker:" << attacker->name << attacker->id << "SceneTrap:" << trap->id << "主人:" << master->name << master->id << XEND;
        }
        break;
      }
    }
    while (0);

    if (del)
      --num;
  }

  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

CursedCircleSkill::CursedCircleSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

CursedCircleSkill::~CursedCircleSkill()
{

}

bool CursedCircleSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
  collectTarget(oRunner);

  sendSkillStatusRun(oRunner);

  TSetSceneEntrys targets;
  for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
  {
    xSceneEntryDynamic* tar = xSceneEntryDynamic::getEntryByID(oCmd.data().hitedtargets(i).charid());
    if (!tar)
      continue;
    SceneNpc* npc = dynamic_cast<SceneNpc*> (tar);
    if (npc && (npc->getNpcType() == ENPCTYPE_MVP || npc->getNpcType() == ENPCTYPE_MINIBOSS))
      continue;
    targets.insert(tar);
  }
  if (targets.empty() == false)
  {
    attacker->m_oSkillStatus.enterStatus(ESKILLSTATUS_CURSEDCIRCLE, targets, this);
  }

  addBuff(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);

  return true;
}

RideChangeSkill::RideChangeSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp):BaseSkill(id, eType, eLogic, eCamp)
{

}

RideChangeSkill::~RideChangeSkill()
{

}

bool RideChangeSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_bSelfDestructor = params.getMutableData("Logic_Param").getTableInt("self_destructor") == 1;
  m_dwMachineSkill = params.getMutableData("Logic_Param").getTableInt("machine_skill");
  return true;
}

bool RideChangeSkill::run(SkillRunner& oRunner) const
{
  if (m_bSelfDestructor)
  {
    oRunner.setCount(oRunner.getCount() + 1);
    return selfDestructor(oRunner);
  }
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (!attacker)
    return false;

  DWORD dwAttrAffect2 = m_stLogicParam.m_oParam.getTableInt("AttrEffect2");

  oRunner.setCount(oRunner.getCount() + 1);
  //骑狼术
  if (18 == dwAttrAffect2)
  {
    //骑乘状态下使用技能解除骑乘状态
    //if (attacker->isRideWolf())
    if (attacker->getBuff().haveBuffType(EBUFFTYPE_RIDEWOLF))
    {
      delSelfBuffs(oRunner);
      return true;
    }
  }
  //魔导机械状态
  else if( 20 == dwAttrAffect2)
  {
    //变身状态下使用技能，解除变身状态
    if(attacker->isBeMagicMachine())
    {
      delSelfBuffs(oRunner);
      return true;
    }
  }
  else
  {
    XERR << "[技能-Buff], 配置错误, 技能参数 AttrEffect2:" << dwAttrAffect2 << "SkillID" << m_dwSkillID  << XEND;
    return false;
  }
  addSelfBuffs(oRunner);
  return true;
}

bool RideChangeSkill::selfDestructor(SkillRunner& oRunner) const
{
  //用于魔导机械自爆
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (!attacker)
    return false;
  const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(m_dwMachineSkill);
  if (!pSkill)
    return false;
  for(auto& buffid : pSkill->getSelfBuffs())
    attacker->m_oBuff.del(buffid);
  return true;
}

void RideChangeSkill::addSelfBuffs(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (!attacker)
    return;
  for(auto& buffid : getSelfBuffs())
  {
    attacker->m_oBuff.add(buffid, attacker, m_dwSkillID);
  }
}

void RideChangeSkill::delSelfBuffs(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (!attacker)
    return;
  for(auto& buffid : getSelfBuffs())
  {
    attacker->m_oBuff.del(buffid);
  }
}

void RideChangeSkill::onReset(xSceneEntryDynamic* pEntry) const
{
  if (!pEntry)
    return;
  for(auto& buffid : getSelfBuffs())
  {
    pEntry->m_oBuff.del(buffid);
  }
}


ReviveSkill::ReviveSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp)
  :BaseSkill(id, eType, eLogic, eCamp)
{

}

ReviveSkill::~ReviveSkill()
{

}

bool ReviveSkill::init(xLuaData& params)
{
  if (!BaseSkill::init(params))
    return false;
  auto f = [&](const std::string& key, xLuaData& data)
  {
    m_setOnceBuff.insert(data.getInt());
  };
  params.getMutableData("Logic_Param").getMutableData("OnceBuff").foreach(f);
  return true;
}

bool ReviveSkill::run(SkillRunner& oRunner) const
{
  if (oRunner.isFirst())
  {
    oRunner.setIsFirst(false);
    xSceneEntryDynamic* attacker = oRunner.getEntry();
    if (checkAttacker(attacker) == false)
      return false;

    Scene* pScene = attacker->getScene();
    if (pScene == nullptr)
      return false;

    SceneUser* pReliver = dynamic_cast<SceneUser*> (attacker);
    if (pReliver == nullptr || pReliver->getScene() == nullptr)
      return false;

    SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
    collectTarget(oRunner);

    // PhaseData data(oCmd.data());
    // oCmd.mutable_data()->clear_hitedtargets();
    // // attack targets
    // PhaseData* pData = oCmd.mutable_data();
    for (int i = 0; i < oCmd.data().hitedtargets_size(); ++i)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(oCmd.data().hitedtargets(i).charid());
      if (pUser == nullptr || pUser->getStatus() != ECREATURESTATUS_DEAD)
        continue;
      if (attacker->isMyEnemy(pUser))
        continue;

      // pData->add_hitedtargets()->set_charid(pUser->id);
      // oRunner.getBuffTargets().insert(pUser->id);

      if (pScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
      {
        DWORD relivecd = pUser->getReliveCD();
        if (relivecd && now() < pUser->getDieTime() + relivecd)
        {
          pUser->setWaitRelive(attacker->id, pUser->getDieTime() + relivecd);
          DieTimeCountEventCmd cmd;
          cmd.set_time(pUser->getDieTime() + relivecd - now());
          cmd.set_name(attacker->name);
          PROTOBUF(cmd, send2, len2);
          pUser->sendCmdToMe(send2, len2);

          pUser->setDataMark(EUSERDATATYPE_STATUS);
          pUser->refreshDataAtonce();
          continue;
        }
      }
      pUser->relive(ERELIVETYPE_SKILL, pReliver);
      // xPos pos = pUser->getPos();
      // pUser->getScene()->getRandPos(pos, 5, pos);
      // pUser->goTo(pos);

      for(auto& buffid : m_setOnceBuff)
      {
        pUser->m_oBuff.add(buffid, pReliver, m_dwSkillID);
      }
    }
  }

  if (!BaseSkill::run(oRunner))
    return false;
  return true;

}

SummonElementSkill::SummonElementSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp)
  :BaseSkill(id, eType, eLogic, eCamp)
{
}

SummonElementSkill::~SummonElementSkill()
{
}

bool SummonElementSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  
  auto f = [&](const string& key, xLuaData& data)
  {
    if (data.getInt() != 0)
      m_setElementIDs.insert(data.getInt());
  };
  params.getMutableData("Logic_Param").getMutableData("element_ids").foreach(f);
  m_dwBaseLastSeconds = params.getMutableData("Logic_Param").getTableInt("base_seconds");
  m_dwEffectSkillID = params.getMutableData("Logic_Param").getTableInt("effect_skill");
  m_fEffectRatio = params.getMutableData("Logic_Param").getTableFloat("effect_ratio");
  
  return true;
}

bool SummonElementSkill::run(SkillRunner& oRunner) const
{
  SceneUser* user = dynamic_cast<SceneUser*>(oRunner.getEntry());
  if (user == nullptr || user->isAlive() == false)
    return false;

  DWORD elementid = user->getUserSceneData().getSkillOptValue(ESKILLOPTION_SUMMON_ELEMENT);
  if (elementid == 0)
  {
    if (m_setElementIDs.empty() == false)
      elementid = *(m_setElementIDs.begin());
    else
      return false;
  }

  if (m_setElementIDs.find(elementid) == m_setElementIDs.end())
    return false;

  DWORD dwSkillLv = user->getSkillLv(m_dwEffectSkillID);
  DWORD dwLastSeconds = (DWORD)(m_dwBaseLastSeconds + dwSkillLv * m_fEffectRatio);
  if (dwLastSeconds == 0)
    return false;
  if (user->getUserElementElf().summon(elementid, dwLastSeconds) == false)
    return false;

  addBuff(oRunner);
  sendSkillStatusRun(oRunner);
  oRunner.setCount(oRunner.getCount() + 1);
  return true;
}

ElementTrapSkill::ElementTrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp)
  :BaseSkill(id, eType, eLogic, eCamp)
{
}

ElementTrapSkill::~ElementTrapSkill()
{
}

bool ElementTrapSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_bClearTrap = params.getMutableData("Logic_Param").getTableInt("clear_trap") == 1;
  return true;
}

bool ElementTrapSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker == nullptr)
    return false;
  Scene* scene = attacker->getScene();
  if (scene == nullptr)
    return false;

  if (m_bClearTrap)
  {
    xPos pos;
    pos.set(oRunner.getCmd().data().pos().x(), oRunner.getCmd().data().pos().y(), oRunner.getCmd().data().pos().z());
    xSceneEntrySet targets;
    scene->getEntryListInBlock(SCENE_ENTRY_NPC, pos, m_stLogicParam.fRange, targets);
    scene->getEntryListInBlock(SCENE_ENTRY_TRAP, pos, m_stLogicParam.fRange, targets);

    for (auto& v : targets)
    {
      do
      {
        TrapNpc* npc = dynamic_cast<TrapNpc*>(v);
        if (npc && npc->canImmunedByFieldArea())
        {
          xSceneEntryDynamic* master = xSceneEntryDynamic::getEntryByID(npc->getSkillMasterID());
          if (master)
          {
            master->m_oSkillProcessor.delTrapSkill(npc->id);
            XLOG << "[技能-清除效果] attacker:" << attacker->name << attacker->id << "TrapNpc:" << npc->id << "主人:" << master->name << master->id << XEND;
          }
          break;
        }
        SceneTrap* trap = dynamic_cast<SceneTrap*>(v);
        if (trap && trap->canImmunedByFieldArea())
        {
          xSceneEntryDynamic* master = trap->getMaster();
          if (master)
          {
            master->m_oSkillProcessor.delTrapSkill(trap->id);
            XLOG << "[技能-清除效果] attacker:" << attacker->name << attacker->id << "SceneTrap:" << trap->id << "主人:" << master->name << master->id << XEND;
          }
          break;
        }
      }
      while (0);
    }
  }

  return true;
}

SoloSkill::SoloSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

SoloSkill::~SoloSkill()
{

}

bool SoloSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwCostSp = params.getMutableData("Logic_Param").getTableInt("cost_sp");
  m_setWeaponItemType.clear();
  auto weaponf = [&](const string& key, xLuaData& data)
  {
    DWORD itemtype = data.getInt();
    if (itemtype <= EITEMTYPE_MIN || itemtype >= EITEMTYPE_MAX || EItemType_IsValid(itemtype) == false)
      return;
    m_setWeaponItemType.insert(static_cast<EItemType>(itemtype));
  };
  params.getMutableData("Logic_Param").getMutableData("weapon_type").foreach(weaponf);
  return true;
}

bool SoloSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  SceneUser* user = dynamic_cast<SceneUser*>(attacker);
  if (user == nullptr)
    return false;

  if (m_setWeaponItemType.empty() == false && m_setWeaponItemType.find(static_cast<EItemType>(user->getWeaponType())) == m_setWeaponItemType.end())
    return false;

  if (oRunner.isFirst())
  {
    if (attacker->m_oSkillStatus.enterStatus(ESKILLSTATUS_SOLO, TSetSceneEntrys{}, this) == false)
      return false;
    const SSkillTypeCFG* typecfg = MiscConfig::getMe().getSkillCFG().getSkillTypeCFG(m_eType);
    if (typecfg)
    {
      for (auto buffid : typecfg->setBuff)
        attacker->m_oBuff.add(buffid, attacker, m_dwSkillID);
    }
    attacker->setGMAttr(EATTRTYPE_SOLO, m_dwSkillID);
    attacker->refreshDataAtonce();
  }
  else
  {
    float spper = 0, spcost = 0;
    if (user->getFighter() && user->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
    {
      spper = user->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_SPCOSTPER);
      spcost = user->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_SPCOST);
    }

    float sp = m_dwCostSp * (1 + spper) + spcost;
    sp = sp >= 0 ? sp : 0;
    if (sp && user->getSp() < sp)
      return false;
    if (sp)
      user->setSp(user->getSp() - sp);
  }

  // 获得技能目标
  if (collectTarget(oRunner) == false)
    return false;

  // 通知前端伤害信息 & 技能运行阶段同步
  if (oRunner.isFirst())
    sendSkillStatusRun(oRunner);

  // 添加buff
  addBuff(oRunner);

  oRunner.setCount(oRunner.getCount() + 1);
  oRunner.setIsFirst(false);

  return true;
}

void SoloSkill::end(SkillRunner& oRunner) const
{
  BaseSkill::end(oRunner);
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker)
  {
    attacker->m_oSkillStatus.delStatus(ESKILLSTATUS_SOLO);

    SceneUser* user = dynamic_cast<SceneUser*>(attacker);
    if (user && user->getFighter())
      user->getFighter()->getSkill().setLastConcertSkillID(getSkillID() / ONE_THOUSAND);
  }
}

EnsembleSkill::EnsembleSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

EnsembleSkill::~EnsembleSkill()
{

}

bool EnsembleSkill::init(xLuaData& params)
{
  if (BaseSkill::init(params) == false)
    return false;
  m_dwCostSp = params.getMutableData("Logic_Param").getTableInt("cost_sp");
  m_dwPartnerSkillID = params.getMutableData("Logic_Param").getTableInt("partner_skillid");
  m_setWeaponItemType.clear();
  auto weaponf = [&](const string& key, xLuaData& data)
  {
    DWORD itemtype = data.getInt();
    if (itemtype <= EITEMTYPE_MIN || itemtype >= EITEMTYPE_MAX || EItemType_IsValid(itemtype) == false)
      return;
    m_setWeaponItemType.insert(static_cast<EItemType>(itemtype));
  };
  params.getMutableData("Logic_Param").getMutableData("weapon_type").foreach(weaponf);
  return true;
}

DWORD EnsembleSkill::getCostSp(SceneUser* user) const
{
  if (user == nullptr)
    return 0;
  float spper = 0, spcost = 0;
  if (user->getFighter() && user->getFighter()->getSkill().haveSpecSkill(m_dwSkillID))
  {
    spper = user->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_SPCOSTPER);
    spcost = user->getFighter()->getSkill().getSkillAttr(m_dwSkillID, EATTRTYPE_SPCOST);
  }
  float sp = m_dwCostSp * (1 + spper) + spcost;
  return sp >= 0 ? sp : 0;
}

bool EnsembleSkill::run(SkillRunner& oRunner) const
{
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;
  SceneUser* user = dynamic_cast<SceneUser*>(attacker);
  if (user == nullptr)
    return false;
  QWORD partnerid = oRunner.getEnsemblePartner();
  if (partnerid == 0)
    return false;
  SceneUser* partner = dynamic_cast<SceneUser*>(xSceneEntryDynamic::getEntryByID(partnerid));
  if (partner == nullptr || partner->getScene() != attacker->getScene())
    return false;
  if (user->isAlive() == false || partner->isAlive() == false || partner->isAttrCanSkill(this) == false)
    return false;

  if (m_setWeaponItemType.empty() == false && (m_setWeaponItemType.find(static_cast<EItemType>(user->getWeaponType())) == m_setWeaponItemType.end() ||
                                               m_setWeaponItemType.find(static_cast<EItemType>(partner->getWeaponType())) == m_setWeaponItemType.end()))
    return false;

  if (oRunner.isFirst())
  {
    if (user->m_oSkillStatus.enterStatus(ESKILLSTATUS_ENSEMBLE, TSetSceneEntrys{partner}, this) == false)
      return false;

    partner->m_oCDTime.add(m_dwPartnerSkillID * ONE_THOUSAND + partner->getSkillLv(m_dwPartnerSkillID), getCD(partner), CD_TYPE_SKILL);

    const SSkillTypeCFG* typecfg = MiscConfig::getMe().getSkillCFG().getSkillTypeCFG(m_eType);
    if (typecfg)
    {
      for (auto buffid : typecfg->setBuff)
      {
        attacker->m_oBuff.add(buffid, attacker, m_dwSkillID);
        partner->m_oBuff.add(buffid, attacker, m_dwSkillID);
      }
    }
    attacker->setGMAttr(EATTRTYPE_ENSEMBLE, m_dwSkillID);
    attacker->refreshDataAtonce();
    partner->setGMAttr(EATTRTYPE_ENSEMBLE, m_dwPartnerSkillID * ONE_THOUSAND + m_dwSkillID % ONE_THOUSAND);
    partner->refreshDataAtonce();
  }
  else
  {
    float fRange = m_stLogicParam.fRange + user->getFighter()->getSkill().getChangeRange(m_dwSkillID);
    fRange = fRange < 0 ? 0 : fRange;

    SkillBroadcastUserCmd& oCmd = oRunner.getCmd();
    xPos oPos;
    oPos.set(oCmd.data().pos().x(), oCmd.data().pos().y(), oCmd.data().pos().z());
    if (getXZDistance(oPos, user->getPos()) > fRange || getXZDistance(oPos, partner->getPos()) > fRange)
      return false;

    DWORD sp1 = getCostSp(user), sp2 = getCostSp(partner);
    if ((sp1 && user->getSp() < sp1) || (sp2 && partner->getSp() < sp2))
      return false;

    if (sp1)
      user->setSp(user->getSp() - sp1);
    if (sp2)
      partner->setSp(partner->getSp() - sp2);
  }

  if (BaseSkill::run(oRunner) == false)
    return false;

  return true;
}

void EnsembleSkill::end(SkillRunner& oRunner) const
{
  BaseSkill::end(oRunner);
  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (attacker)
  {
    attacker->m_oSkillStatus.delStatus(ESKILLSTATUS_ENSEMBLE);

    SceneUser* user = dynamic_cast<SceneUser*>(attacker);
    if (user && user->getFighter())
      user->getFighter()->getSkill().setLastConcertSkillID(getSkillID() / ONE_THOUSAND);
  }
}

xSceneEntryDynamic* EnsembleSkill::getPartner(xSceneEntryDynamic* user, DWORD& skillid) const
{
  SceneUser* master = dynamic_cast<SceneUser*>(user);
  if (master == nullptr)
    return nullptr;
  QWORD cur = xTime::getCurMSec();
  if (master->m_oSkillStatus.inStatus(cur))
    return nullptr;

  vector<pair<xSceneEntryDynamic*, DWORD>> teammate;
  float fRange = m_stLogicParam.fRange + master->getFighter()->getSkill().getChangeRange(m_dwSkillID);
  fRange = fRange < 0 ? 0 : fRange;
  for (auto& m : master->getTeam().getTeamMemberList())
  {
    if (m.second.charid() == master->id)
      continue;
    SceneUser* p = SceneUserManager::getMe().getUserByID(m.second.charid());
    if (p == nullptr || p->getFighter() == nullptr || p->getScene() == nullptr || p->getScene() != master->getScene() || p->m_oSkillStatus.inStatus(cur) || p->isAttrCanSkill(this) == false)
      continue;
    DWORD sklv = p->getSkillLv(m_dwPartnerSkillID);
    if (sklv <= 0)
      continue;
    if (m_setWeaponItemType.empty() == false && m_setWeaponItemType.find(static_cast<EItemType>(p->getWeaponType())) == m_setWeaponItemType.end())
      continue;
    if (getXZDistance(master->getPos(), p->getPos()) > fRange)
      continue;
    teammate.push_back(make_pair((xSceneEntryDynamic*)p, sklv));
  }
  if (teammate.empty())
    return nullptr;
  auto& partner = teammate[randBetween(0, teammate.size() - 1)];
  skillid = getFamilyID() + (m_dwSkillID % ONE_THOUSAND + partner.second) / 2;
  return partner.first;
}

StopConcertSkill::StopConcertSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp) : BaseSkill(id, eType, eLogic, eCamp)
{

}

StopConcertSkill::~StopConcertSkill()
{

}

bool StopConcertSkill::run(SkillRunner& oRunner) const
{
  if (BaseSkill::run(oRunner) == false)
    return false;

  xSceneEntryDynamic* attacker = oRunner.getEntry();
  if (checkAttacker(attacker) == false)
    return false;

  attacker->m_oSkillStatus.delStatus(ESKILLSTATUS_SOLO);
  attacker->m_oSkillStatus.delStatus(ESKILLSTATUS_ENSEMBLE);

  return true;
}
