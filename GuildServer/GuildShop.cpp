#include "GuildShop.h"
#include "ShopConfig.h"
#include "Guild.h"
#include "GuildServer.h"

GuildShopMgr::GuildShopMgr(Guild* guild) : m_pGuild(guild)
{
}

GuildShopMgr::~GuildShopMgr()
{
}

bool GuildShopMgr::buyItem(GMember* member, DWORD shopid, DWORD count)
{
  if (count <= 0)
    return false;
  if (member == nullptr)
    return false;

  if (m_pGuild->getMisc().hasAuth(member->getJob(), EAUTH_GUILD_SHOP) == false)
  {
    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3808);
    return false;
  }

  const ShopItem* item = ShopConfig::getMe().getShopItem(shopid);
  if (item == nullptr || item->itemid() == 0)
    return false;
  if (ShopConfig::getMe().canBuyItemNow(item) == false)
    return false;

  TVecItemInfo cost;
  DWORD moneyid = 0, moneycount = 0;
  if (item->moneyid() != 0)
  {
    ItemInfo money;
    money.set_id(item->moneyid());
    money.set_count(item->moneycount() * count);
    combinItemInfo(cost, TVecItemInfo{money});
    moneyid = item->moneyid();
    moneycount = money.count();
  }
  if (item->moneyid2() != 0)
  {
    ItemInfo money;
    money.set_id(item->moneyid2());
    money.set_count(item->moneycount2() * count);
    combinItemInfo(cost, TVecItemInfo{money});
    moneyid = item->moneyid2();
    moneycount = money.count();
  }

  if (m_pGuild->getPack().reduceItem(cost, ESOURCE_GUILD_SHOP) != ESYSTEMMSG_ID_MIN)
    return false;

  ItemInfo goods;
  goods.set_id(item->itemid());
  goods.set_count(count);
  goods.set_source(ESOURCE_GUILD_SHOP);
  if (m_pGuild->getPack().addItem(goods) != ESYSTEMMSG_ID_MIN)
  {
    XERR << "[公会商店-购买]" << m_pGuild->getGUID() << m_pGuild->getName() << "玩家:" << member->getCharID() << "shopid:" << shopid << "itemid:" << item->itemid() << "count:" << count << "添加道具失败" << XEND;
    return false;
  }

  m_pGuild->getPack().update(true);

  XLOG << "[公会商店-购买]" << m_pGuild->getGUID() << m_pGuild->getName() << "玩家:" << member->getCharID()  << "shopid:" << shopid << "itemid:" << item->itemid() << "count:" << count << "购买成功" << XEND;

  if (moneyid)
  {
    MsgParams params;
    params.addNumber(moneyid);
    params.addNumber(moneycount);
    params.addNumber(item->itemid());
    params.addNumber(count);
    thisServer->sendMsg(member->getZoneID(), member->getCharID(), 3809, params);
  }

  BuyShopItem cmd;
  cmd.set_id(shopid);
  cmd.set_count(count);
  cmd.set_success(true);
  PROTOBUF(cmd, send, len);
  member->sendCmdToMe(send, len);
  return true;
}
