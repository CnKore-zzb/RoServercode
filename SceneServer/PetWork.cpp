#include "PetWork.h"
#include "SceneUser.h"
#include "PetConfig.h"
#include "ProtoCommon.pb.h"
#include "Menu.h"
#include "ActivityManager.h"
#include "MailManager.h"

// work space
void SWorkSpace::fromData(const Cmd::WorkSpace& rData)
{
  dwID = rData.id();
  dwStartTime = rData.starttime();
  dwLastRewardTime = rData.lastrewardtime();
  bUnlock = rData.unlock();
  eState = rData.state();

  mapEggs.clear();
  for (int i = 0; i < rData.datas_size(); ++i)
  {
    const ItemData& rEgg = rData.datas(i);
    mapEggs[rEgg.base().guid()].CopyFrom(rEgg);
  }
  vecCounts.clear();
  for (int i = 0; i < rData.counts_size(); ++i)
    vecCounts.push_back(rData.counts(i));
  vecLastCounts.clear();
  for (int i = 0; i < rData.last_counts_size(); ++i)
    vecLastCounts.push_back(rData.last_counts(i));
}

void SWorkSpace::toData(Cmd::WorkSpace* pData)
{
  if (pData == nullptr)
    return;

  pData->Clear();

  pData->set_id(dwID);
  pData->set_starttime(dwStartTime);
  pData->set_lastrewardtime(dwLastRewardTime);
  pData->set_unlock(bUnlock);
  pData->set_state(eState);

  for (auto &m : mapEggs)
    pData->add_datas()->CopyFrom(m.second);
  for (auto &v : vecCounts)
    pData->add_counts(v);
  for (auto &v : vecLastCounts)
    pData->add_last_counts(v);
}

void SWorkSpace::toClient(Cmd::WorkSpace* pData)
{
  if (pData == nullptr)
    return;

  pData->Clear();

  pData->set_id(dwID);
  pData->set_starttime(dwStartTime);
  pData->set_lastrewardtime(dwLastRewardTime);
  pData->set_unlock(bUnlock);
  pData->set_state(eState);

  for (auto &m : mapEggs)
  {
    ItemData* pEgg = pData->add_datas();

    pEgg->mutable_base()->set_guid(m.second.base().guid());
    pEgg->mutable_base()->set_id(m.second.base().id());

    pEgg->mutable_egg()->set_id(m.second.egg().id());
    pEgg->mutable_egg()->set_lv(m.second.egg().lv());
    pEgg->mutable_egg()->set_friendlv(m.second.egg().friendlv());
    pEgg->mutable_egg()->set_body(m.second.egg().body());
    pEgg->mutable_egg()->set_name(m.second.egg().name());

    for (int i = 0; i < m.second.egg().equips_size(); ++i)
      pEgg->mutable_egg()->add_equips()->mutable_base()->set_id(m.second.egg().equips(i).base().id());

    pEgg->mutable_egg()->clear_skillids();
    for (int i = 0; i < m.second.egg().skillids_size(); ++i)
      pEgg->mutable_egg()->add_skillids(m.second.egg().skillids(i));
  }
  for (auto &v : vecCounts)
    pData->add_counts(v);
  for (auto &v : vecLastCounts)
    pData->add_last_counts(v);
}

void SWorkPetExtra::fromData(const Cmd::WorkPetExtra& rData)
{
  guid = rData.guid();
  dwLastSpaceID = rData.lastspaceid();
  dwCount = rData.count();
}

void SWorkPetExtra::toData(Cmd::WorkPetExtra* pData)
{
  if (pData == nullptr)
    return;

  pData->Clear();
  pData->set_guid(guid);
  pData->set_lastspaceid(dwLastSpaceID);
  pData->set_count(dwCount);
}

// pet work
PetWork::PetWork(SceneUser* pUser) : m_pUser(pUser)
{

}

PetWork::~PetWork()
{

}

bool PetWork::load(const Cmd::BlobPetWork& rData)
{
  m_oManual.Clear();
  m_oManual.CopyFrom(rData.manual());

  m_mapWorkSpace.clear();
  for (int i = 0; i < rData.datas_size(); ++i)
  {
    const WorkSpace& rSpace = rData.datas(i);
    m_mapWorkSpace[rSpace.id()].fromData(rSpace);
  }

  m_mapPetExtra.clear();
  for (int i = 0; i < rData.pets_size(); ++i)
  {
    const WorkPetExtra& rExtra = rData.pets(i);
    m_mapPetExtra[rExtra.guid()].fromData(rExtra);
  }

  m_mapLastReward.clear();
  for (int i = 0; i < rData.rewards_size(); ++i)
  {
    const WorkDayReward& rReward = rData.rewards(i);
    m_mapLastReward[rReward.id()] = rReward.time();
  }

  m_dwCardExpireTime = rData.card_expiretime();

  refreshWorkSpace(m_bQueryData);
  resetPetExtra();
  return true;
}

bool PetWork::save(Cmd::BlobPetWork* pData)
{
  if (pData == nullptr)
    return false;

  pData->mutable_manual()->CopyFrom(m_oManual);

  pData->clear_datas();
  for (auto &m : m_mapWorkSpace)
    m.second.toData(pData->add_datas());

  resetPetExtra();
  pData->clear_pets();
  for (auto &m : m_mapPetExtra)
    m.second.toData(pData->add_pets());

  pData->clear_rewards();
  for (auto &m : m_mapLastReward)
  {
    WorkDayReward* pReward = pData->add_rewards();
    pReward->set_id(m.first);
    pReward->set_time(m.second);
  }

  pData->set_card_expiretime(m_dwCardExpireTime);
  XDBG << "[宠物打工-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小" << pData->ByteSize() << XEND;
  return true;
}

