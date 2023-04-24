#include "PetAdventure.h"
#include "SceneUser.h"
#include "MsgManager.h"
#include "SceneManager.h"
#include "PlatLogManager.h"
#include "PetAdventure.h"
#include "SkillManager.h"

PetAdventure::PetAdventure(SceneUser* pUser) : m_pUser(pUser)
{

}

PetAdventure::~PetAdventure()
{

}

bool PetAdventure::toData(PetAdventureDBItem& rDBItem, const PetAdventureItem& rItem)
{
  rDBItem.set_id(rItem.id());
  rDBItem.set_starttime(rItem.starttime());
  rDBItem.set_status(rItem.status());
  rDBItem.set_specid(rItem.specid());

  for (int i = 0; i < rItem.eggs_size(); ++i)
    rDBItem.add_eggs()->CopyFrom(rItem.eggs(i));

  // 1226, step 移除, 换用rewardinfo
  //for (int i = 0; i < rItem.steps_size(); ++i)
    //rDBItem.add_steps()->CopyFrom(rItem.steps(i));

  for (int i = 0; i < rItem.raresreward_size(); ++i)
    rDBItem.add_raresreward()->CopyFrom(rItem.raresreward(i));

  for (int i = 0; i < rItem.eff_size(); ++i)
    rDBItem.add_eff()->CopyFrom(rItem.eff(i));

  for (int i = 0; i < rItem.rewardinfo_size(); ++i)
    rDBItem.add_rewardinfo()->CopyFrom(rItem.rewardinfo(i));

  for (int i = 0; i < rItem.extrarewardinfo_size(); ++i)
    rDBItem.add_extrarewardinfo()->CopyFrom(rItem.extrarewardinfo(i));
  return true;
}

bool PetAdventure::fromData(PetAdventureItem& rItem, const PetAdventureDBItem& rDBItem)
{
  rItem.set_id(rDBItem.id());
  rItem.set_starttime(rDBItem.starttime());
  rItem.set_status(rDBItem.status());
  rItem.set_specid(rDBItem.specid());

  for (int i = 0; i < rDBItem.eggs_size(); ++i)
    rItem.add_eggs()->CopyFrom(rDBItem.eggs(i));

  for (int i = 0; i < rDBItem.steps_size(); ++i)
    rItem.add_steps()->CopyFrom(rDBItem.steps(i));

  for (int i = 0; i < rDBItem.raresreward_size(); ++i)
    rItem.add_raresreward()->CopyFrom(rDBItem.raresreward(i));

  for (int i = 0; i < rDBItem.eff_size(); ++i)
    rItem.add_eff()->CopyFrom(rDBItem.eff(i));

  for (int i = 0; i < rDBItem.rewardinfo_size(); ++i)
    rItem.add_rewardinfo()->CopyFrom(rDBItem.rewardinfo(i));

  for (int i = 0; i < rDBItem.extrarewardinfo_size(); ++i)
    rItem.add_extrarewardinfo()->CopyFrom(rDBItem.extrarewardinfo(i));

  return true;
}

bool PetAdventure::calcPreview(SceneUser* pUser, PetAdventureItem& rItem, const SPetAdventureCFG* pCFG)
{
  if (pUser == nullptr || pCFG == nullptr)
    return false;

  if (pCFG->dwRareReward != 0)
  {
    TVecItemData vec;
    const SRewardCFG* pRewardCFG = RewardConfig::getMe().getRewardCFG(pCFG->dwRareReward);
    if (pRewardCFG != nullptr)
    {
      for (auto &v : pRewardCFG->vecItems)
      {
        for (auto &item : v.vecItems)
        {
          ItemData oData;
          oData.mutable_base()->CopyFrom(item.oItem);
          oData.mutable_base()->set_count(1.0f * v.dwRate / BASE_RATE * item.oItem.count() * ONE_THOUSAND);
          combineItemData(vec, oData);
        }
      }
      for (auto &v : vec)
        rItem.add_raresreward()->CopyFrom(v);
    }
    else
    {
      XERR << "[宠物冒险-奖励预览]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "id :" << pCFG->dwID << "计算成功,随机稀有奖励" << pCFG->dwRareReward << "奖励失败" << XEND;
    }
  }

  // 计算配置表直接配置的奖励
  auto calcNormalReward = [&](TVecItemData& rewards)
  {
    for (auto &s : pCFG->setRewardIDs)
    {
      const SRewardCFG* pRewardCFG = RewardConfig::getMe().getRewardCFG(s);
      if (pRewardCFG != nullptr)
      {
        for (auto &v : pRewardCFG->vecItems)
        {
          for (auto &item : v.vecItems)
          {
            ItemData oData;
            oData.mutable_base()->CopyFrom(item.oItem);
            oData.mutable_base()->set_count(1.0f * v.dwRate / BASE_RATE * item.oItem.count() * ONE_THOUSAND);
            combineItemData(rewards, oData);
          }
        }
      }
      else
      {
        XERR << "[宠物冒险-奖励预览]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "id :" << pCFG->dwID << "计算成功,随机固有奖励" << s << "奖励失败" << XEND;
      }
    }
  };

  // 计算monster对应的奖励
  auto calcMonsterReward = [&](const TVecDWORD& monsters, DWORD kind, TVecItemData& rewards)
  {
    if (kind == 0) return;
    for (auto &v : monsters)
    {
      const SNpcCFG* pNpcCFG = NpcConfig::getMe().getNpcCFG(v);
      if (pNpcCFG == nullptr)
      {
        XERR << "[宠物冒险-奖励计算]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "id :" << pCFG->dwID << "计算成功,随机Monster" << v << "奖励失败,未在 Table_Monster.txt 表中找到" << XEND;
        continue;
      }
      for (auto &v : pNpcCFG->vecRewardIDs)
      {
        const SRewardCFG* pRewardCFG = RewardConfig::getMe().getRewardCFG(v);
        if (pRewardCFG != nullptr)
        {
          for (auto &v : pRewardCFG->vecItems)
          {
            for (auto &item : v.vecItems)
            {
              ItemData oData;
              oData.mutable_base()->CopyFrom(item.oItem);
              oData.mutable_base()->set_count(1.0f * v.dwRate / BASE_RATE * item.oItem.count() * ONE_THOUSAND * pCFG->dwTime / kind);
              combineItemData(rewards, oData);
            }
          }
        }
        else
        {
          XERR << "[宠物冒险-奖励计算]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "id :" << pCFG->dwID << "计算成功,随机Monster" << v << "奖励" << v << "失败" << XEND;
        }
      }
      if (pNpcCFG->qwBaseExp != 0)
      {
        ItemData oData;
        oData.mutable_base()->set_id(ITEM_BASEEXP);
        oData.mutable_base()->set_count(1.0f * pNpcCFG->qwBaseExp * ONE_THOUSAND * pCFG->dwTime / kind);
        combineItemData(rewards, oData);
      }
      if (pNpcCFG->qwJobExp != 0)
      {
        ItemData oData;
        oData.mutable_base()->set_id(ITEM_JOBEXP);
        oData.mutable_base()->set_count(1.0f * pNpcCFG->qwJobExp * ONE_THOUSAND * pCFG->dwTime / kind);
        combineItemData(rewards, oData);
      }
    }
  };

  TVecDWORD vecMonsterIDs;
  DWORD dwKind = 0;
  TVecItemData vecReward;

  collectMonsterID(pCFG, vecMonsterIDs, dwKind, rItem.specid());
  dwKind = dwKind == 0 ? 1 : dwKind;

  PetMonsterRewardInfo* pInfo = rItem.add_rewardinfo();
  pInfo->set_monsterid(rItem.specid());

  calcNormalReward(vecReward);
  calcMonsterReward(vecMonsterIDs, dwKind, vecReward);

  for (auto &v : vecReward)
    pInfo->add_items()->CopyFrom(v);

  // 计算所有怪物预览(总的和单个怪物)
  if (rItem.specid() == 0)
  {
    for (auto &v : pCFG->vecMonsterIDs)
    {
      vecMonsterIDs.clear();
      vecReward.clear();
      collectMonsterID(pCFG, vecMonsterIDs, dwKind, v);

      PetMonsterRewardInfo* pInfo = rItem.add_rewardinfo();
      pInfo->set_monsterid(v);
      calcMonsterReward(vecMonsterIDs, dwKind, vecReward);

      for (auto &v : vecReward)
        pInfo->add_items()->CopyFrom(v);
    }
  }

  return true;
}

