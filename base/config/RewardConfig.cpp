#include "RewardConfig.h"
#include "xLuaTable.h"
#include "ItemConfig.h"
#include "NpcConfig.h"
#include "xTime.h"
#include "MapConfig.h"
#include "LuaManager.h"
#include "BaseConfig.h"
#include "MsgManager.h"

// config data
bool SRewardItem::isValid(const RewardEntry& rEntry) const
{
  if (vecProfession.empty() == false)
  {
    if (find(vecProfession.begin(), vecProfession.end(), rEntry.pro()) == vecProfession.end())
      return false;
  }
  if (pairBaseLvReq.first != 0 && pairBaseLvReq.second != 0)
  {
    if (rEntry.baselv() < pairBaseLvReq.first || rEntry.baselv() > pairBaseLvReq.second)
      return false;
  }
  if (pairJobLvReq.first != 0 && pairJobLvReq.second != 0)
  {
    if (rEntry.joblv() < pairJobLvReq.first || rEntry.joblv() > pairJobLvReq.second)
      return false;
  }

  return true;
}

// reward config
RewardConfig::RewardConfig()
{

}

RewardConfig::~RewardConfig()
{

}

bool RewardConfig::loadConfig()
{
  bool bCorrect = true;
  if (loadRewardConfig() == false)
    bCorrect = false;

  return bCorrect;
}

bool RewardConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto m = m_mapRewardCFG.begin(); m != m_mapRewardCFG.end(); ++m)
  {
    for (auto v = m->second.vecItems.begin(); v != m->second.vecItems.end(); ++v)
    {
      for (auto k = v->vecItems.begin(); k != v->vecItems.end(); ++k)
      {
        if (m->second.eType == EREWARDTYPE_ALL || m->second.eType == EREWARDTYPE_ONE || m->second.eType == EREWARDTYPE_ONE_DYNAMIC)
        {
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(k->oItem.id());
          if (pCFG == nullptr)
          {
            XERR << "[RewardConfig] teamid : " << m->first << " itemid : " << k->oItem.id() << " 未在 Table_Item.txt 表中找到" << XEND;
            bCorrect = false;
            continue;
          }
        }
        else if (m->second.eType == EREWARDTYPE_REWARD)
        {
          auto n = m_mapRewardCFG.find(k->oItem.id());
          if (n == m_mapRewardCFG.end())
          {
            XERR << "[RewardConfig] teamid : " << m->first << " rewardid : " << k->oItem.id() << " 未在 Table_Reward.txt 表中找到" << XEND;
            bCorrect = false;
            continue;
          }
        }
      }
    }
  }

  return bCorrect;
}

const SRewardCFG* RewardConfig::getRewardCFG(DWORD id) const
{
  auto m = m_mapRewardCFG.find(id);
  if (m == m_mapRewardCFG.end())
    return nullptr;

  return &m->second;
}
/**
 * 暂时用于宠物冒险阶段
 */ 
bool RewardConfig::roll_adventure(DWORD id, const RewardEntry& rEntry, TVecItemInfo& vecItemInfo, ESource source,const OtherFactor& factor ,float fRatio /* = 1.0f*/, DWORD dwMapID /* = 0 */)
{
  auto itrep = m_mapReplacePairs.find(id);
  if (itrep != m_mapReplacePairs.end())
  {
    if (itrep->second == 0)
    {
      XLOG << "[Reward-随机] 奖励id:" << id << "已被替换为0, 不可获得" << XEND;
      return true;
    }
    XLOG << "[Reward-随机] 奖励id:" << id << "被替换为:" << itrep->second << XEND;
    id = itrep->second;
  }

  // get config
  const SRewardCFG* pCFG = getRewardCFG(id);
  if (pCFG == nullptr)
  {
    XERR << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() << "source :" << source << "ratio :" << fRatio << "未在Table_Reward.txt表中找到" << XEND;
    return false;
  }

  if (!pCFG->setValidMap.empty() && dwMapID != 0)
  {
    if (pCFG->setValidMap.find(dwMapID) == pCFG->setValidMap.end())
    {
      XLOG << "[Reward-随机] id :" << id << "地图 : " << dwMapID << "不可在此地图获得" << XEND;
      return true;
    }
  }

  vecItemInfo.clear();

  bool bCorrect = false;
  if (pCFG->eType == EREWARDTYPE_ALL)
    bCorrect = roll_all_adventure(pCFG, vecItemInfo, rEntry, source, fRatio, factor);
  else if (pCFG->eType == EREWARDTYPE_ONE)
    bCorrect = roll_one(pCFG, vecItemInfo, rEntry, source, fRatio);
  else if (pCFG->eType == EREWARDTYPE_REWARD)
    bCorrect = roll_reward(pCFG, vecItemInfo, rEntry, source, fRatio);
  else if (pCFG->eType == EREWARDTYPE_REWARD_ALL)
    bCorrect = roll_reward_all(pCFG, vecItemInfo, rEntry, source, fRatio);
  else
  {
    XERR << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() <<  "source :" << source << "ratio :" << fRatio << "type :" << pCFG->eType << "不合法" << XEND;
    return false;
  }

  for (auto it = vecItemInfo.begin(); it != vecItemInfo.end(); )
  {
    const SItemCFG* tmpcfg = ItemConfig::getMe().getItemCFG(it->id());
    if (tmpcfg)
    {
      if (tmpcfg->isTimeValid() == false)
      {
        it = vecItemInfo.erase(it);
        XLOG << "[Reward-随机] id :" << id << "item:" << it->id() << "source:" << source << "道具未达生效时间,冒险玩法随机" << XEND;
        continue;
      }

      if (source == ESOURCE_SEAL && ItemConfig::getMe().isEquip(tmpcfg->eItemType))
      {
        DWORD refinelv = LuaManager::getMe().call<DWORD> ("CalcSealEquipRefineLv", (DWORD)tmpcfg->eQualityType);
        if (refinelv != 0)
        {
          it->set_refinelv(refinelv);
          XLOG << "[Reward-随机] 封印装备掉落" << it->id() << "设置精炼等级 :" << refinelv << XEND;
        }
      }
    }
    ++it;
  }

  if (!bCorrect)
  {
    XERR << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() << "source :" << source << "ratio :" << fRatio << " 失败" << XEND;
    return bCorrect;
  }

  XLOG << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() << "source :" << source << "ratio :" << fRatio << "获取以下物品" << vecItemInfo << XEND;
  return bCorrect;
}