void PetWork::reload()
{
  for (auto &m : m_mapWorkSpace)
  {
    SWorkSpace& rSpace = m.second;
    rSpace.pCFG = PetConfig::getMe().getWorkCFG(m.first);
  }
  XLOG << "[宠物打工-重加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进行重加载" << XEND;
}

void PetWork::queryWorkManual()
{
  Cmd::QueryPetWorkManualPetCmd cmd;
  cmd.mutable_manual()->CopyFrom(m_oManual);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XDBG << "[宠物打工-手册同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步手册数据" << cmd.ShortDebugString() << XEND;
}

void PetWork::queryWorkData()
{
  refreshWorkSpace();
  resetPetExtra();

  Cmd::QueryPetWorkDataPetCmd cmd;
  for (auto &m : m_mapWorkSpace)
  {
    if (m.second.pCFG == nullptr)
      continue;
    const SPetWorkCFG* pCFG = m.second.pCFG;
    if (pCFG->dwActID != 0 && ActivityManager::getMe().isOpen(pCFG->dwActID) == false)
      continue;
    m.second.toClient(cmd.add_datas());
  }
  for (auto &m : m_mapPetExtra)
    m.second.toData(cmd.add_extras());
  cmd.set_max_space(getMaxWorkCount());
  cmd.set_card_expiretime(m_dwCardExpireTime);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_bQueryData = true;
  XDBG << "[宠物打工-场所同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步场所数据" << cmd.ShortDebugString() << XEND;
}

void PetWork::refreshWorkSpace(bool bNtf /*= true*/)
{
  if (m_oManual.unlock() == false)
    return;

  const TMapPetWorkCFG& mapCFG = PetConfig::getMe().getWorkCFGList();
  for (auto &m : mapCFG)
  {
    SWorkSpace* pSpace = getSpaceData(m.first);
    if (pSpace == nullptr)
    {
      if (checkEnable(m.second.stOpenCond, m_pUser) == false)
        continue;

      pSpace = &m_mapWorkSpace[m.first];
      pSpace->dwID = m.first;
      m_setUpdateIDs.insert(pSpace->dwID);
    }
    pSpace->pCFG = &m.second;

    if (pSpace->dwID == PET_WORK_SPACE_CAPRA)
    {
      refreshCapra(*pSpace);
      m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_PETWORK_SPACE);
      continue;
    }
    if (pSpace->pCFG->dwActID != 0)
    {
      refreshActivity(*pSpace);
      continue;
    }

    if (!pSpace->bUnlock)
    {
      if (checkEnable(m.second.stUnlockCond, m_pUser) == false)
        continue;

      pSpace->bUnlock = true;
      pSpace->eState = EWORKSTATE_UNUSED;
      m_setUpdateIDs.insert(pSpace->dwID);
      m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_PETWORK_SPACE);
    }
  }

  if (bNtf)
    update();
  else
    m_setUpdateIDs.clear();
}

EError PetWork::unlockManual(bool bCheck /*= true*/)
{
  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[宠物打工-解锁手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "解锁手册失败,未找到" << EPACKTYPE_MAIN << XEND;
    return EERROR_FAIL;
  }

  const string& guid = pMainPack->getGUIDByType(ITEM_PET_WORK);
  ItemBase* pBase = pMainPack->getItem(guid);
  if (pBase == nullptr)
  {
    XERR << "[宠物打工-解锁手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "解锁手册失败,未找到打工手册" << ITEM_PET_WORK << XEND;
    return EERROR_FAIL;
  }

  if (m_oManual.unlock() == true)
  {
    XERR << "[宠物打工-解锁手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "解锁手册失败,打工手册已激活" << XEND;
    return EERROR_FAIL;
  }

  if (bCheck)
  {
    Menu& rMenu = m_pUser->getMenu();
    rMenu.refreshNewMenu(EMENUCOND_UNLOCK);
    if (rMenu.isOpen(EMENUID_PET_WORK_UNLOCK) == false)
    {
      MsgManager::sendMsg(m_pUser->id, 66);
      return EERROR_FAIL;
    }
  }

  m_oManual.set_unlock(true);
  queryWorkManual();

  XLOG << "[宠物打工-解锁手册]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "解锁手册成功" << (bCheck ? "没有" : "使用") << "GM指令" << XEND;
  return EERROR_SUCCESS;
}

