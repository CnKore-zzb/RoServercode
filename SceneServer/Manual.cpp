#include "Manual.h"
#include "SceneUser.h"
#include "MapConfig.h"
#include "SceneManager.h"
#include "MailManager.h"
#include "ComposeConfig.h"
#include "MsgManager.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "SceneNpcManager.h"
#include "DepositConfig.h"
#include "Menu.h"
#include "BossConfig.h"

// data
void SManualQuest::fromData(const ManualQuest& rItem)
{
  dwID = rItem.id();
  dwProcess = rItem.process();
  bFinish = rItem.finish();
  bRewardGet = rItem.rewardget();

  pCFG = ManualConfig::getMe().getManualQuestCFG(dwID);
}

void SManualQuest::toData(ManualQuest* pItem)
{
  if (pItem == nullptr)
    return;

  pItem->set_id(dwID);
  pItem->set_process(dwProcess);
  pItem->set_finish(bFinish);
  pItem->set_rewardget(bRewardGet);
}

void SManualSubItem::fromData(const ManualSubItem& rItem)
{
  dwID = rItem.id();
  dwStoreID = rItem.storeid();
  eStatus = rItem.status();
  bUnlock = rItem.unlock();
  bStore = rItem.store();
  oItem.CopyFrom(rItem.item());

  vecQuests.clear();
  vecQuests.reserve(rItem.quests_size());
  for (int i = 0; i < rItem.quests_size(); ++i)
  {
    SManualQuest stQuest;
    stQuest.fromData(rItem.quests(i));
    vecQuests.push_back(stQuest);
  }

  for (int i = 0; i < rItem.params_size(); ++i)
    arrParams[i] = rItem.params(i);
  for (int i = 0; i < rItem.data_params_size(); ++i)
  {
    arrDataParams[i] = rItem.data_params(i);
    if (arrDataParams[i].empty() == false)
      XDBG << "index :" << i << "data :" << arrDataParams[i] << XEND;
  }
}

void SManualSubItem::toData(ManualSubItem* pItem)
{
  if (pItem == nullptr)
    return;

  pItem->set_id(dwID);
  pItem->set_storeid(dwStoreID);
  pItem->set_status(eStatus);
  pItem->set_unlock(bUnlock);
  pItem->set_store(bStore);
  if (bStore)
    pItem->mutable_item()->CopyFrom(oItem);

  pItem->clear_quests();
  for (auto v = vecQuests.begin(); v != vecQuests.end(); ++v)
    v->toData(pItem->add_quests());

  for (DWORD d = 0; d < MAX_MANUAL_PARAM; ++d)
    pItem->add_params(arrParams[d]);
  for (DWORD d = 0; d < MAX_MANUAL_PARAM; ++d)
    pItem->add_data_params(arrDataParams[d]);
}

SManualQuest* SManualSubItem::getManualQuest(DWORD dwID)
{
  auto v = find_if(vecQuests.begin(), vecQuests.end(), [dwID](const SManualQuest& r) -> bool{
    return r.dwID == dwID;
  });
  if (v != vecQuests.end())
    return &(*v);

  return nullptr;
}

bool SManualSubItem::checkPreQuest(const TSetDWORD& setIDs)
{
  for (auto &s : setIDs)
  {
    SManualQuest* pQuest = getManualQuest(s);
    if (pQuest == nullptr || !pQuest->bFinish)
      return false;
  }
  return true;
}

bool SManualSubItem::setParam(DWORD dwIndex, DWORD dwValue)
{
  if (dwIndex >= MAX_MANUAL_PARAM)
    return false;
  arrParams[dwIndex] = dwValue;
  return true;
}

DWORD SManualSubItem::getParam(DWORD dwIndex) const
{
  if (dwIndex >= MAX_MANUAL_PARAM)
    return 0;
  return arrParams[dwIndex];
}

bool SManualSubItem::setDataParam(DWORD dwIndex, const string& data)
{
  if (dwIndex >= MAX_MANUAL_PARAM)
    return false;
  arrDataParams[dwIndex] = data;
  return true;
}

const string& SManualSubItem::getDataParam(DWORD dwIndex) const
{
  if (dwIndex >= MAX_MANUAL_PARAM)
    return STRING_EMPTY;
  return arrDataParams[dwIndex];
}

void SManualItem::fromData(const ManualItem& rItem)
{
  eType = rItem.type();
  dwVersion = rItem.version();

  vecQuests.clear();
  vecQuests.reserve(rItem.quests_size());
  for (int i = 0; i < rItem.quests_size(); ++i)
  {
    SManualQuest stQuest;
    stQuest.fromData(rItem.quests(i));
    vecQuests.push_back(stQuest);
  }

  vecSubItems.clear();
  vecSubItems.reserve(rItem.items_size());
  for (int i = 0; i < rItem.items_size(); ++i)
  {
    SManualSubItem stSubItem;
    stSubItem.fromData(rItem.items(i));
    if(stSubItem.eStatus == EMANUALSTATUS_DISPLAY &&
        (eType == EMANUALTYPE_NPC || eType == EMANUALTYPE_SCENERY))
      continue;

    vecSubItems.push_back(stSubItem);
  }
}

void SManualItem::toData(ManualItem* pItem, bool bClient)
{
  if (pItem == nullptr)
    return;

  pItem->set_type(eType);
  pItem->set_version(dwVersion);

  pItem->clear_quests();
  for (auto &v : vecQuests)
    v.toData(pItem->add_quests());

  pItem->clear_items();
  for (auto v = vecSubItems.begin(); v != vecSubItems.end(); ++v)
  {
    if(v->eStatus == EMANUALSTATUS_DISPLAY &&
        (eType == EMANUALTYPE_NPC || eType == EMANUALTYPE_SCENERY))
      continue;

    ManualSubItem* pSubItem = pItem->add_items();
    v->toData(pSubItem);

    if (bClient)
    {
      for (auto &v : vecQuests)
        v.toData(pSubItem->add_quests());
    }

    for (int i = 0; i < pSubItem->quests_size(); ++i)
      XDBG << "[冒险手册-月卡测试]" << pSubItem->ShortDebugString() << XEND;
  }
}

void SManualItem::toData(ManualVersion* pVersion)
{
  if (pVersion == nullptr)
    return;

  pVersion->set_type(eType);
  pVersion->set_version(dwVersion);
}

SManualSubItem* SManualItem::getSubItem(DWORD dwID)
{
  auto v = find_if(vecSubItems.begin(), vecSubItems.end(), [dwID](const SManualSubItem& r) -> bool{
    return r.dwID == dwID;
  });
  if (v != vecSubItems.end())
    return &(*v);

  return nullptr;
}

bool SManualItem::delSubItem(DWORD dwID)
{
  auto v = find_if(vecSubItems.begin(), vecSubItems.end(), [dwID](SManualSubItem& r) -> bool{
    return r.dwID == dwID;
  });
  if (v != vecSubItems.end())
  {
    vecSubItems.erase(v);
    return true;
  }

  return false;
}

bool SManualItem::addSubItem(DWORD dwID, EManualStatus eStatus)
{
  auto v = find_if(vecSubItems.begin(), vecSubItems.end(), [dwID,eStatus](SManualSubItem& r) -> bool{
    return r.dwID == dwID;
  });
  if (v == vecSubItems.end())
  {
    SManualSubItem stSubItem;
    stSubItem.dwID = dwID;
    stSubItem.eStatus = eStatus;
    vecSubItems.push_back(stSubItem);
    dwVersion = xTime::getCurSec();
  }
  else
  {
    if(eStatus == v->eStatus)
      return false;

    v->eStatus = eStatus;
    dwVersion = xTime::getCurSec();
  }

  return true;
}

SManualQuest* SManualItem::getManualQuest(DWORD dwID)
{
  auto v = find_if(vecQuests.begin(), vecQuests.end(), [dwID](const SManualQuest& r) -> bool{
    return r.dwID == dwID;
  });
  if (v != vecQuests.end())
    return &(*v);

  return nullptr;
}

bool SManualItem::isFinishAll(EManualType eType, DWORD dwID)
{
  TVecManualQuestCFG vecCFG;
  ManualConfig::getMe().collectQuest(eType, dwID, vecCFG);

  if (eType == EMANUALTYPE_COLLECTION)
  {
    for (auto &v : vecCFG)
    {
      SManualQuest* pQuest = getManualQuest(v.dwID);
      if (pQuest == nullptr || !pQuest->bFinish)
        return false;
    }
  }
  else
  {
    SManualSubItem* pSubItem = getSubItem(dwID);
    if (pSubItem == nullptr)
      return false;
    for (auto &v : vecCFG)
    {
      SManualQuest* pQuest = pSubItem->getManualQuest(v.dwID);
      if (pQuest == nullptr || !pQuest->bFinish)
        return false;
    }
  }

  return true;
}

bool SManualItem::checkPreQuest(const TSetDWORD& setIDs)
{
  for (auto &s : setIDs)
  {
    SManualQuest* pQuest = getManualQuest(s);
    if (pQuest == nullptr || !pQuest->bFinish)
      return false;
  }
  return true;
}

void SManualGroup::fromData(const ManualGroup& rGroup)
{
  dwID = rGroup.id();
}

void SManualGroup::toData(ManualGroup* pGroup)
{
  if (pGroup == nullptr)
    return;

  pGroup->set_id(dwID);
}

SManualItem* SManualData::getManualItem(EManualType eType)
{
  auto v = find_if(vecItems.begin(), vecItems.end(), [eType](const SManualItem& rItem) -> bool{
    return rItem.eType == eType;
  });
  if (v != vecItems.end())
    return &(*v);

  return nullptr;
}

bool SUnsovledUserPhoto::fromData(const UnsolvedUserPhoto& rData)
{
  qwCharID = rData.charid();
  strName = rData.name();

  for (int i = 0; i < rData.photos_size(); ++i)
  {
    const UnsolvedPhoto& r = rData.photos(i);
    mapPhotos[r.id()].CopyFrom(r);
  }

  return true;
}

bool SUnsovledUserPhoto::toData(UnsolvedUserPhoto* pData)
{
  if (pData == nullptr)
    return false;

  pData->set_charid(qwCharID);
  pData->set_name(strName);
  for (auto &item : mapPhotos)
    pData->add_photos()->CopyFrom(item.second);
  return true;
}

// manual
Manual::Manual(SceneUser* pUser) : m_pUser(pUser)
{

}

Manual::~Manual()
{

}

bool Manual::load(const BlobManual& oAcc, const BlobManual& oChar, const BlobUnsolvedPhoto& oPhoto)
{
  // acc data
  m_stData.dwVersion = oAcc.data().version();
  m_stData.dwPoint = oAcc.data().point();
  m_stData.dwExchangeTime = oAcc.data().exchange_time();
  m_stData.dwLevel = oAcc.data().level();
  if (m_stData.dwLevel == 0)
    m_stData.dwLevel = 1;
  m_pLvCFG = ManualConfig::getMe().getManualLvCFG(m_stData.dwLevel);

  m_stData.vecItems.clear();
  m_stData.vecItems.reserve(EMANUALTYPE_MAX);
  for (int i = 0; i < oAcc.data().items_size(); ++i)
  {
    const ManualItem &item = oAcc.data().items(i);
    EManualType eType = item.type();
    if (eType <= EMANUALTYPE_MIN || eType >= EMANUALTYPE_MAX || EManualType_IsValid(eType) == false)
    {
      XERR << "[冒险手册-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加载" << oAcc.data().items(i).ShortDebugString() << "失败" << XEND;
      continue;
    }

    SManualItem stItem;
    stItem.fromData(item);
    m_stData.vecItems.push_back(stItem);
  }

  m_stData.mapGroups.clear();
  for (int i = 0; i < oAcc.data().groups_size(); ++i)
  {
    const ManualGroup& rGroup = oAcc.data().groups(i);
    m_stData.mapGroups[rGroup.id()].fromData(rGroup);
  }

  // acc photo
  m_mapUser2Photo.clear();
  for (int i = 0; i < oPhoto.photos_size(); ++i)
  {
    const UnsolvedUserPhoto& rUserPhoto = oPhoto.photos(i);
    m_mapUser2Photo[rUserPhoto.charid()].fromData(rUserPhoto);
  }

  /*if (oAcc.isadd() == false)
  {
    DWORD dwSkillPoint = m_stData.dwLevel;
    if (dwSkillPoint == 0)
      dwSkillPoint = 1;
    addSkillPoint(dwSkillPoint, false);
  }*/

  // char data
  m_stCharData.dwVersion = oChar.data().version();
  m_stCharData.dwLevel = oChar.data().level();

  version_update();
  initDefaultItem();
  calcSkillPoint();

  return true;
}

