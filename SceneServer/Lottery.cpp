#include "Lottery.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "Package.h"
#include "MsgManager.h"
#include "LuaManager.h"
#include "DScene.h"
#include "ActivityManager.h"
#include "MailManager.h"
#include "ActivityEventManager.h"
#include "SysmsgConfig.h"

// dojo
Lottery::Lottery(SceneUser* pUser) : m_pUser(pUser)
{
}

Lottery::~Lottery()
{
}
void Lottery::sendLotteryCfg(Cmd::ELotteryType type)
{
  QueryLotteryInfo cmd;
  cmd.set_type(type);
  if (checkLotteryCnt(type))
  {
    cmd.set_today_cnt(m_pUser->getVar().getVarValue(getVarCntType(type)));
    cmd.set_max_cnt(MiscConfig::getMe().getLotteryCFG().getMaxLotteryCnt(DWORD(type)));
  }  
  const TMapLotteryCFG* pMapCFG = ItemConfig::getMe().getAllLotteryCFG(type);
  if (pMapCFG)
  {
    LotteryInfo* pInfo = nullptr;
    for (auto& m : *pMapCFG)
    {
      if (!isRightTime(m.second.dwYear, m.second.dwMonth))
        continue;
      
      if (type == ELotteryType_Magic)
      { //合并为一个LotteryInfo
        if (pInfo == nullptr)
          pInfo = cmd.add_infos();
      }
      else
      {
        pInfo = cmd.add_infos();
      }     
      if (pInfo == nullptr)
        continue;
      pInfo->set_year(m.second.dwYear);
      pInfo->set_month(m.second.dwMonth);

      //check activity discount
      DWORD dwYearMonth = m.second.dwYear * 100 + m.second.dwMonth;
      ActivityEventInfo* pDiscountEventInfo = ActivityEventManager::getMe().getLotteryDiscount(m_pUser, type, ECoinType_Coin, dwYearMonth, 1);
      DWORD useDiscountCnt = 0;
      DWORD discount = 0;
      if (pDiscountEventInfo)
      {
        useDiscountCnt = m_pUser->getUserSceneData().getActivityEventCnt(EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT, pDiscountEventInfo->id(), pDiscountEventInfo->lotterydiscount().usertype());
        if ((useDiscountCnt + 1) > pDiscountEventInfo->lotterydiscount().count())
        {
        }
        else
        {
          discount = pDiscountEventInfo->lotterydiscount().discount();
        }
      }

      pInfo->set_price(getPrice(&m.second, discount));
      pInfo->set_discount(getDisCount(&m.second, discount));
      if (type == ELotteryType_Head)
      {
        const SLotteryGiveCFG* pGiveCFG = ItemConfig::getMe().getLotteryGiveCFGByTime(m.second.dwYear, m.second.dwMonth);
        if (pGiveCFG)
          pInfo->set_lotterybox(pGiveCFG->dwLotteryBoxItemId);
      }

      for (auto& subM : m.second.mapItemCFG)
      {
        LotterySubInfo* pSubInfo = pInfo->add_subinfo();
        if (!pSubInfo)
          continue;
        pSubInfo->set_id(subM.first);
        pSubInfo->set_itemid(subM.second.getItemId());
        pSubInfo->set_recover_price(subM.second.dwRecoveryCount);
        pSubInfo->set_rarity(subM.second.strRarity);
        pSubInfo->set_recover_itemid(subM.second.dwRecoveryItemId);
        if (type == ELotteryType_Magic)
          pSubInfo->set_cur_batch(subM.second.isCurBatch());
        else
          pSubInfo->set_cur_batch(false);
        pSubInfo->set_rate(subM.second.getRate());
        pSubInfo->set_count(subM.second.dwCount);
        pSubInfo->set_female_itemid(subM.second.dwFemaleItemId);
      }
    }
  } 

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[扭蛋-请求配置] charid" << m_pUser->id <<"type" << type << "msg" << cmd.ShortDebugString() << XEND;
  if (type == ELotteryType_Head)
    sendBuyLotteryCnt();
}

