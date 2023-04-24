#include "SceneShop.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"
#include "SkillManager.h"
#include "MsgManager.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "GuildConfig.h"
#include "DScene.h"
#include "MatchSCmd.pb.h"
#include "ActivityEventManager.h"
#include "Menu.h"

SceneShop::SceneShop(SceneUser* pUser) : m_pUser(pUser)
{

}

SceneShop::~SceneShop()
{

}

bool SceneShop::load(const BlobShopGotItem& data)
{
  m_vecShopGotItem.clear();
  for (int i = 0; i < data.list_size(); ++i)
    m_vecShopGotItem.push_back(TPairShopGotItem(data.list(i).id(), data.list(i).count()));
  for (int i = 0; i < data.random_size(); ++i)
  {
    const ShopGotItem& rItem = data.random(i);
    m_mapShopRand[rItem.id()] = rItem.count();
  }
  m_mapLvRandShopItem.clear();
  for (int i = 0; i < data.randombylv_size(); ++i)
    m_mapLvRandShopItem[data.randombylv(i).id()] = data.randombylv(i).count();
  m_dwRefreshLv = data.refreshlv();
  m_dwRefreshLvResetTime = data.refreshlv_resettime();
  m_vecShopGotItemWeek.clear();
  for (int i = 0; i < data.listweek_size(); ++i)
    m_vecShopGotItemWeek.push_back(TPairShopGotItem(data.listweek(i).id(), data.listweek(i).count()));

  m_vecUserShopGotItemMonth.clear();
  for (int i = 0; i < data.limitmonth_size(); ++i)
    m_vecUserShopGotItemMonth.push_back(TPairShopGotItem(data.limitmonth(i).id(), data.limitmonth(i).count()));

  for(int i = 0; i < data.limititem_size(); ++i)
    m_mapLimitItem.emplace(data.limititem(i).id(), data.limititem(i).count());

  return true;
}

bool SceneShop::save(BlobShopGotItem* data)
{
  if (data == nullptr)
    return false;

  data->Clear();

  for (auto v = m_vecShopGotItem.begin(); v != m_vecShopGotItem.end(); ++v)
  {
    ShopGotItem* pItem = data->add_list();
    pItem->set_id(v->first);
    pItem->set_count(v->second);
  }

  for (auto&m : m_mapShopRand)
  {
    ShopGotItem* pItem = data->add_random();
    if (pItem)
    {
      pItem->set_id(m.first);
      pItem->set_count(m.second);
    }
  }

  for (auto& v : m_mapLvRandShopItem)
  {
    ShopGotItem* pItem = data->add_randombylv();
    if (pItem)
    {
      pItem->set_id(v.first);
      pItem->set_count(v.second);
    }
  }

  data->set_refreshlv(m_dwRefreshLv);
  data->set_refreshlv_resettime(m_dwRefreshLvResetTime);

  for (auto v = m_vecShopGotItemWeek.begin(); v != m_vecShopGotItemWeek.end(); ++v)
  {
    ShopGotItem* pItem = data->add_listweek();
    pItem->set_id(v->first);
    pItem->set_count(v->second);
  }

  for (auto v = m_vecUserShopGotItemMonth.begin(); v != m_vecUserShopGotItemMonth.end(); ++v)
  {
    ShopGotItem* pItem = data->add_limitmonth();
    pItem->set_id(v->first);
    pItem->set_count(v->second);
  } 

  for(auto &it : m_mapLimitItem)
  {
    ShopGotItem* pItem = data->add_limititem();
    pItem->set_id(it.first);
    pItem->set_count(it.second);
  }

  XDBG << "[场景商店-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << data->ByteSize() << XEND;
  return true;
}


bool SceneShop::loadAcc(const BlobShopGotItem& data)
{
  m_vecAccShopGotItem.clear();
  for (int i = 0; i < data.list_size(); ++i)
    m_vecAccShopGotItem.push_back(TPairShopGotItem(data.list(i).id(), data.list(i).count()));
  
  m_vecAccShopGotItemAlways.clear();
  for (int i = 0; i < data.month_size(); ++i)
    m_vecAccShopGotItemAlways.push_back(TPairShopGotItem(data.month(i).id(), data.month(i).count()));

  m_mapAccLvRandShopItem.clear();
  for (int i = 0; i < data.randombylv_size(); ++i)
    m_mapAccLvRandShopItem[data.randombylv(i).id()] = data.randombylv(i).count();

  m_mapAccDiscountActItem.clear();
  for (int i = 0; i < data.discountact_size(); ++i)
    m_mapAccDiscountActItem[data.discountact(i).id()] = data.discountact(i).count();

  m_mapShopRandAcc.clear();
  for (int i = 0; i < data.randomacc_size(); ++i)
    m_mapShopRandAcc[data.randomacc(i).id()] = data.randomacc(i).count();

  m_vecAccShopGotItemWeek.clear();
  for (int i = 0; i < data.listweek_size(); ++i)
    m_vecAccShopGotItemWeek.push_back(TPairShopGotItem(data.listweek(i).id(), data.listweek(i).count()));

  m_vecAccShopGotItemMonth.clear();
  for (int i = 0; i < data.limitmonth_size(); ++i)
    m_vecAccShopGotItemMonth.push_back(TPairShopGotItem(data.limitmonth(i).id(), data.limitmonth(i).count()));

  m_mapAccAddLimitCnt.clear();
  for (int i = 0; i < data.addcount_size(); ++i)
    m_mapAccAddLimitCnt[data.addcount(i).id()] = data.addcount(i).count();
  return true;
}

bool SceneShop::saveAcc(BlobShopGotItem* data)
{
  if (data == nullptr)
    return false;

  data->Clear();

  for (auto v = m_vecAccShopGotItem.begin(); v != m_vecAccShopGotItem.end(); ++v)
  {
    ShopGotItem* pItem = data->add_list();
    pItem->set_id(v->first);
    pItem->set_count(v->second);
  }

  for (auto v = m_vecAccShopGotItemAlways.begin(); v != m_vecAccShopGotItemAlways.end(); ++v)
  {
    ShopGotItem* pItem = data->add_month();
    pItem->set_id(v->first);
    pItem->set_count(v->second);
  }

  for (auto& v : m_mapAccLvRandShopItem)
  {
    ShopGotItem* pItem = data->add_randombylv();
    if (pItem)
    {
      pItem->set_id(v.first);
      pItem->set_count(v.second);
    }
  }

  for (auto& v : m_mapAccDiscountActItem)
  {
    ShopGotItem* pItem = data->add_discountact();
    if (pItem)
    {
      pItem->set_id(v.first);
      pItem->set_count(v.second);
    }
  }

  for (auto& v : m_mapShopRandAcc)
  {
    ShopGotItem* pItem = data->add_randomacc();
    if (pItem)
    {
      pItem->set_id(v.first);
      pItem->set_count(v.second);
    }
  }

  for (auto v = m_vecAccShopGotItemWeek.begin(); v != m_vecAccShopGotItemWeek.end(); ++v)
  {
    ShopGotItem* pItem = data->add_listweek();
    pItem->set_id(v->first);
    pItem->set_count(v->second);
  }

  for (auto v = m_vecAccShopGotItemMonth.begin(); v != m_vecAccShopGotItemMonth.end(); ++v)
  {
    ShopGotItem* pItem = data->add_limitmonth();
    pItem->set_id(v->first);
    pItem->set_count(v->second);
  }


  for (auto &m : m_mapAccAddLimitCnt)
  {
    ShopGotItem* pItem = data->add_addcount();
    if (pItem)
    {
      pItem->set_id(m.first);
      pItem->set_count(m.second);
    }
  }

  XDBG << "[场景商店-acc保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << data->ByteSize() << XEND;
  return true;
}

