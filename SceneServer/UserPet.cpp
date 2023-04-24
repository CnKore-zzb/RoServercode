#include "UserPet.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "PetConfig.h"
#include "MsgManager.h"
#include "SkillManager.h"
#include "PlatLogManager.h"
#include "Menu.h"

bool SScenePetData::toEggData(EggData* pData)
{
  if (pData == nullptr)
    return false;
  pData->set_exp(qwExp);
  pData->set_friendexp(qwFriendExp);
  pData->set_rewardexp(qwRewardExp);

  pData->set_id(dwID);
  pData->set_lv(dwLv);
  pData->set_friendlv(dwFriendLv);
  pData->set_body(dwBody);
  pData->set_relivetime(dwReliveTime);
  pData->set_hp(dwHp);
  pData->set_restoretime(xTime::getCurSec());
  pData->set_time_happly(dwTimeHapply);
  pData->set_time_excite(dwTimeExcite);
  pData->set_time_happiness(dwTimeHappiness);
  pData->set_time_happly_gift(dwTimeHapplyGift);
  pData->set_time_excite_gift(dwTimeExciteGift);
  pData->set_time_happiness_gift(dwTimeHappinessGift);
  pData->set_touch_tick(dwTouchTick);
  pData->set_feed_tick(dwFeedTick);
  pData->set_guid(strGUID);
  pData->set_name(strName);
  pData->set_skilloff(bSkillOff);

  BlobVar oBlob;
  if (oVar.save(&oBlob) == true)
  {
    string data;
    if (oBlob.SerializeToString(&data) == true)
      pData->set_var(data.c_str(), data.size());
  }

  if (bLive)
  {
    SceneNpc* pPet = SceneNpcManager::getMe().getNpcByTempID(qwTempID);
    if (pPet && pPet->getNpcType() == ENPCTYPE_PETNPC)
    {
      oBuff.Clear();
      pPet->m_oBuff.save(&oBuff);
      if (oBuff.list_size())
      {
        string data;
        if (oBuff.SerializeToString(&data))
          pData->set_buff(data.c_str(), data.size());
      }
    }
  }

  pData->clear_skillids();
  for (auto &s : setSkillIDs)
    pData->add_skillids(s);
  pData->clear_unlock_equip();
  for (auto &s : setUnlockEquipIDs)
    pData->add_unlock_equip(s);
  pData->clear_unlock_body();
  for (auto &s : setUnlockBodyIDs)
    pData->add_unlock_body(s);

  if (pEquip != nullptr)
    pEquip->toEggEquip(pData->add_equips());

  // 初始变装
  for (auto &m : mapInitPos2EquipID)
  {
    PetEquipData* p = pData->add_defaultwears();
    if (p == nullptr)
      continue;
    p->set_epos(m.first);
    p->set_itemid(m.second);
  }
  // 当前变装
  for (auto &m : mapCurPos2EquipID)
  {
    PetEquipData* p = pData->add_wears();
    if (p == nullptr)
      continue;
    p->set_epos(m.first);
    p->set_itemid(m.second);
  }

  pData->set_version(EGG_VERSION);
  return true;
}

bool SScenePetData::fromEggData(const EggData& rData)
{
  qwExp = rData.exp();
  qwFriendExp = rData.friendexp();
  qwRewardExp = rData.rewardexp();

  dwID = rData.id();
  dwLv = rData.lv();
  dwFriendLv = rData.friendlv();
  dwBody = rData.body();
  dwReliveTime = rData.relivetime();
  dwHp = rData.hp();

  dwRestoreTime = rData.restoretime();
  dwTimeHapply = rData.time_happly();
  dwTimeExcite = rData.time_excite();
  dwTimeHappiness = rData.time_happiness();

  dwTimeHapplyGift = rData.time_happly_gift();
  dwTimeExcite = rData.time_excite_gift();
  dwTimeHappinessGift = rData.time_happiness_gift();

  dwTouchTick = rData.touch_tick();
  dwFeedTick = rData.feed_tick();
  bSkillOff = rData.skilloff();

  // 计算心情值
  DWORD dwNow = xTime::getCurSec();
  if (dwRestoreTime != 0 && dwTimeHapply > dwRestoreTime && dwNow > dwRestoreTime)
  {
    DWORD dwDelta = dwNow > dwTimeHapply ? dwTimeHapply - dwRestoreTime : dwNow - dwRestoreTime;
    dwTimeHapply += dwDelta;
    XDBG << "[宠物-加载] petid :" << dwID << "开心心情离线, 好感度延迟" << dwDelta << XEND;

    dwTimeHapplyGift += dwDelta;
    XDBG << "[宠物-加载] petid :" << dwID << "开心心情离线, 礼物值延迟" << dwDelta << XEND;
  }
  if (dwRestoreTime != 0 && dwTimeExcite > dwRestoreTime && dwNow > dwRestoreTime)
  {
    DWORD dwDelta = dwNow > dwTimeExcite ? dwTimeExcite - dwRestoreTime : dwNow - dwRestoreTime;
    dwTimeExcite += dwDelta;
    XDBG << "[宠物-加载] petid :" << dwID << "兴奋心情离线, 好感度延迟" << dwDelta << XEND;

    dwTimeExciteGift += dwDelta;
    XDBG << "[宠物-加载] petid :" << dwID << "兴奋心情离线, 礼物值延迟" << dwDelta << XEND;
  }
  if (dwTimeHappiness > dwRestoreTime && dwNow > dwRestoreTime)
  {
    DWORD dwDelta = dwNow > dwTimeHappiness ? dwTimeHappiness - dwRestoreTime : dwNow - dwRestoreTime;
    dwTimeHappiness += dwDelta;
    XDBG << "[宠物-加载] petid :" << dwID << "幸福心情离线, 好感度延迟" << dwDelta << XEND;

    dwTimeHappinessGift += dwDelta;
    XDBG << "[宠物-加载] petid :" << dwID << "幸福心情离线, 礼物值延迟" << dwDelta << XEND;
  }
  /*if (dwRestoreTime != 0 && dwNow > dwRestoreTime)
    addFriendExp((dwNow - dwRestoreTime) / MIN_T);*/
  dwTimeTick = dwNow + MIN_T;

  strGUID = rData.guid();
  strName = rData.name();

  BlobVar oBlob;
  if (oBlob.ParseFromString(rData.var()) == true)
    oVar.load(oBlob);

  setSkillIDs.clear();
  for (int i = 0; i < rData.skillids_size(); ++i)
    setSkillIDs.insert(rData.skillids(i));
  setUnlockEquipIDs.clear();
  for (int i = 0; i < rData.unlock_equip_size(); ++i)
    setUnlockEquipIDs.insert(rData.unlock_equip(i));
  setUnlockBodyIDs.clear();
  for (int i = 0; i < rData.unlock_body_size(); ++i)
    setUnlockBodyIDs.insert(rData.unlock_body(i));

  refreshFriendLevelOpen();

  if (rData.equips_size() != 0)
  {
    const EggEquip& rEquip = rData.equips(0);
    pEquip = dynamic_cast<ItemEquip*>(ItemManager::getMe().createItem(rEquip.base()));
    if (pEquip != nullptr)
      pEquip->fromEggEquip(rEquip);
    else
      XERR << "[宠物-加载] petid :" << dwID << "加载装备" << rEquip.ShortDebugString() << "失败,创建对象失败" << XEND;
  }

  if (now() >= dwReliveTime)
    bLive = true;

  pCFG = PetConfig::getMe().getPetCFG(dwID);
  if (pCFG == nullptr)
  {
    XERR << "[宠物-加载], petid:" << dwID << "找不到对应配置" << XEND;
    return false;
  }

  if (bLive)
  {
    oBuff.ParseFromString(rData.buff());
  }

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  dwActionTouchTick = dwNow + randBetween(0, rCFG.dwUserPetTimeTick / 3);
  dwActionFeedTick = dwNow + randBetween(rCFG.dwUserPetTimeTick / 3, rCFG.dwUserPetTimeTick / 2);
  dwActionIdleTick = dwNow + randBetween(rCFG.dwUserPetTimeTick / 2, rCFG.dwUserPetTimeTick);

  mapInitPos2EquipID.clear();
  for (int i = 0; i < rData.defaultwears_size(); ++i)
  {
    auto &r = rData.defaultwears(i);
    mapInitPos2EquipID[r.epos()] = r.itemid();
  }
  mapCurPos2EquipID.clear();
  for (int i = 0; i < rData.wears_size(); ++i)
  {
    auto &r = rData.wears(i);
    mapCurPos2EquipID[r.epos()] = r.itemid();
  }

  dwVersion = rData.version();
  for (DWORD i = dwVersion; i < EGG_VERSION; ++i)
  {
    if (i == 0)
      patch_1();
    else if (i == 1)
      patch_2();
    else if (i == 2)
      patch_3();
    else if (i == 3)
      patch_4();
  }
  dwVersion = EGG_VERSION;

  obitset.reset();
  return true;
}

bool SScenePetData::toPetData(UserPetData* pData)
{
  if (pData == nullptr)
    return false;
  EggData* pEggData = pData->mutable_basedata();
  if (toEggData(pEggData) == false)
    return false;

  pData->set_inhand(bHandStatus);
  // ..
  return true;
}

bool SScenePetData::fromPetData(const UserPetData& rData)
{
  if (fromEggData(rData.basedata()) == false)
    return false;
  // ..
  bHandStatus = rData.inhand();
  return true;
}

bool SScenePetData::toClientData(PetInfo* pInfo)
{
  if (pInfo == nullptr)
    return false;
  pInfo->set_exp(qwExp);
  pInfo->set_friendexp(qwFriendExp);
  pInfo->set_rewardexp(qwRewardExp);

  pInfo->set_time_happly(dwTimeHapply);
  pInfo->set_time_excite(dwTimeExcite);
  pInfo->set_time_happiness(dwTimeHappiness);
  pInfo->set_body(dwBody);

  pInfo->set_petid(dwID);
  pInfo->set_name(strName);
  pInfo->set_lv(dwLv);
  pInfo->set_friendlv(dwFriendLv);
  pInfo->set_relivetime(dwReliveTime);
  pInfo->set_guid(qwTempID);
  pInfo->set_skilloff(bSkillOff);

  for (auto &s : setSkillIDs)
    pInfo->add_skills(s);

  for (auto &s : setUnlockEquipIDs)
    pInfo->add_unlock_equip(s);
  for (auto &s : setUnlockBodyIDs)
    pInfo->add_unlock_body(s);

  if (pEquip != nullptr)
    pEquip->toItemData(pInfo->add_equips());

  return true;
}

bool SScenePetData::reload()
{
  pCFG = PetConfig::getMe().getPetCFG(dwID);
  return pCFG != nullptr;
}

void SScenePetData::addBaseExp(QWORD exp)
{
  if (exp == 0)
    return;
  qwExp += exp;
  obitset.set(EPETDATA_EXP);
  baseLevelup();
  updateData();
}

void SScenePetData::addFriendExp(QWORD exp)
{
  if (exp == 0)
    return;
  qwFriendExp += exp;
  obitset.set(EPETDATA_FRIENDEXP);
  friendLevelup();
  updateData();
}

void SScenePetData::addRewardExp(QWORD exp)
{
  if (exp == 0)
    return;
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  if (qwRewardExp >= rCFG.dwUserPetGiftMaxValue)
  {
    XDBG << "[宠物-礼物值]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "的宠物" << dwID << "礼物值超过上限" << rCFG.dwUserPetGiftMaxValue << XEND;
    return;
  }
  qwRewardExp += exp;
  if (qwRewardExp >= MiscConfig::getMe().getPetCFG().dwUserPetGiftMaxValue)
    qwRewardExp = MiscConfig::getMe().getPetCFG().dwUserPetGiftMaxValue;
  obitset.set(EPETDATA_REWARDEXP);
  updateData();
}

void SScenePetData::subRewardExp(QWORD exp)
{
  if (exp == 0)
    return;
  if (qwRewardExp < exp)
    return;
  qwRewardExp -= exp;
  obitset.set(EPETDATA_REWARDEXP);
  updateData();
}

void SScenePetData::baseLevelup()
{
  if (pOwner == nullptr)
    return;

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD maxlv = rCFG.dwOverUserLv + pOwner->getLevel();

  bool bup = false;
  DWORD dwBefore = dwLv;
  while (true)
  {
    const SPetBaseLvCFG* pCFG = PetConfig::getMe().getPetBaseLvCFG(dwLv + 1);
    if (pCFG == nullptr || qwExp < pCFG->qwNewCurExp)
      break;
    if (dwLv >= maxlv)
      break;

    qwExp -= pCFG->qwNewCurExp;
    dwLv += 1;
    bup = true;
  }
  if (bup)
  {
    obitset.set(EPETDATA_LV);
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(qwTempID);
    if (npc && npc->define.m_oVar.m_qwOwnerID == pOwner->id)
    {
      npc->define.setLevel(dwLv);
      npc->setCollectMark(ECOLLECTTYPE_BASE);
      npc->updateAttribute();
      npc->setAttr(EATTRTYPE_HP, npc->getAttr(EATTRTYPE_MAXHP));
      npc->refreshDataAtonce();
    }

    pOwner->getAchieve().onPetBaseLvUp();
    pOwner->getQuest().onPetAdd();
    pOwner->getMenu().refreshNewMenu(EMENUCOND_PET);
    XLOG << "[宠物-基础升级]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "的宠物" << dwID << "等级" << dwBefore << "升级到" << dwLv << XEND;

    //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Pet_Change;
    EPetChangeType eChangeType = EPetChangeType_Lv;

    PlatLogManager::getMe().eventLog(thisServer,
      pOwner->getUserSceneData().getPlatformId(),
      pOwner->getZoneID(),
      pOwner->accid,
      pOwner->id,
      eid,
      pOwner->getUserSceneData().getCharge(), eType, 0, 1);

    DWORD dwAfter = dwLv;
    string strBefore;
    string strAfter;

    PlatLogManager::getMe().PetChangeLog(thisServer,
      pOwner->accid,
      pOwner->id,
      eType,
      eid,
      
      eChangeType,
      dwID,
      strName,
      dwBefore,
      dwAfter,
      strBefore,
      strAfter
    );    
  }
}