bool PetAdventure::calcReward(SceneUser* pUser, PetAdventureItem& rItem, const SPetAdventureCFG* pCFG, DWORD dwRareCount, float fEffectValue, const OtherFactor& oFactor, float fRatio /*= 1.0f */)
{
  if (pUser == nullptr || pCFG == nullptr)
    return false;

  DWORD dwKind = 0;
  TVecDWORD vecMonsterIDs;
  collectMonsterID(pCFG, vecMonsterIDs, dwKind, rItem.specid());

  DWORD dwRealTime = pCFG->dwTime * fEffectValue;
  if (dwRealTime == 0)
  {
    XERR << "[宠物冒险-奖励计算]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "id :" << pCFG->dwID << "频率为" << dwRealTime << "次" << XEND;
    dwRealTime = 1;
  }

  TVecItemInfo vecReward;
  for (DWORD d = 0; d < dwRealTime; ++d)
  {
    if (d == 0)
    {
      if (pCFG->dwRareReward != 0 && dwRareCount != 0)
      {
        TVecItemInfo vec;
        if (RewardManager::roll_adventure(pCFG->dwRareReward, pUser, vec, ESOURCE_PET_ADVENTURE, oFactor) == true)
        {
          for (auto &v : vec)
            v.set_count(v.count() * dwRareCount);
          for (auto &v : vec)
          {
            ItemData oData;
            oData.mutable_base()->CopyFrom(v);
            rItem.add_raresreward()->CopyFrom(oData);
          }
        }
        else
        {
          XERR << "[宠物冒险-奖励计算]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "id :" << pCFG->dwID << "计算成功,随机稀有奖励" << pCFG->dwRareReward << "奖励失败" << XEND;
        }
      }
      for (auto &s : pCFG->setRewardIDs)
      {
        TVecItemInfo vec;
        if (RewardManager::roll_adventure(s, pUser, vec, ESOURCE_PET_ADVENTURE, oFactor) == true)
        {
          combinItemInfo(vecReward, vec);
        }
        else
        {
          XERR << "[宠物冒险-奖励计算]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "id :" << pCFG->dwID << "计算成功,随机固有奖励" << s << "奖励失败" << XEND;
        }
      }
    }

    if (vecMonsterIDs.empty() == false)
    {
      DWORD dwMonsterID = vecMonsterIDs[d % vecMonsterIDs.size()];
      const SNpcCFG* pNpcCFG = NpcConfig::getMe().getNpcCFG(dwMonsterID);
      if (pNpcCFG == nullptr)
      {
        XERR << "[宠物冒险-奖励计算]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "id :" << pCFG->dwID << "计算成功,随机Monster" << dwMonsterID << "奖励失败,未在 Table_Monster.txt 表中找到" << XEND;
        continue;
      }
      for (auto &v : pNpcCFG->vecRewardIDs)
      {
        TVecItemInfo vec;
        if (RewardManager::roll_adventure(v, pUser, vec, ESOURCE_PET_ADVENTURE, oFactor) == false)
        {
          XERR << "[宠物冒险-奖励计算]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "id :" << pCFG->dwID << "计算成功,随机Monster" << dwMonsterID << "奖励" << v << "失败" << XEND;
          continue;
        }
        combinItemInfo(vecReward, vec);
      }
      if (pNpcCFG->qwBaseExp != 0)
      {
        ItemInfo oItem;
        oItem.set_id(ITEM_BASEEXP);
        oItem.set_count(pNpcCFG->qwBaseExp);
        combinItemInfo(vecReward, oItem);
      }
      if (pNpcCFG->qwJobExp != 0)
      {
        ItemInfo oItem;
        oItem.set_id(ITEM_JOBEXP);
        oItem.set_count(pNpcCFG->qwJobExp);
        combinItemInfo(vecReward, oItem);
      }
    }

    /*for (auto &v : vecReward)
    {
      ItemData oData;
      oData.mutable_base()->CopyFrom(v);
      oData.mutable_base()->set_count(oData.base().count());
      pStep->add_items()->CopyFrom(oData);
    }
    */
  }


  PetMonsterRewardInfo* pReward = rItem.add_rewardinfo();
  pReward->set_monsterid(rItem.specid());
  for (auto &v : vecReward)
  {
    ItemData oData;
    oData.mutable_base()->CopyFrom(v);
    pReward->add_items()->CopyFrom(oData);
  }

  return true;
}

