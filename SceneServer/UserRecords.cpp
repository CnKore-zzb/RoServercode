#include "UserRecords.h"
#include "SceneUser.h"
#include "PetWork.h"
#include "xTime.h"
#include "Menu.h"

UserRecords::UserRecords(SceneUser* pUser) : m_pUser(pUser)
{
  m_dwCurBuyTimes = 0;
  m_dwLastLoadTime = 0;
  m_dwDestSlotID = 0;
  m_dwDestCharID = 0;
  m_dwDestMapID = 0;
  m_dwCardExpireTime = 0;
}

UserRecords::~UserRecords()
{
}

bool UserRecords::loadAcc(const BlobRecordInfo& rData)
{
  m_mapType2Record.clear();
  m_mapType2AstrolMaterial.clear();

  m_dwCurBuyTimes = rData.buytimes();
  m_dwLastLoadTime = rData.last_load_time();
  m_dwDestSlotID = rData.dest_slotid();
  m_dwDestCharID = rData.dest_charid();
  m_dwDestMapID = rData.dest_map();
  m_dwCardExpireTime = rData.card_expiretime();

  if (m_dwDestCharID != m_pUser->id)
  {
    clearMultiRoleData();
  }

  for (int i = 0; i < rData.records_size(); ++i)
  {
    ProfessionData& record = m_mapType2Record[rData.records(i).id()];
    record.CopyFrom(rData.records(i));
  }

  for (int i = 0; i < rData.astrol_data_size(); ++i)
  {
    UserAstrolMaterialData& astrol = m_mapType2AstrolMaterial[rData.astrol_data(i).charid()];
    astrol.CopyFrom(rData.astrol_data(i));
  }

  initSlotInfo();

  return true;
}

bool UserRecords::saveAcc(BlobRecordInfo* pBlob)
{
  if (m_dwDestCharID == m_pUser->id)
  {
    clearMultiRoleData();
  }
  pBlob->set_buytimes(m_dwCurBuyTimes);
  pBlob->set_last_load_time(m_dwLastLoadTime);
  pBlob->set_dest_slotid(m_dwDestSlotID);
  pBlob->set_dest_charid(m_dwDestCharID);
  pBlob->set_dest_map(m_dwDestMapID);
  pBlob->set_card_expiretime(m_dwCardExpireTime);

  for (auto& it : m_mapType2Record)
  {
    ProfessionData* pRecordInfo = pBlob->add_records();
    if (pRecordInfo == nullptr)
    {
      XERR << "[存档-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "存档数据:" << it.first << "record info protobuf error" << XEND;
      continue;
    }
    pRecordInfo->CopyFrom(it.second);
  }

  for (auto& it : m_mapType2AstrolMaterial)
  {
    if (it.first == m_pUser->id)
      continue;

    UserAstrolMaterialData* pAstrol = pBlob->add_astrol_data();
    if (pAstrol == nullptr)
    {
      XERR << "[存档-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "存档数据:" << it.first << "astrol data protobuf error" << XEND;
      continue;
    }
    pAstrol->CopyFrom(it.second);
  }

  UserAstrolMaterialData* pAstrol = pBlob->add_astrol_data();
  if (pAstrol != nullptr) 
  {
    collectTotalAstrolMaterialData(pAstrol);
  }

  return true;
}