//cmd.guid() 表示使用的时扭蛋来扭蛋
bool Lottery::lottery(Cmd::LotteryCmd& cmd)
{
  //check npc pos
  if (cmd.guid().empty())
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(cmd.npcid());
    if (pNpc == nullptr || pNpc->getScene() == nullptr)
    {
      XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "未通过npc进行扭蛋操作" << XEND;
      return false;
    }
    float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
    if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
    {
      XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "失败,距离npc过远" << XEND;
      return false;
    }
  }
  else
  {
    if (fillCmd(cmd) == false)
      return false;
  }

  // 检测联动时间
  if(!MiscConfig::getMe().getLotteryCFG().checkLinkage(cmd.type(), cmd.year(), cmd.month()))
  {
    MsgManager::sendMsg(m_pUser->id, 25610);
    return false;
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;

  bool bCoin = false;
  // bool bTicket = false;
  // bool bItem = false;
  if (cmd.ticket())
  {
    // bTicket = true;
  }
  else
  {
    bCoin = true;
  }
  
  //check lottery cnt
  DWORD dwLotteryCnt = 0;
  EVarType varCntType = getVarCntType(cmd.type());
  DWORD dwLotteryPoolCoin = getLotteryPoolCoin(cmd.type(), cmd.year(), cmd.month());
#ifdef _DEBUG
  XLOG << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->name << "扭蛋池积分，coin" << dwLotteryPoolCoin << "type" << cmd.type() << "year" << cmd.year() << "month" << cmd.month() << XEND;
#endif
  if (bCoin)
  {
    DWORD c = cmd.count();
    if (c == 0) 
      c = 1;
    dwLotteryCnt = m_pUser->getVar().getVarValue(varCntType);
    DWORD dwMaxCount = MiscConfig::getMe().getLotteryCFG().getMaxLotteryCnt(cmd.type());
    if (dwMaxCount)
    {
      if (checkLotteryCnt(cmd.type()))
      {
        if ((dwLotteryCnt + c) > dwMaxCount)
        {
          XERR << "[包裹-扭蛋] 超过今日上限" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "dwLotteryCnt" << dwLotteryCnt << c << "上限次数" << dwMaxCount << XEND;
          return false;
        }
      }
    }
  }
  
  if (!isRightTime(cmd.year(), cmd.month()))
  {
    XERR << "[包裹-扭蛋], 不能超前扭蛋" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "year" << cmd.year() << "month" << cmd.month() << XEND;
    return false;
  }
  
  DWORD itemType = 0;
  if (cmd.type() == ELotteryType_Magic)
  {
    itemType = MiscConfig::getMe().getLotteryCFG().randomItemType(cmd.ticket());
    if (itemType == 0)
    {
      XERR << "[包裹-扭蛋], 魔力扭蛋随机itemtype 出错" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "year" << cmd.year() << "month" << cmd.month() << XEND;
      return false;
    }
  }

  const SLotteryCFG* pLotteryCFG = ItemConfig::getMe().getLotteryCFG(cmd.type(), cmd.year(), cmd.month(), itemType);
  if (pLotteryCFG == nullptr)
  {
    XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未能找到配置" << "type" << cmd.type() << "year" << cmd.year() << "month" << cmd.month() << "itemType" << itemType << XEND;
    return false;
  }

  const SGuildBuildingCFG* pGBuildingCFG = nullptr;
  if (cmd.type() == ELotteryType_CatLitterBox)
  {
    if (pMainPack->isFull())
    {
      MsgManager::sendMsg(m_pUser->id, 989);
      return false;
    }
    GuildScene* pGuildScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
    if (pGuildScene == nullptr)
    {
      XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "不在公会场景" << XEND;
      return false;
    }
    DWORD buildinglv = pGuildScene->getGuild().getBuildingLevel(EGUILDBUILDING_CAT_LITTER_BOX);
    if (buildinglv <= 0)
    {
      XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑未建造" << XEND;
      return false;
    }
    pGBuildingCFG = GuildConfig::getMe().getGuildBuildingCFG(EGUILDBUILDING_CAT_LITTER_BOX, buildinglv);
    if (pGBuildingCFG == nullptr)
    {
      XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑等级:" << buildinglv << "建筑配置找不到" << XEND;
      return false;
    }
  }

  // 扭蛋次数
  // if (cmd.count() > 10)
  //   return false;
  DWORD times = 1;//cmd.count() <= 0 ? 1 : cmd.count();

  bool needaddcount = isNeedAddCount(cmd.ticket() != 0, cmd.type());
  if (cmd.guid().empty())
  {
    // 扭蛋次数影响价格时不支持一次请求扭多次
    if (needaddcount && times > 1)
      return false;
  }
  else
  {
    needaddcount=true;
    times=cmd.count();   
  }

  DWORD count = 0;
  //check money
  DWORD money = 0;
  ECoinType eCoinType = ECoinType_Min;
  if (cmd.ticket())
    eCoinType = ECoinType_Ticket;
  else if (cmd.guid().empty())
    eCoinType = ECoinType_Coin;
  //活动折扣
  DWORD dwYearMonth = cmd.year() * 100 + cmd.month();
  ActivityEventInfo* pDiscountEventInfo = ActivityEventManager::getMe().getLotteryDiscount(m_pUser, cmd.type(), eCoinType, dwYearMonth, times);
  DWORD useDiscountCnt = 0;
  DWORD discount = 0;
  if (pDiscountEventInfo)
  {
    useDiscountCnt = m_pUser->getUserSceneData().getActivityEventCnt(EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT, pDiscountEventInfo->id(), pDiscountEventInfo->lotterydiscount().usertype());
    XDBG << "[活动模板-扭蛋打折] charid" << m_pUser->id << m_pUser->name << "扭蛋类型" << cmd.type() << "货币类型" << eCoinType << "使用折扣次数" << useDiscountCnt << "上限折扣次数" << pDiscountEventInfo->lotterydiscount().count() << "本次想要次数" << times << "配置折扣" << pDiscountEventInfo->lotterydiscount().discount() << XEND;
    if ((useDiscountCnt + times) > pDiscountEventInfo->lotterydiscount().count())
    {
      XERR << "[活动模板-扭蛋打折] 打折次数已经用完，charid" << m_pUser->id << m_pUser->name << "扭蛋类型" << cmd.type() << "货币类型" << eCoinType << "使用折扣次数" << useDiscountCnt << "上限折扣次数" << pDiscountEventInfo->lotterydiscount().count() << "本次想要次数" << times << "配置折扣" << pDiscountEventInfo->lotterydiscount().discount() << XEND;
    }
    else
    {
      discount = pDiscountEventInfo->lotterydiscount().discount();
      XLOG << "[活动模板-扭蛋打折] charid" << m_pUser->id << m_pUser->name << "扭蛋类型" << cmd.type() << "货币类型" << eCoinType << "使用折扣次数" << useDiscountCnt << "上限折扣次数" << pDiscountEventInfo->lotterydiscount().count() << "本次想要次数" << times << "配置折扣" << pDiscountEventInfo->lotterydiscount().discount() << XEND;
    }
  }

  if (cmd.ticket())
  {
    count = MiscConfig::getMe().getLotteryCFG().getTicketCount(cmd.type(), cmd.ticket());
    if (count == 0)
      return false;
    count *= times;    
    if (discount)
      count = (count * discount) / 100;
    // check item count
    if (pMainPack->checkItemCount(cmd.ticket(), count) == false)
    {
      XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "扭蛋券不足,需要" << count << cmd.ticket() << XEND;
      return false;
    }
  }
  else
  {
    if (cmd.guid().empty())
    {
      money = getPrice(pLotteryCFG, discount);
      if (money == 0)
      {
        XERR << "[包裹-扭蛋] 价格为0" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "价格不一致" << "year" << cmd.year() << "month" << cmd.month() << "服务器价格" << money << "客户端价格" << cmd.price() << XEND;
        return false;
      }
      money *= times;
      if (discount)
        money = (money * discount) / 100;

      if (money != cmd.price())
      {
        XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "价格不一致" << "year" << cmd.year() << "month" << cmd.month() << "服务器价格" << money << "客户端价格" << cmd.price() << XEND;
        MsgManager::sendMsg(m_pUser->id, 3550);
        sendLotteryCfg(cmd.type());
        return false;
      }

      if (m_pUser->checkMoney(EMONEYTYPE_LOTTERY, money) == false)
      {
        MsgManager::sendMsg(m_pUser->id, 3551);
        XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "扭蛋币不足,需要" << money << XEND;
        return false;
      }
    }
    else
    {
      //扭蛋扭蛋 在fillCmd已经验证过了
    }
  }

  // map<DWORD, DWORD> items;
  std::vector<std::tuple <UINT, UINT, UINT>> items;

  for (DWORD i = 0; i < times; ++i)
  {
    DWORD itemId = 0, itemCount = 1, itemRate = 0;
    if (cmd.ticket())
    {
      if (cmd.type() == ELotteryType_CatLitterBox) // 公会建筑福利猫砂盆, 随机数量与公会等级相关
      {
        if (pGBuildingCFG == nullptr)
          return false;
        if (pGBuildingCFG->oUnlockParam.has("rewardid") == false)
        {
          XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑:" << pGBuildingCFG->eType << pGBuildingCFG->dwLv << "rewardid未配置" << XEND;
          return false;
        }
        DWORD rewardid = pGBuildingCFG->oUnlockParam.getTableInt("rewardid");
        TVecItemInfo items;
        if (RewardManager::roll(rewardid, m_pUser, items, ESOURCE_LOTTERY_CATLITTERBOX) == false)
        {
          XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑:" << pGBuildingCFG->eType << pGBuildingCFG->dwLv << "rewardid:" << reward << "获取道具失败" << XEND;
          return false;
        }
        if (items.empty())
        {
          XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑:" << pGBuildingCFG->eType << pGBuildingCFG->dwLv << "rewardid:" << reward << "获得道具为空" << XEND;
          return false;
        }
        itemId = items[0].id();
        itemCount = items[0].count();
      }
      else
      {
        if (pLotteryCFG->randomWithHideWeight(itemId,itemCount, m_pUser->getUserSceneData().getGender(), itemRate) == false)
        {
          XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name <<"type" << cmd.type()<< "扭蛋券，扭蛋币随机出错" << money << "扭蛋券" << cmd.ticket() << "itemtype" << itemType << XEND;
          return false;
        }
      }
    }
    else
    {
      // 先查看保底池
      // 如果获取id为0继续；否则走原来流程
      DWORD keyLP = LotteryPool::getKey(cmd.type(), cmd.year(), cmd.month());
      if(ItemConfig::getMe().getItemFromLotteryPool(itemId, itemCount, itemRate, m_pUser->getUserSceneData().getGender(), keyLP, dwLotteryPoolCoin, m_vecItemPool))
      {
        if(!itemId || !itemCount)
        {
          XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name  <<"type" << cmd.type()<< "扭蛋机保底池出错" << money << "扭蛋券" << cmd.ticket() <<"itemtype"<<itemType << "itemId" << itemId << "itemCount" << itemCount << XEND;
          return false;
        }
        XLOG << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name  <<"type" << cmd.type()<< "扭蛋币保底池获取成功" << money << "扭蛋券" << cmd.ticket() << "itemtype" << itemType << "itemId" << itemId << "itemCount" << itemCount << XEND;
      }
      else
      {
        if (pLotteryCFG->random(itemId,itemCount, m_pUser->getUserSceneData().getGender(), itemRate) == false)
        {
          XERR << "[包裹-扭蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name  <<"type" << cmd.type()<< "扭蛋币随机出错" << money << "扭蛋券" << cmd.ticket() <<"itemtype"<<itemType << XEND;
          return false;
        }
      }
    }
    // auto it = items.find(itemId);
    // if (it == items.end())
    //   items[itemId] = 0;
    // items[itemId] += itemCount;
    items.push_back(std::make_tuple(itemId, itemCount, itemRate));

    if (bCoin)
    {
      dwLotteryCnt++;
    }
    useDiscountCnt++;
  }

  auto getSource = [](Cmd::ELotteryType type, bool isCurMonth)->ESource {    
    switch (type)
    {
    case ELotteryType_Head:
      if (isCurMonth)
        return ESOURCE_LOTTERY_HEAD;
      else
        return ESOURCE_LOTTERY_HEAD_OLD;
    case ELotteryType_Equip:
      return ESOURCE_LOTTERY_EQUIP;
    case ELotteryType_Card:
      return ESOURCE_LOTTERY_CARD;
    case ELotteryType_CatLitterBox:
      return ESOURCE_LOTTERY_CATLITTERBOX;
    case ELotteryType_Magic:
      return ESOURCE_LOTTERY_MAGIC;
    default:
      break;
    }
    return ESOURCE_LOTTERY;
  };

  ESource src = getSource(cmd.type(), pLotteryCFG->isCurMonth());
  TVecItemInfo vecItem;
  for (auto& v : items)
  {
    ItemInfo itemInfo;
    itemInfo.set_id(get<0>(v));
    itemInfo.set_count(get<1>(v));
    itemInfo.set_source(src);
    vecItem.push_back(itemInfo);
  }
  if (pMainPack->checkAddItem(vecItem) == false)
  {
    MsgManager::sendMsg(m_pUser->id, 989);
    return false;
  }

  if (cmd.ticket())
    pMainPack->reduceItem(cmd.ticket(), src, count);
  else
  {
    if (cmd.guid().empty())
      m_pUser->subMoney(EMONEYTYPE_LOTTERY, money, src);
    else
      pMainPack->reduceItem(cmd.guid(), src, times);
  }
  for (auto&v : vecItem)
  {
  	pMainPack->addItem(v);
  }

  for (auto& v : items)
  {
    ItemInfo* p = cmd.add_items();
    if (p)
    {
      string guid = pMainPack->getGUIDByType(get<0>(v));
      p->set_guid(guid);
      p->set_id(get<0>(v));
    }
  }
  cmd.set_charid(m_pUser->id);
  cmd.set_today_cnt(dwLotteryCnt);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  getSpringReward(cmd.type());

  EFirstActionType actionType = EFIRSTACTION_LOTTERY;
  switch (cmd.type())
  {
  case ELotteryType_Head:
    actionType = EFIRSTACTION_LOTTERY;
    break;
  case ELotteryType_Equip:
    actionType = EFIRSTACTION_LOTTERY_EQUIP;
    break;
  case ELotteryType_Card:
    actionType = EFIRSTACTION_LOTTERY_CARD;
    break;
  case ELotteryType_Magic:
    actionType = EFIRSTACTION_LOTTERY_MAGIC;
  default:
    break;
  }

  if (bCoin)
  {
    m_pUser->getVar().setVarValue(varCntType, dwLotteryCnt);
    addLotteryPoolCoin(cmd.type(), cmd.year(), cmd.month(), money);
  }
  if (discount)
  {
    m_pUser->getUserSceneData().setActivityEventCnt(EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT, pDiscountEventInfo->id(), pDiscountEventInfo->lotterydiscount().usertype(), useDiscountCnt);
    ActivityEventManager::getMe().sendActivityEventCount(m_pUser, EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT);
  }

  DWORD cnt = 0;
  if (needaddcount && !discount) //参加活动折扣了这个次数就不增加了
  {
    EAccVarType varType = getAccVarType(cmd.type());
    cnt = m_pUser->getVar().getAccVarValue(varType);
    cnt += times;
    m_pUser->getVar().setAccVarValue(varType, cnt);
    sendLotteryCfg(cmd.type());
  }

  if (cmd.guid().empty())
  {
    bool bFirst = m_pUser->getUserSceneData().addFirstActionDone(actionType);

    if (bFirst)
      m_bSkipLotteryAnim = false;
    else
      m_bSkipLotteryAnim = cmd.skip_anim();
  }

  if(cmd.type() != ELotteryType_CatLitterBox)
  {
    m_pUser->getServant().onFinishEvent(ETRIGGER_LOTTERY);
    if(cmd.type() == ELotteryType_Magic)
      m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_BIG_LOTTERY);
  }
  else
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EXTRACT_CAT_LITTER);

  for(auto& v : items)
  {
    DWORD itemId = get<0>(v);
    DWORD itemRate = get<2>(v);
    // 判断物品是否为稀有物品
    if(ItemConfig::getMe().isRareItem(itemId))
    {
      auto it = std::find(m_vecItemPool.begin(), m_vecItemPool.end(), itemId);
      if(m_vecItemPool.end() == it)
      {
        m_vecItemPool.push_back(itemId);
#ifdef _DEBUG
        XLOG << "[包裹-扭蛋] 获取保底池物品，charid" << m_pUser->id << "item" << itemId << XEND;
#endif
      }
    }

    DWORD rateMax = MiscConfig::getMe().getLotteryCFG().m_dwSaveRateMax;
    if(0 == rateMax)
      rateMax = 500; // 保底值
    if(itemRate > rateMax)
      continue;

    const SItemCFG* itemCfg = ItemConfig::getMe().getItemCFG(itemId);
    if(!itemCfg)
      continue;

    LotteryResultRecordCmd cmd1;
    cmd1.set_charid(m_pUser->id);
    cmd1.set_itemid(itemId);
    cmd1.set_type(cmd.type());
    cmd1.set_name(m_pUser->name);
    cmd1.set_itemname(itemCfg->strNameZh);
    cmd1.set_rate(itemRate);

    PROTOBUF(cmd1, send1, len1);
    thisServer->sendCmdToData(send1, len1);
  }

  XLOG << "[包裹-扭蛋] 成功扭蛋，charid" << m_pUser->id << "type" << cmd.type() << "ticket" << cmd.ticket() << "花费" << money << "跳过动画" << m_bSkipLotteryAnim << "今日次数" << cnt << "年" << cmd.year() << "月" << cmd.month()  <<"itemtype"<<itemType<<"今日扭蛋次数" << dwLotteryCnt<<  "获得";
  for (auto& v : items)
    XLOG << get<0>(v) << get<1>(v) << get<2>(v);
  for (auto& v : vecItem)
    XLOG << v.id() << v.count();
  XLOG << XEND;
  return true;
}

