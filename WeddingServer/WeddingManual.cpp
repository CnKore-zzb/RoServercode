#include "WeddingManual.h"
#include "Wedding.h"
#include "WeddingConfig.h"
#include "WeddingServer.h"
#include "MiscConfig.h"
#include "WeddingUserMgr.h"
#include "GCharManager.h"
#include "MailManager.h"
#include "ShopConfig.h"
#include "ItemConfig.h"

WeddingManual::WeddingManual(Wedding* wedding) : m_pWedding(wedding)
{
}

bool WeddingManual::fromData(const WeddingManualInfo& data)
{
  for (int i = 0; i < data.serviceids_size(); ++i)
    m_setServiceID.insert(data.serviceids(i));
  m_dwRingID = data.ringid();
  m_dwPhotoIndex1 = data.photoindex1();
  m_dwPhotoIndex2 = data.photoindex2();
  m_dwPhotoTime1 = data.phototime1();
  m_dwPhotoTime2 = data.phototime2();
  for (int i = 0; i < data.invitees_size(); ++i)
    m_mapInvitee[data.invitees(i).charid()].fromData(data.invitees(i));
  m_strName1 = data.name1();
  m_strName2 = data.name2();
  for (int i = 0; i < data.itemrecords_size(); ++i)
    m_vecItemRecord.push_back(data.itemrecords(i));
  return true;
}

bool WeddingManual::toData(WeddingManualInfo* data, bool invitee/* = true*/)
{
  if (data == nullptr)
    return false;
  for (auto id : m_setServiceID)
    data->add_serviceids(id);
  data->set_ringid(m_dwRingID);
  data->set_photoindex1(m_dwPhotoIndex1);
  data->set_photoindex2(m_dwPhotoIndex2);
  data->set_phototime1(m_dwPhotoTime1);
  data->set_phototime2(m_dwPhotoTime2);
  if (invitee)
  {
    for (auto& v : m_mapInvitee)
      v.second.toData(data->add_invitees());
  }
  data->set_name1(m_strName1);
  data->set_name2(m_strName2);
  for (auto& v : m_vecItemRecord)
  {
    ItemData* p = data->add_itemrecords();
    if (p)
      p->CopyFrom(v);
  }
  return true;
}

bool WeddingManual::fromString(const string& data)
{
  WeddingManualInfo manual;
  if (manual.ParseFromString(data) == false)
    return false;
  return fromData(manual);
}

bool WeddingManual::toString(string& data)
{
  WeddingManualInfo manual;
  if (toData(&manual) == false)
    return false;
  return manual.SerializeToString(&data);
}

bool WeddingManual::toClientWeddingManual(ClientWeddingManual* data, DWORD who)
{
  for (auto& id : m_setServiceID)
    data->add_packageids(id);
  data->set_ringid(m_dwRingID);
  if (who == 1)
  {
    data->set_photoindex(m_dwPhotoIndex1);
    data->set_phototime(m_dwPhotoTime1);
  }
  else
  {
    data->set_photoindex(m_dwPhotoIndex2);
    data->set_phototime(m_dwPhotoTime2);
  }
  return true;
}

void WeddingManual::init(bool toppackage)
{
  addFreePackage(false);
  if (toppackage)
    buyService(MiscConfig::getMe().getWeddingMiscCFG().dwTopPackageID, false);

  // 设置默认戒指
  buyRing(MiscConfig::getMe().getWeddingMiscCFG().dwDefaultRingID, false);

  m_strName1 = m_pWedding->getName(m_pWedding->getCharId1());
  m_strName2 = m_pWedding->getName(m_pWedding->getCharId2());
}

// 自动添加免费的套餐
void WeddingManual::addFreePackage(bool ntf)
{
  const TSetDWORD& ids = WeddingConfig::getMe().getFreeWeddingPackage();
  for (auto id : ids)
  {
    if (buyService(id, false))
      XLOG << "[结婚手册-自动添加免费套餐]" << "婚礼:" << m_pWedding->getId() << "套餐:" << id << "添加成功" << XEND;
  }
}