void UserRecords::initSlotInfo()
{
  if (m_dwCardExpireTime == 0)
  {
    //以宠物打工的月卡时间初始化
    m_dwCardExpireTime = m_pUser->getPetWork().getCardExpireTime();
  }
  m_mapType2Slot.clear();

  const SRecordCFG& stRecordCFG = MiscConfig::getMe().getMiscRecordCFG();
  DWORD dwGridID = 1;
  //初始化免费格子
  for (DWORD i = 0; i < stRecordCFG.dwDefaultSlotNum; ++i)
  {
    SlotInfo& slot = m_mapType2Slot[dwGridID];
    slot.set_id(dwGridID);
    slot.set_type(ESLOT_DEFAULT);
    slot.set_active(true);
    dwGridID ++;
  }

  //初始化月卡开启的格子
  for (DWORD i = 0; i < stRecordCFG.dwMonthCardSlotNum; ++i)
  {
    SlotInfo& slot = m_mapType2Slot[dwGridID];
    slot.set_id(dwGridID);
    slot.set_type(ESLOT_MONTH_CARD);
    if (isInMonthCard())
    {
      slot.set_active(true);
    }
    else
    {
      slot.set_active(false);
    }
    dwGridID ++;
  }

  //初始化需要花钱购买的格子
  for (DWORD i = 0; i < stRecordCFG.dwTotalBuyNum; ++i)
  {
    SlotInfo& slot = m_mapType2Slot[dwGridID];
    slot.set_id(dwGridID);
    slot.set_type(ESLOT_BUY);
    if (i < m_dwCurBuyTimes)
    {
      slot.set_active(true);
    }
    else
    {
      slot.set_active(false);
      auto it = stRecordCFG.mapBuyTimes2CostIDAndNum.find(i + 1);
      if (it == stRecordCFG.mapBuyTimes2CostIDAndNum.end())
      {
        //找不到对应的价格 终止循环
        break;
      }
      slot.set_costid(it->second.first);
      slot.set_costnum(it->second.second);
    }
    dwGridID ++;
  }
}

bool UserRecords::isInMonthCard() const
{
  return xTime::getCurSec() < m_dwCardExpireTime;
}

bool UserRecords::isInValidMap() const
{
  return m_pUser->checkMapForChangeProfession();
}

bool UserRecords::isTwoExchangePro() const
{
  return m_pUser->getEvo() >= 2;
}

bool UserRecords::isInLoadCD() const
{
  DWORD dwCurTime = xTime::getCurSec();
  if (dwCurTime > m_dwLastLoadTime)
  {
    return false;
  }
  return true;
}