bool Manual::save(BlobManual* pAcc, BlobManual* pChar, BlobUnsolvedPhoto* pPhoto)
{
  if (pAcc == nullptr || pChar == nullptr || pPhoto == nullptr)
    return false;

  // acc data
  pAcc->Clear();

  pAcc->mutable_data()->set_version(m_stData.dwVersion);
  pAcc->mutable_data()->set_point(m_stData.dwPoint);
  pAcc->mutable_data()->set_level(m_stData.dwLevel);
  pAcc->mutable_data()->set_exchange_time(m_stData.dwExchangeTime);

  for (auto v = m_stData.vecItems.begin(); v != m_stData.vecItems.end(); ++v)
    v->toData(pAcc->mutable_data()->add_items(), false);
  for (auto m : m_stData.mapGroups)
    m.second.toData(pAcc->mutable_data()->add_groups());
  pAcc->set_isadd(true);

  // acc photo
  for (auto &m : m_mapUser2Photo)
    m.second.toData(pPhoto->add_photos());

  // char data
  pChar->Clear();

  pChar->mutable_data()->set_version(m_stCharData.dwVersion);
  pChar->mutable_data()->set_skillpoint(m_stCharData.dwSkillPoint);

  XDBG << "[冒险手册-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 acc :" << pAcc->ByteSize() << "char :" << pChar->ByteSize() << XEND;
  return true;
}

void Manual::reload()
{
  m_pLvCFG = ManualConfig::getMe().getManualLvCFG(m_stData.dwLevel);
  if (m_pLvCFG == nullptr)
    XERR << "[冒险手册-重加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "level :" << m_stData.dwLevel << "未在 Table_AdventureLevel.txt 中找到" << XEND;

  for (auto &v : m_stData.vecItems)
  {
    for (auto &o : v.vecSubItems)
    {
      for (auto &k : o.vecQuests)
      {
        k.pCFG = ManualConfig::getMe().getManualQuestCFG(o.dwID);
        if (k.pCFG == nullptr)
          XERR << "[冒险手册-重加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
            << "id :" << o.dwID << "未在 Table_AdventureAppend.txt 中找到" << XEND;
      }
    }
  }

  XLOG << "[冒险手册-重加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行重加载" << XEND;
}

void Manual::initDefaultItem()
{
  DWORD dwNow = xTime::getCurSec();
  for (int i = EMANUALTYPE_MIN + 1; i < EMANUALTYPE_MAX; ++i)
  {
    EManualType eType = static_cast<EManualType>(i);
    if (eType <= EMANUALTYPE_MIN || eType >= EMANUALTYPE_MAX || EManualType_IsValid(eType) == false)
      continue;

    SManualItem* pItem = m_stData.getManualItem(static_cast<EManualType>(i));
    if (pItem != nullptr)
      continue;

    SManualItem stItem;
    stItem.eType = static_cast<EManualType>(i);
    stItem.dwVersion = dwNow;
    m_stData.vecItems.push_back(stItem);
  }

  refreshGroup();
}

void Manual::initDefaultData()
{
  initDefaultItem();

  /*const TSetItemNoSource& setList = ItemConfig::getMe().getItemNoSourceList();
  for (auto s = setList.begin(); s != setList.end(); ++s)
    onItemAdd(*s);*/

  /*
  const TMapDepositCard& mapList = DepositConfig::getMe().getCardCFGList();
  for (auto &m : mapList)
  {
    if (m.second.type == ETITLE_TYPE_MONTH)
      onItemAdd(m.first);
  }
  */
  //onPointChange();
}

void Manual::queryManualData(EManualType eType)
{
  if(eType == EMANUALTYPE_EQUIP)
    return;

  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
    return;

  QueryManualData cmd;
  cmd.set_type(eType);
  pItem->toData(cmd.mutable_item(), true);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XDBG << "[冒险手册-数据请求]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "请求数据" << cmd.ShortDebugString() << XEND;
}

void Manual::queryUnsolvedPhoto()
{
  if (xTime::getCurSec() > m_stData.dwExchangeTime + MiscConfig::getMe().getManualCFG().dwUnsolvedPhotoOvertime)
  {
    XLOG << "[冒险手册-离线照片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "过期" << XEND;
    return;
  }
  if (m_mapUser2Photo.empty() == true)
    return;

  QueryUnsolvedPhotoManualCmd cmd;
  cmd.set_time(m_stData.dwExchangeTime + MiscConfig::getMe().getManualCFG().dwUnsolvedPhotoOvertime);
  for (auto &m : m_mapUser2Photo)
    m.second.toData(cmd.add_photos());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[冒险手册-离线照片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步" << cmd.ShortDebugString() << XEND;
}

void Manual::sendManualData()
{
  sendPoint();
  if(m_pUser->getMenu().isOpen(EMENUID_MANUAL) == false)
    return;

  sendAllVersion();
  sendSkillPoint();
  sendLevel();
  queryUnsolvedPhoto();
}

bool Manual::subSkillPoint(DWORD dwPoint)
{
  if (checkSkillPoint(dwPoint) == false)
    return false;

  m_stCharData.dwSkillPoint -= dwPoint;
  sendSkillPoint();
  return true;
}

void Manual::addSkillPoint(DWORD dwPoint, bool bNotify /*= true*/)
{
  if (dwPoint == 0 || m_pUser == nullptr)
    return;

  m_stCharData.dwSkillPoint += dwPoint;

  if (bNotify)
  {
    sendSkillPoint();
    MsgManager::sendMsg(m_pUser->id, 551, MsgParams(dwPoint));
  }

  XLOG << "[冒险手册-技能点增加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获得" << dwPoint << "技能点 总技能点为" << m_stData.dwSkillPoint << XEND;
}

void Manual::onEnterMap(DWORD dwMapID, bool bRealOpen /*= true*/)
{
  if (m_pUser == nullptr || MapConfig::getMe().isRaidMap(dwMapID) == true)
    return;

  const SMapCFG* pMapBase = MapConfig::getMe().getMapCFG(dwMapID);
  if (pMapBase == nullptr)
    return;
  SWORD swAdventureValue = pMapBase->swAdventureValue;// getTableInt("AdventureValue");
  if (swAdventureValue == 0)
    return;

  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MAP);
  if (pItem == nullptr)
    return;

  auto v = find_if(pItem->vecSubItems.begin(), pItem->vecSubItems.end(), [dwMapID](const SManualSubItem& rItem) -> bool{
    return rItem.dwID == dwMapID;
  });
  if (v != pItem->vecSubItems.end() && (v->eStatus == EMANUALSTATUS_UNLOCK_CLIENT || v->eStatus == EMANUALSTATUS_UNLOCK))
    return;
  if (v != pItem->vecSubItems.end() && !bRealOpen)
    return;

  if (v == pItem->vecSubItems.end())
  {
    SManualSubItem stSubItem;
    stSubItem.dwID = dwMapID;
    stSubItem.eStatus = bRealOpen ? EMANUALSTATUS_UNLOCK_CLIENT: EMANUALSTATUS_DISPLAY;
    pItem->vecSubItems.push_back(stSubItem);
    pItem->dwVersion = xTime::getCurSec();
    v = find_if(pItem->vecSubItems.begin(), pItem->vecSubItems.end(), [dwMapID](const SManualSubItem& rItem) -> bool{
      return rItem.dwID == dwMapID;
    });
    if (v == pItem->vecSubItems.end())
      return;
  }

  v->eStatus = bRealOpen? EMANUALSTATUS_UNLOCK_CLIENT: EMANUALSTATUS_DISPLAY;
  pItem->dwVersion = xTime::getCurSec();

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(EMANUALTYPE_MAP);
  cmd.mutable_update()->set_version(pItem->dwVersion);
  ManualSubItem* pSubItem = cmd.mutable_update()->add_items();
  v->toData(pSubItem);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getQuest().onManualUnlock();
  XLOG << "[冒险手册-地图解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mapid :" << v->dwID << "status :" << v->eStatus << XEND;

  // unlock monster
  const SceneBase* pBase = SceneManager::getMe().getDataByID(dwMapID);
  if (pBase != nullptr)
  {
    const SceneObject *pObject = pBase->getSceneObject(0);
    if (pObject != nullptr)
    {
      const list<SceneNpcTemplate>& npclist = pObject->getNpcList();
      for (auto &l : npclist)
        onKillMonster(l.m_oDefine.getID(), false);
    }
  }

  // unlock boss : 这段代码很奇怪,BossUnlockCallBack中的dwMapID的值会奇怪的变化,保留此代码,以后查问题
  /*struct BossUnlockCallBack : public xEntryCallBack
  {
    Manual& rManual;
    DWORD dwMapID = 0;

    BossUnlockCallBack(Manual& manual) : rManual(manual) {}

    virtual bool exec(xEntry* entry)
    {
      const BossBase* pBase = dynamic_cast<const BossBase*>(entry);
      if (pBase == nullptr)
        return false;
      if (pBase->mapid != dwMapID)
        return false;

      rManual.onKillMonster(static_cast<DWORD>(pBase->id), false);
      return true;
    }
  };
  BossUnlockCallBack stBack(*this);
  stBack.dwMapID = dwMapID;
  g_pBossBaseM->forEach(stBack);*/
  auto findf = [dwMapID, this](const SBossCFG& rCFG)
  {
    if (!rCFG.blInit || rCFG.getMapID() != dwMapID)
      return;

    onKillMonster(static_cast<DWORD>(rCFG.dwID), false);
  };
  BossConfig::getMe().foreach(findf);
}

void Manual::onKillMonster(DWORD dwMonsterID, bool bRealKill /*= true*/)
{
  if (m_pUser == nullptr)
    return;

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwMonsterID);
  if (pCFG == nullptr || pCFG->eNpcType == ENPCTYPE_GATHER)
    return;
  if (pCFG->swAdventureValue == 0)
    return;

  EManualType eType = pCFG->eNpcType == ENPCTYPE_NPC ? EMANUALTYPE_NPC : EMANUALTYPE_MONSTER;
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
    return;

  SManualSubItem* pSubItem = pItem->getSubItem(dwMonsterID);
  if (pSubItem != nullptr && (pSubItem->eStatus == EMANUALSTATUS_UNLOCK_STEP || pSubItem->eStatus == EMANUALSTATUS_UNLOCK_CLIENT || pSubItem->eStatus == EMANUALSTATUS_UNLOCK))
    return;
  bool bLock = checkLock(pCFG->eLockType, false, bRealKill, false, ESOURCE_MIN);
  if (pSubItem != nullptr && !bLock)
    return;

 // bool blMonster = (eType == EMANUALTYPE_MONSTER) && bRealKill;
  if (pSubItem == nullptr)
  {
    SManualSubItem stSubItem;
    stSubItem.dwID = dwMonsterID;
    if(bRealKill)
      stSubItem.eStatus = bLock ? EMANUALSTATUS_UNLOCK_CLIENT : EMANUALSTATUS_DISPLAY;
    else
      stSubItem.eStatus = EMANUALSTATUS_DISPLAY;
    pItem->vecSubItems.push_back(stSubItem);
    pSubItem = pItem->getSubItem(dwMonsterID);
  }
  if (pSubItem == nullptr)
    return;

  pSubItem->eStatus = bLock ? EMANUALSTATUS_UNLOCK_CLIENT : EMANUALSTATUS_DISPLAY;
  pItem->dwVersion = xTime::getCurSec();

  //if(pSubItem->eStatus != EMANUALSTATUS_DISPLAY)
  {
    ManualUpdate cmd;
    cmd.mutable_update()->set_type(eType);
    cmd.mutable_update()->set_version(pItem->dwVersion);
    ManualSubItem* subItem = cmd.mutable_update()->add_items();
    pSubItem->toData(subItem);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  m_pUser->getQuest().onManualUnlock();
  XLOG << "[冒险手册-怪物解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pSubItem->dwID << "status :" << pSubItem->eStatus << XEND;

  // unlock item - reward
  for (auto v = pCFG->vecRewardIDs.begin(); v != pCFG->vecRewardIDs.end(); ++v)
  {
    const SRewardCFG* pRewardCFG = RewardConfig::getMe().getRewardCFG(*v);
    if (pRewardCFG != nullptr)
    {
      for (auto o = pRewardCFG->vecItems.begin(); o != pRewardCFG->vecItems.end(); ++o)
      {
        for (auto k = o->vecItems.begin(); k != o->vecItems.end(); ++k)
          onItemAdd(k->oItem.id(), false);
      }
    }
  }

  // unlock item - shop item
  if (eType == EMANUALTYPE_NPC)
  {
    for (auto m = pCFG->stNpcFunc.mapTypes.begin(); m != pCFG->stNpcFunc.mapTypes.end(); ++m)
    {
      if (m->second.empty())
        continue;
      TVecShopItem vecItems;
      ShopConfig::getMe().collectShopItem(m->first, *m->second.begin(), vecItems);
      for (auto v = vecItems.begin(); v != vecItems.end(); ++v)
        onItemAdd(v->itemid());
    }
  }
}