EError PetWork::startWork(const Cmd::StartWorkPetCmd& cmd)
{
  const SPetWorkCFG* pCFG = PetConfig::getMe().getWorkCFG(cmd.id());
  if (pCFG == nullptr)
  {
    XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << cmd.id() << "开始打工失败, 该场所未在 Table_WorkSpace.txt 表中找到" << XEND;
    return EERROR_FAIL;
  }
  if (pCFG->dwActID == 0)
  {
    DWORD dwCurCount = getCurWorkCount();
    DWORD dwMaxCount = getMaxWorkCount();
    if (dwCurCount >= dwMaxCount)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在场所" << cmd.id() << "开始打工失败, 当前打工场所数量" << dwCurCount << "超过最大打工上限" << dwMaxCount << XEND;
      return EERROR_FAIL;
    }
  }
  SWorkSpace* pSpace = getSpaceData(cmd.id());
  if (pSpace == nullptr)
  {
    return EERROR_FAIL;
  }
  if (!pSpace->bUnlock)
  {
    XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << pSpace->dwID << "开始打工失败, 该场所未开放或未解锁" << XEND;
    return EERROR_FAIL;
  }
  if (pSpace->eState != EWORKSTATE_UNUSED)
  {
    XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << pSpace->dwID << "开始打工失败, 该场所状态 :" << pSpace->eState << "未处于空闲状态" << XEND;
    return EERROR_FAIL;
  }
  if (pCFG->dwActID != 0 && ActivityManager::getMe().isOpen(pCFG->dwActID) == false)
  {
    XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "在场所" << pSpace->dwID << "开始打工失败, 该场所状态 :" << pSpace->eState << "的活动" << pCFG->dwActID << "未开启" << XEND;
    return EERROR_FAIL;
  }

  TSetString setEggIDs;
  for (int i = 0; i < cmd.pets_size(); ++i)
    setEggIDs.insert(cmd.pets(i));
  if (setEggIDs.size() > 1)
  {
    XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << pSpace->dwID << "开始打工失败, 该场所不支持同时" << setEggIDs.size() << "个宠物同事打工" << XEND;
    return EERROR_FAIL;
  }

  Package& rPackage = m_pUser->getPackage();
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  set<ItemEgg*> setDatas;
  for (auto &s : setEggIDs)
  {
    if (rPackage.checkItemCount(s, 1, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PETWORK) == false)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << "未找到" << XEND;
      return EERROR_FAIL;
    }

    ItemEgg* pEgg = dynamic_cast<ItemEgg*>(rPackage.getItem(s));
    if (pEgg == nullptr)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << "未找到" << XEND;
      return EERROR_FAIL;
    }
    if (pCFG->isForbid(pEgg->getPetID()) == true)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << pEgg->getPetID() << "不允许在该场所打工" << XEND;
      return EERROR_FAIL;
    }
    if (pEgg->getName().empty() == true)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << pEgg->getPetID() << "未孵化过" << XEND;
      return EERROR_FAIL;
    }
    if (pEgg->getLevel() < pCFG->dwPetLvReq)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << pEgg->getPetID() << "等级" << pEgg->getLevel() << "低于场所要求" << pCFG->dwPetLvReq << XEND;
      return EERROR_FAIL;
    }
    if (pEgg->getFriendLv() < rCFG.dwWorkManualFriendLv)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << pEgg->getPetID() << "好感度等级" << pEgg->getFriendLv() << "低于要求" << rCFG.dwWorkManualFriendLv << XEND;
      return EERROR_FAIL;
    }
    if (pCFG->dwActID == 0)
    {
      const SWorkPetExtra* pPetExtra = getPetExchange(pEgg->getGUID());
      if (pPetExtra != nullptr && pPetExtra->dwLastSpaceID != pSpace->dwID && pPetExtra->dwCount >= rCFG.dwWorkMaxExchange)
      {
        XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << pEgg->getPetID() << "超过最大转换上限" << rCFG.dwWorkMaxExchange << "次" << XEND;
        return EERROR_FAIL;
      }
    }
    if (ItemEgg::getWorkSkillID(pEgg->getEggData()) == 0)
    {
      XERR << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "在场所" << pSpace->dwID << "开始打工失败, 宠物 :" << s << pEgg->getPetID() << "未包含打工技能" << XEND;
      return EERROR_FAIL;
    }
    setDatas.insert(pEgg);
  }

  for (auto &s : setDatas)
  {
    ItemData oData;
    s->toItemData(&oData);

    pSpace->mapEggs[oData.base().guid()].CopyFrom(oData);
    addPetExchangeCount(oData.base().guid(), *pSpace);
    rPackage.reduceItem(s->getGUID(), oData.base().count(), ESOURCE_PET_WORK, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PETWORK);
  }

  pSpace->vecCounts.clear();
  DWORD& dwTime = m_mapLastReward[pSpace->dwID];
  DWORD dwNow = xTime::getCurSec();
  if (dwNow > dwTime)
    pSpace->vecLastCounts.clear();

  pSpace->dwStartTime = pSpace->dwLastRewardTime = xTime::getCurSec();
  pSpace->eState = EWORKSTATE_WORKING;

  m_setUpdateIDs.insert(pSpace->dwID);
  update();

  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_START_PETWORK);

  XLOG << "[宠物打工-开始打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "在场所" << pSpace->dwID << "开始打工成功,开始时间" << pSpace->dwStartTime << "上次重置时间" << dwTime << "状态" << pSpace->eState << "场所宠物为";
  for (auto &m : pSpace->mapEggs)
    XLOG << m.second.ShortDebugString();
  XLOG << XEND;
  return EERROR_SUCCESS;
}