void UserRecords::refreshSlotInfo()
{
  initSlotInfo();

  UpdateRecordInfoUserCmd cmd;

  for (auto& v : m_mapType2Slot)
  {
    SlotInfo* pSlot = cmd.add_slots();
    if (pSlot == nullptr)
      continue;
    pSlot->CopyFrom(v.second);
  }

  cmd.set_card_expiretime(m_dwCardExpireTime);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool UserRecords::checkSlotStatusOn(DWORD dwSlotID)
{
  auto it = m_mapType2Slot.find(dwSlotID);
  if (it == m_mapType2Slot.end())
  {
    return false;
  }

  if (it->second.type() == ESLOT_MONTH_CARD)
  {
    if (isInMonthCard())
    {
      it->second.set_active(true);
    }
    else
    {
      it->second.set_active(false);
    }
  }

  return it->second.active();
}

bool UserRecords::checkHasRecord(DWORD dwSlotID)
{
  auto it = m_mapType2Record.find(dwSlotID);
  if (it == m_mapType2Record.end())
  {
    return false;
  }

  return true;
}

bool UserRecords::checkSlotCanBuy(DWORD dwSlotID)
{
  for(DWORD i = 1; i < dwSlotID; i++)
  {
    auto it = m_mapType2Slot.find(i);
    if (it == m_mapType2Slot.end())
      return false;
    if (it->second.type() == ESLOT_BUY && !it->second.active())
      return false;
  }

  auto it = m_mapType2Slot.find(dwSlotID);
  if (it == m_mapType2Slot.end())
    return false;
  if (it->second.type() != ESLOT_BUY)
    return false;
  if (it->second.active())
    return false;
  return true;
}

bool UserRecords::checkRecordName(std::string strRecordName, std::string strOldName /*= ""*/)
{
  if (strRecordName == strOldName)
  {
    return false;
  }

  ESysMsgID ret = MiscConfig::getMe().getSystemCFG().checkNameValid(strRecordName, ENAMETYPE_RECORD);
  if (ret == ESYSTEMMSG_ID_GUILD_NAMELEN)
  {
    MsgManager::sendMsg(m_pUser->id, 25409);
  }
  else if(ret != ESYSTEMMSG_ID_MIN)
  {
    MsgManager::sendMsg(m_pUser->id, 2604);
  }
  return ret == ESYSTEMMSG_ID_MIN;
}

bool UserRecords::checkHasMenuOpen()
{
  return m_pUser->getMenu().isOpen(MiscConfig::getMe().getMiscRecordCFG().dwMenuID);
}

SlotInfo* UserRecords::getSlotInfo(DWORD dwSlotID)
{
  auto it = m_mapType2Slot.find(dwSlotID);
  if(it == m_mapType2Slot.end())
    return nullptr;
  return &(it->second);
}

ProfessionData* UserRecords::getRecordUserInfo(DWORD dwSlotID)
{
  auto it = m_mapType2Record.find(dwSlotID);
  if(it == m_mapType2Record.end())
    return nullptr;
  return &(it->second);
}

void UserRecords::sendRecordsData()
{
  //由于数据包大小限制，决定分开发送存档信息
  UpdateRecordInfoUserCmd cmd;
  for (auto& v : m_mapType2Slot)
  {
    SlotInfo* pSlot = cmd.add_slots();
    if (pSlot == nullptr)
      continue;
    pSlot->CopyFrom(v.second);
  }

  cmd.set_card_expiretime(m_dwCardExpireTime);

  for (auto& it : m_mapType2AstrolMaterial)
  {
    if (it.first == m_pUser->id)
      continue;

    UserAstrolMaterialData* pAstrol = cmd.add_astrol_data();
    if (pAstrol == nullptr)
      continue;
    pAstrol->CopyFrom(it.second);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  //单独发送存档信息
  UpdateRecordInfoUserCmd record_cmd;
  for (auto& v : m_mapType2Record)
  {
    record_cmd.clear_records();
    ProfessionUserInfo* pRecord = record_cmd.add_records();
    if (pRecord == nullptr)
      continue;
    m_pUser->m_oProfession.collectProfessionUserInfo(v.second, pRecord);
    record_cmd.set_card_expiretime(m_dwCardExpireTime);

    PROTOBUF(record_cmd, record_send, record_len);
    m_pUser->sendCmdToMe(record_send, record_len);
  }
}

void UserRecords::userSaveRecord(SaveRecordUserCmd& cmd)
{
  if (!checkHasMenuOpen())
  {
    XERR << "[角色存档-保存存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,功能未开启" << XEND;
    MsgManager::sendMsg(m_pUser->id, 25431);
    return;
  }
  //[1] 检查是否在规定的地图
  if (!isInValidMap())
  {
    MsgManager::sendMsg(m_pUser->id, 25407);
    return;
  }

  //[2] 检查当前角色职业是否符合
  if (!isTwoExchangePro())
  {
    MsgManager::sendMsg(m_pUser->id, 25408);
    return;
  }
  
  //[3] 检查存档位是否开启, 是否有存档
  if (!checkSlotStatusOn(cmd.slotid()))
  {
    XERR << "[角色存档-保存存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位未开启" << XEND;
    return;
  }

  if (checkHasRecord(cmd.slotid()))
  {
    XERR << "[角色存档-保存存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位已有存档" << XEND;
    return;
  }

  //[4] 检查存档名是否合法
  if (!checkRecordName(cmd.record_name()))
  {
    XLOG << "[角色存档-存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存档失败:" << cmd.slotid() << ",存档名不合法:" << cmd.record_name() << XEND;
    return;
  }

  //[4] 存档
  ProfessionData& record = m_mapType2Record[cmd.slotid()];
  record.Clear();
  record.set_id(cmd.slotid());
  record.set_recordname(cmd.record_name());
  record.set_recordtime(xTime::getCurSec());
  record.set_charid(m_pUser->id);
  record.set_charname(m_pUser->name);
  record.set_type(ETypeRecord);
  DWORD dwBranchCur = m_pUser->getBranch();
  record.set_pro_branch(dwBranchCur);
  collectCurRecordData(record);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  UpdateRecordInfoUserCmd update_cmd;
  ProfessionUserInfo* pRecord = update_cmd.add_records();
  if (pRecord == nullptr)
    return;
  m_pUser->m_oProfession.collectProfessionUserInfo(record, pRecord);
  update_cmd.set_card_expiretime(m_dwCardExpireTime);

  PROTOBUF(update_cmd, update_send, update_len);
  m_pUser->sendCmdToMe(update_send, update_len);

  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_MVP_RECORD_SAVE);

  XLOG << "[角色存档-存档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存档位:" << cmd.slotid() << "存档名:" << cmd.record_name() << XEND;
}

void UserRecords::userLoadRecord(LoadRecordUserCmd& cmd, bool multi_role/* = false*/)
{
  if(!m_pUser)
    return;

  if(m_pUser->m_oBooth.hasOpen())
    return;

  if (!checkHasMenuOpen())
  {
    XERR << "[角色存档-加载存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,功能未开启" << XEND;
    MsgManager::sendMsg(m_pUser->id, 25431);
    return;
  }

  //[1] 检查load cd时间
  if (isInLoadCD())
  {
    MsgManager::sendMsg(m_pUser->id, 25420);
    return;
  }
  //[2] 检查是否在规定的地图
  if (multi_role)
  {
    if (m_dwDestCharID != m_pUser->id)
    {
      XERR << "[角色存档-加载存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,跨角色加载，目标角色ID不一致" << m_dwDestCharID << cmd.slotid() << XEND;
      return;
    }
    if (m_dwDestSlotID != cmd.slotid())
    {
      XERR << "[角色存档-加载存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,跨角色加载，目标存档位不一致" << m_dwDestSlotID << cmd.slotid() << XEND;
      return;
    }
  }
  else
  {
    if (!isInValidMap())
    {
      MsgManager::sendMsg(m_pUser->id, 25407);
      return;
    }
  }

  //[3] 检查存档位是否开启,是否有存档
  if (!checkSlotStatusOn(cmd.slotid()))
  {
    XERR << "[角色存档-加载存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位未开启" << XEND;
    return;
  }

  ProfessionData* pRecord = getRecordUserInfo(cmd.slotid());
  if (pRecord == nullptr)
  {
    XERR << "[角色存档-加载存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位为空" << XEND;
    return;
  }

  if (!multi_role && pRecord->charid() != m_pUser->id)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);

    m_dwDestSlotID = cmd.slotid();
    m_dwDestCharID = pRecord->charid();
    m_dwDestMapID = m_pUser->getScene()->id;

    XLOG << "[角色存档-读档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存档位:" << cmd.slotid() << ",跨角色读取" << XEND;
    return;
  }

  //[4] 加载
  XLOG << "[角色存档-读档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存档位:" << cmd.slotid() << XEND;

  loadRecordData(cmd.slotid());

  m_pUser->getEvent().onLoadRecord();

  const SRecordCFG& stRecordCFG = MiscConfig::getMe().getMiscRecordCFG();
  m_dwLastLoadTime = xTime::getCurSec() + stRecordCFG.dwLoadCD;

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_MVP_RECORD_LOAD);
  
  if (multi_role)
  {
    DWORD dwMapId = m_dwDestMapID;
    DWORD dwCurMapId = m_pUser->getScene() ? m_pUser->getScene()->id : 0;
    //跨角色加载存档时 策划要求回到主城或公会领地
    if (dwMapId == 0)
    {
      dwMapId = 1;
    }
    if (dwMapId == 10001 && !m_pUser->hasGuild())
    {
      dwMapId = 1;
    }
    if (dwMapId != dwCurMapId)
    {
      clearMultiRoleData();
      m_pUser->gomap(dwMapId, GoMapType::System);
      return;
    }
  }
  clearMultiRoleData();
}