bool WeddingManual::buyService(DWORD id, bool ntf)
{
  const SWeddingServiceCFG* cfg = WeddingConfig::getMe().getWeddingServiceCFG(id);
  if (cfg == nullptr)
  {
    XERR << "[结婚手册-购买服务]" << "婚礼:" << m_pWedding->getId() << "服务:" << id << "配置找不到" << XEND;
    return false;
  }
  switch (cfg->eType)
  {
  case EWEDDINGSERVICE_PACKAGE:
  {
    if (m_setServiceID.find(id) != m_setServiceID.end())
      return false;
    m_setServiceID.insert(id);
    // 自动购买服务已包含于此套装中的其他套餐
    for (auto subid : cfg->setSubPackageID)
    {
      if (m_setServiceID.find(subid) == m_setServiceID.end())
      {
        m_setServiceID.insert(subid);
        XLOG << "[结婚手册-购买服务]" << "婚礼:" << m_pWedding->getId() << "套餐:" << subid << "自动购买包含套餐成功" << XEND;
      }
    }
    break;
  }
  default:
    return false;
  }

  m_pWedding->setUpdate(EWEDDINGDATA_MANUAL);

  if (ntf)
    notifyManual();
  return true;
}

bool WeddingManual::buyRing(DWORD id, bool ntf)
{
  if (m_dwRingID == id)
    return false;
  m_dwRingID = id;

  m_pWedding->setUpdate(EWEDDINGDATA_MANUAL);

  if (ntf)
    notifyManual();
  return true;
}

bool WeddingManual::buyPackage(QWORD charid, const BuyWeddingPackageCCmd& cmd)
{
  if (isBuyServiceLock())
    return false;
  if (m_pWedding->getStatus() != EWeddingStatus_Reserve)
    return false;
  if (m_setServiceID.find(cmd.id()) != m_setServiceID.end())
    return false;
  WeddingUser* user = WeddingUserMgr::getMe().getWeddingUser(charid);
  if (!user)
    return false;
  const SWeddingServiceCFG* cfg = WeddingConfig::getMe().getWeddingServiceCFG(cmd.id());
  if (cfg == nullptr || cfg->eType != EWEDDINGSERVICE_PACKAGE)
    return false;

  DWORD price = 0;
  if (WeddingConfig::getMe().getPackagePrice(price, cmd.priceitem(), cmd.id(), m_setServiceID) == false)
    return false;

  BuyServiceWeddingSCmd scmd;
  scmd.set_weddingid(m_pWedding->getId());
  scmd.set_charid(user->m_qwCharId);
  scmd.set_source(ESOURCE_WEDDING_BUY_PACKAGE);
  scmd.set_serviceid(cfg->dwID);
  ItemInfo* item = scmd.add_items();
  if (item)
  {
    item->set_id(cmd.priceitem());
    item->set_count(price);
  }
  else
  {
    return false;
  }
  PROTOBUF(scmd, send, len);
  if (user->sendCmdToScene(send, len) == false)
    return false;
  lockBuyService();

  XLOG << "[结婚手册-购买套餐]" << user->m_qwCharId << "套餐:" << cfg->dwID << "成功发送到SceneServer处理" << XEND;
  return true;
}