void SScenePetData::friendLevelup()
{
  if (pOwner == nullptr)
    return;

  bool bUp = false;
  TSetDWORD setNewEquipIDs;
  TSetDWORD setNewBodyIDs;
  DWORD dwBefore = dwFriendLv;
  while (true)
  {
    const SPetFriendLvCFG* pCFG = PetConfig::getMe().getFriendLvCFG(dwID, dwFriendLv + 1);
    if (pCFG == nullptr || qwFriendExp < pCFG->qwExp)
      break;

    dwFriendLv += 1;
    obitset.set(EPETDATA_FRIENDLV);
    bUp = true;

    for (auto &s : pCFG->setUnlockEquipIDs)
    {
      if (setUnlockEquipIDs.find(s) == setUnlockEquipIDs.end())
      {
        setUnlockEquipIDs.insert(s);
        setNewEquipIDs.insert(s);
      }
    }
    for (auto &s : pCFG->setUnlockBodyIDs)
    {
      if (setUnlockBodyIDs.find(s) == setUnlockBodyIDs.end())
      {
        setUnlockBodyIDs.insert(s);
        setNewBodyIDs.insert(s);
      }
    }
  }

  if (bUp)
  {
    pOwner->getAchieve().onPetFriendLvUp(dwFriendLv);
    pOwner->getQuest().onPetAdd();
    pOwner->getMenu().refreshNewMenu(EMENUCOND_PET);
    const SPetFriendLvCFG* pCFG = PetConfig::getMe().getFriendLvCFG(dwID, dwFriendLv + 1);
    if(pCFG == nullptr)
      pOwner->getServant().onGrowthFinishEvent(ETRIGGER_PET_FRIEND_FULL);
    XLOG << "[宠物-好感度升级]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name
      << "的宠物" << dwID << "等级" << dwFriendLv - 1 << "升级到" << dwFriendLv << "解锁装备" << setNewEquipIDs << "解锁皮肤" << setNewBodyIDs << XEND;

    //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Pet_Change;
    EPetChangeType eChangeType = EPetChangeType_Good;

    PlatLogManager::getMe().eventLog(thisServer,
      pOwner->getUserSceneData().getPlatformId(),
      pOwner->getZoneID(),
      pOwner->accid,
      pOwner->id,
      eid,
      pOwner->getUserSceneData().getCharge(), eType, 0, 1);

    DWORD dwAfter = dwFriendLv;
    string strBefore;
    string strAfter;

    PlatLogManager::getMe().PetChangeLog(thisServer,
      pOwner->accid,
      pOwner->id,
      eType,
      eid,

      eChangeType,
      dwID,
      strName,
      dwBefore,
      dwAfter,
      strBefore,
      strAfter
    );
  }
  if (setNewEquipIDs.empty() == false || setNewBodyIDs.empty() == false)
  {
    UnlockNtfPetCmd cmd;
    cmd.set_petid(dwID);
    for (auto &s : setNewEquipIDs)
      cmd.add_equipids(s);
    for (auto &s : setNewBodyIDs)
      cmd.add_bodys(s);
    PROTOBUF(cmd, send, len);
    pOwner->sendCmdToMe(send, len);
    pOwner->getAchieve().onPetEquip();

    if (setNewBodyIDs.empty() == false && pCFG != nullptr)
      MsgManager::sendMsg(pOwner->id, ESYSTEMMSG_ID_PET_NEW_BODY, MsgParams(pCFG->strName));
  }
}

void SScenePetData::refreshFriendLevelOpen()
{
  for (DWORD d = 1; d <= dwFriendLv; ++d)
  {
    const SPetFriendLvCFG* pCFG = PetConfig::getMe().getFriendLvCFG(dwID, d);
    if (pCFG == nullptr)
      continue;

    for (auto &s : pCFG->setUnlockEquipIDs)
    {
      if (setUnlockEquipIDs.find(s) == setUnlockEquipIDs.end())
        setUnlockEquipIDs.insert(s);
    }
    for (auto &s : pCFG->setUnlockBodyIDs)
    {
      if (setUnlockBodyIDs.find(s) == setUnlockBodyIDs.end())
        setUnlockBodyIDs.insert(s);
    }
  }
}

bool SScenePetData::addFeelingTime(EPetDataType eType, DWORD dwTime)
{
  DWORD dwNow = xTime::getCurSec();
  if (eType == EPETDATA_TIME_HAPPLY)
  {
    dwTimeHapply = dwTimeHapply < dwNow ? dwNow + dwTime : dwTimeHapply + dwTime;
    dwTimeHapplyGift = dwTimeHapplyGift < dwNow ? dwNow + dwTime : dwTimeHapplyGift + dwTime;
  }
  else if (eType == EPETDATA_TIME_EXCITE)
  {
    dwTimeExcite = dwTimeExcite < dwNow ? dwNow + dwTime : dwTimeExcite + dwTime;
    dwTimeExciteGift = dwTimeExciteGift < dwNow ? dwNow + dwTime : dwTimeExciteGift + dwTime;
  }
  else if (eType == EPETDATA_TIME_HAPPINESS)
  {
    dwTimeHappiness = dwTimeHappiness < dwNow ? dwNow + dwTime : dwTimeHappiness + dwTime;
    dwTimeHappinessGift = dwTimeHappinessGift < dwNow ? dwNow + dwTime : dwTimeHappinessGift + dwTime;
  }
  else
    return false;

  obitset.set(eType);
  updateData();
  return true;
}

EError SScenePetData::canTouch()
{
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD dwTouchCount = oVar.getVarValue(EVARTYPE_USERPET_TOUCH);
  if (dwTouchCount >= rCFG.dwUserPetTouchPerDay)
    return EERROR_PET_TOUCH_MAX;

  DWORD dwNow = xTime::getCurSec();
  if (dwNow < dwTouchTick)
    return EERROR_PET_TOUCH_CD;

  return EERROR_SUCCESS;
}

bool SScenePetData::touch()
{
  EError eErr = canTouch();
  if (eErr != EERROR_SUCCESS)
  {
    XERR << "[宠物-抚摸]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "抚摸宠物 :" << dwID << "失败, error :" << eErr << XEND;
    return false;
  }

  DWORD dwTouchCount = oVar.getVarValue(EVARTYPE_USERPET_TOUCH);
  DWORD dwNow = xTime::getCurSec();
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();

  setTouchTick(dwNow + rCFG.dwUserPetTouchTick);
  addFriendExp(rCFG.dwUserPetTouchFriend);
  addFeelingTime(EPETDATA_TIME_HAPPLY, rCFG.dwUserPetTouchTime);
  oVar.setVarValue(EVARTYPE_USERPET_TOUCH, dwTouchCount + 1);
  obitset.set(EPETDATA_TOUCH_COUNT);

  playAction(EPETACTION_OWNER_TOUCH);
  pOwner->getAchieve().onPetTouch();

  XLOG << "[宠物-抚摸]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name
    << "抚摸宠物 :" << dwID << "成功, Happly心情 :" << dwTimeHapply << "HapplyGift :" << dwTimeHapplyGift << "当天抚摸次数 :" << (dwTouchCount + 1) << XEND;
  return true;
}

EError SScenePetData::canFeed()
{
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD dwFeedCount = oVar.getVarValue(EVARTYPE_USERPET_FEED);
  if (dwFeedCount >= rCFG.dwUserPetFeedPerDay)
    return EERROR_PET_FEED_MAX;

  DWORD dwNow = xTime::getCurSec();
  if (dwNow < dwFeedTick)
    return EERROR_PET_FEED_CD;

  return EERROR_SUCCESS;
}

bool SScenePetData::feed()
{
  EError eErr = canFeed();
  if (eErr != EERROR_SUCCESS)
  {
    XERR << "[宠物-喂食]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "喂食宠物 :" << dwID << "失败,error :" << eErr << XEND;
    playAction(EPETACTION_OWNER_NO_FEED);
    return false;
  }

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD dwFeedCount = oVar.getVarValue(EVARTYPE_USERPET_FEED);
  DWORD dwNow = xTime::getCurSec();

  setFeedTick(dwNow + rCFG.dwUserPetFeedTick);
  addFriendExp(rCFG.dwUserPetFeedFriend);
  addFeelingTime(EPETDATA_TIME_EXCITE, rCFG.dwUserPetFeedTime);
  oVar.setVarValue(EVARTYPE_USERPET_FEED, dwFeedCount + 1);
  obitset.set(EPETDATA_FEED_COUNT);

  playAction(EPETACTION_OWNER_FEED);
  pOwner->getAchieve().onPetFeed();
  XLOG << "[宠物-喂食]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name
    << "喂食宠物 :" << dwID << "成功, Excite心情 :" << dwTimeExcite << "ExciteGift :" << dwTimeExciteGift << "当天喂食次数 :" << (dwFeedCount + 1) << XEND;
  return true;
}

bool SScenePetData::playAction(EPetAction eAction)
{
  if (pCFG == nullptr)
    return false;
  const SPetActionCFG* pActionCFG = pCFG->getActionCFG(eAction);
  if (pActionCFG == nullptr)
    return false;

  if (eAction == EPETACTION_PET_TOUCH && canTouch() != EERROR_SUCCESS)
    return false;
  else if (eAction == EPETACTION_PET_FEED && canFeed() != EERROR_SUCCESS)
    return false;

  PetNpc* pNpc = dynamic_cast<PetNpc*>(SceneNpcManager::getMe().getNpcByTempID(qwTempID));
  if (pNpc == nullptr)
    return false;

  DWORD dwRand = randBetween(0, 10000);
  if (dwRand > pActionCFG->dwRate)
  {
    XDBG << "[宠物-行为]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "petid :" << dwID << "action :" << eAction << dwRand << pActionCFG->dwRate << XEND;
    return false;
  }
  DWORD dwNow = xTime::getCurSec();
  if (arrActionCD[eAction] > dwNow)
  {
    XDBG << "[宠物-行为]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "petid :" << dwID << "action :" << eAction << arrActionCD[eAction] << XEND;
    return false;
  }
  arrActionCD[eAction] = dwNow + pActionCFG->dwCD;

  DWORD* p = randomStlContainer(pActionCFG->setTalkIDs);
  if (p != nullptr && pNpc->getMasterUser()->getUserSceneData().getOption(EOPTIONTYPE_USE_PETTALK))
  {
    pNpc->sendTalkInfo(*p);
    XDBG << "[宠物-行为]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "petid :" << dwID << "action :" << eAction << "说话 :" << *p << XEND;
  }

  p = randomStlContainer(pActionCFG->setExpressionIDs);
  if (p != nullptr)
  {
    UserActionNtf cmd;
    cmd.set_charid(pNpc->id);
    cmd.set_value(*p);
    cmd.set_type(EUSERACTIONTYPE_EXPRESSION);
    PROTOBUF(cmd, send, len);
    pNpc->sendCmdToNine(send, len);
    XDBG << "[宠物-行为]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "petid :" << dwID << "action :" << eAction << "表情 :" << *p << XEND;
  }

  p = randomStlContainer(pActionCFG->setActionIDs);
  if (p != nullptr)
  {
    UserActionNtf cmd;
    cmd.set_charid(pNpc->id);
    cmd.set_value(*p);
    cmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
    PROTOBUF(cmd, send, len);
    pNpc->sendCmdToNine(send, len);
    XDBG << "[宠物-行为]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "petid :" << dwID << "action :" << eAction << "动作 :" << *p << XEND;
  }

  return true;
}

void SScenePetData::processAction(DWORD curSec)
{
  if (dwActionTouchTick < curSec)
  {
    playAction(EPETACTION_PET_TOUCH);
    dwActionTouchTick = curSec + MiscConfig::getMe().getPetCFG().dwUserPetTimeTick;
  }

  if (dwActionFeedTick < curSec)
  {
    playAction(EPETACTION_PET_FEED);
    dwActionFeedTick = curSec + MiscConfig::getMe().getPetCFG().dwUserPetTimeTick;
  }

  if (dwActionIdleTick < curSec)
  {
    playAction(EPETACTION_PET_IDLE);
    dwActionIdleTick = curSec + MiscConfig::getMe().getPetCFG().dwUserPetTimeTick;
  }
}

