#include "UserEvent.h"
#include "xTools.h"
#include "Scene.h"
#include "SceneUser.h"
#include "SceneManager.h"
#include "Package.h"
#include "SceneUserManager.h"
#include "DScene.h"
#include "ChatRoom.h"
#include "ChatRoomManager.h"
#include "ChatManager_SC.h"
#include "GMCommandRuler.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "Manual.h"
#include "SceneServer.h"
#include "PlatLogManager.h"
#include "MsgManager.h"
#include "xSha1.h"
#include "StatisticsDefine.h"
#include "ActivityManager.h"
#include "PatchManager.h"
#include "AuguryMgr.h"
#include "SkillItem.h"
#include "AstrolabeConfig.h"
#include "GuildCityManager.h"
#include "PetWork.h"
#include "SkillManager.h"
#include "SceneShop.h"
#include "Menu.h"
#include "UserRecords.h"
#include "DressUpStageMgr.h"
#include "ExchangeShop.h"
#include "BossMgr.h"
#include "StatMgr.h"

UserEvent::UserEvent(SceneUser *user):m_pUser(user)
{
}

UserEvent::~UserEvent()
{
}

void UserEvent::onLogin()
{
  UserSceneData& oData = m_pUser->getUserSceneData();
  oData.setOnlineTime();
  oData.setTeamTimeLen(0);
  oData.notifyDeadBoss();
  //oData.setFollowerIDNoCheck(0);
  //if (oData.getFollowerID())
  //{
    /*FollowerUser cmd;
    cmd.set_userid(oData.getFollowerID());
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);*/
  //}

  m_pUser->m_oHands.userOnline();
  m_pUser->antiAddictRefresh();
  m_pUser->addcitTips(true);

  if (m_pUser->getVar().getVarValue(EVARTYPE_DAY_GET_ZENY_COUNT) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_DAY_GET_ZENY_COUNT, 1);
    m_pUser->getUserSceneData().setDailyNormalZeny(0);
    m_pUser->getUserSceneData().setDailyChargeZeny(0);
  }

  // offline exp
  int isBaseLvFull = 0;
  int isJobLvFull = 0;
  DWORD dwBaseExp = LuaManager::getMe().call<DWORD>("calcUserOfflineExp", m_pUser, xTime::getCurSec(), oData.getOfflineTime());
  if (dwBaseExp != 0)
    m_pUser->addBaseExp(dwBaseExp, ESOURCE_OFFLINE, &isBaseLvFull);

  DWORD dwJobExp = LuaManager::getMe().call<DWORD>("calcUserOfflineJobExp", m_pUser, xTime::getCurSec(), oData.getOfflineTime());
  if (dwJobExp != 0)
    m_pUser->addJobExp(dwJobExp, ESOURCE_OFFLINE, &isJobLvFull);
  if (dwBaseExp || dwJobExp)
  {
    DWORD dwTime = LuaManager::getMe().call<DWORD>("calcUserOfflineTime", xTime::getCurSec(), oData.getOfflineTime());
    if (!isBaseLvFull && !isJobLvFull)
    {
      MsgParams msgParams;
      msgParams.addNumber(dwTime);
      msgParams.addNumber(dwBaseExp);
      msgParams.addNumber(dwJobExp);
      MsgManager::sendMsg(m_pUser->id, 2502, msgParams);
    }
    else if (isJobLvFull && !isBaseLvFull)
    {
      MsgParams msgParams;
      msgParams.addNumber(dwTime);
      msgParams.addNumber(dwBaseExp);
      MsgManager::sendMsg(m_pUser->id, 2504, msgParams);
    }
    else if (isBaseLvFull && !isJobLvFull)
    {
      MsgParams msgParams;
      msgParams.addNumber(dwTime);
      msgParams.addNumber(dwJobExp);
      MsgManager::sendMsg(m_pUser->id, 2505, msgParams);
    }
  }

  if (m_pUser->getUserSceneData().getOriginalZoneID() == 0 || m_pUser->getUserSceneData().getOriginalZoneID() == thisServer->getZoneID())
    SceneUserManager::getMe().setSvrMaxBaseLv(m_pUser);

  // first login everyday
  if (m_pUser->getVar().getVarValue(EVARTYPE_DAY_ONLINE_FIRST) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_DAY_ONLINE_FIRST, 1);
    const SCreditCFG& creditCFG = MiscConfig::getMe().getCreditCFG();
    if (m_pUser->getUserSceneData().getCredit() > (int)creditCFG.dwDecLimitValue)
    {
      m_pUser->getUserSceneData().decCredit(creditCFG.dwDayDecValue);
      XLOG << "[玩家-信用度], 玩家" << m_pUser->name << m_pUser->id << "每日上线扣除信用度:" << creditCFG.dwDayDecValue << XEND;
    }
  }

  std::pair<DWORD, DWORD> wantedquest;
  if (m_pUser->getQuest().getWantedQuest(wantedquest, true))
  {
    TeamerQuestUpdateSocialCmd teamcmd;
    MemberWantedQuest* pTeamQuest = teamcmd.mutable_quest();
    pTeamQuest->set_action(EQUESTACTION_ACCEPT);
    pTeamQuest->set_charid(m_pUser->id);
    pTeamQuest->set_questid(wantedquest.first);
    pTeamQuest->set_step(wantedquest.second);
    PROTOBUF(teamcmd, send2, len2);
    thisServer->sendCmdToSession(send2, len2);
  }
  else
  {
    TeamerQuestUpdateSocialCmd teamcmd;
    MemberWantedQuest* pTeamQuest = teamcmd.mutable_quest();
    pTeamQuest->set_action(EQUESTACTION_ABANDON_GROUP);
    pTeamQuest->set_charid(m_pUser->id);
    PROTOBUF(teamcmd, send2, len2);
    thisServer->sendCmdToSession(send2, len2);
  }
  // call lua
  LuaManager::getMe().call<void>("onLogin", m_pUser);

  // 重登陆检查新技能
  if (m_pUser->getFighter())
    m_pUser->getFighter()->getSkill().refreshEnableSkill();

  m_pUser->getGuildChallenge().onLogin();
  m_pUser->m_oUserStat.onLogin();
  m_pUser->m_oBooth.onLogin();

  m_pUser->SyncOperateRewardToSession();
  m_pUser->getEvent().onPassTower();
  m_pUser->getAchieve().onProfession();

  Package& rPackage = m_pUser->getPackage();
  rPackage.reqUsedItemCode();
  rPackage.resetPetWorkItem();
  rPackage.resetPetPackItem();
  rPackage.resetFoodPackItem();
  rPackage.resetQuestManualItem();

  Quest& rQuest = m_pUser->getQuest();
  rQuest.refreshCollectPuzzles();

  // 周年庆-好友回归
  m_pUser->sendRecallReward();
  m_pUser->getUserSceneData().notifyActivityEventToClient();

  GuildCityManager::getMe().userOnLogin(m_pUser);
  Menu& rMenu = m_pUser->getMenu();
  rMenu.refreshNewMenu(EMENUCOND_PROFESSION);

  //追赶系统
  m_pUser->getExchangeShop().onLogin();

  m_pUser->setDataMark(EUSERDATATYPE_ENSEMBLESKILL);
  // send var to session
  SyncUserVarSessionCmd varcmd;
  m_pUser->getVar().collectSessionVars(varcmd);
  if (varcmd.vars_size())
  {
    varcmd.set_charid(m_pUser->id);
    PROTOBUF(varcmd, varsend, varlen);
    thisServer->sendCmdToSession(varsend, varlen);
  }

  std::stringstream ss2;
  ss2 << m_pUser->id << "_" << now();
  char sign[SHA1_LEN + 1];
  bzero(sign, sizeof(sign));
  string& logsign = m_pUser->getLogSign();
  logsign.clear();
  if (getSha1Result(sign, ss2.str().c_str(), ss2.str().size()))
    logsign = sign;
  PlatLogManager::getMe().loginLog(thisServer,
    oData.getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    "", /*ip*/
    oData.getCharge(),
    1,
    oData.getRolelv(),
    logsign,
    ""/*device*/,
    0/*guest*/,
    ""/*mac*/,
    ""/*agent*/,     
    oData.getOnlineMapID(),
    0,0, oData.isNew());
    
  //活跃玩家日志
  do 
  {
    DWORD sendCount = m_pUser->getVar().getAccVarValue(EACCVARTYPE_INACTIVE_USER_SEND_COUNT);
    //本周不用推送
    if (sendCount >= 9999)
      break;

    DWORD day = xTime::getDayStart(now());
    DWORD lastDay = m_pUser->getVar().getAccVarValue(EACCVARTYPE_INACTIVE_USER_LAST_SEND_DAY);
    if (lastDay == 0)
    {
      m_pUser->getVar().setAccVarValue(EACCVARTYPE_INACTIVE_USER_LAST_SEND_DAY, day);
      break;
    }
    if (lastDay == day)
      break;  //今天已经推送过
    sendCount++;
    PlatLogManager::getMe().InactiveUserLog(thisServer,
      m_pUser->accid,
      m_pUser->id,
      m_pUser->name,
      m_pUser->getProfession(),
      m_pUser->getLevel(),
      m_pUser->getUserSceneData().getSilver(),
      m_pUser->getUserSceneData().getLastMapID(),
      m_pUser->getGuild().id(),
      m_pUser->getUserSceneData().getCreateTime(),
      sendCount
     );
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_INACTIVE_USER_SEND_COUNT, sendCount);
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_INACTIVE_USER_LAST_SEND_DAY, day);

  } while (0);  
 
  XLOG << "[玩家-登陆] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 在 mapid:" << oData.getOnlineMapID() << " 登陆" << XEND;
}