bool RewardConfig::roll(DWORD id, const RewardEntry& rEntry, TVecItemInfo& vecItemInfo, ESource source, float fRatio /* = 1.0f*/, DWORD dwMapID /* = 0 */)
{
  auto itrep = m_mapReplacePairs.find(id);
  if (itrep != m_mapReplacePairs.end())
  {
    if (itrep->second == 0)
    {
      XLOG << "[Reward-随机] 奖励id:" << id << "已被替换为0, 不可获得" << XEND;
      return true;
    }
    XLOG << "[Reward-随机] 奖励id:" << id << "被替换为:" << itrep->second << XEND;
    id = itrep->second;
  }

  // get config
  const SRewardCFG* pCFG = getRewardCFG(id);
  if (pCFG == nullptr)
  {
    XERR << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() << "source :" << source << "ratio :" << fRatio << "未在Table_Reward.txt表中找到" << XEND;
    return false;
  }

  if (!pCFG->setValidMap.empty() && dwMapID != 0)
  {
    if (pCFG->setValidMap.find(dwMapID) == pCFG->setValidMap.end())
    {
      XLOG << "[Reward-随机] id :" << id << "地图 : " << dwMapID << "不可在此地图获得" << XEND;
      return true;
    }
  }

  vecItemInfo.clear();

  bool bCorrect = false;
  if (pCFG->eType == EREWARDTYPE_ALL)
    bCorrect = roll_all(pCFG, vecItemInfo, rEntry, source, fRatio);
  else if (pCFG->eType == EREWARDTYPE_ONE)
    bCorrect = roll_one(pCFG, vecItemInfo, rEntry, source, fRatio);
  else if (pCFG->eType == EREWARDTYPE_REWARD)
    bCorrect = roll_reward(pCFG, vecItemInfo, rEntry, source, fRatio);
  else if (pCFG->eType == EREWARDTYPE_REWARD_ALL)
    bCorrect = roll_reward_all(pCFG, vecItemInfo, rEntry, source, fRatio);
  else if (pCFG->eType == EREWARDTYPE_ONE_DYNAMIC)
    bCorrect = roll_one_dynamic(pCFG, vecItemInfo, rEntry, source, fRatio);
  else
  {
    XERR << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() <<  "source :" << source << "ratio :" << fRatio << "type :" << pCFG->eType << "不合法" << XEND;
    return false;
  }

  for (auto it = vecItemInfo.begin(); it != vecItemInfo.end(); )
  {
    const SItemCFG* tmpcfg = ItemConfig::getMe().getItemCFG(it->id());
    if (tmpcfg)
    {
      if (tmpcfg->isTimeValid() == false)
      {
        it = vecItemInfo.erase(it);
        XLOG << "[Reward-随机] id :" << id << "item:" << it->id() << "source:" << source << "道具未达生效时间" << XEND;
        continue;
      }

      if (source == ESOURCE_SEAL && ItemConfig::getMe().isEquip(tmpcfg->eItemType))
      {
        DWORD refinelv = LuaManager::getMe().call<DWORD> ("CalcSealEquipRefineLv", (DWORD)tmpcfg->eQualityType);
        if (refinelv != 0)
        {
          it->set_refinelv(refinelv);
          XLOG << "[Reward-随机] 封印装备掉落" << it->id() << "设置精炼等级 :" << refinelv << XEND;
        }
      }
    }
    ++it;
  }

  if (!bCorrect)
  {
    XERR << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() << "source :" << source << "ratio :" << fRatio << " 失败" << XEND;
    return bCorrect;
  }

  XLOG << "[Reward-随机] id :" << id << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() << "source :" << source << "ratio :" << fRatio << "获取以下物品" << vecItemInfo << XEND;
  return bCorrect;
}

bool RewardConfig::checkCardBuffConfition(const SRewardItem& rItem, const OtherFactor& factor)
{
  if (!factor.getEnable())
    return false;
  if (!factor.hasCardBuff())
    return false;
  for (auto& item: rItem.vecItems)
  {
    DWORD id = item.oItem.id();
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
    if (nullptr == pCFG)
      continue;
    if (ItemConfig::getMe().isCard(pCFG->eItemType))
      return true;
  }
  return false;
}

bool RewardConfig::roll_all_adventure(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio, const OtherFactor& factor)
{
  if (pCFG == nullptr || pCFG->eType != EREWARDTYPE_ALL)
    return false;

  auto randitem = [&](const SRewardItem& rItem, DWORD dwRate)
  {
    DWORD rate = randBetween(0, BASE_RATE);
    //检测, 如果有卡片Buff，判断是不是卡片
    DWORD dwTmpRate = dwRate ;
    
    if (checkCardBuffConfition(rItem, factor))
    {
      dwTmpRate += dwRate * factor.getCardBuffRatio();
      XDBG << "[Reward-随机]" << "冒险卡片随机,原概率为:" << dwRate << "实际概率为:" << dwTmpRate << XEND;
    }

    if (factor.getKindRatio() > 0 )
    {
      dwTmpRate += dwRate * factor.getKindRatio();
      XDBG << "[Reward-随机]" << "冒险物品随机,原概率为:" << dwRate << "实际概率为:" << dwTmpRate << XEND;
    }

    if (rate > dwTmpRate)
      return;
    for (auto &item : rItem.vecItems)
    {
      for (DWORD d = 0; d < item.dwStack; ++d)
      {
        ItemInfo oItem;
        oItem.set_id(item.oItem.id());
        oItem.set_count(item.oItem.count() / item.dwStack);
        oItem.set_source(eSource);
        vecItemInfo.push_back(oItem);
      }
    }
  };
  for (auto v = pCFG->vecItems.begin(); v != pCFG->vecItems.end(); ++v)
  {
    if (v->isValid(rEntry) == false)
      continue;

    // rand count
    DWORD dwCount = static_cast<DWORD>(ceil(v->dwCount * fRatio));
    for (DWORD d = 0; d < dwCount; ++d)
      randitem(*v, BASE_RATE);

    // rand extra
    float fRaw = 1.0f * v->dwRate / 10000.0f;   //v->dwRate already * 10000
    float f1 = fRaw * fRatio;
    DWORD dw1 = 0;
    DWORD dw2 = 0;
    if (f1 >= 10000.0f)
    {
      dw1 = f1 / 10000.0f;
      float f2 = f1 - dw1 * 10000.0f;
      dw2 = f2 * 10000.0f;
    }
    else
    {
      dw2 = f1 * 10000.0f;
    }

    for (DWORD d = 0; d < dw1; ++d)
      randitem(*v, BASE_RATE);

    randitem(*v, dw2);
  }

  return true;
}

