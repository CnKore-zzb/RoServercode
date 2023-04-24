#include "SceneNpc.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "SceneItemManager.h"
#include "SceneManager.h"
#include "SceneServer.h"
#include "BossCmd.pb.h"
//#include "SceneActManager.h"
#include "MusicBoxManager.h"
#include "FeatureConfig.h"
#include "PlatLogManager.h"
#include "GuidManager.h"
#include "StatisticsDefine.h"
#include "ActivityManager.h"
#include "TeamSealManager.h"
#include "GMCommandRuler.h"
#include "MsgManager.h"
#include "DScene.h"
#include "PetConfig.h"
#include "MiscConfig.h"
#include "FoodConfig.h"
#include "GuildMusicBoxManager.h"
#include "SkillManager.h"
#include "CommonConfig.h"
#include "ActivityEventManager.h"
#include "BossSCmd.pb.h"
#include "BossMgr.h"

SceneNpc::SceneNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : xSceneEntryDynamic(id, id)
  , define(def)
  , m_oOriDefine(def)
  , m_pCurCFG(pCFG)
  
  , m_pOriCFG(pCFG)
  , m_ai(this)
  , m_sai(this)
  , m_oEmoji(this)
{

}

SceneNpc::~SceneNpc()
{

}

bool SceneNpc::initAttr()
{
  if (m_pAttribute == nullptr)
  {
    m_pAttribute = NEW NpcAttribute(this);
    if (m_pAttribute == nullptr)
      return false;
  }

  loadAttr();
  return true;
}

bool SceneNpc::init(const SNpcCFG* pCFG, const NpcDefine &def)
{
  if (pCFG == nullptr || m_pOriCFG == nullptr)
    return false;

  m_pCurCFG = pCFG;
  define = def;
  if (m_pCurCFG == nullptr || initAttr() == false)
    return false;

  if (define.getPursue() != 0 && define.getTerritory() >= define.getPursue())
  {
    XERR << "[怪物配置错误], territory大于pursue, 怪物ID:" << define.getID() << "territory:" << define.getTerritory() << "pursue:" << define.getPursue() << XEND;
  }
  if (0 != strncmp(define.getName().c_str(), "", MAX_NAMESIZE))
    strncpy(name, define.getName().c_str(), MAX_NAMESIZE);
  else
    strncpy(name, m_pCurCFG->strName.c_str(), MAX_NAMESIZE);

  if (define.getScaleMin() != 1.0f && define.getScaleMin() != 0 && define.getScaleMax() != 0)
    m_flScale = randFBetween(define.getScaleMin(), define.getScaleMax());
  else
    m_flScale = m_pCurCFG->fScale;
  m_flScale = m_flScale == 0.0f ? 1.0f : m_flScale;

  if (define.getDisptime() != 0)
    m_dwSetClearTime = now() + define.getDisptime();

  //m_dwDeadCount = 0;
  m_dwEvoTime = xTime::getCurSec();
  m_birthTime = xTime::getCurMSec();
  beHitFirst = true;
  m_dwDeadTime = 0;
  m_dwReliveTime = 0;
  m_dwDeadTime = 0;
  m_oOwner.first = 0;
  m_oOwner.second = 0;

  m_mapUserDamage.clear();
  m_setEventUser.clear();
  m_dwProtectInterval = 0;
  m_ai.clear();
  m_oMove.clear();

  m_mapUser2MvpInfo.clear();
  if (pCFG->dwDefaultGear != 0)
    m_dwGearStatus = pCFG->dwDefaultGear;
  m_bSpecialGearStatus = false;
  m_dwCarryMoney = define.getSuperAiNpc() == true ? 0 : pCFG->dwCarryMoney;

  m_eBossType = static_cast<EBossType>(define.getBossType());
  XLOG << "[场景npc-初始化]" << id << getNpcID() << name << "scale :" << m_flScale << "初始化成功" << XEND;
  return true;
}

void SceneNpc::reload()
{
  m_pCurCFG = NpcConfig::getMe().getNpcCFG(define.getID());
  if (m_pCurCFG == nullptr)
    XERR << "[场景npc-重加载]" << id << define.getID() << name << "当前 重加载未在 Table_Npc.txt 或者 Table_Monster.txt 表中找到" << XEND;
  else
    XDBG << "[场景npc-重加载]" << id << define.getID() << name << "当前 成功" << XEND;

  m_pOriCFG = NpcConfig::getMe().getNpcCFG(m_oOriDefine.getID());
  if (m_pOriCFG == nullptr)
    XERR << "[场景npc-重加载]" << id << m_oOriDefine.getID() << name << "变身前 重加载未在 Table_Npc.txt 或者 Table_Monster.txt 表中找到" << XEND;
  else
    XDBG << "[场景npc-重加载]" << id << m_oOriDefine.getID() << name << "变身前 成功" << XEND;
}

void SceneNpc::onOneSecTimeUp(QWORD curMSec)
{
  xSceneEntryDynamic::onOneSecTimeUp(curMSec);
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  m_sai.checkSig("time");

  m_sai.checkSig("hpless");

  m_sai.checkSig("alert");

  setTimeTalk(curSec);
  checkNightWork(curSec);
}

void SceneNpc::onFiveSecTimeUp(QWORD curMSec)
{
  xSceneEntryDynamic::onFiveSecTimeUp(curMSec);
}

void SceneNpc::onOneMinTimeUp(QWORD curMSec)
{
  xSceneEntryDynamic::onOneMinTimeUp(curMSec);

  setFollowerFace();
  if (getNpcType() == ENPCTYPE_MVP && isAlive())
    refreshMvpInfo(curMSec / ONE_THOUSAND);
}

void SceneNpc::onTenMinTimeUp(QWORD curMSec)
{
  xSceneEntryDynamic::onTenMinTimeUp(curMSec);
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);
  checkEvoState(curSec);
}

void SceneNpc::onDailyRefresh(QWORD curMSec)
{
  xSceneEntryDynamic::onDailyRefresh(curMSec);
}

bool SceneNpc::enterScene(Scene* scene)
{
  if (getStatus() == ECREATURESTATUS_CLEAR || scene == nullptr || m_pCurCFG == nullptr)
    return false;
  setScene(scene);

  // put before sendMeToNine
  m_oBuff.onEnterScene();

  bool oldPosRelive = false;
  if (getPos().empty() == false && (m_bReliveAtOldPos || define.isReliveAtDeadPos()))
    oldPosRelive = true;

  if (define.getIgnoreNavMesh())
  {
    xPos pos = define.getPos();
    if (getScene()->isValidPos(pos) == false)
    {
      return false;
    }
    setPos(pos);
  }
  else if (!oldPosRelive)
  {
    xPos pos;
    if (define.getRange())
    {
      getScene()->getRandPos(define.getPos(), define.getRange(), pos);
      getScene()->getValidPos(pos);

      if (getScene()->isValidPos(pos) == false)
      {
        if (getScene()->getRandPos(pos) == false)
          return false;
      }
      setPos(pos);
      m_oBirthPos = pos;
    }
    else
    {
      if (m_pCurCFG->getFeaturesByType(ENPCFEATURESPARAM_USEDEFINEPOS))
      {
        pos = define.getPos();
      }
      else
      {
        getScene()->getValidPos(define.getPos(), pos);
        if (getScene()->isValidPos(pos) == false)
        {
          if (getScene()->getRandPos(pos) == false)
            return false;
        }
      }
      setPos(pos);
      m_oBirthPos = pos;
    }
  }
  m_bReliveAtOldPos = false;
  m_bTempJustBirth = true;

  getScene()->addEntryAtPosI(this);
  sendMeToNine();

  const TSetDWORD& setIDs = define.getSuperAI().empty() == true ? m_pCurCFG->setSuperAI : define.getSuperAI();
  if (define.getExtraAI().empty())
  {
    m_sai.init(setIDs);
  }
  else
  {
    TSetDWORD allIDs;
    if (setIDs.empty() == false)
      allIDs.insert(setIDs.begin(), setIDs.end());
    allIDs.insert(define.getExtraAI().begin(), define.getExtraAI().end());
    m_sai.init(allIDs);
  }

  m_sai.checkSig("birth");
  m_oEmoji.check("Birth");

  m_dwBornTime = now();

  GuildScene *pGuildScene = dynamic_cast<GuildScene*>(getScene());
  if (pGuildScene != nullptr)
    GuildMusicBoxManager::getMe().initMusicNpc(this);
  else
    MusicBoxManager::getMe().initMusicNpc(this);

  MusicBoxManager::getMe().initActivityMusicNpc(this);

  return true;
}

void SceneNpc::leaveScene()
{
  if (!getScene())
    return;

  m_dwSceneID = getScene()->id;

  refreshDataAtonce();
  updateData(xTime::getCurSec());

  getScene()->delEntryAtPosI(this);
  delMeToNine();

  if (m_qwActivityUid)
  {
    ActivityBase* pActivity = ActivityManager::getMe().getActivityByUid(m_qwActivityUid);
    if (pActivity)
      pActivity->onDelNpc(this);
  }

  if (define.m_oVar.m_qwNpcOwnerID)
  {
    SceneNpc* pOwner = SceneNpcManager::getMe().getNpcByTempID(define.m_oVar.m_qwNpcOwnerID);
    if (pOwner != nullptr)
    {
      if (define.getLife() != 0)
        pOwner->m_oFollower.removeServant(id);
      pOwner->m_sai.checkSig("servant_die");
    }
  }

  if (m_eBossType == EBOSSTYPE_WORLD || m_eBossType == EBOSSTYPE_DEAD)
  {
    if (m_pAttribute != nullptr && m_pAttribute->getAttr(EATTRTYPE_HP) > 0)
    {
      Cmd::BossDieBossSCmd bossCmd;
      bossCmd.set_npcid(getNpcID());
      bossCmd.set_mapid(getScene()->getMapID());
      PROTOBUF(bossCmd, send, len);
      thisServer->sendCmdToSession(send, len);
      XDBG << "[AI-npc消失]" << id << getNpcID() << name << "在地图" << getScene()->getMapID() << "ai消失,通知会话击杀" << XEND;
    }
    if (m_eBossType == EBOSSTYPE_WORLD)
      BossMgr::getMe().onShow(this);
  }

  m_ai.clear();

  setScene(nullptr);
  SceneNpcManager::getMe().addLeaveSceneNpc(this);
}

void SceneNpc::delMeToNine()
{
  if (!getScene()) return;
  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);
  //if (isMonster() == false)
    //cmd.set_fadeout(MiscConfig::getMe().getSystemCFG().dwNpcFadeoutTime);

  if (m_dwTempFadeOutTimeMs)
  {
    cmd.set_fadeout(m_dwTempFadeOutTimeMs);
    m_dwTempFadeOutTimeMs = 0;
  }

  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
}

void SceneNpc::sendMeToNine()
{
  if (!getScene()) return;

  if (isMask()) return;

  Cmd::AddMapNpc cmd;
  fillMapNpcData(cmd.add_npcs());
  PROTOBUF(cmd, send, len);

  UserActionNtf message;
  if (m_dwGearStatus)
  {
    message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
    message.set_value(m_dwGearStatus);
    message.set_charid(this->id);
  }
  PROTOBUF(message, gsend, glen);

  xSceneEntrySet set;
  getScene()->getEntryListInNine(SCENE_ENTRY_USER, this->getPos(), set);

  bool hide = needCheckHideUser() && getAttr(EATTRTYPE_HIDE);
  for (auto &iter : set)
  {
    if (this->isVisableToSceneUser((SceneUser *)iter))
    {
      if (isScreenLimit((SceneUser*)iter))
        continue;
      if (hide && isHideUser((SceneUser*)iter))
        continue;
      ((SceneUser *)iter)->sendCmdToMe(send, len);
      this->informUserAdd((SceneUser*)iter);

      if (m_dwGearStatus)
        ((SceneUser *)iter)->sendCmdToMe(gsend, glen);
      sendExtraInfo((SceneUser*)iter);
    }
  }
}

void SceneNpc::sendCmdToNine(const void* cmd, DWORD len, GateIndexFilter filter/*=GateIndexFilter()*/)
{
  if (getScene() == nullptr || isMask() == true)
    return;

  if (this->isVisableToAll())
  {
    //getScene()->sendCmdToNine(this->getPos(), cmd, len);
    xSceneEntryDynamic::sendCmdToNine(cmd, len, filter);
  }
  else
  {
    xSceneEntrySet set;
    getScene()->getEntryListInNine(SCENE_ENTRY_USER, this->getPos(), set);
    for (auto &iter : set)
    {
      if (this->isVisableToSceneUser((SceneUser *)iter))
        ((SceneUser *)iter)->sendCmdToMe(cmd, len);
    }
  }
}

void SceneNpc::sendCmdToScope(const void* cmd, DWORD len)
{
  if (getScene() == nullptr || isMask() == true)
    return;

  SceneUser *pUser = getScreenUser();
  if (pUser)
  {
    getScene()->sendCmdToScope(pUser, cmd, len);
  }
  else
  {
    getScene()->sendCmdToNine(this->getPos(), cmd, len);
  }
}

void SceneNpc::fillMapNpcData(Cmd::MapNpc *data)
{
  if (data == nullptr || m_pCurCFG == nullptr)
    return;

  data->set_id(tempid);
  data->set_npcid(m_pCurCFG->dwID);
  if (define.getName().empty() == false)
    data->set_name(define.getName());
  else
  data->set_name(m_pCurCFG->strName);

  Cmd::ScenePos *p = data->mutable_pos();
  p->set_x(getPos().getX());
  p->set_y(getPos().getY());
  p->set_z(getPos().getZ());

  const xPos& destPos = m_oMove.getStraightPoint();
  if (!destPos.empty())
  {
    Cmd::ScenePos *dest = data->mutable_dest();
    dest->set_x(destPos.getX());
    dest->set_y(destPos.getY());
    dest->set_z(destPos.getZ());
  }

  add_data(data->add_datas(), EUSERDATATYPE_BODYSCALE, getScale());
  add_data(data->add_datas(), EUSERDATATYPE_STATUS, getStatus());
  add_data(data->add_datas(), EUSERDATATYPE_DIR, m_ai.isInChangeAngle() ? m_ai.getAngle() * ONE_THOUSAND : define.getDir() * ONE_THOUSAND);
  if (m_pCurCFG->figure.shadercolor != 0)
    add_data(data->add_datas(), EUSERDATATYPE_SHADERCOLOR, m_pCurCFG->figure.shadercolor);

  if (isMonster())
  {
    add_data(data->add_datas(), EUSERDATATYPE_NORMAL_SKILL, m_pCurCFG->dwNormalSkillID);
    add_data(data->add_datas(), EUSERDATATYPE_ROLELEVEL, getLevel());
  }
  if (isWeaponPet())
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
    if (user && user->getWeaponPet().isBuildHand(this->id))
      add_data(data->add_datas(), EUSERDATATYPE_HANDID, user->id);
  }

  NpcAttribute* pAttribute = dynamic_cast<NpcAttribute*>(m_pAttribute);
  if (pAttribute != nullptr)
    pAttribute->toData(data);

  if (define.m_oVar.m_qwFollowerID)
    data->set_owner(define.m_oVar.m_qwFollowerID);
  if (define.m_oVar.m_qwOwnerID)
  {
    bool bServant = MiscConfig::getMe().getServantCFG().isExist(getNpcID());
    if (m_pCurCFG->eNpcType == ENPCTYPE_PETNPC || m_pCurCFG->eNpcType == ENPCTYPE_CATCHNPC || m_pCurCFG->eNpcType == ENPCTYPE_BEING || m_pCurCFG->eNpcType == ENPCTYPE_ELEMENTELF
        || bServant == true || isTrapNpc())
      data->set_owner(define.m_oVar.m_qwOwnerID);
  }

  data->set_behaviour(getBehaviours());

  if (define.getUniqueID())
    data->set_uniqueid(define.getUniqueID());

  data->set_waitaction(define.getWaitAction());

  if (m_ai.m_dwSearchRange)
    data->set_searchrange(m_ai.m_dwSearchRange);

  auto buff = [&data, this](const SBufferData& r)
  {
    BufferData* pData = data->add_buffs();
    if (pData == nullptr)
      return;
    pData->set_id(r.id);
    pData->set_time(r.endTime);
    if (!r.strFromName.empty())
      pData->set_fromname(r.strFromName);
    if (r.fromID != this->id)
      pData->set_fromid(r.fromID);
    if (r.lv != 0)
      pData->set_level(r.lv % 100);
  };
  m_oBuff.foreach(buff);

  if (m_dwActionID)
  {
    data->set_motionactionid(m_dwActionID);
  }
  if (m_strEffectPath.empty() == false)
  {
    data->set_effect(m_strEffectPath);
    data->set_effectpos(m_dwEffectPos);
    data->set_effectpos(m_dwEffectPos);
    data->set_effectindex(m_dwEffectIndex);
  }
  m_oSpEffect.collectSpEffectData(data);

  if (m_bTempJustBirth)
  {
    data->set_isbirth(true);
    m_bTempJustBirth = false;
  }

  if (define.getFadeIn())
  {
    data->set_fadein(define.getFadeIn());
    if (getNpcType() == ENPCTYPE_PETNPC)
      define.setFadeIn(0);
  }
  if (define.m_oVar.m_qwGuildID)
    data->set_guildid(define.m_oVar.m_qwGuildID);

  static const std::set<EUserDataType> typeSet = {
    EUSERDATATYPE_BODY, EUSERDATATYPE_LEFTHAND, EUSERDATATYPE_RIGHTHAND,
    EUSERDATATYPE_HEAD, EUSERDATATYPE_BACK, EUSERDATATYPE_HAIR,
    EUSERDATATYPE_HAIRCOLOR, EUSERDATATYPE_FACE, EUSERDATATYPE_TAIL,
    EUSERDATATYPE_MOUNT, EUSERDATATYPE_MOUTH,
  };
  for (auto &e : typeSet)
  {
    DWORD dwValue = getEquipData(e);
    if (dwValue != 0)
      add_data(data->add_datas(), e, dwValue);
  }

  if (define.getSearch())
    data->set_search(define.getSearch());
  if (getNpcType() == ENPCTYPE_BEING)
  {
    SceneUser* pMaster = getMasterUser();
    if (pMaster)
    {
      const string& strName = pMaster->getUserBeing().getName(getNpcID());
      if (strName.empty() == false)
        data->set_name(strName);
    }
  }

  if (m_eBossType != EBOSSTYPE_MIN)
    data->set_bosstype(static_cast<DWORD>(m_eBossType));
  else if (getNpcZoneType() == ENPCZONE_RAIDDEADBOSS)
    data->set_bosstype(3); // 图标显示
}

void SceneNpc::onNpcDie()
{
  if (getScene() == nullptr || getStatus() != ECREATURESTATUS_REMOVE)
    return;

  m_ai.onNpcDie();
  m_sai.checkSig("death");
  if (isMonster())
  {
    xSceneEntrySet npcset;
    getScene()->getEntryListInNine(SCENE_ENTRY_NPC, getPos(), npcset);
    for (auto &s : npcset)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (npc && npc->isMonster() && npc->isAlive())
      {
        npc->m_sai.checkSig("seedead");
      }
    }
  }

  m_oMove.stop();
  m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_DEAD, id);

  //if (m_pMaster == nullptr)
  SceneUser* m_pMaster = SceneUserManager::getMe().getUserByID(m_ai.getLastAttacker());
  if (m_pMaster == nullptr)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_ai.getLastAttacker());
    if (npc)
    {
      npc->m_sai.checkSig("killmonster");
      m_pMaster = npc->getMasterUser();
    }
  }

  getScene()->onNpcDie(this, m_pMaster);
  ActivityManager::getMe().onNpcDie(this);

  if (!m_bEvo)
    ++m_dwDeadCount;
  m_dwDeadTime = xTime::getCurSec();
  setTrigTalk("die");
  m_oEmoji.check("Die");
  m_oBuff.setClearState();
  m_oSkillProcessor.setClearState();

  //QWORD questUserID = 0;
  if (define.m_oVar.m_qwQuestOwnerID)
  {
    SceneUser *user = SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwQuestOwnerID);
    if (user)
    {
      user->getQuestNpc().onNpcDie(this);
      //questUserID = user->id;
      user->getSeal().onMonsterDie(this);
    }
  }
  if (define.m_oVar.m_qwTeamID != 0)
  {
    SceneTeamSeal* pTeamSeal = TeamSealManager::getMe().getTeamSealByID(define.m_oVar.m_qwTeamID);
    if (pTeamSeal != nullptr)
      pTeamSeal->onMonsterDie(this);
  }

  // if tree npc
  if (define.m_oVar.m_qwNpcOwnerID)
  {
    xSceneEntryDynamic *master = xSceneEntryDynamic::getEntryByID(define.m_oVar.m_qwNpcOwnerID);
    TreeNpc* pTreeNpc = dynamic_cast<TreeNpc*>(master);
    if (pTreeNpc != nullptr)
    {
      getScene()->onNpcDie(this, nullptr);
      pTreeNpc->m_oFollower.removeTreeMonster(this);
    }
  }

  // active tower monster
  TowerMonsterKill cmd;
  cmd.set_monsterid(define.getID());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  // if has summoned 喽啰, 喽啰die
  if (m_oFollower.hasServant())
  {
    TVecQWORD vecIDs;
    m_oFollower.getServantIDs(vecIDs);
    for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
    {
      SceneNpc* pSer = SceneNpcManager::getMe().getNpcByTempID(*v);
      if (pSer == nullptr)
        continue;

      //若有复活属性,取消复活属性
      //pSer->define.setLife(1);
      //pSer->m_dwReliveCount = 0;

      pSer->attackMe(pSer->getAttr(EATTRTYPE_HP), m_pMaster);
      if (pSer->getStatus() != ECREATURESTATUS_REMOVE)
      {
        pSer->setAttr(EATTRTYPE_HP, 0);
        pSer->setStatus(ECREATURESTATUS_REMOVE);
      }
    }
  }

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(getNpcID());
  if(m_pMaster != nullptr && pCFG != nullptr && pCFG->vecRelevancyIDs.empty() == false)
  {
    for(auto m = pCFG->vecRelevancyIDs.begin(); m != pCFG->vecRelevancyIDs.end(); ++m)
      m_pMaster->getManual().onKillMonster(*m);
  }
  m_oSkillStatus.onDie();

  if(m_pMaster != nullptr)
    m_pMaster->getServant().onKillNpc(this);
  if (getNpcType() == ENPCTYPE_MVP)
  {
    if(m_pMaster != nullptr)
      m_pMaster->getServant().onFinishEvent(ETRIGGER_KILL_MVP);
    XLOG << "[Mvp], 进入死亡状态" << name << id << XEND;
  }
  else if(getNpcType() == ENPCTYPE_MINIBOSS && m_pMaster != nullptr)
    m_pMaster->getServant().onFinishEvent(ETRIGGER_KILL_MINI);
}