void Manual::onKillProcess(DWORD dwMonsterID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return;
  SManualSubItem* pSubItem = pItem->getSubItem(dwMonsterID);
  if (pSubItem == nullptr )//|| pSubItem->eStatus != EMANUALSTATUS_UNLOCK_STEP)
    return;

  acceptQuest(EMANUALTYPE_MONSTER, dwMonsterID, pItem, pSubItem);

  bool bUpdate = false;
  for (auto v = pSubItem->vecQuests.begin(); v != pSubItem->vecQuests.end(); ++v)
  {
    if (v->bFinish || v->pCFG == nullptr || v->pCFG->eQuestType != EMANUALQUEST_KILL)
      continue;

    v->dwProcess += 1;

    if (v->pCFG->vecParams.empty() == false)
      v->bFinish = v->dwProcess >= v->pCFG->vecParams[0];
    bUpdate = true;
  }

  if (bUpdate)
  {
    pItem->dwVersion = xTime::getCurSec();
    ManualUpdate cmd;
    cmd.mutable_update()->set_type(EMANUALTYPE_MONSTER);
    cmd.mutable_update()->set_version(pItem->dwVersion);
    pSubItem->toData(cmd.mutable_update()->add_items());
    PROTOBUF(cmd, send, len);

    m_pUser->sendCmdToMe(send, len);
  }
}

//void Manual::onItemAdd(DWORD dwItemID, bool bRealAdd /*= true*/, bool bNotify /*= true*/, bool bCompose /*= false*/, bool bPickup /*= false*/, bool bUsed /*= false*/, bool bDraw /*= false*/)
void Manual::onItemAdd(DWORD dwItemID, bool bRealAdd, bool bNotify, bool bUsed, ESource source)
{
  if (m_pUser == nullptr)
    return;

  auto unlockitem = [this](DWORD dwItemID, bool bRealAdd, bool bNotify, bool bUsed, ESource source)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(dwItemID);
    if (pCFG == nullptr)
      return;
    if (pCFG->dwGroupID != 0)
    {
      dwItemID = pCFG->dwGroupID;
      pCFG = ItemConfig::getMe().getItemCFG(dwItemID);
      if (pCFG == nullptr)
        return;
    }

    if (ItemConfig::getMe().canEnterManual(pCFG) == false)
      return;

    EManualType eType = ManualConfig::getMe().getManualType(pCFG->eItemType);
    SManualItem* pItem = m_stData.getManualItem(eType);
    if (pItem == nullptr)
      return;

    // 月卡 不能入册灰显
    if (pCFG->eItemType == EITEMTYPE_MONTHCARD && !bUsed)
      return;

    SManualSubItem* pSubItem = pItem->getSubItem(dwItemID);
    if (pSubItem != nullptr)
    {
      if (pSubItem->eStatus == EMANUALSTATUS_UNLOCK_CLIENT || pSubItem->eStatus == EMANUALSTATUS_UNLOCK)
        return;

      bool canLock = false;
      for(auto &v : pCFG->vecLockType)
      {
        EManualLockMethod eLockType = static_cast<EManualLockMethod>(v);
        if (checkLock(eLockType, bRealAdd, false, bUsed, source) == true)
        {
          canLock = true;
          break;
        }
      }
      if(!canLock)
        return;

      pSubItem->eStatus = EMANUALSTATUS_UNLOCK_CLIENT;
    }
    else
    {
      SManualSubItem stSubItem;
      stSubItem.dwID = dwItemID;
      bool canLock = false;
      for(auto &v : pCFG->vecLockType)
      {
        EManualLockMethod eLockType = static_cast<EManualLockMethod>(v);
        if (checkLock(eLockType, bRealAdd, false, bUsed, source) == true)
        {
          canLock = true;
          break;
        }
      }
      stSubItem.eStatus = canLock ? EMANUALSTATUS_UNLOCK_CLIENT : EMANUALSTATUS_DISPLAY;

      pItem->vecSubItems.push_back(stSubItem);
      pSubItem = pItem->getSubItem(dwItemID);
      if (pSubItem == nullptr)
        return;
    }

    pItem->dwVersion = xTime::getCurSec();

    if (pSubItem->eStatus == EMANUALSTATUS_UNLOCK)
    {
      if (eType == EMANUALTYPE_COLLECTION)
      {
        refreshGroup();
        m_pUser->getQuest().acceptCondQuest(EQUESTCOND_MANUAL);
      }
      m_pUser->getMenu().refreshNewMenu(EMENUCOND_MANUALUNLOCK);
    }

    if (bNotify)
    {
      ManualUpdate cmd;
      cmd.mutable_update()->set_type(eType);
      cmd.mutable_update()->set_version(pItem->dwVersion);
      pSubItem->toData(cmd.mutable_update()->add_items());
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }

    m_pUser->getQuest().onManualUnlock();
    XLOG << "[冒险手册-物品解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pSubItem->dwID <<
      "status:" << pSubItem->eStatus << "ESource: " << source << XEND;
  };

  unlockitem(dwItemID, bRealAdd, bNotify, bUsed, source);

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
  if (pCFG != nullptr && pCFG->eItemType == EITEMTYPE_SHEET)
  {
    const SComposeCFG* pComposeCFG = ComposeConfig::getMe().getComposeCFG(pCFG->dwComposeID);
    if (pComposeCFG != nullptr)
      unlockitem(pComposeCFG->stProduct.dwTypeID, false, bNotify, false, ESOURCE_MIN);
  }
}

void Manual::onPhoto(DWORD dwMonsterID)
{
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwMonsterID);
  if (pCFG == nullptr)
  {
    XERR << "[冒险手册-摄影]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "拍摄" << dwMonsterID << "不存在" << XEND;
    return;
  }
  if (pCFG->eNpcType == ENPCTYPE_NPC)
  {
    onKillMonster(dwMonsterID);
    return;
  }

  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return;

  SManualSubItem* pSubItem = pItem->getSubItem(dwMonsterID);
  //if (pSubItem == nullptr || pSubItem->bUnlock)
  //  return;
  //if (pSubItem->eStatus == EMANUALSTATUS_DISPLAY || pSubItem->eStatus == EMANUALSTATUS_UNLOCK_CLIENT)
  //  return;
  if(pSubItem == nullptr && pCFG->swAdventureValue != 0)
  {
    SManualSubItem stSubItem;
    stSubItem.dwID = dwMonsterID;
    stSubItem.eStatus = EMANUALSTATUS_DISPLAY;
    pItem->vecSubItems.push_back(stSubItem);
    pSubItem = pItem->getSubItem(dwMonsterID);
  }

  if (pSubItem == nullptr || pSubItem->bUnlock)
    return;

  TVecManualQuestCFG vecCFG;
  ManualConfig::getMe().collectQuest(EMANUALTYPE_MONSTER, pSubItem->dwID, vecCFG);
  for (auto &v : vecCFG)
  {
    if (v.eQuestType != EMANUALQUEST_PHOTO)
      continue;
    SManualQuest* pQuest = pSubItem->getManualQuest(v.dwID);
    if (pQuest != nullptr)
      continue;
    if (v.setPreIDs.empty() == false)
    {
       bool bSuccess = true;
       for (auto &s : v.setPreIDs)
       {
         pQuest = pSubItem->getManualQuest(s);
         if (pQuest == nullptr || !pQuest->bFinish)
         {
           bSuccess = false;
           break;
         }
       }
       if (!bSuccess)
         continue;
    }
    SManualQuest stQuest;
    stQuest.dwID = v.dwID;
    stQuest.pCFG = ManualConfig::getMe().getManualQuestCFG(v.dwID);
    pSubItem->vecQuests.push_back(stQuest);
    XLOG << "[冒险手册-拍照], 自动接取拍照任务" << m_pUser->name << m_pUser->id << "接取" << stQuest.dwID << XEND;
  }

  pSubItem->bUnlock = true;
  pItem->dwVersion = xTime::getCurSec();

  MsgManager::sendMsg(m_pUser->id, 560, MsgParams(pCFG->strName));

  for (auto &v : pSubItem->vecQuests)
  {
    if (v.pCFG != nullptr && v.pCFG->eQuestType == EMANUALQUEST_PHOTO)
    {
      v.bFinish = true;
      v.bRewardGet = false;
    }
  }

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(pItem->eType);
  cmd.mutable_update()->set_version(pItem->dwVersion);
  ManualSubItem* subItem = cmd.mutable_update()->add_items();
  pSubItem->toData(subItem);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Manual_PhotoMonster;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);
  PlatLogManager::getMe().ManualLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    EManual_PhotoMonster,
    0,
    0,
    dwMonsterID);

  m_pUser->getQuest().onManualUnlock();
  m_pUser->getAchieve().onManual(EMANUALTYPE_MONSTER, EACHIEVECOND_MONSTER_PHOTO);

  XLOG << "[冒险手册-摄影]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "id :" << pSubItem->dwID << "status :" << pSubItem->eStatus << "unlock :" << pSubItem->bUnlock << XEND;
}

void Manual::onPhotoProcess(DWORD dwMonsterID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return;
  SManualSubItem* pSubItem = pItem->getSubItem(dwMonsterID);
  if (pSubItem == nullptr || pSubItem->eStatus != EMANUALSTATUS_UNLOCK_STEP)
    return;

  bool bUpdate = false;
  for (auto v = pSubItem->vecQuests.begin(); v != pSubItem->vecQuests.end(); ++v)
  {
    if (v->pCFG == nullptr || v->pCFG->eQuestType != EMANUALQUEST_PHOTO)
      continue;

    v->bFinish = true;
    bUpdate = true;
  }

  if (bUpdate)
  {
    pItem->dwVersion = xTime::getCurSec();
    ManualUpdate cmd;
    cmd.mutable_update()->set_type(EMANUALTYPE_MONSTER);
    cmd.mutable_update()->set_version(pItem->dwVersion);
    pSubItem->toData(cmd.mutable_update()->add_items());
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void Manual::onScenery(DWORD dwSceneryID, bool bRealAdd /*= true*/)
{
  if (m_pUser == nullptr)
    return;

  const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(dwSceneryID);
  if (pBase == nullptr)
    return;
  SWORD swValue = pBase->getAdvectureValue();
  if (swValue != -1 && swValue == 0)
    return;

  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_SCENERY);
  if (pItem == nullptr)
    return;

  SManualSubItem* pSubItem = pItem->getSubItem(dwSceneryID);
  if (pSubItem != nullptr && (pSubItem->eStatus == EMANUALSTATUS_UNLOCK_CLIENT || pSubItem->eStatus == EMANUALSTATUS_UNLOCK))
    return;
  if (pSubItem != nullptr && !bRealAdd)
    return;

  if (pSubItem == nullptr)
  {
    SManualSubItem stSubItem;
    stSubItem.dwID = dwSceneryID;
    stSubItem.eStatus = bRealAdd ? EMANUALSTATUS_UNLOCK_CLIENT : EMANUALSTATUS_DISPLAY;
    pItem->vecSubItems.push_back(stSubItem);
    pSubItem = pItem->getSubItem(dwSceneryID);
    if (pSubItem == nullptr)
      return;
  }

  pSubItem->eStatus = bRealAdd ? EMANUALSTATUS_UNLOCK_CLIENT : EMANUALSTATUS_DISPLAY;
  pItem->dwVersion = xTime::getCurSec();

  if(pSubItem->eStatus != EMANUALSTATUS_DISPLAY)
  {
    ManualUpdate cmd;
    cmd.mutable_update()->set_type(pItem->eType);
    cmd.mutable_update()->set_version(pItem->dwVersion);
    ManualSubItem* subItem = cmd.mutable_update()->add_items();
    pSubItem->toData(subItem);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Manual_PhotoScenery;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);
  PlatLogManager::getMe().ManualLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    EManual_PhotoScenery,
    0,
    0,
    dwSceneryID);

  m_pUser->getQuest().onManualUnlock();
  XLOG << "[冒险手册-场景解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << pSubItem->dwID << "status :" << pSubItem->eStatus << XEND;
}