void UserEvent::onUseSkill(const BaseSkill* pSkill)
{
  if (pSkill == nullptr)
    return;
  //TODO
  //加成就123
  //完成某任务
  //etc.

  //非拍照技能打断坐下。
  if (pSkill->getSkillType() != ESKILLTYPE_FLASH)
  {
    m_pUser->seatUp();
    m_pUser->getSceneFood().breakCooking();
  }
  
  if (!MiscConfig::getMe().getFoodCfg().isEatSkill(pSkill->getSkillID()))
    m_pUser->getSceneFood().breakEating();

  m_pUser->m_oBuff.onUseSkill(pSkill);
  m_pUser->m_oSkillStatus.onUseSkill();
  m_pUser->getTransform().onUseSkill(pSkill->getSkillID());
}

//warning:队友击杀的 bSelf 也为true
void UserEvent::onKillNpc(SceneNpc* pNpc, bool bSelf, bool count)
{
  //任务计数+1
  //加成就233
  //etc.

  if (m_pUser == NULL || pNpc == nullptr)
    return;

  //m_pUser->m_oBuff.onKillMonster(pNpc);
  m_pUser->getQuest().onMonsterKill(pNpc->id);
  BossMgr::getMe().onKillMonster(m_pUser, pNpc->getNpcID());
  bool bManual = false;
  if (pNpc->getNpcType() == ENPCTYPE_MINIBOSS || pNpc->getNpcType() == ENPCTYPE_MVP)
    bManual = true;
  else
    bManual = bSelf;
  if (bManual)
  {
    m_pUser->getManual().onKillMonster(pNpc->m_oOriDefine.getID());
    //m_pUser->getManual().onKillProcess(pNpc->m_oOriDefine.getID());
  }

  if (pNpc->getNpcType() == ENPCTYPE_MVP)
    m_pUser->getUserPet().playAction(EPETACTION_OWNER_KILLMVP);
  else if (pNpc->getNpcType() == ENPCTYPE_MINIBOSS)
    m_pUser->getUserPet().playAction(EPETACTION_OWNER_KILLMINI);

  if(count)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(pNpc->define.getID());
    bool blCount = false;
    Scene* pScene = m_pUser->getScene();
    if(pScene->isDScene())
    {
      DScene* pDScene = dynamic_cast<DScene*>(pScene);
      TowerScene* pTower = dynamic_cast<TowerScene*>(pDScene);
      GuildScene* pGuild = dynamic_cast<GuildScene*>(pDScene);
      if(pTower != nullptr || pGuild != nullptr)
        blCount = true;
    }
    if (pCFG != nullptr && !blCount)
      m_pUser->getManual().onKillProcess(pNpc->m_oOriDefine.getID());
  }

  if (bSelf)
  {
    if (pNpc->isMonster())
    {
      const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
      if (pNpc->getNpcType() == ENPCTYPE_MONSTER)
      {
        m_pUser->getUserSceneData().addMonsterCredit(rCFG.dwMonsterValue);
      }
      else if (pNpc->getNpcType() == ENPCTYPE_MVP)
      {
        m_pUser->getUserSceneData().addCredit(rCFG.dwMvpValue);
      }
      else if (pNpc->getNpcType() == ENPCTYPE_MINIBOSS)
      {
        m_pUser->getUserSceneData().addCredit(rCFG.dwMiniValue);
      }

      m_pUser->getAchieve().onKillMonster(pNpc->getNpcType(), pNpc->getNpcID());
      m_pUser->getGuildChallenge().onKillNpc(pNpc);

      if (m_pUser->getScene() && m_pUser->getScene()->getSceneType() == SCENE_TYPE_GUILD_FIRE)
        m_pUser->getUserGvg().onKillMonster();
    }
  }
}

void UserEvent::onBeBuffStatus(DWORD status)
{
  if (m_pUser->getScene() == nullptr)
    return;
  DWORD range = FeatureConfig::getMe().getNpcAICFG().stStatusAttack.dwFindRange;
  xSceneEntrySet uSet;
  m_pUser->getScene()->getEntryList(m_pUser->getPos(), range, uSet);
  for (auto &s : uSet)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
    if (npc == nullptr)
      continue;
    npc->m_ai.onSeeStatus(m_pUser, status);
  }
}