bool SceneShop::queryGotItem()
{
  // check var
  resetShopItemCount(false);
  sendGotItem();
  return true;
}

bool SceneShop::buyItem(DWORD dwShopID, DWORD dwCount, DWORD dwPrice/* = 0*/, DWORD dwPrice2/* = 0*/, bool fromMatch/*=false*/)
{
  if(m_pUser->checkPwd(EUNLOCKTYPE_NPC_BUY) == false)
  {
    XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买:" << dwShopID << "失败,密码验证失败" << XEND;
    return false;
  }

  // check max count
  if (dwCount > 999)
  {
    XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,超过单次购买上限" << XEND;
    return false;
  }
  dwCount = dwCount == 0 ? 1 : dwCount;

  // get shop config
  const ShopItem* pItem = ShopConfig::getMe().getShopItem(dwShopID);
  if (pItem == nullptr)
  {
    XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,为在Table_Shop.txt表中找到" << XEND;
    return false;
  }

  if (pItem->shoptype() == EShopType_ForClient)
    return false;
  if (pItem->itemtype() > 1)
    return false;

  if (pItem->source() == ESHOPSOURCE_GUILD)
  {
    // 商店来源为公会(消耗公会仓库的道具, 购买的道具添加到公会仓库中), 必须是会长/副会长
    if (m_pUser->hasGuild() == false)
      return false;
    const GuildSMember* member = m_pUser->getGuild().getMember(m_pUser->id);
    if (member == nullptr)
      return false;
    if (member->job() != EGUILDJOB_CHAIRMAN && member->job() != EGUILDJOB_VICE_CHAIRMAN)
    {
      MsgManager::sendMsg(m_pUser->id, 3808);
      return false;
    }
    ShopBuyItemGuildSCmd cmd;
    cmd.set_charid(m_pUser->id);
    cmd.set_id(dwShopID);
    cmd.set_count(dwCount);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
    return true;
  }

  DWORD shopmaxcount = pItem->maxcount();
  if (pItem->shoptype() == EShopType_RandomDay || pItem->shoptype() == EShopType_RandomDayAcc)
  {
    shopmaxcount = getShopItemMaxCount(pItem->shoptype(),dwShopID);
    if (shopmaxcount <= 0)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,在随机表中未找到" << XEND;
      return false;
    }
  }
  else if (pItem->shoptype() == EShopType_RandomDayByLv)
  {
    shopmaxcount = getRandShopItemMaxCount(dwShopID);
    if (shopmaxcount <= 0)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,在随机商店中未找到" << XEND;
      return false;
    }
  }
  else if (pItem->shoptype() == EShopType_RandomDayByAccLv)
  {
    shopmaxcount = getAccRandShopItemMaxCount(dwShopID);
    if (shopmaxcount <= 0)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,在账号随机商店中未找到" << XEND;
      return false;
    }
  }
  else if (pItem->shoptype() == EShopType_RandomWeek)
  {
    if (!ShopConfig::getMe().isInWeekRand(dwShopID))
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,在每周随机商店没找到" << XEND;
      return false;
    }
  }

  if (shopmaxcount)
  {
    auto it = m_mapAccAddLimitCnt.find(pItem->id());
    if (it != m_mapAccAddLimitCnt.end())
      shopmaxcount += it->second;
  }

  // 商品是否已上架
  if (canBuyItemNow(pItem) == false)
  {
    XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,商品未上架或已下架或等级不足" << XEND;
    MsgManager::sendMsg(m_pUser->id, 990);
    return false;
  }

  // 公会建筑商店, 检查商品是否解锁
  EGuildBuilding building = GuildConfig::getMe().getGuildBuildingByShopType(pItem->shoptype());
  if (building != EGUILDBUILDING_MIN)
  {
    GuildScene* pGuildScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
    if (pGuildScene == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount
           << "建筑:" << building << "不在公会场景" << XEND;
      return false;
    }
    DWORD buildinglv = pGuildScene->getGuild().getBuildingLevel(building);
    if (buildinglv <= 0)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount
           << "建筑:" << building << "建筑未建造" << XEND;
      return false;
    }
    const SGuildBuildingCFG* pGBuildingCFG = GuildConfig::getMe().getGuildBuildingCFG(building, buildinglv);
    if (pGBuildingCFG == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount
           << "建筑:" << building << "建筑等级:" << buildinglv << "建筑配置找不到" << XEND;
      return false;
    }
    if (pGBuildingCFG->dwShopType != pItem->shoptype())
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount
           << "建筑:" << building << "建筑等级:" << buildinglv << "需要type:" << pGBuildingCFG->dwShopType << "商品type:" << pItem->shoptype() << "不一致" << XEND;
      return false;
    }
    if (pGBuildingCFG->setShopID.find(pItem->id()) == pGBuildingCFG->setShopID.end())
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount
           << "建筑:" << building << "建筑等级:" << buildinglv << "商品未解锁" << XEND;
      return false;
    }
  }

  // check and reset count
  resetShopItemCount(true);

  // check skill
  if (pItem->skillid() != 0)
  {
    SceneFighter* pFighter = m_pUser->getFighter(EPROFESSION_NOVICE);
    if (pFighter == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,skillid :" << pItem->skillid() << "失败,未找到初心者" << XEND;
      return false;
    }
    SSkillData* pData = pFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
    if (pData == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,skillid :" << pItem->skillid() << "失败,未找到初心者" << XEND;
      return false;
    }
    SSkillItem* pSkillItem = pData->getSkillItem(pItem->skillid());
    if (pSkillItem != nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,skillid :" << pItem->skillid() << "失败,已学习过该技能" << XEND;
      return false;
    }
  }

  // check pack
  const SFoodMiscCFG& rCFG = MiscConfig::getMe().getFoodCfg();
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(pItem->itemid());
  if (pItem->itemid() != 0)
  {
    if (pCFG == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,itemid :" << pItem->itemid() << "失败,未在Table_Item.txt表中找到" << XEND;
      return false;
    }
    if (pCFG->eItemType == EITEMTYPE_HAIR || pCFG->eItemType == EITEMTYPE_HAIR_MALE || pCFG->eItemType == EITEMTYPE_HAIR_FEMALE)
    {
      if (m_pUser->getHairInfo().getCurHair(true) == pCFG->dwHairID)
      {
        XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "购买" << dwShopID << dwCount << "个,itemid :" << pItem->itemid() << "hairid :" << pCFG->dwHairID << "失败,当前已经是该发型" << XEND;
        return false;
      }
    }
    else if (pCFG->eItemType == EITEMTYPE_EYE_MALE || pCFG->eItemType == EITEMTYPE_EYE_FEMALE)
    {
      if (m_pUser->getEye().getCurID(true) == pCFG->dwTypeID)
      {
        XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个,itemid :" << pItem->itemid() << "失败,当前已经是此美瞳" << XEND;
        return false;
      }
    }
    else
    {
      bool bFood = m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == true && rCFG.isFoodItem(pCFG->eItemType) == true;
      EPackType eType = bFood ? EPACKTYPE_FOOD : EPACKTYPE_MAIN;
      BasePackage* pPack = m_pUser->getPackage().getPackage(eType);
      if (pPack == nullptr)
      {
        XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "购买" << dwShopID << dwCount << "个,itemid :" << pItem->itemid() << "失败,未找到" << eType << XEND;
        return false;
      }
      DWORD dwNum = pItem->num() == 0 ? 1 : pItem->num();

      // 检查道具获得次数上限, 购买数若超过剩余可获得数, 则不允许购买
      DWORD dwGetCnt = m_pUser->getPackage().getVarGetCnt(pItem->itemid(), ESOURCE_MIN);
      DWORD dwGetLimit = pCFG->getItemGetLimit(m_pUser->getLevel(), ESOURCE_MIN, m_pUser->getDeposit().hasMonthCard());
      if (dwCount * dwNum + dwGetCnt > dwGetLimit)
        return false;

      ItemInfo oItem;
      oItem.set_id(pItem->itemid());
      oItem.set_count(dwCount * dwNum);
      oItem.set_source(ESOURCE_SHOP);
      if (pCFG->eItemType != EITEMTYPE_CODE)    //礼包码破格入包
      {
        if (pPack->checkAddItem(oItem, EPACKMETHOD_CHECK_WITHPILE) == false)
        {
          MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
          return false;
        }
      }
    }

    // 判断道具是否有购买数量限制
    DWORD dwLimitNum = MiscConfig::getMe().getGameShopCFG().getLimitNum(pItem->itemid());
    if(dwLimitNum > 0)
    {
      DWORD dwNum = pItem->num() == 0 ? 1 : pItem->num();
      DWORD dwBuyNum = dwCount * dwNum;
      auto it = m_mapLimitItem.find(pItem->itemid());
      if(it != m_mapLimitItem.end())
        dwBuyNum += it->second;
      if(dwBuyNum > dwLimitNum)
      {
        MsgManager::sendMsg(m_pUser->id, 3436);
        return false;
      }
    }
  }

  //全服限购发送到匹配服验证数量
  if (pItem->producenum() && fromMatch == false)
  {
    //发送matchserver
    CheckCanBuyMatchSCmd matchCmd;
    matchCmd.set_id(dwShopID);
    matchCmd.set_count(dwCount);
    matchCmd.set_price(dwPrice);
    matchCmd.set_price2(dwPrice2);
    matchCmd.set_zoneid(thisServer->getZoneID());
    matchCmd.set_charid(m_pUser->id);
    PROTOBUF(matchCmd, send, len);
    thisServer->sendCmdToSession(send, len);
    XLOG << "[场景商店-购物], 全服商品，向matchserver验证数量" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "购买" << dwShopID << dwCount << "个" << XEND;
    return true;
  }

  DWORD dwClothColorID = 0;
  // check hair color
  if (pItem->haircolorid() != 0)
  {
    if (pItem->haircolorid() == m_pUser->getHairInfo().getRealHairColor())
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,haircolorid :" << pItem->haircolorid() << "失败,当前已是该发色" << XEND;
      return false;
    }
    const SHairColor* pCFG = TableManager::getMe().getHairColorCFG(pItem->haircolorid());
    if (pCFG == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,haircolorid :" << pItem->haircolorid() << "未在 Table_HairColor.txt 表中找到" << XEND;
      return false;
    }
  }
  else if (pItem->clothcolorid() != 0)
  {
    if (m_pUser->getMenu().isOpen(EMENUID_CLOTHCOLOR) == false)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:只有四转职业才可以使用");
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,clothcolorid :" << pItem->clothcolorid() << "失败,只有四转职业才可以使用" << XEND;
      return false;
    }
    const SCloth* pCFG = TableManager::getMe().getClothCFG(pItem->clothcolorid());
    if (pCFG == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,clothcolorid :" << pItem->clothcolorid() << "失败,未在 Table_Couture.txt 表中找到" << XEND;
      return false;
    }
    UserSceneData& rData = m_pUser->getUserSceneData();
    if (pCFG->getGender() != EGENDER_MIN && rData.getGender() != pCFG->getGender())
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,clothcolorid :" << pItem->clothcolorid() << "失败,性别错误" << XEND;
      return false;
    }
    if (pCFG->getProfession() != EPROFESSION_MIN && rData.getProfession() != pCFG->getProfession())
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,clothcolorid :" << pItem->clothcolorid() << "失败,职业错误" << XEND;
      return false;
    }
    if (m_pUser->getUserSceneData().getClothColor(true) == pCFG->getClothID())
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,clothcolorid :" << pItem->clothcolorid() << "失败,当前衣服已是该颜色" << XEND;
      return false;
    }
    dwClothColorID = pItem->clothcolorid();
  }
  else if (pItem->clothcolorids_size() != 0)
  {
    if (pItem->clothcolorids_size() != 2 || pItem->clothcolorids(0) > pItem->clothcolorids(1))
      return false;

    DWORD dwMin = pItem->clothcolorids(0);
    DWORD dwMax = pItem->clothcolorids(1);
    for (DWORD d = dwMin; d <= dwMax; ++d)
    {
      const SCloth* pCloth = TableManager::getMe().getClothCFG(d);
      if (pCloth == nullptr || pCloth->isOnSale() == false)
        continue;
      if (pCloth->getGender() != EGENDER_MIN && pCloth->getGender() != m_pUser->getUserSceneData().getGender())
        continue;
      if (pCloth->getProfession() != EPROFESSION_MIN && pCloth->getProfession() != m_pUser->getProfession())
        continue;
      dwClothColorID = d;
      break;
    }
    if (dwClothColorID == 0)
      return false;
    const SCloth* pCFG = TableManager::getMe().getClothCFG(dwClothColorID);
    if (pCFG == nullptr)
      return false;
    if (m_pUser->getUserSceneData().getClothColor(true) == pCFG->getClothID())
      return false;
  }

  // check item unlock
  if (pItem->menuid() != 0 && m_pUser->getMenu().isOpen(pItem->menuid()) == false)
  {
    XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "购买" << dwShopID << dwCount << "个失败,需要" << pItem->menuid() << "开启" << XEND;
    return false;
  }

  // check item has show
  if (!checkShowShopItem(*pItem))
  {
    // 检查该商品是否已经向玩家展示出售，若没有 说明前端数据可能不是最新
    QueryShopConfigCmd ntf;
    ntf.set_type(pItem->shoptype());
    ntf.set_shopid(pItem->shopid());
    queryConfigByType(ntf);
    return false;
  }

  // check appellation
  if (pItem->skillid() != 0)
  {
    SceneFighter* pNoviceFighter = m_pUser->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter == nullptr)
      return false;
    SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
    if (pData == nullptr || pData->getSkillItem(pItem->skillid()) != nullptr)
      return false;
    const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(pItem->skillid());
    if (pSkillCFG == nullptr || pSkillCFG->checkCondition(m_pUser) == false)
      return false;
    if (m_pUser->getManual().getSkillPoint() < pSkillCFG->getLevelCost())
      return false;

    m_pUser->getManual().subSkillPoint(pSkillCFG->getLevelCost());
  }

  // check max item
  if (ShopConfig::buyLimit(pItem->limittype()))
  {
    DWORD shopcount = dwCount;
    DWORD count = getShopItemCount(dwShopID);
    if (shopmaxcount != 0)
    {
      if (count + shopcount > shopmaxcount)
      {
        XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "购买" << dwShopID << dwCount << "个失败,超过最大购买上限" << shopmaxcount << XEND;
        return false;
      }

      // add count
      addShopItemCount(dwShopID, dwCount);
    }
  }

  DWORD actDiscount = 0, disCount = 0, origCount = dwCount;
  const SShopDiscountActivity* discountAct = ShopConfig::getMe().getDiscountActivity(pItem->id());
  if (discountAct != nullptr && discountAct->isActivityStart())
  {
    if (discountAct->vecQuestID.empty() == false)
    {
      bool questdone = false;
      for (auto questid : discountAct->vecQuestID)
      {
        if (QuestManager::getMe().isQuestComplete(questid, m_pUser))
        {
          questdone = true;
          break;
        }
      }
      if (questdone == false)
      {
        MsgManager::sendMsg(m_pUser->id, 3603);
        return false;
      }
    }

    if (discountAct->isDiscount())
    {
      DWORD actCount = 0, count = getDiscountActItemCount(dwShopID);
      if (count < discountAct->dwCount)
        actCount = discountAct->dwCount - count; // 可享受折扣商品数量
      actDiscount = discountAct->dwDiscount; // 折扣

      if (actDiscount > 0 && actCount > 0)
      {
        if (actCount >= dwCount)
        {
          disCount = dwCount; // 折扣商品数量
          origCount = 0; // 原价商品数量
        }
        else
        {
          disCount = actCount; // 折扣商品数量
          origCount = dwCount - actCount; // 原价商品数量
        }
      }
    }
  }

  // check and sub money
  bool bNeedZeny = true;
  TVecItemInfo vecItems;
  if (dwCount == 1 && pItem->precost())
  {    
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr || pMainPack->checkItemCount(pItem->precost()) == false)
    {
      bNeedZeny = true;
    }
    else
    {
      pMainPack->reduceItem(pItem->precost(), ESOURCE_SHOP, 1);
      bNeedZeny = false;
    }
  }

  if (bNeedZeny &&(pItem->moneyid() != 0 || pItem->moneyid2() != 0))
  {
    float attrDiscount = 0;
    if (pItem->business() == 1) // 购买时部分商品可打折
    {
      attrDiscount = m_pUser->getAttr(EATTRTYPE_BUYDISCOUNT);
      if (attrDiscount > 1000.0f)
        attrDiscount = 1000.0f;
    }
    auto calcPrice = [&](DWORD moneycount) -> DWORD
    {
      double dOrigPrice = 1.0 * moneycount * pItem->discount() / 100.0 * origCount + 1.0 * moneycount * actDiscount / 100.0 * disCount;
      double dDiscount = floor(1.0 * dOrigPrice * attrDiscount / 1000.0);
      return static_cast<DWORD>(floor(dOrigPrice - dDiscount));
    };
    if (pItem->moneyid() != 0)
    {
      ItemInfo oItem;
      oItem.set_id(pItem->moneyid());
      oItem.set_count(calcPrice(pItem->moneycount()));

      // 在打折活动打折了的情况下, 校验前端计算的价格, 若不一致说明前端数据可能不是最新
      if (disCount > 0 && oItem.count() != dwPrice)
      {
        QueryShopConfigCmd ntf;
        ntf.set_type(pItem->shoptype());
        ntf.set_shopid(pItem->shopid());
        queryConfigByType(ntf);
        return false;
      }

      combinItemInfo(vecItems, TVecItemInfo{oItem});
    }
    if (pItem->moneyid2() != 0)
    {
      ItemInfo oItem;
      oItem.set_id(pItem->moneyid2());
      oItem.set_count(calcPrice(pItem->moneycount2()));

      // 在打折活动打折了的情况下, 校验前端计算的价格, 若不一致说明前端数据可能不是最新
      if (disCount > 0 && oItem.count() != dwPrice2)
      {
        QueryShopConfigCmd ntf;
        ntf.set_type(pItem->shoptype());
        ntf.set_shopid(pItem->shopid());
        queryConfigByType(ntf);
        return false;
      }

      combinItemInfo(vecItems, TVecItemInfo{oItem});
    }
    /*BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr || pMainPack->checkItemCount(vecItems) == false)// checkItemCount(oItem.id(), oItem.count()))
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个失败,需要";
      for (auto &v : vecItems)
        XERR << v.count() << "个" << v.id() << " ";
      XERR << XEND;
      return false;
    }
    for (auto &v : vecItems)
    {
      pMainPack->reduceItem(v.id(), ESOURCE_SHOP, v.count());

      if (v.id() == 100)
        m_pUser->getAchieve().onShopBuy(v.count());
    }*/
    Package& rPackage = m_pUser->getPackage();
    ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP;
    EPackFunc eFunc = EPACKFUNC_SHOP;
    if (rPackage.checkItemCount(vecItems, eMethod, eFunc) == false)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "购买" << dwShopID << dwCount << "个失败,需要" << vecItems << XEND;
      return false;
    }
    rPackage.reduceItem(vecItems, ESOURCE_SHOP, eMethod, eFunc);

    auto v = find_if(vecItems.begin(), vecItems.end(), [](const ItemInfo& rInfo) -> bool{
      return rInfo.id() == ITEM_ZENY;
    });
    if (v != vecItems.end())
      m_pUser->getAchieve().onShopBuy(v->count());

    // 打折活动增加打折次数
    if (disCount > 0)
      addDiscountActItemCount(pItem->id(), disCount);
  }

  DWORD dwTrueCount = dwCount;
  // add items
  if (pItem->itemid() != 0)
  {
    if (pCFG == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,itemid :" << pItem->itemid() << "失败,未在Table_Item.txt表中找到" << XEND;
      return false;
    }
    if (pCFG->eItemType == EITEMTYPE_HAIR || pCFG->eItemType == EITEMTYPE_HAIR_MALE || pCFG->eItemType == EITEMTYPE_HAIR_FEMALE)
    {
      m_pUser->getHairInfo().useHair(pItem->itemid());
      dwTrueCount = 1;
    }
    else if (pCFG->eItemType == EITEMTYPE_EYE_MALE || pCFG->eItemType == EITEMTYPE_EYE_FEMALE)
    {
      m_pUser->getEye().useEye(pItem->itemid());
      dwTrueCount = 1;
    }
    else
    {
      DWORD dwNum = pItem->num() == 0 ? 1 : pItem->num();
      bool display = ShopConfig::getMe().canDisplay(dwShopID);
      ItemInfo oItem;
      oItem.set_id(pItem->itemid());
      oItem.set_count(dwCount * dwNum);
      oItem.set_source(ESOURCE_SHOP);
      if (pCFG->eItemType == EITEMTYPE_CODE)    //礼包码破格入包
        m_pUser->getPackage().addItem(oItem, EPACKMETHOD_NOCHECK, display, true);
      else
        m_pUser->getPackage().addItem(oItem, EPACKMETHOD_AVAILABLE, display, true);
      m_pUser->getAchieve().onWedding(EACHIEVECOND_WEDDING_PURCHASE, TVecQWORD{oItem.id()});
      dwTrueCount = dwCount * dwNum;
    }

    DWORD dwLimitNum = MiscConfig::getMe().getGameShopCFG().getLimitNum(pItem->itemid());
    if(dwLimitNum > 0)
    {
      auto it = m_mapLimitItem.find(pItem->itemid());
      if(it != m_mapLimitItem.end())
        it->second += dwTrueCount;
      else
        m_mapLimitItem.emplace(pItem->itemid(), dwTrueCount);
      UpdateShopGotItem shopcmd;
      shopcmd.mutable_limititem()->set_id(pItem->itemid());
      shopcmd.mutable_limititem()->set_count(m_mapLimitItem[pItem->itemid()]);
      PROTOBUF(shopcmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      XLOG << "[场景商店-购物] 限购道具" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买商店" << dwShopID << "物品" << dwCount <<  "个,itemid:" << pItem->itemid() << "truecount:" << dwTrueCount;
    }
  }
  if (pItem->skillid() != 0)
  {
    m_pUser->addSkill(pItem->skillid(), 0, ESOURCE_SHOP);
  }
  else if (pItem->haircolorid() != 0)
  {
    m_pUser->getHairInfo().useColor(pItem->haircolorid());
  }
  else if (dwClothColorID != 0)
  {
    const SCloth* pCFG = TableManager::getMe().getClothCFG(dwClothColorID);
    if (pCFG == nullptr)
    {
      XERR << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "购买" << dwShopID << dwCount << "个,clothcolorid :" << dwClothColorID << "设置失败,未在 Table_Couture.txt 表中找到" << XEND;
    }
    else
    {
      m_pUser->getUserSceneData().setClothColor(pCFG->getClothID());
    }
  }

  if (pItem->shoptype() == EShopType_Drawing)
  {
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EXCHANGE_HEAD_DRAWING, dwTrueCount);
  }
  else if(pItem->shoptype() == EShopType_VENDING_MACHINE)
      m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_VENDING_MACHINE);

  if(dwShopID == 60000)
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_FRIENDSHIP_MORACOIN);
  else if(dwShopID == 242)
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_SPACE_BREAK_SKILL);

  // inform client
  BuyShopItem cmd;
  cmd.set_id(dwShopID);
  cmd.set_count(dwCount);
  cmd.set_success(true);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[场景商店-购物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "购买商店" << dwShopID << "物品" << dwCount <<  "个,itemid:" << pItem->itemid() << "truecount:" << dwTrueCount;
  if (bNeedZeny == false)
    XLOG << "优先扣除一个道具" << pItem->precost();
  else
    for (auto &v : vecItems)
      XLOG << "moneyid:" << v.id() << "cost:" << v.count();
  XLOG << XEND;
  
  //全服限购发送到增加数量
  if (pItem->producenum() && fromMatch == true)
  {
    //发送matchserver
    AddBuyCntMatchSCmd matchCmd;
    matchCmd.set_id(cmd.id());
    matchCmd.set_count(cmd.count());
    matchCmd.set_zoneid(thisServer->getZoneID());
    matchCmd.set_charid(m_pUser->id);
    PROTOBUF(matchCmd, send, len);
    thisServer->sendCmdToSession(send, len);
    XLOG << "[场景商店-购物], 全服商品，向matchserver 增加数量" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "购买" << dwShopID << dwCount << "个" << XEND;
    return true;
  }

  //检查触发出售新的商品
  if (checkShowNextItem(*pItem))
  {
    updateShopConfigByPreItem(*pItem);
  }

  return true;
}