bool RewardConfig::roll_all(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio)
{
  if (pCFG == nullptr || pCFG->eType != EREWARDTYPE_ALL)
    return false;

  auto randitem = [&](const SRewardItem& rItem, DWORD dwRate, bool force = false) -> bool
  {
    if (force == false)
    {
      DWORD rate = randBetween(0, BASE_RATE);
      if (rate > dwRate)
        return false;
    }
    for (auto &item : rItem.vecItems)
    {
      for (DWORD d = 0; d < item.dwStack; ++d)
      {
        ItemInfo oItem;
        oItem.set_id(item.oItem.id());
        oItem.set_count(item.oItem.count() / item.dwStack);
        oItem.set_source(eSource);
        vecItemInfo.push_back(oItem);
      }
    }
    return true;
  };
  bool safetyroll = false;
  for (auto v = pCFG->vecItems.begin(); v != pCFG->vecItems.end(); ++v)
  {
    if (v->isValid(rEntry) == false)
      continue;

    bool roll = false;
    // rand count
    DWORD dwCount = static_cast<DWORD>(ceil(v->dwCount * fRatio));
    for (DWORD d = 0; d < dwCount; ++d)
      if (randitem(*v, BASE_RATE))
        roll = true;

    // rand extra
    float fRaw = 1.0f * v->dwRate / 10000.0f;   //v->dwRate already * 10000
    float f1 = fRaw * fRatio;
    DWORD dw1 = 0;
    DWORD dw2 = 0;
    if (f1 >= 10000.0f)
    {
      dw1 = f1 / 10000.0f;
      float f2 = f1 - dw1 * 10000.0f;
      dw2 = f2 * 10000.0f;
    }
    else
    {
      dw2 = f1 * 10000.0f;
    }

    for (DWORD d = 0; d < dw1; ++d)
      if (randitem(*v, BASE_RATE))
        roll = true;

    if (randitem(*v, dw2))
      roll = true;

    if (v->bIsSafety) // 被保底的奖励
      safetyroll = roll;
  }

  // 保底处理
  if (pCFG->oSafety.isSafety())
  {
    const SRewardItem* safetyitem = const_cast<RewardEntry&>(rEntry).safety(pCFG, safetyroll);
    if (safetyitem)
      randitem(*safetyitem, 0, true);
  }

  return true;
}

bool RewardConfig::roll_one(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio)
{
  if (pCFG == nullptr || pCFG->eType != EREWARDTYPE_ONE)
    return false;

  if (pCFG->bType2RatioEnable)
  {
    DWORD dwRate = BASE_RATE * fRatio;
    DWORD dwRand = randBetween(0, BASE_RATE);
    if (dwRand > dwRate)
      return false;
  }

  auto getitem = [&](const SRewardItem& ritem)
  {
    for (auto &item : ritem.vecItems)
    {
      for (DWORD d = 0; d < item.dwStack; ++d)
      {
        ItemInfo oItem;
        oItem.set_id(item.oItem.id());
        oItem.set_count(item.oItem.count() / item.dwStack);
        oItem.set_source(eSource);
        vecItemInfo.push_back(oItem);
      }
    }
  };

  bool roll = false;
  DWORD rate = randBetween(0, pCFG->maxRand);
  for (auto v = pCFG->vecItems.begin(); v != pCFG->vecItems.end(); ++v)
  {
    if (v->isValid(rEntry) == false)
      continue;

    if (rate > v->dwRate)
      continue;

    getitem(*v);

    if (v->bIsSafety)
      roll = true;

    break;
  }

  if (pCFG->oSafety.isSafety())
  {
    const SRewardItem* safetyitem = const_cast<RewardEntry&>(rEntry).safety(pCFG, roll);
    if (safetyitem)
    {
      vecItemInfo.clear();
      getitem(*safetyitem);
    }
  }

  return true;
}

bool RewardConfig::roll_reward(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio)
{
  if (pCFG == nullptr || pCFG->eType != EREWARDTYPE_REWARD)
    return false;

  auto getitem = [&](const SRewardItem& item)
  {
    for (auto o = item.vecItems.begin(); o != item.vecItems.end(); ++o)
    {
      TVecItemInfo vecItems;
      if (roll(o->oItem.id(), rEntry, vecItems, eSource, fRatio) == false)
      {
        XERR << "[Reward-随机-reward] id :" << o->oItem.id() << "pro :" << rEntry.pro() << "baselv :" << rEntry.baselv() << "joblv :" << rEntry.joblv() << "source :" << eSource << "ratio :" << fRatio << "失败" << XEND;
        continue;
      }

      for (auto &v : vecItems)
      {
        v.set_count(v.count() * o->oItem.count());
        vecItemInfo.push_back(v);
      }
    }
  };

  bool roll = false;
  for (auto v = pCFG->vecItems.begin(); v != pCFG->vecItems.end(); ++v)
  {
    if (v->isValid(rEntry) == false)
      continue;

    DWORD rate = randBetween(0, pCFG->maxRand);
    if (rate > v->dwRate)
      continue;

    getitem(*v);

    if (v->bIsSafety)
      roll = true;

    break;
  }

  if (pCFG->oSafety.isSafety()) // 被保底的奖励
  {
    const SRewardItem* safetyitem = const_cast<RewardEntry&>(rEntry).safety(pCFG, roll);
    if (safetyitem)
    {
      vecItemInfo.clear();
      getitem(*safetyitem);
    }
  }

  return true;
}