void PetAdventure::calcBuffReward(const SPetAdventureCFG* pCFG, PetAdventureItem& rItem)
{
  const float precision = 0.001f;
  // 计算冒险Buffer增加额外奖励
  if (rItem.rewardinfo_size() <= 0)
    return;
  PetMonsterRewardInfo* pReward = rItem.mutable_rewardinfo(0);
  if (!pReward)
    return;
  XDBG << "冒险Buff生效前的奖励: " << XEND;
  auto debugLogFunc = [&](){
    for (auto i = 0; i < pReward->items_size(); ++i)
    {
      ItemData* pData = pReward->mutable_items(i);
      if (!pData)
        continue;
      XDBG <<"Id = " << pData->base().id() << "Count = " << pData->base().count() << XEND;
    }
    XDBG << "额外奖励:" << XEND;
    for (auto i = 0; i < rItem.extrarewardinfo_size(); ++i) 
    {
      XDBG <<"Id = "<< rItem.extrarewardinfo(i).base().id() << "Count = " <<  rItem.extrarewardinfo(i).base().count() << XEND;
    }
  };
  debugLogFunc();

  for (auto i = 0; i < pReward->items_size(); ++i)
  {
    //Base Exp 
    ItemData* pData = pReward->mutable_items(i);
    if (!pData)
      continue;
    DWORD itemID = pData->base().id();
    if (ITEM_BASEEXP == itemID)
    {
      DWORD oldCount  = pData->base().count();
      DWORD extraCount = 0;
      for(auto& vecBuffs : m_vecPetAdventureBuff)
      {
        if (vecBuffs.empty())
          continue;
        for (auto ptrBuff : vecBuffs)
        {
          if (!ptrBuff)
            continue;
          if (ptrBuff->getBase() > precision)
            extraCount += (DWORD)(ptrBuff->getBase() * oldCount);
        }
      }
      if (extraCount > 0)
      {
        auto pItemData = rItem.add_extrarewardinfo();
        pItemData->mutable_base()->set_id(itemID);
        pItemData->mutable_base()->set_count(extraCount);
        pItemData->mutable_base()->set_source(ESOURCE_PET_ADVENTURE);
      }
    }
    //Job Exp 
    else if (ITEM_JOBEXP == itemID)
    {
      DWORD oldCount  = pData->base().count();
      DWORD extraCount = 0;
      for(auto& vecBuffs : m_vecPetAdventureBuff)
      {
        if (vecBuffs.empty())
          continue;
        for (auto& ptrBuff : vecBuffs)
        {
          if (!ptrBuff)
            continue;
          if (ptrBuff->getJob() > precision)
            extraCount += (DWORD)(ptrBuff->getJob() * oldCount);
        }
      }
      if (extraCount > 0)
      {
        auto pItemData = rItem.add_extrarewardinfo();
        pItemData->mutable_base()->set_id(itemID);
        pItemData->mutable_base()->set_count(extraCount);
        pItemData->mutable_base()->set_source(ESOURCE_PET_ADVENTURE);
      }
    }
    //素材
    else if ( itemID >= 52101 &&  itemID <= 52410 )
    {
      const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(itemID);
      if (!pCFG)
        continue;
      //DWORD extraCount = 0;
      float totalRatio = 0.0f;
      DWORD totalNum = 0;
      for (auto& vecBuffs : m_vecPetAdventureBuff)
      {
        if (vecBuffs.empty())
          continue;
        for (auto& ptrBuff : vecBuffs)
        {
          if (!ptrBuff)
            continue;
          float ratio = ptrBuff->getRatioByQualify(pCFG->eQualityType);
          totalRatio += ratio;
          DWORD num = ptrBuff->getNumByQualify(pCFG->eQualityType);
          totalNum += num;
        }
      }
      //if ((DWORD)randBetween(1,100) < totalRatio * 100) 
      if (totalNum != 0)
      {
        //extraCount = totalNum;
        auto pItemData = rItem.add_extrarewardinfo();
        pItemData->mutable_base()->set_id(itemID);
        pItemData->mutable_base()->set_count(totalNum);
        pItemData->mutable_base()->set_source(ESOURCE_PET_ADVENTURE);
        XLOG << "[宠物冒险-buff奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获得额外奖励" << pItemData->ShortDebugString() << XEND;
      }
    }
    /* 这里需要等策划来确认一下
    else  //卡片
    {
      const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(itemID);
      if (!pCFG)
        continue;
      if (ItemConfig::getMe().isCard(pCFG->eItemType))
      {
        if ((DWORD)randBetween(1,100) < getCardRatio()* 100)
        {
          auto pItemData = rItem.add_extrarewardinfo();
          pItemData->mutable_base()->set_id(itemID);
          pItemData->mutable_base()->set_count(1);
          pItemData->mutable_base()->set_source(ESOURCE_PET_ADVENTURE);
        }
      }
    }
    */ 
  }
  // 计算稀有箱子的奖励
  if (pCFG != nullptr && pCFG->dwRareReward != 0)
  {
    if ((DWORD)randBetween(1,10000) < getRareBoxRatio())
    {
      TVecItemInfo vec;
      if (RewardManager::roll(pCFG->dwRareReward, nullptr, vec, ESOURCE_PET_ADVENTURE) == true)
      {
        for (auto &v : vec)
          rItem.add_extrarewardinfo()->mutable_base()->CopyFrom(v);
      }
    }
  }
  XDBG << "冒险Buff生效后的奖励: " << XEND;
  debugLogFunc();
}

bool PetAdventure::collectMonsterID(const SPetAdventureCFG* pCFG, TVecDWORD& vecResult, DWORD& dwKind, DWORD specMonsterID)
{
  if (pCFG == nullptr)
    return false;

  if (pCFG->vecMonsterIDs.empty() == true)
    return false;

  if (specMonsterID)
  {
    auto it = find(pCFG->vecMonsterIDs.begin(), pCFG->vecMonsterIDs.end(), specMonsterID);
    if (it != pCFG->vecMonsterIDs.end())
      vecResult.push_back(specMonsterID);
  }
  if (vecResult.empty())
    vecResult.insert(vecResult.end(), pCFG->vecMonsterIDs.begin(), pCFG->vecMonsterIDs.end());

  TSetDWORD setKind;
  for (auto &v : pCFG->vecMonsterIDs)
    setKind.insert(v);
  dwKind = setKind.size();

  XDBG << "[宠物冒险-奖励计算] monster从配置采集id";
  for (auto &v : vecResult)
    XDBG << v;
  XDBG << "种类 :" << dwKind << "个" << XEND;
  return true;
}