EError PetWork::stopWork(const Cmd::StopWorkPetCmd& cmd, EWorkRewardMethod eMethod /*= EWORKREWARDMETHOD_PACK*/)
{
  // check invalid
  SWorkSpace* pSpace = getSpaceData(cmd.id());
  if (pSpace == nullptr)// || !pSpace->bUnlock)
  {
    XERR << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结束场所" << cmd.id() << "打工失败,该场所未开启或未解锁" << XEND;
    return EERROR_FAIL;
  }
  const SPetWorkCFG* pCFG = PetConfig::getMe().getWorkCFG(pSpace->dwID);
  if (pCFG == nullptr)
  {
    XERR << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结束场所" << cmd.id() << "打工失败,该场所未在 Table_Pet_WorkSpace.txt 表中找到" << XEND;
    return EERROR_FAIL;
  }
  if (pSpace->mapEggs.empty() == true)
  {
    XERR << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结束场所" << cmd.id() << "打工失败,该场所未有宠物在打工" << XEND;
    return EERROR_FAIL;
  }

  // check has reward
  DWORD dwEnableCount = 0;
  DWORD dwFrequency = 0;
  DWORD dwNow = xTime::getCurSec();
  if (calcRewardCount(*pSpace, dwNow, dwEnableCount, dwFrequency) == false)
  {
    XERR << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结束场所" << cmd.id() << "打工失败,计算奖励失败" << XEND;
    return EERROR_FAIL;
  }

  DWORD dwDestTime = getDestTime(*pSpace, dwNow);
  DWORD dwMaxReward = LuaManager::getMe().call<DWORD>("calcPetWorkMaxReward", pSpace->dwStartTime, dwDestTime, pCFG->dwMaxReward);
  bool bNeedGet = false;
  if (pSpace->vecCounts.size() < pCFG->vecRewardIDs.size())
    pSpace->vecCounts.resize(pCFG->vecRewardIDs.size());
  if (pSpace->vecLastCounts.size() < pCFG->vecRewardIDs.size())
    pSpace->vecLastCounts.resize(pCFG->vecRewardIDs.size());
  for (size_t i = 0; i < pCFG->vecRewardIDs.size(); ++i)
  {
    if (dwMaxReward > pSpace->vecLastCounts[i])
    {
      DWORD dwMaxSpaceReward = dwMaxReward - pSpace->vecLastCounts[i];
      if (dwEnableCount > dwMaxSpaceReward)
        dwEnableCount = dwMaxSpaceReward;
    }
    if (pSpace->vecCounts[i] < dwEnableCount)
    {
      bNeedGet = true;
      break;
    }
  }
  if (bNeedGet)
  {
    Cmd::GetPetWorkRewardPetCmd scmd;
    scmd.set_id(cmd.id());
    if (getWorkReward(scmd, eMethod) == EERROR_FAIL)
    {
      XERR << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结束场所" << cmd.id() << "打工失败,领取奖励失败" << XEND;
      return EERROR_FAIL;
    }
  }

  // return eggs and reset
  TVecItemData vecEggDatas;
  TSetString setEggNames;
  for (auto &m : pSpace->mapEggs)
  {
    vecEggDatas.push_back(m.second);
    setEggNames.insert(m.second.egg().name());
  }
  TVecItemData vecCopy(vecEggDatas);
  if (eMethod == EWORKREWARDMETHOD_PACK)
  {
    m_pUser->getPackage().addItem(vecEggDatas, EPACKMETHOD_NOCHECK);
    if (!pSpace->bUnlock)
    {
      for (auto &s : setEggNames)
        MsgManager::sendMsg(m_pUser->id, 8113, MsgParams(s));
    }
  }
  else if (eMethod == EWORKREWARDMETHOD_MAIL)
  {
    MailManager::getMe().sendMail(m_pUser->id, 12114, vecEggDatas, MsgParams(), true, true, EMAILTYPE_NORMAL_NOTIME);
    XLOG << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结束场所" << cmd.id() << "打工,通过邮件返回宠物" << vecEggDatas << XEND;
  }
  else
  {
    XERR << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "结束场所" << cmd.id() << "打工失败,method" << eMethod << "非法" << XEND;
    return EERROR_FAIL;
  }

  refreshLastCount(*pSpace, dwNow, dwEnableCount);
  pSpace->eState = EWORKSTATE_UNUSED;
  pSpace->mapEggs.clear();

  m_setUpdateIDs.insert(pSpace->dwID);
  update();

  XLOG << "[宠物打工-结束打工]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "结束场所" << cmd.id() << "打工成功,状态" << pSpace->eState << "归还方式 method :" << eMethod << "归还宠物" << vecCopy << XEND;
  return EERROR_SUCCESS;
}