bool RewardConfig::roll_reward_all(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio)
{
  if (pCFG == nullptr || pCFG->eType != EREWARDTYPE_REWARD_ALL)
    return false;

  auto randitem = [&](const SRewardItem& rItem, DWORD dwRate, bool force = false) -> bool
  {
    if (force == false)
    {
      DWORD rate = randBetween(0, BASE_RATE);
      if (rate > dwRate)
        return false;
    }

    TVecItemInfo vecItem;
    for (auto &v : rItem.vecItems)
    {
      if (roll(v.oItem.id(), rEntry, vecItem, eSource, fRatio) == false)
        continue;

      for (auto &o : vecItem)
      {
        o.set_count(o.count() * v.oItem.count());
        vecItemInfo.push_back(o);
      }
    }
    return true;
  };

  bool safetyroll = false;
  for (auto v = pCFG->vecItems.begin(); v != pCFG->vecItems.end(); ++v)
  {
    if (v->isValid(rEntry) == false)
      continue;

    bool roll = false;
    // rand count
    DWORD dwCount = static_cast<DWORD>(ceil(v->dwCount * fRatio));
    for (DWORD d = 0; d < dwCount; ++d)
      if (randitem(*v, BASE_RATE))
        roll = true;

    // rand extra
    float fRaw = 1.0f * v->dwRate / 10000.0f;   //v->dwRate already * 10000
    float f1 = fRaw * fRatio;
    DWORD dw1 = 0;
    DWORD dw2 = 0;
    if (f1 >= 10000.0f)
    {
      dw1 = f1 / 10000.0f;
      float f2 = f1 - dw1 * 10000.0f;
      dw2 = f2 * 10000.0f;
    }
    else
    {
      dw2 = f1 * 10000.0f;
    }

    for (DWORD d = 0; d < dw1; ++d)
      if (randitem(*v, BASE_RATE))
        roll = true;

    if (randitem(*v, dw2))
      roll = true;
    /*DWORD dwCount = static_cast<DWORD>(ceil(v->dwCount * fRatio));
    for (DWORD d = 0; d < dwCount; ++d)
      randitem(*v, BASE_RATE);

    randitem(*v, static_cast<DWORD>(v->dwRate * fRatio));*/

    if (v->bIsSafety) // 被保底的奖励
      safetyroll = roll;
  }

  // 保底处理
  if (pCFG->oSafety.isSafety())
  {
    const SRewardItem* safetyitem = const_cast<RewardEntry&>(rEntry).safety(pCFG, safetyroll);
    if (safetyitem)
      randitem(*safetyitem, 0, true);
  }

  return true;
}

bool RewardConfig::roll_one_dynamic(const SRewardCFG* pCFG, TVecItemInfo& vecItemInfo, const RewardEntry& rEntry, ESource eSource, float fRatio)
{
  if (pCFG == nullptr || pCFG->eType != EREWARDTYPE_ONE_DYNAMIC)
    return false;

  if (pCFG->bType2RatioEnable)
  {
    DWORD dwRate = BASE_RATE * fRatio;
    DWORD dwRand = randBetween(0, BASE_RATE);
    if (dwRand > dwRate)
      return false;
  }

  DWORD total = 0;
  vector<TVecRewardItem::const_iterator> items;
  for (auto v = pCFG->vecItems.begin(); v != pCFG->vecItems.end(); ++v)
  {
    if (v->isValid(rEntry) == false)
      continue;
    total += v->dwRate;
    items.push_back(v);
  }

  if (items.empty())
  {
    XLOG << "[RewardConfig] one_dynamic奖励:" << pCFG->id << "entry信息" << rEntry.pro() << rEntry.baselv() << rEntry.joblv() << "source:" << eSource << "fRatio:" << fRatio << "没有奖励" << XEND;
    return true;
  }

  DWORD rate = randBetween(0, total), pass = 0;
  for (auto& v : items)
  {
    pass += v->dwRate;
    if (rate > pass)
      continue;

    for (auto &item : v->vecItems)
    {
      for (DWORD d = 0; d < item.dwStack; ++d)
      {
        ItemInfo oItem;
        oItem.set_id(item.oItem.id());
        oItem.set_count(item.oItem.count() / item.dwStack);
        oItem.set_source(eSource);
        vecItemInfo.push_back(oItem);
      }
    }
    break;
  }

  return true;
}