bool PetAdventure::load(const BlobPetAdventure& rBlob)
{
  m_setUnlockArea.clear();
  m_mapIDAdventure.clear();
  for (int i = 0; i < rBlob.items_size(); ++i)
  {
    const PetAdventureDBItem& rDBItem = rBlob.items(i);
    PetAdventureItem oItem;
    if (fromData(oItem, rDBItem) == false)
      continue;
    m_mapIDAdventure[oItem.id()].CopyFrom(oItem);

    const SPetAdventureCFG* pCFG = PetConfig::getMe().getAdventureCFG(rDBItem.id());
    if (pCFG != nullptr && pCFG->dwUnlockArea != 0)
      m_setUnlockArea.insert(pCFG->dwUnlockArea);
  }

  for (int i = 0; i < rBlob.unlockarea_size(); ++i)
    m_setUnlockArea.insert(rBlob.unlockarea(i));

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  for (auto &s : rCFG.setDefaultAdventureArea)
    m_setUnlockArea.insert(s);

  m_dwVersion = rBlob.version();
  for (DWORD i = m_dwVersion; i < MAX_VERSION; ++i)
  {
    if (i == REMOVE_STEP_VERSION)
      patch_1();
  }
  m_dwVersion = MAX_VERSION;

  return true;
}

void PetAdventure::patch_1()
{
  for (auto &m : m_mapIDAdventure)
  {
    PetMonsterRewardInfo* pReward = m.second.add_rewardinfo();
    if (pReward == nullptr)
      continue;
    pReward->set_monsterid(m.second.specid());

    TVecItemInfo vecItem;
    for (int i = 0; i < m.second.steps_size(); ++i)
    {
      const PetAdventureStep& rStep = m.second.steps(i);
      for (int j = 0; j < rStep.items_size(); ++j)
      {
        combinItemInfo(vecItem, rStep.items(j).base());
      }
    }
    for (auto &v : vecItem)
    {
      ItemData oData;
      oData.mutable_base()->CopyFrom(v);
      pReward->add_items()->CopyFrom(oData);
      XLOG << "[宠物冒险-Path1], step转化奖励, 玩家:" << m_pUser->name << m_pUser->id << "id" << m.first << "奖励:" << v.id() << v.count() << XEND;
    }
  }

  for (auto &m : m_mapIDAdventure)
  {
    std::vector<PetMonsterRewardInfo> validinfo;
    TSetDWORD ids;
    for (int i = 0; i < m.second.rewardinfo_size(); ++i)
    {
      if (ids.find(m.second.rewardinfo(i).monsterid()) == ids.end())
      {
        PetMonsterRewardInfo info;
        info.CopyFrom(m.second.rewardinfo(i));
        validinfo.push_back(info);
        ids.insert(m.second.rewardinfo(i).monsterid());
        continue;
      }
    }
    m.second.clear_rewardinfo();
    for (auto &v : validinfo)
    {
      PetMonsterRewardInfo* pReward = m.second.add_rewardinfo();
      if (pReward == nullptr)
        continue;
      pReward->CopyFrom(v);
    }
  }
}

bool PetAdventure::save(BlobPetAdventure* pBlob)
{
  if (pBlob == nullptr)
    return false;

  pBlob->clear_items();
  for (auto &m : m_mapIDAdventure)
  {
    PetAdventureDBItem oDBItem;
    if (toData(oDBItem, m.second) == false)
    {
      XERR << "[宠物冒险-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "初始化" << m.second.ShortDebugString() << "失败" << XEND;
      continue;
    }
    pBlob->add_items()->CopyFrom(oDBItem);
  }

  pBlob->clear_unlockarea();
  for (auto &s : m_setUnlockArea)
    pBlob->add_unlockarea(s);

  pBlob->set_version(m_dwVersion);

  XDBG << "[宠物冒险-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << pBlob->ByteSize() << XEND;
  return true;
}

bool PetAdventure::isComplete(DWORD dwID) const
{
  auto m = m_mapIDAdventure.find(dwID);
  if (m == m_mapIDAdventure.end())
    return false;
  return m->second.status() == EPETADVENTURESTATUS_SUBMIT;
}

bool PetAdventure::hasPet(DWORD dwPetID, DWORD dwBaseLv, DWORD dwFriendLv) const
{
  for (auto &m : m_mapIDAdventure)
  {
    if (m.second.status() == EPETADVENTURESTATUS_SUBMIT)
      continue;
    for (int i = 0; i < m.second.eggs_size(); ++i)
    {
      const ItemData& rEgg = m.second.eggs(i);
      if (dwPetID != 0 && dwPetID != rEgg.egg().id())
        continue;
      if (rEgg.egg().lv() >= dwBaseLv && rEgg.egg().friendlv() >= dwFriendLv)
        return true;
    }
  }

  return false;
}