EError PetWork::getWorkReward(const Cmd::GetPetWorkRewardPetCmd& cmd, EWorkRewardMethod eMethod /*= EWORKREWARDMETHOD_PACK*/)
{
  SWorkSpace* pSpace = getSpaceData(cmd.id());
  if (pSpace == nullptr)
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,该场所未开启" << XEND;
    return EERROR_FAIL;
  }
  if (pSpace->mapEggs.empty() == true)
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,该场所未有宠物在打工" << XEND;
    return EERROR_FAIL;
  }
  const SPetWorkCFG* pCFG = PetConfig::getMe().getWorkCFG(cmd.id());
  if (pCFG == nullptr)
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,该场所未在 Table_Pet_WorkSpace.txt 表中找到" << XEND;
    return EERROR_FAIL;
  }

  DWORD dwNow = xTime::getCurSec();
  if (pSpace->dwStartTime >= dwNow)
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,打工开始时间" << pSpace->dwStartTime << "大于当前时间" << dwNow << XEND;
    return EERROR_FAIL;
  }
  if (pSpace->dwID == PET_WORK_SPACE_CAPRA)
  {
    refreshCapra(*pSpace);
    if (pSpace->dwStartTime >= m_dwCardExpireTime)
    {
      XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "领取场所" << cmd.id() << "的奖励失败,打工开始时间" << pSpace->dwStartTime << "大于月卡时间" << m_dwCardExpireTime << XEND;
      return EERROR_FAIL;
    }
  }
  if (pCFG->dwActID != 0)
  {
    refreshActivity(*pSpace, false);
    DWORD dwEndTime = MiscConfig::getMe().getActCFG().getPetWorkEndTime(pCFG->dwActID);
    if (pSpace->dwStartTime >= dwEndTime)
    {
      XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "领取场所" << cmd.id() << "的奖励失败,打工开始时间" << pSpace->dwStartTime << "大于活动" << pCFG->dwActID << "时间" << dwEndTime << XEND;
      return EERROR_FAIL;
    }
  }

  DWORD dwEnableCount = 0;
  DWORD dwFrequency = 0;
  DWORD dwMaxEnable = 0;
  if (calcRewardCount(*pSpace, dwNow, dwEnableCount, dwFrequency) == false)
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,奖励计算结果为0" << XEND;
    return EERROR_FAIL;
  }
  if (calcRewardCount(*pSpace, getCFGDestTime(*pSpace), dwMaxEnable, dwFrequency) == false)
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,计算最大可领取奖励失败" << XEND;
    return EERROR_FAIL;
  }

  if (pSpace->vecCounts.size() < pCFG->vecRewardIDs.size())
    pSpace->vecCounts.resize(pCFG->vecRewardIDs.size());
  if (pSpace->vecLastCounts.size() < pCFG->vecRewardIDs.size())
    pSpace->vecLastCounts.resize(pCFG->vecRewardIDs.size());

  TVecItemInfo vecDatas;
  DWORD dwIndex = 0;
  DWORD dwRealCount = 0;
  srand(pSpace->dwStartTime * ONE_THOUSAND);
  for (size_t i = 0; i < pCFG->vecRewardIDs.size(); ++i)
  {
    if (pSpace->vecCounts[i] >= dwEnableCount)
    {
      XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "领取场所" << cmd.id() << "的奖励" << pCFG->vecRewardIDs[i] << "已领取" << pSpace->vecCounts[i] << "次,超过可领取次数" << dwEnableCount << XEND;
      continue;
    }

    dwRealCount = dwEnableCount - pSpace->vecCounts[i];
    if (dwRealCount == 0)
    {
      XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "领取场所" << cmd.id() << "的奖励" << pCFG->vecRewardIDs[i] << "已领取" << pSpace->vecCounts[i] << "可领取" << dwEnableCount << "实际领取为0" << XEND;
      continue;
    }

    dwIndex = i;
    for (DWORD d = 0; d < dwRealCount; ++d)
    {
      TVecItemInfo vecItems;
      if (RewardManager::roll(pCFG->vecRewardIDs[i], m_pUser, vecItems, ESOURCE_PET_WORK) == false)
      {
        XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,rewardid :" << pCFG->vecRewardIDs[i] << "随机失败" << XEND;
        return EERROR_FAIL;
      }
      combinItemInfo(vecDatas, vecItems);
    }
  }
  srand(xTime::getCurUSec());

  if (vecDatas.empty() == true)
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,没有随机到任何奖励" << XEND;
    return EERROR_SUCCESS;
  }

  TVecItemInfo vecCopy(vecDatas);
  if (eMethod == EWORKREWARDMETHOD_PACK)
  {
    if (vecDatas.empty() == false && m_pUser->getPackage().addItem(vecDatas, EPACKMETHOD_AVAILABLE) == false)
    {
      XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励失败,添加到包裹失败" << XEND;
      return EERROR_FAIL;
    }
  }
  else if (eMethod == EWORKREWARDMETHOD_MAIL)
  {
    TVecItemData vecMailDatas;
    for (auto &v : vecDatas)
    {
      ItemData oData;
      oData.mutable_base()->CopyFrom(v);
      combineItemData(vecMailDatas, oData);
    }
    if (vecMailDatas.empty() == false)
      MailManager::getMe().sendMail(m_pUser->id, 12114, vecMailDatas, MsgParams(), true, true, EMAILTYPE_NORMAL_NOTIME);
    XLOG << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励,奖励通过邮件给与,奖励为" << vecDatas << XEND;
  }
  else
  {
    XERR << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "领取场所" << cmd.id() << "的奖励,method" << eMethod << "非法" << XEND;
    return EERROR_FAIL;
  }

  pSpace->vecCounts[dwIndex] += dwRealCount;
  pSpace->eState = !pSpace->bUnlock ? EWORKSTATE_UNUSED : pSpace->eState;
  pSpace->dwLastRewardTime = dwNow;

  refreshLastCount(*pSpace, dwNow, dwEnableCount);
  if (pSpace->bUnlock)
  {
    DWORD dwCount = pSpace->vecCounts.empty() == true ? 0 : pSpace->vecCounts[0];
    DWORD dwLastCount = pSpace->vecLastCounts.empty() == true ? 0 : pSpace->vecLastCounts[0];
    DWORD dwCFGDestTime = getCFGDestTime(*pSpace);
    if (dwNow < dwCFGDestTime)
    {
      DWORD dwOffset = LuaManager::getMe().call<DWORD>("calcDuringTime", pSpace->dwStartTime, dwNow, dwFrequency, dwCount, pCFG->dwMaxReward, dwLastCount);
      XLOG << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "领取场所" << cmd.id() << "的奖励成功,流失时间计算 starttime :" << pSpace->dwStartTime << "now :" << dwNow << "frequency :" << dwFrequency << "count :" << dwCount << "lastcount :" << dwLastCount
        << "结果 :" << dwOffset << XEND;
      pSpace->dwStartTime = dwNow > dwOffset ? dwNow - dwOffset : dwNow;
    }
    else
    {
      pSpace->dwStartTime = dwNow;
    }

    pSpace->vecCounts.clear();
    pSpace->eState = EWORKSTATE_WORKING;
  }

  m_setUpdateIDs.insert(pSpace->dwID);
  update();
  m_pUser->getServant().onFinishEvent(ETRIGGER_PETWORK);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_PETWORK);

  XLOG << "[宠物打工-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "领取场所" << cmd.id() << "的奖励成功,该次总共领取" << dwRealCount << "次,最大可领取上限" << dwMaxEnable << "奖励方式 method :" << eMethod << "获得奖励" << vecCopy << XEND;
  return EERROR_SUCCESS;
}