void SceneShop::resetShopItemCount(bool notify)
{
  bool ntf = false;

  auto userF = [&](TVecShopGotItem& rVecGotItem, EVarType varType) {
    if (m_pUser->getVar().getVarValue(varType) == 0)
    {
      rVecGotItem.clear();
      m_pUser->getVar().setVarValue(varType, 1);
      ntf = true;
    }
  };

  auto accF = [&](TVecShopGotItem& rVecGotItem, EAccVarType varrType) {
    if (m_pUser->getVar().getAccVarValue(varrType) == 0)
    {
      rVecGotItem.clear();
      m_pUser->getVar().setAccVarValue(varrType, 1);
      ntf = true;
    }
  };

  userF(m_vecShopGotItem, EVARTYPE_SHOP);
  userF(m_vecShopGotItemWeek, EVARTYPE_SHOP_WEEK);
  userF(m_vecUserShopGotItemMonth, EVARTYPE_SHOP_MONTH);

  accF(m_vecAccShopGotItem, EACCVARTYPE_SHOP_GOT);
  accF(m_vecAccShopGotItemWeek, EACCVARTYPE_SHOP_GOT_WEEK);
  accF(m_vecAccShopGotItemMonth, EACCVARTYPE_SHOP_GOT_MONTH);
  
  if (notify && ntf)
    sendGotItem();
}