void SScenePetData::updateData()
{
  if (pOwner == nullptr || obitset.any() == false)
    return;
  PetInfoUpdatePetCmd cmd;
  cmd.set_petid(dwID);

  auto adddata = [&](EPetDataType etype, DWORD value, const string& name = "")
  {
    PetMemberData* pData = cmd.add_datas();
    if (pData == nullptr)
      return;
    pData->set_etype(etype);
    pData->set_value(value);
    pData->set_data(name);
  };

  for (int i = EPETDATA_MIN + 1; i < EPETDATA_MAX; ++i)
  {
    EPetDataType s = static_cast<EPetDataType>(i);
    if (obitset.test(s) == false)
      continue;
    switch (s)
    {
      case EPETDATA_LV:
        adddata(s, dwLv);
        playAction(EPETACTION_PET_BASELVUP);
        break;
      case EPETDATA_EXP:
        adddata(s, qwExp);
        break;
      case EPETDATA_FRIENDLV:
        adddata(s, dwFriendLv);
        playAction(EPETACTION_PET_FRIENDLVUP);
        break;
      case EPETDATA_FRIENDEXP:
        adddata(s, qwFriendExp);
        break;
      case EPETDATA_RELIVETIME:
        adddata(s, dwReliveTime);
        break;
      case EPETDATA_REWARDEXP:
        adddata(s, qwRewardExp);
        break;
      case EPETDATA_TIME_HAPPLY:
        adddata(s, dwTimeHapply);
        break;
      case EPETDATA_TIME_EXCITE:
        adddata(s, dwTimeExcite);
        break;
      case EPETDATA_TIME_HAPPINESS:
        adddata(s, dwTimeHappiness);
        break;
      case EPETDATA_TOUCH_TICK:
        adddata(s, dwTouchTick);
        break;
      case EPETDATA_TOUCH_COUNT:
        adddata(s, oVar.getVarValue(EVARTYPE_USERPET_TOUCH));
        break;
      case EPETDATA_FEED_TICK:
        adddata(s, dwFeedTick);
        break;
      case EPETDATA_FEED_COUNT:
        adddata(s, oVar.getVarValue(EVARTYPE_USERPET_FEED));
        break;
      case EPETDATA_REWARD_COUNT:
        adddata(s, oVar.getVarValue(EVARTYPE_USERPET_GIFT));
        break;
      case EPETDATA_BODY:
        adddata(s, dwBody);
        break;
      case EPETDATA_SKILL:
        {
          PetMemberData* pData = cmd.add_datas();
          pData->set_etype(s);
          for (auto &s : setSkillIDs)
            pData->add_values(s);
        }
        break;
      case EPETDATA_NAME:
        adddata(s, 0, strName);
        break;
      case EPETDATA_SKILLSWITCH:
        adddata(s, bSkillOff);
        break;
      default:
        break;
    }
  }
  obitset.reset();

  if (cmd.datas_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    pOwner->sendCmdToMe(send, len);

    XDBG << "[宠物-数据更新]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "收到更新" << cmd.ShortDebugString() << XEND;
  }
}

void SScenePetData::timer(DWORD curSec)
{
  // gift and friend
  if (curSec > dwTimeTick)
  {
    const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();

    dwTimeTick = curSec + MIN_T;
    addFriendExp(1);
    addRewardExp(1);

    if (curSec < dwTimeHapply)
      addFriendExp(rCFG.dwUserPetHapplyFriend);
    if (curSec < dwTimeHapplyGift)
      addRewardExp(rCFG.dwUserPetHapplyGift);
    if (curSec < dwTimeExcite)
      addFriendExp(rCFG.dwUserPetExciteFriend);
    if (curSec < dwTimeExciteGift)
      addRewardExp(rCFG.dwUserPetExciteGift);
    if (curSec < dwTimeHappiness)
      addFriendExp(rCFG.dwUserPetHappinessFriend);
    if (curSec < dwTimeHappinessGift)
      addRewardExp(rCFG.dwUserPetHappinessGift);
  }

  // action
  processAction(curSec);
}

void SScenePetData::patch_1()
{
  const SPetBaseLvCFG* pCFG = PetConfig::getMe().getPetBaseLvCFG(dwLv);
  const SPetBaseLvCFG* pNextCFG = PetConfig::getMe().getPetBaseLvCFG(dwLv + 1);
  if (pCFG == nullptr || pNextCFG == nullptr)
  {
    XERR << "[宠物-补丁]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "执行补丁失败,当前等级 :" << dwLv << "未在 Table_PetBaseLevel.txt 表中找到" << XEND;
    return;
  }

  if (pCFG->qwNewTotExp < qwExp)
  {
    XERR << "[宠物-补丁]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name
      << "执行补丁失败,当前等级 :" << dwLv << "新总经验" << pCFG->qwNewTotExp << "小于当前经验" << qwExp << XEND;
  }
  if (qwExp < pCFG->qwExp)
  {
    XERR << "[宠物-补丁]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name
      << "执行补丁失败,当前等级 :" << dwLv << "当前经验" << qwExp << "小于旧总经验" << pCFG->qwExp << XEND;
    return;
  }
  if (pNextCFG->qwExp < pCFG->qwExp)
  {
    XERR << "[宠物-补丁]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name
      << "执行补丁失败,当前等级 :" << dwLv << "下一级旧总经验" << pNextCFG->qwExp << "小于当前旧总经验" << pCFG->qwExp << XEND;
    return;
  }

  DWORD dwLost = pCFG->qwNewTotExp < qwExp ? 0 : pCFG->qwNewTotExp - qwExp;
  QWORD qwTemp = (dwLost + ((qwExp - pCFG->qwExp) / (1.0f * pNextCFG->qwExp - pCFG->qwExp + 1) * (pNextCFG->qwNewCurExp + 1)));
  XLOG << "[宠物-补丁]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name
    << "执行补丁成功,等级 :" << dwLv << "经验 :" << qwExp << "旧总经验" << pCFG->qwExp << "新总经验" << pCFG->qwNewTotExp << "新经验" << pCFG->qwNewCurExp
    << "下一级旧总经验" << pNextCFG->qwExp << "获得补偿经验" << qwTemp << XEND;

  qwExp += qwTemp;
  dwLv = 1;
  baseLevelup();
}

void SScenePetData::patch_2()
{
  map<DWORD, DWORD> mapBodyMonster;

  mapBodyMonster[10153] = 700010;
  mapBodyMonster[10157] = 700020;
  mapBodyMonster[10155] = 700030;
  mapBodyMonster[10161] = 700040;
  //mapBodyMonster[?????] = 700050;
  mapBodyMonster[10164] = 700060;
  mapBodyMonster[10158] = 700070;
  mapBodyMonster[10162] = 700080;
  mapBodyMonster[10159] = 700090;
  //mapBodyMonster[?????] = 700100;
  //mapBodyMonster[?????] = 700110;
  //mapBodyMonster[?????] = 700120;
  //mapBodyMonster[?????] = 700130;
  mapBodyMonster[20023] = 700140;
  mapBodyMonster[10165] = 700150;
  mapBodyMonster[10183] = 700160;

  for (auto &m : mapBodyMonster)
  {
    auto s = setUnlockBodyIDs.find(m.first);
    if (s != setUnlockBodyIDs.end())
    {
      XLOG << "[宠物-补丁2]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "原body :" << *s << "被移除,添加新monsterid :" << m.second << XEND;
      setUnlockBodyIDs.erase(s);
      setUnlockBodyIDs.insert(m.second);
    }
  }

  auto m = mapBodyMonster.find(dwBody);
  if (m != mapBodyMonster.end())
  {
    XLOG << "[宠物-补丁2]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "当前body :" << dwBody << "被移除,添加新monsterid :" << m->second << XEND;
    dwBody = m->second;
  }

  XLOG << "[宠物-补丁2]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "已执行" << XEND;
}

void SScenePetData::patch_3()
{
  if (pCFG == nullptr)
  {
    XERR << "[宠物-补丁3]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "执行失败,配置错误" << XEND;
    return;
  }
  if (setSkillIDs.empty() == true)
  {
    XERR << "[宠物-补丁3]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "新宠物,无需补丁" << XEND;
    return;
  }

  TSetDWORD setRandomSkill;
  pCFG->getRandomSkill(setRandomSkill);

  DWORD dwSkillID = 0;
  for (auto &s : setRandomSkill)
  {
    if (s / 1000 == pCFG->dwWorkSkillID)
    {
      dwSkillID = s;
      break;
    }
  }
  if (dwSkillID == 0)
  {
    XERR << "[宠物-补丁3]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "执行失败,未找到打工技能" << XEND;
    return;
  }

  auto skill = find_if(setSkillIDs.begin(), setSkillIDs.end(), [&](DWORD r) -> bool{
    return dwSkillID / 1000 == r / 1000;
  });
  bool b = skill == setSkillIDs.end();
  if (b)
    setSkillIDs.insert(dwSkillID);
  XLOG << "[宠物-补丁3]" << pOwner->accid << pOwner->id << pOwner->getProfession() << pOwner->name << "已执行" << (b ? "添加" : "已包含") << "打工技能" << XEND;
}

void SScenePetData::patch_4()
{
  if (!pEquip || !pOwner)
    return;

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD equipid = pEquip->getTypeID();
  auto it = rCFG.mapOldEquip2NewUseItem.find(equipid);
  if (it == rCFG.mapOldEquip2NewUseItem.end())
  {
    XERR << "[宠物-补丁4], 找不到装备对应的useitem" << "玩家:" << pOwner->name << pOwner->id << "装备" << equipid << XEND;
    return;
  }
  DWORD newitemid = it->second;

  // 删除装备
  SAFE_DELETE(pEquip);
  pEquip = nullptr;

  // 添加转换道具
  ItemInfo item;
  item.set_id(newitemid);
  item.set_count(1);
  pOwner->getPackage().addItem(item, EPACKMETHOD_AVAILABLE);
  XLOG << "[宠物-补丁4], 装备替换成功, 玩家:" << pOwner->name << pOwner->id << "宠物:" << dwID << "旧的装备:" << equipid << "新的装备:" << newitemid << XEND;
}

DWORD SScenePetData::getEquipIDByDataType(EUserDataType eType) const
{
  for (auto &m : mapCurPos2EquipID)
  {
    if (UserPet::getDataTypeByPos(m.first) == eType)
      return m.second;
  }
  for (auto &m : mapInitPos2EquipID)
  {
    if (UserPet::getDataTypeByPos(m.first) == eType)
      return m.second;
  }
  return 0;
}

DWORD SScenePetData::getBodyID() const
{
  DWORD monsterid = dwBody ? dwBody : dwID;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(monsterid);
  if (pCFG == nullptr)
    return 0;
  return pCFG->figure.body;
}

void SEquipUnlockInfo::fromData(const PetEquipUnlockInfo& oData)
{
  setUnlockItems.clear();
  vecSpecUnlockInfo.clear();
  for (int i = 0; i < oData.items_size(); ++i)
    setUnlockItems.insert(oData.items(i));

  vecSpecUnlockInfo.resize(oData.bodyitems_size());
  for (int i = 0; i < oData.bodyitems_size(); ++i)
    vecSpecUnlockInfo[i].CopyFrom(oData.bodyitems(i));
}

void SEquipUnlockInfo::toData(PetEquipUnlockInfo* pData)
{
  if (pData == nullptr)
    return;

  for (auto &s : setUnlockItems)
    pData->add_items(s);
  for (auto &v : vecSpecUnlockInfo)
    pData->add_bodyitems()->CopyFrom(v);
}

// user pet
UserPet::UserPet(SceneUser* user) : m_pUser(user)
{
  m_dwPetAchieveTick = xTime::getCurSec();
}

UserPet::~UserPet()
{
  for (auto &l : m_listPetData)
  {
    if (l.pEquip != nullptr)
      SAFE_DELETE(l.pEquip);
  }
}

bool UserPet::preCatchPet(SceneNpc* npc)
{
  if (npc == nullptr || npc->isAlive() == false)
    return false;

  // check can catch
  const SCatchPetCFG* pCFG = PetConfig::getMe().getCatchCFGByMonster(npc->getNpcID());
  if (pCFG == nullptr)
    return false;

  //npc->setClearState();
  npc->setDeadRemoveAtonce();

  DWORD lineid = MiscConfig::getMe().getPetCFG().dwCatchLineID;
  if (m_qwCatchPetID)
  {
    SceneNpc* oldnpc = SceneNpcManager::getMe().getNpcByTempID(m_qwCatchPetID);
    CatchPetNpc* pCatchNpc = dynamic_cast<CatchPetNpc*>(oldnpc);
    if (pCatchNpc)
    {
      m_pUser->getSpEffect().del(lineid, 0);
      pCatchNpc->onCatchOther();
    }
  }

  NpcDefine def;
  def.setID(pCFG->dwCatchNpcID);
  def.setPos(npc->getPos());
  def.m_oVar.m_qwOwnerID = m_pUser->id;

  SceneNpc* newnpc = SceneNpcManager::getMe().createNpc(def, m_pUser->getScene());
  if (newnpc == nullptr)
    return false;

  CatchPetNpc* pCatchNpc = dynamic_cast<CatchPetNpc*> (newnpc);
  if (pCatchNpc == nullptr)
    return false;

  if (pCatchNpc->initMasterNpc(npc->getNpcID()) == false)
  {
    pCatchNpc->removeAtonce();
    return false;
  }

  if (lineid)
  {
    TSetQWORD targetSet;
    targetSet.insert(pCatchNpc->id);
    m_pUser->getSpEffect().add(lineid, targetSet);
  }

  m_qwCatchPetID = pCatchNpc->id;
  XLOG << "[宠物-捕捉], 建立交互关系成功,玩家:" << m_pUser->name << m_pUser->id << "场景怪物:" << npc->name << npc->id << "交互npc:" << pCatchNpc->name << pCatchNpc->id << XEND;

  return true;
}

void UserPet::giveGiftCatchNpc(QWORD npcguid)
{
  if (npcguid != 0 && npcguid != m_qwCatchPetID)
    return;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwCatchPetID);
  if (npc == nullptr)
    return;
  CatchPetNpc* catchnpc = dynamic_cast<CatchPetNpc*>(npc);
  if (catchnpc == nullptr)
    return;
  if (catchnpc->checkFullValue())
  {
    MsgManager::sendMsg(m_pUser->id, 9001);
    XLOG << "[宠物-捕捉], 赠送, 捕捉值已满, 不需赠送, 玩家:" << m_pUser->name << m_pUser->id << "捕捉npc:" << npc->name << npc->id << XEND;
    return;
  }
  const SCatchPetCFG* pCFG = PetConfig::getMe().getCatchCFGByMonster(catchnpc->getMasterNpcID());
  if (pCFG == nullptr)
    return;
  BasePackage* pPackage = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pPackage == nullptr)
    return;

  DWORD itemid = pCFG->dwGiftItemID;
  if (pPackage->checkItemCount(itemid, 1) == false)
    return;
  pPackage->reduceItem(itemid, ESOURCE_PET);
  catchnpc->addCatchValue(pCFG->dwItemAddCatchValue);
  XLOG << "[宠物-捕捉], 玩家:" << m_pUser->name << m_pUser->id << "赠送捕捉npc道具, 扣除道具:" << itemid << XEND;
}

