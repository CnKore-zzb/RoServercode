#include "SceneUser.h"
#include "ServerTask.h"
#include "SceneServer.h"
#include "SceneManager.h"
#include "SceneItemManager.h"
#include "SceneNpcManager.h"
#include "SceneActManager.h"
#include "SceneBoothManager.h"
#include "SceneTrap.h"
#include "DScene.h"
#include "MsgManager.h"
#include "GuidManager.h"
#include "SceneUserManager.h"
#include "GMCommandRuler.h"
//#include "BarrageManager.h"
#include "xSha1.h"
#include "PlatLogManager.h"
#include "MailManager.h"
#include "StatisticsDefine.h"
#include "SkillManager.h"
#include "SessionWeather.pb.h"
#include "ChatRoomManager.h"
#include "SceneNpc.h"
#include "ChatManager_SC.h"
#include "RedisManager.h"
#include "GCharManager.h"
#include "CommonConfig.h"
#include "DepositConfig.h"
#include "MsgManager.h"
#include "SceneUser2.pb.h"
#include "GuildRaidConfig.h"
#include "AstrolabeConfig.h"
#include "ActivityManager.h"
#include "GuildCityManager.h"
#include "RegCmd.h"
#include "SceneUserData.h"
#include "SceneUserDataThread.h"
#include "PetWork.h"
#include "SceneShop.h"
#include "Menu.h"
#include "UserRecords.h"
#include "TeamRaidCmd.pb.h"
#include "ExchangeShop.h"
#include "DeadConfig.h"

SceneUser::SceneUser(QWORD uID, const char* name, QWORD uAccID, ServerTask* net) :
  xSceneEntryDynamic(uID, uAccID)
  , m_event(this)
  , m_oPackage(this)
  , m_oUserSceneData(this)
  , m_oQuest(this)
  , m_oQuestNpc(this)
  , m_oPet(this)
  , m_stage(this)
  , m_oPortrait(this)
  , m_oCarrier(this)
  , m_oTeamRaid(this)
  , m_oHair(this)
  , m_oGear(this)
  , m_oTip(this)
  , m_oMsg(this)
  //, m_oGameTime(this)
  , m_oShortcut(this)
  , m_oTower(this)
  , m_oFreyja(this)
  , m_oSeal(this)
  , m_oInter(this)
  , m_oManual(this)
  , m_oLaboratory(this)
  , m_oScenery(this)
  , m_oHands(this)
  , m_oTitle(this)
  , m_oTransform(this)
  , m_oDojo(this)
  , m_oHandNpc(this) 
  , m_oItemMusic(this)
  , m_oCamera(this)
  , m_oZone(this)
  , m_oDeposit(this)
  , m_oUserChat(this)
  , m_oUserStat(this)
  , m_oWeaponPet(this)
  , m_oGingerBread(this)
  , m_oTicket(this)
  , m_oShare(this)
  , m_oAchieve(this)
  , m_oAstrolabes(this)
  , m_oPhoto(this)
  , m_oUserMap(this)
  , m_oUserPet(this)
  , m_oPetAdventure(this)
  , m_oSceneFood(this)
  , m_oLottery(this)
  , m_oTutorTask(this)
  , m_oAction(this)
  , m_oEye(this)
  , m_oUserBeing(this)
  , m_oHighRefine(this)
  , m_oGuildChallenge(this)
  , m_oUserGvg(this)
  , m_oProposal(this)
  , m_oUserWedding(this)
  , m_oServant(this)
  , m_oProfession(this)
  , m_oBooth(this)
  , m_oDressUp(this)
  , m_oTransfer(this)
  , m_oUserElementElf(this)
  , m_oMatchData(this)
  , m_oCheatTag(this)
{
  accid = uAccID;
  set_name(name);
  gatetask = net;
  user_state = USER_STATE_NONE;

  initCharSuccess = false;
  m_blInScene = false;

  m_dwRecordSave = xTime::getCurSec();
#ifdef _LX_DEBUG
  // bOpenMoveTrack = true;
#endif
  m_oGTeam.setCharID(uID);
  m_oGuild.setAccID(uAccID);
  m_oGuild.setCharID(uID);

  m_pPetWork = new PetWork(this);
  m_pSceneShop = new SceneShop(this);
  m_pMenu = new Menu(this);
  m_pUserRecord = new UserRecords(this);
  m_pExchangeShop = new ExchangeShop(this);
}

SceneUser::~SceneUser()
{
  clearFighter();

  SAFE_DELETE(m_pPetWork);
  SAFE_DELETE(m_pUserRecord);
  SAFE_DELETE(m_pSceneShop);
  SAFE_DELETE(m_pMenu);
  SAFE_DELETE(m_pExchangeShop);
}

void SceneUser::toData(SocialUser* pUser)
{
  if (pUser == nullptr)
    return;

  pUser->set_accid(accid);
  pUser->set_charid(id);
  pUser->set_zoneid(thisServer->getZoneID());
  pUser->set_baselv(getLevel());
  pUser->set_name(name);
}

void SceneUser::sendCmdToMe(const void* cmd,DWORD len)
{
  if (!gatetask) return;

  BUFFER_CMD_SIZE(send, ForwardToUserGatewayCmd, sizeof(ForwardToUserGatewayCmd)+len);
  send->accid = accid;
  send->len = len;
  bcopy(cmd,send->data,len);
  gatetask->sendCmd(send, sizeof(ForwardToUserGatewayCmd)+len);
}

void SceneUser::refreshMe(QWORD curMSec)
{
  if (getScene() == nullptr)
    return;

  xSceneEntryDynamic::refreshMe(curMSec);

  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  {
    ExecutionTime_Scope;
    m_oMove.heartbeat(curMSec);
  }

  {
    ExecutionTime_Scope;
    m_oPackage.timer(curSec);
  }

  // 放在1s计时器外面, 方便属性变化立即刷新至前端
  if (curSec - m_dwDataTick > 10 && m_blInScene)
  {
    if (getScene()->isSuperGvg() == false || curSec >= m_dwLastRefreshTimeTick + CommonConfig::m_dwSuperGvgRefreshInterval)
    {
      m_dwLastRefreshTimeTick = curSec;
      m_dwDataTick = curSec;
      ExecutionTime_Scope;
      updateData(UnregType::Null);
      m_oFollower.refreshMe();
    }
  }

  // 1s计时器, 暂未用
  // if (curSec != m_dwSecTick)
  // {
  //   m_dwSecTick = curSec;
  // }
}

void SceneUser::onOneSecTimeUp(QWORD curMSec)
{
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  // save data
  if (curSec - m_dwRecordSave > DATA_FLUSH_INTERVAL)
  {
    if (m_blInScene)
    {
      ExecutionTime_Scope;
      m_dwRecordSave = curSec;
      refreshTeamTime(DATA_FLUSH_INTERVAL);
      refreshDataToRecord(UnregType::SaveBinary);
    }
  }

  ExecutionTime_Scope;
  xSceneEntryDynamic::onOneSecTimeUp(curMSec);

  // game time
  syncOwnTime(curSec);

  if (m_pCurFighter != nullptr)
    m_pCurFighter->timer(curSec);
  m_oQuest.timer(curSec);
  m_oTip.timer(curSec);
  m_oShortcut.timer(curSec);
  m_oSeal.timer(curSec);
  m_oWeaponPet.timer(curSec);
  m_oUserPet.timer(curSec);
  m_oAction.timer(curSec);
  m_oUserBeing.timer(curSec);
  m_oUserElementElf.timer(curSec);
  updateVar();

  // monters who lock me dec my flee/def
  watchLockEffect(curSec);

  // record monsters who attacked me
  DWORD chainAtkLmtTime = MiscConfig::getMe().getBeLockCFG().dwChainAtkLmtTime;
  for (auto s = m_vecTime2Attacker.begin(); s != m_vecTime2Attacker.end(); )
  {
    if (curSec > s->first + chainAtkLmtTime)
    {
      s = m_vecTime2Attacker.erase(s);
      continue;
    }
    ++s;
  }

  // team member pos
  syncMyPosToTeam();

  // music recover
  checkMusicRecover(curSec);

  // battle time calc
  {
    /*if (!m_setLockMeIDs.empty())
    {
      addTeamSkillTimePoint(curSec);
    }
    */

    // [-------A1---------B1--------] [---------A2------------B2---] [--------A3(B3)---------------]
    /*DWORD configGap = MiscConfig::getMe().getSystemCFG().dwBattleInterval;
    configGap = configGap != 0 ? configGap : 10;
    if (curSec >= m_dwNextCalcBattleTime)
    {
      DWORD begintime = ((m_dwLastBattleT != 0 && m_dwLastBattleT + configGap > m_dwBeginBattleT) ? m_dwLastBattleT : m_dwBeginBattleT);
      DWORD endtime = (m_dwEndBattleT != 0 ? m_dwEndBattleT : m_dwBeginBattleT);
      // add battle time
      bool isPvpMap = getScene() == nullptr ? false : (getScene() && getScene()->base && getScene()->base->isPvPMap());
      bool isRaidMap = getScene() == nullptr ? false : (getScene()->isDScene());
      if (!isPvpMap && !isRaidMap && endtime > begintime)
      {
        addBattleTime(endtime - begintime);

        //attict tips
        addcitTips(false);
      }
      m_dwLastBattleT = m_dwEndBattleT;
      m_dwNextCalcBattleTime = curSec + configGap;

      m_dwBeginBattleT = 0;
      m_dwEndBattleT = 0;
    }
    */

    DWORD configGap = MiscConfig::getMe().getSystemCFG().dwBattleInterval;
    configGap = configGap != 0 ? configGap : 10;
    if (curSec >= m_dwNextCalcBattleTime)
    {
      m_dwNextCalcBattleTime = curSec + configGap;
      if (/*isAlive() && */m_dwLastBattleT + configGap >= curSec && curSec >= m_dwLastBattleT)
      {
        if (isInBattleTimeMap())
        {
          addBattleTime(configGap);
          //attict tips
          addcitTips(false);
          m_event.onAddBattleTimeInBattle(configGap);
        }
      }
    }
  }

  /*if (m_dwBarrageTime && m_dwBarrageTime > curSec)
  {
    m_dwBarrageTime = curSec + 5;
    UserBarrageMsgCmd message;
    DWORD rand = randBetween(1, 10);
    for (DWORD i=0; i<rand; ++i)
    {
      DWORD r = randBetween(1, 1000);
      DialogBase *base = g_pDialogBaseM->getDataByID(r);
      if (base)
      {
        message.set_str(base->getText());
        ScenePos *p = message.mutable_msgpos();
        float x = getPos().x + randBetween(-10000, 10000);
        float z = sqrt(10000*10000 - x * x);
        p->set_x(x * 1000);
        p->set_y(getPos().getY() + randBetween(2000000, 5000000));
        p->set_z(z * 1000);
        ColorInfo *info = message.mutable_clr();
        r = randBetween(1,7);
        switch (r)
        {
          case 1:
            {
              info->set_r(255);
              info->set_g(255);
              info->set_b(255);
            }
            break;
          case 2:
            {
              info->set_g(255);
            }
            break;
          case 3:
            {
              info->set_b(255);
            }
            break;
          case 4:
            {
              info->set_r(255);
            }
            break;
          case 5:
            {
              info->set_r(255);
              info->set_g(255);
            }
            break;
          case 6:
            {
              info->set_r(255);
              info->set_g(105);
              info->set_b(180);
            }
            break;
          case 7:
            {
              info->set_r(138);
              info->set_g(43);
              info->set_b(226);
            }
            break;
          default:
            break;
        }
        PROTOBUF(message, send, len);
        BarrageManager::getMe().broadcastBarrage(send, len);
      }
    }
  }*/

  if (m_oItemMusic.hasMusicItem())
    m_oItemMusic.checkMusicItem(curSec);  
  
  if (m_bPlayCharge)
    playChargeNpcTick(curSec);

  m_oGingerBread.timer(curSec);

  // m_dwSkillQueueCntTime （s)打印一次技能队列超时异常统计
  if (curSec >= m_dwSkillQueueCntTick)
  {
    m_dwSkillQueueCntTick = curSec + CommonConfig::m_dwSkillQueueCntTime;

    const pair<DWORD, DWORD>& normalCnt = m_oSkillProcessor.getNormalQueueCnt();
    const pair<DWORD, DWORD>& noNormalCnt = m_oSkillProcessor.getNoNormalQueueCnt();
    float normalPer = 0;
    float noNormalPer = 0;
    float allper = 0;
    if (normalCnt.second)
      normalPer = (float)normalCnt.first / normalCnt.second;
    if (noNormalCnt.second)
      noNormalPer = (float)noNormalCnt.first / noNormalCnt.second;
    if (normalCnt.second || noNormalCnt.second)
      allper = (float)(noNormalCnt.first + normalCnt.first) / (noNormalCnt.second + normalCnt.second);

    DWORD mapid = getScene() ? getScene()->id : 0;
    float spd = getAttr(EATTRTYPE_MOVESPD);
    if (normalCnt.second || noNormalCnt.second)
    {
      XLOG << "[技能-超时统计], 玩家:" << name << id << "当前地图:" << mapid << "攻速:" << spd << "普攻: 总次数" << normalCnt.second << "超时次数" << normalCnt.first << "比率" << normalPer << "非普攻: 总次数"
        << noNormalCnt.second << "超时次数" << noNormalCnt.first << "比率" << noNormalPer << "总的比率:" << allper << XEND;
      m_oSkillProcessor.clearQueueCnt();
    }
  }

  if (m_bMarkRefreshTradeInfo)
  {
    m_bMarkRefreshTradeInfo = false;
    refreshPendingCount();
    refrshTradeBackMoneyPer();
  }

  if (m_bMarkRefreshBoothInfo)
  {
    m_bMarkRefreshBoothInfo = false;
    refreshBoothCount();
  }

  Scene* pScene = getScene();
  if (pScene != nullptr && pScene->isDScene() == true)
  {
    DScene* pDScene = dynamic_cast<DScene*>(pScene);
    if(pDScene != nullptr && pDScene->isPVPScene() == true)
    {
      pDScene->checkCombo(id,curSec);
    }
  }

  if (curSec >= m_dwKillCountPeriod)
  {
    DWORD period = MiscConfig::getMe().getSystemCFG().dwKillMonsterPeriod;
    m_dwKillCountPeriod = curSec + period;

    DWORD mincnt = MiscConfig::getMe().getSystemCFG().dwStatKillNum;
    for (auto &m : m_mapMonsterKillNum)
    {
      DWORD cnt = m.second;
      if (cnt > mincnt)
        m_oUserStat.addMonsterCnt(m.first, cnt);
    }
    m_mapMonsterKillNum.clear();
  }
  m_oSceneFood.timer(curSec);

  if (m_eStatus == ECREATURESTATUS_DEAD)
  {
    if (m_dwTime2Relive > m_dwDieTime && m_dwTime2Relive <= curSec)
    {
      delReliveMeUser(m_qwReliverID);
      SceneUser* user = SceneUserManager::getMe().getUserByID(m_qwReliverID);
      if (user)
        relive(ERELIVETYPE_SKILL, user);
      else
        relive(ERELIVETYPE_SKILL);
      m_dwTime2Relive = 0;
    }
    else
    {
      // 公会战死亡超过一段时间, 强制驱逐
      if (pScene && pScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
      {
        GuildFireScene* pGScene = dynamic_cast<GuildFireScene*>(pScene);
        if (pGScene)
        {
          const SGuildFireCFG& rCFG = MiscConfig::getMe().getGuildFireCFG();
          if (pGScene->getDefenseGuildID() == 0 || pGScene->getDefenseGuildID() != getGuild().id())
          {
            if (rCFG.dwAttExpelTime && curSec >= m_dwDieTime + rCFG.dwAttExpelTime)
            {
              relive(ERELIVETYPE_RETURN);
            }
          }
          else
          {
            if (rCFG.dwDefExpelTime && curSec >= m_dwDieTime + rCFG.dwDefExpelTime)
            {
              relive(ERELIVETYPE_RETURN);
            }
          }
        }
      }
      else if (pScene && pScene->isSuperGvg())
      {
        const SGuildFireCFG& rCFG = MiscConfig::getMe().getGuildFireCFG();
        if (rCFG.dwAttExpelTime && curSec >= m_dwDieTime + rCFG.dwAttExpelTime)
        {
          relive(ERELIVETYPE_RETURN);
        }
      }
      else if (pScene && pScene->getSceneType() == SCENE_TYPE_MVPBATTLE) // 强制驱逐
      {
        const SMvpBattleCFG& rCFG = MiscConfig::getMe().getMvpBattleCFG();
        if (rCFG.dwExpelTime && curSec >= m_dwDieTime + rCFG.dwExpelTime)
          relive(ERELIVETYPE_RETURN);
      }
    }
  }
}

void SceneUser::onFiveSecTimeUp(QWORD curMSec)
{
  ExecutionTime_Scope;
  xSceneEntryDynamic::onFiveSecTimeUp(curMSec);

  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  m_oHandNpc.timer(curSec);
  m_oAchieve.timer(curSec);
  setExpression(0);
  m_oUserGvg.timer(curSec);

  if (this->m_blShowSkill)
  {
    Cmd::GoToExitPosUserCmd message;
    PROTOBUF(message, send, len);
    doUserCmd((const Cmd::UserCmd*)send, len);
  }  
}

void SceneUser::onTenSecTimeUp(QWORD curMSec)
{
  ExecutionTime_Scope;
  xSceneEntryDynamic::onTenSecTimeUp(curMSec);

  m_oHands.timeTick(curMSec / ONE_THOUSAND);
  m_oPetAdventure.timer(curMSec / ONE_THOUSAND);
  m_oServant.timer(curMSec / ONE_THOUSAND);
  getExchangeShop().timer(curMSec / ONE_THOUSAND);
  checkSeatTime();
}

void SceneUser::onOneMinTimeUp(QWORD curMSec)
{
  ExecutionTime_Scope;
  xSceneEntryDynamic::onOneMinTimeUp(curMSec);
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  m_oQuest.queryOtherData(EOTHERDATA_DAILY);
  m_oQuest.resetCycleQuest();
  m_oPackage.refreshDeleteTimeItems();
  antiAddictRefresh();
  if (m_oUserSceneData.getCreditForbidRCTime())
  {
    if (m_oUserSceneData.getCreditForbidRCTime() <= curMSec / ONE_THOUSAND)
    {
      m_oUserSceneData.setCreditForbidRCTime(0);
      m_oUserSceneData.setCredit(MiscConfig::getMe().getCreditCFG().dwDefaultValue);
    }
  }
  m_oSceneFood.refreshSatiety();
  m_oTutorTask.timer(curSec);
  m_oUserSceneData.resetAEReward();
  notifyInviteeWeddingStart();
  onAltmanEnd();
  altmanCheck();
}

void SceneUser::onTenMinTimeUp(QWORD curMSec)
{
  ExecutionTime_Scope;
  xSceneEntryDynamic::onTenMinTimeUp(curMSec);
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  m_oDeposit.timeTick(curSec);

  // 在线时长累积一段时间, 未触发bad monitor, 增加信用度
  DWORD savedTime = m_oUserSceneData.getCreditSavedTime();
  DWORD curOnlineTime = curSec - m_oUserSceneData.getOnlineTime();
  const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
  if (savedTime + curOnlineTime >= rCFG.dwAddInterval)
  {
    m_oUserSceneData.setCreditSavedTime(0);
    m_oUserSceneData.addCredit(rCFG.dwIntervalValue);
    XLOG << "[玩家-信用度], 玩家" << name << id << "持续" << rCFG.dwAddInterval << "表现良好, 增加信用度" << rCFG.dwIntervalValue << XEND;
  }
  m_oAchieve.onCat(false);
  m_oTicket.cmdResend(curSec);
  m_oQuest.resetQuestTime(curSec);
  m_oAchieve.onWedding(EACHIEVECOND_WEDDING_DAY);
  m_oTicket.cmdResend(curMSec / 1000);
  m_oQuest.resetQuestTime(curMSec / ONE_THOUSAND);
  m_oServant.onAppearEvent(ETRIGGER_TIME_INTERVAL);
  m_oServant.checkActivityRealOpen();
  m_oQuest.onTenMinTimeUp(curSec);
  m_oCheatTag.timer();

  //检查发送当天获取zeny数
  if (m_oVar.getVarValue(EVARTYPE_DAY_GET_ZENY_COUNT) == 0)
  {
    m_oVar.setVarValue(EVARTYPE_DAY_GET_ZENY_COUNT, 1);
    m_oUserStat.sendDayGetZenyCountLog(m_oUserSceneData.getDailyNormalZeny(), m_oUserSceneData.getDailyChargeZeny());
    m_oUserSceneData.setDailyChargeZeny(0);
    m_oUserSceneData.setDailyNormalZeny(0);
  }
}

void SceneUser::onDailyRefresh(QWORD curMSec)
{
  ExecutionTime_Scope;
  xSceneEntryDynamic::onDailyRefresh(curMSec);

  m_oVar.refreshAllVar();

  LuaManager::getMe().call<void>("onDailyRefresh", this);
}

void SceneUser::addOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i, DWORD ext)
{
  if (gatetask)
  {
    AddOneLevelIndexGatewayCmd send;
    send.indexT = indexT;
    send.i = i;
    send.accid = accid;

    gatetask->sendCmd(&send, sizeof(send));
  }
}

void SceneUser::delOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i)
{
  if (gatetask)
  {
    DelOneLevelIndexGatewayCmd send;
    send.indexT = indexT;
    send.i = i;
    send.accid = accid;
    gatetask->sendCmd(&send, sizeof(send));
  }
}

void SceneUser::addTwoLevelIndex(TWO_LEVEL_INDEX_TYPE indexT, DWORD i, DWORD i2)
{
  if (gatetask)
  {
    AddTwoLevelIndexGatewayCmd send;
    send.i = i;
    send.i2 = i2;
    send.accid = accid;
    gatetask->sendCmd(&send, sizeof(send));
  }
}

void SceneUser::delTwoLevelIndex(TWO_LEVEL_INDEX_TYPE indexT, DWORD i, DWORD i2)
{
  if (gatetask)
  {
    DelTwoLevelIndexGatewayCmd send;
    send.i = i;
    send.i2 = i2;
    send.accid = accid;
    gatetask->sendCmd(&send, sizeof(send));
  }
}

bool SceneUser::initChar(SceneUserData *pData)
{
  if (!pData) return false;

  // init attribute
  if (initAttr() == false)
  {
    XERR << "[玩家-初始化]" << accid << id << name << "初始化属性对象失败" << XEND;
    return initCharSuccess;
  }

  const RecordUserData& oCmd = pData->getRecordUserData();
  const BlobAccData &oAccData = pData->m_oBlobAccData;
  const BlobData &oData = pData->m_oBlobData;

  // load data
  if (m_oUserSceneData.fromAccData(oCmd.acc()) == false)
  {
    XERR << "[玩家-初始化]" << accid << id << name << "账号信息初始化失败" << XEND;
    return initCharSuccess;
  }
  if (m_oUserSceneData.fromBaseData(oCmd.base()) == false)
  {
    XERR << "[玩家-初始化]" << accid << id << name << "基础信息初始化失败" << XEND;
    return initCharSuccess;
  }

  m_oUserSceneData.load(oAccData, oData);
  load(oData);

  if (m_pCurFighter == nullptr)
  {
    XERR << "[玩家-初始化]" << accid << id << name << "没有找到职位为" << m_oUserSceneData.getProfession() << "职业的武将" << XEND;
    return initCharSuccess;
  }
  m_stage.load(oData.stage());
  m_oQuestNpc.load(oAccData.questnpc(), oData.questnpc());
  m_oBuff.load(oData.buffer());
  m_oPortrait.load(oAccData.portrait(), oData.portrait());
  m_oHair.load(oData.hair());
  m_oGear.load(oData.gear());
  m_oTip.load(oData.tip());
  m_oFollower.load(oData.follower());
  m_oCDTime.load(oData.cd());
  m_oShortcut.load(oData.shortcut());
  m_oFreyja.load(oData.freyja());
  m_oLaboratory.load(oData.lab());
  m_oScenery.load(oAccData.scenery());
  m_oCarrier.load(oData.carrier());
  getMenu().load(oAccData.menu(), oData.menu());
  getSceneShop().load(oData.shop());
  m_oInter.load(oData.iter());
  m_oHands.load(oData.hand());
  m_oDojo.load(oData.dojo());
  if (m_oPackage.load(oAccData.pack(), oData.pack()/*, oCmd.store()*/) == false) return initCharSuccess;
  if (m_oQuest.load(oAccData.quest(), oData.quest()) == false) return initCharSuccess;
  m_oQuest.loadActivityQuest(oData.activityquest());
  m_oTitle.load(oAccData.title(), oData.title());
  m_oSpEffect.load(oData.speffect());
  m_oPet.load(oData.pet());
  m_oVar.load(oData.var());
  m_oMsg.load(oData.chatmsg());
  m_oTower.load(oData.tower());
  m_oSeal.load(oData.seal());
  m_oManual.load(oAccData.manual(), oData.manual(), oAccData.photo());
  m_oHandNpc.load(oData.handnpc());
  m_oCamera.load(oData.camera());
  m_oZone.load(oData.zone());
  m_oDeposit.load(oData.deposit());
  m_oUserChat.load(oData.chat());
  m_oWeaponPet.load(oData.weaponpet());
  m_oUserStat.load(oData.statvar());
  m_oTicket.load(oData.ticket());
  m_oShare.load(oData.share());
  m_oAstrolabes.load(oData.astrolabe());
  m_oAchieve.load(oAccData.achieve(), oData.achieve());
  m_oPhoto.load(oData.photo());
  m_oUserPet.load(oData.userpet());
  m_oPetAdventure.load(oData.petadventure());
  m_oSceneFood.load(oAccData.food(), oData.food());
  m_oTutorTask.load(oData.tutortask());
  m_oEye.load(oData.eye());
  m_oUserBeing.load(oData.being());
  m_oHighRefine.load(oData.highrefine());
  m_oGuildChallenge.load(oData.gchallenge());
  m_oUserGvg.load(oData.gvgdata());
  m_oServant.load(oAccData.servant(), oData.servant());
  m_oProfession.load(oData.profession());
  m_oBooth.load(oData.booth());
  if (m_pAttribute != nullptr)
    m_pAttribute->fromBlobAttr(oData.attr());
  m_oTransfer.load(oData.transfer());
  getExchangeShop().load(oData.exchangeshop());
  m_oUserElementElf.load(oData.element_elf());

  m_oCheatTag.load();

  m_oUserSceneData.loadActivity(oAccData.acevent(), oData.acevent());
  m_oUserSceneData.loadCredit(oAccData.credit());
  m_oUserSceneData.loadAccUser(oAccData.user());
  m_oUserSceneData.loadBoss(oData.boss());
  getSceneShop().loadAcc(oAccData.shop());
  m_oVar.loadAcc(oAccData.var());
  if (getPetWork().load(oAccData.petwork()) == false) return initCharSuccess;
  getUserRecords().loadAcc(oAccData.record());
  m_oLottery.loadAcc(oAccData.lottery());

  // init NEW char
  if (oCmd.base().createtime() == 0)
  {
    initCharSuccess = initNewChar();
    if (!initCharSuccess)
      return initCharSuccess;
  }
  initCharSuccess = true;

  SceneFighter* pNoviceFighter = getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter)
    pNoviceFighter->getSkill().loadAcc(oAccData.skill());

  // 普攻,重击不存数据库, 重新添加
  const SRoleBaseCFG* pRoleCFG = m_pCurFighter->getRoleCFG();
  if (pRoleCFG != nullptr)
  {
    m_pCurFighter->getSkill().addSkill(pRoleCFG->normalSkill, 0, ESOURCE_MIN, false);
    m_pCurFighter->getSkill().addSkill(pRoleCFG->strengthSkill, 0, ESOURCE_MIN, false);
  }

  //put on the end, buff is based on equip, card, skill, etc.
  m_oBuff.loadFromUser();

  // update attribute
  setCollectMark(ECOLLECTTYPE_BASE);
  //setCollectMark(ECOLLECTTYPE_SKILL);
  setCollectMark(ECOLLECTTYPE_EQUIP);
  //setCollectMark(ECOLLECTTYPE_CARD);
  setCollectMarkAllBuff();
  setCollectMark(ECOLLECTTYPE_FOOD);
  setCollectMark(ECOLLECTTYPE_PROFESSION);

  Attribute* pAttr = getAttribute();
  if (pAttr != nullptr)
    pAttr->setMark();
  updateAttribute();

  // check hp zero
  if (getAttr(EATTRTYPE_HP) == 0)
  {
    const SReliveCFG& rCFG = MiscConfig::getMe().getReliveCFG();
    changeHp(getAttr(EATTRTYPE_MAXHP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f, this);
    setSp(getAttr(EATTRTYPE_MAXSP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f);
  }

  bool isLogin = (getUserState() == USER_STATE_LOGIN);
  if (isLogin)
  {
    m_event.onLogin();
  }

  // preset msg
  m_oMsg.toClient();
  // clear update mark
  m_bitset.reset();
  // enter server
  enterServer();

  // clear tmp data
  m_userdata.clear();

  addEquipAttrAction();

  XLOG << "[玩家-初始化]" << accid << id << getProfession() << name << "初始化" << (initCharSuccess ? "成功" : "失败") <<"saveindex" << m_oUserSceneData.getSaveIndex() << XEND;
  return initCharSuccess;
}

void SceneUser::appendData(const Cmd::UserDataRecordCmd& rCmd)
{
  m_userdata.append(rCmd.data().c_str(), rCmd.data().size());
}

bool SceneUser::initNewChar()
{
  const SSystemCFG& rSysCFG = MiscConfig::getMe().getSystemCFG();

  // set default data
  m_oUserSceneData.setRolelv(1);
  m_oUserSceneData.setRoleexp(0);
  m_oUserSceneData.setSaveMap(rSysCFG.dwNewCharMapID);
  m_oUserSceneData.setCreateTime(xTime::getCurSec());

  // set default pos
  const SceneBase* pBase = SceneManager::getMe().getDataByID(rSysCFG.dwNewCharMapID);
  if (pBase == nullptr)
  {
    XERR << "[创建角色失败]" << accid << id << name << " 找不到新手地图" << XEND;
    return false;
  }
  m_oUserSceneData.setOnlineMapPos(rSysCFG.dwNewCharMapID, xPos());

  // create novice
  clearFighter();
  const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(EPROFESSION_NOVICE);
  if (pCFG == nullptr)
  {
    XERR << "[创建角色失败]" << accid << id << name << "找不到初心者职业配置" << XEND;
    return false;
  }
  m_pCurFighter = NEW SceneFighter(this, pCFG);
  if (m_pCurFighter == nullptr)
    return false;
  m_pCurFighter->setProfession(EPROFESSION_NOVICE);
  m_pCurFighter->setJobLv(1);
  m_vecFighters.push_back(m_pCurFighter);

  // init default manual data
  m_oManual.initDefaultData();

  // process levelup
  m_event.onBaseLevelup(1, 1);

  // add normal skill
  m_pCurFighter->getSkill().addSkill(pCFG->normalSkill, 0, ESOURCE_MIN);
  m_pCurFighter->getSkill().addSkill(pCFG->strengthSkill, 0, ESOURCE_MIN);

  // set fighter data : do this after process levelup for open skillgrid
  m_pCurFighter->getSkill().refreshEnableSkill();
  m_pCurFighter->getSkill().clearUpdate();

  // get NEW role config
  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();

  // give default item
  BasePackage* pMainPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr || pMainPack->checkAddItem(rCFG.vecItems, EPACKMETHOD_CHECK_WITHPILE) == false)
  {
    XERR << "[玩家-创建]" << accid << id << getProfession() << name << "添加初始化道具失败" << XEND;
    //return false;
  }
  //pMainPack->addItemFull(rCFG.vecItems, false, false);
  if (pMainPack)
  {
    TVecItemInfo vecItems = rCFG.vecItems;
    pMainPack->addItem(vecItems, EPACKMETHOD_CHECK_WITHPILE, false, false);
    pMainPack->clearUpdateIDs();
  }

  // set init purify
  m_oUserSceneData.setPurify(rCFG.dwPurify);

  // give default hair
  for (auto v = rCFG.vecHair.begin(); v != rCFG.vecHair.end(); ++v)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getHairCFG(*v);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-创建]" << accid << id << getProfession() << name << "添加发型" << *v << "失败,未在Table_Item.txt表中找到" << XEND;
      continue;
    }
    if (m_oHair.checkAddHair(pCFG->dwTypeID) == true)
    {
      m_oHair.addNewHair(pCFG->dwTypeID);
      m_oManual.onItemAdd(*v, true, false);
    }
  }

  // give default eye
  for (auto v = rCFG.vecEye.begin(); v != rCFG.vecEye.end(); ++v)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(*v);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-创建]" << accid << id << getProfession() << name << "添加美瞳" << *v << "失败,未在Table_Item.txt表中找到" << XEND;
      continue;
    }
    m_oEye.addNewEye(*v);
  }

  // give default action and expression
  for (auto &v : rCFG.vecAction)
    m_oUserSceneData.addAction(v);
  for (auto &v : rCFG.vecExpression)
    m_oUserSceneData.addExpression(v);

  // give default manual - unlock
  for (auto v = rCFG.vecManualCard.begin(); v != rCFG.vecManualCard.end(); ++v)
    m_oManual.onItemAdd(*v, false, false, false, ESOURCE_PICKUP);

  // put default manual - lock
  for (auto &v : rCFG.vecManualNpcs)
    m_oManual.onKillMonster(v, false);
  for (auto &v : rCFG.vecManualItems)
    m_oManual.onItemAdd(v, false, false);

  // give default portrait
  if (m_oUserSceneData.getGender() == EGENDER_MALE)
  {
    for (auto v = rCFG.vecMalePortrait.rbegin(); v != rCFG.vecMalePortrait.rend(); ++v)
    {
      m_oPortrait.addNewItems(*v, false);
      m_oPortrait.usePortrait(*v, false);
      m_oManual.onItemAdd(*v, true, false);
    }
  }
  else
  {
    for (auto v = rCFG.vecFemalePortrait.rbegin(); v != rCFG.vecFemalePortrait.rend(); ++v)
    {
      m_oPortrait.addNewItems(*v, false);
      m_oPortrait.usePortrait(*v, false);
      m_oManual.onItemAdd(*v, true, false);
    }
  }
  for (auto v = rCFG.vecFrame.rbegin(); v != rCFG.vecFrame.rend(); ++v)
  {
    m_oPortrait.addNewItems(*v, false);
    m_oPortrait.useFrame(*v, false);
    m_oManual.onItemAdd(*v, true, false);
  }

  // buff
  for (auto v = rCFG.vecBuffs.begin(); v != rCFG.vecBuffs.end(); ++v)
    m_oBuff.add(*v, this);

  // active map
  for (auto v = rCFG.vecActiveMap.begin(); v != rCFG.vecActiveMap.end(); ++v)
    m_oFreyja.addFreyja(*v, false);

  // set user hair
  const SItemCFG* pHairCFG = ItemConfig::getMe().getHairCFG(m_oUserSceneData.getShowHair());
  if (pHairCFG == nullptr)
  {
    XERR << "[玩家-创建]" << accid << id << getProfession() << name << "发型 :" << m_oUserSceneData.getShowHair() << "未在Table_Item.txt表中找到" << XEND;
    return false;
  }
  m_oHair.useHairFree(pHairCFG->dwTypeID);
  m_oHair.useColorFree(m_oUserSceneData.getShowHairColor());

  // init hp
  setCollectMark(ECOLLECTTYPE_BASE);
  //setCollectMark(ECOLLECTTYPE_SKILL);
  setCollectMark(ECOLLECTTYPE_EQUIP);
  //setCollectMark(ECOLLECTTYPE_CARD);
  setCollectMarkAllBuff();

  updateAttribute();

  m_pCurFighter->setHp(m_pAttribute->getAttr(EATTRTYPE_MAXHP));
  m_pCurFighter->setSp(m_pAttribute->getAttr(EATTRTYPE_MAXSP));

  // remove red tip
  m_oTip.removeRedTip(EREDSYS_ROLE_IMG);
  m_oTip.removeRedTip(EREDSYS_MONSTER_IMG);
  m_oTip.removeRedTip(EREDSYS_PHOTOFRAME);
  m_oTip.clearUpdate();

  // save data if NEW role
  saveDataNow();

  reqFixBranch();

  XLOG << "[玩家-创建] " << accid << id << getProfession() << name << " 初始化" << XEND;
  return true;
}

void SceneUser::updateData(UnregType eType, EUserSyncType eSync /*= EUSERSYNCTYPE_SYNC*/)
{
  if (!initCharSuccess)
    return;

  // change mark
  UserSyncCmd sync;
  UserNineSyncCmd nine;
  UserDataSync session;

  // set sync type
  sync.set_type(eSync);

  // collect change datas
  fetchChangeData(sync, nine, session);

  // update attr to team
  for (int i = 0; i < session.attrs_size(); ++i)
  {
    switch (session.attrs(i).type())
    {
      case EATTRTYPE_HP:
        setMark(EMEMBERDATA_HP);
        break;
      case EATTRTYPE_MAXHP:
        setMark(EMEMBERDATA_MAXHP);
        break;
      case EATTRTYPE_SP:
        setMark(EMEMBERDATA_SP);
        break;
      case EATTRTYPE_MAXSP:
        setMark(EMEMBERDATA_MAXSP);
        break;
      default:
        break;
    }
  }
  updateDataToTeam();

  // send server sync
  if ((session.datas_size() > 0 || session.attrs_size() > 0) && (eType == UnregType::Null || eType == UnregType::ChangeScene))
  {
    PROTOBUF(session, send, len);
    thisServer->sendCmdToSession(send, len);

#ifdef _DEBUG
    for (int i = 0; i < session.attrs_size(); ++i)
    {
      const UserAttr& rAttr = session.attrs(i);
      XDBG << "[玩家-属性会话同步]" << accid << id << getProfession() << name << "attr:" << rAttr.type() << "value:" << rAttr.value() << XEND;
    }
    for (int i = 0; i < session.datas_size(); ++i)
    {
      const UserData& rData = session.datas(i);
      XDBG << "[玩家-数据会话同步]" << accid << id << getProfession() << name << "data:" << rData.type() << "value:" << rData.value() << "data :" << rData.data() << XEND;
    }
#endif
  }

  // send client sync
  if (eType == UnregType::Null && (sync.attrs_size() > 0 || sync.datas_size() > 0))
  {
    PROTOBUF(sync, send1, len1);
    sendCmdToMe(send1, len1);

#ifdef _DEBUG
    for (int i = 0; i < sync.attrs_size(); ++i)
    {
      const UserAttr& rAttr = sync.attrs(i);
      XDBG << "[玩家-属性同步]" << accid << id << getProfession() << name << "attr:" << rAttr.type() << "value:" << rAttr.value() << "p:" << sync.pointattrs(i).value() << XEND;
    }
    for (int i = 0; i < sync.datas_size(); ++i)
    {
      const UserData& rData = sync.datas(i);
      XDBG << "[玩家-数据同步]" << accid << id << getProfession() << name << "data:" << rData.type() << "value:" << rData.value() << "data:" << rData.data() << XEND;
    }
#endif
  }

  // send client nine sync
  if (getUserState() != USER_STATE_LOGIN && eType == UnregType::Null && (nine.datas_size() > 0 || nine.attrs_size() > 0))
  {
    PROTOBUF(nine, send2, len2);
    sendCmdToNine(send2, len2, id);

#ifdef _DEBUG
    for (int i = 0; i < nine.attrs_size(); ++i)
    {
      const UserAttr& rAttr = nine.attrs(i);
      XDBG << "[玩家-属性九屏同步]" << accid << id << getProfession() << name << "attr:" << rAttr.type() << "value:" << rAttr.value() << XEND;
    }
    for (int i = 0; i < nine.datas_size(); ++i)
    {
      const UserData& rData = nine.datas(i);
      XDBG << "[玩家-数据九屏同步]" << accid << id << getProfession() << name << "data:" << rData.type() << "value:" << rData.value() << "data :" << rData.data() << XEND;
    }
#endif
  }

  m_bitset.reset();
}

void SceneUser::fetchChangeData(UserSyncCmd& sync, UserNineSyncCmd& nine, UserDataSync& session)
{
  if (m_pCurFighter == nullptr)
  {
    XERR << "[玩家-数据收集]" << accid << id << getProfession() << name << "未有武将信息,异常账号,收集失败" << XEND;
    return;
  }

  // fetch attrs
  UserAttribute* pUserAttr = dynamic_cast<UserAttribute*>(m_pAttribute);
  if (pUserAttr != nullptr)
    pUserAttr->collectSyncAttrCmd(sync, session, nine);

  nine.set_guid(id);
  session.set_id(id);

  // fetch datas
  if (m_bitset.any() == false)
    return;

  for (DWORD d = 0; d < EUSERDATATYPE_MAX; ++d)
  {
    if (m_bitset.test(d) == false || EUserDataType_IsValid(d) == false)
      continue;

    EUserDataType eType = static_cast<EUserDataType>(d);
    switch (eType)
    {
      case EUSERDATATYPE_MIN:
      case EUSERDATATYPE_MAPID:   // 对于会话同步,在notifyChangeScene中优先同步,防止跟随延迟
      case EUSERDATATYPE_RAIDID:  // 对于会话同步,在notifyChangeScene中优先同步,防止跟随延迟
        break;
      case EUSERDATATYPE_SEX:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getGender());
        add_data(session.add_datas(), eType, m_oUserSceneData.getGender());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getGender());
        break;
      case EUSERDATATYPE_PROFESSION:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getProfession());
        add_data(session.add_datas(), eType, m_oUserSceneData.getProfession());
        break;
      case EUSERDATATYPE_DESTPROFESSION:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDestProfession());
        break;
      case EUSERDATATYPE_JOBLEVEL:
        add_data(sync.add_datas(), eType, m_pCurFighter->getJobLv());
        add_data(session.add_datas(), eType, m_pCurFighter->getJobLv());
        break;
      case EUSERDATATYPE_ROLELEVEL:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getRolelv());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getRolelv());
        add_data(session.add_datas(), eType, m_oUserSceneData.getRolelv());
        break;
      case EUSERDATATYPE_JOBEXP:
        add_data(sync.add_datas(), eType, m_pCurFighter->getJobExp());
        break;
      case EUSERDATATYPE_ROLEEXP:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getRoleexp());
        break;
      case EUSERDATATYPE_CHARGE:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getCharge());
        break;
      case EUSERDATATYPE_DIAMOND:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDiamond());
        break;
      case EUSERDATATYPE_SILVER:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getSilver());
        break;
      case EUSERDATATYPE_GOLD:  // 金币取消 : momo
      case EUSERDATATYPE_PURIFY:  // 功能废弃,代码保留
        break;
      case EUSERDATATYPE_GARDEN:      // 乐园币货币废弃,代码保留
        //add_data(sync.add_datas(), eType, pMainPack->getItemCount(ITEM_GARDEN));
        break;
      case EUSERDATATYPE_FRIENDSHIP:  // 友情之证货币废弃,代码保留
        //add_data(sync.add_datas(), eType, m_oUserSceneData.getFriendShip());
        break;
      case EUSERDATATYPE_PVPCOIN:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getPvpCoin());
        break;
      case EUSERDATATYPE_LOTTERY:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getLotteryCoin());
        break;
      case EUSERDATATYPE_GUILDHONOR:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getGuildHonor());
        break;
      case EUSERDATATYPE_ONLINETIME:
      case EUSERDATATYPE_OFFLINETIME:
      case EUSERDATATYPE_ADDICT:
      case EUSERDATATYPE_BATTLETIME:
      case EUSERDATATYPE_REBATTLETIME:
      case EUSERDATATYPE_USEDBATTLETIME:
      case EUSERDATATYPE_TUTORBATTLETIME:
      case EUSERDATATYPE_USEDTUTORBATTLETIME:
      case EUSERDATATYPE_ADDICTTIPSTIME:
        break;
      case EUSERDATATYPE_HAIR:
        add_data(sync.add_datas(), eType, m_oHair.getCurHair());
        add_data(session.add_datas(), eType, m_oHair.getRealHair());
        add_data(nine.add_datas(), eType, m_oHair.getCurHair());

        break;
      case EUSERDATATYPE_HAIRCOLOR:
        add_data(sync.add_datas(), eType, m_oHair.getCurHairColor());
        add_data(session.add_datas(), eType, m_oHair.getRealHairColor());
        add_data(nine.add_datas(), eType, m_oHair.getCurHairColor());
        break;
      case EUSERDATATYPE_CLOTHCOLOR:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getClothColor());
        add_data(session.add_datas(), eType, m_oUserSceneData.getClothColor());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getClothColor());
        break;
      case EUSERDATATYPE_SHADERCOLOR:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getShaderColor());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getShaderColor());
        break;
      case EUSERDATATYPE_LEFTHAND:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getLefthand());
        //add_data(session.add_datas(), eType, m_oUserSceneData.getLefthand(true));
        add_data(nine.add_datas(), eType, m_oUserSceneData.getLefthand());
        break;
      case EUSERDATATYPE_RIGHTHAND:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getRighthand());
        //add_data(session.add_datas(), eType, m_oUserSceneData.getRighthand(true));
        add_data(nine.add_datas(), eType, m_oUserSceneData.getRighthand());
        break;
      case EUSERDATATYPE_BODY:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getBody());
        add_data(session.add_datas(), eType, m_oUserSceneData.getRealBody());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getBody());
        break;
      case EUSERDATATYPE_BODYSCALE:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getBodyScale());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getBodyScale());
        break;
      case EUSERDATATYPE_HEAD:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getHead());
        add_data(session.add_datas(), eType, m_oUserSceneData.getHead(true));
        add_data(nine.add_datas(), eType, m_oUserSceneData.getHead());
        m_oItemMusic.checkHeadEffect(m_oUserSceneData.getHead(), getAction(), eType);
        break;
      case EUSERDATATYPE_BACK:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getBack());
        add_data(session.add_datas(), eType, m_oUserSceneData.getBack(true));
        add_data(nine.add_datas(), eType, m_oUserSceneData.getBack());
        m_oItemMusic.checkHeadEffect(m_oUserSceneData.getBack(), getAction(), eType);
        break;
      case EUSERDATATYPE_FACE:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getFace());
        add_data(session.add_datas(), eType, m_oUserSceneData.getFace(true));
        add_data(nine.add_datas(), eType, m_oUserSceneData.getFace());
        m_oItemMusic.checkHeadEffect(m_oUserSceneData.getFace(), getAction(), eType);
        break;
      case EUSERDATATYPE_TAIL:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getTail());
        add_data(session.add_datas(), eType, m_oUserSceneData.getTail(true));
        add_data(nine.add_datas(), eType, m_oUserSceneData.getTail());
        m_oItemMusic.checkHeadEffect(m_oUserSceneData.getTail(), getAction(), eType);
        break;
      case EUSERDATATYPE_MOUNT:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getMount());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getMount());
        break;
      case EUSERDATATYPE_MOUTH:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getMouth());
        add_data(session.add_datas(), eType, m_oUserSceneData.getMouth(true));
        add_data(nine.add_datas(), eType, m_oUserSceneData.getMouth());
        m_oItemMusic.checkHeadEffect(m_oUserSceneData.getMouth(), getAction(), eType);
        break;
      case EUSERDATATYPE_STRPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getAttrPoint(EATTRTYPE_STR));
        break;
      case EUSERDATATYPE_INTPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getAttrPoint(EATTRTYPE_INT));
        break;
      case EUSERDATATYPE_AGIPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getAttrPoint(EATTRTYPE_AGI));
        break;
      case EUSERDATATYPE_DEXPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getAttrPoint(EATTRTYPE_DEX));
        break;
      case EUSERDATATYPE_VITPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getAttrPoint(EATTRTYPE_VIT));
        break;
      case EUSERDATATYPE_LUKPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getAttrPoint(EATTRTYPE_LUK));
        break;
      case EUSERDATATYPE_TOTALPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getTotalPoint());
        break;
      case EUSERDATATYPE_USEDPOINT:
        break;
      case EUSERDATATYPE_NORMAL_SKILL:
        add_data(sync.add_datas(), eType, m_pCurFighter->getSkill().getNormalSkill());
        add_data(nine.add_datas(), eType, m_pCurFighter->getSkill().getNormalSkill());
        break;
      case EUSERDATATYPE_COLLECT_SKILL: // 客户端读本地配置
      case EUSERDATATYPE_TRANS_SKILL:   // 客户端读本地配置
        break;
      case EUSERDATATYPE_SKILL_POINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getSkill().getSkillPoint());
        break;
      case EUSERDATATYPE_STATUS:
        {
          ECreatureStatus status = getStatus();
          if (status == ECREATURESTATUS_DEAD)
          {
            if (inReliveStatus())
              status = ECREATURESTATUS_INRELIVE;
          }
          add_data(sync.add_datas(), eType, status);
          add_data(nine.add_datas(), eType, status);
        }
        break;
      case EUSERDATATYPE_EQUIPMASTER:   // 功能废弃:代码保留
      case EUSERDATATYPE_REFINEMASTER:  // 功能废弃:代码保留
        break;
      case EUSERDATATYPE_PORTRAIT:
        add_data(sync.add_datas(), eType, m_oPortrait.getCurPortrait());
        add_data(session.add_datas(), eType, m_oPortrait.getCurPortrait());
        add_data(nine.add_datas(), eType, m_oPortrait.getCurPortrait());
        break;
      case EUSERDATATYPE_FRAME:         // 功能废弃:代码保留
        //addSync(eType, m_oPortrait.getCurFrame());
        //addsession(eType, m_oPortrait.getCurFrame(), "");
        //addnine(eType, m_oPortrait.getCurFrame());
        break;
      case EUSERDATATYPE_BATTLEPOINT:
        add_data(sync.add_datas(), eType, m_pCurFighter->getBattlePoint());
        break;
      case EUSERDATATYPE_PET_PARTNER:
        add_data(sync.add_datas(), eType, m_oPet.getPartnerID());
        add_data(nine.add_datas(), eType, m_oPet.getPartnerID());
        break;
      case EUSERDATATYPE_PET_SELF:
        break;
      case EUSERDATATYPE_CREATETIME:
        add_data(session.add_datas(), eType, m_oUserSceneData.getCreateTime());
        break;
      case EUSERDATATYPE_SAVEMAP:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getSaveMap());
        break;
      case EUSERDATATYPE_FOLLOWID:  // FollowerUser也同步了,有时间可和客户端优化
        //add_data(sync.add_datas(), eType, m_oUserSceneData.getFollowerID());
        add_data(session.add_datas(), eType, m_oUserSceneData.getFollowerID());
        break;
      case EUSERDATATYPE_HANDID:
        //add_data(session.add_datas(), eType, m_oHands.isMaster() == true ? m_oHands.getFollowerID() : m_oHands.getMasterID());
        //add_data(session.add_datas(), eType, m_oHands.getHandFollowID());
        add_data(nine.add_datas(), eType, m_oHands.getHandFollowID());
        break;
      case EUSERDATATYPE_CARRIER:
        add_data(session.add_datas(), eType, m_oCarrier.getCarrierID());
        break;
      case EUSERDATATYPE_MUSIC_CURID:
        add_data(sync.add_datas(), eType, m_dwMusicID);
        break;
      case EUSERDATATYPE_MUSIC_START:
        add_data(sync.add_datas(), eType, m_dwStartTime);
        break;
      case EUSERDATATYPE_MUSIC_DEMAND:
        add_data(sync.add_datas(), eType, static_cast<DWORD>(m_bDemand));
        add_data(nine.add_datas(), eType, static_cast<DWORD>(m_bDemand));
        break;
      case EUSERDATATYPE_DIR:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDir());
        add_data(nine.add_datas(), eType, m_oUserSceneData.getDir());
        break;
      case EUSERDATATYPE_GAGTIME:
      case EUSERDATATYPE_NOLOGINTIME:
        break;
      case EUSERDATATYPE_EYE:
        add_data(sync.add_datas(), eType, m_oEye.getCurID());
        add_data(nine.add_datas(), eType, m_oEye.getCurID());
        add_data(session.add_datas(), eType, m_oEye.getCurID(true));
        break;
      case EUSERDATATYPE_NAME:
        add_data(sync.add_datas(), eType, 0, name);
        add_data(nine.add_datas(), eType, 0, name);
        add_data(session.add_datas(), eType, 0, name);
        break;
      case EUSERDATATYPE_GIFTPOINT:
        add_data(sync.add_datas(), eType, m_oGuild.giftpoint());
        break;
      case EUSERDATATYPE_MANUAL_LV:
        add_data(session.add_datas(), eType, m_oManual.getManualLv());
        break;
      case EUSERDATATYPE_MANUAL_EXP:
        add_data(session.add_datas(), eType, m_oManual.getManualPoint());
        break;
      case EUSERDATATYPE_CUR_TITLE:
        add_data(session.add_datas(), eType, m_oTitle.getCurTitle(ETITLE_TYPE_MANNUAL));
        break;
      case EUSERDATATYPE_KILLERNAME:
        add_data(sync.add_datas(), eType, 0, m_strKillerName);
        m_strKillerName.clear();
        break;
      case EUSERDATATYPE_DROPBASEEXP:
        add_data(sync.add_datas(), eType, m_dwDieDropExp);
        break;
      case EUSERDATATYPE_QUERYTYPE:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getQueryType());
        add_data(session.add_datas(), eType, m_oUserSceneData.getQueryType());
        break;
      case EUSERDATATYPE_QUERYWEDDINGTYPE:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getQueryWeddingType());
        add_data(session.add_datas(), eType, m_oUserSceneData.getQueryWeddingType());
        break;
      case EUSERDATATYPE_NORMALSKILL_OPTION:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getNormalSkillOption());
        break;
      case EUSERDATATYPE_FASHIONHIDE:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getFashionHide());
        break;
      case EUSERDATATYPE_BLINK:
        add_data(session.add_datas(), eType, m_oUserSceneData.getBlink());
        break;
      case EUSERDATATYPE_ZONEID:
        add_data(sync.add_datas(), eType, getClientZoneID(m_oUserSceneData.getZoneID()));
        break;
      case EUSERDATATYPE_DEST_ZONEID:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDestZoneID());
        break;
      case EUSERDATATYPE_ORIGINAL_ZONEID:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getOriginalZoneID());
        break;
      case EUSERDATATYPE_QUOTA:
        add_data(sync.add_datas(), eType, m_oDeposit.getQuota());
        break;
      case EUSERDATATYPE_QUOTA_LOCK:
        add_data(sync.add_datas(), eType, m_oDeposit.getQuotaLock());
        break;
      case EUSERDATATYPE_HASCHARGE:
      {
        if (m_oDeposit.everHasQuota())
          add_data(sync.add_datas(), eType, 1);
        else
          add_data(sync.add_datas(), eType, 0);
        break;
      }
      case EUSERDATATYPE_TREESTATUS:
      case EUSERDATATYPE_ALPHA:
        break;
      case EUSERDATATYPE_ZENY_DEBT:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getZenyDebt());
        add_data(session.add_datas(), eType, m_oUserSceneData.getZenyDebt());
        break;
      case EUSERDATATYPE_PVP_COLOR:
      {
        if (thisServer->getZoneCategory())
          add_data(sync.add_datas(), eType, m_oZone.getColorIndex());
        else if (getScene() && (getScene()->isSuperGvg() || getScene()->getSceneType() == SCENE_TYPE_TEAMPWS))
          add_data(sync.add_datas(), eType, m_oZone.getColorIndex());
        else
          add_data(sync.add_datas(), eType, 0);
        break;
      }
      case EUSERDATATYPE_GUILDRAIDINDEX:
        break;
      case EUSERDATATYPE_CONTRIBUTE:
        add_data(sync.add_datas(), eType, m_oGuild.contribute());
        break;
      case EUSERDATATYPE_MONTHCARD:
        add_data(session.add_datas(), eType, m_oDeposit.hasMonthCard() ? 1 : 0);
        break;
      case EUSERDATATYPE_COOKER_EXP:
        add_data(sync.add_datas(), eType, m_oSceneFood.getCookerExp());
        break;
      case EUSERDATATYPE_COOKER_LV:
        add_data(sync.add_datas(), eType, m_oSceneFood.getCookerLv());
        break;
      case EUSERDATATYPE_TASTER_EXP:
        add_data(sync.add_datas(), eType, m_oSceneFood.getTasterExp());
        break;
      case EUSERDATATYPE_TASTER_LV:
        add_data(sync.add_datas(), eType, m_oSceneFood.getTasterLv());
        break;
      case EUSERDATATYPE_SATIETY:
        add_data(sync.add_datas(), eType, m_oSceneFood.getEatedFoodNum());
        break;
      case EUSERDATATYPE_OPTION:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getOptionSet());
        break; 
      case EUSERDATATYPE_TUTOR_PROFIC:
        add_data(sync.add_datas(), eType, m_oTutorTask.getProficiency());
        add_data(session.add_datas(), eType, m_oTutorTask.getProficiency());
        break;
      case EUSERDATATYPE_TUTOR_ENABLE:
        add_data(session.add_datas(), eType, m_oTutorTask.canBeTutor());
        break;
      case EUSERDATATYPE_PEAK_EFFECT:
        add_data(sync.add_datas(), eType, getPeakEffect());
        add_data(nine.add_datas(), eType, getPeakEffect());
        break;
      case EUSERDATATYPE_CUR_MAXJOB:
        add_data(sync.add_datas(), eType, m_pCurFighter->getMaxJobLv());
        break;
      case EUSERDATATYPE_DATA:
        break;
      case EUSERDATATYPE_JOY:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getTotalJoy());
        break;
      case EUSERDATATYPE_MUSIC_LOOP:
        add_data(sync.add_datas(), eType, m_bLoop);
        break;
      case EUSERDATATYPE_MARITAL:
        add_data(sync.add_datas(), eType, m_oUserWedding.getMaritalState());
        break;
      case EUSERDATATYPE_DIVORCE_ROLLERCOASTER:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDivorceRollerCoaster());
        break;
      case EUSERDATATYPE_TWINS_ACTIONID:
        add_data(sync.add_datas(), eType, m_dwTwinsActionID);
        add_data(nine.add_datas(), eType, m_dwTwinsActionID);
        break;
      case EUSERDATATYPE_EQUIPED_WEAPON:
        add_data(nine.add_datas(), eType, getEquipedWeapon());
        break;
      case EUSERDATATYPE_FAVORABILITY:
        add_data(sync.add_datas(), eType, m_oVar.getAccVarValue(EACCVARTYPE_FAVORABILITY));
        break;
      case EUSERDATATYPE_SERVANTID:
        add_data(sync.add_datas(), eType, m_oServant.getServantID());
        break;
      case EUSERDATATYPE_BOOTH_SCORE:
        add_data(sync.add_datas(), eType, m_oBooth.getScore());
        break;
      case EUSERDATATYPE_DRESSUP:
        add_data(sync.add_datas(), eType, m_oDressUp.getDressUpStatus());
        add_data(nine.add_datas(), eType, m_oDressUp.getDressUpStatus());
        break;
      case EUSERDATATYPE_ENSEMBLESKILL:
        add_data(session.add_datas(), eType, 0, getEnsembleSkill());
        break;
      case EUSERDATATYPE_DEADCOIN:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDeadCoin());
        break;
      case EUSERDATATYPE_DEADLV:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDeadLv());
        break;
      case EUSERDATATYPE_DEADEXP:
        add_data(sync.add_datas(), eType, m_oUserSceneData.getDeadExp());
        break;
      case EUSERDATATYPE_MAX:
        break;
    }
  }
}

void SceneUser::refreshDataToRecord(UnregType eType)
{
  SceneUserDataSave *pData = NEW SceneUserDataSave();
  UserDataRecordCmd &cmd = pData->m_oUserDataRecordCmd;

  cmd.set_charid(id);
  cmd.set_accid(accid);
  cmd.set_unregtype(static_cast<DWORD>(eType));

  // collect data
  RecordUserData &data = pData->getRecordUserData();
  m_oUserSceneData.toBaseData(*data.mutable_base());

  // base
  RedisUserData* pRedis = data.mutable_redis();
  pRedis->set_portrait(m_oPortrait.getCurPortrait(true));
  pRedis->set_clothcolor(m_oUserSceneData.getClothColor(true));
  pRedis->set_manuallv(m_oManual.getManualLv());
  pRedis->set_manualexp(m_oManual.getManualPoint());
  pRedis->set_querytype(m_oUserSceneData.getQueryType());
  pRedis->set_profic(m_oTutorTask.getProficiency());
  pRedis->set_blink(m_oUserSceneData.getBlink());
  pRedis->set_canbetutor(m_oTutorTask.canBeTutor());

  // blob
  BlobData &oData = pData->m_oBlobData;
  BlobAccData &oAccData = pData->m_oBlobAccData;

  m_oUserSceneData.save(oAccData, oData);
  save(oData);
  m_stage.save(oData.mutable_stage());
  m_oQuestNpc.save(oAccData.mutable_questnpc(), oData.mutable_questnpc());
  m_oBuff.save(oData.mutable_buffer());
  m_oPortrait.save(oAccData.mutable_portrait(), oData.mutable_portrait());
  m_oHair.save(oData.mutable_hair());
  m_oGear.save(oData.mutable_gear());
  m_oTip.save(oData.mutable_tip());
  m_oFollower.save(oData.mutable_follower());
  m_oCDTime.save(oData.mutable_cd());
  m_oShortcut.save(oData.mutable_shortcut());
  m_oFreyja.save(oData.mutable_freyja());
  m_oLaboratory.save(oData.mutable_lab());
  m_oScenery.save(oAccData.mutable_scenery());
  m_oCarrier.save(oData.mutable_carrier());
  getMenu().save(oAccData.mutable_menu(), oData.mutable_menu());
  getSceneShop().save(oData.mutable_shop());
  m_oInter.save(oData.mutable_iter());
  m_oHands.save(oData.mutable_hand());
  //optional BlobOption opt = 25;
  m_oDojo.save(oData.mutable_dojo());
  m_oPackage.save(oAccData.mutable_pack(), oData.mutable_pack());
  m_oQuest.save(oAccData.mutable_quest(), oData.mutable_quest());
  m_oQuest.saveActivityQuest(oData.mutable_activityquest());
  m_oTitle.save(oAccData.mutable_title(), oData.mutable_title());
  m_oSpEffect.save(oData.mutable_speffect());
  m_oPet.save(oData.mutable_pet());
  m_oVar.save(oData.mutable_var());
  m_oMsg.save(oData.mutable_chatmsg());
  m_oTower.save(oData.mutable_tower());
  m_oSeal.save(oData.mutable_seal());
  m_oManual.save(oAccData.mutable_manual(), oData.mutable_manual(), oAccData.mutable_photo());
  m_oHandNpc.save(oData.mutable_handnpc());
  m_oZone.save(oData.mutable_zone());
  m_oCamera.save(oData.mutable_camera());
  m_oDeposit.save(oData.mutable_deposit());
  m_oUserChat.save(oData.mutable_chat());
  m_oWeaponPet.save(oData.mutable_weaponpet());
  m_oUserStat.save(oData.mutable_statvar());
  if (m_pAttribute != nullptr)
    m_pAttribute->toBlobAttr(oData.mutable_attr());
  m_oTicket.save(oData.mutable_ticket());
  m_oShare.save(oData.mutable_share());
  m_oAchieve.save(oAccData.mutable_achieve(), oData.mutable_achieve());
  m_oAstrolabes.save(oData.mutable_astrolabe());
  m_oPhoto.save(oData.mutable_photo());
  m_oUserPet.save(oData.mutable_userpet());
  m_oPetAdventure.save(oData.mutable_petadventure());
  m_oSceneFood.save(oAccData.mutable_food(), oData.mutable_food());
  m_oTutorTask.save(oData.mutable_tutortask());
  m_oEye.save(oData.mutable_eye());
  m_oUserBeing.save(oData.mutable_being());
  m_oHighRefine.save(oData.mutable_highrefine());
  m_oGuildChallenge.save(oData.mutable_gchallenge());
  m_oUserGvg.save(oData.mutable_gvgdata());
  m_oServant.save(oAccData.mutable_servant(), oData.mutable_servant());
  m_oProfession.save(oData.mutable_profession());
  m_oBooth.save(oData.mutable_booth());
  m_oTransfer.save(oData.mutable_transfer());
  getExchangeShop().save(oData.mutable_exchangeshop());
  m_oUserElementElf.save(oData.mutable_element_elf());

  // common
  m_oUserSceneData.saveActivity(oAccData.mutable_acevent(), oData.mutable_acevent());
  m_oUserSceneData.saveCredit(oAccData.mutable_credit());
  m_oUserSceneData.saveAccUser(oAccData.mutable_user());
  m_oUserSceneData.saveBoss(oData.mutable_boss());
  getSceneShop().saveAcc(oAccData.mutable_shop());
  m_oVar.saveAcc(oAccData.mutable_var());
  getPetWork().save(oAccData.mutable_petwork());
  getUserRecords().saveAcc(oAccData.mutable_record());
  m_oLottery.saveAcc(oAccData.mutable_lottery());

  SceneFighter* pNoviceFighter = getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter)
    pNoviceFighter->getSkill().saveAcc(oAccData.mutable_skill());

  SceneUserSaveThread::getMe().add(pData);
}

void SceneUser::enterServer()
{
  // 通知client,客户端创建角色，在进入场景之前发
  sendMainUserData();
  // 通知Session
  sendSessionUserData();
  // 通知Gate
  sendGateUserData();

  if (getUserState() == USER_STATE_LOGIN)
  {
    m_oShare.addNormalData(ESHAREDATATYPE_S_LOGINCOUNT, 1);
    m_oUserPet.playAction(EPETACTION_OWNER_LOGIN);
  }

  setUserState(USER_STATE_RUN);
  notifyChangeScene();
  
  //检测老玩家的冒险称号
  if (m_oTitle.hasTitle(1003))
  {
    if (m_oUserSceneData.addFirstActionDone(EFIRSTACTION_FOOD_MAIL, false))
    {
      MailManager::getMe().sendMail(id, 10008);
    }
  }

  XLOG << "[玩家-登录]" << accid << id << getProfession() << name << "读档成功，完成登录" << XEND;
}

void SceneUser::sendMainUserData()
{
  if (getUserState() == USER_STATE_CHANGE_SCENE)
    return;

  // var data
  VarUpdate cmd;
  m_oVar.collectVar(cmd, true);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  // menu data
  getMenu().sendAllMenu();
  // portrait data
  m_oPortrait.sendAllUnlockItems();
  // hair data
  m_oHair.sendAllUnlockHairs();
  // tower data
  m_oTower.sendTowerData();
  // interlocution data
  m_oInter.sendInterlocution();
  // manual data
  m_oManual.sendManualData();
  m_oManual.addLotteryFashion();
  // freyja
  m_oFreyja.sendList();
  // fighter
  sendFighterInfo();
  // send map area
  m_oUserSceneData.sendMapAreaList();
  // send action action
  m_oUserSceneData.sendAllActions();
  // item trace
  m_oUserSceneData.sendTraceItemList();
  // first action
  m_oUserSceneData.sendFirstAction();
  //title data
  m_oTitle.sendTitleData();
  //deposit data
  m_oDeposit.sendDataToClient();
  m_oDeposit.onLine();
  // achieve data
  m_oAchieve.sendAchieveData();
  // upyun data
  sendUpyunUrl();
  sendUpyunAuthorization();
  // scenery data
  m_oScenery.sendunsolved();
  // photo data
  m_oPhoto.sendPhotoList();
  // food data
  m_oSceneFood.sendFoodData();
  // eye data
  m_oEye.sendUnlockList();
  //setGChar();
  //highrefine
  m_oHighRefine.sendHighRefineData();
  // tutor data
  m_oTutorTask.sendTaskData();
  m_oUserBeing.sendData();
  // equip pos data
  m_oPackage.sendEquipPosData();
  // md5 list
  m_oUserSceneData.queryPhotoMd5List();
  // pet work
  getPetWork().queryWorkManual();
  // servant
  m_oServant.sendAllRecommendInfo();
  m_oServant.sendAllGrowthInfo();
  // user records
  getUserRecords().sendRecordsData();
  // profession
  m_oProfession.sendBranchData();
  // transfer
  m_oTransfer.sendList();
  // exchange shop
  getExchangeShop().sendExchangeShopItems();

  m_bitset.set();
  updateData(UnregType::Null, EUSERSYNCTYPE_INIT);

  // 职业等数据推送后再发送
  // astrolabe data
  m_oAstrolabes.sendAstrolabeData();
  m_oTutorTask.sendGrowRewardData();
  // skilldata
  if (m_pCurFighter != nullptr)
    m_pCurFighter->getSkill().sendData();
  else
    XERR << "[玩家-数据同步]" << accid << id << getProfession() << name << "同步技能数据失败" << XEND;

  onAltmanEnd();
}

void SceneUser::notifyChangeScene()
{
  Scene *pScene = SceneManager::getMe().getSceneByID(m_oUserSceneData.getOnlineMapID());
  if (!pScene)
  {
    XERR << "[玩家-场景切换通知]" << accid << id << getProfession() << name << "onlinemapid:" << m_oUserSceneData.getOnlineMapID() << "未在此场景服务器中" << XEND;
    return;
  }

  xPos pos = m_oUserSceneData.getOnlinePos();
  if (pScene->isMatchScene())
  {
    MatchScene* pMScene = dynamic_cast<MatchScene*>(pScene);
    if (pMScene)
    {
      pMScene->getBornPos(this, pos);
    }
    m_oUserSceneData.setOnlineMapPos(pScene->id, pos);
  }
  else if (pos.empty())
  {
    if (pScene->isTowerScene())
    {
      TowerScene* pTScene = dynamic_cast<TowerScene*> (pScene);
      if (pTScene)
        pTScene->getBornPos(pos);

      if (pScene->isValidPos(pos) == false)
        pos.clear();
      else
        m_oUserSceneData.setOnlineMapPos(pScene->id, pos);
    }
    if (pos.empty())
    {
      const SceneObject *pObject = pScene->getSceneObject();
      if (pObject)
      {
        const map<DWORD, SBornPoint>& mapList = pObject->getBornPointList();
        for (auto &m : mapList)
        {
          pos = m.second.oPos;
          if (m.second.dwRange)
            pScene->getRandPos(m.second.oPos, m.second.dwRange, pos);

          m_oUserSceneData.setOnlineMapPos(pScene->id, pos);
          break;
        }
      }
    }
  }
  else
  {
    pos.clear();
    if (pScene->getValidPos(m_oUserSceneData.getOnlinePos(), pos))
    {
      m_oUserSceneData.setOnlineMapPos(pScene->id, pos);
    }
    if (pos.empty())
    {
      const SceneObject *pObject = pScene->getSceneObject();
      if (pObject)
      {
        const map<DWORD, SBornPoint>& mapList = pObject->getBornPointList();
        for (auto &m : mapList)
        {
          pos = m.second.oPos;
          if (m.second.dwRange)
            pScene->getRandPos(m.second.oPos, m.second.dwRange, pos);

          m_oUserSceneData.setOnlineMapPos(pScene->id, pos);
          break;
        }
      }
    }
  }

  bool blNotifyClient = true;

  DWORD dmapid = 0;
  DScene* pDScene = dynamic_cast<DScene*>(pScene);
  if (pDScene != nullptr)
    dmapid = pDScene->getRaidID();

  // 先发送地图切换消息:防止队伍跟随队员延迟
  UserDataSync session;
  session.set_id(id);

  UserData* pData = session.add_datas();
  if (pData != nullptr)
  {
    pData->set_type(EUSERDATATYPE_MAPID);
    pData->set_value(pScene->getMapID());
  }
  pData = session.add_datas();
  if (pData != nullptr)
  {
    pData->set_type(EUSERDATATYPE_RAIDID);
    pData->set_value(dmapid);
  }
  pData = session.add_datas();
  if (pData != nullptr)
  {
    pData->set_type(EUSERDATATYPE_GUILDRAIDINDEX);
    DWORD value = 0;
    if (pScene && pScene->getSceneType() == SCENE_TYPE_GUILD_RAID)
    {
      GuildRaidScene* pGRScene = dynamic_cast<GuildRaidScene*> (pScene);
      if (pGRScene) value = pGRScene->getMapIndex();
    }
    pData->set_value(value);
  }
  PROTOBUF(session, ssend, slen);
  thisServer->sendCmdToSession(ssend, slen);
  syncMyPosToTeam(true);

  Cmd::ChangeSceneUserCmd cmd;
  if (SceneImage::isImages(pScene->getMapID(), pScene->id, m_oUserSceneData.getLastRealMapID(), m_oUserSceneData.getLastMapID()) || pScene->m_oImages.isImageScene())
  {
    // 断线重连, 即使是镜像地图也要通知前端, 将不通知改为通知在当前地图切换
    cmd.set_imageid(dmapid);
    dmapid = 0;
    //blNotifyClient = false;
  }
  // inform client
  cmd.set_mapname(pScene->name);
  cmd.set_mapid(pScene->getMapID());
  cmd.set_dmapid(dmapid);
  Cmd::ScenePos *p = cmd.mutable_pos();
  p->set_x(m_oUserSceneData.getOnlinePos().getX());
  p->set_y(m_oUserSceneData.getOnlinePos().getY());
  p->set_z(m_oUserSceneData.getOnlinePos().getZ());

  // set layer if towerscene
  TowerScene* pTScene = dynamic_cast<TowerScene*>(pScene);
  if (pTScene != nullptr)
  {
    stringstream sstr;
    sstr << pTScene->getRaidCFG()->strNameZh << pTScene->getLayer();
    cmd.set_mapname(sstr.str());
  }

  const SceneObject *pObject = pScene->getSceneObject();
  if (pObject)
  {
    const map<DWORD, ExitPoint>& mapList = pObject->getExitPointList();
    for (auto it : mapList)
    {
      if (!it.second.isVisible(this, pScene))
      {
        cmd.add_invisiblexit(it.second.m_dwExitID);
        XLOG << "[切换场景] " << accid << id << getProfession() << name << " 不可见传送点:" << it.second.m_dwExitID << XEND;
      }
    }
  }

  if (m_oUserSceneData.isNewMap(pScene->getMapID()) == true)
  {
    cmd.set_preview(pScene->base->isPreview());
    m_oUserSceneData.addNewMap(pScene->getMapID());
    m_oServant.onAppearEvent(ETRIGGER_MAPID);
    m_oServant.onGrowthAppearEvent(ETRIGGER_MAPID);
    m_oServant.onGrowthFinishEvent(ETRIGGER_MAPID);
  }
  const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(dmapid);
  if (pRaidCFG != nullptr && (pRaidCFG->eRaidType == ERAIDTYPE_RAIDTEMP4 || pRaidCFG->eRaidType == ERAIDTYPE_PVP_POLLY))
    cmd.set_preview(1);

  // add map name if tower scene
  if (pDScene != nullptr && getTeamID() != 0 && pDScene->getRaidType() == ERAIDTYPE_TOWER)
  {
    TowerScene* pTScene = dynamic_cast<TowerScene*>(getScene());
    if (pTScene != nullptr)
    {
      ostringstream ostr;
      ostr << pTScene->base->getMapInfo().name;
      ostr << pTScene->getLayer();
      cmd.set_mapname(ostr.str());
    }
  }
  setDataMark(EUSERDATATYPE_RAIDID);

  if (blNotifyClient)
  {
    PROTOBUF(cmd, send, len);
    sendCmdToMe(send, len);
  }

  if (!blNotifyClient)
  {
    if (!this->enterScene(pScene))
    {
      XLOG << "[玩家-切换场景通知] " << accid << id << getProfession() << name << "进入场景失败" << pScene->name << XEND;
    }
  }

  XLOG << "[玩家-切换场景通知] " << accid << id << getProfession() << name << "通知客户端切换地图 " << pScene->name
    << ":(" << m_oUserSceneData.getOnlinePos().getX() << m_oUserSceneData.getOnlinePos().getY() << m_oUserSceneData.getOnlinePos().getZ() << ")" << XEND;
}

void SceneUser::sendSessionUserData()
{
  UserLoginNtfSessionCmd cmd;
  cmd.set_charid(id);
  cmd.set_servername(thisServer->getServerName());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

void SceneUser::sendGateUserData()
{
  UserDataGatewayCmd gSend;
  strncpy(gSend.data.sceneServerName, thisServer->getServerName(), MAX_NAMESIZE);
  gSend.accid = accid;
  gSend.data.charid = id;
  strncpy(gSend.data.name,name,MAX_NAMESIZE);
  gatetask->sendCmd(&gSend, sizeof(gSend));
}

void SceneUser::unReg(UnregType type)
{
  unRegBeforeSave();

  switch (type)
  {
    case UnregType::Kuaqu:
      {
      }
      break;
    case UnregType::ServerStop:
    case UnregType::Normal:
      {
        //m_oCarrier.offline();

        XLOG << "[玩家-坐标] " << accid << id << getProfession() << name << " 设置前(" << getPos().x << getPos().y << getPos().z << ")" << XEND;
        XLOG << "[玩家-上线坐标] " << accid << id << getProfession() << name << " 设置前(" << m_oUserSceneData.getOnlinePos().x << m_oUserSceneData.getOnlinePos().y << m_oUserSceneData.getOnlinePos().z << ")" << XEND;
        if (m_blInScene)
        {
          m_oUserSceneData.setOnlineMapPos(getScene()->id, getPos());
          m_oUserSceneData.setLastMapID(getScene()->id, getScene()->getMapID());
        }
        XLOG << "[玩家-上线坐标] " << accid << id << getProfession() << name << " 设置后(" << m_oUserSceneData.getOnlinePos().x << m_oUserSceneData.getOnlinePos().y << m_oUserSceneData.getOnlinePos().z << ")" << XEND;
        //m_oFollower.clear();
        //从九屏删除人物
        setUserState(USER_STATE_QUIT);
        onUserOffline();
      }
      break;
    case UnregType::ChangeScene:
      {
        setUserState(USER_STATE_CHANGE_SCENE);
      }
      break;
    default:
      break;
  }

  leaveScene();

  if (initCharSuccess)
  {
    refreshDataToRecord(type);
    unRegAfterSave();
  }

  XLOG << "[玩家-注销] " << accid << id << getProfession() << name << XEND;
}

void SceneUser::unRegBeforeSave()
{
  stopCalcTeamLen();    //在线组队时长
  leaveDressSatgeOnUnreg();
}

void SceneUser::unRegAfterSave()
{
  m_oFollower.clear();
}

void SceneUser::onUserOffline()
{
  DWORD n = xTime::getCurSec();
  m_oUserSceneData.setOfflineTime(n);
  m_oSeal.onUserOffline();
  if (getTeamSeal())
    getTeamSeal()->onUserOffline(this);
  m_oHands.userOffline();

  /*if (m_oUserSceneData.getZoneID() != thisServer->getZoneID())
  {
    DWORD buffid = MiscConfig::getMe().getSystemCFG().dwZoneBossLimitBuff;
    if (!hasGuild())
    {
      m_oBuff.add(buffid);
    }
    else
    {
      if (m_oUserSceneData.getZoneID() != m_oGuildInfo.zoneid())
      {
        m_oBuff.add(buffid);
      }
    }
  }*/

  //登出日志 

  DWORD onlineTime = 0; //在线时长
  DWORD teamLen = getTeamTimeLen();    //在线组队时长
  if (n > m_oUserSceneData.getOnlineTime())
  {
    onlineTime = n - m_oUserSceneData.getOnlineTime();
  }

  PlatLogManager::getMe().loginLog(thisServer,
    m_oUserSceneData.getPlatformId(),
    getZoneID(),
    accid,
    id,
    "",/*ip*/
    m_oUserSceneData.getCharge(),
    0,
    m_oUserSceneData.getRolelv(),
    m_logSign,
    ""/*device*/,
    0/*guest*/,
    ""/*mac*/,
    ""/*agent*/,
    m_oUserSceneData.getLastRealMapID(),
    onlineTime, teamLen, m_oUserSceneData.isNew()
  );
  
  //统计
  //StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_ONLINE_INFO, id, getProfession(), getLevel(), onlineTime);
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_TEAM_TIME, 0, 0, getLevel(), teamLen);
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_ONLINE_TIME, 0, 0, getLevel(), onlineTime);

  if (m_oUserStat.checkAndSet(ESTATTYPE_ONLINE_COUNT))
    m_oUserStat.sendStatLog(ESTATTYPE_ONLINE_COUNT, 0, getProfession(), getLevel(), 1);
  m_oUserStat.sendDayGetZenyCountLog(m_oUserSceneData.getDailyNormalZeny(), m_oUserSceneData.getDailyChargeZeny());
  
  if (m_pAttribute)
  {
    std::vector<EAttrType> vecAttr = { EATTRTYPE_ATK,EATTRTYPE_MATK ,EATTRTYPE_DEF,EATTRTYPE_MDEF,
      EATTRTYPE_FLEE,EATTRTYPE_CRI,EATTRTYPE_MAXHP,EATTRTYPE_MAXSP,
      EATTRTYPE_STR,EATTRTYPE_INT,EATTRTYPE_AGI,EATTRTYPE_DEX,
      EATTRTYPE_VIT,EATTRTYPE_LUK
    };

    for (auto &v : vecAttr)
    {
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MAX_ATTR, v/*key*/, getProfession(), getLevel(), m_pAttribute->getAttr(v));
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_AVG_ATTR, v/*key*/, getProfession(), getLevel(), m_pAttribute->getAttr(v));
    }
  }
  if (m_pCurFighter)
  {
    TVecPosSkill vecSkillPosInfo;
    m_pCurFighter->getSkill().getSkillPosInfo(vecSkillPosInfo);

    if (m_oUserStat.checkAndSet(ESTATTYPE_SKILL_LEARN))
    {
      for (auto &v : vecSkillPosInfo)
      {
        DWORD tskillId = (v.dwID / 100) * 1000 + v.dwSourceid;
        DWORD skllLv = v.dwID % 100;
        //StatisticsDefine::sendStatLog2(thisServer, ESTATTYPE_SKILL_LEARN, id, tskillId, v.dwPos, 0, skllLv);
        m_oUserStat.sendStatLog2(ESTATTYPE_SKILL_LEARN, 0, tskillId, v.dwPos, 0, skllLv);
      }
    }
  }

  //剩余zeny币
  if (m_oUserStat.checkAndSet(ESTATTYPE_REMAIN_ZENY_SUM))
    m_oUserStat.sendStatLog(ESTATTYPE_REMAIN_ZENY_SUM, 0, 0, getLevel(), DWORD(m_oUserSceneData.getSilver()));
  m_oUserStat.checkAndSendCurLevelToStat();
}

bool SceneUser::enterScene(Scene* scene)
{
  if (getUserZone().online(scene)) return false;
  if (m_blInScene || scene == nullptr) return false;
  if (!scene->canEnter(this))
  {
      getUserMap().gotoLastStaticMap();
      return false;
  }

  if (!scene->isValidPos(m_oUserSceneData.getOnlinePos()))
  {
    xPos pos;
    scene->getRandPos(pos);
    m_oUserSceneData.setOnlineMapPos(m_oUserSceneData.getOnlineMapID(), pos);
    if (!scene->isValidPos(m_oUserSceneData.getOnlinePos())) return false;
  }

  m_blInScene = true;
  setScene(scene);
  scene->addScope(this);

  addOneLevelIndex(ONE_LEVEL_INDEX_TYPE_MAP, getScene()->id);

  //setPos(m_onlinePos);
  m_oMove.clearPath();
  setPos(m_oUserSceneData.getOnlinePos());

  getScene()->addEntryAtPosI(this);

  sendNineToMe();
  sendMeToNine();

  getScene()->userEnter(this);
  m_event.onEnterScene();

  setOwnWeather(getScene()->getWeather());
  setOwnSky(getScene()->getSky());
  setOwnTime(0);
  scene->sendSeatToUser(this);

  {
    Cmd::MapCmdEnd message;
    PROTOBUF(message, send, len);
    this->sendCmdToMe(send, len);
  }

  bool playEffect = true;
  if (scene->isDScene() && ((DScene*)scene)->isNoPlayGoMap())
    playEffect = false;

  if (playEffect)
  {
    // 播放特效音效
    const SEffectPath& configPath = MiscConfig::getMe().getEffectPath();

    xLuaData data;
    data.setData("type", "effect");
    data.setData("effect", configPath.strEnterSceneEffect);
    data.setData("posbind", 1);
    GMCommandRuler::getMe().execute(this, data);

    xLuaData sound;
    sound.setData("type", "sound_effect");
    sound.setData("se", configPath.strEnterSceneSound);
    sound.setData("sync", 1);
    GMCommandRuler::getMe().execute(this, sound);
  }

  // to inform social server
  setDataMark(EUSERDATATYPE_QUERYTYPE);

  GuildInfoNtf cmd;
  cmd.set_charid(id);
  cmd.set_id(m_oGuild.id());
  cmd.set_name(m_oGuild.name());
  cmd.set_icon(m_oGuild.portrait());
  cmd.set_job(m_oGuild.jobname());
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  sendGuildCity();

  m_bMarkInScene = true;

  DWORD dwUserCount = 0;
  DScene* pDScene = dynamic_cast<DScene*>(scene);
  if (pDScene != nullptr)
  {
    xSceneEntrySet set;
    pDScene->getAllEntryList(SCENE_ENTRY_USER, set);
    dwUserCount = set.size();
  }
  syncMemberPosToMe();
  XLOG << "[场景玩家-进入场景]" << accid << id << getProfession() << name
    << "地图:" << getScene()->name << "pos:(" << getPos().x << getPos().y << getPos().z << ")" << "当前场景人数 :" << dwUserCount << XEND;

  m_event.onEnterSceneEnd();

  return true;
}

void SceneUser::leaveScene()
{
  if (!getScene()) return;

  m_oCarrier.leave();
  // m_oQuest.timer(0);
  ChatManager_SC::getMe().onLeaveScene(this);

  checkReliveWhenLoginout();
  updateData(UnregType::Null);

  getScene()->sendVisibleNpc(this, false);
  m_event.onLeaveScene();

  delMeToNine();
  delNineToMe();

  getScene()->delEntryAtPosI(this);
  delOneLevelIndex(ONE_LEVEL_INDEX_TYPE_MAP, getScene()->id);

  getScene()->userLeave(this);
  getScene()->destroyScope(this);
  m_blInScene = false;

  if (getScene()->isDPvpScene() || getScene()->isSuperGvg())
  {
    if (getScene()->getSceneType() != SCENE_TYPE_TEAMPWS)
      getUserSceneData().setOnlineMapPos(getUserMap().getLastStaticMapID(), getUserMap().getLastStaticMapPos());
  }

  if (isOnPvp() || isOnGvg())
  {
    Attribute* pAttr = getAttribute();
    if (pAttr != nullptr)
      pAttr->setMark();
  }

  XLOG << "[场景玩家-离开场景]" << accid << id << getProfession() << name << "地图:" << getScene()->name << "pos:(" << getPos().x << getPos().y << getPos().z << ")" << XEND;
  setScene(nullptr);
  m_bMarkInScene = false;
}

void SceneUser::sendMeToNine()
{
}

void SceneUser::sendCmdToScope(const void* cmd, DWORD len)
{
  if (!getScene()) return;

  getScene()->sendCmdToScope(this, cmd, len);
}

void SceneUser::sendMeToUser(SceneUser* user)
{
  if (!getScene()) return;

  getScene()->sendUserToUser(this, user);
}

void SceneUser::delMeToUser(SceneUser* user)
{
  if (!getScene()) return;

  getScene()->removeScope(this, user);
}

void SceneUser::sendNineNpcToMe()
{
  if (!getScene()) return;
  xPosIVector vec;
  getScene()->getNineScreen(getPos(), vec);
  for (std::vector<xPosI>::iterator it=vec.begin();it!=vec.end();it++)
  {
    getScene()->sendNpcEntry(this, *it);
  }
}

void SceneUser::sendNineToMe()
{
  beforeSendNine();

  if (!getScene()) return;

  {
    xSceneEntrySet uSet;
    getScene()->getEntryListInNine(SCENE_ENTRY_USER, getPos(), uSet);
    if (!uSet.empty())
    {
      for (auto &it : uSet)
      {
        SceneUser *user = (SceneUser *)(it);
        getScene()->addScope(this, user);
      }
      getScene()->refreshScope(this);
    }
  }
  sendNineNpcToMe();
  {
    Cmd::AddMapItem cmd;
    xSceneEntrySet uSet;
    getScene()->getEntryListInNine(SCENE_ENTRY_ITEM, getPos(), uSet);
    //if (uSet.empty()) return;

    auto it = uSet.begin(), end = uSet.end();
    for (; it != end; ++it)
    {
      SceneItem* pItem = dynamic_cast<SceneItem*>(*it);
      if (pItem == nullptr)
        continue;

      if (pItem->viewByUser(this->id) == false)
        continue;
      pItem->fillMapItemData(cmd.add_items());
    }
    if (cmd.items_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
    }
  }
  {
    Cmd::AddMapTrap cmd;
    xSceneEntrySet uSet;
    getScene()->getEntryListInNine(SCENE_ENTRY_TRAP, getPos(), uSet);
    //if (uSet.empty()) return;

    auto it = uSet.begin(), end = uSet.end();
    for (; it != end; ++it)
    {
      SceneTrap* pTrap = dynamic_cast<SceneTrap *>(*it);
      if (pTrap == nullptr)
        continue;

      SceneUser* user = pTrap->getScreenUser();
      if (user && !getScene()->inScope(this, user))
      {
        continue;
      }
      pTrap->fillMapTrapData(cmd.add_traps());
    }
    if (cmd.traps_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
    }
  }
  {
    Cmd::AddMapUser cmd;
    xSceneEntrySet uSet;
    getScene()->getEntryListInNine(SCENE_ENTRY_BOOTH, getPos(), uSet);

    auto it = uSet.begin(), end = uSet.end();
    for (; it != end; ++it)
    {
      SceneBooth* booth = dynamic_cast<SceneBooth*>(*it);
      if (!booth)
        continue;

      booth->fillData(cmd.add_users());
    }
    if (cmd.users_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
    }
  }

  {
    Cmd::AddMapAct cmd;
    xSceneEntrySet uSet;
    getScene()->getEntryListInNine(SCENE_ENTRY_ACT, getPos(), uSet);
    //if (uSet.empty()) return;

    for (auto it = uSet.begin(); it != uSet.end(); ++it)
    {
      SceneActBase* pAct = dynamic_cast<SceneActBase*> (*it);
      if (pAct == nullptr)
        continue;
      if (pAct->viewByUser(this->id) == false)
        continue;
      
      if (pAct->getActType() == EACTTYPE_EFFECT)
      {
        SceneActEffect* pEffect = dynamic_cast<SceneActEffect*>(pAct);
        if (pEffect)
        {
          pEffect->sendMeToUser(this);
        }
      }
      else
      {
        if (pAct->getActType() == EACTTYPE_SCENEEVENT)
        {
          SceneActEvent* pEvent = dynamic_cast<SceneActEvent*>(pAct);
          if (pEvent && pEvent->checkUserInRange(this))
            pEvent->onUserIn(this);
        }
        pAct->fillMapActData(cmd.add_acts());
      }
    }
    if (cmd.acts_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
    }
  }
}

void SceneUser::delMeToNine()
{
}

void SceneUser::delNineToMe()
{
  if (!getScene()) return;

  xPosIVector vec;
  getScene()->getNineScreen(getPos(), vec);
  for (auto &it : vec)
  {
    getScene()->delDynamicEntry(this, it);
  }
  getScene()->refreshScope(this);
}

// 填充摆摊信息
void SceneUser::fillMapBoothData(MapUser* data)
{
  if (data == nullptr)
    return;

  data->set_guid(id);
  data->set_name(name);
  data->set_gender(m_oUserSceneData.getGender());

  Cmd::ScenePos *p = data->mutable_pos();
  p->set_x(getPos().getX());
  p->set_y(getPos().getY());
  p->set_z(getPos().getZ());
  xPos destPos = m_oMove.getStraightPoint();
  if (!destPos.empty())
  {
    Cmd::ScenePos *dest = data->mutable_dest();
    dest->set_x(destPos.getX());
    dest->set_y(destPos.getY());
    dest->set_z(destPos.getZ());
  }

  // attr
  auto addattr = [&](EAttrType eType, float fValue)
  {
    const RoleData* pData = RoleDataConfig::getMe().getRoleData(eType);
    if (pData == nullptr || !pData->bSync)
      return;

    UserAttr* pAttr = data->add_attrs();
    if (pAttr == nullptr)
      return;

    pAttr->set_type(eType);
    if (pData->bPercent)
      pAttr->set_value(fValue * FLOAT_TO_DWORD);
    else
      pAttr->set_value(fValue);
  };

  // 同步hp&sp
  static const vector<EAttrType> vecTypes = { EATTRTYPE_HP, EATTRTYPE_MAXHP, EATTRTYPE_SP, EATTRTYPE_MAXSP };
  for (auto &v : vecTypes)
  {
    float value = getAttr(v);
    if (value != 0)
      addattr(v, value);
  }

  // data
  add_data(data->add_datas(), EUSERDATATYPE_ROLELEVEL, m_oUserSceneData.getRolelv());
  add_data(data->add_datas(), EUSERDATATYPE_SEX, m_oUserSceneData.getGender());
  add_data(data->add_datas(), EUSERDATATYPE_BODY, m_oUserSceneData.getBody());
  add_data(data->add_datas(), EUSERDATATYPE_BODYSCALE, m_oUserSceneData.getBodyScale());
  add_data(data->add_datas(), EUSERDATATYPE_HAIR, m_oHair.getCurHair());
  add_data(data->add_datas(), EUSERDATATYPE_HAIRCOLOR, m_oHair.getCurHairColor());
  add_data(data->add_datas(), EUSERDATATYPE_CLOTHCOLOR, m_oUserSceneData.getClothColor());
  add_data(data->add_datas(), EUSERDATATYPE_LEFTHAND, m_oUserSceneData.getLefthand());
  add_data(data->add_datas(), EUSERDATATYPE_RIGHTHAND, m_oUserSceneData.getRighthand());
  add_data(data->add_datas(), EUSERDATATYPE_HEAD, m_oUserSceneData.getHead());
  add_data(data->add_datas(), EUSERDATATYPE_BACK, m_oUserSceneData.getBack());
  add_data(data->add_datas(), EUSERDATATYPE_FACE, m_oUserSceneData.getFace());
  add_data(data->add_datas(), EUSERDATATYPE_TAIL, m_oUserSceneData.getTail());
  add_data(data->add_datas(), EUSERDATATYPE_PROFESSION, m_oUserSceneData.getProfession());
  add_data(data->add_datas(), EUSERDATATYPE_MOUNT, m_oUserSceneData.getMount());
  add_data(data->add_datas(), EUSERDATATYPE_NORMAL_SKILL, m_pCurFighter->getSkill().getNormalSkill());
  add_data(data->add_datas(), EUSERDATATYPE_PORTRAIT, m_oPortrait.getCurPortrait());
  add_data(data->add_datas(), EUSERDATATYPE_FRAME, m_oPortrait.getCurFrame());
  add_data(data->add_datas(), EUSERDATATYPE_PET_PARTNER, m_oPet.getPartnerID());
  add_data(data->add_datas(), EUSERDATATYPE_STATUS, getStatus());
  add_data(data->add_datas(), EUSERDATATYPE_MUSIC_DEMAND, static_cast<DWORD>(m_bDemand));
  add_data(data->add_datas(), EUSERDATATYPE_DIR, m_oUserSceneData.getDir());
  add_data(data->add_datas(), EUSERDATATYPE_SHADERCOLOR, m_oUserSceneData.getShaderColor());
  add_data(data->add_datas(), EUSERDATATYPE_EYE, m_oEye.getCurID());
  add_data(data->add_datas(), EUSERDATATYPE_MOUTH, m_oUserSceneData.getMouth());
  add_data(data->add_datas(), EUSERDATATYPE_EQUIPED_WEAPON, getEquipedWeapon());
  add_data(data->add_datas(), EUSERDATATYPE_ZONEID, getZoneID());
  if (thisServer->getZoneCategory())
    add_data(data->add_datas(), EUSERDATATYPE_PVP_COLOR, m_oZone.getColorIndex());
  else if (getScene() && (getScene()->isSuperGvg() || getScene()->getSceneType() == SCENE_TYPE_TEAMPWS))
    add_data(data->add_datas(), EUSERDATATYPE_PVP_COLOR, m_oZone.getColorIndex());
  else
    add_data(data->add_datas(), EUSERDATATYPE_PVP_COLOR, 0);
  add_data(data->add_datas(), EUSERDATATYPE_PEAK_EFFECT, getPeakEffect());

  if (m_oHands.getHandFollowID())
    add_data(data->add_datas(), EUSERDATATYPE_HANDID, m_oHands.getHandFollowID());

  if (getTwinsID())
    add_data(data->add_datas(), EUSERDATATYPE_TWINS_ACTIONID, m_dwTwinsActionID);

  if (getTeamID() != 0)
    data->set_teamid(getTeamID());

  if (m_oHands.has())
    data->set_handsmaster(m_oHands.getMasterID());

  data->set_guildid(m_oGuild.id());
  data->set_guildname(m_oGuild.name());
  data->set_guildicon(m_oGuild.portrait());
  data->set_guildjob(m_oGuild.jobname());

  if (m_oHandNpc.haveHandNpc())
  {
    if (getScene() && getScene()->getBaseCFG() && getScene()->getBaseCFG()->noHandNpc() == false)
    {
      HandNpcData* pHandData = data->mutable_handnpc();
      m_oHandNpc.getHandNpcData(pHandData);
    }
  }
  data->set_motionactionid(getAction());
  data->set_seatid(m_seatId);

  //title
  data->set_achievetitle(m_oTitle.getCurTitle(ETITLE_TYPE_ACHIEVEMENT));
  //booth
  m_oBooth.toBoothData(data->mutable_info());
}

void SceneUser::fillMapUserData(MapUser* data, bool bSendSelf/*=false*/)
{
  if (data == nullptr)
    return;

  data->set_guid(id);
  data->set_name(name);

  data->set_gender(m_oUserSceneData.getGender());

  /*string str;
  data->SerializeToString(&str);
  XLOG("[玩家-九屏添加-other] %llu, %llu, %u, %s 添加 size : %d serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), str.size());*/

  Cmd::ScenePos *p = data->mutable_pos();
  p->set_x(getPos().getX());
  p->set_y(getPos().getY());
  p->set_z(getPos().getZ());
  xPos destPos = m_oMove.getStraightPoint();
  if (!destPos.empty())
  {
    Cmd::ScenePos *dest = data->mutable_dest();
    dest->set_x(destPos.getX());
    dest->set_y(destPos.getY());
    dest->set_z(destPos.getZ());
  }

  // attr
  auto addattr = [&](EAttrType eType, float fValue)
  {
    const RoleData* pData = RoleDataConfig::getMe().getRoleData(eType);
    if (pData == nullptr || !pData->bSync)
      return;

    UserAttr* pAttr = data->add_attrs();
    if (pAttr == nullptr)
      return;

    pAttr->set_type(eType);
    if (pData->bPercent)
      pAttr->set_value(fValue * FLOAT_TO_DWORD);
    else
      pAttr->set_value(fValue);
  };
  bool bUserCanFire = getScene() && getScene()->isUserCanFireScene();
  if (bUserCanFire)
  {
    /*str.clear();
    data->SerializeToString(&str);
    XLOG("[玩家-九屏添加-位置] %llu, %llu, %u, %s 添加 size : %d serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), str.size());
    DWORD count = 0;*/
    for (auto &i : AttributeValidEnum::get())
    {
      EAttrType eType = static_cast<EAttrType>(i);
      float value = getAttr(eType);
      if (value == 0.0f)
        continue;

      addattr(eType, value);

      //++count;
      //XLOG("[玩家-九屏添加-属性item] %llu, %llu, %u, %s 添加 type : %u value : %u size : %d ", accid, id, getProfession(), name, pAttr->type(), pAttr->value(), pAttr->ByteSize());
    }

    //str.clear();
    //data->SerializeToString(&str);
    //XLOG("[玩家-九屏添加-属性] %llu, %llu, %u, %s 添加 size : %d synccount : %u serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), count, str.size());
  }
  else
  {
    static const vector<EAttrType> vecTypes = { EATTRTYPE_HP, EATTRTYPE_MAXHP, EATTRTYPE_SP, EATTRTYPE_MAXSP, EATTRTYPE_TRANSFORMID, EATTRTYPE_HIDE, EATTRTYPE_MOVESPD, EATTRTYPE_DEFATTR,
      EATTRTYPE_NOACT, EATTRTYPE_FREEZE, EATTRTYPE_NOSTIFF, EATTRTYPE_NOEFFECTMOVE, EATTRTYPE_ATKSPD, EATTRTYPE_BEHEALENCPER, EATTRTYPE_SOLO, EATTRTYPE_ENSEMBLE,
    };
    for (auto &v : vecTypes)
    {
      float value = getAttr(v);
      if (value != 0)
        addattr(v, value);
    }
  }

  // data
  add_data(data->add_datas(), EUSERDATATYPE_ROLELEVEL, m_oUserSceneData.getRolelv());
  add_data(data->add_datas(), EUSERDATATYPE_SEX, m_oUserSceneData.getGender());
  add_data(data->add_datas(), EUSERDATATYPE_BODY, m_oUserSceneData.getBody());
  add_data(data->add_datas(), EUSERDATATYPE_BODYSCALE, m_oUserSceneData.getBodyScale());
  add_data(data->add_datas(), EUSERDATATYPE_HAIR, m_oHair.getCurHair());
  add_data(data->add_datas(), EUSERDATATYPE_HAIRCOLOR, m_oHair.getCurHairColor());
  add_data(data->add_datas(), EUSERDATATYPE_CLOTHCOLOR, m_oUserSceneData.getClothColor());
  add_data(data->add_datas(), EUSERDATATYPE_LEFTHAND, m_oUserSceneData.getLefthand());
  add_data(data->add_datas(), EUSERDATATYPE_RIGHTHAND, m_oUserSceneData.getRighthand());
  add_data(data->add_datas(), EUSERDATATYPE_HEAD, m_oUserSceneData.getHead());
  add_data(data->add_datas(), EUSERDATATYPE_BACK, m_oUserSceneData.getBack());
  add_data(data->add_datas(), EUSERDATATYPE_FACE, m_oUserSceneData.getFace());
  add_data(data->add_datas(), EUSERDATATYPE_TAIL, m_oUserSceneData.getTail());
  add_data(data->add_datas(), EUSERDATATYPE_PROFESSION, m_oUserSceneData.getProfession());
  add_data(data->add_datas(), EUSERDATATYPE_MOUNT, m_oUserSceneData.getMount());
  add_data(data->add_datas(), EUSERDATATYPE_NORMAL_SKILL, m_pCurFighter->getSkill().getNormalSkill());
  add_data(data->add_datas(), EUSERDATATYPE_PORTRAIT, m_oPortrait.getCurPortrait());
  add_data(data->add_datas(), EUSERDATATYPE_FRAME, m_oPortrait.getCurFrame());
  add_data(data->add_datas(), EUSERDATATYPE_PET_PARTNER, m_oPet.getPartnerID());
  add_data(data->add_datas(), EUSERDATATYPE_STATUS, getStatus());
  add_data(data->add_datas(), EUSERDATATYPE_MUSIC_DEMAND, static_cast<DWORD>(m_bDemand));
  add_data(data->add_datas(), EUSERDATATYPE_DIR, m_oUserSceneData.getDir());
  add_data(data->add_datas(), EUSERDATATYPE_SHADERCOLOR, m_oUserSceneData.getShaderColor());
  add_data(data->add_datas(), EUSERDATATYPE_EYE, m_oEye.getCurID());
  add_data(data->add_datas(), EUSERDATATYPE_MOUTH, m_oUserSceneData.getMouth());
  add_data(data->add_datas(), EUSERDATATYPE_EQUIPED_WEAPON, getEquipedWeapon());
  add_data(data->add_datas(), EUSERDATATYPE_DRESSUP, m_oDressUp.getDressUpStatus());
  if (thisServer->getZoneCategory())
    add_data(data->add_datas(), EUSERDATATYPE_PVP_COLOR, m_oZone.getColorIndex());
  else if (getScene() && (getScene()->isSuperGvg() || getScene()->getSceneType() == SCENE_TYPE_TEAMPWS))
    add_data(data->add_datas(), EUSERDATATYPE_PVP_COLOR, m_oZone.getColorIndex());
  else
    add_data(data->add_datas(), EUSERDATATYPE_PVP_COLOR, 0);
  add_data(data->add_datas(), EUSERDATATYPE_PEAK_EFFECT, getPeakEffect());

  if (m_oHands.getHandFollowID())
    add_data(data->add_datas(), EUSERDATATYPE_HANDID, m_oHands.getHandFollowID());

  if (getTwinsID())
    add_data(data->add_datas(), EUSERDATATYPE_TWINS_ACTIONID, m_dwTwinsActionID);
  /*str.clear();
  data->SerializeToString(&str);
  XLOG("[玩家-九屏添加-数据] %llu, %llu, %u, %s 添加 size : %d serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), str.size());*/
  auto buff = [&data, this, bSendSelf](SBufferData r)
  {
    /*9屏同步仅同步部分buff*/
    if (!bSendSelf && !r.bSyncNine)
      return;

    BufferData* pData = data->add_buffs();
    if (pData == nullptr)
      return;

    pData->set_id(r.id);
    pData->set_time(r.endTime);
    pData->set_layer(r.layers);
    if (!r.activeFlag)
    pData->set_active(r.activeFlag);
    if (!r.strFromName.empty())
      pData->set_fromname(r.strFromName);
    if (r.fromID != this->id)
      pData->set_fromid(r.fromID);
    if (r.lv != 0)
      pData->set_level(r.lv % 100);
  };
  m_oBuff.foreach(buff);

  /*str.clear();
  data->SerializeToString(&str);
  XLOG("[玩家-九屏添加-buff] %llu, %llu, %u, %s 添加 size : %d serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), str.size());*/
  if (getTeamID() != 0)
    data->set_teamid(getTeamID());

  if (m_oCarrier.m_oData.m_dwCarrierID)
  {
    Cmd::CarrierInfo *info = data->mutable_carrier();
    info->set_id(m_oCarrier.m_oData.m_dwCarrierID);
    info->set_masterid(m_oCarrier.m_oData.m_qwMasterID);
    info->set_index(m_oCarrier.m_dwIndex);
    info->set_progress(m_oCarrier.m_dwProgress);
    info->set_line(m_oCarrier.m_oData.m_dwLine);
    info->set_assemble(m_oCarrier.m_oData.m_dwAssembleID);
  }

  /*str.clear();
  data->SerializeToString(&str);
  XLOG("[玩家-九屏添加-载具] %llu, %llu, %u, %s 添加 size : %d serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), str.size());*/
  if (hasChatRoom())
  {
    auto pChatRoom = getChatRoom();
    if (pChatRoom)
    {
      Cmd::ChatRoomSummary *pSummary = data->mutable_chatroom();
      if (nullptr != pSummary && id == pChatRoom->getOwnerID())
        pChatRoom->toSummary(*pSummary);
    }
  }

  /*str.clear();
  data->SerializeToString(&str);
  XLOG("[玩家-九屏添加-聊天室] %llu, %llu, %u, %s 添加 size : %d serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), str.size());*/
  if (m_oHands.has())
    data->set_handsmaster(m_oHands.getMasterID());

  // speffect
  m_oSpEffect.collectSpEffectData(data);

  data->set_guildid(m_oGuild.id());
  data->set_guildname(m_oGuild.name());
  data->set_guildicon(m_oGuild.portrait());
  data->set_guildjob(m_oGuild.jobname());

  if (m_oHandNpc.haveHandNpc())
  {
    if (getScene() && getScene()->getBaseCFG() && getScene()->getBaseCFG()->noHandNpc() == false)
    {
      HandNpcData* pHandData = data->mutable_handnpc();
      m_oHandNpc.getHandNpcData(pHandData);
    }
  }
  data->set_motionactionid(getAction());
  data->set_seatid(m_seatId);

  //title
  data->set_achievetitle(m_oTitle.getCurTitle(ETITLE_TYPE_ACHIEVEMENT));
  m_oGingerBread.collectNineData(data);  
  m_oSceneFood.mutableCookState(data->mutable_cookstate());

  //booth
  m_oBooth.toBoothData(data->mutable_info());

  /*str.clear();
  data->SerializeToString(&str);
  XLOG("[玩家-九屏添加-总共] %llu, %llu, %u, %s 添加 size : %d serialize : %llu", accid, id, getProfession(), name, data->ByteSize(), str.size());*/
}

BYTE SceneUser::getPrivilege() const
{
  BYTE pri = HUMAN_NORMAL;
//#ifdef _ALL_SUPER_GM
  return pri | HUMAN_SUPER_GM | HUMAN_GM | HUMAN_TEST | HUMAN_GM_CAPTAIN;
//#endif

  QWORD realid = id&0xffffffff;
  if (!realid) return pri;

  if(realid==1)
    pri = pri| HUMAN_SUPER_GM; else if(realid<=6)
    pri = pri|HUMAN_GM_CAPTAIN;
  else if(realid<=15)
    pri = pri| HUMAN_GM;
  return pri;
}

// 攻击相关
bool SceneUser::beAttack(QWORD damage, xSceneEntryDynamic* attacker)
{
  if (attacker == nullptr)
    return false;

  Scene* pScene = getScene();
  if (pScene == nullptr)
    return false;

  // increate fuben beattack count
  if (pScene->isDScene() == true)
  {
    setFubenBeAttackCount(getFubenBeAttackCount() + 1);

    SceneUser* pSceneAttacker = nullptr;
    if(attacker->getEntryType() == SCENE_ENTRY_USER)
      pSceneAttacker =  dynamic_cast<SceneUser*>(attacker);
    else
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*>(attacker);
      if (npc)
      {
        pSceneAttacker = npc->getMasterUser();
      }
    }


    if(pSceneAttacker != nullptr)
    {
      DScene* pDScene = dynamic_cast<DScene*>(pScene);
      if(pDScene != nullptr && pDScene->isPVPScene() == true)
      {
        pDScene->addDamageUser(attacker->id, id, damage);

      }
      if(pDScene && getStatus() == ECREATURESTATUS_DEAD)
        pDScene->onKillUser(pSceneAttacker,this);
    }
  }

  // breakup hand if dead
  if (getStatus() == ECREATURESTATUS_DEAD)
  {
    m_oHands.breakup();
    if (attacker != this)
    {
      m_strKillerName = attacker->name;
      setDataMark(EUSERDATATYPE_KILLERNAME);
    }
    XLOG << "[玩家-死亡], 玩家:" << name << id << "攻击者:" << attacker->name << attacker->id << XEND;
  }
  if (getStatus() == ECREATURESTATUS_FAKEDEAD)
  {
    setStatus(ECREATURESTATUS_LIVE);
  }

  if (attacker->getEntryType() == SCENE_ENTRY_NPC)
    addAttackMe(attacker->id, now());
  else if (attacker->getEntryType() == SCENE_ENTRY_USER)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(attacker);
    if (pUser && pUser != this && isMyEnemy(pUser))
    {
      pUser->getUserBeing().onUserAttack(this);
      pUser->getWeaponPet().onUserAttack(this);
      pUser->getUserPet().onUserAttack(this);
      pUser->getUserElementElf().onUserAttack(this);
    }
  }

  // check skill break
  m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_HP, attacker->id, static_cast<DWORD>(damage));

  // check event if attack is npc
  if (attacker->getEntryType() == SCENE_ENTRY_NPC)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(attacker);
    if (pNpc)
    {
      if (pNpc->getNpcType() == ENPCTYPE_MVP)
      {
        m_mapMvpID2Damage[pNpc->id] += damage;
        if (getStatus() == ECREATURESTATUS_DEAD)
          m_qwKillMeMvpID = pNpc->id;
      }
      else if (pNpc->define.m_oVar.m_qwNpcOwnerID)
      {
        SceneNpc* pOwner = SceneNpcManager::getMe().getNpcByTempID(pNpc->define.m_oVar.m_qwNpcOwnerID);
        if (pOwner && pOwner->getNpcType() == ENPCTYPE_MVP)
        {
          m_mapMvpID2Damage[pOwner->id] += damage;
          if (getStatus() == ECREATURESTATUS_DEAD)
            m_qwKillMeMvpID = pOwner->id;
        }
      }
    }

    if (getStatus() == ECREATURESTATUS_DEAD && pNpc)
    {
      pNpc->setTrigTalk("kill");
      pNpc->m_oEmoji.check("EnemyDie");
      pNpc->m_ai.setMyKillUser(id);
      pNpc->m_sai.checkSig("killuser");

      //platlog
      // if is 喽啰 summoned by Monster A,Monster A kill me
      SceneNpc* pKillMe = pNpc;
      if (pNpc->define.m_oVar.m_qwNpcOwnerID != 0)
      {
        SceneNpc* pOwner = SceneNpcManager::getMe().getNpcByTempID(pNpc->define.m_oVar.m_qwNpcOwnerID);
        if (pOwner != nullptr)
          pKillMe = pOwner;
      }
      if (pKillMe && pKillMe->getCFG() != nullptr)
      {
        bool isMvp = false;
        EMONSTER_TYPE type = EMONSTERTYPE_MONSTER;
        if (pKillMe->getNpcType() == ENPCTYPE_MVP) {
          if (pKillMe->define.getReborn() != 0)
          {
            isMvp = pKillMe->getNpcType() == ENPCTYPE_MVP ? true : false;
            type = EMONSTERTYPE_MVP;
          }
          else {
            type = EMONSTERTYPE_MONSTER;
          }
        }
        else if (pKillMe->getNpcType() == ENPCTYPE_MINIBOSS) {
          type = EMONSTERTYPE_MINI;
        }

        QWORD eid = xTime::getCurUSec();
        EVENT_TYPE eType = EventType_Kill;
        PlatLogManager::getMe().eventLog(thisServer,
          getUserSceneData().getPlatformId(),
          getZoneID(),
          accid,
          id,
          eid,
          getUserSceneData().getCharge(), eType, 0, 1);

        PlatLogManager::getMe().KillLog(thisServer,
          getUserSceneData().getPlatformId(),
          getZoneID(),
          accid,
          id,
          eType,
          eid,
          pKillMe->define.getID(),
          id,
          pKillMe->getCFG()->dwGroupID,
          0,
          0,
          isMvp, type, getUserSceneData().getRolelv(), 2);
      }

      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_DIE_COUNT, 0, 0, getLevel(), (DWORD)1);

      markKillByMonster();
      dropBaseExp(ESOURCE_MONSTERKILL);    
    }
  }
  
  seatUp();
  m_oSceneFood.breakCooking();
  m_oSceneFood.breakEating();

  return true;
}

// 属性
bool SceneUser::initAttr()
{
  SAFE_DELETE(m_pAttribute);
  m_pAttribute = NEW UserAttribute(this);
  if (m_pAttribute == nullptr)
    return false;

  return true;
}

SQWORD SceneUser::changeHp(SQWORD value, xSceneEntryDynamic* entry, bool bshare /*=false*/, bool bforce /*=false*/)
{
  if (entry == nullptr)
    return 0;
  SQWORD oldHp = getAttr(EATTRTYPE_HP);
  SQWORD maxHp = getAttr(EATTRTYPE_MAXHP);
  SQWORD ret = value + oldHp;
  if (ret < 0) ret = 0;

  if (value > 0)
  {
    if (!isNotCure() || bforce)
    {
      DWORD dechp = maxHp - oldHp;
      DWORD realHealHp = dechp > (DWORD)value ? value : dechp;
      onHealMe(entry, realHealHp);
    }
    else 
      ret = oldHp;
  }
  if (ret > maxHp)
    ret = maxHp;
  if (ret == oldHp)
    return oldHp - ret;

  if (ret == 0 && isNoDie())
    ret = 1;
  if (ret == 0 && m_oBuff.isUndead())
  {
    ret = 1;
    m_oBuff.onApproachDie();
  }

  setAttr(EATTRTYPE_HP, ret);
  m_pCurFighter->setHp(ret);

  if (ret == 0)
  {
    playDynamicExpression(EAVATAREXPRESSION_DEAD);
    setSp(0);
    setStatus(ECREATURESTATUS_DEAD);
    m_oBuff.setClearState();
    m_oUserPet.playAction(EPETACTION_OWNER_DEAD);
    //m_oUserSceneData.setFollowerIDNoCheck(0);
  }
  else
  {
    if (value < 0 && (oldHp - ret) > oldHp / 2)
      playDynamicExpression(EAVATAREXPRESSION_HIT);
    if (getAttr(EATTRTYPE_HP) / getAttr(EATTRTYPE_MAXHP) < 0.4f)
      m_oUserPet.playAction(EPETACTION_OWNER_HPLESS);
  }

  m_oBuff.onHpChange();
  refreshDataAtonce();

  return oldHp - ret;
}

void SceneUser::setSp(DWORD sp)
{
  if (m_pCurFighter == nullptr)
    return;

  // 无法增加sp
  if (isNoAddSp())
  {
    if (sp > m_pCurFighter->getSp())
      return;
  }

  if (sp > getAttr(EATTRTYPE_MAXSP))
    sp = getAttr(EATTRTYPE_MAXSP);

  if (sp == m_pCurFighter->getSp())
    return;

  m_pCurFighter->setSp(sp);

  m_oBuff.onSpChange();
  refreshDataAtonce();
}

void SceneUser::setStatus(ECreatureStatus eStatus)
{
  if (m_eStatus == eStatus)
    return;

  if (m_eStatus == ECREATURESTATUS_FAKEDEAD)
  {
    // 装死结束, 添加技能cd
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(10020001);
    if (pSkill)
    {
      m_oCDTime.add(pSkill->getSkillID(), pSkill->getCD(this), CD_TYPE_SKILL);
      m_oCDTime.add(pSkill->getSkillID(), pSkill->getDelayCD(this), CD_TYPE_SKILLDEALY);
    }
  }

  m_eStatus = eStatus;

  m_event.onStatusChange();

  if (eStatus == ECREATURESTATUS_DEAD)
  {
    // 增加复活cd
    if (getScene() && (getScene()->isGvg() || getScene()->getSceneType() == SCENE_TYPE_TEAMPWS))
    {
      DWORD interval = now() - m_dwLastDieTime;
      const SGuildFireCFG& cfg = MiscConfig::getMe().getGuildFireCFG();
      if (interval > cfg.dwResetCDTime)
      {
        m_dwReliveCD = 0;
      }
      else
      {
        m_dwReliveCD += cfg.getReliveCD(interval);
        m_dwReliveCD = m_dwReliveCD > cfg.dwMaxCD ? cfg.dwMaxCD : m_dwReliveCD;
      }
      if (getScene()->isSuperGvg())
      {
        SuperGvgScene* pGScene = dynamic_cast<SuperGvgScene*>(getScene());
        if (pGScene->needExpelDieUser(this))
        {
          xPos p = getPos();
          pGScene->getBornPos(this, p);
          goTo(p);
        }
      }
    }
    else if (getScene() && getScene()->getSceneType() == SCENE_TYPE_MVPBATTLE)
    {
      DWORD interval = now() - m_dwLastDieTime;
      const SMvpBattleCFG& cfg = MiscConfig::getMe().getMvpBattleCFG();
      if (interval > cfg.dwResetCDTime)
      {
        m_dwReliveCD = 0;
      }
      else
      {
        m_dwReliveCD += cfg.getReliveCD(interval);
        m_dwReliveCD = m_dwReliveCD > cfg.dwMaxCD ? cfg.dwMaxCD : m_dwReliveCD;
      }
    }
    m_dwDieTime = now();
    m_oAchieve.onDead(false, 0);
  }
  else
  {
    if (m_dwDieTime)
    {
      m_dwLastDieTime = m_dwDieTime;
      m_dwDieTime = 0;
    }
  }

  setDataMark(EUSERDATATYPE_STATUS);
  refreshDataAtonce();
}

bool SceneUser::canUseItem(DWORD itemid, QWORD targetid, bool isSkillCost /*= false*/)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(itemid);
  if (pCFG == nullptr)
    return false;
  if (ItemConfig::getMe().isUseItem(pCFG->eItemType) == false)
    return isSkillCost;

  DWORD cur = now();
  if (pCFG->dwUseEndTime && (cur < pCFG->dwUseStartTime || cur >= pCFG->dwUseEndTime))
  {
    MsgManager::sendMsg(id, 25316);
    return false;
  }

  MainPackage* pPackage = dynamic_cast<MainPackage*>(m_oPackage.getPackage(EPACKTYPE_MAIN));
  if (pPackage == nullptr)
    return false;
  if (!isSkillCost && pPackage->checkItemCount(itemid) == false)
    return false;

  if (m_oBuff.haveLimitUseItem())
  {
    if (m_oBuff.canUseItem(pCFG->eItemType) == false)
      return false;
  }

  if (!isSkillCost && (pCFG->dwCD != 0 || pCFG->dwPVPCD != 0))
  {
    if (m_oCDTime.done(CD_TYPE_ITEM, itemid) == false)
      return false;

    const TSetDWORD& setIDs = ItemConfig::getMe().getCDGroup(pCFG->dwCDGroup);
    BasePackage* pPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
    if (pPack != nullptr)
    {
      for (auto &s : setIDs)
        pPack->setCDByType(s);
    }
  }

  // check daily count
  if (pCFG->dwDaliyLimit != 0)
  {
    if (m_oPackage.getVarUseCnt(pCFG->dwTypeID) >= pCFG->dwDaliyLimit)
    {
      MsgManager::sendMsg(id, 61);
      return false;
    }
  }
  // check week count
  if (pCFG->dwWeekLimit != 0)
  {
    if (pCFG->eItemType != EITEMTYPE_MULTITIME || !m_oDeposit.hasMonthCard())
    {
      if (m_oPackage.getVarUseCnt(pCFG->dwTypeID) >= pCFG->dwWeekLimit)
      {
        MsgManager::sendMsg(id, 62);
        return false;
      }
    }
  }

  xSceneEntryDynamic* pTarget = nullptr;
  if (targetid)
  {
    pTarget = xSceneEntryDynamic::getEntryByID(targetid);
    SceneUser* user = dynamic_cast<SceneUser*> (pTarget);
    if(user && user->getDressUp().getDressUpStatus() != 0)
      return false;
  }

  int intMsgID = LuaManager::getMe().call<int>("itemUserCheck", (xSceneEntryDynamic*)(this), pCFG->dwTypeID, pTarget);
  if (intMsgID != 0)
  {
    if (intMsgID != -1)
      MsgManager::sendMsg(id, (DWORD)intMsgID);
    return false;
  }

  if (pCFG->eItemType == EITEMTYPE_FRIEND)
  {
    if(pTarget == nullptr || pTarget->getEntryType() != SCENE_ENTRY_USER)
    {
      MsgManager::sendMsg(id, 711);
      return false;
    }
    if(checkOtherRelation(targetid, ESOCIALRELATION_FRIEND) == false)
    {
      MsgManager::sendMsg(id, 25401);
      return false;
    }
  }

  // check hands
  if (pCFG->dwInHandLmt && m_oHands.has() && m_oHands.isFollower())
  {
    MsgManager::sendMsg(id, 822);
    return false;
  }
  if (pCFG->dwHandInLmt)
  {
    if (m_oHands.has() && m_oHands.isMaster())
    {
      MsgManager::sendMsg(id, 822);
      return false;
    }
    if (m_oWeaponPet.haveHandCat())
    {
      MsgManager::sendMsg(id, 822);
      return false;
    }
  }

  // check transform
  if (pCFG->dwTransformLmt && m_oTransform.isInTransform() == true)
  {
    MsgManager::sendMsg(id, 830);
    return false;
  }

  if (pCFG->dwLevel > m_oUserSceneData.getRolelv())
  {
    const SItemErrorCFG* pErrCFG = ItemConfig::getMe().getItemErrorCFG(pCFG->eItemType);
    if (pErrCFG != nullptr)
      MsgManager::sendMsg(id, pErrCFG->dwLvErrMsg, MsgParams(pCFG->dwLevel));
    return false;
  }

  if(pCFG->isRightJobForUse(getProfession()) == false)
  {
    if(itemid == 3703 )
      MsgManager::sendMsg(id, 3600);
    else if(itemid == 3704)
      MsgManager::sendMsg(id, 3601);
    else if(itemid == 3705)
      MsgManager::sendMsg(id, 3602);

    return false;
  }

  //check time capsule
  if(itemid == 5521)
  {
    if(getMenu().isOpen(EMENUID_ENDLESSTOWER) == false)
    {
      MsgManager::sendMsg(id, 3400, MsgParams(pCFG->strNameZh));
      return false;
    }
    else if(getTeamID() == 0)
    {
      MsgManager::sendMsg(id, 3407, MsgParams(pCFG->strNameZh));
      return false;
    }
    else if(m_oTower.getMaxLayer() == 0)
    {
      MsgManager::sendMsg(id, 3401, MsgParams(pCFG->strNameZh));
      return false;
    }
    else if(m_oTower.getMaxLayer() == m_oTower.getCurMaxLayer() || m_oTower.getCurMaxLayer() >= MiscConfig::getMe().getSystemCFG().dwOpenTowerLayer)
    {
      MsgManager::sendMsg(id, 3402, MsgParams(pCFG->strNameZh));
      return false;
    }
  }
  if (pCFG->dwUseLimit)
  {
    // 公会战副本不能使用
    if (pCFG->dwUseLimit & (DWORD)EUSELIMITTYPE_GVG)
    {
      if (getScene() && getScene()->isGvg())
        return false;
    }
    if (pCFG->dwUseLimit & (DWORD)EUSELIMITTYPE_ONLYGUILD)
    {
      if (getScene() && getScene()->getMapID() != MiscConfig::getMe().getGuildCFG().dwTerritory)
      {
        MsgManager::sendMsg(id, 2806);
        return false;
      }
    }
    if (pCFG->dwUseLimit & (DWORD)EUSELIMITTYPE_PVPGVG)
    {
      if (getScene() && getScene()->isPVPScene() == false && getScene()->isGvg() == false)
      {
        MsgManager::sendMsg(id, 3795);
        return false;
      }
    }
    if (pCFG->dwUseLimit & (DWORD)EUSELIMITTYPE_NOTPVECARD)
    {
      if (getScene() && getScene()->getSceneType() == SCENE_TYPE_PVECARD)
      {
        MsgManager::sendMsg(id, 3795);
        return false;
      }
    }
    if(pCFG->dwUseLimit & (DWORD)EUSELIMITTYPE_NOTMVP)
    {
      if (getScene() && getScene()->getSceneType() == SCENE_TYPE_MVPBATTLE)
      {
        MsgManager::sendMsg(id, 25522);
        return false;
      }
    }
    if(pCFG->dwUseLimit & (DWORD)EUSELIMITTYPE_TEAMMATES)
    {
      std::set<SceneUser*> setUsers = getTeamSceneUserInPvpGvg();
      if(setUsers.empty()  == true)
      {
        MsgManager::sendMsg(id, 25524);
        return false;
      }
    }
    if (pCFG->dwUseLimit & (DWORD)(EUSELIMITTYPE_NOTEAMPWS)) // 组队排位赛不能使用
    {
      if (getScene() && getScene()->getSceneType() == SCENE_TYPE_TEAMPWS)
      {
        MsgManager::sendMsg(id, 3795);
        return false;
      }
    }
  }

  return true;
}

// 包裹
bool SceneUser::useItem(const Cmd::ItemUse& cmd)
{
  if (isInPollyScene() || m_oDressUp.getDressUpStatus() != 0)
    return false;
  const std::string& guid = cmd.itemguid();

  // get package
  MainPackage* pPackage = dynamic_cast<MainPackage*>(m_oPackage.getPackage(EPACKTYPE_MAIN));
  if (pPackage == nullptr)
    return false;

  // get item
  ItemBase* pBase = pPackage->getItem(guid);
  if (pBase == nullptr)
    return false;

  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr)
    return false;

  Scene* pScene = getScene();
  if(pScene == nullptr)
    return false;

  if(pCFG->eDelType == EITEMDELTYPE_TIME && pCFG->dwDelTime != 0 && pBase->getCreateTime() + pCFG->dwDelTime <= xTime::getCurSec())
  {
    pPackage->refreshDeleteTimeItems();
    MsgManager::sendMsg(id, 3627);
    return false;
  }

  if(pCFG->eDelType == EITEMDELTYPE_DATE && pCFG->dwDelDate != 0 && pCFG->dwDelDate <= xTime::getCurSec())
  {
    pPackage->refreshDeleteTimeItems();
    MsgManager::sendMsg(id, 3627);
    return false;
  }

  if (pCFG->getUseMultiple() == 0)
  {
    if (cmd.count() > 1)
    {
      XERR << "[玩家-使用道具]" << accid << id << getProfession() << name << "失败,不可使用多个" <<"count "<<cmd.count() << XEND;
      return false;
    }
  }
  else
  {
    if (cmd.count() > pCFG->getUseMultiple())
    {
      XERR << "[玩家-使用道具]" << accid << id << getProfession() << name << "失败,个数超过使用上限" << "count " << cmd.count() << "上限个数" << pCFG->getUseMultiple() << XEND;
      return false;
    }

    if (pBase->getCount() < cmd.count())
    {
      XERR << "[玩家-使用道具]" << accid << id << getProfession() << name << "失败,个数超过服务器个数" << "count " << cmd.count() <<"服务器个数"<<pBase->getCount() << XEND;
      return false;
    }
  }

  if(checkPwd(EUNLOCKTYPE_USEITEM,pBase->getTypeID()) == false)
  {
    XERR << "[玩家-使用道具]" << accid << id << getProfession() << name << "失败,密码验证失败" << XEND;
    return false;
  }

  if (pCFG->vecEquipPro.empty() == false)
  {
    auto v = find(pCFG->vecEquipPro.begin(), pCFG->vecEquipPro.end(), getProfession());
    if (v == pCFG->vecEquipPro.end())
      return false;
  }

  QWORD tarid = cmd.targets_size() ? cmd.targets(0) : 0;
  if (canUseItem(pBase->getTypeID(), tarid) == false)
    return false;

  //使用道具打断座位
  seatUp();
  m_oSceneFood.breakCooking();
  m_oSceneFood.breakEating();
  // use item
  DWORD failCount = 0;
  DWORD count = 0;
  if (cmd.count() <= 1)
  {
    Benchmarck b =  Benchmarck("道具", 0, "使用单个道具");
    count = 1;
    if (pBase->use(this, cmd) == false)
    {
      if (pCFG->dwFailSys != 0)
        MsgManager::sendMsg(id, pCFG->dwFailSys);
      return false;
    }
  }
  else
  {
    Benchmarck b =  Benchmarck("道具", 0, "使用多个道具");

    DWORD maxcnt = cmd.count();
    if (pCFG->dwWeekLimit)
    {
      DWORD curcnt = m_oPackage.getVarUseCnt(pCFG->dwTypeID);
      maxcnt = pCFG->dwWeekLimit > curcnt ? pCFG->dwWeekLimit - curcnt : 0;
    }
    if (pCFG->dwDaliyLimit)
    {
      DWORD curcnt = m_oPackage.getVarUseCnt(pCFG->dwTypeID);
      maxcnt = pCFG->dwDaliyLimit > curcnt ? pCFG->dwDaliyLimit - curcnt : 0;
    }

    count = cmd.count();
    m_oPackage.startMultipleUse();
    DWORD i = 0;
    for (i = 0; i < count; ++i)
    {
      if (i >= maxcnt)
        break;
      if (pBase->use(this, cmd) == false)
        break;
    }

    if (i < count)
    {
      failCount = count - i;
      //sys msg
      //XX个道具未能使用成功
      MsgManager::sendMsg(id, 1280, MsgParams(failCount));
      count = i;
    }
    m_oPackage.stopMultipleUse();

    // 都使用失败
    if (count == 0)
      return false;
  }

  // check manual item
  m_oManual.onItemAdd(pBase->getTypeID(), false, true, true);

  // reduce item
  if (ItemConfig::getMe().isUseNoConsumeItem(pCFG->eItemType) == false)
  {
    if (tarid != 0)
    {
      SceneUser* pTarget = SceneUserManager::getMe().getUserByID(tarid);
      if (pTarget != nullptr)
        pTarget->getEvent().onItemBeUsed(pBase->getTypeID());
    }

    //add cd
    pBase->setCD(xTime::getCurMSec());
    if(pScene->isPVPScene() || pScene->isGvg())
      m_oCDTime.add(pBase->getTypeID(), pCFG->dwPVPCD, CD_TYPE_ITEM);
    else
      m_oCDTime.add(pBase->getTypeID(), pCFG->dwCD, CD_TYPE_ITEM);


    //*************************************************** !!!!!!!! reduceItem 可能会删除pBase, reduceItem后不可再使用pBase指针 ***************************
    pPackage->reduceItem(guid, ESOURCE_USEITEM, count);
    //*************************************************** !!!!!!!! reduceItem 可能会删除pBase, reduceItem后不可再使用pBase指针 ***************************

    // use event
    m_event.onItemUsed(pCFG->dwTypeID, count);
    // quest
    if (tarid != 0)
      m_oQuest.onItemUse(tarid, pCFG->dwTypeID);
  }

  // add use count
  if (pCFG->dwDaliyLimit != 0 || pCFG->dwWeekLimit != 0)
    m_oPackage.setVarUseCnt(pCFG->dwTypeID, m_oPackage.getVarUseCnt(pCFG->dwTypeID) + count);


  // hint
  if (pCFG->dwUsedSys != 0)
    MsgManager::sendMsg(id, pCFG->dwUsedSys);

  if (pCFG->dwTargeUsedSys != 0)
  {
    MsgParams params;
    params.addString(getName());
    MsgManager::sendMsg(tarid, pCFG->dwTargeUsedSys, params);
  }

  // do something
  LuaManager::getMe().call<int>("doAfterUsingItem", this, pCFG->dwTypeID);

  if (pCFG->dwTypeID == 50001)
    getShare().addNormalData(ESHAREDATATYPE_S_HUEDIE, 1);
  if (pCFG->dwTypeID == 5024)
    getShare().addNormalData(ESHAREDATATYPE_S_CANGYING, 1);

  XLOG << "[玩家-使用道具] " << accid << id << getProfession() << name << " 使用了 item : " << guid << " id : " << pCFG->dwTypeID << " 的物品" <<"个数" << count <<"失败个数"<< failCount << XEND;
  return true;
}

// 数据
bool SceneUser::addAttrPoint(EUserDataType eType, DWORD point)
{
  if (m_pCurFighter == nullptr)
    return false;

  if (m_pCurFighter->addAttrPoint(eType, point) == false)
    return false;

  AddAttrPoint cmd;
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  return true;
}

bool SceneUser::resetAttrPoint()
{
  if (m_pCurFighter == nullptr)
    return false;

  return m_pCurFighter->resetAttrPoint();
}

bool SceneUser::exchangeProfession(EProfession profession)
{
  // check profession valid
  if (profession <= EPROFESSION_MIN || profession >= EPROFESSION_MAX || profession == EPROFESSION_NOVICE)
  {
    XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,职位不合法" << XEND;
    return false;
  }

  // check cur profession
  if (m_oUserSceneData.getProfession() == profession)
  {
    XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,当前已是该职业" << XEND;
    return false;
  }

  // old profession
  EProfession oldProfes = m_oUserSceneData.getProfession();

  // exchange
  if (m_pCurFighter == nullptr)
  {
    XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,异常,当前没Fighter" << XEND;
    return false;
  }

  // get config
  const SRoleBaseCFG* pOldCFG = m_pCurFighter->getRoleCFG();
  const SRoleBaseCFG* pDestCFG = RoleConfig::getMe().getRoleBase(profession);
  if (pDestCFG == nullptr || pOldCFG == nullptr)
  {
    XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,未包含正确的配置" << XEND;
    return false;
  }

  // check gender
  if(!pDestCFG->checkGender(getUserSceneData().getGender()))
  {
    XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,性别不匹配" << XEND;
    return false;
  }

  // get novice
  SceneFighter* pNoviceFighter = getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter == nullptr)
  {
    XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,未包含初心者职业" << XEND;
    return false;
  }

  // check exist
  SceneFighter* pFighter = nullptr;
  if (m_pCurFighter->getProfession() == EPROFESSION_NOVICE)
  {
    const SRoleBaseCFG* pFighterCFG = m_pCurFighter->getRoleCFG();
    if (pFighterCFG == nullptr)
    {
      XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,未包含正确的配置" << XEND;
      return false;
    }

    if (m_pCurFighter->getJobLv() < pFighterCFG->maxJobLv)
    {
      XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,job等级不足,需要" << pFighterCFG->maxJobLv << XEND;
      return false;
    }
    if (pFighterCFG->canExchange(profession) == false)
    {
      XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,无法转换该职业" << XEND;
      return false;
    }

    pFighter = NEW SceneFighter(this, pDestCFG);
    if (pFighter == nullptr)
    {
      XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,创建新武将失败" << XEND;
      return false;
    }

    if (hasMonthCard())
      pFighter->getSkill().validMonthCardSkillPos();

    pFighter->setJobLv(pNoviceFighter->getJobLv() + 1);
    pFighter->getSkill().setSkillPoint(pNoviceFighter->getSkill().getSkillPoint() + 1, ESOURCE_MIN);
    pFighter->getSkill().setMaxPos(m_dwMaxSkillPos);
    pFighter->getSkill().setAutoMaxPos(m_dwAutoSkillPos);
    pFighter->getSkill().setMaxExtendPos(m_dwExtendSkillPos);
    pFighter->setProfession(pDestCFG->profession);
    pFighter->setTotalPoint(m_dwTotalAttrPoint);
    m_vecFighters.push_back(pFighter);

    m_oUserSceneData.setProfession(pDestCFG->profession);
  }
  /*else if ((pFighter = getFighter(pDestCFG->profession)) != nullptr)
  {
    m_oUserSceneData.setProfession(pDestCFG->profession);
  }
  else if ((pFighter = getFighter(pDestCFG->dwType, pDestCFG->dwTypeBranch)) != nullptr)
  {
    if (pFighter->getProfession() > profession)
      return false;

    const SRoleBaseCFG* pFighterCFG = pFighter->getRoleCFG();
    if (pFighterCFG == nullptr)
      return false;
    if (pFighter->getJobLv() < pFighterCFG->maxJobLv)
      return false;
    if (pFighterCFG->canExchange(profession) == false)
      return false;

    pFighter->setRoleCFG(pDestCFG);
    pFighter->setJobLv(pFighter->getJobLv() + 1);
    pFighter->getSkill().setSkillPoint(pFighter->getSkill().getSkillPoint() + 1, ESOURCE_MIN);
    pFighter->setProfession(pDestCFG->profession);
    m_oUserSceneData.setProfession(pDestCFG->profession);
  }*/
  else
  {
    /*pFighter = getFighter(RoleConfig::getMe().getBaseProfession(profession));
    if (pFighter == nullptr)
    {
      XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,未找到初始职业" << XEND;
      return false;
    }*/
    pFighter = m_pCurFighter;
    {
      const SRoleBaseCFG* pFighterCFG = pFighter->getRoleCFG();
      if (pFighterCFG == nullptr)
      {
        XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,未包含正确的配置" << XEND;
        return false;
      }
      if (pFighter->getJobLv() < pFighterCFG->maxJobLv)
      {
        XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,job等级不足,需要" << pFighterCFG->maxJobLv << XEND;
        return false;
      }
      if (pFighterCFG->canExchange(profession) == false)
      {
        XERR << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "失败,无法转换为该职业" << XEND;
        return false;
      }

      pFighter->setRoleCFG(pDestCFG);
      pFighter->setJobLv(pFighter->getJobLv() + 1);
      pFighter->getSkill().setSkillPoint(pFighter->getSkill().getSkillPoint() + 1, ESOURCE_MIN);
      pFighter->setProfession(pDestCFG->profession);
      m_oUserSceneData.setProfession(pDestCFG->profession);
    }
    /*else
    {
      const SRoleBaseCFG* pBaseCFG = RoleConfig::getMe().getRoleBase(RoleConfig::getMe().getBaseProfession(profession));
      if (pBaseCFG == nullptr)
        return false;
      DWORD dwCount = getFighterTypeCount(pBaseCFG->profession);
      if (dwCount >= pBaseCFG->vecAdvancePro.size())
        return false;
      pFighter = NEW SceneFighter(this, pBaseCFG);
      if (pFighter == nullptr)
        return false;

      pFighter->setJobLv(pNoviceFighter->getJobLv() + 1);
      pFighter->getSkill().setSkillPoint(pNoviceFighter->getSkill().getSkillPoint() + 1, ESOURCE_MIN);
      pFighter->getSkill().setMaxPos(m_dwMaxSkillPos);
      pFighter->getSkill().setAutoMaxPos(m_dwAutoSkillPos);
      pFighter->getSkill().setMaxExtendPos(m_dwExtendSkillPos);
      pFighter->setProfession(pBaseCFG->profession);
      pFighter->setTotalPoint(m_dwTotalAttrPoint);
      m_vecFighters.push_back(pFighter);
      m_oUserSceneData.setProfession(pBaseCFG->profession);
    }*/
  }

  // set profession
  m_pCurFighter = pFighter;

  // set maxjoblv
  setMaxJobLv(m_pCurFighter->getMaxJobLv());

  // sync maxjoblv
  m_pCurFighter->syncMaxJobLv(getMaxJobLv());

  // remove old normal skill
  if (m_pCurFighter->getSkill().getReplaceNormalSkill() != 0)
    m_pCurFighter->getSkill().restoreNormalSkill();

  m_pCurFighter->getSkill().removeSkill(pOldCFG->normalSkill, 0, ESOURCE_MIN);
  m_pCurFighter->getSkill().removeSkill(pOldCFG->strengthSkill, 0, ESOURCE_MIN);
  m_pCurFighter->getSkill().addSkill(pDestCFG->normalSkill, 0, ESOURCE_MIN);
  m_pCurFighter->getSkill().addSkill(pDestCFG->strengthSkill, 0, ESOURCE_MIN);
  //m_pCurFighter->getSkill().equipSkill(11001, 3);
  //m_pCurFighter->getSkill().equipSkill(ESKILLSHORTCUT_NORMAL, 11001, 0, 3);

  // remove old profession buff, add NEW profession buff
  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();
  const TSetDWORD& oldbuf = rCFG.getClassInitBuff(pOldCFG->profession);
  const TSetDWORD& newbuf = rCFG.getClassInitBuff(pDestCFG->profession);
  for (auto &s : oldbuf)
    m_oBuff.del(s);
  for (auto &s : newbuf)
    m_oBuff.add(s);
  
  // refresh enable skill
  m_pCurFighter->getSkill().refreshEnableSkill();
  m_pCurFighter->getSkill().clearUpdate();
  /*ReqSkillData cmd;
  m_pCurFighter->getSkill().toClient(cmd);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);*/
  m_pCurFighter->getSkill().sendSkillData();

  // inform skill pos
  /*SkillValidPos cmd1;
  m_pCurFighter->getSkill().toClient(cmd1);
  PROTOBUF(cmd1, send1, len1);
  sendCmdToMe(send1, len1);*/
  m_pCurFighter->getSkill().sendValidPos();

  // quest
  m_oQuest.acceptNewQuest();
  m_oQuest.removeInvalidQuest();

  //redtip
  if(pDestCFG->vecAdvancePro.empty() == false)
    m_oTip.addRedTip(EREDSYS_NEW_PROFESSION);
  else
  {
    m_oTip.removeRedTip(EREDSYS_NEW_PROFESSION);
    m_oTip.removeRedTip(EREDSYS_PROFESSION_UP);
  }

  // share
  /*if (dwEvo == 1)
    m_oShare.addNormalData(ESHAREDATATYPE_S_BE_PRO_1_TIME, xTime::getCurSec());
  else if (dwEvo == 2)
    m_oShare.addNormalData(ESHAREDATATYPE_S_BE_PRO_2_TIME, xTime::getCurSec());
  else if (dwEvo == 3)
    m_oShare.addNormalData(ESHAREDATATYPE_S_BE_PRO_3_TIME, xTime::getCurSec());*/

  m_oServant.onFinishEvent(ETRIGGER_CHANGE_PROFESSION);
  m_oServant.onFinishEvent(ETRIGGER_MAXJOBLEVEL);
  m_oServant.onGrowthAppearEvent(ETRIGGER_CHANGE_PROFESSION);
  m_oServant.onGrowthFinishEvent(ETRIGGER_CHANGE_PROFESSION);
  // set branch
  setBranch();

  // set exchange time
  DWORD dwEvo = profession % 10;
  m_oProfession.setExchangeTime(pDestCFG->dwTypeBranch, dwEvo, now());

  // reset attrpoint if novice
  if (oldProfes == EPROFESSION_NOVICE)
    m_pCurFighter->resetAttrPoint();

  // update attribute
  setCollectMark(ECOLLECTTYPE_BASE);
  //setCollectMark(ECOLLECTTYPE_SKILL);
  setCollectMark(ECOLLECTTYPE_EQUIP);
  //setCollectMark(ECOLLECTTYPE_CARD);
  setCollectMarkAllBuff();
  setCollectMark(ECOLLECTTYPE_PROFESSION);

  // recover hp to max
  m_pCurFighter->setHp(getAttr(EATTRTYPE_MAXHP));

  // send fighter info
  sendFighterInfo();

  // user event
  m_event.onProfesChange(oldProfes);
  getUserRecords().onJobLvChange(getJobLv());

  // exchangeshop
  getExchangeShop().resetExchangeShopItems();

  // save data
  setDataMark(EUSERDATATYPE_BODY);
  setDataMark(EUSERDATATYPE_LEFTHAND);
  setDataMark(EUSERDATATYPE_RIGHTHAND);
  setDataMark(EUSERDATATYPE_PET_PARTNER);
  setDataMark(EUSERDATATYPE_STRPOINT);
  setDataMark(EUSERDATATYPE_INTPOINT);
  setDataMark(EUSERDATATYPE_AGIPOINT);
  setDataMark(EUSERDATATYPE_DEXPOINT);
  setDataMark(EUSERDATATYPE_VITPOINT);
  setDataMark(EUSERDATATYPE_LUKPOINT);
  setDataMark(EUSERDATATYPE_TOTALPOINT);
  setDataMark(EUSERDATATYPE_NORMAL_SKILL);
  setDataMark(EUSERDATATYPE_JOBLEVEL);
  setDataMark(EUSERDATATYPE_JOBEXP);
  setDataMark(EUSERDATATYPE_SKILL_POINT);
  setDataMark(EUSERDATATYPE_CLOTHCOLOR);
  setDataMark(EUSERDATATYPE_CUR_MAXJOB);

  // data inform
  UserSyncCmd sync;
  UserNineSyncCmd nine;
  UserDataSync session;
  fetchChangeData(sync, nine, session);

  // to client
  ExchangeProfession cmdme;
  ExchangeProfession cmdnine;

  cmdme.set_guid(id);
  cmdnine.set_guid(id);

  for (int i = 0; i < sync.datas_size(); ++i)
  {
    UserData* pData = cmdme.add_datas();
    if (pData != nullptr)
    {
      pData->CopyFrom(sync.datas(i));
      XDBG << "[玩家-转职数据同步]" << accid << id << getProfession() << name << "type:" << pData->type() << "value:" << pData->value() << XEND;
    }
  }
  for (int i = 0; i < sync.attrs_size(); ++i)
  {
    UserAttr* pAttr = cmdme.add_attrs();
    if (pAttr != nullptr)
    {
      pAttr->CopyFrom(sync.attrs(i));
      XDBG << "[玩家-转职属性同步]" << accid << id << getProfession() << name << "type:" << pAttr->type() << "value:" <<  pAttr->value() << XEND;
    }
  }
  for (int i = 0; i < sync.pointattrs_size(); ++i)
  {
    UserAttr* pAttr = cmdme.add_pointattrs();
    if (pAttr != nullptr)
    {
      pAttr->CopyFrom(sync.pointattrs(i));
      XDBG << "[玩家-转职属性同步]" << accid << id << getProfession() << name << "type:" << pAttr->type() << "value:" << pAttr->value() << XEND;
    }
  }

  for (int i = 0; i < nine.datas_size(); ++i)
  {
    UserData* pData = cmdnine.add_datas();
    if (pData != nullptr)
    {
      pData->CopyFrom(nine.datas(i));
      XDBG << "[玩家-转职九屏数据同步]" << accid << id << getProfession() << name << "type:" << pData->type() << "value:" << pData->value() << XEND;
    }
  }
  for (int i = 0; i < nine.attrs_size(); ++i)
  {
    UserAttr* pAttr = cmdnine.add_attrs();
    if (pAttr != nullptr)
    {
      pAttr->CopyFrom(nine.attrs(i));
      XDBG << "[玩家-转职九屏属性同步]" << accid << id << getProfession() << name << "type:" << pAttr->type() << "value:" << pAttr->value() << XEND;
    }
  }

  PROTOBUF(cmdme, sendme, lenme);
  sendCmdToMe(sendme, lenme);

  PROTOBUF(cmdnine, sendnine, lennine);
  sendCmdToNine(sendnine, lennine, id);

  // to server
  PROTOBUF(session, sendsession, lensession);
  thisServer->sendCmdToSession(sendsession, lensession);

  // inform add attr
  if (m_pAttribute != nullptr)
  {
    m_pAttribute->clearShowAttr();
    LuaManager::getMe().call<void>("calcUserShowAttrValuePro", static_cast<xSceneEntryDynamic*>(this), m_oUserSceneData.getRolelv(), oldProfes, m_oUserSceneData.getProfession());
    const TVecAttrSvrs& vecShow = m_pAttribute->getShowAttr();
    for (auto &v : vecShow)
    {
      const RoleData* pData = RoleDataConfig::getMe().getRoleData(v.type());
      if (pData != nullptr)
        MsgManager::sendMsg(id, 42, MsgParams(pData->prop, static_cast<DWORD>(v.value())));
    }
  }

  // play expression
  playDynamicExpression(EAVATAREXPRESSION_SMILE);

  XLOG << "[玩家-转职]" << accid << id << getProfession() << name << "从" << getProfession() << "转职" << profession << "成功" << XEND;
  //XLOG << "[玩家-转职] " << accid << id << name << " oldpro : " << oldProfes << "curpro : " << m_oUserSceneData.getProfession() << "totalpro : " << m_vecFighters.size() << "个" << XEND;
  return true;
}

void SceneUser::rename(const string& newName)
{
  string oldName = name;
  setRenameTime(xTime::getCurSec());
  set_name(newName.c_str());
  setDataMark(EUSERDATATYPE_NAME);
  if(getTeamID())
    setMark(EMEMBERDATA_NAME);
  refreshDataAtonce();

  // 同步给存档数据
  getUserRecords().onCharNameChange(newName);

  QWORD qwWeddingParnterID = getUserWedding().getWeddingParnter();
  if (qwWeddingParnterID != 0)
  {
    Cmd::WeddingInfo& rInfo = m_oUserWedding.getWeddingInfo();
    UserRenameWedSCmd scmd;
    scmd.set_weddingid(rInfo.id());
    scmd.set_charid(id);
    PROTOBUF(scmd, ssend, slen);
    thisServer->sendSCmdToWeddingServer(id, name, ssend, slen);
  }

  // 同步客户端
  UserRenameCmd cmd;
  cmd.set_name(newName);

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);

  // 通知gate刷新快照
  UserRenameRegCmd regCmd;
  regCmd.accid = accid;

  if(gatetask)
    gatetask->sendCmd(&regCmd, sizeof(regCmd));

  PlatLogManager::getMe().changeFlagLog(thisServer,
      getUserSceneData().getPlatformId(),
      getZoneID(),
      id,
      EChangeFlag_User_Name, oldName, newName);

  XLOG << "[玩家-改名]" << accid << id << getProfession() << "更改名字" << oldName << "为" << newName << "成功" << XEND;
}

// change name
bool SceneUser::changeName(const string& newName)
{
  return true;
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  if (rCFG.checkNameValid(name, ENAMETYPE_USER) == true)
  {
    XERR << "[玩家-改名]" << accid << id << getProfession() << name << "名字中未包含屏蔽字" << XEND;
    return false;
  }
  if (rCFG.checkNameValid(newName, ENAMETYPE_USER) == true)
  {
    XERR << "[玩家-改名]" << accid << id << getProfession() << "更改名字" << name << "为" << newName << "失败,新名字中含有屏蔽字" << XEND;
    return false;
  }

  set_name(newName.c_str());
  setDataMark(EUSERDATATYPE_NAME);
  refreshDataAtonce();

  // return to client
  ChangeNameUserCmd cmd;
  cmd.set_name(newName);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);

  XLOG << "[玩家-改名]" << accid << id << getProfession() << "更改名字" << name << "为" << newName << "成功" << XEND;
  return true;
}

EError SceneUser::levelupDead()
{
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_DEAD) == true)
  {
    XERR << "[玩家-亡者等级]" << accid << id << getProfession() << name << "升级失败,功能屏蔽中" << XEND;
    return EERROR_FAIL;
  }
  const SRoleBaseCFG* pCFG = getRoleBaseCFG();
  if (pCFG == nullptr)
  {
    XERR << "[玩家-亡者等级]" << accid << id << getProfession() << name << "升级失败,该玩家未包含正确的配置" << XEND;
    return EERROR_FAIL;
  }

  DWORD dwTotal = m_oUserSceneData.getDeadExp() + m_oUserSceneData.getDeadCoin();
  DWORD dwOldLv = m_oUserSceneData.getDeadLv();
  DWORD dwMaxLv = MiscConfig::getMe().getDeadCFG().dwMaxLv;
  if (m_oUserSceneData.getDeadLv() >= dwMaxLv)
  {
    XERR << "[玩家-亡者等级]" << accid << id << getProfession() << name << "升级失败,亡者等级" << m_oUserSceneData.getDeadLv() << "已达到本职业最大等级" << XEND;
    return EERROR_FAIL;
  }

  while (true)
  {
    const SDeadLvCFG* pCFG = DeadConfig::getMe().getDeadCFG(m_oUserSceneData.getDeadLv() + 1);
    if (pCFG == nullptr || dwTotal < pCFG->dwExp)
      break;
    if (m_oUserSceneData.getDeadLv() + 1 > dwMaxLv)
      break;

    XLOG << "[玩家-亡者等级]" << accid << id << getProfession() << name << "消耗" << pCFG->dwExp << "亡者气息,lv" << m_oUserSceneData.getDeadLv() << "->" << m_oUserSceneData.getDeadLv() + 1  << XEND;
    dwTotal -= pCFG->dwExp;
    m_oUserSceneData.setDeadLv(m_oUserSceneData.getDeadLv() + 1);
  }

  m_oUserSceneData.setDeadCoin(0);
  m_oUserSceneData.setDeadExp(dwTotal);

  if (dwOldLv != m_oUserSceneData.getDeadLv())
  {
    MsgManager::sendMsg(id, 3499, MsgParams(m_oUserSceneData.getDeadLv()));
    setDataMark(EUSERDATATYPE_CUR_MAXJOB);
    refreshDataAtonce();
  }
  XLOG << "[玩家-亡者等级]" << accid << id << getProfession() << name << "亡者气息" << m_oUserSceneData.getDeadCoin() << "亡者经验" << m_oUserSceneData.getDeadExp() << "亡者等级" << m_oUserSceneData.getDeadLv() << XEND;
  return EERROR_SUCCESS;
}

// 升级
void SceneUser::addBaseExp(QWORD exp, ESource eSource, int* pIsFull/*nullptr*/)
{
  if (pIsFull)
    *pIsFull = 0;

  // check max level
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  if (m_oUserSceneData.getRolelv() >= rCFG.dwMaxBaseLv)
  {
    XLOG << "[玩家-base经验增加]" << accid << id << getProfession() << name << "超级最大等级" << rCFG.dwMaxBaseLv << "无法获取经验 source :" << eSource << XEND;
    if (pIsFull)
      *pIsFull = 1;
    return;
  }

  // add exp
  DWORD baseExp = m_oUserSceneData.getRoleexp();
  baseExp += exp;
  DWORD oldLv = m_oUserSceneData.getRolelv();

  // level up
  while (true)
  {
    // get config
    const SUserBaseLvCFG* pCFG = BaseLevelConfig::getMe().getBaseLvCFG(m_oUserSceneData.getRolelv() + 1);
    if (pCFG == NULL)
    {
      XLOG << "[玩家-base经验增加] 失败" << accid << id << getProfession() << name << "配置中没有该等级：" << m_oUserSceneData.getRolelv() + 1 << "无法获取经验 source :" << eSource << XEND;
      break;
    }

    // check exp
    if (baseExp < pCFG->needExp)
      break;

    // levelup
    baseExp -= pCFG->needExp;
    m_oUserSceneData.setRolelv(m_oUserSceneData.getRolelv() + 1);

    // add point
    for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
      (*v)->setTotalPoint((*v)->getTotalPoint() + pCFG->point);
    m_dwTotalAttrPoint += pCFG->point;

    // check max level
    if (m_oUserSceneData.getRolelv() >= rCFG.dwMaxBaseLv)
    {
      m_oUserSceneData.setRolelv(rCFG.dwMaxBaseLv);
      baseExp = 0;
      XLOG << "[玩家-base经验增加]" << accid << id << getProfession() << name << "超级最大等级" << rCFG.dwMaxBaseLv << "无法获取经验" << XEND;
      break;
    }
  }

  if(baseExp == m_oUserSceneData.getRoleexp() && oldLv == m_oUserSceneData.getRolelv())
    return ;

  // set exp
  m_oUserSceneData.setRoleexp(baseExp);

  // levelup
  if (oldLv != m_oUserSceneData.getRolelv())
    baseLevelup(oldLv, m_oUserSceneData.getRolelv());

  // update data
  refreshDataAtonce();

  //// inform
  //if (eSource == ESOURCE_OFFLINE)
  //{
  //  DWORD dwTime = SceneLuaManager::getMe().call<DWORD>("calcUserOfflineTime", xTime::getCurSec(), m_oUserSceneData.getOfflineTime());
  //  MsgManager::sendMsg(id, 2502, MsgParams(dwTime, exp));
  //}
  MsgManager::sendMsg(id, 13, MsgParams(exp), EMESSAGETYPE_GETEXP);

  addExpLog(300, exp, m_oUserSceneData.getRoleexp(), eSource);
  XLOG << "[玩家-base经验增加]" << accid << id << getProfession() << name << "exp:" << exp << "totalexp:" << baseExp << "oldlv:" << oldLv << "curlv:" << m_oUserSceneData.getRolelv() << "source:" << eSource << XEND;
}

void SceneUser::addJobExp(QWORD exp, ESource eSource, int* pIsFull/*nullptr*/)
{
  if (pIsFull)
    *pIsFull = 0;
  if (m_pCurFighter == nullptr)
    return;
  const SRoleBaseCFG* pCFG = m_pCurFighter->getRoleCFG();
  if (pCFG == nullptr)
    return;
  DWORD maxLv = m_pCurFighter->getMaxJobLv();
  if (m_pCurFighter->getJobLv() >= maxLv)
  {
    if (pIsFull)
      *pIsFull = 1;
    return;
  }

  // add exp
  DWORD jobExp = m_pCurFighter->getJobExp();
  jobExp += exp;
  DWORD oldLv = m_pCurFighter->getJobLv();

  // level up
  while (true)
  {
    // get config
    const SUserJobLvCFG* pDestCFG = JobLevelConfig::getMe().getJobLvCFG(m_pCurFighter->getJobLv() + 1);
    if (pDestCFG == nullptr)
    {
      XLOG << "[玩家-job经验增加] 失败" << accid << id << getProfession() << name << "exp:" << exp << "totalexp:" << jobExp << "oldlv:" << oldLv << "配置中没有该等级: " << m_pCurFighter->getJobLv()+1 << "source:" << eSource << XEND;
      break;
    }

    // check exp
    if (jobExp < pDestCFG->needExp)
      break;

    // level up
    jobExp -= pDestCFG->needExp;
    m_pCurFighter->setJobLv(m_pCurFighter->getJobLv() + 1);

    // add skill point
    m_pCurFighter->getSkill().setSkillPoint(m_pCurFighter->getSkill().getSkillPoint() + 1, ESOURCE_LVUP);

    // check max level
    if (m_pCurFighter->getJobLv() >= maxLv)
    {
      jobExp = 0;
      break;
    }
  }

  if(jobExp == m_pCurFighter->getJobExp() && oldLv == m_pCurFighter->getJobLv())
    return ;

  // set exp
  m_pCurFighter->setJobExp(jobExp);

  // update attribute
  if (oldLv != m_pCurFighter->getJobLv())
    jobLevelup(oldLv, m_pCurFighter->getJobLv());

  // update data
  refreshDataAtonce();

  // inform
  MsgManager::sendMsg(id, 14, MsgParams(exp), EMESSAGETYPE_GETEXP);

  addExpLog(400, exp, m_oUserSceneData.getRoleexp(), eSource);
  // log
  XLOG << "[玩家-job经验增加]" << accid << id << getProfession() << name << "exp:" << exp << "totalexp:" << jobExp << "oldlv:" << oldLv << "curlv:" << m_pCurFighter->getJobLv() << "source:" << eSource << XEND;
}

void SceneUser::dropBaseExp(ESource eSource)
{
  if (m_oUserSceneData.getRoleexp() == 0)
    return;

  Scene* pScene = getScene();
  if (pScene == nullptr || pScene->getSceneBase() == nullptr || pScene->isPVPScene() == true)
    return;

  DWORD dropExp = LuaManager::getMe().call<DWORD>("calcUserDieDropBaseExp", this);
  DWORD baseExp = m_oUserSceneData.getRoleexp();
  // record die drop exp for relive to return
  SDWORD rate = m_oBuff.onDropBaseExp(eSource); // buff影响经验扣除值
  dropExp = floor(1.0f * dropExp * (1.0f - (rate > 1000 ? 1000 : rate) / 1000.0f));
  m_dwDieDropExp = (baseExp >= dropExp ? dropExp : baseExp);
  m_oUserSceneData.setRoleexp(baseExp >= dropExp ? baseExp - dropExp : 0);

  if (m_dwDieDropExp > 0)
    m_oAchieve.onDead(false, m_dwDieDropExp);
  setDataMark(EUSERDATATYPE_DROPBASEEXP);
  //MsgManager::sendMsg(id, 2501, MsgParams(m_dwDieDropExp));

  // log
  reduceExpLog(300, dropExp, m_oUserSceneData.getRoleexp(), eSource);
  XLOG << "[玩家-base经验扣除]" << accid << id << getProfession() << name << "扣除 exp:" << m_dwDieDropExp << "totalexp:" << m_oUserSceneData.getRoleexp() << "source:" << eSource << XEND;
}

void SceneUser::baseLevelup(DWORD oldLv, DWORD newLv)
{
  // update attribute
  setCollectMark(ECOLLECTTYPE_BASE);
  updateAttribute();

  if (m_pAttribute != nullptr)
  {
    // recover hp
    m_pCurFighter->setHp(m_pAttribute->getAttr(EATTRTYPE_MAXHP));
    m_pCurFighter->setSp(m_pAttribute->getAttr(EATTRTYPE_MAXSP));

    // inform add attr
    m_pAttribute->clearShowAttr();
    LuaManager::getMe().call<void>("calcUserShowAttrValueLv", static_cast<xSceneEntryDynamic*>(this), getProfession(), oldLv, newLv);
    const TVecAttrSvrs& vecShow = m_pAttribute->getShowAttr();
    for (auto &v : vecShow)
    {
      const RoleData* pData = RoleDataConfig::getMe().getRoleData(v.type());
      if (pData != nullptr)
        MsgManager::sendMsg(id, 42, MsgParams(pData->prop, static_cast<DWORD>(v.value())));
    }
  }

  DWORD dwCurSec = now();
  DWORD dwCostTime = 0;
  if (dwCurSec > m_oUserSceneData.getLevelUpTime())
    dwCostTime = dwCurSec - m_oUserSceneData.getLevelUpTime();
  // log
  PlatLogManager::getMe().levelUpLog(thisServer,
    m_oUserSceneData.getPlatformId(), getZoneID(),
    accid,
    id,
    oldLv,
    m_oUserSceneData.getRolelv(),
    m_oUserSceneData.getCharge(),
    ELevelType_BaseLv,
    dwCostTime
  );

  // process event
  m_event.onBaseLevelup(newLv, oldLv);
  XLOG << "[玩家-base升级] " << accid << id << getProfession() << name << " old:" << oldLv << "new:" << newLv << XEND;
}

void SceneUser::jobLevelup(DWORD oldLv, DWORD newLv)
{
  // update attribute
  setCollectMark(ECOLLECTTYPE_BASE);

  // process event
  m_event.onJobLevelup(oldLv, newLv);
  m_oServant.onAppearEvent(ETRIGGER_JOBLEVELUP);

  // log
  XLOG << "[玩家-job升级] " << accid << id << getProfession() << name << " old:" << oldLv << "new:" << newLv << XEND;

  PlatLogManager::getMe().levelUpLog(thisServer,
    m_oUserSceneData.getPlatformId(), getZoneID(),
    accid,
    id,
    oldLv,
    newLv,
    m_oUserSceneData.getCharge(),
    ELevelType_JobLv
  );
}

void SceneUser::gomap(DWORD mapid, GoMapType type, const xPos& pos /*= xPos(0, 0, 0)*/)
{
  if (!m_bMarkInScene && m_blInScene)
  {
    XERR << "[地图跳转-异常], 玩家:" << name << id << "跳往地图:" << mapid << (DWORD)type << XEND;
    return;
  }

  if (!mapid)
  {
    XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "不合法" << XEND;
    return;
  }

  if (isAlive() == false)
    relive(ERELIVETYPE_MIN);
  else if (getStatus() == ECREATURESTATUS_FAKEDEAD)
    setStatus(ECREATURESTATUS_LIVE);

  bool playEffect = false;
  const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(mapid);
  if (pRaidCFG && pRaidCFG->bNoPlayGoMap)
  {
    playEffect = false;
  }
  else
  {
    switch (type)
    {
      case GoMapType::Freyja:
      case GoMapType::Tower:
      case GoMapType::GM:
      case GoMapType::GMFollow:
      case GoMapType::GoPVP:
      case GoMapType::KickUser:
      case GoMapType::RemoveTeamMemeber:
      case GoMapType::Quest:
      case GoMapType::Follow:
      case GoMapType::Laboratory:
      case GoMapType::GoCity:
      case GoMapType::Skill:
      default:
        playEffect = true;
        break;
      case GoMapType::System:
      case GoMapType::ExitPoint:
      case GoMapType::Relive:
        break;
    }
  }

  if (getScene() == nullptr || m_oUserSceneData.getOnlineMapID() != mapid || getScene()->check2PosInNine(pos, getPos()) == false)
    setMusicData(0, 0, 0);

  do
  {
    // raid map
    if (pRaidCFG != nullptr)
    {
      if (pRaidCFG->dwLimitLv != 0 && getLevel() < pRaidCFG->dwLimitLv)
      {
        XLOG << "[玩家-地图跳转], 等级过低, 不能进入该副本" << pRaidCFG->dwRaidID << name << id << getLevel() << XEND;
        MsgManager::sendMsg(id, 204);
        break;
      }

      CreateDMapParams params;

      if (pRaidCFG->eRaidType == ERAIDTYPE_TOWER)
      {
        if (getLevel() < MiscConfig::getMe().getEndlessTowerCFG().dwLimitUserLv)
        {
          XLOG << "[玩家-地图跳转], 等级过低, 不能进入无限塔" << name << id << getLevel() << XEND;
          break;
        }

        if (GoMapType::Follow == type)
        {
          /*if (isSameTeamZone(m_oUserSceneData.getFollowerID()) == false)
          {
            playEffect = false;
            XERR << "[玩家-地图跳转]" << accid << id << getProfession() << name
              << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "无限塔传送,不同分线无法跟随" << XEND;
          }
          else*/
          {            
            EnterTower cmd;
            PROTOBUF(cmd, send, len);
            thisServer->forwardCmdToSessionUser(id, send, len);
            XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name
              << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "无限塔传送,发往至社交服处理" << XEND;
          }
          break;
        }
        else if (GoMapType::GM == type)
        {
        }
        else
        {
          playEffect = false;
          XERR << "[玩家-地图跳转]" << accid << id << getProfession() << name
            << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "无限塔传送,只能跟随进入" << XEND;
          break;
        }
      }
      else if (pRaidCFG->eRaidType == ERAIDTYPE_DOJO)
      {
        DWORD dwGroupID = DojoConfig::getMe().getGroupIDByRaid(pRaidCFG->dwRaidID);
        DWORD dwLvReq = MiscConfig::getMe().getGuildDojoCFG().getBaseLvReq(dwGroupID);
        if (getLevel() < dwLvReq)
        {
          playEffect = false;
          MsgManager::sendMsg(id, ESYSTEMMSG_ID_DOJO_BASELV_REQ);
          XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << "等级不足，无法进道场, 需要等级 :" << dwLvReq << XEND;
          break;
        }

        if (GoMapType::Follow == type)
        {
          /*if (isSameTeamZone(m_oUserSceneData.getFollowerID()) == false)
          {
            playEffect = false;
            XERR << "[玩家-地图跳转]" << accid << id << getProfession() << name
              << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "道场传送,不同分线无法跟随" << XEND;
          }
          else*/
          {         
            EnterDojo cmd;
            PROTOBUF(cmd, send, len);
            thisServer->forwardCmdToSessionUser(id, send, len);
            XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name
              << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "道场传送,发往至社交服处理" << XEND;
          }
          break;
        }
        else if (GoMapType::GM == type)
        {
        }
        else
        {
          playEffect = false;
          XERR << "[玩家-地图跳转]" << accid << id << getProfession() << name
            << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "道场传送,只能跟随进入" << XEND;
          break;
        }
      }
      else if (pRaidCFG->eRaidType == ERAIDTYPE_PVECARD && GoMapType::Follow == type)
      {
        MsgManager::sendMsg(id, 116);
        // pve 不可跟随进入
        break;
      }
      else if (pRaidCFG->eRaidType == ERAIDTYPE_SUPERGVG && GoMapType::Follow == type)
      {
        //gvg 不可跟随进入
        break;
      }
      else if(pRaidCFG->eRaidType == ERAIDTYPE_ALTMAN)
      {
        if (GoMapType::Follow == type)
        {
          {
            TeamRaidEnterCmd cmd;
            cmd.set_raid_type(ERAIDTYPE_ALTMAN);
            PROTOBUF(cmd, send, len);
            thisServer->forwardCmdToSessionUser(id, send, len);
            XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name
              << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "奥特曼副本传送,发往至社交服处理" << XEND;
          }
          break;
        }
        else if (GoMapType::GM == type)
        {
        }
        else
        {
          playEffect = false;
          XERR << "[玩家-地图跳转]" << accid << id << getProfession() << name
            << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "奥特曼副本传送,只能跟随进入" << XEND;
          break;
        }
      }
      else if (ERAIDRESTRICT_TEAM == pRaidCFG->eRestrict || (ERAIDRESTRICT_USER_TEAM == pRaidCFG->eRestrict && 0 != getTeamID()))
      {
        DWORD zoneid = getUserZone().getRaidZoneID(mapid);
        if (0 == zoneid)
        {
          if (getTeamID() == 0)
          {
            playEffect = false;
            break;
          }
          GoTeamRaidSocialCmd message;
          message.set_raidid(mapid);
          message.set_myzoneid(thisServer->getZoneID());
          message.set_teamid(getTeamID());
          message.set_charid(this->id);
          message.set_gomaptype((DWORD)type);
          PROTOBUF(message, send, len);
          thisServer->sendCmdToSession(send, len);
          XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name
            << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "副本传送,发往至社交服处理" << XEND;
          break;
        }
        else
        {
          if (zoneid == thisServer->getZoneID())
          {
            getUserZone().delRaidZoneID(mapid);
          }
          else
          {
            getUserZone().gomap(zoneid, mapid, type);
            XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name
              << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "副本传送,分线跳转" << XEND;
            break;
          }
        }
      }
#ifdef D_ROBOT_DEBUG
      else if (ERAIDRESTRICT_GUILD_FIRE == pRaidCFG->eRestrict)
      {
        const TMapGuildCityCFG& cfg = GuildRaidConfig::getMe().getGuildCityCFGList();
        for (auto &m : cfg)
        {
          auto it = m.second.setGroupRaids.find(mapid);
          if (it != m.second.setGroupRaids.end())
          {
            params.m_dwGuildRaidIndex = m.second.dwCityID;
            break;
          }
        }
      }
#endif

      //CreateDMapParams params;
      params.dwRaidID = mapid;
      params.m_oEnterPos = pos;
      params.m_oType = type;
      params.qwCharID = this->id;
      params.m_qwRoomId = getUserZone().getRoomId();
#ifdef _DEBUG
      if (m_blShowSkill)
        params.m_qwRoomId = 0;
#endif
      m_oGuild.toData(&params.oGuildInfo);
      SceneManager::getMe().createDScene(params);
      XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name
        << "mapid:" << mapid << "pos:(" << pos.x << pos.y << pos.z << ") type:" << (DWORD)type << "发送至会话进行副本创建" << XEND;
      break;
    }

    // static map
    Scene *s = SceneManager::getMe().getSceneByID(mapid);
    if (s != nullptr && s->isDScene() == true)
    {
      playEffect = false;
      XERR << "[玩家-地图跳转]" << accid << id << getProfession() << name
        << "mapid:" << mapid << "pos:( " << pos.x << pos.y << pos.z << "), type:" << (DWORD)type << "在静态跳转模块进行了副本跳转" << XEND;
      return;
    }

    if (s && s->isSScene())
    {
      if (s == getScene())
        goTo(pos, GoMapType::ExitPoint==type);
      else
        gomap(s, pos);
      XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name << "mapid:" << mapid << "pos:( " << pos.x << pos.y << pos.z << "), type:" << (DWORD)type << "本场景跳转" << XEND;
      break;
    }

    Cmd::ChangeSceneSessionCmd message;
    ScenePos *p = message.mutable_pos();
    if (p)
    {
      p->set_x(pos.getX());
      p->set_y(pos.getY());
      p->set_z(pos.getZ());
    }
    message.set_mapid(mapid);
    message.add_charid(id);
    PROTOBUF(message, send, len);
    thisServer->sendCmdToSession(send, len);

    XLOG << "[玩家-地图跳转]" << accid << id << getProfession() << name << "mapid:" << mapid << "pos:( " << pos.x << pos.y << pos.z << "), type:" << (DWORD)type << "发送至会话处理" << XEND;
  } while (0);

#ifdef _DEBUG
  playEffect = true;  // 打开传送特效方便发现问题
#endif

  if (playEffect)
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

  //log  不是很准，跟随拦截或者地图跳不过去。
  Scene* pScene = getScene();
  DWORD oldMapid = 0;
  if (pScene) {
    oldMapid = pScene->getMapID();
    if (pScene->isDScene())
    {
      DScene* pDs = dynamic_cast<DScene*> (pScene);
      if (pDs)
      {
        oldMapid = pDs->getRaidID();
      }
    }
  }
  {
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Change_Move;
    PlatLogManager::getMe().eventLog(thisServer,
      getUserSceneData().getPlatformId(),
      getZoneID(),
      accid,
      id,
      eid,
      getUserSceneData().getCharge(),  /*charge */
      eType, 0, 1);
    PlatLogManager::getMe().changeLog(thisServer,
      getUserSceneData().getPlatformId(),
      getZoneID(),
      accid,
      id,
      eType,
      eid,
      EChange_Move,
      oldMapid,
      mapid,
      0);

    if (GoMapType::Follow == type || GoMapType::GMFollow == type || m_oUserSceneData.getFollowerID() != 0)
    {
      QWORD eid = xTime::getCurUSec();
      EVENT_TYPE eType = EventType_Change_Follow;
      PlatLogManager::getMe().eventLog(thisServer,
        getUserSceneData().getPlatformId(),
        getZoneID(),
        accid,
        id,
        eid,
        getUserSceneData().getCharge(),  /*charge */
        eType, 0, 1);

      PlatLogManager::getMe().changeLog(thisServer,
        getUserSceneData().getPlatformId(),
        getZoneID(),
        accid,
        id,
        eType,
        eid,
        EChange_Follow,
        oldMapid,
        mapid,
        m_oUserSceneData.getFollowerID());
    }
  }
  // 确认代码
  /*if (GoMapType::Hands != type && GoMapType::Skill != type)
  {
    GuildScene* pScene = dynamic_cast<GuildScene*>(getScene());
    if (pScene != nullptr)
    {
      DWORD lastmapid = m_oUserSceneData.getLastRealMapID();
      if (!MapConfig::getMe().isRaidMap(lastmapid))
        mapid = lastmapid;
      else
        mapid = m_oUserSceneData.getSaveMap();
    }
  }*/
  // 确认代码:在leavescene中发送了
  /*if (GoMapType::GMFollow != type && GoMapType::Follow != type)
  {
    //m_oUserSceneData.setFollowerID(0);
    // send change data first to prevent client can't receive data when jump map
    updateData(UnregType::Null);
  }*/
  // 确认代码:功能整改
  /*if (GoMapType::Hands != type)
  {
    if (m_oHands.has() && !m_oHands.isMaster())
    {
      m_oHands.breakup();
    }
  }*/
}

SceneNpc* SceneUser::getVisitNpcObj()
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_qwVisitNpc);
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
    return nullptr;
  if (pNpc->getScene() == nullptr || getScene() == nullptr || pNpc->getScene() != getScene())
    return nullptr;
  float fDist = ::getXZDistance(getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
    return nullptr;
  return pNpc;
}

void SceneUser::goTo(xPos pos, bool isGoMap/*=false*/, bool noCheckScene/*=false*/)
{
  if(m_oBooth.hasOpen())
    return;

  if(DressUpStageMgr::getMe().isInStageRange(this, pos, 0, nullptr) == true && getDressUp().getDressUpStatus() == 0)
    return;

  xSceneEntryDynamic::goTo(pos, isGoMap, noCheckScene);
  syncMyPosToTeam(true);
}

void SceneUser::gomap(Scene *s, const xPos& newpos)
{
  if (!s) return;
  if (!m_bMarkInScene && m_blInScene)
  {
    XERR << "[地图跳转-异常], 玩家:" << name << id << "跳往地图:" << s->name << XEND;
    return;
  }

  //m_oHands.onChangeScene(s->id, newpos);
  leaveScene();
  m_oUserSceneData.setOnlineMapPos(s->id, newpos);
  notifyChangeScene();
}

// 货币
bool SceneUser::checkMoney(EMoneyType eType, QWORD value)
{
  switch (eType)
  {
    case EMONEYTYPE_MIN:
      return false;
    case EMONEYTYPE_DIAMOND:
      return m_oUserSceneData.getDiamond() >= value;
    case EMONEYTYPE_SILVER:
      return m_oUserSceneData.getSilver() >= value;
    case EMONEYTYPE_GOLD:
      return m_oUserSceneData.getGold() >= value;
    case EMONEYTYPE_GARDEN:
      return false;//m_oUserSceneData.getGarden() >= value;
    case EMONEYTYPE_MANUALSKILL:
      return m_oManual.getSkillPoint() >= value;
    case EMONEYTYPE_FRIENDSHIP:
      return false;//m_oUserSceneData.getFriendShip() >= value;
    case EMONEYTYPE_CONTRIBUTE:
      /*if (isSubContributeLock())
        return false;
      return m_oUserSceneData.getCon() >= value;*/
      return m_oGuild.contribute() >= value;
    case EMONEYTYPE_GUILDASSET:
      return false;
    case EMONEYTYPE_PVPCOIN:
      return m_oUserSceneData.getPvpCoin() >= value;
    case EMONEYTYPE_LOTTERY:
      return m_oUserSceneData.getLotteryCoin() >= value;
    case EMONEYTYPE_GUILDHONOR:
      return m_oUserSceneData.getGuildHonor() >= value;
    case EMONEYTYPE_DEADCOIN:
      return m_oUserSceneData.getDeadCoin() >= value;
    case EMONEYTYPE_MAX:
      return false;
  }

  return false;
}

bool SceneUser::addMoney(EMoneyType eType, QWORD value, ESource eSource, bool itemShow/*false*/)
{
  QWORD qwTotal = 0;

  bool blRet = false;
  DWORD itemId = 0;

  switch (eType)
  {
    case EMONEYTYPE_DIAMOND:
      m_oUserSceneData.setDiamond(m_oUserSceneData.getDiamond() + value);
      qwTotal = m_oUserSceneData.getDiamond();
      blRet = true;
      break;
    case EMONEYTYPE_SILVER:
      m_oUserSceneData.setSilver(m_oUserSceneData.getSilver() + value);
      qwTotal = m_oUserSceneData.getSilver();
      blRet = true;
      if (eSource != ESOURCE_RESTORE && eSource != ESOURCE_UNSTRENGTH)
        m_oAchieve.onAddMoney(value);
      if (eSource == ESOURCE_CHARGE)
      {
        m_oUserSceneData.setChargeZeny(m_oUserSceneData.getChargeZeny() + value);
        m_oUserSceneData.setDailyChargeZeny(m_oUserSceneData.getDailyChargeZeny() + value);
      }
      else if (eSource == ESOURCE_SHOP)
      {
        m_oUserSceneData.setDailyChargeZeny(m_oUserSceneData.getDailyChargeZeny() + value);
      }
      m_oUserSceneData.setDailyNormalZeny(m_oUserSceneData.getDailyNormalZeny() + value);
      itemId = 100;
      break;
    case EMONEYTYPE_GOLD:
     /* m_oUserSceneData.setGold(m_oUserSceneData.getGold() + value);
      qwTotal = m_oUserSceneData.getGold();
      blRet = true;*/
      {
        XERR << "[玩家-货币增加] 金币类型已取消" << accid << id << getProfession() << name << " type:" << eType << "add:" << value << "total:" << qwTotal << "source:" << eSource << XEND;
        blRet = false;
      }
      break;
    case EMONEYTYPE_GARDEN:
      /*m_oUserSceneData.setGarden(m_oUserSceneData.getGarden() + value);
      qwTotal = m_oUserSceneData.getGarden();*/
      blRet = false;
      break;
    case EMONEYTYPE_PVPCOIN:
      m_oUserSceneData.setPvpCoin(m_oUserSceneData.getPvpCoin() + value);
      qwTotal = m_oUserSceneData.getPvpCoin();
      blRet = true;
      break;
    case EMONEYTYPE_MANUALSKILL:
      blRet = false;
      break;
    case EMONEYTYPE_CONTRIBUTE:
      {
        /*xLuaData data;
        data.setData("num", value);
        data.setData("source", static_cast<DWORD>(eSource));
        GMCommandRuler::addcon(this, data);*/

        stringstream sstr;
        sstr << "guild cmd=addcon num=" << value << " source=" << static_cast<DWORD>(eSource);
        GMCommandRuler::getMe().execute(this, sstr.str());
      }
      blRet = true;
      break;
    case EMONEYTYPE_GUILDASSET:
      {
        /*xLuaData data;
        data.setData("num", value);
        data.setData("self", 1);
        data.setData("source", static_cast<DWORD>(eSource));
        GMCommandRuler::addasset(this, data);
        XLOG << "[公会-资金] 个人增加，accid" << accid << "charid" << id << "num" << value << XEND;*/

        stringstream sstr;
        sstr << "guild cmd=addasset num=" << value << " self=1" << " nocheck=0" << " source=" << static_cast<DWORD>(eSource);
        GMCommandRuler::getMe().execute(this, sstr.str());
      }
      blRet = true;
      break;
    case EMONEYTYPE_FRIENDSHIP:
      /*m_oUserSceneData.setFriendShip(m_oUserSceneData.getFriendShip() + value);
      qwTotal = m_oUserSceneData.getFriendShip();
      blRet = true;*/
      blRet = false;
      break;
    case EMONEYTYPE_LOTTERY:
      m_oUserSceneData.setLotteryCoin(m_oUserSceneData.getLotteryCoin() + value);
      qwTotal = m_oUserSceneData.getLotteryCoin();
      itemId = 151;
      blRet = true;
      break;
    case EMONEYTYPE_GUILDHONOR:
      {
        QWORD oldValue = value;
        value = m_oDeposit.getGuildHonor(value);
        m_oUserSceneData.setGuildHonor(m_oUserSceneData.getGuildHonor() + value);
        qwTotal = m_oUserSceneData.getGuildHonor();
        blRet = true;
        if (value > oldValue)
        {
          MsgParams params;
          params.addNumber(156);
          params.addNumber(156);
          params.addNumber(value - oldValue);
          MsgManager::sendMsg(id, 6, params, EMESSAGETYPE_FRAME, EMESSAGEACT_ADD);
        }
      }
      break;
    case EMONEYTYPE_DEADCOIN:
      {
        if (getMenu().isOpen(EMENUID_DEAD) == false)
        {
          XERR << "[玩家-货币增加]" << accid << id << getProfession() << name << "type:" << eType << "add:" << value << "失败,menu :" << EMENUID_DEAD << "未开启" << XEND;
          break;
        }
        const SDeadMiscCFG& rCFG = MiscConfig::getMe().getDeadCFG();
        if (m_oUserSceneData.getDeadCoin() >= rCFG.dwMaxCoin)
        {
          XDBG << "[玩家-货币增加]" << accid << id << getProfession() << name << "type:" << eType << "add:" << value << "失败,数量 :" << m_oUserSceneData.getDeadCoin() << "超过上限" << rCFG.dwMaxCoin << XEND;
          break;
        }
        DWORD dwResult = m_oUserSceneData.getDeadCoin() + value;
        DWORD dwOffset = 0;
        if (dwResult >= rCFG.dwMaxCoin)
        {
          dwOffset = dwResult - rCFG.dwMaxCoin;
          dwResult = rCFG.dwMaxCoin;
        }
        m_oUserSceneData.setDeadCoin(dwResult);
        qwTotal = m_oUserSceneData.getDeadCoin();

        m_dwTempDeadGet = value - dwOffset;
        m_oVar.setVarValue(EVARTYPE_DEAD_COIN, m_oVar.getVarValue(EVARTYPE_DEAD_COIN) + m_dwTempDeadGet);
        blRet = true;
      }
      break;
    default:
      blRet = false;
      break;
  }

  QWORD eid = xTime::getCurUSec();

  if (itemId&&itemShow)
  {
    ItemShow64 cmd;
    cmd.set_id(itemId);
    cmd.set_count(value);

    PROTOBUF(cmd, send, len);
    sendCmdToMe(send, len);
  }

  if(eType != EMONEYTYPE_CONTRIBUTE && eType != EMONEYTYPE_GUILDASSET)
  {
    PlatLogManager::getMe().incomeMoneyLog(thisServer,
        m_oUserSceneData.getPlatformId(),
        getZoneID(), accid, id, eid,
        m_oUserSceneData.getCharge(), eType, value, qwTotal,
        eSource);
  }

  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_INCOME_COUNT, eType, eSource, getLevel(), value);

  if (blRet)
  {
    m_oQuest.onMoneyChange();
    m_event.onMoneyChange(eType);
  }

  XLOG << "[玩家-货币增加] " << accid << id << getProfession() << name << " type:" << eType << "add:" << value << "total:" << qwTotal << "source:" << eSource << XEND;

  if (eSource == ESOURCE_CHARGE || eSource == ESOURCE_TRADE || eSource == ESOURCE_TRADE_PUBLICITY || eSource == ESOURCE_TRADE_PUBLICITY_FAILRET)
  {
    saveDataNow();
  }

  return blRet;
}

bool SceneUser::subMoney(EMoneyType eType, QWORD value, ESource eSource)
{
  if (checkMoney(eType, value) == false)
    return false;

  QWORD qwTotal = 0;
  QWORD qwChargeMoney = 0;
  QWORD qwLeftChargeMoney = 0;
  bool blRet = false;
  switch (eType)
  {
    case EMONEYTYPE_DIAMOND:
      m_oUserSceneData.setDiamond(m_oUserSceneData.getDiamond() - value);
      qwTotal = m_oUserSceneData.getDiamond();
      blRet = true;
      break;
    case EMONEYTYPE_SILVER:
      m_oUserSceneData.setSilver(m_oUserSceneData.getSilver() - value);
      qwTotal = m_oUserSceneData.getSilver();
      blRet = true;
      qwLeftChargeMoney = m_oUserSceneData.getChargeZeny();
      if (qwLeftChargeMoney > value)
      {
        qwLeftChargeMoney = qwLeftChargeMoney - value;
        qwChargeMoney = value;
      }
      else
      {
        qwChargeMoney = qwLeftChargeMoney;
        qwLeftChargeMoney = 0;
      }
      m_oUserSceneData.setChargeZeny(qwLeftChargeMoney);
      break;
    case EMONEYTYPE_GOLD:
     /* m_oUserSceneData.setGold(m_oUserSceneData.getGold() - value);
      qwTotal = m_oUserSceneData.getGold();
      blRet = true;*/
      {
        XERR << "[玩家-货币扣除] 金币类型已取消" << accid << id << getProfession() << name << " type:" << eType << "sub:" << value << "total:" << qwTotal << "source:" << eSource << XEND;
        blRet = false;
      }
      break;
    case EMONEYTYPE_GARDEN:
      /*m_oUserSceneData.setGarden(m_oUserSceneData.getGarden() - value);
      qwTotal = m_oUserSceneData.getGarden();*/
      blRet = false;
      break;
    case EMONEYTYPE_PVPCOIN:
      {
        DWORD pcoin = m_oUserSceneData.getPvpCoin();
        pcoin = pcoin > value ? pcoin - value : 0;
        m_oUserSceneData.setPvpCoin(pcoin);
        qwTotal = m_oUserSceneData.getGarden();
        blRet = true;
      }
      break;
    case EMONEYTYPE_MANUALSKILL:
      m_oManual.subSkillPoint(value);
      qwTotal = m_oManual.getSkillPoint();
      blRet = true;
      break;
    case EMONEYTYPE_CONTRIBUTE:
      {
        /*xLuaData data;
        data.setData("num", value);
        GMCommandRuler::subcon(this, data);*/

        stringstream sstr;
        sstr << "guild cmd=subcon num=" << value;
        GMCommandRuler::getMe().execute(this, sstr.str());
        m_oGuild.set_contribute(m_oGuild.contribute() - value);
        setDataMark(EUSERDATATYPE_CONTRIBUTE);
        refreshDataAtonce();
      }
      blRet = true;
      break;
    case EMONEYTYPE_FRIENDSHIP:
      /*m_oUserSceneData.setFriendShip(m_oUserSceneData.getFriendShip() - value);
      qwTotal = m_oUserSceneData.getFriendShip();
      blRet = true;*/
      blRet = false;
      break;
    case EMONEYTYPE_LOTTERY:
    {
      m_oUserSceneData.setLotteryCoin(m_oUserSceneData.getLotteryCoin() - value);
      qwTotal = m_oUserSceneData.getLotteryCoin();
      blRet = true;
    }
    break;
    case EMONEYTYPE_GUILDHONOR:
      {
        DWORD honor = m_oUserSceneData.getGuildHonor();
        honor = honor > value ? honor - value : 0;
        m_oUserSceneData.setGuildHonor(honor);
        qwTotal = m_oUserSceneData.getGuildHonor();
        blRet = true;
      }
      break;
    case EMONEYTYPE_DEADCOIN:
      {
        m_oUserSceneData.setDeadCoin(m_oUserSceneData.getDeadCoin() - value);
        qwTotal = m_oUserSceneData.getDeadCoin();
        blRet = true;
      }
      break;
    default:
      blRet = false;
      break;
  }

  // 交易所购买, 增加信用度
  if (eSource == ESOURCE_TRADE)
  {
    const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
    if (rCFG.dwBuyRatio)
      m_oUserSceneData.addCredit(value / rCFG.dwBuyRatio);
  }

  QWORD eid = xTime::getCurUSec();
  PlatLogManager::getMe().outcomeMoneyLog(thisServer,
    m_oUserSceneData.getPlatformId(),
    getZoneID(), accid, id, eid,
    m_oUserSceneData.getCharge(), eType, value, qwTotal,
    eSource, qwChargeMoney, qwLeftChargeMoney);

  XLOG << "[玩家-货币扣除] " << accid << id << getProfession() << name << " type:" << eType << "sub:" << value << "total:" << qwTotal << "source:" << eSource << qwChargeMoney << qwLeftChargeMoney << XEND;
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_CONSUME_COUNT, eType, eSource, getLevel(), value);
  return blRet;
}

bool SceneUser::addCharge(DWORD chargeMoney)
{
  if (!chargeMoney)
    return false;

  //数据库里存的charge 已分为单位
  m_oUserSceneData.setCharge(m_oUserSceneData.getCharge() + chargeMoney * 100);
  m_oAchieve.onCharge();
  XLOG << "[充值-修改充值数据]" << accid << id << getProfession() << name << " 充值了" << chargeMoney << "元人民币" << XEND;

  // 充值增加信用度
  const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
  m_oUserSceneData.addCredit(rCFG.dwChargeRatio * chargeMoney * 100);
  
  stopSendInactiveLog();
  return true;
}

bool SceneUser::canClientRelive(EReliveType eType) const
{
  return eType != ERELIVETYPE_MIN && eType != ERELIVETYPE_SKILL && eType != ERELIVETYPE_TOWER;
}

void SceneUser::relive(EReliveType eType, SceneUser* pReliver /*=nullptr*/)
{
  if (getStatus() != ECREATURESTATUS_DEAD)
  {
    XERR << "[玩家-复活]" << accid << id << getProfession() << name << "没有死亡 无法复活" << XEND;
    return;
  }

  if (eType < ERELIVETYPE_MIN || eType >= ERELIVETYPE_MAX)
  {
    XERR << "[玩家-复活]" << accid << id << getProfession() << name << "复活 type:" << eType << "不合法" << XEND;
    return;
  }

  DWORD tomapid = MiscConfig::getMe().getSystemCFG().dwNewCharMapID;
  xPos topos;
  const SReliveCFG& rCFG = MiscConfig::getMe().getReliveCFG();

  // 公会战复活处理, 不在保存地图复活
  if (eType == ERELIVETYPE_RETURNSAVE && getScene() && getScene()->getSceneType() == SCENE_TYPE_GUILD_FIRE)
    eType = ERELIVETYPE_RETURN;

  switch (eType)
  {
    case ERELIVETYPE_MIN:
      changeHp(getAttr(EATTRTYPE_MAXHP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f, this, false, true);
      setSp(getAttr(EATTRTYPE_MAXSP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f);
      tomapid = 0;
      break;
    case ERELIVETYPE_RETURN:
      {
        changeHp(getAttr(EATTRTYPE_MAXHP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f, this, false, true);
        setSp(getAttr(EATTRTYPE_MAXSP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f);

        Scene* pScene = getScene();
        if (pScene != nullptr && pScene->base != nullptr)
        {
          if (pScene->base->isPvPMap() == true)
          {
            xPos pos;
            pScene->getRandPos(pos);
            changeHp(getAttr(EATTRTYPE_MAXHP), this, false, true);
            setSp(getAttr(EATTRTYPE_MAXSP));
            tomapid = pScene->id;
            topos = pos;
          }
          else if (pScene->isDScene() == true)
          {
            if (pScene->isMatchScene())
            {
              xPos pos;
              MatchScene* pMScene = dynamic_cast<MatchScene*>(pScene);
              if (pMScene)
              {
                if (pScene->getSceneType() == SCENE_TYPE_MVPBATTLE)
                {
                  MvpBattleScene* pMvpScene = dynamic_cast<MvpBattleScene*>(pMScene);
                  if (pMvpScene) pMvpScene->getRelivePos(this, pos);
                }
                else
                {
                  pMScene->getBornPos(this, pos);
                }
              }
              tomapid = 0;
              if (pScene->isDPvpScene())
              {
                changeHp(getAttr(EATTRTYPE_MAXHP), this, false, true);
                setSp(getAttr(EATTRTYPE_MAXSP));
              }
              else if (pScene->isSuperGvg())
              {
                changeHp(getAttr(EATTRTYPE_MAXHP), this, false, true);
                setSp(getAttr(EATTRTYPE_MAXSP));
              }
              goTo(pos);
            }
            else if (pScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
            {
              GuildFireScene* pGScene = dynamic_cast<GuildFireScene*>(pScene);
              if (pGScene)
              {
                changeHp(getAttr(EATTRTYPE_MAXHP), this, false, true);
                setSp(getAttr(EATTRTYPE_MAXSP));
                QWORD defguildid = pGScene->getDefenseGuildID();
                // 防守方复活在公会
                if (getGuild().id() && getGuild().id() == defguildid)
                {
                  tomapid = MiscConfig::getMe().getGuildCFG().dwTerritory;
                }
                // 攻击方复活在保存地图
                else
                {
                  tomapid = m_oUserSceneData.getSaveMap();
                }
                /*// 攻击方复活在第一张入口地图
                else
                {
                  GuildCity* pCity = GuildCityManager::getMe().getCityByID(pGScene->getCityID());
                  if (pCity)
                  {
                    GuildFireScene* pReliveScene = pCity->getReliveScene();
                    if (pReliveScene && pReliveScene == pGScene)
                    {
                      tomapid = 0;
                      const SceneObject *pObject = pScene->getSceneObject();
                      if (pObject)
                      {
                        const map<DWORD, xPos>& mapList = pObject->getBornPointList();
                        for (auto &m : mapList)
                        {
                          goTo(m.second);
                          break;
                        }
                      }
                    }
                    else if (pReliveScene)
                    {
                      tomapid = 0;
                      gomap(pReliveScene, xPos());
                    }
                  }
                }*/
              }
            }
            else
            {
              tomapid = pScene->base->getReliveMapID();
              const SceneBase* pBase = SceneManager::getMe().getDataByID(tomapid);
              if (pBase != nullptr)
              {
                DWORD raidId = 0;
                if (pBase->isStaticMap())
                  raidId = 0;
                else
                  raidId = tomapid;
                const SceneObject* pObject = pBase->getSceneObject(raidId);
                if (pObject != nullptr)
                {
                  const xPos* pPos = pObject->getBornPoint(pScene->getSceneBase()->getReliveBp());
                  if (pPos)
                    topos = *pPos;
                }
              }
              if (tomapid != pScene->id)
              {
                pScene->addReliveLeaveUser(id);
              }             
            }
          }
          else if (pScene->isSScene() == true)
          {
            tomapid = m_oUserSceneData.getSaveMap();
          }
        }
      }
      break;
    case ERELIVETYPE_MONEY:
      {
        BasePackage* pMainPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
        if (pMainPack)
        {
          for (auto v = rCFG.vecConsume.begin(); v != rCFG.vecConsume.end(); ++v)
          {
            if (pMainPack->checkItemCount(v->id(), v->count()) == false)
              return;
          }
          for (auto v = rCFG.vecConsume.begin(); v != rCFG.vecConsume.end(); ++v)
            pMainPack->reduceItem(v->id(),ESOURCE_RELIVE, v->count());

          changeHp(getAttr(EATTRTYPE_MAXHP), this, false, true);
          setSp(getAttr(EATTRTYPE_MAXSP));
        }
        if (m_bMarkKillByMonster && m_dwDieDropExp != 0)
        {
          this->addBaseExp(m_dwDieDropExp, ESOURCE_RELIVE);
          MsgManager::sendMsg(id, 2506, MsgParams(m_dwDieDropExp));
        }
        if (getScene() != nullptr)
        {
          tomapid = getScene()->id;
          topos = getPos();
        }
      }
      break;
    case ERELIVETYPE_RAND:
      /*{
        Scene* pScene = getScene();
        if (pScene == nullptr || !pScene->base->isPvPMap())
          return;

        xPos pos;
        pScene->getRandPos(pos);
        changeHp(getAttr(EATTRTYPE_MAXHP));
        setSp(getAttr(EATTRTYPE_MAXSP));
        tomapid = pScene->id;
        topos = pos;
      }*/
      break;
    case ERELIVETYPE_RETURNSAVE:
      changeHp(getAttr(EATTRTYPE_MAXHP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f, this, false, true);
      setSp(getAttr(EATTRTYPE_MAXSP) * 1.0f * rCFG.dwReliveHpPercent / 100.0f);
      tomapid = m_oUserSceneData.getSaveMap();
      break;
    case ERELIVETYPE_SKILL:
      if (m_bMarkKillByMonster)
      {
        if (m_dwDieDropExp != 0)
          this->addBaseExp(m_dwDieDropExp, ESOURCE_ACTSKILL);
        if (pReliver)
        {
          MsgManager::sendMsg(pReliver->id, 2512, MsgParams(name));
          MsgManager::sendMsg(id, 2507, MsgParams(pReliver->name, m_dwDieDropExp));
          if (m_qwKillMeMvpID)
          {
            SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwKillMeMvpID);
            if (npc && npc->getNpcType() == ENPCTYPE_MVP)
              npc->addMvpRelaReliveTimes(pReliver->id);
          }
        }
      }
      if (pReliver)
      {
        getShare().addCalcData(ESHAREDATATYPE_MOST_BESAVED, pReliver->id, 1);
        pReliver->getShare().addCalcData(ESHAREDATATYPE_MOST_SAVE, id, 1);
        pReliver->getUserGvg().onReliveUser();
        if (getScene())
          getScene()->onReliveUser(this, pReliver);
      }

      if (getScene() != nullptr)
      {
        tomapid = getScene()->id;
        topos = getPos();
      }
      break;
    case ERELIVETYPE_TOWER:
      tomapid = 0;
      changeHp(1, this, false, true);
      break;
    case ERELIVETYPE_MAX:
      break;
  }

  // 清空标记
  m_bMarkKillByMonster = false;
  m_qwKillMeMvpID = 0;
  m_dwDieDropExp = 0;

  setStatus(ECREATURESTATUS_LIVE);
  m_event.onRelive(eType);

  if (tomapid)
    gomap(tomapid, GoMapType::Relive, topos);

  XLOG << "[玩家-复活]" << accid << id << getProfession() << name << "使用" << eType << "复活到地图 :" << tomapid << "pos:(" << topos.x << topos.y << topos.z << ")" << XEND;
}

void SceneUser::checkReliveWhenLoginout()
{
  if (isAlive() == true)
    return;
  relive(ERELIVETYPE_MIN);

  const SceneBase* pBase = SceneManager::getMe().getDataByID(m_oUserSceneData.getSaveMap());
  if (pBase == nullptr)
  {
    XERR << "[玩家-离线复活]" << accid << id << getProfession() << name << "map :" << m_oUserSceneData.getOnlineMapID() << "未在Table_Map.txt表中找到" << XEND;
    return;
  }
  DWORD dwReliveMapID = m_oUserSceneData.getSaveMap();
  const SceneObject *pObject = pBase->getSceneObject(0);
  if (pObject == nullptr)
  {
    XERR << "[玩家-离线复活]" << accid << id << getProfession() << name << "map :" << m_oUserSceneData.getOnlineMapID() << "未取到地图数据" << XEND;
    return;
  }
  const xPos* pBorn = pObject->getBornPoint(1);
  if (pBorn == nullptr)
  {
    XERR << "[玩家-离线复活]" << accid << id << getProfession() << name << "map :" << m_oUserSceneData.getOnlineMapID() << "未取到出生点1" << XEND;
    return;
  }

  bool setRelivePos = true;
  if (getScene() && getScene()->getSceneType() == SCENE_TYPE_TEAMPWS)
    setRelivePos = false;

  if (setRelivePos)
  {
    m_oUserSceneData.setOnlineMapPos(dwReliveMapID, *pBorn);
      XLOG << "[玩家-离线复活]" << accid << id << getProfession() << name
    << "oldmap:" << pBase->getMapInfo().id << "tomapid:" << dwReliveMapID << "pos:(" << pBorn->x << pBorn->y << pBorn->z << ")" << XEND;
  }
}

void SceneUser::updateVar()
{
  if (m_oVar.hasNew() == false)
    return;

  VarUpdate cmd;
  m_oVar.collectVar(cmd);
  m_oVar.clearNew();

  if (cmd.vars_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    sendCmdToMe(send, len);
  }
}

// 合成
bool SceneUser::checkCompose(DWORD id, EPackMethod eMethod /*= EPACKMETHOD_NOCHECK*/, GlobalActivityType eEvent,
    ECheckMethod eCheckMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  const SComposeCFG* pBase = ComposeConfig::getMe().getComposeCFG(id);
  if (pBase == nullptr)
    return false;

  DWORD dwDiscount = 100;
  DWORD dwItemDiscount = 100;
  bool isOpen = ActivityManager::getMe().isOpen(static_cast<DWORD>(eEvent));
  if(isOpen == true)
  {
    /*const SGlobalActivityCFG* pGlobal = MiscConfig::getMe().getGlobalActivityCFG(static_cast<DWORD>(eEvent));
    if(pGlobal != nullptr)
    {
      dwDiscount = pGlobal->times;
      if(eEvent == GACTIVITY_SAFE_REFINE)
        dwItemDiscount = pGlobal->times;
    }*/
    const SGlobalActCFG*pGlobal = ActivityConfig::getMe().getGlobalActCFG(static_cast<DWORD>(eEvent));
    if (pGlobal)
    {
      dwDiscount = pGlobal->getParam(0);
      if (eEvent == GACTIVITY_SAFE_REFINE)
        dwItemDiscount = pGlobal->getParam(0);
    }
  }

  BasePackage* pMainPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;

  // check menu unlock
  if (pBase->dwNeedMenuID != 0)
  {
    if (getMenu().isOpen(pBase->dwNeedMenuID) == false)
      return false;
  }

  TVecItemInfo vecMainItems;
  if (pBase->dwROB*dwDiscount/100 != 0)
  {
    ItemInfo item;
    item.set_id(ITEM_ZENY);
    item.set_count(pBase->dwROB*dwDiscount/100);
    combinItemInfo(vecMainItems, item);
  }
  if (pBase->dwGold*dwDiscount/100 != 0)
  {
    ItemInfo item;
    item.set_id(ITEM_GOLD);
    item.set_count(pBase->dwGold*dwDiscount/100);
    combinItemInfo(vecMainItems, item);
  }
  if (pBase->dwDiamond*dwDiscount/100 != 0)
  {
    ItemInfo item;
    item.set_id(50);
    item.set_count(pBase->dwDiamond*dwDiscount/100);
    combinItemInfo(vecMainItems, item);
  }
  for (auto v = pBase->vecMaterial.begin(); v != pBase->vecMaterial.end(); ++v)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
    if (pCFG == nullptr)
      continue;
    ItemInfo item;
    item.CopyFrom(*v);
    item.set_count(v->count()*dwItemDiscount/100);
    combinItemInfo(vecMainItems, item);
  }
  for (auto v = pBase->vecConsume.begin(); v != pBase->vecConsume.end(); ++v)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
    if (pCFG == nullptr)
      continue;
    ItemInfo item;
    item.CopyFrom(*v);
    item.set_count(v->count()*dwItemDiscount/100);
    combinItemInfo(vecMainItems, item);
  }

  //if (pMainPack->checkItemCount(vecMainItems, eCheckMethod) == false)
  if (m_oPackage.checkItemCount(vecMainItems, eCheckMethod, eFunc) == false)
    return false;

  if (pBase->stProduct.dwTypeID != 0)
  {
    ItemData oData;
    oData.mutable_base()->set_id(pBase->stProduct.dwTypeID);
    oData.mutable_base()->set_count(pBase->stProduct.dwNum);
    if (eMethod != EPACKMETHOD_AVAILABLE && pMainPack->checkAddItem(oData, eMethod) == false)
      return false;
  }
  if (pBase->stCriProduct.dwTypeID != 0)
  {
    ItemData oData;
    oData.mutable_base()->set_id(pBase->stCriProduct.dwTypeID);
    oData.mutable_base()->set_count(pBase->stCriProduct.dwNum);
    if (eMethod != EPACKMETHOD_AVAILABLE && pMainPack->checkAddItem(oData, eMethod) == false)
      return false;
  }

  return true;
}

string SceneUser::doCompose(DWORD id, EPackMethod eMethod /*= EPACKMETHOD_NOCHECK*/, GlobalActivityType eEvent,
    bool bShow /*= true*/, bool bInform /*= true*/, bool bForceShow /*= true*/, ECheckMethod eCheckMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  if (checkCompose(id, eMethod, eEvent, eCheckMethod, eFunc) == false)
    return STRING_EMPTY;

  const SComposeCFG* pBase = ComposeConfig::getMe().getComposeCFG(id);
  if (pBase == nullptr)
  {
    XERR << "[玩家-合成]" << accid << SceneUser::id << getProfession() << name << "composeid:" << id << "未在Table_Compose.txt表中找到" << XEND;
    return STRING_EMPTY;
  }

  const SItemCFG* pCFG = nullptr;

  const SProductCFG* pProductCFG = pBase->getProductCFG(getUserSceneData().getGender());
  if (pProductCFG == nullptr)
  {
    XERR << "[玩家-合成] 没找到产出物品配置" << accid << SceneUser::id << getProfession() << name << "composeid:" << id << "性别" << getUserSceneData().getGender() << XEND;
    return STRING_EMPTY;
  }
  DWORD itemid = pProductCFG->dwTypeID;
  DWORD count = pProductCFG->dwNum;

  if (itemid != 0)
  {
    pCFG = ItemManager::getMe().getItemCFG(itemid);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-合成]" << accid << SceneUser::id << getProfession() << name << "composeid:" << id << "destitemid:" << itemid << "未在Table_Item.txt表中找到" << XEND;
      return STRING_EMPTY;
    }
  }

  BasePackage* pMainPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return STRING_EMPTY;

  DWORD dwDiscount = 100;
  DWORD dwItemDiscount = 100;
  bool isOpen = ActivityManager::getMe().isOpen(static_cast<DWORD>(eEvent));
  if(isOpen == true)
  {
    const SGlobalActCFG*pGlobal = ActivityConfig::getMe().getGlobalActCFG(static_cast<DWORD>(eEvent));
    if (pGlobal)
    {
      dwDiscount = pGlobal->getParam(0);
      if (eEvent == GACTIVITY_SAFE_REFINE)
        dwItemDiscount = pGlobal->getParam(0);
      XLOG << "[玩家-合成折扣] " << accid << id << getProfession() << name
        << "compose :" << id << "活动ID: " << eEvent << "Zeny折扣: " << dwDiscount << "道具折扣" << dwItemDiscount << XEND;
    }

   /* const SGlobalActivityCFG* pGlobal = MiscConfig::getMe().getGlobalActivityCFG(static_cast<DWORD>(eEvent));
    if(pGlobal != nullptr)
    {
      dwDiscount = pGlobal->times;
      if(eEvent == GACTIVITY_SAFE_REFINE)
        dwItemDiscount = pGlobal->times;
      XLOG << "[玩家-合成折扣] " << accid << id << getProfession() << name
        << "compose :" << id << "活动ID: " << eEvent << "Zeny折扣: " << dwDiscount << "道具折扣" << dwItemDiscount << XEND;
    }*/
  }

  // 仅用与打印/推送log
  auto reduceItemLog = [&](const TVecItemInfo& vecItems, const DWORD dwItemDiscount)
  {
    for (auto v = vecItems.begin(); v != vecItems.end(); ++v)
    {
      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
      if (pCFG == nullptr)
        continue;

      DWORD dwMainCount = pMainPack->getItemCount(v->id());
      DWORD dwReduceMainCount = dwMainCount;
      if (dwMainCount >= v->count()*dwItemDiscount/100)
      {
        dwReduceMainCount = v->count()*dwItemDiscount/100;
      }

      // costss << v->id() << "," << dwReduceMainCount << ";";
      if (ItemManager::getMe().isEquip(pCFG->eItemType) == true)
      {
        if (dwReduceMainCount)
        {
          DWORD dwCount = 0;
          TVecString vecIDs;
          const TSetItemBase& setList = pMainPack->getItemBaseList(v->id());
          for (auto &s : setList)
          {
            ItemEquip* pEquip = dynamic_cast<ItemEquip*>(s);
            if (pEquip == nullptr || pEquip->canBeMaterial() == false)
              continue;
            vecIDs.push_back(s->getGUID());
            if (++dwCount >= dwReduceMainCount)
              break;
          }
          // for (auto &v : vecIDs)
          //   pMainPack->reduceItem(v, ESOURCE_COMPOSE);

          XLOG << "[装备精炼-成功], 消耗道具" << accid << id << getProfession() << name
            << "typeid:" << v->id() <<  XEND;
          StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_EQUIP_EXCHANGE_COUNT, v->id(), 0, 0, (DWORD)dwCount);
        }

        continue;
      }

      // if (dwReduceMainCount)
      //   pMainPack->reduceItem(v->id(),ESOURCE_COMPOSE, dwReduceMainCount);
    }
  };

  // reduce material
  // reduceItem(pBase->vecMaterial, dwItemDiscount);

  std::stringstream costss;
  TVecItemInfo vecMainItems;
  if (pBase->dwROB*dwDiscount/100 != 0)
  {
    ItemInfo item;
    item.set_id(ITEM_ZENY);
    item.set_count(pBase->dwROB*dwDiscount/100);
    combinItemInfo(vecMainItems, item);
    costss << ITEM_ZENY << "," << pBase->dwROB*dwDiscount/100 << ";";
  }
  if (pBase->dwGold*dwDiscount/100 != 0)
  {
    ItemInfo item;
    item.set_id(ITEM_GOLD);
    item.set_count(pBase->dwGold*dwDiscount/100);
    combinItemInfo(vecMainItems, item);
    costss << ITEM_GOLD << "," << pBase->dwGold*dwDiscount/100 << ";";
  }
  if (pBase->dwDiamond*dwDiscount/100 != 0)
  {
    ItemInfo item;
    item.set_id(50);
    item.set_count(pBase->dwDiamond*dwDiscount/100);
    combinItemInfo(vecMainItems, item);
    costss << 50 << "," << pBase->dwDiamond << ";";
  }
  for (auto v = pBase->vecMaterial.begin(); v != pBase->vecMaterial.end(); ++v)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
    if (pCFG == nullptr)
      continue;
    ItemInfo item;
    item.CopyFrom(*v);
    item.set_count(v->count()*dwItemDiscount/100);
    costss << item.id() << "," << item.count() << ";";
    combinItemInfo(vecMainItems, item);
  }
  /*if (pMainPack->checkItemCount(vecMainItems, eCheckMethod) == false)
    return STRING_EMPTY;
  pMainPack->reduceItem(vecMainItems, ESOURCE_COMPOSE, eCheckMethod);*/
  if (m_oPackage.checkItemCount(vecMainItems, eCheckMethod, eFunc) == false)
    return STRING_EMPTY;
  m_oPackage.reduceItem(vecMainItems, ESOURCE_COMPOSE, eCheckMethod, eFunc);
  reduceItemLog(pBase->vecMaterial, dwItemDiscount);

  DWORD rate = 0;
  if (pBase->bDynamicRate)
  {
    rate = LuaManager::getMe().call<DWORD> ("calcProduceRate", (xSceneEntryDynamic*)(this), pBase->eType, pBase->dwCategory, id);
  }
  else
  {
    rate = pBase->dwRate;
  }

  bool bRandCriOk = false;
  bool bRandOk = rate >= (DWORD)randBetween(1, 10000);

  if (bRandOk == false)
  {
    if (pBase->bDynamicRate)
      MsgManager::sendMsg(this->id, 3106);

    return STRING_EMPTY;
  }

  // 先判断是否成功, 再判断是否暴击
  if (pBase->bDynamicRate && pBase->stCriProduct.dwTypeID && pBase->stCriProduct.dwNum)
  {
    DWORD criRate = LuaManager::getMe().call<DWORD> ("calcProduceCriRate", (xSceneEntryDynamic*)(this), pBase->eType, pBase->dwCategory);
    bRandCriOk = criRate ? criRate >= (DWORD)randBetween(1, 10000) : 0;
  }
  // reduce comsume
  // reduceItem(pBase->vecConsume);
  TVecItemInfo vecComsumeItems;
  for (auto v = pBase->vecConsume.begin(); v != pBase->vecConsume.end(); ++v)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
    if (pCFG == nullptr)
      continue;
    ItemInfo item;
    item.CopyFrom(*v);
    item.set_count(v->count()*dwItemDiscount/100);
    costss << item.id() << "," << item.count() << ";";
    combinItemInfo(vecComsumeItems, item);
  }
  //if (pMainPack->checkItemCount(vecComsumeItems, eCheckMethod) == false)
  if (m_oPackage.checkItemCount(vecComsumeItems, eCheckMethod, eFunc) == false)
    return STRING_EMPTY;
  //pMainPack->reduceItem(vecComsumeItems, ESOURCE_COMPOSE, eCheckMethod);
  m_oPackage.reduceItem(vecComsumeItems, ESOURCE_COMPOSE, eCheckMethod, eFunc);
  reduceItemLog(pBase->vecConsume, dwItemDiscount);

  if (pCFG == nullptr)
    return STRING_EMPTY;

  if (pCFG->eItemType == EITEMTYPE_HEAD)
  {
    m_oUserPet.playAction(EPETACTION_OWNER_PRODUCE_HEAD);
  }

  string guid = GuidManager::getMe().newGuidStr(getZoneID(), getUserSceneData().getOnlineMapID());
  if (ItemManager::getMe().isGUIDValid(guid) == false)
    return STRING_EMPTY;

  itemid = bRandCriOk ? pBase->stCriProduct.dwTypeID : itemid;
  count = bRandCriOk ? pBase->stCriProduct.dwNum : count;

  ItemData oData;
  oData.mutable_base()->set_id(itemid);
  oData.mutable_base()->set_count(count);
  oData.mutable_base()->set_guid(guid);
  oData.mutable_base()->set_source(ESOURCE_COMPOSE);

  ItemData oCopy;
  oCopy.CopyFrom(oData);
  m_oPackage.addItem(oCopy, eMethod, bShow, bInform, bForceShow);   //note 制作头饰播放3d展示

  if (pBase->bDynamicRate)
  {
    MsgParams params;
    params.addNumber(oData.base().id());
    params.addNumber(oData.base().id());
    params.addNumber(oData.base().count());
    MsgManager::sendMsg(this->id, 3105, params);
  }

  XLOG << "[玩家-合成] " << accid << xSceneEntryDynamic::id << m_oUserSceneData.getProfession() << name
    << "compose :" << id << "item :" << oData.ShortDebugString() << "成功" << "数量:" << count << XEND;
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_EQUIP_COMPOSE_COUNT, oData.base().id(), 0, 0, (DWORD)count);

  if (ItemManager::getMe().isHead(pCFG->eItemType) == true)
  {
    m_oAchieve.onEquip(EACHIEVECOND_COMPOSE);
    m_oServant.onGrowthFinishEvent(ETRIGGER_HEAD_COMPOSE);
  }
  else if (ItemManager::getMe().isEquip(pCFG->eItemType) == true)
  {
    m_oAchieve.onProduceEquip(EACHIEVECOND_PRODUCE_EQUIP);
    if(pCFG->eEquipType == EEQUIPTYPE_WEAPON)
      m_oServant.onGrowthFinishEvent(ETRIGGER_EQUIP_COMPOSE_WEAPON);
    else if(pCFG->eEquipType == EEQUIPTYPE_ARMOUR)
      m_oServant.onGrowthFinishEvent(ETRIGGER_EQUIP_COMPOSE_ARMOUR);
    //else
    //  m_oServant.onGrowthFinishEvent(ETRIGGER_EQUIP_COMPOSE);
  }

  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Compose;
  PlatLogManager::getMe().eventLog(thisServer,
    getUserSceneData().getPlatformId(),
    getZoneID(),
    accid,
    id,
    eid,
    getUserSceneData().getCharge(), eType, 0, count);

  PlatLogManager::getMe().ComposeLog(thisServer,
    accid,
    id,
    eType,
    eid,
    itemid,
    guid,
    costss.str());

  return guid;
}

void SceneUser::onTeamChange(const GTeam& oldTeam, const GTeam& nowTeam, bool bOnline /*=false*/)
{
  if (gatetask)
  {
    SynTeamUserCmdGatewayCmd send;
    send.accid = accid;
    send.teamid = getTeamID();
    gatetask->sendCmd(&send, sizeof(SynTeamUserCmdGatewayCmd));
  }

  if (nowTeam.getTeamID() == 0)
  {
    DScene* pScene = dynamic_cast<DScene*>(getScene());
    if (pScene != nullptr && (pScene->getRaidRestrict() == ERAIDRESTRICT_TEAM  || pScene->getRaidRestrict() == ERAIDRESTRICT_USER_TEAM || pScene->getRaidRestrict() == ERAIDRESTRICT_GUILD_TEAM
          || pScene->getRaidRestrict() == ERAIDRESTRICT_GUILD_RANDOM_RAID))
      pScene->addKickList(this);

    // 镜像副本踢出队伍时，若是最后一人,通知队友副本关闭, 用于任务重置
    if (pScene && pScene->m_oImages.isImageScene() && oldTeam.getTeamID() != 0)
    {
      xSceneEntrySet userset;
      pScene->getAllEntryList(SCENE_ENTRY_USER, userset);
      userset.erase(this);
      if (userset.empty())
      {
        QuestRaidCloseSessionCmd cmd;
        cmd.set_raidid(pScene->getRaidID());
        for (auto &m : oldTeam.getTeamMemberList())
        {
          if (m.first == id)
            continue;
          cmd.set_userid(m.first);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
    }

    if (pScene != nullptr && (pScene->getRaidType() == ERAIDTYPE_PVP_SMZL || pScene->getRaidType()  == ERAIDTYPE_PVP_HLJS || pScene->getRaidType() == ERAIDTYPE_MVPBATTLE))
    {      
      pScene->addKickList(this);
    }

    if (pScene != nullptr && pScene->getSceneType() == SCENE_TYPE_TEAMPWS && oldTeam.getTeamID() != 0)
    {
      pScene->addKickList(this);
      TeamPwsScene* pTScene = dynamic_cast<TeamPwsScene*>(pScene);
      if (pTScene)
        pTScene->onUserLeaveTeam(this);
    }

    stopCalcTeamLen();

    if (m_oHands.has() == true)
      m_oHands.breakup();

    m_oWeaponPet.onEnterOwnTeam(oldTeam.getTeamID() != 0);
    m_oSceneFood.onLeaveTeam();
  }
  else
  {
    if (nowTeam.getTeamMemberList().empty() == false)
      startCalcTeamLen();

    updateTeamTower();

    //itemimage
    sendItemImage(false);

    syncMyPosToTeam(true);
    syncMemberPosToMe();

    // cat enter team
    m_oWeaponPet.onEnterTeam(oldTeam.getTeamID() == 0 && !bOnline, bOnline);
    if (bOnline)
    {
      /*设置mvp房间队伍信息*/
      if (getScene() && getScene()->getSceneType() == SCENE_TYPE_MVPBATTLE)
      {
        MvpBattleScene* pMScene = dynamic_cast<MvpBattleScene*>(getScene());
        if (pMScene)
          pMScene->onUserGetTeamInfo(this);
      }
    }

    setDataMark(EUSERDATATYPE_ENSEMBLESKILL);
  }

  m_oSkillStatus.onTeamChange(oldTeam, nowTeam);

  Scene* pScene = getScene() != nullptr ? getScene() : SceneManager::getMe().getSceneByID(m_oUserSceneData.getLastMapID());
  if (pScene == nullptr)
    return;

  if (oldTeam.getTeamID() == 0 && nowTeam.getTeamID() != 0)
  {
    // 裂隙添加新队员
    SceneTeamSeal* pSeal = TeamSealManager::getMe().getTeamSealByID(nowTeam.getTeamID());
    if (pSeal)
      pSeal->addMember(id);

    std::set<SceneUser*> userSet;
    for (auto &m : nowTeam.getTeamMemberList())
    {
      if (id == m.second.charid())
        continue;
      SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
      if (user == nullptr || user->getScene() != pScene)
        continue;
      userSet.insert(user);
    }
    if (!userSet.empty())
    {
      TSetDWORD othersNpc;
      for (auto &s : userSet)
      {
        const TSetDWORD& setnpc = s->getUserSceneData().getShowNpcs();
        othersNpc.insert(setnpc.begin(), setnpc.end());
      }
      const TSetDWORD& myNpc = getUserSceneData().getShowNpcs();
      TSetDWORD myTempNpc;
      myTempNpc.insert(myNpc.begin(), myNpc.end());

      // get my spec. npc
      for (auto &q : othersNpc)
      {
        myTempNpc.erase(q);
      }
      // get team others npc
      for (auto &q : myNpc)
      {
        othersNpc.erase(q);
      }

      for (auto &q : othersNpc)
      {
        std::list<SceneNpc*> npclist;
        pScene->getSceneNpcByBaseID(q, npclist);
        for (auto &s : npclist)
        {
          if (s->check2PosInNine(this) && s->isVisableToSceneUser(this))
            s->sendMeToUser(this);
        }
      }
      for (auto &q : myTempNpc)
      {
        std::list<SceneNpc*> npclist;
        pScene->getSceneNpcByBaseID(q, npclist);
        for (auto &s : npclist)
        {
          for (auto &user : userSet)
          {
            if (s->check2PosInNine(user) && s->isVisableToSceneUser(this))
              s->sendMeToUser(user);
          }
        }
      }

      std::set<SceneNpc*> skillnpc;
      m_oSkillProcessor.getAllTrapNpcs(skillnpc);
      // 我的skillnpc发送队友
      for (auto &s : skillnpc)
      {
        if (s->getNpcType() != ENPCTYPE_SKILLNPC)
          continue;
        for (auto &user : userSet)
        {
          if (s->check2PosInNine(user))
            s->sendMeToUser(user);
        }
      }
      // 队友的skillnpc发送给我
      for (auto &user : userSet)
      {
        skillnpc.clear();
        user->m_oSkillProcessor.getAllTrapNpcs(skillnpc);
        for (auto &s : skillnpc)
        {
          if (s->getNpcType() != ENPCTYPE_SKILLNPC)
            continue;
          if (s->check2PosInNine(this))
            s->sendMeToUser(this);
        }
      }
    }

    std::pair<DWORD, DWORD> wantedquest;
    if (getQuest().getWantedQuest(wantedquest, true))
    {
      TeamerQuestUpdateSocialCmd teamcmd;
      MemberWantedQuest* pTeamQuest = teamcmd.mutable_quest();
      pTeamQuest->set_action(EQUESTACTION_ACCEPT);
      pTeamQuest->set_charid(id);
      pTeamQuest->set_questid(wantedquest.first);
      pTeamQuest->set_step(wantedquest.second);
      PROTOBUF(teamcmd, send2, len2);
      thisServer->sendCmdToSession(send2, len2);
    }
    if (!userSet.empty())
    {
      std::list<SceneNpc*> mylist;
      m_oWeaponPet.getPetNpcs(mylist);

      std::list<SceneNpc*> petlist;
      for (auto &s : userSet)
      {
        // teamers's npc send to me
        petlist.clear();
        s->getWeaponPet().getPetNpcs(petlist);
        for (auto &npc : petlist)
          npc->sendMeToUser(this);
        // my npc send to teamers
        for (auto &npc : mylist)
          npc->sendMeToUser(s);
      }
    }

    // 隐匿状态互相同步
    if (pScene->isHideUser())
    {
      bool hide = getAttr(EATTRTYPE_HIDE);
      for (auto &s : userSet)
      {
        if (s->check2PosInNine(this) == false)
          continue;
        if(hide)
        {
          sendMeToUser(s);
          SceneNpc* npc = m_oUserPet.getPetNpc();
          if (npc)
            npc->sendMeToUser(s);
        }
        if (s->getAttr(EATTRTYPE_HIDE))
        {
          s->sendMeToUser(this);
          SceneNpc* npc = s->getUserPet().getPetNpc();
          if (npc)
            npc->sendMeToUser(this);
        }
      }
    }
  }

  if (oldTeam.getTeamID() != 0 && nowTeam.getTeamID() == 0)
  {
    // 裂隙删除队员
    SceneTeamSeal* pSeal = TeamSealManager::getMe().getTeamSealByID(oldTeam.getTeamID());
    if (pSeal)
      pSeal->removeMember(id);

    std::set<SceneUser*> userSet;
    for (auto &m : oldTeam.getTeamMemberList())
    {
      if (id == m.second.charid())
        continue;
      SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
      if (user == nullptr || user->getScene() != pScene)
        continue;
      userSet.insert(user);
    }
    if (!userSet.empty())
    {
      TSetDWORD othersNpc;
      for (auto &s : userSet)
      {
        const TSetDWORD& setnpc = s->getUserSceneData().getShowNpcs();
        othersNpc.insert(setnpc.begin(), setnpc.end());
      }
      const TSetDWORD& myNpc = getUserSceneData().getShowNpcs();
      TSetDWORD myTempNpc;
      myTempNpc.insert(myNpc.begin(), myNpc.end());

      // get my spec. npc
      for (auto &q : othersNpc)
      {
        myTempNpc.erase(q);
      }
      // get team others npc
      for (auto &q : myNpc)
      {
        othersNpc.erase(q);
      }

      for (auto &q : othersNpc)
      {
        std::list<SceneNpc*> npclist;
        pScene->getSceneNpcByBaseID(q, npclist);
        for (auto &s : npclist)
        {
          if (s->check2PosInNine(this))
            s->delMeToUser(this);
        }
      }
      for (auto &q : myTempNpc)
      {
        std::list<SceneNpc*> npclist;
        pScene->getSceneNpcByBaseID(q, npclist);
        for (auto &s : npclist)
        {
          for (auto &user : userSet)
          {
            if (s->check2PosInNine(user))
              s->delMeToUser(user);
          }
        }
      }

      std::set<SceneNpc*> skillnpc;
      m_oSkillProcessor.getAllTrapNpcs(skillnpc);
      // 我的skillnpc 对队友取消可见
      for (auto &s : skillnpc)
      {
        if (s->getNpcType() != ENPCTYPE_SKILLNPC)
          continue;
        for (auto &user : userSet)
        {
          if (s->check2PosInNine(user))
            s->delMeToUser(user);
        }
      }
      // 队友的skillnpc 对我取消可见
      for (auto &user : userSet)
      {
        skillnpc.clear();
        user->m_oSkillProcessor.getAllTrapNpcs(skillnpc);
        for (auto &s : skillnpc)
        {
          if (s->getNpcType() != ENPCTYPE_SKILLNPC)
            continue;
          if (s->check2PosInNine(this))
            s->delMeToUser(this);
        }
      }
    }
    if (!userSet.empty())
    {
      std::list<SceneNpc*> mylist;
      m_oWeaponPet.getPetNpcs(mylist);

      std::list<SceneNpc*> petlist;
      for (auto &s : userSet)
      {
        // teamers's npc del to me
        petlist.clear();
        s->getWeaponPet().getPetNpcs(petlist);
        for (auto &npc : petlist)
          npc->delMeToUser(this);
        // my npc del to teamers
        for (auto &npc : mylist)
          npc->delMeToUser(s);
      }
    }

    // 隐匿状态互相同步
    if (pScene->isHideUser())
    {
      bool hide = getAttr(EATTRTYPE_HIDE);
      for (auto &s : userSet)
      {
        if (s->check2PosInNine(this) == false)
          continue;
        if(hide)
        {
          delMeToUser(s);
          SceneNpc* npc = m_oUserPet.getPetNpc();
          if (npc)
            npc->delMeToUser(s);
        }
        if (s->getAttr(EATTRTYPE_HIDE))
        {
          s->delMeToUser(this);
          SceneNpc* npc = s->getUserPet().getPetNpc();
          if (npc)
            npc->delMeToUser(this);
        }
      }
    }
  }

  std::set<SceneUser*> oldMembers;
  for (auto &m : oldTeam.getTeamMemberList())
  {
    if (id == m.second.charid())
      oldMembers.insert(this);
    else
    {
      SceneUser* user = SceneUserManager::getMe().getUserByID(m.second.charid());
      if (user == nullptr || user->getScene() != pScene)
        continue;
      oldMembers.insert(user);
    }
  }
  for (auto &u : oldMembers) {
    u->m_oBuff.onTeamChange();
  }
  for (auto &u : getTeamSceneUser()) {
    u->m_oBuff.onTeamChange();
  }

  if (bOnline)
  {
    if (m_oUserSceneData.getFollowerID())
    {
      EFollowType t = EFOLLOWTYPE_MIN;
      if (m_oHands.has())
	t = EFOLLOWTYPE_HAND;
      QWORD folid = m_oUserSceneData.getFollowerID();
      m_oUserSceneData.setFollowerID(0);
      m_oUserSceneData.setFollowerID(folid, t);      
    }
  }
  else
  {
    QWORD folid = m_oUserSceneData.getFollowerID();
    if (folid && isMyTeamMember(folid) == false)
      m_oUserSceneData.setFollowerIDNoCheck(0);
  }

  checkRecallBuff();
  checkWeddingBuff();
}

void SceneUser::updateDataToTeam()
{
  MemberDataUpdate cmd;
  fetchTeamData(cmd);
  if (cmd.members_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    thisServer->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_TEAM, m_oGTeam.getTeamID(), send, len);
  }

#ifdef _DEBUG
  for (int i = 0; i < cmd.members_size(); ++i)
  {
    const MemberData& rData = cmd.members(i);
    XDBG << "[玩家-成员数据-组队]" << accid << id << getProfession() << name << "更新" << rData.ShortDebugString() << XEND;
  }
#endif
}

void SceneUser::updateDataToUser(QWORD qwTargetID)
{
  MemberDataUpdate cmd;
  fetchTeamData(cmd);
  if (cmd.members_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(qwTargetID, send, len);
  }

#ifdef _DEBUG
  for (int i = 0; i < cmd.members_size(); ++i)
  {
    const MemberData& rData = cmd.members(i);
    XDBG << "[玩家-成员数据-个人]" << accid << id << getProfession() << name << "更新给" << qwTargetID << rData.ShortDebugString() << XEND;
  }
#endif
}

void SceneUser::fetchTeamData(MemberDataUpdate& cmd)
{
  cmd.Clear();
  static const set<EMemberData> setData = set<EMemberData>{EMEMBERDATA_MAPID, EMEMBERDATA_HP, EMEMBERDATA_MAXHP, EMEMBERDATA_SP, EMEMBERDATA_MAXSP, EMEMBERDATA_GUILDRAIDINDEX, EMEMBERDATA_NAME};
  if (m_TeamBitset.any() == false)
    return;

  if (m_oGTeam.getTeamID() == 0)
  {
    m_TeamBitset.reset();
    return;
  }

  cmd.set_id(id);
  for (auto &s : setData)
  {
    if (m_TeamBitset.test(s) == false)
      continue;

    switch (s)
    {
      case EMEMBERDATA_MAPID:
        GTeam::add_mdata(cmd.add_members(), s, getScene() == nullptr ? m_oUserSceneData.getOnlineMapID() : getScene()->getMapID());
        break;
      case EMEMBERDATA_HP:
        GTeam::add_mdata(cmd.add_members(), s, getAttr(EATTRTYPE_HP));
        break;
      case EMEMBERDATA_MAXHP:
        GTeam::add_mdata(cmd.add_members(), s, getAttr(EATTRTYPE_MAXHP));
        break;
      case EMEMBERDATA_SP:
        GTeam::add_mdata(cmd.add_members(), s, getAttr(EATTRTYPE_SP));
        break;
      case EMEMBERDATA_MAXSP:
        GTeam::add_mdata(cmd.add_members(), s, getAttr(EATTRTYPE_MAXSP));
        break;
      case EMEMBERDATA_GUILDRAIDINDEX:
        if (getScene() && getScene()->getSceneType() == SCENE_TYPE_GUILD_RAID)
        {
          GuildRaidScene* pGRaidScene = dynamic_cast<GuildRaidScene*> (getScene());
          if (pGRaidScene == nullptr)
            break;
          GTeam::add_mdata(cmd.add_members(), s, pGRaidScene->getMapIndex());
        }
        break;
      case EMEMBERDATA_NAME:
        GTeam::add_mdata(cmd.add_members(), s, 0, name);
        break;
      default:
        XERR << "[玩家-成员数据]" << accid << id << getProfession() << name << "更新 s :" << "未处理" << XEND;
        break;
    }
  }
  m_TeamBitset.reset();
}

void SceneUser::updateTeamTower()
{
  //if (getTeamLeaderID() == id)
  {
    TowerLeaderInfoSyncSocialCmd cmd;
    toData(cmd.mutable_user());
    m_oTower.toData(cmd.mutable_info());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
}

/*bool SceneUser::isSameTeamZone(QWORD charId)
{
  if (getTeamID() == 0)
    return false;
  for (int i = 0; i < m_oTeamInfo.member_size(); ++i)
  {
    const TeamMemberInfo& info = m_oTeamInfo.member(i);
    if (charId == info.charid())
    {
      if (info.zoneid() == getZoneID())
        return true;
    }
  }
  return false;
}*/

bool SceneUser::isMyTeamMember(QWORD id)
{
  if (id == this->id)
    return true;
  if (m_oWeaponPet.isMyPet(id))
    return true;

  if (getTeamID() == 0)
    return id == this->id;
  return m_oGTeam.getTeamMember(id) != nullptr;
}

void SceneUser::startCalcTeamLen()
{
  m_teamTimePoint = now();
}

void SceneUser::stopCalcTeamLen()
{
  if (m_teamTimePoint == 0)
    return;
  DWORD sec = now();
  DWORD diff = sec - m_teamTimePoint;
  m_oUserSceneData.setTeamTimeLen(m_oUserSceneData.getTeamTimeLen() + diff);
  m_teamTimePoint = 0;
}

void SceneUser::syncMyPosToTeam(bool bInit /*= false*/)
{
  if (getTeamID() == 0)
    return;

  const xPos& oPos = getScene() == nullptr ? m_oUserSceneData.getOnlinePos() : getPos();
  DWORD dwSceneID = getScene() == nullptr ? m_oUserSceneData.getOnlineMapID() : getScene()->id;
  if (getScene() && getScene()->m_oImages.isImageScene())
    dwSceneID = getScene()->getMapID();

  bool have = true;
  if (!bInit && m_oMove.empty() == true)
    have = false;

  if (m_oHands.has() && m_oHands.isInWait() == false && m_oHands.isFollower())
  {
    SceneUser* pOther = m_oHands.getOther();
    if (pOther && pOther->m_oMove.empty() == false)
      have = true;
  }
  if (m_bSyncPosAtonce)
  {
    have = true;
    m_bSyncPosAtonce = false;
  }

  if (!have)
    return;

  MemberPosUpdate cmd;
  cmd.set_id(id);
  cmd.mutable_pos()->set_x(oPos.getX());
  cmd.mutable_pos()->set_y(oPos.getY());
  cmd.mutable_pos()->set_z(oPos.getZ());
  PROTOBUF(cmd, send, len);

  for (auto &m : m_oGTeam.getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    if (rMember.catid() != 0 || rMember.charid() == id)
      continue;

    SceneUser* pMember = SceneUserManager::getMe().getUserByID(rMember.charid());
    if (pMember == nullptr)
      continue;
    DWORD dwMemberSceneID = 0;
    Scene* pMScene = pMember->getScene();
    if (pMScene)
    {
      if (pMScene->m_oImages.isImageScene())
        dwMemberSceneID = pMScene->getMapID();
      else
        dwMemberSceneID = pMScene->id;
    }
    else
    {
      dwMemberSceneID = pMember->getUserSceneData().getOnlineMapID();
    }

    if (dwMemberSceneID != dwSceneID)
      continue;
    pMember->sendCmdToMe(send, len);
    XDBG << "[玩家-队伍成员位置同步]" << accid << id << getProfession() << name << "pos :(" << "\b" << oPos.x << oPos.y << oPos.z << "\b)" << XEND;
  }
}

void SceneUser::syncMemberPosToMe()
{
  if (getTeamID() == 0)
    return;

  const TeamMemberInfo* pMember = getTeamMember(id);
  if (pMember == nullptr)
    return;

  DWORD dwMySceneID = 0;
  Scene* pScene = getScene();
  if (pScene)
  {
    if (pScene->m_oImages.isImageScene())
      dwMySceneID = pScene->getMapID();
    else
      dwMySceneID = pScene->id;
  }
  else
  {
    dwMySceneID = m_oUserSceneData.getOnlineMapID();
  }

  for (auto &m : m_oGTeam.getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    if (rMember.catid() != 0)
      continue;
    if (rMember.charid() == id)
      continue;

    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rMember.charid());
    if (pUser == nullptr || pUser->getScene() == nullptr || pUser->getTeamID() == 0)
      continue;
    DWORD userMapID = pUser->getScene()->id;
    if (pUser->getScene()->m_oImages.isImageScene())
      userMapID = pUser->getScene()->getMapID();

    if (dwMySceneID != userMapID)
      continue;
    const xPos& pos = pUser->getPos();

    MemberPosUpdate cmd;
    cmd.set_id(pUser->id);
    cmd.mutable_pos()->set_x(pos.getX());
    cmd.mutable_pos()->set_y(pos.getY());
    cmd.mutable_pos()->set_z(pos.getZ());
    PROTOBUF(cmd, send, len);
    sendCmdToMe(send, len);
    XDBG << "[玩家-队伍成员位置同步]" << accid << id << getProfession() << name << "收到了成员" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "pos :(" << "\b" << pos.x << pos.y << pos.z << "\b)" << XEND;
  }
}

DWORD SceneUser::getTeamMemberCount(DWORD dwMapID, DWORD dwRaidID)
{
  if (getTeamID() == 0)
    return 1;

  DWORD dwCount = 0;
  for (auto &m : getTeam().getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    SceneUser* user = SceneUserManager::getMe().getUserByID(rMember.charid());
    if (user == nullptr || user->getScene() == nullptr)
        continue;
    if (user->getScene() == getScene())
        ++dwCount;
  }

  return dwCount;
}

DWORD SceneUser::getSceneTeamCnt()
{
  if (getTeamID() == 0)
    return 1;
  DWORD dwCount = 0;
  for (auto &m : getTeam().getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    SceneUser* user = SceneUserManager::getMe().getUserByID(rMember.charid());
    if (user == nullptr || user->getScene() == nullptr)
        continue;
    if (user->getScene() == getScene())
        ++dwCount;
  }
  return dwCount;
}

std::set<SceneUser*> SceneUser::getTeamSceneUser()
{
  std::set<SceneUser*> res;
  if (getTeamID() == 0)
    return res;
  Scene* pScene = getScene();
  if (!pScene)
    return res;
  for (auto &m : m_oGTeam.getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    SceneUser* pUser = nullptr;
    if (rMember.charid() == id)
      res.insert(this);
    else
    {
      pUser = SceneUserManager::getMe().getUserByID(rMember.charid());
      if (pUser && pUser->getScene() && pUser->getScene() == pScene)
        res.insert(pUser);
    }
  }

  return res;
}

std::set<SceneUser*> SceneUser::getTeamRewardUsers()
{
  if (m_oGTeam.isPickupShare())
    return getTeamSceneUser();

  std::set<SceneUser*> res;
  res.insert(this);
  if (getTeamID() == 0)
    return res;
  Scene* pScene = getScene();
  if (!pScene)
    return res;
  float range = MiscConfig::getMe().getSceneItemCFG().fTeamValidRange;
  for (auto &m : m_oGTeam.getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    if (rMember.charid() == id)
      continue;
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rMember.charid());
    if (pUser && pUser->getScene() && pUser->getScene() == pScene && getXZDistance(pUser->getPos(), getPos()) <= range)
      res.insert(pUser);
  }
  return res;
}

DWORD SceneUser::getTeamTimeLen()
{
  if (getTeamID() != 0 && getTeam().getTeamMemberList().empty() == false) {
    stopCalcTeamLen();
    startCalcTeamLen();
  }
  return m_oUserSceneData.getTeamTimeLen();
}

void SceneUser::gotoUserMap(QWORD charid)
{
  SceneUser *pUser = SceneUserManager::getMe().getUserByID(charid);
  if (pUser)
  {
    if (pUser->getScene())
    {
      gomap(pUser->getScene(), pUser->getPos());
    }
  }
  else
  {
    Cmd::GoToUserMapSessionCmd message;
    message.set_targetuserid(charid);
    message.set_gotouserid(id);
    PROTOBUF(message, send, len);
    thisServer->sendCmdToSession(send, len);
  }
}

// 角色
SceneFighter* SceneUser::getFighter(EProfession profession) const
{
  auto v = find_if(m_vecFighters.begin(), m_vecFighters.end(), [profession](const SceneFighter* p) -> bool{
    if (p == nullptr)
      return false;

    return profession == p->getProfession();
  });
  if (v == m_vecFighters.end())
    return nullptr;

  return *v;
}

SceneFighter* SceneUser::getFighter(DWORD dwType, DWORD dwBranch) const
{
  auto v = find_if(m_vecFighters.begin(), m_vecFighters.end(), [dwType, dwBranch](const SceneFighter* p) -> bool{
    if (p == nullptr || p->getRoleCFG() == nullptr)
      return false;

    return dwType == p->getRoleCFG()->dwType && dwBranch == p->getRoleCFG()->dwTypeBranch;
  });
  if (v == m_vecFighters.end())
    return nullptr;

  return *v;
}

DWORD SceneUser::getFighterTypeCount(EProfession eProfession) const
{
  eProfession = RoleConfig::getMe().getBaseProfession(eProfession);
  DWORD dwCount = 0;
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
  {
    if (*v == nullptr)
      continue;

    EProfession eBase = RoleConfig::getMe().getBaseProfession((*v)->getProfession());
    if (eProfession != eBase)
      continue;

    const SRoleBaseCFG* pCFG = (*v)->getRoleCFG();
    if (pCFG == nullptr || pCFG->dwTypeBranch == 0)
      continue;

    ++dwCount;
  }

  return dwCount;
}

DWORD SceneUser::getJobLv() const
{
  if (m_pCurFighter == nullptr)
    return 0;

  return m_pCurFighter->getJobLv();
}

bool SceneUser::load(const BlobData& oBlob)
{
  const BlobFighter& oFighter = oBlob.fighter();
  XLOG << "[玩家-加载武将]" << accid << id << getProfession() << name << "武将数据数量 :" << oFighter.datas_size() << XEND;

  clearFighter();
  for (int i = 0; i < oFighter.datas_size(); ++i)
  {
    const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(oFighter.datas(i).profession());
    if (pCFG == nullptr)
    {
      XERR << "[玩家-加载武将]" << accid << id << getProfession() << name << "pro:" << oFighter.datas(i).profession() << "未在配置表Table_Class.txt中找到" << XEND;
      continue;
    }

    SceneFighter* pFighter = NEW SceneFighter(this, pCFG);
    if (pFighter == nullptr || pFighter->fromRoleData(oFighter.datas(i)) == false)
    {
      XERR << "[玩家-加载武将]" << accid << id << getProfession() << name << "pro:" << oFighter.datas(i).profession() << "创建失败" << XEND;
      SAFE_DELETE(pFighter);
      return false;
    }

    m_vecFighters.push_back(pFighter);
    XLOG << "[玩家-加载武将]" << accid << id << getProfession() << name << "成功加载 pro :" << oFighter.datas(i).profession() << XEND;
  }
  if (getFighter(EPROFESSION_NOVICE) == nullptr)
  {
    const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(EPROFESSION_NOVICE);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-加载武将]" << accid << id << getProfession() << name << "pro:初心者 未在配置表Table_Class.txt中找到" << XEND;
      return false;
    }
    m_pCurFighter = NEW SceneFighter(this, pCFG);
    if (m_pCurFighter == nullptr)
    {
      XERR << "[玩家-加载武将]" << accid << id << getProfession() << name << "创建默认初心者失败" << XEND;
      return false;
    }
    m_pCurFighter->setProfession(EPROFESSION_NOVICE);
    m_pCurFighter->setJobLv(1);
    m_vecFighters.push_back(m_pCurFighter);

    // add normal skill
    //m_pCurFighter->getSkill().addSkill(pCFG->normalSkill, 0, ESOURCE_MIN);
    //m_pCurFighter->getSkill().addSkill(pCFG->strengthSkill, 0, ESOURCE_MIN);

    // set fighter data : do this after process levelup for open skillgrid
    m_pCurFighter->getSkill().refreshEnableSkill();
    m_pCurFighter->getSkill().clearUpdate();

    XLOG << "[玩家-加载武将]" << accid << id << getProfession() << name << "成功创建初心者" << XEND;
  }

  m_dwMaxJobLv = oFighter.maxjoblv();
  m_dwMaxCurJobLv = oFighter.maxcurjoblv();
  m_dwTotalAttrPoint = oFighter.totalpoint();
  m_dwMaxSkillPos = oFighter.maxskillpos();
  m_dwAutoSkillPos = oFighter.maxautopos();
  m_dwExtendSkillPos = oFighter.maxextendpos();
  // guild
  m_oGuild.updateInfo(oBlob.guild().info());

  // team
  //m_oTeamInfo.CopyFrom(oBlob.team().info());

  // set cur fighter
  SceneFighter* pFighter = getFighter(m_oUserSceneData.getProfession());
  if (pFighter == nullptr)
  {
    XERR << "[玩家-加载武将]" << accid << id << getProfession() << name << "没有找到职位为" << m_oUserSceneData.getProfession() << "职业的武将" << XEND;
    return false;
  }
  m_pCurFighter = pFighter;

  // update skill func
  m_oUserSceneData.setBlink(getLearnedSkillByID(MiscConfig::getMe().getExpressionCFG().dwBlinkNeedSkill) != nullptr);
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  for (auto &s : rCFG.setPackSkill)
  {
    if (getLearnedSkillByID(s) != nullptr)
      m_oUserSceneData.insertPackSkill(s);
  }

  // 多职业功能开启后，首次登录角色保存其巅峰等级
  if(m_dwMaxJobLv < m_pCurFighter->getMaxJobLv())
    m_dwMaxJobLv = m_pCurFighter->getMaxJobLv();

  if (m_dwMaxCurJobLv < m_pCurFighter->getJobLv())
    m_dwMaxCurJobLv = m_pCurFighter->getJobLv();

  setBranch();
  reqFixBranch();
  updateEnsembleSkill();
  XLOG << "[玩家-加载武将]" << accid << id << getProfession() << name << "实在加载" << m_vecFighters.size() << "个" << XEND;
  return true;
}

bool SceneUser::save(BlobData& oBlob)
{
  BlobFighter* pFighter = oBlob.mutable_fighter();
  if (pFighter == nullptr)
    return false;

  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
  {
    UserRoleData* pData = pFighter->add_datas();
    if (pData == nullptr)
      continue;

    if ((*v)->toRoleData(*pData) == false)
      XERR << "[玩家-角色保存]" << accid << id << getProfession() << name << "fighter:" << (*v)->getProfession() << "create protobuf error" << XEND;
  }

  pFighter->set_maxjoblv(m_dwMaxJobLv);
  pFighter->set_maxcurjoblv(m_dwMaxCurJobLv);
  pFighter->set_totalpoint(m_dwTotalAttrPoint);
  pFighter->set_maxskillpos(m_dwMaxSkillPos);
  pFighter->set_maxautopos(m_dwAutoSkillPos);
  pFighter->set_maxextendpos(m_dwExtendSkillPos);
  // guild
  m_oGuild.toData(oBlob.mutable_guild()->mutable_info());

  // team
  //oBlob.mutable_team()->mutable_info()->CopyFrom(m_oTeamInfo);

  XDBG << "[玩家-角色保存]" << accid << id << getProfession() << name << "数据大小:" << pFighter->ByteSize() << XEND;
  return true;
}

void SceneUser::sendFighterInfo()
{
  QueryFighterInfo cmd;
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
  {
    FighterInfo* pInfo = cmd.add_fighters();
    if (pInfo == nullptr)
      continue;

    (*v)->toData(pInfo);
  }

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

void SceneUser::addMaxSkillPos()
{
  ++m_dwMaxSkillPos;
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
    (*v)->getSkill().setMaxPos(m_dwMaxSkillPos);
}

void SceneUser::decSkillPos()
{
  if (m_dwMaxSkillPos == 0)
    return;
  --m_dwMaxSkillPos;
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
    (*v)->getSkill().decMaxPos();
}

void SceneUser::addAutoSkillPos()
{
  ++m_dwAutoSkillPos;
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
    (*v)->getSkill().setAutoMaxPos(m_dwAutoSkillPos);
}

void SceneUser::validMonthCardSkillPos()
{
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
    (*v)->getSkill().validMonthCardSkillPos();
}

void SceneUser::decAutoSkillPos()
{
  if (m_dwAutoSkillPos == 0)
    return;
  --m_dwAutoSkillPos;
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
    (*v)->getSkill().decAutoMaxPos();
}

void SceneUser::addExtendSkillPos()
{
  ++m_dwExtendSkillPos;
  for (auto &v : m_vecFighters)
  {
    v->getSkill().setMaxExtendPos(m_dwExtendSkillPos);
  }
}

void SceneUser::decExtendSkillPos()
{
  if (m_dwExtendSkillPos == 0)
    return;
  --m_dwExtendSkillPos;
  for (auto &v : m_vecFighters)
  {
    v->getSkill().decMaxExtendPos();
  }
}

void SceneUser::invalidMonthCardSkillPos()
{
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
    (*v)->getSkill().invalidMonthCardSkillPos();
}

void SceneUser::reloadConfig(ConfigType type)
{
  switch (type)
  {
    case ConfigType::item:
      m_oPackage.reload();
      this->getTitle().reloadConfigCheck();
      m_oUserPet.reloadItem();
      break;
    case ConfigType::role:
      for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
        (*v)->setRoleCFG(RoleConfig::getMe().getRoleBase((*v)->getProfession()));
      break;
    case ConfigType::quest:
      m_oQuest.reload();
      break;
    case ConfigType::skill:
      for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
        (*v)->getSkill().reload();
      break;
    case ConfigType::manual:
      m_oManual.reload();
      break;
    case ConfigType::buffer:
      m_oBuff.reloadAllBuff();
      break;
    case ConfigType::baselevel:
      addBaseExp(0,ESOURCE_GM);
      break;
    case ConfigType::joblevel:
      addJobExp(0,ESOURCE_GM);
      break;
    case ConfigType::pet:
      m_oUserPet.reload();
      getPetWork().reload();
      break;
    case ConfigType::achieve:
      m_oAchieve.reload();
      break;
    case ConfigType::being:
      m_oUserBeing.reload();
      break;
    case ConfigType::exchangeshop:
      getExchangeShop().reloadExchangeShopConfig();
      break;
    default:
      break;
  }
}

void SceneUser::clearFighter()
{
  for (auto v = m_vecFighters.begin(); v != m_vecFighters.end(); ++v)
    SAFE_DELETE(*v);
  m_vecFighters.clear();
}

void SceneUser::setChatRoomID(DWORD roomid)
{
  m_dwChatRoomID = roomid;

  const TVecDWORD& vecBuffIDs = MiscConfig::getMe().getChatRoomCFG().vecBuffIDs;
  if (m_dwChatRoomID)
  {
    for (auto &v : vecBuffIDs)
    {
      m_oBuff.add(v);
    }
    getUserSceneData().setFollowerIDNoCheck(0);
  }
  else
  {
    for (auto &v : vecBuffIDs)
    {
      m_oBuff.del(v);
    }
  }
  XLOG << "[聊天室]" << accid << id << name << "设置roomid:" << roomid;
}

ChatRoom* SceneUser::getChatRoom()
{
  if (!m_dwChatRoomID) return nullptr;

  return ChatRoomManager::getMe().getRoom(m_dwChatRoomID);
}

bool SceneUser::addSkill(DWORD id, DWORD sourceid, ESource eSource)
{
  if (m_pCurFighter == nullptr)
    return false;
  bool bResult = m_pCurFighter->getSkill().addSkill(id, sourceid, eSource);

  if (bResult)
  {
    //XLOG("[玩家-技能添加] %llu, %llu, %u, %s, skill : %u, sourceid : %u, source : %u 成功", accid, xSceneEntryDynamic::id, m_oUserSceneData.getProfession(), name, id, sourceid, eSource);
    return bResult;
  }

  //XLOG("[玩家-技能添加] %llu, %llu, %u, %s, skill : %u, sourceid : %u, source : %u 失败", accid, xSceneEntryDynamic::id, m_oUserSceneData.getProfession(), name, id, sourceid, eSource);
  return bResult;
}

bool SceneUser::removeSkill(DWORD id, DWORD sourceid, ESource eSource)
{
  if (m_pCurFighter == nullptr)
    return false;
  bool bResult = m_pCurFighter->getSkill().removeSkill(id, sourceid, eSource);
  if (bResult)
  {
    //XLOG("[玩家-技能删除] %llu, %llu, %u, %s, skill : %u, sourceid : %u, source : %u 成功", accid, xSceneEntryDynamic::id, m_oUserSceneData.getProfession(), name, id, sourceid, eSource);
    return bResult;
  }

  //XLOG("[玩家-技能删除] %llu, %llu, %u, %s, skill : %u, sourceid : %u, source : %u 成功", accid, xSceneEntryDynamic::id, m_oUserSceneData.getProfession(), name, id, sourceid, eSource);
  return bResult;
}

bool SceneUser::addPurify(DWORD value, bool upperLimit)
{
  DWORD gain = value;
  DWORD nowvalue = m_oUserSceneData.getPurify();
  if (upperLimit)
  {
    DWORD maxvalue = MiscConfig::getMe().getSPurifyCFG().dwMaxPurify;
    if (nowvalue >= maxvalue)
      return false;
    gain = (maxvalue - nowvalue > value) ? value : (maxvalue - nowvalue);
  }
  m_oUserSceneData.setPurify(nowvalue + gain);
  if (m_pCurFighter == nullptr)
    return false;
  m_pCurFighter->getSkill().refresheConsume();

  XLOG << "[玩家-净化值增加]" << accid << id << getProfession() << name << "add:" << value << "total:" << m_oUserSceneData.getPurify() << XEND;
  return true;
}

bool SceneUser::subPurify(DWORD value)
{
  DWORD nowValue = m_oUserSceneData.getPurify();
  if (value > nowValue)
    return false;
  nowValue -= value;
  m_oUserSceneData.setPurify(nowValue);

  if (m_pCurFighter == nullptr)
    return false;
  m_pCurFighter->getSkill().refresheConsume();

  XLOG << "[玩家-净化值减少]" << accid << id << getProfession() << name << " sub:" << value << "total:" << m_oUserSceneData.getPurify() << XEND;
  return true;
}

/*void SceneUser::purifyRaidBoss(QWORD charid)
{
  PurifyNpc* pRaidBoss = dynamic_cast<PurifyNpc*>(SceneNpcManager::getMe().getNpcByTempID(charid));
  if (!pRaidBoss || !pRaidBoss->needPurify())
    return;
  pRaidBoss->purifyByUser(this->id);
}*/

void SceneUser::setOwnWeather(DWORD dwWeather, bool bPrivate /*= false*/)
{
  if (dwWeather == 0)
    return;

  m_dwOwnWeather = dwWeather;
  m_bPrivateSky = bPrivate;

  WeatherChange cmd;
  cmd.set_id(dwWeather);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);

  XLOG << "[玩家-设置天气]" << accid << id << getProfession() << name << "weather:" << dwWeather << "private:" << bPrivate << XEND;
}

void SceneUser::setOwnSky(DWORD dwSky, bool bPrivate /*= false*/, DWORD sec /*=0*/)
{
  if (dwSky == 0)
    return;

  m_dwOwnSky = dwSky;
  m_bPrivateSky = bPrivate;

  SkyChange cmd;
  cmd.set_id(m_dwOwnSky);
  cmd.set_sec(sec);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);

  XLOG << "[玩家-设置天空]" << accid << id << getProfession() << name << "sky:" << dwSky << "private:" << bPrivate << XEND;
}

void SceneUser::setOwnTime(DWORD dwDestTime, bool bPrivate /*= false*/)
{
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();

  m_bPrivateTime = bPrivate;
  SDWORD swCurTime = xTime::getCurSec();
  if (m_bPrivateTime == true)
  {
    SDWORD swDaySec = swCurTime - static_cast<SDWORD>(xTime::getDayStart(swCurTime));
    SDWORD swGameSec = swDaySec % rCFG.dwGameDaySec * rCFG.dwTimeSpeed;
    SDWORD swGameHour = swGameSec / 3600;
    m_sdwGameTimeOffset = (static_cast<SDWORD>(dwDestTime) - swGameHour) * 3600 / static_cast<SDWORD>(rCFG.dwTimeSpeed);
  }
  else
    m_sdwGameTimeOffset = 0;

  syncOwnTime(swCurTime, true);

  XLOG << "[玩家-设置时间]" << accid << id << getProfession() << name << "time:" << dwDestTime << "private:" << bPrivate << XEND;
}

void SceneUser::syncOwnTime(DWORD curTime, bool bAtOnce /*= false*/)
{
  static DWORD syncTick = 0;
  if (curTime < syncTick && !bAtOnce)
    return;

  syncTick = curTime + 900;

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();

  SDWORD swCurTime = curTime;
  SDWORD swDaySec = swCurTime - static_cast<SDWORD>(xTime::getDayStart(swCurTime));

  GameTimeCmd cmd;
  cmd.set_opt(EGAMETIMEOPT_SYNC);
  cmd.set_sec(((swDaySec + m_sdwGameTimeOffset) % rCFG.dwGameDaySec) * rCFG.dwTimeSpeed);
  cmd.set_speed(MiscConfig::getMe().getSystemCFG().dwTimeSpeed);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);

  XLOG << "[玩家-时间同步]" << accid << id << getProfession() << name << " syshour:" << swDaySec / 3600 << "gamehour:" << cmd.sec() / 3600 << "gamesec:" << cmd.sec() << XEND;
}

void SceneUser::watchLockEffect(DWORD curTime)
{
  if (curTime < m_dwLockTimeTick)
    return;
  const SLockEffectCFG& rCFG = MiscConfig::getMe().getBeLockCFG();

  DWORD gap = rCFG.dwRefreshInterval;
  gap = gap < 2 ? 2 : gap; // protect
  m_dwLockTimeTick = curTime + gap;

  DWORD num = getValidNumLockMe();
  if (num == m_dwLastLockMeNum)
    return;
  if (rCFG.haveEffect(num) == false && rCFG.haveEffect(m_dwLastLockMeNum) == false)
    return;
  m_dwLastLockMeNum = num;

  updateAttribute();
  refreshDataAtonce();
}

DWORD SceneUser::getValidNumLockMe()
{
  TSetQWORD tempDelIDs;
  DWORD num = 0;
  for (auto &q : m_setLockMeIDs)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(q);
    if (pNpc == nullptr)
    {
      tempDelIDs.insert(q);
      continue;
    }
    if (pNpc->m_ai.getStateType() != ENPCSTATE_ATTACK)
      continue;
    if (getDistance(getPos(), pNpc->getPos()) > 5)
      continue;
    num++;
  }
  for (auto &q : tempDelIDs)
  {
    m_setLockMeIDs.erase(q);
  }
  return num;
}

void SceneUser::lockMeCheckEmoji(const char *t)
{
  if (!t) return;
  if (m_setLockMeIDs.empty()) return;

  for (auto &it : m_setLockMeIDs)
  {
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(it);
    if (npc)
      npc->m_oEmoji.check(t);
  }
}

DWORD SceneUser::getWeaponType()
{
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_oPackage.getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return 0;
  ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_WEAPON);
  if (pEquip == nullptr)
    return 0;
  return pEquip->getType();
}

DWORD SceneUser::getEquipedItemNum(DWORD itemid)
{
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_oPackage.getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return 0;
  return pEquipPack->getEquipedItemNum(itemid);
}

DWORD SceneUser::getMainPackageItemNum(DWORD itemid)
{
  MainPackage* pMainPack = dynamic_cast<MainPackage*> (m_oPackage.getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
    return 0;
  return pMainPack->getItemCount(itemid);
}

void SceneUser::showSkill()
{
  if (m_pCurFighter == nullptr)
    return;

  if (randBetween(1,100) > 50)
  {
    DWORD skillid = randBetween(1,100) > 50 ? 800001 : 81001;

    setAttr(EATTRTYPE_ATK, 300);
    setAttr(EATTRTYPE_MATK, 300);
    setAttr(EATTRTYPE_CTCHANGE, -2);
    setAttr(EATTRTYPE_CTFIXED, -2);
    setAttr(EATTRTYPE_CASTSPD, 3);

    xPos pos = getPos();
    if (getScene())
      getScene()->getRandPos(getPos(), 3, pos);

    Cmd::SkillBroadcastUserCmd cmd;
    cmd.set_skillid(skillid);
    Cmd::PhaseData *pData = cmd.mutable_data();
    Cmd::ScenePos *p = pData->mutable_pos();
    p->set_x(pos.getX());
    p->set_y(pos.getY());
    p->set_z(pos.getZ());
    SceneFighter* pf = getFighter();
    if (pf != nullptr)
      pf->getSkill().addTempSkill(skillid, true);
    m_oSkillProcessor.addServerTrigSkill(skillid);
    m_oSkillProcessor.setActiveSkill(cmd);
    return;
  }

  EProfession pro = m_pCurFighter->getProfession() == EPROFESSION_NOVICE ? m_oUserSceneData.getDestProfession() : m_pCurFighter->getProfession();
  if (m_oTmpData.m_eProfession)
    pro = m_oTmpData.m_eProfession;
  const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(pro);
  if (pCFG == nullptr)
    return;
  TVecDWORD vecSkill;
  vecSkill.push_back(pCFG->normalSkill);
  vecSkill.push_back(pCFG->strengthSkill);
  for (auto &v : pCFG->vecEnableSkill)
  {
    for (auto &d : v.second)
    {
      vecSkill.push_back(d);
    }
  }
  DWORD size = vecSkill.size();
  if(size == 0)
    return;

  DWORD randIndex = randBetween(0, size - 1);
  DWORD showSkillID = vecSkill[randIndex];
  m_oSkillProcessor.showSkill(showSkillID);
}

void SceneUser::addHitMe(QWORD id, DWORD damage)
{
  // 20160801, 策划去除自动反击
  /*
  DWORD nowtime = now();
  //protect
  if (m_mapEnemy2HitTime.size() > 20)
    return;

  auto it = m_mapEnemy2HitTime.find(id);
  if (it == m_mapEnemy2HitTime.end())
  {
    const SLockEffectCFG& rCFG = MiscConfig::getMe().getBeLockCFG();
    if (damage != 0)
      m_mapEnemy2HitTime[id] = nowtime + rCFG.dwAutoHitTime;
    return;
  }
  // 被持续攻击了5s
  if (nowtime >= it->second)
  {
    // inform client
    UserAutoHitCmd cmd;
    cmd.set_charid(id);
    PROTOBUF(cmd, send, len);
    this->sendCmdToMe(send, len);
    m_mapEnemy2HitTime.erase(it);
    XLOG("[攻击目标],通知客户端攻击, 角色:%llu, 目标:%llu", this->id, id);
  }
  */
}

void SceneUser::beforeSendNine()
{
  /*
  if (getScene() == nullptr)
    return;
  const TSetDWORD& setShowNpc = m_oUserSceneData.getShowNpcs();
  if (setShowNpc.empty())
    return;

  std::list<SceneNpc *> list;
  for (auto &it : setShowNpc)
  {
    getScene()->getSceneNpcByBaseID(it, list);
  }

  if (list.empty())
    return;

  for (auto &iter : list)
  {
    iter->addEnableUser(this->id);
    iter->sendMeToNine();
  }
  */
}

void SceneUser::playDynamicExpression(EAvatarExpression eExpression)
{
  if (getStatus() == ECREATURESTATUS_DEAD)
  {
    XERR << "[玩家-播放表情]" << accid << id << getProfession() << name << "在死亡状态下播放表情" << eExpression << "被忽略" << XEND;
    return;
  }
  if (eExpression != EAVATAREXPRESSION_MIN)
  {
    const SAvatarCFG* pCFG = AvatarConfig::getMe().getAvatarCFG(eExpression);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-播放表情]" << accid << id << getProfession() << name << "播放表情" << eExpression << "不存在" << XEND;
      return;
    }

    DWORD dwRand = randBetween(0, 10000);
    if (dwRand > pCFG->dwRate)
      return;

    if (pCFG->dwAdventureSkill != 0)
    {
      SceneFighter* pFighter = getFighter(EPROFESSION_NOVICE);
      if (pFighter == nullptr || pFighter->getSkill().isSkillEnable(pCFG->dwAdventureSkill) == false)
        return;
    }
  }

  PlayExpressionChatCmd cmd;
  cmd.set_charid(id);
  cmd.set_expressionid(eExpression);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSessionUser(id, send, len);

  XLOG << "[玩家-表情播放]" << accid << id << getProfession() << name << "播放表情 :" << eExpression << "发往社交服处理" << XEND;
}

void SceneUser::addShowNpc(DWORD id, bool bShare /*= false*/)
{
  const set<DWORD>& setShowNpc = m_oUserSceneData.getShowNpcs();
  if (setShowNpc.find(id) != setShowNpc.end())
    return;
  m_oUserSceneData.addShowNpc(id, bShare);

  XLOG << "[ShowNpc], 设置npc ID = " << id << "对玩家 : " << this->id << "可见" << XEND;

  if (getScene() != nullptr)
  {
    std::list<SceneNpc *> npclist;
    getScene()->getSceneNpcByBaseID(id, npclist);
    Cmd::NtfVisibleNpcUserCmd npccmd;
    for (auto &iter : npclist)
    {
      iter->sendMeToNine();

      const SNpcCFG* pNpcCfg = NpcConfig::getMe().getNpcCFG(id);
      if(pNpcCfg != nullptr && pNpcCfg->strMapIcon.empty() == false)
      {
        getScene()->refreshVisibleNpc(npccmd, &(*iter));
      }
    }
    if(npccmd.npcs_size() > 0)
    {
      npccmd.set_type(1);
      PROTOBUF(npccmd, send, len);
      sendCmdToMe(send, len);
    }
  }
}

void SceneUser::delShowNpc(DWORD id)
{
  const set<DWORD>& setShowNpc = m_oUserSceneData.getShowNpcs();
  if (setShowNpc.find(id) == setShowNpc.end())
    return;
  m_oUserSceneData.delShowNpc(id);

  XLOG << "[ShowNpc], 设置npc ID = " << id << "对玩家 : " << this->id << "取消可见" << XEND;

  if (getScene() != nullptr)
  {
    std::list<SceneNpc *> npclist;
    getScene()->getSceneNpcByBaseID(id, npclist);

    Cmd::NtfVisibleNpcUserCmd npccmd;
    for (auto &iter : npclist)
    {
      // 当前地图该队玩家都不看见了
      if (iter->isVisableToSceneUser(this) == false)
      {
        // del
        if (iter->check2PosInNine(this))
          iter->delMeToUser(this);

        const SNpcCFG* pNpcCfg = iter->getCFG();
        if(pNpcCfg != nullptr && pNpcCfg->strMapIcon.empty() == false)
        {
          getScene()->refreshVisibleNpc(npccmd, &(*iter));
        }

        if (getTeamID() != 0)
        {
          const GTeam& rTeam = getTeam();
          for (auto &m : rTeam.getTeamMemberList())
          {
            const TeamMemberInfo& rMember = m.second;
            if (rMember.charid() == this->id)
              continue;
            SceneUser* pTeamer = SceneUserManager::getMe().getUserByID(rMember.charid());
            if (pTeamer == nullptr || pTeamer->check2PosInNine(iter) == false)
              continue;
            iter->delMeToUser(pTeamer);
          }
        }
      }
    }
    if(npccmd.npcs_size() > 0)
    {
      npccmd.set_type(0);
      PROTOBUF(npccmd, send, len);
      sendCmdToMe(send, len);
    }
  }
}

// 点唱机
void SceneUser::setMusicData(DWORD dwMusicID, DWORD dwStartTime, bool bDemand, xPos pos /*=xPos(0,0,0)*/, bool bLoop/* = false*/)
{
  if (m_dwMusicID != dwMusicID)
  {
    m_dwMusicID = dwMusicID;
    setDataMark(EUSERDATATYPE_MUSIC_CURID);
    refreshDataAtonce();
    XDBG << "[场景玩家-点唱机] " << accid << id << getProfession() << name << " 收到 musicid:" << m_dwMusicID << XEND;
  }

  if (m_dwStartTime  != dwStartTime)
  {
    m_dwStartTime = dwStartTime;
    setDataMark(EUSERDATATYPE_MUSIC_START);
    refreshDataAtonce();
    XDBG << "[场景玩家-点唱机] " << accid << id << getProfession() << name << " 收到 starttime:" << m_dwStartTime << XEND;
  }

  if (m_bDemand != bDemand)
  {
    m_bDemand = bDemand;
    setDataMark(EUSERDATATYPE_MUSIC_DEMAND);
    refreshDataAtonce();
    XDBG << "[场景玩家-点唱机] " << accid << id << getProfession() << name << " 收到 点播人:" << m_bDemand << XEND;
  }

  if (m_bLoop != bLoop)
  {
    m_bLoop = bLoop;
    setDataMark(EUSERDATATYPE_MUSIC_LOOP);
    refreshDataAtonce();
    XDBG << "[场景玩家-点唱机] " << accid << id << getProfession() << name << " 收到 循环播放:" << m_bLoop << XEND;
  }

  m_oMusicPos = pos;

  checkMusicRecover(now(), true);
}

bool SceneUser::isInMusicNpcRange()
{
  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();

  if (m_oMusicPos.empty())
    return false;

  if (getDistance(getPos(), m_oMusicPos) > (float)rCFG.dwMusicRange)
  {
    return false;
  }
  return true;
}

void SceneUser::antiAddictRefresh(bool clear /*=false*/)
{
  if (clear)
  {//daily clear
    m_oUserSceneData.setBattleTime(0);
    m_oUserSceneData.setReBattleTime(0);
    m_oUserSceneData.setUsedBattleTime(0);
    m_oUserSceneData.setTutorBattleTime(0);
    m_oUserSceneData.setUsedTutorBattleTime(0);
    m_oVar.setVarValue(EVARTYPE_ANTI_ADDICT_DAILY, 1);
    m_oBuff.onBattleStatusChange();
    setAddictTipsTime(0);
    XLOG << "[防沉迷-清空]，charid：" << id << "清空沉迷时间" << XEND;
  }
  if (m_oVar.getVarValue(EVARTYPE_ANTI_ADDICT_DAILY) == 0)
  {
    DWORD daymax = getOneDayBattleTime();
    DWORD lastdaytotal = m_oUserSceneData.getTotalBattleTime();

    DWORD lastdayused = m_oUserSceneData.getBattleTime();
    DWORD lastdayspare = lastdaytotal > lastdayused ? lastdaytotal - lastdayused : 0;

    DWORD onlinetime = m_oUserSceneData.getOnlineTime();
    DWORD offlinetime = m_oUserSceneData.getOfflineTime();
    bool isSameDay = m_oVar.getVarDayNum(EVARTYPE_ANTI_ADDICT_DAILY, onlinetime, now()) == 0; // 上线时间与当前时间是同一个游戏日
    if (isSameDay && offlinetime != 0 && onlinetime > offlinetime)
    {
      if (offlinetime != 0 && lastdaytotal != 0)
      {
        DWORD varDay = m_oVar.getVarDayNum(EVARTYPE_ANTI_ADDICT_DAILY, offlinetime, onlinetime);
        varDay = varDay > 0 ? varDay - 1 : 0;
        lastdayspare += varDay * daymax;
      }
    }
    DWORD maxextra = MiscConfig::getMe().getSystemCFG().dwMaxExtraAddBattleTime;
    lastdayspare = lastdayspare > maxextra ? maxextra : lastdayspare;
    m_oUserSceneData.setTotalBattleTime(lastdayspare + daymax);

    // msg 提示
    /*MsgManager::sendMsg(id, 2515, MsgParams(300)); // 恢复300分钟提示
    if (m_oDeposit.hasMonthCard())
    {
      DWORD extraTime = m_oDeposit.getAddcitTime(0) / 60;  //分钟
      MsgManager::sendMsg(id, 2514, MsgParams(extraTime));
    }
    */
    if (lastdayspare != 0)
      MsgManager::sendMsg(id, 2516, MsgParams(lastdayspare / 60, m_oUserSceneData.getTotalBattleTime() / 60));

    m_oUserSceneData.setBattleTime(0);
    m_oUserSceneData.setReBattleTime(0);
    m_oUserSceneData.setUsedBattleTime(0);
    m_oUserSceneData.setTutorBattleTime(0);
    m_oUserSceneData.setUsedTutorBattleTime(0);
    m_oVar.setVarValue(EVARTYPE_ANTI_ADDICT_DAILY, 1);
    m_oBuff.onBattleStatusChange();
    setAddictTipsTime(0);
    XLOG << "[防沉迷-清空]，charid：" << id << "清空沉迷时间" << XEND;
  }

  if (m_oUserSceneData.getTotalBattleTime() == 0)
    m_oUserSceneData.setTotalBattleTime(getOneDayBattleTime());
}

DWORD SceneUser::getAddictTime()
{
  DWORD addictTime = getBattleTime();

  if (addictTime > m_oUserSceneData.getTotalBattleTime())
  {
    if (m_oUserSceneData.getReBattleTime() > m_oUserSceneData.getUsedBattleTime())
    {
      return m_oUserSceneData.getUsedBattleTime();
    }
    if (m_oUserSceneData.getTutorBattleTime() > m_oUserSceneData.getUsedTutorBattleTime())
    {
      return m_oUserSceneData.getUsedTutorBattleTime();
    }
  }
  return addictTime;
}

DWORD SceneUser::getOneDayBattleTime()
{
  return  LuaManager::getMe().call<DWORD>("getAddictTipsTime", this);
}

void SceneUser::addcitTips(bool isLogin)
{
  //attict tips
  DWORD dwConfigSec = m_oUserSceneData.getTotalBattleTime();

  if (getAddictTipsTime() < dwConfigSec)
  {
    setAddictTipsTime(dwConfigSec);
  }

  if (isLogin && getBattleTime() >= dwConfigSec)
  {
    DWORD dwBattleMin = (getBattleTime() / 60);
    MsgManager::sendMsg(id, 2511, MsgParams(dwBattleMin));
    XLOG << "[防沉迷-登录提示]，charid：" << id << "沉迷时间：" << getBattleTime() << "秒， 下次提示要超过时间：" << getAddictTipsTime() << "battleHour：" << dwBattleMin << "configHour：" << dwConfigSec/60 << XEND;

    return;
  }

  if (!isLogin && getBattleTime() >= getAddictTipsTime())
  {
    DWORD dwBattleMin = (getBattleTime() / 60);
    DWORD dwConfigMin = dwConfigSec / 60;  //首次提示时间
    const DWORD interval = 60 * 60; //提示间隔
    MsgManager::sendMsg(id, 2517, MsgParams(dwBattleMin));

    //月卡额外提供[63cd4e]%s[-]分钟战斗时长。
    /*if (m_oDeposit.hasMonthCard())
    {
      DWORD extraTime = m_oDeposit.getAddcitTime(0) / 60;  //分钟
      MsgManager::sendMsg(id, 2514, MsgParams(extraTime));
    }
    */

    DWORD hour = getBattleTime() / interval;
    hour++;
    setAddictTipsTime(hour * interval);
    XLOG << "[防沉迷-提示]，charid：" << id << "沉迷时间：" << getBattleTime () << "秒， 下次提示要超过时间：" << getAddictTipsTime() << "battleHour：" << dwBattleMin << "configHour：" << dwConfigMin << XEND;
  }
}

bool SceneUser::checkAddict(ENpcType npcType)
{
  if (getScene() == nullptr)
    return false;

  if (!getScene()->isSScene())
    return false;

  if (npcType == ENPCTYPE_MINIBOSS || npcType == ENPCTYPE_MVP)
    return false;
  return true;
}

//void SceneUser::setNextAddictTipsTime(DWORD time)
//{
//  const DWORD interval = 60*60; //提示间隔
//  DWORD hour = time / interval;
//  hour++;
//  setAddictTipsTime(hour*interval);
//}

void SceneUser::sendGuildInfoToNine()
{
  GuildInfoNtf cmd;
  cmd.set_charid(id);
  cmd.set_id(m_oGuild.id());
  cmd.set_name(m_oGuild.name());
  cmd.set_icon(m_oGuild.portrait());
  cmd.set_job(m_oGuild.jobname());

  PROTOBUF(cmd, send, len);
  sendCmdToNine(send, len);
}

void SceneUser::sendGuildCity()
{
  if (getScene() == nullptr)
  {
    XDBG << "[玩家-公会城池]" << accid << id << getProfession() << name << "场景未初始化,未能收到公会城池信息" << XEND;
    return;
  }

  vector<GuildCityInfo> vecInfo;
  GuildCityManager::getMe().collectCityInfo(getScene()->getMapID(), vecInfo);
  if (vecInfo.empty() == true)
    return;

  QueryGuildCityInfoGuildCmd cmd;
  for (auto &v : vecInfo)
    cmd.add_infos()->CopyFrom(v);

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  XDBG << "[玩家-公会城池]" << accid << id << getProfession() << name << "收到公会城池信息" << cmd.ShortDebugString() << XEND;
}

DWORD SceneUser::getGuildPrayTimes()
{
  DWORD dwTimes = 0;
  const GuildUserInfo& rInfo = m_oGuild.getGuildUserInfo();
  if(rInfo.prays_size() == 0)
    return dwTimes;
  for(int i = 0; i < rInfo.prays_size(); ++i)
  {
    const SGuildPrayCFG* pCFG = GuildConfig::getMe().getGuildPrayCFG(rInfo.prays(i).pray());
    if(pCFG != nullptr && pCFG->eType == EPRAYTYPE_GODDESS && rInfo.prays(i).lv() != 0)
      ++dwTimes;
  }
  return dwTimes;
}

void SceneUser::addTeamSkillTimePoint(DWORD curtime)
{
  addSkillTimePoint(curtime);
  if (getTeamID() == 0)
    return;
  std::set<SceneUser*> others = getTeamRewardUsers();
  others.erase(this);
  for (auto &s : others)
  {
    s->addSkillTimePoint(curtime);
  }
}

DWORD SceneUser::getSkillLv(DWORD skillGroupID)
{
  if (m_pCurFighter == nullptr)
    return 0;
  return m_pCurFighter->getSkill().getSkillLv(skillGroupID * ONE_THOUSAND);
}

DWORD SceneUser::getRandomNum()
{
  if (m_dwRandIndex <= 0 || m_dwRandIndex > m_vecRandomNums.size() * 5)
    m_dwRandIndex = 1;

  // get value
  DWORD groupValue = m_vecRandomNums[(m_dwRandIndex-1) / 5];
  DWORD key = m_dwRandIndex - m_dwRandIndex / 5 * 5;
  key = key != 0 ? key : 5;
  DWORD value = (groupValue / (DWORD)pow(100, 5-key)) % 100;

  // set index
  m_dwRandIndex ++;
  if (m_dwRandIndex > MAX_RANDOM_FOR_SKILL * 5) m_dwRandIndex = 1;

  return value;
}

void SceneUser::setRandIndex(DWORD index)
{
  m_dwRandIndex = (m_dwRandIndex == 0 ? 1 : m_dwRandIndex); // 初始化

  DWORD oriindex = index;
  if (index <= 0 || index > m_vecRandomNums.size() * 5)
  {
    XERR << "[技能-随机数], 非法的客户端, 玩家:" << name << id << "职业:" << getProfession() << "随机数:" << index << XEND;
    index = m_dwRandIndex;
  }

  // index: 客户端游标,  m_dwRandIndex: 服务端游标, m_dwClientLastIndex: 客户端上次发送的游标
  if (m_dwRandIndex != index)
  {
    if (m_dwClientLastIndex >= index && m_dwClientLastIndex - index < MAX_RANDOM_FOR_SKILL * 5 / 2)
    {
      // 不合法时, 以服务端为准
      XERR << "[技能-随机数], 随机数异常, 玩家:" << name << id << "职业:" << getProfession() << "随机数:" << m_dwRandIndex << index << m_dwClientLastIndex << XEND;
      index = m_dwRandIndex;
    }
    if (m_dwRandIndexErrCnt++ >= 5)
    {
      // 低概率出现
      XERR << "[技能-随机数], 随机数连续多次不合法, 玩家:" << name << id << "职业:" << getProfession() << "随机数:" << m_dwRandIndex << index << m_dwClientLastIndex << XEND;
    }
  }
  else
  {
    m_dwRandIndexErrCnt = 0;
  }

  // check update
  {
    if (index > MAX_RANDOM_FOR_SKILL / 2 * 5 && m_dwClientLastIndex <= MAX_RANDOM_FOR_SKILL / 2 * 5)
      m_bUpdateAGroup = true;
    if (index < MAX_RANDOM_FOR_SKILL / 2 * 5 && m_dwClientLastIndex > MAX_RANDOM_FOR_SKILL * 5)
      m_bUpdateBGroup = true;
    updateRandom();
  }

  m_dwRandIndex = index;
  m_dwClientLastIndex = oriindex;
}

void SceneUser::createRandom(bool AGroup, bool BGroup)
{
  if (!AGroup && !BGroup)
    return;
  // if empty, do init
  if (m_vecRandomNums.size() < MAX_RANDOM_FOR_SKILL)
  {
    for (DWORD i = 0; i < MAX_RANDOM_FOR_SKILL; ++i)
      m_vecRandomNums.push_back(0);
  }
  auto produce = [this]() -> DWORD
  {
    DWORD value = 0;
    for (int i = 4; i >= 0; i--)
    {
      DWORD rand = randBetween(0, 99);
      if (rand == 0 && i == 4) rand = 1;
      value += (pow(100, i) * rand);
    }
    return value;
  };
  if (AGroup)
  {
    for (DWORD i = 0; i < MAX_RANDOM_FOR_SKILL / 2; ++i)
      m_vecRandomNums[i] = produce();
  }
  if (BGroup)
  {
    for (DWORD i = MAX_RANDOM_FOR_SKILL / 2; i < MAX_RANDOM_FOR_SKILL; ++i)
      m_vecRandomNums[i] = produce();
  }
}

void SceneUser::sendRandom(bool AGroup, bool BGroup)
{
  UpdateRandomUserEvent cmd;

  if (m_vecRandomNums.size() != MAX_RANDOM_FOR_SKILL)
    return;
  if (!AGroup && !BGroup)
    return;

  DWORD begin = 0;
  DWORD end = 0;
  if (AGroup)
  {
    begin = 1;
    end = MAX_RANDOM_FOR_SKILL / 2;
    for (DWORD i = 0; i < MAX_RANDOM_FOR_SKILL / 2; ++i)
      cmd.add_randoms(m_vecRandomNums[i]);
  }
  if (BGroup)
  {
    begin = AGroup ? begin : MAX_RANDOM_FOR_SKILL / 2 + 1;
    end = MAX_RANDOM_FOR_SKILL;
    for (DWORD i = MAX_RANDOM_FOR_SKILL / 2; i < MAX_RANDOM_FOR_SKILL; ++i)
      cmd.add_randoms(m_vecRandomNums[i]);
  }
  cmd.set_beginindex(begin);
  cmd.set_endindex(end);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

void SceneUser::updateRandom()
{
  if (!m_bUpdateAGroup && !m_bUpdateBGroup)
    return;
  createRandom(m_bUpdateAGroup, m_bUpdateBGroup);
  sendRandom(m_bUpdateAGroup, m_bUpdateBGroup);
  m_bUpdateAGroup = false;
  m_bUpdateBGroup = false;
}

void SceneUser::addBattleTime(DWORD timelen, bool bExtra /*= true*/)
{
  m_oShare.addNormalData(ESHAREDATATYPE_S_BATTLETIME, timelen);

  bool bEnough = m_oUserSceneData.haveEnoughBattleTime();

  DWORD usedBattleTime = m_oUserSceneData.getUsedBattleTime();
  DWORD reBattleTime = m_oUserSceneData.getReBattleTime();
  DWORD usedtutorbattletime = m_oUserSceneData.getUsedTutorBattleTime();
  DWORD tutorbattletime = m_oUserSceneData.getTutorBattleTime();
  if (reBattleTime != 0 && usedBattleTime < reBattleTime)
  {
    DWORD retimelen = (reBattleTime - usedBattleTime < timelen ? reBattleTime - usedBattleTime : timelen);
    //timelen = (reBattleTime - usedBattleTime < timelen ? reBattleTime - usedBattleTime : timelen);
    m_oUserSceneData.setUsedBattleTime(retimelen + usedBattleTime);

    if (timelen > retimelen)
      m_oUserSceneData.setBattleTime(m_oUserSceneData.getBattleTime() + timelen - retimelen);
  }
  else if (tutorbattletime != 0 && usedtutorbattletime < tutorbattletime)
  {
    DWORD retimelen = tutorbattletime - usedtutorbattletime < timelen ? tutorbattletime - usedtutorbattletime : timelen;
    m_oUserSceneData.setUsedTutorBattleTime(retimelen + usedtutorbattletime);

    if (timelen > retimelen)
      m_oUserSceneData.setBattleTime(m_oUserSceneData.getBattleTime() + timelen - retimelen);
  }
  else
  {
    m_oUserSceneData.setBattleTime(m_oUserSceneData.getBattleTime() + timelen);
    m_oAchieve.onBattleTime(timelen);
  }

  if (bExtra)
  {
    if (m_bHasHitFieldMonster)
    {
      m_bHasHitFieldMonster = false;
      if (bEnough)
        m_oBuff.onAddBattleTime(timelen);
    }

    if (bEnough && m_oUserSceneData.haveEnoughBattleTime() == false)
      m_oBuff.onBattleStatusChange();
  }

  m_oServant.onFinishEvent(ETRIGGER_BATTLE_TIME, timelen);
  getExchangeShop().onAddBattleTime();
}

bool SceneUser::addReBattleTime(DWORD timelen)
{
  DWORD maxReturnTime = MiscConfig::getMe().getSystemCFG().dwMaxMusicBattleTime;
  DWORD returnedTime = m_oUserSceneData.getReBattleTime();
  if (returnedTime >= maxReturnTime)
    return false;

  bool bEnough = m_oUserSceneData.haveEnoughBattleTime();

  timelen = (maxReturnTime - returnedTime < timelen ? maxReturnTime - returnedTime : timelen);
  m_oUserSceneData.setReBattleTime(timelen + returnedTime);

  if (!bEnough)
    m_oBuff.onBattleStatusChange();
  return true;
}

void SceneUser::checkMusicRecover(DWORD curTime, bool atonce /*=false*/)
{
  if (!atonce && curTime < m_dwMusicTimeTick)
    return;

  if (m_dwMusicID == MiscConfig::getMe().getJoyLimitCFG().dwMusicID)
    return;

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();

  m_dwMusicTimeTick = curTime + rCFG.dwMusicCheckTime;

  // 无音乐在播, 或者未在范围
  const xPos dressupPos = MiscConfig::getMe().getDressStageCFG().m_pMusicPos;
  bool bOutDressupPos = getDistance(getPos(), dressupPos) > (float)rCFG.dwMusicRange;
  if (!m_dwMusicID || !m_dwStartTime ||
      (getDistance(getPos(), m_oMusicPos) > (float)rCFG.dwMusicRange && bOutDressupPos == true))
  {
    if (m_dwListenBeginTime != 0)
    {
      m_dwListenBeginTime = 0;
      m_oBuff.del(rCFG.dwMusicBuff);
    }
    return;
  }

  if (m_oUserSceneData.getReBattleTime() < rCFG.dwMaxMusicBattleTime)
  {
    // 添加显示buff图标
    m_oBuff.add(rCFG.dwMusicBuff);

    // 设置开始听音乐时间
    if (m_dwListenBeginTime == 0)
    {
      m_dwListenBeginTime = curTime;
      return;
    }

    // 恢复战斗时间
    if (m_dwListenBeginTime + rCFG.dwMusicContinueTime <= curTime)
    {
      if (addReBattleTime(rCFG.dwMusicReturnTime))
        MsgManager::sendMsg(id, 451);
      m_dwListenBeginTime = curTime;

      m_dwMusicTime += rCFG.dwMusicContinueTime;
      m_oQuest.onMusic(m_dwMusicTime);
      m_oServant.onFinishEvent(ETRIGGER_LISTEN_MUSIC);
      m_oServant.onGrowthFinishEvent(ETRIGGER_LISTEN_MUSIC);
    }
  }
}

const BaseSkill* SceneUser::getLearnedSkillByID(DWORD skillid, bool bJustNormal /*=false*/)
{
  bool bFind = false;
  for (auto &p : m_vecFighters)
  {
    if (p == nullptr)
      continue;
    if (p->getSkill().isSkillEnable(skillid))
    {
      bFind = true;
      break;
    }
  }

  if (bJustNormal)
  {
    bFind = false;
    for (auto &p : m_vecFighters)
    {
      const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(p->getProfession());
      if (pCFG == nullptr)
        return nullptr;
      if (pCFG->haveSkill(skillid) || pCFG->normalSkill == skillid || pCFG->strengthSkill == skillid)
      {
        bFind = true;
        break;
      }
    }
  }

  return bFind ? SkillManager::getMe().getSkillCFG(skillid) : nullptr;
}

bool SceneUser::jumpZone(QWORD qwNpcID, DWORD dwLine)
{
  // check same zoneid
  if (getClientZoneID(thisServer->getZoneID()) == dwLine)
  {
    XERR << "[玩家-切线]" << accid << id << getProfession() << name << "进行本线" << thisServer->getZoneID() << "切线" << XEND;
    return false;
  }

  // check npc
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (pNpc == nullptr)
  {
    XERR << "[玩家-切线]" << accid << id << getProfession() << name << "请求切换到" << thisServer->getZoneID() << "线,未发现切线npc :" << qwNpcID << XEND;
    return false;
  }

  const SZoneMiscCFG& rCFG = MiscConfig::getMe().getZoneCFG();

  // check return guild zone
  if (getClientZoneID(m_oGuild.zoneid()) != dwLine)
  {
    // get zone status
    EZoneStatus eStatus = ChatManager_SC::getMe().getZoneStatus(getServerZoneID(thisServer->getZoneID(), dwLine));
    if (eStatus == EZONESTATUS_MIN)
    {
      XERR << "[玩家-切线]" << accid << id << getProfession() << name << "没有找到分线" << thisServer->getZoneID() << "状态" << XEND;
      return false;
    }

    // check resource
    if (m_oVar.getVarValue(EVARTYPE_FIRST_EXCHANGEZONE) != 0)
    {
      BasePackage* pMainPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
      if (pMainPack == nullptr)
        return false;

      const TVecItemInfo& vecCost = rCFG.getJumpCost(eStatus);
      if (vecCost.empty() == true || pMainPack->checkItemCount(vecCost) == false)
      {
        XERR << "[玩家-切线]" << accid << id << getProfession() << name << "请求切换到" << thisServer->getZoneID() << "线,未持有足够的资金:" << XEND;
        return false;
      }
    }
  }

  // accept action quest
  m_oQuest.abandonGroup(rCFG.dwActionQuest);
  if (m_oQuest.acceptQuest(rCFG.dwActionQuest, true) == false)
  {
    XERR << "[玩家-切线]" << accid << id << getProfession() << name << "请求切换到" << thisServer->getZoneID() << "线,未能接取表现人物 quest :" << rCFG.dwActionQuest << XEND;
    return false;
  }

  setTmpJumpZone(dwLine);
  XLOG << "[玩家-切线]" << accid << id << getProfession() << name << "开始准备切换到" << dwLine << "线 zoneid :" << dwLine << XEND;
  return true;
}

void SceneUser::sendItemImage(bool broadcast)
{
  if (!getTeamID())
    return;
  
  std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ITEMIMAGE, getTeamID(), thisServer->getZoneID(), "user");
  QWORD qwUserId = 0;
  RedisManager::getMe().getData(key, qwUserId);
  
  ItemImageUserNtfUserCmd cmd;
  cmd.set_userid(qwUserId);
  PROTOBUF(cmd, send, len);
  if (!broadcast)
  {
    sendCmdToMe(send, len);
    return;
  }
  const GTeam& rTeam = getTeam();
  for (auto &m : rTeam.getTeamMemberList())
  {
    const TeamMemberInfo& memInfo = m.second;
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(memInfo.charid());
    if(pUser && pUser->getDressUp().getDressUpStatus() != 0)
      continue;
    thisServer->sendCmdToMe(memInfo.charid(), send, len);
  }
}

void SceneUser::addAttackMe(QWORD guid, DWORD time)
{
  if (m_vecTime2Attacker.size() > 20)
    return;

  auto it = find_if(m_vecTime2Attacker.begin(), m_vecTime2Attacker.end(), [guid](const pair<DWORD, QWORD>& pa) -> bool
  {
    return guid == pa.second;
  });
  if (it != m_vecTime2Attacker.end())
  {
    it->first = time;
    return;
  }
  m_vecTime2Attacker.push_back(pair<DWORD, QWORD> (time, guid));
  XDBG << "[成就]id" << guid << "打我了" << XEND;
  m_oAchieve.onMonsterDraw(xTime::getCurSec());
}

bool SceneUser::isAttackedMe(QWORD qwID)
{
  auto v = find_if(m_vecTime2Attacker.begin(), m_vecTime2Attacker.end(), [qwID](const pair<DWORD, QWORD>& r) -> bool{
    return r.second == qwID;
  });
  return v != m_vecTime2Attacker.end();
}

const string& SceneUser::getTeamerName(QWORD id)
{
  static const string name = "";
  for (auto &m : getTeam().getTeamMemberList())
  {
    if (id == m.second.charid())
    {
      return m.second.name();
    }
  }
  return name;
}

void SceneUser::getHelpReward(const std::set<SceneUser*>& helpSet, EHelpType eType, bool bSelf /*= false*/, DWORD specFriendCnt /*=0*/, DWORD specGuildCnt /*=0*/)
{
  auto findFriend = [&]() -> bool {
    for (auto &s : helpSet)
    {
      if (s == nullptr)
        return false;
      if (s == this)
        continue;
      if (m_oSocial.checkRelation(s->id, ESOCIALRELATION_FRIEND))
        return true;
    }
    return false;
  };

  auto findGuild = [&]() -> bool {
    if (m_oGuild.id() == 0)
      return false;
    for (auto &s : helpSet)
    {
      if (s == nullptr)
        return false;
      if (s == this)
        continue;
      if (s->getGuild().id() == getGuild().id())
        return true;
    }
    return false;
  };

  bool haveGuild = findGuild();
  bool haveFriend = haveGuild ? true : findFriend();

  const SFriendShipCFG& rCFG = MiscConfig::getMe().getFriendShipCFG();
  if (haveFriend)
  {
    DWORD var = getVar().getVarValue(EVARTYPE_FRIENDSHIP_FRIEND);
    const SFriendShipReward* friCFG = rCFG.getReward(EVARTYPE_FRIENDSHIP_FRIEND);
    if (friCFG != nullptr)
    {
      DWORD rewardCnt = friCFG->getCountByType(eType);
      if (specFriendCnt) rewardCnt = specFriendCnt;

      if (rewardCnt != 0)
      {
        if (var >= friCFG->dwMaxLimitCount)
        {
          MsgManager::sendMsg(id, 432);
          XLOG << "[Help-Reward], 帮助好友完成,帮助类型:" << eType << " 队伍:" << getTeamID() << "玩家:" << name << id << "本周奖励已达上限" << XEND;
        }
        else
        {
          DWORD oldCnt = 0;
          BasePackage* pMainPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
          BasePackage* pTempPack = m_oPackage.getPackage(EPACKTYPE_TEMP_MAIN);
          if (pMainPack != nullptr) oldCnt += pMainPack->getItemCount(friCFG->dwItemId);
          if (pTempPack != nullptr) oldCnt += pTempPack->getItemCount(friCFG->dwItemId);

          DWORD count = rewardCnt > friCFG->dwMaxLimitCount - var ? friCFG->dwMaxLimitCount - var : rewardCnt;
          ItemInfo stInfo;
          stInfo.set_id(friCFG->dwItemId);
          stInfo.set_count(count);
          stInfo.set_source(ESOURCE_HELP);
          getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);

          DWORD newCnt = 0;
          if (pMainPack != nullptr) newCnt += pMainPack->getItemCount(friCFG->dwItemId);
          if (pTempPack != nullptr) newCnt += pTempPack->getItemCount(friCFG->dwItemId);

          if (newCnt >= oldCnt)
          {
            getVar().setVarValue(EVARTYPE_FRIENDSHIP_FRIEND, var + newCnt - oldCnt);
            count = newCnt - oldCnt;
          }
          else
          {
            XERR << "[Help-Reward], 帮助好友完成,帮助类型:" << eType << ", 队伍:" << getTeamID() << "玩家:" << name << id << "获得奖励:" << stInfo.id() << stInfo.count()
                 << "newCnt < oldCnt" << newCnt << oldCnt << XEND;
          }
          MsgManager::sendMsg(id, 430, MsgParams(count));
          XLOG << "[Help-Reward], 帮助好友完成,帮助类型:" << eType << ", 队伍:" << getTeamID() << "玩家:" << name << id << "获得奖励:" << stInfo.id() << stInfo.count()
               << count << oldCnt << newCnt << XEND;
        }
      }
    }
  }
  if (!bSelf && haveGuild)
  {
    DWORD var = getVar().getVarValue(EVARTYPE_FRIENDSHIP_GUILD);
    const SFriendShipReward* friCFG = rCFG.getReward(EVARTYPE_FRIENDSHIP_GUILD);
    if (friCFG != nullptr)
    {
      DWORD rewardCnt = friCFG->getCountByType(eType);
      if (specGuildCnt) rewardCnt = specGuildCnt;

      if (rewardCnt != 0)
      {
        if (var >= friCFG->dwMaxLimitCount)
        {
          MsgManager::sendMsg(id, 433);
          XLOG << "[Help-Reward], 帮助公会成员完成,帮助类型:" << eType << " 公会:" << m_oGuild.id() << "玩家:" << name << id << "本周奖励已达上限" << XEND;
        }
        else
        {
          DWORD count = rewardCnt > friCFG->dwMaxLimitCount - var ? friCFG->dwMaxLimitCount - var : rewardCnt;
          ItemInfo stInfo;
          stInfo.set_id(friCFG->dwItemId);
          stInfo.set_count(count);
          stInfo.set_source(ESOURCE_HELP);
          getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);
          getVar().setVarValue(EVARTYPE_FRIENDSHIP_GUILD, var + count);
          MsgManager::sendMsg(id, 430, MsgParams(count));
          XLOG << "[Help-Reward], 帮助公会成员完成,帮助类型:" << eType << " 公会:" << m_oGuild.id() << "玩家:" << name << id << "获得奖励:" << stInfo.id() << stInfo.count() << XEND;
        }
      }
    }
  }
}

void SceneUser::addExpLog(DWORD expId, DWORD count, DWORD after, ESource source)
{
  QWORD eid = xTime::getCurUSec();
  PlatLogManager::getMe().gainItemLog(thisServer,
    getUserSceneData().getPlatformId(),
    getZoneID(),
    accid,
    id,
    eid,
    getUserSceneData().getCharge(),
    expId,
    count,
    after,
    source);

  /*DWORD eventType = PlatLogManager::getMe().EventTypeConvert(source);
  PlatLogManager::getMe().eventLog(thisServer,
    getUserSceneData().getPlatformId(),
    getZoneID(),
    accid,
    id,
    eid,
    getUserSceneData().getCharge(), eventType, 0, 1);*/

}

void SceneUser::reduceExpLog(DWORD expId, DWORD  decCount, DWORD after, ESource source)
{
  QWORD eid = xTime::getCurUSec();
  PlatLogManager::getMe().consumeItemLog(thisServer,
    getUserSceneData().getPlatformId(),
    getZoneID(),
    accid,
    id,
    eid,
    getUserSceneData().getCharge(),
    expId,/*itemid */
    decCount, after,
    source
  );

  //DWORD eventType = PlatLogManager::getMe().EventTypeConvert(source);
  //PlatLogManager::getMe().eventLog(thisServer,
  //  getUserSceneData().getPlatformId(),
  //  getZoneID(),
  //  accid,
  //  id,
  //  eid,
  //  getUserSceneData().getCharge(), eventType, 0, 1);
}

/*void SceneUser::setGChar()
{
  GCharWriter gChar(thisServer->getRegionID(), id);

  gChar.setName(name);
  gChar.setAccID(accid);
  gChar.setMapID(m_oUserSceneData.getOnlineMapID());
  gChar.setGender(m_oUserSceneData.getGender());
  gChar.setProfession(getProfession());
  gChar.setBaseLevel(m_oUserSceneData.getRolelv());
  gChar.setOfflineTime(getUserState() != USER_STATE_QUIT ? 0 : m_oUserSceneData.getOfflineTime());
  gChar.setHair(m_oHair.getCurHair());
  gChar.setHairColor(m_oHair.getCurHairColor());
  gChar.setLeftHand(m_oUserSceneData.getLefthand());
  gChar.setRightHand(m_oUserSceneData.getRighthand());
  gChar.setBody(m_oUserSceneData.getBody());
  gChar.setHead(m_oUserSceneData.getHead());
  gChar.setBack(m_oUserSceneData.getBack());
  gChar.setFace(m_oUserSceneData.getFace());
  gChar.setTail(m_oUserSceneData.getTail());
  gChar.setEye(m_oUserSceneData.getEye());
  gChar.setZoneID(m_oUserSceneData.getZoneID());
  gChar.setPortrait(m_oPortrait.getCurPortrait());
  gChar.setClothColor(m_oUserSceneData.getClothColor());
  gChar.setBlink(m_oUserSceneData.getBlink());
  gChar.setManualLv(m_oManual.getManualLv());
  gChar.setManualExp(m_oManual.getManualPoint());
  gChar.setTitleID(m_oTitle.getCurTitle(ETITLE_TYPE_MANNUAL));
  gChar.setQueryType(m_oUserSceneData.getQueryType());

  gChar.save();
  XLOG << "[玩家-全局玩家]" << accid << id << getProfession() << name << "更新全局数据成功" << XEND;
}*/

// 仅处理逻辑, 不改变实际血量
void SceneUser::onHealMe(xSceneEntryDynamic* entry, DWORD healhp)
{
  if (!entry)
    return;
  /*
  if (!m_mapMvpID2Damage.empty())
  {
    bool isHealSelf = entry->id == id;
    for (auto m = m_mapMvpID2Damage.begin(); m != m_mapMvpID2Damage.end(); )
    {
      DWORD validhp = (m->second > healhp ? healhp : m->second);
      m->second -= validhp;

      if (entry->getEntryType() == SCENE_ENTRY_USER)
      {
        SceneNpc* pMvp = SceneNpcManager::getMe().getNpcByTempID(m->first);
        if (pMvp && pMvp->getScene() == getScene())
        {
          pMvp->addMvpRelaHealHp(entry->id, validhp, isHealSelf);
          XDBG << "[玩家-回血-MVP], 玩家" << name << id << "回血量:" << validhp;
        }
      }

      if (m->second == 0)
      {
        m = m_mapMvpID2Damage.erase(m);
        continue;
      }
      ++m;
    }
  }
  */
}

// 仅处理逻辑, 不改变实际血量
void SceneUser::onSkillHealMe(xSceneEntryDynamic* entry, DWORD healhp)
{
  if (!entry)
    return;
  if (!m_mapMvpID2Damage.empty() && healhp != 0)
  {
    bool isHealSelf = entry->id == id;
    for (auto m = m_mapMvpID2Damage.begin(); m != m_mapMvpID2Damage.end(); )
    {
      DWORD validhp = (m->second > healhp ? healhp : m->second);
      m->second -= validhp;

      if (entry->getEntryType() == SCENE_ENTRY_USER)
      {
        SceneNpc* pMvp = SceneNpcManager::getMe().getNpcByTempID(m->first);
        if (pMvp && pMvp->getScene() == getScene())
        {
          pMvp->addMvpRelaHealHp(entry->id, validhp, isHealSelf);
          XDBG << "[玩家-回血-MVP], 玩家" << name << id << "回血量:" << validhp << XEND;
        }
      }

      if (m->second == 0)
      {
        m = m_mapMvpID2Damage.erase(m);
        continue;
      }
      ++m;
    }
  }
  if (entry->getEntryType() == SCENE_ENTRY_USER && entry->id != this->id)
  {
    for (auto &q : m_setLockMeIDs)
    {
      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(q);
      if (npc && npc->isAlive())
      {
        if (npc->getNpcType() == ENPCTYPE_MVP || npc->getNpcType() == ENPCTYPE_MINIBOSS)
          npc->m_ai.putAttacker(entry);
      }
    }
  }

  Scene* pScene = getScene();
  if (pScene == nullptr || pScene->isDScene() == false)
     return;

  DScene* pDScene = dynamic_cast<DScene*>(pScene);
  if(pDScene != nullptr && isMyTeamMember(entry->id))
    pDScene->addHealUser(id,entry->id, healhp);
}

void SceneUser::replaceBgmToMe(const string& bgm)
{
  ChangeBgmCmd cmd;
  cmd.set_bgm(bgm);
  cmd.set_type(EBGM_TYPE_REPLACE);

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

bool SceneUser::isInBattleTimeMap()
{
  if (getScene() == nullptr || getScene()->base == nullptr)
    return false;
  if (getScene()->isPVPScene())
    return false;
  if (getScene()->isDScene())
    return false;

  return true;
}

float SceneUser::getAddictRatio()
{
  float fRatio = LuaManager::getMe().call<float>("calcAddictRatio", this, getAddictTime());
  XINF << "[沉迷-个人衰减率] charid" << id << "ratio" << fRatio << XEND;
  return fRatio;
}

float SceneUser::getTeamAddictRatio()
{
  std::set<SceneUser*> setUser = getTeamRewardUsers();
  //不管有没有组队 一定包括了自己
  setUser.insert(this);
  
  float fSumRatio = 0;
  for (auto &v : setUser)
  {
    if (v)
    {
      fSumRatio += v->getAddictRatio();
    }
  }  
  float fRatio = (fSumRatio / setUser.size());

  XINF << "[沉迷-队伍衰减率] charid" << id << "sum ratio" << fSumRatio << "team count" << setUser.size() << "result ratio" << fRatio << XEND;
  return fRatio;
}

void SceneUser::getExtraReward(EExtraRewardType eType)
{
  if (RewardConfig::getMe().hasExtraReward(eType) == false)
    return;

  bool bAccLimit = RewardConfig::getMe().getExtraRwdAccLimit(eType);
  if(bAccLimit == true)
  {
    EVarType varType = RewardConfig::getMe().getVarByExtra(eType);
    EAccVarType varAccType = RewardConfig::getMe().getAccVarByExtra(eType);
    if(m_oVar.getAccVarValue(varAccType) == 0)
    {
      m_oVar.setAccVarValue(varAccType, 1);
      m_oVar.setVarValue(varType, 1);
    }
    else
    {
      if(m_oVar.getVarValue(varType) == 0)
        return ;
    }
  }

  TSetDWORD extraRwds;

  DWORD alreadycount = m_oUserSceneData.getExtraRewardTimes(eType);
  DWORD donecount = 0;
  switch(eType)
  {
    case EEXTRAREWARD_WANTEDQUEST:
      donecount = m_oVar.getVarValue(EVARTYPE_QUEST_WANTED);
      break;
    case EEXTRAREWARD_DAILYMONSTER:
      donecount = m_oQuest.getDailyCount();
      break;
    case EEXTRAREWARD_SEAL:
      donecount = m_oVar.getVarValue(EVARTYPE_SEAL);
      break;
    case EEXTRAREWARD_LABORATORY:
      donecount = 1;
      break;
    case EEXTRAREWARD_ENDLESS:
      donecount = 1;
      break;
    case EEXTRAREWARD_GUILD_QUEST:
      {
        donecount = m_oVar.getVarValue(EVARTYPE_GUILD_QUEST);
        alreadycount = m_oVar.getVarValue(EVARTYPE_GUILD_QUEST_EXTRAREWARD);
      }
      break;
    case EEXTRAREWARD_GUILD_DONATE:
      {
        donecount = 1;
        alreadycount = m_oVar.getVarValue(EVARTYPE_GUILD_DONATE_EXTRAREWARD);
      }
      break;
    case EEXTRAREWARD_PVECARD:
      donecount = 1;
      break;
    default:
      break;
  }

  if (RewardConfig::getMe().getExtraByType(eType, donecount, alreadycount, extraRwds) == false)
    return;

  if(eType == EEXTRAREWARD_GUILD_QUEST)
    m_oVar.setVarValue(EVARTYPE_GUILD_QUEST_EXTRAREWARD, alreadycount+1);
  else if(eType == EEXTRAREWARD_GUILD_DONATE)
    m_oVar.setVarValue(EVARTYPE_GUILD_DONATE_EXTRAREWARD, alreadycount+1);

  m_oUserSceneData.setExtraRewardTimes(eType, alreadycount + 1);
  for (auto &s : extraRwds)
  {
    XLOG <<  "[额外奖励], 玩家" << name << id << "奖励类型:" << eType << "获得额外奖励ID:" << s << XEND;
    m_oPackage.rollReward(s, EPACKMETHOD_AVAILABLE, false, true);
  }
}

DWORD SceneUser::getDoubleReward(EDoubleRewardType eType, TVecItemInfo& vecReward)
{
  if (RewardConfig::getMe().hasDoubleReward(eType) == false)
    return 1;

  bool bAccLimit = RewardConfig::getMe().getDoubleRwdAccLimit(eType);
  if(bAccLimit == true)
  {
    EVarType varType = RewardConfig::getMe().getVarByDouble(eType);
    EAccVarType varAccType = RewardConfig::getMe().getAccVarByDouble(eType);
    if(m_oVar.getAccVarValue(varAccType) == 0)
    {
      m_oVar.setAccVarValue(varAccType, 1);
      m_oVar.setVarValue(varType, 1);
    }
    else
    {
      if(m_oVar.getVarValue(varType) == 0)
        return 1;
    }
  }

  //DWORD alreadycount = m_oUserSceneData.getExtraRewardTimes(eType);
  DWORD alreadycount = 1;   //暂时先不处理这个条件
  DWORD donecount = 0;
  switch(eType)
  {
    case EDOUBLEREWARD_WANTEDQUEST:
      donecount = m_oVar.getVarValue(EVARTYPE_QUEST_WANTED) + 1;
      break;
    case EDOUBLEREWARD_DAILYMONSTER:
      donecount = m_oQuest.getDailyCount();
      break;
    case EDOUBLEREWARD_SEAL:
      donecount = m_oVar.getVarValue(EVARTYPE_SEAL);
      break;
    case EDOUBLEREWARD_LABORATORY:
      donecount = 1;
      break;
    case EDOUBLEREWARD_ENDLESS:
      donecount = 1;
      break;
    case EDOUBLEREWARD_PVECARD:
      donecount = 1;
      break;
    default:
      break;
  }

  DWORD dwTimes = RewardConfig::getMe().getDoubleRewardTimesByType(eType, donecount, alreadycount);

  for (auto &v : vecReward) {
    DWORD count = v.count() * dwTimes;
    v.set_count(count);
  }

  XLOG <<  "[双倍奖励], 玩家" << name << id << "奖励类型:" << eType << "活动次数:" << donecount << "倍数: " << dwTimes << XEND;
  return dwTimes;
}

void SceneUser::saveDataNow()
{
  // DWORD curSec = now();
  // save data
  m_dwRecordSave = 0;
}

void SceneUser::playChargeNpc(const Cmd::ChargePlayUserCmd& rev)
{
  for (int i = 0; i < rev.chargeids_size(); ++i)
  {
    m_chargeList.push_back(rev.chargeids(i));
  }

  if (!m_chargeList.empty())
    m_bPlayCharge = true;
}

void SceneUser::playChargeNpcTick(DWORD curSec)
{
  if (getScene() == nullptr)
    return;
  if (m_bPlayCharge == false)
    return;
  
  if (m_dwNextPlayTime > curSec)
    return;

  if (m_chargeList.empty())
  {
    //sys msg
    MsgManager::sendMsg(id, 202);
    m_bPlayCharge = false;
    XDBG << "[充值-动画] 结束 charid" << id << XEND;
    return;
  }
  
  auto it = m_chargeList.begin();
  
  const SDeposit* pCfg = DepositConfig::getMe().getSDeposit(*it);
  if (pCfg == nullptr)
  {
    m_bPlayCharge = false;
    m_chargeList.erase(it);
    XERR << "[充值-动画] 找不到配置表 charid" << id << "charge id" <<*it << XEND;
    return;
  }

  //summon npc
  NpcDefine def;
  def.setID(pCfg->npcId);
  def.setBehaviours(def.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);
  def.setTerritory(0);
  def.setDisptime(pCfg->duration);
  def.setWaitAction("functional_action");
  xPos lastPos;
  
  xPos dest;
  DWORD dwCount = 0;
  while (++dwCount < 30)
  {
    if (getScene()->getCircleRoundPos(getPos(), 2.0f, dest) == false)
    {
      dest = getPos();
      continue;
    }
    break;
  }
  def.setPos(dest);
  DWORD dir = calcAngle(dest, getPos());
  def.setDir(dir);
  SceneNpc *pNpc = SceneNpcManager::getMe().createNpc(def, getScene());
  if (pNpc)
  {
    pNpc->m_blGod = true;
    pNpc->setNotSyncDead();
  }

  //
  m_dwNextPlayTime = curSec + pCfg->duration + 2;
  XDBG << "[充值-动画] charid" << id << "charge id" << pCfg->id << "npcid" << pCfg->npcId << "duration" << pCfg->duration << "next time" << m_dwNextPlayTime << "left size" << m_chargeList.size() - 1<<XEND;

  m_chargeList.erase(it);
  return;
}

void SceneUser::setPhone(const string& phone)
{
  XDBG << "[手机号-绑定] " << accid << id << phone << XEND;
  m_strPhone = phone;
}

EBattleStatus SceneUser::getBattleStatus()
{
  DWORD totaltime = m_oUserSceneData.getTotalBattleTime();
  DWORD todaytime = m_oUserSceneData.getBattleTime();

  if (totaltime > todaytime)
    return EBATTLESTATUS_EASY;

  if (m_oUserSceneData.getReBattleTime() > m_oUserSceneData.getUsedBattleTime())
    return EBATTLESTATUS_EASY;
  
  if(m_oUserSceneData.getTutorBattleTime() > m_oUserSceneData.getUsedTutorBattleTime())
    return EBATTLESTATUS_EASY;

  DWORD stagetime = LuaManager::getMe().call<DWORD>("getAddictStage");
  if (stagetime + totaltime > todaytime)
    return EBATTLESTATUS_TIRED;

  return EBATTLESTATUS_HIGHTIRED;
}

void SceneUser::sendBattleStatusToMe()
{
  DWORD rebattletime =  m_oUserSceneData.getReBattleTime();
  DWORD usedrebattletime = m_oUserSceneData.getUsedBattleTime();
  DWORD tutorbattletime = m_oUserSceneData.getTutorBattleTime();
  DWORD usedtutorbattletime = m_oUserSceneData.getUsedTutorBattleTime();

  BattleTimelenUserCmd cmd;
  cmd.set_totaltime(m_oUserSceneData.getTotalBattleTime() + rebattletime + tutorbattletime);
  cmd.set_timelen(m_oUserSceneData.getBattleTime() + usedrebattletime + usedtutorbattletime);
  cmd.set_musictime(rebattletime);
  cmd.set_tutortime(tutorbattletime);
  cmd.set_estatus(getBattleStatus());

  XDBG << "[战斗时长同步] total:"<<m_oUserSceneData.getTotalBattleTime()<<" battletime:"<<m_oUserSceneData.getBattleTime()<<" rebattle:"<<rebattletime<<" "<<usedrebattletime<<" tutor:"<<tutorbattletime<<" "<<usedtutorbattletime<<XEND;
  XDBG << "[战斗时长同步] totaltime:"<<cmd.totaltime()<<" timelen:"<<cmd.timelen()<<" musictime:"<<cmd.musictime()<<" tutortime:"<<cmd.tutortime()<<XEND;
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

bool SceneUser::checkSeat(DWORD seatId)
{
  bool bSuccess = false;
  do 
  {
    Scene* pScene = getScene();
    if (pScene == nullptr)
      break;
    if (pScene->base == nullptr)
      break;
    Seat* pSeat = pScene->base->getSeat(seatId);
    if (pSeat == nullptr || pSeat->m_bOpen == false)
    {
      XERR << "[占座]" << id << "非法的座位的id" << seatId << pScene->getMapID() << pScene->name << XEND;
      break;
    }

    if (pSeat->m_seatUser && pSeat->m_seatUser != id)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(pSeat->m_seatUser);
      if (pUser != nullptr && pUser->getScene() == pScene)
      {
        MsgManager::sendMsg(id, 922);
        XERR << "[占座]" << id << "位子已经被别人占领 seatid" << seatId << pScene->getMapID() << pScene->name << XEND;
        break;
      }
      //容错
      pSeat->seatUp(pSeat->m_seatUser);
    }

    if (pSeat->check(getPos()) == false)
    {
      MsgManager::sendMsg(id, 921);
      XERR << "[占座]" << id << "距离不够" << seatId << pScene->getMapID() << pScene->name << XEND;
      break;
    }

    if(pSeat->m_mapCost.empty() == false)
    {
      TVecItemInfo vecCostItems;
      for(auto s : pSeat->m_mapCost)
      {
        ItemInfo item;
        item.set_id(s.first);
        item.set_count(s.second);
        combinItemInfo(vecCostItems, item);
      }
      BasePackage* pMainPack = m_oPackage.getPackage(EPACKTYPE_MAIN);
      if (pMainPack == nullptr)
        break;
      if(pMainPack->checkItemCount(vecCostItems) == false)
      {
        MsgManager::sendMsg(id, 1);
        break;
      }
      pMainPack->reduceItem(vecCostItems, ESOURCE_NORMAL);
    }


    m_oMove.clear();

    pSeat->seatDown(id);
    //setScenePos(pSeat->m_oSeatPos);
    m_seatId = seatId;
    sendSeatCmdToNine(seatId, true);
    bSuccess = true;
    XLOG << "[占座] 成功 " << id << "座位id" << seatId << "地图" << pScene->getMapID() << XEND;
  } while (0);
  
  return bSuccess;
}

void SceneUser::seatUp()
{
  if (m_seatId == 0)
    return;

  Scene* pScene = getScene();
  if (pScene == nullptr)
    return ;
  if (pScene->base == nullptr)
    return;
  Seat* pSeat = pScene->base->getSeat(m_seatId);
  if (pSeat == nullptr)
    return ;

  if (pSeat->m_seatUser && pSeat->m_seatUser != id)
  {
    XERR << "[占座]" << id << "座位玩家不一致" <<m_seatId << pSeat->m_seatUser << XEND;
    return;
  }
  pSeat->seatUp(id);
  MsgManager::sendMsg(id, 923);
  sendSeatCmdToNine(m_seatId, false);
  XLOG << "[占座]" << id << "离开座位" << m_seatId<<"地图"<<pScene->getMapID() << XEND;
  m_seatId = 0;
}

void SceneUser::sendSeatCmdToNine(DWORD seatId, bool isSeatDown)
{
  Cmd::NtfSeatUserCmd cmd;

  cmd.set_charid(id);
  cmd.set_seatid(seatId);
  cmd.set_isseatdown(isSeatDown);
  PROTOBUF(cmd, send, len);
  sendCmdToNine(send, len);

  if (isSeatDown && getScene() != nullptr)
  {
    xSceneEntrySet set;
    getScene()->getEntryListInNine(SCENE_ENTRY_USER, getPos(), set);
    for (auto &s : set)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(s);
      if (pUser != nullptr)
        pUser->getAchieve().onSeat();
    }
  }
}

bool SceneUser::checkYoYoSeat(QWORD npcguid)
{
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(npcguid);
  if(npc == nullptr)
    return false;
  Scene* pScene = getScene();
  if (pScene == nullptr || pScene->base == nullptr)
    return false;

  const TSetDWORD setSeat = npc->define.getSeat();
  for(auto s : setSeat)
  {
    Seat* pSeat = pScene->base->getSeat(s);
    if(pSeat == nullptr)
      continue;
    if(pSeat->m_seatUser && pSeat->m_seatUser != id)
      continue;
    return checkSeat(s);
  }

  MsgManager::sendMsg(id, 922);
  return false;
}

bool SceneUser::checkSeatTime()
{
  if (m_seatId == 0)
    return false;

  Scene* pScene = getScene();
  if (pScene == nullptr)
    return false;
  if (pScene->base == nullptr)
    return false;
  Seat* pSeat = pScene->base->getSeat(m_seatId);
  if (pSeat == nullptr)
  {
    XERR << "[占座]" << id << "非法的座位的id" << m_seatId << pScene->getMapID() << pScene->name << XEND;
    return false;
  }
  if(pSeat->m_dwSeatTime == 0 || pSeat->m_seatUser != id)
    return false;

  if (pSeat->m_dwSeatTime + pSeat->m_dwSeatDownTime <= now())
    seatUp();

  return true;
}

void SceneUser::addSeeNpc(DWORD id)
{
  if (m_oUserSceneData.haveSeeNpcs(id))
    return;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(id);
  if (pCFG == nullptr || pCFG->bNormalHide == false)
    return;
  m_oUserSceneData.addSeeNpcs(id);
  XLOG << "[Npc-可见设置], npc:" << id << "对玩家:" << this->name << this->id << "可见" << XEND;
  if (getScene() != nullptr)
  {
    std::list<SceneNpc*> npclist;
    getScene()->getSceneNpcByBaseID(id, npclist);
    for (auto &iter : npclist)
    {
      iter->sendMeToUser(this);
    }
  }
}

void SceneUser::delSeeNpc(DWORD id)
{
  if (m_oUserSceneData.haveSeeNpcs(id) == false)
    return;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(id);
  if (pCFG == nullptr || pCFG->bNormalHide == false)
    return;
  m_oUserSceneData.delSeeNpcs(id);
  XLOG << "[Npc-可见设置], npc:" << id << "对玩家:" << this->name << this->id << "取消可见" << XEND;
  if (getScene() != nullptr)
  {
    std::list<SceneNpc*> npclist;
    getScene()->getSceneNpcByBaseID(id, npclist);
    for (auto &iter : npclist)
    {
      iter->delMeToUser(this);
    }
  }
}

void SceneUser::addHideNpc(DWORD id)
{
  if (m_oUserSceneData.haveHideNpcs(id))
    return;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(id);
  if (pCFG == nullptr || pCFG->bNormalDisplay == false)
    return;

  m_oUserSceneData.addHideNpcs(id);

  XLOG << "[Npc-可见设置], npc:" << id << "对玩家:" << this->name << this->id << "不可见" << XEND;
  if (getScene() != nullptr)
  {
    std::list<SceneNpc*> npclist;
    getScene()->getSceneNpcByBaseID(id, npclist);
    for (auto &iter : npclist)
    {
      iter->delMeToUser(this);
    }
  }
}

void SceneUser::delHideNpc(DWORD id)
{
  if (m_oUserSceneData.haveHideNpcs(id) == false)
    return;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(id);
  if (pCFG == nullptr || pCFG->bNormalDisplay == false)
    return;

  m_oUserSceneData.delHideNpcs(id);

  XLOG << "[Npc-可见设置], npc:" << id << "对玩家:" << this->name << this->id << "取消不可见" << XEND;
  if (getScene() != nullptr)
  {
    std::list<SceneNpc*> npclist;
    getScene()->getSceneNpcByBaseID(id, npclist);
    for (auto &iter : npclist)
    {
      iter->sendMeToUser(this);
    }
  }
}

DWORD SceneUser::getEquipRefineLv(DWORD pos)
{
  if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX)
    return 0;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_oPackage.getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return 0;
  ItemEquip* pEquip = pEquipPack->getEquip((EEquipPos)pos);
  if (pEquip == nullptr)
    return 0;
  return pEquip->getRefineLv();
}

DWORD SceneUser::getEquipCardNum(DWORD pos, DWORD cardid)
{
  if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX || EEquipPos_IsValid(pos) == false)
    return 0;

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_oPackage.getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return 0;

  const ItemEquip* pEquip = pEquipPack->getEquip((EEquipPos)pos);
  if (pEquip == nullptr)
    return 0;

  const TMapEquipCard& cards = pEquip->getCardList();

  DWORD num = 0;
  for (auto c = cards.begin(); c != cards.end(); ++c) {
    if ((*c).second.second == cardid)
      ++num;
  }

  return num;
}

void SceneUser::refreshTeamTime(DWORD dwTime)
{
  if (m_oGTeam.getTeamID() == 0)
    return;

  const TMapGTeamMember& mapMember = m_oGTeam.getTeamMemberList();
  for (auto &m : mapMember)
  {
    if (id != m.first)
      m_oShare.addCalcData(ESHAREDATATYPE_MOST_TEAMTIME, m.first, dwTime);
  }
}

void SceneUser::refreshPendingCount()
{
  DWORD count = LuaManager::getMe().call<DWORD>("calcTradeMaxPendingCout", (xSceneEntryDynamic*)(this));
  if (m_dwTradePendingCount == count)
    return;
  DWORD configCnt = MiscConfig::getMe().getTradeCFG().dwMaxPendingCount;
  if (count == configCnt && m_dwTradePendingCount == 0)
  {
    m_dwTradePendingCount = count;
    return;
  }
  m_dwTradePendingCount = count;

  ExtraPermissionSceneTradeCmd cmd;
  cmd.set_charid(this->id);
  cmd.set_permission(EPERMISSION_MAX_PENDING_LIMIT);
  cmd.set_value(count);
  PROTOBUF(cmd, send, len);

  thisServer->forwardCmdToSessionTrade(this->id, name, send, len);

  XLOG << "[玩家-交易所], 最大上架数量改变, 玩家:" << name << this->id << "数量:" << count << XEND;
  
  GCharWriter gChar(thisServer->getRegionID(), id);
  gChar.setVersion();
  gChar.setPendingLimit(count);
  gChar.save();
}

void SceneUser::refrshTradeBackMoneyPer()
{
  float per = LuaManager::getMe().call<float>("calcTradeBackMoneyPer", (xSceneEntryDynamic*)(this));
  DWORD dwper = 1000 * per;
  if (dwper == m_dwTradeBackMoneyPer)
    return;
  m_dwTradeBackMoneyPer = dwper;

  ExtraPermissionSceneTradeCmd cmd;
  cmd.set_charid(this->id);
  cmd.set_permission(EPERMISSION_RETURN_PERCENT);
  cmd.set_value(dwper);
  PROTOBUF(cmd, send, len);

  thisServer->forwardCmdToSessionTrade(this->id, name, send, len);

  XLOG << "[玩家-交易所], 出售失败返还钱百分比更改, 玩家:" << name << this->id << "当前百分比:" << dwper << "/1000" << XEND;
  
  GCharWriter gChar(thisServer->getRegionID(), id);
  gChar.setVersion();
  gChar.setReturnRate(dwper);
  gChar.save();
}

void SceneUser::refreshTradeQuota()
{
  QWORD quota = m_oDeposit.getQuota();
  ExtraPermissionSceneTradeCmd cmd;
  cmd.set_charid(this->id);
  cmd.set_permission(EPERMISSION_QUOTA);
  cmd.set_value(quota);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSessionTrade(this->id, name, send, len);

  XLOG << "[玩家-交易所], 玩家额度同步到交易所 玩家:" << name << this->id << "额度:" << quota << XEND;

  GCharWriter gChar(thisServer->getRegionID(), id);
  gChar.setVersion();
  gChar.setQuota(quota);
  gChar.save();
}

void SceneUser::refreshBoothCount()
{
  DWORD count = m_oBooth.getSize();

  ExtraPermissionSceneTradeCmd cmd;
  cmd.set_charid(this->id);
  cmd.set_permission(EPERMISSION_MAX_BOOTH_LIMIT);
  cmd.set_value(count);
  PROTOBUF(cmd, send, len);

  thisServer->forwardCmdToSessionTrade(this->id, name, send, len);

  XLOG << "[玩家-摊位], 最大上架数量改变, 玩家:" << name << this->id << "数量:" << count << XEND;
  
  GCharWriter gChar(thisServer->getRegionID(), id);
  gChar.setVersion();
  gChar.setBoothPendingLimit(count);
  gChar.save();
}

DWORD SceneUser::getEquipID(DWORD pos)
{
  if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX)
    return 0;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_oPackage.getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return 0;
  ItemEquip* pEquip = pEquipPack->getEquip((EEquipPos)pos);
  if (pEquip == nullptr)
    return 0;
  return pEquip->getTypeID();
}

bool SceneUser::checkPwd(EUnlockType type, DWORD param)
{
  if(m_blIgnorepwd == true)
    return true;

  switch (type)
  {
    case EUNLOCKTYPE_TRADE:
    case EUNLOCKTYPE_EQUIP_UPGRADE:
    case EUNLOCKTYPE_EQUIP_HOLE:
    case EUNLOCKTYPE_ENCHANT:
    //case EUNLOCKTYPE_NPC_BUY:
    case EUNLOCKTYPE_GUILD:
    case EUNLOCKTYPE_FRIEND:
    case EUNLOCKTYPE_TRADE_GIFT:
    case EUNLOCKTYPE_DECOMPOSE:
    case EUNLOCKTYPE_AUCTION_SELL:
    case EUNLOCKTYPE_AUCTION_BUY:
    case EUNLOCKTYPE_ITEM_CODE:
      return false;
    case EUNLOCKTYPE_NPC_BUY:
      return true;
    case EUNLOCKTYPE_REFINE:
      {
        DWORD lv = MiscConfig::getMe().getAuthorize().dwRefineLv;
        if(param >= lv)
          return false;
      }
      break;
    case EUNLOCKTYPE_SELL:
      {
        const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(param);
        if(pCFG != nullptr && pCFG->hasFeatrue(EFeature_Sell) == true)
          return true;

        return false;
      }
      break;
    case EUNLOCKTYPE_USEITEM:
      {
        const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(param);
        if(pCFG != nullptr && pCFG->hasFeatrue(EFeature_Use) == false)
          return true;

        return false;
      }
      break;
    default:
      return true;
  }

  return true;
}
string SceneUser::getCharIDString()
{
  stringstream ss;
  ss<<id;
  return ss.str();
}

string SceneUser::getAccIDString()
{
  stringstream ss;
  ss<<accid;
  return ss.str();
}

void SceneUser::enterGuildRaid(QWORD npcguid)
{
  GuildScene* pGuildScene = dynamic_cast<GuildScene*>(getScene());
  if (pGuildScene == nullptr)
    return;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(npcguid);
  if (npc == nullptr)
    return;
  if (getDistance(npc->getPos(), getPos()) > 5)
    return;
  DWORD npcbaseid = npc->getNpcID();
  if (m_oUserSceneData.canEnterGuildRaid(npcbaseid) == false)
    return;

  if (m_oVar.getVarValue(EVARTYPE_GUILD_RAID_BAN) == 1)
  {
    XLOG << "[公户副本-开启], 玩家本周切换公会, 不可再次进入, 玩家:" << name << id << XEND;
    return;
  }

  // get group by npcid
  const SGuildRaidCFG& rCFG = MiscConfig::getMe().getGuildRaidCFG();

  CreateDMapParams param;
  param.qwCharID = id;

  DWORD raidgroup = rCFG.getGroupByNpc(npcbaseid);
  if (raidgroup == 0)
    return;
  // check have teamer in it
  for (auto &m : m_oGTeam.getTeamMemberList())
  {
    if (m.first == id)
      continue;
    if (m.second.guildraidindex() != 0 && m.second.guildraidindex() / ONE_THOUSAND == raidgroup)
    {
      param.m_dwGuildRaidIndex = m.second.guildraidindex();
      const SGuildRaidInfo* pInfo = GuildRaidConfig::getMe().getGuildRaidInfo(param.m_dwGuildRaidIndex);
      if (pInfo == nullptr)
        continue;
      param.dwRaidID = pInfo->dwMapID;
      SceneManager::getMe().createDScene(param);
      XLOG << "[玩家-进入公会副本], 玩家:" << name << id << "副本Npc:" << npcbaseid << "进入队友所在地图:" << param.m_dwGuildRaidIndex << XEND;
      return;
    }
  }

  const SGuildRaidInfo* pEntranceMap = GuildRaidConfig::getMe().getGuildEntrance(raidgroup);
  if (pEntranceMap == nullptr)
    return;
  param.dwRaidID = pEntranceMap->dwMapID;
  param.m_dwGuildRaidIndex = pEntranceMap->getMapIndex();
  SceneManager::getMe().createDScene(param);

  XLOG << "[玩家-进入公会副本], 玩家:" << name << id << "副本Npc:" << npcbaseid << "入口地图:" << pEntranceMap->dwMapID << XEND;
 
}

void SceneUser::sendGuildGateDataToMe(DWORD npcid/*=0*/)
{
  Scene* pScene = getScene();
  if (pScene == nullptr)
    return;

  const TMapNpcID2GuildRaidData& rRaids = m_oUserSceneData.getGuildRaids();
  const SGuildRaidCFG& rCfg = MiscConfig::getMe().getGuildRaidCFG();

  UserGuildRaidFubenCmd msg;

  DWORD cur = now() - 5 * 3600;

  for (auto &itCfg : rCfg.mapNpcID2GuildRaid) {
    if (npcid != 0 && itCfg.second.dwNpcID != npcid)
      continue;

    std::list<SceneNpc*> npcs;
    pScene->getSceneNpcByBaseID(itCfg.second.dwNpcID, npcs);
    if (npcs.size() < 1)
      continue;

    Cmd::GuildGateData *gatedata = msg.add_gatedata();
    if (gatedata == nullptr)
      continue;

    gatedata->set_gatenpcid(npcs.front()->getGUID());
    gatedata->set_isspecial(itCfg.second.bSpecial);
    gatedata->set_closetime(xTime::getWeekStart(cur) + 86400 * 7 + 5 * 3600);

    auto itData = rRaids.find(itCfg.second.dwNpcID);
    if (itData == rRaids.end()) {
      m_oUserSceneData.newGuildRaidData(itCfg.second);
      itData = rRaids.find(itCfg.second.dwNpcID);
      if (itData == rRaids.end())
        continue;
    }

    gatedata->set_state(itData->second.eState);
    if (itData->second.eState == EGUILDGATESTATE_OPEN) {
      gatedata->set_killedbossnum(itData->second.setKilledBoss.size());

      DWORD group = rCfg.getGroupByNpc(itCfg.second.dwNpcID);
      gatedata->set_groupindex(group);
    }

    // if (!itCfg.second.bSpecial) {
    //   for (auto itLevel = itCfg.second.setLevel.begin(); itLevel != itCfg.second.setLevel.end(); ++itLevel) {
    //     gatedata->set_level(*itLevel);
    //     break;
    //   }
    // }

    npcs.front()->setSpecialGearStatus(true);

    // UserActionNtf ntf;
    // ntf.set_type(EUSERACTIONTYPE_GEAR_ACTION);
    // ntf.set_charid(npcs.front()->getGUID());
    // switch (gatedata->state()) {
    // case EGUILDGATESTATE_LOCK:
    //   ntf.set_value(1002);
    //   break;
    // case EGUILDGATESTATE_CLOSE:
    //   ntf.set_value(2002);
    //   break;
    // case EGUILDGATESTATE_OPEN:
    //   ntf.set_value(3002);
    //   break;
    // default:
    //   continue;
    // }
    // PROTOBUF(ntf, ntfsend, ntflen);
    // sendCmdToMe(ntfsend, ntflen);
  }

  PROTOBUF(msg, send, len);
  sendCmdToMe(send, len);
}

DWORD SceneUser::getGuildGateGearStatus(SceneNpc* pNpc)
{
  if (pNpc == nullptr)
    return 0;

  GuildScene* pGuildScene = dynamic_cast<GuildScene*>(getScene());
  if (pGuildScene == nullptr)
    return 0;

  const TMapNpcID2GuildRaidData& rRaids = m_oUserSceneData.getGuildRaids();
  const SGuildRaidCFG& rCfg = MiscConfig::getMe().getGuildRaidCFG();
  auto itCfg = rCfg.mapNpcID2GuildRaid.find(pNpc->getNpcID());
  if (itCfg == rCfg.mapNpcID2GuildRaid.end())
    return 0;

  auto itData = rRaids.find(itCfg->second.dwNpcID);
  if (itData == rRaids.end()) {
    m_oUserSceneData.newGuildRaidData(itCfg->second);
    itData = rRaids.find(itCfg->second.dwNpcID);
    if (itData == rRaids.end())
      return 0;
  }

  switch (itData->second.eState) {
  case EGUILDGATESTATE_LOCK:
    return 1002;
  case EGUILDGATESTATE_CLOSE:
    return 2002;
  case EGUILDGATESTATE_OPEN:
    return 3002;
  default:
    return 0;
  }

  return 0;
}

// 解锁公会副本大门
void SceneUser::unlockGuildRaidGate(QWORD npcguid, DWORD level)
{
  // SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(npcguid);
  // if (npc == nullptr)
  //   return;
  // if (getDistance(npc->getPos(), getPos()) > 5)
  //   return;
  // DWORD npcid = npc->getNpcID();

  // const SGuildRaidCFG& rCfg = MiscConfig::getMe().getGuildRaidCFG();
  // auto it = rCfg.mapNpcID2GuildRaid.find(npcid);
  // if (it == rCfg.mapNpcID2GuildRaid.end())
  //   return;

  // TMapNpcID2GuildRaidData& rDatas= m_oUserSceneData.getGuildRaids();
  // auto itData = rDatas.find(npcid);
  // if (itData == rDatas.end())
  //   return;

  // if (itData->second.eState != EGUILDGATESTATE_LOCK)
  //   return;

  // // 解锁等级非法
  // if (it->second.setLevel.find(level) == it->second.setLevel.end())
  //   return;

  // // todo 时间检查

  // // 非特殊大门不需要解锁
  // if (!it->second.bSpecial)
  //   return;

  // // 公会等级不够
  // if (m_oGuildInfo.lv() < it->second.dwGuildLevel)
  //   return;

  // TVecItemInfo vecItems;
  // for (auto &v : it->second.vecUnlockItem) {
  //   ItemInfo oItem;
  //   oItem.set_id(v.first);
  //   oItem.set_count(v.second);
  //   combinItemInfo(vecItems, TVecItemInfo{oItem});
  // }

  // // 扣除道具
  // BasePackage *pMainPack = getPackage().getPackage(EPACKTYPE_MAIN);
  // if (pMainPack == nullptr || pMainPack->checkItemCount(vecItems) == false)
  //   return;
  // for (auto &v : vecItems)
  //   pMainPack->reduceItem(v.id(), ESOURCE_OPEN_GUILD_GATE, v.count());

  // itData->second.eState = EGUILDGATESTATE_CLOSE;

  // XLOG << "[公会副本] 解锁副本大门, 玩家:" << name << "id:" << id << "npcid:" << npcid << XEND;
}

// 开启公会副本大门
void SceneUser::openGuildRaidGate(QWORD npcguid)
{
  if (m_oGuild.id() <= 0)
    return;

  if (m_oVar.getVarValue(EVARTYPE_GUILD_RAID_BAN) == 1)
  {
    MsgManager::sendMsg(id, 7205);
    XLOG << "[公户副本-开启], 玩家本周切换公会, 不可再次开启, 玩家:" << name << id << XEND;
    return;
  }

  SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(npcguid);
  if (npc == nullptr)
    return;
  if (getDistance(npc->getPos(), getPos()) > 5)
    return;
  DWORD npcid = npc->getNpcID();

  const SGuildRaidCFG& rCfg = MiscConfig::getMe().getGuildRaidCFG();
  auto itCfg = rCfg.mapNpcID2GuildRaid.find(npcid);
  if (itCfg == rCfg.mapNpcID2GuildRaid.end())
    return;

  // 玩家等级不够
  if (getLevel() < itCfg->second.dwUserLevel) {
    MsgManager::sendMsg(id, 7204);
    return;
  }

  // 公会等级不够
  if (m_oGuild.lv() < itCfg->second.dwGuildLevel)
    return;

  DWORD cur = now() - 5 * 3600;
  // 重置前1小时不可开启
  if (cur > xTime::getWeekStart(cur) + 604800 - rCfg.unsteady_time) {
    MsgManager::sendMsg(id, 7200);
    return;
  }

  TMapNpcID2GuildRaidData& rDatas= m_oUserSceneData.getGuildRaids();
  auto itData = rDatas.find(npcid);
  if (itData == rDatas.end()) {
    m_oUserSceneData.newGuildRaidData(itCfg->second);
    itData = rDatas.find(npcid);
    if (itData == rDatas.end())
      return;
  }

  if (itData->second.eState != EGUILDGATESTATE_CLOSE)
    return;

  TVecItemInfo vecItems;
  for (auto &v : itCfg->second.vecOpenItem) {
    ItemInfo oItem;
    oItem.set_id(v.first);
    oItem.set_count(v.second);
    combinItemInfo(vecItems, TVecItemInfo{oItem});
  }

  // 扣除道具
  BasePackage *pMainPack = getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr || pMainPack->checkItemCount(vecItems) == false) {
    MsgManager::sendMsg(id, 7201);
    return;
  }
  for (auto &v : vecItems)
    pMainPack->reduceItem(v.id(), ESOURCE_OPEN_GUILD_GATE, v.count());
  MsgManager::sendMsg(id, 7203);

  itData->second.eState = EGUILDGATESTATE_OPEN;

  // 推送npc状态
  UserActionNtf msg;
  msg.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  msg.set_value(3001);
  msg.set_charid(npc->getGUID());
  PROTOBUF(msg, send, len);
  sendCmdToMe(send, len);

  sendGuildGateDataToMe(npcid);

  XLOG << "[公会副本] 开启副本大门, 玩家:" << name << "id:" << id << "npcid:" << npcid << XEND;
}

void SceneUser::unlockGuildRaidGate(SceneNpc* pNpc, DWORD guildlv, bool sync/* = true*/)
{
  if (pNpc == nullptr)
    return;

  const SGuildRaidCFG& rCfg = MiscConfig::getMe().getGuildRaidCFG();
  auto itCfg = rCfg.mapNpcID2GuildRaid.find(pNpc->getNpcID());
  if (itCfg == rCfg.mapNpcID2GuildRaid.end())
    return;

  TMapNpcID2GuildRaidData& rDatas= m_oUserSceneData.getGuildRaids();
  auto itData = rDatas.find(pNpc->getNpcID());
  if (itData == rDatas.end()) {
    m_oUserSceneData.newGuildRaidData(itCfg->second);
    itData = rDatas.find(pNpc->getNpcID());
    if (itData == rDatas.end())
      return;
  }

  if (itData->second.eState != EGUILDGATESTATE_LOCK)
    return;

  if (guildlv >= itCfg->second.dwGuildLevel) {
    itData->second.eState = EGUILDGATESTATE_CLOSE;
    if (sync) sendGuildGateDataToMe(pNpc->getNpcID());

    if (check2PosInNine(pNpc)) {
      UserActionNtf msg;
      msg.set_type(EUSERACTIONTYPE_GEAR_ACTION);
      msg.set_value(2001);
      msg.set_charid(pNpc->getGUID());
      PROTOBUF(msg, send, len);
      sendCmdToMe(send, len);
    }
  }
}

// 进入公会场景时同步公会等级解锁大门, 并同步客户端大门数据
void SceneUser::syncGuildRaidData(DWORD guildlv)
{
  Scene* pScene = getScene();
  if (pScene == nullptr)
    return;

  const SGuildRaidCFG& rCfg = MiscConfig::getMe().getGuildRaidCFG();

  for (auto &itCfg : rCfg.mapNpcID2GuildRaid) {
    std::list<SceneNpc*> npcs;
    pScene->getSceneNpcByBaseID(itCfg.second.dwNpcID, npcs);
    if (npcs.size() < 1)
      continue;

    SceneNpc* pNpc = npcs.front();
    if (pNpc == nullptr)
      continue;

    unlockGuildRaidGate(pNpc, guildlv, false);
  }

  sendGuildGateDataToMe();
}

void SceneUser::getTaskExtraReward(ETaskExtraRewardType type, DWORD times)
{
  if(type == ETASKEXTRAREWARDTYPE_MIN || type >= ETASKEXTRAREWARDTYPE_MAX)
    return;

  const SExtraRewardCFG* pCfg = MiscConfig::getMe().getExtraRewardCFG(type);
  if(pCfg == nullptr)
    return;

  if(times == 0 || pCfg->normalReward.dwFinishTimes == 0)
    return;

  if(times/pCfg->normalReward.dwFinishTimes == 0 || times%pCfg->normalReward.dwFinishTimes != 0)
    return;


  TVecItemInfo vecItemInfo;
  ItemInfo oItem;
  oItem.set_id(pCfg->normalReward.dwItemID);
  oItem.set_count(pCfg->normalReward.dwItemNum);
  oItem.set_source(ESOURCE_REWARD);
  vecItemInfo.push_back(oItem);
  DWORD count = pCfg->normalReward.dwItemNum;

  if(pCfg->dwLevel != 0 && getLevel() >= pCfg->dwLevel)
  {
    ItemInfo pItem;
    pItem.set_id(pCfg->addReward.dwItemID);
    pItem.set_count(pCfg->addReward.dwItemNum);
    pItem.set_source(ESOURCE_REWARD);
    vecItemInfo.push_back(pItem);
    count += pCfg->addReward.dwItemNum;
  }

  getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);

  XLOG << "[玩家-任务额外奖励]:" << name << this->id << type << times << pCfg->normalReward.dwItemID << count << XEND;
}

void SceneUser::addKillCount(DWORD monsterid, float fcnt)
{
  auto it = m_mapMonsterKillNum.find(monsterid);
  if (it == m_mapMonsterKillNum.end())
  {
    m_mapMonsterKillNum[monsterid] = fcnt;
    return;
  }

  it->second += fcnt;
}

float SceneUser::getKillCount(DWORD monsterid)
{
  auto it = m_mapMonsterKillNum.find(monsterid);
  return it != m_mapMonsterKillNum.end() ? it->second : 0;
}

DWORD SceneUser::getEvo()
{
  DWORD pro = getUserSceneData().getProfession();
  return pro == EPROFESSION_NOVICE ? 0 : pro - pro / 10 * 10;
}

DWORD SceneUser::getRuneSpecNum(DWORD specID)
{
  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(specID);
  if (pRuneCFG == nullptr)
    return 0;
  if (pRuneCFG->eType == ERUNESPECTYPE_SELECT)
  {
    if (m_pCurFighter && m_pCurFighter->getSkill().isRuneSpecSelected(specID) == false && getUserBeing().isRuneSpecSelected(specID) == false)
      return 0;
  }
  return m_oAstrolabes.getEffectCnt(specID);
}

void SceneUser::redisPatch()
{
  m_oDeposit.redisPatch();
  XLOG << "[补丁-redis] 处理完毕" << id << XEND;
}

void SceneUser::getCelebrationID(TVecDWORD& vec)
{
  vec.clear();

  DWORD count = m_oManual.getMonthCard();
  if(count == 0)
    return ;

  DWORD cnt1 = m_oVar.getVarValue(EVARTYPE_CELEBRATION_ONE);
  SCelebrationMCardCFG* pCelebration = MiscConfig::getMe().getCelebrationMCardCFGbyID(ECELEBRATIONLEVEL_ONE);
  if(pCelebration != nullptr && cnt1 == 0 && count >= pCelebration->dwCostCard)
    vec.push_back(static_cast<DWORD>(ECELEBRATIONLEVEL_ONE));

  DWORD cnt2 = m_oVar.getVarValue(EVARTYPE_CELEBRATION_TWO);
  SCelebrationMCardCFG* pCelebration2 = MiscConfig::getMe().getCelebrationMCardCFGbyID(ECELEBRATIONLEVEL_TWO);
  if(pCelebration2 != nullptr && cnt2 == 0 && count >= pCelebration2->dwCostCard)
    vec.push_back(static_cast<DWORD>(ECELEBRATIONLEVEL_TWO));

  DWORD cnt3 = m_oVar.getVarValue(EVARTYPE_CELEBRATION_THREE);
  SCelebrationMCardCFG* pCelebration3 = MiscConfig::getMe().getCelebrationMCardCFGbyID(ECELEBRATIONLEVEL_THREE);
  if(pCelebration3 != nullptr && cnt3 == 0 && count >= pCelebration3->dwCostCard)
    vec.push_back(static_cast<DWORD>(ECELEBRATIONLEVEL_THREE));
}

bool SceneUser::getCelebrationReward(DWORD level)
{
  ECelebrationLevel type = static_cast<ECelebrationLevel>(level);
  SCelebrationMCardCFG* pCelebration = MiscConfig::getMe().getCelebrationMCardCFGbyID(type);
  if(pCelebration == nullptr)
    return false;

  DWORD count = m_oManual.getMonthCard();
  if(count < pCelebration->dwCostCard)
  {
    XERR << "[运营-活动] 领取感恩礼盒失败" << accid << id << getProfession() << name << "等级" << type << "已经领取"<< XEND;
    return false;
  }

  bool ret = true;
  if(type == ECELEBRATIONLEVEL_ONE)
  {
    DWORD cnt = m_oVar.getVarValue(EVARTYPE_CELEBRATION_ONE);
    if(cnt != 0)
    {
      ret = false;
    }
    else
    {
      m_oVar.setVarValue(EVARTYPE_CELEBRATION_ONE, 1);
    }
  }
  else if(type == ECELEBRATIONLEVEL_TWO)
  {
    DWORD cnt = m_oVar.getVarValue(EVARTYPE_CELEBRATION_TWO);
    if(cnt != 0)
    {
      ret = false;
    }
    else
    {
      m_oVar.setVarValue(EVARTYPE_CELEBRATION_TWO, 1);
    }
  }
  else if(type == ECELEBRATIONLEVEL_THREE)
  {
    DWORD cnt = m_oVar.getVarValue(EVARTYPE_CELEBRATION_THREE);
    if(cnt != 0)
    {
      ret = false;
    }
    else
    {
      m_oVar.setVarValue(EVARTYPE_CELEBRATION_THREE, 1);
    }
  }
  else
  {
    ret = false;
  }

  OperateTakeSocialCmd cmd;
  cmd.set_type(EOperateType_MonthCard);
  cmd.set_subkey(level);
  if(ret == false)
  {
    cmd.set_state(EOperateState_None);
    XERR << "[运营-活动] 领取感恩礼盒失败" << accid << id << getProfession() << name << "等级" << level << "已经领取"<< XEND;
  }
  else
  {
    cmd.set_state(EOperateState_CanTake);

    ItemInfo stInfo;
    stInfo.set_id(pCelebration->dwRewardID);
    stInfo.set_count(pCelebration->dwCount);
    stInfo.set_source(ESOURCE_REWARD);
    getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);
    XLOG << "[运营-活动] 领取感恩礼盒成功" << accid << id << getProfession() << name << "等级" << level << XEND;
  }

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  return true;
}

bool SceneUser::getActivityReward()
{
  //const SGlobalActivityCFG* pCFG = MiscConfig::getMe().getGlobalActivityCFG(GACTIVITY_AUGURY);
  const SGlobalActCFG*pCFG = ActivityConfig::getMe().getGlobalActCFG(static_cast<DWORD>(GACTIVITY_AUGURY));
  if(pCFG == nullptr)
    return false;
  DWORD activityid = pCFG->m_dwId;
  bool isOpen = ActivityManager::getMe().isOpen(activityid);
  if(isOpen == false)
  {
    XLOG << "[运营-活动] 占卜" << accid << id << getProfession() << name << "活动未开启" << XEND;
    return false;
  }
  //以5点作为每日分界线
  //DWORD lastGetTime = m_oUserSceneData.getAuguryRewardTime();
  DWORD curTime = xTime::getCurSec();
  DWORD dwGetTime = 5*HOUR_T;
  if(m_oUserSceneData.getAuguryRewardTime() > dwGetTime)
    dwGetTime = m_oUserSceneData.getAuguryRewardTime() - dwGetTime;
  if(xTime::isSameDay(dwGetTime, curTime - 5*HOUR_T) == true)
  {
    XLOG << "[运营-活动] 占卜" << accid << id << getProfession() << name << "已经领取奖励" << XEND;
    return false;
  }

  m_oUserSceneData.setAuguryRewardTime();
  DWORD dwMailID = pCFG->getParam(0);
  MailManager::getMe().sendMail(id, dwMailID);
  XLOG << "[运营-活动] 占卜奖励" << "charid" << id << getProfession() << getName() << "dwMailID" << dwMailID << XEND;
  return true;
}

void SceneUser::showMonthCardErrorLog()
{
  DWORD count = m_oManual.getMonthCard();

  DWORD cnt1 = m_oVar.getVarValue(EVARTYPE_CELEBRATION_ONE);
  SCelebrationMCardCFG* pCelebration = MiscConfig::getMe().getCelebrationMCardCFGbyID(ECELEBRATIONLEVEL_ONE);
  if(pCelebration != nullptr && cnt1 != 0 && count < pCelebration->dwCostCard)
    XERR << "[运营-活动] 领取感恩礼盒错误" << accid << id << getProfession() << name << "月卡: " << count << "需要数量:" << pCelebration->dwCostCard <<
      "领取物品: "<< pCelebration->dwRewardID << "数量: " << pCelebration->dwCount << XEND;

  DWORD cnt2 = m_oVar.getVarValue(EVARTYPE_CELEBRATION_TWO);
  SCelebrationMCardCFG* pCelebration2 = MiscConfig::getMe().getCelebrationMCardCFGbyID(ECELEBRATIONLEVEL_TWO);
  if(pCelebration2 != nullptr && cnt2 != 0 && count < pCelebration2->dwCostCard)
    XERR << "[运营-活动] 领取感恩礼盒错误" << accid << id << getProfession() << name << "月卡: " << count << "需要数量:" << pCelebration2->dwCostCard <<
      "领取物品: "<< pCelebration2->dwRewardID << "数量: " << pCelebration2->dwCount << XEND;

  DWORD cnt3 = m_oVar.getVarValue(EVARTYPE_CELEBRATION_THREE);
  SCelebrationMCardCFG* pCelebration3 = MiscConfig::getMe().getCelebrationMCardCFGbyID(ECELEBRATIONLEVEL_THREE);
  if(pCelebration3 != nullptr && cnt3 != 0 && count < pCelebration3->dwCostCard)
    XERR << "[运营-活动] 领取感恩礼盒错误" << accid << id << getProfession() << name << "月卡: " << count << "需要数量:" << pCelebration3->dwCostCard <<
      "领取物品: "<< pCelebration3->dwRewardID << "数量: " << pCelebration3->dwCount << XEND;
}

bool SceneUser::isWantedQuestLeader()
{
  if (getTeamID() == 0 || (getTeamLeaderID() != id && m_oGTeam.getTeampLeaderID() != id) ||
      (m_pCurFighter == nullptr || m_pCurFighter->getSkill().isSkillEnable(50040001) == false))
    return false;
  return true;
}

void SceneUser::addLoveLetter(DWORD letterID, string sendUserName, string bg, DWORD configID, string content)
{
  auto it = m_mapLoveLetter.find(letterID);
  if(it != m_mapLoveLetter.end())
    return;

  LoveLetterData data;
  data.set_sendusername(sendUserName);
  data.set_bg(bg);
  data.set_configid(configID);
  data.set_content(content);

  m_mapLoveLetter.insert(std::make_pair(letterID,data));
  XLOG << "[情书-添加] 添加情书ID: " << accid << id << getProfession() << name << "letterID: " << letterID  << "情书信息: " << data.ShortDebugString() << XEND;
}

bool SceneUser::addLoveLetterItem(DWORD letterID)
{
  auto it = m_mapLoveLetter.find(letterID);
  if(it == m_mapLoveLetter.end())
  {
    XERR << "[道具-添加] 得到情书失败: 未找到对应ID " << accid << id << getProfession() << name << "letterID: " << letterID << XEND;
    return false;
  }

  ItemData oData;
  ItemInfo* stInfo = oData.mutable_base();
  const SLoveLetter* pCFG = TableManager::getMe().getLoveLetterCFG(it->second.configid());
  if(pCFG == nullptr)
  {
    XERR << "[道具-添加] 得到情书失败: 未找到配置ID " << accid << id << getProfession() << name << "letterID: " << letterID << "dwID" << it->second.configid() << XEND;
    return false;
  }
  DWORD dwItem = 0;
  if(pCFG->getType() == ELETTERTYPE_CONSTELLATION)
    dwItem = MiscConfig::getMe().getLetterCFG().dwConstellation;
  else if(pCFG->getType() == ELETTERTYPE_CHRISTMAS)
    dwItem = MiscConfig::getMe().getLetterCFG().dwChristmas;
  else if(pCFG->getType() == ELETTERTYPE_SPRING)
    dwItem = MiscConfig::getMe().getLetterCFG().dwSpring;
  else
    return false;

  stInfo->set_id(dwItem);
  stInfo->set_count(1);
  stInfo->set_source(ESOURCE_PACKAGE);

  LoveLetterData* data = oData.mutable_letter();
  data->set_sendusername(it->second.sendusername());
  data->set_bg(it->second.bg());
  data->set_configid(it->second.configid());
  data->set_content(it->second.content());

  if(getPackage().addItem(oData, EPACKMETHOD_AVAILABLE) == false)
  {
    XERR << "[道具-添加] 得到情书失败: " << accid << id << getProfession() << name << "letterID: " << letterID  << "情书信息: " << data->ShortDebugString() << XEND;
    return false;
  }

  SaveLoveLetterCmd cmd;
  cmd.set_dwid(letterID);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);

  m_mapLoveLetter.erase(it);

  XLOG << "[道具-添加] 得到情书成功: " << accid << id << getProfession() << name << "letterID: " << letterID  << "itemid: " << dwItem << "情书信息: " << data->ShortDebugString() << XEND;
  return true;
}

void SceneUser::sendUpyunUrl()
{
  DownloadSceneryPhotoUserCmd message;

  for (DWORD type = EALBUMTYPE_MIN + 1; type < EALBUMTYPE_MAX; ++type)
  {
    string dirname;
    if (getAlbumName(static_cast<EAlbumType>(type), dirname) == false)
    {
      XERR << "[玩家-upyun下载路径]" << accid << id << getProfession() << name << "获取" << type << "路劲失败" << XEND;
      continue;
    }

    std::stringstream stream;
    stream.str("");
    if (thisServer->isOuter())
      stream << "game/" << dirname << thisServer->getRegionID();
    else
      stream << "debug/" << dirname << thisServer->getRegionID();

    UpyunUrl* pUrl = message.add_urls();
    if (pUrl == nullptr)
      continue;

    pUrl->set_type(static_cast<EAlbumType>(type));

    if(EALBUMTYPE_GUILD_ICON == static_cast<EAlbumType>(type))
    {
      stream << "/guild/";
      pUrl->set_char_url(stream.str());
      pUrl->set_acc_url(stream.str());
    }
    else if (EALBUMTYPE_WEDDING == static_cast<EAlbumType>(type))
    {
      pUrl->set_char_url(stream.str());
    }
    else
    {
      stringstream url;
      url << stream.str() << "/user";
      pUrl->set_char_url(url.str());

      url.str("");
      url << stream.str() << "/acc";
      pUrl->set_acc_url(url.str());
    }
  }

  PROTOBUF(message, send, len);
  sendCmdToMe(send, len);

  XLOG << "[玩家-upyun下载路径]" << accid << id << getProfession() << name << "同步" << message.ShortDebugString() << XEND;
}

void SceneUser::sendUpyunAuthorization()
{
  UpyunAuthorizationCmd message;
  message.set_authvalue(CommonConfig::m_strUpyunAuthValue);
  PROTOBUF(message, send, len);
  sendCmdToMe(send, len);
}

bool SceneUser::getAlbumName(EAlbumType type, string& name)
{
  switch (type)
  {
    case EALBUMTYPE_SCENERY:
      name = "scenery/"; break;
    case EALBUMTYPE_PHOTO:
      name = "photo/"; break;
    case EALBUMTYPE_GUILD_ICON:
      name = "icon/"; break;
    case EALBUMTYPE_WEDDING:
      name = "wedding/"; break;
    default:
      return false;
  }
  return true;
}

void SceneUser::sendTransformPreData()
{
  if(getTransform().isInTransform() == false)
    return;

  TransformPreDataCmd cmd;
  add_data(cmd.add_datas(), EUSERDATATYPE_BODY, m_oUserSceneData.getBody(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_HAIRCOLOR, m_oHair.getRealHairColor());
  add_data(cmd.add_datas(), EUSERDATATYPE_LEFTHAND, m_oUserSceneData.getLefthand(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_RIGHTHAND, m_oUserSceneData.getRighthand(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_HAIR, m_oHair.getRealHair());
  add_data(cmd.add_datas(), EUSERDATATYPE_BACK, m_oUserSceneData.getBack(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_HEAD, m_oUserSceneData.getHead(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_MOUNT, m_oUserSceneData.getMount(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_FACE, m_oUserSceneData.getFace(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_TAIL, m_oUserSceneData.getTail(true));
  add_data(cmd.add_datas(), EUSERDATATYPE_EYE, m_oEye.getCurID());

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

void SceneUser::soundEffect(DWORD dwId)
{
  const SSoundEffect* pCfg = TableManager::getMe().getSoundEffectCFG(dwId);
  if (pCfg == nullptr)
  {
    XERR << "[播放音效] 失败，找不到音效id" << id << dwId << XEND;
    return;
  }
  GMCommandRuler::sound_effect(this, pCfg->getGMData());
}

void SceneUser::handleFollower(const Cmd::InviteFollowUserCmd cmd)
{
  if(cmd.follow() == true)
  {
    InviteFollowUserCmd message;
    message.set_charid(id);
    PROTOBUF(message, send, len);
    const GTeam& rTeam = getTeam();
    if(cmd.charid() != 0)
    {
      for (auto &m : rTeam.getTeamMemberList())
      {
        if (m.second.charid() == id || m.second.charid() != cmd.charid())
          continue;

        if (thisServer->sendCmdToMe(m.second.charid(), send, len) == false)
          XERR << "[组队-邀请上车] 邀请失败" << id << "name" << name << "邀请" << m.second.charid() << XEND;
        else
          XLOG << "[组队-邀请上车] 邀请成功" << id << "name" << name << "邀请" << m.second.charid() << XEND;
        break;
      }
    }
    else
    {
      for (auto &m : rTeam.getTeamMemberList())
      {
        if (m.second.charid() == id)
          continue;
        if (thisServer->sendCmdToMe(m.second.charid(), send, len) == false)
          XERR << "[组队-一键上车] 邀请失败" << id << "name" << name << "邀请" << m.second.charid() << XEND;
        else
          XLOG << "[组队-一键上车] 邀请成功" << id << "name" << name << "邀请" << m.second.charid() << XEND;
      }
      XLOG << "[组队-一键上车] 邀请队友" << id << "name" << name << XEND;
    }
  }
  else if(cmd.follow() == false && cmd.charid() != 0)
  {
    const GTeam& rTeam = getTeam();
    if(id != rTeam.getLeaderID())
    {
      XERR << "[组队-下车] 断开失败" << id << "name" << name << "不是队长" << cmd.charid() << XEND;
      return;
    }

    SceneUser* pFollower = SceneUserManager::getMe().getUserByID(cmd.charid());
    if(pFollower == nullptr)
    {
      XERR << "[组队-下车] 断开失败" << id << "name" << name << "下车" << cmd.charid() << XEND;
      return;
    }
    pFollower->getUserSceneData().setFollowerID(0);
    XLOG << "[组队-下车] 断开成功" << id << "name" << name << "下车" << cmd.charid() << XEND;
  }
  else
    XERR << "[组队-上下车] 参数错误" << id << "name" << name << "参数: " << cmd.follow() << cmd.charid() << XEND;
}

bool SceneUser::getPracticeReward()
{
  bool blNovice = RoleConfig::getMe().isFirstProfession(getProfession());
  if(blNovice == false)
    return false;

  //const SGlobalActivityCFG* pCFG = MiscConfig::getMe().getGlobalActivityCFG(GACTIVITY_NOVICE_WELFARE);
  const SGlobalActCFG*pCFG = ActivityConfig::getMe().getGlobalActCFG(static_cast<DWORD>(GACTIVITY_NOVICE_WELFARE));
  if (pCFG == nullptr)
    return false;

  DWORD activityid = pCFG->m_dwId;
  bool isOpen = ActivityManager::getMe().isOpen(activityid);
  if(isOpen == false)
  {
    XLOG << "[运营-活动] 萌新福利 " << accid << id << getProfession() << name << "活动未开启" << activityid << XEND;
    return false;
  }

  //DWORD rewardID = pCFG->rewardid;
  DWORD rewardID = pCFG->getParam(0);
  MailManager::getMe().sendMail(id, rewardID);
  XLOG <<  "[运营-活动], 萌新福利 " << name << id << "邮件ID:" << rewardID << XEND;
  return true;
}

void SceneUser::setAction(DWORD id)
{
  if (m_dwActionID == id)
    return;
  m_dwActionID = id;
  if (id == 0)
  {
    m_oWeaponPet.onUserStopAction();
  }
  m_oItemMusic.onActionChange(getAction());
}

void SceneUser::stopSendInactiveLog()
{
  m_oVar.setAccVarValue(EACCVARTYPE_INACTIVE_USER_SEND_COUNT, 9999);
  XLOG << "[日志-活跃玩家] charid"<<id <<"name"<<name << XEND;
}

DWORD SceneUser::getArrowID()
{
  DWORD arrow = m_oPackage.getArrowTypeID();
  if (arrow == 0)
    return 0;
  if (getWeaponType() != ((DWORD)EITEMTYPE_WEAPON_BOW))
    return 0;
  return arrow;
}

DWORD SceneUser::getAppleNum()
{
  if (getScene() == nullptr)
    return 0;
  PollyScene* pScene = dynamic_cast<PollyScene*>(getScene());
  if (pScene == nullptr)
    return 0;
  return pScene->getScore(this);
}

bool SceneUser::isInPollyScene()
{
  if (getScene() && getScene()->isPollyScene())
    return true;
  return false;
}


bool SceneUser::isEquipForceOff(DWORD pos)
{
  if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX || EEquipPos_IsValid(pos) == false)
    return false;
  return m_oPackage.isEquipForceOff(static_cast<EEquipPos>(pos));
}

void SceneUser::processLoveLetter(const Cmd::LoveLetterSessionCmd cmd)
{
  bool blcheck = m_oPackage.getPackage(EPACKTYPE_MAIN)->checkItemCount(cmd.itemguid());
  if(blcheck == false)
    return;

  MainPackage* pPackage = dynamic_cast<MainPackage*>(m_oPackage.getPackage(EPACKTYPE_MAIN));
  if (pPackage == nullptr)
    return;
  ItemBase* pBase = pPackage->getItem(cmd.itemguid());
  if (pBase == nullptr)
    return;
  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr)
    return;

  if (pCFG->eItemType == EITEMTYPE_FRIEND)
  {
    if(checkOtherRelation(cmd.targets(), ESOCIALRELATION_FRIEND) == false)
    {
      MsgManager::sendMsg(id, 25401);
      return;
    }
  }

  m_oPackage.getPackage(EPACKTYPE_MAIN)->reduceItem(cmd.itemguid(), ESOURCE_USEITEM);

  ItemInfo stInfo;
  if(cmd.type() == ELETTERTYPE_CHRISTMAS)
    stInfo.set_id(MiscConfig::getMe().getLetterCFG().dwChristmasReward);
  else if(cmd.type() == ELETTERTYPE_SPRING)
    stInfo.set_id(MiscConfig::getMe().getLetterCFG().dwSpringReward);
  else
    return;
  stInfo.set_source(ESOURCE_REWARD);
  getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);

  SceneUser* pTarget = SceneUserManager::getMe().getUserByID(cmd.targets());
  if(pTarget == nullptr)
  {
    LoveLetterSendSessionCmd session;
    session.set_charid(cmd.targets());
    session.set_sendname(name);
    session.set_content(cmd.content());
    session.set_type(cmd.type());
    PROTOBUF(session, ssend, slen);
    thisServer->sendCmdToSession(ssend, slen);
  }
  else
  {
    LoveLetterNtf scmd;
    scmd.set_name(name);
    scmd.set_content(cmd.content());
    scmd.set_type(cmd.type());
    DWORD dwID = TableManager::getMe().randLoveLetter(cmd.type());
    scmd.set_configid(dwID);
    DWORD letterID = GuidManager::getMe().getNextLetterID();
    scmd.set_letterid(letterID);
    //cmd.set_bg(bg);
    pTarget->addLoveLetter(letterID, name, scmd.bg(), dwID, cmd.content());
    PROTOBUF(scmd, send, len);
    pTarget->sendCmdToMe(send, len);

    pTarget->getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);
  }
}

void SceneUser::processLoveLetterSend(const LoveLetterSendSessionCmd& cmd)
{
  ItemInfo stInfo;
  if(cmd.type() == ELETTERTYPE_CHRISTMAS)
    stInfo.set_id(MiscConfig::getMe().getLetterCFG().dwChristmasReward);
  else if(cmd.type() == ELETTERTYPE_SPRING)
    stInfo.set_id(MiscConfig::getMe().getLetterCFG().dwSpringReward);
  else
    return;
  stInfo.set_source(ESOURCE_REWARD);
  getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);

  LoveLetterNtf scmd;
  scmd.set_name(cmd.sendname());
  scmd.set_content(cmd.content());
  scmd.set_type(cmd.type());
  DWORD dwID = TableManager::getMe().randLoveLetter(cmd.type());
  scmd.set_configid(dwID);
  DWORD letterID = GuidManager::getMe().getNextLetterID();
  scmd.set_letterid(letterID);
  //cmd.set_bg(bg);
  PROTOBUF(scmd, send, len);
  sendCmdToMe(send, len);

  addLoveLetter(letterID, cmd.sendname(), scmd.bg(), dwID, cmd.content());
}

DWORD SceneUser::getMapTeamPros()
{
  if (getTeamID() == 0)
    return 1;
  Scene* pScene = getScene();
  if (!pScene)
    return 1;
  std::set<EProfession> setPro;
  for (auto &m : m_oGTeam.getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    if (rMember.charid() == id)
    {
      setPro.insert(getProfession());
    }
    else
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rMember.charid());
      if (pUser && pUser->getScene() && pUser->getScene() == pScene)
        setPro.insert(pUser->getProfession());
    }
  }
  if (setPro.size() == 1)
    return 1;

  return RoleConfig::getMe().getProfessionNum(setPro);
}

bool SceneUser::hasMonthCard()
{
  return m_oDeposit.hasMonthCard();
}

bool SceneUser::isBattleTired()
{
  return getUserSceneData().haveEnoughBattleTime() == false;
}

bool SceneUser::isSkillEnable(DWORD skillid)
{
  return m_pCurFighter && m_pCurFighter->getSkill().isSkillEnable(skillid);
}

void SceneUser::addEquipAttrAction()
{
  BasePackage* pack = getPackage().getPackage(EPACKTYPE_EQUIP);
  if (pack == nullptr)
    return;
  TVecSortItem equips;
  pack->getEquipItems(equips);
  for (auto& item : equips)
  {
    ItemEquip* equip = dynamic_cast<ItemEquip*>(item);
    if (equip == nullptr)
      continue;
    DWORD duration = equip->getRestBreakDuration();
    if (duration > 0)
      getPackage().addEquipAttrAction(equip->getGUID(), duration);
  }
}

DWORD SceneUser::getPeakEffect()
{
  if(m_oQuest.isSubmit(QUEST_PEAK_EFFECT_ID) == true)
    return 1;

  return 0;
}

void SceneUser::sendOpenBuildingGateMsg()
{
  const GuildSMember* member = getGuild().getMember(id);
  if (member && member->buildingeffect())
  {
    GuildScene* scene = dynamic_cast<GuildScene*>(getScene());
    if (scene)
    {
      MsgManager::sendMsg(id, 3713);
      BuildingEffectGuildSCmd cmd;
      cmd.set_charid(id);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToSession(send, len);
    }
  }
}

bool SceneUser::inGuildZone()
{
  return thisServer->getZoneID() == m_oGuild.zoneid();
}

bool SceneUser::inSuperGvg()
{
  return m_oGuild.inSuperGvg();
}

void SceneUser::updateArtifact()
{
  BasePackage* mainpack = getPackage().getPackage(EPACKTYPE_MAIN);
  if (mainpack == nullptr)
    return;
  EquipPackage* equippack = dynamic_cast<EquipPackage*>(getPackage().getPackage(EPACKTYPE_EQUIP));
  if (equippack == nullptr)
    return;

  map<string, ItemEquip*> equipartifacts;
  for (auto pos : ItemConfig::getMe().getArtifactPos())
  {
    ItemEquip* equip = equippack->getEquip(pos);
    if (equip)
      equipartifacts[equip->getGUID()] = equip;
  }

  bool delall = false;
  if (getGuild().id() == 0)
  {
    delall = true;
  }
  else
  {
    const TMapGuildArtifact& artifacts = getGuild().getArtifactList();
    if (artifacts.empty())
    {
      delall = true;
    }
    else
    {
      vector<const GuildArtifactItem*> needadd;
      for (auto& v : artifacts)
      {
        if (v.second.ownerid() != id)
          continue;
        if (equipartifacts.find(v.second.guid()) != equipartifacts.end())
          continue;
        const TSetItemBase& items = mainpack->getItemBaseList(v.second.itemid());
        bool add = true;
        for (auto& item : items)
        {
          if (item->getGUID() == v.second.guid())
          {
            add = false;
            break;
          }
        }
        if (add)
          needadd.push_back(&v.second);
      }
      if (needadd.empty() == false)
      {
        TVecItemInfo items;
        XLOG << "[神器数据初始化]" << accid << id << name << "包裹添加神器:";
        for (auto& artifact : needadd)
        {
          ItemInfo item;
          item.set_guid(artifact->guid());
          item.set_id(artifact->itemid());
          item.set_count(1);
          items.push_back(item);
          XLOG << artifact->itemid() << artifact->guid();
        }
        if (mainpack->addItem(items, EPACKMETHOD_NOCHECK))
          XLOG << "道具添加成功" << XEND;
        else
          XLOG << "道具添加失败" << XEND;
      }
    }
  }

  const TSetString& allguids = mainpack->getAllArtifact();
  if (allguids.empty() == false)
  {
    TSetString items;
    for (auto& aguid : allguids)
    {
      if (delall == false)
      {
        const GuildArtifactItem* a = getGuild().getArtifact(aguid);
        if (a && a->ownerid() == id)
          continue;
      }
      items.insert(aguid);
    }
    if (items.empty() == false)
    {
      for (auto& item : items)
        mainpack->reduceItem(item, ESOURCE_ARTIFACT_DISTRIBUTE);
      XLOG << "[神器数据初始化]" << accid << id << name << "包裹删除神器:";
      for (auto& guid : items)
        XLOG << guid;
      XLOG << "道具删除成功" << XEND;
    }
  }

  if (equipartifacts.empty() == false)
  {
    for (auto& v : equipartifacts)
    {
      bool off = false;
      if (getGuild().id() == 0)
      {
        off = true;
      }
      else
      {
        const GuildArtifactItem* a = getGuild().getArtifact(v.second->getGUID());
        if (a == nullptr || a->ownerid() != id)
          off = true;
      }
      if (off)
      {
        if (getPackage().equipOff(v.second->getGUID()))
        {
          string guid = v.second->getGUID();
          DWORD id = v.second->getTypeID();
          mainpack->reduceItem(guid, ESOURCE_ARTIFACT_DISTRIBUTE);
          XLOG << "[神器数据初始化]" << accid << id << name << "装备删除神器:" << id << guid << "删除成功" << XEND;
        }
        else
        {
          XLOG << "[神器数据初始化]" << accid << id << name << "装备删除神器:" << v.second->getTypeID() << v.second->getGUID() << "删除失败" << XEND;
        }
      }
    }
    setCollectMark(ECOLLECTTYPE_EQUIP);
  }
}

void SceneUser::updateArtifact(const ArtifactUpdateGuildSCmd& cmd)
{
  BasePackage* mainpack = getPackage().getPackage(EPACKTYPE_MAIN);
  if (mainpack == nullptr)
    return;
  EquipPackage* equippack = dynamic_cast<EquipPackage*>(getPackage().getPackage(EPACKTYPE_EQUIP));
  if (equippack == nullptr)
    return;

  map<string, ItemEquip*> artifacts;
  for (auto pos : ItemConfig::getMe().getArtifactPos())
  {
    ItemEquip* equip = equippack->getEquip(pos);
    if (equip)
      artifacts[equip->getGUID()] = equip;
  }

  vector<const GuildArtifactItem*> pending;
  for (int i = 0; i < cmd.itemupdates_size(); ++i)
  {
    if (id != cmd.itemupdates(i).ownerid())
      continue;
    if (artifacts.find(cmd.itemupdates(i).guid()) != artifacts.end())
      continue;
    bool had = false;
    const TSetItemBase& items = mainpack->getItemBaseList(cmd.itemupdates(i).itemid());
    for (auto& item : items)
      if (item->getGUID() == cmd.itemupdates(i).guid())
      {
        had = true;
        break;
      }
    if (!had)
      pending.push_back(&(cmd.itemupdates(i)));
  }
  if (pending.empty() == false)
  {
    TVecItemInfo items;
    XLOG << "[神器数据更新]" << accid << id << name << "包裹添加神器:";
    for (auto& artifact : pending)
    {
      ItemInfo item;
      item.set_guid(artifact->guid());
      item.set_id(artifact->itemid());
      item.set_count(1);
      items.push_back(item);
      XLOG << artifact->itemid() << artifact->guid();
    }
    if (mainpack->addItem(items, EPACKMETHOD_NOCHECK))
      XLOG << "道具添加成功" << XEND;
    else
      XLOG << "道具添加失败" << XEND;
  }

  pending.clear();
  for (int i = 0; i < cmd.itemupdates_size(); ++i)
  {
    if (cmd.itemupdates(i).ownerid())
      continue;
    auto it = artifacts.find(cmd.itemupdates(i).guid());
    if (it != artifacts.end() && it->second)
    {
      if (getPackage().equipOff(it->second->getGUID()))
      {
        string guid = it->second->getGUID();
        DWORD id = it->second->getTypeID();
        mainpack->reduceItem(guid, ESOURCE_ARTIFACT_DISTRIBUTE);
        it->second = nullptr;
        XLOG << "[神器数据更新]" << accid << id << name << "装备删除神器:" << id << guid << "删除成功" << XEND;
      }
      else
      {
        XLOG << "[神器数据更新]" << accid << id << name << "装备删除神器:" << it->second->getTypeID() << it->second->getGUID() << "删除失败" << XEND;
      }
      continue;
    }
    const TSetItemBase& items = mainpack->getItemBaseList(cmd.itemupdates(i).itemid());
    for (auto& item : items)
      if (item->getGUID() == cmd.itemupdates(i).guid())
      {
        pending.push_back(&(cmd.itemupdates(i)));
        break;
      }
  }
  if (pending.empty() == false)
  {
    TSetString items;
    XLOG << "[神器数据更新]" << accid << id << name << "包裹删除神器:";
    for (auto& artifact : pending)
    {
      items.insert(artifact->guid());
      XLOG << artifact->itemid() << artifact->guid();
    }
    XLOG << "道具删除成功" << XEND;
    for (auto& guid : items)
      mainpack->reduceItem(guid, ESOURCE_ARTIFACT_DISTRIBUTE);
  }
}

bool SceneUser::isRealAuthorized() 
{
  if (thisServer->isOuter())
    return m_bRealAuthorized; 
  else 
    return true;
}
// 周年庆-好友回归
void SceneUser::sendRecallReward()
{
  if (ActivityManager::getMe().isOpen(static_cast<DWORD>(GACTIVITY_RECALL)) == false)
    return;

  const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
  DWORD dwNow = xTime::getCurSec();
  if (m_oUserSceneData.getOfflineTime() == 0 || dwNow <= m_oUserSceneData.getOfflineTime() || dwNow - m_oUserSceneData.getOfflineTime() < rCFG.dwNeedOffline)
    return;

  const MailBase* pBase = TableManager::getMe().getMailCFG(rCFG.dwRecallMailID);
  if (pBase == nullptr)
  {
    XERR << "[玩家-回归]" << accid << id << getProfession() << name << "回归奖励邮件" << rCFG.dwRecallMailID << "未在 Table_Mail.txt 表中找到" << XEND;
    return;
  }
  if (m_oBuff.add(rCFG.dwRewardBuffID) == true)
  {
    for (auto &v : rCFG.vecRewardBuff2Layer)
      m_oBuff.addLayers(v.first, v.second);
  }

  if (MailManager::getMe().sendMail(id, rCFG.dwRecallMailID) == false)
  {
    XERR << "[玩家-回归]" << accid << id << getProfession() << name << "发送回归奖励邮件" << rCFG.dwRecallMailID << "失败" << XEND;
    return;
  }

  XLOG << "[玩家-回归]" << accid << id << getProfession() << name
    << "离线时间长达" << (dwNow - m_oUserSceneData.getOfflineTime()) << "秒,获得buff" << rCFG.dwRewardBuffID << "奖励邮件" << rCFG.dwRecallMailID << XEND;
}

void SceneUser::getFirstShareReward()
{
  if (ActivityManager::getMe().isOpen(static_cast<DWORD>(GACTIVITY_RECALL)) == false)
    return;

  const SGlobalActCFG* pCFG = ActivityConfig::getMe().getGlobalActCFG(static_cast<DWORD>(GACTIVITY_RECALL));
  if (pCFG == nullptr)
  {
    XERR << "[玩家-回归分享]" << accid << id << getProfession() << name << "活动" << GACTIVITY_RECALL << "未在 Table_GlobalActivity.txt 表中找到" << XEND;
    return;
  }

  DWORD dwValue = m_oVar.getVarValue(EVARTYPE_FIRST_SHARE);
  DWORD dwHigh = (dwValue & 0xFFFF0000) >> 16;
  if (dwHigh != pCFG->getParam(0))
  {
    m_oVar.setVarValue(EVARTYPE_FIRST_SHARE, 0);
    XLOG << "[玩家-回归分享]" << accid << id << getProfession() << name << "活动从" << dwHigh << "index :" << pCFG->getParam(0) << "重置领取状态" << XEND;
  }

  if (m_oVar.getVarValue(EVARTYPE_FIRST_SHARE) != 0)
  {
    XERR << "[玩家-回归分享]" << accid << id << getProfession() << name << "活动 index :" << dwHigh << "已领取过" << XEND;
    return;
  }

  const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();

  TVecItemInfo vecItems;
  if (RewardManager::roll(rCFG.dwFirstShareReward, this, vecItems, ESOURCE_RECALL) == false)
  {
    XERR << "[玩家-回归分享]" << accid << id << getProfession() << name << "活动 index :" << pCFG->getParam(0) << "随机 rewardid :" << rCFG.dwFirstShareReward << "失败" << XEND;
    return;
  }

  if (m_oPackage.addItem(vecItems, EPACKMETHOD_CHECK_WITHPILE) == false)
  {
    MsgManager::sendMsg(id, ESYSTEMMSG_ID_PACK_FULL);
    XERR << "[玩家-回归分享]" << accid << id << getProfession() << name << "活动 index :" << pCFG->getParam(0) << "添加包裹失败" << XEND;
    return;
  }

  dwValue |= (pCFG->getParam(0) << 16);
  dwValue |= 1;
  m_oVar.setVarValue(EVARTYPE_FIRST_SHARE, dwValue);

  XLOG << "[玩家-回归分享]" << accid << id << getProfession() << name << "活动 index :" << pCFG->getParam(0) << "成功领取rewardid :" << rCFG.dwFirstShareReward << "物品" << vecItems << XEND;
}

void SceneUser::onSocialChange()
{
  checkRecallBuff();

  if (m_oSocial.getRelationCount(ESOCIALRELATION_TUTOR) > 0 && m_oQuest.isSubmit(QUEST_GRADUATION) == true)
  {
    bool bSuccess = m_oQuest.abandonGroup(QUEST_GRADUATION, true);
    if (bSuccess)
    {
      m_oQuest.acceptQuest(QUEST_GRADUATION);
      XLOG << "[玩家-导师]" << accid << id << getProfession() << name << "任务" << QUEST_GRADUATION << "已完成,还存在导师关系,重置任务成功" << XEND;
    }
    else
    {
      XERR << "[玩家-导师]" << accid << id << getProfession() << name << "任务" << QUEST_GRADUATION << "已完成,还存在导师关系,重置任务失败" << XEND;
    }
  }

  if(m_oSocial.getRelationCount(ESOCIALRELATION_STUDENT) != 0)
  {
    m_oServant.onAppearEvent(ETRIGGER_OWN_STUDENT);
    m_oServant.onGrowthAppearEvent(ETRIGGER_OWN_STUDENT);
    m_oServant.onGrowthFinishEvent(ETRIGGER_OWN_STUDENT);
  }
  if(m_oSocial.getRelationCount(ESOCIALRELATION_TUTOR) != 0)
    m_oServant.onFinishEvent(ETRIGGER_OWN_TUTOR);
}

void SceneUser::checkRecallBuff()
{
  bool findrecall = false;
  const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
  if (m_oGTeam.getTeamID())
  {
    for (auto &m : m_oGTeam.getTeamMemberList())
    {
      if (m.second.online() == false)
        continue;
      if (m_oSocial.checkRelation(m.first, ESOCIALRELATION_RECALL))
      {
        m_oBuff.add(rCFG.dwRecallBuffID);
        findrecall = true;
        break;
      }
    }
  }

  if (!findrecall && m_oBuff.haveBuff(rCFG.dwRecallBuffID))
    m_oBuff.del(rCFG.dwRecallBuffID);
}

void SceneUser::checkWeddingBuff()
{
  bool findwedding = false;
  DWORD buffid = MiscConfig::getMe().getWeddingMiscCFG().dwWeddingTeamBuff;
  if (m_oUserWedding.isMarried() && m_oGTeam.getTeamID())
  {
    for (auto &m : m_oGTeam.getTeamMemberList())
    {
      if (m.second.online() == false)
        continue;
      if (m_oUserWedding.checkMarryRelation(m.first))
      {
        m_oBuff.add(buffid);
        findwedding = true;
        break;
      }
    }
  }

  if (!findwedding && m_oBuff.haveBuff(buffid))
    m_oBuff.del(buffid);
}

void SceneUser::notifyInviteeWeddingStart()
{
  BasePackage* pack = getPackage().getPackage(EPACKTYPE_MAIN);
  if (pack == nullptr)
    return;
  const TSetItemBase& items = pack->getItemBaseList(MiscConfig::getMe().getWeddingMiscCFG().dwInvitationItemID);
  if (items.empty())
    return;
  for (auto& item : items)
  {
    ItemWedding* i = dynamic_cast<ItemWedding*>(item);
    if (i && i->isNotifyWeddingStart())
    {
      i->setNotified();
      InviteeWeddingStartNtfUserCmd ntf;
      ntf.set_itemguid(i->getGUID());
      PROTOBUF(ntf, send, len);
      sendCmdToMe(send, len);
    }
  }
}

void SceneUser::getAllFriendNpcs(std::list<SceneNpc*>& list) // 宠物, 猫, 生命体
{
  m_oWeaponPet.getPetNpcs(list);
  SceneNpc* pet = m_oUserPet.getPetNpc();
  if (pet)
    list.push_back(pet);

  SceneNpc* being = m_oUserBeing.getCurBeingNpc();
  if (being)
    list.push_back(being);
}

EError SceneUser::changeGender()
{
  UserWedding& rWedding = getUserWedding();
  UserSceneData& rSceneData = getUserSceneData();

  EGender eOriGender = rSceneData.getGender();
  EGender eDestGender = eOriGender == EGENDER_MALE ? EGENDER_FEMALE : EGENDER_MALE;

  //目标性别与当前职业不匹配
  const SRoleBaseCFG* pRoleCfg = RoleConfig::getMe().getRoleBase(getProfession());
  if(!pRoleCfg)
  {
    XERR << "[玩家-变性]" << accid << id << getProfession() << name << "由" << eOriGender << "变为" << eDestGender << "失败,获取class表配置失败!"<< XEND;
    return EERROR_FAIL;
  }

  if(!pRoleCfg->checkGender(eDestGender))
  {
    XERR << "[玩家-变性]" << accid << id << getProfession() << name << "由" << eOriGender << "变为" << eDestGender << "失败,目标性别与当前职业不匹配!"<< XEND;
    return EERROR_FAIL;
  }

  if (now() < rWedding.reserveTime() + MiscConfig::getMe().getWeddingMiscCFG().dwEngageInviteOverTime)
  {
    XERR << "[玩家-变性]" << accid << id << getProfession() << name << "由" << eOriGender << "变为" << eDestGender << "失败,处于订婚交互阶段" << XEND;
    return EERROR_FAIL;
  }
  const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_MATCH, id);
  DWORD dwMatch = 0;
  RedisManager::getMe().getData<DWORD>(key, dwMatch);
  if (dwMatch != 0)
  {
    XERR << "[玩家-变性]" << accid << id << getProfession() << name << "由" << eOriGender << "变为" << eDestGender << "失败,处理导师匹配阶段" << XEND;
    return EERROR_FAIL;
  }

  rSceneData.setGender(eDestGender);
  refreshDataAtonce();

  getHairInfo().resetHair();
  getEye().resetEye();
  getPackage().equipOffInValidEquip();

  setDataMark(EUSERDATATYPE_BODY);
  refreshDataAtonce();
  XLOG << "[玩家-变性]" << accid << id << getProfession() << name << "由" << eOriGender << "变为" << eDestGender << XEND;

  getEvent().onChangeGender();

  return EERROR_SUCCESS;
}

PetWork& SceneUser::getPetWork() { return *m_pPetWork; }
SceneShop& SceneUser::getSceneShop() { return *m_pSceneShop; }
Menu& SceneUser::getMenu() { return *m_pMenu; }

bool SceneUser::addOperateRewardVar(DWORD eType)
{
  if (EOperateType_IsValid(eType) == false)
    return false;

  DWORD dwVar = m_oVar.getAccVarValue(EACCVARTYPE_OPERATE_REWARD);
  DWORD dwTypeVar = 1 << eType;
  if(dwVar & dwTypeVar)
    return false;

  dwVar += dwTypeVar;
  m_oVar.setAccVarValue(EACCVARTYPE_OPERATE_REWARD, dwVar);
  SyncOperateRewardToSession();
  return true;
}

void SceneUser::SyncOperateRewardToSession()
{
  DWORD dwVar = m_oVar.getAccVarValue(EACCVARTYPE_OPERATE_REWARD);

  SyncOperateRewardSessionCmd cmd;
  cmd.set_charid(id);
  cmd.set_var(dwVar);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

void SceneUser::patch_OperateReward()
{
  for(DWORD i = EOperateType_Summer; i <= EOperateType_MonthCard; ++i)
  {
    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_OPERATE_RWARD, accid, i);
    DWORD count = 0;
    RedisManager::getMe().getData(key, count);
    if (count >= 1)
      addOperateRewardVar(i);
  }
}
bool SceneUser::checkOtherRelation(QWORD tarid, ESocialRelation eType)
{
  SceneUser* pTarget = SceneUserManager::getMe().getUserByID(tarid);
  if (pTarget != nullptr)
  {
    GSocial& rSocial = pTarget->getSocial();
    if(rSocial.checkRelation(id, eType) == true)
      return true;
  }

  return false;
}

void SceneUser::handleTwinsAction(const TwinsActionUserCmd cmd)
{
  switch (cmd.etype())
  {
    case ETWINS_OPERATION_REQUEST:
      {
        SceneUser* user = SceneUserManager::getMe().getUserByID(cmd.userid());
        if(user == nullptr || check2PosInNine(user) == false)
          return;
        if (user->m_oHands.has())
        {
          MsgManager::sendMsg(id, 394);
          return;
        }
        if (m_oHands.has())
        {
          MsgManager::sendMsg(id, 395);
          return;
        }
        if(user->getTwinsID() != 0 && user->getRequestTime() >= xTime::getCurSec())
        {
          MsgManager::sendMsg(id, 407);
          return;
        }
        const SActionAnimBase* pCFG = TableManager::getMe().getActionAnimCFG(cmd.actionid());
        if (pCFG == nullptr)
          return;

        m_qwTwinsID = cmd.userid();
        m_dwTwinsActionID = cmd.actionid();
        m_bSponsor = true;
        m_dwRequestTime = xTime::getCurSec() + MiscConfig::getMe().getTeamCFG().dwInviteTime;

        Cmd::TwinsActionUserCmd message;
        message.set_userid(id);
        message.set_actionid(cmd.actionid());
        message.set_etype(ETWINS_OPERATION_REQUEST);
        PROTOBUF(message, send, len);
        user->sendCmdToMe(send, len);
      }
      break;
    case ETWINS_OPERATION_AGREE:
      {
        SceneUser* user = SceneUserManager::getMe().getUserByID(cmd.userid());
        if(user == nullptr || check2PosInNine(user) == false)
          return;
        if(user->getTwinsID() != id)
          return;

        m_qwTwinsID = user->id;
        m_dwTwinsActionID = user->getTwinsActionID();
        m_dwRequestTime = xTime::getCurSec() + MiscConfig::getMe().getTeamCFG().dwInviteTime;

        notifyTwinsAction(user->id, user->getTwinsActionID(), false);
        user->notifyTwinsAction(id, user->getTwinsActionID(), true);
      }
      break;
    case ETWINS_OPERATION_DISAGREE:
      {
        SceneUser* user = SceneUserManager::getMe().getUserByID(cmd.userid());
        if(user == nullptr)
          return;
        user->setTwinsID(0);
        user->setTwinsActionID(0);
        user->setTwinsSponsor(false);
        user->setRequestTime(0);

        MsgManager::sendMsg(cmd.userid(), 392, MsgParams(name));
      }
      break;
    default:
      break;
  }
}

void SceneUser::notifyTwinsAction(QWORD userid, DWORD actionid, bool sponsor)
{
  if (m_oHands.has())
  {
    m_oHands.breakup();
  }
  if (m_oWeaponPet.haveHandCat())
  {
    m_oWeaponPet.breakHand();
  }
  if (m_oUserPet.handPet())
    m_oUserPet.breakHand();

  getPackage().ride(ERIDETYPE_OFF);

  Cmd::TwinsActionUserCmd cmd;
  cmd.set_userid(userid);
  cmd.set_actionid(actionid);
  cmd.set_etype(ETWINS_OPERATION_COMMIT);
  cmd.set_sponsor(sponsor);
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);

  if(sponsor == false)
  {
    FollowerUser scmd;
    scmd.set_userid(userid);
    scmd.set_etype(EFOLLOWTYPE_TWINSACTION);
    PROTOBUF(scmd, ssend, slen);
    sendCmdToMe(ssend, slen);
  }

  setDataMark(EUSERDATATYPE_TWINS_ACTIONID);
  setDataMark(EUSERDATATYPE_HANDID);
  refreshDataAtonce();

  XLOG << "[双人动作] 通知: " << accid << id << getProfession() << name << "发起人: " << sponsor << "actionid: " << m_dwTwinsActionID << "hander: " << m_qwTwinsID << XEND;
}

void SceneUser::changeTwinsStatus(QWORD userid)
{
  SceneUser* user = SceneUserManager::getMe().getUserByID(userid);
  if (user == nullptr || user->getScene() != getScene())
    return;
  if (getDistance(getPos(), user->getPos()) > MiscConfig::getMe().getSystemCFG().dwHandRange)
    return;

  user->setTwinsClose(true);

  HandStatusUserCmd cmd;
  cmd.set_masterid(userid);
  cmd.set_followid(id);
  cmd.set_type(1);

  PROTOBUF(cmd, send, len);
  sendCmdToNine(send, len);

  m_bTiwnsClose = true;

  XLOG << "[双人动作] 牵手: " << accid << id << getProfession() << name << "发起人: " << user->id << "name: " << user->name << XEND;
}

void SceneUser::onTwinsMove()
{
  if(m_bTiwnsClose == true)
  {
    if(m_bSponsor == false)
    {
      FollowerUser cmd;
      cmd.set_userid(0);
      cmd.set_etype(EFOLLOWTYPE_TWINSACTION);
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
    }

    QWORD dwTwins = m_qwTwinsID;
    m_bTiwnsClose = false;
    m_qwTwinsID = 0;
    m_dwTwinsActionID = 0;
    m_bSponsor = false;
    m_dwRequestTime = 0;
    setDataMark(EUSERDATATYPE_HANDID);
    setDataMark(EUSERDATATYPE_TWINS_ACTIONID);
    refreshDataAtonce();

    SceneUser* user = SceneUserManager::getMe().getUserByID(dwTwins);
    if(user != nullptr && user->getTwinsID() != 0)
      user->onTwinsMove();
    XLOG << "[双人动作] : 断开" << accid << id << getProfession() << name << XEND;
  }
}

DWORD SceneUser::getEquipedWeapon()
{
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_oPackage.getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return 0;
  ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_WEAPON);
  if (pEquip == nullptr)
    return 0;
  return pEquip->getTypeID();
}

bool SceneUser::isRide(DWORD dwId)
{
  return getEquipID(EEQUIPPOS_MOUNT) == dwId;
}

bool SceneUser::isPartnerID(DWORD dwId)
{
  if (0 == dwId  && 0 == m_oPet.getPartnerID())
    return false;
  return m_oPet.getPartnerID() == dwId;
}

UserRecords& SceneUser::getUserRecords() { return *m_pUserRecord; }

bool SceneUser::checkProfessionBuy()
{
  if(getEvo() >= 3)
    return true;
  else if(m_oProfession.getProfessionMax()%10 >= 3)
    return true;

  return false;
}

// 购买分支
bool SceneUser::buyProfession(DWORD dwBranch)
{
  // 检测分支是否存在
  // 检测分支是否已购买
  // 检测金钱是否足够
  // 扣除金钱
  // 添加分支
  if(!m_pCurFighter)
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "m_pCurFighter == nullptr" << XEND;
    return false;
  }

  if(!checkMapForChangeProfession())
    return false;

  if(!checkProfessionBuy())
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "profession not enough!" << XEND;
    return false;
  }

  const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(dwBranch);
  if(!pBranchCfg)
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "get SBranchCFG failed! branch:" << dwBranch << XEND;
    return false;
  }

  if(!pBranchCfg->checkGender(getUserSceneData().getGender()))
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "gender failed! branch:" << dwBranch << "gender:" << getUserSceneData().getGender() << XEND;
    return false;
  }

  if(getBranch() == dwBranch)
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "buy same branch! branch:" << dwBranch << XEND;
    return false;
  }

  if(m_oProfession.hasBranch(dwBranch))
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "branch has exist! branch:" << dwBranch << XEND;
    return false;
  }


  DWORD dwBaseProfession = m_oProfession.getBaseProfession();
  if(!dwBaseProfession)
  {
    const SBranchCFG* pCurBranchCfg = RoleConfig::getMe().getBranchCFG(getBranch());
    if(!pCurBranchCfg)
    {
      XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "get SBranchCFG failed! branch:" << getBranch() << XEND;
      return false;
    }

    dwBaseProfession = pCurBranchCfg->baseId;
  }

  // 优先扣除兑换券
  BasePackage* pMainPack = getPackage().getPackage(EPACKTYPE_MAIN);
  if(!pMainPack)
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "get main pack failed!" << XEND;
    return false;
  }

  if (pMainPack->checkItemCount(pBranchCfg->itemId, 1))
  {
    pMainPack->reduceItem(pBranchCfg->itemId, ESOURCE_PROFESSION_BUY, 1);

    MsgParams param;
    param.addNumber(pBranchCfg->itemId);
    param.addNumber(pBranchCfg->itemId);
    param.addNumber(1);
    MsgManager::sendMsg(id, ESYSTEMMSG_ID_ITEM_REDUCE, param);
  }
  else
  {
    if(pBranchCfg->baseId == dwBaseProfession)
    {
      // 同基础职业
      DWORD dwZeny = MiscConfig::getMe().getProfessionMiscCFG().dwZenyCost;
      if(!checkMoney(EMONEYTYPE_SILVER, dwZeny))
      {
        XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "zeny not enough! branch:" << dwBranch << "need zeny:" << dwZeny << XEND;
        return false;
      }

      subMoney(EMONEYTYPE_SILVER, dwZeny, ESOURCE_PROFESSION_BUY);
    }
    else
    {
      // 不同基础职业
      DWORD dwGold = MiscConfig::getMe().getProfessionMiscCFG().dwGoldCost;
      if(!checkMoney(EMONEYTYPE_LOTTERY, dwGold))
      {
        XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "gold not enough! branch:" << dwBranch << "need gold:" << dwGold << XEND;
        return false;
      }

      subMoney(EMONEYTYPE_LOTTERY, dwGold, ESOURCE_PROFESSION_BUY);
    }
  }

  if(!m_oProfession.addBranch(dwBranch))
  {
    XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "add branch failed! branch:" << dwBranch << XEND;
    return false;
  }

  if(pBranchCfg->giftBranch && !m_oProfession.hasBranch(pBranchCfg->giftBranch))
  {
    if(!m_oProfession.addBranch(pBranchCfg->giftBranch))
    {
      XERR << "[玩家角色-购买职业]" << accid << id << getProfession() << name << "add gift branch failed! branch:" << dwBranch << "gift branch:" << pBranchCfg->giftBranch << XEND;
      return false;
    }
  }

  return true;
}

bool SceneUser::changeProfession(DWORD dwBranch)
{
  if(m_oBooth.hasOpen())
    return false;

  if(!checkMapForChangeProfession())
    return false;

  if(!m_oProfession.checkLoadTime())
  {
    MsgManager::sendMsg(id, 25421);
    return false;
  }

  if(!m_oProfession.hasBranch(dwBranch))
  {
    XERR << "[多职业-切换职业]" << accid << id << getProfession() << name << "has not branch! branch:" << dwBranch << XEND;
    return false;
  }

  if(dwBranch == getBranch())
  {
    XERR << "[多职业-切换职业]" << accid << id << getProfession() << name << "change same branch! branch:" << dwBranch << XEND;
    return false;
  }

  const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(dwBranch);
  if(!pBranchCfg)
  {
    XERR << "[玩家角色-切换职业]" << accid << id << getProfession() << name << "get SBranchCFG failed! branch:" << dwBranch << XEND;
    return false;
  }

  if(!pBranchCfg->checkGender(getUserSceneData().getGender()))
  {
    XERR << "[玩家角色-切换职业]" << accid << id << getProfession() << name << "gender failed! branch:" << dwBranch << "gender:" << getUserSceneData().getGender() << XEND;
    return false;
  }

  DWORD dwZeny = MiscConfig::getMe().getProfessionMiscCFG().dwSwitchCost;
  if(!checkMoney(EMONEYTYPE_SILVER, dwZeny))
  {
    XERR << "[玩家角色-切换职业]" << accid << id << getProfession() << name << "zeny not enough! branch:" << dwBranch << "need zeny:" << dwZeny << XEND;
    return false;
  }

  subMoney(EMONEYTYPE_SILVER, dwZeny, ESOURCE_PROFESSION_CHANGE);

  Cmd::EProfession eProfessionOld = getProfession();
  if(!m_oProfession.changeBranch(dwBranch))
  {
    return false;
  }

  m_event.onProfessionChange(eProfessionOld);
  setDataMark(EUSERDATATYPE_CUR_MAXJOB);
  m_oServant.onGrowthFinishEvent(ETRIGGER_PROFESSION_EXCHANGE);
  return true;
}

void SceneUser::syncProfessionData(Cmd::EProfressionDataType type)
{
  // data inform
  UserSyncCmd sync;
  UserNineSyncCmd nine;
  UserDataSync session;
  fetchChangeData(sync, nine, session);

  // to client
  ExchangeProfession cmdme;
  ExchangeProfession cmdnine;

  cmdme.set_guid(id);
  cmdme.set_type(type);
  cmdnine.set_guid(id);
  cmdnine.set_type(type);

  for (int i = 0; i < sync.datas_size(); ++i)
  {
    UserData* pData = cmdme.add_datas();
    if (pData != nullptr)
    {
      pData->CopyFrom(sync.datas(i));
      XDBG << "[玩家-切换职业数据同步]" << accid << id << getProfession() << name << "type:" << pData->type() << "value:" << pData->value() << XEND;
    }
  }
  for (int i = 0; i < sync.attrs_size(); ++i)
  {
    UserAttr* pAttr = cmdme.add_attrs();
    if (pAttr != nullptr)
    {
      pAttr->CopyFrom(sync.attrs(i));
      XDBG << "[玩家-切换职业属性同步]" << accid << id << getProfession() << name << "type:" << pAttr->type() << "value:" <<  pAttr->value() << XEND;
    }
  }
  for (int i = 0; i < sync.pointattrs_size(); ++i)
  {
    UserAttr* pAttr = cmdme.add_pointattrs();
    if (pAttr != nullptr)
    {
      pAttr->CopyFrom(sync.pointattrs(i));
      XDBG << "[玩家-切换职业属性点同步]" << accid << id << getProfession() << name << "type:" << pAttr->type() << "value:" << pAttr->value() << XEND;
    }
  }

  for (int i = 0; i < nine.datas_size(); ++i)
  {
    UserData* pData = cmdnine.add_datas();
    if (pData != nullptr)
    {
      pData->CopyFrom(nine.datas(i));
      XDBG << "[玩家-切换职业九屏数据同步]" << accid << id << getProfession() << name << "type:" << pData->type() << "value:" << pData->value() << XEND;
    }
  }
  for (int i = 0; i < nine.attrs_size(); ++i)
  {
    UserAttr* pAttr = cmdnine.add_attrs();
    if (pAttr != nullptr)
    {
      pAttr->CopyFrom(nine.attrs(i));
      XDBG << "[玩家-切换职业九屏属性同步]" << accid << id << getProfession() << name << "type:" << pAttr->type() << "value:" << pAttr->value() << XEND;
    }
  }

  PROTOBUF(cmdme, sendme, lenme);
  sendCmdToMe(sendme, lenme);

  PROTOBUF(cmdnine, sendnine, lennine);
  sendCmdToNine(sendnine, lennine, id);

  PROTOBUF(session, sendsession, lensession);
  thisServer->sendCmdToSession(sendsession, lensession);
}

// 设置当前分支
void SceneUser::setBranch()
{
  if(!getBranch() && getEvo() >= 2)
  {
    const SRoleBaseCFG* cfg = RoleConfig::getMe().getRoleBase(getProfession());
    if(!cfg)
    {
      XERR << "[玩家角色-设置分支]" << accid << id << getProfession() << name << "get SRoleBaseCFG failed!"<< XEND;
      return;
    }

    m_pCurFighter->setBranch(cfg->dwTypeBranch);
    XLOG << "[玩家角色-设置分支]" << accid << id << getProfession() << name << "first set current branch for fighter! branch:" << cfg->dwTypeBranch << XEND;
  }
}

bool SceneUser::checkMapForChangeProfession()
{
  const SMapCFG* pMapCfg = MapConfig::getMe().getMapCFG(getMapID());
  if(!pMapCfg)
  {
    XERR << "[玩家角色-检测地图-切换职业]" << accid << id << getProfession() << name << "get SMapCFG failed! mapID:" << getMapID() << XEND;
    return false;
  }

  if(EMAPTYPE_MAIN != pMapCfg->eType && EMAPTYPE_GUILDRAID != pMapCfg->eType)
  {
    XERR << "[玩家角色-检测地图-切换职业]" << accid << id << getProfession() << name << "map type is false! mapType:" << pMapCfg->eType << XEND;
    return false;
  }

  return true;
}

void SceneUser::reqFixBranch()
{
  if(!thisServer)
    return;

  if(MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_MULTI_CAREER))
    return;

  // 检测acc补偿区分
  if(getMainCharId())
    return;

  ReqUserProfessionCmd cmd;
  cmd.set_scenename(thisServer->getServerName());
  cmd.set_charid(id);
  cmd.set_accid(accid);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);
}

bool SceneUser::isBuy()
{
  if (m_pCurFighter == nullptr || m_pCurFighter->getBranch() == 0)
    return false;

  Cmd::ProfessionData* pData = m_oProfession.getProfessionData(m_pCurFighter->getBranch());
  if (pData == nullptr)
    return false;
  return pData->role_data().isbuy();
}

// 补偿职业分支
bool SceneUser::fixBranch(std::vector<std::pair<DWORD, DWORD>>& vecProfessions)
{
  // 一转不满级或以下：没有补偿
  // 一转满级：补偿同系购买兑换券
  // 二转或以上同分支：补偿同系购买兑换券
  // 二转或以上不同分支：送对应分支职业与joblv
  if(2 < vecProfessions.size())
  {
    XERR << "[玩家角色-补偿职业]" << accid << id << getProfession() << name << "the number of professions out of two!" << XEND;
    return false;
  }

  // sort joblv(由大到小)
  std::sort(vecProfessions.begin(), vecProfessions.end(), [&](const std::pair<DWORD, DWORD>& p1, const std::pair<DWORD, DWORD>& p2) -> bool{return p1.second > p2.second;});

  auto addItem = [&](DWORD branch)
  {
    BasePackage* pMainPack = getPackage().getPackage(EPACKTYPE_MAIN);
    if(!pMainPack)
      return;

    const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(branch);
    if(!pBranchCfg)
      return;

    ItemInfo itemInfo;
    itemInfo.set_id(pBranchCfg->itemId);
    itemInfo.set_count(1);
    itemInfo.set_source(ESOURCE_PROFESSION_BUY);

    pMainPack->addItem(itemInfo,EPACKMETHOD_NOCHECK);
  };

  for(auto& p : vecProfessions)
  {
    if(50 > p.second)
      continue;
    else if(50 == p.second)
    {
      addItem(p.first);
    }
    else
    {
      const SRoleBaseCFG* pRoleCfg = RoleConfig::getMe().getRoleBase(static_cast<EProfession>(p.first));
      if(!pRoleCfg)
        continue;

      // 分支为0，奖励转职券
      if(!getBranch())
      {
        addItem(pRoleCfg->dwTypeBranch);
        continue;
      }

      // 分支不为0
      if(pRoleCfg->dwTypeBranch == getBranch())
      {
        addItem(pRoleCfg->dwTypeBranch);
      }
      else
      {
        if(m_oProfession.hasBranch(pRoleCfg->dwTypeBranch))
        {
          addItem(pRoleCfg->dwTypeBranch);
        }
        else
        {
          // 送职业
          m_oProfession.addBranch(p.first, p.second);
        }
      }
    }
  }

  return true;
}

void SceneUser::onAltmanEnd()
{
  if(m_oVar.getVarValue(EVARTYPE_ALTMAN_KILL) == 0)
    return;

  bool isOpen = ActivityManager::getMe().isOpen(static_cast<DWORD>(GACTIVITY_ALTMAN));
  if(isOpen == true)
    return;

  DWORD dwMailID = MiscConfig::getMe().getAltmanCFG().getKillReward(m_oVar.getVarValue(EVARTYPE_ALTMAN_KILL));
  m_oVar.setVarValue(EVARTYPE_ALTMAN_KILL, 0);
  if(dwMailID == 0)
    return;

  MailManager::getMe().sendMail(id, dwMailID);
  XLOG << "[玩家-奥特曼]" << accid << id << getProfession() << name << "邮件: " << dwMailID << XEND;
}

void SceneUser::altmanCheck()
{
  AltmanScene* pScene = dynamic_cast<AltmanScene*>(getScene());
  if (pScene == nullptr)
    return;

  const SAltmanCFG& rCFG = MiscConfig::getMe().getAltmanCFG();
  auto s = find_if(rCFG.setEnterBuffs.begin(), rCFG.setEnterBuffs.end(), [&](DWORD id) -> bool{
    return m_oBuff.haveBuff(id) == true;
  });
  if (s != rCFG.setEnterBuffs.end())
    return;
  gomap(MAP_PRONTERA, GoMapType::GM);
  XLOG << "[玩家-奥特曼]" << accid << id << getProfession() << name << "未处理变身状态,被传送至mapid :" << MAP_PRONTERA << XEND;
}

bool SceneUser::isEquipedAltmanFashion()
{
  bool bEquiped = false;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_oPackage.getPackage(EPACKTYPE_EQUIP));
  if(pEquipPack != nullptr)
  {
    ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_ARMOUR);
    if(pEquip != nullptr)
      bEquiped = MiscConfig::getMe().getAltmanCFG().isAltmanFashion(pEquip->getTypeID());
  }

  if(bEquiped == false)
  {
    FashionEquipPackage* pFashionPack = dynamic_cast<FashionEquipPackage*>(m_oPackage.getPackage(EPACKTYPE_FASHIONEQUIP));
    if(pFashionPack != nullptr)
    {
      ItemEquip* pFashion = pFashionPack->getEquip(EEQUIPPOS_ARMOUR);
      if(pFashion != nullptr)
        return MiscConfig::getMe().getAltmanCFG().isAltmanFashion(pFashion->getTypeID());
    }
    return false;
  }
  else
    return true;
}

void SceneUser::onAltmanFashionEquip()
{
  m_bEquipedAltmanFashion = isEquipedAltmanFashion();
  if(m_bEquipedAltmanFashion == false && m_oBuff.haveBuff(MiscConfig::getMe().getAltmanCFG().dwFashionBuff) == true)
    m_oBuff.del(MiscConfig::getMe().getAltmanCFG().dwFashionBuff);
}

std::set<SceneUser*> SceneUser::getTeamSceneUserInPvpGvg()
{
  std::set<SceneUser*> res;
  if (getTeamID() == 0)
    return res;
  Scene* pScene = getScene();
  if (!pScene)
    return res;

  for (auto &m : m_oGTeam.getTeamMemberList())
  {
    const TeamMemberInfo& rMember = m.second;
    SceneUser* pUser = nullptr;
    if (rMember.charid() == id)
      continue;
    else
    {
      pUser = SceneUserManager::getMe().getUserByID(rMember.charid());
      if (pUser == nullptr || pUser->getScene() == nullptr)
      continue;
      if(pScene->isPVPScene() == true && pUser->getScene()->isPVPScene() == true)
        res.insert(pUser);
      else if(pScene->isGvg() == true && pUser->getScene()->isGvg() == true)
        res.insert(pUser);
    }
  }

  return res;
}

void SceneUser::leaveDressSatgeOnUnreg()
{
  if(m_oDressUp.getDressUpStatus() != EDRESSUP_MIN)
  {
    xPos movePos = MiscConfig::getMe().getDressStageCFG().m_pQuitPos;
    goTo(movePos);
  }
}

void SceneUser::breakAllFollowers()
{
  for (auto &u : getTeamSceneUser())
  {
    if(u->getUserSceneData().getFollowerID() == id)
      u->getUserSceneData().setFollowerID(0);
  }
}

ExchangeShop& SceneUser::getExchangeShop() { return *m_pExchangeShop; }

void SceneUser::checkWorldLevelBuff()
{
  auto checkFun = [this](DWORD dwWorldLevel, DWORD dwCurLevel, DWORD dwBuffID)
  {
    if (dwBuffID == 0)
      return;
    if (dwWorldLevel == 0)
      return;
    if (dwWorldLevel > dwCurLevel)
    {
      m_oBuff.add(dwBuffID);
    }
    else
    {
      m_oBuff.del(dwBuffID);
    }
  };

  checkFun(SceneUserManager::getMe().getBaseWorldLevel(), getLevel(), MiscConfig::getMe().getExchangeShopCFG().dwBaseWorldLevelExpBuff);
  checkFun(SceneUserManager::getMe().getJobWorldLevel(), getJobLv(), MiscConfig::getMe().getExchangeShopCFG().dwJobWorldLevelExpBuff);
}

xSceneEntryDynamic* SceneUser::getEnsemblePartner()
{
  if (m_oSkillStatus.inStatus(ESKILLSTATUS_ENSEMBLE) == false)
    return nullptr;
  const TSetQWORD& others = m_oSkillStatus.getOtherCharID();
  if (others.empty())
    return nullptr;
  return xSceneEntryDynamic::getEntryByID(*(others.begin()));
}

void SceneUser::setEnsembleSkill(const string& str)
{
  if (str == m_strEnsembleSkill)
    return;
  m_strEnsembleSkill = str;
  setDataMark(EUSERDATATYPE_ENSEMBLESKILL);
}

void SceneUser::updateEnsembleSkill()
{
  const SRoleBaseCFG* cfg = RoleConfig::getMe().getRoleBase(getProfession());
  if (cfg == nullptr)
    return;

  const TSetDWORD& ensemble = MiscConfig::getMe().getSkillCFG().getEnsembleSkill(cfg->dwTypeBranch);
  if (ensemble.empty())
  {
    setEnsembleSkill(STRING_EMPTY);
    return;
  }

  stringstream ss;
  bool f = false;

  for (auto& familyid : ensemble)
  {
    for (auto& fighter : m_vecFighters)
    {
      if (fighter && fighter->getSkill().isSkillFamilyEnable(familyid))
      {
        if (f)
          ss << ",";
        f = true;
        ss << familyid;
        break;
      }
    }
  }

  setEnsembleSkill(ss.str());
}

void SceneUser::toModelShowData(QueryUserInfo* pInfo)
{
  if (pInfo == nullptr)
    return;
  pInfo->set_charid(id);
  pInfo->set_name(name);
  pInfo->set_guildid(m_oGuild.id());
  pInfo->set_guildname(m_oGuild.name());

  add_data(pInfo->add_datas(), EUSERDATATYPE_SEX, m_oUserSceneData.getGender());
  add_data(pInfo->add_datas(), EUSERDATATYPE_PROFESSION, getProfession());
  add_data(pInfo->add_datas(), EUSERDATATYPE_ROLELEVEL, getLevel());
  add_data(pInfo->add_datas(), EUSERDATATYPE_HAIR, m_oHair.getCurHair());
  add_data(pInfo->add_datas(), EUSERDATATYPE_HAIRCOLOR, m_oHair.getCurHairColor());
  add_data(pInfo->add_datas(), EUSERDATATYPE_BODY, m_oUserSceneData.getBody());
  add_data(pInfo->add_datas(), EUSERDATATYPE_EYE, m_oEye.getCurID());
  add_data(pInfo->add_datas(), EUSERDATATYPE_CLOTHCOLOR, m_oUserSceneData.getClothColor());
  add_data(pInfo->add_datas(), EUSERDATATYPE_HEAD, m_oUserSceneData.getHead());
  add_data(pInfo->add_datas(), EUSERDATATYPE_BACK, m_oUserSceneData.getBack());
  add_data(pInfo->add_datas(), EUSERDATATYPE_FACE, m_oUserSceneData.getFace());
  add_data(pInfo->add_datas(), EUSERDATATYPE_TAIL, m_oUserSceneData.getTail());
  add_data(pInfo->add_datas(), EUSERDATATYPE_MOUNT, m_oUserSceneData.getMount());
  add_data(pInfo->add_datas(), EUSERDATATYPE_MOUTH, m_oUserSceneData.getMouth());
  add_data(pInfo->add_datas(), EUSERDATATYPE_LEFTHAND, m_oUserSceneData.getLefthand());
  add_data(pInfo->add_datas(), EUSERDATATYPE_RIGHTHAND, m_oUserSceneData.getRighthand());
}

void SceneUser::toPortraitData(UserPortraitData* pData)
{
  if (pData == nullptr)
    return;
  pData->set_portrait(m_oPortrait.getCurPortrait());
  pData->set_body(m_oUserSceneData.getBody());
  pData->set_hair(m_oHair.getCurHair());
  pData->set_haircolor(m_oHair.getCurHairColor());
  pData->set_gender(m_oUserSceneData.getGender());
  pData->set_head(m_oUserSceneData.getHead());
  pData->set_face(m_oUserSceneData.getFace());
  pData->set_mouth(m_oUserSceneData.getMouth());
  pData->set_eye(m_oEye.getCurID());
}

bool SceneUser::canTeamUseWingOfFly()
{
  for(SceneUser* pTeamMember:getTeamSceneUser())
  {
    if(!pTeamMember->canUseWingOfFly())
      return false;
  }
  return true;
}

void SceneUser::sendVarToSession(EVarType eType)
{
  SyncUserVarSessionCmd cmd;
  cmd.set_charid(id);
  auto v = cmd.add_vars();
  if (v)
  {
    v->set_type(eType);
    v->set_value(m_oVar.getVarValue(eType));
  }
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}