void SceneShop::addShopItemCount(DWORD id, DWORD count /*= 1*/)
{
  const ShopItem* pItem = ShopConfig::getMe().getShopItem(id);
  if (pItem == nullptr)
    return;
  auto func = [&](TVecShopGotItem& rVec)
  {
    auto v = find_if(rVec.begin(), rVec.end(), [id](const TPairShopGotItem& r) -> bool {
      return r.first == id;
    });
    if (v == rVec.end())
    {
      rVec.push_back(TPairShopGotItem(id, count));
      v = find_if(rVec.begin(), rVec.end(), [id](const TPairShopGotItem& r) -> bool {
        return r.first == id;
      });
    }
    else
      v->second += count;
    if (v == rVec.end())
      return ;    
    // update client
    UpdateShopGotItem shopcmd;
    shopcmd.mutable_item()->set_id(id);
    shopcmd.mutable_item()->set_count(v->second);
    PROTOBUF(shopcmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  };
 
  if ((pItem->limittype() & ESHOPLIMITTYPE_USER) != 0)
  {
    func(m_vecShopGotItem);
    
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_USER) != 0)
  {
    func(m_vecAccShopGotItem);
  }

  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_USER_ALWAYS) != 0)
  {
    func(m_vecAccShopGotItemAlways);
  }     
  if ((pItem->limittype() & ESHOPLIMITTYPE_USER_WEEK) != 0)
  {
    func(m_vecShopGotItemWeek);
  }

  if ((pItem->limittype() & ESHOPLIMITTYPE_USER_MONTH) != 0)
  {
    func(m_vecUserShopGotItemMonth);
  }

  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_WEEK) != 0)
  {
    func(m_vecAccShopGotItemWeek);
  }

  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_MONTH) != 0)
  {
    func(m_vecAccShopGotItemMonth);
  }

}