void Manual::onSceneryUpload(DWORD dwSceneryID, DWORD dwAngleZ, DWORD dwTime)
{
  if (m_pUser == nullptr)
    return;

  const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(dwSceneryID);
  if (pBase == nullptr)
    return;

  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_SCENERY);
  if (pItem == nullptr)
    return;

  SManualSubItem* pSubItem = pItem->getSubItem(dwSceneryID);
  if (pSubItem == nullptr)
    return;

  stringstream angle; angle << dwAngleZ;
  pSubItem->setDataParam(0, angle.str());
  stringstream time; time << dwTime;
  pSubItem->setDataParam(1, time.str());
  pSubItem->setDataParam(2, "");

  pSubItem->bUnlock = true;
  pItem->dwVersion = xTime::getCurSec();

  if(pSubItem->eStatus != EMANUALSTATUS_DISPLAY)
  {
    ManualUpdate cmd;
    cmd.mutable_update()->set_type(pItem->eType);
    cmd.mutable_update()->set_version(pItem->dwVersion);
    ManualSubItem* subItem = cmd.mutable_update()->add_items();
    pSubItem->toData(subItem);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[冒险手册-景点信息]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwSceneryID << "设置angle :" << dwAngleZ << "time :" << dwTime << XEND;
}

void Manual::onMonthCardLock()
{
  if (m_pUser == nullptr)
    return;

  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_COLLECTION);
  if (pItem == nullptr)
    return;

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(EMANUALTYPE_COLLECTION);
  for (auto &v : pItem->vecQuests)
  {
    const SManualQuestCFG* pCFG = ManualConfig::getMe().getManualQuestCFG(v.dwID);
    if (pCFG == nullptr || v.bFinish || pCFG->vecParams.empty() == true)
      continue;
    v.dwProcess = m_pUser->getDeposit().getMonthCardNum();
    v.bFinish = v.dwProcess >= pCFG->vecParams[0];
    XLOG << "[冒险手册-月卡解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "questid :" << v.dwID << "card :" << v.dwProcess << (v.bFinish ? "完成" : "未完成") << XEND;
  }
  pItem->toData(cmd.mutable_update(), true);

  if (cmd.update().items_size() > 0)
  {
    pItem->dwVersion = xTime::getCurSec();
    cmd.mutable_update()->set_version(pItem->dwVersion);

    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void Manual::onPetHatch(DWORD dwPetID, bool bRealAdd)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_PET);
  if (pItem == nullptr)
    return;
  SManualSubItem* pSubItem = pItem->getSubItem(dwPetID);
  if (pSubItem == nullptr)
  {
    SManualSubItem stItem;
    stItem.dwID = dwPetID;
    pItem->vecSubItems.push_back(stItem);
    pSubItem = pItem->getSubItem(dwPetID);
    if (pSubItem == nullptr)
      return;
  }
  if (pSubItem->eStatus >= EMANUALSTATUS_UNLOCK_CLIENT)
    return;

  pSubItem->eStatus = bRealAdd ? EMANUALSTATUS_UNLOCK_CLIENT : EMANUALSTATUS_DISPLAY;
  pItem->dwVersion = xTime::getCurSec();

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(pItem->eType);
  cmd.mutable_update()->set_version(pItem->dwVersion);
  ManualSubItem* subItem = cmd.mutable_update()->add_items();
  pSubItem->toData(subItem);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getEvent().onPetHatch();
  if(pSubItem->eStatus >= EMANUALSTATUS_UNLOCK_CLIENT)
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_OWN_PET);
  XLOG << "[冒险手册-宠物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "更新宠物" << cmd.ShortDebugString() << XEND;
}

EManualStatus Manual::getMapStatus(DWORD dwMapID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MAP);
  if (pItem == nullptr)
    return EMANUALSTATUS_MIN;

  SManualSubItem* pSubItem = pItem->getSubItem(dwMapID);
  if (pSubItem == nullptr)
    return EMANUALSTATUS_MIN;

  return pSubItem->eStatus;
}

EManualStatus Manual::getItemStatus(DWORD dwItemID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_ITEM);
  if (pItem == nullptr)
    return EMANUALSTATUS_MIN;

  SManualSubItem* pSubItem = pItem->getSubItem(dwItemID);
  if (pSubItem == nullptr)
    return EMANUALSTATUS_MIN;

  return pSubItem->eStatus;
}

EManualStatus Manual::getFashionStatus(DWORD dwFashionID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_FASHION);
  if (pItem == nullptr)
    return EMANUALSTATUS_MIN;

  SManualSubItem* pSubItem = pItem->getSubItem(dwFashionID);
  if (pSubItem == nullptr)
    return EMANUALSTATUS_MIN;

  return pSubItem->eStatus;
}

EManualStatus Manual::getMonsterStatus(DWORD dwMonsterID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return EMANUALSTATUS_MIN;

  SManualSubItem* pSubItem = pItem->getSubItem(dwMonsterID);
  if (pSubItem == nullptr)
    return EMANUALSTATUS_MIN;

  return pSubItem->eStatus;
}

EManualStatus Manual::getCollectionStatus(DWORD dwCollectID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_COLLECTION);
  if (pItem == nullptr)
    return EMANUALSTATUS_MIN;

  SManualSubItem* pSubItem = pItem->getSubItem(dwCollectID);
  if (pSubItem == nullptr)
    return EMANUALSTATUS_MIN;

  return pSubItem->eStatus;
}

bool Manual::getMonsterLock(DWORD dwMonsterID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return EMANUALSTATUS_MIN;

  SManualSubItem* pSubItem = pItem->getSubItem(dwMonsterID);
  if (pSubItem == nullptr)
    return EMANUALSTATUS_MIN;

  return pSubItem->bUnlock;
}

/*bool Manual::getAchieveReward(DWORD dwID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_ACHIEVE);
  if (pItem == nullptr)
  {
    XERR << "[冒险手册-成就奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取 id:" << dwID << "成就奖励失败,未获得achieve模块" << XEND;
    return false;
  }

  SManualSubItem* pSubItem = pItem->getSubItem(dwID);
  if (pSubItem == nullptr || pSubItem->eStatus != EMANUALSTATUS_UNLOCK_CLIENT)
  {
    XERR << "[冒险手册-成就奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取 id:" << dwID << "成就奖励失败,不是解锁状态" << XEND;
    return false;
  }

  const SAchieveCFG* pCFG = ManualConfig::getMe().getAchieveCFG(dwID);
  if (pCFG == nullptr || pCFG->vecReward.empty() == true)
  {
    XERR << "[冒险手册-成就奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取 id:" << dwID << "成就奖励失败,未在Table_Achievement表中找到" << XEND;
    return false;
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[冒险手册-成就奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取 id:" << dwID << "成就奖励失败,未在Table_Achievement表中找到" << XEND;
    return false;
  }
  if (!pCFG->vecReward.empty() && pMainPack->checkAddItem(pCFG->vecReward) == false)
  {
    XERR << "[冒险手册-成就奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取 id:" << dwID << "成就奖励失败,超过背包上限" << XEND;
    return false;
  }

  pMainPack->addItemFull(pCFG->vecReward);
  pSubItem->eStatus = EMANUALSTATUS_UNLOCK;
  pItem->dwVersion = xTime::getCurSec();

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(EMANUALTYPE_ACHIEVE);
  cmd.mutable_update()->set_version(pItem->dwVersion);
  ManualSubItem* subItem = cmd.mutable_update()->add_items();
  pSubItem->toData(subItem);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);


  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Manual_AchieveReward;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);
  PlatLogManager::getMe().ManualLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    EManual_AchieveReward,
    0,
    0,
    dwID);

  XLOG << "[冒险手册-获取成就奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwID << XEND;
  return true;
}*/

bool Manual::unlock(EManualType eType, DWORD dwID)
{
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
  {
    XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type :" << eType << "未找到" << XEND;
    return false;
  }

  SManualSubItem* pSubItem = pItem->getSubItem(dwID);
  if (pSubItem == nullptr || pSubItem->eStatus != EMANUALSTATUS_UNLOCK_CLIENT)
  {
    XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "未能解锁" << XEND;
    return false;
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,未找到包裹" << XEND;
    return false;
  }
  TVecItemInfo vecRewardItems;
  if (eType == EMANUALTYPE_FASHION || eType == EMANUALTYPE_CARD || eType == EMANUALTYPE_EQUIP || eType == EMANUALTYPE_ITEM || eType == EMANUALTYPE_MOUNT || eType == EMANUALTYPE_COLLECTION)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwID);
    if (pCFG == nullptr)
    {
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,未在Table_Item.txt表中找到" << XEND;
      return false;
    }

    vecRewardItems = pCFG->vecManualItems;
    for (auto &s : pCFG->setAdvRewardIDs)
    {
      TVecItemInfo vecReward;
      if (RewardManager::roll(s, m_pUser, vecReward, ESOURCE_MANUAL) == false)
      {
        XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败," << s << "随机奖励失败" << XEND;
        return false;
      }
      combinItemInfo(vecRewardItems, vecReward);
    }
    if (!vecRewardItems.empty() && pMainPack->checkAddItem(vecRewardItems, EPACKMETHOD_CHECK_NOPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,奖励超过背包上限" << XEND;
      return false;
    }
  }
  else if (eType == EMANUALTYPE_MONSTER || eType == EMANUALTYPE_NPC || eType == EMANUALTYPE_PET)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwID);
    if (pCFG == nullptr)
    {
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,未在Table_Npc.txt表中找到" << XEND;
      return false;
    }
    if (!pCFG->vecManualItems.empty() && pMainPack->checkAddItem(pCFG->vecManualItems, EPACKMETHOD_CHECK_NOPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,奖励超过背包上限" << XEND;
      return false;
    }
    combinItemInfo(vecRewardItems, pCFG->vecManualItems);
  }
  else if (eType == EMANUALTYPE_MAP)
  {
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(dwID);
    if (pBase == nullptr)
    {
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,未在Table_Map.txt表中找到" << XEND;
      return false;
    }
    if (!pBase->vecManualReward.empty() && pMainPack->checkAddItem(pBase->vecManualReward, EPACKMETHOD_CHECK_NOPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,奖励超过背包上限" << XEND;
      return false;
    }
    combinItemInfo(vecRewardItems, pBase->vecManualReward);
  }
  else if (eType == EMANUALTYPE_SCENERY)
  {
    const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(dwID);
    if (pBase == nullptr)
    {
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,未在Table_Viewspot表中找到" << XEND;
      return false;
    }

    TVecItemInfo vecItems;
    pBase->collectAdvItems(vecItems);
    if (!vecItems.empty() && pMainPack->checkAddItem(vecItems, EPACKMETHOD_CHECK_NOPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << dwID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "解锁失败,奖励超过背包上限" << XEND;
      return false;
    }
    combinItemInfo(vecRewardItems, vecItems);
  }

  if (acceptQuest(eType, dwID, pItem, pSubItem) == false)
  {
    XERR << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type :" << eType << "id :" << dwID << "接取追加失败" << XEND;
    return false;
  }

  bool bHasQuest = ManualConfig::getMe().isShareQuest(eType) == true ? pItem->hasManualQuest() : pSubItem->hasManualQuest();
  pSubItem->eStatus = bHasQuest ? EMANUALSTATUS_UNLOCK_STEP : EMANUALSTATUS_UNLOCK;
  pItem->dwVersion = xTime::getCurSec();

  if (eType == EMANUALTYPE_FASHION || eType == EMANUALTYPE_CARD || eType == EMANUALTYPE_EQUIP || eType == EMANUALTYPE_ITEM || eType == EMANUALTYPE_MOUNT || eType == EMANUALTYPE_COLLECTION)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwID);
    if (pCFG != nullptr)
    {
      if (pCFG->swAdventureValue > 0)
        addPoint(pCFG->swAdventureValue, eType, dwID);

      for (auto v = pCFG->vecAdvBuffIDs.begin(); v != pCFG->vecAdvBuffIDs.end(); ++v)
        m_pUser->m_oBuff.add(*v);
    }
  }
  else if (eType == EMANUALTYPE_MONSTER || eType == EMANUALTYPE_NPC || eType == EMANUALTYPE_PET)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwID);
    if (pCFG != nullptr)
    {
      if (pCFG->swAdventureValue > 0)
        addPoint(pCFG->swAdventureValue, eType, dwID);

      for (auto v = pCFG->vecAdvBuffIDs.begin(); v != pCFG->vecAdvBuffIDs.end(); ++v)
        m_pUser->m_oBuff.add(*v);
    }
  }
  else if (eType == EMANUALTYPE_MAP)
  {
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(dwID);
    if (pBase != nullptr)
    {
      if (pBase->swAdventureValue > 0)
        addPoint(pBase->swAdventureValue, eType, dwID);

      for (auto v = pBase->setManualBuffIDs.begin(); v != pBase->setManualBuffIDs.end(); ++v)
        m_pUser->m_oBuff.add(*v);
    }
  }
  else if (eType == EMANUALTYPE_SCENERY)
  {
    const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(dwID);
    if (pBase != nullptr)
    {
      if (pBase->getAdvectureValue() > 0)
        addPoint(pBase->getAdvectureValue(), eType, dwID);

      TVecDWORD vecIDs;
      pBase->collectAdvBuffID(vecIDs);
      for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
        m_pUser->m_oBuff.add(*v);
    }
  }

  if (vecRewardItems.empty() == false)
    pMainPack->addItem(vecRewardItems, EPACKMETHOD_CHECK_NOPILE);

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(eType);
  cmd.mutable_update()->set_version(pItem->dwVersion);
  if (eType != EMANUALTYPE_COLLECTION)
    pSubItem->toData(cmd.mutable_update()->add_items());
  else
    pItem->toData(cmd.mutable_update(), true);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getQuest().onManualUnlock();
  if (eType == EMANUALTYPE_EQUIP)
    m_pUser->getAchieve().onManual(eType, EACHIEVECOND_MANUAL_EQUIP);
  else if (eType == EMANUALTYPE_SCENERY)
  {
    m_pUser->getAchieve().onManual(eType, EACHIEVECOND_SCENERY_COUNT);
    m_pUser->getServant().onFinishEvent(ETRIGGER_PHOTO_SCENERY);
  }
  else if (eType == EMANUALTYPE_NPC)
    m_pUser->getAchieve().onManual(eType, EACHIEVECOND_NPC_COUNT);
  else if (eType == EMANUALTYPE_COLLECTION)
  {
    m_pUser->getAchieve().onManual(eType, EACHIEVECOND_MANUAL);
    m_pUser->getAchieve().onCollection();
    m_pUser->getQuest().acceptCondQuest(EQUESTCOND_MANUAL);
    refreshGroup();
  }
  else if (eType == EMANUALTYPE_CARD)
    m_pUser->getAchieve().onManual(eType, EACHIEVECOND_MANUAL);
  else if (eType == EMANUALTYPE_FASHION)
    m_pUser->getServant().onFinishEvent(ETRIGGER_PRODUCE_HEAD);

  m_pUser->getMenu().refreshNewMenu();
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_MANUAL_UNLOCK);

  XLOG << "[冒险手册-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type :" << eType << "id :" << dwID << "解锁成功" << XEND;
  return true;
}

