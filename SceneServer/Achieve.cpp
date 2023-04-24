#include "Achieve.h"
#include "AchieveProcess.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "GuildConfig.h"
#include "Menu.h"

bool SAchieveItem::fromData(const AchieveItem& rData)
{
  dwID = rData.id();
  dwProcess = rData.process();
  dwFinishTime = rData.finishtime();
  bRewardGet = rData.reward_get();
  pCFG = AchieveConfig::getMe().getAchieveCFG(dwID);
  if (dwFinishTime != 0 && pCFG != nullptr)
  {
    dwProcess = pCFG->getProcess();
  }
  else
  {
    for (DWORD d = 0; d < MAX_PARAM; ++d)
    {
      if (d < static_cast<DWORD>(rData.params_size()))
        arrParams[d] = rData.params(d);
    }
  }
  return true;
}

bool SAchieveItem::toData(AchieveItem* pData, SceneUser* pUser)
{
  if (pData == nullptr)
    return false;

  pData->Clear();
  pData->set_id(dwID);
  pData->set_process(dwProcess);
  pData->set_finishtime(dwFinishTime);
  pData->set_reward_get(bRewardGet);
  if (pCFG != nullptr)
  {
    if (pData->finishtime() == 0 && (pCFG->stCondition.eCond == EACHIEVECOND_KILL_MVP || pCFG->stCondition.eCond == EACHIEVECOND_CAT))
    {
      for (DWORD i = 0; i < MAX_PARAM; ++i)
        pData->add_params(arrParams[i]);
    }
    if (pCFG->stCondition.eCond == EACHIEVECOND_QUEST_SUBMIT)
    {
      for (auto &m : pCFG->mapPreQuest)
        pData->add_quests()->CopyFrom(m.second);
    }
  }
  return true;
}

bool SAchieveItem::fromData(const AchieveDBItem& rItem)
{
  dwID = rItem.id();
  dwProcess = rItem.process();
  dwFinishTime = rItem.finishtime();
  bRewardGet = rItem.reward_get();
  pCFG = AchieveConfig::getMe().getAchieveCFG(dwID);
  if (dwFinishTime != 0 && pCFG != nullptr)
  {
    dwProcess = pCFG->getProcess();
  }
  else
  {
    for (DWORD d = 0; d < MAX_PARAM; ++d)
    {
      if (d < static_cast<DWORD>(rItem.params_size()))
        arrParams[d] = rItem.params(d);
    }
  }
  return true;
}

bool SAchieveItem::toData(AchieveDBItem* pItem)
{
  if (pItem == nullptr)
    return false;

  pItem->Clear();
  pItem->set_id(dwID);
  pItem->set_process(dwProcess);
  pItem->set_finishtime(dwFinishTime);
  pItem->set_reward_get(bRewardGet);
  if (pCFG != nullptr)
  {
    if (pItem->finishtime() == 0 && (pCFG->stCondition.eCond == EACHIEVECOND_KILL_MVP || pCFG->stCondition.eCond == EACHIEVECOND_CAT))
    {
      for (DWORD i = 0; i < MAX_PARAM; ++i)
        pItem->add_params(arrParams[i]);
    }
  }
  return true;
}

// Achieve
Achieve::Achieve(SceneUser* pUser) : m_pUser(pUser)
{
}

Achieve::~Achieve()
{

}

bool Achieve::load(const BlobAchieve& rAccAchieve, const BlobAchieve& rAchieve)
{
  m_dwDataVersion = rAccAchieve.version();
  for (int i = 0; i < rAccAchieve.items_size(); ++i)
  {
    const AchieveDBItem& item = rAccAchieve.items(i);
    SAchieveItem& rItem = m_mapItem[item.id()];
    rItem.fromData(item);
  }

  m_mapMaxCache.clear();
  for (int i = 0; i < rAchieve.max_cache_size(); ++i)
  {
    const MaxInfo& rInfo = rAchieve.max_cache(i);
    m_mapMaxCache[static_cast<EShareDataType>(rInfo.type())].push_back(rInfo);
  }
  m_dwCharVersion = rAchieve.char_version();

  version_update();
  return true;
}

bool Achieve::save(BlobAchieve* pAccAchieve, BlobAchieve* pAchieve)
{
  if (pAccAchieve == nullptr || pAchieve == nullptr)
    return false;

  pAccAchieve->Clear();
  pAchieve->Clear();

  for (auto &m : m_mapItem)
    m.second.toData(pAccAchieve->add_items());

  for (auto &m : m_mapMaxCache)
  {
    for (auto &v : m.second)
      pAchieve->add_max_cache()->CopyFrom(v);
  }

  pAccAchieve->set_version(m_dwDataVersion);
  pAchieve->set_char_version(m_dwCharVersion);
  XDBG << "[成就-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 acc :" << pAccAchieve->ByteSize() << "char :" << pAchieve->ByteSize() << XEND;
  return true;
}

bool Achieve::reload()
{
  for (auto &m : m_mapItem)
    m.second.pCFG = AchieveConfig::getMe().getAchieveCFG(m.first);
  XLOG << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "配置重加载" << XEND;
  return true;
}

void Achieve::queryUserResume()
{
  QueryUserResumeAchCmd cmd;

  UserResume* pResume = cmd.mutable_data();
  pResume->set_createtime(m_pUser->getUserSceneData().getCreateTime());
  pResume->set_logintime(m_pUser->getShare().getNormalData(ESHAREDATATYPE_S_LOGINCOUNT));

  const SRoleBaseCFG* pCFG = m_pUser->getRoleBaseCFG();
  if (pCFG != nullptr)
  {
    DWORD dwBranch = pCFG->dwTypeBranch;
    Profession& rProfession = m_pUser->m_oProfession;
    Share& rShare = m_pUser->getShare();
    DWORD dwTime = 0;
    if (m_pUser->getEvo() >= 1)
    {
      if (rProfession.getExchangeTime(dwBranch, 1, dwTime) == false)
      {
        dwTime = rShare.getNormalData(ESHAREDATATYPE_S_BE_PRO_1_TIME);
        rProfession.setExchangeTime(dwBranch, 1, dwTime);
      }
      else
      {
        pResume->set_bepro_1_time(dwTime);
      }
    }
    if (m_pUser->getEvo() >= 2)
    {
      if (rProfession.getExchangeTime(dwBranch, 2, dwTime) == false)
      {
        dwTime = rShare.getNormalData(ESHAREDATATYPE_S_BE_PRO_2_TIME);
        rProfession.setExchangeTime(dwBranch, 2, dwTime);
      }
      else
      {
        pResume->set_bepro_2_time(dwTime);
      }
    }
    if (m_pUser->getEvo() >= 3)
    {
      if (rProfession.getExchangeTime(dwBranch, 3, dwTime) == false)
      {
        dwTime = rShare.getNormalData(ESHAREDATATYPE_S_BE_PRO_3_TIME);
        rProfession.setExchangeTime(dwBranch, 3, dwTime);
      }
      else
      {
        pResume->set_bepro_3_time(dwTime);
      }
    }
  }
  else
  {
    XERR << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置转职时间失败" << XEND;
  }
  /*pResume->set_bepro_1_time(m_pUser->getShare().getNormalData(ESHAREDATATYPE_S_BE_PRO_1_TIME));
  pResume->set_bepro_2_time(m_pUser->getShare().getNormalData(ESHAREDATATYPE_S_BE_PRO_2_TIME));
  pResume->set_bepro_3_time(m_pUser->getShare().getNormalData(ESHAREDATATYPE_S_BE_PRO_3_TIME));*/
  pResume->set_walk_distance(m_pUser->getShare().getNormalData(ESHAREDATATYPE_S_MOVEDIS));
  //pResume->set_max_team(getMostUserName(ESHAREDATATYPE_MOST_TEAMTIME));
  //pResume->set_max_hand(getMostUserName(ESHAREDATATYPE_MOST_HANDTIME));
  //pResume->set_max_wheel(getMostUserName(ESHAREDATATYPE_MOST_WHELL));
  //pResume->set_max_chat(getMostUserName(ESHAREDATATYPE_MOST_CHAT));

  collectMostUserName(ESHAREDATATYPE_MOST_TEAMTIME, 4, cmd);
  collectMostUserName(ESHAREDATATYPE_MOST_HANDTIME, 1, cmd);
  collectMostUserName(ESHAREDATATYPE_MOST_WHELL, 1, cmd);
  collectMostUserName(ESHAREDATATYPE_MOST_CHAT, 1, cmd);
  collectMostUserName(ESHAREDATATYPE_MOST_MUSICCD, 1, cmd);
  collectMostUserName(ESHAREDATATYPE_MOST_SAVE, 1, cmd);
  collectMostUserName(ESHAREDATATYPE_MOST_BESAVED, 1, cmd);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "请求玩家履历" << cmd.ShortDebugString() << XEND;
}