bool RewardConfig::loadRewardConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Reward.txt"))
  {
    XERR << "[RewardConfig], 加载配置Table_Reward.txt失败" << XEND;
    return false;
  }

  m_mapRewardCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Reward", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // id
    DWORD id = m->second.getTableInt("team");

    // check exist
    auto o = m_mapRewardCFG.find(id);
    if (o == m_mapRewardCFG.end())
    {
      SRewardCFG stCFG;

      stCFG.id = id;

      // check reward type
      DWORD type = m->second.getTableInt("type");
      if (type <= EREWARDTYPE_MIN || type >= EREWARDTYPE_MAX)
      {
        XERR << "[RewardConfig], id : " << stCFG.id << " type : " << type << " error!" << XEND;
        bCorrect = false;
        continue;
      }
      stCFG.eType = static_cast<ERewardType>(type);
      stCFG.bType2RatioEnable = m->second.getTableInt("type2ratio") == 1;

      // insert to list
      m_mapRewardCFG[id] = stCFG;
      o = m_mapRewardCFG.find(id);
      if (o == m_mapRewardCFG.end())
      {
        XERR << "[RewardConfig] id : " << stCFG.id << " not found!" << XEND;
        bCorrect = false;
        continue;
      }
    }

    // get map config
    TSetDWORD setmap;
    auto getmap = [&](const string& key, xLuaData& data)
    {
      o->second.setValidMap.insert(data.getInt());
      setmap.insert(data.getInt());
    };
    m->second.getMutableData("Map").foreach(getmap);
    if (setmap.size() != o->second.setValidMap.size())
    {
      XERR << "[RewardConfig] id : " << id << "Map 配置错误" << XEND;
      bCorrect = false;
    }

    // create item
    SRewardItem stItem;

    // rate
    stItem.dwRate = static_cast<DWORD>(m->second.getTableFloat("rate") * 10000);
    stItem.dwCount = m->second.getTableInt("lootnum");

    // get item
    xLuaData& item = m->second.getMutableData("item");
    auto itemf = [&](const string& key, xLuaData& data)
    {
      SSingleItem item;
      item.oItem.set_id(data.getTableInt("id"));
      item.oItem.set_count(data.getTableInt("num"));
      item.dwStack = data.getTableInt("stack");
      if (item.dwStack == 0)
        item.dwStack = 1;

      if (item.dwStack > item.oItem.count() || item.dwStack > 100)
      {
        XERR << "[Reward配置-加载] id : " << id << " stack : " << item.dwStack << " num : " << item.oItem.count() << " stack > num" << XEND;
        bCorrect = false;
        return;
      }
      stItem.vecItems.push_back(item);
    };
    item.foreach(itemf);

    // profession
    xLuaData& oclass = m->second.getMutableData("Class");
    auto classf = [&](const string& str, xLuaData& data)
    {
      DWORD dwClass = data.getInt();
      if (dwClass == 0)
        return;
      if (dwClass <= EPROFESSION_MIN || dwClass >= EPROFESSION_MAX || EProfession_IsValid(dwClass) == false)
      {
        XERR << "[RewardConfig] id : " << o->first << " class : " << dwClass << " invalid" << XEND;
        bCorrect = false;
        return;
      }
      stItem.vecProfession.push_back(static_cast<EProfession>(dwClass));
    };
    oclass.foreach(classf);

    // level
    xLuaData& level = m->second.getMutableData("Level");
    stItem.pairBaseLvReq.first = level.getMutableData("base").getTableInt("1");
    stItem.pairBaseLvReq.second = level.getMutableData("base").getTableInt("2");

    stItem.pairJobLvReq.first = level.getMutableData("job").getTableInt("1");
    stItem.pairJobLvReq.second = level.getMutableData("job").getTableInt("2");

    xLuaData& safetydata = m->second.getMutableData("Safety");
    if (safetydata.has("x"))
    {
      o->second.oSafety.dwX = safetydata.getTableInt("x");
      o->second.oSafety.dwY = safetydata.getTableInt("y");
      o->second.oSafety.dwLimit = safetydata.getTableInt("limit");
      o->second.oSafety.dwVersion = safetydata.getTableInt("version");
      DWORD expire = safetydata.getTableInt("expire");
      if (expire)
        o->second.oSafety.dwExpire = expire * DAY_T;
      else
        o->second.oSafety.dwExpire = REWARD_SAFETY_DEFAULT_EXPIRE * DAY_T;
      if (o->second.oSafety.isSafety())
        stItem.bIsSafety = true;
    }

    o->second.vecItems.push_back(stItem);
  }

  // process items
  for (auto m = m_mapRewardCFG.begin(); m != m_mapRewardCFG.end(); ++m)
  {
    sort(m->second.vecItems.begin(), m->second.vecItems.end(), [](const SRewardItem& r1, const SRewardItem& r2) -> bool{
      return r1.dwRate > r2.dwRate;
    });

    if (m->second.eType == EREWARDTYPE_ONE || m->second.eType == EREWARDTYPE_REWARD)
    {
      m->second.maxRand = 0;
      for (auto v = m->second.vecItems.begin(); v != m->second.vecItems.end(); ++v)
      {
        v->dwRate += m->second.maxRand;
        m->second.maxRand = v->dwRate;
      }
    }
  }

  if (bCorrect)
    XLOG << "[RewardConfig], 成功加载配置Table_Reward.txt成功" << XEND;
  return bCorrect;
}

bool RewardConfig::addReplace(DWORD dwOldID, DWORD dwNewID)
{
  auto it = m_mapRewardCFG.find(dwOldID);
  if (it == m_mapRewardCFG.end())
    return false;

  if (dwNewID != 0)
  {
    it = m_mapRewardCFG.find(dwNewID);
    if (it == m_mapRewardCFG.end())
      return false;
  }

  m_mapReplacePairs[dwOldID] = dwNewID;

  XLOG << "[Reward-替换], ID: " << dwNewID << "替换: " << dwOldID << XEND;
  return true;
}

bool RewardConfig::delReplace(DWORD dwID)
{
  auto it = m_mapReplacePairs.find(dwID);
  if (it == m_mapReplacePairs.end())
    return false;

  m_mapReplacePairs.erase(it);

  XLOG << "[Reward-替换], 删除替换, ID:" << dwID << XEND;

  return true;
}

bool RewardConfig::addExtraRwd(const SExtraNpcRwd& stExtraRwd)
{
  auto it = m_mapExtraRwds.find(stExtraRwd.uniqueID);
  if(it != m_mapExtraRwds.end())
  {
    XERR << "[Rewad-额外], 添加额外奖励, uniqueID值重复:" << stExtraRwd.uniqueID << stExtraRwd.exclude << XEND;
    return false;
  }

  SExtraNpcRwd stRwd;
  stRwd.setExtraIDs.insert(stExtraRwd.setExtraIDs.begin(), stExtraRwd.setExtraIDs.end());
  stRwd.setZoneTypes.insert(stExtraRwd.setZoneTypes.begin(), stExtraRwd.setZoneTypes.end());
  stRwd.setNpcTypes.insert(stExtraRwd.setNpcTypes.begin(), stExtraRwd.setNpcTypes.end());
  stRwd.setRaceTypes.insert(stExtraRwd.setRaceTypes.begin(), stExtraRwd.setRaceTypes.end());
  stRwd.setNatureTypes.insert(stExtraRwd.setNatureTypes.begin(), stExtraRwd.setNatureTypes.end());
  stRwd.exclude = stExtraRwd.exclude;
  stRwd.uniqueID = stExtraRwd.uniqueID;
  m_mapExtraRwds.insert(std::make_pair(stRwd.uniqueID, stRwd));

  XLOG << "[Reward-额外], 添加额外奖励, 怪物类型,包含值,uniqueID:" << stRwd.exclude << stExtraRwd.uniqueID << "额外奖励:";
  for (auto &s : stExtraRwd.setExtraIDs)
    XLOG << s;
  XLOG << XEND;

  return true;
}

bool RewardConfig::delExtraRwd(const SExtraNpcRwd& stExtraRwd)
{
  auto it = m_mapExtraRwds.find(stExtraRwd.uniqueID);
  if(it == m_mapExtraRwds.end())
  {
    XERR << "[Rewad-额外], 删除额外奖励未找到奖励, uniqueID值不存在:" << stExtraRwd.uniqueID << stExtraRwd.exclude << XEND;
    return false;
  }

  m_mapExtraRwds.erase(it);

  XLOG << "[Reward-额外], 删除额外奖励, 怪物类型,uniqueID:" << stExtraRwd.uniqueID << stExtraRwd.exclude << "额外奖励:";
  for (auto &s : stExtraRwd.setExtraIDs)
    XLOG << s;
  XLOG << XEND;

  return true;
}