bool Manual::getQuestReward(DWORD dwQuestID)
{
  const SManualQuestCFG* pCFG = ManualConfig::getMe().getManualQuestCFG(dwQuestID);
  if (pCFG == nullptr)
    return false;

  SManualItem* pItem = m_stData.getManualItem(pCFG->eManualType);
  if (pItem == nullptr)
    return false;

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(pCFG->eManualType);
  cmd.mutable_update()->set_version(pItem->dwVersion);

  DWORD dwRewardID = 0;
  if (ManualConfig::getMe().isShareQuest(pCFG->eManualType) == true)
  {
    SManualQuest* pQuest = pItem->getManualQuest(dwQuestID);
    if (pQuest == nullptr || pQuest->bRewardGet == true || pQuest->bFinish == false || pQuest->pCFG == nullptr)
      return false;
    dwRewardID = pQuest->pCFG->dwRewardID;
    if (m_pUser->getPackage().rollReward(dwRewardID, EPACKMETHOD_AVAILABLE, true, true) == false)
      return false;

    if (pQuest->pCFG->dwBuffID != 0)
      m_pUser->m_oBuff.add(pQuest->pCFG->dwBuffID);

    pQuest->bRewardGet = true;
    if (acceptQuest(pCFG->eManualType, pCFG->dwTargetID, pItem, nullptr) == false)
      return false;
    if (pItem->isFinishAll(pCFG->eManualType, pCFG->dwTargetID) == true)
    {
      for (auto &item : pItem->vecSubItems)
        item.eStatus = EMANUALSTATUS_UNLOCK;
    }
    pItem->toData(cmd.mutable_update(), true);
  }
  else
  {
    SManualSubItem* pSubItem = pItem->getSubItem(pCFG->dwTargetID);
    if (pSubItem == nullptr)
    {
      XERR << "[冒险手册-领取追加奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "领取了 type :" << pCFG->eManualType << "id :" << pCFG->dwTargetID << "questid :" << dwQuestID << "reward :" << dwRewardID << "奖励失败,id未发现" << XEND;
      return false;
    }
    SManualQuest* pQuest = pSubItem->getManualQuest(dwQuestID);

    if (pQuest == nullptr || pQuest->bRewardGet == true || pQuest->bFinish == false || pQuest->pCFG == nullptr)
      return false;

    dwRewardID = pQuest->pCFG->dwRewardID;
    if (m_pUser->getPackage().rollReward(dwRewardID, EPACKMETHOD_AVAILABLE, true, true) == false)
      return false;

    if (pQuest->pCFG->dwBuffID != 0)
      m_pUser->m_oBuff.add(pQuest->pCFG->dwBuffID);

    pQuest->bRewardGet = true;
    if (acceptQuest(pCFG->eManualType, pCFG->dwTargetID, pItem, pSubItem) == false)
      return false;
    if (pItem->isFinishAll(pCFG->eManualType, pCFG->dwTargetID) == true)
      pSubItem->eStatus = EMANUALSTATUS_UNLOCK;
    pSubItem->toData(cmd.mutable_update()->add_items());
  }

  pItem->dwVersion = xTime::getCurSec();

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[冒险手册-领取追加奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "领取了 type :" << pCFG->eManualType << "id :" << pCFG->dwTargetID << "questid :" << dwQuestID << "reward :" << dwRewardID << "奖励" << XEND;
  return true;
}

bool Manual::removeItem(EManualType eType, DWORD dwID)
{
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
  {
    XERR << "[冒险手册-删除选项]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "删除" << eType << dwID << "失败,未发现该类型" << XEND;
    return false;
  }

  auto v = find_if(pItem->vecSubItems.begin(), pItem->vecSubItems.end(), [&](const SManualSubItem& rItem) -> bool{
    return rItem.dwID == dwID;
  });
  if (v == pItem->vecSubItems.end())
  {
    XERR << "[冒险手册-删除选项]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "删除" << eType << dwID << "失败,未发现该id" << XEND;
    return false;
  }

  pItem->vecSubItems.erase(v);
  XLOG << "[冒险手册-删除选项]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "删除" << eType << dwID << "成功" << XEND;
  return true;
}

// temp patch
void Manual::patch1()
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return;

  for (auto &v : pItem->vecSubItems)
  {
    if (v.eStatus != EMANUALSTATUS_UNLOCK_STEP)
      continue;

    SManualSubItem& rItem = v;
    rItem.vecQuests.clear();
    acceptQuest(EMANUALTYPE_MONSTER, rItem.dwID, pItem, &rItem);
  }
}

// temp patch2, give rewards by photo monsters
void Manual::patch2()
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return;

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(pItem->eType);
  cmd.mutable_update()->set_version(pItem->dwVersion);

  for (auto &v : pItem->vecSubItems)
  {
    if (!v.bUnlock)
      continue;
    TVecManualQuestCFG vecCFG;
    ManualConfig::getMe().collectQuest(EMANUALTYPE_MONSTER, v.dwID, vecCFG);
    for (auto &s : vecCFG)
    {
      if (s.eQuestType != EMANUALQUEST_PHOTO)
        continue;
      SManualQuest* pQuest = v.getManualQuest(s.dwID);
      if (pQuest == nullptr)
      {
        SManualQuest stQuest;
        stQuest.dwID = s.dwID;
        stQuest.pCFG = ManualConfig::getMe().getManualQuestCFG(s.dwID);
        v.vecQuests.push_back(stQuest);
      }
      pQuest = v.getManualQuest(s.dwID);
      if (pQuest == nullptr)
        continue;
      pQuest->bRewardGet = false;
      pQuest->bFinish = true;
      XLOG << "[冒险手册-patch], 添加拍照可领取拍照任务" << m_pUser->name << m_pUser->id << "完成的拍照任务:" << pQuest->dwID << XEND;
    }

    ManualSubItem* subItem = cmd.mutable_update()->add_items();
    if (subItem)
      v.toData(subItem);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Manual::patch3()
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return;

  for (auto &v : pItem->vecSubItems)
  {
    TVecManualQuestCFG vecCFG;
    ManualConfig::getMe().collectQuest(EMANUALTYPE_MONSTER, v.dwID, vecCFG);
    DWORD dwIndex = 0;
    for (auto &s : vecCFG)
    {
      if (dwIndex++ != 2)
        continue;
      SManualQuest* pQuest = v.getManualQuest(s.dwID);
      if (pQuest == nullptr || !pQuest->bFinish)
        continue;
      TVecItemInfo vecItems;
      if (RewardManager::roll(s.dwRewardID, m_pUser, vecItems, ESOURCE_GM) == true)
      {
        for (auto &v : vecItems)
        {
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.id());
          if (pCFG == nullptr)
            continue;

          if (pCFG->eItemType == EITEMTYPE_PORTRAIT || pCFG->eItemType == EITEMTYPE_FRAME)
            m_pUser->getPortrait().addNewItems(v.id(), false);
          else
            m_pUser->getPackage().addItem(v, EPACKMETHOD_AVAILABLE, false, false, false);
        }
        //m_pUser->getPackage().rollReward(s.dwRewardID, false, false);
        XLOG << "[冒险手册-patch3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获得 reward :" << s.dwRewardID << XEND;
      }
    }
  }
}