void SceneNpc::setFollowerFace()
{
  if (isAlive() && define.m_oVar.m_qwFollowerID!=0)
  {
    this->m_oEmoji.play(randBetween(0, 1000));
  }
}

void SceneNpc::sendTalkInfo(DWORD talkid)
{
  DWORD cur = now();
  if (cur < m_talkLastTime + 3)
    return;
  if (TableManager::getMe().getDialogCFG(talkid) == nullptr)
    return;
  if (getAttr(EATTRTYPE_HIDE) > 0)
    return;

  m_talkLastTime = cur;

  TalkInfo cmd;
  cmd.set_guid(this->id);
  cmd.set_talkid(talkid);

  PROTOBUF(cmd, send, len);
  sendCmdToNine(send, len);
}

void SceneNpc::setTrigTalk(const string& type)
{
  if (m_pCurCFG == nullptr)
    return;
  DWORD talkid = m_pCurCFG->talk.getTalkByType(type);
  if (talkid == 0) return;
  if ((DWORD)randBetween(1,100) > m_pCurCFG->talk.odds) return;

  sendTalkInfo(talkid);
}

void SceneNpc::setTimeTalk(DWORD cur)
{
  if (m_pCurCFG == nullptr)
    return;
  //follow others
  auto v = find_if(m_vecTime2Talk.begin(), m_vecTime2Talk.end(), [&cur](const pair<DWORD, DWORD> &r) ->bool {
      return r.first <= cur;
      });
  if (v != m_vecTime2Talk.end())
  {
    sendTalkInfo(v->second);
    m_vecTime2Talk.erase(v);
    return;
  }

  //自己按时间说话
  if (cur < m_talkNextTime)
    return;

  const NpcTalk& talk = m_pCurCFG->talk;
  m_talkNextTime = cur + randBetween(talk.talkTime.first, talk.talkTime.second);

  if ((DWORD)randBetween(1,100) > talk.odds) return;

  DWORD talkNum = talk.normalTalkIDs.size();
  if (talkNum == 0) return;
  DWORD talkid = randBetween(0, 1000) % talkNum;
  talkid = talk.normalTalkIDs[talkid];

  sendTalkInfo(talkid);

  //使跟随者说话
  auto m = talk.mapTalk2Follow.find(talkid);
  if (m != talk.mapTalk2Follow.end())
  {
    xSceneEntrySet uSet;
    getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, getPos(), m->second.dwRange, uSet);

    auto getnpcs = [&](set<SceneNpc*> & npcset, DWORD baseid)
    {
      for (auto &s : uSet)
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
        if (npc && npc->getNpcID() == baseid)
          npcset.insert(npc);
      }
    };

    DWORD timeToFol = cur;
    for (auto &v : m->second.vecNpcTalkID)
    {
      set<SceneNpc*> npcset;
      getnpcs(npcset, v.first);
      if (npcset.empty())
        break;
      timeToFol += 2;
      for (auto &s : npcset)
        s->setTimeTalk(v.second, timeToFol);
    }
  }
}

void SceneNpc::checkEvoState(DWORD cur)
{
  if (m_ai.getCurLockID() != 0 || isAlive() == false)
    return;
  if (m_pCurCFG == nullptr || m_pCurCFG->dwEvoID == 0)
    return;
  if (cur - m_dwEvoTime < m_pCurCFG->dwEvoTime)
    return;
  //PurifyNpc* npc = getPurifyNpc();
  //if (npc != nullptr && npc->canSetClear() == false)
  //  return;
  const SNpcCFG* pEvoCFG = NpcConfig::getMe().getNpcCFG(m_pCurCFG->dwEvoID);
  if (pEvoCFG == nullptr)
    return;
  const SceneObject* pObject = getScene()->getSceneObject();
  if (pObject == nullptr)
    return;
  const SceneNpcTemplate* pTemplate = pObject->getNpcTemplate(pEvoCFG->dwID);
  if (pTemplate == nullptr)
  {
    XERR << "[魔物进化-进化]" << id << m_pCurCFG->dwID << name << "进化为" << pEvoCFG->dwID << pEvoCFG->strName << "未找到模板" << XEND;
    return;
  }

  m_dwEvoTime = xTime::getCurSec();
  DWORD dwRand = randBetween(0, 100);
  if (dwRand >= m_pCurCFG->dwEvoRate)
    return;

  evo(pEvoCFG, pTemplate->m_oDefine);
}

void SceneNpc::refreshMe(QWORD curMSec)
{
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);
  switch (getStatus())
  {
    case ECREATURESTATUS_LIVE:
      status_live(curMSec);
      break;
    case ECREATURESTATUS_REMOVE:
      status_remove();
      break;
    case ECREATURESTATUS_DEAD:
      status_dead(curSec);
      break;
    case ECREATURESTATUS_RELIVE:
      status_relive(curSec);
      break;
    case ECREATURESTATUS_LEAVE:
      status_leave();
      break;
    case ECREATURESTATUS_EVO:
      status_evo(curSec);
      break;
    case ECREATURESTATUS_SUICIDE:
      status_suicide();
      break;
    default:
      break;
  }

  updateData(curSec);
  randPos(curSec);
}

void SceneNpc::status_live(QWORD curMSec)
{
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);
  if (m_dwSetClearTime != 0 && m_dwSetClearTime < curSec)
  {
    setStatus(ECREATURESTATUS_REMOVE);
    return;
  }
  xSceneEntryDynamic::refreshMe(curMSec);
  m_ai.timer();
}

void SceneNpc::status_remove()
{
  if (m_bDead == false)
  {
    m_bDead = true;
    if (getAttr(EATTRTYPE_HP) != 0)
      SceneNpc::onNpcDie();
    else
      onNpcDie();
  }

  //播放死亡传送特效
  if (m_deadEffect == true)
  {
    const SEffectPath& configPath = MiscConfig::getMe().getEffectPath();
    xLuaData data;
    data.setData("type", "effect");
    data.setData("effect", configPath.strLeaveSceneEffect);
    data.setData("posbind", 1);
    GMCommandRuler::getMe().execute(this, data);

    xLuaData sound;
    sound.setData("type", "sound_effect");
    sound.setData("sync", 1);
    sound.setData("se", configPath.strLeaveSceneSound);
    GMCommandRuler::getMe().execute(this, sound);
  }

  setStatus(ECREATURESTATUS_DEAD);
  m_oBuff.update(xTime::getCurMSec()); // 死亡时清空buff
  m_oSpEffect.timer(xTime::getCurSec());
}

void SceneNpc::status_dead(DWORD curSec)
{
  if (getNpcType() == ENPCTYPE_MVP && m_bHaveSendMvpNotice == false)
  {
    DWORD time = MiscConfig::getMe().getMvpScoreCFG().dwDeadNoticeTime;
    if (curSec >= m_dwDeadTime + time)
    {
      m_bHaveSendMvpNotice = true;
      sendMvpNotice();
      // send notice
    }
  }

  DWORD dispTime = MiscConfig::getMe().getMonsterCFG().getDisappearTime(getNpcType());
  if (isTrapNpc())
    dispTime = 0;
  if (MiscConfig::getMe().getSealCFG().dwSealNpcID == getNpcID())
    dispTime = 0;
  if (getNpcType() == ENPCTYPE_MONSTER)
    dispTime = 0;
  if (getSyncDead() == false)    //播放传送特效，马上移除
    dispTime = 0;
  //if (m_bEvo)
    //dispTime = 0;
  if (define.getDeadDispTime())
    dispTime = define.getDeadDispTime();

  if (m_bDeadDelAtonce)
    dispTime = 0;

  if (m_dwDeadTime + dispTime > curSec)
    return;

  Scene* pScene = getScene();
  if (pScene == nullptr)
    return;
  leaveScene();

  /*SceneNpc* pOwner = SceneNpcManager::getMe().getNpcByTempID(define.m_oVar.m_qwNpcOwnerID);
  if (pOwner != nullptr)
  {
    setStatus(ECREATURESTATUS_IDLE);
    return;
  }*/

  //if (canRelive() == true)
  bool canRelive = (define.getLife() == 0 || (define.getLife() != 1 && m_dwDeadCount < define.getLife()));
  if (define.m_oVar.m_qwNpcOwnerID != 0)
  {
    // 若大怪死亡, 小怪不再复活
    SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(define.m_oVar.m_qwNpcOwnerID);
    if (pMaster == nullptr || !pMaster->isAlive())
      canRelive = false;
  }
  if (canRelive)
  {
    /*
    DWORD nineUserNum = 0;
    DWORD nineMonsterNum = 0;
    xSceneEntrySet userset;
    xSceneEntrySet npcset;
    pScene->getEntryListInNine(SCENE_ENTRY_USER, getPos(), userset);
    nineUserNum = userset.size();

    pScene->getEntryListInNine(SCENE_ENTRY_NPC, getPos(), npcset);
    for (auto &s : npcset)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (npc && npc->isMonster() && npc->isAlive())
        ++nineMonsterNum;
    }
    */

    if (m_pOriCFG)
      m_dwReliveTime = LuaManager::getMe().call<DWORD>("CalcServerMonsterReload", m_pOriCFG->dwID, (DWORD)getNpcType(), pScene->getMapID(), (DWORD)(curSec - getBirthTime() / ONE_THOUSAND), m_oOriDefine.getReborn(), isStarMonster());
    if (m_bEvo)
      m_dwReliveTime = 0;
    setStatus(ECREATURESTATUS_RELIVE);
    return;
  }

  setStatus(ECREATURESTATUS_CLEAR);
  pScene->delSummonNpc(id);
}

void SceneNpc::status_relive(DWORD curSec)
{
  DWORD dispTime = MiscConfig::getMe().getMonsterCFG().getDisappearTime(getNpcType());
  if (getNpcType() == ENPCTYPE_MONSTER)
    dispTime = 0;
  if (m_bEvo)
    dispTime = 0;

  if (m_dwDeadTime + dispTime + m_dwReliveTime > curSec)
    return;
  relive();
}

void SceneNpc::status_evo(DWORD curSec)
{
  leaveScene();
  setStatus(ECREATURESTATUS_RELIVE);
}

void SceneNpc::status_suicide()
{
  setAttr(EATTRTYPE_HP, 0);
  setStatus(ECREATURESTATUS_DEAD);
}

void SceneNpc::status_leave()
{
  leaveScene();
  setStatus(ECREATURESTATUS_CLEAR);
}

void SceneNpc::removeAtonce()
{
  leaveScene();
  setStatus(ECREATURESTATUS_CLEAR);
}

void SceneNpc::setDeadRemoveAtonce()
{
  // inform client del at first
  delMeToNine();

  setClearState();
}

bool SceneNpc::beAttack(QWORD damage, xSceneEntryDynamic* attacker)
{
  Scene* pScene = getScene();
  if (pScene == nullptr || m_pCurCFG == nullptr)
    return false;

  if (beHitFirst)
  {
    setTrigTalk("beHitFirst");
    beHitFirst = false;
  }

  m_oEmoji.check("BeAttack");

  //check owner
  DWORD curTime = now();
  if (damage != 0 && curTime > m_oOwner.second)
  {
    if (curTime > m_oOwner.second + 10)
    {
      if (m_oOwner.first != attacker->id)
      {
        XINF << "[npc 归属] 归属切换，切换前归属:" << m_oOwner.first << "切换后归属：" << attacker->id << "当前时间:" << curTime << XEND;
        m_oOwner.first = attacker->id;
      }
    }
    if (m_oOwner.first == attacker->id)
    {
      m_oOwner.second = curTime;
    }
  }

  bool bField = m_pCurCFG && m_pCurCFG->isFieldMonster();
  if (attacker->getEntryType() == SCENE_ENTRY_USER)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*> (attacker);
    if (pUser != nullptr)
    {
      if (damage > 0)
      {
        pUser->getShare().onHurt(name, damage);
        pUser->addTeamSkillTimePoint(curTime);
        DamageNpcUserEvent cmd;
        cmd.set_npcguid(id);
        cmd.set_userid(pUser->id);
        PROTOBUF(cmd, send, len);
        if (check2PosInNine(pUser))
          pUser->sendCmdToMe(send, len);

        if (bField)
          pUser->markHitFieldMonster();
        // inform team member if attacker is user
        const GTeam& rTeam = pUser->getTeam();
        if (rTeam.getTeamID() != 0)
        {
          for (auto &m : rTeam.getTeamMemberList())
          {
            if (m.second.charid() == pUser->id)
              continue;
            SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
            if (user == nullptr || user->getScene() != pUser->getScene())
              continue;

          user->getWeaponPet().onTeamerAttack(pUser, this);

          if (bField)
            user->markHitFieldMonster();

          if (check2PosInNine(user) == false)
            continue;
          user->sendCmdToMe(send, len);
          }
        }
      }
      pUser->getWeaponPet().onUserAttack(this);
      pUser->getUserPet().onUserAttack(this);
      pUser->getUserBeing().onUserAttack(this);
      pUser->getServant().onUserAttack(this);
      pUser->getUserElementElf().onUserAttack(this);
    }
  }

  // check cat
  QWORD qwUserID = attacker->id;

  SceneUser* beingMaster = nullptr;
  if (attacker->getEntryType() == SCENE_ENTRY_NPC)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*> (attacker);
    if (pNpc)
    {
      if (pNpc->getNpcType() == ENPCTYPE_WEAPONPET || pNpc->getNpcType() == ENPCTYPE_PETNPC || pNpc->getNpcType() == ENPCTYPE_BEING || pNpc->getNpcType() == ENPCTYPE_ELEMENTELF)
        qwUserID = pNpc->define.m_oVar.m_qwOwnerID;

      if (pNpc->getNpcType() == ENPCTYPE_BEING)
      {
        BeingNpc* being = dynamic_cast<BeingNpc*>(pNpc);
        if (being)
        {
          beingMaster = being->getMasterUser();
          if (beingMaster && bField)
          {
            beingMaster->markHitFieldMonster();
            beingMaster->addTeamSkillTimePoint(curTime);
          }
        }
      }
    }
  }

  addUserDamage(qwUserID, damage);
  SceneUser* pUser = dynamic_cast<SceneUser*>(attacker);
  if (pUser != nullptr)
    pUser->getAchieve().onDamage(getNpcID(), damage);
  if (pScene->isPVPScene() && MiscConfig::getMe().getArenaCFG().dwMetalNpcID == getNpcID() && attacker->getEntryType() == SCENE_ENTRY_USER)
  {
    GlamMetalScene* pGlamScene = dynamic_cast<GlamMetalScene*>(pScene);
    if (pGlamScene)
      pGlamScene->addNpcDamage(attacker->id, damage);
  }
  /*if (pScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
  {
    GuildFireScene* pGScene = dynamic_cast<GuildFireScene*> (pScene);
    if (pGScene)
      pGScene->onNpcBeAttack(this);
  }
  */

  if (m_pCurCFG->eNpcType == ENPCTYPE_GATHER || getAttr(EATTRTYPE_HP) <= 0)
  {
    setAttr(EATTRTYPE_HP, 0);
    setStatus(ECREATURESTATUS_REMOVE);
    //m_pMaster = attacker;
    if (getNpcType() == ENPCTYPE_MVP)
      XLOG << "[MVP-击杀],血量为0" << "怪物:" << name << id << "最后一次攻击者:" << attacker->name << attacker->id << XEND;
  }

  if(getNpcType() == ENPCTYPE_MVP || getNpcType() == ENPCTYPE_MINIBOSS)
  {
    if(pUser != nullptr && pUser->getAltmanFashion() == true && !pUser->getTransform().isInTransform()
        && pUser->m_oBuff.haveBuff(MiscConfig::getMe().getAltmanCFG().dwFashionBuff) == false)
      pUser->m_oBuff.add(MiscConfig::getMe().getAltmanCFG().dwFashionBuff);
  }

  if (isMonster() == false)
    return true;

  m_ai.setLastAttacker(attacker->id);
  m_sai.checkSig("hpchange");
  m_sai.checkSig("hpless");


  if (isAlive() == false)
    return true;

  if (beingMaster && attacker->id != id && beingMaster->id != m_ai.getCurLockID()) // 生命体攻击时, 怪物将攻击目标指向其主人
    m_ai.putAttacker(beingMaster);
  else if (attacker->id != m_ai.getCurLockID())  // 更新攻击者
    m_ai.putAttacker(attacker);

  m_ai.beAttack(attacker);
  return true;
}

SQWORD SceneNpc::changeHp(SQWORD value, xSceneEntryDynamic* entry, bool bshare /*=false*/, bool bforce /*=false*/)
{
  if (entry == nullptr)
    return 0;
  if (m_dwProtectInterval && m_dwProtectMaxHp && value < 0)
  {
    if ((DWORD)(-value) > m_dwProtectMaxHp)
      value = -(SQWORD)m_dwProtectMaxHp;

    DWORD cur = now();
    if (cur >= m_dwProtectTimeTick + m_dwProtectInterval)
    {
      m_dwProtectTimeTick = cur;
      m_dwLostHpInProtect = 0;
    }
    else
    {
      if (m_dwLostHpInProtect >= m_dwProtectMaxHp)
      {
        XLOG << "[怪物-掉血保护], 免疫本次掉血, 怪物:" << getNpcID() << name << id << "免疫:" << m_dwLostHpInProtect << m_dwProtectMaxHp << value << XEND;
        return 0;
      }
      else
      {
        m_dwLostHpInProtect += (-value);
      }
    }
  }

  SQWORD oldHp = getAttr(EATTRTYPE_HP);
  SQWORD ret = value + oldHp;
  if (ret < 0) ret = 0;

  SQWORD maxHp = getAttr(EATTRTYPE_MAXHP);

  if (ret > maxHp)
    ret = maxHp;

  if (maxHp)
  {
    SQWORD oldPer = oldHp * 100 / maxHp;
    SQWORD newPer = ret * 100 / maxHp;
    if (oldPer > 25 && newPer < 25)
    {
      m_oEmoji.check("HP25");
    }
    else if (oldPer > 50 && newPer < 50)
    {
      m_oEmoji.check("HP50");
    }
    else if (oldPer > 75 && newPer < 75)
    {
      m_oEmoji.check("HP75");
    }
  }

  m_dwLastHp = getAttr(EATTRTYPE_HP);
  setAttr(EATTRTYPE_HP, ret);

  if (ret == 0)
    setStatus(ECREATURESTATUS_REMOVE);

  /*Cmd::ChangeHpUserCmd message;
  message.set_charid(id);
  message.set_hp(ret);
  PROTOBUF(message, send, len);
  sendCmdToNine(send, len);*/

  m_oBuff.onHpChange();
  refreshDataAtonce();

  Scene* pScene = getScene();
  if (pScene && pScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
  {
    GuildFireScene* pGScene = dynamic_cast<GuildFireScene*> (pScene);
    if (pGScene)
    {
      if (value < 0)
      {
                //华丽金属
        if (getNpcID() == 40021)
        {
          DWORD maxhp = getAttr(EATTRTYPE_MAXHP);
          DWORD per = maxhp ? (-value) * 10000 / maxhp : 0;
          SceneUser* user = dynamic_cast<SceneUser*> (entry);
          if (user)
            user->getUserGvg().onDamMetal(per);
          else
          {
            SceneNpc* npc = dynamic_cast<SceneNpc*> (entry);
            if (npc)
            {
              user = npc->getMasterUser();
              if (user)
                user->getUserGvg().onDamMetal(per);
            }
          }
        }
      }
    }
  }
  if (pScene)
  {
    if (value < 0)
      pScene->onNpcBeAttack(this, entry, -value);
    else if (value > 0)
      pScene->onNpcBeHeal(this, entry, value);
  }

  // share hp
  if (!bshare && m_bShareDamage)
  {
    SceneNpc* pOwner = nullptr;
    if (m_oFollower.hasServant())
      pOwner = this;
    else
      pOwner = SceneNpcManager::getMe().getNpcByTempID(define.m_oVar.m_qwNpcOwnerID);
    if (pOwner && pOwner->isShareDam())
    {
      std::list<SceneNpc*> sharelist;
      if (pOwner != this)
        sharelist.push_back(pOwner);

      TVecQWORD vecIDs;
      pOwner->m_oFollower.getServantIDs(vecIDs);
      for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
      {
        if (*v == this->id)
          continue;
        SceneNpc* pSer = SceneNpcManager::getMe().getNpcByTempID(*v);
        if (pSer == nullptr)
          continue;
        if (pSer->isShareDam())
          sharelist.push_back(pSer);
      }

      for (auto &npc : sharelist)
      {
        if (npc->isAlive() == false)
          continue;
        npc->changeHp(value, entry, true);

        if (entry->getEntryType() == SCENE_ENTRY_USER)
        {
          DamageNpcUserEvent cmd;
          cmd.set_npcguid(npc->id);
          cmd.set_userid(entry->id);
          PROTOBUF(cmd, send, len);
          if (npc->check2PosInNine(entry))
            entry->sendCmdToMe(send, len);
        }
      }
    }
  }
  return oldHp - ret;
}

float SceneNpc::getMoveSpeed()
{
  if (define.m_oVar.m_qwFollowerID)
  {
    xSceneEntryDynamic *master = xSceneEntryDynamic::getEntryByID(define.m_oVar.m_qwFollowerID);
    if (master)
    {
      return master->m_oFollower.getMoveSpeed(this->id);
    }
  }
  return xSceneEntryDynamic::getMoveSpeed();
}

bool SceneNpc::relive()
{
  if (isAlive() == true)
    return false;
  /*if (canRelive() == false)
  {
    setStatus(ECREATURESTATUS_CLEAR);
    return false;
  }*/

  if (m_bEvo)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwEvoID);
    if (pCFG == nullptr)
      return false;
    m_pCurCFG = pCFG;
    define = m_oEvoDef;
    define.setLife(m_oOriDefine.getLife());
    define.setReborn(m_oOriDefine.getReborn());
    define.setPos(getPos());
    define.setRange(0);
  }

  const SNpcCFG* pReliveCFG = nullptr;
  if (m_bEvo)
    pReliveCFG = m_pCurCFG == nullptr ? m_pOriCFG : m_pCurCFG;
  else
    pReliveCFG = m_pOriCFG;

  const NpcDefine& def = !m_bEvo ? m_oOriDefine : define;
  if (init(pReliveCFG, def) == false)
    return false;

  if (m_bEvo)
  {
    m_ai.setCurLockID(m_ai.getLastAttacker());
    m_ai.setPriAttackUser(m_ai.getLastAttacker());
  }
  m_bEvo = false;

  // update guid for a NEW monster to client logic processing
  if (changeGuid() == false)
    return false;

  setStatus(ECREATURESTATUS_LIVE);
  Scene* pScene = SceneManager::getMe().getSceneByID(m_dwSceneID);
  if (pScene == nullptr)
  {
    setStatus(ECREATURESTATUS_CLEAR);
    return false;
  }
  if (!enterScene(pScene))
  {
    XERR << "[NPC]" << id << getNpcID() << getName() << "进入场景失败" << XEND;
  }

  if (define.m_oVar.m_qwNpcOwnerID != 0)
  {
    SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(define.m_oVar.m_qwNpcOwnerID);
    if (pMaster && pMaster->isAlive() && pMaster->m_ai.getCurLockID())
      m_ai.setCurLockID(pMaster->m_ai.getCurLockID());
  }

  m_bDead = false;
  return true;
}