void PetAdventure::sendAdventureList()
{
  DWORD dwMaxArea = 0;
  for (auto &s : m_setUnlockArea)
  {
    if (s > dwMaxArea)
      dwMaxArea = s;
  }

  TVecPetAdventureCFG vecCFG;
  if (PetConfig::getMe().collectEnableAdventure(vecCFG, dwMaxArea) == false)
  {
    XERR << "[宠物冒险-列表同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步数据失败,随机列表失败" << XEND;
    return;
  }

  QueryPetAdventureListPetCmd cmd;
  DWORD dwNow = xTime::getCurSec();
  for (auto &v : vecCFG)
  {
    auto item = m_mapIDAdventure.find(v.dwID);
    if (item != m_mapIDAdventure.end())
    {
      const SPetAdventureCFG* pCFG = PetConfig::getMe().getAdventureCFG(v.dwID);
      if (pCFG != nullptr)
      {
        if (item->second.status() == EPETADVENTURESTATUS_SUBMIT)
          continue;

        PetAdventureItem* pItem = cmd.add_items();
        pItem->CopyFrom(item->second);
        if (dwNow > item->second.starttime() + pCFG->dwNeedTime)
        {
          pItem->set_status(EPETADVENTURESTATUS_COMPLETE);
          m_pUser->getTip().addRedTip(EREDSYS_PET_ADVENTURE);
        }
      }
      continue;
    }

    PetAdventureItem* pItem = cmd.add_items();
    pItem->set_id(v.dwID);
    pItem->set_status(EPETADVENTURESTATUS_CANACCEPT);
    PetAdventure::calcPreview(m_pUser, *pItem, &v);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[宠物冒险-列表同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步数据" << cmd.items_size() << "个任务,数据大小" << cmd.ByteSize() << XEND;
}

bool PetAdventure::startAdventure(const StartAdventurePetCmd& cmd)
{
  const SPetAdventureCFG* pCFG = PetConfig::getMe().getAdventureCFG(cmd.id());
  if (pCFG == nullptr)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:未找到该冒险配置");
    XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "未在 Table_PetAdventure.txt 表中找到" << XEND;
    return false;
  }

  auto m = m_mapIDAdventure.find(cmd.id());
  if (m != m_mapIDAdventure.end())
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:该冒险任务已经在冒险了");
    XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "已经在冒险中了" << XEND;
    return false;
  }

  UserSceneData& rData = m_pUser->getUserSceneData();
  if (pCFG->dwBattleTimeReq != 0)
  {
    DWORD dwNormalTime = rData.getTotalBattleTime() < rData.getBattleTime() ? 0 : rData.getTotalBattleTime() - rData.getBattleTime();
    DWORD dwMusicTime = rData.getReBattleTime() < rData.getUsedBattleTime() ? 0 : rData.getReBattleTime() - rData.getUsedBattleTime();
    if (dwNormalTime + dwMusicTime < pCFG->dwBattleTimeReq)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PET_ADVENTURE_BATTLETIME_ERROR);
      return false;
    }
  }

  if (pCFG->dwAreaReq != 0 && isAreaUnlock(pCFG->dwAreaReq) == false)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:区域未解锁");
    XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "冒险失败,需要解锁" << pCFG->dwAreaReq << "区域" << XEND;
    return false;
  }

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  DWORD dwCount = 0;
  for (auto &m : m_mapIDAdventure)
  {
    if (m.second.status() == EPETADVENTURESTATUS_COMPLETE || m.second.status() == EPETADVENTURESTATUS_ACCEPT)
      ++dwCount;
  }
  if (dwCount >= rCFG.dwPetAdventureMaxCount)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:超过最大冒险数量");
    XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "id :" << cmd.id() << "冒险失败,超过最大冒险数" << rCFG.dwPetAdventureMaxCount << XEND;
    return false;
  }

  TSetString setPetIDs;
  for (int i = 0; i < cmd.petids_size(); ++i)
  {
    const string& petid = cmd.petids(i);
    if (petid == "0")
      continue;

    auto s = setPetIDs.find(petid);
    if (s != setPetIDs.end())
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物id重复了");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "冒险失败,宠物" << petid << "重复了" << XEND;
      return false;
    }
    setPetIDs.insert(cmd.petids(i));
  }
  if (setPetIDs.empty() == true)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:没有派遣宠物");
    XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "冒险失败,未派遣宠物" << XEND;
    return false;
  }

  Package& rPack = m_pUser->getPackage();
  if (pCFG->vecItemReq.empty() == false)
  {
    if (setPetIDs.size() > pCFG->vecItemReq.size())
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:派遣宠物大于冒险消耗数量");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,派遣宠物" << setPetIDs.size() << "大于任务消耗数量" << XEND;
      return false;
    }
    const ItemInfo& rItemReq = pCFG->vecItemReq[setPetIDs.size() - 1];
    if (rPack.checkItemCount(rItemReq, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PETWORK) == false)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:材料不足");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,需要材料" << rItemReq.ShortDebugString() << XEND;
      return false;
    }
  }

  if (setPetIDs.size() > pCFG->dwMaxPet)
  {
    MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物任务派遣宠物超过最大数量");
    XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "id :" << cmd.id() << "冒险失败,最多只能派遣" << pCFG->dwMaxPet << "只宠物" << XEND;
    return false;
  }

  struct SPetParam
  {
    DWORD dwBLvDelta = 0;
    DWORD dwFLv = 0;
    float fParam = 0.0f;
    DWORD dwPetLv = 0;
  };
  vector<SPetParam> vecParams;
  TSetDWORD setEnableCond;

  m_vecPetAdventureBuff.clear();
  stringstream ssNames;
  for (int i = 0; i < cmd.petids_size(); ++i)
  {
    const string& s = cmd.petids(i);
    if (s == "0")
      continue;

    ItemEgg* pEgg = dynamic_cast<ItemEgg*>(rPack.getItem(s));
    if (pEgg == nullptr)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物不存在");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "冒险失败,宠物" << s << "不存在" << XEND;
      return false;
    }
    if (pEgg->getName().empty() == true)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物未孵化");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "冒险失败,宠物" << s << "未孵化过" << XEND;
      return false;
    }
    if (pEgg->getLevel() < pCFG->dwReqLv)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物等级不足");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,宠物" << s << "等级" << pEgg->getLevel() << "不足" << pCFG->dwReqLv << XEND;
      return false;
    }
    if (pEgg->getFriendLv() < rCFG.dwPetAdventureFriendReq)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物冒险等级不足");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,宠物" << s << "好感度等级" << pEgg->getFriendLv() << "不足" << rCFG.dwPetAdventureFriendReq << XEND;
      return false;
    }
    const SPetFriendLvCFG* pFriendCFG = PetConfig::getMe().getFriendLvCFG(pEgg->getPetID(), pEgg->getFriendLv());
    if (pFriendCFG == nullptr || !pFriendCFG->bCanAdventure)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:该宠物未达到冒险要求");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,宠物" << s << "好感度等级" << pEgg->getFriendLv() << "无法出战" << XEND;
      return false;
    }
    const SNpcCFG* pNpcCFG = NpcConfig::getMe().getNpcCFG(pEgg->getPetID());
    if (pNpcCFG == nullptr)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物NPC配置未找到");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,宠物" << pEgg->getPetID() << "计算稀有奖励失败,未在 Table_Monster.txt 表中找到" << XEND;
      return false;
    }
    for (auto &s : pCFG->setCondIDs)
    {
      const SPetAdventureCondCFG* pCondCFG = PetConfig::getMe().getAdventureCondCFG(s);
      if (pCondCFG == nullptr)
      {
        MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物NPC条件配置未找到");
        XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "id :" << cmd.id() << "冒险失败,宠物" << pEgg->getPetID() << s << "未在 Table_Pet_AdventureCond.txt 表中找到" << XEND;
        return false;
      }
      if (pCondCFG->checkCond(pEgg->getEggData()) == true)
        setEnableCond.insert(s);
    }
    const SPetCFG* pPetCFG = PetConfig::getMe().getPetCFG(pEgg->getPetID());
    if (pPetCFG == nullptr)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:宠物配置未找到");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,宠物" << pEgg->getPetID() << "计算稀有奖励失败,未在 Table_Pet.txt 表中找到" << XEND;
      return false;
    }
    ssNames << pEgg->getName() << ";";

    SPetParam stParam;
    stParam.dwBLvDelta = pEgg->getLevel() - pCFG->dwReqLv;
    stParam.dwFLv = pEgg->getFriendLv();
    stParam.fParam = pPetCFG->getAreaAdventureValue(pCFG->dwAreaReq);
    stParam.dwPetLv = pEgg->getLevel();
    vecParams.push_back(stParam);

    checkAdventureBuffs(pEgg);

  }
  float fEffectValue = 0.0f;
  PetAdventureItem& rItem = m_mapIDAdventure[pCFG->dwID];
  if (pCFG->eType == EPETADVENTURETYPE_ONCE)
  {
    fEffectValue = 1.0f;
  }
  else
  {
    for (auto &v : vecParams)
      fEffectValue += LuaManager::getMe().call<float>("calcPetAdventureValue", v.dwBLvDelta, v.dwFLv, v.fParam, pCFG->dwMaxPet, v.dwPetLv);
    for (int i = EPETEFFICIENCY_MIN + 1; i < EPETEFFICIENCY_MAX; ++i)
    {
      if (EPetEfficiencyType_IsValid(i) == false)
        continue;
      EPetEfficiencyType e = static_cast<EPetEfficiencyType>(i);
      float score = calcScore(e);
      fEffectValue += score;
      PetEfficiencyInfo* pInfo = rItem.add_eff();
      if (pInfo)
      {
        pInfo->set_etype(e);
        pInfo->set_fvalue(score * ONE_THOUSAND);
      }
    }
    if (fEffectValue >= -0.0001f && fEffectValue <= 0.0001f)
    {
      MsgManager::sendDebugMsg(m_pUser->id, "测试log:效率值结果为0");
      XERR << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "id :" << cmd.id() << "冒险失败,效率值结果为0" << XEND;
    }
  }

  if (pCFG->vecItemReq.empty() == false)
  {
    const ItemInfo& rItemReq = pCFG->vecItemReq[setPetIDs.size() - 1];
    //宠物冒险Buff有几率不消耗
    if ( (DWORD)randBetween(1,10000) > getNotConsumeRatio())
      rPack.reduceItem(rItemReq, ESOURCE_PET_ADVENTURE, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PETWORK);
  }
  //宠物冒险Buff有可能减少战斗时间
  DWORD dwBattleTimeReq = pCFG->dwBattleTimeReq - (DWORD)(pCFG->dwBattleTimeReq * getReduceBattleTimeRatio() );
  m_pUser->addBattleTime(dwBattleTimeReq, false);

  //宠物冒险Buff有可能减少冒险消耗时间,通过把开始时间提前达到减少时间的目的
  DWORD dwStartTime = xTime::getCurSec();
  dwStartTime -= (DWORD)(pCFG->dwNeedTime * (getReduceConsumeTimeRatio()));
  rItem.set_id(pCFG->dwID);
  rItem.set_starttime(dwStartTime);
  rItem.set_status(EPETADVENTURESTATUS_ACCEPT);
  rItem.set_specid(cmd.specid());

  for (int i = 0; i < cmd.petids_size(); ++i)
  {
    const string& s = cmd.petids(i);
    if (s == "0")
    {
      rItem.add_eggs();
      continue;
    }

    ItemEgg* pEgg = dynamic_cast<ItemEgg*>(rPack.getItem(s));
    if (pEgg != nullptr)
    {
      ItemData* pEggData = rItem.add_eggs();
      pEgg->toItemData(pEggData);
      rPack.reduceItem(pEgg->getGUID(), pEgg->getCount(), ESOURCE_PET_ADVENTURE, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_PETWORK);
    }
  }

  OtherFactor factor;
   
  float cardRatio = getCardRatio();
  if (cardRatio > 0.0f)
    factor.setCardBuffRatio(cardRatio);
  
  // calc reward
  float kindRatio = getRewardKindRatio() ;
  if (kindRatio > 0.0f)
    factor.setKindRatio(kindRatio);

  PetAdventure::calcReward(m_pUser, rItem, pCFG, setEnableCond.size(), fEffectValue, factor);
  //计算Buffer带来的额外收益
  calcBuffReward(pCFG, rItem);
  m_vecPetAdventureBuff.clear();

  // inform client
  PetAdventureResultNtfPetCmd ret;
  ret.mutable_item()->CopyFrom(rItem);
  PROTOBUF(ret, send, len);
  m_pUser->sendCmdToMe(send, len);

  m_pUser->getServant().onFinishEvent(ETRIGGER_PET_ADVENTURE);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_PET_ADVENTURE);
  // save now
  m_pUser->saveDataNow();

  XLOG << "[宠物冒险-开始冒险]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "id :" << cmd.id() << "冒险成功,稀有奖励数" << setEnableCond.size() << "结果" << rItem.status();
  for (int i = 0; i < rItem.eggs_size(); ++i)
    XLOG << rItem.eggs(i).ShortDebugString();
  for (int i = 0; i < rItem.raresreward_size(); ++i)
    XLOG << rItem.raresreward(i).ShortDebugString();
  XLOG << "数据大小" << rItem.ByteSize() << XEND;

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Pet_Adventure;
  EPetAdventureLogType eLogType = EPetAdventureLogType_Start;

  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  string strNames = ssNames.str();
  PlatLogManager::getMe().PetAdventureLog(thisServer,
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,

    eLogType,
    cmd.id(),
    strNames,
    setEnableCond.size()
  );

  return true;
}

