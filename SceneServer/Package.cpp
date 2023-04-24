#include "Package.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "GuidManager.h"
#include "UserEvent.h"
#include "GMCommandRuler.h"
#include "MsgManager.h"
#include "SceneNpcManager.h"
#include "SceneUserManager.h"
#include "ItemManager.h"
#include "SceneServer.h"
#include "SceneTrade.pb.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "StatisticsDefine.h"
#include <functional>
#include "RoleDataConfig.h"
#include "ActivityManager.h"
#include "PetConfig.h"
#include "DScene.h"
#include "MailManager.h"
#include "Menu.h"
#include "ActivityEventManager.h"

// reward manager
bool RewardManager::roll(DWORD id, SceneUser* pUser, TVecItemInfo& vecItemInfo, ESource source, float fRatio /* = 1.0f*/, DWORD dwMapID /* = 0 */)
{
  RewardEntry oEntry;
  if (pUser != nullptr)
  {
    oEntry.set_pro(pUser->getProfession());
    oEntry.set_baselv(pUser->getLevel());
    oEntry.set_joblv(pUser->getJobLv());
    oEntry.set_charid(pUser->id);
    oEntry.set_safetymap(&pUser->getPackage().getRewardSafetyMap());
  }
  return RewardConfig::getMe().roll(id, oEntry, vecItemInfo, source, fRatio, dwMapID);
}

/**
 * 暂时仅用于宠物冒险
 */ 
bool RewardManager::roll_adventure(DWORD id, SceneUser* pUser, TVecItemInfo& vecItemInfo, ESource source, const OtherFactor& sFactor, float fRatio /*= 1.0f */, DWORD dwMapID /* =0 */)
{
  RewardEntry oEntry;
  if (pUser != nullptr)
  {
    oEntry.set_pro(pUser->getProfession());
    oEntry.set_baselv(pUser->getLevel());
    oEntry.set_joblv(pUser->getJobLv());
  }
  return RewardConfig::getMe().roll_adventure(id, oEntry, vecItemInfo, source, sFactor ,fRatio, dwMapID);
}

// package
Package::Package(SceneUser* pUser) : m_pUser(pUser)
{
  for (int i = EPACKTYPE_MIN; i < EPACKTYPE_MAX; ++i)
    m_pPackage[i] = nullptr;
}

Package::~Package()
{
  for (int i = EPACKTYPE_MAIN; i < EPACKTYPE_MAX; ++i)
    SAFE_DELETE(m_pPackage[i]);
}

bool Package::load(const BlobPack& oAccData, const BlobPack& oData/*, const string& oStore*/)
{
  // create pacakge
  if (createPackage() == false)
    return false;

  /*// parse store data
  PackageData oStoreData;
  if (oStoreData.ParseFromString(oStore) == false)
  {
    XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加载失败,反序列化公共仓库失败" << XEND;
    return false;
  }
  BasePackage* pStorePackage = getPackage(oStoreData.type());
  if (pStorePackage == nullptr || pStorePackage->fromData(oStoreData) == false)
  {
    XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << oStoreData.type() << "加载失败,数据" << oStoreData.ShortDebugString() << XEND;
    return false;
  }*/

  // acc data
  for (int i = 0; i < oAccData.datas_size(); ++i)
  {
    const PackageData& rData = oAccData.datas(i);
    if (rData.type() == EPACKTYPE_CARD)
      continue;

    BasePackage* pPackage = getPackage(rData.type());
    if (pPackage == nullptr || pPackage->fromData(rData) == false)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载失败,数据" << rData.ShortDebugString() << XEND;
      return false;
    }
  }

  // parser data
  for (int i = 0; i < oData.datas_size(); ++i)
  {
    const PackageData& rData = oData.datas(i);
    if (rData.type() == EPACKTYPE_CARD)
      continue;

    BasePackage* pPackage = getPackage(rData.type());
    if (pPackage == nullptr || pPackage->fromData(rData) == false)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载失败,数据" << rData.ShortDebugString() << XEND;
      return false;
    }
  }

  // hint item
  m_setHintItemIDs.clear();
  for (int i = 0; i < oData.hintitems_size(); ++i)
    m_setHintItemIDs.insert(oData.hintitems(i));

  // clear update item
  m_vecItemChangeIDs.clear();

  // item use count
  m_vecItemUseCount.clear();
  m_vecItemUseCount.reserve(oData.itemuse_size());
  for (int i = 0; i < oData.itemuse_size(); ++i)
  {
    m_vecItemUseCount.push_back(pair<DWORD, DWORD>(oData.itemuse(i).itemid(), oData.itemuse(i).usecount()));
  }

  // 道具获得次数
  m_mapItemGetCount.clear();
  for (int i = 0; i < oData.itemget_size(); ++i)
  {
    SItemGetCount& v = m_mapItemGetCount[oData.itemget(i).itemid()];
    if (oData.itemget(i).source() == ESOURCE_MIN)
      v.dwCount = oData.itemget(i).getcount();
    else
      v.mapSource2Count[oData.itemget(i).source()] = oData.itemget(i).getcount();
  }

  // set arrow type
  m_dwActiveArrow = oData.arrowid();
  if (oData.arrowid() != 0 && getPackage(EPACKTYPE_MAIN))
  {
    if (getPackage(EPACKTYPE_MAIN)->getGUIDByType(oData.arrowid()).empty())
      m_dwActiveArrow = 0;
  }

  m_dwDataVersion = oData.version();

  // 装备位
  for (int i = 0; i < oData.equipposdatas_size(); ++i)
  {
    m_mapEquipPos[oData.equipposdatas(i).pos()].fromData(oData.equipposdatas(i));
    m_mapEquipPos[oData.equipposdatas(i).pos()].dwProtectAlways = 0; // 永久保护标志实时算
  }

  for (int i = 0; i < oData.rewardsafetyitems_size(); ++i)
    m_mapRewardSafetyItem[oData.rewardsafetyitems(i).id()] = oData.rewardsafetyitems(i);

  // 修复未生效的附魔属性
  fixInvalidEnchant();

  refreshDeleteTimeItems();

  version_update();
  resetQuestPackItem();
  resetPetPackItem();
  resetPackMoveItem();
  return true;
}

bool Package::save(BlobPack* pAccBlob, BlobPack* pBlob)
{
  if (pBlob == nullptr)
    return false;

  pAccBlob->Clear();
  pBlob->Clear();

  // collect package data
  for (int i = EPACKTYPE_MAIN; i < EPACKTYPE_MAX; ++i)
  {
    if (i == EPACKTYPE_CARD)// || i == EPACKTYPE_STORE)
      continue;

    BasePackage* pack = getPackage(static_cast<EPackType>(i));
    if (pack == nullptr)
    {
      XERR << "[包裹-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "pack=" << i << "not found!" << XEND;
      continue;
    }

    PackageData* pData = ItemConfig::getMe().isAccPack(pack->getPackageType()) == true ? pAccBlob->add_datas() : pBlob->add_datas();
    if (pData == nullptr)
    {
      XERR << "[包裹-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "pack=" << i << "protobuf error" << XEND;
      continue;
    }
    pData->set_type(static_cast<EPackType>(i));

    auto func = [this, pData, i](ItemBase* pBase)
    {
      if (pData == nullptr || pBase == nullptr)
        return;

      ItemData* pItem = pData->add_items();
      if (pItem == nullptr)
      {
        XERR << "[包裹-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "pack=" << i << "item=" << pBase->getGUID() << "id=" << pBase->getTypeID() << "protobuf error" << XEND;
        return;
      }

      pBase->toItemData(pItem);
    };
    pack->foreach(func);

    const TMapInvalidItem& mapList = pack->getInvalidItem();
    for (auto &m : mapList)
      pData->add_items()->CopyFrom(m.second);
  }

  // hint item
  pBlob->clear_hintitems();
  for (auto s = m_setHintItemIDs.begin(); s != m_setHintItemIDs.end(); ++s)
    pBlob->add_hintitems(*s);

  // item use count
  pBlob->clear_itemuse();
  for (auto &p : m_vecItemUseCount)
  {
    ItemUseCount* pUse = pBlob->add_itemuse();
    if (pUse == nullptr)
      continue;
    pUse->set_itemid(p.first);
    pUse->set_usecount(p.second);
  }

  // 道具获得次数
  pBlob->clear_itemget();
  for (auto& it : m_mapItemGetCount)
  {
    if (it.second.dwCount > 0)
    {
      ItemGetCount* pGet = pBlob->add_itemget();
      if (pGet != nullptr)
      {
        pGet->set_itemid(it.first);
        pGet->set_getcount(it.second.dwCount);
      }
    }
    for (auto& v : it.second.mapSource2Count)
    {
      ItemGetCount* pSourceGet = pBlob->add_itemget();
      if (pSourceGet != nullptr)
      {
        pSourceGet->set_itemid(it.first);
        pSourceGet->set_getcount(v.second);
        pSourceGet->set_source(v.first);
      }
    }
  }

  // save arrow type
  pBlob->set_arrowid(m_dwActiveArrow);

  // save version
  pBlob->set_version(m_dwDataVersion);

  // 装备位
  for (auto& v : m_mapEquipPos)
    v.second.toData(pBlob->add_equipposdatas());

  DWORD cur = now();
  for (auto& v : m_mapRewardSafetyItem)
  {
    if (cur >= v.second.expiretime()) // 过期数据不保存
    {
      XLOG << "[包裹-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << v.second.ShortDebugString() << "reward保底数据过期清除" << XEND;
      continue;
    }
    RewardSafetyItem* p = pBlob->add_rewardsafetyitems();
    if (p)
      p->CopyFrom(v.second);
  }

  XDBG << "[包裹-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << pBlob->ByteSize() << XEND;
  return true;
}

// 加载职业信息
bool Package::loadProfessionData(const Cmd::ProfessionData& data, bool isNeedPutOn)
{
  if(!m_pUser)
    return false;

  if(!isNeedPutOn)
  {
    // 策划要求暂时卸载全部无效装备
    equipOffInValidEquip();

    // 卸载无效坐骑和手推车
    /*EquipPackage* pEquipPack = getEquipPackage();
    if(!pEquipPack)
    {
      XERR << "[包裹-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get EquipPackage failed!" << XEND;
      return false;
    }

    enum EEquipPos pos_array[] = {EEQUIPPOS_MOUNT, EEQUIPPOS_BARROW};
    for(auto p : pos_array)
    {
      ItemEquip* pEquip = pEquipPack->getEquip(p);
      if(!pEquip)
        continue;
      if(pEquipPack->canEquip(m_pUser, pEquip))
        continue;

      equip(EEQUIPOPER_OFF, EEQUIPPOS_MIN, pEquip->getGUID());
    }*/

    return true;
  }

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if(!pEquipPack)
  {
    XERR << "[包裹-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get EquipPackage failed! " << XEND;
    return false;
  }

  FashionEquipPackage* pFashionPack = dynamic_cast<FashionEquipPackage*>(getPackage(EPACKTYPE_FASHIONEQUIP));
  if(!pFashionPack)
  {
    XERR << "[包裹-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get FashionEquipPackage failed! " << XEND;
    return false;
  }

  // 先脱
  // 从大到小脱
  for(DWORD pos = EEQUIPPOS_MAX - 1; pos > EEQUIPPOS_MIN; --pos)
  {
    ItemEquip* pEquip = pEquipPack->getEquip(static_cast<EEquipPos>(pos));
    if(pEquip)
      equip(Cmd::EEQUIPOPER_OFF, Cmd::EEQUIPPOS_MIN, pEquip->getGUID());

    ItemEquip* pFashion = pFashionPack->getEquip(static_cast<EEquipPos>(pos));
    if(pFashion)
      equip(Cmd::EEQUIPOPER_OFFFASHION, Cmd::EEQUIPPOS_MIN, pFashion->getGUID());
  }

  // 后穿
  // 从小到大穿
  for(int i=0; i<data.pack_data_size(); ++i)
  {
    const Cmd::EquipPackData& packData = data.pack_data(i);
    EEquipOper eEquipOper = EEQUIPOPER_MIN;
    if(EPACKTYPE_EQUIP == static_cast<EPackType>(packData.type()))
    {
      eEquipOper = Cmd::EEQUIPOPER_ON;
    }
    else if(EPACKTYPE_FASHIONEQUIP == static_cast<EPackType>(packData.type()))
    {
      eEquipOper = Cmd::EEQUIPOPER_PUTFASHION;
    }

    std::map<DWORD, ItemBase*> mapItems;
    for(int j=0; j<packData.datas_size(); ++j)
    {
      BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
      if(!pMainPack)
      {
        XERR << "[包裹-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get MainPackage failed! " << XEND;
        continue;
      }
      ItemBase* pItem = pMainPack->getItem(packData.datas(j).guid());
      if(!pItem)
      {
        XERR << "[包裹-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get ItemBase failed! guid:" << packData.datas(j).guid() << XEND;
        continue;
      }

      mapItems[packData.datas(j).pos()] = pItem;
    }

    for(auto& m : mapItems)
      equip(eEquipOper, static_cast<EEquipPos>(m.first), m.second->getGUID());
  }

  XLOG << "[包裹-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load package data success! " << XEND;
  return true;
}

// 保存职业信息
bool Package::saveProfessionData(Cmd::ProfessionData& data)
{
  if(!m_pUser)
    return false;

  data.clear_pack_data();
  for(int i=EPackType_MIN; i<EPackType_MAX; i++)
  {
    if(EPACKTYPE_EQUIP != i && EPACKTYPE_FASHIONEQUIP != i)
      continue;

    Cmd::EquipPackData* pData = data.add_pack_data();
    if(!pData)
    {
      XERR << "[包裹-多职业保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get Cmd::EquipPackData failed! i:" << i << XEND;
      continue;
    }

    pData->set_type(i);
    BasePackage* pPackage = m_pPackage[i];
    if(!pPackage)
    {
      XERR << "[包裹-多职业保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get BasePackage failed! i:" << i << XEND;
      continue;
    }

    for(auto& m : pPackage->m_mapID2Item)
    {
      Cmd::EquipInfo* pInfo = pData->add_datas();
      if(!pInfo)
      {
        XERR << "[包裹-多职业保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get Cmd::EquipInfo failed! i:" << i << XEND;
        continue;
      }

      pInfo->set_guid(m.first);
      pInfo->set_pos(m.second->getIndex());
      pInfo->set_type_id(m.second->getTypeID());

      XLOG << "[包裹-多职业保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "branch:" << data.id() << "type:" << i << "guid:" << m.first << "itemid:" << m.second->getTypeID() << "pos:" << m.second->getIndex() << XEND;
    }
  }

  return true;
}

void Package::reload()
{
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    BasePackage* pPack = m_pPackage[i];
    if (pPack == nullptr)
      continue;

    for (auto m = pPack->m_mapID2Item.begin(); m != pPack->m_mapID2Item.end(); ++m)
      m->second->setCFG(ItemManager::getMe().getItemCFG(m->second->getTypeID()));
  }
}

void Package::refreshSlot()
{
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    if (m_pPackage[i] != nullptr)
      m_pPackage[i]->refreshSlot();
  }
}

bool Package::toClient(EPackType eType, PackageItem& rItem)
{
  BasePackage* pPackage = getPackage(eType);
  if (pPackage == nullptr)
    return false;

  pPackage->toClient(rItem);
  return true;
}

void Package::sendPackData(EPackType eType)
{
  BasePackage* pPack = getPackage(eType);
  if (pPack == nullptr)
  {
    XERR << "[包裹-数据]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "请求了 type :" << eType << "数据失败,异常" << XEND;
    return;
  }

  PackageItem cmd;
  cmd.set_type(eType);
  cmd.set_maxslot(pPack->getMaxSlot());

  DWORD dwIndex = 0;
  DWORD dwCount = 0;
  auto itemf = [&](ItemBase* pBase)
  {
    if (pBase == nullptr)
      return;

    ItemData* pData = cmd.add_data();
    pBase->toClientData(eType, pData, m_pUser);
    ++dwCount;

    if (cmd.ByteSize() > TRANS_BUFSIZE - pData->ByteSize())
    {
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      XDBG << "[包裹-数据]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "请求了 type :" << eType << "第" << dwIndex++ << "次发送" << cmd.data_size() << "个道具" << XEND;
      cmd.clear_data();
    }
  };
  pPack->foreach(itemf);
  if (cmd.data_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
    XDBG << "[包裹-数据]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "请求了 type :" << eType << "第" << dwIndex++ << "次发送" << cmd.data_size() << "个道具" << XEND;
  }
  XDBG << "[包裹-数据]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "请求了 type :" << eType << "数据, 总共" << dwCount << "个道具" << XEND;
}

bool Package::addItemCount(ItemBase* pBase)
{
  if (pBase == nullptr)
    return false;

  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr)
    return false;

  DWORD id = pBase->getTypeID();
  auto v = find_if(m_vecItemCount.begin(), m_vecItemCount.end(), [id](const pair<DWORD, DWORD>& r) -> bool{
    return r.first == id;
  });
  if (v != m_vecItemCount.end())
    v->second += 1;
  else
    m_vecItemCount.push_back(pair<DWORD, DWORD>(id, 1));

  auto o = find(m_vecItemChangeIDs.begin(), m_vecItemChangeIDs.end(), pBase->getTypeID());
  if (o == m_vecItemChangeIDs.end())
    m_vecItemChangeIDs.push_back(pBase->getTypeID());

  return true;
}

bool Package::decItemCount(DWORD id)
{
  auto v = find_if(m_vecItemCount.begin(), m_vecItemCount.end(), [id](const pair<DWORD, DWORD>& r) -> bool{
    return r.first == id;
  });
  if (v == m_vecItemCount.end())
    return false;

  if (v->second > 0)
    v->second -= 1;
  if (v->second == 0)
    m_vecItemCount.erase(v);

  auto o = find(m_vecItemChangeIDs.begin(), m_vecItemChangeIDs.end(), id);
  if (o == m_vecItemChangeIDs.end())
    m_vecItemChangeIDs.push_back(id);

  return true;
}

DWORD Package::getItemCount(DWORD id)
{
  auto v = find_if(m_vecItemCount.begin(), m_vecItemCount.end(), [id](const pair<DWORD, DWORD>& r) -> bool{
    return r.first == id;
  });
  if (v == m_vecItemCount.end())
    return 0;

  return v->second;
}

DWORD Package::collectItemCount(DWORD id)
{
  TSetDWORD setPack = {EPACKTYPE_MAIN, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_BARROW, EPACKTYPE_TEMP_MAIN};
  
  ItemInfo oItem;
  oItem.set_id(id);
  oItem.set_count(DWORD_MAX);
  TVecItemInfo vecItem;
  combinItemInfo(vecItem, oItem);
  
  TMapReduce mapReduce;
  makeReduceList(vecItem, mapReduce, ECHECKMETHOD_NORMAL, setPack);

  DWORD dwItemCount = 0;
  for (auto &m : mapReduce)
  {
    for (auto &v : m.second)
      dwItemCount += v.count();
  }
  return dwItemCount;
}

bool Package::checkAndFillItemData(ItemData& rInfo)
{
  //check
  if (rInfo.base().guid().empty())
  {
    string guid = GuidManager::getMe().newGuidStr(m_pUser->getZoneID(), m_pUser->getUserSceneData().getOnlineMapID());
    if (ItemManager::getMe().isGUIDValid(guid) == false)
      return false;
    rInfo.mutable_base()->set_guid(guid);
  }

  if (rInfo.has_equip())
  {
    //check strength lv
    EquipData* pData = rInfo.mutable_equip();
    if (pData->strengthlv() > m_pUser->getLevel())
    {
      pData->set_strengthlv(m_pUser->getLevel());
    }
  }

  //fill card guid
  for (int i = 0; i < rInfo.card_size(); i++)
  {
    CardData* pCard = rInfo.mutable_card(i);
    string guid = GuidManager::getMe().newGuidStr(m_pUser->getZoneID(), m_pUser->getUserSceneData().getOnlineMapID());
    if (ItemManager::getMe().isGUIDValid(guid) == false)
      return false;
    pCard->set_guid(guid);
  }

  // clear pet name
  //if (rInfo.egg().name().empty() == false)
  //  rInfo.mutable_egg()->clear_name();

  return true;
}

bool Package::addItem(ItemInfo rInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  ItemData oData;
  oData.mutable_base()->CopyFrom(rInfo);
  return addItem(oData, eMethod, bShow, bInform, bForceShow);
}

bool Package::addItem(TVecItemInfo vecInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  TVecItemData vecData;
  vecData.resize(vecInfo.size());
  for (size_t i = 0; i < vecData.size(); ++i)
    vecData[i].mutable_base()->CopyFrom(vecInfo[i]);
  return addItem(vecData, eMethod, bShow, bInform, bForceShow);
}

bool Package::addItem(ItemData rData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rData.base().id());
  if (pCFG == nullptr)
  {
    XERR << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "使用" << eMethod << "添加" << rData.ShortDebugString() << "失败, 未在 Table_Item.txt 表中找到" << XEND;
    return false;
  }

  const SFoodMiscCFG& rCFG = MiscConfig::getMe().getFoodCfg();
  if (ItemConfig::getMe().isQuest(pCFG->eItemType) == true)
  {
    BasePackage* pQuestPack = m_pPackage[EPACKTYPE_QUEST];
    if (pQuestPack == nullptr)
    {
      XERR << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << rData.ShortDebugString() << "到" << EPACKTYPE_QUEST << "失败,未找到包裹" << XEND;
      return false;
    }
    return pQuestPack->addItem(rData, EPACKMETHOD_NOCHECK);
  }
  else if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == true && rCFG.isFoodItem(pCFG->eItemType) == true)
  {
    BasePackage* pFoodPack = m_pPackage[EPACKTYPE_FOOD];
    if (pFoodPack == nullptr)
    {
      XERR << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << rData.ShortDebugString() << "到" << EPACKTYPE_FOOD << "失败,未找到包裹" << XEND;
      return false;
    }
    return pFoodPack->addItem(rData, EPACKMETHOD_CHECK_WITHPILE, bShow, bInform, bForceShow);
  }
  else if (pCFG->eItemType == EITEMTYPE_EGG)
  {
    BasePackage* pPetPack = m_pPackage[EPACKTYPE_PET];
    if (pPetPack == nullptr)
    {
      XERR << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << rData.ShortDebugString() << "到" << EPACKTYPE_PET << "失败,未找到包裹" << XEND;
      return false;
    }
    return pPetPack->addItem(rData, EPACKMETHOD_CHECK_WITHPILE, bShow, bInform, bForceShow);
  }
  else
  {
    BasePackage* pMainPack = m_pPackage[EPACKTYPE_MAIN];
    if (pMainPack == nullptr)
    {
      XERR << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << rData.ShortDebugString() << "到" << EPACKTYPE_MAIN << "失败,未找到包裹" << XEND;
      return false;
    }
    if (eMethod != EPACKMETHOD_AVAILABLE)
    {
      if (pMainPack->checkAddItem(rData, eMethod) == false)
      {
        XDBG << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "使用" << eMethod << "添加" << rData.ShortDebugString() << "到" << EPACKTYPE_MAIN << "失败,检查未通过" << XEND;
        return false;
      }
      return pMainPack->addItem(rData, eMethod, bShow, bInform, bForceShow);
    }

    BasePackage* pTempPack = m_pPackage[EPACKTYPE_TEMP_MAIN];
    if (pTempPack == nullptr)
    {
      XERR << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << rData.ShortDebugString() << "失败,未找到" << EPACKTYPE_TEMP_MAIN << XEND;
      return false;
    }

    pMainPack->addItem(rData, eMethod, bShow, bInform, bForceShow);
    if (rData.base().count() != 0)
      pTempPack->addItem(rData, eMethod, bShow, bInform, bForceShow);

    if (rData.base().count() != 0)
    {
      XERR << "[包裹-物品添加-Data]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加剩余" << rData.ShortDebugString() << "未找到合适的位置,被丢弃" << XEND;
    }
  }

  return true;
}

bool Package::addItem(TVecItemData vecData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  TVecItemData vecNormal;
  TVecItemData vecQuest;
  TVecItemData vecFood;
  TVecItemData vecPet;

  const SFoodMiscCFG& rCFG = MiscConfig::getMe().getFoodCfg();
  for (auto &v : vecData)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.base().id());
    if (pCFG == nullptr)
    {
      XERR << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "批量添加道具失败 id :" << v.base().id() << "未在 Table_Item.txt 表中找到, 被丢弃道具" << vecData << XEND;
      return false;
    }
    if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == true && rCFG.isFoodItem(pCFG->eItemType) == true)
      vecFood.push_back(v);
    else if (pCFG->eItemType == EITEMTYPE_EGG)
      vecPet.push_back(v);
    else if (ItemConfig::getMe().isQuest(pCFG->eItemType) == false)
      vecNormal.push_back(v);
    else
      vecQuest.push_back(v);
  }

  if (vecNormal.empty() == false)
  {
    BasePackage* pMainPack = m_pPackage[EPACKTYPE_MAIN];
    if (pMainPack == nullptr)
    {
      XERR << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加普通" << vecNormal << "失败,未找到" << EPACKTYPE_MAIN << XEND;
      return false;
    }
    if (eMethod != EPACKMETHOD_AVAILABLE)
    {
      if (pMainPack->checkAddItem(vecNormal, eMethod) == false)
      {
        XDBG << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "使用" << eMethod << "添加普通" << vecNormal << "失败," << EPACKTYPE_MAIN << "未通过" << XEND;
        return false;
      }
      pMainPack->addItem(vecNormal, eMethod, bShow, bInform, bForceShow);
    }
    else
    {
      BasePackage* pTempPack = m_pPackage[EPACKTYPE_TEMP_MAIN];
      if (pTempPack == nullptr)
      {
        XERR << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "使用" << eMethod << "添加任务" << vecNormal << "失败,未找到" << EPACKTYPE_TEMP_MAIN << XEND;
        return false;
      }

      pMainPack->addItem(vecNormal, eMethod, bShow, bInform, bForceShow);
      if (vecNormal.empty() == false)
        pTempPack->addItem(vecNormal, eMethod, bShow, bInform, bForceShow);

      if (vecNormal.empty() == false)
      {
        XERR << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "使用" << eMethod << "添加,剩余" << vecNormal << "未找到合适的位置,被丢弃" << XEND;
      }
    }
  }

  if (vecQuest.empty() == false)
  {
    BasePackage* pQuestPack = m_pPackage[EPACKTYPE_QUEST];
    if (pQuestPack == nullptr)
    {
      XERR << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << vecQuest << "到" << EPACKTYPE_QUEST << "失败,未找到包裹" << XEND;
      return false;
    }
    pQuestPack->addItem(vecQuest, EPACKMETHOD_NOCHECK);
  }

  if (vecFood.empty() == false)
  {
    BasePackage* pFoodPack = m_pPackage[EPACKTYPE_FOOD];
    if (pFoodPack == nullptr)
    {
      XERR << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << vecFood << "到" << EPACKTYPE_FOOD << "失败,未找到包裹" << XEND;
      return false;
    }
    pFoodPack->addItem(vecFood, EPACKMETHOD_NOCHECK);
  }

  if (vecPet.empty() == false)
  {
    BasePackage* pPetPack = m_pPackage[EPACKTYPE_PET];
    if (pPetPack == nullptr)
    {
      XERR << "[包裹-物品添加-VecData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << eMethod << "添加" << vecPet << "到" << EPACKTYPE_PET << "失败,未找到包裹" << XEND;
      return false;
    }
    pPetPack->addItem(vecPet, EPACKMETHOD_NOCHECK);
  }

  return true;
}

bool Package::checkItemCount(const ItemInfo& oItem, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, const TSetDWORD& setPack /*= {EPACKTYPE_MAIN, EPACKTYPE_QUEST}*/)
{
  TVecItemInfo vecItem;
  combinItemInfo(vecItem, oItem);
  return checkItemCount(vecItem, eMethod, setPack);
}

bool Package::reduceItem(const ItemInfo& oItem, ESource eSource, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, const TSetDWORD& setPack /*= {EPACKTYPE_MAIN, EPACKTYPE_QUEST}*/)
{
  if (checkItemCount(oItem, eMethod, setPack) == false)
    return false;
  return reduceItem(TVecItemInfo{oItem}, eSource, eMethod, setPack);
}

bool Package::checkItemCount(const TVecItemInfo& vecItem, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, const TSetDWORD& setPack /*= {EPACKTYPE_MAIN, EPACKTYPE_QUEST}*/)
{
  TMapReduce mapReduce;
  return makeReduceList(vecItem, mapReduce, eMethod, setPack) == true;
}

bool Package::reduceItem(const TVecItemInfo& vecItem, ESource eSource, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, const TSetDWORD& setPack /*= {EPACKTYPE_MAIN, EPACKTYPE_QUEST}*/)
{
  if (checkItemCount(vecItem, eMethod, setPack) == false)
    return false;

  TMapReduce mapReduce;
  if (makeReduceList(vecItem, mapReduce, eMethod, setPack) == false)
    return false;

  for (auto &m : mapReduce)
  {
    BasePackage* pPack = getPackage(m.first);
    if (pPack == nullptr)
    {
      XERR << "[包裹-物品删除-vecitem]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在" << m.first << "中删除" << m.second << "失败,未找到该包裹" << XEND;
      continue;
    }
    pPack->reduceItem(m.second, eSource, eMethod);
  }

  for (auto &v : vecItem)
  {
    MsgParams params;
    params.addNumber(v.id());
    params.addNumber(v.id());
    params.addNumber(v.count());
    MsgManager::sendMsg(m_pUser->id, 75, params);
  }

  return true;
}

bool Package::checkItemCount(const ItemInfo& oItem, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();
  return checkItemCount(oItem, eMethod, rCFG.getPackFunc(eFunc));
}

bool Package::reduceItem(const ItemInfo& oItem, ESource eSource, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();
  return reduceItem(oItem, eSource, eMethod, rCFG.getPackFunc(eFunc));
}

bool Package::checkItemCount(const TVecItemInfo& vecItem, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();
  return checkItemCount(vecItem, eMethod, rCFG.getPackFunc(eFunc));
}

bool Package::reduceItem(const TVecItemInfo& vecItem, ESource eSource, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();
  return reduceItem(vecItem, eSource, eMethod, rCFG.getPackFunc(eFunc));
}

bool Package::checkItemCount(const string& guid, DWORD count, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  TMapReduce mapReduce;
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();
  return makeReduceList(guid, count, mapReduce, eMethod, rCFG.getPackFunc(eFunc));
}

bool Package::reduceItem(const string& guid, DWORD count, ESource eSource, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/, EPackFunc eFunc /*= EPACKFUNC_MIN*/)
{
  if (checkItemCount(guid, count, eMethod, eFunc) == false)
    return false;

  TMapReduce mapReduce;
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();
  if (makeReduceList(guid, count, mapReduce, eMethod, rCFG.getPackFunc(eFunc)) == false)
    return false;

  for (auto &m : mapReduce)
  {
    BasePackage* pPack = getPackage(m.first);
    if (pPack == nullptr)
    {
      XERR << "[包裹-物品删除-vecitem]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在" << m.first << "中删除" << m.second << "失败,未找到该包裹" << XEND;
      continue;
    }
    for (auto &v : m.second)
      pPack->reduceItem(v.guid(), eSource, v.count(), true, eMethod);
  }

  return true;
}

bool Package::rollReward(DWORD dwRewardID, EPackMethod eMethod /*= EPACKMETHOD_NOCHECK*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/,
    DWORD times /*= 0*/, bool bUnlock /*= false*/, ESource source /*= ESOURCE_REWARD*/)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-Reward奖励获得] 未找到玩家" << XEND;
    return false;
  }

  TVecItemInfo vecItemInfo;
  if (RewardManager::roll(dwRewardID, m_pUser, vecItemInfo, source) == false)
  {
    XERR << "[包裹-Reward奖励获得]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "随机 reward :" << dwRewardID << "失败" << XEND;
    return false;
  }

  if(times > 1)
  {
    for (auto &item : vecItemInfo)
    {
      DWORD count = item.count() * times;
      item.set_count(count);
    }
    XLOG << "[包裹-Reward奖励获得] 双倍奖励" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 reward :" << dwRewardID << "成功" << XEND;
  }

  TVecItemInfo vecItem = vecItemInfo;
  m_pUser->getPackage().addItem(vecItem, eMethod, bShow, bInform, bForceShow);

  timer(xTime::getCurSec());

  std::stringstream rewardItemss;
  for (auto &item : vecItemInfo)
  {
    rewardItemss << item.id() << "," << item.count() << ";";
    if (dwRewardID == 3649)  //神秘箱子
      m_pUser->getShare().onOpenMysteryBox(item.id());

    if(bUnlock == true)
      m_pUser->getManual().onItemAdd(item.id(), true, true, true, ESOURCE_MAX);
  }
  std::string rewardItem = rewardItemss.str();
  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Reward;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  PlatLogManager::getMe().RewardLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    dwRewardID,
    m_pUser->getUserSceneData().getProfession(),
    rewardItem
  );

  XLOG << "[包裹-Reward奖励获得]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 reward :" << dwRewardID << "成功" << XEND;
  return true;
}

EquipPackage* Package::getEquipPackage()
{
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if(pEquipPack != nullptr)
    return pEquipPack;

  return nullptr;
}

void Package::changeArrow(DWORD typeID)
{
  // 强制取消箭矢激活 (箭矢数量变为0)
  if (typeID == 0)
  {
    m_dwActiveArrow = 0;
    return;
  }

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(typeID);
  if (pCFG == nullptr || pCFG->eItemType != EITEMTYPE_ARROW)
    return;

  BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
    return;
  // 取消箭矢激活
  if (m_dwActiveArrow == typeID)
  {
    const TSetItemBase& setItem = pPack->getItemBaseList(typeID);
    for (auto &s : setItem)
      pPack->setUpdateIDs(s->getGUID());

    XLOG << "[箭矢变化], 玩家取消使用箭矢, 玩家:" << m_pUser->name << m_pUser->accid << m_pUser->id << "取消前使用箭矢id:" << m_dwActiveArrow << XEND;
    m_dwActiveArrow = 0;
  }
  // 更换箭矢
  else
  {
    const TSetItemBase& setItem = pPack->getItemBaseList(m_dwActiveArrow);
    for (auto &s : setItem)
      pPack->setUpdateIDs(s->getGUID());

    const TSetItemBase& setItemNew = pPack->getItemBaseList(typeID);
    for (auto &s : setItemNew)
      pPack->setUpdateIDs(s->getGUID());

    XLOG << "[箭矢变化], 玩家激活箭矢, 玩家:" << m_pUser->name << m_pUser->accid << m_pUser->id << "取消前使用箭矢id:" << m_dwActiveArrow << "新箭矢id:" << typeID << XEND;
    m_dwActiveArrow = typeID;
  }
}

bool Package::setStoreStatus(bool open)
{
  if (m_bStoreOpened == open)
    return false;

  if (open == false)
  {
    m_bStoreOpened = false;
    XLOG << "[仓库-开关], 玩家关闭仓库, 玩家" << m_pUser->name << m_pUser->id << XEND;
    return true;
  }

  const SItemMiscCFG& rItemCFG = MiscConfig::getMe().getItemCFG();
  const SSystemCFG& rSysCFG = MiscConfig::getMe().getSystemCFG();
  bool bItemUse = false;
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[仓库-开关], 玩家打开仓库, 玩家" << m_pUser->name << m_pUser->id << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }

  if(MiscConfig::getMe().getFunctionSwitchCFG().dwFreePackage != 0 || ActivityEventManager::getMe().isStoreFree())
  {
    m_bStoreOpened = true;
    XLOG << "[仓库-开启], 玩家开启仓库" << m_pUser->name << m_pUser->id << "活动期间免费开启"<< XEND;
    return true;
  }

  if (pMainPack->checkItemCount(rItemCFG.dwKapulaStoreItem) == true)
  {
    pMainPack->reduceItem(rItemCFG.dwKapulaStoreItem, ESOURCE_KAPULA);
    bItemUse = true;

    MsgParams param;
    param.addNumber(rItemCFG.dwKapulaStoreItem);
    param.addNumber(rItemCFG.dwKapulaStoreItem);
    param.addNumber(1);
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_ITEM_REDUCE, param);
  }
  else
  {
    DWORD zenynum = rSysCFG.dwOpenStoreCost;
    if (m_pUser->checkMoney(EMONEYTYPE_SILVER, zenynum) == false)
    {
      XERR << "[仓库-开启], 玩家zeny 不足" << m_pUser->name << m_pUser->id << XEND;
      return false;
    }
    m_pUser->subMoney(EMONEYTYPE_SILVER, zenynum, ESOURCE_OPENSTORE);
    m_pUser->getAchieve().onKplConsume(zenynum);
    MsgParams params;
    params.addNumber(100);
    params.addNumber(100);
    params.addNumber(zenynum);
    MsgManager::sendMsg(m_pUser->id, 75, params, EMESSAGETYPE_FRAME, EMESSAGEACT_DEL);
  }

  m_bStoreOpened = true;
  XLOG << "[仓库-开启], 玩家开启仓库" << m_pUser->name << m_pUser->id << "消耗了" << (bItemUse ? rItemCFG.dwKapulaStoreItem : rSysCFG.dwOpenStoreCost) << (bItemUse ? "道具" : "zeny") << XEND;
  return true;
}

void Package::refreshMaxSlot()
{
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    if (i == EPACKTYPE_EQUIP || i == EPACKTYPE_FASHION || i == EPACKTYPE_FASHIONEQUIP || i == EPACKTYPE_CARD)
      continue;

    BasePackage* pPack = getPackage(static_cast<EPackType>(i));
    if (pPack == nullptr)
      continue;
    DWORD dwLastMaxSlot = pPack->getLastMaxSlot();
    DWORD dwMaxSlot = pPack->getMaxSlot();
    if (dwLastMaxSlot == dwMaxSlot)
      continue;
    PackSlotNtfItemCmd cmd;
    cmd.set_type(pPack->getPackageType());
    cmd.set_maxslot(dwMaxSlot);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
    XLOG << "[包裹-最大上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << pPack->getPackageType() << "刷新最大上限为" << dwMaxSlot << "个" << XEND;
  }
}

bool Package::checkItemCountByID(DWORD id, DWORD count, EPackType eType)
{
  if (eType <= EPACKTYPE_MIN || eType >= EPACKTYPE_MAX)
    return false;

  if(m_pPackage[eType] == nullptr)
    return false;

  return m_pPackage[eType]->checkItemCount(id,count);
}

void Package::setCardSlotForGM(DWORD itemId)
{

  if (m_pPackage[EPACKTYPE_MAIN])
    m_pPackage[EPACKTYPE_MAIN]->setCardSlotForGM(itemId);

  if (m_pPackage[EPACKTYPE_EQUIP])
    m_pPackage[EPACKTYPE_EQUIP]->setCardSlotForGM(itemId);

  if (m_pPackage[EPACKTYPE_STORE])
    m_pPackage[EPACKTYPE_STORE]->setCardSlotForGM(itemId);
  
  if (m_pPackage[EPACKTYPE_PERSONAL_STORE])
    m_pPackage[EPACKTYPE_PERSONAL_STORE]->setCardSlotForGM(itemId);

  if (m_pPackage[EPACKTYPE_TEMP_MAIN])
    m_pPackage[EPACKTYPE_TEMP_MAIN]->setCardSlotForGM(itemId); 

  return ;
}

void Package::patch_equip_strengthlv()
{
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    BasePackage* pPack = m_pPackage[i];
    if (pPack == nullptr)
      continue;

    for (auto &m : pPack->m_mapID2Item)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m.second);
      if (pEquip == nullptr)
        continue;

      DWORD dwStrengthLv = pEquip->getStrengthLv();
      DWORD dwPrice = 0;
      for (DWORD d = 0; d < dwStrengthLv; ++d)
        dwPrice += LuaManager::getMe().call<DWORD>("calcEquipStrengthCost", d, pEquip->getQuality(), pEquip->getType());
      pEquip->resetStrengthCost();
      pEquip->addStrengthCost(dwPrice);
      ItemData oData;
      pEquip->toItemData(&oData);
      XLOG << "[包裹-补丁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在" << static_cast<EPackType>(i) << "对" << oData.ShortDebugString() << "重算强化价格为" << dwPrice << XEND;
    }
  }
  XLOG << "[包裹-补丁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行了强化补丁" << XEND;
}

BasePackage* Package::getPackage(EPackType eType)
{
  if (eType <= EPACKTYPE_MIN || eType >= EPACKTYPE_MAX)
    return nullptr;

  return m_pPackage[eType];
}

bool Package::checkDel(const string& guid)
{
  ItemBase* pItem = getItem(guid);
  if (pItem == nullptr)
    return false;
  const SItemCheckDelCFG& rCFG = MiscConfig::getMe().getItemCheckDelCFG();
  if (rCFG.hasItem(pItem->getTypeID()) == false)
    return false;
  const SItemCheckDel* pCFG = rCFG.getItemInfo(pItem->getTypeID());
  if (pCFG == nullptr)
    return false;
  switch(pCFG->eType)
  {
    case EITEMCHECKDEL_MIN:
      break;
    case EITEMCHECKDEL_LEVEL:
      {
        if (m_pUser->getLevel() > pCFG->param1)
        {
          equipOff(guid);
          for (DWORD i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
          {
            if (m_pPackage[i] == nullptr)
              continue;
            ItemBase* pItem = m_pPackage[i]->getItem(guid);
            if (pItem != nullptr)
            {
              if (pItem->getCFG())
                MsgManager::sendMsg(m_pUser->id, 20004, MsgParams(pItem->getCFG()->strNameZh));
              XLOG << "[等级提升-道具扣除], 玩家" << m_pUser->name << m_pUser->id << "等级:" << m_pUser->getLevel() << "道具:" << pItem->getTypeID() << "guid:" << guid << XEND;
              m_pPackage[i]->reduceItem(guid, ESOURCE_LVUP);
              return true;
            }
          }
        }
        break;
      }
    default:
      break;
  }
  return false;
}

bool Package::equip(EEquipOper oper, EEquipPos ePos, const string& guid, bool bTransfer /*= false*/, bool bWait/* = false*/, DWORD dwCount /*= 0*/)
{
  if (m_pUser->isInPollyScene() || (m_pUser->getScene() && m_pUser->getScene()->isAltmanScene()))
    return false;

  if (checkDel(guid) == true && oper != EEQUIPOPER_DRESSUP_ON && oper != EEQUIPOPER_DRESSUP_OFF)
    return true;

  bool bSuccesss = false;
  if (oper == EEQUIPOPER_ON)
    bSuccesss = equipOn(ePos, guid, bTransfer);
  else if (oper == EEQUIPOPER_OFF)
    bSuccesss = equipOff(guid);
  else if (oper == EEQUIPOPER_PUTFASHION)
    bSuccesss = equipPutFashion(ePos, guid, bTransfer);
  else if (oper == EEQUIPOPER_OFFFASHION)
    bSuccesss = equipOffFashion(guid);
  else if (oper == EEQUIPOPER_PUTSTORE)   // store oper not need update attribute
    return putStore(guid, dwCount);
  else if (oper == EEQUIPOPER_OFFSTORE)   // store oper not need update attribute
  /*{
    if (bWait)
    {
      // 等待record处理完成后再开始下一次操作
      DWORD cur = now();
      if (cur > m_dwOffStoreLock + 15)
      {
        bSuccesss = offstore(guid, dwCount);
        if (bSuccesss)
          m_dwOffStoreLock = cur;
      }
      else
      {
        #ifdef _DEBUG
        MsgManager::sendMsg(m_pUser->id, 10, MsgParams("操作过于频繁"));
        #endif
        bSuccesss = false;
      }
    }
    else
      bSuccesss = offstore(guid, dwCount);
    return bSuccesss;
  }*/
    return offstore(guid, dwCount);
  else if (oper == EEQUIPOPER_OFFALL)
    bSuccesss = equipOffAll();
  else if (oper == EEQUIPOPER_OFFPOS)
    bSuccesss = equipOff(ePos);
  else if (oper == EEQUIPOPER_PUTPSTORE)
    return putPStore(guid, dwCount);
  else if (oper == EEQUIPOPER_OFFPSTORE)
    return offPStore(guid, dwCount);
  else if (oper == EEQUIPOPER_OFFTEMP)
    return offTemp(guid);
  else if (oper == EEQUIPOPER_PUTBARROW)
    return putBarrow(guid, dwCount);
  else if (oper == EEQUIPOPER_OFFBARROW)
    return offBarrow(guid, dwCount);
  else if (oper == EEQUIPOPER_DRESSUP_ON)
    return equipDressUpOn(ePos, guid);
  else if (oper == EEQUIPOPER_DRESSUP_OFF)
    return equipDressUpOff(ePos, guid);
  else
    return false;

  if (bSuccesss)
  {
    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    m_pUser->refreshDataAtonce();
    //m_pUser->getEvent().onEquipChange(oper, guid);

    // 换装打断技能, 15-12-16, 前端需求
    m_pUser->m_oSkillProcessor.breakSkill(m_pUser->id);

    XLOG << "[包裹-装备穿戴]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "oper:" << oper << "pos:" << ePos << "guid:" << guid << "transfer:" << bTransfer << XEND;
  }

  return bSuccesss;
}

ItemBase* Package::getItem(const string& guid)
{
  for (DWORD i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    if (m_pPackage[i] == nullptr)
      continue;

    ItemBase* pItem = m_pPackage[i]->getItem(guid);
    if (pItem != nullptr)
      return pItem;
  }
  return nullptr;
}

ItemBase* Package::getItem(const string& guid, BasePackage** pPkg)
{

  for (DWORD i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    if (m_pPackage[i] == nullptr)
      continue;

    ItemBase* pItem = m_pPackage[i]->getItem(guid);
    (*pPkg) = m_pPackage[i];
    if (pItem != nullptr)
      return pItem;
  }
  *pPkg = nullptr;
  return nullptr;

}


ItemBase* Package::getTradeSellItem(const string& guid, BasePackage**pPkg)
{
  if (pPkg == nullptr)
    return nullptr;
  static std::vector<EPackType> sPackType = {EPACKTYPE_MAIN, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_BARROW};
  
  for (auto&s : sPackType)
  {
    if (m_pPackage[s] == nullptr)
      continue;

    ItemBase* pItem = m_pPackage[s]->getItem(guid);
    if (pItem != nullptr)
    {
      *pPkg = m_pPackage[s];
      return pItem;
    }
  }
  return nullptr;
}

bool Package::sellItem(const SellItem& cmd)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(cmd.npcid());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未在npc旁" << XEND;
    return false;
  }

  const TVecDWORD& vecParams = pNpc->getCFG()->stNpcFunc.getFunctionParam(ENPCFUNCTION_SELL);
  if (vecParams.empty() == true)
  {
    XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未在正确的npc旁" << XEND;
    return false;
  }
  if (vecParams[0] != ENPCFUNCTIONPARAM_NORMALSELL && vecParams[0] != ENPCFUNCTIONPARAM_HIGHSELL)
  {
    XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未在正确的npc旁" << XEND;
    return false;
  }

  // get package
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  BasePackage* pTempPack = getPackage(EPACKTYPE_TEMP_MAIN);
  BasePackage* pFoodPack = getPackage(EPACKTYPE_FOOD);
  if (pMainPack == nullptr || pTempPack == nullptr || pFoodPack == nullptr)
  {
    XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未找到" << EPACKTYPE_MAIN << EPACKTYPE_TEMP_MAIN << EPACKTYPE_FOOD << XEND;
    return false;
  }
  BasePackage* pPetPack = getPackage(EPACKTYPE_PET);
  if (pMainPack == nullptr || pTempPack == nullptr || pPetPack == nullptr)
  {
    XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未找到" << EPACKTYPE_MAIN << EPACKTYPE_TEMP_MAIN << EPACKTYPE_PET << XEND;
    return false;
  }

  TSetString setIDs;
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    const SItem& rItem = cmd.items(i);
    auto s = setIDs.find(rItem.guid());
    if (s != setIDs.end())
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出现重复道具" << *s << "出售" << XEND;
      return false;
    }
    setIDs.insert(rItem.guid());
  }

  // check item valid
  TVecItemData vecReturn;
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    const SItem& rItem = cmd.items(i);
    BasePackage* pPack = pMainPack;

    // get item
    ItemBase* pItem = pPack->getItem(rItem.guid());
    if (pItem == nullptr)
    {
      pPack = pTempPack;
      pItem = pPack->getItem(rItem.guid());
    }
    if (pItem == nullptr)
    {
      pPack = pFoodPack;
      pItem = pPack->getItem(rItem.guid());
    }
    if (pItem == nullptr)
    {
      pPack = pPetPack;
      pItem = pPack->getItem(rItem.guid());
    }
    if (pItem == nullptr)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "从" << pPack->getPackageType() << "出售 item:" << rItem.guid() << "不存在" << XEND;
      return false;
    }

    if(m_pUser->checkPwd(EUNLOCKTYPE_SELL, pItem->getTypeID()) == false)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << pItem->getTypeID() << "失败,密码验证失败" << XEND;
      return false;
    }


    // check equip
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pItem);
    if (pEquip == nullptr && vecParams[0] == ENPCFUNCTIONPARAM_HIGHSELL)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在高级回收出售 item:" << rItem.guid() << "普通物品" << XEND;
      return false;
    }

    if (pEquip != nullptr)
    {
      DWORD dwMaxSellRefineLv = MiscConfig::getMe().getItemCFG().dwMaxSellRefineLv;
      if (vecParams[0] == ENPCFUNCTIONPARAM_NORMALSELL && pEquip->getRefineLv() > dwMaxSellRefineLv)
      {
        XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "从" << pPack->getPackageType() << "出售装备 item:" << pEquip->getGUID() << pEquip->getTypeID() << rItem.count()
          << "精炼等级 :" << pEquip->getRefineLv() << "超过" << dwMaxSellRefineLv << "级出售限制" << XEND;
        return false;
      }
      const TMapEquipCard& mapCard = pEquip->getCardList();
      for (auto &m : mapCard)
      {
        ItemData oData;
        oData.mutable_base()->set_id(m.second.second);
        oData.mutable_base()->set_count(1);
        combineItemData(vecReturn, oData);
      }
    }

    //check item code
    if (pItem->getType() == EITEMTYPE_CODE)
    {
      ItemCode* pCode = dynamic_cast<ItemCode*>(pItem);
      if (pCode && !pCode->isExchanged())
      {
        XERR << "[包裹-物品出售], 未兑换的道具不可以出售" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "item:" << rItem.guid()<< XEND;
        return false;
      }
    }
    // get item config
    const SItemCFG* pCFG = pItem->getCFG();//ItemManager::getMe().getItemCFG(pItem->getTypeID());
    if (pCFG == nullptr) return false;
    if (pCFG->eItemType == EITEMTYPE_FUNCTION || pCFG->bNoSale)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << pPack->getPackageType()
        << "出售" << rItem.guid() << pCFG->dwTypeID << rItem.count() << "无法出售" << XEND;
      return false;
    }

    // check item count
    if (pPack->checkItemCount(rItem.guid(), rItem.count()) == false)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << pPack->getPackageType()
        << "出售" << rItem.guid() << pCFG->dwTypeID << rItem.count() << "数量不足" << XEND;
      return false;
    }
  }

  // check return
  if (pMainPack->checkAddItem(vecReturn, EPACKMETHOD_CHECK_WITHPILE) == false)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
    return false;
  }

  // process sell
  DWORD dwTotalSellZeny = 0;
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    const SItem& rItem = cmd.items(i);
    BasePackage* pPack = pMainPack;

    // get item
    ItemBase* pItem = pPack->getItem(rItem.guid());
    if (pItem == nullptr)
    {
      pPack = pTempPack;
      pItem = pPack->getItem(rItem.guid());
    }
    if (pItem == nullptr)
    {
      pPack = pFoodPack;
      pItem = pPack->getItem(rItem.guid());
    }
    if (pItem == nullptr)
    {
      pPack = pPetPack;
      pItem = pPack->getItem(rItem.guid());
    }
    if (pItem == nullptr)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << pPack->getPackageType() << "出售" << rItem.guid() << "不存在" << XEND;
      continue;
    }

    // check equip
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pItem);
    pPack->m_logRefineLv = 0;
    if (pEquip != nullptr)
    {
      DWORD retMoney;
      pPack->m_logRefineLv = pEquip->getRefineLv();
      if ((pEquip->getStrengthLv() != 0 || pEquip->getRefineLv() != 0) && decompose(pEquip->getGUID(), true, retMoney) == false)
      {
        XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "从" << pPack->getPackageType() << "出售装备 item:" << pEquip->getGUID() << pEquip->getTypeID() << "分解失败" << XEND;
        continue;
      }

      if (pEquip->EquipedCard() && equipAllCardOff(pItem->getGUID()) == false)
      {
        XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "从" << pPack->getPackageType() << "出售装备" << pEquip->getGUID() << pEquip->getTypeID() << "拆卡失败" << XEND;
        continue;
      }
    }

    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(pItem->getTypeID());
    if (pCFG == nullptr)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "从" << pPack->getPackageType() << "出售" << rItem.guid() << "配置指定无法出售" << XEND;
      continue;
    }
    if (pCFG->eItemType == EITEMTYPE_FUNCTION || pCFG->bNoSale)
    {
      XERR << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "从" << pPack->getPackageType() << "出售" << rItem.guid() << pCFG->dwTypeID << "配置指定无法出售" << XEND;
      continue;
    }

    pPack->reduceItem(rItem.guid(), ESOURCE_SELL, rItem.count());
    QWORD qwPrice = pCFG->dwSellPrice * rItem.count();
    qwPrice += floor(1.0 * qwPrice * m_pUser->getAttr(EATTRTYPE_SELLDISCOUNT) / 1000.0);
    m_pUser->addMoney(EMONEYTYPE_SILVER, qwPrice, ESOURCE_SELL);
    dwTotalSellZeny += qwPrice;
    XLOG << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从" << pPack->getPackageType() << "出售" << rItem.guid() << pCFG->dwTypeID << rItem.count() << "个物品" << XEND;

    m_pUser->getAchieve().onShopSell(dwTotalSellZeny);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  DWORD dwSellLast = m_pUser->getVar().getVarValue(EVARTYPE_SELL_WARNING_LAST);
  DWORD dwSellCur = m_pUser->getVar().getVarValue(EVARTYPE_SELL_WARNING_CUR) + dwTotalSellZeny;
  m_pUser->getVar().setVarValue(EVARTYPE_SELL_WARNING_CUR, dwSellCur);
  if (dwSellCur - dwSellLast >= MiscConfig::getMe().getSystemCFG().dwSellLimitWarning)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_SELL_WARNING_LAST, dwSellCur);
    stringstream sstr;
    sstr << "玩家," << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->name << ",当天出售累积" << dwSellCur << "zeny";
    MsgManager::alter_msg(thisServer->getFullName(), sstr.str(), EPUSHMSG_SHOP_SELL);
    XLOG << "[包裹-物品出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "当天累积 :" << dwSellCur << "zeny" << XEND;
  }

  return true;
}

bool Package::strength(EStrengthType type, const string& guid, DWORD count)
{
  if (count == 0)
  {
    XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "强化guid:" << guid << "失败,count = 0" << XEND;
    return false;
  }

  ENpcFunction eFunc = ENPCFUNCTION_MIN;
  if (type == ESTRENGTHTYPE_NORMAL)
    eFunc = ENPCFUNCTION_EQUIP_NORMAL_STRENGTH;
  else if (type == ESTRENGTHTYPE_GUILD)
    eFunc = ENPCFUNCTION_EQUIP_SPECIAL_STRENGTH;

  if (eFunc == ENPCFUNCTION_MIN)
  {
    XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "强化guid:" << guid << "失败,type :" << type << "未对应func" << XEND;
    return false;
  }

  // get equip
  ItemEquip* pEquip = nullptr;
  BasePackage* pPackage = getPackage(EPACKTYPE_EQUIP);
  if (pPackage == nullptr)
  {
    XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "强化guid:" << guid << "count :" << count << "失败,未找到" << EPACKTYPE_EQUIP << XEND;
    return false;
  }
  pEquip = dynamic_cast<ItemEquip*>(pPackage->getItem(guid));
  if (pEquip == nullptr && eFunc == ENPCFUNCTION_EQUIP_SPECIAL_STRENGTH)
  {
    pPackage = getPackage(EPACKTYPE_MAIN);
    if (pPackage == nullptr)
    {
      XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "强化guid:" << guid << "count :" << count << "失败,未找到" << EPACKTYPE_MAIN << XEND;
      return false;
    }
    pEquip = dynamic_cast<ItemEquip*>(pPackage->getItem(guid));
  }
  if (pEquip == nullptr || pEquip->getCFG() == nullptr || pEquip->getCFG()->isForbid(EFORBID_STRENGTH) == true)
  {
    XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "强化guid:" << guid << "count :" << count << "失败,未找到装备正确配置或者该装备被禁止强化" << XEND;
    return false;
  }

  SceneNpc* pNpc = m_pUser->getVisitNpcObj();
  DWORD maxLv = 0;
  if (eFunc == ENPCFUNCTION_EQUIP_NORMAL_STRENGTH)
    maxLv = m_pUser->getLevel();
  else if (eFunc == ENPCFUNCTION_EQUIP_SPECIAL_STRENGTH)
  {
    if (pNpc == nullptr || pNpc->getCFG() == nullptr)
    {
      XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "强化guid:" << guid << "失败,未在正确的npc旁" << XEND;
      return false;
    }
    if (pNpc->getCFG()->stNpcFunc.hasFunction(eFunc) == false)
    {
      XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "强化guid:" << guid << "失败,npc" << pNpc->tempid << pNpc->getNpcID() << "未包含func :" << eFunc << XEND;
      return false;
    }

    GuildScene* pScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
    if (!pScene)
    {
      XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "强化guid:" << guid << "count :" << count << "失败,未在公会场景" << XEND;
      return false;
    }
    DWORD dwLv = pScene->getGuild().getBuildingLevel(EGUILDBUILDING_MAGIC_SEWING);
    const SGuildBuildingCFG* pBuildingCFG = GuildConfig::getMe().getGuildBuildingCFG(EGUILDBUILDING_MAGIC_SEWING, dwLv);
    if (pBuildingCFG == nullptr)
    {
      XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "强化guid:" << guid << "count :" << count << "失败,该建筑" << EGUILDBUILDING_MAGIC_SEWING << "lv" << dwLv << "未在 Table_GuildBuilding.txt 表中找到" << XEND;
      return false;
    }
    if (pBuildingCFG->canStrength(pEquip->getEquipType()) == false)
    {
      XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "强化guid:" << guid << "count :" << count << "失败,该建筑" << EGUILDBUILDING_MAGIC_SEWING << "lv" << dwLv << "未能进行type :" << pEquip->getEquipType() << "装备强化" << XEND;
      return false;
    }
    maxLv = pBuildingCFG->dwMaxStrengthLv;
    pBuildingCFG = GuildConfig::getMe().getGuildBuildingCFG(EGUILDBUILDING_CAT_PILLOW, pScene->getGuild().getBuildingLevel(EGUILDBUILDING_CAT_PILLOW));
    if (pBuildingCFG)
      maxLv += pBuildingCFG->dwStrengthLvAdd;
  }
  else
  {
    XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "强化guid:" << guid << "count :" << count << "失败,func :" << eFunc << "未知func" << XEND;
    return false;
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "强化guid:" << guid << "count :" << count << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }

  // strength
  DWORD index = 0;
  DWORD criCount = 0;
  DWORD oldLv = eFunc == ENPCFUNCTION_EQUIP_NORMAL_STRENGTH ? pEquip->getStrengthLv() : pEquip->getStrengthLv2();
  EStrengthResult eResult = ESTRENGTHRESULT_SUCCESS;
  DWORD dwCost = 0;
  DWORD dwStrengthLv = eFunc == ENPCFUNCTION_EQUIP_NORMAL_STRENGTH ? pEquip->getStrengthLv() : pEquip->getStrengthLv2();
  while (index < count)
  {
    if (dwStrengthLv >= maxLv)
    {
      eResult = ESTRENGTHRESULT_MAXLV;
      break;
    }

    // check price
    if (eFunc == ENPCFUNCTION_EQUIP_NORMAL_STRENGTH)
    {
      DWORD price = LuaManager::getMe().call<DWORD>("calcEquipStrengthCost", dwStrengthLv, pEquip->getQuality(), pEquip->getType());
      if (m_pUser->checkMoney(EMONEYTYPE_SILVER, price) == false)
      {
        eResult = ESTRENGTHRESULT_NOMATERIAL;
        break;
      }

      // sub money
      m_pUser->subMoney(EMONEYTYPE_SILVER, price, ESOURCE_STRENGTH);

      // add cost
      pEquip->addStrengthCost(price);
      dwCost += price;
    }
    else if (eFunc == ENPCFUNCTION_EQUIP_SPECIAL_STRENGTH)
    {
      m_vecStrengthCost.clear();
      LuaManager::getMe().call<void>("calcStrengthCost", m_pUser, pEquip->getQuality(), pEquip->getType(), dwStrengthLv + 1);
      if (pMainPack->checkItemCount(m_vecStrengthCost) == false)
      {
        eResult = ESTRENGTHRESULT_NOMATERIAL;
        break;
      }
      pEquip->addStrengthCost(m_vecStrengthCost);
      pMainPack->reduceItem(m_vecStrengthCost, ESOURCE_STRENGTH);
    }

    dwStrengthLv += 1;
    ++index;

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_STRENGTH_SUM, 0, pEquip->getEquipType(), m_pUser->getLevel(), (DWORD) 1);

    if(m_pUser->m_oUserStat.checkAndSet(ESTATTYPE_STRENGTH_USER_COUNT, pEquip->getEquipType()))
      m_pUser->m_oUserStat.sendStatLog(ESTATTYPE_STRENGTH_USER_COUNT, 0, pEquip->getEquipType(), m_pUser->getLevel(), (DWORD)1);    
  }
  if (eFunc == ENPCFUNCTION_EQUIP_NORMAL_STRENGTH)
    pEquip->setStrengthLv(dwStrengthLv);
  else
    pEquip->setStrengthLv2(dwStrengthLv);

  // save data
  pPackage->setUpdateIDs(pEquip->getGUID());

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  // check event
  m_pUser->getEvent().onEquipStrength(guid);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EQUIP_STRENGTH);

  // achieve
  const SItemCFG* pCFG = pEquip->getCFG();
  if (pCFG != nullptr && eFunc == ENPCFUNCTION_EQUIP_NORMAL_STRENGTH)
    m_pUser->getAchieve().onEquip(EACHIEVECOND_STRENGTH);

  if (type == ESTRENGTHTYPE_GUILD)
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_EQUIP_FIRM);

  // effect
  if (eResult == ESTRENGTHRESULT_SUCCESS && type == ESTRENGTHTYPE_GUILD)
  {
    xLuaData data;
    data.setData("effect", MiscConfig::getMe().getRefineActionCFG().strSuccessEffect);
    data.setData("effectpos", 3.0f);
    data.setData("delay", MiscConfig::getMe().getRefineActionCFG().dwDelayMSec);
    GMCommandRuler::effect(m_pUser, data);

    if (pNpc != nullptr)
    {
      UserActionNtf message;
      message.set_type(EUSERACTIONTYPE_NORMALMOTION);
      message.set_value(MiscConfig::getMe().getStrengthActionCFG().dwGuildNpcAction);
      message.set_charid(pNpc->id);
      PROTOBUF(message, send2, len2);
      pNpc->xSceneEntryDynamic::sendCmdToNine(send2, len2);
    }
  }

  // inform client
  EquipStrength cmd;
  cmd.set_guid(pEquip->getGUID());
  cmd.set_destcount(count);
  cmd.set_count(index);
  cmd.set_cricount(criCount);
  cmd.set_oldlv(oldLv);
  cmd.set_newlv(dwStrengthLv);
  cmd.set_result(eResult);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[包裹-装备强化]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "强化guid:" << guid << "count :" << count << "进行func :" << eFunc << "成功,装备" << pEquip->getTypeID() << "强化等级" << oldLv << "->" << dwStrengthLv << XEND;

  //platlog
  std::stringstream ss;
  ss << EMONEYTYPE_SILVER << "," << dwCost << ";";

  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_EquipStrength;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  PlatLogManager::getMe().EquipUpLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    1,  /*1:强化 2：精炼*/
    index,/*count 强化次数*/
    pEquip->getTypeID(),
    pEquip->getGUID(),
    oldLv,
    dwStrengthLv,
    false, /*isFail*/
    ss.str(),
    "",/*cost item*/
    pEquip->isDamaged());

  return true;
}

bool Package::produce(DWORD id, QWORD qwNpcID, EProduceType eType, bool bQucik)
{
  const SComposeCFG* pBase = ComposeConfig::getMe().getComposeCFG(id);
  if (pBase == nullptr)
    return false;

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-物品合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成id:" << id << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }

  bool bQucikProduce = false;
  if(eType == EPRODUCETYPE_HEAD && bQucik == true)
    bQucikProduce = true;

  SceneNpc* pNpc = nullptr;
  if (pBase->eType == ECOMPOSETYPE_PACKAGE)
  {

  }
  else if (pBase->eType == ECOMPOSETYPE_NPC)
  {
    pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
    if (pNpc == nullptr && bQucikProduce == false)
      return false;
    if (pMainPack->isFull() == true)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }
  }
  else if (pBase->eType == ECOMPOSETYPE_TRADER)
  {
    EProfession profession = m_pUser->getProfession();
    if (profession != EPROFESSION_MERCHANT && profession != EPROFESSION_BLACKSMITH && profession != EPROFESSION_WHITESMITH && profession != EPROFESSION_MECHANIC)
      return false;
    if (m_pUser->getTransform().isInTransform())
    {
      XERR << "[商人-制作], 制作失败, 变身状态不可制作, 玩家:" << m_pUser->name << m_pUser->id << "composeid:" << id << XEND;
      return false;
    }
    if (pMainPack->isFull() == true)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }
  }
  else if (pBase->eType == ECOMPOSETYPE_ALCHEMIST)
  {
    if (pMainPack->isFull() == true)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }
  }
  else if (pBase->eType == ECOMPOSETYPE_RUNEKNIGHT)
  {
    if (m_pUser->getSkillLv(1268) < 1)
      return false;
    if (pMainPack->isFull() == true)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }
  }

  // process compose
  string guid;
  EPackMethod eMethod = EPACKMETHOD_NOCHECK;
  if (m_pUser->checkCompose(id, eMethod, GACTIVITY_MIN, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PRODUCE) == false)
    return false;
  guid = m_pUser->doCompose(id, eMethod, GACTIVITY_MIN, pNpc == nullptr, pNpc == nullptr, pNpc == nullptr, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PRODUCE);

  // play action
  if (pNpc != nullptr)
  {
    // update package first
    timer(xTime::getCurSec());

    const SProduceCFG& rCFG = MiscConfig::getMe().getProduceCFG();

    // npc pre action
    UserActionNtf cmd;
    cmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
    cmd.set_charid(pNpc->id);
    cmd.set_value(rCFG.dwPreAction);
    PROTOBUF(cmd, send, len);
    pNpc->sendCmdToNine(send, len);

    // npc effect
    ItemBase* pBaseItem = getItem(guid);
    if (pBaseItem != nullptr)
    {
      ProduceDone cmddone;
      cmddone.set_npcid(pNpc->id);
      cmddone.set_charid(m_pUser->id);
      cmddone.set_delay(rCFG.dwPreActionTime);
      cmddone.set_itemid(pBase->stProduct.dwTypeID);
      cmddone.set_type(eType);
      PROTOBUF(cmddone, send3, len3);
      pNpc->sendCmdToNine(send3, len3);
    }
  }

  XLOG << "[包裹-物品合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成id:" << id << "result:" << guid << XEND;
  return true;
}

#include "BaseConfig.h"
//bool Package::refine(const string& guid, DWORD composeid, QWORD npcid, bool safeRefine, bool oneclick/*=false*/)
bool Package::refine(const EquipRefine& cmd, bool oneclick/*=false*/)
{
  ItemEquip* pEquip = nullptr;
  BasePackage* pPackage = nullptr;
  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pMainPack == nullptr || pEquipPack == nullptr)
      return false;
  pPackage = pMainPack;
  pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(cmd.guid()));
  if (pEquip == nullptr)
  {
    pPackage = pEquipPack;
    pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(cmd.guid()));
  }
  if (pEquip == nullptr || pEquip->isDamaged() || pEquip->getCFG() == nullptr || pEquip->getCFG()->isForbid(EFORBID_REFINE) == true)
      return false;

  if(m_pUser->checkPwd(EUNLOCKTYPE_REFINE,pEquip->getRefineLv()) == false)
      return false;

  if (pMainPack->isFull() == true)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
    return false;
  }

  SceneNpc* pNpc = m_pUser->getVisitNpcObj();
  if (pNpc == nullptr)
  {
    XERR << "[装备精炼]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "精炼" << cmd.ShortDebugString() << "失败,未在正确的npc旁" << XEND;
    return false;
  }

  // get config
  const SRefineCFG* pRefineCFG = ItemManager::getMe().getRefineCFG(pEquip->getType(), pEquip->getQuality(), pEquip->getRefineLv() + 1);
  if (pRefineCFG == nullptr)
    return false;

  if (!pEquip->getCFG())
    return false;

  ENpcFunction eFunc = ENPCFUNCTION_MIN;
  if (pNpc->getCFG()->stNpcFunc.hasFunction(ENPCFUNCTION_EQUIP_REFINE) == true)
    eFunc = ENPCFUNCTION_EQUIP_REFINE;
  else if (pNpc->getCFG()->stNpcFunc.hasFunction(ENPCFUNCTION_EQUIP_SPECIAL_REFINE) == true)
    eFunc = ENPCFUNCTION_EQUIP_SPECIAL_REFINE;
  else
  {
    XERR << "[装备精炼]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "精炼" << cmd.ShortDebugString() << "失败,位置func" << XEND;
    return false;
  }

  DWORD dwMaxLv = 0;
  if (eFunc == ENPCFUNCTION_EQUIP_REFINE)
    dwMaxLv = pEquip->getCFG()->dwMaxRefineLv;
  else if (eFunc == ENPCFUNCTION_EQUIP_SPECIAL_REFINE)
  {
    GuildScene* pScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
    if (!pScene)
    {
      XERR << "[装备精炼]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "精炼" << cmd.ShortDebugString() << "失败,未在公会场景" << XEND;
      return false;
    }
    DWORD dwLv = pScene->getGuild().getBuildingLevel(EGUILDBUILDING_MAGIC_SEWING);
    const SGuildBuildingCFG* pBuildingCFG = GuildConfig::getMe().getGuildBuildingCFG(EGUILDBUILDING_MAGIC_SEWING, dwLv);
    if (pBuildingCFG == nullptr)
    {
      XERR << "[装备精炼]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "精炼" << cmd.ShortDebugString() << "失败,该建筑" << EGUILDBUILDING_MAGIC_SEWING << "lv" << dwLv << "未在 Table_GuildBuilding.txt 表中找到" << XEND;
      return false;
    }
    if (pBuildingCFG->canRefine(pEquip->getEquipType()) == false)
    {
      XERR << "[装备精炼]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "精炼" << cmd.ShortDebugString() << "失败,该建筑" << EGUILDBUILDING_MAGIC_SEWING << "lv" << dwLv << "未能进行type :" << pEquip->getEquipType() << "装备精炼" << XEND;
      return false;
    }
    dwMaxLv = pBuildingCFG->dwMaxRefineLv;
  }

  if (dwMaxLv != 0 && dwMaxLv <= pEquip->getRefineLv())
  {
    XERR << "[精炼], 当前精炼等级已达上限, 装备:" << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }

  GlobalActivityType eGAType = GACTIVITY_NORMAL_REFINE;
  if(cmd.saferefine() == false)
  {
    const TVecDWORD& vecComposeIDs = pRefineCFG->sComposeRate.vecCompose;
    auto v = find(vecComposeIDs.begin(), vecComposeIDs.end(), cmd.composeid());
    if (v == vecComposeIDs.end())
      return false;
  }
  else
  {
    const TVecDWORD& vecComposeIDs = pRefineCFG->sComposeRate.vecSafeCompose;
    auto v = find(vecComposeIDs.begin(), vecComposeIDs.end(), cmd.composeid());
    if (v == vecComposeIDs.end())
    {
      XLOG << "[精炼-安全精炼], 失败:" << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << "没有找到对应composeID : " << cmd.composeid() << XEND;
      return false;
    }
    eGAType = GACTIVITY_SAFE_REFINE;
  }

  if(pEquip->getCFG()->isLotteryEquip())
    eGAType = GACTIVITY_MIN;

  // check compose
  EPackFunc ePackFunc = EPACKFUNC_REFINE;
  if (m_pUser->checkCompose(cmd.composeid(), EPACKMETHOD_NOCHECK, eGAType, ECHECKMETHOD_NONORMALEQUIP, ePackFunc) == false)
    return false;

  if(cmd.saferefine() == true)
  {
    DWORD needCount = 0;
    if (pEquip->getCFG()->isLotteryEquip())
      needCount = MiscConfig::getMe().getSafeRefineLotteryCount(pEquip->getRefineLv() + 1);
    else
    {
      bool isOpen = ActivityManager::getMe().isOpen(GACTIVITY_SAFE_REFINE_DISCOUNT);
      needCount = MiscConfig::getMe().getSafeRefineCount(pEquip->getRefineLv() + 1, isOpen);
    }

    if(needCount == 0)
    {
      XLOG << "[精炼-安全精炼] " << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << "消耗装备数量为零: " << needCount << XEND;
    }
    else if(needCount == DWORD_MAX)
    {
      XLOG << "[精炼-安全精炼], 失败: " << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv()+1 << "玩家:" << m_pUser->name << m_pUser->id << "配置中没有安全精炼等级: " << needCount << XEND;
      return false;
    }
    else
    {
      TSetString setIDs;
      /*for (int i = 0; i < cmd.itemguid_size(); ++i)
      {
        ItemBase* pStuff = pMainPack->getItem(cmd.itemguid(i));
        ItemEquip* pItem = dynamic_cast<ItemEquip*> (pStuff);
        if(pItem == nullptr || !pEquip->getCFG()->isRepairId(pItem->getCFG()->dwVID) || cmd.itemguid(i) == cmd.guid())
        {
          XLOG << "[精炼-安全精炼], 失败:" << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << "未找到对应装备: " << cmd.itemguid(i) << XEND;
          return false;
        }

        setIDs.insert(cmd.itemguid(i));
        if(static_cast<DWORD>(i+1) >= needCount)
          break;
      }

      if(setIDs.size() != needCount)
      {
        XLOG << "[精炼-安全精炼], 失败:" << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << "消耗装备数量不正确: " << needCount << setIDs.size() << XEND;
        return false;
      }

      for(auto &v : setIDs)
        pMainPack->reduceItem(v, ESOURCE_EQUIP);*/

      for (int i = 0; i < cmd.itemguid_size(); ++i)
        setIDs.insert(cmd.itemguid(i));
      if(setIDs.size() != needCount)
      {
        XLOG << "[精炼-安全精炼], 失败:" << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << "消耗装备数量不正确: " << needCount << setIDs.size() << XEND;
        return false;
      }
      for (auto &s : setIDs)
      {
        if (checkItemCount(s, 1, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_REFINE) == false)
        {
          XERR << "[精炼-安全精炼], 失败:" << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << "未找到对应装备: " << s << XEND;
          return false;
        }
      }
      for (auto &s : setIDs)
        reduceItem(s, 1, ESOURCE_EQUIP, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_REFINE);
    }
  }

  //perform compose
  m_pUser->doCompose(cmd.composeid(), EPACKMETHOD_NOCHECK, eGAType, true, true, true, ECHECKMETHOD_NONORMALEQUIP, ePackFunc);

  if (!oneclick && pNpc)
  {
    // broadcast refine-npc action
    UserActionNtf message;
    message.set_type(EUSERACTIONTYPE_NORMALMOTION);
    DWORD dwAction = eFunc == ENPCFUNCTION_EQUIP_SPECIAL_REFINE ? MiscConfig::getMe().getRefineActionCFG().dwGuildNpcAction : MiscConfig::getMe().getRefineActionCFG().dwNpcAction;
    message.set_value(dwAction);
    message.set_charid(pNpc->id);
    PROTOBUF(message, send2, len2);
    pNpc->xSceneEntryDynamic::sendCmdToNine(send2, len2);
  }

  const SComposeCFG* pCompose = ComposeConfig::getMe().getComposeCFG(cmd.composeid());
  if (pCompose == nullptr)
    return false;
  if (pCompose->vecMaterial.size() != 1)
    return false;
  DWORD stuffid = pCompose->vecMaterial[0].id();
  DWORD succRateA = MiscConfig::getMe().getRefineRate(stuffid);

  DWORD successRate = pRefineCFG->sComposeRate.dwSuccessRate + succRateA;
  DWORD failStatyRate = pRefineCFG->sComposeRate.dwFailStayRate;
  DWORD failNoDamageRate = pRefineCFG->sComposeRate.dwFailNoDamageRate;

  ERefineResult eResult = EREFINERESULT_MIN;
  DWORD dwOldRefineLv = pEquip->getRefineLv();

  bool randSuccess = false;
  if(cmd.saferefine() == true)
  {
    randSuccess = true;
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_SAFE_REFINE);
    XLOG << "[精炼-安全精炼], 成功:" << pEquip->getTypeID() << "等级:" << pEquip->getRefineLv() << "玩家:" << m_pUser->name << m_pUser->id << XEND;
  }
  else
  {

    float extraRate = LuaManager::getMe().call<float>("CalcRefineExtraRate", (xSceneEntryDynamic*)m_pUser, dwOldRefineLv);

    const SRefineActionCFG& rCFG = MiscConfig::getMe().getRefineActionCFG();

  //  bool randSuccess = false;
    if (dwOldRefineLv >= rCFG.dwBeginChangeRateLv)
    {
      DWORD dwRepairChRate = rCFG.fRepairPerRate * ONE_THOUSAND;
      DWORD dwChSuccessRate = successRate * ONE_THOUSAND + dwRepairChRate;
      if (pEquip->isLastRefineFail())
      {
        DWORD dwChRate = rCFG.fLastFailAddRate * ONE_THOUSAND;
        dwChSuccessRate += dwChRate;
      }
      else
      {
        DWORD dwChRate = rCFG.fLastSuccessDecRate * ONE_THOUSAND;
        dwChSuccessRate = dwChSuccessRate > dwChRate ? dwChSuccessRate - dwChRate : 0;
      }

      dwChSuccessRate += (extraRate * ONE_THOUSAND);

      XDBG << "[精炼-概率], 玩家:" << m_pUser->name << m_pUser->id << "道具:" << pEquip->getTypeID() << "等级:" << dwOldRefineLv << "配置概率:" << successRate * ONE_THOUSAND << "/100000, 修正概率:" << dwChSuccessRate << "/100000" << XEND;
      if ((DWORD)randBetween(1, 100 * ONE_THOUSAND) <= dwChSuccessRate)
        randSuccess = true;
    }
    else
    {
      DWORD dwChSuccessRate = successRate * ONE_THOUSAND + extraRate * ONE_THOUSAND;
      if ((DWORD)randBetween(1, 100 * ONE_THOUSAND) <= dwChSuccessRate)
        randSuccess = true;
    }
    if (eFunc == ENPCFUNCTION_EQUIP_SPECIAL_REFINE)
      m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_HEAD_REFINE);
    else if(eFunc == ENPCFUNCTION_EQUIP_REFINE)
      m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EQUIP_REFINE);
  }

  if (randSuccess)
  {
    // only success add refine consume for decompose
    pEquip->addRefineCompose(cmd.composeid());
    // level up
    pEquip->setRefineLv(pEquip->getRefineLv() + 1);
    pEquip->setLastRefineFail(false);
    eResult = EREFINERESULT_SUCCESS;

    // 成功表情
    const TVecDWORD& vecEmoji = MiscConfig::getMe().getRefineActionCFG().vecSuccessEmoji;
    if (vecEmoji.size() > 0)
    {
      DWORD randIndex = randBetween(0, vecEmoji.size() - 1);
      if (TableManager::getMe().getExpressionCFG(vecEmoji[randIndex]) != nullptr)
      {
        UserActionNtf message;
        message.set_type(EUSERACTIONTYPE_EXPRESSION);
        message.set_value(vecEmoji[randIndex]);
        message.set_charid(m_pUser->id);
        message.set_delay(MiscConfig::getMe().getRefineActionCFG().dwDelayMSec);
        PROTOBUF(message, send, len);
        m_pUser->sendCmdToNine(send, len);
      }
      randIndex = randBetween(0, vecEmoji.size() - 1);
      if (TableManager::getMe().getExpressionCFG(vecEmoji[randIndex]) != nullptr)
      {
        if (!oneclick && pNpc)
        {
          UserActionNtf message;
          message.set_type(EUSERACTIONTYPE_EXPRESSION);
          message.set_value(vecEmoji[randIndex]);
          message.set_charid(pNpc->id);
          message.set_delay(MiscConfig::getMe().getRefineActionCFG().dwDelayMSec);
          PROTOBUF(message, send, len);
          pNpc->sendCmdToNine(send, len);
        }
      }
    }
    xLuaData data;
    //data.setData("effect", "Common/24ForgingSuccess");
    data.setData("effect", MiscConfig::getMe().getRefineActionCFG().strSuccessEffect);
    data.setData("effectpos", 3.0f);
    data.setData("delay", MiscConfig::getMe().getRefineActionCFG().dwDelayMSec);
    GMCommandRuler::effect(m_pUser, data);
    XLOG << "[装备精炼-成功]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "精炼guid:" << cmd.guid() << "typeid:" << pEquip->getTypeID() << "当前精炼等级:" << pEquip->getRefineLv() << XEND;
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_REFINE_SUCCESS_COUNT, 0, 0, 0, (DWORD)1);
    m_pUser->getShare().addNormalData(ESHAREDATATYPE_S_REFINESUCCESS, 1);

    const SItemCFG* pCFG = pEquip->getCFG();
    if (pCFG != nullptr)
    {
      if (ItemManager::getMe().isWeapon(pCFG->eItemType) == true || pCFG->eItemType == EITEMTYPE_ACCESSORY)
        m_pUser->getAchieve().onEquip(EACHIEVECOND_REFINE_WEAPON);
      else
        m_pUser->getAchieve().onEquip(EACHIEVECOND_REFINE_EQUIP);

      m_pUser->getAchieve().onRefine(true);
    }
    m_pUser->getUserPet().playAction(EPETACTION_OWNER_REFINE_SUCCESS);
  }
  else
  {
    bool bDamage = false;
    m_pUser->getAchieve().onRefine(false);
    pEquip->setLastRefineFail(true);
    m_pUser->getUserPet().playAction(EPETACTION_OWNER_REFINE_FAIL);
    if ((DWORD)randBetween(1, 100) > failNoDamageRate)
    {
      pEquip->setDamageStatus(true);
      bDamage = true;
      //MsgManager::sendMsg(m_pUser->id, 228);

      // 失败后保留属性
      //if (pEquipPack != nullptr)
        //m_pUser->getEvent().onEquipChange(EEQUIPOPER_OFF, guid);

      XLOG << "[装备精炼-失败], 装备损坏" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "精炼guid:" << cmd.guid() << "typeid:" << pEquip->getTypeID() << "当前精炼等级:" << pEquip->getRefineLv() << XEND;
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_REFINE_DAMAGE_COUNT, 0, 0, 0, (DWORD)1);

      XLOG << "[装备精炼-失败], 装备精炼" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "精炼guid:" << cmd.guid() << "typeid:" << pEquip->getTypeID() << "当前精炼等级:" << pEquip->getRefineLv() << XEND;
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_EQUIP_REFINE_DAMAGE_COUNT, pEquip->getTypeID(), 0, 0, (DWORD)1); 
      m_pUser->getShare().addNormalData(ESHAREDATATYPE_S_REFINEDAMAGE, 1);
    }
    else {
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_REFINE_FAIL_COUNT, 0, 0, 0, (DWORD)1);
    }

    if ((DWORD)randBetween(1, 100) <= failStatyRate)
    {
      // level staty
      eResult = bDamage ? EREFINERESULT_FAILSTAYDAM : EREFINERESULT_FAILSTAY;
      XLOG << "[装备精炼-失败], 保持当前等级" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "精炼guid:" << cmd.guid() << "typeid:" << pEquip->getTypeID() << "当前精炼等级:" << pEquip->getRefineLv() << XEND;
    }
    else
    {
      // level dec
      DWORD lv = pEquip->getRefineLv() > 0 ? pEquip->getRefineLv() - 1 : 0;
      pEquip->setRefineLv(lv);
      eResult = bDamage? EREFINERESULT_FAILBACKDAM : EREFINERESULT_FAILBACK;
      XLOG << "[装备精炼-失败], 精炼等级-1 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "精炼guid:" << cmd.guid() << "typeid:" << pEquip->getTypeID() << "当前精炼等级:" << pEquip->getRefineLv() << XEND;
    }

    xLuaData data;
    //data.setData("effect", "Common/25ForgingFailure");
    data.setData("effect", MiscConfig::getMe().getRefineActionCFG().strFailEffect);
    data.setData("effectpos", 3.0f);
    data.setData("delay", MiscConfig::getMe().getRefineActionCFG().dwDelayMSec);
    GMCommandRuler::effect(m_pUser, data);

    const TVecDWORD& vecEmoji = MiscConfig::getMe().getRefineActionCFG().vecFailEmoji;
    if (vecEmoji.size() > 0)
    {
      DWORD randIndex = randBetween(0, vecEmoji.size() - 1);
      if (TableManager::getMe().getExpressionCFG(vecEmoji[randIndex]) != nullptr)
      {
        UserActionNtf message;
        message.set_type(EUSERACTIONTYPE_EXPRESSION);
        message.set_value(vecEmoji[randIndex]);
        message.set_charid(m_pUser->id);
        message.set_delay(MiscConfig::getMe().getRefineActionCFG().dwDelayMSec);
        PROTOBUF(message, send, len);
        m_pUser->sendCmdToNine(send, len);
      }
      randIndex = randBetween(0, vecEmoji.size() - 1);
      if (TableManager::getMe().getExpressionCFG(vecEmoji[randIndex]) != nullptr)
      {
        if (!oneclick && pNpc)
        {
          UserActionNtf message;
          message.set_type(EUSERACTIONTYPE_EXPRESSION);
          message.set_value(vecEmoji[randIndex]);
          message.set_charid(pNpc->id);
          message.set_delay(MiscConfig::getMe().getRefineActionCFG().dwDelayMSec);
          PROTOBUF(message, send, len);
          pNpc->sendCmdToNine(send, len);
        }
      }
    }
  }

  // save data
  pPackage->setUpdateIDs(pEquip->getGUID());

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  // return client
  EquipRefine ret;
  ret.set_guid(cmd.guid());
  ret.set_refinelv(pEquip->getRefineLv());
  ret.set_eresult(eResult);
  PROTOBUF(ret, send1, len1);
  m_pUser->sendCmdToMe(send1, len1);

  // refine cause buff 高亮
  refreshEnableBuffs(pEquip);

  XLOG << "[包裹-装备精炼]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name<< "精炼guid:" << 
    cmd.guid() << "typeid:" << pEquip->getTypeID() << "装备 quality:" << pEquip->getQuality() << "composeid:" << cmd.composeid() << XEND;

  bool bIsFail = true;
  if (eResult == EREFINERESULT_SUCCESS)
  {
    bIsFail = false;
    m_pUser->playDynamicExpression(EAVATAREXPRESSION_SMILE);
  }
  else
  {
    bIsFail = true;
    m_pUser->playDynamicExpression(EAVATAREXPRESSION_HIT);
  }

  m_pUser->m_oBuff.onEquipRefineChange(pEquip, !bIsFail);

  //platlog
  do
  {
    std::stringstream ss1;
    std::stringstream ss2;
    const SComposeCFG* pBase = ComposeConfig::getMe().getComposeCFG(cmd.composeid());
    if (pBase == nullptr) break;
    if (pBase->dwROB != 0)
    {
      ss1 << EMONEYTYPE_SILVER << "," << pBase->dwROB << ";";
    }
    if (pBase->dwGold != 0)
    {
      ss1 << EMONEYTYPE_GOLD << "," << pBase->dwGold << ";";
    }
    if (pBase->dwDiamond != 0)
    {
      ss1 << EMONEYTYPE_DIAMOND << "," << pBase->dwDiamond << ";";
    }

    for (auto v = pBase->vecMaterial.begin(); v != pBase->vecMaterial.end(); ++v)
    {
      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
      if (pCFG == nullptr)
        continue;;
      
      ss2 << pCFG->strNameZh << "," << v->count() << ";";
    }
    for (auto v = pBase->vecConsume.begin(); v != pBase->vecConsume.end(); ++v)
    {
      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
      if (pCFG == nullptr)
        continue;
      ss2 << pCFG->strNameZh << "," << v->count() << ";";
    }

    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_EquipRefine;
    PlatLogManager::getMe().eventLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eid,
      m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().EquipUpLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eType,
      eid,
      2,  /*1:强化 2：精炼*/
      1,/*count 强化次数*/
      pEquip->getTypeID(),
      pEquip->getGUID(),
      dwOldRefineLv,
      pEquip->getRefineLv(),
      bIsFail, /*isFail*/
      ss1.str(),
      ss2.str(),
      pEquip->isDamaged());
    
    //StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_REFINE_USER_COUNT, m_pUser->id, 0, m_pUser->getLevel(),(DWORD)1);
    if (m_pUser->m_oUserStat.checkAndSet(ESTATTYPE_REFINE_USER_COUNT))
      m_pUser->m_oUserStat.sendStatLog(ESTATTYPE_REFINE_USER_COUNT, 0, 0, m_pUser->getLevel(), (DWORD)1);

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_REFINE_COUNT, 0, 0, 0,(DWORD)1);    
    std::hash<std::string> hash_fn;
    QWORD key = hash_fn(pEquip->getGUID());
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_REFINE_LEVEL_COUNT, key, pEquip->getRefineLv(), 0, (DWORD)1);

    m_pUser->getShare().addNormalData(ESHAREDATATYPE_S_REFINECOUNT, 1);

  } while (0);


  return true;
}

bool Package::refineForce(const string& guid, DWORD newLv, DWORD composeId, DWORD count)
{
  // get equip
  ItemEquip* pEquip = nullptr;
  BasePackage* pPackage = nullptr;
  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pMainPack == nullptr || pEquipPack == nullptr)
    return false;
  pPackage = pMainPack;
  pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(guid));
  if (pEquip == nullptr)
  {
    pPackage = pEquipPack;
    pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(guid));
  }
  if (pEquip == nullptr)
    return false;

  // only success add refine consume for decompose
  pEquip->addRefineCompose(composeId, count);
  // level up
  pEquip->setRefineLv(newLv);
  ERefineResult eResult = EREFINERESULT_SUCCESS;

  xLuaData data;
  data.setData("effect", "Common/24ForgingSuccess");
  data.setData("effectpos", 1.0f);
  GMCommandRuler::effect(m_pUser, data);


  // save data
  pPackage->setUpdateIDs(pEquip->getGUID());

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  // return client
  EquipRefine ret;
  ret.set_guid(guid);
  ret.set_refinelv(pEquip->getRefineLv());
  ret.set_eresult(eResult);
  PROTOBUF(ret, send1, len1);
  m_pUser->sendCmdToMe(send1, len1);

  // refine cause buff 高亮
  refreshEnableBuffs(pEquip);

  XLOG << "[包裹-装备精炼-强制]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "精炼guid:" << guid << "typeid:" << pEquip->getTypeID() << "装备 quality:" << pEquip->getQuality();
  return true;

}

void Package::backStrengthCost(ItemEquip* pEquip)
{
  if (m_pUser == nullptr || pEquip == nullptr)
    return;

  TVecItemInfo vecItems;
  if (pEquip->getStrengthCost() != 0)
  {
    ItemInfo oItem;
    oItem.set_id(ITEM_ZENY);
    oItem.set_count(pEquip->getStrengthCost());
    oItem.set_source(ESOURCE_UNSTRENGTH);
    combinItemInfo(vecItems, TVecItemInfo{oItem});
  }

  if (pEquip->getStrengthItemCost().empty() == false)
    combinItemInfo(vecItems, pEquip->getStrengthItemCost());

  if (vecItems.empty() == false && addItem(vecItems, EPACKMETHOD_AVAILABLE) == false)
  {
    XERR << "[包裹-强化材料]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "返回材料 :" << vecItems << "失败,添加失败" << XEND;
    return;
  }
  XLOG << "[包裹-强化材料]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "返回材料 :" << vecItems << "成功" << XEND;
}

void Package::addStrengthCost(DWORD dwItemID, DWORD dwCount)
{
  ItemInfo oItem;
  oItem.set_id(dwItemID);
  oItem.set_count(dwCount);
  combinItemInfo(m_vecStrengthCost, oItem);
}

bool Package::repair(const string& targetGuid, const string& stuffid)
{
  // get equip
  BasePackage* pTargetPack = nullptr;
  ItemEquip* pTargetEquip = dynamic_cast<ItemEquip*>(getItem(targetGuid, &pTargetPack));
  if (pTargetEquip == nullptr || pTargetEquip->isDamaged() == false || pTargetEquip->getCFG() == nullptr)
  {
    XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "失败,未找到该装备或装备未损坏" << XEND;
    return false;
  }
  if (pTargetPack == nullptr)
  {
    XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "失败,未找到装备所在包裹" << XEND;
    return false;
  }
  EPackType eType = pTargetPack->getPackageType();
  if (eType != EPACKTYPE_MAIN && eType != EPACKTYPE_EQUIP)
  {
    XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "使用" << stuffid << "修复装备" << targetGuid << "失败,只能修复" << EPACKTYPE_MAIN << "和" << EPACKTYPE_EQUIP << "中的装备" << XEND;
    return false;
  }

  // get stuff
  BasePackage* pStuffPack = nullptr;
  ItemBase* pStuff = getItem(stuffid, &pStuffPack);
  if (pStuff == nullptr || pStuff->getCFG() == nullptr)
  {
    XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "失败,材料未找到" << XEND;
    return false;
  }
  if (pStuffPack == nullptr || pStuffPack->getPackageType() == EPACKTYPE_STORE)
  {
    XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "失败,未找到材料所在的包裹" << XEND;
    return false;
  }

  const TSetDWORD& setIDs = MiscConfig::getMe().getPackageCFG().getPackFunc(EPACKFUNC_REPAIR);
  auto s = setIDs.find(pStuffPack->getPackageType());
  if (s == setIDs.end())
  {
    XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "使用" << stuffid << "修复装备" << targetGuid << "失败,材料所在" << pStuffPack->getPackageType() << "不可作为材料来源" << XEND;
    return false;
  }

  // check stuff
  DWORD dwStuffTypeID = pStuff->getTypeID();
  ItemEquip* pStuffEquip = dynamic_cast<ItemEquip*>(pStuff);
  if (pStuffEquip != nullptr)
  {
    //2016-11-08 去掉附魔拦截
    const SRefineActionCFG& rCFG = MiscConfig::getMe().getRefineActionCFG();
    if (pStuffEquip->getRefineLv() > rCFG.dwRepairMaxLv)
    {
      XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << stuffid << "修复装备" << targetGuid << "失败,材料" << pStuff->getTypeID() << "精炼等级大于保护上限" << rCFG.dwRepairMaxLv << XEND;
      return false;
    }

    DWORD dwTempID = pTargetEquip->getTypeID() > 100000 ? pTargetEquip->getTypeID() - 100000 : pTargetEquip->getTypeID();
    const SEquipComposeCFG* pCFG = ItemConfig::getMe().getEquipComposeCFG(dwTempID);
    if (pCFG != nullptr)
    {
      if (pCFG->vecMaterialEquip.empty() == true)
      {
        XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "使用" << stuffid << "修复装备" << targetGuid << "失败,目标装备" << pTargetEquip->getTypeID() << "在 Table_EquipCompose.txt 未发现主装备" << XEND;
        return false;
      }
      DWORD dwNeedID = pCFG->vecMaterialEquip[0].base().id();
      if (pStuffEquip->getTypeID() != dwNeedID)
      {
        XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "使用" << stuffid << "修复装备" << targetGuid << "失败,目标装备" << pTargetEquip->getTypeID() << "未合成装备,需要" << dwNeedID << "修复" << XEND;
        return false;
      }
    }
    else if (!pTargetEquip->getCFG()->isRepairId(pStuffEquip->getCFG()->dwVID))
    {
      XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << stuffid << "修复装备" << targetGuid << "失败,vid" << pTargetEquip->getCFG()->dwVID << pStuffEquip->getCFG()->dwVID << "不匹配" << XEND;
      return false;
    }

    // return strength cost
    backStrengthCost(pStuffEquip);

    // return card
    TVecEquipCard vecCards;
    pStuffEquip->getEquipCard(vecCards);
    for (auto &v : vecCards)
    {
      pStuffEquip->removeCard(v.first);
      ItemInfo oItem;
      oItem.set_id(v.second);
      oItem.set_count(1);
      oItem.set_source(ESOURCE_REPAIR);
      if (addItem(oItem, EPACKMETHOD_AVAILABLE) == false)
        XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "返还材料卡片" << v.second << "失败" << XEND;
      else
        XLOG << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "返还材料卡片" << v.second << "成功" << XEND;
    }
    pStuffPack->reduceItem(stuffid, ESOURCE_REPAIR, pStuff->getCount());
  }
  else
  {
    if (ItemConfig::getMe().isLotteryHead(pTargetEquip->getTypeID()) == false)
    {
      XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "失败,该装备不是Lottery表中物品,无法使用材料修复" << XEND;
      return false;
    }
    const SLotteryRepair* pRepairCFG = MiscConfig::getMe().getLotteryCFG().getRepairCFG(pStuff->getTypeID());
    if (pRepairCFG == nullptr)
    {
      XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "失败,装备无法使用" << pStuff->getTypeID() << "修复" << XEND;
      return false;
    }
    DWORD dwCount = pRepairCFG->getCount(pTargetEquip->getQuality());
    if (dwCount == 0)
    {
      XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用" << stuffid << "修复装备" << targetGuid << "失败" << pStuff->getTypeID() << "无法修复品质" << pTargetEquip->getQuality() << "的装备" << XEND;
      return false;
    }

    ItemData oData;
    pStuff->toItemData(&oData);
    oData.mutable_base()->set_count(dwCount);
    if (checkItemCount(oData.base(), ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_REPAIR) == false)
    {
      XERR << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << "修复装备" << targetGuid << "失败" << pStuff->getTypeID() << "数量不足" << XEND;
      return false;
    }
    reduceItem(oData.base(), ESOURCE_REPAIR, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_REPAIR);
  }

  pTargetEquip->setDamageStatus(false);
  pTargetEquip->addRepairCount();

  // notify client
  EquipRepair cmd;
  cmd.set_targetguid(targetGuid);
  cmd.set_success(true);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  pTargetPack->setUpdateIDs(targetGuid);

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  XLOG << "[包裹-装备修复]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << stuffid << dwStuffTypeID << "修复装备" << targetGuid << pTargetEquip->getTypeID() << "成功" << XEND;
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_REPAIR_COUNT, 0, 0, 0, (DWORD)1);
  return true;
}

bool Package::saveHint(DWORD dwItemID)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-道具提示]" << "保存" << dwItemID << "未找到玩家" << XEND;
    return false;
  }

  if (MiscConfig::getMe().getItemCFG().isExtraHint(dwItemID) == false)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
    if (pCFG == nullptr || pCFG->eEquipType == EEQUIPTYPE_MIN)
    {
      XERR << "[包裹-道具提示]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存" << dwItemID << "失败,只有装备才能存储" << XEND;
      return false;
    }
  }

  m_setHintItemIDs.insert(dwItemID);
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
    browsePackage(static_cast<EPackType>(i), false);
  XLOG << "[包裹-道具提示]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存" << dwItemID << "成功" << XEND;
  return true;
}

bool Package::saveOnce(DWORD dwItemID)
{
  if (m_pUser == nullptr)
    return false;

  if (m_setOnceItemIDs.find(dwItemID) != m_setOnceItemIDs.end())
    return false;

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
  if (pCFG == nullptr || pCFG->eItemType != EITEMTYPE_QUEST_ONCE)
  {
    XERR << "[包裹-单次道具]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存" << dwItemID << "失败,只有任务类型才能被保存" << XEND;
    return false;
  }

  m_setOnceItemIDs.insert(dwItemID);
  XLOG << "[包裹-单次道具]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存" << dwItemID << "成功" << XEND;
  return true;
}

bool Package::enchant(EEnchantType eType, const string& guid)
{
  if(m_pUser->checkPwd(EUNLOCKTYPE_ENCHANT) == false)
  {
    XERR << "[包裹-装备附魔]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "装备附魔:" << eType << "失败,密码验证失败" << XEND;
    return false;
  }

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-装备附魔]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行" << eType << "附魔失败,未在npc旁" << XEND;
    return false;
  }
  if ((eType == EENCHANTTYPE_PRIMARY && pNpc->getCFG()->stNpcFunc.hasFunction(ENPCFUNCTION_EQUIP_ENCHANT_PRIMARY) == false) ||
      (eType == EENCHANTTYPE_MEDIUM && pNpc->getCFG()->stNpcFunc.hasFunction(ENPCFUNCTION_EQUIP_ENCHANT_MEDIUM) == false) ||
      (eType == EENCHANTTYPE_SENIOR && pNpc->getCFG()->stNpcFunc.hasFunction(ENPCFUNCTION_EQUIP_ENCHANT_SENIOR) == false))
  {
    XERR << "[包裹-装备附魔]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行" << eType << "附魔失败,未在正确npc旁" << XEND;
    return false;
  }

  if (specialEnchant(guid))
    return true;

  // get config
  const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(eType);
  if (pCFG == nullptr)
    return false;

  // get equip
  BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
    return false;
  ItemBase* pBase = pPack->getItem(guid);
  if (pBase == nullptr)
  {
    pPack = getPackage(EPACKTYPE_EQUIP);
    if (pPack == nullptr)
      return false;
    pBase = pPack->getItem(guid);
    if (pBase == nullptr)
      return false;
  }
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr || pEquip->getCFG() == nullptr || pEquip->getCFG()->isForbid(EFORBID_ENCHANT) == true)
    return false;

  EItemType eIType = pEquip->getCFG()->eItemType;
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PVE))
  {
    if (eIType == EITEMTYPE_HEAD || eIType == EITEMTYPE_FACE || eIType == EITEMTYPE_MOUTH
        || eIType == EITEMTYPE_TAIL || eIType == EITEMTYPE_BACK)
      return false;
  }
  // check need resource
  /*BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return false;
  if (pCFG->oNeedItem.id() != 0)
  {
    if (pMainPack->checkItemCount(pCFG->oNeedItem.id(), pCFG->oNeedItem.count()) == false)
      return false;
  }*/
  const SItemMiscCFG& rItemCFG = MiscConfig::getMe().getItemCFG();
  bool isSpecialEnchant = rItemCFG.isSpecEnchantItem(eType, pBase->getType());
  if (isSpecialEnchant)
  {
    const TVecItemInfo& vecItemCost = rItemCFG.getSpecEnchantItem(eType, pBase->getType());
    for (auto &v : vecItemCost)
    {
      if (checkItemCount(v, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_ENCHANT) == false)
        return false;
    }
  }
  else
  {
    if (pCFG->oNeedItem.id() != 0)
    {
      if (checkItemCount(pCFG->oNeedItem, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_ENCHANT) == false)
        return false;
    }
    if (m_pUser->checkMoney(EMONEYTYPE_SILVER, pCFG->dwRob) == false)
      return false;
  }

  // remove old extra attr
  EnchantData& rData = pEquip->getPreviewEnchantData();
  //for (int i = 0; i < rData.extras_size(); ++i)
    //m_pUser->m_oBuff.del(rData.extras(i).buffid());

  // enchant equip
  if (pCFG->random(pEquip->getType(), rData) == false)
    return false;

  // reduce money
  //pMainPack->reduceItem(pCFG->oNeedItem.id(), ESOURCE_ENCHANT, pCFG->oNeedItem.count());
  if (isSpecialEnchant)
  {
    const TVecItemInfo& vecItemCost = rItemCFG.getSpecEnchantItem(eType, pBase->getType());
    for (auto &v : vecItemCost)
      reduceItem(v, ESOURCE_ENCHANT, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_ENCHANT);
  }
  else
  {
    if (pCFG->oNeedItem.id() != 0)
      reduceItem(pCFG->oNeedItem, ESOURCE_ENCHANT, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_ENCHANT);
    m_pUser->subMoney(EMONEYTYPE_SILVER, pCFG->dwRob, ESOURCE_ENCHANT);
  }

  m_pUser->getShare().addNormalData(ESHAREDATATYPE_S_ENCHANTCOST, pCFG->dwRob);

  // save data
  pPack->setUpdateIDs(pEquip->getGUID());

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  if(eIType == EITEMTYPE_HEAD || eIType == EITEMTYPE_BACK || eIType == EITEMTYPE_FACE || eIType == EITEMTYPE_TAIL || eIType == EITEMTYPE_MOUTH)
  {
    m_pUser->getServant().onFinishEvent(ETRIGGER_ENCHANT_HEAD);
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_ENCHANT_HEAD);
  }
  // achieve
  m_pUser->getAchieve().onEquip(EACHIEVECOND_ENCHANT);
  if(eType == EENCHANTTYPE_PRIMARY)
  {
    m_pUser->getServant().onFinishEvent(ETRIGGER_ENCHANT_PRIMARY);
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_ENCHANT_PRIMARY);
  }
  else if(eType == EENCHANTTYPE_MEDIUM)
  {
    m_pUser->getServant().onFinishEvent(ETRIGGER_ENCHANT_MIDDLE);
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_ENCHANT_MIDDLE);
  }
  else if(eType == EENCHANTTYPE_SENIOR)
  {
    m_pUser->getServant().onFinishEvent(ETRIGGER_ENCHANT_ADVANCED);
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_ENCHANT_ADVANCED);
  }
  
  if (eType == EENCHANTTYPE_SENIOR)
    m_pUser->stopSendInactiveLog();

  // inform client
  EnchantEquip cmd;
  cmd.set_type(eType);
  cmd.set_guid(guid);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[包裹-装备附魔]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type:" << eType << "guid:" << guid << "成功附魔 获得如下属性" << XEND;
  for (int i = 0; i < rData.attrs_size(); ++i)
    XLOG << "[包裹-装备附魔]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type:" << rData.attrs(i).type() << "value:" << rData.attrs(i).value() << XEND;
  
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_ENCHANT_COUNT, 0, eType, 0,(DWORD)1);
  if (m_pUser->m_oUserStat.checkAndSet(ESTATTYPE_ENCHANT_USER_COUNT, eType))
    m_pUser->m_oUserStat.sendStatLog(ESTATTYPE_ENCHANT_USER_COUNT, 0, eType, 0, (DWORD)1);
  
  m_pUser->getShare().addNormalData(ESHAREDATATYPE_S_ENCHANTCOUNT, 1);
  
  // save data if data empty
  if (pEquip->getEnchantData().type() == EENCHANTTYPE_MIN)
    processEnchant(true, pEquip->getGUID());

  return true;
}

bool Package::specialEnchant(const string& guid)
{
  // get equip
  BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
    return false;
  ItemBase* pBase = pPack->getItem(guid);
  if (pBase == nullptr)
  {
    pPack = getPackage(EPACKTYPE_EQUIP);
    if (pPack == nullptr)
      return false;
    pBase = pPack->getItem(guid);
    if (pBase == nullptr)
      return false;
  }
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr || pEquip->getCFG() == nullptr || pEquip->getCFG()->isForbid(EFORBID_ENCHANT) == true)
    return false;
 
  EnchantData enchantData = pEquip->getEnchantData();
  if (enchantData.type() == EENCHANTTYPE_MIN)
    return false;
  // get config
  const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(enchantData.type());
  if (pCFG == nullptr)
    return false;

  // enchant equip
  if (pCFG->specialRandom(pEquip->getType(), enchantData) == false)
    return false;
  
  EnchantData& rPreData = pEquip->getPreviewEnchantData();
  rPreData.CopyFrom(enchantData);

  // save data
  pPack->setUpdateIDs(pEquip->getGUID());

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  // inform client
  EnchantEquip cmd;
  cmd.set_type(enchantData.type());
  cmd.set_guid(guid);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  DWORD buffid = 0;
  if (enchantData.extras_size() > 0)
    buffid = enchantData.extras(0).buffid();
    
  XLOG << "[附魔-特殊附魔] 遗留附魔附魔成功 charid" << m_pUser->id << "guid" << guid << "buffid" << buffid << XEND;
  processEnchant(true, guid);  
  return true;
}

bool Package::clearEnchantBuffid(char* guid)
{
  BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
  {
    XERR << "[附魔-补丁处理] 找不到主背包， 清除buffid charid" << m_pUser->id << "guid" << guid << XEND;
    return false;
  }
  ItemBase* pBase = pPack->getItem(guid);
  bool bOnEquip = false;
  if (pBase == nullptr)
  {
    pPack = getPackage(EPACKTYPE_EQUIP);
    if (pPack == nullptr)
    {
      XERR << "[附魔-补丁处理] 找不到装备背包， 清除buffid charid" << m_pUser->id << "guid" << guid << XEND;
      return false;
    }
    pBase = pPack->getItem(guid);
    if (pBase == nullptr)
    {
      XERR << "[附魔-补丁处理] 找不到对应guid装备， 清除buffid charid" << m_pUser->id << "guid" << guid << XEND;
      return false;
    }
    bOnEquip = true;
  }
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr || pEquip->getCFG() == nullptr)
  {
    XERR << "[附魔-补丁处理] 对应guid装备不是装备类型， 清除buffid charid" << m_pUser->id << "guid" << guid << XEND;
    return false;
  }

  EnchantData& rData = pEquip->getEnchantData();  

  if (rData.extras_size() == 0)
  {
    XERR << "[附魔-补丁处理] 对应装备没有buffid， 清除buffid charid" << m_pUser->id << "guid" << guid << XEND;
    return false;
  }

  if (bOnEquip)
  {
    for (int i = 0; i < rData.extras_size(); ++i)
      m_pUser->m_oBuff.del(rData.extras(i).buffid());
  }
  DWORD buffid = rData.extras(0).buffid();
  
  rData.clear_extras();  
  pEquip->getPreviewEnchantData().clear_extras();

  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  
  XLOG << "[附魔-补丁处理] 清除buffid charid" << m_pUser->id << "guid" << guid <<"buffid" << buffid << XEND;
  return true;
}

bool Package::processEnchant(bool bSave, const string& guid)
{
  // get equip
  bool bOnEquip = false;
  BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
    return false;
  ItemBase* pBase = pPack->getItem(guid);
  if (pBase == nullptr)
  {
    pPack = getPackage(EPACKTYPE_EQUIP);
    if (pPack == nullptr)
      return false;
    pBase = pPack->getItem(guid);
    if (pBase == nullptr)
      return false;
    bOnEquip = true;
  }
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr || pEquip->getCFG() == nullptr)
    return false;

  const SItemCFG* pItemCFG = pEquip->getCFG();
  if (pItemCFG == nullptr || pItemCFG->isForbid(EFORBID_ENCHANT) == true)
    return false;

  EnchantData& rPreviewData = pEquip->getPreviewEnchantData();
  EnchantData oldData = pEquip->getEnchantData();
  if (rPreviewData.type() == EENCHANTTYPE_MIN)
    return false;

  // get config
  const SEnchantCFG* pCFG = ItemConfig::getMe().getEnchantCFG(rPreviewData.type());
  if (pCFG == nullptr)
    return false;

  // update enchant data
  EnchantData& rData = pEquip->getEnchantData();
  if (bSave)
  {
    if (bOnEquip)
    {
      for (int i = 0; i < rData.extras_size(); ++i)
        m_pUser->m_oBuff.del(rData.extras(i).buffid());
    }

    rData.Clear();
    rData.CopyFrom(rPreviewData);
    rPreviewData.Clear();

    if (bOnEquip)
    {
      for (int i = 0; i < rData.extras_size(); ++i)
      {
        const SEnchantAttr* pAttr = pCFG->getEnchantAttr(rData.extras(i).configid());
        if (pAttr == nullptr || pAttr->vecExtraCondition.size() < 2)
          continue;

        if (pAttr->vecExtraCondition[0] == EENCHANTEXTRACON_REFINELV && pEquip->getRefineLv() >= pAttr->vecExtraCondition[1])
          m_pUser->m_oBuff.add(rData.extras(i).buffid());
      }
    }
    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    m_pUser->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  }
  else
  {
    EnchantData& rPreData = pEquip->getPreviewEnchantData();
    XLOG << "[包裹-装备附魔保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取消了上次附魔结果" << XEND;
    for (int i = 0; i < rPreData.attrs_size(); ++i)
      XLOG << "[包裹-装备附魔保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type:" << rPreData.attrs(i).type() << "value:" << rPreData.attrs(i).value() << XEND;
    for (int i = 0; i < rPreData.extras_size(); ++i)
      XLOG << "[包裹-装备附魔保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type:" << rPreData.type() << "buffid:" << rPreData.extras(i).buffid() << XEND;
    rPreData.Clear();
  }

  // save data
  pPack->setUpdateIDs(pEquip->getGUID());

  if (!bSave)
    return true;

  // log
  std::stringstream oldAttrss;
  std::stringstream oldBufss;
  std::stringstream newAttrss;
  std::stringstream newBufss;

  for (int i = 0; i < oldData.attrs_size(); ++i)
    oldAttrss << oldData.attrs(i).type() << "," << oldData.attrs(i).value() << ";";
  for (int i = 0; i < oldData.extras_size(); ++i)
    oldBufss << oldData.extras(i).buffid() << ",";

  XLOG << "[包裹-装备附魔保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type:" << rData.type() << "guid:" << guid << "成功附魔 获得如下属性" << XEND;
  for (int i = 0; i < rData.attrs_size(); ++i)
  {
    XLOG << "[包裹-装备附魔保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type:" << rData.attrs(i).type() << "value:" << rData.attrs(i).value() << XEND;
    newAttrss << rData.attrs(i).type() << "," << rData.attrs(i).value() << ";";
  }
  for (int i = 0; i < rData.extras_size(); ++i)
  {
    XLOG << "[包裹-装备附魔保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type:" << rData.type() << "buffid:" << rData.extras(i).buffid() << XEND;
    newBufss << rData.extras(i).buffid() << ",";
  }

  std::string oldAttr = oldAttrss.str();
  std::string newAttr = newAttrss.str();
  std::string oldBuffid = oldBufss.str();
  std::string newBuffid = newBufss.str();

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE evType = EventType_Enchant;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), evType, 0, 1);

  PlatLogManager::getMe().EnchantLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    evType,
    eid,
    guid,
    pItemCFG->dwTypeID,
    rData.type(),
    oldAttr,
    newAttr,
    oldBuffid,
    newBuffid,
    pCFG->oNeedItem.id(),
    pCFG->oNeedItem.count(),
    pCFG->dwRob
  );
  return true;
}

bool Package::exchange(const string& guid)
{
  if (m_pUser->checkPwd(EUNLOCKTYPE_EQUIP_HOLE) == false)
  {
    XERR << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "对" << guid << "进行置换失败,密码验证失败" << XEND;
    return false;
  }

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "对" << guid << "进行置换失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }

  BasePackage* pPack = nullptr;
  ItemEquip* pEquip = nullptr;
  for (DWORD d = EPACKTYPE_MIN + 1; d < EPACKTYPE_MAX; ++d)
  {
    if (d == EPACKTYPE_STORE)
      continue;

    pPack = m_pUser->getPackage().getPackage(static_cast<EPackType>(d));
    if (pPack == nullptr)
      continue;

    pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(guid));
    if (pEquip != nullptr)
      break;
  }
  if (pPack == nullptr || pEquip == nullptr)
  {
    XERR << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "对" << guid << "进行置换失败,未找到装备" << XEND;
    return false;
  }

  EPackType eType = pPack->getPackageType();
  const SItemCFG* pCFG = pEquip->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "对" << guid << "进行置换失败,装备配置为空" << XEND;
    return false;
  }
  DWORD dwComposeID = pCFG->dwSubID;
  EPackFunc eFunc = EPACKFUNC_EQUIPEXCHANGE;
  ECheckMethod eMethod = ECHECKMETHOD_NONORMALEQUIP;
  if (m_pUser->checkCompose(dwComposeID, EPACKMETHOD_NOCHECK, GACTIVITY_MIN, eMethod, eFunc) == false)
  {
    XERR << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "对" << guid << "进行置换失败,composeid :" << dwComposeID << "不合法" << XEND;
    return false;
  }

  // save equip info
  DWORD dwStrengthLv = pEquip->getStrengthLv();
  DWORD dwRefineLv = pEquip->getRefineLv();
  DWORD dwStrengthCost = pEquip->getStrengthCost();
  DWORD dwLv = pEquip->getLv();
  DWORD dwStrengthLv2 = pEquip->getStrengthLv2();

  bool bDamage = pEquip->isDamaged();
  const TMapEquipCard mapCard = pEquip->getCardList();
  const TVecRefineCompose vecCompose = pEquip->getRefineCompose();
  const EnchantData enchant = pEquip->getEnchantData();
  const EnchantData preenchant = pEquip->getPreviewEnchantData();

  // remove old equip
  EEquipPos ePos = static_cast<EEquipPos>(pEquip->getIndex());
  if ((eType == EPACKTYPE_EQUIP && equipOff(guid) == false) || (eType == EPACKTYPE_FASHIONEQUIP && equipOffFashion(guid) == false))
  {
    XERR << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "对" << guid << "进行置换失败,composeid :" << dwComposeID << "不合法" << XEND;
    return false;
  }
  pMainPack->reduceItem(pEquip->getGUID(), ESOURCE_EXCHANGE);

  // compose new equip
  const string& itemid = m_pUser->doCompose(dwComposeID, EPACKMETHOD_NOCHECK, GACTIVITY_MIN, true, true, true, eMethod, eFunc);

  pEquip = dynamic_cast<ItemEquip*>(getItem(itemid));
  if (pEquip == nullptr)
  {
    XERR << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "对" << guid << "进行置换后装备为空,composeid:" << dwComposeID << "itemid:" << itemid << XEND;
    return false;
  }

  while (true)
  {
    if (pEquip->getStrengthLv() >= dwStrengthLv)
      break;

    DWORD price = LuaManager::getMe().call<DWORD>("calcEquipStrengthCost", pEquip->getStrengthLv(), pEquip->getQuality(), pEquip->getType());
    if (price > dwStrengthCost)
      break;
    dwStrengthCost -= price;

    pEquip->setStrengthLv(pEquip->getStrengthLv() + 1);
    pEquip->addStrengthCost(price);
  }

  m_pUser->addMoney(EMONEYTYPE_SILVER, dwStrengthCost, ESOURCE_TRANSFER);

  // return equip
  //pEquip->setStrengthLv(dwStrengthLv);
  //pEquip->addStrengthCost(dwStrengthCost);
  pEquip->setRefineLv(dwRefineLv);
  pEquip->setLv(dwLv);
  pEquip->setDamageStatus(bDamage);
  pEquip->getEnchantData().CopyFrom(enchant);
  pEquip->getPreviewEnchantData().CopyFrom(preenchant);
  pEquip->setStrengthLv2(dwStrengthLv2);
  for (auto &v : vecCompose)
    pEquip->addRefineCompose(v.first, v.second);
  for (auto &v : mapCard)
  {
    const SItemCFG* pCardCFG = ItemManager::getMe().getItemCFG(v.second.second);
    if (pCardCFG == nullptr)
      continue;

    ItemCard oCard(pCardCFG);
    oCard.setGUID(v.second.first);
    oCard.init();
    DWORD pos = v.first;
    if (pEquip->getCardByPos(pos) != nullptr)
      pos = pEquip->getEmptyCardPos();
    if (!pEquip->addCard(&oCard, pos))
    { //return cards
      ItemInfo oItem;
      oItem.set_id(pCardCFG->dwTypeID);
      oItem.set_count(1);
      oItem.set_source(ESOURCE_EXCHANGE);
      /*if (pMainPack->checkAddItem(oItem, EPACKMETHOD_CHECK_WITHPILE) == false)
        continue;*/
      ItemInfo oCopy;
      oCopy.CopyFrom(oItem);
      pMainPack->addItem(oCopy, EPACKMETHOD_NOCHECK, false, false);

      MsgParams params;
      params.addNumber(oItem.id());
      params.addNumber(oItem.id());
      params.addNumber(oItem.count());
      MsgManager::sendMsg(m_pUser->id, 6, params);
    }
  }

  // equip on new equip
  if ((eType == EPACKTYPE_EQUIP && equipOn(ePos, itemid) == true) || (eType == EPACKTYPE_FASHIONEQUIP && equipPutFashion(ePos, guid) == true))
    pMainPack->removeUpdateIDs(itemid);

  // inform client
  EquipExchangeItemCmd cmd;
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getEvent().onEquipExchange(guid, itemid);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EQUIP_EXCHANGE);

  XLOG << "[包裹-装备置换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "对" << guid << "进行置换成功" << XEND;
  return true;
}

bool Package::upgrade(const string& guid)
{
  if (m_pUser->checkPwd(EUNLOCKTYPE_EQUIP_UPGRADE) == false)
  {
    XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "升级装备 :" << guid << "失败,密码验证失败" << XEND;
    return false;
  }

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "升级装备 :" << guid << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }
  if (pMainPack->isFull() == true)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
    return false;
  }

  BasePackage* pPack = nullptr;
  ItemEquip* pEquip = nullptr;
  for (DWORD d = EPACKTYPE_MIN + 1; d < EPACKTYPE_MAX; ++d)
  {
    if (d == EPACKTYPE_STORE)
      continue;

    pPack = m_pUser->getPackage().getPackage(static_cast<EPackType>(d));
    if (pPack == nullptr)
      continue;

    pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(guid));
    if (pEquip != nullptr)
      break;
  }
  if (pPack == nullptr || pEquip == nullptr)
  {
    XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "升级装备 :" << guid << "失败,未找到装备" << XEND;
    return false;
  }
  const SItemCFG* pCFG = pEquip->getCFG();
  EPackType eType = pPack->getPackageType();
  if (pCFG == nullptr)
  {
    XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "升级装备 :" << guid << "失败,装备未包含正确的配置" << XEND;
    return false;
  }

  const SEquipUpgradeCFG* pUpgradeCFG = pCFG->getUpgradeCFG(pEquip->getLv() + 1);
  const SEquipUpgradeCFG* pNextUpgradeCFG = pCFG->getUpgradeCFG(pEquip->getLv() + 2);
  if (pUpgradeCFG == nullptr)
  {
    XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "升级装备 :" << guid << "失败,未找到" << pEquip->getLv() << "升级配置" << XEND;
    return false;
  }
  ECheckMethod eMethod = ECHECKMETHOD_UPGRADE;
  if (checkItemCount(pUpgradeCFG->vecMaterial, eMethod, EPACKFUNC_UPGRADE) == false)
  {
    XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "升级装备 :" << guid << "失败,材料不足" << XEND;
    return false;
  }
  DWORD pro = m_pUser->getProfession();
  DWORD evo = pro == EPROFESSION_NOVICE ? 0 : pro % 10;
  if (evo < pUpgradeCFG->dwEvoReq)
  {
    XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "升级装备 :" << guid << "失败,等级" << pEquip->getLv() << "未达到进阶条件" << XEND;
    return false;
  }

  // upgrade
  const SItemCFG* pProduceItemCFG = ItemConfig::getMe().getItemCFG(pUpgradeCFG->oProduct.id());
  if (pNextUpgradeCFG == nullptr && pProduceItemCFG != nullptr)
  {
    if (ItemManager::getMe().isEquip(pProduceItemCFG->eItemType) == false)
    {
      XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "升级装备 :" << guid << "失败,产物 :" << pUpgradeCFG->oProduct.ShortDebugString() << "不是装备" << XEND;
      return false;
    }

    TVecItemInfo vecItem = TVecItemInfo{pUpgradeCFG->oProduct};
    const TMapEquipCard mapCard = pEquip->getCardList();
    for (auto &v : mapCard)
    {
      ItemInfo oItem;
      oItem.set_id(v.second.second);
      oItem.set_count(1);
      oItem.set_source(ESOURCE_EXCHANGE);
      combinItemInfo(vecItem, TVecItemInfo{oItem});
    }
    if (pMainPack->checkAddItem(vecItem, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    EEquipPos ePos = static_cast<EEquipPos>(pEquip->getIndex());
    if ((eType == EPACKTYPE_EQUIP && equipOff(guid) == false) || (eType == EPACKTYPE_FASHIONEQUIP && equipOffFashion(guid) == false))
    {
      XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "升级装备 :" << guid << "失败,卸下失败" << XEND;
      return false;
    }

    ItemData oData;
    pEquip->toItemData(&oData);

    pMainPack->reduceItem(guid, ESOURCE_EXCHANGE);
    reduceItem(pUpgradeCFG->vecMaterial, ESOURCE_EXCHANGE, eMethod, EPACKFUNC_UPGRADE);

    oData.mutable_base()->set_id(pProduceItemCFG->dwTypeID);
    oData.mutable_equip()->set_lv(0);
    oData.mutable_equip()->set_cardslot(pProduceItemCFG->dwCardSlot);
    if (pMainPack->addItem(oData, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "升级装备 :" << guid << "添加产物" << oData.ShortDebugString() << "失败" << XEND;
      return false;
    }

    string newguid = pMainPack->getGUIDByType(pUpgradeCFG->oProduct.id());
    pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(newguid));
    if (pEquip == nullptr)
    {
      XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在" << eType << "升级装备 :" << guid << "成功,达到了满级,但是获得不是装备" << newguid << pUpgradeCFG->oProduct.ShortDebugString() << "直接退出穿戴 原装备属性" << oData.ShortDebugString() << XEND;
      return true;
    }
    DWORD dwStrengthLv = pEquip->getStrengthLv();
    pEquip->setStrengthLv(0);
    DWORD dwRefineDec = MiscConfig::getMe().getItemCFG().dwUpgradeRefineLvDec;
    if (pEquip->getRefineLv() > dwRefineDec)
      pEquip->setRefineLv(pEquip->getRefineLv() - dwRefineDec);
    else
      pEquip->setRefineLv(0);

    // transfer strengthlv
    DWORD dwCost = pEquip->getStrengthCost();
    pEquip->resetStrengthCost();
    while (true)
    {
      if (pEquip->getStrengthLv() >= dwStrengthLv)
        break;

      DWORD price = LuaManager::getMe().call<DWORD>("calcEquipStrengthCost", pEquip->getStrengthLv(), pEquip->getQuality(), pEquip->getType());
      if (price > dwCost)
        break;
      dwCost -= price;

      pEquip->setStrengthLv(pEquip->getStrengthLv() + 1);
      pEquip->addStrengthCost(price);
    }
    if (dwCost > 0)
      m_pUser->addMoney(EMONEYTYPE_SILVER, dwCost, ESOURCE_EXCHANGE);

    // remove card
    for (auto &v : mapCard)
    {
      const SItemCFG* pCardCFG = ItemManager::getMe().getItemCFG(v.second.second);
      if (pCardCFG == nullptr)
      {
        XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "升级装备 :" << guid << "成功,达到了满级,获得新装备" << newguid << pUpgradeCFG->oProduct.ShortDebugString()
          << "拆卡 :" << v.second.first << v.second.second << "失败,未在Table_Item.txt表中找到" << XEND;
        continue;
      }

      if (cardOff(v.second.first, newguid, v.first) == false)
      {
        XERR << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "升级装备 :" << guid << "成功,达到了满级,获得新装备" << newguid << pUpgradeCFG->oProduct.ShortDebugString()
          << "拆卡 :" << v.second.first << v.second.second << "失败" << XEND;
        continue;
      }

      MsgParams params;
      params.addNumber(v.second.second);
      params.addNumber(v.second.second);
      params.addNumber(1);
      MsgManager::sendMsg(m_pUser->id, 6, params);
    }

    bool bOn = false;
    if (eType == EPACKTYPE_EQUIP)
      bOn = equipOn(ePos, newguid);
    else if (eType == EPACKTYPE_FASHIONEQUIP)
      bOn = equipPutFashion(ePos, newguid);

    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    m_pUser->refreshDataAtonce();
    XLOG << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "在" << eType << "升级装备 :" << guid << "成功,达到了满级,获得新装备" << newguid << pUpgradeCFG->oProduct.ShortDebugString();
    if (eType == EPACKTYPE_EQUIP || eType == EPACKTYPE_FASHIONEQUIP)
      XLOG << "穿戴" << (bOn ? "成功" : "失败") << XEND;
    else
      XLOG << XEND;
    return true;
  }

  reduceItem(pUpgradeCFG->vecMaterial, ESOURCE_EXCHANGE, eMethod, EPACKFUNC_UPGRADE);
  DWORD dwOldLv = pEquip->getLv();
  pEquip->setLv(pEquip->getLv() + 1);
  pPack->setUpdateIDs(pEquip->getGUID());

  //add upgrade buff
  if (eType == EPACKTYPE_EQUIP)
    m_pUser->m_oBuff.add(pUpgradeCFG->dwBuffID);
  m_pUser->getAchieve().onEquip(EACHIEVECOND_EQUIP_UPGRADE);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EQUIP_UPGRADE);

  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  // inform client for effect play
  EquipExchangeItemCmd cmd;
  cmd.set_guid(guid);
  cmd.set_type(EEXCHANGETYPE_LEVELUP);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[包裹-装备升级]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << eType << "升级装备 :" << guid << "成功,等级从" << dwOldLv << "->" << pEquip->getLv() << "级" << XEND;
  return true;
}

bool Package::Decompose(const string& guid)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,未通过npc溶解" << XEND;
    return false;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,和npc不在同一场景" << XEND;
    return false;
  }
  float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,距离npc过远" << XEND;
    return false;
  }
  const SNpcCFG* pNpcCFG = pNpc->getCFG();
  if (pNpcCFG == nullptr || pNpcCFG->stNpcFunc.hasFunction(ENPCFUNCTION_EQUIP_DECOMPOSE) == false)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,未在正确的npc旁" << XEND;
    return false;
  }

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(guid));
  if (pEquip == nullptr)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,未找到装备" << guid << XEND;
    return false;
  }
  const SItemCFG* pCFG = pEquip->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,装备" << guid << "未包含正确的配置" << XEND;
    return false;
  }
  const SEquipDecomposeCFG* pDecomposeCFG = ItemConfig::getMe().getEquipDecomposeCFG(pCFG->dwDecomposeID);
  if (pDecomposeCFG == nullptr)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "溶解装备 :" << guid << "失败,装备" << guid << "decomposeid :" << pCFG->dwDecomposeID << "未在Table_EquipDecompose.txt表中找到" << XEND;
    return false;
  }
  const TListDecomposeResult& listResult = pEquip->getDecomposeItem();
  if (listResult.empty() == true)
  {
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "溶解装备 :" << guid << pEquip->GetItemInfo().ShortDebugString() << "失败,未进行过结果查询" << XEND;
    return false;
  }

  if (m_pUser->checkMoney(EMONEYTYPE_SILVER, pDecomposeCFG->dwCost) == false)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_ZENY_NO_ENOUGH);
    XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "溶解装备 :" << guid << "失败,装备" << guid << "zeny不足,需要 :" << pDecomposeCFG->dwCost << XEND;
    return false;
  }
  m_pUser->subMoney(EMONEYTYPE_SILVER, pDecomposeCFG->dwCost, ESOURCE_UPGRADE);

  TVecItemInfo vecItem;
  for (auto &s : listResult)
  {
    DWORD dwCount = static_cast<DWORD>(s.rate() / ONE_THOUSAND);
    DWORD dwRate = static_cast<DWORD>(s.rate() % ONE_THOUSAND);
    dwCount += dwRate < static_cast<DWORD>(randBetween(0, ONE_THOUSAND)) ? 1 : 0;
    if (dwCount == 0)
      continue;

    ItemInfo oItem;
    oItem.CopyFrom(s.item());
    oItem.set_count(dwCount);
    oItem.set_source(ESOURCE_DECOMPOSE);
    combinItemInfo(vecItem, TVecItemInfo{oItem});
  }

  EquipDecompose cmd;
  cmd.set_guid(guid);
  cmd.set_result(vecItem.empty() == false ? pEquip->getDecomposeResult() : EDECOMPOSERESULT_FAIL);

  if (pCFG->dwSellPrice != 0)
  {
    ItemInfo oItem;
    oItem.set_id(ITEM_ZENY);
    oItem.set_count(pCFG->dwSellPrice);
    combinItemInfo(vecItem, TVecItemInfo{oItem});
  }

  pMainPack->reduceItem(guid, ESOURCE_DECOMPOSE);
  if (vecItem.empty() == false)
  {
    if (pMainPack->checkAddItem(vecItem, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      XERR << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "失败,装备" << guid << "背包空间不足" << XEND;
      return false;
    }
    for (auto &v : vecItem)
      cmd.add_items()->CopyFrom(v);
    //pMainPack->addItemFull(vecItem, false, false, false);
    pMainPack->addItem(vecItem, EPACKMETHOD_CHECK_WITHPILE, false, false, false);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EQUIP_DECOMPOSE);

  XLOG << "[包裹-装备溶解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "溶解装备 :" << guid << "成功,结果为" << cmd.ShortDebugString() << "获得";
  for (auto &v : vecItem)
    XLOG << v.ShortDebugString();
  XLOG << XEND;
  return true;
}

bool Package::queryDecomposeResult(const string& guid)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,未通过npc查询" << XEND;
    return false;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,和npc不在同一场景" << XEND;
    return false;
  }
  float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,距离npc过远" << XEND;
    return false;
  }
  const SNpcCFG* pNpcCFG = pNpc->getCFG();
  if (pNpcCFG == nullptr || pNpcCFG->stNpcFunc.hasFunction(ENPCFUNCTION_EQUIP_DECOMPOSE) == false)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,未在正确的npc旁" << XEND;
    return false;
  }

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(guid));
  if (pEquip == nullptr)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,未找到装备" << XEND;
    return false;
  }
  pEquip->setDecomposeResult(EDECOMPOSERESULT_MIN);
  pEquip->setDecomposeItem(TListDecomposeResult{});
  if (pEquip->canDecompose() == false)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,该装备无法被溶解" << XEND;
    return false;
  }
  const SItemCFG* pCFG = pEquip->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备:" << guid << "失败,装备" << guid << "未包含正确的配置" << XEND;
    return false;
  }
  const SEquipDecomposeCFG* pDecomposeCFG = ItemConfig::getMe().getEquipDecomposeCFG(pCFG->dwDecomposeID);
  if (pDecomposeCFG == nullptr)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "查询装备 :" << guid << "失败,装备" << guid << "decomposeid :" << pCFG->dwDecomposeID << "未在Table_EquipDecompose.txt表中找到" << XEND;
    return false;
  }
  DWORD dwNum = pCFG->dwDecomposeNum;
  DWORD dwOriNum = dwNum;
  if (dwNum == 0)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "失败,装备金属价值未在 Table_Equip.txt 表中配置" << XEND;
    return false;
  }
  DWORD dwMTotal = 0;
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  for (DWORD i = 1; i <= pEquip->getLv(); ++i)
  {
    const SEquipUpgradeCFG* p = pCFG->getUpgradeCFG(i);
    if (p == nullptr)
    {
      XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "溶解装备 :" << guid << pEquip->getTypeID() << "失败,等级" << i << "未找到升级配置" << XEND;
      return false;
    }
    for (auto &v : p->vecMaterial)
    {
      DWORD dwPrice = rCFG.getDecomposePrice(v.id());
      if (dwPrice == 0)
      {
        XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "溶解装备 :" << guid << pEquip->getTypeID() << "失败,id :" << v.id() << "未在ServerGame_Item中查询到价格" << XEND;
        return false;
      }
      dwMTotal += dwPrice;
      XLOG << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "溶解装备 :" << guid << pEquip->getTypeID() << "增加材料价格,交易所" << dwPrice << XEND;
    }
  }

  DWORD dwOriEquipID = ComposeConfig::getMe().getOriMaterialEquip(pEquip->getTypeID());
  const SItemCFG* pOriCFG = ItemConfig::getMe().getItemCFG(dwOriEquipID);
  if (pOriCFG != nullptr)
    dwOriNum = pOriCFG->dwDecomposeNum;

  float fParam = LuaManager::getMe().call<float>("calcDecomposeFloatParam", 0);
  float fMinParam = LuaManager::getMe().call<float>("calcDecomposeFloatParam", 1);
  float fMaxParam = LuaManager::getMe().call<float>("calcDecomposeFloatParam", 2);
  DWORD dwResult = LuaManager::getMe().call<DWORD>("calcDecomposeResult", static_cast<DWORD>(fParam * ONE_THOUSAND));
  if (dwResult <= EDECOMPOSERESULT_MIN || dwResult >= EDECOMPOSERESULT_MAX || EDecomposeResult_IsValid(dwResult) == false)
  {
    XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "查询装备 :" << guid << pEquip->GetItemInfo().ShortDebugString() << "失败,结果" << dwResult << "不合法" << XEND;
    return false;
  }

  TListDecomposeResult listResult;
  QueryDecomposeResultItemCmd cmd;
  cmd.set_sell_price(pCFG->dwSellPrice);
  for (auto &v : pDecomposeCFG->vecMaterial)
  {
    DWORD dwPrice = rCFG.getDecomposePrice(v.id());
    if (dwPrice == 0)
    {
      XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "溶解装备 :" << guid << pEquip->getTypeID() << "失败,id :" << v.id() << "未在ServerGame_Item中查询到价格" << XEND;
      return false;
    }

    DecomposeResult oResult;
    oResult.mutable_item()->CopyFrom(v);

    float f = 1.0f * v.count() / TEN_THOUSAND;
    xTime frameTick;
    xSceneEntryDynamic* pEntry = (xSceneEntryDynamic*)m_pUser;
    float fRate = LuaManager::getMe().call<float>("calcDecomposeCount", pEntry, dwNum, dwOriNum, dwMTotal, f, dwPrice, pEquip->getRefineLv(), fParam);
    XDBG << "[包裹-溶解查询] 计算结果 :" << fRate << "耗时" << frameTick.uElapse() << "微秒" << XEND;

    float fMin = LuaManager::getMe().call<float>("calcDecomposeCount", pEntry, dwNum, dwOriNum, dwMTotal, f, dwPrice, pEquip->getRefineLv(), fMinParam);
    float fMax = LuaManager::getMe().call<float>("calcDecomposeCount", pEntry, dwNum, dwOriNum, dwMTotal, f, dwPrice, pEquip->getRefineLv(), fMaxParam);
    XDBG << "[包裹-溶解查询] 计算最小最大 :" << fMin << fMax << "耗时" << frameTick.uElapse() << "微秒" << XEND;

    static const float p_min = -0.00001f;
    static const float p_max = 0.00001f;
    if ((fRate > p_min && fRate < p_max) || (fMin > p_min && fMin < p_max) || (fMax > p_min && fMax < p_max))
    {
      XERR << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "查询装备 :" << guid << pEquip->GetItemInfo().ShortDebugString() << "失败,结果 rate :" << fRate << "min :" << fMin << "max :" << fMax << "不合法" << XEND;
      return false;
    }

    oResult.set_rate(fRate * ONE_THOUSAND);
    oResult.set_min_count(fMin * ONE_THOUSAND);
    oResult.set_max_count(fMax * ONE_THOUSAND);
    listResult.push_back(oResult);
    cmd.add_results()->CopyFrom(oResult);
  }

  pEquip->setDecomposeResult(static_cast<EDecomposeResult>(dwResult));
  pEquip->setDecomposeItem(listResult);

  // inform client
  cmd.set_guid(guid);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[包裹-溶解查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询装备 :" << guid << "成功,结果为";
  for (auto &v : listResult)
    XLOG << v.ShortDebugString();
  XLOG << XEND;
  return true;
}

void Package::refreshEnableBuffs(ItemBase* pItem)
{
  BasePackage* pEquipPack = getPackage(EPACKTYPE_EQUIP);

  if (pItem != nullptr)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*> (pItem);
    if (pEquip == nullptr)
      return;

    if (pEquip->checkBuffChange(m_pUser) == true && pEquipPack != nullptr)
      pEquipPack->m_setUpdateIDs.insert(pEquip->getGUID());

    return;
  }

  TVecSortItem pVecItems;
  // wear on or off equip
  if (pEquipPack != nullptr)
  {
    pEquipPack->getEquipItems(pVecItems);
    for (auto v = pVecItems.begin(); v != pVecItems.end(); ++v)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*> (*v);
      if (pEquip == nullptr)
        continue;
      if (pEquip->checkBuffChange(m_pUser))
      {
        pEquipPack->m_setUpdateIDs.insert(pEquip->getGUID());
      }
    }
  }

  pVecItems.clear();
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack != nullptr)
  {
    pMainPack->getEquipItems(pVecItems);
    for (auto v = pVecItems.begin(); v != pVecItems.end(); ++v)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*> (*v);
      if (pEquip == nullptr)
        continue;
      if (pEquip->checkBuffChange(m_pUser))
      {
        pMainPack->m_setUpdateIDs.insert(pEquip->getGUID());
      }
    }
  }

}

bool Package::decompose(const string& guid, bool bKeepOriEquip,DWORD& returnMoney)
{
  if (m_pUser == nullptr)
    return false;

  returnMoney = 0;

  // get equip
  /*BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  if (pPack == nullptr)
  {
    XERR << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "分解了 guid:" << guid << "失败,未发现" << EPACKTYPE_MAIN << XEND;
    return false;
  }
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(guid));
  if (pEquip == nullptr)
  {
    pPack = getPackage(EPACKTYPE_TEMP_MAIN);
    if (pPack == nullptr)
    {
      XERR << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "分解了 guid:" << guid << "失败,未发现" << EPACKTYPE_MAIN << XEND;
      return false;
    }
    pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(guid));
  }*/
  BasePackage* pPack = nullptr;
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(getItem(guid, &pPack));
  if (pEquip == nullptr || pPack == nullptr)
  {
    XERR << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "分解了 guid:" << guid << "失败,在" << pPack->getPackageType() << "未找到该装备" << XEND;
    return false;
  }
  if(m_pUser->checkPwd(EUNLOCKTYPE_DECOMPOSE) == false)
  {
    XERR << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "失败,密码验证失败" << XEND;
    return false;
  }
  if (pEquip->getStrengthLv() == 0 && pEquip->getRefineLv() == 0)
  {
    XERR << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "分解了 guid:" << guid << "失败,在" << pPack->getPackageType() << "中该装备无需分解" << XEND;
    return false;
  }

  const SItemCFG* pCFG = pEquip->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "分解了 guid:" << guid << "失败,在" << pPack->getPackageType() << "中该装备未包含正确的配置" << XEND;
    return false;
  }

  TVecItemInfo vecItems;
  if (bKeepOriEquip && pEquip->getRefineLv() != 0)
  {
    const SRefineCFG* pCFG = ItemConfig::getMe().getRefineCFG(pEquip->getType(), pEquip->getQuality(), pEquip->getRefineLv());
    if (pCFG != nullptr)
    {
      m_pUser->addMoney(EMONEYTYPE_SILVER, pCFG->sComposeRate.dwReturnZeny * pEquip->getRefineLv(), ESOURCE_DECOMPOSE);
    }
    else
    {
      XERR << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "分解了 guid:" << guid << pEquip->getTypeID() << "未发现refinelv :" << pEquip->getRefineLv() << "quality :" << pEquip->getQuality() << "精炼配置" << XEND;
    }
    /*const TVecRefineCompose& vecCompose = pEquip->getRefineCompose();
    for (auto v = vecCompose.begin(); v != vecCompose.end(); ++v)
    {
      const SComposeCFG* pCompose = ComposeConfig::getMe().getComposeCFG(v->first);
      if (pCompose == nullptr)
        continue;

      TVecItemInfo vecCompose = pCompose->vecMaterial;
      for (auto o = vecCompose.begin(); o != vecCompose.end(); ++o)
        o->set_count(o->count() * v->second);
      combinItemInfo(vecItems, vecCompose);

      ItemInfo oMoney;
      if (pCompose->dwROB != 0)
      {
        oMoney.set_id(100);
        oMoney.set_count(pCompose->dwROB);
        combinItemInfo(vecItems, TVecItemInfo{oMoney});
      }
      if (pCompose->dwGold != 0)
      {
        oMoney.set_id(105);
        oMoney.set_count(pCompose->dwGold);
        combinItemInfo(vecItems, TVecItemInfo{oMoney});
      }
    }*/
    pEquip->setRefineLv(0);
    pEquip->resetRefineCompose();
  }

  // return items
  //backStrengthCost(pEquip);
  if (pEquip->getStrengthCost() != 0)
  {
    ItemInfo oItem;
    oItem.set_id(100);
    oItem.set_count(pEquip->getStrengthCost());
    oItem.set_source(ESOURCE_DECOMPOSE);
    returnMoney += pEquip->getStrengthCost();

    combinItemInfo(vecItems, TVecItemInfo{oItem});
  }
  if (vecItems.empty() == false)
  {
    addItem(vecItems, EPACKMETHOD_AVAILABLE, false, false, false);
    for (auto &v : vecItems)
      XLOG << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "进行分解" << bKeepOriEquip << "收到返还材料 itemid:" << v.id() << "count:" << v.count() << XEND;
  }

  // reset equip
  pEquip->setStrengthLv(0);
  pEquip->resetStrengthCost();

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();
  pPack->setUpdateIDs(pEquip->getGUID());
  pPack->update(0);

  // play client effect
  /*EquipDecompose cmd;
  cmd.set_bkeeporiequip(bKeepOriEquip);
  cmd.set_guid(guid);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);*/

  // message hint
  //MsgManager::sendMsg(m_pUser->id, 408);

  XLOG << "[包裹-装备分解]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "在" << pPack->getPackageType() << "成功分解了 guid:" << guid << "typeid:" << pEquip->getTypeID() << XEND;
  return true;
}

bool Package::queryEquipData(const string& guid)
{
  // get equip
  ItemEquip* pEquip = nullptr;
  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pMainPack == nullptr || pEquipPack == nullptr)
    return false;
  pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(guid));
  if (pEquip == nullptr)
    pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(guid));
  if (pEquip == nullptr)
    return false;

  QueryEquipData cmd;
  cmd.set_guid(guid);
  pEquip->toEquipData(cmd.mutable_data());
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  return true;
}

bool Package::browsePackage(EPackType eType, bool bNotify /*= true*/)
{
  //屏蔽时装和卡片背包浏览的效果，交给红点提醒来更新,--现在屏蔽了时装和卡片的红点
  /*
  if (EPACKTYPE_FASHION == eType)// || EPACKTYPE_CARD == eType)
    return true;
  */
  // refresh new mark
  auto browse = [this](ItemBase* pBase)
  {
    if (pBase == nullptr)
      return;

    pBase->setNew(false);
    pBase->setHint(!isHint(pBase->getTypeID()));
  };
  BasePackage* pPackage = getPackage(eType);
  if (pPackage == nullptr)
  {
    XERR << "[包裹-刷新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未找到" << eType << "包裹" << XEND;
    return false;
  }
  pPackage->foreach(browse);

  // inform client
  if (bNotify)
  {
    BrowsePackage cmd;
    cmd.set_type(eType);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[包裹-刷新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "刷新" << eType << "包裹成功" << XEND;
  return true;
}

bool Package::equipcard(ECardOper oper, const string& cardguid, const string& equipguid, DWORD addpos/* = 0*/)
{
  bool bSuccess = false;

  if (oper == ECARDOPER_EQUIPON)
    bSuccess = cardOn(cardguid, equipguid, addpos);
  else if (oper == ECARDOPER_EQUIPOFF)
    bSuccess = cardOff(cardguid, equipguid, 0);

  if (bSuccess)
  {
    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    m_pUser->refreshDataAtonce();
  }

  EquipCard cmd;
  if (bSuccess)
  {
    cmd.set_oper(oper);
    cmd.set_cardguid(cardguid);
    cmd.set_equipguid(equipguid);
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);


  //card变化事件，一定要写在最后!
  // m_pUser->getEvent().onCardChange(cardguid, equipguid);
  /*if(m_pUser != nullptr && bSuccess)
    m_pUser->m_oBuff.handleEquipCardBuff(oper, cardguid, equipguid);
  */

  return bSuccess;
}

bool Package::restore(const RestoreEquipItemCmd& cmd)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-装备还原] 未找到执行的玩家" << XEND;
    return false;
  }

  if (cmd.strengthlv() == false && cmd.strengthlv2() == false && cmd.cardids_size() == 0 && cmd.enchant() == false && cmd.upgrade() == false)
  {
    XERR << "[包裹-装备还原]"<<m_pUser->accid<<m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,无任何操作" << XEND;
    return false;
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-装备还原]"<<m_pUser->accid<<m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,未找到" << EPACKTYPE_MAIN << "包裹" << XEND;
    return false;
  }

  BasePackage* pPack = nullptr;
  ItemEquip* pEquip = nullptr;
  for (DWORD d = EPACKTYPE_MIN + 1; d < EPACKTYPE_MAX; ++d)
  {
    if (d == EPACKTYPE_STORE)
      continue;

    pPack = m_pUser->getPackage().getPackage(static_cast<EPackType>(d));
    if (pPack == nullptr)
      continue;

    pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(cmd.equipid()));
    if (pEquip != nullptr)
      break;
  }
  if (pPack == nullptr || pEquip == nullptr)
  {
    XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,未找到该装备" << XEND;
    return false;
  }

  DWORD dwCost = 0;
  TVecItemInfo vecReturnAll;
  TVecItemInfo vecReturn;
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  if (cmd.strengthlv() == true && pEquip->getStrengthLv() == 0 && cmd.strengthlv2() == true && pEquip->getStrengthLv2() == 0)
  {
    XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,无强化等级可还原" << XEND;
    return false;
  }
  if (cmd.enchant() && pEquip->isEnchant() == false)
  {
    XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,无附魔可还原" << XEND;
    return false;
  }
  if (cmd.cardids_size() != 0)
  {
    TVecEquipCard vecCards;
    pEquip->getEquipCard(vecCards);
    if (vecCards.empty() == true)
    {
      XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,无可还原的东东" << XEND;
      return false;
    }
  }

  const SItemCFG* pCFG = pEquip->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,未包含合法的配置" << XEND;
    return false;
  }
  if (cmd.upgrade() == true)
  {
    if (pEquip->getLv() <= 0)
    {
      XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,无等级可还原" << XEND;
      return false;
    }
    for (DWORD i = 1; i <= pEquip->getLv(); ++i)
    {
      const SEquipUpgradeCFG* p = pCFG->getUpgradeCFG(i);
      if (p == nullptr)
      {
        XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "失败,等级" << i << "升级配置空" << XEND;
        return false;
      }
      combinItemInfo(vecReturnAll, p->vecMaterial);
      combinItemInfo(vecReturn, p->vecMaterial);
    }
    dwCost += rCFG.getUpgradeRestoreCost(pEquip->getLv());
  }

  for (int i = 0; i < cmd.cardids_size(); ++i)
  {
    const TPairEquipCard* pairCard = pEquip->getCard(cmd.cardids(i));
    if (pairCard == nullptr)
    {
      XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "还原" << cmd.ShortDebugString() << "失败,未找到" << cmd.cardids(i) << "卡片" << XEND;
      return false;
    }
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(pairCard->second);
    if (pCFG == nullptr)
    {
      XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "还原" << cmd.ShortDebugString() << "失败,卡片" << cmd.cardids(i) << pairCard->second << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }
    ItemInfo oCard;
    oCard.set_id(pCFG->dwTypeID);
    oCard.set_count(1);
    oCard.set_quality(pCFG->eQualityType);
    oCard.set_source(ESOURCE_RESTORE);
    combinItemInfo(vecReturnAll, TVecItemInfo{oCard});
    dwCost += rCFG.getCardRestoreCost(pCFG->eQualityType);
  }

  if (cmd.strengthlv() == true || cmd.strengthlv2() == true)
    dwCost += rCFG.dwRestoreStrengthCost;

  if (cmd.strengthlv() == true && pEquip->getStrengthCost() != 0)
  {
    ItemInfo oItem;
    oItem.set_id(ITEM_ZENY);
    oItem.set_count(pEquip->getStrengthCost());
    oItem.set_source(ESOURCE_RESTORE);
    combinItemInfo(vecReturn, oItem);
  }

  if (cmd.strengthlv2() == true && pEquip->getStrengthItemCost().empty() == false)
  {
    combinItemInfo(vecReturnAll, pEquip->getStrengthItemCost());
    combinItemInfo(vecReturn, pEquip->getStrengthItemCost());
  }

  if (cmd.enchant() == true)
    dwCost += rCFG.getEnchantRestoreCost(pEquip->getEnchantData().type());

  if (vecReturnAll.empty() == false && pMainPack->checkAddItem(vecReturnAll, EPACKMETHOD_CHECK_WITHPILE) == false)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
    return false;
  }
  if (m_pUser->checkMoney(EMONEYTYPE_SILVER, dwCost) == false)
  {
    XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "还原" << cmd.ShortDebugString() << "失败,需要zeny" << dwCost << "zeny不足" << XEND;
    return false;
  }
  m_pUser->subMoney(EMONEYTYPE_SILVER, dwCost, ESOURCE_RESTORE);

  if (cmd.strengthlv() == true && pEquip->getStrengthLv() != 0)
  {
    pEquip->setStrengthLv(0);
    pEquip->resetStrengthCost();
  }
  if (cmd.strengthlv2() == true && pEquip->getStrengthLv2() != 0)
  {
    pEquip->setStrengthLv2(0);
    pEquip->resetStrength2Cost();
  }

  if (cmd.enchant() && pEquip->isEnchant())
  {
    pEquip->getEnchantData().Clear();
    pEquip->getPreviewEnchantData().Clear();
  }
  if (cmd.upgrade() == true)
    pEquip->setLv(0);

  for (int i = 0; i < cmd.cardids_size(); ++i)
  {
    if (cardOff(cmd.cardids(i), pEquip->getGUID(), 0) == false)
    {
      XERR << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "还原" << cmd.ShortDebugString() << "卡片" << cmd.cardids(i) << "卸载失败" << XEND;
      continue;
    }
  }

  for (auto &v : vecReturn)
    v.set_source(ESOURCE_RESTORE);

  //pMainPack->addItemFull(vecReturn);
  pMainPack->addItem(vecReturn, EPACKMETHOD_CHECK_WITHPILE);
  pMainPack->update(xTime::getCurSec());

  pPack->setUpdateIDs(pEquip->getGUID());

  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();
  refreshEnableBuffs(pEquip);

  RestoreEquipItemCmd retcmd;
  PROTOBUF(retcmd, retsend, retlen);
  m_pUser->sendCmdToMe(retsend, retlen);

  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_EQUIP_RESTORE);

  XLOG << "[包裹-装备还原]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原" << cmd.ShortDebugString() << "成功,总共消耗" << dwCost << "zeny" << XEND;
  return true;
}

bool Package::exchangeCard(ExchangeCardItemCmd& cmd)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(cmd.npcid());
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
  {
    XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未通过npc进行卡片操作" << XEND;
    return false;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "玩家和npc不在同一地图" << XEND;
    return false;
  }
  if (::getXZDistance(m_pUser->getPos(), pNpc->getPos()) > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "玩家和npc距离过远" << XEND;
    return false;
  }
  ENpcFunction eFunction = ENPCFUNCTION_MIN;
  switch (cmd.type())
  {
  case EEXCHANGECARDTYPE_DRAW:
    eFunction = ENPCFUNCTION_CARD_EXCHANGE; break;
  case EEXCHANGECARDTYPE_COMPOSE:
    eFunction = ENPCFUNCTION_CARD_COMPOSE; break;
  case EEXCHANGECARDTYPE_DECOMPOSE:
    eFunction = ENPCFUNCTION_CARD_DECOMPOSE; break;
  default:
    return false;
  }
  if (pNpc->getCFG()->stNpcFunc.hasFunction(eFunction) == false)
  {
    XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未在正确的npc旁进行卡片操作" << XEND;
    return false;
  }

  bool bFirst = false;
  if (cmd.type() == EEXCHANGECARDTYPE_DRAW)
  {
    if (cmd.material_size() != 3)
    {
      XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "材料数量" << cmd.material_size() << "不正确" << XEND;
      return false;
    }

    const SCardMiscCFG& rCFG = MiscConfig::getMe().getCardMiscCFG();
    Variable& rVar = m_pUser->getVar();
    DWORD dwCount = rVar.getVarValue(EVARTYPE_EXCHANGECARD_DRAWMAX);
    if (dwCount >= rCFG.dwExchangeCardMaxDraw)
    {
      MsgManager::sendMsg(m_pUser->id, 25606);
      XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "超过当日上限" << rCFG.dwExchangeCardMaxDraw << XEND;
      return false;
    }

    map<string, DWORD> mapCards;
    for (int i = 0; i < cmd.material_size(); ++i)
      mapCards[cmd.material(i)]++;

    for (auto &m : mapCards)
    {
      if (checkItemCount(m.first, m.second, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EXCHANGE) == false)
      {
        XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "材料" << m.first << "数量不足" << XEND;
        return false;
      }
    }

    vector<EQualityType> vecQuality;
    for (auto &m : mapCards)
    {
      ItemCard* pCard = dynamic_cast<ItemCard*>(getItem(m.first));
      if (pCard == nullptr || pCard->getCFG() == nullptr)
      {
        XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "材料" << m.first << "不存在" << XEND;
        return false;
      }
      if (pCard->getCFG()->hasCardFunc(ECARDFUNC_NOMATERIAL) == true)
      {
        XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "材料" << m.first << "不可作为材料" << XEND;
        return false;
      }
      for (DWORD d = 0; d < m.second; ++d)
        vecQuality.push_back(pCard->getCFG()->eQualityType);
    }
    sort(vecQuality.begin(), vecQuality.end());

    const SCardRateCFG* pRateCFG = ItemConfig::getMe().getCardRateCFG(vecQuality);
    if (pRateCFG == nullptr)
    {
      XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未找到";
      for (auto &v : vecQuality)
        XERR << v;
      XERR << "的组合" << XEND;
      return false;
    }
    if (m_pUser->checkMoney(EMONEYTYPE_SILVER, pRateCFG->dwZeny) == false)
    {
      XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "银币不足,需要" << pRateCFG->dwZeny << XEND;
      return false;
    }

    DWORD dwRand = randBetween(1, 10000);
    const SItemCFG* pCFG = nullptr;
    if (dwRand <= pRateCFG->dwWriteRate)
      pCFG = ItemConfig::getMe().randCardByQuality(EQUALITYTYPE_WHITE);
    else if (dwRand <= pRateCFG->dwGreenRate)
      pCFG = ItemConfig::getMe().randCardByQuality(EQUALITYTYPE_GREEN);
    else if (dwRand <= pRateCFG->dwBlueRate)
      pCFG = ItemConfig::getMe().randCardByQuality(EQUALITYTYPE_BLUE);
    else if (dwRand <= pRateCFG->dwPurpleRate)
      pCFG = ItemConfig::getMe().randCardByQuality(EQUALITYTYPE_PURPLE);

    if (pCFG == nullptr)
    {
      XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "随机概率" << dwRand << "随机卡片为空" << XEND;
      return false;
    }

    ItemInfo oItem;
    oItem.set_id(pCFG->dwTypeID);
    oItem.set_count(1);
    oItem.set_source(ESOURCE_EXCHANGECARD);

    TVecItemInfo vecItem;
    combinItemInfo(vecItem, pRateCFG->vecReward);
    combinItemInfo(vecItem, TVecItemInfo{oItem});

    for (auto &m : mapCards)
      reduceItem(m.first, m.second, ESOURCE_EXCHANGECARD, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EXCHANGE);

    m_pUser->subMoney(EMONEYTYPE_SILVER, pRateCFG->dwZeny, ESOURCE_EXCHANGECARD);
    m_pUser->getPackage().addItem(vecItem, EPACKMETHOD_AVAILABLE, false, true, false);
    rVar.setVarValue(EVARTYPE_EXCHANGECARD_DRAWMAX, dwCount + 1);

    bFirst = m_pUser->getUserSceneData().addFirstActionDone(EFIRSTACTION_EXCHANGECARD);

    cmd.set_charid(m_pUser->id);
    cmd.set_cardid(pCFG->dwTypeID);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToNine(send, len);
    m_pUser->getServant().onFinishEvent(ETRIGGER_CARD_RESET);
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_CARD_RESET);

    XLOG << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "随机概率" << dwRand << "成功随机出卡片" << pCFG->dwTypeID << "当日已抽卡" << dwCount + 1 << "次" << XEND;
  }
  else if (cmd.type() == EEXCHANGECARDTYPE_COMPOSE)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(cmd.cardid());
    if (pCFG == nullptr)
    {
      XERR << "[包裹-卡片合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成卡片" << cmd.cardid() << "失败,未在Table_Item.txt表中找到" << XEND;
      return false;
    }
    if (pCFG->isCardCompoaseTimeValid() == false)
    {
      DWORD dwNow = now();
      DWORD dwBranch = BaseConfig::getMe().getInnerBranch();
      XERR << "[包裹-卡片合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "合成卡片" << cmd.cardid() << "失败, 当前服务器分支" << dwBranch << "该物品有效期在" << pCFG->dwCardComposeStartTime << pCFG->dwCardComposeEndTime << "之内,当前时间为" << dwNow << XEND;
      return false;
    }

    if (m_pUser->checkCompose(pCFG->dwComposeID, EPACKMETHOD_AVAILABLE, GACTIVITY_MIN, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EXCHANGE) == false)
    {
      XERR << "[包裹-卡片合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成卡片" << cmd.cardid() << "失败" << XEND;
      return false;
    }
    m_pUser->doCompose(pCFG->dwComposeID, EPACKMETHOD_AVAILABLE, GACTIVITY_MIN, false, true, false, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EXCHANGE);
    bFirst = m_pUser->getUserSceneData().addFirstActionDone(EFIRSTACTION_COMPOSECARD);

    cmd.set_charid(m_pUser->id);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToNine(send, len);
    m_pUser->getServant().onFinishEvent(ETRIGGER_CARD_CUSTOMIZE);

    XLOG << "[包裹-卡片合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成卡片" << cmd.cardid() << "成功" << XEND;
  }
  else if (cmd.type() == EEXCHANGECARDTYPE_DECOMPOSE)
  {
    // 目前只支持一次分解一张卡
    if (cmd.material_size() != 1)
      return false;
    map<string, DWORD> guids;
    for (int i = 0; i < cmd.material_size(); ++i)
    {
      if (guids.find(cmd.material(i)) == guids.end())
        guids[cmd.material(i)] = 1;
      else
        guids[cmd.material(i)] += 1;
    }
    TVecItemInfo items;
    if (cardDecompose(guids, items) == false)
      return false;
    m_pUser->getUserSceneData().addFirstActionDone(EFIRSTACTION_DECOMPOSECARD);

    for (auto& item : items)
    {
      ItemInfo* p = cmd.add_items();
      if (p)
        p->CopyFrom(item);
    }
    cmd.set_charid(m_pUser->id);

    PROTOBUF(cmd, send, len);
    if (!cmd.anim())
      m_pUser->sendCmdToNine(send, len);
    else
      m_pUser->sendCmdToMe(send, len);

    return true;
  }
  else
  {
    XERR << "[包裹-卡片抽取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "type :" << cmd.type() << "不合法" << XEND;
    return false;
  }

  m_bCardOperationAnim = bFirst || cmd.anim();
  return true;
}

bool Package::hatchEgg(const EggHatchPetCmd& cmd)
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return false;

  BasePackage* pPack = getPackage(EPACKTYPE_PET);
  if (pPack == nullptr)
  {
    XERR << "[包裹-孵化蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "孵化宠物蛋" << cmd.guid() << "失败，未找到" << EPACKTYPE_PET << XEND;
    return false;
  }

  ItemEgg* pEgg = dynamic_cast<ItemEgg*>(pPack->getItem(cmd.guid()));
  if (pEgg == nullptr)
  {
    XERR << "[包裹-孵化蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "孵化宠物蛋" << cmd.guid() << "失败,未找到宠物蛋"<< XEND;
    return false;
  }

  EggData& egg = pEgg->getEggData();
  if (egg.name().empty())
  {
    if (MiscConfig::getMe().getSystemCFG().checkNameValid(cmd.name(), ENAMETYPE_PET) != ESYSTEMMSG_ID_MIN)
    {
      MsgManager::sendMsg(m_pUser->id, 1005);
      XERR << "[包裹-孵化蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "孵化宠物蛋" << cmd.guid() << "失败,宠物名" << cmd.name() << "不合法" << XEND;
      return false;
    }
    DWORD petid = PetConfig::getMe().getPetIDByItem(pEgg->getTypeID());
    if (petid == 0)
    {
      XERR << "[包裹-孵化蛋], 孵化失败, 找不到宠物信息, 玩家:" << m_pUser->name << m_pUser->id << "宠物蛋:" << pEgg->getTypeID() << XEND;
      return false;
    }
    egg.set_id(petid);
    egg.set_name(cmd.name());
  }

  bool bFirst = egg.skillids_size() == 0;
  egg.set_guid(pEgg->getGUID());
  UserPet& rPet = m_pUser->getUserPet();
  /*TSetDWORD setIDs;
  rPet.collectPetList(setIDs);
  for (auto &s : setIDs)
  {
    EggRestorePetCmd rcmd;
    rcmd.set_petid(s);
    restoreEgg(rcmd);
  }*/
  if (rPet.addPet(egg, bFirst) == false)
  {
    XERR << "[包裹-孵化蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "孵化宠物蛋" << cmd.guid() << "失败,宠物名" << cmd.name() << "不合法" << XEND;
    return false;
  }

  ItemData oData;
  pEgg->toItemData(&oData);

  pPack->reduceItem(cmd.guid(), ESOURCE_PET);
  m_pUser->getQuest().onPetAdd();
  XLOG << "[包裹-孵化蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "孵化宠物蛋" << oData.ShortDebugString() << "成功,在" << m_pUser->getScene()->name << "召唤出" << XEND;
  return true;
}

bool Package::restoreEgg(const EggRestorePetCmd& cmd)
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return false;

  if (m_pUser->getUserPet().toEgg(cmd.petid()) == false)
  {
    XERR << "[包裹-还原蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原宠物蛋" << cmd.petid() << "失败,还原失败" << XEND;
    return false;
  }

  XLOG << "[包裹-还原蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "还原宠物蛋" << cmd.petid() << "成功" << XEND;
  return true;
}

// 请求session获取道具的兑换码, 每一个道具绑定一个兑换码
void Package::useItemCode(UseCodItemCmd& cmd)
{
  BasePackage* pPkg = getPackage(EPACKTYPE_MAIN);
  if (!pPkg)
    return;
  ItemCode*pItem = dynamic_cast<ItemCode*>(pPkg->getItem(cmd.guid()));
  if (!pItem)
    return;
  if (!pItem->canUse())
  {
    XERR << "[道具-兑换码-使用] 已经使用过了" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "guid" << cmd.guid() << XEND;
    return;
  }

  if (pItem->getType() == EITEMTYPE_CODE && m_pUser->checkPwd(EUNLOCKTYPE_ITEM_CODE, 0) == false)
  {
    XERR << "[道具-兑换码-使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "失败,密码验证失败" << XEND;
    return;
  }
  if (pItem->getType() == EITEMTYPE_KFC_CODE && m_pUser->canUseItem(pItem->getTypeID(), 0) == false)
  {
    XERR << "[道具-兑换码-使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "失败,道具不可使用" << XEND;
    return;
  }

  UseItemCodeSessionCmd useCmd;
  useCmd.set_charid(m_pUser->id);
  useCmd.set_guid(cmd.guid());
  useCmd.set_itemid(pItem->getTypeID());
  useCmd.set_type(pItem->getType());
  PROTOBUF(useCmd, send, len);
  thisServer->sendCmdToSession(send, len);
  XLOG << "[道具-兑换码-使用] 向Session获取礼包码" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "guid" << cmd.guid() << XEND;
}

// session返回申请的兑换码, 成功则绑定到道具上
void Package::useItemCodeSessionRes(const UseItemCodeSessionCmd& cmd)
{
  if (cmd.code().empty())
    return;
  BasePackage* pPkg = nullptr;
  ItemCode*pItem = dynamic_cast<ItemCode*>(getItem(cmd.guid(), &pPkg));
  if (!pItem)
    return;
  if (!pItem->canUse())
  {
    XERR << "[道具-兑换码-使用] 已经使用过了" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "guid" << cmd.guid() << XEND;
    return;
  }  

  pItem->setCode(cmd.code());
  if (pPkg)
    pPkg->setUpdateIDs(cmd.guid());
  m_pUser->refreshDataAtonce();

  UseCodItemCmd useCmd;
  useCmd.set_guid(cmd.guid());
  useCmd.set_code(cmd.code());
  PROTOBUF(useCmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[道具-兑换码-使用] 使用成功" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "guid" << cmd.guid() <<"itemid"<<cmd.itemid()<<"code"<<cmd.code() << XEND;
}

// 登录时查找玩家包裹中的所有兑换码道具, 检查兑换码是否已使用
void Package::reqUsedItemCode()
{
  bool req = false;
  auto checkF = [&](const ItemBase* pItemBase) {
    if (!pItemBase)
      return;
    if (pItemBase->getType() != EITEMTYPE_CODE && pItemBase->getType() != EITEMTYPE_KFC_CODE)
      return;
    const ItemCode* pCode = dynamic_cast<const ItemCode*>(pItemBase);
    if (pCode && pCode->needReqUsed())
    {
      req = true;
      return;
    }
  };
  
  getPackage(EPACKTYPE_MAIN)->foreach(checkF);
  if (!req)
    getPackage(EPACKTYPE_TEMP_MAIN)->foreach(checkF);
  if (!req)
    getPackage(EPACKTYPE_PERSONAL_STORE)->foreach(checkF);
  if (!req)
    getPackage(EPACKTYPE_STORE)->foreach(checkF);
  if (!req)
    getPackage(EPACKTYPE_TEMP_MAIN)->foreach(checkF);

  if (!req)
    return;
  ReqUsedItemCodeSessionCmd cmd;
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

}

// session返回查询的兑换码使用情况, 并将状态更新到道具上
void Package::reqUsedItemCodeSessionRes(const ReqUsedItemCodeSessionCmd& cmd)
{
  if (cmd.guid_size() == 0)
    return;
  
  auto setExchanged = [&](const string& guid) {
    BasePackage* pPkg = nullptr;
    ItemBase* pItem = getItem(guid, &pPkg);
    if (!pItem) return;
    ItemCode* pCode = dynamic_cast<ItemCode*>(pItem);
    if (!pCode) return;
    pCode->setExchanged(true);
    if (pPkg)
    {
      if (pCode->getType() == EITEMTYPE_KFC_CODE)
        pPkg->reduceItem(pCode->getGUID(), ESOURCE_KFC_ACTIVITY);
      pPkg->setUpdateIDs(guid);
    }
  };
  
  for (int i = 0; i < cmd.guid_size(); i++)
  {
    setExchanged(cmd.guid(i));
  }
  XLOG << "[道具-兑换码] 更新是否使用记录" << m_pUser->id << cmd.guid_size() << XEND;
}

EError Package::equipCompose(const EquipComposeItemCmd& cmd)
{
  const SEquipComposeCFG* pCFG = ItemConfig::getMe().getEquipComposeCFG(cmd.id());
  if (pCFG == nullptr)
  {
    XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,未在 Table_EquipCompose.txt 表找到" << XEND;
    return EERROR_FAIL;
  }
  const SItemCFG* pProductCFG = ItemConfig::getMe().getItemCFG(cmd.id());
  if (pProductCFG == nullptr)
  {
    XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,未在 Table_Item.txt 表找到" << XEND;
    return EERROR_FAIL;
  }

  TSetString setIDs;
  for (int i = 0; i < cmd.materialequips_size(); ++i)
  {
    const string& guid = cmd.materialequips(i);
    auto s = setIDs.find(guid);
    if (s != setIDs.end())
    {
      XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,guid :" << guid << "重复了" << XEND;
      return EERROR_FAIL;
    }
    setIDs.insert(guid);
  }

  struct SMaterial
  {
    DWORD dwID = 0;
    DWORD dwLv = 0;
    BasePackage* pPack = nullptr;
    ItemEquip* pEquip = nullptr;
  };
  vector<SMaterial> vecMaterial;
  for (auto &v : pCFG->vecMaterialEquip)
  {
    SMaterial stMaterial;
    stMaterial.dwID = v.base().id();
    stMaterial.dwLv = v.equip().lv();
    vecMaterial.push_back(stMaterial);
  }
  map<ItemEquip*, BasePackage*> mapMaterial;
  for (auto &s : setIDs)
  {
    BasePackage* pPack = nullptr;
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(getItem(s, &pPack));
    if (pEquip == nullptr || pPack == nullptr)
    {
      XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,guid :" << s << "未找到" << XEND;
      return EERROR_FAIL;
    }
    if (pEquip->canBeComposeMaterial() == false)
    {
      XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,guid :" << s << "不能作为合成材料" << XEND;
      return EERROR_FAIL;
    }
    for (auto &v : vecMaterial)
    {
      SMaterial& rMaterial = v;
      if (pEquip->getTypeID() == rMaterial.dwID && pEquip->getLv() >= rMaterial.dwLv)
      {
        rMaterial.pPack = pPack;
        rMaterial.pEquip = pEquip;
        break;
      }
    }
  }

  for (auto &v : vecMaterial)
  {
    if (v.pPack == nullptr || v.pEquip == nullptr)
    {
      XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,缺少 id :" << v.dwID << "lv :" << v.dwLv << "的装备材料" << XEND;
      return EERROR_FAIL;
    }
  }

  ItemData oMainOldData;
  vecMaterial[0].pEquip->toItemData(&oMainOldData);

  Package& rPackage = m_pUser->getPackage();
  if (rPackage.checkItemCount(pCFG->vecMaterialItem, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EQUIPCOMPOSE) == false)
  {
    XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,材料不足" << XEND;
    return EERROR_FAIL;
  }
  if (m_pUser->checkMoney(EMONEYTYPE_SILVER, pCFG->dwZenyCost) == false)
  {
    XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "失败,zeny不足, 需要" << pCFG->dwZenyCost << XEND;
    return EERROR_FAIL;
  }

  for (auto &v : vecMaterial)
    v.pPack->reduceItem(v.pEquip->getGUID(), ESOURCE_EQUIP_COMPOSE, v.pEquip->getCount());
  rPackage.reduceItem(pCFG->vecMaterialItem, ESOURCE_EQUIP_COMPOSE, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EQUIPCOMPOSE);
  m_pUser->subMoney(EMONEYTYPE_SILVER, pCFG->dwZenyCost, ESOURCE_EQUIP_COMPOSE);

  oMainOldData.mutable_base()->clear_guid();
  oMainOldData.mutable_base()->set_id(cmd.id());
  oMainOldData.mutable_base()->set_count(1);
  oMainOldData.mutable_equip()->clear_lv();
  if (rPackage.addItem(oMainOldData, EPACKMETHOD_NOCHECK) == false)
  {
    XERR << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "成功, 添加成品" << oMainOldData.ShortDebugString() << "失败" << XEND;
    return EERROR_FAIL;
  }

  XLOG << "[包裹-装备合成]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "合成 id :" << cmd.id() << "成功, 获得融合装备" << oMainOldData.ShortDebugString() << XEND;
  return EERROR_SUCCESS;
}

bool Package::removeEquipcard(const string& cardguid, const string& equipguid)
{
  bool bSuccess = false;
  bSuccess = cardOff(cardguid, equipguid, 0, true);
  if (bSuccess)
  {
    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    m_pUser->refreshDataAtonce();
  }

  EquipCard cmd;
  if (bSuccess)
  {
    cmd.set_oper(ECARDOPER_EQUIPOFF);
    cmd.set_cardguid(cardguid);
    cmd.set_equipguid(equipguid);
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);


  //card变化事件，一定要写在最后!
  // m_pUser->getEvent().onCardChange(cardguid, equipguid);
  /*if(m_pUser != nullptr && bSuccess)
  m_pUser->m_oBuff.handleEquipCardBuff(oper, cardguid, equipguid);
  */

  return bSuccess;
}

bool Package::equipAllCardOff(const string& equipguid)
{
  /*// check equip
  BasePackage* pPackage = getPackage(EPACKTYPE_MAIN);
  if (pPackage == nullptr)
    return false;
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pPackage->getItem(equipguid));
  if (pEquip == nullptr)
  {
    pPackage = getPackage(EPACKTYPE_EQUIP);
    if (pPackage != nullptr)
      pEquip = dynamic_cast<ItemEquip*>(pPackage->getItem(equipguid));
    if (pEquip == nullptr)
    {
      pPackage = getPackage(EPACKTYPE_FASHIONEQUIP);
      if (pPackage != nullptr)
        pEquip = dynamic_cast<ItemEquip*>(pPackage->getItem(equipguid));
    }
  }
  if (pEquip == nullptr)
    return false;*/

  BasePackage* pPack = nullptr;
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(getItem(equipguid, &pPack));
  if (pPack == nullptr || pEquip == nullptr)
    return false;

  const TMapEquipCard cardList = pEquip->getCardList();
  for (auto it = cardList.begin(); it != cardList.end(); ++it)
  {
    if (equipcard(ECARDOPER_EQUIPOFF, it->second.first, equipguid) == false)
      return false;
  }
  return true;
}

bool Package::ride(ERideType eType, DWORD dwID /*= 0*/)
{
  if (eType == ERIDETYPE_ON)
  {
    MainPackage* pPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
    if (pPack == nullptr)
      return false;
    const string& guid = pPack->getMountGUID(dwID);
    return equipOn(EEQUIPPOS_MOUNT, guid);
  }
  else if (eType == ERIDETYPE_OFF)
  {
    EquipPackage* pPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
    if (pPack == nullptr)
      return false;

    ItemEquip* pEquip = pPack->getEquip(EEQUIPPOS_MOUNT);
    if (pEquip == nullptr)
      return false;

    return equipOff(pEquip->getGUID());
  }

  return false;
}

void Package::timer(DWORD curSec)
{
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    if (m_pPackage[i] != nullptr)
      m_pPackage[i]->update(curSec);
  }

  updateFashionPackage();
}

bool Package::createPackage()
{
  auto dpack = [this]()
  {
    for (auto i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
      SAFE_DELETE(m_pPackage[i]);
  };

  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    SAFE_DELETE(m_pPackage[i]);

    if(i == EPACKTYPE_MAIN)
    {
      m_pPackage[i] = new MainPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_EQUIP)
    {
      m_pPackage[i] = new EquipPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_FASHION)
    {
      m_pPackage[i] = new FashionPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_FASHIONEQUIP)
    {
      m_pPackage[i] = new FashionEquipPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    /*else if (i == EPACKTYPE_CARD)
    {
      m_pPackage[i] = new CardPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }*/
    else if (i == EPACKTYPE_STORE)
    {
      m_pPackage[i] = new StorePackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_PERSONAL_STORE)
    {
      m_pPackage[i] = new PersonalStorePackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_TEMP_MAIN)
    {
      m_pPackage[i] = new TempMainPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_BARROW)
    {
      m_pPackage[i] = new BarrowPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_QUEST)
    {
      m_pPackage[i] = new QuestPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_FOOD)
    {
      m_pPackage[i] = new FoodPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
    else if (i == EPACKTYPE_PET)
    {
      m_pPackage[i] = new PetPackage(m_pUser);
      if (m_pPackage[i] == nullptr)
      {
        dpack();
        return false;
      }
    }
  }

  return true;
}

bool Package::equipOn(EEquipPos ePos, const string& guid, bool bTransfer /*= false*/)
{
  // equip package
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return false;

  // check is equiped
  ItemBase* pBase = pEquipPack->getItem(guid);
  if (pBase != nullptr)
    return false;

  // get package and equip
  BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  ItemEquip* pEquip = nullptr;
  if (pPack != nullptr)
    pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(guid));
  if (pPack == nullptr || pEquip == nullptr)
    return false;

  // get valid pos
  ePos = MiscConfig::getMe().getItemCFG().getValidEquipPos(ePos, pEquip->getEquipType());
  if (ePos == EEQUIPPOS_MIN)
    return false;
  // 装备位处于被脱卸状态
  if (isEquipForceOff(ePos))
    return false;
  // 装备位是否被buff禁止穿戴
  if (m_pUser->m_oBuff.isEquipForbid(EPACKTYPE_EQUIP, ePos, EEQUIPOPER_ON))
    return false;
  if (ItemConfig::getMe().isArtifactPos(ePos))
  {
    EEquipPos epos = ItemConfig::getMe().getArtifactEquipPos(ePos);
    if (epos == EEQUIPPOS_MIN)
    {
      XERR << "[包裹-穿戴]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "穿戴" << pEquip->getTypeID() << "pos:" << ePos << "未找到对应装备位" << XEND;
      return false;
    }
    ItemEquip* equip = dynamic_cast<ItemEquip*>(pEquipPack->getEquip(epos));
    if (equip == nullptr || ItemConfig::getMe().isSameItemType(equip->getType(), pEquip->getType()) == false)
    {
      MsgManager::sendMsg(m_pUser->id, 3791);
      return false;
    }
  }
  m_pUser->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);

  // check can equip
  if (pEquipPack->canEquip(m_pUser, pEquip, true) == false)
    return false;

  // equip off old equip if exist
  ItemEquip* pOldEquip = pEquipPack->getEquip(ePos);
  if (pOldEquip != nullptr)
  {
    if (equipOff(pOldEquip->getGUID(), pEquip->getIndex(), false) == false)
      return false;

    if (bTransfer)
    {
      if (transfer(pOldEquip, pEquip) == false)
        return false;
    }
  }

  // remove equip
  pPack->reduceItem(pEquip->getGUID(), ESOURCE_EQUIP, pEquip->getCount(), false);

  // add equip package
  pEquip->setNew(false);
  pEquipPack->setEquip(m_pUser, ePos, pEquip);

  if (ePos == EEQUIPPOS_MOUNT)
    m_pUser->m_oHands.breakup();
  else if (ePos == EEQUIPPOS_BARROW)
  {
    m_pUser->m_oHands.breakup();
    DWORD dwPartnerID = MiscConfig::getMe().getPetCFG().getNpcBarrow(pEquip->getTypeID());
    if (dwPartnerID == 0)
      XERR << "[包裹-穿戴]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "穿戴" << pEquip->getTypeID() << "未发现手推车形象" << XEND;
    else
      m_pUser->getPet().setPartnerID(dwPartnerID);
  }
  else if (ePos == EEQUIPPOS_ARMOUR)
    m_pUser->onAltmanFashionEquip();

  m_pUser->getEvent().onEquipChange(EEQUIPOPER_ON, guid);

  //platlog
  DWORD type;
  if (bTransfer)
    type = 3;
  else
    type = 1;

  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_EquipOn;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  if (pOldEquip)
  {
    PlatLogManager::getMe().EquipLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eType,
      eid,
      type,
      pOldEquip->getTypeID(),
      pOldEquip->getGUID(),
      pOldEquip->getStrengthLv(),
      pOldEquip->getRefineLv(),
      pOldEquip->isDamaged(),
      pEquip->getTypeID(),
      pEquip->getGUID(),
      pEquip->getStrengthLv(),
      pEquip->getRefineLv(),
      pEquip->isDamaged()
    );
  }
  else
  {
    std::string oldGid;
    PlatLogManager::getMe().EquipLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eType,
      eid,
      type,
      0, /*old equip id*/
      oldGid,
      0,
      0,
      0,
      pEquip->getTypeID(),
      pEquip->getGUID(),
      pEquip->getStrengthLv(),
      pEquip->getRefineLv(),
      pEquip->isDamaged()
    );

  }
  return true;
}

bool Package::equipOff(const string& guid, int index /*= 0*/, bool bSlotCheck /*= false*/)
{
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return false;

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(guid));
  if (pEquip == nullptr)
    return false;

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
    return false;
  if (bSlotCheck && pMainPack->checkAddItemObj(pEquip) == false)
    return false;

  EEquipPos ePos = static_cast<EEquipPos>(pEquip->getIndex());

  // 装备位是否被buff禁止脱下
  if (m_pUser->m_oBuff.isEquipForbid(EPACKTYPE_EQUIP, ePos, EEQUIPOPER_OFF))
    return false;

  // equip off first
  pEquipPack->setEquip(m_pUser, ePos, nullptr);

  pEquip->setEquiped(false);
  if (index != 0)
    pEquip->setIndex(index);
  else
    pEquip->setIndex(pMainPack->getNextIndex());

  pMainPack->addItemObj(pEquip, true);
  pMainPack->setUpdateIDs(pEquip->getGUID());

  if (ePos == EEQUIPPOS_BARROW)
    m_pUser->getPet().setPartnerID(0);
  else if (ePos == EEQUIPPOS_ARMOUR)
    m_pUser->onAltmanFashionEquip();

  m_pUser->getEvent().onEquipChange(EEQUIPOPER_OFF, guid);

  //platlog
  DWORD type = 2;
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_EquipOff;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  std::string newGid;
  PlatLogManager::getMe().EquipLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    type,
    pEquip->getTypeID(),
    pEquip->getGUID(),
    pEquip->getStrengthLv(),
    pEquip->getRefineLv(),
    pEquip->isDamaged(),
    0,
    newGid, /*new equip guid*/
    0,
    0,
    false
  );

  // 武器脱掉后须同时脱掉神器
  if (ItemConfig::getMe().isArtifactPos(ePos) == false)
  {
    EEquipPos apos = ItemConfig::getMe().getArtifactEquipPos(ePos);
    if (apos != EEQUIPPOS_MIN)
    {
      ItemEquip* pArtifact = pEquipPack->getEquip(apos);
      if (pArtifact && static_cast<EEquipPos>(pArtifact->getIndex()) == apos)
      {
        if (equipOff(pArtifact->getGUID()) && pArtifact->getCFG())
          MsgManager::sendMsg(m_pUser->id, 3792, MsgParams(pArtifact->getCFG()->strNameZh));
      }
    }
  }

  return true;
}

bool Package::equipOff(EEquipPos ePos, bool bSlotCheck /*= false*/)
{
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return false;

  ItemEquip* pEquip = pEquipPack->getEquip(ePos);
  if (pEquip == nullptr)
    return false;

  // 装备位是否被buff禁止脱下
  if (m_pUser->m_oBuff.isEquipForbid(EPACKTYPE_EQUIP, ePos, EEQUIPOPER_OFF))
    return false;

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
    return false;
  if (bSlotCheck && pMainPack->checkAddItemObj(pEquip) == false)
    return false;

  pEquipPack->setEquip(m_pUser, ePos, nullptr);
  pEquip->setEquiped(false);
  pEquip->setIndex(pMainPack->getNextIndex());
  pMainPack->addItemObj(pEquip, true);
  pMainPack->setUpdateIDs(pEquip->getGUID());
  m_pUser->m_oBuff.onEquipChange(pEquip, EEQUIPOPER_OFFPOS);

  // 武器脱掉后须同时脱掉神器
  if (ItemConfig::getMe().isArtifactPos(ePos) == false)
  {
    EEquipPos apos = ItemConfig::getMe().getArtifactEquipPos(ePos);
    if (apos != EEQUIPPOS_MIN)
    {
      ItemEquip* pArtifact = pEquipPack->getEquip(apos);
      if (pArtifact && static_cast<EEquipPos>(pArtifact->getIndex()) == apos)
      {
        if (equipOff(apos) && pArtifact->getCFG())
          MsgManager::sendMsg(m_pUser->id, 3792, MsgParams(pArtifact->getCFG()->strNameZh));
      }
    }
  }

  return true;
}

// 拆卸神器
bool Package::equipOffAritifact()
{
  for (auto& pos : ItemConfig::getMe().getArtifactPos())
    equipOff(pos);
  return true;
}

bool Package::canHint(EPackType eType, ItemBase* pItem)
{
  if (pItem == nullptr)
    return false;
  DWORD dwItemID = pItem->getTypeID();
  if (isHint(dwItemID) == true)
    return false;
  if (MiscConfig::getMe().getItemCFG().isExtraHint(dwItemID) == true)
    return true;
  if (eType != EPACKTYPE_MAIN)
    return false;
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
  if (pCFG == nullptr || pCFG->eEquipType == EEQUIPTYPE_MIN)
    return false;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*> (getPackage(EPACKTYPE_EQUIP));
  return pEquipPack && pEquipPack->canEquip(m_pUser, pItem);
}

bool Package::equipOffAll()
{
  // 策划说现在不需要了，先屏蔽
  return true;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return false;

  /*for (int i = EEQUIPTYPE_MIN + 1; i < EEQUIPTYPE_MAX; ++i)
  {
    ItemEquip* pEquip = pEquipPack->getEquip(static_cast<EEquipType>(i));
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == false)
      equipOff(pEquip->getGUID());
  }*/

  return true;
}

bool Package::equipPutFashion(EEquipPos ePos, const string& guid, bool bTransfer /*= false*/)
{
  // equip package
  FashionEquipPackage* pEquipPack = dynamic_cast<FashionEquipPackage*>(getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pEquipPack == nullptr)
    return false;

  // check is equiped
  ItemBase* pBase = pEquipPack->getItem(guid);
  if (pBase != nullptr)
    return false;

  // get package and equip
  BasePackage* pPack = getPackage(EPACKTYPE_MAIN);
  ItemEquip* pEquip = nullptr;
  if (pPack != nullptr)
    pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(guid));
  if (pPack == nullptr || pEquip == nullptr)
    return false;

  // get valid pos
  ePos = MiscConfig::getMe().getItemCFG().getValidEquipPos(ePos, pEquip->getEquipType());
  if (ePos == EEQUIPPOS_MIN)
    return false;

  // 装备位是否被buff禁止穿戴
  if (m_pUser->m_oBuff.isEquipForbid(EPACKTYPE_FASHIONEQUIP, ePos, EEQUIPOPER_PUTFASHION))
    return false;

  // check can
  if (pEquipPack->canEquip(m_pUser, pEquip) == false)
    return false;

  // equip off old equip if exist
  ItemEquip* pOldEquip = pEquipPack->getEquip(ePos);
  if (pOldEquip != nullptr)
  {
    if (equipOffFashion(pOldEquip->getGUID(), pEquip->getIndex()) == false)
      return false;

    if (bTransfer)
    {
      if (transfer(pOldEquip, pEquip) == false)
        return false;
    }
  }

  pPack->reduceItem(pEquip->getGUID(), ESOURCE_EQUIP, 1, false);

  // add equip package
  pEquip->setNew(false);
  pEquipPack->setEquip(m_pUser, ePos, pEquip);

  return true;
}

bool Package::equipOffFashion(const string& guid, int index /*= 0*/)
{
  FashionEquipPackage* pEquipPack = dynamic_cast<FashionEquipPackage*>(getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pEquipPack == nullptr)
    return false;

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(guid));
  if (pEquip == nullptr)
    return false;

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
    return false;

  //if (pMainPack->checkAddItemObj(pEquip) == false)
  //  return false;

  EEquipPos ePos = static_cast<EEquipPos>(pEquip->getIndex());

  // 装备位是否被buff禁止脱下
  if (m_pUser->m_oBuff.isEquipForbid(EPACKTYPE_FASHIONEQUIP, ePos, EEQUIPOPER_OFFFASHION))
    return false;

  pEquipPack->setEquip(m_pUser, ePos, nullptr);

  pEquip->setEquiped(false);
  if (index != 0)
    pEquip->setIndex(index);
  else
    pEquip->setIndex(pMainPack->getNextIndex());

  pMainPack->addItemObj(pEquip, true);
  pMainPack->setUpdateIDs(pEquip->getGUID());
  return true;
}

bool Package::putStore(const string& guid, DWORD dwCount)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-仓库]" << "存入" << guid << "失败,未找到玩家" << XEND;
    return false;
  }
  if (m_bStoreOpened == false)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,未正确打开仓库" << XEND;
    return false;
  }
  /*if (canStoreOpen() == false)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,仓库操作中" << XEND;
    return false;
  }*/

  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  if (m_pUser->getLevel() < rCFG.dwStoreBaseLvReq)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,未达到等级需求" << XEND;
    return false;
  }

  // check npc
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,未通过npc存入" << XEND;
    return false;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,和npc不在同一场景" << XEND;
    return false;
  }
  float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,距离npc过远" << XEND;
    return false;
  }

  // get item
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  BasePackage* pStorePack = getPackage(EPACKTYPE_STORE);
  if (pMainPack == nullptr || pStorePack == nullptr)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,未找到背包" << XEND;
    return false;
  }

  ItemBase* pBase = pMainPack->getItem(guid);
  if (pBase == nullptr)// || pBase->getType() == EITEMTYPE_FUNCTION)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,function道具不允许存入仓库" << XEND;
    return false;
  }
  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr || pCFG->isNoStore(ENOSTORE_STORE) == true)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:该物品不能存通用仓库");
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,该道具不允许存入仓库" << XEND;
    return false;
  }
  pBase->setNew(false);

  ItemData oData;
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
  {
    pBase->toItemData(&oData);

    dwCount = dwCount == 0 ? pBase->getCount() : dwCount;
    oData.mutable_base()->set_count(dwCount);

    if (dwCount > pBase->getCount())
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "存入" << guid << "失败" << guid << "count :" << pBase->getCount() << "不足" << dwCount << XEND;
      MsgManager::sendDebugMsg(m_pUser->id, "测试log : 数量不足");
      return false;
    }
    if (pStorePack->checkAddItem(oData, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_STORE_PUT_FULL);
      return false;
    }
    if (pMainPack->checkItemCount(guid, dwCount) == false)
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败" << guid << "不存在" << XEND;
      return false;
    }
    ItemEgg* pEgg = dynamic_cast<ItemEgg*>(pBase);
    if (pEgg != nullptr)
    {
      for (int i = 0; i < oData.egg().equips_size(); ++i)
      {
        const EggEquip& rEquip = oData.egg().equips(i);
        ItemData oEquip;
        oEquip.mutable_base()->CopyFrom(rEquip.base());
        oEquip.mutable_equip()->CopyFrom(rEquip.data());
        pMainPack->addItem(oEquip, EPACKMETHOD_NOCHECK);
        XLOG << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "是宠物蛋,脱下装备" << oEquip.ShortDebugString() << XEND;
      }
      oData.mutable_egg()->clear_equips();
      //oData.mutable_egg()->clear_name();
      oData.mutable_egg()->set_skilloff(false);
    }

    pMainPack->reduceItem(guid, ESOURCE_PUBLIC_PUTSTORE, dwCount);
    oData.mutable_base()->set_source(ESOURCE_PUBLIC_PUTSTORE);
    pStorePack->addItem(oData, EPACKMETHOD_CHECK_WITHPILE, false, false);
  }
  else
  {
    const TMapEquipCard& mapCard = pEquip->getCardList();
    for (auto &m : mapCard)
    {
      const SItemCFG* pCardCFG = ItemConfig::getMe().getItemCFG(m.second.second);
      if (pCardCFG == nullptr)
      {
        XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "存入" << guid << "失败,装备卡片" << m.second.second << "未在 Table_Item.txt 表中找到" << XEND;
        return false;
      }
      if (pCardCFG->isNoStore(ENOSTORE_STORE) == true)
      {
        MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_NOSTOREAGE_WITHCARD, MsgParams(pCardCFG->strNameZh));
        return false;
      }
    }

    if (pEquip->getStrengthLv() != 0)
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,强化过的装备无法存入" << XEND;
      return false;
    }
    if (pStorePack->checkAddItemObj(pBase) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_STORE_PUT_FULL);
      return false;
    }

    pBase->toItemData(&oData);

    pMainPack->reduceItem(guid, ESOURCE_PUBLIC_PUTSTORE, pBase->getCount(), false);
    pStorePack->addItemObj(pBase, false, ESOURCE_PUBLIC_PUTSTORE);
    pBase->setIndex(pStorePack->getNextIndex());
  }

  /*const TSetString& setIDs = pStorePack->getUpdateIDs();
  for (auto &s : setIDs)
  {
    ItemBase* pBase = pStorePack->getItem(s);
    if (pBase == nullptr)
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << s << "失败" << XEND;
      continue;
    }

    PutStoreRecordCmd cmd;
    cmd.set_accid(m_pUser->accid);
    cmd.set_charid(m_pUser->id);
    cmd.set_scenename(thisServer->getServerName());
    pBase->toItemData(cmd.mutable_data());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToRecord(send, len);

    XLOG << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << cmd.data().ShortDebugString() << "成功并发送到RecordServer处理" << XEND;
  }

  setStoreTick(now());
  m_pUser->saveDataNow();*/
  XLOG << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << oData.ShortDebugString() << "成功" << XEND;
  return true;
}

bool Package::offstore(const string& guid, DWORD dwCount)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-仓库]" << "取出" << guid << "失败,未找到玩家" << XEND;
    return false;
  }
  if (m_bStoreOpened == false)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,未正确打开仓库" << XEND;
    return false;
  }
  /*if (canStoreOpen() == false)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,仓库操作中" << XEND;
    return false;
  }*/

  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  if (m_pUser->getLevel() < rCFG.dwStoreTakeBaseLvReq && m_pUser->getDeposit().hasMonthCard() == false)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,未达到等级需求" << XEND;
    return false;
  }

  // check npc
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,未通过npc取出" << XEND;
    return false;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,和npc不在同一场景" << XEND;
    return false;
  }
  float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,距离npc过远" << XEND;
    return false;
  }

  // get item
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  StorePackage* pStorePack = dynamic_cast<StorePackage*>(getPackage(EPACKTYPE_STORE));
  if (pMainPack == nullptr || pStorePack == nullptr)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,获取" << EPACKTYPE_STORE << "失败" << XEND;
    return false;
  }

  ItemBase* pBase = pStorePack->getItem(guid);
  if (pBase == nullptr)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未找到" << guid << "物品" << XEND;
    return false;
  }
  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr)
  {
    XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "物品失败,该道具未包含正确的配置" << XEND;
    return false;
  }

  const SFoodMiscCFG& rFoodCFG = MiscConfig::getMe().getFoodCfg();
  if (ItemConfig::getMe().isQuest(pBase->getType()) == true)
  {
    pMainPack = getPackage(EPACKTYPE_QUEST);
    if (pMainPack == nullptr)
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "道具" << guid << "为任务道具,切换为任务包裹失败,未找到" << XEND;
      return false;
    }
    XLOG << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "道具" << guid << "为任务道具,切换为任务包裹" << XEND;
  }
  else if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == true && rFoodCFG.isFoodItem(pCFG->eItemType) == true)
  {
    pMainPack = getPackage(EPACKTYPE_FOOD);
    if (pMainPack == nullptr)
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "道具" << guid << "为料理道具,切换为" << EPACKTYPE_FOOD << "失败,未找到" << XEND;
      return false;
    }
    XLOG << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "道具" << guid << "为料理道具,切换为" << EPACKTYPE_FOOD << XEND;
  }
  else if (pCFG->eItemType == EITEMTYPE_EGG)
  {
    pMainPack = getPackage(EPACKTYPE_PET);
    if (pMainPack == nullptr)
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "道具" << guid << "为宠物道具,切换为" << EPACKTYPE_PET << "失败,未找到" << XEND;
      return false;
    }
    XLOG << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "道具" << guid << "为料理道具,切换为" << EPACKTYPE_PET << XEND;
  }

  /*OffStoreRecordCmd cmd;
  cmd.set_accid(m_pUser->accid);
  cmd.set_charid(m_pUser->id);
  cmd.set_scenename(thisServer->getServerName());*/
  ItemData oData;

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
  {
    dwCount = dwCount == 0 ? pBase->getCount() : dwCount;

    if (dwCount > pBase->getCount())
    {
      XERR << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败, count :" << dwCount << "不足" << pBase->getCount() << XEND;
      return false;
    }

    /*pBase->toItemData(cmd.mutable_data());
    cmd.mutable_data()->mutable_base()->set_count(dwCount);*/

    pBase->toItemData(&oData);
    oData.mutable_base()->set_count(dwCount);

    //if (pMainPack->checkAddItem(cmd.data().base(), EPACKMETHOD_CHECK_WITHPILE) == false)
    if (pMainPack->checkAddItem(oData, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    pStorePack->reduceItem(guid, ESOURCE_PUBLIC_OFFSTORE, dwCount, true, ECHECKMETHOD_NORMAL);
    oData.mutable_base()->set_source(ESOURCE_PUBLIC_OFFSTORE);
    pMainPack->addItem(oData, EPACKMETHOD_CHECK_WITHPILE);
  }
  else
  {
    if (pMainPack->checkAddItemObj(pBase) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    //pBase->toItemData(cmd.mutable_data());

    pBase->toItemData(&oData);
    pStorePack->reduceItem(guid, ESOURCE_PUBLIC_OFFSTORE, pBase->getCount(), false, ECHECKMETHOD_NORMAL);

    oData.mutable_base()->set_source(ESOURCE_PUBLIC_OFFSTORE);
    pMainPack->addItem(oData, EPACKMETHOD_NOCHECK);
  }

  /*PROTOBUF(cmd, send, len);
  thisServer->sendCmdToRecord(send, len);
  setStoreTick(now());
  m_pUser->saveDataNow();*/
  XLOG << "[包裹-仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << oData.ShortDebugString() << "成功" << XEND;
  return true;
}

bool Package::putPStore(const string& guid, DWORD dwCount)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-个人仓库]" << "存入" << guid << "未找到玩家" << XEND;
    return false;
  }
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  BasePackage* pStorePack = getPackage(EPACKTYPE_PERSONAL_STORE);
  if (pMainPack == nullptr || pStorePack == nullptr)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "获取背包失败" << XEND;
    return false;
  }

  //DWORD dwPutCount = 0;
  ItemBase* pBase = pMainPack->getItem(guid);
  if (pBase == nullptr)// || pBase->getType() == EITEMTYPE_FUNCTION)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,function物品不允许存入仓库" << XEND;
    return false;
  }
  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr || pCFG->isNoStore(ENOSTORE_PSTORE_BARROW) == true)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:该物品不能存仓库");
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,该物品不允许存入仓库" << XEND;
    return false;
  }
  pBase->setNew(false);

  // check npc
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,未通过npc存入" << XEND;
    return false;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,和npc不在同一场景" << XEND;
    return false;
  }
  float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,距离npc过远" << XEND;
    return false;
  }

  dwCount = dwCount == 0 ? pBase->getCount() : dwCount;

  ItemData oData;
  pBase->toItemData(&oData);
  oData.mutable_base()->clear_guid();
  oData.mutable_base()->set_count(dwCount);
  oData.mutable_base()->set_source(ESOURCE_PERSON_PUTSTORE);

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
  {
    if (dwCount > pBase->getCount())
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log : 数量不足");
      XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败count" << pBase->getCount() << "不足" << dwCount << XEND;
      return false;
    }

    if (pStorePack->checkAddItem(oData, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_STORE_PUT_FULL);
      return false;
    }

    pMainPack->reduceItem(guid, ESOURCE_PERSON_PUTSTORE, dwCount);
    pStorePack->addItem(oData, EPACKMETHOD_CHECK_WITHPILE);
  }
  else
  {
    if (pStorePack->checkAddItemObj(pBase) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_STORE_PUT_FULL);
      return false;
    }

    pMainPack->reduceItem(guid, ESOURCE_PERSON_PUTSTORE, pBase->getCount(), false);
    pStorePack->addItemObj(pBase, false, ESOURCE_PERSON_PUTSTORE);
    pBase->setIndex(pStorePack->getNextIndex());
  }

  XLOG << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << oData.ShortDebugString() << "成功" << XEND;
  return true;
}

bool Package::offPStore(const string& guid, DWORD dwCount)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-个人仓库]" << "取出" << guid << "未找到玩家" << XEND;
    return false;
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  BasePackage* pStorePack = getPackage(EPACKTYPE_PERSONAL_STORE);
  if (pMainPack == nullptr || pStorePack == nullptr)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "获取背包失败" << XEND;
    return false;
  }

  ItemBase* pBase = pStorePack->getItem(guid);
  if (pBase == nullptr)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未找到" << guid << "物品" << XEND;
    return false;
  }

  // check npc
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,未通过npc取出" << XEND;
    return false;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,和npc不在同一场景" << XEND;
    return false;
  }
  float fDist = ::getXZDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,距离npc过远" << XEND;
    return false;
  }

  DWORD dwTypeID = pBase->getTypeID();
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
  {
    dwCount = dwCount == 0 ? pBase->getCount() : dwCount;
    if (dwCount > pBase->getCount())
    {
      XERR << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "取出" << guid << "失败,count :" << dwCount << "不足" << pBase->getCount() << XEND;
      return false;
    }

    ItemData oData;
    pBase->toItemData(&oData);
    oData.mutable_base()->clear_guid();
    oData.mutable_base()->set_count(dwCount);
    oData.mutable_base()->set_source(ESOURCE_PERSON_OFFSTORE);

    if (pMainPack->checkAddItem(oData, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_STORE_OFF_FULL);
      return false;
    }

    pStorePack->reduceItem(guid, ESOURCE_PERSON_OFFSTORE, dwCount);
    pMainPack->addItem(oData, EPACKMETHOD_CHECK_WITHPILE);
  }
  else
  {
    if (pMainPack->checkAddItemObj(pBase) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_STORE_OFF_FULL);
      return false;
    }

    pStorePack->reduceItem(guid, ESOURCE_PERSON_OFFSTORE, pBase->getCount(), false);
    pMainPack->addItemObj(pBase, false, ESOURCE_PERSON_OFFSTORE);
  }

  XLOG << "[包裹-个人仓库]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "成功从仓库取出" << guid << dwTypeID << dwCount << "物品" << XEND;
  return true;
}

bool Package::offTemp(const string& guid)
{
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  TempMainPackage* pTempPack = dynamic_cast<TempMainPackage*>(getPackage(EPACKTYPE_TEMP_MAIN));
  if (pMainPack == nullptr || pTempPack == nullptr)
  {
    XERR << "[包裹-临时背包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << (guid.empty() ? "全部" : guid) << "失败,未找到包裹" << XEND;
    return false;
  }

  pTempPack->refreshDeleteTimeItems();

  TSetString setIDs;
  if (guid.empty() == true)
  {
    for (auto &m : pTempPack->m_mapID2Item)
      setIDs.insert(m.first);
  }
  else
  {
    setIDs.insert(guid);
  }
  if (setIDs.empty() == true)
  {
    XERR << "[包裹-临时背包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << (guid.empty() ? "全部" : guid) << "失败,未发现物品" << XEND;
    return false;
  }

  for (auto &s : setIDs)
  {
    ItemBase* pBase = pTempPack->getItem(s);
    if (pBase == nullptr)
    {
      XERR << "[包裹-临时背包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "取出" << (guid.empty() ? "全部" : guid) << "失败,未找到" << s << "物品" << XEND;
      return false;
    }
  }
  for (auto &s : setIDs)
  {
    ItemBase* pBase = pTempPack->getItem(s);
    if (pBase == nullptr)
    {
      XERR << "[包裹-临时背包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "取出" << (guid.empty() ? "全部" : guid) << "失败,未找到" << s << "物品" << XEND;
      continue;
    }

    bool bSuccess = false;
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip == nullptr)
    {
      ItemData oData;
      pBase->toItemData(&oData);
      oData.mutable_base()->set_source(ESOURCE_OFF_TEMPPACK);
      DWORD dwOriCount = oData.base().count();
      pMainPack->setLogSrc(ESOURCE_OFF_TEMPPACK);
      pMainPack->addItem(oData, EPACKMETHOD_AVAILABLE);
      if (dwOriCount > oData.base().count())
      {
        pTempPack->reduceItem(s, ESOURCE_OFF_TEMPPACK, dwOriCount - oData.base().count());
        bSuccess = true;
      }
      pMainPack->clearLogSrc();
    }
    else
    {
      if (pMainPack->checkAddItemObj(pEquip) == true)
      {
        pMainPack->addItemObj(pEquip, false, ESOURCE_OFF_TEMPPACK);  
        pTempPack->reduceItem(pEquip->getGUID(), ESOURCE_OFF_TEMPPACK, pEquip->getCount(), false);
        bSuccess = true;
      }
    }

    if (bSuccess)
    {
      XLOG << "[包裹-临时背包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "取出" << (guid.empty() ? "全部" : guid) << "成功,成功取出" << s << "物品" << XEND;
    }
    else
    {
      XERR << "[包裹-临时背包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "取出" << (guid.empty() ? "全部" : guid) << "失败,无法取出" << s << "物品" << XEND;
    }
  }

  return true;
}

bool Package::putBarrow(const string& guid, DWORD dwCount)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-手推车]" << "存入" << guid << "未找到玩家" << XEND;
    return false;
  }
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  BasePackage* pBarrowPack = getPackage(EPACKTYPE_BARROW);
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pMainPack == nullptr || pBarrowPack == nullptr || pEquipPack == nullptr)
  {
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "获取背包失败" << XEND;
    return false;
  }

  ItemEquip* pBarrow = pEquipPack->getEquip(EEQUIPPOS_BARROW);
  if (pBarrow == nullptr || pBarrow->getCFG() == nullptr || pBarrow->getCFG()->eItemType != EITEMTYPE_BARROW)
  {
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,未装备手推车" << XEND;
    return false;
  }

  ItemBase* pBase = pMainPack->getItem(guid);
  if (pBase == nullptr)// || pBase->getType() == EITEMTYPE_FUNCTION)
  {
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,未发现该道具" << XEND;
    return false;
  }
  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr || pCFG->isNoStore(ENOSTORE_PSTORE_BARROW) == true)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:该物品不能存手推车");
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,该物品不允许存入手推车" << XEND;
    return false;
  }
  pBase->setNew(false);

  ItemData oData;
  pBase->toItemData(&oData);

  dwCount = dwCount == 0 ? pBase->getCount() : dwCount;
  oData.mutable_base()->set_count(dwCount);
  oData.mutable_base()->set_source(ESOURCE_PUT_BARROW);

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
  {
    if (dwCount > pBase->getCount())
    {
      XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << guid << "失败,count :" << pBase->getCount() << "不足" << dwCount << XEND;
      MsgManager::sendDebugMsg(m_pUser->id, "测试log : 数量不足");
      return false;
    }
    oData.mutable_base()->clear_guid();
    if (pBarrowPack->checkAddItem(oData, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    pMainPack->reduceItem(guid, ESOURCE_PUT_BARROW, dwCount);
    ItemData oCopy = oData;
    pBarrowPack->addItem(oCopy, EPACKMETHOD_CHECK_WITHPILE);
  }
  else
  {
    if (pBarrowPack->checkAddItemObj(pBase) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    pMainPack->reduceItem(guid, ESOURCE_PUT_BARROW, pBase->getCount(), false);
    pBarrowPack->addItemObj(pBase, false, ESOURCE_PUT_BARROW);
    pBase->setIndex(pBarrowPack->getNextIndex());
  }

  XLOG << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << oData.ShortDebugString() << "成功" << XEND;
  return true;
}

bool Package::offBarrow(const string& guid, DWORD dwCount)
{
  if (m_pUser == nullptr)
  {
    XERR << "[包裹-手推车]" << "取出" << guid << "未找到玩家" << XEND;
    return false;
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  BasePackage* pBarrowPack = getPackage(EPACKTYPE_BARROW);
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pMainPack == nullptr || pBarrowPack == nullptr || pEquipPack == nullptr)
  {
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "获取背包失败" << XEND;
    return false;
  }

  ItemEquip* pBarrow = pEquipPack->getEquip(EEQUIPPOS_BARROW);
  if (pBarrow == nullptr || pBarrow->getCFG() == nullptr || pBarrow->getCFG()->eItemType != EITEMTYPE_BARROW)
  {
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败,未装备手推车" << XEND;
    return false;
  }

  ItemBase* pBase = pBarrowPack->getItem(guid);
  if (pBase == nullptr)
  {
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "未找到" << guid << "物品" << XEND;
    return false;
  }

  if (dwCount > pBase->getCount())
  {
    XERR << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "取出" << guid << "失败, count :" << dwCount << "不足" << pBase->getCount() << XEND;
    return false;
  }

  dwCount = dwCount == 0 ? pBase->getCount() : dwCount;

  ItemData oData;
  pBase->toItemData(&oData);
  oData.mutable_base()->set_count(dwCount);
  oData.mutable_base()->set_source(ESOURCE_OFF_BARROW);

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
  {
    oData.mutable_base()->clear_guid();
    if (pMainPack->checkAddItem(oData, EPACKMETHOD_CHECK_WITHPILE) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    pBarrowPack->reduceItem(guid, ESOURCE_OFF_BARROW, dwCount);

    ItemData oCopy;
    oCopy.CopyFrom(oData);
    pMainPack->addItem(oCopy, EPACKMETHOD_CHECK_WITHPILE, false, false);
  }
  else
  {
    if (pMainPack->checkAddItemObj(pBase) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    pBarrowPack->reduceItem(guid, ESOURCE_OFF_BARROW, pBase->getCount(), false);
    pMainPack->addItemObj(pBase, false, ESOURCE_OFF_BARROW);
  }

  XLOG << "[包裹-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "成功从" << EPACKTYPE_BARROW << "取出" << oData.ShortDebugString() << "物品" << XEND;
  return true;
}

bool Package::equipDressUpOn(EEquipPos ePos, const string& guid)
{
  if(m_pUser->getDressUp().getDressUpStatus() == EDRESSUP_MIN)
    return false;

  DWORD dwItemID = atoi(guid.c_str());
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
  if(pCFG == nullptr)
    return false;

  if(pCFG->isRightJob(m_pUser->getProfession()) == false || pCFG->isRightJobForUse(m_pUser->getProfession()) == false)
      return false;

  ePos = MiscConfig::getMe().getItemCFG().getValidEquipPos(ePos, pCFG->eEquipType);
  if (ePos == EEQUIPPOS_MIN)
    return false;

  if(pCFG->eSexEquip != EGENDER_MIN && pCFG->eSexEquip != m_pUser->getUserSceneData().getGender())
    return false;

  bool bDressEquip = m_pUser->getDressUp().addDressUpEquipID(ePos, dwItemID);
  if(bDressEquip == false)
    return false;

  if (pCFG->eEquipType == EEQUIPTYPE_WEAPON || pCFG->eEquipType == EEQUIPTYPE_ARTIFACT)
  {
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
    m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
  }
  else if (pCFG->eEquipType == EEQUIPTYPE_SHIELD)
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
  else if (pCFG->eEquipType == EEQUIPTYPE_BACK || pCFG->eEquipType == EEQUIPTYPE_ARTIFACT_BACK)
    m_pUser->setDataMark(EUSERDATATYPE_BACK);
  else if (pCFG->eEquipType == EEQUIPTYPE_HEAD || pCFG->eEquipType == EEQUIPTYPE_ARTIFACT_HEAD)
    m_pUser->setDataMark(EUSERDATATYPE_HEAD);
  else if (pCFG->eEquipType == EEQUIPTYPE_FACE)
    m_pUser->setDataMark(EUSERDATATYPE_FACE);
  else if (pCFG->eEquipType == EEQUIPTYPE_TAIL)
    m_pUser->setDataMark(EUSERDATATYPE_TAIL);
  else if (pCFG->eEquipType == EEQUIPTYPE_MOUNT)
    m_pUser->setDataMark(EUSERDATATYPE_MOUNT);
  else if (pCFG->eEquipType == EEQUIPTYPE_MOUTH)
    m_pUser->setDataMark(EUSERDATATYPE_MOUTH);
  else if (pCFG->eEquipType == EEQUIPTYPE_ARMOUR)
    m_pUser->setDataMark(EUSERDATATYPE_BODY);

  m_pUser->refreshDataAtonce();
  XLOG << "[包裹-换装舞台] 换装" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "成功穿上: " << dwItemID << ePos << XEND;
  return true;
}

bool Package::equipDressUpOff(EEquipPos ePos, const string& guid)
{
  if(m_pUser->getDressUp().getDressUpStatus() == EDRESSUP_MIN)
    return false;

  DWORD dwItemID = atoi(guid.c_str());
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(dwItemID);
  if(pCFG == nullptr)
    return false;

  ePos = MiscConfig::getMe().getItemCFG().getValidEquipPos(ePos, pCFG->eEquipType);
  if (ePos == EEQUIPPOS_MIN)
    return false;

  m_pUser->getDressUp().addDressUpEquipID(ePos, 0);

  if (pCFG->eEquipType == EEQUIPTYPE_WEAPON || pCFG->eEquipType == EEQUIPTYPE_ARTIFACT)
  {
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
    m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
  }
  else if (pCFG->eEquipType == EEQUIPTYPE_SHIELD)
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
  else if (pCFG->eEquipType == EEQUIPTYPE_BACK || pCFG->eEquipType == EEQUIPTYPE_ARTIFACT_BACK)
    m_pUser->setDataMark(EUSERDATATYPE_BACK);
  else if (pCFG->eEquipType == EEQUIPTYPE_HEAD || pCFG->eEquipType == EEQUIPTYPE_ARTIFACT_HEAD)
    m_pUser->setDataMark(EUSERDATATYPE_HEAD);
  else if (pCFG->eEquipType == EEQUIPTYPE_FACE)
    m_pUser->setDataMark(EUSERDATATYPE_FACE);
  else if (pCFG->eEquipType == EEQUIPTYPE_TAIL)
    m_pUser->setDataMark(EUSERDATATYPE_TAIL);
  else if (pCFG->eEquipType == EEQUIPTYPE_MOUNT)
    m_pUser->setDataMark(EUSERDATATYPE_MOUNT);
  else if (pCFG->eEquipType == EEQUIPTYPE_MOUTH)
    m_pUser->setDataMark(EUSERDATATYPE_MOUTH);
  else if (pCFG->eEquipType == EEQUIPTYPE_ARMOUR)
    m_pUser->setDataMark(EUSERDATATYPE_BODY);

  m_pUser->refreshDataAtonce();
  XLOG << "[包裹-换装舞台] 脱下" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "成功脱下: " << dwItemID << ePos << XEND;
  return true;
}

bool Package::cardOn(const string& cardguid, const string& equipguid, DWORD addpos)
{
  if (addpos == 0)
    return false;
  // check card
  bool isEquiped = false;
  BasePackage* pPackage = getPackage(EPACKTYPE_MAIN);
  BasePackage* pEquipPack = getPackage(EPACKTYPE_MAIN);
  if (pPackage == nullptr)
    return false;
  if (pEquipPack == nullptr)
    return false;
  ItemCard* pCard = dynamic_cast<ItemCard*>(pPackage->getItem(cardguid));
  if (pCard == nullptr)
    return false;
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(equipguid));
  if (pEquip == nullptr)
  {
    pEquipPack = getPackage(EPACKTYPE_EQUIP);
    if (pEquipPack!= nullptr)
    {
      pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(equipguid));
      if (pEquip != nullptr)
        isEquiped = true;
    }
    if (pEquip == nullptr)
    {
      pEquipPack = getPackage(EPACKTYPE_FASHIONEQUIP);
      if (pEquipPack != nullptr)
        pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(equipguid));
    }
  }
  if (pEquip == nullptr || pEquip->hasCard(pCard) == true)
    return false;

  // 检查pos上是否有卡
  const TPairEquipCard* pPreCard = pEquip->getCardByPos(addpos);
  if (pPreCard == nullptr)
  {
    if (pEquip->addCard(pCard, addpos) == false)
      return false;
  }
  else
  {
    const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
    // 检查zeny是否足够
    const SItemCFG *pPreCardCfg = ItemManager::getMe().getItemCFG(pPreCard->second);
    if (pPreCardCfg == nullptr)
    {
      XERR << "[包裹-插卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "card:" << pCard->getTypeID() << "equip:" << pEquip->getTypeID() << "pos:" << addpos
           << "原始卡片配置未找到, id:" << pPreCard->second << XEND;
      return false;
    }
    DWORD zeny = rCFG.getCardRestoreCost(pPreCardCfg->eQualityType);
    if (m_pUser->checkMoney(EMONEYTYPE_SILVER, zeny) == false)
    {
      XERR << "[包裹-插卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "card:" << pCard->getTypeID() << "equip:" << pEquip->getTypeID() << "pos:" << addpos
           << "zeny不够, 需要:" << zeny << XEND;
      return false;
    }

    // 检查包裹是否已满
    ItemInfo oCard;
    oCard.set_id(pPreCardCfg->dwTypeID);
    oCard.set_count(1);
    if (pPackage->checkAddItem(oCard) == false)
    {
      XERR << "[包裹-插卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "card:" << pCard->getTypeID() << "equip:" << pEquip->getTypeID() << "pos:" << addpos
           << "包裹已满" << XEND;
      MsgManager::sendMsg(m_pUser->id, 989);
      return false;
    }

    // 检查是否可以添加卡片
    if (pEquip->canAddCard(pCard, addpos, false) == false)
    {
      XERR << "[包裹-插卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "card:" << pCard->getTypeID() << "equip:" << pEquip->getTypeID() << "pos:" << addpos
           << "无法插卡" << XEND;
      return false;
    }

    // 从装备上移除卡片, 添加到包裹
    string precardguid = pPreCard->first;
    if (cardOff(precardguid, equipguid, addpos) == false)
    {
      XERR << "[包裹-插卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "card:" << pCard->getTypeID() << "equip:" << pEquip->getTypeID() << "pos:" << addpos
           << "移除原始卡片失败" << XEND;
      return false;
    }
    // m_pUser->getEvent().onCardChange(precardguid, equipguid);

    // 扣除zeny
    m_pUser->subMoney(EMONEYTYPE_SILVER, zeny, ESOURCE_CARD);

    // 将卡片添加到装备
    if (pEquip->addCard(pCard, addpos) == false)
    {
      XERR << "[包裹-插卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "card:" << pCard->getTypeID() << "equip:" << pEquip->getTypeID() << "pos:" << addpos
           << "插卡失败" << XEND;
      return false;
    }
  }

  if (isEquiped)
    m_pUser->getEvent().onCardChange(pCard->getTypeID(), true);

  //log card
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_CardOn;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  PlatLogManager::getMe().CardLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,/*eid*/
    pEquip->getTypeID(), /*equid id*/
    pEquip->getGUID(),
    1,  /*on*/
    pCard->getTypeID(), /*card id*/
    pCard->getGUID(),
    pEquip->getCardList().size(), /*user slot*/
    pEquip->getCardSlot() /*max slot*/
  );

  // save data
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);

  XLOG << "[包裹-插卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "card:" << pCard->getTypeID() << "equip:" << pEquip->getTypeID() << "pos:" << addpos << XEND;

  pPackage->reduceItem(pCard->getGUID(),ESOURCE_CARD);
  pEquipPack->setUpdateIDs(pEquip->getGUID());

  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_CARD_MOSAIC);

  return true;
}

// removepos表示移除指定卡槽的卡片, 为0则按cardguid移除, cardguid和removepos同时只有一个生效
bool Package::cardOff(const string& cardguid, const string& equipguid, DWORD removepos, bool bDelCard/* = false*/)
{
  // check equip
  BasePackage* pMainPackage = getPackage(EPACKTYPE_MAIN);
  if (pMainPackage == nullptr)
    return false;
  BasePackage* pPack = nullptr;
  ItemEquip* pEquip = nullptr;
  for (DWORD d = EPACKTYPE_MIN + 1; d < EPACKTYPE_MAX; ++d)
  {
    if (d == EPACKTYPE_STORE)
      continue;

    pPack = m_pUser->getPackage().getPackage(static_cast<EPackType>(d));
    if (pPack == nullptr)
      continue;

    pEquip = dynamic_cast<ItemEquip*>(pPack->getItem(equipguid));
    if (pEquip != nullptr)
      break;
  }
  if (pPack == nullptr || pEquip == nullptr)
  {
    XERR << "[包裹-拆卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << equipguid << "拆除card:" << cardguid << "失败,未找到该装备" << XEND;
    return false;
  }
  const TPairEquipCard* pairCard = nullptr;
  if (removepos != 0)
    pairCard = pEquip->getCardByPos(removepos);
  else
    pairCard = pEquip->getCardAndPos(cardguid, removepos);
  if (pairCard == nullptr)
    return false;

  string strCardGuid = cardguid;
  DWORD dwCardID = pairCard->second;
  if (bDelCard)
  {
    pEquip->removeCardByPos(removepos);
  }
  else
  {
    ItemInfo oItem;
    oItem.set_id(dwCardID);
    oItem.set_count(1);
    oItem.set_source(ESOURCE_CARD);
    /*if (pMainPackage->checkAddItem(oItem, EPACKMETHOD_CHECK_WITHPILE) == false)
      return false;*/

    pEquip->removeCardByPos(removepos);
    oItem.set_guid(strCardGuid);
    pMainPackage->addItem(static_cast<const ItemInfo>(oItem), EPACKMETHOD_NOCHECK, false, false);

    MsgParams params;
    params.addNumber(oItem.id());
    params.addNumber(oItem.id());
    params.addNumber(oItem.count());
    MsgManager::sendMsg(m_pUser->id, 6, params);
  }

  if (pPack->getPackageType() == EPACKTYPE_EQUIP || pPack->getPackageType() == EPACKTYPE_FASHIONEQUIP)
    m_pUser->getEvent().onCardChange(dwCardID, false);

  pPack->setUpdateIDs(pEquip->getGUID());

  // save data
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);

  // send tip msg, id = 508
  MsgManager::sendMsg(m_pUser->id, 508);
  XLOG << "[包裹-拆卡]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << pPack->getPackageType() << "中装备" << equipguid << "拆除card:" << cardguid << "成功" << XEND;

  //log card
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_CardOff;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  PlatLogManager::getMe().CardLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,/*eid*/
    pEquip->getTypeID(), /*equid id*/
    pEquip->getGUID(),
    2,  /*off*/
    dwCardID, /*card id*/
    strCardGuid,
    pEquip->getCardList().size(), /*user slot*/
    pEquip->getCardSlot() /*max slot*/
  );


  return true;
}

bool Package::transfer(ItemEquip* pFrom, ItemEquip* pTo)
{
  if (pFrom == nullptr || pTo == nullptr || pTo->getCFG() == nullptr)
    return false;

  // check strength level
  if (!(pFrom->getStrengthLv() != 0 && pTo->getStrengthLv() == 0))
    return true;

  DWORD dwCost = pFrom->getStrengthCost();
  DWORD dwStrengthLv = pFrom->getStrengthLv();

  pFrom->setStrengthLv(0);
  pFrom->resetStrengthCost();

  // transfer strength level
  while (true)
  {
    if (pTo->getStrengthLv() >= dwStrengthLv)
      break;

    DWORD price = LuaManager::getMe().call<DWORD>("calcEquipStrengthCost", pTo->getStrengthLv(), pTo->getQuality(), pTo->getType());
    if (price > dwCost)
      break;
    dwCost -= price;

    pTo->setStrengthLv(pTo->getStrengthLv() + 1);
    pTo->addStrengthCost(price);
  }
  m_pUser->addMoney(EMONEYTYPE_SILVER, dwCost, ESOURCE_TRANSFER);
  XLOG << "[包裹-装备转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "装备" << pFrom->getGUID() << "和" << pTo->getGUID() << "发生转移 强化等" << pTo->getStrengthLv() << "返回了" << dwCost << "银币" << XEND;

  // transfer card : 虞佳2016-12-08移除卡片继承,配置装备还原
  /*TVecEquipCard vecCards = pFrom->getCardList();
  for (auto v = vecCards.begin(); v != vecCards.end(); ++v)
  {
    const SItemCFG* pCardCFG = ItemConfig::getMe().getItemCFG(v->second);
    if (pCardCFG == nullptr)
    {
      XERR << "[包裹-装备转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "装备" << pFrom->getGUID() << "卡片" << v->second << "未在 Table_Item.txt 表中找到 不做任何操作" << XEND;
      continue;
    }

    if (pFrom->removeCard(v->first) == false)
    {
      XERR << "[包裹-装备转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "装备" << pFrom->getGUID() << "卡片" << v->second << "无法卸下" << XEND;
      continue;
    }
    //m_pUser->getEvent().onCardChange(v->first, pFrom->getGUID());

    ItemCard oCard(pCardCFG);
    oCard.setGUID(v->first);
    oCard.init();
    if (pTo->addCard(&oCard) == true)
    {
      //m_pUser->getEvent().onCardChange(v->first, pTo->getGUID());

      XLOG <<"[包裹-装备转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "装备" << pFrom->getGUID() << "发生卡片" << pCardCFG->dwTypeID << "切换到了装备" << pTo->getGUID() << "上" << XEND;
      continue;
    }

    ItemInfo oItem;
    oItem.set_id(pCardCFG->dwTypeID);
    oItem.set_count(1);
    oItem.set_source(ESOURCE_TRANSFER);
    m_pUser->getPackage().addItemAvailable(oItem);
    XLOG << "[包裹-装备转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "装备" << pFrom->getGUID() << "发生卡片" << pCardCFG->dwTypeID << "进入包裹" << XEND;
  }*/

  if (dwCost > 0)
    MsgManager::sendMsg(m_pUser->id, 1702, MsgParams(dwCost));
  else
    MsgManager::sendMsg(m_pUser->id, 1703, MsgParams(dwCost));

  return true;
}

void Package::updateFashionPackage()
{
  if (m_vecItemChangeIDs.empty() == true)
    return;

  /* 移除fashpackage - 20160425
  FashionPackage* pFashionPack = dynamic_cast<FashionPackage*>(getPackage(EPACKTYPE_FASHION));
  if (pFashionPack == nullptr)
    return;

  for (auto v = m_vecItemChangeIDs.begin(); v != m_vecItemChangeIDs.end(); ++v)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(*v);
    if (pCFG == nullptr || ItemManager::getMe().isFashion(pCFG->eEquipType) == false)
      continue;

    DWORD count = getItemCount(*v);
    if (count == 0)
    {
      if (pFashionPack->checkFashion(*v) == true)
        pFashionPack->decFashion(*v);
    }
    else
    {
      if (pFashionPack->checkFashion(*v) == false)
        pFashionPack->addFashion(*v);
    }
  }
  */

  m_vecItemChangeIDs.clear();
}

void Package::checkInvalidEquip(bool bNotify/*=false*/)
{
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*> (getPackage(EPACKTYPE_EQUIP));
  if (pMainPack == nullptr || pEquipPack == nullptr)
  {
    XERR << "[包裹-装备核查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "检查失败,未找到对应包裹" << XEND;
    return;
  }
  TSetString setInvalidID;
  for (auto &m : pEquipPack->m_mapID2Item)
  {
    bool bInvalid = false;

    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m.second);
    if (pEquip == nullptr)
      continue;

    if (!bInvalid)
    {
      if (pEquip == nullptr)
        bInvalid = true;
    }
    if (!bInvalid)
    {
      EEquipPos ePos = MiscConfig::getMe().getItemCFG().getValidEquipPos(static_cast<EEquipPos>(m.second->getIndex()), pEquip->getEquipType());
      if (ePos != static_cast<EEquipPos>(m.second->getIndex()))
        bInvalid = true;
    }
    if (!bInvalid)
    {
      if (pEquipPack->canEquip(m_pUser, m.second) == false)
        setInvalidID.insert(m.first);
    }

    if (bInvalid)
      setInvalidID.insert(m.first);
  }

  for (auto &s : setInvalidID)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(s));
    if (pEquip == nullptr)
      continue;

    if (equipOff(s, 0, false) == false)
    {
      XERR << "[包裹-装备核查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "检查到" << pEquip->getTypeID() << pEquip->getGUID() << "不是合法装备,卸载失败" << XEND;
    }
    else
    {
      XLOG << "[包裹-装备核查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "检查到" << pEquip->getTypeID() << pEquip->getGUID() << "不是合法装备,卸载成功" << XEND;
    }
  }

  if (!bNotify)
  {
    pMainPack->clearUpdateIDs();
    pEquipPack->clearUpdateIDs();
  }
}

bool Package::tradeAddItem(const TradeItemBaseInfo& tradeBaseInfo)
{
  if (tradeBaseInfo.has_item_data())
  {
    ItemData item = tradeBaseInfo.item_data();
    item.mutable_base()->set_source(ESOURCE_TRADE);
    if (item.base().guid().empty() == true)
    {
      XERR << "[交易-加装备] 增加装备失败" << item.base().guid() << "accid:" << m_pUser->accid << m_pUser->id << "guid is empty" << XEND;
      return false;
    }

    if (!addItem(item, EPACKMETHOD_NOCHECK))
    {
      XERR << "[交易-加装备] 增加装备失败,give up，accid:" << m_pUser->accid << m_pUser->id << tradeBaseInfo.ShortDebugString() << XEND;
      return false;
    }
  }
  else
  {//普通道具  不带属性的
    ItemInfo stInfo;
    stInfo.set_id(tradeBaseInfo.itemid());
    stInfo.set_count(tradeBaseInfo.count());
    stInfo.set_source(ESOURCE_TRADE);

    if (!addItem(stInfo, EPACKMETHOD_NOCHECK))
    {
      XERR << "[交易-加装备] 增加装备失败,give up，accid:" << m_pUser->accid << m_pUser->id <<tradeBaseInfo.ShortDebugString() << XEND;
      return false;
    }
  }  
  m_pUser->saveDataNow();
  return true;
}

bool Package::tradePreReduceItem(TradeItemBaseInfo* tradeBaseInfo, ESource eSource)
{
  if (!tradeBaseInfo)
    return false;

  const std::string& guid = tradeBaseInfo->guid();
  DWORD itemId = tradeBaseInfo->itemid();
  DWORD count = tradeBaseInfo->count();
  XLOG << "[交易-出售] user =" <<m_pUser->accid << m_pUser->id << "itemid =" << tradeBaseInfo->itemid() << "count =" << tradeBaseInfo->count() << XEND;

  const SExchangeItemCFG *pData = ItemConfig::getMe().getExchangeItemCFG(itemId);
  if (pData == nullptr)
  {
    XERR << "[交易-出售] 策划表配置的物品不可出售 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    MsgManager::sendMsg(m_pUser->id, 10000);
    return false;
  }

  // 上架时间交由交易所来判断
  /*if (!pData->canTrade())
  {
    XERR << "[交易-出售] 不可上架，未达到上架时间 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    MsgManager::sendMsg(m_pUser->id, 10000);
    return false;
  }*/

  // get package
  BasePackage* pPack = nullptr;
  // get item
  ItemBase* pItem = getTradeSellItem(guid, &pPack);
  if (pItem == nullptr || pPack == nullptr)
  {
    XERR << "[交易-出售] 背包里找不到该物品 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    MsgManager::sendMsg(m_pUser->id, 10107);
    return false;
  }

  /*BasePackage* pPack = nullptr;
  ItemBase* pItem = getItem(guid, &pPack);
  if (pPack == nullptr || pItem == nullptr)
  {
    XERR << "[交易-出售] 包裹和物品未找到 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    return false;
  }*/
  if (pItem->getTypeID() != itemId) {
    XERR << "[交易-出售] 数据异常 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "guid=" << guid << "typeid:" << pItem->getTypeID() << XEND;
    return false;
  }

  // check item count
  if (pPack->checkItemCount(guid, count) == false)
  {
    XERR << "[交易-出售][个数不足] user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    MsgManager::sendMsg(m_pUser->id, 10100);
    return false;
  }

  // check equip
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pItem);
  if (pEquip != nullptr)
  {
    const Cmd::EquipData& sellEquipData = tradeBaseInfo->item_data().equip();

    if (pEquip->getRefineLv() != sellEquipData.refinelv())
    {
      XERR << "[交易-出售]，失败，精炼等级不一致 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << "true refinelv =" << pEquip->getRefineLv() <<" client refinelv="<< sellEquipData.refinelv() << XEND;
      return false;
    }

    //check damage
    if (pEquip->isDamaged() != sellEquipData.damage())
    {
      XERR << "[交易-出售]，失败， 损坏属性不一样 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << "true damage=" << pEquip->isDamaged() << " client damage=" << sellEquipData.damage() << XEND;
      return false;
    }

    //check enchant
    if (!pEquip->checkSameEnchant(tradeBaseInfo->item_data().enchant()))
    {
      XERR << "[交易-出售]，失败， 附魔属性不一样 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << XEND;
      return false;
    }

    if (pEquip->getStrengthLv() != 0)
    {
      /*XERR("[交易-出售] 强化装备不可出售 user = %llu, itemid = %u count = %u", m_pUser->id, itemId, count);
      MsgManager::sendMsg(m_pUser->id, 10106);
      return false;
*/
      DWORD retMoney;
      if (!decompose(guid, false, retMoney))
      {
        XERR << "[交易-出售] 强化装备分解失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << "count =" <<  count << XEND;
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
      //XERR("[交易-出售] 插卡装备不可出售 user = %llu, itemid = %u count = %u", m_pUser->id, itemId, count);
      //MsgManager::sendMsg(m_pUser->id, 10105);
      //return false;    
      if (!equipAllCardOff(guid))
      {
        XERR << "[交易-出售] 装备插卡脱下失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << "count =" << count << XEND;
        return false;
      }
    }
 
    if (pEquip->getLv())
    {
      RestoreEquipItemCmd cmd;
      cmd.set_equipid(guid);
      cmd.set_upgrade(true);
      if (restore(cmd) == false)
      {
        XERR << "[交易-出售] 装备升级等级移除失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << "count =" << count << XEND;
        return false;
      }
    }
    if (pEquip->getStrengthLv2())
    {
      RestoreEquipItemCmd cmd;
      cmd.set_equipid(guid);
      cmd.set_strengthlv2(true);
      if (restore(cmd) == false)
      {
        XERR << "[交易-出售] 加固装备等级移除失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << "count =" << count << XEND;
        return false;
      }
    }

    tradeBaseInfo->set_refine_lv(pEquip->getRefineLv());
    pEquip->toItemData(tradeBaseInfo->mutable_item_data());
    tradeBaseInfo->mutable_item_data()->mutable_base()->set_source(eSource);
  }
  else
  {
    tradeBaseInfo->set_refine_lv(0);
  }

  // check pet egg can sell
  if (pItem->getType() == EITEMTYPE_EGG)
  {
    ItemEgg* pEgg = dynamic_cast<ItemEgg*> (pItem);
    if (pEgg == nullptr)
    {
      XERR << "[交易-出售], 道具类型不合法, 玩家:" << m_pUser->name << m_pUser->id << "道具:" << guid << XEND;
      return false;
    }
    if (pEgg->canSell() == false)
    {
      XERR << "[交易-出售], 宠物蛋, 技能未达满级, 不可出售, 玩家:" << m_pUser->name << m_pUser->id << "道具:" << guid << pItem->getTypeID() << XEND;
      return false;
    }
  }
  XLOG << "[交易-出售][出售成功，扣除预处理成功] user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
  return true;
}

bool Package::tradeReduceItem(TradeItemBaseInfo* tradeBaseInfo, ESource eSource)
{
  if (!tradeBaseInfo)
    return false;

  const std::string& guid = tradeBaseInfo->guid();
  DWORD itemId = tradeBaseInfo->itemid();
  DWORD count = tradeBaseInfo->count();
  // get package
  BasePackage* pPack = nullptr;
  // get item
  ItemBase* pItem = getTradeSellItem(guid, &pPack);
  if (pItem == nullptr || pPack == nullptr)
  {
    XERR << "[交易-出售] 背包里找不到该物品 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    MsgManager::sendMsg(m_pUser->id, 10107);
    return false;
  }

  /*BasePackage* pPack = nullptr;
  ItemBase* pItem = getItem(guid, &pPack);
  if (pPack == nullptr || pItem == nullptr)
  {
    XERR << "[交易-出售] 包裹和物品未找到 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    return false;
  }*/

  // check item count
  if (pPack->checkItemCount(guid, count) == false)
  {
    XERR << "[交易-出售][个数不足] user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    MsgManager::sendMsg(m_pUser->id, 10100);
    return false;
  }
  if (pItem->getTypeID() != itemId) {
    XERR << "[交易-出售] 数据异常 user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "guid=" << guid << "typeid:" << pItem->getTypeID() << XEND;
    return false;
  }

  // check item count
  if (pPack->checkItemCount(guid, count) == false)
  {
    XERR << "[交易-出售][个数不足] user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
    MsgManager::sendMsg(m_pUser->id, 10100);
    return false;
  }

  /*ItemData oData;
  pItem->toItemData(&oData);
  if (oData.egg().equips_size() != 0)
  {
    for (int i = 0; i < oData.egg().equips_size(); ++i)
    {
      const EggEquip& rEquip = oData.egg().equips(i);
      ItemData oEquip;
      oEquip.mutable_base()->CopyFrom(rEquip.base());
      oEquip.mutable_equip()->CopyFrom(rEquip.data());
      pPack->addItem(oEquip, EPACKMETHOD_NOCHECK);
      XDBG << "[交易-出售] user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << "是宠物蛋,脱下装备" << oEquip.ShortDebugString() << XEND;
    }
  }*/

  pPack->reduceItem(guid, eSource, count);  
  m_pUser->saveDataNow();
  XLOG << "[交易-出售][出售成功，扣除物品成功] user =" << m_pUser->accid << m_pUser->id << "itemid =" << itemId << "count =" << count << XEND;
  return true;
}

bool Package::auctionAddItem(const Cmd::TakeRecordSCmd& rev)
{
  if (rev.zeny())
  {
    m_pUser->addMoney(EMONEYTYPE_SILVER, rev.zeny(), ESOURCE_AUCTION, true);
  }
  else if (rev.bcat())
  {
    m_pUser->addMoney(EMONEYTYPE_LOTTERY, rev.bcat(), ESOURCE_AUCTION, true);
  }
  else if (rev.has_itemdata())
  {
    ItemData item = rev.itemdata();
    item.mutable_base()->set_source(ESOURCE_AUCTION);
    if (item.base().guid().empty())
    {
      XERR << "[拍卖行-领取装备] 增加装备失败" << item.base().guid() << "accid:" << m_pUser->accid << m_pUser->id << "guid is empty" << XEND;
      return false;
    }

    if (!addItem(item, EPACKMETHOD_AVAILABLE))
    {
      XERR << "[拍卖行-领取装备] 增加装备失败,give up，accid:" << m_pUser->accid << m_pUser->id << rev.ShortDebugString() << XEND;
      return false;
    }
  }
  else if (rev.item().id())
  {
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr)
      return false;
    if (!pMainPack->checkAddItem(rev.item(), EPACKMETHOD_CHECK_WITHPILE))
    {
      MsgManager::sendMsg(m_pUser->id, 989);
        XERR << "[拍卖行-加道具]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "背包空间不足" << rev.item().count() << XEND;
      return false;
    }

    if (!pMainPack->addItem(rev.item(), EPACKMETHOD_CHECK_WITHPILE))
    {
      XERR << "[拍卖行-加道具] 放入背包失败,accid:" << m_pUser->accid << m_pUser->id << "itemid" << rev.item().id() <<"count"<<rev.item().count() << "type"<<rev.type() <<"id" << rev.id() << XEND;
      return false;
    }
  }
  m_pUser->saveDataNow();
  XLOG << "[拍卖行-加道具] 加物品或者钱成功,accid:" << m_pUser->accid << m_pUser->id <<"zeny" << rev.zeny() <<"bcat"<<rev.bcat() <<"itemid" << rev.item().id() << "count" << rev.item().count() << "type" << rev.type() << "id" << rev.id() << XEND;
  return true;
}

DWORD Package::getVarUseCnt(DWORD itemid)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(itemid);
  if (pCFG == nullptr)
    return 0;

  if (m_pUser->getVar().getVarValue(EVARTYPE_USEITEM_DAY) == 0)
  {
    //m_vecItemUseCount.clear();
    for (auto v = m_vecItemUseCount.begin(); v != m_vecItemUseCount.end(); )
    {
      const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(itemid);
      if (cfg == nullptr)
        continue;
      if (cfg->dwDaliyLimit)
      {
        v = m_vecItemUseCount.erase(v);
        continue;
      }
      ++v;
    }
    m_pUser->getVar().setVarValue(EVARTYPE_USEITEM_DAY, 1);

    if (pCFG->dwDaliyLimit)
      return 0;
  }

  if (m_pUser->getVar().getVarValue(EVARTYPE_USEITEM_WEEK) == 0)
  {
    for (auto v = m_vecItemUseCount.begin(); v != m_vecItemUseCount.end(); )
    {
      const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(itemid);
      if (cfg == nullptr)
        continue;
      if (cfg->dwWeekLimit)
      {
        v = m_vecItemUseCount.erase(v);
        continue;
      }
      ++v;
    }
    m_pUser->getVar().setVarValue(EVARTYPE_USEITEM_WEEK, 1);

    if (pCFG->dwWeekLimit)
      return 0;
  }

  auto it = find_if(m_vecItemUseCount.begin(), m_vecItemUseCount.end(), [itemid](const pair<DWORD, DWORD>& r){
      return r.first == itemid;
      });
  if (it != m_vecItemUseCount.end())
    return it->second;
  return 0;
}

void Package::setVarUseCnt(DWORD itemid, DWORD value)
{
  auto it = find_if(m_vecItemUseCount.begin(), m_vecItemUseCount.end(), [itemid](const pair<DWORD, DWORD>& r){
      return r.first == itemid;
      });
  if (it != m_vecItemUseCount.end())
  {
    it->second = value;
  }
  else
  {
    pair<DWORD, DWORD> pa (itemid, value);
    m_vecItemUseCount.push_back(pa);
  }

  sendVarUseCnt(itemid);
}

void Package::sendVarUseCnt(DWORD itemid)
{
  DWORD count = getVarUseCnt(itemid);
  UseCountItemCmd cmd;
  cmd.set_itemid(itemid);
  cmd.set_count(count);
  PROTOBUF(cmd, send, len);

  m_pUser->sendCmdToMe(send, len);
}

DWORD Package::getVarGetCnt(DWORD itemid, ESource source)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(itemid);
  if (pCFG == nullptr)
    return 0;

  if (m_pUser->getVar().getVarValue(EVARTYPE_GETITEM_DAY) == 0)
  {
    for (auto v = m_mapItemGetCount.begin(); v != m_mapItemGetCount.end(); )
    {
      const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(v->first);
      if (cfg == nullptr)
      {
        ++v;
        continue;
      }
      if (cfg->eGetLimitType == EITEMGETLIMITTYPE_DAY)
      {
        v = m_mapItemGetCount.erase(v);
        continue;
      }
      ++v;
    }
    m_pUser->getVar().setVarValue(EVARTYPE_GETITEM_DAY, 1);

    if (pCFG->eGetLimitType == EITEMGETLIMITTYPE_DAY)
      return 0;
  }

  if (m_pUser->getVar().getVarValue(EVARTYPE_GETITEM_WEEK) == 0)
  {
    for (auto v = m_mapItemGetCount.begin(); v != m_mapItemGetCount.end(); )
    {
      const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(v->first);
      if (cfg == nullptr)
      {
        ++v;
        continue;
      }
      if (cfg->eGetLimitType == EITEMGETLIMITTYPE_WEEK)
      {
        v = m_mapItemGetCount.erase(v);
        continue;
      }
      ++v;
    }
    m_pUser->getVar().setVarValue(EVARTYPE_GETITEM_WEEK, 1);

    if (pCFG->eGetLimitType == EITEMGETLIMITTYPE_WEEK)
      return 0;
  }

  auto it = m_mapItemGetCount.find(itemid);
  if (it == m_mapItemGetCount.end())
    return 0;
  if (source == ESOURCE_MIN)
    return it->second.dwCount;
  if (pCFG->bUniformSource)
  {
    // 所有source统一计算
    DWORD cnt = 0;
    for (auto& v : it->second.mapSource2Count)
      cnt += v.second;
    return cnt;
  }
  auto itsource = it->second.mapSource2Count.find(source);
  if (itsource == it->second.mapSource2Count.end())
    return 0;
  return itsource->second;
}

void Package::addVarGetCnt(DWORD itemid, DWORD value, ESource source)
{
  SItemGetCount& v = m_mapItemGetCount[itemid];
  if (source == ESOURCE_MIN)
  {
    v.dwCount += value;
    sendVarGetCnt(itemid, ESOURCE_MIN);
    return;
  }
  // 指定source的上限不同步前端
  if (v.mapSource2Count.find(source) == v.mapSource2Count.end())
    v.mapSource2Count[source] = value;
  else
    v.mapSource2Count[source] += value;
}

void Package::sendVarGetCnt(DWORD itemid, ESource source)
{
  DWORD count = getVarGetCnt(itemid, source);
  GetCountItemCmd cmd;
  cmd.set_itemid(itemid);
  cmd.set_count(count);
  cmd.set_source(source);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

DWORD Package::updateVarRealGetCnt(const SItemCFG* pCFG, DWORD itemid, DWORD dwCount, ESource source, EPackType packtype)
{
  if (pCFG == nullptr || pCFG->eGetLimitType == EITEMGETLIMITTYPE_NONE)
    return dwCount;

  if (pCFG->vecLimitSource.empty() && source != ESOURCE_HELP && source != ESOURCE_REWARD && source != ESOURCE_SHOP && source != ESOURCE_PVP)
    return dwCount;

  ESource src = pCFG->vecLimitSource.empty() ? ESOURCE_MIN : source; // 统计获取次数用的source
  DWORD dwGetCnt = getVarGetCnt(itemid, src);
  DWORD dwGetLimit = pCFG->getItemGetLimit(m_pUser->getLevel(), src, m_pUser->getDeposit().hasMonthCard());
  if (dwGetLimit == DWORD_MAX)
    return dwCount;

  if (dwGetCnt + dwCount > dwGetLimit)
  {
    if (dwGetCnt >= dwGetLimit)
    {
      XLOG << "[包裹-物品添加-获取上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
           << "添加" << itemid << dwCount << source << "到" << packtype << "超过上限:" << dwGetLimit
           << "上限类型:" << pCFG->eGetLimitType << "已获得道具数:" << dwGetCnt << XEND;
      DWORD dwMsgID = MiscConfig::getMe().getItemGetLimitMsg(pCFG->eGetLimitType);
      if (dwMsgID != 0) MsgManager::sendMsg(m_pUser->id, dwMsgID, MsgParams(pCFG->strNameZh));
      return 0;
    }
    else
    {
      DWORD dwNewCount = dwGetLimit - dwGetCnt;
      XLOG << "[包裹-物品添加-获取上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
           << "添加" << itemid << dwCount << source << "到" << packtype << "超过上限:" << dwGetLimit
           << "只能获得:" << dwNewCount << "个,上限类型:" << pCFG->eGetLimitType << "已获得道具数:" << dwGetCnt << XEND;
      addVarGetCnt(itemid, dwNewCount, src);
      return dwNewCount;
    }
  }

  addVarGetCnt(itemid, dwCount, src);
  return dwCount;
}

// 修复未生效的附魔属性
void Package::fixInvalidEnchant()
{
  set<EPackType> packTypes;
  packTypes.insert(EPACKTYPE_MAIN);
  packTypes.insert(EPACKTYPE_EQUIP);
  packTypes.insert(EPACKTYPE_PERSONAL_STORE);

  for (auto packType : packTypes) {
    BasePackage* pPack = getPackage(packType);
    if (pPack == nullptr) {
      continue;
    }

    for (auto &it : pPack->m_mapID2Item) {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(it.second);
      if (pEquip == nullptr)
        continue;

      EnchantData& rEnchant = pEquip->getEnchantData();
      if (rEnchant.type() != EENCHANTTYPE_MIN || rEnchant.extras_size() <= 0) {
        continue;
      }

      EEnchantType enchantType = EENCHANTTYPE_MIN;

      for (int i = 0; i < rEnchant.extras_size(); ++i) {
        for (auto ectype = EENCHANTTYPE_MIN + 1; ectype < EENCHANTTYPE_MAX; ++ectype) {
          const SEnchantCFG* pCfg = ItemConfig::getMe().getEnchantCFG(static_cast<EEnchantType>(ectype));
          if (pCfg == nullptr)
            continue;
          if (pCfg->getEnchantAttr(rEnchant.extras(i).configid()) == nullptr)
            continue;
          enchantType = static_cast<EEnchantType>(ectype);
          break;
        }
        if (enchantType != EENCHANTTYPE_MIN) {
          rEnchant.set_type(enchantType);
          break;
        }
      }
    }
  }
}

// 调整外网玩家身上已有的高级附魔属性比例
void Package::fixEnchantAttr()
{
  set<EPackType> packTypes;
  packTypes.insert(EPACKTYPE_MAIN);
  packTypes.insert(EPACKTYPE_EQUIP);
  packTypes.insert(EPACKTYPE_FASHIONEQUIP);
  packTypes.insert(EPACKTYPE_STORE);
  packTypes.insert(EPACKTYPE_PERSONAL_STORE);
  packTypes.insert(EPACKTYPE_TEMP_MAIN);
  packTypes.insert(EPACKTYPE_BARROW);

  DWORD version = 20170519;

  for (auto packType : packTypes) {
    BasePackage* pPack = getPackage(packType);
    if (pPack == nullptr) {
      continue;
    }

    bool changed = false;
    for (auto &it : pPack->m_mapID2Item) {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(it.second);
      if (pEquip == nullptr)
        continue;

      EnchantData& rEnchant = pEquip->getEnchantData();
      if (rEnchant.type() != EENCHANTTYPE_SENIOR)
        continue;

      bool patched = false;
      for (int i = 0; i < rEnchant.patch_size(); ++i) {
        if (rEnchant.patch(i) == version)
          patched = true;
      }
      if (patched)
        continue;

      for (int i = 0; i < rEnchant.attrs_size(); ++i) {
        float rate = 0;
        switch (rEnchant.attrs(i).type()) {
        case EATTRTYPE_HIT:
        case EATTRTYPE_FLEE:
        case EATTRTYPE_MAXSP:
        case EATTRTYPE_DEF:
          rate = 2; break;
        case EATTRTYPE_CRIRES:
        case EATTRTYPE_SILENCEDEF:
        case EATTRTYPE_FREEZEDEF:
        case EATTRTYPE_STONEDEF:
        case EATTRTYPE_STUNDEF:
        case EATTRTYPE_BLINDDEF:
        case EATTRTYPE_POSIONDEF:
        case EATTRTYPE_SLOWDEF:
        case EATTRTYPE_CHAOSDEF:
        case EATTRTYPE_CURSEDEF:
          rate = 2.5; break;
        default:
          continue;
        }
        if (rate != 0) {
          DWORD v = rEnchant.attrs(i).value();
          rEnchant.mutable_attrs(i)->set_value(v * rate);
          changed = true;
          XLOG << "[附魔属性修复]" << m_pUser->accid << m_pUser->id << m_pUser->name << "item:" << pEquip->getTypeID() << "attr:" << rEnchant.attrs(i).type() << "value:" << v << "改为" << rEnchant.attrs(i).value() << XEND;
        }
      }

      rEnchant.add_patch(version);
    }

    if (changed && packType == EPACKTYPE_EQUIP)
      m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  }
}

DWORD Package::getPackSlotUsed(DWORD packtype)
{
  EPackType eType = static_cast<EPackType> (packtype);
  if (eType <= EPACKTYPE_MIN || eType >= EPACKTYPE_MAX)
    return 0;

  return m_pPackage[eType]->getUsedSlot();
}

DWORD Package::getPackSlot(DWORD packtype)
{
  EPackType eType = static_cast<EPackType> (packtype);
  if (eType <= EPACKTYPE_MIN || eType >= EPACKTYPE_MAX)
    return 0;

  return m_pPackage[eType]->getLastMaxSlot();
}

void Package::itemModify(DWORD dwOldID, DWORD dwNewID)
{
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    BasePackage* pPack = m_pPackage[i];
    if (pPack == nullptr)
      continue;

    auto func = [&](ItemBase* pBase)
    {
      if (pBase == nullptr || pBase->getTypeID() != dwOldID)
        return;
      const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(dwNewID);
      if (pCFG == nullptr)
      {
        XERR << "[包裹-操作]" << dwNewID << "没有在 Table_Item.txt 表中找到" << XEND;
        return;
      }
      pBase->setCFG(pCFG);
      pPack->setUpdateIDs(pBase->getGUID());

      if (pPack->getPackageType() == EPACKTYPE_STORE)
      {
        ItemModifyRecordCmd cmd;
        cmd.set_accid(m_pUser->accid);
        cmd.set_charid(m_pUser->id);
        cmd.set_newid(dwNewID);
        cmd.set_guid(pBase->getGUID());
        cmd.set_scenename(thisServer->getServerName());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToRecord(send, len);
      }
    };
    pPack->foreach(func);
  }

  XLOG << "[包裹-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行了id变更 old :" << dwOldID << "->" << "new :" << dwNewID << XEND;
}

void Package::itemRemove(DWORD dwItemID, EPackType eType, ESource eSource)
{
  BasePackage* pPack = getPackage(eType);
  if (pPack == nullptr)
  {
    XERR << "[包裹-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行从" << eType << "删除id :" << dwItemID << "失败,未找到" << eType << XEND;
    return;
  }
  DWORD dwCount = pPack->getItemCount(dwItemID);
  if (dwCount == 0)
  {
    XLOG << "[包裹-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行从" << eType << "删除id :" << dwItemID << "成功,没有需要删除的道具" << XEND;
    return;
  }
  pPack->reduceItem(dwItemID, eSource, dwCount);
  XLOG << "[包裹-操作]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行从" << eType << "删除id :" << dwItemID << "成功" << XEND;
}

bool Package::addShowItems(const ItemInfo& rInfo, bool bShow, bool bInform, bool bForceShow)
{
  return addShowItems(rInfo, bShow, bInform, bForceShow, m_vecShowItems, m_vecInformItems);
}

bool Package::addShowItems(const ItemInfo& rInfo, bool bShow, bool bInform, bool bForceShow, TVecItemInfo& rShowItems, TVecItemInfo& rInfoItems)
{
  if (rInfo.count() == 0)
    return false;
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(rInfo.id());
  if (pCFG == nullptr)
    return false;
  if (bShow)
    combinItemInfo(rShowItems, TVecItemInfo{ rInfo });
  if (bInform)
    //rInfoItems.push_back(rInfo);
    combinItemInfo(rInfoItems, TVecItemInfo{ rInfo });
  if (bForceShow)
  {
    if (ItemConfig::getMe().isShowItem(pCFG->eItemType) == true)
    {
      if (m_pUser->getManual().getFashionStatus(rInfo.id()) == EMANUALSTATUS_MIN || m_pUser->getManual().getFashionStatus(rInfo.id()) == EMANUALSTATUS_DISPLAY)
      {
        DWORD dwID = rInfo.id();
        auto check = find_if(rShowItems.begin(), rShowItems.end(), [dwID](const ItemInfo& rInfo) -> bool {
          return rInfo.id() == dwID;
        });
        if (check == rShowItems.end())
          combinItemInfo(rShowItems, TVecItemInfo{ rInfo });
      }
    }
  }

  return rShowItems.empty() == false || rInfoItems.empty() == false;
}

void Package::stopMultipleUse()
{
  m_multipleUse = false;
  showItems(m_vecShowItems, m_vecInformItems);
}

void Package::showItems(TVecItemInfo& rShowItems, TVecItemInfo& rInfoItems)
{
  if (m_pUser == nullptr)
    return;

  auto getGUIDByType = [&](DWORD itemId)->string {
    string str;
    for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
    {
      if (i == EPACKTYPE_MAIN || i == EPACKTYPE_TEMP_MAIN)
      {
        BasePackage* pPack = m_pPackage[i];
        if (pPack == nullptr)
          continue;
        str = pPack->getGUIDByType(itemId);
        if (!str.empty())
          return str;
      }
    }
    return str;
  };

  // item show
  if (rShowItems.empty() == false)
  {
    ItemShow cmd;
    for (auto v = rShowItems.begin(); v != rShowItems.end(); ++v)
    {
      ItemInfo* pInfo = cmd.add_items();
      if (pInfo != nullptr)
      {
        pInfo->set_id(v->id());
        pInfo->set_count(v->count());
        pInfo->set_guid(getGUIDByType(v->id()));
        pInfo->set_source(v->source());
        XLOG << "[包裹-多次使用道具展示]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "展示了 id:" << v->id() << "count:" << v->count() << "guid:" << pInfo->guid() << "道具" << XEND;
      }

      DWORD id = v->id();
      auto item = find_if(rInfoItems.begin(), rInfoItems.end(), [id](const ItemInfo& r) -> bool {
        return r.id() == id;
      });
      if (item != rInfoItems.end())
        rInfoItems.erase(item);
    }
    if (cmd.items_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
    rShowItems.clear();
  }

  // msg inform
  if (rInfoItems.empty() == false)
  {
    MsgParams expParams;
    DWORD baseExp = 0;
    DWORD jobExp = 0;
    DWORD dwDelay = 0;
    for (auto &v : rInfoItems)
    {
      if (v.source() == ESOURCE_WANTEDQUEST)
      {
        if (dwDelay < MiscConfig::getMe().getDelayCFG().dwWantedQuest)
          dwDelay = MiscConfig::getMe().getDelayCFG().dwWantedQuest;
      }
      else if (v.source() == ESOURCE_EXCHANGECARD && m_pUser->getPackage().getCardAnim())
        dwDelay = MiscConfig::getMe().getDelayCFG().dwExchangeCard;
      else if (Lottery::isLotterySource(v.source()) && !m_pUser->getLottery().isSkipLottery())
        dwDelay = MiscConfig::getMe().getDelayCFG().dwLottery;
    }

    bool bIsDeadOpen = m_pUser->getMenu().isOpen(EMENUID_DEAD);
    for (auto v = rInfoItems.begin(); v != rInfoItems.end(); ++v)
    {
      if (v->id() == ITEM_DEAD && !bIsDeadOpen)
        continue;
      if (v->id() == ITEM_DEAD)
      {
        v->set_count(m_pUser->getTempDeadGet());
        if (v->count() == 0)
          continue;
      }

      MsgParams params;
      DWORD id = 0;

      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->id());
      if (pCFG == nullptr)
        continue;

      if (pCFG->eItemType == EITEMTYPE_BASEEXP)
      {
        id = 4;
        baseExp = v->count();
        continue;
      }
      else if (pCFG->eItemType == EITEMTYPE_JOBEXP)
      {
        id = 3;
        jobExp = v->count();
        continue;
      }
      else
      {
        if (v->source() == ESOURCE_PICKUP)
          id = 15;
        else if (v->source() == ESOURCE_QUICKSTORE)
          continue;
        else if (v->source() == ESOURCE_QUICKSTORE_RETURN)
          continue;
        else
          id = 6;
        params.addNumber(v->id());
        params.addNumber(v->id());
        params.addNumber(v->count());
      }
      //奖励时头顶不飘
      MsgManager::sendMsg(m_pUser->id, id, params, EMESSAGETYPE_FRAME, EMESSAGEACT_ADD, dwDelay);
    }
    if (0 == jobExp && baseExp > 0)
    {
      MsgManager::sendMsg(m_pUser->id, 4, MsgParams(baseExp), EMESSAGETYPE_FRAME, EMESSAGEACT_ADD, dwDelay);
    }
    else if (0 == baseExp && jobExp > 0)
    {
      MsgManager::sendMsg(m_pUser->id, 3, MsgParams(jobExp), EMESSAGETYPE_FRAME, EMESSAGEACT_ADD, dwDelay);
    }
    else if (jobExp > 0 && baseExp > 0)
    {
      MsgManager::sendMsg(m_pUser->id, 17, MsgParams(baseExp, jobExp), EMESSAGETYPE_FRAME, EMESSAGEACT_ADD, dwDelay);
    }

    rInfoItems.clear();
  }
}

void Package::version_update()
{
  for (DWORD d = m_dwDataVersion; d < PACKAGE_VERSION; ++d)
  {
    if (d == 0)
      version_1();
    else if (d == 1)
      version_2();
    else if (d == 2)
      version_3();
    else if (d == 3)
      version_4();
    else if (d == 4)
      version_5();
    else if (d == 5)
      version_6();
    else if (d == 6)
      version_7();
  }
  m_dwDataVersion = PACKAGE_VERSION;
}

void Package::version_1()
{
  checkInvalidEquip();
  XLOG << "[包裹-版本-1]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Package::version_2()
{
  DWORD dwGarden = m_pUser->getUserSceneData().getGarden();
  if (dwGarden != 0)
  {
    BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
    if (pMainPack != nullptr)
    {
      ItemData oData;
      oData.mutable_base()->set_id(ITEM_GARDEN);
      oData.mutable_base()->set_count(dwGarden);
      oData.mutable_base()->set_source(ESOURCE_PACKAGE);
      pMainPack->addItem(oData, EPACKMETHOD_NOCHECK);
      m_pUser->getUserSceneData().setGarden(0);
    }
    else
    {
      XERR << "[包裹-版本-2]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行,失败,未找到" << EPACKTYPE_MAIN << XEND;
    }
  }
  XLOG << "[包裹-版本-2]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Package::version_3()
{
  BasePackage* pQuestPack = getPackage(EPACKTYPE_QUEST);
  if (pQuestPack == nullptr)
  {
    XERR << "[包裹-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行失败,未找到" << EPACKTYPE_QUEST << XEND;
    return;
  }

  for (DWORD d = EPACKTYPE_MIN + 1; d < EPACKTYPE_MAX; ++d)
  {
    if (d == EPACKTYPE_EQUIP || d == EPACKTYPE_FASHION || d == EPACKTYPE_FASHIONEQUIP || d == EPACKTYPE_CARD || d == EPACKTYPE_STORE || d == EPACKTYPE_QUEST)
      continue;

    BasePackage* pPack = m_pPackage[d];
    if (pPack == nullptr)
    {
      XERR << "[包裹-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行失败,未找到" << static_cast<EPackType>(d) << XEND;
      continue;
    }

    TVecItemInfo vecQuest;
    TMapPackItem mapItem = pPack->m_mapID2Item;
    for (auto &m : mapItem)
    {
      ItemBase* pBase = m.second;
      if (pBase != nullptr && pBase->getCFG() != nullptr && pBase->getCFG()->eItemType == EITEMTYPE_QUESTITEM)
      {
        ItemData oData;
        pBase->toItemData(&oData);
        combinItemInfo(vecQuest, oData.base());
      }
    }
    if (vecQuest.empty() == false && pPack->checkItemCount(vecQuest) == false)
    {
      XERR << "[包裹-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "执行失败" << static_cast<EPackType>(d) << "收集到" << vecQuest << "但检查失败" << XEND;
      continue;
    }
    pPack->reduceItem(vecQuest, ESOURCE_PACKAGE);

    TVecItemInfo vecCopy = vecQuest;
    if (pQuestPack->addItem(vecQuest, EPACKMETHOD_NOCHECK) == false)
    {
      XERR << "[包裹-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "执行失败" << static_cast<EPackType>(d) << "收集到" << vecCopy << "添加到" << EPACKTYPE_QUEST << "失败" << XEND;
      continue;
    }
    XLOG << "[包裹-版本-3]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "执行成功" << static_cast<EPackType>(d) << "收集到" << vecCopy << "添加到" << EPACKTYPE_QUEST << XEND;
  }
}

void Package::version_4()
{
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return;

  auto func = [&](ItemBase* pItem)
  {
    ItemEgg* pEgg = dynamic_cast<ItemEgg*>(pItem);
    if (pEgg == nullptr)
      return;
    pEgg->patch_1();
  };
  pMainPack->foreach(func);
  XLOG << "[包裹-版本-4], 玩家:" << m_pUser->name << m_pUser->id << "更新宠物蛋body完成" << XEND;
}

void Package::version_5()
{
  checkInvalidEquip();
  XLOG << "[包裹-版本-5]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Package::version_6()
{
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    BasePackage* pPack = getPackage(static_cast<EPackType>(i));
    if (pPack == nullptr)
      continue;
    while (true)
    {
      const string& guid = pPack->getGUIDByType(ITEM_PET_WORK);
      if (guid.empty() == true)
        break;
      pPack->reduceItem(guid, ESOURCE_GM);
    }
  }
  XLOG << "[包裹-版本-6]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Package::version_7()
{
  DWORD dwDelItemID = 41517;
  DWORD dwAddItemID = 42551;
  set<EPackType> setPacks = set<EPackType>{EPACKTYPE_MAIN, EPACKTYPE_PERSONAL_STORE, EPACKTYPE_BARROW};
  for (auto &s : setPacks)
  {
    BasePackage* pPack = getPackage(s);
    if (pPack == nullptr)
    {
      XERR << "[包裹-版本-7]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "执行" << s << "失败,未找到该包裹" << XEND;
      continue;
    }
    DWORD dwCount = pPack->getItemCount(dwDelItemID);
    if (dwCount == 0)
      continue;
    pPack->reduceItem(dwDelItemID, ESOURCE_GM, dwCount);

    ItemData oData;
    oData.mutable_base()->set_id(dwAddItemID);
    oData.mutable_base()->set_count(dwCount);
    pPack->addItem(oData, EPACKMETHOD_NOCHECK);
  }
  XLOG << "[包裹-版本-7]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "已执行" << XEND;
}

void Package::resetQuestPackItem()
{
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-任务包裹]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置任务包裹失败,未找到" << EPACKTYPE_MAIN << XEND;
    return;
  }

  if (pMainPack->getItemCount(ITEM_QUEST_PACK) != 0)
    return;

  ItemData oData;
  oData.mutable_base()->set_id(ITEM_QUEST_PACK);
  oData.mutable_base()->set_count(1);
  if (pMainPack->addItem(oData, EPACKMETHOD_NOCHECK) == false)
  {
    XERR << "[包裹-任务包裹]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置任务包裹失败,添加" << ITEM_QUEST_PACK << "失败" << XEND;
    return;
  }

  XLOG << "[包裹-任务包裹]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置任务包裹成功" << XEND;
}

bool Package::breakEquip(EEquipPos pos, DWORD duration, xSceneEntryDynamic* entry)
{
  if (isEquipProtected(pos) || duration <= 0)
    return false;

  EquipPackage* pPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pPack == nullptr)
    return false;
  ItemEquip* pEquip = pPack->getEquip(pos);
  if (pEquip == nullptr)
    return false;

  bool broken = pEquip->isBroken();

  if (pEquip->breakEquip(duration) == false)
    return false;
  pPack->setUpdateIDs(pEquip->getGUID());

  if (entry)
  {
    MsgParams param;
    param.addNumber(pEquip->getTypeID());
    param.addString(entry->name);
    MsgManager::sendMsg(m_pUser->id, 617, param);

    if (entry->getEntryType() == SCENE_ENTRY_USER)
    {
      param.clear();
      param.addString(m_pUser->name);
      param.addNumber(pEquip->getTypeID());
      MsgManager::sendMsg(entry->id, 618, param);
    }
  }
  // 之前已经被破坏了, 则不处理仅更新破坏时间, 防止buff被重复删除
  if (broken == false)
  {
    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    m_pUser->m_oBuff.delEquipBuffBreakInvalid(pEquip, true);
  }

  m_pUser->refreshDataAtonce();
  addEquipAttrAction(pEquip->getGUID(), duration);

  XLOG << "[包裹-装备破坏]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "pos:" << pos << "时间:" << duration << "成功" << XEND;
  return true;
}

bool Package::forceOffEquip(EEquipPos pos, DWORD duration, xSceneEntryDynamic* entry)
{
  if (duration <= 0)
    return false;

  EquipPackage* pPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pPack == nullptr)
    return false;

  // 对应位置有装备则脱掉
  ItemEquip* pEquip = pPack->getEquip(pos);
  if (pEquip)
  {
    if (equipOff(pEquip->getGUID()) == false)
    {
      XERR << "[包裹-装备脱卸]" << m_pUser->accid << m_pUser->id << m_pUser->name << "pos:" << pos << "duration:" << duration << "脱卸失败" << XEND;
    }
    else
    {
      m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
      m_pUser->refreshDataAtonce();

      if (entry)
      {
        MsgParams param;
        param.addNumber(pEquip->getTypeID());
        param.addString(entry->name);
        MsgManager::sendMsg(m_pUser->id, 615, param);

        if (entry->getEntryType() == SCENE_ENTRY_USER)
        {
          param.clear();
          param.addString(m_pUser->name);
          param.addNumber(pEquip->getTypeID());
          MsgManager::sendMsg(entry->id, 616, param);
        }
      }
    }
  }


  DWORD cur = now();
  SEquipPosData& posdata = m_mapEquipPos[pos];
  posdata.ePos = pos;
  posdata.dwOffStartTime = cur;
  posdata.dwOffEndTime = cur + duration;
  if (pEquip)
    posdata.strGuid = pEquip->getGUID();
  else
    posdata.strGuid.clear();

  sendEquipPosData(pos);

  XLOG << "[包裹-装备脱卸]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "pos:" << pos << "时间:" << duration << "成功" << XEND;
  return true;
}

bool Package::recoverOffEquip(EEquipPos pos/*=EEQUIPPOS_MIN*/)
{
  DWORD cur = now();
  auto recover = [&](EEquipPos e)
  {
    if (isEquipForceOff(e) == false)
      return;
    auto it = m_mapEquipPos.find(e);
    if (it == m_mapEquipPos.end())
      return;
    it->second.dwOffEndTime = cur;

    if (!it->second.strGuid.empty())
    {
      equipOn(e, it->second.strGuid);
      m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
      m_pUser->refreshDataAtonce();
    }

    sendEquipPosData(pos);
    XLOG << "[装备拆卸-恢复], 玩家:" << m_pUser->name << m_pUser->id << "位置:" << e << "装备guid:" << it->second.strGuid << XEND;
  };

  if (pos != EEQUIPPOS_MIN)
  {
    recover(pos);
  }
  else
  {
    for (int i = EEQUIPPOS_MIN + 1; i < EEQUIPPOS_MAX; ++i)
    {
      if (EEquipPos_IsValid(i) == false)
        continue;
      recover((EEquipPos)i);
    }
  }

  return true;
}

bool Package::recoverOffEquipByTime(DWORD time)
{
  DWORD cur = now();
  bool find = false;
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
    return false;

  for (auto &m : m_mapEquipPos)
  {
    if (m.second.dwOffEndTime < cur && m.second.dwOffEndTime + time >= cur )
    {
      if (!m.second.strGuid.empty())
      {
        ItemEquip* pEquip = pEquipPack->getEquip(m.second.ePos);
        if (pEquip == nullptr)
        {
          equipOn(m.second.ePos, m.second.strGuid);
          m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
          m_pUser->refreshDataAtonce();
          m.second.strGuid.clear();
          find = true;
        }
      }
    }
  }

  return find;
}

bool Package::protectEquip(EEquipPos pos, DWORD duration, bool always)
{
  if (duration <= 0 && always == false)
    return false;

  SEquipPosData& posdata = m_mapEquipPos[pos];
  posdata.ePos = pos;

  if (duration > 0)
    posdata.dwProtectTime = now() + duration;
  if (always)
    posdata.dwProtectAlways += 1;

  sendEquipPosData(pos);

  XLOG << "[包裹-装备保护]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "pos:" << pos << "时间:" << duration << "永久保护:" << posdata.dwProtectAlways << "成功" << XEND;
  return true;
}

void Package::cancelAlwaysProtectEquip(EEquipPos pos)
{
  auto it = m_mapEquipPos.find(pos);
  if (it == m_mapEquipPos.end())
    return;
  if (it->second.dwProtectAlways > 0)
    it->second.dwProtectAlways -= 1;
  sendEquipPosData(pos);
  XLOG << "[包裹-取消装备永久保护]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "pos:" << pos << "永久保护:" << it->second.dwProtectAlways << "成功" << XEND;
}

bool Package::fixBrokenEquip(EEquipPos pos)
{
  EquipPackage* pPack = dynamic_cast<EquipPackage*>(getPackage(EPACKTYPE_EQUIP));
  if (pPack == nullptr)
    return false;
  ItemEquip* pEquip = pPack->getEquip(pos);
  if (pEquip == nullptr)
    return false;
  if (pEquip->fixBrokenEquip() == false)
    return false;
  pPack->setUpdateIDs(pEquip->getGUID());
  m_pUser->m_oBuff.addEquipBuffBreakInvalid(pEquip);
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();
  XLOG << "[包裹-装备修理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "pos:" << pos << "成功" << XEND;
  return true;
}

bool Package::isEquipProtected(EEquipPos pos)
{
  auto it = m_mapEquipPos.find(pos);
  if (it == m_mapEquipPos.end())
    return false;
  return now() < it->second.dwProtectTime || it->second.dwProtectAlways > 0;
}

bool Package::isEquipForceOff(EEquipPos pos)
{
  auto it = m_mapEquipPos.find(pos);
  if (it == m_mapEquipPos.end())
    return false;
  DWORD cur = now();
  return it->second.dwOffStartTime <= cur && cur < it->second.dwOffEndTime;
}

bool Package::hasEquipForceOff()
{
  DWORD cur = now();
  for (auto &it : m_mapEquipPos)
  {
    if (it.second.dwOffStartTime <= cur && cur < it.second.dwOffEndTime)
      return true;
  }
  return false;
}

void Package::sendEquipPosData(EEquipPos pos/* = EEQUIPPOS_MIN*/)
{
  EquipPosDataUpdate cmd;
  if (pos == EEQUIPPOS_MIN)
  {
    for (auto& v : m_mapEquipPos)
      v.second.toData(cmd.add_datas());
  }
  else
  {
    auto it = m_mapEquipPos.find(pos);
    if (it != m_mapEquipPos.end())
      it->second.toData(cmd.add_datas());
  }
  if (cmd.datas_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void Package::addEquipAttrAction(const string& guid, DWORD duration)
{
  SActionCFG cfg;
  cfg.dwID = 0;
  cfg.type = EACTIONTYPE_EQUIPATTR;
  cfg.oData.setData("time", duration, true);
  cfg.oData.setData("guid", guid);
  m_pUser->getUserAction().addAction(EQUIP_ATTR_ACTION_ID, cfg);
}

void Package::equipOffInValidEquip()
{
  BasePackage* pEquipPack = getPackage(EPACKTYPE_EQUIP);
  if (pEquipPack != nullptr)
  {
    TSetString setIDs;
    pEquipPack->collectInValidEquip(setIDs);
    for (auto &s : setIDs)
    {
      ItemBase* pBase = pEquipPack->getItem(s);
      if (pBase == nullptr)
      {
        XLOG << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << EPACKTYPE_EQUIP << "中guid :" << s << "未找到" << XEND;
        continue;
      }
      if (equip(EEQUIPOPER_OFF, EEQUIPPOS_MIN, s) == false)
      {
        XERR << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << EPACKTYPE_EQUIP << "中guid :" << s << "不符合穿戴条件,卸下失败" << XEND;
        continue;
      }

      MsgParams params;
      params.addNumber(pBase->getTypeID());
      params.addNumber(pBase->getTypeID());
      params.addNumber(pBase->getCount());
      MsgManager::sendMsg(m_pUser->id, 6, params, EMESSAGETYPE_FRAME, EMESSAGEACT_ADD);
      XLOG << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << EPACKTYPE_EQUIP << "中guid :" << s << "不符合穿戴条件,卸下成功" << XEND;
    }
  }
  else
  {
    XERR << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "检查" << EPACKTYPE_EQUIP << "失败,未找到该包裹" << XEND;
  }

  BasePackage* pFEquipPack = getPackage(EPACKTYPE_FASHIONEQUIP);
  if (pFEquipPack != nullptr)
  {
    TSetString setIDs;
    pFEquipPack->collectInValidEquip(setIDs);
    for (auto &s : setIDs)
    {
      ItemBase* pBase = pFEquipPack->getItem(s);
      if (pBase == nullptr)
      {
        XLOG << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << EPACKTYPE_FASHIONEQUIP << "中guid :" << s << "未找到" << XEND;
        continue;
      }
      if (equip(EEQUIPOPER_OFFFASHION, EEQUIPPOS_MIN, s) == false)
      {
        XERR << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << EPACKTYPE_FASHIONEQUIP << "中guid :" << s << "不符合穿戴条件,卸下失败" << XEND;
        continue;
      }

      MsgParams params;
      params.addNumber(pBase->getTypeID());
      params.addNumber(pBase->getTypeID());
      params.addNumber(pBase->getCount());
      MsgManager::sendMsg(m_pUser->id, 6, params, EMESSAGETYPE_FRAME, EMESSAGEACT_ADD);
      XLOG << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << EPACKTYPE_FASHIONEQUIP << "中guid :" << s << "不符合穿戴条件,卸下成功" << XEND;
    }
  }
  else
  {
    XERR << "[包裹-装备检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "检查" << EPACKTYPE_FASHIONEQUIP << "失败,未找到该包裹" << XEND;
  }
}

EError Package::resetGenderEquip(EGender eGender)
{
  if (eGender <= EGENDER_MIN || eGender >= EGENDER_MAX || EGender_IsValid(eGender) == false)
  {
    XERR << "[包裹-时装转换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "转换为" << eGender << "失败, gender :" << eGender << "不合法" << XEND;
    return EERROR_FAIL;
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-时装转换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "转换为" << eGender << "失败, 未找到" << EPACKTYPE_MAIN << XEND;
    return EERROR_FAIL;
  }

  TSetString setIDs;
  for (auto &m : pMainPack->m_mapID2Item)
    setIDs.insert(m.first);

  EError eError = EERROR_FAIL;
  for (auto &s : setIDs)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(s));
    if (pEquip == nullptr)
      continue;
    const SItemCFG* pCFG = pEquip->getCFG();
    if (pCFG == nullptr)
    {
      XERR << "[包裹-时装转换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "转换为" << eGender << "装备" << pEquip->getGUID() << pEquip->getTypeID() << "配置非法,被忽略" << XEND;
      continue;
    }
    if (pEquip->canBeMaterial() == false)
    {
      XWRN << "[包裹-时装转换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "转换为" << eGender << "装备" << pEquip->getGUID() << pEquip->getTypeID() << "包含各种属性,被忽略" << XEND;
      continue;
    }
    DWORD dwOldEquipID = pEquip->getTypeID();
    DWORD dwEquipID = ItemConfig::getMe().getGenderEquip(eGender, dwOldEquipID);
    if (dwEquipID == 0)
    {
      XWRN << "[包裹-时装转换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "转换为" << eGender << "装备" << pEquip->getGUID() << pEquip->getTypeID() << "未在 Table_EquipGender.txt 表中找到对应ID" << XEND;
      continue;
    }
    pMainPack->reduceItem(pEquip->getGUID(), ESOURCE_FASHION_GEDNER, pEquip->getCount());

    ItemData oData;
    oData.mutable_base()->set_id(dwEquipID);
    oData.mutable_base()->set_count(1);
    pMainPack->addItem(oData, EPACKMETHOD_NOCHECK);
    XLOG << "[包裹-时装转换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "转换为" << eGender << "装备" << dwOldEquipID << "转换为" << dwEquipID << XEND;

    eError = EERROR_SUCCESS;
  }

  XLOG << "[包裹-时装转换]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "转换为" << eGender << "执行结果" << eError << XEND;
  return eError;
}

void Package::resetPackMoveItem()
{
  for (auto &m : m_mapPackMoveItem)
  {
    BasePackage* pPack = getPackage(m.first);
    if (pPack == nullptr)
    {
      XERR << "[包裹-物品转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << m.first << "转入" << m.second << "失败,未找到该包裹" << XEND;
      continue;
    }

    TVecItemData vecCopy{m.second};
    if (pPack->addItem(m.second, EPACKMETHOD_NOCHECK, false, false, false) == false)
      XERR << "[包裹-物品转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << m.first << "转入" << vecCopy << "失败,添加失败" << XEND;
    else
      XLOG << "[包裹-物品转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << m.first << "转入" << vecCopy << "成功" << XEND;
  }

  m_mapPackMoveItem.clear();
}

bool Package::makeReduceList(TVecItemInfo vecItem, TMapReduce& mapReduce, ECheckMethod eMethod, const TSetDWORD& setPack)
{
  if (setPack.empty() == true)
    return false;

  mapReduce.clear();
  for (auto &s : setPack)
  {
    EPackType eType = static_cast<EPackType>(s);
    if (canReduce(eType) == false)
    {
      XERR << "[包裹-物品删除-vecitem]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在" << setPack << "包裹中删除" << vecItem << "失败,包裹" << eType << "不能进行删除操作" << XEND;
      return false;
    }
    BasePackage* pPack = getPackage(eType);
    if (pPack == nullptr)
    {
      XERR << "[包裹-物品删除-vecitem]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在" << setPack << "包裹中删除" << vecItem << "失败,未找到" << eType << XEND;
      return false;
    }
    mapReduce[eType].clear();
  }

  // collect money first
  for (auto v = vecItem.begin(); v != vecItem.end();)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v->id());
    if (pCFG == nullptr)
    {
      XERR << "[包裹-物品删除-vecitem]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在" << setPack << "包裹中删除" << vecItem << "其中id :" << v->id() << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }
    if (pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_DIAMOND) ||
        pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_SILVER) ||
        pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GOLD) ||
        pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_CONTRIBUTE) ||
        pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_PVPCOIN) ||
        pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_LOTTERY) ||
        pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GUILDHONOR)
       )
    {
      if (m_pUser->checkMoney(static_cast<EMoneyType>(pCFG->eItemType), v->count()) == false)
        return false;
      combinItemInfo(mapReduce.begin()->second, *v);
      v = vecItem.erase(v);
      continue;
    }
    ++v;
  }

  // collect item
  TVecItemInfo vecCopy = vecItem;
  for (auto v = vecItem.begin(); v != vecItem.end();)
  {
    for (auto m = mapReduce.begin(); m != mapReduce.end(); ++m)
    {
      if (v->count() == 0)
        break;

      BasePackage* pPack = getPackage(m->first);
      if (pPack == nullptr)
      {
        XERR << "[包裹-物品删除-vecitem]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "在" << setPack << "包裹中删除" << vecItem << "失败" << m->first << "异常" << XEND;
        return false;
      }

      TVecItemInfo& vec = m->second;
      DWORD dwCount = pPack->getItemCount(v->id(), eMethod);
      ItemInfo oItem;
      oItem.set_id(v->id());

      if (dwCount >= v->count())
      {
        oItem.set_count(v->count());
        v->set_count(0);
      }
      else
      {
        oItem.set_count(dwCount);
        v->set_count(v->count() - dwCount);
      }

      if (oItem.count() != 0)
        combinItemInfo(vec, oItem);
    }

    if (v->count() == 0)
      v = vecItem.erase(v);
    else
      ++v;
  }

  if (vecItem.empty() == false)
  {
    XDBG << "[包裹-物品删除-vecitem]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "在" << setPack << "包裹中删除" << vecCopy << "失败,其中" << vecItem << "数量不足" << XEND;
    return false;
  }
  return true;
}

bool Package::canReduce(EPackType eType) const
{
  auto v = find_if(TVEC_REDUCE_PACKS.begin(), TVEC_REDUCE_PACKS.end(), [eType](EPackType e) -> bool{
    return e == eType;
  });
  return v != TVEC_REDUCE_PACKS.end();
}

bool Package::makeReduceList(const string& guid, DWORD count, TMapReduce& mapReduce, ECheckMethod eMethod, const TSetDWORD& setPack)
{
  mapReduce.clear();
  for (auto &s : setPack)
  {
    EPackType eType = static_cast<EPackType>(s);
    BasePackage* pPack = getPackage(eType);
    if (pPack == nullptr)
      return false;
    ItemBase* pBase = pPack->getItem(guid);
    if (pBase == nullptr)
      continue;
    if (eMethod == ECHECKMETHOD_NONORMALEQUIP)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
      if (pEquip != nullptr && pEquip->canBeMaterial() == false)
        continue;
    }
    else if (eMethod == ECHECKMETHOD_UPGRADE)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
      if (pEquip != nullptr && pEquip->canBeUpgradeMaterial() == false)
        continue;
    }
    if (pBase->getCount() < count)
      continue;

    ItemInfo oItem;
    oItem.set_guid(guid);
    oItem.set_count(count);
    combinItemInfo(mapReduce[eType], oItem);
  }

  return mapReduce.empty() == false;
}

bool Package::colorBody(EPackType pkgType, DWORD color)
{
  auto getBodyFromEquip = [&](BasePackage* pPkg)->ItemEquip* {
    if (pPkg == nullptr)
      return nullptr;
    ItemEquip* pEquip = pPkg->getEquip(EEQUIPPOS_HEAD);
    if (!pEquip)
      return nullptr;
    if (pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == false)
      return nullptr;
    if (pEquip->getCFG() == nullptr)
      return nullptr;
    if (pEquip->getCFG()->dwBody == 0)
      return nullptr;
    return pEquip;
  };

  BasePackage* pPkg = nullptr;
  if (pkgType == EPACKTYPE_FASHIONEQUIP)
    pPkg = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(pkgType));
  else if (pkgType == EPACKTYPE_EQUIP)
    pPkg = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(pkgType));
  else
    return false;
  ItemEquip* pEquip = getBodyFromEquip(pPkg);

  if (pEquip == nullptr)
  {
    XERR << "[时装-染色] 找不到时装 "<<m_pUser->accid <<m_pUser->id <<m_pUser->name <<"color"<<color<< XEND;
    return false;
  }

  if (pEquip->getBodyColor() == color)
  {
    XERR << "[时装-染色] 颜色已经染过 " << m_pUser->accid << m_pUser->id << m_pUser->name << "color" << color << XEND;
    return false;
  }  
  pEquip->setBodyColor(color);
  
  m_pUser->setDataMark(EUSERDATATYPE_BODY);
  m_pUser->refreshDataAtonce();
  XLOG <<"[时装-染色] 染成成功" << m_pUser->accid << m_pUser->id << m_pUser->name << "color" << color <<"装备id"<<pEquip->getTypeID() << XEND;
  return true;
}

void Package::refreshDeleteTimeItems()
{
  for (int i = EPACKTYPE_MAIN; i < EPACKTYPE_MAX; ++i)
  {
    if (i == EPACKTYPE_TEMP_MAIN)
      continue;
    BasePackage* pack = getPackage(static_cast<EPackType>(i));
    if(pack == nullptr)
      continue;
    pack->refreshDeleteTimeItems();
  }
}

void Package::setOperItem(const string& guid)
{
  m_strOperItem = guid;
  XLOG << "[包裹-操作道具]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置操作道具" << m_strOperItem << XEND;
}

bool Package::hasEquipedCard(DWORD cardid)
{
  BasePackage* pack = getPackage(EPACKTYPE_EQUIP);
  if (pack == nullptr)
    return false;

  TVecSortItem pVecItems;
  pack->getEquipItems(pVecItems);
  if (pVecItems.empty())
    return false;
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  for (auto v = pVecItems.begin(); v != pVecItems.end(); ++v)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*> (*v);
    if (!pEquip)
      continue;

    if (pEquip->getIndex() != rCFG.getValidEquipPos(static_cast<EEquipPos>(pEquip->getIndex()), pEquip->getEquipType()))
      continue;

    const TMapEquipCard& cards = pEquip->getCardList();
    for (auto &c : cards)
    {
      if (c.second.second == cardid)
        return true;
    }
  }

  return false;
}

bool Package::hasWeddingManual(QWORD qwWeddingID)
{
  if (qwWeddingID == 0)
    return false;
  BasePackage* pack = getPackage(EPACKTYPE_MAIN);
  if (pack == nullptr)
    return false;
  const TSetItemBase& items = pack->getItemBaseList(MiscConfig::getMe().getWeddingMiscCFG().dwInvitationItemID);
  if (items.empty())
    return false;
  for (auto &s : items)
  {
    ItemWedding* pWeddingItem = dynamic_cast<ItemWedding*> (s);
    if (pWeddingItem == nullptr)
      continue;
    const WeddingData& rData = pWeddingItem->getWeddingData();
    if (rData.id() == qwWeddingID)
      return true;
  }
  return false;
}

bool Package::giveWeddingDress(Cmd::GiveWeddingDressCmd& rev)
{
  BasePackage* pPkg = getPackage(EPACKTYPE_MAIN);
  if (!pPkg)
    return false;

  //目前只支持装备
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pPkg->getItem(rev.guid()));
  if (!pEquip)
    return false;
  const SItemCFG* pCFG = pEquip->getCFG();
  if (pCFG == nullptr)
    return false;

  if (!pEquip->canGive())
  {
    XERR <<"[婚纱-赠送] 被赠送一次了，不能再赠送" <<m_pUser->id <<"guid"<<rev.guid() <<pEquip->getTypeID() << XEND;
    return false;
  }

  //check deposit
  const ShopItem* pSItem = ShopConfig::getMe().getShopItemByItemId(pEquip->getTypeID());
  if(!pSItem)
    return false;

  QWORD qwCostDeposit = pSItem->moneycount() * 10000;
  if(!m_pUser->getDeposit().checkQuota(qwCostDeposit))
  {
    XERR << "[婚纱-赠送] 赠送额度不够" << m_pUser->accid << m_pUser->id << m_pUser->name << "cost"<< qwCostDeposit << "left" << m_pUser->getDeposit().getQuota() << XEND;
    return false;
  }

  ItemData weddingDress;
  pEquip->toItemData(&weddingDress);

  //set senderdate  
  weddingDress.mutable_sender()->set_charid(m_pUser->id);
  weddingDress.mutable_sender()->set_name(m_pUser->name);
  
  TVecItemData vecItemData;
  vecItemData.push_back(weddingDress);

  ItemData loveLetter;
  ItemInfo* pItemInfo = loveLetter.mutable_base();
  if (!pItemInfo)
    return false;
  pItemInfo->set_id(MiscConfig::getMe().getWeddingMiscCFG().dwDressLetterId);
  pItemInfo->set_count(1);
  pItemInfo->set_source(ESOURCE_WEDDINGDRESS_GIVE);
  LoveLetterData* pLoveData = loveLetter.mutable_letter();
  if (!pLoveData)
    return false;
  pLoveData->set_sendusername(m_pUser->name);
  pLoveData->set_content(rev.content());

  vecItemData.push_back(loveLetter);
  /*TVecItemInfo vecItemInfo;
  if (!MailManager::getMe().sendMail(rev.receiverid(), m_pUser->id, m_pUser->name, "婚纱赠送", "婚纱赠送", EMAILTYPE_NORMAL, 0, vecItemInfo, vecItemData, EMAILATTACHTYPE_ITEM, false, MsgParams(), 0,  0,  0, false))
    return false;*/

  MsgParams oParams;
  oParams.addString(m_pUser->name);
  oParams.addString(pCFG->strNameZh);
  if (MailManager::getMe().sendMail(rev.receiverid(), m_pUser->id, m_pUser->name, "", "", EMAILTYPE_NORMAL, MAIL_WEDDING_SEND, TVecItemInfo{}, vecItemData, EMAILATTACHTYPE_ITEM, false, oParams) == false)
  {
    XERR << "[婚纱-赠送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "赠送失败, 邮件" << MAIL_WEDDING_SEND << "发送失败" << XEND;
    return false;
  }

  //remove equip
  pPkg->reduceItem(rev.guid(), ESOURCE_WEDDINGDRESS_GIVE);
  m_pUser->getDeposit().subQuota(qwCostDeposit, EQuotaType_C_WeddingDress);

  MsgManager::sendMsg(m_pUser->id, 9631);

  XLOG << "[婚纱-赠送]" << m_pUser->id << "guid" << rev.guid() << "接收者" << rev.receiverid() << XEND;
  return true;

}

bool Package::hasPet(DWORD dwPetID, DWORD dwBaseLv, DWORD dwFriendLv)
{
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();
  TSetDWORD setPacks = rCFG.getPackFunc(EPACKFUNC_PETWORK);

  for (auto &s : setPacks)
  {
    BasePackage* pPack = getPackage(static_cast<EPackType>(s));
    if (pPack == nullptr)
      continue;
    for (auto &m : pPack->m_mapID2Item)
    {
      ItemEgg* pEgg = dynamic_cast<ItemEgg*>(m.second);
      if (pEgg == nullptr)
        continue;
      if (dwPetID != 0 && pEgg->getPetID() != dwPetID)
        continue;
      if (pEgg->getLevel() >= dwBaseLv && pEgg->getFriendLv() >= dwFriendLv)
        return true;
    }
  }

  return false;
}

void Package::resetPetWorkItem()
{
  if (m_pUser->getMenu().isOpen(EMENUID_PET_WORK_MANUAL) == false)
    return;

  DWORD dwCount = 0;
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    BasePackage* pPack = getPackage(static_cast<EPackType>(i));
    if (pPack == nullptr)
      continue;
    dwCount = pPack->getItemCount(ITEM_PET_WORK);
    if (dwCount != 0)
      return;
  }

  ItemData oData;
  oData.mutable_base()->set_id(ITEM_PET_WORK);
  oData.mutable_base()->set_count(1);
  if (addItem(oData, EPACKMETHOD_NOCHECK) == false)
  {
    XERR << "[包裹-打工手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置打工手册失败,添加" << ITEM_PET_WORK << "失败" << XEND;
    return;
  }

  XLOG << "[包裹-打工手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置打工手册成功" << XEND;
}

void Package::resetFoodPackItem()
{
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置料理包失败,未找到" << EPACKTYPE_MAIN << XEND;
    return;
  }

  bool bOpen = m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK);
  DWORD dwCount = pMainPack->getItemCount(ITEM_FOOD_PACK);
  if (bOpen)
  {
    if (dwCount > 0)
      return;

    ItemData oData;
    oData.mutable_base()->set_id(ITEM_FOOD_PACK);
    oData.mutable_base()->set_count(1);
    if (pMainPack->addItem(oData, EPACKMETHOD_NOCHECK) == false)
    {
      XERR << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置料理包失败,添加" << ITEM_FOOD_PACK << "失败" << XEND;
      return;
    }
  }
  else
  {
    if (dwCount <= 0)
      return;
    pMainPack->reduceItem(ITEM_FOOD_PACK, ESOURCE_GM, dwCount);
  }
  XLOG << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置料理包成功" << XEND;
}

void Package::resetFoodItem()
{
  if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == false)
    return;

  BasePackage* pFoodPack = getPackage(EPACKTYPE_FOOD);
  if (pFoodPack == nullptr)
  {
    XERR << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置料理道具失败,未找到" << EPACKTYPE_FOOD << XEND;
    return;
  }

  const SFoodMiscCFG& rCFG = MiscConfig::getMe().getFoodCfg();
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    if (i == EPACKTYPE_STORE || i == EPACKTYPE_FOOD)
      continue;

    EPackType eType = static_cast<EPackType>(i);
    BasePackage* pPack = getPackage(eType);
    if (pPack == nullptr)
    {
      XERR << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << eType << "重置料理道具失败,未找到该包裹" << XEND;
      continue;
    }

    TSetItemBase setInvalidItems;
    auto func = [&](ItemBase* pBase)
    {
      if (pBase == nullptr)
        return;
      if (pBase->getCFG() == nullptr)
      {
        XERR << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "从" << eType << "重置料理道具" << pBase->getTypeID() << "到" << EPACKTYPE_FOOD << "失败,该道具未包含正确的配置" << XEND;
        return;
      }
      if (rCFG.isFoodItem(pBase->getCFG()->eItemType) == false)
        return;

      ItemData oData;
      pBase->toItemData(&oData);
      addPackMoveItem(EPACKTYPE_FOOD, oData);
      setInvalidItems.insert(pBase);
      XLOG << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "从" << eType << "重置料理道具" << oData.ShortDebugString() << "到" << EPACKTYPE_FOOD << "成功,转移到非法道具列表" << XEND;
    };
    pPack->foreach(func);

    for (auto &s : setInvalidItems)
      pPack->reduceItem(s->getGUID(), ESOURCE_GM, s->getCount());
  }

  XLOG << "[包裹-料理包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置料理道具" << XEND;
  resetPackMoveItem();
}

void Package::resetQuestManualItem()
{
  if (m_pUser->getMenu().isOpen(EMENUID_QUEST_MANUAL) == false)
    return;

  DWORD dwCount = 0;
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    BasePackage* pPack = getPackage(static_cast<EPackType>(i));
    if (pPack == nullptr)
      continue;
    dwCount = pPack->getItemCount(ITEM_QUEST_MANUAL);
    if (dwCount != 0)
      return;
  }

  ItemData oData;
  oData.mutable_base()->set_id(ITEM_QUEST_MANUAL);
  oData.mutable_base()->set_count(1);
  if (addItem(oData, EPACKMETHOD_NOCHECK) == false)
  {
    XERR << "[包裹-任务手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置任务手册失败,添加" << ITEM_QUEST_MANUAL << "失败" << XEND;
    return;
  }

  XLOG << "[包裹-任务手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置任务手册成功" << XEND;
}

void Package::resetPetPackItem()
{
  DWORD dwItemID = ITEM_PET_PACK;
  DWORD dwCount = 0;
  for (int i = EPACKTYPE_MIN + 1; i < EPACKTYPE_MAX; ++i)
  {
    BasePackage* pPack = getPackage(static_cast<EPackType>(i));
    if (pPack == nullptr)
      continue;
    dwCount = pPack->getItemCount(dwItemID);
    if (dwCount != 0)
      return;
  }

  ItemData oData;
  oData.mutable_base()->set_id(dwItemID);
  oData.mutable_base()->set_count(1);
  if (addItem(oData, EPACKMETHOD_NOCHECK) == false)
  {
    XERR << "[包裹-宠物包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置宠物包道具失败, 添加道具" << oData.ShortDebugString() << "失败" << XEND;
    return;
  }

  XLOG << "[包裹-宠物包]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置宠物包道具成功" << XEND;
}

bool Package::cardDecompose(const map<string, DWORD>& cardguids, TVecItemInfo& outitem)
{
  BasePackage* pack = getPackage(EPACKTYPE_MAIN);
  if (!pack)
    return false;

  DWORD price = MiscConfig::getMe().getCardMiscCFG().dwDecomposePriceCount, moneycnt = 0, itemcnt = 0;
  outitem.clear();
  stringstream ss;
  for (auto& cardguid : cardguids)
  {
    if (cardguid.second <= 0)
      return false;
    if (!checkItemCount(cardguid.first, cardguid.second, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EXCHANGE))
      return false;
  }

  for (auto& cardguid : cardguids)
  {
    ItemCard* card = dynamic_cast<ItemCard*>(getItem(cardguid.first));
    if (!card || !card->getCFG() || card->getCount() < cardguid.second)
      return false;
    if (card->getCFG()->hasCardFunc(ECARDFUNC_NODECOMPOSE))
    {
      XERR << "[卡片分解]" << m_pUser->accid << m_pUser->id << m_pUser->name << "卡片:" << card->getTypeID() << "卡片不可分解" << XEND;
      return false;
    }

    for (DWORD i = 0; i < cardguid.second; ++i)
    {
      DWORD count = MiscConfig::getMe().getCardMiscCFG().getDecomposeItemCount(card->getCFG()->eQualityType);
      if (count <= 0)
      {
        XERR << "[卡片分解]" << m_pUser->accid << m_pUser->id << m_pUser->name << "卡片:" << card->getGUID() << card->getTypeID() << card->getCFG()->dwCardType << "计算分解产出道具数量出错" << XEND;
        return false;
      }
      itemcnt += count;
      ss << " 卡片: " << card->getGUID() << " " << card->getTypeID() << " " << card->getCFG()->dwCardType << " 道具: " << count;
    }

    moneycnt += price * cardguid.second;
  }

  ItemInfo money;
  money.set_id(MiscConfig::getMe().getCardMiscCFG().dwDecomposePriceID);
  money.set_count(moneycnt);
  if (!pack->checkItemCount(money.id(), money.count()))
    return false;
  pack->reduceItem(money.id(), ESOURCE_CARD_DECOMPOSE, money.count());
  for (auto& cardguid : cardguids)
    reduceItem(cardguid.first, cardguid.second, ESOURCE_CARD_DECOMPOSE, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_EXCHANGE);

  ItemInfo item;
  item.set_id(MiscConfig::getMe().getCardMiscCFG().dwDecomposeItemID);
  item.set_count(itemcnt);
  outitem.push_back(item);
  addItem(item, EPACKMETHOD_AVAILABLE, false, false, false);

  XLOG << "[卡片分解]" << m_pUser->accid << m_pUser->id << m_pUser->name << ss.str() << "总共获得:" << item.id() << itemcnt << "成功" << XEND;
  return true;
}

void Package::onAddSkill(DWORD dwSkillID)
{
  checkResetRestoreBookItem(dwSkillID);
}

void Package::checkResetRestoreBookItem(DWORD dwSkillID)
{
  const SRecordCFG& stRecordCFG = MiscConfig::getMe().getMiscRecordCFG();
  if (dwSkillID != stRecordCFG.dwOpenNoviceSkillID)
  {
    return;
  }
  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-存档物品]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置存档物品失败,未找到" << EPACKTYPE_MAIN << XEND;
    return;
  }

  if (pMainPack->getItemCount(stRecordCFG.dwRestoreBookItemID) != 0)
    return;

  ItemData oData;
  oData.mutable_base()->set_id(stRecordCFG.dwRestoreBookItemID);
  oData.mutable_base()->set_count(1);
  if (pMainPack->addItem(oData, EPACKMETHOD_NOCHECK) == false)
  {
    XERR << "[包裹-存档物品]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置存档物品失败,添加" << stRecordCFG.dwRestoreBookItemID << "失败" << XEND;
    return;
  }

  XLOG << "[包裹-存档物品]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置存档物品成功,添加" << stRecordCFG.dwRestoreBookItemID << XEND;
}

EError Package::quickStoreItem(const QuickStoreItemCmd& cmd)
{
  if (cmd.items_size() == 0)
  {
    MsgManager::sendMsg(m_pUser->id, 25455);
    return EERROR_SUCCESS;
  }

  TSetString setIDs;
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    const ItemInfo& rItem = cmd.items(i);
    auto s = setIDs.find(rItem.guid());
    if (s != setIDs.end())
    {
      XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入失败,guid :" << rItem.guid() << "重复了" << XEND;
      return EERROR_FAIL;
    }
    setIDs.insert(rItem.guid());
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  BasePackage* pPStorePack = getPackage(EPACKTYPE_PERSONAL_STORE);
  if (pMainPack == nullptr || pPStorePack == nullptr)
  {
    XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入失败,未找到包裹" << XEND;
    return EERROR_FAIL;
  }

  TVecItemData vecItems;
  TVecItemData vecPItems;
  set<ItemBase*> setItems;
  for (auto &s : setIDs)
  {
    ItemBase* pBase = pMainPack->getItem(s);
    if (pBase == nullptr)
    {
      XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入失败,guid :" << s << "未在" << pMainPack->getPackageType() << "中找到" << XEND;
      return EERROR_FAIL;
    }
    const SItemCFG* pCFG = pBase->getCFG();
    if (pCFG == nullptr || pCFG->isNoStore(ENOSTORE_PSTORE_BARROW) == true)
    {
      XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入失败,guid :" << s << "不允许存入仓库" << XEND;
      return EERROR_FAIL;
    }
    if (pCFG->dwMaxNum <= 1)
    {
      XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入失败,guid :" << s << "不是堆叠道具" << XEND;
      return EERROR_FAIL;
    }

    ItemData oData;
    pBase->toItemData(&oData);
    oData.mutable_base()->set_source(ESOURCE_QUICKSTORE);

    vecItems.push_back(oData);
    setItems.insert(pBase);

    if (pPStorePack->getGUIDByType(pBase->getTypeID()).empty() == false)
      vecPItems.push_back(oData);
    else
    {
      XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "存入失败,item :" << oData.ShortDebugString() << "在" << EPACKTYPE_STORE << "和" << EPACKTYPE_PERSONAL_STORE << "中没有发现堆叠物品" << XEND;
      return EERROR_FAIL;
    }
  }
  if (setItems.empty() == true || vecPItems.empty() == true)
  {
    XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入失败,无道具可存" << XEND;
    return EERROR_FAIL;
  }

  for (auto &s : setItems)
    pMainPack->reduceItem(s->getGUID(), ESOURCE_QUICKSTORE, s->getCount());

  map<DWORD, ItemInfo> mapHint;
  for (auto v = vecPItems.begin(); v != vecPItems.end();)
  {
    ItemInfo& rHint = mapHint[v->base().id()];
    rHint.set_id(v->base().id());
    rHint.set_count(rHint.count() + v->base().count());

    pPStorePack->addItem(*v, EPACKMETHOD_AVAILABLE);
    if (v->base().count() == 0)
    {
      v = vecPItems.erase(v);
    }
    else
    {
      rHint.set_count(rHint.count() - v->base().count());
      ++v;
    }
  }
  if (vecPItems.empty() == false)
  {
    TVecItemData vecReturn(vecPItems);
    for (auto &v : vecReturn)
      v.mutable_base()->set_source(ESOURCE_QUICKSTORE_RETURN);
    if (pMainPack->addItem(vecReturn, EPACKMETHOD_NOCHECK) == false)
      XERR << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << EPACKTYPE_PERSONAL_STORE << "成功,部分道具未能存入,回退失败,待回退道具" << vecPItems << XEND;
    else
      XLOG << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入" << EPACKTYPE_PERSONAL_STORE << "成功,部分道具未能存入,回退成功,待回退道具" << vecPItems << XEND;
  }

  bool bFull = true;
  for (auto &m : mapHint)
  {
    if (m.second.count() == 1)
      continue;
    bFull = false;
    MsgParams params;
    params.addNumber(m.second.id());
    params.addNumber(m.second.id());
    params.addNumber(m.second.count() - 1); // 减去ItemInfo默认值
    MsgManager::sendMsg(m_pUser->id, 25425, params);
  }
  if (bFull)
    MsgManager::sendMsg(m_pUser->id, 819);

  XLOG << "[包裹-一键存入]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "存入成功,存入道具" << vecItems << XEND;
  return EERROR_SUCCESS;
}

EError Package::quickSellItem(const QuickSellItemCmd& cmd)
{
  map<string, DWORD> mapItems;
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    const SItem& rItem = cmd.items(i);
    auto m = mapItems.find(rItem.guid());
    if (m != mapItems.end())
    {
      XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,guid :" << rItem.guid() << "重复了" << XEND;
      return EERROR_FAIL;
    }
    mapItems[rItem.guid()] = rItem.count();
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,未找到" << EPACKTYPE_MAIN << XEND;
    return EERROR_FAIL;
  }

  set<ItemBase*> setSellItems;
  for (auto &m : mapItems)
  {
    ItemBase* pBase = pMainPack->getItem(m.first);
    if (pBase == nullptr)
    {
      XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,guid :" << m.first << "不存在" << XEND;
      return EERROR_FAIL;
    }
    if (pBase->getCFG() == nullptr)
    {
      XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,guid :" << m.first << "未包含正确的配置" << XEND;
      return EERROR_FAIL;
    }
    if (pBase->getCount() < m.second)
    {
      XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,guid :" << m.first << "数量" << m.second << "大于包裹中的数量" << XEND;
      return EERROR_FAIL;
    }
    if (LuaManager::getMe().call<bool>("canQuickSell", pBase->getTypeID()) == false)
    {
      XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,guid :" << m.first << "itemid :" << pBase->getTypeID() << "不允许被出售" << XEND;
      return EERROR_FAIL;
    }
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip != nullptr && pEquip->canBeQuickSell() == false)
    {
      stringstream sstr;
      sstr << "id : " << pEquip->getTypeID() << "无法出售";
      MsgManager::sendDebugMsg(m_pUser->id, sstr.str());
      XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,guid :" << m.first << "itemid :" << pBase->getTypeID() << "是装备,装备无法出售" << XEND;
      return EERROR_FAIL;
    }

    setSellItems.insert(pBase);
  }
  if (setSellItems.empty() == true)
  {
    XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败,无道具可出售" << XEND;
    return EERROR_FAIL;
  }

  DWORD dwRet = 0;
  for (auto &s : setSellItems)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(s);
    if (pEquip != nullptr)
    {
      DWORD retMoney = 0;
      if ((pEquip->getStrengthLv() != 0 || pEquip->getRefineLv() != 0) && decompose(pEquip->getGUID(), true, retMoney) == false)
      {
        XERR << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "出售失败, id :" << pEquip->getTypeID() << "分解失败" << XEND;
        return EERROR_FAIL;
      }
      dwRet += retMoney;
    }
  }

  DWORD dwPrice = 0;
  for (auto &s : setSellItems)
  {
    dwPrice += s->getCFG()->dwSellPrice;
    pMainPack->reduceItem(s->getGUID(), ESOURCE_SELL, s->getCount());
  }
  DWORD dwExtra = floor(1.0f * dwPrice * m_pUser->getAttr(EATTRTYPE_SELLDISCOUNT) / 1000.0f);
  m_pUser->addMoney(EMONEYTYPE_SILVER, dwPrice + dwExtra + dwRet, ESOURCE_SELL);

  XLOG << "[包裹-一键出售]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "出售" << cmd.ShortDebugString() << "成功,共获得 price :" << dwPrice << "extra :" << dwExtra << "ret :" << dwRet << "zeny" << XEND;
  return EERROR_SUCCESS;
}

EError Package::enchantTrans(const EnchantTransItemCmd& cmd)
{
  if (cmd.from_guid() == cmd.to_guid())
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,不能转移给自身" << XEND;
    return EERROR_FAIL;
  }

  BasePackage* pFromPack = nullptr;
  ItemEquip* pFromEquip = dynamic_cast<ItemEquip*>(getItem(cmd.from_guid(), &pFromPack));
  if (pFromEquip == nullptr)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,未找到原装备" << XEND;
    return EERROR_FAIL;
  }
  if (pFromPack == nullptr)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,未找到原装备包裹" << XEND;
    return EERROR_FAIL;
  }
  EPackType eType = pFromPack->getPackageType();
  if (eType != EPACKTYPE_MAIN && eType != EPACKTYPE_EQUIP)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,只能转移" << EPACKTYPE_MAIN << "或" << EPACKTYPE_EQUIP << "中的装备" << XEND;
    return EERROR_FAIL;
  }
  if (pFromEquip->getEnchantData().type() == EENCHANTTYPE_MIN)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,原装备未附魔" << XEND;
    return EERROR_FAIL;
  }
  if (ItemConfig::getMe().isLotteryHead(pFromEquip->getTypeID()) == false)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,原装备不是扭蛋头饰" << XEND;
    return EERROR_FAIL;
  }

  BasePackage* pToPack = nullptr;
  ItemEquip* pToEquip = dynamic_cast<ItemEquip*>(getItem(cmd.to_guid(), &pToPack));
  if (pToEquip == nullptr || pToEquip->getCFG() == nullptr)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,未找到目标装备" << XEND;
    return EERROR_FAIL;
  }
  if (pToPack == nullptr)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,未找到目标装备包裹" << XEND;
    return EERROR_FAIL;
  }
  eType = pFromPack->getPackageType();
  if (eType != EPACKTYPE_MAIN && eType != EPACKTYPE_EQUIP)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,只能转移" << EPACKTYPE_MAIN << "或" << EPACKTYPE_EQUIP << "中的装备" << XEND;
    return EERROR_FAIL;
  }
  if (ItemConfig::getMe().isLotteryHead(pToEquip->getTypeID()) == false)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,目标装备不是扭蛋头饰" << XEND;
    return EERROR_FAIL;
  }
  if (pToEquip->getCFG()->isForbid(EFORBID_ENCHANT) == true)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,目标装备不能附魔" << XEND;
    return EERROR_FAIL;
  }

  BasePackage* pMainPack = getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,未找到" << EPACKTYPE_MAIN << XEND;
    return EERROR_FAIL;
  }
  if (pMainPack->checkItemCount(ITEM_ENCHANT_TRANS, 1) == false)
  {
    XERR << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "失败,未找到" << ITEM_ENCHANT_TRANS << "附魔转移道具"<< XEND;
    return EERROR_FAIL;
  }
  pMainPack->reduceItem(ITEM_ENCHANT_TRANS, ESOURCE_ENCHANT_TRANS);

  pToEquip->getEnchantData().CopyFrom(pFromEquip->getEnchantData());
  pToEquip->getPreviewEnchantData().Clear();
  pFromEquip->getEnchantData().Clear();
  pFromEquip->getPreviewEnchantData().Clear();

  pFromPack->setUpdateIDs(pFromEquip->getGUID());
  pToPack->setUpdateIDs(pToEquip->getGUID());

  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();

  EnchantTransItemCmd ret;
  ret.CopyFrom(cmd);
  ret.set_success(true);
  PROTOBUF(ret, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[包裹-附魔转移]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "从" << cmd.from_guid() << "转移附魔到" << cmd.to_guid() << "成功,转移属性" << pToEquip->getEnchantData().ShortDebugString() << XEND;
  return EERROR_SUCCESS;
}

bool Package::clearRewardSafetyItem(DWORD id)
{
  auto it = m_mapRewardSafetyItem.find(id);
  if (it == m_mapRewardSafetyItem.end())
    return true;
  m_mapRewardSafetyItem.erase(it);
  XLOG << "[包裹-清除保底奖励记录]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "reward:" << id << "清除成功";
  return true;
}

void Package::luaAddItem(DWORD itemid, DWORD count, DWORD luaParam)
{
  ItemInfo item;
  item.set_id(itemid);
  item.set_count(count);
  addItem(item, EPACKMETHOD_AVAILABLE);
  XLOG << "[包裹-Lua添加], 玩家:" << m_pUser->name << m_pUser->id << "道具:" << itemid << "数量:" << count << "来源:" << luaParam << XEND;
}

// base package
BasePackage::BasePackage(SceneUser* pUser) : m_pUser(pUser), m_dwMaxIndex(0)
{

}

BasePackage::~BasePackage()
{
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
    SAFE_DELETE(m->second);
  m_mapID2Item.clear();
  m_mapTypeID2Item.clear();

  resetGarbageItem();
}

bool BasePackage::toClient(PackageItem& rItem)
{
  rItem.clear_data();
  rItem.set_type(getPackageType());
  rItem.set_maxslot(getMaxSlot());
  auto func = [&](ItemBase* pBase)
  {
    if (pBase == nullptr)
      return;

    ItemData* pData = rItem.add_data();
    if (pData == nullptr)
      return;

    pBase->toClientData(getPackageType(), pData, m_pUser);
  };
  foreach(func);
  return true;
}

bool BasePackage::fromData(const PackageData& rData)
{
  if (rData.type() != getPackageType())
  {
    XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << rData.ShortDebugString() << "失败,背包不匹配" << XEND;
    return false;
  }

  Package& rPackage = m_pUser->getPackage();
  bool bFoodMenu = m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK);
  EPackType ePackType = getPackageType();
  const SFoodMiscCFG& rCFG = MiscConfig::getMe().getFoodCfg();
  for (int j = 0; j < rData.items_size() ; ++j)
  {
    const ItemData& item = rData.items(j);
    if (item.base().guid().empty() == true)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,guid为空" << XEND;
      continue;
    }

    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(item.base().id());
    if (pCFG == nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,未找到配置" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      continue;
    }

    if (pCFG->eItemType == EITEMTYPE_QUESTITEM && getPackageType() != EPACKTYPE_QUEST)
    {
      rPackage.addPackMoveItem(EPACKTYPE_QUEST, item);
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "物品" << item.ShortDebugString() << "是任务道具,转移到非法任务道具列表" << XEND;
      continue;
    }
    else if (pCFG->eItemType != EITEMTYPE_QUESTITEM && getPackageType() == EPACKTYPE_QUEST)
    {
      rPackage.addPackMoveItem(EPACKTYPE_MAIN, item);
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "物品" << item.ShortDebugString() << "不是任务道具,转移到非法任务道具列表" << XEND;
      continue;
    }

    if (bFoodMenu)
    {
      if (rCFG.isFoodItem(pCFG->eItemType) == true && ePackType != EPACKTYPE_FOOD)
      {
        rPackage.addPackMoveItem(EPACKTYPE_FOOD, item);
        XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "中物品" << item.ShortDebugString() << "是料理道具,转移到非法道具列表" << XEND;
        continue;
      }
      else if (rCFG.isFoodItem(pCFG->eItemType) == false && ePackType == EPACKTYPE_FOOD)
      {
        rPackage.addPackMoveItem(EPACKTYPE_MAIN, item);
        XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "中物品" << item.ShortDebugString() << "不是料理道具,转移到非法道具列表" << XEND;
        continue;
      }
    }

    if (pCFG->eItemType == EITEMTYPE_EGG && getPackageType() != EPACKTYPE_PET)
    {
      rPackage.addPackMoveItem(EPACKTYPE_PET, item);
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "物品" << item.ShortDebugString() << "是宠物道具,转移到非法任务道具列表" << XEND;
      continue;
    }
    else if (pCFG->eItemType != EITEMTYPE_EGG && getPackageType() == EPACKTYPE_PET)
    {
      rPackage.addPackMoveItem(EPACKTYPE_MAIN, item);
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "物品" << item.ShortDebugString() << "不是宠物道具,转移到非法任务道具列表" << XEND;
      continue;
    }

    ItemBase* pBase = ItemManager::getMe().createItem(item.base());
    if (pBase == nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      continue;
    }
    pBase->fromItemData(item);
    addItemObj(pBase, true);

    if (item.base().index() > getMaxIndex())
      setMaxIndex(item.base().index());
  }
  clearUpdateIDs();
  return true;
}

DWORD BasePackage::getMaxSlot()
{
  SceneFighter* pFighter = m_pUser->getFighter();
  EPackType eType = getPackageType();
  DWORD dwSkillSlot = 0;
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  const TVecSkillSlot& vecSlot = rCFG.getPackSkillSlot(eType);
  for (auto &v : vecSlot)
  {
    if (m_pUser->getUserSceneData().isPackSkillLearned(v.dwSkillID) == false)
      continue;
    if (v.dwAttrValue != 0)
    {
      if (pFighter == nullptr)
        continue;

      EAttrType eType = static_cast<EAttrType>(RoleDataConfig::getMe().getIDByName(v.attr.c_str()));
      DWORD dwValue = pFighter->getAttrPoint(eType);
      if (dwValue < v.dwAttrValue)
        continue;
    }
    dwSkillSlot += v.dwSlot;
  }
  DWORD baseSlot = rCFG.getPackMaxSlot(eType);
  DWORD buffSlot = m_pUser->m_oBuff.getPackageSlot(getPackageType());
  DWORD depositSlot = m_pUser->getDeposit().getPackageSlot(getPackageType());
  DWORD foodSlot = m_pUser->getSceneFood().getBagSlot(getPackageType());
  DWORD dwLvSlot = ItemConfig::getMe().getPackSlotLvCFG(m_pUser->getLevel(), getPackageType());
  m_dwLastMaxSlot = baseSlot + dwSkillSlot + buffSlot + depositSlot + foodSlot + dwLvSlot;
  XDBG << "[包裹-格子]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << getPackageType() << "最大格子数 :" << m_dwLastMaxSlot << "基础 :" << baseSlot << "skill :" << dwSkillSlot << "buff :" << buffSlot << "deposit :" << depositSlot << "料理 :" << foodSlot << "等级 :" << dwLvSlot << XEND;
  return m_dwLastMaxSlot;
}

bool BasePackage::checkAddItemObj(const ItemBase* pBase)
{
  if (pBase == nullptr)
    return false;
  auto m = m_mapID2Item.find(pBase->getGUID());
  if (m != m_mapID2Item.end())
    return false;
  return m_mapID2Item.size() + 1 <= getMaxSlot();
}

void BasePackage::addItemObj(ItemBase* pBase, bool bInit /*= false*/, ESource source /*=ESOURCE_PACKAGE*/)
{
  if (!bInit && checkAddItemObj(pBase) == false)
    return;

  if (getPackageType() == EPACKTYPE_PET)
  {
    ItemEgg* pEgg = dynamic_cast<ItemEgg*>(pBase);
    if (pEgg != nullptr)
      modifyEgg(&pEgg->getEggData());
  }

  //pBase->setSource(ESOURCE_PACKAGE);
  pBase->setSource(source);
  ItemData oData;
  pBase->toItemData(&oData);

  auto m = m_mapID2Item.find(pBase->getGUID());
  if (m != m_mapID2Item.end())
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip != nullptr)
    {
      m->second->setCount(1);

      XLOG << "[包裹-实体物品添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "添加item:" << oData.ShortDebugString() << "到" << getPackageType() << "异常,发生相同GUID装备,设置堆叠为1" << XEND;
    }
    else
    {
      m->second->setCount(m->second->getCount() + pBase->getCount());
    }
    addGarbageItem(pBase);
  }
  else
  {
    m_mapID2Item[pBase->getGUID()] = pBase;
    m_mapTypeID2Item[pBase->getTypeID()].insert(pBase);
    modifyOverTime(pBase);
    if (ItemConfig::getMe().isArtifact(pBase->getType()))
      m_setArtifactGuid.insert(pBase->getGUID());
  }

  m_pUser->getEvent().onItemAdd(pBase->getGUID(), pBase->getTypeID(), source);
  m_setUpdateIDs.insert(pBase->getGUID());

  if (getPackageType() != EPACKTYPE_FASHION)
    m_pUser->getPackage().addItemCount(pBase);

  if (source == ESOURCE_TRADE || source == ESOURCE_TRADE_PUBLICITY || source == ESOURCE_GIVE)
  {
    addShowItems(pBase->GetItemInfo(), false, true, false);   //展示   
  }
  if (source != ESOURCE_PACKAGE)
  {
    itemLog(pBase->GetItemInfo(), getItemCount(pBase->getTypeID(), ECHECKMETHOD_NORMAL), source);   //平台日志
  }

  if (!bInit)
    XLOG << "[包裹-实体物品添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "添加item:" << oData.ShortDebugString() << "到" << getPackageType() << "成功" << "到期时间: " << pBase->getOverTime() << XEND;
}

ItemBase* BasePackage::getItem(const string& guid)
{
  auto m = m_mapID2Item.find(guid);
  if (m != m_mapID2Item.end())
    return m->second;

  return nullptr;
}
/*void BasePackage::getEquipIDs(TVecDWORD& vecItemID)
{
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
  {
    vecItemID.push_back(m->second->getType());
  }
}*/
void BasePackage::getEquipItems(TVecSortItem& pVecItems)
{
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
  {
    pVecItems.push_back(m->second);
  }
}

void BasePackage::clear()
{
  TVecString vecIDs;
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
    vecIDs.push_back(m->first);
  for (auto v = vecIDs.begin(); v != vecIDs.end(); ++v)
  {
    ItemBase* pItem = getItem(*v);
    if (pItem != nullptr)
      reduceItem(*v,ESOURCE_PACKAGE, pItem->getCount());
  }
}

const string& BasePackage::getGUIDByType(DWORD dwTypeID) const
{
  static const string emptyguid;
  auto m = m_mapTypeID2Item.find(dwTypeID);
  if (m == m_mapTypeID2Item.end() || m->second.empty() == true)
    return emptyguid;

  DWORD dwTime = 0;
  ItemBase* pBase = nullptr;
  for (auto s = m->second.begin(); s != m->second.end(); ++s)
  {
    if ((*s)->getCreateTime() >= dwTime)
    {
      dwTime = (*s)->getCreateTime();
      pBase = *s;
    }
  }
  return pBase == nullptr ? emptyguid : pBase->getGUID();
}

bool BasePackage::setCDByType(DWORD dwType)
{
  auto m = m_mapTypeID2Item.find(dwType);
  if (m == m_mapTypeID2Item.end())
    return false;

  for (auto &s : m->second)
  {
    const SItemCFG* pCFG = s->getCFG();
    if (pCFG == nullptr)
      continue;

    s->setCD(xTime::getCurMSec());
    m_pUser->m_oCDTime.add(s->getTypeID(), pCFG->dwCD, CD_TYPE_ITEM);
    setUpdateIDs(s->getGUID());
  }

  return true;
}

const TSetItemBase& BasePackage::getItemBaseList(DWORD dwTypeID) const
{
  static const TSetItemBase e;
  auto m = m_mapTypeID2Item.find(dwTypeID);
  if (m != m_mapTypeID2Item.end())
    return m->second;

  return e;
}

DWORD BasePackage::getItemCount(DWORD id, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/)
{
  auto m = m_mapTypeID2Item.find(id);
  if (m == m_mapTypeID2Item.end())
    return 0;

  DWORD count = 0;
  for (auto s = m->second.begin(); s != m->second.end(); ++s)
  {
    if ((*s)->getGUID() == m_pUser->getPackage().getOperItem())
      continue;
    if (eMethod == ECHECKMETHOD_NONORMALEQUIP)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
      if (pEquip != nullptr && pEquip->canBeMaterial() == false)
        continue;
    }
    else if (eMethod == ECHECKMETHOD_UPGRADE)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
      if (pEquip != nullptr && pEquip->canBeUpgradeMaterial() == false)
        continue;
    }

    count += (*s)->getCount();
  }

  return count;
}

bool BasePackage::hasItem(DWORD id, DWORD minstrengthlv, DWORD maxstrengthlv)
{
  if (id == 0)
  {
    for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m->second);
      if (pEquip == nullptr)
        continue;

      if (pEquip->getStrengthLv() >= minstrengthlv && pEquip->getStrengthLv() <= maxstrengthlv)
        return true;
    }

    return false;
  }

  auto m = m_mapTypeID2Item.find(id);
  if (m == m_mapTypeID2Item.end())
    return false;

  for (auto s = m->second.begin(); s != m->second.end(); ++s)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
    if (pEquip == nullptr)
      continue;

    if (pEquip->getStrengthLv() >= minstrengthlv && pEquip->getStrengthLv() <= maxstrengthlv)
      return true;
  }

  return false;
}

bool BasePackage::checkAddItem(const ItemInfo& rInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/)
{
  ItemData oData;
  oData.mutable_base()->CopyFrom(rInfo);
  return checkAddItem(oData, eMethod);
}

bool BasePackage::checkAddItem(const TVecItemInfo& vecInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/)
{
  if (vecInfo.empty() == true)
    return false;
  TVecItemData vecData;
  vecData.resize(vecInfo.size());
  for (size_t i = 0; i < vecData.size(); ++i)
    vecData[i].mutable_base()->CopyFrom(vecInfo[i]);
  return checkAddItem(vecData, eMethod);
}

bool BasePackage::checkAddItem(const ItemData& rData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rData.base().id());
  if (pCFG == nullptr)
    return false;

  if (ItemConfig::getMe().isPackCheck(pCFG->eItemType) == false)
  {
    if (pCFG->eItemType == EITEMTYPE_PORTRAIT || pCFG->eItemType == EITEMTYPE_FRAME)
      return m_pUser->getPortrait().checkAddItems(rData.base().id());
    else if (pCFG->eItemType == EITEMTYPE_HAIR || pCFG->eItemType == EITEMTYPE_HAIR_MALE || pCFG->eItemType == EITEMTYPE_HAIR_FEMALE)
      return m_pUser->getHairInfo().checkAddHair(rData.base().id());
    else if (pCFG->eItemType == EITEMTYPE_HONOR)
      return m_pUser->getTitle().checkAddTitle(rData.base().id());
    else if (pCFG->eItemType == EITEMTYPE_EYE_MALE || pCFG->eItemType == EITEMTYPE_EYE_FEMALE)
      return m_pUser->getEye().checkAddEye(rData.base().id());
    return true;
  }

  if (eMethod == EPACKMETHOD_NOCHECK)
    return true;
  if (eMethod == EPACKMETHOD_CHECK_WITHPILE)
  {
    DWORD dwCount = rData.base().count();
    auto m = m_mapTypeID2Item.find(rData.base().id());
    if (m != m_mapTypeID2Item.end())
    {
      for (auto &s : m->second)
      {
        if (dwCount <= 0)
          break;

        /*if (rData.base().guid().empty() == false && s->getGUID() != rData.base().guid())
          continue;*/

        DWORD dwLeft = pCFG->dwMaxNum > s->getCount() ? pCFG->dwMaxNum - s->getCount() : 0;
        if (dwLeft == 0)
          continue;

        if (dwCount < dwLeft)
        {
          dwCount = 0;
          continue;
        }

        dwCount -= dwLeft;
      }
    }
    DWORD dwSlot = dwCount / pCFG->dwMaxNum + ((dwCount % pCFG->dwMaxNum) != 0 ? 1 : 0);
    return dwSlot == 0 ? true : m_mapID2Item.size() + dwSlot <= getMaxSlot();
  }
  if (eMethod == EPACKMETHOD_CHECK_NOPILE)
  {
    DWORD dwSlot = rData.base().count() / pCFG->dwMaxNum + ((rData.base().count() % pCFG->dwMaxNum) != 0 ? 1 : 0);
    return m_mapID2Item.size() + dwSlot <= getMaxSlot();
  }
  if (eMethod == EPACKMETHOD_AVAILABLE)
  {
    DWORD dwCount = rData.base().count();
    auto m = m_mapTypeID2Item.find(rData.base().id());
    if (m != m_mapTypeID2Item.end())
    {
      for (auto &s : m->second)
      {
        if (dwCount <= 0)
          break;

        /*if (rData.base().guid().empty() == false && s->getGUID() != rData.base().guid())
          continue;*/

        DWORD dwLeft = pCFG->dwMaxNum > s->getCount() ? pCFG->dwMaxNum - s->getCount() : 0;
        if (dwLeft == 0)
          continue;

        return true;
      }
    }
    return m_mapID2Item.size() < getMaxSlot();
  }

  return false;
}

bool BasePackage::checkAddItem(const TVecItemData& vecData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/)
{
  DWORD dwSlot = 0;
  TSetString setExclude;
  for (auto &v : vecData)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v.base().id());
    if (pCFG == nullptr)
    {
      XERR << "[包裹-检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << v.base().ShortDebugString() << "失败,未在Table_Item.txt表中找到" << XEND;
      return false;
    }

    if (ItemConfig::getMe().isPackCheck(pCFG->eItemType) == false)
    {
      if (checkAddItem(v) == false)
        return false;

      continue;
    }

    if (pCFG->dwMaxNum == 0)
      return false;

    if (eMethod == EPACKMETHOD_NOCHECK)
      continue;

    if (eMethod == EPACKMETHOD_CHECK_WITHPILE)
    {
      DWORD dwCount = v.base().count();
      auto m = m_mapTypeID2Item.find(v.base().id());
      if (m != m_mapTypeID2Item.end())
      {
        for (auto &s : m->second)
        {
          auto check = setExclude.find(s->getGUID());
          if (check != setExclude.end())
            continue;

          setExclude.insert(s->getGUID());

          if (dwCount <= 0)
            continue;

          /*if (v.base().guid().empty() == false && s->getGUID() != v.base().guid())
            continue;*/

          DWORD dwLeft = pCFG->dwMaxNum > s->getCount() ? pCFG->dwMaxNum - s->getCount() : 0;
          if (dwLeft == 0)
            continue;

          if (dwCount < dwLeft)
          {
            dwCount = 0;
            continue;
          }

          dwCount -= dwLeft;
        }
      }

      dwSlot += dwCount / pCFG->dwMaxNum + ((dwCount % pCFG->dwMaxNum) != 0 ? 1 : 0);
      continue;
    }

    if (eMethod == EPACKMETHOD_CHECK_NOPILE)
    {
      dwSlot += v.base().count() / pCFG->dwMaxNum + ((v.base().count() % pCFG->dwMaxNum) != 0 ? 1 : 0);
      continue;
    }

    if (eMethod == EPACKMETHOD_AVAILABLE)
    {
      DWORD dwCount = v.base().count();
      auto m = m_mapTypeID2Item.find(v.base().id());
      if (m != m_mapTypeID2Item.end())
      {
        for (auto &s : m->second)
        {
          auto check = setExclude.find(s->getGUID());
          if (check != setExclude.end())
            continue;

          setExclude.insert(s->getGUID());

          if (dwCount <= 0)
            continue;

          /*if (v.base().guid().empty() == false && s->getGUID() != v.base().guid())
            continue;*/

          DWORD dwLeft = pCFG->dwMaxNum > s->getCount() ? pCFG->dwMaxNum - s->getCount() : 0;
          if (dwLeft == 0)
            continue;

          return true;
        }
      }
      dwSlot += v.base().count() / pCFG->dwMaxNum + ((v.base().count() % pCFG->dwMaxNum) != 0 ? 1 : 0);
      continue;
    }

    return false;
  }

  return dwSlot == 0 ? true : m_mapID2Item.size() + dwSlot <= getMaxSlot();
}

bool BasePackage::addItem(ItemInfo& rInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  if (checkAddItem(rInfo, eMethod) == false)
  {
    XERR << "[包裹-物品添加-ItemInfo]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << getPackageType() << "检查未通过" << XEND;
    return false;
  }
  ItemData oData;
  oData.mutable_base()->CopyFrom(rInfo);
  if (addItem(oData, eMethod, bShow, bInform, bForceShow) == false)
  {
    XERR << "[包裹-物品添加-ItemInfo]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << getPackageType() << "失败" << XEND;
    return false;
  }
  rInfo.set_count(oData.base().count());
  return true;
}

bool BasePackage::addItem(TVecItemInfo& vecInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  if (checkAddItem(vecInfo, eMethod) == false)
  {
    XERR << "[包裹-物品添加-TVecItemInfo]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << getPackageType() << "检查未通过, items:";
    for(auto& v : vecInfo)
      XERR << v.id() << "," << v.count() << ";";
    XERR << XEND;
    return false;
  }

  TVecItemData vecData;
  vecData.resize(vecInfo.size());
  for (size_t i = 0; i < vecData.size(); ++i)
    vecData[i].mutable_base()->CopyFrom(vecInfo[i]);
  if (addItem(vecData, eMethod, bShow, bInform, bForceShow) == false)
  {
    XERR << "[包裹-物品添加-TVecItemInfo]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << getPackageType() << "失败" << XEND;
    return false;
  }
  vecInfo.clear();
  for (auto &v : vecData)
    vecInfo.push_back(v.base());
  return true;
}

bool BasePackage::addItem(ItemData& rData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  if (checkAddItem(rData, eMethod) == false)
    return false;

  // get item config
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(rData.base().id());
  if (pCFG == nullptr)
  {
    XERR << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "失败,未在Table_Item.txt中找到" << XEND;
    return false;
  }

  // 一定时间内道具获取上限
  if (pCFG->eGetLimitType != EITEMGETLIMITTYPE_NONE)
  {
    DWORD dwCount = rData.base().count();
    rData.mutable_base()->set_count(m_pUser->getPackage().updateVarRealGetCnt(pCFG, rData.base().id(), rData.base().count(), rData.base().source(), getPackageType()));
    if (dwCount != rData.base().count())
    {
      XLOG << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "根据当日上限修正原" << dwCount << "个为" << rData.base().count() << "个" << XEND;
    }
    if (rData.base().count() <= 0)
      return true;
  }
  DWORD dwRealCount = rData.base().count();

  // show item
  if (/*pCFG->eItemType != EITEMTYPE_MANUALPOINT && pCFG->eItemType != EITEMTYPE_CONTRIBUTE && */pCFG->eItemType != EITEMTYPE_ASSET)
    addShowItems(rData.base(), bShow, bInform, bForceShow);

  // add special item
  if (pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_DIAMOND) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_SILVER) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GOLD) ||
      //pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GARDEN) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_CONTRIBUTE) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GUILDASSET) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_PVPCOIN) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_LOTTERY) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GUILDHONOR) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_DEADCOIN)
     )
      //pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_FRIENDSHIP))
  {
    m_pUser->addMoney(static_cast<EMoneyType>(pCFG->eItemType), rData.base().count(), rData.base().source());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_QUOTA)
  {
    m_pUser->getDeposit().addQuota(rData.base().count(), Cmd::EQuotaType_G_Reward);
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_BASEEXP)
  {
    m_pUser->addBaseExp(rData.base().count(), rData.base().source());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_JOBEXP)
  {
    m_pUser->addJobExp(rData.base().count(), rData.base().source());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_PURIFY)
  {
    m_pUser->addPurify(rData.base().count());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_QUESTITEMCOUNT)
  {
    m_pUser->getEvent().onItemAdd(rData.base());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_PORTRAIT || pCFG->eItemType == EITEMTYPE_FRAME)
  {
    m_pUser->getPortrait().addNewItems(rData.base().id());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_MANUALSPOINT)
  {
    m_pUser->getManual().addSkillPoint(rData.base().count());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_MANUALPOINT)
  {
    m_pUser->getManual().addPoint(rData.base().count(), EMANUALTYPE_MIN, 0);
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_HONOR)
  {
    m_pUser->getTitle().addTitle(rData.base().id());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_HAIR || pCFG->eItemType == EITEMTYPE_HAIR_MALE || pCFG->eItemType == EITEMTYPE_HAIR_FEMALE)
  {
    m_pUser->getHairInfo().addNewHair(rData.base().id());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_COOKER_EXP)
  {
    m_pUser->getSceneFood().addCookerExp(rData.base().count());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_EGG)
  {
    modifyEgg(rData.mutable_egg());
  }
  else if (pCFG->eItemType == EITEMTYPE_EYE_MALE || pCFG->eItemType == EITEMTYPE_EYE_FEMALE)
  {
    m_pUser->getEye().addNewEye(rData.base().id());
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_GOLDAPPLE)
  {
    PollyScene* pPoliScene = dynamic_cast<PollyScene*> (m_pUser->getScene());
    if (pPoliScene)
    {
      DWORD score = pPoliScene->getScore(m_pUser);
      pPoliScene->setScore(m_pUser, rData.base().count() + score);
    }
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_GETSKILL)
  {
    if (m_pUser->getTransform().getTransformType() == ETRANSFROM_POLIFIRE)
    {
      const SPoliFireCFG& cfg = MiscConfig::getMe().getPoliFireCFG();
      auto it = cfg.mapItem2Skill.find(rData.base().id());
      if (it != cfg.mapItem2Skill.end())
      {
        m_pUser->getTransform().addSkill(it->second);
      }
    }
    else if (m_pUser->getTransform().getTransformType() == ETRANSFORM_ALTMAN)
    {
      bool ret = false;
      const SAltmanCFG& cfg = MiscConfig::getMe().getAltmanCFG();
      auto it = cfg.mapItem2Skill.find(rData.base().id());
      if (it != cfg.mapItem2Skill.end())
      {
        ret = m_pUser->getTransform().addSkill(it->second);
      }
      if(ret == false)
        return false;
    }
    rData.mutable_base()->set_count(0);
    return true;
  }
  else if (pCFG->eItemType == EITEMTYPE_PICKEFFECT || pCFG->eItemType == EITEMTYPE_PICKEFFECT_1)
  {
    GMCommandRuler::getMe().execute(m_pUser, pCFG->oPickGMData);
    rData.mutable_base()->set_count(0);
    return true;
  }

  auto additem = [&](const ItemData& rData)
  {
    string guid = rData.base().guid();
    if (guid.empty() == true)
    {
      guid = GuidManager::getMe().newGuidStr(m_pUser->getZoneID(), m_pUser->getUserSceneData().getOnlineMapID());
      if (ItemManager::getMe().isGUIDValid(guid) == false)
      {
        XERR << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "失败,guid :" << guid << "不合法" << XEND;
        return;
      }
    }

    // check duplicate
    auto o = m_mapID2Item.find(guid);
    if (o != m_mapID2Item.end())
    {
      XERR << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "失败,guid :" << guid << "已存在" << XEND;
      return;
    }

    // create item
    ItemBase* pBase = ItemManager::getMe().createItem(rData.base());
    if (pBase == nullptr)
    {
      XERR << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "失败,创建物品失败" << XEND;
      return;
    }
    pBase->fromItemData(rData);

    pBase->setCreateTime(xTime::getCurSec());
    pBase->setNew(true);
    pBase->setHint(!m_pUser->getPackage().isHint(rData.base().id()));
    pBase->setGUID(guid);
    pBase->setIndex(getNextIndex());
    pBase->setCount(rData.base().count() == 0 ? 1 : rData.base().count());

    QWORD qwDestTime = m_pUser->m_oCDTime.getCDTime(CD_TYPE_ITEM, pBase->getTypeID());
    if (qwDestTime != 0)
      pBase->setCD(qwDestTime - pCFG->dwCD);

    // modify overtime
    modifyOverTime(pBase);

    // add guid to manager
    ItemManager::getMe().addGUID(guid);

    // insert to list
    m_mapID2Item[pBase->getGUID()] = pBase;
    m_mapTypeID2Item[pBase->getTypeID()].insert(pBase);
    if (ItemConfig::getMe().isArtifact(pBase->getType()))
      m_setArtifactGuid.insert(pBase->getGUID());

    // save data and notify
    m_setUpdateIDs.insert(pBase->getGUID());

    // check item
    m_pUser->getPackage().refreshEnableBuffs(pBase);
    m_pUser->getPackage().addItemCount(pBase);

    // if isFashion refresh attr
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip != nullptr && ItemManager::getMe().isFashion(pEquip->getEquipType()) == true)
    {
      m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
      m_pUser->refreshDataAtonce();
    }
    XLOG << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "成功,单独创建一个堆叠为" << rData.base().count() << "个的道具" 
      << "到期时间: " << pBase->getOverTime() << XEND;
  };

  // find same type
  auto m = m_mapTypeID2Item.find(rData.base().id());
  if (m != m_mapTypeID2Item.end())
  {
    for (auto &s : m->second)
    {
      if (pCFG->dwMaxNum == 1)
        continue;

      DWORD dwLeft = pCFG->dwMaxNum > s->getCount() ? pCFG->dwMaxNum - s->getCount() : 0;
      if (dwLeft == 0)
        continue;

      if (rData.base().count() < dwLeft)
      {
        s->setCount(s->getCount() + rData.base().count());
        m_setUpdateIDs.insert(s->getGUID());
        XLOG << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "成功,在" << s->getGUID() << "堆叠了" << rData.base().count() << "个" << "总" << s->getCount() << "个" << XEND;
        rData.mutable_base()->set_count(0);
        continue;
      }

      s->setCount(s->getCount() + dwLeft);
      m_setUpdateIDs.insert(s->getGUID());
      XLOG << "[包裹-物品添加-ItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "添加" << rData.ShortDebugString() << "到" << getPackageType() << "成功,在" << s->getGUID() << "堆叠了" << dwLeft << "个" << "总" << s->getCount() << "个" << XEND;
      rData.mutable_base()->set_count(rData.base().count() - dwLeft);
    }
  }

  // add new item
  ItemData oCopy;
  oCopy.CopyFrom(rData);
  while (rData.base().count() > 0)
  {
    if (checkAddItem(rData, eMethod) == false)
      break;

    if (pCFG->dwMaxNum >= rData.base().count())
    {
      oCopy.mutable_base()->set_count(rData.base().count());
      rData.mutable_base()->set_count(0);
    }
    else
    {
      oCopy.mutable_base()->set_count(pCFG->dwMaxNum);
      rData.mutable_base()->set_count(rData.base().count() - pCFG->dwMaxNum);
    }

    additem(oCopy);
  }

  ItemInfo oItem;
  oItem.CopyFrom(rData.base());
  oItem.set_count(dwRealCount - rData.base().count());
  m_pUser->getEvent().onItemAdd(oItem);

  if(pCFG->eItemType == EITEMTYPE_MATERIAL && (rData.base().source() == ESOURCE_TRADE || rData.base().source() == ESOURCE_TRADE_PUBLICITY))
    m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_TRADE_MATERIAL);

  static const set<ESource> setSource = set<ESource>{ESOURCE_CHARGE, ESOURCE_TRADE, ESOURCE_TRADE_PUBLICITY, ESOURCE_TRADE_PUBLICITY_FAILRET};
  auto s = setSource.find(rData.base().source());
  if (s != setSource.end())
    m_pUser->saveDataNow();

  oItem.set_source(rData.base().source());
  itemLog(oItem, getItemCount(rData.base().id(), ECHECKMETHOD_NORMAL), rData.base().source());
  return true;
}

bool BasePackage::addItem(TVecItemData& vecData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  if (checkAddItem(vecData, eMethod) == false)
  {
    XERR << "[包裹-物品添加-TVecItemData]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "检查未通过" << XEND;
    return false;
  }

  for (auto &v : vecData)
    addItem(v, eMethod, bShow, bInform, bForceShow);

  for (auto v = vecData.begin(); v != vecData.end();)
  {
    if (v->base().count() == 0)
      v = vecData.erase(v);
    else
      ++v;
  }
  return true;
}

bool BasePackage::fourceShowItem(EItemType itemType, ESource source)
{
  if (ItemConfig::getMe().isShowItem(itemType) == false)
    return false;
  return false;
}

bool BasePackage::checkItemCount(DWORD itemid, DWORD count /*= 1*/, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(itemid);
  if (pCFG == nullptr)
    return false;

  if (pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_DIAMOND) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_SILVER) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GOLD) ||
      //pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GARDEN) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_CONTRIBUTE) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_PVPCOIN) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_LOTTERY) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GUILDHONOR)
      )

      //pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_FRIENDSHIP))
  {
    return m_pUser->checkMoney(static_cast<EMoneyType>(pCFG->eItemType), count);
  }

  DWORD num = 0;
  auto m = m_mapTypeID2Item.find(itemid);
  if (m == m_mapTypeID2Item.end())
    return false;

  for (auto s = m->second.begin(); s != m->second.end(); ++s)
  {
    if ((*s)->getGUID() == m_pUser->getPackage().getOperItem())
      continue;
    if (eMethod == ECHECKMETHOD_NONORMALEQUIP)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
      if (pEquip != nullptr && pEquip->canBeMaterial() == false)
        continue;
    }
    else if (eMethod == ECHECKMETHOD_UPGRADE)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
      if (pEquip != nullptr && pEquip->canBeUpgradeMaterial() == false)
        continue;
    }
    num += (*s)->getCount();
  }

  return num >= count;
}

bool BasePackage::setCardSlotForGM(DWORD itemId)
{
  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(itemId);
  if (pCFG == nullptr)
    return false;
  if (pCFG->dwCardSlot == 0)
    return false;

  auto m = m_mapTypeID2Item.find(itemId);
  if (m == m_mapTypeID2Item.end())
    return false;

  for (auto s = m->second.begin(); s != m->second.end(); ++s)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
    if (pEquip == nullptr)
      continue;
    pEquip->setCardSlot(pCFG->dwCardSlot);
    XLOG << "[GM-设置装备洞数] itemid" << itemId << "洞数" << pCFG->dwCardSlot<<"guild"<<pEquip->getGUID() << XEND;
  }
  return true;
}


void BasePackage::reduceItem(DWORD itemid, ESource source, DWORD count /*= 1*/, bool bDelete /*= true*/, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  if (checkItemCount(itemid, count, eMethod) == false)
    return;

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(itemid);
  if (pCFG == nullptr)
    return;

  if (pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_DIAMOND) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_SILVER) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GOLD) ||
      //pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_GARDEN) ||
      pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_CONTRIBUTE) ||
      pCFG->eItemType == static_cast<DWORD> (EMONEYTYPE_PVPCOIN) ||
      pCFG->eItemType == static_cast<DWORD> (EMONEYTYPE_LOTTERY) ||
      pCFG->eItemType == static_cast<DWORD> (EMONEYTYPE_GUILDHONOR))
      //pCFG->eItemType == static_cast<DWORD>(EMONEYTYPE_FRIENDSHIP))
  {
    m_pUser->subMoney(static_cast<EMoneyType>(pCFG->eItemType), count, source);
    return;
  }

  auto m = m_mapTypeID2Item.find(itemid);
  if (m == m_mapTypeID2Item.end())
    return;
  if (m->second.empty() == true)
    return;

  set<ItemBase*> setBase;
  for (auto s = m->second.begin(); s != m->second.end(); ++s)
    setBase.insert(*s);

  for (auto s = setBase.begin(); s != setBase.end(); ++s)
  {
    if ((*s)->getGUID() == m_pUser->getPackage().getOperItem())
      continue;
    if (eMethod == ECHECKMETHOD_NONORMALEQUIP)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
      if (pEquip != nullptr && pEquip->canBeMaterial() == false)
        continue;
    }
    else if (eMethod == ECHECKMETHOD_UPGRADE)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(*s);
      if (pEquip != nullptr && pEquip->canBeUpgradeMaterial() == false)
        continue;
    }

    ItemBase* pBase = *s;
    if (pBase->getCount() >= count)
    {
      reduceItem(pBase->getGUID(),  source, count, bDelete, eMethod);
      return;
    }

    count -= pBase->getCount();
    reduceItem(pBase->getGUID(), source, pBase->getCount(), bDelete, eMethod);
  }
}

bool BasePackage::checkItemCount(const string& guid, DWORD count /*= 1*/, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  auto m = m_mapID2Item.find(guid);
  if (m == m_mapID2Item.end())
    return false;

  if (eMethod == ECHECKMETHOD_NONORMALEQUIP)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m->second);
    if (pEquip != nullptr && pEquip->canBeMaterial() == false)
      return false;
  }
  else if (eMethod == ECHECKMETHOD_UPGRADE)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m->second);
    if (pEquip != nullptr && pEquip->canBeUpgradeMaterial() == false)
      return false;
  }

  return m->second->getCount() >= count;
}

void BasePackage::reduceItem(const string& guid, ESource source, DWORD count /*= 1*/, bool bDelete /*= true*/, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  if (checkItemCount(guid, count, eMethod) == false)
    return;

  auto m = m_mapID2Item.find(guid);
  if (m == m_mapID2Item.end())
  {
    if (m_setArtifactGuid.find(guid) != m_setArtifactGuid.end())
      m_setArtifactGuid.erase(guid);
    return;
  }

  // check item type
  //if (m->second->getType() == EITEMTYPE_FUNCTION)
  //  return;

  // dec count
  DWORD itemCount = m->second->getCount() - count;
  DWORD id = m->second->getTypeID();
  DWORD realDecCount = count;
  if (itemCount <= 0)
  {
    realDecCount = m->second->getCount();
    itemCount = 0;
  }

  // equip can has itemcount
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m->second);
  if (pEquip != nullptr)
    itemCount = 0;


  //商店出售
  std::string jsonStr;
  if (source == ESOURCE_SELL || source == ESOURCE_PERSON_PUTSTORE || source == ESOURCE_PERSON_OFFSTORE || 
    source == ESOURCE_PUBLIC_PUTSTORE || source == ESOURCE_PUBLIC_OFFSTORE || source == ESOURCE_REPAIR)
  {
    if (pEquip)
    {
      if (0 == m_logRefineLv)
        m_logRefineLv = pEquip->getRefineLv();

      if (m_logRefineLv || pEquip->EquipedCard() || pEquip->isEnchant() || pEquip->getLv())
      {
        ItemData itemData;
        pEquip->toItemData(&itemData);
        if (itemData.mutable_equip())
          itemData.mutable_equip()->set_refinelv(m_logRefineLv);
        jsonStr = pb2json(itemData);
        m_logRefineLv = 0;
      }
    }
  }

  addDelInformItems(m->second->getTypeID(), count, source);

  if (itemCount > 0)
  {
    m->second->setCount(itemCount);
    m_setUpdateIDs.insert(m->second->getGUID());
    XLOG << "[包裹-物品删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << getPackageType() << "add reduce item:" << m->second->getTypeID() << "count:" << count << "left count :" << m->second->getCount() << "来源 :" << source << XEND;
    propsLog(id, realDecCount, getItemCount(id, ECHECKMETHOD_NORMAL), source, jsonStr);
    return;
  }

  DWORD index = m->second->getIndex();
  m_setUpdateIDs.insert(m->second->getGUID());

  ItemBase* pBase = m->second;
  m_mapID2Item.erase(m);
  m_mapTypeID2Item[pBase->getTypeID()].erase(pBase);
  if (ItemConfig::getMe().isArtifact(pBase->getType()))
    m_setArtifactGuid.erase(pBase->getGUID());
  m_mapDeleteID2Item.erase(guid);

  // if isFashion refresh attr
  if (pEquip != nullptr && ItemManager::getMe().isFashion(pEquip->getEquipType()) == true)
  {
    m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    m_pUser->refreshDataAtonce();
  }

  // on delete from package
  m_pUser->getEvent().onItemDelte(pBase);

  if (bDelete)
  {
    //SAFE_DELETE(pBase);
    addGarbageItem(pBase);
  }

  if (index == getMaxIndex() && index > 0)
    setMaxIndex(index - 1);

  // check fashion
  m_pUser->getPackage().decItemCount(id);

  propsLog(id, realDecCount, getItemCount(id, ECHECKMETHOD_NORMAL), source, jsonStr);
  XLOG << "[包裹-物品删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << getPackageType() << "add reduce item:" << id << "count:" << count << "left count : 0" << XEND;
}

//物品获取日志
void BasePackage::itemLog(const ItemInfo& rInfo, DWORD after, ESource source)
{
  if (m_pUser)
  {
    if (source == ESOURCE_PICKUP)
      m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_DROPITEM, rInfo.id(), rInfo.count());  //打怪掉落物品

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_ITEM_COUNT, rInfo.id(), rInfo.source(), m_pUser->getLevel(), rInfo.count());

    QWORD eid = xTime::getCurUSec();
    if (getPackageType() == EPACKTYPE_TEMP_MAIN)
    {
      DWORD eventType = EventType_TempPackAdd;    //进入临时背包
      PlatLogManager::getMe().gainItemLog(thisServer,
        m_pUser->getUserSceneData().getPlatformId(),
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eid,
        m_pUser->getUserSceneData().getCharge(),
        rInfo.id(),
        rInfo.count(),
        after,
        source,
        eventType);
    }
    else
    {
      //==============warning==============
      if (m_logSource != ESOURCE_NORMAL)
        source = m_logSource;

      PlatLogManager::getMe().gainItemLog(thisServer,
        m_pUser->getUserSceneData().getPlatformId(),
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eid,
        m_pUser->getUserSceneData().getCharge(),
        rInfo.id(),
        rInfo.count(),
        after,
        source);
    }
  }
}

//物品使用日志
void BasePackage::propsLog(DWORD id, DWORD  decCount, DWORD after, ESource source, const std::string& jsonStr/*=""*/)
{
  if (m_pUser)
  {
    QWORD eid = xTime::getCurUSec();
    PlatLogManager::getMe().consumeItemLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eid,
      m_pUser->getUserSceneData().getCharge(),
      id,/*itemid */
      decCount, after,
      source,
      0,
      jsonStr
      );

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_PROPS_COUNT, id, source, m_pUser->getLevel(), decCount);
  
    if (getPackageType() == EPACKTYPE_TEMP_MAIN)
    {
      QWORD eid = xTime::getCurUSec();
      DWORD eventType = EventType_TempPackSub;
      PlatLogManager::getMe().consumeItemLog(thisServer,
        m_pUser->getUserSceneData().getPlatformId(),
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eid,
        m_pUser->getUserSceneData().getCharge(),
        id,/*itemid */
        decCount, after,
        source,
        eventType,
        jsonStr
      );
    }  
  }
}

bool BasePackage::checkItemCount(const TVecItemInfo& vecItems, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  for (auto v = vecItems.begin(); v != vecItems.end(); ++v)
  {
    if (checkItemCount(v->id(), v->count(), eMethod) == false)
      return false;
  }

  return true;
}

void BasePackage::reduceItem(const TVecItemInfo& vecItems, ESource source, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  for (auto v = vecItems.begin(); v != vecItems.end(); ++v)
    reduceItem(v->id(), source, v->count(), true, eMethod);
}

void BasePackage::packageSort(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  if (getPackageType() == EPACKTYPE_STORE)
  {
    // collect items
    TVecSortItem vecItems;
    for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
      vecItems.push_back(m->second);

    // create cmd
    PackageSort cmd;
    cmd.set_type(getPackageType());

    // reset index
    m_dwMaxIndex = 0;
    for (auto v = vecItems.begin(); v != vecItems.end(); ++v)
    {
      (*v)->setIndex(getNextIndex());

      SortInfo* pInfo = cmd.add_item();
      if (pInfo != nullptr)
      {
        pInfo->set_guid((*v)->getGUID());
        pInfo->set_index((*v)->getIndex());
      }
    }

    // inform client
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);

    XLOG << "[包裹-整理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "整理" << getPackageType() << "失败,该包裹暂时不支持整理" << XEND;
    return;
  }

  // combine items
  for (auto m = m_mapTypeID2Item.begin(); m != m_mapTypeID2Item.end(); ++m)
  {
    TSetItemBase& setBase = m->second;
    if (setBase.empty() == true)
      continue;

    const SItemCFG* pCFG = (*setBase.begin())->getCFG();
    if (pCFG == nullptr || pCFG->dwMaxNum <= 1)
      continue;

    DWORD dwTotal = 0;
    DWORD dwShowTotal = 0;
    for (auto &s : setBase)
      dwTotal += s->getCount();
    dwShowTotal = dwTotal;

    for (auto s = setBase.begin(); s != setBase.end();)
    {
      DWORD dwCount = dwTotal > pCFG->dwMaxNum ? pCFG->dwMaxNum : dwTotal;
      dwTotal = dwTotal > pCFG->dwMaxNum ? dwTotal - pCFG->dwMaxNum : 0;

      if ((*s)->getCount() != dwCount)
      {
        XLOG << "[包裹-整理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "整理" << getPackageType() << "guid :" << (*s)->getGUID() << "id :" << (*s)->getTypeID() << "count :" << (*s)->getCount() << "合并为 count :" << dwCount
          << "剩余" << dwTotal << "总" << dwShowTotal << XEND;
        (*s)->setCount(dwCount);
        m_setUpdateIDs.insert((*s)->getGUID());
      }

      if ((*s)->getCount() != 0)
      {
        ++s;
      }
      else
      {
        XLOG << "[包裹-整理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "整理" << getPackageType() << "guid :" << (*s)->getGUID() << "id :" << (*s)->getTypeID() << "合并后为0,被移除" << XEND;

        ItemBase* pBase = *s;
        auto m = m_mapID2Item.find(pBase->getGUID());
        if (m != m_mapID2Item.end())
          m_mapID2Item.erase(m);
        else
        {
          XERR << "[包裹-整理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
            << "整理" << getPackageType() << "guid :" << (*s)->getGUID() << "id :" << (*s)->getTypeID() << "合并后为0,总列表移除失败,未发现" << XEND;
        }

        s = setBase.erase(s);
        //SAFE_DELETE(pBase);
        addGarbageItem(pBase);
      }
    }

    if (dwTotal != 0)
    {
      ItemData oData;
      oData.mutable_base()->set_id(pCFG->dwTypeID);
      oData.mutable_base()->set_count(dwTotal);
      addItem(oData, EPACKMETHOD_NOCHECK);
      XLOG << "[包裹-整理]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "整理" << getPackageType() << "id :" << pCFG->dwTypeID << "多出" << dwTotal << "总" << dwShowTotal << XEND;
    }
  }
  update(xTime::getCurSec());

  // collect items
  TVecSortItem vecItems;
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
    vecItems.push_back(m->second);

  // get package config
  const SPackageCFG& rCFG = MiscConfig::getMe().getPackageCFG();

  // sort by time
  sort(vecItems.begin(), vecItems.end(), [pUser, rCFG](const ItemBase* p1, const ItemBase* p2) -> bool {
    if (p1 == nullptr || p2 == nullptr)
      return false;

    const SItemCFG* pCFG1 = p1->getCFG();
    const SItemCFG* pCFG2 = p2->getCFG();
    if (pCFG1 != nullptr && pCFG2 != nullptr && (pCFG1->dwItemSort != 0 || pCFG2->dwItemSort != 0))
    {
      if (pCFG1->dwItemSort < pCFG2->dwItemSort)
        return true;
      if (pCFG1->dwItemSort > pCFG2->dwItemSort)
        return false;
    }

    if (ItemManager::getMe().isEquip(p1->getType()) == true && ItemManager::getMe().isEquip(p2->getType()) == true)
    {
      // weapon profession sort
      DWORD type1 = ItemManager::getMe().getSortWeaponIndex(p1->getType(), pUser->getUserSceneData().getProfession());
      DWORD type2 = ItemManager::getMe().getSortWeaponIndex(p2->getType(), pUser->getUserSceneData().getProfession());
      if (type1 > type2)
        return true;
      if (type1 < type2)
        return false;
    }
    else
    {
      // type sort
      DWORD index1 = rCFG.getIndex(p1->getType());
      DWORD index2 = rCFG.getIndex(p2->getType());
      if (index1 > index2)
        return true;
      if (index1 < index2)
        return false;
    }

    if (p1->getTypeID() < p2->getTypeID())
      return true;
    if (p1->getTypeID() > p2->getTypeID())
      return false;

    return false;
  });

  // create cmd
  PackageSort cmd;
  cmd.set_type(getPackageType());

  // reset index
  m_dwMaxIndex = 0;
  for (auto v = vecItems.begin(); v != vecItems.end(); ++v)
  {
    (*v)->setIndex(getNextIndex());

    SortInfo* pInfo = cmd.add_item();
    if (pInfo != nullptr)
    {
      pInfo->set_guid((*v)->getGUID());
      pInfo->set_index((*v)->getIndex());
    }
  }

  // inform client
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void BasePackage::refreshDeleteTimeItems()
{
  if(m_mapDeleteID2Item.empty() == true)
    return;

  DWORD nowTime = xTime::getCurSec();
  TSetItemBase setItems;
  for (auto m = m_mapDeleteID2Item.begin(); m != m_mapDeleteID2Item.end();)
  {
    ItemBase* pBase = getItem(m->first);
    if (pBase == nullptr)
    {
      XERR << "[包裹-超时删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "item:" << m->first << "未在" << getPackageType() << "中找到,直接删除" << XEND;
      m = m_mapDeleteID2Item.erase(m);
      continue;
    }
    if (m->second <= nowTime)
    {
      setItems.insert(pBase);
      m = m_mapDeleteID2Item.erase(m);
      continue;
    }
    ++m;
  }

  for (auto &s : setItems)
  {
    ItemData oData;
    s->toItemData(&oData);
    reduceItem(s->getGUID(), ESOURCE_PACKAGE, s->getCount());
    XLOG << "[包裹-超时删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "item:" << oData.ShortDebugString() << "超时,从" << getPackageType() << "中删除" << XEND;
  }
}

void BasePackage::modifyOverTime(ItemBase* pBase)
{
  if(pBase == nullptr)
    return;

  const SItemCFG* pCFG = pBase->getCFG();
  if (pCFG == nullptr)
    return;

  if (pCFG->eDelType == EITEMDELTYPE_TIME && pCFG->dwDelTime != 0)
  {
    DWORD nowTime = xTime::getCurSec();
    DWORD exsitTime = nowTime > pBase->getCreateTime() ? (nowTime - pBase->getCreateTime()) : 0;
    if(pCFG->dwDelTime > exsitTime)
      pBase->setOverTime(nowTime + pCFG->dwDelTime - exsitTime);
    else
      pBase->setOverTime(nowTime);
  }
  else if (pCFG->eDelType == EITEMDELTYPE_DATE && pCFG->dwDelDate != 0)
  {
    pBase->setOverTime(pCFG->dwDelDate);
  }
  else
  {
    if (getPackageType() != EPACKTYPE_TEMP_MAIN)
      pBase->setOverTime(0);
  }

  ItemWedding* pWedding = dynamic_cast<ItemWedding*>(pBase);
  if (pWedding != nullptr)
  {
    const WeddingData& rData = pWedding->getWeddingData();
    pWedding->setOverTime(rData.endtime());
  }

  if (pBase->getOverTime() == 0)
    return;

  XDBG << "[包裹-过期时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "item :" << pBase->getGUID() << pBase->getTypeID() << "超时时间" << pBase->getOverTime() << XEND;
  m_mapDeleteID2Item.insert(std::make_pair(pBase->getGUID(), pBase->getOverTime()));
}

void BasePackage::update(DWORD curSec)
{
  if (m_setUpdateIDs.empty() == false)
  {
    PackageUpdate oUpdate;
    oUpdate.set_type(getPackageType());
    for (auto s = m_setUpdateIDs.begin(); s != m_setUpdateIDs.end(); ++s)
    {
      ItemBase* pBase = getItem(*s);
      if (pBase != nullptr)
      {
        ItemData* pData = oUpdate.add_updateitems();
        if (pData == nullptr)
          continue;

        pBase->toClientData(getPackageType(), pData, m_pUser);
      }
      else
      {
        ItemData* pData = oUpdate.add_delitems();
        if (pData == nullptr)
          continue;

        pData->mutable_base()->set_guid(*s);
      }
    }

    if (oUpdate.updateitems_size() > 0 || oUpdate.delitems_size() > 0)
    {
      PROTOBUF(oUpdate, send, len);
      m_pUser->sendCmdToMe(send, len);
    }

    m_setUpdateIDs.clear();
  }

  showItems();

  if (getPackageType() == EPACKTYPE_MAIN)
    m_pUser->getShortcut().refreshShortcut();

  if (m_bRefreshSlot)
  {
    m_bRefreshSlot = false;
    //DWORD dwLastMaxSlot = m_dwLastMaxSlot;
    DWORD dwMaxSlot = getMaxSlot();
    //if (dwLastMaxSlot != dwMaxSlot)
    {
      PackSlotNtfItemCmd cmd;
      cmd.set_type(getPackageType());
      cmd.set_maxslot(dwMaxSlot);
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      XLOG << "[包裹-最大上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << getPackageType() << "刷新最大上限为" << dwMaxSlot << "个" << XEND;
    }
  }

  resetGarbageItem();
}

void BasePackage::addShowItems(const ItemInfo& rInfo, bool bShow, bool bInform, bool bForceShow/*=false*/)
{
  if (m_pUser->getPackage().isMultipleUse())
  {
    m_pUser->getPackage().addShowItems(rInfo, bShow, bInform, bForceShow);
    return;
  }

  m_bShowItem = m_pUser->getPackage().addShowItems(rInfo, bShow, bInform, bForceShow, m_vecShowItems, m_vecInformItems);
}

void BasePackage::addDelInformItems(DWORD id, DWORD count, ESource eSource)
{
  if (eSource == ESOURCE_OFF_TEMPPACK || eSource == ESOURCE_OFF_BARROW || eSource == ESOURCE_PERSON_OFFSTORE)
    return;

  ItemInfo oItem;
  oItem.set_id(id);
  oItem.set_count(count);
  m_vecInformDelItems.push_back(oItem);
}

void BasePackage::showItems()
{
  if (m_pUser == nullptr)
    return;

  if (m_bShowItem)
  {
    m_bShowItem = false;
    m_pUser->getPackage().showItems(m_vecShowItems, m_vecInformItems);
  }

  if (m_vecInformDelItems.empty() == false)
  {
    for (auto &v : m_vecInformDelItems)
    {
      MsgParams params;
      params.addNumber(v.id());
      params.addNumber(v.id());
      params.addNumber(v.count());

      if (getPackageType() == EPACKTYPE_BARROW)
        MsgManager::sendMsg(m_pUser->id, 82, params);
      else if (getPackageType() == EPACKTYPE_PERSONAL_STORE)
        MsgManager::sendMsg(m_pUser->id, 83, params);
      else if (getPackageType() != EPACKTYPE_MAIN)
      {
        if (thisServer->isOuter() == false)
        {
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.id());
          if (pCFG != nullptr)
          {
            stringstream sstr;
            sstr << "测试log : 包裹:" << getPackageType() << " 消耗" << v.id() << v.count() << "个" << XEND;
            MsgManager::sendDebugMsg(m_pUser->id, sstr.str());
          }
        }
      }
    }
    m_vecInformDelItems.clear();
  }
}

void BasePackage::modifyEgg(EggData* pEgg)
{
  if (pEgg == nullptr)
    return;

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD maxlv = rCFG.dwOverUserLv + m_pUser->getLevel();
  pEgg->set_hp(0);

  // 宠物等级大于当前玩家可携带等级(仓库, 大号转小号)
  if (pEgg->lv() > maxlv)
  {
    for (DWORD d = maxlv + 1; d <= pEgg->lv(); ++d)
    {
      const SPetBaseLvCFG* pCFG = PetConfig::getMe().getPetBaseLvCFG(d);
      if (pCFG == nullptr)
      {
        XERR << "[包裹-宠物蛋修正]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "宠物蛋" << pEgg->id() << pEgg->name() << "等级" << pEgg->lv() << "降级为" << maxlv << "补偿" << d << "级经验,失败,未在 Table_PetBaseLevel.txt 表中找到" << XEND;
        continue;
      }
      pEgg->set_exp(pEgg->exp() + pCFG->qwNewCurExp);
      XDBG << "[包裹-宠物蛋修正]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "宠物蛋" << pEgg->id() << pEgg->name() << "等级" << pEgg->lv() << "降级为" << maxlv << "补偿" << d << "级经验" << pCFG->qwNewCurExp << XEND;
    }
    pEgg->set_lv(maxlv);
  }
  else
  {
    XDBG << "[包裹-宠物蛋修正]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "宠物蛋" << pEgg->id() << pEgg->name() << "等级" << pEgg->lv() << "小于" << maxlv << "进行升级" << XEND;

    SScenePetData stData;
    stData.dwLv = pEgg->lv();
    stData.qwExp = pEgg->exp();
    stData.pOwner = m_pUser;
    stData.baseLevelup();
    pEgg->set_lv(stData.dwLv);
    pEgg->set_exp(stData.qwExp);
  }
}

void BasePackage::addGarbageItem(ItemBase* pBase)
{
  if (pBase == nullptr)
    return;
  auto s = m_setGarbageItems.find(pBase);
  if (s != m_setGarbageItems.end())
    return;
  ItemData oData;
  pBase->toItemData(&oData);
  m_setGarbageItems.insert(pBase);
  XLOG << "[包裹-物品回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "item:" << oData.ShortDebugString() << "进入回收站" << XEND;
}

void BasePackage::resetGarbageItem()
{
  if (m_setGarbageItems.empty() == true)
    return;

  for (auto &s : m_setGarbageItems)
  {
    ItemBase* pBase = s;
    ItemData oData;
    pBase->toItemData(&oData);
    XLOG << "[包裹-物品回收]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "item:" << oData.ShortDebugString() << "被回收" << XEND;
    SAFE_DELETE(pBase);
  }
  m_setGarbageItems.clear();
}

// main package
MainPackage::MainPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

MainPackage::~MainPackage()
{

}

const string& MainPackage::getMountGUID(DWORD dwID) const
{
  static const string emptyguid;
  auto m = m_mapTypeID2Item.find(dwID);
  if (m == m_mapTypeID2Item.end())
    return emptyguid;

  if (m->second.empty() == true)
    return emptyguid;

  return (*m->second.begin())->getGUID();
}

// 检测装备上架条件
bool MainPackage::checkSignUp(const std::string& guid, DWORD& dwPointOut, DWORD& dwBuffOut)
{
  if(guid.empty())
    return false;

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(getItem(guid));
  if(!pEquip)
  {
    XERR << "[包裹-检测上架]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "guid:" << guid << "失败,找不到对应ItemBase" << XEND;
    return false;
  }

  DWORD itemId = pEquip->getTypeID();
  if (pEquip->getStrengthLv())
  {
    DWORD retMoney;
    if (!m_pUser->getPackage().decompose(guid, false, retMoney))
    {
      XERR << "[交易-出售] 强化装备分解失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId  << XEND;
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
      XERR << "[交易-出售] 装备插卡脱下失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << XEND;
      return false;
    }
  }

  if (pEquip->getLv())
  {
    RestoreEquipItemCmd cmd;
    cmd.set_equipid(guid);
    cmd.set_upgrade(true);
    if (m_pUser->getPackage().restore(cmd) == false)
    {
      XERR << "[交易-出售] 装备升级等级移除失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << XEND;
      return false;
    }
  }
  if (pEquip->getStrengthLv2())
  {
    RestoreEquipItemCmd cmd;
    cmd.set_equipid(guid);
    cmd.set_strengthlv2(true);
    if (m_pUser->getPackage().restore(cmd) == false)
    {
      XERR << "[交易-出售] 加固装备等级移除失败 user =" << m_pUser->accid << m_pUser->id << "guid =" << guid << "itemid =" << itemId << XEND;
      return false;
    }
  }

  return pEquip->checkSignUp(dwPointOut, dwBuffOut);
}

// equip package
EquipPackage::EquipPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

EquipPackage::~EquipPackage()
{

}

bool EquipPackage::fromData(const PackageData& rData)
{
  if (rData.type() != getPackageType())
  {
    XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << rData.ShortDebugString() << "失败,背包类型不对" << XEND;
    return false;
  }

  for (int j = 0; j < rData.items_size() ; ++j)
  {
    const ItemData& item = rData.items(j);
    if (item.base().guid().empty() == true)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,guid为空" << XEND;
      continue;
    }

    ItemBase* pBase = ItemManager::getMe().createItem(item.base());
    if (pBase == nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      continue;
    }
    pBase->fromItemData(item);
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip == nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,不是装备" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      SAFE_DELETE(pBase);
      continue;
    }

    if (getEquip(static_cast<EEquipPos>(pEquip->getIndex())) != nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,位置有装备" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      SAFE_DELETE(pBase);
      continue;
    }

    setEquip(m_pUser, static_cast<EEquipPos>(pEquip->getIndex()), pEquip, true);
  }

  clearUpdateIDs();
  m_pUser->onAltmanFashionEquip();
  return true;
}

bool EquipPackage::canEquip(SceneUser* pUser, const ItemBase* pBase, bool bNotice /*=false*/)
{
  if (pUser == nullptr)
    return false;

  const ItemEquip* pEquip = dynamic_cast<const ItemEquip*>(pBase);
  if (pEquip == nullptr || pEquip->getCFG() == nullptr || pEquip->getCFG()->dwLevel > pUser->getUserSceneData().getRolelv())
    return false;
  if (pEquip->canEquip(pUser->getUserSceneData().getProfession()) == false)
    return false;
  if (pEquip->canGenderEquip(pUser->getUserSceneData().getGender()) == false)
    return false;
  DWORD errMsg = LuaManager::getMe().call<int>("itemUserCheck", (xSceneEntryDynamic*)(pUser), pBase->getTypeID());
  if (errMsg)
  {
    if (bNotice) MsgManager::sendMsg(pUser->id, errMsg);
    return false;
  }
  if (ItemConfig::getMe().isArtifact(pEquip->getType()))
  {
    if (pUser->hasGuild() == false || pUser->getGuild().isUserOwnArtifact(pUser->id, pEquip->getGUID()) == false)
      return false;
    // 卡牌副本不能装备神器
    if (pUser->getScene() && pUser->getScene()->getSceneType() == SCENE_TYPE_PVECARD)
    {
      MsgManager::sendMsg(pUser->id, 119);
      return false;
    }
    if (pUser->getScene() && pUser->getScene()->getSceneType() == SCENE_TYPE_TEAMPWS)
    {
      return false;
    }
  }
  return true;
}

bool EquipPackage::setEquip(SceneUser* pUser, EEquipPos ePos, ItemBase* pBase, bool bInit /*= false*/)
{
  // if base not null and check can equip
  if (pBase != nullptr && !bInit)
  {
    if (canEquip(pUser, pBase) == false)
      return false;
  }

  // release old equip
  ItemEquip* pEquip = getEquip(ePos);
  if (pEquip != nullptr)
  {
    auto m = m_mapID2Item.find(pEquip->getGUID());
    if (m != m_mapID2Item.end())
      m_mapID2Item.erase(m);
    auto n = m_mapTypeID2Item.find(pEquip->getTypeID());
    if (n != m_mapTypeID2Item.end())
      m_mapTypeID2Item.erase(n);

    pEquip->setEquiped(false);
    m_setUpdateIDs.insert(pEquip->getGUID());

    // update user show data
    if (pEquip->getEquipType() == EEQUIPTYPE_WEAPON || pEquip->getEquipType() == EEQUIPTYPE_ARTIFACT)
    {
      m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
      m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
    }
    else if (pEquip->getEquipType() == EEQUIPTYPE_SHIELD)
      m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
    else if (pEquip->getEquipType() == EEQUIPTYPE_BACK || pEquip->getEquipType() == EEQUIPTYPE_ARTIFACT_BACK)
      m_pUser->setDataMark(EUSERDATATYPE_BACK);
    else if (pEquip->getEquipType() == EEQUIPTYPE_HEAD || pEquip->getEquipType() == EEQUIPTYPE_ARTIFACT_HEAD)
      m_pUser->setDataMark(EUSERDATATYPE_HEAD);
    else if (pEquip->getEquipType() == EEQUIPTYPE_FACE)
      m_pUser->setDataMark(EUSERDATATYPE_FACE);
    else if (pEquip->getEquipType() == EEQUIPTYPE_TAIL)
      m_pUser->setDataMark(EUSERDATATYPE_TAIL);
    else if (pEquip->getEquipType() == EEQUIPTYPE_MOUNT)
      m_pUser->setDataMark(EUSERDATATYPE_MOUNT);
    else if (pEquip->getEquipType() == EEQUIPTYPE_MOUTH)
      m_pUser->setDataMark(EUSERDATATYPE_MOUTH);
    else if (pEquip->getEquipType() == EEQUIPTYPE_ARMOUR)
      m_pUser->setDataMark(EUSERDATATYPE_BODY);

    // check fashion
    m_pUser->getPackage().decItemCount(pEquip->getTypeID());
    m_pUser->refreshDataAtonce();
  }

  if (ePos == EEQUIPPOS_WEAPON)
  {
    m_pUser->setDataMark(EUSERDATATYPE_EQUIPED_WEAPON);
    m_pUser->refreshDataAtonce();
  }

  // set new equip
  pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
    return false;

  pEquip->setEquiped(true);
  pEquip->setIndex(ePos);

  m_mapID2Item[pEquip->getGUID()] = pEquip;
  m_mapTypeID2Item[pEquip->getTypeID()].clear();
  m_mapTypeID2Item[pEquip->getTypeID()].insert(pEquip);

  m_setUpdateIDs.insert(pEquip->getGUID());

  // check item
  m_pUser->getPackage().addItemCount(pEquip);

  // update user show data
  if (pEquip->getEquipType() == EEQUIPTYPE_WEAPON || pEquip->getEquipType() == EEQUIPTYPE_ARTIFACT)
  {
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
    m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
  }
  else if (pEquip->getEquipType() == EEQUIPTYPE_SHIELD)
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
  else if (pEquip->getEquipType() == EEQUIPTYPE_BACK || pEquip->getEquipType() == EEQUIPTYPE_ARTIFACT_BACK)
    m_pUser->setDataMark(EUSERDATATYPE_BACK);
  else if (pEquip->getEquipType() == EEQUIPTYPE_HEAD || pEquip->getEquipType() == EEQUIPTYPE_ARTIFACT_HEAD)
    m_pUser->setDataMark(EUSERDATATYPE_HEAD);
  else if (pEquip->getEquipType() == EEQUIPTYPE_FACE)
    m_pUser->setDataMark(EUSERDATATYPE_FACE);
  else if (pEquip->getEquipType() == EEQUIPTYPE_TAIL)
    m_pUser->setDataMark(EUSERDATATYPE_TAIL);
  else if (pEquip->getEquipType() == EEQUIPTYPE_MOUNT)
    m_pUser->setDataMark(EUSERDATATYPE_MOUNT);
  else if (pEquip->getEquipType() == EEQUIPTYPE_MOUTH)
    m_pUser->setDataMark(EUSERDATATYPE_MOUTH);
  else if (pEquip->getEquipType() == EEQUIPTYPE_ARMOUR)
    m_pUser->setDataMark(EUSERDATATYPE_BODY);

  m_pUser->refreshDataAtonce();
  return true;
}

ItemEquip* EquipPackage::getEquip(EEquipPos ePos)
{
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m->second);
    if (pEquip != nullptr && pEquip->getIndex() == ePos)
      return pEquip;
  }

  return nullptr;
}

void EquipPackage::collectInValidEquip(TSetString& setIDs)
{
  setIDs.clear();
  for (auto &m : m_mapID2Item)
  {
    if (canEquip(m_pUser, m.second) == false)
      setIDs.insert(m.first);
  }
}

DWORD EquipPackage::getEquipedItemNum(DWORD itemid, bool checkBreak/* = false*/)
{
  DWORD num = 0;
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m->second);
    if (pEquip == nullptr)
      continue;
    if (checkBreak && pEquip->isBroken())
      continue;
    if (pEquip->getTypeID() == itemid)
    {
      num++;
      continue;
    }
    const TMapEquipCard& cards = pEquip->getCardList();
    for (auto &v : cards)
    {
      if (v.second.second == itemid)
        num++;
    }
  }

  return num;
}

bool EquipPackage::hasBraokenEquip()
{
  for (auto &m : m_mapID2Item)
  {
    if (m.second)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m.second);
      if (pEquip && pEquip->isBroken())
        return true;
    }
  }
  return false;
}

DWORD EquipPackage::getEquipSuitNum(SceneUser* pUser, DWORD suitid)
{
  if (pUser == nullptr)
    return 0;
  const SSuitCFG* pSuitCFG = ItemConfig::getMe().getSuitCFG(suitid);
  if (pSuitCFG == nullptr)
    return 0;

  TSetDWORD allEquipIDs;
  for (auto &m : m_mapID2Item)
  {
    if (m.second)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m.second);
      if (pEquip && pEquip->isBroken())
        continue;
      allEquipIDs.insert(m.second->getTypeID());
    }
  }

  DWORD suitnum = 0;
  for (auto &v : pSuitCFG->vecEquipIDs)
  {
    // 基础装备
    if (allEquipIDs.find(v) != allEquipIDs.end())
    {
      suitnum ++;
      continue;
    }

    // 升级, 开洞等装备
    TSetDWORD okids;
    ItemConfig::getMe().getSuitUpEquips(v, okids);
    for (auto &s : okids)
    {
      if (allEquipIDs.find(s) != allEquipIDs.end())
      {
        suitnum ++;
        break;
      }
    }
  }

  return suitnum;

  /*
  TSetDWORD setids;
  auto func = [&pUser, &suitid, &setids](const ItemBase* pItem)
  {
    const ItemEquip* pEquip = dynamic_cast<const ItemEquip*>(pItem);
    if (pEquip == nullptr || pEquip->canEquip(pUser->getUserSceneData().getProfession()) == false)
        return;
    //if (ItemManager::getMe().isFashion(pEquip->getEquipType()) == true)
      //  return;
    const TSetDWORD& setsuit = ItemManager::getMe().getSuitIDs(pEquip->getTypeID());
    if (setsuit.find(suitid) != setsuit.end())
      setids.insert(pEquip->getTypeID());
  };
  foreach(func);
  return setids.size();
  */
}

DWORD EquipPackage::getCardSuitNum(SceneUser* pUser, DWORD suitid)
{
  if (pUser == nullptr)
    return 0;
  DWORD num = 0;

  auto func = [&pUser, &suitid, &num] (const ItemBase* pItem)
  {
    const ItemEquip* pEquip = dynamic_cast<const ItemEquip*>(pItem);
    if (pEquip == nullptr || pEquip->canEquip(pUser->getUserSceneData().getProfession()) == false)
        return;

    TVecEquipCard vecCardItems;
    pEquip->getEquipCard(vecCardItems);
    set<DWORD> tmpDSet;
    for( auto v = vecCardItems.begin(); v != vecCardItems.end(); ++v)
    {
      // 相同卡片不构成套装
      if (tmpDSet.find((*v).second) != tmpDSet.end())
        continue;
      tmpDSet.insert((*v).second);

      const SItemCFG* pItemCFG = ItemManager::getMe().getItemCFG((*v).second);
      if (pItemCFG == nullptr)
        continue;
      const TSetDWORD& setsuit = ItemManager::getMe().getSuitIDs(pItemCFG->dwTypeID);
      if (setsuit.find(suitid) != setsuit.end())
        ++num;
    }
  };
  foreach(func);
  return num;
}

// 获取套装装备, 同typeid的装备取refinelv大的
void EquipPackage::getSuitEquipItems(SceneUser* pUser, DWORD suitid, TMapItemEquip& mapEquips)
{
  if (pUser == nullptr) {
    return;
  }
  auto func = [&](ItemBase *pItem) {
    ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pItem);
    if (pEquip == nullptr || pEquip->canEquip(pUser->getUserSceneData().getProfession()) == false) {
      return;
    }
    const TSetDWORD& setsuit = ItemManager::getMe().getSuitIDs(pEquip->getTypeID());
    if (setsuit.find(suitid) != setsuit.end()) {
      auto it = mapEquips.find(pEquip->getTypeID());
      if (it == mapEquips.end())
        mapEquips[pEquip->getTypeID()] = pEquip;
      else
        mapEquips[pEquip->getTypeID()] = it->second->getRefineLv() > pEquip->getRefineLv() ? it->second : pEquip;
    }
  };
  foreach(func);
}

// 是否满足套装
bool EquipPackage::isSuitValid(DWORD suitid)
{
  const SSuitCFG* pCfg = ItemManager::getMe().getSuitCFG(suitid);
  if (pCfg == nullptr)
    return false;
  return getEquipSuitNum(m_pUser, suitid) == pCfg->vecEquipIDs.size();
}

// 计算套装加成属性
//void EquipPackage::collectSuitAttr(TVecAttrSvrs& vecAttrs)
void EquipPackage::collectSuitAttr()
{
  Attribute* pAttr = m_pUser->getAttribute();
  if (pAttr == nullptr)
    return;

  TSetDWORD isCollect;

  // 满足套装条件, 随精炼等级加成
  auto func = [&](ItemBase *pBase) {
    ItemEquip *pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip == nullptr || pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == false)
      return;

    // 同一类型装备只生效一次
    if (isCollect.find(pEquip->getTypeID()) != isCollect.end())
      return;

    const SItemCFG* pCfg = pEquip->getCFG();
    if (pCfg == nullptr)
      return;

    for (auto &suitid : pCfg->vecSuitRefineAttrIDs) {
      if (!isSuitValid(suitid))
        continue;

      for (auto &attr : pCfg->mapSuitRefineAttrs) {
        if (pEquip->getRefineLv() < attr.second.dwLevel)
          continue;

        float value = attr.second.fBase * pEquip->getRefineLv();
        UserAttrSvr oAttr;
        oAttr.set_type(static_cast<EAttrType>(attr.first));
        oAttr.set_value(value > attr.second.fMax ? attr.second.fMax : value);
        pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, oAttr);
        /*float value = attr.second.fBase * pEquip->getRefineLv();
        value = vecAttrs[attr.first].value() + (value > attr.second.fMax ? attr.second.fMax : value);
        vecAttrs[attr.first].set_value(value);*/
        isCollect.insert(pEquip->getTypeID());
      }

      break;
    }
  };
  foreach(func);
}

// fashion package
FashionPackage::FashionPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

FashionPackage::~FashionPackage()
{

}

bool FashionPackage::checkFashion(DWORD itemid)
{
  auto m = m_mapTypeID2Item.find(itemid);
  if (m == m_mapTypeID2Item.end())
    return false;

  return m->second.empty() == false;
}

void FashionPackage::addFashion(DWORD itemid)
{
  if (checkFashion(itemid) == true)
    return;

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(itemid);
  if (pCFG == nullptr || ItemManager::getMe().isFashion(pCFG->eEquipType) == false)
    return;

  ItemInfo oItem;
  oItem.set_id(itemid);
  oItem.set_source(ESOURCE_PACKAGE);
  ItemBase* pBase = ItemManager::getMe().createItem(oItem);
  if (pBase == nullptr)
    return;
  pBase->setNew(true);

  string guid = GuidManager::getMe().newGuidStr(m_pUser->getZoneID(), m_pUser->getUserSceneData().getOnlineMapID());
  pBase->setGUID(guid);
  ItemManager::getMe().addGUID(guid);

  m_mapID2Item[guid] = pBase;
  m_mapTypeID2Item[pBase->getTypeID()].insert(pBase);

  // save data and notify
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();
  m_setUpdateIDs.insert(pBase->getGUID());
}

void FashionPackage::decFashion(DWORD itemid)
{
  auto m = m_mapTypeID2Item.find(itemid);
  if (m == m_mapTypeID2Item.end())
    return;

  for (auto s = m->second.begin(); s != m->second.end(); ++s)
  {
    ItemBase* pBase = *s;

    auto o = m_mapID2Item.find(pBase->getGUID());
    if (o != m_mapID2Item.end())
      m_mapID2Item.erase(o);

    m_setUpdateIDs.insert(pBase->getGUID());
    //SAFE_DELETE(pBase);
    addGarbageItem(pBase);
  }

  m_mapTypeID2Item.erase(m);

  // save data and notify
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();
}

// 2016-04-19 申林移除Table_EquipFashion.txt表
/*void FashionPackage::getFashionAttr(TVecAttrSvrs& vecAttrs)
{
  DWORD fashionNum = m_mapID2Item.size();
  ItemConfig::getMe().getFashionAttr(fashionNum, vecAttrs);
}*/

// fashion equip package
FashionEquipPackage::FashionEquipPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

FashionEquipPackage::~FashionEquipPackage()
{

}

bool FashionEquipPackage::fromData(const PackageData& rData)
{
  if (rData.type() != getPackageType())
  {
    XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << rData.ShortDebugString() << "失败,背包类型不对" << XEND;
    return false;
  }

  for (int j = 0; j < rData.items_size() ; ++j)
  {
    const ItemData& item = rData.items(j);
    if (item.base().guid().empty() == true)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,guid为空" << XEND;
      continue;
    }

    ItemBase* pBase = ItemManager::getMe().createItem(item.base());
    if (pBase == nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      continue;
    }
    pBase->fromItemData(item);
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pBase);
    if (pEquip == nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,不是装备" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      SAFE_DELETE(pBase);
      continue;
    }

    if (getEquip(static_cast<EEquipPos>(pEquip->getIndex())) != nullptr)
    {
      XERR << "[包裹-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << rData.type() << "加载物品" << item.ShortDebugString() << "失败,位置有装备" << XEND;
      m_mapID2InvalidItem[item.base().guid()].CopyFrom(item);
      SAFE_DELETE(pBase);
      continue;
    }

    setEquip(m_pUser, static_cast<EEquipPos>(pEquip->getIndex()), pEquip, true);
  }

  clearUpdateIDs();
  m_pUser->onAltmanFashionEquip();
  return true;
}

bool FashionEquipPackage::canEquip(SceneUser* pUser, const ItemBase* pBase)
{
  if (pUser == nullptr)
    return false;

  const ItemEquip* pEquip = dynamic_cast<const ItemEquip*>(pBase);
  EProfession eProfession = pUser->getUserSceneData().getProfession();
  if (pEquip == nullptr || pEquip->canEquip(eProfession) == false)
    return false;
  if (pEquip->canGenderEquip(pUser->getUserSceneData().getGender()) == false)
    return false;
  bool bFashion = ItemManager::getMe().isFashion(pEquip->getEquipType());
  bool bShield = pEquip->getEquipType() == EEQUIPTYPE_SHIELD;
  bool bResult = eProfession>= EPROFESSION_CRUSADER && eProfession <= EPROFESSION_ROYALGUARD ? (bFashion || bShield) : bFashion;
  if (!bResult)
    return false;

  return true;
}

bool FashionEquipPackage::setEquip(SceneUser* pUser, EEquipPos ePos, ItemBase* pBase, bool bInit /*= false*/)
{
  // if base not null and check can equip
  if (pBase != nullptr && !bInit)
  {
    if (canEquip(pUser, pBase) == false)
      return false;
  }

  // release old equip
  ItemEquip* pEquip = getEquip(ePos);
  if (pEquip != nullptr)
  {
    auto m = m_mapID2Item.find(pEquip->getGUID());
    if (m != m_mapID2Item.end())
      m_mapID2Item.erase(m);

    pEquip->setEquiped(false);
    m_setUpdateIDs.insert(pEquip->getGUID());

    if (pEquip->getEquipType() == EEQUIPTYPE_SHIELD)
      m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
    else if (pEquip->getEquipType() == EEQUIPTYPE_BACK)
      m_pUser->setDataMark(EUSERDATATYPE_BACK);
    else if (pEquip->getEquipType() == EEQUIPTYPE_HEAD)
      m_pUser->setDataMark(EUSERDATATYPE_HEAD);
    else if (pEquip->getEquipType() == EEQUIPTYPE_FACE)
      m_pUser->setDataMark(EUSERDATATYPE_FACE);
    else if (pEquip->getEquipType() == EEQUIPTYPE_TAIL)
      m_pUser->setDataMark(EUSERDATATYPE_TAIL);
    else if (pEquip->getEquipType() == EEQUIPTYPE_MOUTH)
      m_pUser->setDataMark(EUSERDATATYPE_MOUTH);
    else if (pEquip->getEquipType() == EEQUIPTYPE_WEAPON)
    {
      m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
      m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
    }
    else if (pEquip->getEquipType() == EEQUIPTYPE_ARMOUR)
    {
      m_pUser->setDataMark(EUSERDATATYPE_BODY);
      m_pUser->onAltmanFashionEquip();
    }
    else if (pEquip->getEquipType() == EEQUIPTYPE_MOUNT)
      m_pUser->setDataMark(EUSERDATATYPE_MOUNT);
    // check item
    m_pUser->getPackage().decItemCount(pEquip->getTypeID());

    if (pEquip->getCFG())
    {
      for (auto &s : pEquip->getCFG()->setFashionBuffIDs)
        m_pUser->m_oBuff.del(s);
    }
  }

  // set new equip
  pEquip = dynamic_cast<ItemEquip*>(pBase);
  if (pEquip == nullptr)
    return false;

  pEquip->setEquiped(true);
  pEquip->setIndex(ePos);

  m_mapID2Item[pEquip->getGUID()] = pEquip;
  m_setUpdateIDs.insert(pEquip->getGUID());

  // check item
  m_pUser->getPackage().addItemCount(pEquip);

  // add fashion buff
  if (pEquip->getCFG())
  {
    for (auto &s : pEquip->getCFG()->setFashionBuffIDs)
      m_pUser->m_oBuff.add(s);
  }
  // update user show data
  if (pEquip->getEquipType() == EEQUIPTYPE_SHIELD)
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
  else if (pEquip->getEquipType() == EEQUIPTYPE_BACK)
    m_pUser->setDataMark(EUSERDATATYPE_BACK);
  else if (pEquip->getEquipType() == EEQUIPTYPE_HEAD)
    m_pUser->setDataMark(EUSERDATATYPE_HEAD);
  else if (pEquip->getEquipType() == EEQUIPTYPE_FACE)
    m_pUser->setDataMark(EUSERDATATYPE_FACE);
  else if (pEquip->getEquipType() == EEQUIPTYPE_TAIL)
    m_pUser->setDataMark(EUSERDATATYPE_TAIL);
  else if (pEquip->getEquipType() == EEQUIPTYPE_MOUTH)
    m_pUser->setDataMark(EUSERDATATYPE_MOUTH);
  else if (pEquip->getEquipType() == EEQUIPTYPE_WEAPON)
  {
    m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
  }
  else if (pEquip->getEquipType() == EEQUIPTYPE_ARMOUR)
  {
    m_pUser->setDataMark(EUSERDATATYPE_BODY);
    m_pUser->onAltmanFashionEquip();
  }
  else if (pEquip->getEquipType() == EEQUIPTYPE_MOUNT)
    m_pUser->setDataMark(EUSERDATATYPE_MOUNT);
  return true;
}

ItemEquip* FashionEquipPackage::getEquip(EEquipPos ePos)
{
  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
  {
    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(m->second);
    if (pEquip != nullptr && pEquip->getIndex() == ePos)
      return pEquip;
  }

  return nullptr;
}

void FashionEquipPackage::collectInValidEquip(TSetString& setIDs)
{
  setIDs.clear();
  for (auto &m : m_mapID2Item)
  {
    if (canEquip(m_pUser, m.second) == false)
      setIDs.insert(m.first);
  }
}

// store package
StorePackage::StorePackage(SceneUser* pUser) : BasePackage(pUser)
{

}

StorePackage::~StorePackage()
{

}

/*void StorePackage::updateToData(std::function<void(EError)> func)
{
  if (m_setUpdateIDs.empty() == true)
    return;

  Cmd::PackageUpdateItemSCmd cmd;
  cmd.set_accid(m_pUser->accid);
  cmd.set_charid(m_pUser->id);
  cmd.set_scenename(thisServer->getServerName());

  for (auto &s : m_setUpdateIDs)
  {
    ItemBase* pBase = getItem(s);
    if (pBase != nullptr)
      pBase->toItemData(cmd.add_updates());
    else
      cmd.add_dels(s);
  }

  DWORD dwSessionID = GuidManager::getMe().getNextGuildGmSessionID();
  cmd.set_sessionid(dwSessionID);
  m_mapRecallFuncs[dwSessionID] = func;

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);

  XLOG << "[包裹-" << "\b" << getPackageType() << "\b]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "向DataServer更新数据" << cmd.ShortDebugString() << XEND;
}

void StorePackage::doFunc(DWORD dwSessionID, EError eError)
{
  auto m = m_mapRecallFuncs.find(dwSessionID);
  if (m != m_mapRecallFuncs.end())
  {
    m->second(eError);
    m_mapRecallFuncs.erase(m);
    XLOG << "[包裹-" << "\b" << getPackageType() << "\b]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "sessionid :" << dwSessionID << "被执行" << XEND;
  }
}*/

// personal store package
PersonalStorePackage::PersonalStorePackage(SceneUser* pUser) : BasePackage(pUser)
{

}

PersonalStorePackage::~PersonalStorePackage()
{

}

// temp main package
TempMainPackage::TempMainPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

TempMainPackage::~TempMainPackage()
{

}

bool TempMainPackage::checkAddItemObj(const ItemBase* pBase)
{
  refreshDeleteTimeItems();
  return BasePackage::checkAddItemObj(pBase);
}

bool TempMainPackage::addItem(ItemInfo& rInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  bool b = BasePackage::addItem(rInfo, eMethod, bShow, bInform, bForceShow);
  refreshDeleteTimeItems();
  return b;
}

bool TempMainPackage::addItem(TVecItemInfo& vecInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  bool b = BasePackage::addItem(vecInfo, eMethod, bShow, bInform, bForceShow);
  refreshDeleteTimeItems();
  return b;
}

bool TempMainPackage::addItem(ItemData& rData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  bool b = BasePackage::addItem(rData, eMethod, bShow, bInform, bForceShow);
  refreshDeleteTimeItems();
  return b;
}

bool TempMainPackage::addItem(TVecItemData& vecData, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/, bool bShow /*= false*/, bool bInform /*= true*/, bool bForceShow /*= false*/)
{
  bool b = BasePackage::addItem(vecData, eMethod, bShow, bInform, bForceShow);
  refreshDeleteTimeItems();
  return b;
}

void TempMainPackage::reduceItem(const string& guid, ESource source, DWORD count /*= 1*/, bool bDelete /*= true*/, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  BasePackage::reduceItem(guid, source, count, bDelete, eMethod);
  refreshDeleteTimeItems();
}

void TempMainPackage::refreshDeleteTimeItems()
{
  if (m_mapID2Item.size() <= MiscConfig::getMe().getItemCFG().dwTempPackOverTimeCount)
  {
    for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end(); ++m)
    {
      m->second->setOverTime(0);
      m_setUpdateIDs.insert(m->first);
    }
    return;
  }

  DWORD dwNow = xTime::getCurSec();
  DWORD dwOverTime = MiscConfig::getMe().getItemCFG().dwTempMainOvertime;
  DWORD noSaleCnt = 0;

  for (auto m = m_mapID2Item.begin(); m != m_mapID2Item.end();)
  {
    if (m->second->getCFG() && m->second->getCFG()->bNoSale)
    {
      ++noSaleCnt;
      ++m;
      continue;
    }

    if (m->second->getOverTime() == 0)
    {
      m->second->setOverTime(dwNow + dwOverTime);
      m_setUpdateIDs.insert(m->first);
      ++m;
      continue;
    }

    if (m->second->getOverTime() > dwNow)
    {
      ++m;
      continue;
    }

    ItemBase* pBase = m->second;
    m = m_mapID2Item.erase(m);

    auto type = m_mapTypeID2Item.find(pBase->getTypeID());
    if (type != m_mapTypeID2Item.end())
      type->second.erase(pBase);

    m_mapDeleteID2Item.erase(pBase->getGUID());

    m_setUpdateIDs.insert(pBase->getGUID());
    //log
    {
      QWORD eid = xTime::getCurUSec();
      DWORD eventType = EventType_TempPackDismiss;
      PlatLogManager::getMe().consumeItemLog(thisServer,
        m_pUser->getUserSceneData().getPlatformId(),
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eid,
        m_pUser->getUserSceneData().getCharge(),
        pBase->getTypeID(),/*itemid */
        pBase->getCount(), 
        0,
        ESOURCE_TEMPPACK,
        eventType
      );

     /* PlatLogManager::getMe().eventLog(thisServer,
        m_pUser->getUserSceneData().getPlatformId(),
        m_pUser->getZoneID(),
        m_pUser->accid,
        m_pUser->id,
        eid,
        m_pUser->getUserSceneData().getCharge(), eventType, 0, 1);*/
    }

    XLOG << "[包裹-临时包裹]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << pBase->getGUID() << pBase->getTypeID() << pBase->getCount() << "超过" << dwOverTime / HOUR_T << "小时,自动删除" << XEND;
    //SAFE_DELETE(pBase);
    addGarbageItem(pBase);
  }

  if (m_setUpdateIDs.empty() == false)
    m_pUser->getTip().addRedTip(EREDSYS_PACK_TEMP);

  XLOG << "[包裹-临时包裹]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行了临时背包过期道具刷新" << XEND;
}

void TempMainPackage::update(DWORD curSec)
{
  if (curSec > m_dwNextRefreshTime)
  {
    refreshDeleteTimeItems();
    m_dwNextRefreshTime = curSec + 300;
  }
  BasePackage::update(curSec);
}

// barrow package
BarrowPackage::BarrowPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

BarrowPackage::~BarrowPackage()
{

}

bool BarrowPackage::checkAddItem(const ItemInfo& rInfo, EPackMethod eMethod /*= EPACKMETHOD_CHECK_NOPILE*/)
{
  if (BasePackage::checkAddItem(rInfo, eMethod) == false)
    return false;

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rInfo.id());
  if (pCFG == nullptr || pCFG->eItemType == EITEMTYPE_BARROW)
    return false;
  return true;
}

// quest package
QuestPackage::QuestPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

QuestPackage::~QuestPackage()
{

}

// food package
FoodPackage::FoodPackage(SceneUser* pUser) : BasePackage(pUser)
{

}

FoodPackage::~FoodPackage()
{

}

DWORD FoodPackage::getItemCount(DWORD id, ECheckMethod eMethod /*= ECHECKMETHOD_NONORMALEQUIP*/)
{
  if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == false)
    return 0;
  return BasePackage::getItemCount(id, eMethod);
}

bool FoodPackage::checkItemCount(DWORD itemid, DWORD count /*= 1*/, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == false)
    return false;
  return BasePackage::checkItemCount(itemid, count, eMethod);
}

bool FoodPackage::checkItemCount(const string& guid, DWORD count /*= 1*/, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == false)
    return false;
  return BasePackage::checkItemCount(guid, count, eMethod);
}

bool FoodPackage::checkItemCount(const TVecItemInfo& vecItems, ECheckMethod eMethod /*= ECHECKMETHOD_NORMAL*/)
{
  if (m_pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == false)
    return false;
  return BasePackage::checkItemCount(vecItems, eMethod);
}