void UserRecords::userBuySlot(BuyRecordSlotUserCmd& cmd)
{
  if (!checkHasMenuOpen())
  {
    XERR << "[角色存档-购买存档位], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,功能未开启" << XEND;
    MsgManager::sendMsg(m_pUser->id, 25431);
    return;
  }
  //[1] 检查是否在规定的地图
  if (!isInValidMap())
  {
    MsgManager::sendMsg(m_pUser->id, 25407);
    return;
  }

  //[2] 检查存档位是否可购买
  if (!checkSlotCanBuy(cmd.slotid()))
    return;

  //[3] 检查资源是否足够，并扣除
  SlotInfo* pSlot = getSlotInfo(cmd.slotid());
  if (pSlot == nullptr)
  {
    XERR << "[角色存档-购买存档位], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,没有该存档位" << cmd.slotid() << XEND;
    return;
  }
  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[角色存档-购买存档位], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,未找到包裹" << EPACKTYPE_MAIN << XEND;
    return;
  }

  if (pMainPack->checkItemCount(pSlot->costid(), pSlot->costnum()))
  {
    pMainPack->reduceItem(pSlot->costid(), ESOURCE_USER_RECORD, pSlot->costnum());
  }
  else
  {
    XERR << "[角色存档-购买存档位], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "购买存档位" << cmd.slotid() << "失败,需要" << pSlot->costnum() << "个" << pSlot->costid() << XEND;
    return;
  }

  //[4] 增加存档位
  pSlot->set_active(true);
  m_dwCurBuyTimes++;

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  UpdateRecordInfoUserCmd update_cmd;
  SlotInfo* pUpdateSlot = update_cmd.add_slots();
  if (pUpdateSlot == nullptr)
    return;
  pUpdateSlot->CopyFrom(*pSlot);
  update_cmd.set_card_expiretime(m_dwCardExpireTime);

  PROTOBUF(update_cmd, update_send, update_len);
  m_pUser->sendCmdToMe(update_send, update_len);

  XLOG << "[角色存档-购买存档位], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "购买存档位:" << cmd.slotid() << m_dwCurBuyTimes << "成功，花费" << pSlot->costnum() << "个" << pSlot->costid() << XEND;
}