void SceneNpc::updateData(DWORD curTime)
{
  if (m_pCurCFG == nullptr || getScene() == nullptr)
    return;

  if (curTime - m_dwDataTick > 10)
  {
    m_dwDataTick = curTime;

    NpcDataSync cmd;
    fetchChangeData(cmd);

    if (cmd.attrs_size() > 0 || cmd.datas_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      getScene()->sendCmdToNine(getPos(), send, len);

#ifdef _DEBUG
      for (int i = 0; i < cmd.attrs_size(); ++i)
        XDBG << "[NPC-属性同步]" << id << getNpcID() << name << "更新 :" << cmd.attrs(i).ShortDebugString() << XEND;
      for (int i = 0; i < cmd.datas_size(); ++i)
        XDBG << "[NPC-数据同步]" << id << getNpcID() << name << "更新 :" << cmd.datas(i).ShortDebugString() << XEND;
#endif
    }
  }
}

void SceneNpc::fetchChangeData(NpcDataSync& cmd)
{
  if (m_pCurCFG == nullptr)
    return;
  cmd.set_guid(tempid);

  NpcAttribute* pAttribute = dynamic_cast<NpcAttribute*>(m_pAttribute);
  if (pAttribute != nullptr)
    pAttribute->collectSyncAttrCmd(cmd);

  if (m_bitset.any() == false)
    return;

  EUserDataType eType = EUSERDATATYPE_STATUS;

  if (getSyncDead() == true)
  {
    if (m_bitset.test(eType) == true && (getStatus() == ECREATURESTATUS_DEAD || getStatus() == ECREATURESTATUS_LIVE))
      add_data(cmd.add_datas(), eType, getStatus());
  }

  eType = EUSERDATATYPE_SHADERCOLOR;
  if (m_bitset.test(eType) == true)
  {
    DWORD shadercolor = m_oBuff.getShaderColor();
    shadercolor = shadercolor != 0 ? shadercolor : m_pCurCFG->figure.shadercolor;
    add_data(cmd.add_datas(), eType, shadercolor);
  }

  eType = EUSERDATATYPE_BODYSCALE;
  if (m_bitset.test(eType) == true)
    add_data(cmd.add_datas(), eType, getScale());

  eType = EUSERDATATYPE_ALPHA;
  if (m_bitset.test(eType) == true)
    add_data(cmd.add_datas(), eType, getCrystal());

  static const std::set<EUserDataType> typeSet = {
    EUSERDATATYPE_BODY, EUSERDATATYPE_LEFTHAND, EUSERDATATYPE_RIGHTHAND,
    EUSERDATATYPE_HEAD, EUSERDATATYPE_BACK, EUSERDATATYPE_HAIR,
    EUSERDATATYPE_HAIRCOLOR, EUSERDATATYPE_FACE, EUSERDATATYPE_TAIL,
    EUSERDATATYPE_MOUNT, EUSERDATATYPE_MOUTH,
  };
  for (auto &e : typeSet)
  {
    if (m_bitset.test(e))
      add_data(cmd.add_datas(), e, getEquipData(e));
  }
  if (m_bitset.test(EUSERDATATYPE_NAME) == true)
  {
    bool s = false;
    if (getNpcType() == ENPCTYPE_BEING)
    {
      SceneUser* pMaster = getMasterUser();
      if (pMaster)
      {
        const string& strName = pMaster->getUserBeing().getName(getNpcID());
        if (strName.empty() == false)
        {
          add_data(cmd.add_datas(), EUSERDATATYPE_NAME, 0, strName);
          s = true;
        }
      }
    }
    if (s == false)
      add_data(cmd.add_datas(), EUSERDATATYPE_NAME, 0, name);
  }

  m_bitset.reset();
}

void SceneNpc::setScale(float f)
{
  if (f <= 0.1f || f == m_flScale)
    return;

  m_flScale = f;
  setDataMark(EUSERDATATYPE_BODYSCALE);
  refreshDataAtonce();
}

DWORD SceneNpc::getScale()
{
  float buffscaleper = m_oBuff.getBodyScalePer();
  if (buffscaleper == 0)
    buffscaleper = 1;
  return m_flScale * 100 * buffscaleper;
}

DWORD SceneNpc::getCrystal()
{
  DWORD crystal = m_oBuff.getPartTransform(EUSERDATATYPE_ALPHA);
  return crystal ? crystal : ONE_THOUSAND;
}

void SceneNpc::randPos(DWORD curSec)
{
  if (m_pCurCFG == nullptr || m_pCurCFG->dwRandPosTime == 0 || getScene() == nullptr || curSec < m_dwRandPosTime)
    return;

  m_dwRandPosTime = curSec + m_pCurCFG->dwRandPosTime;

  const SEffectPath& configPath = MiscConfig::getMe().getEffectPath();

  xLuaData data;
  data.setData("type", "effect");
  data.setData("effect", configPath.strLeaveSceneEffect);
  data.setData("posbind", 1);
  GMCommandRuler::getMe().execute(this, data);

  xLuaData sound;
  sound.setData("type", "sound_effect");
  sound.setData("sync", 1);
  sound.setData("se", configPath.strLeaveSceneSound);
  GMCommandRuler::getMe().execute(this, sound);

  xPos oOriPos = getPos();
  xPos oPos;
  getScene()->getRandPos(oPos);

  goTo(oPos, false);
  XDBG << "[场景npc-随机位置]" << id << getNpcID() << name << "位置由(" << oOriPos.getX() << oOriPos.getY() << oOriPos.getZ() << ")随机变换到(" << getPos().getX() << getPos().getY() << getPos().getZ() << ")" << XEND;
}

bool SceneNpc::checkNineScreenShow(xSceneEntryDynamic* entry)
{
  if (!entry) return false;

  if (isMask()) return false;
  if (isVisableToAll())
  {
    return true;
  }
  else
  {
    SceneUser* user = dynamic_cast<SceneUser *> (entry);
    return isVisableToSceneUser(user);
  }

  return true;
}

bool SceneNpc::isImmuneSkill(DWORD skillid)
{
  if (m_pCurCFG == nullptr)
    return false;
  if (!m_pCurCFG->setImmuneSkills.empty())
  {
    if (m_pCurCFG->setImmuneSkills.find(skillid) != m_pCurCFG->setImmuneSkills.end())
      return true;
  }
  if (m_ai.isFly())
  {
    const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
    const TVecDWORD& vecIDs = rCFG.dwFly.vecImmuneSkill;
    auto v = find(vecIDs.begin(), vecIDs.end(), skillid);
    if (v != vecIDs.end())
      return true;
  }
  return false;
}

void SceneNpc::addUserDamage(QWORD qwUserID, DWORD dwDamage)
{
  if (qwUserID == 0)
  {
    XERR << "[场景怪物-伤害]" << id << getNpcID() << getName() << "收到伤害" << dwDamage << "失败,userid" << qwUserID << "不合法" << XEND;
    return;
  }
  m_mapUserDamage[qwUserID] += dwDamage;

  if (getNpcType() == ENPCTYPE_MVP)
  {
    addMvpHitTime(qwUserID);

    auto it = m_mapUser2MvpInfo.find(qwUserID);
    if (it != m_mapUser2MvpInfo.end())
    {
      it->second.m_dwHitDamage += dwDamage;
      it->second.m_dwLastMarkTime = now();
    }
    else
    {
      SceneUser* user = SceneUserManager::getMe().getUserByID(qwUserID);
      if (user == nullptr)
        return;
      SMvpKillInfo& sInfo = m_mapUser2MvpInfo[qwUserID];
      sInfo.charid = qwUserID;
      sInfo.name = user->name;
      sInfo.m_dwHitDamage = dwDamage;
      sInfo.m_dwLastMarkTime = now();
    }
  }
}

DWORD SceneNpc::getBehaviours()
{
  if (m_pCurCFG == nullptr)
    return 0;
  if (define.m_oVar.m_qwFollowerID)
    return define.getBehaviours();
  return define.getBehaviours() | m_pCurCFG->dwBehaviours;
}

void SceneNpc::sendMeToUser(SceneUser* pUser)
{
  if (pUser == nullptr || check2PosInNine(pUser) == false)
    return;
  if (!getScene()) return;
  Cmd::AddMapNpc cmd;
  fillMapNpcData(cmd.add_npcs());
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  sendGearStatus(pUser);
  sendExtraInfo(pUser);
}

void SceneNpc::delMeToUser(SceneUser* pUser)
{
  if (pUser == nullptr || check2PosInNine(pUser) == false)
    return;

  if (!getScene()) return;
  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);
  //if (isMonster() == false)
    //cmd.set_fadeout(MiscConfig::getMe().getSystemCFG().dwNpcFadeoutTime);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
}

void SceneNpc::informUserAdd(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;
  MusicNpc* pMusicNpc = dynamic_cast<MusicNpc*>(this);
  if (pMusicNpc != nullptr)
    pUser->setMusicData(pMusicNpc->getMusicID(), pMusicNpc->getStartTime(), pMusicNpc->getDemanID() == pUser->id, getPos(), pMusicNpc->getLoop());
  if (pUser->getStatus() == ECREATURESTATUS_DEAD || pUser->getStatus() == ECREATURESTATUS_FAKEDEAD)
    m_ai.onSeeDead(pUser->id);

  sendExtraInfo(pUser);
}

void SceneNpc::informUserDel(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;
  MusicNpc* pMusicNpc = dynamic_cast<MusicNpc*>(this);
  if (pMusicNpc != nullptr)
    pUser->setMusicData(0, 0, false);

  m_ai.onUserLeaveNine(pUser->id);
}

void SceneNpc::sendGearStatus(xSceneEntryDynamic* entry)
{
  DWORD status = m_dwGearStatus;

  if (m_bSpecialGearStatus)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(entry);
    if (pUser != nullptr)
    {
      status = pUser->getGuildGateGearStatus(this);
      if (status == 0) status = m_dwGearStatus;
    }
  }

  if (status == 0 || entry == nullptr)
    return;

  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  message.set_value(status);
  message.set_charid(this->id);
  PROTOBUF(message, gsend, glen);
  entry->sendCmdToMe(gsend, glen);
}

void SceneNpc::playGearStatusToNine(DWORD actionid, bool bSave /*=true*/)
{
  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  message.set_value(actionid);
  message.set_charid(this->id);
  PROTOBUF(message, gsend, glen);
  sendCmdToNine(gsend, glen);
  if (bSave)
    m_dwGearStatus = actionid / ONE_THOUSAND * ONE_THOUSAND + 2;
}

bool SceneNpc::isVisableToAll()
{
  if (m_pCurCFG == nullptr)
    return false;

  if (define.m_oVar.m_qwQuestOwnerID) return false;

  //PurifyNpc* pPurifyNpc = getPurifyNpc();
  //if (pPurifyNpc != nullptr)
  //  if (pPurifyNpc->needPurify() && getStatus() == ECREATURESTATUS_DEAD/*npcstate == NPC_STATE_DEATH*/) return false;
  if (define.m_oVar.m_qwTeamUserID) return false;
  if (define.m_oVar.m_qwTeamID) return false;
  if (m_pCurCFG->bHide) return false;
  if (isTrapNpc()) return false;
  if (define.m_oVar.m_qwItemUserID) return false;
  if (define.m_oVar.m_dwBuffID) return false;
  if (m_pCurCFG->bNormalDisplay || m_pCurCFG->bNormalHide) return false;
  if (m_pCurCFG->eNpcType == ENPCTYPE_WEAPONPET) return false;
  if (m_pCurCFG->eNpcType == ENPCTYPE_PETNPC && getScene() && getScene()->isGvg()) return false;

  return true;
}

bool SceneNpc::isVisableToSceneUser(SceneUser *user)
{
  if (!user || !m_pCurCFG) return false;

  if (define.m_oVar.m_qwQuestOwnerID)
  {
    return user->id == define.m_oVar.m_qwQuestOwnerID;
  }
  //PurifyNpc* pPurifyNpc = getPurifyNpc();
  //if (pPurifyNpc != nullptr)
  //{
    //if (pPurifyNpc->needPurify() && getStatus() == ECREATURESTATUS_DEAD/*npcstate == NPC_STATE_DEATH*/)
    //{
      //return pPurifyNpc->canPurifyBy(user->id);
    //}
  //}
  if(define.getID() == NPC_DREAMMIRROR_1)
    if(user->getQuest().isSubmit(QUEST_DREAMMIRROR_1))
      return true;

  if(define.getID() == NPC_DREAMMIRROR_2)
    if(user->getQuest().isSubmit(QUEST_DREAMMIRROR_2))
      return true;

  if(define.getID() == NPC_DREAMMIRROR_3)
    if(user->getQuest().isSubmit(QUEST_DREAMMIRROR_3))
      return true;

  if (define.m_oVar.m_qwTeamUserID)
  {
    if (user->id == define.m_oVar.m_qwTeamUserID)
    {
      return true;
    }
    else
    {
      const GTeam& rTeam = user->getTeam();
      if (rTeam.getTeamID() != 0)
      {
        for (auto &m : rTeam.getTeamMemberList())
        {
          if (define.m_oVar.m_qwTeamUserID == m.second.charid())
            return true;
        }
      }
      return false;
    }
  }
  if (define.m_oVar.m_qwTeamID)
  {
    return user->getTeamID() == define.m_oVar.m_qwTeamID;
  }
  if (m_pCurCFG->bHide)
  {
    if (user->getUserSceneData().getShowNpcs().find(m_pCurCFG->dwID) != user->getUserSceneData().getShowNpcs().end())
      return true;
    const GTeam& rTeam = user->getTeam();
    if (rTeam.getTeamID() != 0)
    {
      for (auto &m : rTeam.getTeamMemberList())
      {
        if (user->id == m.second.charid())
          continue;
        SceneUser* teamer = SceneUserManager::getMe().getUserByID(m.second.charid());
        if (teamer == nullptr || teamer->getScene() != getScene())
          continue;
        if (teamer->getUserSceneData().getShowNpcs().find(m_pCurCFG->dwID) != teamer->getUserSceneData().getShowNpcs().end())
          return true;
      }
    }
    return false;
  }
  TrapNpc* pTrapNpc = dynamic_cast<TrapNpc*>(this);
  if (pTrapNpc != nullptr)
    return pTrapNpc->canVisible(user);

  if (define.m_oVar.m_qwItemUserID)
  {
    if (user->id == define.m_oVar.m_qwItemUserID)
      return true;
    else
      return false;
  }

  if (define.m_oVar.m_dwBuffID)
  {
    if (user->m_oBuff.haveBuff(define.m_oVar.m_dwBuffID) == false)
      return false;
  }

  if (m_pCurCFG->bNormalDisplay)
  {
    if (user->getUserSceneData().haveHideNpcs(m_pCurCFG->dwID))
      return false;
  }
  if (m_pCurCFG->bNormalHide)
  {
    if (user->getUserSceneData().haveSeeNpcs(m_pCurCFG->dwID) == false)
      return false;
  }
  if (m_pCurCFG->eNpcType == ENPCTYPE_WEAPONPET)
  {
    if (user->isMyTeamMember(define.m_oVar.m_qwOwnerID) == false)
      return false;
  }
  if (m_pCurCFG->eNpcType == ENPCTYPE_PETNPC)
  {
    // gvg 宠物仅自己可见
    if (getScene() && getScene()->isGvg())
    {
      if (user->id != define.m_oVar.m_qwOwnerID)
        return false;
    }
  }

  return true;
}

void SceneNpc::checkNightWork(DWORD cur)
{
  if (m_ai.isNightWork() == false)
    return;

  const TVecDWORD& vecbuffs = FeatureConfig::getMe().getNpcAICFG().stNightCFG.vecBuffs;
  if (m_ai.isNight(cur))
  {
    if (!m_ai.isInNightWork())
    {
      for (auto &v : vecbuffs)
      {
        m_oBuff.add(v, this);
      }
      m_ai.setNightWork(true);
    }
  }
  else
  {
    if (m_ai.isInNightWork())
    {
      for (auto &v : vecbuffs)
      {
        m_oBuff.del(v);
      }
      m_ai.setNightWork(false);
    }
  }
}

bool SceneNpc::evo(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (pCFG == nullptr || m_pCurCFG == nullptr)
    return false;
  m_dwEvoID = pCFG->dwID;
  m_oEvoDef = def;
  m_bEvo = true;
  /*
  m_pCurCFG = pCFG;
  m_bEvo = true;
  define = def;
  define.setLife(m_oOriDefine.getLife());
  define.setReborn(m_oOriDefine.getReborn());
  define.setPos(getPos());
  define.setRange(0);
  */
  if (isAlive())
  {
    setStatus(ECREATURESTATUS_EVO);
  }
  else if (getStatus() != ECREATURESTATUS_REMOVE)
    setStatus(ECREATURESTATUS_REMOVE);
  XLOG << "[魔物进化-进化]" << id << m_pCurCFG->dwID << name << "进化为" << pCFG->dwID << pCFG->strName << "成功" << XEND;
  return true;
}

void SceneNpc::onGuidChange(QWORD oldguid)
{
  if (define.m_oVar.m_qwNpcOwnerID)
  {
    SceneNpc* pMaster = SceneNpcManager::getMe().getNpcByTempID(define.m_oVar.m_qwNpcOwnerID);
    if (pMaster && pMaster->getScene() && pMaster->isAlive())
    {
      pMaster->m_oFollower.onServantIDChange(oldguid, id);
    }
  }
  Scene* pScene = SceneManager::getMe().getSceneByID(m_dwSceneID);
  if (pScene)
    pScene->onNpcGuidChange(oldguid, id);
  ActivityEventManager::getMe().onNpcGuidChange(oldguid, id);
}

float SceneNpc::calcDropRatio(SceneUser* pUser)
{
  float fRatio = 1.0f;
  if (pUser == nullptr || m_pCurCFG == nullptr)
    return fRatio;

  SDWORD swDeltalv = (SDWORD)getLevel() - (SDWORD)(pUser->getUserSceneData().getRolelv());
  fRatio = LuaManager::getMe().call<float>("calcAddictDropItem", pUser->getAddictTime(), fRatio, swDeltalv);

  return fRatio;
}

/*bool SceneNpc::turnNewMonster(const NpcDefine& def)
{
  DWORD turnID = def.getID();
  const SNpcCFG* pTurnCFG = NpcConfig::getMe().getNpcCFG(turnID);
  if (pTurnCFG == nullptr)
    return false;

  XDBG << "[魔物变身]" << id << m_pCurCFG->dwID << name << "变为" << pTurnCFG->dwID << pTurnCFG->strName << "成功" << XEND;

  // turn to other monster
  if (init(pTurnCFG, def) == false)
    return false;
  define.setPos(getPos());
  define.setRange(0);
  define.setLife(m_oOriDefine.getLife());

  leaveScene();
  setStatus(ECREATURESTATUS_LIVE);

  if (changeGuid() == false)
    return false;
  // enterScene(getScene());
  m_ai.setCurLockID(m_ai.getLastAttacker());

  return true;
}*/

bool SceneNpc::changeGuid()
{
  DWORD qwGUID = GuidManager::getMe().getNextNpcID();
  if (qwGUID != 0)
  {
    DWORD dwOldGUID = tempid;
    SceneNpcManager::getMe().removeObject(this);
    set_id(qwGUID);
    set_tempid(qwGUID);
    if (SceneNpcManager::getMe().addObject(this) == false)
    {
      if (m_pCurCFG != nullptr)
        XERR << "[场景怪物-Guid改变]" << id << m_pCurCFG->dwID << name << "重新加入管理器失败" << XEND;
      return false;
    }
    onGuidChange(dwOldGUID);
    return true;
  }
  return false;
}

void SceneNpc::loadAttr()
{
  setCollectMark(ECOLLECTTYPE_BASE);
  //setCollectMark(ECOLLECTTYPE_SKILL);
  setCollectMark(ECOLLECTTYPE_EQUIP);
  //setCollectMark(ECOLLECTTYPE_CARD);
  setCollectMarkAllBuff();
  setCollectMark(ECOLLECTTYPE_CHARACTER);

  updateAttribute();
  if (define.m_oVar.m_dwDefaultHp != 0 && define.m_oVar.m_dwDefaultHp <= (DWORD)getAttr(EATTRTYPE_MAXHP))
    setAttr(EATTRTYPE_HP, define.m_oVar.m_dwDefaultHp);
  else
    setAttr(EATTRTYPE_HP, getAttr(EATTRTYPE_MAXHP));
  m_dwLastHp = getAttr(EATTRTYPE_HP);
}

void SceneNpc::addMvpLockUserTime(QWORD charid, QWORD msTime)
{
  auto it = m_mapUser2MvpInfo.find(charid);
  if (it != m_mapUser2MvpInfo.end())
  {
    it->second.m_qwBeLockTime += msTime;
    it->second.m_dwLastMarkTime = now();
  }
  else
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(charid);
    if (user == nullptr)
      return;
    SMvpKillInfo& sInfo = m_mapUser2MvpInfo[charid];
    sInfo.charid = charid;
    sInfo.name = user->name;
    sInfo.m_qwBeLockTime = msTime;
    sInfo.m_dwLastMarkTime = now();
  }
}

void SceneNpc::addMvpRelaHealHp(QWORD charid, DWORD hp, bool isHealSelf)
{
  auto it = m_mapUser2MvpInfo.find(charid);
  if (it != m_mapUser2MvpInfo.end())
  {
    it->second.m_dwHealHp += hp;
    if (!isHealSelf)
      it->second.m_dwLastMarkTime = now();
  }
  else
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(charid);
    if (user == nullptr)
      return;
    SMvpKillInfo& sInfo = m_mapUser2MvpInfo[charid];
    sInfo.charid = charid;
    sInfo.name = user->name;
    sInfo.m_dwHealHp = hp;
    if (!isHealSelf)
      sInfo.m_dwLastMarkTime = now();
  }
}

void SceneNpc::addMvpRelaReliveTimes(QWORD charid)
{
  auto it = m_mapUser2MvpInfo.find(charid);
  if (it != m_mapUser2MvpInfo.end())
  {
    it->second.m_dwReliveOtherTimes += 1;
    it->second.m_dwLastMarkTime = now();
  }
  else
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(charid);
    if (user == nullptr)
      return;
    SMvpKillInfo& sInfo = m_mapUser2MvpInfo[charid];
    sInfo.charid = charid;
    sInfo.name = user->name;
    sInfo.m_dwReliveOtherTimes = 1;
    sInfo.m_dwLastMarkTime = now();
  }
}