bool Lottery::lotteryRecovery(Cmd::LotteryRecoveryCmd& cmd)
{
  if (cmd.type() != ELotteryType_Head && cmd.type() != ELotteryType_Equip && cmd.type() != ELotteryType_Magic)
    return false;

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(cmd.npcid());
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
  {
    XERR << "[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "未通过npc进行扭蛋操作" << XEND;
    return false;
  }
  float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "失败,距离npc过远" << XEND;
    return false;
  }

  DWORD recoveryItemId = 0;
  DWORD recoveryCnt = 0;
  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;

  map<string, DWORD> mapMaterial;
  for (int i = 0; i < cmd.guids_size(); ++i)
    mapMaterial[cmd.guids(i)]++;

  for (auto &m : mapMaterial)
  {
    ItemBase* pItemBase = pMainPack->getItem(m.first);
    if (!pItemBase)
    {
      XERR << "[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "材料" << m.first << "背包里找不到" << XEND;
      return false;
    }
    const SLotteryItemCFG* pLotteryItemCFG = ItemConfig::getMe().getLotteryItemCFG(cmd.type(), pItemBase->getTypeID());
    if (!pLotteryItemCFG)
    {
      XERR << "[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "材料" << m.first << pItemBase->getTypeID()
        << "配置表找不到" << XEND;
      return false;
    }
    if (pLotteryItemCFG->eType != cmd.type())
    {
      XERR << "[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "材料" << m.first << pItemBase->getTypeID()
        << "类型不一致"<<pLotteryItemCFG->eType << cmd.type() << XEND;
      return false;
    }

    if (pMainPack->checkItemCount(m.first, m.second) == false)
    {
      XERR << "[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "材料" << m.first << "数量不足" << XEND;
      return false;
    }
    recoveryItemId = pLotteryItemCFG->dwRecoveryItemId;
    recoveryCnt += pLotteryItemCFG->dwRecoveryCount * m.second;
    {
      string guid = m.first;
      DWORD itemId = pItemBase->getTypeID();
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pItemBase);
      if (pEquip)
      {
        if (pEquip->getStrengthLv() != 0)
        {
          DWORD retMoney;
          if (!m_pUser->getPackage().decompose(guid, false, retMoney))
          {
            XERR << "[包裹-扭蛋回收] 强化装备分解失败 user =" << m_pUser->accid << m_pUser->id << "type" << cmd.type() << "guid =" << guid << "itemid =" << itemId << XEND;
            return false;
          }
          else
          {
            if (retMoney)
              MsgManager::sendMsg(m_pUser->id, 10115, MsgParams(retMoney));
          }
        }
        if (pEquip->EquipedCard())
        {  
          if (!m_pUser->getPackage().equipAllCardOff(guid))
          {
            XERR << "[包裹-扭蛋回收] 装备插卡脱下失败 user =" << m_pUser->accid << m_pUser->id << "type" << cmd.type() << "guid =" << guid << "itemid =" << itemId <<XEND;
            return false;
          }
        }        

        if (pEquip->getRefineLv())
        {
          //精炼额外回收个数
          DWORD refineCnt = LuaManager::getMe().call<DWORD>("calcRefineRecovery", itemId, pEquip->getRefineLv(),pEquip->isDamaged(), cmd.type()) *  m.second;
          recoveryCnt += refineCnt;
          XDBG << "[[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name<<"type" <<cmd.type() <<"物品id" <<itemId <<"精炼等级" <<pEquip->getRefineLv() << "精炼额外回收个数" << refineCnt << XEND;
        }
      }
    }
  }
  ItemInfo itemInfo;
  itemInfo.set_id(recoveryItemId);
  itemInfo.set_count(recoveryCnt);
  itemInfo.set_source(ESOURCE_LOTTERY_RECOVERY);

  if (!pMainPack->checkAddItem(itemInfo, EPACKMETHOD_CHECK_WITHPILE))
  {
    MsgManager::sendMsg(m_pUser->id, 989);
    XERR << "[包裹-扭蛋回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type" << cmd.type() << "背包空间不足" << recoveryCnt << XEND;
    return false;
  }

  for (auto &m : mapMaterial)
  {
    pMainPack->reduceItem(m.first, ESOURCE_LOTTERY_RECOVERY, m.second);
  }

  pMainPack->addItem(itemInfo,EPACKMETHOD_CHECK_WITHPILE);

  XLOG << "[包裹-扭蛋回收] 成功回收，charid" << m_pUser->id << "获得扭蛋券" << recoveryItemId << recoveryCnt << "花费"<<"type"<<cmd.type() << cmd.ShortDebugString() << XEND;
  return true;
}