void UserEvent::onEnterScene()
{
  m_pUser->m_oCarrier.onUserEnter();
  m_pUser->m_oFollower.enterScene();
  m_pUser->getQuestNpc().onUserEnterScene();
  m_pUser->getQuest().resetQuest(EQUESTRESET_CHANGEMAP);
  m_pUser->m_oHands.enterScene();
  m_pUser->enterSceneSendRandom();
  m_pUser->getWeaponPet().onUserEnterScene();
  m_pUser->getUserPet().onUserEnterScene();
  m_pUser->getTower().resetData();
  m_pUser->getUserBeing().onUserEnterScene();
  m_pUser->getUserElementElf().onUserEnterScene();
  //m_pUser->getQuest().sendQuestDetailList();
  //m_pUser->getQuest().sendCurQuestList();
  //m_pUser->getQuest().queryOtherData(EOTHERDATA_DAILY);
  m_pUser->getTutorTask().onEnterScene();
  m_pUser->m_oBuff.onEnterScene();

  if (m_pUser->getScene())
  {
    m_pUser->getQuest().sendCurQuestList(m_pUser->getMapID());
    m_pUser->getQuest().queryOtherData(EOTHERDATA_DAILY);
    m_pUser->getQuest().onEnterScene();

    m_pUser->getScenery().send(m_pUser->getScene()->getMapID());
    m_pUser->getSeal().onEnterScene();
    m_pUser->getAchieve().onEnterMap();
    m_pUser->getScene()->sendVisibleNpc(m_pUser, true);

    if (m_pUser->getTeamID() == 0)
    {
      DScene* pScene = dynamic_cast<DScene*>(m_pUser->getScene());
      if (pScene != nullptr && (pScene->getRaidRestrict() == ERAIDRESTRICT_TEAM || pScene->getRaidRestrict() == ERAIDRESTRICT_GUILD_TEAM || pScene->getRaidRestrict() == ERAIDRESTRICT_GUILD_RANDOM_RAID))
      {
        pScene->addKickList(m_pUser);
      }
      if (pScene != nullptr && pScene->getRaidRestrict() == ERAIDRESTRICT_USER_TEAM)
      {
        if (pScene->m_oImages.userCanEnter(m_pUser) == false)
          pScene->addKickList(m_pUser);
      }
      if (pScene && pScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
      {
        GuildFireScene* pFireScene = dynamic_cast<GuildFireScene*>(pScene);
        if (pFireScene)
        {
          if (GuildCityManager::getMe().isCityInFire(pFireScene->getCityID()) == false)
          {
            if (pFireScene->getDefenseGuildID() == 0 || pFireScene->getDefenseGuildID() != m_pUser->getGuild().id())
              pScene->addKickList(m_pUser);
          }
        }
      }
    }
    GuildScene* pGuildScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
    if (pGuildScene != nullptr)
      m_pUser->getUserPet().playAction(EPETACTION_OWNER_ENTERGUILD);
    if (m_pUser->getScene()->isGvg())
    {
      if (m_pUser->getFighter())
      {
        m_pUser->updateAttribute();

        DWORD hp = m_pUser->getAttr(EATTRTYPE_HP);
        const SGuildFireCFG& cfg = MiscConfig::getMe().getGuildFireCFG();
        DWORD rate = cfg.dwHpRate ? cfg.dwHpRate : 1;
        m_pUser->getFighter()->setHp(hp * rate);
        m_pUser->setAttr(EATTRTYPE_HP, hp * rate);
      }
    }

    BossMgr::getMe().onEnterScene(m_pUser);
  }
  m_pUser->m_oGingerBread.onEnterScene();

  LuaManager::getMe().call<void>("onEnterScene", m_pUser);
  // gm effect
  vector<string> gms;
  m_pUser->getUserSceneData().getGMCommand(gms);
  for (auto &s : gms)
  {
    GMCommandRuler::getMe().execute(m_pUser, s);
  }

  if (m_pUser->getStatus() == ECREATURESTATUS_FAKEDEAD)
  {
    m_pUser->setStatus(ECREATURESTATUS_LIVE);
  }
  m_pUser->setAction(0);

  DScene* pDScene = dynamic_cast<DScene*>(m_pUser->getScene());
  if (pDScene != nullptr)
  {
    if (pDScene->getRaidType() == ERAIDTYPE_TOWER && m_pUser->getTeamID() != 0)
    {
      TowerScene* pTowerScene = dynamic_cast<TowerScene*>(m_pUser->getScene());
      if (pTowerScene != nullptr)
        MsgManager::sendMsg(m_pUser->id, 1306, MsgParams(pTowerScene->getLayer()));
    }
  }
  if (m_pUser->getScene() && !SceneImage::isImages(m_pUser->getScene()->getMapID(), m_pUser->getScene()->id, m_pUser->getUserSceneData().getLastRealMapID(), m_pUser->getUserSceneData().getLastMapID()))
  {
    if (m_pUser->isAlive())
    {
      m_pUser->m_oBuff.add(MiscConfig::getMe().getSystemCFG().dwGoMapBuff, m_pUser);
      //m_pUser->m_oBuff.update(xTime::getCurMSec());
    }
  }

  Scene* pScene = m_pUser->getScene();
  if (pScene && pScene->getMapID() == 2)
  {
    m_pUser->m_dwEnterNanMenTime = now();
  }
  if (m_pUser->getTeamSeal())
  {
    m_pUser->getTeamSeal()->onEnterScene(m_pUser);
  }

  ActivityManager::getMe().onEnterScene(m_pUser);

  const TSetDWORD& setShowNpc = m_pUser->getUserSceneData().getShowNpcs();
  if (pScene && !setShowNpc.empty())
  {
    std::list<SceneNpc*> list;
    for (auto &it : setShowNpc)
    {
      pScene->getSceneNpcByBaseID(it, list);
    }
    if (!list.empty())
    {
      for (auto &iter : list)
      {
        iter->sendMeToNine();
      }
    }
  }

  std::set<SceneUser*> teammate = m_pUser->getTeamSceneUser();
  for (auto &u : teammate) {
    u->m_oBuff.onTeamChange();
  }

  if (pScene && pScene->isPVPScene() && m_pUser->getFighter())
  {
    const SPvpCommonCFG& rCFG = MiscConfig::getMe().getPvpCommonCFG();
    if (rCFG.dwHpRate)
    {
      DWORD maxhp = m_pUser->getAttr(EATTRTYPE_MAXHP);
      m_pUser->setAttr(EATTRTYPE_MAXHP, maxhp * rCFG.dwHpRate);

      DWORD hp = m_pUser->getAttr(EATTRTYPE_HP);
      m_pUser->getFighter()->setHp(hp * rCFG.dwHpRate);
      m_pUser->setAttr(EATTRTYPE_HP, hp * rCFG.dwHpRate);
    }
  }
  m_pUser->m_dwEnterSceneTime = now();

  if (m_pUser->getScene() && (m_pUser->getScene()->isPVPScene() || m_pUser->getScene()->isGvg()))
  {
    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);

    if (m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP))
    {
      TVecSortItem pVecItems;
      m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->getEquipItems(pVecItems);
      if (pVecItems.empty() == false)
      {
        const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
        for (auto v = pVecItems.begin(); v != pVecItems.end(); ++v)
        {
          ItemEquip* pEquip = dynamic_cast<ItemEquip*> (*v);
          if (!pEquip)
            continue;
          if (pEquip->getIndex() != rCFG.getValidEquipPos(static_cast<EEquipPos>(pEquip->getIndex()), pEquip->getEquipType()))
            continue;
          m_pUser->m_oBuff.addEquipBuffPVP(pEquip);
        }
      }
    }
  }
  
  m_pUser->getBuff().onChangeMap();

  UserEnterSceneSessionCmd scmd;
  scmd.set_charid(m_pUser->id);
  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmdToSession(ssend, slen);
}

void UserEvent::onEnterSceneEnd()
{
  m_pUser->getUserRecords().onEnterSceneEnd();
}