void SceneNpc::addMvpHitTime(QWORD charid)
{
  auto it = m_mapUser2MvpInfo.find(charid);
  if (it != m_mapUser2MvpInfo.end())
  {
    it->second.m_qwHitTime = xTime::getCurMSec();
    it->second.m_dwLastMarkTime = it->second.m_qwHitTime / ONE_THOUSAND;
    if (it->second.m_qwFirstHitTime == 0)
      it->second.m_qwFirstHitTime = it->second.m_qwHitTime;
  }
  else
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(charid);
    if (user == nullptr)
      return;
    SMvpKillInfo& sInfo = m_mapUser2MvpInfo[charid];
    sInfo.charid = charid;
    sInfo.name = user->name;
    sInfo.m_qwHitTime = xTime::getCurMSec();
    sInfo.m_qwFirstHitTime = sInfo.m_qwHitTime;
    sInfo.m_dwLastMarkTime = sInfo.m_qwHitTime / ONE_THOUSAND;
  }
}

void SceneNpc::refreshMvpInfo(DWORD cur)
{
  DWORD configValidTime = MiscConfig::getMe().getMvpScoreCFG().dwValidTime;
  for (auto m = m_mapUser2MvpInfo.begin(); m != m_mapUser2MvpInfo.end(); )
  {
    if (m->second.m_dwLastMarkTime + configValidTime <= cur)
    {
      XLOG << "[MVP-击杀更新], 玩家:" << m->second.name << m->second.charid << "超时, 从列表移除" << XEND;
      m = m_mapUser2MvpInfo.erase(m);
      continue;
    }
    ++m;
  }
}

void SceneNpc::sendMvpNotice()
{
  auto mvp = m_mapUser2MvpInfo.find(m_qwMvpUserID);
  if (mvp == m_mapUser2MvpInfo.end())
  {
    XERR << "[mvp-notice], 异常" << name << id << "mvp user:" << m_qwMvpUserID << "当前血量:" << getAttr(EATTRTYPE_HP) << XEND;
    return;
  }

  Scene* pScene = getScene() ? getScene() : SceneManager::getMe().getSceneByID(m_dwSceneID);
  if (pScene == nullptr)
  {
    XERR << "[mvp-notice], 找不到地图" << name << id << "mvp user:" << m_qwMvpUserID << XEND;
    return;
  }

  // 公会副本不广播提示
  if (pScene->getSceneType() == SCENE_TYPE_GUILD_RAID || pScene->isSuperGvg())
    return;

  /*std::sort(mvp->second.vecRankInfo.begin(), mvp->second.vecRankInfo.end(), [&](const TPairType2RankScore& r1, const TPairType2RankScore& r2) -> bool{
      return (r1.second.first < r2.second.first) || (r1.second.first == r2.second.first && r1.second.second > r2.second.second);
      });
      */
  const SMvpScoreCFG& rCFG = MiscConfig::getMe().getMvpScoreCFG();
  TVecString showmsg;

  // send self notice
  for (auto &m : m_mapUser2MvpInfo)
  {
    if (m.second.vecRankInfo.empty())
      continue;

    showmsg.clear();
    std::sort(m.second.vecRankInfo.begin(), m.second.vecRankInfo.end(), [&](const TPairType2RankScore& r1, const TPairType2RankScore& r2) ->bool {
        return rCFG.getOrderByType(r1.first) < rCFG.getOrderByType(r2.first);
        });
    for (auto &v : m.second.vecRankInfo)
    {
      bool highest = v.second.first == 1;
      MiscConfig::getMe().formatMvpInfo(v.first, showmsg, m.second, highest);
    }
    /*for (int i = EMVPSCORETYPE_MIN + 1; i < EMVPSCORETYPE_MAX; ++i)
    {
      auto v = find_if(m.second.vecRankInfo.begin(), m.second.vecRankInfo.end(), [&](const TPairType2RankScore& r1) -> bool {
        return r1.first == i;
      });
      bool nodata = (v == m.second.vecRankInfo.end());
      bool highest = (v != m.second.vecRankInfo.end() && v->second.first == 1);
      MiscConfig::getMe().formatMvpInfo((EMvpScoreType)i, showmsg, m.second, highest, nodata);
    }
    */
    MsgManager::sendMsg(m.second.charid, 4001, MsgParams(showmsg));
  }

  MsgParams params;
  params.addString(mvp->second.name);
  params.addString(name);
  MsgManager::sendMapMsg(pScene->id, 4000, params);

  /*
  showmsg.clear();
  // send mvp notice
  for (DWORD i = 0; i < 3 && i < mvp->second.vecRankInfo.size(); ++i)
  {
    const TPairType2RankScore& oPair = mvp->second.vecRankInfo[i];
    MiscConfig::getMe().formatMvpInfo(oPair.first, showmsg, mvp->second, false);
  }
  params.addString(showmsg);
  MsgManager::sendMapMsg(pScene->id, 4000, params);
  */
}

void SceneNpc::testPrintMvpInfo(SceneUser* user)
{
  if (user == nullptr)
    return;
  auto it = m_mapUser2MvpInfo.find(user->id);
  if (it == m_mapUser2MvpInfo.end())
    return;
  const SMvpScoreCFG& rCFG = MiscConfig::getMe().getMvpScoreCFG();
  std::stringstream stream;
  stream.str("");
  stream << rCFG.getNameByType(EMVPSCORETYPE_DAMAGE) << ": " << it->second.m_dwHitDamage << "\n";
  stream << rCFG.getNameByType(EMVPSCORETYPE_BELOCK) << ": " << (float)it->second.m_qwBeLockTime / 1000.0f << "\n";
  stream << rCFG.getNameByType(EMVPSCORETYPE_HEAL) << ": " << it->second.m_dwHealHp << "\n";
  stream << rCFG.getNameByType(EMVPSCORETYPE_RELIVE) << ": " << it->second.m_dwReliveOtherTimes << "\n";

  // TVecString param;
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_DAMAGE, param, it->second, true);
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_HEAL, param, it->second, true);
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_RELIVE, param, it->second, true);
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_BELOCK, param, it->second, true);
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_DEADHIT, param, it->second, true);
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_FIRSTDAM, param, it->second, true);
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_TOPDAMAGE, param, it->second, true);
  // MiscConfig::getMe().formatMvpInfo(EMVPSCORETYPE_BREAKSKILL, param, it->second, true);
  // MsgManager::sendMsg(user->id, 4001, MsgParams(param));

  string str = stream.str();
  MsgManager::sendMsg(user->id, 4001, MsgParams(str));
}


void SceneNpc::calcMvpUser()
{
  // earse users not in this map
  DWORD totalDamage = 0;
  bool checkChangeZone = getScene() && getScene()->isDScene() == false;
  for (auto m = m_mapUser2MvpInfo.begin(); m != m_mapUser2MvpInfo.end(); )
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(m->first);
    if (user == nullptr || user->getScene() != getScene() || (user->isJustInViceZone() && checkChangeZone))
    {
      m = m_mapUser2MvpInfo.erase(m);
      continue;
    }
    totalDamage += m->second.m_dwHitDamage;

    ++m;
  }

  vector<SMvpKillInfo> rankVec;
  for (auto &m : m_mapUser2MvpInfo)
  {
    rankVec.push_back(m.second);
  }
  if (rankVec.empty())
    return;

  auto compareHeal = [&](const SMvpKillInfo& sInfo1, const SMvpKillInfo& sInfo2) -> bool
  {
    return sInfo1.m_dwHealHp > sInfo2.m_dwHealHp;
  };
  auto compareRelive = [&](const SMvpKillInfo& sInfo1, const SMvpKillInfo& sInfo2) -> bool
  {
    return sInfo1.m_dwReliveOtherTimes > sInfo2.m_dwReliveOtherTimes;
  };
  auto compareDamage = [&](const SMvpKillInfo& sInfo1, const SMvpKillInfo& sInfo2) -> bool
  {
    return sInfo1.m_dwHitDamage > sInfo2.m_dwHitDamage;
  };
  auto compareHitTime = [&](const SMvpKillInfo& sInfo1, const SMvpKillInfo& sInfo2) -> bool
  {
    return sInfo1.m_qwHitTime > sInfo2.m_qwHitTime;
  };
  auto compareLockTime = [&](const SMvpKillInfo& sInfo1, const SMvpKillInfo& sInfo2) -> bool
  {
    return sInfo1.m_qwBeLockTime > sInfo2.m_qwBeLockTime;
  };

  TPairType2RankScore oPair;

  const SMvpScoreCFG& rCFG = MiscConfig::getMe().getMvpScoreCFG();
  SceneUser* pUser = nullptr;

  // 治愈得分
  DWORD index = 0;
  std::sort(rankVec.begin(), rankVec.end(), compareHeal);
  oPair.first = EMVPSCORETYPE_HEAL;
  for (auto &v : rankVec)
  {
    if (v.m_dwHealHp == 0 || index >= rCFG.vecHealScore.size())
      break;

    DWORD score = rCFG.vecHealScore[index];

    oPair.second.first = index + 1;
    oPair.second.second = score;
    v.vecRankInfo.push_back(oPair);

    v.m_dwScore += score;
    index ++;
    XLOG << "MVP 治愈得分:" << v.name << score << "治愈量:" << v.m_dwHealHp << XEND;
  }
  pUser = SceneUserManager::getMe().getUserByID(rankVec.begin()->charid);
  if (pUser != nullptr)
    pUser->getAchieve().onMvp(EMVPSCORETYPE_HEAL, getNpcID());

  // 复活其他玩家得分
  index = 0;
  std::sort(rankVec.begin(), rankVec.end(), compareRelive);
  oPair.first = EMVPSCORETYPE_RELIVE;
  for (auto &v : rankVec)
  {
    if (v.m_dwReliveOtherTimes == 0 || index >= rCFG.vecRebirthScore.size())
      break;
    DWORD score = rCFG.vecRebirthScore[index];
    oPair.second.first = index + 1;
    oPair.second.second = score;
    v.vecRankInfo.push_back(oPair);

    v.m_dwScore += score;
    index ++;
    XLOG << "MVP 复活得分:" << v.name << score << "复活人数:" << v.m_dwReliveOtherTimes << XEND;
  }

  // 被锁定时长得分
  index = 0;
  std::sort(rankVec.begin(), rankVec.end(), compareLockTime);
  oPair.first = EMVPSCORETYPE_BELOCK;
  for (auto &v : rankVec)
  {
    if (v.m_qwBeLockTime == 0 || index >= rCFG.vecBeLockScore.size())
      break;
    DWORD score = rCFG.vecBeLockScore[index];
    oPair.second.first = index + 1;
    oPair.second.second = score;
    v.vecRankInfo.push_back(oPair);

    v.m_dwScore += score;
    index ++;
    XLOG << "MVP 锁定得分:" << v.name << score << "锁定时长:" << (float)v.m_qwBeLockTime / 1000.0f << XEND;
  }
  pUser = SceneUserManager::getMe().getUserByID(rankVec.begin()->charid);
  if (pUser != nullptr)
    pUser->getAchieve().onMvp(EMVPSCORETYPE_BELOCK, getNpcID());

  // 致命一击（最后1s 攻击) 得分
  QWORD cur = xTime::getCurMSec();
  index = 0;
  std::sort(rankVec.begin(), rankVec.end(), compareHitTime);
  oPair.first = EMVPSCORETYPE_DEADHIT;
  for (auto &v : rankVec)
  {
    if (v.m_qwHitTime + rCFG.dwDeadHitTime < cur || index >= rCFG.vecDeadHitScore.size())
      break;
    DWORD score = rCFG.vecDeadHitScore[index];
    oPair.second.first = index + 1;
    oPair.second.second = score;
    v.vecRankInfo.push_back(oPair);

    v.m_dwScore += score;
    index ++;
    XLOG << "MVP 致命一击得分:" << v.name << score << XEND;
  }
  pUser = SceneUserManager::getMe().getUserByID(rankVec.begin()->charid);
  if (pUser != nullptr)
    pUser->getAchieve().onMvp(EMVPSCORETYPE_DEADHIT, getNpcID());

  // 最先攻击得分
  QWORD tmptime = QWORD_MAX;
  QWORD firstGuid = 0;
  for (auto &v : rankVec)
  {
    if (v.m_qwFirstHitTime == 0)
      continue;
    if (v.m_qwFirstHitTime < tmptime)
    {
      tmptime = v.m_qwFirstHitTime;
      firstGuid = v.charid;
    }
  }
  auto firstone = find_if(rankVec.begin(), rankVec.end(), [&](const SMvpKillInfo& r)->bool{
      return r.charid == firstGuid;
      });
  if (firstone != rankVec.end())
  {
    firstone->m_dwScore += rCFG.dwFirstHitScore;
    oPair.first = EMVPSCORETYPE_FIRSTDAM;
    oPair.second.first = 1;
    oPair.second.second = rCFG.dwFirstHitScore;
    firstone->vecRankInfo.push_back(oPair);
    XLOG << "MVP 最先攻击得分:" << firstone->name << rCFG.dwFirstHitScore << XEND;

    pUser = SceneUserManager::getMe().getUserByID(firstone->charid);
    if (pUser != nullptr)
      pUser->getAchieve().onMvp(EMVPSCORETYPE_FIRSTDAM, getNpcID());
  }

  // 击杀伤害得分
  index = 0;
  std::sort(rankVec.begin(), rankVec.end(), compareDamage);
  oPair.first = EMVPSCORETYPE_DAMAGE;
  totalDamage = totalDamage != 0 ? totalDamage : getAttr(EATTRTYPE_MAXHP);
  if (totalDamage == 0)
    return;
  for (auto &v : rankVec)
  {
    if (v.m_dwHitDamage == 0 || index >= rCFG.vecDamScore.size())
      break;
    DWORD score = rCFG.vecDamScore[index];
    DWORD decScore = rCFG.getDamDecScore((float)v.m_dwHitDamage / (float)totalDamage * 100);
    score = (score > decScore ? score - decScore : 1);
    oPair.second.first = index + 1;
    oPair.second.second = score;
    v.vecRankInfo.push_back(oPair);

    v.m_dwScore += score;
    index ++;

    if (index == 1)
    {
      TPairType2RankScore topdamage;
      topdamage.first = EMVPSCORETYPE_TOPDAMAGE;
      topdamage.second.first = 1;
      v.vecRankInfo.push_back(topdamage);
    }

    XLOG << "MVP 伤害得分:" << v.name << score << "伤害量:" << v.m_dwHitDamage << "伤害百分比扣分:" << decScore << XEND;
  }
  for (auto &v : rankVec)
  {
    pUser = SceneUserManager::getMe().getUserByID(v.charid);
    if (pUser != nullptr)
      pUser->getAchieve().onMvp(EMVPSCORETYPE_DAMAGE, getNpcID());
  }
  pUser = SceneUserManager::getMe().getUserByID(rankVec.begin()->charid);
  if (pUser != nullptr)
    pUser->getAchieve().onMvp(EMVPSCORETYPE_TOPDAMAGE, getNpcID());

  for (auto &v : rankVec)
  {
    auto it = m_mapUser2MvpInfo.find(v.charid);
    if (it == m_mapUser2MvpInfo.end())
      continue;
    XLOG << "MVP 总得分:" << v.name << v.m_dwScore << XEND;
    it->second.m_dwScore = v.m_dwScore;
    it->second.vecRankInfo.assign(v.vecRankInfo.begin(), v.vecRankInfo.end());
  }

  /*
  auto compareScore = [&](const SMvpKillInfo& sInfo1, const SMvpKillInfo& sInfo2) -> bool
  {
    return sInfo1.m_dwScore < sInfo2.m_dwScore;
  };
  auto mvp = std::max_element(rankVec.begin(), rankVec.end(), compareScore);
  */

  std::random_shuffle(rankVec.begin(), rankVec.end()); // 打乱当前排名, 分数相同时, 随机取一个作为mvp
  auto mvp = rankVec.begin();
  DWORD tmpScore = 0;
  for (auto v = rankVec.begin(); v != rankVec.end(); ++v)
  {
    if (v->m_dwScore > tmpScore)
    {
      tmpScore = v->m_dwScore;
      mvp = v;
    }
  }
  m_qwMvpUserID = mvp->charid;
  pUser = SceneUserManager::getMe().getUserByID(m_qwMvpUserID);
  if (pUser != nullptr)
    pUser->getAchieve().onMvp(EMVPSCORETYPE_MAX, getNpcID());

  XLOG << "[MVP-获得], 怪物" << name << id << "玩家:" << mvp->charid << mvp->name << "各项得分:";
  for (auto &v : mvp->vecRankInfo)
    XLOG << "类型:" << rCFG.getNameByType(v.first) << "排名:" << v.second.first << "得分:" << v.second.second << "||";
  XLOG << "总得分:" << mvp->m_dwScore << XEND;

  bool deadhit = find_if(mvp->vecRankInfo.begin(), mvp->vecRankInfo.end(), [&](const TPairType2RankScore& v) -> bool{
      return v.first == EMVPSCORETYPE_DEADHIT;
      }) != mvp->vecRankInfo.end();

  XLOG << "[MVP-获得], mvp各项数据:" << "治愈量" << mvp->m_dwHealHp << "对boss造成伤害" << mvp->m_dwHitDamage << "被锁定时间(ms):" << mvp->m_qwBeLockTime << "复活他人次数:" << mvp->m_dwReliveOtherTimes << "第一击:" << (firstGuid == mvp->charid) << "致命一击:" << deadhit << XEND;
}

void SceneNpc::useSuperSkill(DWORD skillid, QWORD targetid, xPos pos, bool specpos)
{
  if (m_oMove.checkActionTime() == false)
  {
    m_oSkillProcessor.addQueueNpcSkill(skillid, targetid, pos, specpos);
    return;
  }
  if (m_oSkillProcessor.getRunner().getState() == ESKILLSTATE_CHANT)
  {
    m_oSkillProcessor.addQueueNpcSkill(skillid, targetid, pos, specpos);
    return;
  }
  useSkill(skillid, targetid, pos, specpos);
}

DWORD SceneNpc::getEquipData(EUserDataType eType)
{
  if (m_oBuff.hasPartTransform(eType))
    return m_oBuff.getPartTransform(eType);

  PetNpc* pPetNpc = dynamic_cast<PetNpc*>(this);
  if (pPetNpc != nullptr)
  {
    SceneUser* pUser = getMasterUser();
    if (pUser == nullptr)
      return getShowByType(eType);

    SScenePetData* pData = pUser->getUserPet().getPetData(getNpcID());
    if (pData == nullptr)
      return getShowByType(eType);

    DWORD equipid = pData->getEquipIDByDataType(eType);
    if (equipid)
      return equipid;

    const SNpcCFG* pBodyNpc = NpcConfig::getMe().getNpcCFG(pData->dwBody);
    return pBodyNpc ? pBodyNpc->getShowByType(eType) : 0;
  }

  if (getNpcType() == ENPCTYPE_BEING)
  {
    SceneUser* pMaster = getMasterUser();
    if (pMaster)
    {
      DWORD body = pMaster->getUserBeing().getBody(getNpcID());
      const SNpcCFG* pBodyNpc = NpcConfig::getMe().getNpcCFG(body);
      return pBodyNpc ? pBodyNpc->getShowByType(eType) : 0;
    }
  }

  return getShowByType(eType);
}

DWORD SceneNpc::getShowByType(EUserDataType eType) const
{
  if (eType == EUSERDATATYPE_BODY && define.getBody() != 0)
    return define.getBody();
  else if (eType == EUSERDATATYPE_HAIR && define.getHair() != 0)
    return define.getHair();
  else if (eType == EUSERDATATYPE_LEFTHAND && define.getLeftHand() != 0)
    return define.getLeftHand();
  else if (eType == EUSERDATATYPE_RIGHTHAND && define.getRightHand() != 0)
    return define.getRightHand();
  else if (eType == EUSERDATATYPE_HEAD && define.getHead() != 0)
    return define.getHead();
  else if (eType == EUSERDATATYPE_BACK && define.getBack() != 0)
    return define.getBack();

  return m_pCurCFG ? m_pCurCFG->getShowByType(eType) : 0;
}

bool SceneNpc::isScreenLimit(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  if (define.m_oVar.m_qwFollowerID)
  {
    if (pUser->isMyTeamMember(define.m_oVar.m_qwFollowerID))
      return false;

    SceneUser* pMaster = SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwFollowerID);
    if (pMaster == nullptr)
      return false;

    if (pMaster->getScene() && !pMaster->getScene()->inScope(pMaster, pUser))
      return true;
  }

  return false;
}

bool SceneNpc::needCheckScreenLimit()
{
  if (define.m_oVar.m_qwFollowerID)
    return true;
  return false;
}

SceneUser* SceneNpc::getScreenUser() const
{
  SceneUser* master = getMasterUser();
  if (master)
    return master;

  if (define.m_oVar.m_qwFollowerID)
    return SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwFollowerID);

  return nullptr;
}

DWORD SceneNpc::getMapRewardRatio(DWORD mapid) const
{
  const SMapCFG* pMapBase = MapConfig::getMe().getMapCFG(mapid);
  if (pMapBase == nullptr)
    return 0;

  const DWORD npcid = this->getNpcID();
  return pMapBase->getMonsterRewardRatio(npcid);
}

DWORD SceneNpc::getMapMonsterRatioBuff(DWORD mapid) const
{
  const SMapCFG* pMapBase = MapConfig::getMe().getMapCFG(mapid);
  if (pMapBase == nullptr)
    return 0;

  const DWORD npcid = this->getNpcID();
  return pMapBase->getMonsterRatioBuff(npcid);
}

// func npc
FuncNpc::FuncNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

FuncNpc::~FuncNpc()
{

}

bool FuncNpc::beAttack(QWORD damage, xSceneEntryDynamic* target)
{
  return true;
}

// trap npc
TrapNpc::TrapNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

TrapNpc::~TrapNpc()
{

}

void TrapNpc::addTrapSeeUser(xSceneEntryDynamic* pEntry)
{
  if (getScene() == nullptr || pEntry == nullptr)
    return;
  if (m_setTrapSeeUsers.find(pEntry->id) != m_setTrapSeeUsers.end())
    return;
  SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
  if (pUser == nullptr)
    return;
  xSceneEntrySet uSet;
  getScene()->getEntryListInNine(SCENE_ENTRY_USER, getPos(), uSet);

  for (auto &s : uSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*>(s);
    if (!user)
      continue;
    if (user->isMyEnemy(pUser))
      continue;

    if (pUser->isMyTeamMember(user->id) == false)
    {
      if (!pUser->getScene()->inScope(pUser, user))
      {
        continue;
      }
    }

    this->sendMeToUser(user);
    m_setTrapSeeUsers.insert(user->id);
  }
}