bool RewardConfig::addExtraRwd(DWORD dwNpcID, const TSetDWORD& setRwds)
{
  auto it = m_mapNpcID2ExtraRwds.find(dwNpcID);
  if (it != m_mapNpcID2ExtraRwds.end())
  {
    it->second.insert(setRwds.begin(), setRwds.end());
    XLOG << " [Reward-额外], 添加额外奖励, 怪物ID:" << dwNpcID << "额外奖励:";
    for (auto &s : setRwds)
      XLOG << s;
    XLOG << XEND;
    return true;
  }

  TSetDWORD& setids = m_mapNpcID2ExtraRwds[dwNpcID];
  setids.insert(setRwds.begin(), setRwds.end());
  XLOG << " [Reward-额外], 添加额外奖励, 怪物ID:" << dwNpcID << "额外奖励:";
  for (auto &s : setRwds)
    XLOG << s;
  XLOG << XEND;

  return true;
}

bool RewardConfig::delExtraRwd(DWORD dwNpcID, const TSetDWORD& setRwds)
{
  auto it = m_mapNpcID2ExtraRwds.find(dwNpcID);
  if (it == m_mapNpcID2ExtraRwds.end())
  {
    XERR << "[Reward-额外], 删除额外奖励, 无法找到怪物ID:" << dwNpcID << XEND;
    return false;
  }

  for (auto &s : setRwds)
    it->second.erase(s);
  if (it->second.empty())
    m_mapNpcID2ExtraRwds.erase(it);

  XLOG << "[Reward-额外], 删除额外奖励, 怪物ID:" << dwNpcID << "额外奖励:";
  for (auto &s : setRwds)
    XLOG << s;
  XLOG << XEND;

  return true;
}

bool RewardConfig::getNpcExtraRwd(const SNpcCFG* pCFG, TSetDWORD& setRwds, bool superAiNpc) const
{
  if (pCFG == nullptr || superAiNpc == true)
    return false;
  if (m_mapNpcID2ExtraRwds.empty() && m_mapExtraRwds.empty())
    return false;

  auto it = m_mapNpcID2ExtraRwds.find(pCFG->dwID);
  if (it != m_mapNpcID2ExtraRwds.end())
  {
    setRwds.insert(it->second.begin(), it->second.end());
  }

  for( auto &r : m_mapExtraRwds)
  {
    if(r.second.exclude == 0)
    {
      if(r.second.setZoneTypes.empty() == false)
      {
        auto it = r.second.setZoneTypes.find(pCFG->eZoneType);
        if(it == r.second.setZoneTypes.end())
          continue;
      }

      if(r.second.setNpcTypes.empty() == false)
      {
        auto it = r.second.setNpcTypes.find(pCFG->eNpcType);
        if(it == r.second.setNpcTypes.end())
          continue;
      }

      if(r.second.setRaceTypes.empty() == false)
      {
        auto it = r.second.setRaceTypes.find(pCFG->eRaceType);
        if(it == r.second.setRaceTypes.end())
          continue;
      }

      if(r.second.setNatureTypes.empty() == false)
      {
        auto it = r.second.setNatureTypes.find(pCFG->eNatureType);
        if(it == r.second.setNatureTypes.end())
          continue;
      }
      setRwds.insert(r.second.setExtraIDs.begin(), r.second.setExtraIDs.end());
    }
    else
    {
      if(r.second.setZoneTypes.empty() == false)
      {
        auto it = r.second.setZoneTypes.find(pCFG->eZoneType);
        if(it != r.second.setZoneTypes.end())
          continue;
      }

      if(r.second.setNpcTypes.empty() == false)
      {
        auto it = r.second.setNpcTypes.find(pCFG->eNpcType);
        if(it != r.second.setNpcTypes.end())
          continue;
      }

      if(r.second.setRaceTypes.empty() == false)
      {
        auto it = r.second.setRaceTypes.find(pCFG->eRaceType);
        if(it != r.second.setRaceTypes.end())
          continue;
      }

      if(r.second.setNatureTypes.empty() == false)
      {
        auto it = r.second.setNatureTypes.find(pCFG->eNatureType);
        if(it != r.second.setNatureTypes.end())
          continue;
      }

      setRwds.insert(r.second.setExtraIDs.begin(), r.second.setExtraIDs.end());
    }
  }

  if (!setRwds.empty())
  {
    XLOG << "[Reward-额外掉落], 怪物:" << pCFG->strName << pCFG->dwID << "额外Reward ID";
    for (auto &s : setRwds)
      XLOG << s;
    XLOG << XEND;
  }

  return setRwds.empty() == false;
}

bool RewardConfig::addExtraRwd(EExtraRewardType eType, const SExtraRewardData& rwdData)
{
  auto it = m_mapExtraReward.find(eType);
  if (it != m_mapExtraReward.end())
  {
    XERR << "[Reward-额外], 添加失败, 已包含类型:" << eType << XEND;
    return false;
  }

  SExtraRewardData& stData = m_mapExtraReward[eType];
  stData.eType = eType;
  stData.setRewards.insert(rwdData.setRewards.begin(), rwdData.setRewards.end());
  stData.dwNeedTimes = rwdData.dwNeedTimes;
  stData.dwDayRewardTimes = rwdData.dwDayRewardTimes;
  stData.dwAccLimit = rwdData.dwAccLimit;
  if (!rwdData.mapParam2Rewards.empty())
  {
    for (auto &m : rwdData.mapParam2Rewards)
      stData.mapParam2Rewards[m.first].insert(m.second.begin(), m.second.end());
  }

  XLOG << "[Reward-额外], 添加额外掉落" << eType << "奖励ID:";
  for (auto &s : stData.setRewards)
    XLOG << s;
  XLOG << XEND;

  if (m_mapExtraReward.size() == 1)
    ActivityConfig::getMe().markActivityOpen("extra_reward");
  return true;
}

bool RewardConfig::delExtraRwd(EExtraRewardType eType)
{
  auto it = m_mapExtraReward.find(eType);
  if (it == m_mapExtraReward.end())
  {
    XERR << "[Reward-额外], 删除, 无法找到类型" << eType << XEND;
    return false;
  }

  m_mapExtraReward.erase(it);

  if (m_mapExtraReward.empty())
    ActivityConfig::getMe().markActivityClose("extra_reward");

  XLOG << "[Reward-额外], 删除类型" << eType << "成功" << XEND;
  return true;
}