DWORD SceneShop::getShopItemCount(DWORD id)
{
  const ShopItem* pItem = ShopConfig::getMe().getShopItem(id);
  if (pItem == nullptr)
    return 0;

  auto func = [&](TVecShopGotItem& rVec)->DWORD
  {
    auto v = find_if(rVec.begin(), rVec.end(), [id](const TPairShopGotItem& r) -> bool {
      return r.first == id;
    });
    if (v != rVec.end())
      return v->second;
    return 0;
  };
  DWORD ret = 0;
  if ((pItem->limittype() & ESHOPLIMITTYPE_USER) != 0)
  {
    ret = func(m_vecShopGotItem);
  }

  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_USER) != 0)
  {
    ret = func(m_vecAccShopGotItem);
  }

  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_USER_ALWAYS) != 0)
  {
    ret = func(m_vecAccShopGotItemAlways);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_USER_WEEK) != 0)
  {
    ret = func(m_vecShopGotItemWeek);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_USER_MONTH) != 0)
  {
    ret = func(m_vecUserShopGotItemMonth);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_WEEK) != 0)
  {
    ret = func(m_vecAccShopGotItemWeek);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_MONTH) != 0)
  {
    ret = func(m_vecAccShopGotItemMonth);
  }
  return ret;
}

void SceneShop::setShopItemCount(DWORD id, DWORD count, bool ntf)
{
  const ShopItem* pItem = ShopConfig::getMe().getShopItem(id);
  if (pItem == nullptr)
    return;

  auto func = [&](TVecShopGotItem& rVec)
  {
    auto v = find_if(rVec.begin(), rVec.end(), [id](const TPairShopGotItem& r) -> bool {
      return r.first == id;
    });
    if (v == rVec.end())
    {
      if (count == 0)
        return;
      rVec.push_back(TPairShopGotItem(id, count));
      v = find_if(rVec.begin(), rVec.end(), [id](const TPairShopGotItem& r) -> bool {
        return r.first == id;
      });
    }
    else if (v->second == count)
      return;
    else
      v->second = count;
    if (v != rVec.end() && ntf)
    {
      // update client
      UpdateShopGotItem shopcmd;
      shopcmd.mutable_item()->set_id(id);
      shopcmd.mutable_item()->set_count(v->second);
      PROTOBUF(shopcmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
    XLOG << "[商店-设置商品购买次数]" << m_pUser->id << m_pUser->accid << m_pUser->name << "商品id:" << id << "count:" << count << "设置成功" << XEND;
  };

  if ((pItem->limittype() & ESHOPLIMITTYPE_USER) != 0)
  {
    func(m_vecShopGotItem);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_USER) != 0)
  {
    func(m_vecAccShopGotItem);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_USER_ALWAYS) != 0)
  {
    func(m_vecAccShopGotItemAlways);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_USER_WEEK) != 0)
  {
    func(m_vecShopGotItemWeek);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_USER_MONTH) != 0)
  {
    func(m_vecUserShopGotItemMonth);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_WEEK) != 0)
  {
    func(m_vecAccShopGotItemWeek);
  }
  if ((pItem->limittype() & ESHOPLIMITTYPE_ACC_MONTH) != 0)
  {
    func(m_vecAccShopGotItemMonth);
  }
}