bool TrapNpc::canVisible(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;
  if (m_bTrapAllViewed)
    return true;
  for (auto &q : m_setTrapSeeUsers)
  {
    if (pUser->id == q)
      return true;
    xSceneEntryDynamic* enaUser = xSceneEntryDynamic::getEntryByID(q);
    if (enaUser == nullptr)
      continue;
    if (pUser->isMyEnemy(enaUser) == false)
      return true;
  }

  return false;
}

bool TrapNpc::isScreenLimit(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  if (isMyEnemy(pUser))
    return false;

  if (pUser->isMyTeamMember(m_qwSkillMasterID))
    return false;

  SceneUser* pMaster = SceneUserManager::getMe().getUserByID(m_qwSkillMasterID);
  if (pMaster == nullptr)
    return false;

  if (pMaster->getScene() && !pMaster->getScene()->inScope(pMaster, pUser))
    return true;

  return false;
}

// monster npc
MonsterNpc::MonsterNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

MonsterNpc::~MonsterNpc()
{

}

bool MonsterNpc::init(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;

  if (m_pCurCFG == nullptr)
    return false;

  m_vecCharac.clear();
  if((m_pCurCFG->eNpcType == ENPCTYPE_MINIBOSS || m_pCurCFG->eNpcType == ENPCTYPE_MVP) && randBetween(1, 100) <= 50)
    NpcConfig::getMe().collectCharacter(m_vecCharac);
  return true;
}

void MonsterNpc::fillMapNpcData(Cmd::MapNpc *data)
{
  if (data == nullptr || m_pCurCFG == nullptr)
    return;

  SceneNpc::fillMapNpcData(data);

  for (auto m = m_vecCharac.begin(); m != m_vecCharac.end(); ++m)
    data->add_character(*m);
}

void MonsterNpc::fetchChangeData(NpcDataSync& cmd)
{
  SceneNpc::fetchChangeData(cmd);
}

DWORD MonsterNpc::calcBaseExp(SceneUser * pUser, DWORD exp)
{
  if (!pUser)
    return 0;
  float v = pUser->getAttr(EATTRTYPE_BASEEXPPER);
  if (v == 0)
    return exp;
  float fExp = exp * v + exp;
  float r = std::ceil(fExp);
  XDBG << "[杀怪-掉落经验] 扭蛋装备 charid" <<pUser->id << "base" << exp << "attr"<< EATTRTYPE_BASEEXPPER  << v << "最终经验" << r << XEND;
  return r;
};

DWORD MonsterNpc::calcJobExp(SceneUser * pUser, DWORD exp)
{
  if (!pUser)
    return 0;
  float v = pUser->getAttr(EATTRTYPE_JOBEXPPER);
  if (v == 0)
    return exp;
  float fExp = exp * v + exp;
  float r = std::ceil(fExp);
  XDBG << "[杀怪-掉落经验] 扭蛋装备 charid" <<pUser->id << "job" << exp << "attr" << EATTRTYPE_JOBEXPPER << v << "最终经验" << r << XEND;
  return r;
};

void MonsterNpc::onNpcDie()
{
  if (/*needPurify() == true || */getScene() == nullptr || m_pCurCFG == nullptr)
    return;

  SceneNpc::onNpcDie();
  // 宕机 临时 判断
  if (getScene() == nullptr)
    return;

  auto errSendMvpSession = [&]()
  {
    if (isSummonBySession() == false)
      return;
    Cmd::BossDieBossSCmd bossCmd;
    bossCmd.set_npcid(getNpcID());
    bossCmd.set_mapid(getScene() == nullptr ? 0 : getScene()->getMapID());
    PROTOBUF(bossCmd, send, len);
    thisServer->sendCmdToSession(send, len);
  };
  // ----- 计算怪物归属 ------
  std::set<SceneUser*> allUsers;
  for (auto &m : m_mapUserDamage)
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(m.first);
    if (user && user->getScene() == getScene())
      allUsers.insert(user);
  }
  if (allUsers.empty())
  {
    XERR << "[怪物-掉落], 未找到击杀者" << name << getNpcID() << id << XEND;
    errSendMvpSession();
    return;
  }
  SceneUser** ppuser = randomStlContainer(allUsers);
  if (!ppuser)
  {
    XERR << "[怪物-掉落], 未随机到击杀者" << name << getNpcID() << id << XEND;
    return;
  }
  SceneUser* pOwner = *ppuser;
  if (pOwner == nullptr)
  {
    XERR << "[怪物-掉落], 未随机到击杀者" << name << getNpcID() << id << XEND;
    return;
  }

  SceneUser* pMvp = nullptr;
  if (getNpcType() == ENPCTYPE_MVP)
  {
    calcMvpUser();
    pMvp = SceneUserManager::getMe().getUserByID(m_qwMvpUserID);
    if (pMvp == nullptr || pMvp->getScene() != getScene())
    {
      XERR << "[MVP-获取失败], 异常" << name << id << m_qwMvpUserID << XEND;
      errSendMvpSession();
      return;
    }
  }
  if (pMvp)
    pOwner = pMvp; // mvp 覆盖

  if (getNpcType() == ENPCTYPE_MINIBOSS || isStarMonster())
  {
    SceneUser* pMini = SceneUserManager::getMe().getUserByID(m_oOwner.first);
    if (pMini)
    {
      pOwner = pMini;
      XLOG << "[怪物-归属] 切换为第一刀玩家, 怪物" << name << id << "玩家:" << pMini->name << pMini->id << XEND;
    }
  }
  // ----- 计算怪物归属 ------

  // 副本事件
  getScene()->onNpcDieReward(this, pOwner);

  // --------------- get quest reward -----------
  TSetQWORD tmpTeamIDs;
  map<QWORD, vector<pair<DWORD, float>>> mapQuestReward;
  TSetDWORD setSelfGatherRwds;
  for (auto &s : allUsers)
  {
    const GTeam& rTeam = s->getTeam();
    if (tmpTeamIDs.find(rTeam.getTeamID()) != tmpTeamIDs.end())
      continue;
    if (rTeam.getTeamID() != 0)
    {
      for (auto &m : rTeam.getTeamMemberList())
      {
        SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
        if (user == nullptr || user->getScene() != getScene() /*|| user->isAlive() == false*/)
          continue;

        TSetDWORD setReward;
        if (user == pOwner)
          user->getQuest().collectQuestReward(define.getID(), setReward, setSelfGatherRwds);
        else
          user->getQuest().collectQuestReward(define.getID(), setReward);

        if (setReward.empty())
          continue;
        vector<pair<DWORD, float>>& vec = mapQuestReward[user->id];
        for (auto &s : setReward)
          vec.push_back(make_pair(s, 1));

        tmpTeamIDs.insert(rTeam.getTeamID());
        XLOG << "[场景怪物-任务Reward]" << name << getNpcID() << id << "地图:" << getScene()->getMapID() << "玩家:" << user->name << user->id << XEND;
        break;
      }
    }
    else
    {
      TSetDWORD setReward;
      s->getQuest().collectQuestReward(define.getID(), setReward, setSelfGatherRwds);
      vector<pair<DWORD, float>>& vec = mapQuestReward[s->id];
      for (auto &s : setReward)
        vec.push_back(make_pair(s, 1));
      XLOG << "[场景怪物-任务Reward]" << name << getNpcID() << id << "地图:" << getScene()->getMapID() << "玩家:" << s->name << s->id << XEND;
    }
  }

  // self gahter go into package directly
  if (setSelfGatherRwds.empty() == false)
  {
    for (auto &s : setSelfGatherRwds)
    {
      XLOG << "[场景怪物-任务SelfGather], 玩家:" << pOwner->name << pOwner->id << "怪物:" << name << id << "RewardID:" << s << XEND;
      pOwner->getPackage().rollReward(s, EPACKMETHOD_AVAILABLE, false, true, false);
    }
  }
  // --------------- get quest reward -----------

  // --------------- get normal reward -----------
  vector<pair<DWORD, float>> vecReward;
  float fRatio = 0.0f;
  fRatio = LuaManager::getMe().call<float>("calcMapRewardRatio", (SceneNpc*)this, pOwner, false, false);
  XLOG << "[怪物-掉落], 怪物" << name << getNpcID() << id << "地图:" << getScene()->getMapID() << "随机到玩家:" << pOwner->name << pOwner->id << "掉率:" << fRatio << XEND;
  // collect hand reward
  if (pOwner->m_oHands.has())
  {
    if (m_ai.isAngerHand() && m_pCurCFG->dwHandReward != 0)
      vecReward.push_back(pair<DWORD, float>(m_pCurCFG->dwHandReward, fRatio));
  }
  // collect self drop reward
  if (m_pCurCFG != nullptr && define.getSuperAiNpc() == false)
  {
    for (auto &v : m_pCurCFG->vecRewardIDs)
      vecReward.push_back(pair<DWORD, float>(v, fRatio));

    // dead reward
    const TSetDWORD& setRewardIDs = define.getDeadRewardIDs();
    for (auto &s : setRewardIDs)
      vecReward.push_back(pair<DWORD, float>(s, fRatio));
  }
  TSetDWORD setExtraRwds;
  if (getScene()->isDScene() == false && RewardConfig::getMe().getNpcExtraRwd(m_pCurCFG, setExtraRwds, define.getSuperAiNpc()))
  {
    for (auto &s : setExtraRwds)
      vecReward.push_back(pair<DWORD, float>(s, fRatio));
  }
  // statics log
  SceneUser* m_pMaster = SceneUserManager::getMe().getUserByID(m_ai.getLastAttacker());
  if (m_pMaster == nullptr)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_ai.getLastAttacker());
    if (npc)
      m_pMaster = npc->getMasterUser();
  }
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MONSTER_ITEM_REWARD,  getNpcID(), 0, pOwner->getLevel(), fRatio);
  if (m_pMaster)
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MONSTER_KILL, getNpcID(), 0, m_pMaster->getLevel(), (DWORD)1);
  // --------------- get normal reward -----------


  // --------------- mvp 专属掉落 mvp reward item -------------
  TVecItemInfo vecItemInfo;
  if (pOwner != nullptr && m_pCurCFG != nullptr && m_pCurCFG->dwMvpReward != 0 && getNpcType() == ENPCTYPE_MVP)
  {
    fRatio = 1.0f;
    fRatio = LuaManager::getMe().call<float>("calcMapRewardRatio", (SceneNpc*)this, pOwner, false, true);

    if (RewardManager::roll(m_pCurCFG->dwMvpReward, nullptr, vecItemInfo, ESOURCE_MONSTERKILL, fRatio) == true)
    {
      for (auto v = vecItemInfo.begin(); v != vecItemInfo.end(); ++v)
        XLOG << "[场景怪物-mvp物品掉落]" << id << getNpcID() << name << "专属掉落id:" << v->id() << "count:" << v->count() << "获取额外掉率" << fRatio << XEND;

      if (vecItemInfo.empty() == false)
      {
        pOwner->getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);
      }
    }
  }
  // --------------- mvp 专属掉落 mvp reward item -------------

  // CommonReward - 摸过, 即可全队拥有归属权
  if (m_pCurCFG && m_pCurCFG->setCommonRewards.empty() == false && define.getSuperAiNpc() == false)
  {
    TVecItemInfo comVecItems;
    auto rwdUser = [&](SceneUser* user, SceneNpc* npc)
    {
      if (user == nullptr || npc == nullptr)
        return;

      comVecItems.clear();
      //float fRatio = LuaManager::getMe().call<float>("calcMapRewardRatio", npc, user, false, false);
      float fRatio = user->getAddictRatio();
      for (auto &d : m_pCurCFG->setCommonRewards)
      {
        TVecItemInfo item;
        if (RewardManager::roll(d, nullptr, item, ESOURCE_MONSTERKILL, fRatio, getScene()->getMapID()) == false)
        {
          XERR << "[场景怪物-通用奖励随机]" << id << getNpcID() << name << "随机reward:" << d << "失败" << XEND;
          continue;
        }
        for (auto &i : item)
        {
          i.set_source_npc(getNpcID());
          comVecItems.push_back(i);
        }
      }
      XLOG << "[怪物-通用奖励获取], 怪物:" << name << id << "玩家:" << user->name << user->id << "道具数量:" << comVecItems.size() << XEND;
      if (comVecItems.empty() == false)
      {
        user->getPackage().addItem(comVecItems, EPACKMETHOD_AVAILABLE, false, true);
      }
    };

    TSetQWORD tmpTeamIDs;
    std::set<SceneUser*> rwdUserSet;
    rwdUserSet.insert(allUsers.begin(), allUsers.end());
    for (auto &s : allUsers)
    {
      const GTeam& rTeam = s->getTeam();
      if (tmpTeamIDs.find(rTeam.getTeamID()) != tmpTeamIDs.end())
        continue;
      if (s->check2PosInNine(this) == false)
        continue;

      rwdUser(s, (SceneNpc*)this);

      if (rTeam.getTeamID() != 0)
      {
        tmpTeamIDs.insert(rTeam.getTeamID());
        for (auto &m : rTeam.getTeamMemberList())
        {
          if (m.second.charid() == s->id)
            continue;
          SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
          if (user == nullptr || user->getScene() != getScene() || user->isAlive() == false)
            continue;
          if (user->check2PosInNine(this) == false)
            continue;
          rwdUser(user, (SceneNpc*)this);
        }
      }
    }
  }
  // CommonReward

  // 根据掉落衰减因子 随机item掉落
  auto rollreward = [&](const vector<pair<DWORD, float>>& vecRwd2Ratio, TVecItemInfo& vecItems)
  {
    vecItems.clear();
    for (auto& v : vecRwd2Ratio)
    {
      TVecItemInfo item;
      if (RewardManager::roll(v.first, nullptr, item, ESOURCE_MONSTERKILL, v.second, getScene()->getMapID()) == false)
      {
        XERR << "[场景怪物-击杀奖励随机]" << id << getNpcID() << name << "随机reward:" << v.first << "失败" << XEND;
        continue;
      }

      for (auto &i : item)
      {
        i.set_source_npc(getNpcID());
        vecItems.push_back(i);
      }
    }
  };
  // 根据掉落衰减因子 随机item掉落


  // 设置归属玩家, 掉落物品到场景
  Cmd::AddMapItem cmd;
  DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
  float fDropRange = MiscConfig::getMe().getSceneItemCFG().getRange(static_cast<DWORD>(vecItemInfo.size()));
  auto dropitem = [&](SceneUser* pUser, const TVecItemInfo& items, bool bQuest)
  {
    if (pUser == nullptr || vecItemInfo.empty())
      return;
    // collect item owner
    TSetQWORD setItemOwner;
    if (pUser != nullptr)
    {
      setItemOwner.insert(pUser->id);
      const GTeam& rTeam = pUser->getTeam();
      if (rTeam.getTeamID())
      {
        for (auto &m : pUser->getTeamRewardUsers())
        {
          setItemOwner.insert(m->id);
        }
      }
    }
    // create scene normal item
    for (size_t i = 0; i < vecItemInfo.size(); ++i)
    {
      xPos dest;
      getScene()->getRandPos(getPos(), fDropRange, dest);
      vecItemInfo[i].set_source(ESOURCE_PICKUP);
      //vecItemInfo[i].set_source_npc(getNpcID());
      SceneItem* pItem = SceneItemManager::getMe().createSceneItem(getScene(), vecItemInfo[i], dest);
      if (pItem != nullptr)
      {
        for (auto& s : setItemOwner)
          pItem->addOwner(s);
        if (bQuest)
          pItem->setViewLimit();
        pItem->fillMapItemData(cmd.add_items(), extraTime * (i + 1));
        XLOG << "[场景怪物-物品掉落]" << id << getNpcID() << name << "掉落了物品id:" << vecItemInfo[i].id() << "count:" << vecItemInfo[i].count()
          << "在场景pos:(" << dest.x << dest.y << dest.z << ")" << getScene()->id << getScene()->name << "上" << XEND;
      }
    }
  };
  // 设置归属玩家, 掉落物品到场景

  // --------------------  普通道具掉落 -----------------
  vecItemInfo.clear();
  // 正常道具随机
  rollreward(vecReward, vecItemInfo);
  // 怪物捡东西
  combinItemInfo(vecItemInfo, m_vecPickItem);
  resetPickItem();
  // 怪物盗窃
  if (!m_vecStealItem2Count.empty())
  {
    for (auto &v : m_vecStealItem2Count)
    {
      ItemInfo oItem;
      oItem.set_id(v.first);
      oItem.set_count(v.second);
      oItem.set_source(ESOURCE_MONSTERAI);
      vecItemInfo.push_back(oItem);
    }
    resetStealItem();
  }
  // 掉落到场景
  dropitem(pOwner, vecItemInfo, false);

  if (cmd.items_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    getScene()->sendCmdToNine(getPos(), send, len);
  }
  // --------------------  普通道具掉落 -----------------

  // --------------------  任务道具掉落 -----------------
  for (auto &m : mapQuestReward)
  {
    cmd.clear_items();
    vecItemInfo.clear();
    SceneUser* user = SceneUserManager::getMe().getUserByID(m.first);
    if (user == nullptr)
      continue;
    rollreward(m.second, vecItemInfo);
    dropitem(user, vecItemInfo, true);
    if (cmd.items_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      if (user->check2PosInNine(this))
        user->sendCmdToMe(send, len);

      // 仅队伍可见
      std::set<SceneUser*> teamers = user->getTeamSceneUser();
      for (auto &s : teamers)
      {
        if (s == user)
          continue;
        if (s->check2PosInNine(this))
          s->sendCmdToMe(send, len);
      }
    }
  }
  // --------------------  任务道具掉落 -----------------

  // add kill count
  float teamCnt = pOwner->getSceneTeamCnt();
  if (teamCnt)
  {
    std::set<SceneUser*> setUsers = pOwner->getTeamSceneUser();
    setUsers.insert(pOwner);
    for (auto &s : setUsers)
    {
      s->addKillCount(getNpcID(), 1.0 / teamCnt);
    }
  }

  // kill and exp
  DWORD dwKillBaseExp = 0;
  DWORD dwKillJobExp = 0;

  auto team = [this](SceneUser* pUser, DWORD dwBaseExp, DWORD dwJobExp, ENpcType npcType, DWORD dwDamage, DWORD dwKiller)
  {
    if (pUser == nullptr || pUser->getTeamID() == 0 || getScene() == nullptr)
      return;

    //Scene* pDScene = dynamic_cast<DScene*>(getScene());

    //const GTeam& rTeam = pUser->getTeam();
    //DWORD dwTeamMemCount = pUser->getTeamMemberCount(getScene()->getMapID(), pDScene == nullptr ? 0 : pDScene->getRaidID());
    std::set<SceneUser*> teamUsers = pUser->getTeamRewardUsers();
    DWORD dwTeamMemCount = teamUsers.size();
    for (auto &pTeamMember : teamUsers)
    {
      if (pUser->id == pTeamMember->id)
        continue;
      // 非mvp怪 队友的经验自己算
      DWORD dwDailyExtra = 0;
      if (npcType != ENPCTYPE_MVP)
      {
        dwDailyExtra = pTeamMember->getQuest().getDailyExtra();
        if (getNpcType() == ENPCTYPE_MINIBOSS)
          dwDailyExtra = 0;
        dwBaseExp = LuaManager::getMe().call<DWORD>("calcNpcDropBaseExp", pTeamMember, dynamic_cast<SceneNpc*>(this), dwDamage, 0, dwDailyExtra, dwTeamMemCount);
        dwJobExp = LuaManager::getMe().call<DWORD>("calcNpcDropJobExp", pTeamMember, dynamic_cast<SceneNpc*>(this), dwDamage, 0, dwDailyExtra, dwTeamMemCount);
      }

      /*
      bool blCount = true;
      if (npcType == ENPCTYPE_MINIBOSS || npcType == ENPCTYPE_MVP)
        blCount = false;
      */

      // calc addict
      DWORD addictBaseExp = dwBaseExp;
      DWORD addictJobExp = dwJobExp;
      if (pTeamMember->checkAddict(npcType))
      {
        addictBaseExp = LuaManager::getMe().call<DWORD>("calcAddictDropExp", pTeamMember, pTeamMember->getAddictTime(), addictBaseExp);
        addictJobExp = LuaManager::getMe().call<DWORD>("calcAddictDropExp", pTeamMember, pTeamMember->getAddictTime(), addictJobExp);
      }

      // add exp
      DWORD rawExp = addictBaseExp/(1 + dwDailyExtra);
      DWORD dailyExp = addictBaseExp - rawExp;
      addictBaseExp = dailyExp + pTeamMember->getDeposit().getExp(EFuncType_EXP, rawExp);
      addictBaseExp = calcBaseExp(pTeamMember, addictBaseExp);
      //job exp
      DWORD rawJobExp = addictJobExp / (1 + dwDailyExtra);
      DWORD dailyJobExp = addictJobExp - rawJobExp;
      addictJobExp = dailyJobExp + pTeamMember->getDeposit().getExp(EFuncType_JOBEXP, rawJobExp);
      addictJobExp = calcJobExp(pTeamMember, addictJobExp);

      pTeamMember->getQuest().reduceDailyExp(addictBaseExp);
      pTeamMember->getQuest().reduceDailyExp(addictJobExp);
      pTeamMember->addBaseExp(addictBaseExp, ESOURCE_MONSTERKILL);
      pTeamMember->getUserPet().addBaseExp(addictBaseExp);
      pTeamMember->getUserBeing().addBaseExp(addictBaseExp);
      pTeamMember->addJobExp(addictJobExp, ESOURCE_MONSTERKILL);
      pTeamMember->m_oBuff.onGetMonsterExp(this, addictBaseExp/(1 + dwDailyExtra), addictJobExp/(1 + dwDailyExtra));

      //怪物真正经验的衰减比
      float ratio = LuaManager::getMe().call<float>("calcNpBaseExpRatio", dynamic_cast<SceneNpc*>(this), addictBaseExp, dwDailyExtra);
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MONSTER_EXP_REWARD, getNpcID(), 0, pTeamMember->getLevel(), ratio);

      // kill event
      if (check2PosInNine(pTeamMember) == true)
      {
        if (m_setEventUser.find(pTeamMember->id) == m_setEventUser.end())
        {
          m_setEventUser.insert(pTeamMember->id);
          pTeamMember->getEvent().onKillNpc(this, dwKiller, true);
        }
      }
    }
  };
  for (auto m = m_mapUserDamage.begin(); m != m_mapUserDamage.end(); ++m)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(m->first);
    if (pUser == nullptr || pUser->getScene() == nullptr || getScene() == nullptr || pUser->getScene()->id != getScene()->id)
      continue;

    DWORD dwKiller = m_pMaster == nullptr ? 0 : static_cast<DWORD>(m_pMaster->id == pUser->id);
    DWORD dwDailyExtra = pUser->getQuest().getDailyExtra();
    if (getNpcType() == ENPCTYPE_MINIBOSS || getNpcType() == ENPCTYPE_MVP)
      dwDailyExtra = 0;

    //DScene* pDScene = dynamic_cast<DScene*>(getScene());
    //DWORD dwTeamMemCount = pUser->getTeamMemberCount(getScene()->getMapID(), pDScene == nullptr ? 0 : pDScene->getRaidID());
    DWORD dwTeamMemCount = pUser->getTeamRewardUsers().size();

    // calc exp
    DWORD dwBaseExp = LuaManager::getMe().call<DWORD>("calcNpcDropBaseExp", pUser, dynamic_cast<SceneNpc*>(this), m->second, dwKiller, dwDailyExtra, dwTeamMemCount);
    DWORD dwJobExp = LuaManager::getMe().call<DWORD>("calcNpcDropJobExp", pUser, dynamic_cast<SceneNpc*>(this), m->second, dwKiller, dwDailyExtra, dwTeamMemCount);

    if (pOwner == pUser) {
      dwKillBaseExp = dwBaseExp;
      dwKillJobExp = dwJobExp;
    }
    // calc addict
    DWORD addictBaseExp = dwBaseExp;
    DWORD addictJobExp = dwJobExp;
    if (pUser->checkAddict(getNpcType()))
    {
      addictBaseExp = LuaManager::getMe().call<DWORD>("calcAddictDropExp", pUser, pUser->getAddictTime(), addictBaseExp);
      addictJobExp = LuaManager::getMe().call<DWORD>("calcAddictDropExp", pUser, pUser->getAddictTime(), addictJobExp);
    }

    // add exp
    DWORD rawExp = addictBaseExp / (1 + dwDailyExtra);
    DWORD dailyExp = addictBaseExp - rawExp;
    addictBaseExp = dailyExp + pUser->getDeposit().getExp(EFuncType_EXP, rawExp);
    addictBaseExp = calcBaseExp(pUser, addictBaseExp);
    pUser->getQuest().reduceDailyExp(addictBaseExp);
    pUser->addBaseExp(addictBaseExp, ESOURCE_MONSTERKILL);
    pUser->getUserPet().addBaseExp(addictBaseExp);
    pUser->getUserBeing().addBaseExp(addictBaseExp);

    //job exp
    DWORD rawJobExp = addictJobExp / (1 + dwDailyExtra);
    DWORD dailyJobExp = addictJobExp - rawJobExp;
    addictJobExp = dailyJobExp + pUser->getDeposit().getExp(EFuncType_JOBEXP, rawJobExp);
    addictJobExp = calcJobExp(pUser, addictJobExp);
    pUser->getQuest().reduceDailyExp(addictJobExp);
    pUser->addJobExp(addictJobExp, ESOURCE_MONSTERKILL);
    pUser->m_oBuff.onGetMonsterExp(this, addictBaseExp/(1 + dwDailyExtra), addictJobExp/(1 + dwDailyExtra));

    //怪物真正经验的衰减比
    float ratio = LuaManager::getMe().call<float>("calcNpBaseExpRatio", dynamic_cast<SceneNpc*>(this), addictBaseExp, dwDailyExtra);
    StatisticsDefine::sendStatLog(thisServer,ESTATTYPE_MONSTER_EXP_REWARD,define.getID(), 0, pUser->getLevel(), ratio);

    // process team
    DWORD teamKiller = dwKiller;  //队员击杀也算自己击杀的
    if (teamKiller == 0)
    {
      if (m_pMaster)
        teamKiller = m_pMaster->isMyTeamMember(pUser->id);
    }

    team(pUser, dwBaseExp, dwJobExp, getNpcType(), m->second, teamKiller);
    // log
    XLOG << "[场景怪物-玩家击杀伤害]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "对" << id << getNpcID() << name << "造成了" << m->second << "伤害" <<"teamkiller"<< teamKiller << XEND;
    XLOG << "[场景怪物-经验掉落]" << id << getNpcID() << name << "给与玩家" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "base:" << dwBaseExp << "job:" << dwJobExp << "addict base:" << addictBaseExp << "addict job:" << addictJobExp << "经验" << XEND;
    // kill event
    if (m_setEventUser.find(pUser->id) == m_setEventUser.end())
    {
      m_setEventUser.insert(pUser->id);
      pUser->getEvent().onKillNpc(this, teamKiller, true);
    }
  }
  QWORD qwUserID = 0;
  QWORD qwAccID = 0;
  DWORD dwProfession = EPROFESSION_MIN;
  string strUserName;
  if (pOwner != nullptr)
  {
    qwAccID = pOwner->accid;
    qwUserID = pOwner->id;
    dwProfession = pOwner->getProfession();
    strUserName = pOwner->name;
    m_strKillerName = pOwner->name;

    if (m_qwActivityUid)
    {
      ActivityBase* pActivity = ActivityManager::getMe().getActivityByUid(m_qwActivityUid);
      if (pActivity)
      {
        pActivity->setKillerName(getNpcID(), m_strKillerName);
      }
    }
  }

  // send boss kill to session and send expression
  if (isSummonBySession())
  {
    Cmd::BossDieBossSCmd bossCmd;
    bossCmd.set_npcid(getNpcID());
    bossCmd.set_killer(pOwner->name);
    bossCmd.set_killid(pOwner->id);
    bossCmd.set_mapid(getScene() == nullptr ? 0 : getScene()->getMapID());
    PROTOBUF(bossCmd, send, len);
    thisServer->sendCmdToSession(send, len);

    if (isBoss())
    {
      Cmd::KillBossUserCmd cmd;
      cmd.set_userid(pOwner->id);
      PROTOBUF(cmd, send1, len1);
      pOwner->sendCmdToNine(send1, len1);
      pOwner->playDynamicExpression(EAVATAREXPRESSION_SMILE);
    }
  }

  //厨身的鹰眼掉落
  if (m_pMaster)
  {
    m_pMaster->getBuff().onKillMonster(this, EBUFFTYPE_DROPFOODREWARD);
     
    const GTeam& rTeam = m_pMaster->getTeam();
    for (auto &m : rTeam.getTeamMemberList())
    {
      const TeamMemberInfo& rMember = m.second;
      if (m_pMaster->id == rMember.charid())
        continue;
      SceneUser* pTeamMember = SceneUserManager::getMe().getUserByID(rMember.charid());
      if (pTeamMember == nullptr || pTeamMember->getScene() == nullptr || m_pMaster->getScene() == nullptr || m_pMaster->getScene()->id != pTeamMember->getScene()->id /*|| pTeamMember->isAlive() == false*/)
        continue;
      pTeamMember->getBuff().onKillMonster(this, EBUFFTYPE_DROPFOODREWARD);
    }
  }

  if (m_pMaster && (getNpcType() == ENPCTYPE_MINIBOSS || getNpcType() == ENPCTYPE_MVP )&& !isStarMonster())
  {
    m_pMaster->stopSendInactiveLog();
  }


  if (pOwner)
  {
    TVecItemInfo items;
    define.getRandomReward(items);
    if (items.empty() == false)
    {
      XLOG << "[场景怪物-活动怪物reward], 玩家:" << pOwner->name << pOwner->id << "怪物:" << name << id << "道具:";
      for (auto& item : items)
        XLOG << item.id() << item.count();
      XLOG << XEND;
      pOwner->getPackage().addItem(items, EPACKMETHOD_AVAILABLE, false, true);
    }
  }

  // platlog --击杀日志
  if (pOwner && m_pCurCFG)
  {
    bool isMvp = false;
    EMONSTER_TYPE type = EMONSTERTYPE_MONSTER;
    if (getNpcType() == ENPCTYPE_MVP) {
      if (isBoss())
      {
        isMvp = getNpcType() == ENPCTYPE_MVP ? true : false;
        type = EMONSTERTYPE_MVP;
        StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MVP_KILL_COUNT, 0, 0, pOwner->getLevel(), (DWORD)1);
        pOwner->getShare().addNormalData(ESHAREDATATYPE_S_MVPCOUNT, 1);
        pOwner->getShare().onKillMvp(getNpcID(), getScene()->getMapID());
      }
      else {
        type = EMONSTERTYPE_MONSTER;
        StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_NORMAL_KILL_COUNT, 0, 0, pOwner->getLevel(), (DWORD)1);
      }
    }
    else if (getNpcType() == ENPCTYPE_MINIBOSS) {
      type = EMONSTERTYPE_MINI;
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MINI_KILL_COUNT, 0, 0, pOwner->getLevel(), (DWORD)1);
      pOwner->getShare().addNormalData(ESHAREDATATYPE_S_MINICOUNT, 1);
      pOwner->getShare().onKillMvp(getNpcID(), getScene()->getMapID());
    }
    else {
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_NORMAL_KILL_COUNT, 0, 0, pOwner->getLevel(), (DWORD)1);
    }

    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Kill;
    PlatLogManager::getMe().eventLog(thisServer,
      pOwner->getUserSceneData().getPlatformId(),
      pOwner->getZoneID(),
      pOwner->accid,
      pOwner->id,
      eid,
      pOwner->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().KillLog(thisServer,
      pOwner->getUserSceneData().getPlatformId(),
      pOwner->getZoneID(),
      pOwner->accid,
      pOwner->id,
      eType,
      eid,
      define.getID(),
      id,
      m_pCurCFG->dwGroupID,
      dwKillBaseExp,
      dwKillJobExp,
      isMvp, type, pOwner->getUserSceneData().getRolelv(), 1);


    pOwner->getShare().addNormalData(ESHAREDATATYPE_S_KILLMONSTER, 1);
    pOwner->getShare().addCalcData(ESHAREDATATYPE_MOST_KILLMONSTER, getNpcID(), 1);
  }

  if (pOwner != nullptr && getNpcType() == ENPCTYPE_MVP)
    pOwner->getAchieve().onKillMonster(getNpcType(), getNpcID());

  SceneUser* pFirstUser = SceneUserManager::getMe().getUserByID(m_oOwner.first); // 第一刀 (需持续攻击x秒)玩家
  XLOG << "[场景怪物-击杀]" << id << getNpcID() << name<<"npctype"<<getNpcType() << "被" << qwAccID << qwUserID << dwProfession << strUserName
    << "在场景" << (getScene() == nullptr ? 0 : getScene()->id) << (getScene() == nullptr ? "" : getScene()->name) << "pos(" << getPos().x << getPos().y << getPos().z << ") 击杀了"
    <<"最后一击玩家"<<(m_pMaster? m_pMaster->getName():"null") <<"第一击玩家"<<(pFirstUser ? pFirstUser->getName():"null")<<"奖励归属者"<<(pOwner ?pOwner->getName():"null")
    << XEND;

}