void UserEvent::onLeaveScene()
{
  m_pUser->m_oMove.clearPath();
  m_pUser->m_oFollower.leaveScene();
  m_pUser->getQuestNpc().onUserLeaveScene();
  m_pUser->m_oHands.leaveScene();
  m_pUser->getHandNpc().onLeaveScene();
  m_pUser->getWeaponPet().onUserLeaveScene();
  m_pUser->getUserPet().onUserLeaveScene();
  m_pUser->getUserBeing().onUserLeaveScene();
  m_pUser->getUserElementElf().onUserLeaveScene();

  m_pUser->m_oSkillProcessor.breakSkill(m_pUser->id);
  m_pUser->m_oSkillProcessor.setClearState();
  m_pUser->m_oSkillStatus.onLeaveScene(); // Put before buff
  m_pUser->m_oBuff.onLeaveScene();
  m_pUser->getQuest().onLeaveScene();
  m_pUser->getSpEffect().onLeaveScene();
  m_pUser->m_oBooth.onLeaveScene();

  m_pUser->m_oGingerBread.onLeaveScene();
  m_pUser->onTwinsMove();
  m_pUser->getServant().showServant(false);
  m_pUser->getDressUp().onLeaveScene();
  
  m_pUser->getCheatTag().save();

  Scene* pScene = m_pUser->getScene();
  if (pScene != nullptr)
  {
    m_pUser->getUserSceneData().setLastMapID(pScene->id, pScene->getMapID());
    m_pUser->getSeal().onLeaveScene();
    if (m_pUser->m_blCheckTeamImage)
    {
      pScene->m_oImages.checkTeam(m_pUser);
      m_pUser->m_blCheckTeamImage = false;
    }
    if (pScene->isSScene() && pScene->isNoramlScene())
    {
      m_pUser->getUserMap().setLastStaticMap(pScene->getMapID(), m_pUser->getPos());
    }
    pScene->onLeaveScene(m_pUser);
  }

  if (m_pUser->hasChatRoom())
  {
    ChatRoomManager::getMe().exitRoom(m_pUser, m_pUser->getChatRoomID());
  }

  //ChatManager_SC::getMe().onLeaveScene(m_pUser);

  /*if (m_pUser && m_pUser->m_oSkillProcessor.getRunner().getCFG() && m_pUser->m_oSkillProcessor.getRunner().getCFG()->getSkillType() == ESKILLTYPE_REPAIR)
  {
    m_pUser->m_oSkillProcessor.breakSkill();
  }*/

  // 离开地图设置hide npc对队友不可见
  const set<DWORD>& setShowNpc = m_pUser->getShowNpcs();
  if (pScene != nullptr && m_pUser->getTeamID() != 0 && !setShowNpc.empty())
  {
    std::set<SceneUser*> userSet;
    const GTeam& rTeam = m_pUser->getTeam();
    for (auto &m : rTeam.getTeamMemberList())
    {
      if (m_pUser->id == m.second.charid())
        continue;
      SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
      if (user && user->getScene() == m_pUser->getScene())
        userSet.insert(user);
    }

    if (!userSet.empty())
    {
      std::list<SceneNpc *> npclist;
      for (auto &d : setShowNpc)
      {
        pScene->getSceneNpcByBaseID(d, npclist);
      }
      auto findNpc = [&](DWORD npcBaseid) ->bool {
        for (auto &s : userSet)
        {
          if (s->getUserSceneData().getShowNpcs().find(npcBaseid) != s->getUserSceneData().getShowNpcs().end())
            return true;
        }
        return false;
      };
      for (auto &iter : npclist)
      {
        if (findNpc(iter->getNpcID()))
          continue;
        for (auto &s : userSet)
        {
          if (iter->checkNineScreenShow(s))
            iter->delMeToUser(s);
        }
      }
    }
  }

  m_pUser->updateData(UnregType::Null);

  m_pUser->getItemMusic().leaveScene();

  if (pScene && pScene->getMapID() == 2)
  {
    DWORD nanMenTime = 0;
    DWORD curSec = now();
    if (curSec > m_pUser->m_dwEnterNanMenTime)
    {
      nanMenTime = curSec - m_pUser->m_dwEnterNanMenTime;
    }
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_NANMEN_TIME, 0, 0, m_pUser->getLevel(), nanMenTime);
  }
  if (m_pUser->getTeamSeal())
  {
    m_pUser->getTeamSeal()->onLeaveScene(m_pUser);
  }
  m_pUser->getPackage().setStoreStatus(false);

  m_pUser->seatUp();
  m_pUser->getSceneFood().breakCooking();
  m_pUser->getSceneFood().breakEating();

  AuguryMgr::getMe().quit(m_pUser);
  ActivityManager::getMe().onLeaveScene(m_pUser);

  std::set<SceneUser*> teammate = m_pUser->getTeamSceneUser();
  for (auto &u : teammate) {
    u->m_oBuff.onTeamChange();
  }

  if (pScene && pScene->isPVPScene() && m_pUser->getFighter())
  {
    const SPvpCommonCFG& rCFG = MiscConfig::getMe().getPvpCommonCFG();
    DWORD hp = m_pUser->getAttr(EATTRTYPE_HP);
    if (rCFG.dwHpRate)
      m_pUser->getFighter()->setHp(hp / rCFG.dwHpRate);
  }
  if (pScene && pScene->isGvg())
  {
    if (m_pUser->getFighter())
    {
      DWORD hp = m_pUser->getAttr(EATTRTYPE_HP);
      const SGuildFireCFG& cfg = MiscConfig::getMe().getGuildFireCFG();
      DWORD rate = cfg.dwHpRate ? cfg.dwHpRate : 1;
      m_pUser->getFighter()->setHp(hp / rate);
    }
  }

  m_pUser->getTutorTask().onLeaveScene();
}

void UserEvent::onMove(float dis)
{
  if (!m_pUser) return;
  /*if (m_pUser->m_oSkillProcessor.getRunner().getCFG() && m_pUser->m_oSkillProcessor.getRunner().getCFG()->getSkillType() == ESKILLTYPE_REPAIR)
  {
    if (m_pUser->m_oSkillProcessor.getRunner().getStartTime() < xTime::getCurMSec())
      m_pUser->m_oSkillProcessor.breakSkill(9999999, false, 0);
  }*/
  m_pUser->setAction(0);
  m_pUser->m_oHands.move(m_pUser->getPos());
  m_pUser->getWeaponPet().onUserMoveTo(m_pUser->getPos());
  m_pUser->getUserPet().onUserMoveTo(m_pUser->getPos());
  m_pUser->getUserBeing().onUserMoveTo(m_pUser->getPos());
  m_pUser->getServant().onUserMoveTo(m_pUser->getPos());
  if (m_pUser->getScene())
  {
    if (m_pUser->getScene()->isDScene())
    {
      DScene* pDScene = dynamic_cast<DScene*>(m_pUser->getScene());
      if (pDScene != nullptr)
        pDScene->getFuben().check("move");
    }
    m_pUser->getScene()->m_oImages.check(m_pUser);
  }

  if (m_pUser->getItemMusic().hasMusicItem())
    m_pUser->getItemMusic().checkMusicItem(0);

  m_pUser->getShare().addNormalData(ESHAREDATATYPE_S_MOVEDIS, (QWORD)dis);
  m_pUser->getAchieve().onPetHand(dis);
}

void UserEvent::onItemAdd(const ItemInfo& rInfo)
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return;

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(rInfo.id());
  if (pCFG == nullptr)
    return;

  m_pUser->getManual().onItemAdd(rInfo.id(), true, true, false, rInfo.source());
  m_pUser->getServant().onAppearEvent(ETRIGGER_ITEM_GET);
  m_pUser->getServant().onFinishEvent(ETRIGGER_ITEM_GET);
  m_pUser->getServant().onGrowthAppearEvent(ETRIGGER_ITEM_GET);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_ITEM_GET);
  if (pCFG->eItemType == EITEMTYPE_QUESTITEMCOUNT && m_pUser->getTeamID() != 0)
  {
    const GTeam& rTeam = m_pUser->getTeam();
    for (auto &m : rTeam.getTeamMemberList())
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(m.second.charid());
      if (pUser == nullptr || pUser->getScene() == nullptr || pUser->getScene()->id != m_pUser->getScene()->id)
        continue;

      pUser->getQuest().onItemAdd(rInfo);
    }
  }
  else
  {
    m_pUser->getQuest().onItemAdd(rInfo);
  }
  if (pCFG->eItemType == EITEMTYPE_EGG)
    m_pUser->getQuest().onPetAdd();

  if (ItemConfig::getMe().isRealAdd(rInfo.source()) == true)
  {
    if (pCFG->eItemType == EITEMTYPE_QUEST_ONCE)
      m_pUser->getPackage().saveOnce(rInfo.id());

    m_pUser->getShortcut().onItemAdd(rInfo);
    //if (rInfo.source() == ESOURCE_PICKUP)
    m_pUser->getAchieve().onItemAdd(rInfo);
    m_pUser->getExchangeShop().onItemAdd(rInfo);
    if (rInfo.id() == 5261)
    {
      //金质勋章
      m_pUser->getExchangeShop().onAddMedalCount();
    }
  }

  static TSetDWORD s_ItemId = { 1002, 1003, 1004,1005,1006 };
  if (s_ItemId.find(rInfo.id()) != s_ItemId.end())
    m_pUser->stopSendInactiveLog();

  if (rInfo.source() == ESOURCE_SHOP)
  {
    if (rInfo.id() == 5039)
      m_pUser->stopSendInactiveLog();
  }

  if (pCFG->eItemType == EITEMTYPE_HONOR)
    m_pUser->getPetWork().refreshWorkSpace();
}