void SceneShop::queryConfigByType(QueryShopConfigCmd &rev)
{
  bool bNeedSold = false;
  const TMapTypeShopItem& mapShopItem = ShopConfig::getMe().getAllShopItem();
  auto it = mapShopItem.find(rev.type());
  if (it == mapShopItem.end())
  {
    XERR << "[查询商店配置]" << m_pUser->accid << m_pUser->id << m_pUser->name << "type:" << rev.type() << "未找到配置" << XEND;
    return;
  }

  // 公会建筑商店
  const SGuildBuildingCFG* pGBuildingCFG = nullptr;
  EGuildBuilding building = GuildConfig::getMe().getGuildBuildingByShopType(rev.type());
  if (building != EGUILDBUILDING_MIN)
  {
    GuildScene* pGuildScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
    if (pGuildScene == nullptr)
    {
      XERR << "[查询商店配置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑:" << building << "不在公会场景" << XEND;
      return;
    }
    DWORD buildinglv = pGuildScene->getGuild().getBuildingLevel(building);
    if (buildinglv <= 0)
    {
      XERR << "[查询商店配置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑:" << building << "建筑未建造" << XEND;
      return;
    }
    pGBuildingCFG = GuildConfig::getMe().getGuildBuildingCFG(building, buildinglv);
    if (pGBuildingCFG == nullptr)
    {
      XERR << "[查询商店配置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑:" << building << "建筑等级:" << buildinglv << "建筑配置找不到" << XEND;
      return;
    }
    if (pGBuildingCFG->dwShopType != rev.type())
    {
      XERR << "[查询商店配置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "建筑:" << building << "建筑等级:" << buildinglv << "需要type:" << pGBuildingCFG->dwShopType << "请求type:" << rev.type() << "不一致" << XEND;
      return;
    }
  }

  resetShopItemCount(true);

  if (rev.type() == EShopType_RandomDay)
    randomShop();
  if (rev.type() == EShopType_RandomDayAcc)
    randomShopAcc();

  DWORD screen = 0;
  rev.clear_goods();
  ShopItem to;
  for (auto &v : it->second)
  {
    if (v.shopid() != rev.shopid())
      continue;

    if (packShopItem(v, to, screen, pGBuildingCFG) == false)
      continue;

    ShopItem* pItem = rev.add_goods();
    if (pItem == nullptr)
      continue;
    pItem->CopyFrom(to);   
    if (pItem->producenum())
    {
      bNeedSold = true;
    }
  }
  rev.set_screen(screen);
  PROTOBUF(rev, send, len);
  m_pUser->sendCmdToMe(send, len);

  if (bNeedSold)
  {
    QuerySoldCntMatchSCmd cmd;
    cmd.set_charid(m_pUser->id);
    cmd.set_zoneid(thisServer->getZoneID());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
  XLOG << "[查询商店配置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结果" << rev.ShortDebugString() << XEND;
}

void SceneShop::queryQuickBuy(QueryQuickBuyConfigCmd &rev)
{  
  resetShopItemCount(true);

  const ShopItem*pItem = nullptr;
  ShopItem to;
  DWORD screen = 0;
  for (int i = 0; i < rev.itemids_size(); i++)
  {
    pItem = ShopConfig::getMe().getShopItemByItemId(rev.itemids(i));
    if (pItem == nullptr)
      continue;

    if (pItem->shoptype() == EShopType_RandomDay)
      randomShop();

    if (packShopItem(*pItem, to, screen, nullptr) == false)
      continue;
    
    ShopItem* pGoods = rev.add_goods();
    if (pGoods == nullptr)
      continue;
    pGoods->CopyFrom(to);
  }
  PROTOBUF(rev, send, len);
  m_pUser->sendCmdToMe(send, len);
  
  XLOG <<"[商店-快速购买返回] charid" <<m_pUser->id <<"res:"<<rev.ShortDebugString() << XEND;
}

bool SceneShop::packShopItem(const ShopItem& from, ShopItem& to, DWORD& screen, const SGuildBuildingCFG* pGBuildingCFG)
{
  to.Clear();
  ShopItem* pItem = &to;
  pItem->CopyFrom(from);

  if (!canBuyItemNow(&from))
    return false;
  if (!checkShowShopItem(from))
    return false;
  DWORD maxNum = from.maxcount();
  if (from.shoptype() == EShopType_RandomDay)
  {
    auto it = m_mapShopRand.find(from.id());
    if (it == m_mapShopRand.end())
      return false;
    else
      maxNum = it->second;
  }
  else if (from.shoptype() == EShopType_RandomDayByLv)
  {
    maxNum = getRandShopItemMaxCount(from.id());
    if (maxNum <= 0)
      return false;
  }
  else if (from.shoptype() == EShopType_RandomDayByAccLv)
  {
    maxNum = getAccRandShopItemMaxCount(from.id());
    if (maxNum <= 0)
      return false;
  } 
  else if (from.shoptype() == EShopType_RandomWeek)
  {
    if (!ShopConfig::getMe().isInWeekRand(from.id()))
      return false;
  }
  else if (from.shoptype() == EShopType_RandomDayAcc)
  {
    auto it = m_mapShopRandAcc.find(from.id());
    if (it == m_mapShopRandAcc.end())
      return false;
    else
      maxNum = it->second;
  }
  else if (from.shoptype() == EShopType_Cloth)
  {
    if (from.clothcolorid() != 0)
    {
      const SCloth* pCFG = TableManager::getMe().getClothCFG(from.clothcolorid());
      if (pCFG == nullptr || pCFG->isOnSale() == false)
        return false;
      if (pCFG->getGender() != EGENDER_MIN && pCFG->getGender() != m_pUser->getUserSceneData().getGender())
        return false;
      if (pCFG->getProfession() != EPROFESSION_MIN && pCFG->getProfession() != m_pUser->getProfession())
        return false;
      pItem->set_lockarg(from.lockarg());
    }
    else
    {
      if (from.clothcolorids_size() != 2 || from.clothcolorids(0) > from.clothcolorids(1))
        return false;

      DWORD dwMin = from.clothcolorids(0);
      DWORD dwMax = from.clothcolorids(1);
      for (DWORD d = dwMin; d <= dwMax; ++d)
      {
        const SCloth* pCloth = TableManager::getMe().getClothCFG(d);
        if (pCloth == nullptr || pCloth->isOnSale() == false)
          continue;
        if (pCloth->getGender() != EGENDER_MIN && pCloth->getGender() != m_pUser->getUserSceneData().getGender())
          continue;
        if (pCloth->getProfession() != EPROFESSION_MIN && pCloth->getProfession() != m_pUser->getProfession())
          continue;
        pItem->set_clothcolorid(d);
        pItem->set_des(MsgParams::fmtString(from.des(), MsgParams(pCloth->getName())));
        XDBG << "[玩家-商店配置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "商店id :" << from.id() << "获得clothcolorid" << d << XEND;
        break;
      }
      if (pItem->clothcolorid() == 0)
        return false;
    }
  }

  if (maxNum)
  {
    auto it = m_mapAccAddLimitCnt.find(from.id());
    if (it != m_mapAccAddLimitCnt.end())
      maxNum += it->second;
  }

  if (pItem->screen() != 0) screen = pItem->screen();

  if (maxNum)
    pItem->set_maxcount(maxNum);

  // 商品未解锁原因
  //pItem->clear_lockarg();
  if (pItem->menuid() != 0 && m_pUser->getMenu().isOpen(pItem->menuid()) == false)
  {
    const SMenuCFG* pCfg = MenuConfig::getMe().getMenuCFG(pItem->menuid());
    if (pCfg != nullptr && pCfg->stCondition.eCond == EMENUCOND_QUEST && pCfg->stCondition.vecParams.empty() == false)
    {
      pItem->set_locktype(ESHOPLOCKTYPE_QUEST);
      const SQuestCFGEx* pQuestCfg = QuestManager::getMe().getQuestCFG(pCfg->stCondition.vecParams[0]);
      if (pQuestCfg != nullptr)
        pItem->set_lockarg(pQuestCfg->getName());
    }
    else
    {
      if (MiscConfig::getMe().getJoyLimitCFG().dwShopType == pItem->shoptype() && MiscConfig::getMe().getJoyLimitCFG().dwShopID == pItem->shopid())
        pItem->set_lockarg(from.lockarg());
    }
  }

  // 公会建筑解锁
  if (pGBuildingCFG)
  {
    if (pGBuildingCFG->setShopID.find(pItem->id()) == pGBuildingCFG->setShopID.end())
    {
      pItem->set_locktype(ESHOPLOCKTYPE_GUILDBUILDING);
      pItem->set_lockarg(from.lockarg());
    }
  }

  const SShopDiscountActivity* discountAct = ShopConfig::getMe().getDiscountActivity(pItem->id());
  if (discountAct != nullptr && discountAct->isActivityStart())
  {
    if (discountAct->isDiscount())
    {
      pItem->set_discountmax(discountAct->dwCount);
      pItem->set_actdiscount(discountAct->dwDiscount);
    }
  }

  DWORD order = ActivityEventManager::getMe().getShopItemOrder(pItem->id());
  if (order)
    pItem->set_shoporder(order);

  // 未使用数据不发送
  if (pItem->moneyid2() == 0) pItem->clear_moneyid2();
  if (pItem->moneycount2() == 0) pItem->clear_moneycount2();
  if (pItem->itemid() == 0) pItem->clear_itemid();
  if (pItem->skillid() == 0) pItem->clear_skillid();
  if (pItem->haircolorid() == 0) pItem->clear_haircolorid();
  if (pItem->clothcolorid() == 0) pItem->clear_clothcolorid();
  if (pItem->num() == 0) pItem->clear_num();
  if (pItem->business() == 0) pItem->clear_business();
  if (pItem->precost() == 0) pItem->clear_precost();
  if (pItem->ifmsg() == 0) pItem->clear_ifmsg();
  if (pItem->lv() == 0) pItem->clear_lv();
  if (pItem->profession_size() == 0) pItem->clear_profession();
  if (pItem->itemtype() == 0) pItem->clear_itemtype();
  pItem->clear_display();
  pItem->clear_starttime();
  pItem->clear_endtime();
  pItem->clear_screen();
  pItem->clear_shopid();
  pItem->clear_nextgoodsid();
  return true;
}


void SceneShop::randomShop()
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_SHOP_RANDOM) == 1)
    return;
  
  const TMapTypeShopItem& mapShopItem = ShopConfig::getMe().getAllShopItem();
  auto it = mapShopItem.find(EShopType_RandomDay);
  if (it == mapShopItem.end())
    return;
  
  const TVecShopItem& vecShopItem = it->second;
  if (vecShopItem.empty())
    return;
   
  m_mapShopRand.clear();  
  DWORD count = MiscConfig::getMe().getSystemCFG().dwShopRandomCnt;
  while (count--)
  {
    ShopItem* pItem = randomStlContainer(vecShopItem);
    if (!pItem)
      continue;
    
    DWORD& count = m_mapShopRand[pItem->id()];
    count += pItem->maxcount();    
  }
  m_pUser->getVar().setVarValue(EVARTYPE_SHOP_RANDOM, 1);

  XLOG << "[商店-随机] 每日随机，角色数据，accid" << m_pUser->accid << m_pUser->id << m_pUser->name << "随机条目" << m_mapShopRand.size() << XEND;
}