void MonsterNpc::addPickItem(const ItemInfo& rInfo)
{
  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  if (m_vecPickItem.size() >= rCFG.dwPickupMaxItem)
    return;

  combinItemInfo(m_vecPickItem, TVecItemInfo{rInfo});
  XLOG << "[场景怪物-物品捡取]" << id << getNpcID() << name<< "在场景" << (getScene() == nullptr ? 0 : getScene()->id) << (getScene() == nullptr ? "" : getScene()->name)
    << "pos(" << getPos().x << getPos().y << getPos().z << ") 捡取了物品 id:" << rInfo.id() << "count:" << rInfo.count() << XEND;
}

void MonsterNpc::addStealItem(DWORD item, DWORD count)
{
  m_vecStealItem2Count.clear();
  XLOG << "[怪物-盗窃], item:" << item << "count: " << count << XEND;

  m_vecStealItem2Count.push_back(pair<DWORD, DWORD> (item, count));
}

/*// purify npc
PurifyNpc::PurifyNpc(QWORD id, Scene* pScene, const SNpcCFG* pCFG) : SceneNpc(id, pScene, pCFG)
{

}

PurifyNpc::~PurifyNpc()
{

}

bool PurifyNpc::init(const NpcDefine& def)
{
  return SceneNpc::init(def);
}

void PurifyNpc::fillMapNpcData(Cmd::MapNpc *data)
{
  if (data == nullptr || base == nullptr)
    return;
  SceneNpc::fillMapNpcData(data);
}

void PurifyNpc::fetchChangeData(NpcDataSync& cmd)
{
  if (m_bitset.any() == false)
    return;
  SceneNpc::fetchChangeData(cmd);
}

void PurifyNpc::onNpcDie(xSceneEntryDynamic* pMaster)
{
  SceneNpc::onNpcDie(pMaster);

  // if raid boss, set owner users & don't reward immediately
  if (needPurify() == false)
    return;
  const SPurifyCFG& purifyCFG = MiscConfig::getMe().getSPurifyCFG();
  DWORD range = purifyCFG.dwPurifyRange;
  SceneActBase* pAct = SceneActManager::getMe().createSceneAct(getScene(), getPos(), range, this->id, EACTTYPE_PURIFY);
  if (pAct == nullptr)
    return;
  SceneActPurify* pPur = dynamic_cast<SceneActPurify*> (pAct);
  if (pPur == nullptr)
    return;
  QWORD questUserID = define.m_oVar.m_qwQuestOwnerID;
  if (questUserID != 0)
    pPur->setPrivateUser(questUserID);
  // pAct->enterScene(getScene());
  setRaidBossUser(questUserID);
}

bool PurifyNpc::purifyByUser(QWORD userid)
{
  if (!needPurify())
    return false;
  auto iter = find(m_vecRaidBossUserIDs.begin(), m_vecRaidBossUserIDs.end(), userid);
  if (iter == m_vecRaidBossUserIDs.end())
    return false;
  m_vecRaidBossUserIDs.erase(iter);
  return true;
}

void PurifyNpc::setRaidBossUser(QWORD userid = 0)
{
  if (!getScene())
    return;
  if (userid != 0)
  {
    m_vecRaidBossUserIDs.push_back(userid);
    return;
  }
  xSceneEntrySet uSet;
  getScene()->getAllEntryList(SCENE_ENTRY_USER, uSet);
  for (auto s = uSet.begin(); s != uSet.end(); ++s)
  {
    m_vecRaidBossUserIDs.push_back((*s)->id);
  }
}*/

// tower npc
TowerNpc::TowerNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

TowerNpc::~TowerNpc()
{

}

bool TowerNpc::init(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;

  //if (def.m_oVar.dwSpecial != 0)
  //  setNpcType(static_cast<ENpcType>(def.m_oVar.dwSpecial));

  return true;
}

void TowerNpc::fillMapNpcData(Cmd::MapNpc *data)
{
  if (data == nullptr)
    return;
  SceneNpc::fillMapNpcData(data);
  add_data(data->add_datas(), EUSERDATATYPE_ROLELEVEL, getLevel());
}

void TowerNpc::setNpcType(ENpcType eType)
{
  if (eType <= ENPCTYPE_MIN || eType >= ENPCTYPE_MAX)
    return;
  m_eType = eType;
}

void TowerNpc::onNpcDie()
{
  SceneNpc::onNpcDie();

  auto team = [this] (SceneUser* pUser, DWORD dwKiller)
  {
    if(pUser == nullptr || pUser->getTeamID() == 0 || getScene() == nullptr)
      return;

    const GTeam& rTeam = pUser->getTeam();
    for (auto &m : rTeam.getTeamMemberList())
    {
      const TeamMemberInfo& rMember = m.second;
      if (pUser->id == rMember.charid())
        continue;
      SceneUser* pTeamMember = SceneUserManager::getMe().getUserByID(rMember.charid());
      if (pTeamMember == nullptr || pTeamMember->getScene() == nullptr || pUser->getScene()->id != pTeamMember->getScene()->id /*|| pTeamMember->isAlive() == false*/)
        continue;

      if (check2PosInNine(pTeamMember) == true)
      {
        if (m_setEventUser.find(pTeamMember->id) == m_setEventUser.end())
        {
          m_setEventUser.insert(pTeamMember->id);
          pTeamMember->getEvent().onKillNpc(this, dwKiller, true);
        }
      }
    }
  };

  // 最后击杀者
  SceneUser* m_pMaster = SceneUserManager::getMe().getUserByID(m_ai.getLastAttacker());
  if (m_pMaster == nullptr)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_ai.getLastAttacker());
    if (npc)
      m_pMaster = npc->getMasterUser();
  }

  TSetQWORD tmpTeamIDs;
  map<QWORD, vector<pair<DWORD, float>>> mapQuestReward;
  for (auto m = m_mapUserDamage.begin(); m != m_mapUserDamage.end(); ++m)
  {
    //无限塔怪物也参加任务杀怪计数
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(m->first);
    if (pUser == nullptr || pUser->getScene() == nullptr || getScene() == nullptr || pUser->getScene()->id != getScene()->id)
      continue;

    DWORD dwKiller = m_pMaster == nullptr ? 0 : static_cast<DWORD>(m_pMaster->id == pUser->id);
    DWORD teamKiller = dwKiller;  //队员击杀也算自己击杀的
    if (teamKiller == 0)
    {
      if (m_pMaster)
        teamKiller = m_pMaster->isMyTeamMember(pUser->id);
    }

    team(pUser, teamKiller);
    if (m_setEventUser.find(pUser->id) == m_setEventUser.end())
    {
      m_setEventUser.insert(pUser->id);
      pUser->getEvent().onKillNpc(this, teamKiller, true);
    }

    //拾取任务计数
    const GTeam& rTeam = pUser->getTeam();
    if (tmpTeamIDs.find(rTeam.getTeamID()) != tmpTeamIDs.end())
      continue;
    if (rTeam.getTeamID() != 0)
    {
      for (auto &m : rTeam.getTeamMemberList())
      {
        SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
        if (user == nullptr || user->getScene() != getScene()/* || user->isAlive() == false*/)
          continue;

        TSetDWORD setReward;
        user->getQuest().collectQuestReward(define.getID(), setReward);

        if (setReward.empty())
          continue;
        vector<pair<DWORD, float>>& vec = mapQuestReward[user->id];
        for (auto &s : setReward)
          vec.push_back(make_pair(s, 1));

        tmpTeamIDs.insert(rTeam.getTeamID());
        XLOG << "[场景怪物-任务Reward]" << "无限塔怪物" << name << getNpcID() << id << "地图:" << getScene()->getMapID() << "玩家:" << user->name << user->id << XEND;
        break;
      }
    }
    else
    {
      TSetDWORD setReward;
      pUser->getQuest().collectQuestReward(define.getID(), setReward);
      vector<pair<DWORD, float>>& vec = mapQuestReward[pUser->id];
      for (auto &s : setReward)
        vec.push_back(make_pair(s, 1));
      XLOG << "[场景怪物-任务Reward]" << "无限塔怪物" << name << getNpcID() << id << "地图:" << getScene()->getMapID() << "玩家:" << pUser->name << pUser->id << XEND;
    }
  }

  // 根据掉落衰减因子 随机item掉落
  auto rollreward = [&](const vector<pair<DWORD, float>>& vecRwd2Ratio, TVecItemInfo& vecItems)
  {
    vecItems.clear();
    for (auto& v : vecRwd2Ratio)
    {
      TVecItemInfo item;
      if (RewardManager::roll(v.first, nullptr, item, ESOURCE_MONSTERKILL, v.second, getScene()->getMapID()) == false)
      {
        XERR << "[场景怪物-击杀奖励随机]" << id << getNpcID() << name << "随机reward:" << v.first << "失败" << XEND;
        continue;
      }

      for (auto &i : item)
      {
        i.set_source_npc(getNpcID());
        vecItems.push_back(i);
      }
    }
  };

  Cmd::AddMapItem cmd;
  DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
  auto dropitem = [&](SceneUser* pUser, TVecItemInfo& items, bool bQuest, float fDropRange)
  {
    if (pUser == nullptr || items.empty())
      return;
    // collect item owner
    TSetQWORD setItemOwner;
    if (pUser != nullptr)
    {
      setItemOwner.insert(pUser->id);
      const GTeam& rTeam = pUser->getTeam();
      if (rTeam.getTeamID())
      {
        for (auto &m : rTeam.getTeamMemberList())
        {
          setItemOwner.insert(m.second.charid());
        }
      }
    }
    // create scene normal item
    for (size_t i = 0; i < items.size(); ++i)
    {
      xPos dest;
      getScene()->getRandPos(getPos(), fDropRange, dest);
      items[i].set_source(ESOURCE_PICKUP);
      //vecItemInfo[i].set_source_npc(getNpcID());
      SceneItem* pItem = SceneItemManager::getMe().createSceneItem(getScene(), items[i], dest);
      if (pItem != nullptr)
      {
        for (auto& s : setItemOwner)
          pItem->addOwner(s);
        if (bQuest)
          pItem->setViewLimit();
        pItem->fillMapItemData(cmd.add_items(), extraTime * (i + 1));
        XLOG << "[场景怪物-物品掉落]" << "无限塔怪物" << id << getNpcID() << name << "掉落了物品id:" << items[i].id() << "count:" << items[i].count()
          << "在场景pos:(" << dest.x << dest.y << dest.z << ")" << getScene()->id << getScene()->name << "上" << XEND;
      }
    }
  };

  //  ---------------   任务道具掉落    --------------
  TVecItemInfo vecItemInfo;
  for (auto &m : mapQuestReward)
  {
    cmd.clear_items();
    vecItemInfo.clear();
    SceneUser* user = SceneUserManager::getMe().getUserByID(m.first);
    if (user == nullptr)
      continue;
    rollreward(m.second, vecItemInfo);
    float fRange = MiscConfig::getMe().getSceneItemCFG().getRange(static_cast<DWORD>(vecItemInfo.size()));
    dropitem(user, vecItemInfo, true, fRange);
    if (cmd.items_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      if (user->check2PosInNine(this))
        user->sendCmdToMe(send, len);

      // 仅队伍可见
      std::set<SceneUser*> teamers = user->getTeamSceneUser();
      for (auto &s : teamers)
      {
        if (s == user)
          continue;
        if (s->check2PosInNine(this))
          s->sendCmdToMe(send, len);
      }
    }
  }
}
/*
void TowerNpc::onNpcDie()
{
  SceneNpc::onNpcDie();

  Scene* pScene = getScene();
  SceneUser* pMaster = SceneUserManager::getMe().getUserByID(m_ai.getLastAttacker());
  if (pScene && pMaster)
  {
    TSetDWORD setExtraRwds;
    if (RewardConfig::getMe().getNpcExtraRwd(m_pCurCFG, setExtraRwds))
    {
      vector<pair<DWORD, float>> vecReward;
      //float fRatio = LuaManager::getMe().call<float>("calcTowerRewardRatio", (SceneNpc*)this);
      float fRatio = 1;
      for (auto &s : setExtraRwds)
      {
        vecReward.push_back(pair<DWORD, float>(s, fRatio));
        XLOG << "[无限塔-怪物额外掉落], 怪物:" << name << id << "额外掉落reward ID:" << s << "玩家:" << pMaster->name << pMaster->id << XEND;
      }

      TVecItemInfo vecItemInfo;
      for (auto &v : vecReward)
      {
        TVecItemInfo item;
        if (RewardConfig::getMe().roll(v.first, EPROFESSION_MIN, item, ESOURCE_MONSTERKILL, v.second, pScene->getMapID()) == false)
        {
          XERR << "[场景怪物-击杀奖励随机]" << id << getNpcID() << name << "随机reward:" << v.first << "失败" << XEND;
          continue;
        }
        for (auto &i : item)
        {
          i.set_source_npc(getNpcID());
          vecItemInfo.push_back(i);
        }
      }

      Cmd::AddMapItem cmd;
      DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
      float fDropRange = MiscConfig::getMe().getSceneItemCFG().getRange(static_cast<DWORD>(vecItemInfo.size()));
      // create scene normal item
      for (size_t i = 0; i < vecItemInfo.size(); ++i)
      {
        xPos dest;
        pScene->getRandPos(getPos(), fDropRange, dest);
        vecItemInfo[i].set_source(ESOURCE_PICKUP);
        SceneItem* pItem = SceneItemManager::getMe().createSceneItem(pScene, vecItemInfo[i], dest);
        if (pItem != nullptr)
        {
          const GTeam& rTeam = pMaster->getTeam();
          if (rTeam.getTeamID())
          {
            for (auto &m : rTeam.getTeamMemberList())
              pItem->addOwner(m.second.charid());
          }
          else
          {
            pItem->addOwner(pMaster->id);
          }

          pItem->fillMapItemData(cmd.add_items(), extraTime * (i + 1));
        }
      }

      if (cmd.items_size() > 0)
      {
        PROTOBUF(cmd, send, len);
        pScene->sendCmdToNine(getPos(), send, len);
      }
    }
  }
}
*/