bool RewardConfig::getExtraByType(EExtraRewardType eType, DWORD donecount, DWORD nowtimes, TSetDWORD& setRwds, DWORD param /*=0*/)
{
  auto it = m_mapExtraReward.find(eType);
  if (it == m_mapExtraReward.end())
    return false;

  if (eType != EEXTRAREWARD_GVG && eType != EEXTRAREWARD_SUPERGVG)
  {
    if (donecount == 0)
      return false;

    if (it->second.dwDayRewardTimes <= nowtimes)
      return false;

    if (it->second.dwNeedTimes && donecount % it->second.dwNeedTimes != 0)
      return false;
  }

  // 条件筛选, param : 决战等级等
  if (param)
  {
    auto s = it->second.mapParam2Rewards.find(param);
    if (s == it->second.mapParam2Rewards.end())
      return false;
    setRwds.insert(s->second.begin(), s->second.end());
  }
  else
  {
    setRwds.insert(it->second.setRewards.begin(), it->second.setRewards.end());
  }

  XLOG << "[Reward-额外], 类型:" << eType << "获得额外奖励ID:";
  for (auto &s : setRwds)
    XLOG << s;
  XLOG << XEND;

  return true;
}

bool RewardConfig::getExtraRwdAccLimit(EExtraRewardType eType)
{
  auto it = m_mapExtraReward.find(eType);
  if (it == m_mapExtraReward.end())
    return false;

  return it->second.dwAccLimit;
}

EVarType RewardConfig::getVarByExtra(EExtraRewardType eType)
{
  EVarType varType = EVARTYPE_MIN;
  switch(eType)
  {
    case EEXTRAREWARD_WANTEDQUEST:
      varType = EVARTYPE_EXTRARWD_WANTEDQUEST;
      break;
    case EEXTRAREWARD_DAILYMONSTER:
      varType = EVARTYPE_EXTRARWD_DAILYMONSTER;
      break;
    case EEXTRAREWARD_SEAL:
      varType = EVARTYPE_EXTRARWD_SEAL;
      break;
    case EEXTRAREWARD_LABORATORY:
      varType = EVARTYPE_EXTRARWD_LABORATORY;
      break;
    case EEXTRAREWARD_ENDLESS:
      varType = EVARTYPE_EXTRARWD_ENDLESS;
      break;
    case EEXTRAREWARD_GUILD_QUEST:
      varType = EVARTYPE_EXTRARWD_GUILD_QUEST;
      break;
    case EEXTRAREWARD_GUILD_DONATE:
      varType = EVARTYPE_EXTRARWD_GUILD_DONATE;
      break;
    case EEXTRAREWARD_PVECARD:
      varType = EVARTYPE_EXTRARWD_PVECARD;
      break;
    default:
      break;
  }

  return varType;
}

EAccVarType RewardConfig::getAccVarByExtra(EExtraRewardType eType)
{
  EAccVarType varType = EACCVARTYPE_MIN;
  switch(eType)
  {
    case EEXTRAREWARD_WANTEDQUEST:
      varType = EACCVARTYPE_EXTRARWD_WANTEDQUEST;
      break;
    case EEXTRAREWARD_DAILYMONSTER:
      varType = EACCVARTYPE_EXTRARWD_DAILYMONSTER;
      break;
    case EEXTRAREWARD_SEAL:
      varType = EACCVARTYPE_EXTRARWD_SEAL;
      break;
    case EEXTRAREWARD_LABORATORY:
      varType = EACCVARTYPE_EXTRARWD_LABORATORY;
      break;
    case EEXTRAREWARD_ENDLESS:
      varType = EACCVARTYPE_EXTRARWD_ENDLESS;
      break;
    case EEXTRAREWARD_GUILD_QUEST:
      varType = EACCVARTYPE_EXTRARWD_GUILD_QUEST;
      break;
    case EEXTRAREWARD_GUILD_DONATE:
      varType = EACCVARTYPE_EXTRARWD_GUILD_DONATE;
      break;
    case EEXTRAREWARD_PVECARD:
      varType = EACCVARTYPE_EXTRARWD_PVECARD;
      break;
    default:
      break;
  }

  return varType;
}

bool RewardConfig::addDoubleRwd(EDoubleRewardType eType, const SDoubleRewardData& rwdData)
{
  auto it = m_mapDoubleReward.find(eType);
  if (it != m_mapDoubleReward.end())
  {
    XERR << "[Reward-双倍奖励], 添加失败, 已包含类型:" << eType << XEND;
    return false;
  }

  SDoubleRewardData& stData = m_mapDoubleReward[eType];
  stData.eType = eType;
  stData.dwNeedTimes = rwdData.dwNeedTimes;
  stData.dwDayRewardTimes = rwdData.dwDayRewardTimes;
  stData.dwTimes = rwdData.dwTimes;
  stData.dwAccLimit = rwdData.dwAccLimit;

  XLOG << "[Reward-双倍奖励], 添加双倍奖励成功" << eType << "奖励ID:";
  XLOG << "[Reward-双倍奖励], 配置"<< rwdData.dwNeedTimes << rwdData.dwDayRewardTimes << rwdData.dwTimes << rwdData.dwAccLimit <<XEND;

  if (m_mapExtraReward.size() == 1)
    ActivityConfig::getMe().markActivityOpen("double_reward");
  return true;
}

bool RewardConfig::delDoubleRwd(EDoubleRewardType eType)
{
  auto it = m_mapDoubleReward.find(eType);
  if (it == m_mapDoubleReward.end())
  {
    XERR << "[Reward-双倍奖励], 删除, 无法找到类型" << eType << XEND;
    return false;
  }

  m_mapDoubleReward.erase(it);

  if (m_mapDoubleReward.empty())
    ActivityConfig::getMe().markActivityClose("double_reward");

  XLOG << "[Reward-双倍奖励], 删除类型" << eType << "成功" << XEND;
  return true;
}