bool WeddingManual::buyRing(QWORD charid, const BuyWeddingRingCCmd& cmd)
{
  if (isBuyServiceLock())
    return false;
  if (m_pWedding->getStatus() != EWeddingStatus_Reserve)
    return false;
  WeddingUser* user = WeddingUserMgr::getMe().getWeddingUser(charid);
  if (!user)
    return false;
  // Warning!!!: 前端需复用商店ui, 估戒指配置放在shop表中
  const ShopItem* cfg = ShopConfig::getMe().getShopItem(cmd.id());
  if (cfg == nullptr || cfg->itemid() == 0 || cfg->moneyid() == 0 || cfg->moneycount() <= 0 || m_dwRingID == cfg->itemid() || cmd.priceitem() != cfg->moneyid() ||
      cfg->shoptype() != MiscConfig::getMe().getWeddingMiscCFG().dwRingShopType || cfg->shopid() != MiscConfig::getMe().getWeddingMiscCFG().dwRingShopID)
    return false;

  BuyServiceWeddingSCmd scmd;
  scmd.set_weddingid(m_pWedding->getId());
  scmd.set_charid(user->m_qwCharId);
  scmd.set_source(ESOURCE_WEDDING_BUY_RING);
  scmd.set_serviceid(cfg->itemid());
  ItemInfo* item = scmd.add_items();
  if (!item)
    return false;
  item->set_id(cmd.priceitem());
  item->set_count(cfg->moneycount());

  PROTOBUF(scmd, send, len);
  if (user->sendCmdToScene(send, len) == false)
    return false;
  lockBuyService();

  XLOG << "[结婚手册-购买戒指]" << user->m_qwCharId << "戒指:" << cfg->itemid() << "费用:" << cmd.priceitem() << cfg->moneycount() << "成功发送到SceneServer处理" << XEND;
  return true;
}

bool WeddingManual::handleBuyServiceWeddingSCmd(const BuyServiceWeddingSCmd& cmd)
{
  unlockBuyService();

  if (!cmd.success() || (cmd.charid() != m_pWedding->getCharId1() && cmd.charid() != m_pWedding->getCharId2()) || cmd.serviceid() <= 0)
  {
    XERR << "[结婚手册-购买服务]" << "玩家:" << cmd.charid() << "婚礼:" << cmd.weddingid() << "服务:" << cmd.serviceid() << "cmd:" << cmd.ShortDebugString() << "SceneServer处理失败" << XEND;
    return false;
  }

  if (cmd.source() == ESOURCE_WEDDING_BUY_PACKAGE && buyService(cmd.serviceid(), true) == false)
  {
    XERR << "[结婚手册-购买服务]" << "玩家:" << cmd.charid() << "婚礼:" << cmd.weddingid() << "服务:" << cmd.serviceid() << "cmd:" << cmd.ShortDebugString() << "购买失败" << XEND;
    return false;
  }
  else
  {
    WeddingUser* pUser = WeddingUserMgr::getMe().getWeddingUser(cmd.charid());
    if (pUser)
    {
      const SWeddingServiceCFG* cfg = WeddingConfig::getMe().getWeddingServiceCFG(cmd.serviceid());
      if (cfg)
        pUser->sendMsg(9629, MsgParams(cfg->strName));
    }
    XLOG << "[结婚手册-购买服务]" << "玩家:" << cmd.charid() << "婚礼:" << cmd.weddingid() << "服务:" << cmd.serviceid() << "cmd:" << cmd.ShortDebugString() << "购买成功" << XEND;
  }

  if (cmd.source() == ESOURCE_WEDDING_BUY_RING && buyRing(cmd.serviceid(), true) == false)
  {
    XERR << "[结婚手册-购买服务]" << "玩家:" << cmd.charid() << "婚礼:" << cmd.weddingid() << "戒指:" << cmd.serviceid() << "cmd:" << cmd.ShortDebugString() << "购买失败" << XEND;
    return false;
  }
  else
  {
    WeddingUser* pUser = WeddingUserMgr::getMe().getWeddingUser(cmd.charid());
    if (pUser)
    {
      const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(cmd.serviceid());
      if (cfg)
        pUser->sendMsg(9629, MsgParams(cfg->strNameZh));
    }
    XLOG << "[结婚手册-购买服务]" << "玩家:" << cmd.charid() << "婚礼:" << cmd.weddingid() << "服务:" << cmd.serviceid() << "cmd:" << cmd.ShortDebugString() << "购买成功" << XEND;
  }

  return true;
}