void Manual::sendAllVersion()
{
  QueryVersion cmd;

  for (auto v = m_stData.vecItems.begin(); v != m_stData.vecItems.end(); ++v)
    v->toData(cmd.add_versions());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Manual::sendPoint()
{
  if (m_pUser == nullptr)
    return;

  PointSync cmd;
  cmd.set_point(m_stData.dwPoint);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Manual::sendSkillPoint()
{
  if (m_pUser == nullptr)
    return;

  SkillPointSync cmd;
  cmd.set_skillpoint(m_stCharData.dwSkillPoint);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Manual::sendLevel()
{
  if (m_pUser == nullptr)
    return;

  LevelSync cmd;
  cmd.set_level(m_stData.dwLevel);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Manual::refreshGroup()
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_COLLECTION);
  if (pItem == nullptr)
  {
    XERR << "[玩家-冒险手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活组失败,未找到珍藏品选项卡" << XEND;
    return;
  }

  TVecItemInfo vecRewards;
  const TMapManualGroupCFG& mapGroups = ManualConfig::getMe().getManualGroupList();
  for (auto &m : mapGroups)
  {
    auto item = m_stData.mapGroups.find(m.first);
    if (item != m_stData.mapGroups.end())
      continue;

    bool bSuccess = true;
    for (auto &s : m.second.setManualIDs)
    {
      SManualSubItem* pSubItem = pItem->getSubItem(s);
      if (pSubItem == nullptr || pSubItem->eStatus < EMANUALSTATUS_UNLOCK)
      {
        bSuccess = false;
        break;
      }
    }
    if (!bSuccess)
      continue;

    bSuccess = true;
    for (auto &s : m.second.setRewardIDs)
    {
      TVecItemInfo vecItems;
      if (RewardManager::roll(s, m_pUser, vecItems, ESOURCE_MANUAL) == false)
      {
        bSuccess = false;
        XERR << "[玩家-冒险手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活组 :" << m.first << "失败,随机rewardid :" << s << "失败" << XEND;
        break;
      }
      combinItemInfo(vecRewards, vecItems);
    }
    if (!bSuccess)
      break;

    SManualGroup& rGroup = m_stData.mapGroups[m.first];
    rGroup.dwID = m.first;
    m_pUser->setCollectMark(ECOLLECTTYPE_ACHIEVEMENT);
    XLOG << "[玩家-冒险手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "激活组 :" << m.first << "成功" << XEND;
  }

  if (vecRewards.empty() == false)
  {
    for (auto &v : vecRewards)
      v.set_source(ESOURCE_MANUAL);
    m_pUser->getPackage().addItem(vecRewards, EPACKMETHOD_NOCHECK);
  }
}

/*void Manual::onPointChange(bool bNotify = true)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_ACHIEVE);
  if (pItem == nullptr)
    return;

  ManualUpdate cmd;
  cmd.mutable_update()->set_type(EMANUALTYPE_ACHIEVE);

  DWORD dwNow = xTime::getCurSec();
  DWORD dwAllPoint = 0;
  const TVecAchieveCFG& vecList = ManualConfig::getMe().getAchieveList(EACHIEVETYPE_VALUE);
  for (auto v = vecList.begin(); v != vecList.end(); ++v)
  {
    const SAchieveCFG& rCFG = *v;
    SManualSubItem* pSubItem = pItem->getSubItem(rCFG.dwID);
    if (pSubItem != nullptr)
      continue;
    if (checkAchieve(rCFG) == false)
      continue;

    SManualSubItem stSubItem;
    stSubItem.dwID = rCFG.dwID;
    stSubItem.eStatus = rCFG.vecReward.empty() == true ? EMANUALSTATUS_UNLOCK : EMANUALSTATUS_UNLOCK_CLIENT;

    pItem->vecSubItems.push_back(stSubItem);
    pItem->dwVersion = dwNow;

    dwAllPoint += rCFG.dwAdventureValue;

    ManualSubItem* pSub = cmd.mutable_update()->add_items();
    stSubItem.toData(pSub);

    if (rCFG.dwRewardMailID != 0 && MailManager::getMe().sendMail(m_pUser->id, rCFG.dwRewardMailID) == false)
      XERR << "[冒险手册-积分变化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "unlock achieve :" << rCFG.dwID << "mailid :" << rCFG.dwRewardMailID << "send error!" << XEND;

    XLOG << "[冒险手册-积分变化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "unlock achieve :" << rCFG.dwID << "open!" << XEND;
  }

  if (bNotify && cmd.update().items_size() > 0)
  {
    cmd.mutable_update()->set_version(pItem->dwVersion);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}*/

/*bool Manual::checkAchieve(const SAchieveCFG& rCFG)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_ACHIEVE);
  if (pItem == nullptr)
    return false;

  for (auto v = rCFG.vecPreIDs.begin(); v != rCFG.vecPreIDs.end(); ++v)
  {
    if (pItem->getSubItem(*v) == nullptr)
      return false;
  }

  for (auto v = rCFG.vecCondition.begin(); v != rCFG.vecCondition.end(); ++v)
  {
  }

  return false;
}*/

bool Manual::acceptQuest(EManualType eType, DWORD dwID, SManualItem* pItem, SManualSubItem* pSubItem)
{
  if (pItem == nullptr)
    return false;

  if (ManualConfig::getMe().isShareQuest(eType) == true)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwID);
    if (pCFG != nullptr)
      dwID = pCFG->eItemType;
  }

  TVecManualQuestCFG vecCFG;
  ManualConfig::getMe().collectQuest(eType, dwID, vecCFG);

  for (auto &v : vecCFG)
  {
    SManualQuest* pQuest = nullptr;
    if (ManualConfig::getMe().isShareQuest(eType) == true)
    {
      pQuest = pItem->getManualQuest(v.dwID);
      if (pQuest != nullptr)
        continue;
      if (pItem->checkPreQuest(v.setPreIDs) == false)
        continue;
    }
    else
    {
      if (pSubItem == nullptr)
        continue;
      pQuest = pSubItem->getManualQuest(v.dwID);
      if (pQuest != nullptr)
        continue;
      if (pSubItem->checkPreQuest(v.setPreIDs) == false)
        continue;
    }

    SManualQuest stQuest;
    stQuest.dwID = v.dwID;
    stQuest.pCFG = ManualConfig::getMe().getManualQuestCFG(v.dwID);
    if (ManualConfig::getMe().isShareQuest(eType) == true)
    {
      stQuest.dwProcess = m_pUser->getDeposit().getMonthCardNum();
      stQuest.bFinish = stQuest.dwProcess >= v.vecParams[0];
    }

    if (ManualConfig::getMe().isShareQuest(eType) == true)
    {
      pItem->vecQuests.push_back(stQuest);
    }
    else
    {
      if (pSubItem != nullptr)
        pSubItem->vecQuests.push_back(stQuest);
    }

    XLOG << "[冒险手册-任务接取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "接取" << stQuest.dwID << v.eQuestType << stQuest.bFinish << stQuest.bRewardGet << XEND;
  }

  return true;
}

//bool Manual::checkLock(EManualLockMethod eMethod, bool bRealAdd, bool bCompose, bool bPickup, bool bKill, bool bUsed, bool bDraw)
bool Manual::checkLock(EManualLockMethod eMethod, bool bRealAdd, bool bKill, bool bUsed, ESource source)
{
  bool bOK = false;
  switch (eMethod)
  {
    case EMANUALLOCKMETHOD_MIN:
      break;
    case EMANUALLOCKMETHOD_KILL:
      bOK = bKill;
      break;
    case EMANUALLOCKMETHOD_PICK:
      {
        bOK = (source == ESOURCE_PICKUP || source == ESOURCE_TOWER);
      }
      break;
    case EMANUALLOCKMETHOD_PRODUCE:
      {
        bOK = (source == ESOURCE_COMPOSE);
      }
      break;
    case EMANUALLOCKMETHOD_GET:
      {
        bOK = bRealAdd;
      }
      break;
    case EMANUALLOCKMETHOD_VISIT:
      bOK = bKill;
      break;
    case EMANUALLOCKMETHOD_PHOTO:
      bOK = bKill;
      break;
    case EMANUALLOCKMETHOD_USED:
      {
        bOK = bUsed;
      }
      break;
    case EMANUALLOCKMETHOD_DRAW:
      {
        bOK = (source == ESOURCE_EXCHANGECARD);
      }
      break;
    case EMANUALLOCKMETHOD_MAX:
      break;
  }

  if (source == ESOURCE_PET_ADVENTURE || source == ESOURCE_MAX || Lottery::isLotterySource(source))
    bOK = true;
  return bOK;
}

bool Manual::unsolvedPhoto(const UpdateSolvedPhotoManualCmd& cmd)
{
  auto m = m_mapUser2Photo.find(cmd.charid());
  if (m == m_mapUser2Photo.end())
  {
    XERR << "[冒险手册-临时相册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "处理失败,未找到 charid :" << cmd.charid() << "临时照片" << XEND;
    return false;
  }

  SUnsovledUserPhoto& rPhoto = m->second;
  auto photo = rPhoto.mapPhotos.find(cmd.sceneryid());
  if (photo == rPhoto.mapPhotos.end())
  {
    XERR << "[冒险手册-临时相册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "处理失败,未找到 charid :" << cmd.charid() << "sceneryid :" << cmd.sceneryid() << "临时照片" << XEND;
    return false;
  }

  rPhoto.mapPhotos.erase(photo);
  if (rPhoto.mapPhotos.empty() == true)
    m_mapUser2Photo.erase(m);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[冒险手册-临时相册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "处理" << cmd.ShortDebugString() << "成功" << XEND;
  return true;
}

void Manual::addPoint(DWORD dwPoint, EManualType eType, DWORD dwID, bool bNotify /*= true*/)
{
  if (dwPoint == 0 || m_pUser == nullptr)
    return;

  m_stData.dwPoint += dwPoint;
  m_pUser->setDataMark(EUSERDATATYPE_MANUAL_EXP);
  //onPointChange();
  levelup();

  if (bNotify)
    sendPoint();

  XLOG << "[冒险手册-积分增加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获得" << dwPoint << "积分 总积分为" << m_stData.dwPoint << XEND;

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE evType = EventType_Manual_Exp;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), evType, 0, 1);

  PlatLogManager::getMe().ManualLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    evType,
    eid,
    EManual_Exp,
    eType,   /*by*/
    dwID,
    dwPoint);

}

void Manual::levelup()
{
  if (m_pLvCFG == nullptr)
    return;

  bool bSuccess = false;
  DWORD dwOldLv = m_stData.dwLevel;
  while (true)
  {
    const SManualLvCFG* pNextCFG = ManualConfig::getMe().getManualLvCFG(m_stData.dwLevel + 1);
    if (pNextCFG == nullptr)
      break;

    if (m_pLvCFG->dwNeedExp > m_stData.dwPoint)
      break;
    m_stData.dwPoint -= m_pLvCFG->dwNeedExp;
    m_stData.dwLevel += 1;
    addSkillPoint(m_pLvCFG->dwSkillPoint);
    bSuccess = true;
    XLOG << "[冒险手册-升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "升级成功 level :" << m_stData.dwLevel << "exp :" << m_stData.dwPoint << XEND;

    m_pLvCFG = pNextCFG;
  }

  if (bSuccess)
  {
    m_pUser->getQuest().acceptNewQuest();
    m_pUser->setDataMark(EUSERDATATYPE_MANUAL_LV);
    sendLevel();
    m_pUser->setCollectMark(ECOLLECTTYPE_ACHIEVEMENT);
    m_pUser->updateAttribute();
    m_pUser->getEvent().onManualLevelup();

    PlatLogManager::getMe().levelUpLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(), 
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      dwOldLv,
      m_stData.dwLevel,
      m_pUser->getUserSceneData().getCharge(),
      ELevelType_ManualLv
    );
  }
}

void Manual::version_update()
{
  for (DWORD d = m_stData.dwVersion; d < MANUAL_VERSION; ++d)
  {
    if (d == 0)
      version_1();
    else if (d == 1)
      version_2();
    else if (d == 2)
      version_3();
  }
  m_stData.dwVersion = MANUAL_VERSION;
}