bool PetAdventure::getAdventureReward(const GetAdventureRewardPetCmd& cmd)
{
  auto m = m_mapIDAdventure.find(cmd.id());
  if (m == m_mapIDAdventure.end())
  {
    XERR << "[宠物冒险-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "领取失败,未在冒险" << XEND;
    return false;
  }
  if (m->second.status() == EPETADVENTURESTATUS_SUBMIT)
  {
    XERR << "[宠物冒险-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "领取失败,已领取" << XEND;
    return false;
  }

  const SPetAdventureCFG* pCFG = PetConfig::getMe().getAdventureCFG(cmd.id());
  if (pCFG == nullptr)
  {
    XERR << "[宠物冒险-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "领取失败,未在 Table_PetAdventure.txt 表中找到" << XEND;
    return false;
  }

  if (xTime::getCurSec() < m->second.starttime() + pCFG->dwNeedTime)
  {
    XERR << "[宠物冒险-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "id :" << cmd.id() << "领取失败,冒险未完成,该冒险开始" << m->second.starttime() << "完成时间 :" << (m->second.starttime() + pCFG->dwNeedTime) << XEND;
    return false;
  }

  TVecItemData vecRewards;
  TVecItemData vecExtraRewards;
  /*for (int i = 0; i < m->second.steps_size(); ++i)
  {
    const PetAdventureStep& rStep = m->second.steps(i);
    for (int j = 0; j < rStep.items_size(); ++j)
      combineItemData(vecRewards, rStep.items(j));
  }
  */
  for (int i = 0; i < m->second.rewardinfo_size(); ++i)
  {
    const PetMonsterRewardInfo& rwdinfo = m->second.rewardinfo(i);
    for (int j = 0; j < rwdinfo.items_size(); ++j)
      combineItemData(vecRewards, rwdinfo.items(j));
  }
  for (int i = 0; i < m->second.raresreward_size(); ++i)
    combineItemData(vecRewards, m->second.raresreward(i));

  for (int i = 0 ; i < m->second.extrarewardinfo_size(); ++i)
    combineItemData(vecExtraRewards, m->second.extrarewardinfo(i));

  DWORD dwCount = 0;
  auto v = find_if(vecRewards.begin(), vecRewards.end(), [](const ItemData& r) -> bool{
    return r.base().id() == ITEM_BASEEXP;
  });
  if (v != vecRewards.end())
    dwCount = v->base().count();
  auto vExtra = find_if(vecExtraRewards.begin(), vecExtraRewards.end(), [](const ItemData& r) -> bool{
   return r.base().id() == ITEM_BASEEXP;
  });
  if (vExtra != vecExtraRewards.end())
    dwCount += vExtra->base().count();

  Package& rPackage = m_pUser->getPackage();
  BasePackage* pMainPack = rPackage.getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[宠物冒险-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "领取失败,未找到" << EPACKTYPE_MAIN << XEND;
    return false;
  }
  TVecItemData vecAllRewardIDs;
  combineItemData(vecAllRewardIDs, vecRewards);
  combineItemData(vecAllRewardIDs, vecExtraRewards);
  if (pMainPack->checkAddItem(vecAllRewardIDs, EPACKMETHOD_CHECK_WITHPILE) == false)
  {
    MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PACK_FULL);
    XDBG << m->second.ShortDebugString() << XEND;
    return false;
  }
  pMainPack->addItem(vecRewards, EPACKMETHOD_CHECK_WITHPILE);
  pMainPack->update(xTime::getCurSec());

  pMainPack->addItem(vecExtraRewards, EPACKMETHOD_CHECK_WITHPILE);
  pMainPack->update(xTime::getCurSec());

  // return egg
  stringstream ssNames;
  for (int i = 0; i < m->second.eggs_size(); ++i)
  {
    ItemData* pEgg = m->second.mutable_eggs(i);
    if (pEgg->base().id() == 0)
      continue;

    DWORD dwExp = LuaManager::getMe().call<DWORD>("calcPetAdventureBaseExp", pCFG->dwPetBaseExp, dwCount, pCFG->dwReqLv, pEgg->egg().lv(), pCFG->dwMaxPet);
    pEgg->mutable_egg()->set_exp(pEgg->egg().exp() + dwExp);
    if (dwExp != 0)
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_PET_EXP_GET, MsgParams{pEgg->egg().name(), dwExp});

    /*SScenePetData stData;
    stData.dwLv = pEgg->egg().lv();
    stData.qwExp = pEgg->egg().exp();
    stData.pOwner = m_pUser;
    stData.baseLevelup();
    pEgg->mutable_egg()->set_lv(stData.dwLv);
    pEgg->mutable_egg()->set_exp(stData.qwExp);*/

    //pMainPack->addItem(*pEgg, EPACKMETHOD_NOCHECK);
    rPackage.addItem(*pEgg, EPACKMETHOD_NOCHECK);
    XLOG << "[宠物冒险-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "id :" << cmd.id() << "宠物 id :" << pEgg->base().guid() << pEgg->egg().id() << "获取经验" << dwExp << XEND;
    ssNames << pEgg->egg().name() << ";";
  }

  m_setUnlockArea.insert(pCFG->dwUnlockArea);

  // delete adventure
  if (pCFG->eType == EPETADVENTURETYPE_ONCE)
  {
    m->second.set_status(EPETADVENTURESTATUS_SUBMIT);

    /*TVecItemData vecReward;
    for (int i = 0; i < m->second.steps_size(); ++i)
    {
      const PetAdventureStep& rStep = m->second.steps(i);
      for (int j = 0; j < rStep.items_size(); ++j)
        combineItemData(vecReward, rStep.items(j));
    }
    m->second.clear_steps();
    PetAdventureStep* pStep = m->second.add_steps();
    for (auto &v : vecReward)
      pStep->add_items()->CopyFrom(v);
      */

    for (int i = 0; i < m->second.eggs_size(); ++i)
    {
      ItemData* pEggData = m->second.mutable_eggs(i);
      DWORD dwID = pEggData->base().id();
      DWORD dwEggID = pEggData->egg().id();
      DWORD dwBaseLv = pEggData->egg().lv();
      DWORD dwFriendLv = pEggData->egg().friendlv();

      pEggData->Clear();
      pEggData->mutable_base()->set_id(dwID);
      pEggData->mutable_egg()->set_id(dwEggID);
      pEggData->mutable_egg()->set_lv(dwBaseLv);
      pEggData->mutable_egg()->set_friendlv(dwFriendLv);
    }
  }
  else
  {
    m_mapIDAdventure.erase(m);
  }

  // inform client
  sendAdventureList();

  // save now
  m_pUser->getAchieve().onPetAdventure();
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_PET_ADVENTURE_REWARD);
  m_pUser->saveDataNow();

  XLOG << "[宠物冒险-领取奖励]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << cmd.id() << "领取成功" << XEND;

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Pet_Adventure;
  EPetAdventureLogType eLogType = EPetAdventureLogType_Take;

  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  string strNames = ssNames.str();
    PlatLogManager::getMe().PetAdventureLog(thisServer,
      m_pUser->accid,
      m_pUser->id,
      eType,
      eid,

      eLogType,
      cmd.id(),
      strNames,
      0
    );
  return true;
}