void Achieve::queryAchieveData(EAchieveType eType)
{
  DWORD dwSendCount = 0;
  QueryAchieveDataAchCmd cmd;
  cmd.set_type(eType);
  for (auto &m : m_mapItem)
  {
    if (m.second.pCFG == nullptr || m.second.pCFG->eType != eType)
      continue;
    m.second.toData(cmd.add_items(), m_pUser);
    if (cmd.ByteSize() > TRANS_BUFSIZE)
    {
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      cmd.clear_items();
      ++dwSendCount;
    }
  }
  if (cmd.items_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
    ++dwSendCount;
  }

  //XDBG << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "大小" << cmd.ByteSize() << "次数" << dwSendCount << "数据" << cmd.ShortDebugString() << XEND;
}

void Achieve::sendAchieveData()
{
  static const vector<EAchieveType> vecTypes = vector<EAchieveType>{
      EACHIEVETYPE_USER, EACHIEVETYPE_SOCIAL, EACHIEVETYPE_ADVENTURE, EACHIEVETYPE_BATTLE,
      EACHIEVETYPE_DRAMA, EACHIEVETYPE_ACTIVITY, EACHIEVETYPE_OTHER};
  for (auto &v : vecTypes)
    queryAchieveData(v);
}

SAchieveItem* Achieve::getAchieveItem(DWORD dwID)
{
  auto m = m_mapItem.find(dwID);
  if (m != m_mapItem.end())
    return &m->second;
  return nullptr;
}

bool Achieve::getReward(DWORD dwID)
{
  auto m = m_mapItem.find(dwID);
  if (m == m_mapItem.end())
  {
    XERR << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取成就 :" << dwID << "奖励失败,该成就不存在" << XEND;
    return false;
  }
  if (m->second.dwFinishTime == 0)
  {
    XERR << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取成就 :" << dwID << "奖励失败,该成就未完成" << XEND;
    return false;
  }
  if (m->second.bRewardGet)
  {
    XERR << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取成就 :" << dwID << "奖励失败,已领取" << XEND;
    return false;
  }
  if (m->second.pCFG == nullptr)
  {
    XERR << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取成就 :" << dwID << "奖励失败,未在 Table_Achievement.txt 表中找到" << XEND;
    return false;
  }

  if (m->second.pCFG->vecReward.empty() == false && m_pUser->getPackage().addItem(m->second.pCFG->vecReward, EPACKMETHOD_AVAILABLE) == false)
  {
    XERR << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取成就 :" << dwID << "奖励失败,添加奖励失败" << XEND;
    return false;
  }

 /* if (m->second.pCFG->stRewardFunction.eRewardFunction != EACHIEVEREWARD_MIN)
  {
    const SAchieveRewardFunction& rCFG = m->second.pCFG->stRewardFunction;;
    switch(rCFG.eRewardFunction)
    {
      case EACHIEVEREWARD_MIN:
        break;
      case EACHIEVEREWARD_ADDSHOPCNT:
        m_pUser->getSceneShop().addAccBuyLimitCnt(rCFG.dwParam1, rCFG.dwParam2);
        break;
      default:
        break;
    }
  }
  */

  m->second.bRewardGet = true;
  m_pUser->getManual().addPoint(m->second.pCFG->dwManualExp, EMANUALTYPE_MIN, 0);
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_MANUALUNLOCK);

  NewAchieveNtfAchCmd cmd;
  cmd.set_type(m->second.pCFG->eType);
  m->second.toData(cmd.add_items(), m_pUser);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取成就 :" << dwID << "奖励成功" << XEND;
  return true;
}

bool Achieve::isFinishAchieve(DWORD dwID)
{
  auto m = m_mapItem.find(dwID);
  if (m == m_mapItem.end())
    return false;

  if (m->second.dwFinishTime == 0)
    return false;

  return true;
}

DWORD Achieve::getUnlockedAchieveNum() const
{
  DWORD dwNum = 0;
  for(auto &it : m_mapItem)
  {
    if (it.second.dwFinishTime > 0 && it.second.bRewardGet)
      ++dwNum;
  }
  return dwNum;
}

void Achieve::onLevelup()
{
  processOnce(EACHIEVECOND_LEVELUP);
}

void Achieve::onAttrChange()
{
  processOnce(EACHIEVECOND_USER_ATTR);
}

void Achieve::onAddFriend()
{
  processTime(EACHIEVECOND_ADDFRIEND);
}

void Achieve::onDemandMusic()
{
  processTime(EACHIEVECOND_PLAYMUSIC);
  onTravel();
}

void Achieve::onFerrisWheel()
{
  processTime(EACHIEVECOND_FERRISWHEEL);
  onTravel();
}

void Achieve::onHandTime(DWORD dwTime)
{
  onTravel();
  if (dwTime == 0)
    return;
  processTime(EACHIEVECOND_HAND, TVecQWORD{}, dwTime);
}

void Achieve::onExpressAdd()
{
  processTime(EACHIEVECOND_EMOJI);
}

void Achieve::onExpressUse()
{
  processTime(EACHIEVECOND_EXPRESSION);
}

void Achieve::onEnterGuild(bool bCreate)
{
  if (bCreate)
    processOnce(EACHIEVECOND_CREATEGUILD);
  else
    processOnce(EACHIEVECOND_ENTERGUILD);
}

void Achieve::onPhoto(EAchieveCond eCond, const TSetQWORD& setIDs /*= TSetQWORD{}*/)
{
  if (eCond == EACHIEVECOND_PHOTO || eCond == EACHIEVECOND_GHOST_PHOTO)
    processTime(eCond);
  else if (eCond == EACHIEVECOND_PHOTO_MAN || eCond == EACHIEVECOND_PHOTO_MONSTER || eCond == EACHIEVECOND_PHOTO_USER)
  {
    TVecQWORD vecIDs;
    vecIDs.insert(vecIDs.end(), setIDs.begin(), setIDs.end());
    processTime(eCond, vecIDs);
  }
}