void PetWork::testFrequency()
{
  //function PetFun.calcPetWorkFrequency(petid, petbaselv, petfriendlv, spaceid, skillid)
  const TMapPetWorkCFG& mapWorkCFG = PetConfig::getMe().getWorkCFGList();
  const TMapPetCFG& mapPetCFG = PetConfig::getMe().getPetCFGList();

  for (auto &space : mapWorkCFG)
  {
    for (auto &pet : mapPetCFG)
    {
      for (DWORD baselv = 1; baselv <= 200; ++baselv)
      {
        for (DWORD friendlv = 1; friendlv <= 10; ++friendlv)
        {
          for (DWORD skilllv = 1; skilllv <= 10; ++skilllv)
          {
            DWORD dwFrequency = LuaManager::getMe().call<DWORD>("calcPetWorkFrequency", pet.first, baselv, friendlv, space.second.dwID, pet.second.dwWorkSkillID * 1000 + skilllv);
            XDBG << "[宠物打工-频率测试]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
              << "petid :" << pet.first << "baselv :" << baselv << "friendlv :" << friendlv << "spaceid :" << space.second.dwID << "skillid :" << pet.second.dwWorkSkillID * 1000 + skilllv << "频率结果" << dwFrequency << XEND;
          }
        }
      }
    }
  }
}

SWorkSpace* PetWork::getSpaceData(DWORD dwID)
{
  auto m = m_mapWorkSpace.find(dwID);
  if (m != m_mapWorkSpace.end())
  {
    if (m->first == PET_WORK_SPACE_CAPRA)
      refreshCapra(m->second);
    return &m->second;
  }
  return nullptr;
}

bool PetWork::checkEnable(const SPetWorkCond& rCond, SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  if (rCond.eCond == EPETWORKCOND_MIN || rCond.eCond == EPETWORKCOND_MAX)
    return true;

  if (rCond.eCond == EPETWORKCOND_MENU)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    for (auto &v : rCond.vecParams)
    {
      if (pUser->getMenu().isOpen(v) == false)
        return false;
    }
    return true;
  }

  return false;
}

bool PetWork::isInMonthCard() const
{
  return xTime::getCurSec() < m_dwCardExpireTime;
}

bool PetWork::calcRewardCount(const SWorkSpace& rSpace, DWORD dwNow, DWORD& rCount, DWORD& rFrequency) const
{
  const SPetWorkCFG* pCFG = PetConfig::getMe().getWorkCFG(rSpace.dwID);
  if (pCFG == nullptr)
  {
    XERR << "[宠物打工-奖励计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "场所" << rSpace.dwID << "未在 Table_Pet_WorkSpace.txt 表中找到" << XEND;
    return false;
  }

  rFrequency = 0;
  for (auto &m : rSpace.mapEggs)
  {
    const EggData& rEgg = m.second.egg();
    const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(rEgg.id());
    if (pCFG == nullptr)
    {
      XERR << "[宠物打工-奖励计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "场所" << rSpace.dwID << "奖励计算失败,宠物" << rEgg.id() << "未在 Table_Pet.txt 表中找到" << XEND;
      return false;
    }

    DWORD dwWorkSkill = ItemEgg::getWorkSkillID(rEgg);
    DWORD dwFrequency = LuaManager::getMe().call<DWORD>("calcPetWorkFrequency", rEgg.id(), rEgg.lv(), rEgg.friendlv(), rSpace.dwID, dwWorkSkill);
    if (dwFrequency == 0)
    {
      XERR << "[宠物打工-奖励计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "场所" << rSpace.dwID << "计算奖励失败,宠物" << rEgg.id() << rEgg.lv() << rEgg.friendlv() << "计算频率结果为0" << XEND;
      return false;
    }
    XLOG << "[宠物打工-奖励计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "场所" << rSpace.dwID << "宠物" << rEgg.id() << rEgg.lv() << rEgg.friendlv() << dwWorkSkill << "频率结果" << dwFrequency << XEND;

    rFrequency += dwFrequency;
  }

  DWORD dwDest = getDestTime(rSpace, dwNow);
  DWORD dwLastCount = rSpace.vecLastCounts.empty() == true ? 0 : rSpace.vecLastCounts[0];
  rCount = LuaManager::getMe().call<DWORD>("calcPetWorkRewardCount", rSpace.dwStartTime, dwDest, rFrequency, pCFG->dwMaxReward, dwLastCount);

  XLOG << "[宠物打工-奖励计算]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "场所" << rSpace.dwID << "计算成功, start :" << rSpace.dwStartTime << "dest :" << dwDest << "frequency :" << rFrequency << "daymaxreward :" << pCFG->dwMaxReward << "lastcount :" << dwLastCount
    << "结果" << rCount << XEND;
  return true;
}