void PetAdventure::timer(DWORD curSec)
{
  for (auto &m : m_mapIDAdventure)
  {
    if (m.second.status() == EPETADVENTURESTATUS_SUBMIT || m.second.status() == EPETADVENTURESTATUS_COMPLETE)
      continue;

    const SPetAdventureCFG* pCFG = PetConfig::getMe().getAdventureCFG(m.first);
    if (pCFG == nullptr)
      continue;

    if (curSec < m.second.starttime() + pCFG->dwNeedTime)
      continue;

    m.second.set_status(EPETADVENTURESTATUS_COMPLETE);
    m_pUser->getTip().addRedTip(EREDSYS_PET_ADVENTURE);
  }
}

float PetAdventure::calcScore(EPetEfficiencyType eType)
{
  DWORD score = 0;
  const SPetAdventureEffCFG& rCFG = MiscConfig::getMe().getPetAdvEffCFG();
  switch(eType)
  {
    case EPETEFFICIENCY_MIN:
    case EPETEFFICIENCY_MAX:
      break;
    case EPETEFFICIENCY_REFINE:
      {
        EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
        if (pEquipPack == nullptr)
          return 0;
        TVecSortItem pVecItems;
        pEquipPack->getEquipItems(pVecItems);

        for (auto &v : pVecItems)
        {
          ItemEquip* pEquip = dynamic_cast<ItemEquip*>(v);
          if (pEquip == nullptr)
            continue;
          DWORD refinelv = pEquip->getRefineLv();
          if (refinelv == 0)
            continue;
          if (rCFG.vecRefineScore.size() >= refinelv)
            score += rCFG.vecRefineScore[refinelv - 1];
        }
        score = score > rCFG.dwRefineMax ? rCFG.dwRefineMax : score;
      }
      break;
    case EPETEFFICIENCY_ENCHANT:
      {
        EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
        if (pEquipPack == nullptr)
          return 0;
        TVecSortItem pVecItems;
        pEquipPack->getEquipItems(pVecItems);

        for (auto &v : pVecItems)
        {
          ItemEquip* pEquip = dynamic_cast<ItemEquip*>(v);
          const EnchantData& endata = pEquip->getEnchantData();
          for (int i = 0; i < endata.attrs_size(); ++i)
          {
            auto it = rCFG.mapType2MaxScoreAndBaseAttr.find(endata.attrs(i).type());
            if (it != rCFG.mapType2MaxScoreAndBaseAttr.end())
            {
              if (it->second.second && (endata.attrs(i).value() * 100 / it->second.second >= rCFG.dwEnchantCalcPer))
                score += endata.attrs(i).value() / it->second.second * it->second.first;
            }
          }
          for (int i = 0; i < endata.extras_size(); ++i)
          {
            DWORD buffid = endata.extras(i).buffid();
            if (buffid == 0)
              continue;
            DWORD lv = buffid % 10;
            if (lv <= rCFG.vecSuperEnchantScore.size() && lv > 0)
              score += rCFG.vecSuperEnchantScore[lv - 1];
          }
        }
        score = score > rCFG.dwEnchantMax ? rCFG.dwEnchantMax : score;;
      }
      break;
    case EPETEFFICIENCY_STAR:
      {
        score = m_pUser->getAstrolabes().getPetAdventureScore();
        score = score > rCFG.dwStarMax ? rCFG.dwStarMax : score;
      }
      break;
    case EPETEFFICIENCY_TITLE:
      {
        for (auto &m : rCFG.mapItemID2Score)
        {
          if (m_pUser->getTitle().hasTitle(m.first))
            score += m.second;
        }
        score = score > rCFG.dwTitleMax ? rCFG.dwTitleMax : score;
      }
      break;
    case EPETEFFICIENCY_HEADWEAR:
      {
        score = m_pUser->getManual().getPetAdventureScore(EMANUALTYPE_FASHION);
        score = score > rCFG.dwHeadWearMax ? rCFG.dwHeadWearMax : score;
      }
      break;
    case EPETEFFICIENCY_CARD:
      {
        score = m_pUser->getManual().getPetAdventureScore(EMANUALTYPE_CARD);
        score = score > rCFG.dwCardMax ? rCFG.dwCardMax : score;
      }
      break;
  }
  return (float)score / 10000.0f;
}