void UserPet::addBaseExp(QWORD qwExp)
{
  for (auto &l : m_listPetData)
    l.addBaseExp(qwExp);
}

void UserPet::addFriendExp(QWORD qwExp)
{
  for (auto &l : m_listPetData)
    l.addFriendExp(qwExp);
}

void UserPet::addRewardExp(QWORD qwExp)
{
  for (auto &l : m_listPetData)
    l.addRewardExp(qwExp);
}

DWORD UserPet::getMaxBaseLv() const
{
  DWORD dwLv = 0;
  for (auto &l : m_listPetData)
  {
    if (l.dwLv > dwLv)
      dwLv = l.dwLv;
  }
  return dwLv;
}

DWORD UserPet::getMaxFriendLv() const
{
  DWORD dwLv = 0;
  for (auto &l : m_listPetData)
  {
    if (l.dwFriendLv > dwLv)
      dwLv = l.dwFriendLv;
  }
  return dwLv;
}

DWORD UserPet::getMaxEquipUnlockCount() const
{
  DWORD dwCount = 0;
  for (auto &l : m_listPetData)
  {
    if (l.setUnlockEquipIDs.size() > dwCount)
      dwCount = l.setUnlockEquipIDs.size();
  }
  return dwCount;
}

bool UserPet::touch(QWORD qwPetID)
{
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return qwPetID == r.qwTempID;
      });
  if (it == m_listPetData.end())
  {
    XERR << "[宠物-抚摸]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "抚摸宠物 :" << qwPetID << "失败,没有该宠物" << XEND;
    return false;
  }

  return it->touch();
}

bool UserPet::feed(DWORD dwPetID)
{
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return dwPetID == r.dwID;
      });
  if (it == m_listPetData.end())
  {
    XERR << "[宠物-喂食]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "喂食宠物 :" << dwPetID << "失败,没有该宠物" << XEND;
    return false;
  }

  return it->feed();
}

bool UserPet::sendGift(DWORD dwPetID, const string& itemguid)
{
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return dwPetID == r.dwID;
      });
  if (it == m_listPetData.end())
  {
    XERR << "[宠物-赠送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "赠送礼品给宠物 :" << dwPetID << "失败,没有该宠物" << XEND;
    return false;
  }

  ItemBase* pItem = m_pUser->getPackage().getItem(itemguid);
  if (pItem == nullptr)
    return false;

  const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(it->dwID);
  if (pCFG == nullptr)
    return false;

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD friendvalue = rCFG.dwUserPetGiftFriend;
  if (PetConfig::getMe().isComposePet(it->dwID) == false)
  {
    if (pCFG->setHobbyItems.find(pItem->getTypeID()) == pCFG->setHobbyItems.end())
    {
      XERR << "[宠物-赠送], 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << it->dwID << "赠送道具:" << pItem->getTypeID() << "不属于该宠物喜爱道具" << XEND;
      return false;
    }
  }
  else // 融合宠物可以吃任意道具
  {
    friendvalue = PetConfig::getMe().getComPetValueByItem(pItem->getTypeID());
    if (friendvalue == 0)
    {
      XERR << "[宠物-赠送], 玩家:" << m_pUser->name << m_pUser->id << "融合宠物:" << it->dwID << "赠送道具:" << pItem->getTypeID() << "非法" << XEND;
      return false;
    }
  }

  DWORD dwGiftCount = it->oVar.getVarValue(EVARTYPE_USERPET_GIFT);
  if (dwGiftCount >= rCFG.dwUserPetGiftPerDay)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PET_GIFT_MAX);
    return false;
  }

  m_pUser->getPackage().reduceItem(itemguid, 1, ESOURCE_PET, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PETWORK);

  it->addFriendExp(friendvalue);
  it->addFeelingTime(EPETDATA_TIME_HAPPINESS, rCFG.dwUserPetGiftTime);
  it->oVar.setVarValue(EVARTYPE_USERPET_GIFT, dwGiftCount + 1);
  it->playAction(EPETACTION_OWNER_SENDGIFT);
  MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PET_FRIENDUP, MsgParams(it->strName));
  m_pUser->getAchieve().onPetGift();
  XLOG << "[宠物-赠送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "赠送礼品给宠物 :" << it->dwID << "成功,happiness心情 :" << it->dwTimeHappiness << "happinessGift :" << it->dwTimeHappinessGift << "当前赠送次数 :" << (dwGiftCount + 1) << XEND;
  return true;
}

bool UserPet::getGift(DWORD dwPetID)
{
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return dwPetID == r.dwID;
      });
  if (it == m_listPetData.end())
  {
    XERR << "[宠物-领取礼物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从宠物 :" << dwPetID << "领取礼物失败,没有该宠物" << XEND;
    return false;
  }

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  if (it->qwRewardExp < rCFG.dwUserPetGiftReqValue)
  {
    XERR << "[宠物-领取礼物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从宠物 :" << it->dwID << "领取礼物失败,礼物值" << it->qwRewardExp << "没有达到要求" << rCFG.dwUserPetGiftReqValue << XEND;
    return false;
  }

  const SPetFriendLvCFG* pCFG = PetConfig::getMe().getFriendLvCFG(dwPetID, it->dwFriendLv);
  if (pCFG == nullptr)
  {
    XERR << "[宠物-领取礼物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从宠物 :" << it->dwID << "领取礼物失败,友情等级" << it->dwFriendLv << "未在Table_PetFriendLevel.txt表中找到"<< XEND;
    return false;
  }

  TVecItemInfo vecItems;
  if (RewardManager::roll(pCFG->dwRewardID, m_pUser, vecItems, ESOURCE_REWARD) == false)
  {
    XERR << "[宠物-领取礼物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "从宠物 :" << it->dwID << "领取礼物失败,友情等级" << it->dwFriendLv << "未在Table_PetFriendLevel.txt表中找到"<< XEND;
    return false;
  }
  for (auto &v : vecItems)
    v.set_source(ESOURCE_PET);

  for (auto &v : vecItems)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.id());
    if (pCFG == nullptr)
    {
      XERR << "[宠物-领取礼物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "从宠物 :" << it->dwID << "领取礼物失败,友情等级" << it->dwFriendLv << "中随机 id :" << v.id() << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }

    DWORD dwGetLimit = pCFG->getItemGetLimit(m_pUser->getLevel(), ESOURCE_PET, m_pUser->getDeposit().hasMonthCard());
    if (dwGetLimit != DWORD_MAX)
    {
      DWORD dwGetCnt = m_pUser->getPackage().getVarGetCnt(v.id(), ESOURCE_PET);
      if (dwGetCnt >= dwGetLimit)
      {
        XERR << "[宠物-领取礼物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "从宠物 :" << it->dwID << "领取礼物失败,友情等级" << it->dwFriendLv << "中随机 id :" << v.id() << "今天已获得" << dwGetCnt << "超过今天上限" << dwGetLimit << XEND;
        return false;
      }
    }
  }

  if (m_pUser->getDeposit().hasMonthCard() == true)
  {
    for (auto &v : vecItems)
      v.set_count(v.count() * rCFG.dwUserPetGiftMonthCard);
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr || pMainPack->checkAddItem(vecItems, EPACKMETHOD_CHECK_WITHPILE) == false)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
    return false;
  }

  it->subRewardExp(rCFG.dwUserPetGiftReqValue);
  pMainPack->addItem(vecItems, EPACKMETHOD_CHECK_WITHPILE);
  it->playAction(EPETACTION_OWNER_GETGIFT);

  XLOG << "[宠物-领取礼物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "从宠物 :" << it->dwID << "领取礼物成功,礼物值变为" << it->qwRewardExp << "获得";
  for (auto &v : vecItems)
    XLOG << v.ShortDebugString();
  XLOG << XEND;
  return true;
}

bool UserPet::equip(const EquipOperPetCmd& cmd)
{
  DWORD dwPetID = cmd.petid();
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return dwPetID == r.dwID;
      });
  if (it == m_listPetData.end())
  {
    MsgManager::sendDebugMsg(m_pUser->id, "没有找到该宠物");
    XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "宠物 :" << dwPetID << "执行" << cmd.ShortDebugString() << "失败,没有该宠物" << XEND;
    return false;
  }

  const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(it->dwID);
  if (pCFG == nullptr)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "没有找到该宠物的配置");
    XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "宠物 :" << dwPetID << "执行" << cmd.ShortDebugString() << "失败,未在 Table_Pet.txt 表中找到" << XEND;
    return false;
  }

  PetNpc* pNpc = dynamic_cast<PetNpc*>(SceneNpcManager::getMe().getNpcByTempID(it->qwTempID));
  if (pNpc == nullptr)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "没有找到该宠物的实体npc");
    XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败,好感度等级 :" << it->dwFriendLv << "未在 Table_Pet_FriendLevel.txt 表中找到" << XEND;
    return false;
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "背包没了,炸炸炸");
    XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败,未找到背包" << XEND;
    return false;
  }

  if (cmd.oper() == EPETEQUIPOPER_ON)
  {
    //0705
    XERR << "[宠物-装备], 旧的宠物装备不再支持, 玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;

    ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(cmd.guid()));
    if (pEquip == nullptr)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "没有这个装备");
      XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败,未找到该道具" << XEND;
      return false;
    }
    if (pCFG->canEquip(pEquip->getTypeID()) == false)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "这个装备不能装");
      XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败,该宠物不能装备" << XEND;
      return false;
    }
    if (PetConfig::getMe().isUnlockEquip(it->dwID, pEquip->getTypeID()) == true && it->isEquipUnlocked(pEquip->getTypeID()) == false)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "这个装备不能装");
      XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败,该装备未解锁" << XEND;
      return false;
    }

    ItemEquip* pOldEquip = it->pEquip;
    if (pOldEquip != nullptr)
    {
      if (pMainPack->checkAddItemObj(pOldEquip) == false)
      {
        MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
        return false;
      }
      pMainPack->addItemObj(pOldEquip);
      it->pEquip = nullptr;
    }

    pMainPack->reduceItem(cmd.guid(), ESOURCE_PET, 1, false);
    it->pEquip = pEquip;
    it->playAction(EPETACTION_OWNER_EQUIP);

    // update show data
    if (it->pEquip->getEquipType() == EEQUIPTYPE_WEAPON)
    {
      pNpc->setDataMark(EUSERDATATYPE_LEFTHAND);
      pNpc->setDataMark(EUSERDATATYPE_RIGHTHAND);
    }
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_BACK)
      pNpc->setDataMark(EUSERDATATYPE_BACK);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_HEAD)
      pNpc->setDataMark(EUSERDATATYPE_HEAD);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_FACE)
      pNpc->setDataMark(EUSERDATATYPE_FACE);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_TAIL)
      pNpc->setDataMark(EUSERDATATYPE_TAIL);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_MOUNT)
      pNpc->setDataMark(EUSERDATATYPE_MOUNT);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_MOUTH)
      pNpc->setDataMark(EUSERDATATYPE_MOUTH);
    pNpc->refreshDataAtonce();
  }
  else if (cmd.oper() == EPETEQUIPOPER_OFF)
  {
    //0705
    XERR << "[宠物-装备], 旧的宠物装备不再支持, 玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;

    ItemEquip* pEquip = it->pEquip;
    if (pEquip == nullptr)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "没有这个装备");
      XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败,该位置无装备" << XEND;
      return false;
    }

    if (pMainPack->checkAddItemObj(pEquip) == false)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
      return false;
    }

    // update show data
    if (it->pEquip->getEquipType() == EEQUIPTYPE_WEAPON)
    {
      pNpc->setDataMark(EUSERDATATYPE_LEFTHAND);
      pNpc->setDataMark(EUSERDATATYPE_RIGHTHAND);
    }
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_BACK)
      pNpc->setDataMark(EUSERDATATYPE_BACK);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_HEAD)
      pNpc->setDataMark(EUSERDATATYPE_HEAD);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_FACE)
      pNpc->setDataMark(EUSERDATATYPE_FACE);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_TAIL)
      pNpc->setDataMark(EUSERDATATYPE_TAIL);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_MOUNT)
      pNpc->setDataMark(EUSERDATATYPE_MOUNT);
    else if (it->pEquip->getEquipType() == EEQUIPTYPE_MOUTH)
      pNpc->setDataMark(EUSERDATATYPE_MOUTH);
    pNpc->refreshDataAtonce();

    it->pEquip = nullptr;
    pMainPack->addItemObj(pEquip);
  }
  else if (cmd.oper() == EPETEQUIPOPER_BODY)
  {
    DWORD dwBodyID = atoi(cmd.guid().c_str());
    if (dwBodyID == it->dwBody)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "皮肤没有变化");
      XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败, oldbody :" << it->dwBody << "newbody :" << dwBodyID << "没有变化" << XEND;
      return false;
    }
    if (dwBodyID != 0)
    {
      if (it->isBodyUnlocked(dwBodyID) == false)
      {
        MsgManager::sendDebugMsg(m_pUser->id, "皮肤未解锁");
        XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败, body :" << dwBodyID << "未解锁" << XEND;
        return false;
      }
      SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(it->qwTempID);
      if (pNpc == nullptr)
      {
        MsgManager::sendDebugMsg(m_pUser->id, "测试log:未找到场景NPC");
        XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败, 未找到对应npc" << XEND;
        return false;
      }
    }

    it->setBody(dwBodyID);
    pNpc->setDataMark(EUSERDATATYPE_BODY);
    pNpc->setDataMark(EUSERDATATYPE_LEFTHAND);
    pNpc->setDataMark(EUSERDATATYPE_RIGHTHAND);
    pNpc->setDataMark(EUSERDATATYPE_BACK);
    pNpc->setDataMark(EUSERDATATYPE_HEAD);
    pNpc->setDataMark(EUSERDATATYPE_FACE);
    pNpc->setDataMark(EUSERDATATYPE_TAIL);
    pNpc->setDataMark(EUSERDATATYPE_MOUNT);
    pNpc->setDataMark(EUSERDATATYPE_MOUTH);
    pNpc->refreshDataAtonce();
  }
  else
  {
    XERR << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "失败,操作不合法" << XEND;
    return false;
  }

  if (cmd.oper() == EPETEQUIPOPER_ON || cmd.oper() == EPETEQUIPOPER_OFF)
  {
    EquipUpdatePetCmd retcmd;
    retcmd.set_petid(cmd.petid());
    if (it->pEquip != nullptr)
      it->pEquip->toItemData(retcmd.mutable_update());
    else
      retcmd.set_del(cmd.guid());
    PROTOBUF(retcmd, retsend, retlen);
    m_pUser->sendCmdToMe(retsend, retlen);
  }

  XLOG << "[宠物-装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "宠物 :" << it->dwID << "执行" << cmd.ShortDebugString() << "成功" << XEND;
  return true;
}