void Manual::version_1()
{
  if (m_pUser->getUserSceneData().getOfflineTime()  != 0 && m_stData.dwLevel != m_stCharData.dwLevel && m_stData.dwExchangeTime > m_pUser->getUserSceneData().getCreateTime())
  {
    DWORD dwCurPoint = 0;
    DWORD dwOldPoint = 0;
    const TMapManualLvCFG& mapCFG = ManualConfig::getMe().getManualLvCFG();
    for (auto &m : mapCFG)
    {
      if (m_stCharData.dwLevel >= m.second.dwLevel)
        dwOldPoint += m.second.dwSkillPoint;
      if (m_stData.dwLevel >= m.second.dwLevel)
        dwCurPoint += m.second.dwSkillPoint;
    }
    if (dwCurPoint > dwOldPoint)
    {
      DWORD dwAdd = dwCurPoint - dwOldPoint;
      const SManualMiscCFG& rCFG = MiscConfig::getMe().getManualCFG();
      const MailBase* pBase = TableManager::getMe().getMailCFG(rCFG.dwSkillReturnMailID);
      if (pBase != nullptr)
      {
        MailManager::getMe().sendMail(m_pUser->id, pBase->id, TVecItemInfo{}, MsgParams(m_stData.dwLevel, (dwAdd > 0 ? dwAdd - 1 : dwAdd)), true, false);
        XLOG << "[冒险手册-版本-char-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "技能点补偿邮件 补偿" << (dwAdd > 0 ? dwAdd - 1 : dwAdd) << "点" << XEND;
      }
      else
      {
        XERR << "[冒险手册-版本-char-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "技能点补偿邮件" << rCFG.dwSkillReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
      }
    }
  }

  XLOG << "[冒险手册-版本-char-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Manual::version_2()
{
  const DWORD FASHION1 = 51021;
  const DWORD FASHION2 = 51031;

  bool b51021 = false;
  bool b51031 = false;

  set<EPackType> setTypes = set<EPackType>{EPACKTYPE_MAIN, EPACKTYPE_EQUIP, EPACKTYPE_FASHIONEQUIP, EPACKTYPE_STORE, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_TEMP_MAIN, EPACKTYPE_BARROW};
  for (auto &s : setTypes)
  {
    BasePackage* pPack = m_pUser->getPackage().getPackage(s);
    if (pPack == nullptr)
    {
      XERR << "[冒险手册-版本-2]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行失败,未获得" << s << XEND;
      continue;
    }
    if (!b51021)
      b51021 = pPack->getItemCount(FASHION1) > 0;
    if (!b51031)
      b51031 = pPack->getItemCount(FASHION2) > 0;
  }

  if (b51021)
    onItemAdd(FASHION1, true, true, true, ESOURCE_EXCHANGECARD);
  if (b51031)
    onItemAdd(FASHION2, true, true, true, ESOURCE_EXCHANGECARD);

  XLOG << "[冒险手册-版本-2]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Manual::version_3()
{
  for (auto &v : m_stData.vecItems)
  {
    for (auto &sub : v.vecSubItems)
    {
      if (!sub.bStore)
        continue;
      sub.oItem.mutable_base()->set_id(sub.dwStoreID != 0 ? sub.dwStoreID : sub.dwID);
      XLOG << "[冒险手册-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << sub.dwID << "道具" << sub.oItem.base().id() << "存储道具转移到item中" << XEND;
    }
  }
  XLOG << "[冒险手册-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

#include "SkillManager.h"
void Manual::calcSkillPoint(bool bSync /*= false*/)
{
  SceneFighter* pNoviceFighter = m_pUser->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter == nullptr)
  {
    XERR << "[冒险手册-技能点计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "计算失败,未找到" << EPROFESSION_NOVICE << XEND;
    return;
  }

  DWORD dwUsedPoint = 0;
  const TSetDWORD& setIDs = ShopConfig::getMe().getManualSkillList();
  for (auto &s : setIDs)
  {
    const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(s);
    if (pSkillCFG == nullptr)
    {
      XERR << "[冒险手册-技能点计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "计算 skillid :" << s << "失败" << XEND;
      continue;
    }
    if (pNoviceFighter->getSkill().isSkillEnable(s) == false)
      continue;
    dwUsedPoint += pSkillCFG->getLevelCost();
  }

  DWORD dwTotalPoint = 0;
  for (DWORD lv = 1; lv <= m_stData.dwLevel; ++lv)
  {
    const SManualLvCFG* pCFG = ManualConfig::getMe().getManualLvCFG(lv);
    if (pCFG == nullptr)
    {
      XERR << "[冒险手册-技能点计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "采集技能点失败, lv :" << lv << "未在 Table_AdventureLevel.txt 表中找到" << XEND;
      continue;
    }

    dwTotalPoint += pCFG->dwSkillPoint;
  }

  if (dwUsedPoint > dwTotalPoint)
  {
    XERR << "[冒险手册-技能点计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "计算失败, 消耗" << dwUsedPoint << "大于总" << dwTotalPoint << XEND;
    return;
  }

  m_stCharData.dwSkillPoint = dwTotalPoint - dwUsedPoint;
  if (bSync)
    sendSkillPoint();
  XDBG << "[冒险手册-技能点计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "计算成功, 消耗" << dwUsedPoint << "总" << dwTotalPoint << "当前有" << m_stCharData.dwSkillPoint << "点" << XEND;
}

DWORD Manual::getNumByStatus(EManualType eType, EManualStatus eStatus, ENpcType eMonster /*= ENPCTYPE_MIN*/, EItemType eItemType /*= EITEMTYPE_MIN*/)
{
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
    return 0;

  DWORD num = 0;
  if (eType == EMANUALTYPE_MONSTER)
  {
    for (auto &v : pItem->vecSubItems)
    {
      if (v.eStatus < eStatus)
        continue;
      const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(v.dwID);
      if (pCFG == nullptr)
        continue;
      if (eMonster != ENPCTYPE_MIN && pCFG->eNpcType != eMonster)
        continue;
      num++;
    }
  }
  else if (eType == EMANUALTYPE_COLLECTION)
  {
    const SAchieveMiscCFG& rCFG = MiscConfig::getMe().getAchieveCFG();
    for (auto &v : pItem->vecSubItems)
    {
      if (rCFG.isExclude(v.dwID) == true)
        continue;
      if (v.eStatus < eStatus)
        continue;
      const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.dwID);
      if (pCFG == nullptr)
        continue;
      if (eItemType != EITEMTYPE_MIN && pCFG->eItemType != eItemType)
        continue;
      ++num;
    }
  }
  else
  {
    for (auto &v : pItem->vecSubItems)
    {
      if (v.eStatus >= eStatus)
        num++;
    }
  }

  return num;
}

DWORD Manual::getSceneryUnlockCount(DWORD dwMapID)
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_SCENERY);
  if (pItem == nullptr)
    return 0;

  DWORD dwCount = 0;
  for (auto &v : pItem->vecSubItems)
  {
    const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(v.dwID);
    if (pBase == nullptr || pBase->getMapID() != dwMapID)
      continue;
    ++dwCount;
  }
  return dwCount;
}

DWORD Manual::getMonsterPhotoNum()
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_MONSTER);
  if (pItem == nullptr)
    return 0;

  DWORD dwCount = 0;
  for (auto &v : pItem->vecSubItems)
  {
    if (v.bUnlock)
      ++dwCount;
  }

  return dwCount;
}

bool Manual::storeItem(EManualType eType, const string& guid)
{
  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[冒险手册-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "guid :" << guid << "存取失败,未找到包裹" << XEND;
    return false;
  }

  ItemBase* pBase = pMainPack->getItem(guid);
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr || pBase == nullptr)
  {
    XERR << "[冒险手册-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type :" << eType << "未找到" << XEND;
    return false;
  }

  DWORD itemId = pBase->getTypeID();
  SManualSubItem* pSubItem = nullptr;
  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[冒险手册-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << itemId << "未在 Table_Item.txt 表中找到" << XEND;
    return false;
  }
  if (pCFG->dwGroupID != 0)
    pSubItem = pItem->getSubItem(pCFG->dwGroupID);
  else
    pSubItem = pItem->getSubItem(itemId);

  if (pSubItem == nullptr)
  {
    XERR << "[冒险手册-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << itemId << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "未能解锁" << XEND;
    return false;
  }

  if(pSubItem->bStore == true)
  {
    XERR << "[冒险手册-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << itemId << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "存取失败,已经存储" << XEND;
    return false;
  }

  ItemData oData;
  pBase->toItemData(&oData);
  oData.mutable_base()->set_count(1);
  if (eType == EMANUALTYPE_FASHION)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip == nullptr || pEquip->EquipedCard() == true || pEquip->getLv() > 0 || /*pEquip->getEnchantData().type() != EENCHANTTYPE_MIN ||*/ pEquip->getStrengthLv() > 0)
    {
      MsgManager::sendMsg(m_pUser->id, 549);
      XERR << "[冒险手册-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "type :" << eType << "id :" << itemId << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "存取失败,未找到物品配置" << XEND;
      return false;
    }

    if (pEquip && pEquip->getEnchantData().type() != EENCHANTTYPE_MIN && MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PVE))
      return false;

    pMainPack->reduceItem(guid,  ESOURCE_MANUAL);
  }
  else if (eType == EMANUALTYPE_CARD)
  {
    pMainPack->reduceItem(guid, ESOURCE_MANUAL);
  }
  else
    return false;

  pSubItem->bStore = true;

  pSubItem->oItem.Clear();
  pSubItem->oItem.mutable_base()->set_id(oData.base().id());
  pSubItem->oItem.mutable_base()->set_count(oData.base().count());
  pSubItem->oItem.mutable_equip()->CopyFrom(oData.equip());
  pSubItem->oItem.mutable_refine()->CopyFrom(oData.refine());
  pSubItem->oItem.mutable_sender()->CopyFrom(oData.sender());
  pSubItem->oItem.mutable_enchant()->CopyFrom(oData.enchant());

  if (pCFG->dwGroupID != 0)
    pSubItem->dwStoreID = pCFG->dwTypeID;

  for (auto v = pCFG->vecStoreBuffIDs.begin(); v != pCFG->vecStoreBuffIDs.end(); ++v)
    m_pUser->m_oBuff.add(*v);

  pItem->dwVersion = xTime::getCurSec();
  ManualUpdate cmd;
  cmd.mutable_update()->set_type(eType);
  cmd.mutable_update()->set_version(pItem->dwVersion);
  if (eType != EMANUALTYPE_COLLECTION)
    pSubItem->toData(cmd.mutable_update()->add_items());
  else
    pItem->toData(cmd.mutable_update(), true);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[冒险手册-存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "存储" << oData.ShortDebugString() << "到" << eType << "成功,手册中道具" << pSubItem->oItem.ShortDebugString() << XEND;
  return true;
}

bool Manual::getItem(EManualType eType, DWORD dwItemID)
{
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
  {
    XERR << "[冒险手册-取出]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type :" << eType << "未找到" << XEND;
    return false;
  }

  SManualSubItem* pSubItem = pItem->getSubItem(dwItemID);
  if (pSubItem == nullptr)
  {
    XERR << "[冒险手册-取出]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << dwItemID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "未找到" << XEND;
    return false;
  }

  if(pSubItem->bStore == false)
  {
    XERR << "[冒险手册-取出]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << dwItemID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "未找到" << XEND;
    return false;
  }

  if (pSubItem->dwStoreID != 0)
  {
    XLOG << "[冒险手册-取出]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << dwItemID << "为特有存储,itemid设置为storeid :" << pSubItem->dwStoreID << XEND;
    dwItemID = pSubItem->dwStoreID;
  }
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
  if(pCFG == nullptr)
  {
    XERR << "[冒险手册-取出]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << dwItemID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "存取失败,不能存储" << XEND;
    return false;
  }

  /*ItemInfo stInfo;
  stInfo.set_id(pSubItem->dwStoreID == 0 ? dwItemID : pSubItem->dwStoreID);
  stInfo.set_count(1);
  stInfo.set_source(ESOURCE_MANUAL);*/
  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if(pMainPack != nullptr && pMainPack->checkAddItem(pSubItem->oItem, EPACKMETHOD_CHECK_WITHPILE) == true)
  {
    //pMainPack->addItemFull(stInfo);
    pMainPack->addItem(pSubItem->oItem, EPACKMETHOD_CHECK_WITHPILE);
    pSubItem->bStore = false;
    pSubItem->dwStoreID = 0;
    pSubItem->oItem.Clear();

    DWORD dwNow = xTime::getCurSec();
    m_pUser->getPackage().timer(dwNow);
  }
  else
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
    XERR << "[冒险手册-取出]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "type :" << eType << "id :" << dwItemID << "status :" << (pSubItem == nullptr ? EMANUALSTATUS_MIN : pSubItem->eStatus) << "存取失败,存入包裹失败" << XEND;
    return false;
  }

  for (auto v = pCFG->vecStoreBuffIDs.begin(); v != pCFG->vecStoreBuffIDs.end(); ++v)
    m_pUser->m_oBuff.del(*v);

  pItem->dwVersion = xTime::getCurSec();
  ManualUpdate cmd;
  cmd.mutable_update()->set_type(eType);
  cmd.mutable_update()->set_version(pItem->dwVersion);
  if (eType != EMANUALTYPE_COLLECTION)
    pSubItem->toData(cmd.mutable_update()->add_items());
  else
    pItem->toData(cmd.mutable_update(), true);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getUserPet().onManualOffEquip(dwItemID);

  XLOG << "[冒险手册-取出]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type :" << eType << "id :" << dwItemID << "取出成功" << XEND;
  return true;
}
DWORD Manual::getMonthCard()
{
  SManualItem* pItem = m_stData.getManualItem(EMANUALTYPE_COLLECTION);
  if (pItem == nullptr)
    return 0;

  DWORD count = 0;
  for (auto &item : pItem->vecSubItems)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(item.dwID);
    if (pCFG == nullptr ||  pCFG->eItemType != EITEMTYPE_MONTHCARD)
      continue;

    if(item.eStatus != EMANUALSTATUS_UNLOCK)
      continue;

    count++;
  }

  return count;
}