DWORD RewardConfig::getDoubleRewardTimesByType(EDoubleRewardType eType, DWORD donwcount, DWORD nowtimes)
{
  auto it = m_mapDoubleReward.find(eType);
  if (it == m_mapDoubleReward.end())
    return 1;

  if (donwcount == 0 || it->second.dwTimes == 0)
    return 1;

  if (it->second.dwDayRewardTimes != 0 && it->second.dwDayRewardTimes <= nowtimes)
    return 1;

  if (it->second.dwNeedTimes && donwcount % it->second.dwNeedTimes != 0)
    return 1;

  return it->second.dwTimes;
}

bool RewardConfig::getDoubleRwdAccLimit(EDoubleRewardType eType)
{
  auto it = m_mapDoubleReward.find(eType);
  if (it == m_mapDoubleReward.end())
    return false;

  return it->second.dwAccLimit;
}

EVarType RewardConfig::getVarByDouble(EDoubleRewardType eType)
{
  EVarType varType = EVARTYPE_MIN;
  switch(eType)
  {
    case EDOUBLEREWARD_WANTEDQUEST:
      varType = EVARTYPE_DOUBLERWD_WANTEDQUEST;
      break;
    case EDOUBLEREWARD_DAILYMONSTER:
      varType = EVARTYPE_DOUBLERWD_DAILYMONSTER;
      break;
    case EDOUBLEREWARD_SEAL:
      varType = EVARTYPE_DOUBLERWD_SEAL;
      break;
    case EDOUBLEREWARD_LABORATORY:
      varType = EVARTYPE_DOUBLERWD_LABORATORY;
      break;
    case EDOUBLEREWARD_ENDLESS:
      varType = EVARTYPE_DOUBLERWD_ENDLESS;
      break;
    case EDOUBLEREWARD_PVECARD:
      varType = EVARTYPE_DOUBLERWD_PVECARD;
      break;
    default:
      break;
  }

  return varType;
}

EAccVarType RewardConfig::getAccVarByDouble(EDoubleRewardType eType)
{
  EAccVarType varType = EACCVARTYPE_MIN;
  switch(eType)
  {
    case EDOUBLEREWARD_WANTEDQUEST:
      varType = EACCVARTYPE_DOUBLERWD_WANTEDQUEST;
      break;
    case EDOUBLEREWARD_DAILYMONSTER:
      varType = EACCVARTYPE_DOUBLERWD_DAILYMONSTER;
      break;
    case EDOUBLEREWARD_SEAL:
      varType = EACCVARTYPE_DOUBLERWD_SEAL;
      break;
    case EDOUBLEREWARD_LABORATORY:
      varType = EACCVARTYPE_DOUBLERWD_LABORATORY;
      break;
    case EDOUBLEREWARD_ENDLESS:
      varType = EACCVARTYPE_DOUBLERWD_ENDLESS;
      break;
    case EDOUBLEREWARD_PVECARD:
      varType = EACCVARTYPE_DOUBLERWD_PVECARD;
    default:
      break;
  }

  return varType;
}

RewardSafetyItem* RewardEntry::safetyitem(const SRewardCFG* pCFG)
{
  if (m_pmapSafetyItem == nullptr || pCFG == nullptr || pCFG->oSafety.isSafety() == false)
    return nullptr;

  DWORD id = pCFG->id;
  auto it = m_pmapSafetyItem->find(id);
  if (it == m_pmapSafetyItem->end())
  {
    RewardSafetyItem& item = (*m_pmapSafetyItem)[id];
    item.set_id(id);
    item.set_expiretime(pCFG->oSafety.getExpireTime());
    item.set_nextsafetycount(pCFG->oSafety.getNextSafetyCount());
    item.set_version(pCFG->oSafety.dwVersion);
    it = m_pmapSafetyItem->find(id);
    if (it == m_pmapSafetyItem->end())
      return nullptr;
  }
  else
  {
    if (now() >= it->second.expiretime() || it->second.version() != pCFG->oSafety.dwVersion)
    {
      XLOG << "[Reward-保底数据清理]" << m_qwCharID << it->second.ShortDebugString() << "version:" << pCFG->oSafety.dwVersion << "数据清理成功" << XEND;
      it->second.set_expiretime(pCFG->oSafety.getExpireTime());
      it->second.set_rollcount(0);
      it->second.set_rewardcount(0);
      it->second.set_nextsafetycount(pCFG->oSafety.getNextSafetyCount());
      it->second.set_version(pCFG->oSafety.dwVersion);
    }
  }

  return &it->second;
}

const SRewardItem* RewardEntry::safety(const SRewardCFG* pCFG, bool roll)
{
  RewardSafetyItem* sitem = safetyitem(pCFG);
  if (sitem == nullptr)
    return nullptr;

  sitem->set_rollcount(sitem->rollcount() + 1);

  if (roll) // 摇中,只记录摇中次数
  {
    sitem->set_rewardcount(sitem->rewardcount() + 1);
    sitem->set_rollcount(0);
    sitem->set_nextsafetycount(pCFG->oSafety.getNextSafetyCount());
    XLOG << "[Reward-保底更新]" << m_qwCharID << sitem->ShortDebugString() << "摇中保底奖励" << XEND;
  }
  else if ((pCFG->oSafety.dwLimit == 0 || sitem->rewardcount() < pCFG->oSafety.dwLimit) && sitem->nextsafetycount() && sitem->rollcount() >= sitem->nextsafetycount()) // 未摇中,判断是否触发保底机制
  {
    int idx = 0;
    for (auto& v : pCFG->vecItems)
    {
      if (v.bIsSafety && v.isValid(*this))
      {
        XLOG << "[Reward-保底更新]" << m_qwCharID << sitem->ShortDebugString() << "摇中保底奖励索引:" << idx << XEND;
        sitem->set_rewardcount(sitem->rewardcount() + 1);
        sitem->set_rollcount(0);
        sitem->set_nextsafetycount(pCFG->oSafety.getNextSafetyCount());
        if (BaseConfig::getMe().getInnerBranch() == BRANCH_DEBUG)
            MsgManager::sendDebugMsg(m_qwCharID, "恭喜中奖");
        return &v;
      }
      idx += 1;
    }
    XERR << "[Reward-保底更新]" << m_qwCharID << sitem->ShortDebugString() << "未找到保底奖励" << XEND;
    return nullptr;
  }
  else
  {
    XLOG << "[Reward-保底更新]" << m_qwCharID << sitem->ShortDebugString() << "没有摇中奖励,未触发保底" << XEND;
  }

  return nullptr;
}