void SceneShop::randomShopAcc()
{
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_SHOP_RANDOM_ACC) == 1)
    return;

  const TMapTypeShopItem& mapShopItem = ShopConfig::getMe().getAllShopItem();
  auto it = mapShopItem.find(EShopType_RandomDayAcc);
  if (it == mapShopItem.end())
    return;

  const TVecShopItem& vecShopItem = it->second;
  if (vecShopItem.empty())
    return;

  m_mapShopRandAcc.clear();
  DWORD count = MiscConfig::getMe().getSystemCFG().dwShopRandomCnt;
  while (count--)
  {
    ShopItem* pItem = randomStlContainer(vecShopItem);
    if (!pItem)
      continue;

    DWORD& count = m_mapShopRandAcc[pItem->id()];
    count += pItem->maxcount();
  }
  m_pUser->getVar().setAccVarValue(EACCVARTYPE_SHOP_RANDOM_ACC, 1);
  XLOG << "[商店-随机] 每日随机，账号共享数据，accid" << m_pUser->accid << m_pUser->id << m_pUser->name << "随机条目" << m_mapShopRandAcc.size() << XEND;
}


DWORD SceneShop::getShopItemMaxCount(DWORD shopType, DWORD dwShopId)
{
  if (shopType == EShopType_RandomDay)
  {
    randomShop();
    auto it = m_mapShopRand.find(dwShopId);
    if (it == m_mapShopRand.end())
      return 0;
    return it->second;
  }
  else if (shopType == EShopType_RandomDayAcc)
  {
    randomShopAcc();
    auto it = m_mapShopRandAcc.find(dwShopId);
    if (it == m_mapShopRandAcc.end())
      return 0;
    return it->second;
  }
  return 0;
}