void UserRecords::userChangeRecordName(ChangeRecordNameUserCmd& cmd)
{
  if (!checkHasMenuOpen())
  {
    XERR << "[角色存档-更改存档名], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,功能未开启" << XEND;
    MsgManager::sendMsg(m_pUser->id, 25431);
    return;
  }
  //[1] 检查是否在规定的地图
  if (!isInValidMap())
  {
    MsgManager::sendMsg(m_pUser->id, 25407);
    return;
  }

  //[2] 检查存档位是否开启,是否有存档
  if (!checkSlotStatusOn(cmd.slotid()))
  {
    XERR << "[角色存档-更改存档名], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位未开启" << XEND;
    return;
  }

  //[3] 改名
  ProfessionData* pRecord = getRecordUserInfo(cmd.slotid());
  if (pRecord == nullptr)
  {
    XERR << "[角色存档-更改存档名], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位为空" << XEND;
    return;
  }
  string old_name = pRecord->recordname();

  if (!checkRecordName(cmd.record_name(), old_name))
  {
    XLOG << "[角色存档-更改存档名], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "更改存档名:" << cmd.slotid() << old_name << "失败, 新名字:" << cmd.record_name() << XEND;
    return;
  }

  pRecord->set_recordname(cmd.record_name());
  
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  UpdateRecordInfoUserCmd update_cmd;
  ProfessionUserInfo* pRecordUser = update_cmd.add_records();
  if (pRecordUser == nullptr)
    return;
  m_pUser->m_oProfession.collectProfessionUserInfo(*pRecord, pRecordUser);
  update_cmd.set_card_expiretime(m_dwCardExpireTime);

  PROTOBUF(update_cmd, update_send, update_len);
  m_pUser->sendCmdToMe(update_send, update_len);

  XLOG << "[角色存档-更改存档名], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "更改存档名:" << cmd.slotid() << cmd.record_name() << "成功，旧名字:" << old_name << XEND;
}