bool UserPet::resetSkill(const ResetSkillPetCmd& cmd)
{
  if (m_listPetData.empty() == true)
  {
    XERR << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用id :" << cmd.id() << "重置失败,未找到孵化后宠物" << XEND;
    return false;
  }

  for (auto &l : m_listPetData)
  {
    if (l.pCFG == nullptr)
    {
      XERR << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "使用id :" << cmd.id() << "重置失败" << l.dwID << "未在 Table_Pet.txt 表中找到" << XEND;
      return false;
    }
  }

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  if (rCFG.canRandSkill(cmd.id()) == false)
  {
    XERR << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用id :" << cmd.id() << "重置失败,不是重置道具" << XEND;
    return false;
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用id :" << cmd.id() << "重置失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }
  if (pMainPack->checkItemCount(cmd.id()) == false)
  {
    XERR << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用id :" << cmd.id() << "重置失败,材料不足" <<  XEND;
    return false;
  }

  bool bSuccess = false;
  for (auto &l : m_listPetData)
  {
    if (l.pCFG == nullptr)
      continue;
    if (l.pCFG->isMaxSkill(l.setSkillIDs) == true)
      continue;

    TSetDWORD setFullSkillIDs;
    l.pCFG->getFullSkill(setFullSkillIDs);

    TSetDWORD setNewSkillIDs;
    for (auto &s : setFullSkillIDs)
    {
      DWORD dwLv = rCFG.randSkill(cmd.id());
      if (dwLv == 0)
      {
        XERR << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "petid : " << l.dwID << "使用id :" << cmd.id() << "重置失败,level为0" <<  XEND;
        return false;
      }
      DWORD dwNewID = s / ONE_THOUSAND * ONE_THOUSAND + dwLv;
      if (dwNewID > s)
      {
        XERR << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "petid : " << l.dwID << "使用id :" << cmd.id() << "重置失败,newskill :" << dwNewID << "超过配置最大等级" << s <<  XEND;
        return false;
      }
      setNewSkillIDs.insert(dwNewID);
    }

    TSetDWORD setOldSkillIDs = l.setSkillIDs;
    l.setSkillIDs = setNewSkillIDs;

    XLOG << "[宠物-技能]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用id :" << cmd.id() << "重置petid :" << l.dwID << "成功,oldskill :";
    for (auto &s : setOldSkillIDs)
      XLOG << s;
    XLOG << "newskill :";
    for (auto &s : l.setSkillIDs)
      XLOG << s;
    XLOG << XEND;

    PetNpc* pNpc = dynamic_cast<PetNpc*>(SceneNpcManager::getMe().getNpcByTempID(l.qwTempID));
    if (pNpc != nullptr)
      pNpc->resetActiveSkills(l.setSkillIDs);

    l.obitset.set(EPETDATA_SKILL);
    l.updateData();

    // achieve
    if ( !l.setSkillIDs.empty())
    {
      DWORD minlv = 100;
      for (auto &s : l.setSkillIDs)
      {
        if (s % 100 < minlv)
          minlv = s % 100;
      }
      m_pUser->getAchieve().onPetSkillAllLv(minlv);
    }

    bSuccess = true;

    //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Pet_Change;
    EPetChangeType eChangeType = EPetChangeType_Skill;

    PlatLogManager::getMe().eventLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eid,
      m_pUser->getUserSceneData().getCharge(), eType, 0, 1);


    stringstream ssBefore;
    stringstream ssAfter;
    for (auto &s : setOldSkillIDs)
      ssBefore << s << ";";
    XLOG << "newskill :";
    for (auto &s : l.setSkillIDs)
      ssAfter << s << ";";

    string strBefore = ssBefore.str();
    string strAfter = ssAfter.str();

    PlatLogManager::getMe().PetChangeLog(thisServer,
      m_pUser->accid,
      m_pUser->id,
      eType,
      eid,

      eChangeType,
      l.dwID,
      l.strName,
      0,
      0,
      strBefore,
      strAfter
    );

  }

  if (!bSuccess)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PET_MAX_SKILL);
    return false;
  }

  pMainPack->reduceItem(cmd.id(), ESOURCE_PET);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  return true;
}

bool UserPet::unlockExtraBody(DWORD dwPetID, DWORD dwBodyID)
{
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwBodyID);
  if (pCFG == nullptr)
  {
    XERR << "[宠物-body]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "解锁额外 :" << dwBodyID << "失败,未在 Table_Monster.txt 表找到" << XEND;
    return false;
  }

  SScenePetData* pData = getPetData(dwPetID);
  if (pData == nullptr || pData->pCFG == nullptr)
  {
    const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(dwPetID);
    if (pCFG != nullptr)
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PET_BODY_ERROR, MsgParams(pCFG->strName));
    XERR << "[宠物-body]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "解锁额外 :" << dwBodyID << "失败,petid :" << dwPetID << "未找到" << XEND;
    return false;
  }

  if (pData->setUnlockBodyIDs.find(dwBodyID) != pData->setUnlockBodyIDs.end())
  {
    MsgManager::sendMsg(m_pUser->id, 3642);
    XERR << "[宠物-body]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwPetID << "解锁额外 :" << dwBodyID << "失败,已存在" << XEND;
    return false;
  }

  pData->setUnlockBodyIDs.insert(dwBodyID);
  MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PET_NEW_BODY, MsgParams(pData->pCFG->strName));

  UnlockNtfPetCmd cmd;
  cmd.set_petid(dwPetID);
  cmd.add_bodys(dwBodyID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[宠物-body]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwPetID << "解锁额外 :" << dwBodyID << "成功" << XEND;
  return true;
}

void UserPet::playAction(EPetAction eAction)
{
  for (auto &l : m_listPetData)
    l.playAction(eAction);
}

DWORD UserPet::getPetCount(DWORD dwBaseLv, DWORD dwFriendLv) const
{
  DWORD dwCount = 0;
  for (auto &l : m_listPetData)
  {
    if (l.dwLv >= dwBaseLv && l.dwFriendLv >= dwFriendLv)
      ++dwCount;
  }

  auto eggf = [&](const ItemBase* pBase)
  {
    const ItemEgg* pEgg = dynamic_cast<const ItemEgg*>(pBase);
    if (pEgg == nullptr || pEgg->getLevel() < dwBaseLv || pEgg->getFriendLv() < dwFriendLv || pEgg->getName().empty() == true)
      return;
    ++dwCount;
  };
  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
    return dwCount;
  pMainPack->foreach(eggf);
  return dwCount;
}

bool UserPet::changeName(DWORD dwPetID, const string& name)
{
  SScenePetData* pData = getPetData(dwPetID);
  if (pData == nullptr)
  {
    XERR << "[宠物-改名]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "把宠物" << dwPetID << "更名为" << name << "失败,没有发现该宠物" << XEND;
    return false;
  }
  PetNpc* pNpc = dynamic_cast<PetNpc*>(SceneNpcManager::getMe().getNpcByTempID(pData->qwTempID));
  if (pNpc == nullptr)
  {
    XERR << "[宠物-改名]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "把宠物" << dwPetID << "更名为" << name << "失败,没有出场" << XEND;
    return false;
  }
  if (MiscConfig::getMe().getSystemCFG().checkNameValid(name, ENAMETYPE_PET) != ESYSTEMMSG_ID_MIN)
  {
    MsgManager::sendMsg(m_pUser->id, 1005);
    XERR << "[宠物-改名]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "把宠物" << dwPetID << "更名为" << name << "失败,名字非法" << XEND;
    return false;
  }
  pData->setName(name);
  pNpc->set_name(name.c_str());

  pNpc->setDataMark(EUSERDATATYPE_NAME);
  pNpc->refreshDataAtonce();

  XLOG << "[宠物-改名]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "把宠物" << dwPetID << "更名为" << name << "成功" << XEND;
  return true;
}

void UserPet::switchSkill(DWORD dwPetID, bool open)
{
  bool off = !open;
  SScenePetData* pData = getPetData(dwPetID);
  if (pData == nullptr)
  {
    XERR << "[宠物-技能开关], 宠物ID非法:" << dwPetID << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return;
  }
  if (pData->bSkillOff == off)
    return;
  pData->bSkillOff = off;
  pData->obitset.set(EPETDATA_SKILLSWITCH);
  pData->updateData();
  if (off)
    MsgManager::sendMsg(m_pUser->id, 9018);
  else
    MsgManager::sendMsg(m_pUser->id, 9019);
}

bool UserPet::load(const BlobUserPet& oData)
{
  if (oData.data().empty() == false)
  {
    for (int i = 0; i < oData.data_size(); ++i)
    {
      SScenePetData tmpdata;
      tmpdata.pOwner = m_pUser;
      if (tmpdata.fromPetData(oData.data(i)) == false)
        continue;

      m_listPetData.push_back(tmpdata);
    }
  }
  m_oEquipUnlockInfo.fromData(oData.unlockinfo());
  return true;
}

bool UserPet::save(BlobUserPet* pData)
{
  if (pData == nullptr)
    return false;
  for (auto &s : m_listPetData)
  {
    UserPetData* data = pData->add_data();
    if (s.toPetData(data) == false)
      return false;
  }
  m_oEquipUnlockInfo.toData(pData->mutable_unlockinfo());
  return true;
}

bool UserPet::reload()
{
  for (auto &l : m_listPetData)
    l.reload();
  return true;
}

bool UserPet::reloadItem()
{
  for (auto &l : m_listPetData)
  {
    if (l.pEquip)
    {
      l.pEquip->setCFG(ItemManager::getMe().getItemCFG(l.pEquip->getTypeID()));
      XDBG <<"[宠物-重加载配置] item表重加载配置，charid" <<m_pUser->id <<"itemid"<<l.pEquip->getTypeID() << XEND;
    }
  }
  return true;
}

void UserPet::onUserEnterScene()
{
  sendUnlockInfo();

  if (m_listPetData.empty())
    return;
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return;
  for (auto &s : m_listPetData)
  {
    if (s.bLive)
      enterScene(s);
  }

  sendPetInfo();

  for (auto &s : m_listPetData)
  {
    if (s.bLive)
      s.playAction(EPETACTION_OWNER_HATCH);
  }
}

void UserPet::onUserLeaveScene()
{
  for (auto &l : m_listPetData)
  {
    leaveScene(l);
  }

  if (m_qwCatchPetID)
  {
    m_pUser->getSpEffect().del(MiscConfig::getMe().getPetCFG().dwCatchLineID, 0);
    CatchPetNpc* pNpc = dynamic_cast<CatchPetNpc*>(SceneNpcManager::getMe().getNpcByTempID(m_qwCatchPetID));
    if (pNpc)
    {
      pNpc->onUserLeaveScene();
    }
  }
}

bool UserPet::enterScene(SScenePetData& stData, bool bFirst /*= false*/, bool bFromEgg /*=false*/)
{
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return false;

  // 部分地图 不可携带
  if (pScene->getBaseCFG() && pScene->getBaseCFG()->noPet())
    return true;

  NpcDefine def;
  def.setID(stData.dwID);
  def.setPos(m_pUser->getPos());
  def.setRange(MiscConfig::getMe().getPetCFG().dwBirthRange);
  def.m_oVar.m_qwOwnerID = m_pUser->id;
  def.setLife(1);
  def.setBehaviours(def.getBehaviours() & ~BEHAVIOUR_OUT_RANGE_BACK);
  def.setTerritory(0);
  def.setName(stData.strName.c_str());
  def.setLevel(stData.dwLv);
  if (bFromEgg)
    def.setFadeIn(MiscConfig::getMe().getPetCFG().dwHatchFadeInTime);

  if (stData.dwHp)
    def.m_oVar.m_dwDefaultHp = stData.dwHp;
  SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, pScene);
  if (npc == nullptr)
    return false;
  PetNpc* pPet= dynamic_cast<PetNpc*> (npc);
  if (pPet == nullptr)
  {
    npc->setClearState();
    return false;
  }
  pPet->addActiveSkills(stData.setSkillIDs);
  pPet->m_oBuff.load(stData.oBuff);

  // 玩家切线, 宠物添加打boss、mini等切线保护buff
  if (m_pUser->isJustInViceZone())
  {
    DWORD limitbuff = MiscConfig::getMe().getSystemCFG().dwZoneBossLimitBuff;
    QWORD endtime = m_pUser->m_oBuff.getBuffDelTime(limitbuff);
    if (endtime)
      pPet->m_oBuff.add(limitbuff, m_pUser, 0, 0, endtime);
  }

  for (auto &s : stData.setSkillIDs)
  {
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(s);
    if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
    {
      pPet->m_oBuff.addSkillBuff(s);
      const TSetDWORD& masterbuff = pSkill->getPetMasterBuffs();
      for (auto &b : masterbuff)
      {
        m_pUser->m_oBuff.add(b, pPet, s);
      }
    }
  }

  stData.qwTempID = pPet->id;
  stData.bLive = true;
  if (stData.dwHp == 0)
    stData.dwHp = pPet->getAttr(EATTRTYPE_HP);

  if (stData.bHandStatus)
    sendHandMark(pPet->id);

  // update show data
  pPet->setDataMark(EUSERDATATYPE_BODY);
  pPet->setDataMark(EUSERDATATYPE_LEFTHAND);
  pPet->setDataMark(EUSERDATATYPE_RIGHTHAND);
  pPet->setDataMark(EUSERDATATYPE_BACK);
  pPet->setDataMark(EUSERDATATYPE_HEAD);
  pPet->setDataMark(EUSERDATATYPE_FACE);
  pPet->setDataMark(EUSERDATATYPE_TAIL);
  pPet->setDataMark(EUSERDATATYPE_MOUNT);
  pPet->setDataMark(EUSERDATATYPE_MOUTH);
  pPet->refreshDataAtonce();

  /*
  if (!bFirst)
    stData.playAction(EPETACTION_PET_BORN);
  else
    stData.playAction(EPETACTION_OWNER_HATCH);
    */
  return true;
}