void UserEvent::onItemAdd(const string& guid, DWORD itemID, ESource source)
{
  if (m_pUser == nullptr)
    return;

  m_pUser->getManual().onItemAdd(itemID, true, true);

  if (ItemConfig::getMe().isRealAdd(source) == false)
    return;
  m_pUser->getQuest().onItemAdd(guid);
}

void UserEvent::onPassRaid(DWORD raidid, bool bSuccess)
{
  if (m_pUser == nullptr)
    return;

  bool bQuestImageSceneNotClose = false;
  DScene* pDScene = dynamic_cast<DScene*> (m_pUser->getScene());
  if (pDScene && pDScene->m_oImages.isImageScene() && m_pUser->getTeamID())
  {
    std::set<SceneUser*> userset = m_pUser->getTeamSceneUser();
    userset.erase(m_pUser);
    if (userset.empty())
    {
      QuestRaidCloseSessionCmd cmd;
      cmd.set_raidid(pDScene->getRaidID());
      GTeam& myteam = m_pUser->getTeam();
      for (auto &m : myteam.getTeamMemberList())
      {
        if (m.first == m_pUser->id)
          continue;
        cmd.set_userid(m.first);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
      }
    }
    else
    {
      bQuestImageSceneNotClose = true;
    }
  }
  bool noNeedRunQuest = !bSuccess && bQuestImageSceneNotClose; // 镜像地图副本未关闭时, 退出副本不跳转任务

  if (!noNeedRunQuest)
    m_pUser->getQuest().onPassRaid(raidid, bSuccess);

  if(raidid == 60123)
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_FLAME_RAID);
}

void UserEvent::onBaseLevelup(DWORD newLv, DWORD oldLv)
{
  if (m_pUser == nullptr)
    return;

  m_pUser->getMenu().refreshNewMenu(EMENUCOND_BASE_LEVEL);
  m_pUser->getQuest().onLevelup(newLv, m_pUser->getJobLv());
  m_pUser->getQuest().acceptNewQuest();
  m_pUser->playDynamicExpression(EAVATAREXPRESSION_SMILE);
  m_pUser->getUserSceneData().setLevelUpTime(now());
  m_pUser->getWeaponPet().onUserLevelUp();
  m_pUser->getAchieve().onLevelup();
  m_pUser->getUserPet().playAction(EPETACTION_OWNER_BASELVUP);
  m_pUser->getUserSceneData().updateMaxBaseLv(oldLv);
  m_pUser->getSceneShop().onLevelup(oldLv);
  m_pUser->getTutorTask().onLevelUp();
  m_pUser->getPackage().refreshSlot();
  m_pUser->getServant().onAppearEvent(ETRIGGER_LEVELUP);
  m_pUser->getExchangeShop().onBaseLevelUp();
  m_pUser->checkWorldLevelBuff();
  m_pUser->getServant().onGrowthAppearEvent(ETRIGGER_LEVELUP);
  m_pUser->getServant().checkNewGrowthGroup();

  if (m_pUser->getUserSceneData().getOriginalZoneID() == 0)
    SceneUserManager::getMe().setSvrMaxBaseLv(m_pUser);
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);;
}

void UserEvent::onJobLevelup(DWORD oldLv, DWORD newLv)
{
  if (m_pUser == nullptr)
    return;

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr)
  {
    pFighter->getSkill().refreshEnableSkill();
    pFighter->checkUnlock();

    const SRoleBaseCFG* pFighterCFG = pFighter->getRoleCFG();
    if(pFighterCFG != nullptr && newLv >= pFighterCFG->maxJobLv && oldLv < pFighterCFG->maxJobLv)
      m_pUser->getTip().addRedTip(EREDSYS_PROFESSION_UP);
  }

  m_pUser->getQuest().onLevelup(m_pUser->getUserSceneData().getRolelv(), newLv);
  m_pUser->getQuest().acceptNewQuest();
  m_pUser->getUserRecords().onJobLvChange(newLv);
  m_pUser->playDynamicExpression(EAVATAREXPRESSION_SMILE);
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_JOB_LEVEL);

  if (newLv > m_pUser->getMaxCurJobLv())
  {
    m_pUser->setMaxCurJobLv(newLv);
    m_pUser->getExchangeShop().onJobLevelUp();
    m_pUser->checkWorldLevelBuff();
  }
}

void UserEvent::onQuestSubmit(DWORD questid)
{
  if (m_pUser == nullptr)
    return;

  if(questid == QUEST_PEAK_EFFECT_ID)
  {
    m_pUser->setDataMark(EUSERDATATYPE_PEAK_EFFECT);
    m_pUser->refreshDataAtonce();
  }
  // patch
  PatchManager::getMe().submitPatch(m_pUser, questid);
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_QUEST);
  m_pUser->getQuest().onQuestSubmit();
  m_pUser->getQuest().acceptNewQuest();
  m_pUser->getSeal().onQuestSubmit(questid);
  m_pUser->getAchieve().onQuestSubmit(questid);
  m_pUser->getTutorTask().onQuestSubmit(questid);
  m_pUser->getGuildChallenge().onQuestSubmit(questid);
  m_pUser->getServant().onAppearEvent(ETRIGGER_QUEST_SUBMIT);
  m_pUser->getServant().onFinishEvent(ETRIGGER_QUEST_SUBMIT);
  m_pUser->getServant().onGrowthAppearEvent(ETRIGGER_QUEST_SUBMIT);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_QUEST_SUBMIT);

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr)
    pFighter->getSkill().refreshEnableSkill();

  const SQuestCFGEx* pCfg = QuestManager::getMe().getQuestCFG(questid);
  if(pCfg && pCfg->eType == EQUESTTYPE_GUILD)
  {
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_QUEST_SUBMIT);
  }
}

void UserEvent::onEquipChange(EEquipOper oper,const string& guid)
{
  ItemBase* pEquip = m_pUser->getPackage().getItem(guid);
  if (pEquip == nullptr)
    return;
  m_pUser->m_oBuff.onEquipChange(pEquip, oper);
  m_pUser->getPackage().refreshEnableBuffs();
  //m_pUser->getManual().onItemAdd(pEquip->getTypeID(), false, true, false, false, true);
  if (pEquip->getType() == EITEMTYPE_WEAPON_BOW)
  {
    DWORD arrow = m_pUser->getPackage().getArrowTypeID();
    BasePackage* pPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (arrow != 0 && pPack != nullptr)
    {
      const string& guid = pPack->getGUIDByType(arrow);
      if (!guid.empty())
        pPack->setUpdateIDs(guid);
    }
  }

  /*
  // remove enchant effect
  if (oper == EEQUIPOPER_OFF)
  {
    ItemEquip* pRealEquip = dynamic_cast<ItemEquip*>(pEquip);
    if (pRealEquip != nullptr)
    {
      EnchantData& rData = pRealEquip->getEnchantData();
      for (int i = 0; i < rData.extras_size(); ++i)
        m_pUser->m_oBuff.del(rData.extras(i).buffid());
    }
  }
  */
}

void UserEvent::onEquipExchange(const string& guidOld, const string& guidNew)
{
  if(!m_pUser)
    return;

  m_pUser->m_oProfession.onEquipExchange(guidOld, guidNew);
  m_pUser->getUserRecords().onEquipExchange(guidOld, guidNew);
}

void UserEvent::onCardChange(DWORD dwTypeID, bool isAdd)
{
  if (!m_pUser)
    return;
  m_pUser->m_oBuff.onCardChange(dwTypeID, isAdd);
}