void UserRecords::userDeleteRecord(DeleteRecordUserCmd& cmd)
{
  if (!checkHasMenuOpen())
  {
    XERR << "[角色存档-删除存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,功能未开启" << XEND;
    MsgManager::sendMsg(m_pUser->id, 25431);
    return;
  }
  //[1] 检查是否在规定的地图
  if (!isInValidMap())
  {
    MsgManager::sendMsg(m_pUser->id, 25407);
    return;
  }

  //[2] 检查存档位是否开启,是否有存档
  if (!checkSlotStatusOn(cmd.slotid()))
  {
    XERR << "[角色存档-删除存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位未开启" << XEND;
    return;
  }
  if (!checkHasRecord(cmd.slotid()))
  {
    XERR << "[角色存档-删除存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,存档位为空" << XEND;
    return;
  }

  //[3] 删除
  m_mapType2Record.erase(cmd.slotid());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  UpdateRecordInfoUserCmd update_cmd;
  update_cmd.add_delete_ids(cmd.slotid());
  update_cmd.set_card_expiretime(m_dwCardExpireTime);
  PROTOBUF(update_cmd, update_send, update_len);
  m_pUser->sendCmdToMe(update_send, update_len);

  XLOG << "[角色存档-删除存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "删除存档:" << cmd.slotid() << XEND;
}

