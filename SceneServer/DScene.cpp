#include "DScene.h"
#include "SceneNpc.h"
#include "SceneUser.h"
#include "SceneNpcManager.h"
#include "StatisticsDefine.h"
#include "SceneUserManager.h"
#include "GMCommandRuler.h"
#include "MatchCCmd.pb.h"
#include "MatchSCmd.pb.h"
#include "GuildRaidConfig.h"
#include "GuildMusicBoxManager.h"
#include "GuildCityManager.h"
#include "ActivityEventManager.h"
#include "GuildCmd.pb.h"
#include "SceneWeddingMgr.h"
#include "ChatManager_SC.h"
#include "WeddingCCmd.pb.h"
#include "WeddingConfig.h"
#include "PveCardEffect.h"
#include "PveCard.pb.h"
#include "SceneActManager.h"
#include "SceneAct.h"
#include "MailManager.h"
#include "TeamRaidCmd.pb.h"
#include "SysmsgConfig.h"

// quest scene
RaidScene::RaidScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :
  DScene(sID, sName, sBase, pRaidCFG)
{

}

bool RaidScene::init()
{
  return DScene::init();
}

// laboratory scene
#include "MsgManager.h"
#include "SceneServer.h"
#include "PlatLogManager.h"
LaboratoryScene::LaboratoryScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :
  DScene(sID, sName, sBase, pRaidCFG)
{

}

bool LaboratoryScene::init()
{
  return DScene::init();
}

void LaboratoryScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userEnter(user);
  //m_oLaboratory.userEnter(user);
  if (user->getVar().getVarValue(EVARTYPE_LABORATORY) == 0)
    user->getVar().setVarValue(EVARTYPE_LABORATORY, 1);

  if (getRaidID() == 30002)  //研究所战斗区
  {
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_YJS_TRY_COUNT, 0, 0,user->getLevel(), (DWORD)1);
  }
}

void LaboratoryScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userLeave(user);
}

void LaboratoryScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  if (npc == nullptr)
    return;

  DScene::onNpcDie(npc, killer);

  LabNpc* pNpc = dynamic_cast<LabNpc*>(npc);
  if (pNpc && pNpc->getLaboratoryPoint())
  {
    //m_oLaboratory.addPoint(npc->getLaboratoryPoint(), npc->getSurviveTime());
    addPoint(pNpc->getLaboratoryPoint(), npc->getSurviveTime());
    pNpc->setLaboratoryPoint(0);
    m_oFuben.check("laboratory");
  }
}

bool LaboratoryScene::summon(DWORD roundid)
{
  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (entrySet.empty()) return false;
  m_dwRoundNum = roundid;
  std::list<float> levels;
  float maxLevel = 0;
  for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
  {
    SceneUser *pUser = (SceneUser *)(*entryIt);
    float l = pUser->getLevel();
    levels.push_back(l);
    if (l > maxLevel) maxLevel = l;
    //userEnter(pUser);
  }

  float baseLevel = 0.85f * maxLevel;
  float add = 0.0f;
  DWORD entrySize = entrySet.size();
  for (auto &it : levels)
  {
    if (it > baseLevel)
    {
      add += (it-baseLevel)/entrySize;
    }
  }
  DWORD level = baseLevel + add;
  if (level < 15) level = 15;
  m_dwLevel = level;
  for (auto it : Laboratory::s_cfg)
  {
    if (level >= it.second.level.min && level <= it.second.level.max)
    {
      DWORD rand = randBetween(1, it.second.npclist.size());
      auto iter = it.second.npclist.find(rand);
      if (iter != it.second.npclist.end())
      {
        auto &list = iter->second;
        for (auto &npciter : list)
        {
          summon(npciter.m_dwGroupID, npciter.m_dwNpcID, npciter.m_dwPoint);
        }
      }
      break;
    }
  }
  return true;
}

void LaboratoryScene::summon(DWORD groupid, DWORD npcid, DWORD point)
{
  const SceneObject *pObject = getSceneObject();
  if (pObject)
  {
    const SceneNpcTemplate* pRaidNpc = pObject->getRaidNpcTemplate(groupid);
    if (pRaidNpc != nullptr)
    {
      NpcDefine tmpDefine = pRaidNpc->m_oDefine;
      tmpDefine.setID(npcid);
      tmpDefine.setLevel(m_dwLevel);
      tmpDefine.m_oVar.dwLabPoint = point;
      tmpDefine.m_oVar.dwLabRound = m_dwRoundNum;
      for (DWORD i=0; i<pRaidNpc->m_dwNum; ++i)
      {
        SceneNpc *npc = SceneNpcManager::getMe().createNpc(tmpDefine, this);
        if (npc == nullptr)
          XERR << "[Laboratory::summon groupid =" << groupid << "npcid =" << npcid << "point =" << point << "summon error]" << XEND;
      }
    }
  }
}

void LaboratoryScene::addPoint(DWORD point, DWORD surviveTime)
{
  if (!point) return;

  DWORD dec = 0;
  if (surviveTime > 10)
  {
    dec = (surviveTime - 10);// / 5;
  }

  if (dec > 40)
    dec = 40;


  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (entrySet.empty()) return;
  Cmd::LaboratoryUserCmd message;
  message.set_round(m_dwRoundNum);
  for (auto &it : entrySet)
  {
    SceneUser *pUser = dynamic_cast<SceneUser*>(it);
    if (pUser == nullptr)
      continue;
    if (!pUser->isAlive()) continue;
    if (!m_dwLevel) m_dwLevel = 1;

    DWORD add = static_cast<DWORD>(point * (1 + (m_dwLevel - 1) / 10.0f));

    if (dec)
    {
      add = add * (100 - dec) / 100;
    }

    if (!add) continue;

    m_mapPointList[pUser->id] += add;
    DWORD p = m_mapPointList[pUser->id];
    //pUser->getLaboratory().setTodayPoint(p);
    message.set_curscore(p);
    message.set_maxscore(pUser->getLaboratory().getTodayMaxPoint());
    PROTOBUF(message, send, len);
    pUser->sendCmdToMe(send, len);

    MsgManager::sendMsg(pUser->id, 21, MsgParams(add), EMESSAGETYPE_GETEXP);
    XDBG << "[研究所-积分增加]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "增加了积分:" << add << "dec:" << dec << XEND;
  }
}

void LaboratoryScene::finish()
{
  const SLaboratoryCFG& rCFG = MiscConfig::getMe().getLaboratoryCFG();

  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  std::set<SceneUser*> firstUserSet;
  std::set<SceneUser*> notFirstSet;
  std::set<SceneUser*> allUser;
  for (auto &s : entrySet)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(s);
    if (pUser == nullptr)
      continue;

    auto m = m_mapPointList.find(pUser->id);
    if (m == m_mapPointList.end())
      continue;

    DWORD dwTodayMaxPoint = pUser->getLaboratory().getTodayMaxPoint();
    if (dwTodayMaxPoint == 0)
    {
      firstUserSet.insert(pUser);
    }
    else
    {
      notFirstSet.insert(pUser);
    }
    allUser.insert(pUser);
    m_mapCalcPointList[pUser->id] = dwTodayMaxPoint;
    if (m->second <= dwTodayMaxPoint)
      continue;

    DWORD finishcount = pUser->getVar().getVarValue(EVARTYPE_LABORATORY_COUNT) + 1;
    pUser->getVar().setVarValue(EVARTYPE_LABORATORY_COUNT, finishcount);

    TVecItemInfo vecReward;
    DWORD doublereward = pUser->getDoubleReward(EDOUBLEREWARD_LABORATORY, vecReward);
    m_mapRewardTimes[pUser->id] = doublereward;

    // 额外奖励/多倍奖励
    TVecItemInfo extrareward;
    if(doublereward <= 1)
      ActivityEventManager::getMe().getReward(pUser, EAEREWARDMODE_LABORATORY, finishcount, extrareward, doublereward);

    float zenyratio = pUser->m_oBuff.getExtraZenyRatio(EEXTRAREWARD_LABORATORY);
    if (zenyratio)
      XLOG << "[回归-Buff], 研究所, 额外zeny倍率:" << zenyratio << "玩家:" << pUser->name << pUser->id << XEND;
    zenyratio += 1;

    DWORD dwDelta = m->second - dwTodayMaxPoint;
    DWORD dwGarden = static_cast<DWORD>(ceil(1.0f * dwDelta / (rCFG.dwGarden + pUser->getLevel() * rCFG.fGarden)));
    dwGarden *= zenyratio;

    if(doublereward)
      dwGarden = dwGarden * doublereward;
    if (dwGarden != 0)
    {
      ItemData oData;
      oData.mutable_base()->set_id(ITEM_GARDEN);
      oData.mutable_base()->set_count(dwGarden);
      oData.mutable_base()->set_source(ESOURCE_LABORATORY);
      pUser->getPackage().addItem(oData, EPACKMETHOD_NOCHECK);
    }

    DWORD dwRob = static_cast<DWORD>(ceil(1.0f * dwDelta / (rCFG.dwRob + pUser->getLevel() * rCFG.fRob)));
    dwRob *= zenyratio;

    if(doublereward)
      dwRob = dwRob * doublereward;
    pUser->addMoney(EMONEYTYPE_SILVER, dwRob, ESOURCE_LABORATORY);

    pUser->getLaboratory().setTodayPoint(m->second);
    dwTodayMaxPoint = pUser->getLaboratory().getTodayMaxPoint();

    XLOG << "[研究所]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "获得积分:" << m->second << "当天最高得分:" << dwTodayMaxPoint << "baselv:" << pUser->getLevel() << "获得乐园币:" << dwGarden << "获得rob:" << dwRob << "活动翻倍:" << doublereward << XEND;

    pUser->getExtraReward(EEXTRAREWARD_LABORATORY);
    if (pUser->getVar().getVarValue(EVARTYPE_LABORATORY_EXTASKREWARD) == 0)
    {
      pUser->getVar().setVarValue(EVARTYPE_LABORATORY_EXTASKREWARD, 1);
      pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_INSTITUTE, 1);
    }

    if (extrareward.empty() == false)
    {
      XLOG << "[研究所]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "活动额外奖励:";
      for (auto& v : extrareward)
        XLOG << v.id() << v.count();
      XLOG << XEND;
      pUser->getPackage().addItem(extrareward, EPACKMETHOD_AVAILABLE);
    }

    pUser->getTutorTask().onLaboratoryFinish();
    pUser->m_oBuff.onFinishEvent(EEXTRAREWARD_LABORATORY);
    pUser->getServant().onFinishEvent(ETRIGGER_LABORATORY);

    //platlog 
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Complete_Laboratory;
    PlatLogManager::getMe().eventLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eid,
      pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().CompleteLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eType,
      eid,
      ECompleteType_Laboratory,
      m_dwRoundNum,
      dwTodayMaxPoint,
      EMONEYTYPE_GARDEN, dwGarden,
      pUser->getLevel());

    {
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_YJS_PASS_COUNT, 0,0, pUser->getLevel(), (DWORD)1);
    }
  }
  for (auto &user : notFirstSet)
  {
    user->getHelpReward(firstUserSet, EHELPTYPE_LABORATORY);
  }
  // 只要一起的人中有我的好友即可
  for (auto &user : firstUserSet)
  {
    user->getHelpReward(allUser, EHELPTYPE_LABORATORY, true);
  }
}

// tower scene
#include "TowerConfig.h"
#include "SceneManager.h"
TowerScene::TowerScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD dwLayer, DWORD dwNoMonsterLayer) :
  DScene(sID, sName, sBase, pRaidCFG)
  , m_dwLayer(dwLayer), m_oTimer(1), m_dwNoMonsterLayer(dwNoMonsterLayer)
{

}

bool TowerScene::init()
{
  if (DScene::init() == false)
    return false;

  return true;
}

void TowerScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  // layer ntf
  TowerLayerSyncTowerCmd sync;
  sync.set_layer(m_dwLayer);
  PROTOBUF(sync, syncsend, synclen);
  user->sendCmdToMe(syncsend, synclen);
  DScene::userEnter(user);

  // reward box ntf
  summonUserRewardBox(user, true);

  // boss layer effect ntf
  const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
  if (m_dwLayer != 0 && m_dwLayer % 10 == 0)
  {
    xLuaData data;
    data.setData("etype", rCFG.dwBossFloorEffect);
    GMCommandRuler::effect(user, data);
  }

  // sky ntf
  const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(m_dwLayer);
  if (pCFG != nullptr)
  {
    user->setOwnSky(pCFG->dwSky);
    if (pCFG->strBgm.empty() == false)
      user->replaceBgmToMe(pCFG->strBgm);
  }
  else
  {
    XERR << "[无限塔场景-玩家进入]" << user->accid << user->id << user->getProfession() << user->name << "未获取 layer :" << m_dwLayer << "配置,没有设置天空球" << XEND;
  }
}

void TowerScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;
  DScene::userLeave(user);
  if (!m_bChangeRaid)
  {
    QWORD qwFollowid = user->getUserSceneData().getFollowerID();
    SceneUser*pUser = SceneUserManager::getMe().getUserByID(qwFollowid);
    if (pUser && pUser->getScene() == this)
    {
      user->getUserSceneData().setFollowerIDNoCheck(0);
    }
  }
}

void TowerScene::entryAction(QWORD curMSec)
{
  DScene::entryAction(curMSec); 
  DWORD curSec = curMSec / 1000;

  //1s tick
  if (m_oTimer.timeUp(curSec))
  {
    DWORD closeTime = xTime::getWeekStart(curSec) + 5 * 3600; //周一五点
    DWORD wDay = xTime::getWeek(curSec);
    if (wDay == 1)
    {
      static DWORD PRE_CLOSE = 2;
      static TSetDWORD setPreCloseTip = { 10 * 60, 7 * 60, 5 * 60, 3 * 60, 2 * 60, 1 * 60 }; 

      for (auto &v : setPreCloseTip)
      {
        if (curSec == closeTime - v)      //会存在误差
        {
          xSceneEntrySet userset;
          getAllEntryList(SCENE_ENTRY_USER, userset);
          for (auto &s : userset)
            MsgManager::sendMsg(s->id, 1318, MsgParams(v/60));    //一股神秘的力量袭来，恩德勒斯塔还有%s分钟即将关闭。
          XLOG << "[无限塔-关闭] 周一五点刷新关闭无限塔,距离关闭还有"<<v/60 << "分钟，当前时间" << curSec << "关闭时间" << closeTime << "当前层" << m_dwLayer << XEND;
          break;
        }
      }

      //周一五点前强制关闭无限塔
      if (curSec > closeTime && curSec <= closeTime + PRE_CLOSE)
      {
        setCloseState();
        XLOG << "[无限塔-关闭] 周一五点刷新关闭无限塔" << "当前时间" << curSec << "关闭时间" << closeTime << "当前层" << m_dwLayer << XEND;
      }
    }
  }
}

void TowerScene::addMonsterReward(DWORD npcid, vector<pair<DWORD, float>>& m_vecAllRewardIDs, DWORD layer, DWORD mapid, bool superAiNpc)
{
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(npcid);
  if (pCFG == nullptr)
    return;
  float fRatio = LuaManager::getMe().call<float>("calcTowerRewardRatio", (DWORD)pCFG->eNpcType, (DWORD)pCFG->eZoneType);
  if(superAiNpc == false)
  {
    for (auto v = pCFG->vecRewardIDs.begin(); v != pCFG->vecRewardIDs.end(); ++v)
    {
      m_vecAllRewardIDs.push_back(pair<DWORD, float>(*v, fRatio));
      XLOG << "[无限塔-怪物掉率]" << pCFG->dwID << pCFG->strName << "在 layer :" << layer << "掉落 reward:" << *v << "ratio:" << fRatio << XEND;
    }
  }
  TSetDWORD setExtraRwds;
  RewardConfig::getMe().getNpcExtraRwd(pCFG, setExtraRwds, superAiNpc);
  for (auto &s : setExtraRwds)
  {
    m_vecAllRewardIDs.push_back(pair<DWORD, float>(s, 1));
    XLOG << "[无限塔-额外掉落]" << pCFG->dwID << pCFG->strName << "额外掉落ID:" << s << "当前地图:" << mapid << XEND;
  }
}

void TowerScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  if (npc == nullptr || npc->getCFG() == nullptr)
    return;

  DScene::onNpcDie(npc, killer);

  XLOG << "[无限塔-怪物击杀]" << npc->id << npc->getNpcID() << npc->name << "在layer :" << m_dwLayer << "被" 
    << (killer == nullptr ? 0 : killer->id) << (killer == nullptr ? "" : killer->name) << "击杀" << XEND;

  addMonsterReward(npc->getNpcID(), m_vecAllRewardIDs, m_dwLayer, getMapID(), npc->define.getSuperAiNpc());
  /*
  float fRatio = LuaManager::getMe().call<float>("calcTowerRewardRatio", (DWORD)npc->getNpcType(), (DWORD)npc->getNpcZoneType());
  for (auto v = npc->getCFG()->vecRewardIDs.begin(); v != npc->getCFG()->vecRewardIDs.end(); ++v)
  {
    m_vecAllRewardIDs.push_back(pair<DWORD, float>(*v, fRatio));
    XLOG << "[无限塔-怪物掉率]" << npc->id << npc->getNpcID() << npc->name << "在 layer :" << m_dwLayer << "掉落 reward:" << *v << "ratio:" << fRatio << XEND;
  }
  TSetDWORD setExtraRwds;
  if (RewardConfig::getMe().getNpcExtraRwd(npc->getCFG(), setExtraRwds))
  {
    for (auto &s : setExtraRwds)
    {
      m_vecAllRewardIDs.push_back(pair<DWORD, float>(s, 1));
      XLOG << "[无限塔-额外掉落]" << npc->getNpcID() << npc->name << npc->id << "额外掉落ID:" << s << "当前地图:" << id << XEND;
    }
  }
  */
}

void TowerScene::onClose()
{
  //切地图的时候不清空数据，如果地图切换失败，会存在无限塔数据没清空的bug。
  if (!m_bChangeRaid)
  {
    m_bChangeRaid = false;
    XLOG << "[无限塔-关闭] 关闭无限塔，清空数据" << XEND;
  }
}

void TowerScene::summonUserRewardBox(SceneUser* pUser, bool bEnter)
{
  if (pUser == nullptr)
    return;
  bool bReward = pUser->getTower().isRewarded(getLayer());
  if (!bReward && bEnter)
    return;

  const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
  NpcDefine oDefine;
  oDefine.load(rCFG.oRewardBoxDefine);
  xPos oPos;
  if (bEnter)
  {
    oPos = rCFG.getPos(m_dwLayer);
  }
  else
  {
    oPos = pUser->getPos();
    if (pUser->getScene() != nullptr)
    {
      xSceneEntrySet userSet;
      pUser->getScene()->getAllEntryList(SCENE_ENTRY_USER, userSet);
      DWORD count = 30;
      while(count--)
      {
        pUser->getScene()->getCircleRoundPos(pUser->getPos(), rCFG.fRewardBoxRange, oPos);
        for (auto &s : userSet)
        {
          if (getDistance(s->getPos(), oPos) * 2 < rCFG.fRewardBoxRange)
            break;
        }
        count = 0;
      }
    }
    else
    {
      oDefine.setRange(rCFG.fRewardBoxRange);
    }
  }
  oDefine.setPos(oPos);
  oDefine.m_oVar.m_qwQuestOwnerID = pUser->id;
  SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, this);
  if (pNpc == nullptr)
  {
    XERR << "[无限塔场景-玩家进入]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "layer :" << m_dwLayer << "召唤奖励宝箱 :" << oDefine.getID() << "失败" << XEND;
    return;
  }

  UserActionNtf cmd;
  cmd.set_charid(pNpc->tempid);
  cmd.set_value(bReward ? rCFG.dwRewardBoxGetAction : rCFG.dwRewardBoxUngetAction);
  cmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  if (!bEnter)
  {
    xLuaData data;
    data.setData("effect", rCFG.strRewardBoxEffect);
    data.setData("npcid", pNpc->getNpcID());
    data.setData("effectpos", 1);
    GMCommandRuler::effect(pUser, data);
  }
}

bool TowerScene::getBornPos(xPos& pos)
{
  if (!base)
    return false;
  const SceneObject *pObj = base->getSceneObject(getRaidID());
  if (pObj == nullptr)
    return false;

  auto nomonster = [&]()
  {
    if (m_dwNoMonsterLayer < m_dwLayer)
      return false;
    const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(m_dwLayer);
    if (pCFG == nullptr)
      return false;
    return pCFG->dwMiniCount == 0 && pCFG->dwMvpCount == 0;
  };

  if (nomonster())
  {
    const ExitPoint* pExit = pObj->getExitPoint(1);
    if (pExit)
    {
      xPos ePos = pExit->m_oPos;
      DWORD i = 0;
      while(i++ < 30)
      {
        if (getCircleRoundPos(pExit->m_oPos, 5, ePos))
        {
          pos = ePos;
          return true;
        }
      }
    }
  }

  const map<DWORD, SBornPoint>& mapList = pObj->getBornPointList();
  for (auto &m : mapList)
  {
    pos = m.second.oPos;
    return true;
  }

  return false;
}

void TowerScene::getRandomDeadBoss(std::list<pair<DWORD,DWORD>>& bosslist)
{
  const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
  for (DWORD i = 0; i < rCFG.dwDeadBossNum; ++i)
  {
    DWORD lv = LuaManager::getMe().call<DWORD>("CalcRaidDeadBossSummonLevel", (DWORD)ERAID_DEADBOSSTYPE_TOWER);
    DWORD monsterid = NpcConfig::getMe().getOneRandomRaidDeadBoss(ERAID_DEADBOSSTYPE_TOWER, lv);
    bosslist.push_back(std::make_pair(monsterid, rCFG.dwDeadBossUID));
  }
}

bool TowerScene::checkSummonDeadBoss()
{
  const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
  return m_dwLayer >= rCFG.dwDeadBossLayer;
}

// ferriswheel scene
#include "SceneUserManager.h"
FerrisWheelScene::FerrisWheelScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :
  DScene(sID, sName, sBase, pRaidCFG)
{

}

bool FerrisWheelScene::init()
{
  return DScene::init();
}

void FerrisWheelScene::userEnter(SceneUser* user)
{
  if (user == nullptr)
    return;

  DScene::userEnter(user);
  if (0 == m_qwCarrierMasterID)
  {
    user->m_oCarrier.create(7,1,0);
    m_qwCarrierMasterID = user->id;
    if (user->m_oCarrier.m_oData.m_dwAssembleID)
    {
      user->m_oCarrier.changeAssemble(user->m_oCarrier.m_oData.m_dwAssembleID);
    }
  }
  else if ((QWORD)-1 == m_qwCarrierMasterID)
  {
  }
  else
  {
    SceneUser *master = SceneUserManager::getMe().getUserByID(m_qwCarrierMasterID);
    if (master)
    {
      master->m_oCarrier.m_oInvites.insert(user->id);
      master->m_oCarrier.join(user);
      master->m_oCarrier.start();

      m_qwCarrierMasterID = (QWORD)-1;

      master->getShare().addCalcData(ESHAREDATATYPE_MOST_WHELL, user->id, 1);
      user->getShare().addCalcData(ESHAREDATATYPE_MOST_WHELL, master->id, 1);
    }
  }
}

void FerrisWheelScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userLeave(user);

  if (m_qwCarrierMasterID && user->id == m_qwCarrierMasterID)
    m_qwCarrierMasterID = 0;
}

// dojo scene
#include "MsgManager.h"
DojoScene::DojoScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD dwDojoId) :
  DScene(sID, sName, sBase, pRaidCFG), m_dwDojoId(dwDojoId)
{
}

bool DojoScene::init()
{
  if (DScene::init() == false)
    return false;

  return true;
}

//void DojoScene::entryAction(QWORD curMSec)
//{
//  DScene::entryAction(curMSec);
//
//  if (getState() == xScene::SCENE_STATE_RUN)
//  {
//    if (m_dwCountDown)
//    {
//      DWORD curTime = now();
//      if (curTime > m_dwNextCountDownTick)
//      {
//        if (!isAllDead())
//        { //关闭倒计时
//          stopCountDown();
//        }
//        else if (curTime > m_dwCountDown) {  //倒计时结束
//          setCloseState();
//        }
//        m_dwNextCountDownTick = curTime + 1;
//      }
//    }
//  }
//}

void DojoScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userEnter(user);
}

void DojoScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;
  DScene::userLeave(user);  
  QWORD qwFollowid = user->getUserSceneData().getFollowerID();
  SceneUser*pUser = SceneUserManager::getMe().getUserByID(qwFollowid);
  if (pUser && pUser->getScene() == this)
  {
    user->getUserSceneData().setFollowerIDNoCheck(0);
  }
}

void DojoScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  if (npc == nullptr)
    return;

  DScene::onNpcDie(npc, killer);
}

void DojoScene::onUserDie(SceneUser *user, xSceneEntryDynamic *killer)
{
  if (user == nullptr)
    return;
  DScene::onUserDie(user, killer);
}

// guild scene
GuildScene::GuildScene(DWORD sID, const char* name, const SceneBase* pBase, const SRaidCFG* pRaidCFG, const GuildInfo& oInfo)
  : DScene(sID, name, pBase, pRaidCFG)
{
  setGuildInfo(oInfo);
  m_dwPhotoRefreshTick = xTime::getCurSec() + MiscConfig::getMe().getGuildCFG().dwPhotoRefreshTime;
  updateBuildingDataRefreshTime();
}

bool GuildScene::init()
{
  if (Scene::init() == false)
    return false;

  GuildMusicBoxManager::getMe().sendGuildMusicQueryRecord(id, m_oGuild.id());
  refreshNpc(false);
  return true;
}

void GuildScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userEnter(user);

  user->syncGuildRaidData(m_oGuild.lv());
  user->getQuest().sendGuildQuestList();

  if (!m_bQueryPhoto)
  {
    m_bQueryPhoto = true;
    QueryPhotoListGuildSCmd cmd;
    cmd.set_guildid(m_oGuild.id());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
  else
  {
    sendPhotoWall(user);
  }

  if (m_bGuildInfoInit == false)
  {
    QueryGuildInfoGuildSCmd cmd;
    cmd.set_guildid(m_oGuild.id());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
  else
  {
    // 同步建筑信息
    syncBuildingDataToUser(user);
  }

  if (!m_bGuildTreasureInit)
  {
    m_bGuildTreasureInit = true;
    QueryTreasureGuildSCmd cmd;
    cmd.set_guildid(m_oGuild.id());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
  else
  {
    syncCurTreasureStatus(user);
  }

  user->sendOpenBuildingGateMsg();
  user->getServant().onAppearEvent(ETRIGGER_GUILD_BUILDING);
  user->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_BUILDING);
}

void GuildScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userLeave(user);

  if (m_qwTreasureOperID == user->id)
  {
    TreasureActionGuildCmd cmd;
    cmd.set_charid(m_qwTreasureOperID);
    cmd.set_action(ETREASUREACTION_FRAME_OFF);
    treasure_action_frame_off(nullptr, cmd);
  }
}

void GuildScene::setGuildInfo(const GuildInfo& rInfo)
{
  m_oGuild.updateInfo(rInfo);

  m_listQuest.clear();
  for (int i = 0; i < rInfo.quests_size(); ++i)
  {
    const GuildQuest& rQuest = rInfo.quests(i);
    const SGuildQuestCFG* pCFG = GuildConfig::getMe().getGuildQuestCFG(rQuest.questid());
    if (pCFG == nullptr)
    {
      XERR << "[公会副本-创建] questid :" << rQuest.questid() << "未在 Table_Guild_Quest.txt 表中找到" << XEND;
      continue;
    }
    m_listQuest.push_back(*pCFG);
  }
}

void GuildScene::refreshNpc(bool bNtf /*= true*/)
{
  const SceneObject* pObj = getSceneObject();
  if (pObj == nullptr)
    return;

  const TMapGuildFuncCFG& mapList = GuildConfig::getMe().getGuildFuncList();
  for (auto &m : mapList)
  {
    if (m.second.dwGuildLv > m_oGuild.lv() || (m.second.eShowFunc > EGUILDFUNCTION_MIN && m.second.eShowFunc < EGUILDFUNCTION_MAX && m_oGuild.isFunctionOpen(m.second.eShowFunc) == false))
      continue;

    // 公会建筑npc处理, 必须在guildinfo初始化后, 玩家的guildinfo不包含建筑信息
    bool isBuildingNpc = false;
    const GuildBuilding* building = nullptr;
    if (m.second.isBuildingNpc())
    {
      if (m_bGuildInfoInit == false)
        continue;
      building = m_oGuild.getBuilding(m.second.stBuildParam.eType);
      if (building == nullptr)
        continue;
      if (m.second.stBuildParam.isShowNpc(building) == false)
        continue;
      isBuildingNpc = true;
    }

    const SceneNpcTemplate* pNpcTemplate = pObj->getRaidNpcTemplate(m.second.dwUniqueID);
    if (pNpcTemplate == nullptr)
    {
      XERR << "[公会领地-npc刷新] uniqueid:" << m.second.dwUniqueID << "为在地图文件中找到" << XEND;
      continue;
    }

    bool bQuestInvalid = m.second.setQuestIDs.empty() == false;
    if (bQuestInvalid)
    {
      const TSetDWORD& setQuestIDs = m.second.setQuestIDs;
      auto l = find_if(m_listQuest.begin(), m_listQuest.end(), [&](const SGuildQuestCFG& r) -> bool{
        return find(setQuestIDs.begin(), setQuestIDs.end(), r.dwQuestID) != setQuestIDs.end();
      });
      bQuestInvalid = l == m_listQuest.end();
    }

    bool bInValid = bQuestInvalid || m.second.isOverTime()
      || m_oGuild.lv() >= m.second.dwDisGuildLv
      || (m.second.eDisFunc > EGUILDFUNCTION_MIN && m.second.eDisFunc < EGUILDFUNCTION_MAX && m_oGuild.isFunctionOpen(m.second.eDisFunc))
      || (isBuildingNpc && m.second.stBuildParam.isDisNpc(building));
    if (bInValid)
    {
      auto npc = m_mapExistNpc.find(m.second.dwUniqueID);
      if (npc != m_mapExistNpc.end())
      {
        SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(npc->second);
        if (pNpc != nullptr)
        {
          pNpc->removeAtonce();
          m_mapExistNpc.erase(m.second.dwUniqueID);
          XLOG << "[公会领地-npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << m.second.dwUniqueID << "消失了" << XEND;
        }
      }

      if (m_mapDelayCreatedNpc.find(m.second.dwUniqueID) != m_mapDelayCreatedNpc.end())
      {
        m_mapDelayCreatedNpc.erase(m.second.dwUniqueID);
        XLOG << "[公会领地-npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << m.second.dwUniqueID << "清除延迟创建" << XEND;
      }
    }
    else
    {
      auto npc = m_mapExistNpc.find(m.second.dwUniqueID);
      if (npc == m_mapExistNpc.end())
      {
        if (isBuildingNpc && m.second.stBuildParam.dwCreateDelay > 0)
        {
          if (m_mapDelayCreatedNpc.find(m.second.dwUniqueID) == m_mapDelayCreatedNpc.end())
          {
            DWORD delay = now() + m.second.stBuildParam.dwCreateDelay;
            m_mapDelayCreatedNpc[m.second.dwUniqueID] = delay;
            XLOG << "[公会领地-npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << m.second.dwUniqueID << "延迟创建:" << delay << XEND;
          }
          continue;
        }

        SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(pNpcTemplate->m_oDefine, this);
        if (pNpc == nullptr)
        {
          XERR << "[公会领地-npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << m.second.dwUniqueID << "刷新失败" << XEND;
          continue;
        }

        m_mapExistNpc.insert(make_pair(m.second.dwUniqueID, pNpc->id));
        XLOG << "[公会领地-npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << m.second.dwUniqueID << "出现了" << XEND;

          // to do...
        if (bNtf)
        {

        }
      }
    }
  }

  refreshBuildingGate();
  refreshBuildingNpc();
  refreshArtifactNpc();
}

void GuildScene::entryAction(QWORD curMSec)
{
  DScene::entryAction(curMSec);
  refreshPhotoWall(curMSec / ONE_THOUSAND);
  refreshDelayCreateNpc(curMSec / ONE_THOUSAND);

  DWORD cur = now();
  if (cur >= m_dwBuildingDataRefreshTime)
  {
    syncBuildingDataToUser();
    updateBuildingDataRefreshTime();
  }
}

void GuildScene::updateBuildingDataRefreshTime()
{
  m_dwBuildingDataRefreshTime = xTime::getDayStart(now(), 5 * HOUR_T) + DAY_T + 5 * HOUR_T;
}

void GuildScene::syncGuildRaidGate()
{
  // 同步公会副本大门状态
  const SGuildRaidCFG& rCfg = MiscConfig::getMe().getGuildRaidCFG();

  xSceneEntrySet set;
  getAllEntryList(SCENE_ENTRY_USER, set);

  for (auto &itCfg : rCfg.mapNpcID2GuildRaid)
  {
    std::list<SceneNpc*> npcs;
    getSceneNpcByBaseID(itCfg.second.dwNpcID, npcs);
    if (npcs.size() < 1)
      continue;

    SceneNpc* pNpc = npcs.front();
    if (pNpc == nullptr)
      continue;

    for (auto &s : set)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(s);
      if (pUser == nullptr)
        continue;
      pUser->unlockGuildRaidGate(pNpc, m_oGuild.lv());
    }
  }
}

void GuildScene::queryPhotoFromGuild(const QueryPhotoListGuildSCmd& cmd)
{
  m_mapFramePhoto.clear();
  m_mapAccPhoto.clear();
  m_mapPhotoFrame.clear();
  m_mapFrameIndexp.clear();

  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  for (int i = 0; i < cmd.frames_size(); ++i)
  {
    const PhotoFrame& rFrame = cmd.frames(i);
    if (rCFG.isValidFrame(rFrame.frameid()) == false)
    {
      XERR << "[公会领地-照片同步]" << m_oGuild.id() << m_oGuild.name() << "同步照片中frameid :" << rFrame.frameid() << "不是有效的相框" << XEND;
      continue;
    }

    vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[rFrame.frameid()];
    for (int j = 0; j < rFrame.photo_size(); ++j)
    {
      const GuildPhoto& rPhoto = rFrame.photo(j);
      string id = GGuild::getPhotoGUID(rPhoto);

      vecPhoto.push_back(rFrame.photo(j));

      m_mapPhotoFrame[id] = rFrame.frameid();
      m_mapAccPhoto[rPhoto.accid_svr()].push_back(id);
    }
  }

  m_bPhotoInit = true;

  sendPhotoWall(nullptr);

  XDBG << "[公会领地-照片同步]" << m_oGuild.id() << m_oGuild.name() << "同步照片" << cmd.ShortDebugString() << XEND;
}

void GuildScene::updatePhoto(const PhotoUpdateGuildSCmd& cmd)
{
  if (cmd.update().accid_svr() != 0)
  {
    string guid = GGuild::getPhotoGUID(cmd.update());
    auto frame = m_mapPhotoFrame.find(guid);
    if (frame != m_mapPhotoFrame.end())
    {
      vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[frame->second];
      auto l = find_if(vecPhoto.begin(), vecPhoto.end(), [&](const GuildPhoto& r) -> bool{
        return GGuild::isSame(cmd.update(), r);
      });
      if (l != vecPhoto.end())
      {
        l->CopyFrom(cmd.update());
        XLOG << "[公会领地-照片更新]" << m_oGuild.id() << m_oGuild.name() << "更新照片" << cmd.update().ShortDebugString() << XEND;
      }
    }
  }

  if (cmd.del().accid_svr() != 0)
  {
    string guid = GGuild::getPhotoGUID(cmd.del());
    auto frame = m_mapPhotoFrame.find(guid);
    if (frame != m_mapPhotoFrame.end())
    {
      vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[frame->second];
      auto l = find_if(vecPhoto.begin(), vecPhoto.end(), [&](const GuildPhoto& r) -> bool{
        return GGuild::isSame(cmd.del(), r);
      });
      if (l != vecPhoto.end())
        vecPhoto.erase(l);

      TVecString& vecIDs = m_mapAccPhoto[cmd.del().accid_svr()];
      auto v = find(vecIDs.begin(), vecIDs.end(), guid);
      if (v != vecIDs.end())
        vecIDs.erase(v);

      m_mapPhotoFrame.erase(guid);
      XLOG << "[公会领地-照片更新]" << m_oGuild.id() << m_oGuild.name() << "删除照片" << cmd.del().ShortDebugString() << XEND;
    }
  }
}

bool GuildScene::frameAction(SceneUser* pUser, const FrameActionPhotoCmd& cmd)
{
  if (pUser == nullptr)
    return false;

  if (!m_bPhotoInit)
  {
#ifdef _DEBUG
    MsgManager::sendMsg(pUser->id, 10, MsgParams("测试,这不是bug,不要着急,还在初始化,需要等彭冉配置这个msg"));
#endif
    XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "上传照片" << cmd.ShortDebugString() << "失败,相框未初始化" << XEND;
    return false;
  }

  if (GGuild::hasAuth(pUser->getGuild().auth(), EAUTH_UPLOAD_PHOTO) == false)
  {
#ifdef _DEBUG
    MsgManager::sendMsg(pUser->id, 10, MsgParams("测试,这不是bug,你还有上传照片权限,需要等彭冉配置这个msg"));
#endif
    XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "上传照片" << cmd.ShortDebugString() << "失败,相框未初始化" << XEND;
    return false;
  }

  FrameActionPhotoCmd ret;
  ret.CopyFrom(cmd);
  ret.clear_photos();
  for (int i = 0; i < cmd.photos_size(); ++i)
  {
    const GuildPhoto& rPhoto = cmd.photos(i);
    if (photoAction(pUser, cmd.action(), cmd.frameid(), rPhoto) == true)
      ret.add_photos()->CopyFrom(cmd.photos(i));
  }

  if (ret.photos_size() > 0)
  {
    PROTOBUF(ret, send, len);
    pUser->sendCmdToMe(send, len);
  }

  XLOG << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "上传照片" << cmd.photos_size() << "张,成功" << ret.photos_size() << "张" << XEND;
  return ret.photos_size() > 0;
}

bool GuildScene::queryFrame(SceneUser* pUser, DWORD dwFrameID)
{
  if (pUser == nullptr)
    return false;
  if (!m_bPhotoInit)
  {
    XERR << "[公会领地-相框请求]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "请求相框" << dwFrameID << "中照片失败,相框未初始化" << XEND;
    return false;
  }

  QueryFramePhotoListPhotoCmd cmd;
  cmd.set_frameid(dwFrameID);

  DWORD dwNow = xTime::getCurSec();
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[dwFrameID];
  for (auto &v : vecPhoto)
  {
    const GuildSMember* pMember = pUser->getGuild().getMember(v.charid(), v.accid_svr());
    if (pMember == nullptr || (pMember->offlinetime() > pMember->onlinetime() && dwNow > pMember->offlinetime() + rCFG.dwPhotoMemberActiveDay * DAY_T))
      continue;
    cmd.add_photos()->CopyFrom(v);
  }

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  XLOG << "[公会领地-相框请求]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "请求相框" << dwFrameID << "获得照片" << cmd.ShortDebugString() << XEND;
  return true;
}

void GuildScene::sendSelfPhoto(SceneUser* pUser)
{
  if (!m_bPhotoInit || pUser == nullptr)
    return;

  map<DWORD, vector<GuildPhoto>> mapPhoto;
  auto m = m_mapAccPhoto.find(pUser->accid);
  if (m != m_mapAccPhoto.end())
  {
    for (auto &v : m->second)
    {
      auto frame = m_mapPhotoFrame.find(v);
      if (frame == m_mapPhotoFrame.end())
        continue;

      auto photo = m_mapFramePhoto.find(frame->second);
      if (photo == m_mapFramePhoto.end())
        continue;

      auto item = find_if(photo->second.begin(), photo->second.end(), [&](const GuildPhoto& r) -> bool{
          return GGuild::getPhotoGUID(r) == v;
          });
      if (item != photo->second.end())
        mapPhoto[frame->second].push_back(*item);
    }
  }

  QueryUserPhotoListPhotoCmd usercmd;
  usercmd.set_maxphoto(MiscConfig::getMe().getGuildCFG().dwMaxPhotoPerMember);
  usercmd.set_maxframe(MiscConfig::getMe().getGuildCFG().dwMaxFramePhotoCount);
  for (auto &m : mapPhoto)
  {
    PhotoFrame* pFrame = usercmd.add_frames();
    pFrame->set_frameid(m.first);
    for (auto &v : m.second)
      pFrame->add_photo()->CopyFrom(v);
  }
  PROTOBUF(usercmd, usersend, userlen);
  pUser->sendCmdToMe(usersend, userlen);

  XLOG << "[公会领地-个人照片]" << m_oGuild.id() << m_oGuild.name() << "成员" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "同步上传照片" << usercmd.ShortDebugString() << XEND;
}

void GuildScene::refreshPhotoWall(DWORD curSec)
{
  if (m_dwPhotoRefreshTick > curSec)
    return;
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  m_dwPhotoRefreshTick = curSec + rCFG.dwPhotoRefreshTime;

  for (auto &s : rCFG.setFrameIDs)
  {
    vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[s];
    DWORD& rIndex = m_mapFrameIndexp[s];

    if (vecPhoto.empty() == true)
    {
      rIndex = 0;
      continue;
    }

    if (++rIndex >= vecPhoto.size())
      rIndex = 0;

    XLOG << "[公会领地-相框同步] 公会" << m_oGuild.id() << m_oGuild.name() << "frame :" << s << "刷新相框" << rIndex << XEND;
  }

  sendPhotoWall(nullptr);
}

void GuildScene::sendPhotoWall(SceneUser* pUser)
{
  SceneUser* pTemp = nullptr;
  if (pUser != nullptr)
  {
    pTemp = pUser;
  }
  else
  {
    xSceneEntrySet set;
    getAllEntryList(SCENE_ENTRY_USER, set);
    for (auto &s : set)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(s);
      if (pUser != nullptr)
        pTemp = pUser;
    }
  }

  DWORD dwNow = xTime::getCurSec();
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  UpdateFrameShowPhotoCmd cmd;
  for (auto &s : rCFG.setFrameIDs)
  {
    vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[s];
    if (vecPhoto.empty() == true)
      continue;

    DWORD& rIndex = m_mapFrameIndexp[s];
    if (rIndex >= vecPhoto.size())
      continue;

    const GuildPhoto& rPhoto = vecPhoto[rIndex];

    if (pTemp != nullptr)
    {
      const GuildSMember* pMember = pTemp->getGuild().getMember(rPhoto.charid(), rPhoto.accid_svr());
      if (pMember == nullptr || (pMember->offlinetime() > pMember->onlinetime() && dwNow > pMember->offlinetime() + rCFG.dwPhotoMemberActiveDay * DAY_T))
        continue;
      if (GGuild::hasAuth(pMember->auth(), EAUTH_UPLOAD_PHOTO) == false)
        continue;
    }

    FrameShow* pFrame = cmd.add_shows();
    pFrame->set_frameid(s);
    pFrame->mutable_photo()->CopyFrom(rPhoto);
  }

  PROTOBUF(cmd, send, len);
  if (pUser == nullptr)
  {
    MsgManager::sendMapCmd(id, send, len);
    XLOG << "[公会领地-相框同步] 公会" << m_oGuild.id() << m_oGuild.name() << "全地图同步相框照片" << cmd.ShortDebugString() << XEND;
  }
  else
  {
    pUser->sendCmdToMe(send, len);
    XLOG << "[公会领地-相框同步] 公会" << m_oGuild.id() << m_oGuild.name() << "向成员" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "同步相框照片" << cmd.ShortDebugString() << XEND;
  }
}

bool GuildScene::photoAction(SceneUser* pUser, EFrameAction eAction, DWORD dwFrameID, const GuildPhoto& rPhoto)
{
  if (pUser == nullptr)
    return false;

  bool bRefresh = false;
  GuildPhoto oPhoto;
  oPhoto.CopyFrom(rPhoto);
  if (modifyPhoto(pUser, oPhoto) == false)
  {
    MsgManager::sendDebugMsg(pUser->id, "测试,这不是bug,不存在的照片,需要等彭冉配置这个msg");
    XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "失败,照片不存在" << XEND;
    return false;
  }

  if (GuildConfig::getMe().isSuitableFrame(dwFrameID, oPhoto.anglez()) == false)
  {
    MsgManager::sendDebugMsg(pUser->id, "测试,这不是bug,这个照片传这个相框不适合,需要等彭冉配置这个msg");
    XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "失败,相框不对应" << XEND;
    return false;
  }

  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  if (rCFG.isValidFrame(dwFrameID) == false)
  {
    MsgManager::sendDebugMsg(pUser->id, "测试,这不是bug,不是合法的相框,需要等彭冉配置这个msg");
    XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "失败,相框不合法" << XEND;
    return false;
  }

  FramePhotoUpdatePhotoCmd cmd;
  FrameUpdateGuildSCmd scmd;

  cmd.set_frameid(dwFrameID);
  scmd.set_frameid(dwFrameID);

  if (eAction == EFRAMEACTION_UPLOAD)
  {
    string id = GGuild::getPhotoGUID(oPhoto);

    auto frame = m_mapPhotoFrame.find(id);
    if (frame != m_mapPhotoFrame.end() && frame->second != cmd.frameid())
    {
      MsgManager::sendDebugMsg(pUser->id, "测试,这不是bug,该照片在其他相框中上传过,需要等彭冉配置这个msg");
      return false;
    }

    TVecString& vecIDs = m_mapAccPhoto[pUser->accid];
    auto idv = find(vecIDs.begin(), vecIDs.end(), id);
    if (idv == vecIDs.end())
    {
      if (vecIDs.size() >= rCFG.dwMaxPhotoPerMember)
      {
#ifdef _DEBUG
        MsgManager::sendMsg(pUser->id, 10, MsgParams("测试,这不是bug,超过个人最大上传照片数量,需要等彭冉配置这个msg"));
#endif
        XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "失败,超过个人上传最大数" << rCFG.dwMaxPhotoPerMember << XEND;
        return false;
      }
    }

    vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[dwFrameID];
    auto photo = find_if(vecPhoto.begin(), vecPhoto.end(), [oPhoto](const GuildPhoto& r) -> bool{
        return GGuild::isSame(r, oPhoto);
        });
    if (photo == vecPhoto.end())
    {
      if (vecPhoto.size() >= rCFG.dwMaxFramePhotoCount && removeFirstPhoto(dwFrameID) == false)
      {
#ifdef _DEBUG
        MsgManager::sendMsg(pUser->id, 10, MsgParams("测试,这不是bug,超过相框最大上传照片数量,删除最早的失败,需要等彭冉配置这个msg"));
#endif
        XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "失败,超过相框上传最大数" << rCFG.dwMaxPhotoPerMember << XEND;
        return false;
      }
      idv = find(vecIDs.begin(), vecIDs.end(), id);
      photo = find_if(vecPhoto.begin(), vecPhoto.end(), [oPhoto](const GuildPhoto& r) -> bool{
          return GGuild::isSame(r, oPhoto);
          });
    }

    m_mapPhotoFrame[id] = dwFrameID;
    if (idv == vecIDs.end())
      vecIDs.push_back(id);
    if (photo != vecPhoto.end())
    {
      photo->CopyFrom(oPhoto);
    }
    else
    {
      if (vecPhoto.empty() == true)
        bRefresh = true;
      vecPhoto.push_back(oPhoto);
    }

    cmd.mutable_update()->CopyFrom(oPhoto);
    scmd.mutable_update()->CopyFrom(oPhoto);
  }
  else if (eAction == EFRAMEACTION_REMOVE)
  {
    vector<GuildPhoto>& vecPhoto = m_mapFramePhoto[dwFrameID];
    auto v = find_if(vecPhoto.begin(), vecPhoto.end(), [oPhoto](const GuildPhoto& r) -> bool{
        return GGuild::isSame(r, oPhoto);
        });
    if (v == vecPhoto.end())
    {
      XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "失败,未找到该照片" << XEND;
      return false;
    }

    string id = GGuild::getPhotoGUID(*v);
    TVecString& vecIDs = m_mapAccPhoto[pUser->accid];
    auto idv = find(vecIDs.begin(), vecIDs.end(), id);
    if (idv == vecIDs.end())
    {
      XERR << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "失败,不是自己的照片" << XEND;
      return false;
    }

    vecIDs.erase(idv);
    m_mapPhotoFrame.erase(id);
    cmd.mutable_del()->CopyFrom(*v);
    scmd.mutable_del()->CopyFrom(*v);
    vecPhoto.erase(v);

    if (vecPhoto.empty() == true)
      bRefresh = true;
  }

  if (cmd.update().accid_svr() != 0 || cmd.del().accid_svr() != 0)
  {
    PROTOBUF(cmd, send, len);
    MsgManager::sendMapCmd(id, send, len);

    scmd.set_guildid(m_oGuild.id());
    PROTOBUF(scmd, ssend, slen);
    thisServer->sendCmdToSession(ssend, slen);

    if (bRefresh)
      sendPhotoWall(nullptr);
  }

  XLOG << "[公会领地-照片操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "对相框" << dwFrameID << "中照片" << oPhoto.ShortDebugString() << "进行" << eAction << "成功 立即刷新 :" << bRefresh << XEND;
  return true;
}

bool GuildScene::removeFirstPhoto(DWORD dwFrameID)
{
  auto m = m_mapFramePhoto.find(dwFrameID);
  if (m == m_mapFramePhoto.end())
    return false;

  if (m->second.empty() == true || m->second.size() < MiscConfig::getMe().getGuildCFG().dwMaxFramePhotoCount)
    return true;

  GuildPhoto& rPhoto = *m->second.begin();
  string id = GGuild::getPhotoGUID(rPhoto);

  m_mapPhotoFrame.erase(id);

  FrameUpdateGuildSCmd scmd;
  scmd.set_frameid(dwFrameID);
  scmd.mutable_del()->CopyFrom(rPhoto);
  scmd.set_guildid(m_oGuild.id());
  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmdToSession(ssend, slen);

  for (auto &photo : m_mapAccPhoto)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByAccID(photo.first);
    for (auto v = photo.second.begin(); v != photo.second.end();)
    {
      if (*v != id)
      {
        ++v;
        continue;
      }

      if (pUser != nullptr)
      {
        FramePhotoUpdatePhotoCmd cmd;

        cmd.set_frameid(dwFrameID);
        cmd.mutable_del()->CopyFrom(rPhoto);

        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
      }

      v = photo.second.erase(v);
    }
  }

  m->second.erase(m->second.begin());
  XLOG << "[公会领地-自动移除] frameid :" << dwFrameID << rPhoto.ShortDebugString() << "被移除" << XEND;
  return true;
}

void GuildScene::refreshBuildingGate()
{
  // 公会建筑入口大门
  std::list<SceneNpc*> npcs;
  getSceneNpcByBaseID(MiscConfig::getMe().getGuildBuildingCFG().dwGateNpcID, npcs);
  if (npcs.size() < 1)
    return;

  SceneNpc* npc = npcs.front();
  if (npc != nullptr)
  {
    if (npc->getGearStatus() != 2001 && m_oGuild.isFunctionOpen(EGUILDFUNCTION_BUILDING))
    {
      npc->setGearStatus(2001);
      xSceneEntrySet set;
      getEntryListInNine(SCENE_ENTRY_USER, npc->getPos(), set);
      for (auto& s : set)
      {
        SceneUser* user = dynamic_cast<SceneUser*>(s);
        if (user == nullptr)
          continue;
        npc->sendGearStatus(user);
      }
    }
  }
}

// 刷新建筑npc的机关状态
void GuildScene::refreshBuildingNpc()
{
  // 公会建筑npc处理, 必须在guildinfo初始化后, 玩家的guildinfo不包含建筑信息
  if (m_bGuildInfoInit == false)
    return;

  // 公会建筑
  set<SceneNpc*> npcs;
  TVecDWORD remove;
  for (auto& v : m_mapExistNpc)
  {
    const SGuildFuncCFG* cfg = GuildConfig::getMe().getGuildFuncCFG(v.first);
    if (cfg == nullptr || cfg->isBuildingNpc() == false)
      continue;

    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v.second);
    if (npc == nullptr)
      continue;

    if (cfg->stBuildParam.isDisNpc(m_oGuild.getBuilding(cfg->stBuildParam.eType)))
    {
      npc->removeAtonce();
      remove.push_back(cfg->dwUniqueID);
      XLOG << "[公会领地-npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << cfg->dwUniqueID << "消失了" << XEND;
      continue;
    }

    if (cfg->stBuildParam.bGearStatus)
    {
      DWORD gs = npc->getGearStatus(), newgs = 0, buff = 0;
      const GuildBuilding* b = m_oGuild.getBuilding(cfg->stBuildParam.eType);
      if (b == nullptr)
      {
        newgs = 1001;
      }
      else
      {
        if (b->level() <= 0)
        {
          if (b->isbuilding())
            newgs = 2001;
          else
            newgs = 1001;
        }
        else
        {
          newgs = 3001;
          // 升级状态中的建筑需添加buff, 用于显示特效正在升级中
          if (b->isbuilding() && cfg->stBuildParam.dwBuffWhenLvup)
            buff = cfg->stBuildParam.dwBuffWhenLvup;
        }
      }
      if (gs != newgs)
      {
        XDBG << "[公会场景-建筑状态切换]" << m_oGuild.id() << m_oGuild.name() << "npc:" << cfg->strName << "旧状态:" << gs << "新状态:" << newgs << XEND;
        npc->setGearStatus(newgs);
        npcs.insert(npc);
      }
      if (buff && npc->m_oBuff.haveBuff(buff) == false)
        npc->m_oBuff.add(buff);
      else if (buff <= 0 && cfg->stBuildParam.dwBuffWhenLvup && npc->m_oBuff.haveBuff(cfg->stBuildParam.dwBuffWhenLvup))
        npc->m_oBuff.del(cfg->stBuildParam.dwBuffWhenLvup);
    }
  }

  for (auto id : remove)
    m_mapExistNpc.erase(id);

  if (npcs.empty() == false)
  {
    xSceneEntrySet set;
    for (auto npc : npcs)
    {
      getEntryListInNine(SCENE_ENTRY_USER, npc->getPos(), set);
      for (auto& s : set)
      {
        SceneUser* user = dynamic_cast<SceneUser*>(s);
        if (user == nullptr)
          continue;
        npc->sendGearStatus(user);
      }
    }
  }
}

void GuildScene::syncBuildingDataToUser(SceneUser* user/* = nullptr*/, const set<EGuildBuilding>& types/* = set<EGuildBuilding>()*/)
{
  auto toclient = [](GuildBuilding* b, SceneUser* u)
  {
    if (b == nullptr)
      return;
    const SGuildBuildingCFG* cfg = GuildConfig::getMe().getGuildBuildingCFG(b->type(), b->level());
    if (cfg == nullptr)
      return;

    float total = 0, cur = 0;
    map<DWORD, DWORD> submit, rest;
    for (int i = 0; i < b->materials_size(); ++i)
    {
      DWORD id = b->materials(i).id(), count = b->materials(i).count();
      if (submit.find(id) == submit.end())
        submit[id] = 0;
      submit[id] += count / 100;
      auto it = cfg->mapMaterial.find(id);
      if (it != cfg->mapMaterial.end())
        cur += count >= it->second * 100 ? it->second : count / 100.0;
      else
        cur += count / 100.0;
    }
    for (auto& all : cfg->mapMaterial)
    {
      total += all.second;
      auto it = submit.find(all.first);
      if (it == submit.end())
      {
        rest[all.first] = all.second;
      }
      else if (it->second < all.second)
      {
        rest[all.first] = all.second - it->second;
      }
    }

    for (auto& v : rest)
    {
      const SGuildBuildingMaterialCFG* cfg = GuildConfig::getMe().getGuildBuildingMaterial(v.first);
      if (cfg == nullptr)
        continue;
      DWORD itemid = 0, itemcount = 0;
      if (cfg->getRandItem(itemid, itemcount, b->type(), u->id, v.first) == false)
        continue;
      GuildBuildMaterial* p = b->add_restmaterials();
      if (p)
      {
        p->set_id(v.first);
        p->set_count(v.second);
        p->set_itemid(itemid);
        p->set_itemcount(itemcount);
        p->set_rewardid(cfg->dwRewardID);
      }
    }

    if (cur > total)
      b->set_progress(1000);
    else if (total)
      b->set_progress(floor(cur / total * 1000));
  };

  auto sendf = [&](SceneUser* u)
  {
    BuildingNtfGuildCmd cmd;
    if (types.empty())
    {
      const TMapGuildBuilding& buildings = m_oGuild.getBuildingList();
      for (auto& v : buildings)
      {
        GuildBuilding* p = cmd.add_buildings();
        if (p)
        {
          p->CopyFrom(v.second);
          toclient(p, u);
        }
      }
    }
    else
    {
      for (auto type : types)
      {
        const GuildBuilding* b = m_oGuild.getBuilding(type);
        if (b == nullptr)
          continue;
        GuildBuilding* p = cmd.add_buildings();
        if (p)
        {
          p->CopyFrom(*b);
          toclient(p, u);
        }
      }
    }

    if (cmd.buildings_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      u->sendCmdToMe(send, len);
    }
  };

  if (user == nullptr)
  {
    xSceneEntrySet set;
    getAllEntryList(SCENE_ENTRY_USER, set);
    for (auto& s : set)
    {
      SceneUser* u = dynamic_cast<SceneUser*>(s);
      if (u == nullptr)
        continue;
      sendf(u);
    }
  }
  else
  {
    sendf(user);
  }
}

void GuildScene::refreshDelayCreateNpc(DWORD curSec)
{
  if (m_mapDelayCreatedNpc.empty())
    return;

  const SceneObject* pObj = getSceneObject();
  if (pObj == nullptr)
    return;

  for (auto it = m_mapDelayCreatedNpc.begin(); it != m_mapDelayCreatedNpc.end(); )
  {
    if (curSec < it->second)
    {
      ++it;
      continue;
    }

    auto npc = m_mapExistNpc.find(it->first);
    if (npc != m_mapExistNpc.end())
    {
      it = m_mapDelayCreatedNpc.erase(it);
      continue;
    }

    const SceneNpcTemplate* pNpcTemplate = pObj->getRaidNpcTemplate(it->first);
    if (pNpcTemplate == nullptr)
    {
      XERR << "[公会领地-延迟npc刷新] uniqueid:" << it->first << "为在地图文件中找到" << XEND;
      ++it;
      continue;
    }

    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(pNpcTemplate->m_oDefine, this);
    if (pNpc == nullptr)
    {
      XERR << "[公会领地-延迟npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << it->first << "刷新失败" << XEND;
      ++it;
      continue;
    }

    m_mapExistNpc.insert(make_pair(it->first, pNpc->id));
    XLOG << "[公会领地-延迟npc刷新]" << m_oGuild.id() << m_oGuild.name() << "NPC :" << it->first << "出现了" << XEND;

    it = m_mapDelayCreatedNpc.erase(it);
  }
}

void GuildScene::updateGuildInfo(const QueryGuildInfoGuildSCmd& cmd)
{
  m_bGuildInfoInit = true;
  setGuildInfo(cmd.info());
  refreshNpc();
  syncBuildingDataToUser();
}

void GuildScene::refreshArtifactNpc()
{
  set<SceneNpc*> npcs;
  const TMapGuildArtifact& artifacts = m_oGuild.getArtifactList();
  const TSetString& needgear = m_oGuild.getNewArtifact();
  for (auto& artifact : artifacts)
  {
    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(artifact.second.itemid());
    if (cfg == nullptr || cfg->eBuildingType != EGUILDBUILDING_HIGH_REFINE) // 目前只有神器武器相关的神器有石像npc
      continue;
    auto it = m_mapExistNpc.find(cfg->dwNpcUniqueID);
    if (it == m_mapExistNpc.end())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(it->second);
    if (npc == nullptr)
      continue;
    DWORD gs = npc->getGearStatus(), newgs = 0;
    if (gs != 0 && needgear.find(artifact.second.guid()) == needgear.end())
      continue;
    if (cfg->dwLevel == 1)
      newgs = 2001;
    else if (cfg->dwLevel > 1)
      newgs = 3001;
    newgs = gs > newgs ? gs : newgs;
    XDBG << "[公会场景-神器状态切换]" << m_oGuild.id() << m_oGuild.name() << "神器:" << cfg->strName << "npc:" << it->first << "旧状态:" << gs << "新状态:" << newgs << XEND;
    npc->setGearStatus(newgs);
    npcs.insert(npc);
  }

  m_oGuild.clearNewArtifact();

  if (npcs.empty() == false)
  {
    xSceneEntrySet set;
    for (auto npc : npcs)
    {
      getEntryListInNine(SCENE_ENTRY_USER, npc->getPos(), set);
      for (auto& s : set)
      {
        SceneUser* user = dynamic_cast<SceneUser*>(s);
        if (user == nullptr)
          continue;
        npc->sendGearStatus(user);
      }
    }
  }
}

void GuildScene::queryTreasureFromGuild(const QueryTreasureGuildSCmd& cmd)
{
  m_mapTreasure.clear();
  for (int i = 0; i < cmd.treasures_size(); ++i)
  {
    const GuildTreasure& rTreasure = cmd.treasures(i);
    m_mapTreasure[rTreasure.id()] += rTreasure.count();
  }

  m_dwBCoinCount = cmd.bcoin_count();
  m_dwAssetCount = cmd.asset_count();

  if (m_eTreasureType != EGUILDTREASURETYPE_MIN && m_qwTreasureOperID != 0)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(m_qwTreasureOperID);
    if (pUser != nullptr)
    {
      TreasureActionGuildCmd cmd;
      cmd.set_charid(pUser->id);
      cmd.set_action(m_eLastTreasureAction);
      treasure_action_open(pUser, cmd);
    }
  }

  XDBG << "[公会宝箱-查询]" << m_oGuild.id() << m_oGuild.name() << "查询宝箱数据" << cmd.ShortDebugString() << XEND;
}

void GuildScene::syncCurTreasureStatus(SceneUser* pTarget)
{
  if (pTarget == nullptr)
    return;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(m_qwTreasureOperID);
  if (pUser == nullptr)
    return;

  TreasureActionGuildCmd cmd;
  cmd.set_charid(m_qwTreasureOperID);
  cmd.set_guild_treasure_count(m_dwAssetCount);
  cmd.set_bcoin_treasure_count(m_dwBCoinCount);

  if (m_eTreasureType == EGUILDTREASURETYPE_GVG)
  {
    if (m_mapTreasure.empty() == true)
      return;

    cmd.set_action(ETREASUREACTION_GVG_FRAME_ON);

    cmd.mutable_treasure()->set_id(m_mapTreasure.begin()->first);
    cmd.mutable_treasure()->set_count(m_mapTreasure.begin()->second);
  }
  else if (m_eTreasureType == EGUILDTREASURETYPE_GUILD_ASSET || m_eTreasureType == EGUILDTREASURETYPE_GUILD_BCOIN)
  {
    if (m_dwTreasureIndex >= m_vecCurTreasureCFG.size())
      return;

    cmd.set_action(ETREASUREACTION_GUILD_FRAME_ON);
    cmd.mutable_treasure()->set_id(m_vecCurTreasureCFG[m_dwTreasureIndex].dwID);
  }

  PROTOBUF(cmd, send, len);
  pTarget->sendCmdToMe(send, len);
  XDBG << "[公会宝箱-状态]" << m_oGuild.id() << m_oGuild.name() << "成员" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "同步状态" << cmd.ShortDebugString() << XEND;
}

void GuildScene::treasureAction(SceneUser* pUser, TreasureActionGuildCmd& cmd)
{
  if (pUser == nullptr)
    return;

  SceneNpc* pNpc = pUser->getVisitNpcObj();
  if (pNpc == nullptr)
  {
    XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行" << cmd.ShortDebugString() << "失败,未访问npc" << XEND;
    return;
  }
  if (m_qwTreasureOperID != 0 && pUser->id != m_qwTreasureOperID)
  {
    MsgManager::sendMsg(pUser->id, 4033);
    return;
  }
  if (cmd.action() == ETREASUREACTION_GVG_FRAME_ON || cmd.action() == ETREASUREACTION_OPEN_GVG)
  {
    if (pUser->getGuild().hasAuth(EAUTH_TREASURE_OPT) == false)
    {
      MsgManager::sendMsg(pUser->id, 4034);
      XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行" << cmd.ShortDebugString() << "失败,没有权限" << XEND;
      return;
    }
  }

  cmd.set_charid(m_qwTreasureOperID);
  cmd.set_guild_treasure_count(m_dwAssetCount);
  cmd.set_bcoin_treasure_count(m_dwBCoinCount);

  switch (cmd.action())
  {
    case ETREASUREACTION_MIN:
    case ETREASUREACTION_MAX:
      break;
    case ETREASUREACTION_GVG_FRAME_ON:
      treasure_action_gvg_frame_on(pUser, cmd);
      break;
    case ETREASUREACTION_GUILD_FRAME_ON:
      treasure_action_guild_frame_on(pUser, cmd);
      break;
    case ETREASUREACTION_FRAME_OFF:
      treasure_action_frame_off(pUser, cmd);
      break;
    case ETREASUREACTION_LEFT:
    case ETREASUREACTION_RIGHT:
      treasure_action_change(pUser, cmd);
      break;
    case ETREASUREACTION_OPEN_GVG:
    case ETREASUREACTION_OPEN_GUILD:
      treasure_action_open_pre(pUser, cmd);
      break;
  }

  m_eLastTreasureAction = cmd.action();
  XLOG << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行" << cmd.ShortDebugString() << "成功" << XEND;
}

void GuildScene::notifyServantEvent()
{
  xSceneEntrySet set;
  getAllEntryList(SCENE_ENTRY_USER, set);
  for (auto& s : set)
  {
    SceneUser* u = dynamic_cast<SceneUser*>(s);
    if (u == nullptr)
      continue;
    u->getServant().onAppearEvent(ETRIGGER_GUILD_BUILDING);
    u->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_BUILDING);
  }
}

void GuildScene::treasure_action_gvg_frame_on(SceneUser* pUser, TreasureActionGuildCmd& cmd)
{
  if (m_mapTreasure.empty() == true)
  {
    XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "打开据点界面失败,未包含任何据点宝箱" << XEND;
    return;
  }

  m_qwTreasureOperID = pUser->id;
  m_eTreasureType = EGUILDTREASURETYPE_GVG;
  m_dwTreasureIndex = 0;

  cmd.set_charid(m_qwTreasureOperID);
  cmd.mutable_treasure()->set_id(m_mapTreasure.begin()->first);
  cmd.mutable_treasure()->set_count(m_mapTreasure.begin()->second);

  PROTOBUF(cmd, send, len);
  MsgManager::sendMapCmd(id, send, len);
}

void GuildScene::treasure_action_guild_frame_on(SceneUser* pUser, TreasureActionGuildCmd& cmd)
{
  const TVecGuildTreasureCFG& vecCFG = GuildConfig::getMe().getGuildTreasureCFGList();
  if (vecCFG.empty() == true)
  {
    XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "打开公会界面失败,未包含任何公会宝箱" << XEND;
    return;
  }
  m_qwTreasureOperID = pUser->id;
  m_eTreasureType = vecCFG.begin()->eType;
  m_dwTreasureIndex = 0;

  m_vecCurTreasureCFG.clear();
  for (auto &v : vecCFG)
  {
    if (v.dwCityID == 0)
    {
      m_vecCurTreasureCFG.push_back(v);
      continue;
    }

    const GuildCityInfo* pInfo = GuildCityManager::getMe().getRealCityInfoByGuild(m_oGuild.id());
    if (pInfo == nullptr || pInfo->flag() != v.dwCityID)
      continue;
    m_vecCurTreasureCFG.push_back(v);
  }

  if (m_vecCurTreasureCFG.empty() == true)
  {
    XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "开公会界面失败,未收集到任何宝箱" << XEND;
    return;
  }

  sort(m_vecCurTreasureCFG.begin(), m_vecCurTreasureCFG.end(), [](const SGuildTreasureCFG& r1, const SGuildTreasureCFG& r2) -> bool{
    return r1.dwOrderID < r2.dwOrderID;
  });

  if (m_vecCurTreasureCFG.size() > 1)
    cmd.set_point(ETREASUREPOINT_RIGHT);
  else
    cmd.set_point(ETREASUREPOINT_NONE);

  cmd.set_charid(m_qwTreasureOperID);
  cmd.mutable_treasure()->set_id(m_vecCurTreasureCFG.begin()->dwID);
  cmd.mutable_treasure()->set_count(1);

  PROTOBUF(cmd, send, len);
  MsgManager::sendMapCmd(id, send, len);
}

void GuildScene::treasure_action_frame_off(SceneUser* pUser, TreasureActionGuildCmd& cmd)
{
  m_qwTreasureOperID = m_dwTreasureIndex = 0;
  m_eTreasureType = EGUILDTREASURETYPE_MIN;

  cmd.clear_treasure();
  PROTOBUF(cmd, send, len);
  MsgManager::sendMapCmd(id, send, len);
}

void GuildScene::treasure_action_change(SceneUser* pUser, TreasureActionGuildCmd& cmd)
{
  if (m_eTreasureType == EGUILDTREASURETYPE_GVG)
  {
    if (m_mapTreasure.empty() == true)
    {
      XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "切换gvg宝箱失败,未包含gvg宝箱" << XEND;
      return;
    }
    cmd.mutable_treasure()->set_id(m_mapTreasure.begin()->first);
    cmd.mutable_treasure()->set_count(m_mapTreasure.begin()->second);
  }
  else if (m_eTreasureType == EGUILDTREASURETYPE_GUILD_BCOIN || m_eTreasureType == EGUILDTREASURETYPE_GUILD_ASSET)
  {
    if (m_vecCurTreasureCFG.empty() == true)
      return;

    if (cmd.action() == ETREASUREACTION_LEFT)
    {
      if (m_dwTreasureIndex == 0)
        return;
      m_dwTreasureIndex -= 1;

      if (m_dwTreasureIndex == 0)
        cmd.set_point(ETREASUREPOINT_RIGHT);
      else
        cmd.set_point(ETREASUREPOINT_ALL);

      cmd.mutable_treasure()->set_id(m_vecCurTreasureCFG[m_dwTreasureIndex].dwID);
      cmd.mutable_treasure()->set_count(1);
    }
    else if (cmd.action() == ETREASUREACTION_RIGHT)
    {
      if (m_dwTreasureIndex >= m_vecCurTreasureCFG.size() - 1)
        return;
      m_dwTreasureIndex += 1;

      if (m_dwTreasureIndex == m_vecCurTreasureCFG.size() - 1)
        cmd.set_point(ETREASUREPOINT_LEFT);
      else
        cmd.set_point(ETREASUREPOINT_ALL);

      cmd.mutable_treasure()->set_id(m_vecCurTreasureCFG[m_dwTreasureIndex].dwID);
      cmd.mutable_treasure()->set_count(1);
    }
  }

  PROTOBUF(cmd, send, len);
  MsgManager::sendMapCmd(id, send, len);
}

void GuildScene::treasure_action_open_pre(SceneUser* pUser, TreasureActionGuildCmd& cmd)
{
  DWORD dwTreasureID = 0;
  if (m_eTreasureType == EGUILDTREASURETYPE_GVG)
  {
    if (m_mapTreasure.empty() == true)
    {
      MsgManager::sendDebugMsg(pUser->id, "测试log:没有gvg宝箱");
      XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "打开gvg宝箱失败,index" << m_dwTreasureIndex << "不存在" << XEND;
      return;
    }
    dwTreasureID = m_mapTreasure.begin()->first;
  }
  else if (m_eTreasureType == EGUILDTREASURETYPE_GUILD_BCOIN || m_eTreasureType == EGUILDTREASURETYPE_GUILD_ASSET)
  {
    if (m_vecCurTreasureCFG.empty() == true)
    {
      MsgManager::sendDebugMsg(pUser->id, "测试log:没有公会宝箱");
      XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "打开公会宝箱失败,未收集到公会宝箱配置" << XEND;
      return;
    }

    if (m_dwTreasureIndex >= m_vecCurTreasureCFG.size())
    {
      MsgManager::sendDebugMsg(pUser->id, "测试log:公会宝箱index异常");
      XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "打开公会宝箱失败,index" << m_dwTreasureIndex << "不存在" << XEND;
      return;
    }

    const SGuildTreasureCFG& rTreasureCFG = m_vecCurTreasureCFG[m_dwTreasureIndex];
    dwTreasureID = rTreasureCFG.dwID;

    // check city
    if (rTreasureCFG.dwCityID != 0)
    {
      const GuildCityInfo* pInfo = GuildCityManager::getMe().getRealCityInfoByGuild(m_oGuild.id());
      if (pInfo == nullptr || pInfo->flag() != rTreasureCFG.dwCityID)
      {
        MsgManager::sendMsg(pUser->id, 4041);
        XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "打开公会宝箱进行了城池检查" << XEND;
        return;
      }
    }

    const GuildSMember* pMember = pUser->getGuild().getMember(pUser->id);
    if (pMember == nullptr)
    {
      XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用BCoin打开公会宝箱失败,获取GuildSMember失败" << XEND;
      return;
    }

    DWORD weekstart = xTime::getWeekStart(now(), 5 * 3600);
    if(pMember->entertime() >= weekstart)
    {
      MsgManager::sendMsg(pUser->id, 4043);
      return;
    }

    const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
    if (rTreasureCFG.eType == EGUILDTREASURETYPE_GUILD_BCOIN)
    {
      DWORD dwPrice = MiscConfig::getMe().getGuildCFG().getBCoinPrice(m_dwBCoinCount + 1);
      if (dwPrice == DWORD_MAX)
      {
        MsgManager::sendDebugMsg(pUser->id, "测试log:获取BCOIN价格失败");
        XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用BCoin打开公会宝箱失败,获取BCoin数量失败" << XEND;
        return;
      }
      if (pUser->checkMoney(EMONEYTYPE_LOTTERY, dwPrice) == false)
      {
        MsgManager::sendDebugMsg(pUser->id, "测试log:BCOIN不足");
        XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用BCoin打开公会宝箱失败,BCoin数量不足" << XEND;
        return;
      }

      QWORD qwCostDeposit = (QWORD)dwPrice * 10000;
      //check deposit
      if(!pUser->getDeposit().checkQuota(qwCostDeposit))
      {
        XERR << "[公会宝箱-操作] 赠送额度不够" << pUser->accid << pUser->id << pUser->name << "cost"<< qwCostDeposit << "left" << pUser->getDeposit().getQuota() << XEND;
        return;
      }

      if (m_dwBCoinCount >= rCFG.mapTreasureBCoin.size())
      {
        MsgManager::sendDebugMsg(pUser->id, "测试log:BCOIN获取上限");
        XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用BCoin打开公会宝箱失败,超过领取上限" << XEND;
        return;
      }
      ++m_dwBCoinCount;
      pUser->subMoney(EMONEYTYPE_LOTTERY, dwPrice, ESOURCE_GUILD_TREASURE);
      pUser->getDeposit().subQuota(qwCostDeposit, EQuotaType_C_GuildBox);
    }
    else if (rTreasureCFG.eType == EGUILDTREASURETYPE_GUILD_ASSET)
    {
      if (m_dwAssetCount >= rCFG.mapTreasureAsset.size())
      {
        MsgManager::sendDebugMsg(pUser->id, "测试log:ASSET获取上限");
        XERR << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用资金打开公会宝箱失败,超过领取上限" << XEND;
        return;
      }
      ++m_dwAssetCount;
    }
  }

  if (dwTreasureID == 0)
    return;

  stringstream sstr;
  sstr << "guild cmd=gvg action=treasure_open id=" << dwTreasureID << " guildid=" << m_oGuild.id();
  GMCommandRuler::getMe().execute(pUser, sstr.str());
}

void GuildScene::treasure_action_open(SceneUser* pUser, TreasureActionGuildCmd& cmd)
{
  if (pUser == nullptr)
    return;

  if (m_eTreasureType == EGUILDTREASURETYPE_GVG)
  {
    if (m_mapTreasure.empty())
      cmd.clear_treasure();
    else
    {
      cmd.mutable_treasure()->set_id(m_mapTreasure.begin()->first);
      cmd.mutable_treasure()->set_count(m_mapTreasure.begin()->second);
    }
  }
  else
  {
    if (m_vecCurTreasureCFG.empty() == true)
    {
      cmd.clear_treasure();
    }
    else
    {
      if (m_dwTreasureIndex >= m_vecCurTreasureCFG.size())
        cmd.mutable_treasure()->set_id(m_vecCurTreasureCFG.begin()->dwID);
      else
        cmd.mutable_treasure()->set_id(m_vecCurTreasureCFG[m_dwTreasureIndex].dwID);
      cmd.mutable_treasure()->set_count(1);
    }
  }

  cmd.set_guild_treasure_count(m_dwAssetCount);
  cmd.set_bcoin_treasure_count(m_dwBCoinCount);

  PROTOBUF(cmd, send, len);
  MsgManager::sendMapCmd(id, send, len);
  XLOG << "[公会宝箱-操作] 公会" << m_oGuild.id() << m_oGuild.name() << "场景中" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行" << cmd.ShortDebugString() << "成功" << XEND;
}

void GuildScene::onClose()
{
  GuildMusicBoxManager::getMe().onSceneClose(m_oGuild.id());
}

//item image scene
ItemImageScene::ItemImageScene(DWORD sID, const char* name, const SceneBase* pBase, const SRaidCFG* pRaidCFG, xPos pos, DWORD npcId) :
  DScene(sID, name, pBase, pRaidCFG),
  m_centerPos(pos),
  m_dwNpcId(npcId)
{
}

bool ItemImageScene::init()
{
  if (DScene::init() == false)
    return false;

  createNpc();
  return true;
}

void ItemImageScene::createNpc()
{  
  //创建特效npc
  NpcDefine oDefine;
  oDefine.setID(m_dwNpcId);
  oDefine.resetting(); 
  oDefine.setPos(m_centerPos);
  // 设置不可点选
  oDefine.setBehaviours(oDefine.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);

  SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, this);
  if (pNpc)
  {
    UserActionNtf message;
    message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
    message.set_value(1001);
    message.set_charid(pNpc->id);
    pNpc->setGearStatus(1002);
    PROTOBUF(message, send, len);
    pNpc->sendCmdToNine(send, len);
    m_pNpc = pNpc;
  }
  ////创建爱心npc
  //NpcDefine oDef;
  //oDef.setID(MiscConfig::getMe().getItemImageCFG().dwLoveNpcId);
  //oDef.resetting();
  //xPos newPos;
  //this->getRandPos(m_centerPos, 2.0f, newPos);
  //oDef.setPos(newPos);
  //// 设置不可点选
  //oDef.setBehaviours(oDef.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);
  //SceneNpcManager::getMe().createNpc(oDef, this);
}

void ItemImageScene::entryAction(QWORD curMSec)
{
  DWORD dwNow = curMSec / ONE_THOUSAND;
  if (dwNow + 10 > m_dwCloseTime)
  {
    if (!m_bNtfed)
    {
      MsgManager::sendMapMsg(this->id, 73, MsgParams(10), EMESSAGETYPE_TIME_DOWN);
      m_bNtfed = true;
    }
  }
  DScene::entryAction(curMSec);
}

//************************************************************************
//********************************MatchScene******************************
//************************************************************************
MatchScene::MatchScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) : DScene(sID, sName, sBase, pRaidCFG)
{

}

MatchScene::~MatchScene()
{

}

bool MatchScene::init()
{
  if (DScene::init() == false)
    return false;

  // to do
  if (!base)
  {
    return false;
  }
  const SceneObject*  pSceneObject = base->getSceneObject(getRaidID());
  if (!pSceneObject)
  {
    return false;
  }
  m_mapPos.clear();
  const map<DWORD, SBornPoint>& bornlist = pSceneObject->getBornPointList();
  for (auto &m : bornlist)
  {
    m_mapPos[m.first] = m.second.oPos;
  }
  ntfMatchOpenRoom();

  return true;
}

void MatchScene::ntfMatchCloseRoom()
{
  SyncRaidSceneMatchSCmd cmd;
  cmd.set_open(false);
  cmd.set_sceneid(id);
  cmd.set_zoneid(thisServer->getZoneID());
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmdToSession(send, len);
  XLOG << "[斗技场-关闭] 通知匹配服关闭房间 sceneid"<<id <<"ret"<<ret << XEND;
}

void MatchScene::ntfMatchOpenRoom()
{
  SyncRaidSceneMatchSCmd cmd;
  cmd.set_open(true);
  cmd.set_sceneid(id);
  cmd.set_zoneid(thisServer->getZoneID());
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmdToSession(send, len);
  XLOG << "[斗技场-开始] 通知匹配服 sceneid" << id << "ret" << ret << XEND;
}

// pvp
PvpBaseScene::PvpBaseScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG):MatchScene(sID, sName, sBase, pRaidCFG)
{

}

PvpBaseScene::~PvpBaseScene()
{

}

bool PvpBaseScene::init()
{
  if (MatchScene::init() == false)
    return false;

  XDBG << "[斗技场-加载出生点] mapid"<<getMapID()<<"raidid"<<getRaidID()<<"possize" << m_mapPos.size() << XEND;
  return true;
}

bool PvpBaseScene::isHideUser()
{
  // test
  //return true;
  return false;
}

void PvpBaseScene::checkCombo(QWORD id, DWORD curSec)
{
  auto m = m_mapUserShow.find(id);
  if(m == m_mapUserShow.end())
    return;

  DWORD combotime = MiscConfig::getMe().getArenaCFG().dwComboTime;
  if(m->second.dwLastKillTime != 0 && m->second.dwComboCount != 0 && m->second.dwLastKillTime + combotime < curSec)
  {
    m->second.dwComboCount = 0;
    comboNotify(id,0);
  }
}

void PvpBaseScene::onKillUser(SceneUser* pKiller, SceneUser* pDeath)
{
  if (m_isOver)
    return;

  if (pKiller == nullptr || pKiller->getAttr(EATTRTYPE_MAXHP) == 0 || pDeath == nullptr)
    return;

  auto attacker = m_mapUserShow.find(pKiller->id);
  auto sufferer = m_mapUserShow.find(pDeath->id);
  if(attacker == m_mapUserShow.end() || sufferer == m_mapUserShow.end())
    return;

  //处理击杀者
  {
    MsgManager::sendMsg(pKiller->id, 964,MsgParams(pDeath->name));
    attacker->second.dwKillCount += 1;
    DWORD curSec = xTime::getCurSec();
    DWORD helptime = MiscConfig::getMe().getArenaCFG().dwHelpTime;
    DWORD combotime = MiscConfig::getMe().getArenaCFG().dwComboTime;
    DWORD braverhp = MiscConfig::getMe().getArenaCFG().dwBraverHP;
    DWORD saviorhp = MiscConfig::getMe().getArenaCFG().dwSaviorHP;
    DWORD HPpercent = static_cast<DWORD>(pKiller->getAttr(EATTRTYPE_HP) / pKiller->getAttr(EATTRTYPE_MAXHP) * 100);

    getPvpCoin(pKiller, EPVPREWARD_KILL);
    pKiller->getServant().onFinishEvent(ETRIGGER_PVP_KILL);
    pKiller->getServant().onGrowthFinishEvent(ETRIGGER_PVP_KILL);

    //协助
    auto assistKiller = m_mapHeal.find(pKiller->id);
    if (assistKiller != m_mapHeal.end())
    {
      for(auto m = assistKiller->second.begin(); m != assistKiller->second.end(); ++m)
      {
        if(m->first != pKiller->id && m->first != pDeath->id && m->second+helptime >= curSec)
        {
          auto fighter = m_mapUserShow.find(m->first);
          if(fighter == m_mapUserShow.end())
            continue;

          MsgManager::sendMsg(m->first, 965,MsgParams(pKiller->name, pDeath->name));
          fighter->second.dwHelpCount += 1;
          SceneUser* user = SceneUserManager::getMe().getUserByID(fighter->first);
          if (user)
            getPvpCoin(user, EPVPREWARD_HELPKILL);
          //救世主
          if(HPpercent < saviorhp)
          {
            fighter->second.dwSaviorCount += 1;
            MsgManager::sendMsg(m->first, 967, MsgParams(pKiller->name));
          }
        }
      }
    }

    auto assistDeath = m_mapDamage.find(pDeath->id);
    if (assistDeath != m_mapDamage.end())
    {
      for(auto m = assistDeath->second.begin(); m != assistDeath->second.end(); ++m)
      {
        if(assistKiller != m_mapHeal.end())
        {
          auto exist = assistKiller->second.find(m->first);
          if(exist != assistKiller->second.end() && exist->second+helptime >= curSec)
            continue;
        }

        if(m->first != pKiller->id && m->first != pDeath->id && m->second+helptime >= curSec)
        {
          auto fighter = m_mapUserShow.find(m->first);
          if(fighter == m_mapUserShow.end())
            continue;

          SceneUser* user = SceneUserManager::getMe().getUserByID(fighter->first);
          if (user)
            getPvpCoin(user, EPVPREWARD_HELPKILL);

          MsgManager::sendMsg(m->first, 965, MsgParams(pKiller->name, pDeath->name));
          fighter->second.dwHelpCount += 1;

          //救世主
          if(HPpercent < saviorhp)
          {
            fighter->second.dwSaviorCount += 1;
            MsgManager::sendMsg(m->first, 967, MsgParams(pKiller->name));
          }
        }
      }
    }

    //combo
    if(attacker->second.dwLastKillTime != 0 && attacker->second.dwLastKillTime + combotime >= curSec)
    {
      attacker->second.dwComboCount += 1;
      attacker->second.dwTotalCombo += 1;
      comboNotify(pKiller->id,attacker->second.dwComboCount);
    }
    attacker->second.dwLastKillTime = curSec;

    //无畏者
    if(HPpercent < braverhp)
    {
      attacker->second.dwBraveCount += 1;
      MsgManager::sendMsg(pKiller->id, 968, MsgParams(pDeath->name));
    }
  }

  //处理被击杀者
  {
    MsgManager::sendMsg(pDeath->id, 963,MsgParams(pKiller->name));
    sufferer->second.dwBeKillCount += 1;
    sufferer->second.dwLastKillTime = 0;
    m_mapDamage.erase(pDeath->id);
    m_mapHeal.erase(pDeath->id);
  }
}

void PvpBaseScene::addDamageUser(QWORD attackID, QWORD suffererID, DWORD damage)
{
  if (m_isOver)
    return;

  QWORD curSec = xTime::getCurSec();
  m_mapDamage[suffererID][attackID] = curSec;
  m_mapUserShow[attackID].dwDamage += damage;
}

void PvpBaseScene::addHealUser(QWORD id, QWORD healID, DWORD hp)
{
  if (m_isOver)
    return;
  QWORD curSec = xTime::getCurSec();
  m_mapHeal[id][healID] = curSec;
  m_mapUserShow[healID].dwHealHp += hp;
}

void PvpBaseScene::comboNotify(QWORD id, DWORD num)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(id);
  if(pUser == nullptr)
    return;

  ComboNotifyCCmd cmd;
  cmd.set_combonum(num);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  XDBG << "[斗技场-连击]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << num << XEND;
}

void PvpBaseScene::backOriginMap(SceneUser *user)
{
  if(user == nullptr)
    return;

  user->relive(ERELIVETYPE_RETURN);

  DWORD tomapid;
  xPos topos(0,0,0);
  tomapid = user->getUserMap().getLastStaticMapID();
  topos = user->getUserMap().getLastStaticMapPos();
  user->gomap(tomapid, GoMapType::KickUser, topos);
}

void PvpBaseScene::entryAction(QWORD curMSec)
{
  MatchScene::entryAction(curMSec);

  if (m_setKickPvpList.empty() == false)
  {
    for (auto &q : m_setKickPvpList)
    {
      SceneUser* user = SceneUserManager::getMe().getUserByID(q);
      if (user && user->getScene() == this)
        backOriginMap(user);
    }
    m_setKickPvpList.clear();
  }
}

void PvpBaseScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  MatchScene::userEnter(user);
  user->getAchieve().onPvp(false);

  auto m = m_mapUserShow.find(user->id);
  if (m != m_mapUserShow.end())
  {
    m_mapUserShow.erase(m);
  }

  SPvpUserShow fUser;
  fUser.qwID = user->id;

  m_mapUserShow.insert(make_pair(user->id,fUser) );
  user->getServant().onGrowthFinishEvent(ETRIGGER_ENTER_PVP);
}

void PvpBaseScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  MatchScene::userLeave(user);

  auto m = m_mapUserShow.find(user->id);
  if (m != m_mapUserShow.end())
    m_mapUserShow.erase(m);

  auto damageUser = m_mapDamage.find(user->id);
  if(damageUser != m_mapDamage.end())
    m_mapDamage.erase(damageUser);

  auto healUser = m_mapHeal.find(user->id);
  if(healUser != m_mapHeal.end())
    m_mapHeal.erase(healUser);

  LeavePvpMap cmd;
  cmd.set_charid(user->id);
  cmd.set_originzoneid(user->getUserSceneData().getOriginalZoneID());
  PROTOBUF(cmd, send, len);

  thisServer->sendCmdToSession(send, len);
  
}

void PvpBaseScene::broadcastFinalScore()
{
  //QWORD maxKillCntUser = 0;
  DWORD tempMaxKillCnt = 0;

  //QWORD maxHelpCntUser = 0;
  DWORD tempMaxHelpCnt = 0;

  //QWORD maxComboCntUser = 0;
  DWORD tempMaxComboCnt = 0;

  //QWORD maxBraveCntUser = 0;
  DWORD tempMaxBraveCnt = 0;

  //QWORD maxSaviorCntUser = 0;
  DWORD tempMaxSaviorCnt = 0;

  DWORD tempMaxHealHp = 0;

  DWORD tempMaxDamage = 0;
  for (auto &m : m_mapUserShow)
  {
    if (m.second.dwKillCount > tempMaxKillCnt)
    {
      //maxKillCntUser = m.first;
      tempMaxKillCnt = m.second.dwKillCount;
    }
    if (m.second.dwHelpCount > tempMaxHelpCnt)
    {
      //maxHelpCntUser = m.first;
      tempMaxHelpCnt = m.second.dwHelpCount;
    }
    if (m.second.dwTotalCombo > tempMaxComboCnt)
    {
      //maxComboCntUser = m.first;
      tempMaxComboCnt = m.second.dwTotalCombo;
    }
    if (m.second.dwBraveCount > tempMaxBraveCnt)
    {
      //maxBraveCntUser = m.first;
      tempMaxBraveCnt = m.second.dwBraveCount;
    }
    if (m.second.dwSaviorCount > tempMaxSaviorCnt)
    {
      //maxSaviorCntUser = m.first;
      tempMaxSaviorCnt = m.second.dwSaviorCount;
    }
    if (m.second.dwHealHp > tempMaxHealHp)
    {
      tempMaxHealHp = m.second.dwHealHp;
    }
    if (m.second.dwDamage > tempMaxDamage)
    {
      tempMaxDamage = m.second.dwDamage;
    }
  }

  auto sendInfoToUser = [&](QWORD userid, const SPvpUserShow& info)
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(userid);
    if (user == nullptr || user->getScene() != this)
      return;

    MsgParams msg;
    
    msg.addSubString(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_KNOCK_DOWN_PLAYER)); msg.addSubNumber(info.dwKillCount); msg.addSubString(" (" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FIELD_HIGHEST) + " "); msg.addSubNumber(tempMaxKillCnt); msg.addSubString(")\n");

    msg.addSubString(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_ASSISTS) + "    "); msg.addSubNumber(info.dwHelpCount); msg.addSubString(" (" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FIELD_HIGHEST) + " "); msg.addSubNumber(tempMaxHelpCnt); msg.addSubString(")\n");

    msg.addSubString(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_COMBO) + "   "); msg.addSubNumber(info.dwTotalCombo); msg.addSubString(" (" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FIELD_HIGHEST) + " "); msg.addSubNumber(tempMaxComboCnt); msg.addSubString(")\n");

    msg.addSubString(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FEARLESS) + "  "); msg.addSubNumber(info.dwBraveCount); msg.addSubString(" (" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FIELD_HIGHEST) + " "); msg.addSubNumber(tempMaxBraveCnt); msg.addSubString(")\n");

    msg.addSubString(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_SAVIOR) + "  "); msg.addSubNumber(info.dwSaviorCount); msg.addSubString(" (" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FIELD_HIGHEST) + " "); msg.addSubNumber(tempMaxSaviorCnt); msg.addSubString(")\n");

    msg.addSubString(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_DAMAGE) + "   "); msg.addSubNumber(info.dwDamage); msg.addSubString(" (" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FIELD_HIGHEST) + " "); msg.addSubNumber(tempMaxDamage); msg.addSubString(")\n");

    msg.addSubString(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_EFFECTIVE_TREATMENT)); msg.addSubNumber(info.dwHealHp); msg.addSubString(" (" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_FIELD_HIGHEST) + " "); msg.addSubNumber(tempMaxHealHp); msg.addSubString(")\n");

    MsgManager::sendMsg(userid, 4001, msg);
  };

  for (auto &m : m_mapUserShow)
    sendInfoToUser(m.first, m.second);
}

void PvpBaseScene::getPvpCoin(SceneUser* pUser, EPvpRewardType eType)
{
  if (pUser == nullptr)
    return;

  const SPvpCommonCFG& rCFG = MiscConfig::getMe().getPvpCommonCFG();
  auto giveCoin = [&](DWORD orinum)
  {
    DWORD daycoin = pUser->getVar().getVarValue(EVARTYPE_PVPCOIN_DAY);
    DWORD weekcoin = pUser->getVar().getVarValue(EVARTYPE_PVPCOIN_WEEK);
    DWORD daymax = rCFG.dwDayMaxCoin;
    DWORD weekmax = rCFG.dwWeekMaxCoin;
    if (daycoin >= daymax || weekcoin >= weekmax)
      return;

    orinum = daymax - daycoin >= orinum ? orinum : daymax - daycoin;
    orinum = weekmax - weekcoin >= orinum ? orinum : weekmax - weekcoin;
    if (orinum == 0)
      return;
    pUser->getUserSceneData().setPvpCoin(pUser->getUserSceneData().getPvpCoin() + orinum);
    pUser->getVar().setVarValue(EVARTYPE_PVPCOIN_DAY, daycoin + orinum);
    pUser->getVar().setVarValue(EVARTYPE_PVPCOIN_WEEK, weekcoin + orinum);
    MsgParams param;
    param.addNumber(150);
    param.addNumber(150);
    param.addNumber(orinum);
    MsgManager::sendMsg(pUser->id, 6, param);

    DWORD neednum = rCFG.dwExtraRewardCoinNum;
    if (neednum != 0)
    {
      if (daycoin / neednum < (daycoin + orinum) / neednum)
      {
        DWORD rate = (daycoin + orinum) / neednum - daycoin / neednum;
        ItemInfo info;
        info.set_id(rCFG.paExtraItem2Num.first);
        info.set_count(rCFG.paExtraItem2Num.second * rate);
        info.set_source(ESOURCE_PVP);
        if (info.count())
        {
          pUser->getPackage().addItem(info, EPACKMETHOD_AVAILABLE);
          XLOG << "[PVP-额外奖励获得], 玩家:" << pUser->name << pUser->id << "道具:" << info.id() << info.count() << XEND;
        }
      }
    }
    XLOG << "[PVP-斗币获得],玩家:" << pUser->name << pUser->id << " 获得数量:" << orinum << "当前总数量:" << pUser->getUserSceneData().getPvpCoin() << "今日获得:" << daycoin + orinum << "本周获得:" << weekcoin + orinum << XEND;
  };

  switch (eType)
  {
    case EPVPREWARD_KILL:
      giveCoin(rCFG.dwKillCoin);
      break;
    case EPVPREWARD_HELPKILL:
      giveCoin(rCFG.dwHelpKillCoin);
      break;
    case EPVPREWARD_DESERTWIN:
      giveCoin(rCFG.dwDesertWinCoin);
      break;
    case EPVPREWARD_GLAMWIN:
      giveCoin(rCFG.dwGlamWinCoin);
      break;
    default:
      break;
  }
}

// pvp glam metal
GlamMetalScene::GlamMetalScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG):PvpBaseScene(sID, sName, sBase, pRaidCFG)
{
  m_dwMetalNpcHpPer = 100;
}

GlamMetalScene::~GlamMetalScene()
{

}

void GlamMetalScene::userEnter(SceneUser* user)
{
  if (!user)
    return;

  PvpBaseScene::userEnter(user);

  auto it = m_setOnceIn.find(user->id);
  if (it == m_setOnceIn.end())
  {
    MsgManager::sendMsg(user->id, 974);
  }  
  m_setOnceIn.insert(user->id);
  notify(user);
  
  if (m_isOver)
  {
    user->m_oBuff.del(120244);
  }
}

void GlamMetalScene::userLeave(SceneUser *user)
{
  if (!user)
    return;

  PvpBaseScene::userLeave(user);
  user->m_oBuff.del(120244);
}

void GlamMetalScene::notify(SceneUser* user)
{
  if (user == nullptr)
    return;

  // send trace frame
  NtfFightStatCCmd cmd;
  cmd.set_pvp_type(EPVPTYPE_HLJS);
  cmd.set_starttime(m_dwStartTime);
  cmd.set_remain_hp(m_dwMetalNpcHpPer);

  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

void GlamMetalScene::notifyAllUser()
{
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  for (auto &s : userSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user)
      notify(user);
  }
}

/*void GlamMetalScene::userLeave(SceneUser* user)
{
  // del trace frame
}*/

bool GlamMetalScene::init()
{
  if (PvpBaseScene::init() == false)
    return false;

  const SPvpCFG* pCfg = MiscConfig::getMe().getPvpCfg(EPVPTYPE_HLJS);
  if (!pCfg)
  {
    XERR << "[斗技场-华丽金属] 找不到pvp配置" << XEND;
    return false;
  }
  m_dwStartTime = now();
  m_dwEndTime = m_dwStartTime + pCfg->dwDuration;

  XLOG << "[斗技场-华丽金属] 倒计时时间starttime" << m_dwStartTime << "endtime" << m_dwEndTime << pCfg->dwDuration << XEND;
  return true;
}

void GlamMetalScene::entryAction(QWORD curMSec)
{
  PvpBaseScene::entryAction(curMSec);
  DWORD cur = curMSec / ONE_THOUSAND;
  
  if (m_isOver)
    return;

  if (cur >= m_dwCheckRefreshTime)
  {
    m_dwCheckRefreshTime = cur + 1;

    if (m_qwGlamMetalGuid == 0)
    {
      std::list<SceneNpc*> npclist;
      DWORD baseid = MiscConfig::getMe().getArenaCFG().dwMetalNpcID;
      Scene::getSceneNpcByBaseID(baseid, npclist);
      if (npclist.empty())
        return ;

      SceneNpc* npc = *(npclist.begin());
      if (npc == nullptr)
        return ;
      m_qwGlamMetalGuid = npc->id;
    }

    SceneNpc* pMetalNpc = SceneNpcManager::getMe().getNpcByTempID(m_qwGlamMetalGuid);
    if (pMetalNpc)
    {
      DWORD hpper = m_dwMetalNpcHpPer;
      float maxhp = pMetalNpc->getAttr(EATTRTYPE_MAXHP);
      if (maxhp)
      {
        float tmp = pMetalNpc->getAttr(EATTRTYPE_HP) * 100 / maxhp;
        hpper = ceil(tmp);        
      }
      if(hpper != m_dwMetalNpcHpPer)
      {
        m_dwMetalNpcHpPer = hpper;
        notifyAllUser();
      }
    }
  }

  if (m_calcState == ECALCSTATE_WIN && m_dwSendWinResultTime && cur > m_dwSendWinResultTime)
  {
    sendWinResult();
    return;
  }
  
  if (m_calcState == ECALCSTATE_FIGHTING && m_dwEndTime && cur > m_dwEndTime)
  {    
    xSceneEntrySet userSet;
    getAllEntryList(SCENE_ENTRY_USER, userSet);
    for (auto&s : userSet)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      {
        MsgManager::sendMsg(user->id, 977);
      }
    }
    m_dwSendTimerOverTime = cur + 5;
    m_calcState = ECALCSTATE_TIMEOUT;
    XDBG << "[斗技场-华丽金属-超时消息] killerid" << m_killerId << "977" << "m_dwSendTimerOverTime" << m_dwSendTimerOverTime << XEND;
    m_dwEndTime = 0;
  }

  if (m_calcState == ECALCSTATE_TIMEOUT  && m_dwSendTimerOverTime && cur > m_dwSendTimerOverTime)
  {
    sendTimeOverResult();
    m_dwSendTimerOverTime = 0;
  }
}

void GlamMetalScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  if (m_calcState != ECALCSTATE_FIGHTING)
    return;

  if (npc && npc->id == m_qwGlamMetalGuid)
  {
    notifyAllUser();

    xSceneEntrySet userSet;
    getAllEntryList(SCENE_ENTRY_USER, userSet);

    // 找到有效击杀者
    SceneUser* pKillUser = dynamic_cast<SceneUser*> (killer);
    if (pKillUser == nullptr)
    {
      for (auto &s : userSet)
      {
        SceneUser* user = dynamic_cast<SceneUser*> (s);
        if (user && user->isMyTeamMember(npc->m_ai.getLastAttacker()))
        {
          pKillUser = user;
          break;
        }
      }
      if (pKillUser == nullptr && !userSet.empty())
        pKillUser = dynamic_cast<SceneUser*> (*(userSet.begin()));
    }
    if (pKillUser == nullptr)
      return;

    m_killerId = pKillUser->id;
    //msg
    MsgParams params;
    params.addString(pKillUser->getName());
    params.addString(pKillUser->getTeam().getName());
    
    for (auto&s : userSet)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      {      
        MsgManager::sendMsg(user->id, 975, params);
      }
    }
    m_dwSendWinResultTime = now() + 5;  
    m_calcState = ECALCSTATE_WIN;
    XDBG << "[斗技场-华丽金属-胜利消息] killerid" << m_killerId << "975" << "end time" << m_dwSendWinResultTime << XEND;

   m_bIsNpcDie = true;
  }
}

void GlamMetalScene::getBornPos(SceneUser* pUser, xPos& pos)
{
  if (!pUser)
    return;
  DWORD index = pUser->getUserZone().getColorIndex();
  xPos* ppos = nullptr;
  xPos tempPos;
  do
  {
    auto it = m_mapTeamPos.find(index);
    if (it != m_mapTeamPos.end())
    {
      ppos = &(it->second);
      break;
    }

    if (m_mapPos.empty())
      break;

    std::vector<DWORD> vecPos;
    vecPos.reserve(m_mapPos.size());

    for (auto &m : m_mapPos)
    {
      vecPos.push_back(m.first);
    }
    std::random_shuffle(vecPos.begin(), vecPos.end());

    if (vecPos.begin() == vecPos.end())
      break;
    DWORD offset = *(vecPos.begin());
    tempPos = m_mapPos[offset];
    m_mapPos.erase(offset);
    ppos = &tempPos;
    m_mapTeamPos[index] = *ppos;
  } while (0);

  if (ppos == nullptr)
  {
    getRandPos(tempPos);
    ppos = &tempPos;
    m_mapTeamPos[index] = *ppos;
  }
  pos = *ppos;

  XDBG << "[斗技场-获取出生点-华丽金属]" << pUser->getTempID() << pUser->getName() << "index" << index << "x" << pos.getX() << "y" << pos.getY() << "z" << pos.getZ() << XEND;
}

void GlamMetalScene::sendWinResult()
{
  PvpResultCCmd cmdsuc;
  cmdsuc.set_type(EPVPTYPE_HLJS);
  cmdsuc.set_result(EPVPRESULT_SUCCESS);
  PROTOBUF(cmdsuc, sendsuc, lensuc);

  PvpResultCCmd cmdfail;
  cmdfail.set_type(EPVPTYPE_HLJS);
  cmdfail.set_result(EPVPRESULT_FAIL);
  PROTOBUF(cmdfail, sendfail, lenfail);

  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  for (auto&s : userSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user->isMyTeamMember(m_killerId))
    {
      user->sendCmdToMe(sendsuc, lensuc);
      getPvpCoin(user, EPVPREWARD_GLAMWIN);
      user->getAchieve().onPvp(true);
    }
    else
    {
      user->sendCmdToMe(sendfail, lenfail);
    }
  }
  m_dwSendWinResultTime = 0;
  setEnd();
  m_calcState = ECALCSTATE_WIN_END;
}

void GlamMetalScene::sendTimeOverResult()
{
  if (!m_bIsNpcDie)
  {
    //规定时间没杀死npc所有人失败
    xSceneEntrySet userSet;
    getAllEntryList(SCENE_ENTRY_USER, userSet);

    PvpResultCCmd cmdfail;
    cmdfail.set_type(EPVPTYPE_HLJS);
    cmdfail.set_result(EPVPRESULT_FAIL);
    PROTOBUF(cmdfail, sendfail, lenfail);
    for (auto&s : userSet)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (user)
        user->sendCmdToMe(sendfail, lenfail);
    }
  }
  setEnd();
  m_calcState = ECALCSTATE_TIMEOUT_END;
}

void GlamMetalScene::setEnd()
{ 
  broadcastFinalScore();
  setCloseTime(now() + 30);  
  ntfMatchCloseRoom();

  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  for (auto&s : userSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (!user)
      continue;
    user->m_oBuff.add(120244);
    SceneNpc* pet = user->getUserPet().getPetNpc();
    if (pet)
    {
      pet->m_ai.setCurLockID(0);
      pet->m_blGod = true;
    }
    SceneNpc* being = user->getUserBeing().getCurBeingNpc();
    if (being)
    {
      being->m_ai.setCurLockID(0);
      being->m_blGod = true;
    }
  }
  m_isOver = true;

}

void GlamMetalScene::addNpcDamage(QWORD userid, DWORD damage)
{
  m_mapUserShow[userid].dwDamage += damage;
}
/************************************************************************/
/*MonkeyScene                                                                      */
/************************************************************************/
MonkeyScene::MonkeyScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG):PvpBaseScene(sID, sName, sBase, pRaidCFG)
{

}

bool MonkeyScene::init()
{
  return DScene::init();
}

void MonkeyScene::userEnter(SceneUser* user)
{
  if (user == nullptr)
    return;

  PvpBaseScene::userEnter(user);

  DWORD score = MiscConfig::getMe().getArenaCFG().dwOriginScore;
  m_mapScore.insert(make_pair(user->id,score));
  notifyAllUser();
  syncSceneUserCount();
  XLOG << "[斗技场-进入溜溜猴]" << user->accid << user->id << user->getProfession() << user->name << score << XEND;
}

void MonkeyScene::userLeave(SceneUser* user)
{
  if (user == nullptr)
    return;

  PvpBaseScene::userLeave(user);

  auto m = m_mapScore.find(user->id);
  if(m != m_mapScore.end())
    m_mapScore.erase(m);
  notifyAllUser();
  syncSceneUserCount();
}

void MonkeyScene::syncSceneUserCount()
{
  SyncRaidSceneMatchSCmd cmd;
  cmd.set_sceneid(id);
  cmd.set_open(true);
  cmd.set_count(m_mapScore.size());
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmdToSession(send, len);
  XLOG << "[斗技场-进入溜溜猴] 同步场景人数到匹配服 sceneid" << id << "当前场景人数" << m_mapScore.size() << "ret" << ret << XEND;
}

void MonkeyScene::onKillUser(SceneUser* pKiller, SceneUser* pDeath)
{
  if (pKiller == nullptr || pDeath == nullptr)
    return;

  PvpBaseScene::onKillUser(pKiller,pDeath);

  for(auto m = m_mapScore.begin(); m != m_mapScore.end(); ++m)
    calculateScore(m->first);

  auto deadUser = m_mapScore.find(pDeath->id);
  if(deadUser != m_mapScore.end() && deadUser->second == 0)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(pDeath->id);
    if(pUser != nullptr)
    {
      addPvpKickUser(pUser->id);
      //backOriginMap(pUser);
      XDBG << "[斗技场-退出溜溜猴]" << "被击杀" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << XEND;
    }
  }

  XDBG << "[斗技场-死亡]" << "击杀者" << pKiller->accid << pKiller->id << pKiller->getProfession() << pKiller->name <<
              "被击杀者:" << pDeath->accid << pDeath->id << pDeath->getProfession()<< XEND;
}

void MonkeyScene::calculateScore(QWORD id)
{
  auto fighter = m_mapUserShow.find(id);
  auto userScore = m_mapScore.find(id);
  if(fighter == m_mapUserShow.end() || userScore == m_mapScore.end())
    return;

  DWORD oldScore = userScore->second;
  DWORD score = 0;
  DWORD addScore = 0;
  DWORD subScore = 0;
  DWORD killscore = MiscConfig::getMe().getArenaCFG().dwKillScore;
  DWORD bekilledscore = MiscConfig::getMe().getArenaCFG().dwBeKilledScore;
  DWORD helpscore = MiscConfig::getMe().getArenaCFG().dwHelpScore;
  DWORD braverscore = MiscConfig::getMe().getArenaCFG().dwBraverScore;
  DWORD comboscore = MiscConfig::getMe().getArenaCFG().dwComboScore;
  DWORD saviorscore = MiscConfig::getMe().getArenaCFG().dwSaviorScore;
  DWORD originscore = MiscConfig::getMe().getArenaCFG().dwOriginScore;
  addScore = originscore + fighter->second.dwKillCount*killscore + fighter->second.dwHelpCount*helpscore + fighter->second.dwBraveCount*braverscore
            + fighter->second.dwTotalCombo*comboscore + fighter->second.dwSaviorCount*saviorscore;
  subScore = fighter->second.dwBeKillCount*bekilledscore;

  if(addScore > subScore)
    score = addScore - subScore;
  else
    MsgManager::sendMsg(id, 953);

  userScore->second = score;
  if(score != oldScore && score != 0)
  {
    notify(id);
    XDBG << "[斗技场-积分变化]" << id << oldScore << score << XEND;
  }
}

void MonkeyScene::notify(QWORD id)
{
  auto m = m_mapScore.find(id);
  if(m == m_mapScore.end())
    return;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(id);
  if(pUser == nullptr)
    return;

  NtfFightStatCCmd cmd;
  cmd.set_pvp_type(EPVPTYPE_LLH);
  cmd.set_player_num(m_mapScore.size());
  cmd.set_score(m->second);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
}

void MonkeyScene::notifyAllUser()
{
  for(auto m = m_mapScore.begin(); m != m_mapScore.end(); ++m)
  {
    if(m->second != 0)
      notify(m->first);
  }
}

void MonkeyScene::getBornPos(SceneUser* pUser, xPos& pos)
{
  if (!pUser)
    return;
  getRandPos(pos);
  XDBG << "[斗技场-获取出生点-溜溜猴]" << pUser->getTempID() << pUser->getName()  << "x" << pos.getX() << "y" << pos.getY() << "z" << pos.getZ() << XEND;
}


/************************************************************************/
/*DesertWolfScene                                                                      */
/************************************************************************/
DesertWolfScene::DesertWolfScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :PvpBaseScene(sID, sName, sBase, pRaidCFG)
{
}

DesertWolfScene::~DesertWolfScene()
{
}

bool DesertWolfScene::init()
{
  PvpBaseScene::init();
    
  const SPvpCFG* pCfg = MiscConfig::getMe().getPvpCfg(EPVPTYPE_SMZL);
  if (!pCfg)
  {
    XERR <<"[斗技场-沙漠之狼] 找不到pvp配置"<< XEND;
    return false;
  }
  m_dwMaxScore = pCfg->dwMaxScore;
  m_dwStartTime = now();
  m_dwEndTime = m_dwStartTime + pCfg->dwDuration;
  XLOG << "[斗技场-沙漠之狼] 倒计时时间starttime" << m_dwStartTime << "endtime" << m_dwEndTime << pCfg->dwDuration << XEND;
  return true;
}

void DesertWolfScene::entryAction(QWORD curMSec)
{
  PvpBaseScene::entryAction(curMSec);

  if (m_isOver)
    return;

  DWORD cur = curMSec / ONE_THOUSAND;
  if (cur >= m_dwCheckRefreshTime)
  {
    m_dwCheckRefreshTime = cur + 1;
    
    if (m_calcState == ECALCSTATE_WIN && m_dwShouResultTime && cur > m_dwShouResultTime)
    {
      return sendWinResult();
    }

    if (m_calcState == ECALCSTATE_FIGHTING && m_dwEndTime && cur > m_dwEndTime)
    {
      setEnd();      
    }
  }
}

void DesertWolfScene::userEnter(SceneUser* user)
{  
  if (user == nullptr)
    return;

  PvpBaseScene::userEnter(user);

  auto it = m_setOnceIn.find(user->id);
  if (it == m_setOnceIn.end())
  {
    MsgManager::sendMsg(user->id, 973);
  }
  m_setOnceIn.insert(user->id);

  notify(user);
  
  if (m_isOver)
  {
    user->m_oBuff.del(120244);
  }

  XDBG << "[斗技场-沙漠之狼-进入]" << user->accid << user->id << user->getProfession() << user->name << "teamid"<<user->getTeamID() << XEND;
}

void DesertWolfScene::userLeave(SceneUser* user)
{
  if (user == nullptr)
    return;
  PvpBaseScene::userLeave(user);

  std::set<QWORD> m_setTeam;
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  for (auto &s : userSet)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*> (s);
    if (pUser)
    {
      if (pUser == user)
        continue;
      if (pUser->getTeamID())
        m_setTeam.insert(pUser->getTeamID());
    }
  }
  
  if (m_calcState == ECALCSTATE_FIGHTING && m_setTeam.size() == 1)
  {
    setEnd(*(m_setTeam.begin()));
    m_calcState = ECALCSTATE_LEAVE_END;
  }

  if (m_isOver)
  {
    user->m_oBuff.del(120244);
  }

  XDBG << "[斗技场-沙漠之狼-离开]" << user->accid << user->id << user->getProfession() << user->name << "teamid" << user->getTeamID()<<"剩余队伍"<< m_setTeam.size() << XEND;
}

void DesertWolfScene::onKillUser(SceneUser* pKiller, SceneUser* pDeath)
{
  if (m_isOver)
    return;
  if (pKiller == nullptr || pDeath == nullptr)
    return;

  PvpBaseScene::onKillUser(pKiller, pDeath);
  
  addTeamScore(pKiller, 1);  
  XDBG << "[斗技场-沙漠之狼-击杀敌人]" << "击杀者" << pKiller->id <<"队伍id"<<pKiller->getTeamID() <<
    "被击杀者:" <<pDeath->id << "队伍id"<<pDeath->getTeamID() << XEND;
  
}

void DesertWolfScene::notify(SceneUser* pUser)
{
  if (!pUser)
    return;
  NtfFightStatCCmd cmd;
  cmd.set_pvp_type(EPVPTYPE_SMZL);
  cmd.set_starttime(m_dwStartTime);
  cmd.set_my_teamscore(getMyTeamScore(pUser));
  cmd.set_enemy_teamscore(getEnemyTeamScore(pUser));
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
}

void DesertWolfScene::notifyAllUser()
{
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  for (auto &s : userSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user)
      notify(user);
  }
}

DWORD DesertWolfScene::getMyTeamScore(SceneUser* pUser)
{
  if (!pUser)
    return 0;
  QWORD teamId = pUser->getTeamID();
  if (teamId == 0)
    return 0;
  
  auto it = m_mapTeamScore.find(teamId);
  if (it == m_mapTeamScore.end())
    return 0;
  
  return it->second;
}

DWORD DesertWolfScene::getEnemyTeamScore(SceneUser* pUser)
{
  if (!pUser)
    return 0;
  QWORD teamId = pUser->getTeamID();
  
  for (auto &m : m_mapTeamScore)
  {
    if (m.first != teamId)
      return m.second;
  }
  return 0;
}

void DesertWolfScene::addTeamScore(SceneUser* pUser, DWORD score)
{
  if (!pUser)
    return ;
  QWORD teamId = pUser->getTeamID();
  if (teamId == 0)
    return ;
  DWORD newScore = m_mapTeamScore[teamId] + score;
  m_mapTeamScore[teamId] = newScore;

  XDBG << "[斗技场-沙漠之狼] 队伍得分 charid" << pUser->id << pUser->name << "teamid" << teamId << "allscore" << m_mapTeamScore[teamId] << "score" << score << XEND;
  notifyAllUser();
  
  m_mapTeamName[teamId] = pUser->getTeam().getName();  
  if (m_calcState == ECALCSTATE_FIGHTING && newScore >= m_dwMaxScore)
  {
    setEnd();
  }
}

void DesertWolfScene::setEnd(QWORD winTeam/*=0*/)
{
  if (m_calcState != ECALCSTATE_FIGHTING)
    return;
  
  //find winner
  QWORD qwWinTeam = 0;
  DWORD dwMaxScore = 0;
  DWORD dwMinScore = 9999999;
  //平局
  bool bTie = false;
  if (winTeam == 0)
  {
    for (auto & m : m_mapTeamScore)
    {
      if (m.second >= dwMaxScore)
      {
        dwMaxScore = m.second;
        qwWinTeam = m.first;
        m_qwWinTeamId = qwWinTeam;
      }
      if (m.second <= dwMinScore)
      {
        dwMinScore = m.second;
      }
    }

    if (m_mapTeamScore.size() < 2)
      dwMinScore = 0;
    if (dwMaxScore == dwMinScore)
    {
      bTie = true;
    }
  }
  else
    qwWinTeam = winTeam;  
  
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  
  if (bTie == false && m_qwWinTeamId)
  {
    MsgParams params;

    string name;
    auto it = m_mapTeamName.find(m_qwWinTeamId);
    if (it != m_mapTeamName.end())
      name = it->second;
    params.addString(name);
    params.addNumber(dwMaxScore);
    params.addString(name);

    for (auto&s : userSet)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      {
        MsgManager::sendMsg(user->id, 976, params);
      }
    }
    m_dwShouResultTime = now() + 5;
    m_calcState = ECALCSTATE_WIN;
    XLOG << "[斗技场-沙漠之狼-战斗结果] 最高分" << dwMaxScore << "队伍id" << qwWinTeam << "m_qwWinTeamId" << m_qwWinTeamId << "低分" << dwMinScore << "是否平局" << bTie << "躺赢的队伍" << winTeam<<"5秒延迟时间"<< m_dwShouResultTime << XEND;
    return;
  }

  XLOG << "[斗技场-沙漠之狼-战斗结果] 最高分" << dwMaxScore << "队伍id" << qwWinTeam << "m_qwWinTeamId" << m_qwWinTeamId << "低分" << dwMinScore << "是否平局" << bTie << "躺赢的队伍" << winTeam << XEND;
  
  // 通知所有玩家结果
  PvpResultCCmd cmdsuc;
  cmdsuc.set_type(EPVPTYPE_SMZL);
  cmdsuc.set_result(EPVPRESULT_SUCCESS);
  PROTOBUF(cmdsuc, sendsuc, lensuc);

  PvpResultCCmd cmdfail;
  cmdfail.set_type(EPVPTYPE_SMZL);
  cmdfail.set_result(EPVPRESULT_FAIL);
  PROTOBUF(cmdfail, sendfail, lenfail);

  //平手
  PvpResultCCmd cmdtie;
  cmdtie.set_type(EPVPTYPE_SMZL);
  cmdtie.set_result(EPVPRESULT_TIE);
  PROTOBUF(cmdtie, sendtie, lentie);
  
  bool realwin = true;
  if (winTeam)
  {
    realwin = false;
    auto itwin = m_mapTeamScore.find(qwWinTeam);
    if (itwin != m_mapTeamScore.end() && itwin->second >= MiscConfig::getMe().getPvpCommonCFG().dwDesertWinScore)
      realwin = true;
  }

  for (auto&s : userSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (!user)
      continue;
    if (bTie)
    {
      user->sendCmdToMe(sendtie, lentie);
    }
    else
    {
      if (user->getTeamID() == qwWinTeam)
      {
        user->sendCmdToMe(sendsuc, lensuc);
        if (realwin)
          getPvpCoin(user, EPVPREWARD_DESERTWIN);
        user->getAchieve().onPvp(true);
      }
      else
        user->sendCmdToMe(sendfail, lenfail);
    } 
    user->m_oBuff.add(120244);
    SceneNpc* pet = user->getUserPet().getPetNpc();
    if (pet)
    {
      pet->m_ai.setCurLockID(0);
      pet->m_blGod = true;
    }
    SceneNpc* being = user->getUserBeing().getCurBeingNpc();
    if (being)
    {
      being->m_ai.setCurLockID(0);
      being->m_blGod = true;
    }
  }
  broadcastFinalScore();
  setCloseTime(now() + 30);
  ntfMatchCloseRoom();
  m_dwEndTime = 0;
  m_isOver = true;
  m_calcState = ECALCSTATE_TIMEOUT_END;
}

void DesertWolfScene::sendWinResult()
{
  if (!m_qwWinTeamId)
    return;

  // 通知所有玩家结果
  PvpResultCCmd cmdsuc;
  cmdsuc.set_type(EPVPTYPE_SMZL);
  cmdsuc.set_result(EPVPRESULT_SUCCESS);
  PROTOBUF(cmdsuc, sendsuc, lensuc);

  PvpResultCCmd cmdfail;
  cmdfail.set_type(EPVPTYPE_SMZL);
  cmdfail.set_result(EPVPRESULT_FAIL);
  PROTOBUF(cmdfail, sendfail, lenfail);
  
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  for (auto&s : userSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (!user)
      continue;
    if (user->getTeamID() == m_qwWinTeamId)
    {
      user->sendCmdToMe(sendsuc, lensuc);
      getPvpCoin(user, EPVPREWARD_DESERTWIN);
      user->getAchieve().onPvp(true);
      XDBG << "[沙漠之狼-发送胜利消息] charid" << user->id << "name" << user->name << "胜利队伍" << m_qwWinTeamId << XEND;
    }
    else
    {
      user->sendCmdToMe(sendfail, lenfail);
      XDBG << "[沙漠之狼-发送胜利消息] charid" << user->id << "name" << user->name << "我的队伍" << user->getTeamID() <<"胜利队伍" << m_qwWinTeamId << XEND;
    }
    user->m_oBuff.add(120244);
    SceneNpc* pet = user->getUserPet().getPetNpc();
    if (pet)
    {
      pet->m_ai.setCurLockID(0);
      pet->m_blGod = true;
    }
    SceneNpc* being = user->getUserBeing().getCurBeingNpc();
    if (being)
    {
      being->m_ai.setCurLockID(0);
      being->m_blGod = true;
    }
  }
  broadcastFinalScore();
  setCloseTime(now() + 30);
  ntfMatchCloseRoom();
  m_dwEndTime = 0;
  m_isOver = true;
  m_dwShouResultTime = 0;
}

void DesertWolfScene::getBornPos(SceneUser* pUser, xPos& pos)
{
  if (!pUser)
    return;
  DWORD index = pUser->getUserZone().getColorIndex();
  xPos* ppos = nullptr;
  xPos tempPos;
  do
  {
    auto it = m_mapTeamPos.find(index);
    if (it != m_mapTeamPos.end())
    {
      ppos = &(it->second);
      break;
    }

    if (m_mapPos.empty())
      break;

    std::vector<DWORD> vecPos;
    vecPos.reserve(m_mapPos.size());

    for (auto &m : m_mapPos)
    {
      vecPos.push_back(m.first);
    }
    std::random_shuffle(vecPos.begin(), vecPos.end());

    if (vecPos.begin() == vecPos.end())
      break;
    DWORD offset = *(vecPos.begin());
    tempPos = m_mapPos[offset];
    m_mapPos.erase(offset);
    ppos = &tempPos;
    m_mapTeamPos[index] = *ppos;
  } while (0);

  if (ppos == nullptr)
  {
    getRandPos(tempPos);
    ppos = &tempPos;
    m_mapTeamPos[index] = *ppos;
  }
  pos = *ppos;

  XDBG << "[斗技场-获取出生点-沙漠之狼]" << pUser->getTempID() << pUser->getName() << "index" << index <<"x" << pos.getX() << "y" << pos.getY() << "z" << pos.getZ() << XEND;
}

/************************************************************************/
/*PollyScene                                                                      */
/************************************************************************/

void ScoreInfo::init(QWORD charId, const std::string&name, DWORD defaultScore)
{
  resetRank();
  qwCharId = charId;
  strName = name;
  dwScore = defaultScore;
  dwMaxScore = defaultScore;
  qwScoreStartTime = xTime::getCurMSec();
}

void ScoreInfo::setNewSocre(DWORD newScore)
{
  if (dwScore == newScore)
    return;

  QWORD curMSec = xTime::getCurMSec();
  QWORD& rDuration = mapKeepDuration[dwScore];
  if (curMSec > qwScoreStartTime)
    rDuration += (curMSec - qwScoreStartTime);

  dwScore = newScore;
  qwScoreStartTime = curMSec;

  if (newScore > dwMaxScore)    //更新最高积分
    dwMaxScore = newScore;    
}

//刷新维持时间
void ScoreInfo::refreshKeepTime(QWORD curMSec)
{
  QWORD& rDuration = mapKeepDuration[dwScore];
  if (curMSec > qwScoreStartTime)
    rDuration += (curMSec - qwScoreStartTime);
  qwScoreStartTime = curMSec;
}

PollyScene::PollyScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :PvpBaseScene(sID, sName, sBase, pRaidCFG)
{
}
PollyScene::~PollyScene()
{
}

bool PollyScene::init()
{
  if (PvpBaseScene::init() == false)
    return false;

  const SPvpCFG* pCfg = MiscConfig::getMe().getPvpCfg(EPVPTYPE_POLLY);
  if (!pCfg)
  {
    XERR << "[斗技场-波利乱斗] 找不到pvp配置" << XEND;
    return false;
  }
  m_dwStartTime = now();
  m_dwEndTime = m_dwStartTime + pCfg->dwDuration;
  m_dwMsgTime = m_dwEndTime - MiscConfig::getMe().getPoliFireCFG().dwPreCloseMsgTime;
  m_dwMaxScore = MiscConfig::getMe().getPoliFireCFG().dwDefualtScore;
  m_bScoreChanged = false;

  XLOG << "[斗技场-波利乱斗] 倒计时时间starttime" << m_dwStartTime << "endtime" << m_dwEndTime << pCfg->dwDuration <<"初级积分" << m_dwMaxScore << XEND;
  return true;
}

void PollyScene::entryAction(QWORD curMSec)
{
  PvpBaseScene::entryAction(curMSec);

  if (m_calcState != ECALCSTATE_FIGHTING)
    return;

  DWORD cur = curMSec / ONE_THOUSAND;
  
  if (m_bNeedRank && cur > m_dwNextRankTime)
  {
    rank();
    notifyAllUser();
    m_dwNextRankTime = cur + 1;
  }
  checkEnd(cur);

  if (cur == m_dwMsgTime)
  {
    DWORD time = MiscConfig::getMe().getPoliFireCFG().dwPreCloseMsgTime;
    MsgManager::sendMapMsg(this->id, 122, MsgParams(time));
    m_dwMsgTime = 0;
  }
}

void PollyScene::getBornPos(SceneUser* pUser, xPos& pos)
{
  if (!pUser)
    return;
  DWORD index = pUser->getUserZone().getColorIndex();
  xPos* ppos = nullptr;
  xPos tempPos;

  auto it = m_mapPos.find(index);
  if (it != m_mapPos.end())
    ppos = &(it->second);

  if (ppos == nullptr)
  {
    getRandPos(tempPos);
    ppos = &tempPos;
  }
  pos = *ppos;

  XDBG << "[斗技场-获取出生点-波利乱斗]" << pUser->getTempID() << pUser->getName() << "index" << index << "x" << pos.getX() << "y" << pos.getY() << "z" << pos.getZ() <<"index"<<index << XEND;
}

void PollyScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  PvpBaseScene::userEnter(user);

  //解散队伍
  {
    ExitTeam cmd;
    PROTOBUF(cmd, send, len);
    thisServer->forwardCmdToSessionUser(user->id, send, len);
  }

  const SPoliFireCFG& rCfg = MiscConfig::getMe().getPoliFireCFG();

  DWORD defaultScore = rCfg.dwDefualtScore;
  ScoreInfo& rInfo = m_mapScoreInfo[user->id];
  rInfo.init(user->id, user->name, defaultScore);
  
  //变身
  DWORD buffId = rCfg.getBuffId(user->getUserZone().getColorIndex());
  user->m_oBuff.add(buffId, user);

  for (DWORD i = 0; i < defaultScore; ++i)
    user->m_oBuff.add(rCfg.dwShowBuff);

  //无敌
  user->m_oBuff.add(rCfg.dwGodBuffid, user);
  
  //贪得无厌buff
  user->m_oBuff.add(rCfg.dwGreedyBuff);

  GodEndTimeCCmd cmd;
  cmd.set_endtime(now() + rCfg.dwGodDuration);
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);

  notifyAllUser();
  
  user->getAchieve().onJoinPolly();

  //隐藏buff
  for (const auto& s : rCfg.getMaskBuffId())
  {
    user->getBuff().clientHideBuff(s);
  }

  m_setEntered.insert(user->id);
  if (m_dwProtectTime == 0)  
    m_dwProtectTime = now() + 10;

  user->getServant().onFinishEvent(ETRIGGER_POLLY_FIGHT);
  XLOG << "[斗技场-进入波利乱斗]" << user->accid << user->id << user->getProfession() << user->name << defaultScore <<"buffid"<<buffId <<"index"<<user->getUserZone().getColorIndex() << XEND;
}

void PollyScene::onLeaveScene(SceneUser* user)
{
  if (user == nullptr)
    return;

  //移除buff
  //先移除同步变身buff
  const SPoliFireCFG& rCFG = MiscConfig::getMe().getPoliFireCFG();
  DWORD buffId = rCFG.getBuffId(user->getUserZone().getColorIndex());
  user->m_oBuff.del(buffId);
  user->getBuff().update(xTime::getCurMSec());

  user->m_oBuff.del(rCFG.dwGhostPoliBuff);
  user->m_oBuff.del(rCFG.dwGodBuffid);
  user->m_oBuff.del(rCFG.dwShowBuff, true);
  user->getBuff().update(xTime::getCurMSec());
  user->getAttribute()->updateAttribute();
  user->changeHp(user->getAttr(EATTRTYPE_MAXHP), user);
  user->setSp(user->getAttr(EATTRTYPE_MAXSP));
  user->getBuff().clearClientHideBuff();
}

void PollyScene::userLeave(SceneUser* user)
{
  if (user == nullptr)
    return;

  //移除buff
  /*const SPoliFireCFG& rCFG = MiscConfig::getMe().getPoliFireCFG();
  user->m_oBuff.del(rCFG.dwGhostPoliBuff);
  user->m_oBuff.del(rCFG.dwGodBuffid);
  DWORD buffId = rCFG.getBuffId(user->getUserZone().getColorIndex());
  user->m_oBuff.del(buffId);
  user->m_oBuff.del(rCFG.dwShowBuff, true);
  */

  PvpBaseScene::userLeave(user);
  m_mapScoreInfo.erase(user->id);
  user->getUserZone().setColorIndex(0);

  // 贪婪buff
  const SPoliFireCFG& rCFG = MiscConfig::getMe().getPoliFireCFG();
  user->m_oBuff.del(rCFG.dwGreedyBuff);

  checkEnd(0);

  //玩家离开更新排名
  if (m_calcState == ECALCSTATE_FIGHTING)
  {
    m_bNeedRank = true;
  }
}

void PollyScene::notify(QWORD id)
{
  auto it = m_mapScoreInfo.find(id);
  if (it == m_mapScoreInfo.end())
    return;
  ScoreInfo* pInfo = &(it->second);

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(pInfo->qwCharId);
  if (pUser == nullptr)
    return;

  NtfFightStatCCmd cmd;
  cmd.set_pvp_type(EPVPTYPE_POLLY);
  cmd.set_player_num(m_mapScoreInfo.size());
  cmd.set_starttime(m_dwStartTime);
  cmd.set_score(pInfo->dwScore);
  cmd.set_myrank(pInfo->dwRank);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  XLOG << "[斗技场-波利乱斗] 推送信息给客户端 charid" << pUser->id << pUser->name << "苹果数" << pInfo->dwScore << "我的排名" << pInfo->dwRank << XEND;
}

void PollyScene::notifyAllUser()
{
  for (auto m = m_mapScoreInfo.begin(); m != m_mapScoreInfo.end(); ++m)
  {
    notify(m->first);
  }
}

ScoreInfo* PollyScene::getScoreInfo(QWORD charId)
{
  auto it = m_mapScoreInfo.find(charId);
  if (it == m_mapScoreInfo.end())
    return nullptr;
  return &(it->second);
}

bool PollyScene::canPickUp(QWORD charid)
{
  ScoreInfo* pInfo = getScoreInfo(charid);
  if (pInfo == nullptr)
    return false;
  return pInfo->bCanPickUp;
}

bool PollyScene::canPickUpApple(QWORD charid)
{
  ScoreInfo* pInfo = getScoreInfo(charid);
  if (pInfo == nullptr)
    return false;
  static DWORD cd = MiscConfig::getMe().getPoliFireCFG().dwDropAppleCDMs;
  if (pInfo->qwDropAppleTime && xTime::getCurMSec() < pInfo->qwDropAppleTime + cd)
    return false;
  return true;
}

void PollyScene::markDropApple(SceneUser* user)
{
  if (user == nullptr)
    return;
  ScoreInfo* pInfo = getScoreInfo(user->id);
  if (pInfo)
  {
    pInfo->qwDropAppleTime = xTime::getCurMSec();
  }
}

DWORD PollyScene::getScore(SceneUser* user)
{
  if (user == nullptr)
    return 0;
  ScoreInfo* pInfo = getScoreInfo(user->id);
  if (pInfo)
    return pInfo->dwScore;
  return 0;
}

QWORD PollyScene::getNextPickAppleTime(SceneUser* user)
{
  if (user == nullptr)
    return 0;
  ScoreInfo* pInfo = getScoreInfo(user->id);
  if (pInfo == nullptr)
    return 0;
  return pInfo->qwNextPickTime;
}

void PollyScene::setScore(SceneUser* user, DWORD newScore)
{
  if (user == nullptr)
    return;
  ScoreInfo* pInfo = getScoreInfo(user->id);
  if (!pInfo)
    return;
  DWORD old = pInfo->dwScore;
  if (old == newScore)
    return;

  const SPoliFireCFG& rCFG = MiscConfig::getMe().getPoliFireCFG();

  if (newScore > old)
    pInfo->qwNextPickTime = xTime::getCurMSec() + rCFG.dwPickAppleCDMs;

  if (newScore == 0)
  {
    pInfo->bCanPickUp = false;
    user->m_oBuff.add(rCFG.dwGhostPoliBuff);
  }
  else
  {
    if (pInfo->bCanPickUp == false && newScore >= rCFG.dwRecoverAppleNum)
    {
      // add buff;
      DWORD buffId = rCFG.getBuffId(user->getUserZone().getColorIndex());
      user->m_oBuff.add(buffId, user);
      pInfo->bCanPickUp = true;
    }
  }

  m_bScoreChanged = true;
  if (newScore > m_dwMaxScore)
    m_dwMaxScore = newScore;
  
  if (pInfo->dwScore < newScore)
  {
    DWORD gotApple = newScore - pInfo->dwScore;
    user->getAchieve().onGoldAppleTotal(gotApple);
  }

  if (newScore > old)
  {
    for (DWORD i = 0; i < newScore - old; ++i)
      user->m_oBuff.add(rCFG.dwShowBuff);
  }
  else
  {
    for (DWORD i = 0; i < old - newScore; ++i)
      user->m_oBuff.del(rCFG.dwShowBuff);
  }

  pInfo->setNewSocre(newScore);
  m_bNeedRank = true;
  user->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF); //刷新buff(苹果数影响移动速度)
  user->refreshDataAtonce();
  XLOG << "[斗技场-波利乱斗] 积分更新 charid" << user->id << "accid" << user->accid << user->name << "原来积分" << old << "新积分" << newScore << XEND;
}

bool PollyScene::checkEnd(DWORD curSec)
{
  if (m_calcState != ECALCSTATE_FIGHTING)
    return false;
  bool bEnd = false;
  do 
  {
    if (curSec)
    {
      if (m_dwEndTime <= curSec)
      {
        bEnd = true;
        break;
      }
    }
    else
    {//只剩下最后一人，则结束
      if (m_mapScoreInfo.size() == 1)
      {
        if (m_setEntered.size() >= MiscConfig::getMe().getPvpCfg(EPVPTYPE_POLLY)->dwPeopleLImit || now() >= m_dwProtectTime)
        {
          bEnd = true;
          break;
        }
        else
        {
          XERR << "[斗技场-波利乱斗] 保护期间只剩最后一人，不算结束,进来过的玩家数"<<m_setEntered.size() << MiscConfig::getMe().getPvpCfg(EPVPTYPE_POLLY)->dwPeopleLImit <<"保护时间"<<m_dwProtectTime <<"当前时间"<<now() << XEND;
        }
      }
    }
  } while (0);
  
  if (!bEnd)
    return false;
  
  m_calcState = ECALCSTATE_WIN;  
  processResult();
  
  if (curSec == 0)
    curSec = now();
  setCloseTime(curSec + 120);
  return true;
}

void PollyScene::processResult()
{
  m_bNeedRank = true;
  rank();  

  PvpResultCCmd cmd;
  cmd.set_result(EPVPRESULT_SUCCESS);
  cmd.set_type(EPVPTYPE_POLLY);

  for (auto &m : m_mapScoreInfo)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(m.first);
    if (!pUser)
    {
      XERR <<"[斗技场-波利乱斗-结算] 找不到玩家 charid" <<m.first  << XEND;
      continue;
    }

    RankInfo* pRankInfo = cmd.add_rank();
    if (pRankInfo)
    {
      pRankInfo->set_charid(pUser->id);
      pRankInfo->set_index(pUser->getUserZone().getColorIndex());
      pRankInfo->set_rank(m.second.dwRank);
      pRankInfo->set_name(pUser->name);
    }
    m.second.pUser = pUser;
  }

  for (auto&m : m_mapScoreInfo)
  {
    if (!m.second.pUser)
      continue;
    sendResult(m.second.pUser, m.second, cmd);
  }
}

void PollyScene::sendResult(SceneUser* pUser, ScoreInfo& rInfo, PvpResultCCmd& cmd)
{
  if (!pUser)
    return;
  cmd.clear_reward();  
  TVecItemInfo vecItemInfo;
  if (!pUser->getVar().getAccVarValue(EACCVARTYPE_POLLY_FIRST))
  {
    DWORD rewardId = MiscConfig::getMe().getPoliFireCFG().getRewardId(pUser->getLevel());
    XLOG << "[斗技场-波利乱斗-奖励] 首次完成奖励 charid" <<pUser->id <<"accid"<<pUser->accid <<pUser->name <<"lv"<<pUser->getLevel() <<"rewarid"<<rewardId << XEND;
    if (rewardId)
    {
      RewardManager::roll(rewardId, pUser, vecItemInfo, ESOURCE_PVP_POLLY);
      pUser->getVar().setAccVarValue(EACCVARTYPE_POLLY_FIRST, 1);
    }    
  }

  DWORD dwLimitScore = MiscConfig::getMe().getPoliFireCFG().dwAppleLimitCount;
  DWORD dwScore = rInfo.dwScore * 10;
  DWORD dwGotScore = pUser->getVar().getAccVarValue(EACCVARTYPE_POLLY_DAY_SCORE);
  DWORD dwCanGetScore = 0;
  if (dwLimitScore > dwGotScore)
    dwCanGetScore = dwLimitScore - dwGotScore; 
  if (dwCanGetScore > dwScore)
    dwCanGetScore = dwScore;  
  if (dwCanGetScore)
  {
    ItemInfo scoreItem;
    scoreItem.set_id(MiscConfig::getMe().getPoliFireCFG().dwSocreItemId);
    scoreItem.set_count(dwCanGetScore);
    scoreItem.set_source(ESOURCE_PVP_POLLY);
    combinItemInfo(vecItemInfo, scoreItem);
    dwGotScore += dwCanGetScore;
    pUser->getVar().setAccVarValue(EACCVARTYPE_POLLY_DAY_SCORE, dwGotScore);
  }

  DWORD dwExtraCanGetScore = 0;
  DWORD dwExtraScore = LuaManager::getMe().call<DWORD>("calcExtraScore", MiscConfig::getMe().getPoliFireCFG().dwAppleLimitCount, rInfo.dwRank);
  if (dwExtraScore)
  {
    if (dwLimitScore > dwGotScore)
      dwExtraCanGetScore = dwLimitScore - dwGotScore;
    if (dwExtraCanGetScore > dwExtraScore)
      dwExtraCanGetScore = dwExtraScore;
    if (dwExtraCanGetScore)
    {
      ItemInfo scoreItem;
      scoreItem.set_id(MiscConfig::getMe().getPoliFireCFG().dwSocreItemId);
      scoreItem.set_count(dwExtraCanGetScore);
      scoreItem.set_source(ESOURCE_PVP_POLLY);
      combinItemInfo(vecItemInfo, scoreItem);
      dwGotScore += dwExtraCanGetScore;
      pUser->getVar().setAccVarValue(EACCVARTYPE_POLLY_DAY_SCORE, dwGotScore);
    }
  }

  //add item
  pUser->getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);
  
  cmd.set_apple(rInfo.dwScore);  
  for (auto& v : vecItemInfo)
  {
    if (v.count() == 0)
      continue;
    RewardInfo* pRewardInfo = cmd.add_reward();
    if (pRewardInfo)
    {
      pRewardInfo->set_itemid(v.id());
      pRewardInfo->set_count(v.count());
    }
  }
  
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  
  pUser->getAchieve().onGoldAppleGame(rInfo.dwScore);

  XLOG << "[斗技场-波利乱斗-结算] charid" << pUser->id << "accid" << pUser->accid << pUser->name <<"苹果数量"<< rInfo.dwScore <<"我的排名"<< rInfo.dwRank << "上限积分限制" << dwLimitScore << "普通获得积分" << dwCanGetScore << "额外积分" << dwExtraCanGetScore <<"msg"<<cmd.ShortDebugString() << XEND;
}

bool PollyScene::rank()
{
  if (m_bNeedRank == false)
    return false;
  
  if (!m_bScoreChanged)
  {
    XDBG << "[斗技场-波利乱斗] 积分没有变化，不需要排名" << XEND;
    return false;
  }

  std::vector<ScoreInfo*> vecRank;
  vecRank.reserve(m_mapScoreInfo.size());
  for (auto it = m_mapScoreInfo.begin(); it != m_mapScoreInfo.end(); ++it)
  {
    it->second.refreshKeepTime(xTime::getCurMSec());
    vecRank.push_back(&(it->second));
  }

  std::sort(vecRank.begin(), vecRank.end(), RankFun());
  
  m_vecTopRank.clear();
  for (DWORD i = 0; i < vecRank.size(); ++i)
  {
    if (i < 3)
    {      
      m_vecTopRank.push_back(*(vecRank[i]));
    }

    vecRank[i]->dwRank = i+1; 
    XLOG << "[斗技场-波利乱斗-排名] charid" << vecRank[i]->qwCharId <<"排名"<< vecRank[i]->dwRank <<"score"<< vecRank[i]->dwScore <<"maxscore" << vecRank[i]->dwMaxScore <<"保持时间" << vecRank[i]->mapKeepDuration[vecRank[i]->dwMaxScore] << XEND;
  }  
  
  sendRank2Client();
  m_bNeedRank = false;
  return true;
}

void PollyScene::sendRank2Client()
{
  if (m_vecTopRank.empty())
  {
    XLOG << "[斗技场-波利乱斗] 排名为空" << XEND;
    return;
  }
  NtfRankChangeCCmd cmd;
  for (auto&v : m_vecTopRank)
  {
    RankNameInfo* pInfo = cmd.add_ranks();
    if (pInfo)
    {
      pInfo->set_name(v.strName);
      pInfo->set_apple(v.dwScore);
    }
  }
  PROTOBUF(cmd, send, len);
  Scene::sendCmdToAll(send, len);
  XLOG << "[斗技场-波利乱斗]， 同步前三名给客户端"<< m_vecTopRank.size() <<"msg"<<cmd.ShortDebugString() << XEND;
}

/************************************************************************/
/*GuildRaidScene                                                                      */
/************************************************************************/
GuildRaidScene::GuildRaidScene(DWORD sID, const char* name, const SceneBase* pBase, const SRaidCFG* pRaidCFG, QWORD guildid, QWORD teamid, DWORD dwGuildRaidIndex) :
  DScene(sID, name, pBase, pRaidCFG)
{
  m_qwGuildID = guildid;
  m_qwTeamID = teamid;
  m_dwGuildRaidIndex = dwGuildRaidIndex;
}

bool GuildRaidScene::init()
{
  if (DScene::init() == false)
    return false;

  XLOG << "[公会副本], 创建地图, 地图index:" << m_dwGuildRaidIndex << "地图ID:" << getRaidID() << "动态id:" << id << XEND;
  return true;
}

void GuildRaidScene::entryAction(QWORD curMSec)
{
  DScene::entryAction(curMSec);

  if (m_dwCurEpID != 0 && !m_bGoOtherGRaid)
  {
    checkExit();
  }
}

void GuildRaidScene::checkExit(const ExitPoint* pPoint)
{
  if (pPoint == nullptr)
    return;

  if (m_dwCurEpID != 0)
    return;

  m_dwCurEpID = pPoint->m_dwExitID;
  m_dwExitCheckTime = now() + 3;
  checkExit();
}

void GuildRaidScene::checkExit()
{
  const SceneObject *pObject = getSceneObject();
  if (!pObject) return;
  const ExitPoint* pPoint = pObject->getExitPoint(m_dwCurEpID);
  if (pPoint == nullptr)
    return;

  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  DWORD dwCount = 0;
  for (auto s = entrySet.begin(); s != entrySet.end(); )
  {
    SceneUser* user = dynamic_cast<SceneUser*> (*s);
    if (user == nullptr || user->isAlive() == false)
    {
      s = entrySet.erase(s);
      continue;
    }
    if (checkRadius(user->getPos(), pPoint->m_oPos , pPoint->m_flRange))
      dwCount++;
    ++s;
  }

  STimerDownState & rTimerDownState = m_oFuben.getTimerDown();
  switch (rTimerDownState.eTimerState)
  {
    case ETIMERDOWNSTATE_STOP:
      {
        if (dwCount > 0)
        {
          rTimerDownState.eTimerState = ETIMERDOWNSTATE_RUN;
          rTimerDownState.dwTimerSec = 5;
          rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
        }
        else if (dwCount >= entrySet.size())
        {
          rTimerDownState.eTimerState = ETIMERDWONSTATE_PASS;
        }
        else
        {
          if (now() >= m_dwExitCheckTime)
            m_dwCurEpID = 0;
        }
        break;
      }
    case ETIMERDOWNSTATE_RUN:
      {
        if (dwCount == 0)
        {
          rTimerDownState.eTimerState = ETIMERDOWNSTATE_STOP;
          rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
          rTimerDownState.dwTimerSec = 5;
          // 取消倒计时
          for (auto entryIt : entrySet)
            MsgManager::sendMsg(entryIt->id, 1202, MsgParams(5), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
          break;
        }
        else if (dwCount >= entrySet.size())
        {
          rTimerDownState.eTimerState = ETIMERDWONSTATE_PASS;
          break;
        }
        // time count
        // notify client show time count down
        DWORD dwCurTimeSec = xTime::getCurSec();
        if (dwCurTimeSec <= rTimerDownState.dwRefreshTimeSec)
          break;

        if (0 == rTimerDownState.dwTimerSec)
        {
          rTimerDownState.eTimerState = ETIMERDWONSTATE_PASS;
          rTimerDownState.dwTimerSec = 5;
          break;
        }
        if (rTimerDownState.dwTimerSec == 5)
        {
          for (auto entryIt : entrySet)
            MsgManager::sendMsg(entryIt->id, 1202, MsgParams(5), EMESSAGETYPE_TIME_DOWN);
        }
        rTimerDownState.dwTimerSec--;
        rTimerDownState.dwRefreshTimeSec = dwCurTimeSec;
        break;
      }
    case ETIMERDWONSTATE_PASS:
      {
        // 取消倒计时
        for (auto entryIt : entrySet)
          MsgManager::sendMsg(entryIt->id, 1202, MsgParams(5), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
        rTimerDownState.eTimerState = ETIMERDOWNSTATE_STOP;
        rTimerDownState.dwTimerSec = 5;
        rTimerDownState.dwRefreshTimeSec = xTime::getCurSec();
        goNextMap(pPoint);
        m_dwCurEpID = 0;
        break;
      }
    default:
      break;
  }
}

bool GuildRaidScene::goNextMap(const ExitPoint* pPoint)
{
  if (pPoint == nullptr)
    return false;

  const SGuildRaidInfo* pRaidInfo = GuildRaidConfig::getMe().getGuildRaidInfo(m_dwGuildRaidIndex);
  if (pRaidInfo == nullptr)
    return false;

  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (entrySet.empty())
    return false;
  CreateDMapParams param;
  param.qwCharID = (*(entrySet.begin()))->id;
  entrySet.erase(entrySet.begin());
  for (auto &s : entrySet)
    param.vecMembers.push_back(s->id);

  if (pPoint->m_intNextMapID == -1)
  {
    const SGuildRaidLink* pNextMap = pRaidInfo->getNextMapInfo(pPoint->m_dwExitID);
    if (pNextMap)
    {
      param.dwRaidID = pNextMap->dwNextMapID;
      param.m_dwGuildRaidIndex = pNextMap->getMapIndex();
      const SceneBase* nextbase = SceneManager::getMe().getDataByID(pNextMap->dwNextMapID);
      if (nextbase && nextbase->getSceneObject(pNextMap->dwNextMapID))
      {
          const xPos* pPos = nextbase->getSceneObject(pNextMap->dwNextMapID)->getBornPoint(pNextMap->dwNextBornPoint);
          if (pPos)
            param.m_oEnterPos = *pPos;
      }
      SceneManager::getMe().createDScene(param);
      m_bGoOtherGRaid = true;
      //XLOG << "[公会副本]"
    }
  }
  else
  {
    param.dwRaidID = pPoint->m_intNextMapID;
    SceneManager::getMe().createDScene(param);
    closeGuildRaid();
  }

  return true;
}

void GuildRaidScene::closeGuildRaid()
{
  GuildRaidCloseSessionCmd cmd;
  cmd.set_mapid(id);
  cmd.set_curmapindex(m_dwGuildRaidIndex);
  cmd.set_guildid(m_qwGuildID);
  cmd.set_teamid(m_qwTeamID);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

void GuildRaidScene::userEnter(SceneUser* user)
{
  if (user == nullptr)
    return;
  DScene::userEnter(user);

  // 记录玩家是否在公会副本之间传送
  m_bGoOtherGRaid = false;
}

void GuildRaidScene::userLeave(SceneUser* user)
{
  if (user == nullptr)
    return;
  DScene::userLeave(user);

  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (entrySet.empty())
  {
    if (m_bGoOtherGRaid == false)
    {
      closeGuildRaid();
    }
  }

  if (user->getUserSceneData().getOnlineMapID() == this->id)
  {
    user->getUserSceneData().setOnlineMapPos(user->getUserSceneData().getLastRealMapID(), xPos());
  }
}

void GuildRaidScene::summonRewardBox(SceneUser* pUser)
{
  if (pUser == nullptr || pUser->getScene() != this)
    return;
  const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
  NpcDefine oDefine;
  oDefine.load(rCFG.oRewardBoxDefine);

  xPos oPos = pUser->getPos();
  oDefine.setPos(oPos);
  oDefine.setRange(rCFG.fRewardBoxRange);
  oDefine.m_oVar.m_qwQuestOwnerID = pUser->id;

  SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, this);
  if (pNpc == nullptr)
  {
    XERR << "[公会副本-召唤宝箱], 玩家:" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "召唤 :" << oDefine.getID() << "失败" << XEND;
    return;
  }

  UserActionNtf cmd;
  cmd.set_charid(pNpc->tempid);
  cmd.set_value(rCFG.dwRewardBoxUngetAction);
  cmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  xLuaData data;
  data.setData("effect", rCFG.strRewardBoxEffect);
  data.setData("npcid", pNpc->getNpcID());
  data.setData("effectpos", 1);
  GMCommandRuler::effect(pUser, data);
}

void GuildRaidScene::getRandomDeadBoss(std::list<pair<DWORD,DWORD>>& bosslist)
{
  const SGuildRaidCFG& rCFG = MiscConfig::getMe().getGuildRaidCFG();
  for (DWORD i = 0; i < rCFG.dwDeadBossNum; ++i)
  {
    DWORD lv = LuaManager::getMe().call<DWORD>("CalcRaidDeadBossSummonLevel", (DWORD)ERAID_DEADBOSSTYPE_GUILD);
    DWORD monsterid = NpcConfig::getMe().getOneRandomRaidDeadBoss(ERAID_DEADBOSSTYPE_GUILD, lv);
    bosslist.push_back(std::make_pair(monsterid, rCFG.dwDeadBossUID));
  }
}

bool GuildRaidScene::checkSummonDeadBoss()
{
  const SGuildRaidCFG& rCFG = MiscConfig::getMe().getGuildRaidCFG();
  DWORD lv = rCFG.getLvByGroup(m_dwGuildRaidIndex);
  return lv >= rCFG.dwDeadBossRaidLv;
}

// dateland scene
DateLandScene::DateLandScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :
  DScene(sID, sName, sBase, pRaidCFG)
{

}

bool DateLandScene::init()
{
  return DScene::init();
}

void DateLandScene::userEnter(SceneUser* user)
{
  if (user == nullptr)
    return;

  DScene::userEnter(user);
}

void DateLandScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userLeave(user);
}


// 公会战副本
GuildFireScene::GuildFireScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD dwMapIndex, bool bFireOpen):
  DScene(sID, sName, sBase, pRaidCFG), m_oOneSecTimer(1)
{
  m_dwMapIndex = dwMapIndex;
}

bool GuildFireScene::init()
{
  if (DScene::init() == false)
    return false;

  m_pCity = GuildCityManager::getMe().getCityByID(m_dwMapIndex);
  if (m_pCity == nullptr)
  {
    m_pCity = GuildCityManager::getMe().createCity(m_dwMapIndex);
    if (m_pCity == nullptr)
      return false;
  }

  m_pCity->addScene(this);

  m_vecSafeArea.clear();
  const SGuildCityCFG* pCFG = GuildRaidConfig::getMe().getGuildCityCFG(m_dwMapIndex);
  if (pCFG && pCFG->hasSafaArea(getMapID()))
  {
    SGvgSafeArea area;
    if (pCFG->getRecPosAndWidth(getMapID(), area.pos1, area.pos2, area.fWidth))
      m_vecSafeArea.push_back(area);
  }

  const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();
  addFreeSkill(firecfg.dwExpelSkill);
  return true;
}

void GuildFireScene::entryAction(QWORD curMSec)
{
  DScene::entryAction(curMSec);

  if (m_oOneSecTimer.timeUp(curMSec / ONE_THOUSAND)  && m_vecSafeArea.empty() == false)
  {
    const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();

    SGvgSafeArea& area = m_vecSafeArea[0];
    xSceneEntrySet userset;
    getEntryIn2Pos(area.pos1, area.pos2, area.fWidth, userset);
    for (auto &s : userset)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (user)
      {
        user->m_oBuff.add(firecfg.dwSafeBuffID);
      }
      else
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (s); //猫等
        if (npc && npc->getMasterUser() != nullptr)
          npc->m_oBuff.add(firecfg.dwSafeBuffID);
      }
    }
  }
}

void GuildFireScene::userEnter(SceneUser* user)
{
  DScene::userEnter(user);

  user->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);
  if (m_pCity)
  {
    m_pCity->userEnter(user);
    if (m_pCity->checkKickUser(user) && user->getUserSceneData().getLastMapID() == this->id)
      addKickList(user);
  }
}

void GuildFireScene::userLeave(SceneUser* user)
{
  DScene::userLeave(user);

  if (m_pCity)
  {
    m_pCity->userLeave(user);
  }
  // 清空记录的静态坐标点, 防止 踩传送点->据点->公会领地->离开  -> 踩传送点..
  user->getUserMap().setLastStaticMap(user->getUserMap().getLastStaticMapID(), xPos());
}

void GuildFireScene::onKillUser(SceneUser* pKiller, SceneUser* pDeath)
{
  if (pKiller == nullptr || pDeath == nullptr)
    return;
  XLOG << "[公会战], 杀死玩家, 攻击方:" << pKiller->name << pKiller->id << "死亡方:" << pDeath->name << pDeath->id << "城池:" << m_dwMapIndex << XEND;
  pKiller->getUserGvg().onKillUser();
}

void GuildFireScene::onNpcBeAttack(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp)
{
  if (m_pCity)
    m_pCity->onNpcBeAttack(npc);
}

void GuildFireScene::onNpcBeHeal(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp)
{
  if (m_pCity)
    m_pCity->onNpcBeAttack(npc, true);
}

void GuildFireScene::onNpcDie(SceneNpc* npc, xSceneEntryDynamic* killer)
{
  DScene::onNpcDie(npc, killer);
  if (m_pCity)
    m_pCity->onNpcDie(npc, killer);
}

bool GuildFireScene::checkMonsterCity()
{
  if (m_pCity == nullptr)
    return false;
  if (m_pCity->getState() != EGUILDFIRE_FIRE)
    return false;
  if (m_pCity->getDefenseGuildID() != 0)
    return false;
  return true;
}

EGuildFireState GuildFireScene::getFireState()
{
  if (m_pCity == nullptr)
    return EGUILDFIRE_PEACE;
  return m_pCity->getState();
}

QWORD GuildFireScene::getDefenseGuildID()
{
  if (m_pCity == nullptr)
    return 0;
  return m_pCity->getDefenseGuildID();
}

void GuildFireScene::onSummonMetalNpc(QWORD npcid)
{
  if (m_pCity)
    m_pCity->setMetalNpcID(npcid);
}

void GuildFireScene::onClose()
{
  if (m_pCity)
    m_pCity->delScene(this);
}

WeddingScene::WeddingScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, QWORD qwWeddingID) : DScene(sID, sName, sBase, pRaidCFG)
{
  m_qwWeddingID = qwWeddingID;;
}

bool WeddingScene::init()
{
  if (DScene::init() == false)
    return false;

  SceneWeddingMgr::getMe().setCurWeddingSceneID(this->id);

  const WeddingInfo& wedinfo = SceneWeddingMgr::getMe().getWeddingInfo();
  m_oWeddingInfo.CopyFrom(wedinfo);

  setCloseTime(wedinfo.endtime());

  doServiceEffect();
  XLOG << "[婚礼-创建], scne id:" << id << "婚礼id:" << m_oWeddingInfo.id() << "婚礼双方:" << m_oWeddingInfo.charid1() << m_oWeddingInfo.charid2() << XEND;
  return true;
}

void WeddingScene::updateManual(const WeddingManualInfo& info)
{
  m_oWeddingInfo.mutable_manual()->CopyFrom(info);
}

void WeddingScene::doServiceEffect(bool marry/*=false*/)
{
  TSetDWORD effids;
  const WeddingManualInfo& manual = m_oWeddingInfo.manual();
  for (int i = 0; i < manual.serviceids_size(); ++i)
  {
    const SWeddingServiceCFG* pCFG = WeddingConfig::getMe().getWeddingServiceCFG(manual.serviceids(i));
    if (pCFG == nullptr || pCFG->eType != EWEDDINGSERVICE_PACKAGE)
      continue;

    for (auto &s : pCFG->setServiceID)
    {
      if (effids.find(s) != effids.end())
        continue;
      effids.insert(s);
      const SWeddingServiceCFG* p = WeddingConfig::getMe().getWeddingServiceCFG(s);
      if (!p || p->eType != EWEDDINGSERVICE_PLAN)
        continue;

      auto effectd = marry ? p->vecSuccessGMData : p->vecEffectGMData;
      for (auto &d : effectd)
      {
        DWORD mapid = d.getTableInt("map");
        if (mapid == getRaidID())
        {
          GMCommandRuler::getMe().scene_execute(this, d);
          XLOG << "[婚礼-副本], 执行套餐效果, 婚礼id:" << m_oWeddingInfo.id() << "套餐:" << manual.serviceids(i) << s << XEND;
        }
      }
    }
  }
}

void WeddingScene::onAddService(const TSetDWORD& ids)
{
  for (auto &s : ids)
  {
    const SWeddingServiceCFG* p = WeddingConfig::getMe().getWeddingServiceCFG(s);
    if (!p || p->eType != EWEDDINGSERVICE_PLAN)
      continue;

    for (auto &d : p->vecEffectGMData)
    {
      DWORD mapid = d.getTableInt("map");
      if (mapid == getRaidID())
      {
        GMCommandRuler::getMe().scene_execute(this, d);
        XLOG << "[婚礼-副本], 套餐更新, 执行套餐效果, 婚礼id:" << m_oWeddingInfo.id() << "套餐效果:" << s << XEND;
      }
    }

    if (m_eWeddingState == EWEDDINGSTATE_FINISH)
    {
      for (auto &d : p->vecSuccessGMData)
      {
        DWORD mapid = d.getTableInt("map");
        if (mapid == getRaidID())
        {
          GMCommandRuler::getMe().scene_execute(this, d);
          XLOG << "[婚礼-副本], 套餐更新, 执行仪式完成套餐效果, 婚礼id:" << m_oWeddingInfo.id() << "套餐效果:" << s << XEND;
        }
      }
    }
  }
}

bool WeddingScene::isWeddingUser(SceneUser* user)
{
  return user && (user->id == m_oWeddingInfo.charid1() || user->id == m_oWeddingInfo.charid2());
}

void WeddingScene::sendCmdToWeddingUser(const void* cmd, DWORD len)
{
  SceneUser* user1 = SceneUserManager::getMe().getUserByID(m_oWeddingInfo.charid1());
  if (user1)
    user1->sendCmdToMe(cmd, len);
  SceneUser* user2 = SceneUserManager::getMe().getUserByID(m_oWeddingInfo.charid2());
  if (user2)
    user2->sendCmdToMe(cmd, len);
}

void WeddingScene::userEnter(SceneUser* user)
{
  if (user == nullptr)
    return;
  DScene::userEnter(user);

  if (isWeddingUser(user) && m_eWeddingState != EWEDDINGSTATE_FINISH)
    user->getQuest().acceptQuest(MiscConfig::getMe().getWeddingMiscCFG().dwShowQuestID, true);
  user->getAchieve().onWedding(EACHIEVECOND_WEDDING_JOINCEREMONY);

  if (m_qwTempCeremonyNpcID == 0)
  {
    const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();

    xSceneEntrySet set;
    getAllEntryList(SCENE_ENTRY_NPC, set);
    for (auto &s : set)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (npc && npc->getNpcID() == rCFG.dwWeddingNpcID)
      {
        m_qwTempCeremonyNpcID = npc->id;
        break;
      }
    }
  }
}

void WeddingScene::userLeave(SceneUser* user)
{
  DScene::userLeave(user);

  if (isWeddingUser(user))
    user->getQuest().abandonQuest(MiscConfig::getMe().getWeddingMiscCFG().dwShowQuestID);

  // 婚礼过程中一方离开
  if (isWeddingUser(user) && m_eWeddingState != EWEDDINGSTATE_FINISH)
  {
    resetWedding();
  }
}

void WeddingScene::onInviteOk(QWORD masterid, QWORD followerid)
{
  if (m_eWeddingState != EWEDDINGSTATE_WAITSTART)
    return;

  if (masterid != m_oWeddingInfo.charid1() && masterid != m_oWeddingInfo.charid2())
    return;
  if (followerid != m_oWeddingInfo.charid1() && followerid != m_oWeddingInfo.charid2())
    return;

  m_qwMasterID = masterid;
  m_qwFollowerID = followerid;

  m_eWeddingState = EWEDDINGSTATE_START;
  GoToWeddingPosCCmd cmd;
  PROTOBUF(cmd, send, len);
  sendCmdToWeddingUser(send, len);

  XLOG << "[婚礼-仪式], 邀请成功, 等待玩家进入指定位置, 场景:" << id << "婚礼id:" << m_oWeddingInfo.id() << "玩家双方:" << m_oWeddingInfo.charid1() << m_oWeddingInfo.charid2() << XEND;
}

void WeddingScene::onUserInPos(SceneUser* user)
{
  if (m_eWeddingState != EWEDDINGSTATE_START)
    return;

  if (user == nullptr || isWeddingUser(user) == false)
    return;

  if (m_qwTempOnPosUser == user->id)
    return;
  if (m_qwTempOnPosUser == 0)
  {
    m_qwTempOnPosUser = user->id;
    return;
  }

  // 双方都到达
  m_eWeddingState = EWEDDINGSTATE_QUESTION_0;
  m_dwCurQuestionIndex = 0;
  //resetQuestion();

  // 开始答题
  WeddingSwitchQuestionCCmd cmd;
  cmd.set_onoff(true);
  cmd.set_npcguid(m_qwTempCeremonyNpcID);
  PROTOBUF(cmd, send, len);
  sendCmdToWeddingUser(send, len);

  processQuestion_0();

  XLOG << "[婚礼-仪式], 玩家到达指定位置, 开始答题, 场景:" << id << "婚礼id:" << m_oWeddingInfo.id() << "玩家双方:" << m_oWeddingInfo.charid1() << m_oWeddingInfo.charid2() << XEND;
}

// 答题前言
void WeddingScene::processQuestion_0()
{
  if (m_eWeddingState != EWEDDINGSTATE_QUESTION_0)
    return;
  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
  if (rCFG.vecPreQuestion.empty())
    return;

  if (rCFG.vecPreQuestion.size() <= m_dwCurQuestionIndex)
  {
    m_eWeddingState = EWEDDINGSTATE_QUESTION_1;
    m_dwCurQuestionIndex = 0;
    m_qwCurAnswerUserID = 0;
    processQuestion_1();
    return;
  }

  DWORD questionid = rCFG.vecPreQuestion[m_dwCurQuestionIndex];

  QuestionWeddingCCmd cmd;
  cmd.set_questionid(questionid);
  cmd.add_charids(m_qwMasterID);
  cmd.set_npcguid(m_qwTempCeremonyNpcID);
  PROTOBUF(cmd, send, len);
  sendCmdToWeddingUser(send, len);

  sendChatQuestion(questionid);//聊天框显示

  m_dwCurQuestionIndex ++;
  XLOG << "[婚礼-仪式],准备阶段, 答题推进, 婚礼:" << m_oWeddingInfo.id() << "问题:" << questionid << m_dwCurQuestionIndex << XEND;
}

void WeddingScene::processQuestion_1()
{
  if (m_eWeddingState != EWEDDINGSTATE_QUESTION_1)
    return;
  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
  if (rCFG.vecFirstStageQuestion.empty())
    return;

  // switch user to answer, master first
  if (m_qwCurAnswerUserID == 0)
  {
    m_qwCurAnswerUserID = m_qwMasterID;
  }
  else if (m_qwCurAnswerUserID == m_qwMasterID)
  {
    m_qwCurAnswerUserID = m_qwFollowerID;
  }
  else if (m_qwCurAnswerUserID == m_qwFollowerID)
  {
    m_qwCurAnswerUserID = m_qwMasterID;
    m_dwCurQuestionIndex ++;
  }
  else
  {
    return;
  }

  // 第一阶段回答完毕
  if (rCFG.vecFirstStageQuestion.size() <= m_dwCurQuestionIndex)
  {
    m_eWeddingState = EWEDDINGSTATE_QUESTION_2;
    m_dwCurQuestionIndex = 0;
    m_qwCurAnswerUserID = 0;

    // 面对面转向
    SceneUser* master = SceneUserManager::getMe().getUserByID(m_qwMasterID);
    SceneUser* follower = SceneUserManager::getMe().getUserByID(m_qwFollowerID);
    if (master && follower)
    {
      NpcChangeAngle angle1;
      angle1.set_guid(master->id);
      angle1.set_targetid(follower->id);
      PROTOBUF(angle1, send1, len1);
      master->sendCmdToNine(send1, len1);

      DWORD dir1 = calcAngle(master->getPos(), follower->getPos()) * ONE_THOUSAND;
      master->getUserSceneData().setDir(dir1);

      NpcChangeAngle angle2;
      angle2.set_guid(follower->id);
      angle2.set_targetid(master->id);
      PROTOBUF(angle2, send2, len2);
      master->sendCmdToNine(send2, len2);

      DWORD dir2 = calcAngle(follower->getPos(), master->getPos()) * ONE_THOUSAND;
      follower->getUserSceneData().setDir(dir2);
    }

    processQuestion_2();
    return;
  }

  DWORD questionid = rCFG.vecFirstStageQuestion[m_dwCurQuestionIndex];

  QuestionWeddingCCmd cmd;
  cmd.set_questionid(questionid);
  cmd.add_charids(m_qwCurAnswerUserID);
  cmd.set_npcguid(m_qwTempCeremonyNpcID);
  PROTOBUF(cmd, send, len);
  sendCmdToWeddingUser(send, len);

  sendChatQuestion(questionid);//聊天框显示

  XLOG << "[婚礼-仪式], 第一阶段, 答题推进, 婚礼:" << m_oWeddingInfo.id() << "问题:" << questionid << m_dwCurQuestionIndex << XEND;
}

void WeddingScene::processQuestion_2()
{
  if (m_eWeddingState != EWEDDINGSTATE_QUESTION_2)
    return;
  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
  if (rCFG.vecSecondStageQuestion.empty())
    return;

  if (rCFG.vecSecondStageQuestion.size() <= m_dwCurQuestionIndex) //第二阶段回答完毕
  {
    onFinishWedding();
    m_eWeddingState = EWEDDINGSTATE_FINISH;
    return;
  }

  DWORD questionid = rCFG.vecSecondStageQuestion[m_dwCurQuestionIndex];

  QuestionWeddingCCmd cmd;
  cmd.set_questionid(questionid);
  cmd.add_charids(m_qwMasterID);
  cmd.add_charids(m_qwFollowerID);
  cmd.set_npcguid(m_qwTempCeremonyNpcID);
  PROTOBUF(cmd, send, len);
  sendCmdToWeddingUser(send, len);

  sendChatQuestion(questionid);//聊天框显示

  m_dwCurQuestionIndex ++;
  XLOG << "[婚礼-仪式], 第二阶段, 答题推进, 婚礼:" << m_oWeddingInfo.id() << "问题:" << questionid << m_dwCurQuestionIndex << XEND;
}

void WeddingScene::onReceiveAnswer(SceneUser* user, DWORD questionid, DWORD answer)
{
  if (user == nullptr)
    return;

  // check question and user
  const SQuestion* pQuestion = TableManager::getMe().getQuestion(questionid);
  if (pQuestion == nullptr)
    return;
  bool right = (m_eWeddingState == EWEDDINGSTATE_QUESTION_0 ? true : pQuestion->getRightAnswer() == answer);

  // 附近聊天频道显示
  const char* pAnswerStr = pQuestion->getAnswerStr(answer);
  if (pAnswerStr)
    ChatManager_SC::getMe().sendChatMsgToNine(user, pAnswerStr, ECHAT_CHANNEL_ROUND);

  if (right)
  {
    if (m_eWeddingState == EWEDDINGSTATE_QUESTION_0) // 前言, 主牵人回答
    {
      if (user->id != m_qwMasterID)
        return;
      processQuestion_0();
    }
    else if (m_eWeddingState == EWEDDINGSTATE_QUESTION_1) //阶段一, 两人轮流回答
    {
      if (m_qwCurAnswerUserID != user->id)
        return;
      processQuestion_1();
    }
    else if (m_eWeddingState == EWEDDINGSTATE_QUESTION_2) //阶段二, 两人一起回答
    {
      if (m_qwCurAnswerUserID && m_qwCurAnswerUserID != user->id) // 两个人都回答完毕
      {
        m_qwCurAnswerUserID = 0;
        processQuestion_2();
      }
      else
      {
        m_qwCurAnswerUserID = user->id; // 等待另一方回答完毕
      }
    }
    XLOG << "[婚礼-仪式], 收到玩家回答, 婚礼:" << m_oWeddingInfo.id() << "玩家:" << user->name << user->id << "问题:" << questionid << XEND;
  }
  else
  {
    user->changeHp(-(int)user->getAttr(EATTRTYPE_MAXHP), user);
    resetWedding();
    XLOG << "[婚礼-仪式], 答题错误, 婚礼重置, 婚礼:" << m_oWeddingInfo.id() << "玩家:" << user->name << user->id << "问题:" << questionid << "回答:" << answer << XEND;
  }
}

void WeddingScene::resetWedding()
{
  if (isInAnswer())
  {
    // 打断答题流程
    WeddingSwitchQuestionCCmd cmd;
    cmd.set_onoff(false);
    PROTOBUF(cmd, send, len);
    sendCmdToWeddingUser(send, len);
  }

  m_eWeddingState = EWEDDINGSTATE_WAITSTART;
  sendResult(false);
  XLOG << "[婚礼-仪式], 婚礼重置, 婚礼id:" << m_oWeddingInfo.id() << "场景:" << id << XEND;
}

void WeddingScene::resetQuestion()
{
  m_dwCurQuestionIndex = 0;
  m_qwCurAnswerUserID = 0;
}

void WeddingScene::sendChatQuestion(DWORD questionid)
{
  const SQuestion* pQuestion = TableManager::getMe().getQuestion(questionid);
  if (pQuestion == nullptr)
    return;

  const char* pStr = pQuestion->getQuestionStr();
  if (pStr == nullptr)
    return;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwTempCeremonyNpcID);
  if (!npc)
    return;
  NpcChatNtf cmd;
  cmd.set_msg(pStr);
  cmd.set_channel(ECHAT_CHANNEL_ROUND);
  cmd.set_npcid(npc->getNpcID());
  cmd.set_npcguid(npc->id);

  if (questionid == 11) //'%s你愿意与%s结为伴侣，永远爱ta守护ta吗, param添加玩家名字'
  {
    SceneUser* master = SceneUserManager::getMe().getUserByID(m_qwMasterID);
    SceneUser* follower = SceneUserManager::getMe().getUserByID(m_qwFollowerID);
    if (master && follower)
    {
      if (m_qwCurAnswerUserID == m_qwMasterID)
      {
        cmd.add_params()->set_param(master->name);
        cmd.add_params()->set_param(follower->name);
      }
      else
      {
        cmd.add_params()->set_param(follower->name);
        cmd.add_params()->set_param(master->name);
      }
    }
  }

  PROTOBUF(cmd, send, len);
  npc->sendCmdToNine(send, len);
}

void WeddingScene::sendResult(bool ret)
{
  WeddingOverCCmd cmd;
  cmd.set_success(ret);
  PROTOBUF(cmd, send, len);

  sendCmdToWeddingUser(send, len);
}

void WeddingScene::onFinishWedding()
{
  SceneUser* master = SceneUserManager::getMe().getUserByID(m_qwMasterID);
  if (!master)
    return;
  SceneUser* follower = SceneUserManager::getMe().getUserByID(m_qwFollowerID);
  if (!follower)
    return;

  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();

  DWORD ringid = m_oWeddingInfo.manual().ringid() ?  m_oWeddingInfo.manual().ringid() : rCFG.dwRingItemID;//若有购买则使用购买的，否则用默认的

  std::list<ItemData> recordData;
  auto recordItem = [&](QWORD charid, const ItemData& data)
  {
    if (charid == m_oWeddingInfo.charid1())
      recordData.push_front(data);
    else
      recordData.push_back(data);
  };

  //结婚戒指
  ItemData itemdata;
  itemdata.mutable_base()->set_id(ringid);
  itemdata.mutable_base()->set_count(1);
  WeddingData* pWedData = itemdata.mutable_wedding();
  if (pWedData == nullptr)
    return;
  pWedData->set_id(m_oWeddingInfo.id());
  pWedData->set_zoneid(thisServer->getZoneID());
  pWedData->set_charid1(m_oWeddingInfo.charid1());
  pWedData->set_charid2(m_oWeddingInfo.charid2());
  pWedData->set_weddingtime(now());

  ItemData itemdata2;
  itemdata2.CopyFrom(itemdata);

  pWedData->set_myname(master->name);
  pWedData->set_partnername(follower->name);
  recordItem(master->id, itemdata);
  master->getPackage().addItem(itemdata, EPACKMETHOD_AVAILABLE);

  itemdata2.mutable_wedding()->set_myname(follower->name);
  itemdata2.mutable_wedding()->set_partnername(master->name);
  recordItem(follower->id, itemdata2);
  follower->getPackage().addItem(itemdata2, EPACKMETHOD_AVAILABLE);

  // 结婚证
  ItemData certificateItem;
  {
    certificateItem.mutable_base()->set_id(rCFG.dwWeddingCertificate);
    certificateItem.mutable_base()->set_count(1);
    WeddingData* pCerData = certificateItem.mutable_wedding();
    if (pWedData == nullptr)
      return;
    pCerData->set_id(m_oWeddingInfo.id());
    pCerData->set_zoneid(thisServer->getZoneID());
    pCerData->set_charid1(m_oWeddingInfo.charid1());
    pCerData->set_charid2(m_oWeddingInfo.charid2());
    pCerData->set_weddingtime(now());
  }

  ItemData certificateItem2;
  certificateItem2.CopyFrom(certificateItem);

  WeddingData* pCerData1 = certificateItem.mutable_wedding();//master
  WeddingData* pCerData2 = certificateItem2.mutable_wedding();//follower
  if (pCerData1 == nullptr || pCerData2 == nullptr)
    return;
  pCerData1->set_myname(master->name);
  pCerData1->set_partnername(follower->name);
  pCerData2->set_myname(follower->name);
  pCerData2->set_partnername(master->name);

  if (master->id == m_oWeddingInfo.charid1())
  {
    pCerData1->set_photoidx(m_oWeddingInfo.manual().photoindex1());
    pCerData1->set_phototime(m_oWeddingInfo.manual().phototime1());
    pCerData2->set_photoidx(m_oWeddingInfo.manual().photoindex2());
    pCerData2->set_phototime(m_oWeddingInfo.manual().phototime2());
  }
  else
  {
    pCerData1->set_photoidx(m_oWeddingInfo.manual().photoindex2());
    pCerData1->set_phototime(m_oWeddingInfo.manual().phototime2());
    pCerData2->set_photoidx(m_oWeddingInfo.manual().photoindex1());
    pCerData2->set_phototime(m_oWeddingInfo.manual().phototime1());
  }
  recordItem(master->id, certificateItem);
  recordItem(follower->id, certificateItem2);
  master->getPackage().addItem(certificateItem, EPACKMETHOD_AVAILABLE);
  follower->getPackage().addItem(certificateItem2, EPACKMETHOD_AVAILABLE);

  master->getAchieve().onWedding(EACHIEVECOND_WEDDING_CEREMONY);
  follower->getAchieve().onWedding(EACHIEVECOND_WEDDING_CEREMONY);

  MarrySCmd cmd;
  cmd.set_charid1(m_oWeddingInfo.charid1());
  cmd.set_charid2(m_oWeddingInfo.charid2());
  cmd.set_weddingid(m_oWeddingInfo.id());
  for (auto &s : recordData)
  {
    cmd.add_items()->CopyFrom(s);
  }
  PROTOBUF(cmd, send, len);

  thisServer->sendSCmdToWeddingServer(master->id, master->name, send, len);

  sendResult(true);

  WeddingSwitchQuestionCCmd cmd2;
  cmd2.set_onoff(false);
  PROTOBUF(cmd2, send2, len2);
  sendCmdToWeddingUser(send2, len2);

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwTempCeremonyNpcID);
  if (npc)
  {
    NpcChatNtf cmd;
    cmd.set_msgid(9643);
    cmd.set_channel(ECHAT_CHANNEL_ROUND);
    cmd.set_npcid(npc->getNpcID());
    cmd.set_npcguid(npc->id);
    PROTOBUF(cmd, send, len);
    npc->sendCmdToNine(send, len);
  }

  doServiceEffect(true);
  XLOG << "[婚礼-仪式], 结婚成功, 婚礼id:" << m_oWeddingInfo.id() << "玩家:" << m_oWeddingInfo.charid1() << m_oWeddingInfo.charid2() << "场景:" << id << "获得戒指ID:" << ringid << XEND;
}

void WeddingScene::onUserQuitQuestion(SceneUser* user)
{
  if (user == nullptr || isWeddingUser(user) == false)
    return;
  if (isInAnswer())
  {
    XLOG << "[婚礼-仪式], 玩家退出答题, 中断婚礼, 玩家:" << user->name << user->id << "婚礼:" << m_oWeddingInfo.id() << XEND;
    resetWedding();
  }
}

// ferriswheel scene
#include "SceneUserManager.h"
DivorceRollerCoasterScene::DivorceRollerCoasterScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :
  DScene(sID, sName, sBase, pRaidCFG)
{

}

bool DivorceRollerCoasterScene::init()
{
  return DScene::init();
}

void DivorceRollerCoasterScene::userEnter(SceneUser* user)
{
  if (user == nullptr)
    return;

  DScene::userEnter(user);
  if (0 == m_qwCarrierMasterID)
  {
    user->m_oCarrier.create(MiscConfig::getMe().getWeddingMiscCFG().dwDivorceCarrierId, 1, 0);
    m_qwCarrierMasterID = user->id;
  }
  else if ((QWORD)-1 == m_qwCarrierMasterID)
  {
  }
  else
  {
    SceneUser *master = SceneUserManager::getMe().getUserByID(m_qwCarrierMasterID);
    if (master)
    {
      master->m_oCarrier.m_oInvites.insert(user->id);
      master->m_oCarrier.join(user);
      master->m_oCarrier.start();
      m_qwCarrierMasterID = (QWORD)-1;
    }
  }
}

void DivorceRollerCoasterScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userLeave(user);

  if (m_qwCarrierMasterID && user->id == m_qwCarrierMasterID)
    m_qwCarrierMasterID = 0;
}

//********************************************************************
//******************PVE CARD RAID*************************************
//********************************************************************
PveCardScene::PveCardScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG, DWORD configid) : DScene(sID, sName, sBase, pRaidCFG), m_dwPveRaidConfgID(configid), m_oOneSecTimer(1)
{
}

bool PveCardScene::init()
{
  if (DScene::init() == false)
    return false;
  setCloseTime(PveCardConfig::getMe().getNextCreateTime());
  return true;
}

void PveCardScene::queryAllCardInfo(SceneUser* user)
{
  if (user == nullptr)
    return;
  const SPveRaidCFG* pCFG = PveCardConfig::getMe().getPveRaidCFGByID(m_dwPveRaidConfgID);
  if (pCFG == nullptr)
    return;

  DWORD index = 1;
  QueryCardInfoCmd cmd;
  for (auto &v : pCFG->vecCardInfo)
  {
    PveCardInfo* pInfo = cmd.add_cards();
    if (pInfo == nullptr)
      continue;
    pInfo->set_index(index);
    for (auto &va : v.vecAllCardIDs)
      pInfo->add_cardids(va);
  }
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

bool PveCardScene::selectCard(SceneUser* user, DWORD index)
{
  if (!user)
    return false;
  if (m_eState != EPVECARDSTATE_MIN)
    return false;
  index --; // 1,2,3 -> 0,1,2
  m_vecAllCards.clear();
  if (PveCardConfig::getMe().shuffleCard(m_dwPveRaidConfgID, index, m_vecAllCards) == false)
  {
    XERR << "[Pve-副本], 打乱卡牌异常, 地图:" << name << id << "玩家:" << user->name << user->id << "卡牌索引:" << m_dwPveRaidConfgID << index << XEND;
    return false;
  }

  SyncProcessPveCardCmd cmd;
  PveCardInfo* pInfo = cmd.mutable_card();
  if (pInfo)
  {
    pInfo->set_index(m_dwPveRaidConfgID);
    for (auto &v : m_vecAllCards)
      pInfo->add_cardids(v);
  }
  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);

  m_eState = EPVECARDSTATE_SELECTOK;

  XLOG << "[Pve-副本], 打乱卡牌成功, 地图:" << name << id << "玩家:" << user->name << user->id << "卡牌索引:" << m_dwPveRaidConfgID << index << XEND;
  XDBG << "[Pve-副本], 卡牌排序:";
  for (auto &v : m_vecAllCards)
    XDBG << v;
  XDBG << XEND;

  return true;
}

bool PveCardScene::beginPlayCard(SceneUser* user)
{
  if (!user)
    return false;

  if (m_eState != EPVECARDSTATE_SELECTOK)
    return false;

  sendDialogToUser(58557);

  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
  SceneNpc* pEnemyNpc = SceneNpcManager::getMe().getNpcByTempID(m_qwEnemyNpcID);
  if (pEnemyNpc)
  {
    GMCommandRuler::getMe().effect(pEnemyNpc, rPveMiscCFG.oPreGotoEffect);
    pEnemyNpc->goTo(rPveMiscCFG.oEnemyNpcDestPos);
    GMCommandRuler::getMe().effect(pEnemyNpc, rPveMiscCFG.oEndGotoEffect);
    // 转向
    NpcChangeAngle cmd2;
    cmd2.set_guid(pEnemyNpc->id);
    cmd2.set_angle(rPveMiscCFG.dwEnemyNpcDir);
    PROTOBUF(cmd2, send2, len2);
    pEnemyNpc->sendCmdToNine(send2, len2);
    pEnemyNpc->define.setDir(rPveMiscCFG.dwEnemyNpcDir);
    // 坐下
    UserActionNtf cmd;
    cmd.set_type(EUSERACTIONTYPE_MOTION);
    cmd.set_value(rPveMiscCFG.dwSitActionID);
    cmd.set_charid(pEnemyNpc->id);
    PROTOBUF(cmd, send, len);
    pEnemyNpc->sendCmdToNine(send, len);
    pEnemyNpc->setAction(3);
  }

  m_eState = EPVECARDSTATE_PLAYCARD;
  m_dwTimeTick = now() + rPveMiscCFG.dwPrepareCardTime;

  UpdateProcessPveCardCmd cmd;
  cmd.set_process(1); //即将打出的牌, 从1开始计数
  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);

  XLOG << "[Pve-副本], 开启战斗, 副本:" << name << id << "玩家:" << user->name << user->id << XEND;
  return true;
}

void PveCardScene::entryAction(QWORD curMSec)
{
  DScene::entryAction(curMSec);
  DWORD cur = curMSec / ONE_THOUSAND;
  if (m_oOneSecTimer.timeUp(cur))
  {
    const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
    if (cur >= m_dwTimeTick)
    {
      switch(m_eState)
      {
        case EPVECARDSTATE_MIN:
        case EPVECARDSTATE_SELECTOK:
          break;
        case EPVECARDSTATE_PLAYCARD:
          {
            if (m_bSummonedCardNpc == false)
            {
              summonCardNpc(false);
              m_dwTimeTick = cur + rPveMiscCFG.dwCardNpcShowTime;
              break;
            }
            showCardNpc();
            playCard();
            // 敌方npc出牌
            m_eState = EPVECARDSTATE_FRIENDPLAYCARD;

            m_dwTimeTick = cur + rPveMiscCFG.dwFriendCardDelay - rPveMiscCFG.dwCardNpcShowTime;

            checkFinishReward();
          }
          break;
        case EPVECARDSTATE_FRIENDPLAYCARD:
          {
            if (m_bSummonedCardNpc == false)
            {
              summonCardNpc(true);
              m_dwTimeTick = cur + rPveMiscCFG.dwCardNpcShowTime;
              break;
            }
            // 友方npc出牌
            showCardNpc();
            friendPlayCard();
            if (m_dwCardIndex == m_vecAllCards.size())
            {
              m_eState = EPVECARDSTATE_FINISHCARD;
              setCloseTime(cur + rPveMiscCFG.dwFinishCardCloseTime);
              if (!rPveMiscCFG.setSafeRange.empty())
              {
                m_dwSafeRange = *(rPveMiscCFG.setSafeRange.rbegin());
                m_dwTimeTick = cur + rPveMiscCFG.dwSafeShrinkInterval;
              }
              SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwSandglassNpcID);
              if (npc)
              {
                npc->playGearStatusToNine(2001, true);
              }
              XLOG << "[Pve-副本], 打牌结束, 进入倒计时阶段, 副本:" << name << id << XEND;
            }
            else
            {
              m_eState = EPVECARDSTATE_PLAYCARD;
              m_dwTimeTick = cur + rPveMiscCFG.dwPlayCardInterval - rPveMiscCFG.dwFriendCardDelay - rPveMiscCFG.dwCardNpcShowTime;
            }
          }
          break;
        case EPVECARDSTATE_FINISHCARD:
          {
            // shrink safe range
            for (auto s = rPveMiscCFG.setSafeRange.rbegin(); s != rPveMiscCFG.setSafeRange.rend(); ++s)
            {
              if (*s < m_dwSafeRange)
              {
                m_dwSafeRange = *s;
                m_dwShrinkIndex ++;
                break;
              }
            }
            m_dwTimeTick = cur + rPveMiscCFG.dwSafeShrinkInterval;

            if (m_dwShrinkIndex > 0 && m_dwShrinkIndex <= rPveMiscCFG.vecEffectPath.size())
            {
              // 召唤毒圈
              if (m_qwPosionEffectID)
              {
                /*SceneActBase* pAct = SceneActManager::getMe().getSceneAct(m_qwPosionEffectID);
                if (pAct)
                  pAct->setClearState();
                  */
              }
              SceneActBase* pAct = SceneActManager::getMe().createSceneAct(this, rPveMiscCFG.oSafeCentralPos, 0, 0, EACTTYPE_EFFECT);
              if (pAct == nullptr)
                break;
              SceneActEffect* pActEffect = dynamic_cast<SceneActEffect*> (pAct);
              if (pActEffect == nullptr)
              {
                SAFE_DELETE(pAct);
                break;
              }
              string effectstr = rPveMiscCFG.vecEffectPath[m_dwShrinkIndex - 1];
              pActEffect->setEffectInfo(effectstr, rPveMiscCFG.dwFinishCardCloseTime, 0, 0);
              //pActEffect->setClearTime(m_dwTimeTick);
              pActEffect->enterScene(this);
              m_qwPosionEffectID = pActEffect->id;
            }
            if (m_dwShrinkIndex > 0 && m_dwShrinkIndex < rPveMiscCFG.vecSandGlassAction.size())
            {
              // 沙漏npc动作
              SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwSandglassNpcID);
              if (npc)
              {
                DWORD actionid = rPveMiscCFG.vecSandGlassAction[m_dwShrinkIndex - 1];
                npc->playGearStatusToNine(actionid, true);
              }
            }
            XDBG << "[Pve-副本], 毒圈缩小, 当前范围:" << m_dwSafeRange << "副本:" << name << id << XEND;
          }
          break;
        case EPVECARDSTATE_OVER:
          {
            if (getDifficulty() < rPveMiscCFG.dwDeadBossMinDifficulty)
            {
              m_eState = EPVECARDSTATE_BOSSOVER;
              setCloseTime(now() + 30);
              break;
            }
            m_dwTimeTick = now() + rPveMiscCFG.dwWaitDeadNpcTime;
            m_eState = EPVECARDSTATE_DEADBOSS;
          }
          break;
        case EPVECARDSTATE_DEADBOSS:
          {
            // summon  npc for dead boss
            m_eState = EPVECARDSTATE_BOSSOVER;
            const SceneObject *pObject = getSceneObject();
            if (pObject == nullptr)
              break;
            const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(rPveMiscCFG.dwDeadBossNpcUID);
            if (pTemplate == nullptr)
              break;
            SceneNpc* npc = SceneNpcManager::getMe().createNpc(pTemplate->m_oDefine, this);
            if (npc)
            {
              XLOG << "[Pve-卡牌副本], 召唤亡者boss交互npc成功, 副本:" << name << id << "npc:" << npc->name << npc->id;
            }
          }
          break;
        case EPVECARDSTATE_BOSSOVER:
          break;
        default:
          break;
      }
    }

    float maxdis = rPveMiscCFG.dwMaxValidDis;
    // 离开战斗圆圈时, 强制传送到圈内
    if (maxdis)
    {
      xSceneEntrySet entrySet;
      getEntryList(rPveMiscCFG.oEnemyNpcDestPos, 10, entrySet);
      for (auto &s : entrySet)
      {
        xSceneEntryDynamic* entry = dynamic_cast<xSceneEntryDynamic*>(s);
        if (!entry)
          continue;
        if (entry->getEntryType() == SCENE_ENTRY_NPC)
        {
          SceneNpc* npc = dynamic_cast<SceneNpc*>(entry);
          if (npc && npc->isMonster() == false)
            continue;
        }
        if (getXZDistance(entry->getPos(), rPveMiscCFG.oSafeCentralPos) > maxdis)
        {
          xPos newpos = entry->getPos();
          if (getRandPos(rPveMiscCFG.oSafeCentralPos, maxdis/2, newpos))
            entry->goTo(newpos);
        }
      }
    }
    if (m_eState == EPVECARDSTATE_FINISHCARD)
    {
      xSceneEntrySet userset;
      getAllEntryList(SCENE_ENTRY_USER, userset);
      auto checkAddPosion = [&](xSceneEntryDynamic* entry)
      {
        if (!entry)
          return;
        if (getXZDistance(entry->getPos(), rPveMiscCFG.oSafeCentralPos) > m_dwSafeRange)
        {
          for (auto &s : rPveMiscCFG.setPosionBuff)
            entry->m_oBuff.add(s);
        }
      };
      for (auto &s : userset)
      {
        SceneUser* user = dynamic_cast<SceneUser*> (s);
        if (user)
        {
          checkAddPosion(user);
          std::list<SceneNpc*> petlist;
          user->getAllFriendNpcs(petlist);
          for (auto &l : petlist)
            checkAddPosion(l);
        }
      }
    }
  }
}

void PveCardScene::playCard()
{
  if (m_dwCardIndex + 3 > m_vecAllCards.size())
  {
    XERR << "[Pve-副本], 打牌错误, 溢出, index:" << m_dwCardIndex << "副本:" << name << id << XEND;
    return;
  }

  PlayPveCardCmd playcmd;
  playcmd.set_npcguid(m_qwEnemyNpcID);

  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
  for (DWORD i = 0; i < 3; ++i)
  {
    DWORD cardid = m_vecAllCards[i + m_dwCardIndex];
    playcmd.add_cardids(cardid);

    if (!rPveMiscCFG.vecCardPos.empty())
    {
      DWORD size = rPveMiscCFG.vecCardPos.size();
      m_oTempCurCardPos = rPveMiscCFG.vecCardPos[i % size];
    }

    if (doCardEffect(cardid))
      XLOG << "[Pve-副本], 打牌成功, 卡牌:" << cardid << "副本:" << name << id << XEND;
  }
  m_dwCardIndex += 3;

  // 打牌特效..todo
  PROTOBUF(playcmd, psend, plen);
  sendCmdToAll(psend, plen);

  // update process
  if (m_dwCardIndex <  m_vecAllCards.size()) // 尚未打完
  {
    UpdateProcessPveCardCmd cmd;
    cmd.set_process(m_dwCardIndex + 1); //即将打出的牌, 从1开始计数
    PROTOBUF(cmd, send, len);
    sendCmdToAll(send, len);
  }
  else // 已打完
  {
    FinishPlayCardCmd cmd;
    PROTOBUF(cmd, send, len);
    sendCmdToAll(send, len);
  }
}

void PveCardScene::friendPlayCard()
{
  const TSetDWORD& friendcards = PveCardConfig::getMe().getAllCardByType(EPVECARDTYPE_FRIEND);
  if (friendcards.empty())
    return;
  auto s = randomStlContainer(friendcards);
  if (!s)
    return;
  if (doCardEffect(*s))
    XLOG << "[Pve-副本], 友方打牌成功, 卡牌:" << *s << "副本:" << name << id << XEND;

  PlayPveCardCmd cmd;
  cmd.set_npcguid(m_qwFriendNpcID);
  cmd.add_cardids(*s);
  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);
}

bool PveCardScene::doCardEffect(DWORD cardid)
{
  const SPveCardCFG* pCardCFG = PveCardConfig::getMe().getPveCardCFG(cardid);
  if (pCardCFG == nullptr)
    return false;

  for (auto &v : pCardCFG->vecEffectIDs)
  {
    PveCardBase* pCardEffect = PveCardManager::getMe().getCardEffect(v);
    if (pCardEffect == nullptr)
    {
      XERR << "[Pve-副本], 打牌错误, 找不到对应的卡牌效果, 卡牌:" << cardid << "效果ID:" << v << "副本:" << name << id << XEND;
      return false;
    }
    pCardEffect->doEffect(this);
  }
  return true;
}

void PveCardScene::userEnter(SceneUser* user)
{
  DScene::userEnter(user);

  if (user == nullptr)
    return;
  // 拆卸神器
  user->getPackage().equipOffAritifact();
  // 变身解除
  user->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);

  if (m_vecAllCards.empty())
  {
    selectCard(user, 1);
  }

  if (!m_vecAllCards.empty())
  {
    SyncProcessPveCardCmd cmd;
    PveCardInfo* pInfo = cmd.mutable_card();
    if (pInfo)
    {
      pInfo->set_index(m_dwPveRaidConfgID);
      for (auto &v : m_vecAllCards)
        pInfo->add_cardids(v);
    }
    if (m_eState == EPVECARDSTATE_PLAYCARD || m_eState == EPVECARDSTATE_FRIENDPLAYCARD)
      cmd.set_process(m_dwCardIndex + 1);

    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
  }

  if (m_qwEnemyNpcID == 0 || m_qwFriendNpcID == 0 || m_qwSandglassNpcID == 0)
  {
    const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
    xSceneEntrySet set;
    getAllEntryList(SCENE_ENTRY_NPC, set);
    for (auto &s : set)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (!npc)
        continue;
      if (npc->getNpcID() == rPveMiscCFG.dwFriendNpcID)
        m_qwFriendNpcID = npc->id;
      else if (npc->getNpcID() == rPveMiscCFG.dwEnemyNpcID)
        m_qwEnemyNpcID = npc->id;
      else if (npc->getNpcID() == rPveMiscCFG.dwSandGlassNpcID)
        m_qwSandglassNpcID = npc->id;
    }
  }
}

void PveCardScene::checkFinishReward()
{
  // check是否结束
  switch(m_eState)
  {
    case EPVECARDSTATE_OVER:
    case EPVECARDSTATE_DEADBOSS:
    case EPVECARDSTATE_BOSSOVER:
      return;
    default:
      break;
  }

  if (m_eState != EPVECARDSTATE_OVER && isEnemyCardOver())
  {
    DWORD alivenum = getAllAliveMonsterNum();

    // 异常排查log
    if (alivenum < 3)
    {
      xSceneEntrySet set;
      getAllEntryList(SCENE_ENTRY_NPC, set);
      XLOG << "[Pve卡牌-怪物检查], 当前怪物:";
      for (auto &s : set)
      {
        XLOG << s->name << s->id;
      }
      XLOG << "副本:" << name << id << XEND;
    }

    if (alivenum != 0)
      return;

    sendDialogToUser(58560);
    sendDialogToUser(58561);

    // give reward
    // EVARTYPE_PVECARD_DIFFICULTY_1
    const SPveRaidCFG* pCFG = PveCardConfig::getMe().getPveRaidCFGByID(m_dwPveRaidConfgID);
    if (pCFG == nullptr)
      return;
    EVarType eType = EVARTYPE_PVECARD_DIFFICULTY_1;
    switch (pCFG->dwDifficulty)
    {
      case 1 :
        eType = EVARTYPE_PVECARD_DIFFICULTY_1;
        break;
      case 2 :
        eType = EVARTYPE_PVECARD_DIFFICULTY_2;
        break;
      case 3 :
        eType = EVARTYPE_PVECARD_DIFFICULTY_3;
        break;
      default:
        break;
    }

    const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
    xSceneEntrySet userset;
    getAllEntryList(SCENE_ENTRY_USER, userset);
    DWORD maxtimes = rPveMiscCFG.dwWeekRewardNum + ActivityEventManager::getMe().getExtraTimes(EAEREWARDMODE_PVECARD);
    std::set<SceneUser*> validUserSet;
    std::set<SceneUser*> invalidUserSet;
    for (auto &s : userset)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (s);
      if (user == nullptr)
        continue;
      if(eType == EVARTYPE_PVECARD_DIFFICULTY_1)
        user->getServant().onGrowthFinishEvent(ETRIGGER_PVE_CARD_SIMPLE);
      else if(eType == EVARTYPE_PVECARD_DIFFICULTY_2)
        user->getServant().onGrowthFinishEvent(ETRIGGER_PVE_CARD_MIDDLE);
      user->getServant().onGrowthFinishEvent(ETRIGGER_PVE_CARD);
      DWORD varnum = user->getVar().getVarValue(eType);
      if (varnum >= maxtimes)
      {
        invalidUserSet.insert(user);
        summonRewardBox(user, false);
        continue;
      }
      user->getVar().setVarValue(eType, 1 + varnum);
      validUserSet.insert(user);
      summonRewardBox(user, true);

      TVecItemInfo tempvec;
      DWORD times = user->getDoubleReward(EDOUBLEREWARD_PVECARD, tempvec);

      // 活动模板额外奖励/多倍奖励
      TVecItemInfo extrareward;
      if (times <= 1)
        ActivityEventManager::getMe().getReward(user, EAEREWARDMODE_PVECARD, pCFG->dwDifficulty, extrareward, times);

      for (auto &d : pCFG->setRewardID)
      {
        if (times > 1)
        {
          user->getPackage().rollReward(d, EPACKMETHOD_AVAILABLE, false, true, false, times);
          XLOG << "[Pve-副本], 通过奖励, 翻倍, 玩家:" << user->name << user->id << "副本:" << name << id << "倍数:" << times << XEND;
        }
        else
        {
          user->getPackage().rollReward(d, EPACKMETHOD_AVAILABLE);
        }
      }
      user->getExtraReward(EEXTRAREWARD_PVECARD);
      user->getServant().onFinishEvent(ETRIGGER_PVE_CARD);

      if (extrareward.empty() == false)
      {
        XLOG << "[Pve-副本]" << user->name << user->id << "活动额外奖励:";
        for (auto& v : extrareward)
          XLOG << v.id() << v.count();
        XLOG << XEND;
        user->getPackage().addItem(extrareward, EPACKMETHOD_AVAILABLE);
      }

      XLOG << "[Pve-副本], 通关成功, 玩家获取奖励, 玩家:" << user->name << user->id << "副本:" << m_dwPveRaidConfgID << XEND;
    }
    // 帮助奖励
    for (auto &s : invalidUserSet)
    {
      s->getHelpReward(validUserSet, EHELPTYPE_PVECARD);
    }
    m_eState = EPVECARDSTATE_OVER;
    setCloseTime(now() + 300);
  }
}

DWORD PveCardScene::getDifficulty() const
{
  const SPveRaidCFG* pCFG = PveCardConfig::getMe().getPveRaidCFGByID(m_dwPveRaidConfgID);
  if (pCFG == nullptr)
    return 0;
  return pCFG->dwDifficulty;
}

void PveCardScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  DScene::onNpcDie(npc, killer);

  if (npc)
    XLOG << "[Pve-卡牌副本], 怪物死亡, 检查结束, 怪物:" << npc->name << npc->id << "副本:" << name << id << XEND;

  checkFinishReward();
}

void PveCardScene::summonCardNpc(bool isFriend)
{
  m_bSummonedCardNpc = true;
  m_setSummonedCardNpc.clear();
  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
  if (isFriend)
  {
    NpcDefine def;
    def.setID(rPveMiscCFG.dwNormalCardNpcID);
    def.setPos(rPveMiscCFG.oFriendCardPos);
    SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
    if (npc == nullptr)
      return;
    npc->setDeadDelAtonce();

    npc->playGearStatusToNine(1001, true);

    m_setSummonedCardNpc.insert(npc->id);
  }
  else
  {
    if (rPveMiscCFG.vecCardPos.size() != 3)
      return;
    if (m_dwCardIndex + 3 > m_vecAllCards.size())
      return;
    for (DWORD i = 0; i < 3; ++i)
    {
      DWORD cardid = m_vecAllCards[m_dwCardIndex + i];
      const SPveCardCFG* pCardCFG = PveCardConfig::getMe().getPveCardCFG(cardid);
      if (pCardCFG == nullptr)
        continue;
      DWORD summonnpcid = (pCardCFG->eType == EPVECARDTYPE_BOSS ? rPveMiscCFG.dwBossCardNpcID : rPveMiscCFG.dwNormalCardNpcID);

      NpcDefine def;
      def.setID(summonnpcid);
      def.setPos(rPveMiscCFG.vecCardPos[i]);
      def.setUniqueID(cardid);
      def.setDir(rPveMiscCFG.dwEnemyCardDir);
      SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
      if (npc == nullptr)
        continue;
      npc->setDeadDelAtonce();
      npc->playGearStatusToNine(1001, true);

      m_setSummonedCardNpc.insert(npc->id);
    }
  }
}

void PveCardScene::showCardNpc()
{
  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
  for (auto &s : m_setSummonedCardNpc)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s);
    if (npc == nullptr)
      continue;
    npc->playGearStatusToNine(2001, true);

    npc->setClearTime(now() + rPveMiscCFG.dwCardNpcStayTime);
  }

  m_bSummonedCardNpc = false;
  m_setSummonedCardNpc.clear();
}

void PveCardScene::sendDialogToUser(DWORD dialogid)
{
  xSceneEntrySet userset;
  getAllEntryList(SCENE_ENTRY_USER, userset);

  UserActionNtf cmd;
  cmd.set_type(EUSERACTIONTYPE_DIALOG);
  cmd.set_value(dialogid);
  for (auto &s : userset)
  {
    SceneUser* user = dynamic_cast<SceneUser*>(s);
    if (!user)
      continue;
    cmd.set_charid(user->id);
    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
  }
}

void PveCardScene::onUserDie(SceneUser *user, xSceneEntryDynamic *killer)
{
  if (user == nullptr)
    return;
  DScene::onUserDie(user, killer);
  if (isAllDead())
  {
    sendDialogToUser(58558);
    sendDialogToUser(58559);
  }
}

void PveCardScene::summonRewardBox(SceneUser* pUser, bool bNeedReward)
{
  if (pUser == nullptr)
    return;
  const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
  NpcDefine oDefine;
  oDefine.load(rCFG.oRewardBoxDefine);

  xPos oPos = pUser->getPos();
  oDefine.setPos(oPos);
  oDefine.setRange(rCFG.fRewardBoxRange);
  oDefine.m_oVar.m_qwQuestOwnerID = pUser->id;

  SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, this);
  if (pNpc == nullptr)
  {
    XERR << "[卡牌副本-召唤宝箱], 玩家:" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "召唤 :" << oDefine.getID() << "失败" << XEND;
    return;
  }

  UserActionNtf cmd;
  cmd.set_charid(pNpc->tempid);
  cmd.set_value(bNeedReward ? rCFG.dwRewardBoxUngetAction : rCFG.dwRewardBoxGetAction);
  cmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  xLuaData data;
  data.setData("effect", rCFG.strRewardBoxEffect);
  data.setData("npcid", pNpc->getNpcID());
  data.setData("effectpos", 1);
  GMCommandRuler::effect(pUser, data);
}

void PveCardScene::getRandomDeadBoss(std::list<pair<DWORD,DWORD>>& bosslist)
{
  if (m_eState != EPVECARDSTATE_BOSSOVER)
    return;
  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
  const SPveRaidCFG* pCFG = PveCardConfig::getMe().getPveRaidCFGByID(m_dwPveRaidConfgID);
  if (pCFG == nullptr)
    return;
  for (DWORD i = 0; i < rPveMiscCFG.dwDeadBossNum; ++i)
  {
    DWORD lv = LuaManager::getMe().call<DWORD>("CalcRaidDeadBossSummonLevel", (DWORD)ERAID_DEADBOSSTYPE_PVECARD, pCFG->dwDifficulty);
    DWORD monsterid = NpcConfig::getMe().getOneRandomRaidDeadBoss(ERAID_DEADBOSSTYPE_PVECARD, lv);
    bosslist.push_back(std::make_pair(monsterid, rPveMiscCFG.dwDeadBossUID));
  }
}

bool PveCardScene::checkSummonDeadBoss()
{
  if (m_eState != EPVECARDSTATE_BOSSOVER)
    return false;
  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
  const SPveRaidCFG* pCFG = PveCardConfig::getMe().getPveRaidCFGByID(m_dwPveRaidConfgID);
  if (pCFG == nullptr)
    return false;
  return pCFG->dwDifficulty >= rPveMiscCFG.dwDeadBossMinDifficulty;
}

//****************************************************************
//*********************MvpBattle**********************************
//****************************************************************
MvpBattleScene::MvpBattleScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) : MatchScene(sID, sName, sBase, pRaidCFG), m_oOneSecTimer(1)
{

}

bool MvpBattleScene::init()
{
  if (MatchScene::init() == false)
    return false;

  const SMvpBattleCFG& rCFG = MiscConfig::getMe().getMvpBattleCFG();
  for (DWORD i = 0; i < rCFG.dwDeadBossNum; ++i)
  {
    DWORD time = randBetween(rCFG.paSummonDeadBossTime.first, rCFG.paSummonDeadBossTime.second);
    time += now();
    m_listDeadBossSummonTime.push_back(time);
  }
  m_listDeadBossSummonTime.sort();

  return true;
}

void MvpBattleScene::entryAction(QWORD curMSec)
{
  MatchScene::entryAction(curMSec);

  DWORD cur = curMSec / ONE_THOUSAND;
  if (m_oOneSecTimer.timeUp(cur))
  {
    if (m_listDeadBossSummonTime.empty() == false)
    {
      if (cur >= m_listDeadBossSummonTime.front())
      {
        m_listDeadBossSummonTime.pop_front();
        summonDeadBoss();
      }
    }
  }
}

void MvpBattleScene::summonDeadBoss()
{
  DWORD lv = LuaManager::getMe().call<DWORD>("CalcRaidDeadBossSummonLevel", (DWORD)ERAID_DEADBOSSTYPE_MVPBATTLE);
  DWORD monsterid = NpcConfig::getMe().getOneRandomRaidDeadBoss(ERAID_DEADBOSSTYPE_MVPBATTLE, lv);

  XLOG << "[Mvp-竞争战], 随机亡者boss, 等级:" << lv << "怪物ID:" << monsterid << "副本:" << name << id << XEND;

  const SMvpBattleCFG& rCFG = MiscConfig::getMe().getMvpBattleCFG();
  const TSetDWORD& uids = rCFG.setDeadBossUniqIDs;
  auto p = randomStlContainer(uids);
  if (!p)
    return;

  const SceneObject *pObject = getSceneObject();
  if (pObject == nullptr)
    return;

  const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(*p);
  if (pTemplate == nullptr)
    return;
  NpcDefine def = pTemplate->m_oDefine;
  def.setID(monsterid);

  SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
  if (npc)
  {
    m_vecDeadBossIDs.push_back(npc->getNpcID());
    XLOG << "[Mvp-竞争战], 亡者boss召唤成功, 怪物:" << npc->name << npc->id << "副本:" << name << id << XEND;
  }

  SyncMvpInfoFubenCmd cmd;
  cmd.set_usernum(m_dwUserNum);
  for (auto &v : m_vecLiveBossAndMini)
    cmd.add_liveboss(v);
  for (auto &v : m_vecDieBossAndMini)
    cmd.add_dieboss(v);
  for (auto &v : m_vecDeadBossIDs)
    cmd.add_liveboss(v);
  for (auto &v : m_vecDieDeadBossIDs)
    cmd.add_dieboss(v);
  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);
}

void MvpBattleScene::userEnter(SceneUser* user)
{
  MatchScene::userEnter(user);

  m_dwUserNum ++;
  if (m_dwTeamNum)
  {
    SyncMvpInfoFubenCmd cmd;
    cmd.set_usernum(m_dwUserNum);
    for (auto &v : m_vecLiveBossAndMini)
      cmd.add_liveboss(v);
    for (auto &v : m_vecDieBossAndMini)
      cmd.add_dieboss(v);
    for (auto &v : m_vecDeadBossIDs)
      cmd.add_liveboss(v);
    for (auto &v : m_vecDieDeadBossIDs)
      cmd.add_dieboss(v);

    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);

    // 更新人数显示
    UpdateUserNumFubenCmd cmd2;
    cmd2.set_usernum(m_dwUserNum);
    PROTOBUF(cmd2, send2, len2);
    sendCmdToAll(send2, len2);
  }

  if (user->getTeamID())
  {
    QWORD teamid = user->getTeamID();
    auto it = m_mapTeamMvpData.find(teamid);
    if (it != m_mapTeamMvpData.end())
    {
      it->second.strTeamName = user->getTeam().getName();
    }
    else
    {
      SMvpBattleTeamData& data = m_mapTeamMvpData[teamid];
      data.qwTeamID = teamid;
      data.strTeamName = user->getTeam().getName();
    }
  }
}

void MvpBattleScene::onUserGetTeamInfo(SceneUser* user)
{
  if (user == nullptr)
    return;
  if (user->getTeamID())
  {
    QWORD teamid = user->getTeamID();
    auto it = m_mapTeamMvpData.find(teamid);
    if (it != m_mapTeamMvpData.end())
    {
      it->second.strTeamName = user->getTeam().getName();
    }
    else
    {
      SMvpBattleTeamData& data = m_mapTeamMvpData[teamid];
      data.qwTeamID = teamid;
      data.strTeamName = user->getTeam().getName();
    }
  }
}

void MvpBattleScene::userLeave(SceneUser* user)
{
  MatchScene::userLeave(user);

  m_dwUserNum --;
  // 更新人数显示
  UpdateUserNumFubenCmd cmd2;
  cmd2.set_usernum(m_dwUserNum);
  PROTOBUF(cmd2, send2, len2);
  sendCmdToAll(send2, len2);

  user->getUserZone().setColorIndex(0);
}

void MvpBattleScene::onReceiveRoomInfo(const Cmd::SyncRoomSceneMatchSCmd& cmd)
{
  if (m_dwTeamNum != 0)
    return;
  m_dwTeamNum = cmd.roomsize();
  m_qwRoomID = cmd.roomid();

  const SceneObject *pObject = getSceneObject();
  if (pObject == nullptr)
  {
    XERR << "[Mvp-竞争战], 招怪失败, 取不到地图配置" << name << id << XEND;
    return;
  }

  //TODO
  pair<DWORD, DWORD> bossmininum;
  MiscConfig::getMe().getMvpBattleCFG().getBossAndMiniNum(m_dwTeamNum, bossmininum);
  auto summon = [&](ENpcType eType, DWORD num) ->bool
  {
    TVecDWORD vecUIDs;
    if (NpcConfig::getMe().getRandomMonsterByType(getMapID(), eType, num, vecUIDs) == false)
    {
      XERR << "[Mvp-竞争战], 随机mvp失败, 房间:" << cmd.roomid() << "地图:" << name << id << "数量:" << num << "类型" << eType << XEND;
      return false;
    }
    else
    {
      // 每次活动期间相同id副本招怪id一致, 设置随机数种子
      DWORD daytime = xTime::getDayStart(now());
      DWORD randseed = daytime * ONE_THOUSAND + getMapID();
      srand(randseed);

      for (auto &v : vecUIDs)
      {
        DWORD bossid = NpcConfig::getMe().getRandMonsterByGroup(getMapID(), v);
        if (bossid == 0)
        {
          XERR << "[Mvp-竞争战], 随机mvp失败,房间:" << cmd.roomid() << "地图:" << name << id << "Uid:" << v << "类型" << eType << XEND;
          continue;
        }
        const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(v);
        if (pTemplate == nullptr)
        {
          XERR << "[Mvp-竞争战], 随机怪物失败, 找不到对应的uniqueid配置,房间:" << cmd.roomid() << "地图:" << name << id << "Uid:" << v << "类型:" << eType << XEND;
          continue;
        }
        m_vecLiveBossAndMini.push_back(bossid);

        NpcDefine tmpDefine = pTemplate->m_oDefine;
        tmpDefine.setID(bossid);
        tmpDefine.setUniqueID(v);
        SceneNpc* npc = SceneNpcManager::getMe().createNpc(tmpDefine, this);
        if (npc)
          XLOG << "[Mvp-竞争战], 随机召唤怪物, 房间:" << cmd.roomid() << "地图:" << name << id << "怪物:" << bossid << npc->name << v << "类型" << eType << XEND;
      }
      // 重置随机数种子
      srand(xTime::getCurUSec());
    }
    return true;
  };
  if (bossmininum.first) // boss
    summon(ENPCTYPE_MVP, bossmininum.first);
  if (bossmininum.second)
    summon(ENPCTYPE_MINIBOSS, bossmininum.second);

  // if user already came, notify
  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (!entrySet.empty())
  {
    SyncMvpInfoFubenCmd cmd;
    cmd.set_usernum(entrySet.size());
    for (auto &v : m_vecLiveBossAndMini)
      cmd.add_liveboss(v);
    for (auto &v : m_vecDeadBossIDs)
      cmd.add_liveboss(v);
    PROTOBUF(cmd, send, len);
    sendCmdToAll(send, len);
  }

  XLOG << "[Mvp-竞争战], 收到房间信息, 房间:" << cmd.roomid() << "队伍数量:" << m_dwTeamNum << "地图:" << name << id << XEND;
}

void MvpBattleScene::onNpcDieReward(SceneNpc* npc, SceneUser* user)
{
  if (npc == nullptr || user == nullptr)
    return;
  DWORD npcid = npc->getNpcID();
  bool end = false;
  auto it = find(m_vecLiveBossAndMini.begin(), m_vecLiveBossAndMini.end(), npcid);
  if (it != m_vecLiveBossAndMini.end())
  {
    const SRandomMonsterCFG* pCFG = NpcConfig::getMe().getRandomMonsterCFG(npcid);
    if (pCFG == nullptr)
    {
      XERR << "[Mvp-竞争战], 怪物死亡异常, 找不到怪物信息:" << npc->name << npc->id << npcid << "地图:" << name << id << "房间:" << m_qwRoomID << XEND;
      return;
    }
    const SMvpBattleCFG& rCFG = MiscConfig::getMe().getMvpBattleCFG();
    DWORD maxcnt = (pCFG->eNpcType == ENPCTYPE_MVP ? rCFG.dwMaxBossRewardCnt : rCFG.dwMaxMiniRewardCnt);
    EVarType vartype = (pCFG->eNpcType == ENPCTYPE_MVP ? EVARTYPE_MVPREWARDNUM : EVARTYPE_MINIREWARDNUM);

    user->getServant().onGrowthFinishEvent(ETRIGGER_MVP_BATTLE_KILL);

    std::set<SceneUser*> teamusers = user->getTeamSceneUser();
    for (auto &s : teamusers)
    {
      DWORD var = s->getVar().getVarValue(vartype);
      if (var >= maxcnt)
      {
        DWORD msgid = (pCFG->eNpcType == ENPCTYPE_MVP ? 7314 : 7315);
        MsgManager::sendMsg(s->id, msgid);
        XLOG << "[Mvp-竞争战], 奖励, 玩家奖励次数已达上限, 玩家:" << s->name << s->id << "次数:" << var << "房间:" << m_qwRoomID << "怪物:" << npc->name << npc->id << XEND;
        continue;
      }
      s->getVar().setVarValue(vartype, var + 1);
      for (auto &d : pCFG->setBossReward)
      {
        s->getPackage().rollReward(d, EPACKMETHOD_AVAILABLE, false, true, false);
      }
      XLOG << "[Mvp-竞争战], 玩家获取奖励, 玩家:" << s->name << s->id << "次数:" << var + 1 << "房间:" << m_qwRoomID << "怪物:" << npc->name << npc->id << XEND;
    }

    /*记录队伍击杀*/
    auto team = m_mapTeamMvpData.find(user->getTeamID());
    if (team != m_mapTeamMvpData.end())
    {
      if (pCFG->eNpcType == ENPCTYPE_MVP)
        team->second.vecKillMvpIDs.push_back(npcid);
      else
        team->second.vecKillMiniIDs.push_back(npcid);
    }

    // update bossnum to client
    BossDieFubenCmd cmd;
    cmd.set_npcid(npcid);
    PROTOBUF(cmd, send, len);
    sendCmdToAll(send, len);
    MsgManager::sendMapMsg(this->id, 25603, MsgParams(user->getTeam().getLeaderName(), npc->name));

    m_vecDieBossAndMini.push_back(npcid);
    m_vecLiveBossAndMini.erase(it);

    // 击杀mvp后清除CD
    if (pCFG->eNpcType == ENPCTYPE_MVP && m_setClearCDTeams.find(user->getTeamID()) == m_setClearCDTeams.end())
    {
      ClearMvpCDMatchSCmd mcmd;
      mcmd.set_roomid(m_qwRoomID);
      mcmd.set_teamid(user->getTeamID());
      PROTOBUF(mcmd, msend, mlen);
      thisServer->sendCmdToSession(msend, mlen);
      m_setClearCDTeams.insert(user->getTeamID());
    }
    // 战斗结束
    if (m_vecLiveBossAndMini.empty() && m_vecDeadBossIDs.empty() && m_listDeadBossSummonTime.empty())
    {
      end = true;
    }
    XLOG << "[Mvp-竞争战], 怪物死亡, 怪物:" << npc->name << npc->id << "房间:" << m_qwRoomID << "击杀归属玩家:" << user->name << user->id << XEND;
  }
  else
  {
    auto its = find(m_vecDeadBossIDs.begin(), m_vecDeadBossIDs.end(), npc->getNpcID());
    if (its != m_vecDeadBossIDs.end())
    {
      BossDieFubenCmd cmd;
      cmd.set_npcid(npc->getNpcID());
      PROTOBUF(cmd, send, len);
      sendCmdToAll(send, len);

      m_vecDeadBossIDs.erase(its);
      m_vecDieDeadBossIDs.push_back(npc->getNpcID());
      if (m_vecDeadBossIDs.empty() && m_vecLiveBossAndMini.empty() && m_listDeadBossSummonTime.empty())
      {
        end = true;
      }

      XLOG << "[Mvp-竞争战], 亡者boss击杀, 玩家:" << user->name << user->id << "怪物:" << npc->name << npc->id << "副本:" << name << id << XEND;

      const SRaidDeadBossCFG* pCFG = NpcConfig::getMe().getRaidDeadBossCFG(npc->getNpcID());
      if (pCFG == nullptr)
        return;

      const SMvpBattleCFG& rCFG = MiscConfig::getMe().getMvpBattleCFG();
      std::set<SceneUser*> teamusers = user->getTeamSceneUser();
      for (auto &s : teamusers)
      {
        DWORD cnt = s->getVar().getVarValue(EVARTYPE_DEADBOSS_COUNT_MVP);
        if (cnt >= rCFG.dwDeadBossKillTime)
        {
          XLOG << "[Mvp-竞争战], 亡者boss击杀, 玩家已没有剩余次数, 玩家:" << s->name << s->id << "副本:" << name << id << XEND;
          continue;
        }

        s->getVar().setVarValue(EVARTYPE_DEADBOSS_COUNT_MVP, cnt + 1);
        for (auto &d : pCFG->setRewards)
        {
          s->getPackage().rollReward(d, EPACKMETHOD_AVAILABLE, false, true, false);
        }
        XLOG << "[Mvp-竞争战], 亡者boss击杀, 玩家获取奖励, 玩家:" << s->name << s->id << "boss:" << npc->name << npc->id << "副本:" << name << id << XEND;
      }

      /*记录队伍击杀*/
      auto team = m_mapTeamMvpData.find(user->getTeamID());
      if (team != m_mapTeamMvpData.end())
      {
        team->second.vecKillDeadBoss.push_back(npc->getNpcID());
      }
    }
  }

  if (end)
  {
    const SMvpBattleCFG& rCFG = MiscConfig::getMe().getMvpBattleCFG();
    xSceneEntrySet userset;
    getAllEntryList(SCENE_ENTRY_USER, userset);
    UserActionNtf cmd;
    cmd.set_type(EUSERACTIONTYPE_DIALOG);
    cmd.set_value(rCFG.dwEndDialogID);
    for (auto &s : userset)
    {
      cmd.set_charid(s->id);
      PROTOBUF(cmd, send, len);
      ((SceneUser*)s)->sendCmdToMe(send, len);
      ((SceneUser*)s)->getServant().onFinishEvent(ETRIGGER_MVP_BATTLE);
      ((SceneUser*)s)->getServant().onGrowthFinishEvent(ETRIGGER_MVP_BATTLE);
    }

    setCloseTime(now() + rCFG.dwEndWaitTime);
    ntfMatchCloseRoom();
    MsgManager::sendMapMsg(this->id, 7308);
    reportEndInfo();
    XLOG << "[Mvp-竞争战], 所有boss和mini均已被击杀, 战斗结束, 房间:" << m_qwRoomID << XEND;
  }
  XLOG << "[Mvp-竞争战], 怪物死亡, 怪物:" << npc->name << npc->id << "房间:" << m_qwRoomID << "击杀归属玩家:" << user->name << user->id << XEND;
}

void MvpBattleScene::getBornPos(SceneUser* pUser, xPos& pos)
{
  if (pUser == nullptr)
    return;
  DWORD index = pUser->getUserZone().getColorIndex();
  auto it = m_mapTeamBornPos.find(index);
  if (it != m_mapTeamBornPos.end())
  {
    pos = it->second;
    return;
  }

  getRandPos(pos);
  m_mapTeamBornPos[index] = pos;
}

void MvpBattleScene::getRelivePos(SceneUser* pUser, xPos& pos)
{
  if (pUser == nullptr)
    return;
  QWORD teamid = pUser->getTeamID();
  auto it = m_mapTeamRelivePos.find(teamid);
  if (it != m_mapTeamRelivePos.end())
  {
    pos = it->second;
    return;
  }

  // 5支队伍, 3个bp, :1,2,3,1,2
  tempReliveIndex ++;
  DWORD size = m_mapPos.size();
  if (size == 0)
    getRandPos(pos);
  else
  {
    tempReliveIndex %= size;
  }
  DWORD i = 0;
  for (auto &m : m_mapPos)
  {
    if (i++ == tempReliveIndex)
    {
      pos = m.second;
      m_mapTeamRelivePos[teamid] = m.second;
      return;
    }
  }
}

void MvpBattleScene::onKillUser(SceneUser* pKiller, SceneUser* pDeath)
{
  if (pKiller == nullptr || pDeath == nullptr)
    return;
  auto it = m_mapTeamMvpData.find(pKiller->getTeamID());
  if (it != m_mapTeamMvpData.end())
    it->second.dwKillUserNum ++;
}

void SMvpBattleTeamData::toData(MvpBattleTeamData* pData)
{
  if (pData == nullptr)
    return;
  pData->set_teamid(qwTeamID);
  pData->set_teamname(strTeamName);
  pData->set_killusernum(dwKillUserNum);
  for (auto &v : vecKillMvpIDs)
    pData->add_killmvps(v);
  for (auto &v : vecKillMiniIDs)
    pData->add_killminis(v);
  for (auto &v : vecKillDeadBoss)
    pData->add_deadboss(v);
}

void MvpBattleScene::reportEndInfo()
{
  MvpBattleReportFubenCmd cmd;
  for (auto &m : m_mapTeamMvpData)
  {
    m.second.toData(cmd.add_datas());
  }

  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);
}


//*****************************************************************************
//***************************公会战决战****************************************
//*****************************************************************************

void SSuperGvgGuildData::toData(GvgCrystalInfo* pInfo)
{
  if (pInfo == nullptr)
    return;
  pInfo->set_rank(dwRank);
  pInfo->set_guildid(qwGuildID);
  pInfo->set_crystalnum(dwCrystalNum);
  pInfo->set_chipnum(dwChipNum);
}

void SSuperGvgGuildData::toData(GvgGuildInfo* pInfo)
{
  if (pInfo == nullptr)
    return;
  pInfo->set_index(dwColor);
  pInfo->set_guildid(qwGuildID);
  pInfo->set_guildname(strGuildName);
  pInfo->set_icon(strGuildIcon);
  pInfo->set_metal_live(bMetalLive);

  toData(pInfo->mutable_crystal());
}

void SGvgTowerData::toData(GvgTowerData* pData)
{
  if (pData == nullptr)
    return;
  pData->set_etype(eType);
  pData->set_estate(eState);
  pData->set_owner_guild(qwCurOwnerGuildID);

  for (auto &m : mapGuild2Value)
  {
    GvgTowerValue* pValue = pData->add_info();
    if (pValue == nullptr)
      continue;
    pValue->set_guildid(m.first);
    pValue->set_value(m.second);
  }
}

void SSugvgUserData::toData(SuperGvgUserData* pData)
{
  if (pData == nullptr)
    return;
  pData->set_username(strUserName);
  pData->set_profession(dwProfession);

  pData->set_killusernum(dwKillUserNum);
  pData->set_dienum(dwDieNum);
  pData->set_chipnum(dwChipNum);
  pData->set_towertime(dwTowerTime);
  pData->set_healhp(dwHealHp);
  pData->set_relivenum(dwReliveNum);
  pData->set_metaldamage(dwMetalDamage);
}

SuperGvgScene::SuperGvgScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) : MatchScene(sID, sName, sBase, pRaidCFG), m_oOneSecTimer(1)
{

}

SuperGvgScene::~SuperGvgScene()
{

}

bool SuperGvgScene::init()
{
  if (MatchScene::init() == false)
    return false;

  // 晶塔
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  for (auto &m : rCFG.mapTowerCFG)
  {
    auto v = m.second;
    SGvgTowerData& data = m_mapTowerData[v.eType];
    data.eType = v.eType;
    data.eState = EGVGTOWERSTATE_INITFREE;
    data.dwValue = v.dwAllValue;
    data.fRange = v.fRange;
    data.pos = v.pos;

    NpcDefine def;
    def.setID(v.dwShowNpcID);
    def.setPos(v.pos);
    SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
    if (npc)
    {
      data.qwShowNpcID = npc->id;
      npc->setGearStatus(rCFG.dwNpcDefaultActionID + 1);
    }
  }

  addFreeSkill(rCFG.dwExpelSkill);

  return true;
}

void SuperGvgScene::onReceiveRoomInfo(const Cmd::SyncRoomSceneMatchSCmd& cmd)
{
  m_qwRoomID = cmd.roomid();
  m_dwRoomLevel = cmd.level();
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  m_dwOpenFireTime = cmd.raidtime() + rCFG.dwMatchToFireTime;
  m_dwStopFireTime = m_dwOpenFireTime + 30 * 60;
  m_dwTowerOpenTime = rCFG.dwTowerOpenTime + m_dwOpenFireTime;

  if (now() >= m_dwOpenFireTime)
    m_eSGvgState = ESGVGSTATE_FIRE;
  else
    m_eSGvgState = ESGVGSTATE_PREPARE;

  for (int i = 0; i < cmd.sugvgdata_size(); ++i)
  {
    const SuperGvgRoomData& data = cmd.sugvgdata(i);
    SSuperGvgGuildData& stData = m_mapGuild2Data[data.guildid()];
    stData.qwGuildID = data.guildid();
    stData.dwColor = data.color();
    stData.strGuildName = data.guildname();
    stData.strGuildIcon = data.guildicon();
    stData.dwFireCount = data.firecount();
    stData.dwFireScore = data.firescore();
  }
  // create metal npc
  do
  {
    const SceneObject *pObject = getSceneObject();
    if (pObject == nullptr)
      break;
    for (auto &m : m_mapGuild2Data)
    {
      const SGvgCampCFG* pCFG = rCFG.getCampCFG(m.second.dwColor);
      if (pCFG == nullptr)
        continue;

      // 水晶
      const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(pCFG->dwMetalUniqID);
      if (pTemplate == nullptr)
        continue;
      NpcDefine def = pTemplate->m_oDefine;
      def.m_oVar.m_qwGuildID = m.first;
      SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
      if (npc == nullptr)
        continue;
      // 添加无敌buff
      npc->m_oBuff.add(rCFG.dwMetalGodBuff, npc, 0, 0, (QWORD)(m_dwOpenFireTime + rCFG.dwMetalGodTime) * ONE_THOUSAND);

      m.second.qwMetalNpcID = npc->id;
      m.second.dwCrystalNum = 1; // 设置初始一个水晶
      //m.second.dwRank = m.second.dwColor; // 初始排名按颜色
      m.second.dwBpID = pCFG->dwBpID;
      m.second.dwNpcShowActionID = pCFG->dwNpcShowActionID;
      m.second.strColorName = pCFG->strColorName;

      XLOG << "[公会战决战-副本], 召唤华丽金属, 地图:" << name << id << "华丽金属:" << npc->name << npc->id << XEND;

      // 空气墙
      pTemplate = pObject->getRaidNpcTemplate(pCFG->dwGearUniqID);
      if (pTemplate == nullptr)
        continue;
      SceneNpc* gearnpc = SceneNpcManager::getMe().createNpc(pTemplate->m_oDefine, this);
      if (gearnpc == nullptr)
        continue;
      if (m_eSGvgState == ESGVGSTATE_PREPARE)
        m_oGear.set(pCFG->dwGearUniqID, 1, nullptr); // 设置空气墙
      else
        m_oGear.set(pCFG->dwGearUniqID, 2, nullptr); // 设置空气墙
    }
  } while(0);

  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  if (!userSet.empty())
  {
    for (auto &s : userSet)
    {
      SceneUser* user = dynamic_cast<SceneUser*>(s);
      if (user == nullptr)
        continue;
      auto it = m_mapPos.find(user->getUserZone().getColorIndex());
      if (it != m_mapPos.end())
        user->goTo(it->second);

      syncAllDataToUser(user);
    }
  }

  XLOG << "[公会战决战-副本], 收到房间信息, 房间:" << m_qwRoomID << "段位:" << m_dwRoomLevel << "地图:" << name << id << "公会信息:";
  for (auto &m : m_mapGuild2Data)
    XLOG << m.first << m.second.dwColor;
  XLOG << XEND;
}

void SuperGvgScene::entryAction(QWORD curMSec)
{
  MatchScene::entryAction(curMSec);

  DWORD cur = curMSec / ONE_THOUSAND;
  if (m_oOneSecTimer.timeUp(cur))
  {
    switch (m_eSGvgState)
    {
      case ESGVGSTATE_MIN:
        break;
      case ESGVGSTATE_PREPARE:
        if (cur >= m_dwOpenFireTime)
        {
          const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
          m_dwNorthBossTimeTick = cur + rCFG.dwFirstBossTime;
          m_dwSouthBossTimeTick = cur + rCFG.dwFirstBossTime;
          addTimeBossMsg(m_dwNorthBossTimeTick, 25506);

          // 移除空气墙
          clearBarrier();
          m_eSGvgState = ESGVGSTATE_FIRE;
          MsgManager::sendMapMsg(id, 25519);
        }
        else if (m_dwOpenFireTime - cur == 60)
        {
          // 战斗开始倒计时
          MsgManager::sendMapMsg(id, 25504, MsgParams(60));
        }
        break;
      case ESGVGSTATE_FIRE:
        {
          // 副本计时结束
          if (cur >= m_dwStopFireTime)
          {
            onTimeUp();
            m_eSGvgState = ESGVGSTATE_END;
            break;
          }
          // 占领圈
          updateTower(cur);
          if (!m_setUpdateGuildCrystals.empty())
          {
            updateRank();
            checkWin();
          }
          // boss
          if (cur >= m_dwNorthBossTimeTick)
          {
            summonBoss(false);
            const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
            m_dwNorthBossTimeTick = randBetween(rCFG.dwMinBossInterval, rCFG.dwMaxBossInterval) + cur;
            addTimeBossMsg(cur + rCFG.dwMinBossInterval, 25507);
          }
          if (cur >= m_dwSouthBossTimeTick)
          {
            summonBoss(true);
            const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
            m_dwSouthBossTimeTick = randBetween(rCFG.dwMinBossInterval, rCFG.dwMaxBossInterval) + cur;
            addTimeBossMsg(cur + rCFG.dwMinBossInterval, 25508);
          }
          checkBossMsg(cur); // boss出现提示

          // 场景效果
          if (cur >= m_dwSceneEffectTimeTick)
          {
            if (m_dwSceneEffectTimeTick)
              doSceneEffect();
            const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
            m_dwSceneEffectTimeTick  = cur + rCFG.dwBuffEffectInterval;
          }
        }
        break;
      case ESGVGSTATE_END:
        break;
    }
  }
}

void SuperGvgScene::updateTower(DWORD cur)
{
  // 等待开启
  if (cur < m_dwTowerOpenTime)
  {
    if (m_dwTowerOpenTime - cur == 60)
      MsgManager::sendMapMsg(id, 25520, MsgParams(60));
    return;
  }

  GvgTowerUpdateFubenCmd allCmd;
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  auto exec = [&](SGvgTowerData& d)
  {
    xSceneEntrySet userSet;
    getEntryListInBlock(SCENE_ENTRY_USER, d.pos, d.fRange, userSet);
    if (userSet.empty())
      return;
    map<QWORD, DWORD> guild2num;
    map<QWORD, TSetQWORD> validUsers;
    for (auto &s : userSet)
    {
      SceneUser* user = dynamic_cast<SceneUser*>(s);
      if (!user || user->isAlive() == false)
        continue;
      QWORD guildid = user->getGuild().id();
      if (!guildid)
        continue;
      guild2num[guildid] ++;
      validUsers[guildid].insert(user->id);
    }
    // 找到人数最多公会, 若最多有多个, 不发生变化
    QWORD maxGuild = 0;
    DWORD maxNum = 0;
    for (auto &it : guild2num)
    {
      if (it.second > maxNum)
      {
        maxGuild = it.first;
        maxNum = it.second;
      }
    }
    DWORD gnum = 0;
    for (auto& it : guild2num)
    {
      if (it.second == maxNum)
        gnum++;
    }
    // 最多有多个
    if (gnum > 1 || maxGuild == 0)
      return;

    // 上一秒不是此公会
    if (d.qwLastGuildID != maxGuild)
    {
      d.qwLastGuildID = maxGuild;
      return;
    }

    const SGvgTowerCFG* pCFG = rCFG.getTowerCFG(d.eType);
    if (pCFG == nullptr)
      return;
    const SGvgTowerCFG& r = *pCFG;

    bool syncAll = false;/*是否同步所有玩家, 仅当平台占领者变化时为true*/
    DWORD speed = r.getSpeed(guild2num.size(), maxNum);
    switch(d.eState)
    {
      case EGVGTOWERSTATE_INITFREE:
        {
          speed = (d.dwValue < speed ? d.dwValue : speed);
          d.dwValue -= speed;
          d.mapGuild2Value[maxGuild] += speed;
          if (d.dwValue == 0)
          {
            if (d.mapGuild2Value[maxGuild] == r.dwAllValue)
            {
              d.eState = EGVGTOWERSTATE_OCCUPY;
              d.qwCurOwnerGuildID = maxGuild;
              onTakeTower(r, maxGuild);
              syncAll = true;
            }
            else
            {
              d.eState = EGVGTOWERSTATE_FREE;
            }
          }
        }
        break;
      case EGVGTOWERSTATE_OCCUPY:
      case EGVGTOWERSTATE_FREE:
        {
          DWORD validsize = 0;
          for (auto &it : d.mapGuild2Value)
          {
            if (it.second && maxGuild != it.first)
              validsize ++;
          }
          if (validsize < 1)
            return;
          DWORD dec = speed / validsize;
          DWORD realSpd = 0;
          for (auto &it : d.mapGuild2Value)
          {
            if (it.first == maxGuild)
              continue;
            if (it.second >= dec)
            {
              it.second -= dec;
              realSpd += dec;
            }
            else // 不够减
            {
              realSpd += it.second;
              it.second = 0;
            }
          }
          if (realSpd == 0)
            return;

          DWORD& value = d.mapGuild2Value[maxGuild];
          value += realSpd;
          if (d.eState == EGVGTOWERSTATE_OCCUPY)
          {
            if (d.qwCurOwnerGuildID != maxGuild)
            {
              if (value == r.dwAllValue) // 更换占据
              {
                onTakeTower(r, maxGuild);
                onLoseTower(r, d.qwCurOwnerGuildID, false);
                d.qwCurOwnerGuildID = maxGuild;
                syncAll = true;
              }
              else if (d.mapGuild2Value[d.qwCurOwnerGuildID] == 0) // 老的公会被丢弃
              {
                onLoseTower(r, d.qwCurOwnerGuildID, true);
                d.eState = EGVGTOWERSTATE_FREE;
                syncAll = true;
              }
            }
          }
          else
          {
            if (value == r.dwAllValue) // 新占据
            {
              d.eState = EGVGTOWERSTATE_OCCUPY;
              onTakeTower(r, maxGuild);
              d.qwCurOwnerGuildID = maxGuild;
              syncAll = true;
            }
          }
        }
        break;
    }
    // send to client
    GvgTowerUpdateFubenCmd cmd;
    d.toData(cmd.add_towers());
    PROTOBUF(cmd, send, len);
    if (syncAll)
    {
      sendCmdToAll(send, len);
    }
    else
    {
      for (auto &s : d.setSyncUsers)
      {
        if (m_setSyncUsers.find(s) != m_setSyncUsers.end())
          continue;
        s->sendCmdToMe(send, len);
      }
      if (!m_setSyncUsers.empty())
        d.toData(allCmd.add_towers());
    }

    // 记录玩家占塔时间
    TSetQWORD& users = validUsers[maxGuild];
    for (auto &s : users)
    {
      auto it = m_mapSuGvgUserData.find(s);
      if (it != m_mapSuGvgUserData.end())
        it->second.dwTowerTime ++;
    }
  };

  for (auto &m : m_mapTowerData)
  {
    exec(m.second);
  }

  if (!m_setSyncUsers.empty() && allCmd.towers_size())
  {
    PROTOBUF(allCmd, send, len);
    for (auto &s : m_setSyncUsers)
      s->sendCmdToMe(send, len);
  }
}

void SuperGvgScene::onTakeTower(const SGvgTowerCFG& tower, QWORD guildid)
{
  auto it = m_mapGuild2Data.find(guildid);
  if (it == m_mapGuild2Data.end())
    return;
  SSuperGvgGuildData& data = it->second;

  data.dwCrystalNum += tower.dwCrystalNum;
  data.qwGetScoreTime = xTime::getCurUSec();
  // notice
  onAddCrystal(data);

  MsgParams param;
  param.addString(data.strColorName);
  param.addString(data.strGuildName);
  param.addString(tower.strTowerName);
  param.addNumber(tower.dwCrystalNum);
  MsgManager::sendMapMsg(id, 25505, param);

  m_setUpdateGuildCrystals.insert(guildid);
  m_bOpenRank = true;

  auto m = m_mapTowerData.find(tower.eType);
  if (m != m_mapTowerData.end())
  {
    // 占领圈切换颜色
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m->second.qwShowNpcID);
    if (npc)
      npc->playGearStatusToNine(data.dwNpcShowActionID);
  }
  XLOG << "[公会战决战-占领塔], 塔类型:" << tower.eType << "公会:" << guildid << "水晶数量:" << data.dwCrystalNum << "副本:" << m_qwRoomID << id << XEND;
}

void SuperGvgScene::onLoseTower(const SGvgTowerCFG& tower, QWORD guildid, bool empty)
{
  auto it = m_mapGuild2Data.find(guildid);
  if (it == m_mapGuild2Data.end())
    return;
  SSuperGvgGuildData& data = it->second;

  data.dwCrystalNum = (data.dwCrystalNum > tower.dwCrystalNum ? data.dwCrystalNum - tower.dwCrystalNum : 0);
  // notice

  m_setUpdateGuildCrystals.insert(guildid);

  // 无人占领
  if (empty)
  {
    auto m = m_mapTowerData.find(tower.eType);
    if (m != m_mapTowerData.end())
    {
      const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
      // 占领圈切换颜色
      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m->second.qwShowNpcID);
      if (npc)
        npc->playGearStatusToNine(rCFG.dwNpcDefaultActionID);
    }
  }

  XLOG << "[公会战决战-丢失塔], 塔类型:" << tower.eType << "公会:" << guildid << "水晶数量:" << data.dwCrystalNum << "副本:" << m_qwRoomID << id << XEND;
}

void SuperGvgScene::updateRank(bool bForce /*=false*/)
{
  if (!bForce && m_setUpdateGuildCrystals.empty())
    return;
  if (m_bOpenRank)
  {
    vector<SSuperGvgGuildData*> vec;
    for (auto &m : m_mapGuild2Data)
    {
      // 水晶数为0, 均排名4
      if (m.second.dwCrystalNum == 0)
      {
        if (m.second.dwRank != 4)
        {
          m.second.dwRank = 4;
          m_setUpdateGuildCrystals.insert(m.second.qwGuildID); // 需要更新
        }
        continue;
      }
      // 水晶数为1, 且没有过获取记录, 均排名3
      if (m.second.dwCrystalNum == 1 && m.second.qwGetScoreTime == 0)
      {
        if (m.second.dwRank != 3)
        {
          m.second.dwRank = 3;
          m_setUpdateGuildCrystals.insert(m.second.qwGuildID); // 需要更新
        }
        continue;
      }
      // 水晶数 >= 1, 且有获取记录, 按正常规则排名
      vec.push_back(&(m.second));
    }
    std::sort(vec.begin(), vec.end(), [&](SSuperGvgGuildData* p1, SSuperGvgGuildData* p2) -> bool
    {
      if (p1->dwCrystalNum != p2->dwCrystalNum)
        return p1->dwCrystalNum > p2->dwCrystalNum;
      if (p1->qwGetScoreTime != p2->qwGetScoreTime)
      {
        if (p1->qwGetScoreTime == 0)
          return false;
        if (p2->qwGetScoreTime == 0)
          return true;
        return p1->qwGetScoreTime < p2->qwGetScoreTime; //先获取积分的公会排名高
      }
      return randBetween(1, 100) > 50; // 不太可能出现
    });
    DWORD rank = 0;
    for (auto &p : vec)
    {
      rank ++;
      if (p->dwRank != rank)
      {
        m_setUpdateGuildCrystals.insert(p->qwGuildID); // 需要更新
        p->dwRank = rank;
      }
    }
  }
  GvgCrystalUpdateFubenCmd cmd;
  for (auto &s : m_setUpdateGuildCrystals)
  {
    auto it = m_mapGuild2Data.find(s);
    if (it == m_mapGuild2Data.end())
      continue;
    it->second.toData(cmd.add_crystals());
  }
  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);

  m_setUpdateGuildCrystals.clear();
}

bool SuperGvgScene::checkWin()
{
  if (m_eSGvgState == ESGVGSTATE_END)
    return false;
  bool win = false;
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  for (auto &m : m_mapGuild2Data)
  {
    if (m.second.dwRank != 1)
      continue;
    if (m.second.dwCrystalNum >= rCFG.dwWinMetalNum)
    {
      win = true;
      m_eSGvgState = ESGVGSTATE_END;
      XLOG << "[公会战决战-胜利], 公会:" << m.second.qwGuildID << "华丽水晶收集完成, 提前胜利" << "副本:" << name << id << m_qwRoomID << XEND;
      break;
    }
  }

  if (win)
    reward();

  return win;
}

void SuperGvgScene::queryTowerInfo(SceneUser* user, EGvgTowerType type, bool open)
{
  if (user == nullptr)
    return;
  if (EGvgTowerType_IsValid(type) == false)
    return;
  if (open)
  {
    GvgTowerUpdateFubenCmd cmd;
    if (type == EGVGTOWERTYPE_MIN) // 请求所有
    {
      for (auto &m : m_mapTowerData)
        m.second.toData(cmd.add_towers());
      m_setSyncUsers.insert(user);
    }
    else
    {
      auto it = m_mapTowerData.find(type);
      if (it == m_mapTowerData.end())
        return;
      it->second.toData(cmd.add_towers());
      it->second.setSyncUsers.insert(user);
    }
    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
  }
  else
  {
    if (type == EGVGTOWERTYPE_MIN) // 关闭请求界面
    {
      m_setSyncUsers.erase(user);
    }
    else
    {
      auto it = m_mapTowerData.find(type);
      if (it != m_mapTowerData.end())
        it->second.setSyncUsers.erase(user);
    }
  }
}
void SuperGvgScene::getBornPos(SceneUser* pUser, xPos& pos)
{
  if (pUser == nullptr)
    return;
  DWORD bp = 0;
  DWORD index = pUser->getUserZone().getColorIndex();
  if (index == 0)
  {
    auto it = m_mapUser2ColorIndex.find(pUser->id);
    if (it != m_mapUser2ColorIndex.end())
      index = it->second;
  }

  for (auto &m : m_mapGuild2Data)
  {
    if (m.second.dwColor == index)
    {
      bp = m.second.dwBpID;
      break;
    }
  }
  if (bp == 0)
    bp = 1;
  auto it = m_mapPos.find(bp);
  if (it != m_mapPos.end())
    pos = it->second;
}

void SuperGvgScene::syncAllDataToUser(SceneUser* user)
{
  if (user == nullptr)
    return;
  SuperGvgSyncFubenCmd cmd;
  for (auto &m : m_mapGuild2Data)
  {
    m.second.toData(cmd.add_guildinfo());
  }
  for (auto &m : m_mapTowerData)
  {
    m.second.toData(cmd.add_towers());
  }
  cmd.set_firebegintime(m_dwOpenFireTime);
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

void SuperGvgScene::userEnter(SceneUser *user)
{
  if (!user)
    return;
  MatchScene::userEnter(user);

  // 删除变身
  user->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);

  DWORD color = user->getUserZone().getColorIndex();
  auto it = m_mapUser2ColorIndex.find(user->id);
  if (it != m_mapUser2ColorIndex.end())
  {
    if (color != it->second)
    {
      user->getUserZone().setColorIndex(it->second);
      user->setDataMark(EUSERDATATYPE_PVP_COLOR);
      user->refreshDataAtonce();
    }
  }
  else
  {
    if (color != 0)
      m_mapUser2ColorIndex[user->id] = color;
  }

  auto m = m_mapSuGvgUserData.find(user->id);
  if (m == m_mapSuGvgUserData.end())
  {
    SSugvgUserData& data = m_mapSuGvgUserData[user->id];
    data.qwUserID = user->id;
    data.qwGuildID = user->getGuild().id();
    data.strUserName = user->name;
    m = m_mapSuGvgUserData.find(user->id);
  }
  if (m != m_mapSuGvgUserData.end())
  {
    m->second.dwProfession = user->getProfession();
  }

  if (m_qwRoomID) // 已初始化完成
    syncAllDataToUser(user);

  DWORD cur = now();
  if (m_dwTowerOpenTime > cur && m_dwTowerOpenTime <= cur + 60)
    MsgManager::sendMsg(user->id, 25520, MsgParams(m_dwTowerOpenTime - cur));
  if (m_eSGvgState == ESGVGSTATE_PREPARE && m_dwOpenFireTime > cur && m_dwOpenFireTime <= cur + 60)
    MsgManager::sendMsg(user->id, 25504, MsgParams(m_dwOpenFireTime - cur));
}

void SuperGvgScene::userLeave(SceneUser* user)
{
  MatchScene::userLeave(user);

  if (!user)
    return;
  user->getUserZone().setColorIndex(0);

  // del sync users
  for (auto &m : m_mapTowerData)
  {
    m.second.setSyncUsers.erase(user);
  }
  m_setSyncUsers.erase(user);
}

void SuperGvgScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  MatchScene::onNpcDie(npc, killer);
  if (npc == nullptr)
    return;
  SceneUser* user = dynamic_cast<SceneUser*> (killer);
  if (user == nullptr)
    return;
  if (m_eSGvgState == ESGVGSTATE_END)
    return;
  do
  {
    if (npc->define.m_oVar.m_qwGuildID == 0)
      break;
    const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
    for (auto &m : m_mapGuild2Data)
    {
      if (m.second.qwMetalNpcID != npc->id)
        continue;
      m.second.dwCrystalNum = (m.second.dwCrystalNum ? m.second.dwCrystalNum - 1 : 0);
      m.second.bMetalLive = false;
      m.second.dwExpelUserTime = rCFG.dwMetalDieExpelTime + now();

      QWORD guildid = user->getGuild().id();
      auto it = m_mapGuild2Data.find(guildid);
      if (it != m_mapGuild2Data.end())
      {
        MsgParams param;
        param.addString(it->second.strColorName);
        param.addString(user->name);
        param.addString(m.second.strColorName);
        param.addString(m.second.strGuildName);
        param.addString(m.second.strColorName);
        param.addString(m.second.strGuildName);
        MsgManager::sendMapMsg(id, 25513, param);
      }
      xSceneEntrySet userSet;
      getAllEntryList(SCENE_ENTRY_USER, userSet);
      for (auto &u : userSet)
      {
        SceneUser* user = dynamic_cast<SceneUser*>(u);
        if (user == nullptr || user->isAlive())
          continue;
        if (user->getGuild().id() != m.second.qwGuildID)
          continue;
        xPos pos = user->getPos();
        getBornPos(user, pos);
        user->goTo(pos);
      }

      m_setUpdateGuildCrystals.insert(m.first);
      m_bOpenRank = true;
      updateRank();

      GvgMetalDieFubenCmd cmd;
      cmd.set_index(m.second.dwColor);
      PROTOBUF(cmd, send, len);
      sendCmdToAll(send, len);

      XLOG << "[公会战决战-华丽水晶死亡], 公会:" << m.second.strGuildName << m.second.qwGuildID << "水晶:" << npc->id << "当前积分:" << m.second.dwCrystalNum << XEND;
      break;
    }
  } while(0);

  do
  {
    bool south = true;
    if (m_setSouthBossIDs.find(npc->id) != m_setSouthBossIDs.end())
    {
      south = true;
      m_setSouthBossIDs.erase(npc->id);
    }
    else if (m_setNorthBossIDs.find(npc->id) != m_setNorthBossIDs.end())
    {
      south = false;
      m_setNorthBossIDs.erase(npc->id);
    }
    else
    {
      break;
    }

    QWORD guildid = user->getGuild().id();
    auto it = m_mapGuild2Data.find(guildid);
    if (it == m_mapGuild2Data.end())
      break;
    MsgParams param;
    param.addString(it->second.strColorName);
    param.addString(it->second.strGuildName);
    param.addString(it->second.strColorName);
    param.addString(user->name);
    MsgManager::sendMapMsg(id, south ? 25510 : 25509, param);

    it->second.dwChipNum ++;
    m_setUpdateGuildCrystals.insert(guildid);
    const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
    if (it->second.dwChipNum >= rCFG.dwComposeChipNum)
    {
      it->second.dwCrystalNum ++;
      it->second.qwGetScoreTime = xTime::getCurUSec();
      it->second.dwChipNum -= rCFG.dwComposeChipNum;

      // msg 合成提示
      MsgParams param;
      param.addString(it->second.strColorName);
      param.addString(it->second.strGuildName);
      MsgManager::sendMapMsg(id, 25511, param);

      m_bOpenRank = true;
      updateRank();
      checkWin();
      onAddCrystal(it->second);
      XLOG << "[公会战决战-合成碎片], 公会:" << guildid << it->second.strGuildName << "boss:" << npc->name << npc->id << "当前水晶数量:" << it->second.dwCrystalNum << "地图:" << name << id << XEND;
    }
    else
    {
      updateRank();
    }

    onGetChip(user);

    XLOG << "[公会战决战-获取碎片], 公会:" << guildid << it->second.strGuildName << "boss:" << npc->name << npc->id << "当前水晶数量:" << it->second.dwCrystalNum << "碎片数:" << it->second.dwChipNum << XEND;
  } while(0);

}

void SuperGvgScene::clearBarrier()
{
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  for (auto &m : m_mapGuild2Data)
  {
    const SGvgCampCFG* pCFG = rCFG.getCampCFG(m.second.dwColor);
    if (pCFG == nullptr)
      continue;
    m_oGear.set(pCFG->dwGearUniqID, 2, nullptr); // 关闭空气墙

    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m.second.qwMetalNpcID);
    if (npc)
      npc->m_oBuff.add(rCFG.dwMetalGodBuff, npc, 0, 0, (QWORD)(m_dwOpenFireTime + rCFG.dwMetalGodTime) * ONE_THOUSAND);
  }
}

void SuperGvgScene::summonBoss(bool south)
{
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  const TSetDWORD& uids = south ? rCFG.setSouthBossUids : rCFG.setNorthBossUids;
  auto p = randomStlContainer(uids);
  if (p == nullptr)
    return;
  DWORD uid = *p;

  const TSetDWORD& bossids = rCFG.setBossIDs;
  auto p2 = randomStlContainer(bossids);
  if (p2 == nullptr)
    return;
  DWORD bossid = *p2;

  const SceneObject *pObject = getSceneObject();
  if (pObject == nullptr)
    return;
      // 水晶
  const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(uid);
  if (pTemplate == nullptr)
  {
    XERR << "[公会战决战-boss], 召唤晶兽失败, 找不到怪物, uid:" << uid << "地图:" << name << id << m_qwRoomID << XEND;
    return;
  }
  NpcDefine def = pTemplate->m_oDefine;
  def.setID(bossid);
  SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
  if (npc == nullptr)
  {
    XERR << "[公会战决战-boss], 召唤晶兽失败, uid:" << uid << "bossid:" << bossid << "地图:" << name << id << m_qwRoomID << XEND;
    return;
  }

  if (south)
    m_setSouthBossIDs.insert(npc->id);
  else
    m_setNorthBossIDs.insert(npc->id);
  XLOG << "[公会战决战-boss], 召唤晶兽成功, uid:" << uid << "boss:" << npc->name << npc->id << bossid << "方位:" << south << "地图:" << name << id << m_qwRoomID << XEND;
}

void SuperGvgScene::addTimeBossMsg(DWORD time, DWORD msgid)
{
  m_mapTimeTick2BossMsg[time].insert(msgid);
}

void SuperGvgScene::checkBossMsg(DWORD cur)
{
  if (m_mapTimeTick2BossMsg.empty())
    return;
  for (auto m = m_mapTimeTick2BossMsg.begin(); m != m_mapTimeTick2BossMsg.end(); )
  {
    if (cur >= m->first)
    {
      for (auto &s : m->second)
        MsgManager::sendMapMsg(id, s);

      m = m_mapTimeTick2BossMsg.erase(m);
      continue;
    }
    ++m;
  }
}

void SuperGvgScene::onTimeUp()
{
  XLOG << "[公会战决战-结束], 到达结束时间, 进行结算, 副本:" << name << id << m_qwRoomID << XEND;
  m_bOpenRank = true;
  updateRank(true);
  reward();
}

// 场景buff 等
void SuperGvgScene::doSceneEffect()
{
  auto effect = [&](const TSetDWORD& effectids)
  {
    auto p = randomStlContainer(effectids);
    if (!p)
      return;
    DWORD effectid = *p;
    PveCardBase* pCardEffect = PveCardManager::getMe().getCardEffect(effectid);
    if (pCardEffect == nullptr)
    {
      XERR << "[公会战决战-副本], 找不到对应的效果效果ID:" << effectid << "副本:" << name << id << XEND;
      return;
    }
    pCardEffect->doEffect(this);
    XLOG << "[公会战决战-副本], 执行场景效果成功, 效果ID:" << effectid << "副本:" << name << id << m_qwRoomID << XEND;
  };
  const TSetDWORD& south = MiscConfig::getMe().getSuperGvgCFG().setSouthSceneEffectIDs;
  const TSetDWORD& north = MiscConfig::getMe().getSuperGvgCFG().setNorthSceneEffectIDs;
  effect(south);
  effect(north);
}

void SuperGvgScene::onAddCrystal(const SSuperGvgGuildData& data)
{
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  if (data.dwCrystalNum == rCFG.dwWinMetalNum - 1)
  {
    MsgParams param;
    param.addString(data.strColorName);
    param.addString(data.strGuildName);
    MsgManager::sendMapMsg(id, 25514, param);
  }
}

void SuperGvgScene::addPartinTime(SceneUser* user, DWORD time)
{
  if (user == nullptr)
    return;
  auto it = m_mapSuGvgUserData.find(user->id);
  if (it == m_mapSuGvgUserData.end())
  {
    m_mapSuGvgUserData[user->id].qwGuildID = user->getGuild().id();

    it = m_mapSuGvgUserData.find(user->id);
    if (it == m_mapSuGvgUserData.end())
      return;
  }
  it->second.dwPartInTime += time;
  XLOG << "[公会战决战-参战时间], 玩家参战时间增加, 玩家:" << user->name << user->id << "当前时间:" << it->second.dwPartInTime << "副本:" << name << id << m_qwRoomID << XEND;
}

void SuperGvgScene::reward()
{
  const SSuperGvgCFG& rCFG = MiscConfig::getMe().getSuperGvgCFG();
  const TVecItemInfo& vecGuildReward = rCFG.vecBaseGuildReward;
  const TVecItemInfo& vecUserReward = rCFG.vecBaseUserReward;
  map<QWORD, pair<TVecItemInfo, TVecItemInfo>> guild2reward; // userreard, guildreward

  // 通知公会服更新排名
  EndSuperGvgGuildSCmd message;

  for (auto &m : m_mapGuild2Data)
  {
    if (m.second.dwRank == 0) // 正常逻辑不可能出现
    {
      XERR << "[公会战决战-奖励], 未知异常, 排名为0, 副本:" << name << id << m_qwRoomID << "公会:" << m.first << m.second.strGuildName << XEND;
      m.second.dwRank = 3;
    }
    DWORD rank = m.second.dwRank;

    message.set_guildid(m.first);
    message.set_rank(rank);
    PROTOBUF(message, send, len);
    thisServer->sendCmdToSession(send, len);
    XLOG << "[公会战决战-排名更新], 发送公会服成功, 公会:" << m.first << m.second.strGuildName << "排名:" << rank << XEND;

    float userper = rCFG.getRewardPer(rank, m_dwRoomLevel, true);
    float guildper = rCFG.getRewardPer(rank, m_dwRoomLevel, false);
    XLOG << "[公会战决战-奖励], 计算奖励系数, 房间段位:" << m_dwRoomLevel << "公会:" << m.first << m.second.strGuildName << "奖励系数,玩家/公会:" << userper << guildper << XEND;

    pair<TVecItemInfo, TVecItemInfo>& reward = guild2reward[m.first];
    for (auto &v : vecGuildReward)
    {
      ItemInfo item;
      item.set_count(v.count() * guildper);
      item.set_id(v.id());
      if (item.count() == 0)
        continue;
      reward.second.push_back(item);
    }
    for (auto &v : vecUserReward)
    {
      ItemInfo item;
      item.set_count(v.count() * userper);
      item.set_id(v.id());
      if (item.count() == 0)
        continue;
      reward.first.push_back(item);
    }
  }

  const TVecItemInfo& vecStableReward = rCFG.vecUserStableReward;
  // 个人奖励
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  map<QWORD, SceneUser*> tempusers;
  for (auto &s : userSet)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user)
      tempusers[user->id] = user;
  }
  for (auto &m : m_mapSuGvgUserData)
  {
    // 判断达到参战时间
    if (m.second.dwPartInTime < rCFG.dwPartinRewardTime)
      continue;
    auto itp = guild2reward.find(m.second.qwGuildID);
    if (itp == guild2reward.end())
      continue;
    TVecItemInfo vecItem;
    vecItem.assign(itp->second.first.begin(), itp->second.first.end());
    for (auto &v : vecStableReward)
      vecItem.push_back(v);

    // 在线直接发
    auto it = tempusers.find(m.first);
    if (it != tempusers.end())
    {
      SceneUser* pUser = it->second;
      pUser->getPackage().addItem(vecItem, EPACKMETHOD_AVAILABLE);
      XLOG << "[公会战决战-奖励], 玩家直接下发奖励, 玩家:" << pUser->name << pUser->id << "副本:" << name << id << m_qwRoomID << XEND;
    }
    else
    {
      MailManager::getMe().sendMail(m.first, rCFG.dwUserRewardMail, vecItem);
      XLOG << "[公会战决战-奖励], 不在线, 发送邮件奖励, 玩家:" << m.first << "副本:" << name << id << m_qwRoomID << XEND;
    }
  }

  // 下发公会奖励
  SuperGvgRewardInfoFubenCmd cmd;
  for (auto &m : m_mapGuild2Data)
  {
    SuperGvgRewardData* pData = cmd.add_rewardinfo();
    if (pData)
    {
      pData->set_guildid(m.first);
      pData->set_rank(m.second.dwRank);
    }

    auto it = guild2reward.find(m.first);
    if (it == guild2reward.end())
      continue;
    const TVecItemInfo& vecItem = it->second.second;
    if (vecItem.empty())
    {
      XLOG << "[公会战决战-公会奖励], 奖励为0, 公会:" << m.first << m.second.strGuildName << "副本:" << name << id << m_qwRoomID << "排名:" << m.second.dwRank << "战绩:" << m.second.dwFireCount << m.second.dwFireScore << XEND;
      continue;
    }

    for (auto &v : vecItem)
    {
      RewardItemData* pItem = pData ? pData->add_items() : nullptr;
      if (pItem)
      {
        pItem->set_itemid(v.id());
        pItem->set_count(v.count());
      }

      xLuaData data;
      data.setData("cmd","additem");
      data.setData("guildid", m.first);
      data.setData("id", v.id());
      data.setData("count", v.count());
      GMCommandRuler::getMe().guild(nullptr, data);
      XLOG << "[公会战决战-公会奖励], 获取公会奖励, 公会:" << m.first << m.second.strGuildName << "副本:" << name << id << m_qwRoomID << "奖励:" << v.id() << v.count() << "战绩:" << m.second.dwFireCount << m.second.dwFireScore << m.second.dwRank << XEND;
    }
  }
  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);

  // 活动奖励
  TSetDWORD extrarwds;
  RewardConfig::getMe().getExtraByType(EEXTRAREWARD_SUPERGVG, 1, 0, extrarwds, m_dwRoomLevel);
  if (extrarwds.empty() == false)
  {
    TVecItemInfo vecItems;
    for (auto &s : extrarwds)
      RewardConfig::getMe().roll(s, RewardEntry(), vecItems, ESOURCE_GVG);
    if (!vecItems.empty())
    {
      GuildBrocastMailGuildSCmd mailcmd;
      mailcmd.set_mailid(rCFG.dwExtraMailID);

      GuildBrocastMsgGuildSCmd msgcmd;
      msgcmd.set_msgid(rCFG.dwExtraMsgID);

      MsgParam* p = msgcmd.add_params();
      if (!p)
        return;
      const SSuGvgRewardCFG* pRwdCFG = rCFG.getRewardCFGByLv(m_dwRoomLevel);
      if (pRwdCFG)
        p->set_param(pRwdCFG->strLvName);

      p = msgcmd.add_params();
      if (!p)
        return;

      for (auto &v : vecItems)
      {
        mailcmd.add_items()->CopyFrom(v);

        const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.id());
        if (pCFG)
        {
          p->add_subparams("[");
          p->add_subparams(pCFG->strNameZh);

          std::stringstream stream;
          stream.str("");
          stream << " ] * [ " << v.count() << " ] ";
          p->add_subparams(stream.str());
        }
      }

      for (auto &m : m_mapSuGvgUserData)
      {
        mailcmd.set_guildid(m.second.qwGuildID);
        msgcmd.set_guildid(m.second.qwGuildID);

        PROTOBUF(mailcmd, mailsend, maillen);
        PROTOBUF(msgcmd, msgsend, msglen);
        thisServer->sendCmdToSession(mailsend, maillen);
        thisServer->sendCmdToSession(msgsend, msglen);
      }
    }
  }

  setCloseTime(now() + 30); // 结算后, 强制踢出
  XLOG << "[公会战决战-奖励], 发放完成, 副本:" << name << id << m_qwRoomID << "段位:" << m_dwRoomLevel << XEND;
}

void SuperGvgScene::onNpcBeAttack(SceneNpc* npc, xSceneEntryDynamic* attacker, DWORD hp)
{
  if (npc == nullptr || attacker == nullptr)
    return;
  for (auto &m : m_mapGuild2Data)
  {
    if (npc->id == m.second.qwMetalNpcID)
    {
      SceneUser* user = dynamic_cast<SceneUser*>(attacker);
      if (user == nullptr)
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*>(attacker);
        if (npc == nullptr)
          break;
        user = npc->getMasterUser();
        if (user == nullptr)
          break;
      }
      QWORD guildid = user->getGuild().id();
      if (guildid == 0)
        break;
      DWORD& time = m.second.mapGuild2AttackTime[guildid];
      DWORD cur = now();
      if (cur < time)
        break;
      time = cur + 30; // 30秒提示一次

      auto it = m_mapGuild2Data.find(guildid);
      if (it == m_mapGuild2Data.end())
        break;

      onDamageMeatal(user, hp);

      MsgParams param;
      param.addString(it->second.strColorName);
      param.addString(it->second.strGuildName);
      param.addString(m.second.strColorName);
      param.addString(m.second.strGuildName);
      MsgManager::sendMapMsg(id, 25512, param);
      break;
    }
  }
}

void SuperGvgScene::openFireAtonce()
{
  if (m_eSGvgState == ESGVGSTATE_PREPARE)
  {
    m_dwOpenFireTime = now();
    m_dwStopFireTime = m_dwOpenFireTime + 30 * 60;
    XLOG << "[决战-测试], 设置开启决战, 副本:" << name << id << XEND;
  }
}

bool SuperGvgScene::needExpelDieUser(SceneUser* user)
{
  if (user == nullptr)
    return false;
  auto it = m_mapGuild2Data.find(user->getGuild().id());
  if (it != m_mapGuild2Data.end())
  {
    return it->second.bMetalLive == false && it->second.dwExpelUserTime >= now();
  }
  return false;
}

void SuperGvgScene::onKillUser(SceneUser* pKiller, SceneUser* pDeath)
{
  MatchScene::onKillUser(pKiller, pDeath);
  if (pKiller == nullptr || pDeath == nullptr)
    return;
  auto it = m_mapSuGvgUserData.find(pKiller->id);
  if (it != m_mapSuGvgUserData.end())
    it->second.dwKillUserNum ++;
}

void SuperGvgScene::onUserDie(SceneUser *user, xSceneEntryDynamic *killer)
{
  MatchScene::onUserDie(user, killer);
  if (user == nullptr)
    return;
  auto it = m_mapSuGvgUserData.find(user->id);
  if (it != m_mapSuGvgUserData.end())
    it->second.dwDieNum ++;
}

void SuperGvgScene::addHealUser(QWORD id, QWORD healID, DWORD hp)
{
  MatchScene::addHealUser(id, healID, hp);

  auto it = m_mapSuGvgUserData.find(healID);
  if (it != m_mapSuGvgUserData.end())
    it->second.dwHealHp += hp;
}

void SuperGvgScene::onGetChip(SceneUser* user)
{
  if (user== nullptr)
    return;
  // 队伍中玩家增加碎片计数
  std::set<SceneUser*> teamusers = user->getTeamSceneUser();
  teamusers.insert(user);
  for (auto &s : teamusers)
  {
    auto it = m_mapSuGvgUserData.find(s->id);
    if (it != m_mapSuGvgUserData.end())
      it->second.dwChipNum ++;
  }
}

void SuperGvgScene::onReliveUser(SceneUser* user, SceneUser* reliver) //reliver复活user
{
  if (user == nullptr || reliver == nullptr)
    return;
  auto it = m_mapSuGvgUserData.find(reliver->id);
  if (it != m_mapSuGvgUserData.end())
    it->second.dwReliveNum ++;
}

void SuperGvgScene::onDamageMeatal(SceneUser* user, DWORD hp)
{
  if (user == nullptr)
    return;
  auto it = m_mapSuGvgUserData.find(user->id);
  if (it != m_mapSuGvgUserData.end())
    it->second.dwMetalDamage += hp;
}

void SuperGvgScene::queryUserData(SceneUser* user)
{
  if (user == nullptr)
    return;

  SuperGvgQueryUserDataFubenCmd cmd;
  map<QWORD, SuperGvgGuildUserData*> tempGuild2Data;
  for (auto &m : m_mapGuild2Data)
  {
    if (m.first == 0)
      continue;
    SuperGvgGuildUserData* pData = cmd.add_guilduserdata();
    if (pData == nullptr)
      continue;
    pData->set_guildid(m.first);
    tempGuild2Data[m.first] = pData;
  }
  for (auto &m : m_mapSuGvgUserData)
  {
    if (m.second.qwGuildID == 0)
    {
      SceneUser* user = SceneUserManager::getMe().getUserByID(m.first); // scene::userEnter, 可能公会信息尚未同步至场景
      if (user)
        m.second.qwGuildID = user->getGuild().id();
    }

    auto it = tempGuild2Data.find(m.second.qwGuildID);
    if (it == tempGuild2Data.end())
      continue;
    m.second.toData(it->second->add_userdatas());
  }

  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

// altman scene
AltmanScene::AltmanScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :
  DScene(sID, sName, sBase, pRaidCFG)
{

}

bool AltmanScene::init()
{
  return DScene::init();
}

void AltmanScene::entryAction(QWORD curMSec)
{
  DScene::entryAction(curMSec);
  DWORD curSec = curMSec / 1000;
  DWORD dwClearTime = MiscConfig::getMe().getAltmanCFG().dwClearNpcTime;
  DWORD dwCloseTime = getCloseTime();
  if(dwCloseTime <= curSec + dwClearTime && m_bRewarded == false)
  {
    xSceneEntrySet entrySet;
    getAllEntryList(SCENE_ENTRY_NPC, entrySet);
    for (auto entryIt = entrySet.begin(); entryIt != entrySet.end(); ++entryIt)
    {
      SceneNpc *npc = (SceneNpc *)(*entryIt);
      if(npc != nullptr)
        npc->setStatus(ECREATURESTATUS_LEAVE);
    }

    reward();
    XLOG << "[奥特曼-结束], " << "scene: " << id << XEND;
  }
}

void AltmanScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userEnter(user);

  // to prevent can add transform buff
  UserHands& oHands = user->m_oHands;
  if (oHands.has() == true)
    oHands.breakup();

  const SAltmanCFG& rCfg = MiscConfig::getMe().getAltmanCFG();
  DWORD buffID = rCfg.randomTransBuff();
  user->m_oBuff.add(buffID, user);
  notify(user);
  XLOG << "[奥特曼-进入], " << user->name << user->id << user->accid << "teamid: " << user->getTeamID() << "scene: " << id << "buff: " << buffID << XEND;
}

void AltmanScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  DScene::userLeave(user);
  if(m_dwDeadMonster > user->getVar().getVarValue(EVARTYPE_ALTMAN_KILL))
  {
    user->getVar().setVarValue(EVARTYPE_ALTMAN_KILL, m_dwDeadMonster);
    XLOG << "[奥特曼-最高击杀], " << user->name << user->id << user->accid << "击杀数量:" << m_dwDeadMonster << XEND;
  }

  XLOG << "[奥特曼-离开], " << user->name << user->id << user->accid << "teamid: " << user->getTeamID() << "scene: " << id << XEND;
}

void AltmanScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  if (npc == nullptr)
    return;

  DScene::onNpcDie(npc, killer);

  SceneUser* pUser = dynamic_cast<SceneUser*>(killer);
  if(pUser == nullptr)
    return;
  auto it = m_mapUserKill.find(pUser->id);
  if(it != m_mapUserKill.end())
  {
    ++(it->second);
  }
  else
  {
    m_mapUserKill.emplace(pUser->id, 1);
  }

  m_dwDeadMonster++;
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);

  for (auto userIt = userSet.begin(); userIt != userSet.end(); ++userIt)
  {
    SceneUser *pUser = dynamic_cast<SceneUser*>(*userIt);
    if (nullptr == pUser || pUser->getTeamID() == 0)
      continue;
    notify(pUser);
  }
}

void AltmanScene::onUserDie(SceneUser *user, xSceneEntryDynamic *killer)
{
  if (user == nullptr)
    return;
  DScene::onUserDie(user, killer);
}

void AltmanScene::onClose()
{
}

void AltmanScene::summonUserRewardBox(SceneUser* pUser)
{
  if(pUser == nullptr)
    return;

  const SAltmanCFG& rCfg = MiscConfig::getMe().getAltmanCFG();
  const SEndlessTowerCFG& rTowerCFG = MiscConfig::getMe().getEndlessTowerCFG();
  NpcDefine oDefine;
  oDefine.load(rCfg.oRewardBoxDefine);
  xPos oPos;

  // 定义宝箱位置
  {
    oPos = pUser->getPos();
    if(pUser->getScene() != nullptr)
    {
      pUser->getScene()->getCircleRoundPos(pUser->getPos(), rTowerCFG.fRewardBoxRange, oPos);
    }
    else
    {
      oDefine.setRange(rTowerCFG.fRewardBoxRange);
    }
  }

  oDefine.setPos(oPos);
  oDefine.m_oVar.m_qwQuestOwnerID = pUser->id;
  SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, this);
  if (pNpc == nullptr)
  {
    XERR << "[奥特曼场景-宝箱召唤]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "召唤奖励宝箱 :" << oDefine.getID() << "失败" << XEND;
    return;
  }

  bool bReward = pUser->getVar().getVarValue(EVARTYPE_ALTMAN_REWARD);

  UserActionNtf cmd;
  cmd.set_charid(pNpc->tempid);
  cmd.set_value(!bReward ? rTowerCFG.dwRewardBoxGetAction : rTowerCFG.dwRewardBoxUngetAction);
  cmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  {
    xLuaData data;
    data.setData("effect", rTowerCFG.strRewardBoxEffect);
    data.setData("npcid", pNpc->getNpcID());
    data.setData("effectpos", 1);
    GMCommandRuler::effect(pUser, data);
  }
}

void AltmanScene::reward()
{
  m_bRewarded = true;

  DWORD dwClearTime = MiscConfig::getMe().getAltmanCFG().dwClearNpcTime;
  DWORD dwCloseTime = getCloseTime();
  if(now() + dwClearTime < dwCloseTime)
    setCloseTime(now() + dwClearTime);

  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);

  const SAltmanCFG& rCfg = MiscConfig::getMe().getAltmanCFG();
  for (auto userIt = userSet.begin(); userIt != userSet.end(); ++userIt)
  {
    SceneUser *pUser = dynamic_cast<SceneUser*>(*userIt);
    if (nullptr == pUser || pUser->getTeamID() == 0)
      continue;

    bool bReward = pUser->getVar().getVarValue(EVARTYPE_ALTMAN_REWARD);
    if (!bReward)
    {
      pUser->getPackage().rollReward(rCfg.dwRewardID, EPACKMETHOD_AVAILABLE, false, true);
      DWORD dwUnlockHead = pUser->getManual().getConstantUnlockNum(EMANUALTYPE_FASHION, rCfg.setManualHeads);
      DWORD dwExtraRewardID = rCfg.getHeadReward(dwUnlockHead);
      if(dwExtraRewardID != 0)
        pUser->getPackage().rollReward(dwExtraRewardID, EPACKMETHOD_AVAILABLE, false, true);

      summonUserRewardBox(pUser);
      pUser->getVar().setVarValue(EVARTYPE_ALTMAN_REWARD, 1);
      XLOG << "[奥特曼-奖励], 通关: 0" << pUser->name << pUser->id << pUser->accid << "当前玩家数量:" << userSet.size() << "奖励id:" << rCfg.dwRewardID
        << "头饰奖励: " << dwExtraRewardID << XEND;
    }
    else
    {
      summonUserRewardBox(pUser);
      XLOG << "[奥特曼-通关] " << pUser->name << pUser->id << pUser->accid << "当前玩家数量:" << userSet.size() << XEND;
    }
    notify(pUser);
  }
}

void AltmanScene::notify(SceneUser* pUser)
{
  if(pUser == nullptr)
    return;

  TeamRaidAltmanShowCmd cmd;
  cmd.set_lefttime(getCloseTime());
  cmd.set_killcount(m_dwDeadMonster);
  cmd.set_selfkill(m_mapUserKill[pUser->id]);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
}

void AltmanScene::onLeaveScene(SceneUser *user)
{
  for(auto s : MiscConfig::getMe().getAltmanCFG().setEnterBuffs)
    user->m_oBuff.del(s, user);
  user->getBuff().update(xTime::getCurMSec());
  user->getAttribute()->updateAttribute();
}

/***************************************************************************************/
/******************************组队排位赛***********************************************/
/***************************************************************************************/

void SPwsUserData::toData(TeamPwsRaidUserInfo* pData)
{
  if (pData == nullptr)
    return;
  pData->set_charid(qwUserID);
  pData->set_name(strName);
  pData->set_killnum(dwKillUserNum);
  pData->set_heal(dwHealHp);
  pData->set_killscore(dwKillScore);
  pData->set_ballscore(fBallScore * ONE_THOUSAND);
  pData->set_buffscore(dwBuffScore);
  pData->set_dienum(dwDieNum);
}

SPwsTeamData::SPwsTeamData(TeamPwsScene* scene) : pScene(scene)
{

}

void SPwsTeamData::updateBallScore(DWORD cur)
{
  if (cur < dwBallScoreNextTime)
    return;
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(bRelax);
  dwBallScoreNextTime = cur + rCFG.dwBallInterval;
  std::list<SPwsUserData*> validUsers;
  for (auto &m : mapUserDatas)
  {
    if (m.second.eHoldBall != EMAGICBALL_MIN && cur >= m.second.dwBallBeginValueTime)
      validUsers.push_back(&(m.second));
  }
  if (validUsers.empty())
    return;

  DWORD num = validUsers.size();
  DWORD add = rCFG.getScoreByBallNum(num);

  if (dwScore + add > rCFG.dwWinScore)
    add = rCFG.dwWinScore - dwScore;

  addScore(add);

  for (auto &s : validUsers)
    s->fBallScore += (float)add/num;
}

std::set<EMagicBallType> SPwsTeamData::getAllBalls() const
{
  std::set<EMagicBallType> balls;
  for (auto &m : mapUserDatas)
  {
    if (m.second.eHoldBall != EMAGICBALL_MIN)
      balls.insert(m.second.eHoldBall);
  }
  return balls;
}

bool SPwsTeamData::inMagic(EMagicBallType b1, EMagicBallType b2)
{
  return (oPairMagic.first == b1 && oPairMagic.second == b2) || (oPairMagic.first == b2 && oPairMagic.second == b1);
}

void SPwsTeamData::addScore(DWORD add)
{
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(bRelax);
  if (dwScore < rCFG.dwWarnScore && dwScore + add >= rCFG.dwWarnScore)
  {
    if (pScene)
      MsgManager::sendMapMsg(pScene->id, 25915, MsgParams(strTeamColorName));
  }

  dwScore += add;

  XDBG << "[组队排位赛-积分更新], 队伍:" << qwTeamID << "颜色:" << eColor << "当前得分:" << dwScore << XEND;
  setUpdate();
}

// 追踪栏, 进副本同步
void SPwsTeamData::toData(TeamPwsInfoSyncData* pData)
{
  if (pData == nullptr)
    return;

  pData->set_teamid(qwTeamID);
  pData->set_color(eColor);
  pData->set_score(dwScore);
  pData->set_effectcd(dwBallMagicCD);
  pData->set_magicid(MiscConfig::getMe().getTeamPwsCFG(bRelax).getMagicIDByBall(oPairBuffMagic.first, oPairBuffMagic.second));
  for (auto &m : mapUserDatas)
  {
    if (m.second.eHoldBall != EMAGICBALL_MIN)
      pData->add_balls(m.second.eHoldBall);
  }
}

// 队伍各成员详细数据
void SPwsTeamData::toData(TeamPwsRaidTeamInfo* pTeamInfo)
{
  if (pTeamInfo == nullptr)
    return;
  for (auto &m : mapUserDatas)
  {
    m.second.toData(pTeamInfo->add_userinfos());
  }
  pTeamInfo->set_teamid(qwTeamID);
  pTeamInfo->set_color(eColor);
}

void SPwsTeamData::createData(const TeamPwsRoomData& matchdata, bool relax)
{
  qwTeamID = matchdata.teamid();
  eColor = static_cast<ETeamPwsColor>(matchdata.color());
  bRelax = relax;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(bRelax);
  const SPwsColorInfo* pInfo = rCFG.getColorInfo(matchdata.color());
  if (pInfo)
    strTeamColorName = pInfo->strName;

  for (int i = 0; i < matchdata.users_size(); ++i)
  {
    auto& d = matchdata.users(i);
    SPwsUserData& user = mapUserDatas[d.charid()];
    user.qwUserID = d.charid();
    user.dwOriginScore = d.score();
  }
}

TeamPwsScene::TeamPwsScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) : MatchScene(sID, sName, sBase, pRaidCFG), m_oOneSecTimer(1), m_stRedTeam(this), m_stBlueTeam(this)
{

}

TeamPwsScene::~TeamPwsScene()
{

}

bool TeamPwsScene::init()
{
  if (MatchScene::init() == false)
    return false;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  addFreeSkill(rCFG.dwCollectSkill);
  addFreeSkill(rCFG.dwColleckBuffSkill);

  // 副本结束前不可点击小地图离开
  addForbidSkill(MiscConfig::getMe().getNewRoleCFG().dwTransSkill);
  return true;
}

void TeamPwsScene::onReceiveRoomInfo(const Cmd::SyncRoomSceneMatchSCmd& cmd)
{
  m_qwRoomID = cmd.roomid();
  m_bRelax = cmd.pvptype() == EPVPTYPE_TEAMPWS_RELAX;
  for (int i = 0; i < cmd.pwsdata_size(); ++i)
  {
    auto& team = cmd.pwsdata(i);
    if ((DWORD)ETEAMPWS_RED == team.color())
    {
      m_stRedTeam.createData(team, m_bRelax);
    }
    else if ((DWORD)ETEAMPWS_BLUE == team.color())
    {
      m_stBlueTeam.createData(team, m_bRelax);
    }
  }

  // 收到房间消息时, 已有玩家进入
  xSceneEntrySet userSet;
  getAllEntryList(SCENE_ENTRY_USER, userSet);
  if (!userSet.empty())
  {
    for (auto &s : userSet)
    {
      SceneUser* user = dynamic_cast<SceneUser*>(s);
      if (user == nullptr)
        continue;
      onUserIn(user);
    }
  }

  XLOG << "[组队排位赛-房间信息], 设置队伍信息成功, 副本:" << LOG_SCENE_INFO << XEND;
}

void TeamPwsScene::onUserIn(SceneUser* user)
{
  if (user == nullptr)
    return;

  // 追踪栏信息
  TeamPwsInfoSyncFubenCmd cmd;
  m_stRedTeam.toData(cmd.add_teaminfo());
  m_stBlueTeam.toData(cmd.add_teaminfo());
  cmd.set_endtime(m_dwEndTime);
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);

  do
  {
    SPwsTeamData* pTeam = getTeamData(user);
    if (!pTeam)
      break;
    auto p = pTeam->getUserData(user->id);
    if (!p)
      break;
    if (p->bJoined == false)
    {
      // 第一次进入
      p->bJoined = true;
      if (!m_bRelax)
      {
        DWORD value = user->getVar().getVarValue(EVARTYPE_TEAMPWS_COUNT);
        user->getVar().setVarValue(EVARTYPE_TEAMPWS_COUNT, value+1);
        user->sendVarToSession(EVARTYPE_TEAMPWS_COUNT);
        XLOG << "[组队排位赛-玩家扣除次数], 玩家:" << user->name << user->id << "当前次数:" << value + 1 << LOG_SCENE_INFO << XEND;
      }

      p->strName = user->name;
      user->toPortraitData(&(p->oPortrait));
      p->eProfession = user->getProfession();
    }
  }
  while(0);
}

void TeamPwsScene::userEnter(SceneUser* user)
{
  MatchScene::userEnter(user);

  if (!user)
    return;

  user->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);
  // 拆卸神器
  user->getPackage().equipOffAritifact();

  if (m_eState == ETEAMPWS_MIN)
  {
    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
    m_eState = ETEAMPWS_PREPARE;
    DWORD cur = now();
    m_dwTimeTick = cur + rCFG.dwPrepareTime;
    m_dwEndTime = m_dwTimeTick + rCFG.dwLastTime;
  }

  if (m_eState == ETEAMPWS_PREPARE)
  {
    MsgManager::sendMsg(user->id, 25908, MsgParams(m_dwTimeTick-now()));
  }
  else if (m_eState == ETEAMPWS_FIRE)
  {
    DWORD cur = now();
    if (m_mapBallData.empty() && cur >= m_dwTimeTick-15 && cur < m_dwTimeTick)
      MsgManager::sendMsg(user->id, 25927, MsgParams(m_dwTimeTick - cur));
  }

  if (m_qwRoomID)
  {
    onUserIn(user);
  }

  SPwsTeamData* pTeam = getTeamData(user);
  if (pTeam && pTeam->hasMagicBuff())
  {
    // 添加buff
    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
    const SPwsMagicCombine* pMagicCFG = rCFG.getMagicByBall(pTeam->oPairBuffMagic.first, pTeam->oPairBuffMagic.second);
    if (pMagicCFG != nullptr)
    {
      for (auto &s : pMagicCFG->setBuffIDs)
        user->m_oBuff.add(s);
    }
  }

  if (pTeam && pTeam->eColor)
    user->getUserZone().setColorIndex(pTeam->eColor);

  // 检查添加buff
  auto p = getUserData(user);
  if (p)
  {
    QWORD time = xTime::getCurMSec();
    for (auto &m : p->mapBuff2EndTime)
    {
      if (m.second > time)
        user->m_oBuff.add(m.first, user, 0, 0, m.second, true);
    }
  }
}

void TeamPwsScene::userLeave(SceneUser* user)
{
  MatchScene::userLeave(user);
  if (!user)
    return;
}

void TeamPwsScene::getBornPos(SceneUser* pUser, xPos& pos)
{
  if (pUser == nullptr)
    return;
  DWORD color = 0;
  SPwsTeamData* pTeam = getTeamData(pUser);
  if (pTeam)
  {
    color = (DWORD)(pTeam->eColor);
  }

  if (color == 0)
    color = pUser->getUserZone().getColorIndex();

  auto it = m_mapPos.find(color);
  if (it != m_mapPos.end())
    pos = it->second;
}

void TeamPwsScene::entryAction(QWORD curMSec)
{
  MatchScene::entryAction(curMSec);
  DWORD cur = curMSec / ONE_THOUSAND;
  if (m_oOneSecTimer.timeUp(cur))
  {
    switch(m_eState)
    {
      case ETEAMPWS_MIN:
        break;
      case ETEAMPWS_PREPARE:
        {
          if (cur >= m_dwTimeTick)
          {
            m_eState = ETEAMPWS_FIRE;
            const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
            m_dwTimeTick = cur + MiscConfig::getMe().getTeamPwsCFG(m_bRelax).dwSummonBallTime;
            m_dwSummonBuffTick = cur + rCFG.dwBuffNpcBeginTime;
            MsgManager::sendMapMsg(id, 25919);
            MsgManager::sendMapMsg(id, 25920);
          }
        }
        break;
      case ETEAMPWS_FIRE:
        {
          if (m_mapBallData.empty())
          {
            if (cur == m_dwTimeTick - 15)
            {
              MsgManager::sendMapMsg(id, 25927, MsgParams(15));
            }
            else if (cur >= m_dwTimeTick)
            {
              // 召唤法球
              summonAllBall();
            }
          }
          // 刷新buff
          if (cur >= m_dwSummonBuffTick)
          {
            m_dwSummonBuffTick = cur + MiscConfig::getMe().getTeamPwsCFG(m_bRelax).dwBuffNpcInterval;
            summonBuffNpc();
          }

          checkTeamMagicCD(cur);
          m_stRedTeam.updateBallScore(cur);
          m_stBlueTeam.updateBallScore(cur);
          if (cur >= m_dwEndTime)
          {
            XLOG << "[组队排位赛-结束], 到达结束时间, 副本:" << LOG_SCENE_INFO << XEND;
            checkWin(true);
            m_eState = ETEAMPWS_END;
          }
          else
          {
            checkWin();
          }
        }
        break;
     case ETEAMPWS_END:
        break;
    }
  }
}

void TeamPwsScene::summonAllBall()
{
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  if (rCFG.mapBallData.size() != rCFG.vecBallUniqueID.size())
    return;

  const SceneObject *pObject = getSceneObject();
  if (pObject == nullptr)
    return;

  TVecDWORD vecIDs = rCFG.vecBallUniqueID;
  std::random_shuffle(vecIDs.begin(), vecIDs.end());
  DWORD index = 0;
  for (auto &m : rCFG.mapBallData)
  {
    const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(vecIDs[index++]);
    if (pTemplate == nullptr)
      continue;

    NpcDefine def = pTemplate->m_oDefine;
    def.setID(m.second.dwSummonNpcID);
    SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
    if (npc == nullptr)
    {
      XERR << "[组队排位赛-召唤元素球], 召唤失败, 副本:" << name << id << "球:" << def.getID() << XEND;
      return;
    }
    npc->setDeadDelAtonce();
    pair<QWORD, xPos>& pa = m_mapBallData[m.first];
    pa.first = npc->id;
    pa.second = npc->getPos();
  }
}

void TeamPwsScene::summonBuffNpc()
{
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  const SceneObject *pObject = getSceneObject();
  if (pObject == nullptr)
    return;
  const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(rCFG.dwBuffNpcUniqueID);
  if (pTemplate == nullptr)
    return;

  TSetDWORD allids;
  for (auto &m : rCFG.mapNpc2Buffs)
    allids.insert(m.first);
  if (allids.empty())
    return;

  auto p = randomStlContainer(allids);
  if (!p)
    return;

  NpcDefine def = pTemplate->m_oDefine;
  def.setID(*p);

  if (rCFG.dwBuffNpcClearTime)
    def.setDisptime(rCFG.dwBuffNpcClearTime);

  SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
  if (npc == nullptr)
    return;
  npc->setDeadDelAtonce();
  XLOG << "[组队排位赛-刷新buff], buff npc:" << npc->name << npc->id << npc->getNpcID() << LOG_SCENE_INFO << XEND;
}

bool TeamPwsScene::checkWin(bool bTimeOut /*=false*/)
{
  // 检查更新信息
  updateTeamInfoToCient();

  if (m_eState != ETEAMPWS_FIRE)
    return false;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  bool end = bTimeOut ? true : (m_stRedTeam.dwScore >= rCFG.dwWinScore || m_stBlueTeam.dwScore >= rCFG.dwWinScore);

  if (!end)
    return false;

  auto getBetterTeam = [&]() -> SPwsTeamData*
  {
    if (m_stRedTeam.dwScore > m_stBlueTeam.dwScore)
      return &m_stRedTeam;
    else if (m_stRedTeam.dwScore < m_stBlueTeam.dwScore)
      return &m_stBlueTeam;
    else // 分数相等
    {
      if (m_stRedTeam.qwScoreTime < m_stBlueTeam.qwScoreTime)
        return &m_stRedTeam;
      else if (m_stRedTeam.qwScoreTime > m_stBlueTeam.qwScoreTime)
        return &m_stBlueTeam;
      return (randBetween(1,100) > 50 ? &m_stRedTeam : &m_stBlueTeam);
    }
    return nullptr;
  };

  SPwsTeamData* pWinTeam = getBetterTeam();
  if (pWinTeam == nullptr)
    return false;
  SPwsTeamData* pLoseTeam = (pWinTeam == &m_stRedTeam ? &m_stBlueTeam : &m_stRedTeam);

  // do reward
  m_eState = ETEAMPWS_END;
  // 副本结束允许传送离开
  delForbidSkill(MiscConfig::getMe().getNewRoleCFG().dwTransSkill);
  // 设置副本关闭时间
  setCloseTime(now() + 120);

  // 队伍内玩家进行排名
  std::vector<SPwsUserData*> winusers;
  std::vector<SPwsUserData*> loseusers;
  std::vector<SPwsUserData*> allusers;
  winusers.reserve(pWinTeam->mapUserDatas.size());
  loseusers.reserve(pLoseTeam->mapUserDatas.size());
  allusers.reserve(pWinTeam->mapUserDatas.size() + pLoseTeam->mapUserDatas.size());
  for (auto &m : pWinTeam->mapUserDatas)
  {
    winusers.push_back(&(m.second));
    allusers.push_back(&(m.second));
  }
  for (auto &m : pLoseTeam->mapUserDatas)
  {
    loseusers.push_back(&(m.second));
    allusers.push_back(&(m.second));
  }

  auto sortUserFunc = [](const SPwsUserData* u1, const SPwsUserData* u2) -> bool
  {
    // 优先得分高
    float s1 = u1->fBallScore + u1->dwBuffScore + u1->dwKillScore;
    float s2 = u2->fBallScore + u2->dwBuffScore + u2->dwKillScore;

    // 得分相同, 判断时间
    if (s1 == s2)
      return u1->qwScoreTime < u2->qwScoreTime;
    return s1 > s2;
  };

  map<QWORD, DWORD> user2ExtraScore; // 玩家表现得分影响最终积分
  // 按各项排名额外增加积分
  auto addscore_byrank = [&](const TVecDWORD& vecScoreCFG, DWORD scoreType)
  {
    for (DWORD i = 0; i < vecScoreCFG.size(); ++i)
    {
      if (i >= allusers.size())
        break;
      auto p = allusers[i];
      if (scoreType == 1 && p->qwScoreTime == 0) // 未有得分记录
        break;
      if (scoreType == 2 && p->dwKillUserNum == 0) // 击杀数为0
        break;
      if (scoreType == 3 && p->dwHealHp == 0) // 治疗量为0
        break;

      user2ExtraScore[p->qwUserID] += vecScoreCFG[i];
    }
  };

  // 全场总得分排名
  std::sort(allusers.begin(), allusers.end(), sortUserFunc);
  addscore_byrank(rCFG.vecAllScoreAdd, 1);

  // 全场击杀得分排名
  std::sort(allusers.begin(), allusers.end(), [](const SPwsUserData* u1,  const SPwsUserData* u2) -> bool
  {
    if (u1->dwKillUserNum == u2->dwKillUserNum)
      return u1->qwKillScoreTime < u2->qwKillScoreTime;
    return u1->dwKillUserNum > u2->dwKillUserNum;
  });
  addscore_byrank(rCFG.vecKillScoreAdd, 2);

  // 全场治疗得分排名
  std::sort(allusers.begin(), allusers.end(), [](const SPwsUserData* u1, const SPwsUserData* u2) -> bool
  {
    return u1->dwHealHp > u2->dwHealHp;
  });
  addscore_byrank(rCFG.vecHealSocreAdd, 3);

  // 总得分队伍内排名
  std::sort(winusers.begin(), winusers.end(), sortUserFunc);
  std::sort(loseusers.begin(), loseusers.end(), sortUserFunc);

  // ->match, 积分更新
  if (!m_bRelax)
  {
    UpdateScoreMatchSCmd scorecmd;
    scorecmd.set_etype(EPVPTYPE_TEAMPWS);

    auto getAveAndMaxScore = [](SPwsTeamData* p, float& ave, DWORD& max)
    {
      if (!p)return;
      float all = 0;
      max = 0;
      for (auto &m : p->mapUserDatas)
      {
        if (m.second.dwOriginScore > max)
          max = m.second.dwOriginScore;

        all += m.second.dwOriginScore;
      }
      if (all)
        ave = all / p->mapUserDatas.size();
    };

    float winave = 0, loseave = 0;
    DWORD winmax = 0, losemax = 0;
    getAveAndMaxScore(pWinTeam, winave, winmax);
    getAveAndMaxScore(pLoseTeam, loseave, losemax);

    auto calcscore = [&](SPwsUserData* p, bool win)
    {
      if (!p)
        return;
      auto &data = *p;
      if (data.bJoined == false)
        return;
      if (data.bLeave)
      {
        XLOG << "[组队排位赛-玩家积分], 玩家离开队伍, 无法获取积分, 玩家:" << data.qwUserID << data.strName << LOG_SCENE_INFO << XEND;
        return;
      }
      int score = LuaManager::getMe().call<int>("CalcTeamPwsScore", winave, loseave, winmax, losemax, data.dwOriginScore, win); // 胜队均分, 负队均分, 胜队最高, 负队最高, 自己积分, 胜负
      DWORD extrascore = user2ExtraScore.find(data.qwUserID) != user2ExtraScore.end() ? user2ExtraScore[data.qwUserID] : 0; // 额外积分
      score += extrascore;
      MatchScoreData* pUserScore = scorecmd.add_userscores();
      if (pUserScore)
      {
        pUserScore->set_charid(data.qwUserID);
        pUserScore->set_name(data.strName);
        pUserScore->mutable_portrait()->CopyFrom(data.oPortrait);
        pUserScore->set_profession(data.eProfession);
        pUserScore->set_score(score);
      }
      XLOG << "[组队排位赛-玩家积分], 玩家:" << data.qwUserID << data.strName << "当前积分:" << data.dwOriginScore << "获取最终分数:" << score << "其中额外积分:" << extrascore << "胜利:" << win << LOG_SCENE_INFO << XEND;

      // 奖励
      const SPwsRankCFG* pRankCFG = rCFG.getRankInfoByScore(data.dwOriginScore);
      if (pRankCFG == nullptr)
        return;
      TVecItemInfo items = win ? pRankCFG->vecWinItems : pRankCFG->vecLoseItems;
      SceneUser* user = SceneUserManager::getMe().getUserByID(data.qwUserID);
      // 在线直接发
      if (user)
      {
        user->getPackage().addItem(items, EPACKMETHOD_AVAILABLE);
        XLOG << "[组队排位赛-副本奖励], 玩家:" << user->name << user->id << "胜利:" << win << "段位:" << pRankCFG->eRank << LOG_SCENE_INFO << XEND;
      }
      else
      {
        MailManager::getMe().sendMail(data.qwUserID, rCFG.dwRewardMailID, items);
        XLOG << "[组队排位赛-副本奖励], 发送邮件, 玩家:" << data.qwUserID << "胜利:" << win << "段位:" << pRankCFG->eRank << LOG_SCENE_INFO << XEND;
      }
    };

    for (auto &u : winusers)
      calcscore(u, true);
    for (auto &u : loseusers)
      calcscore(u, false);

    // 发送match, 更新玩家积分
    PROTOBUF(scorecmd, scoresend, scorelen);
    thisServer->sendCmdToSession(scoresend, scorelen);
  }

  // 发送客户端结算面板
  TeamPwsReportFubenCmd cmd;
  m_stRedTeam.toData(cmd.add_teaminfo());
  m_stBlueTeam.toData(cmd.add_teaminfo());
  cmd.set_winteam(pWinTeam->eColor);
  for (auto &u : winusers)
  {
    // mvp 需在线
    SceneUser* mvpuser = SceneUserManager::getMe().getUserByID(u->qwUserID);
    if (!mvpuser)
      continue;
    mvpuser->toModelShowData(cmd.mutable_mvpuserinfo());
    XLOG << "[组队排位赛-Mvp], 玩家:" << mvpuser->name << mvpuser->id << LOG_SCENE_INFO << XEND;
    break;
  }

  onEnd();

  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);

  XLOG << "[组队排位赛-结算], 是否休闲模式:" << m_bRelax << "胜方:" << pWinTeam->qwTeamID << pWinTeam->eColor << "成员:";
  for (auto &u : m_stRedTeam.mapUserDatas)
    XLOG << u.first;
  XLOG << XEND;

  XLOG << "[组队排位赛-结算], 是否休闲模式:" << m_bRelax << "负方:" << pLoseTeam->qwTeamID << pLoseTeam->eColor << "成员:";
  for (auto &u : m_stBlueTeam.mapUserDatas)
    XLOG << u.first;
  XLOG << XEND;

  return false;
}

void TeamPwsScene::onEnd()
{
  // 复活玩家
  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  for (auto &s : entrySet)
  {
    SceneUser* us = dynamic_cast<SceneUser*> (s);
    if (us && us->isAlive() == false)
      us->relive(ERELIVETYPE_RETURN);
  }

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  // 删除buff
  auto clearbuff = [&](SPwsTeamData& team)
  {
    auto magic = team.oPairBuffMagic;
    const SPwsMagicCombine* pMagicCFG = rCFG.getMagicByBall(magic.first, magic.second);

    for (auto &m : team.mapUserDatas)
    {
      SceneUser* user = SceneUserManager::getMe().getUserByID(m.first);
      if (user == nullptr || user->getScene() != this)
        continue;
      // 魔法
      if (pMagicCFG)
      {
        for (auto &s : pMagicCFG->setBuffIDs)
          user->m_oBuff.del(s);
      }
      // 持球
      if (m.second.eHoldBall != EMAGICBALL_MIN)
      {
        for (auto &s : rCFG.setBallBuffs)
          user->m_oBuff.del(s);
        auto pball = rCFG.getBallData(m.second.eHoldBall);
        if (pball)
          user->m_oBuff.del(pball->dwBuffID);
      }
    }
  };
  clearbuff(m_stRedTeam);
  clearbuff(m_stBlueTeam);
}

void TeamPwsScene::onKillUser(SceneUser* pKiller, SceneUser* pDeath)
{
  if (pKiller == nullptr || pDeath == nullptr)
    return;

  if (m_eState != ETEAMPWS_FIRE)
    return;

  SPwsTeamData* pTeam = getTeamData(pKiller);
  if (pTeam == nullptr)
    return;
  auto p = pTeam->getUserData(pKiller->id);
  if (p == nullptr)
    return;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  DWORD add = (pTeam->dwScore + rCFG.dwKillScore > rCFG.dwWinScore ? rCFG.dwWinScore - pTeam->dwScore : rCFG.dwKillScore);

  QWORD curm = xTime::getCurMSec();

  pTeam->addScore(add);
  pTeam->qwScoreTime = curm;

  p->dwKillUserNum ++;
  p->dwKillScore += add;
  p->qwScoreTime = curm;
  p->qwKillScoreTime = curm;
  XLOG << "[组队排位赛-击杀玩家], 攻击方:" << pKiller->name << pKiller->id << "死亡方:" << pDeath->name << pDeath->id << "玩家当前击杀得分:" << p->dwKillScore << LOG_SCENE_INFO << XEND;

  checkWin();
}


void TeamPwsScene::onUserDie(SceneUser *user, xSceneEntryDynamic *killer)
{
  if (user == nullptr)
    return;

  SPwsTeamData* pTeam = getTeamData(user);
  if (pTeam == nullptr)
    return;
  auto p = pTeam->getUserData(user->id);
  if (p == nullptr)
    return;

  p->dwDieNum ++;

  if (p->eHoldBall != EMAGICBALL_MIN)
  {
    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
    auto pball = rCFG.getBallData(p->eHoldBall);
    if (pball)
    {
      MsgParams param;
      param.addString(user->name);
      param.addString(pball->strName);
      MsgManager::sendMapMsg(id, 25913, param);
    }
  }

  dropUserBall(user);
}

void TeamPwsScene::addHealUser(QWORD id, QWORD healID, DWORD hp)
{
  SceneUser* user = SceneUserManager::getMe().getUserByID(healID);
  if (user == nullptr)
    return;
  SPwsTeamData* pTeam = getTeamData(user);
  if (pTeam == nullptr)
    return;

  auto p = pTeam->getUserData(user->id);
  if (p == nullptr)
    return;

  p->dwHealHp += hp;
}

void TeamPwsScene::onTeamGetBall(SPwsTeamData* pTeam)
{
  if (pTeam == nullptr)
    return;
  SPwsTeamData* pOtherTeam = (pTeam == &m_stRedTeam ? &m_stBlueTeam : &m_stRedTeam);

  std::set<EMagicBallType> balls = pTeam->getAllBalls();
  if (balls.size() > 2)
    MsgManager::sendMapMsg(id, 25926, MsgParams(pTeam->strTeamColorName));

  switch(balls.size())
  {
    // 0->1, do nothing
    case 1:
      break;
    // 1->2, 给对方添加魔法
    case 2:
      {
        EMagicBallType e1 = *(balls.begin());
        EMagicBallType e2 = *(balls.rbegin());
        teamAddMagic(pOtherTeam, e1, e2);
      }
      break;
    // 2->3, 移除对方魔法
    case 3:
      {
        teamDelMagic(pOtherTeam);
      }
      break;
    // 3->4, do nothing
    case 4:
      break;
    default:
      break;
  }
  pTeam->setUpdate();
}

void TeamPwsScene::onTeamLoseBall(SPwsTeamData* pTeam, bool swap)
{
  if (pTeam == nullptr)
    return;
  SPwsTeamData* pOtherTeam = (pTeam == &m_stRedTeam ? &m_stBlueTeam : &m_stRedTeam);

  std::set<EMagicBallType> balls = pTeam->getAllBalls();
  switch(balls.size())
  {
    // 1->0, do nothing
    case 0:
      break;
    // 2->1, 移除对方魔法
    case 1:
      {
        teamDelMagic(pOtherTeam);
      }
      break;
    // 3->2, 自己若有魔法,失效; 给对方添加魔法
    case 2:
      {
        EMagicBallType e1 = *(balls.begin());
        EMagicBallType e2 = *(balls.rbegin());
        if (swap)
        {
          // 交换丢掉的球属于当前组合(若有), 导致不能继续当前魔法
          if (!pTeam->inMagic(e1, e2))
            teamDelMagic(pTeam);
        }
        else
        {
          // 自己队伍魔法失效(若有)
          teamDelMagic(pTeam);

          // 给对方添加魔法
          teamAddMagic(pOtherTeam, e1, e2);
        }
      }
      break;
    // 4->3, 若丢失的球属于当前自己魔法, 则魔法失效
    case 3:
      {
        if (!pTeam->hasMagic())
          break;
        TPairBallMagic pa = pTeam->oPairMagic;
        if (balls.find(pa.first) != balls.end() && balls.find(pa.second) != balls.end())
          break;
        teamDelMagic(pTeam);
      }
      break;
    default:
      break;
  }

  pTeam->setUpdate();
}

void TeamPwsScene::teamAddMagic(SPwsTeamData* pTeam, EMagicBallType ball1, EMagicBallType ball2)
{
  if (pTeam == nullptr)
    return;

  EMagicBallType firstBall = ball1 > ball2 ? ball2 : ball1;
  EMagicBallType secondBall = ball1 > ball2 ? ball1 : ball2;

  if (pTeam->inMagic(firstBall, secondBall))
    return;

  pTeam->oPairMagic = std::make_pair(firstBall, secondBall);

  // 未处于效果cd中, 立即生效
  if (pTeam->dwBallMagicCD == 0)
  {
    switchTeamMagicBuff(pTeam, pTeam->oPairMagic, true);

    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
    // 设置cd, 在此cd中
    pTeam->dwBallMagicCD = now() + rCFG.dwMagicCD;
    pTeam->setUpdate();
  }

  XLOG << "[组队排位赛-添加魔法组合], 队伍:" << pTeam->qwTeamID << "更换后魔法求组合:" << firstBall << secondBall << "效果CD:" << pTeam->dwBallMagicCD - now() << LOG_SCENE_INFO << XEND;
}

void TeamPwsScene::teamDelMagic(SPwsTeamData* pTeam)
{
  if (pTeam == nullptr || pTeam->hasMagic() == false)
    return;

  TPairBallMagic& pa = pTeam->oPairMagic;

  // 未处于效果cd中, 立即删除buff
  if (pTeam->dwBallMagicCD == 0)
    switchTeamMagicBuff(pTeam, pa, false);

  pa.first = EMAGICBALL_MIN;
  pa.second = EMAGICBALL_MIN;

  XLOG << "[组队排位赛-移除魔法组合], 队伍:" << pTeam->qwTeamID << LOG_SCENE_INFO << XEND;
}

void TeamPwsScene::switchTeamMagicBuff(SPwsTeamData* pTeam, TPairBallMagic magic, bool add)
{
  if (pTeam == nullptr)
    return;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  const SPwsMagicCombine* pMagicCFG = rCFG.getMagicByBall(magic.first, magic.second);
  if (pMagicCFG == nullptr)
    return;

  for (auto &m : pTeam->mapUserDatas)
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(m.first);
    if (user == nullptr || user->getScene() != this)
      continue;
    if (add)
    {
      for (auto &s : pMagicCFG->setBuffIDs)
        user->m_oBuff.add(s);
    }
    else
    {
      for (auto &s : pMagicCFG->setBuffIDs)
        user->m_oBuff.del(s);
    }
  }

  SPwsTeamData* pOtherTeam = (pTeam == &m_stRedTeam ? &m_stBlueTeam : &m_stRedTeam);
  MsgParams param;
  param.addString(pOtherTeam->strTeamColorName);
  param.addString(pMagicCFG->strName);
  // 设置当前buff效果对应的buffmagic
  if (add)
  {
    pTeam->oPairBuffMagic = magic;
    MsgManager::sendMapMsg(id, 25912, param);
  }
  else
  {
    pTeam->oPairBuffMagic = std::make_pair(EMAGICBALL_MIN, EMAGICBALL_MIN);
    MsgManager::sendMapMsg(id, 25914, param);
  }

  pTeam->setUpdate();
}

void TeamPwsScene::checkTeamMagicCD(DWORD cur)
{
  auto check = [&](SPwsTeamData& team)
  {
    if (team.dwBallMagicCD == 0 || cur < team.dwBallMagicCD)
      return;

    team.dwBallMagicCD = 0;

    // 检查效果
    if (team.oPairMagic == team.oPairBuffMagic)
      return;

    // 更换buff效果为当前魔法对应的buff
    switchTeamMagicBuff(&team, team.oPairBuffMagic, false);
    switchTeamMagicBuff(&team, team.oPairMagic, true);
  };

  check(m_stRedTeam);
  check(m_stBlueTeam);
}

void TeamPwsScene::summonBall(EMagicBallType eType)
{
  auto it = m_mapBallData.find(eType);
  if (it == m_mapBallData.end())
    return;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(it->second.first);
  if (npc && npc->isAlive())
  {
    XERR << "[组队排位赛-召唤元素球], 失败, 已存在, 球:" << eType << npc->id << XEND;
    return;
  }

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  auto m = rCFG.mapBallData.find(eType);
  if (m == rCFG.mapBallData.end())
    return;
  NpcDefine def;
  def.setPos(it->second.second);
  def.setID(m->second.dwSummonNpcID);
  npc = SceneNpcManager::getMe().createNpc(def, this);
  if (npc == nullptr)
    return;
  npc->setDeadDelAtonce();
  it->second.first = npc->id;
  XLOG << "[组队排位赛-召唤元素球], 召唤成功:" << npc->name << npc->id << eType << "副本:" << name << id << XEND;
}

SPwsTeamData* TeamPwsScene::getTeamData(QWORD teamid)
{
  if (m_stRedTeam.qwTeamID == teamid)
    return &m_stRedTeam;
  if (m_stBlueTeam.qwTeamID == teamid)
    return &m_stBlueTeam;
  return nullptr;
}

SPwsTeamData* TeamPwsScene::getTeamData(SceneUser* user)
{
  if (user == nullptr)
    return nullptr;
  if (user->getTeamID())
    return getTeamData(user->getTeamID());

  // 离队的情况
  if (m_stRedTeam.mapUserDatas.find(user->id) != m_stRedTeam.mapUserDatas.end())
    return &m_stRedTeam;
  if (m_stBlueTeam.mapUserDatas.find(user->id) != m_stBlueTeam.mapUserDatas.end())
    return &m_stBlueTeam;

  return nullptr;
}

SPwsUserData* TeamPwsScene::getUserData(SceneUser* user)
{
  if (!user)
    return nullptr;
  auto team = getTeamData(user);
  if (team == nullptr)
    return nullptr;
  return team->getUserData(user->id);
}

void TeamPwsScene::onUserCollectBall(SceneUser* user, SceneNpc* npc)
{
  if (user == nullptr || npc == nullptr || npc->isAlive() == false)
    return;

  if (m_eState != ETEAMPWS_FIRE)
    return;

  SPwsTeamData* pTeam = getTeamData(user->getTeamID());
  if (pTeam == nullptr)
    return;
  SPwsUserData* pUserData = pTeam->getUserData(user->id);
  if (pUserData == nullptr)
    return;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  for (auto &m : m_mapBallData)
  {
    if (npc->id != m.second.first)
      continue;

    // 丢掉老的
    if (pUserData->eHoldBall != EMAGICBALL_MIN)
    {
      if (pUserData->eHoldBall == m.first) //异常
        return;
      summonBall(pUserData->eHoldBall);

      auto pball = rCFG.getBallData(pUserData->eHoldBall);
      if (pball)
        user->m_oBuff.del(pball->dwBuffID);

      pUserData->eHoldBall = EMAGICBALL_MIN;
      onTeamLoseBall(pTeam, true);
    }
    else
    {
      // 从无到有, 设置更新时间戳
      if (pTeam->getAllBalls().empty())
        pTeam->dwBallScoreNextTime = 0;

      QWORD curm = xTime::getCurMSec();
      pTeam->qwScoreTime = curm;
      pUserData->qwScoreTime = curm;
      pUserData->dwBallBeginValueTime = curm / ONE_THOUSAND + rCFG.dwBallDelayTime;

      // 添加持球通用buff
      for (auto &s : rCFG.setBallBuffs)
        user->m_oBuff.add(s);
    }

    pUserData->eHoldBall = m.first;
    onTeamGetBall(pTeam);

    // 添加当前球对应的buff
    auto pball = rCFG.getBallData(pUserData->eHoldBall);
    if (pball)
      user->m_oBuff.add(pball->dwBuffID);
  }

  npc->setClearState();
  XLOG << "[组队排位赛-采集法球], 玩家:" << user->name << user->id << "队伍:" << user->getTeamID() << "采集法球成功, 法球:" << npc->name << npc->id << XEND;
}

void TeamPwsScene::onUserCollectBuff(SceneUser* user, SceneNpc* npc)
{
  if (user == nullptr || npc == nullptr || npc->isAlive() == false)
    return;

  if (m_eState != ETEAMPWS_FIRE)
    return;

  SPwsTeamData* pTeam = getTeamData(user);
  if (pTeam == nullptr)
    return;
  auto p = pTeam->getUserData(user->id);
  if (p == nullptr)
    return;
  // 仅持球者可以拾取
  if (p->eHoldBall == EMAGICBALL_MIN)
    return;

  // check
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  auto it = rCFG.mapNpc2Buffs.find(npc->getNpcID());
  if (it == rCFG.mapNpc2Buffs.end())
    return;

  // 添加buff效果
  // 个人
  QWORD curm = xTime::getCurMSec();
  for (auto &m : it->second.mapSelfBuff2Time)
  {
    user->m_oBuff.add(m.first);
    p->mapBuff2EndTime[m.first] = curm + m.second * ONE_THOUSAND;
  }
  // 组队
  for (auto &m : it->second.mapTeamBuff2Time)
  {
    for (auto &u : pTeam->mapUserDatas)
    {
      SceneUser* pu = SceneUserManager::getMe().getUserByID(u.first);
      if (pu)
        pu->m_oBuff.add(m.first);
      u.second.mapBuff2EndTime[m.first] = curm + m.second * ONE_THOUSAND;
    }
  }

  DWORD add = (pTeam->dwScore + rCFG.dwPickBuffScore > rCFG.dwWinScore ? rCFG.dwWinScore - pTeam->dwScore : rCFG.dwPickBuffScore);
  pTeam->addScore(add);
  pTeam->qwScoreTime = curm;

  p->dwBuffScore += add;
  p->qwScoreTime = curm;

  MsgParams param;
  param.addString(user->name);
  param.addString(it->second.strName);
  param.addNumber(add);
  MsgManager::sendMapMsg(id, 25916, param);

  XLOG << "[组队排位赛-拾取buff], 玩家:" << user->name << user->id << "npc:" << npc->name << npc->id << LOG_SCENE_INFO << XEND;

  npc->setClearState();

  checkWin();
}

void TeamPwsScene::dropUserBall(SceneUser* user)
{
  if (!user)
    return;

  SPwsTeamData* pTeam = getTeamData(user);
  if (pTeam)
  {
    auto p = pTeam->getUserData(user->id);
    if (p == nullptr)
      return;

    // 玩家有持球
    if (p->eHoldBall != EMAGICBALL_MIN)
    {
      const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
      summonBall(p->eHoldBall);

      // 删除buff
      for (auto &s : rCFG.setBallBuffs)
        user->m_oBuff.del(s);

      auto pball = rCFG.getBallData(p->eHoldBall);
      if (pball)
        user->m_oBuff.del(pball->dwBuffID);

      p->eHoldBall = EMAGICBALL_MIN;
      onTeamLoseBall(pTeam);
    }
  }
}

void TeamPwsScene::selectMagic(SceneUser* user, DWORD magicid)
{
  if (user == nullptr || user->getTeamID() == 0)
    return;

  //SPwsTeamData* pTeam = (m_stRedTeam.qwTeamID == user->getTeamID() ? &m_stRedTeam : &m_stBlueTeam);
  SPwsTeamData* pOtherTeam = (m_stRedTeam.qwTeamID != user->getTeamID() ? &m_stRedTeam : &m_stBlueTeam);

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
  const SPwsMagicCombine* pMagicCFG = rCFG.getMagicByID(magicid);
  if (pMagicCFG == nullptr)
    return;

  // 重复
  if (pOtherTeam->inMagic(pMagicCFG->eBall1, pMagicCFG->eBall2))
    return;

  // 检查持球
  std::set<EMagicBallType> balls = pOtherTeam->getAllBalls();
  if (balls.find(pMagicCFG->eBall1) == balls.end())
    return;
  if (balls.find(pMagicCFG->eBall2) == balls.end())
    return;

  // 删掉旧的magic
  if (pOtherTeam->hasMagic())
    teamDelMagic(pOtherTeam);

  // 设置magic
  teamAddMagic(pOtherTeam, pMagicCFG->eBall1, pMagicCFG->eBall2);

  XLOG << "[组队排位赛-选择魔法组合], 玩家:" << user->name << user->id << "对方队伍:" << pOtherTeam->qwTeamID << "魔法组合:" << pMagicCFG->eBall1 << pMagicCFG->eBall2 << LOG_SCENE_INFO << XEND;
}

void TeamPwsScene::updateTeamInfoToCient()
{
  if (!m_stRedTeam.bUpdate && !m_stBlueTeam.bUpdate)
    return;

  UpdateTeamPwsInfoFubenCmd cmd;
  if (m_stRedTeam.bUpdate)
  {
    m_stRedTeam.bUpdate = false;
    m_stRedTeam.toData(cmd.add_teaminfo());
  }
  if (m_stBlueTeam.bUpdate)
  {
    m_stBlueTeam.bUpdate = false;
    m_stBlueTeam.toData(cmd.add_teaminfo());
  }
  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);
}

void TeamPwsScene::queryDetailInfo(SceneUser* user)
{
  if (user == nullptr)
    return;
  QueryTeamPwsUserInfoFubenCmd cmd;
  m_stRedTeam.toData(cmd.add_teaminfo());
  m_stBlueTeam.toData(cmd.add_teaminfo());

  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

void TeamPwsScene::onUserLeaveTeam(SceneUser* user)
{
  if (user == nullptr)
    return;

  if (!m_bRelax)
  {
    SPwsUserData* p = getUserData(user);
    if (p == nullptr)
      return;
    p->bLeave = true;

    UserLeaveRaidMatchSCmd cmd;
    cmd.set_etype(EPVPTYPE_TEAMPWS);
    cmd.set_charid(user->id);
    PROTOBUF(cmd, send, len);

    thisServer->sendCmdToSession(send, len);
    XLOG << "[组队排位赛-玩家离队], 添加惩罚, 玩家:" << user->name << user->id << LOG_SCENE_INFO << XEND;
  }
}

void TeamPwsScene::onLeaveScene(SceneUser* user)
{
  if (!user)
    return;

  SPwsTeamData* pTeam = getTeamData(user);
  if (pTeam && pTeam->hasMagicBuff())
  {
    // 删除buff
    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG(m_bRelax);
    const SPwsMagicCombine* pMagicCFG = rCFG.getMagicByBall(pTeam->oPairBuffMagic.first, pTeam->oPairBuffMagic.second);
    if (pMagicCFG != nullptr)
    {
      for (auto &s : pMagicCFG->setBuffIDs)
        user->m_oBuff.del(s);
    }
  }

  auto p = getUserData(user);
  if (p)
  {
    QWORD time = xTime::getCurMSec();
    for (auto &m : p->mapBuff2EndTime)
    {
      if (m.second > time)
        user->m_oBuff.del(m.first);
    }
  }

  dropUserBall(user);
  user->m_oBuff.update(xTime::getCurMSec());
}