bool WeddingManual::invite(QWORD operatorid, const TSetQWORD& charids)
{
  if (m_pWedding->getStatus() != EWeddingStatus_Reserve)
    return false;
  if (m_setServiceID.size() + charids.size() > MiscConfig::getMe().getWeddingMiscCFG().dwInviteMaxCount)
    return false;

  TVecItemData items(1);
  items[0].mutable_base()->set_id(MiscConfig::getMe().getWeddingMiscCFG().dwInvitationItemID);
  items[0].mutable_base()->set_count(1);
  items[0].mutable_wedding()->set_id(m_pWedding->getId());
  items[0].mutable_wedding()->set_zoneid(m_pWedding->getZoneId());
  items[0].mutable_wedding()->set_charid1(m_pWedding->getCharId1());
  items[0].mutable_wedding()->set_charid2(m_pWedding->getCharId2());
  items[0].mutable_wedding()->set_myname(m_pWedding->getName(operatorid));
  items[0].mutable_wedding()->set_partnername(m_pWedding->getName(m_pWedding->getCharId1() == operatorid ? m_pWedding->getCharId2() : m_pWedding->getCharId1()));
  items[0].mutable_wedding()->set_starttime(m_pWedding->getStartTime());
  items[0].mutable_wedding()->set_endtime(m_pWedding->getEndTime());

  MsgParams param(items[0].wedding().myname(), items[0].wedding().partnername(), thisServer->getZoneName(items[0].wedding().zoneid()));
  param.addString(getWeddingTimeString(m_pWedding->getStartTime(), m_pWedding->getEndTime()));

  TSetQWORD updates;
  for (auto charid : charids)
  {
    if (charid == m_pWedding->getCharId1() || charid == m_pWedding->getCharId2() || getInvitee(charid))
      continue;
    GCharReader gchar(thisServer->getRegionID(), charid);
    if (gchar.getBySocial() == false)
      continue;

    SWeddingInvitee& invitee = m_mapInvitee[charid];
    invitee.qwCharID = charid;
    invitee.strName = gchar.getName();
    invitee.dwInviteTime = now();
    updates.insert(charid);

    // 发送包含邀请函的邮件
    MailManager::getMe().sendMail(charid, MiscConfig::getMe().getWeddingMiscCFG().dwInvitationMailID, items, param, false, true, EMAILTYPE_WEDDINGINVITATION);

    XLOG << "[结婚手册-邀请]" << operatorid << "被邀人:" << charid << "道具:" << items[0].ShortDebugString() << "邀请成功" << XEND;
  }

  if (!updates.empty())
  {
    m_pWedding->setUpdate(EWEDDINGDATA_MANUAL);
    notifyInvitee(updates);
  }

  return true;
}

bool WeddingManual::uploadPhoto(QWORD charid, DWORD index, DWORD time)
{
  bool ntf = false;
  if (m_pWedding->getCharId1() == charid)
  {
    if (m_dwPhotoIndex1 != index)
    {
      m_dwPhotoIndex1 = index;
      m_dwPhotoTime1 = time;
      ntf = true;
    }
  }
  else if (m_pWedding->getCharId2() == charid)
  {
    if (m_dwPhotoIndex2 != index)
    {
      m_dwPhotoIndex2 = index;
      m_dwPhotoTime2 = time;
      ntf = true;
    }
  }
  else
  {
    return false;
  }

  m_pWedding->setUpdate(EWEDDINGDATA_MANUAL);

  if (!ntf)
    return true;

  notifyManual();
  XLOG << "[结婚手册-上传结婚证照片]" << charid << "index:" << index << "上传成功" << XEND;
  return true;
}

SWeddingInvitee* WeddingManual::getInvitee(QWORD charid)
{
  auto it = m_mapInvitee.find(charid);
  if (it == m_mapInvitee.end())
    return nullptr;
  return &it->second;
}

void WeddingManual::handleUpdateWeddingManualCCmd(QWORD charid)
{
  DWORD who;
  if (m_pWedding->getCharId1() == charid)
  {
    m_bNtf1 = true;
    who = 1;
  }
  else if (m_pWedding->getCharId2() == charid)
  {
    m_bNtf2 = true;
    who = 2;
  }
  else
  {
    return;
  }

  UpdateWeddingManualCCmd cmd;
  toClientWeddingManual(cmd.mutable_manual(), who);
  for (auto& v : m_mapInvitee)
    v.second.toData(cmd.add_invitees());
  PROTOBUF(cmd, send, len);
  WeddingUserMgr::getMe().sendCmdToUser(charid, send, len);
}