// laboratory npc
LabNpc::LabNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

LabNpc::~LabNpc()
{

}

bool LabNpc::init(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;

  m_dwLaboratoryPoint = def.m_oVar.dwLabPoint;
  m_dwRoundID = def.m_oVar.dwLabRound;

  return true;
}

// seal npc
SealNpc::SealNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def, DWORD mapid) : SceneNpc(id, pCFG, def)
{
  m_dwTempSealMapID = mapid;
}

SealNpc::~SealNpc()
{

}

bool SealNpc::init(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;

  return true;
}

DWORD SealNpc::getLevel() const
{
  DWORD dwMapID = getScene() == nullptr ? 0 : getScene()->getMapID();
  return MiscConfig::getMe().getSealCFG().getSealLv(dwMapID);
}

// music npc
MusicNpc::MusicNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

MusicNpc::~MusicNpc()
{

}

void MusicNpc::fillMapNpcData(Cmd::MapNpc *data)
{
  if (data == nullptr)
    return;

  SceneNpc::fillMapNpcData(data);

  add_data(data->add_datas(), EUSERDATATYPE_MUSIC_CURID, m_dwMusicID);
  add_data(data->add_datas(), EUSERDATATYPE_MUSIC_START, m_dwStartTime);
  add_data(data->add_datas(), EUSERDATATYPE_MUSIC_LOOP, m_bLoop ? 1 : 0);
}

void MusicNpc::setMusicData(DWORD dwID, DWORD dwStartTime, QWORD qwCharID, bool bLoop/* = false*/)
{
  m_dwStartTime = dwStartTime;
  m_dwMusicID = dwID;
  m_qwDemanID = qwCharID;
  m_bLoop = bLoop;

  xSceneEntrySet uSet;
  getScene()->getEntryListInNine(SCENE_ENTRY_USER, getPos(), uSet);
  for (auto s = uSet.begin(); s != uSet.end(); ++s)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(*s);
    if (pUser != nullptr)
      pUser->setMusicData(m_dwMusicID, m_dwStartTime, pUser->id == m_qwDemanID, getPos(), bLoop);
  }
}

// dojo npc
DojoNpc::DojoNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

DojoNpc::~DojoNpc()
{

}

bool DojoNpc::init(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;

  m_dwDojoLevel = def.m_oVar.dwDojoLevel;
  return true;
}

// tree npc
TreeNpc::TreeNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

TreeNpc::~TreeNpc()
{

}

bool TreeNpc::init(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;

  setGearStatus(MiscConfig::getMe().getTreasureCFG().dwShakeActionNpc + 1);

  m_eTreeType = static_cast<ETreeType>(def.m_oVar.dwTreeType);
  m_dwLastRefreshTime = now();
  m_dwMoveNextTime = MiscConfig::getMe().getTreasureCFG().dwDisTime + m_dwLastRefreshTime;
  define.m_oVar.m_dwBuffID = MiscConfig::getMe().getTreasureCFG().dwKnownBuffID;
  return true;
}

void TreeNpc::fillMapNpcData(Cmd::MapNpc *data)
{
  if (data == nullptr)
    return;

  SceneNpc::fillMapNpcData(data);

  add_data(data->add_datas(), EUSERDATATYPE_TREESTATUS, getTreeStatus());
}

void TreeNpc::fetchChangeData(NpcDataSync& cmd)
{
  if (m_bitset.any() == false)
    return;

  EUserDataType eType = EUSERDATATYPE_TREESTATUS;
  if (m_bitset.test(eType) == true)
    add_data(cmd.add_datas(), eType, getTreeStatus());

  SceneNpc::fetchChangeData(cmd);
}

void TreeNpc::onNpcDie()
{
  SceneTreasure& rTreasure = getScene()->getSceneTreasure();
  rTreasure.onTreeDie(this);
}

void TreeNpc::addTreeIndex()
{
  if (getScene() == nullptr)
    return;

  refreshTreeStatus();

  ++m_dwIndex;

  const STreasureCFG* pCFG = TreasureConfig::getMe().getTreasureCFG(getScene()->id);
  if (pCFG == nullptr)
    return;
  const STreasureNpc* pNpc = pCFG->getTree(m_eTreeType);
  if (pNpc == nullptr)
    return;
  const STreasureNpcProcess* pProcess = pNpc->getProcess(m_dwIndex);
  if (pProcess == nullptr)
  {
    setStatus(ECREATURESTATUS_LEAVE);
    refreshDataAtonce();
    return;
  }
}

void TreeNpc::refreshTreeStatus()
{
  if (m_oFollower.isTreeEmpty())
    m_eTreeStatus = ETREESTATUS_NORMAL;
  else
    m_eTreeStatus = ETREESTATUS_MONSTER;

  setDataMark(EUSERDATATYPE_TREESTATUS);
  refreshDataAtonce();
}

ETreeStatus TreeNpc::getTreeStatus()
{
  const STreasureCFG* pCFG = TreasureConfig::getMe().getTreasureCFG(getScene()->id);
  if (pCFG == nullptr)
    return m_eTreeStatus;
  const STreasureNpc* pNpc = pCFG->getTree(m_eTreeType);
  if (pNpc == nullptr)
    return m_eTreeStatus;
  const STreasureNpcProcess* pProcess = pNpc->getProcess(m_dwIndex);
  if (pProcess == nullptr)
    return m_eTreeStatus;

  if (pProcess->dwNpcID != 0)
  {
    if (m_oFollower.isTreeEmpty() == true)
      return ETREESTATUS_NORMAL;
    m_eTreeStatus = ETREESTATUS_MONSTER;
  }
  else if (pProcess->dwRewardID != 0)
  {
    m_eTreeStatus = ETREESTATUS_REWARD;
    return ETREESTATUS_NORMAL;
  }

  return m_eTreeStatus;
}

void TreeNpc::onFiveSecTimeUp(QWORD curMSec)
{
  SceneNpc::onFiveSecTimeUp(curMSec);

  DWORD curSec = curMSec / ONE_THOUSAND;

  if (curSec > m_dwLastRefreshTime + 10)
    m_dwMoveNextTime = curSec + (m_dwMoveNextTime - m_dwLastRefreshTime);

  m_dwLastRefreshTime = curSec;

  if (curSec >= m_dwMoveNextTime && getScene() && m_oFollower.isTreeEmpty())
  {
    getScene()->getSceneTreasure().goOtherPos(this);
    m_dwMoveNextTime = MiscConfig::getMe().getTreasureCFG().dwDisTime + curSec;
  }
}

// 战斗猫
WeaponPetNpc::WeaponPetNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

WeaponPetNpc::~WeaponPetNpc()
{

}

void WeaponPetNpc::onNpcDie()
{
  SceneNpc::onNpcDie();

  SceneUser* pMaster = getMasterUser();
  if (pMaster)
  {
    pMaster->getWeaponPet().onNpcDie(this);
  }
  // to do ..
}

SceneUser* WeaponPetNpc::getMasterUser() const
{
  if (define.m_oVar.m_qwOwnerID == 0)
    return nullptr;

  return SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
}

DWORD WeaponPetNpc::getLevel() const
{
  SceneUser* pUser = getMasterUser();
  return pUser ? pUser->getLevel() : 0;
}

void WeaponPetNpc::updateData(DWORD curTime)
{
  if (m_pCurCFG == nullptr || getScene() == nullptr)
    return;

  if (curTime - m_dwDataTick > 10)
  {
    m_dwDataTick = curTime;

    NpcDataSync cmd;
    fetchChangeData(cmd);

    if (cmd.attrs_size() > 0 || cmd.datas_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      getScene()->sendCmdToNine(getPos(), send, len);

      SceneUser* pUser = getMasterUser();
      if (pUser != nullptr)
      {
        for (int i = 0; i < cmd.attrs_size(); ++i)
        {
          const UserAttr& rAttr = cmd.attrs(i);
          switch (rAttr.type())
          {
            case EATTRTYPE_HP:
              pUser->getWeaponPet().setMark(getWeaponPetID(), EMEMBERDATA_HP);
              break;
            case EATTRTYPE_MAXHP:
              pUser->getWeaponPet().setMark(getWeaponPetID(), EMEMBERDATA_MAXHP);
              break;
            default:
              break;
          }
        }
        pUser->getWeaponPet().updateDataToTeam();
      }

#ifdef _DEBUG
      for (int i = 0; i < cmd.attrs_size(); ++i)
        XDBG << "[WeaponPetNpc-属性同步]" << id << getNpcID() << name << "attr :" << cmd.attrs(i).type() << "value :" << cmd.attrs(i).value() << XEND;
      for (int i = 0; i < cmd.datas_size(); ++i)
        XDBG << "[WeaponPetNpc-数据同步]" << id << getNpcID() << name << "data :" << cmd.datas(i).type() << "value :" << cmd.datas(i).value() << XEND;
#endif
    }
  }
}

bool WeaponPetNpc::isMyTeamMember(QWORD id)
{
  if (this->id == id)
    return true;
  SceneUser* pUser = getMasterUser();
  if (pUser == nullptr)
    return false;
  if (pUser->isMyTeamMember(id))
    return true;
  return false;
}

void WeaponPetNpc::setAction(DWORD actionid)
{
  if (m_dwActionID == actionid)
    return;
  m_dwActionID = actionid;

  if (m_dwActionID != 0)
  {
    SceneUser* pUser = getMasterUser();
    if (pUser)
    {
      pUser->getWeaponPet().leaveHand(this->id);
    }
  }
}

SkillNpc::SkillNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{
}

SkillNpc::~SkillNpc()
{
}

SceneUser* SkillNpc::getMasterUser() const
{
  return SceneUserManager::getMe().getUserByID(m_qwMasterID);
}

void SkillNpc::onNpcDie()
{
  SceneNpc::onNpcDie();

  SceneUser* pMaster = getMasterUser();
  if (pMaster)
  {
    pMaster->m_oSkillProcessor.onSkillNpcDie(this->id);
  }
}

void SkillNpc::sendExtraInfo(SceneUser* user)
{
  SceneNpc::sendExtraInfo(user);

  if (m_dwRelatedSkillID == 0)
    return;
  MarkSkillNpcSkillCmd cmd;
  cmd.set_npcguid(id);
  cmd.set_skillid(m_dwRelatedSkillID);

  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

void SkillNpc::setRelatedSkill(DWORD skillid)
{
  if (m_dwRelatedSkillID == skillid)
    return;
  m_dwRelatedSkillID = skillid;

  Scene* pScene = getScene();
  if (pScene == nullptr)
    return;
  xSceneEntrySet userset;
  pScene->getEntryListInNine(SCENE_ENTRY_USER, getPos(), userset);
  for (auto &s : userset)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user == nullptr)
      continue;
    sendExtraInfo(user);
  }
}

void SkillNpc::onTriggered(SceneUser* user)
{
  if (user == nullptr)
    return;
  if (isVisableToSceneUser(user) == false)
    return;

  SceneUser* pMater = getMasterUser();
  if (pMater)
    pMater->m_oSkillProcessor.onTriggerNpc(this->id, user);
}

bool SkillNpc::isScreenLimit(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  if (pUser->isMyTeamMember(define.m_oVar.m_qwOwnerID))
    return false;

  SceneUser* pMaster = SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
  if (pMaster == nullptr)
    return false;

  if (pMaster->getScene() && !pMaster->getScene()->inScope(pMaster, pUser))
    return true;

  return false;
}

// 宠物捕捉互动npc
CatchPetNpc::CatchPetNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

CatchPetNpc::~CatchPetNpc()
{

}

bool CatchPetNpc::init(const SNpcCFG* pCFG, const NpcDefine& def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  m_dwDisperseTime = now() + rCFG.dwMaxCatchTime;

  return true;
}

bool CatchPetNpc::initMasterNpc(DWORD monsterID)
{
  const SCatchPetCFG* pPetCFG = PetConfig::getMe().getCatchCFGByMonster(monsterID);
  if (pPetCFG == nullptr)
  {
    XERR << "[宠物-捕捉], 捕捉npc初始化失败, 找不到对应配置, npc:" << define.getID() << "masterID:" << monsterID<< XEND;
    return false;
  }

  m_dwMasterNpcID = monsterID;

  addCatchValue(pPetCFG->dwDefaultCatchValue);

  return true;
}

void CatchPetNpc::processCatch(QWORD curMSec)
{
  // 捕捉
  if (m_bCatching == false)
    return;

  const SCatchPetCFG* pPetCFG = PetConfig::getMe().getCatchCFGByMonster(m_dwMasterNpcID);
  if (pPetCFG == nullptr)
    return;

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD cur = curMSec / ONE_THOUSAND;
  if (cur < m_dwCatchTimerTick)
    return;

  SceneUser* user = getMasterUser();
  if (user == nullptr || user->getScene() != getScene())
  {
    m_bCatching = false;
    changeCatchState(ECATCHPETSTATE_MIN, 0);
    return;
  }

  switch(m_eCatchState)
  {
    // 技能播放完成
    case ECATCHPETSTATE_SKILL:
      {
        // 立即刷新变身
        DWORD transformBuff = rCFG.dwTransformEggBuff;
        m_oBuff.add(transformBuff);
        m_oBuff.update(curMSec);
        updateData(cur);

        changeCatchState(ECATCHPETSTATE_TRANSFORM, cur + rCFG.dwTransformTime);
      }
      break;
    // 变身效果表现完成
    case ECATCHPETSTATE_TRANSFORM:
      {
        DWORD odds = m_dwCatchValue; // get odds

        if (odds >= (DWORD)randBetween(1, 100))
        {
          m_bCatchResult = true;
          ItemInfo item;
          item.set_id(PetConfig::getMe().getItemIDByPet(pPetCFG->dwPetID));
          item.set_count(1);
          user->getPackage().addItem(item, EPACKMETHOD_AVAILABLE, false, false, false);

          /*MsgParams params;
          params.addNumber(item.id());
          params.addNumber(1);
          MsgManager::sendMsg(user->id, 11, params);
          */
        }
        else
        {
          m_bCatchResult = false;
          // dec catch value
          m_dwCatchValue = m_dwCatchValue > pPetCFG->dwFailDecCatchValue ? m_dwCatchValue - pPetCFG->dwFailDecCatchValue : 0;
        }

        if(rCFG.cancelEffects(pPetCFG->dwPetID) == true)
        {
          if(m_bCatchResult == true)
          {
            DWORD dwItem = PetConfig::getMe().getItemIDByPet(pPetCFG->dwPetID);
            MsgParams params;
            params.addNumber(dwItem);
            params.addNumber(dwItem);
            params.addNumber(1);
            MsgManager::sendMsg(user->id, 25718, params);
          }
          else
          {
            MsgManager::sendMsg(user->id, 25719);
          }
        }
        else
        {
          CatchResultPetCmd cmd;
          cmd.set_npcguid(this->id);
          cmd.set_success(m_bCatchResult);
          PROTOBUF(cmd, send, len);
          user->sendCmdToMe(send, len);
        }

        user->getAchieve().onPetCapture(m_bCatchResult);
        changeCatchState(ECATCHPETSTATE_CARTOON, cur + rCFG.dwMaxCatchCartoonTime);
        if(m_bCatchResult == true)
          user->getServant().onGrowthFinishEvent(ETRIGGER_PET_CAPTURE);
        XLOG << "[宠物-捕捉], 玩家:" << user->name << user->id << "捕捉" << name << id << "捕捉结果:" << m_bCatchResult << "当前捕获值:" << m_dwCatchValue << XEND;
      }
      break;
    // 拉霸机播放完成
    case ECATCHPETSTATE_CARTOON:
      {
        /*m_bCatching = false;
        // 捕获成功, 马上移除
        if (m_bCatchResult)
        {
          removeAtonce();
          return;
        }
        */
        // 捕获失败, 变身恢复
        /*DWORD transformBuff = rCFG.dwTransformEggBuff;
        m_oBuff.del(transformBuff);
        m_oBuff.update(curMSec);
        updateData(cur);
        */

        UserActionNtf actcmd;
        actcmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
        actcmd.set_charid(this->id);
        if (m_bCatchResult)
          actcmd.set_value(200);
        else
          actcmd.set_value(201);
        PROTOBUF(actcmd, send, len);
        sendCmdToNine(send, len);

        changeCatchState(ECATCHPETSTATE_RESULT, cur + 3);
      /*
        if (m_dwCatchValue == 0)
        {
          // talk
          DWORD* p = randomStlContainer(pPetCFG->setValueFailDialogs);
          if (p)
            sendTalkInfo(*p);

          setClearState();
        }
        else
        {
          // talk
          DWORD *p = randomStlContainer(pPetCFG->setTryFailDialogs);
          if (p)
            sendTalkInfo(*p);
        }

        changeCatchState(ECATCHPETSTATE_MIN, 0);
        */
      }
      break;
    case ECATCHPETSTATE_RESULT:
      {
        m_bCatching = false;
        if (m_bCatchResult)
        {
          MsgParams params;
          params.addNumber(PetConfig::getMe().getItemIDByPet(pPetCFG->dwPetID));
          params.addNumber(1);
          MsgManager::sendMsg(user->id, 11, params);
        }

        if (m_dwCatchValue == 0)
        {
          // talk
          DWORD* p = randomStlContainer(pPetCFG->setValueFailDialogs);
          if (p)
            sendTalkInfo(*p);

          setClearState();
        }
        else
        {
          // talk
          DWORD *p = randomStlContainer(pPetCFG->setTryFailDialogs);
          if (p)
            sendTalkInfo(*p);
        }

        if (m_bCatchResult)
        {
          setDeadRemoveAtonce();
          //setClearState();
        }
        else
        {
          DWORD transformBuff = rCFG.dwTransformEggBuff;
          m_oBuff.del(transformBuff);
          m_oBuff.update(curMSec);
          updateData(cur);
        }
        changeCatchState(ECATCHPETSTATE_MIN, 0);
      }
      break;
    default:
      break;
  }
}

void CatchPetNpc::onOneSecTimeUp(QWORD curMSec)
{
  SceneNpc::onOneSecTimeUp(curMSec);

  DWORD cur = curMSec / ONE_THOUSAND;

  // 捕捉
  if (m_bCatching)
  {
    processCatch(curMSec);
    return;
  }

  const SCatchPetCFG* pPetCFG = PetConfig::getMe().getCatchCFGByMonster(m_dwMasterNpcID);
  if (pPetCFG == nullptr)
    return;

  // 超时
  if (m_dwDisperseTime && cur >= m_dwDisperseTime)
  {
    DWORD *p = randomStlContainer(pPetCFG->setEndDialogs);
    if (p)
      sendTalkInfo(*p);
    setClearState();
    return;
  }

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  SceneUser* user = getMasterUser();
  if (user && user->getScene() == getScene())
  {
    if (getXZDistance(getPos(), user->getPos()) > rCFG.dwMaxCatchDistance)
    {
      MsgManager::sendMsg(user->id, 9009);
      setClearState();
    }
    if (m_dwOfflineDelTime)
    {
      if (m_dwOfflineDelTime >= cur)
      {
        setClearState();
        return;
      }
      else
      {
        m_dwOfflineDelTime = 0;
        // set user pet guid
        if (user->getUserPet().getCatchPetID() == 0)
        {
          user->getUserPet().setCatchPetID(this->id);
        }
        else // avoid catch other
        {
          setClearState();
        }
      }
    }
  }
  else // 掉线处理
  {
    if (m_dwOfflineDelTime == 0)
    {
      m_dwOfflineDelTime = cur + rCFG.dwOfflineKeepCatchTime;
    }
    else if (cur > m_dwOfflineDelTime)
    {
      setClearState();
    }
  }

  // 超过一段时间没有操作, 提示
  if (m_dwLastInteractionTime == 0 && cur >= getBirthTime() + rCFG.dwNoOperationNoticeTime)
  {
    DWORD *p = randomStlContainer(pPetCFG->setIgnoreDialogs);
    if (p)
      sendTalkInfo(*p);

    m_dwLastInteractionTime = cur;
  }
}

SceneUser* CatchPetNpc::getMasterUser() const
{
  if (define.m_oVar.m_qwOwnerID == 0)
    return nullptr;
  return SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
}

void CatchPetNpc::onCatchOther()
{
  setClearState();
}

void CatchPetNpc::onUserLeaveScene()
{
  m_dwOfflineDelTime = now() + MiscConfig::getMe().getPetCFG().dwOfflineKeepCatchTime;
}

void CatchPetNpc::sendExtraInfo(SceneUser* user)
{
  SceneNpc::sendExtraInfo(user);

  if (m_dwCatchValue)
  {
    // send catch value to user
    CatchValuePetCmd cmd;
    cmd.set_npcguid(this->id);
    cmd.set_value(m_dwCatchValue);
    cmd.set_from_npcid(m_dwMasterNpcID);
    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
  }
}