void PetAdventure::checkAdventureBuffs(ItemEgg* pEgg)
{
    VecAdventureBuff adventureBuffs;
    adventureBuffs.reserve(pEgg->getEggData().skillids().size());
    adventureBuffs.clear();

    for (auto skillid : pEgg->getEggData().skillids())
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(skillid);
      if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
      {
        if (pSkill->haveDirectBuff() == false)
          continue;
        for (auto& buffid : pSkill->getSelfBuffs())
        {
          TPtrBufferState buffPtr = BufferManager::getMe().getBuffById(buffid);
          if (nullptr == buffPtr)
          {
            XERR << "Buffer 没有找到, ID =" << buffid << XEND;
            continue;
          }

          if (EBuffType::EBUFFTYPE_PETADVENTURE ==  buffPtr->getBuffType())
          {
            BuffPetAdventure* buff = dynamic_cast<BuffPetAdventure*>(buffPtr.get());
            if (!buff) continue;
            adventureBuffs.push_back(buff);
          }
        }
      }
    }
    m_vecPetAdventureBuff.push_back(std::move(adventureBuffs));
}

DWORD PetAdventure::getNotConsumeRatio()
{
  float ratio = 0.0f;
  for (auto& buffs : m_vecPetAdventureBuff)
  {
    for (auto& ptrBuff : buffs)
    {
      if (ptrBuff)
        ratio += ptrBuff->getRatioNotConsume();
    }
  }

  return (DWORD)(ratio);
}

float PetAdventure::getReduceBattleTimeRatio()
{
  float ratio = 0.0f;
  for (auto& buffs : m_vecPetAdventureBuff)
  {
    for (auto& ptrBuff : buffs)
    {
      if (ptrBuff)
        ratio += ptrBuff->getRatioReduceBattleTime();
    }
  }
  return (ratio > 1.0f ? 1.0f : ratio);
}

float PetAdventure::getReduceConsumeTimeRatio()
{
  float ratio = 0.0f;
  for (auto& buffs : m_vecPetAdventureBuff)
  {
    for (auto& ptrBuff : buffs)
    {
      if (ptrBuff)
        ratio += ptrBuff->getRatioReduceConsumeTime();
    }
  }
  return (ratio > 1.0f ? 1.0f : ratio);
}

float PetAdventure::getRewardKindRatio()
{
  float ratio = 0.0f;
  for (auto& buffs : m_vecPetAdventureBuff)
  {
    for (auto& ptrBuff : buffs)
    {
      if (ptrBuff)
        ratio += ptrBuff->getRatioKind();
    }
  }
  return ratio;
}

float PetAdventure::getCardRatio()
{
  float ratio = 0.0f;
  for (auto& buffs : m_vecPetAdventureBuff)
  {
    for (auto& ptrBuff : buffs)
    {
      if (ptrBuff)
        ratio += ptrBuff->getRatioCard();
    }
  }
  return ratio;
}

DWORD PetAdventure::getRareBoxRatio()
{
  float ratio = 0.0f;
  for(auto& buffs : m_vecPetAdventureBuff)
  {
    for (auto& ptrBuff : buffs)
    {
      if (ptrBuff)
        ratio += ptrBuff->getRatioRareBox();
    }
  }
  return (DWORD)(ratio);
}