void WeddingManual::notifyManual()
{
  // server
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_pWedding->getCharId1());
  WeddingUserMgr::getMe().syncWeddingInfo2Scene(m_pWedding->getCharId2());

  if (m_pWedding->getWeddingState() == ESyncState_StartWedding)
  {
    UpdateWeddingManualSCmd upcmd;
    upcmd.set_weddingid(m_pWedding->getId());
    toData(upcmd.mutable_manual());
    PROTOBUF(upcmd, upsend, uplen);
    thisServer->sendCmdToZone(m_pWedding->getZoneId(), upsend, uplen);
  }
  // client
  if (m_bNtf1 || m_bNtf2)
  {
    if (m_bNtf1)
    {
      UpdateWeddingManualCCmd cmd;
      toClientWeddingManual(cmd.mutable_manual(), 1);
      PROTOBUF(cmd, send, len);
      WeddingUserMgr::getMe().sendCmdToUser(m_pWedding->getCharId1(), send, len);
    }
    if (m_bNtf2)
    {
      UpdateWeddingManualCCmd cmd;
      toClientWeddingManual(cmd.mutable_manual(), 2);
      PROTOBUF(cmd, send, len);
      WeddingUserMgr::getMe().sendCmdToUser(m_pWedding->getCharId2(), send, len);
    }
  }
}

void WeddingManual::notifyInvitee(const set<QWORD>& charids)
{
  if (!m_bNtf1 && !m_bNtf2)
    return;
  if (charids.empty())
    return;

  UpdateWeddingManualCCmd cmd;
  for (auto id : charids)
  {
    SWeddingInvitee* v = getInvitee(id);
    if (v)
      v->toData(cmd.add_invitees());
  }
  if (!cmd.invitees_size())
    return;

  if (m_bNtf1)
  {
    toClientWeddingManual(cmd.mutable_manual(), 1);
    PROTOBUF(cmd, send, len);
    WeddingUserMgr::getMe().sendCmdToUser(m_pWedding->getCharId1(), send, len);
  }
  if (m_bNtf2)
  {
    toClientWeddingManual(cmd.mutable_manual(), 2);
    PROTOBUF(cmd, send, len);
    WeddingUserMgr::getMe().sendCmdToUser(m_pWedding->getCharId2(), send, len);
  }
}

void WeddingManual::onUserOffline(QWORD charid)
{
  if (m_pWedding->getCharId1() == charid)
    m_bNtf1 = false;
  else if (m_pWedding->getCharId2() == charid)
    m_bNtf2 = false;
}

//通知删除邀请函
void WeddingManual::delInvitation()
{
  // 需求,邀请函无需立即删除
  return;
  WeddingEventMsgCCmd eventMsg;
  eventMsg.set_event(EWeddingEvent_DelInvitation);
  eventMsg.set_id(m_pWedding->getId());
  for (auto& m : m_mapInvitee)
  {
    eventMsg.set_charid(m.second.qwCharID);
    MailManager::getMe().sendWeddingEventMsgMail(m.second.qwCharID, eventMsg);
  }
}

void WeddingManual::onWeddingStart()
{
  for (auto& v : m_mapInvitee)
  {
    WeddingUser* user = WeddingUserMgr::getMe().getWeddingUser(v.second.qwCharID);
    if (user)
      user->sendMsg(9639);
  }
}

void WeddingManual::saveWeddingItem(const TVecItemData& items)
{
  if (items.empty())
    return;
  m_vecItemRecord.insert(m_vecItemRecord.end(), items.begin(), items.end());
  m_pWedding->setUpdate(EWEDDINGDATA_MANUAL);

  XLOG << "[结婚手册-保存婚礼道具]" << m_pWedding->getId() << m_pWedding->getCharId1() << m_pWedding->getCharId2() << "道具:";
  for (auto& v : items)
    XLOG << v.ShortDebugString();
  XLOG << "保存成功" << XEND;
}