void PetWork::refreshCapra(SWorkSpace& rSpace)
{
  setCardExpireTime(m_pUser->getDeposit().getExpireTime(ETITLE_TYPE_MONTH));
  bool bUnlock = isInMonthCard();
  if (bUnlock != rSpace.bUnlock)
  {
    rSpace.bUnlock = bUnlock;
    if (rSpace.mapEggs.empty() == true)
      rSpace.eState = bUnlock ? EWORKSTATE_UNUSED : EWORKSTATE_MIN;
  }
}

void PetWork::refreshActivity(SWorkSpace& rSpace, bool bStop /*= true*/)
{
  if (rSpace.pCFG == nullptr || rSpace.pCFG->dwActID == 0)
    return;

  const SPetWorkCFG* pCFG = rSpace.pCFG;
  rSpace.bUnlock = ActivityManager::getMe().isOpen(pCFG->dwActID) == true && now() <= MiscConfig::getMe().getActCFG().getPetWorkEndTime(rSpace.pCFG->dwActID);
  m_setUpdateIDs.erase(pCFG->dwID);
  if (rSpace.mapEggs.empty() == true)
    rSpace.eState = rSpace.bUnlock ? EWORKSTATE_UNUSED : EWORKSTATE_MIN;
  if (bStop && !rSpace.bUnlock && rSpace.mapEggs.empty() == false)
  {
    Cmd::StopWorkPetCmd cmd;
    cmd.set_id(rSpace.dwID);
    stopWork(cmd, EWORKREWARDMETHOD_MAIL);
    XLOG << "[宠物打工-场所刷新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "场所" << cmd.id() << "活动结束,结束该场所打工" << XEND;
  }
}

void PetWork::refreshLastCount(SWorkSpace& rSpace, DWORD dwNow, DWORD dwEnableCount)
{
  const SPetWorkCFG* pCFG = PetConfig::getMe().getWorkCFG(rSpace.dwID);
  if (pCFG == nullptr)
  {
    XERR << "[宠物打工-奖励次数]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "计算奖励次数失败 id :" << rSpace.dwID << "未在 Table_Pet_WorkSpace.txt 表找到" << XEND;
    return;
  }

  DWORD dwDest = getCFGDestTime(rSpace);
  DWORD dwDestTime = getDestTime(rSpace, dwNow);
  if (dwNow < dwDest)
  {
    DWORD dwCount = 0;
    DWORD dwFrequency = 0;
    DWORD dwFiveNowTime = xTime::getDayStart(dwNow) + 5 * 3600;
    DWORD dwLastFiveNowTime = dwFiveNowTime - DAY_T;
    for (size_t i = 0; i < rSpace.vecCounts.size(); ++i)
    {
      if (dwDestTime <= dwFiveNowTime)
      {
        if (rSpace.dwStartTime <= dwLastFiveNowTime)
        {
          if (calcRewardCount(rSpace, dwLastFiveNowTime, dwCount, dwFrequency) == false)
            continue;
          DWORD dwMax = LuaManager::getMe().call<DWORD>("calcPetWorkMaxReward", rSpace.dwStartTime, dwLastFiveNowTime, pCFG->dwMaxReward);
          if (dwCount > dwMax)
            dwCount = dwMax;
          dwCount = dwEnableCount - dwCount;
        }
        else
        {
          dwCount = rSpace.vecCounts[i];
        }
      }
      else
      {
        if (rSpace.dwStartTime <= dwFiveNowTime)
        {
          if (calcRewardCount(rSpace, dwFiveNowTime, dwCount, dwFrequency) == false)
            continue;
          DWORD dwMax = LuaManager::getMe().call<DWORD>("calcPetWorkMaxReward", rSpace.dwStartTime, dwFiveNowTime, pCFG->dwMaxReward);
          if (dwCount > dwMax)
            dwCount = dwMax;
          dwCount = dwEnableCount - dwCount;
        }
        else
        {
          dwCount = rSpace.vecCounts[i];
        }
      }
    }
    DWORD dwTime = m_mapLastReward[pCFG->dwID];
    bool bAdd = dwNow < dwTime;
    for (size_t i = 0; i < pCFG->vecRewardIDs.size(); ++i)
    {
      if (bAdd)
        rSpace.vecLastCounts[i] += dwCount;
      else
        rSpace.vecLastCounts[i] = dwCount;
      XLOG << "[宠物打工-奖励次数]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "计算奖励次数为" << rSpace.vecLastCounts[i] << XEND;
    }
  }

  DWORD& dwTime = m_mapLastReward[pCFG->dwID];
  DWORD dwFiveTime = xTime::getDayStart(dwNow) + 5 * 3600;
  if (dwNow < dwFiveTime)
    dwTime = dwFiveTime;
  else
    dwTime = dwFiveTime + DAY_T;
}

DWORD PetWork::getCurWorkCount()
{
  DWORD dwCount = 0;
  for (auto &m : m_mapWorkSpace)
  {
    const SPetWorkCFG* pCFG = m.second.pCFG;
    if (pCFG == nullptr || pCFG->dwActID != 0)
      continue;
    if (m.second.eState != EWORKSTATE_MIN && m.second.eState != EWORKSTATE_UNUSED)
      ++dwCount;
  }
  return dwCount;
}