void UserRecords::onProfesChange()
{
  UpdateRecordInfoUserCmd cmd;
  bool bNeedUpdate = false;

  for (auto& v : m_mapType2Record)
  {
    if (v.second.charid() == m_pUser->id && v.second.pro_branch() == m_pUser->getBranch())
    {
      v.second.set_profession(m_pUser->getProfession());
      ProfessionUserInfo* pRecord = cmd.add_records();
      if (pRecord == nullptr)
        continue;
      m_pUser->m_oProfession.collectProfessionUserInfo(v.second, pRecord);
      bNeedUpdate = true;
    }
  }
  cmd.set_card_expiretime(m_dwCardExpireTime);

  if (bNeedUpdate)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserRecords::onJobLvChange(DWORD dwJobLv)
{
  UpdateRecordInfoUserCmd cmd;
  bool bNeedUpdate = false;

  for (auto& v : m_mapType2Record)
  { 
    if (v.second.charid() == m_pUser->id && v.second.pro_branch() == m_pUser->getBranch())
    { 
      v.second.set_joblv(dwJobLv);
      ProfessionUserInfo* pRecord = cmd.add_records();
      if (pRecord == nullptr)
        continue;
      m_pUser->m_oProfession.collectProfessionUserInfo(v.second, pRecord);
      bNeedUpdate = true;
    }
  }
  cmd.set_card_expiretime(m_dwCardExpireTime);

  if (bNeedUpdate)
  { 
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserRecords::onCharNameChange(std::string strCharName)
{
  UpdateRecordInfoUserCmd cmd;
  bool bNeedUpdate = false;

  for (auto& v : m_mapType2Record)
  { 
    if (v.second.charid() == m_pUser->id)
    { 
      v.second.set_charname(strCharName);
      ProfessionUserInfo* pRecord = cmd.add_records();
      if (pRecord == nullptr)
        continue;
      m_pUser->m_oProfession.collectProfessionUserInfo(v.second, pRecord);
      bNeedUpdate = true;
    }
  }
  cmd.set_card_expiretime(m_dwCardExpireTime);

  if (bNeedUpdate)
  { 
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserRecords::onDelChar(QWORD qwDelCharid)
{
  std::set<DWORD> setTmpSlotID;
  for (auto& v : m_mapType2Record)
  {
    if (v.second.charid() == qwDelCharid)
    {
      setTmpSlotID.insert(v.first);
    }
  }
  
  if (setTmpSlotID.empty())
    return;

  UpdateRecordInfoUserCmd cmd;
  for (auto& v : setTmpSlotID)
  {
    m_mapType2Record.erase(v);
    cmd.add_delete_ids(v);
  }
  cmd.set_card_expiretime(m_dwCardExpireTime);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserRecords::onEquipExchange(const std::string& guidOld, const std::string& guidNew)
{
  for (auto& m : m_mapType2Record)
  {
    for(int i=0; i<m.second.pack_data_size(); ++i)
    {
      Cmd::EquipPackData* pDataPack = m.second.mutable_pack_data(i);
      if(!pDataPack)
      {
        XERR << "[角色存档-置换装备], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get Cmd::EquipPackData failed! i:" << i << " old guid:" << guidOld << "new guid:" << guidNew << XEND;
        continue;
      }

      for(int j=0; j<pDataPack->datas_size(); ++j)
      {
        Cmd::EquipInfo* pDataEquip = pDataPack->mutable_datas(j);
        if(!pDataEquip)
        {
          XERR << "[角色存档-置换装备], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get Cmd::EquipInfo failed! j:" << j << " old guid:" << guidOld << "new guid:" << guidNew << XEND;
          continue;
        }

        if(pDataEquip->guid() == guidOld)
        {
          pDataEquip->set_guid(guidNew);
          ItemBase* pItemBase = m_pUser->getPackage().getItem(guidNew);
          if(pItemBase)
            pDataEquip->set_type_id(pItemBase->getTypeID());
        }
      }
    }
  }
}

void UserRecords::onEnterSceneEnd()
{
  if (m_pUser->id != m_dwDestCharID)
  {
    return;
  }

  if (m_dwDestSlotID == 0 || m_dwDestMapID == 0)
    return;

  LoadRecordUserCmd cmd;
  cmd.set_slotid(m_dwDestSlotID);
  userLoadRecord(cmd, true);
}

void UserRecords::setCardExpireTime(DWORD dwTime)
{
  if (dwTime > m_dwCardExpireTime) 
  {
    m_dwCardExpireTime = dwTime;
    refreshSlotInfo();
  }
}

void UserRecords::collectCurRecordData(ProfessionData& record)
{
  m_pUser->m_oProfession.saveProfessionData(record);
}

void UserRecords::loadRecordData(DWORD dwSlotID)
{
  ProfessionData* pRecord = getRecordUserInfo(dwSlotID);
  if (pRecord == nullptr)
    return;

  if (pRecord->charid() != m_pUser->id)
  {
    XERR << "[角色存档-读取存档], 玩家" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败,不是当前角色存档，存档Charid:" << pRecord->charid() << EPACKTYPE_MAIN << XEND;
    return;
  }
  m_pUser->m_oProfession.saveCurProfessionData();
  m_pUser->m_oProfession.loadRecordProfessionData(*pRecord);

  if (m_pUser->getCurFighter())
    m_pUser->getCurFighter()->getSkill().fixEvo4Skill();
}

void UserRecords::clearMultiRoleData()
{
  m_dwDestSlotID = 0;
  m_dwDestCharID = 0;
  m_dwDestMapID = 0;
}

void UserRecords::collectTotalAstrolMaterialData(UserAstrolMaterialData* pAstrolData)
{
  if (pAstrolData == nullptr)
    return;

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return;

  TVecItemInfo total_item;
  m_pUser->getAstrolabes().getTotalCost(total_item);

  //zeny
  ItemInfo curItem;
  curItem.set_id(100);
  QWORD cur_zeny = m_pUser->getUserSceneData().getSilver();
  if(cur_zeny > DWORD_MAX)
  {
      curItem.set_count(DWORD_MAX);
  }
  else
  {
      curItem.set_count(cur_zeny);
  }
  combinItemInfo(total_item, curItem);

  //贡献
  curItem.Clear();
  curItem.set_id(140);
  curItem.set_count(m_pUser->getGuild().contribute());
  combinItemInfo(total_item, curItem);

  //金质勋章
  curItem.Clear();
  curItem.set_id(5261);
  curItem.set_count(pMainPack->getItemCount(5261));
  combinItemInfo(total_item, curItem);

  pAstrolData->set_charid(m_pUser->id);

  for (auto& v : total_item)
  {
    AstrolabeCostData* pMaterialData = pAstrolData->add_materials();
    if (pMaterialData)
    {
      pMaterialData->set_id(v.id());
      pMaterialData->set_count(v.count());
    }
  }
}