void Achieve::onChat(EGameChatChannel eChannel)
{
  if (eChannel == ECHAT_CHANNEL_FRIEND)
    processTime(EACHIEVECOND_USER_CHAT, TVecQWORD{eChannel, 0});
  else if (eChannel == ECHAT_CHANNEL_WORLD)
  {
    processTime(EACHIEVECOND_USER_CHAT, TVecQWORD{eChannel, m_pUser->getVar().getVarValue(EVARTYPE_ACHIEVE_CHAT_WORLD)});
    if (m_pUser->getVar().getVarValue(EVARTYPE_ACHIEVE_CHAT_WORLD) == 0)
      m_pUser->getVar().setVarValue(EVARTYPE_ACHIEVE_CHAT_WORLD, 1);
  }
  else if (eChannel == ECHAT_CHANNEL_GUILD)
  {
    processTime(EACHIEVECOND_USER_CHAT, TVecQWORD{eChannel, m_pUser->getVar().getVarValue(EVARTYPE_ACHIEVE_CHAT_GUILD)});
    if (m_pUser->getVar().getVarValue(EVARTYPE_ACHIEVE_CHAT_GUILD) == 0)
      m_pUser->getVar().setVarValue(EVARTYPE_ACHIEVE_CHAT_GUILD, 1);
  }
}

void Achieve::onBody()
{
  processOnce(EACHIEVECOND_USER_BODY);
}

void Achieve::onRune()
{
  processTime(EACHIEVECOND_USER_RUNE);
}

void Achieve::onEnterMap()
{
  DWORD dwMapID = m_pUser->getScene() == nullptr ? m_pUser->getUserSceneData().getOnlineMapID() : m_pUser->getScene()->getMapID();
  if (dwMapID == 0)
    return;
  processOnce(EACHIEVECOND_MAPMOVE, TVecQWORD{dwMapID});
}

void Achieve::onPortrait()
{
  processTime(EACHIEVECOND_USER_PORTRAIT);
}

void Achieve::onHair(bool bChange)
{
  processTime(EACHIEVECOND_USER_HAIR, TVecQWORD{bChange ? 1UL : 0UL});
}

void Achieve::onTitle(DWORD dwTitleID)
{
  if (m_pUser->getTitle().hasTitle(dwTitleID) == false)
    return;
  processOnce(EACHIEVECOND_USER_TITLE, TVecQWORD{dwTitleID});
}

void Achieve::onCat(bool online)
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_ACHIEVE_CAT) != 0)
    return;
  m_pUser->getVar().setVarValue(EVARTYPE_ACHIEVE_CAT, 1);
  processTime(EACHIEVECOND_CAT, TVecQWORD{online ? 1UL : 2UL});
}

void Achieve::onUseSkill(DWORD dwSkillID)
{
  processTime(EACHIEVECOND_USE_SKILL, TVecQWORD{dwSkillID});
}

void Achieve::onBattleTime(DWORD dwTime)
{
  if (dwTime == 0)
    return;

  m_dwBattleTimeTmp += dwTime;
  if (m_dwBattleTimeTmp / MIN_T == 0)
    return;
  processTime(EACHIEVECOND_BATTLE_TIME, TVecQWORD{}, m_dwBattleTimeTmp / MIN_T);
  m_dwBattleTimeTmp = 0;
}

void Achieve::onPvp(bool bWin)
{
  processTime(EACHIEVECOND_PVP, TVecQWORD{bWin ? 2UL : 1UL});
}

void Achieve::onDead(bool bFind, DWORD dwLostExp)
{
  processTime(EACHIEVECOND_USER_DEAD, TVecQWORD{bFind ? 1UL : 0UL, dwLostExp});
}

void Achieve::onTrans()
{
  processTime(EACHIEVECOND_USER_TRANSFER);
}

void Achieve::onDamage(DWORD dwNpcID, DWORD dwDamage)
{
  processTime(EACHIEVECOND_USER_DAMAGE, TVecQWORD{dwNpcID}, dwDamage);
}

void Achieve::onHelpQuest()
{
  processTime(EACHIEVECOND_HELP_QUEST);
}

void Achieve::onEquip(EAchieveCond eCond)
{
  processTime(eCond);
}

void Achieve::onProduceEquip(DWORD dwItemID)
{
  processTime(EACHIEVECOND_PRODUCE_EQUIP, TVecQWORD{dwItemID});
}

void Achieve::onRefine(bool bSuccess)
{
  processTime(EACHIEVECOND_REFINE_FAIL, TVecQWORD{bSuccess ? 1UL : 0UL});
}

void Achieve::onItemAdd(const ItemInfo& rInfo)
{
  if (rInfo.source() == ESOURCE_STORE || rInfo.source() == ESOURCE_PERSON_OFFSTORE)
    return;

  processTime(EACHIEVECOND_ITEM_GET, TVecQWORD{rInfo.id()}, rInfo.count(), false, rInfo.source());
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rInfo.id());
  if (pCFG == nullptr || pCFG->eItemType < EITEMTYPE_CARD_WEAPON || pCFG->eItemType > EITEMTYPE_CARD_HEAD || (rInfo.source() != ESOURCE_TRADE && rInfo.source() != ESOURCE_PICKUP))
    return;
  processTime(EACHIEVECOND_GET_ITEM);
}

void Achieve::onRepairSeal()
{
  processTime(EACHIEVECOND_REPAIR_SEAL);
}

void Achieve::onPassTower()
{
  processOnce(EACHIEVECOND_TOWER_PASS, TVecQWORD{}, m_pUser->getTower().getCurMaxLayer());
}

void Achieve::onTransWithKPL()
{
  processTime(EACHIEVECOND_KPL_TRANS);
}

void Achieve::onPassDojo(bool bHelp)
{
  processTime(EACHIEVECOND_DOJO, TVecQWORD{static_cast<QWORD>(bHelp ? 1 : 0)});
}

void Achieve::onCarrier(DWORD dwBusID)
{
  processTime(EACHIEVECOND_VEHICLE);
  onTravel();
  onWedding(EACHIEVECOND_WEDDING_CARRIER, TVecQWORD{dwBusID});
}

void Achieve::onShopBuy(QWORD qwZeny)
{
  processTime(EACHIEVECOND_MONEY_SHOP_BUY, TVecQWORD{}, qwZeny);
}

void Achieve::onShopSell(QWORD qwZeny)
{
  processTime(EACHIEVECOND_MONEY_SHOP_SELL, TVecQWORD{}, qwZeny);
}

void Achieve::onTradeBuy(QWORD qwZeny)
{
  processTime(EACHIEVECOND_MONEY_TRADE_BUY, TVecQWORD{}, qwZeny);
  processOnce(EACHIEVECOND_MONEY_TRADE_ONCEBUY, TVecQWORD{}, qwZeny);
}

void Achieve::onTradeSell(QWORD qwZeny)
{
  processTime(EACHIEVECOND_MONEY_TRADE_SELL, TVecQWORD{}, qwZeny);
  processOnce(EACHIEVECOND_MONEY_TRADE_ONCESELL, TVecQWORD{}, qwZeny);
}

void Achieve::onAddMoney(QWORD qwZeny)
{
  processTime(EACHIEVECOND_MONEY_GET, TVecQWORD{}, qwZeny);
}

void Achieve::onCharge()
{
  processTime(EACHIEVECOND_MONEY_CHARGE);
}