void UserEvent::onProfesChange(EProfession oldProfes)
{
  if (m_pUser == nullptr)
    return;

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr)
  {
    pFighter->checkUnlock();

    //if (m_pUser->getFighter(oldProfes) != nullptr)
    //    pFighter->getSkill().addEquipSkill(m_pUser->getFighter(oldProfes)->getSkill());
  }

  m_pUser->m_oBuff.onProfesChange(oldProfes);
  m_pUser->getPracticeReward();

  if (m_pUser->getArrowID())
  {
    int msgid = LuaManager::getMe().call<int>("itemUserCheck", (xSceneEntryDynamic*)(m_pUser), m_pUser->getArrowID());
    if (msgid != 0)
    {
      m_pUser->getPackage().changeArrow(m_pUser->getArrowID());
    }
  }

  m_pUser->getAstrolabes().onProfesChange(oldProfes);

  m_pUser->getUserBeing().onProfesChange();

  m_pUser->getUserElementElf().onProfesChange();

  m_pUser->getMenu().refreshNewMenu(EMENUCOND_EVO);

  m_pUser->getUserRecords().onProfesChange();

  m_pUser->getMenu().refreshNewMenu(EMENUCOND_EVO);

  m_pUser->getAchieve().onProfession();

  m_pUser->getUserSceneData().setMaxPro();
  
  if (m_pUser->getJobLv() > m_pUser->getMaxCurJobLv())
  {
    m_pUser->setMaxCurJobLv(m_pUser->getJobLv());
    m_pUser->getExchangeShop().onJobLevelUp();
    m_pUser->checkWorldLevelBuff();
  }

  m_pUser->getQuest().setRefresh(true);

  PlatLogManager::getMe().changeFlagLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(), 
    m_pUser->id, 
    EChangeFlag_Profession, oldProfes, m_pUser->getUserSceneData().getProfession());

}

void UserEvent::onMountChange()
{
  if (m_pUser == nullptr)
    return;
  m_pUser->m_oBuff.onMountChange();
}

void UserEvent::onMenuOpen(DWORD id)
{
  if (m_pUser == nullptr)
    return;

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr)
    pFighter->getSkill().refreshEnableSkill();
  m_pUser->getPetWork().refreshWorkSpace();
  m_pUser->getServant().onAppearEvent(ETRIGGER_MENU);
  m_pUser->getServant().onFinishEvent(ETRIGGER_MENU);
  m_pUser->getServant().onGrowthAppearEvent(ETRIGGER_MENU);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_MENU);
}

void UserEvent::onRolePointChange()
{
  if (nullptr == m_pUser)
    return;

  m_pUser->getTip().onRolePoint();
}

void UserEvent::onSkillPointChange()
{
  if (nullptr == m_pUser)
    return;

  m_pUser->getTip().onSkillPoint();
  m_pUser->setDataMark(EUSERDATATYPE_BATTLEPOINT);
}

void UserEvent::onAction(EUserActionType eType, QWORD id)
{
  if (m_pUser == nullptr)
    return;

  m_pUser->clearHitMe();
  m_pUser->getQuest().onAction(eType, id);
  if (eType == EUSERACTIONTYPE_MOTION)
  {
    if (m_pUser->getStatus() == ECREATURESTATUS_FAKEDEAD)
    {
      m_pUser->setStatus(ECREATURESTATUS_LIVE);
      m_pUser->refreshDataAtonce();
    }
    if (id != m_pUser->getAction())
    {
      m_pUser->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
      m_pUser->refreshDataAtonce();
      m_pUser->setAction(id);
      m_pUser->getWeaponPet().onUserAction(id);
    }
    /*
    0615 周鑫, 部分动作不下坐骑, 移至前端判断
    EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
    if (pEquipPack)
    {
      ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_MOUNT);
      if (pEquip && pEquip->getType() != EITEMTYPE_BARROW)
        m_pUser->getPackage().equipOff(EEQUIPPOS_MOUNT);
    }
    */
    m_pUser->m_oMove.forceSync();
  }
  else if (eType == EUSERACTIONTYPE_EXPRESSION)
  {
    m_pUser->setExpression(id);
    m_pUser->m_oBuff.onEmoji(id);
    m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_EXPRESSION, id, 1);
    m_pUser->getAchieve().onExpressUse();

    if (id == STATIC_EXPRESSION_LAUGH ||
        id == STATIC_EXPRESSION_HEART ||
        id == STATIC_EXPRESSION_KISS)
    {
      m_pUser->playDynamicExpression(EAVATAREXPRESSION_SMILE);
    }
  }
}

void UserEvent::onAttrChange()
{
  if (m_pUser == nullptr)
    return;
  m_pUser->m_oBuff.onAttrChange();
  m_pUser->getAchieve().onAttrChange();
  m_pUser->getPackage().refreshMaxSlot();
  m_pUser->getUserBeing().onUserAttrChange();

  std::list<SceneNpc *> npclist;
  m_pUser->m_oFollower.get(npclist);
  for (auto &it : npclist)
  {
    if (!it)
      continue;

    it->updateAttribute();
    it->refreshDataAtonce();
  }
}

void UserEvent::onRelive(EReliveType eType)
{
  if (nullptr == m_pUser)
    return;

  m_pUser->m_oBooth.onRelive(eType);

  switch (eType)
  {
    case ERELIVETYPE_RETURN:
      {
        ChatRoom *pRoom = m_pUser->getChatRoom();
        if (nullptr != pRoom)
          ChatRoomManager::getMe().exitRoom(m_pUser, pRoom->getRoomID());
        break;
      }
    default:
      break;
  }
  TSetQWORD& reliveMeUsers = m_pUser->getReliveMeUsers();
  TSetQWORD tempset;
  tempset.insert(reliveMeUsers.begin(), reliveMeUsers.end());
  for (auto &q : tempset)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(q);
    if (!pUser)
      continue;
    // break 复活术
    if (pUser->m_oSkillProcessor.getRunner().getCFG() && pUser->m_oSkillProcessor.getRunner().getCFG()->getSkillType() == ESKILLTYPE_REBIRTH)
      pUser->m_oSkillProcessor.breakSkill(pUser->id);
  }
  reliveMeUsers.clear();
  m_pUser->m_oBuff.onRelive();
  m_pUser->playDynamicExpression(EAVATAREXPRESSION_MIN);
  if (eType == ERELIVETYPE_SKILL)
    m_pUser->playDynamicExpression(EAVATAREXPRESSION_SMILE);
  m_pUser->getWeaponPet().onUserRelive();
}

void UserEvent::onCameraChange()
{
  DWORD cur = now();
  if (cur < m_dwCameraTimetick)
    return;

  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return;
  const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
  float radius = rCFG.dwGoCamera.dwRangeFind;
  DWORD spdBuff = rCFG.dwGoCamera.dwBuff;

  xSceneEntrySet uSet;
  m_pUser->getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, m_pUser->getPos(), radius, uSet);
  for (auto &s : uSet)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
    if (npc == nullptr)
      continue;

    if (!npc->m_ai.isGocamera())
      continue;
    if (npc->m_ai.getStateType() == ENPCSTATE_CAMERA)
      continue;
    if (cur < npc->m_ai.getNextGocameraTime())
      continue;
    npc->m_oMove.stop();
    npc->m_oBuff.add(spdBuff);
    npc->m_ai.changeState(ENPCSTATE_CAMERA);
    npc->m_ai.setCurLockID(m_pUser->id);
  }

  // do
  m_dwCameraTimetick = cur + 2;
}