DWORD Lottery::getPrice(const SLotteryCFG* pCfg, bool bDiscount)
{
  if (!pCfg)
    return 0;
  //使用活动折扣了，使用原价
  if (bDiscount) 
  {
    return MiscConfig::getMe().getLotteryCFG().dwPrice;
  }
  else
  {
    EAccVarType varType = getAccVarType(pCfg->eType);
    if (varType != EACCVARTYPE_MIN)
    {
      DWORD cnt = m_pUser->getVar().getAccVarValue(varType);
      return LuaManager::getMe().call<DWORD>("calcLotteryCost", pCfg->eType, cnt);
    }
    else
      return pCfg->needMoney();
  } 
}

DWORD Lottery::getDisCount(const SLotteryCFG* pCfg, bool bDiscount)
{
  if (!pCfg)
    return 0;
  //使用活动折扣了，使用原价
  if (bDiscount)
  {
    return 0;
  }

  if (pCfg->eType == ELotteryType_Head)
    return pCfg->getDisCount();
  return 0;
}

Cmd::EAccVarType Lottery::getAccVarType(DWORD type)
{
  switch (type)
  {
  case ELotteryType_Head:
    return EACCVARTYPE_MIN;
  case ELotteryType_Equip:
    return EACCVARTYPE_LOTTERY_CNT_EQUIP;
  case ELotteryType_Card:
    return EACCVARTYPE_LOTTERY_CNT_CARD;
  case ELotteryType_Magic:
    return EACCVARTYPE_MIN;
  default:
    break;
  }
  return EACCVARTYPE_MIN;
}