void CatchPetNpc::addCatchValue(DWORD value)
{
  m_dwCatchValue += value;

  // talk
  const SCatchPetCFG* pPetCFG = PetConfig::getMe().getCatchCFGByMonster(m_dwMasterNpcID);
  if (pPetCFG != nullptr)
  {
    DWORD *p = randomStlContainer(pPetCFG->setAddDialogs);
    if (p)
      sendTalkInfo(*p);
  }

  // send catch value to nine
  CatchValuePetCmd cmd;
  cmd.set_npcguid(this->id);
  cmd.set_value(m_dwCatchValue);
  cmd.set_from_npcid(m_dwMasterNpcID);
  PROTOBUF(cmd, send, len);
  sendCmdToNine(send, len);
}

bool CatchPetNpc::catchMe()
{
  SceneUser* user = getMasterUser();
  if (user == nullptr || user->getScene() != getScene())
    return false;
  if (m_bCatching)
    return false;
  if (isAlive() == false)
    return false;

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  m_bCatching = true;

  DWORD time = now() + rCFG.dwCatchSkillPlayTime; // 投掷动作表现时间
  changeCatchState(ECATCHPETSTATE_SKILL, time);

  DWORD skillid = rCFG.dwCatchSkill;
  user->m_oSkillProcessor.useBuffSkill(user, this, skillid, true, true);

  XLOG << "[宠物-捕捉], 玩家:" << user->name << user->id << "开始捕捉" << name << id << XEND;
  return true;
}

void CatchPetNpc::onStopCatch()
{
  if (m_eCatchState != ECATCHPETSTATE_CARTOON)
    return;
  // 标记捕捉结束
  DWORD cur = now();
  m_dwCatchTimerTick = cur;
  processCatch(cur);
}

void CatchPetNpc::onNpcDie()
{
  SceneNpc::onNpcDie();
  SceneUser* user = getMasterUser();
  if (user)
    user->getUserPet().clearCatchPetID(this->id);
}

PetNpc::PetNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{

}

PetNpc::~PetNpc()
{

}

SceneUser* PetNpc::getMasterUser() const
{
  if (define.m_oVar.m_qwOwnerID == 0)
    return nullptr;
  return SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
}

void PetNpc::onNpcDie()
{
  SceneNpc::onNpcDie();
  SceneUser* master = getMasterUser();
  if (master)
    master->getUserPet().onPetDie(this);
}

DWORD PetNpc::getModifiedSkill(DWORD skillid) const
{
  for (auto &s : m_setActiveSkills)
  {
    if (s / ONE_THOUSAND == skillid / ONE_THOUSAND)
      return s;
  }

  return skillid;
}

DWORD PetNpc::getSkillLv(DWORD skillGroupID)
{
  for (auto &s : m_setActiveSkills)
  {
    if (s / ONE_THOUSAND == skillGroupID)
      return s % ONE_THOUSAND;
  }

  return 0;
}

void PetNpc::resetActiveSkills(TSetDWORD& skills)
{
  SceneUser* user = getMasterUser();
  if (user)
  {
    for (auto &s : m_setActiveSkills)
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(s);
      if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
      {
        m_oBuff.delSkillBuff(s);
        const TSetDWORD& masterbuff = pSkill->getPetMasterBuffs();
        for (auto &b : masterbuff)
          user->m_oBuff.del(b);
      }
    }
    for (auto &s : skills)
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(s);
      if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
      {
        m_oBuff.addSkillBuff(s);
        const TSetDWORD& masterbuff = pSkill->getPetMasterBuffs();
        for (auto &b : masterbuff)
          user->m_oBuff.add(b, this, s);
      }
    }
  }
  m_setActiveSkills.clear();
  m_setActiveSkills.insert(skills.begin(), skills.end());
}

void PetNpc::useSuperSkill(DWORD skillid, QWORD targetid, xPos pos, bool specpos)
{
  for (auto &s : m_setActiveSkills)
  {
    if (skillid / ONE_THOUSAND == s / ONE_THOUSAND)
    {
      skillid = s;
      break;
    }
  }

  SceneNpc::useSuperSkill(skillid, targetid, pos, specpos);
}

bool PetNpc::isScreenLimit(SceneUser* pUser)
{
  // return true, 表示限制可见
  if (pUser == nullptr)
    return false;

  if (pUser->isMyTeamMember(define.m_oVar.m_qwOwnerID))
    return false;

  SceneUser* pMaster = getMasterUser();
  if (pMaster == nullptr)
    return false;

  if (pMaster->getScene() && !pMaster->getScene()->inScope(pMaster, pUser))
    return true;

  return false;
}

bool PetNpc::isHideUser(SceneUser* pUser)
{
  if (getAttr(EATTRTYPE_HIDE) == 0)
    return false;
  if (getScene() == nullptr || getScene()->isHideUser() == false)
    return false;
  SceneUser* master = getMasterUser();
  if (!master)
    return false;
  return !master->canSeeHideBy(pUser);
}

bool PetNpc::inGuildZone()
{
  SceneUser* pMaster = getMasterUser();
  if (pMaster == nullptr)
    return false;
  return pMaster->inGuildZone();
}

bool PetNpc::inSuperGvg()
{
  SceneUser* pMaster = getMasterUser();
  if (pMaster == nullptr)
    return false;
  return pMaster->inSuperGvg();
}
/************************************************************************/
/*FoodNpc                                                                      */
/************************************************************************/

FoodNpc::FoodNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{
  reset();
}

FoodNpc::~FoodNpc()
{
}

bool FoodNpc::init(const SNpcCFG* pCFG, const NpcDefine &def)
{
  if (SceneNpc::init(pCFG, def) == false)
    return false;
  if (m_pCurCFG == nullptr)
    return false;
  reset();
  return true;
}

void FoodNpc::setOwnerID(QWORD ownerId, const string& ownerName) {
  m_qwOwnerId = ownerId; m_strOwnerName = ownerName;
  SceneNpcManager::getMe().addFoodNpcCount(ownerId);
  m_bSys = false;
  updateClearTime();
}

bool FoodNpc::checkCanEat(SceneUser* pUser)
{
  if (!pUser)
    return false;  
  //check end
  if (m_bOver)
    return false;
  //check power  
  switch (m_power)
  {
  case EEATPOWR_SELF:
  {
    if (pUser->id != m_qwOwnerId)
      return false;
    break;
  }
  case EEATPOWR_TEAM:
  {
    if (!pUser->isMyTeamMember(m_qwOwnerId))
      return false;
    break;
  }
  case EEATPOWR_ALL:
  {
    break;
  }
  default:
    return false;
    break;
  }  
  return true;
}

bool FoodNpc::addEater(SceneUser* pUser)
{
  if (!pUser)
    return false;
  
  if (getStatus() != ECREATURESTATUS_LIVE)
    return false;

  QWORD playerId = pUser->id;
  auto it = find_if(m_eaters.begin(), m_eaters.end(), [playerId](TEaterPair& r) -> bool {
    return r.first == playerId;
  });    

  if (it != m_eaters.end())
    return false;

  if(pUser->getSceneFood().checkFoodLimitNum(getFoodId()) == false)
    return false;
  
  TEaterPair p;
  p.first = playerId;
  FoodEaterInfo stEater;
  stEater.m_dwTime = now();
  stEater.m_dwProgress = 0;
  p.second = stEater;
  m_eaters.push_back(p);  
  
  //放技能
  DWORD* pSkillId = randomStlContainer(MiscConfig::getMe().getFoodCfg().vecEatSkill);
  if (!pSkillId)
    return false;
  
  pUser->m_oSkillProcessor.useBuffSkill(pUser, this, *pSkillId, true);
  updateClearTime();
  
  XLOG << "[料理-吃料理] 开始吃 charid" << playerId << "料理npcguid" << getGUID() << "料理itemid" << m_dwItemId << "放置者" << m_qwOwnerId << "当前进度" << m_dwProgress << "吃的人数" << m_eaters.size() << "技能id" << *pSkillId << XEND;
  return true;
}

bool FoodNpc::addPetEater(SceneNpc* pPet)
{
  if (pPet == nullptr || pPet->getNpcType() != ENPCTYPE_PETNPC)
    return false;

  SceneUser* pMaster = pPet->getMasterUser();
  if (pMaster == nullptr)
    return false;

  QWORD playerId = pPet->id;
  auto it = find_if(m_eaters.begin(), m_eaters.end(), [playerId](TEaterPair& r) -> bool {
    return r.first == playerId;
  });

  if (it != m_eaters.end())
    return false;

  TEaterPair p;
  p.first = playerId;
  FoodEaterInfo stEater;
  stEater.m_dwTime = now();
  stEater.m_dwProgress = 0;
  p.second = stEater;
  m_eaters.push_back(p);  

  updateClearTime();
  XLOG << "[宠物-吃料理], 开始" << "料理:" << name << id << "宠物:" << pPet->name << pPet->id << "主人:" << pMaster->name << pMaster->id << XEND;
  return true;
}

bool FoodNpc::delEater(SceneUser* pUser)
{
  if (!pUser)
    return false;
  QWORD playerId = pUser->id;
  auto it = find_if(m_eaters.begin(), m_eaters.end(), [playerId](TEaterPair& r) -> bool {
    return r.first == playerId;
  });

  if (it == m_eaters.end())
    return false;
  //m_eaters.erase(it);
  m_setDel.insert(playerId);
  //打断技能消息
  pUser->m_oSkillProcessor.breakSkill(pUser->id);

  XLOG << "[料理-吃料理] 停止吃 charid" << playerId << "料理npcguid" << getGUID() << "料理itemid" << m_dwItemId << "放置者" << m_qwOwnerId << "当前进度" << m_dwProgress << "吃的人数" << m_eaters.size() << XEND;
  return true;
}

bool FoodNpc::delPetEater(SceneNpc* pPet)
{
  if (pPet == nullptr || pPet->getNpcType() != ENPCTYPE_PETNPC)
    return false;

  SceneUser* pMaster = pPet->getMasterUser();
  if (pMaster == nullptr)
    return false;

  QWORD playerId = pPet->id;
  auto it = find_if(m_eaters.begin(), m_eaters.end(), [playerId](TEaterPair& r) -> bool {
    return r.first == playerId;
  });

  if (it == m_eaters.end())
    return false;
  //m_eaters.erase(it);
  m_setDel.insert(playerId);

  XLOG << "[宠物-吃料理], 删除" << "料理:" << name << id << "宠物:" << pPet->name << pPet->id << "主人:" << pMaster->name << pMaster->id << XEND;
  return true;
}

void FoodNpc::onOneSecTimeUp(QWORD curMSec)
{
  const SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(m_dwItemId);
  if (pFoodCfg == nullptr)
    return;

  SceneNpc::onOneSecTimeUp(curMSec);
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);
  
  if (m_bOver == false)
  {
    delIter();

    for (auto& v : m_eaters)
    {
      if (curSec - pFoodCfg->m_dwStepDuration > v.second.m_dwTime)
      {
        v.second.m_dwTime += pFoodCfg->m_dwStepDuration;
        //只放置一个料理的话 走以前的流程
        if (getFoodTotalNum() == 1 || pFoodCfg->m_bMultiEat)
        {
          m_dwProgress++;
          //状态同步给九屏
          setStatus1(m_dwProgress);
          sendMeToNine();
          setStatus2(m_dwProgress);
          XDBG << "[料理-吃料理] 料理npc状态同步，npcguid" << getGUID() << "itemid" << m_dwItemId << "放置者" << m_qwOwnerId << "当前进度" << m_dwProgress << "总进度" << pFoodCfg->m_dwTotalStep << "单进度时长" << pFoodCfg->m_dwStepDuration << XEND;
          //多人份
          if (pFoodCfg->m_bMultiEat)
          {
            onEatSuccess(v.first);
          }
          else
          {//单人份
            if (m_dwProgress >= pFoodCfg->m_dwTotalStep)
            {
              onEatSuccess(v.first);
            }
          }   

          if (m_dwProgress >= pFoodCfg->m_dwTotalStep)
          {
            onEatOver();
            break;
          }
        }
        else
        {
          //有n个料理的话 每个玩家的进度条独立
          v.second.m_dwProgress++;
          if (v.second.m_dwProgress >= pFoodCfg->m_dwTotalStep)
          {
            onEatSuccess(v.first);
            DWORD dwProgress = calcFoodNumProgress(pFoodCfg->m_dwTotalStep);
            setStatus1(dwProgress);
            sendMeToNine();
            setStatus2(dwProgress);

            XDBG << "[料理-吃料理] 料理npc状态同步，npcguid" << getGUID() << "itemid" << m_dwItemId << "剩余数量" << getFoodNum() << "总数" << getFoodTotalNum() << "放置者" << m_qwOwnerId << "当前进度" << dwProgress << "总进度" << pFoodCfg->m_dwTotalStep << XEND;
          }

          if (getFoodNum() == 0)
          {
            onEatOver();
            break;
          }
        }
      }
    }  

    delIter();
  }
}

void FoodNpc::delIter()
{
  if (m_setDel.empty())
    return;

  for (auto it = m_eaters.begin(); it != m_eaters.end();)
  {
    if (isInDel(it->first))
    {
      it = m_eaters.erase(it);
      continue;
    }
    ++it;
  }
  m_setDel.clear();
}

void FoodNpc::onEatSuccess(QWORD id)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(id);
  /*if (pUser == nullptr)
  {
    XERR << "[料理-吃料理] 失败，玩家不在线，不可能发生，charid" << id << "料理npcguid" << getGUID() << XEND;
    return;
  }  
  */
  if (pUser)
  {
    pUser->getSceneFood().onEatFood(this); 
    pUser->getSceneFood().stopEat(this, true);
    XLOG << "[料理-吃料理] 获得一份料理效果 charid" << id << "料理npcguid" << getGUID() << "料理itemid" << m_dwItemId << "放置者" << m_qwOwnerId << "当前进度" << m_dwProgress << "吃的人数" << m_eaters.size() << XEND;
    return;
  }

  SceneNpc* pPet = SceneNpcManager::getMe().getNpcByTempID(id);
  if (pPet)
  {
    SceneUser* user = pPet->getMasterUser();
    if (user)
    {
      user->getSceneFood().onPetEatFood(this, pPet);
      user->getUserPet().onStopFood(this);
    }
    delPetEater(pPet);
  }
}

void FoodNpc::onEatOver()
{
  //通知玩家停止进食  
  std::vector<SceneUser*> allUser;
  allUser.reserve(m_eaters.size());
  for (auto &v : m_eaters)
  {
    if (isInDel(v.first))
      continue;

    SceneUser* pUser = SceneUserManager::getMe().getUserByID(v.first);
    if (pUser)
      allUser.push_back(pUser);
    else
    {
      SceneNpc* pPet = SceneNpcManager::getMe().getNpcByTempID(v.first);
      if (pPet)
      {
        SceneUser* pMaster = pPet->getMasterUser();
        if (pMaster)
        {
          pMaster->getUserPet().onStopFood(this);
        }
      }
    }
  }
  
  for (auto&v : allUser)
    v->getSceneFood().stopEat(this, false);

  //删除npc
  setClearState();
  m_bOver = true;
}

void FoodNpc::changePower(SceneUser* pOwner, Cmd::EEatPower power)
{
  if (!pOwner)
    return;
  if (m_power == power)
    return;   
  setPower(power);

  std::vector<SceneUser*> allUser;
  allUser.reserve(m_eaters.size());

  std::list<SceneUser*> petmasters;
  for (auto &v : m_eaters)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(v.first);
    if (pUser)
      allUser.push_back(pUser);
    else
    {
      SceneNpc* pPet = SceneNpcManager::getMe().getNpcByTempID(v.first);
      if (pPet)
      {
        SceneUser* pMaster = pPet->getMasterUser();
        if (pMaster && checkCanEat(pMaster) == false)
        {
          pMaster->getUserPet().onStopFood(this);
        }
      }
    }
  }

  for (auto&v : allUser)
  {
    if (checkCanEat(v) == false)
    {
      //权限被改不能吃了
      //“xxx修改了品尝权限”
      MsgManager::sendMsg(v->id, 3503, MsgParams(pOwner->name));
      v->getSceneFood().stopEat(this, false);
    }
  } 
}

DWORD FoodNpc::calcFoodNumProgress(DWORD dwTotalStep)
{
  if (m_dwItemTotalNum == 0)
    return dwTotalStep;

  if (m_dwItemTotalNum >= m_dwItemNum)
  {
    return (DWORD)(((m_dwItemTotalNum - m_dwItemNum) * 1.0f / m_dwItemTotalNum) * dwTotalStep);
  }
  return dwTotalStep;
}

void FoodNpc::setStatus1(DWORD dwProgress)
{
  switch (dwProgress)
  {
  case 0:
  {
    m_dwGearStatus = 1001;
    break;
  }
  case 1:
  {
    m_dwGearStatus = 2001;
    break;
  }
  case 2:
  {
    m_dwGearStatus = 3001;
    break;
  }
  case 3:
  {
    m_dwGearStatus = 4001;
    break;
  }
  case 4:
  {
    m_dwGearStatus = 5001;
    break;
  }
  case 5:
  {
    m_dwGearStatus = 6001;
    break;
  }
  default:
    break;
  }
}

void FoodNpc::setStatus2(DWORD dwProgress)
{
  switch (dwProgress)
  {
  case 0:
  {
    m_dwGearStatus = 1002;
    break;
  }
  case 1:
  {
    m_dwGearStatus = 2002;
    break;
  }
  case 2:
  {
    m_dwGearStatus = 3002;
    break;
  }
  case 3:
  {
    m_dwGearStatus = 4002;
    break;
  }
  case 4:
  {
    m_dwGearStatus = 5002;
    break;
  }
  case 5:
  {
    m_dwGearStatus = 6002;
    break;
  }
  default:
    break;
  }
}

void FoodNpc::updateClearTime()
{
  if (m_bSys)
    return;

  DWORD d = MiscConfig::getMe().getFoodCfg().dwFoodNpcDuration;
  if (d)
    m_dwSetClearTime = now() + d;
  XDBG << "[料理-刷新存在时间] 料理npcguid" << getGUID() << "料理itemid" << m_dwItemId << "时间" << m_dwSetClearTime << XEND;
}

bool FoodNpc::checkPetCanEat(SceneNpc* pPet)
{
  if (pPet == nullptr)
    return false;
  SceneUser* pMaster = pPet->getMasterUser();
  if (pMaster == nullptr)
    return false;
  return checkCanEat(pMaster);
}

void FoodNpc::reset()
{
  if (m_dwItemId == 0)
  {
    m_dwItemId = FoodConfig::getMe().getFoodIdByNpcId(SceneNpc::getNpcID());
  }

  SFoodConfg* pFoodCfg = FoodConfig::getMe().getFoodCfg(m_dwItemId);
  if (pFoodCfg != nullptr)
  {
    m_dwItemNum = pFoodCfg->m_dwTotalStep;
    m_dwItemTotalNum = pFoodCfg->m_dwTotalStep;
  }

  m_eaters.clear();
  m_dwProgress = 0;
  m_bOver = false;
  XDBG << "[料理-npc] init，itemid" << SceneNpc::getNpcID() << XEND;
}

bool FoodNpc::isScreenLimit(SceneUser* pUser)
{
  // return true, 表示限制可见
  if (pUser == nullptr)
    return false;

  if (pUser->isMyTeamMember(define.m_oVar.m_qwOwnerID))
    return false;

  SceneUser* pMaster = SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
  if (pMaster == nullptr)
    return false;

  if (pMaster->getScene() && !pMaster->getScene()->inScope(pMaster, pUser))
    return true;

  return false;
}

// 生命体
BeingNpc::BeingNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{
}

BeingNpc::~BeingNpc()
{
}

SceneUser* BeingNpc::getMasterUser() const
{
  if (define.m_oVar.m_qwOwnerID == 0)
    return nullptr;
  return SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
}

DWORD BeingNpc::getMasterCurLockID() const
{  
  SceneUser* pMaster = getMasterUser();
  if(pMaster)
    return pMaster->getUserBeing().getMasterCurLockID();
  return 0;
}

void BeingNpc::onNpcDie()
{
  SceneNpc::onNpcDie();
  SceneUser* master = getMasterUser();
  if (master)
    master->getUserBeing().onBeingDie(this);
}

bool BeingNpc::canUseSkill(const BaseSkill* skill)
{
  if (SceneNpc::canUseSkill(skill) == false)
    return false;

  bool noCheckCost = false;
  if (skill->isCostBeforeChant() && m_oSkillProcessor.getRunner().getState() == ESKILLSTATE_CHANT) noCheckCost = true;//引导技能等, 仅在吟唱开始前检查消耗

  if (!noCheckCost && skill->checkSkillCost(this) == false)
    return false;

  return true;
}

bool BeingNpc::isScreenLimit(SceneUser* pUser)
{
  // return true, 表示限制可见
  if (pUser == nullptr)
    return false;

  if (pUser->isMyTeamMember(define.m_oVar.m_qwOwnerID))
    return false;

  SceneUser* pMaster = getMasterUser();
  if (pMaster == nullptr || pMaster->getScene() == nullptr)
    return false;

  if (!pMaster->getScene()->inScope(pMaster, pUser))
    return true;

  return false;
}

bool BeingNpc::isHideUser(SceneUser* pUser)
{
  if (getAttr(EATTRTYPE_HIDE) == 0)
    return false;
  if (getScene() == nullptr || getScene()->isHideUser() == false)
    return false;
  SceneUser* master = getMasterUser();
  if (!master)
    return false;
  return !master->canSeeHideBy(pUser);
}

bool BeingNpc::inGuildZone()
{
  SceneUser* pMaster = getMasterUser();
  if (pMaster == nullptr)
    return false;
  return pMaster->inGuildZone();
}

bool BeingNpc::inSuperGvg()
{
  SceneUser* pMaster = getMasterUser();
  if (pMaster == nullptr)
    return false;
  return pMaster->inSuperGvg();
}

DWORD BeingNpc::getSkillLv(DWORD skillGroupID)
{
  SceneUser* master = getMasterUser();
  if (master == nullptr)
    return 0;
  SSceneBeingData* being = master->getUserBeing().getBeingData(getNpcID());
  if (being == nullptr)
    return 0;
  return being->getSkillLv(skillGroupID);
}

//贤者召唤的 元素精灵
ElementElfNpc::ElementElfNpc(QWORD id, const SNpcCFG* pCFG, const NpcDefine& def) : SceneNpc(id, pCFG, def)
{
}

ElementElfNpc::~ElementElfNpc()
{
}

SceneUser* ElementElfNpc::getMasterUser() const
{
  if (define.m_oVar.m_qwOwnerID == 0)
    return nullptr;
  return SceneUserManager::getMe().getUserByID(define.m_oVar.m_qwOwnerID);
}

DWORD ElementElfNpc::getMasterCurLockID() const
{
  SceneUser* pMaster = getMasterUser();
  if(pMaster)
    return pMaster->getUserElementElf().getMasterCurLockID();
  return 0;
}