void Achieve::onKplConsume(QWORD qwZeny)
{
  processTime(EACHIEVECOND_KPL_CONSUME, TVecQWORD{}, qwZeny);
}

void Achieve::onTradeRecord()
{
  processTime(EACHIEVECOND_TRADE_RECORD);
}

void Achieve::onKillMonster(DWORD dwType, DWORD dwID)
{
  processTime(EACHIEVECOND_KILL_MONSTER, TVecQWORD{dwType, dwID});
  processTime(EACHIEVECOND_BATTLE, TVecQWORD{dwType, dwID});
  processTime(EACHIEVECOND_CAT, TVecQWORD{dwID});
}

void Achieve::onMvp(EMvpScoreType eType, DWORD dwID)
{
  processTime(EACHIEVECOND_KILL_MVP, TVecQWORD{eType, dwID});
}

void Achieve::onItemUsed(DWORD dwItemID)
{
  processTime(EACHIEVECOND_USEITEM, TVecQWORD{dwItemID});
}

void Achieve::onItemBeUsed(DWORD dwItemID)
{
  processTime(EACHIEVECOND_ITEM, TVecQWORD{dwItemID});
}

void Achieve::onManual(EManualType eType, EAchieveCond eCond)
{
  if (eCond == EACHIEVECOND_MONSTER_PHOTO)
    processTime(eCond);
  else if (eCond == EACHIEVECOND_NPC_COUNT)
    processTime(eCond, TVecQWORD{eType});
  else if (eCond == EACHIEVECOND_SCENERY_COUNT)
  {
    processTime(eCond, TVecQWORD{eType});
    processOnce(EACHIEVECOND_SCENERY);
  }
  processTime(EACHIEVECOND_MANUAL);
}

void Achieve::onCollection()
{
  processTime(EACHIEVECOND_COLLECTION);
}

void Achieve::onQuestSubmit(DWORD dwQuestID)
{
  if (m_pUser->getQuest().isSubmit(dwQuestID) == false)
    return;
  const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(dwQuestID);
  if (pCFG == nullptr)
    return;
  if (pCFG->eType == EQUESTTYPE_WANTED)
    processTime(EACHIEVECOND_WANTEDQUEST);
  else
    processTime(EACHIEVECOND_QUEST_SUBMIT, TVecQWORD{dwQuestID});
  processTime(EACHIEVECOND_QUEST);
}

void Achieve::onAchieveFinish(const TVecQWORD& vecFinishAchIDs /*= TVecQWORD{}*/)
{
  if (vecFinishAchIDs.empty() == true)
    return;
  processTime(EACHIEVECOND_ACHIEVE_FINISH, vecFinishAchIDs);
}

void Achieve::onMonsterDraw(DWORD curSec)
{
  if (m_dwTimeTick > curSec)
    return;
  m_dwTimeTick = curSec + 5;
  processTime(EACHIEVECOND_MONSTER_DRAW);
}

void Achieve::onSeat()
{
  processTime(EACHIEVECOND_SEAT);
}

void Achieve::onTravel()
{
  processOnce(EACHIEVECOND_TRAVEL);
}

void Achieve::onManualOpen()
{
  if (m_pUser == nullptr || m_pUser->getMenu().isOpen(EMENUID_MANUAL) == false)
    return;

  map<EAchieveType, NewAchieveNtfAchCmd> mapUpdates;
  for (auto &m : m_mapItem)
  {
    if (m.second.dwFinishTime == 0 || m.second.pCFG == nullptr)
      continue;
    AchieveItem* item = mapUpdates[m.second.pCFG->eType].add_items();
    m.second.toData(item, m_pUser);
  }

  for (auto &m : mapUpdates)
  {
    m.second.set_type(m.first);
    PROTOBUF(m.second, send, len);
    m_pUser->sendCmdToMe(send, len);
    XLOG << "[成就-冒险手册解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "冒险手册解锁,同步了完成成就" << m.second.ShortDebugString() << XEND;
  }
}

bool Achieve::finish(DWORD dwID)
{
  TVecQWORD vecFinishIDs;
  DWORD dwNow = xTime::getCurSec();

  const SAchieveCFG* pCfg = AchieveConfig::getMe().getAchieveCFG(dwID);
  if(nullptr == pCfg) return false;

  SAchieveItem* pItem = getAchieveItem(dwID);
  if(nullptr == pItem)
  {
    pItem = &m_mapItem[dwID];
    pItem->pCFG = pCfg;
    pItem->dwID = dwID;
  }

  if(0 != pItem->dwFinishTime) return false;

  pItem->dwFinishTime = dwNow;
  pItem->dwProcess = pCfg->getProcess();

  vecFinishIDs.push_back(dwID);

  TVecItemInfo vecItems = pCfg->vecReward;
  m_pUser->getPackage().addItem(vecItems, EPACKMETHOD_AVAILABLE);
  m_pUser->getManual().addPoint(pCfg->dwManualExp, EMANUALTYPE_MIN, 0);


  NewAchieveNtfAchCmd cmd;
  AchieveItem* item = cmd.add_items();
  pItem->toData(item, m_pUser);

  cmd.set_type(pCfg->eType);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "gm达成成就" << item->ShortDebugString() << XEND;

  if (vecFinishIDs.empty() == false)
  {
    onAchieveFinish(vecFinishIDs);
    m_pUser->getMenu().refreshNewMenu(EMENUCOND_ACHIEVE);
  }

  return true;
}

// pet
void Achieve::onPetCapture(bool bSuccess) { processTime(bSuccess ? EACHIEVECOND_PET_CAPTURE_SUCCESS : EACHIEVECOND_PET_CAPTURE_FAIL); }
void Achieve::onPetBaseLvUp() { processOnce(EACHIEVECOND_PET_BASELV); }
void Achieve::onPetFeed() { processTime(EACHIEVECOND_PET_FEED); }
void Achieve::onPetTouch() { processTime(EACHIEVECOND_PET_TOUCH); }
void Achieve::onPetGift() { processTime(EACHIEVECOND_PET_GIFT); }
void Achieve::onPetHand(DWORD dwDistance) { if (m_pUser->getUserPet().handPet() == true and dwDistance >= 1) processTime(EACHIEVECOND_PET_HANDWALK); }
void Achieve::onPetTime(DWORD dwTime) { if (dwTime >= MIN_T) processTime(EACHIEVECOND_PET_TIME); }
//void onPetEquip();
//void Achieve::onPetFriendLvUp() { processOnce(EACHIEVECOND_PET_FRIENDLV); }
void Achieve::onPetDead() { processTime(EACHIEVECOND_PET_DEAD); }
void Achieve::onPetAdventure() { processOnce(EACHIEVECOND_PET_ADVENTURE_FINISH); processTime(EACHIEVECOND_PET_ADVENTURE_COUNT); }
void Achieve::onPetEquip() { processOnce(EACHIEVECOND_PET_EQUIP); }

void Achieve::onCookFood(DWORD dwTimes, bool bSuccess)
{
  processTime(EACHIEVECOND_COOKFOOD, TVecQWORD{ static_cast<QWORD>(bSuccess ? 1 : 0) }, dwTimes);
}

void Achieve::onEatFood(DWORD dwItemId, DWORD dwItemNum, bool notMine)
{
  processTime(EACHIEVECOND_EATFOOD, TVecQWORD{ static_cast<QWORD>(notMine ? 1 : 0) }, dwItemNum);
  processTime(EACHIEVECOND_EATFOODID, TVecQWORD{ dwItemId }, dwItemNum);
}