bool Lottery::isRightTime(DWORD dwYear, DWORD dwMonth)
{

  DWORD curSec = now();
  curSec -= 5 * 60 * 60;
  DWORD year = xTime::getYear(curSec);
  DWORD month = xTime::getMonth(curSec);
  if (year < dwYear)
    return false;
  if (year == dwYear && month < dwMonth)
    return false;
  
  return true;
}

void Lottery::getSpringReward(Cmd::ELotteryType type)
{
  const SGlobalActCFG* pCFG = ActivityConfig::getMe().getGlobalActCFG(static_cast<DWORD>(GACTIVITY_SPRING_LOTTERY));
  if (pCFG == nullptr)
    return ;

  bool isOpen = ActivityManager::getMe().isOpen(pCFG->m_dwId);
  if(isOpen == false)
    return;

  if(pCFG->isParamExist(static_cast<DWORD>(type)) == false)
    return;

  DWORD dwRewardID = pCFG->getParam(0);
  m_pUser->getPackage().rollReward(dwRewardID, EPACKMETHOD_AVAILABLE, false, true);
  XLOG << "[包裹-扭蛋] 额外奖励" << m_pUser->accid << m_pUser->id << m_pUser->name << "type : " << type << "rewardid: "<< dwRewardID << XEND;
}