DWORD PetWork::getMaxWorkCount()
{
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();

  DWORD dwCount = rCFG.dwWorkMaxWorkCount;
  XDBG << "[宠物打工-场所上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "基础上限" << dwCount << "个" << XEND;

  if (isInMonthCard() == true)
  {
    dwCount = DepositConfig::getMe().doFunc(ETITLE_TYPE_MONTH, EFuncType_PETWORK, dwCount);
    XDBG << "[宠物打工-场所上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "月卡增加为" << dwCount << "个" << XEND;
  }

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr)
  {
    FighterSkill& rSkill = pFighter->getSkill();
    for (auto &m : rCFG.mapSkillWork)
    {
      if (rSkill.getSkillLv(m.first) != 0)
      {
        dwCount += m.second;
        XDBG << "[宠物打工-场所上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "冒险技能" << m.first << "增加为" << dwCount << "个" << XEND;
      }
    }
  }

  XDBG << "[宠物打工-场所上限]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "最大打工场所上限" << dwCount << "个" << XEND;
  return dwCount;
}

DWORD PetWork::getDestTime(const SWorkSpace& rSpace, DWORD dwNow) const
{
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD dwDest = 0;
  if (rSpace.dwID == PET_WORK_SPACE_CAPRA)
  {
    dwDest = m_dwCardExpireTime;
  }
  else if (rSpace.pCFG != nullptr && rSpace.pCFG->dwActID != 0)
  {
    dwDest = MiscConfig::getMe().getActCFG().getPetWorkEndTime(rSpace.pCFG->dwActID);
  }
  else
  {
    dwDest = rSpace.dwStartTime + rCFG.dwWorkMaxContinueDay * DAY_T;
  }

  if (dwNow > dwDest)
  {
    dwNow = dwDest;
    XDBG << "[宠物打工-终点时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "场所" << rSpace.dwID << "starttime" << rSpace.dwStartTime << "计算后, 终点时间" << dwDest << "超过当前时间,修正now为" << dwNow << XEND;
  }
  DWORD dwPassDay = (dwNow - rSpace.dwStartTime) / DAY_T;
  if (dwPassDay >= rCFG.dwWorkMaxContinueDay)
  {
    dwNow = rSpace.dwStartTime + rCFG.dwWorkMaxContinueDay * DAY_T;
    XDBG << "[宠物打工-终点时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "场所" << rSpace.dwID << "从starttime" << rSpace.dwStartTime << "开始通过天数" << dwPassDay << "超过" << rCFG.dwWorkMaxContinueDay << "天修正now为" << dwNow << XEND;
  }
  return dwNow;
}

DWORD PetWork::getCFGDestTime(const SWorkSpace& rSpace) const
{
  DWORD dwDest = rSpace.dwStartTime + MiscConfig::getMe().getPetCFG().dwWorkMaxContinueDay * DAY_T;
  if (rSpace.dwID == PET_WORK_SPACE_CAPRA)
  {
    if (m_dwCardExpireTime < dwDest)
      dwDest = m_dwCardExpireTime;
  }
  else if (rSpace.pCFG != nullptr && rSpace.pCFG->dwActID != 0)
  {
    DWORD dwEndTime = MiscConfig::getMe().getActCFG().getPetWorkEndTime(rSpace.pCFG->dwActID);
    if (dwEndTime < dwDest)
      dwDest = dwEndTime;
  }
  return dwDest;
}

const SWorkPetExtra* PetWork::getPetExchange(const std::string& guid)
{
  auto m = m_mapPetExtra.find(guid);
  if (m != m_mapPetExtra.end())
    return &m->second;
  return 0;
}

void PetWork::addPetExchangeCount(const std::string& guid, const SWorkSpace& rSpace)
{
  if (rSpace.pCFG == nullptr)
  {
    XERR << "[宠物打工-转换更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "宠物" << guid << "在场所" << rSpace.dwID << "新增转换次数失败,该场所未包含正确配置" << XEND;
    return;
  }
  if (rSpace.pCFG->dwActID != 0)
  {
    XLOG << "[宠物打工-转换更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "宠物" << guid << "在场所" << rSpace.dwID << "新增转换次数成功,活动场所不计数" << XEND;
    return;
  }
  SWorkPetExtra& rExtra = m_mapPetExtra[guid];
  rExtra.guid = guid;

  DWORD dwTemp = rExtra.dwLastSpaceID;
  rExtra.dwLastSpaceID = rSpace.dwID;
  if (dwTemp == 0 || dwTemp == rSpace.dwID)
    return;

  ++rExtra.dwCount;

  PetExtraUpdatePetCmd cmd;
  rExtra.toData(cmd.add_updates());
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[宠物打工-转换更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "宠物" << guid << "在场所" << rSpace.dwID << "新增转换次数成功,更新数据" << cmd.ShortDebugString() << XEND;
}

void PetWork::update()
{
  if (m_setUpdateIDs.empty() == true)
    return;

  WorkSpaceUpdate cmd;
  for (auto &s : m_setUpdateIDs)
  {
    SWorkSpace* pSpace = getSpaceData(s);
    if (pSpace != nullptr)
    {
      if (pSpace->dwID == PET_WORK_SPACE_CAPRA)
        refreshCapra(*pSpace);
      pSpace->toClient(cmd.add_updates());
    }
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[宠物打工-更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "更新数据" << cmd.ShortDebugString() << XEND;
  m_setUpdateIDs.clear();
}

void PetWork::resetPetExtra()
{
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_PETWORK_EXCHANGE) != 0)
    return;
  m_pUser->getVar().setAccVarValue(EACCVARTYPE_PETWORK_EXCHANGE, 1);

  m_mapPetExtra.clear();
  XLOG << "[宠物打工-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "打工额外数据被重置" << XEND;
}