void Achieve::onFoodMaterialLvUp(DWORD dwItemId, DWORD dwNewLv)
{
  if (dwNewLv < 10)
    return;
  processOnce(EACHIEVECOND_FOODMATERIALLV);
}

void Achieve::onCookerLvUp(DWORD dwNewLv)
{
  processOnce(EACHIEVECOND_COOKERLV);
}
void Achieve::onTasterLvUp(DWORD dwNewLv)
{
  processOnce(EACHIEVECOND_TASTERLV);
}
void Achieve::onUseSaveHp(DWORD dwValue)
{
  processTime(EACHIEVECOND_COSTSAVEHP, TVecQWORD{dwValue});
}

void Achieve::onUseSaveSp(DWORD dwValue)
{
  processTime(EACHIEVECOND_COSTSAVESP, TVecQWORD{ dwValue });
}

void Achieve::onFoodCookLvUp(DWORD dwItemId, DWORD dwNewLv)
{
  processOnce(EACHIEVECOND_FOODCOOKLV, TVecQWORD{ dwItemId, dwNewLv });
}

void Achieve::onTutorGuide()
{
  processTime(EACHIEVECOND_TUTOR_GUIDE);
}

void Achieve::onStudentGraduation()
{
  processTime(EACHIEVECOND_TUTOR_STUDENT_GRADUATION);
}

void Achieve::onGoldAppleGame(DWORD dwValue)
{
  processOnce(EACHIEVECOND_GOLD_APPLE_GAME, TVecQWORD{ static_cast<QWORD>(dwValue) });
}

void Achieve::onJoinPolly()
{
  processTime(EACHIEVECOND_JOIN_POLLY, TVecQWORD{ static_cast<QWORD>(1) });
}
void Achieve::onGoldAppleTotal(DWORD dwValue)
{
  processTime(EACHIEVECOND_GOLD_APPLE_TOTAL, TVecQWORD{ static_cast<QWORD>(dwValue) });
}

void Achieve::onWedding(EAchieveCond eCond, const TVecQWORD& vecParam /*= TVecQWORD{}*/)
{
  processTime(eCond, vecParam);
}

void Achieve::timer(DWORD curSec)
{
  onMonsterDraw(curSec);
}

bool Achieve::collectMostUserName(EShareDataType eType, DWORD dwNum, QueryUserResumeAchCmd& cmd)
{
  TSetQWORD setCharIDs;
  if (m_pUser->getShare().collectMostCharID(eType, dwNum, setCharIDs) == false)
    return false;
  if (setCharIDs.empty() == true)
    return false;

  TListMaxInfo& listInfo = m_mapMaxCache[eType];

  // remove old
  for (auto l = listInfo.begin(); l != listInfo.end();)
  {
    auto s = setCharIDs.find(l->charid());
    if (s == setCharIDs.end())
    {
      l = listInfo.erase(l);
      continue;
    }
    ++l;
  }

  // add new
  for (auto &s : setCharIDs)
  {
    auto v = find_if(listInfo.begin(), listInfo.end(), [s](const MaxInfo& r) -> bool{
      return r.charid() == s;
    });
    if (v == listInfo.end())
    {
      MaxInfo oInfo;
      oInfo.set_type(eType);
      oInfo.set_charid(s);
      listInfo.push_back(oInfo);
    }
  }

  // update name
  for (auto l = listInfo.begin(); l != listInfo.end(); ++l)
  {
    if (l->name().empty() == false)
      continue;

    if (eType == ESHAREDATATYPE_MOST_MUSICCD)
    {
      const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(l->charid());
      if (pCFG != nullptr)
        l->set_name(pCFG->strNameZh);
      continue;
    }

    SceneUser* pUser = SceneUserManager::getMe().getUserByID(l->charid());
    if (pUser != nullptr)
    {
      l->set_name(pUser->name);
    }
    else
    {
      GCharReader oReader(thisServer->getRegionID(), l->charid());
      if (oReader.getNameOnly() == true)
        l->set_name(oReader.getName());
    }
  }

  if (eType == ESHAREDATATYPE_MOST_TEAMTIME)
  {
    cmd.mutable_data()->clear_max_teams();
    for (auto &l : listInfo)
      cmd.mutable_data()->add_max_teams(l.name());
  }
  else if (eType == ESHAREDATATYPE_MOST_HANDTIME)
  {
    cmd.mutable_data()->clear_max_hands();
    for (auto &l : listInfo)
      cmd.mutable_data()->add_max_hands(l.name());
  }
  else if (eType == ESHAREDATATYPE_MOST_WHELL)
  {
    cmd.mutable_data()->clear_max_wheels();
    for (auto &l : listInfo)
      cmd.mutable_data()->add_max_wheels(l.name());
  }
  else if (eType == ESHAREDATATYPE_MOST_CHAT)
  {
    cmd.mutable_data()->clear_max_chats();
    for (auto &l : listInfo)
      cmd.mutable_data()->add_max_chats(l.name());
  }
  else if (eType == ESHAREDATATYPE_MOST_MUSICCD)
  {
    cmd.mutable_data()->clear_max_music();
    for (auto &l : listInfo)
      cmd.mutable_data()->add_max_music(l.name());
  }
  else if (eType == ESHAREDATATYPE_MOST_SAVE)
  {
    cmd.mutable_data()->clear_max_save();
    for (auto &l : listInfo)
      cmd.mutable_data()->add_max_save(l.name());
  }
  else if (eType == ESHAREDATATYPE_MOST_BESAVED)
  {
    cmd.mutable_data()->clear_max_besave();
    for (auto &l : listInfo)
      cmd.mutable_data()->add_max_besave(l.name());
  }
  else
  {
    return false;
  }

  return true;
}