bool UserPet::leaveScene(SScenePetData& stData, bool bToEgg /*= false*/)
{
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return false;

  for (auto &s : stData.setSkillIDs)
  {
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(s);
    if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
    {
      const TSetDWORD& masterbuff = pSkill->getPetMasterBuffs();
      for (auto &b : masterbuff)
      {
        m_pUser->m_oBuff.del(b);
      }
    }
  }

  stData.oBuff.Clear();
  npc->m_oBuff.save(&(stData.oBuff));

  // 通知前端打断牵手
  if (stData.bHandStatus && npc->isAlive())
  {
    sendHandBreak(npc->id);
  }

  changeState(stData, EPETSTATAE_WAIT_BATTLE);
  if (npc->isAlive())
  {
    stData.dwHp = npc->getAttr(EATTRTYPE_HP);
    if (bToEgg)
      npc->setTempFadeOutTime(MiscConfig::getMe().getPetCFG().dwHatchFadeOutTime);
    npc->removeAtonce();
  }
  return true;
}

bool UserPet::toEgg(DWORD petid)
{
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return petid == r.dwID;
      });
  if (it == m_listPetData.end())
    return false;

  DWORD dwItemID = PetConfig::getMe().getItemIDByPet(it->dwID);
  if (dwItemID == 0)
    return false;

  ItemData oData;
  oData.mutable_base()->set_guid(it->strGUID);
  oData.mutable_base()->set_id(dwItemID);
  oData.mutable_base()->set_count(1);

  if (it->bHandStatus)
    breakHand();

  XLOG << "[宠物-还原蛋]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "还原宠物蛋" << it->dwID << "成功,添加宠物蛋" << oData.ShortDebugString() << "到" << EPACKTYPE_MAIN << XEND;

  it->playAction(EPETACTION_OWNER_RESTORE);
  leaveScene(*it, true);

  if (it->toEggData(oData.mutable_egg()) == false)
    return false;
  m_listPetData.erase(it);

  sendPetDel(petid);
  m_pUser->getPackage().addItem(oData, EPACKMETHOD_AVAILABLE);
  return true;
}

bool UserPet::addPet(const EggData& eggdata, bool bFirst)
{
  if (m_listPetData.empty() == false)
  {
    XERR << "[宠物-添加], 已有宠物出场, 不可再次添加, 玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }
  const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(eggdata.id());
  if (pCFG == nullptr)
  {
    XERR << "[宠物-添加],添加失败, 不合法的宠物id:" << eggdata.id() << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }

  SScenePetData petdata;
  petdata.pOwner = m_pUser;
  petdata.fromEggData(eggdata);
  petdata.pCFG = pCFG;

  // 首次孵化
  if (bFirst)
  {
    // 获取技能
    pCFG->getRandomSkill(petdata.setSkillIDs);
    const TSetDWORD& commonskill = pCFG->setCommonSkills;
    petdata.setSkillIDs.insert(commonskill.begin(), commonskill.end());

    if (!petdata.setSkillIDs.empty())
    {
      DWORD minlv = 100;
      for (auto &s : petdata.setSkillIDs)
      {
        if (s % 100 < minlv)
          minlv = s % 100;
      }
      m_pUser->getAchieve().onPetSkillAllLv(minlv);
    }
    // 设置初始等级
    petdata.dwLv = 1;
    m_pUser->getManual().onPetHatch(petdata.dwID, true);

    // 设置默认变装
    const SPetAvatarCFG* pAvaCFG = PetConfig::getMe().getAvatarCFGByBody(petdata.getBodyID()); // 初始body为0, 用petdata.dwID对应的body
    if (pAvaCFG)
    {
      for (auto &m : pAvaCFG->mapPos2RandomList)
      {
        auto p = randomStlContainer(m.second);
        if (p)
          petdata.mapInitPos2EquipID[m.first] = *p;
      }
    }
  }
  /*else
  {
    checkChangeLevel(petdata);
  }*/

  for (auto m = petdata.mapCurPos2EquipID.begin(); m != petdata.mapCurPos2EquipID.end(); )
  {
    if (checkWearUnlock(m->second, petdata.getBodyID(), m->first) == false)
    {
      XLOG << "[宠物-孵化], 当前装备未解锁, 移除, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << petdata.dwID << "部位:" << m->first << "装备:" << m->second << XEND;
      m = petdata.mapCurPos2EquipID.erase(m);
      continue;
    }
    ++m;
  }

  petdata.baseLevelup();
  petdata.friendLevelup();
  petdata.obitset.reset();

  if (petdata.bLive)
    enterScene(petdata, bFirst, true);

  m_listPetData.push_back(petdata);
  sendPetInfo();
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_PET);

  if (!bFirst)
    petdata.playAction(EPETACTION_PET_BORN);
  else
    petdata.playAction(EPETACTION_OWNER_HATCH);

  XLOG << "[宠物-添加], 玩家:" << m_pUser->name << m_pUser->id << "添加宠物:" << petdata.dwID << XEND;
  return true;
}

void UserPet::onPetDie(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  for (auto &l : m_listPetData)
  {
    if (l.qwTempID != npc->id)
      continue;

    if (l.bHandStatus)
      breakHand();
    leaveScene(l);

    l.playAction(EPETACTION_PET_DEAD);
    l.qwTempID = 0;
    l.bLive = false;
    l.dwReliveTime = now() + MiscConfig::getMe().getPetCFG().dwPetReliveTime;
    l.dwHp = 0;
    l.obitset.set(EPETDATA_RELIVETIME);
    l.updateData();
    m_pUser->getAchieve().onPetDead();
  }
}

void UserPet::onUserAttack(xSceneEntryDynamic* enemy)
{
  if (m_listPetData.empty())
    return;

  if (!enemy)
    return;
  // 玩家切线后 猫不会攻击boss or mini
  if (m_pUser->isJustInViceZone())
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*> (enemy);
    if (pNpc && pNpc->getNpcZoneType() == ENPCZONE_FIELD)
    {
      if (pNpc->getNpcType() == ENPCTYPE_MVP || pNpc->getNpcType() == ENPCTYPE_MINIBOSS)
        return;
    }
  }

  DWORD cur = now();
  for (auto &l : m_listPetData)
  {
    if (l.bLive == false)
      continue;
    if (l.bSkillOff)
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
    if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;

    switch(l.eState)
    {
      case EPETSTATAE_NORAML:
        break;
      case EPETSTATAE_WAIT_BATTLE:
      case EPETSTATAE_HAND:
        changeState(l, EPETSTATAE_NORAML);
        break;
      case EPETSTATAE_EATFOOD:
      case EPETSTATAE_BETOUCH:
        continue;
        break;
      default:
        break;
    }
    npc->m_ai.setCurLockID(enemy->id);
    l.dwTempEndBattleTime = cur + 3;
    l.dwLastBattleTime = cur;
  }
}

void UserPet::onUserMove(const xPos& dest)
{
  if (m_listPetData.empty())
    return;
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return;
  for (auto &l : m_listPetData)
  {
    if (l.bLive == false)
      continue;
    switch(l.eState)
    {
      case EPETSTATAE_NORAML:
      case EPETSTATAE_WAIT_BATTLE:
      case EPETSTATAE_BETOUCH:
      {
        SceneNpc* pet = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
        if (pet == nullptr || pet->define.m_oVar.m_qwOwnerID != m_pUser->id)
          continue;

        if (getDistance(pet->getPos(), dest) < 1.5)
          continue;

        std::list<xPos> list;
        if (m_pUser->getScene()->findingPath(dest, m_pUser->getPos(), list, TOOLMODE_PATHFIND_FOLLOW) == false)
          return;
        if (list.size() < 2)
          return;

        list.pop_front();
        xPos dest1 = list.front();
        xPos mydest = dest1;
        float foldis = 1;
        float dist = getDistance(dest, dest1);
        mydest.x = foldis / dist * (dest1.x - dest.x) + dest.x;
        mydest.z = foldis / dist * (dest1.z - dest.z) + dest.z;

        auto getPosOnCircle = [&](const xPos& pos0, const xPos& pos1, float angle, xPos& out)
        {
          angle = angle * 3.14 / 180.0f;
          out = pos0;
          out.z = pos0.z + (pos1.z - pos0.z) * cos(angle) - (pos1.x - pos0.x) * sin(angle);
          out.x = pos0.x + (pos1.z - pos0.z) * sin(angle) + (pos1.x - pos0.x) * cos(angle);
        };
        xPos outpos;
        getPosOnCircle(dest, mydest, -70, outpos);
        if (pScene->getValidPos(outpos) == false)
          continue;
        pet->m_ai.moveTo(outpos);
      }
      break;
      case EPETSTATAE_HAND:
        continue;
      case EPETSTATAE_EATFOOD:
        {
          SceneNpc* pet = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
          if (pet == nullptr || pet->define.m_oVar.m_qwOwnerID != m_pUser->id)
            continue;
          if (getXZDistance(pet->getPos(), m_pUser->getPos()) < 15)
            continue;
          changeState(l, EPETSTATAE_WAIT_BATTLE);
          pet->m_ai.moveTo(dest);
        }
        break;
      //case EPETSTATAE_BETOUCH:
        //break;
      default:
        break;
    }
  }
}

void UserPet::onUserMoveTo(const xPos& dest)
{
  for (auto &l : m_listPetData)
  {
    if (l.eState == EPETSTATAE_HAND && l.bLive)
    {
      SceneNpc* pPet = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
      if (pPet)
        pPet->setScenePos(dest);
    }
  }
}

void UserPet::onUserGoTo(const xPos& dest)
{
  for (auto &l : m_listPetData)
  {
    if (l.bLive == false)
      continue;
    changeState(l, EPETSTATAE_NORAML);
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
    if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
    {
      xPos pos = dest;
      if (npc->getScene())
        npc->getScene()->getRandPos(m_pUser->getPos(), 5, pos);
      npc->m_oMove.clear();
      npc->goTo(pos);
    }
  }
}

void UserPet::onUserDie()
{
  for (auto &l : m_listPetData)
  {
    if (l.bLive == false)
      continue;
    changeState(l, EPETSTATAE_WAIT_BATTLE);
  }
}