void UserEvent::onFocusNpc(const TVecQWORD& vecIDs)
{
  DWORD range = FeatureConfig::getMe().getNpcAICFG().dwGoCamera.dwRangeFind;
  for (auto &v : vecIDs)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v);
    if (npc == nullptr || npc->getScene() == nullptr)
      continue;
    // 检查距离合法性
    if (getDistance(npc->getPos(), m_pUser->getPos()) > range + 3)
      continue;
    npc->m_ai.onFocusByCamera(m_pUser->id);
    npc->m_sai.checkSig("camera_lock");
  }
}

void UserEvent::onBeBreakSkill(QWORD attackerid)
{
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(attackerid);
  if (npc != nullptr)
  {
    npc->m_ai.onBreakUserSkill(m_pUser->id);
  }
}

void UserEvent::onStatusChange()
{
  if (m_pUser->getStatus() == ECREATURESTATUS_DEAD || m_pUser->getStatus() == ECREATURESTATUS_FAKEDEAD)
  {
    const SNpcAICFG& rCFG = FeatureConfig::getMe().getNpcAICFG();
    float alertRange = rCFG.dwAlert.dwRange;
    float expelRange = rCFG.dwExpel.dwRange;
    float range = alertRange > expelRange ? alertRange : expelRange;

    xSceneEntrySet uSet;
    if (m_pUser->getScene() == nullptr)
      return;
    m_pUser->getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, m_pUser->getPos(), range, uSet);

    for (auto &s : uSet)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (npc == nullptr)
        continue;
      npc->m_ai.onSeeDead(m_pUser->id);
    }

    BossMgr::getMe().onStatus(m_pUser);
  }

  if (m_pUser->getStatus() == ECREATURESTATUS_DEAD)
    onUserDie();
}

void UserEvent::onOpenCamera()
{
  m_pUser->getUserCamera().onCamera();
}

void UserEvent::onVisitNpc(SceneNpc *npc)
{
  if (!npc || !npc->getScene()) return;
  if (!m_pUser->getScene() || m_pUser->getScene() != npc->getScene()) return;
  if (!m_pUser->getScene()->check2PosInNine(m_pUser->getPos(), npc->getPos())) return;

  npc->setLastVisitor(m_pUser->id);

  m_pUser->getManual().onKillMonster(npc->define.getID());
  m_pUser->m_oSkillProcessor.triggerTrap(npc->id);

  if(npc->getNpcID() == 2303)   //统计招财猫
  {
    if (m_pUser->m_oUserStat.checkAndSet(ESTATTYPE_VISIT_CAT_USER_COUNT))
      m_pUser->m_oUserStat.sendStatLog(ESTATTYPE_VISIT_CAT_USER_COUNT, 0, 0, 0, 1);

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_VISIT_CAT_COUNT, 0, 0, 0, (DWORD)1);
  }
  
  //激活被访问ai
  npc->m_sai.checkSig("visit");

  //if (Freyja::isFreyja(npc))
  //  m_pUser->m_oFreyja.addFreyja(npc->getScene()->getRealMapID(), npc->define.getUniqueID());

  npc->m_sai.checkSig("visit");
}

void UserEvent::onEquipStrength(const string& guid)
{
  if (m_pUser == nullptr)
    return;

  m_pUser->getQuest().onItemAdd(guid);
}

void UserEvent::onUserDie()
{ 

  // scene 
  Scene* pScene = m_pUser->getScene();
  if (pScene != nullptr)
  {
    pScene->onUserDie(m_pUser, nullptr);
  }

  m_pUser->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_DEAD, m_pUser->id);
  m_pUser->m_oMove.stop();
  m_pUser->getWeaponPet().onUserDie();
  m_pUser->m_oSkillStatus.onDie();
  m_pUser->getUserPet().onUserDie();
  m_pUser->getUserElementElf().onUserDie();
}

void UserEvent::onItemDelte(const ItemBase* pItem)
{
  if (pItem == nullptr)
    return;
  if (pItem->getType() == EITEMTYPE_ARROW && m_pUser->getArrowID() == pItem->getTypeID())
  {
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack && pMainPack->getItemCount(pItem->getTypeID()) == 0)
      m_pUser->getPackage().changeArrow(0);
  }
}

void UserEvent::onEnterGuild(DWORD qwGuildID, bool bCreate)
{
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_INGUILD);
  m_pUser->getAchieve().onEnterGuild(bCreate);
  m_pUser->getPetWork().refreshWorkSpace();
  m_pUser->getServant().onAppearEvent(ETRIGGER_JOIN_GUILD);
  m_pUser->getServant().onFinishEvent(ETRIGGER_JOIN_GUILD);
  m_pUser->getServant().onGrowthAppearEvent(ETRIGGER_JOIN_GUILD);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_JOIN_GUILD);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_ARTIFACT);
}

void UserEvent::onPlayMusic(DWORD dwMusciId)
{
  if (m_pUser == NULL)
    return;
  m_pUser->getQuest().onPlayMusic(dwMusciId);
}

void UserEvent::onAddSkill(DWORD skillId)
{
  if (MiscConfig::getMe().getSkillCFG().pairSkillAction.first == skillId)
  {
    m_pUser->getUserSceneData().addAction(MiscConfig::getMe().getSkillCFG().pairSkillAction.second);
  }
  m_pUser->getPhoto().onAddSkill(skillId);
  m_pUser->getPackage().onAddSkill(skillId);
}

void UserEvent::onDelSkill(DWORD skillId)
{
  if (MiscConfig::getMe().getSkillCFG().pairSkillAction.first == skillId)
  {
    m_pUser->getUserSceneData().delAction(MiscConfig::getMe().getSkillCFG().pairSkillAction.second);
  }
  m_pUser->getPhoto().onDelSkill(skillId);
  const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(skillId); 
  if (pSkill)
    pSkill->onReset(m_pUser);
}

void UserEvent::onTrigNpcFunction(QWORD npcguid, DWORD funcid)
{
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(npcguid);
  if (npc == nullptr)
    return;
  if (getDistance(m_pUser->getPos(), npc->getPos()) > 10)
    return;
  npc->m_sai.setFunctionID(funcid);
  npc->m_sai.checkSig("npc_function");
  npc->m_sai.setFunctionID(0);
}

void UserEvent::onUnlockRuneSpecial(DWORD specID)
{
  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(specID);
  if (pRuneCFG == nullptr)
    return;
  if (pRuneCFG->eType != ERUNESPECTYPE_SELECT)
  {
    for (auto &s : pRuneCFG->setBuffIDs)
      m_pUser->m_oBuff.add(s);
  }
  else
  {
    if (pRuneCFG->setSkillIDs.empty() == false)
    {
      DWORD skillid = *(pRuneCFG->setSkillIDs.begin());
      if (m_pUser->getFighter() && m_pUser->getFighter()->getSkill().getRuneSelectID(skillid) == 0)
        m_pUser->getFighter()->getSkill().selectRuneSpecID(skillid, specID);
    }
  }

  if (pRuneCFG->dwBeingSkillPoint > 0)
    m_pUser->getUserBeing().addSkillPoint(pRuneCFG->dwBeingSkillPoint);
  if (pRuneCFG->mapBeingBuff.empty() == false)
    for (auto& v : pRuneCFG->mapBeingBuff)
      m_pUser->getUserBeing().addBuff(v.first, v.second);
}

void UserEvent::onUnlockRune(DWORD id, bool bSpecial)
{
  m_pUser->getAchieve().onRune();
  if(bSpecial)
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_LUN_SHARD_SPECIAL);
  else
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_LUN_SHARD);
}