void Achieve::processOnce(EAchieveCond eCond, const TVecQWORD& vecParam /*= TVecQWORD{}*/, DWORD dwAdd /*= 1*/, bool bPatch /*= false*/, DWORD dwSource)
{
  TVecQWORD vecFinishIDs;
  DWORD dwNow = xTime::getCurSec();
  map<EAchieveType, NewAchieveNtfAchCmd> mapUpdates;

  const TVecAchieveCFG& vecCFG = AchieveConfig::getMe().getAchieveCond(eCond);
  for (auto &v : vecCFG)
  {
    if(!v.checkSource(dwSource))
      continue;

    if (v.checkEnable(m_pUser->getProfession(), eCond, vecParam) == false)
      continue;

    SAchieveItem* pItem = getAchieveItem(v.dwID);
    if (pItem != nullptr && pItem->dwFinishTime != 0)
    {
      if (bPatch)
        pItem->dwProcess = pItem->dwFinishTime = 0;
      else
        continue;
    }

    DWORD dwRealProcess = AchieveProcess::getMe().getProcess(m_pUser, v, vecParam, 0, dwAdd);
    if (v.checkFinish(vecParam, dwRealProcess) == false)
      continue;

    SAchieveItem& rItem = pItem == nullptr ? m_mapItem[v.dwID] : *pItem;
    rItem.dwID = v.dwID;
    rItem.dwProcess = v.getProcess();
    rItem.dwFinishTime = dwNow;
    rItem.pCFG = AchieveConfig::getMe().getAchieveCFG(v.dwID);

    vecFinishIDs.push_back(v.dwID);

    AchieveItem* item = mapUpdates[v.eType].add_items();
    rItem.toData(item, m_pUser);

    XLOG << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "达成成就" << item->ShortDebugString() << XEND;
  }

  for (auto &m : mapUpdates)
  {
    if (m_pUser->getMenu().isOpen(EMENUID_MANUAL) == true && m.second.items_size() > 0)
    {
      m.second.set_type(m.first);
      PROTOBUF(m.second, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
  }

  if (vecFinishIDs.empty() == false)
  {
    onAchieveFinish(vecFinishIDs);
    m_pUser->getMenu().refreshNewMenu(EMENUCOND_ACHIEVE);
  }
}

void Achieve::processTime(EAchieveCond eCond, const TVecQWORD& vecParam /*= TVecQWORD{}*/, DWORD dwAdd /*= 1*/, bool bPatch /*= false*/, DWORD dwSource)
{
  TVecQWORD vecFinishIDs;
  DWORD dwNow = xTime::getCurSec();
  map<EAchieveType, NewAchieveNtfAchCmd> mapUpdates;

  const TVecAchieveCFG& vecCFG = AchieveConfig::getMe().getAchieveCond(eCond);
  for (auto &v : vecCFG)
  {
    if(!v.checkSource(dwSource))
      continue;

    if (!bPatch && v.checkEnable(m_pUser->getProfession(), eCond, vecParam) == false)
      continue;

    SAchieveItem* pItem = getAchieveItem(v.dwID);
    if (pItem != nullptr && pItem->dwFinishTime != 0)
    {
      if (bPatch)
        pItem->dwProcess = pItem->dwFinishTime = 0;
      else
        continue;
    }

    SAchieveItem& rItem = pItem == nullptr ? m_mapItem[v.dwID] : *pItem;
    rItem.dwID = v.dwID;
    rItem.pCFG = rItem.pCFG == nullptr ? AchieveConfig::getMe().getAchieveCFG(v.dwID) : rItem.pCFG;

    DWORD dwOri = rItem.dwProcess;
    rItem.dwProcess = AchieveProcess::getMe().getProcess(m_pUser, v, vecParam, dwOri, dwAdd);
    if (dwOri == rItem.dwProcess)
      continue;

    rItem.dwFinishTime = v.checkFinish(vecParam, rItem.dwProcess) == false ? 0 : dwNow;

    if (rItem.dwFinishTime != 0)
    {
      rItem.dwProcess = v.getProcess();
      vecFinishIDs.push_back(v.dwID);
    }

    AchieveItem oItem;
    AchieveItem* item = (!v.bVisibility || rItem.dwFinishTime != 0) ? mapUpdates[v.eType].add_items() : &oItem;
    rItem.toData(item, m_pUser);

    XLOG << "[玩家-成就]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << (rItem.dwFinishTime == 0 ? "推进成就" : "达成成就") << item->ShortDebugString() << XEND;
  }

  for (auto &m : mapUpdates)
  {
    if (m_pUser->getMenu().isOpen(EMENUID_MANUAL) == true && m.second.items_size() > 0)
    {
      m.second.set_type(m.first);
      PROTOBUF(m.second, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
  }

  if (vecFinishIDs.empty() == false)
  {
    onAchieveFinish(vecFinishIDs);
    m_pUser->getMenu().refreshNewMenu(EMENUCOND_ACHIEVE);
  }
}

void Achieve::version_update()
{
  for (DWORD d = m_dwDataVersion; d < ACHIEVE_VERSION; ++d)
  {
    if (d == 2)
      version_2();
    else if (d == 3)
      version_3();
    else if (d == 4)
      version_4();
    else if (d == 5)
      version_5();
    else if (d == 6)
      version_6();
    else if (d == 7)
      version_7();
    else if (d == 8)
      version_8();
    else if (d == 9)
      version_9();
    else if (d == 10)
      version_10();
    else if (d == 11)
      version_11();
    else if (d == 12)
      version_12();
    else if (d == 13)
      version_13();
  }

  m_dwDataVersion = ACHIEVE_VERSION;

  for (DWORD d = m_dwCharVersion; d < ACHIEVE_CHAR_VERSION; ++d)
  {
    if (d == 0)
      char_version_0();
    else if (d == 1)
      char_version_1();
  }
  m_dwCharVersion = ACHIEVE_CHAR_VERSION;

  initDefault();
  refreshAchieve();
}

void Achieve::version_2()
{
  for (auto &m : m_mapItem)
  {
    SAchieveItem& rItem = m.second;
    if (rItem.dwFinishTime != 0 && rItem.pCFG != nullptr)
    {
      TVecItemInfo vecItems = rItem.pCFG->vecReward;
      m_pUser->getPackage().addItem(vecItems, EPACKMETHOD_AVAILABLE);
    }
  }

  onCharge();
  onExpressAdd();
  onPassDojo(false);
  onPortrait();
  onManual(EMANUALTYPE_SCENERY, EACHIEVECOND_SCENERY_COUNT);
  onRune();
  onHair(false);
  onCat(false);
  onQuestSubmit(17020001);

  for (DWORD d = 1003; d <= 1007; ++d)
    onTitle(d);

  XLOG << "[成就-版本-2]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_3()
{
  DWORD dwNow = xTime::getCurSec();
  DWORD dwCount = m_pUser->getManual().getNumByStatus(EMANUALTYPE_FASHION, EMANUALSTATUS_UNLOCK_STEP);
  for (DWORD d = 1601001; d <= 1601005; ++d)
  {
    SAchieveItem* pItem = getAchieveItem(d);
    if (pItem == nullptr || pItem->dwFinishTime != 0)
      continue;
    if (pItem->dwProcess >= dwCount)
      continue;

    pItem->dwProcess = dwCount;
    pItem->dwFinishTime = pItem->pCFG->checkFinish(TVecQWORD{}, pItem->dwProcess) == true ? dwNow : 0;
    XLOG << "[成就-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "成就" << d << "更新进度" << pItem->dwProcess << "状态" << pItem->dwFinishTime << XEND;
  }
}

void Achieve::version_4()
{
  onManual(EMANUALTYPE_SCENERY, EACHIEVECOND_SCENERY_COUNT);
  XLOG << "[成就-版本-4]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_5()
{
  static const DWORD id = 1451083;
  const SAchieveCFG* pCFG = AchieveConfig::getMe().getAchieveCFG(id);
  if (pCFG == nullptr)
  {
    XERR << "[成就-版本-5]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行,id :" << id << "未在 Table_Achievement.txt 表中找到" << XEND;
    return;
  }
  auto m = m_mapItem.find(id);
  if (m == m_mapItem.end())
  {
    XERR << "[成就-版本-5]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行,id :" << id << "还未开始" << XEND;
    return;
  }

  SAchieveItem& rItem = m->second;
  rItem.dwProcess = AchieveProcess::getMe().getProcess(m_pUser, *pCFG, TVecQWORD{}, rItem.dwProcess, 0);
  rItem.dwFinishTime = pCFG->checkFinish(TVecQWORD{}, rItem.dwProcess) == false ? 0 : xTime::getCurSec();

  if (rItem.dwFinishTime != 0)
  {
    rItem.dwProcess = pCFG->getProcess();
    TVecItemInfo vecItems = pCFG->vecReward;
    m_pUser->getPackage().addItem(vecItems, EPACKMETHOD_AVAILABLE);
    m_pUser->getManual().addPoint(pCFG->dwManualExp, EMANUALTYPE_MIN, 0);
  }

  XLOG << "[成就-版本-5]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << (rItem.dwFinishTime == 0 ? "推进成就" : "达成成就") << rItem.dwID << XEND;
  XLOG << "[成就-版本-5]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_6()
{
  auto m = m_mapItem.find(1403012);
  if (m != m_mapItem.end() && m->second.dwFinishTime == 0)
  {
    auto item = m_mapItem.find(1403011);
    if (item != m_mapItem.end())
    {
      if (item->second.dwProcess > m->second.dwProcess)
        m->second.dwProcess = item->second.dwProcess;
    }
  }
  m = m_mapItem.find(1403013);
  if (m != m_mapItem.end() && m->second.dwFinishTime == 0)
  {
    auto item = m_mapItem.find(1403012);
    if (item != m_mapItem.end())
    {
      if (item->second.dwProcess > m->second.dwProcess)
        m->second.dwProcess = item->second.dwProcess;
    }
  }

  XLOG << "[成就-版本-6]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_7()
{
  for (auto &m : m_mapItem)
  {
    if (m.second.dwFinishTime != 0)
      m.second.bRewardGet = true;
  }
  XLOG << "[成就-版本-7]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_8()
{
  DWORD dwMax = 0;
  for (DWORD d = 1601001; d <= 1601005; ++d)
  {
    auto m = m_mapItem.find(d);
    if (m == m_mapItem.end())
      continue;

    if (m->second.dwProcess > dwMax)
      dwMax = m->second.dwProcess;
  }

  if (dwMax != 0)
  {
    auto item = m_mapItem.find(1601006);
    if (item == m_mapItem.end())
    {
      SAchieveItem& rItem = m_mapItem[1601006];
      rItem.dwID = 1601006;
      rItem.pCFG = AchieveConfig::getMe().getAchieveCFG(1601006);

      item = m_mapItem.find(1601006);
    }

    if (item != m_mapItem.end() && item->second.dwFinishTime == 0)
    {
      if (dwMax > item->second.dwProcess)
      {
        item->second.dwProcess = dwMax;
        if (item->second.pCFG != nullptr)
        {
          item->second.dwFinishTime = item->second.pCFG->checkFinish(TVecQWORD{}, item->second.dwProcess) == false ? 0 : xTime::getCurSec();
          if (item->second.dwFinishTime != 0)
            item->second.dwProcess = item->second.pCFG->getProcess();
        }
        else
        {
          XERR << "[成就-版本-8]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "1601006 未在 Table_Achievement.txt 表中找到" << XEND;
        }
      }
    }
  }

  XLOG << "[成就-版本-8]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_9()
{
  for (DWORD d = 1602053; d <= 1602055; ++d)
  {
    SAchieveItem* pItem = getAchieveItem(d);
    if (pItem == nullptr || pItem->dwFinishTime != 0)
      continue;
    if (pItem->pCFG == nullptr)
    {
      XERR << "[成就-版本-9]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行失败 id :" << d << "未在 Table_Achievement.txt 表中找到" << XEND;
      continue;
    }

    pItem->dwProcess = AchieveProcess::getMe().getProcess(m_pUser, *pItem->pCFG, TVecQWORD{}, pItem->dwProcess, 0);
    if (pItem->dwProcess < pItem->pCFG->getProcess())
      continue;

    pItem->dwProcess = pItem->pCFG->getProcess();
    pItem->dwFinishTime = xTime::getCurSec();
  }
  for (DWORD d = 1203001; d <= 1203003; ++d)
  {
    SAchieveItem* pItem = getAchieveItem(d);
    if (pItem == nullptr || pItem->dwFinishTime != 0)
      continue;
    if (pItem->pCFG == nullptr)
    {
      XERR << "[成就-版本-9]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行失败 id :" << d << "未在 Table_Achievement.txt 表中找到" << XEND;
      continue;
    }

    pItem->dwProcess = AchieveProcess::getMe().getProcess(m_pUser, *pItem->pCFG, TVecQWORD{}, pItem->dwProcess, 0);
    if (pItem->dwProcess < pItem->pCFG->getProcess())
      continue;

    pItem->dwProcess = pItem->pCFG->getProcess();
    pItem->dwFinishTime = xTime::getCurSec();
  }

  XLOG << "[成就-版本-9]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_10()
{
  return;
  DWORD dwNow = xTime::getCurSec();
  const TVecAchieveCFG& vecCFG = AchieveConfig::getMe().getAchieveCond(EACHIEVECOND_QUEST_SUBMIT);
  for (auto &v : vecCFG)
  {
    auto m = m_mapItem.find(v.dwID);
    if (m == m_mapItem.end())
      continue;

    bool bFinish = false;
    m->second.dwProcess = 0;
    for (auto &quest : v.stCondition.vecParams)
    {
      bool bSubmit = m_pUser->getQuest().isSubmit(quest);
      bFinish |= bSubmit;
      if (bSubmit)
        m->second.dwProcess += 1;
    }
    m->second.dwFinishTime = bFinish ? dwNow : 0;
    XLOG << "[成就-版本-10]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << v.dwID << "设置完成状态" << (bFinish ? "完成" : "未完成") << XEND;
  }
  XLOG << "[成就-版本-10]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_11()
{
  const TSetDWORD setIDs = TSetDWORD{1203053, 1203054, 1203055};
  DWORD dwMax = 0;
  DWORD dwNow = xTime::getCurSec();
  bool bAdd = false;

  for (auto &s : setIDs)
  {
    SAchieveItem& rItem = m_mapItem[s];
    if (rItem.dwFinishTime != 0)
    {
      dwMax = rItem.dwProcess;
      bAdd = true;
      continue;
    }
    if (rItem.dwProcess >= dwMax)
      continue;

    rItem.dwID = s;
    if (rItem.pCFG == nullptr)
      rItem.pCFG = AchieveConfig::getMe().getAchieveCFG(s);
    if (rItem.pCFG == nullptr)
      continue;

    if (bAdd)
      rItem.dwProcess += dwMax;
    else
      rItem.dwProcess = dwMax;
    dwMax = rItem.dwProcess;

    if (rItem.dwProcess >= rItem.pCFG->getProcess())
    {
      rItem.dwFinishTime = dwNow;
      rItem.dwProcess = rItem.pCFG->getProcess();
    }

    bAdd = false;

    XLOG << "[成就-版本-11]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "成就" << s << "刷新process :" << rItem.dwProcess << "time :" << rItem.dwFinishTime << XEND;
  }
  XLOG << "[成就-版本-11]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_12()
{
  DWORD dwOldID = 1501006;
  DWORD dwNewID = 1502004;

  auto m = m_mapItem.find(dwOldID);
  if (m != m_mapItem.end())
  {
    SAchieveItem& rNewItem = m_mapItem[dwNewID];
    rNewItem.dwID = m->second.dwID;
    rNewItem.dwProcess = m->second.dwProcess;
    rNewItem.dwFinishTime = m->second.dwFinishTime;
    rNewItem.bRewardGet = m->second.bRewardGet;
    rNewItem.pCFG = AchieveConfig::getMe().getAchieveCFG(dwNewID);
    for (DWORD d = 0; d < MAX_PARAM; ++d)
      rNewItem.arrParams[d] = m->second.arrParams[d];

    m_mapItem.erase(m);
  }

  XLOG << "[成就-版本-12]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::version_13()
{
  processTime(EACHIEVECOND_QUEST_SUBMIT, TVecQWORD{}, 1, true);
  XLOG << "[成就-版本-13]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Achieve::char_version_0()
{
  return;
  // 祈祷等级
  onPrayLvUp();
  // 祈祷晶体卡片消耗
  auto getnum = [&](const SGuildPrayCFG* pCFG, DWORD lv) -> DWORD
  {
    if (pCFG == nullptr)
      return 0;
    DWORD num = 0;
    for (DWORD i = 1; i <= lv; ++i)
    {
      const SGuildPrayItemCFG* pItemCFG = pCFG->getItem(i);
      if (pItemCFG == nullptr)
        continue;
      for (auto &v : pItemCFG->vecCosts)
      {
        if (v.id() == ITEM_PRAY_ATKCARD || v.id() == ITEM_PRAY_DEFCARD || v.id() == ITEM_PRAY_ELEMCARD)
          num += v.count();
      }
    }
    return num;
  };

  DWORD costnum = 0;
  const GuildUserInfo& rInfo = m_pUser->getGuild().getGuildUserInfo();
  for (int i = 0; i < rInfo.prays_size(); ++i)
  {
    const GuildMemberPray& pray = rInfo.prays(i);
    const SGuildPrayCFG* pCFG = GuildConfig::getMe().getGuildPrayCFG(pray.pray());
    if (pCFG == nullptr)
      continue;
    costnum += getnum(pCFG, pray.lv());
  }

  if (costnum)
    onCostPrayItem(costnum);
}

void Achieve::char_version_1()
{
  DWORD dwAchieveID = 1207005;
  const SAchieveCFG* pCFG = AchieveConfig::getMe().getAchieveCFG(dwAchieveID);
  if (pCFG == nullptr)
  {
    XERR << "[成就-角色版本-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwAchieveID << "未在 Table_Achievement.txt 表中找到" << XEND;
    return;
  }

  SAchieveItem* pItem = getAchieveItem(pCFG->dwID);
  if (pItem != nullptr && pItem->dwFinishTime != 0)
  {
    XLOG << "[成就-角色版本-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwAchieveID << "已完成" << XEND;
    return;
  }

  bool bHas = false;

  SManualItem* pManualItem = m_pUser->getManual().getManualItem(EMANUALTYPE_FASHION);
  if (pManualItem != nullptr)
  {
    SManualSubItem* pSubItem = pManualItem->getSubItem(51083);
    bHas = pSubItem != nullptr && pSubItem->bStore;
    if (!bHas)
    {
      pSubItem = pManualItem->getSubItem(51109);
      bHas = pSubItem != nullptr && pSubItem->bStore;
    }
  }

  if (!bHas)
  {
    TSetDWORD setPacks = TSetDWORD{EPACKTYPE_MAIN, EPACKTYPE_EQUIP, EPACKTYPE_BARROW, EPACKTYPE_PERSONAL_STORE};
    for (auto &pack : setPacks)
    {
      EPackType eType = static_cast<EPackType>(pack);
      BasePackage* pPack = m_pUser->getPackage().getPackage(eType);
      if (pPack == nullptr)
      {
        XERR << "[成就-角色版本-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << eType << "未找到" << XEND;
        continue;
      }
      auto fashion = find_if(pCFG->stCondition.vecParams.begin(), pCFG->stCondition.vecParams.end(), [pPack](DWORD d) -> bool{
          return pPack->getGUIDByType(d).empty() == false;
          });
      if (fashion != pCFG->stCondition.vecParams.end())
      {
        bHas = true;
        break;
      }
    }
  }

  if (bHas)
  {
    SAchieveItem& rItem = m_mapItem[dwAchieveID];

    rItem.dwID = dwAchieveID;
    rItem.dwProcess = pCFG->getProcess();
    rItem.dwFinishTime = xTime::getCurSec();
    rItem.pCFG = pCFG;
  }

  XLOG << "[成就-角色版本-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行结果" << (bHas ? "完成" : "未完成") << XEND;
}

void Achieve::patch_1(DWORD dwID)
{
  for (auto &m : m_mapItem)
  {
    SAchieveItem& rItem = m.second;
    if (rItem.dwID == dwID)
    {
      if(rItem.dwFinishTime != 0 && rItem.pCFG != nullptr)
      {
        TVecItemInfo vecItems = rItem.pCFG->vecReward;
        m_pUser->getPackage().addItem(vecItems, EPACKMETHOD_AVAILABLE);
      }
      break;
    }
  }

  XLOG << "[成就-补丁-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << dwID << XEND;
}

void Achieve::initDefault()
{
  const TVecAchieveCFG& vecCFG = AchieveConfig::getMe().getAchieveCond(EACHIEVECOND_QUEST_SUBMIT);
  for (auto &v : vecCFG)
  {
    auto m = m_mapItem.find(v.dwID);
    if (m != m_mapItem.end())
      continue;

    SAchieveItem& rItem = m_mapItem[v.dwID];
    rItem.dwID = v.dwID;
    rItem.dwProcess = rItem.dwFinishTime = 0;
    rItem.pCFG = &v;
  }
}

void Achieve::refreshAchieve()
{
  DWORD dwNow = xTime::getCurSec();

  const TVecAchieveCFG& vecCFGAch = AchieveConfig::getMe().getAchieveCond(EACHIEVECOND_ACHIEVE_FINISH);
  for (auto &v : vecCFGAch)
  {
    auto m = m_mapItem.find(v.dwID);
    if (m == m_mapItem.end())
    {
      SAchieveItem& rItem = m_mapItem[v.dwID];
      rItem.dwID = v.dwID;
      rItem.pCFG = AchieveConfig::getMe().getAchieveCFG(v.dwID);
      m = m_mapItem.find(v.dwID);
      if (m == m_mapItem.end())
        continue;
    }
    if (m->second.dwFinishTime != 0)
      continue;

    DWORD dwProcess = 0;
    for (auto &achieve : v.stCondition.vecParams)
    {
      SAchieveItem* pItem = getAchieveItem(achieve);
      if (pItem != nullptr && pItem->dwFinishTime != 0)
        ++dwProcess;
    }
    m->second.dwProcess = dwProcess;
    if (m->second.dwProcess < v.getProcess())
      continue;

    m->second.dwFinishTime = dwNow;
    XLOG << "[成就-刷新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << v.dwID << "刷新进度" << m->second.dwProcess << "时间" << m->second.dwFinishTime << XEND;
  }
}

void Achieve::onPrayLvUp()
{
  processTime(EACHIEVECOND_DEF_PRAY_LV);
  processTime(EACHIEVECOND_ATK_PRAY_LV);
  processTime(EACHIEVECOND_ELEM_PRAY_LV);
}

void Achieve::onCostPrayItem(DWORD dwNum)
{
  processTime(EACHIEVECOND_PRAY_CARD_USE, TVecQWORD{}, dwNum);
}

void Achieve::onProfession()
{
  processOnce(EACHIEVECOND_PROFESSION);
}

void Achieve::onComposePet()
{
  processTime(EACHIEVECOND_PET_COMPOSE);
}

void Achieve::onPetSkillAllLv(DWORD level)
{
  processOnce(EACHIEVECOND_PETSKILL_ALL_LV, TVecQWORD{level});
}

void Achieve::onPetFriendLvUp(DWORD level)
{
  processOnce(EACHIEVECOND_PET_FRIENDLV);
  processTime(EACHIEVECOND_PET_FRIENDLV_NUM, TVecQWORD{level});
}