void UserPet::sendPetInfo()
{
  if (m_listPetData.empty())
    return;
  PetInfoPetCmd cmd;
  for (auto &l : m_listPetData)
  {
    PetInfo* pInfo = cmd.add_petinfo();
    if (pInfo == nullptr)
      continue;
    if (l.toClientData(pInfo) == false)
      continue;
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserPet::sendPetDel(DWORD petid)
{
  PetOffPetCmd cmd;
  cmd.set_petid(petid);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserPet::timer(DWORD cur)
{
  for (auto &l : m_listPetData)
  {
    if (l.bLive == false)
    {
      if (cur >= l.dwReliveTime)
      {
        enterScene(l);
        sendPetInfo();
        l.playAction(EPETACTION_OWNER_HATCH);
        XLOG << "[宠物-复活], 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << l.dwID << XEND;
      }
    }
    else
    {
      // stop battle
      if (l.dwTempEndBattleTime && cur >= l.dwTempEndBattleTime)
      {
        l.dwTempEndBattleTime = 0;
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
        if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
        {
          if (npc->m_ai.getCurLockID() != 0)
            npc->m_ai.setCurLockID(0);
        }
      }

      // 更新状态
      updatePetState(l);

      l.timer(cur);

      int a = 0;
      if (a == 1)
        changeName(l.dwID, "ssss");
    }
  }

  if (m_listPetData.empty() == false && cur - m_dwPetAchieveTick > MIN_T)
  {
    m_pUser->getAchieve().onPetTime(cur - m_dwPetAchieveTick);
    m_dwPetAchieveTick = cur;
  }
}

void UserPet::changeState(SScenePetData& stData, EPetState eState)
{
  if (stData.eState == eState)
    return;

  switch(stData.eState)
  {
    case EPETSTATAE_NORAML:
    case EPETSTATAE_WAIT_BATTLE:
      break;
    case EPETSTATAE_HAND:
      sendHandStatus(false, stData.qwTempID);
      break;
    case EPETSTATAE_EATFOOD:
      {
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempID);
        if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
        {
          npc->m_oSkillProcessor.breakSkill(npc->id);
        }

        SceneNpc* p = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempFoodNpcID);
        FoodNpc* pFoodNpc = dynamic_cast<FoodNpc*> (p);
        if (pFoodNpc)
        {
          pFoodNpc->delPetEater(npc);
        }

        stData.dwEatBeginTime = 0;
        stData.qwTempFoodNpcID = 0;

        if (npc)
        {
          npc->stopEffect();
        }
      }
      break;
    case EPETSTATAE_BETOUCH:
      {
        if (stData.bLive == false)
          break;
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempID);
        if (!npc || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
          break;

        // 播放动作
        UserActionNtf cmd;
        cmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
        cmd.set_value(0);
        cmd.set_charid(stData.qwTempID);
        PROTOBUF(cmd, send, len);
        npc->sendCmdToNine(send, len);

        npc->stopEffect();
      }
      break;
    default:
      break;
  }

  switch(eState)
  {
    case EPETSTATAE_NORAML:
    case EPETSTATAE_WAIT_BATTLE:
      break;
    case EPETSTATAE_HAND:
      sendHandStatus(true, stData.qwTempID);
      break;
    case EPETSTATAE_EATFOOD:
      {
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempID);
        if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
        {
          npc->m_ai.setCurLockID(0);
        }
        stData.dwEatBeginTime = 0;
        if (npc)
        {
          npc->effect(MiscConfig::getMe().getPetCFG().dwUserPetFeedEffectID);
        }
      }
      break;
    case EPETSTATAE_BETOUCH:
      {
        if (stData.bLive == false)
          break;
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempID);
        if (!npc || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
          break;

        // 面向玩家
        NpcChangeAngle cmd1;
        cmd1.set_guid(npc->id);
        cmd1.set_targetid(m_pUser->id);
        PROTOBUF(cmd1, send1, len1);
        npc->sendCmdToNine(send1, len1);

        UserActionNtf cmd;
        cmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
        cmd.set_value(84);
        cmd.set_charid(stData.qwTempID);
        PROTOBUF(cmd, send, len);
        npc->sendCmdToNine(send, len);

        npc->effect(MiscConfig::getMe().getPetCFG().dwUserPetTouchEffectID);
      }
      break;
    default:
      break;
  }

  stData.eState = eState;
}

void UserPet::sendHandStatus(bool bBuild, QWORD qwPetID)
{
  HandStatusUserCmd cmd;
  cmd.set_build(bBuild);
  cmd.set_masterid(m_pUser->id);
  cmd.set_followid(qwPetID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);
}

void UserPet::sendHandBreak(QWORD qwPetID)
{
  BeFollowUserCmd cmd;
  cmd.set_etype(EFOLLOWTYPE_BREAK);
  cmd.set_userid(qwPetID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserPet::sendHandMark(QWORD qwPetID)
{
  BeFollowUserCmd cmd;
  cmd.set_etype(EFOLLOWTYPE_HAND);
  cmd.set_userid(qwPetID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool UserPet::handPet() const
{
  for (auto &l : m_listPetData)
  {
    if (l.bHandStatus)
      return true;
  }
  return false;
}

void UserPet::inviteHand(QWORD qwPetID)
{
  /*
     宠物牵手不限制技能
  if (m_pUser->isHandEnable() == false)
    return;
    */

  DWORD needFriendLv = MiscConfig::getMe().getPetCFG().dwHandReqFriendLv;
  bool canHand = false;
  for (auto &l : m_listPetData)
  {
    if (l.qwTempID == qwPetID && l.dwFriendLv >= needFriendLv)
      canHand = true;
  }
  if (!canHand)
  {
    XERR << "[宠物-牵手], 条件不满足, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << qwPetID << XEND;
    return;
  }

  if (m_pUser->m_oHands.has())
    m_pUser->m_oHands.breakup();
  if (m_pUser->getHandNpc().haveHandNpc())
    m_pUser->getHandNpc().delHandNpc();
  if (m_pUser->getWeaponPet().haveHandCat())
    m_pUser->getWeaponPet().breakHand();

  if (handPet())
    breakHand();

  for (auto &l : m_listPetData)
  {
    if (l.bLive == false || l.qwTempID != qwPetID)
      continue;
    l.bHandStatus = true;
    l.playAction(EPETACTION_OWNER_HAND);
    sendHandMark(qwPetID);
  }
}

void UserPet::breakHand()
{
  for (auto &l : m_listPetData)
  {
    if (!l.bHandStatus)
      continue;
    l.bHandStatus = false;
    changeState(l, EPETSTATAE_WAIT_BATTLE);
    sendHandBreak(l.qwTempID);
  }
}

void UserPet::testhand(bool build)
{
  for (auto &l : m_listPetData)
  {
    if (build)
      inviteHand(l.qwTempID);
    else
      breakHand();
    break;
  }
}

void UserPet::updatePetState(SScenePetData& stData)
{
  if (stData.bLive == false)
    return;

  switch(stData.eState)
  {
    case EPETSTATAE_NORAML:
      if (now() > stData.dwLastBattleTime + MiscConfig::getMe().getWeaponPetCFG().dwOffOwnerToBattleTime)
      {
        changeState(stData, EPETSTATAE_WAIT_BATTLE);
      }
      break;
    case EPETSTATAE_WAIT_BATTLE:
      {
        if (stData.bHandStatus)
        {
          SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempID);
          if (npc)
          {
            if (npc->m_oMove.empty())
              npc->m_ai.moveTo(m_pUser->getPos());
            if (getXZDistance(npc->getPos(), m_pUser->getPos()) < 2 && npc->getAction() == 0)
              changeState(stData, EPETSTATAE_HAND);
          }
        }
      }
      break;
    case EPETSTATAE_HAND:
      break;
    case EPETSTATAE_EATFOOD:
      {
        if (stData.dwEatBeginTime == 0)
        {
          SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempID);
          if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
            break;
          if (getXZDistance(npc->m_oMove.getDestPos(), npc->getPos()) > 3)
            break;
          stData.dwEatBeginTime = now();

          // 料理没有了
          SceneNpc* foodnpc = SceneNpcManager::getMe().getNpcByTempID(stData.qwTempFoodNpcID);
          if (foodnpc == nullptr)
          {
            changeState(stData, EPETSTATAE_WAIT_BATTLE);
            break;
          }
          DWORD skill = MiscConfig::getMe().getPetCFG().dwEatFoodSkill;
          npc->useSkill(skill, foodnpc->id, foodnpc->getPos());

          FoodNpc* pFoodNpc = dynamic_cast<FoodNpc*>(foodnpc);
          if (pFoodNpc)
            pFoodNpc->addPetEater(npc);
        }
        else if(now() >= stData.dwEatBeginTime + 18)
        {
          // 料理吃完了
          changeState(stData, EPETSTATAE_WAIT_BATTLE);
        }
      }
      break;
    case EPETSTATAE_BETOUCH:
      break;
    default:
      break;
  }
}

void UserPet::sendBattlePets()
{
  QueryBattlePetCmd cmd;
  for (auto &l : m_listPetData)
  {
    DWORD dwItemID = PetConfig::getMe().getItemIDByPet(l.dwID);
    if (dwItemID == 0)
      continue;

    ItemData* pData = cmd.add_pets();
    l.toEggData(pData->mutable_egg());
    pData->mutable_base()->set_id(dwItemID);
    pData->mutable_egg()->clear_var();
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[宠物-出战宠物]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取列表" << cmd.ShortDebugString() << XEND;
}

void UserPet::onSeeFood(SceneNpc* foodNpc)
{
  if (foodNpc == nullptr)
    return;
  // check food can eat
  // ..

  for (auto &l : m_listPetData)
  {
    if (l.bLive == false)
      continue;
    // check full
    if (l.canFeed() != EERROR_SUCCESS)
    {
      FoodNpc* pNpc = dynamic_cast<FoodNpc*>(foodNpc);
      if (pNpc != nullptr && pNpc->getOwnerID() == m_pUser->id)
        l.playAction(EPETACTION_OWNER_NO_FEED);
      continue;
    }

    SceneNpc* pPet = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
    if (pPet == nullptr || pPet->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;

    if (l.eState == EPETSTATAE_EATFOOD)
      continue;
    changeState(l, EPETSTATAE_EATFOOD);
    l.qwTempFoodNpcID = foodNpc->id;
    pPet->m_ai.moveTo(foodNpc->getPos());
  }
}

bool UserPet::del(DWORD petid)
{
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return petid == r.dwID;
  });
  if (it == m_listPetData.end())
    return false;
  leaveScene(*it);
  m_listPetData.erase(it);
  return true;
}

DWORD UserPet::getBaseLv(QWORD petguid) const
{
  for (auto &l : m_listPetData)
  {
    if (l.qwTempID == petguid)
      return l.dwLv;
  }
  return 0;
}

SceneNpc* UserPet::getPetNpc() const
{
  for (auto &l : m_listPetData)
  {
    if (l.bLive)
      return SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
  }
  return nullptr;
}

SScenePetData* UserPet::getPetData(DWORD dwPetID)
{
  if (dwPetID == 0)
    return m_listPetData.empty() == true ? nullptr : &(*m_listPetData.begin());

  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return dwPetID == r.dwID;
      });
  if (it == m_listPetData.end())
    return nullptr;
  return &(*it);
}

void UserPet::collectPetList(TSetDWORD& setIDs)
{
  setIDs.clear();
  for (auto &l : m_listPetData)
    setIDs.insert(l.dwID);
}

bool UserPet::hasPet(DWORD dwPetID, DWORD dwBaseLv, DWORD dwFriendLv) const
{
  for (auto &l : m_listPetData)
  {
    if (dwPetID != 0 && dwPetID != l.dwID)
      continue;
    if (l.dwLv >= dwBaseLv && l.dwFriendLv >= dwFriendLv)
      return true;
  }
  return false;
}

void UserPet::onStopFood(SceneNpc* foodNpc)
{
  for (auto &l : m_listPetData)
  {
    if (l.eState == EPETSTATAE_EATFOOD)
      changeState(l, EPETSTATAE_WAIT_BATTLE);
  }
}


void UserPet::setCatchPetID(QWORD npcguid)
{
  if (m_qwCatchPetID == npcguid)
    return;

  m_qwCatchPetID = npcguid;
  if (npcguid)
  {
    TSetQWORD targetSet;
    targetSet.insert(npcguid);
    m_pUser->getSpEffect().add(MiscConfig::getMe().getPetCFG().dwCatchLineID, targetSet);
  }
}

void UserPet::clearCatchPetID(QWORD npcguid)
{
  if (m_qwCatchPetID != npcguid)
    return;
  m_qwCatchPetID = 0;

  m_pUser->getSpEffect().del(MiscConfig::getMe().getPetCFG().dwCatchLineID, 0);
}

void UserPet::onUserOffCarrier()
{
  for (auto &l : m_listPetData)
  {
    if (l.eState == EPETSTATAE_HAND)
    {
      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(l.qwTempID);
      if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
        npc->setScenePos(m_pUser->getPos());

      changeState(l, EPETSTATAE_WAIT_BATTLE);
    }
  }
}

void UserPet::onUserStartTouch(QWORD npcguid)
{
  for (auto &l : m_listPetData)
  {
    if (l.qwTempID != npcguid)
      continue;
    changeState(l, EPETSTATAE_BETOUCH);
  }
}

void UserPet::onUserStopTouch()
{
  for (auto &l : m_listPetData)
  {
    if (l.eState == EPETSTATAE_BETOUCH)
      changeState(l, EPETSTATAE_WAIT_BATTLE);
  }
}

/*void UserPet::checkChangeLevel(SScenePetData& stData)
{
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD maxlv = rCFG.dwOverUserLv + m_pUser->getLevel();

  // 宠物等级大于当前玩家可携带等级(仓库, 大号转小号)
  if (stData.dwLv > maxlv)
  {
    XLOG << "[宠物], 大号转小号, 等级扣除, 前等级:" << stData.dwLv << "设置后等级:" << maxlv << " 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << stData.dwID << stData.strName << XEND;
    stData.dwHp = 0;

    for (DWORD d = maxlv + 1; d <= stData.dwLv; ++d)
    {
      const SPetBaseLvCFG* pCFG = PetConfig::getMe().getPetBaseLvCFG(d);
      if (pCFG == nullptr)
      {
        XERR << "[宠物], 大号转小号, 等级扣除, 等级 :" << d << "未在 Table_PetBaseLevel.txt 表中找到" << m_pUser->name << m_pUser->id << "宠物:" << stData.dwID << stData.strName << XEND;
        continue;
      }
      stData.qwExp += pCFG->qwNewCurExp;
      XLOG << "[宠物], 大号转小号, 等级扣除, 等级 :" << d << "补偿经验 :" << pCFG->qwNewCurExp << m_pUser->name << m_pUser->id << "宠物:" << stData.dwID << stData.strName << XEND;
    }
    stData.dwLv = maxlv;
  }
  else
  {
    XLOG << "[宠物], 小号转大号, 多余经验升级开始, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << stData.dwID << stData.strName << XEND;
    stData.dwHp = 0;
    stData.baseLevelup();
  }
}*/