bool Lottery::lotteryGive(const Cmd::LotteryGiveInfo& cmd)
{  
  //check count
  DWORD maxCnt = MiscConfig::getMe().getLotteryCFG().m_dwDayBuyLotteryGiveCnt;
  DWORD boughtCnt = m_pUser->getVar().getVarValue(EVARTYPE_DAY_LOTTERY_BUY_GIVE_CNT);
  if (maxCnt && (boughtCnt + cmd.count()) > maxCnt)
  {
    XERR <<"[扭蛋-赠送] 失败，超过次数"<<m_pUser->accid <<m_pUser->id <<m_pUser->name <<"购买次数"<<boughtCnt <<cmd.count() <<"最大次数"<< maxCnt<< XEND;
    return false;
  }

  if (cmd.year() == 0 || cmd.month() == 0 || cmd.receiverid() == 0 || cmd.count() == 0)
  {
    return false;
  }
  
  const SLotteryGiveCFG* pGiveCFG = ItemConfig::getMe().getLotteryGiveCFGByTime(cmd.year(), cmd.month());
  if (pGiveCFG == nullptr)
  {
    XERR << "[扭蛋-赠送] 找不到配置" << m_pUser->accid << m_pUser->id << m_pUser->name << "year" << cmd.year() << "month" << cmd.month() << XEND;
    return false;
  }
  bool bIsCurMonth = xTime::isCurMonth(pGiveCFG->dwYear, pGiveCFG->dwMonth);
  DWORD dwPrice = bIsCurMonth ? MiscConfig::getMe().getLotteryCFG().dwDiscountPrice : MiscConfig::getMe().getLotteryCFG().dwSendPrice;
  if (dwPrice == 0)
    return false;
  QWORD qwCostMoney = dwPrice * cmd.count();
  //check money
  if (!m_pUser->checkMoney(EMONEYTYPE_LOTTERY, qwCostMoney))
  {
    XERR << "[扭蛋-赠送] b格猫金币不够" << m_pUser->accid << m_pUser->id << m_pUser->name << "year" << cmd.year() << "month" << cmd.month() <<"price"<<dwPrice <<"cost"<<qwCostMoney << XEND;
    return false;
  }

  QWORD qwCostDeposit = qwCostMoney * 10000;
  //check deposit
  if(!m_pUser->getDeposit().checkQuota(qwCostDeposit))
  {
    XERR << "[扭蛋-赠送] 赠送额度不够" << m_pUser->accid << m_pUser->id << m_pUser->name << "year" << cmd.year() << "month" << cmd.month() <<"cost"<<qwCostDeposit << "left" << m_pUser->getDeposit().getQuota() << XEND;
    return false;
  }

  //扭蛋盒子
  TVecItemInfo vecItemInfo;
  ItemInfo boxItem;
  boxItem.set_id(pGiveCFG->dwLotteryBoxItemId);
  boxItem.set_count(cmd.count());
  boxItem.set_source(ESOURCE_LOTTERY_GIVE);
  vecItemInfo.push_back(boxItem);
  
  //祝福卡
  if (MiscConfig::getMe().getLotteryCFG().dwLoveLeterItemId == 0)
    return false;
  const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(pGiveCFG->dwLotteryItemId);
  if (!pItemCFG)
    return false;

  TVecItemData vecItemData;
  ItemData oItemData;
  ItemInfo* pItemInfo = oItemData.mutable_base();
  if (!pItemInfo)
    return false;
    
  pItemInfo->set_id(MiscConfig::getMe().getLotteryCFG().dwLoveLeterItemId);
  pItemInfo->set_count(1);
  pItemInfo->set_source(ESOURCE_LOTTERY_GIVE);

  LoveLetterData* pLoveData = oItemData.mutable_letter();
  if (!pLoveData)
    return false;
  pLoveData->set_sendusername(m_pUser->name);
  pLoveData->set_configid(cmd.configid());
  pLoveData->set_content(cmd.content());
  std::stringstream ss;
  ss << SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_THIS) << cmd.count() * 10 << SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_PIECES) <<pItemCFG->strNameZh << SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_LITTLE_MIND);
  pLoveData->set_content2(ss.str());
  vecItemData.push_back(oItemData);
  if (MailManager::getMe().sendMail(cmd.receiverid(), m_pUser->id, m_pUser->name, SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_GIVE_LOTTERY), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_GIVE_LOTTERY), EMAILTYPE_LOTTERY_GIVE, 0, vecItemInfo, vecItemData, EMAILATTACHTYPE_ITEM, false, MsgParams(), 0, 0, 0, false) == false)
    return false;
  m_pUser->subMoney(EMONEYTYPE_LOTTERY, qwCostMoney, ESOURCE_LOTTERY_GIVE);
  m_pUser->getDeposit().subQuota(qwCostDeposit, EQuotaType_C_Lottery);
  
  MsgManager::sendMsg(m_pUser->id, 25007);

  boughtCnt += cmd.count();

  m_pUser->getVar().setVarValue(EVARTYPE_DAY_LOTTERY_BUY_GIVE_CNT, boughtCnt);
  sendBuyLotteryCnt();
  XLOG << "[扭蛋-赠送] 成功" << m_pUser->accid << m_pUser->id << m_pUser->name << "year" << cmd.year() << "month" << cmd.month() << "count" << cmd.count() << "cost" << qwCostMoney <<"扭蛋盒"<<pGiveCFG->dwLotteryBoxItemId <<"祝福卡id"<< MiscConfig::getMe().getLotteryCFG().dwLoveLeterItemId << XEND;
  return true;
}