void UserEvent::onResetRuneSpecial(DWORD specID)
{
  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(specID);
  if (pRuneCFG == nullptr)
    return;
  if (pRuneCFG->eType != ERUNESPECTYPE_SELECT)
  {
    for (auto &s : pRuneCFG->setBuffIDs)
      m_pUser->m_oBuff.del(s);
  }
  else
  {
    if (m_pUser->getAstrolabes().isEffectUnlock(specID) == false)
    {
      if (m_pUser->getFighter())
        m_pUser->getFighter()->getSkill().onRuneReset(specID);
    }
    /*
    // if skill select this specID, reset it
    if (pRuneCFG->setSkillIDs.empty() == false)
    {
      DWORD skillid = *(pRuneCFG->setSkillIDs.begin());
      if (m_pUser->getFighter() && m_pUser->getFighter()->getSkill().isRuneSpecSelected(specID))
        m_pUser->getFighter()->getSkill().selectRuneSpecID(skillid, 0);
    }
    */
  }

  m_pUser->getUserBeing().onRuneReset(specID);
}

void UserEvent::onItemUsed(DWORD itemid, DWORD count)
{
  m_pUser->getAchieve().onItemUsed(itemid);
  m_pUser->getTutorTask().onItemUsed(itemid);

  ItemInfo oItem;
  oItem.set_id(itemid);
  oItem.set_count(count);
  StatMgr::getMe().onItemUse(m_pUser, oItem);

  //打开礼包 10,000,000 Zeny  也算作充值获得的zeny里面
  if (itemid == 3841)
  {
    m_pUser->getUserSceneData().setDailyChargeZeny(m_pUser->getUserSceneData().getDailyChargeZeny() + 10000000 * count);
  }
}

void UserEvent::onItemBeUsed(DWORD itemid)
{
  m_pUser->getAchieve().onItemBeUsed(itemid);
  m_pUser->getTutorTask().onItemBeUsed(itemid);
}

void UserEvent::onSkillLevelUp(DWORD skillid)
{
  m_pUser->getUserBeing().onUserSkillLevelUp(skillid);
}

void UserEvent::onSkillReset()
{
  m_pUser->getUserBeing().onUserSkillReset();
  m_pUser->m_oSkillStatus.onSkillReset();
}

void UserEvent::onEnterGVG()
{
  m_pUser->getGuildChallenge().onEnterGVG();
}

void UserEvent::onPetHatch()
{
  m_pUser->getPetWork().refreshWorkSpace();
}

void UserEvent::onReceiveEventMail(const UserEventMailCmd& event)
{
  switch (event.etype())
  {
    case EEVENTMAILTYPE_DELCAHR:
      {
        if (event.param64_size() == 0)
          break;
        QWORD delcharid = event.param64(0);
        SceneFighter* pNoviceFighter = m_pUser->getFighter(EPROFESSION_NOVICE);
        if (pNoviceFighter)
          pNoviceFighter->getSkill().onDeleteChar(delcharid);

        m_pUser->getUserRecords().onDelChar(delcharid);
        XLOG << "[事件-邮件], 删除角色事件处理成功, 玩家:" << m_pUser->name << m_pUser->id << "删除的角色id:" << delcharid << XEND;
      }
      break;
    case EEVENTMAILTYPE_MIN:
    default:
      break;
  }
}

void UserEvent::onPassTower()
{
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_TOWERLAYER);
}

void UserEvent::onManualLevelup()
{
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_MANUALLEVEL);
}

void UserEvent::onMoneyChange(EMoneyType eType)
{
  if (eType == EMONEYTYPE_PVPCOIN)
    m_pUser->getMenu().refreshNewMenu(EMENUCOND_PVPCOIN);
}

void UserEvent::onChangeGender()
{
  if(!m_pUser)
    return;

  m_pUser->m_oBuff.onChangeGender();
  m_pUser->setCollectMark(ECOLLECTTYPE_PROFESSION);
  m_pUser->getQuest().setRefresh(true);
}

void UserEvent::onProfessionChange(Cmd::EProfession eProfessionOld)
{
  if(!m_pUser)
    return;

  m_pUser->getQuest().setRefresh(true);
  m_pUser->m_oBuff.onProfesChange(eProfessionOld);
  m_pUser->getUserElementElf().onProfesChange();
  if (m_pUser->getArrowID())
  {
    int msgid = LuaManager::getMe().call<int>("itemUserCheck", (xSceneEntryDynamic*)(m_pUser), m_pUser->getArrowID());
    if (msgid != 0)
    {
      m_pUser->getPackage().changeArrow(m_pUser->getArrowID());
    }
  }
}

void UserEvent::onProfessionAdd()
{
  Menu& rMenu = m_pUser->getMenu();
  rMenu.refreshNewMenu(EMENUCOND_PROFESSION);
}

void UserEvent::onLoadRecord()
{
  if(!m_pUser)
    return;

  m_pUser->getQuest().setRefresh(true);
  m_pUser->setDataMark(EUSERDATATYPE_CUR_MAXJOB);
}

void UserEvent::onAddBattleTimeInBattle(DWORD dwTime)
{
  // 只在有效战斗时长内给导师加战斗时长
  if (m_pUser == nullptr || !m_pUser->getUserSceneData().haveEnoughBattleTime())
    return;

  GSocial& rSocial = m_pUser->getSocial();
  QWORD qwTutorID = rSocial.getTutorCharID();
  if (qwTutorID != 0)
  {
    SceneUser* pTutor = SceneUserManager::getMe().getUserByID(qwTutorID);
    if (pTutor != nullptr && pTutor->hasMonthCard() == true && pTutor->check2PosInNine(m_pUser) == true)
    {
      const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();
      
      UserSceneData& rData = pTutor->getUserSceneData();
      DWORD dwValue = rData.getTutorBattleTime();
      if (dwValue < rCFG.dwTutorExtraMaxBattleTime)
      {
        DWORD dwAdd = dwTime * rCFG.fTutorExtraBattleTimePer;

        // 加上消耗战斗时长的加成
        DWORD ratio = m_pUser->m_oBuff.getMultiTimeRate();
        dwAdd *= (ratio + 1);

        bool bEnough = rData.haveEnoughBattleTime();
        rData.setTutorBattleTime(dwValue + dwAdd);
        if (!bEnough)
          pTutor->m_oBuff.onBattleStatusChange();
        XDBG << "[导师优化-额外时长] 导师" << pTutor->accid << pTutor->id << pTutor->getProfession() << pTutor->name
          << "收到学生" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "额外时长" << dwAdd << "秒" << XEND;
      }
    }
  }
}

void UserEvent::onBoothOpen()
{
  if(!m_pUser)
    return;

  // 关闭聊天室
  if (m_pUser->hasChatRoom())
    ChatRoomManager::getMe().exitRoom(m_pUser, m_pUser->getChatRoomID());

  // 断开牵手
  if(m_pUser->m_oHands.has())
    m_pUser->m_oHands.breakup();
}

void UserEvent::onWeddingUpdate()
{
  UserWedding& rWedding = m_pUser->getUserWedding();
  Package& rPackage = m_pUser->getPackage();

  Cmd::WeddingInfo& rInfo = rWedding.getWeddingInfo();
  if (rInfo.id() != 0)
  {
    BasePackage* pMainPack = rPackage.getPackage(EPACKTYPE_MAIN);
    if (pMainPack != nullptr)
    {
      const TSetItemBase& setItems = pMainPack->getItemBaseList(ITEM_WEDDING_CERT);
      for (auto &s : setItems)
      {
        ItemWedding* pWeddingItem = dynamic_cast<ItemWedding*>(s);
        if (pWeddingItem != nullptr)
        {
          bool bUpdate = false;
          const WeddingData& rData = pWeddingItem->getWeddingData();
          if (rData.id() != rInfo.id())
          {
            pWeddingItem->setID(rInfo.id());
            bUpdate = true;
          }
          if (rData.myname() != rInfo.manual().name1())
          {
            pWeddingItem->setMyName(rInfo.manual().name1());
            bUpdate = true;
          }
          if (rData.partnername() != rInfo.manual().name2())
          {
            pWeddingItem->setPartnerName(rInfo.manual().name2());
            bUpdate = true;
          }

          if (bUpdate)
            pMainPack->setUpdateIDs(pWeddingItem->getGUID());
        }
      }
    }
  }
}