void SceneShop::sendGotItem()
{
  QueryShopGotItem cmd;
  
  for (auto& v : m_mapAccDiscountActItem)
  {
    ShopGotItem* pItem = cmd.add_discountitems();
    if (pItem == nullptr)
      continue;

    pItem->set_id(v.first);
    pItem->set_count(v.second);
  }

  for(auto& it : m_mapLimitItem)
  {
    DWORD dwLimit = MiscConfig::getMe().getGameShopCFG().getLimitNum(it.first);
    if(dwLimit == 0)
      continue;
    ShopGotItem* pItem = cmd.add_limititems();
    if(pItem == nullptr)
      continue;

    pItem->set_id(it.first);
    pItem->set_count(it.second);
  }

  auto func = [&](TVecShopGotItem& rVec)
  {
    for (auto v = rVec.begin(); v != rVec.end(); ++v)
    {
      ShopGotItem* pItem = cmd.add_items();
      if (pItem == nullptr)
        continue;

      pItem->set_id(v->first);
      pItem->set_count(v->second);
    }
  };

  func(m_vecShopGotItem);
  func(m_vecShopGotItemWeek);
  func(m_vecUserShopGotItemMonth);

  func(m_vecAccShopGotItem);
  func(m_vecAccShopGotItemWeek);
  func(m_vecAccShopGotItemMonth);
  func(m_vecAccShopGotItemAlways);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

DWORD SceneShop::getRefreshLv()
{
  updateRefreshLv(m_pUser->getLevel());
  return m_dwRefreshLv;
}

void SceneShop::updateRefreshLv(DWORD oldLv)
{
  DWORD cur = now();
  if (xTime::getDayStart(cur, 5 * 3600) < xTime::getDayStart(m_dwRefreshLvResetTime, 5 * 3600) + 86400)
    return;
  m_dwRefreshLv = oldLv;
  m_dwRefreshLvResetTime = cur;
}

void SceneShop::onLevelup(DWORD oldLv)
{
  updateRefreshLv(oldLv);
}

DWORD SceneShop::getRandShopItemMaxCount(DWORD id)
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_SHOP_RANDOM_BY_LV) == 0)
  {
    m_mapLvRandShopItem.clear();
    ShopConfig::getMe().getRandomShopItemsByLv(getRefreshLv(), m_mapLvRandShopItem);
    m_pUser->getVar().setVarValue(EVARTYPE_SHOP_RANDOM_BY_LV, 1);
  }

  auto it = m_mapLvRandShopItem.find(id);
  if (it == m_mapLvRandShopItem.end())
    return 0;
  return it->second;
}

DWORD SceneShop::getAccRandShopItemMaxCount(DWORD id)
{
  if (m_pUser->getVar().getAccVarValue(EVARTYPE_SHOP_RANDOM_BY_ACCLV) == 0)
  {
    m_mapAccLvRandShopItem.clear();
    ShopConfig::getMe().getRandomShopItemsByAccLv(m_pUser->getMaxBaseLv(), m_mapAccLvRandShopItem);
    m_pUser->getVar().setAccVarValue(EVARTYPE_SHOP_RANDOM_BY_ACCLV, 1);
  }

  auto it = m_mapAccLvRandShopItem.find(id);
  if (it == m_mapAccLvRandShopItem.end())
   return 0;
  return it->second;
}

DWORD SceneShop::getDiscountActItemCount(DWORD id)
{
  auto it = m_mapAccDiscountActItem.find(id);
  if (it == m_mapAccDiscountActItem.end())
    return 0;
  return it->second;
}

void SceneShop::addDiscountActItemCount(DWORD id, DWORD value)
{
  if (m_mapAccDiscountActItem.find(id) == m_mapAccDiscountActItem.end())
    m_mapAccDiscountActItem[id] = 0;
  m_mapAccDiscountActItem[id] += value;

  UpdateShopGotItem cmd;
  cmd.mutable_discountitem()->set_id(id);
  cmd.mutable_discountitem()->set_count(m_mapAccDiscountActItem[id]);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void SceneShop::addAccBuyLimitCnt(DWORD dwShopID, DWORD dwCount)
{
  auto it = m_mapAccAddLimitCnt.find(dwShopID);
  if (it != m_mapAccAddLimitCnt.end())
    it->second += dwCount;
  else
    m_mapAccAddLimitCnt[dwShopID] = dwCount;

  const ShopItem* pItem = ShopConfig::getMe().getShopItem(dwShopID);
  if (pItem)
  {
    ShopDataUpdateCmd cmd;
    cmd.set_type(pItem->shoptype());
    cmd.set_shopid(pItem->shopid());
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  XLOG << "[场景商店], 增加购买上限次数, 玩家:" << m_pUser->name << m_pUser->id << "账号:" << m_pUser->accid << "shop id:" << dwShopID << "数量:" << dwCount << "当前数量:" << m_mapAccAddLimitCnt[dwShopID] << XEND;
}

bool SceneShop::checkShowShopItem(const ShopItem& pItem)
{
  //1 检查前置商品都已经达到购买上限,这里在加载配置的时候已经确保没有环形配置，避免死循环
  DWORD dwShopID = pItem.id();
  DWORD dwPreShopID = ShopConfig::getMe().getPreGoodsID(dwShopID);
  DWORD dwCount = 0;
  while (dwPreShopID > 0)
  {
    const ShopItem* preItem = ShopConfig::getMe().getShopItem(dwPreShopID);
    if (preItem == nullptr)
    {
      break;
    }
    dwCount = getShopItemCount(dwPreShopID);
    if (dwCount < preItem->maxcount())
    {
      return false;
    }

    dwPreShopID = ShopConfig::getMe().getPreGoodsID(preItem->id());
  }

  // 更新需求: 有后置商品的原来的商品也要显示,前端显示售罄
  
  /*
  //2 当前达到购买上限 并且无后置商品
  dwCount = getShopItemCount(dwShopID);
  if (dwCount >= pItem.maxcount())
  {
    if (pItem.nextgoodsid() > 0)
    {
      return false;
    }
  }
  */

  return true;
}

bool SceneShop::checkShowNextItem(const ShopItem& pItem)
{
  DWORD dwShopID = pItem.id();
  //当前达到购买上限 并且有后置商品
  DWORD dwCount = getShopItemCount(dwShopID);
  if (dwCount >= pItem.maxcount())
  {
    if (pItem.nextgoodsid() > 0)
    {
      return true;
    }
  }

  return false;
}
void SceneShop::updateShopConfigByPreItem(const ShopItem& pItem)
{
  UpdateShopConfigCmd cmd;
  cmd.set_type(pItem.shoptype());
  cmd.set_shopid(pItem.shopid());
  // 更新需求: 有后置商品的原来的商品不删除,前端显示售罄
  //cmd.add_del_goods_id(pItem.id());
  DWORD dwNextGoodsID = pItem.nextgoodsid();
  const ShopItem* pNextItem = ShopConfig::getMe().getShopItem(dwNextGoodsID);
  if (pNextItem != nullptr)
  {
    ShopItem to;
    DWORD screen = 0;

    if (pNextItem->shoptype() == EShopType_RandomDay)
      randomShop();

    if (packShopItem(*pNextItem, to, screen, nullptr) != false)
    {
      ShopItem* pAddGoods = cmd.add_add_goods();
      if (pAddGoods != nullptr)
        pAddGoods->CopyFrom(to);
    }
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool SceneShop::canBuyItemNow(const ShopItem *pItem)
{
  return pItem && ShopConfig::getMe().canBuyItemNow(pItem) && ActivityEventManager::getMe().canBuyShopItem(pItem->id());
}