bool Manual::delManualItem(EManualType eType, DWORD dwItemID)
{
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
  {
    return false;
  }

  SManualSubItem* pSubItem = pItem->getSubItem(dwItemID);
  if(pSubItem == nullptr)
    return false;
  if(pSubItem->bStore == true)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
    if(pCFG == nullptr)
      return false;
    for (auto v = pCFG->vecStoreBuffIDs.begin(); v != pCFG->vecStoreBuffIDs.end(); ++v)
      m_pUser->m_oBuff.del(*v);
  }

  bool ret = pItem->delSubItem(dwItemID);
  if (ret == true)
  {
    sendManualData();
    XLOG << "[冒险手册-GM删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwItemID << "type :" << eType << XEND;
    return true;
  }

  return false;
}

bool Manual::addManualItem(EManualType eType, DWORD dwItemID, EManualStatus eStatus, bool bCheck)
{
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
  {
    XERR << "[冒险手册-GM添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加失败,未找到" << eType << "选项卡" << XEND;
    return false;
  }

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(dwItemID);
  if (pCFG == nullptr || ItemConfig::getMe().canEnterManual(pCFG) == false)
  {
    XERR << "[冒险手册-GM添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "添加失败,itemid" << dwItemID << "未在 Table_Item.txt 表中找到或者未能入册" << XEND;
    return false;
  }

  EManualStatus preStatus = EMANUALSTATUS_MIN;
  SManualSubItem* pSubItem = pItem->getSubItem(dwItemID);
  if(pSubItem != nullptr)
    preStatus = pSubItem->eStatus;

  if(bCheck == true && preStatus >= eStatus)
    return false;

  bool ret = pItem->addSubItem(dwItemID, eStatus);
  if (ret)
  {
    SManualSubItem* pSubItem = pItem->getSubItem(dwItemID);
    if (pSubItem != nullptr)
    {
      ManualUpdate cmd;
      cmd.mutable_update()->set_type(eType);
      cmd.mutable_update()->set_version(pItem->dwVersion);
      pSubItem->toData(cmd.mutable_update()->add_items());
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
    if(preStatus == EMANUALSTATUS_UNLOCK)
    {
      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
      if (pCFG != nullptr)
      {
        for (auto v = pCFG->vecAdvBuffIDs.begin(); v != pCFG->vecAdvBuffIDs.end(); ++v)
          m_pUser->m_oBuff.del(*v);
      }
    }
  }
  else
    return false;

  XLOG << "[冒险手册-GM添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwItemID << "type :" << eType << "status :" << eStatus <<"ret"<<ret << XEND;
  return true;
}

bool Manual::groupAction(EGroupAction eAction, DWORD dwGroupID)
{
  if (eAction <= EGROUPACTION_MIN || eAction >= EGROUPACTION_MAX || EGroupAction_IsValid(eAction) == false)
  {
    XERR << "[冒险手册-组操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行" << dwGroupID << eAction << "失败,非法的action" << XEND;
    return false;
  }

  auto m = m_stData.mapGroups.find(dwGroupID);
  if (m == m_stData.mapGroups.end())
  {
    XERR << "[冒险手册-组操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行" << dwGroupID << eAction << "失败,该租内物品未全部激活" << XEND;
    return false;
  }

  const SManualGroupCFG* pCFG = ManualConfig::getMe().getManualGroupCFG(dwGroupID);
  if (pCFG == nullptr)
  {
    XERR << "[冒险手册-组操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行" << dwGroupID << eAction << "失败,该组未在 Table_CollectionGroup.txt 表中找到" << XEND;
    return false;
  }

  Quest& rQuest = m_pUser->getQuest();
  if (rQuest.abandonGroup(pCFG->dwQuestID) == false)
  {
    XERR << "[冒险手册-组操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行" << dwGroupID << eAction << "失败,任务放弃失败" << XEND;
    return false;
  }

  rQuest.acceptQuest(pCFG->dwQuestID);
  XLOG << "[冒险手册-组操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行" << dwGroupID << eAction << "成功" << XEND;
  return true;
}

//void Manual::collectAttr(TVecAttrSvrs& attrs)
void Manual::collectAttr()
{
  Attribute* pAttr = m_pUser->getAttribute();
  if (pAttr == nullptr)
    return;

  map<EAttrType, UserAttrSvr> mapAttrs;
  for (auto &m : m_stData.mapGroups)
  {
    const SManualGroupCFG* pCFG = ManualConfig::getMe().getManualGroupCFG(m.first);
    if (pCFG == nullptr)
    {
      XERR << "[冒险手册-属性]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "groupid :" << m.first << "未在 Table_Collection.txt 表中找到" << XEND;
      continue;
    }
    for (auto &attr : pCFG->mapAttrs)
    {
      UserAttrSvr& rAttr = mapAttrs[attr.first];
      rAttr.set_type(attr.first);
      rAttr.set_value(rAttr.value() + attr.second.value());
    }
  }
  if (m_pLvCFG != nullptr)
  {
    for (auto &attr : m_pLvCFG->mapAttrs)
    {
      UserAttrSvr& rAttr = mapAttrs[attr.first];
      rAttr.set_type(attr.first);
      rAttr.set_value(rAttr.value() + attr.second.value());
    }
  }
  else
  {
    XERR << "[冒险手册-属性]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "采集等级属性失败 lv :" << m_stCharData.dwLevel << "未在 Table_AdventureLevel.txt 表中找到" << XEND;
  }

  for (auto &m : mapAttrs)
  {
    pAttr->modifyCollect(ECOLLECTTYPE_ACHIEVEMENT, m.second);
    //attrs[m.first].set_value(attrs[m.first].value() + m.second.value());
    XDBG << "[冒险手册-属性]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "采集到" << m.second.ShortDebugString() << XEND;
  }
}

DWORD Manual::getPetAdventureScore(EManualType eType)
{
  SManualItem* pItem = m_stData.getManualItem(eType);
  if (pItem == nullptr)
    return 0;

  const SPetAdventureEffCFG& rCFG = MiscConfig::getMe().getPetAdvEffCFG();
  DWORD score = 0;
  if (eType == EMANUALTYPE_FASHION)
  {
    for (auto &v : pItem->vecSubItems)
    {
      if (!v.bStore)
        continue;

      const SItemCFG* pItem = ItemConfig::getMe().getItemCFG(v.dwID);
      if (pItem == nullptr)
        continue;
      auto it = rCFG.mapHeadWearQua2Score.find(pItem->eQualityType);
      if (it != rCFG.mapHeadWearQua2Score.end())
        score += it->second;
    }
    return score;
  }

  if (eType == EMANUALTYPE_CARD)
  {
    for (auto &v : pItem->vecSubItems)
    {
      if (!v.bStore)
        continue;

      const SItemCFG* pItem = ItemConfig::getMe().getItemCFG(v.dwID);
      if (pItem == nullptr)
        continue;
      auto it = rCFG.mapCardQua2Score.find(pItem->eQualityType);
      if (it != rCFG.mapCardQua2Score.end())
        score += it->second;
    }

    return score;
  }

  return 0;
}

DWORD Manual::getUnlockNum(EManualType eType, DWORD feature)
{
  if(eType ==  EMANUALTYPE_MIN)
    return getMonthCard();

  DWORD ret = 0;
  SManualItem* pItem = m_pUser->getManual().getManualItem(eType);
  if(pItem == nullptr)
    return ret;

  for(auto s : pItem->vecSubItems)
  {
    if(s.eStatus <= EMANUALSTATUS_UNLOCK_CLIENT)
      continue;

    if(feature == 0)
    {
      ++ret;
      continue;
    }

    if(eType == EMANUALTYPE_CARD)
    {
      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(s.dwID);
      if(pCFG == nullptr || pCFG->eQualityType != static_cast<EQualityType>(feature))
        continue;
    }
    else if(eType == EMANUALTYPE_FASHION)
    {
      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(s.dwID);
      if(pCFG == nullptr || pCFG->eEquipType != static_cast<EEquipType>(feature))
        continue;
    }
    else if (eType == EMANUALTYPE_MONSTER || eType == EMANUALTYPE_NPC)
    {
      const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(s.dwID);
      if(pCFG == nullptr || pCFG->eNpcType != static_cast<ENpcType>(feature))
        continue;
    }

    ++ret;
  }

  return ret;
}

DWORD Manual::getConstantUnlockNum(EManualType eType, TSetDWORD setID)
{
  DWORD ret = 0;
  SManualItem* pItem = m_pUser->getManual().getManualItem(eType);
  if(pItem == nullptr)
    return ret;

  for(auto s : pItem->vecSubItems)
  {
    auto it = setID.find(s.dwID);
    if(it == setID.end())
      continue;
    if(s.eStatus == EMANUALSTATUS_UNLOCK)
      ret++;
  }

  return ret;
}

bool Manual::addAttributes(EManualType eType)
{
  TSetDWORD setItems;
  ItemConfig::getMe().getItemByManualType(setItems, eType);
  for(auto s : setItems)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(s);
    if(pCFG == nullptr)
      continue;

    SManualItem* pItem = m_stData.getManualItem(eType);
    if(pItem == nullptr)
      continue;

    SManualSubItem* pSubItem = pItem->getSubItem(s);
    if(pSubItem == nullptr)
    {
      SManualSubItem stSubItem;
      stSubItem.dwID = s;
      stSubItem.eStatus = EMANUALSTATUS_UNLOCK;
      stSubItem.bStore = true;
      pItem->vecSubItems.push_back(stSubItem);
      pItem->dwVersion = xTime::getCurSec();
      for (auto v = pCFG->vecAdvBuffIDs.begin(); v != pCFG->vecAdvBuffIDs.end(); ++v)
        m_pUser->m_oBuff.add(*v);
      for (auto v = pCFG->vecStoreBuffIDs.begin(); v != pCFG->vecStoreBuffIDs.end(); ++v)
        m_pUser->m_oBuff.add(*v);
    }
    else
    {
      if(pSubItem->eStatus != EMANUALSTATUS_UNLOCK)
      {
        pSubItem->eStatus = EMANUALSTATUS_UNLOCK;
        for (auto v = pCFG->vecAdvBuffIDs.begin(); v != pCFG->vecAdvBuffIDs.end(); ++v)
          m_pUser->m_oBuff.add(*v);

        pItem->dwVersion = xTime::getCurSec();
      }
      if(pSubItem->bStore == false)
      {
        pSubItem->bStore = true;
        for (auto v = pCFG->vecStoreBuffIDs.begin(); v != pCFG->vecStoreBuffIDs.end(); ++v)
          m_pUser->m_oBuff.add(*v);

        pItem->dwVersion = xTime::getCurSec();
      }
    }
  }

  return true;
}
bool Manual::leveldown(DWORD dwLevel)
{
  if (m_pLvCFG == nullptr)
    return false;

  DWORD dwOldLv = m_stData.dwLevel;
  if(dwOldLv <= dwLevel)
    return false;

  bool bSuccess = false;
  while (dwOldLv > dwLevel)
  {
    const SManualLvCFG* pNextCFG = ManualConfig::getMe().getManualLvCFG(dwOldLv - 1);
    if (pNextCFG == nullptr)
      break;

    m_stData.dwPoint = 0;
    m_stData.dwLevel -= 1;
    bSuccess = true;
    XLOG << "[冒险手册-降级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "降级成功 level :" << m_stData.dwLevel << "exp :" << m_stData.dwPoint << XEND;

    m_pLvCFG = pNextCFG;
    dwOldLv--;
  }

  if (bSuccess)
  {
    m_pUser->setDataMark(EUSERDATATYPE_MANUAL_LV);
    sendLevel();
    m_pUser->setCollectMark(ECOLLECTTYPE_ACHIEVEMENT);
    m_pUser->updateAttribute();
  }

  return bSuccess;
}

void Manual::addLotteryFashion()
{
  if(m_pUser->getMenu().isOpen(EMENUID_MANUAL) == false)
    return;

  TSetDWORD setIDs = ItemConfig::getMe().getFashionIDSet();
  for(auto s : setIDs)
    onItemAdd(s, false, false);
}