bool UserPet::compose(const ComposePetCmd& cmd)
{
  DWORD petid = cmd.id();

  const SPetComposeCFG* pComposeCFG = PetConfig::getMe().getComposeCFG(petid);
  const SPetCFG* pPetCFG = PetConfig::getMe().getPetCFG(petid);
  if (pComposeCFG == nullptr || pPetCFG == nullptr)
  {
    XERR << "[宠物-融合], 合成的宠物非法, id:" << petid << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }
  if (pComposeCFG->dwZenyCost && m_pUser->checkMoney(EMONEYTYPE_SILVER, pComposeCFG->dwZenyCost) == false)
  {
    XERR << "[宠物-融合], zeny不足, petid:" << petid << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }

  // check material
  std::vector<ItemEgg*> vecEggs; // 材料判断时, 有序判断
  std::set<ItemEgg*> setEggs; // 检查重复
  for (int i = 0; i < cmd.eggguids_size(); ++i)
  {
    ItemEgg* pEgg = dynamic_cast<ItemEgg*> (m_pUser->getPackage().getItem(cmd.eggguids(i)));
    if (pEgg == nullptr)
    {
      XERR << "[宠物-融合], 无效guid:" << cmd.eggguids(i) << "玩家:" << m_pUser->name << m_pUser->id << XEND;
      return false;
    }
    if (setEggs.find(pEgg) != setEggs.end())
    {
      XERR << "[宠物-融合], 材料重复, 非法, petid:" << petid << "玩家:" << m_pUser->name << m_pUser->id << XEND;
      return false;
    }
    vecEggs.push_back(pEgg);
    setEggs.insert(pEgg);
  }
  if (vecEggs.size() != pComposeCFG->vecPetMaterials.size())
  {
    XERR << "[宠物-融合], 非法, 材料数量不足" << petid << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }

  DWORD index = 0;
  for (auto &v : pComposeCFG->vecPetMaterials)
  {
    ItemEgg* pEgg = vecEggs[index++];
    if (pEgg->getPetID() != v.dwPetID)
    {
      XERR << "[宠物-融合], 材料宠物id不匹配:" << pEgg->getPetID() << v.dwPetID << "玩家:" << m_pUser->name << m_pUser->id << "待合成宠物:" << petid << XEND;
      return false;
    }
    if (pEgg->getLevel() < v.dwBaseLv)
    {
      XERR << "[宠物-融合], 材料宠物等级不匹配:" << v.dwPetID << "等级:" << pEgg->getLevel() << "玩家:" << m_pUser->name << m_pUser->id << "待合成宠物:" << petid << XEND;
      return false;
    }
    if (pEgg->getFriendLv() < v.dwFriendLv)
    {
      XERR << "[宠物-融合], 材料宠物好感度不匹配:" << v.dwPetID << "好感等级:" << pEgg->getFriendLv() << "玩家:" << m_pUser->name << m_pUser->id << "待合成宠物:" << petid << XEND;
      return false;
    }
  }

  // reduce materials
  for (auto &p : vecEggs)
    m_pUser->getPackage().reduceItem(p->getGUID(), 1, ESOURCE_COMPOSE, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PETWORK);
  m_pUser->subMoney(EMONEYTYPE_SILVER, pComposeCFG->dwZenyCost, ESOURCE_COMPOSE);

  // compose new pet egg
  ItemInfo item;
  item.set_id(pPetCFG->dwEggID);
  item.set_count(1);
  m_pUser->getPackage().addItem(item, EPACKMETHOD_AVAILABLE);

  m_pUser->getAchieve().onComposePet();

  XLOG << "[宠物-融合], 合成宠物蛋成功, 宠物蛋id:" << pPetCFG->dwEggID << "宠物id:" << petid << "玩家:" << m_pUser->name << m_pUser->id << XEND;
  return true;
}

void UserPet::sendUnlockInfo()
{
  PetEquipListCmd cmd;
  m_oEquipUnlockInfo.toData(cmd.mutable_unlockinfo());
  if (cmd.unlockinfo().items_size() || cmd.unlockinfo().bodyitems_size())
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

bool UserPet::unlockWear(DWORD itemid)
{
  if (m_oEquipUnlockInfo.setUnlockItems.find(itemid) != m_oEquipUnlockInfo.setUnlockItems.end())
  {
    XERR << "[宠物-解锁装扮], 重复解锁, 装备id:" << itemid << XEND;
    return false;
  }
  m_oEquipUnlockInfo.setUnlockItems.insert(itemid);

  UpdatePetEquipListCmd cmd;
  cmd.add_additems(itemid);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[宠物-解锁装扮], 成功解锁, 装备id:" << itemid << XEND;
  return true;
}

bool UserPet::unlockWear(DWORD itemid, DWORD body, EEquipPos pos)
{
  for (auto &v : m_oEquipUnlockInfo.vecSpecUnlockInfo)
  {
    if (v.itemid() == itemid && body == v.bodyid())
    {
      if (v.epos() == EEQUIPPOS_MIN)
      {
        XERR << "[宠物-解锁装扮], 当前部位已全部解锁, 无需单独解锁," << itemid << body << pos << "玩家:" << m_pUser->name << m_pUser->id << XEND;
        return false;
      }
      if (v.epos() == pos)
      {
        XERR << "[宠物-解锁装扮], 当前部位已全部解锁, 无需单独解锁," << itemid << body << pos << "玩家:" << m_pUser->name << m_pUser->id << XEND;
        return false;
      }
    }
  }

  SpecPetEquip info;
  info.set_itemid(itemid);
  info.set_bodyid(body);
  info.set_epos(pos);

  m_oEquipUnlockInfo.vecSpecUnlockInfo.push_back(info);

  UpdatePetEquipListCmd cmd;
  cmd.add_addbodyitems()->CopyFrom(info);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[宠物-解锁装扮], 装备:" << itemid << "body:" << body << "位置:" << pos << "玩家:" << m_pUser->name << m_pUser->id << XEND;
  return true;
}

bool UserPet::checkWearUnlock(DWORD itemid, DWORD body, EEquipPos pos)
{
  if (m_oEquipUnlockInfo.setUnlockItems.find(itemid) != m_oEquipUnlockInfo.setUnlockItems.end())
    return true;
  for (auto &v : m_oEquipUnlockInfo.vecSpecUnlockInfo)
  {
    if (v.bodyid() == body && v.itemid() == itemid)
    {
      if (v.epos() == EEQUIPPOS_MIN || v.epos() == pos)
        return true;
    }
  }

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(itemid);
  if (pCFG && pCFG->isForbid(EFORBID_UNLOCKPET) == false)
  {
    SManualItem* pItem = m_pUser->getManual().getManualItem(EMANUALTYPE_FASHION);
    if (pItem == nullptr)
      return false;

    for (auto &v : pItem->vecSubItems)
    {
      if (v.dwID == itemid && v.bStore)
        return true;
    }
  }
  return false;
}

EUserDataType UserPet::getDataTypeByPos(EEquipPos epos)
{
  static const map<EEquipPos, EUserDataType> pos2type = {
    {EEQUIPPOS_FACE, EUSERDATATYPE_FACE},
    {EEQUIPPOS_HEAD, EUSERDATATYPE_HEAD},
    {EEQUIPPOS_BACK, EUSERDATATYPE_BACK},
    {EEQUIPPOS_TAIL, EUSERDATATYPE_TAIL},
    {EEQUIPPOS_MOUTH, EUSERDATATYPE_MOUTH},
  };
  auto it = pos2type.find(epos);
  return it != pos2type.end() ? it->second : EUSERDATATYPE_MIN;
}

bool UserPet::changeWear(const ChangeWearPetCmd& cmd)
{
  DWORD dwPetID = cmd.petid();
  auto it = find_if(m_listPetData.begin(), m_listPetData.end(), [&](const SScenePetData& r) ->bool {
      return dwPetID == r.dwID;
      });
  if (it == m_listPetData.end())
  {
    XERR << "[宠物-换装], 找不到对应的宠物, 宠物:" << dwPetID << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }
  SceneNpc* pPet = SceneNpcManager::getMe().getNpcByTempID(it->qwTempID);
  if (pPet == nullptr)
  {
    XERR << "[宠物-换装], 宠物不在场景中, 不可更换, 宠物:" << dwPetID << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }

  UpdateWearPetCmd message;
  message.set_petid(dwPetID);
  for (int i = 0; i < cmd.wearinfo_size(); ++i)
  {
    const PetWearInfo& info = cmd.wearinfo(i);
    EEquipPos epos = info.epos();

    EUserDataType edatatype = getDataTypeByPos(epos);
    if (edatatype == EUSERDATATYPE_MIN)
      continue;

    if (info.oper() == EPETEQUIPOPER_ON)
    {
      DWORD itemid = info.itemid();
      DWORD curbody = it->getBodyID();
      if (checkWearUnlock(itemid, curbody, epos) == false)
      {
        XERR << "[宠物-换装], 更换失败, 装备未解锁, 宠物id:" << dwPetID << "body:" << curbody << "部位:" << epos << "装备id:" << itemid << "玩家:" << m_pUser->name << m_pUser->id << XEND;
        continue;
      }

      auto oldp = it->mapCurPos2EquipID.find(epos);
      if (oldp != it->mapCurPos2EquipID.end() && oldp->second == itemid)
      {
        XERR << "[宠物-换装], 更换失败, 重复装扮, 宠物id:" << dwPetID << "body:" << curbody << "部位:" << epos << "装备id:" << itemid << "玩家:" << m_pUser->name << m_pUser->id << XEND;
        continue;
      }
      it->mapCurPos2EquipID[epos] = itemid;
      pPet->setDataMark(edatatype);
      XLOG << "[宠物-换装], 更换成功, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << dwPetID << "body:" << curbody << "部位:" << epos << "装备id:" << itemid << XEND;
    }
    else if (info.oper() == EPETEQUIPOPER_OFF)
    {
      auto m = it->mapCurPos2EquipID.find(epos);
      if (m == it->mapCurPos2EquipID.end())
      {
        XERR << "[宠物-换装], 脱卸失败, 该部位没有装备, 宠物:" << dwPetID << "部位:" << epos << "玩家:" << m_pUser->name << m_pUser->id << XEND;
        continue;
      }
      it->mapCurPos2EquipID.erase(m);
      pPet->setDataMark(edatatype);
      XLOG << "[宠物-换装], 脱卸成功, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << dwPetID << "部位:" << epos << XEND;
    }
    else if (info.oper() == EPETEQUIPOPER_DELETE)
    {
      auto m = it->mapCurPos2EquipID.find(epos);
      if (m != it->mapCurPos2EquipID.end())
      {
        it->mapCurPos2EquipID.erase(m);
        pPet->setDataMark(edatatype);
        XLOG << "[宠物-换装], 一键删除, 脱卸成功, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << dwPetID << "部位:" << epos << XEND;
      }
      auto s = it->mapInitPos2EquipID.find(epos);
      if (s != it->mapInitPos2EquipID.end())
      {
        XLOG << "[宠物-换装], 一键删除, 删除默认装备, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << dwPetID << "部位:" << epos << "装备:" << s->second << XEND;

        it->mapInitPos2EquipID.erase(s);
        pPet->setDataMark(edatatype);
      }
    }

    message.add_wearinfo()->CopyFrom(info);
  }

  if (message.wearinfo_size() > 0)
  {
    pPet->refreshDataAtonce();
    PROTOBUF(message, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  return true;
}

void UserPet::onManualOffEquip(DWORD itemid)
{
  if (m_oEquipUnlockInfo.setUnlockItems.find(itemid) != m_oEquipUnlockInfo.setUnlockItems.end())
    return;
  auto checkSpecUnlock = [&](DWORD body, DWORD pos) -> bool
  {
    for (auto &v : m_oEquipUnlockInfo.vecSpecUnlockInfo)
    {
      if (v.bodyid() == body && v.itemid() == itemid)
      {
        if (v.epos() == EEQUIPPOS_MIN || v.epos() == pos)
          return true;
      }
    }
    return false;
  };

  UpdateWearPetCmd message;
  for (auto &s : m_listPetData)
  {
    SceneNpc* pet = SceneNpcManager::getMe().getNpcByTempID(s.qwTempID);

    for (auto m = s.mapCurPos2EquipID.begin(); m != s.mapCurPos2EquipID.end(); )
    {
      if (m->second == itemid)
      {
        auto it = s.mapInitPos2EquipID.find(m->first);
        bool isdefault = (it != s.mapInitPos2EquipID.end() && it->second == m->second);
        if (!isdefault && checkSpecUnlock(s.getBodyID(), m->first) == false)
        {
          if (pet && pet->isAlive())
          {
            pet->setDataMark(getDataTypeByPos(m->first));
            pet->refreshDataAtonce();
          }

          PetWearInfo* pInfo = message.add_wearinfo();
          if (pInfo)
          {
            pInfo->set_oper(EPETEQUIPOPER_OFF);
            pInfo->set_epos(m->first);
          }
          XLOG << "[宠物-换装], 冒险手册取出, 导致宠物拆卸装备, 玩家:" << m_pUser->name << m_pUser->id << "宠物:" << s.dwID << "部位:" << m->first << "装备id:" << itemid << XEND;

          m = s.mapCurPos2EquipID.erase(m);
          continue;
        }
      }
      ++m;
    }

    if (message.wearinfo_size())
    {
      message.set_petid(s.dwID);
      PROTOBUF(message, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
  }
}
DWORD UserPet::getFirendFullNum() const
{
  DWORD num = 0;
  for (auto &l : m_listPetData)
  {
    const SPetFriendLvCFG* pCFG = PetConfig::getMe().getFriendLvCFG(l.dwID, l.dwFriendLv + 1);
    if(pCFG == nullptr)
      ++num;
  }

  BasePackage* pPack = m_pUser->getPackage().getPackage(EPACKTYPE_PET);
  if(pPack == nullptr)
    return num;

  auto func = [&](ItemBase* pBase)
  {
    if(pBase == nullptr)
      return;
    ItemEgg* pEgg = dynamic_cast<ItemEgg*>(pBase);
    if(pEgg == nullptr || pEgg->getFriendLv() == 0)
      return;

    const SPetFriendLvCFG* pCFG = PetConfig::getMe().getFriendLvCFG(pEgg->getPetID(), pEgg->getFriendLv() + 1);
    if(pCFG == nullptr)
      ++num;
  };
  pPack->foreach(func);

  return num;
}