bool Lottery::fillCmd(Cmd::LotteryCmd& cmd)
{
  if (cmd.guid().empty())
    return false;
  ItemBase* pItem = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN)->getItem(cmd.guid());
  if (!pItem)
    return false;
  if (pItem->getCount()<cmd.count())
    return false;	  
  const SLotteryGiveCFG*pCfg = ItemConfig::getMe().getLotteryGiveCFGByLotteryId(pItem->getTypeID());
  if (!pCfg)
    return false;
  cmd.set_year(pCfg->dwYear);
  cmd.set_month(pCfg->dwMonth);
  return true;
}

void Lottery::sendBuyLotteryCnt()
{
  LotterGivBuyCountCmd cmd;
  cmd.set_got_count(m_pUser->getVar().getVarValue(EVARTYPE_DAY_LOTTERY_BUY_GIVE_CNT));
  cmd.set_max_count(MiscConfig::getMe().getLotteryCFG().m_dwDayBuyLotteryGiveCnt);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

EVarType Lottery::getVarCntType(Cmd::ELotteryType type)
{
  switch (type)
  {
    case ELotteryType_Head:
      return EVARTYPE_DAY_LOTTERY_CNT_HEAD;
    case ELotteryType_Equip:
      return EVARTYPE_DAY_LOTTERY_CNT_EQUIP;
    case ELotteryType_Card:
      return EVARTYPE_DAY_LOTTERY_CNT_CARD;
    case ELotteryType_Magic:
      return EVARTYPE_DAY_LOTTERY_CNT_MAGIC;
    default:
      return EVARTYPE_MIN;
  }
  return EVARTYPE_MIN;
} 

Cmd::EOptionType Lottery::getOptionType(Cmd::ELotteryType type)
{
  switch (type)
  {
  case ELotteryType_Head:
    return EOPTIONTYPE_LOTTERY_CNT_HEAD;
  case ELotteryType_Equip:
    return EOPTIONTYPE_LOTTERY_CNT_EQUIP;
  case ELotteryType_Card:
    return EOPTIONTYPE_LOTTERY_CNT_CARD;
  case ELotteryType_Magic:
    return EOPTIONTYPE_LOTTERY_CNT_MAGIC;
  default:
    return EOPTIONTYPE_MAX;
  }
  return EOPTIONTYPE_MAX;
}

bool Lottery::checkLotteryCnt(Cmd::ELotteryType type)
{
  Cmd::EOptionType eOptTye = Lottery::getOptionType(type);
  if (eOptTye != EOPTIONTYPE_MAX && m_pUser->getUserSceneData().getOption(eOptTye))
    return true;
  return false;
}

bool Lottery::load(const Cmd::BlobLottery& oData)
{
  m_dwPoolTime = oData.pooltime();
#ifdef _DEBUG
    XLOG << "[扭蛋-加载] pooltime:" << oData.pooltime() << XEND;
#endif
  m_vecItemPool.clear();
  for(int i=0; i<oData.items_size(); ++i)
  {
    m_vecItemPool.push_back(oData.items(i));
#ifdef _DEBUG
    XLOG << "[扭蛋-加载] item:" << oData.items(i) << XEND;
#endif
  }

  m_mapPoolCoin.clear();
  for(int i=0; i<oData.pools_size(); ++i)
  {
    m_mapPoolCoin[oData.pools(i).poolid()] = oData.pools(i).coin();
#ifdef _DEBUG
    XLOG << "[扭蛋-加载] poolid:" << oData.pools(i).poolid() << "coin:" << oData.pools(i).coin() << XEND;
#endif
  }

  return true;
}

bool Lottery::loadAcc(const Cmd::BlobLottery& oData)
{
  m_dwPoolTime = oData.pooltime();
#ifdef _DEBUG
    XLOG << "[扭蛋-加载] pooltime:" << oData.pooltime() << XEND;
#endif
  m_vecItemPool.clear();
  for(int i=0; i<oData.items_size(); ++i)
  {
    m_vecItemPool.push_back(oData.items(i));
#ifdef _DEBUG
    XLOG << "[扭蛋-加载] item:" << oData.items(i) << XEND;
#endif
  }

  m_mapPoolCoin.clear();
  for(int i=0; i<oData.pools_size(); ++i)
  {
    m_mapPoolCoin[oData.pools(i).poolid()] = oData.pools(i).coin();
#ifdef _DEBUG
    XLOG << "[扭蛋-加载] poolid:" << oData.pools(i).poolid() << "coin:" << oData.pools(i).coin() << XEND;
#endif
  }

  return true;
}

bool Lottery::save(Cmd::BlobLottery* pData)
{
  if(!pData || !m_pUser)
    return false;

  pData->Clear();
  pData->set_pooltime(m_dwPoolTime);
#ifdef _DEBUG
    XLOG << "[扭蛋-保存] pooltime:" << m_dwPoolTime << XEND;
#endif
  for(auto& m : m_vecItemPool)
  {
    pData->add_items(m);
#ifdef _DEBUG
    XLOG << "[扭蛋-保存] item:" << m << XEND;
#endif
  }

  for(auto& m : m_mapPoolCoin)
  {
    LotteryPoolData* pool = pData->add_pools();
    if(!pool)
      continue;
    pool->set_poolid(m.first);
    pool->set_coin(m.second);
#ifdef _DEBUG
    XLOG << "[扭蛋-保存] poolid:" << m.first << "coin:" << m.second << XEND;
#endif
  }

  XDBG << "[扭蛋-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "数据大小:" << pData->ByteSize() << XEND;
  return true;
}

bool Lottery::saveAcc(Cmd::BlobLottery* pData)
{
  if(!pData || !m_pUser)
    return false;

  pData->Clear();
  pData->set_pooltime(m_dwPoolTime);
#ifdef _DEBUG
    XLOG << "[扭蛋-保存] pooltime:" << m_dwPoolTime << XEND;
#endif
  for(auto& m : m_vecItemPool)
  {
    pData->add_items(m);
#ifdef _DEBUG
    XLOG << "[扭蛋-保存] item:" << m << XEND;
#endif
  }

  for(auto& m : m_mapPoolCoin)
  {
    LotteryPoolData* pool = pData->add_pools();
    if(!pool)
      continue;
    pool->set_poolid(m.first);
    pool->set_coin(m.second);
#ifdef _DEBUG
    XLOG << "[扭蛋-保存] poolid:" << m.first << "coin:" << m.second << XEND;
#endif
  }

  XDBG << "[扭蛋-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "数据大小:" << pData->ByteSize() << XEND;
  return true;
}

DWORD Lottery::getLotteryPoolCoin(DWORD dwType, DWORD dwYear, DWORD dwMonth)
{
  DWORD nowTime = now();
  if(!xTime::isSameMonth(m_dwPoolTime, nowTime, 0) || nowTime > m_dwPoolTime + 86400 * 32)
  {
    m_mapPoolCoin.clear();
    m_dwPoolTime = nowTime;
    return 0;
  }

  DWORD key = LotteryPool::getKey(dwType, dwYear, dwMonth);
  auto it = m_mapPoolCoin.find(key);
  if(m_mapPoolCoin.end() == it)
    return 0;

  return it->second;
}

void Lottery::addLotteryPoolCoin(DWORD dwType, DWORD dwYear, DWORD dwMonth, DWORD dwCoin)
{
  DWORD key = LotteryPool::getKey(dwType, dwYear, dwMonth);
  m_mapPoolCoin[key] += dwCoin;
  m_dwPoolTime = now();
}

